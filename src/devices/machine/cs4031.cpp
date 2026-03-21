// license:BSD-3-Clause
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
        - MC146818 RTC

    TODO:
        - No emulation of memory parity checks
        - Move IPC core to its own file so it can be shared with
          other chipsets

***************************************************************************/

#include "emu.h"
#include "cs4031.h"

#define LOG_REGISTER    (1U << 1)
#define LOG_MEMORY      (1U << 2)
#define LOG_IO          (1U << 3)
#define LOG_KEYBOARD    (1U << 4)

#define VERBOSE (LOG_REGISTER | LOG_MEMORY | LOG_IO /*| LOG_KEYBOARD*/)
#include "logmacro.h"

#define LOGREGISTER(...)    LOGMASKED(LOG_REGISTER, __VA_ARGS__)
#define LOGMEMORY(...)      LOGMASKED(LOG_MEMORY,   __VA_ARGS__)
#define LOGIO(...)          LOGMASKED(LOG_IO,       __VA_ARGS__)
#define LOGKEYBOARD(...)    LOGMASKED(LOG_KEYBOARD, __VA_ARGS__)


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CS4031, cs4031_device, "cs4031", "CS4031")

const char* const cs4031_device::m_register_names[] =
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
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void cs4031_device::device_add_mconfig(machine_config &config)
{
	AM9517A(config, m_dma1, 0);
	m_dma1->out_hreq_callback().set(m_dma2, FUNC(am9517a_device::dreq0_w));
	m_dma1->out_eop_callback().set(FUNC(cs4031_device::dma1_eop_w));
	m_dma1->in_memr_callback().set(FUNC(cs4031_device::dma_read_byte));
	m_dma1->out_memw_callback().set(FUNC(cs4031_device::dma_write_byte));
	m_dma1->in_ior_callback<0>().set(FUNC(cs4031_device::dma1_ior0_r));
	m_dma1->in_ior_callback<1>().set(FUNC(cs4031_device::dma1_ior1_r));
	m_dma1->in_ior_callback<2>().set(FUNC(cs4031_device::dma1_ior2_r));
	m_dma1->in_ior_callback<3>().set(FUNC(cs4031_device::dma1_ior3_r));
	m_dma1->out_iow_callback<0>().set(FUNC(cs4031_device::dma1_iow0_w));
	m_dma1->out_iow_callback<1>().set(FUNC(cs4031_device::dma1_iow1_w));
	m_dma1->out_iow_callback<2>().set(FUNC(cs4031_device::dma1_iow2_w));
	m_dma1->out_iow_callback<3>().set(FUNC(cs4031_device::dma1_iow3_w));
	m_dma1->out_dack_callback<0>().set(FUNC(cs4031_device::dma1_dack0_w));
	m_dma1->out_dack_callback<1>().set(FUNC(cs4031_device::dma1_dack1_w));
	m_dma1->out_dack_callback<2>().set(FUNC(cs4031_device::dma1_dack2_w));
	m_dma1->out_dack_callback<3>().set(FUNC(cs4031_device::dma1_dack3_w));

	AM9517A(config, m_dma2, 0);
	m_dma2->out_hreq_callback().set(FUNC(cs4031_device::dma2_hreq_w));
	m_dma2->in_memr_callback().set(FUNC(cs4031_device::dma_read_word));
	m_dma2->out_memw_callback().set(FUNC(cs4031_device::dma_write_word));
	m_dma2->in_ior_callback<1>().set(FUNC(cs4031_device::dma2_ior1_r));
	m_dma2->in_ior_callback<2>().set(FUNC(cs4031_device::dma2_ior2_r));
	m_dma2->in_ior_callback<3>().set(FUNC(cs4031_device::dma2_ior3_r));
	m_dma2->out_iow_callback<1>().set(FUNC(cs4031_device::dma2_iow1_w));
	m_dma2->out_iow_callback<2>().set(FUNC(cs4031_device::dma2_iow2_w));
	m_dma2->out_iow_callback<3>().set(FUNC(cs4031_device::dma2_iow3_w));
	m_dma2->out_dack_callback<0>().set(FUNC(cs4031_device::dma2_dack0_w));
	m_dma2->out_dack_callback<1>().set(FUNC(cs4031_device::dma2_dack1_w));
	m_dma2->out_dack_callback<2>().set(FUNC(cs4031_device::dma2_dack2_w));
	m_dma2->out_dack_callback<3>().set(FUNC(cs4031_device::dma2_dack3_w));

	PIC8259(config, m_intc1, 0);
	m_intc1->out_int_callback().set(FUNC(cs4031_device::intc1_int_w));
	m_intc1->in_sp_callback().set_constant(1);
	m_intc1->read_slave_ack_callback().set(FUNC(cs4031_device::intc1_slave_ack_r));

	PIC8259(config, m_intc2, 0);
	m_intc2->out_int_callback().set(m_intc1, FUNC(pic8259_device::ir2_w));
	m_intc2->in_sp_callback().set_constant(0);

	PIT8254(config, m_ctc, 0);
	m_ctc->set_clk<0>(XTAL(14'318'181) / 12.0);
	m_ctc->out_handler<0>().set(m_intc1, FUNC(pic8259_device::ir0_w));
	m_ctc->set_clk<1>(XTAL(14'318'181) / 12.0);
	m_ctc->out_handler<1>().set(FUNC(cs4031_device::ctc_out1_w));
	m_ctc->set_clk<2>(XTAL(14'318'181) / 12.0);
	m_ctc->out_handler<2>().set(FUNC(cs4031_device::ctc_out2_w));

	DS12885(config, m_rtc);
	m_rtc->irq().set(m_intc2, FUNC(pic8259_device::ir0_w));
	m_rtc->set_century_index(0x32);
}


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cs4031_device - constructor
//-------------------------------------------------

cs4031_device::cs4031_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, CS4031, tag, owner, clock),
	m_read_ior(*this, 0),
	m_write_iow(*this),
	m_write_tc(*this),
	m_write_hold(*this),
	m_write_nmi(*this),
	m_write_intr(*this),
	m_write_cpureset(*this),
	m_write_a20m(*this),
	m_write_spkr(*this),
	m_cpu(*this, finder_base::DUMMY_TAG),
	m_keybc(*this, finder_base::DUMMY_TAG),
	m_isa(*this, finder_base::DUMMY_TAG),
	m_bios(*this, finder_base::DUMMY_TAG),
	m_space(nullptr),
	m_space_io(nullptr),
	m_ram(nullptr),
	m_dma1(*this, "dma1"),
	m_dma2(*this, "dma2"),
	m_intc1(*this, "intc1"),
	m_intc2(*this, "intc2"),
	m_ctc(*this, "ctc"),
	m_rtc(*this, "rtc"),
	m_ram_dev(*this, finder_base::DUMMY_TAG),
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

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cs4031_device::device_start()
{
	// make sure the ram device is already running
	if (!m_ram_dev->started())
		throw device_missing_dependencies();

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
	save_item(NAME(m_emu_gatea20));
	save_item(NAME(m_address));
	save_item(NAME(m_address_valid));
	save_item(NAME(m_registers));

	m_space = &m_cpu->memory().space(AS_PROGRAM);
	m_space_io = &m_cpu->memory().space(AS_IO);

	m_ram = m_ram_dev->pointer();
	u32 ram_size = m_ram_dev->size();

	// install base memory
	m_space->install_ram(0x000000, 0x09ffff, m_ram);

	// install extended memory
	if (ram_size > 0x100000)
		m_space->install_ram(0x100000, ram_size - 1, m_ram + 0x100000);

	// install bios rom at cpu initial pc
	m_space->install_rom(0xffff0000, 0xffffffff, m_bios + 0xf0000);

	// install i/o accesses
	m_space_io->install_readwrite_handler(0x0000, 0x000f, read8sm_delegate(*m_dma1, FUNC(am9517a_device::read)), write8sm_delegate(*m_dma1, FUNC(am9517a_device::write)), 0xffffffff);
	m_space_io->install_readwrite_handler(0x0020, 0x0023, read8sm_delegate(*m_intc1, FUNC(pic8259_device::read)), write8sm_delegate(*m_intc1, FUNC(pic8259_device::write)), 0x0000ffff);
	m_space_io->install_write_handler(0x0020, 0x0023, write8smo_delegate(*this, FUNC(cs4031_device::config_address_w)), 0x00ff0000);
	m_space_io->install_readwrite_handler(0x0020, 0x0023, read8smo_delegate(*this, FUNC(cs4031_device::config_data_r)), write8smo_delegate(*this, FUNC(cs4031_device::config_data_w)), 0xff000000);
	m_space_io->install_readwrite_handler(0x0040, 0x0043, read8sm_delegate(*m_ctc, FUNC(pit8254_device::read)), write8sm_delegate(*m_ctc, FUNC(pit8254_device::write)), 0xffffffff);
	m_space_io->install_readwrite_handler(0x0060, 0x0063, read8smo_delegate(*this, FUNC(cs4031_device::keyb_data_r)), write8smo_delegate(*this, FUNC(cs4031_device::keyb_data_w)), 0x000000ff);
	m_space_io->install_readwrite_handler(0x0060, 0x0063, read8smo_delegate(*this, FUNC(cs4031_device::portb_r)), write8smo_delegate(*this, FUNC(cs4031_device::portb_w)), 0x0000ff00);
	m_space_io->install_readwrite_handler(0x0064, 0x0067, read8smo_delegate(*this, FUNC(cs4031_device::keyb_status_r)), write8smo_delegate(*this, FUNC(cs4031_device::keyb_command_w)), 0x000000ff);
	m_space_io->install_write_handler(0x0070, 0x0073, write8smo_delegate(*this, FUNC(cs4031_device::rtc_nmi_w)), 0x000000ff); // RTC address (84035) and NMI mask (84031) are both write-only
	m_space_io->install_readwrite_handler(0x0070, 0x0073, read8smo_delegate(*m_rtc, FUNC(mc146818_device::data_r)), write8smo_delegate(*m_rtc, FUNC(mc146818_device::data_w)), 0x0000ff00);
	m_space_io->install_readwrite_handler(0x0080, 0x008f, read8sm_delegate(*this, FUNC(cs4031_device::dma_page_r)), write8sm_delegate(*this, FUNC(cs4031_device::dma_page_w)), 0xffffffff);
	m_space_io->install_readwrite_handler(0x0090, 0x0093, read8smo_delegate(*this, FUNC(cs4031_device::sysctrl_r)), write8smo_delegate(*this, FUNC(cs4031_device::sysctrl_w)), 0x00ff0000);
	m_space_io->install_readwrite_handler(0x00a0, 0x00a3, read8sm_delegate(*m_intc2, FUNC(pic8259_device::read)), write8sm_delegate(*m_intc2, FUNC(pic8259_device::write)), 0x0000ffff);
	m_space_io->install_readwrite_handler(0x00c0, 0x00df, read8sm_delegate(*this, FUNC(cs4031_device::dma2_r)), write8sm_delegate(*this, FUNC(cs4031_device::dma2_w)), 0xffffffff);
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

u8 cs4031_device::dma_read_byte(offs_t offset)
{
	if (m_dma_channel == -1)
		return 0xff;

	return m_space->read_byte(page_offset() + offset);
}

void cs4031_device::dma_write_byte(offs_t offset, u8 data)
{
	if (m_dma_channel == -1)
		return;

	m_space->write_byte(page_offset() + offset, data);
}

u8 cs4031_device::dma_read_word(offs_t offset)
{
	if (m_dma_channel == -1)
		return 0xff;

	u16 result = m_space->read_word((page_offset() & 0xfe0000) | (offset << 1));
	m_dma_high_byte = result >> 8;

	return result;
}

void cs4031_device::dma_write_word(offs_t offset, u8 data)
{
	if (m_dma_channel == -1)
		return;

	m_space->write_word((page_offset() & 0xfe0000) | (offset << 1), (m_dma_high_byte << 8) | data);
}

void cs4031_device::dma2_dack0_w(int state)
{
	m_dma1->hack_w(state ? 0 : 1); // inverted?
}

void cs4031_device::dma1_eop_w(int state)
{
	m_dma_eop = state;
	if (m_dma_channel != -1)
		m_write_tc(m_dma_channel, state, 0xff);
}

void cs4031_device::set_dma_channel(int channel, bool state)
{
	//m_write_dack(channel, state);

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
		u32 dma_clock = clock() / m_dma_clock_divider[m_registers[DMA_CLOCK] & 0x0f];

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
void cs4031_device::trigger_nmi()
{
	if (m_nmi_mask & BIT(m_portb, 6))
	{
		m_write_nmi(1);
		m_write_nmi(0);
	}
}

u8 cs4031_device::intc1_slave_ack_r(offs_t offset)
{
	if (offset == 2) // IRQ 2
		return m_intc2->acknowledge();

	return 0x00;
}

void cs4031_device::iochck_w(int state)
{
	LOGIO("cs4031_device::iochck_w: %u\n", state);

	if (BIT(m_portb, 3) == 0)
	{
		if (m_iochck && state == 0)
		{
			// set channel check latch
			m_portb |= 1 << 6;
			trigger_nmi();
		}

		m_iochck = state;
	}
}


//**************************************************************************
//  TIMER
//**************************************************************************

void cs4031_device::ctc_out1_w(int state)
{
	m_refresh_toggle ^= state;
	m_portb = (m_portb & 0xef) | (m_refresh_toggle << 4);
}

void cs4031_device::ctc_out2_w(int state)
{
	m_write_spkr(!(state & BIT(m_portb, 1)));
	m_portb = (m_portb & 0xdf) | (state << 5);
}


//**************************************************************************
//  CHIPSET CONFIGURATION
//**************************************************************************

void cs4031_device::config_address_w(u8 data)
{
	m_address = data;
	m_address_valid = (m_address < 0x20) ? true : false;
}

u8 cs4031_device::config_data_r()
{
	u8 result = 0xff;

	if (m_address_valid)
	{
		LOGREGISTER("cs4031_device: read %s = %02x\n", m_register_names[m_address], m_registers[m_address]);

		result = m_registers[m_address];
	}

	// after a read the selected address needs to be reset
	m_address_valid = false;

	return result;
}

void cs4031_device::config_data_w(u8 data)
{
	if (m_address_valid)
	{
		LOGREGISTER("cs4031_device: write %s = %02x\n", m_register_names[m_address], data);

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
			update_a20m();
			break;
		}
	}

	// after a write the selected address needs to be reset
	m_address_valid = false;
}


//**************************************************************************
//  MEMORY MAPPER
//**************************************************************************

void cs4031_device::update_read_region(int index, offs_t start, offs_t end)
{
	if (!BIT(m_registers[SHADOW_READ], index) && BIT(m_registers[ROMCS], index))
	{
		LOGMEMORY("ROM read from %x to %x\n", start, end);

		m_space->install_rom(start, end, m_bios + start);
	}
	else if (!BIT(m_registers[SHADOW_READ], index) && !BIT(m_registers[ROMCS], index))
	{
		LOGMEMORY("ISA read from %x to %x\n", start, end);

		m_space->install_rom(start, end, m_isa + start - 0xc0000);
	}
	else if (BIT(m_registers[SHADOW_READ], index))
	{
		LOGMEMORY("RAM read from %x to %x\n", start, end);

		m_space->install_rom(start, end, m_ram + start);
	}
	else
	{
		LOGMEMORY("NOP read from %x to %x\n", start, end);

		m_space->nop_read(start, end);
	}
}

void cs4031_device::update_write_region(int index, offs_t start, offs_t end)
{
	if (!BIT(m_registers[SHADOW_WRITE], index) && BIT(m_registers[ROMCS], index) && BIT(m_registers[ROMCS], 7))
	{
		LOGMEMORY("ROM write from %x to %x\n", start, end);

		m_space->install_writeonly(start, end, m_bios + start);
	}
	else if (!BIT(m_registers[SHADOW_WRITE], index) && !BIT(m_registers[ROMCS], index))
	{
		LOGMEMORY("ISA write from %x to %x\n", start, end);

		m_space->install_writeonly(start, end, m_isa + start - 0xc0000);
	}
	else if (BIT(m_registers[SHADOW_WRITE], index))
	{
		LOGMEMORY("RAM write from %x to %x\n", start, end);

		m_space->install_writeonly(start, end, m_ram + start);
	}
	else
	{
		LOGMEMORY("NOP write from %x to %x\n", start, end);

		m_space->nop_write(start, end);
	}
}

void cs4031_device::update_read_regions()
{
	update_read_region(0, 0xc0000, 0xc3fff);
	update_read_region(1, 0xc4000, 0xc7fff);
	update_read_region(2, 0xc8000, 0xcbfff);
	update_read_region(3, 0xcc000, 0xcffff);
	update_read_region(4, 0xd0000, 0xdffff);
	update_read_region(5, 0xe0000, 0xeffff);
	update_read_region(6, 0xf0000, 0xfffff);
}

void cs4031_device::update_write_regions()
{
	update_write_region(0, 0xc0000, 0xc3fff);
	update_write_region(1, 0xc4000, 0xc7fff);
	update_write_region(2, 0xc8000, 0xcbfff);
	update_write_region(3, 0xcc000, 0xcffff);
	update_write_region(4, 0xd0000, 0xdffff);
	update_write_region(5, 0xe0000, 0xeffff);
	update_write_region(6, 0xf0000, 0xfffff);
}


//**************************************************************************
//  KEYBOARD / 8042
//**************************************************************************

void cs4031_device::update_a20m()
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
		update_a20m();
	}
}

void cs4031_device::fast_gatea20(int state)
{
	m_fast_gatea20 = state;
	update_a20m();
}

void cs4031_device::keyboard_gatea20(int state)
{
	m_ext_gatea20 = state;
	update_a20m();
}

u8 cs4031_device::keyb_status_r()
{
	LOGKEYBOARD("cs4031_device::keyb_status_r\n");

	return m_keybc->status_r();
}

void cs4031_device::keyb_command_blocked_w(u8 data)
{
	// command is optionally blocked
	if (!BIT(m_registers[SOFT_RESET_AND_GATEA20], 7))
		m_keybc->command_w(data);
}

void cs4031_device::keyb_command_w(u8 data)
{
	LOGKEYBOARD("cs4031_device::keyb_command_w: %02x\n", data);

	m_keybc_d1_written = false;

	switch (data)
	{
	// self-test
	case 0xaa:
		emulated_kbreset(1);
		emulated_gatea20(1);

		// self-test is never blocked
		m_keybc->command_w(data);
		break;

	case 0xd1:
		m_keybc_d1_written = true;
		keyb_command_blocked_w(data);
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

		keyb_command_blocked_w(data);

		break;

	case 0xff:
		// last data write was blocked?
		if (m_keybc_data_blocked)
		{
			m_keybc_data_blocked = false;
			keyb_command_blocked_w(data);
		}
		else
			m_keybc->command_w(data);

		break;

	// everything else goes directly to the keyboard controller
	default:
		m_keybc->command_w(data);
		break;
	}
}

u8 cs4031_device::keyb_data_r()
{
	LOGKEYBOARD("cs4031_device::keyb_data_r\n");

	return m_keybc->data_r();
}

void cs4031_device::keyb_data_w(u8 data)
{
	LOGKEYBOARD("cs4031_device::keyb_data_w: %02x\n", data);

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
		m_keybc->data_w(data);
	}
}

void cs4031_device::gatea20_w(int state)
{
	LOGKEYBOARD("cs4031_device::gatea20_w: %u\n", state);

	keyboard_gatea20(state);
}

void cs4031_device::kbrst_w(int state)
{
	LOGKEYBOARD("cs4031_device::kbrst_w: %u\n", state);

	// convert to active low signal (gets inverted in at_keybc.cpp)
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
void cs4031_device::sysctrl_w(u8 data)
{
	LOGIO("cs4031_device::sysctrl_w: %u\n", data);

	fast_gatea20(BIT(data, 1));

	if (m_cpureset == 0 && BIT(data, 0))
	{
		// pulse reset line
		m_write_cpureset(1);
		m_write_cpureset(0);
	}

	m_cpureset = BIT(data, 0);
}

u8 cs4031_device::sysctrl_r()
{
	u8 result = 0; // reserved bits read as 0?

	result |= m_cpureset << 0;
	result |= m_fast_gatea20 << 1;

	LOGIO("cs4031_device::sysctrl_r: %u\n", result);

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

u8 cs4031_device::portb_r()
{
	if (0)
		logerror("cs4031_device::portb_r: %02x\n", m_portb);

	return m_portb;
}

void cs4031_device::portb_w(u8 data)
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
void cs4031_device::rtc_nmi_w(u8 data)
{
	if (0)
		logerror("cs4031_device::rtc_nmi_w: %02x\n", data);

	m_nmi_mask = !BIT(data, 7);
	data &= 0x7f;

	m_rtc->address_w(data);
}
