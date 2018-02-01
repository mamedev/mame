// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        Chess-Master

        TODO:
        - figure out why chessmsta won't work, for starters it assume z80 carry flag is set at poweron?

****************************************************************************/


#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "machine/z80pio.h"
#include "sound/beep.h"
#include "sound/spkrdev.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "speaker.h"

#include "chessmst.lh"
#include "chessmstdm.lh"


class chessmst_state : public driver_device
{
public:
	chessmst_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_pia2(*this, "z80pio2"),
			m_speaker(*this, "speaker"),
			m_beeper(*this, "beeper"),
			m_extra(*this, "EXTRA")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<z80pio_device> m_pia2;
	optional_device<speaker_sound_device> m_speaker;
	optional_device<beep_device> m_beeper;
	required_ioport m_extra;

	uint16_t m_matrix;
	uint16_t m_led_sel;
	uint8_t m_sensor[64];
	uint8_t m_digit_matrix;
	int m_digit_dot;
	uint16_t m_digit;

	virtual void machine_reset() override;

	DECLARE_WRITE8_MEMBER( digits_w );
	DECLARE_WRITE8_MEMBER( pio1_port_a_w );
	DECLARE_WRITE8_MEMBER( pio1_port_b_w );
	DECLARE_WRITE8_MEMBER( pio1_port_b_dm_w );
	DECLARE_READ8_MEMBER( pio2_port_a_r );
	DECLARE_WRITE8_MEMBER( pio2_port_b_w );
	DECLARE_INPUT_CHANGED_MEMBER(chessmst_sensor);
	DECLARE_INPUT_CHANGED_MEMBER(reset_button);
	DECLARE_INPUT_CHANGED_MEMBER(view_monitor_button);
	DECLARE_WRITE_LINE_MEMBER( timer_555_w );

	void chessmst(machine_config &config);
	void chessmsta(machine_config &config);
	void chessmstdm(machine_config &config);
	void chessmst_io(address_map &map);
	void chessmst_mem(address_map &map);
	void chessmstdm(address_map &map);
	void chessmstdm_io(address_map &map);
private:
	void update_display();
};


ADDRESS_MAP_START(chessmst_state::chessmst_mem)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x7fff) // A15 not connected
	AM_RANGE( 0x0000, 0x27ff ) AM_ROM
	AM_RANGE( 0x3400, 0x3bff ) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START(chessmst_state::chessmstdm)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0x3fff ) AM_ROM
	AM_RANGE( 0x4000, 0x7fff ) AM_DEVREAD("cartslot", generic_slot_device, read_rom)
	AM_RANGE( 0x8000, 0x8bff ) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START(chessmst_state::chessmst_io)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	//AM_RANGE(0x00, 0x03) AM_MIRROR(0xf0) read/write in both, not used by the software
	AM_RANGE(0x04, 0x07) AM_MIRROR(0xf0) AM_DEVREADWRITE("z80pio1", z80pio_device, read, write)
	AM_RANGE(0x08, 0x0b) AM_MIRROR(0xf0) AM_DEVREADWRITE("z80pio2", z80pio_device, read, write)
ADDRESS_MAP_END

ADDRESS_MAP_START(chessmst_state::chessmstdm_io)
	AM_IMPORT_FROM(chessmst_io)
	AM_RANGE(0x4c, 0x4c) AM_WRITE(digits_w)
ADDRESS_MAP_END

WRITE_LINE_MEMBER( chessmst_state::timer_555_w )
{
	m_pia2->strobe_b(state);
	m_pia2->data_b_write(m_matrix);
}

INPUT_CHANGED_MEMBER(chessmst_state::reset_button)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE);
	machine_reset();
}

INPUT_CHANGED_MEMBER(chessmst_state::view_monitor_button)
{
	// pressing both VIEW and MONITOR buttons causes a reset
	if ((m_extra->read() & 0x03) == 0x03)
	{
		m_maincpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
		machine_reset();
	}
}

INPUT_CHANGED_MEMBER(chessmst_state::chessmst_sensor)
{
	uint8_t pos = (uint8_t)(uintptr_t)param;

	if (newval)
	{
		m_sensor[pos] = !m_sensor[pos];
	}
}

/* Input ports */
static INPUT_PORTS_START( chessmst )
	PORT_START("COL_A")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor,  0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor,  1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor,  2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor,  3)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor,  4)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor,  5)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor,  6)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor,  7)
	PORT_START("COL_B")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor,  8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor,  9)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 10)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 11)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 12)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 13)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 14)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 15)
	PORT_START("COL_C")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 16)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 17)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 18)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 19)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 20)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 21)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 22)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 23)
	PORT_START("COL_D")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 24)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 25)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 26)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 27)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 28)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 29)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 30)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 31)
	PORT_START("COL_E")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 32)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 33)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 34)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 35)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 36)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 37)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 38)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 39)
	PORT_START("COL_F")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 40)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 41)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 42)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 43)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 44)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 45)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 46)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 47)
	PORT_START("COL_G")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 48)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 49)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 50)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 51)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 52)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 53)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 54)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 55)
	PORT_START("COL_H")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 56)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 57)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 58)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 59)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 60)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 61)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 62)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, chessmst_sensor, 63)

	PORT_START("BUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Hint     [7]")    PORT_CODE(KEYCODE_7)    PORT_CODE(KEYCODE_H)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Random   [6]")    PORT_CODE(KEYCODE_6)    PORT_CODE(KEYCODE_R)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Referee  [5]")    PORT_CODE(KEYCODE_5)    PORT_CODE(KEYCODE_F)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Selfplay [4]")    PORT_CODE(KEYCODE_4)    PORT_CODE(KEYCODE_S)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Board    [3]")    PORT_CODE(KEYCODE_3)    PORT_CODE(KEYCODE_B)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Color    [2]")    PORT_CODE(KEYCODE_2)    PORT_CODE(KEYCODE_C)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Level    [1]")    PORT_CODE(KEYCODE_1)    PORT_CODE(KEYCODE_L)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("New Game [0]")    PORT_CODE(KEYCODE_0)    PORT_CODE(KEYCODE_ENTER)

	PORT_START("EXTRA")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Halt") PORT_CODE(KEYCODE_F2) PORT_WRITE_LINE_DEVICE_MEMBER("z80pio1", z80pio_device, strobe_a) // -> PIO(1) ASTB pin
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_NAME("Reset") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, reset_button, 0) // -> Z80 RESET pin
INPUT_PORTS_END

static INPUT_PORTS_START( chessmstdm )
	PORT_INCLUDE(chessmst)

	PORT_MODIFY("BUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD)  PORT_NAME("Move Fore")               PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD)  PORT_NAME("Move Back")               PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD)  PORT_NAME("Board")                   PORT_CODE(KEYCODE_B)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD)  PORT_NAME("Match / Time")            PORT_CODE(KEYCODE_M)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD)  PORT_NAME("Parameter / Information") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD)  PORT_NAME("Selection / Dialogue")    PORT_CODE(KEYCODE_S)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD)  PORT_NAME("Function / Notation")     PORT_CODE(KEYCODE_F)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD)  PORT_NAME("Enter")                   PORT_CODE(KEYCODE_ENTER)

	PORT_MODIFY("EXTRA")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Monitor")                 PORT_CODE(KEYCODE_F1)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, view_monitor_button, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("View")                    PORT_CODE(KEYCODE_F2)   PORT_CHANGED_MEMBER(DEVICE_SELF, chessmst_state, view_monitor_button, 0)
INPUT_PORTS_END


void chessmst_state::machine_reset()
{
	//reset all sensors
	memset(m_sensor, 1, sizeof(m_sensor));

	//positioning the pieces for start next game
	for (int i=0; i<64; i+=8)
		m_sensor[i+0] = m_sensor[i+1] = m_sensor[i+6] = m_sensor[i+7] = 0;
}

void chessmst_state::update_display()
{
	for(int i=0; i<4; i++)
	{
		if (BIT(m_digit_matrix, i))
			output().set_indexed_value("digit", i, bitswap<16>(m_digit, 3,5,12,10,14,1,2,13,8,6,11,15,7,9,4,0) | (m_digit_dot << 16));
	}
}

WRITE8_MEMBER( chessmst_state::digits_w )
{
	m_digit = (m_digit << 4) | (data & 0x0f);
	m_digit_matrix = (data >> 4) & 0x0f;

	update_display();
}

WRITE8_MEMBER( chessmst_state::pio1_port_a_w )
{
	for (int row=1; row<=8; row++)
	{
		if (m_led_sel & 0x01)
			output().set_indexed_value("led_a", row, BIT(data, 8-row));
		if (m_led_sel & 0x02)
			output().set_indexed_value("led_b", row, BIT(data, 8-row));
		if (m_led_sel & 0x04)
			output().set_indexed_value("led_c", row, BIT(data, 8-row));
		if (m_led_sel & 0x08)
			output().set_indexed_value("led_d", row, BIT(data, 8-row));
		if (m_led_sel & 0x10)
			output().set_indexed_value("led_e", row, BIT(data, 8-row));
		if (m_led_sel & 0x20)
			output().set_indexed_value("led_f", row, BIT(data, 8-row));
		if (m_led_sel & 0x40)
			output().set_indexed_value("led_g", row, BIT(data, 8-row));
		if (m_led_sel & 0x80)
			output().set_indexed_value("led_h", row, BIT(data, 8-row));
		if (m_led_sel & 0x100)
			output().set_indexed_value("led_i", row, BIT(data, 8-row));
		if (m_led_sel & 0x200)
			output().set_indexed_value("led_j", row, BIT(data, 8-row));
	}

	m_led_sel = 0;
}

WRITE8_MEMBER( chessmst_state::pio1_port_b_w )
{
	m_matrix = (m_matrix & 0xff) | ((data & 0x01)<<8);
	m_led_sel = (m_led_sel & 0xff) | ((data & 0x03)<<8);

	m_speaker->level_w(BIT(data, 6));
}

WRITE8_MEMBER( chessmst_state::pio1_port_b_dm_w )
{
	m_matrix = (m_matrix & 0xff) | ((data & 0x04)<<6);

	m_digit_dot = BIT(data, 4);
	if (m_digit_dot)
		update_display();

	m_beeper->set_state(BIT(data, 3));

	output().set_value("monitor_led", !BIT(data, 5));
	output().set_value("playmode_led", !BIT(data, 6));
}

READ8_MEMBER( chessmst_state::pio2_port_a_r )
{
	uint8_t data = 0x00;

	// The pieces position on the chessboard is identified by 64 Hall
	// sensors, which are in a 8x8 matrix with the corresponding LEDs.
	for (int i=0; i<8; i++)
		for (int j=0; j<8; j++)
		{
			if (m_matrix & (1 << j))
				data |= (m_sensor[j * 8 + i] ? (1 << i) : 0);
		}

	if (m_matrix & 0x100)
		data |= ioport("BUTTONS")->read();

	return data;
}

WRITE8_MEMBER( chessmst_state::pio2_port_b_w )
{
	m_matrix = (data & 0xff) | (m_matrix & 0x100);
	m_led_sel = (data & 0xff) | (m_led_sel & 0x300);
}

static const z80_daisy_config chessmst_daisy_chain[] =
{
	{ "z80pio1" },
	{ nullptr }
};

static const z80_daisy_config chessmstdm_daisy_chain[] =
{
	{ "z80pio2" },
	{ nullptr }
};

MACHINE_CONFIG_START(chessmst_state::chessmst)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL(9'830'400)/4) // U880 Z80 clone
	MCFG_CPU_PROGRAM_MAP(chessmst_mem)
	MCFG_CPU_IO_MAP(chessmst_io)
	MCFG_Z80_DAISY_CHAIN(chessmst_daisy_chain)

	MCFG_DEVICE_ADD("z80pio1", Z80PIO, XTAL(9'830'400)/4)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(chessmst_state, pio1_port_a_w))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(chessmst_state, pio1_port_b_w))

	MCFG_DEVICE_ADD("z80pio2", Z80PIO, XTAL(9'830'400)/4)
	MCFG_Z80PIO_IN_PA_CB(READ8(chessmst_state, pio2_port_a_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(chessmst_state, pio2_port_b_w))

	MCFG_DEFAULT_LAYOUT(layout_chessmst)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(chessmst_state::chessmsta)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL(8'000'000)/4) // U880 Z80 clone
	MCFG_CPU_PROGRAM_MAP(chessmst_mem)
	MCFG_CPU_IO_MAP(chessmst_io)
	MCFG_Z80_DAISY_CHAIN(chessmst_daisy_chain)

	MCFG_DEVICE_ADD("z80pio1", Z80PIO, XTAL(8'000'000)/4)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(chessmst_state, pio1_port_a_w))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(chessmst_state, pio1_port_b_w))

	MCFG_DEVICE_ADD("z80pio2", Z80PIO, XTAL(8'000'000)/4)
	MCFG_Z80PIO_IN_PA_CB(READ8(chessmst_state, pio2_port_a_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(chessmst_state, pio2_port_b_w))

	MCFG_DEFAULT_LAYOUT(layout_chessmst)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(chessmst_state::chessmstdm)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL(8'000'000)/2) // U880 Z80 clone
	MCFG_CPU_PROGRAM_MAP(chessmstdm)
	MCFG_CPU_IO_MAP(chessmstdm_io)
	MCFG_Z80_DAISY_CHAIN(chessmstdm_daisy_chain)

	MCFG_DEVICE_ADD("z80pio1", Z80PIO, XTAL(8'000'000)/4)
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(chessmst_state, pio1_port_a_w))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(chessmst_state, pio1_port_b_dm_w))
	MCFG_Z80PIO_IN_PB_CB(IOPORT("EXTRA"))

	MCFG_DEVICE_ADD("z80pio2", Z80PIO, XTAL(8'000'000)/4)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80PIO_IN_PA_CB(READ8(chessmst_state, pio2_port_a_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(chessmst_state, pio2_port_b_w))

	MCFG_DEFAULT_LAYOUT(layout_chessmstdm)

	MCFG_DEVICE_ADD("555_timer", CLOCK, 500) // from 555 timer
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(chessmst_state, timer_555_w))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 1000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_GENERIC_CARTSLOT_ADD("cartslot", generic_plain_slot, "chessmstdm_cart")
	MCFG_SOFTWARE_LIST_ADD("cart_list", "chessmstdm")
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( chessmst )
	ROM_REGION( 0x2800, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "056.bin", 0x0000, 0x0400, CRC(2b90e5d3) SHA1(c47445964b2e6cb11bd1f27e395cf980c97af196) )
	ROM_LOAD( "057.bin", 0x0400, 0x0400, CRC(e666fc56) SHA1(3fa75b82cead81973bea94191a5c35f0acaaa0e6) )
	ROM_LOAD( "058.bin", 0x0800, 0x0400, CRC(6a17fbec) SHA1(019051e93a5114477c50eaa87e1ff01b02eb404d) )
	ROM_LOAD( "059.bin", 0x0c00, 0x0400, CRC(e96e3d07) SHA1(20fab75f206f842231f0414ebc473ce2a7371e7f) )
	ROM_LOAD( "060.bin", 0x1000, 0x0400, CRC(0e31f000) SHA1(daac924b79957a71a4b276bf2cef44badcbe37d3) )
	ROM_LOAD( "061.bin", 0x1400, 0x0400, CRC(69ad896d) SHA1(25d999b59d4cc74bd339032c26889af00e64df60) )
	ROM_LOAD( "062.bin", 0x1800, 0x0400, CRC(c42925fe) SHA1(c42d8d7c30a9b6d91ac994cec0cc2723f41324e9) )
	ROM_LOAD( "063.bin", 0x1c00, 0x0400, CRC(86be4cdb) SHA1(741f984c15c6841e227a8722ba30cf9e6b86d878) )
	ROM_LOAD( "064.bin", 0x2000, 0x0400, CRC(e82f5480) SHA1(38a939158052f5e6484ee3725b86e522541fe4aa) )
	ROM_LOAD( "065.bin", 0x2400, 0x0400, CRC(4ec0e92c) SHA1(0b748231a50777391b04c1778750fbb46c21bee8) )
ROM_END

ROM_START( chessmsta )
	ROM_REGION( 0x2800, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "2764.bin",       0x0000, 0x2000, CRC(6be28876) SHA1(fd7d77b471e7792aef3b2b3f7ff1de4cdafc94c9) )
	ROM_LOAD( "u2616bm108.bin", 0x2000, 0x0800, CRC(6e69ace3) SHA1(e099b6b6cc505092f64b8d51ab9c70aa64f58f70) )
ROM_END

ROM_START( chessmstdm )
	ROM_REGION( 0x4000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD("CMD_bm002_bm201.bin", 0x0000, 0x4000, CRC(47858079) SHA1(eeae1126b514e4853d056690e72e7f5c6dfb3008))
ROM_END


/* Driver */

//    YEAR  NAME        PARENT    COMPAT  MACHINE     INPUT       STATE           INIT  COMPANY                       FULLNAME                FLAGS
COMP( 1984, chessmst,   0,        0,      chessmst,   chessmst,   chessmst_state, 0,    "VEB Mikroelektronik Erfurt", "Chess-Master (set 1)", MACHINE_NOT_WORKING | MACHINE_CLICKABLE_ARTWORK )
COMP( 1984, chessmsta,  chessmst, 0,      chessmsta,  chessmst,   chessmst_state, 0,    "VEB Mikroelektronik Erfurt", "Chess-Master (set 2)", MACHINE_NOT_WORKING | MACHINE_CLICKABLE_ARTWORK )
COMP( 1987, chessmstdm, 0,        0,      chessmstdm, chessmstdm, chessmst_state, 0,    "VEB Mikroelektronik Erfurt", "Chess-Master Diamond", MACHINE_NOT_WORKING | MACHINE_CLICKABLE_ARTWORK )
