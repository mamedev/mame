
#include "emu.h"
#include "cpu/z80/z80.h"

class zodiack_state : public driver_device
{
public:
	zodiack_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_audiocpu(*this, "audiocpu"),
		  m_videoram(*this, "videoram"),
		  m_videoram_2(*this, "videoram_2"),
		  m_attributeram(*this, "attributeram"),
		  m_spriteram(*this, "spriteram"),
		  m_bulletsram(*this, "bulletsram"),
		  m_videoram_size(*this, "videoram"),
		  m_spriteram_size(*this, "spriteram"),
		  m_bulletsram_size(*this, "bulletsram")
	{ m_percuss_hardware = 0; }

	// in drivers/zodiack.c
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(sound_nmi_enable_w);
	DECLARE_WRITE8_MEMBER(master_soundlatch_w);
	DECLARE_WRITE8_MEMBER(control_w);

	// in video/zodiack.c
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(videoram2_w);
	DECLARE_WRITE8_MEMBER(attributes_w);
	DECLARE_WRITE8_MEMBER(flipscreen_w);

	void draw_bullets(bitmap_t *bitmap, const rectangle *cliprect);
	void draw_sprites(bitmap_t *bitmap, const rectangle *cliprect);

	// devices
	required_device<z80_device> m_maincpu;
	required_device<z80_device> m_audiocpu;

	// shared pointers
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_videoram_2;
	required_shared_ptr<UINT8> m_attributeram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_bulletsram;
	required_shared_size m_videoram_size;
	required_shared_size m_spriteram_size;
	required_shared_size m_bulletsram_size;
	// currently this driver uses generic palette handling

	// state
	// video-related
	tilemap_t   *m_bg_tilemap;
	tilemap_t   *m_fg_tilemap;

	// sound-related
	UINT8     m_nmi_enable;
	UINT8     m_sound_nmi_enabled;

	// misc
	int       m_percuss_hardware;

protected:

	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
	virtual bool screen_update(screen_device &screen, bitmap_t &bitmap, const rectangle &cliprect);
};

class percuss_state : public zodiack_state
{
public:
	percuss_state(const machine_config &mconfig, device_type type, const char *tag)
		: zodiack_state(mconfig, type, tag)
	{ m_percuss_hardware = 1; }
};

// in video/zodiack.c
PALETTE_INIT( zodiack );
