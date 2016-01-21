// license:LGPL-2.1+
// copyright-holders:Dirk Verwiebe, Cowering
/******************************************************************************
 Mephisto Chess Computers using plugin modules

 Vancouver Chess Computer 68000 (driver recalled) by R. Schaefer (2010)

 (most of the magnetic sensor versions with 680x0 family modules)

 Almeria 68000 12Mhz
 Almeria 68020 12Mhz
 Lyon 68000 12Mhz
 Lyon 68020 12Mhz
 Vancouver 68000 12Mhz
 Vancouver 68020 12Mhz
 Genius 68030 V4.00 33.333 Mhz
 Genius 68030 V4.01 33.333 Mhz
 Genius 68030 V4.01 33.333x2 Mhz (custom MESS overclocked version for higher ELO)
 Berlin Pro 68020 24.576 Mhz (not modular board, but otherwise close to milano)
 Berlin Pro (London) 68020 24.576 Mhz (not modular board, but otherwise close to milano)
 London 68030 V5.00k 33.333 Mhz (probably the Genius 3/4 update ROM)

 note: The Almeria 68000 is the 'parent' of all these, but Vancouver 68000 is used as parent in the driver.
       It was the first to have 'Bavaria' sensors, and will make the other drivers easier when inheriting this
       feature later


 misc non-modular/non-68K boards

 RISC 2500 (Skeleton, does not work, and not sure the same internal chessboard connections are used)
 Academy 65C02 4.9152mhz
 Monte Carlo IV, Mega IV and any others not listed above have correct ROM/RAM and maybe LCD, not much else for now

 Novag boards: (should go in different .c eventually)
 Super Forte and Super Expert are the same, except one is touch board, other is magnetic.
 The Diablo 68000 is Super Expert board but 68K based

 Notes by Cowering (2011)

 TODO:   add Bavaria sensor support (unknown1,2,3 handlers in current driver)
         proper 'bezel' for all games/cpuspeeds so 'Vancouver' does not say 'Almeria', etc
         custom handler to read/write the Battery RAM so 68000 can share files with 020/030 (real modular machine can do this)
         find a timing to let the player see the flashing LEDs on the 68030 versions (might be fixed with waitstates in core)
         add the missing machines.. including all Portorose and the very rare overclocked 'TM' Tournament Machines
         match I/S= diag speed test with real hardware (good test for proper waitstates)
         load LCD rom inside the LCD driver

 TODO2:  (these worked before .140 core)
         LCD off by a couple of pixels in artwork now
         no longer see the custom white hand when selecting chess pieces
         parent/clone for artwork so there are not multiple copies of chessboard+pieces


 TODO3:  Oct 2011 - commented out all ACIA functions until they are ported to new device

 Undocumented buttons: holding ENTER and LEFT cursor on cold boot runs diagnostics on modular 680x0 boards
                       holding UP and RIGHT cursor will clear the Battery Backed RAM on modular 680x0 boards
                       holding CLEAR clears Battery Backed RAM on the Berlin (Pro) 68020
******************************************************************************/

/******************************************************************************
 Mephisto Polgar & Milano
 2010 Dirk V., Ralf Schafer, Cowering

******************************************************************************/

/*

CPU 65C02 P4 4.9152 MHz

// lese Tastatur von 2c00 - 2c07,
2c00 Trn
2c01 Info
2c02 Mem
2c03 Pos
2c04 Lev
2c05 Fct
2c06 Ent
2c07 CL

$1ff0
Bit 0 LCD Command/Data  0/1
Bit 1 LCD Enable (signals Data is valid)
Bit 2 Beeper
Bit 3 Beeper
Bit 4+7  LED A-H enable
Bit 5+6  LED 1-8 enable
*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6502/m65c02.h"
#include "cpu/arm/arm.h"
#include "sound/beep.h"
//#include "machine/mos6551.h"
#include "video/hd44780.h"

#include "rendlay.h"

#include "includes/mboard.h"
#include "machine/nvram.h"

//static UINT16 unknown2_data = 0;
// Berlin Pro 68020
static UINT32 BPL32latch_data = 0;


// Mephisto Academy
static UINT8 academyallowNMI = 0;

//Monte Carlo IV key/board selects

static UINT8 monteciv_select[2] = { 0xff,0xff };
static UINT8 montecivtop = 0, montecivbot = 0;
static UINT8 montecivtopnew = 0, montecivbotnew = 0;

//Mephisto Mega IV latch
UINT8 latch2400 = 0;

UINT8 diablo68_3c0000 = 0;

UINT8 sfortea_latch = 0;


INPUT_PORTS_EXTERN( chessboard );

UINT8 lcd32_char;

class polgar_state : public mboard_state
{
public:
	polgar_state(const machine_config &mconfig, device_type type, const char *tag)
		: mboard_state(mconfig, type, tag),
			m_lcdc(*this, "hd44780"),
			m_beeper(*this, "beeper")
		{ }

	optional_device<hd44780_device> m_lcdc;
	optional_device<beep_device> m_beeper;

	UINT8 led_status;
	UINT8 lcd_char;
	//UINT8 led7;
	UINT8 latch_data;
	DECLARE_WRITE8_MEMBER(write_polgar_IO);
	DECLARE_WRITE8_MEMBER(write_LCD_polgar);
	DECLARE_READ8_MEMBER(read_1ff1_sfortea);
	DECLARE_READ8_MEMBER(read_1ff0_sfortea);
	DECLARE_WRITE8_MEMBER(write_latch_sfortea);
	DECLARE_WRITE8_MEMBER(write_lcd_IO_sfortea);
	DECLARE_WRITE8_MEMBER(write_LCD_academy);
//  DECLARE_WRITE16_MEMBER(diablo68_aciawrite);
//  DECLARE_READ16_MEMBER(diablo68_aciaread);
	DECLARE_WRITE16_MEMBER(diablo68_reg_select);
	DECLARE_WRITE16_MEMBER(diablo68_write_LCD);
	DECLARE_WRITE8_MEMBER(milano_write_LED);
	DECLARE_WRITE8_MEMBER(megaiv_write_LED);
	DECLARE_WRITE8_MEMBER(academy_write_LED);
	DECLARE_WRITE8_MEMBER(academy_inhibitNMI);
	DECLARE_WRITE32_MEMBER(write_LED_BPL32);
	DECLARE_WRITE8_MEMBER(polgar_write_LED);
	DECLARE_WRITE8_MEMBER(monteciv_write_LCD);
	DECLARE_WRITE8_MEMBER(monteciv_3007);
	DECLARE_WRITE8_MEMBER(monteciv_3005);
	DECLARE_WRITE8_MEMBER(monteciv_3006);
	DECLARE_WRITE8_MEMBER(academy_write_board);
	DECLARE_WRITE8_MEMBER(milano_write_board);
	DECLARE_READ8_MEMBER(milano_read_board);
	DECLARE_READ8_MEMBER(read_keys);
	DECLARE_READ8_MEMBER(read_keys_megaiv);
	DECLARE_READ32_MEMBER(read_keys_BPL32);
	DECLARE_WRITE8_MEMBER(beep_academy);
	DECLARE_WRITE8_MEMBER(megaiv_IO);
	DECLARE_WRITE8_MEMBER(monteciv_select_line);
	DECLARE_READ8_MEMBER(read_keys_board_monteciv);
	DECLARE_READ8_MEMBER(read_keys_board_academy);
	DECLARE_WRITE32_MEMBER(write_board_BPL32);
	DECLARE_READ32_MEMBER(read_buttons_gen32);
	DECLARE_READ32_MEMBER(read_buttons_van32);
	DECLARE_READ16_MEMBER(read_buttons_van16);
	DECLARE_WRITE32_MEMBER(write_LCD_data_32);
	DECLARE_WRITE16_MEMBER(write_LCD_data);
	DECLARE_WRITE32_MEMBER(write_IOenables_32);
	DECLARE_WRITE16_MEMBER(write_IOenables);
	DECLARE_READ32_MEMBER(read_unknown1_32);
	DECLARE_READ16_MEMBER(read_unknown1);
	DECLARE_WRITE32_MEMBER(write_unknown2_32);
	DECLARE_WRITE16_MEMBER(write_unknown2);
	DECLARE_READ32_MEMBER(read_unknown3_32);
	DECLARE_READ16_MEMBER(read_unknown3);
	DECLARE_READ32_MEMBER(read_1800000);
	DECLARE_WRITE32_MEMBER(write_1000000);
	DECLARE_DRIVER_INIT(polgar);
	DECLARE_MACHINE_START(polgar);
	DECLARE_MACHINE_RESET(polgar);
	DECLARE_MACHINE_START(sfortea);
	DECLARE_MACHINE_RESET(sfortea);
	DECLARE_MACHINE_START(van32);
	DECLARE_MACHINE_RESET(van16);
	DECLARE_MACHINE_RESET(monteciv);
	DECLARE_MACHINE_START(diablo68);
	DECLARE_MACHINE_START(van16);
	DECLARE_MACHINE_START(risc);
	DECLARE_MACHINE_RESET(academy);
	DECLARE_PALETTE_INIT(chess_lcd);
	TIMER_DEVICE_CALLBACK_MEMBER(cause_nmi);
	TIMER_DEVICE_CALLBACK_MEMBER(cause_M6502_irq);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_update_irq6);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_update_irq2);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_update_irq_academy);
	void common_chess_start();
	UINT8 convert_imputmask(UINT8 input);
	UINT8 convertMCIV2LED(UINT8 codedchar);
	void write_IOenable(unsigned char data,address_space &space);
};

UINT8 polgar_state::convert_imputmask(UINT8 input)
{
	input^=0xff;
	switch (input) {
		case 0x01:
			return 0x80;
		case 0x02:
			return 0x40;
		case 0x04:
			return 0x20;
		case 0x08:
			return 0x10;
		case 0x10:
			return 0x08;
		case 0x20:
			return 0x04;
		case 0x40:
			return 0x02;
		case 0x80:
			return 0x01;
		default:
			return 0x00;
		}
}

WRITE8_MEMBER(polgar_state::write_polgar_IO)
{
	int i;

	if (BIT(data,1)) {
		m_lcdc->write(space, BIT(data,0), lcd_char);
	}

	if (BIT(data,2) || BIT(data,3))
			m_beeper->set_state(1);
		else
			m_beeper->set_state(0);

	if (BIT(data,7) && BIT(data, 4)) {
		for (i = 0;i < 8;i++)
		output().set_led_value(i,!BIT(latch_data,i));
	}
	else if (BIT(data,6) && BIT(data,5)) {
		for (i = 0;i < 8;i++)
		output().set_led_value(10+i,!BIT(latch_data,7-i));
	}
	else if (!data && (!strcmp(machine().system().name,"milano"))) {
		for (i=0;i<8;i++) {
			output().set_led_value(i,!BIT(latch_data,i));
			output().set_led_value(10+i,!BIT(latch_data,7-i));
		}
	}


	//logerror("LCD Status  Data = %d\n",data);

}


WRITE8_MEMBER(polgar_state::write_LCD_polgar)
{
	lcd_char = data;

	logerror("LCD Data = %d %02x [%c]\n",data,data,(data&0xff));

}

READ8_MEMBER(polgar_state::read_1ff1_sfortea)
{
	UINT8 data;
	data = ioport("BUTTONS_SFOR1")->read();
	logerror("1ff0 data %02x\n",data);
	return 0;
//  return data;
}

READ8_MEMBER(polgar_state::read_1ff0_sfortea)
{
	UINT8 data;
	data = ioport("BUTTONS_SFOR2")->read();
	logerror("1ff0 data %02x\n",data);
	return 0;
//  return data;

//  static UINT8 temp = 0;
//  temp++;
//  logerror("read 1ff0 %02x\n",temp);
//  printf("read 1ff0 %02x\n",temp);

//  return (0x00);
}



WRITE8_MEMBER(polgar_state::write_latch_sfortea)
{
	sfortea_latch = data;
	logerror("latch data %02x\n",data);
//  printf("latch data %02x\n",data);

}

WRITE8_MEMBER(polgar_state::write_lcd_IO_sfortea)
{
/* bits
7
6 paired with 5
5
4
3 irq on off?
2 select HD44780
1
0 LCD command/data select
*/


	if (BIT(sfortea_latch,2))
	{
		if(BIT(sfortea_latch,0)) {
			m_lcdc->data_write(space, 0, data);
			logerror("LCD DTA = %02x\n",data);
		} else {
			if (BIT(data,7)) {
				if ((data & 0x7f) >= 0x40) data -= 56;  // adjust for 16x1 display as 2 sets of 8
			}
			m_lcdc->control_write(space, 0, data);
			logerror("LCD CMD = %02x\n",data);
		}
	}
}

WRITE8_MEMBER(polgar_state::write_LCD_academy)
{
	m_lcdc->write(space, offset & 1, data);
}

//  AM_RANGE( 0x3a0000,0x3a0000 ) AM_READ(diablo68_write_LCD)
//  AM_RANGE( 0x3c0000,0x3c0000 ) AM_READ(diablo68_reg_select)

/*WRITE16_MEMBER(polgar_state::diablo68_aciawrite)
{
//  device_t *acia = machine().device("acia65c51");
//  acia_6551_w(acia, offset & 0x03, data >> 8);
//  printf("ACIA write data %04x offset %02x\n",data,offset);
    logerror("ACIA write data %04x offset %02x\n",data,offset);
}

READ16_MEMBER(polgar_state::diablo68_aciaread)
{
//  device_t *acia = machine().device("acia65c51");
    UINT16 result;
//  result = acia_6551_r(acia, offset & 0x03);
//  logerror("ACIA read offset %02x\n",offset);
//  printf("ACIA read offset %02x\n",offset);

    result <<= 8;

    return result;
}
*/

WRITE16_MEMBER(polgar_state::diablo68_reg_select)
{
	diablo68_3c0000 = data >> 8;
	//printf("3c0000 = %04x\n",data>>8);
	logerror("3c0000 = %04x\n",data>>8);

}


WRITE16_MEMBER(polgar_state::diablo68_write_LCD)
{
	data >>= 8;
	if (!(diablo68_3c0000 & 0x02)) {
		if (BIT(data,7)) {
			if ((data & 0x7f) >= 0x40) data -= 56;  // adjust for 16x1 display as 2 sets of 8
		}
		m_lcdc->control_write(space, 0, data);
//      logerror("Control %02x\n", data);
//      printf("Control %02x\n", data);
	} else {
		m_lcdc->data_write(space, 0, data);
//      printf("LCDdata %04x [%c]\n", data,data);
//      logerror("LCDdata %04x [%c]\n", data,data);
	}

	//logerror("LCD Status  Data = %d\n",data);

}

WRITE8_MEMBER(polgar_state::milano_write_LED)
{
	UINT8 LED_offset = 100;
	if (data == 0xff)   output().set_led_value(LED_offset+offset,1);
	else                output().set_led_value(LED_offset+offset,0);

	//logerror("LEDs  Offset = %d Data = %d\n",offset,data);
}

WRITE8_MEMBER(polgar_state::megaiv_write_LED)
{
	if (BIT(data,7))
		m_beeper->set_state(1);
	else
		m_beeper->set_state(0);
	output().set_led_value(102,BIT(data,1)?1:0);
	output().set_led_value(107,BIT(data,6)?1:0);

//  logerror("LEDs  FUNC = %02x found = %d\n",data,found);
	logerror("LED mask %d\n",data);
//  printf("LED mask %d\n",data);

}


WRITE8_MEMBER(polgar_state::academy_write_LED)
{
int found = 0;

// LED mask debugging, please don't delete
/*static int r[4] = { 0,0,0,0 };
static int pr[4] = { -1,-1,-1,-1 };
static int new1 = 0,start = 0,z = 0,i2;
if (data == 0xf8) {
    if (start == 0) {
        start = 1;
        z = 0;
    }
}
if (start == 1) {
    new1 = 0;
    pr[z] = r[z];
    r[z] = data;
    for (i2 = 0; i2 < 4; i2++) if(pr[i2] != r[i2]) new1 = 1;
    z++;
    if(z == 4) z = 0;
    if (new1) printf("[%02x] [%02x] [%02x] [%02x]\n",r[0],r[1],r[2],r[3]);
}
*/

if ((data & 0x68) == 0x68) {
	output().set_led_value(103,BIT(data,4)?0:1); // POS
	output().set_led_value(107,BIT(data,7)?0:1); // white
	found = 1;
}

if ((data & 0x64) == 0x64) {
	output().set_led_value(102,BIT(data,4)?0:1); // MEM
	output().set_led_value(106,BIT(data,7)?0:1); // black
	found = 1;
}

if ((data & 0xa2) == 0xa2) {
	output().set_led_value(101,BIT(data,4)?0:1); // INFO
	output().set_led_value(105,BIT(data,6)?0:1); // FUNC
	found = 1;
}

if ((data & 0xa1) == 0xa1) {
	output().set_led_value(100,BIT(data,4)?0:1); // TRN
	output().set_led_value(104,BIT(data,6)?0:1); // LVL
	found = 1;
}

if (BIT(data,7))
	m_beeper->set_state(1);
else
	m_beeper->set_state(0);
if (BIT(data,1))
	m_beeper->set_state(1);
else
	m_beeper->set_state(0);
//  logerror("LEDs  FUNC = %02x found = %d\n",data,found);
	if (!found) {
		logerror("unknown LED mask %d\n",data);
//      printf("unknown LED mask %d\n",data);
	}
}


WRITE8_MEMBER(polgar_state::academy_inhibitNMI)
{
	academyallowNMI = data;

}


WRITE32_MEMBER(polgar_state::write_LED_BPL32)
{
	int i;

	data >>= 24;
	for (i=0;i<8;i++) {
		output().set_led_value(i,BIT(data,i));
		output().set_led_value(10+i,!BIT(BPL32latch_data,7-i));
	}

	logerror("LEDs  Offset = %d Data = %08x Latch = %08x\n",offset,data,BPL32latch_data);

}


WRITE8_MEMBER(polgar_state::polgar_write_LED)
{
#define LED_offset 100

	data &= 0x80;

	if (data == 0) {
		led_status &= 255-(1<<offset);
	} else {
		led_status|=1<<offset;
	}

	if (offset < 6) output().set_led_value(LED_offset+offset, led_status&1<<offset?1:0);
	logerror("LEDs  Offset = %d Data = %d\n",offset,data);
}

UINT8 polgar_state::convertMCIV2LED(UINT8 codedchar)
{
	UINT8 data = 0;
	if (BIT(codedchar,0)) data |= 0x80;
	if (BIT(codedchar,1)) data |= 0x01;
	if (BIT(codedchar,2)) data |= 0x20;
	if (BIT(codedchar,3)) data |= 0x40;
	if (BIT(codedchar,4)) data |= 0x02;
	if (BIT(codedchar,5)) data |= 0x04;
	if (BIT(codedchar,6)) data |= 0x08;
	if (BIT(codedchar,7)) data |= 0x10;
	return data;
}

WRITE8_MEMBER(polgar_state::monteciv_write_LCD)
// first write == 00 (start) then 8 writes (to 74595) x 4 chars, then ff to finish
{
	static UINT8 charstodisplay[4] = { 0,0,0,0 };
	static UINT8 tempchar = 0,shift = 0, whichchar = 0;

	if ((montecivtop == 0) && (montecivbot == 0)) {  // if both are 0 then no screen chosen
		montecivbotnew = 1;
		montecivtopnew = 1;
		logerror("no screen!\n");
		//printf("no screen! %02x\n",1/(montecivtopnew-1));
		return;
	}
	if (montecivtop == 0xff) {
		if (montecivtopnew) {
			montecivtopnew = 0; // skip start bit reset pointer for new chars
			tempchar = 0;
			shift = 1;
			whichchar = 0;
			return;
		}
		tempchar |= BIT(data,7);
		tempchar <<= 1;
		shift++;
		if (shift == 8) {
			shift = 0;
			charstodisplay[whichchar] = convertMCIV2LED(tempchar);
			whichchar++;
			tempchar = 0;
		}
		if (whichchar == 4) {
			output().set_digit_value(0,charstodisplay[0]);
			output().set_digit_value(1,charstodisplay[1]);
			output().set_digit_value(2,charstodisplay[2]);
			output().set_digit_value(3,charstodisplay[3]);
			whichchar = 0;
		}
	}
	if (montecivbot == 0xff) {
		if (montecivbotnew) {
			montecivbotnew = 0; // skip start bit reset pointer for new chars
			tempchar = 0;
			shift = 1;
			whichchar = 0;
			return;
		}
		tempchar |= BIT(data,7);
		tempchar <<= 1;
		shift++;
		if (shift == 8) {
			shift = 0;
			charstodisplay[whichchar] = convertMCIV2LED(tempchar);
			whichchar++;
			tempchar = 0;
		}
		if (whichchar == 4) {
			output().set_digit_value(4+0,charstodisplay[0]);
			output().set_digit_value(4+1,charstodisplay[1]);
			output().set_digit_value(4+2,charstodisplay[2]);
			output().set_digit_value(4+3,charstodisplay[3]);
			whichchar = 0;
		}
	}
//    logerror("$3004 %02x\n",data);
//    printf("$3004 %02x %02x \n",data,tempchar);
}

WRITE8_MEMBER(polgar_state::monteciv_3007)
{
//    logerror("$3007 SELECTTOP %02x\n",data);
//    printf("$3007 SELECTTOP %02x\n",data);
	montecivtop = data;
	montecivtopnew = 1;
}

WRITE8_MEMBER(polgar_state::monteciv_3005)
{
//    logerror("$3005 SELECTBOT %02x\n",data);
//    printf("$3005 SELECTBOT %02x\n",data);
	montecivbot = data;
	montecivbotnew = 1;
}

WRITE8_MEMBER(polgar_state::monteciv_3006)
{
	logerror("$3006 CLK %02x\n",data);
//    printf("$3006 CLK %02x\n",data);
}


WRITE8_MEMBER(polgar_state::academy_write_board)
{
	latch_data = data;
//    logerror("acad_write_latch %02x\n",data);
	if (data != 0xff) mboard_write_board_8(space,0, data);
}

WRITE8_MEMBER(polgar_state::milano_write_board)
{
	latch_data = data;
}

READ8_MEMBER(polgar_state::milano_read_board)
{
	int line;
	static const char *const board_lines[8] =
			{ "LINE.0", "LINE.1", "LINE.2", "LINE.3", "LINE.4", "LINE.5", "LINE.6", "LINE.7" };

	UINT8 data = 0x00;
	UINT8 tmp; // = 0xff;

	if (latch_data)
	{
		line = get_first_cleared_bit(latch_data);
		tmp = ioport(board_lines[line])->read();

		if (tmp != 0xff)
			data = convert_imputmask(tmp);

	}

	return data;

}

READ8_MEMBER(polgar_state::read_keys)
{
	UINT8 data;
	static const char *const keynames[1][8] =
	{
		{ "KEY1_0", "KEY1_1", "KEY1_2", "KEY1_3", "KEY1_4", "KEY1_5", "KEY1_6", "KEY1_7" }
	};

	data = ioport(keynames[0][offset])->read();
	// logerror("Keyboard Port = %s Data = %d\n  ", ((led_status & 0x80) == 0x00) ? keynames[0][offset] : keynames[1][offset], data);
	return data | 0x7f;
}

READ8_MEMBER(polgar_state::read_keys_megaiv)
{
	UINT8 data;
	static const char *const keynames[1][8] =
	{
		{ "KEY1_0", "KEY1_1", "KEY1_2", "KEY1_3", "KEY1_4", "KEY1_5", "KEY1_6", "KEY1_7" }
	};

	data = ioport(keynames[0][offset])->read();
	logerror("Keyboard Port = %s Data = %d\n  ", keynames[0][offset] , data);
	return data | 0x7f;
}


READ32_MEMBER(polgar_state::read_keys_BPL32)
{
	UINT32 data = 0;
	UINT8 tmp = 0xff, line = 0;
	static const char *const board_lines[8] =
			{ "LINE.0", "LINE.1", "LINE.2", "LINE.3", "LINE.4", "LINE.5", "LINE.6", "LINE.7" };

	if (BPL32latch_data == 0xff) {
		tmp = ioport("BUTTONS_BPL")->read();
		logerror("Keyboard Port Offset = %d tmp %d\n", offset,tmp);

		data = tmp << 24;
	} else {
		if (BPL32latch_data & 0x7f) {
			logerror("ReadingBoard %02x\n",BPL32latch_data);
			line = get_first_cleared_bit(BPL32latch_data);
			tmp = ioport(board_lines[line])->read();

			if (tmp != 0xff)
				data = convert_imputmask(tmp) << 24;
		}
	}

//  logerror("Keyboard Port = %s Data = %d Offset = %d tmp %d line %02x\n", keynames[0][line], data, offset,tmp,line);
	return data;
}

WRITE8_MEMBER(polgar_state::beep_academy)
{
	if (!BIT(data,7))
			m_beeper->set_state(1);
		else
			m_beeper->set_state(0);
}

WRITE8_MEMBER(polgar_state::megaiv_IO)
{
//  if (BIT(data,0)) beep_set_state(machine->device("beeper"),1); else beep_set_state(machine->device("beeper"),0);
	logerror("$2400 = %02x\n",data);
	latch2400 = data;
}


WRITE8_MEMBER(polgar_state::monteciv_select_line)
{
	monteciv_select[offset] = data;
}

// FIXME : unlike polgar, academy shares port IO for keys and board, and i just can't seem to get the board latched right (H7 and H8 are always flashing) -- Cow
READ8_MEMBER(polgar_state::read_keys_board_monteciv)
{
	UINT8 data = 0;

	if (monteciv_select[0] == 0xff && monteciv_select[1] == 0xff) {
			data = mboard_read_board_8(space,0);
	} else {
		if (monteciv_select[0] == 0x0) {
			data = ioport("BUTTONS_MONTE2")->read();
#if 0
			if (data) {
				output().set_digit_value(0,64);
				output().set_digit_value(1,113+128);
				output().set_digit_value(2,190);
				output().set_digit_value(3,64);

				output().set_digit_value(4,246-128);
				output().set_digit_value(5,247-128);
				output().set_digit_value(6,219-128);
				output().set_digit_value(7,249-128);
			}
#endif
		} else {
			data = ioport("BUTTONS_MONTE1")->read();
		}
	}
	return data;
}


// FIXME : unlike polgar, academy shares port IO for keys and board, and I just can't seem to get the board latched right (H7 and H8 are always flashing) -- Cow
READ8_MEMBER(polgar_state::read_keys_board_academy)
{
//  static int startup = 0;
	UINT8 data = 0;
//  UINT8 tmp = 0xff, line = 0;
//  static const char *const board_lines[8] =
//          { "LINE.0", "LINE.1", "LINE.2", "LINE.3", "LINE.4", "LINE.5", "LINE.6", "LINE.7" };

	if (latch_data == 0xff) {
		data = ioport("BUTTONS_ACAD")->read();
	} else {
//      if (latch_data & 0x7f) {
		data = mboard_read_board_8(space,0);
//      data = milano_read_board(space,0);

//          logerror("ReadingBoard %02x\n",latch_data);
//          line = get_first_cleared_bit(latch_data);
//          tmp = machine.root_device().ioport(board_lines[line])->read();
//          mboard_write_board_8(space,0, latch_data);
//          data = mboard_read_board_8(space,0);
//          logerror("BoardRead Port Offset = %d data %02x Latch %02x\n", offset,data,latch_data);
//          printf  ("BoardRead Port Offset = %d data %02x Latch %02x\n", offset,data,latch_data);
//      } else {
//          logerror("no keys or board\n");
//      }
	}
//  logerror("Keyboard Port Offset = %d tmp %d Latch %d\n", offset,tmp,latch_data);
//  logerror("Keyboard Port = %s Data = %d Offset = %d tmp %d line %02x\n", keynames[0][line], data, offset,tmp,line);
	return data;
}

TIMER_DEVICE_CALLBACK_MEMBER(polgar_state::cause_nmi)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI,PULSE_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(polgar_state::cause_M6502_irq)
{
	m_maincpu->set_input_line(M65C02_IRQ_LINE, HOLD_LINE);
}


WRITE32_MEMBER(polgar_state::write_board_BPL32)
{
	BPL32latch_data = data >> 24;
	logerror("Write BPL32 board Data Latch %08x Offset: %08x\n",BPL32latch_data,offset);

}

READ32_MEMBER(polgar_state::read_buttons_gen32)
{
	UINT32 data;
	static const char *const keynames[4] = { "BUTTON_1", "BUTTON_2", "", "BUTTON_3" };

	data = ioport(keynames[offset])->read();

	data = data|data<<8|data<<16|data<<24;  // this might not be needed if MAME does handle odd alignment over 32bit boundaries right
	logerror("Read from Buttons: %08x %08x\n",offset,data);
	return data;
}

READ32_MEMBER(polgar_state::read_buttons_van32)
{
	UINT32 data;
	static const char *const keynames[4] = { "BUTTON_1", "", "BUTTON_2", "BUTTON_3" };


	data = ioport(keynames[offset])->read();

	data = data << 8;
	logerror("Read from Buttons: %08x %08x\n",offset,data);
	return data;
}

READ16_MEMBER(polgar_state::read_buttons_van16)
{
	UINT16 data;
	static const char *const keynames[3] = { "BUTTON_1", "BUTTON_2", "BUTTON_3" };


	data = ioport(keynames[offset>>1])->read() << 8;

	logerror("Read from %06x offset: %x %04x\n",0xf00000,offset,data);
	return data;
}

WRITE32_MEMBER(polgar_state::write_LCD_data_32)
{
//  printf("Write LCD Data Latch %08x o: %08x\n",data,offset);
	logerror("Write LCD Data Latch %08x o: %08x\n",data,offset);
	lcd32_char = data>>24;
//  cpu_adjust_icount(cpu,-5000);

}

WRITE16_MEMBER(polgar_state::write_LCD_data)
{
	lcd32_char = data>>8;

}

void polgar_state::write_IOenable(unsigned char data,address_space &space)
{
	hd44780_device * hd44780 = space.machine().device<hd44780_device>("hd44780");

	if (BIT(data,5) && BIT(data,4)) {
		if (BIT(data,1)) {
			// delay until LCD is ready
			// (this is now patched on a per ROM basis)
			// ROMs have a very specific delay loop to avoid a busy spin while checking moves in the background
			// 48 tst.l (a4)+ instuctions need to be > ~42us (for 33.33mhz 030) (a4 is pointing at ROM)
			// MAME core does not appear to have this opcode timed right.
			// This also allows 'fake' clocks to test ELO at impossibly high speeds on real hardware
			// The original programmer says RAM is 2x as fast as the ROM on the 030 machines, maybe waitstates can be put in MAME core someday
//          cpu_spinuntil_time(space.cpu, ATTOTIME_IN_USEC(50));
			if (BIT(data,0)) {
				logerror("Write LCD_DATA [%02x] [%c]\n",lcd32_char,lcd32_char);
//              printf("Write LCD_DATA [%02x] [%c]\n",lcd32_char,lcd32_char);
			} else {
				logerror("Write LCD_CTRL [%02x] [%c]\n",lcd32_char,lcd32_char);
//              printf("Write LCD_CTRL [%02x] [%c]\n",lcd32_char,lcd32_char);
			}

			hd44780->write(space, BIT(data,0), lcd32_char);
		}

	logerror("Write to IOENBL data: %08x\n",data);

		if (BIT(data,2) || BIT(data,3))
					m_beeper->set_state(1);
				else
					m_beeper->set_state(0);
	}

}

WRITE32_MEMBER(polgar_state::write_IOenables_32){
	write_IOenable(data>>24,space);
}

WRITE16_MEMBER(polgar_state::write_IOenables)
{
	write_IOenable(data>>8,space);
}

/* Unknown read/write */

READ32_MEMBER(polgar_state::read_unknown1_32)
{
	logerror("Read from unknown1 offset: %x\n",offset);
	return 0xff00ff00;
}

READ16_MEMBER(polgar_state::read_unknown1)
{
	logerror("Read from %06x offset: %x\n",0xe80002,offset);
	return 0xff00;
}

WRITE32_MEMBER(polgar_state::write_unknown2_32)
{
	//unknown2_data = data;
	logerror("Write to   unknown2 data: %04x\n",data);
}

WRITE16_MEMBER(polgar_state::write_unknown2)
{
	//unknown2_data = data;
	logerror("Write from %06x data: %04x\n",0xe80004,data);
}

READ32_MEMBER(polgar_state::read_unknown3_32)
{
	logerror("Read from unknown3 offset: %x %08x\n",offset,(unsigned int) m_maincpu->state_int(M68K_PC));
	return 0xffffffff;
	//return unknown2_data|unknown2_data<<24;

}

READ16_MEMBER(polgar_state::read_unknown3)
{
	logerror("Read from %06x offset: %x\n",0xe80006,offset);
	return 0xffff;
	//return unknown2_data;

}

READ32_MEMBER(polgar_state::read_1800000)
{
	logerror("Read from RISC2500 1800000\n");
	return 0;
}

WRITE32_MEMBER(polgar_state::write_1000000)
{
	logerror("Write to  RISC2500 1000000\n");
}

TIMER_DEVICE_CALLBACK_MEMBER(polgar_state::timer_update_irq6)
{
	m_maincpu->set_input_line(6, HOLD_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(polgar_state::timer_update_irq2)
{
	m_maincpu->set_input_line(2, HOLD_LINE);
}


TIMER_DEVICE_CALLBACK_MEMBER(polgar_state::timer_update_irq_academy)
{
	if (academyallowNMI) {
		m_maincpu->set_input_line(6, HOLD_LINE);
	}
}


MACHINE_START_MEMBER(polgar_state,van32)
{
// patch LCD delay loop on the 68030 machines until waitstates and/or opcode timings are fixed in MAME core
// patches gen32 gen32_41 gen32_oc lond030

	UINT8 *rom = memregion("maincpu")->base();

	if(rom[0x870] == 0x0c && rom[0x871] == 0x78) {
		if (!strcmp(machine().system().name,"gen32_oc")) {
			rom[0x870] = 0x6c;
		} else {
			rom[0x870] = 0x38;
		}
	}
}


MACHINE_START_MEMBER(polgar_state,risc)
{
}


void polgar_state::common_chess_start()
{
	mboard_set_border_pieces();
	mboard_set_board();
}

MACHINE_START_MEMBER(polgar_state,polgar)
{
	common_chess_start();
	mboard_savestate_register();
}

MACHINE_START_MEMBER(polgar_state,sfortea)
{
	common_chess_start();
	mboard_savestate_register();
}

MACHINE_START_MEMBER(polgar_state,diablo68)
{
	common_chess_start();
}

MACHINE_START_MEMBER(polgar_state,van16)
{
	mboard_savestate_register();
}

MACHINE_RESET_MEMBER(polgar_state,van16)
{
	common_chess_start();
}

MACHINE_RESET_MEMBER(polgar_state,polgar)
{
	common_chess_start();
}

MACHINE_RESET_MEMBER(polgar_state,sfortea)
{
	common_chess_start();
}

MACHINE_RESET_MEMBER(polgar_state,monteciv)
{
	montecivtop = 0;
	montecivbot = 0;
	monteciv_select[0] = 0;
	monteciv_select[1] = 0;
	academyallowNMI = 0;
	common_chess_start();
}


MACHINE_RESET_MEMBER(polgar_state,academy)
{
	academyallowNMI = 0;
	common_chess_start();
}

PALETTE_INIT_MEMBER(polgar_state,chess_lcd)
{
	// palette.set_pen_color(0, rgb_t(138, 146, 148)); // some think this is closer, but slightly less readable
	palette.set_pen_color(0, rgb_t(255, 255, 255));
	palette.set_pen_color(1, rgb_t(0, 0, 0));
}

static const gfx_layout chess_charlayout =
{
	5, 8,                   /* 5 x 8 characters */
	224,                    /* 224 characters */
	1,                      /* 1 bits per pixel */
	{ 0 },                  /* no bitplanes */
	{ 3, 4, 5, 6, 7},
	{ 0, 8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8},
	8*8                     /* 8 bytes */
};

static GFXDECODE_START( chess_lcd )
	GFXDECODE_ENTRY( "hd44780:cgrom", 0x0000, chess_charlayout, 0, 1 )
GFXDECODE_END

static ADDRESS_MAP_START(polgar_mem , AS_PROGRAM, 8, polgar_state )
	AM_RANGE( 0x0000, 0x1fff ) AM_RAM
	AM_RANGE( 0x2400, 0x2400 ) AM_WRITE(mboard_write_LED_8 )        // Chessboard
	AM_RANGE( 0x2800, 0x2800 ) AM_WRITE(mboard_write_board_8)       // Chessboard
	AM_RANGE( 0x3000, 0x3000 ) AM_READ(mboard_read_board_8 )        // Chessboard
	AM_RANGE( 0x3400, 0x3405 ) AM_WRITE(polgar_write_LED)   // Function LEDs
	AM_RANGE( 0x2c00, 0x2c07 ) AM_READ(read_keys)
	AM_RANGE( 0x2004, 0x2004 ) AM_WRITE(write_polgar_IO )   // LCD Instr. Reg + Beeper
	AM_RANGE( 0x2000, 0x2000 ) AM_WRITE(write_LCD_polgar )          // LCD Char Reg.
	AM_RANGE( 0x4000, 0xffff ) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(sfortea_mem , AS_PROGRAM, 8, polgar_state )
	AM_RANGE( 0x0000, 0x1fef ) AM_RAM
	AM_RANGE( 0x1ff6, 0x1ff6 ) AM_WRITE(write_latch_sfortea)    // IO control
	AM_RANGE( 0x1ff7, 0x1ff7 ) AM_WRITE(write_lcd_IO_sfortea)   // LCD Char Reg.
//  AM_RANGE( 0x1ffc, 0x1fff ) AM_DEVREADWRITE_LEGACY("acia65c51", acia_6551_r,acia_6551_w)
	AM_RANGE( 0x1ff1, 0x1ff1 ) AM_READ(read_1ff1_sfortea )
	AM_RANGE( 0x1ff0, 0x1ff0 ) AM_READ(read_1ff0_sfortea )
	AM_RANGE( 0x2000, 0xffff ) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START(gen32_mem, AS_PROGRAM, 32, polgar_state )

	AM_RANGE( 0x00000000,  0x0003ffff )  AM_ROM

	AM_RANGE( 0xc0000000 , 0xc0000003 )  AM_READ(mboard_read_board_32 )
	AM_RANGE( 0xc8000000 , 0xc8000003 )  AM_WRITE(mboard_write_board_32 )
	AM_RANGE( 0xc8000004 , 0xc8000007 )  AM_WRITE(mboard_write_board_32 )
	AM_RANGE( 0xd0000000 , 0xd0000003 )  AM_WRITE(mboard_write_LED_32 )
	AM_RANGE( 0xd0000004 , 0xd0000007 )  AM_WRITE(mboard_write_LED_32 )
	AM_RANGE( 0xf0000004 , 0xf0000013 )  AM_READ(read_buttons_gen32 )
	AM_RANGE( 0xe0000000 , 0xe0000003 )  AM_WRITE(write_LCD_data_32 )
	AM_RANGE( 0xe0000010 , 0xe0000013 )  AM_WRITE(write_IOenables_32 )
	AM_RANGE( 0xd8000008 , 0xd800000b )  AM_WRITE(write_unknown2_32 )
	AM_RANGE( 0xd8000004 , 0xd8000007 )  AM_READ(read_unknown1_32 )
	AM_RANGE( 0xd800000c , 0xd800000f )  AM_READ(read_unknown3_32 )

	AM_RANGE( 0x40000000, 0x4007ffff )  AM_RAM      /* 512KB */
	AM_RANGE( 0x80000000, 0x8003ffff )  AM_RAM      /* 256KB */
	AM_RANGE( 0xe8000000, 0xe8007fff )  AM_RAM AM_SHARE("nvram")

	ADDRESS_MAP_END

static ADDRESS_MAP_START(bpl32_mem, AS_PROGRAM, 32, polgar_state )

	AM_RANGE( 0x000000,  0x03ffff )  AM_ROM
	AM_RANGE( 0x800000 , 0x800003 )  AM_READ(read_keys_BPL32 )
	AM_RANGE( 0x900000 , 0x900003 )  AM_WRITE(write_board_BPL32 )
	AM_RANGE( 0xa00000 , 0xa00003 )  AM_WRITE(write_LED_BPL32 )
	AM_RANGE( 0xc00000 , 0xc00003 )  AM_WRITE(write_LCD_data_32 )
	AM_RANGE( 0xb00000 , 0xb00003 )  AM_WRITE(write_IOenables_32 )
	AM_RANGE( 0x400000 , 0x4fffff )  AM_RAM      /* 1024KB */
	AM_RANGE( 0xd00000 , 0xd07fff )  AM_RAM AM_SHARE("nvram")

	ADDRESS_MAP_END

static ADDRESS_MAP_START(van32_mem, AS_PROGRAM, 32, polgar_state )

	AM_RANGE( 0x00000000,  0x0003ffff )  AM_ROM

	AM_RANGE( 0x800000fc , 0x800000ff )  AM_READ(mboard_read_board_32 )
	AM_RANGE( 0x88000000 , 0x88000007 )  AM_WRITE(mboard_write_board_32 )
	AM_RANGE( 0x90000000 , 0x90000007 )  AM_WRITE(mboard_write_LED_32 )
	AM_RANGE( 0x800000ec , 0x800000ff )  AM_READ(read_buttons_van32 )
	AM_RANGE( 0xa0000000 , 0xa0000003 )  AM_WRITE(write_LCD_data_32 )
	AM_RANGE( 0xa0000010 , 0xa0000013 )  AM_WRITE(write_IOenables_32 )
	AM_RANGE( 0x98000008 , 0x9800000b )  AM_WRITE(write_unknown2_32 )
	AM_RANGE( 0x98000004 , 0x98000007 )  AM_READ(read_unknown1_32 )
	AM_RANGE( 0x9800000c , 0x9800000f )  AM_READ(read_unknown3_32 )

	AM_RANGE( 0x40000000, 0x400fffff )  AM_RAM      /* 1024KB */
	AM_RANGE( 0xa8000000, 0xa8007fff )  AM_RAM AM_SHARE("nvram")

	ADDRESS_MAP_END


static ADDRESS_MAP_START(alm32_mem, AS_PROGRAM, 32, polgar_state )

	AM_RANGE( 0x00000000,  0x0001ffff )  AM_ROM

	AM_RANGE( 0x800000fc , 0x800000ff )  AM_READ(mboard_read_board_32 )
	AM_RANGE( 0x88000000 , 0x88000007 )  AM_WRITE(mboard_write_board_32 )
	AM_RANGE( 0x90000000 , 0x90000007 )  AM_WRITE(mboard_write_LED_32 )
	AM_RANGE( 0x800000ec , 0x800000ff )  AM_READ(read_buttons_van32 )
	AM_RANGE( 0xa0000000 , 0xa0000003 )  AM_WRITE(write_LCD_data_32 )
	AM_RANGE( 0xa0000010 , 0xa0000013 )  AM_WRITE(write_IOenables_32 )
	AM_RANGE( 0x98000008 , 0x9800000b )  AM_WRITE(write_unknown2_32 )
	AM_RANGE( 0x98000004 , 0x98000007 )  AM_READ(read_unknown1_32 )
	AM_RANGE( 0x9800000c , 0x9800000f )  AM_READ(read_unknown3_32 )

	AM_RANGE( 0x40000000, 0x400fffff )  AM_RAM
	AM_RANGE( 0xa8000000, 0xa8007fff )  AM_RAM AM_SHARE("nvram")

	ADDRESS_MAP_END

static ADDRESS_MAP_START(risc_mem, AS_PROGRAM, 32, polgar_state )

	AM_RANGE( 0x02000000,  0x0201ffff )  AM_ROM AM_REGION("maincpu", 0) // AM_MIRROR(0x2000000)
	AM_RANGE( 0x01000000,  0x01000003 )  AM_WRITE(write_1000000 )
	AM_RANGE( 0x01800000,  0x01800003 )  AM_READ(read_1800000 )
	AM_RANGE( 0x00000000,  0x0001ffff )  AM_RAM

	ADDRESS_MAP_END

static ADDRESS_MAP_START(van16_mem, AS_PROGRAM, 16, polgar_state )

	AM_RANGE( 0x000000,  0x03ffff )  AM_ROM

	AM_RANGE( 0xc00000 , 0xc00001 )  AM_READ(mboard_read_board_16 )
	AM_RANGE( 0xc80000 , 0xc80001 )  AM_WRITE(mboard_write_board_16 )
	AM_RANGE( 0xd00000 , 0xd00001 )  AM_WRITE(mboard_write_LED_16 )
	AM_RANGE( 0xf00000 , 0xf00009 )  AM_READ(read_buttons_van16 )
	AM_RANGE( 0xd80000 , 0xd80001 )  AM_WRITE(write_LCD_data )
	AM_RANGE( 0xd80008 , 0xd80009 )  AM_WRITE(write_IOenables )
	AM_RANGE( 0xe80004 , 0xe80005 )  AM_WRITE(write_unknown2 )
	AM_RANGE( 0xe80002 , 0xe80003 )  AM_READ(read_unknown1 )
	AM_RANGE( 0xe80006 , 0xe80007 )  AM_READ(read_unknown3 )

	AM_RANGE( 0x400000, 0x47ffff )  AM_RAM      /* 512KB */
	AM_RANGE( 0x800000, 0x803fff )  AM_RAM AM_SHARE("nvram")

	ADDRESS_MAP_END

static ADDRESS_MAP_START(alm16_mem, AS_PROGRAM, 16, polgar_state )

	AM_RANGE( 0x000000,  0x01ffff )  AM_ROM

	AM_RANGE( 0xc00000 , 0xc00001 )  AM_READ(mboard_read_board_16 )
	AM_RANGE( 0xc80000 , 0xc80001 )  AM_WRITE(mboard_write_board_16 )
	AM_RANGE( 0xd00000 , 0xd00001 )  AM_WRITE(mboard_write_LED_16 )
	AM_RANGE( 0xf00000 , 0xf00009 )  AM_READ(read_buttons_van16 )
	AM_RANGE( 0xd80000 , 0xd80001 )  AM_WRITE(write_LCD_data )
	AM_RANGE( 0xd80008 , 0xd80009 )  AM_WRITE(write_IOenables )
//  AM_RANGE( 0xe80004 , 0xe80005 )  AM_WRITE(write_unknown2 )
//  AM_RANGE( 0xe80002 , 0xe80003 )  AM_READ(read_unknown1 )
//  AM_RANGE( 0xe80006 , 0xe80007 )  AM_READ(read_unknown3 )


	AM_RANGE( 0x400000, 0x47ffff )  AM_RAM      /* 512KB */
	AM_RANGE( 0x800000, 0x803fff )  AM_RAM  AM_SHARE("nvram")
ADDRESS_MAP_END


static ADDRESS_MAP_START(milano_mem , AS_PROGRAM, 8, polgar_state )
	AM_RANGE( 0x0000, 0x1f9f ) AM_RAM
	AM_RANGE( 0x1fd0, 0x1fd0 ) AM_WRITE(milano_write_board )        // Chessboard
	AM_RANGE( 0x1fe0, 0x1fe0 ) AM_READ(milano_read_board )      // Chessboard
	AM_RANGE( 0x1fe8, 0x1fed ) AM_WRITE(milano_write_LED )  // Function LEDs
	AM_RANGE( 0x1fd8, 0x1fdf ) AM_READ(read_keys)
	AM_RANGE( 0x1ff0, 0x1ff0 ) AM_WRITE(write_polgar_IO)    // IO control
	AM_RANGE( 0x1fc0, 0x1fc0 ) AM_WRITE(write_LCD_polgar)   // LCD Char Reg. (latched)
	AM_RANGE( 0x2000, 0xffff ) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START(academy_mem , AS_PROGRAM, 8, polgar_state )
	AM_RANGE( 0x0000, 0x1fff ) AM_RAM
	AM_RANGE( 0x2400, 0x2400 ) AM_READ(read_keys_board_academy )
	AM_RANGE( 0x2800, 0x2800 ) AM_WRITE(academy_write_board )       // Chessboard
	AM_RANGE( 0x2c00, 0x2c00 ) AM_WRITE(mboard_write_LED_8 )        // Chessboard
	AM_RANGE( 0x3002, 0x3002 ) AM_WRITE(beep_academy )
	AM_RANGE( 0x3001, 0x3001 ) AM_WRITE(academy_inhibitNMI )
	AM_RANGE( 0x3400, 0x3400 ) AM_WRITE(academy_write_LED )
	AM_RANGE( 0x3800, 0x3801 ) AM_WRITE(write_LCD_academy )
	AM_RANGE( 0x4000, 0xffff ) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(monteciv_mem , AS_PROGRAM, 8, polgar_state )
	AM_RANGE( 0x0000, 0x1fff ) AM_RAM
	AM_RANGE( 0x2400, 0x2400 ) AM_READ(read_keys_board_monteciv )
	AM_RANGE( 0x2800, 0x2800 ) AM_WRITE(academy_write_board )       // Chessboard
	AM_RANGE( 0x2c00, 0x2c00 ) AM_WRITE(mboard_write_LED_8 )        // Chessboard
	AM_RANGE( 0x3400, 0x3400 ) AM_WRITE(academy_write_LED )         // Status LEDs
	AM_RANGE( 0x3000, 0x3001 ) AM_WRITE(monteciv_select_line )          // Select Keyline
	AM_RANGE( 0x3002, 0x3002 ) AM_WRITE(beep_academy )
	AM_RANGE( 0x3004, 0x3004 ) AM_WRITE(monteciv_write_LCD )
	AM_RANGE( 0x3005, 0x3005 ) AM_WRITE(monteciv_3005 )
	AM_RANGE( 0x3007, 0x3007 ) AM_WRITE(monteciv_3007 )
	AM_RANGE( 0x3006, 0x3006 ) AM_WRITE(monteciv_3006 )
	AM_RANGE( 0x2000, 0x2000 ) AM_WRITE(academy_inhibitNMI )
	AM_RANGE( 0x8000, 0xffff ) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(megaiv_mem , AS_PROGRAM, 8, polgar_state )
	AM_RANGE( 0x0000, 0x1fff ) AM_RAM
/// AM_RANGE( 0x2400, 0x2400 ) AM_READ(read_keys_board_monteciv )
	AM_RANGE( 0x6800, 0x6800 ) AM_WRITE(academy_write_board )   // 2800 // Chessboard
/// AM_RANGE( 0x2c00, 0x2c00 ) AM_WRITE(mboard_write_LED_8 )      // Chessboard
/// AM_RANGE( 0x3400, 0x3400 ) AM_WRITE(academy_write_LED )           // Status LEDs
	AM_RANGE( 0x4400, 0x4400 ) AM_WRITE(megaiv_write_LED )  // 2400     // Select Keyline
	AM_RANGE( 0x7000, 0x7001 ) AM_WRITE(megaiv_IO )         // Select Keyline
/// AM_RANGE( 0x3002, 0x3002 ) AM_WRITE(beep_academy )
	AM_RANGE( 0x4000, 0x4007 ) AM_READ(read_keys_megaiv ) // 3000-7 fixio
	AM_RANGE( 0x2c04, 0x2c04 ) AM_WRITE(monteciv_write_LCD ) // 2c04
	AM_RANGE( 0x2c05, 0x2c05 ) AM_WRITE(monteciv_3005 ) // 2c05
	AM_RANGE( 0x2c07, 0x2c07 ) AM_WRITE(monteciv_3007 ) // 2c07
	AM_RANGE( 0x2c06, 0x2c06 ) AM_WRITE(monteciv_3006 ) // 2c06
/// AM_RANGE( 0x2000, 0x2000 ) AM_WRITE(academy_inhibitNMI )
	AM_RANGE( 0x8000, 0xffff ) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(diablo68_mem , AS_PROGRAM, 16, polgar_state )
	AM_RANGE( 0x00000000, 0x0000ffff ) AM_ROM // OS
//  AM_RANGE( 0x00200000, 0x0020ffff ) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE( 0x00ff0000, 0x00ff7fff ) AM_ROM AM_REGION("maincpu",10000) // Opening Book
//  AM_RANGE( 0x00300000, 0x00300007 ) AM_READ(diablo68_aciaread)
//  AM_RANGE( 0x00300000, 0x00300007 ) AM_READ(diablo68_aciawrite)
//  AM_RANGE( 0x00300002, 0x00300003 ) AM_READ(diablo68_flags)
	AM_RANGE( 0x003a0000, 0x003a0001 ) AM_WRITE(diablo68_write_LCD)
	AM_RANGE( 0x003c0000, 0x003c0001 ) AM_WRITE(diablo68_reg_select)
	AM_RANGE( 0x00280000, 0x0028ffff ) AM_RAM  // hash tables
	AM_RANGE( 0x00ff8000, 0x00ffffff ) AM_RAM
ADDRESS_MAP_END


/* Input ports */

static INPUT_PORTS_START( polgar )

	PORT_START("KEY1_0") //Port $2c00
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(" Trn") PORT_CODE(KEYCODE_F1)
	PORT_START("KEY1_1") //Port $2c01
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(" Info") PORT_CODE(KEYCODE_F2)
	PORT_START("KEY1_2") //Port $2c02
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(" Mem") PORT_CODE(KEYCODE_F3)
	PORT_START("KEY1_3") //Port $2c03
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(" Pos") PORT_CODE(KEYCODE_F4)
	PORT_START("KEY1_4") //Port $2c04
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(" LEV") PORT_CODE(KEYCODE_F5)
	PORT_START("KEY1_5") //Port $2c05
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(" FCT") PORT_CODE(KEYCODE_F6)
	PORT_START("KEY1_6") //Port $2c06
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(" ENT") PORT_CODE(KEYCODE_F7)
	PORT_START("KEY1_7") //Port $2c07
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(" CL") PORT_CODE(KEYCODE_F8)
//  PORT_START("KEY1_8")
//  PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(" A") PORT_CODE(KEYCODE_1)

	PORT_INCLUDE( chessboard )

INPUT_PORTS_END

static INPUT_PORTS_START( sfortea )
	PORT_START("BUTTONS_SFOR1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("ENT") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("CL") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("UP") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("DOWN") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("LEFT") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("RIGHT") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("RST1") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("RST2") PORT_CODE(KEYCODE_8)

	PORT_START("BUTTONS_SFOR2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("ENT") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("CL") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("UP") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("DOWN") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("LEFT") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("RIGHT") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("RST1") PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("RST2") PORT_CODE(KEYCODE_F8)

/*  PORT_START("BUTTONS_SFOR3")
    PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("ENT") PORT_CODE(KEYCODE_1)
    PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("CL") PORT_CODE(KEYCODE_2)
    PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("UP") PORT_CODE(KEYCODE_3)
    PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("DOWN") PORT_CODE(KEYCODE_4)
    PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("LEFT") PORT_CODE(KEYCODE_5)
    PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("RIGHT") PORT_CODE(KEYCODE_6)
    PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("RST1") PORT_CODE(KEYCODE_7)
    PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("RST2") PORT_CODE(KEYCODE_8)
*/

	PORT_INCLUDE( chessboard )
INPUT_PORTS_END

static INPUT_PORTS_START( academy )
	PORT_START("BUTTONS_ACAD")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(" Trn") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(" Info") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(" Mem") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(" Pos") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(" LEV") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(" FCT") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(" ENT") PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(" CL") PORT_CODE(KEYCODE_F8)

	PORT_INCLUDE( chessboard )
INPUT_PORTS_END

static INPUT_PORTS_START( megaiv )

	PORT_START("KEY1_0") //Port $2c00
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(" Trn") PORT_CODE(KEYCODE_F1)
	PORT_START("KEY1_1") //Port $2c01
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(" Info") PORT_CODE(KEYCODE_F2) // mem
	PORT_START("KEY1_2") //Port $2c02
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(" Mem") PORT_CODE(KEYCODE_F3)
	PORT_START("KEY1_3") //Port $2c03
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(" Pos") PORT_CODE(KEYCODE_F4)
	PORT_START("KEY1_4") //Port $2c04
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(" LEV") PORT_CODE(KEYCODE_F5)
	PORT_START("KEY1_5") //Port $2c05
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(" FCT") PORT_CODE(KEYCODE_F6)
	PORT_START("KEY1_6") //Port $2c06
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(" ENT") PORT_CODE(KEYCODE_F7)
	PORT_START("KEY1_7") //Port $2c07
	PORT_BIT(0x080, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(" CL") PORT_CODE(KEYCODE_F8)
//  PORT_START("KEY1_8")
//  PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(" A") PORT_CODE(KEYCODE_1)

	PORT_INCLUDE( chessboard )

INPUT_PORTS_END


static INPUT_PORTS_START( monteciv )
	PORT_START("BUTTONS_MONTE1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(" 1 Pawn") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(" 2 Knight") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(" 3 Bishop") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(" 4 Rook") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(" 5 Queen") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(" 6 King") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(" 7 Black") PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(" 8 White") PORT_CODE(KEYCODE_F8)

	PORT_START("BUTTONS_MONTE2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9 Book") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0 Pos") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Mem") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Info") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Clear") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Level") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Reset") PORT_CODE(KEYCODE_8)


	PORT_INCLUDE( chessboard )
INPUT_PORTS_END


static INPUT_PORTS_START( gen32 )

	PORT_START("BUTTON_1")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("ENT") PORT_CODE(KEYCODE_ENTER)

	PORT_START("BUTTON_2")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("UP") PORT_CODE(KEYCODE_UP)

	PORT_START("BUTTON_3")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("CL") PORT_CODE(KEYCODE_BACKSLASH)

	PORT_INCLUDE( chessboard )

INPUT_PORTS_END

static INPUT_PORTS_START( bpl32 )
	PORT_START("BUTTONS_BPL")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("RST1") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("RST2") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("ENT") PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("UP") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("CL") PORT_CODE(KEYCODE_BACKSLASH)

	PORT_INCLUDE( chessboard )
INPUT_PORTS_END



static INPUT_PORTS_START( van32 )

	PORT_START("BUTTON_3")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_NAME("ENT") PORT_CODE(KEYCODE_ENTER)

	PORT_START("BUTTON_2")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_NAME("UP") PORT_CODE(KEYCODE_UP)

	PORT_START("BUTTON_1")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)  PORT_NAME("CL") PORT_CODE(KEYCODE_BACKSLASH)

	PORT_INCLUDE( chessboard )

	INPUT_PORTS_END

static INPUT_PORTS_START( van16 )

	PORT_START("BUTTON_1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("LEFT") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("ENT") PORT_CODE(KEYCODE_ENTER)

	PORT_START("BUTTON_2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("RIGHT") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("UP") PORT_CODE(KEYCODE_UP)

	PORT_START("BUTTON_3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("DOWN") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD)  PORT_NAME("CL") PORT_CODE(KEYCODE_BACKSLASH)

	PORT_INCLUDE( chessboard )

INPUT_PORTS_END

static MACHINE_CONFIG_FRAGMENT ( chess_common )

	/* video hardware */

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_SIZE(100, 22)
	MCFG_SCREEN_VISIBLE_AREA(0, 100-1, 0, 22-3)
	MCFG_SCREEN_UPDATE_DEVICE("hd44780", hd44780_device, screen_update)
	MCFG_SCREEN_PALETTE("palette")
	MCFG_PALETTE_ADD("palette", 2)
	MCFG_PALETTE_INIT_OWNER(polgar_state,chess_lcd)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", chess_lcd)

	MCFG_HD44780_ADD("hd44780")
	MCFG_HD44780_LCD_SIZE(2, 16)

	MCFG_DEFAULT_LAYOUT(layout_lcd)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 3250)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END

static MACHINE_CONFIG_START( polgar, polgar_state )
	MCFG_CPU_ADD("maincpu",M65C02,4915200)
	MCFG_CPU_PROGRAM_MAP(polgar_mem)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))
	MCFG_MACHINE_START_OVERRIDE(polgar_state, polgar )
	MCFG_MACHINE_RESET_OVERRIDE(polgar_state, polgar )
	MCFG_FRAGMENT_ADD( chess_common )

	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq_timer", polgar_state, cause_nmi, attotime::from_hz(600))
	MCFG_TIMER_START_DELAY(attotime::from_hz(60))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("artwork_timer", polgar_state, mboard_update_artwork, attotime::from_hz(100))

MACHINE_CONFIG_END

static MACHINE_CONFIG_START( sfortea, polgar_state )
	MCFG_CPU_ADD("maincpu",M65C02,5000000)
	MCFG_CPU_PROGRAM_MAP(sfortea_mem)
	MCFG_QUANTUM_TIME(attotime::from_hz(60))
	MCFG_MACHINE_START_OVERRIDE(polgar_state, sfortea )
	MCFG_MACHINE_RESET_OVERRIDE(polgar_state, sfortea )
	MCFG_FRAGMENT_ADD( chess_common )

	/* acia */
//  MCFG_MOS6551_ADD("acia65c51", XTAL_1_8432MHz, NULL)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq_timer", polgar_state, cause_M6502_irq, attotime::from_hz(600))
	MCFG_TIMER_START_DELAY(attotime::from_hz(60))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("artwork_timer", polgar_state, mboard_update_artwork, attotime::from_hz(100))
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( alm32, polgar_state )
	MCFG_CPU_ADD("maincpu", M68020, XTAL_12MHz)
	MCFG_CPU_PROGRAM_MAP(alm32_mem)
	MCFG_MACHINE_START_OVERRIDE(polgar_state,van32)
	MCFG_MACHINE_RESET_OVERRIDE(polgar_state,van16)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("int_timer", polgar_state, timer_update_irq6, attotime::from_hz(750))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("artwork_timer", polgar_state, mboard_update_artwork, attotime::from_hz(120))

	MCFG_FRAGMENT_ADD( chess_common )
	MCFG_NVRAM_ADD_0FILL("nvram")

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( academy, polgar )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(academy_mem)
	MCFG_MACHINE_RESET_OVERRIDE(polgar_state, academy )
	//MCFG_DEVICE_REMOVE("int_timer")
	MCFG_TIMER_DRIVER_ADD_PERIODIC("int_timer", polgar_state, timer_update_irq_academy, attotime::from_hz(600))

MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( milano, polgar )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(milano_mem)
MACHINE_CONFIG_END


static MACHINE_CONFIG_START( monteciv, polgar_state )
	MCFG_CPU_ADD("maincpu",M65C02,8000000)
	MCFG_CPU_PROGRAM_MAP( monteciv_mem )
	MCFG_MACHINE_START_OVERRIDE(polgar_state, polgar )
	MCFG_MACHINE_RESET_OVERRIDE(polgar_state, monteciv )
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("beeper", BEEP, 3250)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq_timer", polgar_state, cause_nmi, attotime::from_hz(600))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("artwork_timer", polgar_state, mboard_update_artwork, attotime::from_hz(100))

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( megaiv, monteciv )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK( 4915200 )
	MCFG_CPU_PROGRAM_MAP(megaiv_mem)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( diablo68, polgar_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_16MHz)
	MCFG_CPU_PROGRAM_MAP(diablo68_mem)
	MCFG_MACHINE_START_OVERRIDE(polgar_state,diablo68)
	MCFG_FRAGMENT_ADD( chess_common )

	/* acia */
//  MCFG_MOS6551_ADD("acia65c51", XTAL_1_8432MHz, NULL)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("int_timer", polgar_state, timer_update_irq2, attotime::from_hz(60))
	MCFG_TIMER_START_DELAY(attotime::from_hz(30))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("artwork_timer", polgar_state, mboard_update_artwork, attotime::from_hz(120))


MACHINE_CONFIG_END

static MACHINE_CONFIG_START( van16, polgar_state )
	MCFG_CPU_ADD("maincpu", M68000, XTAL_12MHz)
	MCFG_CPU_PROGRAM_MAP(van16_mem)
	MCFG_MACHINE_START_OVERRIDE(polgar_state,van16)
	MCFG_MACHINE_RESET_OVERRIDE(polgar_state,van16)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("int_timer", polgar_state, timer_update_irq6, attotime::from_hz(600))
	//MCFG_TIMER_DRIVER_ADD_PERIODIC("int_timer", polgar_state, timer_update_irq6, attotime::from_hz(587))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("artwork_timer", polgar_state, mboard_update_artwork, attotime::from_hz(120))
	MCFG_FRAGMENT_ADD( chess_common )
	MCFG_NVRAM_ADD_0FILL("nvram")

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( alm16, van16 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(alm16_mem)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( van32, polgar_state )
	MCFG_CPU_ADD("maincpu", M68020, XTAL_12MHz)
	MCFG_CPU_PROGRAM_MAP(van32_mem)
	MCFG_MACHINE_START_OVERRIDE(polgar_state,van32)
	MCFG_MACHINE_RESET_OVERRIDE(polgar_state,van16)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("int_timer", polgar_state, timer_update_irq6, attotime::from_hz(750))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("artwork_timer", polgar_state, mboard_update_artwork, attotime::from_hz(120))

	MCFG_FRAGMENT_ADD( chess_common )
	MCFG_NVRAM_ADD_0FILL("nvram")

MACHINE_CONFIG_END

static MACHINE_CONFIG_START( risc, polgar_state )
	MCFG_CPU_ADD("maincpu", ARM, 14000000)
	MCFG_CPU_PROGRAM_MAP(risc_mem)
	MCFG_MACHINE_START_OVERRIDE(polgar_state,risc)
	MCFG_MACHINE_RESET_OVERRIDE(polgar_state,van16)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("artwork_timer", polgar_state, mboard_update_artwork, attotime::from_hz(120))

	MCFG_FRAGMENT_ADD( chess_common )
//  MCFG_NVRAM_ADD_0FILL("nvram")

MACHINE_CONFIG_END

static MACHINE_CONFIG_START( gen32, polgar_state )
	MCFG_CPU_ADD("maincpu", M68030, XTAL_33_333MHz)
	MCFG_CPU_PROGRAM_MAP(gen32_mem)
	MCFG_MACHINE_START_OVERRIDE(polgar_state,van32)
	MCFG_MACHINE_RESET_OVERRIDE(polgar_state,van16)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("int_timer", polgar_state, timer_update_irq6, attotime::from_hz(375))
	//MCFG_TIMER_DRIVER_ADD_PERIODIC("int_timer", polgar_state, timer_update_irq6, attotime::from_hz(368.64))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("artwork_timer", polgar_state, mboard_update_artwork, attotime::from_hz(120))

	MCFG_FRAGMENT_ADD( chess_common )
	MCFG_NVRAM_ADD_0FILL("nvram")


MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( gen32_oc, gen32 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK( XTAL_33_333MHz * 2 )
	MCFG_DEVICE_REMOVE("int_timer")
	MCFG_TIMER_DRIVER_ADD_PERIODIC("int_timer", polgar_state, timer_update_irq6, attotime::from_hz(500))


MACHINE_CONFIG_END

static MACHINE_CONFIG_START( bpl32, polgar_state )
	MCFG_CPU_ADD("maincpu", M68020, XTAL_24_576MHz)
	MCFG_CPU_PROGRAM_MAP(bpl32_mem)
	MCFG_MACHINE_START_OVERRIDE(polgar_state,van32)
	MCFG_MACHINE_RESET_OVERRIDE(polgar_state,van16)
	MCFG_TIMER_DRIVER_ADD_PERIODIC("int_timer", polgar_state, timer_update_irq6, attotime::from_hz(750))
	MCFG_TIMER_DRIVER_ADD_PERIODIC("artwork_timer", polgar_state, mboard_update_artwork, attotime::from_hz(100))

	MCFG_FRAGMENT_ADD( chess_common )
	MCFG_NVRAM_ADD_0FILL("nvram")

MACHINE_CONFIG_END


/* ROM definitions */



ROM_START(polgar)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("polgar.bin", 0x0000, 0x10000, CRC(88d55c0f) SHA1(e86d088ec3ac68deaf90f6b3b97e3e31b1515913))
ROM_END

ROM_START(sfortea)
	ROM_REGION(0x18000,"maincpu",0)
	ROM_LOAD("sfalo.bin", 0x0000, 0x8000, CRC(86e0230a) SHA1(0d6e18a17e636b8c7292c8f331349d361892d1a8))
	ROM_LOAD("sfahi.bin", 0x8000, 0x8000, CRC(81c02746) SHA1(0bf68b68ade5a3263bead88da0a8965fc71483c1))
	ROM_LOAD("sfabook.bin", 0x10000, 0x8000, CRC(3e42cf7c) SHA1(b2faa36a127e08e5755167a25ed4a07f12d62957))
ROM_END

ROM_START( alm16 )
	ROM_REGION16_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("alm16eve.bin", 0x00000, 0x10000,CRC(EE5B6EC4) SHA1(30920C1B9E16FFAE576DA5AFA0B56DA59ADA3DBB))
	ROM_LOAD16_BYTE("alm16odd.bin" , 0x00001, 0x10000,CRC(D0BE4EE4) SHA1(D36C074802D2C9099CD44E75F9DE3FC7D1FD9908))
ROM_END

ROM_START( alm32 )
	ROM_REGION32_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD("alm32.bin", 0x00000, 0x20000,CRC(38F4B305) SHA1(43459A057FF29248C74D656A036AC325202B9C15))
ROM_END

ROM_START(sforteb)
	ROM_REGION(0x18000,"maincpu",0)
	ROM_LOAD("forte_b.lo", 0x0000, 0x8000, CRC(48bfe5d6) SHA1(323642686b6d2fb8db2b7d50c6cd431058078ce1))
	ROM_LOAD("forte_b.hi1", 0x8000, 0x8000, CRC(9778ca2c) SHA1(d8b88b9768a1a9171c68cbb0892b817d68d78351))
	ROM_LOAD("forte_b.hi0", 0x10000, 0x8000, CRC(bb07ad52) SHA1(30cf9005021ab2d7b03facdf2d3588bc94dc68a6))
ROM_END

ROM_START(sforteba)
	ROM_REGION(0x18000,"maincpu",0)
	ROM_LOAD("forte b_l.bin", 0x0000, 0x8000, CRC(e3d194a1) SHA1(80457580d7c57e07895fd14bfdaf14b30952afca))
	ROM_LOAD("forte b_h.bin", 0x8000, 0x8000, CRC(dd824be8) SHA1(cd8666b6b525887f9fc48a730b71ceabcf07f3b9))
	ROM_LOAD("forte_b.hi0", 0x10000, 0x8000, BAD_DUMP CRC(bb07ad52) SHA1(30cf9005021ab2d7b03facdf2d3588bc94dc68a6))
ROM_END

ROM_START(sexpertb)
	ROM_REGION(0x18000,"maincpu",0)
	ROM_LOAD("seb69u3.bin", 0x0000, 0x8000, CRC(92002eb6) SHA1(ed8ca16701e00b48fa55c856fa4a8c6613079c02))
	ROM_LOAD("seb69u1.bin", 0x8000, 0x8000, CRC(814b4420) SHA1(c553e6a8c048dcc1cf48d410111a86e06b99d356))
	ROM_LOAD("seb605u2.bin", 0x10000, 0x8000, CRC(bb07ad52) SHA1(30cf9005021ab2d7b03facdf2d3588bc94dc68a6))
ROM_END

ROM_START(academy)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("acad8000.bin", 0x8000, 0x8000, CRC(a967922b) SHA1(1327903ff89bf96d72c930c400f367ae19e3ec68))
	ROM_LOAD("acad4000.bin", 0x4000, 0x4000, CRC(ee1222b5) SHA1(98541d87755a7186b69b9723cc4adbd07f20f0e2))
ROM_END

ROM_START(megaiv)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("megaiv.bin", 0x8000, 0x8000, CRC(dee355d2) SHA1(6bc79c0fb169020f017412f5f9696b9ecafbf99f))

ROM_END

ROM_START(milano)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("milano.bin", 0x0000, 0x10000, CRC(0e9c8fe1) SHA1(e9176f42d86fe57e382185c703c7eff7e63ca711))
ROM_END


ROM_START(sfortec)
	ROM_REGION(0x18000,"maincpu",0)
	ROM_LOAD("sfclow.bin", 0x0000, 0x8000, CRC(f040cf30) SHA1(1fc1220b8ed67cdffa3866d230ce001721cf684f))
	ROM_LOAD("sfchi.bin", 0x8000, 0x8000, CRC(0f926b32) SHA1(9c7270ecb3f41dd9172a9a7928e6e04e64b2a340))
	ROM_LOAD("sfcbook.bin", 0x10000, 0x8000, CRC(c6a1419a) SHA1(017a0ffa9aa59438c879624a7ddea2071d1524b8))
ROM_END


ROM_START(sexpertc)
	ROM_REGION(0x18000,"maincpu",0) // Version 3.6 of firmware
	ROM_LOAD("seclow.bin", 0x0000, 0x8000, CRC(5a29105e) SHA1(be37bb29b530dbba847a5e8d27d81b36525e47f7))
	ROM_LOAD("sechi.bin", 0x8000, 0x8000, CRC(0085c2c4) SHA1(d84bf4afb022575db09dd9dc12e9b330acce35fa))
	ROM_LOAD("secbook.bin", 0x10000, 0x8000, CRC(2d085064) SHA1(76162322aa7d23a5c07e8356d0bbbb33816419af))
ROM_END

ROM_START( lyon16 )
	ROM_REGION16_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("lyon16ev.bin", 0x00000, 0x10000,CRC(497BD41A) SHA1(3FFEFEEAC694F49997C10D248EC6A7AA932898A4))
	ROM_LOAD16_BYTE("lyon16od.bin" , 0x00001, 0x10000,CRC(F9DE3F54) SHA1(4060E29566D2F40122CCDE3C1F84C94A9C1ED54F))
ROM_END

ROM_START( lyon32 )
	ROM_REGION32_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD("lyon32.bin", 0x00000, 0x20000, CRC(5c128b06) SHA1(954c8f0d3fae29900cb1e9c14a41a9a07a8e185f))
ROM_END


ROM_START(monteciv)
	ROM_REGION(0x10000,"maincpu",0)
	ROM_LOAD("mciv.bin", 0x8000, 0x8000, CRC(c4887694) SHA1(7f482d2a40fcb3125266e7a5407da315b4f9b49c))

ROM_END

ROM_START( diablo68 )
	ROM_REGION16_BE( 0x20000, "maincpu", 0 )
	ROM_LOAD16_BYTE("evenurom.bin", 0x00000, 0x8000,CRC(03477746) SHA1(8bffcb159a61e59bfc45411e319aea6501ebe2f9))
	ROM_LOAD16_BYTE("oddlrom.bin",  0x00001, 0x8000,CRC(e182dbdd) SHA1(24dacbef2173fa737636e4729ff22ec1e6623ca5))
	ROM_LOAD16_BYTE("book.bin", 0x10000, 0x8000,CRC(553a5c8c) SHA1(ccb5460ff10766a5ca8008ae2cffcff794318108))
ROM_END

ROM_START( van16 )
	ROM_REGION16_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD16_BYTE("va16even.bin", 0x00000, 0x20000,CRC(E87602D5) SHA1(90CB2767B4AE9E1B265951EB2569B9956B9F7F44))
	ROM_LOAD16_BYTE("va16odd.bin" , 0x00001, 0x20000,CRC(585F3BDD) SHA1(90BB94A12D3153A91E3760020E1EA2A9EAA7EC0A))
ROM_END


ROM_START( van32 )
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD("vanc32.bin", 0x00000, 0x40000,CRC(F872BEB5) SHA1(9919F207264F74E2B634B723B048AE9CA2CEFBC7))
ROM_END


ROM_START( risc )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD("s2500.bin", 0x000000, 0x20000, CRC(7a707e82) SHA1(87187fa58117a442f3abd30092cfcc2a4d7c7efc))
ROM_END

ROM_START( gen32 )
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD("gen32_4.bin", 0x00000, 0x40000,CRC(6CC4DA88) SHA1(EA72ACF9C67ED17C6AC8DE56A165784AA629C4A1))
ROM_END

ROM_START( gen32_41 )
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD("gen32_41.bin", 0x00000, 0x40000,CRC(ea9938c0) SHA1(645cf0b5b831b48104ad6cec8d78c63dbb6a588c))
ROM_END

ROM_START( gen32_oc )
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD("gen32_41.bin", 0x00000, 0x40000,CRC(ea9938c0) SHA1(645cf0b5b831b48104ad6cec8d78c63dbb6a588c))
ROM_END

ROM_START( berlinp )
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD("berlinp.bin", 0x00000, 0x40000,CRC(82FBAF6E) SHA1(729B7CEF3DFAECC4594A6178FC4BA6015AFA6202))
ROM_END

ROM_START( bpl32 )
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD("bpl32.bin", 0x00000, 0x40000,CRC(D75E170F) SHA1(AC0EBDAA114ABD4FEF87361A03DF56928768B1AE))
ROM_END

ROM_START( lond020 )
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD("lond020.bin", 0x00000, 0x40000,CRC(3225B8DA) SHA1(FD8F6F4E9C03B6CDC86D8405E856C26041BFAD12))
ROM_END

ROM_START( lond030 )
	ROM_REGION32_BE( 0x40000, "maincpu", 0 )
	ROM_LOAD("lond030.bin", 0x00000, 0x40000,CRC(853BAA4E) SHA1(946951081D4E91E5BDD9E93D0769568A7FE79BAD))
ROM_END

DRIVER_INIT_MEMBER(polgar_state,polgar)
{
	led_status=0;
}

/*       YEAR  NAME      PARENT   COMPAT  MACHINE    INPUT     INIT     COMPANY                      FULLNAME                     FLAGS */
	CONS(  1986, polgar,   0,       0,      polgar,    polgar, polgar_state,   polgar,  "Hegener & Glaser",          "Mephisto Polgar Schachcomputer", MACHINE_REQUIRES_ARTWORK | MACHINE_CLICKABLE_ARTWORK)
	CONS(  1987, sfortea,  0,       0,      sfortea,   sfortea, driver_device,  0,       "Novag",                     "Novag Super Forte Chess Computer (version A)", MACHINE_NO_SOUND|MACHINE_SUPPORTS_SAVE|MACHINE_NOT_WORKING | MACHINE_CLICKABLE_ARTWORK )
	CONS(  1988, alm16,    van16,   0,      alm16,     van16, driver_device,    0,       "Hegener & Glaser Muenchen", "Mephisto Almeria 68000", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK | MACHINE_CLICKABLE_ARTWORK )
	CONS(  1988, alm32,    van16,   0,      alm32,     van32, driver_device,    0,       "Hegener & Glaser Muenchen", "Mephisto Alimera 68020", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK | MACHINE_CLICKABLE_ARTWORK )
	CONS(  1988, sforteb,  sfortea, 0,      sfortea,   sfortea, driver_device,  0,       "Novag",                     "Novag Super Forte Chess Computer (version B)", MACHINE_NO_SOUND|MACHINE_SUPPORTS_SAVE|MACHINE_NOT_WORKING | MACHINE_CLICKABLE_ARTWORK )
	CONS(  1988, sforteba, sfortea, 0,      sfortea,   sfortea, driver_device,  0,       "Novag",                     "Novag Super Forte Chess Computer (version B, alt)", MACHINE_NO_SOUND|MACHINE_SUPPORTS_SAVE|MACHINE_NOT_WORKING | MACHINE_CLICKABLE_ARTWORK )
	CONS(  1988, sexpertb, sfortea, 0,      sfortea,   sfortea, driver_device,  0,       "Novag",                     "Novag Super Expert B Chess Computer", MACHINE_NO_SOUND|MACHINE_SUPPORTS_SAVE|MACHINE_NOT_WORKING | MACHINE_CLICKABLE_ARTWORK )
	CONS(  1989, academy,  0,       0,      academy,   academy, driver_device,  0,       "Hegener & Glaser",          "Mephisto Academy Schachcomputer", MACHINE_REQUIRES_ARTWORK|MACHINE_NOT_WORKING | MACHINE_CLICKABLE_ARTWORK )
	CONS(  1989, megaiv,   0,       0,      megaiv,    megaiv, driver_device,   0,       "Hegener & Glaser",          "Mephisto Mega IV Schachcomputer", MACHINE_SUPPORTS_SAVE|MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK | MACHINE_CLICKABLE_ARTWORK )
	CONS(  1989, milano,   polgar,  0,      milano,    polgar, polgar_state,   polgar,  "Hegener & Glaser",          "Mephisto Milano Schachcomputer", MACHINE_REQUIRES_ARTWORK | MACHINE_CLICKABLE_ARTWORK )
//CONS(  1989, montec4,  0,       0,      monteciv,  monteciv, driver_device, 0,       "Hegener & Glaser",          "Mephisto Monte Carlo IV", MACHINE_SUPPORTS_SAVE|MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK | MACHINE_CLICKABLE_ARTWORK )
	CONS(  1989, sfortec,  sfortea, 0,      sfortea,   sfortea, driver_device,  0,       "Novag",                     "Novag Super Forte Chess Computer (version C)", MACHINE_NO_SOUND|MACHINE_SUPPORTS_SAVE|MACHINE_NOT_WORKING | MACHINE_CLICKABLE_ARTWORK )
	CONS(  1989, sexpertc, sfortea, 0,      sfortea,   sfortea, driver_device,  0,       "Novag",                     "Novag Super Expert C Chess Computer", MACHINE_NO_SOUND|MACHINE_SUPPORTS_SAVE|MACHINE_NOT_WORKING | MACHINE_CLICKABLE_ARTWORK )
	CONS(  1990, lyon16,   van16,   0,      alm16,     van16, driver_device,    0,       "Hegener & Glaser Muenchen", "Mephisto Lyon 68000", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK | MACHINE_CLICKABLE_ARTWORK )
	CONS(  1990, lyon32,   van16,   0,      alm32,     van32, driver_device,    0,       "Hegener & Glaser Muenchen", "Mephisto Lyon 68020", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK | MACHINE_CLICKABLE_ARTWORK )
	CONS(  1990, monteciv, 0,       0,      monteciv,  monteciv, driver_device, 0,       "Hegener & Glaser",          "Mephisto Monte Carlo IV LE Schachcomputer", MACHINE_SUPPORTS_SAVE|MACHINE_NOT_WORKING|MACHINE_REQUIRES_ARTWORK | MACHINE_CLICKABLE_ARTWORK )
	CONS(  1991, diablo68, 0,       0,      diablo68,  sfortea, driver_device,  0,       "Novag",                     "Novag Diablo 68000 Chess Computer", MACHINE_NO_SOUND|MACHINE_SUPPORTS_SAVE|MACHINE_NOT_WORKING | MACHINE_CLICKABLE_ARTWORK )
	CONS(  1991, van16,    0,       0,      van16,     van16, driver_device,    0,       "Hegener & Glaser Muenchen", "Mephisto Vancouver 68000", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK | MACHINE_CLICKABLE_ARTWORK )
	CONS(  1991, van32,    van16,   0,      van32,     van32, driver_device,    0,       "Hegener & Glaser Muenchen", "Mephisto Vancouver 68020", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK | MACHINE_CLICKABLE_ARTWORK )
	CONS(  1992, risc,     0,       0,      risc,      van16, driver_device,    0,       "Saitek",                    "RISC2500", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_NOT_WORKING | MACHINE_CLICKABLE_ARTWORK )
	CONS(  1993, gen32,    van16,   0,      gen32,     gen32, driver_device,    0,       "Hegener & Glaser Muenchen", "Mephisto Genius030 V4.00", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK | MACHINE_CLICKABLE_ARTWORK )
	CONS(  1993, gen32_41, van16,   0,      gen32,     gen32, driver_device,    0,       "Hegener & Glaser Muenchen", "Mephisto Genius030 V4.01", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK | MACHINE_CLICKABLE_ARTWORK )
	CONS(  1993, gen32_oc, van16,   0,      gen32_oc,  gen32, driver_device,    0,       "Hegener & Glaser Muenchen", "Mephisto Genius030 V4.01OC", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK|MACHINE_UNOFFICIAL | MACHINE_CLICKABLE_ARTWORK )
	CONS(  1994, berlinp,  van16,   0,      bpl32,     bpl32, driver_device,    0,       "Hegener & Glaser Muenchen", "Mephisto Berlin Pro 68020", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK | MACHINE_CLICKABLE_ARTWORK )
	CONS(  1996, bpl32,    van16,   0,      bpl32,     bpl32, driver_device,    0,       "Hegener & Glaser Muenchen", "Mephisto Berlin Pro London Upgrade V5.00", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK | MACHINE_CLICKABLE_ARTWORK )
	CONS(  1996, lond020,  van16,   0,      van32,     van32, driver_device,    0,       "Hegener & Glaser Muenchen", "Mephisto London 68020 32 Bit", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK | MACHINE_CLICKABLE_ARTWORK )
	CONS(  1996, lond030,  van16,   0,      gen32,     gen32, driver_device,    0,       "Hegener & Glaser Muenchen", "Mephisto Genius030 London Upgrade V5.00", MACHINE_SUPPORTS_SAVE|MACHINE_REQUIRES_ARTWORK | MACHINE_CLICKABLE_ARTWORK )
