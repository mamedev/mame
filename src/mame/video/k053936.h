#pragma once
#ifndef __K053936_H__
#define __K053936_H__


/* */


#define MCFG_K053936_ADD(_tag, _interface) \
	MCFG_DEVICE_ADD(_tag, K053936, 0) \
	MCFG_DEVICE_CONFIG(_interface)



void K053936_0_zoom_draw(screen_device &screen, bitmap_ind16 &bitmap,const rectangle &cliprect,tilemap_t *tmap,int flags,UINT32 priority, int glfgreat_hack);
void K053936_wraparound_enable(int chip, int status);
void K053936_set_offset(int chip, int xoffs, int yoffs);

// GX specific implementations...
void K053936GP_set_offset(int chip, int xoffs, int yoffs);
void K053936GP_clip_enable(int chip, int status);
void K053936GP_set_cliprect(int chip, int minx, int maxx, int miny, int maxy);
void K053936GP_0_zoom_draw(running_machine &machine, bitmap_rgb32 &bitmap, const rectangle &cliprect, tilemap_t *tmap, int tilebpp, int blend, int alpha, int pixeldouble_output, UINT16* temp_m_k053936_0_ctrl_16, UINT16* temp_m_k053936_0_linectrl_16, UINT16* temp_m_k053936_0_ctrl, UINT16* temp_m_k053936_0_linectrl, palette_device *palette);




struct k053936_interface
{
	int                m_wrap, m_xoff, m_yoff;
};



class k053936_device : public device_t,
										public k053936_interface
{
public:
	k053936_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k053936_device() {}

	DECLARE_WRITE16_MEMBER( ctrl_w );
	DECLARE_READ16_MEMBER( ctrl_r );
	DECLARE_WRITE16_MEMBER( linectrl_w );
	DECLARE_READ16_MEMBER( linectrl_r );
	void zoom_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, tilemap_t *tmap, int flags, UINT32 priority, int glfgreat_hack);
	// void wraparound_enable(int status);   unused? // shall we merge this into the configuration intf?
	// void set_offset(int xoffs, int yoffs); unused?   // shall we merge this into the configuration intf?

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	// internal state
	UINT16    *m_ctrl;
	UINT16    *m_linectrl;
};

extern const device_type K053936;




#endif
