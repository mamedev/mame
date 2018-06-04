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

#include "screen.h"

#define LOG_GENERAL (1U <<  0)

#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC printf
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

	template <unsigned N> DECLARE_WRITE_LINE_MEMBER(irq) { m_pic->r_w(N, state?0:1); }

	I8275_DRAW_CHARACTER_MEMBER(display_pixels);
	I8275_DRAW_CHARACTER_MEMBER(display_attr);

	DECLARE_WRITE_LINE_MEMBER(hrq_w);
	DECLARE_WRITE_LINE_MEMBER(irq_w);

	DECLARE_WRITE8_MEMBER(pic_w);
	IRQ_CALLBACK_MEMBER(ms6102_int_ack);

	DECLARE_READ8_MEMBER(memory_read_byte);
	DECLARE_WRITE8_MEMBER(vdack_w);

	DECLARE_READ8_MEMBER(crtc_r);
	DECLARE_WRITE8_MEMBER(crtc_w);

	DECLARE_READ8_MEMBER(misc_status_r);
	u16 m_dmaaddr;

	DECLARE_WRITE8_MEMBER(kbd_uart_clock_w);

	required_shared_ptr<uint8_t> m_p_videoram;
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
	map(0x00, 0x00).rw(m_i8251, FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0x01, 0x01).rw(m_i8251, FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0x10, 0x18).rw(m_dma8257, FUNC(i8257_device::read), FUNC(i8257_device::write));
	map(0x20, 0x23).rw("pit8253", FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x30, 0x30).mirror(0x0f).rw("589wa1", FUNC(ay31015_device::receive), FUNC(ay31015_device::transmit));
	map(0x40, 0x41).rw(this, FUNC(ms6102_state::crtc_r), FUNC(ms6102_state::crtc_w));
	map(0x50, 0x5f).noprw(); // video disable?
	map(0x60, 0x6f).w(this, FUNC(ms6102_state::pic_w));
	map(0x70, 0x7f).r(this, FUNC(ms6102_state::misc_status_r));
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
	/* HACK - this should be connected to the HOLD line of 8080 */
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);

	/* HACK - this should be connected to the HLDA line of 8080 */
	m_dma8257->hlda_w(state);
}

WRITE_LINE_MEMBER(ms6102_state::irq_w)
{
	m_maincpu->set_input_line(I8085_INTR_LINE, ASSERT_LINE);
}

READ8_MEMBER(ms6102_state::memory_read_byte)
{
	m_dmaaddr = offset;
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

I8275_DRAW_CHARACTER_MEMBER(ms6102_state::display_pixels)
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	u8 gfx = (lten) ? 0xff : 0;
	if (!vsp)
		gfx = m_p_chargen[linecount | (charcode << 4)];

	if (rvv)
		gfx ^= 0xff;

	for(u8 i=0; i<8; i++)
		bitmap.pix32(y, x + i) = palette[BIT(gfx, 7-i) ? (hlgt ? 2 : 1) : 0];
}

I8275_DRAW_CHARACTER_MEMBER(ms6102_state::display_attr) // TODO: attributes
{
}

READ8_MEMBER(ms6102_state::crtc_r)
{
	m_crtc2->read(space, offset);
	return m_crtc1->read(space, offset); // cs is same for both crtcs so they should return the same thing
}

WRITE8_MEMBER(ms6102_state::crtc_w)
{
	m_crtc1->write(space, offset, data);
	m_crtc2->write(space, offset, data);
}

READ8_MEMBER(ms6102_state::misc_status_r)
{
	uint8_t status = 0;
	if (!m_kbd_uart->tbmt_r())
		status |= 1 << 6;
	return status;
}

WRITE8_MEMBER(ms6102_state::kbd_uart_clock_w)
{
	m_kbd_uart->write_tcp(BIT(data, 1));
	m_kbd_uart->write_rcp(BIT(data, 1));

	if (data == 0 || data == 3)
		m_keyboard->signal_line_w(m_kbd_uart->so_r());
	else
		m_keyboard->signal_line_w(BIT(data, 0));
}


WRITE8_MEMBER(ms6102_state::pic_w)
{
	m_pic->b_w((data & 7) ^ 7);
	m_pic->sgs_w(BIT(data, 3) ^ 1);
}

WRITE8_MEMBER(ms6102_state::vdack_w)
{
	if(m_dmaaddr & 1)
		m_crtc1->dack_w(space, offset, data);
	else
		m_crtc2->dack_w(space, offset, data | 0x80);
}

IRQ_CALLBACK_MEMBER(ms6102_state::ms6102_int_ack)
{
	m_maincpu->set_input_line(I8085_INTR_LINE, CLEAR_LINE);
	return 0xc7 | (m_pic->a_r() << 3);
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


MACHINE_CONFIG_START(ms6102_state::ms6102)
	MCFG_DEVICE_ADD("maincpu", I8080, XTAL(18'432'000) / 9)
	MCFG_DEVICE_PROGRAM_MAP(ms6102_mem)
	MCFG_DEVICE_IO_MAP(ms6102_io)
	MCFG_I8085A_INTE(WRITELINE("i8214", i8214_device, inte_w))
	MCFG_DEVICE_IRQ_ACKNOWLEDGE_DRIVER(ms6102_state, ms6102_int_ack)

	MCFG_DEVICE_ADD("i8214", I8214, XTAL(18'432'000) / 9)
	MCFG_I8214_INT_CALLBACK(WRITELINE(*this, ms6102_state, irq_w))

	MCFG_DEVICE_ADD("earom", KR1601RR1, 0)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_UPDATE_DEVICE("i8275_1", i8275_device, screen_update)
	MCFG_SCREEN_RAW_PARAMS(XTAL(16'400'000), 784, 0, 80*8, 375, 0, 25*12)
	MCFG_DEVICE_ADD("gfxdecode", GFXDECODE, "palette", gfx_ms6102)
	MCFG_PALETTE_ADD_MONOCHROME_HIGHLIGHT("palette")

	MCFG_DEVICE_ADD("dma8257", I8257, XTAL(18'432'000) / 9)
	MCFG_I8257_OUT_HRQ_CB(WRITELINE(*this, ms6102_state, hrq_w))
	MCFG_I8257_IN_MEMR_CB(READ8(*this, ms6102_state, memory_read_byte))
	MCFG_I8257_OUT_IOW_2_CB(WRITE8(*this, ms6102_state, vdack_w))

	MCFG_DEVICE_ADD("i8275_1", I8275, XTAL(16'400'000) / 8) // XXX
	MCFG_I8275_CHARACTER_WIDTH(8)
	MCFG_I8275_DRAW_CHARACTER_CALLBACK_OWNER(ms6102_state, display_pixels)
	MCFG_I8275_DRQ_CALLBACK(WRITELINE("dma8257", i8257_device, dreq2_w))
	MCFG_VIDEO_SET_SCREEN("screen")

	MCFG_DEVICE_ADD("i8275_2", I8275, XTAL(16'400'000) / 8) // XXX
	MCFG_I8275_CHARACTER_WIDTH(8)
	MCFG_I8275_DRAW_CHARACTER_CALLBACK_OWNER(ms6102_state, display_attr)
	MCFG_I8275_IRQ_CALLBACK(WRITELINE(*this, ms6102_state, irq<5>))
	MCFG_VIDEO_SET_SCREEN("screen")

	// keyboard
	MCFG_DEVICE_ADD("589wa1", AY31015, 0)
	MCFG_AY31015_WRITE_DAV_CB(WRITELINE(*this, ms6102_state, irq<1>))
	MCFG_AY31015_AUTO_RDAV(true)

	MCFG_DEVICE_ADD("ie5", RIPPLE_COUNTER, XTAL(16'400'000) / 30)
	MCFG_RIPPLE_COUNTER_STAGES(2)
	MCFG_RIPPLE_COUNTER_COUNT_OUT_CB(WRITE8(*this, ms6102_state, kbd_uart_clock_w))

	MCFG_DEVICE_ADD("keyboard", MS7002, 0)
	MCFG_VT100_KEYBOARD_SIGNAL_OUT_CALLBACK(WRITELINE("589wa1", ay31015_device, write_si))

	// serial connection to host
	MCFG_DEVICE_ADD("i8251", I8251, 0)
	MCFG_I8251_TXD_HANDLER(WRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_I8251_RXRDY_HANDLER(WRITELINE(*this, ms6102_state, irq<3>))

	MCFG_DEVICE_ADD("rs232", RS232_PORT, default_rs232_devices, "null_modem")
	MCFG_RS232_RXD_HANDLER(WRITELINE("i8251", i8251_device, write_rxd))

	MCFG_DEVICE_ADD("pit8253", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL(16'400'000) / 9)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE("i8251", i8251_device, write_txc))
	MCFG_PIT8253_CLK1(XTAL(16'400'000) / 9)
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE("i8251", i8251_device, write_rxc))
MACHINE_CONFIG_END

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
