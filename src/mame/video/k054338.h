
#pragma once
#ifndef __K054338_H__
#define __K054338_H__

#include "k055555.h"

#define MCFG_K054338_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K054338, 0) \
	MCFG_DEVICE_CONFIG(_interface)



/* K054338 mixer/alpha blender */
void K054338_vh_start(running_machine &machine, k055555_device* k055555);
DECLARE_WRITE16_HANDLER( K054338_word_w ); // "CLCT" registers
DECLARE_WRITE32_HANDLER( K054338_long_w );
int K054338_read_register(int reg);
void K054338_update_all_shadows(running_machine &machine, int rushingheroes_hack, palette_device *palette);          // called at the beginning of SCREEN_UPDATE()
void K054338_fill_solid_bg(bitmap_ind16 &bitmap);               // solid backcolor fill
void K054338_fill_backcolor(running_machine &machine, screen_device &screen, bitmap_rgb32 &bitmap, int mode);  // unified fill, 0=solid, 1=gradient
int  K054338_set_alpha_level(int pblend);                           // blend style 0-2
void K054338_invert_alpha(int invert);                              // 0=0x00(invis)-0x1f(solid), 1=0x1f(invis)-0x00(solod)
void K054338_export_config(int **shdRGB);

#define K338_REG_BGC_R      0
#define K338_REG_BGC_GB     1
#define K338_REG_SHAD1R     2
#define K338_REG_BRI3       11
#define K338_REG_PBLEND     13
#define K338_REG_CONTROL    15

#define K338_CTL_KILL       0x01    /* 0 = no video output, 1 = enable */
#define K338_CTL_MIXPRI     0x02
#define K338_CTL_SHDPRI     0x04
#define K338_CTL_BRTPRI     0x08
#define K338_CTL_WAILSL     0x10
#define K338_CTL_CLIPSL     0x20


struct k054338_interface
{
	int                m_alpha_inv;
	const char         *m_k055555_tag;
};


class k054338_device : public device_t,
										public device_video_interface,
										public k054338_interface
{
public:
	k054338_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k054338_device() {}

	DECLARE_WRITE16_MEMBER( word_w ); // "CLCT" registers
	DECLARE_WRITE32_MEMBER( long_w );

	DECLARE_READ16_MEMBER( word_r );        // CLTC

	int register_r(int reg);
	void update_all_shadows(int rushingheroes_hack, palette_device *palette);          // called at the beginning of SCREEN_UPDATE()
	void fill_solid_bg(bitmap_rgb32 &bitmap);             // solid backcolor fill
	void fill_backcolor(bitmap_rgb32 &bitmap, int mode);  // unified fill, 0=solid, 1=gradient (by using a k055555)
	int  set_alpha_level(int pblend);                         // blend style 0-2
	void invert_alpha(int invert);                                // 0=0x00(invis)-0x1f(solid), 1=0x1f(invis)-0x00(solod)
	//void export_config(int **shdRGB);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state
	UINT16    m_regs[32];
	int       m_shd_rgb[9];

	k055555_device *m_k055555;  /* used to fill BG color */
};

extern const device_type K054338;

#endif
