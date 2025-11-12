// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_PC98_61SIMM_SIMM_H
#define MAME_BUS_PC98_61SIMM_SIMM_H

#pragma once

#include "slot.h"

class pc9801_61_2mb_device : public device_t
					  	   , public device_pc9801_61_interface
{
public:
	pc9801_61_2mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};

class pc9801_61_4mb_device : public device_t
					  	   , public device_pc9801_61_interface
{
public:
	pc9801_61_4mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};

class pc9801_61_8mb_device : public device_t
					  	   , public device_pc9801_61_interface
{
public:
	pc9801_61_8mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};

class pc9801_61_16mb_device : public device_t
					  	    , public device_pc9801_61_interface
{
public:
	pc9801_61_16mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};

class pc9801_61_20mb_device : public device_t
					  	    , public device_pc9801_61_interface
{
public:
	pc9801_61_20mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};

class pc9801_61_32mb_device : public device_t
					  	    , public device_pc9801_61_interface
{
public:
	pc9801_61_32mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};

class pc9801_61_64mb_device : public device_t
					  	    , public device_pc9801_61_interface
{
public:
	pc9801_61_64mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};


DECLARE_DEVICE_TYPE(PC9801_61_2MB,   pc9801_61_2mb_device)
DECLARE_DEVICE_TYPE(PC9801_61_4MB,   pc9801_61_4mb_device)
DECLARE_DEVICE_TYPE(PC9801_61_8MB,   pc9801_61_8mb_device)
DECLARE_DEVICE_TYPE(PC9801_61_16MB,  pc9801_61_16mb_device)
DECLARE_DEVICE_TYPE(PC9801_61_20MB,  pc9801_61_20mb_device)
DECLARE_DEVICE_TYPE(PC9801_61_32MB,  pc9801_61_32mb_device)
DECLARE_DEVICE_TYPE(PC9801_61_64MB,  pc9801_61_64mb_device)

#endif // MAME_BUS_PC98_61SIMM_SIMM_H
