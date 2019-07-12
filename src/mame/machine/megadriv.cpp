// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Megadrive / Genesis support

this could probably do with a complete rewrite at this point to take into
account all new information discovered since this was created, it's looking
rather old now.



Cleanup / Rewrite notes:



Known Non-Issues (confirmed on Real Genesis)
    Castlevania - Bloodlines (U) [!] - Pause text is missing on upside down level
    Blood Shot (E) (M4) [!] - corrupt texture in level 1 is correct...



*/


#include "emu.h"
#include "includes/megadriv.h"
#include "speaker.h"


void md_base_state::megadriv_z80_bank_w(uint16_t data)
{
	m_genz80.z80_bank_addr = ((m_genz80.z80_bank_addr >> 1) | (data << 23)) & 0xff8000;
}

WRITE16_MEMBER(md_base_state::megadriv_68k_z80_bank_write )
{
	//logerror("%06x: 68k writing bit to bank register %01x\n", m_maincpu->pc(),data&0x01);
	megadriv_z80_bank_w(data & 0x01);
}

WRITE8_MEMBER(md_base_state::megadriv_z80_z80_bank_w)
{
	//logerror("%04x: z80 writing bit to bank register %01x\n", m_maincpu->pc(),data&0x01);
	megadriv_z80_bank_w(data & 0x01);
}

READ8_MEMBER(md_base_state::megadriv_68k_YM2612_read)
{
	//osd_printf_debug("megadriv_68k_YM2612_read %02x %04x\n",offset,mem_mask);
	if ((m_genz80.z80_has_bus == 0) && (m_genz80.z80_is_reset == 0))
	{
		return m_ymsnd->read(offset);
	}
	else
	{
		logerror("%s: 68000 attempting to access YM2612 (read) without bus\n", machine().describe_context());
		return 0;
	}

	// never executed
	//return -1;
}


WRITE8_MEMBER(md_base_state::megadriv_68k_YM2612_write)
{
	//osd_printf_debug("megadriv_68k_YM2612_write %02x %04x %04x\n",offset,data,mem_mask);
	if ((m_genz80.z80_has_bus == 0) && (m_genz80.z80_is_reset == 0))
	{
		m_ymsnd->write(offset, data);
	}
	else
	{
		logerror("%s: 68000 attempting to access YM2612 (write) without bus\n", machine().describe_context());
	}
}

// this is used by 6 button pads and gets installed in machine_start for drivers requiring it
TIMER_CALLBACK_MEMBER(md_base_state::io_timeout_timer_callback)
{
	m_io_stage[(int)(uintptr_t)ptr] = -1;
}


/*

    A10001h = A0         Version register

    A10003h = 7F         Data register for port A
    A10005h = 7F         Data register for port B
    A10007h = 7F         Data register for port C

    A10009h = 00         Ctrl register for port A
    A1000Bh = 00         Ctrl register for port B
    A1000Dh = 00         Ctrl register for port C

    A1000Fh = FF         TxData register for port A
    A10011h = 00         RxData register for port A
    A10013h = 00         S-Ctrl register for port A

    A10015h = FF         TxData register for port B
    A10017h = 00         RxData register for port B
    A10019h = 00         S-Ctrl register for port B

    A1001Bh = FF         TxData register for port C
    A1001Dh = 00         RxData register for port C
    A1001Fh = 00         S-Ctrl register for port C




 Bit 7 - (Not connected)
 Bit 6 - TH
 Bit 5 - TL
 Bit 4 - TR
 Bit 3 - RIGHT
 Bit 2 - LEFT
 Bit 1 - DOWN
 Bit 0 - UP


*/

INPUT_PORTS_START( md_common )
	PORT_START("PAD1")      /* Joypad 1 (3 button + start) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 B") // b
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 C") // c
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 A") // a
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) PORT_NAME("P1 START") // start

	PORT_START("PAD2")      /* Joypad 2 (3 button + start) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 B") // b
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 C") // c
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 A") // a
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) PORT_NAME("P2 START") // start
INPUT_PORTS_END


INPUT_PORTS_START( megadriv )
	PORT_INCLUDE( md_common )

	PORT_START("RESET")     /* Buttons on Genesis Console */
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Reset Button") PORT_IMPULSE(1) // reset, resets 68k (and..?)
INPUT_PORTS_END

INPUT_PORTS_START( megadri6 )
	PORT_INCLUDE( megadriv )

	PORT_START("EXTRA1")    /* Extra buttons for Joypad 1 (6 button + start + mode) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(1) PORT_NAME("P1 Z") // z
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("P1 Y") // y
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) PORT_NAME("P1 X") // x
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(1) PORT_NAME("P1 MODE") // mode

	PORT_START("EXTRA2")    /* Extra buttons for Joypad 2 (6 button + start + mode) NOT READ DIRECTLY */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_PLAYER(2) PORT_NAME("P2 Z") // z
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("P2 Y") // y
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) PORT_NAME("P2 X") // x
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_PLAYER(2) PORT_NAME("P2 MODE") // mode
INPUT_PORTS_END

void md_base_state::megadrive_reset_io()
{
	int i;

	m_megadrive_io_data_regs[0] = 0x7f;
	m_megadrive_io_data_regs[1] = 0x7f;
	m_megadrive_io_data_regs[2] = 0x7f;
	m_megadrive_io_ctrl_regs[0] = 0x00;
	m_megadrive_io_ctrl_regs[1] = 0x00;
	m_megadrive_io_ctrl_regs[2] = 0x00;
	m_megadrive_io_tx_regs[0] = 0xff;
	m_megadrive_io_tx_regs[1] = 0xff;
	m_megadrive_io_tx_regs[2] = 0xff;

	for (i=0; i<3; i++)
	{
		m_io_stage[i] = -1;
	}
}

/************* 6 buttons version **************************/
READ8_MEMBER(md_base_state::megadrive_io_read_data_port_6button)
{
	int portnum = offset;
	uint8_t retdata, helper = (m_megadrive_io_ctrl_regs[portnum] & 0x3f) | 0xc0; // bits 6 & 7 always come from m_megadrive_io_data_regs

	if (m_megadrive_io_data_regs[portnum] & 0x40)
	{
		if (m_io_stage[portnum] == 2)
		{
			/* here we read B, C & the additional buttons */
			retdata = (m_megadrive_io_data_regs[portnum] & helper) |
						((((m_io_pad_3b[portnum] ? m_io_pad_3b[portnum]->read() : 0) & 0x30) |
							((m_io_pad_6b[portnum] ? m_io_pad_6b[portnum]->read() : 0) & 0x0f)) & ~helper);
		}
		else
		{
			/* here we read B, C & the directional buttons */
			retdata = (m_megadrive_io_data_regs[portnum] & helper) |
						(((m_io_pad_3b[portnum] ? m_io_pad_3b[portnum]->read() : 0) & 0x3f) & ~helper);
		}
	}
	else
	{
		if (m_io_stage[portnum] == 1)
		{
			/* here we read ((Start & A) >> 2) | 0x00 */
			retdata = (m_megadrive_io_data_regs[portnum] & helper) |
						((((m_io_pad_3b[portnum] ? m_io_pad_3b[portnum]->read() : 0) & 0xc0) >> 2) & ~helper);
		}
		else if (m_io_stage[portnum]==2)
		{
			/* here we read ((Start & A) >> 2) | 0x0f */
			retdata = (m_megadrive_io_data_regs[portnum] & helper) |
						(((((m_io_pad_3b[portnum] ? m_io_pad_3b[portnum]->read() : 0) & 0xc0) >> 2) | 0x0f) & ~helper);
		}
		else
		{
			/* here we read ((Start & A) >> 2) | Up and Down */
			retdata = (m_megadrive_io_data_regs[portnum] & helper) |
						(((((m_io_pad_3b[portnum] ? m_io_pad_3b[portnum]->read() : 0) & 0xc0) >> 2) |
							((m_io_pad_3b[portnum] ? m_io_pad_3b[portnum]->read() : 0) & 0x03)) & ~helper);
		}
	}

//  osd_printf_debug("read io data port stage %d port %d %02x\n",m_io_stage[portnum],portnum,retdata);

	return retdata | (retdata << 8);
}


/************* 3 buttons version **************************/
READ8_MEMBER(md_base_state::megadrive_io_read_data_port_3button)
{
	int portnum = offset;
	uint8_t retdata, helper = (m_megadrive_io_ctrl_regs[portnum] & 0x7f) | 0x80; // bit 7 always comes from m_megadrive_io_data_regs

	if (m_megadrive_io_data_regs[portnum] & 0x40)
	{
		/* here we read B, C & the directional buttons */
		retdata = (m_megadrive_io_data_regs[portnum] & helper) |
					((((m_io_pad_3b[portnum] ? m_io_pad_3b[portnum]->read() : 0) & 0x3f) | 0x40) & ~helper);
	}
	else
	{
		/* here we read ((Start & A) >> 2) | Up and Down */
		retdata = (m_megadrive_io_data_regs[portnum] & helper) |
					(((((m_io_pad_3b[portnum] ? m_io_pad_3b[portnum]->read() : 0) & 0xc0) >> 2) |
						((m_io_pad_3b[portnum] ? m_io_pad_3b[portnum]->read() : 0) & 0x03) | 0x40) & ~helper);
	}

	return retdata;
}

uint8_t md_base_state::megadrive_io_read_ctrl_port(int portnum)
{
	uint8_t retdata;
	retdata = m_megadrive_io_ctrl_regs[portnum];
	//osd_printf_debug("read io ctrl port %d %02x\n",portnum,retdata);

	return retdata | (retdata << 8);
}

uint8_t md_base_state::megadrive_io_read_tx_port(int portnum)
{
	uint8_t retdata;
	retdata = m_megadrive_io_tx_regs[portnum];
	return retdata | (retdata << 8);
}

uint8_t md_base_state::megadrive_io_read_rx_port(int portnum)
{
	return 0x00;
}

uint8_t md_base_state::megadrive_io_read_sctrl_port(int portnum)
{
	return 0x00;
}


READ16_MEMBER(md_base_state::megadriv_68k_io_read )
{
	uint8_t retdata;

	retdata = 0;
		/* Charles MacDonald ( http://cgfm2.emuviews.com/ )
		  D7 : Console is 1= Export (USA, Europe, etc.) 0= Domestic (Japan)
		  D6 : Video type is 1= PAL, 0= NTSC
		  D5 : Sega CD unit is 1= not present, 0= connected.
		  D4 : Unused (always returns zero)
		  D3 : Bit 3 of version number
		  D2 : Bit 2 of version number
		  D1 : Bit 1 of version number
		  D0 : Bit 0 of version number
		*/

	//return (machine().rand()&0x0f0f)|0xf0f0;//0x0000;
	switch (offset)
	{
		case 0:
			logerror("%06x read version register\n", m_maincpu->pc());
			retdata = m_version_hi_nibble | 0x01; // Version number contained in bits 3-0
			break;

		/* Joystick Port Registers */

		case 0x1:
		case 0x2:
		case 0x3:
//          retdata = megadrive_io_read_data_port(offset-1);
			retdata = m_megadrive_io_read_data_port_ptr(space, offset-1, 0xff);
			break;

		case 0x4:
		case 0x5:
		case 0x6:
			retdata = megadrive_io_read_ctrl_port(offset-4);
			break;

		/* Serial I/O Registers */

		case 0x7: retdata = megadrive_io_read_tx_port(0); break;
		case 0x8: retdata = megadrive_io_read_rx_port(0); break;
		case 0x9: retdata = megadrive_io_read_sctrl_port(0); break;

		case 0xa: retdata = megadrive_io_read_tx_port(1); break;
		case 0xb: retdata = megadrive_io_read_rx_port(1); break;
		case 0xc: retdata = megadrive_io_read_sctrl_port(1); break;

		case 0xd: retdata = megadrive_io_read_tx_port(2); break;
		case 0xe: retdata = megadrive_io_read_rx_port(2); break;
		case 0xf: retdata = megadrive_io_read_sctrl_port(2); break;

	}

	return retdata | (retdata << 8);
}


WRITE16_MEMBER(md_base_state::megadrive_io_write_data_port_3button)
{
	int portnum = offset;
	m_megadrive_io_data_regs[portnum] = data;
	//osd_printf_debug("Writing IO Data Register #%d data %04x\n",portnum,data);

}


/****************************** 6 buttons version*****************************/

WRITE16_MEMBER(md_base_state::megadrive_io_write_data_port_6button)
{
	int portnum = offset;
	if (m_megadrive_io_ctrl_regs[portnum]&0x40)
	{
		if (((m_megadrive_io_data_regs[portnum]&0x40)==0x00) && ((data&0x40) == 0x40))
		{
			m_io_stage[portnum]++;
			m_io_timeout[portnum]->adjust(m_maincpu->cycles_to_attotime(8192));
		}

	}

	m_megadrive_io_data_regs[portnum] = data;
	//osd_printf_debug("Writing IO Data Register #%d data %04x\n",portnum,data);

}


/*************************** 3 buttons version ****************************/

void md_base_state::megadrive_io_write_ctrl_port(int portnum, uint16_t data)
{
	m_megadrive_io_ctrl_regs[portnum] = data;
//  osd_printf_debug("Setting IO Control Register #%d data %04x\n",portnum,data);
}

void md_base_state::megadrive_io_write_tx_port(int portnum, uint16_t data)
{
	m_megadrive_io_tx_regs[portnum] = data;
}

void md_base_state::megadrive_io_write_rx_port(int portnum, uint16_t data)
{
}

void md_base_state::megadrive_io_write_sctrl_port(int portnum, uint16_t data)
{
}


WRITE16_MEMBER(md_base_state::megadriv_68k_io_write )
{
//  osd_printf_debug("IO Write #%02x data %04x mem_mask %04x\n",offset,data,mem_mask);


	switch (offset)
	{
		case 0x0:
			osd_printf_debug("Write to Version Register?!\n");
			break;

		/* Joypad Port Registers */

		case 0x1:
		case 0x2:
		case 0x3:
//          megadrive_io_write_data_port(offset-1,data);
			m_megadrive_io_write_data_port_ptr(space, offset-1,data, 0xffff);
			break;

		case 0x4:
		case 0x5:
		case 0x6:
			megadrive_io_write_ctrl_port(offset-4,data);
			break;

		/* Serial I/O Registers */

		case 0x7: megadrive_io_write_tx_port(0,data); break;
		case 0x8: megadrive_io_write_rx_port(0,data); break;
		case 0x9: megadrive_io_write_sctrl_port(0,data); break;

		case 0xa: megadrive_io_write_tx_port(1,data); break;
		case 0xb: megadrive_io_write_rx_port(1,data); break;
		case 0xc: megadrive_io_write_sctrl_port(1,data); break;

		case 0xd: megadrive_io_write_tx_port(2,data); break;
		case 0xe: megadrive_io_write_rx_port(2,data); break;
		case 0xf: megadrive_io_write_sctrl_port(2,data); break;
	}
}



void md_base_state::megadriv_map(address_map &map)
{
	map(0x000000, 0x3fffff).rom();
	/*      (0x000000 - 0x3fffff) == GAME ROM (4Meg Max, Some games have special banking too) */

	map(0xa00000, 0xa01fff).rw(FUNC(md_base_state::megadriv_68k_read_z80_ram), FUNC(md_base_state::megadriv_68k_write_z80_ram));
	map(0xa02000, 0xa03fff).w(FUNC(md_base_state::megadriv_68k_write_z80_ram));
	map(0xa04000, 0xa04003).rw(FUNC(md_base_state::megadriv_68k_YM2612_read), FUNC(md_base_state::megadriv_68k_YM2612_write));

	map(0xa06000, 0xa06001).w(FUNC(md_base_state::megadriv_68k_z80_bank_write));

	map(0xa10000, 0xa1001f).rw(FUNC(md_base_state::megadriv_68k_io_read), FUNC(md_base_state::megadriv_68k_io_write));

	map(0xa11100, 0xa11101).rw(FUNC(md_base_state::megadriv_68k_check_z80_bus), FUNC(md_base_state::megadriv_68k_req_z80_bus));
	map(0xa11200, 0xa11201).w(FUNC(md_base_state::megadriv_68k_req_z80_reset));

	map(0xc00000, 0xc0001f).rw(m_vdp, FUNC(sega315_5313_device::vdp_r), FUNC(sega315_5313_device::vdp_w));
	map(0xd00000, 0xd0001f).rw(m_vdp, FUNC(sega315_5313_device::vdp_r), FUNC(sega315_5313_device::vdp_w)); // the earth defend
	map(0xe00000, 0xe0ffff).ram().mirror(0x1f0000).share("megadrive_ram");
//  AM_RANGE(0xff0000, 0xffffff) AM_READONLY
	/*       0xe00000 - 0xffffff) == MAIN RAM (64kb, Mirrored, most games use ff0000 - ffffff) */
}


void md_base_state::dcat16_megadriv_map(address_map &map)
{
	megadriv_map(map);
	map(0x000000, 0x7fffff).rom();
}


/* z80 sounds/sub CPU */


READ16_MEMBER(md_base_state::megadriv_68k_read_z80_ram )
{
	//osd_printf_debug("read z80 ram %04x\n",mem_mask);

	if ((m_genz80.z80_has_bus == 0) && (m_genz80.z80_is_reset == 0))
	{
		return m_genz80.z80_prgram[(offset<<1)^1] | (m_genz80.z80_prgram[(offset<<1)]<<8);
	}
	else
	{
		logerror("%06x: 68000 attempting to access Z80 (read) address space without bus\n", m_maincpu->pc());
		return machine().rand();
	}
}

WRITE16_MEMBER(md_base_state::megadriv_68k_write_z80_ram )
{
	//logerror("write z80 ram\n");

	if ((m_genz80.z80_has_bus == 0) && (m_genz80.z80_is_reset == 0))
	{
		if (!ACCESSING_BITS_0_7) // byte (MSB) access
		{
			m_genz80.z80_prgram[(offset<<1)] = (data & 0xff00) >> 8;
		}
		else if (!ACCESSING_BITS_8_15)
		{
			m_genz80.z80_prgram[(offset<<1)^1] = (data & 0x00ff);
		}
		else // for WORD access only the MSB is used, LSB is ignored
		{
			m_genz80.z80_prgram[(offset<<1)] = (data & 0xff00) >> 8;
		}
	}
	else
	{
		logerror("%06x: 68000 attempting to access Z80 (write) address space without bus\n", m_maincpu->pc());
	}
}


READ16_MEMBER(md_base_state::megadriv_68k_check_z80_bus )
{
	uint16_t retvalue;

	/* Double Dragon, Shadow of the Beast, Super Off Road, and Time Killers have buggy
	   sound programs.  They request the bus, then have a loop which waits for the bus
	   to be unavailable, checking for a 0 value due to bad coding.  The real hardware
	   appears to return bits of the next instruction in the unused bits, thus meaning
	   the value is never zero.  Time Killers is the most fussy, and doesn't like the
	   read_next_instruction function from system16, so I just return a random value
	   in the unused bits */
	uint16_t nextvalue = machine().rand();//read_next_instruction(space)&0xff00;


	/* Check if the 68k has the z80 bus */
	if (!ACCESSING_BITS_0_7) // byte (MSB) access
	{
		if (m_genz80.z80_has_bus || m_genz80.z80_is_reset) retvalue = nextvalue | 0x0100;
		else retvalue = (nextvalue & 0xfeff);

		//logerror("%06x: 68000 check z80 Bus (byte MSB access) returning %04x mask %04x\n", m_maincpu->pc(),retvalue, mem_mask);
		return retvalue;

	}
	else if (!ACCESSING_BITS_8_15) // is this valid?
	{
		//logerror("%06x: 68000 check z80 Bus (byte LSB access) %04x\n", m_maincpu->pc(),mem_mask);
		if (m_genz80.z80_has_bus || m_genz80.z80_is_reset) retvalue = 0x0001;
		else retvalue = 0x0000;

		return retvalue;
	}
	else
	{
		//logerror("%06x: 68000 check z80 Bus (word access) %04x\n", m_maincpu->pc(),mem_mask);
		if (m_genz80.z80_has_bus || m_genz80.z80_is_reset) retvalue = nextvalue | 0x0100;
		else retvalue = (nextvalue & 0xfeff);

	//  osd_printf_debug("%06x: 68000 check z80 Bus (word access) %04x %04x\n", m_maincpu->pc(),mem_mask, retvalue);
		return retvalue;
	}
}


TIMER_CALLBACK_MEMBER(md_base_state::megadriv_z80_run_state)
{
	/* Is the z80 RESET line pulled? */
	if (m_genz80.z80_is_reset)
	{
		m_z80snd->reset();
		m_z80snd->suspend(SUSPEND_REASON_HALT, 1);
		m_ymsnd->reset();
	}
	else
	{
		/* Check if z80 has the bus */
		if (m_genz80.z80_has_bus)
			m_z80snd->resume(SUSPEND_REASON_HALT);
		else
			m_z80snd->suspend(SUSPEND_REASON_HALT, 1);
	}
}


WRITE16_MEMBER(md_base_state::megadriv_68k_req_z80_bus )
{
	/* Request the Z80 bus, allows 68k to read/write Z80 address space */
	if (!ACCESSING_BITS_0_7) // byte access
	{
		if (data & 0x0100)
		{
			//logerror("%06x: 68000 request z80 Bus (byte MSB access) %04x %04x\n", m_maincpu->pc(),data,mem_mask);
			m_genz80.z80_has_bus = 0;
		}
		else
		{
			//logerror("%06x: 68000 return z80 Bus (byte MSB access) %04x %04x\n", m_maincpu->pc(),data,mem_mask);
			m_genz80.z80_has_bus = 1;
		}
	}
	else if (!ACCESSING_BITS_8_15) // is this valid?
	{
		if (data & 0x0001)
		{
			//logerror("%06x: 68000 request z80 Bus (byte LSB access) %04x %04x\n", m_maincpu->pc(),data,mem_mask);
			m_genz80.z80_has_bus = 0;
		}
		else
		{
			//logerror("%06x: 68000 return z80 Bus (byte LSB access) %04x %04x\n", m_maincpu->pc(),data,mem_mask);
			m_genz80.z80_has_bus = 1;
		}
	}
	else // word access
	{
		if (data & 0x0100)
		{
			//logerror("%06x: 68000 request z80 Bus (word access) %04x %04x\n", m_maincpu->pc(),data,mem_mask);
			m_genz80.z80_has_bus = 0;
		}
		else
		{
			//logerror("%06x: 68000 return z80 Bus (byte LSB access) %04x %04x\n", m_maincpu->pc(),data,mem_mask);
			m_genz80.z80_has_bus = 1;
		}
	}

	/* If the z80 is running, sync the z80 execution state */
	if (!m_genz80.z80_is_reset)
		machine().scheduler().timer_set(attotime::zero, timer_expired_delegate(FUNC(md_base_state::megadriv_z80_run_state),this));
}

WRITE16_MEMBER(md_base_state::megadriv_68k_req_z80_reset )
{
	if (!ACCESSING_BITS_0_7) // byte access
	{
		if (data & 0x0100)
		{
			//logerror("%06x: 68000 clear z80 reset (byte MSB access) %04x %04x\n", m_maincpu->pc(),data,mem_mask);
			m_genz80.z80_is_reset = 0;
		}
		else
		{
			//logerror("%06x: 68000 start z80 reset (byte MSB access) %04x %04x\n", m_maincpu->pc(),data,mem_mask);
			m_genz80.z80_is_reset = 1;
		}
	}
	else if (!ACCESSING_BITS_8_15) // is this valid?
	{
		if (data & 0x0001)
		{
			//logerror("%06x: 68000 clear z80 reset (byte LSB access) %04x %04x\n", m_maincpu->pc(),data,mem_mask);
			m_genz80.z80_is_reset = 0;
		}
		else
		{
			//logerror("%06x: 68000 start z80 reset (byte LSB access) %04x %04x\n", m_maincpu->pc(),data,mem_mask);
			m_genz80.z80_is_reset = 1;
		}
	}
	else // word access
	{
		if (data & 0x0100)
		{
			//logerror("%06x: 68000 clear z80 reset (word access) %04x %04x\n", m_maincpu->pc(),data,mem_mask);
			m_genz80.z80_is_reset = 0;
		}
		else
		{
			//logerror("%06x: 68000 start z80 reset (byte LSB access) %04x %04x\n", m_maincpu->pc(),data,mem_mask);
			m_genz80.z80_is_reset = 1;
		}
	}
	machine().scheduler().timer_set(attotime::zero, timer_expired_delegate(FUNC(md_base_state::megadriv_z80_run_state),this));
}


// just directly access the 68k space, this makes it easier to deal with
// add-on hardware which changes the cpu mapping like the 32x and SegaCD.
// - we might need to add exceptions for example, z80 reading / writing the
//   z80 area of the 68k if games misbehave
READ8_MEMBER(md_base_state::z80_read_68k_banked_data )
{
	address_space &space68k = m_maincpu->space();
	uint8_t ret = space68k.read_byte(m_genz80.z80_bank_addr+offset);
	return ret;
}

WRITE8_MEMBER(md_base_state::z80_write_68k_banked_data )
{
	address_space &space68k = m_maincpu->space();
	space68k.write_byte(m_genz80.z80_bank_addr+offset,data);
}


WRITE8_MEMBER(md_base_state::megadriv_z80_vdp_write )
{
	switch (offset)
	{
		case 0x11:
		case 0x13:
		case 0x15:
		case 0x17:
			// accessed by either segapsg_device or sn76496_device
			m_vdp->vdp_w(space, offset >> 1, data, 0x00ff);
			break;

		default:
			osd_printf_debug("unhandled z80 vdp write %02x %02x\n",offset,data);
	}

}



READ8_MEMBER(md_base_state::megadriv_z80_vdp_read )
{
	osd_printf_debug("megadriv_z80_vdp_read %02x\n",offset);
	return machine().rand();
}

READ8_MEMBER(md_base_state::megadriv_z80_unmapped_read )
{
	return 0xff;
}

void md_base_state::megadriv_z80_map(address_map &map)
{
	map(0x0000, 0x1fff).bankrw("bank1").mirror(0x2000); // RAM can be accessed by the 68k
	map(0x4000, 0x4003).rw(m_ymsnd, FUNC(ym2612_device::read), FUNC(ym2612_device::write));

	map(0x6000, 0x6000).w(FUNC(md_base_state::megadriv_z80_z80_bank_w));
	map(0x6001, 0x6001).w(FUNC(md_base_state::megadriv_z80_z80_bank_w)); // wacky races uses this address

	map(0x6100, 0x7eff).r(FUNC(md_base_state::megadriv_z80_unmapped_read));

	map(0x7f00, 0x7fff).rw(FUNC(md_base_state::megadriv_z80_vdp_read), FUNC(md_base_state::megadriv_z80_vdp_write));

	map(0x8000, 0xffff).rw(FUNC(md_base_state::z80_read_68k_banked_data), FUNC(md_base_state::z80_write_68k_banked_data)); // The Z80 can read the 68k address space this way
}

void md_base_state::megadriv_z80_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x0000, 0xff).noprw();
}

uint32_t md_base_state::screen_update_megadriv(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* Copy our screen buffer here */
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		uint32_t* desty = &bitmap.pix32(y, 0);
		uint32_t* srcy;

		if (!m_vdp->m_use_alt_timing)
			srcy = &m_vdp->m_render_bitmap->pix32(y, 0);
		else
			srcy = m_vdp->m_render_line.get();

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			desty[x] = srcy[x];
		}
	}

	return 0;
}



/*****************************************************************************************/

VIDEO_START_MEMBER(md_base_state,megadriv)
{
}

MACHINE_START_MEMBER(md_base_state,megadriv)
{
	m_io_pad_3b[0] = ioport("PAD1");
	m_io_pad_3b[1] = ioport("PAD2");
	m_io_pad_3b[2] = ioport("IN0");
	m_io_pad_3b[3] = ioport("UNK");

	save_item(NAME(m_io_stage));
	save_item(NAME(m_megadrive_io_data_regs));
	save_item(NAME(m_megadrive_io_ctrl_regs));
	save_item(NAME(m_megadrive_io_tx_regs));
}

MACHINE_RESET_MEMBER(md_base_state,megadriv)
{
	/* default state of z80 = reset, with bus */
	osd_printf_debug("Resetting Megadrive / Genesis\n");

	if (m_z80snd)
	{
		m_genz80.z80_is_reset = 1;
		m_genz80.z80_has_bus = 1;
		m_genz80.z80_bank_addr = 0;
		m_vdp->set_scanline_counter(-1);
		machine().scheduler().timer_set(attotime::zero, timer_expired_delegate(FUNC(md_base_state::megadriv_z80_run_state),this));
	}

	megadrive_reset_io();

	if (!m_vdp->m_use_alt_timing)
	{
		m_vdp->m_megadriv_scanline_timer = m_scan_timer;
		m_vdp->m_megadriv_scanline_timer->adjust(attotime::zero);
	}

	if (m_megadrive_ram)
		memset(m_megadrive_ram, 0x00, 0x10000);

	m_vdp->device_reset_old();
}

void md_base_state::megadriv_stop_scanline_timer()
{
	if (!m_vdp->m_use_alt_timing)
		m_vdp->m_megadriv_scanline_timer->reset();
}



// this comes from the VDP on lines 240 (on) 241 (off) and is connected to the z80 irq 0
WRITE_LINE_MEMBER(md_base_state::vdp_sndirqline_callback_genesis_z80)
{
	if (m_z80snd)
	{
		if (state == ASSERT_LINE)
		{
			if ((m_genz80.z80_has_bus == 1) && (m_genz80.z80_is_reset == 0))
				m_z80snd->set_input_line(0, HOLD_LINE);
		}
		else if (state == CLEAR_LINE)
		{
			m_z80snd->set_input_line(0, CLEAR_LINE);
		}
	}
}

// this comes from the vdp, and is connected to 68k irq level 6 (main vbl interrupt)
WRITE_LINE_MEMBER(md_base_state::vdp_lv6irqline_callback_genesis_68k)
{
	if (state == ASSERT_LINE)
		m_maincpu->set_input_line(6, HOLD_LINE);
	else
		m_maincpu->set_input_line(6, CLEAR_LINE);
}

// this comes from the vdp, and is connected to 68k irq level 4 (raster interrupt)
WRITE_LINE_MEMBER(md_base_state::vdp_lv4irqline_callback_genesis_68k)
{
	if (state == ASSERT_LINE)
		m_maincpu->set_input_line(4, HOLD_LINE);
	else
		m_maincpu->set_input_line(4, CLEAR_LINE);
}

/* Callback when the 68k takes an IRQ */
IRQ_CALLBACK_MEMBER(md_base_state::genesis_int_callback)
{
	if (irqline==4)
	{
		m_vdp->vdp_clear_irq4_pending();
	}

	if (irqline==6)
	{
		m_vdp->vdp_clear_irq6_pending();
	}

	return (0x60+irqline*4)/4; // vector address
}

void md_base_state::megadriv_timers(machine_config &config)
{
	TIMER(config, m_scan_timer).configure_generic("gen_vdp", FUNC(sega315_5313_device::megadriv_scanline_timer_callback));
}


void md_base_state::md_ntsc(machine_config &config)
{
	M68000(config, m_maincpu, MASTER_CLOCK_NTSC / 7); /* 7.67 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &md_base_state::megadriv_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(md_base_state::genesis_int_callback));

	/* IRQs are handled via the timers */

	Z80(config, m_z80snd, MASTER_CLOCK_NTSC / 15); /* 3.58 MHz */
	m_z80snd->set_addrmap(AS_PROGRAM, &md_base_state::megadriv_z80_map);
	m_z80snd->set_addrmap(AS_IO, &md_base_state::megadriv_z80_io_map);
	/* IRQ handled via the timers */

	MCFG_MACHINE_START_OVERRIDE(md_base_state,megadriv)
	MCFG_MACHINE_RESET_OVERRIDE(md_base_state,megadriv)

	megadriv_timers(config);

	SEGA315_5313(config, m_vdp, MASTER_CLOCK_NTSC, m_maincpu);
	m_vdp->set_is_pal(false);
	m_vdp->snd_irq().set(FUNC(md_base_state::vdp_sndirqline_callback_genesis_z80));
	m_vdp->lv6_irq().set(FUNC(md_base_state::vdp_lv6irqline_callback_genesis_68k));
	m_vdp->lv4_irq().set(FUNC(md_base_state::vdp_lv4irqline_callback_genesis_68k));
	m_vdp->set_screen("megadriv");
	m_vdp->add_route(ALL_OUTPUTS, "lspeaker", 0.25);
	m_vdp->add_route(ALL_OUTPUTS, "rspeaker", 0.25);

	screen_device &screen(SCREEN(config, "megadriv", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0)); // Vblank handled manually.
	screen.set_size(64*8, 620);
	screen.set_visarea(0, 32*8-1, 0, 28*8-1);
	screen.set_screen_update(FUNC(md_base_state::screen_update_megadriv)); /* Copies a bitmap */
	screen.screen_vblank().set(FUNC(md_base_state::screen_vblank_megadriv)); /* Used to Sync the timing */

	MCFG_VIDEO_START_OVERRIDE(md_base_state, megadriv)

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	YM2612(config, m_ymsnd, MASTER_CLOCK_NTSC/7); /* 7.67 MHz */
	m_ymsnd->add_route(0, "lspeaker", 0.50);
	m_ymsnd->add_route(1, "rspeaker", 0.50);
}

void md_cons_state::dcat16_megadriv_base(machine_config &config)
{
	md_ntsc(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &md_base_state::dcat16_megadriv_map);
}

/************ PAL hardware has a different master clock *************/

void md_base_state::md_pal(machine_config &config)
{
	M68000(config, m_maincpu, MASTER_CLOCK_PAL / 7); /* 7.67 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &md_base_state::megadriv_map);
	m_maincpu->set_irq_acknowledge_callback(FUNC(md_base_state::genesis_int_callback));
	/* IRQs are handled via the timers */

	Z80(config, m_z80snd, MASTER_CLOCK_PAL / 15); /* 3.58 MHz */
	m_z80snd->set_addrmap(AS_PROGRAM, &md_base_state::megadriv_z80_map);
	m_z80snd->set_addrmap(AS_IO, &md_base_state::megadriv_z80_io_map);
	/* IRQ handled via the timers */

	MCFG_MACHINE_START_OVERRIDE(md_base_state,megadriv)
	MCFG_MACHINE_RESET_OVERRIDE(md_base_state,megadriv)

	megadriv_timers(config);

	SEGA315_5313(config, m_vdp, MASTER_CLOCK_PAL, m_maincpu);
	m_vdp->set_is_pal(true);
	m_vdp->snd_irq().set(FUNC(md_base_state::vdp_sndirqline_callback_genesis_z80));
	m_vdp->lv6_irq().set(FUNC(md_base_state::vdp_lv6irqline_callback_genesis_68k));
	m_vdp->lv4_irq().set(FUNC(md_base_state::vdp_lv4irqline_callback_genesis_68k));
	m_vdp->set_screen("megadriv");
	m_vdp->add_route(ALL_OUTPUTS, "lspeaker", 0.25);
	m_vdp->add_route(ALL_OUTPUTS, "rspeaker", 0.25);

	screen_device &screen(SCREEN(config, "megadriv", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0)); // Vblank handled manually.
	screen.set_size(64*8, 620);
	screen.set_visarea(0, 32*8-1, 0, 28*8-1);
	screen.set_screen_update(FUNC(md_base_state::screen_update_megadriv)); /* Copies a bitmap */
	screen.screen_vblank().set(FUNC(md_base_state::screen_vblank_megadriv)); /* Used to Sync the timing */

	MCFG_VIDEO_START_OVERRIDE(md_base_state, megadriv)

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	YM2612(config, m_ymsnd, MASTER_CLOCK_NTSC/7); /* 7.67 MHz */
	m_ymsnd->add_route(0, "lspeaker", 0.50);
	m_ymsnd->add_route(1, "rspeaker", 0.50);
}


WRITE8_MEMBER(md_base_state::megadriv_tas_callback)
{
	return; // writeback not allowed
}

void md_base_state::megadriv_init_common()
{
	/* Look to see if this system has the standard Sound Z80 */
	if (m_z80snd)
	{
		m_genz80.z80_prgram = std::make_unique<uint8_t[]>(0x2000);
		membank("bank1")->set_base(m_genz80.z80_prgram.get());
		save_item(NAME(m_genz80.z80_is_reset));
		save_item(NAME(m_genz80.z80_has_bus));
		save_item(NAME(m_genz80.z80_bank_addr));
		save_pointer(NAME(m_genz80.z80_prgram), 0x2000);
	}

	m_maincpu->set_tas_write_callback(write8_delegate(FUNC(md_base_state::megadriv_tas_callback),this));

	m_megadrive_io_read_data_port_ptr = read8_delegate(FUNC(md_base_state::megadrive_io_read_data_port_3button),this);
	m_megadrive_io_write_data_port_ptr = write16_delegate(FUNC(md_base_state::megadrive_io_write_data_port_3button),this);
}

void md_base_state::init_megadriv_c2()
{
	megadriv_init_common();

	m_vdp->set_use_cram(0); // C2 uses its own palette ram
	m_vdp->set_vdp_pal(false);
	m_vdp->set_framerate(60);
	m_vdp->set_total_scanlines(262);

	m_version_hi_nibble = 0x20; // JPN NTSC no-SCD
}



void md_base_state::init_megadriv()
{
	megadriv_init_common();

	// todo: move this to the device interface?
	m_vdp->set_use_cram(1);
	m_vdp->set_vdp_pal(false);
	m_vdp->set_framerate(60);
	m_vdp->set_total_scanlines(262);

	m_version_hi_nibble = 0xa0; // Export NTSC no-SCD
}

void md_base_state::init_megadrij()
{
	megadriv_init_common();

	// todo: move this to the device interface?
	m_vdp->set_use_cram(1);
	m_vdp->set_vdp_pal(false);
	m_vdp->set_framerate(60);
	m_vdp->set_total_scanlines(262);

	m_version_hi_nibble = 0x20; // JPN NTSC no-SCD
}

void md_base_state::init_megadrie()
{
	megadriv_init_common();

	// todo: move this to the device interface?
	m_vdp->set_use_cram(1);
	m_vdp->set_vdp_pal(true);
	m_vdp->set_framerate(50);
	m_vdp->set_total_scanlines(313);

	m_version_hi_nibble = 0xe0; // Export PAL no-SCD
}

WRITE_LINE_MEMBER(md_base_state::screen_vblank_megadriv)
{
	if (m_io_reset.read_safe(0) & 0x01)
		m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);

	// rising edge
	if (state)
	{
		if (!m_vdp->m_use_alt_timing)
		{
			m_vdp->vdp_handle_eof();
			m_vdp->m_megadriv_scanline_timer->adjust(attotime::zero);
		}
	}
}
