// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Micronas Intermetall VPX-3220A Video Pixel Decoder

    Skeleton device

***************************************************************************/

#ifndef MAME_NAMCO_VPX3220A_H
#define MAME_NAMCO_VPX3220A_H

#include "machine/i2chle.h"

#pragma once

class vpx3220a_device : public device_t, public i2c_hle_interface
{
public:
	vpx3220a_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual u8 read_data(u16 offset) override;
	virtual void write_data(u16 offset, u8 data) override;

private:
	enum subaddr_t : u8;
	enum fpaddr_t : u16;

	u8 m_ifc;
	u8 m_afend;
	u8 m_unk_34;
	u8 m_unk_3a;
	u8 m_refsig;
	u8 m_ymax;
	u8 m_ymin;
	u8 m_umax;
	u8 m_umin;
	u8 m_vmax;
	u8 m_vmin;
	u8 m_cbm_bri;
	u8 m_cbm_con;
	u8 m_format;
	u8 m_misc;
	u8 m_ofifo;
	u8 m_omux;
	u8 m_oena;
	u8 m_driver_a;
	u8 m_driver_b;

	u16 m_fpaddr;
	u8 m_fpsta;
	bool m_fp_lsb;

	u16 m_fp_tint;
	u16 m_fp_gain;
	u16 m_fp_xlg;
	u16 m_fp_hpll;
	u16 m_fp_dvco;
	u16 m_fp_adjust;
	u16 m_fp_vbeg[2];
	u16 m_fp_vlinei[2];
	u16 m_fp_vlineo[2];
	u16 m_fp_hbeg[2];
	u16 m_fp_hlen[2];
	u16 m_fp_npix[2];
	u16 m_fp_accref;
	u16 m_fp_accr;
	u16 m_fp_accb;
	u16 m_fp_kilvl;
	u16 m_fp_agcref;
	u16 m_fp_sgain;
	u16 m_fp_vsdt;
	u16 m_fp_cmdwd;
	u16 m_fp_infowd;
	u16 m_fp_tvstndwr;
	u16 m_fp_tvstndrd;
};

DECLARE_DEVICE_TYPE(VPX3220A, vpx3220a_device)

#endif // MAME_NAMCO_VPX3220A_H
