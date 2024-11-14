// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_NAMCO_NAMCO54_H
#define MAME_NAMCO_NAMCO54_H

#include "sound/discrete.h"
#include "cpu/mb88xx/mb88xx.h"


class namco_54xx_device : public device_t
{
public:
	namco_54xx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_discrete(T &&tag) { m_discrete.set_tag(std::forward<T>(tag)); }
	void set_basenote(int node) { m_basenode = node; }

	void reset(int state);
	void chip_select(int state);
	void write(uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	// internal state
	required_device<mb88_cpu_device> m_cpu;
	required_device<discrete_device> m_discrete;

	int m_basenode;
	uint8_t m_latched_cmd;

	uint8_t K_r();
	uint8_t R0_r();
	void O_w(uint8_t data);
	void R1_w(uint8_t data);
	TIMER_CALLBACK_MEMBER( write_sync );
};

DECLARE_DEVICE_TYPE(NAMCO_54XX, namco_54xx_device)



/* discrete nodes */
#define NAMCO_54XX_0_DATA(base)     (NODE_RELATIVE(base, 0))
#define NAMCO_54XX_1_DATA(base)     (NODE_RELATIVE(base, 1))
#define NAMCO_54XX_2_DATA(base)     (NODE_RELATIVE(base, 2))
#define NAMCO_54XX_P_DATA(base)     (NODE_RELATIVE(base, 3))


#endif // MAME_NAMCO_NAMCO54_H
