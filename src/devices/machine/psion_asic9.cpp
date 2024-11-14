// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion ASIC9

    ASIC9 is a composite chip comprising of a V30H processor, ASIC1, ASIC2 and
    general I/O and PSU control logic all on one IC. ASIC9 thus integrates all
    the digital logic required to produce a SIBO architecture computer less the
    memory onto one chip. ASIC9 has a few additional features such as an extra
    free-running clock (FRC) and a codec interface for sound.

    TODO:
    - improve RAM configuration for mx machines
    - set RTC timer
    - ASIC9MX implements V30MX, and likely the Temic (Condor) device found in 3c/Siena

******************************************************************************/

#include "emu.h"
#include "psion_asic9.h"
#include "cpu/nec/nec.h"
#include "screen.h"


#define VERBOSE 0
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


DEFINE_DEVICE_TYPE(PSION_ASIC9, psion_asic9_device, "psion_asic9", "Psion ASIC9 V30H")
DEFINE_DEVICE_TYPE(PSION_ASIC9MX, psion_asic9mx_device, "psion_asic9mx", "Psion ASIC9 V30MX")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

psion_asic9_device::psion_asic9_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_memory_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, m_v30(*this, "v30")
	, m_ram(*this, finder_base::DUMMY_TAG)
	, m_rom(*this, finder_base::DUMMY_TAG)
	, m_ram_config("asic9_ram", ENDIANNESS_LITTLE, 16, 24, 0)
	, m_rom_config("asic9_rom", ENDIANNESS_LITTLE, 16, 24, 0)
	, m_tick_timer(nullptr)
	, m_frc1_timer(nullptr)
	, m_frc2_timer(nullptr)
	, m_watchdog_timer(nullptr)
	, m_rtc_timer(nullptr)
	, m_snd_timer(nullptr)
	, m_buz_cb(*this)
	, m_col_cb(*this)
	, m_port_ab_r(*this, 0)
	, m_port_ab_w(*this)
	, m_pcm_in(*this, 0)
	, m_pcm_out(*this)
	, m_data_r(*this, 0x00)
	, m_data_w(*this)
{
}

psion_asic9_device::psion_asic9_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: psion_asic9_device(mconfig, PSION_ASIC9, tag, owner, clock)
{
}

psion_asic9mx_device::psion_asic9mx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: psion_asic9_device(mconfig, PSION_ASIC9MX, tag, owner, clock)
{
}


void psion_asic9_device::mem_map(address_map &map)
{
	map(0x00000, 0xfffff).rw(FUNC(psion_asic9_device::mem_r), FUNC(psion_asic9_device::mem_w));
}

void psion_asic9_device::io_map(address_map &map)
{
	map(0x0000, 0x00ff).rw(FUNC(psion_asic9_device::io_r), FUNC(psion_asic9_device::io_w));
}


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void psion_asic9_device::device_add_mconfig(machine_config &config)
{
	V30(config, m_v30, DERIVED_CLOCK(1, 1));
	m_v30->set_addrmap(AS_PROGRAM, &psion_asic9_device::mem_map);
	m_v30->set_addrmap(AS_IO, &psion_asic9_device::io_map);
	m_v30->set_irq_acknowledge_callback(FUNC(psion_asic9_device::inta_cb));
}


//-------------------------------------------------
//  memory_space_config - return a description of
//  any address spaces owned by this device
//-------------------------------------------------

device_memory_interface::space_config_vector psion_asic9_device::memory_space_config() const
{
	return space_config_vector{
		std::make_pair(AS_A9_RAM, &m_ram_config),
		std::make_pair(AS_A9_ROM, &m_rom_config)
	};
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void psion_asic9_device::device_start()
{
	if (!m_ram->started())
		throw device_missing_dependencies();

	m_ram_space = &space(AS_A9_RAM);
	m_rom_space = &space(AS_A9_ROM);

	switch (m_ram->size())
	{
	case 0x010000: case 0x020000:
		m_ram_type = 0;
		break;

	case 0x040000: case 0x080000:
		m_ram_type = 1;
		break;

	case 0x100000: case 0x200000:
		m_ram_type = 2;
		break;

	case 0x400000: case 0x800000:
		m_ram_type = 3;
		break;
	}

	configure_ram(m_ram_type);
	configure_rom();

	m_tick_timer = timer_alloc(FUNC(psion_asic9_device::tick), this);
	m_frc1_timer = timer_alloc(FUNC(psion_asic9_device::frc1), this);
	m_frc2_timer = timer_alloc(FUNC(psion_asic9_device::frc2), this);
	m_watchdog_timer = timer_alloc(FUNC(psion_asic9_device::watchdog), this);
	m_rtc_timer = timer_alloc(FUNC(psion_asic9_device::rtc), this);
	m_snd_timer = timer_alloc(FUNC(psion_asic9_device::snd), this);
	m_busy_timer = timer_alloc(FUNC(psion_asic9_device::busy), this);

	m_a9_control = 0x00;
	m_a9_status = 0x00;

	save_item(NAME(m_a9_control));
	save_item(NAME(m_a9_lcd_size));
	save_item(NAME(m_a9_interrupt_status));
	save_item(NAME(m_a9_interrupt_mask));
	save_item(NAME(m_a9_protection_mode));
	save_item(NAME(m_a9_protection_upper));
	save_item(NAME(m_a9_protection_lower));
	save_item(NAME(m_a9_port_ab_ddr));
	save_item(NAME(m_a9_port_c_ddr));
	save_item(NAME(m_a9_port_d_ddr));
	save_item(NAME(m_a9_psel_6000));
	save_item(NAME(m_a9_psel_7000));
	save_item(NAME(m_a9_psel_8000));
	save_item(NAME(m_a9_psel_9000));
	save_item(NAME(m_a9_control_extra));
	save_item(NAME(m_frc1_count));
	save_item(NAME(m_frc1_reload));
	save_item(NAME(m_frc2_count));
	save_item(NAME(m_frc2_reload));
	save_item(NAME(m_watchdog_count));
	save_item(NAME(m_rtc));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void psion_asic9_device::device_reset()
{
	m_tick_timer->adjust(attotime::from_hz(32.768), 0, attotime::from_hz(32.768));
	m_frc1_timer->adjust(attotime::from_hz(512000), 0, attotime::from_hz(512000));
	m_frc2_timer->adjust(attotime::from_hz(512000), 0, attotime::from_hz(512000));
	m_watchdog_timer->adjust(attotime::from_hz(4), 0, attotime::from_hz(4));
	m_rtc_timer->adjust(attotime::from_hz(1), 0, attotime::from_hz(1));
	m_snd_timer->adjust(attotime::from_hz(8000), 0, attotime::from_hz(8000)); // unknown data rate

	m_post = 0x00;

	m_a9_interrupt_status = 0x00;
	m_a9_interrupt_mask = 0x00;

	m_frc1_count = 0;
	m_frc1_reload = 0;
	m_frc2_count = 0;
	m_frc2_reload = 0;
	m_buz_toggle = 0;
	m_watchdog_count = 0;
	m_a9_protection_mode = false;
	m_a9_protection_lower = 0x00;
	m_a9_protection_upper = 0x00;
	m_a9_port_ab_ddr = 0x00;
	m_a9_port_c_ddr = 0x00;
	m_a9_port_d_ddr = 0x00;
	m_a9_psel_6000 = 0x00;
	m_a9_psel_7000 = 0x00;
	m_a9_psel_8000 = 0x00;
	m_a9_psel_9000 = 0x00;
	m_a9_control_extra = 0x00;
	m_rtc = 0;

	m_a9_status |= 0x0020; // A9MMainsPresent
	m_a9_status |= 0xe000; // A9MCold

	m_a9_serial_control = 0x00;
	m_a9_channel_select = 0x00;
}


TIMER_CALLBACK_MEMBER(psion_asic9_device::tick)
{
	m_a9_interrupt_status |= 0x02; // Timer
	update_interrupts();
}

TIMER_CALLBACK_MEMBER(psion_asic9_device::frc1)
{
	switch (--m_frc1_count)
	{
	case 0x0000:
		if (BIT(m_a9_control_extra, 5)) // A9MBuzzFromFrc1OrTog
		{
			m_buz_cb(m_buz_toggle ^= 1);
		}
		m_a9_interrupt_status |= 0x40; // A9MFrc1
		update_interrupts();
		break;

	case 0xffff:
		if (BIT(m_a9_control, 12)) // A9MFrc1PreScale
			m_frc1_count = m_frc1_reload;
		break;
	}
}

TIMER_CALLBACK_MEMBER(psion_asic9_device::frc2)
{
	switch (--m_frc2_count)
	{
	case 0x0000:
		m_a9_interrupt_status |= 0x80; // A9MFrc2
		update_interrupts();
		break;

	case 0xffff:
		if (BIT(m_a9_control, 14)) // A9MFrc2PreScale
			m_frc2_count = m_frc2_reload;
		break;
	}
}

TIMER_CALLBACK_MEMBER(psion_asic9_device::watchdog)
{
	m_watchdog_count++;
	m_watchdog_count &= 3;

	if (m_watchdog_count == 3)
	{
		m_a9_status |= 0x0001; // A9MWatchDogNMI
		update_interrupts();
	}
}

TIMER_CALLBACK_MEMBER(psion_asic9_device::rtc)
{
	m_rtc++;
}

TIMER_CALLBACK_MEMBER(psion_asic9_device::snd)
{
	if (BIT(m_a9_control, 11)) // A9MSoundEnable
	{
		switch (BIT(m_a9_control_extra, 7)) // A9MSoundDir
		{
		case 0:
			if (!m_snd_fifo.full())
				m_snd_fifo.enqueue(m_pcm_in());
			if (m_snd_fifo.full())
				m_a9_status |= 0x0800;
			break;

		case 1:
			if (!m_snd_fifo.empty())
				m_pcm_out(m_snd_fifo.dequeue());
			if (!m_snd_fifo.full())
				m_a9_status &= ~0x0800;
			break;
		}
		m_a9_interrupt_status |= 0x01; // Sound
		update_interrupts();
	}
}

void psion_asic9_device::sds_int_w(int state)
{
	if (state)
		m_a9_interrupt_status |= 0x04; // A9MSlave
	else
		m_a9_interrupt_status &= ~0x04;

	update_interrupts();
}

void psion_asic9_device::eint0_w(int state)
{
	if (state)
		m_a9_interrupt_status |= 0x08; // A9MExpIntC
	else
		m_a9_interrupt_status &= ~0x08;

	update_interrupts();
}

void psion_asic9_device::eint1_w(int state)
{
	if (state)
		m_a9_interrupt_status |= 0x10; // A9MExpIntA
	else
	m_a9_interrupt_status &= ~0x10;

	update_interrupts();
}

void psion_asic9_device::eint2_w(int state)
{
	if (state)
		m_a9_interrupt_status |= 0x20; // A9MExpIntB
	else
		m_a9_interrupt_status &= ~0x20;

	update_interrupts();
}


void psion_asic9_device::medchng_w(int state)
{
	if (state)
		m_a9_status |= 0x04; // A9MDoorNMI
	else
		m_a9_status &= ~0x04;

	update_interrupts();
}

void psion_asic9_device::update_interrupts()
{
	bool irq = m_a9_interrupt_status & m_a9_interrupt_mask;
	bool nmi = m_a9_status & 0x000f;

	m_v30->set_input_line(INPUT_LINE_IRQ0, irq ? ASSERT_LINE : CLEAR_LINE);
	m_v30->set_input_line(INPUT_LINE_NMI,  nmi ? ASSERT_LINE : CLEAR_LINE);
}


IRQ_CALLBACK_MEMBER(psion_asic9_device::inta_cb)
{
	// IRQ  Vector  Name   Description
	//  0    0x78   CSINT  CODEC sound interrupt.
	//  1    0x79   TINT   32 Hz tick interrupt.
	//  2    0x7A   SSINT  Serial slave interrupt.
	//  3    0x7B   EINT0  External interrupt input 0.
	//  4    0x7C   EINT1  External interrupt input 1 (inverted).
	//  5    0x7D   EINT2  External interrupt input 2 (inverted).
	//  6    0x7E   FRC1OI FRC1 overflow interrupt.
	//  7    0x7F   FRC2OI FRC2 overflow interrupt.
	uint8_t vector = 0x78;
	for (int irq = 0; irq < 8; irq++)
	{
		if (m_a9_interrupt_status & m_a9_interrupt_mask & (1 << irq))
		{
			vector += irq;
			break;
		}
	}
	return vector;
}


TIMER_CALLBACK_MEMBER(psion_asic9_device::busy)
{
	m_v30->set_input_line(NEC_INPUT_LINE_POLL, ASSERT_LINE);
}


//**************************************************************************
//  RAM/ROM CONFIGURATION
//**************************************************************************

uint32_t psion_asic9_device::ram_device_size(uint8_t device_type)
{
	uint32_t size = 0;

	switch (device_type & 3)
	{
	case 0: size = 0x010000; break; // 256KBits
	case 1: size = 0x040000; break; // 1MBits
	case 2: size = 0x100000; break; // 4MBits
	case 3: size = 0x400000; break; // 16MBits
	}
	return size;
}

void psion_asic9_device::configure_ram(uint8_t device_type)
{
	uint32_t device_size_actual = ram_device_size(m_ram_type);
	uint32_t device_size_test   = ram_device_size(device_type);

	m_ram_space->unmap_readwrite(0, 0xffffff);

	for (int i = 0; i < (m_ram->size() / device_size_actual); i++)
	{
		offs_t addrstart  =  device_size_test * i;
		offs_t addrend    = (device_size_test * i) + std::min(device_size_actual, device_size_test) - 1;
		offs_t addrmirror = 0xffffff ^ ((device_size_test * 4) - 1);

		if (device_size_actual < device_size_test)
			addrmirror ^= (device_size_test - device_size_actual);

		LOG("configure_ram: %d type %d ramsize %06x devsize %06x start %06x end %06x mirror %06x\n", i, device_type, m_ram->size(), device_size_actual, addrstart, addrend, addrmirror);
		m_ram_space->install_ram(addrstart, addrend, addrmirror, m_ram->pointer() + (device_size_actual * i));
	}
}

void psion_asic9_device::configure_rom()
{
	m_rom_space->install_rom(0x800000, 0x800000 + m_rom->bytes() - 1, 0x7fffff ^ (m_rom->bytes() - 1), m_rom->base());
}


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

uint8_t psion_asic9_device::col_r()
{
	return m_a9_control_extra & 0x0f;
}


bool psion_asic9_device::is_protected(offs_t offset)
{
	if (m_a9_protection_mode && (offset <= m_a9_protection_lower || offset > m_a9_protection_upper))
	{
	  LOG("%s is_protected: %05x < %05x <= %05x\n", machine().describe_context(), m_a9_protection_lower, offset, m_a9_protection_upper);
	  m_a9_status |= 0x0002; // A9MProtectedModeNMI
	  update_interrupts();
	  return true;
	}
	return false;
}

offs_t psion_asic9_device::translate_address(offs_t offset)
{
	switch (offset & 0xf0000)
	{
	case 0x00000: case 0x10000: case 0x20000: case 0x30000: case 0x40000: case 0x50000:
		break;

	case 0x60000:
		offset = (m_a9_psel_6000 << 16) | (offset & 0xffff);
		break;

	case 0x70000:
		offset = (m_a9_psel_7000 << 16) | (offset & 0xffff);
		break;

	case 0x80000:
		offset = (m_a9_psel_8000 << 16) | (offset & 0xffff);
		break;

	case 0x90000:
		offset = (m_a9_psel_9000 << 16) | (offset & 0xffff);
		break;

	case 0xa0000: case 0xb0000: case 0xc0000: case 0xd0000: case 0xe0000: case 0xf0000:
		offset = 0xf00000 | offset;
		break;
	}
	return offset;
}

uint16_t psion_asic9_device::mem_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0xffff;

	offset <<= 1;

	offs_t addr = translate_address(offset);

	switch (offset & 0x80000)
	{
	case 0x00000:
		data = m_ram_space->read_word(addr, mem_mask);
		break;

	case 0x80000:
		data = m_rom_space->read_word(addr, mem_mask);
		break;
	}

	return data;
}

void psion_asic9_device::mem_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset <<= 1;

	offs_t addr = translate_address(offset);

	if (!is_protected(addr))
	{
		switch (offset & 0x80000)
		{
		case 0x00000:
			m_ram_space->write_word(addr, data, mem_mask);
			break;
		}
	}
}


uint16_t psion_asic9_device::io_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t data = 0x00;

	offset <<= 1;

	switch (offset)
	{
	case 0x00: // A9Index - Not used
		LOG("%s io_r: A9Index => %02x\n", machine().describe_context(), data);
		break;

	case 0x02: // A9WControl
		//   b0    A9MDisableClockDivide
		//   b1    A9MDisableIoWait
		//   b2    A9MDisableMemWait
		// b3-b4   A9MRamDeviceSize
		//   b5    A9MDisableDMADivide
		//   b6    A9MArmStandBy
		//   b7    A9MZeroIsGrayMode
		//   b8    A9MDoorNMIEnable
		//   b9    A9MLowBatNMIEnable
		//   b10   A9MLcdEnable
		//   b11   A9MSoundEnable
		//   b12   A9MFrc1PreScale
		//   b13   A9MFrc1Is512KHzOr1Hz
		//   b14   A9MFrc2PreScale
		//   b15   A9MFrc2Is512KHzOr1KHz
		data = m_a9_control; // | 0x18;
		LOG("%s io_r: A9WControl => %04x\n", machine().describe_context(), data);
		break;

	case 0x04: // A9WStatus
		//   b0    A9MWatchDogNMI
		//   b1    A9MProtectedModeNMI
		//   b2    A9MDoorNMI
		//   b3    A9MLowBatNMI
		//   b4    A9MDoorSwitch
		//   b5    A9MMainsPresent
		//   b6    A9MSlaveClock
		//   b7    A9MKeyboard
		//   b8    A9MSlaveDataValid
		//   b9    A9MSlaveControlFrame
		//   b10   A9MSlaveOverrun
		//   b11   A9MFifoFull
		//   b12   A9MNoBattery
		//   b13   A9MReset
		//   b14   A9MPowerFail
		//   b15   A9MCold
		data = m_a9_status;
		if (!machine().side_effects_disabled())
		{
			m_a9_status &= ~0x2000; // clear A9MReset
		}
		LOG("%s io_r: A9WStatus => %04x\n", machine().describe_context(), data);
		break;

	case 0x06: // A9BInterruptStatus
		if (ACCESSING_BITS_0_7)
		{
			data = m_a9_interrupt_status & m_a9_interrupt_mask;
			LOG("%s io_r: A9InterruptStatus => %02x\n", machine().describe_context(), data);
		}
		break;

	case 0x08: // A9BInterruptMask
		if (ACCESSING_BITS_0_7)
		{
			data = m_a9_interrupt_mask;
			LOG("%s io_r: A9BInterruptMask => %02x\n", machine().describe_context(), data);
		}
		break;

	case 0x12: // A9WFrc1Data
		data = m_frc1_count;
		LOG("%s io_r: A9WFrc1Data => %04x\n", machine().describe_context(), data);
		break;

	case 0x14: // A9BProtectionOff
		if (ACCESSING_BITS_0_7)
		{
			//LOG("%s io_r: A9BProtectionOff => %02x\n", machine().describe_context(), data);
			m_a9_protection_mode = false;
		}
		break;

	case 0x1a: // A9BSoundData
		if (ACCESSING_BITS_0_7)
		{
			data = m_snd_fifo.dequeue();
			if (!m_snd_fifo.full())
				m_a9_status &= ~0x0800;
			LOG("%s io_r: A9BSoundData => %02x\n", machine().describe_context(), data);
		}
		break;

	case 0x1e: // A9WFrc2Data
		data = m_frc2_count;
		LOG("%s io_r: A9WFrc2Data => %04x\n", machine().describe_context(), data);
		break;

	case 0x20: // A9WPortABData
		data = m_port_ab_r() & ~m_a9_port_ab_ddr;
		LOG("%s io_r: A9WPortABData => %04x\n", machine().describe_context(), data);
		break;

	case 0x22: // A9WPortABDDR
		if (ACCESSING_BITS_0_7)
		{
			data |= m_a9_port_ab_ddr & 0x00ff;
			LOG("%s io_r: A9WPortADDR => %02x\n", machine().describe_context(), data);
		}
		if (ACCESSING_BITS_8_15)
		{
			data |= m_a9_port_ab_ddr & 0xff00;
			LOG("%s io_r: A9WPortBDDR => %02x\n", machine().describe_context(), data >> 8);
		}
		break;

	case 0x24: // A9WPortCDData
		if (ACCESSING_BITS_0_7)
		{
			data |= 0x00;
			LOG("%s io_r: A9WPortCData => %02x\n", machine().describe_context(), data);
		}
		if (ACCESSING_BITS_8_15)
		{
			data |= 0x00 << 8;
			LOG("%s io_r: A9WPortDData => %02x\n", machine().describe_context(), data >> 8);
		}
		break;

	case 0x26: // A9BPortCDDDR
		if (ACCESSING_BITS_0_7)
		{
			data |= m_a9_port_c_ddr;
			LOG("%s io_r: A9BPortCDDR => %04x\n", machine().describe_context(), data);
		}
		if (ACCESSING_BITS_8_15)
		{
			data |= m_a9_port_d_ddr << 8;
			LOG("%s io_r: A9WPortDDDR <= %02x\n", machine().describe_context(), data >> 8);
		}
		break;

	case 0x28: // A9BPageSelect6000
		if (ACCESSING_BITS_0_7)
		{
			data |= m_a9_psel_6000;
			LOG("%s io_r: A9BPageSelect6000 => %02x\n", machine().describe_context(), data);
		}
		if (ACCESSING_BITS_8_15)
		{
			data |= m_a9_psel_7000 << 8;
			LOG("%s io_r: A9BPageSelect7000 => %02x\n", machine().describe_context(), data >> 8);
		}
		break;

	case 0x2a: // A9BPageSelect8000
		if (ACCESSING_BITS_0_7)
		{
			data |= m_a9_psel_8000;
			//LOG("%s io_r: A9BPageSelect8000 => %02x\n", machine().describe_context(), data);
		}
		if (ACCESSING_BITS_8_15)
		{
			data |= m_a9_psel_9000 << 8;
			//LOG("%s io_r: A9BPageSelect9000 => %02x\n", machine().describe_context(), data >> 8);
		}
		break;

	case 0x2c: // A9WControlExtra
		data = m_a9_control_extra;
		LOG("%s io_r: A9WControlExtra => %04x\n", machine().describe_context(), data);
		break;

	case 0x2e: // A9WPumpControl
		LOG("%s io_r: A9WPumpControl <= %04x\n", machine().describe_context(), data);
		break;

	case 0x80: // A9WRtcLSW
		data = m_rtc & 0x0000ffff;
		//LOG("%s io_r: A9WRtcLSW <= %04x\n", machine().describe_context(), data);
		break;

	case 0x82: // A9WRtcMSW
		data = (m_rtc & 0xffff0000) >> 16;
		//LOG("%s io_r: A9WRtcMSW <= %04x\n", machine().describe_context(), data);
		break;

	case 0x88: // A9BSlaveData
		LOG("%s io_r: A9BSlaveData <= %02x\n", machine().describe_context(), data);
		break;

	case 0x8a: // A9BSerialData
		data = m_a9_serial_data;
		if ((m_a9_serial_control & 0x10) == 0x10)
			m_a9_serial_data = receive_frame();
		LOG("%s io_r: A9BSerialData => %02x\n", machine().describe_context(), data);
		break;

	case 0x8e: // A9BChannelSelect
		data = m_a9_channel_select;
		break;

	default:
		logerror("%s io_r: Unhandled register %02x => %04x\n", machine().describe_context(), offset, data);
		break;
	}
	return data;
}

void psion_asic9_device::io_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	offset <<= 1;

	switch (offset)
	{
	case 0x00: // Post
		if (m_post != data)
		{
			switch (data)
			{
			case 0x10: LOG("%s io_w: Post <= %02x ROM Test\n", machine().describe_context(), data); break;
			case 0x20: LOG("%s io_w: Post <= %02x System RAM Test\n", machine().describe_context(), data); break;
			case 0x30: LOG("%s io_w: Post <= %02x Video RAM Test\n", machine().describe_context(), data); break;
			case 0xf0: LOG("%s io_w: Post <= %02x Complete\n", machine().describe_context(), data); break;
			default:   LOG("%s io_w: Post <= %02x Unknown\n", machine().describe_context(), data); break;
			}
			m_post = data;
		}
		break;

	case 0x02: // A9WControl
		//   b0    A9MDisableClockDivide
		//   b1    A9MDisableIoWait
		//   b2    A9MDisableMemWait
		//   b3-b4 A9MRamDeviceSize
		//   b5    A9MDisableDMADivide
		//   b6    A9MArmStandBy
		//   b7    A9MZeroIsGrayMode
		//   b8    A9MDoorNMIEnable
		//   b9    A9MLowBatNMIEnable
		//   b10   A9MLcdEnable
		//   b11   A9MSoundEnable
		//   b12   A9MFrc1PreScale
		//   b13   A9MFrc1Is512KHzOr1Hz
		//   b14   A9MFrc2PreScale
		//   b15   A9MFrc2Is512KHzOr1KHz
		LOG("%s io_w: A9WControl <= %04x, RamDeviceSize %s, Frc1 %s%s, Frc2 %s%s\n", machine().describe_context(), data,
			BIT(data, 3,2) == 0 ? "256KBits" : (BIT(data, 3,2) == 1 ? "1MBits" : (BIT(data, 3,2) == 2 ? "4MBits" : "16MBits")),
			BIT(data, 13) ? "512KHz" : "1Hz", BIT(data, 12) ? " PreScale" : "",
			BIT(data, 15) ? "512KHz" : "1024Hz", BIT(data, 14) ? " PreScale" : "");

		if (BIT(data, 3, 2) != BIT(m_a9_control, 3, 2))
		{
			configure_ram(BIT(data, 3, 2));
		}
		if (BIT(data, 13) != BIT(m_a9_control, 13))
		{
			if (data & 0x2000)
				m_frc1_timer->adjust(attotime::zero, 0, attotime::from_hz(512000));
			else
				m_frc1_timer->adjust(attotime::zero, 0, attotime::from_hz(1));
		}
		if (BIT(data, 15) != BIT(m_a9_control, 15))
		{
			if (data & 0x8000)
				m_frc2_timer->adjust(attotime::zero, 0, attotime::from_hz(512000));
			else
				m_frc2_timer->adjust(attotime::zero, 0, attotime::from_hz(1024));
		}
		m_a9_control = data;
		break;

	case 0x04: // A9WLcdSize
		//  b0-b10 A9MLcdNumberOfPixels - (end of frame address / 16) - 1
		// b11-b15 A9MLcdLineLength     - (number of pixels per line / 32) - 1
		LOG("%s io_w: A9WLcdSize <= %04x, Pixels in line %d, Total pixels in display %d\n", machine().describe_context(), data, (BIT(data, 11, 5) + 1) * 32, (BIT(data, 0, 11) + 1) * 128);
		m_a9_lcd_size = data;
		break;

	case 0x06: // A9WLcdControl
		//  b0-b4  LcdPixelRate  - 3
		//  b5-b9  LcdACLineRate - 13
		// b10-b11 LcdMode       - 3 (Dual Screen mode)
		LOG("%s io_w: A9WLcdControl <= %04x, Mode %d %s Page\n", machine().describe_context(), data, BIT(data, 10,2), BIT(data, 10,2) == 3 ? "Dual" : "Single");
		break;

	case 0x08: // A9BInterruptMask
		if (ACCESSING_BITS_0_7)
		{
			// b0 A9MSound
			// b1 A9MTimer
			// b2 A9MSlave
			// b3 A9MExpIntC
			// b4 A9MExpIntA
			// b5 A9MExpIntB
			// b6 A9MFrc1
			// b7 A9MFrc2
			LOG("%s io_w: A9BInterruptMask <= %02x\n", machine().describe_context(), data & 0xff);
			m_a9_interrupt_mask = data & 0xff;
		}
		if (ACCESSING_BITS_8_15)
		{
			LOG("%s io_w: A9BNmiClear <= %02x\n", machine().describe_context(), data >> 8);
			m_a9_status &= 0xfff0;
		}
		update_interrupts();
		break;

	case 0x0a: // A9BNonSpecificEoi
		if (ACCESSING_BITS_0_7)
		{
			LOG("%s io_w: A9BNonSpecificEoi <= %02x\n", machine().describe_context(), data & 0xff);
		}
		if (ACCESSING_BITS_8_15)
		{
			LOG("%s io_w: A9BStartFlagClear <= %02x\n", machine().describe_context(), data >> 8);
		}
		break;

	case 0x0c: // A9BTimerEoiW
		if (ACCESSING_BITS_0_7)
		{
			LOG("%s io_w: A9BTimerEoi <= %02x\n", machine().describe_context(), data & 0xff);
			m_a9_interrupt_status &= ~0x02; // Timer
		}
		if (ACCESSING_BITS_8_15)
		{
			LOG("%s io_w: A9BSerialSlaveEoi <= %02x\n", machine().describe_context(), data >> 8);
			m_a9_interrupt_status &= ~0x04; // SerialSlave
		}
		update_interrupts();
		break;

	case 0x0e: // A9Frc1Eoi
		if (ACCESSING_BITS_0_7)
		{
			LOG("%s io_w: A9Frc1Eoi <= %02x\n", machine().describe_context(), data & 0xff);
			m_a9_interrupt_status &= ~0x40; // Frc1Expired
		}
		if (ACCESSING_BITS_8_15)
		{
			LOG("%s io_w: A9Frc2Eoi <= %02x\n", machine().describe_context(), data >> 8);
			m_a9_interrupt_status &= ~0x80; // Frc2Expired
		}
		update_interrupts();
		break;

	case 0x10: // A9WResetWatchDog
		//LOG("%s io_w: A9WResetWatchDog <= %04x\n", machine().describe_context(), data);
		m_watchdog_count = 0;
		break;

	case 0x12: // A9WFrc1Data
		LOG("%s io_w: A9WFrc1Data <= %04x\n", machine().describe_context(), data);
		m_frc1_reload = data;
		m_frc1_count = data;
		break;

	case 0x14: // A9BProtectionOn
		if (ACCESSING_BITS_0_7)
		{
			//LOG("%s io_w: A9BProtectionOn <= %02x\n", machine().describe_context(), data);
			m_a9_protection_mode = true;
		}
		if (ACCESSING_BITS_8_15)
		{
			//LOG("%s io_w: A9BProtectionOff <= %02x\n", machine().describe_context(), data);
			m_a9_protection_mode = false;
		}
		break;

	case 0x16: // A9WProtectionUpper
		//LOG("%s io_w: A9WProtectionUpper <= %04x\n", machine().describe_context(), data);
		m_a9_protection_upper = (data << 4) | 0x0f;
		break;

	case 0x18: // A9WProtectionLower
		//LOG("%s io_w: A9WProtectionLower <= %04x\n", machine().describe_context(), data);
		m_a9_protection_lower = data << 4;
		break;

	case 0x1a: // A9BSoundData
		if (ACCESSING_BITS_0_7)
		{
			LOG("%s io_w: A9BSoundData <= %04x\n", machine().describe_context(), data);
			m_snd_fifo.enqueue(data & 0xff);
			if (m_snd_fifo.full())
				m_a9_status |= 0x0800;
		}
		break;

	case 0x1c: // A9BSoundEoi
		if (ACCESSING_BITS_0_7)
		{
			LOG("%s io_w: A9BSoundEoi <= %02x\n", machine().describe_context(), data);
			m_a9_interrupt_status &= ~0x01; // Sound
			update_interrupts();
		}
		break;

	case 0x1e: // A9WFrc2Data
		LOG("%s io_w: A9WFrc2Data <= %04x\n", machine().describe_context(), data);
		m_frc2_reload = data;
		m_frc2_count = data;
		break;

	case 0x20: // A9WPortABData
		if (ACCESSING_BITS_0_7)
		{
			LOG("%s io_w: A9WPortAData <= %02x\n", machine().describe_context(), data & 0xff);
		}
		if (ACCESSING_BITS_8_15)
		{
			LOG("%s io_w: A9WPortBData <= %02x\n", machine().describe_context(), data >> 8);
		}
		break;

	case 0x22: // A9WPortABDDR
		if (ACCESSING_BITS_0_7)
		{
			LOG("%s io_w: A9WPortADDR <= %02x\n", machine().describe_context(), data & 0xff);
			m_a9_port_ab_ddr = (m_a9_port_ab_ddr & 0xff00) | data;
		}
		if (ACCESSING_BITS_8_15)
		{
			LOG("%s io_w: A9WPortBDDR <= %02x\n", machine().describe_context(), data >> 8);
			m_a9_port_ab_ddr = (m_a9_port_ab_ddr & 0x00ff) | (data << 8);
		}
		break;

	case 0x24: // A9WPortCDData
		if (ACCESSING_BITS_0_7)
		{
			LOG("%s io_w: A9WPortCData <= %02x\n", machine().describe_context(), data & 0xff);
		}
		if (ACCESSING_BITS_8_15)
		{
			LOG("%s io_w: A9WPortDData <= %02x\n", machine().describe_context(), data >> 8);
		}
		break;

	case 0x26: // A9BPortCDDDR
		if (ACCESSING_BITS_0_7)
		{
			LOG("%s io_w: A9WPortCDDR <= %02x\n", machine().describe_context(), data & 0xff);
			m_a9_port_c_ddr = data & 0xff;
		}
		if (ACCESSING_BITS_8_15)
		{
			LOG("%s io_w: A9WPortDDDR <= %02x\n", machine().describe_context(), data >> 8);
			m_a9_port_d_ddr = data >> 8;
		}
		break;

	case 0x28: // A9BPageSelect6000
		if (ACCESSING_BITS_0_7)
		{
			LOG("%s io_w: A9BPageSelect6000 <= %02x\n", machine().describe_context(), data & 0xff);
			m_a9_psel_6000 = data & 0xff;
		}
		if (ACCESSING_BITS_8_15)
		{
			LOG("%s io_w: A9BPageSelect7000 <= %02x\n", machine().describe_context(), data >> 8);
			m_a9_psel_7000 = data >> 8;
		}
		break;

	case 0x2a: // A9BPageSelect8000
		if (ACCESSING_BITS_0_7)
		{
			//LOG("%s io_w: A9BPageSelect8000 <= %02x\n", machine().describe_context(), data & 0xff);
			m_a9_psel_8000 = data & 0xff;
		}
		if (ACCESSING_BITS_8_15)
		{
			//LOG("%s io_w: A9BPageSelect9000 <= %02x\n", machine().describe_context(), data >> 8);
			m_a9_psel_9000 = data >> 8;
		}
		break;

	case 0x2c: // A9WControlExtra
		// b0-b3   A9MKeyCol
		// b4      A9MBuzzTog
		// b5      A9MBuzzFromFrc1OrTog
		// b6      A9MExonDisable
		// b7      A9MSoundDir
		// b8      A9MClockEnable1
		// b9      A9MClockEnable2
		// b10     A9MClockEnable3
		// b11     A9MClockEnable4
		// b12     A9MClockEnable5
		// b13-b14 A9MClkDiv
		// b15     A9MSlaveIntEnable
		LOG("%s io_w: A9WControlExtra <= %04x\n", machine().describe_context(), data);
		m_a9_control_extra = data;

		// enable keyboard COLs
		switch (data & 0xf)
		{
		case 0x0: m_col_cb(0xff); break;
		case 0x8: m_col_cb(0x01); break;
		case 0x9: m_col_cb(0x02); break;
		case 0xa: m_col_cb(0x04); break;
		case 0xb: m_col_cb(0x08); break;
		case 0xc: m_col_cb(0x10); break;
		case 0xd: m_col_cb(0x20); break;
		case 0xe: m_col_cb(0x40); break;
		case 0xf: m_col_cb(0x80); break;
		default:  m_col_cb(0x00); break;
		}

		if (!BIT(data, 5))
		{
			m_buz_cb(BIT(data, 4));
		}
		break;

	case 0x2e: // A9WPumpControl
		LOG("%s io_w: A9WPumpControl <= %04x\n", machine().describe_context(), data);
		break;

	case 0x80: // A9WRtcLSW
		//LOG("%s io_w: A9WRtcLSW <= %04x\n", machine().describe_context(), data);
		m_rtc = (m_rtc & 0xffff0000) | data;
		break;

	case 0x82: // A9WRtcMSW
		//LOG("%s io_w: A9WRtcMSW <= %04x\n", machine().describe_context(), data);
		m_rtc = (m_rtc & 0x0000ffff) | (data << 16);
		break;

	case 0x84: // A9WNullFrame
		LOG("%s io_w: A9WNullFrame <= %02x\n", machine().describe_context(), data);
		transmit_frame(NULL_FRAME);
		break;

	case 0x8a: // A9BSerialData
		LOG("%s io_w: A9BSerialData <= %02x\n", machine().describe_context(), data & 0xff);
		if ((m_a9_serial_control & 0xc0) == 0x80)
			transmit_frame(DATA_FRAME | (data & 0xff));
		break;

	case 0x8c: // A9BSerialControl - Serial channel write control register
		// WriteSingle  10000000b;  ReadSingle   11000000b
		// WriteMulti   10010000b;  ReadMulti    11010000b
		// Reset        00000000b;  Select       01000000b
		// Asic2SlaveId      001h;  Asic5PackId       002h
		// Asic5NormalId     003h;  Asic6Id           004h
		// Asic8Id           005h;  Asic4Id           006h
		LOG("%s io_w: A2SerialControl <= %02x\n", machine().describe_context(), data & 0xff);
		m_a9_serial_control = data & 0xff;
		transmit_frame(CONTROL_FRAME | m_a9_serial_control);

		if ((m_a9_serial_control & 0x40) == 0x40)
			m_a9_serial_data = receive_frame();
		break;

	case 0x8e: // A9BChannelSelect - Serial channel select register
		// b0    Pack1Enable         - 1 to select pack 1
		// b1    Pack2Enable         - 1 to select pack 2
		// b2    Pack3Enable         - 1 to select pack 3
		// b3    Pack4Enable         - 1 to select pack 4
		// b4    Pack5Enable         - 1 to select pack 5
		// b5-b6 SerialClockRate     - 0 Medium, 1 Special, 2 Slow, 3 Fast
		// b7    MultiplexEnable     - 1 to loop slave channel to pack channel
		LOG("%s io_w: A9BChannelSelect <= %02x Channels %c %c %c %c %c\n", machine().describe_context(), data & 0xff,
			BIT(data, 0) ? '0' : ' ',
			BIT(data, 1) ? '1' : ' ',
			BIT(data, 2) ? '2' : ' ',
			BIT(data, 3) ? '3' : ' ',
			BIT(data, 4) ? '4' : ' ');
		m_a9_channel_select = data & 0xff;
		break;

	default:
		logerror("%s io_w: Unhandled register %02x <= %04x\n", machine().describe_context(), offset, data);
		break;
	}
}


//-------------------------------------------------
//  SIBO Serial Protocol Controller
//-------------------------------------------------

bool psion_asic9_device::channel_active(int channel)
{
	switch (channel)
	{
	case 0: case 1: case 2: case 3: case 4:
		return BIT(m_a9_channel_select, channel);
	}
	return false;
}

void psion_asic9_device::transmit_frame(uint16_t data)
{
	m_busy_timer->adjust(attotime::from_ticks(12, clock() / 2));
	m_v30->set_input_line(NEC_INPUT_LINE_POLL, CLEAR_LINE);

	for (int ch = 0; ch < 8; ch++)
	{
		if (channel_active(ch))
		{
			LOG("%s Channel %d Transmit %s frame %02x\n", machine().describe_context(), ch, (data & DATA_FRAME) ? "Data" : (data & CONTROL_FRAME) ? "Control" : "Null", data & 0xff);
			m_data_w[ch](data);
		}
	}
}

uint8_t psion_asic9_device::receive_frame()
{
	uint8_t data = 0x00;

	m_busy_timer->adjust(attotime::from_ticks(12, clock() / 2));
	m_v30->set_input_line(NEC_INPUT_LINE_POLL, CLEAR_LINE);

	for (int ch = 0; ch < 8; ch++)
	{
		if (channel_active(ch))
		{
			data |= m_data_r[ch]();
			LOG("%s Channel %d Receive Data frame %02x\n", machine().describe_context(), ch, data);
		}
	}

	return data;
}


//-------------------------------------------------
//  LCD Controller
//-------------------------------------------------

uint32_t psion_asic9_device::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (m_a9_control & 0x0400) // LCD enable bit
	{
		pen_t const *const pens = screen.palette().pens();

		int const width = (BIT(m_a9_lcd_size, 11, 5) + 1) * 32;
		uint16_t const size = (BIT(m_a9_lcd_size, 0, 11) + 1) * 16;

		for (int y = screen.visible_area().min_y; y <= screen.visible_area().max_y; y++)
		{
			for (int x = screen.visible_area().min_x; x <= (screen.visible_area().max_x / 8); x++)
			{
				uint8_t const black = m_ram_space->read_byte(0x0400 + y * (width / 8) + x);
				uint8_t const grey = m_ram_space->read_byte(0x0400 + size + y * (width / 8) + x);
				uint16_t *p = &bitmap.pix(y, x << 3);
				for (int i = 0; i < 8; i++)
					*p++ = BIT(black, i) ? pens[1] : (BIT(grey, i) ? pens[2] : pens[0]);
			}
		}
	}
	else
	{
		bitmap.fill(0, cliprect);
	}
	return 0;
}
