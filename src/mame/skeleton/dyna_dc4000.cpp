// license:BSD-3-Clause
// copyright-holders:

/*
Dyna games using the DYNA DC4000 GFX custom.
Currently it has been seen on
DYNA D9105 and D9106 PCBs

The following are the main components on a DYNA D9106C PCB:
-Zilog Z0840006
-DYNA DC4000 (GFX custom - some kind of CRTC?)
-DYNA 22A078803 (I/O custom - similar to I8255 according to misc/cb2001.cpp)
-6116 RAM (work / NVRAM)
-4x M5M4C264L or equivalent (video RAM)
-24 MHz XTAL
-Winbond WF19054 (AY8910 clone)
-5 banks of 8 DIP switches, plus an unpopulated location on the PCB for a sixth one.
*/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class dyna_dc4000_state : public driver_device
{
public:
	dyna_dc4000_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfx_rom(*this, "gfx")
	{ }

	void d9106(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_region_ptr<uint8_t> m_gfx_rom;

	uint8_t m_command = 0;

	void regs_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
};


void dyna_dc4000_state::regs_w(uint8_t data)
{
	switch (m_command)
	{
		// init / reset?
		case 0x00:
			logerror("%s init: %02x\n", machine().describe_context(), m_command);
			break;

		// regs write? after this command, sends 8 consecutive bytes here
		case 0x08:
			logerror("%s regs write: %02x\n", machine().describe_context(), m_command);
			break;

		// palette write?
		case 0x10:
			logerror("%s palette write: %02x\n", machine().describe_context(), m_command);
			break;

		// page select?
		case 0x11:
			logerror("%s page select: %02x\n", machine().describe_context(), m_command);
			break;

		default:
			logerror("%s unknown command: %02x\n", machine().describe_context(), m_command);
	}
}

uint32_t dyna_dc4000_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void dyna_dc4000_state::machine_start()
{
	save_item(NAME(m_command));
}


void dyna_dc4000_state::program_map(address_map &map)
{
	map(0x0000, 0xdfff).rom();
	map(0xe000, 0xefff).ram().share("nvram");
}

void dyna_dc4000_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x02, 0x03).w("aysnd", FUNC(ay8910_device::data_address_w));
	map(0x10, 0x10).lw8(NAME([this] (uint8_t data) { m_command = data; }));
	map(0x11, 0x11).w(FUNC(dyna_dc4000_state::regs_w));
	map(0x20, 0x23).rw("ppi", FUNC(i8255_device::read), FUNC(i8255_device::write));  // TODO: inputs / GFX chip busy line
	map(0x30, 0x30).portr("IN0"); // TODO: w lamps, hopper, meters
	map(0x31, 0x31).portr("IN1"); // TODO: w hopper motor, coin counters
	map(0x32, 0x32).portr("IN2"); // TODO: w ??
}


static INPUT_PORTS_START( default )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(2)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(3)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(4) // GFX chip busy?
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(4)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(4)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(4)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(4)

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW3:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW3:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW3:8")

	PORT_START("DSW4")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW4:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW4:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW4:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW4:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW4:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW4:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW4:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW4:8")

	PORT_START("DSW5")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW5:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW5:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW5:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW5:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW5:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW5:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW5:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW5:8")
INPUT_PORTS_END


void dyna_dc4000_state::d9106(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 24_MHz_XTAL / 4); // divider not verified, but chip rated for 6 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &dyna_dc4000_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &dyna_dc4000_state::io_map);

	i8255_device &ppi(I8255A(config, "ppi")); // actually DYNA 22A078803
	ppi.in_pa_callback().set_ioport("DSW1");
	ppi.in_pb_callback().set_ioport("DSW2");
	ppi.in_pc_callback().set_ioport("DSW3");

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER)); // TODO: everything
	screen.set_refresh_hz(60);
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(dyna_dc4000_state::screen_update));
	screen.screen_vblank().set_inputline(m_maincpu, 0, HOLD_LINE);

	PALETTE(config, "palette").set_entries(0x100); // TODO

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", 24_MHz_XTAL / 16)); // divider not verified, actually Winbond WF19054
	aysnd.port_a_read_callback().set_ioport("DSW4");
	aysnd.port_a_write_callback().set([this] (uint8_t data) { logerror("AY port A write: %02x\n", data); });
	aysnd.port_b_read_callback().set_ioport("DSW5");
	aysnd.port_b_write_callback().set([this] (uint8_t data) { logerror("AY port B write: %02x\n", data); });
	aysnd.add_route(ALL_OUTPUTS, "mono", 1.00);
}


//  The Aladdin. Dyna, 1991
//  V1.2U - PCB D9106
ROM_START( aladdin )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD16_WORD( "ald2_v1.2u_27c512.d15", 0x00000, 0x10000, CRC(66c638ca) SHA1(ac0e9af5cd7535e8a86573723851b987c4a80c63) )

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD( "538000_as_27c080.h2", 0x000000, 0x100000, CRC(800c6c8d) SHA1(bf8d8f05b21e6cd4f0efed1ae7b66c2d9d8f43ee) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "mb7114h.h14", 0x000, 0x100, CRC(a69819b8) SHA1(5818046aae387f5b137c379cc4d78a15739c71cc) )
	ROM_LOAD( "mb7114h.h15", 0x100, 0x100, CRC(36c08918) SHA1(fa87dea8fd27c1ac7e007e2cdef77ef5eabf1a7b) )
	ROM_LOAD( "mb7114h.h16", 0x200, 0x100, CRC(71e66913) SHA1(800b05ea8eb1bb89e933a6f44632c7ebfea52e03) )
ROM_END

//  The Aladdin. Dyna, 1991
//  V1.1A - PCB D9106
ROM_START( aladdina )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD16_WORD( "ald2_v1.1a_27c512.d15", 0x00000, 0x10000, CRC(b13baf47) SHA1(2c45edca22add535a5cf367810ac26d84f7abd82) )

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD( "538000_as_27c080.h2", 0x000000, 0x100000, CRC(800c6c8d) SHA1(bf8d8f05b21e6cd4f0efed1ae7b66c2d9d8f43ee) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "mb7114h.h14", 0x000, 0x100, CRC(a69819b8) SHA1(5818046aae387f5b137c379cc4d78a15739c71cc) )
	ROM_LOAD( "mb7114h.h15", 0x100, 0x100, CRC(36c08918) SHA1(fa87dea8fd27c1ac7e007e2cdef77ef5eabf1a7b) )
	ROM_LOAD( "mb7114h.h16", 0x200, 0x100, CRC(71e66913) SHA1(800b05ea8eb1bb89e933a6f44632c7ebfea52e03) )
ROM_END

ROM_START( cmast92 ) // DYNA D9106B PCB
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "cm9230d.rom",   0x00000, 0x10000, CRC(214a0a2d) SHA1(2d349e0888ac2da3df954517fdeb9214a3b17ae1) )  // V1.2D

	// the rest of the PROMs were dumped for V1.1D, adding them as bad until it can be verified they're good for this newer version, too
	ROM_REGION( 0x120000, "gfx", 0 )
	ROM_LOAD( "dyna dm9105.2h", 0x000000, 0x100000, NO_DUMP )
	ROM_LOAD( "1h",             0x000000, 0x020000, BAD_DUMP CRC(2ca1ba89) SHA1(dec50bb0f68f03d3433cc3a09eec5ee60f2d096c) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "14h", 0x000, 0x100, BAD_DUMP CRC(20e594fe) SHA1(d798f142732e8da6ec9764133955c041d2259f64) )
	ROM_LOAD( "15h", 0x100, 0x100, BAD_DUMP CRC(83fab238) SHA1(7c5451d69f865a10b63c013169ddbf57405bc3a9) )
	ROM_LOAD( "16h", 0x200, 0x100, BAD_DUMP CRC(706e7ee6) SHA1(dca1cc0e2c1c27bc211516ad369f557eb4b3980a) )
ROM_END

ROM_START( cmast92a ) // DYNA D9106B PCB
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "15d", 0x00000, 0x10000, CRC(d703c8e5) SHA1(77d8228878b64a299b4b6f3fe3befcea179ca4af) )  // V1.1D

	ROM_REGION( 0x120000, "gfx", 0 )
	ROM_LOAD( "dyna dm9105.2h", 0x000000, 0x100000, NO_DUMP )
	ROM_LOAD( "1h",              0x00000, 0x020000, CRC(2ca1ba89) SHA1(dec50bb0f68f03d3433cc3a09eec5ee60f2d096c) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "14h", 0x000, 0x100, CRC(20e594fe) SHA1(d798f142732e8da6ec9764133955c041d2259f64) )
	ROM_LOAD( "15h", 0x100, 0x100, CRC(83fab238) SHA1(7c5451d69f865a10b63c013169ddbf57405bc3a9) )
	ROM_LOAD( "16h", 0x200, 0x100, CRC(706e7ee6) SHA1(dca1cc0e2c1c27bc211516ad369f557eb4b3980a) )
ROM_END

ROM_START( eldoradd )  // String "DYNA ELD3 V5.1DR" on program ROM
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD16_WORD( "51d_el3_m27c512.15d", 0x00000, 0x10000, CRC(a7769d4a) SHA1(2ccd14be94a0b752113f529431b3dd4fadbf619b) )

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD( "1h_el3_tms27c040.1h", 0x000000, 0x080000, CRC(0ba677ac) SHA1(4492183cd01ba6f8ba3da233a6fd4fcb86447308) )
	ROM_LOAD( "2h_el3_tms27c040.2h", 0x080000, 0x080000, CRC(79a37ee1) SHA1(510e4ab168003d48173d5f8ddbf396668caf8e3e) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "eh_82s135.15h", 0x000, 0x100, CRC(bc64fea7) SHA1(7aef1bd14936c8f445a7ce08547e7ab962cea797) )
	ROM_LOAD( "eg_82s135.15g", 0x100, 0x100, CRC(19214600) SHA1(33a62cd91bf73fa5aa37ab961797b8c5e4ac4e30) )

	ROM_REGION( 0x600, "plds", 0 )
	ROM_LOAD( "pal16l8.13f", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.11e", 0x200, 0x104, NO_DUMP )
	ROM_LOAD( "gal16v8.9f",  0x400, 0x104, NO_DUMP )
ROM_END

ROM_START( eldoraddo )  // String "DYNA ELD3 V1.1TA" on program ROM, DYNA D9105B PCB
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD16_WORD( "dyna nel 20t.c14", 0x00000, 0x10000, CRC(77b3b2ce) SHA1(e94b976ae9e5a899d916fffc8118486cbedab8b6) )

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD( "tc538000p-dyna dm9106.g15", 0x000000, 0x100000, CRC(fa84c372) SHA1(a71e57e76321b7ebb16933d9bc983b9160995b4a) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "mb7114.e8",  0x000, 0x100, CRC(fa274678) SHA1(6712cb1f7ead1a7aa703ec799e7199c33ace857c) )
	ROM_LOAD( "mb7114.e10", 0x100, 0x100, CRC(e58877ea) SHA1(30fa873fc05d91610ef68eef54b78f2c7301a62a) )
	ROM_LOAD( "mb7114.e12", 0x200, 0x100, CRC(781b2842) SHA1(566667d4f81e93b29bb01dbc51bf144c02dff75d) )

	ROM_REGION( 0x400, "plds", 0 )  // available as brute-forced dumps, need to be verified and converted
	ROM_LOAD( "pal16l8.d13", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.e11", 0x200, 0x104, NO_DUMP )
ROM_END

ROM_START( eldoraddob )  // String "DYNA ELD3 V2.0D" in program ROM, DYNA D9105B PCB
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD16_WORD( "elb.50d.d15", 0x00000, 0x10000, CRC(34d55507) SHA1(8cc293bb5e493a837320e14d0316a0658084dde3) )

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD( "tc538000p-dyna dm9106.h2", 0x000000, 0x100000, CRC(fa84c372) SHA1(a71e57e76321b7ebb16933d9bc983b9160995b4a) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "e14", 0x000, 0x100, CRC(fa274678) SHA1(6712cb1f7ead1a7aa703ec799e7199c33ace857c) )
	ROM_LOAD( "e15", 0x100, 0x100, CRC(e58877ea) SHA1(30fa873fc05d91610ef68eef54b78f2c7301a62a) )
	ROM_LOAD( "e16", 0x200, 0x100, CRC(781b2842) SHA1(566667d4f81e93b29bb01dbc51bf144c02dff75d) )

	ROM_REGION( 0x400, "plds", 0 ) // available as brute-forced dumps, need to be verified and converted
	ROM_LOAD( "pal16l8.d13", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.e11", 0x200, 0x104, NO_DUMP )
ROM_END

ROM_START( eldoraddoc )  // String "DYNA ELD3 V1.1J" in program ROM, DYNA D9105B PCB
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD16_WORD( "nel.20d.14c", 0x00000, 0x10000, CRC(fee901b9) SHA1(d304fd5ea39cada5787c9f742f6b7801cf12670c) )

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD( "tc538000p-dyna dm9106.g15", 0x000000, 0x100000, CRC(fa84c372) SHA1(a71e57e76321b7ebb16933d9bc983b9160995b4a) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "mb7114.e8",  0x000, 0x100, CRC(fa274678) SHA1(6712cb1f7ead1a7aa703ec799e7199c33ace857c) )
	ROM_LOAD( "mb7114.e10", 0x100, 0x100, CRC(e58877ea) SHA1(30fa873fc05d91610ef68eef54b78f2c7301a62a) )
	ROM_LOAD( "mb7114.e12", 0x200, 0x100, CRC(781b2842) SHA1(566667d4f81e93b29bb01dbc51bf144c02dff75d) )

	ROM_REGION( 0x400, "plds", 0 )  // available as brute-forced dumps, need to be verified and converted
	ROM_LOAD( "pal16l8.d13", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.e11", 0x200, 0x104, NO_DUMP )
ROM_END

ROM_START( eldoraddod )  // String "DYNA ELD3 V1.1U" in program ROM, DYNA D9105B PCB
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD16_WORD( "27c512_v1.1u.c14", 0x00000, 0x10000, CRC(3274d388) SHA1(180c9389fe7ccdee716b28a87effdc3970e057bf) )

	ROM_REGION( 0x100000, "gfx", 0 )
	ROM_LOAD( "tc538009.g15", 0x000000, 0x100000, CRC(fa84c372) SHA1(a71e57e76321b7ebb16933d9bc983b9160995b4a) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "bprom.d8",  0x000, 0x100, CRC(fa274678) SHA1(6712cb1f7ead1a7aa703ec799e7199c33ace857c) )
	ROM_LOAD( "bprom.d10", 0x100, 0x100, CRC(e58877ea) SHA1(30fa873fc05d91610ef68eef54b78f2c7301a62a) )
	ROM_LOAD( "bprom.d12", 0x200, 0x100, CRC(781b2842) SHA1(566667d4f81e93b29bb01dbc51bf144c02dff75d) )

	ROM_REGION( 0x400, "plds", 0 )  // available as brute-forced dumps, need to be verified and converted
	ROM_LOAD( "pal16l8.d13", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.e11", 0x200, 0x104, NO_DUMP )
ROM_END

} // anonymous namespace


// Dyna D9106 / D9106B / D9106C PCB
GAME(  1991, eldoradd,   0,        d9106, default, dyna_dc4000_state, empty_init, ROT0, "Dyna", "El Dorado (V5.1DR)",        MACHINE_NOT_WORKING )
GAME(  1991, aladdin,    0,        d9106, default, dyna_dc4000_state, empty_init, ROT0, "Dyna", "The Aladdin (V1.2U)",       MACHINE_NOT_WORKING )
GAME(  1991, aladdina,   aladdin,  d9106, default, dyna_dc4000_state, empty_init, ROT0, "Dyna", "The Aladdin (V1.1A)",       MACHINE_NOT_WORKING )
GAME(  1992, cmast92,    0,        d9106, default, dyna_dc4000_state, empty_init, ROT0, "Dyna", "Cherry Master '92 (V1.2D)", MACHINE_NOT_WORKING )
GAME(  1992, cmast92a,   cmast92,  d9106, default, dyna_dc4000_state, empty_init, ROT0, "Dyna", "Cherry Master '92 (V1.1D)", MACHINE_NOT_WORKING )

// Dyna D9105 PCB
GAME(  1991, eldoraddo,  eldoradd, d9106, default, dyna_dc4000_state, empty_init, ROT0, "Dyna", "El Dorado (V1.1TA)",        MACHINE_NOT_WORKING )
GAME(  1991, eldoraddob, eldoradd, d9106, default, dyna_dc4000_state, empty_init, ROT0, "Dyna", "El Dorado (V2.0D)",         MACHINE_NOT_WORKING )
GAME(  1991, eldoraddoc, eldoradd, d9106, default, dyna_dc4000_state, empty_init, ROT0, "Dyna", "El Dorado (V1.1J)",         MACHINE_NOT_WORKING )
GAME(  1991, eldoraddod, eldoradd, d9106, default, dyna_dc4000_state, empty_init, ROT0, "Dyna", "El Dorado (V1.1U)",         MACHINE_NOT_WORKING )
