// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/***************************************************************************

    Elektronika MS6102.02 terminal

    https://goo.gl/photos/xJManS26QTxG1T7M7
        Schematics

    http://sfrolov.livejournal.com/110770.html
        Photos

    To do:
    - why DMA stops after 2nd char on each row?
    - what does second 8275 do?
    - keyboard (MS7002)

    Chips:
    - DD5 - KR580WM80A (8080 clone) - CPU
    - DD7 - KR580WT57 (8257 clone) - DMAC
    - DD9 - KR1601RR1 (ER2401 clone) - NVRAM
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
#include "machine/keyboard.h"
#include "machine/nvram.h"
#include "machine/pit8253.h"
#include "video/i8275.h"

#include "screen.h"

#define LOG_GENERAL (1U <<  0)

//#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC printf
#include "logmacro.h"


class ms6102_state : public driver_device
{
public:
	ms6102_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_p_videoram(*this, "videoram")
		, m_maincpu(*this, "maincpu")
		, m_nvram(*this, "nvram")
		, m_pic(*this, "i8214")
		, m_dma8257(*this, "dma8257")
		, m_i8251(*this, "i8251")
		, m_rs232(*this, "rs232")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_p_chargen(*this, "chargen")
		{ }

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	template <unsigned N> DECLARE_WRITE_LINE_MEMBER(irq) { m_pic->r_w(N, state?0:1); }

	I8275_DRAW_CHARACTER_MEMBER(display_pixels);

	DECLARE_DRIVER_INIT(ms6102);
	DECLARE_WRITE_LINE_MEMBER(hrq_w);

	DECLARE_WRITE8_MEMBER(pic_w);
	IRQ_CALLBACK_MEMBER(ms6102_int_ack);

	DECLARE_READ8_MEMBER(memory_read_byte);

	DECLARE_READ8_MEMBER(misc_r);
	DECLARE_READ8_MEMBER(kbd_get);
	void kbd_put(u8 data);

	void ms6102(machine_config &config);
	void ms6102_io(address_map &map);
	void ms6102_mem(address_map &map);
private:
	bool m_kbd_ready;
	uint8_t m_kbd_data;

	virtual void machine_reset() override;
	virtual void machine_start() override;
	required_shared_ptr<uint8_t> m_p_videoram;
	required_device<i8080_cpu_device> m_maincpu;
	required_device<nvram_device> m_nvram;
	required_device<i8214_device> m_pic;
	required_device<i8257_device> m_dma8257;
	required_device<i8251_device> m_i8251;
	required_device<rs232_port_device> m_rs232;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_region_ptr<u8> m_p_chargen;
};

ADDRESS_MAP_START(ms6102_state::ms6102_mem)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE (0x0000, 0x2fff) AM_ROM
	AM_RANGE (0x3800, 0x3bff) AM_RAM AM_SHARE("nvram")
	AM_RANGE (0xc000, 0xffff) AM_RAM AM_SHARE("videoram")
ADDRESS_MAP_END

ADDRESS_MAP_START(ms6102_state::ms6102_io)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE (0x00, 0x00) AM_DEVREADWRITE("i8251", i8251_device, data_r, data_w)
	AM_RANGE (0x01, 0x01) AM_DEVREADWRITE("i8251", i8251_device, status_r, control_w)
	AM_RANGE (0x10, 0x18) AM_DEVREADWRITE("dma8257", i8257_device, read, write)
	AM_RANGE (0x20, 0x23) AM_DEVREADWRITE("pit8253", pit8253_device, read, write)
	AM_RANGE (0x30, 0x3f) AM_READ(kbd_get)
	AM_RANGE (0x40, 0x41) AM_DEVREADWRITE("i8275", i8275_device, read, write)
	AM_RANGE (0x50, 0x5f) AM_NOP // video disable?
	AM_RANGE (0x60, 0x6f) AM_WRITE(pic_w)
	AM_RANGE (0x70, 0x7f) AM_READ(misc_r)
ADDRESS_MAP_END

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

static GFXDECODE_START(ms6102)
	GFXDECODE_ENTRY("chargen", 0x0000, ms6102_charlayout, 0, 1)
GFXDECODE_END


WRITE_LINE_MEMBER(ms6102_state::hrq_w)
{
	/* HACK - this should be connected to the HOLD line of 8080 */
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);

	/* HACK - this should be connected to the HLDA line of 8080 */
	m_dma8257->hlda_w(state);
}

READ8_MEMBER(ms6102_state::memory_read_byte)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

I8275_DRAW_CHARACTER_MEMBER(ms6102_state::display_pixels)
{
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	u8 gfx = (lten) ? 0xff : 0;
	if (linecount < 12 && !vsp)
		gfx = m_p_chargen[linecount | (charcode << 4)];

	if (rvv)
		gfx ^= 0xff;

	for(u8 i=0; i<8; i++)
		bitmap.pix32(y, x + i) = palette[BIT(gfx, 7-i) ? (hlgt ? 2 : 1) : 0];
}


READ8_MEMBER(ms6102_state::misc_r)
{
	return m_kbd_ready << 6;
}

READ8_MEMBER(ms6102_state::kbd_get)
{
	return m_kbd_data;
}

void ms6102_state::kbd_put(u8 data)
{
	m_kbd_ready = true;
	m_kbd_data = data;
	m_pic->r_w(1, 0);
}


WRITE8_MEMBER(ms6102_state::pic_w)
{
	m_pic->b_w(data & 7);
	m_pic->sgs_w(BIT(data, 3));
}

IRQ_CALLBACK_MEMBER(ms6102_state::ms6102_int_ack)
{
	return 0xc7 | (m_pic->a_r() << 3);
}


void ms6102_state::machine_reset()
{
	m_kbd_ready = false;
}

void ms6102_state::machine_start()
{
	m_pic->etlg_w(1);
}

DRIVER_INIT_MEMBER( ms6102_state, ms6102 )
{
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
			m_p_chargen[i*16+j] = m_p_chargen[0x1800+i*8+j];
	// copy the russian symbols to codes 0xc0-0xff for now
	for (i = 0xc0; i < 0x100; i++)
		for (j = 0; j < 8; j++)
			m_p_chargen[i*16+j] = m_p_chargen[0x1800+i*8+j];
	// for punctuation, get the last 4 lines into place
	for (i = 0x20; i < 0x40; i++)
		for (j = 0; j < 4; j++)
			m_p_chargen[i*16+8+j] = m_p_chargen[0x1700+i*8+j];
	// for letters, get the last 4 lines into place
	for (i = 0x40; i < 0x80; i++)
		for (j = 0; j < 4; j++)
			m_p_chargen[i*16+8+j] = m_p_chargen[0x1a00+i*8+j];
	// for russian, get the last 4 lines into place
	for (i = 0xc0; i < 0x100; i++)
		for (j = 0; j < 4; j++)
			m_p_chargen[i*16+8+j] = m_p_chargen[0x1604+i*8+j];
}


MACHINE_CONFIG_START(ms6102_state::ms6102)
	MCFG_CPU_ADD("maincpu", I8080, XTAL(18'432'000) / 9)
	MCFG_CPU_PROGRAM_MAP(ms6102_mem)
	MCFG_CPU_IO_MAP(ms6102_io)
	MCFG_I8085A_INTE(DEVWRITELINE("i8214", i8214_device, inte_w))
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(ms6102_state, ms6102_int_ack)

	MCFG_NVRAM_ADD_0FILL("nvram")

	MCFG_DEVICE_ADD("i8214", I8214, XTAL(18'432'000) / 9)
	MCFG_I8214_INT_CALLBACK(INPUTLINE("maincpu", I8085_INTR_LINE))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_UPDATE_DEVICE("i8275", i8275_device, screen_update)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(784, 375)
	MCFG_SCREEN_VISIBLE_AREA(100, 100+80*8-1, 7, 7+24*15-1)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", ms6102)
	MCFG_PALETTE_ADD_MONOCHROME_HIGHLIGHT("palette")

	MCFG_DEVICE_ADD("dma8257", I8257, XTAL(18'432'000) / 9)
	MCFG_I8257_OUT_HRQ_CB(WRITELINE(ms6102_state, hrq_w))
	MCFG_I8257_IN_MEMR_CB(READ8(ms6102_state, memory_read_byte))
	MCFG_I8257_OUT_IOW_2_CB(DEVWRITE8("i8275", i8275_device, dack_w))

	MCFG_DEVICE_ADD("i8275", I8275, XTAL(16'400'000) / 8) // XXX
	MCFG_I8275_CHARACTER_WIDTH(8)
	MCFG_I8275_DRAW_CHARACTER_CALLBACK_OWNER(ms6102_state, display_pixels)
	MCFG_I8275_DRQ_CALLBACK(DEVWRITELINE("dma8257", i8257_device, dreq2_w))
	MCFG_I8275_IRQ_CALLBACK(WRITELINE(ms6102_state, irq<5>))

	// keyboard
	MCFG_DEVICE_ADD("589wa1", AY31015, 0)

	MCFG_DEVICE_ADD("keyboard", GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(PUT(ms6102_state, kbd_put))

	// serial connection to host
	MCFG_DEVICE_ADD("i8251", I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_I8251_RXRDY_HANDLER(WRITELINE(ms6102_state, irq<3>))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "null_modem")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("i8251", i8251_device, write_rxd))

	MCFG_DEVICE_ADD("pit8253", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL(16'400'000) / 9)
	MCFG_PIT8253_OUT0_HANDLER(DEVWRITELINE("i8251", i8251_device, write_txc))
	MCFG_PIT8253_CLK1(XTAL(16'400'000) / 9)
	MCFG_PIT8253_OUT1_HANDLER(DEVWRITELINE("i8251", i8251_device, write_rxc))
MACHINE_CONFIG_END

ROM_START( ms6102 )
	ROM_REGION(0x3000, "maincpu", 0)
	ROM_LOAD("MC6102_02_KS573RF2_DD26",   0x0000, 0x0800, CRC(f96ba806) SHA1(60d155b781e97e86d31dc2194ad367030470eeb6))
	ROM_LOAD("MC6102_02_KS573RF2_DD30",   0x0800, 0x0800, CRC(1d69ba62) SHA1(bf7d19400fe606239ce8a057850cf4c63ff4cdb2))
	ROM_LOAD("MC6102_02_KS573RF2_0034",   0x1000, 0x0800, CRC(4bce121a) SHA1(e97c635c2fab70a71a31db3b53284209b5881f2c))
	ROM_LOAD("MC6102_02_KS573RF2_0037",   0x1800, 0x0800, CRC(1b22543f) SHA1(fc6cc54baf3abadca30dfaf39a50cae7fbf601b2))
	ROM_LOAD("MC6102_02_KS573RF2_0045",   0x2000, 0x0800, CRC(fd741cfe) SHA1(153abb57ca4833286811082ff50c7b36136274dc))
	ROM_LOAD("MC6102_02_KS573RF2_DD49",   0x2800, 0x0800, CRC(748f6cee) SHA1(a35e6495ea108824f2f1f9907f5e651174e9cf15))

	ROM_REGION(0x2000, "chargen", ROMREGION_ERASE00)
	ROM_LOAD("MC6102_02_K555RE4_chargen", 0x1000, 0x0800, CRC(b0e3546b) SHA1(25aca264cc64f368ffcefdfd356120a314a44947))

	ROM_REGION(0x0100, "charmap", 0)
	ROM_LOAD("MC6102_02_K556RT4_D64",     0x0000, 0x0100, CRC(a59fdaa7) SHA1(0851a8b12e838e8f7e5ce840a0262facce303442))
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT  COMPAT   MACHINE    INPUT    CLASS          INIT       COMPANY       FULLNAME       FLAGS */
COMP( 1984, ms6102,  0,      0,       ms6102,    0,       ms6102_state,  ms6102, "Elektronika", "MS 6102.02", MACHINE_IS_SKELETON)
