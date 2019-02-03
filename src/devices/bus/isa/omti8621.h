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

#ifndef MAME_BUS_ISA_OMTI8621_H
#define MAME_BUS_ISA_OMTI8621_H

#pragma once

#include "isa.h"
#include "machine/pc_fdc.h"
#include "imagedev/floppy.h"

/***************************************************************************
 FUNCTION PROTOTYPES
 ***************************************************************************/

class omti_disk_image_device;

/* ----- device interface ----- */

class omti8621_device : public device_t, public device_isa16_card_interface
{
public:
	DECLARE_READ16_MEMBER(read);
	DECLARE_WRITE16_MEMBER(write);

	static void set_verbose(int on_off);

protected:
	static constexpr unsigned OMTI_MAX_LUN = 1;
	static constexpr unsigned CDB_SIZE = 10;

	omti8621_device(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner,
			uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	virtual uint8_t dack_r(int line) override;
	virtual void dack_w(int line, uint8_t data) override;
	virtual void dack_line_w(int line, int state) override;
	virtual void eop_w(int state) override;

	void set_interrupt(enum line_state line_state);

	required_device<pc_fdc_interface> m_fdc;
	required_ioport m_iobase;
	required_ioport m_biosopts;

	omti_disk_image_device *our_disks[OMTI_MAX_LUN+1];

	std::string cpu_context() const;

private:
	DECLARE_WRITE_LINE_MEMBER( fdc_irq_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_drq_w );
	DECLARE_FLOPPY_FORMATS( floppy_formats );

	uint16_t jumper;

	uint8_t omti_state;

	uint8_t status_port;
	uint8_t config_port;
	uint8_t mask_port;

	// command descriptor block
	uint8_t command_buffer[CDB_SIZE];
	int command_length;
	int command_index;
	int command_status;

	// data buffer
	std::vector<uint8_t> sector_buffer;
	uint8_t *data_buffer;
	int data_length;
	int data_index;

	// sense data
	uint8_t sense_data[4];

	// these are used only to satisfy dex
	uint32_t diskaddr_ecc_error;
	uint32_t diskaddr_format_bad_track;
	uint8_t alternate_track_buffer[4];
	uint32_t alternate_track_address[2];

	emu_timer *m_timer;

	bool m_installed;

	void clear_sense_data();
	void set_sense_data(uint8_t code, const uint8_t * cdb);
	void set_configuration_data(uint8_t lun);
	uint8_t get_lun(const uint8_t * cdb);
	uint8_t check_disk_address(const uint8_t *cdb);
	uint32_t get_disk_track(const uint8_t * cdb);
	uint32_t get_disk_address(const uint8_t * cdb);
	void set_data_transfer(uint8_t *data, uint16_t length);
	void read_sectors_from_disk(int32_t diskaddr, uint8_t count, uint8_t lun);
	void write_sectors_to_disk(int32_t diskaddr, uint8_t count, uint8_t lun);
	void copy_sectors(int32_t dst_addr, int32_t src_addr, uint8_t count, uint8_t lun);
	void format_track(const uint8_t * cdb);
	void set_esdi_defect_list(uint8_t lun, uint8_t head);

	template <typename Format, typename... Params> void logerror(Format &&fmt, Params &&... args) const;

	void log_command(const uint8_t cdb[], const uint16_t cdb_length);
	void log_data();
	void do_command(const uint8_t cdb[], const uint16_t cdb_length);
	uint8_t get_command_length(uint8_t command_byte);
	uint16_t get_data();
	void set_data(uint16_t data);
	void set_jumper(uint16_t disk_type);
	DECLARE_READ8_MEMBER(read8);
	DECLARE_WRITE8_MEMBER(write8);
};

/* ----- omti8621 for PC device interface ----- */

class omti8621_pc_device : public omti8621_device
{
public:
	omti8621_pc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(ISA16_OMTI8621, omti8621_pc_device)

/* ----- omti8621 for apollo device interface ----- */

class omti8621_apollo_device : public omti8621_device
{
public:
	omti8621_apollo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// get sector diskaddr of logical unit lun into data_buffer
	uint32_t get_sector(int32_t diskaddr, uint8_t *data_buffer, uint32_t length, uint8_t lun);
protected:
	virtual const tiny_rom_entry *device_rom_region() const override;
};

DECLARE_DEVICE_TYPE(ISA16_OMTI8621_APOLLO, omti8621_apollo_device)

//###############################################################

#endif // MAME_BUS_ISA_OMTI8621_H
