// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Busicom 141-PF

        04/08/2009 Initial driver by Miodrag Milanovic

****************************************************************************/

#include "emu.h"
#include "cpu/i4004/i4004.h"
#include "includes/busicom.h"


UINT8 busicom_state::get_bit_selected(UINT32 val,int num)
{
	int i;
	for(i=0;i<num;i++) {
		if (BIT(val,i)==0) return i;
	}
	return 0;
}
READ8_MEMBER(busicom_state::keyboard_r)
{
	static const char *const keynames[] = { "LINE0", "LINE1", "LINE2", "LINE3", "LINE4", "LINE5", "LINE6", "LINE7", "LINE8" , "LINE9"};
	return ioport(keynames[get_bit_selected(m_keyboard_shifter & 0x3ff,10)])->read();
}

READ8_MEMBER(busicom_state::printer_r)
{
	UINT8 retVal = 0;
	if (m_drum_index==0) retVal |= 1;
	retVal |= ioport("PAPERADV")->read() & 1 ? 8 : 0;
	return retVal;
}


WRITE8_MEMBER(busicom_state::shifter_w)
{
	if (BIT(data,0)) {
		m_keyboard_shifter <<= 1;
		m_keyboard_shifter |= BIT(data,1);
	}
	if (BIT(data,2)) {
		m_printer_shifter <<= 1;
		m_printer_shifter |= BIT(data,1);
	}
}

WRITE8_MEMBER(busicom_state::printer_w)
{
	int i,j;
	if (BIT(data,0)) {
		logerror("color : %02x %02x %d\n",BIT(data,0),data,m_drum_index);
		m_printer_line_color[10] = 1;

	}
	if (BIT(data,1)) {
		for(i=3;i<18;i++) {
			if(BIT(m_printer_shifter,i)) {
				m_printer_line[10][i-3] = m_drum_index + 1;
			}
		}
		if(BIT(m_printer_shifter,0)) {
			m_printer_line[10][15] = m_drum_index + 13 + 1;
		}
		if(BIT(m_printer_shifter,1)) {
			m_printer_line[10][16] = m_drum_index + 26 + 1;
		}
	}
	if (BIT(data,3)) {
		for(j=0;j<10;j++) {
			for(i=0;i<17;i++) {
				m_printer_line[j][i] = m_printer_line[j+1][i];
				m_printer_line_color[j] = m_printer_line_color[j+1];
			}
		}
		for(i=0;i<17;i++) {
			m_printer_line[10][i] = 0;
		}
		m_printer_line_color[10] = 0;

	}
}
WRITE8_MEMBER(busicom_state::status_w)
{
#if 0
	UINT8 mem_lamp = BIT(data,0);
	UINT8 over_lamp = BIT(data,1);
	UINT8 minus_lamp = BIT(data,2);
#endif
	//logerror("status %c %c %c\n",mem_lamp ? 'M':'x',over_lamp ? 'O':'x',minus_lamp ? '-':'x');
}

WRITE8_MEMBER(busicom_state::printer_ctrl_w)
{
}

static ADDRESS_MAP_START(busicom_rom, AS_PROGRAM, 8, busicom_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x04FF) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(busicom_mem, AS_DATA, 8, busicom_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07F) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( busicom_io , AS_IO, 8, busicom_state )
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x00) AM_WRITE(shifter_w) // ROM0 I/O
	AM_RANGE(0x01, 0x01) AM_READWRITE(keyboard_r,printer_ctrl_w) // ROM1 I/O
	AM_RANGE(0x02, 0x02) AM_READ(printer_r)  // ROM2 I/O
	AM_RANGE(0x10, 0x10) AM_WRITE(printer_w) // RAM0 output
	AM_RANGE(0x11, 0x11) AM_WRITE(status_w)  // RAM1 output
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( busicom )
	PORT_START("LINE0")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CM") PORT_CODE(KEYCODE_1)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RM") PORT_CODE(KEYCODE_2)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M-") PORT_CODE(KEYCODE_3)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M+") PORT_CODE(KEYCODE_4)
	PORT_START("LINE1")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("SQRT") PORT_CODE(KEYCODE_S)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("%") PORT_CODE(KEYCODE_5)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M=-") PORT_CODE(KEYCODE_6)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("M=+") PORT_CODE(KEYCODE_7)
	PORT_START("LINE2")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("diamond") PORT_CODE(KEYCODE_D)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH_PAD)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("*") PORT_CODE(KEYCODE_ASTERISK)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("=") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_START("LINE3")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("+") PORT_CODE(KEYCODE_PLUS_PAD)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("diamond 2") PORT_CODE(KEYCODE_TILDE)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("000") PORT_CODE(KEYCODE_8)
	PORT_START("LINE4")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6_PAD)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3_PAD)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_START("LINE5")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5_PAD)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2_PAD)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("00") PORT_CODE(KEYCODE_9)
	PORT_START("LINE6")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7_PAD)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4_PAD)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1_PAD)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0_PAD)
	PORT_START("LINE7")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Sign") PORT_CODE(KEYCODE_RALT) PORT_CODE(KEYCODE_LALT)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("EX") PORT_CODE(KEYCODE_X)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("CE") PORT_CODE(KEYCODE_E)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_START("LINE8")
		PORT_CONFNAME( 0x0f, 0x00, "Digital point")
			PORT_CONFSETTING( 0x00, "0" )
			PORT_CONFSETTING( 0x01, "1" )
			PORT_CONFSETTING( 0x02, "2" )
			PORT_CONFSETTING( 0x03, "3" )
			PORT_CONFSETTING( 0x04, "4" )
			PORT_CONFSETTING( 0x05, "5" )
			PORT_CONFSETTING( 0x06, "6" )
			PORT_CONFSETTING( 0x08, "8" )
	PORT_START("LINE9")
		PORT_CONFNAME( 0x0f, 0x00, "Rounding")
			PORT_CONFSETTING( 0x01, "/N" )
			PORT_CONFSETTING( 0x00, "FL" )
			PORT_CONFSETTING( 0x08, "5/4" )
	PORT_START("PAPERADV")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Paper adv.") PORT_CODE(KEYCODE_SPACE)

INPUT_PORTS_END


TIMER_DEVICE_CALLBACK_MEMBER(busicom_state::timer_callback)
{
	m_timer ^=1;
	if (m_timer==1) m_drum_index++;
	if (m_drum_index==13) m_drum_index=0;
	m_maincpu->set_test(m_timer);

}

void busicom_state::machine_start()
{
}

void busicom_state::machine_reset()
{
	int i,j;
	m_drum_index =0;
	m_keyboard_shifter = 0;
	m_printer_shifter = 0;

	for(i=0;i<17;i++) {
		for(j=0;j<11;j++) {
			m_printer_line[j][i] = 0;
			m_printer_line_color[j] = 0;
		}
	}

}

//static const char layout_busicom [] = "busicom";

static MACHINE_CONFIG_START( busicom, busicom_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I4004, 750000)
	MCFG_CPU_PROGRAM_MAP(busicom_rom)
	MCFG_CPU_DATA_MAP(busicom_mem)
	MCFG_CPU_IO_MAP(busicom_io)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(40*17, 44*11)
	MCFG_SCREEN_VISIBLE_AREA(0, 40*17-1, 0, 44*11-1)
	MCFG_SCREEN_UPDATE_DRIVER(busicom_state, screen_update_busicom)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 16)
	MCFG_PALETTE_INIT_OWNER(busicom_state, busicom)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("busicom_timer", busicom_state, timer_callback, attotime::from_msec(28*2))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( busicom )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "busicom.l01", 0x0000, 0x0100, CRC(51ae2513) SHA1(5cb4097a3945db35af4ed64b629b20b08fc9824f))
	ROM_LOAD( "busicom.l02", 0x0100, 0x0100, CRC(a05411ad) SHA1(81503a99a0d34fa29bf1245de0a44af2f174abdd))
	ROM_LOAD( "busicom.l05", 0x0200, 0x0100, CRC(6120addf) SHA1(4b7ec183613630120b3c313c782122713d4327c5))
	ROM_LOAD( "busicom.l07", 0x0300, 0x0100, CRC(84a90daa) SHA1(e2931753b0fd35144cb5a9d73fcae8e104e5e3ed))
	ROM_LOAD( "busicom.l11", 0x0400, 0x0100, CRC(4d2b2942) SHA1(9a59db76eff084369797735ec19da8cbc70d0d39))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1974, busicom,  0,       0,   busicom,    busicom, driver_device,  0,  "Business Computer Corporation",   "Busicom 141-PF",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
