// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Atari Jaguar "Blitter" device

***************************************************************************/

#ifndef MAME_ATARI_JAG_BLITTER_H
#define MAME_ATARI_JAG_BLITTER_H

#pragma once



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> jag_blitter_device

class jag_blitter_device : public device_t,
						   public device_memory_interface
{
public:
	// construction/destruction
	jag_blitter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// TODO: add which device triggered the I/O
	void iobus_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 iobus_r(offs_t offset, u32 mem_mask = ~0);

protected:
	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

	const address_space_config m_space_config;

//  address_space *m_host_space;

private:
	void regs_map(address_map &map) ATTR_COLD;

	void a1_base_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void a1_xstep_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void a1_ystep_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u32 status_r();
	void command_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void count_outer_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void count_inner_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	// timer setups
	emu_timer *m_command_timer = nullptr;
	TIMER_CALLBACK_MEMBER(command_run);
	inline void command_start();
	inline void command_done();

	// functional switches

	typedef void (jag_blitter_device::*op_func)(void);

	static const op_func upda_ops[8];
	void op_nop();
	void op_unemulated();
	void op_upda1();

	u32 m_command_latch;
	bool m_status_idle;
	u32 m_count_lines, m_count_pixels;

	struct {
		u32 base;
		s16 xstep, ystep;
		u32 ptr = 0; /**< Current pixel address */
	} m_a1;
};


// device type definition
DECLARE_DEVICE_TYPE(JAG_BLITTER, jag_blitter_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


#endif // MAME_ATARI_JAG_BLITTER_H
