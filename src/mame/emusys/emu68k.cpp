// license:BSD-3-Clause
// copyright-holders:AJR
/***********************************************************************************************************************************

    Skeleton driver for E-mu Proteus and other MC68000-based synthesizer modules.

***********************************************************************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/mc68901.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"


namespace {

class emu68k_state : public driver_device
{
public:
	emu68k_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_mfp(*this, "mfp")
	{
	}

	void proteus1(machine_config &config);
	void vintkeys(machine_config &config);
	void phatt(machine_config &config);
	void proteusxr(machine_config &config);

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

	required_device<m68000_base_device> m_maincpu;
	required_device<mc68901_device> m_mfp;
};

void emu68k_state::machine_start()
{
}

HD44780_PIXEL_UPDATE(emu68k_state::lcd_pixel_update)
{
	if (x < 5 && y < 8 && line < 2 && pos < 16)
		bitmap.pix(line * 8 + y, pos * 6 + x) = state;
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
	map(0x400000, 0x400001).nopw(); // ???
	map(0x700000, 0x700003).rw("lcdc", FUNC(hd44780_device::read), FUNC(hd44780_device::write)).umask16(0x00ff);
	map(0x800000, 0x8003ff).nopw(); // ???
	map(0x900000, 0x90002f).rw(m_mfp, FUNC(mc68901_device::read), FUNC(mc68901_device::write)).umask16(0x00ff);
	map(0xa00000, 0xa00001).nopr(); // watchdog?
	map(0xb00000, 0xb00001).nopw(); // ???
	map(0xffc000, 0xffffff).ram();
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


static INPUT_PORTS_START(proteus1)
INPUT_PORTS_END

static INPUT_PORTS_START(procuss)
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
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
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

	// TODO: NVRAM = 2x CXK5864BP-12L + DS1210 + battery?

	MC68901(config, m_mfp, 40_MHz_XTAL / 4); // MC68901P
	m_mfp->set_timer_clock(40_MHz_XTAL / 16); // TODO: determine divider
	m_mfp->out_irq_cb().set_inputline(m_maincpu, M68K_IRQ_1); // TODO: verify level

	add_lcd(config);
}

// TODO: all of these have sample ROMs as well (none currently dumped)

ROM_START(proteus1)
	ROM_REGION16_BE(0x20000, "program", 0)
	ROM_LOAD16_BYTE("e-mu proteus 1 msb 2.05.bin", 0x00000, 0x10000, CRC(6a84ac60) SHA1(0b0b3689678522c26087919a2387ed184391d3f4)) // 27C512
	ROM_LOAD16_BYTE("e-mu proteus 1 lsb 2.05.bin", 0x00001, 0x10000, CRC(3ae786b4) SHA1(b9ad95a5cab86f2d316af34e6cb1b2873c58a9b5)) // 27C512
ROM_END

ROM_START(proteusxr)
	ROM_REGION16_BE(0x20000, "program", 0)
	ROM_LOAD16_BYTE("ip441c_emu__5089.ic38", 0x00000, 0x10000, CRC(01f11633) SHA1(a0612007b22c8e06c803ee5a5bad98e3f2bc4421)) // NMC27C512AN
	ROM_LOAD16_BYTE("ip442c_emu__5089.ic37", 0x00001, 0x10000, CRC(e8154a65) SHA1(1ec834712dc9ea4f4f01090b2fd5e558c1f08208)) // NMC27C512AN
ROM_END

ROM_START(procuss)
	ROM_REGION16_BE(0x20000, "program", 0) // v1.01
	ROM_LOAD16_BYTE("ip516aemu9115.bin", 0x00000, 0x10000, CRC(8c0be608) SHA1(54d5629e4adebd72d5d5924941be3e6319d20427))
	ROM_LOAD16_BYTE("ip517aemu9115.bin", 0x00001, 0x10000, CRC(cb2e454b) SHA1(b142285b998829ec8b3c6ce98eb94d9d4964aff6))
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


SYST(1989, proteus1,  0, 0, proteus1,  proteus1,  emu68k_state, empty_init, "E-mu Systems", "Proteus/1 16-Bit Multi-Timbral Digital Sound Module", MACHINE_IS_SKELETON)
SYST(1989, proteusxr, 0, 0, proteusxr, proteus1,  emu68k_state, empty_init, "E-mu Systems", "Proteus/1 XR 16-Bit Multi-Timbral Digital Sound Module", MACHINE_IS_SKELETON)
SYST(1991, procuss,   0, 0, proteusxr, procuss,   emu68k_state, empty_init, "E-mu Systems", "Pro/Cussion Maximum Percussion Module", MACHINE_IS_SKELETON)
SYST(1993, vintkeys,  0, 0, vintkeys,  vintkeys,  emu68k_state, empty_init, "E-mu Systems", "Vintage Keys Classic Analog Keyboards", MACHINE_IS_SKELETON)
SYST(1996, orbit9090, 0, 0, phatt,     orbit9090, emu68k_state, empty_init, "E-mu Systems", "Orbit 9090 - The Dance Planet", MACHINE_IS_SKELETON)
SYST(1997, phatt,     0, 0, phatt,     phatt,     emu68k_state, empty_init, "E-mu Systems", "Planet Phatt - The Swing System", MACHINE_IS_SKELETON)
SYST(1997, carnaval,  0, 0, phatt,     carnaval,  emu68k_state, empty_init, "E-mu Systems", "Carnaval - Jugando con Fuego", MACHINE_IS_SKELETON)
