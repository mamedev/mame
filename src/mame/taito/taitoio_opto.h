// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_TAITO_TAITOIO_OPTO_H
#define MAME_TAITO_TAITOIO_OPTO_H

#pragma once


class taitoio_opto_device : public device_t
{
public:
	// construction/destruction
	taitoio_opto_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void coin_sense_w(int state);
	int opto_h_r() const { return m_opto_h ? 1 : 0; }
	int opto_l_r() const { return m_opto_l ? 1 : 0; }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

private:
	emu_timer *m_opto_timer;
	bool m_coin_sense;
	bool m_opto_h;
	bool m_opto_l;
	attotime m_opto_start;
	attotime m_opto_end;

	TIMER_CALLBACK_MEMBER(opto_clear_cb);
};


// device type definition
DECLARE_DEVICE_TYPE(TAITOIO_OPTO, taitoio_opto_device)

#endif // MAME_TAITO_TAITOIO_OPTO_H
