// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    Netronics Elf II

****************************************************************************/

#include "emu.h"
#include "elf.h"
#include "screen.h"
#include "elf2.lh"

#define LOAD \
	BIT(m_special->read(), 1)

#define MEMORY_PROTECT \
	BIT(m_special->read(), 2)

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

uint8_t elf2_state::memory_r(offs_t offset)
{
	return m_ram->pointer()[offset];
}

void elf2_state::memory_w(offs_t offset, uint8_t data)
{
	if (MEMORY_PROTECT)
	{
		/* latch data from memory */
		data = m_ram->pointer()[offset];
	}
	else
	{
		m_ram->pointer()[offset] = data;
	}

	if (LOAD)
	{
		/* write data to 7 segment displays */
		m_led_l->a_w(data & 0x0f);
		m_led_h->a_w(data >> 4);
	}
}

/* Memory Maps */

void elf2_state::elf2_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).mirror(0xff00).rw(FUNC(elf2_state::memory_r), FUNC(elf2_state::memory_w));
}

void elf2_state::elf2_io(address_map &map)
{
	map.unmap_value_high();
	map(0x01, 0x01).r(FUNC(elf2_state::dispon_r));
	map(0x04, 0x04).rw(FUNC(elf2_state::data_r), FUNC(elf2_state::data_w));
}

/* Input Ports */

INPUT_CHANGED_MEMBER(elf2_state::load_w)
{
	/* DMAIN is reset while LOAD is off */
	if (!newval)
		m_dmain = 0;
}

INPUT_CHANGED_MEMBER(elf2_state::input_w)
{
	/* assert DMAIN */
	if (LOAD && !newval && ~m_sc & 2)
		m_dmain = 1;
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
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("LOAD") PORT_CODE(KEYCODE_L) PORT_TOGGLE PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(elf2_state::load_w), 0)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("MP") PORT_CODE(KEYCODE_M) PORT_TOGGLE
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("IN") PORT_CODE(KEYCODE_ENTER) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(elf2_state::input_w), 0)
INPUT_PORTS_END

/* CDP1802 Configuration */

void elf2_state::sc_w(uint8_t data)
{
	/* DMAIN is reset while SC1 is high */
	if (data & 2)
		m_dmain = 0;

	m_sc = data;
}

/* MM74C923 Interface */

void elf2_state::da_w(int state)
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
	/* register for state saving */
	save_item(NAME(m_data));
	save_item(NAME(m_sc));
	save_item(NAME(m_dmain));
}

/* Machine Driver */

QUICKLOAD_LOAD_MEMBER(elf2_state::quickload_cb)
{
	int const size = image.length();

	if (size > m_ram->size())
	{
		return std::make_pair(image_error::INVALIDLENGTH, std::string());
	}

	image.fread(m_ram->pointer(), size);

	return std::make_pair(std::error_condition(), std::string());
}

void elf2_state::elf2(machine_config &config)
{
	/* basic machine hardware */
	CDP1802(config, m_maincpu, XTAL(3'579'545)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &elf2_state::elf2_mem);
	m_maincpu->set_addrmap(AS_IO, &elf2_state::elf2_io);
	m_maincpu->wait_cb().set_ioport("SPECIAL").bit(1).invert();
	m_maincpu->clear_cb().set_ioport("SPECIAL").bit(0);
	m_maincpu->dma_in_cb().set([this]() { return m_dmain; });
	m_maincpu->ef2_cb().set(m_exp, FUNC(elf2_expansion_bus_device::ef2_r));
	m_maincpu->ef3_cb().set(m_exp, FUNC(elf2_expansion_bus_device::ef3_r));
	m_maincpu->ef4_cb().set([this]() { return BIT(m_special->read(), 3) | m_exp->ef4_r(); });
	m_maincpu->q_cb().set_output("led0");
	m_maincpu->q_cb().append(m_exp, FUNC(elf2_expansion_bus_device::q_w));
	m_maincpu->dma_rd_cb().set(FUNC(elf2_state::data_r));
	m_maincpu->dma_wr_cb().set(m_vdc, FUNC(cdp1861_device::dma_w));
	m_maincpu->sc_cb().set(FUNC(elf2_state::sc_w));
	m_maincpu->sc_cb().append(m_vdc, FUNC(cdp1861_device::sc_w));
	m_maincpu->tpa_cb().set(m_vdc, FUNC(cdp1861_device::tpa_w));
	m_maincpu->int_cb().set(m_vdc, FUNC(cdp1861_device::int_r));
	m_maincpu->dma_out_cb().set(m_vdc, FUNC(cdp1861_device::dma_out_r));
	m_maincpu->ef1_cb().set([this]() { return m_vdc->efx_r() | m_exp->ef1_r(); });

	/* video hardware */
	config.set_default_layout(layout_elf2);

	CDP1861(config, m_vdc, XTAL(3'579'545)/2).set_screen(SCREEN_TAG);
	SCREEN(config, SCREEN_TAG);

	/* devices */
	MM74C923(config, m_kb);
	m_kb->set_cap_osc(CAP_U(0.15));
	m_kb->set_cap_debounce(CAP_U(1));
	m_kb->da_wr_callback().set(FUNC(elf2_state::da_w));
	m_kb->x1_rd_callback().set_ioport("X1");
	m_kb->x2_rd_callback().set_ioport("X2");
	m_kb->x3_rd_callback().set_ioport("X3");
	m_kb->x4_rd_callback().set_ioport("X4");

	DM9368(config, m_led_h).update_cb().set_output("digit0");
	DM9368(config, m_led_l).update_cb().set_output("digit1");

	ELF2_EXPANSION_BUS(config, m_exp);
	m_exp->set_program_space(m_maincpu, AS_PROGRAM);
	m_exp->set_io_space(m_maincpu, AS_IO);
	ELF2_EXPANSION_SLOT(config, "slot1", m_exp, elf2_expansion_cards, "giant");
	ELF2_EXPANSION_SLOT(config, "slot2", m_exp, elf2_expansion_cards, nullptr);
	ELF2_EXPANSION_SLOT(config, "slot3", m_exp, elf2_expansion_cards, nullptr);
	ELF2_EXPANSION_SLOT(config, "slot4", m_exp, elf2_expansion_cards, nullptr);
	ELF2_EXPANSION_SLOT(config, "slot5", m_exp, elf2_expansion_cards, nullptr);

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
COMP( 1978, elf2, 0,      0,      elf2,    elf2,  elf2_state, empty_init, "Netronics", "Elf II", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW )
