/**********************************************************************

    Morrow Designs Disk Jockey 2D/B floppy controller board emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __S100_DJ2DB__
#define __S100_DJ2DB__


#include "emu.h"
#include "machine/s100.h"
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
	s100_dj2db_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	// not really public
	void fdc_intrq_w(bool state);
	void fdc_drq_w(bool state);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete() { m_shortname = "dj2db"; }

	// device_s100_card_interface overrides
	virtual UINT8 s100_smemr_r(address_space &space, offs_t offset);
	virtual void s100_mwrt_w(address_space &space, offs_t offset, UINT8 data);
	virtual UINT8 s100_sinp_r(address_space &space, offs_t offset);
	virtual void s100_sout_w(address_space &space, offs_t offset, UINT8 data);
	virtual void s100_phantom_w(int state);
	virtual bool s100_has_terminal() { return true; }
	virtual void s100_terminal_w(UINT8 data);

private:
	// internal state
	required_device<mb8866_t> m_fdc;
	required_device<com8116_device> m_dbrg;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;
	floppy_image_device *m_floppy;
	required_ioport m_j1a;
	optional_shared_ptr<UINT8> m_ram;

	// floppy state
	int m_drive;                // selected drive
	int m_head;                 // head loaded
	int m_int_enbl;             // interrupt enable

	// S-100 bus state
	const UINT8 *m_rom;
	int m_access_enbl;          // access enable
	int m_board_enbl;           // board enable
	int m_phantom;              // phantom
};


// device type definition
extern const device_type S100_DJ2DB;


#endif
