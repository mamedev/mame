// license:BSD-3-Clause
// copyright-holders:O. Galibert

// uPD72120, the high-resolution successor to the uPD7220

#ifndef MAME_VIDEO_UPD72120_H
#define MAME_VIDEO_UPD72120_H

#pragma once

class upd72120_device :  public device_t
{
public:
	upd72120_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void map(address_map &map);

protected:
	u32 m_eadorg;
	u8 m_dadorg;
	u32 m_ead1;
	u8 m_dad1;
	u32 m_ead2;
	u8 m_dad2;
	u32 m_pdisps;
	u32 m_pdispd;
	u16 m_pmax;
	u8 m_mod;
	u32 m_ptnp;
	u32 m_stack;
	u8 m_bank;
	u8 m_control;
	u16 m_x;
	u16 m_y;
	u16 m_dx;
	u16 m_dy;
	u16 m_xs;
	u16 m_ys;
	u16 m_xe;
	u16 m_ye;
	u16 m_xc;
	u16 m_yc;
	u16 m_dh;
	u16 m_dv;
	u16 m_pitchs;
	u16 m_pitchd;
	u16 m_stmax;
	u16 m_planes;
	u16 m_ptn_cnt;
	u16 m_xclmin;
	u16 m_yclmin;
	u16 m_xclmax;
	u16 m_yclmax;
	u8 m_mag;
	u8 m_clip;
	u8 m_flags;
	u8 m_command;
	u16 m_display_flags;
	u16 m_display_pitch;
	u32 m_display_address;
	u8 m_wc;
	std::array<u16, 9> m_video_timings;
	u8 m_video_timings_index;
	u16 m_gcsrx, m_gcsrys, m_gcsrye;

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void unk_w(offs_t offset, u8 data);
	u8 unk_r(offs_t offset);

	void eadorg_w(offs_t offset, u8 data);
	void dadorg_w(u8 data);
	void ead1_w(offs_t offset, u8 data);
	void dad1_w(u8 data);
	void ead2_w(offs_t offset, u8 data);
	void dad2_w(u8 data);
	void pdisps_w(offs_t offset, u8 data);
	void pdispd_w(offs_t offset, u8 data);
	void pmax_w(offs_t offset, u8 data);
	void mod_w(u8 data);
	void ptnp_w(offs_t offset, u8 data);
	void stack_w(offs_t offset, u8 data);

	void bank_w(u8 data);
	void control_w(u8 data);
	void x_w(offs_t offset, u8 data);
	void y_w(offs_t offset, u8 data);
	void dx_w(offs_t offset, u8 data);
	void dy_w(offs_t offset, u8 data);
	void xs_w(offs_t offset, u8 data);
	void ys_w(offs_t offset, u8 data);
	void xe_w(offs_t offset, u8 data);
	void ye_w(offs_t offset, u8 data);
	void xc_w(offs_t offset, u8 data);
	void yc_w(offs_t offset, u8 data);
	void dh_w(offs_t offset, u8 data);
	void dv_w(offs_t offset, u8 data);
	void pitchs_w(offs_t offset, u8 data);
	void pitchd_w(offs_t offset, u8 data);
	void stmax_w(offs_t offset, u8 data);
	void planes_w(offs_t offset, u8 data);
	void ptn_cnt_w(offs_t offset, u8 data);
	void xclmin_w(offs_t offset, u8 data);
	void yclmin_w(offs_t offset, u8 data);
	void xclmax_w(offs_t offset, u8 data);
	void yclmax_w(offs_t offset, u8 data);
	void mag_w(u8 data);
	void clip_w(u8 data);
	void flags_w(u8 data);
	void command_w(u8 data);
	void display_flags_w(offs_t offset, u8 data);
	void display_pitch_w(offs_t offset, u8 data);
	void display_address_w(offs_t offset, u8 data);
	void wc_w(u8 data);
	void video_timings_w(offs_t offset, u8 data);
	void gcsrx_w(offs_t offset, u8 data);
	void gcsrys_w(offs_t offset, u8 data);
	void gcsrye_w(offs_t offset, u8 data);
};

DECLARE_DEVICE_TYPE(UPD72120, upd72120_device)

#endif // MAME_VIDEO_UPD72120_H
