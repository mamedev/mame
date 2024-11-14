// license:BSD-3-Clause
// copyright-holders:Sven Schnelle

#ifndef MAME_BUS_HPDIO_HUMAN_INTERFACE_H
#define MAME_BUS_HPDIO_HUMAN_INTERFACE_H

#pragma once

#include "hp_dio.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/tms9914.h"
#include "machine/msm58321.h"
#include "sound/sn76496.h"
#include "bus/hp_hil/hp_hil.h"
#include "bus/hp_hil/hil_devices.h"
#include "bus/ieee488/ieee488.h"

namespace bus::hp_dio {

class human_interface_device :
	public device_t,
	public device_dio16_card_interface
{
public:
	human_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	human_interface_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:

	/* 8042 interface */
	void iocpu_port1_w(uint8_t data);
	void iocpu_port2_w(uint8_t data);
	uint8_t iocpu_port1_r();
	uint8_t iocpu_test0_r();

	/* GPIB */
	uint8_t gpib_r(offs_t offset);
	void gpib_w(offs_t offset, uint8_t data);
	void ieee488_dio_w(uint8_t data);

	void gpib_irq(int state);
	void gpib_dreq(int state);

	/* RTC */
	void rtc_d0_w(int state);
	void rtc_d1_w(int state);
	void rtc_d2_w(int state);
	void rtc_d3_w(int state);

	void reset_in(int state) override;

	void dmack_w_in(int channel, uint8_t data) override;
	uint8_t dmack_r_in(int channel) override;
	void update_gpib_irq();
	void update_gpib_dma();

	required_device<i8042ah_device> m_iocpu;
	required_device<hp_hil_mlc_device> m_mlc;
	required_device<sn76494_device> m_sound;
	required_device<tms9914_device> m_tms9914;
	required_device<msm58321_device> m_rtc;
	required_device<ieee488_device> m_ieee488;

	void iocpu_map(address_map &map) ATTR_COLD;

	static constexpr uint8_t HIL_CS = 0x01;
	static constexpr uint8_t HIL_WE = 0x02;
	static constexpr uint8_t HIL_OE = 0x04;
	static constexpr uint8_t LATCH_EN = 0x08;
	static constexpr uint8_t KBD_RESET = 0x40;
	static constexpr uint8_t SN76494_EN = 0x80;

	static constexpr uint8_t PPOLL_IE = 0x80;
	static constexpr uint8_t PPOLL_IR = 0x40;

	bool m_hil_read;
	bool m_kbd_nmi;

	bool m_gpib_irq_line;
	bool m_gpib_dma_line;

	bool m_old_latch_enable;
	bool m_gpib_dma_enable;

	uint8_t m_hil_data;
	uint8_t m_latch_data;
	uint8_t m_rtc_data;
	uint8_t m_ppoll_sc;
	uint8_t m_ppoll_mask;
};

} // namespace bus::hp_dio

// device type definition
DECLARE_DEVICE_TYPE_NS(HPDIO_HUMAN_INTERFACE, bus::hp_dio, human_interface_device)

#endif // MAME_BUS_HPDIO_HUMAN_INTERFACE_H
