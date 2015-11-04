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


MACHINE_CONFIG_EXTERN( megadriv );

void megadriv_z80_hold(running_machine &machine)
{
	md_base_state *state = machine.driver_data<md_base_state>();
	if ((state->m_genz80.z80_has_bus == 1) && (state->m_genz80.z80_is_reset == 0))
		state->m_z80snd->set_input_line(0, HOLD_LINE);
}

void megadriv_z80_clear(running_machine &machine)
{
	md_base_state *state = machine.driver_data<md_base_state>();
	state->m_z80snd->set_input_line(0, CLEAR_LINE);
}

void md_base_state::megadriv_z80_bank_w(UINT16 data)
{
	m_genz80.z80_bank_addr = ((m_genz80.z80_bank_addr >> 1) | (data << 23)) & 0xff8000;
}

WRITE16_MEMBER(md_base_state::megadriv_68k_z80_bank_write )
{
	//logerror("%06x: 68k writing bit to bank register %01x\n", space.device().safe_pc(),data&0x01);
	megadriv_z80_bank_w(data & 0x01);
}

WRITE8_MEMBER(md_base_state::megadriv_z80_z80_bank_w)
{
	//logerror("%04x: z80 writing bit to bank register %01x\n", space.device().safe_pc(),data&0x01);
	megadriv_z80_bank_w(data & 0x01);
}

READ8_MEMBER(md_base_state::megadriv_68k_YM2612_read)
{
	//osd_printf_debug("megadriv_68k_YM2612_read %02x %04x\n",offset,mem_mask);
	if ((m_genz80.z80_has_bus == 0) && (m_genz80.z80_is_reset == 0))
	{
		return m_ymsnd->read(space, offset);
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
		m_ymsnd->write(space, offset, data);
	}
	else
	{
		logerror("%s: 68000 attempting to access YM2612 (write) without bus\n", machine().describe_context());
	}
}

// this is used by 6 button pads and gets installed in machine_start for drivers requiring it
TIMER_CALLBACK_MEMBER(md_base_state::io_timeout_timer_callback)
{
	m_io_stage[(int)(FPTR)ptr] = -1;
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
	UINT8 retdata, helper = (m_megadrive_io_ctrl_regs[portnum] & 0x3f) | 0xc0; // bits 6 & 7 always come from m_megadrive_io_data_regs

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
	UINT8 retdata, helper = (m_megadrive_io_ctrl_regs[portnum] & 0x7f) | 0x80; // bit 7 always comes from m_megadrive_io_data_regs

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

UINT8 md_base_state::megadrive_io_read_ctrl_port(int portnum)
{
	UINT8 retdata;
	retdata = m_megadrive_io_ctrl_regs[portnum];
	//osd_printf_debug("read io ctrl port %d %02x\n",portnum,retdata);

	return retdata | (retdata << 8);
}

UINT8 md_base_state::megadrive_io_read_tx_port(int portnum)
{
	UINT8 retdata;
	retdata = m_megadrive_io_tx_regs[portnum];
	return retdata | (retdata << 8);
}

UINT8 md_base_state::megadrive_io_read_rx_port(int portnum)
{
	return 0x00;
}

UINT8 md_base_state::megadrive_io_read_sctrl_port(int portnum)
{
	return 0x00;
}


READ16_MEMBER(md_base_state::megadriv_68k_io_read )
{
	UINT8 retdata;

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

	//return (space.machine().rand()&0x0f0f)|0xf0f0;//0x0000;
	switch (offset)
	{
		case 0:
			logerror("%06x read version register\n", space.device().safe_pc());
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

void md_base_state::megadrive_io_write_ctrl_port(int portnum, UINT16 data)
{
	m_megadrive_io_ctrl_regs[portnum] = data;
//  osd_printf_debug("Setting IO Control Register #%d data %04x\n",portnum,data);
}

void md_base_state::megadrive_io_write_tx_port(int portnum, UINT16 data)
{
	m_megadrive_io_tx_regs[portnum] = data;
}

void md_base_state::megadrive_io_write_rx_port(int portnum, UINT16 data)
{
}

void md_base_state::megadrive_io_write_sctrl_port(int portnum, UINT16 data)
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



static ADDRESS_MAP_START( megadriv_map, AS_PROGRAM, 16, md_base_state )
	AM_RANGE(0x000000, 0x3fffff) AM_ROM
	/*      (0x000000 - 0x3fffff) == GAME ROM (4Meg Max, Some games have special banking too) */

	AM_RANGE(0xa00000, 0xa01fff) AM_READWRITE(megadriv_68k_read_z80_ram,megadriv_68k_write_z80_ram)
	AM_RANGE(0xa02000, 0xa03fff) AM_WRITE(megadriv_68k_write_z80_ram)
	AM_RANGE(0xa04000, 0xa04003) AM_READWRITE8(megadriv_68k_YM2612_read,megadriv_68k_YM2612_write, 0xffff)

	AM_RANGE(0xa06000, 0xa06001) AM_WRITE(megadriv_68k_z80_bank_write)

	AM_RANGE(0xa10000, 0xa1001f) AM_READWRITE(megadriv_68k_io_read,megadriv_68k_io_write)

	AM_RANGE(0xa11100, 0xa11101) AM_READWRITE(megadriv_68k_check_z80_bus,megadriv_68k_req_z80_bus)
	AM_RANGE(0xa11200, 0xa11201) AM_WRITE(megadriv_68k_req_z80_reset)

	/* these are fake - remove allocs in video_start to use these to view ram instead */
//  AM_RANGE(0xb00000, 0xb0ffff) AM_RAM AM_SHARE("megadrive_vdp_vram")
//  AM_RANGE(0xb10000, 0xb1007f) AM_RAM AM_SHARE("megadrive_vdp_vsram")
//  AM_RANGE(0xb10100, 0xb1017f) AM_RAM AM_SHARE("megadrive_vdp_cram")

	AM_RANGE(0xc00000, 0xc0001f) AM_DEVREADWRITE("gen_vdp", sega315_5313_device, vdp_r, vdp_w)
	AM_RANGE(0xd00000, 0xd0001f) AM_DEVREADWRITE("gen_vdp", sega315_5313_device, vdp_r, vdp_w) // the earth defend
	AM_RANGE(0xe00000, 0xe0ffff) AM_RAM AM_MIRROR(0x1f0000) AM_SHARE("megadrive_ram")
//  AM_RANGE(0xff0000, 0xffffff) AM_READONLY
	/*       0xe00000 - 0xffffff) == MAIN RAM (64kb, Mirrored, most games use ff0000 - ffffff) */
ADDRESS_MAP_END


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
		logerror("%06x: 68000 attempting to access Z80 (read) address space without bus\n", space.device().safe_pc());
		return space.machine().rand();
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
		logerror("%06x: 68000 attempting to access Z80 (write) address space without bus\n", space.device().safe_pc());
	}
}


READ16_MEMBER(md_base_state::megadriv_68k_check_z80_bus )
{
	UINT16 retvalue;

	/* Double Dragon, Shadow of the Beast, Super Off Road, and Time Killers have buggy
	   sound programs.  They request the bus, then have a loop which waits for the bus
	   to be unavailable, checking for a 0 value due to bad coding.  The real hardware
	   appears to return bits of the next instruction in the unused bits, thus meaning
	   the value is never zero.  Time Killers is the most fussy, and doesn't like the
	   read_next_instruction function from system16, so I just return a random value
	   in the unused bits */
	UINT16 nextvalue = space.machine().rand();//read_next_instruction(space)&0xff00;


	/* Check if the 68k has the z80 bus */
	if (!ACCESSING_BITS_0_7) // byte (MSB) access
	{
		if (m_genz80.z80_has_bus || m_genz80.z80_is_reset) retvalue = nextvalue | 0x0100;
		else retvalue = (nextvalue & 0xfeff);

		//logerror("%06x: 68000 check z80 Bus (byte MSB access) returning %04x mask %04x\n", space.device().safe_pc(),retvalue, mem_mask);
		return retvalue;

	}
	else if (!ACCESSING_BITS_8_15) // is this valid?
	{
		//logerror("%06x: 68000 check z80 Bus (byte LSB access) %04x\n", space.device().safe_pc(),mem_mask);
		if (m_genz80.z80_has_bus || m_genz80.z80_is_reset) retvalue = 0x0001;
		else retvalue = 0x0000;

		return retvalue;
	}
	else
	{
		//logerror("%06x: 68000 check z80 Bus (word access) %04x\n", space.device().safe_pc(),mem_mask);
		if (m_genz80.z80_has_bus || m_genz80.z80_is_reset) retvalue = nextvalue | 0x0100;
		else retvalue = (nextvalue & 0xfeff);

	//  osd_printf_debug("%06x: 68000 check z80 Bus (word access) %04x %04x\n", space.device().safe_pc(),mem_mask, retvalue);
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
			//logerror("%06x: 68000 request z80 Bus (byte MSB access) %04x %04x\n", space.device().safe_pc(),data,mem_mask);
			m_genz80.z80_has_bus = 0;
		}
		else
		{
			//logerror("%06x: 68000 return z80 Bus (byte MSB access) %04x %04x\n", space.device().safe_pc(),data,mem_mask);
			m_genz80.z80_has_bus = 1;
		}
	}
	else if (!ACCESSING_BITS_8_15) // is this valid?
	{
		if (data & 0x0001)
		{
			//logerror("%06x: 68000 request z80 Bus (byte LSB access) %04x %04x\n", space.device().safe_pc(),data,mem_mask);
			m_genz80.z80_has_bus = 0;
		}
		else
		{
			//logerror("%06x: 68000 return z80 Bus (byte LSB access) %04x %04x\n", space.device().safe_pc(),data,mem_mask);
			m_genz80.z80_has_bus = 1;
		}
	}
	else // word access
	{
		if (data & 0x0100)
		{
			//logerror("%06x: 68000 request z80 Bus (word access) %04x %04x\n", space.device().safe_pc(),data,mem_mask);
			m_genz80.z80_has_bus = 0;
		}
		else
		{
			//logerror("%06x: 68000 return z80 Bus (byte LSB access) %04x %04x\n", space.device().safe_pc(),data,mem_mask);
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
			//logerror("%06x: 68000 clear z80 reset (byte MSB access) %04x %04x\n", space.device().safe_pc(),data,mem_mask);
			m_genz80.z80_is_reset = 0;
		}
		else
		{
			//logerror("%06x: 68000 start z80 reset (byte MSB access) %04x %04x\n", space.device().safe_pc(),data,mem_mask);
			m_genz80.z80_is_reset = 1;
		}
	}
	else if (!ACCESSING_BITS_8_15) // is this valid?
	{
		if (data & 0x0001)
		{
			//logerror("%06x: 68000 clear z80 reset (byte LSB access) %04x %04x\n", space.device().safe_pc(),data,mem_mask);
			m_genz80.z80_is_reset = 0;
		}
		else
		{
			//logerror("%06x: 68000 start z80 reset (byte LSB access) %04x %04x\n", space.device().safe_pc(),data,mem_mask);
			m_genz80.z80_is_reset = 1;
		}
	}
	else // word access
	{
		if (data & 0x0100)
		{
			//logerror("%06x: 68000 clear z80 reset (word access) %04x %04x\n", space.device().safe_pc(),data,mem_mask);
			m_genz80.z80_is_reset = 0;
		}
		else
		{
			//logerror("%06x: 68000 start z80 reset (byte LSB access) %04x %04x\n", space.device().safe_pc(),data,mem_mask);
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
	UINT8 ret = space68k.read_byte(m_genz80.z80_bank_addr+offset);
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
			m_snsnd->write(space, 0, data);
			break;

		default:
			osd_printf_debug("unhandled z80 vdp write %02x %02x\n",offset,data);
	}

}



READ8_MEMBER(md_base_state::megadriv_z80_vdp_read )
{
	osd_printf_debug("megadriv_z80_vdp_read %02x\n",offset);
	return space.machine().rand();
}

READ8_MEMBER(md_base_state::megadriv_z80_unmapped_read )
{
	return 0xff;
}

static ADDRESS_MAP_START( megadriv_z80_map, AS_PROGRAM, 8, md_base_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAMBANK("bank1") AM_MIRROR(0x2000) // RAM can be accessed by the 68k
	AM_RANGE(0x4000, 0x4003) AM_DEVREADWRITE("ymsnd", ym2612_device, read, write)

	AM_RANGE(0x6000, 0x6000) AM_WRITE(megadriv_z80_z80_bank_w)
	AM_RANGE(0x6001, 0x6001) AM_WRITE(megadriv_z80_z80_bank_w) // wacky races uses this address

	AM_RANGE(0x6100, 0x7eff) AM_READ(megadriv_z80_unmapped_read)

	AM_RANGE(0x7f00, 0x7fff) AM_READWRITE(megadriv_z80_vdp_read,megadriv_z80_vdp_write)

	AM_RANGE(0x8000, 0xffff) AM_READWRITE(z80_read_68k_banked_data,z80_write_68k_banked_data) // The Z80 can read the 68k address space this way
ADDRESS_MAP_END

static ADDRESS_MAP_START( megadriv_z80_io_map, AS_IO, 8, md_base_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x0000, 0xff) AM_NOP
ADDRESS_MAP_END


/************************************ Megadrive Bootlegs *************************************/

// smaller ROM region because some bootlegs check for RAM there (used by topshoot and hshavoc)
static ADDRESS_MAP_START( md_bootleg_map, AS_PROGRAM, 16, md_boot_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM /* Cartridge Program Rom */
	AM_RANGE(0x200000, 0x2023ff) AM_RAM // tested

	AM_RANGE(0xa00000, 0xa01fff) AM_READWRITE(megadriv_68k_read_z80_ram, megadriv_68k_write_z80_ram)
	AM_RANGE(0xa02000, 0xa03fff) AM_WRITE(megadriv_68k_write_z80_ram)
	AM_RANGE(0xa04000, 0xa04003) AM_READWRITE8(megadriv_68k_YM2612_read, megadriv_68k_YM2612_write, 0xffff)
	AM_RANGE(0xa06000, 0xa06001) AM_WRITE(megadriv_68k_z80_bank_write)

	AM_RANGE(0xa10000, 0xa1001f) AM_READWRITE(megadriv_68k_io_read, megadriv_68k_io_write)
	AM_RANGE(0xa11100, 0xa11101) AM_READWRITE(megadriv_68k_check_z80_bus, megadriv_68k_req_z80_bus)
	AM_RANGE(0xa11200, 0xa11201) AM_WRITE(megadriv_68k_req_z80_reset)

	AM_RANGE(0xc00000, 0xc0001f) AM_DEVREADWRITE("gen_vdp", sega315_5313_device, vdp_r, vdp_w)
	AM_RANGE(0xd00000, 0xd0001f) AM_DEVREADWRITE("gen_vdp", sega315_5313_device, vdp_r, vdp_w)

	AM_RANGE(0xe00000, 0xe0ffff) AM_RAM AM_MIRROR(0x1f0000) AM_SHARE("megadrive_ram")
ADDRESS_MAP_END

MACHINE_CONFIG_START( md_bootleg, md_boot_state )
	MCFG_FRAGMENT_ADD( md_ntsc )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(md_bootleg_map)
MACHINE_CONFIG_END



UINT32 md_base_state::screen_update_megadriv(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	/* Copy our screen buffer here */
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		UINT32* desty = &bitmap.pix32(y, 0);
		UINT16* srcy;

		if (!m_vdp->m_use_alt_timing)
			srcy = &m_vdp->m_render_bitmap->pix(y, 0);
		else
			srcy = m_vdp->m_render_line;

		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			UINT16 src = srcy[x];
			desty[x] = rgb_t(pal5bit(src >> 10), pal5bit(src >> 5), pal5bit(src >> 0));
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
	m_io_reset = ioport("RESET");
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
		m_vdp->m_megadriv_scanline_timer = machine().device<timer_device>("md_scan_timer");
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
			megadriv_z80_hold(machine());
		}
		else if (state == CLEAR_LINE)
		{
			megadriv_z80_clear(machine());
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

MACHINE_CONFIG_FRAGMENT( megadriv_timers )
	MCFG_TIMER_DEVICE_ADD("md_scan_timer", "gen_vdp", sega315_5313_device, megadriv_scanline_timer_callback)
MACHINE_CONFIG_END


MACHINE_CONFIG_FRAGMENT( md_ntsc )
	MCFG_CPU_ADD("maincpu", M68000, MASTER_CLOCK_NTSC / 7) /* 7.67 MHz */
	MCFG_CPU_PROGRAM_MAP(megadriv_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(md_base_state,genesis_int_callback)

	/* IRQs are handled via the timers */

	MCFG_CPU_ADD("genesis_snd_z80", Z80, MASTER_CLOCK_NTSC / 15) /* 3.58 MHz */
	MCFG_CPU_PROGRAM_MAP(megadriv_z80_map)
	MCFG_CPU_IO_MAP(megadriv_z80_io_map)
	/* IRQ handled via the timers */

	MCFG_MACHINE_START_OVERRIDE(md_base_state,megadriv)
	MCFG_MACHINE_RESET_OVERRIDE(md_base_state,megadriv)

	MCFG_FRAGMENT_ADD(megadriv_timers)

	MCFG_DEVICE_ADD("gen_vdp", SEGA315_5313, 0)
	MCFG_SEGA315_5313_IS_PAL(false)
	MCFG_SEGA315_5313_SND_IRQ_CALLBACK(WRITELINE(md_base_state, vdp_sndirqline_callback_genesis_z80));
	MCFG_SEGA315_5313_LV6_IRQ_CALLBACK(WRITELINE(md_base_state, vdp_lv6irqline_callback_genesis_68k));
	MCFG_SEGA315_5313_LV4_IRQ_CALLBACK(WRITELINE(md_base_state, vdp_lv4irqline_callback_genesis_68k));
	MCFG_VIDEO_SET_SCREEN("megadriv")

	MCFG_SCREEN_ADD("megadriv", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0)) // Vblank handled manually.
	MCFG_SCREEN_SIZE(64*8, 620)
	MCFG_SCREEN_VISIBLE_AREA(0, 32*8-1, 0, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(md_base_state, screen_update_megadriv) /* Copies a bitmap */
	MCFG_SCREEN_VBLANK_DRIVER(md_base_state, screen_eof_megadriv) /* Used to Sync the timing */

	MCFG_VIDEO_START_OVERRIDE(md_base_state, megadriv)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2612, MASTER_CLOCK_NTSC/7) /* 7.67 MHz */
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)

	/* sound hardware */
	MCFG_SOUND_ADD("snsnd", SEGAPSG, MASTER_CLOCK_NTSC/15)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25) /* 3.58 MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker",0.25) /* 3.58 MHz */
MACHINE_CONFIG_END

/************ PAL hardware has a different master clock *************/

MACHINE_CONFIG_FRAGMENT( md_pal )
	MCFG_CPU_ADD("maincpu", M68000, MASTER_CLOCK_PAL / 7) /* 7.67 MHz */
	MCFG_CPU_PROGRAM_MAP(megadriv_map)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(md_base_state,genesis_int_callback)
	/* IRQs are handled via the timers */

	MCFG_CPU_ADD("genesis_snd_z80", Z80, MASTER_CLOCK_PAL / 15) /* 3.58 MHz */
	MCFG_CPU_PROGRAM_MAP(megadriv_z80_map)
	MCFG_CPU_IO_MAP(megadriv_z80_io_map)
	/* IRQ handled via the timers */

	MCFG_MACHINE_START_OVERRIDE(md_base_state,megadriv)
	MCFG_MACHINE_RESET_OVERRIDE(md_base_state,megadriv)

	MCFG_FRAGMENT_ADD(megadriv_timers)

	MCFG_DEVICE_ADD("gen_vdp", SEGA315_5313, 0)
	MCFG_SEGA315_5313_IS_PAL(true)
	MCFG_SEGA315_5313_SND_IRQ_CALLBACK(WRITELINE(md_base_state, vdp_sndirqline_callback_genesis_z80));
	MCFG_SEGA315_5313_LV6_IRQ_CALLBACK(WRITELINE(md_base_state, vdp_lv6irqline_callback_genesis_68k));
	MCFG_SEGA315_5313_LV4_IRQ_CALLBACK(WRITELINE(md_base_state, vdp_lv4irqline_callback_genesis_68k));
	MCFG_VIDEO_SET_SCREEN("megadriv")

	MCFG_SCREEN_ADD("megadriv", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0)) // Vblank handled manually.
	MCFG_SCREEN_SIZE(64*8, 620)
	MCFG_SCREEN_VISIBLE_AREA(0, 32*8-1, 0, 28*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(md_base_state, screen_update_megadriv) /* Copies a bitmap */
	MCFG_SCREEN_VBLANK_DRIVER(md_base_state, screen_eof_megadriv) /* Used to Sync the timing */

	MCFG_VIDEO_START_OVERRIDE(md_base_state, megadriv)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymsnd", YM2612, MASTER_CLOCK_PAL/7) /* 7.67 MHz */
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)

	/* sound hardware */
	MCFG_SOUND_ADD("snsnd", SEGAPSG, MASTER_CLOCK_PAL/15)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.25) /* 3.58 MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker",0.25) /* 3.58 MHz */
MACHINE_CONFIG_END


WRITE8_MEMBER(md_base_state::megadriv_tas_callback)
{
	return; // writeback not allowed
}

void md_base_state::megadriv_init_common()
{
	/* Look to see if this system has the standard Sound Z80 */
	if (m_z80snd)
	{
		//printf("GENESIS Sound Z80 cpu found '%s'\n", machine().device("genesis_snd_z80")->tag());
		m_genz80.z80_prgram = auto_alloc_array(machine(), UINT8, 0x2000);
		membank("bank1")->set_base(m_genz80.z80_prgram);
		save_item(NAME(m_genz80.z80_is_reset));
		save_item(NAME(m_genz80.z80_has_bus));
		save_item(NAME(m_genz80.z80_bank_addr));
		save_pointer(NAME(m_genz80.z80_prgram), 0x2000);
	}

	m_maincpu->set_tas_write_callback(write8_delegate(FUNC(md_base_state::megadriv_tas_callback),this));

	m_megadrive_io_read_data_port_ptr = read8_delegate(FUNC(md_base_state::megadrive_io_read_data_port_3button),this);
	m_megadrive_io_write_data_port_ptr = write16_delegate(FUNC(md_base_state::megadrive_io_write_data_port_3button),this);
}

DRIVER_INIT_MEMBER(md_base_state,megadriv_c2)
{
	megadriv_init_common();

	m_vdp->set_use_cram(0); // C2 uses its own palette ram
	m_vdp->set_vdp_pal(FALSE);
	m_vdp->set_framerate(60);
	m_vdp->set_total_scanlines(262);

	m_version_hi_nibble = 0x20; // JPN NTSC no-SCD
}



DRIVER_INIT_MEMBER(md_base_state, megadriv)
{
	megadriv_init_common();

	// todo: move this to the device interface?
	m_vdp->set_use_cram(1);
	m_vdp->set_vdp_pal(FALSE);
	m_vdp->set_framerate(60);
	m_vdp->set_total_scanlines(262);

	m_version_hi_nibble = 0xa0; // Export NTSC no-SCD
}

DRIVER_INIT_MEMBER(md_base_state, megadrij)
{
	megadriv_init_common();

	// todo: move this to the device interface?
	m_vdp->set_use_cram(1);
	m_vdp->set_vdp_pal(FALSE);
	m_vdp->set_framerate(60);
	m_vdp->set_total_scanlines(262);

	m_version_hi_nibble = 0x20; // JPN NTSC no-SCD
}

DRIVER_INIT_MEMBER(md_base_state, megadrie)
{
	megadriv_init_common();

	// todo: move this to the device interface?
	m_vdp->set_use_cram(1);
	m_vdp->set_vdp_pal(TRUE);
	m_vdp->set_framerate(50);
	m_vdp->set_total_scanlines(313);

	m_version_hi_nibble = 0xe0; // Export PAL no-SCD
}

void md_base_state::screen_eof_megadriv(screen_device &screen, bool state)
{
	if (m_io_reset && m_io_reset->read() & 0x01)
		m_maincpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);

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
