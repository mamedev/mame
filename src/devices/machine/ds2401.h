// license:BSD-3-Clause
// copyright-holders:smf
/*
 * DS2401
 *
 * Dallas Semiconductor
 * Silicon Serial Number
 *
 */

#ifndef MAME_MACHINE_DS2401_H
#define MAME_MACHINE_DS2401_H

#pragma once

class ds2401_device : public device_t
{
public:
	// construction/destruction
	ds2401_device(const machine_config &mconfig, const char *tag, device_t *owner)
		: ds2401_device(mconfig, tag, owner, uint32_t(0))
	{
	}

	ds2401_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(int state);
	int read();
	uint8_t direct_read(int index);

protected:
	enum {
		SIZE_DATA = 8,

		COMMAND_READROM = 0x33,
		COMMAND_READROM_COMPAT = 0x0f
	};

	enum {
		STATE_IDLE,
		STATE_RESET,
		STATE_RESET1,
		STATE_RESET2,
		STATE_COMMAND,
		STATE_READROM
	};

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(reset_tick);
	TIMER_CALLBACK_MEMBER(main_tick);

	// internal state
	int m_state, m_bit, m_shift;
	uint8_t m_byte;
	bool m_rx, m_tx;
	uint8_t m_data[SIZE_DATA];
	emu_timer *m_timer_main, *m_timer_reset;
	attotime t_samp, t_rdv, t_rstl, t_pdh, t_pdl;

private:
	inline void verboselog(int n_level, const char *s_fmt, ...) ATTR_PRINTF(3,4);
};


// device type definition
DECLARE_DEVICE_TYPE(DS2401, ds2401_device)

#endif // MAME_MACHINE_DS2401_H
