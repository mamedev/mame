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
	void pia2_pa_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pia2_firq_a(int state);
	void pia2_firq_b(int state);

	/* psg */
	uint8_t psg_porta_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void psg_porta_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	/* fdc */
	void fdc_intrq_w(int state);
	void fdc_drq_w(int state);

protected:
	/* driver overrides */
	virtual void device_start(void) override;
	virtual void device_reset(void) override;

	/* interrupts */
	virtual bool firq_get_line(void) override;

	/* PIA1 */
	virtual uint8_t ff20_read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) override;
	virtual void ff20_write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff) override;

private:
	uint8_t m_just_reset;

	/* modem */
	uint8_t modem_r(offs_t offset);
	void modem_w(offs_t offset, uint8_t data);
};

#endif /* __DGNALPHA__ */
