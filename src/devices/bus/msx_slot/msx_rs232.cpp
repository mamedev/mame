// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

/*
Emulation of MSX internal RS-232 modules

TODO:
- What does the Mitsubishi RS232 switch do? Disable the entire RS-232 unit?
*/

#include "emu.h"
#include "msx_rs232.h"


DEFINE_DEVICE_TYPE(MSX_SLOT_RS232, msx_slot_rs232_device, "msx_slot_rs232", "MSX Internal RS-232C")
DEFINE_DEVICE_TYPE(MSX_SLOT_RS232_MITSUBISHI, msx_slot_rs232_mitsubishi_device, "msx_slot_rs232_mitsubishi", "Mitsubishi Internal RS-232C")
DEFINE_DEVICE_TYPE(MSX_SLOT_RS232_SONY, msx_slot_rs232_sony_device, "msx_slot_rs232_sony", "Sony Internal RS-232C")
DEFINE_DEVICE_TYPE(MSX_SLOT_RS232_SVI738, msx_slot_rs232_svi738_device, "msx_slot_rs232_svi738", "SVI-738 Internal RS-232C")
DEFINE_DEVICE_TYPE(MSX_SLOT_RS232_TOSHIBA, msx_slot_rs232_toshiba_device, "msx_slot_rs232_toshiba", "Toshiba Internal RS-232C")
DEFINE_DEVICE_TYPE(MSX_SLOT_RS232_TOSHIBA_HX3X, msx_slot_rs232_toshiba_hx3x_device, "msx_slot_rs232_toshiba_hx3x", "Toshiba HX-33/HX-34 Internal RS-232C and firmware")



static INPUT_PORTS_START(msx_rs232_enable_switch)
	PORT_START("SWITCH")
	PORT_CONFNAME(0x01, 0x01, "RS-232C is")
	PORT_CONFSETTING(0x00, "disabled")
	PORT_CONFSETTING(0x01, "enabled")
INPUT_PORTS_END


msx_slot_rs232_base_device::msx_slot_rs232_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_rom_device(mconfig, type, tag, owner, clock)
	, m_i8251(*this, "i8251")
	, m_i8253(*this, "i8253")
	, m_rs232(*this, "rs232")
	, m_irq_handler(*this)
	, m_irq_mask(0xff)
	, m_out2(false)
	, m_cts(false)
	, m_dcd(false)
	, m_ri(false)
	, m_rxrdy(false)
{
}

void msx_slot_rs232_base_device::device_add_mconfig(machine_config &config)
{
	// Config based on svi738 schematics, are they the same for other machines?

	I8251(config, m_i8251, 1.8432_MHz_XTAL);
	m_i8251->txd_handler().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_i8251->dtr_handler().set(m_rs232, FUNC(rs232_port_device::write_dtr));
	m_i8251->rts_handler().set(m_rs232, FUNC(rs232_port_device::write_rts));
	m_i8251->rxrdy_handler().set(*this, FUNC(msx_slot_rs232_base_device::rxrdy_w));
	m_i8251->txrdy_handler().set(*this, FUNC(msx_slot_rs232_base_device::txrdy_w));

	PIT8253(config, m_i8253);
	m_i8253->set_clk<0>(1.8432_MHz_XTAL);
	m_i8253->set_clk<1>(1.8432_MHz_XTAL);
	m_i8253->set_clk<2>(1.8432_MHz_XTAL);
	m_i8253->out_handler<0>().set(m_i8251, FUNC(i8251_device::write_rxc));
	m_i8253->out_handler<1>().set(m_i8251, FUNC(i8251_device::write_txc));
	m_i8253->out_handler<2>().set(*this, FUNC(msx_slot_rs232_base_device::out2_w));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_i8251, FUNC(i8251_device::write_rxd));
	m_rs232->dcd_handler().set(*this, FUNC(msx_slot_rs232_base_device::dcd_w));
	m_rs232->ri_handler().set(*this, FUNC(msx_slot_rs232_base_device::ri_w));
	m_rs232->cts_handler().set(*this, FUNC(msx_slot_rs232_base_device::cts_w));
	m_rs232->dsr_handler().set(m_i8251, FUNC(i8251_device::write_dsr));
}

void msx_slot_rs232_base_device::device_start()
{
	msx_slot_rom_device::device_start();

	m_irq_handler.resolve_safe();

	save_item(NAME(m_irq_mask));
	save_item(NAME(m_out2));
	save_item(NAME(m_cts));
	save_item(NAME(m_dcd));
	save_item(NAME(m_ri));
	save_item(NAME(m_rxrdy));
	save_item(NAME(m_txrdy));
}

void msx_slot_rs232_base_device::device_reset()
{
	m_irq_mask = 0xff;
	m_out2 = false;
	m_cts = false;
	m_dcd = false;
	m_ri =false;
	m_rxrdy = false;
}

void msx_slot_rs232_base_device::irq_mask_w(offs_t offset, uint8_t data)
{
	// According to MSX datapack:
	// bit3 - timer interrupt from i8253 channel 2
	// bit2 - sync character detect / break detect
	// bit1 - transmit data ready
	// bit0 - receive data ready
	m_irq_mask = data;
}

WRITE_LINE_MEMBER(msx_slot_rs232_base_device::out2_w)
{
	m_out2 = state;
	update_irq_state();
}

WRITE_LINE_MEMBER(msx_slot_rs232_base_device::cts_w)
{
	m_cts = state;
	m_i8251->write_cts(state);
}

WRITE_LINE_MEMBER(msx_slot_rs232_base_device::dcd_w)
{
	m_dcd = state;
	update_irq_state();
}

WRITE_LINE_MEMBER(msx_slot_rs232_base_device::ri_w)
{
	m_ri = state;
}

WRITE_LINE_MEMBER(msx_slot_rs232_base_device::rxrdy_w)
{
	m_rxrdy = state;
	update_irq_state();
}

WRITE_LINE_MEMBER(msx_slot_rs232_base_device::txrdy_w)
{
	m_txrdy = state;
	update_irq_state();
}



msx_slot_rs232_device::msx_slot_rs232_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_rs232_base_device(mconfig, MSX_SLOT_RS232, tag, owner, clock)
{
}

void msx_slot_rs232_device::device_start()
{
	msx_slot_rs232_base_device::device_start();

	// Install IO read/write handlers
	io_space().install_readwrite_handler(0x80, 0x81, read8sm_delegate(*m_i8251, FUNC(i8251_device::read)), write8sm_delegate(*m_i8251, FUNC(i8251_device::write)));
	io_space().install_readwrite_handler(0x82, 0x82, read8sm_delegate(*this, FUNC(msx_slot_rs232_device::status_r)), write8sm_delegate(*this, FUNC(msx_slot_rs232_device::irq_mask_w)));
	io_space().install_readwrite_handler(0x84, 0x87, read8sm_delegate(*m_i8253, FUNC(pit8253_device::read)), write8sm_delegate(*m_i8253, FUNC(pit8253_device::write)));
}

uint8_t msx_slot_rs232_device::status_r(offs_t offset)
{
	// bit7 - CTS
	// bit6 - 8253 channel 2 output
	// bit1 - Ring indicator
	// bit0 - Carrier detect

	uint8_t result = 0x00;

	if (m_cts)
		result |= 0x80;

	if (m_out2)
		result |= 0x40;

	if (m_ri)
		result |= 0x02;

	if (m_dcd)
		result |= 0x01;

	return result;
}

void msx_slot_rs232_device::update_irq_state()
{
	m_irq_handler(m_rxrdy);
}



msx_slot_rs232_mitsubishi_device::msx_slot_rs232_mitsubishi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_rs232_base_device(mconfig, MSX_SLOT_RS232_MITSUBISHI, tag, owner, clock)
	, m_switch_port(*this, "SWITCH")
{
}

ioport_constructor msx_slot_rs232_mitsubishi_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(msx_rs232_enable_switch);
}

void msx_slot_rs232_mitsubishi_device::device_start()
{
	msx_slot_rs232_base_device::device_start();

	// Install IO read/write handlers
	io_space().install_readwrite_handler(0x80, 0x81, read8sm_delegate(*m_i8251, FUNC(i8251_device::read)), write8sm_delegate(*m_i8251, FUNC(i8251_device::write)));
	io_space().install_readwrite_handler(0x82, 0x82, read8sm_delegate(*this, FUNC(msx_slot_rs232_mitsubishi_device::status_r)), write8sm_delegate(*this, FUNC(msx_slot_rs232_mitsubishi_device::irq_mask_w)));
	io_space().install_readwrite_handler(0x84, 0x87, read8sm_delegate(*m_i8253, FUNC(pit8253_device::read)), write8sm_delegate(*m_i8253, FUNC(pit8253_device::write)));
}

uint8_t msx_slot_rs232_mitsubishi_device::status_r(offs_t offset)
{
	// bit7 - CTS
	// bit6 - 8253 channel 2 output
	// bit0 - Carrier detect

	uint8_t result = 0x00;

	if (m_cts)
		result |= 0x80;

	if (m_out2)
		result |= 0x40;

	if (m_dcd)
		result |= 0x01;

	return result;
}

void msx_slot_rs232_mitsubishi_device::update_irq_state()
{
	if (!BIT(m_irq_mask, 0))
		m_irq_handler(m_rxrdy);
}



msx_slot_rs232_sony_device::msx_slot_rs232_sony_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_rs232_base_device(mconfig, MSX_SLOT_RS232_SONY, tag, owner, clock)
	, m_switch_port(*this, "SWITCH")
{
}

ioport_constructor msx_slot_rs232_sony_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(msx_rs232_enable_switch);
}

void msx_slot_rs232_sony_device::device_start()
{
	msx_slot_rs232_base_device::device_start();

	m_ram.resize(RAM_SIZE);
	save_item(NAME(m_ram));

}

uint8_t msx_slot_rs232_sony_device::status_r(offs_t offset)
{
	// bit7 - CTS
	// bit6 - 8253 channel 2 output
	// bit4 - ??
	// bit1 - Ring indicator
	// bit0 - Carrier detect

	uint8_t result = 0x00;

	if (m_cts)
		result |= 0x80;

	if (m_out2)
		result |= 0x40;

	if (m_ri)
		result |= 0x02;

	if (m_dcd)
		result |= 0x01;

	return result;
}

void msx_slot_rs232_sony_device::update_irq_state()
{
	if (!BIT(m_irq_mask, 0))
		m_irq_handler(m_rxrdy);
}

uint8_t msx_slot_rs232_sony_device::read(offs_t offset)
{
	if ((offset & 0x3800) == 0x2000)
	{
		return m_ram[offset & 0x07ff];
	}

	switch (offset)
	{
	case 0xbff8:
	case 0xbff9:
		return m_i8251->read(offset);
	case 0xbffa:
		return status_r(offset);
	case 0xbffc:
	case 0xbffd:
	case 0xbffe:
	case 0xbfff:
		return m_i8253->read(offset);
	}
	return msx_slot_rs232_base_device::read(offset);
}

void msx_slot_rs232_sony_device::write(offs_t offset, uint8_t data)
{
	if ((offset & 0x3800) == 0x2000)
	{
		m_ram[offset & 0x07ff] = data;
		return;
	}

	switch (offset)
	{
	case 0xbff8:
	case 0xbff9:
		m_i8251->write(offset, data);
		break;
	case 0xbffa:
		irq_mask_w(offset, data);
		break;
	case 0xbffc:
	case 0xbffd:
	case 0xbffe:
	case 0xbfff:
		m_i8253->write(offset, data);
		break;
	}
}



msx_slot_rs232_svi738_device::msx_slot_rs232_svi738_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_rs232_base_device(mconfig, MSX_SLOT_RS232_SVI738, tag, owner, clock)
{
}

void msx_slot_rs232_svi738_device::device_start()
{
	msx_slot_rs232_base_device::device_start();

	// Install IO read/write handlers
	io_space().install_readwrite_handler(0x80, 0x81, read8sm_delegate(*m_i8251, FUNC(i8251_device::read)), write8sm_delegate(*m_i8251, FUNC(i8251_device::write)));
	io_space().install_read_handler(0x82, 0x82, read8sm_delegate(*this, FUNC(msx_slot_rs232_svi738_device::status_r)));
	io_space().install_readwrite_handler(0x84, 0x87, read8sm_delegate(*m_i8253, FUNC(pit8253_device::read)), write8sm_delegate(*m_i8253, FUNC(pit8253_device::write)));
}

uint8_t msx_slot_rs232_svi738_device::status_r(offs_t offset)
{
	// bit7 - CTS
	// bit6 - 8253 channel 2 output
	// bit0 - Carrier detect

	uint8_t result = 0x00;

	if (m_cts)
		result |= 0x80;

	if (m_out2)
		result |= 0x40;

	if (m_dcd)
		result |= 0x01;

	return result;
}

void msx_slot_rs232_svi738_device::update_irq_state()
{
	m_irq_handler(m_rxrdy);
}



msx_slot_rs232_toshiba_device::msx_slot_rs232_toshiba_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_rs232_base_device(mconfig, MSX_SLOT_RS232_TOSHIBA, tag, owner, clock)
	, m_switch_port(*this, "SWITCH")
{
}

ioport_constructor msx_slot_rs232_toshiba_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(msx_rs232_enable_switch);
}

void msx_slot_rs232_toshiba_device::device_start()
{
	msx_slot_rs232_base_device::device_start();

	// Install IO read/write handlers
	io_space().install_readwrite_handler(0x80, 0x81, read8sm_delegate(*m_i8251, FUNC(i8251_device::read)), write8sm_delegate(*m_i8251, FUNC(i8251_device::write)));
	io_space().install_readwrite_handler(0x82, 0x82, read8sm_delegate(*this, FUNC(msx_slot_rs232_toshiba_device::status_r)), write8sm_delegate(*this, FUNC(msx_slot_rs232_toshiba_device::irq_mask_w)));
	io_space().install_readwrite_handler(0x84, 0x87, read8sm_delegate(*m_i8253, FUNC(pit8253_device::read)), write8sm_delegate(*m_i8253, FUNC(pit8253_device::write)));
}

uint8_t msx_slot_rs232_toshiba_device::status_r(offs_t offset)
{
	// bit7 - CTS
	// bit6 - 8253 channel 2 output
	// bit3 - Switch detect
	// bit1 - Ring indicator
	// bit0 - Carrier detect

	uint8_t result = 0x00;

	if (m_cts)
		result |= 0x80;

	if (m_out2)
		result |= 0x40;

	if (BIT(m_irq_mask, 0) && m_switch_port->read())
		result |= 0x08;

	if (m_ri)
		result |= 0x02;

	if (m_dcd)
		result |= 0x01;

	return result;
}

void msx_slot_rs232_toshiba_device::update_irq_state()
{
	if (!BIT(m_irq_mask, 0))
		m_irq_handler(m_rxrdy);
}



msx_slot_rs232_toshiba_hx3x_device::msx_slot_rs232_toshiba_hx3x_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: msx_slot_rs232_base_device(mconfig, MSX_SLOT_RS232_TOSHIBA, tag, owner, clock)
	, m_switch_port(*this, "SWITCH")
	, m_copy_port(*this, "COPY")
	, m_nvram(*this, "nvram")
{
}

static INPUT_PORTS_START(msx_hx3x_ports)
	PORT_INCLUDE(msx_rs232_enable_switch)

	PORT_START("COPY")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Copy")
INPUT_PORTS_END

ioport_constructor msx_slot_rs232_toshiba_hx3x_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(msx_hx3x_ports);
}

void msx_slot_rs232_toshiba_hx3x_device::device_add_mconfig(machine_config &config)
{
	msx_slot_rs232_base_device::device_add_mconfig(config);

	NVRAM(config, m_nvram, nvram_device::DEFAULT_ALL_0);
}


void msx_slot_rs232_toshiba_hx3x_device::device_start()
{
	msx_slot_rs232_base_device::device_start();

	// Sanity checks
	// 16KB for RS-232C rom and 4 16KB banks for firmware
	if (m_rom_region->bytes() < m_region_offset + 0x14000)
	{
		fatalerror("Memory region '%s' is too small for rom slot '%s'\n", m_rom_region.finder_tag(), tag());
	}

	// Install IO read/write handlers
	io_space().install_readwrite_handler(0x80, 0x81, read8sm_delegate(*m_i8251, FUNC(i8251_device::read)), write8sm_delegate(*m_i8251, FUNC(i8251_device::write)));
	io_space().install_readwrite_handler(0x82, 0x82, read8sm_delegate(*this, FUNC(msx_slot_rs232_toshiba_hx3x_device::status_r)), write8sm_delegate(*this, FUNC(msx_slot_rs232_toshiba_hx3x_device::irq_mask_w)));
	io_space().install_readwrite_handler(0x84, 0x87, read8sm_delegate(*m_i8253, FUNC(pit8253_device::read)), write8sm_delegate(*m_i8253, FUNC(pit8253_device::write)));

	m_sram.resize(SRAM_SIZE);
	m_nvram->set_base(m_sram.data(), SRAM_SIZE);

	save_item(NAME(m_bank_reg));

	m_bank_reg = 0;
	m_bank_base_8000 = m_rom + 0x4000;
}

void msx_slot_rs232_toshiba_hx3x_device::device_post_load()
{
	set_bank();
}

void msx_slot_rs232_toshiba_hx3x_device::set_bank()
{
	m_bank_base_8000 = m_rom + 0x4000 + (0x4000 * (m_bank_reg & 0x03));
}

uint8_t msx_slot_rs232_toshiba_hx3x_device::status_r(offs_t offset)
{
	// bit7 - CTS
	// bit6 - 8253 channel 2 output
	// bit3 - Switch detect
	// bit1 - Ring indicator
	// bit0 - Carrier detect

	uint8_t result = 0x00;

	if (m_cts)
		result |= 0x80;

	if (m_out2)
		result |= 0x40;

	if (BIT(m_irq_mask, 0) && m_switch_port->read())
		result |= 0x08;

	if (m_ri)
		result |= 0x02;

	if (m_dcd)
		result |= 0x01;

	return result;
}

void msx_slot_rs232_toshiba_hx3x_device::update_irq_state()
{
	if (!BIT(m_irq_mask, 0))
		m_irq_handler(m_rxrdy);
}

void msx_slot_rs232_toshiba_hx3x_device::write(offs_t offset, uint8_t data)
{
	if (offset == 0x7fff)
	{
		logerror("write %04x, %02x\n", offset, data);
		m_bank_reg = data & 0x7f;
		set_bank();
	}
	msx_slot_rs232_base_device::write(offset, data);
}

uint8_t msx_slot_rs232_toshiba_hx3x_device::read(offs_t offset)
{
	if (offset == 0x7fff)
	{
		return (m_copy_port->read() & 0x80) | m_bank_reg;
	}
	if (offset > 0x8000)
	{
		return m_bank_base_8000[offset & 0x3fff];
	}
	return msx_slot_rs232_base_device::read(offset);
}
