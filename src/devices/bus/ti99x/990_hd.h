// license:GPL-2.0+
// copyright-holders:Raphael Nabet
/*
    990_hd.h: include file for 990_hd.c
*/
#ifndef __990_HD_H_
#define __990_HD_H_

#include "imagedev/harddriv.h"

/* max disk units per controller: 4 is the protocol limit, but it may be
overriden if more than one controller is used */
#define MAX_DISK_UNIT 4

class ti990_hdc_device : public device_t
{
public:
	ti990_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &static_set_int_callback(device_t &device, _Object object) { return downcast<ti990_hdc_device &>(device).m_interrupt_callback.set_callback(object); }

	DECLARE_READ16_MEMBER(read);
	DECLARE_WRITE16_MEMBER(write);

	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( ti990_hd );
	DECLARE_DEVICE_IMAGE_UNLOAD_MEMBER( ti990_hd );
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	int get_id_from_device( device_t *device );

	inline int is_unit_loaded(int unit);
	int cur_disk_unit(void);
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
	enum format_t
	{
		format_mame,
		format_old
	};

	/* disk drive unit descriptor */
	struct hd_unit_t
	{
		device_image_interface *img;                        /* image descriptor */
		format_t format;
		hard_disk_file *hd_handle;      /* mame hard disk descriptor - only if format == format_mame */
		unsigned int wp : 1;                    /* TRUE if disk is write-protected */
		unsigned int unsafe : 1;                /* TRUE when a disk has just been connected */

		/* disk geometry */
		unsigned int cylinders, heads, sectors_per_track, bytes_per_sector;
	};

	UINT16 m_w[8];

	devcb_write_line m_interrupt_callback;

	hd_unit_t m_d[MAX_DISK_UNIT];
};

#define MCFG_TI990_HDC_INT_CALLBACK( _write ) \
	devcb = &ti990_hdc_device::static_set_int_callback( *device, DEVCB_##_write );

extern const device_type TI990_HDC;

#endif
