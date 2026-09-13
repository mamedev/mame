// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Western Digital WD1002A-WX1 Winchester Disk Controller emulation

    Chip set: WD11C00-17-02 (PC/XT host interface), WD1015A-02 (buffer
    manager control processor), WD1010A-05 (Winchester disk controller/
    sequencer), WD10C20B (self-adjusting MFM data separator).  Supports
    two ST-506/412 (MFM) drives.

    The WD1015A is a masked 8049; its firmware is undumped (the 8000h
    ROM already on this board, "600693-001 type 5", is the host BIOS
    extension, not the WD1015A's own microcode -- compare the genuine
    WD1015 dump used by wdxt_gen.cpp, which uses the same WD11C00-17
    host chip and a real i8049).  So the WD1015A's command-block
    decode and buffer management are high-level emulated here in front
    of the genuine wd11c00_17_device (host bus interface) and
    wd1010_device (task file / seek / read-write sequencer), the same
    approach used for the WD1002-HD0/-05 boards (wd1002_hd0.cpp).

    Host protocol: the BIOS/driver polls the status register (port
    321h) then issues a 6-byte Command Descriptor Block through the
    data register (port 320h), following the WD1002/XT command set
    (also implemented by the Linux "xd" driver's WD support, and
    documented in the IBM Fixed Disk Adapter Technical Reference).
    Byte layouts for the less common commands (FORMAT, INITIALIZE
    DRIVE CHARACTERISTICS) and the exact sense-byte format are
    best-effort reconstructions from that public documentation --
    there is no WD1015A ROM to verify them against.

**********************************************************************/

#ifndef MAME_BUS_ISA_WD1002A_WX1_H
#define MAME_BUS_ISA_WD1002A_WX1_H

#pragma once

#include "isa.h"
#include "machine/wd11c00_17.h"
#include "machine/wd1010.h"
#include "imagedev/harddriv.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> isa8_wd1002a_wx1_device

class isa8_wd1002a_wx1_device : public device_t,
								public device_isa8_card_interface
{
public:
	// construction/destruction
	isa8_wd1002a_wx1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_isa8_card_interface overrides
	virtual uint8_t dack_r(int line) override;
	virtual void dack_w(int line, uint8_t data) override;
	virtual void dack_line_w(int line, int state) override;

private:
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
		CDB_INIT_DRIVE  = 0x0c
	};

	// WD1015A front-end protocol phase (what the next byte through the RAM window means)
	enum : uint8_t
	{
		PHASE_CDB,       // host is sending the 6-byte command descriptor block
		PHASE_INIT_DATA, // host is sending the INITIALIZE DRIVE CHARACTERISTICS parameter block (discarded)
		PHASE_DATA_OUT,  // host is sending sector data (WRITE)
		PHASE_WAIT_HDC,  // waiting for the WD1010 to finish the current sector operation
		PHASE_DATA_IN,   // host is reading result data (READ sector / REQUEST SENSE)
		PHASE_STATUS     // host is reading the final completion byte
	};

	void irq5_w(int state);
	void drq3_w(int state);
	void mr_w(int state);
	void busy_w(int state);
	uint8_t rd322_r();
	uint8_t ram_r(offs_t offset);
	void ram_w(offs_t offset, uint8_t data);

	void hdc_intrq_w(int state);
	uint8_t hdc_data_r();
	void hdc_data_w(uint8_t data);

	void begin_command();
	void setup_task_file(int drive, int head, int cylinder, int sector);
	void issue_hdc_command(uint8_t command);
	void advance_or_finish();
	void complete(uint8_t error);
	void start_data_in(bool sense, int length);
	void enter_status_phase();

	required_device<wd11c00_17_device> m_host;
	required_device<wd1010_device> m_hdc;
	required_device_array<harddisk_image_device, 2> m_hdd;

	uint8_t m_cdb[6];
	uint8_t m_cdb_ptr;

	uint8_t m_completion;    // final status byte presented to the host in PHASE_STATUS
	uint8_t m_sense[4];      // request-sense data (populated on error)
	uint8_t m_secbuf[512];   // shared sector data buffer

	uint16_t m_secptr;       // byte position within m_secbuf / the init-data / cdb collection
	bool m_din_sense;        // PHASE_DATA_IN source: sense data (true) or m_secbuf (false)
	uint16_t m_din_len;

	uint8_t m_phase;
	uint8_t m_pending_command; // the CDB opcode currently being executed
	uint8_t m_remaining;       // sectors left to transfer for a multi-sector READ/WRITE/VERIFY
	uint8_t m_sector_number;
	bool m_error;
};


// device type definition
DECLARE_DEVICE_TYPE(ISA8_WD1002A_WX1, isa8_wd1002a_wx1_device)

#endif // MAME_BUS_ISA_WD1002A_WX1_H
