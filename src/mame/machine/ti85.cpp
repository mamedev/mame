// license:BSD-3-Clause
// copyright-holders:Krzysztof Strzecha,Jon Sturm
/***************************************************************************
  TI-85 driver by Krzysztof Strzecha
  TI-83 Plus, TI-84 Plus, and Silver Edition support by Jon Sturm

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include <cstdarg>
#include "emu.h"
#include "cpu/z80/z80.h"
#include "includes/ti85.h"

#define TI85_SNAPSHOT_SIZE   32976
#define TI86_SNAPSHOT_SIZE  131284

TIMER_CALLBACK_MEMBER(ti85_state::ti85_timer_callback)
{
	if (ioport("ON")->read() & 0x01)
	{
		if (m_ON_interrupt_mask && !m_ON_pressed)
		{
			m_maincpu->set_input_line(0, HOLD_LINE);
			m_ON_interrupt_status = 1;
			if (!m_timer_interrupt_mask) m_timer_interrupt_mask = 2;
		}
		m_ON_pressed = 1;
		return;
	}
	else
		m_ON_pressed = 0;
	if (m_timer_interrupt_mask)
	{
		m_maincpu->set_input_line(0, HOLD_LINE);
		m_timer_interrupt_status = m_timer_interrupt_mask;
	}
}

TIMER_CALLBACK_MEMBER(ti85_state::ti83_timer1_callback)
{
	if (ioport("ON")->read() & 0x01)
	{
		if (m_ON_interrupt_mask && !m_ON_pressed)
		{
			m_maincpu->set_input_line(0, HOLD_LINE);
			m_ON_interrupt_status = 1;
		}
		m_ON_pressed = 1;
		return;
	}
	else
	{
		m_ON_pressed = 0;
	}
	if (m_timer_interrupt_mask & 2)
	{
		m_maincpu->set_input_line(0, HOLD_LINE);
		m_timer_interrupt_status = m_timer_interrupt_status | 2;
	}
}

TIMER_CALLBACK_MEMBER(ti85_state::ti83_timer2_callback)
{
	if (m_timer_interrupt_mask & 4)
	{
		m_maincpu->set_input_line(0, HOLD_LINE);
		m_timer_interrupt_status = m_timer_interrupt_status | 4;
	}
}

void ti85_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case CRYSTAL_TIMER3:
	case CRYSTAL_TIMER2:
	case CRYSTAL_TIMER1:
		if (m_ctimer[id].count)
		{
			m_ctimer[id].count--;
			if (!m_ctimer[id].count)
			{
				if (!(m_ctimer[id].loop & 4))
				{
					if (!(m_ctimer[id].loop & 1))
					{
						m_ctimer[id].setup = 0;
					}
					else
					{
						ti83pse_count(id, m_ctimer[id].max);
					}
					if (!(m_ctimer[id].loop & 2))
					{
						//generate interrupt
						m_ctimer_interrupt_status |= (0x20 << id);
						m_maincpu->set_input_line(0, HOLD_LINE);
					}
					m_ctimer[id].loop &= 2;
				}
			}
		}
		break;
	case HW_TIMER1:
		if (ioport("ON")->read() & 0x01)
		{
			if (m_ON_interrupt_mask && !m_ON_pressed)
			{
				m_maincpu->set_input_line(0, HOLD_LINE);
				m_ON_interrupt_status = 1;
			}
			m_ON_pressed = 1;
			return;
		}
		else
		{
			m_ON_pressed = 0;
		}
		if (m_timer_interrupt_mask & 2)
		{
			m_maincpu->set_input_line(0, HOLD_LINE);
			m_timer_interrupt_status = m_timer_interrupt_status | 2;
		}
		break;
	case HW_TIMER2:
		if (m_timer_interrupt_mask & 4)
		{
			m_maincpu->set_input_line(0, HOLD_LINE);
			m_timer_interrupt_status = m_timer_interrupt_status | 4;
		}
	}
}

void ti85_state::ti8x_update_bank(address_space &space, uint8_t bank, uint8_t *base, uint8_t page, bool is_ram)
{
	static const char *const tag[] = {"bank1", "bank2", "bank3", "bank4"};

	membank(tag[bank&3])->set_base(base + (0x4000 * page));

	if (is_ram)
		space.install_write_bank(bank * 0x4000, bank * 0x4000 + 0x3fff, membank(tag[bank&3]));
	else
		space.nop_write(bank * 0x4000, bank * 0x4000 + 0x3fff);
}

void ti85_state::update_ti85_memory ()
{
	membank("bank2")->set_base(m_bios + 0x004000*m_ti8x_memory_page_1);
}

void ti85_state::update_ti83p_memory ()
{
	//address_space &space = m_maincpu->space(AS_PROGRAM);

	m_membank[0]->set_bank(m_booting ? 0x1f : 0); //Always flash page 0, well almost

	if (m_ti83p_port4 & 1)
	{
		m_membank[1]->set_bank(m_ti8x_memory_page_1 & 0xfe);

		m_membank[2]->set_bank(m_ti8x_memory_page_1);

		m_membank[3]->set_bank(m_ti8x_memory_page_2);

	}
	else
	{
		m_membank[1]->set_bank(m_ti8x_memory_page_1);

		m_membank[2]->set_bank(m_ti8x_memory_page_2);

		m_membank[3]->set_bank(0x40); //Always first ram page

	}
}

void ti85_state::update_ti83pse_memory ()
{
	//address_space &space = m_maincpu->space(AS_PROGRAM);

	m_membank[0]->set_bank(m_booting ? (m_model==TI84P ? 0x3f : 0x7f) : 0);

	if (m_ti83p_port4 & 1)
	{
		m_membank[1]->set_bank(m_ti8x_memory_page_1 & 0xfe);

		m_membank[2]->set_bank(m_ti8x_memory_page_1 | 1);

		m_membank[3]->set_bank(m_ti8x_memory_page_2);


	}
	else
	{
		m_membank[1]->set_bank(m_ti8x_memory_page_1);

		m_membank[2]->set_bank(m_ti8x_memory_page_2);

		m_membank[3]->set_bank(m_ti8x_memory_page_3 + 0x80);

	}
}

void ti85_state::update_ti86_memory ()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	if (m_ti8x_memory_page_1 & 0x40)
	{
		ti8x_update_bank(space, 1, m_ti8x_ram.get(), m_ti8x_memory_page_1 & 0x07, true);
	}
	else
	{
		ti8x_update_bank(space, 1, m_bios, m_ti8x_memory_page_1 & 0x0f, false);
	}

	if (m_ti8x_memory_page_2 & 0x40)
	{
		ti8x_update_bank(space, 2, m_ti8x_ram.get(), m_ti8x_memory_page_2 & 0x07, true);
	}
	else
	{
		ti8x_update_bank(space, 2, m_bios, m_ti8x_memory_page_2 & 0x0f, false);
	}

}

/***************************************************************************
  Machine Initialization
***************************************************************************/

void ti85_state::machine_start()
{
	m_model = TI85;

	address_space &space = m_maincpu->space(AS_PROGRAM);
	m_bios = memregion("bios")->base();

	m_timer_interrupt_mask = 0;
	m_timer_interrupt_status = 0;
	m_ON_interrupt_mask = 0;
	m_ON_interrupt_status = 0;
	m_ON_pressed = 0;
	m_power_mode = 0;
	m_keypad_mask = 0;
	m_ti8x_memory_page_1 = 0;
	m_LCD_memory_base = 0;
	m_LCD_status = 0;
	m_LCD_mask = 0;
	m_video_buffer_width = 0;
	m_interrupt_speed = 0;
	m_port4_bit0 = 0;
	m_ti81_port_7_data = 0;

	m_ti85_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ti85_state::ti85_timer_callback), this));
	m_ti85_timer->adjust(attotime::from_hz(256), 0, attotime::from_hz(256));

	space.unmap_write(0x0000, 0x3fff);
	space.unmap_write(0x4000, 0x7fff);
	membank("bank1")->set_base(m_bios);
	membank("bank2")->set_base(m_bios + 0x04000);
}

MACHINE_RESET_MEMBER(ti85_state,ti85)
{
	m_PCR = 0xc0;
}

uint8_t ti85_state::ti83p_membank2_r(offs_t offset)
{
	/// http://wikiti.brandonw.net/index.php?title=83Plus:State_of_the_calculator_at_boot
	/// should only trigger when fetching opcodes
	if (m_booting && !machine().side_effects_disabled())
	{
		m_booting = false;

		if (m_model == TI83P)
		{
			update_ti83p_memory();
		}
		else
		{
			update_ti83pse_memory();
		}
	}

	return m_membank[1]->read8(offset);
}

uint8_t ti85_state::ti83p_membank3_r(offs_t offset)
{
	/// http://wikiti.brandonw.net/index.php?title=83Plus:State_of_the_calculator_at_boot
	/// should only trigger when fetching opcodes
	/// should be using port 6 instead of 4
	if (m_booting && (m_ti83p_port4 & 1) && !machine().side_effects_disabled())
	{
		m_booting = false;

		if (m_model == TI83P)
		{
			update_ti83p_memory();
		}
		else
		{
			update_ti83pse_memory();
		}
	}

	return m_membank[2]->read8(offset);
}

MACHINE_RESET_MEMBER(ti85_state,ti83p)
{
	m_PCR = 0x00;


	m_ti8x_memory_page_1 = 0;
	m_ti8x_memory_page_2 = 0;
	m_ti8x_memory_page_3 = 0;
	m_ti83p_port4 = 1;
	m_booting = true;
	if (m_model == TI83P)
	{
		update_ti83p_memory();
	}
	else
	{
		update_ti83pse_memory();
	}
}

MACHINE_START_MEMBER(ti85_state,ti83p)
{
	m_model = TI83P;
	//address_space &space = m_maincpu->space(AS_PROGRAM);
	//m_bios = memregion("flash")->base();

	m_timer_interrupt_mask = 0;
	m_timer_interrupt_status = 0;
	m_ON_interrupt_mask = 0;
	m_ON_interrupt_status = 0;
	m_ON_pressed = 0;
	m_ti8x_memory_page_1 = 0;
	m_ti8x_memory_page_2 = 0;
	m_ti8x_memory_page_3 = 0;
	m_LCD_memory_base = 0;
	m_LCD_status = 0;
	m_LCD_mask = 0;
	m_power_mode = 0;
	m_keypad_mask = 0;
	m_video_buffer_width = 0;
	m_interrupt_speed = 0;
	m_ti83p_port4 = 1;
	m_flash_unlocked = 0;

	m_booting = true;

	ti85_state::update_ti83p_memory();

	m_ti83_1st_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ti85_state::ti83_timer1_callback), this));
	m_ti83_1st_timer->adjust(attotime::from_hz(256), 0, attotime::from_hz(256));
	m_ti83_2nd_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ti85_state::ti83_timer2_callback), this));
	m_ti83_2nd_timer->adjust(attotime::from_hz(512), 0, attotime::from_hz(512));

	/* save states and debugging */
	save_item(NAME(m_timer_interrupt_status));
	save_item(NAME(m_timer_interrupt_mask));
	save_item(NAME(m_ti8x_memory_page_1));
	save_item(NAME(m_ti8x_memory_page_2));
	save_item(NAME(m_ti8x_memory_page_3));
	save_item(NAME(m_ti83p_port4));
	save_item(NAME(m_booting));
}

void ti85_state::ti8xpse_init_common()
{
	//address_space &space = m_maincpu->space(AS_PROGRAM);

	m_timer_interrupt_mask = 0;
	m_timer_interrupt_status = 0;
	m_ctimer_interrupt_status = 0;
	m_ON_interrupt_mask = 0;
	m_ON_interrupt_status = 0;
	m_ON_pressed = 0;
	m_ti8x_memory_page_1 = 0;
	m_ti8x_memory_page_2 = 0;
	m_ti8x_memory_page_3 = 0;
	m_LCD_memory_base = 0;
	m_LCD_status = 0;
	m_LCD_mask = 0;
	m_power_mode = 0;
	m_keypad_mask = 0;
	m_video_buffer_width = 0;
	m_interrupt_speed = 0;
	m_ti83p_port4 = 1;
	m_flash_unlocked = 0;

	ti85_state::update_ti83pse_memory();

	m_ti83_1st_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ti85_state::ti83_timer1_callback), this));
	m_ti83_1st_timer->adjust(attotime::from_hz(256), 0, attotime::from_hz(256));
	m_ti83_2nd_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ti85_state::ti83_timer2_callback), this));
	m_ti83_2nd_timer->adjust(attotime::from_hz(512), 0, attotime::from_hz(512));

	m_crystal_timer1 = timer_alloc(CRYSTAL_TIMER1);
	m_crystal_timer2 = timer_alloc(CRYSTAL_TIMER2);
	m_crystal_timer3 = timer_alloc(CRYSTAL_TIMER3);

		/* save states and debugging */
	save_item(NAME(m_ctimer_interrupt_status));
	save_item(NAME(m_timer_interrupt_status));
	save_item(NAME(m_ti8x_memory_page_1));
	save_item(NAME(m_ti8x_memory_page_2));
	save_item(NAME(m_ti8x_memory_page_3));
	save_item(NAME(m_ti83p_port4));
}


MACHINE_START_MEMBER(ti85_state,ti83pse)
{
	m_model = TI84PSE;

	ti8xpse_init_common();
}

MACHINE_START_MEMBER(ti85_state,ti84pse)
{
	m_model = TI83PSE;

	ti8xpse_init_common();
}

MACHINE_START_MEMBER(ti85_state,ti84p)
{
	m_model = TI84P;

	ti8xpse_init_common();
}

MACHINE_START_MEMBER(ti85_state,ti86)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	m_bios = memregion("bios")->base();

	m_timer_interrupt_mask = 0;
	m_timer_interrupt_status = 0;
	m_ON_interrupt_mask = 0;
	m_ON_interrupt_status = 0;
	m_ON_pressed = 0;
	m_ti8x_memory_page_1 = 0;
	m_ti8x_memory_page_2 = 0;
	m_LCD_memory_base = 0;
	m_LCD_status = 0;
	m_LCD_mask = 0;
	m_power_mode = 0;
	m_keypad_mask = 0;
	m_video_buffer_width = 0;
	m_interrupt_speed = 0;
	m_port4_bit0 = 0;

	m_ti8x_ram = std::make_unique<uint8_t[]>(128*1024);
	memset(m_ti8x_ram.get(), 0, sizeof(uint8_t)*128*1024);

	space.unmap_write(0x0000, 0x3fff);

	membank("bank1")->set_base(m_bios);
	membank("bank2")->set_base(m_bios + 0x04000);

	membank("bank4")->set_base(m_ti8x_ram.get());
	subdevice<nvram_device>("nvram")->set_base(m_ti8x_ram.get(), sizeof(uint8_t)*128*1024);

	m_ti85_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(ti85_state::ti85_timer_callback), this));
	m_ti85_timer->adjust(attotime::from_hz(256), 0, attotime::from_hz(256));
}


/* I/O ports handlers */

uint8_t ti85_state::ti85_port_0000_r()
{
	return 0xff;
}

uint8_t ti85_state::ti8x_keypad_r()
{
	int data = 0xff;
	int port;
	int bit;
	static const char *const bitnames[] = { "BIT0", "BIT1", "BIT2", "BIT3", "BIT4", "BIT5", "BIT6", "BIT7" };

	if (m_keypad_mask == 0x7f) return data;

	for (bit = 0; bit < 7; bit++)
	{
		if (~m_keypad_mask&(0x01<<bit))
		{
			for (port = 0; port < 8; port++)
			{
				data ^= ioport(bitnames[port])->read() & (0x01<<bit) ? 0x01<<port : 0x00;
			}
		}
	}
	return data;
}

	uint8_t ti85_state::ti85_port_0002_r()
{
	return 0xff;
}

	uint8_t ti85_state::ti85_port_0003_r()
{
	int data = 0;

	if (m_LCD_status)
		data |= m_LCD_mask;
	if (m_ON_interrupt_status)
		data |= 0x01;
	if (m_timer_interrupt_status)
		data |= 0x04;
	if (!m_ON_pressed)
		data |= 0x08;
	m_ON_interrupt_status = 0;
	m_timer_interrupt_status = 0;
	return data;
}

	uint8_t ti85_state::ti85_port_0004_r()
{
	return 0xff;
}

	uint8_t ti85_state::ti85_port_0005_r()
{
	return m_ti8x_memory_page_1;
}

uint8_t ti85_state::ti85_port_0006_r()
{
	return m_power_mode;
}

uint8_t ti85_state::ti8x_serial_r()
{
	// 7: unknown (ROM always sets to 1)
	// 6: unknown (ROM always sets to 1)
	// 5: enable ring output
	// 4: enable tip output
	// 3: ring output
	// 2: tip output
	// 1: ring input
	// 0: tip input
	// If the calculator is driving a line low it will always read low

	uint8_t const tip_in((!m_link_port || m_link_port->tip_r()) ? 0x03 : 0x02);
	uint8_t const ring_in((!m_link_port || m_link_port->ring_r()) ? 0x03 : 0x01);
	return (~((m_PCR >> 2) & (m_PCR >> 4)) & tip_in & ring_in) | (m_PCR & 0xfc);
}

uint8_t ti85_state::ti82_port_0002_r()
{
	return m_ti8x_port2;
}

uint8_t ti85_state::ti86_port_0005_r()
{
	return m_ti8x_memory_page_1;
}

uint8_t ti85_state::ti86_port_0006_r()
{
	return m_ti8x_memory_page_2;
}

uint8_t ti85_state::ti83pse_port_0005_r()
{
	return m_ti8x_memory_page_3;
}

uint8_t ti85_state::ti83_port_0000_r()
{
	return ((m_ti8x_memory_page_1 & 0x08) << 1) | 0x0C;
}

uint8_t ti85_state::ti83_port_0002_r()
{
	return m_ti8x_port2;
}

uint8_t ti85_state::ti83_port_0003_r()
{
	int data = 0;

	if (m_ON_interrupt_status)
		data |= 0x01;
	if (!m_ON_pressed)
		data |= 0x08;

	data |= m_timer_interrupt_status;

	return data;
}

uint8_t ti85_state::ti8x_plus_serial_r()
{
	// 7: unknown
	// 6: byte receive in progress (TI-83+ only)
	// 5: ring output
	// 4: tip output
	// 3: received byte ready (cleared by reading port 0x05, TI-83+ only)
	// 2: enable byte receive (TI-83+ only)
	// 1: ring input
	// 0: tip input
	// Note that tip/ring outputs are inverted by an NPN transistor

	uint8_t const tip_in((!m_link_port || m_link_port->tip_r()) ? 0x03 : 0x02);
	uint8_t const ring_in((!m_link_port || m_link_port->ring_r()) ? 0x03 : 0x01);
	return (~(m_PCR >> 4) & tip_in & ring_in) | (m_PCR & 0xfc);
}

uint8_t ti85_state::ti83p_port_0002_r()
{
	return m_ti8x_port2|3;
}

uint8_t ti85_state::ti83p_port_0004_r()
{
	int data = 0;

	//data |= m_LCD_mask;

	if (m_ON_interrupt_status)
		data |= 0x01;
	if (!m_ON_pressed)
		data |= 0x08;

	data |= m_timer_interrupt_status;

	data |= m_ctimer_interrupt_status;

	return data;
}

//------------------------
// bit 0 - battery test (not implemented)
// Bit 1 - LCD wait
// bit 2 - flash lock
// bit 3 - not used
// bit 4 - not used
// bit 5 - Set if USB hardware is present
// bit 6 - Indicates if Link Assist is available
// bit 7 - SE or Basic

uint8_t ti85_state::ti83pse_port_0002_r()
{
	return 0xC3 | (m_flash_unlocked << 2);
}

uint8_t ti85_state::ti83pse_port_0009_r()
{
	return 0;
}

uint8_t ti85_state::ti83pse_port_0015_r()
{
	return 0x33;
}


void ti85_state::ti81_port_0007_w(uint8_t data)
{
	m_ti81_port_7_data = data;
}

void ti85_state::ti85_port_0000_w(uint8_t data)
{
	m_LCD_memory_base = data;
}

void ti85_state::ti8x_keypad_w(uint8_t data)
{
	m_keypad_mask = data&0x7f;
}

void ti85_state::ti85_port_0002_w(uint8_t data)
{
	m_LCD_contrast = data&0x1f;
}

void ti85_state::ti85_port_0003_w(uint8_t data)
{
	if (m_LCD_status && !(data&0x08))   m_timer_interrupt_mask = 0;
	m_ON_interrupt_mask = data&0x01;
//  m_timer_interrupt_mask = data&0x04;
	m_LCD_mask = data&0x02;
	m_LCD_status = data&0x08;
}

void ti85_state::ti85_port_0004_w(uint8_t data)
{
	m_video_buffer_width = (data>>3)&0x03;
	m_interrupt_speed = (data>>1)&0x03;
	m_port4_bit0 = data&0x01;
}

void ti85_state::ti85_port_0005_w(uint8_t data)
{
	m_ti8x_memory_page_1 = data;
	update_ti85_memory();
}

void ti85_state::ti85_port_0006_w(uint8_t data)
{
	m_power_mode = data;
}

void ti85_state::ti8x_serial_w(uint8_t data)
{
	// 7: unknown (ROM always sets to 1)
	// 6: unknown (ROM always sets to 1)
	// 5: enable ring output
	// 4: enable tip output
	// 3: ring output
	// 2: tip output
	// 1: unused
	// 0: unused
	// Note that tip/ring outputs are inverted by an NPN transistor
	// In practice, the ROM only uses values 0xc0, 0xd4 and 0xe8

	m_PCR = data & 0xfc;
	if (m_link_port)
	{
		m_link_port->tip_w(BIT(~data, 2) | BIT(~data, 4));
		m_link_port->ring_w(BIT(~data, 3) | BIT(~data, 5));
	}
}

void ti85_state::ti86_port_0005_w(uint8_t data)
{
	m_ti8x_memory_page_1 = data&((data&0x40)?0x47:0x4f);
	update_ti86_memory();
}

void ti85_state::ti86_port_0006_w(uint8_t data)
{
	m_ti8x_memory_page_2 = data&((data&0x40)?0x47:0x4f);
	update_ti86_memory();
}

void ti85_state::ti82_port_0002_w(uint8_t data)
{
	m_ti8x_memory_page_1 = (data & 0x07);
	update_ti85_memory();
	m_ti8x_port2 = data;
}

void ti85_state::ti83_port_0000_w(uint8_t data)
{
	m_ti8x_memory_page_1 = (m_ti8x_memory_page_1 & 7) | ((data & 16) >> 1);
	update_ti85_memory();
}

void ti85_state::ti83_port_0002_w(uint8_t data)
{
	m_ti8x_memory_page_1 = (m_ti8x_memory_page_1 & 8) | (data & 7);
	update_ti85_memory();
	m_ti8x_port2 = data;
}

void ti85_state::ti83_port_0003_w(uint8_t data)
{
	if (m_LCD_status && !(data&0x08))   m_timer_interrupt_mask = 0;
	m_ON_interrupt_mask = data&0x01;
	//m_timer_interrupt_mask = data&0x04;
	m_LCD_mask = data&0x02;
	m_LCD_status = data&0x08;
}

void ti85_state::ti8x_plus_serial_w(uint8_t data)
{
	// 7: unknown
	// 6: unknown
	// 5: unknown
	// 4: unknown
	// 3: unknown
	// 2: enable byte receive (TI-83+ only)
	// 1: ring output
	// 0: tip output
	// Note that tip/ring outputs are inverted by an NPN transistor

	m_PCR = (m_PCR & 0xc8) | (data & 0x04) | ((data << 4) & 0x30);
	if (m_link_port)
	{
		m_link_port->tip_w(BIT(~data, 2) | BIT(~data, 4));
		m_link_port->ring_w(BIT(~data, 3) | BIT(~data, 5));
	}
}

void ti85_state::ti83pse_int_ack_w(uint8_t data)
{
	//Lets ignore this for now, I think it'll be fine.
	m_ON_interrupt_status = data & 1;
	m_timer_interrupt_status = data & 0x06;
}

void ti85_state::ti83p_int_mask_w(uint8_t data)
{
	//m_LCD_mask = (data&0x08) >> 2;
	m_ON_interrupt_mask = data & 0x01;
	m_ON_interrupt_status &= m_ON_interrupt_mask;

	m_timer_interrupt_mask = data & 0x06;

	m_timer_interrupt_status &= m_timer_interrupt_mask;
}

void ti85_state::ti83p_port_0004_w(uint8_t data)
{
	m_ti83p_port4 = data | 0xe0;
	update_ti83p_memory();
}

void ti85_state::ti83p_port_0006_w(uint8_t data)
{
	m_ti8x_memory_page_1 = data & ((data&0x40) ? 0x41 : 0x5f);
	update_ti83p_memory();
}

void ti85_state::ti83p_port_0007_w(uint8_t data)
{
	m_ti8x_memory_page_2 = data & ((data&0x40) ? 0x41 : 0x5f);
	update_ti83p_memory();
}

void ti85_state::ti83pse_port_0004_w(uint8_t data)
{
	m_ti83p_port4 = data;
	update_ti83pse_memory();
}

void ti85_state::ti83pse_port_0005_w(uint8_t data)
{
	m_ti8x_memory_page_3 = data & 0x07;
	update_ti83pse_memory();
}

void ti85_state::ti83pse_port_0006_w(uint8_t data)
{
	if ((m_model == TI84P) && (data < 0x80))
	{
		m_ti8x_memory_page_1 = data & 0x3f;
	}
	else
	{
		m_ti8x_memory_page_1 = data;
	}
	update_ti83pse_memory();
}

void ti85_state::ti83pse_port_0007_w(uint8_t data)
{
	if ((m_model == TI84P) && (data < 0x80))
	{
		m_ti8x_memory_page_2 = data & 0x3f;
	}
	else
	{
		m_ti8x_memory_page_2 = data;
	}
	update_ti83pse_memory();
}

void ti85_state::ti83p_port_0014_w(uint8_t data)
{
	m_flash_unlocked = data;
	update_ti83pse_memory();
}

void ti85_state::ti83pse_port_0020_w(uint8_t data)
{
	m_cpu_speed = data;
	if(data)
	{
		m_maincpu->set_unscaled_clock(15000000);
	}
	else
	{
		m_maincpu->set_unscaled_clock(6000000);
	}
}

uint8_t ti85_state::ti83pse_port_0020_r()
{
	return m_cpu_speed;
}

void ti85_state::ti83pse_port_0021_w(uint8_t data)
{
	m_ti83pse_port21 = data & 0x0f;
}

uint8_t ti85_state::ti83pse_port_0021_r()
{
	return m_ti83pse_port21;
}

uint8_t ti85_state::ti84pse_port_0055_r()
{
	return 0x1f;
}

uint8_t ti85_state::ti84pse_port_0056_r()
{
	return 0;
}

//timer ports

void ti85_state::ti83pse_count( uint8_t timer, uint8_t data)
{
	m_ctimer[timer].max = m_ctimer[timer].count = data;

	if (m_ctimer[timer].setup)
	{
		switch (m_ctimer[timer].setup & 0x07)
		{
		case 0x00:
			m_ctimer[timer].divsor = 3.0;
			break;
		case 0x01:
			m_ctimer[timer].divsor = 32.0;
			break;
		case 0x02:
			m_ctimer[timer].divsor = 327.000;
			break;
		case 0x03:
			m_ctimer[timer].divsor = 3276.00;
			break;
		case 0x04:
			m_ctimer[timer].divsor = 1.0;
			break;
		case 0x05:
			m_ctimer[timer].divsor = 16.0;
			break;
		case 0x06:
			m_ctimer[timer].divsor = 256.0;
			break;
		case 0x07:
			m_ctimer[timer].divsor = 4096.0;
			break;
		}
		switch (timer)
		{
		case CRYSTAL_TIMER1:
			m_crystal_timer1->adjust(attotime::zero, 0, attotime::from_hz( 32768.0f/m_ctimer[timer].divsor));
			m_crystal_timer1->enable(true);
			break;
		case CRYSTAL_TIMER2:
			m_crystal_timer2->adjust(attotime::zero, 0, attotime::from_hz( 32768.0f/m_ctimer[timer].divsor));
			m_crystal_timer2->enable(true);
			break;
		case CRYSTAL_TIMER3:
			m_crystal_timer3->adjust(attotime::zero, 0, attotime::from_hz( 32768.0f/m_ctimer[timer].divsor));
			m_crystal_timer3->enable(true);
			break;

		}
	}
}


uint8_t ti85_state::ti83pse_ctimer1_setup_r()
{
	return m_ctimer[CRYSTAL_TIMER1].setup;
}

void ti85_state::ti83pse_ctimer1_setup_w(uint8_t data)
{
	m_crystal_timer1->enable(false);
	m_ctimer[CRYSTAL_TIMER1].setup = data;
}

uint8_t ti85_state::ti83pse_ctimer1_loop_r()
{
	return m_ctimer[CRYSTAL_TIMER1].loop;
}

void ti85_state::ti83pse_ctimer1_loop_w(uint8_t data)
{
	m_ctimer[CRYSTAL_TIMER1].loop = data & 0x03;
	m_ctimer_interrupt_status = 0;
}

uint8_t ti85_state::ti83pse_ctimer1_count_r()
{
	return m_ctimer[CRYSTAL_TIMER1].count;
}

void ti85_state::ti83pse_ctimer1_count_w(uint8_t data)
{
	ti83pse_count(CRYSTAL_TIMER1, data);

}

//

uint8_t ti85_state::ti83pse_ctimer2_setup_r()
{
	return m_ctimer[CRYSTAL_TIMER2].setup;
}

void ti85_state::ti83pse_ctimer2_setup_w(uint8_t data)
{
	m_crystal_timer2->enable(false);
	m_ctimer[CRYSTAL_TIMER2].setup = data;
}

uint8_t ti85_state::ti83pse_ctimer2_loop_r()
{
	return m_ctimer[CRYSTAL_TIMER2].loop;
}

void ti85_state::ti83pse_ctimer2_loop_w(uint8_t data)
{
	m_ctimer[CRYSTAL_TIMER2].loop = data & 0x03;
	m_ctimer_interrupt_status = 0;
}

uint8_t ti85_state::ti83pse_ctimer2_count_r()
{
	return m_ctimer[CRYSTAL_TIMER2].count;
}

void ti85_state::ti83pse_ctimer2_count_w(uint8_t data)
{
	ti83pse_count(CRYSTAL_TIMER2, data);

}

//

uint8_t ti85_state::ti83pse_ctimer3_setup_r()
{
	return m_ctimer[CRYSTAL_TIMER3].setup;
}

void ti85_state::ti83pse_ctimer3_setup_w(uint8_t data)
{
	m_crystal_timer3->enable(false);
	m_ctimer[CRYSTAL_TIMER3].setup = data;
}

uint8_t ti85_state::ti83pse_ctimer3_loop_r()
{
	return m_ctimer[CRYSTAL_TIMER3].loop;
}

void ti85_state::ti83pse_ctimer3_loop_w(uint8_t data)
{
	m_ctimer[CRYSTAL_TIMER3].loop = data & 0x03;
	m_ctimer_interrupt_status = 0;
}

uint8_t ti85_state::ti83pse_ctimer3_count_r()
{
	return m_ctimer[CRYSTAL_TIMER3].count;
}

void ti85_state::ti83pse_ctimer3_count_w(uint8_t data)
{
	ti83pse_count(CRYSTAL_TIMER3, data);

}



/***************************************************************************
  TI calculators snapshot files (SAV)
***************************************************************************/

void ti85_state::ti8x_snapshot_setup_registers (uint8_t * data)
{
	unsigned char lo,hi;
	unsigned char * reg = data + 0x40;

	/* Set registers */
	lo = reg[0x00] & 0x0ff;
	hi = reg[0x01] & 0x0ff;
	m_maincpu->set_state_int(Z80_AF, (hi << 8) | lo);
	lo = reg[0x04] & 0x0ff;
	hi = reg[0x05] & 0x0ff;
	m_maincpu->set_state_int(Z80_BC, (hi << 8) | lo);
	lo = reg[0x08] & 0x0ff;
	hi = reg[0x09] & 0x0ff;
	m_maincpu->set_state_int(Z80_DE, (hi << 8) | lo);
	lo = reg[0x0c] & 0x0ff;
	hi = reg[0x0d] & 0x0ff;
	m_maincpu->set_state_int(Z80_HL, (hi << 8) | lo);
	lo = reg[0x10] & 0x0ff;
	hi = reg[0x11] & 0x0ff;
	m_maincpu->set_state_int(Z80_IX, (hi << 8) | lo);
	lo = reg[0x14] & 0x0ff;
	hi = reg[0x15] & 0x0ff;
	m_maincpu->set_state_int(Z80_IY, (hi << 8) | lo);
	lo = reg[0x18] & 0x0ff;
	hi = reg[0x19] & 0x0ff;
	m_maincpu->set_state_int(Z80_PC, (hi << 8) | lo);
	lo = reg[0x1c] & 0x0ff;
	hi = reg[0x1d] & 0x0ff;
	m_maincpu->set_state_int(Z80_SP, (hi << 8) | lo);
	lo = reg[0x20] & 0x0ff;
	hi = reg[0x21] & 0x0ff;
	m_maincpu->set_state_int(Z80_AF2, (hi << 8) | lo);
	lo = reg[0x24] & 0x0ff;
	hi = reg[0x25] & 0x0ff;
	m_maincpu->set_state_int(Z80_BC2, (hi << 8) | lo);
	lo = reg[0x28] & 0x0ff;
	hi = reg[0x29] & 0x0ff;
	m_maincpu->set_state_int(Z80_DE2, (hi << 8) | lo);
	lo = reg[0x2c] & 0x0ff;
	hi = reg[0x2d] & 0x0ff;
	m_maincpu->set_state_int(Z80_HL2, (hi << 8) | lo);
	m_maincpu->set_state_int(Z80_IFF1, reg[0x30]&0x0ff);
	m_maincpu->set_state_int(Z80_IFF2, reg[0x34]&0x0ff);
	m_maincpu->set_state_int(Z80_HALT, reg[0x38]&0x0ff);
	m_maincpu->set_state_int(Z80_IM, reg[0x3c]&0x0ff);
	m_maincpu->set_state_int(Z80_I, reg[0x40]&0x0ff);

	m_maincpu->set_state_int(Z80_R, (reg[0x44]&0x7f) | (reg[0x48]&0x80));

	m_maincpu->set_input_line(0, 0);
	m_maincpu->set_input_line(INPUT_LINE_NMI, 0);
	m_maincpu->set_input_line(INPUT_LINE_HALT, 0);
}

void ti85_state::ti85_setup_snapshot (uint8_t * data)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	int i;
	unsigned char lo,hi;
	unsigned char * hdw = data + 0x8000 + 0x94;

	ti8x_snapshot_setup_registers (data);

	/* Memory dump */
	for (i = 0; i < 0x8000; i++)
		space.write_byte(i + 0x8000, data[i+0x94]);

	m_keypad_mask = hdw[0x00]&0x7f;

	m_ti8x_memory_page_1 = hdw[0x08]&0xff;
	update_ti85_memory();

	m_power_mode = hdw[0x14]&0xff;

	lo = hdw[0x28] & 0x0ff;
	hi = hdw[0x29] & 0x0ff;
	m_LCD_memory_base = (((hi << 8) | lo)-0xc000)>>8;

	m_LCD_status = hdw[0x2c]&0xff ? 0 : 0x08;
	if (m_LCD_status) m_LCD_mask = 0x02;

	m_LCD_contrast = hdw[0x30]&0xff;

	m_timer_interrupt_mask = !hdw[0x38];
	m_timer_interrupt_status = 0;

	m_ON_interrupt_mask = !m_LCD_status && m_timer_interrupt_status;
	m_ON_interrupt_status = 0;
	m_ON_pressed = 0;

	m_video_buffer_width = 0x02;
	m_interrupt_speed = 0x03;
}

void ti85_state::ti86_setup_snapshot (uint8_t * data)
{
	unsigned char lo,hi;
	unsigned char * hdw = data + 0x20000 + 0x94;

	ti8x_snapshot_setup_registers ( data);

	/* Memory dump */
	memcpy(m_ti8x_ram.get(), data+0x94, 0x20000);

	m_keypad_mask = hdw[0x00]&0x7f;

	m_ti8x_memory_page_1 = hdw[0x04]&0xff ? 0x40 : 0x00;
	m_ti8x_memory_page_1 |= hdw[0x08]&0x0f;

	m_ti8x_memory_page_2 = hdw[0x0c]&0xff ? 0x40 : 0x00;
	m_ti8x_memory_page_2 |= hdw[0x10]&0x0f;

	update_ti86_memory();

	lo = hdw[0x2c] & 0x0ff;
	hi = hdw[0x2d] & 0x0ff;
	m_LCD_memory_base = (((hi << 8) | lo)-0xc000)>>8;

	m_LCD_status = hdw[0x30]&0xff ? 0 : 0x08;
	if (m_LCD_status) m_LCD_mask = 0x02;

	m_LCD_contrast = hdw[0x34]&0xff;

	m_timer_interrupt_mask = !hdw[0x3c];
	m_timer_interrupt_status = 0;

	m_ON_interrupt_mask = !m_LCD_status && m_timer_interrupt_status;
	m_ON_interrupt_status = 0;
	m_ON_pressed = 0;

	m_video_buffer_width = 0x02;
	m_interrupt_speed = 0x03;
}

SNAPSHOT_LOAD_MEMBER(ti85_state::snapshot_cb)
{
	int expected_snapshot_size = 0;
	std::vector<uint8_t> ti8x_snapshot_data;

	if (!strncmp(machine().system().name, "ti85", 4))
		expected_snapshot_size = TI85_SNAPSHOT_SIZE;
	else if (!strncmp(machine().system().name, "ti86", 4))
		expected_snapshot_size = TI86_SNAPSHOT_SIZE;

	logerror("Snapshot loading\n");

	if (snapshot_size != expected_snapshot_size)
	{
		logerror ("Incomplete snapshot file\n");
		return image_init_result::FAIL;
	}

	ti8x_snapshot_data.resize(snapshot_size);

	image.fread( &ti8x_snapshot_data[0], snapshot_size);

	if (!strncmp(machine().system().name, "ti85", 4))
		ti85_setup_snapshot(&ti8x_snapshot_data[0]);
	else if (!strncmp(machine().system().name, "ti86", 4))
		ti86_setup_snapshot(&ti8x_snapshot_data[0]);

	return image_init_result::PASS;
}
