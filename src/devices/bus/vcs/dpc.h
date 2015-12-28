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
	UINT8   top;
	UINT8   bottom;
	UINT8   low;
	UINT8   high;
	UINT8   flag;
	UINT8   music_mode;     /* Only used by data fetchers 5,6, and 7 */
	UINT8   osc_clk;        /* Only used by data fetchers 5,6, and 7 */
};

//  m_dpc.oscillator = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(a2600_state::modeDPC_timer_callback),this));

class dpc_device : public device_t
{
public:
	// construction/destruction
	dpc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	df_t    m_df[8];
	UINT8   m_movamt;
	UINT8   m_latch_62;
	UINT8   m_latch_64;
	UINT8   m_dlc;
	UINT8   m_shift_reg;
	UINT8   *m_displaydata;
	void set_display_data(UINT8 *data) { m_displaydata = data; }

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual DECLARE_READ8_MEMBER(read);
	virtual DECLARE_WRITE8_MEMBER(write);

private:

	void decrement_counter(UINT8 data_fetcher);
	void check_flag(UINT8 data_fetcher);

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
	a26_rom_dpc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// device-level overrides
	virtual void device_start() override;
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_reset() override;

	required_device<dpc_device> m_dpc;

	// reading and writing
	virtual DECLARE_READ8_MEMBER(read_rom) override;
	virtual DECLARE_WRITE8_MEMBER(write_bank) override;
	virtual DECLARE_DIRECT_UPDATE_MEMBER(cart_opbase) override;

	virtual void setup_addon_ptr(UINT8 *ptr) override;
};


// device type definition
extern const device_type A26_ROM_DPC;

#endif
