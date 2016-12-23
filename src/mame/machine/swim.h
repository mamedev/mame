// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    swim.h

    Implementation of the Apple SWIM FDC controller; used on (less)
    early Macs

*********************************************************************/

#ifndef __SWIM_H__
#define __SWIM_H__

#include "machine/applefdc.h"


/***************************************************************************
    DEVICE
***************************************************************************/

extern const device_type SWIM;

class swim_device : public applefdc_base_device
{
public:
	swim_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// read/write
	virtual uint8_t read(uint8_t offset) override;
	virtual void write(uint8_t offset, uint8_t data) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// other overrides
	virtual void iwm_modereg_w(uint8_t data) override;

private:
	uint8_t       m_swim_mode;
	uint8_t       m_swim_magic_state;
	uint8_t       m_parm_offset;
	uint8_t       m_ism_regs[8];
	uint8_t       m_parms[16];
};



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_SWIM_ADD(_tag, _intrf) \
	MCFG_DEVICE_ADD(_tag, SWIM, 0) \
	MCFG_APPLEFDC_CONFIG(_intrf)

#define MCFG_SWIM_MODIFY(_tag, _intrf) \
	MCFG_DEVICE_MODIFY(_tag)          \
	MCFG_APPLEFDC_CONFIG(_intrf)

#endif // __SWIM_H__
