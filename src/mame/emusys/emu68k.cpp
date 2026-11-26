// license:BSD-3-Clause
// copyright-holders:AJR, Grant McNaught
/***********************************************************************************************************************************

    Skeleton driver for E-mu Proteus and other MC68000-based synthesizer modules.

***********************************************************************************************************************************/

#include "emu.h"
#include "proteus_snd.h"

#include "bus/midi/midi.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/mc68901.h"
#include "machine/nvram.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class emu68k_state : public driver_device
{
public:
	emu68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mfp(*this, "mfp")
		, m_snd(*this, "snd")
		, m_jacks(*this, "JACKS")
		, m_volume(*this, "VOLUME")
	{
	}

	void proteus1(machine_config &config);
	void vintkeys(machine_config &config);
	void phatt(machine_config &config);
	void proteusxr(machine_config &config);
	void proteusxrpl(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void add_lcd(machine_config &config);

	HD44780_PIXEL_UPDATE(lcd_pixel_update);
	void palette_init(palette_device &palette);

	void proteus1_map(address_map &map) ATTR_COLD;
	void proteusxr_map(address_map &map) ATTR_COLD;
	void vintkeys_map(address_map &map) ATTR_COLD;
	void phatt_map(address_map &map) ATTR_COLD;
	void fc7_map(address_map &map) ATTR_COLD;
	void proteusxr_waves_map(address_map &map) ATTR_COLD;
	void proteusxrpl_waves_map(address_map &map) ATTR_COLD;

	u8 jacks_r();
	u16 volume_strobe_r();
	TIMER_CALLBACK_MEMBER(volume_pulse_end);

	required_device<m68000_base_device> m_maincpu;
	required_device<mc68901_device> m_mfp;
	optional_device<proteus_snd_device> m_snd;
	optional_ioport m_jacks;
	optional_ioport m_volume;
	emu_timer *m_volume_pulse = nullptr;
};

void emu68k_state::machine_start()
{
	m_volume_pulse = timer_alloc(FUNC(emu68k_state::volume_pulse_end), this);
}

// Reading 0xa00000 fires a one-shot whose width follows the front-panel volume pot;
// MFP timer B measures it in pulse-width mode (TBCR = 0x0c, /50 prescale) and the
// firmware scales the count to 0-127 against a self-calibrating min/max.
// Pulse polarity (active high, AER bit 3 set) and width range are inferred.
u16 emu68k_state::volume_strobe_r()
{
	if (!machine().side_effects_disabled())
	{
		const int pot = m_volume.read_safe(100); // 0..100 %
		const int counts = 12 + pot * 190 / 100; // timer B counts at 2.5 MHz / 50
		m_mfp->tbi_w(1);
		// end half a count after the last one, so the count does not depend on which of two
		// timers expiring at the same time runs first
		m_volume_pulse->adjust(attotime::from_hz(40_MHz_XTAL / 16 / 50) * (2 * counts + 1) / 2);
	}
	return 0xffff;
}

TIMER_CALLBACK_MEMBER(emu68k_state::volume_pulse_end)
{
	m_mfp->tbi_w(0);
}

HD44780_PIXEL_UPDATE(emu68k_state::lcd_pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 16)
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
}


u8 emu68k_state::jacks_r()
{
	// Output-jack sense (inferred): bits 0-1 sub out 1, bits 2-3 sub out 2; 0 = no plug.
	return m_jacks.read_safe(0);
}

void emu68k_state::proteus1_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom().region("program", 0);
	map(0x600080, 0x600083).rw("lcdc", FUNC(hd44780_device::read), FUNC(hd44780_device::write)).umask16(0x00ff);
	map(0x600100, 0x60012f).rw(m_mfp, FUNC(mc68901_device::read), FUNC(mc68901_device::write)).umask16(0x00ff);
	map(0x800000, 0x8003ff).nopw(); // ???
	map(0xffc000, 0xffffff).ram();
}

void emu68k_state::proteusxr_map(address_map &map)
{
	map(0x000000, 0x01ffff).rom().region("program", 0);
	map(0x400000, 0x400001).nopw(); // latch at 0x400001, written 0x0d every main-loop pass; function unknown
	map(0x500000, 0x50004f).nopw();
	map(0x500008, 0x500047).w("snd", FUNC(proteus_snd_device::pan_w)).umask16(0x00ff); // per-voice output/pan byte at 0x500009 + 2 * voice
	map(0x700000, 0x700003).rw("lcdc", FUNC(hd44780_device::read), FUNC(hd44780_device::write)).umask16(0x00ff);
	map(0x800000, 0x8003ff).rw("snd", FUNC(proteus_snd_device::read), FUNC(proteus_snd_device::write));
	map(0x900000, 0x90002f).rw(m_mfp, FUNC(mc68901_device::read), FUNC(mc68901_device::write)).umask16(0x00ff);
	map(0xa00000, 0xa00001).r(FUNC(emu68k_state::volume_strobe_r));
	map(0xb00000, 0xb00001).nopw(); // latch at 0xb00001, function unknown
	map(0xc00001, 0xc00001).r(FUNC(emu68k_state::jacks_r));
	map(0xd00000, 0xd00003).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write)).umask16(0x00ff); // factory debug serial
	map(0xff0000, 0xffffff).ram().share("nvram"); // the firmware detects the XR by probing RAM at 0xff0000; RAM size, decoding and battery-backed part unverified
}

void emu68k_state::vintkeys_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom().region("program", 0);
	map(0x600080, 0x600083).rw("lcdc", FUNC(hd44780_device::read), FUNC(hd44780_device::write)).umask16(0x00ff);
	map(0x600100, 0x60012f).rw(m_mfp, FUNC(mc68901_device::read), FUNC(mc68901_device::write)).umask16(0x00ff);
	map(0x800000, 0x8003ff).nopw(); // ???
	map(0xffc000, 0xffffff).ram();
}

void emu68k_state::phatt_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("program", 0);
	map(0x600080, 0x600083).rw("lcdc", FUNC(hd44780_device::read), FUNC(hd44780_device::write)).umask16(0x00ff);
	map(0x600100, 0x60012f).rw(m_mfp, FUNC(mc68901_device::read), FUNC(mc68901_device::write)).umask16(0x00ff);
	map(0x800000, 0x8003ff).nopw(); // ???
	map(0xffc000, 0xffffff).ram();
}

void emu68k_state::fc7_map(address_map &map)
{
	map(0xfffff3, 0xfffff3).r(m_mfp, FUNC(mc68901_device::get_vector));
}

// Sound ROM space, 16-bit word addresses. Decoding above 0x3fffff is unverified; it mirrors here.
void emu68k_state::proteusxr_waves_map(address_map &map)
{
	map(0x000000, 0x1fffff).mirror(0xc00000).rom().region("waves", 0);
}

void emu68k_state::proteusxrpl_waves_map(address_map &map)
{
	proteusxr_waves_map(map);
	map(0x200000, 0x3fffff).mirror(0xc00000).rom().region("expansion", 0);
}


static INPUT_PORTS_START(proteus1)
INPUT_PORTS_END

static INPUT_PORTS_START(proteusxr)
	PORT_START("JACKS")
	PORT_CONFNAME(0x03, 0x00, "Sub out 1 jack")
	PORT_CONFSETTING(0x00, "Unplugged")
	PORT_CONFSETTING(0x03, "Plugged")
	PORT_CONFNAME(0x0c, 0x00, "Sub out 2 jack")
	PORT_CONFSETTING(0x00, "Unplugged")
	PORT_CONFSETTING(0x0c, "Plugged")

	PORT_START("VOLUME")
	PORT_ADJUSTER(100, "Volume")
INPUT_PORTS_END

static INPUT_PORTS_START(procuss)
	PORT_INCLUDE(proteusxr)
INPUT_PORTS_END

static INPUT_PORTS_START(vintkeys)
INPUT_PORTS_END

static INPUT_PORTS_START(orbit9090)
INPUT_PORTS_END

static INPUT_PORTS_START(phatt)
INPUT_PORTS_END

static INPUT_PORTS_START(carnaval)
INPUT_PORTS_END

void emu68k_state::palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(131, 136, 139));
	palette.set_pen_color(1, rgb_t( 92,  83,  88));
}

void emu68k_state::add_lcd(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen").set_lcd());
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_screen_update("lcdc", FUNC(hd44780_device::screen_update));
	screen.set_size(6*16, 8*2);
	screen.set_visarea_full();
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(emu68k_state::palette_init), 2);

	hd44780_device &lcdc(HD44780(config, "lcdc", 270'000)); // TODO: clock not measured, datasheet typical clock used
	lcdc.set_lcd_size(2, 16);
	lcdc.set_pixel_update_cb(FUNC(emu68k_state::lcd_pixel_update));
}

void emu68k_state::proteus1(machine_config &config)
{
	M68000(config, m_maincpu, 10'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &emu68k_state::proteus1_map);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &emu68k_state::fc7_map);

	MC68901(config, m_mfp, 10'000'000); // FIXME: not the right type at all
	m_mfp->set_timer_clock(2'500'000);
	m_mfp->out_irq_cb().set_inputline(m_maincpu, M68K_IRQ_1); // TODO: verify level

	add_lcd(config);
}

void emu68k_state::vintkeys(machine_config &config)
{
	proteus1(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &emu68k_state::vintkeys_map);
}

void emu68k_state::phatt(machine_config &config)
{
	proteus1(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &emu68k_state::phatt_map);
}

void emu68k_state::proteusxr(machine_config &config)
{
	M68000(config, m_maincpu, 40_MHz_XTAL / 4); // MC68000P10
	m_maincpu->set_addrmap(AS_PROGRAM, &emu68k_state::proteusxr_map);
	m_maincpu->set_addrmap(m68000_base_device::AS_CPU_SPACE, &emu68k_state::fc7_map);

	// TODO: RAM chips (2x CXK5864BP-12L, 16 KB, on the Proteus/1?), DS1210 + battery; the 64 KB mapping covers the firmware's XR probe at 0xff0000

	MC68901(config, m_mfp, 40_MHz_XTAL / 4); // MC68901P
	m_mfp->set_timer_clock(40_MHz_XTAL / 16); // TODO: determine divider (2.5 MHz gives a 1 kHz timer D tick with the firmware's /100, 25 setup)
	m_mfp->out_irq_cb().set_inputline(m_maincpu, M68K_IRQ_1); // TODO: verify level
	m_mfp->out_so_cb().set("mdout", FUNC(midi_port_device::write_txd));

	// MIDI: UCR = 0x88 (x16 clock) -> 500 kHz receive/transmit clock (source unverified)
	clock_device &midiclk(CLOCK(config, "midiclk", 500'000));
	midiclk.signal_handler().set(m_mfp, FUNC(mc68901_device::rc_w));
	midiclk.signal_handler().append(m_mfp, FUNC(mc68901_device::tc_w));
	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(m_mfp, FUNC(mc68901_device::si_w));
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// Factory debug serial port at 0xd00001/0xd00003 (clock unverified)
	acia6850_device &acia(ACIA6850(config, "acia"));
	rs232_port_device &rs232(RS232_PORT(config, "rs232", default_rs232_devices, nullptr));
	acia.txd_handler().set(rs232, FUNC(rs232_port_device::write_txd));
	rs232.rxd_handler().set(acia, FUNC(acia6850_device::write_rxd));
	clock_device &aciaclk(CLOCK(config, "aciaclk", 9600 * 16));
	aciaclk.signal_handler().set(acia, FUNC(acia6850_device::write_rxc));
	aciaclk.signal_handler().append(acia, FUNC(acia6850_device::write_txc));

	SPEAKER(config, "speaker", 2).front();
	PROTEUS_SND(config, m_snd, 40_MHz_XTAL / 2);
	m_snd->set_addrmap(0, &emu68k_state::proteusxr_waves_map);
	m_snd->add_route(0, "speaker", 1.0, 0);
	m_snd->add_route(1, "speaker", 1.0, 1);

	add_lcd(config);
}

void emu68k_state::proteusxrpl(machine_config &config)
{
	proteusxr(config);
	m_snd->set_addrmap(0, &emu68k_state::proteusxrpl_waves_map);
}

// TODO: the Proteus/1 has the same sound ROMs as the XR (PROTEUS1_WAVES); the other models' sound ROMs are not dumped

// Proteus/1 sound set: 8x 512 KB mask ROMs (AKN62314BP), 16-bit signed samples, 4-way word
// interleave: word n = { IP40x[n >> 2], IP38x[n >> 2] }, x = 2 + (n & 3). The per-chip checksums
// in the ROM header match.
#define PROTEUS1_WAVES \
	ROM_LOAD64_BYTE("ip402_b0msb.bin", 0x000000, 0x80000, CRC(ecb30a59) SHA1(edb714212bce3a84d5b76abbff1ac4af2961a42a)) \
	ROM_LOAD64_BYTE("ip382_b0lsb.bin", 0x000001, 0x80000, CRC(f30101b6) SHA1(5eacb2208e254043676de685b545d8f17f284f5c)) \
	ROM_LOAD64_BYTE("ip403_b1msb.bin", 0x000002, 0x80000, CRC(ab2086ec) SHA1(7ff5eb2c83ea45f33b9fc8983f94b8cf7cca4d4c)) \
	ROM_LOAD64_BYTE("ip383_b1lsb.bin", 0x000003, 0x80000, CRC(8df7c2d2) SHA1(c3abe07987703fe4a19f27de40ca10ae3cf6b706)) \
	ROM_LOAD64_BYTE("ip404_b2msb.bin", 0x000004, 0x80000, CRC(fd8fd68a) SHA1(a1c02c379d97faac2680507244b863fc3730d069)) \
	ROM_LOAD64_BYTE("ip384_b2lsb.bin", 0x000005, 0x80000, CRC(b1bea229) SHA1(c246851b97e13fdd057fcfe0fbe6af7016a22e07)) \
	ROM_LOAD64_BYTE("ip405_b3msb.bin", 0x000006, 0x80000, CRC(80d1a873) SHA1(66361832d6f9a160f49068e0730a80ae65b85fe3)) \
	ROM_LOAD64_BYTE("ip385_b3lsb.bin", 0x000007, 0x80000, CRC(375ca330) SHA1(4d662ac18bc2bd8f5fdeb45124b48a62e0ac5489))

ROM_START(proteus1)
	ROM_REGION16_BE(0x20000, "program", 0)
	ROM_LOAD16_BYTE("e-mu proteus 1 msb 2.05.bin", 0x00000, 0x10000, CRC(6a84ac60) SHA1(0b0b3689678522c26087919a2387ed184391d3f4)) // 27C512
	ROM_LOAD16_BYTE("e-mu proteus 1 lsb 2.05.bin", 0x00001, 0x10000, CRC(3ae786b4) SHA1(b9ad95a5cab86f2d316af34e6cb1b2873c58a9b5)) // 27C512
ROM_END

// v2.30 is E-mu's update to v2.00
#define PROTEUSXR_FIRMWARE \
	ROM_REGION16_BE(0x20000, "program", 0) \
	ROM_SYSTEM_BIOS(0, "v230", "v2.30") \
	ROMX_LOAD("ip486_901101.ic38", 0x00000, 0x10000, CRC(036bf047) SHA1(7e8f6474545bf24e6af318db4cd573ad05c66cbb), ROM_BIOS(0) | ROM_SKIP(1)) \
	ROMX_LOAD("ip485_901101.ic37", 0x00001, 0x10000, CRC(3c1d85c6) SHA1(bcd668ac2c5ffbf43dc1c50a3bbd85a87b537845), ROM_BIOS(0) | ROM_SKIP(1)) \
	ROM_SYSTEM_BIOS(1, "v200", "v2.00") \
	ROMX_LOAD("ip441c_emu__5089.ic38", 0x00000, 0x10000, CRC(01f11633) SHA1(a0612007b22c8e06c803ee5a5bad98e3f2bc4421), ROM_BIOS(1) | ROM_SKIP(1)) /* NMC27C512AN */ \
	ROMX_LOAD("ip442c_emu__5089.ic37", 0x00001, 0x10000, CRC(e8154a65) SHA1(1ec834712dc9ea4f4f01090b2fd5e558c1f08208), ROM_BIOS(1) | ROM_SKIP(1)) /* NMC27C512AN */

ROM_START(proteusxr)
	PROTEUSXR_FIRMWARE

	ROM_REGION16_BE(0x400000, "waves", 0)
	PROTEUS1_WAVES
ROM_END

// Proteus/1 XR with the ProtoLogic 4 MB sound expansion (InVision Interactive)
ROM_START(proteusxrpl)
	PROTEUSXR_FIRMWARE

	// Sound ROMs: 16-bit signed samples, 4-way word interleave across chip pairs:
	// word n = { IP70x[n >> 2], IP50x[n >> 2] }, x = 5 + (n & 3)  (verified against a WAV conversion of the set)
	ROM_REGION16_BE(0x400000, "waves", 0)
	PROTEUS1_WAVES

	ROM_REGION16_BE(0x400000, "expansion", 0)
	ROM_LOAD64_BYTE("ip705.bin", 0x000000, 0x80000, CRC(9291ac91) SHA1(35c2ee0fafa9bf99643a34a610a8ab0b32a0f42d))
	ROM_LOAD64_BYTE("ip505.bin", 0x000001, 0x80000, CRC(26c3cbc5) SHA1(e7c587010713bd911b9805c8616e1e920c4307e1))
	ROM_LOAD64_BYTE("ip706.bin", 0x000002, 0x80000, CRC(e23d5890) SHA1(f02a46f2bbfa928356a0c4521b021f9fa38049b1))
	ROM_LOAD64_BYTE("ip506.bin", 0x000003, 0x80000, CRC(a316d1c7) SHA1(4eb67d722c064d66ab1604ffb9ffe02b039dfcd9))
	ROM_LOAD64_BYTE("ip707.bin", 0x000004, 0x80000, CRC(94b926ed) SHA1(1d10905cad823d340cfa5ceec1b3ade323f2bc31))
	ROM_LOAD64_BYTE("ip507.bin", 0x000005, 0x80000, CRC(e0fc7a84) SHA1(316df7f76b48f17b49b4397268a9c7021c84ed12))
	ROM_LOAD64_BYTE("ip708.bin", 0x000006, 0x80000, CRC(6235247e) SHA1(18fc57153bf69e854efa0c48f4fcec0b62c73654))
	ROM_LOAD64_BYTE("ip508.bin", 0x000007, 0x80000, CRC(5d48b113) SHA1(54571ee9b4c1992172a94557cf8e9c4b74fef0d4))
ROM_END

ROM_START(procuss)
	ROM_REGION16_BE(0x20000, "program", 0) // v1.01
	ROM_LOAD16_BYTE("ip516aemu9115.bin", 0x00000, 0x10000, CRC(8c0be608) SHA1(54d5629e4adebd72d5d5924941be3e6319d20427))
	ROM_LOAD16_BYTE("ip517aemu9115.bin", 0x00001, 0x10000, CRC(cb2e454b) SHA1(b142285b998829ec8b3c6ce98eb94d9d4964aff6))

	ROM_REGION16_BE(0x400000, "waves", ROMREGION_ERASE00) // sound ROMs not dumped
ROM_END

ROM_START(vintkeys)
	ROM_REGION16_BE(0x40000, "program", 0) // v1.03
	ROM_LOAD16_BYTE("vintage_keys_msb.bin", 0x00000, 0x20000, CRC(8ac3123f) SHA1(29180ea26c3de326af0f789a2d7a85face528d1a))
	ROM_LOAD16_BYTE("vintage_keys_lsb.bin", 0x00001, 0x20000, CRC(032e4064) SHA1(f59841eda06e2f348f50887d5bf10aef3a027346))
ROM_END

ROM_START(orbit9090)
	// These may or may not need to be split into separate sets
	ROM_REGION16_BE(0x80000, "program", 0)
	ROM_SYSTEM_BIOS(0, "v2", "v2.00") // Original chips are STMicro M27C2001-12F1
	ROMX_LOAD("orbit-2.00-msb.bin", 0x00000, 0x40000, CRC(6bda893e) SHA1(0ff01a46ba49fd49b8d85edcc9ac141009f73ebe), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("orbit-2.00-lsb.bin", 0x00001, 0x40000, CRC(95ef87f6) SHA1(c0dde419e9b93aa9f4d5b48c9b9bd039a33d2413), ROM_BIOS(0) | ROM_SKIP(1))
	ROM_SYSTEM_BIOS(1, "v1", "v1.00") // Original chips are TMS27C010A-15; "©EMU'96"
	ROMX_LOAD("mip840a__1496.bin", 0x00000, 0x20000, CRC(f60cde2a) SHA1(6762c1e0aa5044e504799041f960d037a600cb62), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("lip839a__1496.bin", 0x00001, 0x20000, CRC(5f16e6db) SHA1(e820bf6ce61256f191c063d4193ed4602c500065), ROM_BIOS(1) | ROM_SKIP(1))
ROM_END

ROM_START(phatt)
	ROM_REGION16_BE(0x80000, "program", 0)
	ROM_LOAD16_BYTE("e-mu planet phatt msb 1.01.bin", 0x00000, 0x40000, CRC(4db960c2) SHA1(1af53bd85ca7fb06709981ce3915cb46e8785cc2)) // 27C2001
	ROM_LOAD16_BYTE("e-mu planet phatt lsb 1.01.bin", 0x00001, 0x40000, CRC(0b2deb2b) SHA1(7715ac34f5eb9d27337eb77e62b66da8c3e80950)) // 27C2001
ROM_END

ROM_START(carnaval)
	ROM_REGION16_BE(0x80000, "program", 0)
	// Original chips are STMicro M27C2001-12F1
	ROM_LOAD16_BYTE("carnaval-1.00-msb.bin", 0x00000, 0x40000, CRC(962b3d2b) SHA1(e4668306e21fef8b0b696f08bed7c6e66941c8b2))
	ROM_LOAD16_BYTE("carnaval-1.00-lsb.bin", 0x00001, 0x40000, CRC(95506a6f) SHA1(1f6f2cce4fac36f4daa22bded1013f3dd0ca72db))
ROM_END

} // anonymous namespace


SYST(1989, proteus1,  0, 0, proteus1,  proteus1,  emu68k_state, empty_init, "E-mu Systems", "Proteus/1 16-Bit Multi-Timbral Digital Sound Module", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
SYST(1989, proteusxr, 0, 0, proteusxr, proteusxr, emu68k_state, empty_init, "E-mu Systems", "Proteus/1 XR 16-Bit Multi-Timbral Digital Sound Module", MACHINE_NOT_WORKING)
SYST(199?, proteusxrpl, proteusxr, 0, proteusxrpl, proteusxr, emu68k_state, empty_init, "E-mu Systems", "Proteus/1 XR 16-Bit Multi-Timbral Digital Sound Module (with ProtoLogic expansion)", MACHINE_NOT_WORKING)
SYST(1991, procuss,   0, 0, proteusxr, procuss,   emu68k_state, empty_init, "E-mu Systems", "Pro/Cussion Maximum Percussion Module", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
SYST(1993, vintkeys,  0, 0, vintkeys,  vintkeys,  emu68k_state, empty_init, "E-mu Systems", "Vintage Keys Classic Analog Keyboards", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
SYST(1996, orbit9090, 0, 0, phatt,     orbit9090, emu68k_state, empty_init, "E-mu Systems", "Orbit 9090 - The Dance Planet", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
SYST(1997, phatt,     0, 0, phatt,     phatt,     emu68k_state, empty_init, "E-mu Systems", "Planet Phatt - The Swing System", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
SYST(1997, carnaval,  0, 0, phatt,     carnaval,  emu68k_state, empty_init, "E-mu Systems", "Carnaval - Jugando con Fuego", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
