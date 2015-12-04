// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*

    Convergent NGen series

    10-11-14 - Skeleton driver

    Interrupts based on patents:
    level 1 - SIO
    level 3 - timer (from PIT, presumably channel 0? Patent says "channel 3")
    level 4 - "interrupt detector" - keyboard, printer, RTC
    level 7 - floppy/hard disk

    DMA channels:
    channel 0 - communications (RS-232)
    channel 1 - X-Bus expansion modules (except disk and graphics)
    channel 2 - graphics?
    channel 3 - hard disk
    On the CP-001/B26 channels 4 on are handled by the 80186.
    channel 4 - floppy disk

    To get to "menu mode", press Space quickly after reset (might need good timing)
    The bootstrap ROM version number is displayed, along with "B,D,L,M,P,T:"
    You can press one of these keys for the following tests:
    B: Bootstrap
       Loads the system image file (from disk or master workstation)
    D: Dump
       RAM contents are dumped to a local disk drive or master workstation
    L: Load
       Loads the system image file, then enters the Panel Debugger.  Exiting the Panel
       Debugger will continue execution of the system image
    M: Memory Test
       Continuously performs the Memory Test until the system is reset.
    P: Panel Debugger
       Enters the Panel Debugger
    T: Type of Operating System
       Gives an "OS:" prompt, at which you can enter the number of the system image to
       load at the master workstation.

    Panel Debugger:
    - Open/Modify RAM
    Enter an address (seg:off) followed by a forward-slash, the contents of this word will
    appear, you can enter a value to set it to, or just press Next (default: Enter) to leave
    it as is.  It will then go on to the next word.  Pressing Return (scan code unknown
    currently) will return to the debugger prompt.
    - Open/Modify Register
    Enter the register only, and the contents will appear, you can leave it or alter it (you
    must enter all digits (eg: 0A03 if you're modifying DX) then press Return.
    - I/O to or from a port
    Input: Address (segment is ignored, and not required) followed by I, a byte is read from
    the port defined by the offset, and the byte is displayed.
    Output: Address followed by O, you are now prompted with an '='.  Enter the byte to send
    to the port, and press Return.
    - Set Haltpoint:
    Enter an address (seg:off) followed by H.  Sets a haltpoint at the specified address.  Does
    not work for ROM addresses.  Only one allowed at a time.  Haltpoint info is stored at
    0000:01F0.  Uses a software interrupt (INT 7C), rather than INT 3.

    To start or continue from the current address, enter P.
    To start from a specific address, enter the address (seg:off) followed by a G.
*/

#include "emu.h"
#include "cpu/i86/i186.h"
#include "cpu/i386/i386.h"
#include "video/mc6845.h"
#include "machine/i8251.h"
#include "machine/am9517a.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/z80dart.h"
#include "machine/wd_fdc.h"
#include "machine/wd2010.h"
#include "bus/rs232/rs232.h"
#include "machine/ngen_kb.h"
#include "machine/clock.h"
#include "imagedev/harddriv.h"

class ngen_state : public driver_device
{
public:
	ngen_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_i386cpu(*this,"i386cpu"),
		m_crtc(*this,"crtc"),
		m_viduart(*this,"videouart"),
		m_iouart(*this,"iouart"),
		m_dmac(*this,"dmac"),
		m_pic(*this,"pic"),
		m_pit(*this,"pit"),
		m_disk_rom(*this,"disk"),
		m_fdc(*this,"fdc"),
		m_fd0(*this,"fdc:0"),
		m_fdc_timer(*this,"fdc_timer"),
		m_hdc(*this,"hdc"),
		m_hdc_timer(*this,"hdc_timer"),
		m_hd_buffer(*this,"hd_buffer_ram")
	{}

	DECLARE_WRITE_LINE_MEMBER(pit_out0_w);
	DECLARE_WRITE_LINE_MEMBER(pit_out1_w);
	DECLARE_WRITE_LINE_MEMBER(pit_out2_w);
	DECLARE_WRITE_LINE_MEMBER(cpu_timer_w);
	DECLARE_WRITE_LINE_MEMBER(timer_clk_out);
	DECLARE_WRITE16_MEMBER(cpu_peripheral_cb);
	DECLARE_WRITE16_MEMBER(peripheral_w);
	DECLARE_READ16_MEMBER(peripheral_r);
	DECLARE_WRITE16_MEMBER(xbus_w);
	DECLARE_READ16_MEMBER(xbus_r);
	DECLARE_WRITE_LINE_MEMBER(dma_hrq_changed);
	DECLARE_WRITE_LINE_MEMBER(dma_eop_changed);
	DECLARE_WRITE_LINE_MEMBER(dack0_w);
	DECLARE_WRITE_LINE_MEMBER(dack1_w);
	DECLARE_WRITE_LINE_MEMBER(dack2_w);
	DECLARE_WRITE_LINE_MEMBER(dack3_w);
	DECLARE_READ8_MEMBER(dma_read_word);
	DECLARE_WRITE8_MEMBER(dma_write_word);
	MC6845_UPDATE_ROW(crtc_update_row);
	// TODO: sort out what devices use which channels
	DECLARE_READ8_MEMBER( dma_0_dack_r ) { UINT16 ret = 0xffff; m_dma_high_byte = ret & 0xff00; return ret; }
	DECLARE_READ8_MEMBER( dma_1_dack_r ) { UINT16 ret = 0xffff; m_dma_high_byte = ret & 0xff00; return ret; }
	DECLARE_READ8_MEMBER( dma_2_dack_r ) { UINT16 ret = 0xffff; m_dma_high_byte = ret & 0xff00; return ret; }
	DECLARE_READ8_MEMBER( dma_3_dack_r );
	DECLARE_WRITE8_MEMBER( dma_0_dack_w ){ popmessage("IOW0: data %02x",data); }
	DECLARE_WRITE8_MEMBER( dma_1_dack_w ){  }
	DECLARE_WRITE8_MEMBER( dma_2_dack_w ){  }
	DECLARE_WRITE8_MEMBER( dma_3_dack_w ){ popmessage("IOW3: data %02x",data); }

	DECLARE_WRITE16_MEMBER(hfd_w);
	DECLARE_READ16_MEMBER(hfd_r);
	DECLARE_WRITE_LINE_MEMBER(fdc_irq_w);
	DECLARE_WRITE_LINE_MEMBER(fdc_drq_w);
	DECLARE_WRITE8_MEMBER(fdc_control_w);
	DECLARE_READ8_MEMBER(irq_cb);
	DECLARE_WRITE8_MEMBER(hdc_control_w);
	DECLARE_WRITE8_MEMBER(disk_addr_ext);
	DECLARE_READ8_MEMBER(hd_buffer_r);
	DECLARE_WRITE8_MEMBER(hd_buffer_w);

	DECLARE_READ16_MEMBER(b38_keyboard_r);
	DECLARE_WRITE16_MEMBER(b38_keyboard_w);
	DECLARE_READ16_MEMBER(b38_crtc_r);
	DECLARE_WRITE16_MEMBER(b38_crtc_w);
protected:
	virtual void machine_reset();
	virtual void machine_start();

private:
	optional_device<i80186_cpu_device> m_maincpu;
	optional_device<i386_device> m_i386cpu;
	required_device<mc6845_device> m_crtc;
	required_device<i8251_device> m_viduart;
	required_device<upd7201_device> m_iouart;
	required_device<am9517a_device> m_dmac;
	required_device<pic8259_device> m_pic;
	required_device<pit8254_device> m_pit;
	optional_memory_region m_disk_rom;
	memory_array m_vram;
	memory_array m_fontram;
	optional_device<wd2797_t> m_fdc;
	optional_device<floppy_connector> m_fd0;
	optional_device<pit8253_device> m_fdc_timer;
	optional_device<wd2010_device> m_hdc;
	optional_device<pit8253_device> m_hdc_timer;
	optional_shared_ptr<UINT8> m_hd_buffer;

	void set_dma_channel(int channel, int state);

	UINT8 m_xbus_current;  // currently selected X-Bus module
	UINT16 m_peripheral;
	UINT16 m_upper;
	UINT16 m_middle;
	UINT16 m_port00;
	UINT16 m_periph141;
	UINT8 m_dma_offset[4];
	INT8 m_dma_channel;
	UINT16 m_dma_high_byte;
	UINT16 m_control;
	UINT16 m_disk_rom_ptr;
	UINT8 m_hdc_control;
	UINT8 m_disk_page;
};

class ngen386_state : public ngen_state
{
public:
	ngen386_state(const machine_config &mconfig, device_type type, const char *tag)
		: ngen_state(mconfig, type, tag)
		{}
private:
};

WRITE_LINE_MEMBER(ngen_state::pit_out0_w)
{
	m_pic->ir3_w(state);  // Timer interrupt
	popmessage("PIT Timer 0 state %i\n",state);
}

WRITE_LINE_MEMBER(ngen_state::pit_out1_w)
{
	popmessage("PIT Timer 1 state %i\n",state);
	m_iouart->rxcb_w(state);
	m_iouart->txcb_w(state);  // channels in the correct order?
}

WRITE_LINE_MEMBER(ngen_state::pit_out2_w)
{
	m_iouart->rxca_w(state);
	m_iouart->txca_w(state);
	popmessage("PIT Timer 2 state %i\n",state);
}

WRITE_LINE_MEMBER(ngen_state::cpu_timer_w)
{
	if(state != 0)
		popmessage("80186 Timer 0 state %i\n",state);
	m_pic->ir5_w(state);
}

WRITE_LINE_MEMBER(ngen_state::timer_clk_out)
{
	m_viduart->write_rxc(state);  // Keyboard UART Rx/Tx clocks
	m_viduart->write_txc(state);
	// 80186 timer pins also?  EXT bit is enabled for BTOS PIT test.
	if(m_maincpu)
	{
		m_maincpu->tmrin0_w(state);
		//m_maincpu->tmrin1_w(state);
	}
}

WRITE16_MEMBER(ngen_state::cpu_peripheral_cb)
{
	UINT32 addr;

	switch(offset)
	{
	case 0:  // upper memory
		m_upper = data;
		break;
	case 2:  // peripheral
		m_peripheral = data;
		addr = (m_peripheral & 0xffc0) << 4;
		if(m_middle & 0x0040)
		{
			m_maincpu->device_t::memory().space(AS_PROGRAM).install_readwrite_handler(addr, addr + 0x3ff, read16_delegate(FUNC(ngen_state::peripheral_r), this), write16_delegate(FUNC(ngen_state::peripheral_w), this));
			logerror("Mapped peripherals to memory 0x%08x\n",addr);
		}
		else
		{
			addr &= 0xffff;
			m_maincpu->device_t::memory().space(AS_IO).install_readwrite_handler(addr, addr + 0x3ff, read16_delegate(FUNC(ngen_state::peripheral_r), this), write16_delegate(FUNC(ngen_state::peripheral_w), this));
			logerror("Mapped peripherals to I/O 0x%04x\n",addr);
		}
		break;
	case 4:
		m_middle = data;
		break;
	}
}

// 80186 peripheral space
// Largely guesswork at this stage
WRITE16_MEMBER(ngen_state::peripheral_w)
{
	switch(offset)
	{
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case 0x09:
	case 0x0a:
	case 0x0b:
	case 0x0c:
	case 0x0d:
	case 0x0e:
	case 0x0f:
		if(mem_mask & 0x00ff)
			m_dmac->write(space,offset,data & 0xff);
		break;
	case 0x80: // DMA page offset?
	case 0x81:
	case 0x82:
	case 0x83:
		if(mem_mask & 0x00ff)
			m_dma_offset[offset-0x80] = data & 0xff;
		break;
	case 0xc0:  // X-Bus modules reset
		m_xbus_current = 0;
		break;
	case 0x10c:
		if(mem_mask & 0x00ff)
			m_pic->write(space,0,data & 0xff);
		break;
	case 0x10d:
		if(mem_mask & 0x00ff)
			m_pic->write(space,1,data & 0xff);
		break;
	case 0x110:
	case 0x111:
	case 0x112:
	case 0x113:
		if(mem_mask & 0x00ff)
			m_pit->write(space,offset-0x110,data & 0xff);
		break;
	case 0x141:
		// bit 1 enables speaker?
		COMBINE_DATA(&m_periph141);
		break;
	case 0x144:
		if(mem_mask & 0x00ff)
			m_crtc->address_w(space,0,data & 0xff);
		break;
	case 0x145:
		if(mem_mask & 0x00ff)
			m_crtc->register_w(space,0,data & 0xff);
		break;
	case 0x146:
		if(mem_mask & 0x00ff)
			m_viduart->data_w(space,0,data & 0xff);
		break;
	case 0x147:
		if(mem_mask & 0x00ff)
			m_viduart->control_w(space,0,data & 0xff);
		break;
	case 0x1a0:  // serial?
		logerror("Serial(?) 0x1a0 write offset %04x data %04x mask %04x\n",offset,data,mem_mask);
		break;
	default:
		logerror("Unknown 80186 peripheral write offset %04x data %04x mask %04x\n",offset,data,mem_mask);
	}
}

READ16_MEMBER(ngen_state::peripheral_r)
{
	UINT16 ret = 0xffff;
	switch(offset)
	{
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x07:
	case 0x08:
	case 0x09:
	case 0x0a:
	case 0x0b:
	case 0x0c:
	case 0x0d:
	case 0x0e:
	case 0x0f:
		if(mem_mask & 0x00ff)
			ret = m_dmac->read(space,offset);
		logerror("DMA read offset %04x mask %04x returning %04x\n",offset,mem_mask,ret);
		break;
	case 0x80: // DMA page offset?
	case 0x81:
	case 0x82:
	case 0x83:
		if(mem_mask & 0x00ff)
			ret = m_dma_offset[offset-0x80] & 0xff;
		break;
	case 0x10c:
		if(mem_mask & 0x00ff)
			ret = m_pic->read(space,0);
		break;
	case 0x10d:
		if(mem_mask & 0x00ff)
			ret = m_pic->read(space,1);
		break;
	case 0x110:
	case 0x111:
	case 0x112:
	case 0x113:
		if(mem_mask & 0x00ff)
			ret = m_pit->read(space,offset-0x110);
		break;
	case 0x141:
		ret = m_periph141;
		break;
	case 0x144:
		if(mem_mask & 0x00ff)
			ret = m_crtc->status_r(space,0);
		break;
	case 0x145:
		if(mem_mask & 0x00ff)
			ret = m_crtc->register_r(space,0);
		break;
	case 0x146:
		if(mem_mask & 0x00ff)
			ret = m_viduart->data_r(space,0);
		break;
	case 0x147:  // keyboard UART
		// expects bit 0 to be set (UART transmit ready)
		if(mem_mask & 0x00ff)
			ret = m_viduart->status_r(space,0);
		break;
	case 0x1a0:  // I/O control register?
		ret = m_control;  // end of DMA transfer? (maybe a per-channel EOP?) Bit 6 is set during a transfer?
		break;
//  default:
//      logerror("Unknown 80186 peripheral read offset %04x mask %04x returning %04x\n",offset,mem_mask,ret);
	}
	return ret;
}

// X-bus module select
// The bootstrap ROM creates a table at 0:FC9h, with a count, followed by the module IDs of each
// expansion module.  The base I/O address for the currently selected module is set by writing to
// this register (bits 0-7 are ignored)
// TODO: make expansion modules slot devices
WRITE16_MEMBER(ngen_state::xbus_w)
{
	UINT16 addr = (data & 0x00ff) << 8;
	cpu_device* cpu;

	if(m_maincpu)
		cpu = m_maincpu;
	else
		cpu = m_i386cpu;
	address_space& io = cpu->device_t::memory().space(AS_IO);
	switch(m_xbus_current)
	{
		case 0x00:  // Floppy/Hard disk module
			io.install_readwrite_handler(addr,addr+0xff,read16_delegate(FUNC(ngen_state::hfd_r),this),write16_delegate(FUNC(ngen_state::hfd_w),this),0xffffffff);
			break;
		default:
			cpu->set_input_line(INPUT_LINE_NMI,PULSE_LINE);  // reached end of the modules
			break;
	}
	if(addr != 0)
		logerror("SYS: X-Bus module %i address set %04x\n",m_xbus_current+1,addr);
	m_xbus_current++;
}

// returns X-bus module ID and info in the low byte (can indicate if the device is bootable, has a boot ROM (needs to be written to RAM via DMA), or if it supports a non-80186 CPU)
// bit 6, I think, indicates a bootable device
// Known module IDs:
//  0x1070 - Floppy/Hard disk module
//  0x3141 - QIC Tape module
READ16_MEMBER(ngen_state::xbus_r)
{
	UINT16 ret = 0xffff;

	switch(m_xbus_current)
	{
		case 0x00:
			ret = 0x1070;  // Floppy/Hard disk module
			break;
		default:
			if(m_maincpu)
				m_maincpu->set_input_line(INPUT_LINE_NMI,PULSE_LINE);  // reached the end of the modules
			else
				m_i386cpu->set_input_line(INPUT_LINE_NMI,PULSE_LINE);
			ret = 0x0080;
			break;
	}
	return ret;
}


// Floppy/Hard disk module
WRITE16_MEMBER(ngen_state::hfd_w)
{
	switch(offset)
	{
		case 0x00:
		case 0x01:
		case 0x02:
			if(mem_mask & 0x00ff)
				m_fdc->write(space,offset,data & 0xff);
			break;
		case 0x03:
			if(mem_mask & 0x00ff)
			{
				m_fdc->write(space,offset,data & 0xff);
				m_fdc_timer->write_clk0(1);
				m_fdc_timer->write_clk0(0);  // Data register access clocks the FDC's PIT channel 0
			}
			break;
		case 0x04:
			if(mem_mask & 0x00ff)
				fdc_control_w(space,0,data & 0xff);
			break;
		case 0x05:
			if(mem_mask & 0x00ff)
				hdc_control_w(space,0,data & 0xff);
			break;
		case 0x07:
			if(mem_mask & 0x00ff)
				disk_addr_ext(space,0,data & 0xff);
			break;
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
			if(mem_mask & 0x00ff)
				m_fdc_timer->write(space,offset-0x08,data & 0xff);
			break;
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
			if(mem_mask & 0x00ff)
				m_hdc->write(space,offset-0x10,data & 0xff);
			logerror("WD1010 register %i write %02x mask %04x\n",offset-0x10,data & 0xff,mem_mask);
			break;
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
			if(mem_mask & 0x00ff)
				m_hdc_timer->write(space,offset-0x18,data & 0xff);
			break;
	}
}

READ16_MEMBER(ngen_state::hfd_r)
{
	UINT16 ret = 0xffff;

	switch(offset)
	{
		case 0x00:
		case 0x01:
		case 0x02:
			if(mem_mask & 0x00ff)
				ret = m_fdc->read(space,offset);
			break;
		case 0x03:
			if(mem_mask & 0x00ff)
			{
				ret = m_fdc->read(space,offset);
				m_fdc_timer->write_clk0(1);
				m_fdc_timer->write_clk0(0);  // Data register access clocks the FDC's PIT channel 0
			}
			break;
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
			if(mem_mask & 0x00ff)
				ret = m_fdc_timer->read(space,offset-0x08);
			break;
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
			if(mem_mask & 0x00ff)
				ret = m_hdc->read(space,offset-0x10);
			logerror("WD1010 register %i read, mask %04x\n",offset-0x10,mem_mask);
			break;
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
			if(mem_mask & 0x00ff)
				ret = m_hdc_timer->read(space,offset-0x18);
			break;
	}

	return ret;
}

WRITE_LINE_MEMBER(ngen_state::fdc_irq_w)
{
	m_pic->ir7_w(state);
}

WRITE_LINE_MEMBER(ngen_state::fdc_drq_w)
{
	m_dmac->dreq3_w(state);
}

// Floppy disk control register
// Bit 0 - enable drive and LED
// Bit 2 - floppy motor
// Bit 5 - side select
// Bit 6 - 1 = 2Mhz for seek, 0 = 1MHz for read/write
// Bit 7 - FDC reset
WRITE8_MEMBER(ngen_state::fdc_control_w)
{
	m_fdc->set_floppy(m_fd0->get_device());
	m_fd0->get_device()->mon_w(~data & 0x04);
	m_fd0->get_device()->ss_w(data & 0x20);
	if(~data & 0x80)
		m_fdc->soft_reset();
}

// Hard disk control register
// bit 0 - Drive select 0 - selects module hard disk
// bit 1 - Drive select 1 - selects expansion module hard disk (if available)
// bit 2 - enable DMA transfer of module ROM contents to X-Bus master memory
// bits 3-5 - select head / expansion module head
// bit 6 - write enable, must be set to write to a hard disk
// bit 7 - HDC reset
WRITE8_MEMBER(ngen_state::hdc_control_w)
{
	m_hdc_control = data;
	if(m_hdc_control & 0x04)
	{
		m_disk_rom_ptr = 0;
		popmessage("HDD: DMA ROM transfer start\n");
		m_dmac->dreq3_w(1);
		//m_dmac->dreq3_w(0);
	}
}

// page of system RAM to access
// bit 7 = disables read/write signals to the WD1010
WRITE8_MEMBER(ngen_state::disk_addr_ext)
{
	m_disk_page = data & 0x7f;
}

READ8_MEMBER(ngen_state::hd_buffer_r)
{
	return m_hd_buffer[offset];
}

WRITE8_MEMBER(ngen_state::hd_buffer_w)
{
	m_hd_buffer[offset] = data;
}

WRITE_LINE_MEMBER( ngen_state::dma_hrq_changed )
{
	if(m_maincpu)
		m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);
	else
		m_i386cpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER( ngen_state::dma_eop_changed )
{
	if(m_dma_channel == 0)
	{
		if(state)
			m_control |= 0x02;
		else
			m_control &= ~0x02;
	}
	if(m_dma_channel == 3)
	{
		if(state)
		{
			if(m_hdc_control & 0x04) // ROM transfer
				m_hdc_control &= ~0x04;  // switch it off when done
		}
	}
}

void ngen_state::set_dma_channel(int channel, int state)
{
	if(!state)
		m_dma_channel = channel;
	else if(m_dma_channel == channel)
		m_dma_channel = -1;
}

WRITE_LINE_MEMBER( ngen_state::dack0_w ) { set_dma_channel(0, state); }
WRITE_LINE_MEMBER( ngen_state::dack1_w ) { set_dma_channel(1, state); }
WRITE_LINE_MEMBER( ngen_state::dack2_w ) { set_dma_channel(2, state); }
WRITE_LINE_MEMBER( ngen_state::dack3_w ) { set_dma_channel(3, state); }

READ8_MEMBER(ngen_state::dma_3_dack_r)
{
	UINT16 ret = 0xffff;

	if((m_hdc_control & 0x04) && m_disk_rom)
	{
		ret = m_disk_rom->base()[m_disk_rom_ptr++] << 8;
		printf("DMA3 DACK: returning %02x\n",ret);
		if(m_disk_rom_ptr < 0x1000)
		{
			m_dmac->dreq3_w(1);
			//m_dmac->dreq3_w(0);
		}
	}
	m_dma_high_byte = ret & 0xff00;
	return ret;
}

READ8_MEMBER(ngen_state::dma_read_word)
{
	cpu_device* cpu;
	UINT16 result;

	if(m_maincpu)
		cpu = m_maincpu;
	else
		cpu = m_i386cpu;
	address_space& prog_space = cpu->space(AS_PROGRAM); // get the right address space

	if(m_dma_channel == -1)
		return 0xff;
	offs_t page_offset = (((offs_t) m_dma_offset[m_dma_channel]) << 16) & 0xFE0000;

	result = prog_space.read_word(page_offset + (offset << 1));
	m_dma_high_byte = result & 0xFF00;
	popmessage("DMA byte address %06x read %04x\n",page_offset+(offset<<1),result);
	return result & 0xff;
}


WRITE8_MEMBER(ngen_state::dma_write_word)
{
	cpu_device* cpu;

	if(m_maincpu)
		cpu = m_maincpu;
	else
		cpu = m_i386cpu;
	address_space& prog_space = cpu->space(AS_PROGRAM); // get the right address space

	if(m_dma_channel == -1)
		return;
	offs_t page_offset = (((offs_t) m_dma_offset[m_dma_channel]) << 16) & 0xFE0000;

	prog_space.write_word(page_offset + (offset << 1), data);
	popmessage("DMA byte address %06x write %04x\n",page_offset+(offset<<1), m_dma_high_byte | data);
}


MC6845_UPDATE_ROW( ngen_state::crtc_update_row )
{
	UINT16 addr = ma;

	for(int x=0;x<bitmap.width();x+=9)
	{
		UINT8 ch = m_vram.read16(addr++) & 0xff;
		for(int z=0;z<9;z++)
		{
			if(BIT(m_fontram.read16(ch*16+ra),8-z))
				bitmap.pix32(y,x+z) = rgb_t(0,0xff,0);
			else
				bitmap.pix32(y,x+z) = rgb_t(0,0,0);
		}
	}
}

READ8_MEMBER( ngen_state::irq_cb )
{
	return m_pic->acknowledge();
}

READ16_MEMBER( ngen_state::b38_keyboard_r )
{
	UINT8 ret = 0;
	switch(offset)
	{
	case 0:
		if(mem_mask & 0x00ff)
			ret = m_viduart->data_r(space,0);
		break;
	case 1:  // keyboard UART
		// expects bit 0 to be set (UART transmit ready)
		if(mem_mask & 0x00ff)
			ret = m_viduart->status_r(space,0);
		break;
	}
	return ret;
}

WRITE16_MEMBER( ngen_state::b38_keyboard_w )
{
	switch(offset)
	{
	case 0:
		if(mem_mask & 0x00ff)
			m_viduart->data_w(space,0,data & 0xff);
		break;
	case 1:
		if(mem_mask & 0x00ff)
			m_viduart->control_w(space,0,data & 0xff);
		break;
	}
}

READ16_MEMBER( ngen_state::b38_crtc_r )
{
	UINT8 ret = 0;
	switch(offset)
	{
	case 0:
		if(mem_mask & 0x00ff)
			ret = m_crtc->register_r(space,0);
		break;
	case 1:
		if(mem_mask & 0x00ff)
			ret = m_viduart->data_r(space,0);
		break;
	}
	return ret;
}

WRITE16_MEMBER( ngen_state::b38_crtc_w )
{
	switch(offset)
	{
	case 0:
		if(mem_mask & 0x00ff)
			m_crtc->address_w(space,0,data & 0xff);
		break;
	case 1:
		if(mem_mask & 0x00ff)
			m_crtc->register_w(space,0,data & 0xff);
		break;
	}
}

void ngen_state::machine_start()
{
	memory_share* vidshare = memshare("vram");
	memory_share* fontshare = memshare("fontram");
	m_hd_buffer.allocate(1024*8);  // 8kB buffer RAM for HD controller
	if(vidshare == nullptr || fontshare == nullptr)
		fatalerror("Error: VRAM not found");
	m_vram.set(*vidshare,2);
	m_fontram.set(*fontshare,2);
}

void ngen_state::machine_reset()
{
	m_port00 = 0;
	m_control = 0;
	m_xbus_current = 0;
	m_viduart->write_dsr(0);
	m_viduart->write_cts(0);
	m_fd0->get_device()->set_rpm(300);
}

// boot ROMs from modules are not mapped anywhere, instead, they have to send the code from the boot ROM via DMA
static ADDRESS_MAP_START( ngen_mem, AS_PROGRAM, 16, ngen_state )
	AM_RANGE(0x00000, 0xf7fff) AM_RAM
	AM_RANGE(0xf8000, 0xf9fff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0xfa000, 0xfbfff) AM_RAM AM_SHARE("fontram")
	AM_RANGE(0xfc000, 0xfcfff) AM_RAM
	AM_RANGE(0xfe000, 0xfffff) AM_ROM AM_REGION("bios",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ngen_io, AS_IO, 16, ngen_state )
	AM_RANGE(0x0000, 0x0001) AM_READWRITE(xbus_r,xbus_w)

	// Floppy/Hard disk module
//  AM_RANGE(0x0100, 0x0107) AM_DEVREADWRITE8("fdc",wd2797_t,read,write,0x00ff)  // a guess for now
//  AM_RANGE(0x0108, 0x0109) AM_WRITE8(fdc_control_w,0x00ff)
//  AM_RANGE(0x010a, 0x010b) AM_WRITE8(hdc_control_w,0x00ff)
//  AM_RANGE(0x010e, 0x010f) AM_WRITE8(disk_addr_ext,0x00ff)  // X-Bus extended address register
//  AM_RANGE(0x0110, 0x0117) AM_DEVREADWRITE8("fdc_timer",pit8253_device,read,write,0x00ff)
	// 0x0120-0x012f - WD1010 Winchester disk controller (unemulated)
//  AM_RANGE(0x0130, 0x0137) AM_DEVREADWRITE8("hdc_timer",pit8253_device,read,write,0x00ff)

ADDRESS_MAP_END

static ADDRESS_MAP_START( ngen386_mem, AS_PROGRAM, 32, ngen_state )
	AM_RANGE(0x00000000, 0x000f7fff) AM_RAM
	AM_RANGE(0x000f8000, 0x000f9fff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0x000fa000, 0x000fbfff) AM_RAM AM_SHARE("fontram")
	AM_RANGE(0x000fc000, 0x000fcfff) AM_RAM
	AM_RANGE(0x000fe000, 0x000fffff) AM_ROM AM_REGION("bios",0)
	AM_RANGE(0x00100000, 0x00ffffff) AM_RAM  // some extra RAM
	AM_RANGE(0xffffe000, 0xffffffff) AM_ROM AM_REGION("bios",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ngen386i_mem, AS_PROGRAM, 32, ngen_state )
	AM_RANGE(0x00000000, 0x000f7fff) AM_RAM
	AM_RANGE(0x000f8000, 0x000f9fff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0x000fa000, 0x000fbfff) AM_RAM AM_SHARE("fontram")
	AM_RANGE(0x000fc000, 0x000fffff) AM_ROM AM_REGION("bios",0)
	AM_RANGE(0x00100000, 0x00ffffff) AM_RAM  // some extra RAM
	AM_RANGE(0xffffc000, 0xffffffff) AM_ROM AM_REGION("bios",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ngen386_io, AS_IO, 32, ngen_state )
	AM_RANGE(0x0000, 0x0003) AM_READWRITE16(xbus_r, xbus_w, 0x0000ffff)
//  AM_RANGE(0xf800, 0xfeff) AM_READWRITE16(peripheral_r, peripheral_w,0xffffffff)
	AM_RANGE(0xfd08, 0xfd0b) AM_READWRITE16(b38_crtc_r, b38_crtc_w,0xffffffff)
	AM_RANGE(0xfd0c, 0xfd0f) AM_READWRITE16(b38_keyboard_r, b38_keyboard_w,0xffffffff)
ADDRESS_MAP_END

static INPUT_PORTS_START( ngen )
INPUT_PORTS_END

static SLOT_INTERFACE_START(keyboard)
	SLOT_INTERFACE("ngen", NGEN_KEYBOARD)
SLOT_INTERFACE_END

static SLOT_INTERFACE_START( ngen_floppies )
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD )
SLOT_INTERFACE_END

static MACHINE_CONFIG_START( ngen, ngen_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu", I80186, XTAL_16MHz / 2)
	MCFG_CPU_PROGRAM_MAP(ngen_mem)
	MCFG_CPU_IO_MAP(ngen_io)
	MCFG_80186_CHIP_SELECT_CB(WRITE16(ngen_state, cpu_peripheral_cb))
	MCFG_80186_TMROUT0_HANDLER(WRITELINE(ngen_state, cpu_timer_w))
	MCFG_80186_IRQ_SLAVE_ACK(READ8(ngen_state, irq_cb))

	MCFG_PIC8259_ADD( "pic", DEVWRITELINE("maincpu",i80186_cpu_device,int0_w), VCC, NULL )

	MCFG_DEVICE_ADD("pit", PIT8254, 0)
	MCFG_PIT8253_CLK0(78120/4)  // 19.53kHz, /4 of the CPU timer output?
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(ngen_state, pit_out0_w))  // RS232 channel B baud rate
	MCFG_PIT8253_CLK1(XTAL_14_7456MHz/12)  // correct? - based on patent
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(ngen_state, pit_out1_w))  // RS232 channel A baud rate
	MCFG_PIT8253_CLK2(XTAL_14_7456MHz/12)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(ngen_state, pit_out2_w))

	MCFG_DEVICE_ADD("dmac", AM9517A, XTAL_14_7456MHz / 3)  // NEC D8237A, divisor unknown
	MCFG_I8237_OUT_HREQ_CB(WRITELINE(ngen_state, dma_hrq_changed))
	MCFG_I8237_OUT_EOP_CB(WRITELINE(ngen_state, dma_eop_changed))
	MCFG_I8237_IN_MEMR_CB(READ8(ngen_state, dma_read_word))  // DMA is always 16-bit
	MCFG_I8237_OUT_MEMW_CB(WRITE8(ngen_state, dma_write_word))
	MCFG_I8237_OUT_DACK_0_CB(WRITELINE(ngen_state, dack0_w))
	MCFG_I8237_OUT_DACK_1_CB(WRITELINE(ngen_state, dack1_w))
	MCFG_I8237_OUT_DACK_2_CB(WRITELINE(ngen_state, dack2_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(ngen_state, dack3_w))
	MCFG_I8237_IN_IOR_0_CB(READ8(ngen_state, dma_0_dack_r))
	MCFG_I8237_IN_IOR_1_CB(READ8(ngen_state, dma_1_dack_r))
	MCFG_I8237_IN_IOR_2_CB(READ8(ngen_state, dma_2_dack_r))
	MCFG_I8237_IN_IOR_3_CB(READ8(ngen_state, dma_3_dack_r))
	MCFG_I8237_OUT_IOW_0_CB(WRITE8(ngen_state, dma_0_dack_w))
	MCFG_I8237_OUT_IOW_1_CB(WRITE8(ngen_state, dma_1_dack_w))
	MCFG_I8237_OUT_IOW_2_CB(WRITE8(ngen_state, dma_2_dack_w))
	MCFG_I8237_OUT_IOW_3_CB(WRITE8(ngen_state, dma_3_dack_w))

	// I/O board
	MCFG_UPD7201_ADD("iouart",0,0,0,0,0) // clocked by PIT channel 2?
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE("rs232_a", rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_TXDB_CB(DEVWRITELINE("rs232_b", rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE("rs232_a", rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_DTRB_CB(DEVWRITELINE("rs232_b", rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE("rs232_a", rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_RTSB_CB(DEVWRITELINE("rs232_b", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232_a", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("iouart", upd7201_device, rxa_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("iouart", upd7201_device, ctsa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("iouart", upd7201_device, dcda_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("iouart", upd7201_device, ria_w))

	MCFG_RS232_PORT_ADD("rs232_b", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("iouart", upd7201_device, rxb_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("iouart", upd7201_device, ctsb_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("iouart", upd7201_device, dcdb_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("iouart", upd7201_device, rib_w))

	// TODO: SCN2652 MPCC (not implemented), used for RS-422 cluster communications?

	// video board
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE(720,348)
	MCFG_SCREEN_VISIBLE_AREA(0,719,0,347)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_DEVICE("crtc",mc6845_device, screen_update)

	MCFG_MC6845_ADD("crtc", MC6845, nullptr, 19980000 / 9)  // divisor unknown -- /9 gives 60Hz output, so likely correct
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(9)
	MCFG_MC6845_UPDATE_ROW_CB(ngen_state, crtc_update_row)
	MCFG_VIDEO_SET_SCREEN("screen")

	// keyboard UART (patent says i8251 is used for keyboard communications, it is located on the video board)
	MCFG_DEVICE_ADD("videouart", I8251, 0)  // main clock unknown, Rx/Tx clocks are 19.53kHz
//  MCFG_I8251_TXEMPTY_HANDLER(DEVWRITELINE("pic",pic8259_device,ir4_w))
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("keyboard", rs232_port_device, write_txd))
	MCFG_RS232_PORT_ADD("keyboard", keyboard, "ngen")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("videouart", i8251_device, write_rxd))

	MCFG_DEVICE_ADD("refresh_clock", CLOCK, 19200*16)  // should be 19530Hz
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(ngen_state,timer_clk_out))

	// floppy disk / hard disk module (WD2797 FDC, WD1010 HDC, plus an 8253 timer for each)
	MCFG_WD2797_ADD("fdc", XTAL_20MHz / 20)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(ngen_state,fdc_irq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(DEVWRITELINE("maincpu",i80186_cpu_device,drq1_w))
	MCFG_WD_FDC_FORCE_READY
	MCFG_DEVICE_ADD("fdc_timer", PIT8253, 0)
	MCFG_PIT8253_CLK0(0)
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE("pic",pic8259_device,ir5_w))  // clocked on FDC data register access
	MCFG_PIT8253_CLK1(XTAL_20MHz / 20)
//  MCFG_PIT8253_OUT1_HANDLER(DEVWRITELINE("pic",pic8259_device,ir5_w))  // 1MHz
	MCFG_PIT8253_CLK2(XTAL_20MHz / 20)
//  MCFG_PIT8253_OUT2_HANDLER(DEVWRITELINE("pic",pic8259_device,ir5_w))

	// TODO: WD1010 HDC (not implemented), use WD2010 for now
	MCFG_DEVICE_ADD("hdc", WD2010, XTAL_20MHz / 4)
	MCFG_WD2010_OUT_INTRQ_CB(DEVWRITELINE("pic",pic8259_device,ir2_w))
	MCFG_WD2010_IN_BCS_CB(READ8(ngen_state,hd_buffer_r))
	MCFG_WD2010_OUT_BCS_CB(WRITE8(ngen_state,hd_buffer_w))
	MCFG_WD2010_IN_DRDY_CB(VCC)
	MCFG_WD2010_IN_INDEX_CB(VCC)
	MCFG_WD2010_IN_WF_CB(VCC)
	MCFG_WD2010_IN_TK000_CB(VCC)
	MCFG_WD2010_IN_SC_CB(VCC)
	MCFG_DEVICE_ADD("hdc_timer", PIT8253, 0)
	MCFG_PIT8253_CLK2(XTAL_20MHz / 10)  // 2MHz
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ngen_floppies, "525qd", floppy_image_device::default_floppy_formats)
	MCFG_HARDDISK_ADD("hard0")

MACHINE_CONFIG_END

static MACHINE_CONFIG_START( ngen386, ngen386_state )
	MCFG_CPU_ADD("i386cpu", I386, XTAL_50MHz / 2)
	MCFG_CPU_PROGRAM_MAP(ngen386_mem)
	MCFG_CPU_IO_MAP(ngen386_io)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic", pic8259_device, inta_cb)

	MCFG_PIC8259_ADD( "pic", INPUTLINE("i386cpu",0), VCC, NULL )

	MCFG_DEVICE_ADD("pit", PIT8254, 0)
	MCFG_PIT8253_CLK0(78120/4)  // 19.53kHz, /4 of the CPU timer output?
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(ngen_state, pit_out0_w))  // RS232 channel B baud rate
	MCFG_PIT8253_CLK1(XTAL_14_7456MHz/12)  // correct? - based on patent
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(ngen_state, pit_out1_w))  // RS232 channel A baud rate
	MCFG_PIT8253_CLK2(XTAL_14_7456MHz/12)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(ngen_state, pit_out2_w))

	MCFG_DEVICE_ADD("dmac", AM9517A, XTAL_14_7456MHz / 3)  // NEC D8237A, divisor unknown
	MCFG_I8237_OUT_HREQ_CB(WRITELINE(ngen_state, dma_hrq_changed))
	MCFG_I8237_OUT_EOP_CB(WRITELINE(ngen_state, dma_eop_changed))
	MCFG_I8237_IN_MEMR_CB(READ8(ngen_state, dma_read_word))  // DMA is always 16-bit
	MCFG_I8237_OUT_MEMW_CB(WRITE8(ngen_state, dma_write_word))
	MCFG_I8237_OUT_DACK_0_CB(WRITELINE(ngen_state, dack0_w))
	MCFG_I8237_OUT_DACK_1_CB(WRITELINE(ngen_state, dack1_w))
	MCFG_I8237_OUT_DACK_2_CB(WRITELINE(ngen_state, dack2_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(ngen_state, dack3_w))
	MCFG_I8237_IN_IOR_0_CB(READ8(ngen_state, dma_0_dack_r))
	MCFG_I8237_IN_IOR_1_CB(READ8(ngen_state, dma_1_dack_r))
	MCFG_I8237_IN_IOR_2_CB(READ8(ngen_state, dma_2_dack_r))
	MCFG_I8237_IN_IOR_3_CB(READ8(ngen_state, dma_3_dack_r))
	MCFG_I8237_OUT_IOW_0_CB(WRITE8(ngen_state, dma_0_dack_w))
	MCFG_I8237_OUT_IOW_1_CB(WRITE8(ngen_state, dma_1_dack_w))
	MCFG_I8237_OUT_IOW_2_CB(WRITE8(ngen_state, dma_2_dack_w))
	MCFG_I8237_OUT_IOW_3_CB(WRITE8(ngen_state, dma_3_dack_w))

	// I/O board
	MCFG_UPD7201_ADD("iouart",0,0,0,0,0) // clocked by PIT channel 2?
	MCFG_Z80DART_OUT_TXDA_CB(DEVWRITELINE("rs232_a", rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_TXDB_CB(DEVWRITELINE("rs232_b", rs232_port_device, write_txd))
	MCFG_Z80DART_OUT_DTRA_CB(DEVWRITELINE("rs232_a", rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_DTRB_CB(DEVWRITELINE("rs232_b", rs232_port_device, write_dtr))
	MCFG_Z80DART_OUT_RTSA_CB(DEVWRITELINE("rs232_a", rs232_port_device, write_rts))
	MCFG_Z80DART_OUT_RTSB_CB(DEVWRITELINE("rs232_b", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232_a", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("iouart", upd7201_device, rxa_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("iouart", upd7201_device, ctsa_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("iouart", upd7201_device, dcda_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("iouart", upd7201_device, ria_w))

	MCFG_RS232_PORT_ADD("rs232_b", default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("iouart", upd7201_device, rxb_w))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("iouart", upd7201_device, ctsb_w))
	MCFG_RS232_DCD_HANDLER(DEVWRITELINE("iouart", upd7201_device, dcdb_w))
	MCFG_RS232_RI_HANDLER(DEVWRITELINE("iouart", upd7201_device, rib_w))

	// TODO: SCN2652 MPCC (not implemented), used for RS-422 cluster communications?

	// video board
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE(720,348)
	MCFG_SCREEN_VISIBLE_AREA(0,719,0,347)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_DEVICE("crtc",mc6845_device, screen_update)

	MCFG_MC6845_ADD("crtc", MC6845, nullptr, 19980000 / 9)  // divisor unknown -- /9 gives 60Hz output, so likely correct
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(9)
	MCFG_MC6845_UPDATE_ROW_CB(ngen_state, crtc_update_row)
	MCFG_VIDEO_SET_SCREEN("screen")

	// keyboard UART (patent says i8251 is used for keyboard communications, it is located on the video board)
	MCFG_DEVICE_ADD("videouart", I8251, 0)  // main clock unknown, Rx/Tx clocks are 19.53kHz
//  MCFG_I8251_TXEMPTY_HANDLER(DEVWRITELINE("pic",pic8259_device,ir4_w))
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("keyboard", rs232_port_device, write_txd))
	MCFG_RS232_PORT_ADD("keyboard", keyboard, "ngen")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("videouart", i8251_device, write_rxd))

	MCFG_DEVICE_ADD("refresh_clock", CLOCK, 19200*16)  // should be 19530Hz
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(ngen_state,timer_clk_out))

	// floppy disk / hard disk module (WD2797 FDC, WD1010 HDC, plus an 8253 timer for each)
	MCFG_WD2797_ADD("fdc", XTAL_20MHz / 20)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(ngen_state,fdc_irq_w))
//  MCFG_WD_FDC_DRQ_CALLBACK(DEVWRITELINE("i386cpu",i80186_cpu_device,drq1_w))
	MCFG_WD_FDC_FORCE_READY
	MCFG_DEVICE_ADD("fdc_timer", PIT8253, 0)
	MCFG_PIT8253_CLK0(0)
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE("pic",pic8259_device,ir5_w))  // clocked on FDC data register access
	MCFG_PIT8253_CLK1(XTAL_20MHz / 20)
//  MCFG_PIT8253_OUT1_HANDLER(DEVWRITELINE("pic",pic8259_device,ir5_w))  // 1MHz
	MCFG_PIT8253_CLK2(XTAL_20MHz / 20)
//  MCFG_PIT8253_OUT2_HANDLER(DEVWRITELINE("pic",pic8259_device,ir5_w))

	// TODO: WD1010 HDC (not implemented), use WD2010 for now
	MCFG_DEVICE_ADD("hdc", WD2010, XTAL_20MHz / 4)
	MCFG_WD2010_OUT_INTRQ_CB(DEVWRITELINE("pic",pic8259_device,ir2_w))
	MCFG_WD2010_IN_BCS_CB(READ8(ngen_state,hd_buffer_r))
	MCFG_WD2010_OUT_BCS_CB(WRITE8(ngen_state,hd_buffer_w))
	MCFG_WD2010_IN_DRDY_CB(VCC)
	MCFG_WD2010_IN_INDEX_CB(VCC)
	MCFG_WD2010_IN_WF_CB(VCC)
	MCFG_WD2010_IN_TK000_CB(VCC)
	MCFG_WD2010_IN_SC_CB(VCC)
	MCFG_DEVICE_ADD("hdc_timer", PIT8253, 0)
	MCFG_PIT8253_CLK2(XTAL_20MHz / 10)  // 2MHz
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", ngen_floppies, "525qd", floppy_image_device::default_floppy_formats)
	MCFG_HARDDISK_ADD("hard0")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( 386i, ngen386 )
	MCFG_CPU_MODIFY("i386cpu")
	MCFG_CPU_PROGRAM_MAP(ngen386i_mem)
MACHINE_CONFIG_END

ROM_START( ngen )
	ROM_REGION( 0x2000, "bios", 0)
	ROM_LOAD16_BYTE( "72-00414_80186_cpu.bin",  0x000000, 0x001000, CRC(e1387a03) SHA1(ddca4eba67fbf8b731a8009c14f6b40edcbc3279) )  // bootstrap ROM v8.4
	ROM_LOAD16_BYTE( "72-00415_80186_cpu.bin",  0x000001, 0x001000, CRC(a6dde7d9) SHA1(b4d15c1bce31460ab5b92ff43a68c15ac5485816) )

	ROM_REGION16_LE( 0x2000, "vram", ROMREGION_ERASE00 )
	ROM_REGION16_LE( 0x2000, "fontram", ROMREGION_ERASE00 )

	ROM_REGION( 0x1000, "disk", 0)
	ROM_LOAD( "72-00422_10mb_disk.bin", 0x000000, 0x001000,  CRC(f5b046b6) SHA1(b303c6f6aa40504016de9826879bc316e44389aa) )

	ROM_REGION( 0x20, "disk_prom", 0)
	ROM_LOAD( "72-00422_10mb_disk_15d.bin", 0x000000, 0x000020,  CRC(121ee494) SHA1(9a8d3c336cc7378a71f9d48c99f88515eb236fbf) )
ROM_END

// not sure just how similar these systems are to the 80186 model, but are here at the moment to document the dumps
ROM_START( ngenb38 )
	ROM_REGION( 0x2000, "bios", 0)
	ROM_LOAD16_BYTE( "72-168_fpc_386_cpu.bin",  0x000000, 0x001000, CRC(250a3b68) SHA1(49c070514bac264fa4892f284f7d2c852ae6605d) )
	ROM_LOAD16_BYTE( "72-167_fpc_386_cpu.bin",  0x000001, 0x001000, CRC(4010cc4e) SHA1(74a3024d605569056484d08b63f19fbf8eaf31c6) )

	ROM_REGION16_LE( 0x2000, "vram", ROMREGION_ERASE00 )
	ROM_REGION16_LE( 0x2000, "fontram", ROMREGION_ERASE00 )
ROM_END

ROM_START( 386i )
	ROM_REGION( 0x4000, "bios", 0)
	ROM_LOAD16_BYTE( "72-1561o_386i_cpu.bin",  0x000000, 0x002000, CRC(b5efd768) SHA1(8b250d47d9c6eb82e1afaeb2244d8c4134ecbc47) )
	ROM_LOAD16_BYTE( "72-1562e_386i_cpu.bin",  0x000001, 0x002000, CRC(002d0d3a) SHA1(31de8592999377db9251acbeff348390a2d2602a) )

	ROM_REGION16_LE( 0x2000, "vram", ROMREGION_ERASE00 )
	ROM_REGION16_LE( 0x2000, "fontram", ROMREGION_ERASE00 )

	ROM_REGION( 0x2000, "video", 0)
	ROM_LOAD( "72-1630_gc-104_vga.bin",  0x000000, 0x002000, CRC(4e4d8ebe) SHA1(50c96ccb4d0bd1beb2d1aee0d18b2c462d25fc8f) )
ROM_END


COMP( 1983, ngen,    0,      0,      ngen,           ngen, driver_device, 0,      "Convergent Technologies",  "NGEN CP-001", MACHINE_IS_SKELETON | MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1991, ngenb38, ngen,   0,      ngen386,        ngen, driver_device, 0,      "Financial Products Corp.", "B28/38",      MACHINE_IS_SKELETON | MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
COMP( 1990, 386i,    ngen,   0,      386i,           ngen, driver_device, 0,      "Convergent Technologies",  "386i",        MACHINE_IS_SKELETON | MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
