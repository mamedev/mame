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


msx_slot_rs232_base_device::msx_slot_rs232_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
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

void msx_slot_rs232_base_device::irq_mask_w(offs_t offset, u8 data)
{
	// According to MSX datapack:
	// 7654---- unused
	// ----3--- timer interrupt from i8352 channel 2
	// -----2-- sync character detect / break detect
	// ------1- transmit data ready
	// -------0 receive data ready
	// but most rs232 interface only seem to support receive data ready irq
	m_irq_mask = data;
}

void msx_slot_rs232_base_device::out2_w(int state)
{
	m_out2 = state;
	update_irq_state();
}

void msx_slot_rs232_base_device::cts_w(int state)
{
	m_cts = state;
	m_i8251->write_cts(state);
}

void msx_slot_rs232_base_device::dcd_w(int state)
{
	m_dcd = state;
	update_irq_state();
}

void msx_slot_rs232_base_device::ri_w(int state)
{
	m_ri = state;
}

void msx_slot_rs232_base_device::rxrdy_w(int state)
{
	m_rxrdy = state;
	update_irq_state();
}

void msx_slot_rs232_base_device::txrdy_w(int state)
{
	m_txrdy = state;
	update_irq_state();
}



msx_slot_rs232_device::msx_slot_rs232_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_rs232_base_device(mconfig, MSX_SLOT_RS232, tag, owner, clock)
{
}

void msx_slot_rs232_device::device_start()
{
	msx_slot_rs232_base_device::device_start();

	// Install IO read/write handlers
	io_space().install_readwrite_handler(0x80, 0x81, emu::rw_delegate(*m_i8251, FUNC(i8251_device::read)), emu::rw_delegate(*m_i8251, FUNC(i8251_device::write)));
	io_space().install_readwrite_handler(0x82, 0x82, emu::rw_delegate(*this, FUNC(msx_slot_rs232_device::status_r)), emu::rw_delegate(*this, FUNC(msx_slot_rs232_device::irq_mask_w)));
	io_space().install_readwrite_handler(0x84, 0x87, emu::rw_delegate(*m_i8253, FUNC(pit8253_device::read)), emu::rw_delegate(*m_i8253, FUNC(pit8253_device::write)));
}

u8 msx_slot_rs232_device::status_r(offs_t offset)
{
	// 7------- CTS
	// -6------ 8253 channel 2 output
	// --5432-- unused
	// ------1- ring indicator
	// -------0 carrier detect

	u8 result = 0x00;

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



msx_slot_rs232_mitsubishi_device::msx_slot_rs232_mitsubishi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
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
	io_space().install_readwrite_handler(0x80, 0x81, emu::rw_delegate(*m_i8251, FUNC(i8251_device::read)), emu::rw_delegate(*m_i8251, FUNC(i8251_device::write)));
	io_space().install_readwrite_handler(0x82, 0x82, emu::rw_delegate(*this, FUNC(msx_slot_rs232_mitsubishi_device::status_r)), emu::rw_delegate(*this, FUNC(msx_slot_rs232_mitsubishi_device::irq_mask_w)));
	io_space().install_readwrite_handler(0x84, 0x87, emu::rw_delegate(*m_i8253, FUNC(pit8253_device::read)), emu::rw_delegate(*m_i8253, FUNC(pit8253_device::write)));
}

u8 msx_slot_rs232_mitsubishi_device::status_r(offs_t offset)
{
	// 7------- CTS
	// -6------ 8253 channel 2 output
	// --54321- unused
	// -------0 carrier detect

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



msx_slot_rs232_sony_device::msx_slot_rs232_sony_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
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

	m_ram = std::make_unique<u8[]>(RAM_SIZE);
	save_pointer(NAME(m_ram), RAM_SIZE);

	// TODO unmap rom from page 0
	page(0)->install_ram(0x2000, 0x27ff, &m_ram[0]);
	page(1)->install_rom(0x4000, 0x5fff, m_rom_region->base() + m_region_offset);
	page(1)->install_ram(0x6000, 0x67ff, &m_ram[0]);
	page(2)->install_ram(0xa000, 0xa7ff, &m_ram[0]);
	page(2)->install_read_handler(0xbff8, 0xbff9, emu::rw_delegate(*m_i8251, FUNC(i8251_device::read)));
	page(2)->install_write_handler(0xbff8, 0xbff9, emu::rw_delegate(*m_i8251, FUNC(i8251_device::write)));
	page(2)->install_read_handler(0xbffa, 0xbffa, emu::rw_delegate(*this, FUNC(msx_slot_rs232_sony_device::status_r)));
	page(2)->install_write_handler(0xbffa, 0xbffa, emu::rw_delegate(*this, FUNC(msx_slot_rs232_sony_device::irq_mask_w)));
	page(2)->install_read_handler(0xbffc, 0xbfff, emu::rw_delegate(*m_i8253, FUNC(pit8253_device::read)));
	page(2)->install_write_handler(0xbffc, 0xbfff, emu::rw_delegate(*m_i8253, FUNC(pit8253_device::write)));
	page(3)->install_ram(0xe000, 0xe7ff, &m_ram[0]);
}

u8 msx_slot_rs232_sony_device::status_r(offs_t offset)
{
	// 7------- CTS
	// -6------ 8253 channel 2 output
	// --5----- unused
	// ---4---- ??
	// ----32-- unused
	// ------1- ring indicator
	// -------0 carrier detect

	u8 result = 0x00;

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



msx_slot_rs232_svi738_device::msx_slot_rs232_svi738_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_rs232_base_device(mconfig, MSX_SLOT_RS232_SVI738, tag, owner, clock)
{
}

void msx_slot_rs232_svi738_device::device_start()
{
	msx_slot_rs232_base_device::device_start();

	// Install IO read/write handlers
	io_space().install_readwrite_handler(0x80, 0x81, emu::rw_delegate(*m_i8251, FUNC(i8251_device::read)), emu::rw_delegate(*m_i8251, FUNC(i8251_device::write)));
	io_space().install_read_handler(0x82, 0x82, emu::rw_delegate(*this, FUNC(msx_slot_rs232_svi738_device::status_r)));
	io_space().install_readwrite_handler(0x84, 0x87, emu::rw_delegate(*m_i8253, FUNC(pit8253_device::read)), emu::rw_delegate(*m_i8253, FUNC(pit8253_device::write)));
}

u8 msx_slot_rs232_svi738_device::status_r(offs_t offset)
{
	// 7------- CTS
	// -6------ 8253 channel 2 output
	// --54321- unused
	// -------0 carrier detect

	u8 result = 0x00;

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



msx_slot_rs232_toshiba_device::msx_slot_rs232_toshiba_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
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
	io_space().install_readwrite_handler(0x80, 0x81, emu::rw_delegate(*m_i8251, FUNC(i8251_device::read)), emu::rw_delegate(*m_i8251, FUNC(i8251_device::write)));
	io_space().install_readwrite_handler(0x82, 0x82, emu::rw_delegate(*this, FUNC(msx_slot_rs232_toshiba_device::status_r)), emu::rw_delegate(*this, FUNC(msx_slot_rs232_toshiba_device::irq_mask_w)));
	io_space().install_readwrite_handler(0x84, 0x87, emu::rw_delegate(*m_i8253, FUNC(pit8253_device::read)), emu::rw_delegate(*m_i8253, FUNC(pit8253_device::write)));
}

u8 msx_slot_rs232_toshiba_device::status_r(offs_t offset)
{
	// 7------- CTS
	// -6------ 8253 channel 2 output
	// --54---- unused
	// ----3--- switch detection
	// -----2-- unused
	// ------1- ring indicator
	// -------0 carrier detect

	u8 result = 0x00;

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



msx_slot_rs232_toshiba_hx3x_device::msx_slot_rs232_toshiba_hx3x_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: msx_slot_rs232_base_device(mconfig, MSX_SLOT_RS232_TOSHIBA_HX3X, tag, owner, clock)
	, m_switch_port(*this, "SWITCH")
	, m_copy_port(*this, "COPY")
	, m_nvram(*this, "nvram")
	, m_rombank(*this, "rombank")
	, m_view(*this, "view")
{
}

static INPUT_PORTS_START(msx_hx3x_ports)
	PORT_INCLUDE(msx_rs232_enable_switch)

	PORT_START("COPY")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_OTHER) PORT_NAME("Copy")
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
	io_space().install_readwrite_handler(0x80, 0x81, emu::rw_delegate(*m_i8251, FUNC(i8251_device::read)), emu::rw_delegate(*m_i8251, FUNC(i8251_device::write)));
	io_space().install_readwrite_handler(0x82, 0x82, emu::rw_delegate(*this, FUNC(msx_slot_rs232_toshiba_hx3x_device::status_r)), emu::rw_delegate(*this, FUNC(msx_slot_rs232_toshiba_hx3x_device::irq_mask_w)));
	io_space().install_readwrite_handler(0x84, 0x87, emu::rw_delegate(*m_i8253, FUNC(pit8253_device::read)), emu::rw_delegate(*m_i8253, FUNC(pit8253_device::write)));

	m_sram = std::make_unique<u8[]>(SRAM_SIZE);
	m_nvram->set_base(&m_sram[0], SRAM_SIZE);

	save_item(NAME(m_bank_reg));
	save_pointer(NAME(m_sram), SRAM_SIZE);

	m_rombank->configure_entries(0, 4, m_rom_region->base() + m_region_offset + 0x4000, 0x4000);

	page(1)->install_write_handler(0x7fff, 0x7fff, emu::rw_delegate(*this, FUNC(msx_slot_rs232_toshiba_hx3x_device::bank_w)));
	page(1)->install_read_handler(0x7fff, 0x7fff, emu::rw_delegate(*this, FUNC(msx_slot_rs232_toshiba_hx3x_device::bank_r)));
	page(2)->install_view(0x8000, 0xbfff, m_view);
	m_view[0].install_read_bank(0x8000, 0xbfff, m_rombank);
	m_view[1].install_ram(0x8000, 0x87ff, 0x3800, &m_sram[0]);
}

void msx_slot_rs232_toshiba_hx3x_device::device_reset()
{
	msx_slot_rs232_base_device::device_reset();

	m_bank_reg = 0;
	m_view.select(0);
	m_rombank->set_entry(0);
}

u8 msx_slot_rs232_toshiba_hx3x_device::status_r(offs_t offset)
{
	// 7------- CTS
	// -6------ 8253 channel 2 output
	// --54---- unused
	// ----3--- switch detection
	// -----2-- unused
	// ------1- ring indicator
	// -------0 carrier detect

	u8 result = 0x00;

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

u8 msx_slot_rs232_toshiba_hx3x_device::bank_r()
{
	return (m_copy_port->read() & 0x80) | m_bank_reg;
}

void msx_slot_rs232_toshiba_hx3x_device::bank_w(u8 data)
{
	m_bank_reg = data & 0x7f;
	m_rombank->set_entry(data & 0x03);
	m_view.select(((data & 0x60) == 0x60) ? 1 : 0);
}
