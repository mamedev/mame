// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Western Digital WDXT-GEN ISA XT MFM Hard Disk Controller

**********************************************************************

    Emulated here is the variant supplied with Amstrad PC1512/1640,
    which has a custom BIOS and is coupled with a Tandom TM262 HDD.

    chdman -createblankhd tandon_tm262.chd 615 4 17 512

**********************************************************************/

#pragma once

#ifndef __ISA8_WDXT_GEN__
#define __ISA8_WDXT_GEN__


#include "emu.h"
#include "cpu/mcs48/mcs48.h"
#include "isa.h"
#include "machine/wd11c00_17.h"
#include "machine/wd2010.h"
#include "imagedev/harddriv.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wdxt_gen_device

class wdxt_gen_device : public device_t,
						public device_isa8_card_interface
{
public:
	// construction/destruction
	wdxt_gen_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual const rom_entry *device_rom_region() const;

	// not really public
	DECLARE_WRITE_LINE_MEMBER( irq5_w );
	DECLARE_WRITE_LINE_MEMBER( drq3_w );
	DECLARE_WRITE_LINE_MEMBER( mr_w );
	DECLARE_READ8_MEMBER( rd322_r );
	DECLARE_READ8_MEMBER( ram_r );
	DECLARE_WRITE8_MEMBER( ram_w );
	DECLARE_READ8_MEMBER( wd1015_t0_r );
	DECLARE_READ8_MEMBER( wd1015_t1_r );
	DECLARE_READ8_MEMBER( wd1015_p1_r );
	DECLARE_WRITE8_MEMBER( wd1015_p1_w );
	DECLARE_READ8_MEMBER( wd1015_p2_r );
	DECLARE_WRITE8_MEMBER( wd1015_p2_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_isa8_card_interface
	virtual UINT8 dack_r(int line);
	virtual void dack_w(int line, UINT8 data);

private:
	required_device<cpu_device> m_maincpu;
	required_device<wd11c00_17_device> m_host;
	required_device<wd2010_device> m_hdc;

	UINT8 m_ram[0x800];

	//UINT8 m_hdc_addr;
};


// device type definition
extern const device_type ISA8_WDXT_GEN;

#endif
