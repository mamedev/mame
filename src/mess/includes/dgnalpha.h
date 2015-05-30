// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    dgnalpha.h

    Dragon Alpha

***************************************************************************/

#pragma once

#ifndef __DGNALPHA__
#define __DGNALPHA__


#include "includes/dragon.h"
#include "sound/ay8910.h"
#include "machine/wd_fdc.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

/* devices */
#define PIA2_TAG                    "pia2"
#define AY8912_TAG                  "ay8912"
#define WD2797_TAG                  "wd2797"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class dragon_alpha_state : public dragon64_state
{
public:
	dragon_alpha_state(const machine_config &mconfig, device_type type, const char *tag)
	: dragon64_state(mconfig, type, tag),
		m_pia_2(*this, PIA2_TAG),
		m_ay8912(*this, AY8912_TAG),
		m_fdc(*this, WD2797_TAG),
		m_floppy0(*this, WD2797_TAG ":0"),
		m_floppy1(*this, WD2797_TAG ":1"),
		m_floppy2(*this, WD2797_TAG ":2"),
		m_floppy3(*this, WD2797_TAG ":3")
	{
	}

	DECLARE_FLOPPY_FORMATS(dragon_formats);

	required_device<pia6821_device> m_pia_2;
	required_device<ay8912_device> m_ay8912;
	required_device<wd2797_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	required_device<floppy_connector> m_floppy2;
	required_device<floppy_connector> m_floppy3;


	/* pia2 */
	DECLARE_WRITE8_MEMBER( pia2_pa_w );
	DECLARE_WRITE_LINE_MEMBER( pia2_firq_a );
	DECLARE_WRITE_LINE_MEMBER( pia2_firq_b );

	/* psg */
	DECLARE_READ8_MEMBER( psg_porta_read );
	DECLARE_WRITE8_MEMBER( psg_porta_write );

	/* fdc */
	DECLARE_WRITE_LINE_MEMBER( fdc_intrq_w );
	DECLARE_WRITE_LINE_MEMBER( fdc_drq_w );

protected:
	/* driver overrides */
	virtual void device_start(void);
	virtual void device_reset(void);

	/* interrupts */
	virtual bool firq_get_line(void);

	/* PIA1 */
	virtual DECLARE_READ8_MEMBER( ff20_read );
	virtual DECLARE_WRITE8_MEMBER( ff20_write );

private:
	UINT8 m_just_reset;

	/* modem */
	UINT8 modem_r(offs_t offset);
	void modem_w(offs_t offset, UINT8 data);
};

#endif /* __DGNALPHA__ */
