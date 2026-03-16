// license:BSD-3-Clause
// copyright-holders: Angelo Salese

#ifndef MAME_BUS_PCI_TRIDENT_4DWAVEDX_H
#define MAME_BUS_PCI_TRIDENT_4DWAVEDX_H

#pragma once

#include "pci_slot.h"

#include "bus/pc_joy/pc_joy.h"
#include "sound/ac97_stac9704.h"

class t4dwave_pcm_device;

class trident_4dwavedx_device : public pci_card_device
{
public:
	trident_4dwavedx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr feature_type imperfect_features() { return feature::SOUND; }
	static constexpr feature_type unemulated_features() { return feature::MICROPHONE; }

protected:
	trident_4dwavedx_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void map_extra(uint64_t memory_window_start, uint64_t memory_window_end, uint64_t memory_offset, address_space *memory_space,
						   uint64_t io_window_start, uint64_t io_window_end, uint64_t io_offset, address_space *io_space) override;

	virtual void config_map(address_map &map) override;

	virtual u8 capptr_r() override;

private:
	void io_map(address_map &map);
	void mmio_map(address_map &map);

	void gameport_map(address_map &map);

	required_device<t4dwave_pcm_device> m_pcm;
	required_device<ac97_stac9704_device> m_ac97;
	required_device<pc_joy_device> m_joy;

	u32 m_ddma_config;
	u32 m_legacy_control;
	struct {
		bool enable;
		u8 vector;
	} m_interrupt_snoop;

	u8 m_power_state;

	u32 m_asr3;
	u8 m_asr4, m_asr5, m_asr6;
};

class t4dwave_pcm_device : public device_t
                         , public device_sound_interface
{
public:
	t4dwave_pcm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto datain_cb() { return m_datain_cb.bind(); }
	auto irq_cb()    { return m_irq_cb.bind(); }

	u32 banka_status_r(offs_t offset);
	void starta_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void stopa_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 aina_r(offs_t offset);
	void aina_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 aintena_r(offs_t offset);
	void aintena_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 wavevol_r(offs_t offset);
	void wavevol_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 global_control_r(offs_t offset);
	void global_control_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 miscint_r(offs_t offset);
	void miscint_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 bankb_status_r(offs_t offset);
	void startb_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void stopb_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 ainb_r(offs_t offset);
	void ainb_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 aintenb_r(offs_t offset);
	void aintenb_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 cso_r(offs_t offset);
	void cso_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	//u32 lba_r(offs_t offset);
	void lba_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 eso_r(offs_t offset);
	void eso_w(offs_t offset, u32 data, u32 mem_mask = ~0);

	u32 gvsel_r(offs_t offset);
	void gvsel_w(offs_t offset, u32 data, u32 mem_mask = ~0);

protected:
	t4dwave_pcm_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream) override;
private:
	sound_stream *m_stream;

	devcb_read32     m_datain_cb;
	devcb_write_line m_irq_cb;

	struct channel_t {
		u32 lba;
		u32 cso; // current sample pointer
		u32 eso_cache;
		u32 hso[8]; // half trigger irq
		u32 eso[8]; // end trigger irq
		u16 delta; // 4.12 format / 48 kHz

		u32 gvsel_cache;
		bool gvsel;
		bool pan_control;
		u8 pan_vol;
		u8 vol;
		u8 play_mode;
		//bool is_16bit;
		//bool is_stereo;
		//bool is_signed;
		bool loop_enable;
		u16 ec_envelope;
		u8 pci_buf;

		// internal variables for interpolation
		u32 ticks;
		u32 sample_data;
		bool dma_fetch;
	};

	u32 m_global_control;
	u8 m_cir;

	u32 m_aina, m_aintena;
	u32 m_bankA_keyon;

	u32 m_ainb, m_aintenb;
	u32 m_bankB_keyon;

	enum {
		PB_UNDERUN_IRQ = 0,
		REC_OVERUN_IRQ,
		SB_IRQ,
		MPU401_IRQ,
		OPL3_IRQ,
		ADDRESS_IRQ,
		ENVELOPE_IRQ
	};

	u32 m_miscint;

	enum {
		MUSICVOL = 0,
		WAVEVOL = 1
	};

	u32 m_vol_cache;
	u8 m_volL[2], m_volR[2];

	channel_t m_channel[64];

	void update_irq_state();

	typedef std::tuple<s16, s16> (t4dwave_pcm_device::*get_sample_func)(u32 sample_data);
	static const get_sample_func get_sample_table[8];

	std::tuple<s16, s16> get_sample_u8_mono(u32 sample_data);
	std::tuple<s16, s16> get_sample_s8_mono(u32 sample_data);
	std::tuple<s16, s16> get_sample_u8_stereo(u32 sample_data);
	std::tuple<s16, s16> get_sample_s8_stereo(u32 sample_data);
	std::tuple<s16, s16> get_sample_u16_mono(u32 sample_data);
	std::tuple<s16, s16> get_sample_s16_mono(u32 sample_data);
	std::tuple<s16, s16> get_sample_u16_stereo(u32 sample_data);
	std::tuple<s16, s16> get_sample_s16_stereo(u32 sample_data);


	std::string print_audio_state(u64 keyon);
};

DECLARE_DEVICE_TYPE(TRIDENT_4DWAVEDX, trident_4dwavedx_device)
DECLARE_DEVICE_TYPE(T4DWAVE_PCM,      t4dwave_pcm_device)

#endif // MAME_BUS_PCI_TRIDENT_4DWAVEDX_H
