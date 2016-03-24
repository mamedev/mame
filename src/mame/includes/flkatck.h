// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/*************************************************************************

    Flak Attack / MX5000

*************************************************************************/
#include "sound/k007232.h"
#include "video/k007121.h"

class flkatck_state : public driver_device
{
public:
	flkatck_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_k007121_ram(*this, "k007121_ram"),
		m_audiocpu(*this, "audiocpu"),
		m_k007121(*this, "k007121"),
		m_k007232(*this, "k007232"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_k007121_ram;

	/* video-related */
	tilemap_t    *m_k007121_tilemap[2];
	int        m_flipscreen;

	/* misc */
	int        m_irq_enabled;
	int        m_multiply_reg[2];

	/* devices */
	required_device<cpu_device> m_audiocpu;
	required_device<k007121_device> m_k007121;
	required_device<k007232_device> m_k007232;
	DECLARE_WRITE8_MEMBER(flkatck_bankswitch_w);
	DECLARE_READ8_MEMBER(flkatck_ls138_r);
	DECLARE_WRITE8_MEMBER(flkatck_ls138_w);
	DECLARE_READ8_MEMBER(multiply_r);
	DECLARE_WRITE8_MEMBER(multiply_w);
	DECLARE_WRITE8_MEMBER(flkatck_k007121_w);
	DECLARE_WRITE8_MEMBER(flkatck_k007121_regs_w);
	TILE_GET_INFO_MEMBER(get_tile_info_A);
	TILE_GET_INFO_MEMBER(get_tile_info_B);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_flkatck(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(flkatck_interrupt);
	DECLARE_WRITE8_MEMBER(volume_callback);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};
