// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    VIA VT8231 South Bridge - PCI to ISA Bridge

***************************************************************************/

#include "emu.h"
#include "vt8231_isa.h"

#define LOG_MAPPING (1U << 1)
#define LOG_REG     (1U << 2)
#define LOG_SUPERIO (1U << 3)

#define VERBOSE (LOG_GENERAL | LOG_MAPPING | LOG_REG | LOG_SUPERIO)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(VT8231_ISA, vt8231_isa_device, "vt8231_isa", "VT8231 South Bridge - PCI to ISA Bridge")

vt8231_isa_device::vt8231_isa_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)   :
	pci_device(mconfig, VT8231_ISA, tag, owner, clock),
	m_com1(*this, "com1"),
	m_com1_txd_cb(*this), m_com1_dtr_cb(*this), m_com1_rts_cb(*this),
	m_initialized(false)
{
	set_ids(0x11068231, 0x00, 0x060100, 0x00000000);
}

void vt8231_isa_device::device_add_mconfig(machine_config &config)
{
	NS16550(config, m_com1, 1'843'200);
	m_com1->out_tx_callback().set(FUNC(vt8231_isa_device::com1_txd_w));
	m_com1->out_dtr_callback().set(FUNC(vt8231_isa_device::com1_dtr_w));
	m_com1->out_rts_callback().set(FUNC(vt8231_isa_device::com1_rts_w));
}

void vt8231_isa_device::device_start()
{
	pci_device::device_start();

	// register for save states (TODO)
}

void vt8231_isa_device::device_reset()
{
	pci_device::device_reset();

	// setup superio configuration defaults
	std::fill_n(m_superio_cfg, 0x10, 0x00);
	m_superio_cfg[0x00] = 0x3c; // device id
	m_superio_cfg[0x01] = 0x01; // revision
	m_superio_cfg[0x02] = 0x03; // function select
	m_superio_cfg[0x04] = 0xfe; // serial port base address (3f8)
	m_superio_cfg[0x06] = 0xde; // parallel port base address (378)
	m_superio_cfg[0x07] = 0xfc; // floppy controller base address (3f0)

	m_baud_divisor = 0x01; // 115200 baud

	m_initialized = true;
	remap_cb();
}

void vt8231_isa_device::map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
									 uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space)
{
	if (!m_initialized)
		return;

	io_space->install_device(0x000, 0x7ff, *this, &vt8231_isa_device::io_map);

	// serial port enabled?
	if (BIT(m_superio_cfg[0x02], 2) == 1)
	{
		uint16_t com1_base = m_superio_cfg[0x04] << 2;
		LOGMASKED(LOG_MAPPING, "Mapping COM1 from %04x to %04x\n", com1_base, com1_base + 0x0f);
		io_space->install_device(com1_base, com1_base + 0x0f, *this, &vt8231_isa_device::com1_map);
	}
}

void vt8231_isa_device::config_map(address_map &map)
{
	pci_device::config_map(map);
	map(0x50, 0x50).rw(FUNC(vt8231_isa_device::function_control_1_r), FUNC(vt8231_isa_device::function_control_1_w));
}

uint8_t vt8231_isa_device::function_control_1_r()
{
	// 7-------  mc97 enable
	// -6------  ac97 enable
	// --5-----  usb enable
	// ---4----  usb enable
	// ----3---  ide enable
	// -----2--  super io configuration enable
	// ------1-  super io enable
	// -------0  internal audio enable

	return 0x00;
}

void vt8231_isa_device::function_control_1_w(uint8_t data)
{
	LOGMASKED(LOG_REG, "function_control_1_w: %02x\n", data);
}

void vt8231_isa_device::io_map(address_map &map)
{
	map(0x3f0, 0x3f0).rw(FUNC(vt8231_isa_device::superio_cfg_idx_r), FUNC(vt8231_isa_device::superio_cfg_idx_w));
	map(0x3f1, 0x3f1).rw(FUNC(vt8231_isa_device::superio_cfg_data_r), FUNC(vt8231_isa_device::superio_cfg_data_w));
}


//**************************************************************************
//  SUPER IO
//**************************************************************************

uint8_t vt8231_isa_device::superio_cfg_idx_r()
{
	return m_superio_cfg_idx;
}

void vt8231_isa_device::superio_cfg_idx_w(uint8_t data)
{
	LOGMASKED(LOG_SUPERIO, "superio_cfg_idx_w: %02x\n", data);
	m_superio_cfg_idx = data & 0x0f;
}

uint8_t vt8231_isa_device::superio_cfg_data_r()
{
	return m_superio_cfg[m_superio_cfg_idx];
}

void vt8231_isa_device::superio_cfg_data_w(uint8_t data)
{
	LOGMASKED(LOG_SUPERIO, "superio_cfg_data_w: %02x\n", data);

	// first two registers are read-only
	if (m_superio_cfg_idx < 2)
		return;

	m_superio_cfg[m_superio_cfg_idx] = data;
	remap_cb();

	switch (m_superio_cfg_idx)
	{
	case 0x02:
		// 765-----  reserved
		// ---4----  floppy controller enable
		// ----3---  reserved
		// -----2--  serial port enable
		// ------10  parellel port enable/mode
		LOGMASKED(LOG_SUPERIO, "SuperIO function select\n");
		break;

	case 0x03:
		LOGMASKED(LOG_SUPERIO, "SuperIO power down control\n");
		break;

	case 0x04:
		LOGMASKED(LOG_SUPERIO, "SuperIO serial port i/o base address\n");
		break;

	case 0x06:
		LOGMASKED(LOG_SUPERIO, "SuperIO parallel port i/o base address\n");
		break;

	case 0x07:
		LOGMASKED(LOG_SUPERIO, "SuperIO floppy controller base address\n");
		break;

	case 0x09:
		LOGMASKED(LOG_SUPERIO, "SuperIO serial port control\n");
		break;

	case 0x0a:
		LOGMASKED(LOG_SUPERIO, "SuperIO parallel port control\n");
		break;

	case 0x0b:
		LOGMASKED(LOG_SUPERIO, "SuperIO floppy controller control\n");
		break;

	case 0x0c:
		LOGMASKED(LOG_SUPERIO, "SuperIO floppy controller drive type\n");
		break;

	case 0x0e:
		LOGMASKED(LOG_SUPERIO, "SuperIO test mode a\n");
		break;

	case 0x0f:
		LOGMASKED(LOG_SUPERIO, "SuperIO test mode b\n");
		break;

	default:
		LOGMASKED(LOG_SUPERIO, "SuperIO reserved register\n");
		break;
	}
}


//**************************************************************************
//  SERIAL PORT
//**************************************************************************

void vt8231_isa_device::com1_map(address_map &map)
{
	map(0x00, 0x07).rw("com1", FUNC(ins8250_device::ins8250_r), FUNC(ins8250_device::ins8250_w));
	map(0x08, 0x09).rw(FUNC(vt8231_isa_device::com1_baud_r), FUNC(vt8231_isa_device::com1_baud_w));
}

uint16_t vt8231_isa_device::com1_baud_r()
{
	return m_baud_divisor;
}

void vt8231_isa_device::com1_baud_w(uint16_t data)
{
	logerror("com1_baud_w: %04x\n", data);
	m_baud_divisor = data;

	// TODO: setting this to 0 might disable it?
	if (data > 0)
		m_com1->set_clock_scale(1.0 / data);
}
