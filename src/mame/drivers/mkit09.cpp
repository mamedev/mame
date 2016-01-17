// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Multitech Microkit09

    2013-12-08 Mostly working driver.

    The only documentation is in French, so the operation of the system
    is a bit of a mystery.

ToDo:
    - Fix Cassette
    - Need software to test with

Pasting:
    0-F : as is
    (inc) : ^
    (dec) : V
    M (memory) : -
    G (Go) : X

Test Paste:
    -0000 00^11^22^33^44^55^66^77^88^99^--0000
    Now press up-arrow to confirm the data has been entered.



    2015-10-02 Added alternate bios found on a forum. Memory map is different.
               Still to fix keyboard and display. No documentation exists.

****************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "imagedev/cassette.h"
#include "sound/wave.h"
#include "mkit09.lh"


class mkit09_state : public driver_device
{
public:
	mkit09_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag)
		, m_pia(*this, "pia")
		, m_cass(*this, "cassette")
		, m_maincpu(*this, "maincpu")
	{ }

	DECLARE_READ8_MEMBER(pa_r);
	DECLARE_READ8_MEMBER(pb_r);
	DECLARE_WRITE8_MEMBER(pa_w);
	DECLARE_WRITE8_MEMBER(pb_w);
	DECLARE_INPUT_CHANGED_MEMBER(trigger_reset);
	DECLARE_INPUT_CHANGED_MEMBER(trigger_nmi);
private:
	UINT8 m_keydata;
	virtual void machine_reset() override;
	required_device<pia6821_device> m_pia;
	required_device<cassette_image_device> m_cass;
	required_device<cpu_device> m_maincpu;
};


static ADDRESS_MAP_START(mkit09_mem, AS_PROGRAM, 8, mkit09_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000,0x07ff) AM_MIRROR(0x1800) AM_RAM
	AM_RANGE(0xa004,0xa007) AM_MIRROR(0x1ff8) AM_DEVREADWRITE("pia", pia6821_device, read_alt, write_alt)
	AM_RANGE(0xe000,0xe7ff) AM_MIRROR(0x1800) AM_ROM AM_REGION("roms", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(mkit09a_mem, AS_PROGRAM, 8, mkit09_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000,0x07ff) AM_RAM
	AM_RANGE(0xe600,0xe603) AM_DEVREADWRITE("pia", pia6821_device, read_alt, write_alt)
	AM_RANGE(0xee00,0xefff) AM_RAM
	AM_RANGE(0xf000,0xffff) AM_ROM AM_REGION("roms", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( mkit09_io, AS_IO, 8, mkit09_state)
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( mkit09 )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Inc") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Dec") PORT_CODE(KEYCODE_DOWN) PORT_CHAR('V')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("MEM") PORT_CODE(KEYCODE_M) PORT_CHAR('-')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("BP") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("REG") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("GO") PORT_CODE(KEYCODE_G) PORT_CHAR('X')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("cnt") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("ofs") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("L") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("P") PORT_CODE(KEYCODE_P)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("X") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("RST") PORT_CODE(KEYCODE_ESC) PORT_CHANGED_MEMBER(DEVICE_SELF, mkit09_state, trigger_reset, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("NMI") PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, mkit09_state, trigger_nmi, 0)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( mkit09_state::trigger_reset )
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? CLEAR_LINE : ASSERT_LINE);
}

INPUT_CHANGED_MEMBER( mkit09_state::trigger_nmi )
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}


void mkit09_state::machine_reset()
{
	m_keydata = 0;
}

// read keyboard
READ8_MEMBER( mkit09_state::pa_r )
{
	if (m_keydata < 4)
	{
		char kbdrow[4];
		sprintf(kbdrow,"X%d",m_keydata);
		return ioport(kbdrow)->read();
	}

	return 0xff;
}

// read cassette
READ8_MEMBER( mkit09_state::pb_r )
{
	return m_keydata | ((m_cass->input() > +0.03) ? 0x80 : 0);
}

// write display segments
WRITE8_MEMBER( mkit09_state::pa_w )
{
	data ^= 0xff;
	if (m_keydata > 3)
	{
		output().set_digit_value(m_keydata, BITSWAP8(data, 7, 0, 5, 6, 4, 2, 1, 3));
		m_keydata = 0;
	}

	return;
}

// write cassette, select keyboard row, select a digit
WRITE8_MEMBER( mkit09_state::pb_w )
{
	m_cass->output(BIT(data, 6) ? -1.0 : +1.0);
	m_keydata = data & 15;
	return;
}


static MACHINE_CONFIG_START( mkit09, mkit09_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6809E, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(mkit09_mem)
	MCFG_CPU_IO_MAP(mkit09_io)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_mkit09)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* Devices */
	MCFG_DEVICE_ADD("pia", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(mkit09_state, pa_r))
	MCFG_PIA_READPB_HANDLER(READ8(mkit09_state, pb_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(mkit09_state, pa_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(mkit09_state, pb_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6809e_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6809e_device, irq_line))

	MCFG_CASSETTE_ADD( "cassette" )
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( mkit09a, mkit09_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6809E, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(mkit09a_mem)
	MCFG_CPU_IO_MAP(mkit09_io)

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_mkit09)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_WAVE_ADD(WAVE_TAG, "cassette")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* Devices */
	MCFG_DEVICE_ADD("pia", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(mkit09_state, pa_r))
	MCFG_PIA_READPB_HANDLER(READ8(mkit09_state, pb_r))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(mkit09_state, pa_w))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(mkit09_state, pb_w))
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6809e_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6809e_device, irq_line))

	MCFG_CASSETTE_ADD( "cassette" )
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( mkit09 )
	ROM_REGION( 0x800, "roms", 0 )
	ROM_LOAD( "micromon.bin", 0x0000, 0x0800, CRC(c993c7c2) SHA1(2f54a2b423b925798f669f8a6d2cadeb8a82e968) )
ROM_END

ROM_START( mkit09a )
	ROM_REGION( 0x1000, "roms", 0 )
	ROM_LOAD( "ukit09like.bin", 0x0000, 0x1000, CRC(2cdb6a84) SHA1(edfc1dfc954bdba80c3df64abf4d7553343c1fae) )
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT  COMPAT   MACHINE     INPUT   CLASS          INIT    COMPANY         FULLNAME   FLAGS */
COMP( 1983, mkit09,  0,      0,       mkit09,     mkit09, driver_device,   0,   "Multitech",  "Microkit09", MACHINE_NO_SOUND_HW )
COMP( 1983, mkit09a, mkit09, 0,       mkit09a,    mkit09, driver_device,   0,   "Multitech",  "Microkit09 (Alt version)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
