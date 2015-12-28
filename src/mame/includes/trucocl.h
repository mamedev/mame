// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi
#include "sound/dac.h"

class trucocl_state : public driver_device
{
public:
	enum
	{
		TIMER_DAC_IRQ
	};

	trucocl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_dac(*this, "dac"),
		m_gfxdecode(*this, "gfxdecode") { }

	int m_cur_dac_address;
	int m_cur_dac_address_index;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	tilemap_t *m_bg_tilemap;

	UINT8 m_irq_mask;
	DECLARE_WRITE8_MEMBER(irq_enable_w);
	DECLARE_WRITE8_MEMBER(trucocl_videoram_w);
	DECLARE_WRITE8_MEMBER(trucocl_colorram_w);
	DECLARE_WRITE8_MEMBER(audio_dac_w);
	DECLARE_DRIVER_INIT(trucocl);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(trucocl);
	UINT32 screen_update_trucocl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(trucocl_interrupt);
	required_device<cpu_device> m_maincpu;
	required_device<dac_device> m_dac;
	required_device<gfxdecode_device> m_gfxdecode;

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
