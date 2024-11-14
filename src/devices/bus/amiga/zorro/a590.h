// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    Commodore A590 / A2091

    DMAC based SCSI controller for the Amiga 500 and Zorro-II

***************************************************************************/

#ifndef MAME_BUS_AMIGA_ZORRO_A590_H
#define MAME_BUS_AMIGA_ZORRO_A590_H

#pragma once

#include "zorro.h"
#include "machine/dmac.h"
#include "machine/wd33c9x.h"


namespace bus::amiga::zorro {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> dmac_hdc_device_base

class dmac_hdc_device_base : public device_t
{
protected:
	// construction/destruction
	dmac_hdc_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// to slot
	virtual void cfgout_w(int state) = 0;
	virtual void int2_w(int state) = 0;
	virtual void int6_w(int state) = 0;

	// should be called when the ram size changes
	void resize_ram(int config);

	// amiga interrupt target, int 2 or 6
	bool m_int6;

	// sub-devices
	required_device<amiga_dmac_device> m_dmac;
	required_device<wd33c93a_device> m_wdc;

	std::vector<uint8_t> m_ram;

private:
	uint8_t dmac_scsi_r(offs_t offset);
	void dmac_scsi_w(offs_t offset, uint8_t data);
	void dmac_int_w(int state);
	void dmac_cfgout_w(int state) { cfgout_w(state); }
	void scsi_irq_w(int state);
	void scsi_drq_w(int state);

	static void scsi_devices(device_slot_interface &device) ATTR_COLD;
	void wd33c93(device_t *device);
};

// ======================> a590_device

class a590_device : public dmac_hdc_device_base, public device_exp_card_interface
{
public:
	// construction/destruction
	a590_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// output to slot
	virtual void cfgout_w(int state) override { m_slot->cfgout_w(state); }
	virtual void int2_w(int state) override { m_slot->int2_w(state); }
	virtual void int6_w(int state) override { m_slot->int6_w(state); }

	// input from slot
	virtual void cfgin_w(int state) override;

private:
	required_ioport m_dips;
	required_ioport m_jp1;
	required_ioport m_jp2;
	required_ioport m_jp4;
};

// ======================> a2091_device

class a2091_device : public dmac_hdc_device_base, public device_zorro2_card_interface
{
public:
	// construction/destruction
	a2091_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// output to slot
	virtual void cfgout_w(int state) override { m_slot->cfgout_w(state); }
	virtual void int2_w(int state) override { m_slot->int2_w(state); }
	virtual void int6_w(int state) override { m_slot->int6_w(state); }

	// input from slot
	virtual void cfgin_w(int state) override;

private:
	required_ioport m_jp1;
	required_ioport m_jp2;
	required_ioport m_jp3;
	required_ioport m_jp5;
	required_ioport m_jp201;
};

} // namespace bus::amiga::zorro

// device type definition
DECLARE_DEVICE_TYPE_NS(ZORRO_A590,  bus::amiga::zorro, a590_device)
DECLARE_DEVICE_TYPE_NS(ZORRO_A2091, bus::amiga::zorro, a2091_device)

#endif // MAME_BUS_AMIGA_ZORRO_A590_H
