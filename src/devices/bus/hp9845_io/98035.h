// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    98035.h

    98035 module (Real time clock)

*********************************************************************/

#pragma once

#ifndef _98035_H_
#define _98035_H_

#include "hp9845_io.h"
#include "cpu/nanoprocessor/nanoprocessor.h"
#include "dirtc.h"

class hp98035_io_card : public hp9845_io_card_device,
						public device_rtc_interface
{
public:
	// construction/destruction
	hp98035_io_card(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~hp98035_io_card();

	// device-level overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual DECLARE_READ16_MEMBER(reg_r) override;
	virtual DECLARE_WRITE16_MEMBER(reg_w) override;

	DECLARE_WRITE8_MEMBER(ram_addr_w);
	DECLARE_READ8_MEMBER(ram_data_r);
	DECLARE_WRITE8_MEMBER(ram_addr_data_w);
	DECLARE_WRITE8_MEMBER(ram_data_w);

	DECLARE_WRITE8_MEMBER(clock_key_w);
	DECLARE_READ8_MEMBER(clock_digit_r);

	DECLARE_WRITE8_MEMBER(odr_w);
	DECLARE_READ8_MEMBER(idr_r);
	DECLARE_READ8_MEMBER(np_status_r);
	DECLARE_WRITE8_MEMBER(clear_np_irq_w);
	DECLARE_READ8_MEMBER(clock_mux_r);
	DECLARE_WRITE8_MEMBER(set_irq_w);
	DECLARE_READ8_MEMBER(clr_inten_r);
	DECLARE_WRITE8_MEMBER(clr_inten_w);

	DECLARE_WRITE8_MEMBER(dc_w);

private:
	required_device<hp_nanoprocessor_device> m_cpu;

	// Internal RAM & I/F
	uint8_t m_np_ram[ 256 ];
	uint8_t m_ram_addr;
	uint8_t m_ram_data_in;

	// DC lines
	uint8_t m_dc;

	// NP interrupt
	bool m_np_irq;

	// Periodic interrupt
	emu_timer *m_msec_timer;

	// Interface state
	bool m_flg;
	bool m_inten;
	bool m_intflag;
	bool m_irq;
	bool m_idr_full;
	uint8_t m_idr;  // Input Data Register
	uint8_t m_odr;  // Output Data Register

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

	emu_timer *m_clock_timer;
	unsigned m_clock_1s_div;
	clock_state_t m_clock_state;
	uint8_t m_clock_digits[ 3 ];
	uint8_t m_clock_mux;
	bool m_clock_segh;
	uint8_t m_clock_keys;
	uint8_t m_prev_clock_keys;
	unsigned m_clock_key_cnt;

	void half_init(void);
	void set_flg(bool value);
	void update_irq(void);
	void update_dc(void);

	void set_lhs_digits(unsigned v);
	void set_rhs_digits(unsigned v);
	void regen_clock_image(void);
	void clock_short_press(void);
	void clock_long_press(void);
	void log_current_time(void);
	virtual void rtc_clock_updated(int year, int month, int day, int day_of_week, int hour, int minute, int second) override;
};

// device type definition
extern const device_type HP98035_IO_CARD;

#endif /* _98035_H_ */
