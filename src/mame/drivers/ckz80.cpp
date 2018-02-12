// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************

    Chess King generic Z80 based chess computer driver

    NOTE: MAME doesn't include a generalized implementation for boardpieces yet,
    greatly affecting user playability of emulated electronic board games.
    As workaround for the chess games, use an external chess GUI on the side,
    such as Arena(in editmode).

    TODO:
    - ckmaster 1 WAIT CLK per M1, workaround with z80_set_cycle_tables is possible
      (wait state is similar to MSX) but I can't be bothered, better solution
      is to add M1 pin to the z80 core. Until then, it'll run ~20% too fast.

******************************************************************************

Master:
- Z80 CPU(NEC D780C-1) @ 4MHz(8MHz XTAL), IRQ from 555 timer
- 8KB ROM(NEC D2764C-3), 2KB RAM(NEC D4016C), ROM is scrambled for easy PCB placement
- simple I/O via 2*74373 and a 74145
- 8*8 chessboard buttons, 32+1 border leds, piezo

******************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/timer.h"
#include "machine/bankdev.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "speaker.h"

// internal artwork
#include "ck_master.lh" // clickable


class ckz80_state : public driver_device
{
public:
	ckz80_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dac(*this, "dac"),
		m_master_map(*this, "master_map"),
		m_inp_matrix(*this, "IN.%u", 0),
		m_out_x(*this, "%u.%u", 0U, 0U),
		m_out_a(*this, "%u.a", 0U),
		m_out_digit(*this, "digit%u", 0U),
		m_display_wait(33),
		m_display_maxy(1),
		m_display_maxx(0)
	{ }

	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<dac_2bit_binary_weighted_ones_complement_device> m_dac;
	optional_device<address_map_bank_device> m_master_map;
	optional_ioport_array<10> m_inp_matrix; // max 10
	output_finder<0x20, 0x20> m_out_x;
	output_finder<0x20> m_out_a;
	output_finder<0x20> m_out_digit;

	TIMER_DEVICE_CALLBACK_MEMBER(irq_on) { m_maincpu->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE); }
	TIMER_DEVICE_CALLBACK_MEMBER(irq_off) { m_maincpu->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE); }

	// misc common
	u16 m_inp_mux;                  // multiplexed keypad mask
	u16 m_led_select;
	u16 m_led_data;

	u16 read_inputs(int columns);

	// display common
	int m_display_wait;             // led/lamp off-delay in milliseconds (default 33ms)
	int m_display_maxy;             // display matrix number of rows
	int m_display_maxx;             // display matrix number of columns (max 31 for now)

	u32 m_display_state[0x20];      // display matrix rows data (last bit is used for always-on)
	u16 m_display_segmask[0x20];    // if not 0, display matrix row is a digit, mask indicates connected segments
	u8 m_display_decay[0x20][0x20]; // (internal use)

	TIMER_DEVICE_CALLBACK_MEMBER(display_decay_tick);
	void display_update();
	void set_display_size(int maxx, int maxy);
	void display_matrix(int maxx, int maxy, u32 setx, u32 sety, bool update = true);

	// Master
	DECLARE_READ8_MEMBER(master_input_r);
	DECLARE_WRITE8_MEMBER(master_control_w);
	DECLARE_DRIVER_INIT(master);
	DECLARE_READ8_MEMBER(master_trampoline_r);
	DECLARE_WRITE8_MEMBER(master_trampoline_w);
	void master_map(address_map &map);
	void master_trampoline(address_map &map);
	void master(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
};


// machine start/reset

void ckz80_state::machine_start()
{
	// resolve handlers
	m_out_x.resolve();
	m_out_a.resolve();
	m_out_digit.resolve();

	// zerofill
	memset(m_display_state, 0, sizeof(m_display_state));
	memset(m_display_decay, 0, sizeof(m_display_decay));
	memset(m_display_segmask, 0, sizeof(m_display_segmask));

	m_inp_mux = 0;
	m_led_select = 0;
	m_led_data = 0;

	// register for savestates
	save_item(NAME(m_display_maxy));
	save_item(NAME(m_display_maxx));
	save_item(NAME(m_display_wait));

	save_item(NAME(m_display_state));
	save_item(NAME(m_display_decay));
	save_item(NAME(m_display_segmask));

	save_item(NAME(m_inp_mux));
	save_item(NAME(m_led_select));
	save_item(NAME(m_led_data));
}

void ckz80_state::machine_reset()
{
}



/***************************************************************************

  Helper Functions

***************************************************************************/

// The device may strobe the outputs very fast, it is unnoticeable to the user.
// To prevent flickering here, we need to simulate a decay.

void ckz80_state::display_update()
{
	for (int y = 0; y < m_display_maxy; y++)
	{
		u32 active_state = 0;

		for (int x = 0; x <= m_display_maxx; x++)
		{
			// turn on powered segments
			if (m_display_state[y] >> x & 1)
				m_display_decay[y][x] = m_display_wait;

			// determine active state
			u32 ds = (m_display_decay[y][x] != 0) ? 1 : 0;
			active_state |= (ds << x);

			// output to y.x, or y.a when always-on
			if (x != m_display_maxx)
				m_out_x[y][x] = ds;
			else
				m_out_a[y] = ds;
		}

		// output to digity
		if (m_display_segmask[y] != 0)
			m_out_digit[y] = active_state & m_display_segmask[y];
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(ckz80_state::display_decay_tick)
{
	// slowly turn off unpowered segments
	for (int y = 0; y < m_display_maxy; y++)
		for (int x = 0; x <= m_display_maxx; x++)
			if (m_display_decay[y][x] != 0)
				m_display_decay[y][x]--;

	display_update();
}

void ckz80_state::set_display_size(int maxx, int maxy)
{
	m_display_maxx = maxx;
	m_display_maxy = maxy;
}

void ckz80_state::display_matrix(int maxx, int maxy, u32 setx, u32 sety, bool update)
{
	set_display_size(maxx, maxy);

	// update current state
	u32 mask = (1 << maxx) - 1;
	for (int y = 0; y < maxy; y++)
		m_display_state[y] = (sety >> y & 1) ? ((setx & mask) | (1 << maxx)) : 0;

	if (update)
		display_update();
}


// generic input handlers

u16 ckz80_state::read_inputs(int columns)
{
	u16 ret = 0;

	// read selected input rows
	for (int i = 0; i < columns; i++)
		if (m_inp_mux >> i & 1)
			ret |= m_inp_matrix[i]->read();

	return ret;
}



// Devices, I/O

/******************************************************************************
    Master
******************************************************************************/

// TTL/generic

WRITE8_MEMBER(ckz80_state::master_control_w)
{
	// d0-d3: 74145 A-D
	// 74145 0-9: input mux, led select
	u16 sel = 1 << (data & 0xf) & 0x3ff;
	m_inp_mux = sel;

	// d4,d5: led data
	display_matrix(2, 9, data >> 4 & 3, sel & 0x1ff);

	// d6,d7: speaker +/-
	m_dac->write(data >> 6 & 3);
}

READ8_MEMBER(ckz80_state::master_input_r)
{
	// d0-d7: multiplexed inputs (active low)
	return ~read_inputs(10);
}

DRIVER_INIT_MEMBER(ckz80_state, master)
{
	u8 *rom = memregion("maincpu")->base();
	const u32 len = memregion("maincpu")->bytes();

	// descramble data lines
	for (int i = 0; i < len; i++)
		rom[i] = bitswap<8>(rom[i], 4,5,0,7,6,1,3,2);

	// descramble address lines
	std::vector<u8> buf(len);
	memcpy(&buf[0], rom, len);
	for (int i = 0; i < len; i++)
		rom[i] = buf[bitswap<16>(i, 15,14,13,12,11,3,7,9, 10,8,6,5,4,2,1,0)];
}



/******************************************************************************
    Address Maps
******************************************************************************/

// Master

ADDRESS_MAP_START(ckz80_state::master_map)
	AM_RANGE(0x0000, 0x1fff) AM_MIRROR(0x6000) AM_ROM AM_REGION("maincpu", 0) // _A15
	AM_RANGE(0xa000, 0xa000) AM_MIRROR(0x1fff) AM_READWRITE(master_input_r, master_control_w) // A13
	AM_RANGE(0xc000, 0xc7ff) AM_MIRROR(0x3800) AM_RAM // A14
ADDRESS_MAP_END

// PCB design is prone to bus conflicts, but should be fine if software obeys
WRITE8_MEMBER(ckz80_state::master_trampoline_w)
{
	if (offset & 0x2000)
		m_master_map->write8(space, (offset & 0x3fff) | 0x8000, data);
	if (offset & 0x4000)
		m_master_map->write8(space, (offset & 0x7fff) | 0x8000, data);
}

READ8_MEMBER(ckz80_state::master_trampoline_r)
{
	u8 data = 0xff;
	if (~offset & 0x8000)
		data &= m_master_map->read8(space, offset);
	if (offset & 0x2000)
		data &= m_master_map->read8(space, (offset & 0x3fff) | 0x8000);
	if (offset & 0x4000)
		data &= m_master_map->read8(space, (offset & 0x7fff) | 0x8000);

	return data;
}

ADDRESS_MAP_START(ckz80_state::master_trampoline)
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(master_trampoline_r, master_trampoline_w)
ADDRESS_MAP_END



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( cb_buttons )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board Sensor")
INPUT_PORTS_END


static INPUT_PORTS_START( master )
	PORT_INCLUDE( cb_buttons )

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Change Position")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Clear Board")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("New Game")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Take Back")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("King")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Queen")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Rook")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN.9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Bishop")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("Pawn")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("White")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Black")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Move")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Level")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Sound")
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

MACHINE_CONFIG_START(ckz80_state::master)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, 8_MHz_XTAL/2)
	MCFG_CPU_PROGRAM_MAP(master_trampoline)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq_on", ckz80_state, irq_on, attotime::from_hz(429)) // theoretical frequency from 555 timer (22nF, 150K, 1K5), measurement was 418Hz
	MCFG_TIMER_START_DELAY(attotime::from_hz(429) - attotime::from_nsec(22870)) // active for 22.87us
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq_off", ckz80_state, irq_off, attotime::from_hz(429))

	MCFG_DEVICE_ADD("master_map", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(master_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATA_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDR_WIDTH(16)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("display_decay", ckz80_state, display_decay_tick, attotime::from_msec(1))
	MCFG_DEFAULT_LAYOUT(layout_ck_master)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("speaker")
	MCFG_SOUND_ADD("dac", DAC_2BIT_BINARY_WEIGHTED_ONES_COMPLEMENT, 0) MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.25)
	MCFG_DEVICE_ADD("vref", VOLTAGE_REGULATOR, 0) MCFG_VOLTAGE_REGULATOR_OUTPUT(5.0)
	MCFG_SOUND_ROUTE_EX(0, "dac", 1.0, DAC_VREF_POS_INPUT) MCFG_SOUND_ROUTE_EX(0, "dac", -1.0, DAC_VREF_NEG_INPUT)
MACHINE_CONFIG_END



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( ckmaster )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("ckmaster.ic2", 0x0000, 0x2000, CRC(59cbec9e) SHA1(2e0629e65778da62bed857406b91a334698d2fe8) ) // D2764C, no label
ROM_END



/******************************************************************************
    Drivers
******************************************************************************/

/*    YEAR  NAME       PARENT    COMPAT  MACHINE  INPUT   STATE         INIT    COMPANY, FULLNAME, FLAGS */
CONS( 1984, ckmaster,  0,        0,      master,  master, ckz80_state,  master, "Chess King", "Master (Chess King)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
