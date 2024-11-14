// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/******************************************************************************

    Psion ASIC1

******************************************************************************/

#ifndef MAME_MACHINE_PSION_ASIC1_H
#define MAME_MACHINE_PSION_ASIC1_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> psion_asic1_device

class psion_asic1_device : public device_t,
	public device_memory_interface,
	public device_video_interface
{
public:
	// construction/destruction
	psion_asic1_device(const machine_config &mconfig, const char* tag, device_t* owner, uint32_t clock);

	void set_laptop_mode(bool laptop) { m_laptop_mode = laptop; }

	// callbacks
	auto int_cb() { return m_int_cb.bind(); }
	auto nmi_cb() { return m_nmi_cb.bind(); }
	auto frcovl_cb() { return m_frcovl_cb.bind(); }

	uint16_t mem_r(offs_t offset, uint16_t mem_mask);
	void mem_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t io_r(offs_t offset, uint16_t mem_mask);
	void io_w(offs_t offset, uint16_t data, uint16_t mem_mask);

	IRQ_CALLBACK_MEMBER(inta_cb);

	void eint1_w(int state);
	void eint2_w(int state);
	void eint3_w(int state);
	void enmi_w(int state);

	uint32_t screen_update_single(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_dual(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;

private:
	const address_space_config m_space_config;
	address_space *m_space;

	emu_timer *m_tick_timer;
	emu_timer *m_frc_timer;
	emu_timer *m_watchdog_timer;

	TIMER_CALLBACK_MEMBER(tick);
	TIMER_CALLBACK_MEMBER(frc);
	TIMER_CALLBACK_MEMBER(watchdog);

	void update_interrupts(bool address_trap = false);
	bool is_protected(offs_t offset);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int plates);

	uint16_t m_a1_status;
	uint16_t m_a1_lcd_size;
	uint16_t m_a1_lcd_control;
	uint16_t m_frc_count;
	uint16_t m_frc_reload;
	int m_frc_ovl;
	uint8_t m_watchdog_count;
	bool m_a1_protection_mode;
	uint32_t m_a1_protection_upper;
	uint32_t m_a1_protection_lower;

	uint8_t m_a1_interrupt_status;
	uint8_t m_a1_interrupt_mask;

	devcb_write_line m_int_cb;
	devcb_write_line m_nmi_cb;
	devcb_write_line m_frcovl_cb;

	bool m_laptop_mode;
	uint8_t lcd_type();
};


// device type definition
DECLARE_DEVICE_TYPE(PSION_ASIC1, psion_asic1_device)

#endif // MAME_MACHINE_PSION_ASIC1_H
