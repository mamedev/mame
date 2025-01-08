// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    Turret Tower hardware

****************************************************************************/
#ifndef MAME_NAMCO_TURRETT_H
#define MAME_NAMCO_TURRETT_H

#pragma once

#include "cpu/mips/mips1.h"
#include "bus/ata/ataintf.h"
#include "emupal.h"
#include "speaker.h"
#include "screen.h"


class turrett_state : public driver_device
{
public:
	turrett_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ata(*this, "ata")
		, m_bank_a(*this, "bank_a")
		, m_bank_b(*this, "bank_b")
		, m_screen(*this, "screen")
	{
	}

	void turrett(machine_config &config);

	INPUT_CHANGED_MEMBER(ipt_change);

private:
	// constants
	static const uint32_t X_VISIBLE = 336;
	static const uint32_t Y_VISIBLE = 244;
	static const uint32_t DIMM_BANK_WORDS = 128 * 1024 * 1024 / 2;
	static const uint32_t DIMM_BANK_MASK  = DIMM_BANK_WORDS - 1;
	static const uint32_t VRAM_BANK_WORDS = 256 * 1024;

	// devices
	required_device<r3041_device> m_maincpu;
	required_device<ata_interface_device> m_ata;
	required_shared_ptr<uint16_t> m_bank_a;
	required_shared_ptr<uint16_t> m_bank_b;
	required_device<screen_device> m_screen;

	// handlers
	void dma_w(offs_t offset, uint32_t data);
	uint32_t video_r(offs_t offset, uint32_t mem_mask = ~0);
	void video_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t int_r();
	void int_w(uint32_t data);
	int sbrc2_r();
	int sbrc3_r();

	TIMER_CALLBACK_MEMBER(dma_complete);
	INTERRUPT_GEN_MEMBER(vblank);
	INTERRUPT_GEN_MEMBER(adc);

	// functions
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t write_video_ram(uint16_t data);
	void update_video_addr(void);
	uint32_t update_inputs(void);

	// members
	emu_timer *m_dma_timer = nullptr;
	uint32_t  m_inputs_active = 0;
	std::unique_ptr<uint16_t[]>  m_video_ram[2];
	uint16_t  m_last_pixel = 0;
	int32_t   m_video_ctrl = 0;
	uint16_t  m_video_fade = 0;
	int16_t   m_x_pos = 0;
	int16_t   m_x_start = 0;
	int16_t   m_x_mod = 0;
	int16_t   m_dx = 0;
	int16_t   m_y_pos = 0;
	int16_t   m_scale_cnt_y = 0;
	int16_t   m_scale_cnt_x = 0;
	bool    m_skip_x = false;
	bool    m_skip_y = false;
	int16_t   m_scale = 0;
	int16_t   m_hotspot_x = 0;
	int16_t   m_hotspot_y = 0;
	bool    m_dma_idle = false;
	uint32_t  m_dma_addr[2]{};
	uint32_t  m_ipt_val = 0;
	uint8_t   m_frame = 0;
	uint8_t   m_adc = 0;

	void cpu_map(address_map &map) ATTR_COLD;
	void turrett_sound_map(address_map &map) ATTR_COLD;

	// driver_device overrides
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
};


class turrett_device : public device_t,
						public device_sound_interface,
						public device_memory_interface
{
	static const uint32_t SOUND_CHANNELS = 32;

public:
	// construction/destruction
	turrett_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint32_t read(offs_t offset);
	void write(offs_t offset, uint32_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	const address_space_config  m_space_config;

private:
	memory_access<28, 1, 0, ENDIANNESS_LITTLE>::cache m_cache;
	sound_stream *m_stream = nullptr;

	struct
	{
		uint32_t  m_address = 0;
		uint32_t  m_volume = 0;
		bool    m_playing = false;
	} m_channels[SOUND_CHANNELS];

	int32_t m_volume_table[0x50]{};
};

// device type definition
DECLARE_DEVICE_TYPE(TURRETT, turrett_device)

#endif // MAME_NAMCO_TURRETT_H
