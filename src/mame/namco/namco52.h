// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_NAMCO_NAMCO52_H
#define MAME_NAMCO_NAMCO52_H

#include "sound/discrete.h"
#include "cpu/mb88xx/mb88xx.h"

class namco_52xx_device : public device_t
{
public:
	namco_52xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_discrete(T &&tag) { m_discrete.set_tag(std::forward<T>(tag)); }
	void set_basenote(int node) { m_basenode = node; }
	void set_extclock(attoseconds_t clk) { m_extclock = clk; }
	auto romread_callback() { return m_romread.bind(); }
	auto si_callback() { return m_si.bind(); }

	void reset(int state);
	void chip_select(int state);
	void write(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	TIMER_CALLBACK_MEMBER( write_sync );
	TIMER_CALLBACK_MEMBER( external_clock_pulse );

private:
	// internal state
	required_device<mb88_cpu_device> m_cpu;
	required_device<discrete_device> m_discrete;

	int m_basenode;
	attoseconds_t m_extclock;
	emu_timer *m_extclock_pulse_timer;
	devcb_read8 m_romread;
	devcb_read8 m_si;

	uint8_t m_latched_cmd;
	uint32_t m_address;

	uint8_t K_r();
	int SI_r();
	uint8_t R0_r();
	uint8_t R1_r();
	void P_w(uint8_t data);
	void R2_w(uint8_t data);
	void R3_w(uint8_t data);
	void O_w(uint8_t data);
};

DECLARE_DEVICE_TYPE(NAMCO_52XX, namco_52xx_device)



/* discrete nodes */
#define NAMCO_52XX_P_DATA(base)     (base)


#endif  // MAME_NAMCO_NAMCO52_H
