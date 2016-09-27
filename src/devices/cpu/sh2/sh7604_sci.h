// license:BSD-3-Clause
// copyright-holders:<author_name>
/***************************************************************************

Template for skeleton device

***************************************************************************/

#pragma once

#ifndef __SH7604_SCIDEV_H__
#define __SH7604_SCIDEV_H__



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

	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;
protected:
	// device-level overrides
//	virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start() override;
	virtual void device_reset() override;
private:
	const address_space_config      m_space_config;
};


// device type definition
extern const device_type SH7604_SCI;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
