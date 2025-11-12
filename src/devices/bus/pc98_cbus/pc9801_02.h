// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_BUS_PC98_CBUS_PC9801_02_H
#define MAME_BUS_PC98_CBUS_PC9801_02_H

#pragma once

#include "slot.h"

class pc9801_02_128kb_device : public device_t
							 , public device_pc98_cbus_slot_interface
{
public:
	pc9801_02_128kb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void remap(int space_id, offs_t start, offs_t end) override;

private:
	std::vector<u16> m_ram;
};

class pc9801_02_256kb_device : public device_t
							 , public device_pc98_cbus_slot_interface
{
public:
	pc9801_02_256kb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void remap(int space_id, offs_t start, offs_t end) override;

private:
	std::vector<u16> m_ram;
};

class pc9801_02_384kb_device : public device_t
							 , public device_pc98_cbus_slot_interface
{
public:
	pc9801_02_384kb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void remap(int space_id, offs_t start, offs_t end) override;

private:
	std::vector<u16> m_ram;
};

class pc9801_02_512kb_device : public device_t
							 , public device_pc98_cbus_slot_interface
{
public:
	pc9801_02_512kb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void remap(int space_id, offs_t start, offs_t end) override;

private:
	std::vector<u16> m_ram;
};

class pc9801_02_640kb_device : public device_t
							 , public device_pc98_cbus_slot_interface
{
public:
	pc9801_02_640kb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void remap(int space_id, offs_t start, offs_t end) override;

private:
	std::vector<u16> m_ram;
};


// device type definition
DECLARE_DEVICE_TYPE(PC9801_02_128KB, pc9801_02_128kb_device)
DECLARE_DEVICE_TYPE(PC9801_02_256KB, pc9801_02_256kb_device)
DECLARE_DEVICE_TYPE(PC9801_02_384KB, pc9801_02_384kb_device)
DECLARE_DEVICE_TYPE(PC9801_02_512KB, pc9801_02_512kb_device)
DECLARE_DEVICE_TYPE(PC9801_02_640KB, pc9801_02_640kb_device)


#endif // MAME_BUS_PC98_CBUS_PC9801_02_H
