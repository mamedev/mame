// license:BSD-2-Clause
// copyright-holders:Lubomir Rintel

/***************************************************************************

    Altos 586 Hard Disk Controller emulation

***************************************************************************/

#ifndef MAME_ALTOS_ALTOS586_HDC_H
#define MAME_ALTOS_ALTOS586_HDC_H

#pragma once

#include "cpu/i8089/i8089.h"
#include "imagedev/harddriv.h"

class altos586_hdc_device : public device_t
{
public:
	template <typename T>
	altos586_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&bus_tag)
		: altos586_hdc_device(mconfig, tag, owner, clock)
	{
		m_bus.set_tag(std::forward<T>(bus_tag), AS_PROGRAM);
	}

	altos586_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t mem_r(offs_t offset, uint16_t mem_mask = ~0);
	void mem_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// Register on main bus.
	void attn_w(uint16_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	void altos586_hdc_io(address_map &map) ATTR_COLD;
	void altos586_hdc_mem(address_map &map) ATTR_COLD;

private:
	// Disk controller registers on IOP's I/O bus.
	uint16_t data_r(offs_t offset) ;
	void data_w(offs_t offset, uint16_t data);
	void head_select_w(offs_t offset, uint16_t data);
	uint16_t seek_status_r(offs_t offset) ;
	void cyl_w(offs_t offset, uint16_t data);
	uint16_t status_r(offs_t offset) ;
	void command_w(offs_t offset, uint16_t data);

	// Disk access routines and state.
	bool sector_exists(uint8_t index);
	uint32_t sector_lba(uint8_t index);
	void sector_read(uint8_t index);
	void sector_write(uint8_t index);

	// Image mount and unmount.
	template <uint8_t Index> std::error_condition hdd_load(device_image_interface &image);
	template <uint8_t Index> void hdd_unload(device_image_interface &image) { m_geom[Index] = nullptr; }

	required_address_space m_bus;
	required_device<i8089_device> m_iop;
	required_device_array<harddisk_image_device, 2> m_hdd;

	// Disk controller state.
	uint8_t m_status;
	uint8_t m_seek_status;

	uint16_t m_cyl_latch;
	uint16_t m_cyl[2];

	uint8_t m_sector[517];
	int m_secoffset;

	uint8_t m_drive;
	uint8_t m_head;
	const hard_disk_file::info *m_geom[2];
};

DECLARE_DEVICE_TYPE(ALTOS586_HDC, altos586_hdc_device)

#endif // MAME_ALTOS_ALTOS586_HDC_H
