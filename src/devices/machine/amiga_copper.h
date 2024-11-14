// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Angelo Salese
/***************************************************************************

    Amiga Copper

***************************************************************************/

#ifndef MAME_MACHINE_AMIGA_COPPER_H
#define MAME_MACHINE_AMIGA_COPPER_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class amiga_copper_device : public device_t
{
public:
	// construction/destruction
	amiga_copper_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// config
	template<class T> void set_host_cpu_tag(T &&tag) { m_host_cpu.set_tag(std::forward<T>(tag)); }
	auto mem_read_cb() { return m_chipmem_r.bind(); }
	void set_ecs_mode(bool ecs_mode) { m_cdang_min_reg = ecs_mode ? 0x00 : 0x20; }

	// I/O operations
	void regs_map(address_map &map) ATTR_COLD;
	void dmacon_set(u16 data);
	void copcon_w(u16 data);

	// getters/setters
	void vblank_sync();
	int execute_next(int xpos, int ypos, bool is_blitter_busy);

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
	bool m_state_waitblit;
	u16 m_waitval;
	u16 m_waitmask;
	u16 m_pending_offset;
	u16 m_pending_data;
//  int m_wait_offset;

	// waitstate delays for copper
	// basically anything that doesn't belong to Angus has a penalty for Copper
	static constexpr u16 delay[256] =
	{
		1,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,    /* 0x000 - 0x03e */
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                        /* 0x040 - 0x05e */
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                        /* 0x060 - 0x07e */
		0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0,                        /* 0x080 - 0x09e */
		1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,    /* 0x0a0 - 0x0de */
		/* BPLxPTH/BPLxPTL */
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                        /* 0x0e0 - 0x0fe */
		/* BPLCON0-3,BPLMOD1-2 */
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                        /* 0x100 - 0x11e */
		/* SPRxPTH/SPRxPTL */
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,                        /* 0x120 - 0x13e */
		/* SPRxPOS/SPRxCTL/SPRxDATA/SPRxDATB */
		1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,    /* 0x140 - 0x17e */
		/* COLORxx */
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    /* 0x180 - 0x1be */
		/* RESERVED */
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 /* 0x1c0 - 0x1fe */
	};

};


// device type definition
DECLARE_DEVICE_TYPE(AMIGA_COPPER, amiga_copper_device)

#endif // MAME_MACHINE_AMIGA_COPPER_H
