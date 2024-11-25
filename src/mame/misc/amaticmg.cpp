// license:BSD-3-Clause
// copyright-holders: Roberto Fresca, Angelo Salese

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
  1x KS82C55A (8255A PPI variant).
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

  1x Custom DIP24 IC labeled K-666 9330 (equivalent to YM3812).
  1x Custom DIP8 IC labeled K-664 (equivalent to YM3014).
  1x LM358P (DIP8, dual operational amplifier).
  1x MC14538BCL (dual precision monostable multivibrator).
  1x TDA2003 (10W. audio amplifier).
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
  1x LM358M (DIP8, dual operational amplifier).
  1x MC14538 (dual precision monostable multivibrator).
  1x TDA2003 (10W. audio amplifier) heatsinked.
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


#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/ds1994.h"
#include "machine/i8255.h"
//#include "sound/dac.h"
#include "sound/ymopl.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "suprstar.lh"


// configurable logging
#define LOG_PPIOUT     (1U << 1)

//#define VERBOSE (LOG_GENERAL | LOG_PPIOUT)

#include "logmacro.h"

#define LOGPPIOUT(...)     LOGMASKED(LOG_PPIOUT,     __VA_ARGS__)


namespace {

class amaticmg_state : public driver_device
{
public:
	amaticmg_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_attr(*this, "attr"),
		m_vram(*this, "vram"),
		m_rombank(*this, "rombank"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_tch(*this, "touch_m"),
		m_lamps(*this, "lamp%u", 0U)
	{
	}

	void amaticmg2(machine_config &config);
	void amaticmg(machine_config &config);
	void amaticmg4(machine_config &config);

	void init_ama8000_3_o();
	void init_ama8000_2_i();
	void init_ama8000_2_v();
	void init_ama8000_1_x();
	void init_am_mg5hu();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_shared_ptr<uint8_t> m_attr;
	required_shared_ptr<uint8_t> m_vram;
	required_memory_bank m_rombank;

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<ds1994_device> m_tch;
	output_finder<7> m_lamps;

	uint8_t m_nmi_mask = 0;

	uint8_t epm_code_r();
	uint8_t touchm_r();
	void touchm_w(uint8_t data);
	void rombank_w(uint8_t data);
	void nmi_mask_w(uint8_t data);
	void unk80_w(uint8_t data);

	void out_a_w(uint8_t data);
	void out_c_w(uint8_t data);
	void amaticmg_palette(palette_device &palette) const;
	void amaticmg2_palette(palette_device &palette) const;
	uint32_t screen_update_amaticmg(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_amaticmg2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void amaticmg2_irq(int state);
	void encf(uint8_t ciphertext, int address, uint8_t &plaintext, int &newaddress);
	void decrypt(int key1, int key2);

	void amaticmg2_portmap(address_map &map) ATTR_COLD;
	void amaticmg4_portmap(address_map &map) ATTR_COLD;
	void amaticmg_map(address_map &map) ATTR_COLD;
	void amaticmg_portmap(address_map &map) ATTR_COLD;
};


/************************************
*          Video Hardware           *
************************************/

uint32_t amaticmg_state::screen_update_amaticmg(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	int count = 0;

	for (int y = 0; y < 32; y++)
	{
		for (int x = 0; x < 96; x++)
		{
			uint16_t tile = m_vram[count];

			tile += ((m_attr[count] & 0x0f) << 8);
			// TODO: this looks so out of place ...
			uint8_t const color = (m_attr[count] & 0xf0) >> 3;

			gfx->opaque(bitmap, cliprect, tile, color, 0, 0, x * 4, y * 8);
			count++;
		}
	}

	return 0;
}

uint32_t amaticmg_state::screen_update_amaticmg2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	int count = 16;

	for (int y = 0; y < 32; y++)
	{
		for (int x = 0; x < 96; x++)
		{
			uint16_t tile = m_vram[count];

			tile += ((m_attr[count] & 0xff) << 8);
			uint8_t color = 0;

			gfx->opaque(bitmap, cliprect, tile, color, 0, 0, x * 4, y * 8);
			count++;
		}
	}

	return 0;
}

void amaticmg_state::amaticmg_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();

	for (int i = 0; i < 0x200; ++i)
	{
		int bit0, bit1, bit2;

		bit0 = 0;
		bit1 = BIT(color_prom[i], 6);
		bit2 = BIT(color_prom[i], 7);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = BIT(color_prom[i], 0);
		bit1 = BIT(color_prom[i], 1);
		bit2 = BIT(color_prom[i], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


void amaticmg_state::amaticmg2_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	for (int i = 0, n = memregion("proms")->bytes(); i < n; i += 2)
	{
		int const b = ((color_prom[1] & 0xf8) >> 3);
		int const g = ((color_prom[0] & 0xc0) >> 6) | ((color_prom[1] & 0x7) << 2);
		int const r = ((color_prom[0] & 0x3e) >> 1);

		palette.set_pen_color(i >> 1, pal5bit(r), pal5bit(g), pal5bit(b));
		color_prom += 2;
	}
}

/************************************
*       Read/Write Handlers         *
************************************/
uint8_t amaticmg_state::epm_code_r()
{
	return 0x65;
}

uint8_t amaticmg_state::touchm_r()
{
	return m_tch->read() & 1;
}

void amaticmg_state::touchm_w(uint8_t data)
{
	m_tch->write(data & 1);
}

void amaticmg_state::rombank_w(uint8_t data)
{
	m_rombank->set_entry(data & 0xf);
}

void amaticmg_state::nmi_mask_w(uint8_t data)
{
	m_nmi_mask = (data & 1) ^ 1;
}

void amaticmg_state::out_a_w(uint8_t data)
{
/*  LAMPS A:

    7654 3210
    x--- -xxx  (unknown)
    ---- x---  START
    ---x ----  BET
    --x- ----  HOLD3
    -x-- ----  HOLD4
*/

	m_lamps[0] = BIT(data, 3);  // START
	m_lamps[1] = BIT(data, 4);  // BET
	m_lamps[2] = BIT(data, 5);  // HOLD3
	m_lamps[3] = BIT(data, 6);  // HOLD4

	LOGPPIOUT("port A: %2X\n", data);
}

void amaticmg_state::out_c_w(uint8_t data)
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
	m_lamps[4] = BIT(data, 1);  // HOLD1
	m_lamps[5] = BIT(data, 4);  // HOLD2
	m_lamps[6] = BIT(data, 6);  // CANCEL

//  machine().bookkeeping().coin_counter_w(0, data & 0x04);  // Coin In
//  machine().bookkeeping().coin_counter_w(1, data & 0x01);  // Coin Out

	LOGPPIOUT("port C: %2X\n", data);
}

void amaticmg_state::unk80_w(uint8_t data)
{
//  m_dac->write(BIT(data, 0));       // Sound DAC
}



/************************************
*      Memory Map Information       *
************************************/

void amaticmg_state::amaticmg_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x9fff).ram(); // .share("nvram");
	map(0xa000, 0xafff).ram().share(m_vram);
	map(0xb000, 0xbfff).ram().share(m_attr);
	map(0xc000, 0xffff).bankr(m_rombank);
}

void amaticmg_state::amaticmg_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x20, 0x23).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x40, 0x41).w("ymsnd", FUNC(ym3812_device::write));
	map(0x60, 0x60).w("crtc", FUNC(mc6845_device::address_w));
	map(0x61, 0x61).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x80, 0x80).w(FUNC(amaticmg_state::unk80_w));
	map(0xc0, 0xc0).w(FUNC(amaticmg_state::rombank_w));
//  map(0x00, 0x00).w("dac1", FUNC(dac_byte_interface::data_w));
//  map(0x00, 0x00).w("dac2", FUNC(dac_byte_interface::data_w));
}

void amaticmg_state::amaticmg2_portmap(address_map &map)
{
	map.global_mask(0xff);
//  map.unmap_value_high();
	map(0x00, 0x03).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x20, 0x23).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x40, 0x41).w("ymsnd", FUNC(ym3812_device::write));
	map(0x60, 0x60).w("crtc", FUNC(mc6845_device::address_w));                                    // 0e for mg_iii_vger_3.64_v_8309
	map(0x61, 0x61).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w)); // 0f for mg_iii_vger_3.64_v_8309
	map(0xc0, 0xc0).w(FUNC(amaticmg_state::rombank_w));
	map(0xe0, 0xe0).rw(FUNC(amaticmg_state::touchm_r),FUNC(amaticmg_state::touchm_w));            // Touch Memory DS1994f
	map(0xe4, 0xe4).r(FUNC(amaticmg_state::epm_code_r));                                          // Input(0x00E4)  must give back 0x65  in case of  EPM Code 8201
	map(0xe6, 0xe6).w(FUNC(amaticmg_state::nmi_mask_w));
	map(0xe8, 0xeb).rw("ppi8255_2", FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void amaticmg_state::amaticmg4_portmap(address_map &map)
{
	map.global_mask(0xff);
//  map.unmap_value_high();
	map(0x00, 0x03).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x04, 0x07).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x08, 0x0b).rw("ppi8255_2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x50, 0x51).w("ymsnd", FUNC(ym3812_device::write));
	map(0x0e, 0x0e).w("crtc", FUNC(mc6845_device::address_w));
	map(0x0f, 0x0f).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
//  map(0xc0, 0xc0).w(FUNC(amaticmg_state::rombank_w));
	map(0xe6, 0xe6).w(FUNC(amaticmg_state::nmi_mask_w));
}


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
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE2 )       PORT_NAME("Service B (Dienst B") PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )          PORT_NAME("Coin 2 (Muenze 2)")   PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )          PORT_NAME("Hopper Payout pulse") PORT_IMPULSE(3)      PORT_CODE(KEYCODE_Q)  // Hopper paying pulse
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // 'Ausgegeben 0 - Hopper Leer' (spent 0 - hopper empty)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )    PORT_NAME("Hold 3 (Halten 3)")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )    PORT_NAME("Hold 2 (Halten 2)")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Clear / Take (Loeschen)")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Hold 1 (Halten 1)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )       PORT_NAME("Service A (Dienst A") PORT_TOGGLE
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BET )     PORT_NAME("Bet (Setzen) / Half Take")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE3 )       PORT_NAME("Service C (Dienst C") PORT_TOGGLE
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Service (Master)")
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
	{ 3, 2, 1, 0 }, // tiles are x-flipped
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*4*2
};


/************************************
*    Graphics Decode Information    *
************************************/

static GFXDECODE_START( gfx_amaticmg )
	GFXDECODE_ENTRY( "chars", 0x0000, charlayout_4bpp, 0, 0x20 )
GFXDECODE_END

static GFXDECODE_START( gfx_amaticmg2 )
	GFXDECODE_ENTRY( "chars", 0x0000, charlayout_6bpp, 0, 0x10000/0x40 )
GFXDECODE_END


/************************************
*       Machine Start & Reset       *
************************************/

void amaticmg_state::machine_start()
{
	uint8_t *rombank = memregion("maincpu")->base();

	m_rombank->configure_entries(0, 0x10, &rombank[0x8000], 0x4000);

	m_lamps.resolve();

	save_item(NAME(m_nmi_mask));
}

void amaticmg_state::machine_reset()
{
	m_rombank->set_entry(0);
	m_nmi_mask = 0;
}


/************************************
*          Machine Drivers          *
************************************/

void amaticmg_state::amaticmg(machine_config &config)
{
	constexpr XTAL MASTER_CLOCK = XTAL(16'000'000);
	constexpr XTAL CPU_CLOCK = MASTER_CLOCK / 4; // guess
	constexpr XTAL SND_CLOCK = MASTER_CLOCK / 4; // guess
	constexpr XTAL CRTC_CLOCK = MASTER_CLOCK / 8; // guess

	// basic machine hardware
	Z80(config, m_maincpu, CPU_CLOCK);     // WRONG!
	m_maincpu->set_addrmap(AS_PROGRAM, &amaticmg_state::amaticmg_map);
	m_maincpu->set_addrmap(AS_IO, &amaticmg_state::amaticmg_portmap);

//  NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// 3x 8255
	i8255_device &ppi0(I8255A(config, "ppi8255_0"));
	ppi0.in_pa_callback().set_ioport("IN0");
	ppi0.in_pb_callback().set_ioport("IN1");
	ppi0.in_pc_callback().set_ioport("IN2");

	i8255_device &ppi1(I8255A(config, "ppi8255_1"));
	ppi1.out_pa_callback().set(FUNC(amaticmg_state::out_a_w));
	ppi1.in_pb_callback().set_ioport("SW1");
	ppi1.out_pc_callback().set(FUNC(amaticmg_state::out_c_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 256);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(amaticmg_state::screen_update_amaticmg));

	mc6845_device &crtc(MC6845(config, "crtc", CRTC_CLOCK));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(4);
	crtc.out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_NMI); // no NMI mask?

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_amaticmg);
	PALETTE(config, m_palette, FUNC(amaticmg_state::amaticmg_palette), 0x200);

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	YM3812(config, "ymsnd", SND_CLOCK).add_route(ALL_OUTPUTS, "speaker", 0.5); // Y3014B DAC
}


void amaticmg_state::amaticmg2_irq(int state)
{
	if (state && m_nmi_mask)
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}


void amaticmg_state::amaticmg2(machine_config &config)
{
	amaticmg(config);

	m_maincpu->set_addrmap(AS_IO, &amaticmg_state::amaticmg2_portmap);

	I8255A(config, "ppi8255_2"); // MG4: 0x89 -> A:out; B:out; C(h):in; C(l):in.

	subdevice<screen_device>("screen")->set_screen_update(FUNC(amaticmg_state::screen_update_amaticmg2));

	subdevice<mc6845_device>("crtc")->out_vsync_callback().set(FUNC(amaticmg_state::amaticmg2_irq));

	m_gfxdecode->set_info(gfx_amaticmg2);
	m_palette->set_init(FUNC(amaticmg_state::amaticmg2_palette));
	m_palette->set_entries(0x10000);

	DS1994(config, "touch_m");
}

void amaticmg_state::amaticmg4(machine_config &config)
{
	amaticmg(config);

	m_maincpu->set_addrmap(AS_IO, &amaticmg_state::amaticmg4_portmap);

	I8255A(config, "ppi8255_2"); // MG4: 0x89 -> A:out; B:out; C(h):in; C(l):in.

	subdevice<screen_device>("screen")->set_screen_update(FUNC(amaticmg_state::screen_update_amaticmg2));

	subdevice<mc6845_device>("crtc")->out_vsync_callback().set(FUNC(amaticmg_state::amaticmg2_irq));

	m_gfxdecode->set_info(gfx_amaticmg2);
	m_palette->set_init(FUNC(amaticmg_state::amaticmg2_palette));
	m_palette->set_entries(0x10000);
}


/************************************
*             Rom Load              *
************************************/

ROM_START( suprstar )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x20000, "mainprg", 0 ) // encrypted program ROM
	ROM_LOAD( "u3.bin",  0x00000, 0x20000, CRC(29bf4a95) SHA1(a73873f7cd1fdf5accc3e79f4619949f261400b8) )

	ROM_REGION( 0x10000, "chars", 0 )
	ROM_LOAD( "u10.bin", 0x00000, 0x08000, CRC(6a811c81) SHA1(af01cd9b1ce6aca92df71febb05fe216b18cf42a) )
	ROM_CONTINUE(        0x00000, 0x08000 )
	ROM_LOAD( "u9.bin",  0x08000, 0x08000, CRC(823a736a) SHA1(a5227e3080367736aac1198d9dbb55efc4114624) )
	ROM_CONTINUE(        0x08000, 0x08000 )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "n82s147a.bin", 0x0000, 0x0200, CRC(dfeabd11) SHA1(21e8bbcf4aba5e4d672e5585890baf8c5bc77c98) )
ROM_END


//******** Multi Game sets ********

ROM_START( am_mg24 )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x40000, "mainprg", 0 ) // encrypted program ROM
	ROM_LOAD( "mgi_vger_3.9-i-8201.i6.bin", 0x00000, 0x40000, CRC(9ce159f7) SHA1(101c277d579a69cb03f879288b2cecf838cf1741) )

	ROM_REGION( 0x180000, "chars", 0 )
	ROM_LOAD( "multi_2.4_zg1.i17.bin", 0x100000, 0x80000, CRC(4a60a718) SHA1(626991abee768da58e87c7cdfc4fcbae86c6ea2a) )
	ROM_LOAD( "multi_2.4_zg2.i18.bin", 0x080000, 0x80000, CRC(b504e1b8) SHA1(ffa17a2c212eb2fffb89b131868e69430cb41203) )
	ROM_LOAD( "multi_2.4_zg3.i33.bin", 0x000000, 0x80000, CRC(9b66bb4d) SHA1(64035d2028a9b68164c87475a1ec9754453ad572) )

	ROM_REGION( 0x4000, "proms", 0 )
	ROM_LOAD( "m2061295.bin", 0x0000, 0x1c00, CRC(05f4a6af) SHA1(b14e9c80d3313fa5bf076d129a509a711d80f982) )
	ROM_LOAD( "m2080196.bin", 0x2000, 0x1c00, CRC(8cf6c3a6) SHA1(6454077c2ab94093e878cbc1c0102bbb6c4bc367) )

	ROM_REGION( 0x0248, "touch_m", 0 )
	ROM_LOAD( "ds1994.bin", 0x0000, 0x0248, CRC(7f581301) SHA1(33b2652f053a5e09442ccaa078b5d245255bb415) )
ROM_END

/*
  1x 40-pin custom CPU labeled:

     Amatic Trading GMBH
     Lfnd. Nr. 0940
     Type:     801 L
     Datum:    11.12.95
*/
ROM_START( am_mg24a )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x40000, "mainprg", 0 ) // encrypted program ROM
	ROM_LOAD( "multi_stm_27_cl_8202.bin", 0x00000, 0x40000, CRC(e3625367) SHA1(cea3ae4042522c720119ea94c8f05f74cbcdcab0) )

	ROM_REGION( 0x180000, "chars", 0 )
	ROM_LOAD( "multi_2.4_zg1.bin", 0x100000, 0x80000, CRC(29c3a45b) SHA1(97157a4d436a3dc8b81ffd7eb51f96f3bd969f4b) )  // this one doesn't match the parent.
	ROM_LOAD( "multi_2.4_zg2.bin", 0x080000, 0x80000, CRC(b504e1b8) SHA1(ffa17a2c212eb2fffb89b131868e69430cb41203) )  // identical to the parent.
	ROM_LOAD( "multi_2.4_zg3.bin", 0x000000, 0x80000, CRC(9b66bb4d) SHA1(64035d2028a9b68164c87475a1ec9754453ad572) )  // identical to the parent.

	ROM_REGION( 0x20000/*0x0400*/, "proms", 0 )
	ROM_LOAD( "n82s147n_1.bin", 0x0000, 0x0200, CRC(08e304e3) SHA1(e6f7cda9a626bb4b123889446dac9807983fa8c1) )
	ROM_LOAD( "n82s147n_2.bin", 0x0200, 0x0200, BAD_DUMP CRC(c962a66d) SHA1(d93aa03a9aa5cd93131e830c1221da5366662474) )
ROM_END


//******** MG III ********

ROM_START( am_mg3 )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x40000, "mainprg", 0 ) // encrypted program ROM
	ROM_LOAD( "mg_iii_vger_3.5-i-8205.bin", 0x00000, 0x40000, CRC(21d64029) SHA1(d5c3fde02833a96dd7a43481a489bfc4a5c9609d) )

	ROM_REGION( 0x180000, "chars", 0 )
	ROM_LOAD( "mg_iii_51_zg1.bin", 0x100000, 0x80000, CRC(84f86874) SHA1(c483a50df6a9a71ddfdf8530a894135f9b852b89) )
	ROM_LOAD( "mg_iii_51_zg2.bin", 0x080000, 0x80000, CRC(4425e535) SHA1(726c322c5d0b391b82e49dd1797ebf0abfa4a65a) )
	ROM_LOAD( "mg_iii_51_zg3.bin", 0x000000, 0x80000, CRC(36d4c0fa) SHA1(20352dbbb2ce2233be0f4f694ddf49b8f5d6a64f) )

	ROM_REGION( 0x20000, "proms", 0 )
	ROM_LOAD( "v.bin", 0x00000, 0x20000, CRC(524767e2) SHA1(03a108494f42365c820fdfbcba9496bda86f3081) )
ROM_END

ROM_START( am_mg3a )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x40000, "mainprg", 0 ) // encrypted program ROM
	ROM_LOAD( "mg_iii_vger_3.64_v_8309.i16", 0x00000, 0x40000, CRC(c54f97c4) SHA1(d5ce91be7332ada304d18d07706e3b98ac0fa74b) )

	ROM_REGION( 0x180000, "chars", 0 )
	ROM_LOAD( "mg_iii_51_zg1.i17", 0x100000, 0x80000, CRC(84f86874) SHA1(c483a50df6a9a71ddfdf8530a894135f9b852b89) )
	ROM_LOAD( "mg_iii_51_zg2.i18", 0x080000, 0x80000, CRC(4425e535) SHA1(726c322c5d0b391b82e49dd1797ebf0abfa4a65a) )
	ROM_LOAD( "mg_iii_51_zg3.i19", 0x000000, 0x80000, CRC(36d4c0fa) SHA1(20352dbbb2ce2233be0f4f694ddf49b8f5d6a64f) )

	ROM_REGION( 0x20000, "proms", 0 )
	ROM_LOAD( "iv.i35", 0x00000, 0x20000, CRC(82af7296) SHA1(1a07d6481e0f8fd785be9f1b737182d7e0b84605) )
ROM_END

// Italian sets...

ROM_START( am_mg31i )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x40000, "mainprg", 0 ) // encrypted program ROM
	ROM_LOAD( "mgi_sita_3.1_o_8270.bin", 0x00000, 0x40000, CRC(7358bdde) SHA1(674b57ddaaaed9b88ad563762b2421be7057e498) )

	ROM_REGION( 0x180000, "chars", 0 )
	ROM_LOAD( "mg2_ita.1", 0x100000, 0x80000, BAD_DUMP CRC(8663ce10) SHA1(00606bc69bd3a81a2f1b618d018d3ac315169fe4) )
	ROM_LOAD( "mg2_ita.2", 0x080000, 0x80000, BAD_DUMP CRC(7dbaf752) SHA1(afefbd239519abb4898348a3923ff093e36fbcb0) )
	ROM_LOAD( "mg2_ita.3", 0x000000, 0x80000, BAD_DUMP CRC(5dba55cf) SHA1(d237f8b3c72e8b59974059156070d0618ec41e9a) )

	ROM_REGION( 0x4000, "proms", 0 )
	ROM_LOAD( "ama80003_fprom.bin", 0x0000, 0x4000, BAD_DUMP CRC(65a784b8) SHA1(bd23136261e22f0294cff90040f3015ba0c10d7e) )
ROM_END

ROM_START( am_mg33i )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x40000, "mainprg", 0 ) // encrypted program ROM
	ROM_LOAD( "mgi_sita_3.3_o_8270.bin", 0x00000, 0x40000, CRC(eaa1ed83) SHA1(e50d06ea3631bd6e4f5fe14d8283c3550b2779a6) )

	ROM_REGION( 0x180000, "chars", 0 )
	ROM_LOAD( "mg2_ita.1", 0x100000, 0x80000, BAD_DUMP CRC(8663ce10) SHA1(00606bc69bd3a81a2f1b618d018d3ac315169fe4) )
	ROM_LOAD( "mg2_ita.2", 0x080000, 0x80000, BAD_DUMP CRC(7dbaf752) SHA1(afefbd239519abb4898348a3923ff093e36fbcb0) )
	ROM_LOAD( "mg2_ita.3", 0x000000, 0x80000, BAD_DUMP CRC(5dba55cf) SHA1(d237f8b3c72e8b59974059156070d0618ec41e9a) )

	ROM_REGION( 0x4000, "proms", 0 )
	ROM_LOAD( "ama80003_fprom.bin", 0x0000, 0x4000, BAD_DUMP CRC(65a784b8) SHA1(bd23136261e22f0294cff90040f3015ba0c10d7e) )
ROM_END

ROM_START( am_mg34i )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x40000, "mainprg", 0 ) // encrypted program ROM
	ROM_LOAD( "mgi_sita_3.4_o_8270.bin", 0x00000, 0x40000, CRC(bea7cd25) SHA1(89c9e02b48f34b2168e8624e552ead476cc339b9) )

	ROM_REGION( 0x180000, "chars", 0 )
	ROM_LOAD( "mg2_ita.1", 0x100000, 0x80000, BAD_DUMP CRC(8663ce10) SHA1(00606bc69bd3a81a2f1b618d018d3ac315169fe4) )
	ROM_LOAD( "mg2_ita.2", 0x080000, 0x80000, BAD_DUMP CRC(7dbaf752) SHA1(afefbd239519abb4898348a3923ff093e36fbcb0) )
	ROM_LOAD( "mg2_ita.3", 0x000000, 0x80000, BAD_DUMP CRC(5dba55cf) SHA1(d237f8b3c72e8b59974059156070d0618ec41e9a) )

	ROM_REGION( 0x4000, "proms", 0 )
	ROM_LOAD( "ama80003_fprom.bin", 0x0000, 0x4000, BAD_DUMP CRC(65a784b8) SHA1(bd23136261e22f0294cff90040f3015ba0c10d7e) )
ROM_END

ROM_START( am_mg35i )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x40000, "mainprg", 0 ) // encrypted program ROM
	ROM_LOAD( "mgi_sita_3.5_o_8270.bin", 0x00000, 0x40000, CRC(816eb41e) SHA1(0cad597e764455011d03f519e4adafb310e75451) )

	ROM_REGION( 0x180000, "chars", 0 )
	ROM_LOAD( "mg2_ita.1", 0x100000, 0x80000, BAD_DUMP CRC(8663ce10) SHA1(00606bc69bd3a81a2f1b618d018d3ac315169fe4) )
	ROM_LOAD( "mg2_ita.2", 0x080000, 0x80000, BAD_DUMP CRC(7dbaf752) SHA1(afefbd239519abb4898348a3923ff093e36fbcb0) )
	ROM_LOAD( "mg2_ita.3", 0x000000, 0x80000, BAD_DUMP CRC(5dba55cf) SHA1(d237f8b3c72e8b59974059156070d0618ec41e9a) )

	ROM_REGION( 0x4000, "proms", 0 )
	ROM_LOAD( "ama80003_fprom.bin", 0x0000, 0x4000, BAD_DUMP CRC(65a784b8) SHA1(bd23136261e22f0294cff90040f3015ba0c10d7e) )
ROM_END


//******** MG IV ********

ROM_START( am_mg4v )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x40000, "mainprg", 0 ) // encrypted program ROM
	ROM_LOAD( "mg_iv_vger_3__3.44_v_8373.bin", 0x00000, 0x40000, CRC(fab3aa28) SHA1(889870ca6ebfb0361e74803b7b50ff78c5e0df46) )

	ROM_REGION( 0x180000, "chars", 0 )
	ROM_LOAD( "mg_iv_vger_3__zg_1.bin", 0x100000, 0x80000, CRC(da9a1282) SHA1(15c6e4b460184603108d79bc104b8a0d036aad43) )
	ROM_LOAD( "mg_iv_vger_3__zg_2.bin", 0x080000, 0x80000, CRC(98dc36ed) SHA1(53ee317198730a8b34fec51f9fa237f980424fb9) )
	ROM_LOAD( "mg_iv_vger_3__zg_3.bin", 0x000000, 0x80000, CRC(1525c235) SHA1(b873f0b8dc8537558e43b37c95b5663fbddc09cc) )

	ROM_REGION( 0x20000, "proms", 0 )
	ROM_LOAD( "v.bin", 0x00000, 0x20000, CRC(77c82358) SHA1(a126aa123523965b62503ffd1ee99afaad7c77a1) )
ROM_END


// TYPE: 80-O - AMA8000-2  board
ROM_START( am_mg4sk )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x40000, "mainprg", 0 ) // encrypted program ROM
	ROM_LOAD( "m27c2001.i6", 0x00000, 0x40000, CRC(add64d33) SHA1(6779412d9ef2333d417128b2d5ed2a18c17f7ac6) ) // unwritten yellow label

	ROM_REGION( 0x180000, "chars", 0 )
	ROM_LOAD( "mg iv 4.1 - zg 1.i17", 0x100000, 0x80000, CRC(a28fbf91) SHA1(9f6e35af6b5f840a5f36cdd098e57ea7635d3da5) )
	ROM_LOAD( "mg iv 4.2 - zg 2.i18", 0x080000, 0x80000, CRC(e018b1e9) SHA1(5c0046e8c5cafcf59ff1b24e827f9b07b49ef9f9) )
	ROM_LOAD( "mg iv 4.3 - zg 3.i33", 0x000000, 0x80000, CRC(25eb9d1f) SHA1(eda6702ca1b1e565831f14caa4939f506d251d46) )

	ROM_REGION( 0x20000, "proms", 0 )
	ROM_LOAD( "iv.i35", 0x00000, 0x20000, CRC(82af7296) SHA1(1a07d6481e0f8fd785be9f1b737182d7e0b84605) )
ROM_END


//******** MG V ********

/*
  Multi Game 5.
  PCB: AMA 8000-2
  Program: AMGHU_VB3.65

  ROMs:
  OMH (program): 27C020.
  MG5 ZG1: 27C4001.
  MG5 ZG2: 27C4001.
  MG5 ZG3: 27C4000DC.
  1BFF: AM27C1024.

  -------------------

  Notes about the Dallas device...

  Name: DS1994
  Alternate Names: DS2404, Time-in-a-can, DS1427

  Device Address: CD00000015923304 (04 33 92 15 00 00 00 CD)

  Description: 4096 bit read/write nonvolatile memory partitioned
               into sixteen pages of 256 bits each and a real time
               clock/calendar in binary format.

  SCRATCHPAD:
  Page0 (0H)
  9B 92 93 94 7D 95 97 98 99 9A 91 9C 9D 9E 9F A0
  A1 A2 A3 A4 A5 A6 A7 A8 A9 AA 26 CE 8E B3 82 BF

  MAIN MEMORY:
  Page0 (0h)
  58 47 E8 6A 58 36 8B 79 54 3A 87 7D 2A 29 28 17
  16 15 14 13 CE 80 A7 81 80 AB 4E 34 AD AE AC AD
  Page1 (20h)
  B9 92 93 94 95 96 97 98 99 9A 9B 9C 9D 9E 9F A0
  A1 A2 A3 A4 A5 A6 A7 A8 A9 AA AB AC AD AE 88 BD
  Page2 (40h)
  91 92 93 94 95 96 97 98 99 9A 9B 9C 89 9E 9F A0
  A1 A2 A3 A4 A5 A6 A7 A8 A9 AA AB AC AD AE BC BD
  Page3 (60H)
  91 92 93 94 95 96 97 98 99 9A 9B 9C 9D 9E 9F A0
  A1 A2 A3 A4 A5 A6 A7 A8 A9 AA AB AC AD AE 50 BC
  Page4 (80H)
  91 92 93 94 95 96 97 98 99 9A 9B 9C 9D 9E 9F A0
  A1 A2 A3 A4 A5 A6 A7 A8 A9 AA AB AC AD AE 50 BC
  Page5 (A0H)
  B7 92 92 95 83 95 93 9D B1 9A 9B 9C 89 9E 9F A0
  A1 A2 A3 A4 A5 A6 A7 A8 A9 AA AB AC AD AE 2A BD
  Page6 (C0H)
  91 92 92 9E D3 97 94 98 99 9A 9A 9D 9C 76 9C A0
  A1 A2 A3 A4 A5 C2 27 A2 A9 5E AA AD AD AE 89 A0
  Page7 (E0H)
  9B 92 93 94 7D 95 97 98 99 9A 91 9C 9D 9E 9F A0
  A1 A2 A3 A4 A5 A6 A7 A8 A9 AA 26 CE 8E B3 82 BF
  Page8 (100H)
  E6 E5 E4 E3 E2 E1 E0 EF EE ED EC EB EA E9 E8 D7
  D6 D5 D4 D3 D2 D1 D0 DF A9 AA AB AC AD AE 88 A8
  Page9 (120H)
  E6 E5 E4 E3 E2 E1 E0 EF EE ED EC EB EA E9 E8 D7
  D6 D5 D4 D3 D2 D1 D0 DF A9 AA AB AC AD AC 86 A8
  Page10 (140H)
  DD 57 5E A4 BF 97 97 99 99 09 70 BD 50 93 B5 61
  4C E9 30 36 E1 4D 36 2A 83 AA AB AC AD AE C9 A7
  Page11 (160H)
  25 BA 94 7A 17 BD 79 1A A3 B0 73 1F 75 1D B4 C1
  ED 4B 93 92 44 EB 91 82 2A 00 00 00 00 00 14 AB
  Page12 (180H)
  91 92 93 94 95 96 97 98 99 9A 9B 9C 9D 9E 9F A0
  A1 A2 A3 A4 A5 A6 A7 A8 A9 AA AB AC AD AE AF B0
  Page13 (1A0H)
  1E 10 B9 19 17 5B 10 D4 C0 CA E8 83 E8 83 2B 61
  4C E9 30 36 E1 4D 36 2A 83 AA AB AC AD AE 5B A8
  Page14 (1C0H)
  91 92 93 94 95 96 97 98 99 9A 9B 9C 9D 9E 9F A0
  A1 A2 A3 A4 A5 A6 A7 A8 A9 AA AB AC AD AE AF B0
  Page15 (1E0H)
  B2 AE 6C BE A8 AB AB 95 A5 A6 A7 FC 52 A0 A3 91
  97 C7 9A 0C AE 19 C0 16 94 87 97 EA 91 93 07 A6

  CLOCK/ALARM REGISTERS:
  38 10 E8 DA 1D 22 00 D2 DD F0 28 1D 52 16 00 00
  8E 58 00 00 48 88 0A 12 81 26 82 00 12 0D FF FF

  - Todo: Construct & hook the device.

*/
ROM_START( am_mg5hu )
	ROM_REGION( 0x40000, "maincpu", ROMREGION_ERASE00 )

	ROM_REGION( 0x40000, "mainprg", 0 ) // encrypted program ROM
	ROM_LOAD( "omh.bin", 0x00000, 0x40000, CRC(e68522df) SHA1(b53ef40ee65df855b4dc843119a2337fa0a39d6e) )

	ROM_REGION( 0x180000, "chars", 0 )
	ROM_LOAD( "mg5_zg1.bin", 0x100000, 0x80000, CRC(e3c0e0a5) SHA1(9c672f49bf10dd96f9dc6eb9ca58aaba93576764) )
	ROM_LOAD( "mg5_zg2.bin", 0x080000, 0x80000, CRC(76bbce77) SHA1(cc11efb151e749040ca69c4e91e7adaa992577ce) )
	ROM_LOAD( "mg5_zg3.bin", 0x000000, 0x80000, CRC(2a78f9b5) SHA1(eb46c2da70a0aba4d1f93b99f39a2d3d594cb758) )

	ROM_REGION( 0x20000, "proms", 0 )
	ROM_LOAD( "1bff.bin", 0x00000, 0x20000, CRC(99d1750e) SHA1(22d000e358ed236a42d927d0b3e01d8c6e5c31d9) )
ROM_END


/************************************
*       Driver Initialization       *
************************************/

void amaticmg_state::encf(uint8_t ciphertext, int address, uint8_t &plaintext, int &newaddress)
{
	int aux = address & 0xfff;
	aux = aux ^ (aux >> 6);
	aux = ((aux << 6) | (aux >> 6)) & 0xfff;
	uint8_t aux2 = bitswap<8>(aux, 9, 10, 4, 1, 6, 0, 7, 3);
	aux2 ^= aux2 >> 4;
	aux2 = (aux2 << 4) | (aux2 >> 4);
	ciphertext ^= ciphertext << 4;
	plaintext = (ciphertext << 4) | (ciphertext >> 4);
	plaintext ^= aux2;
	newaddress = (address & ~0xfff) | aux;
}

void amaticmg_state::decrypt(int key1, int key2)
{
	uint8_t plaintext;
	int newaddress;

	uint8_t *src = memregion("mainprg")->base();
	uint8_t *dest = memregion("maincpu")->base();
	int len = memregion("mainprg")->bytes();

	for (int i = 0; i < len; i++)
	{
		encf(src[i], i, plaintext, newaddress);
		dest[newaddress ^ (key1 ^ (key1 >> 6))] = plaintext ^ key2;
	}
}

void amaticmg_state::init_ama8000_1_x()
{
	decrypt(0x4d1, 0xf5);
}

void amaticmg_state::init_ama8000_2_i()
{
	decrypt(0x436, 0x55);
}

void amaticmg_state::init_ama8000_2_v()
{
	decrypt(0x703, 0xaf);
}

void amaticmg_state::init_ama8000_3_o()
{
	decrypt(0x56e, 0xa7);
}

void amaticmg_state::init_am_mg5hu()
{
	decrypt(0x540, 0xa6);
}

} // anonymous namespace


/************************************
*           Game Drivers            *
************************************/

/*     YEAR  NAME      PARENT    MACHINE    INPUT     STATE           INIT              ROT    COMPANY                FULLNAME                        FLAGS                                                                                                                       LAYOUT */
GAMEL( 1996, suprstar, 0,        amaticmg,  amaticmg, amaticmg_state, init_ama8000_1_x, ROT90, "Amatic Trading GmbH", "Super Stars",                  MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING,                                                                              layout_suprstar )
GAME(  2000, am_mg24,  0,        amaticmg2, amaticmg, amaticmg_state, init_ama8000_2_i, ROT0,  "Amatic Trading GmbH", "Multi Game I (V.Ger 2.4)",     MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME(  2000, am_mg24a, 0,        amaticmg2, amaticmg, amaticmg_state, init_ama8000_1_x, ROT0,  "Amatic Trading GmbH", "Multi Game I (V.Stm 2.7)",     MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME(  2000, am_mg3,   0,        amaticmg2, amaticmg, amaticmg_state, init_ama8000_2_i, ROT0,  "Amatic Trading GmbH", "Multi Game III (V.Ger 3.5)",   MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME(  2000, am_mg3a,  0,        amaticmg4, amaticmg, amaticmg_state, init_ama8000_2_v, ROT0,  "Amatic Trading GmbH", "Multi Game III (V.Ger 3.64)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME(  2000, am_mg35i, 0,        amaticmg2, amaticmg, amaticmg_state, init_ama8000_3_o, ROT0,  "Amatic Trading GmbH", "Multi Game III (S.Ita 3.5)",   MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME(  2000, am_mg34i, am_mg35i, amaticmg2, amaticmg, amaticmg_state, init_ama8000_3_o, ROT0,  "Amatic Trading GmbH", "Multi Game III (S.Ita 3.4)",   MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME(  2000, am_mg33i, am_mg35i, amaticmg2, amaticmg, amaticmg_state, init_ama8000_3_o, ROT0,  "Amatic Trading GmbH", "Multi Game III (S.Ita 3.3)",   MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME(  2000, am_mg31i, am_mg35i, amaticmg2, amaticmg, amaticmg_state, init_ama8000_3_o, ROT0,  "Amatic Trading GmbH", "Multi Game III (S.Ita 3.1)",   MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME(  2000, am_mg4v,  0,        amaticmg4, amaticmg, amaticmg_state, init_ama8000_2_v, ROT0,  "Amatic Trading GmbH", "Multi Game IV (V.Ger 3.44)",   MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME(  2003, am_mg4sk, 0,        amaticmg2, amaticmg, amaticmg_state, init_ama8000_3_o, ROT0,  "Amatic Trading GmbH", "Multi Game IV (AMGSK_VA3.85)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME(  2001, am_mg5hu, 0,        amaticmg4, amaticmg, amaticmg_state, init_am_mg5hu,    ROT0,  "Amatic Trading GmbH", "Multi Game V (AMGHU_VB3.65)",  MACHINE_IMPERFECT_GRAPHICS | MACHINE_WRONG_COLORS | MACHINE_UNEMULATED_PROTECTION | MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
