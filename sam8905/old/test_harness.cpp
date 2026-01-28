// SAM8905 Standalone Test Harness
// Loads A-RAM/D-RAM captures and generates WAV output

#include "sam8905_core.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <fstream>

// WAV file header
struct WavHeader {
	char riff[4] = {'R', 'I', 'F', 'F'};
	uint32_t file_size;
	char wave[4] = {'W', 'A', 'V', 'E'};
	char fmt[4] = {'f', 'm', 't', ' '};
	uint32_t fmt_size = 16;
	uint16_t audio_format = 1;  // PCM
	uint16_t num_channels = 2;
	uint32_t sample_rate = 22050;
	uint32_t byte_rate;
	uint16_t block_align;
	uint16_t bits_per_sample = 16;
	char data[4] = {'d', 'a', 't', 'a'};
	uint32_t data_size;
};

bool write_wav(const char* filename, const int16_t* samples, size_t num_samples, int sample_rate = 22050)
{
	WavHeader header;
	header.sample_rate = sample_rate;
	header.num_channels = 2;
	header.bits_per_sample = 16;
	header.block_align = header.num_channels * header.bits_per_sample / 8;
	header.byte_rate = header.sample_rate * header.block_align;
	header.data_size = num_samples * 2 * sizeof(int16_t);  // stereo
	header.file_size = sizeof(WavHeader) - 8 + header.data_size;

	std::ofstream f(filename, std::ios::binary);
	if (!f) {
		fprintf(stderr, "Failed to create WAV file: %s\n", filename);
		return false;
	}

	f.write(reinterpret_cast<char*>(&header), sizeof(header));
	f.write(reinterpret_cast<const char*>(samples), header.data_size);

	printf("Wrote WAV: %s (%zu samples, %.2f seconds)\n",
		   filename, num_samples, (float)num_samples / sample_rate);
	return true;
}

void print_usage(const char* prog)
{
	printf("SAM8905 Test Harness\n\n");
	printf("Usage: %s [options]\n\n", prog);
	printf("Options:\n");
	printf("  -a <file>    Load A-RAM algorithm from file (into slot 0)\n");
	printf("  -d <file>    Load D-RAM configuration from file\n");
	printf("  -o <file>    Output WAV file (default: output.wav)\n");
	printf("  -s <seconds> Duration in seconds (default: 1.0)\n");
	printf("  -r <rate>    Sample rate: 22050 or 44100 (default: 22050)\n");
	printf("  --test       Run built-in test (minimal sine)\n");
	printf("  --dump       Dump A-RAM and D-RAM state after loading\n");
	printf("  -h           Show this help\n");
	printf("\nExample:\n");
	printf("  %s -a test_data/algo_minimal.bin -d test_data/dram_sine_440hz.bin -o sine.wav\n", prog);
}

// Helper to build SAM8905 instruction opcode
// CMD: 0=RM, 1=RADD, 2=RP, 3=RSP
// Receivers are active LOW (0 = enabled, 1 = disabled)
uint16_t make_inst(int mad, int cmd, bool wsp,
                   bool wa_dis, bool wb_dis, bool wm_dis, bool wphi_dis,
                   bool wxy_dis, bool clrb_dis, bool wwf_dis, bool wacc_dis)
{
	uint16_t inst = 0;
	inst |= (mad & 0xF) << 11;
	inst |= (cmd & 0x3) << 9;
	inst |= wsp ? 0x100 : 0;
	inst |= wa_dis ? 0x80 : 0;
	inst |= wb_dis ? 0x40 : 0;
	inst |= wm_dis ? 0x20 : 0;
	inst |= wphi_dis ? 0x10 : 0;
	inst |= wxy_dis ? 0x08 : 0;
	inst |= clrb_dis ? 0x04 : 0;
	inst |= wwf_dis ? 0x02 : 0;
	inst |= wacc_dis ? 0x01 : 0;
	return inst;
}

void run_builtin_test(SAM8905Core& sam, const char* output_file, float duration)
{
	printf("Running built-in test: internal sine wave\n\n");

	sam.set_ssr_mode(true);  // 22.05kHz for Keyfox10

	// Instruction format (15 bits):
	// | MAD(4) | CMD(2) | WSP | WA | WB | WM | WPHI | WXY | clrB | WWF | WACC |
	// | 14-11  | 10-9   |  8  |  7 |  6 |  5 |   4  |  3  |   2  |  1  |   0  |
	// CMD: 0=RM, 1=RADD, 2=RP, 3=RSP
	// Receivers are active LOW (0 = enabled)

	// Manual's Sine Oscillator Algorithm (Section 7, Example 1)
	// PHI=0, DPHI=1, AMP=2
	//
	// RM      PHI,    <WA,WPHI,WSP>  ;Areg=PHIreg=D-RAM(PHI) WFreg=100H (sinus)
	// RM      DPHI,   <WB>           ;Breg=D-RAM(DPHI)
	// RM      AMP,    <WXY,WSP>      ;X=sin(PHI) Y=AMP mix updated
	// RADD    PHI,    <WM>           ;D-RAM(PHI)= Areg + Breg (PHI+DPHI)
	// RSP                            ;NOP to wait 3 cycles from WXY
	//                   ,<WACC>      ;accumulate AMP x sin(PHI) through mix

	// Inst 0: RM PHI, <WA,WPHI,WSP> - Load A and PHI, set WF=0x100
	// Note: "WPHI WSP takes priority over WA WSP giving a normal WA" (Section 8-3)
	uint16_t inst0 = make_inst(0, 0, true,   // MAD=0, CMD=RM, WSP=1
	                           false, true, true, false, true, true, true, true);
	printf("Inst 0: 0x%04X (RM[0], WA, WPHI, WSP)\n", inst0);
	sam.write_aram(0, inst0);

	// Inst 1: RM DPHI, <WB> - Load frequency to B
	uint16_t inst1 = make_inst(1, 0, false,  // MAD=1, CMD=RM, WSP=0
	                           true, false, true, true, true, true, true, true);
	printf("Inst 1: 0x%04X (RM[1], WB)\n", inst1);
	sam.write_aram(1, inst1);

	// Inst 2: RM AMP, <WXY,WSP> - X=sin(PHI), Y=AMP, load mix
	uint16_t inst2 = make_inst(2, 0, true,   // MAD=2, CMD=RM, WSP=1
	                           true, true, true, true, false, true, true, true);
	printf("Inst 2: 0x%04X (RM[2], WXY, WSP)\n", inst2);
	sam.write_aram(2, inst2);

	// Inst 3: RADD PHI, <WM> - Store new phase (A+B) to D-RAM[0]
	uint16_t inst3 = make_inst(0, 1, false,  // MAD=0, CMD=RADD, WSP=0
	                           true, true, false, true, true, true, true, true);
	printf("Inst 3: 0x%04X (RADD[0], WM)\n", inst3);
	sam.write_aram(3, inst3);

	// Inst 4: RSP - Wait for multiplier (3 cycles from WXY to WACC)
	uint16_t inst4 = make_inst(0, 3, false,  // MAD=0, CMD=RSP, WSP=0
	                           true, true, true, true, true, true, true, true);
	printf("Inst 4: 0x%04X (RSP/NOP)\n", inst4);
	sam.write_aram(4, inst4);

	// Inst 5: <WACC> - Accumulate to output
	uint16_t inst5 = make_inst(0, 3, false,  // MAD=0, CMD=RSP, WSP=0
	                           true, true, true, true, true, true, true, false);
	printf("Inst 5: 0x%04X (WACC)\n", inst5);
	sam.write_aram(5, inst5);

	// NOP for remaining
	uint16_t nop = make_inst(0, 3, false, true, true, true, true, true, true, true, true);
	for (int i = 6; i < 64; ++i) {
		sam.write_aram(i, nop);
	}
	printf("\n");

	// Set up D-RAM for slot 0 (matching manual: PHI=0, DPHI=1, AMP=2)
	// Word 0: Initial phase = 0 (already zero from reset)

	// Word 1: DPHI for 440Hz
	// f = 0.042057 * DPHI for 22.05kHz mode
	// DPHI = 440 / 0.042057 = 10462
	sam.write_dram(1, 10462);

	// Word 2: Amplitude + MIX
	// Format: | AMP (bits 18-7) | X | MIXL (bits 5-3) | MIXR (bits 2-0) |
	uint32_t amp = 0x7FF;  // max amplitude (12 bits)
	uint32_t mixl = 7, mixr = 7;  // 0dB
	sam.write_dram(2, (amp << 7) | (mixl << 3) | mixr);

	// Word 15: Control - ALG=0, active
	sam.write_dram(15, 0);

	// Mark other slots as idle
	for (int s = 1; s < 16; ++s) {
		sam.write_dram((s << 4) | 15, 0x800);  // I bit = 1
	}

	printf("D-RAM slot 0 configured:\n");
	sam.dump_slot(0);

	printf("\nA-RAM algorithm 0:\n");
	sam.dump_aram(0);

	// Generate samples
	size_t num_samples = (size_t)(22050 * duration);
	std::vector<int16_t> samples(num_samples * 2);

	printf("\nGenerating %zu samples...\n", num_samples);
	sam.generate_samples(samples.data(), num_samples);

	// Check output
	int32_t max_l = 0, max_r = 0;
	for (size_t i = 0; i < num_samples; ++i) {
		int32_t l = abs(samples[i * 2]);
		int32_t r = abs(samples[i * 2 + 1]);
		if (l > max_l) max_l = l;
		if (r > max_r) max_r = r;
	}
	printf("Max amplitude: L=%d R=%d\n", max_l, max_r);

	write_wav(output_file, samples.data(), num_samples, 22050);
}

int main(int argc, char* argv[])
{
	const char* aram_file = nullptr;
	const char* dram_file = nullptr;
	const char* output_file = "output.wav";
	float duration = 1.0f;
	int sample_rate = 22050;
	bool run_test = false;
	bool dump_state = false;

	// Parse arguments
	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "-a") == 0 && i + 1 < argc) {
			aram_file = argv[++i];
		} else if (strcmp(argv[i], "-d") == 0 && i + 1 < argc) {
			dram_file = argv[++i];
		} else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
			output_file = argv[++i];
		} else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
			duration = atof(argv[++i]);
		} else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc) {
			sample_rate = atoi(argv[++i]);
		} else if (strcmp(argv[i], "--test") == 0) {
			run_test = true;
		} else if (strcmp(argv[i], "--dump") == 0) {
			dump_state = true;
		} else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			print_usage(argv[0]);
			return 0;
		} else {
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
			print_usage(argv[0]);
			return 1;
		}
	}

	SAM8905Core sam;
	sam.set_ssr_mode(sample_rate == 22050);

	if (run_test) {
		run_builtin_test(sam, output_file, duration);
		return 0;
	}

	if (!aram_file && !dram_file) {
		fprintf(stderr, "No input files specified. Use --test for built-in test.\n\n");
		print_usage(argv[0]);
		return 1;
	}

	// Load A-RAM
	if (aram_file) {
		sam.load_aram_file(aram_file, 0);  // Load into algo slot 0
	}

	// Load D-RAM
	if (dram_file) {
		sam.load_dram_file(dram_file);
	}

	if (dump_state) {
		printf("\nLoaded state:\n");
		printf("A-RAM algorithm 0:\n");
		sam.dump_aram(0);
		printf("\nD-RAM slot 0:\n");
		sam.dump_slot(0);
	}

	// Generate samples
	size_t num_samples = (size_t)(sample_rate * duration);
	std::vector<int16_t> samples(num_samples * 2);

	printf("Generating %zu samples at %d Hz...\n", num_samples, sample_rate);
	sam.generate_samples(samples.data(), num_samples);

	// Check output levels
	int32_t max_l = 0, max_r = 0;
	for (size_t i = 0; i < num_samples; ++i) {
		int32_t l = abs(samples[i * 2]);
		int32_t r = abs(samples[i * 2 + 1]);
		if (l > max_l) max_l = l;
		if (r > max_r) max_r = r;
	}
	printf("Max amplitude: L=%d R=%d\n", max_l, max_r);

	if (max_l == 0 && max_r == 0) {
		printf("WARNING: No audio output! Check D-RAM configuration.\n");
	}

	write_wav(output_file, samples.data(), num_samples, sample_rate);

	return 0;
}
