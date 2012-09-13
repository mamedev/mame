/******************************************************************
 *
 *  Fairchild Channel F driver
 *
 *  Juergen Buchmueller
 *  Frank Palazzolo
 *  Sean Riddle
 *
 *  Fredric "e5frog" Blaoholtz, added support large cartridges
 *    also spanning from $3000 to $FFFF. Added clones
 *
 ******************************************************************/

#include "includes/channelf.h"

#ifndef VERBOSE
#define VERBOSE 0
#endif

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

#define MASTER_CLOCK_PAL	2000000  /* PAL unit has a separate crystal at 4.000 MHz */
#define PAL_VBLANK_TIME     4623


/* The F8 has latches on its port pins
 * These mimic's their behavior
 * [0]=port0, [1]=port1, [2]=port4, [3]=port5
 *
 * Note: this should really be moved into f8.c,
 * but it's complicated by possible peripheral usage.
 *
 * If the read/write operation is going external to the F3850
 * (or F3853, etc.), then the latching applies.  However, relaying the
 * port read/writes from the F3850 to a peripheral like the F3853
 * should not be latched in this way. (See mk1 driver)
 *
 * The f8 cannot determine how its ports are mapped at runtime,
 * so it can't easily decide to latch or not.
 *
 * ...so it stays here for now.
 */

UINT8 channelf_state::port_read_with_latch(UINT8 ext, UINT8 latch_state)
{
	return (~ext | latch_state);
}

READ8_MEMBER( channelf_state::channelf_port_0_r )
{
	return port_read_with_latch(ioport("PANEL")->read(), m_latch[0]);
}

READ8_MEMBER( channelf_state::channelf_port_1_r )
{
	UINT8 ext_value;

	if ((m_latch[0] & 0x40) == 0)
		ext_value = ioport("RIGHT_C")->read();
	else
		ext_value = 0xc0 | ioport("RIGHT_C")->read();

	return port_read_with_latch(ext_value,m_latch[1]);
}

READ8_MEMBER( channelf_state::channelf_port_4_r )
{
	UINT8 ext_value;

	if ((m_latch[0] & 0x40) == 0)
		ext_value = ioport("LEFT_C")->read();
	else
		ext_value = 0xff;

	return port_read_with_latch(ext_value,m_latch[2]);
}

READ8_MEMBER( channelf_state::channelf_port_5_r )
{
	return port_read_with_latch(0xff, m_latch[3]);
}

READ8_MEMBER( channelf_state::channelf_2102A_r )	/* SKR */
{
	UINT8 pdata;

	if (m_r2102.r_w==0)
	{
		m_r2102.addr=(m_r2102.a[0]&1)+((m_r2102.a[1]<<1)&2)+((m_r2102.a[2]<<2)&4)+((m_r2102.a[3]<<3)&8)+((m_r2102.a[4]<<4)&16)+((m_r2102.a[5]<<5)&32)+((m_r2102.a[6]<<6)&64)+((m_r2102.a[7]<<7)&128)+((m_r2102.a[8]<<8)&256)+((m_r2102.a[9]<<9)&512);
		m_r2102.d=m_r2102.ram[m_r2102.addr]&1;
		pdata=m_latch[4]&0x7f;
		pdata|=(m_r2102.d<<7);
		LOG(("rhA: addr=%d, d=%d, r_w=%d, ram[%d]=%d,  a[9]=%d, a[8]=%d, a[7]=%d, a[6]=%d, a[5]=%d, a[4]=%d, a[3]=%d, a[2]=%d, a[1]=%d, a[0]=%d\n",m_r2102.addr,m_r2102.d,m_r2102.r_w,m_r2102.addr,m_r2102.ram[m_r2102.addr],m_r2102.a[9],m_r2102.a[8],m_r2102.a[7],m_r2102.a[6],m_r2102.a[5],m_r2102.a[4],m_r2102.a[3],m_r2102.a[2],m_r2102.a[1],m_r2102.a[0]));
		return port_read_with_latch(0xff,pdata);
	}
	else
		LOG(("rhA: r_w=%d\n",m_r2102.r_w));

	return port_read_with_latch(0xff, m_latch[4]);
}

READ8_MEMBER( channelf_state::channelf_2102B_r )  /* SKR */
{
	LOG(("rhB\n"));
	return port_read_with_latch(0xff, m_latch[5]);
}

WRITE8_MEMBER( channelf_state::channelf_port_0_w )
{
	int offs;

	m_latch[0] = data;

	if (data & 0x20)
	{
		offs = m_row_reg*128+m_col_reg;
		m_p_videoram[offs] = m_val_reg;
	}
}

WRITE8_MEMBER( channelf_state::channelf_port_1_w )
{
	m_latch[1] = data;
	m_val_reg = ((data ^ 0xff) >> 6) & 0x03;
}

WRITE8_MEMBER( channelf_state::channelf_port_4_w )
{
	m_latch[2] = data;
	m_col_reg = (data | 0x80) ^ 0xff;
}

WRITE8_MEMBER( channelf_state::channelf_port_5_w )
{
	m_latch[3] = data;
	channelf_sound_w(machine().device("custom"), (data>>6)&3);
	m_row_reg = (data | 0xc0) ^ 0xff;
}

WRITE8_MEMBER( channelf_state::channelf_2102A_w )  /* SKR */
{
	m_latch[4]=data;
	m_r2102.a[2]=(data>>2)&1;
	m_r2102.a[3]=(data>>1)&1;
	m_r2102.r_w=data&1;
	m_r2102.addr=(m_r2102.a[0]&1)+((m_r2102.a[1]<<1)&2)+((m_r2102.a[2]<<2)&4)+((m_r2102.a[3]<<3)&8)+((m_r2102.a[4]<<4)&16)+((m_r2102.a[5]<<5)&32)+((m_r2102.a[6]<<6)&64)+((m_r2102.a[7]<<7)&128)+((m_r2102.a[8]<<8)&256)+((m_r2102.a[9]<<9)&512);
	m_r2102.d=(data>>3)&1;
	if(m_r2102.r_w==1)
		m_r2102.ram[m_r2102.addr]=m_r2102.d;
	LOG(("whA: data=%d, addr=%d, d=%d, r_w=%d, ram[%d]=%d\n",data,m_r2102.addr,m_r2102.d,m_r2102.r_w,m_r2102.addr,m_r2102.ram[m_r2102.addr]));
}

WRITE8_MEMBER( channelf_state::channelf_2102B_w )  /* SKR */
{
	m_latch[5]=data;
	m_r2102.a[9]=(data>>7)&1;
	m_r2102.a[8]=(data>>6)&1;
	m_r2102.a[7]=(data>>5)&1;
	m_r2102.a[1]=(data>>4)&1;
	m_r2102.a[6]=(data>>3)&1;
	m_r2102.a[5]=(data>>2)&1;
	m_r2102.a[4]=(data>>1)&1;
	m_r2102.a[0]=data&1;
	LOG(("whB: data=%d, a[9]=%d,a[8]=%d,a[0]=%d\n",data,m_r2102.a[9],m_r2102.a[8],m_r2102.a[0]));
}

static ADDRESS_MAP_START( channelf_map, AS_PROGRAM, 8, channelf_state )
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x0800, 0x27ff) AM_ROM /* Cartridge Data */
	AM_RANGE(0x2800, 0x2fff) AM_RAM /* Schach RAM */
	AM_RANGE(0x3000, 0xffff) AM_ROM /* Cartridge Data continued */
ADDRESS_MAP_END

static ADDRESS_MAP_START( channelf_io, AS_IO, 8, channelf_state )
	AM_RANGE(0x00, 0x00) AM_READWRITE(channelf_port_0_r, channelf_port_0_w) /* Front panel switches */
	AM_RANGE(0x01, 0x01) AM_READWRITE(channelf_port_1_r, channelf_port_1_w) /* Right controller     */
	AM_RANGE(0x04, 0x04) AM_READWRITE(channelf_port_4_r, channelf_port_4_w) /* Left controller      */
	AM_RANGE(0x05, 0x05) AM_READWRITE(channelf_port_5_r, channelf_port_5_w)

	AM_RANGE(0x20, 0x20) AM_READWRITE(channelf_2102A_r, channelf_2102A_w) /* SKR 2102 control and addr for cart 18 */
	AM_RANGE(0x21, 0x21) AM_READWRITE(channelf_2102B_r, channelf_2102B_w) /* SKR 2102 addr */
	AM_RANGE(0x24, 0x24) AM_READWRITE(channelf_2102A_r, channelf_2102A_w) /* SKR 2102 control and addr for cart 10 */
	AM_RANGE(0x25, 0x25) AM_READWRITE(channelf_2102B_r, channelf_2102B_w) /* SKR 2102 addr */
ADDRESS_MAP_END



static INPUT_PORTS_START( channelf )
	PORT_START("PANEL") /* Front panel buttons */
	PORT_BIT ( 0x01, IP_ACTIVE_HIGH, IPT_START )	/* TIME  (1) */
	PORT_BIT ( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON5 )	/* HOLD  (2) */
	PORT_BIT ( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON6 )	/* MODE  (3) */
	PORT_BIT ( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON7 )	/* START (4) */
	PORT_BIT ( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("RIGHT_C") /* Right controller */
	PORT_BIT ( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1)
	PORT_BIT ( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_PLAYER(1)
	PORT_BIT ( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_PLAYER(1)
	PORT_BIT ( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_PLAYER(1)
	PORT_BIT ( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4) /* C-CLOCKWISE */ PORT_PLAYER(1)
	PORT_BIT ( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3) /* CLOCKWISE   */ PORT_PLAYER(1)
	PORT_BIT ( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2) /* PULL UP     */ PORT_PLAYER(1)
	PORT_BIT ( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1) /* PUSH DOWN   */ PORT_PLAYER(1)

	PORT_START("LEFT_C") /* Left controller */
	PORT_BIT ( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)
	PORT_BIT ( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT) PORT_PLAYER(2)
	PORT_BIT ( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN) PORT_PLAYER(2)
	PORT_BIT ( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP) PORT_PLAYER(2)
	PORT_BIT ( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4) /* C-CLOCKWISE */ PORT_PLAYER(2)
	PORT_BIT ( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON3) /* CLOCKWISE   */ PORT_PLAYER(2)
	PORT_BIT ( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON2) /* PULL UP     */ PORT_PLAYER(2)
	PORT_BIT ( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON1) /* PUSH DOWN   */ PORT_PLAYER(2)
INPUT_PORTS_END



static DEVICE_IMAGE_LOAD( channelf_cart )
{
	UINT32 size;

	if (image.software_entry() == NULL)
	{
		size = image.length();

		if (size > 0xf800)
		{
			image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
			return IMAGE_INIT_FAIL;
		}

		if (image.fread( image.device().machine().root_device().memregion("maincpu")->base() + 0x0800, size) != size)
		{
			image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unable to fully read from file");
			return IMAGE_INIT_FAIL;
		}

	}
	else
	{
		size = image.get_software_region_length("rom");
		memcpy(image.device().machine().root_device().memregion("maincpu")->base() + 0x0800, image.get_software_region("rom"), size);
	}

	return IMAGE_INIT_PASS;
}

static MACHINE_CONFIG_FRAGMENT( channelf_cart )
	/* cartridge */
	MCFG_CARTSLOT_ADD("cart")
	MCFG_CARTSLOT_EXTENSION_LIST("bin,chf")
	MCFG_CARTSLOT_INTERFACE("channelf_cart")
	MCFG_CARTSLOT_LOAD(channelf_cart)

	/* Software lists */
	MCFG_SOFTWARE_LIST_ADD("cart_list","channelf")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( channelf, channelf_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", F8, 3579545/2)        /* Colorburst/2 */
	MCFG_CPU_PROGRAM_MAP(channelf_map)
	MCFG_CPU_IO_MAP(channelf_io)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(128, 64)
	MCFG_SCREEN_VISIBLE_AREA(4, 112 - 7, 4, 64 - 3)
	MCFG_SCREEN_UPDATE_STATIC( channelf )

	MCFG_PALETTE_LENGTH(8)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("custom", CHANNELF, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_FRAGMENT_ADD( channelf_cart )
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( sabavdpl, channelf_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", F8, MASTER_CLOCK_PAL)        /* PAL speed */
	MCFG_CPU_PROGRAM_MAP(channelf_map)
	MCFG_CPU_IO_MAP(channelf_io)
	MCFG_QUANTUM_TIME(attotime::from_hz(50))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(PAL_VBLANK_TIME)) /* approximate */
	MCFG_SCREEN_SIZE(128, 64)
	MCFG_SCREEN_VISIBLE_AREA(4, 112 - 7, 4, 64 - 3)
	MCFG_SCREEN_UPDATE_STATIC( channelf )

	MCFG_PALETTE_LENGTH(8)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("custom", CHANNELF, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_FRAGMENT_ADD( channelf_cart )
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( channlf2, channelf_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", F8, 3579545/2)        /* Colorburst / 2 */
	MCFG_CPU_PROGRAM_MAP(channelf_map)
	MCFG_CPU_IO_MAP(channelf_io)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(128, 64)
	MCFG_SCREEN_VISIBLE_AREA(4, 112 - 7, 4, 64 - 3)
	MCFG_SCREEN_UPDATE_STATIC( channelf )

	MCFG_PALETTE_LENGTH(8)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("custom", CHANNELF, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_FRAGMENT_ADD( channelf_cart )
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( sabavpl2, channelf_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", F8, MASTER_CLOCK_PAL)        /* PAL speed */
	MCFG_CPU_PROGRAM_MAP(channelf_map)
	MCFG_CPU_IO_MAP(channelf_io)
	MCFG_QUANTUM_TIME(attotime::from_hz(50))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(PAL_VBLANK_TIME)) /* not accurate */
	MCFG_SCREEN_SIZE(128, 64)
	MCFG_SCREEN_VISIBLE_AREA(4, 112 - 7, 4, 64 - 3)
	MCFG_SCREEN_UPDATE_STATIC( channelf )

	MCFG_PALETTE_LENGTH(8)


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("custom", CHANNELF, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_FRAGMENT_ADD( channelf_cart )
MACHINE_CONFIG_END

ROM_START( channelf )
	ROM_REGION(0x10000,"maincpu",0)
	ROM_SYSTEM_BIOS( 0, "sl90025", "Luxor Video Entertainment System" )
	ROMX_LOAD("sl90025.rom",  0x0000, 0x0400, CRC(015c1e38) SHA1(759e2ed31fbde4a2d8daf8b9f3e0dffebc90dae2), ROM_BIOS(1))
	ROM_SYSTEM_BIOS( 1, "sl31253", "Channel F" )
	ROMX_LOAD("sl31253.rom",  0x0000, 0x0400, CRC(04694ed9) SHA1(81193965a374d77b99b4743d317824b53c3e3c78), ROM_BIOS(2))
	ROM_LOAD("sl31254.rom",   0x0400, 0x0400, CRC(9c047ba3) SHA1(8f70d1b74483ba3a37e86cf16c849d601a8c3d2c))
	ROM_REGION(0x2000, "vram", ROMREGION_ERASE00)
ROM_END

#define rom_sabavdpl rom_channelf
#define rom_luxorves rom_channelf
#define rom_channlf2 rom_channelf
#define rom_sabavdpl rom_channelf
#define rom_sabavpl2 rom_channelf
#define rom_luxorvec rom_channelf
#define rom_itttelma rom_channelf
#define rom_ingtelma rom_channelf

/***************************************************************************

  Game driver(s)

***************************************************************************/

/*    YEAR  NAME        PARENT  COMPAT  MACHINE     INPUT     INIT   COMPANY         FULLNAME        FLAGS */
CONS( 1976, channelf,  0,        0,    channelf,  channelf, driver_device,   0,      "Fairchild",    "Channel F",                            0 )
CONS( 1977, sabavdpl,  channelf, 0,    sabavdpl,  channelf, driver_device,   0,      "SABA",         "SABA Videoplay",                       0 )
CONS( 197?, luxorves,  channelf, 0,    sabavdpl,  channelf, driver_device,   0,      "Luxor",        "Luxor Video Entertainment System",     0 )
CONS( 1978, channlf2,  0, channelf,    channlf2,  channelf, driver_device,   0,      "Fairchild",    "Channel F II",                         0 )
CONS( 1978, sabavpl2,  channlf2, 0,    sabavpl2,  channelf, driver_device,   0,      "SABA",         "SABA Videoplay 2",                     0 )
CONS( 197?, luxorvec,  channlf2, 0,    sabavpl2,  channelf, driver_device,   0,      "Luxor",        "Luxor Video Entertainment Computer",   0 )
CONS( 197?, itttelma,  channlf2, 0,    sabavpl2,  channelf, driver_device,   0,      "ITT",          "ITT Tele-Match Processor",             0 )
CONS( 1978, ingtelma,  channlf2, 0,    sabavpl2,  channelf, driver_device,   0,      "Ingelen",      "Ingelen Tele-Match Processor",         0 )
