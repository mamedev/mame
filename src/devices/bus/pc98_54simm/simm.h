// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_PC98_54SIMM_SIMM_H
#define MAME_BUS_PC98_54SIMM_SIMM_H

#pragma once

#include "slot.h"

class pc9801_54_2mb_device : public device_t
						   , public device_pc9801_54_interface
{
public:
	pc9801_54_2mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};

class pc9801_54_4mb_device : public device_t
						   , public device_pc9801_54_interface
{
public:
	pc9801_54_4mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};

class pc9801_54_7mb_device : public device_t
						   , public device_pc9801_54_interface
{
public:
	pc9801_54_7mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};


class pc9801_54_8mb_device : public device_t
						   , public device_pc9801_54_interface
{
public:
	pc9801_54_8mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};

class pc9801_54_9mb_device : public device_t
						   , public device_pc9801_54_interface
{
public:
	pc9801_54_9mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};

class pc9801_54_15mb_device : public device_t
						   , public device_pc9801_54_interface
{
public:
	pc9801_54_15mb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
};


DECLARE_DEVICE_TYPE(PC9801_54_2MB,   pc9801_54_2mb_device)
DECLARE_DEVICE_TYPE(PC9801_54_4MB,   pc9801_54_4mb_device)
DECLARE_DEVICE_TYPE(PC9801_54_7MB,   pc9801_54_7mb_device)
DECLARE_DEVICE_TYPE(PC9801_54_8MB,   pc9801_54_8mb_device)
DECLARE_DEVICE_TYPE(PC9801_54_9MB,   pc9801_54_9mb_device)
DECLARE_DEVICE_TYPE(PC9801_54_15MB,  pc9801_54_15mb_device)

#endif // MAME_BUS_PC98_54SIMM_SIMM_H
