// license:LGPL-2.1+
// copyright-holders:Tomasz Slanina
#include "sound/dac.h"


#define RLT_NUM_BLITTER_REGS    8
#define RLT_NUM_BITMAPS         8

class rltennis_state : public driver_device
{
public:
	rltennis_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_dac1(*this, "dac1"),
		m_dac2(*this, "dac2"),
		m_data760000(0), m_data740000(0), m_dac_counter(0), m_sample_rom_offset_1(0), m_sample_rom_offset_2(0),
		m_offset_shift(0) { }

	required_device<cpu_device> m_maincpu;
	required_device<dac_byte_interface> m_dac1;
	required_device<dac_byte_interface> m_dac2;

	uint16_t m_blitter[RLT_NUM_BLITTER_REGS];
	int32_t m_data760000;
	int32_t m_data740000;
	int32_t m_dac_counter;
	int32_t m_sample_rom_offset_1;
	int32_t m_sample_rom_offset_2;
	int32_t m_offset_shift;
	int32_t m_unk_counter;
	std::unique_ptr<bitmap_ind16> m_tmp_bitmap[RLT_NUM_BITMAPS];
	uint8_t *m_samples_1;
	uint8_t *m_samples_2;
	uint8_t *m_gfx;
	emu_timer *m_timer;

	uint16_t io_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void snd1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void snd2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void blitter_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void interrupt(device_t &device);
	void sample_player(void *ptr, int32_t param);
};
