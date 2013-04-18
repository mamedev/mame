#ifndef __ENTERP_H__
#define __ENTERP_H__


/* there are 64us per line, although in reality
   about 50 are visible. */
#define ENTERPRISE_SCREEN_WIDTH (50*16)

/* there are 312 lines per screen, although in reality
   about 35*8 are visible */
#define ENTERPRISE_SCREEN_HEIGHT    (35*8)


#define NICK_PALETTE_SIZE   256


struct NICK_STATE;

class ep_state : public driver_device
{
public:
	ep_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }

	UINT8 exdos_card_value;  /* state of the wd1770 irq/drq lines */
	UINT8 keyboard_line;     /* index of keyboard line to read */
	bitmap_ind16 m_bitmap;
	NICK_STATE *nick;
	DECLARE_READ8_MEMBER(exdos_card_r);
	DECLARE_WRITE8_MEMBER(exdos_card_w);
	DECLARE_WRITE8_MEMBER(epnick_reg_w);
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_epnick(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE8_MEMBER(enterprise_dave_reg_write);
	DECLARE_READ8_MEMBER(enterprise_dave_reg_read);
	DECLARE_WRITE_LINE_MEMBER(enterp_wd1770_intrq_w);
	DECLARE_WRITE_LINE_MEMBER(enterp_wd1770_drq_w);
	required_device<cpu_device> m_maincpu;
	void enterprise_update_memory_page(address_space &space, offs_t page, int index);
	char Nick_FetchByte(unsigned long Addr);
	void nick_write_pixel(int ci);
	void Nick_CalcVisibleClocks(int Width);
	void Nick_Init();
	void Nick_WriteBorder(int Clocks);
	void Nick_DoLeftMargin();
	void Nick_DoRightMargin();
	int Nick_GetColourIndex(int PenIndex);
	void Nick_WritePixels2Colour(unsigned char Pen0, unsigned char Pen1, unsigned char DataByte);
	void Nick_WritePixels2ColourLPIXEL(unsigned char Pen0, unsigned char Pen1, unsigned char DataByte);
	void Nick_WritePixels(unsigned char DataByte, unsigned char CharIndex);
	void Nick_WritePixelsLPIXEL(unsigned char DataByte, unsigned char CharIndex);
	void Nick_DoPixel(int ClocksVisible);
	void Nick_DoLPixel(int ClocksVisible);
	void Nick_DoAttr(int ClocksVisible);
	void Nick_DoCh256(int ClocksVisible);
	void Nick_DoCh128(int ClocksVisible);
	void Nick_DoCh64(int ClocksVisible);
	void Nick_DoDisplay();
	void Nick_UpdateLPT();
	void Nick_ReloadLPT();
	void Nick_DoLine();
	void Nick_DoScreen(bitmap_ind16 &bm);	
};


#endif /* __ENTERP_H__ */
