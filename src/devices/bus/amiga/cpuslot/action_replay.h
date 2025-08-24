// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Datel Electronics Action Replay

    Freezer cartridge for the A500 and A2000

***************************************************************************/

#ifndef MAME_BUS_AMIGA_CPUSLOT_ACTION_REPLAY_H
#define MAME_BUS_AMIGA_CPUSLOT_ACTION_REPLAY_H

#pragma once

#include "cpuslot.h"


namespace bus::amiga::cpuslot {

class action_replay_device_base : public device_t, public device_amiga_cpuslot_interface
{
public:
	DECLARE_INPUT_CHANGED_MEMBER( freeze ) { freeze_w(newval); };
	virtual void freeze_w(int state) = 0;

protected:
	action_replay_device_base(const machine_config &mconfig, device_type type, size_t ram_size, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	required_region_ptr<uint16_t> m_rom;
	memory_share_creator<uint16_t> m_ram;
};

class action_replay_mk1_device : public action_replay_device_base
{
public:
	action_replay_mk1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// action_replay_device_base overrides
	virtual void freeze_w(int state) override;
};

class action_replay_mk2_device : public action_replay_device_base
{
public:
	action_replay_mk2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	action_replay_mk2_device(const machine_config &mconfig, device_type type, size_t ram_size, const char *tag, device_t *owner, uint32_t clock);

	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_cpuslot_interface overrides
	virtual void rst_w(int state) override;

	// action_replay_device_base overrides
	virtual void freeze_w(int state) override;

private:
	void regs_map(address_map &map) ATTR_COLD;
	void install_chipmem_taps();

	uint16_t status_r(offs_t offset, uint16_t mem_mask);
	void mode_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	void restore_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	static constexpr uint8_t STATUS_BUTTON = 0x00;
	static constexpr uint8_t STATUS_READ = 0x01; // 0xbfe001
	static constexpr uint8_t STATUS_WRITE = 0x02; // 0xbfd100
	static constexpr uint8_t STATUS_RESET = 0x03;

	memory_passthrough_handler m_custom_chip_tap;
	memory_passthrough_handler m_chipmem_read_tap;
	memory_passthrough_handler m_chipmem_write_tap;

	uint8_t m_status;
	uint8_t m_mode;

	bool m_nmi_active;
	bool m_reset;
};

class action_replay_mk3_device : public action_replay_mk2_device
{
public:
	action_replay_mk3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

} // namespace bus::amiga::cpuslot

// device type declaration
DECLARE_DEVICE_TYPE_NS(AMIGA_CPUSLOT_ACTION_REPLAY_MK1, bus::amiga::cpuslot, action_replay_mk1_device)
DECLARE_DEVICE_TYPE_NS(AMIGA_CPUSLOT_ACTION_REPLAY_MK2, bus::amiga::cpuslot, action_replay_mk2_device)
DECLARE_DEVICE_TYPE_NS(AMIGA_CPUSLOT_ACTION_REPLAY_MK3, bus::amiga::cpuslot, action_replay_mk3_device)

#endif // MAME_BUS_AMIGA_CPUSLOT_ACTION_REPLAY_H
