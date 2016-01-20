// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Roberto Fresca
/*************************************************************************************************

  Forte Card
  1994, Fortex Ltd.

  Driver by Angelo Salese.
  Additional work by Roberto Fresca & Rob Ragon.

  Notes:
  - NMI mask and vblank bit are made thru SW usage of the I register (that is unused
    if the z80 is in IM 1).


  TODO:
  - fortecar requires a proper EEPROM dump, doesn't start due of it.
  - fortecrd has non-default data in the default EEPROM.

  English set: bp 512 do pc=53e
  Spanish set: bp 512 do pc=562

-------------------------------------------------------------------------------------------------

  The Fortex company manufactured its first game 'Forte Card' in 1994. It soon became a favorite
  with the Bulgarian players. Improved versions were developed and were sold in Russia, Austria,
  Brazil, Argentina and the Balkan peninsular.

  In 1995, Fortex, first of all Bulgarian games manufacturers, exibited at the Plovdiv Fair a com-
  puter based version of Forte Card. The following years were times of hard teamwork, which resul-
  ted in a variety of products: centralized cash desk, on-line jackpot and a network control and
  management system for game centers.

-------------------------------------------------------------------------------------------------

  Forte Card (POKER GAME)

  CPU  SGS Z8400AB1 (Z80ACPU)
  VIDEO CM607P
  PIO M5L8255AP
  snd ay38910/P (microchip)
  + 8251A

  RAM 6116 + GOLDSTAR GM76C256ALL-70
  dip 1X8

-------------------------------------------------------------------------------------------------

  Forte Card - By Fortex Ltd.
  (Spanish Version)

  Sound    AY 3-8910 @1.5Mhz
  USART    8251
  uProc    Z80 @3MHz
  Video    6845 @1.5MHz
  PIO      8255
  NVRam    6116
  RAM      84256 (Video, I think)
  EEPROM   NM93CS56N - (MICROWIRETM Bus Interface)
           256-/1024-/2048-/4096-Bit Serial EEPROM
           with Data Protect and Sequential Read.
  RTC      V3021
  Dip 1x8

  Xtal: 12 MHz.


  NOTE:

  The CM607P IC is an exact clone of Motorola's MC6845P CRT Controller circuit
  from MC6800 family. Used to perform the interface to raster scan CRT displays.
  Made in Bulgaria in the Pravetz factory, where Pravetz-8 and Pravetz-16 compu-
  ters were made in 1980's.

  FULL equivalent to MC6845P, UM6845R, EF6845P, HD6845P, etc.

  The ST93CS56 and ST93CS57 are 2K bit Electrically Erasable Programmable Memory (EEPROM)
  fabricated with SGS-THOMSON's High Endurance Single Polysilicon CMOS technology. The memory
  is accessed through a serial input D and output Q. The 2K bit memory is organized as 128 x 16 bit
  words.The memory is accessed by a set of instructions which include Read, Write, Page Write, Write
  All and instructions used to set the memory protection. A Read instruction loads the address of the
  first word to be read into an internal address pointer.


-------------------------------------------------------------------------------------------------

  Game Notes...


  There are 3 keys for service modes:

  1) The Owner Key.
  2) The Rental Key.
  3) The Credits Key.


  To enter credits through an operator:

  1) Turn ON the Credits Key.
  2) Press RED button to add x1000, or BLACK to add x100
  3) Turn OFF the Credit Key.


-------------------------------------------------------------------------------------------------

  From the manual (sic)...

  Initialization of the Forte Card circuit board
  (Init machine)

  1) Open the door of the machine, switch on the game and wait until the message
     'Permanent RAM test failed' appears.
  2) Turn the Main Control and hold it in this position.
  3) Enter the serial number of the circuit board with the eight keys.
  4) Press STAR key and wait until the message 'Machine initialization completed' appears.
  5) Release the Main Control, switch off the game and close the door.
  6) Switch on the game and wait until the demonstration displays appear.
  7) Turn the Main Control, adjust the time. This is the last step of the initialization procedure.


-------------------------------------------------------------------------------------------------

  Edge Connector / Pinouts....

  -------------------+--+------------------
     Components Side |PN| Solder Side
  -------------------+--+------------------
                 GND |01| GND
                 GND |02| GND
                 +5V |03| +5V
                 +5V |04| +5V
                +12V |05| +12V
      CUR. LOOP IN + |06| CUR. LOOP OUT +
      CUR. LOOP IN - |07| CUR. LOOP OUT -
         DOOR SWITCH |08| COUNTER IN
             PAYMENT |09| COUNTER OUT
                COIN |10| COUNTER KEY IN
        HOPPER COUNT |11| HOPPER DRIVE
         BANKNOTE IN |12| RESERVED
              CREDIT |13| RESERVED
             MANAGER |14| RESERVED
              PAGE 1 |15| RESERVE OUT
          RED BUTTON |16| RED LAMP
        BLACK BUTTON |17| BLACK LAMP
       HOLD 1 BUTTON |18| HOLD 1 LAMP
       HOLD 2 BUTTON |19| HOLD 2 LAMP
       HOLD 3 BUTTON |20| HOLD 3 LAMP
       HOLD 4 BUTTON |21| HOLD 4 LAMP
       HOLD 5 BUTTON |22| HOLD 5 LAMP
        START BUTTON |23| START LAMP
          GAIN SOUND |24| SPEAKER OUT
           GND SOUND |25| SP.OUT R/INTENSITY
  COMP.SYNC / H SYNC |26| RED
              V SYNC |27| GREEN
         GND MONITOR |28| BLUE


  Note: MANAGER and PAGE 1, are "Owner" and "Rental" modes.


-------------------------------------------------------------------------------------------------

  Serial EEPROM
  -------------

  Forte Card (Ver 110, Spanish)

  0x80 x 16bit words

  Word   Bytes   Description                         Mode    Notes
  -----+-------+-----------------------------------+-------+--------
  0x00   00-01   Total Entradas (low)                Owner
  0x01   02-03   Total Entradas (med)                Owner
  0x02   04-05   Total Entradas (high)               Owner
  0x03   06-07   Total Salidas (low)                 Owner
  0x04   08-09   Total Salidas (med)                 Owner
  0x05   0A-0B   Total Salidas (high)                Owner
  0x06   0C-0D   Total Juegos (low)                  Owner
  0x07   0E-0F   Total Juegos (med)                  Owner
  0x08   10-11   Total Juegos (high)                 Owner
  0x09   12-13   Total Ganancias (low)               Owner
  0x0A   14-15   Total Ganancias (med)               Owner
  0x0B   16-17   Total Ganancias (high)              Owner
  0x0C   18-19   Total Ganancias C/Apuestas (low)    Owner
  0x0D   1A-1B   Total Ganancias C/Apuestas (med)    Owner
  0x0E   1C-1D   Total Ganancias C/Apuestas (high)   Owner
  0x0F   1E-1F   Total Apuestas (low)                Owner

  0x10   20-21   Total Apuestas (med)                Owner
  0x11   22-23   Total Apuestas (high)               Owner
  0x12   24-25   Valor Moneda / Nivel Ganancia
  0x13   26-27   Min Bet / Max Bet step (0-B) - (Clase C, bit1 active??)
  0x14   28-29   Jokers (1-2) / ????
  0x15   2A-2B   Registro Dinero Minimo (low)
  0x16   2C-2D   Registro Dinero Minimo (high)
  0x17   2E-2F   Factor de Calculo (1-10-100)
  0x18   30-31   Serial Number (low, 4 digits)
  0x19   32-33   Serial Number (high, 0970)
  0x1A   34-35   Unknown (6F2B)
  0x1B   36-37   Unknown (0341)
  0x1C   38-39   Unknown (980C)
  0x1D   3A-3B   Balance (low)                       Owner
  0x1E   3C-3D   Balance (med)                       Owner
  0x1F   3E-3F   Balance (high)                      Owner

  0x20   40-41   Unknown (0000)
  0x21   42-43   Unknown (0000)
  0x22   44-45   Unknown (0000)
  0x23   46-47   Unknown (0000)
  0x24   48-49   Unknown (0000)
  0x25   4A-4B   Unknown (0000)
  0x26   4C-4D   Unknown (0000)
  0x27   4E-4F   Unknown (0000)
  0x28   50-51   Unknown (0000)
  0x29   52-53   Unknown (0000)
  0x2A   54-55   Unknown (0000)
  0x2B   56-57   Unknown (0000)
  0x2C   58-59   Unknown (0000)
  0x2D   5A-5B   Unknown (0000)
  0x2E   5C-5D   Unknown (0000)
  0x2F   5E-5F   Unknown (0000)

  0x30   60-61   Unknown (0000)
  0x31   62-63   Unknown (0000)
  0x32   64-65   Unknown (0000)
  0x33   66-67   Unknown (0000)
  0x34   68-69   Unknown (0000)
  0x35   6A-6B   Unknown (0000)
  0x36   6C-6D   Unknown (0000)
  0x37   6E-6F   Unknown (0000)
  0x38   70-71   Unknown (0000)
  0x39   72-73   Unknown (0000)
  0x3A   74-75   Unknown (0000)
  0x3B   76-77   Unknown (0000)
  0x3C   78-79   Unknown (0000)
  0x3D   7A-7B   Unknown (0000)
  0x3E   7C-7D   Unknown (0000)
  0x3F   7E-7F   Unknown (0000)

  0x40   80-81   Unknown (0000)
  0x41   82-83   Unknown (0000)
  0x42   84-85   Unknown (0000)
  0x43   86-87   Unknown (1187)
  0x44   88-89   Unknown (B042)
  0x45   8A-8B   Unknown (0000)
  0x46   8C-8D   Unknown (0000)
  0x47   8E-8F   Unknown (0000)
  0x48   90-91   Unknown (0000)
  0x49   92-93   Unknown (0000)
  0x4A   94-95   Unknown (0000)
  0x4B   96-97   Unknown (0000)
  0x4C   98-99   Unknown (0000)
  0x4D   9A-9B   Unknown (0000)
  0x4E   9C-9D   Unknown (0000)
  0x4F   9E-9F   Unknown (0000)

  0x50   A0-A1   Unknown (0000)
  0x51   A2-A3   Unknown (0000)
  0x52   A4-A5   Unknown (0000)
  0x53   A6-A7   Unknown (0000)
  0x54   A8-A9   Unknown (0000)
  0x55   AA-AB   Unknown (0000)
  0x56   AC-AD   Unknown (0000)
  0x57   AE-AF   Unknown (0000)
  0x58   B0-B1   Unknown (0000)
  0x59   B2-B3   Unknown (0000)
  0x5A   B4-B5   Unknown (0000)
  0x5B   B6-B7   Unknown (0000)
  0x5C   B8-B9   Unknown (0000)
  0x5D   BA-BB   Unknown (0000)
  0x5E   BC-BD   Unknown (0000)
  0x5F   BE-BF   Unknown (0000)

  0x60   C0-C1   Unknown (0000)
  0x61   C2-C3   Unknown (0000)
  0x62   C4-C5   Unknown (0000)
  0x63   C6-C7   Unknown (0000)
  0x64   C8-C9   Unknown (0000)
  0x65   CA-CB   Unknown (0000)
  0x66   CC-CD   Unknown (0000)
  0x67   CE-CF   Unknown (0000)
  0x68   D0-D1   Unknown (0000)
  0x69   D2-D3   Unknown (0000)
  0x6A   D4-D5   Unknown (0000)
  0x6B   D6-D7   Unknown (1187)
  0x6C   D8-D9   Unknown (B042)
  0x6D   DA-DB   Global Entradas (low)               Rental
  0x6E   DC-DD   Global Entradas (high)              Rental
  0x6F   DE-DF   Unknown (1187)

  0x70   E0-E1   Unknown (B042)
  0x71   E2-E3   Global Salidas (low)                Rental
  0x72   E4-E5   Global Salidas (high)               Rental
  0x73   E6-E7   Unknown (172E)
  0x74   E8-E9   Unknown (980B)
  0x75   EA-EB   Total Entradas Anterior (low)
  0x76   EC-ED   Total Entradas Anterior (high)
  0x77   EE-EF   Unknown (172E)
  0x78   F0-F1   Unknown (980B)
  0x79   F2-F3   Total Salidas Anterior (low)
  0x7A   F4-F5   Total Salidas Anterior (high)
  0x7B   F6-F7   Unknown (2114)
  0x7C   F8-F9   ??? / Valor Billete
  0x7D   FA-FB   Unknown (0000)
  0x7E   FC-FD   Unknown (0000)
  0x7F   FE-FF   Checksum


**************************************************************************************************/


#define MASTER_CLOCK    XTAL_12MHz
#define CPU_CLOCK       (MASTER_CLOCK/4)
#define CRTC_CLOCK      (MASTER_CLOCK/8)
#define AY_CLOCK        (MASTER_CLOCK/8)

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "sound/ay8910.h"
#include "machine/i8255.h"
#include "machine/v3021.h"
#include "video/mc6845.h"
#include "machine/nvram.h"
#include "video/resnet.h"

#include "fortecrd.lh"


class fortecar_state : public driver_device
{
public:
	fortecar_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_vram(*this, "vram"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_vram;
	DECLARE_WRITE8_MEMBER(ppi0_portc_w);
	DECLARE_READ8_MEMBER(ppi0_portc_r);
	DECLARE_WRITE8_MEMBER(ayporta_w);
	DECLARE_WRITE8_MEMBER(ayportb_w);
	DECLARE_DRIVER_INIT(fortecar);
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(fortecar);
	UINT32 screen_update_fortecar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


void fortecar_state::video_start()
{
}

UINT32 fortecar_state::screen_update_fortecar(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y,count;
	count = 0;

	for (y=0;y<0x1e;y++)
	{
		for(x=0;x<0x4b;x++)
		{
			int tile,color,bpp;

			tile = (m_vram[(count*4)+1] | (m_vram[(count*4)+2]<<8)) & 0xfff;
			color = m_vram[(count*4)+3] & 0x1f;
			bpp = (m_vram[(count*4)+3] & 0x20) >> 5;

			if(bpp)
				color&=0x3;

			m_gfxdecode->gfx(bpp)->opaque(bitmap,cliprect,tile,color,0,0,x*8,y*8);
			count++;

		}
	}

	return 0;
}

PALETTE_INIT_MEMBER(fortecar_state, fortecar)
{
	const UINT8 *color_prom = memregion("proms")->base();
/* Video resistors...

O1 (LS374) R1K  RED
O2 (LS374) R510 RED
O3 (LS374) R220 RED
O4 (LS374) R1K  GREEN
O5 (LS374) R510 GREEN
O6 (LS374) R220 GREEN
O7 (LS374) R510 BLUE
O8 (LS374) R220 BLUE

R = 82 Ohms Pull Down.
*/
	int i;
	static const int resistances_rg[3] = { 1000, 510, 220 };
	static const int resistances_b [2] = { 510, 220 };
	double weights_r[3], weights_g[3], weights_b[2];

	compute_resistor_weights(0, 255,    -1.0,
			3,  resistances_rg, weights_r,  82, 0,
			3,  resistances_rg, weights_g,  82, 0,
			2,  resistances_b,  weights_b,  82, 0);

	for (i = 0; i < 512; i++)
	{
		int bit0, bit1, bit2, r, g, b;

		/* red component */
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		r = combine_3_weights(weights_r, bit0, bit1, bit2);

		/* green component */
		bit0 = (color_prom[i] >> 3) & 0x01;
		bit1 = (color_prom[i] >> 4) & 0x01;
		bit2 = (color_prom[i] >> 5) & 0x01;
		g = combine_3_weights(weights_g, bit0, bit1, bit2);

		/* blue component */
		bit0 = (color_prom[i] >> 6) & 0x01;
		bit1 = (color_prom[i] >> 7) & 0x01;
		b = combine_2_weights(weights_b, bit0, bit1);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


WRITE8_MEMBER(fortecar_state::ppi0_portc_w)
{
/*
NM93CS56N Serial EEPROM

CS   PPI_PC0
CK   PPI_PC1
DIN  PPI_PC2
DOUT PPI_PC4
*/
	m_eeprom->di_write((data & 0x04) >> 2);
	m_eeprom->cs_write((data & 0x01) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->clk_write((data & 0x02) ? ASSERT_LINE : CLEAR_LINE);
}

READ8_MEMBER(fortecar_state::ppi0_portc_r)
{
//  popmessage("%s",machine().describe_context());
	return ((m_eeprom->do_read()<<4) & 0x10);
}

WRITE8_MEMBER(fortecar_state::ayporta_w)
{
/*  System Lamps...

    - bits -
    7654 3210
    ---- ---x   START lamp.
    ---- --x-   HOLD5 lamp.
    ---- -x--   HOLD4 lamp.
    ---- x---   HOLD3 lamp.
    ---x ----   HOLD2 lamp.
    --x- ----   HOLD1 lamp.
    -x-- ----   BLACK lamp.
    x--- ----   RED/BET lamp.

    Also used for POST?...

    0x01 (start): RAM test d000-d7ff
    0x02 (hold5): VRAM test d800-ffff
    0x04 (hold4): Video SYNC test
    0x08 (hold3): ROM check
    0x10 (hold2): NVRAM check
    0x20 (hold1): IRQ test
    0x40 (black): Stack RAM check
*/
	int i;

	for(i = 0; i < 8; i++)
		output().set_lamp_value(i, (data >> i) & 1);
}


WRITE8_MEMBER(fortecar_state::ayportb_w)
{
/*

There is a RC to 7705's Reset.
Bit7 of port B is a watchdog.

A square wave is fed to through resistor R to capacitor C, with a constant charge/discharge
time relative to the value of resistor R and value of capacitor C. If the square wave halts,
capacitor C will charge beyond the hysteresis threshhold of the TL7705 (leg 6), causing it to
trigger a reset.

Seems to work properly, but must be checked closely...

*/
	if (((data >> 7) & 0x01) == 0)      /* check for bit7 */
	{
		machine().watchdog_reset();
	}

//  logerror("AY port B write %02x\n",data);
}

static ADDRESS_MAP_START( fortecar_map, AS_PROGRAM, 8, fortecar_state )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xc7ff) AM_ROM
	AM_RANGE(0xd000, 0xd7ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0xd800, 0xffff) AM_RAM AM_SHARE("vram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( fortecar_ports, AS_IO, 8, fortecar_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x20, 0x20) AM_DEVWRITE("crtc", mc6845_device, address_w)  // pc=444
	AM_RANGE(0x21, 0x21) AM_DEVWRITE("crtc", mc6845_device, register_w)
	AM_RANGE(0x40, 0x40) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE(0x40, 0x41) AM_DEVWRITE("aysnd", ay8910_device, address_data_w)
	AM_RANGE(0x60, 0x63) AM_DEVREADWRITE("fcppi0", i8255_device, read, write)//M5L8255AP
//  AM_RANGE(0x80, 0x81) //8251A UART
	AM_RANGE(0xa0, 0xa0) AM_DEVREADWRITE("rtc", v3021_device, read, write)
	AM_RANGE(0xa1, 0xa1) AM_READ_PORT("DSW")
ADDRESS_MAP_END
/*

CRTC REGISTER: 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f
CRTC DATA    : 5f 4b 50 08 21 05 1e 1f 00 07 20 00 06 00 00 00

Error messages:

"FALSA PRUEBA NVR"              (NVRAM new, no serial EEPROM connected)
"FALLO EN NVR"                  (NVRAM ok, no serial EEPROM connected)
"FALSA PRUEBA NVRAM PERMANENTE" (NVRAM new, serial EEPROM connected)

*/

static INPUT_PORTS_START( fortecar )
	PORT_START("DSW")   /* 8bit */
	PORT_DIPNAME( 0x01, 0x01, "DSW-1" )             PORT_DIPLOCATION("DSW:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Attract Mode" )      PORT_DIPLOCATION("DSW:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW-3" )             PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW-4" )             PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW-5" )             PORT_DIPLOCATION("DSW:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW-6" )             PORT_DIPLOCATION("DSW:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW-7" )             PORT_DIPLOCATION("DSW:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW-8" )             PORT_DIPLOCATION("DSW:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("INPUT") /* 8bit */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )  PORT_NAME("Red / Bet")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )   PORT_NAME("Black")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )

	PORT_START("SYSTEM")    /* 8bit */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )      PORT_NAME("Rear Door")   PORT_CODE(KEYCODE_D)  PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Payout")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )         PORT_NAME("Coin In")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )       /* to trace */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )         PORT_NAME("Note In")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )      PORT_NAME("Credits Key") PORT_CODE(KEYCODE_Q)  PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )      PORT_NAME("Owner Key")   PORT_CODE(KEYCODE_0)  PORT_TOGGLE
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE1 )      PORT_NAME("Rental Key")  PORT_CODE(KEYCODE_9)  PORT_TOGGLE
INPUT_PORTS_END


static const gfx_layout tiles8x8_layout_3bpp =
{
	8, 8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(2,3)+4, RGN_FRAC(1,3)+4, RGN_FRAC(0,3)+4 },
	{ 8, 9, 10, 11, 0, 1, 2, 3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout tiles8x8_layout_6bpp =
{
	8, 8,
	RGN_FRAC(1,3),
	6,
	{ RGN_FRAC(2,3)+0, RGN_FRAC(1,3)+0, RGN_FRAC(0,3)+0, RGN_FRAC(2,3)+4, RGN_FRAC(1,3)+4, RGN_FRAC(0,3)+4 },
	{ 8, 9, 10, 11, 0, 1, 2, 3 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};


static GFXDECODE_START( fortecar )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout_3bpp, 0x000, 0x20 )
	GFXDECODE_ENTRY( "gfx1", 0, tiles8x8_layout_6bpp, 0x100, 0x04 )
GFXDECODE_END



void fortecar_state::machine_reset()
{
	int i;

	/* apparently there's a random fill in there (checked thru trojan TODO: extract proper algorythm) */
	for(i = 0; i < m_vram.bytes(); i++)
		m_vram[i] = machine().rand();
}


static MACHINE_CONFIG_START( fortecar, fortecar_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, CPU_CLOCK)      /* 3 MHz, measured */
	MCFG_CPU_PROGRAM_MAP(fortecar_map)
	MCFG_CPU_IO_MAP(fortecar_ports)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", fortecar_state,  nmi_line_pulse)
	MCFG_WATCHDOG_TIME_INIT(attotime::from_msec(200))   /* guess */

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(640, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 600-1, 0, 240-1)    /* driven by CRTC */
	MCFG_SCREEN_UPDATE_DRIVER(fortecar_state, screen_update_fortecar)
	MCFG_SCREEN_PALETTE("palette")


	MCFG_EEPROM_SERIAL_93C56_ADD("eeprom")
	MCFG_EEPROM_SERIAL_DEFAULT_VALUE(0)

	MCFG_DEVICE_ADD("fcppi0", I8255A, 0)
	/*  Init with 0x9a... A, B and high C as input
	 Serial Eprom connected to Port C */
	MCFG_I8255_IN_PORTA_CB(IOPORT("SYSTEM"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("INPUT"))
	MCFG_I8255_IN_PORTC_CB(READ8(fortecar_state, ppi0_portc_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(fortecar_state, ppi0_portc_w))

	MCFG_V3021_ADD("rtc")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", fortecar)
	MCFG_PALETTE_ADD("palette", 0x200)
	MCFG_PALETTE_INIT_OWNER(fortecar_state, fortecar)

	MCFG_MC6845_ADD("crtc", MC6845, "screen", CRTC_CLOCK)    /* 1.5 MHz, measured */
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, AY_CLOCK)   /* 1.5 MHz, measured */
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(fortecar_state, ayporta_w))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(fortecar_state, ayportb_w))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


ROM_START( fortecar )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "fortecar.u7", 0x00000, 0x010000, CRC(2a4b3429) SHA1(8fa630dac949e758678a1a36b05b3412abe8ae16)  )

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "fortecar.u38", 0x00000, 0x10000, CRC(c2090690) SHA1(f0aa8935b90a2ab6043555ece69f926372246648) )
	ROM_LOAD( "fortecar.u39", 0x10000, 0x10000, CRC(fc3ddf4f) SHA1(4a95b24c4edb67f6d59f795f86dfbd12899e01b0) )
	ROM_LOAD( "fortecar.u40", 0x20000, 0x10000, CRC(9693bb83) SHA1(e3e3bc750c89a1edd1072ce3890b2ce498dec633) )

	/* took from the Spanish version, these are likely to be identical anyway */
	ROM_REGION( 0x0800, "nvram", 0 )    /* default NVRAM */
	ROM_LOAD( "fortecrd_nvram.u6", 0x0000, 0x0800, BAD_DUMP CRC(7d3e7eb5) SHA1(788fe7adc381bcc6eaefed33f5aa1081340608a0) )

	ROM_REGION( 0x0100, "eeprom", 0 )   /* default serial EEPROM */
	ROM_LOAD16_WORD_SWAP( "forte_card_93cs56_serial_12345678.u13", 0x0000, 0x0100, BAD_DUMP CRC(2fc5961d) SHA1(f958c8b2b4e48cc6e5a607a6751acde5592bd27f) )

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD( "forte_card_82s147.u47", 0x0000, 0x0200, BAD_DUMP CRC(7e631818) SHA1(ac08b0de30260278af3a1c5dee5810d4304cb9ca) )
ROM_END

ROM_START( fortecrd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "forte_card.u7", 0x00000, 0x010000, CRC(79fc6dd3) SHA1(5454f2ee12b62d573b61c54e48398f43332b000e) )

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "forte_card.u38", 0x00000, 0x10000, CRC(258fb7bf) SHA1(cd75001fe40836b2dc229caddfc38f6076df7a79) )
	ROM_LOAD( "forte_card.u39", 0x10000, 0x10000, CRC(3d9c478e) SHA1(eb86115d1c36038f2c80cd116f5aeddd94036424) )
	ROM_LOAD( "forte_card.u40", 0x20000, 0x10000, CRC(9693bb83) SHA1(e3e3bc750c89a1edd1072ce3890b2ce498dec633) )

	ROM_REGION( 0x0800, "nvram", 0 )    /* default NVRAM */
	ROM_LOAD( "fortecrd_nvram.u6", 0x0000, 0x0800, CRC(7d3e7eb5) SHA1(788fe7adc381bcc6eaefed33f5aa1081340608a0) )

	ROM_REGION( 0x0100, "eeprom", 0 )   /* default serial EEPROM */
	ROM_LOAD16_WORD_SWAP( "forte_card_93cs56_serial_12345678.u13", 0x0000, 0x0100, CRC(2fc5961d) SHA1(f958c8b2b4e48cc6e5a607a6751acde5592bd27f) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "forte_card_82s147.u47", 0x0000, 0x0200, CRC(7e631818) SHA1(ac08b0de30260278af3a1c5dee5810d4304cb9ca) )
ROM_END


DRIVER_INIT_MEMBER(fortecar_state,fortecar)
{
	// ...
}


/*     YEAR  NAME      PARENT    MACHINE   INPUT     STATE           INIT      ROT    COMPANY       FULLNAME                        FLAGS             LAYOUT */
GAMEL( 1994, fortecar, 0,        fortecar, fortecar, fortecar_state, fortecar, ROT0, "Fortex Ltd", "Forte Card (Ver 103, English)", MACHINE_NOT_WORKING, layout_fortecrd )
GAMEL( 1994, fortecrd, fortecar, fortecar, fortecar, fortecar_state, fortecar, ROT0, "Fortex Ltd", "Forte Card (Ver 110, Spanish)", 0,                layout_fortecrd )
