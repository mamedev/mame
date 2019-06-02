// license:BSD-3-Clause
// copyright-holders:AJR
/**********************************************************************

    Intel 82355 Bus Master Interface Controller (BMIC)

    Currently very little of this device is emulated besides storing
    data that the local processor writes to it. Its interface will
    likely evolve greatly in tandem with EISA bus emulation.

**********************************************************************/

#include "emu.h"
#include "i82355.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(I82355, i82355_device, "i82355", "Intel 82355 BMIC")


//**************************************************************************
//  DEVICE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  i82355_device - constructor
//-------------------------------------------------

i82355_device::i82355_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, I82355, tag, owner, clock)
	, m_eint_callback(*this)
	, m_lint_callback(*this)
	, m_local_index(0x00)
	, m_local_status(0x00)
	, m_id{0}
	, m_global_config(0x00) // TBD: revision code in upper 4 bits
	, m_system_interrupt(0x00)
	, m_semaphore_flag{false, false}
	, m_semaphore_etest{false, false}
	, m_semaphore_ltest{false, false}
	, m_local_doorbell_status(0x00)
	, m_local_doorbell_enable(0x00)
	, m_eisa_doorbell_status(0x00)
	, m_eisa_doorbell_enable(0x00)
	, m_peek_poke_data{0}
	, m_peek_poke_address{0}
	, m_peek_poke_status(0x00)
	, m_io_decode_base{0x00, 0x00}
	, m_io_decode_control{0x00, 0x00}
	, m_transfer_config{0x00, 0x00}
	, m_transfer_status{0x00, 0x00}
	, m_base_count{{0}, {0}}
	, m_base_address{{0}, {0}}
	, m_current_count{{0}, {0}}
	, m_current_address{{0}, {0}}
	, m_tbi_base{{0}, {0}}
	, m_tbi_current{{0}, {0}}
{
	// TODO: 24-byte FIFO for each data transfer channel
	std::fill(std::begin(m_mailbox), std::end(m_mailbox), 0x00);
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void i82355_device::device_resolve_objects()
{
	m_eint_callback.resolve_safe();
	m_lint_callback.resolve_safe();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i82355_device::device_start()
{
	// Register save state
	save_item(NAME(m_local_index));
	save_item(NAME(m_local_status));
	save_item(NAME(m_id.d));
	save_item(NAME(m_global_config));
	save_item(NAME(m_system_interrupt));
	save_item(NAME(m_semaphore_flag));
	save_item(NAME(m_semaphore_etest));
	save_item(NAME(m_semaphore_ltest));
	save_item(NAME(m_local_doorbell_status));
	save_item(NAME(m_local_doorbell_enable));
	save_item(NAME(m_eisa_doorbell_enable));
	save_item(NAME(m_eisa_doorbell_status));
	save_item(NAME(m_mailbox));
	save_item(NAME(m_peek_poke_data.d));
	save_item(NAME(m_peek_poke_address.d));
	save_item(NAME(m_peek_poke_control));
	save_item(NAME(m_peek_poke_status));
	save_item(NAME(m_io_decode_base));
	save_item(NAME(m_io_decode_control));
	save_item(NAME(m_transfer_config));
	save_item(NAME(m_transfer_status));
	for (int i = 0; i < 2; i++)
	{
		save_item(NAME(m_base_count[i].d), i);
		save_item(NAME(m_base_address[i].d), i);
		save_item(NAME(m_current_count[i].d), i);
		save_item(NAME(m_current_address[i].d), i);
		save_item(NAME(m_tbi_base[i].w), i);
		save_item(NAME(m_tbi_current[i].w), i);
	}
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i82355_device::device_reset()
{
	// Reset local index register
	m_local_index = 0x00;

	// Reset interrupt/status registers
	m_local_status = 0x00;
	m_local_doorbell_status = 0x00;
	m_local_doorbell_enable = 0x00;
	m_eisa_doorbell_enable = 0x00;
	m_eisa_doorbell_status = 0x00;
	m_system_interrupt = 0x00;
	m_eint_callback(1);
	m_lint_callback(1);

	// Reset global configuration register
	m_global_config &= 0xf0;

	// Reset semaphore flags
	for (int i = 0; i < 2; i++)
		m_semaphore_flag[i] = m_semaphore_etest[i] = m_semaphore_ltest[i] = false;

	// ID register byte 0 set to delay value
	m_id.b.h3 = 0x70 | (m_id.b.h3 & 0x0f);

	// Reset peek/poke address and control registers
	m_peek_poke_address.d = 0x00000000;
	m_peek_poke_control = 0x00;

	// Reset data transfer configuration and status registers (TODO: 24-byte FIFOs)
	for (int i = 0; i < 2; i++)
	{
		m_transfer_config[i] = 0x00;
		m_transfer_status[i] = 0x00;
	}

	// Reset I/O range decode registers (TODO: differs if local processor not present)
	m_io_decode_base[0] = 0xe0;
	m_io_decode_base[1] = 0x00;
	m_io_decode_control[0] = m_io_decode_control[1] = 0x20;
}

//**************************************************************************
//  INTERRUPT REGISTRATION AND CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  set_local_interrupt - set an interrupt flag
//  in the local register; maybe assert LINT
//-------------------------------------------------

void i82355_device::set_local_interrupt(u8 flag)
{
	m_local_status |= flag;
	if ((m_local_status & 0x18) == 0x10)
	{
		m_local_status |= 0x08;
		if (BIT(m_global_config, 2))
			m_lint_callback(1);
		else
			m_lint_callback(0);
	}
}


//-------------------------------------------------
//  clear_local_interrupt - clear an interrupt
//  flag in the local register; maybe clear LINT
//-------------------------------------------------

void i82355_device::clear_local_interrupt(u8 flag)
{
	m_local_status &= ~flag;
	if ((m_local_status & 0xf8) == 0x18)
	{
		m_local_status &= 0xf7;
		if (BIT(m_global_config, 2))
			m_lint_callback(0);
		else
			m_lint_callback(1);
	}
}


//-------------------------------------------------
//  set_system_interrupt - set an interrupt flag
//  in the system register; maybe assert EINT
//-------------------------------------------------

void i82355_device::set_system_interrupt(u8 flag)
{
	if (m_system_interrupt == 0x01)
	{
		if (BIT(m_global_config, 3))
			m_eint_callback(1);
		else
			m_eint_callback(0);
	}
	m_system_interrupt |= flag;
}


//-------------------------------------------------
//  clear_system_interrupt - clear an interrupt
//  flag in the system register; maybe clear EINT
//-------------------------------------------------

void i82355_device::clear_system_interrupt(u8 flag)
{
	if ((m_system_interrupt & flag) == 0)
		return;

	m_system_interrupt &= ~flag;
	if (m_system_interrupt == 0x01)
	{
		if (BIT(m_global_config, 3))
			m_eint_callback(0);
		else
			m_eint_callback(1);
	}
}


//-------------------------------------------------
//  global_config - handle writes to the global
//  configuration register
//-------------------------------------------------

void i82355_device::global_config(u8 data)
{
	// Handle changes in interrupt line polarity
	if (BIT(data, 3) != BIT(m_global_config, 3) && BIT(m_system_interrupt, 0))
	{
		if (BIT(data, 3))
			m_eint_callback((m_system_interrupt & 0xfe) != 0 ? 1 : 0);
		else
			m_eint_callback((m_system_interrupt & 0xfe) != 0 ? 0 : 1);
	}
	if (BIT(data, 2) != BIT(m_global_config, 2))
	{
		if (BIT(data, 2))
			m_lint_callback(BIT(m_local_status, 3) ? 1 : 0);
		else
			m_lint_callback(BIT(m_local_status, 3) ? 0 : 1);
	}

	// Revision code remains the same
	m_global_config = (m_global_config & 0xf0) | (data & 0x0f);
	logerror("%s: Global configuration = %02X\n", machine().describe_context(), m_global_config);
}


//-------------------------------------------------
//  identify_board - report board ID
//-------------------------------------------------

void i82355_device::identify_board()
{
	logerror("%s: Board identified as %c%c%c (%04X) product %02X.%02X\n", machine().describe_context(),
		((m_id.w.h & 0x7c00) >> 10) + 'A' - 1,
		((m_id.w.h & 0x03e0) >> 5) + 'A' - 1,
		(m_id.w.h & 0x001f) + 'A' - 1,
		m_id.w.h,
		m_id.b.h,
		m_id.b.l);
}


//**************************************************************************
//  PEEK/POKE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  peek_poke_control - handle writes to the
//  peek/poke control register
//-------------------------------------------------

void i82355_device::peek_poke_control(u8 data)
{
	const u32 lane_mask = (BIT(data, 3) ? 0xff000000 : 0)
		| (BIT(data, 2) ? 0x00ff0000 : 0)
		| (BIT(data, 1) ? 0x0000ff00 : 0)
		| (BIT(data, 0) ? 0x000000ff : 0);

	switch (data & 0x60)
	{
	case 0x40:
		logerror("%s: Requesting %s peek (read cycle) at %08X & %08X\n", machine().describe_context(),
			BIT(data, 4) ? "memory" : "I/O",
			m_peek_poke_address.d & 0xfffffffc, lane_mask);
		break;

	case 0x20:
		logerror("%s: Requesting %s poke (write cycle) at %08X & %08X\n", machine().describe_context(),
			BIT(data, 4) ? "memory" : "I/O",
			m_peek_poke_address.d & 0xfffffffc, lane_mask);
		break;

	case 0x60:
		logerror("%s: Requesting %s peek/poke (locked exchange) at %08X & %08X\n", machine().describe_context(),
			BIT(data, 4) ? "memory" : "I/O",
			m_peek_poke_address.d & 0xfffffffc, lane_mask);
		break;
	}

	m_peek_poke_control = data;
}

//**************************************************************************
//  DATA TRANSFER CHANNELS
//**************************************************************************

//-------------------------------------------------
//  transfer_config - handle writes to each data
//  transfer channel's configuration register
//-------------------------------------------------

void i82355_device::transfer_config(int channel, u8 data)
{
	logerror("%s: Data channel %d transfer configuration register = %02X\n", machine().describe_context(), channel, data);
	m_transfer_config[channel] = data;
}


//-------------------------------------------------
//  transfer_strobe - programmed strobe for either
//  data transfer channel
//-------------------------------------------------

void i82355_device::transfer_strobe(int channel)
{
	logerror("%s: Data channel %d transfer strobe\n", machine().describe_context(), channel);
}

//**************************************************************************
//  LOCAL REGISTER ACCESS
//**************************************************************************

//-------------------------------------------------
//  local_register_read - read data from a given
//  register from the local interface
//-------------------------------------------------

u8 i82355_device::local_register_read(u8 reg)
{
	switch (reg)
	{
	case 0x00:
		// ID register byte 0
		return m_id.b.h3;

	case 0x01:
		// ID register byte 1
		return m_id.b.h2;

	case 0x02:
		// ID register byte 2
		return m_id.b.h;

	case 0x03:
		// ID register byte 3
		return m_id.b.l;

	case 0x08:
		// Global configuration register
		return m_global_config;

	case 0x09:
		// System interrupt enable register
		return m_system_interrupt;

	case 0x0a:
	case 0x0b:
		// Semaphore ports
		return m_semaphore_flag[BIT(reg, 0)] | (m_semaphore_ltest[BIT(reg, 0)] << 1);

	case 0x0c:
		// Local doorbell enable register
		return m_local_doorbell_enable;

	case 0x0d:
		// Local doorbell status register
		return m_local_doorbell_status;

	case 0x0e:
		// EISA doorbell enable register
		return m_eisa_doorbell_enable;

	case 0x0f:
		// EISA doorbell status register
		return m_eisa_doorbell_status;

	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1a:
	case 0x1b:
	case 0x1c:
	case 0x1d:
	case 0x1e:
	case 0x1f:
		// Mailbox registers
		return m_mailbox[reg & 0x0f];

	case 0x30:
		// Peek data register byte 0
		return m_peek_poke_data.b.l;

	case 0x31:
		// Peek data register byte 1
		return m_peek_poke_data.b.h;

	case 0x32:
		// Peek data register byte 2
		return m_peek_poke_data.b.h2;

	case 0x33:
		// Peek data register byte 3
		return m_peek_poke_data.b.h3;

	case 0x34:
		// Peek/poke address register byte 0
		return m_peek_poke_address.b.l;

	case 0x35:
		// Peek/poke address register byte 1
		return m_peek_poke_address.b.h;

	case 0x36:
		// Peek/poke address register byte 2
		return m_peek_poke_address.b.h2;

	case 0x37:
		// Peek/poke address register byte 3
		return m_peek_poke_address.b.h3;

	case 0x38:
		// Peek/poke control register
		return m_peek_poke_control;

	case 0x39:
		// Range 0 I/O decode base address
		return m_io_decode_base[0];

	case 0x3a:
		// Range 0 I/O decode control address
		return m_io_decode_control[0];

	case 0x3b:
		// Range 1 I/O decode base address
		return m_io_decode_base[1];

	case 0x3c:
		// Range 1 I/O decode control address
		return m_io_decode_control[1];

	case 0x40:
	case 0x60:
		// Data transfer base count register byte 0
		return m_base_count[BIT(reg, 5)].b.l;

	case 0x41:
	case 0x61:
		// Data transfer base count register byte 1
		return m_base_count[BIT(reg, 5)].b.h;

	case 0x42:
	case 0x62:
		// Data transfer base count register byte 2
		return m_base_count[BIT(reg, 5)].b.h2;

	case 0x43:
	case 0x63:
		// Data transfer base address register byte 0
		return m_base_address[BIT(reg, 5)].b.l;

	case 0x44:
	case 0x64:
		// Data transfer base address register byte 1
		return m_base_address[BIT(reg, 5)].b.h;

	case 0x45:
	case 0x65:
		// Data transfer base address register byte 2
		return m_base_address[BIT(reg, 5)].b.h2;

	case 0x46:
	case 0x66:
		// Data transfer base address register byte 3
		return m_base_address[BIT(reg, 5)].b.h3;

	case 0x48:
	case 0x68:
		// Data transfer channel configuration register
		return m_transfer_config[BIT(reg, 5)];

	case 0x4a:
	case 0x6a:
		// Data transfer channel status register
		return m_transfer_status[BIT(reg, 5)];

	case 0x4b:
	case 0x6b:
		// TBI base register byte 0
		return m_tbi_base[BIT(reg, 5)].b.l;

	case 0x4c:
	case 0x6c:
		// TBI base register byte 1
		return m_tbi_base[BIT(reg, 5)].b.h;

	case 0x50:
	case 0x70:
		// Data transfer current count register byte 0
		return m_current_count[BIT(reg, 5)].b.l;

	case 0x51:
	case 0x71:
		// Data transfer current count register byte 1
		return m_current_count[BIT(reg, 5)].b.h;

	case 0x52:
	case 0x72:
		// Data transfer current count register byte 2
		return m_current_count[BIT(reg, 5)].b.h2;

	case 0x53:
	case 0x73:
		// Data transfer current address register byte 0
		return m_current_address[BIT(reg, 5)].b.l;

	case 0x54:
	case 0x74:
		// Data transfer current address register byte 1
		return m_current_address[BIT(reg, 5)].b.h;

	case 0x55:
	case 0x75:
		// Data transfer current address register byte 2
		return m_current_address[BIT(reg, 5)].b.h2;

	case 0x56:
	case 0x76:
		// Data transfer current address register byte 3
		return m_current_address[BIT(reg, 5)].b.h3;

	case 0x5b:
	case 0x7b:
		// TBI current register byte 0
		return m_tbi_current[BIT(reg, 5)].b.l;

	case 0x5c:
	case 0x7c:
		// TBI current register byte 1
		return m_tbi_current[BIT(reg, 5)].b.h;

	default:
		if (!machine().side_effects_disabled())
			logerror("%s: Local read from undefined register %02X\n", machine().describe_context(), reg);
		return 0;
	}
}


//-------------------------------------------------
//  local_register_write - write data to a given
//  register from the local interface
//-------------------------------------------------

void i82355_device::local_register_write(u8 reg, u8 data)
{
	switch (reg)
	{
	case 0x00:
		// ID register byte 0 (manufacturer's code, first and second portions
		// According to Intel, this should be written after the other 3 ID bytes, but manufacturers don't agree
		if ((std::exchange(m_id.b.h3, data) & 0xf0) == 0x70 && m_id.w.l != 0x0000)
			identify_board();
		break;

	case 0x01:
		// ID register byte 1 (manufacturer's code, second and third portions)
		m_id.b.h2 = data;
		break;

	case 0x02:
		// ID register byte 2 (product number)
		m_id.b.h = data;
		break;

	case 0x03:
		// ID register byte 3 (product revision)
		m_id.b.l = data;
		if ((m_id.b.h3 & 0xf0) != 0x70)
			identify_board();
		break;

	case 0x08:
		// Global configuration register
		global_config(data);
		break;

	case 0x0a:
	case 0x0b:
		// Semaphore ports
		m_semaphore_ltest[BIT(reg, 0)] = std::exchange(m_semaphore_flag[BIT(reg, 0)], BIT(data, 0));
		break;

	case 0x0c:
		// Local doorbell enable register
		m_local_doorbell_enable = data;
		if ((m_local_doorbell_status & m_local_doorbell_enable) != 0x00)
			set_local_interrupt(0x80);
		else
			clear_local_interrupt(0x80);
		break;

	case 0x0d:
		// Local doorbell interrupt register (reset bits)
		m_local_doorbell_status &= ~data;
		if ((m_local_doorbell_status & m_local_doorbell_enable) == 0x00)
			clear_local_interrupt(0x80);
		break;

	case 0x0f:
		// EISA doorbell interrupt register (set bits)
		m_eisa_doorbell_status |= data;
		if ((m_eisa_doorbell_status & m_eisa_doorbell_enable) != 0x00)
			set_system_interrupt(0x02);
		break;

	case 0x10:
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1a:
	case 0x1b:
	case 0x1c:
	case 0x1d:
	case 0x1e:
	case 0x1f:
		// Mailbox registers
		m_mailbox[reg & 0x0f] = data;
		break;

	case 0x30:
		// Poke data register byte 0
		m_peek_poke_data.b.l = data;
		break;

	case 0x31:
		// Poke data register byte 1
		m_peek_poke_data.b.h = data;
		break;

	case 0x32:
		// Poke data register byte 2
		m_peek_poke_data.b.h2 = data;
		break;

	case 0x33:
		// Poke data register byte 3
		m_peek_poke_data.b.h3 = data;
		break;

	case 0x34:
		// Peek/poke address register byte 0 (lowest 2 bits unused)
		m_peek_poke_address.b.l = data;
		break;

	case 0x35:
		// Peek/poke address register byte 1
		m_peek_poke_address.b.h = data;
		break;

	case 0x36:
		// Peek/poke address register byte 2
		m_peek_poke_address.b.h2 = data;
		break;

	case 0x37:
		// Peek/poke address register byte 3
		m_peek_poke_address.b.h3 = data;
		break;

	case 0x38:
		// Peek/poke control register
		peek_poke_control(data);
		break;

	case 0x39:
		// Range 0 I/O decode base address
		m_io_decode_base[0] = data;
		break;

	case 0x3a:
		// Range 0 I/O decode control address
		m_io_decode_control[0] = data;
		break;

	case 0x3b:
		// Range 1 I/O decode base address
		m_io_decode_base[1] = data;
		break;

	case 0x3c:
		// Range 1 I/O decode control address
		m_io_decode_control[1] = data;
		break;

	case 0x40:
	case 0x60:
		// Data transfer base count register byte 0
		m_base_count[BIT(reg, 5)].b.l = data;
		break;

	case 0x41:
	case 0x61:
		// Data transfer base count register byte 1
		m_base_count[BIT(reg, 5)].b.h = data;
		break;

	case 0x42:
	case 0x62:
		// Data transfer base count register byte 2
		m_base_count[BIT(reg, 5)].b.h2 = data;
		break;

	case 0x43:
	case 0x63:
		// Data transfer base address register byte 0
		m_base_address[BIT(reg, 5)].b.l = data;
		break;

	case 0x44:
	case 0x64:
		// Data transfer base address register byte 1
		m_base_address[BIT(reg, 5)].b.h = data;
		break;

	case 0x45:
	case 0x65:
		// Data transfer base address register byte 2
		m_base_address[BIT(reg, 5)].b.h2 = data;
		break;

	case 0x46:
	case 0x66:
		// Data transfer base address register byte 3
		m_base_address[BIT(reg, 5)].b.h3 = data;
		break;

	case 0x48:
	case 0x68:
		// Data transfer channel configuration register
		transfer_config(BIT(reg, 5), data);
		break;

	case 0x49:
	case 0x69:
		// Data transfer strobe registers (data written is ignored)
		transfer_strobe(BIT(reg, 5));
		break;

	case 0x4a:
	case 0x6a:
		// Data transfer channel status register (reset bits)
		m_transfer_status[BIT(reg, 5)] &= ~(data & 0x03);
		break;

	default:
		logerror("%s: Local write to undefined or read-only register %02X (%02X)\n", machine().describe_context(), reg, data);
		break;
	}
}


//-------------------------------------------------
//  local_r - read data onto the local CPU bus
//-------------------------------------------------

u8 i82355_device::local_r(offs_t offset)
{
	switch (offset & 3)
	{
	case 0:
	{
		u8 result = local_register_read(m_local_index & 0x7f);

		// Autoincrement mode
		if (BIT(m_local_index, 7) && !machine().side_effects_disabled())
			m_local_index = (m_local_index + 1) | 0x80;

		return result;
	}

	case 1:
		return m_local_index;

	case 2:
		return m_local_status;

	default:
		if (!machine().side_effects_disabled())
			logerror("%s: Local read from reserved register\n", machine().describe_context());
		return 0;
	}
}


//-------------------------------------------------
//  local_w - write data from the local CPU bus
//-------------------------------------------------

void i82355_device::local_w(offs_t offset, u8 data)
{
	switch (offset & 3)
	{
	case 0:
		local_register_write(m_local_index & 0x7f, data);

		// Autoincrement mode
		if (BIT(m_local_index, 7) && !machine().side_effects_disabled())
			m_local_index = (m_local_index + 1) | 0x80;

		break;

	case 1:
		m_local_index = data;
		break;

	case 2:
		// Local control register (all but one bit is read-only)
		if (BIT(data, 4))
			m_local_status |= 0x10;
		else
		{
			m_local_status &= 0xef;
			clear_local_interrupt(0xe0);
		}
		break;

	default:
		logerror("%s: Local write to reserved register (%02X)\n", machine().describe_context(), data);
		break;
	}
}
