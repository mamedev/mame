// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Morrow Designs Disk Jockey 2D/B floppy controller board emulation

**********************************************************************/

#ifndef MAME_BUS_S100_DJ2DB_H
#define MAME_BUS_S100_DJ2DB_H

#pragma once

#include "s100.h"
#include "imagedev/floppy.h"
#include "machine/ay31015.h"
#include "machine/com8116.h"
#include "machine/wd_fdc.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> s100_dj2db_device

class s100_dj2db_device : public device_t,
							public device_s100_card_interface
{
public:
	// construction/destruction
	s100_dj2db_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_s100_card_interface overrides
	virtual uint8_t s100_smemr_r(offs_t offset) override;
	virtual void s100_mwrt_w(offs_t offset, uint8_t data) override;
	virtual uint8_t s100_sinp_r(offs_t offset) override;
	virtual void s100_sout_w(offs_t offset, uint8_t data) override;
	virtual void s100_phantom_w(int state) override;

private:
	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);

	// internal state
	required_device<mb8866_device> m_fdc;
	required_device<com8116_device> m_dbrg;
	required_device<ay31015_device> m_uart;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
	floppy_image_device *m_floppy;
	required_memory_region m_rom;
	memory_share_creator<uint8_t> m_ram;
	required_ioport m_j1a;
	required_ioport m_j3a;
	required_ioport m_j4;
	required_ioport m_sw1;

	// floppy state
	int m_drive;                // selected drive
	int m_head;                 // head loaded
	int m_int_enbl;             // interrupt enable

	// S-100 bus state
	int m_access_enbl;          // access enable
	int m_board_enbl;           // board enable
	int m_phantom;              // phantom
};


// device type definition
DECLARE_DEVICE_TYPE(S100_DJ2DB, s100_dj2db_device)

#endif // MAME_BUS_S100_DJ2DB_H
