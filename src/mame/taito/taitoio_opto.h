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

	int opto_h_r();
	int opto_l_r();
	DECLARE_INPUT_CHANGED_MEMBER(coin_sense_cb);

protected:
	virtual ioport_constructor device_input_ports() const override;

	// device-level overrides
//  virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	required_ioport m_coin_in;

	bool m_opto_h = false;
	bool m_opto_l = false;
	emu_timer *m_opto_timer = nullptr;
	attotime m_opto_start = attotime::never;
	attotime m_opto_end = attotime::never;
	TIMER_CALLBACK_MEMBER(opto_clear_cb);
};


// device type definition
DECLARE_DEVICE_TYPE(TAITOIO_OPTO, taitoio_opto_device)

#endif // MAME_TAITO_TAITOIO_OPTO_H
