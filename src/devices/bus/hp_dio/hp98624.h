// license:BSD-3-Clause
// copyright-holders:Sven Schnelle

#ifndef MAME_BUS_HPDIO_98624_H
#define MAME_BUS_HPDIO_98624_H

#pragma once

#include "hp_dio.h"
#include "machine/tms9914.h"
#include "bus/ieee488/ieee488.h"

namespace bus::hp_dio {

class dio16_98624_device :
		public device_t,
		public device_dio16_card_interface
{
public:
	dio16_98624_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	dio16_98624_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	void dmack_w_in(int channel, uint8_t data) override;
	uint8_t dmack_r_in(int channel) override;

	uint16_t io_r(offs_t offset);
	void io_w(offs_t offset, uint16_t data);

	required_device<tms9914_device> m_tms9914;
	required_device<ieee488_device> m_ieee488;
	required_ioport m_switches;
private:
	void update_gpib_irq();
	void update_gpib_dma();
	void gpib_irq(int state);
	void gpib_dreq(int state);

	bool m_gpib_irq_line;
	bool m_gpib_dma_line;

	bool     m_installed_io;
	uint8_t  m_control;
};

} // namespace bus::hp_dio

DECLARE_DEVICE_TYPE_NS(HPDIO_98624, bus::hp_dio, dio16_98624_device)

#endif // MAME_BUS_HPDIO_98624_H
