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
	m_pic(*this, "pic%u", 0U),
	m_com1(*this, "com1"),
	m_intr_cb(*this),
	m_com1_txd_cb(*this), m_com1_dtr_cb(*this), m_com1_rts_cb(*this),
	m_initialized(false)
{
	set_ids(0x11068231, 0x10, 0x060100, 0x00000000);
	set_multifunction_device(true);
}

void vt8231_isa_device::device_add_mconfig(machine_config &config)
{
	PIC8259(config, m_pic[0]);
	m_pic[0]->in_sp_callback().set_constant(1);
	m_pic[0]->out_int_callback().set([this] (int state) { m_intr_cb(state); });
	m_pic[0]->read_slave_ack_callback().set(FUNC(vt8231_isa_device::pic_slave_ack_r));

	PIC8259(config, m_pic[1]);
	m_pic[1]->in_sp_callback().set_constant(0);
	m_pic[1]->out_int_callback().set(m_pic[0], FUNC(pic8259_device::ir2_w));

	NS16550(config, m_com1, 1'843'200);
	m_com1->out_int_callback().set(FUNC(vt8231_isa_device::com1_int_w));
	m_com1->out_tx_callback().set(FUNC(vt8231_isa_device::com1_txd_w));
	m_com1->out_dtr_callback().set(FUNC(vt8231_isa_device::com1_dtr_w));
	m_com1->out_rts_callback().set(FUNC(vt8231_isa_device::com1_rts_w));
}

void vt8231_isa_device::device_start()
{
	pci_device::device_start();

	// register for save states
	save_item(NAME(m_isa_pnp_routing_1));
	save_item(NAME(m_isa_pnp_routing_2));
	save_item(NAME(m_isa_pnp_routing_3));
	save_item(NAME(m_function_control_1));
	save_item(NAME(m_misc_functions));
	save_item(NAME(m_superio_cfg));
	save_item(NAME(m_superio_cfg_idx));
	save_item(NAME(m_baud_divisor));
	save_item(NAME(m_initialized));
}

void vt8231_isa_device::device_reset()
{
	pci_device::device_reset();

	// configuration space defaults
	m_isa_pnp_routing_1 = 0x00;
	m_isa_pnp_routing_2 = 0x00;
	m_isa_pnp_routing_3 = 0x00;
	m_function_control_1 = 0x09;
	m_misc_functions = 0x00;

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
	map(0x45, 0x45).rw(FUNC(vt8231_isa_device::isa_pnp_routing_1_r), FUNC(vt8231_isa_device::isa_pnp_routing_1_w));
	map(0x46, 0x46).rw(FUNC(vt8231_isa_device::isa_pnp_routing_2_r), FUNC(vt8231_isa_device::isa_pnp_routing_2_w));
	map(0x47, 0x47).rw(FUNC(vt8231_isa_device::isa_pnp_routing_3_r), FUNC(vt8231_isa_device::isa_pnp_routing_3_w));
	map(0x50, 0x50).rw(FUNC(vt8231_isa_device::function_control_1_r), FUNC(vt8231_isa_device::function_control_1_w));
}

uint8_t vt8231_isa_device::isa_pnp_routing_1_r()
{
	return m_isa_pnp_routing_1;
}

void vt8231_isa_device::isa_pnp_routing_1_w(uint8_t data)
{
	LOGMASKED(LOG_REG, "isa_pnp_routing_1_w: %02x\n", data);
	m_isa_pnp_routing_1 = data;
}

uint8_t vt8231_isa_device::isa_pnp_routing_2_r()
{
	return m_isa_pnp_routing_2;
}

void vt8231_isa_device::isa_pnp_routing_2_w(uint8_t data)
{
	LOGMASKED(LOG_REG, "isa_pnp_routing_2_w: %02x\n", data);
	m_isa_pnp_routing_2 = data & 0x0f;
}

uint8_t vt8231_isa_device::isa_pnp_routing_3_r()
{
	return m_isa_pnp_routing_3;
}

void vt8231_isa_device::isa_pnp_routing_3_w(uint8_t data)
{
	LOGMASKED(LOG_REG, "isa_pnp_routing_3_w: %02x\n", data);
	m_isa_pnp_routing_3 = data & 0x0f;
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

	return m_function_control_1;
}

void vt8231_isa_device::function_control_1_w(uint8_t data)
{
	LOGMASKED(LOG_REG, "function_control_1_w: %02x\n", data);
	m_function_control_1 = data & 0xfe;
}

void vt8231_isa_device::io_map(address_map &map)
{
	map(0x000, 0x00f).mirror(0x10).unmaprw(); // master dma controller
	map(0x020, 0x021).mirror(0x1e).rw(m_pic[0], FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	map(0x040, 0x043).mirror(0x1c).unmaprw(); // timer/counter
	//map(0x060, 0x060).mirror(0x0a).unmaprw(); // kbd data
	map(0x061, 0x061).mirror(0x0e).rw(FUNC(vt8231_isa_device::misc_functions_r), FUNC(vt8231_isa_device::misc_functions_w));
	//map(0x064, 0x064).mirror(0x0a).noprw(); // kbd command/status
	//map(0x070, 0x077).unmaprw(); // rtc/cmos/nmi disable
	//map(0x081, 0x08f).unmaprw(); // dma page register
	//map(0x092, 0x092).unmaprw(); // system control
	map(0x0a0, 0x0a1).mirror(0x1e).rw(m_pic[1], FUNC(pic8259_device::read), FUNC(pic8259_device::write));
	//map(0x0c0, 0x0de).mirror(0x01).unmaprw(); // slave dma controller
	map(0x3f0, 0x3f0).rw(FUNC(vt8231_isa_device::superio_cfg_idx_r), FUNC(vt8231_isa_device::superio_cfg_idx_w));
	map(0x3f1, 0x3f1).rw(FUNC(vt8231_isa_device::superio_cfg_data_r), FUNC(vt8231_isa_device::superio_cfg_data_w));
}


//**************************************************************************
//  INTERFACE TO OTHER PCI FUNCTIONS
//**************************************************************************

void vt8231_isa_device::ide_irq_w(unsigned channel, int state)
{
	LOG("ide_irq_w: channel %d -> %d\n", channel, state);
	irq_to_pic(channel == 0 ? 14 : 15, state);
}


//**************************************************************************
//  LEGACY I/O
//**************************************************************************

uint8_t vt8231_isa_device::misc_functions_r()
{
	return m_misc_functions;
}

void vt8231_isa_device::misc_functions_w(uint8_t data)
{
	LOGMASKED(LOG_REG, "misc_functions_w: %02x\n", data);
	m_misc_functions = data & 0xf8;
}

uint8_t vt8231_isa_device::pic_slave_ack_r(offs_t offset)
{
	if (offset == 2) // IRQ2
		return m_pic[1]->acknowledge();

	return 0x00;
}

void vt8231_isa_device::irq_to_pic(int irq, int state)
{
	switch (irq)
	{
	case 14:
		m_pic[1]->ir6_w(state);
		break;
	case 15:
		m_pic[1]->ir7_w(state);
		break;
	default:
		LOG("Unhandled IRQ %d\n", irq);
	}
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

void vt8231_isa_device::com1_int_w(int state)
{
	if (state)
		LOG("com1_int_w: %d\n", state);
}
