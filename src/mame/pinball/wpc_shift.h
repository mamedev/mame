// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Williams Pinball Controller Shift-based protection simulation

#ifndef MAME_PINBALL_WPC_SHIFT_H
#define MAME_PINBALL_WPC_SHIFT_H

class wpc_shift_device : public device_t
{
public:
	wpc_shift_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~wpc_shift_device();

	void registers(address_map &map) ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	uint16_t adr = 0;
	uint8_t val1 = 0, val2 = 0;

	uint8_t adrh_r();
	void adrh_w(uint8_t data);
	uint8_t adrl_r();
	void adrl_w(uint8_t data);
	uint8_t val1_r();
	void val1_w(uint8_t data);
	uint8_t val2_r();
	void val2_w(uint8_t data);
};

DECLARE_DEVICE_TYPE(WPC_SHIFT, wpc_shift_device)

#endif // MAME_PINBALL_WPC_SHIFT_H
