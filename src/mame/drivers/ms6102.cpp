// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    Elektronika MS6102.02 terminal

    https://goo.gl/photos/xJManS26QTxG1T7M7
        Schematics

    http://sfrolov.livejournal.com/110770.html
        Photos

    To do:
    - character attributes
    - improve keyboard response and add LED layout (MS7002)
    - verify CRTC clock

    Chips:
    - DD5 - KR580WM80A (8080 clone) - CPU
    - DD7 - KR580WT57 (8257 clone) - DMAC
    - DD9 - KR1601RR1 (1024x4 bit NVRAM)
    - DD21 - KR581WA1A (TR6402 clone) - UART
    - DD55, DD56 - KR580WG75 (8275 clone) - CRTC
    - DD59 - KR556RT5 - alternate chargen ROM
    - DD64 - K556RT4 - chargen layout table ROM
    - DD70 - K555RE4 - default chargen ROM
    - DD75 - KR580WI53 (8253 clone) - timer
    - DD76 - KR580WW51A (8251 clone) - UART
    - DD80 - K589IK14 (8214 clone) - interrupt control unit

****************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "machine/ay31015.h"
#include "machine/clock.h"
#include "machine/i8214.h"
#include "machine/i8251.h"
#include "machine/i8257.h"
#include "machine/kr1601rr1.h"
#include "machine/pit8253.h"
#include "machine/ripple_counter.h"
#include "machine/vt100_kbd.h"
#include "video/i8275.h"

#include "emupal.h"
#include "screen.h"


#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


class ms6102_state : public driver_device
{
public:
	ms6102_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_p_videoram(*this, "videoram")
		, m_maincpu(*this, "maincpu")
		, m_earom(*this, "earom")
		, m_pic(*this, "i8214")
		, m_dma8257(*this, "dma8257")
		, m_i8251(*this, "i8251")
		, m_rs232(*this, "rs232")
		, m_kbd_uart(*this, "589wa1")
		, m_keyboard(*this, "keyboard")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_crtc1(*this, "i8275_1")
		, m_crtc2(*this, "i8275_2")
		, m_p_chargen(*this, "chargen")
	{ }

	void ms6102(machine_config &config);
	void ms6102_io(address_map &map);
	void ms6102_mem(address_map &map);

protected:
	virtual void machine_reset() override;
	virtual void machine_start() override;

private:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	template <unsigned N> DECLARE_WRITE_LINE_MEMBER(irq) { m_pic->r_w(N, state ? 0 : 1); }

	I8275_DRAW_CHARACTER_MEMBER(display_pixels);
	I8275_DRAW_CHARACTER_MEMBER(display_attr);

	DECLARE_WRITE_LINE_MEMBER(hrq_w);
	DECLARE_WRITE_LINE_MEMBER(irq_w);

	void pic_w(u8 data);
	IRQ_CALLBACK_MEMBER(ms6102_int_ack);

	u8 memory_read_byte(offs_t offset);
	void vdack_w(u8 data);

	u8 crtc_r(offs_t offset);
	void crtc_w(offs_t offset, u8 data);

	u8 misc_status_r();
	u16 m_dmaaddr;

	void kbd_uart_clock_w(u8 data);

	required_shared_ptr<u8> m_p_videoram;
	required_device<i8080_cpu_device> m_maincpu;
	required_device<kr1601rr1_device> m_earom;
	required_device<i8214_device> m_pic;
	required_device<i8257_device> m_dma8257;
	required_device<i8251_device> m_i8251;
	required_device<rs232_port_device> m_rs232;
	required_device<ay31015_device> m_kbd_uart;
	required_device<vt100_keyboard_device> m_keyboard;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<i8275_device> m_crtc1;
	required_device<i8275_device> m_crtc2;
	required_region_ptr<u8> m_p_chargen;
};

void ms6102_state::ms6102_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x2fff).rom();
	map(0x3800, 0x3bff).rw(m_earom, FUNC(kr1601rr1_device::read), FUNC(kr1601rr1_device::write));
	map(0xc000, 0xffff).ram().share("videoram");
}

void ms6102_state::ms6102_io(address_map &map)
{
	map.unmap_value_high();
	map(0x00, 0x01).rw(m_i8251, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x10, 0x18).rw(m_dma8257, FUNC(i8257_device::read), FUNC(i8257_device::write));
	map(0x20, 0x23).rw("pit8253", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x30, 0x30).mirror(0x0f).rw("589wa1", FUNC(ay31015_device::receive), FUNC(ay31015_device::transmit));
	map(0x40, 0x41).rw(FUNC(ms6102_state::crtc_r), FUNC(ms6102_state::crtc_w));
	map(0x50, 0x5f).noprw(); // video disable?
	map(0x60, 0x6f).w(FUNC(ms6102_state::pic_w));
	map(0x70, 0x7f).r(FUNC(ms6102_state::misc_status_r));
}

static const gfx_layout ms6102_charlayout =
{
	8, 12,
	256,
	1,
	{ 0 },
	{ STEP8(1,1) },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8 },
	16*8
};

static GFXDECODE_START(gfx_ms6102)
	GFXDECODE_ENTRY("chargen", 0x0000, ms6102_charlayout, 0, 1)
GFXDECODE_END


WRITE_LINE_MEMBER(ms6102_state::hrq_w)
{
	/* FIXME: this should be connected to the HOLD line of 8080 */
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);

	/* FIXME: this should be connected to the HLDA line of 8080 */
	m_dma8257->hlda_w(state);
}

WRITE_LINE_MEMBER(ms6102_state::irq_w)
{
	m_maincpu->set_input_line(I8085_INTR_LINE, ASSERT_LINE);
}

u8 ms6102_state::memory_read_byte(offs_t offset)
{
	m_dmaaddr = offset;
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

I8275_DRAW_CHARACTER_MEMBER(ms6102_state::display_pixels)
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u8 gfx = (lten) ? 0xff : 0;
	if (!vsp)
		gfx = m_p_chargen[linecount | (charcode << 4)];

	if (rvv)
		gfx ^= 0xff;

	for(u8 i=0; i<8; i++)
		bitmap.pix(y, x + i) = palette[BIT(gfx, 7-i) ? (hlgt ? 2 : 1) : 0];
}

I8275_DRAW_CHARACTER_MEMBER(ms6102_state::display_attr) // TODO: attributes
{
}

u8 ms6102_state::crtc_r(offs_t offset)
{
	m_crtc2->read(offset);
	return m_crtc1->read(offset); // cs is same for both crtcs so they should return the same thing
}

void ms6102_state::crtc_w(offs_t offset, u8 data)
{
	m_crtc1->write(offset, data);
	m_crtc2->write(offset, data);
}

u8 ms6102_state::misc_status_r()
{
	u8 status = 0;
	if (!m_kbd_uart->tbmt_r())
		status |= 1 << 6;
	return status;
}

void ms6102_state::kbd_uart_clock_w(u8 data)
{
	m_kbd_uart->write_tcp(BIT(data, 1));
	m_kbd_uart->write_rcp(BIT(data, 1));

	if (data == 0 || data == 3)
		m_keyboard->signal_line_w(m_kbd_uart->so_r());
	else
		m_keyboard->signal_line_w(BIT(data, 0));
}


void ms6102_state::pic_w(u8 data)
{
	m_pic->b_sgs_w(~data);
}

void ms6102_state::vdack_w(u8 data)
{
	if(m_dmaaddr & 1)
		m_crtc1->dack_w(data);
	else
		m_crtc2->dack_w(data | 0x80);
}

IRQ_CALLBACK_MEMBER(ms6102_state::ms6102_int_ack)
{
	m_maincpu->set_input_line(I8085_INTR_LINE, CLEAR_LINE);
	return 0xc7 | ((m_pic->a_r() ^ 7) << 3);
}


void ms6102_state::machine_reset()
{
}

void ms6102_state::machine_start()
{
	m_kbd_uart->write_eps(1);
	m_kbd_uart->write_nb1(1);
	m_kbd_uart->write_nb2(1);
	m_kbd_uart->write_tsb(0);
	m_kbd_uart->write_np(1);
	m_kbd_uart->write_cs(1);
	m_kbd_uart->write_swe(0);

	m_i8251->write_cts(0);

	m_pic->etlg_w(1);

	// rearrange the chargen to be easier for us to access
	int i,j;
	for (i = 0; i < 0x100; i++)
		for (j = 0; j < 2; j++)
			m_p_chargen[0x1800+i*8+j+6] = m_p_chargen[0x1000+i*8+j];
	for (i = 0; i < 0x100; i++)
		for (j = 2; j < 8; j++)
			m_p_chargen[0x1800+i*8+j-2] = m_p_chargen[0x1000+i*8+j];
	// since we don't know which codes are for the russian symbols, give each unused char a unique marker
	for (i = 0; i < 256; i++)
		m_p_chargen[i*16] = i;
	// copy over the ascii chars into their new positions (lines 0-7)
	for (i = 0x20; i < 0x80; i++)
		for (j = 0; j < 8; j++)
			m_p_chargen[i*16+j+1] = m_p_chargen[0x1800+i*8+j];
	// copy the russian symbols to codes 0xc0-0xff for now
	for (i = 0xc0; i < 0x100; i++)
		for (j = 0; j < 8; j++)
			m_p_chargen[i*16+j+1] = m_p_chargen[0x1800+i*8+j];
	// for punctuation, get the last 4 lines into place
	for (i = 0x20; i < 0x40; i++)
		for (j = 0; j < 4; j++)
			m_p_chargen[i*16+8+j+1] = m_p_chargen[0x1700+i*8+j];
	// for letters, get the last 4 lines into place
	for (i = 0x40; i < 0x80; i++)
		for (j = 0; j < 4; j++)
			m_p_chargen[i*16+8+j+1] = m_p_chargen[0x1a00+i*8+j];
	// for russian, get the last 4 lines into place
	for (i = 0xc0; i < 0x100; i++)
		for (j = 0; j < 4; j++)
			m_p_chargen[i*16+8+j+1] = m_p_chargen[0x1604+i*8+j];
}


void ms6102_state::ms6102(machine_config &config)
{
	I8080(config, m_maincpu, XTAL(18'432'000) / 9);
	m_maincpu->set_addrmap(AS_PROGRAM, &ms6102_state::ms6102_mem);
	m_maincpu->set_addrmap(AS_IO, &ms6102_state::ms6102_io);
	m_maincpu->out_inte_func().set("i8214", FUNC(i8214_device::inte_w));
	m_maincpu->set_irq_acknowledge_callback(FUNC(ms6102_state::ms6102_int_ack));

	I8214(config, m_pic, XTAL(18'432'000) / 9);
	m_pic->int_wr_callback().set(FUNC(ms6102_state::irq_w));

	KR1601RR1(config, m_earom, 0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_screen_update("i8275_1", FUNC(i8275_device::screen_update));
	m_screen->set_raw(XTAL(16'400'000), 784, 0, 80*8, 375, 0, 25*12);

	GFXDECODE(config, "gfxdecode", m_palette, gfx_ms6102);
	PALETTE(config, m_palette, palette_device::MONOCHROME_HIGHLIGHT);

	I8257(config, m_dma8257, XTAL(18'432'000) / 9);
	m_dma8257->out_hrq_cb().set(FUNC(ms6102_state::hrq_w));
	m_dma8257->in_memr_cb().set(FUNC(ms6102_state::memory_read_byte));
	m_dma8257->out_iow_cb<2>().set(FUNC(ms6102_state::vdack_w));

	I8275(config, m_crtc1, XTAL(16'400'000) / 8);
	m_crtc1->set_character_width(8);
	m_crtc1->set_display_callback(FUNC(ms6102_state::display_pixels));
	m_crtc1->drq_wr_callback().set("dma8257", FUNC(i8257_device::dreq2_w));
	m_crtc1->set_screen(m_screen);

	I8275(config, m_crtc2, XTAL(16'400'000) / 8);
	m_crtc2->set_character_width(8);
	m_crtc2->set_display_callback(FUNC(ms6102_state::display_attr));
	m_crtc2->irq_wr_callback().set(FUNC(ms6102_state::irq<5>));
	m_crtc2->set_screen(m_screen);

	// keyboard
	AY31015(config, m_kbd_uart);
	m_kbd_uart->write_dav_callback().set(FUNC(ms6102_state::irq<1>));
	m_kbd_uart->set_auto_rdav(true);

	ripple_counter_device &ie5(RIPPLE_COUNTER(config, "ie5", XTAL(16'400'000) / 30));
	ie5.set_stages(2);
	ie5.count_out_cb().set(FUNC(ms6102_state::kbd_uart_clock_w));

	MS7002(config, m_keyboard, 0).signal_out_callback().set(m_kbd_uart, FUNC(ay31015_device::write_si));

	// serial connection to host
	I8251(config, m_i8251, 0);
	m_i8251->txd_handler().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_i8251->rxrdy_handler().set(FUNC(ms6102_state::irq<3>));

	RS232_PORT(config, m_rs232, default_rs232_devices, "null_modem");
	m_rs232->rxd_handler().set(m_i8251, FUNC(i8251_device::write_rxd));

	pit8253_device &pit8253(PIT8253(config, "pit8253", 0));
	pit8253.set_clk<0>(XTAL(16'400'000) / 9);
	pit8253.out_handler<0>().set(m_i8251, FUNC(i8251_device::write_txc));
	pit8253.set_clk<1>(XTAL(16'400'000) / 9);
	pit8253.out_handler<1>().set(m_i8251, FUNC(i8251_device::write_rxc));
}

ROM_START( ms6102 )
	ROM_REGION(0x3000, "maincpu", 0)
	ROM_LOAD("mc6102_02_ks573rf2_dd26",   0x0000, 0x0800, CRC(f96ba806) SHA1(60d155b781e97e86d31dc2194ad367030470eeb6))
	ROM_LOAD("mc6102_02_ks573rf2_dd30",   0x0800, 0x0800, CRC(1d69ba62) SHA1(bf7d19400fe606239ce8a057850cf4c63ff4cdb2))
	ROM_LOAD("mc6102_02_ks573rf2_0034",   0x1000, 0x0800, CRC(4bce121a) SHA1(e97c635c2fab70a71a31db3b53284209b5881f2c))
	ROM_LOAD("mc6102_02_ks573rf2_0037",   0x1800, 0x0800, CRC(1b22543f) SHA1(fc6cc54baf3abadca30dfaf39a50cae7fbf601b2))
	ROM_LOAD("mc6102_02_ks573rf2_0045",   0x2000, 0x0800, CRC(fd741cfe) SHA1(153abb57ca4833286811082ff50c7b36136274dc))
	ROM_LOAD("mc6102_02_ks573rf2_dd49",   0x2800, 0x0800, CRC(748f6cee) SHA1(a35e6495ea108824f2f1f9907f5e651174e9cf15))

	ROM_REGION(0x2000, "chargen", ROMREGION_ERASE00)
	ROM_LOAD("mc6102_02_k555re4_chargen", 0x1000, 0x0800, CRC(b0e3546b) SHA1(25aca264cc64f368ffcefdfd356120a314a44947))

	ROM_REGION(0x0100, "charmap", 0)
	ROM_LOAD("mc6102_02_k556rt4_d64",     0x0000, 0x0100, CRC(a59fdaa7) SHA1(0851a8b12e838e8f7e5ce840a0262facce303442))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  CLASS         INIT        COMPANY        FULLNAME      FLAGS */
COMP( 1984, ms6102, 0,      0,      ms6102,  0,     ms6102_state, empty_init, "Elektronika", "MS 6102.02", MACHINE_NOT_WORKING )
