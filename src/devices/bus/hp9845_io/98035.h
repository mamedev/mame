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

#define HP98035_IBUFFER_LEN	16	// Totally arbitrary
#define HP98035_OBUFFER_LEN	16	// Totally arbitrary
#define HP98035_UNIT_COUNT	4	// Count of counter/timer units

class hp98035_io_card : public hp9845_io_card_device
{
public:
	// construction/destruction
	hp98035_io_card(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~hp98035_io_card();

	// device-level overrides
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	DECLARE_READ16_MEMBER(reg_r);
	DECLARE_WRITE16_MEMBER(reg_w);

private:
	// Interface state
	bool m_flg;
	bool m_inten;
	bool m_intflag;
	bool m_irq;
	bool m_idr_full;
	UINT8 m_idr;	// Input Data Register
	UINT8 m_odr;	// Output Data Register
	UINT8 m_error;
	UINT8 m_triggered;
	UINT8 m_lost_irq;
	UINT8 m_ibuffer[ HP98035_IBUFFER_LEN + 1 ];
	unsigned m_ibuffer_ptr;
	UINT8 m_obuffer[ HP98035_OBUFFER_LEN ];
	unsigned m_obuffer_len;
	unsigned m_obuffer_ptr;

	// Clock/timer state
	unsigned m_msec;	// Milliseconds
	UINT8 m_sec;	// Seconds
	UINT8 m_min;	// Minutes
	UINT8 m_hrs;	// Hours
	UINT8 m_dom;	// Day of month
	UINT8 m_mon;	// Month
	// Strangely enough this RTC has no notion of current year
	emu_timer *m_msec_timer;

	// Timer units
	typedef enum {
		UNIT_IDLE,	// Not active
		UNIT_ACTIVE,	// Active (output units: waiting for date/time match)
		UNIT_WAIT_FOR_TO	// Active, output units only: waiting for timeout
	} unit_state_t;

	typedef struct {
		unit_state_t m_state;	// State
		bool m_input;	// Input or output
		UINT8 m_port;	// Assigned port # (0 if not assigned)
		UINT8 m_match_datetime[ 4 ];	// Date&time to match (month is not included)
		unsigned m_delay;	// Timer delay
		unsigned m_period;	// Timer period (when != 0)
		unsigned m_value;	// Current counter value

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
	bool assign_unit(timer_unit_t& unit , const UINT8*& p , bool input);
	bool parse_unit_command(const UINT8*& p, unsigned unit_no);
	void clear_obuffer(void);
	void set_obuffer(UINT8 b);
	void set_obuffer(const char* s);
	void update_obuffer(void);
	void set_error(UINT8 mask);
	bool parse_datetime(const UINT8*& p, UINT8 *out) const;
	bool parse_unit_no(const UINT8*& p, unsigned& unit) const;
	bool parse_msec(const UINT8*& p, unsigned& msec) const;
};

// device type definition
extern const device_type HP98035_IO_CARD;

#endif /* _98035_H_ */
