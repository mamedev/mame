// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Cumana 68008 Upgrade Board

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Cumana_680082ndProcessor.html

**********************************************************************/

#ifndef MAME_BUS_BBC_INTERNAL_CUMANA68K_H
#define MAME_BUS_BBC_INTERNAL_CUMANA68K_H

#include "internal.h"
#include "cpu/m68000/m68008.h"
#include "machine/6821pia.h"
#include "machine/input_merger.h"
#include "machine/mc146818.h"
#include "machine/nscsi_cb.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_cumana68k_device :
	public device_t,
	public device_bbc_internal_interface
{
public:
	// construction/destruction
	bbc_cumana68k_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void pia_rtc_pb_w(uint8_t data);
	void pia_sasi_pb_w(uint8_t data);

	uint8_t mem6502_r(offs_t offset);
	void mem6502_w(offs_t offset, uint8_t data);

	static void floppy_formats(format_registration &fr);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset_after_children() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	void irq6502_w(int state) override;

private:
	void reset68008_w(int state);
	void rtc_ce_w(int state);

	required_device<m68000_base_device> m_m68008;
	required_device<pia6821_device> m_pia_sasi;
	required_device<pia6821_device> m_pia_rtc;
	required_device<input_merger_device> m_irqs;
	required_device<mc146818_device> m_rtc;
	required_device<wd2797_device> m_fdc;
	required_device_array<floppy_connector, 4> m_floppy;
	required_device<nscsi_callback_device> m_sasi;

	void cumana68k_mem(address_map &map) ATTR_COLD;

	void fsel_w(offs_t offset, uint8_t data);

	int m_masknmi;

	void mc146818_set(int as, int ds, int rw);
	uint8_t m_mc146818_data;
	int m_mc146818_as;
	int m_mc146818_ds;
	int m_mc146818_rw;
	int m_mc146818_ce;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_CUMANA68K, bbc_cumana68k_device)


#endif // MAME_BUS_BBC_INTERNAL_CUMANA68K_H
