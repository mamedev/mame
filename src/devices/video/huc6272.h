// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Angelo Salese

#ifndef MAME_VIDEO_HUC6272_H
#define MAME_VIDEO_HUC6272_H

#pragma once

#include "bus/scsi/scsi.h"
#include "bus/scsi/scsicd.h"
#include "video/huc6271.h"
#include "speaker.h"


class huc6272_device :  public device_t,
						public device_memory_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND | feature::GRAPHICS; }

	// construction/destruction
	huc6272_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	auto irq_changed_callback() { return m_irq_changed_cb.bind(); }
	template <typename T> void set_rainbow_tag(T &&tag) { m_huc6271.set_tag(std::forward<T>(tag)); }

	void amap(address_map &map) ATTR_COLD;

	// ADPCM operations
	u8 adpcm_update_0();
	u8 adpcm_update_1();

	// CD-DA operations
	void cdda_update(offs_t offset, u8 data);

	static void cdrom_config(device_t *device);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

private:
	required_device<huc6271_device> m_huc6271;
	required_device<speaker_device> m_cdda;

	u8 m_register;
	u32 m_kram_addr_r, m_kram_addr_w;
	u16 m_kram_inc_r, m_kram_inc_w;
	u8 m_kram_page_r, m_kram_page_w;
	u32 m_kram_load_reg, m_kram_write_reg;
	u32 m_page_setting;

	struct{
		u32 bat_address = 0;
		u32 cg_address = 0;
		u8 mode = 0;
		u16 height = 0;
		u16 width = 0;
		u16 xscroll = 0;
		u16 yscroll = 0;
		u8 priority = 0;
	}m_bg[4];

	struct{
		u32 bat_address = 0;
		u32 cg_address = 0;
		u16 height = 0;
		u16 width = 0;
	}m_bg0sub;

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
	} m_scsi;

	struct dma_t {
		u32 addr = 0;
		u32 size = 0;
		u8 ctrl = 0;
		u8 latch = 0;
		u8 flipflop = 0;
		bool irq = false;
	} m_dma;

	bool m_in_scsi_update = false;

	struct{
		u8 rate = 0;
		u32 status = 0;
		s32 interrupt = 0;
		u8 playing[2]{};
		u8 control[2]{};
		u32 start[2]{};
		u32 end[2]{};
		u32 imm[2]{};
		u32 input[2]{};
		s32 nibble[2]{};
		u32 pos[2]{};
		u32 addr[2]{};
	}m_adpcm;

	const address_space_config           m_program_space_config;
	const address_space_config           m_data_space_config;
	const address_space_config           m_io_space_config;
	required_shared_ptr<u16>             m_microprg_ram;
	required_shared_ptr_array<u16, 2>    m_kram_page;
	required_device<scsi_port_device>    m_scsibus;
	required_device<input_buffer_device> m_scsi_data_in;
	required_device<output_latch_device> m_scsi_data_out;
	required_device<input_buffer_device> m_scsi_ctrl_in;
	required_device<input_buffer_device> m_scsi_cmd_in;

	/* Callback for when the irq line may have changed (mandatory) */
	devcb_write_line    m_irq_changed_cb;

	void write_microprg_data(offs_t address, u16 data);

	u8 adpcm_update(int chan);
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

	void bg_mode_w(offs_t offset, u32 data, u32 mem_mask = ~0);

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
