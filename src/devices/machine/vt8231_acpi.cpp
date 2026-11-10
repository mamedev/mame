// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    VIA VT8231 South Bridge - ACPI

***************************************************************************/

#include "emu.h"
#include "vt8231_acpi.h"

#define LOG_MAPPING (1U << 1)
#define LOG_REG     (1U << 2)
#define LOG_SMBUS   (1U << 3)

#define VERBOSE (LOG_GENERAL | LOG_MAPPING | LOG_REG | LOG_SMBUS)
#include "logmacro.h"


static constexpr const char* const SMBUS_CMD_NAMES[] =
{
	"Quick Read/Write",
	"Byte Read/Write",
	"Byte Data Read/Write",
	"Word Data Read/Write",
	"Reserved",
	"Block Read/Write",
	"Reserved",
	"Reserved"
};

DEFINE_DEVICE_TYPE(VT8231_ACPI, vt8231_acpi_device, "vt8231_acpi", "VT8231 South Bridge - ACPI")

vt8231_acpi_device::vt8231_acpi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)   :
	pci_device(mconfig, VT8231_ACPI, tag, owner, clock),
	m_smbus_clock(*this, "smbus_clock"),
	m_smbck1_cb(*this), m_smbdt1_cb(*this),
	m_initialized(false)
{
	set_ids(0x11068235, 0x10, 0x000000, 0x00000000);
}

void vt8231_acpi_device::device_start()
{
	pci_device::device_start();

	// register for save states
	save_item(NAME(m_general_configuration_0));
	save_item(NAME(m_power_well_control));
	save_item(NAME(m_smbus_iobase));
	save_item(NAME(m_smbus_iobase_mapped));
	save_item(NAME(m_smbus_host_config));
	save_item(NAME(m_smbus_host_status));
	save_item(NAME(m_smbus_host_control));
	save_item(NAME(m_smbus_host_command));
	save_item(NAME(m_smbus_host_command_latch));
	save_item(NAME(m_smbus_host_address));
	save_item(NAME(m_smbus_host_address_latch));
	save_item(NAME(m_smbus_host_data0));
	save_item(NAME(m_smbus_host_data0_latch));
	save_item(NAME(m_smbus_state));
	save_item(NAME(m_smbus_bit));
	save_item(NAME(m_smbus_dt1));
	save_item(NAME(m_initialized));
}

void vt8231_acpi_device::device_reset()
{
	pci_device::device_reset();

	m_general_configuration_0 = 0x00;
	m_power_well_control = 0x80;
	m_smbus_iobase = 0x00000000;
	m_smbus_iobase_mapped = ~0;
	m_smbus_host_config = 0x00;
	m_smbus_host_status = 0x00;
	m_smbus_host_control = 0x00;
	m_smbus_host_command = 0x00;
	m_smbus_host_command_latch = 0x00;
	m_smbus_host_address = 0x00;
	m_smbus_host_address_latch = 0x00;
	m_smbus_host_data0 = 0x00;
	m_smbus_host_data0_latch = 0x00;

	m_smbus_clock->set_period(attotime::never);
	m_smbus_state = SMBUS_STATE_IDLE;
	m_smbus_bit = 0;
	m_smbus_dt1 = true;

	m_initialized = true;

	remap_cb();

	// smbus idle line state
	m_smbck1_cb(1);
	m_smbdt1_cb(1);
}

void vt8231_acpi_device::device_add_mconfig(machine_config &config)
{
	CLOCK(config, m_smbus_clock);
	m_smbus_clock->signal_handler().set(FUNC(vt8231_acpi_device::smbus_clock_w));
}

void vt8231_acpi_device::map_device(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
								   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	if (!m_initialized)
		return;

	LOGMASKED(LOG_MAPPING, "Mapping ACPI\n");

	if (BIT(m_smbus_host_config, 0))
	{
		const uint64_t start = m_smbus_iobase + io_offset;
		const uint64_t end = start + 0x10 - 1;
		if (start >= io_window_start && end <= io_window_end)
			io_space->install_device(start, end, *this, &vt8231_acpi_device::smbus_map);
		m_smbus_iobase_mapped = m_smbus_iobase;
		LOGMASKED(LOG_MAPPING, "-> SMBus %08x-%08x\n", start, end);
	}
	else
	{
		if (m_smbus_iobase_mapped != ~0)
		{
			LOGMASKED(LOG_MAPPING, "-> Unmapping SMBus\n");
			const uint64_t start = m_smbus_iobase_mapped + io_offset;
			const uint64_t end = start + 0x10 - 1;
			if (start >= io_window_start && end <= io_window_end)
				io_space->unmap_readwrite(start, end);
			m_smbus_iobase_mapped = ~0;
		}
	}
}

void vt8231_acpi_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x40, 0x40).rw(FUNC(vt8231_acpi_device::general_configuration_0_r), FUNC(vt8231_acpi_device::general_configuration_0_w));
	map(0x54, 0x54).w(FUNC(vt8231_acpi_device::power_well_control_w));
	map(0x90, 0x93).rw(FUNC(vt8231_acpi_device::smbus_iobase_r), FUNC(vt8231_acpi_device::smbus_iobase_w));
	map(0xd2, 0xd2).rw(FUNC(vt8231_acpi_device::smbus_host_config_r), FUNC(vt8231_acpi_device::smbus_host_config_w));
}


//**************************************************************************
//  POWER MANAGEMENT
//**************************************************************************

uint8_t vt8231_acpi_device::general_configuration_0_r()
{
	return m_general_configuration_0;
}

void vt8231_acpi_device::general_configuration_0_w(uint8_t data)
{
	LOGMASKED(LOG_REG, "general_configuration_0_w: %02x\n", data);
	m_general_configuration_0 = data;
}

void vt8231_acpi_device::power_well_control_w(uint8_t data)
{
	LOGMASKED(LOG_REG, "m_power_well_control: %02x\n", data);
	m_power_well_control = data;
}


//**************************************************************************
//  SMBUS
//**************************************************************************

void vt8231_acpi_device::smbus_map(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(vt8231_acpi_device::smbus_host_status_r), FUNC(vt8231_acpi_device::smbus_host_status_w));
	map(0x02, 0x02).rw(FUNC(vt8231_acpi_device::smbus_host_control_r), FUNC(vt8231_acpi_device::smbus_host_control_w));
	map(0x03, 0x03).rw(FUNC(vt8231_acpi_device::smbus_host_command_r), FUNC(vt8231_acpi_device::smbus_host_command_w));
	map(0x04, 0x04).rw(FUNC(vt8231_acpi_device::smbus_host_address_r), FUNC(vt8231_acpi_device::smbus_host_address_w));
	map(0x05, 0x05).rw(FUNC(vt8231_acpi_device::smbus_host_data0_r), FUNC(vt8231_acpi_device::smbus_host_data0_w));
}

uint32_t vt8231_acpi_device::smbus_iobase_r(offs_t offset, uint32_t mem_mask)
{
	return m_smbus_iobase | 1;
}

void vt8231_acpi_device::smbus_iobase_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_REG, "smbus_iobase_w: %08x\n", data);
	mem_mask &= 0x0000fff0;
	COMBINE_DATA(&m_smbus_iobase);
	remap_cb();
}

uint8_t vt8231_acpi_device::smbus_host_config_r()
{
	return m_smbus_host_config;
}

void vt8231_acpi_device::smbus_host_config_w(uint8_t data)
{
	LOGMASKED(LOG_REG, "smbus_host_config_w: %02x\n", data);
	m_smbus_host_config = data;
	remap_cb();
}

uint8_t vt8231_acpi_device::smbus_host_status_r()
{
	//LOGMASKED(LOG_REG, "smbus_host_status_r\n");
	return m_smbus_host_status;
}

void vt8231_acpi_device::smbus_host_status_w(uint8_t data)
{
	LOGMASKED(LOG_REG, "smbus_host_status_w: %02x\n", data);
	m_smbus_host_status &= ~(data & 0x1e); // bits 4 to 1 can be cleared
}

uint8_t vt8231_acpi_device::smbus_host_control_r()
{
	LOGMASKED(LOG_REG, "smbus_host_control_r\n");
	return m_smbus_host_control;
}

void vt8231_acpi_device::smbus_host_control_w(uint8_t data)
{
	LOGMASKED(LOG_REG, "smbus_host_control_w: %02x\n", data);
	m_smbus_host_control = data & 0x1d;

	if (BIT(data, 1))
	{
		// kill transaction
		m_smbus_host_status |= 0x10; // failed (TODO: interrupt if enabled)
		m_smbus_host_status &= ~0x01; // no longer busy
		m_smbus_state = SMBUS_STATE_IDLE;
		m_smbus_clock->set_period(attotime::never);

		m_smbdt1_cb(1); // release SDA
		m_smbck1_cb(1); // release SCL
	}

	if (BIT(data, 6))
	{
		uint8_t const cmd = (m_smbus_host_control >> 2) & 0x07;
		LOG("SMBus executing command: %s\n", SMBUS_CMD_NAMES[cmd]);

		// we only support one command right now
		if (cmd != SMBUS_CMD_BYTE_DATA_RW || !BIT(m_smbus_host_address, 0))
		{
			// reserved (or unimplemented) command
			LOG("-> Aborted (command reserved or unimplemented)\n");
			m_smbus_host_status |= 0x04; // device error (TODO: interrupt if enabled)
		}
		else
		{
			m_smbus_host_status |= 0x01; // busy
			m_smbus_host_address_latch = m_smbus_host_address;
			m_smbus_host_command_latch = m_smbus_host_command;
			m_smbus_host_data0_latch = 0;
			m_smbus_bit = 0;
			m_smbus_clock->set_period(attotime::from_hz(32'768));
		}
	}
}

uint8_t vt8231_acpi_device::smbus_host_command_r()
{
	LOGMASKED(LOG_REG, "smbus_host_command_r\n");
	return m_smbus_host_command;
}

void vt8231_acpi_device::smbus_host_command_w(uint8_t data)
{
	LOGMASKED(LOG_REG, "smbus_host_command_w: %02x\n", data);
	m_smbus_host_command = data;
}

uint8_t vt8231_acpi_device::smbus_host_address_r()
{
	LOGMASKED(LOG_REG, "smbus_host_address_r\n");
	return m_smbus_host_address;
}

void vt8231_acpi_device::smbus_host_address_w(uint8_t data)
{
	LOGMASKED(LOG_REG, "smbus_host_address_w: %02x\n", data);
	m_smbus_host_address = data;
}

uint8_t vt8231_acpi_device::smbus_host_data0_r()
{
	LOGMASKED(LOG_REG, "smbus_host_data0_r\n");
	return m_smbus_host_data0;
}

void vt8231_acpi_device::smbus_host_data0_w(uint8_t data)
{
	LOGMASKED(LOG_REG, "smbus_host_data0_w: %02x\n", data);
	m_smbus_host_data0 = data;
}

void vt8231_acpi_device::smbus_clock_w(int state)
{
	// save sda at this point, writing the clock may update it
	const bool sda = m_smbus_dt1;

	if (m_smbus_state != SMBUS_STATE_IDLE)
		m_smbck1_cb(state);

	switch (m_smbus_state)
	{
		case SMBUS_STATE_IDLE:
			if (state)
			{
				LOGMASKED(LOG_SMBUS, "Starting SMBus command\n");
				m_smbdt1_cb(0);
				m_smbus_host_address_latch &= 0xfe; // address write
				m_smbus_state = SMBUS_STATE_SEND_ADDRESS_W;
			}
			break;
		case SMBUS_STATE_SEND_ADDRESS_W:
			if (state)
			{
				m_smbus_bit++;
			}
			else
			{
				if (m_smbus_bit == 8)
				{
					m_smbdt1_cb(1); // release SDA for the slave's ACK
					LOGMASKED(LOG_SMBUS, "Address W sent, wait for ACK\n");
					m_smbus_bit = 0;
					m_smbus_state = SMBUS_STATE_WAIT_ADDRESS_W_ACK;
				}
				else
				{
					m_smbdt1_cb(BIT(m_smbus_host_address_latch, 7 - m_smbus_bit));
				}
			}
			break;
		case SMBUS_STATE_WAIT_ADDRESS_W_ACK:
			if (state)
			{
				if (!sda)
				{
					LOGMASKED(LOG_SMBUS, "ACK found\n");
					m_smbus_state = SMBUS_STATE_SEND_CMD;
				}
				else
				{
					LOGMASKED(LOG_SMBUS, "ACK not found, aborting\n");
					m_smbus_host_status |= 0x04; // device error
					m_smbus_state = SMBUS_STATE_SEND_STOP1;
				}
			}
			break;
		case SMBUS_STATE_SEND_CMD:
			if (state)
			{
				m_smbus_bit++;
			}
			else
			{
				if (m_smbus_bit == 8)
				{
					m_smbdt1_cb(1); // release SDA for the slave's ACK
					LOGMASKED(LOG_SMBUS, "Command sent, wait for ACK\n");
					m_smbus_bit = 0;
					m_smbus_state = SMBUS_STATE_WAIT_CMD_ACK;
				}
				else
				{
					m_smbdt1_cb(BIT(m_smbus_host_command_latch, 7 - m_smbus_bit));
				}
			}
			break;
		case SMBUS_STATE_WAIT_CMD_ACK:
			if (state)
			{
				if (!sda)
				{
					LOGMASKED(LOG_SMBUS, "ACK found\n");
					m_smbus_state = SMBUS_STATE_REPEAT_START1;
				}
				else
				{
					LOGMASKED(LOG_SMBUS, "ACK not found, aborting\n");
					m_smbus_host_status |= 0x04; // device error
					m_smbus_state = SMBUS_STATE_SEND_STOP1;
				}
			}
			break;
		case SMBUS_STATE_REPEAT_START1:
			if (!state)
			{
				m_smbdt1_cb(1);
				m_smbus_state = SMBUS_STATE_REPEAT_START2;
			}
			break;
		case SMBUS_STATE_REPEAT_START2:
			if (state)
			{
				m_smbdt1_cb(0); // repeated START: SDA falls while SCL is high
				m_smbus_host_address_latch |= 0x01; // address read
				m_smbus_state = SMBUS_STATE_SEND_ADDRESS_R;
			}
			break;
		case SMBUS_STATE_SEND_ADDRESS_R:
			if (state)
			{
				m_smbus_bit++;
			}
			else
			{
				if (m_smbus_bit == 8)
				{
					m_smbdt1_cb(1); // release SDA for the slave's ACK
					LOG("Address R sent, wait for ACK\n");
					m_smbus_bit = 0;
					m_smbus_state = SMBUS_STATE_WAIT_ADDRESS_R_ACK;
				}
				else
				{
					m_smbdt1_cb(BIT(m_smbus_host_address_latch, 7 - m_smbus_bit));
				}
			}
			break;
		case SMBUS_STATE_WAIT_ADDRESS_R_ACK:
			if (state)
			{
				if (!sda)
				{
					LOGMASKED(LOG_SMBUS, "ACK found\n");
					m_smbus_state = SMBUS_STATE_BYTE_READ;
				}
				else
				{
					LOGMASKED(LOG_SMBUS, "ACK not found, aborting\n");
					m_smbus_host_status |= 0x04; // device error
					m_smbus_state = SMBUS_STATE_SEND_STOP1;
				}
			}
			break;
		case SMBUS_STATE_BYTE_READ:
			if (state)
			{
				m_smbus_bit++;
			}
			else
			{
				if (m_smbus_bit > 0)
					m_smbus_host_data0_latch = (m_smbus_host_data0_latch << 1) | sda;

				if (m_smbus_bit == 8)
				{
					m_smbus_bit = 0;
					m_smbus_host_data0 = m_smbus_host_data0_latch;
					m_smbus_state = SMBUS_STATE_SEND_NACK;
					LOGMASKED(LOG_SMBUS, "Received byte: %02x\n", m_smbus_host_data0);
				}
			}
			break;
		case SMBUS_STATE_SEND_NACK:
			if (state)
			{
				m_smbdt1_cb(1);
				m_smbus_state = SMBUS_STATE_SEND_STOP1;
			}
			break;
		case SMBUS_STATE_SEND_STOP1:
			if (!state)
			{
				m_smbdt1_cb(0);
				m_smbus_state = SMBUS_STATE_SEND_STOP2;
			}
			break;
		case SMBUS_STATE_SEND_STOP2:
			if (state)
			{
				LOGMASKED(LOG_SMBUS, "Command complete\n");
				m_smbdt1_cb(1); // STOP: SDA rises while SCL is high
				m_smbus_host_status &= ~0x01; // no longer busy
				if (!BIT(m_smbus_host_status, 2))
					m_smbus_host_status |= 0x02; // command complete
				m_smbus_state = SMBUS_STATE_IDLE;
				m_smbus_clock->set_period(attotime::never);
			}
			break;
	}
}
