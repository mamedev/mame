// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Gabriele D'Antona

/*

RM 380Z machine

*/

#include "emu.h"
#include "rm380z.h"


/*

PORT0 write in COS 4.0B/M:

bit0: 1=reset keyboard latch (?)
bit1: ?
bit2: ?
bit3: ?
bit4: ?
bit5: 40/80 cols switch (?)
bit6: 0=write to char RAM/1=write to attribute RAM
bit7: 1=map ROM at 0000-0fff/0=RAM

*/

void rm380z_state::port_write(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0xfc:      // PORT0
		if ((data & 0x01) && !(m_port0 & 0x01))
		{
			// only clear keyboard latch if bit has changed value
			m_port0_kbd = 0;
			m_port1 &= ~0x01;
		}

		m_port0 = data;

		config_memory_map();
		break;

	case 0xff:      // user I/O port
		break;

	default:
		printf("unknown port [%2.2x] write of [%2.2x]\n", offset, data);
	}
}

void rm380z_state_cos34::port_write(offs_t offset, uint8_t data)
{
	if (offset == 0xfc)
	{
		m_cassette->output((data & 0xef) ? +1.0 : -1.0); // set 2400hz, bit 4
	}

	rm380z_state::port_write(offset, data);
}

void rm380z_state_cos40::port_write(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0xfd:
		if (m_port0 & 0x08)
		{
			// update user defined character data
			if (m_character >= 128)
			{
				m_user_defined_chars[(m_character % 128) * 16 + m_character_row] = data;
			}
		}
		// ignore updates while bit 4 of port 0 is set
		// (counter is not used to set the scroll register in this case, maybe used for smooth scrolling?)
		else if (!(m_port0 & 0x10))
		{
			// set scroll register (used to verticaly scroll the screen and effect vram addressing)
			m_vram.set_scroll_register(data & 0x1f);
		}
		break;

	case 0xfe:
		if (!(m_port0 & 0x04))
		{
			m_character_row = data;
		}
		else if (m_port0 & 0x08)
		{
			m_character = data;
		}

		m_fbfe = data;
		break;

	case 0xfc:
		rm380z_state::port_write(offset, data);
		config_videomode();
		break;

	default:
		rm380z_state::port_write(offset, data);
		break;
	}
}

void rm380z_state_cos40_hrg::port_write(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x00:
		if ((m_hrg_port0 & 0x01) && !(data & 0x01))
		{
			// set low nibble of scratchpad (palette data) when bit 0 toggled
			change_hrg_scratchpad(m_hrg_port1 >> 4, m_hrg_port1 & 0x0f, 0xf0); 
		}
		else if ((m_hrg_port0 & 0x02) && !(data & 0x02))
		{
			// set high nibble of scratchpad (palette data) when bit 1 toggled
			change_hrg_scratchpad(m_hrg_port1 >> 4, m_hrg_port1 << 4, 0x0f);
		}

		switch (data)
		{
		case 0x03:
			m_hrg_display_mode = hrg_display_mode::high;
			break;
		case 0xa3:
			m_hrg_display_mode = hrg_display_mode::medium_0;
			break;
		case 0xc3:
			m_hrg_display_mode = hrg_display_mode::medium_1;
			break;
		}

		m_hrg_port0 = data;
		break;

	case 0x01:
		// video ram page number (for subsequent read/write) or scratchpad data
		m_hrg_port1 = data;
		break;

	default:
		rm380z_state_cos40::port_write(offset, data);
		break;
	}
}

uint8_t rm380z_state::port_read(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset)
	{
	case 0xfc:      // PORT0
		data = m_port0_kbd;
		break;

	case 0xfe:      // PORT1
		data = m_port1;
		break;

	case 0xff:      // user port
		break;

	default:
		printf("read from unknown port [%2.2x]\n", offset);
	}

	return data;
}

uint8_t rm380z_state_cos34::port_read(offs_t offset)
{
	if (offset == 0xfe)
	{
		if (m_cassette->input() < +0.0)
		{
			m_port1 &= 0xdf;    // bit 5 off
		}
		else
		{
			m_port1 |= 0x20;    // bit 5 on
		}
	}

	return rm380z_state::port_read(offset);
}

uint8_t rm380z_state_cos40::port_read(offs_t offset)
{
	uint8_t data;

	switch (offset)
	{
	case 0xfd:
		if (m_port0 & 0x08)
		{
			// return character data for requested character and row
			if (m_character >= 128)
			{
				data = m_user_defined_chars[(m_character % 128) * 16 + m_character_row];
			}
			else
			{
				data = m_chargen[m_character * 16 + m_character_row];
			}
		}
		else
		{
			data = 0x00;
		}
		break;

	default:
		data = rm380z_state::port_read(offset);
		break;
	}

	return data;
}

uint8_t rm380z_state_cos40_hrg::port_read(offs_t offset)
{
	uint8_t data;

	switch (offset)
	{
	case 0x00:
		// bit 0 is low during HRG frame blanking
		// bit 1 is low duing HRG line blanking
		// (this is the inverse of VDU port 1 shifted 6 bits to the right)
		data = (m_port1 >> 6) ^ 0x03;
		break;

	default:
		data = rm380z_state_cos40::port_read(offset);
		break;
	}

	return data;
}

void rm380z_state::port_write_1b00(offs_t offset, uint8_t data)
{
	port_write(offset + 0xfc, data);
}

uint8_t rm380z_state::port_read_1b00(offs_t offset)
{
	return port_read(offset + 0xfc);
}

uint8_t rm380z_state::rm380z_portlow_r()
{
	return 0xff;
}

void rm380z_state::rm380z_portlow_w(offs_t offset, uint8_t data)
{
	//printf("%s port write [%x] [%x]\n",machine().describe_context().c_str(),offset,data);
}

uint8_t rm380z_state::rm380z_porthi_r()
{
	return 0xff;
}

void rm380z_state::rm380z_porthi_w(offs_t offset, uint8_t data)
{
	//printf("port write [%x] [%x]\n",offset+0xc5,data);
}


#define LINE_SUBDIVISION 82
#define HORZ_LINES 100
#define TIMER_SPEED 50*HORZ_LINES*LINE_SUBDIVISION

//
// this simulates line+frame blanking
// according to the System manual, "frame blanking bit (bit 6) of port1 becomes high
// for about 4.5 milliseconds every 20 milliseconds"
//

TIMER_CALLBACK_MEMBER(rm380z_state::static_vblank_timer)
{
	//printf("timer callback called at [%f]\n",machine().time().as_double());


	m_rasterlineCtr++;
	m_rasterlineCtr %= HORZ_LINES * LINE_SUBDIVISION;

	// frame blanking
	if (m_rasterlineCtr >= ((HORZ_LINES - 22) * LINE_SUBDIVISION))
	{
		m_port1 |= 0x40;
	}
	else
	{
		m_port1 &= ~0x40;
	}

	// line blanking
	// according to wikipedia this occupies 18.8% of a scanline for PAL
	if ((m_rasterlineCtr % LINE_SUBDIVISION) > 67)
	{
		m_port1 |= 0x80;
	}
	else
	{
		m_port1 &= ~0x80;
	}
}

void rm380z_state::keyboard_put(u8 data)
{
	if (data)
	{
		m_port0_kbd = data;
		m_port1 |= 1;
	}
}

//
// ports c0-cf are related to the floppy disc controller
// c0-c3: wd1771
// c4-c8: disk drive port0
//
// CP/M booting:
// from the service manual: "the B command reads a sector from drive 0, track 0, sector 1 into memory
// at 0x0080 to 0x00FF, then jumps to it if there is no error."
//

void rm380z_state::disk_0_control(uint8_t data)
{
	floppy_image_device *floppy = nullptr;

	if (BIT(data, 0)) floppy = m_floppy0->get_device();
	if (BIT(data, 1)) floppy = m_floppy1->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		// don't know how motor on is connected
		floppy->mon_w(0);
		floppy->ss_w(BIT(data, 5));
	}
}

void rm380z_state::machine_start()
{
	m_static_vblank_timer = timer_alloc(FUNC(rm380z_state::static_vblank_timer), this);
	m_static_vblank_timer->adjust(attotime::from_hz(TIMER_SPEED), 0, attotime::from_hz(TIMER_SPEED));
}

void rm380z_state::machine_reset()
{
	m_port0 = 0x00;
	m_port0_kbd = 0x00;
	m_port1 = 0x00;
	m_fbfe = 0x00;

	m_rasterlineCtr = 0;

	config_memory_map();
	m_fdc->reset();
}

void rm380z_state_cos34::machine_reset()
{
	rm380z_state::machine_reset();

	m_vram.reset();
}

void rm380z_state_cos40::machine_reset()
{
	rm380z_state::machine_reset();

	m_vram.reset();
	memset(m_user_defined_chars, 0, sizeof(m_user_defined_chars));
}

void rm380z_state_cos40_hrg::machine_reset()
{
	rm380z_state_cos40::machine_reset();

	m_hrg_port0 = 0x00;
	m_hrg_port1 = 0x00;
	m_hrg_display_mode = hrg_display_mode::none;

	memset(m_hrg_ram, 0, sizeof(m_hrg_ram));
	memset(m_hrg_scratchpad, 0, sizeof(m_hrg_scratchpad));	
}

void rm380z_state::config_memory_map()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	uint8_t *rom = memregion(RM380Z_MAINCPU_TAG)->base();
	uint8_t* m_ram_p = m_messram->pointer();

	if (ports_enabled_high())
	{
		program.install_ram(0x0000, 0xdfff, m_ram_p);
	}
	else
	{
		program.install_rom(0x0000, 0x0fff, rom);
		program.install_readwrite_handler(0x1bfc, 0x1bff, read8sm_delegate(*this, FUNC(rm380z_state::port_read_1b00)), write8sm_delegate(*this, FUNC(rm380z_state::port_write_1b00)));
		program.install_rom(0x1c00, 0x1dff, rom + 0x1400);
		program.install_ram(0x4000, 0xdfff, m_ram_p);
	}
}

MACHINE_RESET_MEMBER( rm480z_state, rm480z )
{
}
