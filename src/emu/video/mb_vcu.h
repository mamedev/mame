// license: ?
// copyright-holders: Angelo Salese
/***************************************************************************

Template for skeleton device

***************************************************************************/

#pragma once

#ifndef __MB_VCUDEV_H__
#define __MB_VCUDEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MB_VCU_ADD(_tag,_freq,_config) \
	MCFG_DEVICE_ADD(_tag, MB_VCU, _freq) \
	MCFG_DEVICE_CONFIG(_config)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mb_vcu_interface

struct mb_vcu_interface
{
	const char         *m_cpu_tag;
};

// ======================> mb_vcu_device

class mb_vcu_device : public device_t,
					  public device_video_interface,
					  public mb_vcu_interface
{
public:
	// construction/destruction
	mb_vcu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// I/O operations
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start();
	virtual void device_reset();
private:

	device_t *m_cpu;
};


// device type definition
extern const device_type MB_VCU;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
