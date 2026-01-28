// license:BSD-3-Clause
// copyright-holders:

#include "emu.h"
#include "cpu/mcs51/i80c52.h"
#include "bus/midi/midi.h"
#include "sound/sam8905.h"
#include "speaker.h"

#define LOG_SAM    (1U << 1)
//#define LOG_SAM    0
#define VERBOSE (LOG_SAM)
#include "logmacro.h"

// Select which ROM set to use (uncomment one):
#define MS4_ROM_SET_XE9L      // XE9 left section (xe9l_v141_uart.bin + xel*.bin) --- Drums on CH13
//#define MS4_ROM_SET_XE9R      // XE9 right section (xe9r_v141_uart.bin + xer*.bin) -- LEAD SQAURE, TANGO ACCORDION DISTORTED
//#define MS4_ROM_SET_MS4_05    // MS4 solo sounds (ms4_05_r1_0.bin)
//#define MS4_ROM_SET_MS4_06    // MS4 drums+accomp (ms4_06_r1_1.bin)

namespace {

class ms4_state : public driver_device
{
public:
	ms4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_sam(*this, "sam")
		, m_midi_out(*this, "mdout")
		, m_samples(*this, "samples")
	{ }

	void ms4(machine_config &config);

private:
	void program_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;

	void port2_w(u8 data);
	u8 port3_r();
	void port3_w(u8 data);

	u8 sam_r(offs_t offset);
	void sam_w(offs_t offset, u8 data);

	u16 sam_waveform_r(offs_t offset);
	void sam_waveform_w(offs_t offset, u16 data);
	void sam_wcs_w(u32 data);  // /WCS callback - latches PHI for 74HC174

	void midi_rxd_w(int state) { m_midi_rxd = state; }

	virtual void machine_start() override ATTR_COLD;

	required_device<i80c32_device> m_maincpu;
	required_device<sam8905_device> m_sam;
	required_device<midi_port_device> m_midi_out;
	optional_region_ptr<u8> m_samples;  // Waveform samples (3MB for XE9)

	// SAM SRAM - 16KB (2× 8KB HY6264AJ)
	// WA1-WA13 → A0-A12 (13 bits = 8KB), WA16 → chip select
	std::unique_ptr<int16_t[]> m_sam_sram;
	static constexpr size_t SAM_SRAM_SIZE = 0x4000;  // 16KB (2 × 8KB)

	u8 m_port3 = 0xff;
	u8 m_midi_rxd = 1;

	// 74HC174 address latch state (XE9 external ROM)
	// Latches PHI[2:10] when /WCS goes low (WWF selects external waveform)
	uint32_t m_phi_latched = 0;  // PHI value at time of WWF
	uint32_t m_wf_latched = 0;   // WF value for bank select
	bool m_wcs_active = false;   // /WCS is low (external waveform selected)
};

void ms4_state::program_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region("program", 0);
}

void ms4_state::data_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	// SAM8905 decodes only A0-A2, chip select on A15
	map(0x8000, 0xffff).rw(FUNC(ms4_state::sam_r), FUNC(ms4_state::sam_w));
}

void ms4_state::port2_w(u8 data)
{
}

u8 ms4_state::port3_r()
{
	// P3.0 = RXD - MIDI input
	u8 data = (m_port3 & 0xfe) | (m_midi_rxd & 0x01);
	return data;
}

void ms4_state::port3_w(u8 data)
{
	// P3.1 = TXD output to MIDI
	if ((m_port3 ^ data) & 0x02)
		m_midi_out->write_txd(BIT(data, 1));

	m_port3 = data;
}

u8 ms4_state::sam_r(offs_t offset)
{
	u8 reg = offset & 0x07;
	if (offset > 0x07) {
		LOGMASKED(LOG_SAM, "INVALID SAM read [%d] (addr=%04X) PC=%04X\n",
			reg, 0x8000 + offset, m_maincpu->pc());
	}
	u8 data = m_sam->read(reg);
	LOGMASKED(LOG_SAM, "SAM read [%d] = 0x%02X (addr=%04X) PC=%04X\n",
		reg, data, 0x8000 + offset, m_maincpu->pc());
	return data;
}

void ms4_state::sam_w(offs_t offset, u8 data)
{
	u8 reg = offset & 0x07;
	if (offset > 0x07) {
		LOGMASKED(LOG_SAM, "INVALID SAM write [%d] = 0x%02X (addr=%04X) PC=%04X\n",
			reg, data, 0x8000 + offset, m_maincpu->pc());
	}
	m_sam->write(reg, data);
	LOGMASKED(LOG_SAM, "SAM write [%d] = 0x%02X (addr=%04X) PC=%04X\n",
		reg, data, 0x8000 + offset, m_maincpu->pc());
}

void ms4_state::sam_wcs_w(u32 data)
{
	// /WCS callback from SAM8905 - fires when WWF executes
	// data = (wf << 12) | phi
	uint32_t wf = (data >> 12) & 0x1FF;
	uint32_t phi = data & 0xFFF;
	bool external = !(wf & 0x100);  // WF[8]=0 means external waveform

	if (external && !m_wcs_active) {
		// /WCS going low - latch current PHI for 74HC174
		// Latched bits: PHI[2:10] → RA9-RA17
		m_phi_latched = phi;
		m_wf_latched = wf;
		m_wcs_active = true;
		// LOGMASKED(LOG_SAM, "WCS latch: WF=0x%03X PHI=0x%03X (latched PHI[2:10]=0x%03X)\n",
		// 	wf, phi, (phi >> 2) & 0x1FF);
	} else if (!external) {
		// /WCS going high - internal waveform selected
		m_wcs_active = false;
	}
}

u16 ms4_state::sam_waveform_r(offs_t offset)
{
	// XE9 external memory read
	// SAM8905 outputs: WA[19:0] = { WAVE[7:0], PHI[11:0] }
	//
	// Memory select via WA16 (= WAVE[4]):
	//   WA16=0: ROM read (6x 512KB waveform ROMs)
	//   WA16=1: SRAM read (2x 8KB delay buffer)
	//
	// SRAM chip select: CS1=/WCS (low), CS2=WA16 (high)
	// SRAM address: WA1-WA13 → A0-A12 (8K words)

	uint32_t wa16 = (offset >> 16) & 1;  // WAVE[4] - memory select

	if (wa16) {
		// SRAM read - WA1-WA13 → A0-A12 (13 bits = 8K addresses)
		uint32_t sram_addr = (offset >> 1) & 0x1FFF;
		if (sram_addr < SAM_SRAM_SIZE)
			return m_sam_sram[sram_addr];
		return 0;
	}

	// ROM read - 74HC174 address latching for waveform ROMs
	// Hardware: 6x 512KB ROMs organized as 4 banks of 12-bit samples
	//   ROMs 0-3 (XEL0-XEL3): upper 8 bits (WD4-WD11)
	//   ROMs 4-5 (XEL4-XEL5): lower nibble (WD0-WD3)
	//
	// 74HC174 latch captures WA[2:10] on /WCS falling edge (WWF with external WF)
	// ROM address generation:
	//   RA[8:0]  = WA[11:3] = current PHI[11:3]  (9 bits, direct - 512 samples/period)
	//   RA[17:9] = latched WA[10:2] = latched PHI[10:2]  (9 bits, from 74HC174)
	//   Bank select: WA[19:18] via 74LS139 → /CS0-/CS3 (4 banks)

	if (!m_samples)
		return 0;  // No sample ROMs loaded

	// Extract current PHI from offset (WA[11:0] = PHI[11:0])
	uint32_t current_phi = offset & 0xFFF;

	// RA[8:0] = current PHI[11:3] (direct path, 9 bits = 512 samples)
	uint32_t ra_low = (current_phi >> 3) & 0x1FF;

	// RA[17:9] = latched PHI[10:2] (from 74HC174, 9 bits = waveform page)
	uint32_t ra_high = (m_phi_latched >> 2) & 0x1FF;

	uint32_t wa17 = (offset >> 17) & 1;

	uint32_t ra18 = wa17 << 18;

	// Build 18-bit ROM address: RA[18:0]
	uint32_t rom_addr = ra18 | (ra_high << 9) | ra_low;

	// Bank select from WA[19:18] via 74LS139
	// WA19 selects ROM pair: 0→(ROM0,ROM1)+ROM4, 1→(ROM2,ROM3)+ROM5
	// WA18 selects within pair: adds 0 or 1 to base ROM
	uint32_t wa19 = (offset >> 19) & 1;  // WAVE[7]
	uint32_t wa18 = (offset >> 18) & 1;  // WAVE[6]
	uint32_t wa1 = (current_phi >> 1) & 1;  // PHI[1] - nibble select via 74LS257

	// Upper byte ROM: 0-3
	// 74LS139 decoding: WA19=0→ROM0/1, WA19=1→ROM2/3, WA18 selects within pair
	uint32_t upper_rom = (wa19 << 1) | wa18;

	// Read upper 8 bits from ROMs 0-3
	// ROM layout: ROM0=0x000000, ROM1=0x080000, ROM2=0x100000, ROM3=0x180000
	uint32_t upper_addr = (upper_rom * 0x80000) + (rom_addr & 0x7FFFF);
	if (upper_addr >= 0x200000) {
		printf("ERR 1\n");
		exit(1);
		return 0;
	}
	uint8_t upper_byte = m_samples[upper_addr];

	// Read lower nibble from ROMs 4-5
	// WA19 selects nibble ROM: 0→ROM4 (0x200000), 1→ROM5 (0x280000)
	//uint32_t nibble_rom_offset = 0x200000 + (wa19 * 0x80000);
	//
	// WA19 selects nibble ROM: 1→ROM4 (0x200000), 0→ROM5 (0x280000)
	uint32_t nibble_rom_offset = 0x200000 + (wa19 == 0 ? 0x80000 : 0x00000);
	uint32_t nibble_addr = nibble_rom_offset + (rom_addr & 0x7FFFF);
	if (nibble_addr >= 0x300000) {
		printf("ERR 2\n");
		exit(1);
		return 0;
	}
	uint8_t nibble_byte = m_samples[nibble_addr];

	// WA1 (PHI[1]) selects low/high nibble via 74LS257 MUX
	uint8_t lower_nibble = wa1 ? (nibble_byte >> 4) : (nibble_byte & 0x0F);

	// Combine to 12-bit sample: upper 8 bits + lower 4 bits
	int16_t sample = ((int8_t)upper_byte << 4) | lower_nibble;

	return sample;
}

void ms4_state::sam_waveform_w(offs_t offset, u16 data)
{
	// SAM external memory write
	// Address from SAM: WA[19:0] = (WF[7:0] << 12) | PHI[11:0]
	//
	// WA16 (= WF[4]) selects SRAM via chip select logic:
	//   WA16=0: ROM (read-only, write ignored)
	//   WA16=1: SRAM write enabled
	//
	// SRAM address: WA1-WA13 → A0-A12 (13 bits = 8K addresses)

	uint32_t wa16 = (offset >> 16) & 1;  // WF[4] - SRAM chip select

	if (!wa16)
		return;  // WA16=0 means ROM selected, ignore write

	// SRAM write - WA1-WA13 → A0-A12
	uint32_t sram_addr = (offset >> 1) & 0x1FFF;

	if (sram_addr < SAM_SRAM_SIZE)
		m_sam_sram[sram_addr] = (int16_t)data;
}

void ms4_state::machine_start()
{
	// Allocate SAM SRAM
	m_sam_sram = std::make_unique<int16_t[]>(SAM_SRAM_SIZE);
	std::fill_n(m_sam_sram.get(), SAM_SRAM_SIZE, 0);

	save_item(NAME(m_port3));
	save_item(NAME(m_midi_rxd));
	save_pointer(NAME(m_sam_sram), SAM_SRAM_SIZE);
	save_item(NAME(m_phi_latched));
	save_item(NAME(m_wf_latched));
	save_item(NAME(m_wcs_active));
}

static INPUT_PORTS_START(ms4)
INPUT_PORTS_END

void ms4_state::ms4(machine_config &config)
{
	I80C32(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &ms4_state::program_map);
	m_maincpu->set_addrmap(AS_DATA, &ms4_state::data_map);
	m_maincpu->port_out_cb<2>().set(FUNC(ms4_state::port2_w));
	m_maincpu->port_in_cb<3>().set(FUNC(ms4_state::port3_r));
	m_maincpu->port_out_cb<3>().set(FUNC(ms4_state::port3_w));

	// MIDI
	midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	mdin.rxd_handler().set(FUNC(ms4_state::midi_rxd_w));
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	// Sound - single SAM8905
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	SAM8905(config, m_sam, 45'158'400, 1024);
	m_sam->waveform_read_callback().set(FUNC(ms4_state::sam_waveform_r));
	m_sam->waveform_write_callback().set(FUNC(ms4_state::sam_waveform_w));
	m_sam->wcs_callback().set(FUNC(ms4_state::sam_wcs_w));
	m_sam->add_route(0, "lspeaker", 1.0);
	m_sam->add_route(1, "rspeaker", 1.0);
}

ROM_START(ms4)
	ROM_REGION(0x10000, "program", 0)
#if defined(MS4_ROM_SET_XE9L)
	ROM_LOAD("xe9l_v141_uart.bin", 0x0000, 0x10000, CRC(9a1f650b) SHA1(60ad0479565f947500eee576caa54e96dd5a0144))
#elif defined(MS4_ROM_SET_XE9R)
	ROM_LOAD("xe9r_v141_uart.bin", 0x0000, 0x10000, CRC(cdef42db) SHA1(8e5712b0fe32385e7eb04be5d7b51c9d8c5467f0))
#elif defined(MS4_ROM_SET_MS4_05)
	ROM_LOAD("ms4_05_r1_0.bin", 0x0000, 0x10000, CRC(b02cd104) SHA1(1c10d26481c20d0de2df9cf3e2b5112cd40fe3ba))
#elif defined(MS4_ROM_SET_MS4_06)
	ROM_LOAD("ms4_06_r1_1.bin", 0x0000, 0x10000, CRC(b02cd104) SHA1(1c10d26481c20d0de2df9cf3e2b5112cd40fe3ba))
#else
	#error "No ROM set defined! Define one of: MS4_ROM_SET_XE9L, MS4_ROM_SET_XE9R, MS4_ROM_SET_MS4_05, MS4_ROM_SET_MS4_06"
#endif

#if defined(MS4_ROM_SET_XE9L)
	// XE9 left section waveform ROMs - 6 × 512KB = 3MB
	ROM_REGION(0x300000, "samples", 0)
	ROM_LOAD("xel01.bin", 0x000000, 0x80000, CRC(c0321237) SHA1(f777ab48444e88bf0186dfd7a9c3e2d60cf84699))
	ROM_LOAD("xel11.bin", 0x080000, 0x80000, CRC(5adeb7ab) SHA1(8250a9634aa26018cd96622a07e21ad0eaf43e9c))
	ROM_LOAD("xel21.bin", 0x100000, 0x80000, CRC(472066d6) SHA1(1057536f2e81d25e177aed01e73ac38e61e7d766))
	ROM_LOAD("xel31.bin", 0x180000, 0x80000, CRC(8e4c2856) SHA1(896f6ee813db6e65c8fcdaf0c514f3ab31aad4c2))
	ROM_LOAD("xel41.bin", 0x200000, 0x80000, CRC(beda3578) SHA1(0e761ed0b241545354876b74a86dadf13a60e65a))
	ROM_LOAD("xel51.bin", 0x280000, 0x80000, CRC(48cbc0ec) SHA1(558748a43e1227bf814e68bc3515a845af605f4e))
#elif defined(MS4_ROM_SET_XE9R)
	// XE9 right section waveform ROMs - 6 × 512KB = 3MB
	ROM_REGION(0x300000, "samples", 0)
	ROM_LOAD("xer01.bin", 0x000000, 0x80000, CRC(444e0c20) SHA1(2aaf4a5af1a84792a497eb72b757c8779039c058))
	ROM_LOAD("xer11.bin", 0x080000, 0x80000, CRC(c24fac2e) SHA1(45f0159fa827d78a13dbcfd2691aa3bf0f97ad36))
	ROM_LOAD("xer21.bin", 0x100000, 0x80000, CRC(6177a3f0) SHA1(d08e38185d7189f594f45db11dc772160ddb2582))
	ROM_LOAD("xer31.bin", 0x180000, 0x80000, CRC(11b3f323) SHA1(9c184bfa8d6e56f8ac2be100be90f668af693e15))
	ROM_LOAD("xer41.bin", 0x200000, 0x80000, CRC(64bc279e) SHA1(39d2177cc90396516efb1296adf6d87bf30a0204))
	ROM_LOAD("xer51.bin", 0x280000, 0x80000, CRC(28199b73) SHA1(c475c9696ab20f2b2a6d81ccf3bcbedd0c4b18da))
#endif
	// MS4 sample ROMs not yet dumped/analyzed
ROM_END

} // anonymous namespace

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY   FULLNAME      FLAGS
SYST( 1992, ms4,  0,      0,      ms4,     ms4,   ms4_state, empty_init, "Solton", "Solton MS4", MACHINE_IMPERFECT_SOUND )
