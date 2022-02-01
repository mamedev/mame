// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha
/***************************************************************************

  machine.c

  Functions to emulate general aspects of PMD-85 (RAM, ROM, interrupts,
  I/O ports)

  Krzysztof Strzecha

***************************************************************************/

#include "emu.h"
#include "includes/pmd85.h"



enum {PMD85_LED_1 = 0, PMD85_LED_2, PMD85_LED_3};
enum {PMD85_1, PMD85_2, PMD85_2A, PMD85_2B, PMD85_3, ALFA, MATO, C2717};



void pmd85_state::pmd851_update_memory()
{
	address_space& space = m_maincpu->space(AS_PROGRAM);
	uint8_t *ram = m_ram->pointer();

	if (m_startup_mem_map)
	{
		space.unmap_write(0x0000, 0x0fff);
		space.nop_write(0x1000, 0x1fff);
		space.unmap_write(0x2000, 0x2fff);
		space.nop_write(0x3000, 0x3fff);

		space.nop_read(0x1000, 0x1fff);
		space.nop_read(0x3000, 0x3fff);

		m_bank[1]->set_base(m_rom);
		m_bank[3]->set_base(m_rom);
		m_bank[5]->set_base(ram + 0xc000);

		m_bank[6]->set_base(m_rom);
		m_bank[7]->set_base(m_rom);
		m_bank[8]->set_base(ram + 0xc000);
	}
	else
	{
		space.install_write_bank(0x0000, 0x0fff, m_bank[1]);
		space.install_write_bank(0x1000, 0x1fff, m_bank[2]);
		space.install_write_bank(0x2000, 0x2fff, m_bank[3]);
		space.install_write_bank(0x3000, 0x3fff, m_bank[4]);
		space.install_write_bank(0x4000, 0x7fff, m_bank[5]);

		space.install_read_bank(0x1000, 0x1fff, m_bank[2]);
		space.install_read_bank(0x3000, 0x3fff, m_bank[4]);

		m_bank[1]->set_base(ram);
		m_bank[2]->set_base(ram + 0x1000);
		m_bank[3]->set_base(ram + 0x2000);
		m_bank[4]->set_base(ram + 0x3000);
		m_bank[5]->set_base(ram + 0x4000);
	}
}

void pmd85_state::pmd852a_update_memory()
{
	address_space& space = m_maincpu->space(AS_PROGRAM);
	uint8_t *ram = m_ram->pointer();

	if (m_startup_mem_map)
	{
		space.unmap_write(0x0000, 0x0fff);
		space.unmap_write(0x2000, 0x2fff);

		m_bank[1]->set_base(m_rom);
		m_bank[2]->set_base(ram + 0x9000);
		m_bank[3]->set_base(m_rom);
		m_bank[4]->set_base(ram + 0xb000);
		m_bank[5]->set_base(ram + 0xc000);
		m_bank[6]->set_base(m_rom);
		m_bank[7]->set_base(ram + 0x9000);
		m_bank[8]->set_base(m_rom);
		m_bank[9]->set_base(ram + 0xb000);
		m_bank[10]->set_base(ram + 0xc000);

	}
	else
	{
		space.install_write_bank(0x0000, 0x0fff, m_bank[1]);
		space.install_write_bank(0x2000, 0x2fff, m_bank[3]);

		m_bank[1]->set_base(ram);
		m_bank[2]->set_base(ram + 0x1000);
		m_bank[3]->set_base(ram + 0x2000);
		m_bank[4]->set_base(ram + 0x5000);
		m_bank[5]->set_base(ram + 0x4000);
	}
}

void pmd85_state::pmd853_update_memory()
{
	uint8_t *ram = m_ram->pointer();

	if (m_startup_mem_map)
	{
		m_bank[1]->set_base(m_rom);
		m_bank[2]->set_base(m_rom);
		m_bank[3]->set_base(m_rom);
		m_bank[4]->set_base(m_rom);
		m_bank[5]->set_base(m_rom);
		m_bank[6]->set_base(m_rom);
		m_bank[7]->set_base(m_rom);
		m_bank[8]->set_base(m_rom);
		m_bank[9]->set_base(ram);
		m_bank[10]->set_base(ram + 0x2000);
		m_bank[11]->set_base(ram + 0x4000);
		m_bank[12]->set_base(ram + 0x6000);
		m_bank[13]->set_base(ram + 0x8000);
		m_bank[14]->set_base(ram + 0xa000);
		m_bank[15]->set_base(ram + 0xc000);
		m_bank[16]->set_base(ram + 0xe000);
	}
	else
	{
		m_bank[1]->set_base(ram);
		m_bank[2]->set_base(ram + 0x2000);
		m_bank[3]->set_base(ram + 0x4000);
		m_bank[4]->set_base(ram + 0x6000);
		m_bank[5]->set_base(ram + 0x8000);
		m_bank[6]->set_base(ram + 0xa000);
		m_bank[7]->set_base(ram + 0xc000);
		m_bank[8]->set_base(m_pmd853_memory_mapping ? m_rom : ram + 0xe000);
	}
}

void pmd85_state::alfa_update_memory()
{
	address_space& space = m_maincpu->space(AS_PROGRAM);
	uint8_t *ram = m_ram->pointer();

	if (m_startup_mem_map)
	{
		space.unmap_write(0x0000, 0x0fff);
		space.unmap_write(0x1000, 0x33ff);
		space.nop_write(0x3400, 0x3fff);

		m_bank[1]->set_base(m_rom);
		m_bank[2]->set_base(m_rom + 0x1000);
		m_bank[4]->set_base(ram + 0xc000);
		m_bank[5]->set_base(m_rom);
		m_bank[6]->set_base(m_rom + 0x1000);
		m_bank[7]->set_base(ram + 0xc000);
	}
	else
	{
		space.install_write_bank(0x0000, 0x0fff, m_bank[1]);
		space.install_write_bank(0x1000, 0x33ff, m_bank[2]);
		space.install_write_bank(0x3400, 0x3fff, m_bank[3]);

		m_bank[1]->set_base(ram);
		m_bank[2]->set_base(ram + 0x1000);
		m_bank[3]->set_base(ram + 0x3400);
		m_bank[4]->set_base(ram + 0x4000);
	}
}

void pmd85_state::mato_update_memory()
{
	address_space& space = m_maincpu->space(AS_PROGRAM);
	uint8_t *ram = m_ram->pointer();

	if (m_startup_mem_map)
	{
		space.unmap_write(0x0000, 0x3fff);

		m_bank[1]->set_base(m_rom);
		m_bank[2]->set_base(ram + 0xc000);
		m_bank[3]->set_base(m_rom);
		m_bank[4]->set_base(ram + 0xc000);
	}
	else
	{
		space.install_write_bank(0x0000, 0x3fff, m_bank[1]);

		m_bank[1]->set_base(ram);
		m_bank[2]->set_base(ram + 0x4000);
	}
}

void pmd85_state::c2717_update_memory()
{
	address_space& space = m_maincpu->space(AS_PROGRAM);
	uint8_t *ram = m_ram->pointer();

	if (m_startup_mem_map)
	{
		space.unmap_write(0x0000, 0x3fff);

		m_bank[1]->set_base(m_rom);
		m_bank[2]->set_base(ram + 0x4000);
		m_bank[3]->set_base(m_rom);
		m_bank[4]->set_base(ram + 0xc000);
	}
	else
	{
		space.install_write_bank(0x0000, 0x3fff, m_bank[1]);
		m_bank[1]->set_base(ram);
		m_bank[2]->set_base(ram + 0x4000);
	}
}

/*******************************************************************************

    Motherboard 8255 (PMD-85.1, PMD-85.2, PMD-85.3, Didaktik Alfa)
    --------------------------------------------------------------
        keyboard, speaker, LEDs

*******************************************************************************/

uint8_t pmd85_state::ppi0_porta_r()
{
	return 0xff;
}

uint8_t pmd85_state::ppi0_portb_r()
{
	return m_io_keyboard[(m_ppi_port_outputs[0][0] & 0x0f)]->read() & m_io_keyboard[15]->read();
}

uint8_t pmd85_state::ppi0_portc_r()
{
	return 0xff;
}

void pmd85_state::ppi0_porta_w(uint8_t data)
{
	m_ppi_port_outputs[0][0] = data;
}

void pmd85_state::ppi0_portb_w(uint8_t data)
{
	m_ppi_port_outputs[0][1] = data;
}

void pmd85_state::ppi0_portc_w(uint8_t data)
{
	m_ppi_port_outputs[0][2] = data;
	m_leds[PMD85_LED_2] = BIT(data, 3);
	//m_leds[PMD85_LED_3] = BIT(data, 2);
	m_speaker->level_w(BIT(data, 2));
}

/*******************************************************************************

    Motherboard 8255 (Mato)
    -----------------------
        keyboard, speaker, LEDs, tape

*******************************************************************************/

uint8_t pmd85_state::mato_ppi0_portb_r()
{
	u8 i,data = 0xff;

	for (i = 0; i < 8; i++)
		if (!BIT(m_ppi_port_outputs[0][0], i))
			data &= m_io_keyboard[i]->read();

	return data;
}

uint8_t pmd85_state::mato_ppi0_portc_r()
{
	u8 data = m_io_keyboard[8]->read() & 0x7f;
	data |= (m_cassette->input() > 0.038) ? 0x80 : 0;
	return data;
}

void pmd85_state::mato_ppi0_portc_w(uint8_t data)
{
	m_ppi_port_outputs[0][2] = data;
	m_leds[PMD85_LED_2] = BIT(data, 3);
	m_leds[PMD85_LED_3] = BIT(data, 2);
	m_speaker->level_w(BIT(data, 1));
	m_cassette->output(BIT(data, 0) ? 1 : -1);
}

/*******************************************************************************

    I/O board 8255
    --------------
        GPIO/0 (K3 connector), GPIO/1 (K4 connector)

*******************************************************************************/

uint8_t pmd85_state::ppi1_porta_r()
{
	return 0xff;
}

uint8_t pmd85_state::ppi1_portb_r()
{
	return 0xff;
}

uint8_t pmd85_state::ppi1_portc_r()
{
	return 0xff;
}

void pmd85_state::ppi1_porta_w(uint8_t data)
{
	m_ppi_port_outputs[1][0] = data;
}

void pmd85_state::ppi1_portb_w(uint8_t data)
{
	m_ppi_port_outputs[1][1] = data;
}

void pmd85_state::ppi1_portc_w(uint8_t data)
{
	m_ppi_port_outputs[1][2] = data;
}

/*******************************************************************************

    I/O board 8255
    --------------
        IMS-2 (K5 connector)

    - 8251 - cassette recorder and V.24/IFSS (selectable by switch)

    - external interfaces connector (K2)

*******************************************************************************/

uint8_t pmd85_state::ppi2_porta_r()
{
	return 0xff;
}

uint8_t pmd85_state::ppi2_portb_r()
{
	return 0xff;
}

uint8_t pmd85_state::ppi2_portc_r()
{
	return 0xff;
}

void pmd85_state::ppi2_porta_w(uint8_t data)
{
	m_ppi_port_outputs[2][0] = data;
}

void pmd85_state::ppi2_portb_w(uint8_t data)
{
	m_ppi_port_outputs[2][1] = data;
}

void pmd85_state::ppi2_portc_w(uint8_t data)
{
	m_ppi_port_outputs[2][2] = data;
}

/*******************************************************************************

    I/O board 8251
    --------------
        cassette recorder and V.24/IFSS (selectable by switch)

*******************************************************************************/

/*******************************************************************************

    I/O board external interfaces connector (K2)
    --------------------------------------------

*******************************************************************************/



/*******************************************************************************

    ROM Module 8255
    ---------------
        port A - data read
        ports B, C - address select

*******************************************************************************/

uint8_t pmd85_state::ppi3_porta_r()
{
	if (memregion("user1")->base())
		return memregion("user1")->base()[m_ppi_port_outputs[3][1] | (m_ppi_port_outputs[3][2] << 8)];
	else
		return 0;
}

uint8_t pmd85_state::ppi3_portb_r()
{
	return 0xff;
}

uint8_t pmd85_state::ppi3_portc_r()
{
	return 0xff;
}

void pmd85_state::ppi3_porta_w(uint8_t data)
{
	m_ppi_port_outputs[3][0] = data;
}

void pmd85_state::ppi3_portb_w(uint8_t data)
{
	m_ppi_port_outputs[3][1] = data;
}

void pmd85_state::ppi3_portc_w(uint8_t data)
{
	m_ppi_port_outputs[3][2] = data;
}

/*******************************************************************************

    I/O ports (PMD-85.1, PMD-85.2, PMD-85.3, Didaktik Alfa)
    -------------------------------------------------------

    I/O board
    1xxx11aa    external interfaces connector (K2)

    0xxx11aa    I/O board interfaces
        000111aa    8251 (casette recorder, V24)
        010011aa    8255 (GPIO/0, GPIO/1)
        010111aa    8253
        011111aa    8255 (IMS-2)

    Motherboard
    1xxx01aa    8255 (keyboard, speaker, LEDs)
            PMD-85.3 memory banking

    ROM Module
    1xxx10aa    8255 (ROM reading)

*******************************************************************************/

uint8_t pmd85_state::io_r(offs_t offset)
{
	if (m_startup_mem_map)
	{
		return 0xff;
	}

	switch (offset & 0x0c)
	{
		case 0x04:  /* Motherboard */
				switch (offset & 0x80)
				{
					case 0x80:  /* Motherboard 8255 */
							return m_ppi0->read(offset & 0x03);
				}
				break;
		case 0x08:  /* ROM module connector */
				if (m_rom_module_present)
				{
					switch (offset & 0x80)
					{
						case 0x80:  /* ROM module 8255 */
							return m_ppi3->read(offset & 0x03);
					}
				}
				break;
		case 0x0c:  /* I/O board */
				switch (offset & 0x80)
				{
					case 0x00:  /* I/O board interfaces */
							switch (offset & 0x70)
							{
								case 0x10:  /* 8251 (cassette recorder, V24) */
										return m_uart->read(offset & 0x01);
								case 0x40:  /* 8255 (GPIO/0, GPIO/1) */
										return m_ppi1->read(offset & 0x03);
								case 0x50:  /* 8253 */
										return m_pit->read(offset & 0x03);
								case 0x70:  /* 8255 (IMS-2) */
										return m_ppi2->read(offset & 0x03);
							}
							break;
					case 0x80:  /* external interfaces */
							break;
				}
				break;
	}
	if ((m_model == ALFA) && ((offset & 0xfe) == 0xf0))
		return m_uart->read(offset & 0x01);

	logerror ("Reading from unmapped port: %02x\n", offset);
	return 0xff;
}

void pmd85_state::io_w(offs_t offset, uint8_t data)
{
	if (m_startup_mem_map)
	{
		m_startup_mem_map = 0;
		(this->*update_memory)();
	}

	switch (offset & 0x0c)
	{
		case 0x04:  /* Motherboard */
				switch (offset & 0x80)
				{
					case 0x80:  /* Motherboard 8255 */
							m_ppi0->write(offset & 0x03, data);
							/* PMD-85.3 memory banking */
							if ((offset & 0x03) == 0x03)
							{
								m_pmd853_memory_mapping = data & 0x01;
								(this->*update_memory)();
							}
							break;
				}
				break;
		case 0x08:  /* ROM module connector */
				if (m_rom_module_present)
				{
					switch (offset & 0x80)
					{
						case 0x80:  /* ROM module 8255 */
							m_ppi3->write(offset & 0x03, data);
							break;
					}
				}
				break;
		case 0x0c:  /* I/O board */
				switch (offset & 0x80)
				{
					case 0x00:  /* I/O board interfaces */
							switch (offset & 0x70)
							{
								case 0x10:  /* 8251 (cassette recorder, V24) */
										m_uart->write(offset & 0x01, data);
										break;
								case 0x40:  /* 8255 (GPIO/0, GPIO/0) */
										m_ppi1->write(offset & 0x03, data);
										break;
								case 0x50:  /* 8253 */
										m_pit->write(offset & 0x03, data);
										logerror ("8253 writing. Address: %02x, Data: %02x\n", offset, data);
										break;
								case 0x70:  /* 8255 (IMS-2) */
										m_ppi2->write(offset & 0x03, data);
										break;
							}
							break;
					case 0x80:  /* external interfaces */
							break;
				}
				break;
	}
	if ((m_model == ALFA) && ((offset & 0xfe) == 0xf0))
		m_uart->write(offset & 0x01, data);
	//logerror ("Writing to unmapped port: %02x:%02X\n", offset,data);
}

/*******************************************************************************

    I/O ports (Mato)
    ----------------

    Motherboard
    1xxx01aa    8255 (keyboard, speaker, LEDs, tape)

*******************************************************************************/

uint8_t pmd85_state::mato_io_r(offs_t offset)
{
	if (m_startup_mem_map)
	{
		return 0xff;
	}

	switch (offset & 0x0c)
	{
		case 0x04:  /* Motherboard */
				switch (offset & 0x80)
				{
					case 0x80:  /* Motherboard 8255 */
							return m_ppi0->read(offset & 0x03);
				}
				break;
	}

	logerror ("Reading from unmapped port: %02x\n", offset);
	return 0xff;
}

void pmd85_state::mato_io_w(offs_t offset, uint8_t data)
{
	if (m_startup_mem_map)
	{
		m_startup_mem_map = 0;
		(this->*update_memory)();
	}

	switch (offset & 0x0c)
	{
		case 0x04:  /* Motherboard */
				switch (offset & 0x80)
				{
					case 0x80:  /* Motherboard 8255 */
							return m_ppi0->write(offset & 0x03, data);
				}
				break;
	}
}

void pmd85_state::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	switch (id)
	{
	case TIMER_CASSETTE:
		cassette_timer_callback(param);
		break;
	default:
		throw emu_fatalerror("Unknown id in pmd85_state::device_timer");
	}
}

TIMER_CALLBACK_MEMBER(pmd85_state::cassette_timer_callback)
{
	bool data;
	bool current_level;

	if (!BIT(m_io_dsw0->read(), 1))   /* V.24 / Tape Switch */
	{
		/* tape reading */
		if (m_cassette->get_state()&CASSETTE_PLAY)
		{
			switch (m_model)
			{
				case PMD85_1:
				case ALFA:
					if (m_clk_level_tape)
					{
						m_previous_level = (m_cassette->input() > 0.038) ? 1 : 0;
						m_clk_level_tape = 0;
					}
					else
					{
						current_level = (m_cassette->input() > 0.038) ? 1 : 0;

						if (m_previous_level!=current_level)
						{
							data = (!m_previous_level && current_level) ? 1 : 0;

							m_uart->write_rxd(data);

							m_clk_level_tape = 1;
						}
					}

					m_uart->write_rxc(m_clk_level_tape);
					return;
				case PMD85_2:
				case PMD85_2A:
				case C2717:
				case PMD85_3:
					// works for pmd852, pmd852a, pmd852b, pmd853, c2717, c2717pmd
					m_uart->write_dsr( (m_cassette->input() > 0.038) ? 0 : 1);
					return;
			}
		}

		/* tape writing */
		if (m_cassette->get_state()&CASSETTE_RECORD)
		{
			m_cassette->output((m_txd ^ m_clk_level_tape) ? 1 : -1);

			m_clk_level_tape = m_clk_level_tape ? 0 : 1;
			m_uart->write_txc(m_clk_level_tape);

			return;
		}

		m_clk_level_tape = 1;

		m_clk_level = m_clk_level ? 0 : 1;
		m_uart->write_txc(m_clk_level);
	}
}

INPUT_CHANGED_MEMBER(pmd85_state::pmd85_reset)
{
	machine().schedule_soft_reset();
}

void pmd85_state::common_driver_init()
{
	m_previous_level = 0;
	m_clk_level = m_clk_level_tape = 1;
	m_cassette_timer = timer_alloc(TIMER_CASSETTE);
	m_cassette_timer->adjust(attotime::zero, 0, attotime::from_hz(2400));
}

void pmd85_state::init_pmd851()
{
	m_model = PMD85_1;
	update_memory = &pmd85_state::pmd851_update_memory;
	common_driver_init();
}

void pmd85_state::init_pmd852()
{
	m_model = PMD85_2;
	update_memory = &pmd85_state::pmd851_update_memory;
	common_driver_init();
}

void pmd85_state::init_pmd852a()
{
	m_model = PMD85_2A;
	update_memory = &pmd85_state::pmd852a_update_memory;
	common_driver_init();
}

void pmd85_state::init_pmd853()
{
	m_model = PMD85_3;
	update_memory = &pmd85_state::pmd853_update_memory;
	common_driver_init();
}

void pmd85_state::init_alfa()
{
	m_model = ALFA;
	update_memory = &pmd85_state::alfa_update_memory;
	common_driver_init();
}

void pmd85_state::init_mato()
{
	m_model = MATO;
	update_memory = &pmd85_state::mato_update_memory;
}

void pmd85_state::init_c2717()
{
	m_model = C2717;
	update_memory = &pmd85_state::c2717_update_memory;
	common_driver_init();
}

void pmd85_state::machine_reset()
{
	/* checking for Rom Module */
	m_rom_module_present = 0;
	switch (m_model)
	{
		case PMD85_1:
		case PMD85_2:
		case PMD85_2A:
		case PMD85_3:
		case C2717:
			m_rom_module_present = BIT(m_io_dsw0->read(), 0);
			break;
		case ALFA:
		case MATO:
			break;
	}

	for (u8 i = 0; i < 4; i++)
		for (u8 j = 0; j < 3; j++)
			m_ppi_port_outputs[i][j] = 0;

	/* memory initialization */
	m_pmd853_memory_mapping = 1;
	m_startup_mem_map = 1;
	(this->*update_memory)();
}

void pmd85_state::machine_start()
{
	m_leds.resolve();
	save_item(NAME(m_txd));
	save_item(NAME(m_rts));
	save_item(NAME(m_rom_module_present));
	save_item(NAME(m_ppi_port_outputs));
	save_item(NAME(m_startup_mem_map));
	save_item(NAME(m_pmd853_memory_mapping));
	save_item(NAME(m_previous_level));
	save_item(NAME(m_clk_level));
	save_item(NAME(m_clk_level_tape));
	save_item(NAME(m_model));
}

