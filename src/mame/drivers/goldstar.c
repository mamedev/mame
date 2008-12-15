/***************************************************************************

Golden Star
Cherry Master

Golden Star and Cherry Master seem to be almost the same thing, running on
different hardware.  There are also various bootlegs / hacks, it isn't clear
exactly what hardware each runs on, some appear to have no OKI for example.

driver by Mirko Buffoni

***************************************************************************/
#include "driver.h"
#include "sound/ay8910.h"
#include "sound/okim6295.h"

static int dataoffset=0;

extern UINT8 *goldstar_video1, *goldstar_video2, *goldstar_video3;
extern size_t goldstar_video_size;
extern UINT8 *goldstar_reel1_scroll, *goldstar_reel2_scroll, *goldstar_reel3_scroll;



extern UINT8 *goldstar_reel1_ram;
extern WRITE8_HANDLER( goldstar_reel1_ram_w );
extern UINT8 *goldstar_reel2_ram;
extern WRITE8_HANDLER( goldstar_reel2_ram_w );
extern UINT8 *goldstar_reel3_ram;
extern WRITE8_HANDLER( goldstar_reel3_ram_w );

WRITE8_HANDLER( goldstar_fa00_w );
VIDEO_START( goldstar );
VIDEO_UPDATE( goldstar );
VIDEO_UPDATE( cherrym );

static UINT8 *nvram;
static size_t nvram_size;

static NVRAM_HANDLER( goldstar )
{
	if (read_or_write)
                mame_fwrite(file,nvram,nvram_size);
	else
	{
		if (file)
                        mame_fread(file,nvram,nvram_size);
		else
			memset(nvram,0xff,nvram_size);
	}
}



static WRITE8_HANDLER( protection_w )
{
	if (data == 0x2a)
		dataoffset = 0;
}

static READ8_HANDLER( protection_r )
{
	static const int data[4] = { 0x47, 0x4f, 0x4c, 0x44 };

	dataoffset %= 4;
	return data[dataoffset++];
}

static ADDRESS_MAP_START( goldstar_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xb7ff) AM_ROM
	AM_RANGE(0xb800, 0xbfff) AM_RAM AM_BASE(&nvram) AM_SIZE(&nvram_size)
	AM_RANGE(0xc000, 0xc7ff) AM_ROM
	AM_RANGE(0xc800, 0xcfff) AM_RAM AM_BASE(&videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0xd000, 0xd7ff) AM_RAM AM_BASE(&colorram)
	AM_RANGE(0xd800, 0xd9ff) AM_RAM AM_WRITE( goldstar_reel1_ram_w ) AM_BASE(&goldstar_reel1_ram)
	AM_RANGE(0xe000, 0xe1ff) AM_RAM AM_WRITE( goldstar_reel2_ram_w ) AM_BASE(&goldstar_reel2_ram)
	AM_RANGE(0xe800, 0xe9ff) AM_RAM AM_WRITE( goldstar_reel3_ram_w ) AM_BASE(&goldstar_reel3_ram)
	AM_RANGE(0xf040, 0xf07f) AM_RAM AM_BASE(&goldstar_reel1_scroll)
	AM_RANGE(0xf080, 0xf0bf) AM_RAM AM_BASE(&goldstar_reel2_scroll)
	AM_RANGE(0xf0c0, 0xf0ff) AM_RAM AM_BASE(&goldstar_reel3_scroll)

	AM_RANGE(0xf800, 0xf800) AM_READ_PORT("IN0")
	AM_RANGE(0xf801, 0xf801) AM_READ_PORT("IN1")	/* Test Mode */
	AM_RANGE(0xf802, 0xf802) AM_READ_PORT("DSW1")
//  AM_RANGE(0xf803, 0xf803)
//  AM_RANGE(0xf804, 0xf804)
	AM_RANGE(0xf805, 0xf805) AM_READ_PORT("DSW4")	/* DSW 4 (also appears in 8910 port) */
	AM_RANGE(0xf806, 0xf806) AM_READ_PORT("DSW7")	/* (don't know to which one of the */
	AM_RANGE(0xf810, 0xf810) AM_READ_PORT("UNK1")
	AM_RANGE(0xf811, 0xf811) AM_READ_PORT("UNK2")
	AM_RANGE(0xf820, 0xf820) AM_READ_PORT("DSW2")
	AM_RANGE(0xf830, 0xf830) AM_READWRITE(ay8910_read_port_0_r,ay8910_write_port_0_w)
	AM_RANGE(0xf840, 0xf840) AM_WRITE(ay8910_control_port_0_w)
	AM_RANGE(0xfa00, 0xfa00) AM_WRITE(goldstar_fa00_w)
	AM_RANGE(0xfb00, 0xfb00) AM_READWRITE(okim6295_status_0_r,okim6295_data_0_w)
	AM_RANGE(0xfd00, 0xfdff) AM_READWRITE(SMH_RAM,paletteram_BBGGGRRR_w) AM_BASE(&paletteram)
	AM_RANGE(0xfe00, 0xfe00) AM_READWRITE(protection_r,protection_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( ncb3_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xb7ff) AM_ROM
	AM_RANGE(0xb800, 0xbfff) AM_RAM AM_BASE(&nvram) AM_SIZE(&nvram_size)
	AM_RANGE(0xc000, 0xc7ff) AM_ROM
	AM_RANGE(0xc800, 0xcfff) AM_RAM AM_BASE(&videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0xd000, 0xd7ff) AM_RAM AM_BASE(&colorram)
	AM_RANGE(0xd800, 0xd9ff) AM_RAM AM_WRITE( goldstar_reel1_ram_w ) AM_BASE(&goldstar_reel1_ram)
	AM_RANGE(0xe000, 0xe1ff) AM_RAM AM_WRITE( goldstar_reel2_ram_w ) AM_BASE(&goldstar_reel2_ram)
	AM_RANGE(0xe800, 0xe9ff) AM_RAM AM_WRITE( goldstar_reel3_ram_w ) AM_BASE(&goldstar_reel3_ram)
	AM_RANGE(0xf040, 0xf07f) AM_RAM AM_BASE(&goldstar_reel1_scroll)
	AM_RANGE(0xf080, 0xf0bf) AM_RAM AM_BASE(&goldstar_reel2_scroll)
	AM_RANGE(0xf100, 0xf17f) AM_RAM AM_BASE(&goldstar_reel3_scroll) // moved compared to ncb3

	AM_RANGE(0xf800, 0xf800) AM_READ_PORT("IN0")
	AM_RANGE(0xf801, 0xf801) AM_READ_PORT("IN1")	/* Test Mode */
	AM_RANGE(0xf802, 0xf802) AM_READ_PORT("DSW1")
//  AM_RANGE(0xf803, 0xf803)
//  AM_RANGE(0xf804, 0xf804)
	AM_RANGE(0xf805, 0xf805) AM_READ_PORT("DSW4")	/* DSW 4 (also appears in 8910 port) */
	AM_RANGE(0xf806, 0xf806) AM_READ_PORT("DSW7")	/* (don't know to which one of the */
	AM_RANGE(0xf810, 0xf810) AM_READ_PORT("UNK1")
	AM_RANGE(0xf811, 0xf811) AM_READ_PORT("UNK2")
	AM_RANGE(0xf820, 0xf820) AM_READ_PORT("DSW2")
//	AM_RANGE(0xf830, 0xf830) AM_READWRITE(ay8910_read_port_0_r,ay8910_write_port_0_w)
//	AM_RANGE(0xf840, 0xf840) AM_WRITE(ay8910_control_port_0_w)
//	AM_RANGE(0xfa00, 0xfa00) AM_WRITE(goldstar_fa00_w)
//	AM_RANGE(0xfb00, 0xfb00) AM_READWRITE(okim6295_status_0_r,okim6295_data_0_w)
//	AM_RANGE(0xfd00, 0xfdff) AM_READWRITE(SMH_RAM,paletteram_BBGGGRRR_w) AM_BASE(&paletteram)
//	AM_RANGE(0xfe00, 0xfe00) AM_READWRITE(protection_r,protection_w)
ADDRESS_MAP_END



static ADDRESS_MAP_START( goldstar_readport, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x10) AM_READ_PORT("DSW6")
ADDRESS_MAP_END


static ADDRESS_MAP_START( cm_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM AM_WRITENOP


	AM_RANGE(0xb800, 0xbfff) AM_RAM AM_BASE(&nvram) AM_SIZE(&nvram_size) // not here..
	AM_RANGE(0xe000, 0xe7ff) AM_RAM AM_BASE(&videoram) AM_SIZE(&videoram_size)
	AM_RANGE(0xe800, 0xefff) AM_RAM AM_BASE(&colorram)

	AM_RANGE(0xd000, 0xd7ff) AM_RAM // main?
	AM_RANGE(0xd800, 0xdfff) AM_RAM

	AM_RANGE(0xf000, 0xf1ff) AM_RAM AM_WRITE( goldstar_reel1_ram_w ) AM_BASE(&goldstar_reel1_ram)
	AM_RANGE(0xf200, 0xf3ff) AM_RAM AM_WRITE( goldstar_reel2_ram_w ) AM_BASE(&goldstar_reel2_ram)
	AM_RANGE(0xf400, 0xf5ff) AM_RAM AM_WRITE( goldstar_reel3_ram_w ) AM_BASE(&goldstar_reel3_ram)
	AM_RANGE(0xf600, 0xf7ff) AM_RAM

	AM_RANGE(0xf800, 0xf87f) AM_RAM AM_BASE(&goldstar_reel1_scroll)
	AM_RANGE(0xf880, 0xf9ff) AM_RAM
	AM_RANGE(0xfa00, 0xfa7f) AM_RAM AM_BASE(&goldstar_reel2_scroll)
	AM_RANGE(0xfa80, 0xfbff) AM_RAM
	AM_RANGE(0xfc00, 0xfc7f) AM_RAM AM_BASE(&goldstar_reel3_scroll)
	AM_RANGE(0xfc80, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cm_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x01, 0x01) AM_READ(ay8910_read_port_0_r)
	AM_RANGE(0x02, 0x02) AM_WRITE(ay8910_write_port_0_w)
	AM_RANGE(0x03, 0x03) AM_WRITE(ay8910_control_port_0_w)
	AM_RANGE(0x04, 0x04) AM_READ_PORT("PLAYER")
	AM_RANGE(0x05, 0x05) AM_READ_PORT("COIN")
	AM_RANGE(0x06, 0x06) AM_READ_PORT("TEST")
	AM_RANGE(0x07, 0x07) AM_WRITENOP
	AM_RANGE(0x08, 0x08) AM_READ_PORT("DSW1")
	AM_RANGE(0x09, 0x09) AM_READ_PORT("DSW2")
	AM_RANGE(0x0a, 0x0a) AM_READ_PORT("DSW3")
	AM_RANGE(0x0b, 0x0b) AM_WRITENOP
	AM_RANGE(0x10, 0x10) AM_WRITENOP
	AM_RANGE(0x11, 0x11) AM_WRITENOP
	AM_RANGE(0x12, 0x12) AM_WRITENOP
	AM_RANGE(0x13, 0x13) AM_WRITENOP
ADDRESS_MAP_END

static INPUT_PORTS_START( cmv801 )
	PORT_START("PLAYER")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 0") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 1") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Big")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Double Up")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Stop / Take Score")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Bet")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Small")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 0") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 1") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 3") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) // PORT_NAME("Bit 4") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN4 ) // PORT_NAME("Bit 5") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )		// Key-In
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )		// 10 Credits

	PORT_START("TEST")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 0") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 1") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 2") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Bit 3") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("Payout")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Limit Over")
	PORT_SERVICE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F2) PORT_NAME("Analyzer")

	PORT_START("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x01, 0x00, "SW1:!1" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x02, 0x00, "Hopper Out Switch" ) PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(    0x02, "Active High" )
	PORT_DIPSETTING(    0x00, "Active Low" )
	PORT_DIPNAME( 0x04, 0x00, "Type Of Payout" ) PORT_DIPLOCATION("SW1:!3")
	PORT_DIPSETTING(    0x04, "Automatic" )
	PORT_DIPSETTING(    0x00, "Payout SH" )
	PORT_DIPNAME( 0x08, 0x00, "@ IN DoubleUp" ) PORT_DIPLOCATION("SW1:!4")
	PORT_DIPSETTING(    0x08, "Even" )
	PORT_DIPSETTING(    0x00, "LOST" )
	PORT_DIPNAME( 0x10, 0x00, "DoubleUp Pay-Rate" ) PORT_DIPLOCATION("SW1:!5")
	PORT_DIPSETTING(    0x00, "80%" )
	PORT_DIPSETTING(    0x10, "90%" )
	PORT_DIPNAME( 0x20, 0x00, "DoubleUp" ) PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Bet Max" ) PORT_DIPLOCATION("SW1:!7,!8") /* Manual says 8/16/32/64 */
	PORT_DIPSETTING(    0x00, "16 Bet" )
	PORT_DIPSETTING(    0x40, "32 Bet" )
	PORT_DIPSETTING(    0x80, "64 Bet" )
	PORT_DIPSETTING(    0xc0, "96 Bet" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Main Game Pay-Rate" ) PORT_DIPLOCATION("SW2:!1,!2,!3")
	PORT_DIPSETTING(    0x07, "55 30% 45" ) /* Displays 35% */
	PORT_DIPSETTING(    0x06, "60 38% 50" ) /* Displays 40% */
	PORT_DIPSETTING(    0x05, "65 46% 55" ) /* Displays 45% */
	PORT_DIPSETTING(    0x04, "70 54% 60" ) /* Displays 50% */
	PORT_DIPSETTING(    0x03, "75 62% 65" ) /* Displays 55% */
	PORT_DIPSETTING(    0x02, "80 70% 70" ) /* Displays 60% */
	PORT_DIPSETTING(    0x01, "85 78% 75" ) /* Displays 65% */
	PORT_DIPSETTING(    0x00, "90 86% 80" ) /* Displays 70% */
	PORT_DIPNAME( 0x18, 0x18, "Hopper Limit" ) PORT_DIPLOCATION("SW2:!4,!5")
	PORT_DIPSETTING(    0x00, "300" )
	PORT_DIPSETTING(    0x08, "500" )
	PORT_DIPSETTING(    0x10, "1000" )
	PORT_DIPSETTING(    0x18, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x20, "Sound On/Off Odds Over 100" ) PORT_DIPLOCATION("SW2:!6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Type Of Key In" ) PORT_DIPLOCATION("SW2:!7") /* Manual says C-Type/D-Type probably a typo */
	PORT_DIPSETTING(    0x40, "A-Type" )
	PORT_DIPSETTING(    0x00, "B-Type" )
	PORT_DIPNAME( 0x80, 0x80, "Bet Limit For Center Super 7" ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPSETTING(    0x80, "Limited" ) /* "Number is fixed by 4-6" */

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Key In Rate" ) PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" ) PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40) // A-Type
	PORT_DIPSETTING(    0x01, "1 Coin/20 Credits" ) PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x02, "1 Coin/50 Credits" ) PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x03, "1 Coin/100 Credits" ) PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x40)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) ) PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00) // B-Type
	PORT_DIPSETTING(    0x01, "1 Coin/10 Credits" ) PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x02, "1 Coin/25 Credits" ) PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x03, "1 Coin/50 Credits" ) PORT_CONDITION("DSW2",0x40,PORTCOND_EQUALS,0x00)
	PORT_DIPNAME( 0x0c, 0x0c, "Coin A Rate" ) PORT_DIPLOCATION("SW3:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x30, 0x30, "Coin D Rate" ) PORT_DIPLOCATION("SW3:5,6")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00) // C-Type
	PORT_DIPSETTING(    0x10, "1 Coin/10 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x20, "1 Coin/25 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x30, "1 Coin/50 Credits" ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10) // D-Type
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSW4",0x10,PORTCOND_EQUALS,0x10)
	PORT_DIPNAME( 0xc0, 0xc0, "Coin C Rate" ) PORT_DIPLOCATION("SW3:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x07, "Credit Limit" ) PORT_DIPLOCATION("SW4:1,2,3")
	PORT_DIPSETTING(    0x07, "5,000" )
	PORT_DIPSETTING(    0x06, "10,000" )
	PORT_DIPSETTING(    0x05, "20,000" )
	PORT_DIPSETTING(    0x04, "30,000" )
	PORT_DIPSETTING(    0x03, "40,000" )
	PORT_DIPSETTING(    0x02, "50,000" )
	PORT_DIPSETTING(    0x01, "100,000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x08, 0x08, "Display Of Payout Limit" ) PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Type Of Coin D" ) PORT_DIPLOCATION("SW4:5")
	PORT_DIPSETTING(    0x10, "C-Type" )
	PORT_DIPSETTING(    0x00, "D-Type" )
	PORT_DIPNAME( 0x20, 0x20, "Min. Bet For Bonus Play" ) PORT_DIPLOCATION("SW4:6")
	PORT_DIPSETTING(    0x20, "16 Bet" )
	PORT_DIPSETTING(    0x00, "8 Bet" )
	PORT_DIPNAME( 0x40, 0x40, "Reel Speed" ) PORT_DIPLOCATION("SW4:7")
	PORT_DIPSETTING(    0x40, DEF_STR( High ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Low ) )
	PORT_DIPNAME( 0x80, 0x80, "Hopper Out By Coin A" ) PORT_DIPLOCATION("SW4:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, "Display Of Doll On Demo" ) PORT_DIPLOCATION("SW5:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x06, 0x06, "Coin In Limit" ) PORT_DIPLOCATION("SW5:2,3")
	PORT_DIPSETTING(    0x06, "1,000" )
	PORT_DIPSETTING(    0x04, "5,000" )
	PORT_DIPSETTING(    0x02, "10,000" )
	PORT_DIPSETTING(    0x00, "20,000" )
	PORT_DIPNAME( 0x18, 0x18, "Condition For 3 Kind Of Bonus" ) PORT_DIPLOCATION("SW5:4,5")
	PORT_DIPSETTING(    0x18, "12-7-1" )
	PORT_DIPSETTING(    0x10, "9-5-1" )
	PORT_DIPSETTING(    0x08, "6-3-1" )
	PORT_DIPSETTING(    0x00, "3-2-1" )
	PORT_DIPNAME( 0x20, 0x20, "Display Of Doll At All Fr. Bonus" ) PORT_DIPLOCATION("SW5:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW5:7" ) /* Listed as "Unused" */
	PORT_DIPNAME( 0x80, 0x80, "Test Mode For Disp. Of Doll" ) PORT_DIPLOCATION("SW5:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( goldstar )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CODE(KEYCODE_V) PORT_NAME("Bet Red/2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CODE(KEYCODE_C) PORT_NAME("Stop 3/Small/1/Info")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CODE(KEYCODE_B) PORT_NAME("Bet Blue/Double/3")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CODE(KEYCODE_Z) PORT_NAME("Stop 1/Take")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CODE(KEYCODE_X) PORT_NAME("Stop 2/Big/Ticket")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CODE(KEYCODE_N) PORT_NAME("Start/Stop All/4")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )	/* this is not a coin, not sure what it is */
												/* maybe it's used to buy tickets. Will check soon. */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x40, IP_ACTIVE_LOW )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F1) PORT_NAME("Statistics")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "Game Style" )
	PORT_DIPSETTING(    0x01, "Gettoni" )
	PORT_DIPSETTING(    0x00, "Ticket" )
	PORT_DIPNAME( 0x02, 0x02, "Hopper Out" )
	PORT_DIPSETTING(    0x02, "Active Low" )
	PORT_DIPSETTING(    0x00, "Active High" )
	PORT_DIPNAME( 0x04, 0x04, "Payout Automatic?" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "W-Up '7'" )
	PORT_DIPSETTING(    0x08, "Loss" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPNAME( 0x10, 0x10, "W-Up Pay Rate" )
	PORT_DIPSETTING(    0x10, "60%" )
	PORT_DIPSETTING(    0x00, "70%" )
	PORT_DIPNAME( 0x20, 0x20, "W-Up Game" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, "Bet Max" )
	PORT_DIPSETTING(    0xc0, "8 Bet" )
	PORT_DIPSETTING(    0x80, "16 Bet" )
	PORT_DIPSETTING(    0x40, "32 Bet" )
	PORT_DIPSETTING(    0x00, "50 Bet" )

	PORT_START("UNK1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x00, "Main Game Pay Rate" )
	PORT_DIPSETTING(    0x00, "75 %" )
	PORT_DIPSETTING(    0x01, "70 %" )
	PORT_DIPSETTING(    0x02, "65 %" )
	PORT_DIPSETTING(    0x03, "60 %" )
	PORT_DIPSETTING(    0x04, "55 %" )
	PORT_DIPSETTING(    0x05, "50 %" )
	PORT_DIPSETTING(    0x06, "45 %" )
	PORT_DIPSETTING(    0x07, "40 %" )
	PORT_DIPNAME( 0x18, 0x00, "Hopper Limit" )
	PORT_DIPSETTING(    0x18, "300" )
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x08, "1000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x20, 0x00, "100 Odds Sound" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "Key-In Type" )
	PORT_DIPSETTING(    0x40, "B-Type" )
	PORT_DIPSETTING(    0x00, "A-Type" )
	PORT_DIPNAME( 0x80, 0x00, "Center Super 7 Bet Limit" )
	PORT_DIPSETTING(    0x80, "Unlimited" )
	PORT_DIPSETTING(    0x00, "Limited" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x04, "1 Coin/20 Credits" )
	PORT_DIPSETTING(    0x08, "1 Coin/50 Credits" )
	PORT_DIPSETTING(    0x0c, "1 Coin/100 Credits" )
	PORT_DIPNAME( 0xc0, 0x40, "Coin C" )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xc0, "1 Coin/10 Credits" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x07, 0x06, "Credit Limited" )
	PORT_DIPSETTING(    0x07, "5000" )
	PORT_DIPSETTING(    0x06, "10000" )
	PORT_DIPSETTING(    0x05, "20000" )
	PORT_DIPSETTING(    0x04, "30000" )
	PORT_DIPSETTING(    0x03, "40000" )
	PORT_DIPSETTING(    0x02, "50000" )
	PORT_DIPSETTING(    0x01, "100000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x08, 0x00, "Display Credit Limit" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Type of Coin D" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Bonus Play Min Bet" )
	PORT_DIPSETTING(    0x20, "16 Bet" )
	PORT_DIPSETTING(    0x00, "8 Bet" )
	PORT_DIPNAME( 0x40, 0x00, "Reel Speed" )
	PORT_DIPSETTING(    0x40, DEF_STR( Low ) )
	PORT_DIPSETTING(    0x00, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x00, "Ticket Payment" )
	PORT_DIPSETTING(    0x80, "1 Ticket/100" )
	PORT_DIPSETTING(    0x00, "Pay All" )

	PORT_START("DSW6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW7")	/* ??? */
	PORT_BIT( 0xdf, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x20, 0x00, "Show Woman" )
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END




static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	4096,    /* 4096 characters */
	3,      /* 3 bits per pixel */
	{ 2, 4, 6 }, /* the bitplanes are packed in one byte */
	{ 0*8+0, 0*8+1, 1*8+0, 1*8+1, 2*8+0, 2*8+1, 3*8+0, 3*8+1 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8   /* every char takes 32 consecutive bytes */
};


static const gfx_layout charlayout_chry10 =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,1),    /* 4096 characters */
	3,      /* 3 bits per pixel */
	{ 2, 4, 6 }, /* the bitplanes are packed in one byte */
	{ 3*8+0, 3*8+1, 2*8+0, 2*8+1, 1*8+0, 1*8+1, 0*8+0, 0*8+1 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8   /* every char takes 32 consecutive bytes */
};

static const gfx_layout tilelayout =
{
	8,32,    /* 8*32 characters */
	256,    /* 256 tiles */
	4,      /* 4 bits per pixel */
	{ 0, 2, 4, 6 },
	{ 0, 1, 1*8+0, 1*8+1, 2*8+0, 2*8+1, 3*8+0, 3*8+1 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8,
			32*8, 36*8, 40*8, 44*8, 48*8, 52*8, 56*8, 60*8,
			64*8, 68*8, 72*8, 76*8, 80*8, 84*8, 88*8, 92*8,
			96*8, 100*8, 104*8, 108*8, 112*8, 116*8, 120*8, 124*8 },
	128*8   /* every char takes 128 consecutive bytes */
};


static const gfx_layout tilelayoutbl =
{
	8,32,    /* 8*32 characters */
	256,    /* 256 tiles */
	4,      /* 4 bits per pixel */
	{ 0, 2, 4, 6 },
	{ 0, 1, 2*8+0, 2*8+1, 1*8+0, 1*8+1, 3*8+0, 3*8+1 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8,
			32*8, 36*8, 40*8, 44*8, 48*8, 52*8, 56*8, 60*8,
			64*8, 68*8, 72*8, 76*8, 80*8, 84*8, 88*8, 92*8,
			96*8, 100*8, 104*8, 108*8, 112*8, 116*8, 120*8, 124*8 },
	128*8   /* every char takes 128 consecutive bytes */
};

static const gfx_layout tilelayout_chry10 =
{
	8,32,    /* 8*32 characters */
	256,    /* 256 tiles */
	4,      /* 4 bits per pixel */
	{ 0, 2, 4, 6 },
	{ 3*8+0, 3*8+1, 2*8+0, 2*8+1, 1*8+0, 1*8+1, 0*8+0, 0*8+1 },
	{ 0*8, 4*8, 8*8, 12*8, 16*8, 20*8, 24*8, 28*8,
			32*8, 36*8, 40*8, 44*8, 48*8, 52*8, 56*8, 60*8,
			64*8, 68*8, 72*8, 76*8, 80*8, 84*8, 88*8, 92*8,
			96*8, 100*8, 104*8, 108*8, 112*8, 116*8, 120*8, 124*8 },
	128*8   /* every char takes 128 consecutive bytes */
};



static const gfx_layout tiles8x8x3_layout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static const gfx_layout tiles8x32x4_layout =
{
	8,32,
	RGN_FRAC(1,4),
	4,
	{ RGN_FRAC(0,4), RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8,  2*8,  3*8,  4*8,  5*8,  6*8,  7*8,
	  8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8,
	 16*8,17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8,
     24*8,25*8, 26*8, 27*8, 28*8, 29*8, 30*8, 31*8
	},
	32*8
};




static GFXDECODE_START( goldstar )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 128,  8 )
GFXDECODE_END
static GFXDECODE_START( bl )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayoutbl, 128,  8 )
GFXDECODE_END
static GFXDECODE_START( ml )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0x18000, tilelayout, 128,  8 )
GFXDECODE_END

static GFXDECODE_START( chry10 )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout_chry10,   0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout_chry10, 128,  8 )
GFXDECODE_END

static GFXDECODE_START( ncb3 )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8x3_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tiles8x32x4_layout, 128+64, 4 )
GFXDECODE_END

static const ay8910_interface ay8910_config =
{
	AY8910_LEGACY_OUTPUT,
	AY8910_DEFAULT_LOADS,
	input_port_7_r,	/* DSW 4 */
	input_port_6_r,	/* DSW 3 */
	NULL,
	NULL
};

static MACHINE_DRIVER_START( goldstar )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80, 3579545)//(4000000?)
	MDRV_CPU_PROGRAM_MAP(goldstar_map,0)
	MDRV_CPU_IO_MAP(goldstar_readport,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(goldstar)
	MDRV_PALETTE_LENGTH(256)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(goldstar)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")//set up a standard mono speaker called 'mono'
	MDRV_SOUND_ADD("ay", AY8910,1500000)//1 AY8910, at clock 150000Hz
	MDRV_SOUND_CONFIG(ay8910_config)//read extra data from interface
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)//all sound goes to the 'mono' speaker, at 0.50 X maximum

	MDRV_SOUND_ADD("oki", OKIM6295, 1056000)//clock
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified //"oki"
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)//all sound goes to the 'mono' speaker, at 1.0 X maximum

MACHINE_DRIVER_END


static MACHINE_DRIVER_START( goldstbl )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80, 3579545)//(4000000?)
	MDRV_CPU_PROGRAM_MAP(goldstar_map,0)
	MDRV_CPU_IO_MAP(goldstar_readport,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(bl)
	MDRV_PALETTE_LENGTH(256)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(goldstar)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")//set up a standard mono speaker called 'mono'
	MDRV_SOUND_ADD("ay", AY8910,1500000)//1 AY8910, at clock 150000Hz
	MDRV_SOUND_CONFIG(ay8910_config)//read extra data from interface
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)//all sound goes to the 'mono' speaker, at 0.50 X maximum

	MDRV_SOUND_ADD("oki", OKIM6295, 1056000)//clock
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified //"oki"
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)//all sound goes to the 'mono' speaker, at 1.0 X maximum
MACHINE_DRIVER_END


static MACHINE_DRIVER_START( moonlght )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80, 3579545)//(4000000?)
	MDRV_CPU_PROGRAM_MAP(goldstar_map,0)
	MDRV_CPU_IO_MAP(goldstar_readport,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(ml)
	MDRV_PALETTE_LENGTH(256)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(goldstar)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")//set up a standard mono speaker called 'mono'
	MDRV_SOUND_ADD("ay", AY8910,1500000)//1 AY8910, at clock 150000Hz
	MDRV_SOUND_CONFIG(ay8910_config)//read extra data from interface
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)//all sound goes to the 'mono' speaker, at 0.50 X maximum

	MDRV_SOUND_ADD("oki", OKIM6295, 1056000)//clock
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high)// clock frequency & pin 7 not verified //"oki"
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)//all sound goes to the 'mono' speaker, at 1.0 X maximum
MACHINE_DRIVER_END



static MACHINE_DRIVER_START( chry10 )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80, 3579545)//(4000000?)
	MDRV_CPU_PROGRAM_MAP(goldstar_map,0)
	MDRV_CPU_IO_MAP(goldstar_readport,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(chry10)
	MDRV_PALETTE_LENGTH(256)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(goldstar)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")//set up a standard mono speaker called 'mono'
	MDRV_SOUND_ADD("ay", AY8910,1500000)//1 AY8910, at clock 150000Hz
	MDRV_SOUND_CONFIG(ay8910_config)//read extra data from interface
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)//all sound goes to the 'mono' speaker, at 0.50 X maximum

	MDRV_SOUND_ADD("oki", OKIM6295, 1056000)//clock
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified //"oki"
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)//all sound goes to the 'mono' speaker, at 1.0 X maximum
MACHINE_DRIVER_END




static MACHINE_DRIVER_START( ncb3 )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80, 3579545)//(4000000?)
	MDRV_CPU_PROGRAM_MAP(ncb3_map,0)
	MDRV_CPU_IO_MAP(goldstar_readport,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(ncb3)
	MDRV_PALETTE_LENGTH(256)
	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(goldstar)
	MDRV_VIDEO_UPDATE(goldstar)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD("ay", AY8910,1500000)
	MDRV_SOUND_CONFIG(ay8910_config)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_DRIVER_END

// wrong
static PALETTE_INIT(cm)
{
	int i;
	for (i=0;i<0x100;i++)
	{
		int r,g,b;
		UINT8 dat;

		UINT8*proms = memory_region(machine, "proms");

		dat = proms[0x000+i] | (proms[0x100+i]<<4);

		r = (dat & 0x07) << 5;
		g = (dat & 0x38) << 2;
		b = (dat & 0xc0) << 0;

		palette_set_color(machine, i, MAKE_RGB(r, g, b));

	}
}
static MACHINE_DRIVER_START( cm )

	/* basic machine hardware */
	MDRV_CPU_ADD("main", Z80, 3579545)//(4000000?)
	MDRV_CPU_PROGRAM_MAP(cm_map,0)
	MDRV_CPU_IO_MAP(cm_portmap,0)
	MDRV_CPU_VBLANK_INT("main", irq0_line_hold)

	/* video hardware */
	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
//  MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 64*8-1, 2*8, 30*8-1)

	MDRV_GFXDECODE(ncb3)
	MDRV_PALETTE_LENGTH(256)
	MDRV_PALETTE_INIT(cm)
//	MDRV_NVRAM_HANDLER(goldstar)

	MDRV_VIDEO_START(goldstar)
	MDRV_VIDEO_UPDATE(cherrym)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")//set up a standard mono speaker called 'mono'
	MDRV_SOUND_ADD("ay", AY8910,1500000)//1 AY8910, at clock 150000Hz
	MDRV_SOUND_CONFIG(ay8910_config)//read extra data from interface
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)//all sound goes to the 'mono' speaker, at 0.50 X maximum

	MDRV_SOUND_ADD("oki", OKIM6295, 1056000)//clock
	MDRV_SOUND_CONFIG(okim6295_interface_pin7high) // clock frequency & pin 7 not verified //"oki"
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)//all sound goes to the 'mono' speaker, at 1.0 X maximum
MACHINE_DRIVER_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( goldstar )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "gs4-cpu.bin",  0x0000, 0x10000, CRC(73e47d4d) SHA1(df2d8233572dc12e8a4b56e5d4f6c566e4ababc9) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "gs2.bin",      0x00000, 0x20000, CRC(a2d5b898) SHA1(84cca22c91628cfefb67013652b151f034a06159) )

	ROM_REGION( 0x08000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "gs3.bin",      0x00000, 0x08000, CRC(8454ce3c) SHA1(74686ebb91f191db8cbc3d0417a5e8112c5b67b1) )

	ROM_REGION( 0x20000, "oki", 0 )	/* Audio ADPCM */
	ROM_LOAD( "gs1-snd.bin",  0x0000, 0x20000, CRC(9d58960f) SHA1(c68edf95743e146398aabf6b9617d18e1f9bf25b) )
ROM_END


ROM_START( goldstbl )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "gsb-cpu.bin",  0x0000, 0x10000, CRC(82b238c3) SHA1(1306e700e213f423bdd79b182aa11335796f7f38) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "gs2.bin",      0x00000, 0x20000, CRC(a2d5b898) SHA1(84cca22c91628cfefb67013652b151f034a06159) )

	ROM_REGION( 0x08000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "gsb-spr.bin",  0x00000, 0x08000, CRC(52ecd4c7) SHA1(7ef013020521a0c19ecd67db1c00047e78a3c736) )

	ROM_REGION( 0x20000, "oki", 0 )	/* Audio ADPCM */
	ROM_LOAD( "gs1-snd.bin",  0x0000, 0x20000, CRC(9d58960f) SHA1(c68edf95743e146398aabf6b9617d18e1f9bf25b) )
ROM_END

/*

Cherry I Gold

Anno    1998
Produttore
N.revisione W4BON (rev.1)


CPU

1x TMPZ84C00AP-6 (u12)(main)
2x D8255AC-2 (u45,u46) (missing)
1x D71055C (u40) (missing)
1x YM2149 (u39)
1x SN76489AN (u38)
1x oscillator 12.0C45

ROMs

1x I27256 (u3)
1x I27C010 (u1)
1x PROM N82S147AN (u2)
1x M27C512 (u20)
1x GAL20V8 (pl1)(read protected)
1x PALCE20V8H (pl2)(read protected)
1x ispLSI1024-60LJ (pl3)(read protected)
3x PALCE16V8H (pl4,pl6,pl7)(read protected)
1x PEEL22CV10 (pl5)(read protected)
Note

1x 36x2 edge connector
1x 10x2 edge connector
2x trimmer (volume)
5x 8x2 switches dip (sw1-5)
1x push lever (TS)


Cherry Gold  (Cherry 10)

Anno    1997
Produttore
N.revisione W4BON (rev.1)

CPU

1x TMPZ84C00AP-6 (u12)(main)
2x D8255AC-2 (u45,u46)
1x D71055C (u40)
1x WF19054 (u39)(equivalent to AY-3-8910)
1x SN76489AN (u38)
1x PIC16F84 (on a small daughterboard)(read protected)
1x oscillator 12.000

ROMs

1x TMS27C256 (u3)
1x TMS27C010 (u1)
1x PROM N82S147AN (u2)
1x M27C512 (u20)
2x PALCE20V8H (pl1,pl2)(read protected)
1x ispLSI1024-60LJ (pl3)(read protected)
3x PALCE16V8H (pl4,pl6,pl7)(read protected)
1x GAL22V10B (pl5)(read protected)

Note

1x 36x2 edge connector
1x 10x2 edge connector
2x trimmer (volume)
5x 8x2 switches dip (sw1-5)
1x push lever (TS)

*/

ROM_START( chry10 )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "ver.1h2.u20",  0x0000, 0x10000, CRC(85bbde06) SHA1(f44d335feb4697b195e9fc7e5aeaabf099e21ed8) )

	ROM_REGION( 0x10000, "pic", 0 )
	ROM_LOAD( "pic16f84.bad.dump",    0x00000, 0x014f4, BAD_DUMP CRC(876ff1ed) SHA1(fcd6892e2b8371030af15e4d8c9f4a351ce0551c) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "27c010.u1",      0x00000, 0x20000, CRC(05515cf8) SHA1(366dd44ae93bdc4cf456f97f38edac83441cbc89) )

	ROM_REGION( 0x08000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "1.u3",      0x00000, 0x08000, CRC(32b46e5c) SHA1(49e59589188324e15ec2b8157839423faea9833f) )

	ROM_REGION( 0x0200, "prom", ROMREGION_DISPOSE )
	ROM_LOAD( "82s147.u2",      0x00000, 0x0200, CRC(5c8f2b8f) SHA1(67d2121e75813dd85d83858c5fc5ec6ad9cc2a7d) )

	ROM_REGION( 0x02e5, "palgal", ROMREGION_DISPOSE )
	ROM_LOAD( "palce20v8h.pl1.bad.dump",    0x00000, 0x0157, BAD_DUMP CRC(f0c6d78c) SHA1(03ff589711179950209c405192bd41a032c6c6d6) )
	ROM_LOAD( "palce20v8h.pl2.bad.dump",    0x00000, 0x0157, BAD_DUMP CRC(f0c6d78c) SHA1(03ff589711179950209c405192bd41a032c6c6d6) )
	ROM_LOAD( "palce16v8h.pl4.bad.dump",    0x00000, 0x0117, BAD_DUMP CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )
	ROM_LOAD( "gal22v10b.pl5.bad.dump",     0x00000, 0x02e5, BAD_DUMP CRC(996854bc) SHA1(647d2f49b739f7ca55c0b85290b6a21256834fd8) )
	ROM_LOAD( "palce16v8h.pl6.bad.dump",    0x00000, 0x0117, BAD_DUMP CRC(7e3d99d8) SHA1(983e10eba11e4aeab5103ae644a8e6181d9b27a9) )
	ROM_LOAD( "palce16v8h.pl7.bad.dump",    0x00000, 0x0117, BAD_DUMP CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )

	ROM_REGION( 0x20000, "oki", ROMREGION_ERASE00 )
	/* no oki on this pcb .. */
ROM_END



ROM_START( chryigld )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "ol-v9.u20",  0x00000, 0x10000, CRC(b61c0695) SHA1(63c44b20fd7f76bdb33331273d2610e8cfd31add) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "ol-la.u1",      0x00000, 0x20000, CRC(c3c912f1) SHA1(a2131f092ae1971f79a11d6a18b031cd98529320) )

	ROM_REGION( 0x08000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "1.u3",      0x00000, 0x08000, CRC(32b46e5c) SHA1(49e59589188324e15ec2b8157839423faea9833f) )

	ROM_REGION( 0x0200, "prom", ROMREGION_DISPOSE )
	ROM_LOAD( "82s147.u2",      0x00000, 0x0200, CRC(5c8f2b8f) SHA1(67d2121e75813dd85d83858c5fc5ec6ad9cc2a7d) )

	ROM_REGION( 0x02dd, "palgal", ROMREGION_DISPOSE )
	ROM_LOAD( "gal20v8.pl1.bad.dump",    0x00000, 0x0157, BAD_DUMP CRC(bf885908) SHA1(6cac1022172ee0c178fd3b9c187b1ffb4742898f) )
	ROM_LOAD( "palce20v8h.pl2.bad.dump", 0x00000, 0x0157, BAD_DUMP CRC(f0c6d78c) SHA1(03ff589711179950209c405192bd41a032c6c6d6) )
	ROM_LOAD( "palce16v8h.pl4.bad.dump", 0x00000, 0x0117, BAD_DUMP CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )
	ROM_LOAD( "peel22cv10a.pl5.bad.dump",0x00000, 0x02dd, BAD_DUMP CRC(8e6075d9) SHA1(f2c1b6497a4d9e873d36b89771c135a2cd91d05f) )
	ROM_LOAD( "palce16v8h.pl6.bad.dump", 0x00000, 0x0117, BAD_DUMP CRC(7e3d99d8) SHA1(983e10eba11e4aeab5103ae644a8e6181d9b27a9) )
	ROM_LOAD( "palce16v8h.pl7.bad.dump", 0x00000, 0x0117, BAD_DUMP CRC(c89d2f52) SHA1(f9d52d9c42ef95b7b85bbf6d09888ebdeac11fd3) )

	ROM_REGION( 0x20000, "oki", ROMREGION_ERASE00 )
	/* no oki on this pcb .. */
ROM_END



ROM_START( moonlght )
	ROM_REGION( 0x20000, "main", 0 )
	ROM_LOAD( "4.bin",  	  0x0000, 0x20000, CRC(ecb06cfb) SHA1(e32613cac5583a0fecf04fca98796b91698e530c) )

	ROM_REGION( 0x20000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "28.bin",      0x00000, 0x20000, CRC(76915c0f) SHA1(3f6d1c0dd3d9bf29538181a0e930291b822dad8c) )

	ROM_REGION( 0x20000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "29.bin",      0x00000, 0x20000, CRC(8a5f274d) SHA1(0f2ad61b00e220fc509c01c11c1a8f4e47b54f2a) )

	ROM_REGION( 0x20000, "oki", 0 )	/* Audio ADPCM */
	ROM_LOAD( "gs1-snd.bin",  0x0000, 0x20000, CRC(9d58960f) SHA1(c68edf95743e146398aabf6b9617d18e1f9bf25b) )
ROM_END


ROM_START( ncb3 )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "8.512", 0x00000, 0x10000, CRC(1f669cd0) SHA1(fd394119e33c017507fde87a710577e37dcdec07) )

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "2.256", 0x00000, 0x08000, CRC(83650a94) SHA1(e79420ab559d3f74013708767ca3f238fd333fb7) )
	ROM_LOAD( "3.256", 0x08000, 0x08000, CRC(2f46a3f5) SHA1(3e6022ee8f84039e48f41aea5e68ee28aabdc556) )
	//ROM_LOAD( "4.256", 0x10000, 0x08000, BAD_DUMP CRC(a390f1f2) SHA1(0a04a5af51f91f04773125f703c7cd3397d192f2) ) // FIXED BITS (xxxx1xxx) - use main_7.256 from set below instead?
	ROM_LOAD( "main_7.256", 0x10000, 0x08000, CRC(dcf97517) SHA1(0a29696e0464c8878c499b1786a17080fd088a72) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "1.764", 0x00000, 0x02000, CRC(cbcc6bfb) SHA1(5bafc934fef1f50d8c182c39d3a7ce795c89d175) )
	ROM_LOAD( "5.764", 0x02000, 0x02000, CRC(91162010) SHA1(3acc21e7074602b247f2f392eb181802092d2f21) )
	ROM_LOAD( "6.764", 0x04000, 0x02000, CRC(e73ea4e3) SHA1(c9fd56461f6986d6bc170403d298fcc408a524e9) )
	ROM_LOAD( "7.764", 0x06000, 0x02000, CRC(7cc6d26b) SHA1(de33e8985affce7bd3ead89463117c9aaa93d5e4) )

	ROM_REGION( 0x0200, "prom", ROMREGION_DISPOSE )
	/* prom missing? */

	ROM_REGION( 0x20000, "oki", ROMREGION_ERASE00 )
	/* no oki on this pcb? */
ROM_END

/*
mame -romident cb3.zip
cpu_u6.512          NO MATCH
main_3.764          = 5.764                 New Cherry Bonus 3
main_4.764          = 1.764                 New Cherry Bonus 3
main_5.256          = 2.256                 New Cherry Bonus 3
main_6.256          = 3.256                 New Cherry Bonus 3
main_7.256          NO MATCH

C:\mame061208>src\mame\mamedriv.c

*/

ROM_START( cb3 )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "cpu_u6.512", 0x00000, 0x10000, CRC(d17c936b) SHA1(bf90edd214118116da675bcfca41247d5891ac90) ) // encrypted??

	ROM_REGION( 0x18000, "gfx1", 0 )
	ROM_LOAD( "main_5.256", 0x00000, 0x08000, CRC(83650a94) SHA1(e79420ab559d3f74013708767ca3f238fd333fb7) )
	ROM_LOAD( "main_6.256", 0x08000, 0x08000, CRC(2f46a3f5) SHA1(3e6022ee8f84039e48f41aea5e68ee28aabdc556) )
	ROM_LOAD( "main_7.256", 0x10000, 0x08000, CRC(dcf97517) SHA1(0a29696e0464c8878c499b1786a17080fd088a72) )

	ROM_REGION( 0x8000, "gfx2", 0 )
	ROM_LOAD( "main_4.764", 0x00000, 0x02000, CRC(cbcc6bfb) SHA1(5bafc934fef1f50d8c182c39d3a7ce795c89d175) )
	ROM_LOAD( "main_3.764", 0x02000, 0x02000, CRC(91162010) SHA1(3acc21e7074602b247f2f392eb181802092d2f21) )
	/* 2 roms missing - roms below taken from above set */
	ROM_LOAD( "6.764", 0x04000, 0x02000, CRC(e73ea4e3) SHA1(c9fd56461f6986d6bc170403d298fcc408a524e9) )
	ROM_LOAD( "7.764", 0x06000, 0x02000, CRC(7cc6d26b) SHA1(de33e8985affce7bd3ead89463117c9aaa93d5e4) )

	ROM_REGION( 0x0200, "prom", ROMREGION_DISPOSE )
	/* prom missing? */

	ROM_REGION( 0x20000, "oki", ROMREGION_ERASE00 )
	/* no oki on this pcb? */
ROM_END



ROM_START( cmv801 )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "prg512",   0x0000, 0x10000, CRC(2f6e3fe9) SHA1(c5ffa51478a0dc2d8ff6a0f286cfb461011bb55d) )

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "m5.256",   0x00000, 0x8000, CRC(19cc1d67) SHA1(47487f9362bfb36a32100ed772960628844462bf) )
	ROM_LOAD( "m6.256",   0x08000, 0x8000, CRC(63b3df4e) SHA1(9bacd23da598805ec18ec5ad15cab95d71eb9262) )
	ROM_LOAD( "m7.256",   0x10000, 0x8000, CRC(e39fff9c) SHA1(22fdc517fa478441622c6245cecb5728c5595757) )

	ROM_REGION( 0x8000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "m1.64",     0x0000, 0x2000, CRC(6dfcb188) SHA1(22430429c798954d9d979e62699b58feae7fdbf4) )
	ROM_LOAD( "m2.64",     0x2000, 0x2000, CRC(9678ead2) SHA1(e80aefa98b2363fe9e6b2415762695ace272e4d3) )
	ROM_LOAD( "m3.64",     0x4000, 0x2000, CRC(8607ffd9) SHA1(9bc94715554aa2473ae2ed249a47f29c7886b3dc) )
	ROM_LOAD( "m4.64",	   0x6000, 0x2000, CRC(c32367be) SHA1(ff217021b9c58e23b2226f8b0a7f5da966225715) )

	ROM_REGION( 0x200, "proms", 0 ) // pal
	ROM_LOAD( "prom2.287", 0x0000, 0x0100, CRC(0489b760) SHA1(78f8632b17a76335183c5c204cdec856988368b0) )
	ROM_LOAD( "prom3.287", 0x0100, 0x0100, CRC(21eb5b19) SHA1(9b8425bdb97f11f4855c998c7792c3291fd07470) )

	ROM_REGION( 0x100, "proms2", 0 ) // something else?
	ROM_LOAD( "prom1.287", 0x0000, 0x0100, CRC(50ec383b) SHA1(ae95b92bd3946b40134bcdc22708d5c6b0f4c23e) )
ROM_END

// this is probably different hardware..
ROM_START( cmaster )
	ROM_REGION( 0x10000, "main", 0 )
	ROM_LOAD( "8.bin",   0x00000, 0x10000, CRC(31a16d9f) SHA1(f007148449d66954b780f12a9f910968a4052482) )

	ROM_REGION( 0x40000, "user1", 0 )
	ROM_LOAD( "9.bin",   0x00000, 0x40000, CRC(92342276) SHA1(f9436752f2ec67cf873fd01c729c7c113dc18be0) ) // ?

	ROM_REGION( 0x18000, "gfx1", ROMREGION_DISPOSE )
	ROM_LOAD( "5.bin",   0x00000, 0x8000, CRC(28ff88cc) SHA1(46bc0407be857e8348159735b60cfb660f047a56) )
	ROM_LOAD( "6.bin",   0x08000, 0x8000, CRC(13582e74) SHA1(27e318542606b8e8d38250749ba996402d314abd) )
	ROM_LOAD( "7.bin",   0x10000, 0x8000, CRC(1edf1f1d) SHA1(558fa01f1efd7f6541047d3930bdce0974bae5b0))

	ROM_REGION( 0x20000, "gfx2", ROMREGION_DISPOSE )
	ROM_LOAD( "1.bin",     0x0000, 0x8000, CRC(71bdab69) SHA1(d2c594ed88d6368df15b623c48eecc1c219b839e) )
	ROM_LOAD( "2.bin",     0x8000, 0x8000, CRC(fccd48d7) SHA1(af564f5ef9ff5b6363897ce6bdf0b21123911fd4) )
	ROM_LOAD( "3.bin",     0x10000, 0x8000, CRC(dc77d04a) SHA1(d8656130cde54d4bb96307899f6d607867e49e6c) )
	ROM_LOAD( "4.bin",	   0x18000, 0x8000, CRC(0dbabaa2) SHA1(44235b19dac1c996e2166672b03f6e3888ecbefa) )

	ROM_REGION( 0x400, "proms", 0 )
	ROM_LOAD( "p1.bin", 0x0000, 0x0100, CRC(ac529f04) SHA1(5bc92e50c85bb23e609172cc15c430ddea7fdcb5) )
	ROM_LOAD( "p2.bin", 0x0100, 0x0100, CRC(3febce95) SHA1(c7c0fec0fb024ebf7d7365a09d28ba3d0037b0b4) )
	ROM_LOAD( "p3.bin", 0x0200, 0x0100, CRC(99dbdf19) SHA1(3680335406f63289f8d9a81b4cd163e4aa0c14d4) )
	ROM_LOAD( "p4.bin", 0x0300, 0x0100, CRC(72212427) SHA1(e87a91f28284313c706ebb8175a3586780636e31) )
ROM_END



static DRIVER_INIT(goldstar)
{
	int A;
	UINT8 *ROM = memory_region(machine, "main");

	for (A = 0;A < 0x10000;A++)
	{
		if ((A & 0x30) == 0)
			ROM[A] ^= 0x82;
		else
			ROM[A] ^= 0xcc;
	}
}

static DRIVER_INIT( chry10 )
{
	// probably has the same block swapping as chryigld below, but data swap is different
}

static DRIVER_INIT( chryigld )
{
	int A;
	UINT8 *ROM = memory_region(machine, "main");
	UINT8 *buffer;

	static UINT16 cherry_swaptables[32] = {
		/* to align with goldstar */
		0x0800, 0x4000, 0x2800, 0x5800,
		0x1800, 0x3000, 0x6800, 0x7000,
		0x0000, 0x4800, 0x2000, 0x5000,
		0x1000, 0x7800, 0x6000, 0x3800,
		/* bit below, I'm not sure, no match */
		0xc000, 0xc800, 0xd000, 0xd800,
		0xe000, 0xe800, 0xf000, 0xf800,
		0x8000, 0x8800, 0x9000, 0x9800,
		0xa000, 0xa800, 0xb000, 0xb800,
	};

	buffer = malloc(0x10000);

	// a data bitswap (this is correct for chryigld, not chry10)
	for (A = 0;A < 0x10000;A++)
	{
		UINT8 dat = ROM[A];
		dat =  BITSWAP8(dat,5,6,3,4,7,2,1,0);
		buffer[A] = dat;
	}

	// swap some 0x800 blocks around..
	for (A =0;A<32; A++)
	{
		memcpy(ROM+A*0x800,buffer+cherry_swaptables[A],0x800);
	}

	free(buffer);

	#if 0
	{
		FILE *fp;
		char filename[256];
		sprintf(filename,"decrypted_%s", machine->gamedrv->name);
		fp=fopen(filename, "w+b");
		if (fp)
		{
			fwrite(ROM, 0x10000, 1, fp);
			fclose(fp);
		}
	}
	#endif
}


GAME( 199?, goldstar, 0,        goldstar, goldstar, goldstar, ROT0, "IGS", "Golden Star", 0 )
GAME( 199?, goldstbl, goldstar, goldstbl, goldstar, 0,        ROT0, "IGS", "Golden Star (Blue version)", 0 )
GAME( 199?, moonlght, goldstar, moonlght, goldstar, 0,        ROT0, "bootleg", "Moon Light (bootleg of Golden Star)", 0 )
GAME( 199?, chry10,  goldstar,  chry10,  goldstar, chry10,  ROT0, "bootleg", "Cherry 10 (bootleg of Golden Star)", GAME_NOT_WORKING )
GAME( 199?, chryigld, goldstar, chry10,  goldstar, chryigld,  ROT0, "bootleg", "Cherry I Gold (bootleg of Golden Star)", GAME_NOT_WORKING )

// are these really dyna, or bootlegs?
GAME( 19??, ncb3,  goldstar,    ncb3, goldstar,  0, ROT0, "Dyna", "(New?) Cherry Bonus III", GAME_NOT_WORKING | GAME_NO_SOUND ) // set was labeled 'new cherry bonus 3' but there is no 'new' in the gfx roms
GAME( 19??, cb3,   goldstar,    ncb3, goldstar,  0, ROT0, "Dyna", "Cherry Bonus III", GAME_NOT_WORKING | GAME_NO_SOUND )

// cherry master hardware has a rather different mem map, but is basically the same
GAME( 198?, cmv801, 0,   cm, cmv801, 0, ROT0, "Corsica", "Cherry Master (Corsica, v8.01)", GAME_IMPERFECT_GRAPHICS | GAME_WRONG_COLORS | GAME_NOT_WORKING ) // says ED-96 where the manufacturer is on some games..
GAME( 1991, cmaster, 0,   cm, cmv801, 0, ROT0, "Dyna", "Cherry Master 91?", GAME_IMPERFECT_GRAPHICS | GAME_WRONG_COLORS | GAME_NOT_WORKING ) // different HW? closer to cherry master 2?
