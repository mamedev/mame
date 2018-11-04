// license:BSD-3-Clause
// copyright-holders:Peter Trauner
/***************************************************************************

    machine/pcshare.c

    Functions to emulate general aspects of the machine
    (RAM, ROM, interrupts, I/O ports)

    The information herein is heavily based on
    'Ralph Browns Interrupt List'
    Release 52, Last Change 20oct96

***************************************************************************/

#include "emu.h"
#include "machine/pcshare.h"
#include "cpu/i86/i286.h"
#include "bus/isa/trident.h"
#include "video/pc_vga.h"
#include "video/clgd542x.h"
#include "screen.h"

/******************
DMA8237 Controller
******************/

WRITE_LINE_MEMBER( pcat_base_state::pc_dma_hrq_changed )
{
	m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);

	/* Assert HLDA */
	m_dma8237_1->hack_w( state );
}


READ8_MEMBER(pcat_base_state::pc_dma_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space

	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16)
		& 0xFF0000;

	return prog_space.read_byte(page_offset + offset);
}


WRITE8_MEMBER(pcat_base_state::pc_dma_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16)
		& 0xFF0000;

	prog_space.write_byte(page_offset + offset, data);
}

READ8_MEMBER(pcat_base_state::dma_page_select_r)
{
	uint8_t data = m_at_pages[offset % 0x10];

	switch(offset % 8)
	{
	case 1:
		data = m_dma_offset[(offset / 8) & 1][2];
		break;
	case 2:
		data = m_dma_offset[(offset / 8) & 1][3];
		break;
	case 3:
		data = m_dma_offset[(offset / 8) & 1][1];
		break;
	case 7:
		data = m_dma_offset[(offset / 8) & 1][0];
		break;
	}
	return data;
}


WRITE8_MEMBER(pcat_base_state::dma_page_select_w)
{
	m_at_pages[offset % 0x10] = data;

	switch(offset % 8)
	{
	case 1:
		m_dma_offset[(offset / 8) & 1][2] = data;
		break;
	case 2:
		m_dma_offset[(offset / 8) & 1][3] = data;
		break;
	case 3:
		m_dma_offset[(offset / 8) & 1][1] = data;
		break;
	case 7:
		m_dma_offset[(offset / 8) & 1][0] = data;
		break;
	}
}

void pcat_base_state::set_dma_channel(int channel, int state)
{
	if (!state) m_dma_channel = channel;
}

WRITE_LINE_MEMBER( pcat_base_state::pc_dack0_w ) { set_dma_channel(0, state); }
WRITE_LINE_MEMBER( pcat_base_state::pc_dack1_w ) { set_dma_channel(1, state); }
WRITE_LINE_MEMBER( pcat_base_state::pc_dack2_w ) { set_dma_channel(2, state); }
WRITE_LINE_MEMBER( pcat_base_state::pc_dack3_w ) { set_dma_channel(3, state); }

/******************
8259 IRQ controller
******************/

READ8_MEMBER( pcat_base_state::get_slave_ack )
{
	if (offset==2) { // IRQ = 2
		return m_pic8259_2->acknowledge();
	}
	return 0x00;
}

WRITE_LINE_MEMBER( pcat_base_state::at_pit8254_out2_changed )
{
	m_pit_out2 = state;
	//at_speaker_set_input( state ? 1 : 0 );
	m_kbdc->write_out2(state);
}


ADDRESS_MAP_START(pcat_base_state::pcat32_io_common)
	AM_RANGE(0x0000, 0x001f) AM_DEVREADWRITE8("dma8237_1", am9517a_device, read, write, 0xffffffff)
	AM_RANGE(0x0020, 0x003f) AM_DEVREADWRITE8("pic8259_1", pic8259_device, read, write, 0xffffffff)
	AM_RANGE(0x0040, 0x005f) AM_DEVREADWRITE8("pit8254", pit8254_device, read, write, 0xffffffff)
	AM_RANGE(0x0060, 0x006f) AM_DEVREADWRITE8("kbdc", kbdc8042_device, data_r, data_w, 0xffffffff)
	AM_RANGE(0x0070, 0x007f) AM_DEVREADWRITE8("rtc", mc146818_device, read, write, 0xffffffff)
	AM_RANGE(0x0080, 0x009f) AM_READWRITE8(dma_page_select_r,dma_page_select_w, 0xffffffff)//TODO
	AM_RANGE(0x00a0, 0x00bf) AM_DEVREADWRITE8("pic8259_2", pic8259_device, read, write, 0xffffffff)
	AM_RANGE(0x00c0, 0x00df) AM_DEVREADWRITE8("dma8237_2", am9517a_device, read, write, 0x00ff00ff)
ADDRESS_MAP_END


MACHINE_CONFIG_START(pcat_base_state::pcvideo_vga)
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL(25'174'800),900,0,640,526,0,480)
	MCFG_SCREEN_UPDATE_DEVICE("vga", vga_device, screen_update)

	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_DEVICE_ADD("vga", VGA, 0)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(pcat_base_state::pcvideo_trident_vga)
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL(25'174'800),900,0,640,526,0,480)
	MCFG_SCREEN_UPDATE_DEVICE("vga", trident_vga_device, screen_update)

	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_DEVICE_ADD("vga", TRIDENT_VGA, 0)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(pcat_base_state::pcvideo_s3_vga)
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL(25'174'800),900,0,640,526,0,480)
	MCFG_SCREEN_UPDATE_DEVICE("vga", s3_vga_device, screen_update)

	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_DEVICE_ADD("vga", S3_VGA, 0)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(pcat_base_state::pcvideo_cirrus_gd5428)
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL(25'174'800),900,0,640,526,0,480)
	MCFG_SCREEN_UPDATE_DEVICE("vga", cirrus_gd5428_device, screen_update)

	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_DEVICE_ADD("vga", CIRRUS_GD5428, 0)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(pcat_base_state::pcvideo_cirrus_gd5430)
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL(25'174'800),900,0,640,526,0,480)
	MCFG_SCREEN_UPDATE_DEVICE("vga", cirrus_gd5430_device, screen_update)

	MCFG_PALETTE_ADD("palette", 0x100)
	MCFG_DEVICE_ADD("vga", CIRRUS_GD5430, 0)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(pcat_base_state::pcat_common)
	MCFG_DEVICE_ADD("pic8259_1", PIC8259, 0)
	MCFG_PIC8259_OUT_INT_CB(INPUTLINE("maincpu", 0))
	MCFG_PIC8259_IN_SP_CB(VCC)
	MCFG_PIC8259_CASCADE_ACK_CB(READ8(pcat_base_state, get_slave_ack))

	MCFG_DEVICE_ADD("pic8259_2", PIC8259, 0)
	MCFG_PIC8259_OUT_INT_CB(DEVWRITELINE("pic8259_1", pic8259_device, ir2_w))
	MCFG_PIC8259_IN_SP_CB(GND)

	MCFG_DEVICE_ADD( "dma8237_1", AM9517A, XTAL(14'318'181)/3 )
	MCFG_I8237_OUT_HREQ_CB(WRITELINE(pcat_base_state, pc_dma_hrq_changed))
	MCFG_I8237_IN_MEMR_CB(READ8(pcat_base_state, pc_dma_read_byte))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(pcat_base_state, pc_dma_write_byte))
	MCFG_I8237_OUT_DACK_0_CB(WRITELINE(pcat_base_state, pc_dack0_w))
	MCFG_I8237_OUT_DACK_1_CB(WRITELINE(pcat_base_state, pc_dack1_w))
	MCFG_I8237_OUT_DACK_2_CB(WRITELINE(pcat_base_state, pc_dack2_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(pcat_base_state, pc_dack3_w))
	MCFG_DEVICE_ADD( "dma8237_2", AM9517A, XTAL(14'318'181)/3 )

	MCFG_DEVICE_ADD("pit8254", PIT8254, 0)
	MCFG_PIT8253_CLK0(4772720/4) /* heartbeat IRQ */
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE("pic8259_1", pic8259_device, ir0_w))
	MCFG_PIT8253_CLK1(4772720/4) /* dram refresh */
	MCFG_PIT8253_CLK2(4772720/4) /* pio port c pin 4, and speaker polling enough */
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(pcat_base_state, at_pit8254_out2_changed))

	MCFG_MC146818_ADD("rtc", XTAL(32'768))
	MCFG_MC146818_IRQ_HANDLER(DEVWRITELINE("pic8259_2", pic8259_device, ir0_w))
	MCFG_MC146818_CENTURY_INDEX(0x32)

	MCFG_DEVICE_ADD("kbdc", KBDC8042, 0)
	MCFG_KBDC8042_KEYBOARD_TYPE(KBDC8042_AT386)
	MCFG_KBDC8042_SYSTEM_RESET_CB(INPUTLINE("maincpu", INPUT_LINE_RESET))
	MCFG_KBDC8042_GATE_A20_CB(INPUTLINE("maincpu", INPUT_LINE_A20))
	MCFG_KBDC8042_INPUT_BUFFER_FULL_CB(DEVWRITELINE("pic8259_1", pic8259_device, ir1_w))
MACHINE_CONFIG_END
