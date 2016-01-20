// license:BSD-3-Clause
// copyright-holders:smf
/*
 * DS2401
 *
 * Dallas Semiconductor
 * Silicon Serial Number
 *
 */

#ifndef __DS2401_H__
#define __DS2401_H__

#define MCFG_DS2401_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, DS2401, 0)

#include "emu.h"

class ds2401_device : public device_t
{
public:
	// construction/destruction
	ds2401_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE_LINE_MEMBER( write );
	DECLARE_READ_LINE_MEMBER( read );
	UINT8 direct_read(int index);

protected:
	enum {
		SIZE_DATA = 8,

		COMMAND_READROM = 0x33
	};

	enum {
		STATE_IDLE,
		STATE_RESET,
		STATE_RESET1,
		STATE_RESET2,
		STATE_COMMAND,
		STATE_READROM
	};

	enum {
		TIMER_MAIN,
		TIMER_RESET
	};

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// internal state
	int m_state, m_bit, m_shift;
	UINT8 m_byte;
	bool m_rx, m_tx;
	UINT8 m_data[SIZE_DATA];
	emu_timer *m_timer_main, *m_timer_reset;
	attotime t_samp, t_rdv, t_rstl, t_pdh, t_pdl;

private:
	inline void verboselog(int n_level, const char *s_fmt, ...) ATTR_PRINTF(3,4);
};


// device type definition
extern const device_type DS2401;

#endif
