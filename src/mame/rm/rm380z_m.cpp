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
		//printf("%s FBFCw[%2.2x] FBFD [%2.2x] FBFE [%2.2x] writenum [%4.4x]\n", machine().describe_context().c_str(), data, m_fbfd, m_fbfe,writenum);
		m_port0 = data;

		m_cassette->output((m_port0 & 0xef) ? +1.0 : -1.0); // set 2400hz, bit 4

		if (data & 0x01)
		{
			//printf("WARNING: bit0 of port0 reset\n");
			m_port0_kbd = 0;
		}
		m_port1 &= ~0x01; //?

		config_videomode();
		config_memory_map();
		break;

	case 0xfd:      // screen line counter (?)
		//printf("%s FBFC [%2.2x] FBFDw[%2.2x] FBFE [%2.2x] writenum [%4.4x]\n",machine().describe_context().c_str(),m_port0,data,m_fbfe,writenum);

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
			m_vram.set_scroll_register(data & m_fbfd_mask);
		}

		break;

	// port 1
	case 0xfe:      // line on screen to write to divided by 2
		//printf("%s FBFC [%2.2x] FBFD [%2.2x] FBFEw[%2.2x] writenum [%4.4x]\n",machine().describe_context().c_str(),m_port0,m_fbfd,data,writenum);

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

	case 0xff:      // user I/O port
		//printf("write of [%x] to FBFF\n",data);
		//logerror("%s: Write %02X to user I/O port\n", machine().describe_context(), data);
		break;

	default:
		printf("unknown port [%2.2x] write of [%2.2x]\n", offset, data);
	}
}

uint8_t rm380z_state::port_read(offs_t offset)
{
	uint8_t data = 0xff;

	switch (offset)
	{
	case 0xfc:      // PORT0
		//m_port0_kbd=getKeyboard();
		data = m_port0_kbd;
		//if (m_port0_kbd!=0) m_port0_kbd = 0;
		//m_port0_kbd=0;
		//printf("%s read of port0 (kbd)\n",machine().describe_context().c_str());
		break;

	case 0xfd:      // "counter" (?)
		//printf("%s: Read from counter FBFD\n", machine().describe_context().c_str());
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

	case 0xfe:      // PORT1
		if (m_cassette->input() < +0.0)
			m_port1 &= 0xdf;    // bit 5 off
		else
			m_port1 |= 0x20;    // bit 5 on

		data = m_port1;
		//printf("%s read of port1\n", machine().describe_context().c_str());
		break;

	case 0xff:      // user port
		//printf("%s: Read from user port\n", machine().describe_context().c_str());
		break;

	default:
		printf("read from unknown port [%2.2x]\n", offset);
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
	if ((m_rasterlineCtr % LINE_SUBDIVISION) > 80)
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

void rm380z_state::init_rm380z()
{
	m_videomode = RM380Z_VIDEOMODE_80COL;
	m_port0_mask = 0xff;
	m_fbfd_mask = 0x1f;     // enable hw scrolling (uses lower 5 bits of counter)
}

void rm380z_state::init_rm380z34d()
{
	m_videomode = RM380Z_VIDEOMODE_40COL;
	m_port0_mask = 0xdf;      // disable 80 column mode
	m_screen->set_size(240, 240);
	m_screen->set_visarea_full();
}

void rm380z_state::init_rm380z34e()
{
	m_videomode = RM380Z_VIDEOMODE_40COL;
	m_port0_mask = 0xdf;      // disable 80 column mode
	m_screen->set_size(240, 240);
	m_screen->set_visarea_full();
}

void rm380z_state::init_rm480z()
{
	// machine not working so do nothing
}

void rm380z_state::machine_reset()
{
	m_port0 = 0x00;
	m_port0_kbd = 0x00;
	m_port1 = 0x00;
	m_fbfe = 0x00;

	m_rasterlineCtr = 0;

	// note: from COS 4.0 videos, screen seems to show garbage at the beginning
	m_vram.reset();

	config_memory_map();
	m_fdc->reset();

	init_graphic_chars();
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

MACHINE_RESET_MEMBER( rm380z_state, rm480z )
{
}
