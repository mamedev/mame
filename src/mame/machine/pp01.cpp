// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        PP-01 driver by Miodrag Milanovic

        08/09/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "includes/pp01.h"


WRITE8_MEMBER(pp01_state::video_write_mode_w)
{
	m_video_write_mode = data & 0x0f;
}

void pp01_state::video_w(uint8_t block,uint16_t offset,uint8_t data,uint8_t part)
{
	uint16_t addroffset = part ? 0x1000  : 0x0000;
	uint8_t *ram = m_ram->pointer();

	if (BIT(m_video_write_mode,3)) {
		// Copy mode
		if(BIT(m_video_write_mode,0)) {
			ram[0x6000+offset+addroffset] = data;
		} else {
			ram[0x6000+offset+addroffset] = 0;
		}
		if(BIT(m_video_write_mode,1)) {
			ram[0xa000+offset+addroffset] = data;
		} else {
			ram[0xa000+offset+addroffset] = 0;
		}
		if(BIT(m_video_write_mode,2)) {
			ram[0xe000+offset+addroffset] = data;
		} else {
			ram[0xe000+offset+addroffset] = 0;
		}
	} else {
		if (block==0) {
			ram[0x6000+offset+addroffset] = data;
		}
		if (block==1) {
			ram[0xa000+offset+addroffset] = data;
		}
		if (block==2) {
			ram[0xe000+offset+addroffset] = data;
		}
	}
}

WRITE8_MEMBER(pp01_state::video_r_1_w)
{
	video_w(0,offset,data,0);
}
WRITE8_MEMBER(pp01_state::video_g_1_w)
{
	video_w(1,offset,data,0);
}
WRITE8_MEMBER(pp01_state::video_b_1_w)
{
	video_w(2,offset,data,0);
}

WRITE8_MEMBER(pp01_state::video_r_2_w)
{
	video_w(0,offset,data,1);
}
WRITE8_MEMBER(pp01_state::video_g_2_w)
{
	video_w(1,offset,data,1);
}
WRITE8_MEMBER(pp01_state::video_b_2_w)
{
	video_w(2,offset,data,1);
}


void pp01_state::set_memory(uint8_t block, uint8_t data)
{
	uint8_t *mem = memregion("maincpu")->base();
	address_space &space = m_maincpu->space(AS_PROGRAM);
	uint16_t startaddr = block*0x1000;
	uint16_t endaddr   = ((block+1)*0x1000)-1;
	uint8_t  blocknum  = block + 1;
	char bank[10];
	sprintf(bank,"bank%d",blocknum);
	if (data>=0xE0 && data<=0xEF) {
		// This is RAM
		space.install_read_bank (startaddr, endaddr, bank);
		switch(data) {
			case 0xe6 :
					space.install_write_handler(startaddr, endaddr, write8_delegate(FUNC(pp01_state::video_r_1_w),this));
					break;
			case 0xe7 :
					space.install_write_handler(startaddr, endaddr, write8_delegate(FUNC(pp01_state::video_r_2_w),this));
					break;
			case 0xea :
					space.install_write_handler(startaddr, endaddr, write8_delegate(FUNC(pp01_state::video_g_1_w),this));
					break;
			case 0xeb :
					space.install_write_handler(startaddr, endaddr, write8_delegate(FUNC(pp01_state::video_g_2_w),this));
					break;
			case 0xee :
					space.install_write_handler(startaddr, endaddr, write8_delegate(FUNC(pp01_state::video_b_1_w),this));
					break;
			case 0xef :
					space.install_write_handler(startaddr, endaddr, write8_delegate(FUNC(pp01_state::video_b_2_w),this));
					break;

			default :
					space.install_write_bank(startaddr, endaddr, bank);
					break;
		}

		membank(bank)->set_base(m_ram->pointer() + (data & 0x0F)* 0x1000);
	} else if (data>=0xF8) {
		space.install_read_bank (startaddr, endaddr, bank);
		space.unmap_write(startaddr, endaddr);
		membank(bank)->set_base(mem + ((data & 0x0F)-8)* 0x1000+0x10000);
	} else {
		logerror("%02x %02x\n",block,data);
		space.unmap_readwrite (startaddr, endaddr);
	}
}

void pp01_state::machine_reset()
{
	int i;
	memset(m_memory_block,0xff,16);
	for(i=0;i<16;i++) {
		m_memory_block[i] = 0xff;
		set_memory(i, 0xff);
	}
	m_uart->write_cts(0);
}

WRITE8_MEMBER(pp01_state::mem_block_w)
{
	m_memory_block[offset] = data;
	set_memory(offset, data);
}

READ8_MEMBER(pp01_state::mem_block_r)
{
	return m_memory_block[offset];
}

void pp01_state::machine_start()
{
}

TIMER_DEVICE_CALLBACK_MEMBER( pp01_state::kansas_r )
{
	if (m_rts)
	{
		m_cass_data[1] = m_cass_data[2] = m_cass_data[3] = 0;
		m_casspol = 1;
		return;
	}

	m_cass_data[1]++;
	m_cass_data[2]++;

	uint8_t cass_ws = (m_cass->input() > +0.04) ? 1 : 0;

	if (cass_ws != m_cass_data[0])
	{
		m_cass_data[0] = cass_ws;
		if (m_cass_data[1] > 13)
			m_casspol ^= 1;
		m_cass_data[1] = 0;
		m_cass_data[2] = 0;
		m_uart->write_rxd(m_casspol);
	}
	if ((m_cass_data[2] & 7)==2)
	{
		m_cass_data[3]++;
		m_uart->write_rxc(BIT(m_cass_data[3], 0));
	}
}

WRITE_LINE_MEMBER(pp01_state::z2_w)
{
	// incoming 1200Hz
	m_uart->write_txc(state);
	if (!m_txe)
		m_cass->output((m_txd ^ state) ? -1.0 : 1.0);
}

READ8_MEMBER(pp01_state::ppi1_porta_r)
{
	return m_video_scroll;
}
WRITE8_MEMBER(pp01_state::ppi1_porta_w)
{
	m_video_scroll = data;
}

READ8_MEMBER(pp01_state::ppi1_portb_r)
{
	static const char *const keynames[] = {
		"LINE0", "LINE1", "LINE2", "LINE3", "LINE4", "LINE5", "LINE6", "LINE7",
		"LINE8", "LINE9", "LINEA", "LINEB", "LINEC", "LINED", "LINEE", "LINEF"
	};

	return (ioport(keynames[m_key_line])->read() & 0x3F) | (ioport("LINEALL")->read() & 0xC0);
}
WRITE8_MEMBER(pp01_state::ppi1_portb_w)
{
	//logerror("pp01_8255_portb_w %02x\n",data);

}

WRITE8_MEMBER(pp01_state::ppi1_portc_w)
{
	if (BIT(data, 4))
		m_key_line = data & 0x0f;
	else
		m_speaker->level_w(BIT(data, 0));
}

READ8_MEMBER(pp01_state::ppi1_portc_r)
{
	return 0xff;
}
