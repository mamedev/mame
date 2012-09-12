/***************************************************************************

Template for skeleton device

***************************************************************************/

#pragma once

#ifndef __SEIBU_COPDEV_H__
#define __SEIBU_COPDEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SEIBU_COP_ADD(_tag,_freq) \
	MCFG_DEVICE_ADD(_tag, SEIBU_COP, _freq) \

#define SEIBU_COP_INTERFACE(_name) \
	const seibu_cop_interface (_name) =


struct seibu_cop_interface
{
	// memory accessors
	devcb_read8			m_in_mreq_cb;
	devcb_write8		m_out_mreq_cb;
};


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> seibu_cop_device

class seibu_cop_device :	public device_t,
							public seibu_cop_interface
{
public:
	// construction/destruction
	seibu_cop_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// I/O operations
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start();
	virtual void device_reset();

private:
	devcb_resolved_read8		m_in_mreq_func;
	devcb_resolved_write8		m_out_mreq_func;
};


// device type definition
extern const device_type SEIBU_COP;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
