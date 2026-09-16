// license:BSD-3-Clause
// copyright-holders:windyfairy
#ifndef MAME_KONAMI_K573KARA_H
#define MAME_KONAMI_K573KARA_H

#pragma once

#include "machine/ds2401.h"
#include "machine/ins8250.h"

class k573kara_device : public device_t
{
public:
	k573kara_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void amap(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	uint16_t uart_r(offs_t offset);
	void uart_w(offs_t offset, uint16_t data);

	uint16_t io_r();
	void lamp1_w(uint16_t data);
	void lamp2_w(uint16_t data);
	void lamp3_w(uint16_t data);

	uint16_t coin_box_r();
	void coin_box_w(uint16_t data);

	void video_selector_w(uint16_t data);

	uint16_t digital_id_r();
	void digital_id_w(uint16_t data);

	required_device<ds2401_device> digital_id;
	required_device<pc16552_device> m_duart_com;

	uint16_t m_coin_box_val;
};

DECLARE_DEVICE_TYPE(KONAMI_573_KARAOKE_IO_BOARD, k573kara_device)

#endif // MAME_KONAMI_K573KARA_H
