// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Western Digital WD1015 Buffer Manager Control Processor

    The WD1015 is a masked 8049 sitting between the WD11C00-17 PC/XT
    host interface and the WD1010/WD2010 Winchester task file, parsing
    the host's 6-byte WD1002/XT command descriptor blocks and running
    the resulting disk sequencing and buffer management. Every board
    built around this chip set (WD1002A-WX1, WDXT-GEN, ...) is high
    level emulated here rather than by executing the 8049's own
    microcode.

    Each board carries its own mask revision of the part, and the command
    decode differs between them, so those revisions are separate device
    types here. Only WD1015-PL-54-02 (wd1015pl5402_device) is dumped; it is
    the disassembly this whole model was written against. wd1015_device
    itself is the generic behaviour, used for the revisions that are not.

    That base class implements the standard WD1002/XT command set (also
    implemented by the Linux "xd" driver's WD support, and documented
    in the IBM Fixed Disk Adapter Technical Reference).

**********************************************************************/

#ifndef MAME_MACHINE_WD1015_H
#define MAME_MACHINE_WD1015_H

#pragma once

#include "wd11c00_17.h"
#include "wd1010.h"
#include "imagedev/harddriv.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wd1015_device

class wd1015_device : public device_t
{
public:
	// construction/destruction
	wd1015_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_host(wd11c00_17_device &host) { m_host.set_tag(host); }
	void set_hdc(wd1010_device &hdc) { m_hdc.set_tag(hdc); }

	// WD11C00-17 glue: RAM window (in_ramcs/out_ramwr), software reset (out_mr), new command (out_busy)
	uint8_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint8_t data);
	void mr_w(int state);
	void busy_w(int state);

	// WD1010/WD2010 glue: sector data callbacks and command-complete interrupt
	uint8_t hdc_data_r();
	void hdc_data_w(uint8_t data);
	void hdc_intrq_w(int state);

protected:
	wd1015_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// XT/WD1002 command descriptor block opcodes (cdb[0])
	enum : uint8_t
	{
		CDB_TEST_READY  = 0x00,
		CDB_RECALIBRATE = 0x01,
		CDB_SENSE       = 0x03,
		CDB_FORMAT_DRV  = 0x04,
		CDB_VERIFY      = 0x05,
		CDB_FORMAT_TRK  = 0x06,
		CDB_FORMAT_BAD  = 0x07,
		CDB_READ        = 0x08,
		CDB_WRITE       = 0x0a,
		CDB_SEEK        = 0x0b,
		CDB_INIT_DRIVE  = 0x0c,
		CDB_ECC_BURST   = 0x0d,
		CDB_READ_BUFFER  = 0x0e,
		CDB_WRITE_BUFFER = 0x0f,
		CDB_RAM_DIAG    = 0xe0,
		CDB_DRIVE_DIAG  = 0xe3,
		CDB_CTRL_DIAG   = 0xe4,
		CDB_READ_LONG   = 0xe5,
		CDB_WRITE_LONG  = 0xe6
	};

	// WD1015 front-end protocol phase (what the next byte through the RAM window means)
	enum : uint8_t
	{
		PHASE_CDB,       // host is sending the 6-byte command descriptor block
		PHASE_INIT_DATA, // host is sending the INITIALIZE DRIVE CHARACTERISTICS parameter block (discarded)
		PHASE_DATA_OUT,  // host is sending sector data (WRITE)
		PHASE_WAIT_HDC,  // waiting for the WD1010 to finish the current sector operation
		PHASE_DATA_IN,   // host is reading result data (READ sector / REQUEST SENSE)
		PHASE_STATUS     // host is reading the final completion byte
	};

	virtual void begin_command();
	virtual void controller_diagnostic_extra() { }

	void setup_task_file(int drive, int head, int cylinder, int sector);
	void issue_hdc_command(uint8_t command);
	bool format_track(int drive, int head, int cylinder);
	void advance_or_finish();
	void keep_host_address_in_window(uint32_t length);
	void complete(uint8_t error);
	void start_data_in(bool sense, int length);
	void enter_status_phase();
	uint8_t sense_code_from_status(uint8_t status) const;
	int max_heads(int drive) const;
	int max_cylinders(int drive) const;
	bool multi_sector_out_of_range(int drive, int head, int cylinder, int sector, int count) const;

	required_device<wd11c00_17_device> m_host;
	required_device<wd1010_device> m_hdc;
	harddisk_image_device *m_hdd[2];

	uint8_t m_cdb[6];
	uint8_t m_cdb_ptr;

	uint8_t m_completion;    // final status byte presented to the host in PHASE_STATUS
	uint8_t m_sense[4];      // request-sense data (populated on error)
	uint8_t m_secbuf[256 * 512]; // shared sector data buffer -- sized for a full native multi-sector burst (see m_chunk_sectors)
	uint8_t m_initbuf[8];    // INITIALIZE DRIVE CHARACTERISTICS parameter block, while it is being collected

	uint32_t m_secptr;       // byte position within m_secbuf / the init-data / cdb collection
	int m_chunk_sectors;     // sectors covered by the WD1010 command currently in flight (1 for
	                         // everything but a native multi-sector READ/VERIFY burst)
	bool m_din_sense;        // PHASE_DATA_IN source: sense data (true) or m_secbuf (false)
	uint32_t m_din_len;

	uint8_t m_phase;
	uint8_t m_pending_command; // the CDB opcode currently being executed
	uint16_t m_remaining;      // sectors left to transfer for a multi-sector READ/WRITE/VERIFY (up to 256)
	uint8_t m_sector_number;
	bool m_error;

	// the working address of the transfer in progress, tracked independently of the CDB
	// so a multi-sector transfer's sense data (on failure) reports where it actually got
	// to rather than echoing back the CDB's original request (see complete())
	int m_cur_drive;
	int m_cur_head;
	int m_cur_cylinder;
	int m_cur_sector;

	// INITIALIZE DRIVE CHARACTERISTICS geometry, per drive; until it is programmed, bound
	// checks fall back to the mounted CHD's own geometry
	struct
	{
		bool programmed;
		uint16_t cylinders;
		uint8_t heads;
	} m_geometry[2];
};


// device type definition
DECLARE_DEVICE_TYPE(WD1015, wd1015_device)


// ======================> wd1015pl5402_device

// WD1015-PL-54-02, the mask revision fitted to the WDXT-GEN, and the only
// WD1015 whose microcode is dumped. Its command decode masks off CDB byte 0
// bit 4 before dispatch -- 0x1x aliases 0x0x and 0xFx aliases 0xE0-0xE6 --
// and its CONTROLLER INTERNAL DIAGNOSTIC leaves two extra bytes in the sector
// buffer on success. Both are read out of the disassembly; everything else is
// the standard command set already implemented by wd1015_device.
class wd1015pl5402_device : public wd1015_device
{
public:
	wd1015pl5402_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void begin_command() override;
	virtual void controller_diagnostic_extra() override;
};


DECLARE_DEVICE_TYPE(WD1015PL5402, wd1015pl5402_device)

#endif // MAME_MACHINE_WD1015_H
