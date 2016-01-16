// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
#pragma once

#ifndef __NORTHBRIDGE_H__
#define __NORTHBRIDGE_H__

#include "emu.h"

#include "machine/ram.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> northbridge_device

class northbridge_device :
		public device_t
{
public:
		// construction/destruction
		northbridge_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);
protected:
		// device-level overrides
		virtual void device_start() override;
		virtual void device_reset() override;
public:
		required_device<cpu_device> m_maincpu;
		required_device<ram_device> m_ram;

};

#endif  /* __NORTHBRIDGE_H__ */
