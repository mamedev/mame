/*****************************************************************************
 *
 * includes/zx.h
 *
 ****************************************************************************/

#ifndef ZX_H_
#define ZX_H_

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/speaker.h"
#include "sound/wave.h"
#include "imagedev/cassette.h"
#include "formats/zx81_p.h"
#include "machine/ram.h"


class zx_state : public driver_device
{
public:
	zx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	bitmap_ind16 m_bitmap;

	DECLARE_READ8_MEMBER(zx_ram_r);
	DECLARE_READ8_MEMBER(zx80_io_r);
	DECLARE_READ8_MEMBER(zx81_io_r);
	DECLARE_READ8_MEMBER(pc8300_io_r);
	DECLARE_READ8_MEMBER(pow3000_io_r);
	DECLARE_WRITE8_MEMBER(zx80_io_w);
	DECLARE_WRITE8_MEMBER(zx81_io_w);
	emu_timer *m_ula_nmi;
	int m_ula_irq_active;
	int m_ula_frame_vsync;
	int m_ula_scanline_count;
	UINT8 m_tape_bit;
	UINT8 m_speaker_state;
	int m_old_x;
	int m_old_y;
	UINT8 m_old_c;
	UINT8 m_charline[32];
	UINT8 m_charline_ptr;
	int m_offs1;
	void zx_ula_bkgnd(UINT8 color);
	DECLARE_WRITE8_MEMBER(zx_ram_w);
	DECLARE_DIRECT_UPDATE_MEMBER(zx_setdirect);
	DECLARE_DIRECT_UPDATE_MEMBER(pc8300_setdirect);
	DECLARE_DIRECT_UPDATE_MEMBER(pow3000_setdirect);
	DECLARE_DRIVER_INIT(zx);
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	DECLARE_PALETTE_INIT(ts1000);
	DECLARE_MACHINE_RESET(pc8300);
	DECLARE_MACHINE_RESET(pow3000);
};


/*----------- defined in machine/zx.c -----------*/





/*----------- defined in video/zx.c -----------*/


SCREEN_VBLANK( zx );

void zx_ula_bkgnd(running_machine &machine, int color);
void zx_ula_r(running_machine &machine, int offs, const char *region, const UINT8 param);

//extern int ula_nmi_active;
//extern int ula_scancode_count;


#endif /* ZX_H_ */
