#include "cpu/z80/z80.h"
#include "video/bufsprite.h"

class crshrace_state : public driver_device
{
public:
	crshrace_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_audiocpu(*this, "audiocpu"),
		  m_k053936(*this, "k053936"),
		  m_spriteram(*this, "spriteram"),
		  m_spriteram2(*this, "spriteram2") { }

	/* memory pointers */
	UINT16 *  m_videoram1;
	UINT16 *  m_videoram2;
//      UINT16 *  m_paletteram;   // currently this uses generic palette handling

	/* video-related */
	tilemap_t   *m_tilemap1;
	tilemap_t   *m_tilemap2;
	int       m_roz_bank;
	int       m_gfxctrl;
	int       m_flipscreen;

	/* misc */
	int m_pending_command;

	/* devices */
	required_device<z80_device> m_audiocpu;
	required_device<k053936_device> m_k053936;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_device<buffered_spriteram16_device> m_spriteram2;
	DECLARE_READ16_MEMBER(extrarom1_r);
	DECLARE_READ16_MEMBER(extrarom2_r);
	DECLARE_WRITE8_MEMBER(crshrace_sh_bankswitch_w);
	DECLARE_WRITE16_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(pending_command_clear_w);
	DECLARE_WRITE16_MEMBER(crshrace_videoram1_w);
	DECLARE_WRITE16_MEMBER(crshrace_videoram2_w);
	DECLARE_WRITE16_MEMBER(crshrace_roz_bank_w);
	DECLARE_WRITE16_MEMBER(crshrace_gfxctrl_w);
};

/*----------- defined in video/crshrace.c -----------*/


VIDEO_START( crshrace );
SCREEN_VBLANK( crshrace );
SCREEN_UPDATE_IND16( crshrace );
