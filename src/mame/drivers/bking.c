// license:???
// copyright-holders:Ed Mueller, Mike Balfour, Zsolt Vasvari
/***************************************************************************


Birdie King / Birdie King II / Birdie King III Memory Map
---------------------------------------------------------

0000-7fff ROM
8000-83ff Scratch RAM
8400-8fff (Scratch RAM again, address lines AB10, AB11 ignored)
9000-97ff Playfield RAM
a000-bfff Unused?

DIP Locations verified for:
    - bking2

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6805/m6805.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "includes/bking.h"


READ8_MEMBER(bking_state::bking_sndnmi_disable_r)
{
	m_sound_nmi_enable = 0;
	return 0;
}

WRITE8_MEMBER(bking_state::bking_sndnmi_enable_w)
{
	m_sound_nmi_enable = 1;
}

WRITE8_MEMBER(bking_state::bking_soundlatch_w)
{
	int i, code = 0;

	for (i = 0;i < 8;i++)
		if (data & (1 << i))
			code |= 0x80 >> i;

	soundlatch_byte_w(space, offset, code);
	if (m_sound_nmi_enable)
		m_audiocpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

WRITE8_MEMBER(bking_state::bking3_addr_l_w)
{
	m_addr_l = data;
}

WRITE8_MEMBER(bking_state::bking3_addr_h_w)
{
	m_addr_h = data;
}

READ8_MEMBER(bking_state::bking3_extrarom_r)
{
	UINT8 *rom = memregion("user2")->base();
	return rom[m_addr_h * 256 + m_addr_l];
}

WRITE8_MEMBER(bking_state::unk_w)
{
	// 0 = finished reading extra rom
	// 1 = started reading extra rom
}

READ8_MEMBER(bking_state::bking3_ext_check_r)
{
	return 0x31; //no "bad rom.", no "bad ext."
}

static ADDRESS_MAP_START( bking_map, AS_PROGRAM, 8, bking_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM
	AM_RANGE(0x9000, 0x97ff) AM_RAM_WRITE(bking_playfield_w) AM_SHARE("playfield_ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( bking_io_map, AS_IO, 8, bking_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0") AM_WRITE(bking_xld1_w)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1") AM_WRITE(bking_yld1_w)
	AM_RANGE(0x02, 0x02) AM_READ_PORT("DSWA") AM_WRITE(bking_xld2_w)
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSWB") AM_WRITE(bking_yld2_w)
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSWC") AM_WRITE(bking_xld3_w)
	AM_RANGE(0x05, 0x05) AM_READWRITE(bking_input_port_5_r, bking_yld3_w)
	AM_RANGE(0x06, 0x06) AM_READWRITE(bking_input_port_6_r, bking_msk_w)
	AM_RANGE(0x07, 0x07) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x08, 0x08) AM_WRITE(bking_cont1_w)
	AM_RANGE(0x09, 0x09) AM_WRITE(bking_cont2_w)
	AM_RANGE(0x0a, 0x0a) AM_WRITE(bking_cont3_w)
	AM_RANGE(0x0b, 0x0b) AM_WRITE(bking_soundlatch_w)
//  AM_RANGE(0x0c, 0x0c) AM_WRITE(bking_eport2_w)   this is not shown to be connected anywhere
	AM_RANGE(0x0d, 0x0d) AM_WRITE(bking_hitclr_w)
	AM_RANGE(0x07, 0x1f) AM_READ(bking_pos_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bking3_io_map, AS_IO, 8, bking_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_READ_PORT("IN0") AM_WRITE(bking_xld1_w)
	AM_RANGE(0x01, 0x01) AM_READ_PORT("IN1") AM_WRITE(bking_yld1_w)
	AM_RANGE(0x02, 0x02) AM_READ_PORT("DSWA") AM_WRITE(bking_xld2_w)
	AM_RANGE(0x03, 0x03) AM_READ_PORT("DSWB") AM_WRITE(bking_yld2_w)
	AM_RANGE(0x04, 0x04) AM_READ_PORT("DSWC") AM_WRITE(bking_xld3_w)
	AM_RANGE(0x05, 0x05) AM_READWRITE(bking_input_port_5_r, bking_yld3_w)
	AM_RANGE(0x06, 0x06) AM_READWRITE(bking_input_port_6_r, bking_msk_w)
	AM_RANGE(0x07, 0x07) AM_WRITE(watchdog_reset_w)
	AM_RANGE(0x08, 0x08) AM_WRITE(bking_cont1_w)
	AM_RANGE(0x09, 0x09) AM_WRITE(bking_cont2_w)
	AM_RANGE(0x0a, 0x0a) AM_WRITE(bking_cont3_w)
	AM_RANGE(0x0b, 0x0b) AM_WRITE(bking_soundlatch_w)
//  AM_RANGE(0x0c, 0x0c) AM_WRITE(bking_eport2_w)   this is not shown to be connected anywhere
	AM_RANGE(0x0d, 0x0d) AM_WRITE(bking_hitclr_w)
	AM_RANGE(0x07, 0x1f) AM_READ(bking_pos_r)
	AM_RANGE(0x2f, 0x2f) AM_DEVREADWRITE("bmcu", buggychl_mcu_device, buggychl_mcu_r, buggychl_mcu_w)
	AM_RANGE(0x4f, 0x4f) AM_DEVREAD("bmcu", buggychl_mcu_device, buggychl_mcu_status_r) AM_WRITE(unk_w)
	AM_RANGE(0x60, 0x60) AM_READ(bking3_extrarom_r)
	AM_RANGE(0x6f, 0x6f) AM_READWRITE(bking3_ext_check_r, bking3_addr_h_w)
	AM_RANGE(0x8f, 0x8f) AM_WRITE(bking3_addr_l_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( bking_audio_map, AS_PROGRAM, 8, bking_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x2fff) AM_ROM //only bking3
	AM_RANGE(0x4000, 0x43ff) AM_RAM
	AM_RANGE(0x4400, 0x4401) AM_DEVWRITE("ay1", ay8910_device, address_data_w)
	AM_RANGE(0x4401, 0x4401) AM_DEVREAD("ay1", ay8910_device, data_r)
	AM_RANGE(0x4402, 0x4403) AM_DEVWRITE("ay2", ay8910_device, address_data_w)
	AM_RANGE(0x4403, 0x4403) AM_DEVREAD("ay2", ay8910_device, data_r)
	AM_RANGE(0x4800, 0x4800) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x4802, 0x4802) AM_READWRITE(bking_sndnmi_disable_r, bking_sndnmi_enable_w)
	AM_RANGE(0xe000, 0xefff) AM_ROM   /* Space for diagnostic ROM */
ADDRESS_MAP_END

#if 0
READ8_MEMBER(bking_state::bking3_68705_port_a_r)
{
	//printf("port_a_r = %02X\n",(m_port_a_out & m_ddr_a) | (m_port_a_in & ~m_ddr_a));
	return (m_port_a_out & m_ddr_a) | (m_port_a_in & ~m_ddr_a);
}

WRITE8_MEMBER(bking_state::bking3_68705_port_a_w)
{
	m_port_a_out = data;
//  printf("port_a_out = %02X\n",data);
}

WRITE8_MEMBER(bking_state::bking3_68705_ddr_a_w)
{
	m_ddr_a = data;
}

READ8_MEMBER(bking_state::bking3_68705_port_b_r)
{
	return (m_port_b_out & m_ddr_b) | (m_port_b_in & ~m_ddr_b);
}

WRITE8_MEMBER(bking_state::bking3_68705_port_b_w)
{
//  if(data != 0xff)
//      printf("port_b_out = %02X\n",data);

	if (~data & 0x02)
	{
		m_port_a_in = from_main;
		if (main_sent) m_mcu->set_input_line(0, CLEAR_LINE);
		main_sent = 0;
	}

	if (~data & 0x04)
	{
		/* 68705 is writing data for the Z80 */
		from_mcu = m_port_a_out;
		mcu_sent = 1;
	}

	if(data != 0xff && data != 0xfb && data != 0xfd)
		printf("port_b_w = %X\n",data);

	m_port_b_out = data;
}

WRITE8_MEMBER(bking_state::bking3_68705_ddr_b_w)
{
	m_ddr_b = data;
}

READ8_MEMBER(bking_state::bking3_68705_port_c_r)
{
	int port_c_in = 0;
	if (main_sent) port_c_in |= 0x01;
	if (!mcu_sent) port_c_in |= 0x02;
//logerror("%04x: 68705 port C read %02x\n",space.device().safe_pc(),port_c_in);
	return port_c_in;
}
#endif

static INPUT_PORTS_START( bking )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNUSED )

	/* continue inputs are labelled in schematics. */
	/* They are not connected though to any button */

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED ) /* Continue 1 */
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) /* Continue 2 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED ) /* Not Connected */

	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x00, "Holes Awarded" ) PORT_DIPLOCATION("SWA:1")
	PORT_DIPSETTING(    0x00, "Par Play: 0 Holes/Birdie: 1 Hole/Eagle: 2 Holes/Double Eagle: 4 Holes" )
	PORT_DIPSETTING(    0x01, "Par Play: 1 Hole/Birdie: 2 Holes/Eagle: 3 Holes/Double Eagle: 4 Holes" )
	PORT_DIPNAME( 0x02, 0x02, "Holes Awarded For Hole-in-One" ) PORT_DIPLOCATION("SWA:2")
	PORT_DIPSETTING(    0x00, "3 Holes" )
	PORT_DIPSETTING(    0x02, "9 Holes" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR(Free_Play) ) PORT_DIPLOCATION("SWA:3")
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x18, 0x18, "Holes Per Play" ) PORT_DIPLOCATION("SWA:4,5")
	PORT_DIPSETTING(    0x18, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "9" )
	PORT_DIPNAME( 0x20, 0x20, "Self Test" ) PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x40, 0x40, DEF_STR(Flip_Screen) ) PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR(Off))
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPNAME( 0x80, 0x00, DEF_STR(Cabinet) ) PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x00, DEF_STR(Upright) )
	PORT_DIPSETTING(    0x80, DEF_STR(Cocktail) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x0f, 0x00, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SWB:1,2,3,4")
	PORT_DIPSETTING(    0x0f, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_8C ) )
	PORT_DIPNAME( 0xf0, 0x00, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SWB:5,6,7,8")
	PORT_DIPSETTING(    0xf0, DEF_STR( 9C_1C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 8C_1C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 7C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_8C ) )

	PORT_START("DSWC")
	PORT_DIPNAME( 0x01, 0x01, "Appearance of Crow" ) PORT_DIPLOCATION("SWC:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x02, "Crow Flight Pattern" ) PORT_DIPLOCATION("SWC:2,3") /* "The hole from which a crow appears and flys with drawing a circle or a 8-shape in the air." */
	PORT_DIPSETTING(    0x00, "1" ) /* Circle 1,7,11,15,18 / 8-shape 3,5,9,13,17 */
	PORT_DIPSETTING(    0x02, "2" ) /* Circle 1,10,16,18 / 8-shape 4,7,13,17 */
	PORT_DIPSETTING(    0x04, "3" ) /* Circle 4,7,13,18 / 8-shape 1,10,16,17 */
	PORT_DIPSETTING(    0x06, "4" ) /* Circle 3,5,9,13,18 / 8-shape 1,7,11,15,17 */
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SWC:4" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x10, 0x10, "Coin Display" ) PORT_DIPLOCATION("SWC:5")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x10, DEF_STR(On))
	PORT_DIPNAME( 0x20, 0x20, "Year Display" ) PORT_DIPLOCATION("SWC:6")
	PORT_DIPSETTING(    0x00, DEF_STR(Off))
	PORT_DIPSETTING(    0x20, DEF_STR(On))
	PORT_DIPNAME( 0x40, 0x40, "Check" ) PORT_DIPLOCATION("SWC:7")
	PORT_DIPSETTING(    0x00, "Check" )
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPNAME( 0x80, 0x00, "Coin System" ) PORT_DIPLOCATION("SWC:8") /* Default is "1 Way" according to manual */
	PORT_DIPSETTING(    0x00, "1 Way" )
	PORT_DIPSETTING(    0x80, "2 Way" )

	PORT_START("TRACK0_X")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) /* Sensitivity, clip, min, max */

	PORT_START("TRACK0_Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_REVERSE /* Sensitivity, clip, min, max */

	PORT_START("TRACK1_X")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_COCKTAIL /* Sensitivity, clip, min, max */

	PORT_START("TRACK1_Y")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(25) PORT_KEYDELTA(10) PORT_REVERSE PORT_COCKTAIL /* Sensitivity, clip, min, max */
INPUT_PORTS_END

static INPUT_PORTS_START( bking2 )
	PORT_INCLUDE( bking )

	PORT_MODIFY("DSWA")
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SWA:6" ) /* Listed as "Unused" */
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	1024,   /* 1024 characters */
	3,      /* 3 bits per pixel */
	{ 0*1024*8*8, 1*1024*8*8, 2*1024*8*8 }, /* the bitplanes are separated */
	{ 7, 6, 5, 4, 3, 2, 1, 0 }, /* reverse layout */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8     /* every char takes 8 consecutive bytes */
};

static const gfx_layout crowlayout =
{
	16,32,  /* 16*32 characters */
	16,     /* 16 characters */
	2,      /* 2 bits per pixel */
	{ 0, 4 },
	{ 3*32*8+3, 3*32*8+2, 3*32*8+1, 3*32*8+0,
		2*32*8+3, 2*32*8+2, 2*32*8+1, 2*32*8+0,
		32*8+3,   32*8+2,   32*8+1,   32*8+0,
				3,        2,        1,        0 }, /* reverse layout */
	{ 31*8, 30*8, 29*8, 28*8, 27*8, 26*8, 25*8, 24*8,
		23*8, 22*8, 21*8, 20*8, 19*8, 18*8, 17*8, 16*8,
		15*8, 14*8, 13*8, 12*8, 11*8, 10*8,  9*8,  8*8,
		7*8,  6*8,  5*8,  4*8,  3*8,  2*8,  1*8,  0*8 },
	128*8    /* every sprite takes 128 consecutive bytes */
};

static const gfx_layout balllayout =
{
	8,16,  /* 8*16 sprites */
	8,     /* 8 sprites */
	1,  /* 1 bit per pixel */
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },   /* pretty straightforward layout */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
		8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*8    /* every sprite takes 16 consecutive bytes */
};

static GFXDECODE_START( bking )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0,           4  ) /* playfield */
	GFXDECODE_ENTRY( "gfx2", 0, crowlayout, 4*8,         4  ) /* crow */
	GFXDECODE_ENTRY( "gfx3", 0, balllayout, 4*8+4*4,     4  ) /* ball 1 */
	GFXDECODE_ENTRY( "gfx4", 0, balllayout, 4*8+4*4+4*2, 4  ) /* ball 2 */
GFXDECODE_END


WRITE8_MEMBER(bking_state::port_b_w)
{
	/* don't know what this is... could be a filter */
	if (data != 0x00)
		logerror("port_b = %02x\n", data);
}

void bking_state::machine_start()
{
	/* video */
	save_item(NAME(m_pc3259_output));
	save_item(NAME(m_pc3259_mask));
	save_item(NAME(m_xld1));
	save_item(NAME(m_xld2));
	save_item(NAME(m_xld3));
	save_item(NAME(m_yld1));
	save_item(NAME(m_yld2));
	save_item(NAME(m_yld3));
	save_item(NAME(m_ball1_pic));
	save_item(NAME(m_ball2_pic));
	save_item(NAME(m_crow_pic));
	save_item(NAME(m_crow_flip));
	save_item(NAME(m_palette_bank));
	save_item(NAME(m_controller));
	save_item(NAME(m_hit));

	/* sound */
	save_item(NAME(m_sound_nmi_enable));
}

MACHINE_START_MEMBER(bking_state,bking3)
{
	bking_state::machine_start();

	/* misc */
	save_item(NAME(m_addr_h));
	save_item(NAME(m_addr_l));
}

void bking_state::machine_reset()
{
	/* video */
	m_pc3259_output[0] = 0;
	m_pc3259_output[1] = 0;
	m_pc3259_output[2] = 0;
	m_pc3259_output[3] = 0;
	m_pc3259_mask = 0;
	m_xld1 = 0;
	m_xld2 = 0;
	m_xld3 = 0;
	m_yld1 = 0;
	m_yld2 = 0;
	m_yld3 = 0;
	m_ball1_pic = 0;
	m_ball2_pic = 0;
	m_crow_pic = 0;
	m_crow_flip = 0;
	m_palette_bank = 0;
	m_controller = 0;
	m_hit = 0;

	/* sound */
	m_sound_nmi_enable = 1;
}

MACHINE_RESET_MEMBER(bking_state,bking3)
{
	m_mcu->set_input_line(0, CLEAR_LINE);

	bking_state::machine_reset();

	/* misc */
	m_addr_h = 0;
	m_addr_l = 0;
}

static MACHINE_CONFIG_START( bking, bking_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("main_cpu", Z80, XTAL_12MHz/4) /* 3 MHz */
	MCFG_CPU_PROGRAM_MAP(bking_map)
	MCFG_CPU_IO_MAP(bking_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", bking_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_6MHz/2)  /* 3 MHz */
	MCFG_CPU_PROGRAM_MAP(bking_audio_map)
	/* interrupts (from Jungle King hardware, might be wrong): */
	/* - no interrupts synced with vblank */
	/* - NMI triggered by the main CPU */
	/* - periodic IRQ, with frequency 6000000/(4*16*16*10*16) = 36.621 Hz, */
	MCFG_CPU_PERIODIC_INT_DRIVER(bking_state, irq0_line_hold,  (double)6000000/(4*16*16*10*16))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(bking_state, screen_update_bking)
	MCFG_SCREEN_VBLANK_DRIVER(bking_state, screen_eof_bking)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", bking)
	MCFG_PALETTE_ADD("palette", 4*8+4*4+4*2+4*2)
	MCFG_PALETTE_INIT_OWNER(bking_state, bking)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, XTAL_6MHz/4)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay2", AY8910, XTAL_6MHz/4)
	MCFG_AY8910_PORT_A_WRITE_CB(DEVWRITE8("dac", dac_device, write_signed8))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(bking_state, port_b_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( bking3, bking )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("main_cpu")
	MCFG_CPU_IO_MAP(bking3_io_map)

	MCFG_CPU_ADD("mcu", M68705, XTAL_3MHz)      /* xtal is 3MHz, divided by 4 internally */
	MCFG_CPU_PROGRAM_MAP(buggychl_mcu_map)
	MCFG_DEVICE_ADD("bmcu", BUGGYCHL_MCU, 0)

	MCFG_MACHINE_START_OVERRIDE(bking_state,bking3)
	MCFG_MACHINE_RESET_OVERRIDE(bking_state,bking3)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( bking )
	ROM_REGION( 0x10000, "main_cpu", 0 )
	ROM_LOAD( "dm_11.f13", 0x0000, 0x1000, CRC(d84fe4f7) SHA1(3ad1641d05e4faca2be28052ccae8f81bc2255bb) )
	ROM_LOAD( "dm_12.f11", 0x1000, 0x1000, CRC(e065bbe6) SHA1(8d6d3334977c1eea1bf238817d59c25acd9d99f0) )
	ROM_LOAD( "dm_13.f10", 0x2000, 0x1000, CRC(aac7cddd) SHA1(12a8887bd8d3334e0d740a7f54374b0e48021140) )
	ROM_LOAD( "dm_14.f8",  0x3000, 0x1000, CRC(1179d074) SHA1(23df9a7e3e1bf42d6ea3a2d85629d27bd68e9af4) )
	ROM_LOAD( "dm_15.f7",  0x4000, 0x1000, CRC(fda31475) SHA1(784ffa089b7bd4ab4cbd454f4c1c26553a11fc48) )
	ROM_LOAD( "dm_16.f5",  0x5000, 0x1000, CRC(b6c3c3ed) SHA1(6c7f67d5eba35e32b556b531e848ef375123de78) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* Sound ROMs */
	ROM_LOAD( "dm_17.f4",  0x0000, 0x1000, CRC(54840bc3) SHA1(225daf7ff8a4095b0e69ce6ccce6d8eab26ec1c8) )
	ROM_LOAD( "dm_18.d4",  0x1000, 0x1000, CRC(2abadd42) SHA1(d921d333ec9b9140a7d3ce7aaddab35f45fae018) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "dm_10.a5",  0x0000, 0x1000, CRC(fe96dd67) SHA1(11014602f926cf6edbf06e7b2acef92036b2f30a) )
	ROM_LOAD( "dm_09.a7",  0x1000, 0x1000, CRC(80c675d7) SHA1(e590a71a15ea485abf099eceaa16d5a1dbe0c3dc) )
	ROM_LOAD( "dm_08.a8",  0x2000, 0x1000, CRC(d9bd6b60) SHA1(3c790b6a69472e0a37f45baa00ce5c7d09e7b588) )
	ROM_LOAD( "dm_07.a10", 0x3000, 0x1000, CRC(65f7a0e4) SHA1(034dbf2fe384cb69963936e9f3029aa54e032e4a) )
	ROM_LOAD( "dm_06.a11", 0x4000, 0x1000, CRC(00fdbafc) SHA1(b2a8d9c96415fecee52f1c4691a5f10c96f484b1) )
	ROM_LOAD( "dm_05.a13", 0x5000, 0x1000, CRC(3e4fe925) SHA1(9ed73601c8b34ea8889717cbb3ee4a00ab7ab458) )

	ROM_REGION( 0x0800, "gfx2", 0 )
	ROM_LOAD( "dm_01.e10", 0x0000, 0x0800, CRC(e5663f0b) SHA1(b0fed8c4cdff7b12bb220e51d5b7188933934a34) ) /* crow graphics */

	ROM_REGION( 0x0800, "gfx3", 0 )
	ROM_LOAD( "dm_02.e7",  0x0000, 0x0800, CRC(fc9cec31) SHA1(5ab1c9b3b15334c6ec06826005ecb66b34d8879a) ) /* ball 1 graphics. Only the first 128 bytes used */

	ROM_REGION( 0x0800, "gfx4", 0 )
	ROM_LOAD( "dm_02.e9",  0x0000, 0x0800, CRC(fc9cec31) SHA1(5ab1c9b3b15334c6ec06826005ecb66b34d8879a) ) /* ball 2 graphics. Only the first 128 bytes used */

	ROM_REGION( 0x0020, "user1", 0 )
	ROM_LOAD( "dm04.c2",   0x0000, 0x0020, CRC(4cb5bd32) SHA1(8851bae033ba67516d5ff6888e5daef10c2116ee) )  /* collision detection */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "dm_03.d1",  0x0000, 0x0200, CRC(61b7a9ff) SHA1(4302de0c0dad2b871ad4719ad934beaee05a0c40) ) /* palette */
ROM_END

ROM_START( bking2 )
	ROM_REGION( 0x10000, "main_cpu", 0 )
	ROM_LOAD( "01.13f",       0x0000, 0x1000, CRC(078ada3f) SHA1(5e82a6d27c65fe29d664dbfc2ede547c0f4869f0) )
	ROM_LOAD( "02.11f",       0x1000, 0x1000, CRC(c37d110a) SHA1(7aec6c949d1cf136c3037140bd86597feaf29108) )
	ROM_LOAD( "03.10f",       0x2000, 0x1000, CRC(2ba5c681) SHA1(d0df24f5e52e6162b40308d8aa38b0348a100f37) )
	ROM_LOAD( "04.8f",        0x3000, 0x1000, CRC(8fad54e8) SHA1(55edc185914686d42efd848a402f78884d42292b) )
	ROM_LOAD( "05.7f",        0x4000, 0x1000, CRC(b4de6b58) SHA1(f62bdc3128b226454b1f00a4cbe382e1219a11b0) )
	ROM_LOAD( "06.5f",        0x5000, 0x1000, CRC(9ac43b87) SHA1(dd562fee01c81317978d1bd8a0178e3d9be6145a) )
	ROM_LOAD( "07.4f",        0x6000, 0x1000, CRC(b3ed40b7) SHA1(d481094c0381234314f797928e3cdb22f36f4e32) )
	ROM_LOAD( "08.2f",        0x7000, 0x1000, CRC(8fddb2e8) SHA1(6ee5f09d154440851f370a97b35450e3726e14e7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )         /* Sound ROMs */
	ROM_LOAD( "15",           0x0000, 0x1000, CRC(f045d0fe) SHA1(3b34081fa6cd0423236d09b6f23e8cf8cfd627c5) )
	ROM_LOAD( "16",           0x1000, 0x1000, CRC(92d50410) SHA1(e6f4c27031744bbc832a1eb121a7dba4da5286c4) )

	ROM_REGION( 0x6000, "gfx1", 0 )
	ROM_LOAD( "14.5a",        0x0000, 0x1000, CRC(52636a94) SHA1(185c4455bd9bb23d14aa2f6f7baa74959da08fc2) )
	ROM_LOAD( "13.7a",        0x1000, 0x1000, CRC(6b9e0564) SHA1(6cdd3820caa3825e98b61fe260960cc05c04d032) )
	ROM_LOAD( "12.8a",        0x2000, 0x1000, CRC(c6d685d9) SHA1(2dd2fda365e6bdf9aa26de90650f4a2588ea0515) )
	ROM_LOAD( "11.10a",       0x3000, 0x1000, CRC(2b949987) SHA1(a94666c4f2fdc25399f7976ed2c25fd454387be6) )
	ROM_LOAD( "10.11a",       0x4000, 0x1000, CRC(eb96f948) SHA1(295ba5a620a8a85a121d3e823804adceeeef64d9) )
	ROM_LOAD( "09.13a",       0x5000, 0x1000, CRC(595e3dd4) SHA1(9dd3388ce704dd5473af034716cd8d48df3dc495) )

	ROM_REGION( 0x0800, "gfx2", 0 )
	ROM_LOAD( "17",           0x0000, 0x0800, CRC(e5663f0b) SHA1(b0fed8c4cdff7b12bb220e51d5b7188933934a34) )    /* crow graphics */

	ROM_REGION( 0x0800, "gfx3", 0 )
	ROM_LOAD( "18",           0x0000, 0x0800, CRC(fc9cec31) SHA1(5ab1c9b3b15334c6ec06826005ecb66b34d8879a) )    /* ball 1 graphics. Only the first 128 bytes used */

	ROM_REGION( 0x0800, "gfx4", 0 )
	ROM_LOAD( "19",           0x0000, 0x0800, CRC(fc9cec31) SHA1(5ab1c9b3b15334c6ec06826005ecb66b34d8879a) )  /* ball 2 graphics. Only the first 128 bytes used */

	ROM_REGION( 0x0020, "user1", 0 )
	ROM_LOAD( "mb7051.2c",    0x0000, 0x0020, CRC(4cb5bd32) SHA1(8851bae033ba67516d5ff6888e5daef10c2116ee) )  /* collision detection */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s141.2d",    0x0000, 0x0200, CRC(61b7a9ff) SHA1(4302de0c0dad2b871ad4719ad934beaee05a0c40) )    /* palette */

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "pal16l8.1",  0x0000, 0x0104, CRC(e75d19f5) SHA1(d51cbb247760312b8884bbd0478a321eee05034f) )
	ROM_LOAD( "pal16l8.2",  0x0200, 0x0104, CRC(0302b683) SHA1(91bfba22c883adb15309d9ec0c42b5b744887c77) )
	ROM_LOAD( "pal16l8.3",  0x0400, 0x0104, CRC(a609d0cf) SHA1(7a18040720646c2dff4c1dc6f272c6a69e538c47) )
ROM_END

/*
Birdie King 3
Taito, 1984

A golf game using a trackball. Uses same harness/pinout as
Elevator Action, Victorious Nine, Jolly Jogger (etc)

PCB Layouts
(Note! There are no PALs on ANY of the PCBs)

Top PCB
-------
DMO70003A
K1000173B (sticker)
 |------------------------|
 |                        |
|-|                       |
| |  Z80        2114      |
| |             2114      |
| |N                      |
| |  A24_18.4F            |
| |      A24_19.4D        |
|-|           A24_20.4B   |
 |                        |
 |  AY3-8910              |
 |                        |
 |      AY3-8910          |
 |                        |
 |                        |
 |                   6MHz |
 |                        |
 |                        |
 |                        |
 | LM3900  LM3900  LM3900 |
 |------------------------|
Notes:
      Z80      - Clock 3.000MHz [6/2]
      AY3-8910 - Clock 1.500MHz [6/4]
      2114     - 1kx4 SRAM (DIP18)
      N        - Flat cable connector, joins to main board
      LM3900   - National LM3900 Quad, dual-input amplifier IC (DIP14)
      plus many resistors/capacitors below the AY3-8910's


Sub PCB (below Top PCB)
-----------------------
SUB PCB J910 0010 A
        K910 0018 A
|------------------------|
|           *            |
|                        |
|                        |
|                        |
|                        |
|                        |
|                        |
|                        |
|                        |
|                        |
|                        |
|                        |
|                        |
|A24_21.IC25             |
|                        |
|                   S2   |
|                        |
|         3MHz         S1|
|T    A24_22.IC17        |
|------------------------|
Notes:
      A24_22.IC17 - Motorola 68705P5 Microcontroller, clock 3.000MHz
      A24_21.IC25 - 2732 EPROM
      *           - DIP24 socket with flat cable below PCB, joins to main board
      T           - 4-pin power connector (5 volts) coming from main board
      S1          - Flat cable connector, joins to main PCB to connector S
      S2          - Flat cable connector, joins to bottom PCB to connector S



Main PCB
--------
J1100001A
K1100001A
M4300001D (sticker)
K1100032A (sticker)
|--------------------------------------------------------------|
|      M3712    VOL             A24_03.2D             M53354   |
|H                                        DM-04.2C            |-|
|                     N                                  *    | |
|           MC14584                                          R| |
|                                                             | |
|                                                             | |
|                                                             | |
|                                                             |-|
|                                                              |
|G          LM3900                                             |
|    MC14093                                                   |
|                   A24_01.7E                                 |-|
|                                                             | |
|                   A24_01.9E                                 | |
|    MC14584                                                 S| |
|                   A24_02.10E                                | |
|    MC14584                    MC1455                        | |
|                                                             |-|
|    DSWC   DSWB   DSWA                                        |
|--------------------------------------------------------------|
Notes:
      M53354    - ?, maybe 74LS154? (DIP24)
      MC1455    - Motorola MC1455 Monolithic Timing Circuit (NE555 compatible)
      A24_01/02 - 2716 EPROMs
      A24_03    - Signetics 82S141 PROM (DIP24)
      DM-04     - Signetics 82S123 PROM (DIP8)
      *         - DIP24 socket with flat cable, joins to SUB PCB DIP24 socket
      R/S       - Flat cable connector, R joins to main board, S joins to SUB PCB
      G         - 22-way Edge Connector
      N         - Flat cable connector, joins to TOP PCB
      H         - 12-pin power connector
      VSync     - 60Hz
      HSync     - 15.67kHz


Bottom PCB
----------
DMO70002A
DMN00002A
K1000172B (sticker)
|--------------------------------------------------------------|
|   A24_17.13A         2114    Z80             A24_04.13F      |
|                                                             |-|
|   A24_16.11A         2114                    A24_05.11F     | |
|                                                            S| |
|   A24_15.10A                                 A24_06.10F     | |
|                                                             | |
|   A24_14.8A                                  A24_07.8F      | |
|                                                             |-|
|T  A24_13.7A                                  A24_08.7F       |
|                                                              |
|   A24_12.5A                                  A24_09.5F       |
|                                                             |-|
|                                              A24_10.4F      | |
|            2114                                             | |
|                                              A24_11.2F     R| |
|            2114                                             | |
|                                                             | |
|            2114                                             |-|
|                                                    12MHz     |
|--------------------------------------------------------------|
Notes:
      R/S   - Flat cable connector, R joins to main board, S joins to SUB PCB
      T     - 18-Way Edge Connector (for +5V/GND only)
      A24_* - 2732 EPROMs
      Z80   - Clock 3.000MHz [12/4]
      2114  - 1kx4 SRAM (DIP18)
*/

ROM_START( bking3 )
	ROM_REGION( 0x10000, "main_cpu", 0 )
	ROM_LOAD( "a24-04.13f",   0x0000, 0x1000, CRC(a0c319a6) SHA1(6b79667288113fde43975fcfd05e93d8e45bf92d) )
	ROM_LOAD( "a24-05.11f",   0x1000, 0x1000, CRC(fedc9b4a) SHA1(3ac22c3ca09df9983f3c8c05e807ecf5999c9fc5) )
	ROM_LOAD( "a24-06.10f",   0x2000, 0x1000, CRC(6a116ebf) SHA1(e58b1f75eb75027749a900b27107930e9072ca5a) )
	ROM_LOAD( "a24-07.8f",    0x3000, 0x1000, CRC(75a74d2d) SHA1(d433e8fcf3819b845936e7e107fef414f72bfc16) )
	ROM_LOAD( "a24-08.7f",    0x4000, 0x1000, CRC(9fe07cf9) SHA1(23fdae48e519a171bf4adeeadf2fdfedfd56f4ea) )
	ROM_LOAD( "a24-09.5f",    0x5000, 0x1000, CRC(51545ced) SHA1(4addad527c6fd675506bf584ec8670a23767787c) )
	ROM_LOAD( "a24-10.4f",    0x6000, 0x1000, CRC(a86b3e62) SHA1(f97a13e31e622b5ac55c23458c65a49c2998196a) )
	ROM_LOAD( "a24-11.2f",    0x7000, 0x1000, CRC(b39db430) SHA1(4f48a34f3aaa1e998a4a5656bc3f399d9e6633c4) )

	ROM_REGION( 0x10000, "audiocpu", 0 )        /* Sound ROMs */
	ROM_LOAD( "a24-18.4f",    0x0000, 0x1000, CRC(fa3bfa98) SHA1(733924e154e301a9d692d80b485afc4ab0e200c1) )
	ROM_LOAD( "a24-19.4d",    0x1000, 0x1000, CRC(817f9c2a) SHA1(7365ecf2700e1fd13016408f5493f8d51aab5bbd) )
	ROM_LOAD( "a24-20.4b",    0x2000, 0x1000, CRC(0e9e16d6) SHA1(43c69602a8d9c34c527ce54472db84168acc4ef4) )

	ROM_REGION( 0x0800, "mcu", 0 )  /* 2k for the microcontroller */
	ROM_LOAD( "a24_22.ic17",  0x000000, 0x000800, CRC(27c497d5) SHA1(c6c72bbf0537da53148fa0a56d412ab46129d29c) )  //M68705P5S uC 3MHz xtal

	ROM_REGION( 0x6000, "gfx1", 0 ) /* Tiles */
	ROM_LOAD( "a24-12.5a",    0x0000, 0x1000, CRC(c5fe4817) SHA1(fbf82d9d85e18b76c7e939932df074a545e73f42) )
	ROM_LOAD( "a24-13.7a",    0x1000, 0x1000, CRC(728bac57) SHA1(3daa246f95b31c971e5418f55b821616d0bce25d) )
	ROM_LOAD( "a24-14.8a",    0x2000, 0x1000, CRC(63cd0009) SHA1(10fcfeec70b23e2206c4f4bf686dc6a48ecba1ce) )
	ROM_LOAD( "a24-15.10a",   0x3000, 0x1000, CRC(590275d0) SHA1(563bebb344c606ca3a2124fc7a8804935a011e90) )
	ROM_LOAD( "a24-16.11a",   0x4000, 0x1000, CRC(728d069e) SHA1(b4adb14281e4874bab7cec7f38ade70b5b7c6b8f) )
	ROM_LOAD( "a24-17.13a",   0x5000, 0x1000, CRC(4c04c4f2) SHA1(8e9eee6d89e91910b398d42ac86597ef91baad96) )

	ROM_REGION( 0x0800, "gfx2", 0 )
	ROM_LOAD( "a24-02.10e",   0x0000, 0x0800, CRC(8560da46) SHA1(56f249f0b56336daac1a3624ef9b71354bb8ca40) ) /* crow graphics */

	ROM_REGION( 0x0800, "gfx3", 0 )
	ROM_LOAD( "a24-01.7e",    0x0000, 0x0800, CRC(369c01e1) SHA1(196e12d0bcaf74cefe4cad3fccb69d104aab061e) ) /* ball 1 graphics. Only the first 128 bytes used */

	ROM_REGION( 0x0800, "gfx4", 0 )
	ROM_LOAD( "a24-01.9e",    0x0000, 0x0800, CRC(369c01e1) SHA1(196e12d0bcaf74cefe4cad3fccb69d104aab061e) ) /* ball 2 graphics. Only the first 128 bytes used */

	ROM_REGION( 0x0020, "user1", 0 )
	ROM_LOAD( "82s123.2c",    0x0000, 0x0020, CRC(4cb5bd32) SHA1(8851bae033ba67516d5ff6888e5daef10c2116ee) ) /* collision detection */

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "a24_03.2d",    0x0000, 0x0200, CRC(599a6cbe) SHA1(eed8592aaba7b2b6d06f26a2b8720a288f9ad90f) )    /* palette */

	ROM_REGION( 0x1000, "user2", 0 )
	ROM_LOAD( "a24-21.25",    0x0000, 0x1000, CRC(3106fcac) SHA1(08454adfb58e5df84140d86ed52fa4ef684df9f1) ) /* extra rom on the same SUB PCB where is the mcu */
ROM_END


GAME( 1982, bking,  0, bking,  bking,  driver_device, 0, ROT270, "Taito Corporation", "Birdie King", MACHINE_SUPPORTS_SAVE )
GAME( 1983, bking2, 0, bking,  bking2, driver_device, 0, ROT90,  "Taito Corporation", "Birdie King 2", MACHINE_SUPPORTS_SAVE )
GAME( 1984, bking3, 0, bking3, bking2, driver_device, 0, ROT90,  "Taito Corporation", "Birdie King 3", MACHINE_SUPPORTS_SAVE )
