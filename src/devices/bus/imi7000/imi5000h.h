// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    IMI 5000H 5.25" Winchester Hard Disk Controller emulation

    Used in Corvus Systems H-Series drives (Model 6/11/20)

**********************************************************************/

#pragma once

#ifndef __IMI5000H__
#define __IMI5000H__

#include "emu.h"
#include "imi7000.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> imi5000h_device

class imi5000h_device :  public device_t,
							public device_imi7000_interface
{
public:
	// construction/destruction
	imi5000h_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual ioport_constructor device_input_ports() const override;

	DECLARE_WRITE_LINE_MEMBER( ctc_z0_w );
	DECLARE_WRITE_LINE_MEMBER( ctc_z1_w );
	DECLARE_WRITE_LINE_MEMBER( ctc_z2_w );

	DECLARE_READ8_MEMBER( pio0_pa_r );
	DECLARE_WRITE8_MEMBER( pio0_pa_w );
	DECLARE_READ8_MEMBER( pio0_pb_r );
	DECLARE_WRITE8_MEMBER( pio0_pb_w );

	DECLARE_READ8_MEMBER( pio2_pa_r );
	DECLARE_WRITE8_MEMBER( pio2_pa_w );
	DECLARE_READ8_MEMBER( pio2_pb_r );
	DECLARE_WRITE8_MEMBER( pio2_pb_w );

	DECLARE_READ8_MEMBER( pio3_pa_r );
	DECLARE_WRITE8_MEMBER( pio3_pa_w );
	DECLARE_READ8_MEMBER( pio3_pb_r );
	DECLARE_WRITE8_MEMBER( pio3_pb_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	enum
	{
		LED_FAULT,
		LED_BUSY,
		LED_READY
	};

	required_device<cpu_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_ioport m_lsi11;
	required_ioport m_mux;
	required_ioport m_format;
	required_ioport m_ub4;
};


// device type definition
extern const device_type IMI5000H;



#endif
