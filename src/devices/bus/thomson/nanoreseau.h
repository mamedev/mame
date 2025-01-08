// license:BSD-3-Clause
// copyright-holders:Antoine Mine, Olivier Galibert

// The "Nanoreseau" was a proprietary networking for MO/TO Thomson
// computers using rs-485.  A PC is supposed to be used as a network
// head.

#ifndef MAME_BUS_THOMSON_NANORESEAU_H
#define MAME_BUS_THOMSON_NANORESEAU_H

#include "extension.h"
#include "machine/mc6854.h"

class nanoreseau_device : public device_t, public thomson_extension_interface
{
public:
	nanoreseau_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, bool no_id = false);
	virtual ~nanoreseau_device() = default;

	virtual void rom_map(address_map &map) override ATTR_COLD;
	virtual void io_map(address_map &map) override ATTR_COLD;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(answer_tick);

private:
	required_device<mc6854_device> m_mc6854;
	required_memory_region m_rom;
	required_ioport m_id;
	bool m_no_id;

	emu_timer *m_timer;
	int m_answer_step;

	void got_frame(uint8_t *data, int length);
	u8 id_r();
};

class nanoreseau_mo_device : public nanoreseau_device
{
public:
	nanoreseau_mo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0, bool no_id = false);
	virtual ~nanoreseau_mo_device() = default;

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class nanoreseau_to_device : public nanoreseau_device
{
public:
	nanoreseau_to_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0, bool no_id = false);
	virtual ~nanoreseau_to_device() = default;

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(NANORESEAU_MO, nanoreseau_mo_device)
DECLARE_DEVICE_TYPE(NANORESEAU_TO, nanoreseau_to_device)

#endif
