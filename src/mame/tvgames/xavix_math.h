// license:BSD-3-Clause
// copyright-holders:David Haywood

#ifndef MAME_TVGAMES_XAVIX_MATH_H
#define MAME_TVGAMES_XAVIX_MATH_H

class xavix_math_device : public device_t
{
public:
	xavix_math_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t mult_r(offs_t offset);
	void mult_w(offs_t offset, uint8_t data);
	uint8_t mult_param_r(offs_t offset);
	void mult_param_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);

	uint8_t barrel_r(offs_t offset);
	void barrel_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:

	uint8_t m_barrel_params[2];
	uint8_t m_multparams[3];
	uint8_t m_multresults[2];
};

DECLARE_DEVICE_TYPE(XAVIX_MATH, xavix_math_device)

#endif // MAME_TVGAMES_XAVIX_MATH_H
