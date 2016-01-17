// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Morrow Designs Disk Jockey 2D/B floppy controller board emulation

**********************************************************************/

#pragma once

#ifndef __S100_DJ2DB__
#define __S100_DJ2DB__

#include "emu.h"
#include "s100.h"
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
	s100_dj2db_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	// not really public
	DECLARE_WRITE_LINE_MEMBER( fr_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_intrq_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_drq_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_s100_card_interface overrides
	virtual UINT8 s100_smemr_r(address_space &space, offs_t offset) override;
	virtual void s100_mwrt_w(address_space &space, offs_t offset, UINT8 data) override;
	virtual UINT8 s100_sinp_r(address_space &space, offs_t offset) override;
	virtual void s100_sout_w(address_space &space, offs_t offset, UINT8 data) override;
	virtual void s100_phantom_w(int state) override;

private:
	// internal state
	required_device<mb8866_t> m_fdc;
	required_device<com8116_device> m_dbrg;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
	floppy_image_device *m_floppy;
	required_memory_region m_rom;
	optional_shared_ptr<UINT8> m_ram;
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
extern const device_type S100_DJ2DB;


#endif
