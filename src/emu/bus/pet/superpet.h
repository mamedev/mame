// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore SuperPET emulation

**********************************************************************/

#pragma once

#ifndef __SUPERPET__
#define __SUPERPET__

#include "exp.h"
#include "machine/mos6551.h"
#include "machine/mos6702.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> superpet_device

class superpet_device : public device_t,
						public device_pet_expansion_card_interface
{
public:
	// construction/destruction
	superpet_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual ioport_constructor device_input_ports() const;

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_WRITE_LINE_MEMBER( acia_irq_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// device_pet_expansion_card_interface overrides
	virtual int pet_norom_r(address_space &space, offs_t offset, int sel);
	virtual UINT8 pet_bd_r(address_space &space, offs_t offset, UINT8 data, int &sel);
	virtual void pet_bd_w(address_space &space, offs_t offset, UINT8 data, int &sel);
	virtual int pet_diag_r();
	virtual void pet_irq_w(int state);

private:
	required_device<cpu_device> m_maincpu;
	required_device<mos6551_device> m_acia;
	required_device<mos6702_device> m_dongle;
	required_memory_region m_rom;
	optional_shared_ptr<UINT8> m_ram;
	required_ioport m_io_sw1;
	required_ioport m_io_sw2;

	inline void update_cpu();
	inline bool is_ram_writable();

	UINT8 m_system;
	UINT8 m_bank;
	UINT8 m_sw1;
	UINT8 m_sw2;
	int m_sel9_rom;
	int m_pet_irq;
	int m_acia_irq;
};


// device type definition
extern const device_type SUPERPET;


#endif
