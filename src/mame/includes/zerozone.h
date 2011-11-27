/*************************************************************************

    Zero Zone

*************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"

class zerozone_state : public driver_device
{
public:
	zerozone_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_audiocpu(*this, "audiocpu"),
		  m_vram(*this, "videoram"),
		  m_vram_size(*this, "videoram")
	{ }

	// in drivers/zerozone.c
	DECLARE_WRITE16_MEMBER(sound_w);

	// in video/zerozone.c
	DECLARE_WRITE16_MEMBER(tilemap_w);
	DECLARE_WRITE16_MEMBER(tilebank_w);

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<z80_device> m_audiocpu;

	// shared pointers
	required_shared_ptr<UINT16> m_vram;
	required_shared_size m_vram_size;
	// currently this driver uses generic palette handling

	// state
	// video-related
	UINT16         m_tilebank;
	tilemap_t     *m_zz_tilemap;

protected:

	// driver_device overrides
	virtual void machine_start();
	virtual void machine_reset();

	virtual void video_start();
	virtual bool screen_update(screen_device &screen, bitmap_t &bitmap, const rectangle &cliprect);
};
