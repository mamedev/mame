// license:BSD-3-Clause
// copyright-holders:Carl

#include "pcd.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/mcs48/mcs48.h"

const device_type PCD_VIDEO = &device_creator<pcd_video_device>;
const device_type PCX_VIDEO = &device_creator<pcx_video_device>;

pcdx_video_device::pcdx_video_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	m_maincpu(*this, ":maincpu"),
	m_mcu(*this, "graphics"),
	m_crtc(*this, "crtc"),
	m_palette(*this, "palette"),
	m_gfxdecode(*this, "gfxdecode"),
	m_pic2(*this, ":pic2")
{
}

pcd_video_device::pcd_video_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	pcdx_video_device(mconfig, PCD_VIDEO, "Siemens PC-D Video", tag, owner, clock, "pcd_video", __FILE__),
	m_mouse_btn(*this, "MOUSE"),
	m_mouse_x(*this, "MOUSEX"),
	m_mouse_y(*this, "MOUSEY"),
	m_vram(32*1024),
	m_charram(8*1024)
{
}

pcx_video_device::pcx_video_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	pcdx_video_device(mconfig, PCX_VIDEO, "Siemens PC-X Video", tag, owner, clock, "pcx_video", __FILE__),
	device_serial_interface(mconfig, *this),
	m_vram(4*1024),
	m_charrom(*this, "char"),
	m_txd_handler(*this)
{
}

ROM_START( pcd_video )
	ROM_REGION(0x400, "graphics", 0)
	ROM_LOAD("s36361-d321-v1.bin", 0x000, 0x400, CRC(69baeb2a) SHA1(98b9cd0f38c51b4988a3aed0efcf004bedd115ff))
ROM_END

const rom_entry *pcd_video_device::device_rom_region() const
{
	return ROM_NAME( pcd_video );
}

ROM_START( pcx_video )
	ROM_REGION(0x2000, "char", 0)
	ROM_LOAD("d12-graka.bin", 0x0000, 0x2000, CRC(e4933c16) SHA1(932ae1f0cd2b029b7f5fc3d2d1679e70b25c0828))

	ROM_REGION(0x6000, "graphics", 0)
	ROM_LOAD("d40-graka.bin", 0x0000, 0x2000, CRC(dce48252) SHA1(0d9a575b2d001168a36864d7bd5db1c3aca5fb8d))
	ROM_LOAD("d39-graka.bin", 0x4000, 0x2000, CRC(02920e25) SHA1(145a6648d75c1dc4788f9bc7790281ef7e8f8426))
ROM_END

const rom_entry *pcx_video_device::device_rom_region() const
{
	return ROM_NAME( pcx_video );
}

static const gfx_layout pcd_charlayout =
{
	8, 14,                   /* 8 x 14 characters */
	512,                    /* 512 characters */
	1,                  /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8 },
	8*16
};

static GFXDECODE_START( pcx )
	GFXDECODE_ENTRY( "char", 0x0000, pcd_charlayout, 0, 1 )
GFXDECODE_END

static INPUT_PORTS_START( pcd_mouse )
	PORT_START("MOUSE")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Right Mouse Button")   PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Left Mouse Button")    PORT_CODE(MOUSECODE_BUTTON2)

	PORT_START("MOUSEX")
	PORT_BIT( 0xfff, 0x000, IPT_MOUSE_X ) PORT_SENSITIVITY(50) PORT_KEYDELTA(0)

	PORT_START("MOUSEY")
	PORT_BIT( 0xfff, 0x000, IPT_MOUSE_Y ) PORT_SENSITIVITY(50) PORT_KEYDELTA(0)
INPUT_PORTS_END

ioport_constructor pcd_video_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(pcd_mouse);
}

static ADDRESS_MAP_START( pcd_vid_io, AS_IO, 8, pcd_video_device )
	AM_RANGE(MCS48_PORT_P1, MCS48_PORT_P1) AM_READ(p1_r)
	AM_RANGE(MCS48_PORT_P2, MCS48_PORT_P2) AM_WRITE(p2_w)
	AM_RANGE(MCS48_PORT_T1, MCS48_PORT_T1) AM_READ(t1_r)
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT( pcd_video )
	MCFG_CPU_ADD("graphics", I8741, XTAL_16MHz/2)
	MCFG_CPU_IO_MAP(pcd_vid_io)

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE(640, 350)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 349)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", scn2674_device, screen_update)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", empty)
	MCFG_PALETTE_ADD("palette", 3)
	MCFG_PALETTE_INIT_OWNER(pcdx_video_device, pcdx)

	MCFG_SCN2674_VIDEO_ADD("crtc", 0, NULL);
	MCFG_SCN2674_TEXT_CHARACTER_WIDTH(8)
	MCFG_SCN2674_GFX_CHARACTER_WIDTH(16)
	MCFG_SCN2674_DRAW_CHARACTER_CALLBACK_OWNER(pcd_video_device, display_pixels)
	MCFG_VIDEO_SET_SCREEN("screen")
	MCFG_TIMER_DRIVER_ADD_PERIODIC("mouse_timer", pcd_video_device, mouse_timer, attotime::from_hz(15000)) // guess
MACHINE_CONFIG_END

machine_config_constructor pcd_video_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pcd_video );
}

static ADDRESS_MAP_START( pcx_vid_map, AS_PROGRAM, 8, pcx_video_device )
	AM_RANGE(0x0000, 0x5fff) AM_ROM AM_REGION("graphics", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( pcx_vid_io, AS_IO, 8, pcx_video_device )
	AM_RANGE(0x8000, 0x8007) AM_DEVREADWRITE("crtc", scn2674_device, read, write)
	AM_RANGE(0x8008, 0x8008) AM_READ(unk_r)
	AM_RANGE(0xa000, 0xa001) AM_READWRITE(vram_latch_r, vram_latch_w)
	AM_RANGE(0xa002, 0xa003) AM_READWRITE(term_mcu_r, term_mcu_w)
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
	AM_RANGE(MCS51_PORT_P1, MCS51_PORT_P1) AM_WRITE(p1_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( pcx_vram, AS_0, 8, pcx_video_device )
	AM_RANGE(0x0000, 0x07ff) AM_READWRITE(vram_r, vram_w)
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT( pcx_video )
	MCFG_CPU_ADD("graphics", I8031, XTAL_24MHz/2)
	MCFG_CPU_PROGRAM_MAP(pcx_vid_map)
	MCFG_CPU_IO_MAP(pcx_vid_io)

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_SIZE(640, 350)
	MCFG_SCREEN_VISIBLE_AREA(0, 639, 0, 349)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_UPDATE_DEVICE("crtc", scn2674_device, screen_update)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pcx)
	MCFG_PALETTE_ADD_BLACK_AND_WHITE("palette")

	MCFG_SCN2674_VIDEO_ADD("crtc", 0, INPUTLINE("graphics", MCS51_INT0_LINE));
	MCFG_SCN2674_TEXT_CHARACTER_WIDTH(8)
	MCFG_SCN2674_GFX_CHARACTER_WIDTH(16)
	MCFG_SCN2674_DRAW_CHARACTER_CALLBACK_OWNER(pcx_video_device, display_pixels)
	MCFG_VIDEO_SET_SCREEN("screen")
	MCFG_DEVICE_ADDRESS_MAP(AS_0, pcx_vram)
MACHINE_CONFIG_END

machine_config_constructor pcx_video_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( pcx_video );
}

SCN2674_DRAW_CHARACTER_MEMBER(pcd_video_device::display_pixels)
{
	address <<= 1;
	if(lg)
	{
		UINT16 data = m_vram[address + 1] | (m_vram[address] << 8);
		if(m_p2 & 8)
			data = ~data;
		for(int i = 0; i < 16; i++)
			bitmap.pix32(y, x + i) = m_palette->pen((data & (1 << (15 - i))) ? 1 : 0);
	}
	else
	{
		UINT8 data, attr;
		int bgnd = 0, fgnd = 1;
		data = m_charram[m_vram[address] * 16 + linecount];
		attr = m_vram[address + 1];
		if(cursor && blink)
			data = 0xff;
		if(ul && (attr & 0x20))
			data = 0xff;

		if(attr & 0x10)
			data = ~data;
		if(m_p2 & 8)
		{
			fgnd = 0;
			bgnd = (attr & 8) ? 2 : 1;
		}
		else if(attr & 8)
			bgnd = 2;
		for(int i = 0; i < 8; i++)
			bitmap.pix32(y, x + i) = m_palette->pen((data & (1 << (7 - i))) ? fgnd : bgnd);
	}
}

SCN2674_DRAW_CHARACTER_MEMBER(pcx_video_device::display_pixels)
{
	UINT8 data;
	address <<= 1;
	data = m_charrom[m_vram[address] * 16 + linecount + (m_vram[address + 1] & 0x20 ? 4096 : 0)];
	if(cursor && blink)
		data = 0xff;
	if(m_p1 & 0x20)
		data = ~data;
	for(int i = 0; i < 8; i++)
		bitmap.pix32(y, x + i) = m_palette->pen((data & (1 << (7 - i))) ? 1 : 0);
}

PALETTE_INIT_MEMBER(pcdx_video_device, pcdx)
{
	palette.set_pen_color(0,rgb_t::black);
	palette.set_pen_color(1,rgb_t::white);
	palette.set_pen_color(2,rgb_t(128,128,128));
}

TIMER_DEVICE_CALLBACK_MEMBER(pcd_video_device::mouse_timer)
{
	m_t1 = (m_t1 == CLEAR_LINE) ? ASSERT_LINE : CLEAR_LINE;
	if(m_t1)
	{
		switch(m_mouse.phase)
		{
		case 0:
			m_mouse.xa = m_mouse.x > m_mouse.prev_x ? CLEAR_LINE : ASSERT_LINE;
			m_mouse.xb = m_mouse.x < m_mouse.prev_x ? CLEAR_LINE : ASSERT_LINE;
			m_mouse.ya = m_mouse.y > m_mouse.prev_y ? CLEAR_LINE : ASSERT_LINE;
			m_mouse.yb = m_mouse.y < m_mouse.prev_y ? CLEAR_LINE : ASSERT_LINE;
			break;
		case 1:
			m_mouse.xa = m_mouse.xb = m_mouse.x != m_mouse.prev_x ? CLEAR_LINE : ASSERT_LINE;
			m_mouse.ya = m_mouse.yb = m_mouse.y != m_mouse.prev_y ? CLEAR_LINE : ASSERT_LINE;
			break;
		case 2:
			m_mouse.xa = m_mouse.x < m_mouse.prev_x ? CLEAR_LINE : ASSERT_LINE;
			m_mouse.xb = m_mouse.x > m_mouse.prev_x ? CLEAR_LINE : ASSERT_LINE;
			m_mouse.ya = m_mouse.y < m_mouse.prev_y ? CLEAR_LINE : ASSERT_LINE;
			m_mouse.yb = m_mouse.y > m_mouse.prev_y ? CLEAR_LINE : ASSERT_LINE;
			break;
		case 3:
			m_mouse.xa = m_mouse.xb = ASSERT_LINE;
			m_mouse.ya = m_mouse.yb = ASSERT_LINE;
			m_mouse.prev_x = m_mouse.x;
			m_mouse.prev_y = m_mouse.y;
			m_mouse.x = m_mouse_x->read();
			m_mouse.y = m_mouse_y->read();
			break;
		}

		m_mouse.phase = (m_mouse.phase + 1) & 3;
	}
}

WRITE8_MEMBER(pcd_video_device::vram_w)
{
	if(m_vram_sw)
		m_vram[offset] = data;
	else if(!(offset & 1))
	{
		offset >>= 1;
		m_charram[offset & 0x1fff] = data;
		m_gfxdecode->gfx(0)->mark_dirty(offset/16);
	}
}

READ8_MEMBER(pcd_video_device::vram_r)
{
	return m_vram[offset];
}

WRITE8_MEMBER(pcd_video_device::vram_sw_w)
{
	m_vram_sw = data & 1;
}

READ8_MEMBER(pcd_video_device::t1_r)
{
	return m_t1;
}

READ8_MEMBER(pcd_video_device::p1_r)
{
	UINT8 data = (m_mouse_btn->read() & 0x30) | 0x80; // char ram/rom jumper?
	data |= (m_mouse.xa != CLEAR_LINE ? 0 : 1);
	data |= (m_mouse.xb != CLEAR_LINE ? 0 : 2);
	data |= (m_mouse.ya != CLEAR_LINE ? 0 : 4);
	data |= (m_mouse.yb != CLEAR_LINE ? 0 : 8);

	return data;
}

WRITE8_MEMBER(pcd_video_device::p2_w)
{
	m_p2 = data;
	m_pic2->ir7_w((data & 0x80) ? CLEAR_LINE : ASSERT_LINE);
}

READ8_MEMBER(pcx_video_device::term_r)
{
	switch(offset)
	{
		case 0:
			m_pic2->ir0_w(CLEAR_LINE);
			m_term_stat &= ~2;
			return m_term_key;
		case 1:
			m_pic2->ir0_w(CLEAR_LINE);
			return m_term_stat >> 1;
	}
	return 0xff;
}

WRITE8_MEMBER(pcx_video_device::term_w)
{
	if(!offset)
	{
		m_mcu->set_input_line(MCS51_INT1_LINE, ASSERT_LINE);
		m_term_char = data;
		m_term_stat |= 4;
	}
}

READ8_MEMBER(pcx_video_device::term_mcu_r)
{
	switch(offset)
	{
		case 0:
			m_mcu->set_input_line(MCS51_INT1_LINE, CLEAR_LINE);
			m_pic2->ir0_w(ASSERT_LINE);
			m_term_stat &= ~4;
			return m_term_char;
		case 1:
			return m_term_stat;
	}
	return 0;
}

WRITE8_MEMBER(pcx_video_device::term_mcu_w)
{
	if(!offset)
	{
		m_term_key = data;
		m_pic2->ir0_w(ASSERT_LINE);
		m_term_stat |= 2;
	}
}

WRITE8_MEMBER(pcx_video_device::vram_w)
{
	offset <<= 1;
	m_vram[offset] = m_vram_latch_w[0];
	m_vram[offset+1] = m_vram_latch_w[1];
}

READ8_MEMBER(pcx_video_device::vram_r)
{
	offset <<= 1;
	m_vram_latch_r[0] = m_vram[offset];
	m_vram_latch_r[1] = m_vram[offset+1];
	return m_vram[offset];
}

WRITE8_MEMBER(pcx_video_device::vram_latch_w)
{
	m_vram_latch_w[offset] = data;
}

READ8_MEMBER(pcx_video_device::vram_latch_r)
{
	return m_vram_latch_r[offset];
}

READ8_MEMBER(pcdx_video_device::detect_r)
{
	return 0;
}

WRITE8_MEMBER(pcdx_video_device::detect_w)
{
}

READ8_MEMBER(pcx_video_device::unk_r)
{
	return 0x80;
}

WRITE8_MEMBER(pcx_video_device::p1_w)
{
	m_p1 = data;
}

void pcd_video_device::device_start()
{
	m_maincpu->space(AS_IO).install_readwrite_handler(0xfb00, 0xfb01, 0, 0, read8_delegate(FUNC(pcdx_video_device::detect_r), this), write8_delegate(FUNC(pcdx_video_device::detect_w), this), 0xff00);
	m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xf0000, 0xf7fff, 0, 0, read8_delegate(FUNC(pcd_video_device::vram_r), this), write8_delegate(FUNC(pcd_video_device::vram_w), this), 0xffff);
	m_gfxdecode->set_gfx(0, std::make_unique<gfx_element>(m_palette, pcd_charlayout, &m_charram[0], 0, 1, 0));
}

void pcd_video_device::device_reset()
{
	m_mouse.phase = 0;
	m_mouse.xa = m_mouse.xb = ASSERT_LINE;
	m_mouse.ya = m_mouse.yb = ASSERT_LINE;
	m_mouse.x = m_mouse.y = 0;
	m_mouse.prev_x = m_mouse.prev_y = 0;
	m_vram_sw = 1;
	m_t1 = CLEAR_LINE;
}

DEVICE_ADDRESS_MAP_START(map, 16, pcd_video_device)
	AM_RANGE(0x00, 0x0f) AM_DEVWRITE8("crtc", scn2674_device, write, 0x00ff)
	AM_RANGE(0x00, 0x0f) AM_DEVREAD8("crtc", scn2674_device, read, 0xff00)
	AM_RANGE(0x20, 0x21) AM_WRITE8(vram_sw_w, 0x00ff)
	AM_RANGE(0x30, 0x33) AM_DEVREADWRITE8("graphics", i8741_device, upi41_master_r, upi41_master_w, 0x00ff)
ADDRESS_MAP_END

void pcx_video_device::device_start()
{
	mcs51_cpu_device *mcu = downcast<mcs51_cpu_device *>(m_mcu.target());
	m_maincpu->space(AS_IO).install_readwrite_handler(0xfb00, 0xfb01, 0, 0, read8_delegate(FUNC(pcdx_video_device::detect_r), this), write8_delegate(FUNC(pcdx_video_device::detect_w), this), 0x00ff);
	m_txd_handler.resolve_safe();

	// set serial callbacks
	mcu->i8051_set_serial_tx_callback(WRITE8_DELEGATE(pcx_video_device, tx_callback));
	mcu->i8051_set_serial_rx_callback(READ8_DELEGATE(pcx_video_device, rx_callback));
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_1);
	set_rate(600*2);  // FIXME: fix the keyboard when the mc2661 baud rate calc is fixed
}

void pcx_video_device::device_reset()
{
	m_term_key = 0;
	m_term_stat = 0;
	transmit_register_reset();
	receive_register_reset();

	m_txd_handler(1);
}

DEVICE_ADDRESS_MAP_START(map, 16, pcx_video_device)
	AM_RANGE(0x0, 0xf) AM_READWRITE8(term_r, term_w, 0xffff)
ADDRESS_MAP_END

READ8_MEMBER(pcx_video_device::rx_callback)
{
	return get_received_char();
}

WRITE8_MEMBER(pcx_video_device::tx_callback)
{
	transmit_register_setup(data);
}

void pcx_video_device::tra_callback()
{
	m_txd_handler(transmit_register_get_data_bit());
}

void pcx_video_device::rcv_complete()
{
	receive_register_extract();
	m_mcu->set_input_line(MCS51_RX_LINE, ASSERT_LINE);
	m_mcu->set_input_line(MCS51_RX_LINE, CLEAR_LINE);
}

void pcx_video_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	device_serial_interface::device_timer(timer, id, param, ptr);
}
