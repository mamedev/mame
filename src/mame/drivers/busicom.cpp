// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Busicom 141-PF

        04/08/2009 Initial driver by Miodrag Milanovic

****************************************************************************/

#include "emu.h"
#include "includes/busicom.h"

#include "screen.h"


uint8_t busicom_state::get_bit_selected(uint32_t val,int num)
{
	int i;
	for(i=0;i<num;i++) {
		if (BIT(val,i)==0) return i;
	}
	return 0;
}

READ8_MEMBER(busicom_state::keyboard_r)
{
	return m_input_lines[get_bit_selected(m_keyboard_shifter & 0x3ff, 10)]->read();
}

READ8_MEMBER(busicom_state::printer_r)
{
	uint8_t retVal = 0;
	if (m_drum_index==0) retVal |= 1;
	retVal |= ioport("PAPERADV")->read() & 1 ? 8 : 0;
	return retVal;
}


WRITE8_MEMBER(busicom_state::shifter_w)
{
	// FIXME: detect edges, maybe make 4003 shifter a device
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
	uint8_t mem_lamp = BIT(data,0);
	uint8_t over_lamp = BIT(data,1);
	uint8_t minus_lamp = BIT(data,2);
#endif
	//logerror("status %c %c %c\n",mem_lamp ? 'M':'x',over_lamp ? 'O':'x',minus_lamp ? '-':'x');
}

WRITE8_MEMBER(busicom_state::printer_ctrl_w)
{
}

void busicom_state::busicom_rom(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x04FF).rom().region("maincpu", 0);
}

void busicom_state::busicom_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07F).ram();
}

void busicom_state::busicom_stat(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x01F).ram();
}

void busicom_state::busicom_rp(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x000f).mirror(0x0700).w(FUNC(busicom_state::shifter_w)); // ROM0 I/O
	map(0x0010, 0x001f).mirror(0x0700).rw(FUNC(busicom_state::keyboard_r), FUNC(busicom_state::printer_ctrl_w)); // ROM1 I/O
	map(0x0020, 0x002f).mirror(0x0700).r(FUNC(busicom_state::printer_r));  // ROM2 I/O
}

void busicom_state::busicom_mp(address_map &map)
{
	map(0x00, 0x00).w(FUNC(busicom_state::printer_w)); // RAM0 output
	map(0x01, 0x01).w(FUNC(busicom_state::status_w));  // RAM1 output
}

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
	m_timer ^= 1;
	if (m_timer == 1) m_drum_index++;
	if (m_drum_index == 13) m_drum_index = 0;
	m_maincpu->set_input_line(I4004_TEST_LINE, m_timer);
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

void busicom_state::busicom(machine_config &config)
{
	/* basic machine hardware */
	I4004(config, m_maincpu, 750000);
	m_maincpu->set_rom_map(&busicom_state::busicom_rom);
	m_maincpu->set_ram_memory_map(&busicom_state::busicom_mem);
	m_maincpu->set_rom_ports_map(&busicom_state::busicom_rp);
	m_maincpu->set_ram_status_map(&busicom_state::busicom_stat);
	m_maincpu->set_ram_ports_map(&busicom_state::busicom_mp);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(40*17, 44*11);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(busicom_state::screen_update_busicom));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(busicom_state::busicom_palette), 16);

	TIMER(config, "busicom_timer").configure_periodic(FUNC(busicom_state::timer_callback), attotime::from_msec(28*2));
}

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

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY                          FULLNAME          FLAGS
COMP( 1974, busicom, 0,      0,      busicom, busicom, busicom_state, empty_init, "Business Computer Corporation", "Busicom 141-PF", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
