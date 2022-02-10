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


uint8_t pcat_base_state::pc_dma_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space

	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16)
		& 0xFF0000;

	return prog_space.read_byte(page_offset + offset);
}


void pcat_base_state::pc_dma_write_byte(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM); // get the right address space
	offs_t page_offset = (((offs_t) m_dma_offset[0][m_dma_channel]) << 16)
		& 0xFF0000;

	prog_space.write_byte(page_offset + offset, data);
}

uint8_t pcat_base_state::dma_page_select_r(offs_t offset)
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


void pcat_base_state::dma_page_select_w(offs_t offset, uint8_t data)
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

uint8_t pcat_base_state::get_slave_ack(offs_t offset)
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


void pcat_base_state::pcat32_io_common(address_map &map)
{
	map(0x0000, 0x001f).rw(m_dma8237_1, FUNC(am9517a_device::read), FUNC(am9517a_device::write));
	map(0x0020, 0x003f).rw(m_pic8259_1, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x0040, 0x005f).rw(m_pit8254, FUNC(pit8254_device::read), FUNC(pit8254_device::write));
	map(0x0060, 0x006f).rw(m_kbdc, FUNC(kbdc8042_device::data_r), FUNC(kbdc8042_device::data_w));
	map(0x0070, 0x007f).rw(m_mc146818, FUNC(mc146818_device::read), FUNC(mc146818_device::write));
	map(0x0080, 0x009f).rw(FUNC(pcat_base_state::dma_page_select_r), FUNC(pcat_base_state::dma_page_select_w));//TODO
	map(0x00a0, 0x00bf).rw(m_pic8259_2, FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x00c0, 0x00df).rw(m_dma8237_2, FUNC(am9517a_device::read), FUNC(am9517a_device::write)).umask32(0x00ff00ff);
}


void pcat_base_state::pcvideo_vga(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.1748_MHz_XTAL, 900, 0, 640, 526, 0, 480);
	screen.set_screen_update("vga", FUNC(vga_device::screen_update));

	vga_device &vga(VGA(config, "vga", 0));
	vga.set_screen("screen");
	vga.set_vram_size(0x100000);
}

void pcat_base_state::pcvideo_trident_vga(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.1748_MHz_XTAL, 900, 0, 640, 526, 0, 480);
	screen.set_screen_update("vga", FUNC(trident_vga_device::screen_update));

	trident_vga_device &vga(TRIDENT_VGA(config, "vga", 0));
	vga.set_screen("screen");
	vga.set_vram_size(0x200000);
}

void pcat_base_state::pcvideo_s3_vga(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.1748_MHz_XTAL, 900, 0, 640, 526, 0, 480);
	screen.set_screen_update("vga", FUNC(s3_vga_device::screen_update));

	s3_vga_device &vga(S3_VGA(config, "vga", 0));
	vga.set_screen("screen");
	vga.set_vram_size(0x100000);
}


void pcat_base_state::pcvideo_cirrus_gd5428(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.1748_MHz_XTAL, 900, 0, 640, 526, 0, 480);
	screen.set_screen_update("vga", FUNC(cirrus_gd5428_device::screen_update));

	cirrus_gd5428_device &vga(CIRRUS_GD5428(config, "vga", 0));
	vga.set_screen("screen");
	vga.set_vram_size(0x200000);

}

void pcat_base_state::pcvideo_cirrus_gd5430(machine_config &config)
{
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.1748_MHz_XTAL, 900, 0, 640, 526, 0, 480);
	screen.set_screen_update("vga", FUNC(cirrus_gd5430_device::screen_update));

	cirrus_gd5430_device &vga(CIRRUS_GD5430(config, "vga", 0));
	vga.set_screen("screen");
	vga.set_vram_size(0x200000);

}

void pcat_base_state::pcat_common(machine_config &config)
{
	PIC8259(config, m_pic8259_1, 0);
	m_pic8259_1->out_int_callback().set_inputline(m_maincpu, 0);
	m_pic8259_1->in_sp_callback().set_constant(1);
	m_pic8259_1->read_slave_ack_callback().set(FUNC(pcat_base_state::get_slave_ack));

	PIC8259(config, m_pic8259_2, 0);
	m_pic8259_2->out_int_callback().set(m_pic8259_1, FUNC(pic8259_device::ir2_w));
	m_pic8259_2->in_sp_callback().set_constant(0);

	AM9517A(config, m_dma8237_1, 14.318181_MHz_XTAL / 3);
	m_dma8237_1->out_hreq_callback().set(FUNC(pcat_base_state::pc_dma_hrq_changed));
	m_dma8237_1->in_memr_callback().set(FUNC(pcat_base_state::pc_dma_read_byte));
	m_dma8237_1->out_memw_callback().set(FUNC(pcat_base_state::pc_dma_write_byte));
	m_dma8237_1->out_dack_callback<0>().set(FUNC(pcat_base_state::pc_dack0_w));
	m_dma8237_1->out_dack_callback<1>().set(FUNC(pcat_base_state::pc_dack1_w));
	m_dma8237_1->out_dack_callback<2>().set(FUNC(pcat_base_state::pc_dack2_w));
	m_dma8237_1->out_dack_callback<3>().set(FUNC(pcat_base_state::pc_dack3_w));

	AM9517A(config, m_dma8237_2, 14.318181_MHz_XTAL / 3);

	PIT8254(config, m_pit8254, 0);
	m_pit8254->set_clk<0>(4772720/4); // heartbeat IRQ
	m_pit8254->out_handler<0>().set(m_pic8259_1, FUNC(pic8259_device::ir0_w));
	m_pit8254->set_clk<1>(4772720/4); // DRAM refresh
	m_pit8254->set_clk<2>(4772720/4); // PIO port C pin 4, and speaker polling enough
	m_pit8254->out_handler<2>().set(FUNC(pcat_base_state::at_pit8254_out2_changed));

	MC146818(config, m_mc146818, 32.768_kHz_XTAL);
	m_mc146818->irq().set(m_pic8259_2, FUNC(pic8259_device::ir0_w));
	m_mc146818->set_century_index(0x32);

	KBDC8042(config, m_kbdc, 0);
	m_kbdc->set_keyboard_type(kbdc8042_device::KBDC8042_STANDARD);
	m_kbdc->system_reset_callback().set_inputline(m_maincpu, INPUT_LINE_RESET);
	m_kbdc->gate_a20_callback().set_inputline(m_maincpu, INPUT_LINE_A20);
	m_kbdc->input_buffer_full_callback().set(m_pic8259_1, FUNC(pic8259_device::ir1_w));
}
