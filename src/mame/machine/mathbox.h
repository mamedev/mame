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
	mathbox_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE8_MEMBER( go_w );
	DECLARE_READ8_MEMBER( status_r );
	DECLARE_READ8_MEMBER( lo_r );
	DECLARE_READ8_MEMBER( hi_r );

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	private:
	// internal state

	/* math box scratch registers */
	INT16 m_reg[16];

	/* math box result */
	INT16 m_result;
};

extern const device_type MATHBOX;
