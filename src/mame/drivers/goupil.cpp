// license:BSD-3-Clause
// copyright-holders:Jean-Francois DEL NERO
/***************************************************************************

    SMT Goupil G1 driver

    Current state :

    -> CPU / ROM / RAM working
    -> Video output working
    -> Keyboard support working (need to be polished... )
    -> Floppy FDC not fully implemented.
    -> Sound support missing.

    Software :
    -> The Monitor is working
    -> The internal Basic is working (-> 6800 0xC3 illegal opcode emulation needed).

    02/04/2016
    Jean-Francois DEL NERO

****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/6522via.h"
#include "machine/i8279.h"
#include "video/ef9364.h"
#include "video/mc6845.h"
#include "machine/6850acia.h"
#include "machine/wd_fdc.h"

#include "softlist.h"

#define MAIN_CLOCK           XTAL_4MHz
#define VIDEO_CLOCK          MAIN_CLOCK / 8     /* 1.75 Mhz */
#define CPU_CLOCK            MAIN_CLOCK / 4     /* 1 Mhz */

class goupil_g1_state : public driver_device
{
public:
	goupil_g1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_acia(*this,  "ef6850")
		, m_ef9364(*this, "ef9364")
		, m_maincpu(*this, "maincpu")
		, m_via_video(*this, "m_via_video")
		, m_via_keyb(*this, "m_via_keyb")
		, m_via_modem(*this, "m_via_modem")
		, m_fdc(*this, "fd1791")
		, m_floppy0(*this, "fd1791:0")
		, m_floppy1(*this, "fd1791:1")
		, m_floppy(NULL)
	{ }

	DECLARE_WRITE8_MEMBER(via_video_pba_w);
	DECLARE_WRITE8_MEMBER(via_video_pbb_w);
	DECLARE_WRITE_LINE_MEMBER(via_video_ca2_w);

	DECLARE_READ8_MEMBER(kbd1_r);
	DECLARE_READ8_MEMBER(kbd2_r);
	DECLARE_READ8_MEMBER(shift_kb1_r);
	DECLARE_READ8_MEMBER(shift_kb2_r);
	DECLARE_READ8_MEMBER(ctrl_kb1_r);
	DECLARE_READ8_MEMBER(ctrl_kb2_r);

	DECLARE_WRITE8_MEMBER(scanlines_kbd1_w);
	DECLARE_WRITE8_MEMBER(scanlines_kbd2_w);

	DECLARE_READ_LINE_MEMBER(via_keyb_ca2_r);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	UINT8 m_row_kbd1;
	UINT8 m_row_kbd2;
	int old_state_ca2;
	UINT8 via_video_pbb_data;
	UINT8 cnttim;
	UINT8 valkeyb;
	TIMER_DEVICE_CALLBACK_MEMBER(goupil_scanline);

private:
	required_device<acia6850_device> m_acia;
	required_device<ef9364_device> m_ef9364;
	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via_video;
	required_device<via6522_device> m_via_keyb;
	required_device<via6522_device> m_via_modem;
	required_device<fd1791_t> m_fdc;
	required_device<floppy_connector> m_floppy0;
	required_device<floppy_connector> m_floppy1;
	floppy_image_device *m_floppy;
};

/**********************************
* Floppy controller I/O Handlers  *
***********************************/
// TODO

/**********************************
*      Keyboard I/O Handlers      *
***********************************/

TIMER_DEVICE_CALLBACK_MEMBER( goupil_g1_state::goupil_scanline )
{
	m_ef9364->update_scanline((UINT16)param);
}

static ADDRESS_MAP_START(goupil_mem, AS_PROGRAM, 8, goupil_g1_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000,0x3fff) AM_RAM
	AM_RANGE(0x4000,0x7fff) AM_RAM
	AM_RANGE(0xC000,0xE3FF) AM_ROM AM_REGION("maincpu", 0x1000) // Basic ROM (BASIC 1 up to BASIC 9).

	AM_RANGE(0xe400,0xe7ff) AM_RAM
	AM_RANGE(0xE800,0xE80F) AM_DEVREADWRITE("ef6850", acia6850_device, data_r, data_w)
	AM_RANGE(0xE810,0xE81F) AM_DEVREADWRITE("m_via_video", via6522_device, read, write)

	AM_RANGE(0xE820,0xE820) AM_DEVREADWRITE("i8279_kb1", i8279_device, data_r, data_w )
	AM_RANGE(0xE821,0xE821) AM_DEVREADWRITE("i8279_kb1", i8279_device, status_r, cmd_w )

	AM_RANGE(0xE830,0xE830) AM_DEVREADWRITE("i8279_kb2", i8279_device, data_r, data_w )
	AM_RANGE(0xE831,0xE831) AM_DEVREADWRITE("i8279_kb2", i8279_device, status_r, cmd_w )

	AM_RANGE(0xE840,0xE84F) AM_DEVREADWRITE("m_via_keyb", via6522_device, read, write)

	AM_RANGE(0xE860,0xE86F) AM_DEVREADWRITE("m_via_modem", via6522_device, read, write)

	AM_RANGE(0xe8f0,0xe8ff) AM_DEVREADWRITE("fd1791", fd1791_t, read, write)
	//AM_RANGE(0xf08a,0xf08a) AM_READWRITE( fdc_sel0_r, fdc_sel0_w )
	//AM_RANGE(0xf08b,0xf08b) AM_READWRITE( fdc_sel1_r, fdc_sel1_w )

	AM_RANGE(0xf400,0xf7ff) AM_ROM AM_REGION("maincpu", 0x0800) // Modem (MOD 3)
	AM_RANGE(0xf800,0xffff) AM_ROM AM_REGION("maincpu", 0x0000) // Monitor (MON 1 + MON 2)
ADDRESS_MAP_END

static ADDRESS_MAP_START( goupil_io, AS_IO, 8, goupil_g1_state)
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

WRITE8_MEMBER( goupil_g1_state::scanlines_kbd1_w )
{
	m_row_kbd1 = data;
}

READ8_MEMBER( goupil_g1_state::ctrl_kb1_r )
{
	char kbdrow[6];
	unsigned char data;

	kbdrow[0] = 'C';
	kbdrow[1] = 'T';
	kbdrow[2] = 'R';
	kbdrow[3] = '0';
	kbdrow[4] = 0;

	data = ioport(kbdrow)->read();
	if( data & 0x02 )
		return 1;
	else
		return 0;
}

READ8_MEMBER( goupil_g1_state::ctrl_kb2_r )
{
	return 1;
}

READ8_MEMBER( goupil_g1_state::shift_kb1_r )
{
	char kbdrow[6];
	unsigned char data;

	kbdrow[0] = 'C';
	kbdrow[1] = 'T';
	kbdrow[2] = 'R';
	kbdrow[3] = '0';
	kbdrow[4] = 0;

	data = ioport(kbdrow)->read();
	if( data & 0x01 )
		return 1;
	else
		return 0;
}

READ8_MEMBER( goupil_g1_state::shift_kb2_r )
{
	return 1;
}

READ8_MEMBER( goupil_g1_state::kbd1_r )
{
	char kbdrow[6];
	UINT8 data = 0xff;

	kbdrow[0] = 'A';
	kbdrow[1] = 'X';
	kbdrow[2] = '0' + ( m_row_kbd1 & 7 ) ;
	kbdrow[3] = 0;

	data = ioport(kbdrow)->read();

	return data;
}

WRITE8_MEMBER( goupil_g1_state::scanlines_kbd2_w )
{
	m_row_kbd2 = data & 7;
}

READ_LINE_MEMBER( goupil_g1_state::via_keyb_ca2_r )
{
	return 0;
}

READ8_MEMBER( goupil_g1_state::kbd2_r )
{
	char kbdrow[6];
	UINT8 data = 0xff;

	kbdrow[0] = 'B';
	kbdrow[1] = 'X';
	kbdrow[2] = '0' + ( m_row_kbd2 & 7 ) ;
	kbdrow[3] = 0;

	data = ioport(kbdrow)->read();

	return data;
}

/* Input ports */
static INPUT_PORTS_START( goupil_g1 )
	PORT_START("AX0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_START("AX1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_START("AX2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('?')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_START("AX3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)
	PORT_START("AX4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)  PORT_CHAR(':') PORT_CHAR('/')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('.')
	PORT_START("AX5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("TAB") PORT_CODE(KEYCODE_TAB) PORT_CHAR(UCHAR_MAMEKEY(TAB))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("BS") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_START("AX6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(0x00F9) PORT_CHAR('%')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_START("AX7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("CTR0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CHAR(UCHAR_SHIFT_2)

	PORT_START("BX0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("BX1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("BX2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("BX3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_START("BX4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("BX5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_START("BX6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_START("BX7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8')
INPUT_PORTS_END

static SLOT_INTERFACE_START( goupil_floppies )
	SLOT_INTERFACE( "525qd", FLOPPY_525_QD )
SLOT_INTERFACE_END

void goupil_g1_state::machine_start()
{
	std::string region_tag;

	m_floppy = NULL;
	valkeyb = 0xFF;
}

void goupil_g1_state::machine_reset()
{
}

WRITE8_MEMBER(goupil_g1_state::via_video_pba_w)
{
	#ifdef DBGMODE
	printf("%s: write via_video_pba_w reg : 0x%X\n",machine().describe_context(),data);
	#endif
	m_ef9364->char_latch_w(data);
}

WRITE8_MEMBER(goupil_g1_state::via_video_pbb_w)
{
	#ifdef DBGMODE
	printf("%s: write via_video_pbb_w reg : 0x%X\n",machine().describe_context(),data);
	#endif
	via_video_pbb_data = data;
}

WRITE_LINE_MEMBER( goupil_g1_state::via_video_ca2_w )
{
	if(old_state_ca2==0 && state==1)
	{
		m_ef9364->command_w(via_video_pbb_data&0xF);
	}
	old_state_ca2 = state;
}

static MACHINE_CONFIG_START( goupil_g1, goupil_g1_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6808, CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(goupil_mem)
	MCFG_CPU_IO_MAP(goupil_io)

	/* sound hardware */
	// TODO !

	MCFG_DEVICE_ADD ("ef6850", ACIA6850, 0)

	/* screen */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_UPDATE_DEVICE("ef9364", ef9364_device, screen_update)

	MCFG_SCREEN_SIZE((64*8), (16*(8+4)))
	MCFG_SCREEN_VISIBLE_AREA(0, (64*8)-1, 0, (16*(8+4))-1)
	MCFG_PALETTE_ADD("palette", 16)

	MCFG_DEVICE_ADD("ef9364", EF9364, VIDEO_CLOCK)
	MCFG_EF9364_PALETTE("palette")
	MCFG_EF9364_PAGES_CNT(1);
	MCFG_TIMER_DRIVER_ADD_SCANLINE("goupil_sl", goupil_g1_state, goupil_scanline, "screen", 0, 10)

	MCFG_DEVICE_ADD("m_via_video", VIA6522, 0)
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(goupil_g1_state, via_video_pba_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(goupil_g1_state, via_video_pbb_w))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE(goupil_g1_state, via_video_ca2_w))

	MCFG_DEVICE_ADD("m_via_keyb", VIA6522, 0)
	MCFG_VIA6522_IRQ_HANDLER(DEVWRITELINE("maincpu", m6808_cpu_device, irq_line))

	MCFG_DEVICE_ADD("m_via_modem", VIA6522, 0)
	MCFG_VIA6522_IRQ_HANDLER(DEVWRITELINE("maincpu", m6808_cpu_device, irq_line))

	/* Floppy */
	MCFG_FD1791_ADD("fd1791", XTAL_8MHz )
	MCFG_FLOPPY_DRIVE_ADD("fd1791:0", goupil_floppies, "525qd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fd1791:1", goupil_floppies, "525qd", floppy_image_device::default_floppy_formats)

	MCFG_DEVICE_ADD("i8279_kb1", I8279, CPU_CLOCK)
	MCFG_I8279_OUT_SL_CB(WRITE8(goupil_g1_state, scanlines_kbd1_w))           // scan SL lines
	MCFG_I8279_IN_RL_CB(READ8(goupil_g1_state, kbd1_r))                         // kbd RL lines
	MCFG_I8279_IN_SHIFT_CB(READ8(goupil_g1_state, shift_kb1_r))
	MCFG_I8279_IN_CTRL_CB(READ8(goupil_g1_state, ctrl_kb1_r))
	MCFG_I8279_OUT_IRQ_CB(DEVWRITELINE("m_via_keyb", via6522_device, write_ca1))

	MCFG_DEVICE_ADD("i8279_kb2", I8279, CPU_CLOCK)
	MCFG_I8279_OUT_SL_CB(WRITE8(goupil_g1_state, scanlines_kbd2_w))           // scan SL lines
	MCFG_I8279_IN_RL_CB(READ8(goupil_g1_state, kbd2_r))                         // kbd RL lines
	MCFG_I8279_IN_SHIFT_CB(READ8(goupil_g1_state, shift_kb2_r))
	MCFG_I8279_IN_CTRL_CB(READ8(goupil_g1_state, ctrl_kb2_r))

MACHINE_CONFIG_END

/* ROM definition */
ROM_START( goupilg1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_DEFAULT_BIOS("v1_0")

	ROM_SYSTEM_BIOS(0, "v1_0", "Version 1.0")
	ROMX_LOAD( "SMT_Goupil_G1_MON_1.bin", 0x0000, 0x0400, CRC(98b7be69) SHA1(69e83fe78a43fcf2b08fb0bcefb0d217a57b1ecb), ROM_BIOS(1) )
	ROM_LOAD ( "SMT_Goupil_G1_MON_2.bin", 0x0400, 0x0400, CRC(19386b81) SHA1(e52f63fd29d374319781e9677de6d3fd61a3684c) )

	ROM_LOAD( "SMT_Goupil_G1_MOD_3.bin", 0x0800, 0x0400, CRC(e662f152) SHA1(11b91c5737e7572a2c18472b66bbd16b485132d5) )

	ROMX_LOAD( "SMT_Goupil_G1_Basic_1.bin", 0x1000, 0x0400, CRC(ad105b12) SHA1(631cd4b997f76b57bf2509e4bff30b1595c8bd13), ROM_BIOS(1) )
	ROMX_LOAD( "SMT_Goupil_G1_Basic_2.bin", 0x1400, 0x0400, CRC(0c5c309c) SHA1(f1cab4b0f9191e53113790a95f1ab7108f9406a1), ROM_BIOS(1) )
	ROMX_LOAD( "SMT_Goupil_G1_Basic_3.bin", 0x1800, 0x0400, CRC(1f1eb127) SHA1(dbbb880c79d515acbfcb2be9a4c96962f3e4edea), ROM_BIOS(1) )
	ROMX_LOAD( "SMT_Goupil_G1_Basic_4.bin", 0x1C00, 0x0400, CRC(09be48e4) SHA1(86cae0d159583c1d572a5754f3bb6b4a2e479359), ROM_BIOS(1) )
	ROMX_LOAD( "SMT_Goupil_G1_Basic_5.bin", 0x2000, 0x0400, CRC(bdeb395c) SHA1(32a50468f1ca772ee45a1f5c61c66f3ecc774074), ROM_BIOS(1) )
	ROMX_LOAD( "SMT_Goupil_G1_Basic_6.bin", 0x2400, 0x0400, CRC(850a4000) SHA1(720f0bb3e45877835219b7e1d943ef4f19b9977d), ROM_BIOS(1) )
	ROMX_LOAD( "SMT_Goupil_G1_Basic_7.bin", 0x2800, 0x0400, CRC(586c7670) SHA1(13e2e96b9f1a53555ce0d55f657cf3c6b96f10a0), ROM_BIOS(1) )
	ROMX_LOAD( "SMT_Goupil_G1_Basic_8.bin", 0x2C00, 0x0400, CRC(33281300) SHA1(ce631fa8157a3f8869c5fefe24b7f40e06696df9), ROM_BIOS(1) )
	ROMX_LOAD( "SMT_Goupil_G1_Basic_9.bin", 0x3000, 0x0400, CRC(a3911201) SHA1(8623a0a2d83eb3a27a795030643c5c05a4350a9f), ROM_BIOS(1) )

	ROM_REGION( 0x400, "ef9364", 0 )
	ROM_LOAD( "SMT_Goupil_G1_Charset.bin", 0x0000, 0x0400, CRC(8b6da54b) SHA1(ac2204600f45c6dd0df1e759b62ed25928f02a12) )
ROM_END

/* Driver */

/*    YEAR   NAME   PARENT  COMPAT   MACHINE    INPUT  CLASS           INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1979, goupilg1,   0,   0,      goupil_g1,  goupil_g1,driver_device,   0,     "SMT", "Goupil G1",    MACHINE_IS_SKELETON )
