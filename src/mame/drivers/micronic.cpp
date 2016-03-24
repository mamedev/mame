// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/***************************************************************************

    Micronics 1000

    06/2010 (Sandro Ronco)
    - ROM/RAM banking
    - keypad input
    - Periodic IRQ (RTC-146818)
    - NVRAM

    TODO:
    - IR I/O port
    - LCD contrast and backlight

    NOTE:
    The display shows "TESTING..." for about 2 min before showing the information screen

    More info:
    http://www.philpem.me.uk/elec/micronic/
    http://members.lycos.co.uk/leeedavison/z80/micronic/index.html
    http://reocities.com/SiliconValley/Port/8052/

****************************************************************************/

/*

    KBD_R:  EQU 00h     ; key matrix read port
    KBD_W:  EQU 02h     ; key matrix write port
    LCD_D:  EQU 03h     ; LCD data port
    Port_04:    EQU 04h     ; IRQ hardware mask
                        ; .... ...0 = keyboard interrupt enable
                        ; .... ..0. = RTC interrupt enable
                        ; .... .0.. = IR port interrupt enable
                        ; .... 0... = main battery interrupt enable
                        ; ...0 .... = backup battery interrupt enable
    Port_05:    EQU 05h     ; interrupt flag byte
                        ; .... ...1 = keyboard interrupt
                        ; .... ..1. = RTC interrupt
                        ; .... .1.. = IR port interrupt ??
                        ; .... 1... = main battery interrupt
                        ; ...1 .... = backup battery interrupt
    Port_07:    EQU 07h     ;
                        ; .... ...x
                        ; .... ..x.
    RTC_A:  EQU 08h     ; RTC address port
    LCD_C:  EQU 23h     ; LCD command port
    RTC_D:  EQU 28h     ; RTC data port
    Port_2A:    EQU 2Ah     ;
                        ; .... ...x
                        ; .... ..x.
                        ; ...x ....
                        ; ..x. ....
    Port_2B:    EQU 2Bh     ; .... xxxx = beep tone
                        ; .... 0000 = off
                        ; .... 0001 = 0.25mS = 4.000kHz
                        ; .... 0010 = 0.50mS = 2.000kHz
                        ; .... 0011 = 0.75mS = 1.333kHz
                        ; .... 0100 = 1.00mS = 1.000kHz
                        ; .... 0101 = 1.25mS = 0.800kHz
                        ; .... 0110 = 1.50mS = 0.667kHz
                        ; .... 0111 = 1.75mS = 0.571kHz
                        ; .... 1000 = 2.00mS = 0.500kHz
                        ; .... 1001 = 2.25mS = 0.444kHz
                        ; .... 1010 = 2.50mS = 0.400kHz
                        ; .... 1011 = 2.75mS = 0.364kHz
                        ; .... 1100 = 3.00mS = 0.333kHz
                        ; .... 1101 = 3.25mS = 0.308kHz
                        ; .... 1110 = 3.50mS = 0.286kHz
                        ; .... 1111 = 3.75mS = 0.267kHz
    Port_2C:    EQU 2Ch     ;
                        ; .... ...x V24_ADAPTER IR port clock
                        ; .... ..x. V24_ADAPTER IR port data
                        ; ...1 .... = backlight on
                        ; ..xx ..xx
    Port_2D:    EQU 2Dh     ;
                        ; .... ...x
                        ; .... ..x.
    Port_33:    EQU 33h     ;
    Port_46:    EQU 46h     ; LCD contrast port
    MEM_P:  EQU 47h     ; memory page
    Port_48:    EQU 48h     ;
                        ; .... ...x
                        ; .... ..x.
    Port_49:    EQU 49h     ;
                        ; .... ...x
                        ; .... ..x.
    Port_4A:    EQU 4Ah     ; end IR port output
                        ; .... ...x
                        ; .... ..x.
                        ; ...x ....
                        ; ..x. ....
                        ; .x.. ....
                        ; x... ....
    Port_4B:    EQU 4Bh     ; IR port status byte
                        ; .... ...1 RX buffer full
                        ; ...x ....
                        ; .x.. ....
                        ; 1... .... TX buffer empty
    Port_4C:    EQU 4Ch     ;
                        ; .... ...x
                        ; x... ....
    Port_4D:    EQU 4Dh     ; IR transmit byte
    Port_4E:    EQU 4Eh     ; IR receive byte
    Port_4F:    EQU 4Fh     ;
                        ; .... ...x
                        ; .... ..x.
                        ; .... .x..
                        ; .... x...
                        ; ...x ....

*/


#include "emu.h"
#include "includes/micronic.h"
#include "rendlay.h"

READ8_MEMBER( micronic_state::keypad_r )
{
	UINT8 data = 0;

	for (UINT8 bit = 0; bit < 8; bit++)
	{
		if (m_kp_matrix & (1 << bit))
		{
			data |= m_bit0->read() & (0x01 << bit) ? 0x01 : 0x00;
			data |= m_bit1->read() & (0x01 << bit) ? 0x02 : 0x00;
			data |= m_bit2->read() & (0x01 << bit) ? 0x04 : 0x00;
			data |= m_bit3->read() & (0x01 << bit) ? 0x08 : 0x00;
			data |= m_bit4->read() & (0x01 << bit) ? 0x10 : 0x00;
			data |= m_bit5->read() & (0x01 << bit) ? 0x20 : 0x00;
		}
	}
	return data;
}

READ8_MEMBER( micronic_state::status_flag_r )
{
	return m_status_flag;
}

WRITE8_MEMBER( micronic_state::status_flag_w )
{
	m_status_flag = data;
}

WRITE8_MEMBER( micronic_state::kp_matrix_w )
{
	m_kp_matrix = data;
}

WRITE8_MEMBER( micronic_state::beep_w )
{
	UINT16 frequency[16] =
	{
			0, 4000, 2000, 1333, 1000, 800, 667, 571,
		500,  444,  400,  364,  333, 308, 286, 267
	};

	m_beep->set_clock(frequency[data & 0x0f]);
	m_beep->set_state((data & 0x0f) ? 1 : 0);
}

READ8_MEMBER( micronic_state::irq_flag_r )
{
	return (m_backbattery->read()<<4) | (m_mainbattery->read()<<3) | (keypad_r(space, offset) ? 0 : 1);
}

WRITE8_MEMBER( micronic_state::bank_select_w )
{
	if (data < 2)
	{
		m_bank1->set_entry(data);
		m_maincpu->space(AS_PROGRAM).unmap_write(0x0000, 0x7fff);
	}
	else
	{
		m_bank1->set_entry((data <= m_banks_num) ? data : m_banks_num);
		m_maincpu->space(AS_PROGRAM).install_write_bank(0x0000, 0x7fff, "bank1");
	}
}

WRITE8_MEMBER( micronic_state::lcd_contrast_w )
{
	m_lcd_contrast = data;
}

WRITE8_MEMBER( micronic_state::port_2c_w )
{
	m_lcd_backlight = BIT(data, 4);
}


/***************************************************************************
    RTC-146818
***************************************************************************/

WRITE8_MEMBER( micronic_state::rtc_address_w )
{
	m_rtc->write(space, 0, data);
}

READ8_MEMBER( micronic_state::rtc_data_r )
{
	return m_rtc->read(space, 1);
}

WRITE8_MEMBER( micronic_state::rtc_data_w )
{
	m_rtc->write(space, 1, data);
}

/***************************************************************************
    Machine
***************************************************************************/

static ADDRESS_MAP_START(micronic_mem, AS_PROGRAM, 8, micronic_state)
	AM_RANGE(0x0000, 0x7fff) AM_RAMBANK("bank1")
	AM_RANGE(0x8000, 0xffff) AM_RAM  AM_SHARE("ram_base")
ADDRESS_MAP_END

static ADDRESS_MAP_START(micronic_io, AS_IO, 8, micronic_state)
	ADDRESS_MAP_GLOBAL_MASK (0xff)

	/* keypad */
	AM_RANGE(0x00, 0x00) AM_READ(keypad_r)
	AM_RANGE(0x02, 0x02) AM_WRITE(kp_matrix_w)

	/* hd61830 */
	AM_RANGE(0x03, 0x03) AM_DEVREADWRITE(HD61830_TAG, hd61830_device, data_r, data_w)
	AM_RANGE(0x23, 0x23) AM_DEVREADWRITE(HD61830_TAG, hd61830_device, status_r, control_w)

	/* rtc-146818 */
	AM_RANGE(0x08, 0x08) AM_WRITE(rtc_address_w)
	AM_RANGE(0x28, 0x28) AM_READWRITE(rtc_data_r, rtc_data_w)

	/* sound */
	AM_RANGE(0x2b, 0x2b) AM_WRITE(beep_w)

	/* basic machine */
	AM_RANGE(0x05, 0x05) AM_READ(irq_flag_r)
	AM_RANGE(0x2c, 0x2c) AM_WRITE(port_2c_w)
	AM_RANGE(0x47, 0x47) AM_WRITE(bank_select_w)
	AM_RANGE(0x46, 0x46) AM_WRITE(lcd_contrast_w)
	AM_RANGE(0x48, 0x48) AM_WRITE(status_flag_w)
	AM_RANGE(0x49, 0x49) AM_READ(status_flag_r)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( micronic )
	PORT_START("MAINBATTERY")
		PORT_CONFNAME( 0x01, 0x01, "Main Battery Status" )
		PORT_CONFSETTING( 0x01, DEF_STR( Normal ) )
		PORT_CONFSETTING( 0x00, "Low Battery" )
	PORT_START("BACKBATTERY")
		PORT_CONFNAME( 0x01, 0x01, "Backup Battery Status" )
		PORT_CONFSETTING( 0x01, DEF_STR( Normal ) )
		PORT_CONFSETTING( 0x00, "Low Battery" )

	PORT_START("BIT0")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("MODE") PORT_CODE(KEYCODE_EQUALS)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("A (") PORT_CODE(KEYCODE_A)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("B )") PORT_CODE(KEYCODE_B)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("2nd") PORT_CODE(KEYCODE_LSHIFT)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("U 1") PORT_CODE(KEYCODE_1)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("BACKSPACE") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_START("BIT1")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C") PORT_CODE(KEYCODE_C)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D DEL") PORT_CODE(KEYCODE_D)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("E #") PORT_CODE(KEYCODE_E)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F &") PORT_CODE(KEYCODE_F)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("V 2") PORT_CODE(KEYCODE_2)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("SPACE 0") PORT_CODE(KEYCODE_0)
	PORT_START("BIT2")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("G +") PORT_CODE(KEYCODE_G)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("H /") PORT_CODE(KEYCODE_H)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("I ,") PORT_CODE(KEYCODE_I)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("J ?") PORT_CODE(KEYCODE_J)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("W 3") PORT_CODE(KEYCODE_3)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("NO") PORT_CODE(KEYCODE_PGUP)
	PORT_START("BIT3")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("K -") PORT_CODE(KEYCODE_K)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("L *") PORT_CODE(KEYCODE_L)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("M .") PORT_CODE(KEYCODE_M)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("N Z") PORT_CODE(KEYCODE_Z)
		PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("ENTER") PORT_CODE(KEYCODE_ENTER)
		PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("YES") PORT_CODE(KEYCODE_PGDN)
	PORT_START("BIT4")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("O 7") PORT_CODE(KEYCODE_7)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("P 8") PORT_CODE(KEYCODE_8)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Q 9") PORT_CODE(KEYCODE_9)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("DEPT") PORT_CODE(KEYCODE_R)
	PORT_START("BIT5")
		PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("R 4") PORT_CODE(KEYCODE_4)
		PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("S 5") PORT_CODE(KEYCODE_5)
		PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("T 6") PORT_CODE(KEYCODE_6)
		PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("END") PORT_CODE(KEYCODE_END)
INPUT_PORTS_END


void micronic_state::nvram_init(nvram_device &nvram, void *data, size_t size)
{
	m_status_flag = 0;
}


PALETTE_INIT_MEMBER(micronic_state, micronic)
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

void micronic_state::machine_start()
{
	/* ROM banks */
	m_bank1->configure_entries(0x00, 0x02, memregion(Z80_TAG)->base(), 0x10000);

	/* RAM banks */
	m_banks_num = (m_ram->size() >> 15) + 1;
	m_bank1->configure_entries(0x02, m_banks_num - 1, m_ram->pointer(), 0x8000);

	m_nvram1->set_base(m_ram_base, 0x8000);
	m_nvram2->set_base(m_ram->pointer(), m_ram->size());

	/* register for state saving */
	save_item(NAME(m_banks_num));
	save_item(NAME(m_kp_matrix));
	save_item(NAME(m_lcd_contrast));
	save_item(NAME(m_lcd_backlight));
	save_item(NAME(m_status_flag));
	// TODO: restore RAM bank at state load...
}

void micronic_state::machine_reset()
{
	m_bank1->set_entry(0);
	m_maincpu->space(AS_PROGRAM).unmap_write(0x0000, 0x7fff);
}


WRITE_LINE_MEMBER( micronic_state::mc146818_irq )
{
	m_maincpu->set_input_line(0, !state ? HOLD_LINE : CLEAR_LINE);
}


static MACHINE_CONFIG_START( micronic, micronic_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_3_579545MHz)
	MCFG_CPU_PROGRAM_MAP(micronic_mem)
	MCFG_CPU_IO_MAP(micronic_io)

	/* video hardware */
	MCFG_SCREEN_ADD(SCREEN_TAG, LCD)
	MCFG_SCREEN_REFRESH_RATE(80)
	MCFG_SCREEN_UPDATE_DEVICE(HD61830_TAG, hd61830_device, screen_update)
	MCFG_SCREEN_SIZE(120, 64)   //6x20, 8x8
	MCFG_SCREEN_VISIBLE_AREA(0, 120-1, 0, 64-1)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(micronic_state, micronic)

	MCFG_DEVICE_ADD(HD61830_TAG, HD61830, XTAL_4_9152MHz/2/2)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO( "mono" )
	MCFG_SOUND_ADD( "beeper", BEEP, 0 )
	MCFG_SOUND_ROUTE( ALL_OUTPUTS, "mono", 1.00 )

	/* ram banks */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("224K")

	MCFG_NVRAM_ADD_CUSTOM_DRIVER("nvram1", micronic_state, nvram_init)  // base ram
	MCFG_NVRAM_ADD_CUSTOM_DRIVER("nvram2", micronic_state, nvram_init)  // additional ram banks

	MCFG_MC146818_ADD( MC146818_TAG, XTAL_32_768kHz )
	MCFG_MC146818_IRQ_HANDLER(WRITELINE(micronic_state, mc146818_irq))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( micronic )
	ROM_REGION( 0x18000, Z80_TAG, 0 )
	ROM_SYSTEM_BIOS(0, "v228", "Micronic 1000")
	ROMX_LOAD( "micron1.bin", 0x0000, 0x8000, CRC(5632c8b7) SHA1(d1c9cf691848e9125f9ea352e4ffa41c288f3e29), ROM_BIOS(1))
	ROMX_LOAD( "micron2.bin", 0x10000, 0x8000, CRC(dc8e7341) SHA1(927dddb3914a50bb051256d126a047a29eff7c65), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "test", "Micronic 1000 LCD monitor")
	ROMX_LOAD( "monitor2.bin", 0x0000, 0x8000, CRC(c6ae2bbf) SHA1(1f2e3a3d4720a8e1bb38b37f4ab9e0e32676d030), ROM_BIOS(2))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 198?, micronic,  0,       0,  micronic,   micronic, driver_device,     0,  "Victor Micronic",   "Micronic 1000",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
