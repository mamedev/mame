// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        PP-01 driver by Miodrag Milanovic

        08/09/2008 Preliminary driver.

****************************************************************************/


#include "includes/pp01.h"


void pp01_state::pp01_video_write_mode_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_video_write_mode = data & 0x0f;
}

void pp01_state::pp01_video_w(uint8_t block,uint16_t offset,uint8_t data,uint8_t part)
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

void pp01_state::pp01_video_r_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	pp01_video_w(0,offset,data,0);
}
void pp01_state::pp01_video_g_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	pp01_video_w(1,offset,data,0);
}
void pp01_state::pp01_video_b_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	pp01_video_w(2,offset,data,0);
}

void pp01_state::pp01_video_r_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	pp01_video_w(0,offset,data,1);
}
void pp01_state::pp01_video_g_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	pp01_video_w(1,offset,data,1);
}
void pp01_state::pp01_video_b_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	pp01_video_w(2,offset,data,1);
}


void pp01_state::pp01_set_memory(uint8_t block, uint8_t data)
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
					space.install_write_handler(startaddr, endaddr, write8_delegate(FUNC(pp01_state::pp01_video_r_1_w),this));
					break;
			case 0xe7 :
					space.install_write_handler(startaddr, endaddr, write8_delegate(FUNC(pp01_state::pp01_video_r_2_w),this));
					break;
			case 0xea :
					space.install_write_handler(startaddr, endaddr, write8_delegate(FUNC(pp01_state::pp01_video_g_1_w),this));
					break;
			case 0xeb :
					space.install_write_handler(startaddr, endaddr, write8_delegate(FUNC(pp01_state::pp01_video_g_2_w),this));
					break;
			case 0xee :
					space.install_write_handler(startaddr, endaddr, write8_delegate(FUNC(pp01_state::pp01_video_b_1_w),this));
					break;
			case 0xef :
					space.install_write_handler(startaddr, endaddr, write8_delegate(FUNC(pp01_state::pp01_video_b_2_w),this));
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
		pp01_set_memory(i, 0xff);
	}
}

void pp01_state::pp01_mem_block_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_memory_block[offset] = data;
	pp01_set_memory(offset, data);
}

uint8_t pp01_state::pp01_mem_block_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return  m_memory_block[offset];
}

void pp01_state::machine_start()
{
}


void pp01_state::pp01_pit_out0(int state)
{
}

void pp01_state::pp01_pit_out1(int state)
{
}

uint8_t pp01_state::pp01_8255_porta_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return m_video_scroll;
}
void pp01_state::pp01_8255_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_video_scroll = data;
}

uint8_t pp01_state::pp01_8255_portb_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	static const char *const keynames[] = {
		"LINE0", "LINE1", "LINE2", "LINE3", "LINE4", "LINE5", "LINE6", "LINE7",
		"LINE8", "LINE9", "LINEA", "LINEB", "LINEC", "LINED", "LINEE", "LINEF"
	};

	return (ioport(keynames[m_key_line])->read() & 0x3F) | (ioport("LINEALL")->read() & 0xC0);
}
void pp01_state::pp01_8255_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	//logerror("pp01_8255_portb_w %02x\n",data);

}

void pp01_state::pp01_8255_portc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	if (BIT(data, 4))
		m_key_line = data & 0x0f;
	else
		m_speaker->level_w(BIT(data, 0));
}

uint8_t pp01_state::pp01_8255_portc_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return 0xff;
}
