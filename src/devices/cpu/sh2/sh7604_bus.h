// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

  SH7604 BUS Controller

***************************************************************************/

#pragma once

#ifndef __SH7604_BUSDEV_H__
#define __SH7604_BUSDEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SH7604_BUS_ADD(_tag,_freq) \
	MCFG_DEVICE_ADD(_tag, SH7604_BUS, _freq)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sh7604_bus_device

class sh7604_bus_device : public device_t,
    					  public device_memory_interface
{
public:
	// construction/destruction
	sh7604_bus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	
	// I/O operations
	DECLARE_WRITE32_MEMBER( write );
	DECLARE_READ32_MEMBER( read );
	DECLARE_READ16_MEMBER( bus_control_1_r );
	DECLARE_WRITE16_MEMBER( bus_control_1_w );
	DECLARE_READ16_MEMBER( bus_control_2_r );
	DECLARE_WRITE16_MEMBER( bus_control_2_w );
	DECLARE_READ16_MEMBER( wait_control_r );
	DECLARE_WRITE16_MEMBER( wait_control_w );
	DECLARE_READ16_MEMBER( memory_control_r );
	DECLARE_WRITE16_MEMBER( memory_control_w );
	DECLARE_READ16_MEMBER( refresh_timer_status_r );
	DECLARE_WRITE16_MEMBER( refresh_timer_control_w );
	DECLARE_READ16_MEMBER( refresh_timer_counter_r );
	DECLARE_WRITE16_MEMBER( refresh_timer_counter_w );
	DECLARE_READ16_MEMBER( refresh_timer_constant_r );
	DECLARE_WRITE16_MEMBER( refresh_timer_constant_w );
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

protected:
	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	bool m_is_slave;
	const address_space_config      m_space_config;
	
	UINT16 m_bcr1;
	UINT16 m_bcr2;
	UINT16 m_wcr;
	UINT16 m_mcr;
 	UINT16 m_rtcsr;
	UINT16 m_rtcor;
};


// device type definition
extern const device_type SH7604_BUS;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
