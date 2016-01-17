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
	mb14241_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE8_MEMBER ( shift_count_w );
	DECLARE_WRITE8_MEMBER ( shift_data_w );
	DECLARE_READ8_MEMBER( shift_result_r );

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	// internal state

	UINT16 m_shift_data;  /* 15 bits only */
	UINT8 m_shift_count;  /* 3 bits */
};

extern const device_type MB14241;


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_MB14241_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, MB14241, 0)

#endif /* __MB14241_H__ */
