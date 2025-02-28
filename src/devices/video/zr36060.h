// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_VIDEO_ZR36060_H
#define MAME_VIDEO_ZR36060_H

#pragma once

class zr36060_device : public device_t, public device_memory_interface
{
public:
	zr36060_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;
private:
	void regs_map(address_map &map);

	u16 m_address;
	address_space_config m_space_config;

	// Codec Control
	u8 load_r(offs_t offset);
	void load_w(offs_t offset, u8 data);
	u8 code_fifo_status_r(offs_t offset);
	u8 code_if_r(offs_t offset);
	void code_if_w(offs_t offset, u8 data);
	u8 codec_mode_r(offs_t offset);
	void codec_mode_w(offs_t offset, u8 data);
	u8 mbcv_r(offs_t offset);
	void mbcv_w(offs_t offset, u8 data);
	u8 markers_enable_r(offs_t offset);
	void markers_enable_w(offs_t offset, u8 data);
	u8 irq_mask_r(offs_t offset);
	void irq_mask_w(offs_t offset, u8 data);
	u8 irq_status_r(offs_t offset);
	u8 tcv_net_r(offs_t offset);
	void tcv_net_w(offs_t offset, u8 data);
	u8 tcv_data_r(offs_t offset);
	void tcv_data_w(offs_t offset, u8 data);
	u8 sf_r(offs_t offset);
	void sf_w(offs_t offset, u8 data);
	u8 af_r(offs_t offset);
	void af_w(offs_t offset, u8 data);
	u8 acv_r(offs_t offset);
	void acv_w(offs_t offset, u8 data);
	u8 act_r(offs_t offset);
	void act_w(offs_t offset, u8 data);
	u8 acv_trun_r(offs_t offset);
	void acv_trun_w(offs_t offset, u8 data);

	bool m_syncrst;
	bool m_load;
	bool m_code16;
	bool m_endian;
	bool m_cfis;
	bool m_code_mstr;
	u8 m_codec_mode;
	u8 m_mbcv;
	u8 m_markers_enable;
	u8 m_irq_mask, m_irq_status;
	// these are all forms of floating-point registers
	u8 m_tcv_net[4], m_tcv_data[4];
	u8 m_sf[2];
	u8 m_af[3]; // 24-bits
	u8 m_acv[4], m_act[4], m_acv_trun[4];

	// Video Registers
	u8 video_control_r(offs_t offset);
	void video_control_w(offs_t offset, u8 data);
	u8 video_polarity_r(offs_t offset);
	void video_polarity_w(offs_t offset, u8 data);
	u8 video_scaling_r(offs_t offset);
	void video_scaling_w(offs_t offset, u8 data);

	u8 m_video_control;
	u8 m_video_polarity;
	u8 m_hscale, m_vscale;

	struct {
		u8 y, u, v;
	} m_bg_color;

	struct {
		u16 vtotal, htotal;
		u8 vsync_size, hsync_size;
		u8 bvstart, bhstart;
		u16 bvend, bhend;
	} m_sync_gen;

	struct {
		u16 vstart, vend, hstart, hend;
	} m_active_area;

	struct {
		u16 svstart, svend, shstart, shend;
	} m_subimg_window;
};

DECLARE_DEVICE_TYPE(ZR36060, zr36060_device)

#endif // MAME_VIDEO_ZR36060_H
