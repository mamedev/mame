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
#include "bus/rs232/rs232.h"
#include "cpu/i386/i386.h"
#include "cpu/i86/i186.h"
#include "imagedev/floppy.h"
#include "imagedev/harddriv.h"
#include "machine/am9517a.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "ngen_kb.h"
#include "machine/pic8259.h"
#include "machine/pit8253.h"
#include "machine/wd2010.h"
#include "machine/wd_fdc.h"
#include "machine/z80sio.h"
#include "video/mc6845.h"
#include "memarray.h"
#include "screen.h"


namespace {

class ngen_state : public driver_device
{
public:
	ngen_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_i386cpu(*this,"i386cpu"),
		m_crtc(*this,"crtc"),
		m_viduart(*this,"videouart"),
		m_iouart(*this,"iouart"),
		m_dmac(*this,"dmac"),
		m_pic(*this,"pic"),
		m_pit(*this,"pit"),
		m_hdc(*this,"hdc"),
		m_fdc(*this,"fdc"),
		m_fdc_timer(*this,"fdc_timer"),
		m_hdc_timer(*this,"hdc_timer"),
		m_disk_rom(*this,"disk"),
		m_fd0(*this,"fdc:0"),
		m_hd_buffer(*this,"hd_buffer_ram", 1024*8, ENDIANNESS_LITTLE)
	{
	}

	void ngen(machine_config &config);

protected:
	uint8_t hd_buffer_r(offs_t offset);
	void hd_buffer_w(offs_t offset, uint8_t data);

	void pit_out0_w(int state);
	void pit_out1_w(int state);
	void pit_out2_w(int state);

	void dma_hrq_changed(int state);
	void dma_eop_changed(int state);
	void dack0_w(int state);
	void dack1_w(int state);
	void dack2_w(int state);
	void dack3_w(int state);
	uint8_t dma_read_word(offs_t offset);
	void dma_write_word(offs_t offset, uint8_t data);
	// TODO: sort out what devices use which channels
	uint8_t dma_0_dack_r() { uint16_t ret = 0xffff; m_dma_high_byte = ret & 0xff00; return ret; }
	uint8_t dma_1_dack_r() { uint16_t ret = 0xffff; m_dma_high_byte = ret & 0xff00; return ret; }
	uint8_t dma_2_dack_r() { uint16_t ret = 0xffff; m_dma_high_byte = ret & 0xff00; return ret; }
	uint8_t dma_3_dack_r();
	void dma_0_dack_w(uint8_t data) { popmessage("IOW0: data %02x",data); }
	void dma_1_dack_w(uint8_t data) { }
	void dma_2_dack_w(uint8_t data) { }
	void dma_3_dack_w(uint8_t data) { popmessage("IOW3: data %02x",data); }

	MC6845_UPDATE_ROW(crtc_update_row);

	void timer_clk_out(int state);

	void fdc_irq_w(int state);

	void ngen386_io(address_map &map) ATTR_COLD;
	void ngen386_mem(address_map &map) ATTR_COLD;
	void ngen386i_mem(address_map &map) ATTR_COLD;

	optional_device<i80186_cpu_device> m_maincpu;
	optional_device<i386_device> m_i386cpu;
	required_device<mc6845_device> m_crtc;
	required_device<i8251_device> m_viduart;
	required_device<upd7201_device> m_iouart;
	required_device<am9517a_device> m_dmac;
	required_device<pic8259_device> m_pic;
	required_device<pit8254_device> m_pit;
	optional_device<wd2010_device> m_hdc;
	optional_device<wd2797_device> m_fdc;
	optional_device<pit8253_device> m_fdc_timer;
	optional_device<pit8253_device> m_hdc_timer;

private:
	void cpu_peripheral_cb(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void peripheral_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t peripheral_r(offs_t offset, uint16_t mem_mask = ~0);
	void xbus_w(uint16_t data);
	uint16_t xbus_r();

	void cpu_timer_w(int state);

	void hfd_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t hfd_r(offs_t offset, uint16_t mem_mask = ~0);
	[[maybe_unused]] void fdc_drq_w(int state);
	void fdc_control_w(uint8_t data);
	uint8_t irq_cb();
	void hdc_control_w(uint8_t data);
	void disk_addr_ext(uint8_t data);

	uint16_t b38_keyboard_r(offs_t offset, uint16_t mem_mask = ~0);
	void b38_keyboard_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t b38_crtc_r(offs_t offset, uint16_t mem_mask = ~0);
	void b38_crtc_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void ngen_io(address_map &map) ATTR_COLD;
	void ngen_mem(address_map &map) ATTR_COLD;

	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

	optional_memory_region m_disk_rom;
	memory_array m_vram;
	memory_array m_fontram;
	optional_device<floppy_connector> m_fd0;
	memory_share_creator<uint8_t> m_hd_buffer;

	void set_dma_channel(int channel, int state);

	uint8_t m_xbus_current;  // currently selected X-Bus module
	uint16_t m_peripheral;
	uint16_t m_upper;
	uint16_t m_middle;
	uint16_t m_port00;
	uint16_t m_periph141;
	uint8_t m_dma_offset[4];
	int8_t m_dma_channel;
	uint16_t m_dma_high_byte;
	uint16_t m_control;
	uint16_t m_disk_rom_ptr;
	uint8_t m_hdc_control;
	uint8_t m_disk_page;
};

class ngen386_state : public ngen_state
{
public:
	ngen386_state(const machine_config &mconfig, device_type type, const char *tag)
		: ngen_state(mconfig, type, tag)
		{}
		void ngen386(machine_config &config);
		void _386i(machine_config &config);
private:
};

void ngen_state::pit_out0_w(int state)
{
	m_pic->ir3_w(state);  // Timer interrupt
	popmessage("PIT Timer 0 state %i\n",state);
}

void ngen_state::pit_out1_w(int state)
{
	popmessage("PIT Timer 1 state %i\n",state);
	m_iouart->rxcb_w(state);
	m_iouart->txcb_w(state);  // channels in the correct order?
}

void ngen_state::pit_out2_w(int state)
{
	m_iouart->rxca_w(state);
	m_iouart->txca_w(state);
	popmessage("PIT Timer 2 state %i\n",state);
}

void ngen_state::cpu_timer_w(int state)
{
	if(state != 0)
		popmessage("80186 Timer 0 state %i\n",state);
	m_pic->ir5_w(state);
}

void ngen_state::timer_clk_out(int state)
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

void ngen_state::cpu_peripheral_cb(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint32_t addr;

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
			m_maincpu->space(AS_PROGRAM).install_readwrite_handler(addr, addr + 0x3ff, read16s_delegate(*this, FUNC(ngen_state::peripheral_r)), write16s_delegate(*this, FUNC(ngen_state::peripheral_w)));
			logerror("Mapped peripherals to memory 0x%08x\n",addr);
		}
		else
		{
			addr &= 0xffff;
			m_maincpu->space(AS_IO).install_readwrite_handler(addr, addr + 0x3ff, read16s_delegate(*this, FUNC(ngen_state::peripheral_r)), write16s_delegate(*this, FUNC(ngen_state::peripheral_w)));
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
void ngen_state::peripheral_w(offs_t offset, uint16_t data, uint16_t mem_mask)
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
		if(ACCESSING_BITS_0_7)
			m_dmac->write(offset,data & 0xff);
		break;
	case 0x80: // DMA page offset?
	case 0x81:
	case 0x82:
	case 0x83:
		if(ACCESSING_BITS_0_7)
			m_dma_offset[offset-0x80] = data & 0xff;
		break;
	case 0xc0:  // X-Bus modules reset
		m_xbus_current = 0;
		break;
	case 0x10c:
		if(ACCESSING_BITS_0_7)
			m_pic->write(0,data & 0xff);
		break;
	case 0x10d:
		if(ACCESSING_BITS_0_7)
			m_pic->write(1,data & 0xff);
		break;
	case 0x110:
	case 0x111:
	case 0x112:
	case 0x113:
		if(ACCESSING_BITS_0_7)
			m_pit->write(offset-0x110,data & 0xff);
		break;
	case 0x141:
		// bit 1 enables speaker?
		COMBINE_DATA(&m_periph141);
		break;
	case 0x144:
		if(ACCESSING_BITS_0_7)
			m_crtc->address_w(data & 0xff);
		break;
	case 0x145:
		if(ACCESSING_BITS_0_7)
			m_crtc->register_w(data & 0xff);
		break;
	case 0x146:
	case 0x147:
		if(ACCESSING_BITS_0_7)
			m_viduart->write(offset & 1, data & 0xff);
		break;
	case 0x1a0:  // serial?
		logerror("Serial(?) 0x1a0 write offset %04x data %04x mask %04x\n",offset,data,mem_mask);
		break;
	default:
		logerror("Unknown 80186 peripheral write offset %04x data %04x mask %04x\n",offset,data,mem_mask);
	}
}

uint16_t ngen_state::peripheral_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t ret = 0xffff;
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
		if(ACCESSING_BITS_0_7)
			ret = m_dmac->read(offset);
		logerror("DMA read offset %04x mask %04x returning %04x\n",offset,mem_mask,ret);
		break;
	case 0x80: // DMA page offset?
	case 0x81:
	case 0x82:
	case 0x83:
		if(ACCESSING_BITS_0_7)
			ret = m_dma_offset[offset-0x80] & 0xff;
		break;
	case 0x10c:
		if(ACCESSING_BITS_0_7)
			ret = m_pic->read(0);
		break;
	case 0x10d:
		if(ACCESSING_BITS_0_7)
			ret = m_pic->read(1);
		break;
	case 0x110:
	case 0x111:
	case 0x112:
	case 0x113:
		if(ACCESSING_BITS_0_7)
			ret = m_pit->read(offset-0x110);
		break;
	case 0x141:
		ret = m_periph141;
		break;
	case 0x144:
		if(ACCESSING_BITS_0_7)
			ret = m_crtc->status_r();
		break;
	case 0x145:
		if(ACCESSING_BITS_0_7)
			ret = m_crtc->register_r();
		break;
	case 0x146:
	case 0x147:  // keyboard UART
		// status expects bit 0 to be set (UART transmit ready)
		if(ACCESSING_BITS_0_7)
			ret = m_viduart->read(offset & 1);
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
void ngen_state::xbus_w(uint16_t data)
{
	uint16_t addr = (data & 0x00ff) << 8;
	cpu_device* cpu;

	if(m_maincpu)
		cpu = m_maincpu;
	else
		cpu = m_i386cpu;
	address_space& io = cpu->space(AS_IO);
	switch(m_xbus_current)
	{
		case 0x00:  // Floppy/Hard disk module
			io.install_readwrite_handler(addr,addr+0xff, read16s_delegate(*this, FUNC(ngen_state::hfd_r)), write16s_delegate(*this, FUNC(ngen_state::hfd_w)), 0xffffffff);
			break;
		default:
			cpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);  // reached end of the modules
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
uint16_t ngen_state::xbus_r()
{
	uint16_t ret = 0xffff;

	switch(m_xbus_current)
	{
		case 0x00:
			ret = 0x1070;  // Floppy/Hard disk module
			break;
		default:
			if(m_maincpu)
				m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);  // reached the end of the modules
			else
				m_i386cpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
			ret = 0x0080;
			break;
	}
	return ret;
}


// Floppy/Hard disk module
void ngen_state::hfd_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch(offset)
	{
		case 0x00:
		case 0x01:
		case 0x02:
			if(ACCESSING_BITS_0_7)
				m_fdc->write(offset,data & 0xff);
			break;
		case 0x03:
			if(ACCESSING_BITS_0_7)
			{
				m_fdc->write(offset,data & 0xff);
				m_fdc_timer->write_clk0(1);
				m_fdc_timer->write_clk0(0);  // Data register access clocks the FDC's PIT channel 0
			}
			break;
		case 0x04:
			if(ACCESSING_BITS_0_7)
				fdc_control_w(data & 0xff);
			break;
		case 0x05:
			if(ACCESSING_BITS_0_7)
				hdc_control_w(data & 0xff);
			break;
		case 0x07:
			if(ACCESSING_BITS_0_7)
				disk_addr_ext(data & 0xff);
			break;
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
			if(ACCESSING_BITS_0_7)
				m_fdc_timer->write(offset-0x08,data & 0xff);
			break;
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
			if(ACCESSING_BITS_0_7)
				m_hdc->write(offset-0x10,data & 0xff);
			logerror("WD1010 register %i write %02x mask %04x\n",offset-0x10,data & 0xff,mem_mask);
			break;
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
			if(ACCESSING_BITS_0_7)
				m_hdc_timer->write(offset-0x18,data & 0xff);
			break;
	}
}

uint16_t ngen_state::hfd_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t ret = 0xffff;

	switch(offset)
	{
		case 0x00:
		case 0x01:
		case 0x02:
			if(ACCESSING_BITS_0_7)
				ret = m_fdc->read(offset);
			break;
		case 0x03:
			if(ACCESSING_BITS_0_7)
			{
				ret = m_fdc->read(offset);
				m_fdc_timer->write_clk0(1);
				m_fdc_timer->write_clk0(0);  // Data register access clocks the FDC's PIT channel 0
			}
			break;
		case 0x08:
		case 0x09:
		case 0x0a:
		case 0x0b:
			if(ACCESSING_BITS_0_7)
				ret = m_fdc_timer->read(offset-0x08);
			break;
		case 0x10:
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
			if(ACCESSING_BITS_0_7)
				ret = m_hdc->read(offset-0x10);
			logerror("WD1010 register %i read, mask %04x\n",offset-0x10,mem_mask);
			break;
		case 0x18:
		case 0x19:
		case 0x1a:
		case 0x1b:
			if(ACCESSING_BITS_0_7)
				ret = m_hdc_timer->read(offset-0x18);
			break;
	}

	return ret;
}

void ngen_state::fdc_irq_w(int state)
{
	m_pic->ir7_w(state);
}

void ngen_state::fdc_drq_w(int state)
{
	m_dmac->dreq3_w(state);
}

// Floppy disk control register
// Bit 0 - enable drive and LED
// Bit 2 - floppy motor
// Bit 5 - side select
// Bit 6 - 1 = 2Mhz for seek, 0 = 1MHz for read/write
// Bit 7 - FDC reset
void ngen_state::fdc_control_w(uint8_t data)
{
	m_fdc->set_floppy(m_fd0->get_device());
	m_fd0->get_device()->mon_w(!BIT(data, 2));
	m_fd0->get_device()->ss_w(BIT(data, 5));
	m_fdc->mr_w(BIT(data, 7));
}

// Hard disk control register
// bit 0 - Drive select 0 - selects module hard disk
// bit 1 - Drive select 1 - selects expansion module hard disk (if available)
// bit 2 - enable DMA transfer of module ROM contents to X-Bus master memory
// bits 3-5 - select head / expansion module head
// bit 6 - write enable, must be set to write to a hard disk
// bit 7 - HDC reset
void ngen_state::hdc_control_w(uint8_t data)
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
void ngen_state::disk_addr_ext(uint8_t data)
{
	m_disk_page = data & 0x7f;
}

uint8_t ngen_state::hd_buffer_r(offs_t offset)
{
	return m_hd_buffer[offset];
}

void ngen_state::hd_buffer_w(offs_t offset, uint8_t data)
{
	m_hd_buffer[offset] = data;
}

void ngen_state::dma_hrq_changed(int state)
{
	if(m_maincpu)
		m_maincpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);
	else
		m_i386cpu->set_input_line(INPUT_LINE_HALT, state ? ASSERT_LINE : CLEAR_LINE);
}

void ngen_state::dma_eop_changed(int state)
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

void ngen_state::dack0_w(int state) { set_dma_channel(0, state); }
void ngen_state::dack1_w(int state) { set_dma_channel(1, state); }
void ngen_state::dack2_w(int state) { set_dma_channel(2, state); }
void ngen_state::dack3_w(int state) { set_dma_channel(3, state); }

uint8_t ngen_state::dma_3_dack_r()
{
	uint16_t ret = 0xffff;

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

uint8_t ngen_state::dma_read_word(offs_t offset)
{
	cpu_device* cpu;
	uint16_t result;

	if(m_maincpu)
		cpu = m_maincpu;
	else
		cpu = m_i386cpu;
	address_space& prog_space = cpu->space(AS_PROGRAM); // get the right address space

	if(m_dma_channel == -1)
		return 0xff;
	offs_t page_offset = ((offs_t) m_dma_offset[m_dma_channel]) << 16;

	result = prog_space.read_word((page_offset & 0xfe0000) | (offset << 1));
	m_dma_high_byte = result & 0xFF00;
	popmessage("DMA byte address %06x read %04x\n", (page_offset & 0xfe0000) | (offset << 1),result);
	return result & 0xff;
}


void ngen_state::dma_write_word(offs_t offset, uint8_t data)
{
	cpu_device* cpu;

	if(m_maincpu)
		cpu = m_maincpu;
	else
		cpu = m_i386cpu;
	address_space& prog_space = cpu->space(AS_PROGRAM); // get the right address space

	if(m_dma_channel == -1)
		return;
	offs_t page_offset = ((offs_t) m_dma_offset[m_dma_channel]) << 16;

	prog_space.write_word((page_offset & 0xfe0000) | (offset << 1), data);
	popmessage("DMA byte address %06x write %04x\n", (page_offset & 0xfe0000) | (offset << 1), m_dma_high_byte | data);
}


MC6845_UPDATE_ROW( ngen_state::crtc_update_row )
{
	uint16_t addr = ma;

	for(int x=0;x<bitmap.width();x+=9)
	{
		uint8_t ch = m_vram.read16(addr++) & 0xff;
		for(int z=0;z<9;z++)
		{
			if(BIT(m_fontram.read16(ch*16+ra),8-z))
				bitmap.pix(y,x+z) = rgb_t(0,0xff,0);
			else
				bitmap.pix(y,x+z) = rgb_t(0,0,0);
		}
	}
}

uint8_t ngen_state::irq_cb()
{
	return m_pic->acknowledge();
}

uint16_t ngen_state::b38_keyboard_r(offs_t offset, uint16_t mem_mask)
{
	uint8_t ret = 0;
	switch(offset)
	{
	case 0:
	case 1:  // keyboard UART
		// status expects bit 0 to be set (UART transmit ready)
		if(ACCESSING_BITS_0_7)
			ret = m_viduart->read(offset & 1);
		break;
	}
	return ret;
}

void ngen_state::b38_keyboard_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch(offset)
	{
	case 0:
	case 1:
		if(ACCESSING_BITS_0_7)
			m_viduart->write(offset & 1, data & 0xff);
		break;
	}
}

uint16_t ngen_state::b38_crtc_r(offs_t offset, uint16_t mem_mask)
{
	uint8_t ret = 0;
	switch(offset)
	{
	case 0:
		if(ACCESSING_BITS_0_7)
			ret = m_crtc->register_r();
		break;
	case 1:
		if(ACCESSING_BITS_0_7)
			ret = m_viduart->data_r();
		break;
	}
	return ret;
}

void ngen_state::b38_crtc_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch(offset)
	{
	case 0:
		if(ACCESSING_BITS_0_7)
			m_crtc->address_w(data & 0xff);
		break;
	case 1:
		if(ACCESSING_BITS_0_7)
			m_crtc->register_w(data & 0xff);
		break;
	}
}

void ngen_state::machine_start()
{
	memory_share* vidshare = memshare("vram");
	memory_share* fontshare = memshare("fontram");
	if(vidshare == nullptr || fontshare == nullptr)
		fatalerror("VRAM not found\n");
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
void ngen_state::ngen_mem(address_map &map)
{
	map(0x00000, 0xf7fff).ram();
	map(0xf8000, 0xf9fff).ram().share("vram");
	map(0xfa000, 0xfbfff).ram().share("fontram");
	map(0xfc000, 0xfcfff).ram();
	map(0xfe000, 0xfffff).rom().region("bios", 0);
}

void ngen_state::ngen_io(address_map &map)
{
	map(0x0000, 0x0001).rw(FUNC(ngen_state::xbus_r), FUNC(ngen_state::xbus_w));

	// Floppy/Hard disk module
//  map(0x0100, 0x0107).rw("fdc", FUNC(wd2797_t::read), FUNC(wd2797_t::write)).umask16(0x00ff);  // a guess for now
//  map(0x0108, 0x0108).w(FUNC(ngen_state::fdc_control_w));
//  map(0x010a, 0x010a).w(FUNC(ngen_state::hdc_control_w));
//  map(0x010e, 0x010e).w(FUNC(ngen_state::disk_addr_ext));  // X-Bus extended address register
//  map(0x0110, 0x0117).rw("fdc_timer", FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	// 0x0120-0x012f - WD1010 Winchester disk controller (unemulated)
//  map(0x0130, 0x0137).rw("hdc_timer", FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);

}

void ngen_state::ngen386_mem(address_map &map)
{
	map(0x00000000, 0x000f7fff).ram();
	map(0x000f8000, 0x000f9fff).ram().share("vram");
	map(0x000fa000, 0x000fbfff).ram().share("fontram");
	map(0x000fc000, 0x000fcfff).ram();
	map(0x000fe000, 0x000fffff).rom().region("bios", 0);
	map(0x00100000, 0x00ffffff).ram();  // some extra RAM
	map(0xffffe000, 0xffffffff).rom().region("bios", 0);
}

void ngen_state::ngen386i_mem(address_map &map)
{
	map(0x00000000, 0x000f7fff).ram();
	map(0x000f8000, 0x000f9fff).ram().share("vram");
	map(0x000fa000, 0x000fbfff).ram().share("fontram");
	map(0x000fc000, 0x000fffff).rom().region("bios", 0);
	map(0x00100000, 0x00ffffff).ram();  // some extra RAM
	map(0xffffc000, 0xffffffff).rom().region("bios", 0);
}

void ngen_state::ngen386_io(address_map &map)
{
	map(0x0000, 0x0001).rw(FUNC(ngen_state::xbus_r), FUNC(ngen_state::xbus_w));
//  map(0xf800, 0xfeff).rw(FUNC(ngen_state::peripheral_r), FUNC(ngen_state::peripheral_w));
	map(0xfd08, 0xfd0b).rw(FUNC(ngen_state::b38_crtc_r), FUNC(ngen_state::b38_crtc_w));
	map(0xfd0c, 0xfd0f).rw(FUNC(ngen_state::b38_keyboard_r), FUNC(ngen_state::b38_keyboard_w));
}

static INPUT_PORTS_START( ngen )
INPUT_PORTS_END

static void keyboard(device_slot_interface &device)
{
	device.option_add("ngen", NGEN_KEYBOARD);
}

static void ngen_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

void ngen_state::ngen(machine_config &config)
{
	// basic machine hardware
	I80186(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &ngen_state::ngen_mem);
	m_maincpu->set_addrmap(AS_IO, &ngen_state::ngen_io);
	m_maincpu->chip_select_callback().set(FUNC(ngen_state::cpu_peripheral_cb));
	m_maincpu->tmrout0_handler().set(FUNC(ngen_state::cpu_timer_w));
	m_maincpu->read_slave_ack_callback().set(FUNC(ngen_state::irq_cb));

	PIC8259(config, m_pic, 0);
	m_pic->out_int_callback().set(m_maincpu, FUNC(i80186_cpu_device::int0_w));

	PIT8254(config, m_pit, 0);
	m_pit->set_clk<0>(78120/4);  // 19.53kHz, /4 of the CPU timer output?
	m_pit->out_handler<0>().set(FUNC(ngen_state::pit_out0_w));  // RS232 channel B baud rate
	m_pit->set_clk<1>(14.7456_MHz_XTAL / 12);  // correct? - based on patent
	m_pit->out_handler<1>().set(FUNC(ngen_state::pit_out1_w));  // RS232 channel A baud rate
	m_pit->set_clk<2>(14.7456_MHz_XTAL / 12);
	m_pit->out_handler<2>().set(FUNC(ngen_state::pit_out2_w));

	AM9517A(config, m_dmac, 14.7456_MHz_XTAL / 3);  // NEC D8237A, divisor unknown
	m_dmac->out_hreq_callback().set(FUNC(ngen_state::dma_hrq_changed));
	m_dmac->out_eop_callback().set(FUNC(ngen_state::dma_eop_changed));
	m_dmac->in_memr_callback().set(FUNC(ngen_state::dma_read_word));  // DMA is always 16-bit
	m_dmac->out_memw_callback().set(FUNC(ngen_state::dma_write_word));
	m_dmac->out_dack_callback<0>().set(FUNC(ngen_state::dack0_w));
	m_dmac->out_dack_callback<1>().set(FUNC(ngen_state::dack1_w));
	m_dmac->out_dack_callback<2>().set(FUNC(ngen_state::dack2_w));
	m_dmac->out_dack_callback<3>().set(FUNC(ngen_state::dack3_w));
	m_dmac->in_ior_callback<0>().set(FUNC(ngen_state::dma_0_dack_r));
	m_dmac->in_ior_callback<1>().set(FUNC(ngen_state::dma_1_dack_r));
	m_dmac->in_ior_callback<2>().set(FUNC(ngen_state::dma_2_dack_r));
	m_dmac->in_ior_callback<3>().set(FUNC(ngen_state::dma_3_dack_r));
	m_dmac->out_iow_callback<0>().set(FUNC(ngen_state::dma_0_dack_w));
	m_dmac->out_iow_callback<1>().set(FUNC(ngen_state::dma_1_dack_w));
	m_dmac->out_iow_callback<2>().set(FUNC(ngen_state::dma_2_dack_w));
	m_dmac->out_iow_callback<3>().set(FUNC(ngen_state::dma_3_dack_w));

	// I/O board
	UPD7201(config, m_iouart, 0); // clocked by PIT channel 2?
	m_iouart->out_txda_callback().set("rs232_a", FUNC(rs232_port_device::write_txd));
	m_iouart->out_txdb_callback().set("rs232_b", FUNC(rs232_port_device::write_txd));
	m_iouart->out_dtra_callback().set("rs232_a", FUNC(rs232_port_device::write_dtr));
	m_iouart->out_dtrb_callback().set("rs232_b", FUNC(rs232_port_device::write_dtr));
	m_iouart->out_rtsa_callback().set("rs232_a", FUNC(rs232_port_device::write_rts));
	m_iouart->out_rtsb_callback().set("rs232_b", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232_a", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_iouart, FUNC(upd7201_device::rxa_w));
	rs232a.cts_handler().set(m_iouart, FUNC(upd7201_device::ctsa_w));
	rs232a.dcd_handler().set(m_iouart, FUNC(upd7201_device::dcda_w));
	rs232a.ri_handler().set(m_iouart, FUNC(upd7201_device::synca_w));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232_b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_iouart, FUNC(upd7201_device::rxb_w));
	rs232b.cts_handler().set(m_iouart, FUNC(upd7201_device::ctsb_w));
	rs232b.dcd_handler().set(m_iouart, FUNC(upd7201_device::dcdb_w));
	rs232b.ri_handler().set(m_iouart, FUNC(upd7201_device::syncb_w));

	// TODO: SCN2652 MPCC (not implemented), used for RS-422 cluster communications?

	// video board
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_size(720, 348);
	screen.set_visarea(0, 719, 0, 347);
	screen.set_refresh_hz(60);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	MC6845(config, m_crtc, 19980000 / 9);  // divisor unknown -- /9 gives 60Hz output, so likely correct
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(9);
	m_crtc->set_update_row_callback(FUNC(ngen_state::crtc_update_row));

	// keyboard UART (patent says i8251 is used for keyboard communications, it is located on the video board)
	I8251(config, m_viduart, 0);  // main clock unknown, Rx/Tx clocks are 19.53kHz
//  m_viduart->txempty_handler().set(m_pic, FUNC(pic8259_device::ir4_w));
	m_viduart->txd_handler().set("keyboard", FUNC(rs232_port_device::write_txd));
	rs232_port_device &kbd(RS232_PORT(config, "keyboard", keyboard, "ngen"));
	kbd.rxd_handler().set(m_viduart, FUNC(i8251_device::write_rxd));

	CLOCK(config, "refresh_clock", 19200*16).signal_handler().set(FUNC(ngen_state::timer_clk_out)); // should be 19530Hz

	// floppy disk / hard disk module (WD2797 FDC, WD1010 HDC, plus an 8253 timer for each)
	WD2797(config, m_fdc, 20_MHz_XTAL / 20);
	m_fdc->intrq_wr_callback().set(FUNC(ngen_state::fdc_irq_w));
	m_fdc->drq_wr_callback().set(m_maincpu, FUNC(i80186_cpu_device::drq1_w));
	m_fdc->set_force_ready(true);

	PIT8253(config, m_fdc_timer, 0);
	m_fdc_timer->set_clk<0>(0);
	m_fdc_timer->out_handler<0>().set(m_pic, FUNC(pic8259_device::ir5_w));  // clocked on FDC data register access
	m_fdc_timer->set_clk<1>(20_MHz_XTAL / 20);
//  m_fdc_timer->out_handler<1>().set(m_pic, FUNC(pic8259_device::ir5_w));  // 1MHz
	m_fdc_timer->set_clk<2>(20_MHz_XTAL / 20);
//  m_fdc_timer->out_handler<2>().set(m_pic, FUNC(pic8259_device::ir5_w));

	// TODO: WD1010 HDC (not implemented), use WD2010 for now
	WD2010(config, m_hdc, 20_MHz_XTAL / 4);
	m_hdc->out_intrq_callback().set(m_pic, FUNC(pic8259_device::ir2_w));
	m_hdc->in_bcs_callback().set(FUNC(ngen_state::hd_buffer_r));
	m_hdc->out_bcs_callback().set(FUNC(ngen_state::hd_buffer_w));
	m_hdc->in_drdy_callback().set_constant(1);
	m_hdc->in_index_callback().set_constant(1);
	m_hdc->in_wf_callback().set_constant(1);
	m_hdc->in_tk000_callback().set_constant(1);
	m_hdc->in_sc_callback().set_constant(1);

	PIT8253(config, m_hdc_timer, 0);
	m_hdc_timer->set_clk<2>(20_MHz_XTAL / 10);  // 2MHz

	FLOPPY_CONNECTOR(config, "fdc:0", ngen_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats);
	HARDDISK(config, "hard0", 0);
}

void ngen386_state::ngen386(machine_config &config)
{
	I386(config, m_i386cpu, 50_MHz_XTAL / 2);
	m_i386cpu->set_addrmap(AS_PROGRAM, &ngen386_state::ngen386_mem);
	m_i386cpu->set_addrmap(AS_IO, &ngen386_state::ngen386_io);
	m_i386cpu->set_irq_acknowledge_callback("pic", FUNC(pic8259_device::inta_cb));

	PIC8259(config, m_pic, 0);
	m_pic->out_int_callback().set_inputline(m_i386cpu, 0);

	PIT8254(config, m_pit, 0);
	m_pit->set_clk<0>(78120/4);  // 19.53kHz, /4 of the CPU timer output?
	m_pit->out_handler<0>().set(FUNC(ngen386_state::pit_out0_w));  // RS232 channel B baud rate
	m_pit->set_clk<1>(14.7456_MHz_XTAL / 12);  // correct? - based on patent
	m_pit->out_handler<1>().set(FUNC(ngen386_state::pit_out1_w));  // RS232 channel A baud rate
	m_pit->set_clk<2>(14.7456_MHz_XTAL / 12);
	m_pit->out_handler<2>().set(FUNC(ngen386_state::pit_out2_w));

	AM9517A(config, m_dmac, 14.7456_MHz_XTAL / 3);  // NEC D8237A, divisor unknown
	m_dmac->out_hreq_callback().set(FUNC(ngen386_state::dma_hrq_changed));
	m_dmac->out_eop_callback().set(FUNC(ngen386_state::dma_eop_changed));
	m_dmac->in_memr_callback().set(FUNC(ngen386_state::dma_read_word));  // DMA is always 16-bit
	m_dmac->out_memw_callback().set(FUNC(ngen386_state::dma_write_word));
	m_dmac->out_dack_callback<0>().set(FUNC(ngen386_state::dack0_w));
	m_dmac->out_dack_callback<1>().set(FUNC(ngen386_state::dack1_w));
	m_dmac->out_dack_callback<2>().set(FUNC(ngen386_state::dack2_w));
	m_dmac->out_dack_callback<3>().set(FUNC(ngen386_state::dack3_w));
	m_dmac->in_ior_callback<0>().set(FUNC(ngen386_state::dma_0_dack_r));
	m_dmac->in_ior_callback<1>().set(FUNC(ngen386_state::dma_1_dack_r));
	m_dmac->in_ior_callback<2>().set(FUNC(ngen386_state::dma_2_dack_r));
	m_dmac->in_ior_callback<3>().set(FUNC(ngen386_state::dma_3_dack_r));
	m_dmac->out_iow_callback<0>().set(FUNC(ngen386_state::dma_0_dack_w));
	m_dmac->out_iow_callback<1>().set(FUNC(ngen386_state::dma_1_dack_w));
	m_dmac->out_iow_callback<2>().set(FUNC(ngen386_state::dma_2_dack_w));
	m_dmac->out_iow_callback<3>().set(FUNC(ngen386_state::dma_3_dack_w));

	// I/O board
	UPD7201(config, m_iouart, 0); // clocked by PIT channel 2?
	m_iouart->out_txda_callback().set("rs232_a", FUNC(rs232_port_device::write_txd));
	m_iouart->out_txdb_callback().set("rs232_b", FUNC(rs232_port_device::write_txd));
	m_iouart->out_dtra_callback().set("rs232_a", FUNC(rs232_port_device::write_dtr));
	m_iouart->out_dtrb_callback().set("rs232_b", FUNC(rs232_port_device::write_dtr));
	m_iouart->out_rtsa_callback().set("rs232_a", FUNC(rs232_port_device::write_rts));
	m_iouart->out_rtsb_callback().set("rs232_b", FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232a(RS232_PORT(config, "rs232_a", default_rs232_devices, nullptr));
	rs232a.rxd_handler().set(m_iouart, FUNC(upd7201_device::rxa_w));
	rs232a.cts_handler().set(m_iouart, FUNC(upd7201_device::ctsa_w));
	rs232a.dcd_handler().set(m_iouart, FUNC(upd7201_device::dcda_w));
	rs232a.ri_handler().set(m_iouart, FUNC(upd7201_device::synca_w));

	rs232_port_device &rs232b(RS232_PORT(config, "rs232_b", default_rs232_devices, nullptr));
	rs232b.rxd_handler().set(m_iouart, FUNC(upd7201_device::rxb_w));
	rs232b.cts_handler().set(m_iouart, FUNC(upd7201_device::ctsb_w));
	rs232b.dcd_handler().set(m_iouart, FUNC(upd7201_device::dcdb_w));
	rs232b.ri_handler().set(m_iouart, FUNC(upd7201_device::syncb_w));

	// TODO: SCN2652 MPCC (not implemented), used for RS-422 cluster communications?

	// video board
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_size(720, 348);
	screen.set_visarea(0, 719, 0, 347);
	screen.set_refresh_hz(60);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	MC6845(config, m_crtc, 19980000 / 9);  // divisor unknown -- /9 gives 60Hz output, so likely correct
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(9);
	m_crtc->set_update_row_callback(FUNC(ngen386_state::crtc_update_row));

	// keyboard UART (patent says i8251 is used for keyboard communications, it is located on the video board)
	I8251(config, m_viduart, 0);  // main clock unknown, Rx/Tx clocks are 19.53kHz
//  m_viduart->txempty_handler().set("pic", FUNC(pic8259_device::ir4_w));
	m_viduart->txd_handler().set("keyboard", FUNC(rs232_port_device::write_txd));
	rs232_port_device &kbd(RS232_PORT(config, "keyboard", keyboard, "ngen"));
	kbd.rxd_handler().set(m_viduart, FUNC(i8251_device::write_rxd));

	CLOCK(config, "refresh_clock", 19200*16).signal_handler().set(FUNC(ngen386_state::timer_clk_out)); // should be 19530Hz

	// floppy disk / hard disk module (WD2797 FDC, WD1010 HDC, plus an 8253 timer for each)
	WD2797(config, m_fdc, 20_MHz_XTAL / 20);
	m_fdc->intrq_wr_callback().set(FUNC(ngen386_state::fdc_irq_w));
	//m_fdc->drq_wr_callback().set(m_i386cpu, FUNC(i80186_cpu_device_device::drq1_w));
	m_fdc->set_force_ready(true);

	PIT8253(config, m_fdc_timer, 0);
	m_fdc_timer->set_clk<0>(0);
	m_fdc_timer->out_handler<0>().set(m_pic, FUNC(pic8259_device::ir5_w));  // clocked on FDC data register access
	m_fdc_timer->set_clk<1>(20_MHz_XTAL / 20);
//  m_fdc_timer->out_handler<1>().set(m_pic, FUNC(pic8259_device::ir5_w));  // 1MHz
	m_fdc_timer->set_clk<2>(20_MHz_XTAL / 20);
//  m_fdc_timer->out_handler<2>().set(m_pic, FUNC(pic8259_device::ir5_w));

	// TODO: WD1010 HDC (not implemented), use WD2010 for now
	WD2010(config, m_hdc, 20_MHz_XTAL / 4);
	m_hdc->out_intrq_callback().set(m_pic, FUNC(pic8259_device::ir2_w));
	m_hdc->in_bcs_callback().set(FUNC(ngen386_state::hd_buffer_r));
	m_hdc->out_bcs_callback().set(FUNC(ngen386_state::hd_buffer_w));
	m_hdc->in_drdy_callback().set_constant(1);
	m_hdc->in_index_callback().set_constant(1);
	m_hdc->in_wf_callback().set_constant(1);
	m_hdc->in_tk000_callback().set_constant(1);
	m_hdc->in_sc_callback().set_constant(1);

	PIT8253(config, m_hdc_timer, 0);
	m_hdc_timer->set_clk<2>(20_MHz_XTAL / 10);  // 2MHz

	FLOPPY_CONNECTOR(config, "fdc:0", ngen_floppies, "525qd", floppy_image_device::default_mfm_floppy_formats);
	HARDDISK(config, "hard0", 0);
}

void ngen386_state::_386i(machine_config &config)
{
	ngen386(config);
	m_i386cpu->set_addrmap(AS_PROGRAM, &ngen386_state::ngen386i_mem);
}

ROM_START( ngen )
	ROM_REGION16_LE( 0x2000, "bios", 0)
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
	ROM_REGION32_LE( 0x2000, "bios", 0)
	ROM_LOAD16_BYTE( "72-168_fpc_386_cpu.bin",  0x000000, 0x001000, CRC(250a3b68) SHA1(49c070514bac264fa4892f284f7d2c852ae6605d) )
	ROM_LOAD16_BYTE( "72-167_fpc_386_cpu.bin",  0x000001, 0x001000, CRC(4010cc4e) SHA1(74a3024d605569056484d08b63f19fbf8eaf31c6) )

	ROM_REGION16_LE( 0x2000, "vram", ROMREGION_ERASE00 )
	ROM_REGION16_LE( 0x2000, "fontram", ROMREGION_ERASE00 )
ROM_END

ROM_START( 386i )
	ROM_REGION32_LE( 0x4000, "bios", 0)
	ROM_LOAD16_BYTE( "72-1561o_386i_cpu.bin",  0x000000, 0x002000, CRC(b5efd768) SHA1(8b250d47d9c6eb82e1afaeb2244d8c4134ecbc47) )
	ROM_LOAD16_BYTE( "72-1562e_386i_cpu.bin",  0x000001, 0x002000, CRC(002d0d3a) SHA1(31de8592999377db9251acbeff348390a2d2602a) )

	ROM_REGION16_LE( 0x2000, "vram", ROMREGION_ERASE00 )
	ROM_REGION16_LE( 0x2000, "fontram", ROMREGION_ERASE00 )

	ROM_REGION( 0x2000, "video", 0)
	ROM_LOAD( "72-1630_gc-104_vga.bin",  0x000000, 0x002000, CRC(4e4d8ebe) SHA1(50c96ccb4d0bd1beb2d1aee0d18b2c462d25fc8f) )
ROM_END

} // anonymous namespace


COMP( 1983, ngen,    0,    0, ngen,    ngen, ngen_state,    empty_init, "Convergent Technologies",  "NGEN CP-001", MACHINE_IS_SKELETON )
COMP( 1991, ngenb38, ngen, 0, ngen386, ngen, ngen386_state, empty_init, "Financial Products Corp.", "B28/38",      MACHINE_IS_SKELETON )
COMP( 1990, 386i,    ngen, 0, _386i,   ngen, ngen386_state, empty_init, "Convergent Technologies",  "386i",        MACHINE_IS_SKELETON )
