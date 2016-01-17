// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

        Chess-Master

        TODO:
        - add Reset and Halt buttons
        - a better artwork

****************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/z80pio.h"
#include "sound/speaker.h"
#include "chessmst.lh"


class chessmst_state : public driver_device
{
public:
	chessmst_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_speaker(*this, "speaker")
				{ }

	required_device<cpu_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;

	UINT16 m_matrix;
	UINT16 m_led_sel;
	UINT8 m_sensor[64];

	virtual void machine_reset() override;

	DECLARE_WRITE8_MEMBER( pio1_port_a_w );
	DECLARE_WRITE8_MEMBER( pio1_port_b_w );
	DECLARE_READ8_MEMBER( pio2_port_a_r );
	DECLARE_WRITE8_MEMBER( pio2_port_b_w );
	DECLARE_INPUT_CHANGED_MEMBER(chessmst_sensor);
};


static ADDRESS_MAP_START(chessmst_mem, AS_PROGRAM, 8, chessmst_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x7fff) // A15 not connected
	AM_RANGE( 0x0000, 0x27ff ) AM_ROM
	AM_RANGE( 0x3400, 0x3bff ) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( chessmst_io , AS_IO, 8, chessmst_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	//AM_RANGE(0x00, 0x03) AM_MIRROR(0xf0) read/write in both, not used by the software
	AM_RANGE(0x04, 0x07) AM_MIRROR(0xf0) AM_DEVREADWRITE("z80pio1", z80pio_device, read, write)
	AM_RANGE(0x08, 0x0b) AM_MIRROR(0xf0) AM_DEVREADWRITE("z80pio2", z80pio_device, read, write)
ADDRESS_MAP_END

INPUT_CHANGED_MEMBER(chessmst_state::chessmst_sensor)
{
	UINT8 pos = (UINT8)(FPTR)param;

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
INPUT_PORTS_END

void chessmst_state::machine_reset()
{
	//reset all sensors
	memset(m_sensor, 1, sizeof(m_sensor));

	//positioning the pieces for start next game
	for (int i=0; i<64; i+=8)
		m_sensor[i+0] = m_sensor[i+1] = m_sensor[i+6] = m_sensor[i+7] = 0;
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

READ8_MEMBER( chessmst_state::pio2_port_a_r )
{
	UINT8 data = 0x00;

	// The pieces position on the chessboard is identified by 64 Hall
	// sensors, which are in a 8x8 matrix with the corresponding LEDs.
	for (int i=0; i<8; i++)
	{
		if (m_matrix & 0x01)
			data |= (m_sensor[0+i] ? (1<<i) : 0);
		if (m_matrix & 0x02)
			data |= (m_sensor[8+i] ? (1<<i) : 0);
		if (m_matrix & 0x04)
			data |= (m_sensor[16+i] ? (1<<i) : 0);
		if (m_matrix & 0x08)
			data |= (m_sensor[24+i] ? (1<<i) : 0);
		if (m_matrix & 0x10)
			data |= (m_sensor[32+i] ? (1<<i) : 0);
		if (m_matrix & 0x20)
			data |= (m_sensor[40+i] ? (1<<i) : 0);
		if (m_matrix & 0x40)
			data |= (m_sensor[48+i] ? (1<<i) : 0);
		if (m_matrix & 0x80)
			data |= (m_sensor[56+i] ? (1<<i) : 0);
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

static MACHINE_CONFIG_START( chessmst, chessmst_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(chessmst_mem)
	MCFG_CPU_IO_MAP(chessmst_io)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_chessmst)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* devices */
	MCFG_DEVICE_ADD("z80pio1", Z80PIO, XTAL_4MHz)
	MCFG_Z80PIO_OUT_INT_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_Z80PIO_OUT_PA_CB(WRITE8(chessmst_state, pio1_port_a_w))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(chessmst_state, pio1_port_b_w))

	MCFG_DEVICE_ADD("z80pio2", Z80PIO, XTAL_4MHz)
	MCFG_Z80PIO_IN_PA_CB(READ8(chessmst_state, pio2_port_a_r))
	MCFG_Z80PIO_OUT_PB_CB(WRITE8(chessmst_state, pio2_port_b_w))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( chessmst )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "056.bin", 0x0000, 0x0400, CRC(2b90e5d3) SHA1(c47445964b2e6cb11bd1f27e395cf980c97af196))
	ROM_LOAD( "057.bin", 0x0400, 0x0400, CRC(e666fc56) SHA1(3fa75b82cead81973bea94191a5c35f0acaaa0e6))
	ROM_LOAD( "058.bin", 0x0800, 0x0400, CRC(6a17fbec) SHA1(019051e93a5114477c50eaa87e1ff01b02eb404d))
	ROM_LOAD( "059.bin", 0x0c00, 0x0400, CRC(e96e3d07) SHA1(20fab75f206f842231f0414ebc473ce2a7371e7f))
	ROM_LOAD( "060.bin", 0x1000, 0x0400, CRC(0e31f000) SHA1(daac924b79957a71a4b276bf2cef44badcbe37d3))
	ROM_LOAD( "061.bin", 0x1400, 0x0400, CRC(69ad896d) SHA1(25d999b59d4cc74bd339032c26889af00e64df60))
	ROM_LOAD( "062.bin", 0x1800, 0x0400, CRC(c42925fe) SHA1(c42d8d7c30a9b6d91ac994cec0cc2723f41324e9))
	ROM_LOAD( "063.bin", 0x1c00, 0x0400, CRC(86be4cdb) SHA1(741f984c15c6841e227a8722ba30cf9e6b86d878))
	ROM_LOAD( "064.bin", 0x2000, 0x0400, CRC(e82f5480) SHA1(38a939158052f5e6484ee3725b86e522541fe4aa))
	ROM_LOAD( "065.bin", 0x2400, 0x0400, CRC(4ec0e92c) SHA1(0b748231a50777391b04c1778750fbb46c21bee8))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1984, chessmst,  0,       0,  chessmst,   chessmst, driver_device,     0,  "VEB Mikroelektronik Erfurt",   "Chess-Master",        MACHINE_NOT_WORKING | MACHINE_CLICKABLE_ARTWORK )
