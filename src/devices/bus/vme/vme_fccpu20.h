// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
#ifndef VME_FCCPU20_H
#define VME_FCCPU20_H
#pragma once

#include "emu.h"

#include "machine/68561mpcc.h"
#include "machine/68230pit.h"
#include "machine/68153bim.h"
#include "bus/vme/vme.h"

extern const device_type VME_FCCPU20;
extern const device_type VME_FCCPU21S;
extern const device_type VME_FCCPU21;
extern const device_type VME_FCCPU21A;
extern const device_type VME_FCCPU21YA;
extern const device_type VME_FCCPU21B;
extern const device_type VME_FCCPU21YB;

// PIT port C Board ID bits
#define CPU20      0x40
#define CPU21      0x00

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

//**************************************************************************
//  Base Device declaration
//**************************************************************************
class vme_fccpu20_device :	public device_t, public device_vme_card_interface
{
public:
	vme_fccpu20_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source, fc_board_t board_id);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_start() override;

	// Below are duplicated declarations from src/mame/drivers/fccpu20.cpp
	DECLARE_READ32_MEMBER (bootvect_r);
	DECLARE_WRITE32_MEMBER (bootvect_w);

	DECLARE_WRITE_LINE_MEMBER(bim_irq_callback);
	uint8_t bim_irq_state;
	int bim_irq_level;

	/* PIT callbacks */
	DECLARE_READ8_MEMBER (pita_r);
	DECLARE_READ8_MEMBER (pitb_r);
	DECLARE_READ8_MEMBER (pitc_r);

private:
	required_device<cpu_device> m_maincpu;
	required_device<pit68230_device> m_pit;
	required_device<bim68153_device> m_bim;
	required_device<mpcc68561_device> m_mpcc;
	required_device<mpcc68561_device> m_mpcc2;
	required_device<mpcc68561_device> m_mpcc3;

	// Pointer to System ROMs needed by bootvect_r and masking RAM buffer for post reset accesses
	uint32_t	*m_sysrom;
	uint32_t	m_sysram[2];
	void 		update_irq_to_maincpu();
	fc_board_t	m_board_id;

	// Below replaces machine_start and machine_reset from  src/mame/drivers/fccpu20.cpp
protected:
	virtual void device_reset() override;
	virtual void device_timer (emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	emu_timer *m_arbiter_start; // Need a startup delay because it is hooked up to the sense inputs of the PIT
};

//**************************************************************************
//  Board Device declarations
//**************************************************************************

class vme_fccpu20_card_device : public vme_fccpu20_device
{
public :
	vme_fccpu20_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	vme_fccpu20_card_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source)
		: vme_fccpu20_device( mconfig, type, name, tag, owner, clock, shortname, source, cpu20) { }
};

class vme_fccpu21s_card_device : public vme_fccpu20_device
{
public :
	vme_fccpu21s_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	vme_fccpu21s_card_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source)
		: vme_fccpu20_device( mconfig, type, name, tag, owner, clock, shortname, source, cpu21s) { }
};

class vme_fccpu21_card_device : public vme_fccpu20_device
{
public :
	vme_fccpu21_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	vme_fccpu21_card_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source)
		: vme_fccpu20_device( mconfig, type, name, tag, owner, clock, shortname, source, cpu21) { }
};

class vme_fccpu21a_card_device : public vme_fccpu20_device
{
public :
	vme_fccpu21a_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	vme_fccpu21a_card_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source)
		: vme_fccpu20_device( mconfig, type, name, tag, owner, clock, shortname, source, cpu21a) { }
};

class vme_fccpu21ya_card_device : public vme_fccpu20_device
{
public :
	vme_fccpu21ya_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	vme_fccpu21ya_card_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source)
		: vme_fccpu20_device( mconfig, type, name, tag, owner, clock, shortname, source, cpu21ya) { }
};

class vme_fccpu21b_card_device : public vme_fccpu20_device
{
public :
	vme_fccpu21b_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	vme_fccpu21b_card_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source)
		: vme_fccpu20_device( mconfig, type, name, tag, owner, clock, shortname, source, cpu21b) { }
};

class vme_fccpu21yb_card_device : public vme_fccpu20_device
{
public :
	vme_fccpu21yb_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	vme_fccpu21yb_card_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source)
		: vme_fccpu20_device( mconfig, type, name, tag, owner, clock, shortname, source, cpu21yb) { }
};


#endif // VME_FCCPU20_H
