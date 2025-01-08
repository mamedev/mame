// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef MAME_BUS_VCS_DPC_H
#define MAME_BUS_VCS_DPC_H

#pragma once

#include "rom.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// TODO: DPC should be made a separate device

class dpc_device : public device_t
{
public:
	// construction/destruction
	dpc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void set_display_data(uint8_t *data) { m_displaydata = data; }

	virtual uint8_t read(offs_t offset);
	virtual void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(oscillator_tick);

	struct df_t {
		uint8_t   top;
		uint8_t   bottom;
		uint8_t   low;
		uint8_t   high;
		uint8_t   flag;
		uint8_t   music_mode;     /* Only used by data fetchers 5,6, and 7 */
		uint8_t   osc_clk;        /* Only used by data fetchers 5,6, and 7 */
	};

	df_t    m_df[8];
	uint8_t   m_movamt;
	uint8_t   m_latch_62;
	uint8_t   m_latch_64;
	uint8_t   m_dlc;
	uint8_t   m_shift_reg;
	uint8_t   *m_displaydata;

private:
	void decrement_counter(uint8_t data_fetcher);
	void check_flag(uint8_t data_fetcher);

	emu_timer *m_oscillator;
};


// device type definition
DECLARE_DEVICE_TYPE(ATARI_DPC, dpc_device)



// ======================> a26_rom_dpc_device

class a26_rom_dpc_device : public a26_rom_f8_device
{
public:
	a26_rom_dpc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	required_device<dpc_device> m_dpc;

	virtual void install_memory_handlers(address_space *space) override;

	virtual void setup_addon_ptr(uint8_t *ptr) override;

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual uint8_t get_start_bank() override { return 0; }
};


// device type definition
DECLARE_DEVICE_TYPE(A26_ROM_DPC, a26_rom_dpc_device)

#endif // MAME_BUS_VCS_DPC_H
