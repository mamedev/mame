// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari

#include "machine/74259.h"
#include "sound/sn76496.h"
#include "emupal.h"

class spcforce_state : public driver_device
{
public:
	spcforce_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainlatch(*this, "mainlatch"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sn1(*this, "sn1"),
		m_sn2(*this, "sn2"),
		m_sn3(*this, "sn3"),
		m_scrollram(*this, "scrollram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void meteors(machine_config &config);
	void spcforce(machine_config &config);

private:
	DECLARE_WRITE8_MEMBER(SN76496_latch_w);
	DECLARE_READ8_MEMBER(SN76496_select_r);
	DECLARE_WRITE8_MEMBER(SN76496_select_w);
	DECLARE_WRITE_LINE_MEMBER(write_sn1_ready);
	DECLARE_WRITE_LINE_MEMBER(write_sn2_ready);
	DECLARE_WRITE_LINE_MEMBER(write_sn3_ready);
	DECLARE_READ_LINE_MEMBER(t0_r);
	DECLARE_WRITE8_MEMBER(soundtrigger_w);
	DECLARE_WRITE8_MEMBER(misc_outputs_w);
	DECLARE_WRITE_LINE_MEMBER(irq_mask_w);
	DECLARE_WRITE_LINE_MEMBER(flip_screen_w);
	DECLARE_WRITE_LINE_MEMBER(unknown_w);

	DECLARE_PALETTE_INIT(spcforce);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(vblank_irq);

	void spcforce_map(address_map &map);
	void spcforce_sound_map(address_map &map);

	virtual void machine_start() override;

	required_device<cpu_device> m_maincpu;
	required_device<ls259_device> m_mainlatch;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<sn76496_device> m_sn1;
	required_device<sn76496_device> m_sn2;
	required_device<sn76496_device> m_sn3;

	required_shared_ptr<uint8_t> m_scrollram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	output_finder<2> m_lamps;

	int m_sn76496_latch;
	int m_sn76496_select;
	int m_sn1_ready;
	int m_sn2_ready;
	int m_sn3_ready;
	uint8_t m_irq_mask;
};
