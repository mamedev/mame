// license:BSD-3-Clause
// copyright-holders:Carl, Brian Johnson
/***************************************************************************

    Epson QX-10 Keyboard Interface

***************************************************************************/

#include "emu.h"
#include "keyboard.h"

#include "matrix.h"

#include "cpu/mcs48/mcs48.h"

#include "qx10ascii.lh"
#include "qx10hasci.lh"


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(EPSON_QX_KEYBOARD_PORT, bus::epson_qx::keyboard::keyboard_port_device, "epson_qx_kbd", "Epson QX-10 Keyboard Port")
DEFINE_DEVICE_TYPE(QX10_KEYBOARD_HASCI, bus::epson_qx::keyboard::qx10_keyboard_hasci, "qx10_keyboard_hasci", "Epson QX-10 Keyboard (HASCI)")
DEFINE_DEVICE_TYPE(QX10_KEYBOARD_ASCII, bus::epson_qx::keyboard::qx10_keyboard_ascii, "qx10_keyboard_ascii", "Epson QX-10 Keyboard (ASCII)")


namespace bus::epson_qx::keyboard {

//**************************************************************************
//  SLOT DEVICE
//**************************************************************************

//-------------------------------------------------
//  keyboard_port_device - constructor
//-------------------------------------------------

keyboard_port_device::keyboard_port_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, EPSON_QX_KEYBOARD_PORT, tag, owner, clock)
	, device_single_card_slot_interface<keyboard_device>(mconfig, *this)
	, m_kbd(nullptr)
	, m_txd_handler(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void keyboard_port_device::device_start()
{
	// resolve callbacks
	m_txd_handler.resolve_safe();
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void keyboard_port_device::device_reset()
{
	// get connected keyboard
	m_kbd = get_card_device();
}

WRITE_LINE_MEMBER( keyboard_port_device::txd_w )
{
	m_txd_handler(state);
}

//-------------------------------------------------
//
//  host to module interface
//-------------------------------------------------

WRITE_LINE_MEMBER( keyboard_port_device::rxd_w )
{
	if (m_kbd)
		m_kbd->rxd_w(state);
}

WRITE_LINE_MEMBER( keyboard_port_device::clk_w )
{
	if (m_kbd)
		m_kbd->clk_w(state);
}

ROM_START(qx10kbd)
	ROM_REGION(0x0800, "mcu", 0)
	ROM_LOAD("mbl8049h.5a", 0x0000, 0x0800, CRC(8615e159) SHA1(26b7f447acfe2c605dbe0fc98e6c777f0fa8a94d))
ROM_END

//**************************************************************************
//  KEYBOARD DEVICE
//**************************************************************************

//-------------------------------------------------
//  keyboard_device - constructor
//-------------------------------------------------

keyboard_device::keyboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_interface(*this, "epson_qx_kbd")
	, m_rows(*this, "LINE%X", 0U)
	, m_mcu(*this, "mcu")
	, m_leds(*this, "led%u", 0U)
{
	m_host = dynamic_cast<keyboard_port_device *>(this->owner());
}

tiny_rom_entry const *keyboard_device::device_rom_region() const
{
	return ROM_NAME(qx10kbd);
}

void keyboard_device::device_add_mconfig(machine_config &config)
{
	auto &mcu(I8049(config, "mcu", 11_MHz_XTAL));
	mcu.p1_out_cb().set(FUNC(keyboard_device::mcu_p1_w));
	mcu.p2_out_cb().set(FUNC(keyboard_device::mcu_p2_w));
	mcu.bus_in_cb().set([this]() { return m_rows[m_row]->read(); });
	mcu.t1_in_cb().set([this]() { return m_rxd; });
	mcu.t0_in_cb().set([this]() { return m_clk_state; });

	config.set_default_layout(layout());
}

void keyboard_device::device_start()
{
	m_leds.resolve();

	m_clk_state = 0;

	save_item(NAME(m_rxd));
	save_item(NAME(m_row));
	save_item(NAME(m_clk_state));
}

void keyboard_device::mcu_p1_w(uint8_t data)
{
	m_row = data & 0xf;
	m_host->txd_w(BIT(data, 7));
}

void keyboard_device::mcu_p2_w(uint8_t data)
{
	for (int i = 0; i < 8; ++i) {
		m_leds[i] = BIT(data, i);
	}
}

WRITE_LINE_MEMBER(keyboard_device::rxd_w)
{
	m_rxd = state;
}

WRITE_LINE_MEMBER(keyboard_device::clk_w)
{
	m_clk_state = !state;
}

//**************************************************************************
//  HASCI KEYBOARD DEVICE
//**************************************************************************

qx10_keyboard_hasci::qx10_keyboard_hasci(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock)
	: keyboard_device(mconfig, QX10_KEYBOARD_HASCI, tag, owner, clock)
{
}

ioport_constructor qx10_keyboard_hasci::device_input_ports() const
{
	return INPUT_PORTS_NAME(qx10_keyboard_hasci);
}

const internal_layout &qx10_keyboard_hasci::layout() const
{
	return layout_qx10hasci;
}

//**************************************************************************
//  ASCII KEYBOARD DEVICE
//**************************************************************************

qx10_keyboard_ascii::qx10_keyboard_ascii(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock)
	: keyboard_device(mconfig, QX10_KEYBOARD_ASCII, tag, owner, clock)
{
}

ioport_constructor qx10_keyboard_ascii::device_input_ports() const
{
	return INPUT_PORTS_NAME(qx10_keyboard_ascii);
}

const internal_layout &qx10_keyboard_ascii::layout() const
{
	return layout_qx10ascii;
}

void keyboard_devices(device_slot_interface &device)
{
	device.option_add("qx10_hasci", QX10_KEYBOARD_HASCI);
	device.option_add("qx10_ascii", QX10_KEYBOARD_ASCII);
}

}
