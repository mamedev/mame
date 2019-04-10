// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
#ifndef MAME_BUS_VME_VME_FCCPU20_H
#define MAME_BUS_VME_VME_FCCPU20_H

#pragma once

#include "bus/vme/vme.h"
#include "machine/68561mpcc.h"
#include "machine/68230pit.h"
#include "machine/68153bim.h"

DECLARE_DEVICE_TYPE(VME_FCCPU20,   vme_fccpu20_card_device)
DECLARE_DEVICE_TYPE(VME_FCCPU21S,  vme_fccpu21s_card_device)
DECLARE_DEVICE_TYPE(VME_FCCPU21,   vme_fccpu21_card_device)
DECLARE_DEVICE_TYPE(VME_FCCPU21A,  vme_fccpu21a_card_device)
DECLARE_DEVICE_TYPE(VME_FCCPU21YA, vme_fccpu21ya_card_device)
DECLARE_DEVICE_TYPE(VME_FCCPU21B,  vme_fccpu21b_card_device)
DECLARE_DEVICE_TYPE(VME_FCCPU21YB, vme_fccpu21yb_card_device)

//**************************************************************************
//  Base Device declaration
//**************************************************************************
class vme_fccpu20_device :  public device_t, public device_vme_card_interface
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

	vme_fccpu20_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, fc_board_t board_id);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	uint8_t bim_irq_state;
	int bim_irq_level;

	emu_timer *m_arbiter_start; // Need a startup delay because it is hooked up to the sense inputs of the PIT

	required_device<cpu_device> m_maincpu;

private:
	DECLARE_WRITE_LINE_MEMBER(bim_irq_callback);

	/* PIT callbacks */
	DECLARE_READ8_MEMBER (pita_r);
	DECLARE_READ8_MEMBER (pitb_r);
	DECLARE_READ8_MEMBER (pitc_r);

	// Below are duplicated declarations from src/mame/drivers/fccpu20.cpp
	DECLARE_READ32_MEMBER (bootvect_r);
	DECLARE_WRITE32_MEMBER (bootvect_w);

	void cpu20_mem(address_map &map);
	void cpu_space_map(address_map &map);

	required_device<pit68230_device> m_pit;
	required_device<bim68153_device> m_bim;
	required_device<mpcc68561_device> m_mpcc;
	required_device<mpcc68561_device> m_mpcc2;
	required_device<mpcc68561_device> m_mpcc3;

	// Pointer to System ROMs needed by bootvect_r and masking RAM buffer for post reset accesses
	uint32_t    *m_sysrom;
	uint32_t    m_sysram[2];
	void        update_irq_to_maincpu();
	const fc_board_t  m_board_id;
};

//**************************************************************************
//  Board Device declarations
//**************************************************************************

class vme_fccpu20_card_device : public vme_fccpu20_device
{
public :
	vme_fccpu20_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_fccpu20_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: vme_fccpu20_device(mconfig, type, tag, owner, clock, cpu20)
	{ }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
};

class vme_fccpu21s_card_device : public vme_fccpu20_device
{
public :
	vme_fccpu21s_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_fccpu21s_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: vme_fccpu20_device(mconfig, type, tag, owner, clock, cpu21s)
	{ }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
};

class vme_fccpu21_card_device : public vme_fccpu20_device
{
public :
	vme_fccpu21_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_fccpu21_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: vme_fccpu20_device(mconfig, type, tag, owner, clock, cpu21)
	{ }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
};

class vme_fccpu21a_card_device : public vme_fccpu20_device
{
public :
	vme_fccpu21a_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_fccpu21a_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: vme_fccpu20_device(mconfig, type, tag, owner, clock, cpu21a)
	{ }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
};

class vme_fccpu21ya_card_device : public vme_fccpu20_device
{
public :
	vme_fccpu21ya_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_fccpu21ya_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: vme_fccpu20_device(mconfig, type, tag, owner, clock, cpu21ya)
	{ }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
};

class vme_fccpu21b_card_device : public vme_fccpu20_device
{
public :
	vme_fccpu21b_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_fccpu21b_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: vme_fccpu20_device(mconfig, type, tag, owner, clock, cpu21b)
	{ }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
};

class vme_fccpu21yb_card_device : public vme_fccpu20_device
{
public :
	vme_fccpu21yb_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_fccpu21yb_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
		: vme_fccpu20_device(mconfig, type, tag, owner, clock, cpu21yb)
	{ }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
};


#endif // MAME_BUS_VME_VME_FCCPU20_H
