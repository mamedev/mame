// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_CPU_F2MC16_MB9061X_H
#define MAME_CPU_F2MC16_MB9061X_H

#pragma once

#include "f2mc16.h"

//**************************************************************************
//  CONSTANTS
//**************************************************************************

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> m5074x_device

class mb9061x_device :  public f2mc16_device
{
	friend class mb90610_device;
	friend class mb90611_device;

public:
	const address_space_config m_program_config;

protected:
	// construction/destruction
	mb9061x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, address_map_constructor internal_map);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void execute_set_input(int inputnum, int state) override;
	virtual space_config_vector memory_space_config() const override;

private:
};

class mb90610_device : public mb9061x_device
{
public:
	mb90610_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void mb90610_map(address_map &map);
protected:
	mb90610_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};

class mb90611_device : public mb9061x_device
{
public:
	mb90611_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void mb90611_map(address_map &map);
protected:
	mb90611_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(MB90610A, mb90610_device)
DECLARE_DEVICE_TYPE(MB90611A, mb90611_device)

#endif // MAME_CPU_F2MC16_MB9061X_H
