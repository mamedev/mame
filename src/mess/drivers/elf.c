/***************************************************************************

    Netronics Elf II

****************************************************************************/

/*

    TODO:

    - proper layout

*/

#include "includes/elf.h"
#include "elf2.lh"

#define RUN	\
	BIT(ioport("SPECIAL")->read(), 0)

#define LOAD \
	BIT(ioport("SPECIAL")->read(), 1)

#define MEMORY_PROTECT \
	BIT(ioport("SPECIAL")->read(), 2)

#define INPUT \
	BIT(ioport("SPECIAL")->read(), 3)

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

static COSMAC_SC_WRITE( elf2_sc_w )
{
	elf2_state *state = device->machine().driver_data<elf2_state>();

	switch (sc)
	{
	case COSMAC_STATE_CODE_S2_DMA:
	case COSMAC_STATE_CODE_S3_INTERRUPT:
		/* clear DMAIN */
		state->m_maincpu->set_input_line(COSMAC_INPUT_LINE_DMAIN, CLEAR_LINE);
		break;

	default:
		break;
	}
}

WRITE_LINE_MEMBER( elf2_state::q_w )
{
	output_set_led_value(0, state);
}

READ8_MEMBER( elf2_state::dma_r )
{
	return m_data;
}

static COSMAC_INTERFACE( elf2_config )
{
	DEVCB_DRIVER_LINE_MEMBER(elf2_state, wait_r),
	DEVCB_DRIVER_LINE_MEMBER(elf2_state, clear_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(elf2_state, ef4_r),
	DEVCB_DRIVER_LINE_MEMBER(elf2_state, q_w),
	DEVCB_DRIVER_MEMBER(elf2_state, dma_r),
	DEVCB_DEVICE_MEMBER(CDP1861_TAG, cdp1861_device, dma_w),
	elf2_sc_w,
	DEVCB_NULL,
	DEVCB_NULL
};

/* MM74C923 Interface */

WRITE_LINE_MEMBER( elf2_state::da_w )
{
	if (state)
	{
		/* shift keyboard data to latch */
		m_data <<= 4;
		m_data |= m_kb->data_out_r() & 0x0f;

		if (LOAD)
		{
			/* write data to 7 segment displays */
			m_led_l->a_w(m_data & 0x0f);
			m_led_h->a_w(m_data >> 4);
		}
	}
}

static MM74C923_INTERFACE( keyboard_intf )
{
	CAP_U(0.15),
	CAP_U(1),
	DEVCB_DRIVER_LINE_MEMBER(elf2_state, da_w),
	DEVCB_INPUT_PORT("X1"),
	DEVCB_INPUT_PORT("X2"),
	DEVCB_INPUT_PORT("X3"),
	DEVCB_INPUT_PORT("X4"),
	DEVCB_NULL
};

/* CDP1861 Interface */

static CDP1861_INTERFACE( elf2_cdp1861_intf )
{
	CDP1802_TAG,
	SCREEN_TAG,
	DEVCB_CPU_INPUT_LINE(CDP1802_TAG, COSMAC_INPUT_LINE_INT),
	DEVCB_CPU_INPUT_LINE(CDP1802_TAG, COSMAC_INPUT_LINE_DMAOUT),
	DEVCB_CPU_INPUT_LINE(CDP1802_TAG, COSMAC_INPUT_LINE_EF1)
};

/* Machine Initialization */

void elf2_state::machine_start()
{
	address_space *program = m_maincpu->space(AS_PROGRAM);

	/* initialize LED displays */
	m_led_l->rbi_w(1);
	m_led_h->rbi_w(1);

	/* setup memory banking */
	program->install_read_bank(0x0000, 0x00ff, "bank1");
	program->install_write_handler(0x0000, 0x00ff, write8_delegate(FUNC(elf2_state::memory_w), this));
	membank("bank1")->configure_entry(0, m_ram->pointer());
	membank("bank1")->set_entry(0);

	/* register for state saving */
	save_item(NAME(m_data));
}

/* Machine Driver */

static const cassette_interface elf_cassette_interface =
{
	cassette_default_formats,
	NULL,
	(cassette_state)(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED),
	NULL,
	NULL
};

static DM9368_INTERFACE( led_h_intf )
{
	0,
	DEVCB_NULL,
	DEVCB_NULL
};

static DM9368_INTERFACE( led_l_intf )
{
	1,
	DEVCB_NULL,
	DEVCB_NULL
};

static QUICKLOAD_LOAD( elf )
{
	elf2_state *state = image.device().machine().driver_data<elf2_state>();

	int size = image.length();

	if (size > state->m_ram->size())
	{
		return IMAGE_INIT_FAIL;
	}

	image.fread(state->m_ram->pointer(), size);

	return IMAGE_INIT_PASS;
}

static MACHINE_CONFIG_START( elf2, elf2_state )
	/* basic machine hardware */
    MCFG_CPU_ADD(CDP1802_TAG, COSMAC, XTAL_3_579545MHz/2)
    MCFG_CPU_PROGRAM_MAP(elf2_mem)
    MCFG_CPU_IO_MAP(elf2_io)
	MCFG_CPU_CONFIG(elf2_config)

    /* video hardware */
	MCFG_DEFAULT_LAYOUT( layout_elf2 )

	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_UPDATE_DEVICE(CDP1861_TAG, cdp1861_device, screen_update)
	MCFG_SCREEN_RAW_PARAMS(XTAL_3_579545MHz/2, CDP1861_SCREEN_WIDTH, CDP1861_HBLANK_END, CDP1861_HBLANK_START, CDP1861_TOTAL_SCANLINES, CDP1861_SCANLINE_VBLANK_END, CDP1861_SCANLINE_VBLANK_START)

	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(black_and_white)

	MCFG_MM74C923_ADD(MM74C923_TAG, keyboard_intf)
	MCFG_DM9368_ADD(DM9368_H_TAG, led_h_intf)
	MCFG_DM9368_ADD(DM9368_L_TAG, led_l_intf)
	MCFG_CDP1861_ADD(CDP1861_TAG, XTAL_3_579545MHz/2, elf2_cdp1861_intf)

	/* devices */
	MCFG_CASSETTE_ADD(CASSETTE_TAG, elf_cassette_interface)
	MCFG_QUICKLOAD_ADD("quickload", elf, "bin", 0)

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
COMP( 1978, elf2,	0,		0,		elf2,	elf2, driver_device,	0,		"Netronics",	"Elf II",	GAME_SUPPORTS_SAVE | GAME_NO_SOUND)
