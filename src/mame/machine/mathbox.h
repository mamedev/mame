// license:BSD-3-Clause
// copyright-holders:Eric Smith
/*
 * mathbox.h: math box simulation (Battlezone/Red Baron/Tempest)
 *
 * Copyright Eric Smith
 *
 */

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_MATHBOX_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MATHBOX, 0)


/* ----- device interface ----- */
class mathbox_device : public device_t
{
public:
	mathbox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void go_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t status_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t lo_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t hi_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	private:
	// internal state

	/* math box scratch registers */
	int16_t m_reg[16];

	/* math box result */
	int16_t m_result;
};

extern const device_type MATHBOX;
