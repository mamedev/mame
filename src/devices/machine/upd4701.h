// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

    NEC uPD4701

    Incremental Encoder Control

***************************************************************************/

#ifndef MAME_MACHINE_UPD4701_H
#define MAME_MACHINE_UPD4701_H

#pragma once


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

DECLARE_DEVICE_TYPE(UPD4701, upd4701_device)


#define MCFG_UPD4701_ADD(tag) \
		MCFG_DEVICE_ADD((tag), UPD4701, 0)

#endif // MAME_MACHINE_UPD4701_H
