// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef NAMCO52_H
#define NAMCO52_H

#include "sound/discrete.h"
#include "cpu/mb88xx/mb88xx.h"

#define MCFG_NAMCO_52XX_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, NAMCO_52XX, _clock)

#define MCFG_NAMCO_52XX_DISCRETE(_tag) \
	namco_52xx_device::set_discrete(*device, "^" _tag);

#define MCFG_NAMCO_52XX_BASENODE(_node) \
	namco_52xx_device::set_basenote(*device, _node);

#define MCFG_NAMCO_52XX_EXT_CLOCK(_clock) \
	namco_52xx_device::set_extclock(*device, _clock);

#define MCFG_NAMCO_52XX_ROMREAD_CB(_devcb) \
	devcb = &namco_52xx_device::set_romread_callback(*device, DEVCB_##_devcb);

#define MCFG_NAMCO_52XX_SI_CB(_devcb) \
	devcb = &namco_52xx_device::set_si_callback(*device, DEVCB_##_devcb);


class namco_52xx_device : public device_t
{
public:
	namco_52xx_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void set_discrete(device_t &device, const char *tag) { downcast<namco_52xx_device &>(device).m_discrete.set_tag(tag); }
	static void set_basenote(device_t &device, int node) { downcast<namco_52xx_device &>(device).m_basenode = node; }
	static void set_extclock(device_t &device, attoseconds_t clk) { downcast<namco_52xx_device &>(device).m_extclock = clk; }
	template<class _Object> static devcb_base &set_romread_callback(device_t &device, _Object object) { return downcast<namco_52xx_device &>(device).m_romread.set_callback(object); }
	template<class _Object> static devcb_base &set_si_callback(device_t &device, _Object object) { return downcast<namco_52xx_device &>(device).m_si.set_callback(object); }

	DECLARE_WRITE8_MEMBER(write);

	DECLARE_READ8_MEMBER( K_r );
	DECLARE_READ8_MEMBER( SI_r );
	DECLARE_READ8_MEMBER( R0_r );
	DECLARE_READ8_MEMBER( R1_r );
	DECLARE_WRITE8_MEMBER( P_w );
	DECLARE_WRITE8_MEMBER( R2_w );
	DECLARE_WRITE8_MEMBER( R3_w );
	DECLARE_WRITE8_MEMBER( O_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	TIMER_CALLBACK_MEMBER( latch_callback );
	TIMER_CALLBACK_MEMBER( irq_clear );
	TIMER_CALLBACK_MEMBER( external_clock_pulse );
private:
	// internal state
	required_device<mb88_cpu_device> m_cpu;
	required_device<discrete_device> m_discrete;

	int m_basenode;
	attoseconds_t m_extclock;
	devcb_read8 m_romread;
	devcb_read8 m_si;

	UINT8 m_latched_cmd;
	UINT32 m_address;
};

extern const device_type NAMCO_52XX;



/* discrete nodes */
#define NAMCO_52XX_P_DATA(base)     (base)


#endif  /* NAMCO52_H */
