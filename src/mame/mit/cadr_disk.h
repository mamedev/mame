// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_MIT_CADR_DISK_H
#define MAME_MIT_CADR_DISK_H

#pragma once

#include "imagedev/harddriv.h"


DECLARE_DEVICE_TYPE(CADR_DISK, cadr_disk_device)


class cadr_disk_device : public device_t
{
public:
	cadr_disk_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	template <typename T> void set_data_space(T &&tag, int spacenum) { m_data_space.set_tag(std::forward<T>(tag), spacenum); }
	auto irq_callback() { return m_irq_cb.bind(); }

	u32 read(offs_t offset);
	void write(offs_t offset, u32 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(disk_done_callback);

private:
	required_address_space m_data_space;
	required_device<harddisk_image_device> m_harddisk;
	devcb_write_line m_irq_cb;
	u32 m_command;
	u32 m_status;
	u32 m_clp;
	u32 m_unit;
	u32 m_cyl;
	u32 m_head;
	u32 m_block;
	u32 m_last_memory_address;
	emu_timer *m_disk_timer;

	u32 status_r();
	u32 command_list_r();
	u32 memory_address_r();
	u32 disk_address_r();
	u32 error_correction_r();
	void command_w(u32 data);
	void command_list_w(u32 data);
	void disk_address_w(u32 data);
	void start_w(u32 data);
};

#endif // MAME_MIT_CADR_DISK_H
