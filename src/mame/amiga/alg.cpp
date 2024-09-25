// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/**************************************************************************************

    American Laser Game Hardware

    Amiga 500 + Sony laserdisc player LDP-1450
    (LDP-3300P for Zorton Brothers, LDP-1500 for Marbella Vice)

    TODO:
    - Implement LD comms (thru Amiga SERDAT / SERDATR ports);
    - Why it enables FDC DMA like at all?
      Is the board really capable of reading floppies for some reason?
    - Picmatic games may really belong to a different driver,
      just sharing the Amiga base state;

    Games Supported:

        Mad Dog McCree [3 versions]
        Who Shot Johnny Rock? [2 versions]
        Mad Dog II: The Lost Gold [4 versions]
        Space Pirates [2 versions]
        Gallagher's Gallery [2 versions]
        Crime Patrol [4 versions]
        Crime Patrol 2: Drug Wars [2 versions]
        The Last Bounty Hunter [2 versions]
        Fast Draw Showdown [2 versions]
        Platoon
        Marbella Vice [2 versions]
        Tierras Salvajes
        Zorton Brothers (Los Justicieros) [2 versions]

    Picmatic games had different versions depending on TV system (PAL or NTSC) and
    TV frequency (NTSC 60Hz, PAL 50Hz, PAL 100Hz):

        Game                   System     ROM dumped  LD dumped
        ---------------------  ---------  ----------  ---------
        Marbella Vice          PAL 50Hz   YES         YES (but not added yet)
        Marbella Vice          PAL 100Hz  YES         YES (but not added yet)
        Marbella Vice          NTSC 60Hz  NO          NO
        Tierras Salvajes       PAL 50Hz   NO          NO
        Tierras Salvajes       PAL 100Hz  YES         NO
        Zorton Brothers v1.00  PAL 50Hz   YES         YES (but not added yet)
        Zorton Brothers v1.01  PAL 50Hz   YES         YES (but not added yet)
        Zorton Brothers v1.01  PAL 100Hz  NO          YES (but not added yet)

**************************************************************************************/

#include "emu.h"
#include "amiga.h"
#include "cpu/m68000/m68000.h"
#include "machine/ldp1450.h"
#include "machine/nvram.h"
#include "machine/amigafdc.h"
#include "speaker.h"


namespace {

class alg_state : public amiga_state
{
public:
	alg_state(const machine_config &mconfig, device_type type, const char *tag)
		: amiga_state(mconfig, type, tag)
		, m_laserdisc(*this, "laserdisc")
		, m_gun1x(*this, "GUN1X")
		, m_gun1y(*this, "GUN1Y")
		, m_gun2x(*this, "GUN2X")
		, m_gun2y(*this, "GUN2Y")
		, m_triggers(*this, "TRIGGERS")
	{ }

	ioport_value lightgun_pos_r();
	int lightgun_trigger_r();
	int lightgun_holster_r();

	void init_aplatoon();
	void init_palr3();
	void init_palr1();
	void init_palr6();
	void init_ntsc();
	void init_pal();

	void alg_r2(machine_config &config);
	void picmatic(machine_config &config);
	void alg_r1(machine_config &config);

protected:
	// amiga_state overrides
	virtual void potgo_w(uint16_t data) override;

private:
	required_device<sony_ldp1450_device> m_laserdisc;
	required_ioport m_gun1x;
	required_ioport m_gun1y;
	optional_ioport m_gun2x;
	optional_ioport m_gun2y;
	optional_ioport m_triggers;

	uint16_t m_input_select;

	int get_lightgun_pos(int player, int *x, int *y);

	DECLARE_VIDEO_START(alg);

	void a500_mem(address_map &map) ATTR_COLD;
	void main_map_picmatic(address_map &map) ATTR_COLD;
	void main_map_r1(address_map &map) ATTR_COLD;
	void main_map_r2(address_map &map) ATTR_COLD;
	void overlay_512kb_map(address_map &map) ATTR_COLD;
};



/*************************************
 *
 *  Lightgun reading
 *
 *************************************/

int alg_state::get_lightgun_pos(int player, int *x, int *y)
{
	const rectangle &visarea = m_screen->visible_area();

	int xpos = (player == 0) ? m_gun1x->read() : m_gun2x.read_safe(0xffffffff);
	int ypos = (player == 0) ? m_gun1y->read() : m_gun2y.read_safe(0xffffffff);

	if (xpos == -1 || ypos == -1)
		return false;

	*x = visarea.left() + xpos * visarea.width() / 255;
	*y = visarea.top() + ypos * visarea.height() / 255;
	return true;
}



/*************************************
 *
 *  Video startup
 *
 *************************************/

VIDEO_START_MEMBER(alg_state,alg)
{
	// Standard video start
	VIDEO_START_CALL_MEMBER(amiga);

	// Configure pen 4096 as transparent in the renderer and use it for the genlock color
	m_palette->set_pen_color(4096, rgb_t(0,0,0,0));
	set_genlock_color(4096);
}



/*************************************
 *
 *  I/O ports
 *
 *************************************/

void alg_state::potgo_w(uint16_t data)
{
	// bit 15 controls whether pin 9 is input/output
	// bit 14 controls the value, which selects which player's controls to read
	m_input_select = (data & 0x8000) ? ((data >> 14) & 1) : 0;
}


ioport_value alg_state::lightgun_pos_r()
{
	int x = 0, y = 0;

	// Get the position based on the input select
	get_lightgun_pos(m_input_select, &x, &y);
	return (y << 8) | (x >> 2);
}


int alg_state::lightgun_trigger_r()
{
	// Read the trigger control based on the input select
	return (m_triggers->read() >> m_input_select) & 1;
}


int alg_state::lightgun_holster_r()
{
	// Read the holster control based on the input select
	return (m_triggers->read() >> (2 + m_input_select)) & 1;
}



/*************************************
 *
 *  Memory map
 *
 *************************************/

void alg_state::overlay_512kb_map(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x07ffff).mirror(0x180000).ram().share("chip_ram");
	map(0x200000, 0x27ffff).rom().region("kickstart", 0);
}

void alg_state::a500_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x000000, 0x1fffff).m(m_overlay, FUNC(address_map_bank_device::amap16));
	map(0xa00000, 0xbfffff).rw(FUNC(alg_state::cia_r), FUNC(alg_state::cia_w));
	map(0xc00000, 0xd7ffff).m(m_chipset, FUNC(address_map_bank_device::amap16));
	map(0xd80000, 0xddffff).noprw();
	map(0xde0000, 0xdeffff).m(m_chipset, FUNC(address_map_bank_device::amap16));
	map(0xdf0000, 0xdfffff).m(m_chipset, FUNC(address_map_bank_device::amap16));
	map(0xe00000, 0xe7ffff).nopw().r(FUNC(alg_state::rom_mirror_r));
	map(0xe80000, 0xefffff).noprw(); // autoconfig space (installed by devices)
	map(0xf80000, 0xffffff).rom().region("kickstart", 0);
}

void alg_state::main_map_r1(address_map &map)
{
	a500_mem(map);
	map(0xf00000, 0xf1ffff).rom().region("game_program", 0);           // Custom ROM
	map(0xf54000, 0xf55fff).ram().share("nvram");
}

void alg_state::main_map_r2(address_map &map)
{
	a500_mem(map);
	map(0xf00000, 0xf3ffff).rom().region("game_program", 0);           // Custom ROM
	map(0xf7c000, 0xf7dfff).ram().share("nvram");
}

void alg_state::main_map_picmatic(address_map &map)
{
	a500_mem(map);
	map(0xf00000, 0xf1ffff).rom().region("game_program", 0);           // Custom ROM
	map(0xf40000, 0xf41fff).ram().share("nvram");
//  TODO: both games accesses 0xf20000-0xf7ffff, what for?
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( alg )
	PORT_START("joy_0_dat")   // Read by Amiga core
	PORT_BIT( 0x0303, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(alg_state, amiga_joystick_convert<0>)
	PORT_BIT( 0xfcfc, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("joy_1_dat")   // Read by Amiga core
	PORT_BIT( 0x0303, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(alg_state, amiga_joystick_convert<1>)
	PORT_BIT( 0xfcfc, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("potgo")     // Read by Amiga core
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xaaff, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("HVPOS")     // Read by Amiga core
	PORT_BIT( 0x1ffff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(alg_state, lightgun_pos_r)

	PORT_START("FIRE")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("p1_joy")     // Referenced by JOY0DAT
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_HIGH )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN2 )

	PORT_START("p2_joy")     // Referenced by JOY1DAT
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("GUN1X")     // Referenced by lightgun_pos_r
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("GUN1Y")     // Referenced by lightgun_pos_r
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(1)
INPUT_PORTS_END


static INPUT_PORTS_START( alg_2p )
	PORT_INCLUDE(alg)

	PORT_MODIFY("potgo")
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(alg_state, lightgun_trigger_r)

	PORT_MODIFY("FIRE")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("p2_joy")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(alg_state, lightgun_holster_r)

	PORT_START("GUN2X")     // Referenced by lightgun_pos_r
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_SENSITIVITY(50) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("GUN2Y")     // Referenced by lightgun_pos_r
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, 1.0, 0.0, 0) PORT_SENSITIVITY(70) PORT_KEYDELTA(10) PORT_PLAYER(2)

	PORT_START("TRIGGERS")  // Referenced by lightgun_trigger_r and lightgun_holster_r
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void alg_state::alg_r1(machine_config &config)
{
	// Basic machine hardware
	M68000(config, m_maincpu, amiga_state::CLK_7M_NTSC);
	m_maincpu->set_addrmap(AS_PROGRAM, &alg_state::main_map_r1);
	m_maincpu->reset_cb().set(FUNC(amiga_state::m68k_reset));

	ADDRESS_MAP_BANK(config, m_overlay).set_map(&alg_state::overlay_512kb_map).set_options(ENDIANNESS_BIG, 16, 22, 0x200000);
	ADDRESS_MAP_BANK(config, m_chipset).set_map(&alg_state::ocs_map).set_options(ENDIANNESS_BIG, 16, 9, 0x200);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// Video hardware
	ntsc_video(config);

	AMIGA_COPPER(config, m_copper, amiga_state::CLK_7M_NTSC);
	m_copper->set_host_cpu_tag(m_maincpu);
	m_copper->mem_read_cb().set(FUNC(amiga_state::chip_ram_r));
	m_copper->set_ecs_mode(false);

	SONY_LDP1450(config, m_laserdisc, 9600);
	m_laserdisc->set_screen("screen");
	m_laserdisc->set_overlay(512*2, 262, FUNC(amiga_state::screen_update));
	m_laserdisc->set_overlay_clip((129-8)*2, (449+8-1)*2, 44-8, 244+8-1);

	// FIXME: should be 4096
	PALETTE(config, m_palette, FUNC(alg_state::amiga_palette), 4097);

	MCFG_VIDEO_START_OVERRIDE(alg_state,alg)

	// Sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	PAULA_8364(config, m_paula, amiga_state::CLK_C1_NTSC);
	m_paula->add_route(0, "lspeaker", 0.25);
	m_paula->add_route(1, "rspeaker", 0.25);
	m_paula->add_route(2, "rspeaker", 0.25);
	m_paula->add_route(3, "lspeaker", 0.25);
	m_paula->mem_read_cb().set(FUNC(amiga_state::chip_ram_r));
	m_paula->int_cb().set(FUNC(amiga_state::paula_int_w));

	m_laserdisc->add_route(0, "lspeaker", 1.0);
	m_laserdisc->add_route(1, "rspeaker", 1.0);

	// cia
	MOS8520(config, m_cia_0, amiga_state::CLK_E_NTSC);
	m_cia_0->irq_wr_callback().set(FUNC(amiga_state::cia_0_irq));
	m_cia_0->pa_rd_callback().set_ioport("FIRE");
	m_cia_0->pa_wr_callback().set(FUNC(amiga_state::cia_0_port_a_write));

	MOS8520(config, m_cia_1, amiga_state::CLK_E_NTSC);
	m_cia_1->irq_wr_callback().set(FUNC(amiga_state::cia_1_irq));

	AMIGA_FDC(config, m_fdc, amiga_state::CLK_7M_NTSC);
	m_fdc->index_callback().set("cia_1", FUNC(mos8520_device::flag_w));
	m_fdc->read_dma_callback().set(FUNC(amiga_state::chip_ram_r));
	m_fdc->write_dma_callback().set(FUNC(amiga_state::chip_ram_w));
	m_fdc->dskblk_callback().set(FUNC(amiga_state::fdc_dskblk_w));
	m_fdc->dsksyn_callback().set(FUNC(amiga_state::fdc_dsksyn_w));
}


void alg_state::alg_r2(machine_config &config)
{
	alg_r1(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &alg_state::main_map_r2);
}


void alg_state::picmatic(machine_config &config)
{
	alg_r1(config);
	// Adjust for PAL specs
	m_maincpu->set_clock(amiga_state::CLK_7M_PAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &alg_state::main_map_picmatic);

	config.device_remove("screen");
	pal_video(config);

	m_paula->set_clock(amiga_state::CLK_C1_PAL);
	m_cia_0->set_clock(amiga_state::CLK_E_PAL);
	m_cia_1->set_clock(amiga_state::CLK_E_PAL);
	m_fdc->set_clock(amiga_state::CLK_7M_PAL);
}



/*************************************
 *
 *  Per-game decryption
 *
 *************************************/

void alg_state::init_ntsc()
{
	m_agnus_id = AGNUS_NTSC;
	m_denise_id = DENISE;
}

void alg_state::init_pal()
{
	m_agnus_id = AGNUS_PAL;
	m_denise_id = DENISE;
}

void alg_state::init_palr1()
{
	init_ntsc();

	uint32_t length = memregion("game_program")->bytes();
	uint8_t *rom = memregion("game_program")->base();
	std::vector<uint8_t> original(length);

	memcpy(&original[0], rom, length);
	for (uint32_t srcaddr = 0; srcaddr < length; srcaddr++)
	{
		uint32_t dstaddr = srcaddr;
		if (srcaddr & 0x2000) dstaddr ^= 0x1000;
		if (srcaddr & 0x8000) dstaddr ^= 0x4000;
		rom[dstaddr] = original[srcaddr];
	}
}

void alg_state::init_palr3()
{
	init_ntsc();

	uint32_t length = memregion("game_program")->bytes();
	uint8_t *rom = memregion("game_program")->base();
	std::vector<uint8_t> original(length);

	memcpy(&original[0], rom, length);
	for (uint32_t srcaddr = 0; srcaddr < length; srcaddr++)
	{
		uint32_t dstaddr = srcaddr;
		if (srcaddr & 0x2000) dstaddr ^= 0x1000;
		rom[dstaddr] = original[srcaddr];
	}
}

void alg_state::init_palr6()
{
	init_ntsc();

	uint32_t length = memregion("game_program")->bytes();
	uint8_t *rom = memregion("game_program")->base();
	std::vector<uint8_t> original(length);

	memcpy(&original[0], rom, length);
	for (uint32_t srcaddr = 0; srcaddr < length; srcaddr++)
	{
		uint32_t dstaddr = srcaddr;
		if (~srcaddr & 0x2000) dstaddr ^= 0x1000;
		if ( srcaddr & 0x8000) dstaddr ^= 0x4000;
		dstaddr ^= 0x20000;
		rom[dstaddr] = original[srcaddr];
	}
}

void alg_state::init_aplatoon()
{
	init_ntsc();

	uint8_t *rom = memregion("game_program")->base();
	std::vector<uint8_t> buffer(0x40000);

	memcpy(&buffer[0], rom, 0x40000);

	for (int i = 0; i < 0x40000; i++) // preliminary, believed ok but to be verified when the driver is fleshed out
		rom[i] = buffer[bitswap<24>(i, 23, 22, 21, 20, 19, 18, 17, 13, 14, 15, 16, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)];
}



/*************************************
 *
 *  BIOS definitions
 *
 *************************************/

#define ALG_BIOS \
	ROM_REGION16_BE(0x80000, "kickstart", 0) \
	ROM_SYSTEM_BIOS(0, "kick13",  "Kickstart 1.3 (34.5)") \
	ROMX_LOAD("315093-02.u2", 0x00000, 0x40000, CRC(c4f0f55f) SHA1(891e9a547772fe0c6c19b610baf8bc4ea7fcb785), ROM_GROUPWORD | ROM_BIOS(0)) \
	ROM_COPY("kickstart", 0x000000, 0x040000, 0x040000)



/*************************************
*
*  ROM definitions
*
*************************************/

// BIOS
ROM_START( alg_bios )
	ALG_BIOS

	ROM_REGION16_BE( 0x20000, "game_program", ROMREGION_ERASEFF )
ROM_END


// Rev. A board
// PAL R1
ROM_START( maddoga )
	ALG_BIOS

	ROM_REGION16_BE( 0x20000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "maddog_01.dat", 0x000001, 0x10000, CRC(04572557) SHA1(3dfe2ce94ced8701a3e73ed5869b6fbe1c8b3286) )
	ROM_LOAD16_BYTE( "maddog_02.dat", 0x000000, 0x10000, CRC(f64014ec) SHA1(d343a2cb5d8992153b8c916f39b11d3db736543d))

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "maddoga", 0, NO_DUMP )
ROM_END


// PAL R3
ROM_START( wsjr )  // 1.6
	ALG_BIOS

	ROM_REGION16_BE( 0x20000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "johnny_01.bin", 0x000001, 0x10000, CRC(edde1745) SHA1(573b79f8808fedaabf3b762350a915792d26c1bc) )
	ROM_LOAD16_BYTE( "johnny_02.bin", 0x000000, 0x10000, CRC(046569b3) SHA1(efe5a8b2be1c555695f2a91c88951d3545f1b915) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "wsjr", 0, NO_DUMP )
ROM_END

ROM_START( wsjr_15 )  // 1.5
	ALG_BIOS

	ROM_REGION16_BE( 0x20000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "wsjr151.bin", 0x000001, 0x10000, CRC(9beeb1d7) SHA1(3fe0265e5d36103d3d9557d75e5e3728e0b30da7) )
	ROM_LOAD16_BYTE( "wsjr152.bin", 0x000000, 0x10000, CRC(8ab626dd) SHA1(e45561f77fc279b71dc1dd2e15a0870cb5c1cd89) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "wsjr", 0, NO_DUMP )
ROM_END


// REV.B
ROM_START( maddog )
	ALG_BIOS

	ROM_REGION16_BE( 0x40000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "md_2.03_1.bin", 0x000001, 0x20000, CRC(6f5b8f2d) SHA1(bbf32bb27a998d53744411d75efdbdb730855809) )
	ROM_LOAD16_BYTE( "md_2.03_2.bin", 0x000000, 0x20000, CRC(a50d3c04) SHA1(4cf100fdb5b2f2236539fd0ec33b3db19c64a6b8) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "maddog", 0, NO_DUMP )
ROM_END


ROM_START( maddog_202 )
	ALG_BIOS

	ROM_REGION16_BE( 0x40000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "md_2.02_u1.bin", 0x000001, 0x20000, CRC(a49890d1) SHA1(148f78fb426f5b912e8c3836a149bfcb966da477) )
	ROM_LOAD16_BYTE( "md_2.02_u2.bin", 0x000000, 0x20000, CRC(f46e1242) SHA1(2960bc1800b22eea50036ae43fd3cb2ab7dcf8a4) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "maddog", 0, NO_DUMP )
ROM_END

ROM_START( maddog2 )
	ALG_BIOS

	ROM_REGION16_BE( 0x40000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "md2_01_v.2.04.bin", 0x000001, 0x20000, CRC(0e1227f4) SHA1(bfd9081bb7d2bcbb77357839f292ce6136e9b228) )
	ROM_LOAD16_BYTE( "md2_02_v.2.04.bin", 0x000000, 0x20000, CRC(361bd99c) SHA1(5de6ef38e334e19f509227de7880306ac984ec23) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "maddog2", 0, NO_DUMP )
ROM_END

ROM_START( maddog2_202 )
	ALG_BIOS

	ROM_REGION16_BE( 0x40000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "md2_01.bin", 0x000001, 0x20000, CRC(4092227f) SHA1(6e5393aa5e64b59887260f483c50960084de7bd1) )
	ROM_LOAD16_BYTE( "md2_02.bin", 0x000000, 0x20000, CRC(addffa51) SHA1(665e9d93ddfa6b2ea5d006b41bf7eac3294244cc) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "maddog2", 0, NO_DUMP )
ROM_END

ROM_START( maddog2_110 )
	ALG_BIOS

	ROM_REGION16_BE( 0x40000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "md2_1.10_u1.bin", 0x000001, 0x20000, CRC(61808612) SHA1(1a0a301e79585a81e6cf46737068970fb8a205fa) )
	ROM_LOAD16_BYTE( "md2_1.10_u2.bin", 0x000000, 0x20000, CRC(0e113b2c) SHA1(739d777f3cb92fbc730c6c1b664d874121d191b1) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "maddog2", 0, NO_DUMP )
ROM_END

ROM_START( maddog2_100 )
	ALG_BIOS

	ROM_REGION16_BE( 0x40000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "md2_1.0_1.bin", 0x000001, 0x20000, CRC(97272a1d) SHA1(109014647c491f019ffb21091c7d0b89e1755b75) )
	ROM_LOAD16_BYTE( "md2_1.0_2.bin", 0x000000, 0x20000, CRC(0ce8db97) SHA1(dd4c09db59bb8c6caba935b1b28babe28ba8516b) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "maddog2", 0, NO_DUMP )
ROM_END


ROM_START( spacepir )
	ALG_BIOS

	ROM_REGION16_BE( 0x40000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "sp_02.dat", 0x000001, 0x20000, CRC(10d162a2) SHA1(26833d5be1057be8639c00a7be18be33404ea751) )
	ROM_LOAD16_BYTE( "sp_01.dat", 0x000000, 0x20000, CRC(c0975188) SHA1(fd7643dc972e7861249ab7e76199975984888ae4) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "spacepir", 0, NO_DUMP )
ROM_END


ROM_START( spacepir_14 )
	ALG_BIOS

	ROM_REGION16_BE( 0x40000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "sp_14_u1.bin", 0x000001, 0x20000, CRC(30390ab0) SHA1(80fa14d881902258398225bdfd71ed5e9e2d6c91) )
	ROM_LOAD16_BYTE( "sp_14_u2.bin", 0x000000, 0x20000, CRC(4102988c) SHA1(969d4668be50990c7debf9ed4e8b8c6e422acdf5) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "spacepir", 0, NO_DUMP )
ROM_END

ROM_START( gallgall )
	ALG_BIOS

	ROM_REGION16_BE( 0x40000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "gg_1.dat", 0x000001, 0x20000, CRC(3793b211) SHA1(dccb1d9c5e2d6a4d249426ae6348e9fc9b72e665)  )
	ROM_LOAD16_BYTE( "gg_2.dat", 0x000000, 0x20000,  CRC(855c9d82) SHA1(96711aaa02f309cacd3e8d8efbe95cfc811aba96) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "gallgall", 0, NO_DUMP )
ROM_END

ROM_START( gallgall_21 )
	ALG_BIOS

	ROM_REGION16_BE( 0x40000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "gg_21_rom1.bin", 0x000001, 0x20000, CRC(4109f39e) SHA1(42d06de42c56f21e4899b4c4252baabb51f24fda)  )
	ROM_LOAD16_BYTE( "gg_21_rom2.bin", 0x000000, 0x20000, CRC(70f887e5) SHA1(cd6cedc85bbe67674dfd140fed9018778f8cd8db) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "gallgall", 0, NO_DUMP )
ROM_END

ROM_START( crimepat )
	ALG_BIOS

	ROM_REGION16_BE( 0x40000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "cp_151_u1.bin", 0x000001, 0x20000, CRC(f2270cee) SHA1(1e735373723c3cffb6dd04d5c30c08e757deae61) )
	ROM_LOAD16_BYTE( "cp_151_u2.bin", 0x000000, 0x20000, CRC(aefd6e09) SHA1(351e665e2e6368047a5cc80d0f9d876bca068b6a) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "crimepat", 0, NO_DUMP )
ROM_END

ROM_START( crimepat_14 )
	ALG_BIOS

	ROM_REGION16_BE( 0x40000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "cp02.dat", 0x000001, 0x20000, CRC(a39a8b50) SHA1(55ca317ef13c3a42f12d68c480e6cc2d4459f6a4) )
	ROM_LOAD16_BYTE( "cp01.dat", 0x000000, 0x20000, CRC(e41fd2e8) SHA1(1cd9875fb4133ba4e3616271975dc736b343f156) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "crimepat", 0, NO_DUMP )
ROM_END

ROM_START( crimepat_12 )
	ALG_BIOS

	ROM_REGION16_BE( 0x40000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "cp_1.20_u1.bin", 0x000001, 0x20000, CRC(814f5777) SHA1(341a1d7b64112af3e8243bdbfec72e7fa85aa903) )
	ROM_LOAD16_BYTE( "cp_1.20_u2.bin", 0x000000, 0x20000, CRC(475e847a) SHA1(82fd160835758cd51ea22f90b08921a40409f94d) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "crimepat", 0, NO_DUMP )
ROM_END

ROM_START( crimepat_10 )
	ALG_BIOS

	ROM_REGION16_BE( 0x40000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "cp_10_rom1.bin", 0x000001, 0x20000, CRC(aac76488) SHA1(fabf4700610480a28dad6e1483e58ec227e465ff) )
	ROM_LOAD16_BYTE( "cp_10_rom2.bin", 0x000000, 0x20000, CRC(6e4614cd) SHA1(0f73f477e13b3f184e9a55f4be95474661c912d5) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "crimepat", 0, NO_DUMP )
ROM_END

ROM_START( crimep2 )
	ALG_BIOS

	ROM_REGION16_BE( 0x40000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "cp2_1.3_1.bin", 0x000001, 0x20000, CRC(e653395d) SHA1(8f6c86d98a52b7d85ae285fd841167cd07979318) )
	ROM_LOAD16_BYTE( "cp2_1.3_2.bin", 0x000000, 0x20000, CRC(dbdaa79a) SHA1(998044909d5c93e3bd1baafefab818fdb7b3f55e) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "crimep2", 0, NO_DUMP )
ROM_END

ROM_START( crimep2_11 )
	ALG_BIOS

	ROM_REGION16_BE( 0x40000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "cp2_1.dat", 0x000001, 0x20000, CRC(47879042) SHA1(8bb6c541e4e8e4508da8d4b93600176a2e7a1f41) )
	ROM_LOAD16_BYTE( "cp2_2.dat", 0x000000, 0x20000, CRC(f4e5251e) SHA1(e0c91343a98193d487c40e7a85f542b2a7a88f03) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "crimep2", 0, NO_DUMP )
ROM_END


ROM_START( lastbh )
	ALG_BIOS

	ROM_REGION16_BE( 0x40000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "lbh_101_u1.bin", 0x000001, 0x20000, CRC(f13b25d2) SHA1(e2f663c23b03592f482ef5e1df87b651937a500d) )
	ROM_LOAD16_BYTE( "lbh_101_u2.bin", 0x000000, 0x20000, CRC(b21c5c42) SHA1(9ac856cdf2c9538cc4ae55f079f337376d3361c0) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "lastbh", 0, NO_DUMP )
ROM_END

ROM_START( lastbh_006 )
	ALG_BIOS

	ROM_REGION16_BE( 0x40000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "bounty_01.bin", 0x000001, 0x20000, CRC(977566b2) SHA1(937e079e992ecb5930b17c1024c326e10962642b) )
	ROM_LOAD16_BYTE( "bounty_02.bin", 0x000000, 0x20000, CRC(2727ef1d) SHA1(f53421390b65c21a7666ff9d0f53ebf2a463d836) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "lastbh", 0, NO_DUMP )
ROM_END

ROM_START( fastdraw )
	ALG_BIOS

	ROM_REGION16_BE( 0x40000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "fd_131_u1.bin", 0x000001, 0x20000, CRC(b7c79ab3) SHA1(6eca1bc9590c22a004fb85901e5c7d41f5b14ee2) )
	ROM_LOAD16_BYTE( "fd_131_u2.bin", 0x000000, 0x20000, CRC(e1ed7982) SHA1(f7562c6e0ce6bf1a9885cc593e08c2509f82bbe1) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "fastdraw", 0, NO_DUMP )
ROM_END

ROM_START( fastdraw_130 )
	ALG_BIOS

	ROM_REGION16_BE( 0x40000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "fast_01.bin", 0x000001, 0x20000, CRC(4c4eb71e) SHA1(3bd487c546b6c80770a5fc880dcb10395ca431a2) )
	ROM_LOAD16_BYTE( "fast_02.bin", 0x000000, 0x20000, CRC(0d76a2da) SHA1(d396371ae1b9b0b6e6bc6f1f85c4b97bfc5dc34d) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "fastdraw", 0, NO_DUMP )
ROM_END

ROM_START( aplatoon )
	ALG_BIOS

	ROM_REGION16_BE( 0x40000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "platoonv4u1.bin", 0x000001, 0x20000, CRC(8b33263e) SHA1(a1df38236321af90b522e2a783984fdf02e4c597) )
	ROM_LOAD16_BYTE( "platoonv4u2.bin", 0x000000, 0x20000, CRC(09a133cf) SHA1(9b3ff63035be8576c88fb284a25c2da5db0d5160) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "platoon", 0, NO_DUMP )
ROM_END


ROM_START( zortonbr_100 )
	ALG_BIOS

	ROM_REGION16_BE( 0x40000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "zb_u2.bin", 0x00001, 0x10000, CRC(938b25cb) SHA1(d0114bbc588dcfce6a469013d0e35afb93e38af5) )
	ROM_LOAD16_BYTE( "zb_u3.bin", 0x00000, 0x10000, CRC(f59cfc4a) SHA1(9fadf7f1e23d6b4e828bf2b3de919d087c690a3f) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "zortonbr", 0, NO_DUMP )
ROM_END

/* zortonbr v1.01
   ROM board labeled "PICMATIC LM6 04-01-92"
   Uses Sony LaserMax LDP-3300P, a separate video encoder PCB with a Motorola MC1378B and a standard A500+ PCB.
   ROM contains text: "Marbella Vice CopyRight 1994 Picmatic S.A. Program chief Brian Meitiner" (but it's Zorton Brothers)
   References to linked libraries: Audio Master IV, AMAS II Version 1.1
   Has a blacklist for high scores:
     "FUCK SHIT CUNT PRICK PENUS BALLS PUTA JODER POLLA PUTO MAMON PICHA COJON TETA TETAS TITS CHULO CULO PENE PIJO LEFA LISTO" */
ROM_START( zortonbr )
	ALG_BIOS

	ROM_REGION16_BE( 0x40000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "zort_1-01_23-3-94_odd.u2",  0x00001, 0x10000, CRC(21e63949) SHA1(0a62ad108f8cfa00dc8f03dea2ff6f1b277e8d5d) )
	ROM_LOAD16_BYTE( "zort_1-01_23-3-94_even.u3", 0x00000, 0x10000, CRC(6a051c6a) SHA1(f8daafab068fef57e47287bd72be860b84e2e75c) )

	ROM_REGION( 0x00800, "unused_nvram", ROMREGION_ERASEFF ) // NVRAM, unused
	ROM_LOAD( "zort_mk48z02b.u13", 0x0000, 0x0800, CRC(45b064a9) SHA1(f446be2b0e3929e182b9f97989c30b3ee308c103) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "zortonbr", 0, NO_DUMP )
ROM_END

// Uses a Sony "LaserMax" LDP-1500
ROM_START( marvice )
	ALG_BIOS

	ROM_REGION16_BE( 0x40000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "mspsl 200 odd 30394.u2",  0x00000, 0x10000, CRC(01c9a503) SHA1(f61ec2cd241b2bf8a982e81e5a18178601f0a0a0) )
	ROM_LOAD16_BYTE( "mspsl 200 even 30394.u3", 0x00001, 0x10000, CRC(78eb6fd6) SHA1(e404818095f03b6e0746620f5ce48c3f7149b8a0) )

	ROM_REGION( 0x800, "unk", ROMREGION_ERASEFF ) // NVRAM contents, should be x4 in size
	ROM_LOAD( "mk48z02b-20.u13", 0x0000, 0x0800, BAD_DUMP CRC(e6079615) SHA1(f528b2a600ab047ad7f87f4dbae65a7e4cd10f8c) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "marvice", 0, NO_DUMP )
ROM_END

ROM_START( marvice100hz )
	ALG_BIOS

	ROM_REGION16_BE( 0x40000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "marbella_vice_100hz_odd.bin",  0x00001, 0x10000, CRC(2c9ebccb) SHA1(4e8dcb8526a8debb70f3f56713903ba8bfbb3ed5) )
	ROM_LOAD16_BYTE( "marbella_vice_100hz_even.bin", 0x00000, 0x10000, CRC(d2b5e5ca) SHA1(de76503961076acad405db9e83bd5334b02bf908) )

	ROM_REGION( 0x800, "unk", ROMREGION_ERASEFF )
	ROM_LOAD( "mk48z02b-20.bin", 0x0000, 0x0800, BAD_DUMP CRC(e6079615) SHA1(f528b2a600ab047ad7f87f4dbae65a7e4cd10f8c) ) // From 'marvice'

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "marvice", 0, NO_DUMP )
ROM_END

/* Tierras Salvajes 100Hz
  PCB silkscreened PICMATIC S.A. 27-11-1992
  ______________________________________________
  |        ________                      ····  |
  |___     |_______|    __________  __________ |
  || |    ___________   F16L8-25CN  |74HC574E| |
  || |    |74HC540P_|                          |
  || |     __________   __________  __________ |
  || |    |__8xDIPS_|  |74HC574E_| |74HC574E_| |
  || |  _____________                          |
  || | |MK48Z02B-20 |   __________  __________ |
  || | |____________|  |74HC4040N| |74HC4040N| |
  || |  _____________   __________  __________ |
  || | | U7 EMPTY   |   F16L8-25CN  U14 EMPTY| |
  || | |____________|   __________  __________ |
  || |  _____________   F16L8-25CN  GAL16V8-20 |
  || | | EPROM ODD  |   __________  ________   |
  || | |____________|   |74HC574E|  Xtal 28.37516 MHz
  || |  _____________                          |
  || | | U6 EMPTY   |   __________  __________ |
  || | |____________|   |74HC541N| |74HC132B1| |
  || |  _____________                          |
  || | | EPROM EVEN |   _________    _________ |
  ||_| |____________|   |       |    |       | |
  |_____________________|       |____|       |_|
                        |_______|    |_______|
*/
ROM_START( tierras100hz )
	ALG_BIOS

	ROM_REGION16_BE( 0x40000, "game_program", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "spilz0d4-odd-27-9-95-salvajes-displays-27c512.u4",  0x00001, 0x10000, CRC(d46dd5e0) SHA1(7d808c6ed88bf9a98cdb468ce71c0952b0b5d7b2) )
	ROM_LOAD16_BYTE( "spilz0d4-even-27-9-95-salvajes-displays-27c512.u5", 0x00000, 0x10000, CRC(730e4696) SHA1(4db0cc7d160de5e70f1d1842fa94adb8ee7f954f) )

	ROM_REGION( 0x00800, "unused_nvram", ROMREGION_ERASEFF ) // NVRAM, unused
	ROM_LOAD( "mk48z02b-20.u8", 0x0000, 0x0800, CRC(18eb5a50) SHA1(9b626978350cda546aa4b890c68ddb28058adf75) )

	ROM_REGION( 0x800, "plds", ROMREGION_ERASEFF )
	ROM_LOAD( "gal16v8.u19", 0x000,  0x117, CRC(f2864878) SHA1(f8f8e7604ece9c6437bc381170d891237fde1372) )
	ROM_LOAD( "f16l8-25cn.1", 0x200, 0x117, NO_DUMP )
	ROM_LOAD( "f16l8-25cn.2", 0x400, 0x117, NO_DUMP )
	ROM_LOAD( "f16l8-25cn.3", 0x600, 0x117, NO_DUMP )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "tierras_salvajes_1995_spanish_english", 0, NO_DUMP )
ROM_END


} // Anonymous namespace



/*************************************
 *
 *  Game drivers
 *
 *************************************/

// BIOS
GAME( 199?, alg_bios,     0,        alg_r1,   alg,    alg_state, init_ntsc,     ROT0,  "American Laser Games", "American Laser Games BIOS", MACHINE_IS_BIOS_ROOT )

// Rev. A board
// PAL R1
GAME( 1990, maddoga,      maddog,   alg_r1,   alg,    alg_state, init_palr1,    ROT0,  "American Laser Games", "Mad Dog McCree (v1C board rev.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// PAL R3
GAME( 1991, wsjr,         alg_bios, alg_r1,   alg,    alg_state, init_palr3,    ROT0,  "American Laser Games", "Who Shot Johnny Rock? (v1.6)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1991, wsjr_15,      wsjr,     alg_r1,   alg,    alg_state, init_palr3,    ROT0,  "American Laser Games", "Who Shot Johnny Rock? (v1.5)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// Rev. B board
// PAL R6

GAME( 1990, maddog,       alg_bios, alg_r2,   alg_2p, alg_state, init_palr6,    ROT0,  "American Laser Games", "Mad Dog McCree (v2.03 board rev.B)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1990, maddog_202,   maddog,   alg_r2,   alg_2p, alg_state, init_palr6,    ROT0,  "American Laser Games", "Mad Dog McCree (v2.02 board rev.B)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )

	// Works OK but uses right player (2) controls only for trigger and holster
GAME( 1992, maddog2,      alg_bios, alg_r2,   alg_2p, alg_state, init_palr6,    ROT0,  "American Laser Games", "Mad Dog II: The Lost Gold (v2.04)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, maddog2_202,  maddog2,  alg_r2,   alg_2p, alg_state, init_palr6,    ROT0,  "American Laser Games", "Mad Dog II: The Lost Gold (v2.02)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, maddog2_110,  maddog2,  alg_r2,   alg_2p, alg_state, init_palr6,    ROT0,  "American Laser Games", "Mad Dog II: The Lost Gold (v1.10)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, maddog2_100,  maddog2,  alg_r2,   alg_2p, alg_state, init_palr6,    ROT0,  "American Laser Games", "Mad Dog II: The Lost Gold (v1.00)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
	// Works ok but uses right player (2) controls only for trigger and holster
GAME( 1992, spacepir,     alg_bios, alg_r2,   alg_2p, alg_state, init_palr6,    ROT0,  "American Laser Games", "Space Pirates (v2.2)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, spacepir_14,  spacepir, alg_r2,   alg_2p, alg_state, init_palr6,    ROT0,  "American Laser Games", "Space Pirates (v1.4)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )

GAME( 1992, gallgall,     alg_bios, alg_r2,   alg_2p, alg_state, init_palr6,    ROT0,  "American Laser Games", "Gallagher's Gallery (v2.2)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1992, gallgall_21,  gallgall, alg_r2,   alg_2p, alg_state, init_palr6,    ROT0,  "American Laser Games", "Gallagher's Gallery (v2.1)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
	// All good, but no holster
GAME( 1993, crimepat,     alg_bios, alg_r2,   alg_2p, alg_state, init_palr6,    ROT0,  "American Laser Games", "Crime Patrol (v1.51)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, crimepat_14,  crimepat, alg_r2,   alg_2p, alg_state, init_palr6,    ROT0,  "American Laser Games", "Crime Patrol (v1.4)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, crimepat_12,  crimepat, alg_r2,   alg_2p, alg_state, init_palr6,    ROT0,  "American Laser Games", "Crime Patrol (v1.2)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, crimepat_10,  crimepat, alg_r2,   alg_2p, alg_state, init_palr6,    ROT0,  "American Laser Games", "Crime Patrol (v1.0)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )

GAME( 1993, crimep2,      alg_bios, alg_r2,   alg_2p, alg_state, init_palr6,    ROT0,  "American Laser Games", "Crime Patrol 2: Drug Wars (v1.3)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1993, crimep2_11,   crimep2,  alg_r2,   alg_2p, alg_state, init_palr6,    ROT0,  "American Laser Games", "Crime Patrol 2: Drug Wars (v1.1)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, lastbh,       alg_bios, alg_r2,   alg_2p, alg_state, init_palr6,    ROT0,  "American Laser Games", "The Last Bounty Hunter (v1.01)",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, lastbh_006,   lastbh,   alg_r2,   alg_2p, alg_state, init_palr6,    ROT0,  "American Laser Games", "The Last Bounty Hunter (v0.06)",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, fastdraw,     alg_bios, alg_r2,   alg_2p, alg_state, init_palr6,    ROT90, "American Laser Games", "Fast Draw Showdown (v1.31)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, fastdraw_130, fastdraw, alg_r2,   alg_2p, alg_state, init_palr6,    ROT90, "American Laser Games", "Fast Draw Showdown (v1.30)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
	// Works OK but uses right player (2) controls only for trigger and holster

// NOVA games on ALG hardware with own address scramble
GAME( 1995, aplatoon,     alg_bios, alg_r2,   alg,    alg_state, init_aplatoon, ROT0,  "Nova?", "Platoon (V.3.1 US)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )

// Picmatic games NTSC or PAL (50 or 100Hz) TV standard, own ROM board
// CHECKME: Zorton Brothers is the English title while Los Justicieros is the Spanish one?
// Eventually confirm if they shares same roms with different LD side depending on language,
// or perhaps the title "Zorton Brothers" is the only one for Arcade LD, while "Los Justicieros" is the later PC release?
GAME( 1993, zortonbr_100, zortonbr, picmatic, alg,    alg_state, init_pal,      ROT0,  "Picmatic", "Zorton Brothers / Los Justicieros (v1.00)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, zortonbr,     alg_bios, picmatic, alg,    alg_state, init_pal,      ROT0,  "Picmatic", "Zorton Brothers / Los Justicieros (v1.01)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, marvice,      alg_bios, picmatic, alg,    alg_state, init_pal,      ROT0,  "Picmatic", "Marbella Vice",                           MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, marvice100hz, alg_bios, picmatic, alg,    alg_state, init_pal,      ROT0,  "Picmatic", "Marbella Vice (100Hz display)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1995, tierras100hz, alg_bios, picmatic, alg,    alg_state, init_pal,      ROT0,  "Picmatic", "Tierras Salvajes (100Hz display)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IMPERFECT_GRAPHICS )
