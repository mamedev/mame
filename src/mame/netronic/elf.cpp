// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Netronics Elf II

****************************************************************************/

/*

    TODO:

    - proper layout

*/

#include "emu.h"
#include "elf.h"
#include "screen.h"
#include "speaker.h"
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

uint8_t elf2_state::dispon_r()
{
	m_vdc->disp_on_w(1);
	m_vdc->disp_on_w(0);

	return 0xff;
}

uint8_t elf2_state::data_r()
{
	return m_data;
}

void elf2_state::data_w(uint8_t data)
{
	m_led_l->a_w(data & 0x0f);
	m_led_h->a_w(data >> 4);
}

void elf2_state::memory_w(offs_t offset, uint8_t data)
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

void elf2_state::elf2_mem(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	// Ram is added dynamically
}

void elf2_state::elf2_io(address_map &map)
{
	map.unmap_value_high();
	map(0x01, 0x01).r(FUNC(elf2_state::dispon_r));
	map(0x04, 0x04).rw(FUNC(elf2_state::data_r), FUNC(elf2_state::data_w));
}

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
	m_led = state ? 1 : 0;
}

uint8_t elf2_state::dma_r()
{
	return m_data;
}

void elf2_state::sc_w(uint8_t data)
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

	m_led.resolve();

	/* initialize LED displays */
	m_7segs.resolve();
	m_led_l->rbi_w(1);
	m_led_h->rbi_w(1);

	/* setup memory banking */
	program.install_rom(0x0000, 0x00ff, m_ram->pointer());
	program.install_write_handler(0x0000, 0x00ff, write8sm_delegate(*this, FUNC(elf2_state::memory_w)));

	/* register for state saving */
	save_item(NAME(m_data));
}

/* Machine Driver */

QUICKLOAD_LOAD_MEMBER(elf2_state::quickload_cb)
{
	int size = image.length();

	if (size > m_ram->size())
	{
		return image_init_result::FAIL;
	}

	image.fread(m_ram->pointer(), size);

	return image_init_result::PASS;
}

void elf2_state::elf2(machine_config &config)
{
	/* basic machine hardware */
	CDP1802(config, m_maincpu, XTAL(3'579'545)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &elf2_state::elf2_mem);
	m_maincpu->set_addrmap(AS_IO, &elf2_state::elf2_io);
	m_maincpu->wait_cb().set(FUNC(elf2_state::wait_r));
	m_maincpu->clear_cb().set(FUNC(elf2_state::clear_r));
	m_maincpu->ef4_cb().set(FUNC(elf2_state::ef4_r));
	m_maincpu->q_cb().set(FUNC(elf2_state::q_w));
	m_maincpu->dma_rd_cb().set(FUNC(elf2_state::dma_r));
	m_maincpu->dma_wr_cb().set(m_vdc, FUNC(cdp1861_device::dma_w));
	m_maincpu->sc_cb().set(FUNC(elf2_state::sc_w));

	/* video hardware */
	config.set_default_layout(layout_elf2);

	CDP1861(config, m_vdc, XTAL(3'579'545)/2).set_screen(SCREEN_TAG);
	m_vdc->int_cb().set_inputline(m_maincpu, COSMAC_INPUT_LINE_INT);
	m_vdc->dma_out_cb().set_inputline(m_maincpu,  COSMAC_INPUT_LINE_DMAOUT);
	m_vdc->efx_cb().set_inputline(m_maincpu, COSMAC_INPUT_LINE_EF1);
	SCREEN(config, SCREEN_TAG, SCREEN_TYPE_RASTER);

	/* devices */
	MM74C923(config, m_kb, 0);
	m_kb->set_cap_osc(CAP_U(0.15));
	m_kb->set_cap_debounce(CAP_U(1));
	m_kb->da_wr_callback().set(FUNC(elf2_state::da_w));
	m_kb->x1_rd_callback().set_ioport("X1");
	m_kb->x2_rd_callback().set_ioport("X2");
	m_kb->x3_rd_callback().set_ioport("X3");
	m_kb->x4_rd_callback().set_ioport("X4");

	DM9368(config, m_led_h, 0).update_cb().set(FUNC(elf2_state::digit_w<0>));
	DM9368(config, m_led_l, 0).update_cb().set(FUNC(elf2_state::digit_w<1>));

	SPEAKER(config, "mono").front_center();

	CASSETTE(config, m_cassette);
	m_cassette->set_default_state(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_ENABLED);
	m_cassette->add_route(ALL_OUTPUTS, "mono", 0.05);

	QUICKLOAD(config, "quickload", "bin").set_load_callback(FUNC(elf2_state::quickload_cb));

	/* internal ram */
	RAM(config, RAM_TAG).set_default_size("256");
}

/* ROMs */

ROM_START( elf2 )
	ROM_REGION( 0x10000, CDP1802_TAG, ROMREGION_ERASE00 )
ROM_END

/* System Drivers */

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS       INIT        COMPANY      FULLNAME  FLAGS
COMP( 1978, elf2, 0,      0,      elf2,    elf2,  elf2_state, empty_init, "Netronics", "Elf II", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND)
