// license:BSD-3-Clause
// copyright-holders:Curt Coder
/*

    Telmac 2000E
    ------------
    (c) 1980 Telercas Oy, Finland

    CPU:        CDP1802A    1.75 MHz
    RAM:        8 KB
    ROM:        8 KB

    Video:      CDP1864     1.75 MHz
    Color RAM:  1 KB

    Colors:     8 fg, 4 bg
    Resolution: 64x192
    Sound:      frequency control, volume on/off
    Keyboard:   ASCII (RCA VP-601/VP-611), KB-16/KB-64

    SBASIC:     24.0


    Telmac TMC-121/111/112
    ----------------------
    (c) 198? Telercas Oy, Finland

    CPU:        CDP1802A    ? MHz

    Built from Telmac 2000 series cards. Huge metal box.

*/

#include "includes/tmc2000e.h"

/* Read/Write Handlers */

READ8_MEMBER( tmc2000e_state::vismac_r )
{
	return 0;
}

WRITE8_MEMBER( tmc2000e_state::vismac_w )
{
}

READ8_MEMBER( tmc2000e_state::floppy_r )
{
	return 0;
}

WRITE8_MEMBER( tmc2000e_state::floppy_w )
{
}

READ8_MEMBER( tmc2000e_state::ascii_keyboard_r )
{
	return 0;
}

READ8_MEMBER( tmc2000e_state::io_r )
{
	return 0;
}

WRITE8_MEMBER( tmc2000e_state::io_w )
{
}

WRITE8_MEMBER( tmc2000e_state::io_select_w )
{
}

WRITE8_MEMBER( tmc2000e_state::keyboard_latch_w )
{
	m_keylatch = data;
}

/* Memory Maps */

static ADDRESS_MAP_START( tmc2000e_map, AS_PROGRAM, 8, tmc2000e_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0xc000, 0xdfff) AM_ROM
	AM_RANGE(0xfc00, 0xffff) AM_WRITEONLY AM_SHARE("colorram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( tmc2000e_io_map, AS_IO, 8, tmc2000e_state )
	AM_RANGE(0x01, 0x01) AM_DEVWRITE(CDP1864_TAG, cdp1864_device, tone_latch_w)
	AM_RANGE(0x02, 0x02) AM_DEVWRITE(CDP1864_TAG, cdp1864_device, step_bgcolor_w)
	AM_RANGE(0x03, 0x03) AM_READWRITE(ascii_keyboard_r, keyboard_latch_w)
	AM_RANGE(0x04, 0x04) AM_READWRITE(io_r, io_w)
	AM_RANGE(0x05, 0x05) AM_READWRITE(vismac_r, vismac_w)
	AM_RANGE(0x06, 0x06) AM_READWRITE(floppy_r, floppy_w)
	AM_RANGE(0x07, 0x07) AM_READ_PORT("DSW0") AM_WRITE(io_select_w)
ADDRESS_MAP_END

/* Input Ports */

static INPUT_PORTS_START( tmc2000e )
	PORT_START("Y0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("Y7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD )

	PORT_START("DSW0")  // System Configuration DIPs
	PORT_DIPNAME( 0x80, 0x00, "Keyboard Type" )
	PORT_DIPSETTING(    0x00, "ASCII" )
	PORT_DIPSETTING(    0x80, "Matrix" )
	PORT_DIPNAME( 0x40, 0x00, "Operating System" )
	PORT_DIPSETTING(    0x00, "TOOL-2000-E" )
	PORT_DIPSETTING(    0x40, "Load from disk" )
	PORT_DIPNAME( 0x30, 0x00, "Display Interface" )
	PORT_DIPSETTING(    0x00, "PAL" )
	PORT_DIPSETTING(    0x10, "CDG-80" )
	PORT_DIPSETTING(    0x20, "VISMAC" )
	PORT_DIPSETTING(    0x30, "UART" )
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("RUN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Run/Reset") PORT_CODE(KEYCODE_R) PORT_TOGGLE
INPUT_PORTS_END

/* Video */

READ_LINE_MEMBER( tmc2000e_state::rdata_r )
{
	return BIT(m_color, 2);
}

READ_LINE_MEMBER( tmc2000e_state::bdata_r )
{
	return BIT(m_color, 1);
}

READ_LINE_MEMBER( tmc2000e_state::gdata_r )
{
	return BIT(m_color, 0);
}

/* CDP1802 Interface */

READ_LINE_MEMBER( tmc2000e_state::clear_r )
{
	return BIT(m_run->read(), 0);
}

READ_LINE_MEMBER( tmc2000e_state::ef2_r )
{
	return m_cassette->input() < 0;
}

READ_LINE_MEMBER( tmc2000e_state::ef3_r )
{
	UINT8 data = ~(m_key_row[m_keylatch / 8])->read();

	return BIT(data, m_keylatch % 8);
}

WRITE_LINE_MEMBER( tmc2000e_state::q_w )
{
	// turn CDP1864 sound generator on/off
	m_cti->aoe_w(state);

	// set Q led status
	set_led_status(machine(), 1, state);

	// tape out
	m_cassette->output(state ? -1.0 : +1.0);

	// floppy control (FDC-6)
}

WRITE8_MEMBER( tmc2000e_state::dma_w )
{
	m_color = (m_colorram[offset & 0x3ff]) & 0x07; // 0x04 = R, 0x02 = B, 0x01 = G

	m_cti->con_w(0); // HACK
	m_cti->dma_w(space, offset, data);
}


/* Machine Initialization */

void tmc2000e_state::machine_start()
{
	// find keyboard rows
	m_key_row[0] = m_y0;
	m_key_row[1] = m_y1;
	m_key_row[2] = m_y2;
	m_key_row[3] = m_y3;
	m_key_row[4] = m_y4;
	m_key_row[5] = m_y5;
	m_key_row[6] = m_y6;
	m_key_row[7] = m_y7;

	/* register for state saving */
	save_item(NAME(m_cdp1864_efx));
	save_item(NAME(m_keylatch));
}

void tmc2000e_state::machine_reset()
{
	m_cti->reset();

	// reset program counter to 0xc000
}

/* Machine Drivers */

static MACHINE_CONFIG_START( tmc2000e, tmc2000e_state )
	// basic system hardware
	MCFG_CPU_ADD(CDP1802_TAG, CDP1802, XTAL_1_75MHz)
	MCFG_CPU_PROGRAM_MAP(tmc2000e_map)
	MCFG_CPU_IO_MAP(tmc2000e_io_map)
	MCFG_COSMAC_WAIT_CALLBACK(VCC)
	MCFG_COSMAC_CLEAR_CALLBACK(READLINE(tmc2000e_state, clear_r))
	MCFG_COSMAC_EF2_CALLBACK(READLINE(tmc2000e_state, ef2_r))
	MCFG_COSMAC_EF3_CALLBACK(READLINE(tmc2000e_state, ef3_r))
	MCFG_COSMAC_Q_CALLBACK(WRITELINE(tmc2000e_state, q_w))
	MCFG_COSMAC_DMAW_CALLBACK(WRITE8(tmc2000e_state, dma_w))

	// video hardware
	MCFG_CDP1864_SCREEN_ADD(SCREEN_TAG, XTAL_1_75MHz)
	MCFG_SCREEN_UPDATE_DEVICE(CDP1864_TAG, cdp1864_device, screen_update)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_CDP1864_ADD(CDP1864_TAG, SCREEN_TAG, XTAL_1_75MHz, GND, INPUTLINE(CDP1802_TAG, COSMAC_INPUT_LINE_INT), INPUTLINE(CDP1802_TAG, COSMAC_INPUT_LINE_DMAOUT), INPUTLINE(CDP1802_TAG, COSMAC_INPUT_LINE_EF1), NULL, READLINE(tmc2000e_state, rdata_r), READLINE(tmc2000e_state, bdata_r), READLINE(tmc2000e_state, gdata_r))
	MCFG_CDP1864_CHROMINANCE(RES_K(2.2), RES_K(1), RES_K(5.1), RES_K(4.7)) // unverified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* devices */
	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("8K")
	MCFG_RAM_EXTRA_OPTIONS("40K")
MACHINE_CONFIG_END

/* ROMs */

ROM_START( tmc2000e )
	ROM_REGION( 0x10000, CDP1802_TAG, 0 )
	ROM_LOAD( "1", 0xc000, 0x0800, NO_DUMP )
	ROM_LOAD( "2", 0xc800, 0x0800, NO_DUMP )
	ROM_LOAD( "3", 0xd000, 0x0800, NO_DUMP )
	ROM_LOAD( "4", 0xd800, 0x0800, NO_DUMP )
ROM_END

//    YEAR  NAME      PARENT   COMPAT   MACHINE   INPUT     INIT    COMPANY        FULLNAME
COMP( 1980, tmc2000e, 0,       0,       tmc2000e, tmc2000e, driver_device, 0,       "Telercas Oy", "Telmac 2000E", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
