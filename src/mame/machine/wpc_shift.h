// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Williams Pinball Controller Shift-based protection simulation

#ifndef MAME_MACHINE_WPC_SHIFT_H
#define MAME_MACHINE_WPC_SHIFT_H

class wpc_shift_device : public device_t
{
public:
	wpc_shift_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~wpc_shift_device();

	void registers(address_map &map);

	DECLARE_READ8_MEMBER(adrh_r);
	DECLARE_WRITE8_MEMBER(adrh_w);
	DECLARE_READ8_MEMBER(adrl_r);
	DECLARE_WRITE8_MEMBER(adrl_w);
	DECLARE_READ8_MEMBER(val1_r);
	DECLARE_WRITE8_MEMBER(val1_w);
	DECLARE_READ8_MEMBER(val2_r);
	DECLARE_WRITE8_MEMBER(val2_w);

protected:
	uint16_t adr;
	uint8_t val1, val2;

	virtual void device_start() override;
	virtual void device_reset() override;
};

DECLARE_DEVICE_TYPE(WPC_SHIFT, wpc_shift_device)

#endif // MAME_MACHINE_WPC_SHIFT_H
