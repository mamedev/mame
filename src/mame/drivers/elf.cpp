// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Netronics Elf II

****************************************************************************/

/*

    TODO:

    - proper layout

*/

#include "includes/elf.h"
#include "elf2.lh"

#define RUN \
	BIT(m_special->read(), 0)

#define LOAD \
	BIT(m_special->read(), 1)

#define MEMORY_PROTECT \
	BIT(m_special->read(), 2)

#define INPUT \
	BIT(m_special->read(), 3)

/* Read/Write Handlers */

READ8_MEMBER( elf2_state::dispon_r )
{
	m_vdc->disp_on_w(1);
	m_vdc->disp_on_w(0);

	return 0xff;
}

READ8_MEMBER( elf2_state::data_r )
{
	return m_data;
}

WRITE8_MEMBER( elf2_state::data_w )
{
	m_led_l->a_w(data & 0x0f);
	m_led_h->a_w(data >> 4);
}

WRITE8_MEMBER( elf2_state::memory_w )
{
	if (LOAD)
	{
		if (MEMORY_PROTECT)
		{
			/* latch data from memory */
			data = m_ram->pointer()[offset];
		}
		else
		{
			/* write latched data to memory */
			m_ram->pointer()[offset] = data;
		}

		/* write data to 7 segment displays */
		m_led_l->a_w(data & 0x0f);
		m_led_h->a_w(data >> 4);
	}
}

/* Memory Maps */

static ADDRESS_MAP_START( elf2_mem, AS_PROGRAM, 8, elf2_state )
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x0000, 0x00ff) AM_RAMBANK("bank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( elf2_io, AS_IO, 8, elf2_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x01, 0x01) AM_READ(dispon_r)
	AM_RANGE(0x04, 0x04) AM_READWRITE(data_r, data_w)
ADDRESS_MAP_END

/* Input Ports */

INPUT_CHANGED_MEMBER( elf2_state::input_w )
{
	if (newval)
	{
		/* assert DMAIN */
		m_maincpu->set_input_line(COSMAC_INPUT_LINE_DMAIN, ASSERT_LINE);
	}
}

static INPUT_PORTS_START( elf2 )
	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SPECIAL")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("RUN") PORT_CODE(KEYCODE_R) PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LOAD") PORT_CODE(KEYCODE_L) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("M/P") PORT_CODE(KEYCODE_M) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("INPUT") PORT_CODE(KEYCODE_ENTER) PORT_CHANGED_MEMBER(DEVICE_SELF, elf2_state, input_w, 0)
INPUT_PORTS_END

/* CDP1802 Configuration */

READ_LINE_MEMBER( elf2_state::wait_r )
{
	return LOAD;
}

READ_LINE_MEMBER( elf2_state::clear_r )
{
	return RUN;
}

READ_LINE_MEMBER( elf2_state::ef4_r )
{
	return INPUT;
}

WRITE_LINE_MEMBER( elf2_state::q_w )
{
	output().set_led_value(0, state);
}

READ8_MEMBER( elf2_state::dma_r )
{
	return m_data;
}

WRITE8_MEMBER( elf2_state::sc_w )
{
	switch (data)
	{
	case COSMAC_STATE_CODE_S2_DMA:
	case COSMAC_STATE_CODE_S3_INTERRUPT:
		/* clear DMAIN */
		m_maincpu->set_input_line(COSMAC_INPUT_LINE_DMAIN, CLEAR_LINE);
		break;

	default:
		break;
	}
}

/* MM74C923 Interface */

WRITE_LINE_MEMBER( elf2_state::da_w )
{
	if (state)
	{
		/* shift keyboard data to latch */
		m_data <<= 4;
		m_data |= m_kb->read() & 0x0f;

		if (LOAD)
		{
			/* write data to 7 segment displays */
			m_led_l->a_w(m_data & 0x0f);
			m_led_h->a_w(m_data >> 4);
		}
	}
}

/* Machine Initialization */

void elf2_state::machine_start()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);

	/* initialize LED displays */
	m_led_l->rbi_w(1);
	m_led_h->rbi_w(1);

	/* setup memory banking */
	program.install_read_bank(0x0000, 0x00ff, "bank1");
	program.install_write_handler(0x0000, 0x00ff, write8_delegate(FUNC(elf2_state::memory_w), this));
	membank("bank1")->configure_entry(0, m_ram->pointer());
	membank("bank1")->set_entry(0);

	/* register for state saving */
	save_item(NAME(m_data));
}

/* Machine Driver */

QUICKLOAD_LOAD_MEMBER( elf2_state, elf )
{
	int size = image.length();

	if (size > m_ram->size())
	{
		return IMAGE_INIT_FAIL;
	}

	image.fread(m_ram->pointer(), size);

	return IMAGE_INIT_PASS;
}

static MACHINE_CONFIG_START( elf2, elf2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(CDP1802_TAG, CDP1802, XTAL_3_579545MHz/2)
	MCFG_CPU_PROGRAM_MAP(elf2_mem)
	MCFG_CPU_IO_MAP(elf2_io)
	MCFG_COSMAC_WAIT_CALLBACK(READLINE(elf2_state, wait_r))
	MCFG_COSMAC_CLEAR_CALLBACK(READLINE(elf2_state, clear_r))
	MCFG_COSMAC_EF4_CALLBACK(READLINE(elf2_state, ef4_r))
	MCFG_COSMAC_Q_CALLBACK(WRITELINE(elf2_state, q_w))
	MCFG_COSMAC_DMAR_CALLBACK(READ8(elf2_state, dma_r))
	MCFG_COSMAC_DMAW_CALLBACK(DEVWRITE8(CDP1861_TAG, cdp1861_device, dma_w))
	MCFG_COSMAC_SC_CALLBACK(WRITE8(elf2_state, sc_w))

	/* video hardware */
	MCFG_DEFAULT_LAYOUT(layout_elf2)

	MCFG_DEVICE_ADD(CDP1861_TAG, CDP1861, XTAL_3_579545MHz/2)
	MCFG_CDP1861_IRQ_CALLBACK(INPUTLINE(CDP1802_TAG, COSMAC_INPUT_LINE_INT))
	MCFG_CDP1861_DMA_OUT_CALLBACK(INPUTLINE(CDP1802_TAG, COSMAC_INPUT_LINE_DMAOUT))
	MCFG_CDP1861_EFX_CALLBACK(INPUTLINE(CDP1802_TAG, COSMAC_INPUT_LINE_EF1))
	MCFG_CDP1861_SCREEN_ADD(CDP1861_TAG, SCREEN_TAG, XTAL_3_579545MHz/2)

	/* devices */
	MCFG_DEVICE_ADD(MM74C923_TAG, MM74C923, 0)
	MCFG_MM74C922_OSC(CAP_U(0.15))
	MCFG_MM74C922_DEBOUNCE(CAP_U(1))
	MCFG_MM74C922_DA_CALLBACK(WRITELINE(elf2_state, da_w))
	MCFG_MM74C922_X1_CALLBACK(IOPORT("X1"))
	MCFG_MM74C922_X2_CALLBACK(IOPORT("X2"))
	MCFG_MM74C922_X3_CALLBACK(IOPORT("X3"))
	MCFG_MM74C922_X4_CALLBACK(IOPORT("X4"))

	MCFG_DEVICE_ADD(DM9368_H_TAG, DM9368, 0)
	MCFG_OUTPUT_NAME("digit0")
	MCFG_DEVICE_ADD(DM9368_L_TAG, DM9368, 0)
	MCFG_OUTPUT_NAME("digit1")

	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED)

	MCFG_QUICKLOAD_ADD("quickload", elf2_state, elf, "bin", 0)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("256")
MACHINE_CONFIG_END

/* ROMs */

ROM_START( elf2 )
	ROM_REGION( 0x10000, CDP1802_TAG, ROMREGION_ERASE00 )
ROM_END

/* System Drivers */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE INPUT   INIT    COMPANY         FULLNAME    FLAGS */
COMP( 1978, elf2,   0,      0,      elf2,   elf2, driver_device,    0,      "Netronics",    "Elf II",   MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND)
