// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Angelo Salese

#ifndef MAME_VIDEO_HUC6272_H
#define MAME_VIDEO_HUC6272_H

#pragma once

#include "machine/nscsi_bus.h"
#include "machine/nscsi_cb.h"
#include "video/huc6271.h"


class huc6272_device :  public device_t,
						public device_memory_interface,
						public device_mixer_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND | feature::GRAPHICS; }

	// construction/destruction
	huc6272_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto irq_changed_callback() { return m_irq_changed_cb.bind(); }
	template <typename T> void set_rainbow_tag(T &&tag) { m_huc6271.set_tag(std::forward<T>(tag)); }

	void amap(address_map &map) ATTR_COLD;

	// ADPCM operations, called at the '6230 sample rate:
	// bit 7 = new nibble in bits 3-0, bits 6-5 = rate (nibble every 2^rate samples)
	u8 adpcm_update_0();
	u8 adpcm_update_1();

	// video output, for the '6261
	void render_bg_line(int raster, u32 *dest);
	int rainbow_line(int raster, u32 *dest);
	void rainbow_hsync() { m_huc6271->hsync(); }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

private:
	required_device<huc6271_device> m_huc6271;

	u8 m_register;
	u32 m_kram_addr_r, m_kram_addr_w;
	u16 m_kram_inc_r, m_kram_inc_w;
	u8 m_kram_page_r, m_kram_page_w;
	u32 m_kram_load_reg, m_kram_write_reg;
	u32 m_page_setting;
	u8 m_kram_mode;

	struct bg_t {
		u16 bat = 0;
		u16 cg = 0;
		u16 size = 0;
		u16 xscroll = 0;
		u16 yscroll = 0;
	} m_bg[4];
	u16 m_bg_mode;
	u16 m_bg_priority;
	u8 m_bg_scroll_mode;
	u16 m_bg0sub_bat;
	u16 m_bg0sub_cg;
	u16 m_bg_affine[6];

	struct{
		u8 index = 0;
		u8 ctrl = 0;
	}m_micro_prg;

	struct scsi_t {
		u8 data = 0;
		u8 ctrl = 0;
		u8 mode = 0;
		u8 phase = 0;
		u8 data_cache = 0;
		bool ack = false;
		bool drq = false;
		bool dma_receive = false;
		bool dma_send = false;
		bool irq = false;
		u8 subchannel_ctrl = 0;
		u8 subchannel_data = 0;      // SCOR Q R S T U V W
		bool subchannel_irq = false; // SBCODE.I
		u8 bus = 0;            // bus control lines as last seen, scsi_bus_signals() layout
		bool req_last = false; // REQ as of the last req_callback, for edge detection
		bool req_rose = false; // REQ rose since scsi_lines_changed last ran (sticky)
	} m_scsi;

	struct dma_t {
		u32 addr = 0;
		u32 size = 0;
		u8 ctrl = 0;
		u8 latch = 0;
		u8 flipflop = 0;
		bool irq = false;
	} m_dma;

	bool m_in_scsi_update;
	emu_timer *m_scsi_line_timer;

	struct rainbow_t {
		u8 control = 0;
		u32 address = 0;
		u16 start_line = 0;
		u8 block_count = 0;
		u16 raster_irq_line = 0;
		u32 read_pos = 0;
		u8 busy = 0;
		u8 blocks = 0;
		bool start_pending = false;
		bool raster_irq = false;
	} m_rainbow;

	struct{
		u8 rate = 0;
		u32 status = 0;
		s32 interrupt = 0;
		u8 playing[2]{};
		u8 control[2]{};
		u32 start[2]{};
		u32 end[2]{};
		u32 imm[2]{};
		u16 halfword[2]{};
		u8 have_halfword[2]{};
		u8 nibble[2]{};
		s32 divider = 0;
		u8 tick = 0;
		u32 addr[2]{};
	}m_adpcm;

	const address_space_config           m_program_space_config;
	const address_space_config           m_data_space_config;
	const address_space_config           m_io_space_config;
	required_shared_ptr<u16>             m_microprg_ram;
	required_shared_ptr_array<u16, 2>    m_kram_page;
	required_device<nscsi_bus_device>      m_scsibus;
	required_device<nscsi_callback_device> m_scsicb;

	/* Callback for when the irq line may have changed (mandatory) */
	devcb_write_line    m_irq_changed_cb;
	bool                m_irq_state;

	void write_microprg_data(offs_t address, u16 data);

	u8 adpcm_update(int chan);
	u16 adpcm_fetch(int chan);
	void interrupt_update();

	void io_map(address_map &map) ATTR_COLD;
	void kram_map(address_map &map) ATTR_COLD;
	void microprg_map(address_map &map) ATTR_COLD;

//  void write(offs_t offset, u32 data);
	// host interface
	u16 status_r(offs_t offset);
	void register_select_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 status2_r(offs_t offset);
	// u32 mem_mask = ~0
	u32 data_r(offs_t offset, u32 mem_mask = ~0);
	void data_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	// SCSI controller
	u8 scsi_bus_signals();
	bool scsi_phase_match();
	void scsi_set_ack(bool state);
	void scsi_set_mode(u8 data);
	void scsi_update_data_bus();
	void scsi_req_w(int state);
	void scsi_line_w(int state);
	void scsi_req_edge_w(int state);
	TIMER_CALLBACK_MEMBER(scsi_lines_changed);
	void scsi_dma_update();
	void dma_write_byte(u8 data);

	// internal I/O handlers
	u32 scsi_data_r(offs_t offset);
	void scsi_data_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 scsi_ctrl_r(offs_t offset);
	void scsi_ctrl_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 scsi_mode_r(offs_t offset);
	void scsi_mode_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 scsi_phase_r(offs_t offset);
	void scsi_phase_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 scsi_bus_status_r(offs_t offset);
	u32 scsi_bus_r(offs_t offset, u32 mem_mask = ~0);
	void scsi_bus_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 scsi_input_data_r(offs_t offset);
	u32 scsi_irq_ack_r(offs_t offset);
	void scsi_dma_start_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 scsi_subchannel_r(offs_t offset);
	void scsi_subcode_w(u8 data);
	void scsi_subchannel_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 dma_address_r(offs_t offset);
	void dma_address_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 dma_size_r(offs_t offset);
	void dma_size_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 dma_control_r(offs_t offset, u32 mem_mask = ~0);
	void dma_control_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 kram_read_address_r(offs_t offset);
	void kram_read_address_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 kram_write_address_r(offs_t offset);
	void kram_write_address_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u16 kram_read_data_r(offs_t offset, u16 mem_mask = ~0);
	void kram_write_data_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	u32 kram_page_setup_r(offs_t offset);
	void kram_page_setup_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 bg_mode_r(offs_t offset);
	void bg_mode_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void bg_scroll_mode_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void bg_affine_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	void bg_priority_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	void microprogram_address_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void microprogram_data_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u8 microprogram_control_r(offs_t offset);
	void microprogram_control_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	template <unsigned N> void bg_bat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <unsigned N> void bg_cg_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void bg0sub_bat_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void bg0sub_cg_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void bg_size_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void bg_scroll_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	void rainbow_control_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void rainbow_address_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void rainbow_start_line_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void rainbow_block_count_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void raster_irq_line_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u8 rainbow_fetch();

	u16 kram_word(int page, u32 base, u32 offset);
	void bg_decode_8(u8 mode, const u16 *cg, u32 pbn, u32 layer_or, u32 *dest);
	void draw_bg_layer(int n, int raster, u32 *line);

	void adpcm_control_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void adpcm_channel_control_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <unsigned N> void adpcm_start_address_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <unsigned N> void adpcm_end_address_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	template <unsigned N> void adpcm_imm_address_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 adpcm_status_r(offs_t offset);
};

// device type definition
DECLARE_DEVICE_TYPE(HUC6272, huc6272_device)

#endif // MAME_VIDEO_HUC6272_H
