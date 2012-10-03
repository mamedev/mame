/***************************************************************************

        PP-01 driver by Miodrag Milanovic

        08/09/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "includes/pp01.h"
#include "machine/ram.h"


WRITE8_MEMBER(pp01_state::pp01_video_write_mode_w)
{
	m_video_write_mode = data & 0x0f;
}

static void pp01_video_w(running_machine &machine,UINT8 block,UINT16 offset,UINT8 data,UINT8 part)
{
	pp01_state *state = machine.driver_data<pp01_state>();
	UINT16 addroffset = part ? 0x1000  : 0x0000;
	UINT8 *ram = machine.device<ram_device>(RAM_TAG)->pointer();

	if (BIT(state->m_video_write_mode,3)) {
		// Copy mode
		if(BIT(state->m_video_write_mode,0)) {
			ram[0x6000+offset+addroffset] = data;
		} else {
			ram[0x6000+offset+addroffset] = 0;
		}
		if(BIT(state->m_video_write_mode,1)) {
			ram[0xa000+offset+addroffset] = data;
		} else {
			ram[0xa000+offset+addroffset] = 0;
		}
		if(BIT(state->m_video_write_mode,2)) {
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

WRITE8_MEMBER(pp01_state::pp01_video_r_1_w)
{
	pp01_video_w(machine(),0,offset,data,0);
}
WRITE8_MEMBER(pp01_state::pp01_video_g_1_w)
{
	pp01_video_w(machine(),1,offset,data,0);
}
WRITE8_MEMBER(pp01_state::pp01_video_b_1_w)
{
	pp01_video_w(machine(),2,offset,data,0);
}

WRITE8_MEMBER(pp01_state::pp01_video_r_2_w)
{
	pp01_video_w(machine(),0,offset,data,1);
}
WRITE8_MEMBER(pp01_state::pp01_video_g_2_w)
{
	pp01_video_w(machine(),1,offset,data,1);
}
WRITE8_MEMBER(pp01_state::pp01_video_b_2_w)
{
	pp01_video_w(machine(),2,offset,data,1);
}


static void pp01_set_memory(running_machine &machine,UINT8 block, UINT8 data)
{
	pp01_state *state = machine.driver_data<pp01_state>();
	UINT8 *mem = state->memregion("maincpu")->base();
	address_space &space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	UINT16 startaddr = block*0x1000;
	UINT16 endaddr   = ((block+1)*0x1000)-1;
	UINT8  blocknum  = block + 1;
	char bank[10];
	sprintf(bank,"bank%d",blocknum);
	if (data>=0xE0 && data<=0xEF) {
		// This is RAM
		space.install_read_bank (startaddr, endaddr, bank);
		switch(data) {
			case 0xe6 :
					space.install_write_handler(startaddr, endaddr, write8_delegate(FUNC(pp01_state::pp01_video_r_1_w),state));
					break;
			case 0xe7 :
					space.install_write_handler(startaddr, endaddr, write8_delegate(FUNC(pp01_state::pp01_video_r_2_w),state));
					break;
			case 0xea :
					space.install_write_handler(startaddr, endaddr, write8_delegate(FUNC(pp01_state::pp01_video_g_1_w),state));
					break;
			case 0xeb :
					space.install_write_handler(startaddr, endaddr, write8_delegate(FUNC(pp01_state::pp01_video_g_2_w),state));
					break;
			case 0xee :
					space.install_write_handler(startaddr, endaddr, write8_delegate(FUNC(pp01_state::pp01_video_b_1_w),state));
					break;
			case 0xef :
					space.install_write_handler(startaddr, endaddr, write8_delegate(FUNC(pp01_state::pp01_video_b_2_w),state));
					break;

			default :
					space.install_write_bank(startaddr, endaddr, bank);
					break;
		}

		state->membank(bank)->set_base(machine.device<ram_device>(RAM_TAG)->pointer() + (data & 0x0F)* 0x1000);
	} else if (data>=0xF8) {
		space.install_read_bank (startaddr, endaddr, bank);
		space.unmap_write(startaddr, endaddr);
		state->membank(bank)->set_base(mem + ((data & 0x0F)-8)* 0x1000+0x10000);
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
		pp01_set_memory(machine(), i, 0xff);
	}
}

WRITE8_MEMBER(pp01_state::pp01_mem_block_w)
{
	m_memory_block[offset] = data;
	pp01_set_memory(machine(), offset, data);
}

READ8_MEMBER(pp01_state::pp01_mem_block_r)
{
	return	m_memory_block[offset];
}

void pp01_state::machine_start()
{
}


WRITE_LINE_MEMBER(pp01_state::pp01_pit_out0)
{
}

WRITE_LINE_MEMBER(pp01_state::pp01_pit_out1)
{
}

const struct pit8253_config pp01_pit8253_intf =
{
	{
		{
			0,
			DEVCB_NULL,
			DEVCB_DRIVER_LINE_MEMBER(pp01_state,pp01_pit_out0)
		},
		{
			2000000,
			DEVCB_NULL,
			DEVCB_DRIVER_LINE_MEMBER(pp01_state,pp01_pit_out1)
		},
		{
			2000000,
			DEVCB_NULL,
			DEVCB_DEVICE_LINE("pit8253", pit8253_clk0_w)
		}
	}
};

READ8_MEMBER(pp01_state::pp01_8255_porta_r)
{
	return m_video_scroll;
}
WRITE8_MEMBER(pp01_state::pp01_8255_porta_w)
{
	m_video_scroll = data;
}

READ8_MEMBER(pp01_state::pp01_8255_portb_r)
{
	static const char *const keynames[] = {
		"LINE0", "LINE1", "LINE2", "LINE3", "LINE4", "LINE5", "LINE6", "LINE7",
		"LINE8", "LINE9", "LINEA", "LINEB", "LINEC", "LINED", "LINEE", "LINEF"
	};

	return (machine().root_device().ioport(keynames[m_key_line])->read() & 0x3F) | (machine().root_device().ioport("LINEALL")->read() & 0xC0);
}
WRITE8_MEMBER(pp01_state::pp01_8255_portb_w)
{
	//logerror("pp01_8255_portb_w %02x\n",data);

}

WRITE8_MEMBER(pp01_state::pp01_8255_portc_w)
{
	m_key_line = data & 0x0f;
}

READ8_MEMBER(pp01_state::pp01_8255_portc_r)
{
	return 0xff;
}



I8255A_INTERFACE( pp01_ppi8255_interface )
{
	DEVCB_DRIVER_MEMBER(pp01_state,pp01_8255_porta_r),
	DEVCB_DRIVER_MEMBER(pp01_state,pp01_8255_porta_w),
	DEVCB_DRIVER_MEMBER(pp01_state,pp01_8255_portb_r),
	DEVCB_DRIVER_MEMBER(pp01_state,pp01_8255_portb_w),
	DEVCB_DRIVER_MEMBER(pp01_state,pp01_8255_portc_r),
	DEVCB_DRIVER_MEMBER(pp01_state,pp01_8255_portc_w)
};

