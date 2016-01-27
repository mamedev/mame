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
		m_dac_1(*this, "dac1"),
		m_dac_2(*this, "dac2"),
		m_data760000(0), m_data740000(0), m_dac_counter(0), m_sample_rom_offset_1(0), m_sample_rom_offset_2(0),
		m_offset_shift(0) { }

	required_device<cpu_device> m_maincpu;
	required_device<dac_device> m_dac_1;
	required_device<dac_device> m_dac_2;

	UINT16 m_blitter[RLT_NUM_BLITTER_REGS];
	INT32 m_data760000;
	INT32 m_data740000;
	INT32 m_dac_counter;
	INT32 m_sample_rom_offset_1;
	INT32 m_sample_rom_offset_2;
	INT32 m_offset_shift;
	INT32 m_unk_counter;
	std::unique_ptr<bitmap_ind16> m_tmp_bitmap[RLT_NUM_BITMAPS];
	UINT8 *m_samples_1;
	UINT8 *m_samples_2;
	UINT8 *m_gfx;
	emu_timer *m_timer;

	DECLARE_READ16_MEMBER(io_r);
	DECLARE_WRITE16_MEMBER(snd1_w);
	DECLARE_WRITE16_MEMBER(snd2_w);
	DECLARE_WRITE16_MEMBER(blitter_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(interrupt);
	TIMER_CALLBACK_MEMBER(sample_player);
};
