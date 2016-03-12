// license:BSD-3-Clause
// copyright-holders:Hans Ostermeyer,R. Belmont
/*
 * omti8621.h - SMS OMTI 8621 disk controller
 *
 *  Created on: August 30, 2010
 *      Author: Hans Ostermeyer
 *
 *  Converted to ISA device March 3, 2014 by R. Belmont
 *
 */

#pragma once

#ifndef ISA_OMTI8621_H_
#define ISA_OMTI8621_H_

#include "emu.h"
#include "isa.h"
#include "machine/pc_fdc.h"

#define OMTI_MAX_LUN 1
#define CDB_SIZE 10

/***************************************************************************
 FUNCTION PROTOTYPES
 ***************************************************************************/

class omti_disk_image_device;

/* ----- device interface ----- */

class omti8621_device : public device_t, public device_isa16_card_interface
{
public:
	omti8621_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, UINT32 clock);
	~omti8621_device() {}

	DECLARE_READ16_MEMBER(read);
	DECLARE_WRITE16_MEMBER(write);

	static void set_verbose(int on_off);

	required_device<pc_fdc_interface> m_fdc;
	required_ioport m_iobase;
	required_ioport m_biosopts;

	DECLARE_WRITE_LINE_MEMBER( fdc_irq_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_drq_w );
	DECLARE_FLOPPY_FORMATS( floppy_formats );

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const rom_entry *device_rom_region() const override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	virtual UINT8 dack_r(int line) override;
	virtual void dack_w(int line, UINT8 data) override;
	virtual void eop_w(int state) override;

	void set_interrupt(enum line_state line_state);

	omti_disk_image_device *our_disks[OMTI_MAX_LUN+1];

private:
	UINT16 jumper;

	UINT8 omti_state;

	UINT8 status_port;
	UINT8 config_port;
	UINT8 mask_port;

	// command descriptor block
	UINT8 command_buffer[CDB_SIZE];
	int command_length;
	int command_index;
	int command_status;

	// data buffer
	dynamic_buffer sector_buffer;
	UINT8 *data_buffer;
	int data_length;
	int data_index;

	// sense data
	UINT8 sense_data[4];

	// these are used only to satisfy dex
	UINT32 diskaddr_ecc_error;
	UINT32 diskaddr_format_bad_track;
	UINT8 alternate_track_buffer[4];
	UINT32 alternate_track_address[2];

	emu_timer *m_timer;

	bool m_installed;

	void clear_sense_data();
	void set_sense_data(UINT8 code, const UINT8 * cdb);
	void set_configuration_data(UINT8 lun);
	UINT8 get_lun(const UINT8 * cdb);
	UINT8 check_disk_address(const UINT8 *cdb);
	UINT32 get_disk_track(const UINT8 * cdb);
	UINT32 get_disk_address(const UINT8 * cdb);
	void set_data_transfer(UINT8 *data, UINT16 length);
	void read_sectors_from_disk(INT32 diskaddr, UINT8 count, UINT8 lun);
	void write_sectors_to_disk(INT32 diskaddr, UINT8 count, UINT8 lun);
	void copy_sectors(INT32 dst_addr, INT32 src_addr, UINT8 count, UINT8 lun);
	void format_track(const UINT8 * cdb);
	void set_esdi_defect_list(UINT8 lun, UINT8 head);

	template <typename Format, typename... Params> void logerror(Format &&fmt, Params &&... args) const;

	void log_command(const UINT8 cdb[], const UINT16 cdb_length);
	void log_data();
	void do_command(const UINT8 cdb[], const UINT16 cdb_length);
	UINT8 get_command_length(UINT8 command_byte);
	UINT16 get_data();
	void set_data(UINT16 data);
	void set_jumper(UINT16 disk_type);
	DECLARE_READ8_MEMBER(read8);
	DECLARE_WRITE8_MEMBER(write8);
};

/* ----- omti8621 for PC device interface ----- */

class omti8621_pc_device : public omti8621_device
{
public:
	omti8621_pc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type ISA16_OMTI8621;

/* ----- omti8621 for apollo device interface ----- */

class omti8621_apollo_device : public omti8621_device
{
public:
	omti8621_apollo_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// get sector diskaddr of logical unit lun into data_buffer
	UINT32 get_sector(INT32 diskaddr, UINT8 *data_buffer, UINT32 length, UINT8 lun);
protected:
	virtual const rom_entry *device_rom_region() const override;
};

extern const device_type ISA16_OMTI8621_APOLLO;

//###############################################################

#endif /* OMTI8621_H_ */
