// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Chips & Technologies CS4031 chipset

    Chipset for 486 based PC/AT compatible systems. Consists of two
    individual chips:

    * F84031
        - DRAM controller
        - ISA-bus controller
        - VESA VL-BUS controller

    * F84035 (82C206 IPC core)
        - 2x 8257 DMA controller
        - 2x 8259 interrupt controller
        - 8254 timer
        - MC14818 RTC

    TODO:
        - No emulation of memory parity checks
        - Move IPC core to its own file so it can be shared with
          other chipsets

***************************************************************************/

#include "emu.h"
#include "machine/ram.h"
#include "machine/cs4031.h"


//**************************************************************************
//  MACROS/CONSTANTS
//**************************************************************************

#define LOG_REGISTER    1
#define LOG_MEMORY      1
#define LOG_IO          1
#define LOG_KEYBOARD    0


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type CS4031 = &device_creator<cs4031_device>;

const char* cs4031_device::m_register_names[] =
{
	/* 00 */ "RESERVED",
	/* 01 */ "DMA WAIT STATE CONTROL",
	/* 02 */ "RESERVED",
	/* 03 */ "RESERVED",
	/* 04 */ "RESERVED",
	/* 05 */ "ISA BUS COMMAND DELAY",
	/* 06 */ "ISA BUS WAIT STATES AND ADDRESS HOLD",
	/* 07 */ "ISA BUS CLOCK SELECTION",
	/* 08 */ "PERFORMANCE CONTROL",
	/* 09 */ "84035 MISC CONTROL",
	/* 0a */ "DMA CLOCK SELECTION",
	/* 0b */ "RESERVED",
	/* 0c */ "RESERVED",
	/* 0d */ "RESERVED",
	/* 0e */ "RESERVED",
	/* 0f */ "RESERVED",
	/* 10 */ "DRAM TIMING",
	/* 11 */ "DRAM SETUP",
	/* 12 */ "DRAM CONFIGURATION 0 AND 1",
	/* 13 */ "DRAM CONFIGURATION 2 AND 3",
	/* 14 */ "DRAM BLOCK 0 STARTING ADDRESS",
	/* 15 */ "DRAM BLOCK 1 STARTING ADDRESS",
	/* 16 */ "DRAM BLOCK 2 STARTING ADDRESS",
	/* 17 */ "DRAM BLOCK 3 STARTING ADDRESS",
	/* 18 */ "VIDEO AREA SHADOW AND LOCAL BUS CONTROL",
	/* 19 */ "DRAM SHADOW READ ENABLE",
	/* 1a */ "DRAM SHADOW WRITE ENABLE",
	/* 1b */ "ROMCS ENABLE",
	/* 1c */ "SOFT RESET AND GATEA20",
	/* 1d */ "RESERVED",
	/* 1e */ "RESERVED",
	/* 1f */ "RESERVED"
};

const float cs4031_device::m_dma_clock_divider[] =
{
	10, 8, 6, 0, 0, 0, 0, 0, 5, 4, 3, 2.5, 2, 1.5, 0, 0
};

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( cs4031 )
	MCFG_DEVICE_ADD("dma1", AM9517A, 0)
	MCFG_I8237_OUT_HREQ_CB(DEVWRITELINE("dma2", am9517a_device, dreq0_w))
	MCFG_I8237_OUT_EOP_CB(WRITELINE(cs4031_device, dma1_eop_w))
	MCFG_I8237_IN_MEMR_CB(READ8(cs4031_device, dma_read_byte))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(cs4031_device, dma_write_byte))
	MCFG_I8237_IN_IOR_0_CB(READ8(cs4031_device, dma1_ior0_r))
	MCFG_I8237_IN_IOR_1_CB(READ8(cs4031_device, dma1_ior1_r))
	MCFG_I8237_IN_IOR_2_CB(READ8(cs4031_device, dma1_ior2_r))
	MCFG_I8237_IN_IOR_3_CB(READ8(cs4031_device, dma1_ior3_r))
	MCFG_I8237_OUT_IOW_0_CB(WRITE8(cs4031_device, dma1_iow0_w))
	MCFG_I8237_OUT_IOW_1_CB(WRITE8(cs4031_device, dma1_iow1_w))
	MCFG_I8237_OUT_IOW_2_CB(WRITE8(cs4031_device, dma1_iow2_w))
	MCFG_I8237_OUT_IOW_3_CB(WRITE8(cs4031_device, dma1_iow3_w))
	MCFG_I8237_OUT_DACK_0_CB(WRITELINE(cs4031_device, dma1_dack0_w))
	MCFG_I8237_OUT_DACK_1_CB(WRITELINE(cs4031_device, dma1_dack1_w))
	MCFG_I8237_OUT_DACK_2_CB(WRITELINE(cs4031_device, dma1_dack2_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(cs4031_device, dma1_dack3_w))
	MCFG_DEVICE_ADD("dma2", AM9517A, 0)
	MCFG_I8237_OUT_HREQ_CB(WRITELINE(cs4031_device, dma2_hreq_w))
	MCFG_I8237_IN_MEMR_CB(READ8(cs4031_device, dma_read_word))
	MCFG_I8237_OUT_MEMW_CB(WRITE8(cs4031_device, dma_write_word))
	MCFG_I8237_IN_IOR_1_CB(READ8(cs4031_device, dma2_ior1_r))
	MCFG_I8237_IN_IOR_2_CB(READ8(cs4031_device, dma2_ior2_r))
	MCFG_I8237_IN_IOR_3_CB(READ8(cs4031_device, dma2_ior3_r))
	MCFG_I8237_OUT_IOW_1_CB(WRITE8(cs4031_device, dma2_iow1_w))
	MCFG_I8237_OUT_IOW_2_CB(WRITE8(cs4031_device, dma2_iow2_w))
	MCFG_I8237_OUT_IOW_3_CB(WRITE8(cs4031_device, dma2_iow3_w))
	MCFG_I8237_OUT_DACK_0_CB(WRITELINE(cs4031_device, dma2_dack0_w))
	MCFG_I8237_OUT_DACK_1_CB(WRITELINE(cs4031_device, dma2_dack1_w))
	MCFG_I8237_OUT_DACK_2_CB(WRITELINE(cs4031_device, dma2_dack2_w))
	MCFG_I8237_OUT_DACK_3_CB(WRITELINE(cs4031_device, dma2_dack3_w))
	MCFG_PIC8259_ADD("intc1", WRITELINE(cs4031_device, intc1_int_w), VCC, READ8(cs4031_device, intc1_slave_ack_r))
	MCFG_PIC8259_ADD("intc2", DEVWRITELINE("intc1", pic8259_device, ir2_w), GND, NULL)

	MCFG_DEVICE_ADD("ctc", PIT8254, 0)
	MCFG_PIT8253_CLK0(XTAL_14_31818MHz / 12)
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE("intc1", pic8259_device, ir0_w))
	MCFG_PIT8253_CLK1(XTAL_14_31818MHz / 12)
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(cs4031_device, ctc_out1_w))
	MCFG_PIT8253_CLK2(XTAL_14_31818MHz / 12)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(cs4031_device, ctc_out2_w))

	MCFG_DS12885_ADD("rtc")
	MCFG_MC146818_IRQ_HANDLER(WRITELINE(cs4031_device, rtc_irq_w))
	MCFG_MC146818_CENTURY_INDEX(0x32)
MACHINE_CONFIG_END

machine_config_constructor cs4031_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( cs4031 );
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cs4031_device - constructor
//-------------------------------------------------

cs4031_device::cs4031_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, CS4031, "CS4031", tag, owner, clock, "cs4031", __FILE__),
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
	m_intc1(*this, "intc1"),
	m_intc2(*this, "intc2"),
	m_ctc(*this, "ctc"),
	m_rtc(*this, "rtc"),
	m_dma_eop(0),
	m_dma_high_byte(0xff),
	m_dma_channel(-1),
	m_portb(0x0f),
	m_refresh_toggle(0),
	m_iochck(1),
	m_nmi_mask(1),
	m_cpureset(0),
	m_kbrst(1),
	m_ext_gatea20(0),
	m_fast_gatea20(0),
	m_emu_gatea20(0),
	m_keybc_d1_written(false),
	m_keybc_data_blocked(false),
	m_address(0),
	m_address_valid(false)
{
}

void cs4031_device::static_set_cputag(device_t &device, std::string tag)
{
	cs4031_device &cs4031 = downcast<cs4031_device &>(device);
	cs4031.m_cputag = tag;
}

void cs4031_device::static_set_isatag(device_t &device, std::string tag)
{
	cs4031_device &cs4031 = downcast<cs4031_device &>(device);
	cs4031.m_isatag = tag;
}

void cs4031_device::static_set_biostag(device_t &device, std::string tag)
{
	cs4031_device &cs4031 = downcast<cs4031_device &>(device);
	cs4031.m_biostag = tag;
}

void cs4031_device::static_set_keybctag(device_t &device, std::string tag)
{
	cs4031_device &cs4031 = downcast<cs4031_device &>(device);
	cs4031.m_keybctag = tag;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cs4031_device::device_start()
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

	// register for state saving
	save_item(NAME(m_dma_eop));
	save_item(NAME(m_dma_page));
	save_item(NAME(m_dma_high_byte));
	save_item(NAME(m_dma_channel));
	save_item(NAME(m_portb));
	save_item(NAME(m_refresh_toggle));
	save_item(NAME(m_iochck));
	save_item(NAME(m_nmi_mask));
	save_item(NAME(m_cpureset));
	save_item(NAME(m_kbrst));
	save_item(NAME(m_ext_gatea20));
	save_item(NAME(m_fast_gatea20));
	save_item(NAME(m_address));
	save_item(NAME(m_address_valid));
	save_item(NAME(m_registers));

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

	// install extended memory
	if (ram_size > 0x100000)
		m_space->install_ram(0x100000, ram_size - 1, m_ram + 0x100000);

	// install bios rom at cpu inital pc
	m_space->install_rom(0xffff0000, 0xffffffff, m_bios + 0xf0000);

	// install i/o accesses
	m_space_io->install_readwrite_handler(0x0000, 0x000f, read8_delegate(FUNC(am9517a_device::read), &(*m_dma1)), write8_delegate(FUNC(am9517a_device::write), &(*m_dma1)), 0xffffffff);
	m_space_io->install_readwrite_handler(0x0020, 0x0023, read8_delegate(FUNC(pic8259_device::read), &(*m_intc1)), write8_delegate(FUNC(pic8259_device::write), &(*m_intc1)), 0x0000ffff);
	m_space_io->install_write_handler(0x0020, 0x0023, write8_delegate(FUNC(cs4031_device::config_address_w), this), 0x00ff0000);
	m_space_io->install_readwrite_handler(0x0020, 0x0023, read8_delegate(FUNC(cs4031_device::config_data_r), this), write8_delegate(FUNC(cs4031_device::config_data_w), this), 0xff000000);
	m_space_io->install_readwrite_handler(0x0040, 0x0043, read8_delegate(FUNC(pit8254_device::read), &(*m_ctc)), write8_delegate(FUNC(pit8254_device::write), &(*m_ctc)), 0xffffffff);
	m_space_io->install_readwrite_handler(0x0060, 0x0063, read8_delegate(FUNC(cs4031_device::keyb_data_r), this), write8_delegate(FUNC(cs4031_device::keyb_data_w), this), 0x000000ff);
	m_space_io->install_readwrite_handler(0x0060, 0x0063, read8_delegate(FUNC(cs4031_device::portb_r), this), write8_delegate(FUNC(cs4031_device::portb_w), this), 0x0000ff00);
	m_space_io->install_readwrite_handler(0x0064, 0x0067, read8_delegate(FUNC(cs4031_device::keyb_status_r), this), write8_delegate(FUNC(cs4031_device::keyb_command_w), this), 0x000000ff);
	m_space_io->install_readwrite_handler(0x0070, 0x0073, read8_delegate(FUNC(mc146818_device::read), &(*m_rtc)), write8_delegate(FUNC(cs4031_device::rtc_w), this), 0x0000ffff);
	m_space_io->install_readwrite_handler(0x0080, 0x008f, read8_delegate(FUNC(cs4031_device::dma_page_r), this), write8_delegate(FUNC(cs4031_device::dma_page_w), this), 0xffffffff);
	m_space_io->install_readwrite_handler(0x0090, 0x0093, read8_delegate(FUNC(cs4031_device::sysctrl_r), this), write8_delegate(FUNC(cs4031_device::sysctrl_w), this), 0x00ff0000);
	m_space_io->install_readwrite_handler(0x00a0, 0x00a3, read8_delegate(FUNC(pic8259_device::read), &(*m_intc2)), write8_delegate(FUNC(pic8259_device::write), &(*m_intc2)), 0x0000ffff);
	m_space_io->install_readwrite_handler(0x00c0, 0x00df, read8_delegate(FUNC(cs4031_device::dma2_r),this), write8_delegate(FUNC(cs4031_device::dma2_w),this), 0xffffffff);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cs4031_device::device_reset()
{
	// setup default values
	memset(&m_registers, 0x00, sizeof(m_registers));
	m_registers[ROMCS] = 0x60;

	// update rom/ram regions below 1mb
	update_read_regions();
	update_write_regions();

	// initialize dma controller clocks
	update_dma_clock();
}

//-------------------------------------------------
//  device_reset_after_children
//-------------------------------------------------

void cs4031_device::device_reset_after_children()
{
	// timer 2 default state
	m_ctc->write_gate2(1);
}


//**************************************************************************
//  DMA CONTROLLER
//**************************************************************************

offs_t cs4031_device::page_offset()
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

READ8_MEMBER( cs4031_device::dma_read_byte )
{
	if (m_dma_channel == -1)
		return 0xff;

	return m_space->read_byte(page_offset() + offset);
}

WRITE8_MEMBER( cs4031_device::dma_write_byte )
{
	if (m_dma_channel == -1)
		return;

	m_space->write_byte(page_offset() + offset, data);
}

READ8_MEMBER( cs4031_device::dma_read_word )
{
	if (m_dma_channel == -1)
		return 0xff;

	UINT16 result = m_space->read_word(page_offset() + (offset << 1));
	m_dma_high_byte = result >> 8;

	return result;
}

WRITE8_MEMBER( cs4031_device::dma_write_word )
{
	if (m_dma_channel == -1)
		return;

	m_space->write_word(page_offset() + (offset << 1), (m_dma_high_byte << 8) | data);
}

WRITE_LINE_MEMBER( cs4031_device::dma2_dack0_w )
{
	m_dma1->hack_w(state ? 0 : 1); // inverted?
}

WRITE_LINE_MEMBER( cs4031_device::dma1_eop_w )
{
	m_dma_eop = state;
	if (m_dma_channel != -1)
		m_write_tc(m_dma_channel, state, 0xff);
}

void cs4031_device::set_dma_channel(int channel, bool state)
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

void cs4031_device::update_dma_clock()
{
	if (m_dma_clock_divider[m_registers[DMA_CLOCK] & 0x0f] != 0)
	{
		UINT32 dma_clock = clock() / m_dma_clock_divider[m_registers[DMA_CLOCK] & 0x0f];

		if (!BIT(m_registers[DMA_WAIT_STATE], 0))
			dma_clock /= 2;

		logerror("cs4031_device::update_dma_clock: dma clock is now %u\n", dma_clock);

		m_dma1->set_unscaled_clock(dma_clock);
		m_dma2->set_unscaled_clock(dma_clock);
	}
}


//**************************************************************************
//  INTERRUPTS
//**************************************************************************

/*
    Check NMI sources and generate NMI if needed

    Not emulated here: Parity check NMI
 */
void cs4031_device::nmi()
{
	if (m_nmi_mask & BIT(m_portb, 6))
	{
		m_write_nmi(1);
		m_write_nmi(0);
	}
}

READ8_MEMBER( cs4031_device::intc1_slave_ack_r )
{
	if (offset == 2) // IRQ 2
		return m_intc2->acknowledge();

	return 0x00;
}

WRITE_LINE_MEMBER( cs4031_device::rtc_irq_w )
{
	m_intc2->ir0_w(state ? 0 : 1); // inverted?
}

WRITE_LINE_MEMBER( cs4031_device::iochck_w )
{
	if (LOG_IO)
		logerror("cs4031_device::iochck_w: %u\n", state);

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


//**************************************************************************
//  TIMER
//**************************************************************************

WRITE_LINE_MEMBER( cs4031_device::ctc_out1_w )
{
	m_refresh_toggle ^= state;
	m_portb = (m_portb & 0xef) | (m_refresh_toggle << 4);
}

WRITE_LINE_MEMBER( cs4031_device::ctc_out2_w )
{
	m_write_spkr(!(state & BIT(m_portb, 1)));
	m_portb = (m_portb & 0xdf) | (state << 5);
}


//**************************************************************************
//  CHIPSET CONFIGURATION
//**************************************************************************

WRITE8_MEMBER( cs4031_device::config_address_w )
{
	m_address = data;
	m_address_valid = (m_address < 0x20) ? true : false;
}

READ8_MEMBER( cs4031_device::config_data_r )
{
	UINT8 result = 0xff;

	if (m_address_valid)
	{
		if (LOG_REGISTER)
			logerror("cs4031_device: read %s = %02x\n", m_register_names[m_address], m_registers[m_address]);

		result = m_registers[m_address];
	}

	// after a read the selected address needs to be reset
	m_address_valid = false;

	return result;
}

WRITE8_MEMBER( cs4031_device::config_data_w )
{
	if (m_address_valid)
	{
		if (LOG_REGISTER)
			logerror("cs4031_device: write %s = %02x\n", m_register_names[m_address], data);

		// update register with new data
		m_registers[m_address] = data;

		// execute command
		switch (m_address)
		{
		case DMA_WAIT_STATE:
			update_dma_clock();
			break;

		case 0x05: break;
		case 0x06: break;
		case 0x07: break;
		case 0x08: break;
		case 0x09: break;

		case DMA_CLOCK:
			update_dma_clock();
			break;

		case 0x10: break;
		case 0x11: break;
		case 0x12: break;
		case 0x13: break;
		case 0x14: break;
		case 0x15: break;
		case 0x16: break;
		case 0x17: break;
		case 0x18: break;

		case SHADOW_READ:
			update_read_regions();
			break;

		case SHADOW_WRITE:
			update_write_regions();
			break;

		case ROMCS:
			update_read_regions();
			update_write_regions();
			break;

		case SOFT_RESET_AND_GATEA20:
			a20m();
			break;
		}
	}

	// after a write the selected address needs to be reset
	m_address_valid = false;
}


//**************************************************************************
//  MEMORY MAPPER
//**************************************************************************

void cs4031_device::update_read_region(int index, const char *region, offs_t start, offs_t end)
{
	if (!BIT(m_registers[SHADOW_READ], index) && BIT(m_registers[ROMCS], index))
	{
		if (LOG_MEMORY)
			logerror("ROM read from %x to %x\n", start, end);

		m_space->install_read_bank(start, end, region);
		machine().root_device().membank(region)->set_base(m_bios + start);
	}
	else if (!BIT(m_registers[SHADOW_READ], index) && !BIT(m_registers[ROMCS], index))
	{
		if (LOG_MEMORY)
			logerror("ISA read from %x to %x\n", start, end);

		m_space->install_read_bank(start, end, region);
		machine().root_device().membank(region)->set_base(m_isa + start - 0xc0000);
	}
	else if (BIT(m_registers[SHADOW_READ], index))
	{
		if (LOG_MEMORY)
			logerror("RAM read from %x to %x\n", start, end);

		m_space->install_read_bank(start, end, region);
		machine().root_device().membank(region)->set_base(m_ram + start);
	}
	else
	{
		if (LOG_MEMORY)
			logerror("NOP read from %x to %x\n", start, end);

		m_space->nop_read(start, end);
	}
}

void cs4031_device::update_write_region(int index, const char *region, offs_t start, offs_t end)
{
	if (!BIT(m_registers[SHADOW_WRITE], index) && BIT(m_registers[ROMCS], index) && BIT(m_registers[ROMCS], 7))
	{
		if (LOG_MEMORY)
			logerror("ROM write from %x to %x\n", start, end);

		m_space->install_write_bank(start, end, region);
		machine().root_device().membank(region)->set_base(m_bios + start);
	}
	else if (!BIT(m_registers[SHADOW_WRITE], index) && !BIT(m_registers[ROMCS], index))
	{
		if (LOG_MEMORY)
			logerror("ISA write from %x to %x\n", start, end);

		m_space->install_write_bank(start, end, region);
		machine().root_device().membank(region)->set_base(m_isa + start - 0xc0000);
	}
	else if (BIT(m_registers[SHADOW_WRITE], index))
	{
		if (LOG_MEMORY)
			logerror("RAM write from %x to %x\n", start, end);

		m_space->install_write_bank(start, end, region);
		machine().root_device().membank(region)->set_base(m_ram + start);
	}
	else
	{
		if (LOG_MEMORY)
			logerror("NOP write from %x to %x\n", start, end);

		m_space->nop_write(start, end);
	}
}

void cs4031_device::update_read_regions()
{
	update_read_region(0, "read_c0000", 0xc0000, 0xc3fff);
	update_read_region(1, "read_c4000", 0xc4000, 0xc7fff);
	update_read_region(2, "read_c8000", 0xc8000, 0xcbfff);
	update_read_region(3, "read_cc000", 0xcc000, 0xcffff);
	update_read_region(4, "read_d0000", 0xd0000, 0xdffff);
	update_read_region(5, "read_e0000", 0xe0000, 0xeffff);
	update_read_region(6, "read_f0000", 0xf0000, 0xfffff);
}

void cs4031_device::update_write_regions()
{
	update_write_region(0, "write_c0000", 0xc0000, 0xc3fff);
	update_write_region(1, "write_c4000", 0xc4000, 0xc7fff);
	update_write_region(2, "write_c8000", 0xc8000, 0xcbfff);
	update_write_region(3, "write_cc000", 0xcc000, 0xcffff);
	update_write_region(4, "write_d0000", 0xd0000, 0xdffff);
	update_write_region(5, "write_e0000", 0xe0000, 0xeffff);
	update_write_region(6, "write_f0000", 0xf0000, 0xfffff);
}


//**************************************************************************
//  KEYBOARD / 8042
//**************************************************************************

void cs4031_device::a20m()
{
	// external signal is ignored when emulation is on
	if (BIT(m_registers[SOFT_RESET_AND_GATEA20], 5))
		m_write_a20m(m_fast_gatea20 | m_emu_gatea20);
	else
		m_write_a20m(m_fast_gatea20 | m_ext_gatea20);
}

void cs4031_device::emulated_kbreset(int state)
{
	if (BIT(m_registers[SOFT_RESET_AND_GATEA20], 4))
	{
		// kbreset (input) is active low
		// cpureset (output) is active high
		m_write_cpureset(!state);
	}
}

void cs4031_device::emulated_gatea20(int state)
{
	if (BIT(m_registers[SOFT_RESET_AND_GATEA20], 5))
	{
		m_emu_gatea20 = state;
		a20m();
	}
}

void cs4031_device::fast_gatea20(int state)
{
	m_fast_gatea20 = state;
	a20m();
}

void cs4031_device::keyboard_gatea20(int state)
{
	m_ext_gatea20 = state;
	a20m();
}

READ8_MEMBER( cs4031_device::keyb_status_r )
{
	if (LOG_KEYBOARD)
		logerror("cs4031_device::keyb_status_r\n");

	return m_keybc->status_r(space, 0);
}

WRITE8_MEMBER( cs4031_device::keyb_command_blocked_w )
{
	// command is optionally blocked
	if (!BIT(m_registers[SOFT_RESET_AND_GATEA20], 7))
		m_keybc->command_w(space, 0, data);
}

WRITE8_MEMBER( cs4031_device::keyb_command_w )
{
	if (LOG_KEYBOARD)
		logerror("cs4031_device::keyb_command_w: %02x\n", data);

	m_keybc_d1_written = false;

	switch (data)
	{
	// self-test
	case 0xaa:
		emulated_kbreset(1);
		emulated_gatea20(1);

		// self-test is never blocked
		m_keybc->command_w(space, 0, data);
		break;

	case 0xd1:
		m_keybc_d1_written = true;
		keyb_command_blocked_w(space, 0, data);
		break;

	case 0xf0:
	case 0xf1:
	case 0xf2:
	case 0xf4:
	case 0xf5:
	case 0xf6:
	case 0xf8:
	case 0xf9:
	case 0xfa:
	case 0xfc:
	case 0xfd:
	case 0xfe:
		// toggle keyboard reset?
		if (!BIT(data, 0))
		{
			emulated_kbreset(0);
			emulated_kbreset(1);
		}

		// toggle gatea20?
		if (!BIT(data, 1))
		{
			emulated_gatea20(0);
			emulated_gatea20(1);
		}

		keyb_command_blocked_w(space, 0, data);

		break;

	case 0xff:
		// last data write was blocked?
		if (m_keybc_data_blocked)
		{
			m_keybc_data_blocked = false;
			keyb_command_blocked_w(space, 0, data);
		}
		else
			m_keybc->command_w(space, 0, data);

		break;

	// everything else goes directly to the keyboard controller
	default:
		m_keybc->command_w(space, 0, data);
		break;
	}
}

READ8_MEMBER( cs4031_device::keyb_data_r )
{
	if (LOG_KEYBOARD)
		logerror("cs4031_device::keyb_data_r\n");

	return m_keybc->data_r(space, 0);
}

WRITE8_MEMBER( cs4031_device::keyb_data_w )
{
	if (LOG_KEYBOARD)
		logerror("cs4031_device::keyb_data_w: %02x\n", data);

	// data is blocked only for d1 command
	if (BIT(m_registers[SOFT_RESET_AND_GATEA20], 7) && m_keybc_d1_written)
	{
		m_keybc_data_blocked = true;
		emulated_kbreset(BIT(data, 0));
		emulated_gatea20(BIT(data, 1));
	}
	else
	{
		m_keybc_data_blocked = false;
		m_keybc->data_w(space, 0, data);
	}
}

WRITE_LINE_MEMBER( cs4031_device::gatea20_w )
{
	if (LOG_KEYBOARD)
		logerror("cs4031_device::gatea20_w: %u\n", state);

	keyboard_gatea20(state);
}

WRITE_LINE_MEMBER( cs4031_device::kbrst_w )
{
	if (LOG_KEYBOARD)
		logerror("cs4031_device::kbrst_w: %u\n", state);

	// convert to active low signal (gets inverted in at_keybc.c)
	state = (state == ASSERT_LINE ? 0 : 1);

	// external kbreset is ignored when emulation enabled
	if (!BIT(m_registers[SOFT_RESET_AND_GATEA20], 4))
	{
		// detect transition
		if (m_kbrst == 1 && state == 0)
		{
			m_write_cpureset(1);
			m_write_cpureset(0);
		}
	}

	m_kbrst = state;
}

/*
    Fast CPU reset and Gate A20

    0 - Fast CPU reset
    1 - Fast Gate A20

 */
WRITE8_MEMBER( cs4031_device::sysctrl_w )
{
	if (LOG_IO)
		logerror("cs4031_device::sysctrl_w: %u\n", data);

	fast_gatea20(BIT(data, 1));

	if (m_cpureset == 0 && BIT(data, 0))
	{
		// pulse reset line
		m_write_cpureset(1);
		m_write_cpureset(0);
	}

	m_cpureset = BIT(data, 0);
}

READ8_MEMBER( cs4031_device::sysctrl_r )
{
	UINT8 result = 0; // reserved bits read as 0?

	result |= m_cpureset << 0;
	result |= m_fast_gatea20 << 1;

	if (LOG_IO)
		logerror("cs4031_device::sysctrl_r: %u\n", result);

	return result;
}


//**************************************************************************
//  MISCELLANEOUS
//**************************************************************************

/*
    "Port B" - AT-compatible port with miscellaneous information

    0 - Timer 2 gate (rw)
    1 - Speaker data (rw)
    2 - Enable parity check (rw) [not emulated]
    3 - Enable IOCHECK (rw)
    4 - Refresh detect (r)
    5 - Timer 2 output (r)
    6 - Channel check latch (r)
    7 - Parity check latch (r) [not emulated]
*/

READ8_MEMBER( cs4031_device::portb_r )
{
	if (0)
		logerror("cs4031_device::portb_r: %02x\n", m_portb);

	return m_portb;
}

WRITE8_MEMBER( cs4031_device::portb_w )
{
	if (0)
		logerror("cs4031_device::portb_w: %02x\n", data);

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

/*
    NMI mask and RTC address

    7   - NMI mask
    6:0 - RTC address
 */
WRITE8_MEMBER( cs4031_device::rtc_w )
{
	if (0)
		logerror("cs4031_device::rtc_w: %02x\n", data);

	if (offset == 0)
	{
		m_nmi_mask = !BIT(data, 7);
		data &= 0x7f;
	}

	m_rtc->write(space, offset, data);
}
