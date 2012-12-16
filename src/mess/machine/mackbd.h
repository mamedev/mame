#pragma once

#ifndef __MACKBD_H__
#define __MACKBD_H__

#include "emu.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define MACKBD_TAG	"mackbd"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MACKBD_ADD() \
    MCFG_DEVICE_ADD(MACKBD_TAG, MACKBD, 0)

#define MCFG_MACKBD_REPLACE() \
    MCFG_DEVICE_REPLACE(MACKBD_TAG, MACKBD, 0)

#define MCFG_MACKBD_REMOVE() \
    MCFG_DEVICE_REMOVE(MACKBD_TAG)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mackbd_device

class mackbd_device :  public device_t
{
public:
    // construction/destruction
    mackbd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER(p0_r);
	DECLARE_WRITE8_MEMBER(p0_w);
	DECLARE_READ8_MEMBER(p1_r);
	DECLARE_WRITE8_MEMBER(p1_w);
	DECLARE_READ8_MEMBER(p2_r);
	DECLARE_WRITE8_MEMBER(p2_w);
	DECLARE_READ8_MEMBER(t1_r);

protected:
    // device-level overrides
    virtual void device_start();
    virtual void device_reset();
    virtual void device_config_complete();
    virtual machine_config_constructor device_mconfig_additions() const;
    virtual const rom_entry *device_rom_region() const;

    required_device<cpu_device> m_maincpu;

private:

//  devcb_resolved_write_line   m_out_reset_func;
};

// device type definition
extern const device_type MACKBD;

#endif
