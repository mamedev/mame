
#ifndef MAME_APPLE_CAPELLA_H
#define MAME_APPLE_CAPELLA_H

#pragma once


#include "cpu/powerpc/ppc.h"

class capella_device :  public device_t
{
public:
	// construction/destruction
	capella_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// interface routines
	virtual void map(address_map &map) ATTR_COLD;

	template <typename... T> void set_maincpu_tag(T &&... args) { m_maincpu.set_tag(std::forward<T>(args)...); }


    void translate_ipl_state_change(int ipl);

	u64 ctrl_r(offs_t offset);
	void ctrl_w(offs_t offset, u64 data);

	u64 ctrl_b_r(offs_t offset);
	void ctrl_b_w(offs_t offset, u64 data);


    u64 ipl_lines_r(offs_t offset);

    u64 irq_ack_r(offs_t offset);
	void irq_ack_w(offs_t offset, u64 data);

	TIMER_CALLBACK_MEMBER(fire_delayed_irq);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<ppc_device> m_maincpu;

	u8 m_ctrl_reg;
	u8 m_ctrl_reg_b;
	int m_last_pending_irq;
	u8 m_ipl_lines;
};

// device type definition
DECLARE_DEVICE_TYPE(CAPELLA, capella_device)

#endif // MAME_APPLE_CAPELLA_H
