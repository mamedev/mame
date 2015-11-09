// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    Tatsumi Lock-On hardware

    driver by Phil Bennett

    Known bugs:
        * None

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/nec/nec.h"
#include "sound/2203intf.h"
#include "sound/flt_vol.h"
#include "includes/lockon.h"

#define V30_GND_ADDR    ((m_ctrl_reg & 0x3) << 16)
#define V30_OBJ_ADDR    ((m_ctrl_reg & 0x18) << 13)


/*************************************
 *
 *  Forward definitions
 *
 *************************************/





/*************************************
 *
 *  Machine functions
 *
 *************************************/


/*************************************

    Main control register

     76543210
    |......xx|  Ground CPU A17-A16
    |.....x..|  /Ground CPU bus req.
    |...xx...|  Object CPU A17-A16
    |..x.....|  /Object CPU bus req.
    |.x......|  /Z80 bus req.
    |x.......|  Display enable

*************************************/

WRITE16_MEMBER(lockon_state::adrst_w)
{
	m_ctrl_reg = data & 0xff;

	/* Bus mastering for shared access */
	m_ground->set_input_line(INPUT_LINE_HALT, data & 0x04 ? ASSERT_LINE : CLEAR_LINE);
	m_object->set_input_line(INPUT_LINE_HALT, data & 0x20 ? ASSERT_LINE : CLEAR_LINE);
	m_audiocpu->set_input_line(INPUT_LINE_HALT, data & 0x40 ? CLEAR_LINE : ASSERT_LINE);
}

READ16_MEMBER(lockon_state::main_gnd_r)
{
	address_space &gndspace = m_ground->space(AS_PROGRAM);
	return gndspace.read_word(V30_GND_ADDR | offset * 2);
}

WRITE16_MEMBER(lockon_state::main_gnd_w)
{
	address_space &gndspace = m_ground->space(AS_PROGRAM);

	if (ACCESSING_BITS_0_7)
		gndspace.write_byte(V30_GND_ADDR | (offset * 2 + 0), data);
	if (ACCESSING_BITS_8_15)
		gndspace.write_byte(V30_GND_ADDR | (offset * 2 + 1), data >> 8);
}

READ16_MEMBER(lockon_state::main_obj_r)
{
	address_space &objspace = m_object->space(AS_PROGRAM);
	return objspace.read_word(V30_OBJ_ADDR | offset * 2);
}

WRITE16_MEMBER(lockon_state::main_obj_w)
{
	address_space &objspace =m_object->space(AS_PROGRAM);

	if (ACCESSING_BITS_0_7)
		objspace.write_byte(V30_OBJ_ADDR | (offset * 2 + 0), data);
	if (ACCESSING_BITS_8_15)
		objspace.write_byte(V30_OBJ_ADDR | (offset * 2 + 1), data >> 8);
}

WRITE16_MEMBER(lockon_state::tst_w)
{
	if (offset < 0x800)
	{
		address_space &gndspace = m_ground->space(AS_PROGRAM);
		address_space &objspace = m_object->space(AS_PROGRAM);

		if (ACCESSING_BITS_0_7)
			gndspace.write_byte(V30_GND_ADDR | (offset * 2 + 0), data);
		if (ACCESSING_BITS_8_15)
			gndspace.write_byte(V30_GND_ADDR | (offset * 2 + 1), data >> 8);

		if (ACCESSING_BITS_0_7)
			objspace.write_byte(V30_OBJ_ADDR | (offset * 2 + 0), data);
		if (ACCESSING_BITS_8_15)
			objspace.write_byte(V30_OBJ_ADDR | (offset * 2 + 1), data >> 8);
	}
}

READ16_MEMBER(lockon_state::main_z80_r)
{
	address_space &sndspace = m_audiocpu->space(AS_PROGRAM);
	return 0xff00 | sndspace.read_byte(offset);
}

WRITE16_MEMBER(lockon_state::main_z80_w)
{
	address_space &sndspace = m_audiocpu->space(AS_PROGRAM);
	sndspace.write_byte(offset, data);
}

WRITE16_MEMBER(lockon_state::inten_w)
{
	m_main_inten = 1;
}

WRITE16_MEMBER(lockon_state::emres_w)
{
	machine().watchdog_reset();
	m_main_inten = 0;
}


/*************************************
 *
 *  CPU memory maps
 *
 *************************************/

static ADDRESS_MAP_START( main_v30, AS_PROGRAM, 16, lockon_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x03fff) AM_RAM
	AM_RANGE(0x04000, 0x04003) AM_READWRITE(lockon_crtc_r, lockon_crtc_w)
	AM_RANGE(0x06000, 0x06001) AM_READ_PORT("DSW")
	AM_RANGE(0x08000, 0x081ff) AM_RAM AM_SHARE("hud_ram")
	AM_RANGE(0x09000, 0x09fff) AM_RAM_WRITE(lockon_char_w) AM_SHARE("char_ram")
	AM_RANGE(0x0a000, 0x0a001) AM_WRITE(adrst_w)
	AM_RANGE(0x0b000, 0x0bfff) AM_WRITE(lockon_rotate_w)
	AM_RANGE(0x0c000, 0x0cfff) AM_WRITE(lockon_fb_clut_w)
	AM_RANGE(0x0e000, 0x0e001) AM_WRITE(inten_w)
	AM_RANGE(0x0f000, 0x0f001) AM_WRITE(emres_w)
	AM_RANGE(0x10000, 0x1ffff) AM_READNOP AM_WRITE(tst_w)
	AM_RANGE(0x20000, 0x2ffff) AM_READWRITE(main_z80_r, main_z80_w)
	AM_RANGE(0x30000, 0x3ffff) AM_READWRITE(main_gnd_r, main_gnd_w)
	AM_RANGE(0x40000, 0x4ffff) AM_READWRITE(main_obj_r, main_obj_w)
	AM_RANGE(0x50000, 0x5ffff) AM_MIRROR(0x80000) AM_ROM
	AM_RANGE(0x60000, 0x6ffff) AM_MIRROR(0x80000) AM_ROM
	AM_RANGE(0x70000, 0x7ffff) AM_MIRROR(0x80000) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( ground_v30, AS_PROGRAM, 16, lockon_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x03fff) AM_RAM
	AM_RANGE(0x04000, 0x04fff) AM_RAM AM_SHARE("scene_ram")
	AM_RANGE(0x08000, 0x08fff) AM_RAM AM_SHARE("ground_ram")
	AM_RANGE(0x0C000, 0x0C001) AM_WRITE(lockon_scene_h_scr_w)
	AM_RANGE(0x0C002, 0x0C003) AM_WRITE(lockon_scene_v_scr_w)
	AM_RANGE(0x0C004, 0x0C005) AM_WRITE(lockon_ground_ctrl_w)
	AM_RANGE(0x20000, 0x2ffff) AM_MIRROR(0xc0000) AM_ROM
	AM_RANGE(0x30000, 0x3ffff) AM_MIRROR(0xc0000) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( object_v30, AS_PROGRAM, 16, lockon_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x03fff) AM_RAM
	AM_RANGE(0x04000, 0x04001) AM_READWRITE(lockon_obj_4000_r, lockon_obj_4000_w)
	AM_RANGE(0x08000, 0x08fff) AM_WRITE(lockon_tza112_w)
	AM_RANGE(0x0c000, 0x0c1ff) AM_RAM AM_SHARE("object_ram")
	AM_RANGE(0x30000, 0x3ffff) AM_MIRROR(0xc0000) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_prg, AS_PROGRAM, 8, lockon_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x6fff) AM_ROM
	AM_RANGE(0x7000, 0x7000) AM_WRITE(sound_vol)
	AM_RANGE(0x7400, 0x7403) AM_READ(adc_r) AM_WRITENOP
	AM_RANGE(0x7800, 0x7fff) AM_MIRROR(0x8000) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_io, AS_IO, 8, lockon_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x01) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
	AM_RANGE(0x02, 0x02) AM_NOP
ADDRESS_MAP_END


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( lockon )
	PORT_START("DSW")
	/* DSW 1 - verified on PCB as a 8 position dipswitch block */
	PORT_DIPNAME( 0x0003, 0x0000, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(      0x0001, "2" )
	PORT_DIPSETTING(      0x0000, "3" )
	PORT_DIPSETTING(      0x0002, "4" )
	PORT_DIPSETTING(      0x0003, "5" )
	PORT_DIPNAME( 0x000c, 0x0000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(      0x0004, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Medium ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x000c, DEF_STR( Hardest ) )

	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(      0x0000, "150K & every 200K" )
	PORT_DIPSETTING(      0x0010, "200K & every 200K" )

	PORT_DIPNAME( 0x0020, 0x0000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(      0x0020, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Yes ) )

	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )

	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW1:8" )

	/* DSW 2 - verified on PCB as a 6 position dipswitch block */
	PORT_DIPNAME( 0x0700, 0x0000, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( Free_Play ) )

	PORT_DIPNAME( 0x3800, 0x0000, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW2:4,5,6")
	PORT_DIPSETTING(      0x3000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x2800, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x1800, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(      0x3800, DEF_STR( 1C_6C ) )

	/*
	    Wire jumper beside the dipswitches on PCB TF011.
	    To access the menu, press the service coin during
	    test mode.
	*/
	PORT_DIPNAME( 0x4000, 0x4000, "Enable H/W Tests Menu" )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("YM2203")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME( DEF_STR( Service_Mode )) PORT_CODE(KEYCODE_F2) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1  )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2  )
	PORT_DIPNAME( 0x40, 0x00, "Jumper 1" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Jumper 0" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("ADC_BANK")
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_X ) PORT_MINMAX(0,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(15)

	PORT_START("ADC_PITCH")
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_Y ) PORT_MINMAX(0,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(15)

	PORT_START("ADC_MISSILE")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_START("ADC_HOVER")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_BUTTON4 )
INPUT_PORTS_END


static INPUT_PORTS_START( lockone )
	PORT_INCLUDE( lockon )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x0700, 0x0000, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(      0x0300, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0600, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0700, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0500, DEF_STR( 1C_3C ) )

	PORT_DIPNAME( 0x0800, 0x0000, "Buy-In" )
	PORT_DIPSETTING(      0x0800, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_1C ) )

	PORT_DIPNAME( 0x1000, 0x0000, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On )  )
	PORT_DIPNAME( 0x2000, 0x0000, DEF_STR( Unused ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On )  )
INPUT_PORTS_END


/*************************************
 *
 *  M58990P ADC
 *
 *************************************/

READ8_MEMBER(lockon_state::adc_r)
{
	switch (offset)
	{
		case 0:  return ioport("ADC_BANK")->read();
		case 1:  return ioport("ADC_PITCH")->read();
		case 2:  return ioport("ADC_MISSILE")->read();
		case 3:  return ioport("ADC_HOVER")->read();
		default: return 0;
	}
}

/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout char_layout =
{
	8,8,
	1024,
	2,
	{ 0, 8*8*1024 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( lockon )
	GFXDECODE_ENTRY( "gfx1", 0, char_layout,  0, 128 )
GFXDECODE_END


/*************************************
 *
 *  Sound related
 *
 *************************************/

WRITE8_MEMBER(lockon_state::sound_vol)
{
#define LO_SHUNT    250.0
#define LO_R0       5600.0
#define LO_R1       10000.0
#define LO_R2       22000.0
#define LO_R3       47000.0
#define LO_R0S      (1/(1/LO_SHUNT + 1/LO_R0))
#define LO_R1S      (1/(1/LO_SHUNT + 1/LO_R1))
#define LO_R2S      (1/(1/LO_SHUNT + 1/LO_R2))
#define LO_R3S      (1/(1/LO_SHUNT + 1/LO_R3))
#define LO_RI       100000.0
#define LO_RP       100000.0


	static const double gains[16] =
	{
		-( 1 / (1/LO_RP + 1/(LO_R3   + LO_R2   + LO_R1   + LO_R0))  ) / LO_RI,
		-( 1 / (1/LO_RP + 1/(LO_R3   + LO_R2   + LO_R1   + LO_R0S)) ) / LO_RI,
		-( 1 / (1/LO_RP + 1/(LO_R3   + LO_R2   + LO_R1S  + LO_R0))  ) / LO_RI,
		-( 1 / (1/LO_RP + 1/(LO_R3   + LO_R2   + LO_R1S  + LO_R0S)) ) / LO_RI,
		-( 1 / (1/LO_RP + 1/(LO_R3   + LO_R2S  + LO_R1   + LO_R0))  ) / LO_RI,
		-( 1 / (1/LO_RP + 1/(LO_R3   + LO_R2S  + LO_R1   + LO_R0S)) ) / LO_RI,
		-( 1 / (1/LO_RP + 1/(LO_R3   + LO_R2S  + LO_R1S  + LO_R0))  ) / LO_RI,
		-( 1 / (1/LO_RP + 1/(LO_R3   + LO_R2S  + LO_R1S  + LO_R0S)) ) / LO_RI,
		-( 1 / (1/LO_RP + 1/(LO_R3S  + LO_R2   + LO_R1   + LO_R0))  ) / LO_RI,
		-( 1 / (1/LO_RP + 1/(LO_R3S  + LO_R2   + LO_R1   + LO_R0S)) ) / LO_RI,
		-( 1 / (1/LO_RP + 1/(LO_R3S  + LO_R2   + LO_R1S  + LO_R0))  ) / LO_RI,
		-( 1 / (1/LO_RP + 1/(LO_R3S  + LO_R2   + LO_R1S  + LO_R0S)) ) / LO_RI,
		-( 1 / (1/LO_RP + 1/(LO_R3S  + LO_R2S  + LO_R1   + LO_R0))  ) / LO_RI,
		-( 1 / (1/LO_RP + 1/(LO_R3S  + LO_R2S  + LO_R1   + LO_R0S)) ) / LO_RI,
		-( 1 / (1/LO_RP + 1/(LO_R3S  + LO_R2S  + LO_R1S  + LO_R0))  ) / LO_RI,
		-( 1 / (1/LO_RP + 1/(LO_R3S  + LO_R2S  + LO_R1S  + LO_R0S)) ) / LO_RI,
	};

	double lgain = gains[data & 0xf];
	double rgain = gains[data >> 4];

	m_f2203_1l->flt_volume_set_volume(lgain);
	m_f2203_2l->flt_volume_set_volume(lgain);
	m_f2203_3l->flt_volume_set_volume(lgain);

	m_f2203_1r->flt_volume_set_volume(rgain);
	m_f2203_2r->flt_volume_set_volume(rgain);
	m_f2203_3r->flt_volume_set_volume(rgain);
}

WRITE_LINE_MEMBER(lockon_state::ym2203_irq)
{
	m_audiocpu->set_input_line(0, state ? ASSERT_LINE : CLEAR_LINE );
}

WRITE8_MEMBER(lockon_state::ym2203_out_b)
{
	coin_counter_w(machine(), 0, data & 0x80);
	coin_counter_w(machine(), 1, data & 0x40);
	coin_counter_w(machine(), 2, data & 0x20);

	/* 'Lock-On' lamp */
	set_led_status(machine(), 1, !(data & 0x10));
}

/*************************************
 *
 *  Machine driver
 *
 *************************************/

void lockon_state::machine_start()
{
	save_item(NAME(m_ground_ctrl));
	save_item(NAME(m_scroll_h));
	save_item(NAME(m_scroll_v));
	save_item(NAME(m_xsal));
	save_item(NAME(m_x0ll));
	save_item(NAME(m_dx0ll));
	save_item(NAME(m_dxll));
	save_item(NAME(m_ysal));
	save_item(NAME(m_y0ll));
	save_item(NAME(m_dy0ll));
	save_item(NAME(m_dyll));
	save_item(NAME(m_iden));
	save_item(NAME(m_obj_pal_latch));
	save_item(NAME(m_obj_pal_addr));
	save_item(NAME(m_ctrl_reg));
	save_item(NAME(m_main_inten));
}

void lockon_state::machine_reset()
{
	m_ground_ctrl = 0;
	m_scroll_h = 0;
	m_scroll_v = 0;
	m_xsal = 0;
	m_x0ll = 0;
	m_dx0ll = 0;
	m_dxll = 0;
	m_ysal = 0;
	m_y0ll = 0;
	m_dy0ll = 0;
	m_dyll = 0;
	m_iden = 0;
	m_obj_pal_latch = 0;
	m_obj_pal_addr = 0;
	m_ctrl_reg = 0;
	m_main_inten = 0;
}

static MACHINE_CONFIG_START( lockon, lockon_state )

	MCFG_CPU_ADD("maincpu", V30, XTAL_16MHz / 2)
	MCFG_CPU_PROGRAM_MAP(main_v30)

	MCFG_CPU_ADD("ground", V30, XTAL_16MHz / 2)
	MCFG_CPU_PROGRAM_MAP(ground_v30)

	MCFG_CPU_ADD("object", V30, XTAL_16MHz / 2)
	MCFG_CPU_PROGRAM_MAP(object_v30)

	MCFG_CPU_ADD("audiocpu", Z80, XTAL_16MHz / 4)
	MCFG_CPU_PROGRAM_MAP(sound_prg)
	MCFG_CPU_IO_MAP(sound_io)

	MCFG_WATCHDOG_TIME_INIT(PERIOD_OF_555_ASTABLE(10000, 4700, 10000e-12) * 4096)
	MCFG_QUANTUM_TIME(attotime::from_hz(600))


	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART)
	MCFG_SCREEN_UPDATE_DRIVER(lockon_state, screen_update_lockon)
	MCFG_SCREEN_VBLANK_DRIVER(lockon_state, screen_eof_lockon)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", lockon)
	MCFG_PALETTE_ADD("palette", 1024 + 2048)
	MCFG_PALETTE_INIT_OWNER(lockon_state, lockon)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2203, XTAL_16MHz / 4)
	MCFG_YM2203_IRQ_HANDLER(WRITELINE(lockon_state, ym2203_irq))
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("YM2203"))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(lockon_state, ym2203_out_b))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.40)
	MCFG_SOUND_ROUTE(0, "rspeaker", 0.40)
	MCFG_SOUND_ROUTE(1, "f2203.1l", 1.0)
	MCFG_SOUND_ROUTE(1, "f2203.1r", 1.0)
	MCFG_SOUND_ROUTE(2, "f2203.2l", 1.0)
	MCFG_SOUND_ROUTE(2, "f2203.2r", 1.0)
	MCFG_SOUND_ROUTE(3, "f2203.3l", 1.0)
	MCFG_SOUND_ROUTE(3, "f2203.3r", 1.0)

	MCFG_FILTER_VOLUME_ADD("f2203.1l", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_FILTER_VOLUME_ADD("f2203.1r", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
	MCFG_FILTER_VOLUME_ADD("f2203.2l", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_FILTER_VOLUME_ADD("f2203.2r", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
	MCFG_FILTER_VOLUME_ADD("f2203.3l", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_FILTER_VOLUME_ADD("f2203.3r", 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/


ROM_START( lockon )
	/* TF012 V30 (Main) */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lo1_02c.89", 0x60000, 0x8000, CRC(bbf17263) SHA1(96821a0ecd6efe6764380fef094f87c1d6e1d299) )
	ROM_LOAD16_BYTE( "lo1_03c.88", 0x60001, 0x8000, CRC(fa58fd36) SHA1(16af24027610bf6d3fdc4c3df3bf6d94c6776420) )

	ROM_LOAD16_BYTE( "lo1_04c.77", 0x50000, 0x8000, CRC(4a88576e) SHA1(80a8bd89cedebf080b2c08a6e81d3c2754024d8a) )
	ROM_LOAD16_BYTE( "lo1_05c.76", 0x50001, 0x8000, CRC(5a171b02) SHA1(f41f641136574e6af67c2245eb5a84799984474a) )

	ROM_LOAD16_BYTE( "lo1_00e.96", 0x70000, 0x8000, CRC(25eec97f) SHA1(884d4e75111d532afed9755bd678cb7678385a86) )
	ROM_LOAD16_BYTE( "lo1_01e.95", 0x70001, 0x8000, CRC(03f0391d) SHA1(53795eab8f1b33c86a72d2f0f1cbcf7faab0011b) )

	/* TF013 V30 (Ground) */
	ROM_REGION( 0x100000, "ground", 0 )
	ROM_LOAD16_BYTE( "lo3_01a.30", 0x20000, 0x8000, CRC(3eacdb6b) SHA1(7934c36dac9253dec4d8910954f6f2ae85951fe9) )
	ROM_LOAD16_BYTE( "lo3_03a.33", 0x20001, 0x8000, CRC(4ce96d71) SHA1(cedbc33e86a93d11d5e11c2ef18bcf6390790a88) )
	ROM_LOAD16_BYTE( "lo3_00b.29", 0x30000, 0x8000, CRC(1835dccb) SHA1(8dfb0fea61a3e61f4da3b7f0da02cd19df2e68be) )
	ROM_LOAD16_BYTE( "lo3_02b.32", 0x30001, 0x8000, CRC(2b8931d3) SHA1(f6f40b7857f3d47da8626450b1c1d3c46a1072ab) )

	/* TF014 V30 (Object)  */
	ROM_REGION( 0x100000, "object", 0 )
	ROM_LOAD16_BYTE( "lo4_00b", 0x30000, 0x8000, CRC(5f6b5a50) SHA1(daf82cafcae86d05587c191b0ff194ca7950e130) )
	ROM_LOAD16_BYTE( "lo4_01b", 0x30001, 0x8000, CRC(7e88bcf2) SHA1(d541458ba6178ec3bce0e9b872b9fa1d8edb107c) )

	/* TF014 Z80 (Sound) */
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "lo1_08b.24", 0x00000, 0x8000, CRC(73860ec9) SHA1(a94afa274321b9f9ac2184e133132f9829fb9485) )

	/* 8x8x2 characters */
	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "lo1_07a", 0x00000, 0x2000, CRC(73673b79) SHA1(246b80f0c465cefb7ce1c87dc90a58f0f0ea3e0d) )
	ROM_LOAD( "lo1_06a", 0x02000, 0x2000, CRC(c8205913) SHA1(c791ff14418873ce68b502440c3d7ccc1f9cc00e) )

	/* 8x8x3 scene tiles and CLUT */
	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "lo3_12a.120", 0x00000, 0x10000, CRC(a34262a7) SHA1(08204a4474ab1b07b9114da8af03442737922d3b) )
	ROM_LOAD( "lo3_11a.119", 0x10000, 0x10000, CRC(018efa36) SHA1(99eec3f06146627c7f7177b854424e7162ab7c8e) )
	ROM_LOAD( "lo3_10a.118", 0x20000, 0x10000, CRC(d5f4a8f3) SHA1(fcfaef46ef89c4b97970418a75d110271e94d55f) )
	ROM_LOAD( "lo3_13a.121", 0x30000, 0x10000, CRC(e44774a7) SHA1(010d95ea497690ddd2406b8fef1b0aee375a165e) )

	/* 8x8x1 HUD sprites */
	ROM_REGION( 0x2000, "gfx3", 0 )
	ROM_LOAD( "lo2_00.53", 0x00000, 0x2000, CRC(8affea15) SHA1(b7bcf0abde9c933e3f2c75c1f5e2ca3417d50ca1) )

	/* Ground GFX and LUTs */
	ROM_REGION( 0x60000, "gfx4", 0 )
	ROM_LOAD( "lo3_07a.94", 0x00000, 0x10000, CRC(cebc50e1) SHA1(f8b06ce576c3d41b0a8e2cc3ac60d3515d434812) )
	ROM_LOAD( "lo3_06a.92", 0x10000, 0x10000, CRC(f6b6ebdd) SHA1(30e92da3bf83c4bb30faf00cbf01664b993f137c) )
	ROM_LOAD( "lo3_05a.90", 0x20000, 0x10000, CRC(5b6f4c8e) SHA1(fc8b2c929c60fb0177ed3e407e3f0aacc5df8401) )
	ROM_LOAD( "lo3_08.104", 0x30000, 0x10000, CRC(f418cecd) SHA1(6cf2d13c9df86bad9c24609cb8387e817b5d4281) )
	ROM_LOAD( "lo3_09.105", 0x40000, 0x10000, CRC(3c245568) SHA1(9ff6a23d83627f55c9d4f68e0bd89927bfe10664) )
	ROM_LOAD( "lo3_04a.88", 0x50000, 0x10000, CRC(80b67ba9) SHA1(fdbef463b26cd13c43596310f585432c6e0896d0) )

	/* 8x8x4 object tiles */
	ROM_REGION( 0x100000, "gfx5", 0 )
	ROM_LOAD( "lo5_28a.76", 0x00000, 0x8000, CRC(1186f9b4) SHA1(55598552dafa8cccfb423fc3b65a7fa15831d75b) )
	ROM_LOAD( "lo5_20a.48", 0x08000, 0x8000, CRC(3c1a67b5) SHA1(399935830b32a457ad0de243dd3eb4d368d5c6a6) )
	ROM_LOAD( "lo5_08a.58", 0x10000, 0x8000, CRC(9dedeff5) SHA1(53b0917a4fde4053182d38ea7f99f66e52543c10) )
	ROM_LOAD( "lo5_00a.14", 0x18000, 0x8000, CRC(e9f23ce6) SHA1(4030384a0e8f47e8eea9483482ed1be264aec992) )
	ROM_LOAD( "lo5_24a.62", 0x20000, 0x8000, CRC(1892d083) SHA1(8ee92be93ac222ecc2d9f4fcda3099b1db67516c) )
	ROM_LOAD( "lo5_16a.32", 0x28000, 0x8000, CRC(c4500159) SHA1(e695e31e363cc954aab449f9d3dbc027e27fe7bf) )
	ROM_LOAD( "lo5_04a.28", 0x30000, 0x8000, CRC(099323bc) SHA1(001d30f4c3c27277fadac89dcf616ff89eb0ea1c) )
	ROM_LOAD( "lo5_12a.8",  0x38000, 0x8000, CRC(2f5164ab) SHA1(df775b9e1c3c605a85d44404e4db42e33e80e664) )

	ROM_LOAD( "lo5_29a.77", 0x40000, 0x8000, CRC(45353d8d) SHA1(45cacd36700d24ae9f6eeaebed2fc860ef2d2978) )
	ROM_LOAD( "lo5_21a.49", 0x48000, 0x8000, CRC(39ce2000) SHA1(05f8e6f364ad714232fcea5b535ed5e181febd1e) )
	ROM_LOAD( "lo5_10a.72", 0x50000, 0x8000, CRC(23eeec5a) SHA1(08edd997d773684d329ef554776bc7acff1ac4ce) )
	ROM_LOAD( "lo5_01a.15", 0x58000, 0x8000, CRC(528d1395) SHA1(0221f81900757c10f288807f5c9549b9fdf5390f) )
	ROM_LOAD( "lo5_25a.93", 0x60000, 0x8000, CRC(7f3418bd) SHA1(5e595500f996b71aa73c637a6fddced30d78e222) )
	ROM_LOAD( "lo5_17a.33", 0x68000, 0x8000, CRC(ccf138d3) SHA1(971e7abe5b4d1a9dc8fc71b1ded5d2b81bcffaf2) )
	ROM_LOAD( "lo5_06a.44", 0x70000, 0x8000, CRC(be539b01) SHA1(1eebcbc592c51a676409b5be6c5d6609cd7118c9) )
	ROM_LOAD( "lo5_14a.22", 0x78000, 0x8000, CRC(e63cd59e) SHA1(0518461acdc6c65dca8f21ca29bf528197e7cabe) )

	ROM_LOAD( "lo5_30a.78", 0x80000, 0x8000, CRC(7d3993c5) SHA1(fb18daffcfc46bc1e1cfdee928eea861494af221) )
	ROM_LOAD( "lo5_22a.50", 0x88000, 0x8000, CRC(b1ed0361) SHA1(4bdf439026a858fdd929d5a7baac7d76f51550c5) )
	ROM_LOAD( "lo5_09a.59", 0x90000, 0x8000, CRC(953289bc) SHA1(197066af45c1193c36cd59b4b72b14f1c3bdd33e) )
	ROM_LOAD( "lo5_02a.16", 0x98000, 0x8000, CRC(07aa32a1) SHA1(712b1983747acdd754d3abe934642cbc02ee13f2) )
	ROM_LOAD( "lo5_26a.64", 0xa0000, 0x8000, CRC(a0b5c040) SHA1(ef63f89a368bc73eb77fc02d83b499a0231c1989) )
	ROM_LOAD( "lo5_18a.34", 0xa8000, 0x8000, CRC(89884b24) SHA1(23c1fcc97f3a1abcaad413f4448db26f7c55fd5e) )
	ROM_LOAD( "lo5_05a.29", 0xb0000, 0x8000, CRC(f6b775a2) SHA1(e0146866f2e89675181c5d9d5aba23116daac420) )
	ROM_LOAD( "lo5_13a.9",  0xb8000, 0x8000, CRC(67fbb061) SHA1(78b071cd54642ee7b6d7b9f6b759a1412bb9eef5) )

	ROM_LOAD( "lo5_31a.79", 0xc0000, 0x8000, CRC(d3595292) SHA1(9c45be919296626796b07f70b871fba5d444dbb3) )
	ROM_LOAD( "lo5_23a.51", 0xc8000, 0x8000, CRC(1487895b) SHA1(9d617f37932ca17d902307a97d16cf3b4bb5bc4e) )
	ROM_LOAD( "lo5_11a.73", 0xd0000, 0x8000, CRC(9df0b287) SHA1(6ea3b32a7826186c854cc079711ddea4ebf2ab7c) )
	ROM_LOAD( "lo5_03a.17", 0xd8000, 0x8000, CRC(7aca5d83) SHA1(95456b6c5adc5b776fbd33fd95cc62d4a83c34b6) )
	ROM_LOAD( "lo5_27a.65", 0xe0000, 0x8000, CRC(119ff70a) SHA1(e64d41bc7822c9e99fd025b771551a6c511d13f2) )
	ROM_LOAD( "lo5_19a.35", 0xe8000, 0x8000, CRC(5aaa6a53) SHA1(f8ff547979883ac9a969e76d90d028ec4286ec4c) )
	ROM_LOAD( "lo5_07a.45", 0xf0000, 0x8000, CRC(313f127f) SHA1(0782b8dd5f3a3384c3e7bc9cacaadf6804e06a38) )
	ROM_LOAD( "lo5_15a.23", 0xf8000, 0x8000, CRC(66f9c5db) SHA1(cc68da9312ee0a3441b62d14107e1b7de9b04de3) )

	/* Object LUTs */
	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "lo4_02.109", 0x0000, 0x8000, CRC(0832edde) SHA1(77f9efbe029773417dbc3836a36687e37b5bee4b) )
	ROM_LOAD( "lo4_03.108", 0x8000, 0x8000, CRC(1efac891) SHA1(faf305a30cab1c6bf8a9d6e2682b2c3745aec956) )

	/* Object chunk LUTs */
	ROM_REGION16_LE( 0x20000, "user2", 0 )
	ROM_LOAD16_BYTE( "lo4_04a.119", 0x00000, 0x10000, CRC(098f4151) SHA1(cf38e3c5f3442fbfa97870d25f7c89c465f847a9) )
	ROM_LOAD16_BYTE( "lo4_05a.118", 0x00001, 0x10000, CRC(3b21667c) SHA1(b8337f733ede35145602ee3f0de25c2d4db1b2a5) )

	/* Colour PROMs */
	ROM_REGION( 0x1800, "proms", 0 )
	ROM_LOAD( "lo1a.5", 0x000, 0x400, CRC(82391f30) SHA1(d7153c1f3a3e54de4d4d6f432fbcd66449b96b6e) )
	ROM_LOAD( "lo2a.2", 0x400, 0x400, CRC(2bfc6288) SHA1(03d293ddc0c614b606be823826a4375b3d35901f) )

	/* Object scale PROMs */
	ROM_LOAD_NIB_LOW ( "lo_3.69", 0x800, 0x800, CRC(9d9c41a9) SHA1(aabeefe95274f10400b4b7810ea50afcc4f19fde) )
	ROM_LOAD_NIB_HIGH( "lo_4.68", 0x800, 0x800, CRC(ca4874ef) SHA1(c742f79729b0dc4d227379e9109c7ed21b4c38bb) )
ROM_END


ROM_START( lockonc )
	/* TF012 V30 (Main) */
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "lo1_02c.89", 0x60000, 0x8000, CRC(bbf17263) SHA1(96821a0ecd6efe6764380fef094f87c1d6e1d299) )
	ROM_LOAD16_BYTE( "lo1_03c.88", 0x60001, 0x8000, CRC(fa58fd36) SHA1(16af24027610bf6d3fdc4c3df3bf6d94c6776420) )

	ROM_LOAD16_BYTE( "lo1_04c.77", 0x50000, 0x8000, CRC(4a88576e) SHA1(80a8bd89cedebf080b2c08a6e81d3c2754024d8a) )
	ROM_LOAD16_BYTE( "lo1_05c.76", 0x50001, 0x8000, CRC(5a171b02) SHA1(f41f641136574e6af67c2245eb5a84799984474a) )

	ROM_LOAD16_BYTE( "lo1_00c.96", 0x70000, 0x8000, CRC(e2db493b) SHA1(7491c634b698973ea54c25612d1e79c7efea8a45) )
	ROM_LOAD16_BYTE( "lo1_01c.95", 0x70001, 0x8000, CRC(3e6065e0) SHA1(d870f5b466fab90d5c51dd27ecc807e7b38b5f79) )

	/* TF013 V30 (Ground) */
	ROM_REGION( 0x100000, "ground", 0 )
	ROM_LOAD16_BYTE( "lo3_01a.30", 0x20000, 0x8000, CRC(3eacdb6b) SHA1(7934c36dac9253dec4d8910954f6f2ae85951fe9) )
	ROM_LOAD16_BYTE( "lo3_03a.33", 0x20001, 0x8000, CRC(4ce96d71) SHA1(cedbc33e86a93d11d5e11c2ef18bcf6390790a88) )
	ROM_LOAD16_BYTE( "lo3_00b.29", 0x30000, 0x8000, CRC(1835dccb) SHA1(8dfb0fea61a3e61f4da3b7f0da02cd19df2e68be) )
	ROM_LOAD16_BYTE( "lo3_02b.32", 0x30001, 0x8000, CRC(2b8931d3) SHA1(f6f40b7857f3d47da8626450b1c1d3c46a1072ab) )

	/* TF014 V30 (Object)  */
	ROM_REGION( 0x100000, "object", 0 )
	ROM_LOAD16_BYTE( "lo4_00b", 0x30000, 0x8000, CRC(5f6b5a50) SHA1(daf82cafcae86d05587c191b0ff194ca7950e130) )
	ROM_LOAD16_BYTE( "lo4_01b", 0x30001, 0x8000, CRC(7e88bcf2) SHA1(d541458ba6178ec3bce0e9b872b9fa1d8edb107c) )

	/* TF014 Z80 (Sound) */
	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "lo1_08b.24", 0x00000, 0x8000, CRC(73860ec9) SHA1(a94afa274321b9f9ac2184e133132f9829fb9485) )

	/* 8x8x2 characters */
	ROM_REGION( 0x4000, "gfx1", 0 )
	ROM_LOAD( "lo1_07a", 0x00000, 0x2000, CRC(73673b79) SHA1(246b80f0c465cefb7ce1c87dc90a58f0f0ea3e0d) )
	ROM_LOAD( "lo1_06a", 0x02000, 0x2000, CRC(c8205913) SHA1(c791ff14418873ce68b502440c3d7ccc1f9cc00e) )

	/* 8x8x3 scene tiles and CLUT */
	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "lo3_12a.120", 0x00000, 0x10000, CRC(a34262a7) SHA1(08204a4474ab1b07b9114da8af03442737922d3b) )
	ROM_LOAD( "lo3_11a.119", 0x10000, 0x10000, CRC(018efa36) SHA1(99eec3f06146627c7f7177b854424e7162ab7c8e) )
	ROM_LOAD( "lo3_10a.118", 0x20000, 0x10000, CRC(d5f4a8f3) SHA1(fcfaef46ef89c4b97970418a75d110271e94d55f) )
	ROM_LOAD( "lo3_13a.121", 0x30000, 0x10000, CRC(e44774a7) SHA1(010d95ea497690ddd2406b8fef1b0aee375a165e) )

	/* 8x8x1 HUD sprites */
	ROM_REGION( 0x2000, "gfx3", 0 )
	ROM_LOAD( "lo2_00.53", 0x00000, 0x2000, CRC(8affea15) SHA1(b7bcf0abde9c933e3f2c75c1f5e2ca3417d50ca1) )

	/* Ground GFX and LUTs */
	ROM_REGION( 0x60000, "gfx4", 0 )
	ROM_LOAD( "lo3_07a.94", 0x00000, 0x10000, CRC(cebc50e1) SHA1(f8b06ce576c3d41b0a8e2cc3ac60d3515d434812) )
	ROM_LOAD( "lo3_06a.92", 0x10000, 0x10000, CRC(f6b6ebdd) SHA1(30e92da3bf83c4bb30faf00cbf01664b993f137c) )
	ROM_LOAD( "lo3_05a.90", 0x20000, 0x10000, CRC(5b6f4c8e) SHA1(fc8b2c929c60fb0177ed3e407e3f0aacc5df8401) )
	ROM_LOAD( "lo3_08.104", 0x30000, 0x10000, CRC(f418cecd) SHA1(6cf2d13c9df86bad9c24609cb8387e817b5d4281) )
	ROM_LOAD( "lo3_09.105", 0x40000, 0x10000, CRC(3c245568) SHA1(9ff6a23d83627f55c9d4f68e0bd89927bfe10664) )
	ROM_LOAD( "lo3_04a.88", 0x50000, 0x10000, CRC(80b67ba9) SHA1(fdbef463b26cd13c43596310f585432c6e0896d0) )

	/* 8x8x4 object tiles */
	ROM_REGION( 0x100000, "gfx5", 0 )
	ROM_LOAD( "lo5_28a.76", 0x00000, 0x8000, CRC(1186f9b4) SHA1(55598552dafa8cccfb423fc3b65a7fa15831d75b) )
	ROM_LOAD( "lo5_20a.48", 0x08000, 0x8000, CRC(3c1a67b5) SHA1(399935830b32a457ad0de243dd3eb4d368d5c6a6) )
	ROM_LOAD( "lo5_08a.58", 0x10000, 0x8000, CRC(9dedeff5) SHA1(53b0917a4fde4053182d38ea7f99f66e52543c10) )
	ROM_LOAD( "lo5_00a.14", 0x18000, 0x8000, CRC(e9f23ce6) SHA1(4030384a0e8f47e8eea9483482ed1be264aec992) )
	ROM_LOAD( "lo5_24a.62", 0x20000, 0x8000, CRC(1892d083) SHA1(8ee92be93ac222ecc2d9f4fcda3099b1db67516c) )
	ROM_LOAD( "lo5_16a.32", 0x28000, 0x8000, CRC(c4500159) SHA1(e695e31e363cc954aab449f9d3dbc027e27fe7bf) )
	ROM_LOAD( "lo5_04a.28", 0x30000, 0x8000, CRC(099323bc) SHA1(001d30f4c3c27277fadac89dcf616ff89eb0ea1c) )
	ROM_LOAD( "lo5_12a.8",  0x38000, 0x8000, CRC(2f5164ab) SHA1(df775b9e1c3c605a85d44404e4db42e33e80e664) )

	ROM_LOAD( "lo5_29a.77", 0x40000, 0x8000, CRC(45353d8d) SHA1(45cacd36700d24ae9f6eeaebed2fc860ef2d2978) )
	ROM_LOAD( "lo5_21a.49", 0x48000, 0x8000, CRC(39ce2000) SHA1(05f8e6f364ad714232fcea5b535ed5e181febd1e) )
	ROM_LOAD( "lo5_10a.72", 0x50000, 0x8000, CRC(23eeec5a) SHA1(08edd997d773684d329ef554776bc7acff1ac4ce) )
	ROM_LOAD( "lo5_01a.15", 0x58000, 0x8000, CRC(528d1395) SHA1(0221f81900757c10f288807f5c9549b9fdf5390f) )
	ROM_LOAD( "lo5_25a.93", 0x60000, 0x8000, CRC(7f3418bd) SHA1(5e595500f996b71aa73c637a6fddced30d78e222) )
	ROM_LOAD( "lo5_17a.33", 0x68000, 0x8000, CRC(ccf138d3) SHA1(971e7abe5b4d1a9dc8fc71b1ded5d2b81bcffaf2) )
	ROM_LOAD( "lo5_06a.44", 0x70000, 0x8000, CRC(be539b01) SHA1(1eebcbc592c51a676409b5be6c5d6609cd7118c9) )
	ROM_LOAD( "lo5_14a.22", 0x78000, 0x8000, CRC(e63cd59e) SHA1(0518461acdc6c65dca8f21ca29bf528197e7cabe) )

	ROM_LOAD( "lo5_30a.78", 0x80000, 0x8000, CRC(7d3993c5) SHA1(fb18daffcfc46bc1e1cfdee928eea861494af221) )
	ROM_LOAD( "lo5_22a.50", 0x88000, 0x8000, CRC(b1ed0361) SHA1(4bdf439026a858fdd929d5a7baac7d76f51550c5) )
	ROM_LOAD( "lo5_09a.59", 0x90000, 0x8000, CRC(953289bc) SHA1(197066af45c1193c36cd59b4b72b14f1c3bdd33e) )
	ROM_LOAD( "lo5_02a.16", 0x98000, 0x8000, CRC(07aa32a1) SHA1(712b1983747acdd754d3abe934642cbc02ee13f2) )
	ROM_LOAD( "lo5_26a.64", 0xa0000, 0x8000, CRC(a0b5c040) SHA1(ef63f89a368bc73eb77fc02d83b499a0231c1989) )
	ROM_LOAD( "lo5_18a.34", 0xa8000, 0x8000, CRC(89884b24) SHA1(23c1fcc97f3a1abcaad413f4448db26f7c55fd5e) )
	ROM_LOAD( "lo5_05a.29", 0xb0000, 0x8000, CRC(f6b775a2) SHA1(e0146866f2e89675181c5d9d5aba23116daac420) )
	ROM_LOAD( "lo5_13a.9",  0xb8000, 0x8000, CRC(67fbb061) SHA1(78b071cd54642ee7b6d7b9f6b759a1412bb9eef5) )

	ROM_LOAD( "lo5_31a.79", 0xc0000, 0x8000, CRC(d3595292) SHA1(9c45be919296626796b07f70b871fba5d444dbb3) )
	ROM_LOAD( "lo5_23a.51", 0xc8000, 0x8000, CRC(1487895b) SHA1(9d617f37932ca17d902307a97d16cf3b4bb5bc4e) )
	ROM_LOAD( "lo5_11a.73", 0xd0000, 0x8000, CRC(9df0b287) SHA1(6ea3b32a7826186c854cc079711ddea4ebf2ab7c) )
	ROM_LOAD( "lo5_03a.17", 0xd8000, 0x8000, CRC(7aca5d83) SHA1(95456b6c5adc5b776fbd33fd95cc62d4a83c34b6) )
	ROM_LOAD( "lo5_27a.65", 0xe0000, 0x8000, CRC(119ff70a) SHA1(e64d41bc7822c9e99fd025b771551a6c511d13f2) )
	ROM_LOAD( "lo5_19a.35", 0xe8000, 0x8000, CRC(5aaa6a53) SHA1(f8ff547979883ac9a969e76d90d028ec4286ec4c) )
	ROM_LOAD( "lo5_07a.45", 0xf0000, 0x8000, CRC(313f127f) SHA1(0782b8dd5f3a3384c3e7bc9cacaadf6804e06a38) )
	ROM_LOAD( "lo5_15a.23", 0xf8000, 0x8000, CRC(66f9c5db) SHA1(cc68da9312ee0a3441b62d14107e1b7de9b04de3) )

	/* Object LUTs */
	ROM_REGION( 0x10000, "user1", 0 )
	ROM_LOAD( "lo4_02.109", 0x0000, 0x8000, CRC(0832edde) SHA1(77f9efbe029773417dbc3836a36687e37b5bee4b) )
	ROM_LOAD( "lo4_03.108", 0x8000, 0x8000, CRC(1efac891) SHA1(faf305a30cab1c6bf8a9d6e2682b2c3745aec956) )

	/* Object chunk LUTs */
	ROM_REGION16_LE( 0x20000, "user2", 0 )
	ROM_LOAD16_BYTE( "lo4_04a.119", 0x00000, 0x10000, CRC(098f4151) SHA1(cf38e3c5f3442fbfa97870d25f7c89c465f847a9) )
	ROM_LOAD16_BYTE( "lo4_05a.118", 0x00001, 0x10000, CRC(3b21667c) SHA1(b8337f733ede35145602ee3f0de25c2d4db1b2a5) )

	/* Colour PROMs */
	ROM_REGION( 0x1800, "proms", 0 )
	ROM_LOAD( "lo1a.5", 0x000, 0x400, CRC(82391f30) SHA1(d7153c1f3a3e54de4d4d6f432fbcd66449b96b6e) )
	ROM_LOAD( "lo2a.2", 0x400, 0x400, CRC(2bfc6288) SHA1(03d293ddc0c614b606be823826a4375b3d35901f) )

	/* Object scale PROMs */
	ROM_LOAD_NIB_LOW ( "lo_3.69", 0x800, 0x800, CRC(9d9c41a9) SHA1(aabeefe95274f10400b4b7810ea50afcc4f19fde) )
	ROM_LOAD_NIB_HIGH( "lo_4.68", 0x800, 0x800, CRC(ca4874ef) SHA1(c742f79729b0dc4d227379e9109c7ed21b4c38bb) )
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1986, lockon,  0,      lockon,  lockon, driver_device,  0, ROT0, "Tatsumi", "Lock-On (rev. E)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, lockonc, lockon, lockon,  lockone, driver_device, 0, ROT0, "Tatsumi", "Lock-On (rev. C)", MACHINE_SUPPORTS_SAVE )
