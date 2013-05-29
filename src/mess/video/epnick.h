#ifndef __EPNICK_H__
#define __EPNICK_H__



/* there are 64us per line, although in reality
 about 50 are visible. */
#define ENTERPRISE_SCREEN_WIDTH (50*16)

/* there are 312 lines per screen, although in reality
 about 35*8 are visible */
#define ENTERPRISE_SCREEN_HEIGHT    (35*8)



/* Nick executes a Display list, in the form of a list of Line Parameter
 Tables, this is the form of the data */
struct LPT_ENTRY
{
	UINT8 SC;       /* scanlines in this modeline (two's complement) */
	UINT8 MB;       /* the MODEBYTE (defines video display mode) */
	UINT8 LM;       /* left margin etc */
	UINT8 RM;       /* right margin etc */
	UINT8 LD1L;     /* (a7..a0) of line data pointer LD1 */
	UINT8 LD1H;     /* (a8..a15) of line data pointer LD1 */
	UINT8 LD2L;     /* (a7..a0) of line data pointer LD2 */
	UINT8 LD2H;     /* (a8..a15) of line data pointer LD2 */
	UINT8 COL[8];   /* COL0..COL7 */
};


class nick_device : public device_t
{
public:
	nick_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~nick_device();

	void set_vram(UINT8 *vram) { m_videoram = vram; }
	DECLARE_WRITE8_MEMBER(reg_w);

	UINT32 screen_update_epnick(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	inline UINT8 fetch_byte(UINT32 offs) { return m_videoram[offs & 0x0ffff]; }
	void write_pixel(int ci);
	void calc_visible_clocks(int width);
	void init();
	void write_border(int clocks);
	void do_left_margin();
	void do_right_margin();

	int get_color_index(int pen_index);
	void write_pixels2color(UINT8 pen0, UINT8 pen1, UINT8 data_byte);
	void write_pixels2color_lpixel(UINT8 pen0, UINT8 pen1, UINT8 data_byte);
	void write_pixels(UINT8 data_byte, UINT8 char_idx);
	void write_pixels_lpixel(UINT8 data_byte, UINT8 char_idx);

	void do_pixel(int clocks_visible);
	void do_lpixel(int clocks_visible);
	void do_attr(int clocks_visible);
	void do_ch256(int clocks_visible);
	void do_ch128(int clocks_visible);
	void do_ch64(int clocks_visible);
	void do_display();
	void update_lpt();
	void reload_lpt();
	void do_line();
	void do_screen(bitmap_ind16 &bm);
	
	bitmap_ind16 m_bitmap;

	/* horizontal position */
	UINT8 horizontal_clock;
	/* current scanline within LPT */
	UINT8 m_scanline_count;
	
	UINT8 m_FIXBIAS;
	UINT8 m_BORDER;
	UINT8 m_LPL;
	UINT8 m_LPH;
	
	UINT16 m_LD1;
	UINT16 m_LD2;
	
	LPT_ENTRY   m_LPT;
	
	UINT16 *m_dest;
	int m_dest_pos;
	int m_dest_max_pos;
	
	UINT8 m_reg[16];
	
	/* first clock visible on left hand side */
	int m_first_visible_clock;
	/* first clock visible on right hand side */
	int m_last_visible_clock;
	
	/* given a bit pattern, this will get the pen index */
	UINT8 m_pen_idx_4col[256];
	/* given a bit pattern, this will get the pen index */
	UINT8 m_pen_idx_16col[256];
	
	UINT8 *m_videoram;
};



// device type definition
extern const device_type NICK;


/***************************************************************************
 DEVICE CONFIGURATION MACROS
 ***************************************************************************/

#define MCFG_NICK_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, NICK, 0)


#endif
