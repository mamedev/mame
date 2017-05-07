// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_AUDIO_NAMCO54_H
#define MAME_AUDIO_NAMCO54_H

#include "sound/discrete.h"
#include "cpu/mb88xx/mb88xx.h"


#define MCFG_NAMCO_54XX_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, NAMCO_54XX, _clock)

#define MCFG_NAMCO_54XX_DISCRETE(_tag) \
	namco_54xx_device::set_discrete(*device, "^" _tag);

#define MCFG_NAMCO_54XX_BASENODE(_node) \
	namco_54xx_device::set_basenote(*device, _node);


class namco_54xx_device : public device_t
{
public:
	namco_54xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static void set_discrete(device_t &device, const char *tag) { downcast<namco_54xx_device &>(device).m_discrete.set_tag(tag); }
	static void set_basenote(device_t &device, int node) { downcast<namco_54xx_device &>(device).m_basenode = node; }

	DECLARE_READ8_MEMBER( K_r );
	DECLARE_READ8_MEMBER( R0_r );
	DECLARE_WRITE8_MEMBER( O_w );
	DECLARE_WRITE8_MEMBER( R1_w );

	DECLARE_WRITE8_MEMBER( write );
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	TIMER_CALLBACK_MEMBER( latch_callback );
	TIMER_CALLBACK_MEMBER( irq_clear );
private:
	// internal state
	required_device<mb88_cpu_device> m_cpu;
	required_device<discrete_device> m_discrete;

	int m_basenode;
	uint8_t m_latched_cmd;
};

DECLARE_DEVICE_TYPE(NAMCO_54XX, namco_54xx_device)



/* discrete nodes */
#define NAMCO_54XX_0_DATA(base)     (NODE_RELATIVE(base, 0))
#define NAMCO_54XX_1_DATA(base)     (NODE_RELATIVE(base, 1))
#define NAMCO_54XX_2_DATA(base)     (NODE_RELATIVE(base, 2))
#define NAMCO_54XX_P_DATA(base)     (NODE_RELATIVE(base, 3))


#endif // MAME_AUDIO_NAMCO54_H
