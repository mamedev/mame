// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_PCI_ZR36057_H
#define MAME_BUS_PCI_ZR36057_H

#pragma once

#include "pci_slot.h"
#include "video/saa7110.h"
#include "video/zr36060.h"

class zr36057_device : public pci_card_device
{
public:
	zr36057_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type unemulated_features() { return feature::CAPTURE; }

protected:
	zr36057_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

//  virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual void config_map(address_map &map) override ATTR_COLD;

private:
	required_device<zr36060_device> m_guest;
	required_device<saa7110a_device> m_decoder;
	void asr_map(address_map &map) ATTR_COLD;

	void software_reset();
	u32 postoffice_r(offs_t offset);
	void postoffice_w(offs_t offset, u32 data, u32 mem_mask);

	// Video Front End
	struct {
		u32 horizontal_config;
		u32 vertical_config;
		u32 format_config;
		int hspol, vspol;
		u16 hstart, hend, vstart, vend;

		int ext_fi, top_field, vclk_pol;
		u8 hfilter;
		int dup_fld;
		u8 hor_dcm, ver_dcm;
		int disp_mod;
		u8 yuv2rgb;
		int err_dif, pack24, little_endian;

		u32 vid_top_base, vid_bottom_base;

		u16 disp_stride;
		bool vid_ovf;
		bool snapshot, frame_grab;

		u32 display_config;
		bool vid_en;
		u8 min_pix;
		bool triton;
		u16 window_height, window_width;

		u32 mask_top_base, mask_bottom_base;
		bool ovl_enable;
		u8 mask_stride;
	} m_vfe;

	bool m_softreset;
	u8 m_gpio_ddr, m_pci_waitstate_control;

	// NOTE: these are technically external/public pins.
	void girq1_w(int state) { m_irq_status |= 1 << 30; update_irq_status(); }
	void girq0_w(int state) { m_irq_status |= 1 << 29; update_irq_status(); }
	void cod_rep_irq_w(int state) { m_irq_status |= 1 << 28; update_irq_status(); }
	void jpeg_rep_irq_w(int state) { m_irq_status |= 1 << 27; update_irq_status(); }

	void update_irq_status();
	u32 m_irq_status, m_irq_enable;
	bool m_inta_pin_enable;
	int m_decoder_sdao_state;

	struct {
		u8 guest_id, guest_reg;
		bool mode;
		u8 sub_mode;
		bool rtbsy_fb, go_en, sync_mstr, fld_per_buff, vfifo_fb, cfifo_fb, still_lendian;
		bool p_reset, cod_trns_en, active;
	} m_jpeg;

	struct {
		u8 vsync_size;
		u16 hsync_start;
		u16 vtotal, htotal;
	} m_sync_gen;

	struct {
		u16 nax, pax, nay, pay;
		bool odd;
	} m_active_area;

	struct {
		u8 time[8];
	} m_guestbus;

	struct {
		bool dir; /**< true: Write, false: Read */
		bool time_out;
		bool pending;
		u8 guest_id;
		u8 guest_reg;
	} m_po; /**< PostOffice */
};

DECLARE_DEVICE_TYPE(ZR36057_PCI, zr36057_device)

#endif // MAME_BUS_PCI_ZR36057_H
