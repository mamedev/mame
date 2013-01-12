/*****************************************************************************
 *
 * includes/odyssey2.h
 *
 ****************************************************************************/

#ifndef ODYSSEY2_H_
#define ODYSSEY2_H_

#include "machine/i8243.h"
#include "video/i8244.h"


#define P1_BANK_LO_BIT            (0x01)
#define P1_BANK_HI_BIT            (0x02)
#define P1_KEYBOARD_SCAN_ENABLE   (0x04)  /* active low */
#define P1_VDC_ENABLE             (0x08)  /* active low */
#define P1_EXT_RAM_ENABLE         (0x10)  /* active low */
#define P1_VDC_COPY_MODE_ENABLE   (0x40)

#define P2_KEYBOARD_SELECT_MASK   (0x07)  /* select row to scan */

#define I824X_START_ACTIVE_SCAN         6
#define I824X_END_ACTIVE_SCAN           (6 + 160)
#define I824X_START_Y                   1
#define I824X_SCREEN_HEIGHT             243
#define I824X_LINE_CLOCKS               228

struct ef9341_t
{
	UINT8   TA;
	UINT8   TB;
	UINT8   busy;
};

struct ef9340_t
{
	UINT8   X;
	UINT8   Y;
	UINT8   Y0;
	UINT8   R;
	UINT8   M;
};

class odyssey2_state : public driver_device
{
public:
	odyssey2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_i8243(*this, "i8243")
		, m_i8244(*this, "i8244")
		, m_g7400(false)
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	optional_device<i8243_device> m_i8243;
	required_device<i8244_device> m_i8244;

	int m_the_voice_lrq_state;
	UINT8 *m_ram;
	UINT8 m_p1;
	UINT8 m_p2;
	size_t m_cart_size;
	bitmap_ind16 m_tmp_bitmap;
	int m_start_vpos;
	int m_start_vblank;
	UINT8 m_lum;
	DECLARE_READ8_MEMBER(t0_read);
	DECLARE_READ8_MEMBER(io_read);
	DECLARE_WRITE8_MEMBER(io_write);
	DECLARE_READ8_MEMBER(bus_read);
	DECLARE_WRITE8_MEMBER(bus_write);
	DECLARE_READ8_MEMBER(g7400_io_read);
	DECLARE_WRITE8_MEMBER(g7400_io_write);
	DECLARE_READ8_MEMBER(p1_read);
	DECLARE_WRITE8_MEMBER(p1_write);
	DECLARE_READ8_MEMBER(p2_read);
	DECLARE_WRITE8_MEMBER(p2_write);
	DECLARE_READ8_MEMBER(t1_read);
	DECLARE_DRIVER_INIT(odyssey2);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	void video_start_g7400();
	virtual void palette_init();
	UINT32 screen_update_odyssey2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(the_voice_lrq_callback);
	DECLARE_WRITE8_MEMBER(i8243_port_w);
	DECLARE_WRITE_LINE_MEMBER(irq_callback);

	void ef9341_w( UINT8 command, UINT8 b, UINT8 data );
	UINT8 ef9341_r( UINT8 command, UINT8 b );

	DECLARE_WRITE16_MEMBER(scanline_postprocess);

protected:
	ef9340_t m_ef9340;
	ef9341_t m_ef9341;
	UINT8   m_ef934x_ram_a[1024];
	UINT8   m_ef934x_ram_b[1024];
	UINT8   m_ef934x_ext_char_ram[1024];
	bool    m_g7400;
	UINT8   m_g7400_ic674_decode[8];
	UINT8   m_g7400_ic678_decode[8];

	inline UINT16 ef9340_get_c_addr(UINT8 x, UINT8 y);
	inline void ef9340_inc_c();
	// Calculate the external chargen address for a character and slice
	inline UINT16 external_chargen_address(UINT8 b, UINT8 slice);

	void ef9340_scanline(int vpos);

	/* timers */
	static const device_timer_id TIMER_LINE = 0;
	static const device_timer_id TIMER_HBLANK = 1;

	emu_timer *m_line_timer;
	emu_timer *m_hblank_timer;

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	void switch_banks();
};

#endif /* ODYSSEY2_H_ */
