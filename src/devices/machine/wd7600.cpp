// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 *  Western Digital WD7600 PC system chipset
 *
 *  WD76C10 - system control
 *  WD76C20 - FDC, RTC, Bus interface
 *  WD76C30 - 1 parallel and 2 serial ports
 *
 *  Created on: 5/05/2014
 *
 *  TODO:  pretty much everything
 */

#include "machine/wd7600.h"

const device_type WD7600 = &device_creator<wd7600_device>;

#define LOG (1)

static MACHINE_CONFIG_FRAGMENT( wd7600 )
	MCFG_DEVICE_ADD("dma1", AM9517A, 0)
	MCFG_I8237_OUT_HREQ_CB(DEVWRITELINE("dma2", am9517a_device, dreq0_w))
	MCFG_I8237_OUT_EOP_CB(WRITELINE(wd7600_device, dma1_eop_w))
	MCFG_I8237_IN_MEMR_CB(READ8(wd7600_device, dma_read_byte))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(wd7600_device, dma_write_byte))
	MCFG_I8237_IN_IOR_0_CB(READ8(wd7600_device, dma1_ior0_r))
	MCFG_I8237_IN_IOR_1_CB(READ8(wd7600_device, dma1_ior1_r))
	MCFG_I8237_IN_IOR_2_CB(READ8(wd7600_device, dma1_ior2_r))
	MCFG_I8237_IN_IOR_3_CB(READ8(wd7600_device, dma1_ior3_r))
	MCFG_I8237_OUT_IOW_0_CB(WRITE8(wd7600_device, dma1_iow0_w))
	MCFG_I8237_OUT_IOW_1_CB(WRITE8(wd7600_device, dma1_iow1_w))
	MCFG_I8237_OUT_IOW_2_CB(WRITE8(wd7600_device, dma1_iow2_w))
	MCFG_I8237_OUT_IOW_3_CB(WRITE8(wd7600_device, dma1_iow3_w))
	MCFG_I8237_OUT_DACK_0_CB(WRITELINE(wd7600_device, dma1_dack0_w))
	MCFG_I8237_OUT_DACK_1_CB(WRITELINE(wd7600_device, dma1_dack1_w))
	MCFG_I8237_OUT_DACK_2_CB(WRITELINE(wd7600_device, dma1_dack2_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(wd7600_device, dma1_dack3_w))
	MCFG_DEVICE_ADD("dma2", AM9517A, 0)
	MCFG_I8237_OUT_HREQ_CB(WRITELINE(wd7600_device, dma2_hreq_w))
	MCFG_I8237_IN_MEMR_CB(READ8(wd7600_device, dma_read_word))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(wd7600_device, dma_write_word))
	MCFG_I8237_IN_IOR_1_CB(READ8(wd7600_device, dma2_ior1_r))
	MCFG_I8237_IN_IOR_2_CB(READ8(wd7600_device, dma2_ior2_r))
	MCFG_I8237_IN_IOR_3_CB(READ8(wd7600_device, dma2_ior3_r))
	MCFG_I8237_OUT_IOW_1_CB(WRITE8(wd7600_device, dma2_iow1_w))
	MCFG_I8237_OUT_IOW_2_CB(WRITE8(wd7600_device, dma2_iow2_w))
	MCFG_I8237_OUT_IOW_3_CB(WRITE8(wd7600_device, dma2_iow3_w))
	MCFG_I8237_OUT_DACK_0_CB(WRITELINE(wd7600_device, dma2_dack0_w))
	MCFG_I8237_OUT_DACK_1_CB(WRITELINE(wd7600_device, dma2_dack1_w))
	MCFG_I8237_OUT_DACK_2_CB(WRITELINE(wd7600_device, dma2_dack2_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(wd7600_device, dma2_dack3_w))
	MCFG_PIC8259_ADD("intc1", WRITELINE(wd7600_device, pic1_int_w), VCC, READ8(wd7600_device, pic1_slave_ack_r))
	MCFG_PIC8259_ADD("intc2", DEVWRITELINE("intc1", pic8259_device, ir2_w), GND, NULL)

	MCFG_DEVICE_ADD("ctc", PIT8254, 0)
	MCFG_PIT8253_CLK0(XTAL_14_31818MHz / 12)
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE("intc1", pic8259_device, ir0_w))
	MCFG_PIT8253_CLK1(XTAL_14_31818MHz / 12)
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(wd7600_device, ctc_out1_w))
	MCFG_PIT8253_CLK2(XTAL_14_31818MHz / 12)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(wd7600_device, ctc_out2_w))

	MCFG_DS12885_ADD("rtc")
	MCFG_MC146818_IRQ_HANDLER(WRITELINE(wd7600_device, rtc_irq_w))
	MCFG_MC146818_CENTURY_INDEX(0x32)
MACHINE_CONFIG_END

machine_config_constructor wd7600_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( wd7600 );
}

void wd7600_device::static_set_cputag(device_t &device, const char *tag)
{
	wd7600_device &chip = downcast<wd7600_device &>(device);
	chip.m_cputag = tag;
}

void wd7600_device::static_set_isatag(device_t &device, const char *tag)
{
	wd7600_device &chip = downcast<wd7600_device &>(device);
	chip.m_isatag = tag;
}

void wd7600_device::static_set_biostag(device_t &device, const char *tag)
{
	wd7600_device &chip = downcast<wd7600_device &>(device);
	chip.m_biostag = tag;
}

void wd7600_device::static_set_keybctag(device_t &device, const char *tag)
{
	wd7600_device &chip = downcast<wd7600_device &>(device);
	chip.m_keybctag = tag;
}

wd7600_device::wd7600_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, WD7600, "WD 7600 chipset", tag, owner, clock, "wd7600", __FILE__),
	m_read_ior(*this),
	m_write_iow(*this),
	m_write_tc(*this),
	m_write_hold(*this),
	m_write_nmi(*this),
	m_write_intr(*this),
	m_write_cpureset(*this),
	m_write_a20m(*this),
	m_write_spkr(*this),
	m_dma1(*this, "dma1"),
	m_dma2(*this, "dma2"),
	m_pic1(*this, "intc1"),
	m_pic2(*this, "intc2"),
	m_ctc(*this, "ctc"),
	m_rtc(*this, "rtc"),
	m_portb(0x0f),
	m_iochck(1),
	m_nmi_mask(1),
	m_alt_a20(0),
	m_ext_gatea20(0),
	m_kbrst(1),
	m_refresh_toggle(0),
	m_dma_eop(0),
	m_dma_high_byte(0xff),
	m_dma_channel(-1)
	{}


void wd7600_device::device_start()
{
	ram_device *ram_dev = machine().device<ram_device>(RAM_TAG);

	// make sure the ram device is already running
	if (!ram_dev->started())
		throw device_missing_dependencies();

	// resolve callbacks
	m_read_ior.resolve_safe(0);
	m_write_iow.resolve_safe();
	m_write_tc.resolve_safe();
	m_write_hold.resolve_safe();
	m_write_nmi.resolve_safe();
	m_write_intr.resolve_safe();
	m_write_cpureset.resolve_safe();
	m_write_a20m.resolve_safe();
	m_write_spkr.resolve_safe();

	device_t *cpu = machine().device(m_cputag);
	m_space = &cpu->memory().space(AS_PROGRAM);
	m_space_io = &cpu->memory().space(AS_IO);

	m_isa = machine().root_device().memregion(m_isatag)->base();
	m_bios = machine().root_device().memregion(m_biostag)->base();
	m_keybc = downcast<at_keyboard_controller_device *>(machine().device(m_keybctag));

	m_ram = ram_dev->pointer();
	UINT32 ram_size = ram_dev->size();

	// install base memory
	m_space->install_ram(0x000000, 0x09ffff, m_ram);
	m_space->install_ram(0x0d0000, 0x0effff, m_ram + 0xd0000);

	// install extended memory
	if (ram_size > 0x100000)
		m_space->install_ram(0x100000, ram_size - 1, m_ram + 0x100000);

	// install video BIOS (we should use the VGA BIOS at the beginning of the system BIOS ROM, but that gives a
	// blank display (but still runs))
	//m_space->install_rom(0x000c0000, 0x000cffff, m_bios + 0xe0000);
	m_space->install_rom(0x000c0000, 0x000cffff, m_isa);

	// install BIOS ROM at cpu inital pc
	m_space->install_rom(0x000f0000, 0x000fffff, m_bios + 0xf0000);
	m_space->install_rom(0xffff0000, 0xffffffff, m_bios + 0xf0000);

	// install i/o accesses
	m_space_io->install_readwrite_handler(0x0000, 0x000f, read8_delegate(FUNC(am9517a_device::read), &(*m_dma1)), write8_delegate(FUNC(am9517a_device::write), &(*m_dma1)), 0xffffffff);
	m_space_io->install_readwrite_handler(0x0020, 0x003f, read8_delegate(FUNC(pic8259_device::read), &(*m_pic1)), write8_delegate(FUNC(pic8259_device::write), &(*m_pic1)), 0x0000ffff);
	m_space_io->install_readwrite_handler(0x0040, 0x0043, read8_delegate(FUNC(pit8254_device::read), &(*m_ctc)), write8_delegate(FUNC(pit8254_device::write), &(*m_ctc)), 0xffffffff);
	m_space_io->install_readwrite_handler(0x0060, 0x0063, read8_delegate(FUNC(wd7600_device::keyb_data_r), this), write8_delegate(FUNC(wd7600_device::keyb_data_w), this), 0x000000ff);
	m_space_io->install_readwrite_handler(0x0060, 0x0063, read8_delegate(FUNC(wd7600_device::portb_r), this), write8_delegate(FUNC(wd7600_device::portb_w), this), 0x0000ff00);
	m_space_io->install_readwrite_handler(0x0064, 0x0067, read8_delegate(FUNC(wd7600_device::keyb_status_r), this), write8_delegate(FUNC(wd7600_device::keyb_cmd_w), this), 0x000000ff);
	m_space_io->install_readwrite_handler(0x0070, 0x007f, read8_delegate(FUNC(mc146818_device::read), &(*m_rtc)), write8_delegate(FUNC(wd7600_device::rtc_w), this), 0x0000ffff);
	m_space_io->install_readwrite_handler(0x0080, 0x009f, read8_delegate(FUNC(wd7600_device::dma_page_r), this), write8_delegate(FUNC(wd7600_device::dma_page_w), this), 0xffffffff);
	m_space_io->install_readwrite_handler(0x0090, 0x0093, read8_delegate(FUNC(wd7600_device::a20_reset_r), this), write8_delegate(FUNC(wd7600_device::a20_reset_w), this), 0x00ff0000);
	m_space_io->install_readwrite_handler(0x00a0, 0x00a3, read8_delegate(FUNC(pic8259_device::read), &(*m_pic2)), write8_delegate(FUNC(pic8259_device::write), &(*m_pic2)), 0x0000ffff);
	m_space_io->install_readwrite_handler(0x00c0, 0x00df, read8_delegate(FUNC(am9517a_device::read), &(*m_dma2)), write8_delegate(FUNC(am9517a_device::write), &(*m_dma2)), 0x00ff00ff);
	m_space_io->install_readwrite_handler(0x2070, 0x2073, read16_delegate(FUNC(wd7600_device::refresh_r), this), write16_delegate(FUNC(wd7600_device::refresh_w), this), 0xffff0000);
	m_space_io->install_readwrite_handler(0x2870, 0x2873, read16_delegate(FUNC(wd7600_device::chipsel_r), this), write16_delegate(FUNC(wd7600_device::chipsel_w), this), 0xffff0000);
	m_space_io->install_readwrite_handler(0x3870, 0x3873, read16_delegate(FUNC(wd7600_device::mem_ctrl_r), this), write16_delegate(FUNC(wd7600_device::mem_ctrl_w), this), 0xffff0000);
	m_space_io->install_readwrite_handler(0x4870, 0x4873, read16_delegate(FUNC(wd7600_device::bank_01_start_r), this), write16_delegate(FUNC(wd7600_device::bank_01_start_w), this), 0xffff0000);
	m_space_io->install_readwrite_handler(0x5070, 0x5073, read16_delegate(FUNC(wd7600_device::bank_23_start_r), this), write16_delegate(FUNC(wd7600_device::bank_23_start_w), this), 0xffff0000);
	m_space_io->install_readwrite_handler(0x5870, 0x5873, read16_delegate(FUNC(wd7600_device::split_addr_r), this), write16_delegate(FUNC(wd7600_device::split_addr_w), this), 0xffff0000);
	m_space_io->install_readwrite_handler(0x9870, 0x9873, read16_delegate(FUNC(wd7600_device::diag_r), this), write16_delegate(FUNC(wd7600_device::diag_w), this), 0xffff0000);
}

void wd7600_device::device_reset()
{
	m_split_start = 0;
	m_chip_sel = 0;
	m_refresh_ctrl = 0;
	m_memory_ctrl = 0;
	m_diagnostic = 0xe080;

	for(auto & elem : m_bank_start)
		elem = 0;

	// initialize dma controller clocks
	m_dma1->set_unscaled_clock(clock());
	m_dma2->set_unscaled_clock(clock());
}


WRITE_LINE_MEMBER( wd7600_device::iochck_w )
{
	if (BIT(m_portb, 3) == 0)
	{
		if (m_iochck && state == 0)
		{
			// set channel check latch
			m_portb |= 1 << 6;
			nmi();
		}

		m_iochck = state;
	}
}

void wd7600_device::nmi()
{
	if (m_nmi_mask & BIT(m_portb, 6))
	{
		m_write_nmi(1);
		m_write_nmi(0);
	}
}

void wd7600_device::a20m()
{
	// TODO: ignore keyboard A20 signal if set in Diagnostic register (0x9872)
	m_write_a20m(m_alt_a20 | m_ext_gatea20);
}

void wd7600_device::keyboard_gatea20(int state)
{
	m_ext_gatea20 = state;
	a20m();
}

WRITE8_MEMBER( wd7600_device::rtc_w )
{
	if (offset == 0)
	{
		m_nmi_mask = !BIT(data, 7);
		data &= 0x7f;
	}

	m_rtc->write(space, offset, data);
}

WRITE_LINE_MEMBER( wd7600_device::rtc_irq_w )
{
	m_pic2->ir0_w(state ? 0 : 1); // inverted?
}

READ8_MEMBER( wd7600_device::pic1_slave_ack_r )
{
	if (offset == 2) // IRQ 2
		return m_pic2->acknowledge();

	return 0x00;
}

// Timer outputs
WRITE_LINE_MEMBER( wd7600_device::ctc_out1_w )
{
	m_refresh_toggle ^= state;
	m_portb = (m_portb & 0xef) | (m_refresh_toggle << 4);
}

WRITE_LINE_MEMBER( wd7600_device::ctc_out2_w )
{
	m_write_spkr(!(state));
	m_portb = (m_portb & 0xdf) | (state << 5);
}

// Keyboard
WRITE8_MEMBER( wd7600_device::keyb_data_w )
{
//  if(LOG) logerror("WD7600 '%s': keyboard data write %02x\n", tag(), data);
	m_keybc->data_w(space,0,data);
}

READ8_MEMBER( wd7600_device::keyb_data_r )
{
	UINT8 ret = m_keybc->data_r(space,0);
//  if(LOG) logerror("WD7600 '%s': keyboard data read %02x\n", tag(), ret);
	return ret;
}

WRITE8_MEMBER( wd7600_device::keyb_cmd_w )
{
//  if(LOG) logerror("WD7600 '%s': keyboard command %02x\n", tag(), data);
	m_keybc->command_w(space,0,data);
}

READ8_MEMBER( wd7600_device::keyb_status_r )
{
	return m_keybc->status_r(space,0);
}

READ8_MEMBER( wd7600_device::portb_r )
{
	return m_portb;
}

WRITE8_MEMBER( wd7600_device::portb_w )
{
	m_portb = (m_portb & 0xf0) | (data & 0x0f);

	// bit 5 forced to 1 if timer disabled
	if (!BIT(m_portb, 0))
		m_portb |= 1 << 5;

	m_ctc->write_gate2(BIT(m_portb, 0));

	m_write_spkr(!BIT(m_portb, 1));

	// clear channel check latch?
	if (BIT(m_portb, 3))
		m_portb &= 0xbf;
}

// DMA controllers
offs_t wd7600_device::page_offset()
{
	switch (m_dma_channel)
	{
		case 0: return (offs_t) m_dma_page[0x07] << 16;
		case 1: return (offs_t) m_dma_page[0x03] << 16;
		case 2: return (offs_t) m_dma_page[0x01] << 16;
		case 3: return (offs_t) m_dma_page[0x02] << 16;
		case 5: return (offs_t) m_dma_page[0x0b] << 16;
		case 6: return (offs_t) m_dma_page[0x09] << 16;
		case 7: return (offs_t) m_dma_page[0x0a] << 16;
	}

	// should never get here
	return 0xff0000;
}

READ8_MEMBER( wd7600_device::dma_read_byte )
{
	if (m_dma_channel == -1)
		return 0xff;

	return m_space->read_byte(page_offset() + offset);
}

WRITE8_MEMBER( wd7600_device::dma_write_byte )
{
	if (m_dma_channel == -1)
		return;

	m_space->write_byte(page_offset() + offset, data);
}

READ8_MEMBER( wd7600_device::dma_read_word )
{
	if (m_dma_channel == -1)
		return 0xff;

	UINT16 result = m_space->read_word(page_offset() + (offset << 1));
	m_dma_high_byte = result >> 8;

	return result;
}

WRITE8_MEMBER( wd7600_device::dma_write_word )
{
	if (m_dma_channel == -1)
		return;

	m_space->write_word(page_offset() + (offset << 1), (m_dma_high_byte << 8) | data);
}

WRITE_LINE_MEMBER( wd7600_device::dma2_dack0_w )
{
	m_dma1->hack_w(state ? 0 : 1); // inverted?
}

WRITE_LINE_MEMBER( wd7600_device::dma1_eop_w )
{
	m_dma_eop = state;
	if (m_dma_channel != -1)
		m_write_tc(m_dma_channel, state, 0xff);
}

void wd7600_device::set_dma_channel(int channel, bool state)
{
	if (!state)
	{
		m_dma_channel = channel;
		if (m_dma_eop)
			m_write_tc(channel, 1, 0xff);
	}
	else
	{
		if (m_dma_channel == channel)
		{
			m_dma_channel = -1;
			if (m_dma_eop)
				m_write_tc(channel, 0, 0xff);
		}
	}
}

WRITE_LINE_MEMBER( wd7600_device::gatea20_w )
{
	keyboard_gatea20(state);
}

WRITE_LINE_MEMBER( wd7600_device::kbrst_w )
{
	// convert to active low signal (gets inverted in at_keybc.c)
	state = (state == ASSERT_LINE ? 0 : 1);

	// detect transition
	if (m_kbrst == 1 && state == 0)
	{
		m_write_cpureset(1);
		m_write_cpureset(0);
	}

	m_kbrst = state;
}

WRITE8_MEMBER( wd7600_device::a20_reset_w )
{
	m_alt_a20 = BIT(data,1);
	a20m();
	// TODO: proper timing.  Reset occurs 128 cycles after changing to a 1, and lasts for 16 cycles
	if(BIT(data,0))
	{
		m_write_cpureset(1);
		m_write_cpureset(0);
		if(LOG) logerror("WD7600 '%s': System reset\n",tag());
	}
}

READ8_MEMBER( wd7600_device::a20_reset_r )
{
	UINT8 ret = 0;
	if(m_alt_a20)
		ret |= 0x02;
	return ret;
}

// port 0x2072 - Refresh Control, and serial/parallel port address select
READ16_MEMBER(wd7600_device::refresh_r)
{
	return m_refresh_ctrl;
}

WRITE16_MEMBER(wd7600_device::refresh_w)
{
	// TODO: select serial/parallel I/O port location
	m_refresh_ctrl = data;
	if(LOG) logerror("WD7600 '%s': Refresh Control write %04x\n",tag(),data);
}

// port 0x2872 - chip select
READ16_MEMBER(wd7600_device::chipsel_r)
{
	return m_chip_sel;
}

WRITE16_MEMBER(wd7600_device::chipsel_w)
{
	m_chip_sel = data;
	if(LOG) logerror("WD7600 '%s': Chip Select write %04x\n",tag(),data);
}

// port 0x3872 - Memory Control
READ16_MEMBER(wd7600_device::mem_ctrl_r)
{
	return m_memory_ctrl;
}

WRITE16_MEMBER(wd7600_device::mem_ctrl_w)
{
	m_memory_ctrl = data;
	if(LOG) logerror("WD7600 '%s': Memory Control write %04x\n",tag(),data);
}

// port 0x4872 - Bank 0 and 1 start address
READ16_MEMBER(wd7600_device::bank_01_start_r)
{
	return (m_bank_start[1] << 8) | m_bank_start[0];
}

WRITE16_MEMBER(wd7600_device::bank_01_start_w)
{
	if(ACCESSING_BITS_0_7)
	{
		m_bank_start[0] = data & 0xff;
		if(LOG) logerror("WD7600 '%s': Bank 0 start address %08x\n",tag(),m_bank_start[0] << 16);
	}
	if(ACCESSING_BITS_8_15)
	{
		m_bank_start[1] = (data & 0xff00) >> 8;
		if(LOG) logerror("WD7600 '%s': Bank 1 start address %08x\n",tag(),m_bank_start[1] << 16);
	}
}

// port 0x5072 - Bank 2 and 3 start address
READ16_MEMBER(wd7600_device::bank_23_start_r)
{
	return (m_bank_start[3] << 8) | m_bank_start[2];
}

WRITE16_MEMBER(wd7600_device::bank_23_start_w)
{
	if(ACCESSING_BITS_0_7)
	{
		m_bank_start[2] = data & 0xff;
		if(LOG) logerror("WD7600 '%s': Bank 2 start address %08x\n",tag(),m_bank_start[2] << 16);
	}
	if(ACCESSING_BITS_8_15)
	{
		m_bank_start[3] = (data & 0xff00) >> 8;
		if(LOG) logerror("WD7600 '%s': Bank 3 start address %08x\n",tag(),m_bank_start[3] << 16);
	}
}

// port 0x5872 - split starting address (used for BIOS shadowing)
READ16_MEMBER(wd7600_device::split_addr_r)
{
	return m_split_start;
}

WRITE16_MEMBER(wd7600_device::split_addr_w)
{
	m_split_start = data;
	if(LOG) logerror("WD7600 '%s': Split start address write %04x\n",tag(),data);
}

// port 0x9872 - Diagnostic
READ16_MEMBER(wd7600_device::diag_r)
{
	return m_diagnostic | 0xe080;
}

WRITE16_MEMBER(wd7600_device::diag_w)
{
	m_diagnostic = data;
	if(LOG) logerror("WD7600 '%s': Diagnostic write %04x\n",tag(),data);
}
