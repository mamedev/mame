// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
#ifndef MAME_BUS_VME_SYS68K_CPU20_H
#define MAME_BUS_VME_SYS68K_CPU20_H

#pragma once

#include "bus/vme/vme.h"
#include "machine/68561mpcc.h"
#include "machine/68230pit.h"
#include "machine/68153bim.h"

DECLARE_DEVICE_TYPE(VME_SYS68K_CPU20,   vme_sys68k_cpu20_card_device)
DECLARE_DEVICE_TYPE(VME_SYS68K_CPU21S,  vme_sys68k_cpu21s_card_device)
DECLARE_DEVICE_TYPE(VME_SYS68K_CPU21,   vme_sys68k_cpu21_card_device)
DECLARE_DEVICE_TYPE(VME_SYS68K_CPU21A,  vme_sys68k_cpu21a_card_device)
DECLARE_DEVICE_TYPE(VME_SYS68K_CPU21YA, vme_sys68k_cpu21ya_card_device)
DECLARE_DEVICE_TYPE(VME_SYS68K_CPU21B,  vme_sys68k_cpu21b_card_device)
DECLARE_DEVICE_TYPE(VME_SYS68K_CPU21YB, vme_sys68k_cpu21yb_card_device)

//**************************************************************************
//  Base Device declaration
//**************************************************************************
class vme_sys68k_cpu20_card_device_base :  public device_t, public device_vme_card_interface
{
protected:
	// PIT port C Board ID bits
	static constexpr unsigned CPU20 = 0x40;
	static constexpr unsigned CPU21 = 0x00;

	/* Board types */
	enum fc_board_t {
		cpu20,
		cpu21,
		cpu21a,
		cpu21ya,
		cpu21b,
		cpu21yb,
		cpu21s
	};

	vme_sys68k_cpu20_card_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, fc_board_t board_id);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(grant_bus);

	int m_bim_irq_state;
	uint8_t m_bim_irq_level;

	emu_timer *m_arbiter_start; // Need a startup delay because it is hooked up to the sense inputs of the PIT

	required_device<cpu_device> m_maincpu;

private:
	void bim_irq_callback(int state);

	/* PIT callbacks */
	uint8_t pita_r();
	uint8_t pitb_r();
	uint8_t pitc_r();

	void cpu20_mem(address_map &map) ATTR_COLD;
	void cpu_space_map(address_map &map) ATTR_COLD;

	required_device<pit68230_device> m_pit;
	required_device<bim68153_device> m_bim;
	required_device<mpcc68561_device> m_mpcc;
	required_device<mpcc68561_device> m_mpcc2;
	required_device<mpcc68561_device> m_mpcc3;

	required_memory_region m_eprom;
	required_shared_ptr<uint32_t> m_ram;
	memory_passthrough_handler m_boot_mph;

	void        update_irq_to_maincpu();
	const fc_board_t  m_board_id;
};

//**************************************************************************
//  Board Device declarations
//**************************************************************************

class vme_sys68k_cpu20_card_device : public vme_sys68k_cpu20_card_device_base
{
public :
	vme_sys68k_cpu20_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_sys68k_cpu20_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: vme_sys68k_cpu20_card_device_base(mconfig, type, tag, owner, clock, cpu20)
	{ }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class vme_sys68k_cpu21s_card_device : public vme_sys68k_cpu20_card_device_base
{
public :
	vme_sys68k_cpu21s_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_sys68k_cpu21s_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: vme_sys68k_cpu20_card_device_base(mconfig, type, tag, owner, clock, cpu21s)
	{ }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class vme_sys68k_cpu21_card_device : public vme_sys68k_cpu20_card_device_base
{
public :
	vme_sys68k_cpu21_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_sys68k_cpu21_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: vme_sys68k_cpu20_card_device_base(mconfig, type, tag, owner, clock, cpu21)
	{ }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class vme_sys68k_cpu21a_card_device : public vme_sys68k_cpu20_card_device_base
{
public :
	vme_sys68k_cpu21a_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_sys68k_cpu21a_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: vme_sys68k_cpu20_card_device_base(mconfig, type, tag, owner, clock, cpu21a)
	{ }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class vme_sys68k_cpu21ya_card_device : public vme_sys68k_cpu20_card_device_base
{
public :
	vme_sys68k_cpu21ya_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_sys68k_cpu21ya_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: vme_sys68k_cpu20_card_device_base(mconfig, type, tag, owner, clock, cpu21ya)
	{ }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class vme_sys68k_cpu21b_card_device : public vme_sys68k_cpu20_card_device_base
{
public :
	vme_sys68k_cpu21b_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_sys68k_cpu21b_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: vme_sys68k_cpu20_card_device_base(mconfig, type, tag, owner, clock, cpu21b)
	{ }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class vme_sys68k_cpu21yb_card_device : public vme_sys68k_cpu20_card_device_base
{
public :
	vme_sys68k_cpu21yb_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_sys68k_cpu21yb_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: vme_sys68k_cpu20_card_device_base(mconfig, type, tag, owner, clock, cpu21yb)
	{ }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};


#endif // MAME_BUS_VME_SYS68K_CPU20_H
