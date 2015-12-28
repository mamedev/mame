// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    VTech Laser/VZ Memory Expansions

***************************************************************************/

#pragma once

#ifndef __VTECH_MEMEXP_MEMORY__
#define __VTECH_MEMEXP_MEMORY__

#include "emu.h"
#include "memexp.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> laser110_16k_device

class laser110_16k_device : public device_t, public device_memexp_interface
{
public:
	// construction/destruction
	laser110_16k_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	std::vector<UINT8> m_ram;
};

// ======================> laser210_16k_device

class laser210_16k_device : public device_t, public device_memexp_interface
{
public:
	// construction/destruction
	laser210_16k_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	std::vector<UINT8> m_ram;
};

// ======================> laser310_16k_device

class laser310_16k_device : public device_t, public device_memexp_interface
{
public:
	// construction/destruction
	laser310_16k_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	std::vector<UINT8> m_ram;
};

// ======================> laser_64k_device

class laser_64k_device : public device_t, public device_memexp_interface
{
public:
	// construction/destruction
	laser_64k_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE8_MEMBER( bankswitch_w );

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	std::vector<UINT8> m_ram;
};

// device type definition
extern const device_type LASER110_16K;
extern const device_type LASER210_16K;
extern const device_type LASER310_16K;
extern const device_type LASER_64K;

#endif // __VTECH_MEMEXP_MEMORY__
