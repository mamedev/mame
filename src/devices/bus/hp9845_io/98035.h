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

#define HP98035_IBUFFER_LEN 16  // Totally arbitrary
#define HP98035_OBUFFER_LEN 16  // Totally arbitrary
#define HP98035_UNIT_COUNT  4   // Count of counter/timer units

class hp98035_io_card : public hp9845_io_card_device
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

	uint16_t reg_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void reg_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

private:
	// Interface state
	bool m_flg;
	bool m_inten;
	bool m_intflag;
	bool m_irq;
	bool m_idr_full;
	uint8_t m_idr;  // Input Data Register
	uint8_t m_odr;  // Output Data Register
	uint8_t m_error;
	uint8_t m_triggered;
	uint8_t m_lost_irq;
	uint8_t m_ibuffer[ HP98035_IBUFFER_LEN + 1 ];
	unsigned m_ibuffer_ptr;
	uint8_t m_obuffer[ HP98035_OBUFFER_LEN ];
	unsigned m_obuffer_len;
	unsigned m_obuffer_ptr;

	// Clock/timer state
	unsigned m_msec;    // Milliseconds
	uint8_t m_sec;  // Seconds
	uint8_t m_min;  // Minutes
	uint8_t m_hrs;  // Hours
	uint8_t m_dom;  // Day of month
	uint8_t m_mon;  // Month
	// Strangely enough this RTC has no notion of current year
	emu_timer *m_msec_timer;

	// Timer units
	typedef enum {
		UNIT_IDLE,  // Not active
		UNIT_ACTIVE,    // Active (output units: waiting for date/time match)
		UNIT_WAIT_FOR_TO    // Active, output units only: waiting for timeout
	} unit_state_t;

	typedef struct {
		unit_state_t m_state;   // State
		bool m_input;   // Input or output
		uint8_t m_port; // Assigned port # (0 if not assigned)
		uint8_t m_match_datetime[ 4 ];  // Date&time to match (month is not included)
		unsigned m_delay;   // Timer delay
		unsigned m_period;  // Timer period (when != 0)
		unsigned m_value;   // Current counter value

		void init(void);
		void deactivate(void);
		void adv_state(bool reset = false);
	} timer_unit_t;

	timer_unit_t m_units[ HP98035_UNIT_COUNT ];

	void half_init(void);
	void set_flg(bool value);
	void update_irq(void);
	void update_ibuffer(void);
	void process_ibuffer(void);
	bool assign_unit(timer_unit_t& unit , const uint8_t*& p , bool input);
	bool parse_unit_command(const uint8_t*& p, unsigned unit_no);
	void clear_obuffer(void);
	void set_obuffer(uint8_t b);
	void set_obuffer(const char* s);
	void update_obuffer(void);
	void set_error(uint8_t mask);
	bool parse_datetime(const uint8_t*& p, uint8_t *out) const;
	bool parse_unit_no(const uint8_t*& p, unsigned& unit) const;
	bool parse_msec(const uint8_t*& p, unsigned& msec) const;
};

// device type definition
extern const device_type HP98035_IO_CARD;

#endif /* _98035_H_ */
