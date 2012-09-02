/**********************************************************************

    Diag264 User Port Loop Back Connector emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __DIAG264_USER_PORT_LOOPBACK__
#define __DIAG264_USER_PORT_LOOPBACK__


#include "emu.h"
#include "machine/plus4user.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> diag264_user_port_loopback_device

class diag264_user_port_loopback_device :  public device_t,
						   				   public device_plus4_user_port_interface
{
public:
    // construction/destruction
    diag264_user_port_loopback_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
    // device-level overrides
	virtual void device_config_complete() { m_shortname = "diag264_user_port_loopback"; }
    virtual void device_start();

	// device_plus4_user_port_interface overrides
    virtual UINT8 plus4_p_r() { logerror("P read %02x\n", ((m_p << 4) & 0xf0) | (m_p >> 4)); return ((m_p << 4) & 0xf0) | (m_p >> 4); }
    virtual void plus4_p_w(UINT8 data) { logerror("P write %02x\n", data); m_p = data; }
    virtual int plus4_rxd_r() { return m_txd; }
    virtual int plus4_dcd_r() { return m_dtr; }
    virtual int plus4_dsr_r() { return m_rts; }
    virtual void plus4_txd_w(int state) { m_txd = state; }
    virtual void plus4_dtr_w(int state) { m_dtr = state; }
    virtual void plus4_rts_w(int state) { m_rts = state; }

private:
    UINT8 m_p;

    int m_txd;
    int m_rts;
    int m_dtr;
};


// device type definition
extern const device_type DIAG264_USER_PORT_LOOPBACK;



#endif
