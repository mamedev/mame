#ifndef __ENTERP_H__
#define __ENTERP_H__


/* there are 64us per line, although in reality
   about 50 are visible. */
#define ENTERPRISE_SCREEN_WIDTH (50*16)

/* there are 312 lines per screen, although in reality
   about 35*8 are visible */
#define ENTERPRISE_SCREEN_HEIGHT	(35*8)


#define NICK_PALETTE_SIZE	256


struct NICK_STATE;

class ep_state : public driver_device
{
public:
	ep_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 exdos_card_value;  /* state of the wd1770 irq/drq lines */
	UINT8 keyboard_line;     /* index of keyboard line to read */
	bitmap_ind16 m_bitmap;
	NICK_STATE *nick;
	DECLARE_READ8_MEMBER(exdos_card_r);
	DECLARE_WRITE8_MEMBER(exdos_card_w);
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_epnick(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE8_MEMBER(enterprise_dave_reg_write);
	DECLARE_READ8_MEMBER(enterprise_dave_reg_read);
	DECLARE_WRITE_LINE_MEMBER(enterp_wd1770_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(enterp_wd1770_drq_w);
};


/*----------- defined in video/epnick.c -----------*/
DECLARE_WRITE8_HANDLER( epnick_reg_w );

#endif /* __ENTERP_H__ */
