/*************************************************************************

    Konami Battlantis Hardware

*************************************************************************/

#include "video/k007342.h"
#include "video/k007420.h"

class battlnts_state : public driver_device
{
public:
	battlnts_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu"),
		m_k007342(*this, "k007342"),
		m_k007420(*this, "k007420"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	/* memory pointers */
//  UINT8 *      paletteram;    // this currently uses generic palette handling

	/* video-related */
	int m_spritebank;
	int m_layer_colorbase[2];


	/* devices */
	required_device<cpu_device> m_audiocpu;
	required_device<k007342_device> m_k007342;
	required_device<k007420_device> m_k007420;
	DECLARE_WRITE8_MEMBER(battlnts_sh_irqtrigger_w);
	DECLARE_WRITE8_MEMBER(battlnts_bankswitch_w);
	DECLARE_WRITE8_MEMBER(battlnts_spritebank_w);
	DECLARE_DRIVER_INIT(rackemup);
	virtual void machine_start();
	virtual void machine_reset();
	UINT32 screen_update_battlnts(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(battlnts_interrupt);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	K007342_CALLBACK_MEMBER(battlnts_tile_callback);
	K007420_CALLBACK_MEMBER(battlnts_sprite_callback);
};
