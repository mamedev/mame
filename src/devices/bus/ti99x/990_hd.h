// license:GPL-2.0+
// copyright-holders:Raphael Nabet
/*
    990_hd.h: include file for 990_hd.c
*/
#ifndef MAME_BUS_TI99X_990_HD_H
#define MAME_BUS_TI99X_990_HD_H

#pragma once

#include "imagedev/harddriv.h"

class ti990_hdc_device : public device_t
{
public:
	ti990_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint16_t read(offs_t offset);
	void write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	std::error_condition load_hd(device_image_interface &image);
	void unload_hd(device_image_interface &image);

	template <typename T> void set_memory_space(T &&tag, int spacenum) { m_memory_space.set_tag(std::forward<T>(tag), spacenum); }
	auto int_cb() { return m_interrupt_callback.bind(); }

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	int get_id_from_device( device_t *device );

	inline int is_unit_loaded(int unit);
	int cur_disk_unit();
	void update_interrupt();
	int check_sector_address(int unit, unsigned int cylinder, unsigned int head, unsigned int sector);
	int sector_to_lba(int unit, unsigned int cylinder, unsigned int head, unsigned int sector, unsigned int *lba);
	int read_sector(int unit, unsigned int lba, void *buffer, unsigned int bytes_to_read);
	int write_sector(int unit, unsigned int lba, const void *buffer, unsigned int bytes_to_write);
	void store_registers();
	void write_format();
	void read_data();
	void write_data();
	void unformatted_read();
	void restore();
	void execute_command();

private:
	// max disk units per controller: 4 is the protocol limit, but it may be overridden if more than one controller is used
	static constexpr unsigned MAX_DISK_UNIT = 4;

	enum format_t
	{
		format_mame,
		format_old
	};

	/* disk drive unit descriptor */
	struct hd_unit_t
	{
		harddisk_image_device *img;                        /* image descriptor */
		format_t format;
		unsigned int wp : 1;                    /* true if disk is write-protected */
		unsigned int unsafe : 1;                /* true when a disk has just been connected */

		/* disk geometry */
		unsigned int cylinders, heads, sectors_per_track, bytes_per_sector;
	};

	uint16_t m_w[8];

	required_address_space m_memory_space;

	devcb_write_line m_interrupt_callback;

	hd_unit_t m_d[MAX_DISK_UNIT];
};

DECLARE_DEVICE_TYPE(TI990_HDC, ti990_hdc_device)

#endif // MAME_BUS_TI99X_990_HD_H
