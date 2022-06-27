// license:GPL-2.0+
// copyright-holders:Jarek Burczynski, Phil Stroffolino, Tomasz Slanina
/**************************************************************************

Change Lanes
(C) Taito 1983

Jarek Burczynski
Phil Stroffolino
Tomasz Slanina

***************************************************************************/

#include "emu.h"
#include "changela.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "speaker.h"

#include "changela.lh"


uint8_t changela_state::mcu_r()
{
	//osd_printf_debug("Z80 MCU  R = %x\n", m_mcu_out);
	return m_mcu_out;
}

/* latch LS374 at U39 */
void changela_state::mcu_w(uint8_t data)
{
	m_mcu_in = data;
	if (!BIT(m_port_c_out, 2))
		m_mcu->pa_w(data);
}


/*********************************
        MCU
*********************************/

void changela_state::changela_68705_port_a_w(uint8_t data)
{
	m_port_a_out = data;
}

void changela_state::changela_68705_port_c_w(uint8_t data)
{
	/* PC3 is connected to the CLOCK input of the LS374, so we latch the data on rising edge */
	if (BIT(data, 3) && !BIT(m_port_c_out, 3))
		m_mcu_out = m_port_a_out & (BIT(m_port_c_out, 2) ? 0xff : m_mcu_in);

	/* PC2 is connected to the /OE input of the LS374 */
	m_mcu->pa_w(BIT(data, 2) ? 0xff : m_mcu_in);

	m_port_c_out = data;
}



/* U30 */
uint8_t changela_state::changela_24_r()
{
	return (BIT(m_port_c_out, 1) << 3) | 0x07;   /* bits 2,1,0-N/C inputs */
}

uint8_t changela_state::changela_25_r()
{
	//collisions on bits 3,2, bits 1,0-N/C inputs
	return (m_tree1_col << 3) | (m_tree0_col << 2) | 0x03;
}

uint8_t changela_state::changela_30_r()
{
	return ioport("WHEEL")->read() & 0x0f;  //wheel control (clocked input) signal on bits 3,2,1,0
}

uint8_t changela_state::changela_31_r()
{
	/* If the new value is less than the old value, and it did not wrap around,
	   or if the new value is greater than the old value, and it did wrap around,
	   then we are moving LEFT. */
	uint8_t curr_value = ioport("WHEEL")->read();

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

uint8_t changela_state::changela_2c_r()
{
	int val = ioport("IN0")->read();

	val = (val & 0x30) | ((val & 1) << 7) | (((val & 1) ^ 1) << 6);

	return val;
}

uint8_t changela_state::changela_2d_r()
{
	/* the schems are unreadable - I'm not sure it is V8 (page 74, SOUND I/O BOARD SCHEMATIC 1 OF 2, FIGURE 24 - in the middle on the right side) */
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

WRITE_LINE_MEMBER(changela_state::mcu_pc_0_w)
{
	m_mcu->pc_w(0xfe | state);
}

WRITE_LINE_MEMBER(changela_state::collision_reset_0_w)
{
	m_collision_reset = state;
}

WRITE_LINE_MEMBER(changela_state::collision_reset_1_w)
{
	m_tree_collision_reset = state;
}

WRITE_LINE_MEMBER(changela_state::coin_counter_1_w)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

WRITE_LINE_MEMBER(changela_state::coin_counter_2_w)
{
	machine().bookkeeping().coin_counter_w(1, state);
}


void changela_state::changela_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x83ff).ram().share("spriteram"); /* OBJ0 RAM */
	map(0x9000, 0x97ff).ram().share("videoram");    /* OBJ1 RAM */
	map(0xa000, 0xa07f).w(FUNC(changela_state::changela_colors_w)).share("colorram");   /* Color 93419 RAM 64x9(nine!!!) bits A0-used as the 8-th bit data input (d0-d7->normal, a0->d8) */
	map(0xb000, 0xbfff).rom();

	map(0xc000, 0xc7ff).rw(FUNC(changela_state::changela_mem_device_r), FUNC(changela_state::changela_mem_device_w)); /* RAM4 (River Bed RAM); RAM5 (Tree RAM) */

	/* LS138 - U16 */
	map(0xc800, 0xc800).nopw();                /* not connected */
	map(0xc900, 0xc900).w(FUNC(changela_state::changela_mem_device_select_w)); /* selects the memory device to be accessible at 0xc000-0xc7ff */
	map(0xca00, 0xca00).w(FUNC(changela_state::changela_slope_rom_addr_hi_w));
	map(0xcb00, 0xcb00).w(FUNC(changela_state::changela_slope_rom_addr_lo_w));

	map(0xd000, 0xd001).rw("ay1", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
	map(0xd010, 0xd011).rw("ay2", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));

	/* LS259 - U44 */
	map(0xd020, 0xd027).w("outlatch", FUNC(ls259_device::write_d0));

	/* LS139 - U24 */
	map(0xd024, 0xd024).r(FUNC(changela_state::changela_24_r));
	map(0xd025, 0xd025).r(FUNC(changela_state::changela_25_r));
	map(0xd028, 0xd028).r(FUNC(changela_state::mcu_r));
	map(0xd02c, 0xd02c).r(FUNC(changela_state::changela_2c_r));
	map(0xd02d, 0xd02d).r(FUNC(changela_state::changela_2d_r));

	map(0xd030, 0xd030).rw(FUNC(changela_state::changela_30_r), FUNC(changela_state::mcu_w));
	map(0xd031, 0xd031).r(FUNC(changela_state::changela_31_r));

	map(0xe000, 0xe000).w("watchdog", FUNC(watchdog_timer_device::reset_w)); /* Watchdog */

	map(0xf000, 0xf7ff).ram(); /* RAM2 (Processor RAM) */
}


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
	PORT_DIPNAME( 0x02, 0x00, "Left Slot" )                 PORT_DIPLOCATION("SWD:2")
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
#ifdef THREE_STATE_SWITCH
	PORT_DIPNAME( 0x30, 0x30, "Self Test Switch" )          PORT_DIPLOCATION("SWT:1,2")
	//PORT_DIPSETTING(    0x00, "?" )                       /* Not possible, 3-state switch */
	PORT_DIPSETTING(    0x20, "Free Game" )                 /* "Puts a credit on the game without increasing the coin counter." */
	PORT_DIPSETTING(    0x10, DEF_STR( Test ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Off ) )
#else // schematics don't make it clear exactly how this switch is supposed to work
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Free Game/Self-Test")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
#endif
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("IN0") /* 0xDx2C */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Gear Shift") PORT_TOGGLE /* Gear shift */
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
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE,0xdf); // Z80
	else if(((scanline % 64) == 0)) // timer irq, 3 times per given vblank field
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE,0xcf); // Z80
}

INTERRUPT_GEN_MEMBER(changela_state::chl_mcu_irq)
{
	m_mcu->pulse_input_line(0, m_mcu->minimum_quantum_time());
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
	save_item(NAME(m_port_a_out));
	save_item(NAME(m_port_c_out));
	save_item(NAME(m_mcu_out));
	save_item(NAME(m_mcu_in));

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

	m_port_a_out = 0xff;
	m_port_c_out = 0xff;
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

void changela_state::changela(machine_config &config)
{
	Z80(config, m_maincpu, 5000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &changela_state::changela_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(changela_state::changela_scanline), "screen", 0, 1);

	M68705P3(config, m_mcu, 2500000);
	m_mcu->portb_r().set_ioport("MCU");
	m_mcu->porta_w().set(FUNC(changela_state::changela_68705_port_a_w));
	m_mcu->portc_w().set(FUNC(changela_state::changela_68705_port_c_w));
	m_mcu->set_vblank_int("screen", FUNC(changela_state::chl_mcu_irq));

	ls259_device &outlatch(LS259(config, "outlatch")); // U44 on Sound I/O Board
	outlatch.q_out_cb<0>().set(FUNC(changela_state::collision_reset_0_w));
	outlatch.q_out_cb<1>().set(FUNC(changela_state::coin_counter_1_w));
	outlatch.q_out_cb<2>().set(FUNC(changela_state::coin_counter_2_w));
	outlatch.q_out_cb<4>().set(FUNC(changela_state::mcu_pc_0_w));
	outlatch.q_out_cb<5>().set(FUNC(changela_state::collision_reset_1_w));

	WATCHDOG_TIMER(config, "watchdog");

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(32*8, 262);  /* vert size is a guess */
	m_screen->set_visarea(0*8, 32*8-1, 4*8, 32*8-1);
	m_screen->set_screen_update(FUNC(changela_state::screen_update_changela));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(0x40);

	SPEAKER(config, "mono").front_center();

	ay8910_device &ay1(AY8910(config, "ay1", 1250000));
	ay1.port_a_read_callback().set_ioport("DSWA");
	ay1.port_b_read_callback().set_ioport("DSWB");
	ay1.add_route(ALL_OUTPUTS, "mono", 0.50);

	ay8910_device &ay2(AY8910(config, "ay2", 1250000));
	ay2.port_a_read_callback().set_ioport("DSWC");
	ay2.port_b_read_callback().set_ioport("DSWD");
	ay2.add_route(ALL_OUTPUTS, "mono", 0.50);
}


ROM_START( changela )
	ROM_REGION( 0x10000, "maincpu", 0 ) /* Z80 code */
	ROM_LOAD( "cl25a",  0x0000, 0x2000, CRC(38530a60) SHA1(0b0ef1abe11c5271fcd1671322b77165217553c3) )
	ROM_LOAD( "cl24a",  0x2000, 0x2000, CRC(2fcf4a82) SHA1(c33355e2d4d3fab32c8d713a680ec0fceedab341) )
	ROM_LOAD( "cl23",   0x4000, 0x2000, CRC(08385891) SHA1(d8d66664ec25db067d5a4a6c35ec0ac65b9e0c6a) )
	ROM_LOAD( "cl22a",  0x6000, 0x2000, CRC(796e0abd) SHA1(64dd9fc1f9bc44519a253ef0c02e181dd13904bf) ) // confirmed dumped from a cl22a labeled ROM. Previously MAME had it as cl22. Wrongly labeled before or do they really have same contents?
	ROM_LOAD( "cl27",   0xb000, 0x1000, CRC(3668afb8) SHA1(bcfb788baf806edcb129ea9f9dcb1d4260684773) )

	ROM_REGION( 0x00800, "mcu", 0 ) /* 68705P3 */
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

GAMEL( 1983, changela, 0, changela, changela, changela_state, empty_init, ROT180, "Taito Corporation", "Change Lanes", MACHINE_SUPPORTS_SAVE, layout_changela )
