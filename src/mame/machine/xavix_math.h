// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_MACHINE_XAVIX_MATH_H
#define MAME_MACHINE_XAVIX_MATH_H

class xavix_math_device : public device_t
{
public:
	xavix_math_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER(mult_r);
	DECLARE_WRITE8_MEMBER(mult_w);
	DECLARE_READ8_MEMBER(mult_param_r);
	DECLARE_WRITE8_MEMBER(mult_param_w);

	DECLARE_READ8_MEMBER(barrel_r);
	DECLARE_WRITE8_MEMBER(barrel_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:

	uint8_t m_barrel_params[2];
	uint8_t m_multparams[3];
	uint8_t m_multresults[2];
};

DECLARE_DEVICE_TYPE(XAVIX_MATH, xavix_math_device)

#endif // MAME_MACHINE_XAVIX_MATH_H
