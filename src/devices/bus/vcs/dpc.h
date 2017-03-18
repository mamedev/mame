// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
#ifndef __VCS_DCP_H
#define __VCS_DCP_H

#include "rom.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// TO DO: DPC should be made a separate device!

struct df_t {
	uint8_t   top;
	uint8_t   bottom;
	uint8_t   low;
	uint8_t   high;
	uint8_t   flag;
	uint8_t   music_mode;     /* Only used by data fetchers 5,6, and 7 */
	uint8_t   osc_clk;        /* Only used by data fetchers 5,6, and 7 */
};

//  m_dpc.oscillator = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(a2600_state::modeDPC_timer_callback),this));

class dpc_device : public device_t
{
public:
	// construction/destruction
	dpc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	df_t    m_df[8];
	uint8_t   m_movamt;
	uint8_t   m_latch_62;
	uint8_t   m_latch_64;
	uint8_t   m_dlc;
	uint8_t   m_shift_reg;
	uint8_t   *m_displaydata;
	void set_display_data(uint8_t *data) { m_displaydata = data; }

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual DECLARE_READ8_MEMBER(read);
	virtual DECLARE_WRITE8_MEMBER(write);

private:

	void decrement_counter(uint8_t data_fetcher);
	void check_flag(uint8_t data_fetcher);

	static const device_timer_id TIMER_OSC = 0;
	emu_timer *m_oscillator;
};


// device type definition
extern const device_type ATARI_DPC;



// ======================> a26_rom_dpc_device

class a26_rom_dpc_device : public a26_rom_f8_device
{
public:
	// construction/destruction
	a26_rom_dpc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_reset() override;

	required_device<dpc_device> m_dpc;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom) override;
	virtual DECLARE_WRITE8_MEMBER(write_bank) override;

	virtual void setup_addon_ptr(uint8_t *ptr) override;
};


// device type definition
extern const device_type A26_ROM_DPC;

#endif
