// license:???
// copyright-holders:Jarek Burczynski, Phil Stroffolino, Tomasz Slanina
/**************************************************************************
Change Lanes
(C) Taito 1983

Jarek Burczynski
Phil Stroffolino
Tomasz Slanina

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m6805/m6805.h"
#include "sound/ay8910.h"
#include "includes/changela.h"

#include "changela.lh"


READ8_MEMBER(changela_state::mcu_r)
{
	//osd_printf_debug("Z80 MCU  R = %x\n", m_mcu_out);
	return m_mcu_out;
}

/* latch LS374 at U39 */
WRITE8_MEMBER(changela_state::mcu_w)
{
	m_mcu_in = data;
}


/*********************************
        MCU
*********************************/

READ8_MEMBER(changela_state::changela_68705_port_a_r)
{
	return (m_port_a_out & m_ddr_a) | (m_port_a_in & ~m_ddr_a);
}

WRITE8_MEMBER(changela_state::changela_68705_port_a_w)
{
	m_port_a_out = data;
}

WRITE8_MEMBER(changela_state::changela_68705_ddr_a_w)
{
	m_ddr_a = data;
}

READ8_MEMBER(changela_state::changela_68705_port_b_r)
{
	return (m_port_b_out & m_ddr_b) | (ioport("MCU")->read() & ~m_ddr_b);
}

WRITE8_MEMBER(changela_state::changela_68705_port_b_w)
{
	m_port_b_out = data;
}

WRITE8_MEMBER(changela_state::changela_68705_ddr_b_w)
{
	m_ddr_b = data;
}

READ8_MEMBER(changela_state::changela_68705_port_c_r)
{
	return (m_port_c_out & m_ddr_c) | (m_port_c_in & ~m_ddr_c);
}

WRITE8_MEMBER(changela_state::changela_68705_port_c_w)
{
	/* PC3 is connected to the CLOCK input of the LS374,
	    so we latch the data on positive going edge of the clock */

/* this is strange because if we do this corectly - it just doesn't work */
	if ((data & 8) /*& (!(m_port_c_out & 8))*/ )
		m_mcu_out = m_port_a_out;

	/* PC2 is connected to the /OE input of the LS374 */
	if (!(data & 4))
		m_port_a_in = m_mcu_in;

	m_port_c_out = data;
}

WRITE8_MEMBER(changela_state::changela_68705_ddr_c_w)
{
	m_ddr_c = data;
}


static ADDRESS_MAP_START( mcu_map, AS_PROGRAM, 8, changela_state )
	ADDRESS_MAP_GLOBAL_MASK(0x7ff)
	AM_RANGE(0x0000, 0x0000) AM_READWRITE(changela_68705_port_a_r, changela_68705_port_a_w)
	AM_RANGE(0x0001, 0x0001) AM_READWRITE(changela_68705_port_b_r, changela_68705_port_b_w)
	AM_RANGE(0x0002, 0x0002) AM_READWRITE(changela_68705_port_c_r, changela_68705_port_c_w)

	AM_RANGE(0x0004, 0x0004) AM_WRITE(changela_68705_ddr_a_w)
	AM_RANGE(0x0005, 0x0005) AM_WRITE(changela_68705_ddr_b_w)
	AM_RANGE(0x0006, 0x0006) AM_WRITE(changela_68705_ddr_c_w)

	AM_RANGE(0x0000, 0x007f) AM_RAM
	AM_RANGE(0x0080, 0x07ff) AM_ROM
ADDRESS_MAP_END



/* U30 */
READ8_MEMBER(changela_state::changela_24_r)
{
	return ((m_port_c_out & 2) << 2) | 7;   /* bits 2,1,0-N/C inputs */
}

READ8_MEMBER(changela_state::changela_25_r)
{
	//collisions on bits 3,2, bits 1,0-N/C inputs
	return (m_tree1_col << 3) | (m_tree0_col << 2) | 0x03;
}

READ8_MEMBER(changela_state::changela_30_r)
{
	return ioport("WHEEL")->read() & 0x0f;  //wheel control (clocked input) signal on bits 3,2,1,0
}

READ8_MEMBER(changela_state::changela_31_r)
{
	/* If the new value is less than the old value, and it did not wrap around,
	   or if the new value is greater than the old value, and it did wrap around,
	   then we are moving LEFT. */
	UINT8 curr_value = ioport("WHEEL")->read();

	if ((curr_value < m_prev_value_31 && (m_prev_value_31 - curr_value) < 0x80)
	||  (curr_value > m_prev_value_31 && (curr_value - m_prev_value_31) > 0x80))
		m_dir_31 = 1;
	if ((m_prev_value_31 < curr_value && (curr_value - m_prev_value_31) < 0x80)
	||  (m_prev_value_31 > curr_value && (m_prev_value_31 - curr_value) > 0x80))
		m_dir_31 = 0;

	m_prev_value_31 = curr_value;

	//wheel UP/DOWN control signal on bit 3, collisions on bits:2,1,0
	return (m_dir_31 << 3) | (m_left_bank_col << 2) | (m_right_bank_col << 1) | m_boat_shore_col;
}

READ8_MEMBER(changela_state::changela_2c_r)
{
	int val = ioport("IN0")->read();

	val = (val & 0x30) | ((val & 1) << 7) | (((val & 1) ^ 1) << 6);

	return val;
}

READ8_MEMBER(changela_state::changela_2d_r)
{
	/* the schems are unreadable - i'm not sure it is V8 (page 74, SOUND I/O BOARD SCHEMATIC 1 OF 2, FIGURE 24 - in the middle on the right side) */
	int v8 = 0;
	int gas;

	if ((m_screen->vpos() & 0xf8) == 0xf8)
		v8 = 1;

	/* Gas pedal is made up of 2 switches, 1 active low, 1 active high */
	switch (ioport("IN1")->read() & 0x03)
	{
		case 0x02:
			gas = 0x80;
			break;
		case 0x01:
			gas = 0x00;
			break;
		default:
			gas = 0x40;
			break;
	}

	return (ioport("IN1")->read() & 0x20) | gas | (v8 << 4);
}

WRITE8_MEMBER(changela_state::mcu_pc_0_w)
{
	m_port_c_in = (m_port_c_in & 0xfe) | (data & 1);
}

WRITE8_MEMBER(changela_state::changela_collision_reset_0)
{
	m_collision_reset = data & 0x01;
}

WRITE8_MEMBER(changela_state::changela_collision_reset_1)
{
	m_tree_collision_reset = data & 0x01;
}

WRITE8_MEMBER(changela_state::changela_coin_counter_w)
{
	machine().bookkeeping().coin_counter_w(offset, data);
}


static ADDRESS_MAP_START( changela_map, AS_PROGRAM, 8, changela_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x83ff) AM_RAM AM_SHARE("spriteram") /* OBJ0 RAM */
	AM_RANGE(0x9000, 0x97ff) AM_RAM AM_SHARE("videoram")    /* OBJ1 RAM */
	AM_RANGE(0xa000, 0xa07f) AM_WRITE(changela_colors_w) AM_SHARE("colorram")   /* Color 93419 RAM 64x9(nine!!!) bits A0-used as the 8-th bit data input (d0-d7->normal, a0->d8) */
	AM_RANGE(0xb000, 0xbfff) AM_ROM

	AM_RANGE(0xc000, 0xc7ff) AM_READWRITE(changela_mem_device_r, changela_mem_device_w) /* RAM4 (River Bed RAM); RAM5 (Tree RAM) */

	/* LS138 - U16 */
	AM_RANGE(0xc800, 0xc800) AM_WRITENOP                /* not connected */
	AM_RANGE(0xc900, 0xc900) AM_WRITE(changela_mem_device_select_w) /* selects the memory device to be accessible at 0xc000-0xc7ff */
	AM_RANGE(0xca00, 0xca00) AM_WRITE(changela_slope_rom_addr_hi_w)
	AM_RANGE(0xcb00, 0xcb00) AM_WRITE(changela_slope_rom_addr_lo_w)

	AM_RANGE(0xd000, 0xd001) AM_DEVREADWRITE("ay1", ay8910_device, data_r, address_data_w)
	AM_RANGE(0xd010, 0xd011) AM_DEVREADWRITE("ay2", ay8910_device, data_r, address_data_w)

	/* LS259 - U44 */
	AM_RANGE(0xd020, 0xd020) AM_WRITE(changela_collision_reset_0)
	AM_RANGE(0xd021, 0xd022) AM_WRITE(changela_coin_counter_w)
//AM_RANGE(0xd023, 0xd023) AM_WRITENOP

	/* LS139 - U24 */
	AM_RANGE(0xd024, 0xd024) AM_READWRITE(changela_24_r, mcu_pc_0_w)
	AM_RANGE(0xd025, 0xd025) AM_READWRITE(changela_25_r, changela_collision_reset_1)
	AM_RANGE(0xd026, 0xd026) AM_WRITENOP
	AM_RANGE(0xd028, 0xd028) AM_READ(mcu_r)
	AM_RANGE(0xd02c, 0xd02c) AM_READ(changela_2c_r)
	AM_RANGE(0xd02d, 0xd02d) AM_READ(changela_2d_r)

	AM_RANGE(0xd030, 0xd030) AM_READWRITE(changela_30_r, mcu_w)
	AM_RANGE(0xd031, 0xd031) AM_READ(changela_31_r)

	AM_RANGE(0xe000, 0xe000) AM_WRITE(watchdog_reset_w) /* Watchdog */

	AM_RANGE(0xf000, 0xf7ff) AM_RAM /* RAM2 (Processor RAM) */
ADDRESS_MAP_END


static INPUT_PORTS_START( changela )
	PORT_START("DSWA")  /* DSWA */
	PORT_DIPNAME( 0x07, 0x01, "Steering Wheel Ratio" )      PORT_DIPLOCATION("SWA:1,2,3")
	//PORT_DIPSETTING(    0x00, "?" ) /* Not documented */
	PORT_DIPSETTING(    0x01, "Recommended Setting" )
	//PORT_DIPSETTING(    0x02, "?" ) /* Not documented */
	//PORT_DIPSETTING(    0x03, "?" ) /* Not documented */
	//PORT_DIPSETTING(    0x04, "?" ) /* Not documented */
	//PORT_DIPSETTING(    0x05, "?" ) /* Not documented */
	//PORT_DIPSETTING(    0x06, "?" ) /* Not documented */
	//PORT_DIPSETTING(    0x07, "?" ) /* Not documented */
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "Ignore Memory Failures" )    PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Controls ) )         PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Joystick ) )
	PORT_DIPSETTING(    0x00, "Steering Wheel" )
	PORT_DIPNAME( 0x40, 0x40, "Diagnostic" )                PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Players ) )          PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x00, "2" )

	PORT_START("DSWB")  /* DSWB */
	PORT_DIPNAME( 0x03, 0x00, "Max Bonus Fuels" )           PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x00, "99" )
	PORT_DIPNAME( 0x0c, 0x08, "Game Difficulty" )           PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x04, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x30, 0x20, "Traffic Difficulty" )        PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( Very_Easy) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x40, 0x00, "Land Collisions Enabled" )   PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "Car Collision Enabled" )     PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSWC")  /* DSWC - coinage */
	PORT_DIPNAME( 0xf0, 0x10, DEF_STR( Coin_A ) )           PORT_DIPLOCATION("SWC:5,6,7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 2C_6C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x50, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_7C ) )
	PORT_DIPNAME( 0x0f, 0x01, DEF_STR( Coin_B ) )           PORT_DIPLOCATION("SWC:1,2,3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 2C_4C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 2C_5C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 2C_6C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 2C_7C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_7C ) )

	PORT_START("DSWD")  /* DSWD - bonus */
	PORT_DIPNAME( 0x01, 0x01, "Right Slot" )                PORT_DIPLOCATION("SWD:1")
	PORT_DIPSETTING(    0x01, "On Right (Bottom) Counter" )
	PORT_DIPSETTING(    0x00, "On Left (Top) Counter" )
	PORT_DIPNAME( 0x02, 0x02, "Left Slot" )                 PORT_DIPLOCATION("SWD:2")
	PORT_DIPSETTING(    0x02, "On Right (Bottom) Counter" )
	PORT_DIPSETTING(    0x00, "On Left (Top) Counter" )
	PORT_DIPNAME( 0x1c, 0x00, "Credits For Bonus" )         PORT_DIPLOCATION("SWD:3,4,5")
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x14, "5" )
	PORT_DIPSETTING(    0x18, "6" )
	PORT_DIPSETTING(    0x1c, "7" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SWD:6" )   /* Listed as "Unused" */
	PORT_DIPNAME( 0x40, 0x00, "'King Of The World' Name Length" )PORT_DIPLOCATION("SWD:7")
	PORT_DIPSETTING(    0x40, "3 Letters" )
	PORT_DIPSETTING(    0x00, "Long" )
	PORT_DIPNAME( 0x80, 0x00, "'King Of The World' Name" )  PORT_DIPLOCATION("SWD:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("MCU") /* MCU */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x30, 0x30, "Self Test Switch" )          PORT_DIPLOCATION("SWT:1,2")
	//PORT_DIPSETTING(    0x00, "?" )                       /* Not possible, 3-state switch */
	PORT_DIPSETTING(    0x20, "Free Game" )                 /* "Puts a credit on the game without increasing the coin counter." */
	PORT_DIPSETTING(    0x10, DEF_STR( Test ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Off ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN0") /* 0xDx2C */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Gear Shift") PORT_CODE(KEYCODE_SPACE) PORT_TOGGLE /* Gear shift */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_CONDITION("DSWA", 0x20, EQUALS, 0x20)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_CONDITION("DSWA", 0x20, EQUALS, 0x20)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* FWD - negated bit 7 */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* REV - gear position */

	PORT_START("IN1") /* 0xDx2D */
	PORT_BIT( 0x03, 0x00, IPT_PEDAL ) PORT_MINMAX(0x00, 0x02) PORT_SENSITIVITY(10) PORT_KEYDELTA(1) //gas pedal
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW,  IPT_TILT )
	//PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) //gas1
	//PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON2 ) //gas2

	PORT_START("WHEEL") /* 0xDx30 DRIVING_WHEEL */
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(50) PORT_KEYDELTA(8) PORT_CONDITION("DSWA", 0x20, EQUALS, 0x00)
INPUT_PORTS_END

TIMER_DEVICE_CALLBACK_MEMBER(changela_state::changela_scanline)
{
	int scanline = param;

	if(scanline == 256) // vblank irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE,0xdf);
	else if(((scanline % 64) == 0)) // timer irq, 3 times per given vblank field
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE,0xcf);
}

INTERRUPT_GEN_MEMBER(changela_state::chl_mcu_irq)
{
	generic_pulse_irq_line(m_mcu, 0, 1);
}

void changela_state::machine_start()
{
	/* video */
	save_item(NAME(m_slopeROM_bank));
	save_item(NAME(m_tree_en));
	save_item(NAME(m_horizon));
	save_item(NAME(m_mem_dev_selected));
	save_item(NAME(m_v_count_river));
	save_item(NAME(m_v_count_tree));
	save_item(NAME(m_tree_on));

	/* mcu */
	save_item(NAME(m_port_a_in));
	save_item(NAME(m_port_a_out));
	save_item(NAME(m_ddr_a));
	save_item(NAME(m_port_b_out));
	save_item(NAME(m_ddr_b));
	save_item(NAME(m_port_c_in));
	save_item(NAME(m_port_c_out));
	save_item(NAME(m_ddr_c));

	save_item(NAME(m_mcu_out));
	save_item(NAME(m_mcu_in));
	save_item(NAME(m_mcu_pc_1));
	save_item(NAME(m_mcu_pc_0));

	/* misc */
	save_item(NAME(m_tree0_col));
	save_item(NAME(m_tree1_col));
	save_item(NAME(m_left_bank_col));
	save_item(NAME(m_right_bank_col));
	save_item(NAME(m_boat_shore_col));
	save_item(NAME(m_collision_reset));
	save_item(NAME(m_tree_collision_reset));
	save_item(NAME(m_prev_value_31));
	save_item(NAME(m_dir_31));
}

void changela_state::machine_reset()
{
	/* video */
	m_slopeROM_bank = 0;
	m_tree_en = 0;
	m_horizon = 0;
	m_mem_dev_selected = 0;
	m_v_count_river = 0;
	m_v_count_tree = 0;
	m_tree_on[0] = 0;
	m_tree_on[1] = 0;

	/* mcu */
	m_mcu_pc_1 = 0;
	m_mcu_pc_0 = 0;

	m_port_a_in = 0;
	m_port_a_out = 0;
	m_ddr_a = 0;
	m_port_b_out = 0;
	m_ddr_b = 0;
	m_port_c_in = 0;
	m_port_c_out = 0;
	m_ddr_c = 0;
	m_mcu_out = 0;
	m_mcu_in = 0;

	/* misc */
	m_tree0_col = 0;
	m_tree1_col = 0;
	m_left_bank_col = 0;
	m_right_bank_col = 0;
	m_boat_shore_col = 0;
	m_collision_reset = 0;
	m_tree_collision_reset = 0;
	m_prev_value_31 = 0;
	m_dir_31 = 0;
}

static MACHINE_CONFIG_START( changela, changela_state )

	MCFG_CPU_ADD("maincpu", Z80,5000000)
	MCFG_CPU_PROGRAM_MAP(changela_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", changela_state, changela_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("mcu", M68705,2500000)
	MCFG_CPU_PROGRAM_MAP(mcu_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", changela_state, chl_mcu_irq)


	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_SIZE(32*8, 262)  /* vert size is a guess */
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 4*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(changela_state, screen_update_changela)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 0x40)


	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, 1250000)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSWA"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSWB"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("ay2", AY8910, 1250000)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSWC"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSWD"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


ROM_START( changela )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code */
	ROM_LOAD( "cl25a",  0x0000, 0x2000, CRC(38530a60) SHA1(0b0ef1abe11c5271fcd1671322b77165217553c3) )
	ROM_LOAD( "cl24a",  0x2000, 0x2000, CRC(2fcf4a82) SHA1(c33355e2d4d3fab32c8d713a680ec0fceedab341) )
	ROM_LOAD( "cl23",   0x4000, 0x2000, CRC(08385891) SHA1(d8d66664ec25db067d5a4a6c35ec0ac65b9e0c6a) )
	ROM_LOAD( "cl22",   0x6000, 0x2000, CRC(796e0abd) SHA1(64dd9fc1f9bc44519a253ef0c02e181dd13904bf) )
	ROM_LOAD( "cl27",   0xb000, 0x1000, CRC(3668afb8) SHA1(bcfb788baf806edcb129ea9f9dcb1d4260684773) )

	ROM_REGION( 0x10000, "mcu", 0 ) /* 68705U3 */
	ROM_LOAD( "cl38a",  0x0000, 0x800, CRC(b70156ce) SHA1(c5eab8bbd65c4f587426298da4e22f991ce01dde) )

	ROM_REGION( 0x4000, "gfx1", 0 ) /* tile data */
	ROM_LOAD( "cl111",  0x0000, 0x2000, CRC(41c0149d) SHA1(3ea53a3821b044b3d0451fec1b4ee2c28da393ca) )
	ROM_LOAD( "cl113",  0x2000, 0x2000, CRC(ddf99926) SHA1(e816b88302c5639c7284f4845d450f232d63a10c) )

	ROM_REGION( 0x1000, "gfx2", 0 ) /* obj 1 data */
	ROM_LOAD( "cl46",   0x0000, 0x1000, CRC(9c0a7d28) SHA1(fac9180ea0d9aeea56a84b35cc0958f0dd86a801) )

	ROM_REGION( 0x8000, "user1", 0 )    /* obj 0 data */
	ROM_LOAD( "cl100",  0x0000, 0x2000, CRC(3fa9e4fa) SHA1(9abd7df5fcf143a0c476bd8c8753c5ea294b9f74) )
	ROM_LOAD( "cl99",   0x2000, 0x2000, CRC(67b27b9e) SHA1(7df0f93851959359218c8d2272e30d242a77039d) )
	ROM_LOAD( "cl98",   0x4000, 0x2000, CRC(bffe4149) SHA1(5cf0b98f9d342bd06d575c565ea01bbd79f5e04b) )
	ROM_LOAD( "cl97",   0x6000, 0x2000, CRC(5abab8f9) SHA1(f5156855bbcdf0740fd44520386318ee53ebbf9a) )

	ROM_REGION( 0x1000, "user2", 0 )    /* math tables: SLOPE ROM (river-tree schematic page 1/3) */
	ROM_LOAD( "cl44",   0x0000, 0x1000, CRC(160d2bc7) SHA1(2609208c2bd4618ea340923ee01af69278980c36) ) /* first and 2nd half identical */

	ROM_REGION( 0x3000, "user3", 0 )    /* math tables: TREE ROM (river-tree schematic page 3/3)*/
	ROM_LOAD( "cl7",    0x0000, 0x0800, CRC(01e3efca) SHA1(b26203787f105ba32773e37035c39253050f9c82) ) /* fixed bits: 0xxxxxxx */
	ROM_LOAD( "cl9",    0x1000, 0x2000, CRC(4e53cdd0) SHA1(6255411cfdccbe2c581c83f9127d582623453c3a) )

	ROM_REGION( 0x0020, "proms", 0 )
	ROM_LOAD( "cl88",   0x0000, 0x0020, CRC(da4d6625) SHA1(2d9a268973518252eb36f479ab650af8c16c885c) ) /* math train state machine */
ROM_END

GAMEL( 1983, changela, 0, changela, changela, driver_device, 0,   ROT180, "Taito Corporation", "Change Lanes", MACHINE_SUPPORTS_SAVE, layout_changela )
