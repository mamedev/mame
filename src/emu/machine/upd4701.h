// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    NEC uPD4701

    Incremental Encoder Control

***************************************************************************/

#ifndef __UPD4701_H__
#define __UPD4701_H__

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

class upd4701_device : public device_t
{
public:
	upd4701_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void x_add( INT16 data );
	void y_add( INT16 data );
	void switches_set( UINT8 data );

	DECLARE_WRITE_LINE_MEMBER( cs_w );
	DECLARE_WRITE_LINE_MEMBER( xy_w );
	DECLARE_WRITE_LINE_MEMBER( ul_w );
	DECLARE_WRITE_LINE_MEMBER( resetx_w );
	DECLARE_WRITE_LINE_MEMBER( resety_w );

	DECLARE_READ16_MEMBER( d_r );
	DECLARE_READ_LINE_MEMBER( cf_r );
	DECLARE_READ_LINE_MEMBER( sf_r );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
private:
	// internal state
	int m_cs;
	int m_xy;
	int m_ul;
	int m_resetx;
	int m_resety;
	int m_latchx;
	int m_latchy;
	int m_startx;
	int m_starty;
	int m_x;
	int m_y;
	int m_switches;
	int m_latchswitches;
	int m_cf;
};

extern const device_type UPD4701;


#define MCFG_UPD4701_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, UPD4701, 0)

#endif  /* __UPD4701_H__ */
