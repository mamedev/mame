/**********************************************************************

    Commodore D9060/D9090 Hard Disk Drive emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************

    D9060: Tandon TM602S
    chdman -createblankhd d9060.chd 153 4 32 256

    D9090: Tandon TM603S
    chdman -createblankhd d9090.chd 153 6 32 256

    How to format the disk:
    HEADER "LABEL",D0,I01

**********************************************************************/

#pragma once

#ifndef __D9060__
#define __D9060__


#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "machine/6522via.h"
#include "machine/6532riot.h"
#include "machine/ieee488.h"
#include "machine/scsibus.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> base_d9060_device

class base_d9060_device :  public device_t,
						   public device_ieee488_interface
{

public:
	enum
	{
		TYPE_9060,
		TYPE_9090
	};

	// construction/destruction
    base_d9060_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, UINT32 variant);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

	// not really public
	DECLARE_WRITE_LINE_MEMBER( req_w );
	DECLARE_READ8_MEMBER( dio_r );
	DECLARE_WRITE8_MEMBER( dio_w );
	DECLARE_READ8_MEMBER( riot1_pa_r );
	DECLARE_WRITE8_MEMBER( riot1_pa_w );
	DECLARE_READ8_MEMBER( riot1_pb_r );
	DECLARE_WRITE8_MEMBER( riot1_pb_w );
	DECLARE_READ8_MEMBER( via_pb_r );
	DECLARE_WRITE8_MEMBER( via_pb_w );
	DECLARE_READ_LINE_MEMBER( req_r );
	DECLARE_WRITE_LINE_MEMBER( ack_w );
	DECLARE_WRITE_LINE_MEMBER( enable_w );

protected:
    // device-level overrides
    virtual void device_start();
	virtual void device_reset();
    virtual void device_config_complete();

	// device_ieee488_interface overrides
	void ieee488_atn(int state);
	void ieee488_ifc(int state);

private:
	inline void update_ieee_signals();

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_hdccpu;
	required_device<riot6532_device> m_riot0;
	required_device<riot6532_device> m_riot1;
	required_device<via6522_device> m_via;
	required_device<scsicb_device> m_sasibus;

	// IEEE-488 bus
	int m_rfdo;							// not ready for data output
	int m_daco;							// not data accepted output
	int m_atna;							// attention acknowledge

	// SASI bus
	int m_enable;

	int m_variant;
};


// ======================> d9060_device

class d9060_device :  public base_d9060_device
{
public:
    // construction/destruction
    d9060_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// ======================> d9090_device

class d9090_device :  public base_d9060_device
{
public:
    // construction/destruction
    d9090_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// device type definition
extern const device_type D9060;
extern const device_type D9090;



#endif
