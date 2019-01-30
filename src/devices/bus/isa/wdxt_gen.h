// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Western Digital WDXT-GEN ISA XT MFM Hard Disk Controller

**********************************************************************

    Emulated here is the variant supplied with Amstrad PC1512/1640,
    which has a custom BIOS and is coupled with a Tandom TM262 HDD.

    chdman -createblankhd tandon_tm262.chd 615 4 17 512

**********************************************************************/

#ifndef MAME_BUS_ISA8_WDXT_GEN_H
#define MAME_BUS_ISA8_WDXT_GEN_H

#pragma once


#include "isa.h"
#include "cpu/mcs48/mcs48.h"
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
	wdxt_gen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// device_isa8_card_interface
	virtual uint8_t dack_r(int line) override;
	virtual void dack_w(int line, uint8_t data) override;
	virtual void dack_line_w(int line, int state) override;

private:
	DECLARE_WRITE_LINE_MEMBER( irq5_w );
	DECLARE_WRITE_LINE_MEMBER( drq3_w );
	DECLARE_WRITE_LINE_MEMBER( mr_w );
	DECLARE_READ8_MEMBER( rd322_r );
	DECLARE_READ8_MEMBER( ram_r );
	DECLARE_WRITE8_MEMBER( ram_w );
	DECLARE_READ_LINE_MEMBER( wd1015_t1_r );
	DECLARE_READ8_MEMBER( wd1015_p1_r );
	DECLARE_WRITE8_MEMBER( wd1015_p1_w );
	DECLARE_READ8_MEMBER( wd1015_p2_r );
	DECLARE_WRITE8_MEMBER( wd1015_p2_w );

	void wd1015_io(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<wd11c00_17_device> m_host;
	required_device<wd2010_device> m_hdc;

	uint8_t m_ram[0x800];

	//uint8_t m_hdc_addr;
};


// device type definition
DECLARE_DEVICE_TYPE(ISA8_WDXT_GEN, wdxt_gen_device)

#endif // MAME_BUS_ISA8_WDXT_GEN_H
