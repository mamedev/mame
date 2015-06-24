// license:BSD-3-Clause
// copyright-holders:David Graves
/*************************************************************************

    Slapshot / Operation Wolf 3

*************************************************************************/

#include "audio/taitosnd.h"
#include "machine/taitoio.h"
#include "video/tc0360pri.h"
#include "video/tc0480scp.h"

struct slapshot_tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int primask;
};

class slapshot_state : public driver_device
{
public:
	enum
	{
		TIMER_SLAPSHOT_INTERRUPT6
	};

	slapshot_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_tc0140syt(*this, "tc0140syt"),
		m_tc0480scp(*this, "tc0480scp"),
		m_tc0360pri(*this, "tc0360pri"),
		m_tc0640fio(*this, "tc0640fio"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_spriteram(*this,"spriteram"),
		m_spriteext(*this,"spriteext") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<tc0140syt_device> m_tc0140syt;
	required_device<tc0480scp_device> m_tc0480scp;
	required_device<tc0360pri_device> m_tc0360pri;
	required_device<tc0640fio_device> m_tc0640fio;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	/* memory pointers */
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_spriteext;
	UINT16 *    m_spriteram_buffered;
	UINT16 *    m_spriteram_delayed;

	/* video-related */
	struct      slapshot_tempsprite *m_spritelist;
	INT32       m_sprites_disabled;
	INT32       m_sprites_active_area;
	INT32       m_sprites_master_scrollx;
	INT32       m_sprites_master_scrolly;
	int         m_sprites_flipscreen;
	int         m_prepare_sprites;
	int         m_dislayer[5];

	emu_timer *m_int6_timer;

	// generic
	DECLARE_READ16_MEMBER(service_input_r);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE16_MEMBER(msb_sound_w);
	DECLARE_READ16_MEMBER(msb_sound_r);

	// opwolf specific
	DECLARE_READ16_MEMBER(opwolf3_adc_r);
	DECLARE_WRITE16_MEMBER(opwolf3_adc_req_w);

	DECLARE_DRIVER_INIT(slapshot);
	virtual void machine_start();
	virtual void video_start();

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof_taito_no_buffer(screen_device &screen, bool state);
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int *primasks, int y_offset );
	void taito_handle_sprite_buffering(  );
	void taito_update_sprites_active_area(  );

	INTERRUPT_GEN_MEMBER(interrupt);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);
};
