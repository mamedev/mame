// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek, Angelo Salese, Brian Johnson
/***************************************************************************

    QX-series uPD7220 (GDC) video cards

***************************************************************************/

#ifndef MAME_BUS_EPSON_QX_VIDEO_QX_GDC_CARDS_H
#define MAME_BUS_EPSON_QX_VIDEO_QX_GDC_CARDS_H

#pragma once

#include "video.h"

#include "video/upd7220.h"
#include "emupal.h"

namespace bus::epson_qx::video {

//**************************************************************************
//  ABSTRACT BASE
//**************************************************************************

class qx_gdc_card_device : public device_t, public device_qx_video_interface
{
protected:
	qx_gdc_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, size_t vram_size);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_qx_video_interface
	virtual void install_io(address_space &space) override ATTR_COLD;
	virtual uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect) override;
	virtual uint8_t dack_r() override { return m_hgdc->dack_r(); }
	virtual void dack_w(uint8_t data) override { m_hgdc->dack_w(data); }

	// hooks for derived cards
	virtual void palette_init(palette_device &palette) const = 0;
	virtual void upd7220_map(address_map &map) ATTR_COLD = 0;
	virtual UPD7220_DISPLAY_PIXELS_MEMBER(hgdc_display_pixels) = 0;
	virtual uint8_t id_r() = 0;
	virtual uint8_t text_color(uint8_t attr) const = 0;

	UPD7220_DRAW_TEXT_LINE_MEMBER(hgdc_draw_text);

	void id_map(address_map &map) ATTR_COLD;
	void gdc_map(address_map &map) ATTR_COLD;
	void zoom_map(address_map &map) ATTR_COLD;
	void zoom_w(uint8_t data) { m_zoom = data & 0x0f; }
	uint8_t lightpen_r() { return 0xff; }  // TODO: light pen request

	required_device<upd7220_device> m_hgdc;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_region_ptr<uint8_t> m_chargen;
	memory_share_creator<uint16_t> m_vram;

	uint8_t m_zoom;
};


//**************************************************************************
//  MONOCHROME BASE
//**************************************************************************

class qx_gdc_mono_card_device : public qx_gdc_card_device
{
protected:
	static constexpr size_t VRAM_SIZE = 0x20000;

	qx_gdc_mono_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void palette_init(palette_device &palette) const override;
	virtual void upd7220_map(address_map &map) override ATTR_COLD;
	virtual UPD7220_DISPLAY_PIXELS_MEMBER(hgdc_display_pixels) override;
	virtual uint8_t id_r() override { return 0x00; }
	virtual uint8_t text_color(uint8_t attr) const override { return (attr & 4) ? 2 : 1; }
};


//**************************************************************************
//  COLOR BASE
//**************************************************************************

class qx_gdc_color_card_device : public qx_gdc_card_device
{
protected:
	static constexpr size_t VRAM_SIZE = 0x60000;

	qx_gdc_color_card_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void install_io(address_space &space) override ATTR_COLD;

	virtual void palette_init(palette_device &palette) const override;
	virtual void upd7220_map(address_map &map) override ATTR_COLD;
	virtual UPD7220_DISPLAY_PIXELS_MEMBER(hgdc_display_pixels) override;
	virtual uint8_t id_r() override { return 0x01; }
	virtual uint8_t text_color(uint8_t attr) const override { return 1; }

private:
	void bank_map(address_map &map) ATTR_COLD;
	uint8_t vram_bank_r() { return m_vram_bank_val; }
	void vram_bank_w(uint8_t data);

	memory_bank_creator m_vram_bank;
	uint8_t m_vram_bank_val;
};


//**************************************************************************
//  CONCRETE CARDS
//**************************************************************************

class q10gms_device : public qx_gdc_mono_card_device
{
public:
	q10gms_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void install_io(address_space &space) override ATTR_COLD;

private:
	void lightpen_map(address_map &map) ATTR_COLD;
};

class q10cms_device : public qx_gdc_color_card_device
{
public:
	q10cms_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void install_io(address_space &space) override ATTR_COLD;

private:
	void lightpen_map(address_map &map) ATTR_COLD;
};

void video_cards(device_slot_interface &device);

} // namespace bus::epson_qx::video


DECLARE_DEVICE_TYPE_NS(QX10_VIDEO_GMS, bus::epson_qx::video, q10gms_device)
DECLARE_DEVICE_TYPE_NS(QX10_VIDEO_CMS, bus::epson_qx::video, q10cms_device)

#endif // MAME_BUS_EPSON_QX_VIDEO_QX_GDC_CARDS_H
