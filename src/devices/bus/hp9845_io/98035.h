// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    98035.h

    98035 module (Real time clock)

*********************************************************************/

#ifndef MAME_BUS_HP_9845_IO_98035_H
#define MAME_BUS_HP_9845_IO_98035_H

#pragma once

#include "hp9845_io.h"
#include "cpu/nanoprocessor/nanoprocessor.h"
#include "dirtc.h"

class hp98035_io_card_device : public device_t, public device_hp9845_io_interface, public device_rtc_interface
{
public:
	// construction/destruction
	hp98035_io_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp98035_io_card_device();

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	// device-level overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual uint16_t reg_r(address_space &space, offs_t offset) override;
	virtual void reg_w(address_space &space, offs_t offset, uint16_t data) override;

private:
	void dc_w(uint8_t data);

	void ram_addr_w(uint8_t data);
	uint8_t ram_data_r();
	void ram_addr_data_w(uint8_t data);
	void ram_data_w(uint8_t data);

	void clock_key_w(uint8_t data);
	uint8_t clock_digit_r();

	void odr_w(uint8_t data);
	uint8_t idr_r();
	uint8_t np_status_r();
	void clear_np_irq_w(uint8_t data);
	uint8_t clock_mux_r();
	void set_irq_w(uint8_t data);
	uint8_t clr_inten_r();
	void clr_inten_w(uint8_t data);

	void np_io_map(address_map &map);
	void np_program_map(address_map &map);

	required_device<hp_nanoprocessor_device> m_cpu;

	// Internal RAM & I/F
	uint8_t m_np_ram[ 256 ]{};
	uint8_t m_ram_addr = 0;
	uint8_t m_ram_data_in = 0;

	// DC lines
	uint8_t m_dc = 0;

	// NP interrupt
	bool m_np_irq = false;

	// Periodic interrupt
	emu_timer *m_msec_timer = nullptr;

	// Interface state
	bool m_flg = false;
	bool m_inten = false;
	bool m_intflag = false;
	bool m_irq = false;
	bool m_idr_full = false;
	uint8_t m_idr = 0;  // Input Data Register
	uint8_t m_odr = 0;  // Output Data Register

	// Clock chip emulation
	typedef enum {
		CLOCK_OFF,  // Display OFF
		CLOCK_HHMM, // Show HH:mm
		CLOCK_SS,   // Show   :SS
		CLOCK_HH,   // Show HH: A/P
		CLOCK_MIN,  // Show   :mm
		CLOCK_MON,  // Show MM:
		CLOCK_DOM,  // Show   :DD
	} clock_state_t;

	emu_timer *m_clock_timer = nullptr;
	unsigned m_clock_1s_div = 0;
	clock_state_t m_clock_state{};
	uint8_t m_clock_digits[ 3 ]{};
	uint8_t m_clock_mux = 0;
	bool m_clock_segh = false;
	uint8_t m_clock_keys = 0;
	uint8_t m_prev_clock_keys = 0;
	unsigned m_clock_key_cnt = 0;

	void half_init();
	void set_flg(bool value);
	void update_irq();
	void update_dc();

	void set_lhs_digits(unsigned v);
	void set_rhs_digits(unsigned v);
	void regen_clock_image();
	void clock_short_press();
	void clock_long_press();
	void log_current_time();
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;
};

// device type definition
DECLARE_DEVICE_TYPE(HP98035_IO_CARD, hp98035_io_card_device)

#endif // MAME_BUS_HP_9845_IO_98035_H
