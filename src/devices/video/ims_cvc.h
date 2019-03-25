// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay
#ifndef MAME_VIDEO_IMS_CVC_H
#define MAME_VIDEO_IMS_CVC_H

#pragma once

class ims_cvc_device :
	public device_t,
	public device_memory_interface,
	public device_palette_interface
{
public:
	virtual void map(address_map &map) = 0;

	void screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	ims_cvc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// device_palette_interface overrides
	virtual u32 palette_entries() const override { return 256; } // FIXME

	u32 colour_palette_r(const offs_t offset) { return 0; }
	void colour_palette_w(const offs_t offset, const u32 data) {}
	u32 halfsync_r() { return m_halfsync; }
	void halfsync_w(const u32 data) { m_halfsync = data; }
	u32 backporch_r() { return m_backporch; }
	void backporch_w(const u32 data) { m_backporch = data; }
	u32 display_r() { return m_display; }
	void display_w(const u32 data) { m_display = data; }
	u32 shortdisplay_r() { return m_shortdisplay; }
	void shortdisplay_w(const u32 data) { m_shortdisplay = data; }
	u32 broadpulse_r() { return m_broadpulse; }
	void broadpulse_w(const u32 data) { m_broadpulse = data; }
	u32 vsync_r() { return m_vsync; }
	void vsync_w(const u32 data) { m_vsync = data; }
	u32 vblank_r() { return m_vblank; }
	void vblank_w(const u32 data) { m_vblank = data; }
	u32 vdisplay_r() { return m_vdisplay; }
	void vdisplay_w(const u32 data) { m_vdisplay = data; }
	u32 linetime_r() { return m_linetime; }
	void linetime_w(const u32 data) { m_linetime = data; }
	//u32 tos_r() { return m_tos; }
	//void tos_w(const u32 data) { m_tos = data; }
	u32 meminit_r() { return m_meminit; }
	void meminit_w(const u32 data) { m_meminit = data; }
	u32 transferdelay_r() { return m_transferdelay; }
	void transferdelay_w(const u32 data) { m_transferdelay = data; }

	u32 mask_r() { return m_mask; }
	void mask_w(const u32 data) { m_mask = data; }
	u32 tos_r() { return m_tos; }
	void tos_w(const u32 data) { m_tos = data; }

	void boot_w(const u32 data) { m_boot = data; }

private:
	// device_memory_interface members
	const address_space_config m_space_config;
	address_space *m_space;

	// device state
	u32 m_halfsync;
	u32 m_backporch;
	u32 m_display;
	u32 m_shortdisplay;
	u32 m_broadpulse;
	u32 m_vsync;
	u32 m_vblank;
	u32 m_vdisplay;
	u32 m_linetime;
	u32 m_meminit;
	u32 m_transferdelay;

	u32 m_mask;
	u32 m_tos;
	u32 m_boot;
};

class g300_device : public ims_cvc_device
{
public:
	g300_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;

protected:
	u32 control_r() { return m_control; }
	void control_w(const u32 data) { m_control = data; }

private:
	u32 m_control;
};

class g332_device : public ims_cvc_device
{
public:
	g332_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;

protected:
	g332_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	u32 vpreequalise_r() { return m_vpreequalise; }
	void vpreequalise_w(const u32 data) { m_vpreequalise = data; }
	u32 vpostequalise_r() { return m_vpostequalise; }
	void vpostequalise_w(const u32 data) { m_vpostequalise = data; }
	u32 linestart_r() { return m_linestart; }
	void linestart_w(const u32 data) { m_linestart = data; }
	u32 control_a_r() { return m_control_a; }
	void control_a_w(const u32 data) { m_control_a = data; }
	u32 control_b_r() { return m_control_b; }
	void control_b_w(const u32 data) { m_control_b = data; }

private:
	u32 m_vpreequalise;
	u32 m_vpostequalise;
	u32 m_linestart;
	u32 m_control_a;
	u32 m_control_b;
};

class g364_device : public g332_device
{
public:
	g364_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(G300, g300_device)
DECLARE_DEVICE_TYPE(G332, g332_device)
DECLARE_DEVICE_TYPE(G364, g364_device)

#endif // MAME_VIDEO_IMS_CVC_H
