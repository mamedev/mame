// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    hp9871.cpp

    HP9871 daisy-wheel printer

*********************************************************************/

#include "emu.h"
#include "hp9871.h"

// device type definition
DEFINE_DEVICE_TYPE(HP9871, hp9871_device, "hp9871" , "HP9871 printer")

hp9871_device::hp9871_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HP9871, tag, owner, clock)
	, device_hp98032_gpio_interface(mconfig, *this)
	, m_printer(*this, "printer")
{
}

hp9871_device::~hp9871_device()
{
}

uint16_t hp9871_device::get_jumpers() const
{
	return hp98032_gpio_slot_device::JUMPER_3 |
		hp98032_gpio_slot_device::JUMPER_5;
}

uint16_t hp9871_device::input_r() const
{
	return 0;
}

uint8_t hp9871_device::ext_status_r() const
{
	// Bit 0: Buffer space available (1)
	// Bit 1: Ready (0)
	uint8_t res = 0;

	if (m_printer->is_ready()) {
		res |= 2;
	}

	return res;
}

void hp9871_device::output_w(uint16_t data)
{
	m_data = uint8_t(data);
}

void hp9871_device::ext_control_w(uint8_t data)
{
	// N/U
}

WRITE_LINE_MEMBER(hp9871_device::pctl_w)
{
	if (!state) {
		m_ibf = true;
		update_busy();
	} else {
		output(m_printer->is_ready());
	}
}

WRITE_LINE_MEMBER(hp9871_device::io_w)
{
	// N/U
}

WRITE_LINE_MEMBER(hp9871_device::preset_w)
{
	// N/U
}

void hp9871_device::device_add_mconfig(machine_config &config)
{
	PRINTER(config, m_printer, 0);
	m_printer->online_callback().set(FUNC(hp9871_device::printer_online));
}

void hp9871_device::device_start()
{
}

void hp9871_device::device_reset()
{
	m_ibf = false;
	update_busy();
	psts_w(1);
}

WRITE_LINE_MEMBER(hp9871_device::printer_online)
{
	output(state);
}

void hp9871_device::update_busy()
{
	pflg_w(!m_ibf);
}

void hp9871_device::output(bool printer_ready)
{
	if (m_ibf && printer_ready) {
		m_printer->output(m_data);
		m_ibf = false;
		update_busy();
	}
}
