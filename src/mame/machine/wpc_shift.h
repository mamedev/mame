// license:BSD-3-Clause
// copyright-holders:Olivier Galibert

// Williams Pinball Controller Shift-based protection simulation

#ifndef WPC_SHIFT_H
#define WPC_SHIFT_H

#define MCFG_WPC_SHIFT_ADD( _tag ) \
	MCFG_DEVICE_ADD( _tag, WPC_SHIFT, 0 )

class wpc_shift_device : public device_t
{
public:
	wpc_shift_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual ~wpc_shift_device();

	DECLARE_ADDRESS_MAP(registers, 8);

	uint8_t adrh_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void adrh_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t adrl_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void adrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t val1_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void val1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t val2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void val2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	uint16_t adr;
	uint8_t val1, val2;

	virtual void device_start() override;
	virtual void device_reset() override;
};

extern const device_type WPC_SHIFT;

#endif
