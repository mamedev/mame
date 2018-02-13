// license:BSD-3-Clause
// copyright-holders:R. Belmont, Robbbert
/***************************************************************************

    2013-09-10 Skeleton driver for Televideo TV950
    2016-07-30 Preliminary not-so-skeleton driver

    TODO:
    - Keyboard
    - CRTC reset and drawing

    Hardware:
    6502 CPU
    6545 CRTC
    6522 VIA, wired to count HSYNCs and to enable the 6502 to pull RESET on the CRTC
    3x 6551 ACIA  1 for the keyboard, 1 for the modem port, 1 for the printer port

    VIA hookup (see schematics):
    PA3 = beep?
    PA5 = inverse video
    PA6 = IRQ in
    PA7 = force blank
    PB6 = Hblank in
    CA1 = reset CRTC in
    CA2 = reset CRTC out
    CB2 = blink timer

    IRQ = ACIAs (all 3 ORed together)
    NMI = 6522 VIA's IRQ line

    http://www.bitsavers.org/pdf/televideo/950/Model_950_Terminal_Theory_of_Operation_26Jan1981.pdf
    http://www.bitsavers.org/pdf/televideo/950/2002100_Model_950_Maintenance_Manual_Nov1983.pdf
    http://www.bitsavers.org/pdf/televideo/950/B300002-001_Model_950_Operators_Manual_Feb81.pdf

****************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m6502/m6502.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/6522via.h"
#include "machine/mos6551.h"
#include "video/mc6845.h"
#include "screen.h"

#define ACIA1_TAG   "acia1"
#define ACIA2_TAG   "acia2"
#define ACIA3_TAG   "acia3"
#define CRTC_TAG    "crtc"
#define VIA_TAG     "via"
#define RS232A_TAG  "rs232a"
#define RS232B_TAG  "rs232b"

#define MASTER_CLOCK XTAL(23'814'000)

class tv950_state : public driver_device
{
public:
	tv950_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_via(*this, VIA_TAG)
		, m_crtc(*this, CRTC_TAG)
		, m_vram(*this, "vram")
		, m_gfx(*this, "graphics")
	{ }

	DECLARE_WRITE8_MEMBER(via_a_w);
	DECLARE_WRITE8_MEMBER(via_b_w);
	DECLARE_READ8_MEMBER(via_b_r);
	DECLARE_WRITE_LINE_MEMBER(crtc_vs_w);
	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_update_addr);

	DECLARE_WRITE8_MEMBER(row_addr_w);
	DECLARE_WRITE_LINE_MEMBER(via_crtc_reset_w);

	void tv950(machine_config &config);
	void tv950_mem(address_map &map);
private:
	uint8_t m_via_row;
	uint8_t m_attr_row;
	uint8_t m_attr_screen;
	virtual void machine_reset() override;
	required_device<m6502_device> m_maincpu;
	required_device<via6522_device> m_via;
	required_device<r6545_1_device> m_crtc;
	required_shared_ptr<uint8_t> m_vram;
	required_region_ptr<uint16_t> m_gfx;

	int m_row_addr;
	int m_row;
};

ADDRESS_MAP_START(tv950_state::tv950_mem)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x2000, 0x3fff) AM_RAM AM_SHARE("vram") // VRAM
	AM_RANGE(0x8100, 0x8100) AM_DEVREADWRITE(CRTC_TAG, r6545_1_device, status_r, address_w)
	AM_RANGE(0x8101, 0x8101) AM_DEVREADWRITE(CRTC_TAG, r6545_1_device, register_r, register_w)
	AM_RANGE(0x9000, 0x9000) AM_WRITE(row_addr_w)
	AM_RANGE(0x9300, 0x9303) AM_DEVREADWRITE(ACIA1_TAG, mos6551_device, read, write)
	AM_RANGE(0x9500, 0x9503) AM_DEVREADWRITE(ACIA2_TAG, mos6551_device, read, write)
	AM_RANGE(0x9900, 0x9903) AM_DEVREADWRITE(ACIA3_TAG, mos6551_device, read, write)
	AM_RANGE(0xb100, 0xb10f) AM_DEVREADWRITE(VIA_TAG, via6522_device, read, write)
	AM_RANGE(0xe000, 0xffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( tv950 )
	PORT_START("DSW0")
	PORT_DIPNAME( 0x01, 0x00, "S01")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPNAME( 0x02, 0x00, "S02")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_DIPNAME( 0x04, 0x00, "S03")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPNAME( 0x08, 0x00, "S04")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPNAME( 0x10, 0x00, "S05")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPNAME( 0x20, 0x20, "S06")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "S07")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPNAME( 0x02, 0x00, "S08")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_DIPNAME( 0x04, 0x00, "S09")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPNAME( 0x08, 0x00, "S10")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPNAME( 0x10, 0x00, "S11")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPNAME( 0x20, 0x20, "S12")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "S13")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPNAME( 0x02, 0x00, "S14")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_DIPNAME( 0x04, 0x00, "S15")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x04, DEF_STR( Off ))
	PORT_DIPNAME( 0x08, 0x00, "S16")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x08, DEF_STR( Off ))
	PORT_DIPNAME( 0x10, 0x00, "S17")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x10, DEF_STR( Off ))
	PORT_DIPNAME( 0x20, 0x20, "S18")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x20, DEF_STR( Off ))
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x00, "S19")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x01, DEF_STR( Off ))
	PORT_DIPNAME( 0x02, 0x00, "S20")
	PORT_DIPSETTING(    0x00, DEF_STR( On ))
	PORT_DIPSETTING(    0x02, DEF_STR( Off ))
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


void tv950_state::machine_reset()
{
	m_row = 0;
	m_via_row = 0;
	m_attr_row = 0;
	m_attr_screen = 0;
}

WRITE_LINE_MEMBER(tv950_state::crtc_vs_w)
{
	m_attr_screen = 0;
}

WRITE_LINE_MEMBER(tv950_state::via_crtc_reset_w)
{
	//printf("via_crtc_reset_w: %d\n", state);
	m_via->write_ca1(state);

	if (!state)
	{
		//m_crtc->device_reset();
	}
}

WRITE8_MEMBER(tv950_state::row_addr_w)
{
	m_row_addr = data;
}

WRITE8_MEMBER(tv950_state::via_a_w)
{
	m_via_row = ~data & 15;
	m_maincpu->set_input_line(M6502_IRQ_LINE, BIT(data, 6) ? CLEAR_LINE : ASSERT_LINE);
	// PA4, 5, 7 to do
}

WRITE8_MEMBER(tv950_state::via_b_w)
{
// bit 7 = speaker, and bit 3 of m_via_row must be active as well
}

READ8_MEMBER(tv950_state::via_b_r)
{
	uint8_t data = 0xff;
	if (BIT(m_via_row, 0))
		data &= ioport("DSW0")->read();
	if (BIT(m_via_row, 1))
		data &= ioport("DSW1")->read();
	if (BIT(m_via_row, 2))
		data &= ioport("DSW2")->read();
	if (BIT(m_via_row, 3))
		data &= ioport("DSW3")->read();
	return data;
}

MC6845_ON_UPDATE_ADDR_CHANGED( tv950_state::crtc_update_addr )
{
}

MC6845_UPDATE_ROW( tv950_state::crtc_update_row )
{
	if (ra)
		m_attr_row = m_attr_screen;
	else
		m_attr_screen = m_attr_row;

	uint32_t *p = &bitmap.pix32(m_row);
	rgb_t fg(255,255,255,255);
	rgb_t bg(0,0,0,0);

	for(uint8_t x = 0; x < x_count; x++)
	{
		uint8_t chr = m_vram[ma + x];
		if ((chr & 0x90)==0x90)
			m_attr_row = chr & 15;
		uint16_t data = m_gfx[chr * 16 + (m_row % 10)];
		if (x == cursor_x)
			data ^= 0xff;
		// apply attributes...

		for (uint8_t i = 0; i < 14; i++)
			*p++ = BIT( data, 13-i ) ? fg : bg;
	}
	m_row = (m_row + 1) % 250;
}

MACHINE_CONFIG_START(tv950_state::tv950)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, MASTER_CLOCK/14)
	MCFG_CPU_PROGRAM_MAP(tv950_mem)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK, 1200, 0, 1120, 370, 0, 250 )   // not real values
	MCFG_SCREEN_UPDATE_DEVICE( CRTC_TAG, r6545_1_device, screen_update )

	// there are many 6845 CRTC submodels, the Theory of Operation manual references the Rockwell R6545-1 specificially.
	MCFG_MC6845_ADD(CRTC_TAG, R6545_1, "screen", MASTER_CLOCK/14)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(14)
	MCFG_MC6845_UPDATE_ROW_CB(tv950_state, crtc_update_row)
	MCFG_MC6845_ADDR_CHANGED_CB(tv950_state, crtc_update_addr)
	MCFG_MC6845_OUT_HSYNC_CB(DEVWRITELINE(VIA_TAG, via6522_device, write_pb6))
	MCFG_MC6845_OUT_VSYNC_CB(WRITELINE(tv950_state, crtc_vs_w))
	MCFG_VIDEO_SET_SCREEN(nullptr)

	MCFG_DEVICE_ADD(VIA_TAG, VIA6522, MASTER_CLOCK/14)
	MCFG_VIA6522_IRQ_HANDLER(INPUTLINE("maincpu", M6502_NMI_LINE))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(tv950_state, via_a_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(tv950_state, via_b_w))
	MCFG_VIA6522_READPB_HANDLER(READ8(tv950_state, via_b_r))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE(tv950_state, via_crtc_reset_w))
	//MCFG_VIA6522_CB2_HANDLER(WRITELINE(tv950_state, via_blink_rate_w))

	MCFG_DEVICE_ADD(ACIA1_TAG, MOS6551, 0)
	MCFG_MOS6551_XTAL(MASTER_CLOCK/13)

	MCFG_DEVICE_ADD(ACIA2_TAG, MOS6551, 0)
	MCFG_MOS6551_XTAL(MASTER_CLOCK/13)

	MCFG_DEVICE_ADD(ACIA3_TAG, MOS6551, 0)
	MCFG_MOS6551_XTAL(MASTER_CLOCK/13)

	MCFG_DEVICE_ADD("kbd", I8748, XTAL(5'714'300))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( tv950 )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "180000-001a_a41_eb17.bin", 0x001000, 0x001000, CRC(b7187cc5) SHA1(41cc8fd51661314e03ee7e00cc1e206e9a694d92) )
	ROM_LOAD( "180000-007a_a42_67d3.bin", 0x000000, 0x001000, CRC(3ef2e6fb) SHA1(21ccfd2b50c37b715eed67671b82faa4d75fc6bb) )

	ROM_REGION16_LE(0x2000, "graphics", 0)
	ROM_LOAD16_BYTE( "180000-002a_a33_9294.bin", 0x000001, 0x001000, CRC(eaf4f346) SHA1(b4c531626846f3f055ddc086ac24fdb1b34f3f8e) )
	ROM_LOAD16_BYTE( "180000-003a_a32_7ebf.bin", 0x000000, 0x001000, CRC(783ca0b6) SHA1(1cec9a9a56ef5795809f7ca7cd2e3f61b27e698d) )

	ROM_REGION(0x400, "kbd", 0)
	ROM_LOAD( "950kbd_8748_pn52080723-02.bin", 0x000000, 0x000400, CRC(11c8f22c) SHA1(99e73e9c74b10055733e89b92adbc5bf7f4ff338) )

	ROM_REGION(0x10000, "user1", 0)
	// came with "tv950.zip"
	ROM_LOAD( "180000-43i.a25", 0x0000, 0x1000, CRC(ac6f0bfc) SHA1(2a3863700405fbb9e510613559d78fceee3544e8) )
	ROM_LOAD( "180000-44i.a20", 0x0000, 0x1000, CRC(db91a727) SHA1(e94724ed1a563fb846f4203ae6523ee6b4c6577f) )
	// came with "tv950kbd.zip"
	ROM_LOAD( "1800000-003a.a32", 0x0000, 0x1000, CRC(eaef0138) SHA1(7198851299fce07c95d18e32cbfbe936c0dbec2a) )
	ROM_LOAD( "1800000-002a.a33", 0x0000, 0x1000, CRC(856dd85c) SHA1(e2570017e098b0e1ead7749e9c2ac40be2367433) )
ROM_END

/* Driver */
//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  STATE        INIT  COMPANY      FULLNAME  FLAGS
COMP( 1981, tv950,  0,      0,      tv950,   tv950, tv950_state, 0,    "TeleVideo", "Model 950 Video Display Terminal",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
