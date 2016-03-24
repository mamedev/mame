// license:BSD-3-Clause
// copyright-holders:Roberto Fresca, Angelo Salese
/**********************************************************************************

  AMA-8000-1 / AMA-8000-2 Multi Game System.
  Amatic Trading GmbH.

  Encrypted gambling hardware based on a custom CPU.

  Driver by Roberto Fresca & Angelo Salese

***********************************************************************************

  Hardware Notes
  --------------

  ------------------------------------------------
    Board #1 (Super Stars) AMA-8000-1
  ------------------------------------------------

  1x 40-pin custom CPU labeled:

     0288
     8012 (last digit is hard to read)
     11.12.96

  1x Unknown 40-pins IC (maybe 6845).
  1x Altera EPM5130LC (84-pins).
  1x KS82C55A (2x PPI).
  1x Unknown 40-pins IC (maybe another PPI).

  1x Dallas DS1236-10 (micro manager).
  1x Push button.

  1x Unknown 24-pin IC labeled SM64.
  1x Unknown 8-pin IC labeled SM65 (looks like a DAC).
  1x Unknown 8-pin IC no labeled (looks like a DAC).
  1x MC14538BCL (Dual precision monostable multivibrator).
  1x TDA2003 Audio Amp.
  1x Pot.

  1x HY6264ALP-10 (RAM).
  1x HY62256ALP-10 (RAM).

  1x 1mb ROM, near CPU.
  2x 27C512 ROMs.
  1x N82S147AN bipolar PROM.

  1x Xtal 16 MHz.
  1x DIP switches bank (x8).
  1x Battery.

  1x 2x17 male connector (like IDE ones).
  1x 2x8 contacts edge connector.
  1x 2x22 contacts edge connector.


  ------------------------------------------------
     Multi-Game I v2.4 (AMA-8000-2)
  ------------------------------------------------

  1x 40-pin custom CPU labeled:

     Amatic Trading GMBH
     Lfnd. Nr. 1000
     Type:     801 I
     Datum:    12.02.96

  1x GoldStar 6845S.
  1x Altera EPM5130LC (84-pins).
  3x PPI 8255AC-2.

  1x Dallas DS1236-10 (micro manager).
  1x Push button.

  1x Unknown 24-pin IC labeled K-666 9330.
  1x Unknown 8-pin IC labeled K-664 9432 (looks like a DAC).
  1x LM358P (8-pin).
  1x MC14538BCL (Dual precision monostable multivibrator).
  1x TDA2003 Audio Amp.
  1x Pot.

  1x KM6264BL-7 (RAM).
  1x HY62256A-10 (RAM).

  1x 27C2001 ROM labeled MGI V GER 3.9/I/8201.
  1x 27C4000 ROM labeled Multi 2.4 ZG1.
  1x 27C4001 ROM labeled Multi 2.4 ZG2.
  1x 27C4001 ROM labeled Multi 2.4 ZG3.
  2x N82S147AN bipolar PROMs.

  1x Xtal 16 MHz.
  1x DIP switches bank (x8).
  1x Battery.

  1x 2x17 male connector (like IDE ones).
  1x 2x36 contacts edge connector.


  PCB Layout:
              ________________________________________________________________________
             | |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  | |
   __________| |  |  |  |  |  |  |  |  |36x2 edge connector |  |  |  |  |  |  |  |  | |__________
  |    ____ ____                                                                                 |
  |   |  :::::  |                                                                                |
  |   |_________|                DIP1                                                            |
  | __________   __________   __________                                             __________  |
  ||ULN 2803A | |ULN 2803A | | 12345678 |                                           |ULN 2803A | |
  ||__________| |__________| |__________|                                           |__________| |
  |                                                                                              |
  |    __________________________    __________________________    __________________________    |
  |   |        NEC JAPAN         |  |        NEC JAPAN         |  |        NEC JAPAN         |   |
  |   |        D8255AC_2         |  |        D8255AC_2         |  |        D8255AC_2         |   |
  |   |        9150xD006         |  |        9150xD006         |  |        9150xD006         |   |
  |   |                          |  |                          |  |                          |   |
  |   |__________________________|  |__________________________|  |__________________________|   |
  |                                                                                              |
  |   _________________________                                                                  |
  |  |AMATIC TRADING GMBH      |                  __________    _______________                  |
  |  |Lfnd. Nr. 1000           |                 |   XTAL   |  |     K_666     |                 |
  |  |Type: 80 / I             |                 |  16 Mhz  |  |     9330      |                 |
  |  |Datum: 12.02.96          |                 |__________|  |               |                 |
  |  |_________________________|                               |_______________|                 |
  |                                                                                              |
  |  ? ____      ___________________       __________________    __                              |
  |   /    \    |MGI V GER          |     |                  |  |H |                             |
  |  | Batt |   |3.9 / I / 8201     |     |      ALTERA      |  |  |                             |
  |  | ery  |   |M27C2001           |     |                  |  |__|                             |
  |   \____/    |___________________|     |    EPM5130LC     |                                   |
  |                                       |      I9542       |                                   |
  |      __        _________________      |                  |   __                 ______       |
  |     |  |      |HY62256A         |     |                  |  |G |               /______\      |
  |     |  |      |LP_10            |     |                  |  |  |              |DALLAS  |     |
  |     |A |      |9506C  Korea     |     |                  |  |__|              |DS1994_F|     |
  |     |  |      |_________________|     |                  |                    |_______5|     |
  |     |  |                              |__________________|                     \______/      |
  |     |__|                                                                                     |
  |       _______________________            __    __    __    __    __    __                    |
  |      |   :::::::::::::::::   |          |  |  |  |  |  |  |  |  |  |  |  |                   |
  |      |___________ ___________|          |  |  |  |  |  |  |  |  |  |  |  |    ____________   |
  |                                         |C |  |B |  |B |  |B |  |B |  |B |   | GD74HCT273 |  |
  |          __________    _______          |  |  |  |  |  |  |  |  |  |  |  |   |____________|  |
  |         |MC14538BCP|  |       |         |  |  |  |  |  |  |  |  |  |  |  |    ____________   |
  |         |__________|  |SEC    |   __    |__|  |__|  |__|  |__|  |__|  |__|   | N82S147AN  |  |
  |                       |  KOREA|  |  |                                        |____________|  |
  |                       |       |  |  |     _______    _______    _______                      |
  |   __   __   __   __   |   506Y|  |E |    |       |  |       |  |       |                     |
  |  |  | |  | |  | |  |  |       |  |  |    |MULTI  |  |MULTI  |  |MULTI  |                     |
  |  |D | |D | |D | |D |  |KM6264B|  |  |    |    2.4|  |    2.4|  |    2.4|                     |
  |  |  | |  | |  | |  |  |L_7    |  |__|    |       |  |       |  |       |      ____________   |
  |  |  | |  | |  | |  |  |       |          |  ZG1  |  |  ZG2  |  |  ZG3  |     | GD74HCT273 |  |
  |  |__| |__| |__| |__|  |_______|          |       |  |       |  |       |     |____________|  |
  |                                          |       |  |       |  |       |      ____________   |
  |  ________________________    __   __     |27C4000|  |27C4001|  |27C4001|     | N82S147AN  |  |
  | |        GOLDSTAR        |  |  | |  |    |       |  |       |  |       |     |____________|  |
  | |        GM68B45S        |  |  | |  |    |       |  |       |  |       |      _________      |
  | |        9512            |  |F | |E |    |_______|  |_______|  |_______|     |GD74HC174|     |
  | |                        |  |  | |  |                                        |_________|     |
  | |________________________|  |  | |  |                                      _________         |
  |                             |  | |  |                                     |GD74HC174|        |
  |                             |__| |__|        AMATIC AMA_8000_2            |_________|        |
  |______________________________________________________________________________________________|

  A = Dallas / DS1236_10 / 9443A5
  B = MALAYSIA 114CS / SN74LS194AN
  C = MALAYSIA 544CS / SN74LS194AN
  D = GS 9447 / GD74LS157
  E = 53A9TKK / SN74HC374N
  F = GS 9504 / GD74HC244
  G = LM358P
  H = K_664 / 9432


  DIP1:
   ___________________
  | ON                |
  |  _______________  |
  | |_|_|_|_|_|_|_|_| |
  | |#|#|#|#|#|#|#|#| |
  | |_______________| |
  |  1 2 3 4 5 6 7 8  |
  |___________________|


  ------------------------------------------------
     Multi-Game III v3.5 (AMA-8000-2)
  ------------------------------------------------

  1x 40-pin custom CPU labeled:

     Amatic
     Lfnd. Nr. 99/5070 467
     Type:     80(1?) I
     Datum:    10.01.00

  1x F68B45P.
  1x Altera EPM5130LC (84-pins).
  3x PPI NEC D71055C.

  1x Dallas DS1236-5 (micro manager).
  1x Push button.

  1x Yamaha YM3812.
  1x Yamaha Y3014B (DAC).
  1x LM358M (8-pin).
  1x MC14538 (Dual precision monostable multivibrator).
  1x TDA2003 (Audio Amp, Heatsinked).
  1x Pot.

  1x HY6264A (RAM).
  1x KM62256 (RAM).

  1x 27C2000 ROM labeled 'MG III VGer 3.5/I/8205'.
  1x 27C4000 ROM labeled 'MG III 51 ZG1'.
  1x 27C040 ROM labeled 'MG III 51 ZG2'.
  1x 27C040 ROM labeled 'MG III 51 ZG3'.
  1x 27C1024 labeled 'V'

  1x Xtal 16 MHz.
  1x DIP switches bank (x8).
  1x Battery.

  2x 2x17 male connector (like IDE ones).
  1x 2x36 contacts edge connector.


***********************************************************************************


  Memory Map
  ----------

  0000-7FFF   ROM Space.
  8000-9FFF   NVRAM.
  A000-AFFF   Video RAM.
  B000-BFFF   RAM.
  C000-FFFF   ROM Banking

  I/O

  00-03       PPI 8255 0
  20-23       PPI 8255 1
  40-41       YM3812 Sound device.
  60-60       MC6845 CRTC Address.
  61-61       MC6845 CRTC Register.
  80-80       unknown (W)
  C0-C0       ROM Bank selector.
  E6-E6       NMI Mask.


***********************************************************************************

  +------------------------------------------------------------------------------+
  |             AMATIC Standard Edge Connector (Videomat, Multigame)             |
  +----------------------------------------+-------------------------------------+
  |            Component Side              |              Solder Side            |
  +-------------------+---------------+----+----+---------------+----------------+
  |     Function      |   Direction   | Nr | Nr |   Direction   |    Function    |
  +-------------------+---------------+----+----+---------------+----------------+
  | Lamp-HOLD3        |    OUTPUT     | 36 | r  |    ------     | N/C            |
  | REMOTE-PL         |    OUTPUT     | 35 | p  |    OUTPUT     | COIN-INVERTER  |
  | REMOTE-CLOCK      |    OUTPUT     | 34 | n  |    OUTPUT     | D-OUT          |
  | D-IN              |    INPUT      | 33 | m  |    SUPPLY     | +5V            |
  | REMOTE-SELECT     |    INPUT      | 32 | l  |    INPUT      | ???            |
  | N/C               |    ------     | 31 | k  |    INPUT      | ???            |
  | Coin-INPUT4       |    INPUT      | 30 | j  |    INPUT      | ???            |
  | GND               |    SUPPLY     | 29 | h  |    SUPPLY     | GND            |
  | HOPPER-RELAIS     |    OUTPUT     | 28 | f  |    INPUT      | ANTENNE        |
  | N/C               |    ------     | 27 | e  |    ------     | N/C            |
  | TICKET-OUT        |    OUTPUT     | 26 | d  |    ------     | N/C            |
  | N/C               |    ------     | 25 | c  |    ------     | N/C            |
  | KEY-DATA          |   TTLINPUT    | 24 | b  |   TTLINPUT    | KEY-DATA       |
  | N/C               |    ------     | 23 | a  |    INPUT      | KEY-CONTACT    |
  | GND               |    SUPPLY     | 22 | Z  |    SUPPLY     | GND            |
  | GND               |    SUPPLY     | 21 | Y  |    SUPPLY     | GND            |
  | GND               |    SUPPLY     | 20 | X  |    SUPPLY     | GND            |
  | +5V               |    SUPPLY     | 19 | W  |    SUPPLY     | +5V            |
  | +12V              |    SUPPLY     | 18 | V  |    SUPPLY     | +12V           |
  | Lamp-HOLD1        |    OUTPUT     | 17 | U  |    OUTPUT     | Lamp-START     |
  | Lamp-HOLD2        |    OUTPUT     | 16 | T  |    OUTPUT     | Lamp-HOLD5     |
  | Lamp-CANCEL       |    OUTPUT     | 15 | S  |    OUTPUT     | Lamp-HOLD4     |
  | Coin-INPUT1       |    INPUT      | 14 | R  |    OUTPUT     | HOPPER-OUT     |
  | Mech. Counter-IN  |    OUTPUT     | 13 | P  |    INPUT      | REMOTE         |
  | Mech. Counter-OUT |    OUTPUT     | 12 | N  |    INPUT      | Button-HOLD1   |
  | Mech. Counter-EXT |    OUTPUT     | 11 | M  |    INPUT      | Button-CANCEL  |
  | Button-HOLD5      |    INPUT      | 10 | L  |    INPUT      | Button-START   |
  | Bookkeeping 2     |    INPUT      | 09 | K  |    INPUT      | Bookkeeping 1  |
  | Button-HOLD2      |    INPUT      | 08 | J  |    INPUT      | Button-HOLD4   |
  | Button-HOPPER-OUT |    INPUT      | 07 | H  |    INPUT      | Button-HOLD3   |
  | HOPPER-COUNT      |    INPUT      | 06 | F  |    ------     | N/C            |
  | Coin-INPUT3       |    INPUT      | 05 | E  |    INPUT      | Coin-INPUT2    |
  | Monitor-GREEN     | TTLOUT-Analog | 04 | D  | TTLOUT-Analog | Monitor-RED    |
  | Monitor-SYNC      | TTLOUT-Analog | 03 | C  | TTLOUT-Analog | Monitor-BLUE   |
  | SPEAKER           |  OUT-Analog   | 02 | B  |    SUPPLY     | Monitor-GND    |
  | CREDIT-CLEAR      |    INPUT      | 01 | A  |    SUPPLY     | SPEAKER-GND    |
  +-------------------+---------------+----+----+---------------+----------------+

***********************************************************************************


  Findings about the encryption scheme
  ------------------------------------

  The program ROM is encrypted, but... The programmers left the cow out...
  They left some blank spaces that allow see some encryption patterns.


  Example:


  1) A string of 4 consecutive values with relation between them, is repeated once.
     Then the whole string is repeated again, but with bit 3 XOR'ed.

         +------------ value1
         |  +--------- value2 = value1 ^ 0x88
         |  |  +------ value3 = value1 ^ 0x22
         |  |  |  +--- value4 = value1 ^ 0x22 ^ 0x88
         |  |  |  |
         |  |  |  |

  $E000: AF 27 8D 05   AF 27 8D 05   A7 2F 85 0D  A7 2F 85 0D
        +-----------+ +-----------+ |                        |
           string1       string1    |                        |
        +-------------------------+ +------------------------+
                  string2                string2 ^ 0x08
        +----------------------------------------------------+
                                string3


  3) Then, all the E000-E00F range repeats in E010-E01F, but with bit 2 XOR'ed.

  $E010: AB 23 89 01 AB 23 89 01 A3 2B 81 09 A3 2B 81 09
        +-----------------------------------------------+
                        string3 ^ 0x04


  4) Then repeat all the E000-E01F range...

  $E000: AF 27 8D 05 AF 27 8D 05 A7 2F 85 0D A7 2F 85 0D
  $E010: AB 23 89 01 AB 23 89 01 A3 2B 81 09 A3 2B 81 09

  $E020: AF 27 8D 05 AF 27 8D 05 A7 2F 85 0D A7 2F 85 0D
  $E030: AB 23 89 01 AB 23 89 01 A3 2B 81 09 A3 2B 81 09


  5) Then the original value changes (0xAF -> 0x63), using the same algorithm
     (steps 1-4) for the next 0x40 bytes...

  $E040: 63 EB 41 C9 63 EB 41 C9 6B E3 49 C1 6B E3 49 C1
  $E050: 67 EF 45 CD 67 EF 45 CD 6F E7 4D C5 6F E7 4D C5
  $E060: 63 EB 41 C9 63 EB 41 C9 6B E3 49 C1 6B E3 49 C1
  $E070: 67 EF 45 CD 67 EF 45 CD 6F E7 4D C5 6F E7 4D C5

  Repeat step (5) ...till $E0FF.


  The encryption pattern repeats for the next 0x100 bytes, so the E000-E0FF range is repeated
  in E100-E1FF. Then the original values changes again, creating another block of 0x200 bytes.

  And so on....


***********************************************************************************


  [2012/08/15]

  - Added dynamic length to the color PROMs decode routines based on ROM region length.
    This fixes a horrible hang/crash in DEBUG=1 builds.

  [2012/04/27]

  - Reworked the decryption function.
  - Added Multi Game III (V.Ger 3.64).

  [2012/04/23]

  - A lot of work on AMA-8000-1 machine and gfx.
  - Identified the slots game as Super Stars.
  - Changed am_uslot to suprstar.
  - Reworked inputs from the scratch.
  - Added support for outputs: lamps & counters.
  - Added a button-lamps layout.
  - Promoted the game to working state.
  - Added technical notes.
  - Renamed amaticmg3 to amaticmg2 since is the AMA-8000-2 system.
  - Found the hopper motor signal. Mapped the hopper pay pulse to
     key 'Q'. Now is possible to payout manually, avoiding the hang
     for hopper empty or timeout.

  [2009/09/11]

  - Initial release.
  - Added hardware specs from PCB pictures. Figured out some components.
  - Added findings about the encryption scheme.
  - Decoded 2 graphics bitplanes.
  - Added pre-defined clocks.
  - Added technical notes.


  *** TO DO ***

  - Super Stars: video garbage at first boot (doesn't happen if you soft-reset), btanb?
  - Hook the remaining GFX bitplanes.
  - Color decode routines.
  - Remaining sound devices.
  - Hopper as device... ;)


***********************************************************************************/


#define MASTER_CLOCK    XTAL_16MHz
#define CPU_CLOCK       MASTER_CLOCK/4  /* guess */
#define SND_CLOCK       MASTER_CLOCK/4  /* guess */
#define CRTC_CLOCK      MASTER_CLOCK/8  /* guess */


#include "emu.h"
#include "cpu/z80/z80.h"
#include "video/mc6845.h"
#include "machine/i8255.h"
#include "sound/3812intf.h"
#include "sound/dac.h"

#include "suprstar.lh"


class amaticmg_state : public driver_device
{
public:
	amaticmg_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_attr(*this, "attr"),
		m_vram(*this, "vram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	required_shared_ptr<UINT8> m_attr;
	required_shared_ptr<UINT8> m_vram;

	DECLARE_WRITE8_MEMBER(rombank_w);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(unk80_w);

	UINT8 m_nmi_mask;
	DECLARE_WRITE8_MEMBER(out_a_w);
	DECLARE_WRITE8_MEMBER(out_c_w);
	DECLARE_DRIVER_INIT(ama8000_3_o);
	DECLARE_DRIVER_INIT(ama8000_2_i);
	DECLARE_DRIVER_INIT(ama8000_2_v);
	DECLARE_DRIVER_INIT(ama8000_1_x);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(amaticmg);
	DECLARE_PALETTE_INIT(amaticmg2);
	UINT32 screen_update_amaticmg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_amaticmg2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(amaticmg2_irq);
	void encf(UINT8 ciphertext, int address, UINT8 &plaintext, int &newaddress);
	void decrypt(int key1, int key2);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};


/************************************
*          Video Hardware           *
************************************/

void amaticmg_state::video_start()
{
}

UINT32 amaticmg_state::screen_update_amaticmg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	int y,x;
	int count = 0;

	for (y=0;y<32;y++)
	{
		for (x=0;x<96;x++)
		{
			UINT16 tile = m_vram[count];
			UINT8 color;

			tile += ((m_attr[count]&0x0f)<<8);
			/* TODO: this looks so out of place ... */
			color = (m_attr[count]&0xf0)>>3;

			gfx->opaque(bitmap,cliprect,tile,color,0,0,x*4,y*8);
			count++;
		}
	}

	return 0;
}

UINT32 amaticmg_state::screen_update_amaticmg2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	int y,x;
	int count = 16;

	for (y=0;y<32;y++)
	{
		for (x=0;x<96;x++)
		{
			UINT16 tile = m_vram[count];
			UINT8 color;

			tile += ((m_attr[count]&0xff)<<8);
			color = 0;

			gfx->opaque(bitmap,cliprect,tile,color,0,0,x*4,y*8);
			count++;
		}
	}

	return 0;
}

PALETTE_INIT_MEMBER(amaticmg_state, amaticmg)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int bit0, bit1, bit2 , r, g, b;
	int i;

	for (i = 0; i < 0x200; ++i)
	{
		bit0 = 0;
		bit1 = (color_prom[0] >> 6) & 0x01;
		bit2 = (color_prom[0] >> 7) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[0] >> 3) & 0x01;
		bit1 = (color_prom[0] >> 4) & 0x01;
		bit2 = (color_prom[0] >> 5) & 0x01;
		b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
		color_prom++;
	}
}


PALETTE_INIT_MEMBER(amaticmg_state,amaticmg2)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int r, g, b;
	int i;

	for (i = 0; i < memregion("proms")->bytes(); i+=2)
	{
		b = ((color_prom[1] & 0xf8) >> 3);
		g = ((color_prom[0] & 0xc0) >> 6) | ((color_prom[1] & 0x7) << 2);
		r = ((color_prom[0] & 0x3e) >> 1);

		palette.set_pen_color(i >> 1, pal5bit(r), pal5bit(g), pal5bit(b));
		color_prom+=2;
	}
}

/************************************
*       Read/Write Handlers         *
************************************/

WRITE8_MEMBER( amaticmg_state::rombank_w )
{
	membank("bank1")->set_entry(data & 0xf);
}

WRITE8_MEMBER( amaticmg_state::nmi_mask_w )
{
	m_nmi_mask = (data & 1) ^ 1;
}

WRITE8_MEMBER(amaticmg_state::out_a_w)
{
/*  LAMPS A:

    7654 3210
    x--- -xxx  (unknown)
    ---- x---  START
    ---x ----  BET
    --x- ----  HOLD3
    -x-- ----  HOLD4
*/

	output().set_lamp_value(0, (data >> 3) & 1);  /* START */
	output().set_lamp_value(1, (data >> 4) & 1);  /* BET */
	output().set_lamp_value(2, (data >> 5) & 1);  /* HOLD3 */
	output().set_lamp_value(3, (data >> 6) & 1);  /* HOLD4 */

	logerror("port A: %2X\n", data);
}

WRITE8_MEMBER(amaticmg_state::out_c_w)
{
/*  LAMPS B:

    7654 3210
    ---- ---x  Coin Out counter
    ---- --x-  HOLD1
    ---- -x--  Coin In counter
    ---x ----  HOLD2
    -x-- ----  CANCEL
    x--- ----  Hopper motor
    --x- x---  (unknown)
*/
	output().set_lamp_value(4, (data >> 1) & 1);  /* HOLD1 */
	output().set_lamp_value(5, (data >> 4) & 1);  /* HOLD2 */
	output().set_lamp_value(6, (data >> 6) & 1);  /* CANCEL */

//  machine().bookkeeping().coin_counter_w(0, data & 0x04);  /* Coin In */
//  machine().bookkeeping().coin_counter_w(1, data & 0x01);  /* Coin Out */

	logerror("port C: %2X\n", data);
}

WRITE8_MEMBER( amaticmg_state::unk80_w )
{
//  m_dac->write_unsigned8(data & 0x01);       /* Sound DAC */
}



/************************************
*      Memory Map Information       *
************************************/

static ADDRESS_MAP_START( amaticmg_map, AS_PROGRAM, 8, amaticmg_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0x9fff) AM_RAM // AM_SHARE("nvram")
	AM_RANGE(0xa000, 0xafff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0xb000, 0xbfff) AM_RAM AM_SHARE("attr")
//  AM_RANGE(0xa010, 0xafff) AM_RAM AM_SHARE("vram")
//  AM_RANGE(0xb010, 0xbfff) AM_RAM AM_SHARE("attr")
	AM_RANGE(0xc000, 0xffff) AM_ROMBANK("bank1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( amaticmg_portmap, AS_IO, 8, amaticmg_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0x40, 0x41) AM_DEVWRITE("ymsnd", ym3812_device, write)
	AM_RANGE(0x60, 0x60) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x61, 0x61) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x80, 0x80) AM_WRITE(unk80_w)
	AM_RANGE(0xc0, 0xc0) AM_WRITE(rombank_w)
//  AM_RANGE(0x00, 0x00) AM_DEVREADWRITE("ppi8255_2", ppi8255_device, read, write)
//  AM_RANGE(0x00, 0x00) AM_DEVWRITE("dac1", dac_device, write_signed8)
//  AM_RANGE(0x00, 0x00) AM_DEVWRITE("dac2", dac_device, write_signed8)

ADDRESS_MAP_END

static ADDRESS_MAP_START( amaticmg2_portmap, AS_IO, 8, amaticmg_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
//  ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0x20, 0x23) AM_DEVREADWRITE("ppi8255_1", i8255_device, read, write)
	AM_RANGE(0x40, 0x41) AM_DEVWRITE("ymsnd", ym3812_device, write)
	AM_RANGE(0x60, 0x60) AM_DEVWRITE("crtc", mc6845_device, address_w)                  // 0e for mg_iii_vger_3.64_v_8309
	AM_RANGE(0x61, 0x61) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w) // 0f for mg_iii_vger_3.64_v_8309
	AM_RANGE(0xc0, 0xc0) AM_WRITE(rombank_w)
	AM_RANGE(0xe6, 0xe6) AM_WRITE(nmi_mask_w)
ADDRESS_MAP_END


/*
    Unknown R/W
    -----------


*/


/************************************
*           Input ports             *
************************************/

static INPUT_PORTS_START( amaticmg )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_NAME("Coin 1 (Muenze 1)") PORT_IMPULSE(3)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 )       PORT_NAME("Service B (Dienst B") PORT_CODE(KEYCODE_8) PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )          PORT_NAME("Coin 2 (Muenze 2)")   PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )          PORT_NAME("Hopper Payout pulse") PORT_IMPULSE(3)      PORT_CODE(KEYCODE_Q)  // Hopper paying pulse
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  PORT_CODE(KEYCODE_W)           // 'Ausgegeben 0 - Hopper Leer' (spent 0 - hopper empty)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Hold 3 (Halten 3)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Hold 2 (Halten 2)")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Clear / Take (Loeschen)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Hold 1 (Halten 1)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )       PORT_NAME("Service A (Dienst A") PORT_CODE(KEYCODE_7) PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BET )     PORT_NAME("Bet (Setzen) / Half Take")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE3 )       PORT_NAME("Service C (Dienst C") PORT_CODE(KEYCODE_9) PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Service (Master)")    PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Hold 4 (Halten 4)")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, "DIP1")                   PORT_DIPLOCATION("DIP:1")
	PORT_DIPSETTING(    0x01, "Off (Aus)" )
	PORT_DIPSETTING(    0x00, "On (Ein)" )
	PORT_DIPNAME( 0x02, 0x02, "DIP2")                   PORT_DIPLOCATION("DIP:2")
	PORT_DIPSETTING(    0x02, "Off (Aus)" )
	PORT_DIPSETTING(    0x00, "On (Ein)" )
	PORT_DIPNAME( 0x04, 0x04, "Coin 1 (Muenzen 1)" )    PORT_DIPLOCATION("DIP:3")
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x08, 0x08, "Coin 2 (Muenzen 2)" )    PORT_DIPLOCATION("DIP:4")
	PORT_DIPSETTING(    0x08, "100" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, "Jackpot")                PORT_DIPLOCATION("DIP:5")
	PORT_DIPSETTING(    0x10, "Jackpot KZB")
	PORT_DIPSETTING(    0x00, "Jackpot LZB")
	PORT_DIPNAME( 0x20, 0x20, "Fruechtebonus" )         PORT_DIPLOCATION("DIP:6")
	PORT_DIPSETTING(    0x20, "Fruechtebonus Bleibt" )
	PORT_DIPSETTING(    0x00, "Fruechtebonus Clear" )
	PORT_DIPNAME( 0x40, 0x40, "DIP7")                   PORT_DIPLOCATION("DIP:7")
	PORT_DIPSETTING(    0x40, "Off (Aus)" )
	PORT_DIPSETTING(    0x00, "On (Ein)" )
	PORT_DIPNAME( 0x80, 0x80, "BH")                     PORT_DIPLOCATION("DIP:8")
	PORT_DIPSETTING(    0x80, "BH Dreifach")
	PORT_DIPSETTING(    0x00, "BH Normal")
INPUT_PORTS_END


/************************************
*         Graphics Layouts          *
************************************/

static const gfx_layout charlayout_4bpp =
{
	4,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(0,2), RGN_FRAC(0,2) + 4, RGN_FRAC(1,2), RGN_FRAC(1,2) + 4 },
	{ 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*4*2
};

static const gfx_layout charlayout_6bpp =
{
	4,8,
	RGN_FRAC(1,3),
	6,
	{ RGN_FRAC(0,3) + 0, RGN_FRAC(0,3) + 4, RGN_FRAC(1,3) + 0, RGN_FRAC(1,3) + 4,RGN_FRAC(2,3) + 0, RGN_FRAC(2,3) + 4, },
	{ 3, 2, 1, 0 }, /* tiles are x-flipped */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*4*2
};


/************************************
*    Graphics Decode Information    *
************************************/

static GFXDECODE_START( amaticmg )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout_4bpp, 0, 0x20 )
GFXDECODE_END

static GFXDECODE_START( amaticmg2 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout_6bpp, 0, 0x10000/0x40 )
GFXDECODE_END


/************************************
*       Machine Start & Reset       *
************************************/

void amaticmg_state::machine_start()
{
	UINT8 *rombank = memregion("maincpu")->base();

	membank("bank1")->configure_entries(0, 0x10, &rombank[0x8000], 0x4000);
}

void amaticmg_state::machine_reset()
{
	membank("bank1")->set_entry(0);
	m_nmi_mask = 0;
}


/************************************
*          Machine Drivers          *
************************************/

static MACHINE_CONFIG_START( amaticmg, amaticmg_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, CPU_CLOCK)     /* WRONG! */
	MCFG_CPU_PROGRAM_MAP(amaticmg_map)
	MCFG_CPU_IO_MAP(amaticmg_portmap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", amaticmg_state,  nmi_line_pulse) // no NMI mask?

//  MCFG_NVRAM_ADD_0FILL("nvram")

	/* 3x 8255 */
	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("IN0"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("IN1"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN2"))

	MCFG_DEVICE_ADD("ppi8255_1", I8255A, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(amaticmg_state, out_a_w))
	MCFG_I8255_IN_PORTB_CB(IOPORT("SW1"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(amaticmg_state, out_c_w))

//  MCFG_DEVICE_ADD("ppi8255_2", I8255A, 0)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 512-1, 0, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(amaticmg_state, screen_update_amaticmg)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_MC6845_ADD("crtc", MC6845, "screen", CRTC_CLOCK)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(4)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", amaticmg)

	MCFG_PALETTE_ADD("palette", 0x200)
	MCFG_PALETTE_INIT_OWNER(amaticmg_state, amaticmg)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, SND_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

//  MCFG_DAC_ADD("dac")   /* Y3014B */
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

MACHINE_CONFIG_END


INTERRUPT_GEN_MEMBER(amaticmg_state::amaticmg2_irq)
{
	if(m_nmi_mask)
		device.execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


static MACHINE_CONFIG_DERIVED( amaticmg2, amaticmg )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_IO_MAP(amaticmg2_portmap)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", amaticmg_state,  amaticmg2_irq)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(amaticmg_state, screen_update_amaticmg2)

	MCFG_GFXDECODE_MODIFY("gfxdecode", amaticmg2)
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_ENTRIES(0x10000)
	MCFG_PALETTE_INIT_OWNER(amaticmg_state,amaticmg2)
MACHINE_CONFIG_END


/************************************
*             Rom Load              *
************************************/

ROM_START( suprstar )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000, "mainprg", 0 ) /* encrypted program ROM...*/
	ROM_LOAD( "u3.bin",  0x00000, 0x20000, CRC(29bf4a95) SHA1(a73873f7cd1fdf5accc3e79f4619949f261400b8) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "u10.bin", 0x00000, 0x08000, CRC(6a811c81) SHA1(af01cd9b1ce6aca92df71febb05fe216b18cf42a) )
	ROM_CONTINUE(        0x00000, 0x08000 )
	ROM_LOAD( "u9.bin",  0x08000, 0x08000, CRC(823a736a) SHA1(a5227e3080367736aac1198d9dbb55efc4114624) )
	ROM_CONTINUE(        0x08000, 0x08000 )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147a.bin", 0x0000, 0x0200, CRC(dfeabd11) SHA1(21e8bbcf4aba5e4d672e5585890baf8c5bc77c98) )
ROM_END


ROM_START( am_mg24 )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x40000, "mainprg", 0 ) /* encrypted program ROM...*/
	ROM_LOAD( "mgi_vger_3.9-i-8201.i6.bin", 0x00000, 0x40000, CRC(9ce159f7) SHA1(101c277d579a69cb03f879288b2cecf838cf1741) )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "multi_2.4_zg1.i17.bin", 0x100000, 0x80000, CRC(4a60a718) SHA1(626991abee768da58e87c7cdfc4fcbae86c6ea2a) )
	ROM_LOAD( "multi_2.4_zg2.i18.bin", 0x080000, 0x80000, CRC(b504e1b8) SHA1(ffa17a2c212eb2fffb89b131868e69430cb41203) )
	ROM_LOAD( "multi_2.4_zg3.i33.bin", 0x000000, 0x80000, CRC(9b66bb4d) SHA1(64035d2028a9b68164c87475a1ec9754453ad572) )

	ROM_REGION( 0x20000/*0x0400*/, "proms", 0 )
	ROM_LOAD( "n82s147a_1.bin", 0x0000, 0x0200, NO_DUMP )
	ROM_LOAD( "n82s147a_2.bin", 0x0200, 0x0200, NO_DUMP )
ROM_END

ROM_START( am_mg3 )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x40000, "mainprg", 0 ) /* encrypted program ROM...*/
	ROM_LOAD( "mg_iii_vger_3.5-i-8205.bin", 0x00000, 0x40000, CRC(21d64029) SHA1(d5c3fde02833a96dd7a43481a489bfc4a5c9609d) )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "mg_iii_51_zg1.bin", 0x100000, 0x80000, CRC(84f86874) SHA1(c483a50df6a9a71ddfdf8530a894135f9b852b89) )
	ROM_LOAD( "mg_iii_51_zg2.bin", 0x080000, 0x80000, CRC(4425e535) SHA1(726c322c5d0b391b82e49dd1797ebf0abfa4a65a) )
	ROM_LOAD( "mg_iii_51_zg3.bin", 0x000000, 0x80000, CRC(36d4c0fa) SHA1(20352dbbb2ce2233be0f4f694ddf49b8f5d6a64f) )

	ROM_REGION( 0x20000, "proms", 0 )
	ROM_LOAD( "v.bin", 0x00000, 0x20000, CRC(524767e2) SHA1(03a108494f42365c820fdfbcba9496bda86f3081) )
ROM_END

ROM_START( am_mg3a )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x40000, "mainprg", 0 ) /* encrypted program ROM...*/
	ROM_LOAD( "mg_iii_vger_3.64_v_8309.i16", 0x00000, 0x40000, CRC(c54f97c4) SHA1(d5ce91be7332ada304d18d07706e3b98ac0fa74b) )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "mg_iii_51_zg1.i17", 0x100000, 0x80000, CRC(84f86874) SHA1(c483a50df6a9a71ddfdf8530a894135f9b852b89) )
	ROM_LOAD( "mg_iii_51_zg2.i18", 0x080000, 0x80000, CRC(4425e535) SHA1(726c322c5d0b391b82e49dd1797ebf0abfa4a65a) )
	ROM_LOAD( "mg_iii_51_zg3.i19", 0x000000, 0x80000, CRC(36d4c0fa) SHA1(20352dbbb2ce2233be0f4f694ddf49b8f5d6a64f) )

	ROM_REGION( 0x20000, "proms", 0 )
	ROM_LOAD( "iv.i35", 0x00000, 0x20000, CRC(82af7296) SHA1(1a07d6481e0f8fd785be9f1b737182d7e0b84605) )
ROM_END

/* Italian sets... */

ROM_START( am_mg31i )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x40000, "mainprg", 0 ) /* encrypted program ROM...*/
	ROM_LOAD( "mgi_sita_3.1_o_8270.bin", 0x00000, 0x40000, CRC(7358bdde) SHA1(674b57ddaaaed9b88ad563762b2421be7057e498) )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "mg2_ita.1", 0x100000, 0x80000, BAD_DUMP CRC(8663ce10) SHA1(00606bc69bd3a81a2f1b618d018d3ac315169fe4) )
	ROM_LOAD( "mg2_ita.2", 0x080000, 0x80000, BAD_DUMP CRC(7dbaf752) SHA1(afefbd239519abb4898348a3923ff093e36fbcb0) )
	ROM_LOAD( "mg2_ita.3", 0x000000, 0x80000, BAD_DUMP CRC(5dba55cf) SHA1(d237f8b3c72e8b59974059156070d0618ec41e9a) )

	ROM_REGION( 0x4000, "proms", 0 )
	ROM_LOAD( "ama80003_fprom.bin", 0x0000, 0x4000, BAD_DUMP CRC(65a784b8) SHA1(bd23136261e22f0294cff90040f3015ba0c10d7e) )
ROM_END

ROM_START( am_mg33i )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x40000, "mainprg", 0 ) /* encrypted program ROM...*/
	ROM_LOAD( "mgi_sita_3.3_o_8270.bin", 0x00000, 0x40000, CRC(eaa1ed83) SHA1(e50d06ea3631bd6e4f5fe14d8283c3550b2779a6) )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "mg2_ita.1", 0x100000, 0x80000, BAD_DUMP CRC(8663ce10) SHA1(00606bc69bd3a81a2f1b618d018d3ac315169fe4) )
	ROM_LOAD( "mg2_ita.2", 0x080000, 0x80000, BAD_DUMP CRC(7dbaf752) SHA1(afefbd239519abb4898348a3923ff093e36fbcb0) )
	ROM_LOAD( "mg2_ita.3", 0x000000, 0x80000, BAD_DUMP CRC(5dba55cf) SHA1(d237f8b3c72e8b59974059156070d0618ec41e9a) )

	ROM_REGION( 0x4000, "proms", 0 )
	ROM_LOAD( "ama80003_fprom.bin", 0x0000, 0x4000, BAD_DUMP CRC(65a784b8) SHA1(bd23136261e22f0294cff90040f3015ba0c10d7e) )
ROM_END

ROM_START( am_mg34i )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x40000, "mainprg", 0 ) /* encrypted program ROM...*/
	ROM_LOAD( "mgi_sita_3.4_o_8270.bin", 0x00000, 0x40000, CRC(bea7cd25) SHA1(89c9e02b48f34b2168e8624e552ead476cc339b9) )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "mg2_ita.1", 0x100000, 0x80000, BAD_DUMP CRC(8663ce10) SHA1(00606bc69bd3a81a2f1b618d018d3ac315169fe4) )
	ROM_LOAD( "mg2_ita.2", 0x080000, 0x80000, BAD_DUMP CRC(7dbaf752) SHA1(afefbd239519abb4898348a3923ff093e36fbcb0) )
	ROM_LOAD( "mg2_ita.3", 0x000000, 0x80000, BAD_DUMP CRC(5dba55cf) SHA1(d237f8b3c72e8b59974059156070d0618ec41e9a) )

	ROM_REGION( 0x4000, "proms", 0 )
	ROM_LOAD( "ama80003_fprom.bin", 0x0000, 0x4000, BAD_DUMP CRC(65a784b8) SHA1(bd23136261e22f0294cff90040f3015ba0c10d7e) )
ROM_END

ROM_START( am_mg35i )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x40000, "mainprg", 0 ) /* encrypted program ROM...*/
	ROM_LOAD( "mgi_sita_3.5_o_8270.bin", 0x00000, 0x40000, CRC(816eb41e) SHA1(0cad597e764455011d03f519e4adafb310e75451) )

	ROM_REGION( 0x180000, "gfx1", 0 )
	ROM_LOAD( "mg2_ita.1", 0x100000, 0x80000, BAD_DUMP CRC(8663ce10) SHA1(00606bc69bd3a81a2f1b618d018d3ac315169fe4) )
	ROM_LOAD( "mg2_ita.2", 0x080000, 0x80000, BAD_DUMP CRC(7dbaf752) SHA1(afefbd239519abb4898348a3923ff093e36fbcb0) )
	ROM_LOAD( "mg2_ita.3", 0x000000, 0x80000, BAD_DUMP CRC(5dba55cf) SHA1(d237f8b3c72e8b59974059156070d0618ec41e9a) )

	ROM_REGION( 0x4000, "proms", 0 )
	ROM_LOAD( "ama80003_fprom.bin", 0x0000, 0x4000, BAD_DUMP CRC(65a784b8) SHA1(bd23136261e22f0294cff90040f3015ba0c10d7e) )
ROM_END


/************************************
*       Driver Initialization       *
************************************/

void amaticmg_state::encf(UINT8 ciphertext, int address, UINT8 &plaintext, int &newaddress)
{
	int aux = address & 0xfff;
	aux = aux ^ (aux>>6);
	aux = ((aux<<6) | (aux>>6)) & 0xfff;
	UINT8 aux2 = BITSWAP8(aux, 9,10,4,1,6,0,7,3);
	aux2 ^= aux2>>4;
	aux2 = (aux2<<4) | (aux2>>4);
	ciphertext ^= ciphertext<<4;
	plaintext = (ciphertext<<4) | (ciphertext>>4);
	plaintext ^= aux2;
	newaddress = (address & ~0xfff) | aux;
}

void amaticmg_state::decrypt(int key1, int key2)
{
	UINT8 plaintext;
	int newaddress;

	UINT8 *src = memregion("mainprg")->base();
	UINT8 *dest = memregion("maincpu")->base();
	int len = memregion("mainprg")->bytes();

	for (int i = 0; i < len; i++)
	{
		encf(src[i], i, plaintext, newaddress);
		dest[newaddress^(key1^(key1>>6))] = plaintext^key2;
	}
}

DRIVER_INIT_MEMBER(amaticmg_state,ama8000_1_x)
{
	decrypt(0x4d1, 0xf5);
}

DRIVER_INIT_MEMBER(amaticmg_state,ama8000_2_i)
{
	decrypt(0x436, 0x55);
}

DRIVER_INIT_MEMBER(amaticmg_state,ama8000_2_v)
{
	decrypt(0x703, 0xaf);
}

DRIVER_INIT_MEMBER(amaticmg_state,ama8000_3_o)
{
	decrypt(0x56e, 0xa7);
}


/************************************
*           Game Drivers            *
************************************/

/*     YEAR  NAME      PARENT    MACHINE    INPUT     STATE           INIT         ROT     COMPANY                FULLNAME                      FLAGS                                                                                                        LAYOUT */
GAMEL( 1996, suprstar, 0,        amaticmg,  amaticmg, amaticmg_state, ama8000_1_x, ROT90, "Amatic Trading GmbH", "Super Stars",                 MACHINE_IMPERFECT_SOUND,                                                                                        layout_suprstar )
GAME(  2000, am_mg24,  0,        amaticmg2, amaticmg, amaticmg_state, ama8000_2_i, ROT0,  "Amatic Trading GmbH", "Multi Game I (V.Ger 2.4)",    MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME(  2000, am_mg3,   0,        amaticmg2, amaticmg, amaticmg_state, ama8000_2_i, ROT0,  "Amatic Trading GmbH", "Multi Game III (V.Ger 3.5)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME(  2000, am_mg3a,  0,        amaticmg2, amaticmg, amaticmg_state, ama8000_2_v, ROT0,  "Amatic Trading GmbH", "Multi Game III (V.Ger 3.64)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME(  2000, am_mg35i, 0,        amaticmg2, amaticmg, amaticmg_state, ama8000_3_o, ROT0,  "Amatic Trading GmbH", "Multi Game III (S.Ita 3.5)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME(  2000, am_mg34i, am_mg35i, amaticmg2, amaticmg, amaticmg_state, ama8000_3_o, ROT0,  "Amatic Trading GmbH", "Multi Game III (S.Ita 3.4)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME(  2000, am_mg33i, am_mg35i, amaticmg2, amaticmg, amaticmg_state, ama8000_3_o, ROT0,  "Amatic Trading GmbH", "Multi Game III (S.Ita 3.3)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME(  2000, am_mg31i, am_mg35i, amaticmg2, amaticmg, amaticmg_state, ama8000_3_o, ROT0,  "Amatic Trading GmbH", "Multi Game III (S.Ita 3.1)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
