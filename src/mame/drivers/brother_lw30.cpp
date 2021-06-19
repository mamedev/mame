// license:BSD-3-Clause
// copyright-holders:Bartman/Abyss

#include "emu.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "machine/timer.h"
#include "cpu/z180/z180.h"
#include "debug/debugcpu.h"
#include "sound/beep.h"
#include "video/mc6845.h"

// if updating project, c:\msys64\win32env.bat
// cd \schreibmaschine\mame_src
// make SUBTARGET=schreibmaschine NO_USE_MIDI=1 NO_USE_PORTAUDIO=1 vs2019

// command line parameters:
// -log -debug -window -intscalex 2 -intscaley 2 lw30 -resolution 960x256 -flop roms\lw30\tetris.img
// -debug -autoboot_script c:\schreibmaschine\mame_src\lw30.lua -log -window -intscalex 8 -intscaley 8 -resolution 1920x512 lw30 -flop c:\brothers\mame\disks\lw30\tetris.img 

#pragma region(LW-30)

//////////////////////////////////////////////////////////////////////////
// LW-30
//////////////////////////////////////////////////////////////////////////

/***************************************************************************

Brother LW-30
1991

Hardware:

#7 
Hitachi
HD64180RP6
8-bit CMOS Micro Processing Unit
fully compatible with Zilog Z80180 (Z180)
6 MHz, DP-64S, Address Space 512 K Byte
MuRata CST12MTW 12.00 MHz Ceramic Resonator

#8 
Mitsubishi
M65122ASP
UA5445-B LC-1

#6 
NEC
D23C4001EC-172
UA2849-A
4MBit Mask ROM for Dictionary

#5
LH532H07
UA5362-A
2MBit Mask ROM

#11 
Hitachi
HM6264ALP-15
High Speed CMOS Static RAM (8kbit x 8) 150ns

#10
Mitsubishi
M65017FP
UA5498-A
Murata CST4MGW 4.00 MHz Ceramic Resonator

#3, #4
Mitsubishi
HD74LS157P

#1, #2
NEC
D41464C-10
Dynamic NMOS RAM (64kbit x 4) 100ns

QA1, QA2
Mitsubishi
M54587P

#12
Texas Instruments
SN74HC04N

Floppy - custom single sided 3.5" DD
240kb capacity
256 bytes/sector
12 sectors/track
78 tracks
custom 5-to-8 GCR encoding (very similar to Apple II's 5-and-3 encoding)
300 rpm
FF FF FF used as sync-start, AB sync-mark for sector header, DE sync-mark for sector data
FAT12 File System

ROHM
BA6580DK
Read/Write Amplifier for FDD

Emulation Status:
Printer not working
Floppy Disk writing not working
Floppy Disk Sync not implemented (reading works)
Dictionary ROM not working
Cursor shapes not implemented except block cursor

TODO: find self-test; verify RAM address map
// 8kb SRAM, 64kb DRAM <- where?

***************************************************************************/

#pragma region(Floppy)

class lw30_floppy_image_device;

class lw30_floppy_connector : public device_t, public device_slot_interface
{
public:
	lw30_floppy_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~lw30_floppy_connector();

	lw30_floppy_image_device *get_device();

protected:
	void device_start() override;
	void device_config_complete() override;
};

class lw30_floppy_image_device : public device_t, public device_image_interface
{
public:
	lw30_floppy_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~lw30_floppy_image_device();

public:
	iodevice_t image_type() const noexcept override { return IO_FLOPPY; }
	bool is_readable() const noexcept override { return true; }
	bool is_writeable() const noexcept override { return true; }
	bool is_creatable() const noexcept override { return true; }
	bool must_be_loaded() const noexcept override { return false; }
	bool is_reset_on_load() const noexcept override { return false; }
	const char *file_extensions() const noexcept override { return "img"; }
	const char *image_interface() const noexcept override { return "lw30_floppy_35"; }
	image_init_result call_load() override;
	void call_unload() override;

	bool loaded = false;
	bool dirty = false;

	// from write_format, write_sector_data_header_data_footer
	constexpr static auto sector_length = 8/*sector_prefix*/ + 2/*track_sync*/ + 2/*sector_sync*/ + 1/*0xdd*/ + 16/*sector_header*/ + 416/*payload*/ + 11/*sector_footer*/;
	constexpr static auto track_length = 2/*0xaa*/ + 48/*0xaa*/ + 12 * sector_length;

	std::array<std::array<uint8_t, track_length + 1024/*safety, temporary*/>, 78> tracks;

protected:
	void device_start() override { }
};

DEFINE_DEVICE_TYPE(LW30_FLOPPY_CONNECTOR, lw30_floppy_connector, "lw30_floppy_connector", "LW-30 Floppy drive connector abstraction")
DEFINE_DEVICE_TYPE(LW30_FLOPPY, lw30_floppy_image_device, "lw30_floppy_35", "LW-30 3.5\" floppy drive")

lw30_floppy_connector::lw30_floppy_connector(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, LW30_FLOPPY_CONNECTOR, tag, owner, clock),
	device_slot_interface(mconfig, *this)
{
}

lw30_floppy_connector::~lw30_floppy_connector()
{
}

void lw30_floppy_connector::device_start()
{
}

void lw30_floppy_connector::device_config_complete()
{
}

lw30_floppy_image_device *lw30_floppy_connector::get_device()
{
	return dynamic_cast<lw30_floppy_image_device *>(get_card_device());
}

lw30_floppy_image_device::lw30_floppy_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, LW30_FLOPPY, tag, owner, clock),
	device_image_interface(mconfig, *this)
{
}

lw30_floppy_image_device::~lw30_floppy_image_device()
{
}

namespace {
	constexpr uint16_t sync_table[]{
		0xDAEF, 0xB7AD, 0xFBBE, 0xEADF, 0xBFFA, 0xAEB6, 0xF5D7, 0xDBEE, 0xBAAB, 0xFDBD, 
		0xEBDE, 0xD5F7, 0xAFB5, 0xF6D6, 0xDDED, 0xBBAA, 0xEDBB, 0xD6DD, 0xB5F6, 0xF7AF, 
		0xDED5, 0xBDEB, 0xABFD, 0xEEBA, 0xD7DB, 0xB6F5, 0xFAAE, 0xDFBF, 0xBEEA, 0xADFB, 
		0xEFB7, 0xDADA, 0xB7EF, 0xFBAD, 0xEABE, 0xBFDF, 0xAEFA, 0xF5B6, 0xDBD7, 0xBAEE, 
		0xFDAB, 0xEBBD, 0xD5DE, 0xAFF7, 0xF6B5, 0xDDD6, 0xBBED, 0xAADD, 0xEDF6, 0xD6AF, 
		0xB5D5, 0xF7EB, 0xDEFD, 0xBDBA, 0xABDB, 0xEEF5, 0xD7AE, 0xB6BF, 0xFAEA, 0xDFFB, 
		0xBEB7, 0xADDA, 0xEFEF, 0xDAAD, 0xB7BE, 0xFBDF, 0xEAFA, 0xBFB6, 0xAED7, 0xF5EE, 
		0xDBAB, 0xBABD, 0xFDDE, 0xEBF7, 0xD5B5, 0xAFD6, 0xF6ED, 0xDDAA, 0xD6BB, 0xB5DD
	};

	constexpr uint8_t gcr_table[]{
		0xAA, 0xAB, 0xAD, 0xAE, 0xAF, 0xB5, 0xB6, 0xB7,
		0xBA, 0xBB, 0xBD, 0xBE, 0xBF, 0xD5, 0xD6, 0xD7,
		0xDA, 0xDB, 0xDD, 0xDE, 0xDF, 0xEA, 0xEB, 0xED,
		0xEE, 0xEF, 0xF5, 0xF6, 0xF7, 0xFA, 0xFB, 0xFD,
		0xFE, 0xFF // reserved
	};

	// format
	constexpr uint8_t sector_prefix[]{
		0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xAB
	};

	// format
	constexpr uint8_t sector_predata[]{
		0xDD, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
		0xAA, 0xAA, 0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
		0xED//, 0xAD, 0xEE, 0xF5, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD,
		//0xDD, 0xDD, 0xDD, 0xDD, 0xDD
	};

	// write
	constexpr uint8_t sector_header[]{
		0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xBF,
		0xFF, 0xFF, 0xFF, 0xFF, /*0xFF,*/ 0xFE, 0xED
	};

	// write
	constexpr uint8_t sector_footer[]{
		0xF5, 0xDD, 0x00, 0x00, 0x00, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD
	};

	constexpr uint8_t sector_interleave1[]{
		1, 6, 11, 4, 9, 2, 7, 12, 5, 10, 3, 8
	};

	constexpr uint8_t sector_interleave2[]{
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12
	};

	constexpr uint8_t sector_interleave3[]{
		1, 4, 7, 10, 6, 9, 12, 3, 11, 2, 5, 8
	};

	void gcr_encode_5_to_8(const uint8_t* input, uint8_t* output) {
		// input:
		// 76543210
		// --------
		// 00000111 0
		// 11222223 1
		// 33334444 2
		// 45555566 3
		// 66677777 4

		output[0] = gcr_table[(input[0] >> 3) & 0x1f];
		output[1] = gcr_table[((input[0] << 2) & 0x1f) | ((input[1] >> 6) & 0b11)];
		output[2] = gcr_table[(input[1] >> 1) & 0x1f];
		output[3] = gcr_table[((input[1] << 4) & 0x1f) | ((input[2] >> 4) & 0b1111)];
		output[4] = gcr_table[((input[2] << 1) & 0x1f) | ((input[3] >> 7) & 0b1)];
		output[5] = gcr_table[(input[3] >> 2) & 0x1f];
		output[6] = gcr_table[((input[3] << 3) & 0x1f) | ((input[4] >> 5) & 0b111)];
		output[7] = gcr_table[input[4] & 0x1f];
	}

	std::array<uint8_t, 3> checksum_256_bytes(const uint8_t* input) {
		size_t i = 0;
		uint8_t a = 0;
		uint8_t c = input[i++];
		uint8_t d = input[i++];
		uint8_t e = input[i++];
		for(size_t b = 0; b < 253; b++) {
			a = d;
			if(c & 0b10000000)
				a ^= 1;
			d = c;
			c = a;
			a = (d << 1) ^ e;
			e = d;
			d = a;
			e ^= input[i++];
		}

		return { c, d, e };
	}

	std::array<uint8_t, 416> gcr_encode_and_checksum(const uint8_t* input /* 256 bytes */) {
		std::array<uint8_t, 416> output;
		for(int i = 0; i < 51; i++)
			gcr_encode_5_to_8(&input[i * 5], &output[i * 8]);

		auto checksum = checksum_256_bytes(input);
		std::array<uint8_t, 5> end_and_checksum{ input[255], checksum[0], checksum[1], checksum[2], 0x58 };
		gcr_encode_5_to_8(&end_and_checksum[0], &output[408]);

		return output;
	}
}

image_init_result lw30_floppy_image_device::call_load()
{
	// create raw data from disk image
	auto p = reinterpret_cast<uint8_t*>(ptr());
	auto l = length();

	if(l != 78 * 12 * 256)
		return image_init_result::FAIL;

	for(size_t track = 0; track < 78; track++) {
		auto& data = tracks[track];
		size_t i = 0;
		for(size_t sector = 0; sector < 12; sector++) {
			// according to check_track_and_sector
			for(size_t j = 0; j < 5; j++)
				data[i++] = 0xff;
			data[i++] = 0xab;
			data[i++] = sync_table[track] & 0xff;
			data[i++] = sync_table[track] >> 8;
			data[i++] = sync_table[sector] & 0xff;
			data[i++] = sync_table[sector] >> 8;
			data[i++] = 0xdd;
			for(const auto& d : sector_header) // 16 bytes
				data[i++] = d;
			auto payload = gcr_encode_and_checksum(p + (track * 12 + sector) * 256); // 416 bytes
			for(const auto& d : payload)
				data[i++] = d;
			for(const auto& d : sector_footer) // 11 bytes
				data[i++] = d;
		}
	}

	loaded = true;
	return image_init_result::PASS;
}

void lw30_floppy_image_device::call_unload()
{
	if(dirty && !is_readonly()) {
		//#todo
		//if(reopen_for_write(filename()) == IMAGE_ERROR_SUCCESS)
		//	fwrite(image.get(), image_length);
	}

	loaded = false;
}

#pragma endregion

#pragma region(Sound)
class brother_beep_device : public device_t, public device_sound_interface
{
public:
	brother_beep_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	void device_start() override;

	// sound stream update overrides
	void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

public:
	DECLARE_WRITE_LINE_MEMBER(set_state);   // enable/disable sound output
	void set_clock(uint32_t frequency);       // output frequency

private:
	sound_stream *m_stream;   /* stream number */
	uint8_t m_state;          /* enable beep */
	int m_frequency;          /* set frequency - this can be changed using the appropriate function */
	int m_incr;               /* initial wave state */
	int8_t m_signal;         /* current signal */
};

DEFINE_DEVICE_TYPE(BROTHER_BEEP, brother_beep_device, "brother_beep", "Brother Beep")

static constexpr auto BROTHER_BEEP_RATE = 48000;

brother_beep_device::brother_beep_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, BROTHER_BEEP, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_stream(nullptr)
	, m_state(0xff)
	, m_frequency(clock)
{
}

void brother_beep_device::device_start()
{
	m_stream = stream_alloc(0, 1, BROTHER_BEEP_RATE);
	m_state = 0xff;
	m_signal = 0x01;

	// register for savestates
	save_item(NAME(m_state));
	save_item(NAME(m_frequency));
	save_item(NAME(m_incr));
	save_item(NAME(m_signal));
}

void brother_beep_device::sound_stream_update(sound_stream& stream, std::vector<read_stream_view> const& inputs, std::vector<write_stream_view>& outputs)
{
	auto& buffer = outputs[0];
	auto signal = m_signal;
	int clock = 0, rate = BROTHER_BEEP_RATE / 2;

	/* get progress through wave */
	int incr = m_incr;

	if(m_frequency > 0)
		clock = m_frequency;

	/* if we're not enabled, just fill with 0 */
	if(m_state == 0xff || clock == 0)
	{
		buffer.fill(0);
		return;
	}

	/* fill in the sample */
	for(int sampindex = 0; sampindex < buffer.samples(); sampindex++)
	{
		if(BIT(m_state, 7) != 0)
			buffer.put(sampindex, signal > 0 ? 1.f : -1.f);
		else
			buffer.put(sampindex, 0.f);

		incr -= clock;
		while(incr < 0)
		{
			incr += rate;
			signal = -signal;
		}
	}

	/* store progress through wave */
	m_incr = incr;
	m_signal = signal;
}

WRITE_LINE_MEMBER(brother_beep_device::set_state)
{
	// only update if new state is not the same as old state
	if(m_state == state)
		return;

	m_stream->update();

	if(m_state == 0) {
		// restart wave from beginning
		m_incr = 0;
		m_signal = 0x01;
	}
	m_state = state;
}

void brother_beep_device::set_clock(uint32_t frequency)
{
	if(m_frequency == frequency)
		return;

	m_stream->update();
	m_frequency = frequency;
	m_signal = 0x01;
	m_incr = 0;
}

#pragma endregion

class lw30_state : public driver_device
{
public:
	lw30_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		maincpu(*this, "maincpu"),
		screen(*this, "screen"),
		floppy(*this, "floppy"),
		beeper(*this, "beeper"),
		m_io_kbrow(*this, "kbrow.%u", 0)
	{ 
		//video_control = 0b00000010; // TEST LW-10 screen height
	}

	void lw30(machine_config& config);

	// helpers
	std::string pc();
	std::string symbolize(uint32_t adr);
	std::string callstack();

	// devices
	required_device<hd64180rp_device> maincpu;
	required_device<screen_device> screen;

	required_device<lw30_floppy_connector> floppy;
	required_device<brother_beep_device> beeper;
	optional_ioport_array<9> m_io_kbrow;

	// screen updates
	uint32_t screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect);

	uint8_t illegal_r(offs_t offset, uint8_t mem_mask = ~0) {
		logerror("%s: unmapped memory read from %0*X & %0*X\n", pc(), 6, offset, 2, mem_mask);
		return 0;
	}
	void illegal_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0) {
		logerror("%s: unmapped memory write to %0*X = %0*X & %0*X\n", pc(), 6, offset, 2, data, 2, mem_mask);
	}

	// ROM
	uint8_t rom42000_r(offs_t offset, uint8_t mem_mask = ~0) {
		return rom[0x02000 + offset] & mem_mask;
	}

	// IO
	void video_cursor_x_w(uint8_t data) { // 70
		video_cursor_x = data;
	}
	void video_cursor_y_w(uint8_t data) { // 71
		video_cursor_y = data;
	}
	void video_pos_x_w(uint8_t data) { // 72
		video_pos_x = data;
	}
	void video_pos_y_w(uint8_t data) { // 73
		video_pos_y = data;
	}
	uint8_t video_data_r() { // 74
		uint8_t data = 0x00;
		if(video_pos_y < 0x20)
			data = videoram[video_pos_y * 256 + video_pos_x];
		else
			logerror("%s: video_data_r out of range: x=%u, y=%u\n", callstack(), video_pos_x, video_pos_y);

		return data;
	}

	void video_data_w(uint8_t data) { // 74
		if(video_pos_y < 0x20)
			videoram[video_pos_y * 256 + video_pos_x] = data;
		else
			logerror("%s: video_data_w out of range: x=%u, y=%u\n", callstack(), video_pos_x, video_pos_y);

		video_pos_x++;
		if(video_pos_x == 0)
			video_pos_y++;
	}

	uint8_t video_control_r() { // 75 
		return video_control; 
	}
	void video_control_w(uint8_t data) { 
		video_control = data; // | 0b00000010; // TEST LW-10 screen height
	} 
	// 76
	uint8_t io_77_r() { // config
		uint8_t out = 0x20; // 14 lines
		out |= 0x00; // german
		//out |= 0x01; // french
		//out |= 0x02; // german
		return ~out;
	}

	// Floppy
	uint8_t io_80_r() {
		auto fd = floppy->get_device();
		if(!fd->loaded)
			return 0;

		if(floppy_steps / 4 < 0 || floppy_steps / 4 >= fd->tracks.size())
			return 0;

		auto& track = fd->tracks[floppy_steps / 4];
		floppy_track_offset %= track.size();

		auto ret = track[floppy_track_offset];
		//logerror("%s: read_io_80 track=%d,offset=%4x => %02x\n", callstack(), floppy_steps / 4, floppy_track_offset, ret);
		floppy_track_offset++;
		return ret;
	}
	uint8_t io_90_r() {
		// bit 7 set; data ready from floppy
		// bit 6 clear; unknown meaning
		return 0b10000000;
	}
	void io_90_w(uint8_t data) {
		// write directly to 4-wire bipolar stepper motor (see stepper_table)
		// a rotation to the left means decrease quarter-track
		// a rotation to the right means increase quarter-track
		auto rol4 = [](uint8_t d) { return ((d << 1) & 0b1111) | ((d >> 3) & 0b0001); };
		auto ror4 = [](uint8_t d) { return ((d >> 1) & 0b0111) | ((d << 3) & 0b1000); };
		switch(io_90) {
		case 0b0011:
		case 0b0110:
		case 0b1100:
		case 0b1001:
			if(data == rol4(io_90))
				floppy_steps--;
			else if(data == ror4(io_90))
				floppy_steps++;
			else
				logerror("%s: illegal step %02x=>%02x\n", callstack(), io_90, data);
			break;
		default:
			logerror("%s: initial step %02x=>%02x\n", callstack(), io_90, data);
			break;
		}
		logerror("%s: floppy_steps=%3d => track=%2d\n", callstack(), floppy_steps, floppy_steps / 4);
		io_90 = data;
	}
	uint8_t io_90{}; // stepper motor control
	uint8_t floppy_steps{}; // quarter track
	uint16_t floppy_track_offset{}; // current offset within track data

	uint8_t illegal_io_r(offs_t offset, uint8_t mem_mask = ~0) {
		logerror("%s: unmapped IO read from %0*X & %0*X\n", pc(), 4, offset + 0x40, 2, mem_mask);
		return 0;
	}
	void illegal_io_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0) {
		logerror("%s: unmapped IO write to %0*X = %0*X & %0*X\n", pc(), 4, offset + 0x40, 2, data, 2, mem_mask);
	}

	uint8_t io_b0_r() {
		// Tetris reads bit 3, needed for correct keyboard layout
		return 0b1000;
	}
	uint8_t lw30_state::io_b8_r() { // B8 (keyboard)
		// keyboard matrix
		if(io_b8 <= 8) {
			if(m_io_kbrow[io_b8].found()) {
				return m_io_kbrow[io_b8].read_safe(0);
			}
		}
		return 0x00;
	}
	void io_b8_w(uint8_t data) {
		io_b8 = data;
	}

	// Tetris Game Over Melody:
	// according to oscilloscope:
	// - total length 2.630s, wavosaur shows 2.043s
	// - last note: 600µs on, 600µs off, on-to-on: 1260µs, wavosaur shows 1083µs
	// according to MAME
	// - 11µs between writes to beeper
	// - 476µs (value=0x310=784) => 0.6071x between writes of 01 and 81 to beeper
	// - 713µs
	// - 724µs
	// - last note: 543µs (value=0x2ba=698) => 0.7779x

	void beeper_w(uint8_t data) { // F0
		#if 0
			static uint8_t lastData{};
			static attotime last{};
			static attotime lastDifferent{};
			attotime cur{ maincpu->local_time() };

			logerror("beeper_w(%02X) @ %s (delta=%sµs diff_delta=%sµs)\n", data, cur.as_string(), ((cur - last) * 1000000).as_string(0), ((cur - lastDifferent) * 1000000).as_string(0));
			if(data != lastData)
				lastDifferent = cur;
			last = cur;
			lastData = data;
		#endif

		beeper->set_state(data);
	}

	void lw30_state::irqack_w(uint8_t) { // F8
		maincpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
	}

	uint8_t videoram[0x2000]; // 80 chars * 14 lines; 2 bytes per char (attribute, char)
	uint8_t video_cursor_x, video_cursor_y, video_pos_x, video_pos_y, video_control, io_b8;
	uint8_t cursor_state;

	TIMER_DEVICE_CALLBACK_MEMBER(int1_timer_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(cursor_timer_callback);

protected:
	// driver_device overrides
	void machine_start() override;
	void machine_reset() override;

	void video_start() override;

	void map_program(address_map& map) {
		map(0x00000, 0x01fff).rom();
		map(0x02000, 0x05fff).ram();
		map(0x06000, 0x3ffff).rom();
		map(0x50000, 0x51fff).ram(); // ???
		map(0x61000, 0x61fff).ram();
		map(0x42000, 0x45fff).rw(FUNC(lw30_state::rom42000_r), FUNC(lw30_state::illegal_w)); // => ROM 0x02000-0x05fff
		map(0x65000, 0x70fff).ram();
	}

	void map_io(address_map& map) {
		map.global_mask(0xff);
		map(0x00, 0x3f).noprw(); // Z180 internal registers

		// video
		map(0x70, 0x70).w(FUNC(lw30_state::video_cursor_x_w));
		map(0x71, 0x71).w(FUNC(lw30_state::video_cursor_y_w));
		map(0x72, 0x72).w(FUNC(lw30_state::video_pos_x_w));
		map(0x73, 0x73).w(FUNC(lw30_state::video_pos_y_w));
		map(0x74, 0x74).rw(FUNC(lw30_state::video_data_r), FUNC(lw30_state::video_data_w));
		map(0x75, 0x75).rw(FUNC(lw30_state::video_control_r), FUNC(lw30_state::video_control_w));
		map(0x76, 0x76).noprw(); // NOP just to shut up the log
		map(0x77, 0x77).r(FUNC(lw30_state::io_77_r)).nopw(); // NOP just to shut up the log

		// floppy
		map(0x80, 0x80).r(FUNC(lw30_state::io_80_r));
		map(0x88, 0x88).noprw(); // NOP just to shut up the log
		map(0x90, 0x90).rw(FUNC(lw30_state::io_90_r), FUNC(lw30_state::io_90_w));
		map(0x98, 0x98).noprw(); // NOP just to shut up the log

		map(0xa8, 0xa8).noprw(); // NOP just to shut up the log
		map(0xb0, 0xb0).r(FUNC(lw30_state::io_b0_r));
		map(0xb8, 0xb8).rw(FUNC(lw30_state::io_b8_r), FUNC(lw30_state::io_b8_w));
		map(0xd8, 0xd8).noprw(); // NOP just to shut up the log
		map(0xf0, 0xf0).w(FUNC(lw30_state::beeper_w));
		map(0xf8, 0xf8).w(FUNC(lw30_state::irqack_w));

		//map(0x40, 0xff).rw(FUNC(lw30_state::illegal_io_r), FUNC(lw30_state::illegal_io_w));
	}

	uint8_t* rom{};
	std::map<uint32_t, std::string> symbols;
};

void lw30_state::video_start()
{
}

std::string lw30_state::pc()
{
	class z180_friend : public z180_device { public: using z180_device::memory_translate; friend class lw350_state; };
	auto cpu = static_cast<z180_friend*>(dynamic_cast<z180_device*>(&machine().scheduler().currently_executing()->device()));
	offs_t phys = cpu->pc();
	cpu->memory_translate(AS_PROGRAM, 0, phys);

	return symbolize(phys);
}

std::string lw30_state::symbolize(uint32_t adr)
{
	if(symbols.empty())
		return string_format("%06x", adr);

	auto floor_it = symbols.lower_bound(adr);
	if((floor_it == symbols.end() && !symbols.empty()) || floor_it->first != adr)
		--floor_it;
	if(floor_it != symbols.end())
		return string_format("%s+%x (%06x)", floor_it->second, adr - floor_it->first, adr);
	else
		return string_format("%06x", adr);
}

std::string lw30_state::callstack()
{
	class z180_friend : public z180_device { public: using z180_device::memory_translate; friend class lw350_state; };
	auto cpu = static_cast<z180_friend*>(dynamic_cast<z180_device*>(&machine().scheduler().currently_executing()->device()));
	offs_t pc = cpu->pc();
	cpu->memory_translate(AS_PROGRAM, 0, pc);

	int depth = 0;
	std::string output;
	output += symbolize(pc) + " >> ";

//	if(output.find("abort") != std::string::npos)
//		__debugbreak();

	// floppy routines
	if(pc >= 0x44000 && pc <= 0x46000) {
		offs_t sp = cpu->sp();
		for(int i = 0; i < 4; i++) {
			offs_t back = cpu->space(AS_PROGRAM).read_word_unaligned(sp);
			cpu->memory_translate(AS_PROGRAM, 0, back);
			if(back >= 0x44000 && back <= 0x46000) {
				output += symbolize(back) + " >> ";
				depth++;
				if(depth < 8)
					i = 0;
			}
			sp += 2;
		}
	}

	return output;
}

#pragma region(LW-30 Font)
// fonts ripped from LW-350 ROM
static const uint8_t lw30_font_normal[]{
	0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
	0b00000010, 0b00001010, 0b00001110, 0b00111110, 0b00001110, 0b00001010, 0b00000010, 0b00000000,
	0b00000010, 0b00000010, 0b00110010, 0b00101010, 0b00111110, 0b00101000, 0b00110000, 0b00000000,
	0b00001000, 0b00111110, 0b00011100, 0b00001000, 0b00111110, 0b00011100, 0b00001000, 0b00000000,
	0b00000000, 0b00000000, 0b00000000, 0b00101010, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
	0b00011100, 0b00001000, 0b00111110, 0b00100000, 0b00111100, 0b00100000, 0b00001000, 0b00011100,
	0b00111110, 0b00000000, 0b00011100, 0b00010000, 0b00011100, 0b00000100, 0b00011100, 0b00000000,
	0b00100000, 0b00100000, 0b00100000, 0b00101000, 0b00111100, 0b00001010, 0b00000010, 0b00000000,
	0b00000000, 0b00111110, 0b00111110, 0b00011100, 0b00011100, 0b00001000, 0b00000000, 0b00000000,
	0b00000000, 0b00111110, 0b00100010, 0b00010100, 0b00010100, 0b00001000, 0b00000000, 0b00000000,
	0b00111110, 0b00000000, 0b00110110, 0b00101010, 0b00100010, 0b00000000, 0b00111110, 0b00000000,
	0b00100000, 0b00110000, 0b00111000, 0b00111100, 0b00111000, 0b00110000, 0b00100000, 0b00000000,
	0b00011010, 0b00101010, 0b00010110, 0b00000010, 0b00011010, 0b00101010, 0b00010110, 0b00000000,
	0b00110110, 0b00100010, 0b00101010, 0b00110110, 0b00101010, 0b00100010, 0b00110110, 0b00000000,
	0b00101010, 0b00000000, 0b00100010, 0b00000000, 0b00100010, 0b00000000, 0b00101010, 0b00000000,
	0b00000010, 0b00000110, 0b00001110, 0b00011110, 0b00001110, 0b00000110, 0b00000010, 0b00000000,
	0b00001010, 0b00001010, 0b00110010, 0b00101010, 0b00111110, 0b00101000, 0b00110000, 0b00000000,
	0b00100100, 0b00111100, 0b00100100, 0b00101110, 0b00000010, 0b00000100, 0b00001110, 0b00000000,
	0b00011100, 0b00100010, 0b00001000, 0b00010100, 0b00100010, 0b00111110, 0b00100010, 0b00000000,
	0b00001100, 0b00010010, 0b00111110, 0b00100000, 0b00111100, 0b00100000, 0b00111110, 0b00000000,
	0b00011100, 0b00100010, 0b00000000, 0b00011100, 0b00001000, 0b00001000, 0b00011100, 0b00000000,
	0b00011100, 0b00100010, 0b00000000, 0b00011100, 0b00100010, 0b00100010, 0b00011100, 0b00000000,
	0b00011100, 0b00100010, 0b00000000, 0b00100010, 0b00100010, 0b00100010, 0b00011100, 0b00000000,
	0b00010100, 0b00000000, 0b00111110, 0b00100000, 0b00111100, 0b00100000, 0b00111110, 0b00000000,
	0b00010100, 0b00000000, 0b00011100, 0b00001000, 0b00001000, 0b00001000, 0b00011100, 0b00000000,
	0b00010100, 0b00000000, 0b00100010, 0b00100010, 0b00010100, 0b00001000, 0b00001000, 0b00000000,
	0b00010100, 0b00000000, 0b00100010, 0b00100010, 0b00011110, 0b00000010, 0b00111100, 0b00000000,
	0b00011000, 0b00000100, 0b00001000, 0b00010100, 0b00100010, 0b00111110, 0b00100010, 0b00000000,
	0b00110000, 0b00001000, 0b00111110, 0b00100000, 0b00111100, 0b00100000, 0b00111110, 0b00000000,
	0b00011000, 0b00000100, 0b00000000, 0b00011100, 0b00001000, 0b00001000, 0b00011100, 0b00000000,
	0b00011000, 0b00000100, 0b00000000, 0b00011100, 0b00100010, 0b00100010, 0b00011100, 0b00000000,
	0b00011000, 0b00000100, 0b00000000, 0b00100010, 0b00100010, 0b00100010, 0b00011100, 0b00000000,
	0b00011110, 0b00100000, 0b00011100, 0b00100010, 0b00011100, 0b00000010, 0b00111100, 0b00000000,
	0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00000000, 0b00001000, 0b00000000,
	0b00010100, 0b00010100, 0b00010100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
	0b00000000, 0b00011100, 0b00100000, 0b00100000, 0b00011100, 0b00001000, 0b00111000, 0b00000000,
	0b00001000, 0b00011110, 0b00101000, 0b00011100, 0b00001010, 0b00111100, 0b00001000, 0b00000000,
	0b00110000, 0b00110010, 0b00000100, 0b00001000, 0b00010000, 0b00100110, 0b00000110, 0b00000000,
	0b00011000, 0b00100100, 0b00101000, 0b00010000, 0b00101010, 0b00100100, 0b00011010, 0b00000000,
	0b00001000, 0b00001000, 0b00001000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
	0b00000100, 0b00001000, 0b00010000, 0b00010000, 0b00010000, 0b00001000, 0b00000100, 0b00000000,
	0b00010000, 0b00001000, 0b00000100, 0b00000100, 0b00000100, 0b00001000, 0b00010000, 0b00000000,
	0b00101010, 0b00011100, 0b00011100, 0b00101010, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
	0b00000000, 0b00001000, 0b00001000, 0b00111110, 0b00001000, 0b00001000, 0b00000000, 0b00000000,
	0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00011000, 0b00001000, 0b00010000, 0b00000000,
	0b00000000, 0b00000000, 0b00000000, 0b00111110, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
	0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00011000, 0b00011000, 0b00000000,
	0b00000010, 0b00000100, 0b00000100, 0b00001000, 0b00010000, 0b00010000, 0b00100000, 0b00000000,
	0b00011100, 0b00100010, 0b00100110, 0b00101010, 0b00110010, 0b00100010, 0b00011100, 0b00000000,
	0b00011000, 0b00101000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00111110, 0b00000000,
	0b00011100, 0b00100010, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00111110, 0b00000000,
	0b00011100, 0b00100010, 0b00000010, 0b00001100, 0b00000010, 0b00100010, 0b00011100, 0b00000000,
	0b00000100, 0b00001100, 0b00010100, 0b00100100, 0b00100100, 0b00111110, 0b00000100, 0b00000000,
	0b00111110, 0b00100000, 0b00111100, 0b00100010, 0b00000010, 0b00100010, 0b00011100, 0b00000000,
	0b00011100, 0b00100010, 0b00100000, 0b00111100, 0b00100010, 0b00100010, 0b00011100, 0b00000000,
	0b00111110, 0b00000010, 0b00000100, 0b00000100, 0b00001000, 0b00001000, 0b00001000, 0b00000000,
	0b00011100, 0b00100010, 0b00100010, 0b00011100, 0b00100010, 0b00100010, 0b00011100, 0b00000000,
	0b00011100, 0b00100010, 0b00100010, 0b00011110, 0b00000010, 0b00100010, 0b00011100, 0b00000000,
	0b00000000, 0b00011000, 0b00011000, 0b00000000, 0b00000000, 0b00011000, 0b00011000, 0b00000000,
	0b00000000, 0b00011000, 0b00011000, 0b00000000, 0b00011000, 0b00001000, 0b00010000, 0b00000000,
	0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00000000,
	0b00000000, 0b00000000, 0b00111110, 0b00000000, 0b00111110, 0b00000000, 0b00000000, 0b00000000,
	0b00001000, 0b00000000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00000000,
	0b00011100, 0b00100010, 0b00000010, 0b00001100, 0b00001000, 0b00000000, 0b00001000, 0b00000000,
	0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b00010000, 0b00001000, 0b00000100, 0b00000000,
	0b00001000, 0b00010100, 0b00100010, 0b00100010, 0b00111110, 0b00100010, 0b00100010, 0b00000000,
	0b00111100, 0b00010010, 0b00010010, 0b00011100, 0b00010010, 0b00010010, 0b00111100, 0b00000000,
	0b00011100, 0b00100010, 0b00100000, 0b00100000, 0b00100000, 0b00100010, 0b00011100, 0b00000000,
	0b00111100, 0b00010010, 0b00010010, 0b00010010, 0b00010010, 0b00010010, 0b00111100, 0b00000000,
	0b00111110, 0b00100000, 0b00100000, 0b00111100, 0b00100000, 0b00100000, 0b00111110, 0b00000000,
	0b00111110, 0b00100000, 0b00100000, 0b00111100, 0b00100000, 0b00100000, 0b00100000, 0b00000000,
	0b00011100, 0b00100010, 0b00100000, 0b00101110, 0b00100010, 0b00100010, 0b00011100, 0b00000000,
	0b00100010, 0b00100010, 0b00100010, 0b00111110, 0b00100010, 0b00100010, 0b00100010, 0b00000000,
	0b00011100, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00011100, 0b00000000,
	0b00001110, 0b00000100, 0b00000100, 0b00000100, 0b00100100, 0b00100100, 0b00011000, 0b00000000,
	0b00100010, 0b00100100, 0b00101000, 0b00110000, 0b00101000, 0b00100100, 0b00100010, 0b00000000,
	0b00100000, 0b00100000, 0b00100000, 0b00100000, 0b00100000, 0b00100000, 0b00111110, 0b00000000,
	0b00100010, 0b00110110, 0b00101010, 0b00101010, 0b00100010, 0b00100010, 0b00100010, 0b00000000,
	0b00100010, 0b00100010, 0b00110010, 0b00101010, 0b00100110, 0b00100010, 0b00100010, 0b00000000,
	0b00011100, 0b00100010, 0b00100010, 0b00100010, 0b00100010, 0b00100010, 0b00011100, 0b00000000,
	0b00111100, 0b00100010, 0b00100010, 0b00111100, 0b00100000, 0b00100000, 0b00100000, 0b00000000,
	0b00011100, 0b00100010, 0b00100010, 0b00100010, 0b00101010, 0b00100100, 0b00011010, 0b00000000,
	0b00111100, 0b00100010, 0b00100010, 0b00111100, 0b00101000, 0b00100100, 0b00100010, 0b00000000,
	0b00011100, 0b00100010, 0b00100000, 0b00011100, 0b00000010, 0b00100010, 0b00011100, 0b00000000,
	0b00111110, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00000000,
	0b00100010, 0b00100010, 0b00100010, 0b00100010, 0b00100010, 0b00100010, 0b00011100, 0b00000000,
	0b00100010, 0b00100010, 0b00100010, 0b00100010, 0b00010100, 0b00010100, 0b00001000, 0b00000000,
	0b00100010, 0b00100010, 0b00101010, 0b00101010, 0b00101010, 0b00010100, 0b00010100, 0b00000000,
	0b00100010, 0b00100010, 0b00010100, 0b00001000, 0b00010100, 0b00100010, 0b00100010, 0b00000000,
	0b00100010, 0b00100010, 0b00100010, 0b00010100, 0b00001000, 0b00001000, 0b00001000, 0b00000000,
	0b00111110, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b00111110, 0b00000000,
	0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00111111, 0b00000000, 0b00111111,
	0b00001100, 0b00010010, 0b00010000, 0b00111100, 0b00010000, 0b00010010, 0b00101100, 0b00000000,
	0b00001000, 0b00001000, 0b00111110, 0b00001000, 0b00001000, 0b00000000, 0b00111110, 0b00000000,
	0b00011000, 0b00100100, 0b00001000, 0b00010000, 0b00111100, 0b00000000, 0b00000000, 0b00000000,
	0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00111111,
	0b00010000, 0b00001000, 0b00000100, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00000000,
	0b00000000, 0b00000000, 0b00011100, 0b00000010, 0b00011110, 0b00100010, 0b00011110, 0b00000000,
	0b00100000, 0b00100000, 0b00111100, 0b00100010, 0b00100010, 0b00100010, 0b00111100, 0b00000000,
	0b00000000, 0b00000000, 0b00011110, 0b00100000, 0b00100000, 0b00100000, 0b00011110, 0b00000000,
	0b00000010, 0b00000010, 0b00011110, 0b00100010, 0b00100010, 0b00100010, 0b00011110, 0b00000000,
	0b00000000, 0b00000000, 0b00011100, 0b00100010, 0b00111110, 0b00100000, 0b00011100, 0b00000000,
	0b00000110, 0b00001000, 0b00111110, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00000000,
	0b00000000, 0b00000000, 0b00011110, 0b00100010, 0b00011110, 0b00000010, 0b00011100, 0b00000000,
	0b00100000, 0b00100000, 0b00111100, 0b00100010, 0b00100010, 0b00100010, 0b00100010, 0b00000000,
	0b00011000, 0b00000000, 0b00011000, 0b00001000, 0b00001000, 0b00001000, 0b00011100, 0b00000000,
	0b00001100, 0b00000000, 0b00001100, 0b00000100, 0b00000100, 0b00100100, 0b00011000, 0b00000000,
	0b00100000, 0b00100000, 0b00100010, 0b00100100, 0b00111000, 0b00100100, 0b00100010, 0b00000000,
	0b00011000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00011100, 0b00000000,
	0b00000000, 0b00000000, 0b00110100, 0b00101010, 0b00101010, 0b00101010, 0b00101010, 0b00000000,
	0b00000000, 0b00000000, 0b00101100, 0b00110010, 0b00100010, 0b00100010, 0b00100010, 0b00000000,
	0b00000000, 0b00000000, 0b00011100, 0b00100010, 0b00100010, 0b00100010, 0b00011100, 0b00000000,
	0b00000000, 0b00000000, 0b00111100, 0b00100010, 0b00111100, 0b00100000, 0b00100000, 0b00000000,
	0b00000000, 0b00000000, 0b00011110, 0b00100010, 0b00011110, 0b00000010, 0b00000010, 0b00000000,
	0b00000000, 0b00000000, 0b00101100, 0b00110010, 0b00100000, 0b00100000, 0b00100000, 0b00000000,
	0b00000000, 0b00000000, 0b00011110, 0b00100000, 0b00011100, 0b00000010, 0b00111100, 0b00000000,
	0b00001000, 0b00001000, 0b00111110, 0b00001000, 0b00001000, 0b00001000, 0b00000110, 0b00000000,
	0b00000000, 0b00000000, 0b00100010, 0b00100010, 0b00100010, 0b00100110, 0b00011010, 0b00000000,
	0b00000000, 0b00000000, 0b00100010, 0b00100010, 0b00100010, 0b00010100, 0b00001000, 0b00000000,
	0b00000000, 0b00000000, 0b00100010, 0b00100010, 0b00101010, 0b00101010, 0b00010100, 0b00000000,
	0b00000000, 0b00000000, 0b00100010, 0b00010100, 0b00001000, 0b00010100, 0b00100010, 0b00000000,
	0b00000000, 0b00000000, 0b00100010, 0b00100010, 0b00011110, 0b00000010, 0b00111100, 0b00000000,
	0b00000000, 0b00000000, 0b00111110, 0b00000100, 0b00001000, 0b00010000, 0b00111110, 0b00000000,
	0b00001000, 0b00000000, 0b00001000, 0b00011000, 0b00100000, 0b00100010, 0b00011100, 0b00000000,
	0b00100010, 0b00010100, 0b00001000, 0b00111110, 0b00001000, 0b00111110, 0b00001000, 0b00000000,
	0b00010100, 0b00010100, 0b00111110, 0b00010100, 0b00111110, 0b00010100, 0b00010100, 0b00000000,
	0b00011100, 0b00100010, 0b00100010, 0b00101100, 0b00100010, 0b00100010, 0b00101100, 0b00000000,
	0b00111000, 0b00000100, 0b00011000, 0b00000100, 0b00111000, 0b00000000, 0b00000000, 0b00000000,
	0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00001000, 0b00000100, 0b00011100, 0b00000000,
	0b00111110, 0b00010010, 0b00001000, 0b00000100, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
	0b00000000, 0b00000000, 0b00000000, 0b00000100, 0b00001000, 0b00010010, 0b00111110, 0b00000000,
	0b00000100, 0b00001010, 0b00001000, 0b00001000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
	0b00000000, 0b00000000, 0b00000000, 0b00001000, 0b00001000, 0b00101000, 0b00010000, 0b00000000,
	0b00000110, 0b00001000, 0b00001000, 0b00001000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
	0b00000000, 0b00000000, 0b00000000, 0b00001000, 0b00001000, 0b00001000, 0b00000110, 0b00000000,
	0b00110000, 0b00001000, 0b00001000, 0b00001000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
	0b00000000, 0b00000000, 0b00000000, 0b00001000, 0b00001000, 0b00001000, 0b00110000, 0b00000000,
	0b00000110, 0b00001000, 0b00001000, 0b00110000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
	0b00000000, 0b00000000, 0b00000000, 0b00110000, 0b00001000, 0b00001000, 0b00000110, 0b00000000,
	0b00001000, 0b00010100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
	0b00010100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
	0b00011000, 0b00000110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
	0b00001100, 0b00110000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
	0b00011010, 0b00100100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
	0b00011000, 0b00000100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
	0b00001100, 0b00010000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
	0b00011100, 0b00100010, 0b00011000, 0b00000100, 0b00011100, 0b00100100, 0b00011100, 0b00000000,
	0b00011100, 0b00100010, 0b00011000, 0b00100100, 0b00111100, 0b00100000, 0b00011100, 0b00000000,
	0b00011100, 0b00100010, 0b00000000, 0b00011000, 0b00001000, 0b00001000, 0b00011100, 0b00000000,
	0b00011100, 0b00100010, 0b00000000, 0b00011000, 0b00100100, 0b00100100, 0b00011000, 0b00000000,
	0b00011100, 0b00100010, 0b00000000, 0b00100010, 0b00100010, 0b00100110, 0b00011010, 0b00000000,
	0b00010100, 0b00000000, 0b00001000, 0b00010100, 0b00100010, 0b00111110, 0b00100010, 0b00000000,
	0b00010100, 0b00000000, 0b00011100, 0b00100010, 0b00100010, 0b00100010, 0b00011100, 0b00000000,
	0b00010100, 0b00000000, 0b00100010, 0b00100010, 0b00100010, 0b00100010, 0b00011100, 0b00000000,
	0b00010100, 0b00000000, 0b00011100, 0b00000010, 0b00011110, 0b00100010, 0b00011110, 0b00000000,
	0b00010100, 0b00000000, 0b00011100, 0b00100010, 0b00111110, 0b00100000, 0b00011100, 0b00000000,
	0b00010100, 0b00000000, 0b00011000, 0b00001000, 0b00001000, 0b00001000, 0b00011100, 0b00000000,
	0b00010100, 0b00000000, 0b00011000, 0b00100100, 0b00100100, 0b00100100, 0b00011000, 0b00000000,
	0b00010100, 0b00000000, 0b00100010, 0b00100010, 0b00100010, 0b00100110, 0b00011010, 0b00000000,
	0b00011000, 0b00000100, 0b00011100, 0b00000010, 0b00011110, 0b00100010, 0b00011110, 0b00000000,
	0b00011000, 0b00000100, 0b00011100, 0b00100010, 0b00111110, 0b00100000, 0b00011100, 0b00000000,
	0b00110000, 0b00001000, 0b00000000, 0b00011000, 0b00001000, 0b00001000, 0b00011100, 0b00000000,
	0b00110000, 0b00001000, 0b00000000, 0b00011000, 0b00100100, 0b00100100, 0b00011000, 0b00000000,
	0b00011000, 0b00000100, 0b00000000, 0b00100010, 0b00100010, 0b00100110, 0b00011010, 0b00000000,
	0b00001100, 0b00010000, 0b00111110, 0b00100000, 0b00111100, 0b00100000, 0b00111110, 0b00000000,
	0b00001100, 0b00010000, 0b00011100, 0b00000010, 0b00011110, 0b00100010, 0b00011110, 0b00000000,
	0b00001100, 0b00010000, 0b00011100, 0b00100010, 0b00111110, 0b00100000, 0b00011100, 0b00000000,
	0b00001100, 0b00010000, 0b00000000, 0b00011000, 0b00100100, 0b00100100, 0b00011000, 0b00000000,
	0b00011010, 0b00100100, 0b00000000, 0b00110010, 0b00101010, 0b00100110, 0b00100010, 0b00000000,
	0b00011010, 0b00100100, 0b00000000, 0b00101100, 0b00110010, 0b00100010, 0b00100010, 0b00000000,
	0b00001000, 0b00010100, 0b00010100, 0b00010100, 0b00011000, 0b00010010, 0b00101100, 0b00000000,
	0b00000000, 0b00100010, 0b00010100, 0b00001000, 0b00010100, 0b00100010, 0b00000000, 0b00000000,
	0b00000000, 0b00000000, 0b00000000, 0b00011000, 0b00011000, 0b00000000, 0b00000000, 0b00000000,
	0b00011000, 0b00011000, 0b00000000, 0b00111100, 0b00000000, 0b00011000, 0b00011000, 0b00000000,
	0b00000110, 0b00001000, 0b00001000, 0b00110000, 0b00001000, 0b00001000, 0b00000110, 0b00000000,
	0b00110000, 0b00001000, 0b00001000, 0b00000110, 0b00001000, 0b00001000, 0b00110000, 0b00000000,
	0b00011000, 0b00100100, 0b00100100, 0b00100100, 0b00011000, 0b00000000, 0b00000000, 0b00000000,
	0b00111110, 0b00100010, 0b00100010, 0b00010100, 0b00010100, 0b00010100, 0b00001000, 0b00000000,
	0b00000000, 0b00000000, 0b00010100, 0b00101010, 0b00101010, 0b00010100, 0b00000000, 0b00000000,
	0b00101010, 0b00101010, 0b00101010, 0b00101010, 0b00011100, 0b00001000, 0b00011100, 0b00000000,
	0b00011100, 0b00001000, 0b00011100, 0b00101010, 0b00011100, 0b00001000, 0b00011100, 0b00000000,
	0b00000000, 0b00001000, 0b00010000, 0b00111110, 0b00010000, 0b00001000, 0b00000000, 0b00000000,
	0b00011000, 0b00101000, 0b00001000, 0b00001000, 0b00001000, 0b00000000, 0b00000000, 0b00000000,
	0b00001000, 0b00010100, 0b00010100, 0b00010100, 0b00100010, 0b00100010, 0b00100010, 0b00000000,
	0b00001000, 0b00011000, 0b00101000, 0b00111100, 0b00001000, 0b00000000, 0b00000000, 0b00000000,
	0b00000000, 0b00001000, 0b00011100, 0b00101010, 0b00001000, 0b00001000, 0b00000000, 0b00000000,
	0b00111110, 0b00010100, 0b00010100, 0b00010100, 0b00010100, 0b00010100, 0b00110110, 0b00000000,
	0b00111000, 0b00100000, 0b00111000, 0b00000100, 0b00111000, 0b00000000, 0b00000000, 0b00000000,
	0b00011100, 0b00100010, 0b00100010, 0b00100010, 0b00010100, 0b00010100, 0b00110110, 0b00000000,
	0b00011000, 0b00000100, 0b00011110, 0b00100010, 0b00100010, 0b00100010, 0b00011100, 0b00000000,
	0b00000000, 0b00000000, 0b00010010, 0b00101010, 0b00100100, 0b00000000, 0b00000000, 0b00000000,
	0b00000000, 0b00001000, 0b00001000, 0b00101010, 0b00011100, 0b00001000, 0b00000000, 0b00000000,
	0b00011000, 0b00100000, 0b00111000, 0b00100100, 0b00011000, 0b00000000, 0b00000000, 0b00000000,
	0b00111110, 0b00010010, 0b00010010, 0b00010000, 0b00010000, 0b00010000, 0b00111000, 0b00000000,
	0b00011100, 0b00100010, 0b00100010, 0b00101010, 0b00100010, 0b00100010, 0b00011100, 0b00000000,
	0b00111110, 0b00010010, 0b00001000, 0b00000100, 0b00001000, 0b00010010, 0b00111110, 0b00000000,
	0b00000000, 0b00001000, 0b00000100, 0b00111110, 0b00000100, 0b00001000, 0b00000000, 0b00000000,
	0b00111110, 0b00100010, 0b00000000, 0b00011100, 0b00000000, 0b00100010, 0b00111110, 0b00000000,
	0b00000000, 0b00000000, 0b00011010, 0b00100100, 0b00100100, 0b00011010, 0b00000000, 0b00000000,
	0b00001000, 0b00010100, 0b00010100, 0b00010100, 0b00100010, 0b00100010, 0b00111110, 0b00000000,
	0b00000000, 0b00111110, 0b00000000, 0b00111110, 0b00000000, 0b00111110, 0b00000000, 0b00000000,
	0b00010100, 0b00101010, 0b00101010, 0b00001000, 0b00001000, 0b00001000, 0b00011100, 0b00000000,
	0b00000000, 0b00000000, 0b00011010, 0b00100100, 0b00000000, 0b00111110, 0b00000000, 0b00000000,
	0b00001000, 0b00001000, 0b00111110, 0b00001000, 0b00001000, 0b00001000, 0b00001000, 0b00000000,
	0b00000010, 0b00000100, 0b00111110, 0b00001000, 0b00111110, 0b00010000, 0b00100000, 0b00000000,
	0b00111100, 0b00000010, 0b00110010, 0b00101010, 0b00101010, 0b00110010, 0b00101010, 0b00000000,
	0b00000100, 0b00001010, 0b00001000, 0b00001000, 0b00001000, 0b00101000, 0b00010000, 0b00000000,
	0b00000110, 0b00000100, 0b00000100, 0b00000100, 0b00110100, 0b00001100, 0b00000100, 0b00000000,
	0b00111100, 0b00000100, 0b00001000, 0b00010000, 0b00010000, 0b00000000, 0b00000000, 0b00000000,
	0b00000000, 0b00000000, 0b00011010, 0b00100110, 0b00100100, 0b00100100, 0b00011010, 0b00000000,
	0b00011100, 0b00100010, 0b00101100, 0b00100010, 0b00110010, 0b00101100, 0b00100000, 0b00000000,
	0b00001000, 0b00001000, 0b00101010, 0b00101010, 0b00101010, 0b00011100, 0b00001000, 0b00000000,
	0b00001000, 0b00001000, 0b00011100, 0b00101010, 0b00101010, 0b00011100, 0b00001000, 0b00000000,
	0b00000000, 0b00000000, 0b00011100, 0b00100010, 0b00011000, 0b00100010, 0b00011100, 0b00000000,
	0b00011000, 0b00100100, 0b00011000, 0b00100100, 0b00011000, 0b00000000, 0b00000000, 0b00000000,
	0b00011000, 0b00001000, 0b00001000, 0b00001100, 0b00010100, 0b00010010, 0b00100010, 0b00000000,
	0b00000000, 0b00000000, 0b00101100, 0b00010010, 0b00010010, 0b00010010, 0b00000010, 0b00000000,
	0b00000000, 0b00000000, 0b00011000, 0b00001000, 0b00001000, 0b00001000, 0b00000110, 0b00000000,
	0b00000000, 0b00000000, 0b00111110, 0b00010100, 0b00010100, 0b00010100, 0b00010110, 0b00000000,
	0b00000000, 0b00000000, 0b00100110, 0b00101000, 0b00111000, 0b00100100, 0b00100010, 0b00000000,
	0b00000000, 0b00000000, 0b00010100, 0b00100010, 0b00101010, 0b00101010, 0b00010100, 0b00000000,
	0b00000000, 0b00000000, 0b00100100, 0b00100100, 0b00100100, 0b00111010, 0b00100000, 0b00000000,
	0b00000000, 0b00000000, 0b00110100, 0b00010010, 0b00010010, 0b00010100, 0b00011000, 0b00000000,
	0b00000000, 0b00000000, 0b00011100, 0b00100010, 0b00100010, 0b00100010, 0b00011100, 0b00000000,
	0b00000000, 0b00000000, 0b00011100, 0b00100010, 0b00100010, 0b00111100, 0b00100000, 0b00000000,
	0b00000000, 0b00000000, 0b00100010, 0b00010100, 0b00001100, 0b00001000, 0b00001000, 0b00000000,
	0b00011100, 0b00100010, 0b00100010, 0b00111110, 0b00100010, 0b00100010, 0b00011100, 0b00000000,
	0b00000000, 0b00000000, 0b00011110, 0b00100100, 0b00100100, 0b00100100, 0b00011000, 0b00000000,
	0b00000000, 0b00000000, 0b00011110, 0b00101000, 0b00001000, 0b00001000, 0b00000110, 0b00000000,
	0b00111110, 0b00100000, 0b00011100, 0b00100000, 0b00011100, 0b00000010, 0b00001100, 0b00000000,
	0b00011000, 0b00100100, 0b00011100, 0b00000100, 0b00011000, 0b00000000, 0b00000000, 0b00000000,
	0b00011100, 0b00100000, 0b00011100, 0b00100010, 0b00100010, 0b00100010, 0b00011100, 0b00000000,
	0b00000000, 0b00000000, 0b00100010, 0b00010100, 0b00001000, 0b00010100, 0b00100010, 0b00000000,
	0b00000000, 0b00000000, 0b00110010, 0b00010010, 0b00010010, 0b00010010, 0b00001100, 0b00000000,
	0b00111110, 0b00010000, 0b00100000, 0b00100000, 0b00011100, 0b00000010, 0b00001100, 0b00000000,
	0b00011100, 0b00100010, 0b00100010, 0b00011100, 0b00001000, 0b00111110, 0b00001000, 0b00000000,
	0b00001000, 0b00011100, 0b00101010, 0b00011100, 0b00100010, 0b00100010, 0b00011100, 0b00000000,
	0b00001110, 0b00110000, 0b00001110, 0b00000000, 0b00111110, 0b00000000, 0b00111110, 0b00000000,
	0b00111000, 0b00000110, 0b00111000, 0b00000000, 0b00111110, 0b00000000, 0b00111110, 0b00000000,
	0b00111100, 0b00000010, 0b00010010, 0b00101010, 0b00100010, 0b00101010, 0b00010010, 0b00000000,
	0b00011110, 0b00110100, 0b00110100, 0b00110100, 0b00010100, 0b00010100, 0b00010100, 0b00000000,
	0b00011000, 0b00100100, 0b00011000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
	0b00011100, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00010000, 0b00011100, 0b00000000,
	0b00011100, 0b00000100, 0b00000100, 0b00000100, 0b00000100, 0b00000100, 0b00011100, 0b00000000,
	0b00100010, 0b00100100, 0b00101000, 0b00010110, 0b00100010, 0b00000100, 0b00000110, 0b00000000,
	0b00101010, 0b00011100, 0b00011100, 0b00100010, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
	0b00111000, 0b00100000, 0b00100110, 0b00110100, 0b00100100, 0b00100100, 0b00100100, 0b00000000,
	0b00001100, 0b00010000, 0b00001000, 0b00010100, 0b00100010, 0b00111110, 0b00100010, 0b00000000,
	0b00001100, 0b00010000, 0b00000000, 0b00011100, 0b00001000, 0b00001000, 0b00011100, 0b00000000,
	0b00001100, 0b00010000, 0b00000000, 0b00011100, 0b00100010, 0b00100010, 0b00011100, 0b00000000,
	0b00001100, 0b00010000, 0b00000000, 0b00100010, 0b00100010, 0b00100010, 0b00011100, 0b00000000,
	0b00001100, 0b00010000, 0b00000000, 0b00011000, 0b00001000, 0b00001000, 0b00011100, 0b00000000,
	0b00001100, 0b00010000, 0b00000000, 0b00100010, 0b00100010, 0b00100110, 0b00011010, 0b00000000,
	0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
	0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
	0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
};
static const uint8_t lw30_font_bold[]{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2A, 0x14, 0x2A, 0x14, 0x2A, 0x14, 0x2A, 0x00, 0x22, 0x22, 0x2A, 0x36, 0x2A, 0x22, 0x22, 0x00,
	0x38, 0x38, 0x3A, 0x3E, 0x3E, 0x1E, 0x3E, 0x00, 0x38, 0x28, 0x2A, 0x2E, 0x22, 0x12, 0x3E, 0x00, 0x1E, 0x0E, 0x0E, 0x06, 0x06, 0x02, 0x02, 0x00, 0x00, 0x08, 0x10, 0x3A, 0x10, 0x08, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x3E, 0x00, 0x00, 0x12, 0x0A, 0x3E, 0x0A, 0x12, 0x00, 0x00, 0x3E, 0x00, 0x36, 0x3E, 0x2A, 0x00, 0x3E, 0x00, 0x30, 0x24, 0x2C, 0x3E, 0x2C, 0x24, 0x30, 0x00,
	0x3E, 0x14, 0x14, 0x14, 0x14, 0x14, 0x3E, 0x00, 0x3F, 0x15, 0x15, 0x15, 0x15, 0x15, 0x3F, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00,
	0x07, 0x0A, 0x11, 0x20, 0x11, 0x0A, 0x07, 0x00, 0x3C, 0x0C, 0x14, 0x24, 0x14, 0x0C, 0x3C, 0x00, 0x1C, 0x22, 0x08, 0x1C, 0x36, 0x3E, 0x36, 0x00, 0x0C, 0x12, 0x3E, 0x30, 0x3C, 0x30, 0x3E, 0x00,
	0x0C, 0x12, 0x00, 0x1E, 0x0C, 0x0C, 0x1E, 0x00, 0x1C, 0x22, 0x1C, 0x36, 0x36, 0x36, 0x1C, 0x00, 0x1C, 0x22, 0x00, 0x36, 0x36, 0x36, 0x1C, 0x00, 0x36, 0x00, 0x3E, 0x30, 0x3C, 0x30, 0x3E, 0x00,
	0x36, 0x00, 0x1E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00, 0x36, 0x00, 0x36, 0x36, 0x3C, 0x18, 0x18, 0x00, 0x36, 0x00, 0x36, 0x36, 0x1E, 0x06, 0x3C, 0x00, 0x18, 0x0C, 0x08, 0x1C, 0x36, 0x3E, 0x36, 0x00,
	0x30, 0x18, 0x3E, 0x30, 0x3C, 0x30, 0x3E, 0x00, 0x18, 0x0C, 0x00, 0x1E, 0x0C, 0x0C, 0x1E, 0x00, 0x18, 0x0C, 0x1C, 0x36, 0x36, 0x36, 0x1C, 0x00, 0x18, 0x0C, 0x00, 0x36, 0x36, 0x36, 0x1C, 0x00,
	0x1E, 0x30, 0x1C, 0x36, 0x1C, 0x06, 0x3C, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x00, 0x36, 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x30, 0x30, 0x1C, 0x18, 0x38, 0x00,
	0x08, 0x1E, 0x38, 0x1C, 0x0E, 0x3C, 0x08, 0x00, 0x38, 0x3A, 0x04, 0x08, 0x10, 0x2E, 0x0E, 0x00, 0x18, 0x34, 0x34, 0x18, 0x3A, 0x34, 0x1A, 0x00, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x18, 0x30, 0x00, 0x2A, 0x3E, 0x1C, 0x3E, 0x2A, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x1C, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x1C, 0x00, 0x06, 0x0C, 0x0C, 0x18, 0x18, 0x30, 0x30, 0x00,
	0x1C, 0x36, 0x26, 0x3E, 0x32, 0x36, 0x1C, 0x00, 0x0C, 0x1C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00, 0x1C, 0x36, 0x06, 0x0C, 0x18, 0x30, 0x3E, 0x00, 0x1C, 0x36, 0x06, 0x0C, 0x06, 0x36, 0x1C, 0x00,
	0x0C, 0x1C, 0x3C, 0x2C, 0x2C, 0x3E, 0x0C, 0x00, 0x3E, 0x30, 0x3C, 0x36, 0x06, 0x36, 0x1C, 0x00, 0x1C, 0x30, 0x30, 0x3C, 0x36, 0x36, 0x1C, 0x00, 0x3E, 0x06, 0x0C, 0x0C, 0x18, 0x18, 0x18, 0x00,
	0x1C, 0x36, 0x36, 0x1C, 0x36, 0x36, 0x1C, 0x00, 0x1C, 0x36, 0x36, 0x1E, 0x06, 0x06, 0x1C, 0x00, 0x00, 0x1C, 0x1C, 0x00, 0x00, 0x1C, 0x1C, 0x00, 0x00, 0x1C, 0x1C, 0x00, 0x1C, 0x0C, 0x18, 0x00,
	0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x3E, 0x3E, 0x00, 0x3E, 0x3E, 0x00, 0x00, 0x0C, 0x00, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x00, 0x1C, 0x36, 0x06, 0x1C, 0x18, 0x00, 0x18, 0x00,
	0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00, 0x08, 0x1C, 0x36, 0x36, 0x3E, 0x36, 0x36, 0x00, 0x3C, 0x36, 0x36, 0x3C, 0x36, 0x36, 0x3C, 0x00, 0x1C, 0x36, 0x30, 0x30, 0x30, 0x36, 0x1C, 0x00,
	0x3C, 0x36, 0x36, 0x36, 0x36, 0x36, 0x3C, 0x00, 0x3E, 0x30, 0x30, 0x3C, 0x30, 0x30, 0x3E, 0x00, 0x3E, 0x30, 0x30, 0x3C, 0x30, 0x30, 0x30, 0x00, 0x1C, 0x36, 0x30, 0x3E, 0x36, 0x36, 0x1C, 0x00,
	0x36, 0x36, 0x36, 0x3E, 0x36, 0x36, 0x36, 0x00, 0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00, 0x0E, 0x06, 0x06, 0x06, 0x36, 0x36, 0x1C, 0x00, 0x36, 0x36, 0x3C, 0x38, 0x3C, 0x36, 0x36, 0x00,
	0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3E, 0x00, 0x22, 0x36, 0x3E, 0x3E, 0x2A, 0x22, 0x22, 0x00, 0x22, 0x32, 0x3A, 0x3E, 0x2E, 0x26, 0x22, 0x00, 0x1C, 0x36, 0x36, 0x36, 0x36, 0x36, 0x1C, 0x00,
	0x3C, 0x36, 0x36, 0x3C, 0x30, 0x30, 0x30, 0x00, 0x1C, 0x36, 0x36, 0x36, 0x3E, 0x34, 0x1A, 0x00, 0x3C, 0x36, 0x36, 0x3C, 0x36, 0x36, 0x36, 0x00, 0x1C, 0x36, 0x30, 0x1C, 0x06, 0x36, 0x1C, 0x00,
	0x3E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x00, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x1C, 0x00, 0x36, 0x36, 0x36, 0x36, 0x36, 0x1C, 0x08, 0x00, 0x22, 0x22, 0x2A, 0x2A, 0x3E, 0x3E, 0x14, 0x00,
	0x36, 0x36, 0x1C, 0x08, 0x1C, 0x36, 0x36, 0x00, 0x36, 0x36, 0x36, 0x1C, 0x18, 0x18, 0x18, 0x00, 0x3E, 0x06, 0x0C, 0x18, 0x30, 0x30, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x3F,
	0x0C, 0x1A, 0x18, 0x3E, 0x18, 0x1A, 0x2E, 0x00, 0x18, 0x18, 0x3E, 0x18, 0x18, 0x00, 0x3E, 0x00, 0x1C, 0x36, 0x0C, 0x18, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F,
	0x30, 0x18, 0x0C, 0x06, 0x0C, 0x18, 0x30, 0x00, 0x00, 0x00, 0x1C, 0x06, 0x1E, 0x36, 0x1E, 0x00, 0x30, 0x30, 0x3C, 0x36, 0x36, 0x36, 0x3C, 0x00, 0x00, 0x00, 0x1E, 0x30, 0x30, 0x30, 0x1E, 0x00,
	0x06, 0x06, 0x1E, 0x36, 0x36, 0x36, 0x1E, 0x00, 0x00, 0x00, 0x1C, 0x36, 0x3E, 0x30, 0x1C, 0x00, 0x0E, 0x18, 0x3E, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x1E, 0x36, 0x1E, 0x06, 0x1C, 0x00,
	0x30, 0x30, 0x3C, 0x36, 0x36, 0x36, 0x36, 0x00, 0x1C, 0x00, 0x1C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00, 0x0E, 0x00, 0x0E, 0x06, 0x06, 0x36, 0x1C, 0x00, 0x30, 0x30, 0x36, 0x3C, 0x38, 0x3C, 0x36, 0x00,
	0x1C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00, 0x00, 0x00, 0x3C, 0x3E, 0x2A, 0x2A, 0x2A, 0x00, 0x00, 0x00, 0x3C, 0x36, 0x36, 0x36, 0x36, 0x00, 0x00, 0x00, 0x1C, 0x36, 0x36, 0x36, 0x1C, 0x00,
	0x00, 0x00, 0x3C, 0x36, 0x3C, 0x30, 0x30, 0x00, 0x00, 0x00, 0x1E, 0x36, 0x1E, 0x06, 0x06, 0x00, 0x00, 0x00, 0x36, 0x3E, 0x38, 0x30, 0x30, 0x00, 0x00, 0x00, 0x1E, 0x38, 0x1C, 0x0E, 0x3C, 0x00,
	0x18, 0x18, 0x3E, 0x18, 0x18, 0x18, 0x0E, 0x00, 0x00, 0x00, 0x36, 0x36, 0x36, 0x36, 0x1E, 0x00, 0x00, 0x00, 0x36, 0x36, 0x36, 0x1C, 0x08, 0x00, 0x00, 0x00, 0x22, 0x2A, 0x3E, 0x3E, 0x14, 0x00,
	0x00, 0x00, 0x36, 0x36, 0x1C, 0x36, 0x36, 0x00, 0x00, 0x00, 0x36, 0x36, 0x1E, 0x06, 0x3C, 0x00, 0x00, 0x00, 0x3E, 0x0C, 0x18, 0x30, 0x3E, 0x00, 0x0C, 0x00, 0x0C, 0x1C, 0x30, 0x36, 0x1C, 0x00,
	0x36, 0x36, 0x1C, 0x3E, 0x18, 0x3E, 0x18, 0x00, 0x14, 0x3E, 0x3E, 0x14, 0x3E, 0x3E, 0x14, 0x00, 0x1C, 0x26, 0x26, 0x2C, 0x26, 0x26, 0x2C, 0x00, 0x3C, 0x06, 0x1C, 0x06, 0x3C, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x0C, 0x06, 0x1E, 0x00, 0x3E, 0x32, 0x18, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x18, 0x32, 0x3E, 0x00, 0x04, 0x0E, 0x0A, 0x08, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x28, 0x38, 0x10, 0x00, 0x06, 0x0C, 0x0C, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x0C, 0x06, 0x00, 0x30, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x30, 0x00, 0x0E, 0x18, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x18, 0x18, 0x0E, 0x00, 0x1C, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1A, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x18, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1C, 0x22, 0x18, 0x0C, 0x1C, 0x2C, 0x1C, 0x00, 0x1C, 0x22, 0x18, 0x2C, 0x3C, 0x20, 0x1C, 0x00,
	0x0C, 0x12, 0x00, 0x1C, 0x0C, 0x0C, 0x1E, 0x00, 0x1C, 0x22, 0x00, 0x1C, 0x36, 0x36, 0x1C, 0x00, 0x1C, 0x22, 0x00, 0x36, 0x36, 0x36, 0x1E, 0x00, 0x36, 0x00, 0x08, 0x1C, 0x36, 0x3E, 0x36, 0x00,
	0x36, 0x00, 0x1C, 0x36, 0x36, 0x36, 0x1C, 0x00, 0x36, 0x00, 0x36, 0x36, 0x36, 0x36, 0x1C, 0x00, 0x36, 0x00, 0x1C, 0x06, 0x1E, 0x36, 0x1E, 0x00, 0x36, 0x00, 0x1C, 0x36, 0x3E, 0x30, 0x1C, 0x00,
	0x36, 0x00, 0x1C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00, 0x36, 0x00, 0x00, 0x1C, 0x36, 0x36, 0x1C, 0x00, 0x36, 0x00, 0x36, 0x36, 0x36, 0x36, 0x1E, 0x00, 0x18, 0x0C, 0x1C, 0x06, 0x1E, 0x36, 0x1E, 0x00,
	0x18, 0x0C, 0x1C, 0x36, 0x3E, 0x30, 0x1C, 0x00, 0x18, 0x0C, 0x00, 0x1C, 0x0C, 0x0C, 0x1E, 0x00, 0x18, 0x0C, 0x00, 0x1C, 0x36, 0x36, 0x1C, 0x00, 0x18, 0x0C, 0x00, 0x36, 0x36, 0x36, 0x1E, 0x00,
	0x0C, 0x18, 0x3E, 0x30, 0x3C, 0x30, 0x3E, 0x00, 0x0C, 0x18, 0x1C, 0x06, 0x1E, 0x26, 0x1E, 0x00, 0x0C, 0x18, 0x1C, 0x36, 0x3E, 0x30, 0x1C, 0x00, 0x0C, 0x18, 0x00, 0x1C, 0x36, 0x36, 0x1C, 0x00,
	0x1A, 0x2C, 0x00, 0x3A, 0x3E, 0x2E, 0x26, 0x00, 0x1A, 0x2C, 0x00, 0x3C, 0x36, 0x36, 0x36, 0x00, 0x1C, 0x36, 0x36, 0x36, 0x3C, 0x18, 0x2E, 0x00, 0x00, 0x36, 0x1C, 0x08, 0x1C, 0x36, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x1C, 0x1C, 0x00, 0x00, 0x00, 0x1C, 0x1C, 0x00, 0x3E, 0x00, 0x1C, 0x1C, 0x00, 0x0E, 0x18, 0x18, 0x30, 0x18, 0x18, 0x0E, 0x00, 0x38, 0x0C, 0x0C, 0x06, 0x0C, 0x0C, 0x38, 0x00,
	0x1C, 0x36, 0x36, 0x36, 0x1C, 0x00, 0x00, 0x00, 0x3E, 0x36, 0x36, 0x36, 0x1C, 0x1C, 0x08, 0x00, 0x00, 0x14, 0x3E, 0x2A, 0x3E, 0x14, 0x00, 0x00, 0x2A, 0x2A, 0x2A, 0x3E, 0x1C, 0x08, 0x1C, 0x00,
	0x1C, 0x08, 0x3E, 0x2A, 0x3E, 0x08, 0x1C, 0x00, 0x00, 0x0C, 0x18, 0x3E, 0x18, 0x0C, 0x00, 0x00, 0x1C, 0x2C, 0x0C, 0x0C, 0x0C, 0x00, 0x00, 0x00, 0x08, 0x1C, 0x1C, 0x36, 0x36, 0x36, 0x36, 0x00,
	0x0C, 0x1C, 0x2C, 0x3E, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x08, 0x1C, 0x3E, 0x2A, 0x08, 0x08, 0x00, 0x3E, 0x3E, 0x14, 0x14, 0x14, 0x36, 0x36, 0x00, 0x3C, 0x30, 0x3C, 0x06, 0x3C, 0x00, 0x00, 0x00,
	0x1C, 0x36, 0x36, 0x36, 0x14, 0x14, 0x36, 0x00, 0x18, 0x0C, 0x1E, 0x36, 0x36, 0x36, 0x1C, 0x00, 0x00, 0x00, 0x1A, 0x3E, 0x24, 0x00, 0x00, 0x00, 0x00, 0x08, 0x08, 0x2A, 0x3E, 0x1C, 0x08, 0x00,
	0x1C, 0x30, 0x3C, 0x36, 0x1C, 0x00, 0x00, 0x00, 0x3E, 0x1A, 0x1A, 0x18, 0x18, 0x18, 0x3C, 0x00, 0x1C, 0x36, 0x36, 0x3E, 0x36, 0x36, 0x1C, 0x00, 0x3E, 0x32, 0x18, 0x0C, 0x18, 0x32, 0x3E, 0x00,
	0x00, 0x18, 0x0C, 0x3E, 0x0C, 0x18, 0x00, 0x00, 0x3E, 0x36, 0x00, 0x1C, 0x00, 0x36, 0x3E, 0x00, 0x00, 0x00, 0x1A, 0x34, 0x34, 0x1A, 0x00, 0x00, 0x08, 0x1C, 0x1C, 0x36, 0x36, 0x36, 0x3E, 0x00,
	0x3E, 0x3E, 0x00, 0x3E, 0x00, 0x3E, 0x3E, 0x00, 0x14, 0x3E, 0x3E, 0x2A, 0x08, 0x08, 0x1C, 0x00, 0x00, 0x00, 0x1A, 0x2C, 0x00, 0x3E, 0x00, 0x00, 0x18, 0x18, 0x3C, 0x18, 0x18, 0x18, 0x18, 0x00,
	0x04, 0x3E, 0x3E, 0x08, 0x3E, 0x3E, 0x10, 0x00, 0x3C, 0x02, 0x32, 0x3A, 0x2A, 0x32, 0x2A, 0x00, 0x04, 0x0E, 0x0A, 0x08, 0x28, 0x38, 0x10, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x2C, 0x1C, 0x0C, 0x00,
	0x3E, 0x06, 0x0C, 0x18, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1A, 0x36, 0x34, 0x36, 0x1A, 0x00, 0x1C, 0x36, 0x3C, 0x36, 0x36, 0x3C, 0x30, 0x00, 0x08, 0x08, 0x2A, 0x2A, 0x3E, 0x1C, 0x08, 0x00,
	0x08, 0x1C, 0x3E, 0x2A, 0x3E, 0x1C, 0x08, 0x00, 0x00, 0x00, 0x1C, 0x32, 0x18, 0x32, 0x1C, 0x00, 0x1C, 0x36, 0x1C, 0x36, 0x1C, 0x00, 0x00, 0x00, 0x1C, 0x0C, 0x0C, 0x1C, 0x16, 0x36, 0x36, 0x00,
	0x00, 0x00, 0x3C, 0x36, 0x36, 0x06, 0x06, 0x00, 0x00, 0x00, 0x38, 0x18, 0x18, 0x18, 0x0E, 0x00, 0x00, 0x00, 0x3E, 0x3E, 0x14, 0x14, 0x16, 0x00, 0x00, 0x00, 0x36, 0x3C, 0x38, 0x3C, 0x36, 0x00,
	0x00, 0x00, 0x14, 0x22, 0x2A, 0x3E, 0x14, 0x00, 0x00, 0x00, 0x36, 0x36, 0x36, 0x3E, 0x30, 0x00, 0x00, 0x00, 0x3A, 0x1A, 0x1E, 0x1C, 0x18, 0x00, 0x00, 0x00, 0x1C, 0x36, 0x36, 0x36, 0x1C, 0x00,
	0x00, 0x00, 0x1C, 0x36, 0x36, 0x3C, 0x30, 0x00, 0x00, 0x00, 0x36, 0x34, 0x1C, 0x18, 0x18, 0x00, 0x18, 0x34, 0x34, 0x3C, 0x34, 0x34, 0x18, 0x00, 0x00, 0x00, 0x1E, 0x34, 0x34, 0x34, 0x18, 0x00,
	0x00, 0x00, 0x1E, 0x2C, 0x0C, 0x0C, 0x06, 0x00, 0x3E, 0x30, 0x1C, 0x30, 0x1C, 0x06, 0x1C, 0x00, 0x1C, 0x36, 0x1E, 0x06, 0x1C, 0x00, 0x00, 0x00, 0x1C, 0x30, 0x1C, 0x36, 0x36, 0x36, 0x1C, 0x00,
	0x00, 0x00, 0x36, 0x1C, 0x08, 0x1C, 0x36, 0x00, 0x00, 0x00, 0x3A, 0x1A, 0x1A, 0x1A, 0x0C, 0x00, 0x3E, 0x18, 0x30, 0x30, 0x1C, 0x06, 0x1C, 0x00, 0x1C, 0x36, 0x36, 0x1C, 0x08, 0x3E, 0x08, 0x00,
	0x08, 0x1C, 0x2A, 0x1C, 0x36, 0x36, 0x1C, 0x00, 0x1E, 0x38, 0x1E, 0x00, 0x3E, 0x00, 0x3E, 0x00, 0x3C, 0x0E, 0x3C, 0x00, 0x3E, 0x00, 0x3E, 0x00, 0x3C, 0x02, 0x1A, 0x32, 0x32, 0x1A, 0x02, 0x00,
	0x1E, 0x3E, 0x36, 0x36, 0x16, 0x16, 0x16, 0x00, 0x1C, 0x36, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E, 0x00, 0x3C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x3C, 0x00,
	0x32, 0x34, 0x38, 0x1E, 0x26, 0x0C, 0x0E, 0x00, 0x2A, 0x3E, 0x1C, 0x36, 0x00, 0x00, 0x00, 0x00, 0x3C, 0x30, 0x36, 0x3C, 0x34, 0x34, 0x34, 0x00, 0x0C, 0x18, 0x08, 0x1C, 0x36, 0x3E, 0x36, 0x00,
	0x06, 0x0C, 0x00, 0x1E, 0x0C, 0x0C, 0x1E, 0x00, 0x0C, 0x18, 0x1C, 0x36, 0x36, 0x36, 0x1C, 0x00, 0x0C, 0x18, 0x00, 0x36, 0x36, 0x36, 0x1C, 0x00, 0x0C, 0x18, 0x00, 0x1C, 0x0C, 0x0C, 0x1E, 0x00,
	0x0C, 0x18, 0x00, 0x36, 0x36, 0x36, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#pragma endregion

TIMER_DEVICE_CALLBACK_MEMBER(lw30_state::cursor_timer_callback)
{
	cursor_state = !cursor_state;
}

uint32_t lw30_state::screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect)
{
	// based on LW-350 ROM draw_char routine @ 6B14
	enum attrs : uint8_t {
		underline			= 0b00000001,
		overline			= 0b00000010,
		bold				= 0b00000100,
		vertical_line		= 0b00001000,
		invert_full			= 0b00010000,
		invert_upper_half	= 0b00100000,
		invert_lower_half	= 0b01000000
	};

	const rgb_t palette[]{
		0xffffffff,
		0xff000000,
	};

	enum control : uint8_t {
		display_on          = 0b00000001,
		half_height         = 0b00000010, // 64px height (LW-10/20) instead of 128px height (LW-30)
		bitmap_mode         = 0b00001000,
		tile_mode           = 0b00100000, // 8x8 tiles at videoram[0x1000]
	};

	if(video_control & display_on) {
		if(video_control & tile_mode) {
			uint8_t pixmap[60 * 128]; // pixel data
			for(auto y = 0; y < 16; y++) {
				for(auto x = 0; x < 60; x++) {
					auto atr = videoram[y * 256 + x * 2 + 0];
					auto chr = videoram[y * 256 + x * 2 + 1];
					auto fnt = &videoram[0x1000 + chr * 8 + ((atr & bold) ? 0x800 : 0)];
					uint8_t charbuf[8];
					for(int i = 0; i < 8; i++) {
						charbuf[i] = fnt[i];
					}
					if(atr & underline) {
						charbuf[7] = 0xff;
					}
					if(atr & vertical_line) {
						for(int i = 0; i < 8; i++) {
							charbuf[i] |= 0b1;
						}
					}

					for(int i = 0; i < 8; i++) {
						pixmap[(y * 8 + i) * 60 + x] = charbuf[i];
					}
				}
			}
			for(auto y = 0; y < 128; y++) {
				uint32_t* p = &bitmap.pix(y);
				for(auto x = 0; x < 480; x += 8) {
					auto gfx = pixmap[y * 60 + x / 8];
					*p++ = palette[BIT(gfx, 7)];
					*p++ = palette[BIT(gfx, 6)];
					*p++ = palette[BIT(gfx, 5)];
					*p++ = palette[BIT(gfx, 4)];
					*p++ = palette[BIT(gfx, 3)];
					*p++ = palette[BIT(gfx, 2)];
					*p++ = palette[BIT(gfx, 1)];
					*p++ = palette[BIT(gfx, 0)];
				}
			}
		} else if(video_control & bitmap_mode) {
			for(auto y = 0; y < 128; y++) {
				uint32_t* p = &bitmap.pix(y);
				for(auto x = 0; x < 480; x += 8) {
					auto gfx = videoram[y * 64 + x / 8];
					*p++ = palette[BIT(gfx, 7)];
					*p++ = palette[BIT(gfx, 6)];
					*p++ = palette[BIT(gfx, 5)];
					*p++ = palette[BIT(gfx, 4)];
					*p++ = palette[BIT(gfx, 3)];
					*p++ = palette[BIT(gfx, 2)];
					*p++ = palette[BIT(gfx, 1)];
					*p++ = palette[BIT(gfx, 0)];
				}
			}
		} else {
			// default font
			uint8_t pixmap[80 * 128]{}; // pixel data
			for(auto y = 0; y < 14; y++) {
				for(auto x = 0; x < 80; x++) {
					auto atr = videoram[y * 256 + x * 2 + 0];
					auto chr = videoram[y * 256 + x * 2 + 1];
					auto fnt = (atr & bold) ? &lw30_font_bold[chr * 8] : &lw30_font_normal[chr * 8];
					uint8_t charbuf[9];
					charbuf[0] = 0x00;
					for(int i = 0; i < 8; i++) {
						charbuf[i + 1] = fnt[i];
					}

					if(atr & underline) {
						charbuf[8] = 0xff;
					}
					if(atr & overline) {
						charbuf[0] = 0xff;
					}
					if(atr & vertical_line) {
						for(int i = 0; i < 9; i++) {
							charbuf[i] |= 0b1;
						}
					}
					if(atr & invert_full) {
						for(int i = 0; i < 9; i++) {
							charbuf[i] ^= 0xff;
						}
					}
					if(atr & invert_lower_half) {
						for(int i = 4; i < 9; i++) {
							charbuf[i] ^= 0xff;
						}
					}
					if(atr & invert_upper_half) {
						for(int i = 0; i < 5; i++) {
							charbuf[i] ^= 0xff;
						}
					}

					for(int i = 0; i < 9; i++) {
						pixmap[(y * 9 + i) * 80 + x] = charbuf[i];
					}
				}
			}

			// draw cursor; TODO: shape
			if(cursor_state) {
				auto cursor_x = video_cursor_x & 0x7f;
				auto cursor_y = (video_cursor_x >> 7) | ((video_cursor_y & 7) << 1);
				if(cursor_x < 80 && cursor_y < 14) {
					for(int i = 0; i < 9; i++) {
						pixmap[(cursor_y * 9 + i) * 80 + cursor_x] ^= 0xff;
					}
				}
			}
			for(auto y = 0; y < 128; y++) {
				uint32_t* p = &bitmap.pix(y);
				for(auto x = 0; x < 640; x += 8) {
					auto gfx = pixmap[y * 80 + x / 8];
					//*p++ = palette[BIT(gfx, 7)];
					//*p++ = palette[BIT(gfx, 6)];
					*p++ = palette[BIT(gfx, 5)];
					*p++ = palette[BIT(gfx, 4)];
					*p++ = palette[BIT(gfx, 3)];
					*p++ = palette[BIT(gfx, 2)];
					*p++ = palette[BIT(gfx, 1)];
					*p++ = palette[BIT(gfx, 0)];
				}
			}
		}
	} else { 
		// display off
		for(auto y = 0; y < 128; y++) {
			uint32_t* p = &bitmap.pix(y);
			for(auto x = 0; x < 480; x++) {
				*p++ = palette[0];
			}
		}
	}

	return 0;
}

TIMER_DEVICE_CALLBACK_MEMBER(lw30_state::int1_timer_callback)
{
	maincpu->set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
}


// Keyboard
//////////////////////////////////////////////////////////////////////////

void lw30_state::machine_start()
{
	screen->set_visible_area(0, 480 - 1, 0, 128 - 1);

	// try to load map file
	FILE* f;
	if(fopen_s(&f, "lw30.map", "rt") == 0) {
		char line[512];
		do {
			if(fgets(line, sizeof(line), f)) {
				int segment, offset;
				char symbol[512];
				if(sscanf(line, "%x:%x %512s", &segment, &offset, symbol) == 3) {
					uint32_t phys = (segment << 4) + offset;
					//TRACE(_T("%04x:%04x => %02x:%04x\n"), segment, offset, bank, offset);
					symbols[phys] = symbol;
				}
			}
		} while(!feof(f));
		fclose(f);
	}

	rom = memregion("maincpu")->base();

	// patch out printer init
	rom[0x280f4] = 0x00;
	// patch out autosave load
	rom[0x28c3a] = rom[0x28c3a + 1] = rom[0x28c3a + 2] = 0x00;
}

void lw30_state::machine_reset()
{
	cursor_state = 0;
	video_cursor_x = video_cursor_y = 0;
	video_pos_x = video_pos_y = 0;
	video_control = 0;

	memcpy(&videoram[0x1000], lw30_font_normal, sizeof(lw30_font_normal));
}

#pragma region(LW-350 Keyboard)

static INPUT_PORTS_START(lw30)
	PORT_START("kbrow.0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_4)      PORT_CHAR('4')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_3)      PORT_CHAR('3')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_W)      PORT_CHAR('w')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_E)      PORT_CHAR('e')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_D)      PORT_CHAR('d')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_X)      PORT_CHAR('x')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_TAB)    PORT_CHAR(UCHAR_MAMEKEY(TAB))

	PORT_START("kbrow.1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_5)      PORT_CHAR('5')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_6)      PORT_CHAR('6')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_R)      PORT_CHAR('r')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_T)      PORT_CHAR('t')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_C)      PORT_CHAR('c')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_F)      PORT_CHAR('f')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_UP)                 PORT_CODE(KEYCODE_UP)     PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("kbrow.2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_8)      PORT_CHAR('8')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_7)      PORT_CHAR('7')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_Z)      PORT_CHAR('z')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_H)      PORT_CHAR('h')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_G)      PORT_CHAR('g')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_V)      PORT_CHAR('v')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("G.S.END")               PORT_CODE(KEYCODE_END)    PORT_CHAR(UCHAR_MAMEKEY(END))

	PORT_START("kbrow.3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_1)          PORT_CHAR('1')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_2)          PORT_CHAR('2')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_Q)          PORT_CHAR('q')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_Y)          PORT_CHAR('y')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_A)          PORT_CHAR('a')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_S)          PORT_CHAR('s')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("kbrow.4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_9)          PORT_CHAR('9')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_J)          PORT_CHAR('j')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_I)          PORT_CHAR('i')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_U)          PORT_CHAR('u')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_B)          PORT_CHAR('b')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_N)          PORT_CHAR('n')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("kbrow.5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_MINUS)      PORT_CHAR(0x00df) PORT_CHAR('?') // ß
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_0)          PORT_CHAR('0')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_P)          PORT_CHAR('p')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_O)          PORT_CHAR('o')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_M)          PORT_CHAR('m')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR(';')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_MENU)       PORT_CHAR(UCHAR_MAMEKEY(MENU))

	PORT_START("kbrow.6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Inhalt")                PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_COLON)      PORT_CHAR(0x00f6) PORT_CHAR(0x00d6) // ö Ö
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('+') PORT_CHAR('*')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR(0x00fc) PORT_CHAR(0x00dc) // ü Ü
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("kbrow.7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SM/Layout")             PORT_CODE(KEYCODE_PRTSCR)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("STORNO")                PORT_CODE(KEYCODE_PAUSE)      PORT_CHAR(UCHAR_MAMEKEY(CANCEL))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(UCHAR_MAMEKEY(ENTER))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(UCHAR_MAMEKEY(BACKSPACE))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    //PORT_CODE(KEYCODE_)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Horz/Vert")             //PORT_CODE(KEYCODE_)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(UCHAR_MAMEKEY(SPACE))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_MAMEKEY(LSHIFT))

	PORT_START("kbrow.8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(0x00b4) PORT_CHAR(0x02cb) // ´ `
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_L)          PORT_CHAR('l')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_TILDE)      PORT_CHAR('\'')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_K)          PORT_CHAR('k')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR(':')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)                                    PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(0x00e4) PORT_CHAR(0x00c4) // ä Ä
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

#pragma endregion

void lw30_state::lw30(machine_config& config) {
	// basic machine hardware
	HD64180RP(config, maincpu, 12'000'000 / 2);
	maincpu->set_addrmap(AS_PROGRAM, &lw30_state::map_program);
	maincpu->set_addrmap(AS_IO, &lw30_state::map_io);
	TIMER(config, "1khz").configure_periodic(FUNC(lw30_state::int1_timer_callback), attotime::from_hz(1000));

	// video hardware
	SCREEN(config, screen, SCREEN_TYPE_RASTER);
	screen->set_color(rgb_t(6, 245, 206));
	screen->set_physical_aspect(480, 128);
	screen->set_refresh_hz(78.1);
	screen->set_screen_update(FUNC(lw30_state::screen_update));
	screen->set_size(480, 128);

	TIMER(config, "cursor").configure_periodic(FUNC(lw30_state::cursor_timer_callback), attotime::from_msec(512));

	// floppy disk
	LW30_FLOPPY_CONNECTOR(config, floppy, 0);
	floppy->option_add("35dd", LW30_FLOPPY);
	floppy->set_default_option("35dd");

	// sound hardware
	SPEAKER(config, "mono").front_center();
	BROTHER_BEEP(config, beeper, 4'000).add_route(ALL_OUTPUTS, "mono", 1.0); // 4.0 kHz
}

#pragma endregion

/***************************************************************************
  Machine driver(s)
***************************************************************************/

ROM_START( lw30 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD("ua5362-a", 0x00000, 0x40000, CRC(DAC77867) SHA1(5c7ab30dec55a24eb1b7f241e5015e3836ebf077))
//	ROM_LOAD("patched", 0x00000, 0x40000, CRC(DAC77867) SHA1(5c7ab30dec55a24eb1b7f241e5015e3836ebf077))
	ROM_REGION(0x80000, "dictionary", 0)
	ROM_LOAD("ua2849-a", 0x00000, 0x80000, CRC(FA8712EB) SHA1(2d3454138c79e75604b30229c05ed8fb8e7d15fe))
ROM_END

//    YEAR  NAME  PARENT COMPAT   MACHINE INPUT  CLASS           INIT     COMPANY         FULLNAME          FLAGS
COMP( 1991, lw30,   0,   0,       lw30,   lw30,  lw30_state,     empty_init,       "Brother",      "Brother LW-30",  MACHINE_NODEVICE_PRINTER )
