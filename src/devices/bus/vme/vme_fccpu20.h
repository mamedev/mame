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

class vme_fccpu20_card_device :
	public device_t
	,public device_vme_card_interface
{
public:
	vme_fccpu20_card_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	vme_fccpu20_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

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
	uint32_t  *m_sysrom;
	uint32_t  m_sysram[2];
	void update_irq_to_maincpu();

	// Below replaces machine_start and machine_reset from  src/mame/drivers/fccpu20.cpp
protected:
	virtual void device_start() override;
	virtual void device_reset() override;
};

#endif // VME_FCCPU20_H
