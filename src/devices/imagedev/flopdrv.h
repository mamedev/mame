// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Miodrag Milanovic
/* flopdrv provides simple emulation of a disc drive */

#ifndef MAME_DEVICES_IMAGEDV_FLOPDRV_H
#define MAME_DEVICES_IMAGEDV_FLOPDRV_H

#pragma once

#include "formats/flopimg_legacy.h"

#define FLOPPY_TYPE_REGULAR 0
#define FLOPPY_TYPE_APPLE   1
#define FLOPPY_TYPE_SONY    2

#define FLOPPY_DRIVE_2_8_INCH   1
#define FLOPPY_DRIVE_3_INCH     2
#define FLOPPY_DRIVE_3_5_INCH   3
#define FLOPPY_DRIVE_5_25_INCH  4
#define FLOPPY_DRIVE_8_INCH     5

// Maximum supported density
#define FLOPPY_DRIVE_SD 1
#define FLOPPY_DRIVE_DD 2
#define FLOPPY_DRIVE_QD 3
#define FLOPPY_DRIVE_HD 4
#define FLOPPY_DRIVE_ED 5

#define FLOPPY_STANDARD_3_SSDD       { FLOPPY_DRIVE_3_INCH,    1, 42, FLOPPY_DRIVE_DD }
#define FLOPPY_STANDARD_3_DSDD       { FLOPPY_DRIVE_3_INCH,    2, 42, FLOPPY_DRIVE_DD }
#define FLOPPY_STANDARD_3_5_SSDD     { FLOPPY_DRIVE_3_5_INCH,  1, 83, FLOPPY_DRIVE_DD }
#define FLOPPY_STANDARD_3_5_DSDD     { FLOPPY_DRIVE_3_5_INCH,  2, 83, FLOPPY_DRIVE_DD }
#define FLOPPY_STANDARD_3_5_DSHD     { FLOPPY_DRIVE_3_5_INCH,  2, 83, FLOPPY_DRIVE_HD }
#define FLOPPY_STANDARD_3_5_DSED     { FLOPPY_DRIVE_3_5_INCH,  2, 83, FLOPPY_DRIVE_ED }
#define FLOPPY_STANDARD_5_25_SSSD_35 { FLOPPY_DRIVE_5_25_INCH, 1, 37, FLOPPY_DRIVE_SD }
#define FLOPPY_STANDARD_5_25_DSSD_35 { FLOPPY_DRIVE_5_25_INCH, 2, 37, FLOPPY_DRIVE_SD }
#define FLOPPY_STANDARD_5_25_SSSD    { FLOPPY_DRIVE_5_25_INCH, 1, 42, FLOPPY_DRIVE_SD }
#define FLOPPY_STANDARD_5_25_DSSD    { FLOPPY_DRIVE_5_25_INCH, 2, 42, FLOPPY_DRIVE_SD }
#define FLOPPY_STANDARD_5_25_SSDD_40 { FLOPPY_DRIVE_5_25_INCH, 1, 42, FLOPPY_DRIVE_DD }
#define FLOPPY_STANDARD_5_25_SSDD_80 { FLOPPY_DRIVE_5_25_INCH, 1, 83, FLOPPY_DRIVE_DD }
#define FLOPPY_STANDARD_5_25_DSDD_40 { FLOPPY_DRIVE_5_25_INCH, 2, 42, FLOPPY_DRIVE_DD }
#define FLOPPY_STANDARD_5_25_SSDD    { FLOPPY_DRIVE_5_25_INCH, 1, 83, FLOPPY_DRIVE_DD }
#define FLOPPY_STANDARD_5_25_DSDD    { FLOPPY_DRIVE_5_25_INCH, 2, 83, FLOPPY_DRIVE_DD }
#define FLOPPY_STANDARD_5_25_DSQD    { FLOPPY_DRIVE_5_25_INCH, 2, 83, FLOPPY_DRIVE_QD }
#define FLOPPY_STANDARD_5_25_DSHD    { FLOPPY_DRIVE_5_25_INCH, 2, 83, FLOPPY_DRIVE_HD }
#define FLOPPY_STANDARD_8_SSSD       { FLOPPY_DRIVE_8_INCH,    1, 77, FLOPPY_DRIVE_SD }
#define FLOPPY_STANDARD_8_DSSD       { FLOPPY_DRIVE_8_INCH,    2, 77, FLOPPY_DRIVE_SD }
#define FLOPPY_STANDARD_8_DSDD       { FLOPPY_DRIVE_8_INCH,    2, 77, FLOPPY_DRIVE_DD }

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

DECLARE_DEVICE_TYPE(LEGACY_FLOPPY, legacy_floppy_image_device)

struct floppy_type_t
{
	uint8_t media_size;
	uint8_t head_number;
	uint8_t max_track_number;
	uint8_t max_density;
};

// ======================> floppy_interface

struct floppy_interface
{
	floppy_type_t floppy_type;
	const struct FloppyFormat *formats;
	const char *interface;
};

struct chrn_id
{
	unsigned char C;
	unsigned char H;
	unsigned char R;
	unsigned char N;
	int data_id;            // id for read/write data command
	unsigned long flags;
};

/* set if drive is ready */
#define FLOPPY_DRIVE_READY                      0x0010
/* set if index has just occurred */
#define FLOPPY_DRIVE_INDEX                      0x0020

class legacy_floppy_image_device :  public device_t,
									public device_image_interface
{
public:
	// construction/destruction
	legacy_floppy_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, const floppy_interface *config)
		: legacy_floppy_image_device(mconfig, tag, owner, clock)
	{
		set_floppy_config(config);
	}

	legacy_floppy_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~legacy_floppy_image_device();

	void set_floppy_config(const floppy_interface *config) { m_config = config; }
	auto out_idx_cb() { return m_out_idx_func.bind(); }

	virtual std::pair<std::error_condition, std::string> call_load() override;
	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *format_options) override;
	virtual void call_unload() override;

	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override;
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *image_interface() const noexcept override;
	virtual const char *file_extensions() const noexcept override { return m_extension_list; }
	virtual const char *image_type_name() const noexcept override { return "floppydisk"; }
	virtual const char *image_brief_type_name() const noexcept override { return "flop"; }
	virtual const util::option_guide &create_option_guide() const override { return floppy_option_guide(); }

	floppy_image_legacy *flopimg_get_image();
	void floppy_drive_set_geometry(floppy_type_t type);
	void floppy_drive_set_flag_state(int flag, int state);
	void floppy_drive_set_ready_state(int state, int flag);
	int floppy_drive_get_flag_state(int flag);
	void floppy_drive_seek(signed int signed_tracks);
	int floppy_drive_get_next_id(int side, chrn_id *id);
	void floppy_drive_read_track_data_info_buffer(int side, void *ptr, int *length );
	void floppy_drive_write_track_data_info_buffer(int side, const void *ptr, int *length );
	void floppy_drive_format_sector(int side, int sector_index,int c,int h, int r, int n, int filler);
	void floppy_drive_read_sector_data(int side, int index1, void *ptr, int length);
	void floppy_drive_write_sector_data(int side, int index1, const void *ptr,int length, int ddam);
	void floppy_install_load_proc(void (*proc)(device_image_interface &image, bool is_created));
	void floppy_install_unload_proc(void (*proc)(device_image_interface &image));
	void floppy_drive_set_index_pulse_callback(void (*callback)(device_t *controller,device_t *image, int state));
	int floppy_drive_get_current_track();
	uint64_t floppy_drive_get_current_track_size(int head);
	void floppy_drive_set_rpm(float rpm);
	void floppy_drive_set_controller(device_t *controller);
	int floppy_get_drive_type();
	void floppy_set_type(int ftype);
	void floppy_ds_w(int state);
	void floppy_mon_w(int state);
	void floppy_drtn_w(int state);
	void floppy_wtd_w(int state);
	void floppy_stp_w(int state);
	void floppy_wtg_w(int state);
	int floppy_wpt_r();
	int floppy_tk00_r();
	int floppy_dskchg_r();
	int floppy_twosid_r();
	int floppy_index_r();
	int floppy_ready_r();


private:
	int flopimg_get_sectors_per_track(int side);
	void flopimg_get_id_callback(chrn_id *id, int id_index, int side);
	void log_readwrite(const char *name, int head, int track, int sector, const char *buf, int length);
	void floppy_drive_set_geometry_absolute(int tracks, int sides);
	TIMER_CALLBACK_MEMBER(floppy_drive_index_callback);
	void floppy_drive_init();
	void floppy_drive_index_func();
	std::error_condition internal_floppy_device_load(bool is_create, int create_format, util::option_resolution *create_args);
	TIMER_CALLBACK_MEMBER( set_wpt );

protected:
	legacy_floppy_image_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;

	// device_image_interface implementation
	virtual const software_list_loader &get_software_list_loader() const override;

	/* callbacks */
	devcb_write_line m_out_idx_func;

	/* state of input lines */
	int m_drtn; /* direction */
	int m_stp;  /* step */
	int m_wtg;  /* write gate */
	int m_mon;  /* motor on */

	/* state of output lines */
	int m_idx;  /* index pulse */
	int m_tk00; /* track 00 */
	int m_wpt;  /* write protect */
	int m_rdy;  /* ready */
	int m_dskchg;     /* disk changed */

	/* drive select logic */
	bool m_active;

	const floppy_interface  *m_config;

	/* flags */
	int m_flags;
	/* maximum track allowed */
	int m_max_track;
	/* num sides */
	int m_num_sides;
	/* current track - this may or may not relate to the present cylinder number
	stored by the fdc */
	int m_current_track;

	/* index pulse timer */
	emu_timer   *m_index_timer;
	/* index pulse callback */
	void    (*m_index_pulse_callback)(device_t *controller,device_t *image, int state);
	/* rotation per minute => gives index pulse frequency */
	float m_rpm;

	emu_timer *m_wpt_timer;
	int m_id_index;

	device_t *m_controller;

	floppy_image_legacy *m_floppy;
	int m_track;
	void (*m_load_proc)(device_image_interface &image, bool is_created);
	void (*m_unload_proc)(device_image_interface &image);
	int m_floppy_drive_type;

	char            m_extension_list[256];
};

#endif // MAME_DEVICES_IMAGEDV_FLOPDRV_H
