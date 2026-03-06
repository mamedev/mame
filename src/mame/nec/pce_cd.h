// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, Angelo Salese
#ifndef MAME_NEC_PCE_CD_H
#define MAME_NEC_PCE_CD_H

#pragma once

/***************************************************************************
 TYPE DEFINITIONS
 ***************************************************************************/

#include "imagedev/cdromimg.h"
#include "machine/nvram.h"
#include "sound/cdda.h"
#include "sound/msm5205.h"



// ======================> pce_cd_device

class pce_cd_device : public device_t,
					  public device_memory_interface,
					  public device_mixer_interface
{
public:
	// construction/destruction
	pce_cd_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	template <typename T> void set_maincpu(T &&tag) { m_maincpu.set_tag(std::forward<T>(tag)); }
	auto irq() { return m_irq_cb.bind(); }

	void update();

	void late_setup();

	void bram_w(offs_t offset, u8 data);
	void intf_w(offs_t offset, u8 data);
	u8 bram_r(offs_t offset);
	u8 intf_r(offs_t offset);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual space_config_vector memory_space_config() const override;

private:
	static constexpr size_t PCE_BRAM_SIZE              = 0x800;
	static constexpr size_t PCE_ADPCM_RAM_SIZE         = 0x10000;
	static constexpr size_t PCE_CD_COMMAND_BUFFER_SIZE = 0x100;

	static constexpr u8 PCE_CD_IRQ_TRANSFER_READY   = 0x40;
	static constexpr u8 PCE_CD_IRQ_TRANSFER_DONE    = 0x20;
	static constexpr u8 PCE_CD_IRQ_BRAM             = 0x10; /* ??? */
	static constexpr u8 PCE_CD_IRQ_SAMPLE_FULL_PLAY = 0x08;
	static constexpr u8 PCE_CD_IRQ_SAMPLE_HALF_PLAY = 0x04;

	static constexpr u8 PCE_CD_ADPCM_PLAY_FLAG = 0x08;
	static constexpr u8 PCE_CD_ADPCM_STOP_FLAG = 0x01;

	static constexpr int PCE_CD_DATA_FRAMES_PER_SECOND = 75;

	enum {
		PCE_CD_CDDA_OFF = 0,
		PCE_CD_CDDA_PLAYING,
		PCE_CD_CDDA_PAUSED
	};

	const address_space_config m_space_config;

	u8 cdc_status_r();
	void cdc_status_w(u8 data);
	u8 cdc_reset_r();
	void cdc_reset_w(u8 data);
	u8 irq_mask_r();
	void irq_mask_w(u8 data);
	u8 irq_status_r();
	u8 cdc_data_r();
	void cdc_data_w(u8 data);
	u8 bram_status_r();
	void bram_unlock_w(u8 data);
	u8 cdda_data_r(offs_t offset);
	u8 cd_data_r();
	u8 adpcm_dma_control_r();
	void adpcm_dma_control_w(u8 data);
	u8 adpcm_status_r();
	u8 adpcm_data_r();
	void adpcm_data_w(u8 data);
	void adpcm_address_lo_w(u8 data);
	void adpcm_address_hi_w(u8 data);
	u8 adpcm_address_control_r();
	void adpcm_address_control_w(u8 data);
	void adpcm_playback_rate_w(u8 data);
	void fader_control_w(u8 data);

	void adpcm_stop(u8 irq_flag);
	void adpcm_play();
	void reply_status_byte(u8 status);
	void test_unit_ready();
	void read_6();
	void nec_set_audio_start_position();
	void nec_set_audio_stop_position();
	void nec_pause();
	void nec_get_subq();
	void nec_get_dir_info();
	void end_of_list();
	void handle_data_output();
	void handle_data_input();
	void handle_message_output();
	void handle_message_input();
	void set_irq_line(int num, int state);
	void set_adpcm_ram_byte(u8 val);
	u8 get_cd_data_byte();
	u8 get_adpcm_ram_byte();

	TIMER_CALLBACK_MEMBER(data_timer_callback);
	TIMER_CALLBACK_MEMBER(cdda_fadeout_callback);
	TIMER_CALLBACK_MEMBER(cdda_fadein_callback);
	TIMER_CALLBACK_MEMBER(adpcm_fadeout_callback);
	TIMER_CALLBACK_MEMBER(adpcm_fadein_callback);
	TIMER_CALLBACK_MEMBER(clear_ack);
	TIMER_CALLBACK_MEMBER(adpcm_dma_timer_callback);

	void msm5205_int(int state);
	void nvram_init(nvram_device &nvram, void *data, size_t size);

	void cdda_end_mark_cb(int state);

	void regs_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<msm5205_device> m_msm;
	required_device<cdda_device> m_cdda;
	required_device<nvram_device> m_nvram;
	required_device<cdrom_image_device> m_cdrom;

	devcb_write_line    m_irq_cb;

	u8 m_reset_reg = 0;
	u8 m_irq_mask = 0;
	u8 m_irq_status = 0;
	u8 m_cdc_status = 0;
	u8 m_cdc_data = 0;
	u8 m_bram_status = 0;
	u8 m_adpcm_status = 0;
	u16 m_adpcm_latch_address = 0;
	u8 m_adpcm_control = 0;
	u8 m_adpcm_dma_reg = 0;
	u8 m_fader_ctrl = 0;

	std::unique_ptr<u8[]> m_bram;
	std::unique_ptr<u8[]> m_adpcm_ram;
	s32  m_bram_locked = 0;
	s32  m_adpcm_read_ptr = 0;
	u8   m_adpcm_read_buf = 0;
	s32  m_adpcm_write_ptr = 0;
	u8   m_adpcm_write_buf = 0;
	s32  m_adpcm_length = 0;
	s32  m_adpcm_clock_divider = 0;
	u32  m_msm_start_addr = 0;
	u32  m_msm_end_addr = 0;
	u32  m_msm_half_addr = 0;
	u8   m_msm_nibble = 0;
	u8   m_msm_idle = 0;
	u8   m_msm_repeat = 0;

	/* SCSI signals */
	s32     m_scsi_BSY = 0;   /* Busy. Bus in use */
	s32     m_scsi_SEL = 0;   /* Select. Initiator has won arbitration and has selected a target */
	s32     m_scsi_CD = 0;    /* Control/Data. Target is sending control (data) information */
	s32     m_scsi_IO = 0;    /* Input/Output. Target is sending (receiving) information */
	s32     m_scsi_MSG = 0;   /* Message. Target is sending or receiving a message */
	s32     m_scsi_REQ = 0;   /* Request. Target is requesting a data transfer */
	s32     m_scsi_ACK = 0;   /* Acknowledge. Initiator acknowledges that it is ready for a data transfer */
	s32     m_scsi_ATN = 0;   /* Attention. Initiator has a message ready for the target */
	s32     m_scsi_RST = 0;   /* Reset. Initiator forces all targets and any other initiators to do a warm reset */
	s32     m_scsi_last_RST = 0;  /* To catch setting of RST signal */
	s32     m_cd_motor_on = 0;
	s32     m_selected = 0;
	std::unique_ptr<u8[]>  m_command_buffer;
	s32     m_command_buffer_index = 0;
	s32     m_status_sent = 0;
	s32     m_message_after_status = 0;
	s32     m_message_sent = 0;
	std::unique_ptr<u8[]> m_data_buffer;
	s32     m_data_buffer_size = 0;
	s32     m_data_buffer_index = 0;
	s32     m_data_transferred = 0;

	u32  m_current_frame = 0;
	u32  m_end_frame = 0;
	u32  m_last_frame = 0;
	u8   m_cdda_status = 0;
	u8   m_cdda_play_mode = 0;
	std::unique_ptr<u8[]>   m_subcode_buffer;
	u8   m_end_mark = 0;

	const cdrom_file::toc*  m_toc = nullptr;
	emu_timer   *m_data_timer = nullptr;
	emu_timer   *m_adpcm_dma_timer = nullptr;

	emu_timer   *m_cdda_fadeout_timer = nullptr;
	emu_timer   *m_cdda_fadein_timer = nullptr;
	double       m_cdda_volume = 0;
	emu_timer   *m_adpcm_fadeout_timer = nullptr;
	emu_timer   *m_adpcm_fadein_timer = nullptr;
	double       m_adpcm_volume = 0;

	emu_timer   *m_ack_clear_timer = nullptr;
};



// device type definition
DECLARE_DEVICE_TYPE(PCE_CD, pce_cd_device)

#endif // MAME_NEC_PCE_CD_H
