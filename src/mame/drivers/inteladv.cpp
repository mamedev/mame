// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

 inteladv.cpp: VTech Intelligence Advance E/R Lerncomputer

 CPU is a Rockwell R65C02 (the dead-end bit-twiddling 65C02, as opposed to
 the WDC version that the 65816 is back-compatible with) with some customizations:

 JMP (ZP) accepts a 3rd ZP location after the 16-bit address to jump to which
 contains a bank value for the ROM window at 0x4000.  I believe it's in 0x4000
 byte segments, as the first long jump uses a value of 4 and that leads to
 the correct start of a subroutine at 0x10000 + the offset.

 JSR / RTS may also push and pop an additional byte for the bank offset but
 this is not proven yet.

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/r65c02.h"
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class inteladv_state : public driver_device
{
public:
	inteladv_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette")
	{ }

	void inteladv(machine_config &config);
	void dyndesk(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	u32 screen_update_inteladv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void inteladv_main(address_map &map);
	void dyndesk_main(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
};

void inteladv_state::video_start()
{
}

u32 inteladv_state::screen_update_inteladv(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	screen.priority().fill(0, cliprect);

	return 0;
}

void inteladv_state::inteladv_main(address_map &map)
{
	map(0x0000, 0x01ff).ram(); // zero page and stack
	map(0x4000, 0x5fff).rom().region("maincpu", 0x0000);    // boot code at 4000
	map(0x8000, 0x8fff).rom().region("maincpu", 0x8000);    // fixed ROM region?
	map(0xf000, 0xffff).rom().region("maincpu", 0x3000);    // boot and other vectors at 3FFx
}

void inteladv_state::dyndesk_main(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x4000, 0x5fff).rom().region("maincpu", 0x4000);    // FIXME: this is banked
	map(0xc000, 0xdfff).rom().region("maincpu", 0x4000);
	map(0xffe0, 0xffff).rom().region("maincpu", 0x7fe0);    // vectors are good for boot, but may also be banked
}

static INPUT_PORTS_START( inteladv )
INPUT_PORTS_END

void inteladv_state::machine_start()
{
}

void inteladv_state::machine_reset()
{
}

void inteladv_state::inteladv(machine_config &config)
{
	/* basic machine hardware */
	R65C02(config, m_maincpu, XTAL(1'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &inteladv_state::inteladv_main);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(59.62);  /* verified on pcb */
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(40, 400-1, 16, 240-1);
	screen.set_screen_update(FUNC(inteladv_state::screen_update_inteladv));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xBGR_888, 256).enable_shadows();

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}

void inteladv_state::dyndesk(machine_config &config)
{
	inteladv(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &inteladv_state::dyndesk_main);
}

ROM_START( inteladv )
	ROM_REGION( 0x800000, "maincpu", 0 ) /* main program */
	ROM_LOAD( "vtechinteladv.bin", 0x000000, 0x800000, CRC(e24dbbcb) SHA1(7cb7f25f5eb123ae4c46cd4529aafd95508b2210) )
ROM_END

ROM_START( dyndesk )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "27-07710-000.u9", 0x000000, 0x200000, CRC(092b0303) SHA1(e3a58cac9b0a1c68f1bdb5ea0af0b0dd223fb340) )
	// PCB also has Hynix HY62WT081ED70C (32Kx8 CMOS SRAM) at U2
ROM_END

ROM_START( cars2lap )
	ROM_REGION( 0x200000, "maincpu", 0 )
	// Flash dump contains some 65C02 code starting at $000D6A, but mostly appears to be custom bytecode or non-executable data banked in 32K blocks
	ROM_LOAD("n25s16.u6", 0x00000, 0x200000, CRC(ec1ba96e) SHA1(51b8844ae77adf20f74f268d380d268c9ce19785))
ROM_END

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY  FULLNAME                                 FLAGS
COMP( 1995, inteladv, 0,      0,      inteladv, inteladv, inteladv_state, empty_init, "VTech", "Intelligence Advance E/R Lerncomputer", MACHINE_NOT_WORKING )
COMP( 2003, dyndesk,  0,      0,      dyndesk,  inteladv, inteladv_state, empty_init, "VTech", "DynamiDesk",                            MACHINE_NOT_WORKING )
COMP( 2012, cars2lap, 0,      0,      inteladv, inteladv, inteladv_state, empty_init, "VTech", "CARS 2 Laptop (Germany)",               MACHINE_IS_SKELETON ) // might not belong here
