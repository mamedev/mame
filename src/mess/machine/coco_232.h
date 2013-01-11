#pragma once

#ifndef __COCO_232_H__
#define __COCO_232_H__

#include "emu.h"
#include "machine/cococart.h"
#include "machine/6551acia.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> coco_232_device

class coco_232_device :
		public device_t,
		public device_cococart_interface
{
public:
		// construction/destruction
		coco_232_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

		// optional information overrides
		virtual machine_config_constructor device_mconfig_additions() const;
protected:
		// device-level overrides
		virtual void device_start();
		virtual void device_config_complete() { m_shortname = "coco_232"; }
		virtual DECLARE_READ8_MEMBER(read);
		virtual DECLARE_WRITE8_MEMBER(write);
private:
		// internal state
		required_device<acia6551_device> m_uart;
};


// device type definition
extern const device_type COCO_232;

#endif  /* __COCO_232_H__ */
