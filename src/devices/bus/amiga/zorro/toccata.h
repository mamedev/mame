// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    MacroSystem Toccata

    16bit/48KHz Audio Digitizer

***************************************************************************/

#ifndef MAME_BUS_AMIGA_ZORRO_TOCCATA_H
#define MAME_BUS_AMIGA_ZORRO_TOCCATA_H

#pragma once

#include "zorro.h"
#include "machine/7200fifo.h"
#include "machine/autoconfig.h"
#include "sound/ad1848.h"


namespace bus::amiga::zorro {

class toccata_device : public device_t, public device_zorro2_card_interface, public amiga_autoconfig
{
public:
	toccata_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void device_start() override ATTR_COLD;

	// device_zorro2_card_interface overrides
	virtual void cfgin_w(int state) override;

	// amiga_autoconfig overrides
	virtual void autoconfig_base_address(offs_t address) override;

private:
	void mmio_map(address_map &map) ATTR_COLD;

	void update_interrupts();

	void playback_hf_w(int state);
	void record_hf_w(int state);
	void drq_w(int state);

	uint8_t status_r(offs_t offset);
	void control_w(offs_t offset, uint8_t data);

	uint16_t fifo_r(offs_t offset, uint16_t mem_mask = ~0);
	void fifo_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint8_t ad1848_idx_r(offs_t offset);
	void ad1848_idx_w(offs_t offset, uint8_t data);
	uint8_t ad1848_data_r(offs_t offset);
	void ad1848_data_w(offs_t offset, uint8_t data);

	required_device<ad1848_device> m_ad1848;
	required_device_array<idt7202_device, 2> m_fifo;

	uint8_t m_status = 0;
	uint8_t m_control = 0;
};

} // namespace bus::amiga::zorro

// device type declaration
DECLARE_DEVICE_TYPE_NS(AMIGA_TOCCATA, bus::amiga::zorro, toccata_device)

#endif // MAME_BUS_AMIGA_ZORRO_TOCCATA_H
