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
	upd4701_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void x_add( int16_t data );
	void y_add( int16_t data );
	void switches_set( uint8_t data );

	void cs_w(int state);
	void xy_w(int state);
	void ul_w(int state);
	void resetx_w(int state);
	void resety_w(int state);

	uint16_t d_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	int cf_r();
	int sf_r();

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;
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
