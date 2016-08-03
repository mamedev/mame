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
#include "cpu/m6502/m6502.h"
#include "video/mc6845.h"
#include "machine/6522via.h"
#include "machine/mos6551.h"
#include "bus/rs232/rs232.h"

#define ACIA1_TAG   "acia1"
#define ACIA2_TAG   "acia2"
#define ACIA3_TAG   "acia3"
#define CRTC_TAG    "crtc"
#define VIA_TAG     "via"
#define RS232A_TAG  "rs232a"
#define RS232B_TAG  "rs232b"

#define MASTER_CLOCK (23814000)

class tv950_state : public driver_device
{
public:
	tv950_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_via(*this, VIA_TAG)
		, m_crtc(*this, CRTC_TAG)
		, m_vram(*this, "vram")
	{ }

	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_update_addr);

	DECLARE_WRITE8_MEMBER(row_addr_w);
	DECLARE_WRITE_LINE_MEMBER(via_crtc_reset_w);

private:
	virtual void machine_reset() override;
	required_device<m6502_device> m_maincpu;
	required_device<via6522_device> m_via;
	required_device<r6545_1_device> m_crtc;
	required_shared_ptr<UINT8> m_vram;

	int m_row_addr;
};

static ADDRESS_MAP_START(tv950_mem, AS_PROGRAM, 8, tv950_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_RAM
	AM_RANGE(0x2000, 0x3fff) AM_RAM AM_SHARE("vram") // VRAM
	AM_RANGE(0x8000, 0x8100) AM_DEVREADWRITE(CRTC_TAG, r6545_1_device, status_r, address_w)
	AM_RANGE(0x8001, 0x8101) AM_DEVREADWRITE(CRTC_TAG, r6545_1_device, register_r, register_w)
	AM_RANGE(0x9000, 0x9000) AM_WRITE(row_addr_w)
	AM_RANGE(0x9300, 0x9303) AM_DEVREADWRITE(ACIA1_TAG, mos6551_device, read, write)
	AM_RANGE(0x9500, 0x9503) AM_DEVREADWRITE(ACIA2_TAG, mos6551_device, read, write)
	AM_RANGE(0x9900, 0x9903) AM_DEVREADWRITE(ACIA3_TAG, mos6551_device, read, write)
	AM_RANGE(0xb100, 0xb10f) AM_DEVREADWRITE(VIA_TAG, via6522_device, read, write)
	AM_RANGE(0xe000, 0xffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( tv950 )
INPUT_PORTS_END


void tv950_state::machine_reset()
{
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

MC6845_ON_UPDATE_ADDR_CHANGED( tv950_state::crtc_update_addr )
{
}

MC6845_UPDATE_ROW( tv950_state::crtc_update_row )
{
}

static MACHINE_CONFIG_START( tv950, tv950_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6502, MASTER_CLOCK/14)
	MCFG_CPU_PROGRAM_MAP(tv950_mem)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK, 882, 0, 720, 370, 0, 350 ) // not real values
	MCFG_SCREEN_UPDATE_DEVICE( CRTC_TAG, r6545_1_device, screen_update )

	// there are many 6845 CRTC submodels, the Theory of Operation manual references the Rockwell R6545-1 specificially.
	MCFG_MC6845_ADD(CRTC_TAG, R6545_1, "screen", MASTER_CLOCK/14)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)
	MCFG_MC6845_UPDATE_ROW_CB(tv950_state, crtc_update_row)
	MCFG_MC6845_ADDR_CHANGED_CB(tv950_state, crtc_update_addr)
	MCFG_MC6845_OUT_HSYNC_CB(DEVWRITELINE(VIA_TAG, via6522_device, write_pb6))

	MCFG_DEVICE_ADD(VIA_TAG, VIA6522, MASTER_CLOCK/14)
	MCFG_VIA6522_IRQ_HANDLER(INPUTLINE("maincpu", M6502_NMI_LINE))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE(tv950_state, via_crtc_reset_w))

	MCFG_DEVICE_ADD(ACIA1_TAG, MOS6551, 0)
	MCFG_MOS6551_XTAL(MASTER_CLOCK/13)

	MCFG_DEVICE_ADD(ACIA2_TAG, MOS6551, 0)
	MCFG_MOS6551_XTAL(MASTER_CLOCK/13)

	MCFG_DEVICE_ADD(ACIA3_TAG, MOS6551, 0)
	MCFG_MOS6551_XTAL(MASTER_CLOCK/13)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( tv950 )
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD( "180000-001a_a41_eb17.bin", 0x001000, 0x001000, CRC(b7187cc5) SHA1(41cc8fd51661314e03ee7e00cc1e206e9a694d92) )
	ROM_LOAD( "180000-007a_a42_67d3.bin", 0x000000, 0x001000, CRC(3ef2e6fb) SHA1(21ccfd2b50c37b715eed67671b82faa4d75fc6bb) )

	ROM_REGION(0x2000, "graphics", 0)
	ROM_LOAD16_BYTE( "180000-002a_a33_9294.bin", 0x000001, 0x001000, CRC(eaf4f346) SHA1(b4c531626846f3f055ddc086ac24fdb1b34f3f8e) )
	ROM_LOAD16_BYTE( "180000-003a_a32_7ebf.bin", 0x000000, 0x001000, CRC(783ca0b6) SHA1(1cec9a9a56ef5795809f7ca7cd2e3f61b27e698d) )

	ROM_REGION(0x1000, "kbd", 0)
	ROM_LOAD( "950kbd_8748_pn52080723-02.bin", 0x000000, 0x000400, CRC(11c8f22c) SHA1(99e73e9c74b10055733e89b92adbc5bf7f4ff338) )
ROM_END

/* Driver */
/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT  STATE         INIT    COMPANY    FULLNAME       FLAGS */
COMP( 1981, tv950,  0,      0,       tv950,     tv950, driver_device,  0,  "TeleVideo", "TV950", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
