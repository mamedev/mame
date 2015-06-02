// license:BSD-3-Clause
// copyright-holders:Wilbert Pol,Gabriele D'Antona

/*

RM 380Z machine

*/


#include "includes/rm380z.h"


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

WRITE8_MEMBER( rm380z_state::port_write )
{
	switch ( offset )
	{
	case 0xFC:      // PORT0
		//printf("FBFCw[%2.2x] FBFD [%2.2x] FBFE [%2.2x] PC [%4.4x] writenum [%4.4x]\n",data,m_fbfd,m_fbfe,m_maincpu->safe_pc(),writenum);
		m_port0 = data;
		if (data&0x01)
		{
			//printf("WARNING: bit0 of port0 reset\n");
			m_port0_kbd=0;
		}
		m_port1&=~0x01; //?

		config_videomode();
		config_memory_map();
		break;

	case 0xFD:      // screen line counter (?)
		//printf("FBFC [%2.2x] FBFDw[%2.2x] FBFE [%2.2x] PC [%4.4x] writenum [%4.4x]\n",m_port0,data,m_fbfe,m_maincpu->safe_pc(),writenum);

		m_old_old_fbfd=m_old_fbfd;
		m_old_fbfd=m_fbfd;
		m_fbfd=data;

		writenum++;

		check_scroll_register();

		break;

	case 0xFE:      // line on screen to write to divided by 2
		//printf("FBFC [%2.2x] FBFD [%2.2x] FBFEw[%2.2x] PC [%4.4x] writenum [%4.4x]\n",m_port0,m_fbfd,data,m_maincpu->safe_pc(),writenum);
		m_fbfe=data;
		break;

	case 0xFF:      // user I/O port
		//printf("write of [%x] to FBFF\n",data);
		//logerror("%s: Write %02X to user I/O port\n", machine().describe_context(), data );
		break;

	default:
		printf("unknown port [%2.2x] write of [%2.2x]\n",offset,data);
	}
}

READ8_MEMBER( rm380z_state::port_read )
{
	UINT8 data = 0xFF;

	switch ( offset )
	{
	case 0xFC:      // PORT0
		//m_port0_kbd=getKeyboard();
		data = m_port0_kbd;
		//if (m_port0_kbd!=0) m_port0_kbd=0;
		//m_port0_kbd=0;
		//printf("read of port0 (kbd) from PC [%x]\n",m_maincpu->safe_pc());
		break;

	case 0xFD:      // "counter" (?)
		//printf("%s: Read from counter FBFD\n", machine().describe_context());
		data = 0x00;
		break;

	case 0xFE:      // PORT1
		data = m_port1;
		//printf("read of port1 from PC [%x]\n",m_maincpu->safe_pc());
		break;

	case 0xFF:      // user port
		//printf("%s: Read from user port\n", machine().describe_context());
		break;

	default:
		printf("read from unknown port [%2.2x]\n",offset);
	}

	return data;
}

WRITE8_MEMBER( rm380z_state::port_write_1b00 )
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	port_write(program,offset+0xfc,data);
}

READ8_MEMBER( rm380z_state::port_read_1b00 )
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	return port_read(program,offset+0xfc);
}

READ8_MEMBER( rm380z_state::hiram_read )
{
	return hiram[offset];
}

WRITE8_MEMBER( rm380z_state::hiram_write )
{
	hiram[offset]=data;
}

READ8_MEMBER( rm380z_state::rm380z_portlow_r )
{
	return 0xff;
}

WRITE8_MEMBER( rm380z_state::rm380z_portlow_w )
{
	//printf("port write [%x] [%x] at PC [%x]\n",offset,data,m_maincpu->safe_pc());
}

READ8_MEMBER( rm380z_state::rm380z_porthi_r )
{
	return 0xff;
}

WRITE8_MEMBER( rm380z_state::rm380z_porthi_w )
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
	m_rasterlineCtr%=(HORZ_LINES*LINE_SUBDIVISION);

	// frame blanking
	if (m_rasterlineCtr>=((HORZ_LINES-22)*LINE_SUBDIVISION))
	{
		m_port1|=0x40;
	}
	else
	{
		m_port1&=~0x40;
	}

	// line blanking
	if ((m_rasterlineCtr%LINE_SUBDIVISION)>80)
	{
		m_port1|=0x80;
	}
	else
	{
		m_port1&=~0x80;
	}
}

WRITE8_MEMBER( rm380z_state::keyboard_put )
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

WRITE8_MEMBER( rm380z_state::disk_0_control )
{
	floppy_image_device *floppy = NULL;

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
	machine().scheduler().timer_pulse(attotime::from_hz(TIMER_SPEED), timer_expired_delegate(FUNC(rm380z_state::static_vblank_timer),this));
}

void rm380z_state::machine_reset()
{
	m_port0=0x00;
	m_port0_kbd=0x00;
	m_port1=0x00;
	m_fbfd=0x00;
	m_fbfe=0x00;
	m_old_fbfd=0x00;
	m_old_old_fbfd=0x00;
	writenum=0;

	m_videomode=RM380Z_VIDEOMODE_80COL;
	m_old_videomode=m_videomode;
	m_rasterlineCtr=0;

	// note: from COS 4.0 videos, screen seems to show garbage at the beginning
	memset(m_mainVideoram,0,RM380Z_VIDEORAM_SIZE);

	memset(m_vramattribs,0,RM380Z_SCREENSIZE);
	memset(m_vramchars,0,RM380Z_SCREENSIZE);
	memset(m_vram,0,RM380Z_SCREENSIZE);

	config_memory_map();
	machine().device("wd1771")->reset();

	init_graphic_chars();
}

void rm380z_state::config_memory_map()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	UINT8 *rom = memregion(RM380Z_MAINCPU_TAG)->base();
	UINT8* m_ram_p = m_messram->pointer();

	if ( RM380Z_PORTS_ENABLED_HIGH )
	{
		program.install_ram( 0x0000, 0xDFFF, m_ram_p );
	}
	else
	{
		program.install_rom( 0x0000, 0x0FFF, rom );
		program.install_readwrite_handler(0x1BFC, 0x1BFF,read8_delegate(FUNC(rm380z_state::port_read_1b00), this),write8_delegate(FUNC(rm380z_state::port_write_1b00), this)    );
		program.install_rom( 0x1C00, 0x1DFF, rom + 0x1400 );
		program.install_ram( 0x4000, 0xDFFF, m_ram_p );
	}
}
