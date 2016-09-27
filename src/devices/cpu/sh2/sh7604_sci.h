// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

  SH7604 SCI Controller

***************************************************************************/

#pragma once

#ifndef __SH7604_SCIDEV_H__
#define __SH7604_SCIDEV_H__

enum {
	STATUS_MPBT = 1 << 0,
	STATUS_MPB =  1 << 1,
	STATUS_TEND = 1 << 2,
	STATUS_PER =  1 << 3,
	STATUS_FER =  1 << 4,
	STATUS_ORER = 1 << 5,
	STATUS_RDRF = 1 << 6,
	STATUS_TDRE = 1 << 7
};



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SH7604_SCI_ADD(_tag,_freq) \
	MCFG_DEVICE_ADD(_tag, SH7604_SCI, _freq)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> sh7604_sci_device

class sh7604_sci_device : public device_t,
    					  public device_memory_interface
{
public:
	// construction/destruction
	sh7604_sci_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// I/O operations
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );
	
	DECLARE_READ8_MEMBER( serial_mode_r );
	DECLARE_WRITE8_MEMBER( serial_mode_w );
	DECLARE_READ8_MEMBER( bitrate_r );
	DECLARE_WRITE8_MEMBER( bitrate_w );
	DECLARE_READ8_MEMBER( serial_control_r );
	DECLARE_WRITE8_MEMBER( serial_control_w );

	DECLARE_READ8_MEMBER( transmit_data_r );
	DECLARE_WRITE8_MEMBER( transmit_data_w );
	DECLARE_READ8_MEMBER( serial_status_r );
	DECLARE_WRITE8_MEMBER( serial_ack_w );
	DECLARE_READ8_MEMBER( receive_data_r );

	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;
protected:
	// device-level overrides
//	virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start() override;
	virtual void device_reset() override;
private:
	const address_space_config      m_space_config;
	UINT8 m_smr;
	UINT8 m_scr;
	UINT8 m_ssr;
	UINT8 m_brr;
};


// device type definition
extern const device_type SH7604_SCI;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
