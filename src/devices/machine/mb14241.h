// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/*****************************************************************************

    MB14241 shifter IC emulation

 *****************************************************************************/

#ifndef __MB14241_H__
#define __MB14241_H__


class mb14241_device : public device_t
{
public:
	mb14241_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void shift_count_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void shift_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t shift_result_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state

	uint16_t m_shift_data;  /* 15 bits only */
	uint8_t m_shift_count;  /* 3 bits */
};

extern const device_type MB14241;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_MB14241_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MB14241, 0)

#endif /* __MB14241_H__ */
