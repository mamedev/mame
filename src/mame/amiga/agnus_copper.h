// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Angelo Salese
/***************************************************************************

    Amiga Copper

***************************************************************************/

#ifndef MAME_AMIGA_AGNUS_COPPER_H
#define MAME_AMIGA_AGNUS_COPPER_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class agnus_copper_device : public device_t
{
public:
	// construction/destruction
	agnus_copper_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// config
	template<class T> void set_host_cpu_tag(T &&tag) { m_host_cpu.set_tag(std::forward<T>(tag)); }
	auto mem_read_cb() { return m_chipmem_r.bind(); }
	void set_ecs_mode(bool ecs_mode) { m_cdang_min_reg = ecs_mode ? 0x00 : 0x20; }

	// I/O operations
	void regs_map(address_map &map) ATTR_COLD;
	void dmacon_set(u16 data);
	void copcon_w(u16 data);

	// getters/setters
	void vblank_sync(bool state);
	int execute_next(int xpos, int ypos, bool is_blitter_busy, int num_planes);
	void suspend_offset(int xpos, int hblank_width);
	int restore_offset();

protected:
	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	//virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_device <cpu_device> m_host_cpu;
	address_space *m_host_space = nullptr;
	// callbacks
	devcb_read16 m_chipmem_r;

	bool m_dma_master_enable, m_dma_copen;
	u16 m_cdang_setting, m_cdang_min_reg;
	u32 m_lc[2], m_pc;

	template <u8 ch> void copxlch_w(u16 data);
	template <u8 ch> void copxlcl_w(u16 data);
	template <u8 ch> void copjmpx_w(u16 data);
	template <u8 ch> u16 copjmpx_r();
public:
	void copins_w(u16 data);
private:
	void set_pc(u8 ch, bool is_sync);

	// internal state
	bool m_state_waiting;
	bool m_state_skipping;
	bool m_state_waitblit;
	u16 m_waitval;
	u16 m_waitmask;
	u16 m_pending_offset;
	u16 m_pending_data;
	u16 m_xpos_state;
	bool m_vertical_blank;
};


// device type definition
DECLARE_DEVICE_TYPE(AGNUS_COPPER, agnus_copper_device)

#endif // MAME_AMIGA_AGNUS_COPPER_H
