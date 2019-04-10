// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#ifndef MAME_BUS_VME_VME_HCPU30_H
#define MAME_BUS_VME_VME_HCPU30_H

#pragma once

#include "bus/vme/vme.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "machine/68230pit.h"
#include "machine/scnxx562.h"
#include "machine/terminal.h"
#include "machine/wd33c9x.h"

DECLARE_DEVICE_TYPE(VME_HCPU30, vme_hcpu30_card_device)

class vme_hcpu30_card_device : public device_t, public device_vme_card_interface
{
public:
	vme_hcpu30_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	vme_hcpu30_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<duscc68562_device> m_dusccterm;

	// Pointer to System ROMs needed by bootvect_r and masking RAM buffer for post reset accesses
	uint32_t    *m_sysrom;
	uint32_t    m_sysram[2];

	DECLARE_READ32_MEMBER(bootvect_r);
	DECLARE_WRITE32_MEMBER(bootvect_w);

	void hcpu30_mem(address_map &map);
};

#endif // MAME_BUS_VME_VME_HCPU30_H
