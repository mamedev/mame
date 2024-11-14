// license:BSD-3-Clause
// copyright-holders:Katherine Rohl
#ifndef MAME_BUS_VME_MVME120_H
#define MAME_BUS_VME_MVME120_H

#pragma once

#include "cpu/m68000/m68010.h"
#include "bus/vme/vme.h"
#include "bus/rs232/rs232.h"
#include "machine/clock.h"
#include "machine/mc68901.h"

DECLARE_DEVICE_TYPE(VME_MVME120,   vme_mvme120_card_device)
DECLARE_DEVICE_TYPE(VME_MVME121,   vme_mvme121_card_device)
DECLARE_DEVICE_TYPE(VME_MVME122,   vme_mvme122_card_device)
DECLARE_DEVICE_TYPE(VME_MVME123,   vme_mvme123_card_device)

//**************************************************************************
//  Base Device declaration
//**************************************************************************
class vme_mvme120_device :  public device_t, public device_vme_card_interface
{
public:
	/* Board types */
	enum mvme12x_variant
	{
		mvme120_board,
		mvme121_board,
		mvme122_board,
		mvme123_board
	};

	vme_mvme120_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, mvme12x_variant board_id);

	// Switch and jumper handlers
	DECLARE_INPUT_CHANGED_MEMBER(s3_autoboot);
	DECLARE_INPUT_CHANGED_MEMBER(s3_baudrate);

	uint16_t vme_to_ram_r(address_space &space, offs_t offset, uint16_t mem_mask);
	void vme_to_ram_w(address_space &space, offs_t address, uint16_t data, uint16_t mem_mask);

protected:
	void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void mvme12x_base_mem(address_map &map) ATTR_COLD;
	void mvme120_mem(address_map &map) ATTR_COLD;
	void mvme121_mem(address_map &map) ATTR_COLD;
	void mvme122_mem(address_map &map) ATTR_COLD;
	void mvme123_mem(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;

	required_device<mc68901_device> m_mfp;
	required_device<rs232_port_device> m_rs232;

	required_ioport m_input_s3;

	memory_passthrough_handler m_rom_shadow_tap;

	required_region_ptr<uint16_t> m_sysrom;
	required_shared_ptr<uint16_t> m_localram;

	uint8_t     m_ctrlreg;              // "VME120 Control Register"
	uint8_t     m_memory_read_count;    // For boot ROM shadowing $000000

	uint8_t     ctrlreg_r(offs_t offset);
	void        ctrlreg_w(offs_t offset, uint8_t data);

	// VMEbus dummy lines
	void vme_bus_timeout();
	uint16_t vme_a24_r();
	void vme_a24_w(uint16_t data);
	uint16_t vme_a16_r();
	void vme_a16_w(uint16_t data);

	void rom_shadow_tap(offs_t address, u16 data, u16 mem_mask);

	void watchdog_reset(int state);
	void mfp_interrupt(int state);

	const mvme12x_variant  m_board_id;
};


//**************************************************************************
//  Board Device declarations
//**************************************************************************

class vme_mvme120_card_device : public vme_mvme120_device
{
public:
	vme_mvme120_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_mvme120_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: vme_mvme120_device(mconfig, type, tag, owner, clock, mvme120_board)
	{ }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class vme_mvme121_card_device : public vme_mvme120_device
{
public:
	vme_mvme121_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_mvme121_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: vme_mvme120_device(mconfig, type, tag, owner, clock, mvme121_board)
	{ }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


class vme_mvme122_card_device : public vme_mvme120_device
{
public:
	vme_mvme122_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_mvme122_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: vme_mvme120_device(mconfig, type, tag, owner, clock, mvme122_board)
	{ }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


class vme_mvme123_card_device : public vme_mvme120_device
{
public:
	vme_mvme123_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_mvme123_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: vme_mvme120_device(mconfig, type, tag, owner, clock, mvme123_board)
	{ }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

#endif // MAME_BUS_VME_MVME120_H
