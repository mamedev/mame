// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Palindrome, FraSher, Roberto Fresca
/***********************************************************************************************************************************************

    Driver: aristmk4

    Manufacturer: Aristocrat Leisure Industries ( aka Ainsworth Nominees P.L. )
    Platform: Aristocrat 540 Video ( MK 2.5 Video / MK IV )
    Driver by Palindrome & FraSher

    original 86lions.c driver by Chris Hardy, Angelo Salese & Roberto Fresca


    ***************** INITIALISATION *********************************************************************

    Method 1 :
    * Key in with the Jackpot Key followed by the Audit Key (K+L Keys)
    * Hit UP (W key) twice (for gunnrose only)
    * Press PB4, PB5 and PB6 keys simultaneously (Z+X+C keys by default)
    * A value (displayed below) will appear next to RF/AMT on the right of the screen
    * Key out both the Jackpot and Audit Keys

    This method works with the following games:
    3bagflnz 200
    3bagflvt 200
    autmoon  200
    blkrhino 200
    cgold    500
    coralr2  200
    eforesta 200
    eforestb 200
    ffortune 200
    fvrpitch 200
    gldnpkr  400
    goldenc  200
    gtroppo  500
    kgbird   200
    kgbirda  200
    phantomp 200
    swtht2nz 200
    wildone  200
    wtigernz 200
    gunnrose

    Method 2 :
    * Key in with the Jackpot Key followed by the Audit Key
    * Press PB4, PB5 and PB6 keys simultaneously (Z+X+C keys by default)
    * This will enter the cashcade screen and increment $100 to the maximum.
    * Press PLAY 2 LINES [listed as BET 2 on the screen] to increment the minimum cashcade value by $5.
      - (optionally, you can decrement with the PLAY 1 LINE [BET 1] button, but you must first increment
        the $5 to start with above or the game won't initialise)
    * A value (displayed below) will appear on the right as RF/AMT when you key in again (not visible
       until you key out and back in again with the Audit Key)
    * Key out both the Jackpot and Audit Keys

    This method works with the following games:
    topgear  500

    Method 3 :
    * cgold2, fhunter and fhuntera ONLY: DIP labeled "5201-5" switch to ON
       This allows setup procedure to complete properly and game to play (if disabled, it acts as a
       'freeze' switch and the games don't accept inputs).
    * Key in with the Jackpot Key followed by the Audit Key.
    * Press PB4, PB5 and PB6 keys simultaneously (Z+X+C keys by default)
    * Press Service (default A) 4 times until you are in the Setup Screen, with Printer Pay Limit.
    * Press Bet 2 (default D) to change the Jackpot Win Limit. A higher value is better (3000 max)
    * Key out both the Jackpot and Audit Keys

    This method works with the following games:
    arcwins
    cgold2
    eforest
    fhunter
    fhuntera


**********************************************************************************************************

    Technical Notes:

    68B09EP Motorola Processor
    R6545AP for CRT video controller
    UPD43256BCZ-70LL for 32kb of static ram used for 3 way electronic meters / 3 way memory
    U6264A for Standard 8K x 8 bit SRAM used for video buffer
    1 x R65C21P2  PIA - Peripheral Interface Adapter, connects to RTC and sends pulses to mechanical meters
    1 x 6522 VIA - 1 x Rockwell - Versatile Interface Adapter.
    2 x WF19054 = AY3-8910 sound chips driven by the 6522 VIA
    1 x PML 2852 ( programmable logic ) used as address decoder.
    1 x PML 2852 programmed as a PIA

    PIA provides output signals to six mechanical meters.
    It also provides the real time clock DS1287 to the CPU.

    VIA drives the programmable sound generators and generates
    a timing interrupt to the CPU (M6809_FIRQ_LINE)

    The VIA uses Port A to write to the D0-D7 on the AY8910s. Port B hooks first 4 bits up to BC1/BC2/BDIR and A9 on AY1 and A8 on AY2
    The remaining 4 bits are connected to other hardware, read via the VIA.

    The AY8910 named ay1 has writes on PORT B to the ZN434 DA convertor.
    The AY8910 named ay2 has writes to lamps and the light tower on Port A and B. these are implemented via the layout


**********************************************************************************************************

    Updates....

    27/04/10 - FrasheR
    2 x Sound Chips connected to the 6522 VIA.

    16/05/10 - FrasheR
    Fixed VIA for good. 5010 - 501F.
    Hooked up push button inputs - FrasheR
    Hooked up ports for the PML 2852 U3 - FrasheR

    16/05/10 - Palindrome
    Lamp outputs and layout added - Palindrome
    NVRAM backup - Palindrome

    20/05/10 - Palindrome
    Connected SW7 for BGCOLOUR map select
    Added LK13. 3Mhz or 1.5 Mhz CPU speed select
    Added sound sample for mechanical meter pulse ( aristmk4.zip ).

    30/5/10 - Palindrome
    Now using mc146818 rtc driver instead of rtc_get_reg.

    The mc146818 driver has issues and is not working correctly.
    MESS developers are looking at it.

    - day of week is incorrect
    - day of month is incorrect ( code is using day instead of mday ).
    - hours are not showing up correct in PM and 12 hour mode
    - rtc causes game to freeze if the game is left in audit mode with continuous writes
       to 0xA reg - 0x80 data )

    9/7/2010 - Palindrome
    Robot Test added
    Default Jackpot key re-assigned to 'I'
    Work around for topgear & cashcade games
    Improved coininput - force CBOPT1 to detect passing coin
    Added new game Golden Poker ( Aristocrat version ) [ bad dump ]
    Added new game Gone Troppo
    Added new game Wild One
    Misc improvements

    12/12/2010 - Palindrome and Heihachi_73
    Updated source to 0.140u2 standards
    Disabled real time clock to stop games from hanging. This causes a graphics glitch
     on the month display but makes the games more reliable in audit mode.
    Fixed ROM names
    Added new game Arctic Wins
    Added new game Caribbean Gold 2 (missing 2 gfx roms, still boots)
    Added new game Clockwise (program ROM nodump, all other roms fine)
    Added new game Fortune Hunter (2 sets)

    06/06/2011 - Heihachi_73
    Added button panel artwork for all games, and renamed the in-game buttons to match
     the artwork and/or Robot Test description.
    Remapped Jackpot Key to 'L'
    Remapped 'power fail' key to ',' (comma)
    Remapped the video poker buttons; holds are now keys S,D,F,G,H
    Un-mapped the unused inputs

    06/08/2011 - FrasheR
    Implement 'Printer Fault' fix.

    08/08/2011 - FrasheR
    Implement Port '5005' for eforest. Changing this only works after performing memory reset.(delete nvram file)
    First 3 bits
    000 = $100 / Credit
    001 = 50c  / Credit
    010 = $5   / Credit
    011 = 10c  / Credit
    100 = $10  / Credit
    101 = 25c  / Credit
    110 = $1   / Credit
    111 = 5c   / Credit (default)
    Implement Bill Acceptor for eforest to add credits.
    arcwins and eforest are now working.

    21/02/2013 - Heihachi_73
    Added new game Caribbean Gold (cgold), however it is not a straight swap as it has slightly different input locations:
     - With unmodified 0.148 source, game complains about logic door being open,
        which is seemingly tied to the current coin input.
     - When HOPCO2 is toggled off/on quickly (default is on otherwise it will
        cause a note acceptor error), the note acceptor works, adding 4 credits ($1?).
        This is seemingly a quarter slot (25c). Not sure if other notes are possible.
     - Same gameplay as Gone Troppo, one interesting thing about this game is that
        the KQJ symbols have actual faces instead of plain letters.

    08/03/2013 - Heihachi_73
    Cleaned up comments and erroneous ROM names (e.g. graphics ROMs named after the program ROM).
    Caribbean Gold II - copied cgold graphics ROMs u8+u11 (aka u20+u45) to cgold2, game now playable.
     Tiles 0x64 and 0x65 are used to show the game's denomination (credit value), however cgold does
     not use these tiles (there are seemingly unused line/bet/number tiles in this location), this
     causes a minor glitch on the $/c sign. Tiles 0x277-0x288 also differ but are unused.
    Promoted Fortune Hunter and clone to working status, as they were in fact working for quite a while.
    Fixed ROM names for kgbird/kgbirda; 5c and 10c variants were mixed up.

    11/12/2014 - Lord-Data
    Added hopper and meter outputs.

    27/03/2014
    Added new game: Gun's and Roses Poker - gunnrose

    13/11/2015 - Roberto Fresca
    Added new game: Fever Pitch? (2VXEC534, NSW, 90.36%).
    Need to confirm the title.


*************************************************************************************************************

    When the games first power on (or when reset), they will display a TILT message on the screen.
    This doesn't affect gameplay, and if there are no pending errors the game should coin up and/or play immediately.

    The tilt message will also appear when an error code is displayed, such as the main door being opened/closed, or
    a hardware error/fault (such as hopper empty, coin yoyo, printer errors; none of which should happen in MAME however).

    The tilt message will disappear if you turn the Audit Key on and off, or after you start playing.
    Despite the name, there is no 'tilt' mechanism in the machine and there is nothing to worry about.
    The first Aristocrat system to have a tilt mechanism was the MK5, which will cause the machine to reset abruptly
    if the player is too rough (e.g. hitting the screen or bumping the machine).

    These games do not feature a backup mechanism in case of power faults or system crashes requiring a reboot;
    if the player was in the middle of a spin or watching a win count up, any credits won on that spin will be voided.
    On the machine's artwork, this is reflected with text reading 'Malfunction voids all pays and plays', of which
    the text has also been carried onto later machines. The Aristocrat MK5 and later systems however feature backup
    mechanisms and will repeat the last game (including free game features and/or gamble selection) when powered on,
    to where the player had left off.

    Gone Troppo and Caribbean Gold 1 and 2 require DIP SW7 to be set to off/off or else the second screen will be broken.
    This is possibly true to the original machine.

    A similar thing happens with Top Gear, the drag cars' tyres will only be the correct colour (grey) if SW7 is off/off.

    In Wild One, the dollar sign on the Insert $2 graphic is the wrong colour on other settings as well. It only appears
    correct when SW7 is off/off. This is probably a bug in the original game, where the graphic designers have used the
    wrong palette for the background of the dollar sign.

    From these findings, it is noted that the off/off setting may in fact be the default background setting of all games.

    cgold, gtroppo and topgear are non-multiplier, 5 payline games, therefore, you cannot bet higher than 5 credits on these machines.

    cgold can be set to credit play or coin play by toggling SW1-5. If SW1-5 is on, game is in credit mode; if SW1-5 is off,
    wins and remaining credits will be automatically paid out as coins.

    Non-US games can enable/disable the double up (gamble) option by toggling the SW1-8 switch. Turning SW1 off will enable
    the double up option (default); turning SW1-8 on will disable double up and enable auto-spin on some games (so far, only
    eforesta and 3bagflvt allow this; other games simply ignore the buttons). The games respond slightly faster between games
    with double up disabled.

    3 Bags Full, Fortune Hunter, Caribbean Gold 1 and 2, Gone Troppo and Top Gear do not have a double up option, and US-based
    games ignore this switch setting (double up is always enabled on US games which support it).


    TODO:

    1. ROMs need redumping for the following games:
     - White Tiger has bad graphics ROMs.
     - Caribbean Gold 2 is missing two graphics chips.
     - Clockwise needs its program ROM redumped, original dump was 32K of 0xFF's. Graphics and video/sound ROM are OK.
     - PROM dumps needed for Top Gear (2CM33), Caribbean Gold (1CM12), Caribbean Gold 2 (unknown), Clockwise (2CM18),
        Golden Poker (unknown), and Fever Pitch (unknown).

    2. Video poker and Keno button panels needed. 06/06/11: Video poker panels done, however they need confirmation
       with a real machine.

    3. Extend the driver to use the keno keyboard input for keno games (no MK2.5/MKIV Keno games dumped yet as of 28/02/2010).

    4. Provide complete cashcade emulation

    5. Look into what the hopper probe signal is for.

    6. Investigate issues with the Poker style games as described below.

    7. When DIP SW7 is set to off/off, speed is dramatically reduced (noticeable on older Pentium 4-based systems).

    8. rewrite video emulation by using custom drawing code.

    9. check what type of mc6845 this HW uses on real PCB, and hook it up properly.

    10. fix 86 Lions (pre-Aristocrat Mk-4 HW, without prom and dunno what else)


    ***************** POKER GAMES ************************************************************************

    Wild One & Golden Poker have a problem where the second branch condition is always true, see assembler below for
    example of Wild One.

    907D    BITA $1800  ( crtc )
    9080    BNE  $907D  ; is zero
    9082    BITA $1800
    9085    BEQ  $9082  ; branches to 9082 indefinitely, value is always zero.
    9087    LDA  #$40

    If the PC ( program counter ) is set to 9087 then the game runs.

    Bug in the 6845 crtc core ? Seems like some kind of logic there not working.

    EDIT: it's a vblank check, BITA opcode checks bit 5 in A register and compares it with the contents of 0x1800
    (that is vblank in mc6845_status_r). Checking if a bit goes low then high it usually means that is moaning for
    a vblank. ;-)
    But now there is a new question: what kind of mc6845 clone this HW uses? It's clearly not standard mc6845,
    since that version doesn't support vblank reading. The vblank bit can be read only on C6545-1, R6545-1, SY6545-1
    and SY6845E subvariants, so it all lies to those. -AS


***********************************************************************************************************************************************/

#define MAIN_CLOCK  XTAL_12MHz

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "video/mc6845.h"
#include "sound/ay8910.h"
#include "machine/6522via.h"
#include "machine/6821pia.h"
#include "machine/i8255.h"
#include "sound/samples.h"
#include "machine/mc146818.h" // DALLAS1287 is functionally compatible.
#include "machine/nvram.h"

// Button panel and lamps
#include "aristmk4.lh" // AU 90cr with gamble
#include "arimk4nz.lh" // NZ 45cr with double up
#include "3bagflnz.lh" // NZ 45cr without gamble
#include "3bagflvt.lh" // AU 90cr without gamble
#include "arcwins.lh"  // US 25cr with gamble
#include "cgold2.lh"   // US 25cr without gamble
#include "eforest.lh"  // US 45cr with gamble
#include "fhunter.lh"  // US 45cr without gamble
#include "goldenc.lh"  // NZ 90cr with double up
#include "kgbird.lh"   // NZ 25cr with double up
#include "topgear.lh"  // NZ 5 line without gamble
#include "wildone.lh"  // Video poker
#include "gunnrose.lh" // Video poker
#include "gldnpkr.lh"  // Video poker
#include "fvrpitch.lh"  // 5 line without gamble

UINT8 crtc_cursor_index = 0;
UINT8 crtc_reg = 0;

class aristmk4_state : public driver_device
{
public:
	aristmk4_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_rtc(*this, "rtc"),
		m_ay1(*this, "ay1"),
		m_ay2(*this, "ay2"),
		m_samples(*this, "samples"),
		m_mkiv_vram(*this, "mkiv_vram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")  { }

	required_device<cpu_device> m_maincpu;
	required_device<mc146818_device> m_rtc;
	required_device<ay8910_device> m_ay1;
	required_device<ay8910_device> m_ay2;
	required_device<samples_device> m_samples;

	required_shared_ptr<UINT8> m_mkiv_vram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	int m_rtc_address_strobe;
	int m_rtc_data_strobe;
	UINT8 *m_shapeRomPtr;
	UINT8 m_shapeRom[0xc000];
	std::unique_ptr<UINT8[]> m_nvram;
	UINT8 m_psg_data;
	int m_ay8910_1;
	int m_ay8910_2;
	int m_u3_p0_w;
	UINT8 m_cgdrsw;
	UINT8 m_ripple;
	int m_hopper_motor;
	int m_inscrd;
	int m_insnote;
	int m_cashcade_c;
	int m_printer_motor;
	DECLARE_READ8_MEMBER(ldsw);
	DECLARE_READ8_MEMBER(cgdrr);
	DECLARE_WRITE8_MEMBER(cgdrw);
	DECLARE_WRITE8_MEMBER(u3_p0);
	DECLARE_READ8_MEMBER(u3_p2);
	DECLARE_READ8_MEMBER(u3_p3);
	DECLARE_READ8_MEMBER(bv_p0);
	DECLARE_READ8_MEMBER(bv_p1);
	DECLARE_READ8_MEMBER(mkiv_pia_ina);
	DECLARE_WRITE8_MEMBER(mkiv_pia_outa);
	DECLARE_WRITE8_MEMBER(mlamps);
	DECLARE_READ8_MEMBER(cashcade_r);
	DECLARE_WRITE8_MEMBER(mk4_printer_w);
	DECLARE_READ8_MEMBER(mk4_printer_r);
	DECLARE_WRITE_LINE_MEMBER(mkiv_pia_ca2);
	DECLARE_WRITE_LINE_MEMBER(mkiv_pia_cb2);
	DECLARE_WRITE8_MEMBER(mkiv_pia_outb);
	DECLARE_READ8_MEMBER(via_a_r);
	DECLARE_READ8_MEMBER(via_b_r);
	DECLARE_WRITE8_MEMBER(via_a_w);
	DECLARE_WRITE8_MEMBER(via_b_w);
	DECLARE_WRITE_LINE_MEMBER(via_ca2_w);
	DECLARE_WRITE_LINE_MEMBER(via_cb2_w);
	DECLARE_WRITE8_MEMBER(pblp_out);
	DECLARE_WRITE8_MEMBER(pbltlp_out);
	DECLARE_WRITE8_MEMBER(zn434_w);
	DECLARE_READ8_MEMBER(pa1_r);
	DECLARE_READ8_MEMBER(pb1_r);
	DECLARE_READ8_MEMBER(pc1_r);
	DECLARE_DRIVER_INIT(aristmk4);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(aristmk4);
	DECLARE_PALETTE_INIT(lions);
	UINT32 screen_update_aristmk4(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(note_input_reset);
	TIMER_CALLBACK_MEMBER(coin_input_reset);
	TIMER_CALLBACK_MEMBER(hopper_reset);
	TIMER_DEVICE_CALLBACK_MEMBER(aristmk4_pf);
	inline void uBackgroundColour();
};

/* Partial Cashcade protocol */
static const UINT8 cashcade_p[] ={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf0};

void aristmk4_state::video_start()
{
	int tile;
	for (tile = 0; tile < m_gfxdecode->gfx(0)->elements(); tile++)
	{
		m_gfxdecode->gfx(0)->get_data(tile);
	}
}

void aristmk4_state::uBackgroundColour()
{
	/* SW7 can be set when the main door is open, this allows the colours for the background
	to be adjusted whilst the machine is running.

	There are 4 possible combinations for colour select via SW7, colours vary based on software installed.
	*/

	switch(ioport("SW7")->read())
	{
	case 0x00:
		// restore defaults
		memcpy(m_shapeRomPtr,m_shapeRom, sizeof(m_shapeRom)); // restore defaults, both switches off
											// OE enabled on both shapes
		break;
	case 0x01:
		// unselect U22 via SW7. OE on U22 is low.
		memset(&m_shapeRomPtr[0x4000],0xff,0x2000);          // fill unused space with 0xff
		memcpy(&m_shapeRomPtr[0xa000],&m_shapeRom[0xa000], 0x2000); // restore defaults here
		break;
	case 0x02:
		// unselect U47 via SW7. OE on U47 is low.
		memcpy(&m_shapeRomPtr[0x4000],&m_shapeRom[0x4000], 0x2000);
		memset(&m_shapeRomPtr[0xa000],0xff,0x2000);
		break;
	case 0x03:
		// unselect U47 & U22 via SW7. Both output enable low.
		memset(&m_shapeRomPtr[0x4000],0xff,0x2000);
		memset(&m_shapeRomPtr[0xa000],0xff,0x2000);
		break;
	}
}

UINT32 aristmk4_state::screen_update_aristmk4(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	int x,y;
	int count = 0;
	int color;
	int tile;
	int bgtile;
	int flipx;
	int flipy;

	for (y=27;y--;)
	{
		for (x=38;x--;)
		{
		color = ((m_mkiv_vram[count]) & 0xe0) >> 5;
			tile = (m_mkiv_vram[count+1]|m_mkiv_vram[count]<<8) & 0x3ff;
			bgtile = (m_mkiv_vram[count+1]|m_mkiv_vram[count]<<8) & 0xff; // first 256 tiles
			uBackgroundColour();   // read sw7
			gfx->mark_dirty(bgtile);    // force the machine to update only the first 256 tiles.
								// as we only update the background, not the entire display.
			flipx = ((m_mkiv_vram[count]) & 0x04);
			flipy = ((m_mkiv_vram[count]) & 0x08);
			gfx->opaque(bitmap,cliprect,tile,color,flipx,flipy,(38-x-1)<<3,(27-y-1)<<3);
			count+=2;
		}
	}
	return 0;
}

READ8_MEMBER(aristmk4_state::ldsw)
{
	int U3_p2_ret= ioport("5002")->read();
	if(U3_p2_ret & 0x1)
	{
	return 0;
	}
	return m_cgdrsw = ioport("5005")->read();
}

READ8_MEMBER(aristmk4_state::cgdrr)
{
	if(m_cgdrsw) // is the LC closed
	{
	return m_ripple; // return a positive value from the ripple counter
	}
	return 0x0; // otherwise the counter outputs are set low.
}

WRITE8_MEMBER(aristmk4_state::cgdrw)
{
	m_ripple = data;
}

WRITE8_MEMBER(aristmk4_state::u3_p0)
{
	m_u3_p0_w = data;

	if ((data&0x80)==0) //Printer Motor Off
	{
		m_printer_motor = 1; // Set this so the next read of u3_p3 returns PTRHOM as OFF.
	}

	//logerror("u3_p0_w: %02X\n",m_u3_p0_w);
}

READ8_MEMBER(aristmk4_state::u3_p2)
{
	int u3_p2_ret= ioport("5002")->read();
	int u3_p3_ret= ioport("5003")->read();

	output_set_lamp_value(19, (u3_p2_ret >> 4) & 1); //auditkey light
	output_set_lamp_value(20, (u3_p3_ret >> 2) & 1); //jackpotkey light

	if (m_u3_p0_w&0x20) // DOPTE on
	{
	if (u3_p3_ret&0x02) // door closed
		u3_p2_ret = u3_p2_ret^0x08; // DOPTI on
	}

	if (m_inscrd==0)
	{
		m_inscrd=ioport("insertcoin")->read();
	}

	if (m_inscrd==1)
		u3_p2_ret=u3_p2_ret^0x02;

	return u3_p2_ret;
}

READ8_MEMBER(aristmk4_state::u3_p3)
{
	int u3_p3_ret= ioport("5003")->read();

	if ((m_printer_motor)==1) // Printer Motor Off

	{
		u3_p3_ret = u3_p3_ret^0x80; // Printer Home Off
		m_printer_motor=0;

	}

	return u3_p3_ret;

}

TIMER_CALLBACK_MEMBER(aristmk4_state::note_input_reset)
{
	m_insnote=0; //reset note input after 150msec
}

READ8_MEMBER(aristmk4_state::bv_p0)
{
	int bv_p0_ret=0x00;

	switch(m_insnote)
	{
	case 0x01:
		bv_p0_ret=ioport("NS")->read()+0x81; //check note selector
		m_insnote++;
		break;
	case 0x02:
		bv_p0_ret=0x89;
		m_insnote++;
		machine().scheduler().timer_set(attotime::from_msec(150), timer_expired_delegate(FUNC(aristmk4_state::note_input_reset),this));
		break;
	default:
		break; //timer will reset the input
	}

	return bv_p0_ret;

}

READ8_MEMBER(aristmk4_state::bv_p1)
{
	int bv_p1_ret=0x00;

	if (m_insnote==0)
		m_insnote=ioport("insertnote")->read();

	if (m_insnote==1)
			bv_p1_ret=0x08;

	if (m_insnote==2)
			bv_p1_ret=0x08;

	return bv_p1_ret;

}


/******************************************************************************

PERIPHERAL INTERFACE ADAPTER CONFIGURATION

PORTA - DALLAS DS1287 RTC
PORTB - MECHANICAL METERS

******************************************************************************/

/*****************************************************************************/
/* DALLAS DS1287 OR mc146818
******************************************************************************/

//input a
READ8_MEMBER(aristmk4_state::mkiv_pia_ina)
{
	/* uncomment this code once RTC is fixed */

	//return m_rtc->read(space,1);
	return 0;   // OK for now, the aussie version has no RTC on the MB so this is valid.
}

//output a
WRITE8_MEMBER(aristmk4_state::mkiv_pia_outa)
{
	if(m_rtc_data_strobe)
	{
		m_rtc->write(space,1,data);
		//logerror("rtc protocol write data: %02X\n",data);
	}
	else
	{
		m_rtc->write(space,0,data);
		//logerror("rtc protocol write address: %02X\n",data);
	}
}

//output ca2
WRITE_LINE_MEMBER(aristmk4_state::mkiv_pia_ca2)
{
	m_rtc_address_strobe = state;
	// logerror("address strobe %02X\n", address_strobe);
}

//output cb2
WRITE_LINE_MEMBER(aristmk4_state::mkiv_pia_cb2)
{
	m_rtc_data_strobe = state;
	//logerror("data strobe: %02X\n", data);
}

//output b
WRITE8_MEMBER(aristmk4_state::mkiv_pia_outb)
{
	UINT8 emet[5];
	int i = 0;
	//pia_data = data;
	emet[0] = data & 0x01;  /* emet1  -  bit 1 - PB0 */
				/* seren1 -  bit 2 - PB1 */
	emet[1] = data & 0x04;  /* emet3  -  bit 3 - PB2 */
	emet[2] = data & 0x08;  /* emet4  -  bit 4 - PB3 */
	emet[3] = data & 0x10;  /* emet5  -  bit 5 - PB4 */
	emet[4] = data & 0x20;  /* emet6  -  bit 6 - PB5 */

	for(i = 0;i<sizeof(emet);i++)
	{
		if(emet[i])
		{
			//logerror("Mechanical meter %d pulse: %02d\n",i+1, emet[i]);
			// Output Physical Meters
			switch(i+1)
			{
				case 4:
					output_set_value("creditspendmeter", emet[i]);
					break;
				case 5:
					output_set_value("creditoutmeter", emet[i]);
					break;
				default:
					printf("Unhandled Mechanical meter %d pulse: %02d\n",i+1, emet[i]);
					break;
			}

			m_samples->start(i,0); // pulse sound for mechanical meters
		}
		else
		{
			// if there is not a value set, this meter is not active, reset output to 0
			switch(i+1)
			{
				case 4:
					output_set_value("creditspendmeter", 0);
					break;
				case 5:
					output_set_value("creditoutmeter", 0);
					break;
				default:
					break;
			}
		}
	}
}

/* sound interface for playing mechanical meter sound */

static const char *const meter_sample_names[] =
{
	"*3bagflvt",
	"tick",
	nullptr
};

/******************************************************************************

VERSATILE INTERFACE ADAPTER CONFIGURATION

******************************************************************************/

TIMER_CALLBACK_MEMBER(aristmk4_state::coin_input_reset)
{
	m_inscrd=0; //reset credit input after 150msec
}

TIMER_CALLBACK_MEMBER(aristmk4_state::hopper_reset)
{
	m_hopper_motor = 0x01;
	output_set_value("hopper_motor", m_hopper_motor);
}

// Port A read (SW1)
READ8_MEMBER(aristmk4_state::via_a_r)
{
	int psg_ret=0;

	if (m_ay8910_1&0x03) // SW1 read.
	{
		psg_ret = m_ay1->data_r(space, 0);
		//logerror("PSG porta ay1 returned %02X\n",psg_ret);
	}

	else if (m_ay8910_2&0x03) //i don't think we read anything from Port A on ay2, Can be removed once game works ok.
	{
		psg_ret = m_ay2->data_r(space, 0);
		//logerror("PSG porta ay2 returned %02X\n",psg_ret);
	}
	return psg_ret;
}

READ8_MEMBER(aristmk4_state::via_b_r)
{
	int ret=ioport("via_port_b")->read();

// Not expecting to read anything from port B on the AY8910's ( controls BC1, BC2 and BDIR )
// However there are extra 4 bits not going to the AY8910's on the schematics, which get read from here.
//   OPTA1  - Bit4 - Coin optics - A
//   OPTB1  - Bit5 - Coin optics - B
//   HOPCO1 - Bit6 - Hopper counter
//   CBOPT1 - Bit7 - Cash box optics
/* Coin input... CBOPT2 goes LOW, then the optic detectors OPTA1 / OPTB1 detect the coin passing */
/* The timer causes one credit, per 150ms or so... */

	switch(m_inscrd)
	{
	case 0x00:
		break;
	case 0x01:
		ret=ret^0x10;
		m_inscrd++;
		break;
	case 0x02:
		ret=ret^0x20;
		m_inscrd++;
		machine().scheduler().timer_set(attotime::from_msec(150), timer_expired_delegate(FUNC(aristmk4_state::coin_input_reset),this));
		break;
	default:
		break; //timer will reset the input
	}

// if user presses collect.. send coins to hopper.

	switch(m_hopper_motor)
	{
	case 0x00:
		ret=ret^0x40;
		machine().scheduler().timer_set(attotime::from_msec(175), timer_expired_delegate(FUNC(aristmk4_state::hopper_reset),this));
		m_hopper_motor=0x02;
		output_set_value("hopper_motor", m_hopper_motor);
		break;
	case 0x01:
		break; //default
	case 0x02:
		ret=ret^0x40;
		break;
	default:
		break;
	}
	return ret;
}

WRITE8_MEMBER(aristmk4_state::via_a_w)
{
	//logerror("VIA port A write %02X\n",data);
	m_psg_data = data;
}

WRITE8_MEMBER(aristmk4_state::via_b_w)
{
	m_ay8910_1 = ( data & 0x0F ) ; //only need first 4 bits per schematics
		//NOTE: when bit 4 is off, we write to AY1, when bit 4 is on, we write to AY2
	m_ay8910_2 = m_ay8910_1;

	if ( m_ay8910_2 & 0x08 ) // is bit 4 on ?
	{
	m_ay8910_2  = (m_ay8910_2 | 0x02) ; // bit 2 is turned on as bit 4 hooks to bit 2 in the schematics
	m_ay8910_1  = 0x00; // write only to ay2
	}
	else
	{
		m_ay8910_2 = 0x00; // write only to ay1
	}

	//only need bc1/bc2 and bdir so drop bit 4.

	m_ay8910_1 = (m_ay8910_1 & 0x07);
	m_ay8910_2 = (m_ay8910_2 & 0x07);

	//PSG ay1

	switch(m_ay8910_1)
	{
	case 0x00:  //INACT -Nothing to do here. Inactive PSG
		break;
	case 0x03:  //READ - Nothing to do here. The read happens in via_a_r
		break;
	case 0x06:  //WRITE
	{
		m_ay1->data_w(space, 0 , m_psg_data);
		//logerror("VIA Port A write data ay1: %02X\n",m_psg_data);
		break;
	}
	case 0x07:  //LATCH Address (set register)
	{
		m_ay1->address_w(space, 0 , m_psg_data);
		//logerror("VIA Port B write register ay1: %02X\n",m_psg_data);
		break;
	}
	default:
		//logerror("Unknown PSG state on ay1: %02X\n",m_ay8910_1);
		break;
	}

	//PSG ay2

	switch(m_ay8910_2)
	{
	case 0x00:  //INACT - Nothing to do here. Inactive PSG
		break;
	case 0x02:  //INACT - '010' Nothing to do here. Inactive PSG. this will only happen on ay2 due to the bit 2 swap on 'inactive'
		break;
	case 0x03:  //READ - Nothing to do here. The read happens in via_a_r
		break;
	case 0x06:  //WRITE
	{
		m_ay2->data_w(space, 0, m_psg_data);
		//logerror("VIA Port A write data ay2: %02X\n",m_psg_data);
		break;
	}
	case 0x07:  //LATCH Address (set register)
	{
		m_ay2->address_w(space, 0, m_psg_data);
		//logerror("VIA Port B write register ay2: %02X\n",m_psg_data);
		break;
	}
		default:
		//logerror("Unknown PSG state on ay2: %02X\n",m_ay8910_2);
		break;
	}
}

WRITE_LINE_MEMBER(aristmk4_state::via_ca2_w)
{
	// CA2 is connected to CDSOL1 on schematics ?
	//logerror("Via Port CA2 write %02X\n",data) ;
}

WRITE_LINE_MEMBER(aristmk4_state::via_cb2_w)
{
	// CB2 = hopper motor (HOPMO1). When it is 0x01, it is not running (active low)
	// when it goes to 0, we're expecting to coins to be paid out, handled in via_b_r
	// as soon as it is 1, HOPCO1 to remain 'ON'

	if (state==0x01)
		m_hopper_motor=state;
	else if (m_hopper_motor<0x02)
		m_hopper_motor=state;

	output_set_value("hopper_motor", m_hopper_motor); // stop motor
}

// Lamp output

WRITE8_MEMBER(aristmk4_state::pblp_out)
{
	output_set_lamp_value(1, (data) & 1);
	output_set_lamp_value(5, (data >> 1) & 1);
	output_set_lamp_value(9, (data >> 2) & 1);
	output_set_lamp_value(11,(data >> 3) & 1);
	output_set_lamp_value(3, (data >> 4) & 1);
	output_set_lamp_value(4, (data >> 5) & 1);
	output_set_lamp_value(2, (data >> 6) & 1);
	output_set_lamp_value(10,(data >> 7) & 1);
	//logerror("Lights port A %02X\n",data);
}

WRITE8_MEMBER(aristmk4_state::pbltlp_out)
{
	output_set_lamp_value(8,  (data) & 1);
	output_set_lamp_value(12, (data >> 1) & 1);
	output_set_lamp_value(6,  (data >> 2) & 1);
	output_set_lamp_value(7,  (data >> 3) & 1);
	output_set_lamp_value(14, (data >> 4) & 1); // light tower
	output_set_lamp_value(15, (data >> 5) & 1); // light tower
	output_set_lamp_value(16, (data >> 6) & 1); // light tower
	output_set_lamp_value(17, (data >> 7) & 1); // light tower
	//logerror("Lights port B: %02X\n",data);
}

WRITE8_MEMBER(aristmk4_state::mlamps)
{
	/* TAKE WIN AND GAMBLE LAMPS */
	output_set_lamp_value(18, (data >> 5) & 1);
	output_set_lamp_value(13, (data >> 6) & 1);
}

WRITE8_MEMBER(aristmk4_state::zn434_w)
{
	// Introducted to prevent warning in log for write to AY1 PORT B
	// this is a write to the ZN434 DA convertors..
}


READ8_MEMBER(aristmk4_state::cashcade_r)
{
	/* work around for cashcade games */
	return cashcade_p[(m_cashcade_c++)%15];
}

WRITE8_MEMBER(aristmk4_state::mk4_printer_w)
{
	//logerror("Printer: %c %d\n",data,data);
}

READ8_MEMBER(aristmk4_state::mk4_printer_r)
{
	return 0;
}

/******************************************************************************

ADDRESS MAP - SLOT GAMES

******************************************************************************/

static ADDRESS_MAP_START( aristmk4_map, AS_PROGRAM, 8, aristmk4_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("mkiv_vram") // video ram -  chips U49 / U50
	AM_RANGE(0x0800, 0x17ff) AM_RAM
	AM_RANGE(0x1800, 0x1800) AM_DEVREADWRITE("crtc", mc6845_device, status_r, address_w)
	AM_RANGE(0x1801, 0x1801) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x1c00, 0x1cff) AM_WRITE(mk4_printer_w)
	AM_RANGE(0x1900, 0x19ff) AM_READ(mk4_printer_r)
	AM_RANGE(0x2000, 0x3fff) AM_ROM  // graphics rom map
	AM_RANGE(0x4000, 0x4fff) AM_RAMBANK("bank1") AM_SHARE("nvram")

	AM_RANGE(0x5000, 0x5000) AM_WRITE(u3_p0)
	AM_RANGE(0x5002, 0x5002) AM_READ(u3_p2)
	AM_RANGE(0x5003, 0x5003) AM_READ(u3_p3)
	AM_RANGE(0x5005, 0x5005) AM_READ(ldsw)
	AM_RANGE(0x500d, 0x500d) AM_READ_PORT("500d")
	AM_RANGE(0x500e, 0x500e) AM_READ_PORT("500e")
	AM_RANGE(0x500f, 0x500f) AM_READ_PORT("500f")
	AM_RANGE(0x5010, 0x501f) AM_DEVREADWRITE("via6522_0",via6522_device,read,write)
	AM_RANGE(0x5200, 0x5200) AM_READ(cashcade_r)
	AM_RANGE(0x5201, 0x5201) AM_READ_PORT("5201")
	AM_RANGE(0x52c0, 0x52c0) AM_READ(bv_p0)
	AM_RANGE(0x52c1, 0x52c1) AM_READ(bv_p1)
	AM_RANGE(0x527f, 0x5281) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0x5300, 0x5300) AM_READ_PORT("5300")
	AM_RANGE(0x5380, 0x5383) AM_DEVREADWRITE("pia6821_0", pia6821_device, read, write)  // RTC data - PORT A , mechanical meters - PORTB ??
	AM_RANGE(0x5440, 0x5440) AM_WRITE(mlamps) // take win and gamble lamps
	AM_RANGE(0x5468, 0x5468) AM_READWRITE(cgdrr,cgdrw) // 4020 ripple counter outputs
	AM_RANGE(0x6000, 0xffff) AM_ROM  // game roms
ADDRESS_MAP_END

/******************************************************************************

ADDRESS MAP - VIDEO POKER GAMES

******************************************************************************/

/*
Poker card style games seem to have different address mapping

The graphics rom is mapped from 0x4000 - 0x4fff

The U87 personality rom is not required, therefore game rom code mapping is from 0x8000-0xffff
*/

static ADDRESS_MAP_START( aristmk4_poker_map, AS_PROGRAM, 8, aristmk4_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("mkiv_vram") // video ram -  chips U49 / U50
	AM_RANGE(0x0800, 0x17ff) AM_RAM
	AM_RANGE(0x1800, 0x1800) AM_DEVREADWRITE("crtc", mc6845_device, status_r, address_w)
	AM_RANGE(0x1801, 0x1801) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x1c00, 0x1cff) AM_WRITE(mk4_printer_w)
	AM_RANGE(0x1900, 0x19ff) AM_READ(mk4_printer_r)
	AM_RANGE(0x4000, 0x4fff) AM_RAMBANK("bank1") AM_SHARE("nvram")

	AM_RANGE(0x5000, 0x5000) AM_WRITE(u3_p0)
	AM_RANGE(0x5002, 0x5002) AM_READ(u3_p2)
	AM_RANGE(0x5003, 0x5003) AM_READ_PORT("5003")
	AM_RANGE(0x5005, 0x5005) AM_READ(ldsw)
	AM_RANGE(0x500d, 0x500d) AM_READ_PORT("500d")
	AM_RANGE(0x500e, 0x500e) AM_READ_PORT("500e")
	AM_RANGE(0x500f, 0x500f) AM_READ_PORT("500f")
	AM_RANGE(0x5010, 0x501f) AM_DEVREADWRITE("via6522_0",via6522_device,read,write)
	AM_RANGE(0x5200, 0x5200) AM_READ(cashcade_r)
	AM_RANGE(0x5201, 0x5201) AM_READ_PORT("5201")
	AM_RANGE(0x52c0, 0x52c0) AM_READ(bv_p0)
	AM_RANGE(0x52c1, 0x52c1) AM_READ(bv_p1)
	AM_RANGE(0x527f, 0x5281) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0x5300, 0x5300) AM_READ_PORT("5300")
	AM_RANGE(0x5380, 0x5383) AM_DEVREADWRITE("pia6821_0", pia6821_device, read, write)  // RTC data - PORT A , mechanical meters - PORTB ??
	AM_RANGE(0x5440, 0x5440) AM_WRITE(mlamps) // take win and gamble lamps
	AM_RANGE(0x5468, 0x5468) AM_READWRITE(cgdrr,cgdrw) // 4020 ripple counter outputs
	AM_RANGE(0x6000, 0x7fff) AM_ROM  // graphics rom map
	AM_RANGE(0x8000, 0xffff) AM_ROM  // game roms
ADDRESS_MAP_END

/******************************************************************************

INPUT PORTS

******************************************************************************/

static INPUT_PORTS_START(aristmk4)

	PORT_START("via_port_b")
	PORT_DIPNAME( 0x10, 0x00, "1" )                                                                                         // "COIN FAULT"
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) ) PORT_DIPLOCATION("AY:1")
	PORT_DIPNAME( 0x20, 0x00, "2" )                                                                                         // "COIN FAULT"
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) ) PORT_DIPLOCATION("AY:2")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Hopper Coin Release") PORT_CODE(KEYCODE_BACKSLASH)              // "ILLEGAL COIN PAID"

	PORT_DIPNAME( 0x80, 0x00, "CBOPT1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) ) PORT_DIPLOCATION("AY:4")

	PORT_START("5002")
	PORT_DIPNAME( 0x01, 0x00, "HOPCO2") // coins out hopper 2 , why triggers logic door ?
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5002:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "CBOPT2") // coin in cash box 2
	PORT_DIPSETTING(    0x02, DEF_STR( On ) ) PORT_DIPLOCATION("5002:2")
	PORT_DIPNAME( 0x04, 0x00, "HOPHI2") // hopper 2 full
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5002:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DOPTI")  // photo optic door                                                                         DOOR OPEN SENSE SWITCH
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN ) PORT_NAME("Audit Key") PORT_TOGGLE PORT_CODE(KEYCODE_K) // AUDTSW
	PORT_DIPNAME( 0x20, 0x00, "HOPLO1") // hopper 1 low
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5002:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "HOPLO2") // hopper 2 low
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5002:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("Robot Test / Hopper Reset") PORT_CODE(KEYCODE_Z) // PB6

	PORT_START("5003")
	PORT_DIPNAME( 0x01, 0x00, "OPTAUI") // opto audit in
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5003:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR ) PORT_NAME("Main Door") PORT_TOGGLE PORT_CODE(KEYCODE_M) // DSWDT
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN ) PORT_NAME("Jackpot Key") PORT_TOGGLE PORT_CODE(KEYCODE_L) // JKPTSW
	PORT_DIPNAME( 0x08, 0x08, "HOPHI1") // hopper 1 full
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5003:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "OPTA2") // coin in a2
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5003:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "OPTB2") // coin in b2
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5003:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "PTRTAC") // printer taco
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5003:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "PTRHOM") // printer home
	PORT_DIPSETTING(    0x80, DEF_STR( On ) ) PORT_DIPLOCATION("5003:8")

	PORT_START("5005")
	PORT_DIPNAME( 0x01, 0x01, "CREDIT SELECT 1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5005:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "CREDIT SELECT 2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5005:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "CREDIT SELECT 3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5005:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "5005-4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5005:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "CGDRSW") // Logic Door (Security Cage)
	PORT_DIPSETTING(    0x10, DEF_STR( On ) ) PORT_DIPLOCATION("5005:5")
	PORT_DIPNAME( 0x20, 0x00, "5005-6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5005:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "5005-7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5005:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "5005-8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5005:8")
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("5300")
	PORT_DIPNAME( 0x01, 0x00, "5300-1")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5300:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "5300-2")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5300:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "5300-3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5300:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "5300-4")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5300:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "5300-5")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5300:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "5300-6")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5300:6") // bill validator d/c , U.S must be on
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "5300-7")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5300:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "5300-8")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5300:8") // mechanical meters, must be on
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("500d")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("5 Credits Per Line") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("10 Credits Per Line") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Collect") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Reserve") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("Gamble") PORT_CODE(KEYCODE_U) // auto gamble & gamble
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take Win") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-7 UNUSED")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-8 UNUSED")

	PORT_START("500e")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("1 Credit Per Line") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Play 1 Line / Red") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("2 Credits Per Line") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Play 9 Lines / Black") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("3 Credits Per Line") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Play 7 Lines") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Play 5 Lines") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Play 3 Lines") PORT_CODE(KEYCODE_D)

	PORT_START("500f")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("2-1 UNUSED")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("2-2 UNUSED")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("2-3 UNUSED")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON12 ) PORT_NAME("AUX1") PORT_CODE(KEYCODE_X) // PB5
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON13 ) PORT_NAME("Memory Reset") PORT_CODE(KEYCODE_C) // PB4
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_NAME("Hopper Test") PORT_CODE(KEYCODE_V) // PB3
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_NAME("Print Data") PORT_CODE(KEYCODE_B) // PB2
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON16 ) PORT_NAME("Clock Init") PORT_CODE(KEYCODE_N) // PB1

	PORT_START("5200")
	PORT_DIPNAME( 0x01, 0x00, "5200-1")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5200:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "5200-2")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5200:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "5200-3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5200:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "5200-4")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5200:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "5200-5")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5200:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "5200-6")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5200:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "5200-7")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5200:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "5200-8")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5200:8")
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("5201")
	PORT_DIPNAME( 0x01, 0x00, "5201-1")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5201:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "5201-2")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5201:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "5201-3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5201:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "5201-4") // fixes link offline error
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5201:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "5201-5") // freeze switch? only affects fhunter(a) and cgold2
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5201:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "5201-6")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5201:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "5201-7")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5201:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "5201-8")
	PORT_DIPSETTING(    0X00, DEF_STR( Off ) ) PORT_DIPLOCATION("5201:8")
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("insertcoin")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_NAME("Insert Credit")

	PORT_START("insertnote")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_NAME("Insert Note")

	PORT_START("powerfail")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Power Fail / Shutdown") PORT_CODE(KEYCODE_COMMA)

	/************************************** LINKS ***************************************************************/

	PORT_START("LK13")
	PORT_DIPNAME( 0x10, 0x10, "Speed Select" ) PORT_DIPLOCATION("LK13:1")
	PORT_DIPSETTING(    0x00, "3 Mhz" )
	PORT_DIPSETTING(    0x10, "1.5 Mhz" )

	/********************************* Dip switch for background color *************************************************/

	PORT_START("SW7")
	PORT_DIPNAME( 0x01, 0x01, "SW7 - U22 BG COLOR" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW7:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "SW7 - U47 BG COLOR" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW7:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )

	/********************************* 9 way control rotary switches ***************************************************/

	PORT_START("SW3")
	PORT_DIPNAME( 0x0f, 0x00, "SW3 - M/C NO" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPSETTING(    0x09, "9" )

	PORT_START("SW4")
	PORT_DIPNAME( 0x0f, 0x00, "SW4 - M/C NO" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPSETTING(    0x09, "9" )

	PORT_START("SW5")
	PORT_DIPNAME( 0x0f, 0x00, "SW5 - M/C NO" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPSETTING(    0x09, "9" )

	PORT_START("SW6")
	PORT_DIPNAME( 0x0f, 0x00, "SW6 - M/C NO" )
	PORT_DIPSETTING(    0x00, "0" )
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x06, "6" )
	PORT_DIPSETTING(    0x07, "7" )
	PORT_DIPSETTING(    0x08, "8" )
	PORT_DIPSETTING(    0x09, "9" )

	/***************** DIP SWITCHES **********************************************************************/

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "DSW1 - Maxbet rejection" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DSW1 - Hopper pay limit - S1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW1 - Hopper pay limit - S2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DSW1 - Hopper pay limit - S3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "DSW1 - Cash credit option" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "DSW1 - Link Jackpot - S1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DSW1 - Link Jackpot - S2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DSW1 - Auto spin" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "DSW2 - Maximum credit - S1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "DSW2 - Maximum credit - S2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "DSW2 - Maximum credit - S3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DSW2 - Jackpot limit - S1" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW2 - Jackpot limit - S2" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW2 - Jackpot limit - S3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "DSW2 - Auto J/P payout" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DSW2 - Unconnected" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	/* Rotary switch for Note selection. I've done it like this so we can use 1 keyboard command (IPT_COIN2) to feed the bill acceptor */
	PORT_START("NS")
	PORT_DIPNAME( 0x0f, 0x00, "Note Selector" )
	PORT_DIPSETTING(    0x00, "$1" )
	PORT_DIPSETTING(    0x01, "$2" )
	PORT_DIPSETTING(    0x02, "$5" )
	PORT_DIPSETTING(    0x03, "$10" )
	PORT_DIPSETTING(    0x04, "$20" )
	PORT_DIPSETTING(    0x05, "$50" )
	PORT_DIPSETTING(    0x06, "$100" )

INPUT_PORTS_END

static INPUT_PORTS_START(3bagflvt)
	PORT_INCLUDE(aristmk4)

	PORT_MODIFY("500d")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Max Bet") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Auto Play") PORT_CODE(KEYCODE_J)

	PORT_MODIFY("500e")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Play 1 Line") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Play 9 Lines") PORT_CODE(KEYCODE_H)
INPUT_PORTS_END

static INPUT_PORTS_START(3bagflnz)
	PORT_INCLUDE(3bagflvt)

	PORT_MODIFY("500d")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("4 Credits Per Line") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("5 Credits Per Line") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-5 UNUSED")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-6 UNUSED")
INPUT_PORTS_END

static INPUT_PORTS_START(eforest)
	PORT_INCLUDE(aristmk4)

	PORT_MODIFY("500d")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Play 7 Lines") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Play 9 Lines / Black") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_CODE(KEYCODE_A)

	PORT_MODIFY("500e")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Play 1 Line / Red") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("1 Credit Per Line") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Play 3 Lines") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("5 Credits Per Line") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Play 5 Lines") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("4 Credits Per Line") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("3 Credits Per Line") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("2 Credits Per Line") PORT_CODE(KEYCODE_D)
INPUT_PORTS_END

static INPUT_PORTS_START(arcwins)
	PORT_INCLUDE(eforest)

	PORT_MODIFY("500d")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Play 4 Lines") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Play 5 Lines / Black") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Cashout") PORT_CODE(KEYCODE_Q)

	PORT_MODIFY("500e")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Play 2 Lines") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Play 3 Lines") PORT_CODE(KEYCODE_R)
INPUT_PORTS_END

static INPUT_PORTS_START(cgold2)
	PORT_INCLUDE(arcwins)

	PORT_MODIFY("500d")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Play 5 Lines") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-5 UNUSED")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-6 UNUSED")

	PORT_MODIFY("500e")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Play 1 Line") PORT_CODE(KEYCODE_W)
INPUT_PORTS_END

static INPUT_PORTS_START(fhunter)
	PORT_INCLUDE(cgold2)

	PORT_MODIFY("500d")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Play 7 Lines") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Play 9 Lines") PORT_CODE(KEYCODE_Y)

	PORT_MODIFY("500e")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Play 3 Lines") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Play 5 Lines") PORT_CODE(KEYCODE_R)
INPUT_PORTS_END

static INPUT_PORTS_START(arimk4nz)
	PORT_INCLUDE(aristmk4)

	PORT_MODIFY("500d")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("4 Credits Per Line") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("5 Credits Per Line") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_CODE(KEYCODE_U)
INPUT_PORTS_END

static INPUT_PORTS_START(goldenc)
	PORT_INCLUDE(aristmk4)

	PORT_MODIFY("500d")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_CODE(KEYCODE_U)
INPUT_PORTS_END

static INPUT_PORTS_START(kgbird)
	PORT_INCLUDE(aristmk4)

	PORT_MODIFY("500d")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("4 Credits Per Line") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("5 Credits Per Line") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_CODE(KEYCODE_U)

	PORT_MODIFY("500e")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Play 5 Lines / Black") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Play 4 Lines") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Play 3 Lines") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Play 2 Lines") PORT_CODE(KEYCODE_D)
INPUT_PORTS_END

static INPUT_PORTS_START(topgear)
	PORT_INCLUDE(aristmk4)

	PORT_MODIFY("500d")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-1 UNUSED")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-2 UNUSED")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Collect") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Change") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-5 UNUSED")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-6 UNUSED")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-7 UNUSED")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-8 UNUSED")

	PORT_MODIFY("500e")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("1-1 UNUSED")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Play 1 Line") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Max Bet") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Play 5 Lines") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("1-5 UNUSED")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Play 4 Lines") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Play 3 Lines") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Play 2 Lines") PORT_CODE(KEYCODE_D)
INPUT_PORTS_END

static INPUT_PORTS_START(wildone)
	PORT_INCLUDE(aristmk4)

	PORT_MODIFY("500d")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Bet 2 / Hold 2") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Bet 3 / Hold 3") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-5 UNUSED")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-6 UNUSED")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-7 UNUSED")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-8 UNUSED")

	PORT_MODIFY("500e")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Collect") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Black") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Red") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Bet 1 / Hold 1") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("High 5 / Hold 5") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Big 5 / Hold 4") PORT_CODE(KEYCODE_G) // no bet 4 button
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take Win") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Draw") PORT_CODE(KEYCODE_R)
INPUT_PORTS_END

static INPUT_PORTS_START(gldnpkr)
	PORT_INCLUDE(wildone)

	PORT_MODIFY("500d")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-1 UNUSED")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("Gamble") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_CODE(KEYCODE_F)

	PORT_MODIFY("500e")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_BET ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Change") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / Red") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Black") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Draw") PORT_CODE(KEYCODE_R)
INPUT_PORTS_END

static INPUT_PORTS_START(fvrpitch)
	PORT_INCLUDE(arcwins)

	PORT_MODIFY("500d")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Play 5 Lines") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Reserve") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-5 UNUSED")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-6 UNUSED")

	PORT_MODIFY("500e")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Play 1 Line") PORT_CODE(KEYCODE_W)
INPUT_PORTS_END


static const gfx_layout layout8x8x6 =
{
	8,8,
	RGN_FRAC(1,6),
	6,
	{
		RGN_FRAC(5,6),
		RGN_FRAC(2,6),
		RGN_FRAC(4,6),
		RGN_FRAC(1,6),
		RGN_FRAC(3,6),
		RGN_FRAC(0,6)
	},
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START(aristmk4)
	GFXDECODE_ENTRY("tile_gfx",0x0,layout8x8x6, 0, 8 )
GFXDECODE_END

/* read m/c number */

READ8_MEMBER(aristmk4_state::pa1_r)
{
	return (ioport("SW3")->read() << 4) + ioport("SW4")->read();
}

READ8_MEMBER(aristmk4_state::pb1_r)
{
	return (ioport("SW5")->read() << 4) + ioport("SW6")->read();
}

READ8_MEMBER(aristmk4_state::pc1_r)
{
	return 0;
}

/* same as Casino Winner HW */
PALETTE_INIT_MEMBER(aristmk4_state, aristmk4)
{
	const UINT8 *color_prom = memregion("proms")->base();
	int i;

	for (i = 0;i < palette.entries();i++)
	{
		int bit0,bit1,bit2,r,g,b;

		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		b = 0x4f * bit0 + 0xa8 * bit1;
		bit0 = (color_prom[0] >> 2) & 0x01;
		bit1 = (color_prom[0] >> 3) & 0x01;
		bit2 = (color_prom[0] >> 4) & 0x01;
		g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = (color_prom[0] >> 5) & 0x01;
		bit1 = (color_prom[0] >> 6) & 0x01;
		bit2 = (color_prom[0] >> 7) & 0x01;
		r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
		color_prom++;
	}
}

DRIVER_INIT_MEMBER(aristmk4_state,aristmk4)
{
	m_shapeRomPtr = (UINT8 *)memregion("tile_gfx")->base();
	memcpy(m_shapeRom,m_shapeRomPtr,sizeof(m_shapeRom)); // back up
	m_nvram = std::make_unique<UINT8[]>(0x1000);
}

void aristmk4_state::machine_start()
{
	save_pointer(NAME(m_nvram.get()), 0x1000); // m_nvram
}

void aristmk4_state::machine_reset()
{
	/* mark 4 has a link on the motherboard to switch between 1.5MHz and 3MHz clock speed */
	switch(ioport("LK13")->read())  // CPU speed control... 3mhz or 1.5MHz
	{
	case 0x00:
		m_maincpu->set_unscaled_clock(MAIN_CLOCK/4);  // 3 MHz
		break;
	case 0x10:
		m_maincpu->set_unscaled_clock(MAIN_CLOCK/8);  // 1.5 MHz
		break;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(aristmk4_state::aristmk4_pf)
{
	/*
	IRQ generator pulses the NMI signal to CPU in the event of power down or power failure.
	This event is recorded in NVRAM to facilitate the Robot Test.

	Would be ideal to use this in our add_exit_callback instead of using a timer but it doesn't seem to
	save the power down state in nvram. Is there a cleaner way to do this?

	To enter the robot test

	1. Open the main door
	2. Trigger powerfail / NMI by presing L for at least 1 second, the game will freeze.
	3. Press F3 ( reset ) whilst holding down robot/hopper test button ( Z )

	Note: The use of 1 Hz in the timer is to avoid unintentional triggering the NMI ( ie.. hold down L for at least 1 second )
	*/

	if(ioport("powerfail")->read()) // send NMI signal if L pressed
	{
	m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE );
	}
}

static MACHINE_CONFIG_START( aristmk4, aristmk4_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, MAIN_CLOCK/8) // 1.5mhz
	MCFG_CPU_PROGRAM_MAP(aristmk4_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", aristmk4_state,  irq0_line_hold)

	MCFG_NVRAM_ADD_0FILL("nvram")
	MCFG_TIMER_DRIVER_ADD_PERIODIC("power_fail", aristmk4_state, aristmk4_pf, attotime::from_hz(1))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(320, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 304-1, 0, 216-1)    /* from the crtc registers... updated by crtc */
	MCFG_SCREEN_UPDATE_DRIVER(aristmk4_state, screen_update_aristmk4)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", aristmk4)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_INIT_OWNER(aristmk4_state, aristmk4)

	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(aristmk4_state, pa1_r))
	MCFG_I8255_IN_PORTB_CB(READ8(aristmk4_state, pb1_r))
	MCFG_I8255_IN_PORTC_CB(READ8(aristmk4_state, pc1_r))

	MCFG_DEVICE_ADD("via6522_0", VIA6522, 0) /* 1 MHz.(only 1 or 2 MHz.are valid) */
	MCFG_VIA6522_READPA_HANDLER(READ8(aristmk4_state, via_a_r))
	MCFG_VIA6522_READPB_HANDLER(READ8(aristmk4_state, via_b_r))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(aristmk4_state, via_a_w))
	MCFG_VIA6522_WRITEPB_HANDLER(WRITE8(aristmk4_state, via_b_w))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE(aristmk4_state, via_ca2_w))
	MCFG_VIA6522_CB2_HANDLER(WRITELINE(aristmk4_state, via_cb2_w))
	MCFG_VIA6522_IRQ_HANDLER(DEVWRITELINE("maincpu", m6809_device, firq_line))
	// CA1 is connected to +5V, CB1 is not connected.

	MCFG_DEVICE_ADD("pia6821_0", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(aristmk4_state, mkiv_pia_ina))
	MCFG_PIA_WRITEPA_HANDLER(WRITE8(aristmk4_state, mkiv_pia_outa))
	MCFG_PIA_WRITEPB_HANDLER(WRITE8(aristmk4_state, mkiv_pia_outb))
	MCFG_PIA_CA2_HANDLER(WRITELINE(aristmk4_state, mkiv_pia_ca2))
	MCFG_PIA_CB2_HANDLER(WRITELINE(aristmk4_state, mkiv_pia_cb2))

	MCFG_MC6845_ADD("crtc", C6545_1, "screen", MAIN_CLOCK/8) // TODO: type is unknown
	/* in fact is a mc6845 driving 4 pixels by memory address.
	 that's why the big horizontal parameters */
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(4)

	MCFG_MC146818_ADD( "rtc", XTAL_4_194304Mhz )

	MCFG_SPEAKER_STANDARD_MONO("mono")

	// the Mark IV has X 2 AY8910 sound chips which are tied to the VIA
	MCFG_SOUND_ADD("ay1", AY8910 , MAIN_CLOCK/8)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW1"))
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(aristmk4_state, zn434_w)) // Port write to set Vout of the DA convertors ( 2 x ZN434 )
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MCFG_SOUND_ADD("ay2", AY8910 , MAIN_CLOCK/8)
	MCFG_AY8910_PORT_A_WRITE_CB(WRITE8(aristmk4_state, pblp_out))   // Port A write - goes to lamps on the buttons x8
	MCFG_AY8910_PORT_B_WRITE_CB(WRITE8(aristmk4_state, pbltlp_out))  // Port B write - goes to lamps on the buttons x4 and light tower x4
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MCFG_SOUND_ADD("samples", SAMPLES, 0)
	MCFG_SAMPLES_CHANNELS(5)  /* one for each meter - can pulse simultaneously */
	MCFG_SAMPLES_NAMES(meter_sample_names)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.05)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( aristmk4_poker, aristmk4 )
	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(aristmk4_poker_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", aristmk4_state,  irq0_line_hold)
MACHINE_CONFIG_END

/* same as Aristocrat Mark-IV HW color offset 7 */
PALETTE_INIT_MEMBER(aristmk4_state,lions)
{
	int i;

	for (i = 0;i < palette.entries();i++)
	{
		int bit0,bit1,r,g,b;

		bit0 = (i >> 0) & 0x01;
		bit1 = (i >> 1) & 0x01;
		b = 0x4f * bit0 + 0xa8 * bit1;
		bit0 = (i >> 2) & 0x01;
		bit1 = (i >> 3) & 0x01;
		g = 0x4f * bit0 + 0xa8 * bit1;
		bit0 = (i >> 4) & 0x01;
		bit1 = (i >> 5) & 0x01;
		r = 0x4f * bit0 + 0xa8 * bit1;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

static MACHINE_CONFIG_DERIVED( 86lions, aristmk4 )
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_INIT_OWNER(aristmk4_state,lions)
MACHINE_CONFIG_END

ROM_START( 3bagflvt )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59",  0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("5vxfc790.u87", 0x06000, 0x2000, CRC(79ee932f) SHA1(de85de107310315b69bd7564f1921c7501b679b2))
	ROM_LOAD("5vxfc790.u86", 0x08000, 0x8000, CRC(b6185f3b) SHA1(db642d7b1d1fd93483642bae518eb99a3e99aec9))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("1vlsh224.u20", 0x00000, 0x2000, CRC(b02d4ce8) SHA1(eace41f870bfbc253124efd72f1c7d6021f2e99f))
	ROM_LOAD("1vlsh224.u21", 0x02000, 0x2000, CRC(06218c95) SHA1(cbda8e50fd4e9c8a3c51a006921a85d4bfaa6f78))
	ROM_LOAD("1vlsh224.u22", 0x04000, 0x2000, CRC(191e73f1) SHA1(e6d510b155f9cd3427a70346e5ff28969309be4e))
	ROM_LOAD("1vlsh224.u45", 0x06000, 0x2000, CRC(054c55cb) SHA1(3df1893095f867220f3d6a52a40bcdffbfc8b529))
	ROM_LOAD("1vlsh224.u46", 0x08000, 0x2000, CRC(f33970b3) SHA1(8814a4d29383545c7c48e5b44f16a53e38b67fc3))
	ROM_LOAD("1vlsh224.u47", 0x0a000, 0x2000, CRC(609ecf9e) SHA1(9d819bb71f62eb4dd1b3d71748e87c7d77e2afe6))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("1cm48.u71", 0x0000, 0x0200, CRC(81daeeb0) SHA1(7dfe198c6def5c4ae4ecac488d65c2911fb3a890))
ROM_END

ROM_START( 3bagflnz )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59",  0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("3vxfc5345.u87", 0x06000, 0x2000, CRC(ba97a469) SHA1(fee56fe7116d1f1aab2b0f2526101d4eb87f0bf1))
	ROM_LOAD("3vxfc5345.u86", 0x08000, 0x8000, CRC(c632c7c7) SHA1(f3090d037f71a0cf099bb55abbc509cf95f0cbba))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("1vlsh224.u20", 0x00000, 0x2000, CRC(b02d4ce8) SHA1(eace41f870bfbc253124efd72f1c7d6021f2e99f)) // original graphics ROMs were bad,
	ROM_LOAD("1vlsh224.u21", 0x02000, 0x2000, CRC(06218c95) SHA1(cbda8e50fd4e9c8a3c51a006921a85d4bfaa6f78)) // using 3bagflvt ROMs for now although
	ROM_LOAD("1vlsh224.u22", 0x04000, 0x2000, CRC(191e73f1) SHA1(e6d510b155f9cd3427a70346e5ff28969309be4e)) // some unused tiles differ between sets
	ROM_LOAD("1vlsh224.u45", 0x06000, 0x2000, CRC(054c55cb) SHA1(3df1893095f867220f3d6a52a40bcdffbfc8b529))
	ROM_LOAD("1vlsh224.u46", 0x08000, 0x2000, CRC(f33970b3) SHA1(8814a4d29383545c7c48e5b44f16a53e38b67fc3))
	ROM_LOAD("1vlsh224.u47", 0x0a000, 0x2000, CRC(609ecf9e) SHA1(9d819bb71f62eb4dd1b3d71748e87c7d77e2afe6))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("1cm48.u71", 0x0000, 0x0200, CRC(81daeeb0) SHA1(7dfe198c6def5c4ae4ecac488d65c2911fb3a890))
ROM_END

ROM_START( blkrhino )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("3vxfc5344.u87", 0x06000, 0x2000, CRC(7aed16f5) SHA1(0229387e352da8e7278e5bc5c61079742d05d900))
	ROM_LOAD("3vxfc5344.u86", 0x08000, 0x8000, CRC(4739f0f0) SHA1(231b6ad26b6b5d413dbd0a23257e86814978449b))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("1vlsh236.u20", 0x00000, 0x2000, CRC(0559fe98) SHA1(2ffb7b3ce3b7ba3bd846cae514b66b1c1a3be91f))
	ROM_LOAD("1vlsh236.u21", 0x02000, 0x2000, CRC(c0b94f7b) SHA1(8fc3bc53c532407b77682e5e9ac6a625081d22a3))
	ROM_LOAD("1vlsh236.u22", 0x04000, 0x2000, CRC(2f4f0fe5) SHA1(b6c75bd3b6281a2de7bfea8162c39d58b0e8fa32))
	ROM_LOAD("1vlsh236.u45", 0x06000, 0x2000, CRC(e483b4cd) SHA1(1cb3f77e7d470d7dcd8e50a0f59298d5546e8b58))
	ROM_LOAD("1vlsh236.u46", 0x08000, 0x2000, CRC(4a0ce91d) SHA1(e2f853c69fb256870c9809cdfbba2b40b47a0004))
	ROM_LOAD("1vlsh236.u47", 0x0a000, 0x2000, CRC(b265276e) SHA1(8fc0b7a0c12549b4138c51eb91b74f13282909dd))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("2cm34.u71", 0x0000, 0x0200, CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END

ROM_START( coralr2 )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("1vxfc5472.u87", 0x06000, 0x2000, CRC(f51e541b) SHA1(00f5b9019cdae77d4b5745156b92343d22ad3a6e))
	ROM_LOAD("1vxfc5472.u86", 0x08000, 0x8000, CRC(d8d27f65) SHA1(19aec2a29e9d3ecbd8ecfd74ae60cfbf197d2faa))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("1vlsh385.u20", 0x00000, 0x2000, CRC(5156f5ec) SHA1(8b4d0699b4477531d513e21f549fcc0ee6ea82ee))
	ROM_LOAD("1vlsh385.u21", 0x02000, 0x2000, CRC(bf27732a) SHA1(9383dfc37c5c3ad0d628f2134f010e977e25ef39))
	ROM_LOAD("1vlsh385.u22", 0x04000, 0x2000, CRC(a563c2fa) SHA1(10dab35515e2d8332d114a5f103343403334a65f))
	ROM_LOAD("1vlsh385.u45", 0x06000, 0x2000, CRC(73814767) SHA1(91c77d7b634bd8a5c32e0ceeb54a8bbeedfe8130))
	ROM_LOAD("1vlsh385.u46", 0x08000, 0x2000, CRC(e13ec0ed) SHA1(80d5ef2d980a8fe1f2bb28b512022518ffc82de1))
	ROM_LOAD("1vlsh385.u47", 0x0a000, 0x2000, CRC(30e88bb4) SHA1(dfcd21c6fc50123dfcc0e60429948c650a6de625))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("2cm34.u71", 0x0000, 0x0200, CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END

ROM_START( eforest )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("12xf528902.u87", 0x06000, 0x2000, CRC(b2f79725) SHA1(66842130b49276bda91e211514af0ab074d2c283))
	ROM_LOAD("12xf528902.u86", 0x08000, 0x8000, CRC(547207f3) SHA1(aedae50abb4cffa0434abfe606a11fbbba037197))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("1vlsh230.u20", 0x00000, 0x2000, CRC(00446ff1) SHA1(e37e2782669667efab07ab3cd4e2c9f87770add5))
	ROM_LOAD("1vlsh230.u21", 0x02000, 0x2000, CRC(75670af6) SHA1(c6ce3dcc9e46ebdee65220cc0e0c8b43ee786ec0))
	ROM_LOAD("1vlsh230.u22", 0x04000, 0x2000, CRC(531378f3) SHA1(7ce52ccfb38c8078eeb77e43cff8e426bd8d4d0f))
	ROM_LOAD("1vlsh230.u45", 0x06000, 0x2000, CRC(e4dd4c72) SHA1(caeb1f754bcb7304b9a61786fb818eea7714808f))
	ROM_LOAD("1vlsh230.u46", 0x08000, 0x2000, CRC(75ad8e3f) SHA1(87812850f08f8ad3057d0e5a2a20ad8acba01a26))
	ROM_LOAD("1vlsh230.u47", 0x0a000, 0x2000, CRC(af9f9869) SHA1(1dac81470889a5fc5b58f3ad0c8dfa1369a800e8))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("2cm34.u71", 0x0000, 0x0200, CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END

ROM_START( eforesta )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("4vxfc818.u87", 0x06000, 0x2000, CRC(03c2890f) SHA1(10d479b7ccece813676ad815a96169bbf259c49d))
	ROM_LOAD("4vxfc818.u86", 0x08000, 0x8000, CRC(36125194) SHA1(dc681dc60b25893ca3ee101f6813c22b914771f5))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("1vlsh230.u20", 0x00000, 0x2000, CRC(00446ff1) SHA1(e37e2782669667efab07ab3cd4e2c9f87770add5))
	ROM_LOAD("1vlsh230.u21", 0x02000, 0x2000, CRC(75670af6) SHA1(c6ce3dcc9e46ebdee65220cc0e0c8b43ee786ec0))
	ROM_LOAD("1vlsh230.u22", 0x04000, 0x2000, CRC(531378f3) SHA1(7ce52ccfb38c8078eeb77e43cff8e426bd8d4d0f))
	ROM_LOAD("1vlsh230.u45", 0x06000, 0x2000, CRC(e4dd4c72) SHA1(caeb1f754bcb7304b9a61786fb818eea7714808f))
	ROM_LOAD("1vlsh230.u46", 0x08000, 0x2000, CRC(75ad8e3f) SHA1(87812850f08f8ad3057d0e5a2a20ad8acba01a26))
	ROM_LOAD("1vlsh230.u47", 0x0a000, 0x2000, CRC(af9f9869) SHA1(1dac81470889a5fc5b58f3ad0c8dfa1369a800e8))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("2cm34.u71", 0x0000, 0x0200, CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END

ROM_START( eforestb )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("3vxfc5343.u87", 0x06000, 0x2000, CRC(49b9c5ef) SHA1(bd1761f41ddb3f19b6b923de77743a2b5ec078e1))
	ROM_LOAD("3vxfc5343.u86", 0x08000, 0x8000, CRC(a3eb0c09) SHA1(5a0947f2f36a87dffe4041fbaebaabb1c694bafe))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("1vlsh230_a.u20", 0x00000, 0x2000, CRC(bf3a23b0) SHA1(00405e0c0ac03ecffba1077bacf61265cca72130)) // alternate graphics EPROMs, same part number
	ROM_LOAD("1vlsh230_a.u21", 0x02000, 0x2000, CRC(ba171964) SHA1(7d43559965f467f07419f77d07d7d34ae60d2e90))
	ROM_LOAD("1vlsh230.u22", 0x04000, 0x2000, CRC(531378f3) SHA1(7ce52ccfb38c8078eeb77e43cff8e426bd8d4d0f))
	ROM_LOAD("1vlsh230.u45", 0x06000, 0x2000, CRC(e4dd4c72) SHA1(caeb1f754bcb7304b9a61786fb818eea7714808f))
	ROM_LOAD("1vlsh230_a.u46", 0x08000, 0x2000, CRC(a3ca69b0) SHA1(c4bdd8afbb4d076f07d4a14a7e7ac8907a0cb7ec))
	ROM_LOAD("1vlsh230.u47", 0x0a000, 0x2000, CRC(af9f9869) SHA1(1dac81470889a5fc5b58f3ad0c8dfa1369a800e8))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("2cm34.u71", 0x0000, 0x0200, CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END

ROM_START( goldenc )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("1vxfc5462.u87", 0x06000, 0x2000, CRC(11b569f7) SHA1(270e1be6bf2a75400af174ceb65436bb6a381a62))
	ROM_LOAD("1vxfc5462.u86", 0x08000, 0x8000, CRC(9714b080) SHA1(41c7d840f600ddff31794ebe949f89c89bd4f2ad))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("1vlsh231.u20", 0x00000, 0x2000, CRC(d4b18412) SHA1(a42a06dbfc55730b27b3857646bfa34ae0e3cb32))
	ROM_LOAD("1vlsh231.u21", 0x02000, 0x2000, CRC(80e22d51) SHA1(5e187070d300209e31f603aa561011e17d4305d2))
	ROM_LOAD("1vlsh231.u22", 0x04000, 0x2000, CRC(1f84ed74) SHA1(df2af247972d6540fd4aac31b51f3aa44248061c))
	ROM_LOAD("1vlsh231.u45", 0x06000, 0x2000, CRC(9d267ef1) SHA1(3781e63552036dc7613b21704a4456ddfb67433f))
	ROM_LOAD("1vlsh231.u46", 0x08000, 0x2000, CRC(a3ca369e) SHA1(e3076c9f3017991b93214bebf7f5227d995eeda1))
	ROM_LOAD("1vlsh231.u47", 0x0a000, 0x2000, CRC(844fa43b) SHA1(b8ef6cc2aca955f41b15cd8e3c281eee4b611e80))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("2cm34.u71", 0x0000, 0x0200, CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END

ROM_START( swtht2nz )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("1vxfc5461.u87", 0x06000, 0x2000, CRC(ae10c63f) SHA1(80e5aca4dec7d2503bf7be81ed8b761ebbe4c174))
	ROM_LOAD("1vxfc5461.u86", 0x08000, 0x8000, CRC(053e71f0) SHA1(4a45bd11b53347be90402cea7bd94a648d6b8129))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", ROMREGION_ERASEFF)
	ROM_LOAD("1vlsh237.u20", 0x00000, 0x2000, CRC(1e38dfc3) SHA1(40a75fc35ebd49ea9c21cb42c30a2aba988c3139))
	ROM_LOAD("1vlsh237.u21", 0x02000, 0x2000, CRC(77caf3fa) SHA1(559898ccffffd8f59c555722dea75600c823997f))
	ROM_LOAD("1vlsh237.u22", 0x04000, 0x2000, CRC(76babc55) SHA1(0902497ad2222490a690fe77feacc350d2997403))
	ROM_LOAD("1vlsh237.u45", 0x06000, 0x2000, CRC(da9514b5) SHA1(d63562095cec463864dfd2c580aa93f45adef853))
	ROM_LOAD("1vlsh237.u46", 0x08000, 0x2000, CRC(4d03c73f) SHA1(7ae629a90feb87019cc01ecef804c5ba28861f00))
	ROM_LOAD("1vlsh237.u47", 0x0a000, 0x2000, CRC(c51e37bb) SHA1(8f3d9b61926fe21089559736b3458fe3b84618f2))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("2cm34.u71", 0x0000, 0x0200, CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END

ROM_START( kgbird )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("4vxfc5341.u87", 0x06000, 0x2000, CRC(5e7c1762) SHA1(2e80be06c7737aca304d46f3c3f1efd24c570cfd))
	ROM_LOAD("4vxfc5341.u86", 0x08000, 0x8000, CRC(3450c544) SHA1(f8883ce3b4bd9073ec6bc985f4666b46f17de092))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("1vlsh159.u20", 0x00000, 0x2000, CRC(df737d57) SHA1(2ec7efe55938ee11376d12d51516c4094ad3fc01))
	ROM_LOAD("1vlsh159.u21", 0x02000, 0x2000, CRC(8e9a65d9) SHA1(e305b8d75b9666377498abf3e2801033effb969b))
	ROM_LOAD("1vlsh159.u22", 0x04000, 0x2000, CRC(5614ee55) SHA1(3eb3872aa8d2b8c2bd798fd46cc715c64bf35714))
	ROM_LOAD("1vlsh159.u45", 0x06000, 0x2000, CRC(cf496cf2) SHA1(cf097835b5f3d5a656ff84063c54d6b1d40703cd))
	ROM_LOAD("1vlsh159.u46", 0x08000, 0x2000, CRC(e155c8d4) SHA1(9c50152dd420d545a88eaea98c2dd2ef49cf056a))
	ROM_LOAD("1vlsh159.u47", 0x0a000, 0x2000, CRC(f91b66ba) SHA1(4f5d0f0562c6a6029ad6d76507091a159983d6f4))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("1cm29.u71", 0x0000, 0x0200, CRC(ef25f5cc) SHA1(51d12f4b8b8712cbd18ec97ec04e1340cd85fc67))
ROM_END

ROM_START( kgbirda )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("4vxfc5341_10c.u87", 0x06000, 0x2000, CRC(21c05874) SHA1(9ddcd34817bc6f88cb2a94374e492d29dd56fb9a))
	ROM_LOAD("4vxfc5341.u86", 0x08000, 0x8000, CRC(3450c544) SHA1(f8883ce3b4bd9073ec6bc985f4666b46f17de092))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("1vlsh159.u20", 0x00000, 0x2000, CRC(df737d57) SHA1(2ec7efe55938ee11376d12d51516c4094ad3fc01))
	ROM_LOAD("1vlsh159.u21", 0x02000, 0x2000, CRC(8e9a65d9) SHA1(e305b8d75b9666377498abf3e2801033effb969b))
	ROM_LOAD("1vlsh159.u22", 0x04000, 0x2000, CRC(5614ee55) SHA1(3eb3872aa8d2b8c2bd798fd46cc715c64bf35714))
	ROM_LOAD("1vlsh159.u45", 0x06000, 0x2000, CRC(cf496cf2) SHA1(cf097835b5f3d5a656ff84063c54d6b1d40703cd))
	ROM_LOAD("1vlsh159.u46", 0x08000, 0x2000, CRC(e155c8d4) SHA1(9c50152dd420d545a88eaea98c2dd2ef49cf056a))
	ROM_LOAD("1vlsh159.u47", 0x0a000, 0x2000, CRC(f91b66ba) SHA1(4f5d0f0562c6a6029ad6d76507091a159983d6f4))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("1cm29.u71", 0x0000, 0x0200, CRC(ef25f5cc) SHA1(51d12f4b8b8712cbd18ec97ec04e1340cd85fc67))
ROM_END

ROM_START( phantomp )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("4vxfc5431.u87", 0x06000, 0x2000, CRC(84e8eeb5) SHA1(95dcbae79b42463480fb3dd2594570070ba1a3ef))
	ROM_LOAD("4vxfc5431.u86", 0x08000, 0x8000, CRC(a6aa3d6f) SHA1(64d97c52355d5d0faebe1ee704f6ad46cc90f0f1))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("1vlsh266.u20", 0x00000, 0x2000, CRC(0f73cf57) SHA1(f99aa9671297d8cefeff86e642af5ea3e7f6f6fb))
	ROM_LOAD("1vlsh266.u21", 0x02000, 0x2000, CRC(2449d69e) SHA1(181d7d093dce1acc332255cab5d56a9043bcab47))
	ROM_LOAD("1vlsh266.u22", 0x04000, 0x2000, CRC(5cb0f179) SHA1(041f7baa5a36f544a98832753ff54ca5238f12c5))
	ROM_LOAD("1vlsh266.u45", 0x06000, 0x2000, CRC(75f94143) SHA1(aac2b0bee1a0d83b25c6fd21f00803209b621543))
	ROM_LOAD("1vlsh266.u46", 0x08000, 0x2000, CRC(6ead5ffc) SHA1(1611d5e2dd5ea06525b6079577a45e713a8065d5))
	ROM_LOAD("1vlsh266.u47", 0x0a000, 0x2000, CRC(c1fb4f23) SHA1(6c9a4e52bd0312c9b49f91a1f563fecd87e5bb82))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("2cm34.u71", 0x0000, 0x0200, CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END

ROM_START( topgear )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("4vxfc969.u87", 0x06000, 0x2000, CRC(5628f477) SHA1(8517905b4d4174fea79e2e3ed38c80fcc6506c6a))
	ROM_LOAD("4vxfc969.u86", 0x08000, 0x8000, CRC(d5afa54e) SHA1(4268c0ddb9beab68348ba520d47bea64b875d8a7))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("1vlsh154.u20", 0x00000, 0x2000, CRC(e3163956) SHA1(b3b55be33fad96858dc683860d72c81ed02b3d97))
	ROM_LOAD("1vlsh154.u21", 0x02000, 0x2000, CRC(9ce936cb) SHA1(cca6ec0190a61cb0b52fbe1b11fb678f5e0960df))
	ROM_LOAD("1vlsh154.u22", 0x04000, 0x2000, CRC(972f091a) SHA1(b94a04e9503fb6f1a687c854076cfc9629ed7b6a))
	ROM_LOAD("1vlsh154.u45", 0x06000, 0x2000, CRC(27fd4204) SHA1(0d082a4297a384c992188dd43be0ecb706117c13))
	ROM_LOAD("1vlsh154.u46", 0x08000, 0x2000, CRC(186f3e3b) SHA1(57f82a79a3d24090f33f5525207d6697e954cdf5))
	ROM_LOAD("1vlsh154.u47", 0x0a000, 0x2000, CRC(dc7d2dab) SHA1(16d223f28b377fafb478d6124fc0eb6d7dd7d591))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("2cm33.u71", 0x0000, 0x0200, BAD_DUMP CRC(ef25f5cc) SHA1(51d12f4b8b8712cbd18ec97ec04e1340cd85fc67)) // Using kgbird's 1CM29 PROM until topgear's 2CM33 PROM is dumped
ROM_END

ROM_START( wtigernz )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("3vxfc5342.u87", 0x06000, 0x2000, CRC(9492b242) SHA1(26bb14cba8e8c3cdbcb4b4903da9592b0a1f8cb3))
	ROM_LOAD("3vxfc5342.u86", 0x08000, 0x8000, CRC(f639ef56) SHA1(5d49deee95df29cd4f5c69fea01bb752aaf2ce99))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("1vlsh157.u20", 0x00000, 0x2000, BAD_DUMP CRC(08624625) SHA1(3c052220b171f8ef009484f0ea38074b538f542b))
	ROM_LOAD("1vlsh157.u21", 0x02000, 0x2000, BAD_DUMP CRC(4bce2fa1) SHA1(8c25cd51ea61a4a9ff1238d1617e38b2cd298c53))
	ROM_LOAD("1vlsh157.u22", 0x04000, 0x2000, BAD_DUMP CRC(da141f20) SHA1(e0ebeeff2e085a30032d29748f5aa6116428aaa8))
	ROM_LOAD("1vlsh157.u45", 0x06000, 0x2000, BAD_DUMP CRC(13783f87) SHA1(662f6afdd027c3d139d7dfcd45a4a2a5a2bf2101))
	ROM_LOAD("1vlsh157.u46", 0x08000, 0x2000, BAD_DUMP CRC(7dfd06ec) SHA1(51fbc3d24e270edb8de432a99ca28695e42e72a6))
	ROM_LOAD("1vlsh157.u47", 0x0a000, 0x2000, BAD_DUMP CRC(177a45ea) SHA1(6b044f88c79de571a007fb71ff2f99587babe474))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("2cm34.u71", 0x0000, 0x0200, CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END

ROM_START( ffortune )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("1vxfc5460.u87", 0x06000, 0x2000, CRC(45047c35) SHA1(4af572a23bca33a360c4711f24fb113167f90447))
	ROM_LOAD("1vxfc5460.u86", 0x08000, 0x8000, CRC(9a8b0eae) SHA1(ffd0419566c2352e3d750040405a760bd75c87d5))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("1vlsh228.u20", 0x00000, 0x2000, CRC(f8bad3c2) SHA1(c3cffeaa34c9c7e8127f69cd1dcbc9d56bd32ed9))
	ROM_LOAD("1vlsh228.u21", 0x02000, 0x2000, CRC(7caba194) SHA1(b0f3f4464ba6a89b572c257b87939457d4f0b2d4))
	ROM_LOAD("1vlsh228.u22", 0x04000, 0x2000, CRC(195967f0) SHA1(f76ba3c4e8b12d480ab1e4c1147bd7971ce8d688))
	ROM_LOAD("1vlsh228.u45", 0x06000, 0x2000, CRC(dc44c3ab) SHA1(74f6230798832f321f7c53c161eac6c552689113))
	ROM_LOAD("1vlsh228.u46", 0x08000, 0x2000, CRC(b0a04c83) SHA1(57247867db6417c525c4c3cdcc409523037e00fd))
	ROM_LOAD("1vlsh228.u47", 0x0a000, 0x2000, CRC(cd24ee39) SHA1(12798e14f7f6308e130da824ffc7c577a36cef04))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("1cm48.u71", 0x0000, 0x0200, CRC(81daeeb0) SHA1(7dfe198c6def5c4ae4ecac488d65c2911fb3a890))
ROM_END

ROM_START( autmoon )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("1vxfc5488.u87", 0x06000, 0x2000, CRC(30ca1eed) SHA1(540635a8b94c14aefa1d8404226d9e1046776111))
	ROM_LOAD("1vxfc5488.u86", 0x08000, 0x8000, CRC(8153a60b) SHA1(54b8a0467645161d827bf8cb9fbceb0d00f9639f))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("1vxfc5488.u20", 0x00000, 0x2000, CRC(fcbbc62e) SHA1(794a7d974e67183468a77a6a81a6f05e0569e229)) // unknown EPROM names, should contain VLSH or VL/SH letters on sticker
	ROM_LOAD("1vxfc5488.u21", 0x02000, 0x2000, CRC(9e6f940e) SHA1(1ad9e7c6231a8d16e868a79d313efccbd1ff58ee))
	ROM_LOAD("1vxfc5488.u22", 0x04000, 0x2000, CRC(1a2ff3a9) SHA1(bddfc3eedcdf9237a31a4b42d062e986beafed39))
	ROM_LOAD("1vxfc5488.u45", 0x06000, 0x2000, CRC(c8d29af8) SHA1(e35f67d6708b26c93617c967aa50c629f7019788))
	ROM_LOAD("1vxfc5488.u46", 0x08000, 0x2000, CRC(fa126a77) SHA1(31d6096c58653a45176b6373835f83c8f2c46f80))
	ROM_LOAD("1vxfc5488.u47", 0x0a000, 0x2000, CRC(50307da0) SHA1(6418a51cf915b37fa11f47d000e4229dacf95951))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("2cm34.u71", 0x0000, 0x0200, CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END

ROM_START( gtroppo ) // MK2.5
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("3vas003.u7", 0x06000, 0x2000, CRC(fe7d0ea4) SHA1(3f3f4809534065c33eca2cfff0d1d2a3e3992406))

		/* GAME EPROMs */
	ROM_LOAD("1vxec542.u9", 0x08000, 0x8000, CRC(09654256) SHA1(234cb74cac92a715f8913b740e69afa57b9b39e8))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("gtroppo.u8",  0x00000, 0x2000, CRC(28ccc30d) SHA1(30f8c44c0b830c81734f515724ba02bb253a956b)) // unknown EPROM names, should contain VLSH or VL/SH letters on sticker
	ROM_LOAD("gtroppo.u10", 0x02000, 0x2000, CRC(fe3cb62a) SHA1(e7e879520b02b50fc0ff8b2c63ae16605cd61f9b)) // reverted to romname.location for now
	ROM_LOAD("gtroppo.u12", 0x04000, 0x2000, CRC(62208b7f) SHA1(c36e6c8ffd05a429251ff39853c0981ec6688a91)) // to avoid duplicate names (there are two U9s as the MK2.5 has graphics ROMs on a separate PCB)
	ROM_LOAD("gtroppo.u9",  0x06000, 0x2000, CRC(79ab593b) SHA1(87408022093542f10890fca027a097cd15dd8039))
	ROM_LOAD("gtroppo.u11", 0x08000, 0x2000, CRC(87ed6fab) SHA1(72428b66d6186dea3bd1f9cfe215341e6b29b3c2))
	ROM_LOAD("gtroppo.u13", 0x0a000, 0x2000, CRC(673a129d) SHA1(cb1ae12e43993bfe399595a8778888eb5a264ec1))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("gtroppo.u40", 0x0000, 0x0200, CRC(918cb0ab) SHA1(2ec37abae2ecae2f0f525daf6fafd03789fca20b))
ROM_END

ROM_START( clkwise ) // MK2.5
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("3vas003.u7", 0x06000, 0x2000, CRC(fe7d0ea4) SHA1(3f3f4809534065c33eca2cfff0d1d2a3e3992406))

		/* GAME EPROMs */
	ROM_LOAD("1vxec534.u9", 0x08000, 0x8000, NO_DUMP) // dead on arrival or blank chip (0xFFFFFFFF throughout ROM)

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("1vlsh101.u8",  0x00000, 0x2000, CRC(424c1e0e) SHA1(168baaa92dd08b58738b491c24b2534d30b770e9))
	ROM_LOAD("1vlsh101.u10", 0x02000, 0x2000, CRC(64792c3a) SHA1(15aa1463c93ed45ca227766e639ff643f1c23f33))
	ROM_LOAD("1vlsh101.u12", 0x04000, 0x2000, CRC(a31bd619) SHA1(60296cf1fa35337076809e827375166340917f01))
	ROM_LOAD("1vlsh101.u9",  0x06000, 0x2000, CRC(59348a2a) SHA1(84c99db54bd75cf9414f306959e7b2c3d7bf9715))
	ROM_LOAD("1vlsh101.u11", 0x08000, 0x2000, CRC(362867bb) SHA1(aba3a74b3bf2a96d8bda4deacada56c5d531bcb4))
	ROM_LOAD("1vlsh101.u13", 0x0a000, 0x2000, CRC(649fbc77) SHA1(22bd81b39279dc393bd791e2e1a2999215581e2b))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("2cm18.u40", 0x0000, 0x0200, BAD_DUMP CRC(918cb0ab) SHA1(2ec37abae2ecae2f0f525daf6fafd03789fca20b)) // Using gtroppo's PROM until clkwise's 2CM18 PROM is dumped
ROM_END

ROM_START( cgold ) // MK2.5
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("3vas003.u7", 0x06000, 0x2000, CRC(fe7d0ea4) SHA1(3f3f4809534065c33eca2cfff0d1d2a3e3992406)) // 3VA/S003/MEM.or 7 Video Sound CARNAVAL
		/* This chip was physically broken in half on arrival, the sticker was the only thing holding it together; luckily this chip is already dumped */

		/* GAME EPROMs */
	ROM_LOAD("3vxec449.u9", 0x08000, 0x8000, CRC(e6643751) SHA1(6eb89fea85bea162fd74888d9efc227cfde25e59)) // original label missing, blank label across EPROM window

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("3vlsh076.u8",  0x00000, 0x2000, CRC(b1eedcd4) SHA1(dcde9cf16da5f361361be45ba134dda084a416fa)) // C-GOLD BLU-L 3VL/SH076/8  C313/D33E GLI-1/95
	ROM_LOAD("3vlsh076.u10", 0x02000, 0x2000, CRC(c1b84d5f) SHA1(898fc03903ec87bf0dc965fa3e90472238df2a98)) // C-GOLD GRN-L 3VL/SH076/10 7691/642B
	ROM_LOAD("3vlsh076.u12", 0x04000, 0x2000, CRC(ec08e24b) SHA1(9dce6952e92c8d10a1722ec0a394b93be6bc7cea)) // C-GOLD RED-L 3VL/SH076/12 UA95/674F
	ROM_LOAD("3vlsh076.u9",  0x06000, 0x2000, CRC(10eff444) SHA1(4658b346add14010efa797ad9d31b6673e8e2526)) // C-GOLD BLU-M 3VL/SH076/9  C41A/C9B3
	ROM_LOAD("3vlsh076.u11", 0x08000, 0x2000, CRC(db156395) SHA1(39439a9610dda608af0a8277728d1fa28f3733bd)) // C-GOLD GRN-M 3VL/SH076/11 348P/3509
	ROM_LOAD("3vlsh076.u13", 0x0a000, 0x2000, CRC(f3cb845a) SHA1(288f7fe991bb60194a9ef9e8c9b2b18ebbd3b49c)) // C-GOLD RED-M 3VL/SH076/13 1CA1/2BB3

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("1cm12.u40", 0x0000, 0x0200, BAD_DUMP CRC(918cb0ab) SHA1(2ec37abae2ecae2f0f525daf6fafd03789fca20b)) // Using gtroppo's PROM until cgold's 1CM12 PROM is dumped; EPROM reader couldn't identify chip
														// 1CM12/40 MV2033*CGOLD/TISLE 943D-GLI 8/93 */
ROM_END

ROM_START( cgold2 )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe)) // unverified, was missing from original dump

		/* GAME EPROMs */
	ROM_LOAD("3xf5182h04.u87", 0x06000, 0x2000, CRC(070a02b2) SHA1(872621275e51c5dca371861a9b9f3038f0dbc8aa))
	ROM_LOAD("3xf5182h04.u86", 0x08000, 0x8000, CRC(5ac1d424) SHA1(42bb8b5eb163a04054621bbcba5cf8203a661baf))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("3vlsh076.u20",   0x00000, 0x2000, BAD_DUMP CRC(b1eedcd4) SHA1(dcde9cf16da5f361361be45ba134dda084a416fa)) // using cgold ROM for now
	ROM_LOAD("3vlsh076_a.u21", 0x02000, 0x2000, CRC(8bf50f7c) SHA1(17705de695d43fa4fa6f1e7afc5c19ecf6f75e35))
	ROM_LOAD("3vlsh076.u22",   0x04000, 0x2000, CRC(ec08e24b) SHA1(9dce6952e92c8d10a1722ec0a394b93be6bc7cea)) // matches cgold
	ROM_LOAD("3vlsh076.u45",   0x06000, 0x2000, BAD_DUMP CRC(10eff444) SHA1(4658b346add14010efa797ad9d31b6673e8e2526)) // using cgold ROM for now
	ROM_LOAD("3vlsh076_a.u46", 0x08000, 0x2000, CRC(9580c2c2) SHA1(8a010fb9e349c066e1af53ed9aa659dbf7dbf17e))
	ROM_LOAD("3vlsh076.u47",   0x0a000, 0x2000, CRC(f3cb845a) SHA1(288f7fe991bb60194a9ef9e8c9b2b18ebbd3b49c)) // matches cgold

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("1cm12.u71", 0x0000, 0x0200, BAD_DUMP CRC(918cb0ab) SHA1(2ec37abae2ecae2f0f525daf6fafd03789fca20b)) // Using gtroppo's PROM for now, assuming both games use 1CM12 as they are almost identical
ROM_END

ROM_START( fhunter )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("2xf5196i01.u87", 0x06000, 0x2000, CRC(f9e6b760) SHA1(af7f16727e84ba8f07400f7f02302862e02d1af4))
	ROM_LOAD("2xf5196i01.u86", 0x08000, 0x8000, CRC(6971ccee) SHA1(1292cfa8125cbaec3bcd9d136cb385a3574bfa4a))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("2xf5196.u20", 0x00000, 0x2000, CRC(96c81134) SHA1(e5e75e8b4897ee7cd9c27b0546fe4006cf384cba)) // unknown EPROM names, should contain VLSH or VL/SH letters on sticker
	ROM_LOAD("2xf5196.u21", 0x02000, 0x2000, CRC(ad7bc6a0) SHA1(145e9a094212841e8a684136ea813bd1bea070fb))
	ROM_LOAD("2xf5196.u22", 0x04000, 0x2000, CRC(450d47bb) SHA1(219a0eeca3989da8cec68405466c9a20f2ee9bfa))
	ROM_LOAD("2xf5196.u45", 0x06000, 0x2000, CRC(560b2417) SHA1(1ed26ceaff87150d2f0115825f952348e34e0414))
	ROM_LOAD("2xf5196.u46", 0x08000, 0x2000, CRC(7704c13f) SHA1(4cfca6ee9e2e543714e8bf0c6de4d9e9406ce250))
	ROM_LOAD("2xf5196.u47", 0x0a000, 0x2000, CRC(a9e6da98) SHA1(3b7d8920d3ef4ae17a55d2e1968318eb3c70264d))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("1cm48.u71", 0x0000, 0x0200, CRC(81daeeb0) SHA1(7dfe198c6def5c4ae4ecac488d65c2911fb3a890))
ROM_END

ROM_START( fhuntera )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("2xf5196i02.u87", 0x06000, 0x2000, CRC(4b532a14) SHA1(98d1753ad1d0d041f81a535947ed501d0eb1d85c))
	ROM_LOAD("2xf5196i01.u86", 0x08000, 0x8000, CRC(6971ccee) SHA1(1292cfa8125cbaec3bcd9d136cb385a3574bfa4a))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("2xf5196.u20", 0x00000, 0x2000, CRC(96c81134) SHA1(e5e75e8b4897ee7cd9c27b0546fe4006cf384cba)) // unknown EPROM names, should contain VLSH or VL/SH letters on sticker
	ROM_LOAD("2xf5196.u21", 0x02000, 0x2000, CRC(ad7bc6a0) SHA1(145e9a094212841e8a684136ea813bd1bea070fb))
	ROM_LOAD("2xf5196.u22", 0x04000, 0x2000, CRC(450d47bb) SHA1(219a0eeca3989da8cec68405466c9a20f2ee9bfa))
	ROM_LOAD("2xf5196.u45", 0x06000, 0x2000, CRC(560b2417) SHA1(1ed26ceaff87150d2f0115825f952348e34e0414))
	ROM_LOAD("2xf5196.u46", 0x08000, 0x2000, CRC(7704c13f) SHA1(4cfca6ee9e2e543714e8bf0c6de4d9e9406ce250))
	ROM_LOAD("2xf5196.u47", 0x0a000, 0x2000, CRC(a9e6da98) SHA1(3b7d8920d3ef4ae17a55d2e1968318eb3c70264d))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("1cm48.u71", 0x0000, 0x0200, CRC(81daeeb0) SHA1(7dfe198c6def5c4ae4ecac488d65c2911fb3a890))
ROM_END

ROM_START( arcwins )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("4xf5227h03.u87", 0x06000, 0x2000, CRC(eec47dcf) SHA1(9d9d56310fc2c69c56aee961d1881328e3aa32d2))
	ROM_LOAD("4xf5227h03.u86", 0x08000, 0x8000, CRC(4e2b955a) SHA1(66202e1c7fe52f706c809d6aa8aa649b54dca4d2))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("4xf5227.u20", 0x00000, 0x2000, CRC(f0438b40) SHA1(cead72e988e8973d95312d62ffd45cb51c982551)) // unknown EPROM names, should contain VLSH or VL/SH letters on sticker
	ROM_LOAD("4xf5227.u21", 0x02000, 0x2000, CRC(0e4c817c) SHA1(dc142d4cf5227496d1e6b82368a8fa186b6372c7))
	ROM_LOAD("4xf5227.u22", 0x04000, 0x2000, CRC(fef65b79) SHA1(38562221ff0513ab973ac96a6ff1e70f0d4e6436))
	ROM_LOAD("4xf5227.u45", 0x06000, 0x2000, CRC(bf7bf9e2) SHA1(32cc8428281f57280ba7aeb7b9a30c51b3a5bec8))
	ROM_LOAD("4xf5227.u46", 0x08000, 0x2000, CRC(c4b2ec7c) SHA1(db0bef392e83a1fb9b1d2255b36a3ec12e73ee1c))
	ROM_LOAD("4xf5227.u47", 0x0a000, 0x2000, CRC(6608d05a) SHA1(7a4014d4dbc8ec6b3dcf14df5a5149696c7ce45e))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("1cm29.u71", 0x0000, 0x0200, CRC(ef25f5cc) SHA1(51d12f4b8b8712cbd18ec97ec04e1340cd85fc67))
ROM_END

/* Video poker games */

ROM_START( wildone )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("3vas003.u59", 0x06000, 0x2000, CRC(fe7d0ea4) SHA1(3f3f4809534065c33eca2cfff0d1d2a3e3992406))

		/* GAME EPROMS */
	ROM_LOAD("4vxec5357.u86", 0x08000, 0x8000, CRC(ad0311b6) SHA1(182efb32556c36f2b6a0fddecc991bc3b0e21dc5))

		/* SHAPE EPROMS */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("8vlsh007.u20", 0x00000, 0x2000, CRC(ff776acb) SHA1(d08a42e72ee639e4303dad3045038c2634d6fba9))
	ROM_LOAD("8vlsh007.u21", 0x02000, 0x2000, CRC(a55dec6f) SHA1(ed19a6d979f6831185d6548c1f12724d3a714854))
	ROM_LOAD("8vlsh007.u22", 0x04000, 0x2000, CRC(6fba7bf3) SHA1(c94ccfb80b51bd1df3da831a7789114b12ac01af))
	ROM_LOAD("8vlsh007.u45", 0x06000, 0x2000, CRC(01c6d826) SHA1(d03dd9843e6666eb4cc90c3fae4de019a1b1611f))
	ROM_LOAD("8vlsh007.u46", 0x08000, 0x2000, CRC(a3bc50dc) SHA1(8cfa4a3415e060be89eb4727eaddb3d64d5f87cb))
	ROM_LOAD("8vlsh007.u47", 0x0a000, 0x2000, CRC(2ba003ea) SHA1(9e4dff2f5d3645ab918b3cc766ca6f5689fc517e))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("2cm07.u71", 0x0000, 0x0200, CRC(1e3f402a) SHA1(f38da1ad6607df38add10c69febf7f5f8cd21744))
ROM_END

ROM_START( gldnpkr ) // MK2.5
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("vidsnd.u7", 0x06000, 0x2000, CRC(568bd63f) SHA1(128b0b085c8b97d1c90baeab4886c522c0bc9a0e)) // unknown EPROM name

		/* GAME EPROMS */
	ROM_LOAD("8vxec037.u9", 0x08000, 0x8000, CRC(a75276b1) SHA1(13950bd26c5f0a26f0dee5938eeee0c16a3119df))

		/* SHAPE EPROMS */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("gldnpkr.u8",  0x00000, 0x2000, CRC(9ebed6c9) SHA1(75741b2f00f6eb1830bf1c5a013fb83e0f0a97b8)) // unknown EPROM names, should contain VLSH or VL/SH letters on sticker
	ROM_LOAD("gldnpkr.u10", 0x02000, 0x2000, CRC(20b58fda) SHA1(9a3441c18f93a6d97637e1b78fd7537b174575fd))
	ROM_LOAD("gldnpkr.u12", 0x04000, 0x2000, CRC(edaa713a) SHA1(61996281ff8e29af058934ee6197bef253c706e6))
	ROM_LOAD("gldnpkr.u9",  0x06000, 0x2000, CRC(d5788ddc) SHA1(f307c179a49d23a0144dfdea69fa4e65c6821032))
	ROM_LOAD("gldnpkr.u11", 0x08000, 0x2000, CRC(e056af8c) SHA1(1ff67c5aed19219a65c1562a971e9968a7e78fad))
	ROM_LOAD("gldnpkr.u13", 0x0a000, 0x2000, CRC(d97876cd) SHA1(23f8b1632c19f2f0a6918a6e4aa987c0feda5cd4))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("2cm07.u40", 0x0000, 0x0200, CRC(1e3f402a) SHA1(f38da1ad6607df38add10c69febf7f5f8cd21744)) // Using 2CM07 until a correct PROM is confirmed
ROM_END

/* Fever Pitch? (2VXEC534, 90.36%)
   Need some proof about the real name.
   Unhandled Mechanical meter 2 pulse: 04 --> Payout pulse.
*/
ROM_START( fvrpitch ) // MK2.5
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("vidsnd.u7", 0x06000, 0x2000, BAD_DUMP CRC(568bd63f) SHA1(128b0b085c8b97d1c90baeab4886c522c0bc9a0e)) // unknown EPROM name

		/* GAME EPROMS */
	ROM_LOAD("2vxec534_fever_pitch_90.36%.u9", 0x08000, 0x8000, CRC(6f8780e8) SHA1(ebf1bfdf2ad727caa2fee34a6ae645ddba42f1cb))  // 90.36%

		/* SHAPE EPROMS */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("1vlbh1299_fever_pitch.u8",  0x00000, 0x2000, CRC(8d6294d2) SHA1(819ab872a3ea99801350dd7bdf07011cbc7689e0)) // unknown EPROM names, should contain VLSH or VL/SH letters on sticker
	ROM_LOAD("1vlbh1299_fever_pitch.u10", 0x02000, 0x2000, CRC(939b30af) SHA1(0253c6b1d336ad589322ee9058c1da68ac1e714a))
	ROM_LOAD("1vlbh1299_fever_pitch.u12", 0x04000, 0x2000, CRC(81913322) SHA1(4ed8b678e38784a41c1a46809a5ecb14256b4c75))
	ROM_LOAD("1vlbh1299_fever_pitch.u9",  0x06000, 0x2000, CRC(e0937d74) SHA1(19f567620e095b10f1d4f2a524331737bfa628b7))
	ROM_LOAD("1vlbh1299_fever_pitch.u11", 0x08000, 0x2000, CRC(bfa3bb9e) SHA1(610de284004906af5a5b594256e7d7ec846afff2))
	ROM_LOAD("1vlbh1299_fever_pitch.u13", 0x0a000, 0x2000, CRC(6d8fb9a6) SHA1(1d8b667eea57f5a4ce173af55f58b9bf56aaa05e))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 ) // Using kgbird's 1CM29 PROM (colors seems correct) until original PROM is dumped.
	ROM_LOAD("1cm29.u71", 0x0000, 0x0200, BAD_DUMP CRC(ef25f5cc) SHA1(51d12f4b8b8712cbd18ec97ec04e1340cd85fc67))
ROM_END


/* 86 Lions */

ROM_START( 86lions )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lion_std.u9", 0xe000, 0x2000, CRC(994842b0) SHA1(72fc31c577ee70b07ce9a4f2e864fe113d32affe) )

	ROM_REGION( 0xc000, "tile_gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "bl0.u8",  0x00000, 0x1000, CRC(00ef4724) SHA1(714fafd035e2befbb35c6d00df52845745e58a93) )
	ROM_LOAD( "gn0.u10", 0x02000, 0x1000, CRC(80dce6f4) SHA1(bf953eba9cb270297b0d0efffe15b926e94dfbe7) )
	ROM_LOAD( "rd1.u12", 0x04000, 0x1000, CRC(350dd017) SHA1(ba273d4231e7e4c44922898cf5a70e8b1d6e2f9d) )
	ROM_LOAD( "bl1.u9",  0x06000, 0x1000, CRC(675e164a) SHA1(99346ca70bfe673b31d71dc6b3bbc3b8f961e87f) )
	ROM_LOAD( "gn1.u11", 0x08000, 0x1000, CRC(80dce6f4) SHA1(bf953eba9cb270297b0d0efffe15b926e94dfbe7) )
	ROM_LOAD( "rd0.u13", 0x0a000, 0x1000, CRC(38c57504) SHA1(cc3ac1df644abc4586fc9f0e88531ba146b86b48) )

	//  ROM_REGION( 0x200, "proms", 0 )
	//  ROM_LOAD( "prom.x", 0x00, 0x20, NO_DUMP )
ROM_END

ROM_START( gunnrose ) // MK2.5
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("gnr.u7", 0x06000, 0x2000, CRC(fe7d0ea4) SHA1(3f3f4809534065c33eca2cfff0d1d2a3e3992406))   // 1VL/SH136 RED AND BLACK

		/* GAME EPROMS */
	ROM_LOAD("gnr.u9", 0x08000, 0x8000, CRC(4fb5f757) SHA1(a4129bca7e573faac0d11de41a9bf8ea144091ee))   // E/C606191SMP

		/* SHAPE EPROMS */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("gnr.u8",  0x00000, 0x2000, CRC(dec9e695) SHA1(a596c4243d6d39e0611ff714e19e14188c90b6f1))  // 1VL/SH136 RED AND BLACK
	ROM_LOAD("gnr.u10", 0x02000, 0x2000, CRC(e83b8e79) SHA1(595f41a5f59f938581a57b445370aa716c6b1409))  // 1VL/SH136 RED AND BLACK
	ROM_LOAD("gnr.u12", 0x04000, 0x2000, CRC(9134d029) SHA1(d698fb91d8f5fa78ffd056149421008d3f12c456))  // 1VL/SH136 RED AND BLACK
	ROM_LOAD("gnr.u9t", 0x06000, 0x2000, CRC(73a0c2cd) SHA1(662056d570eaa069483d378b77efcfb42eff6d0d))  // E/C606191SMP
	ROM_LOAD("gnr.u11", 0x08000, 0x2000, CRC(c50adffe) SHA1(a7c4a3cdd4d5d31a1420e47859408caa75ce2636))  // 1VL/SH136 RED AND BLACK
	ROM_LOAD("gnr.u13", 0x0a000, 0x2000, CRC(e0a6bfc5) SHA1(07e4c8191503f0ea2de4f7ce18fe6290d20ef80e))  // 1VL/SH136 RED AND BLACK

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 ) /* are either of these correct?  They are taken from different games */
	//ROM_LOAD("2cm07.u71", 0x0000, 0x0200, CRC(1e3f402a) SHA1(f38da1ad6607df38add10c69febf7f5f8cd21744)) // Using 2CM07 until a correct PROM is confirmed
	ROM_LOAD("1cm48.u71", 0x0000, 0x0200, BAD_DUMP CRC(81daeeb0) SHA1(7dfe198c6def5c4ae4ecac488d65c2911fb3a890))

ROM_END

GAMEL( 1985, 86lions,  0,        86lions,  aristmk4, aristmk4_state, aristmk4, ROT0, "Aristocrat", "86 Lions", MACHINE_NOT_WORKING, layout_topgear )
GAMEL( 1996, eforest,  0,        aristmk4, eforest,  aristmk4_state, aristmk4, ROT0, "Aristocrat", "Enchanted Forest (12XF528902, US)",         0, layout_eforest  )
GAMEL( 1995, eforesta, eforest,  aristmk4, aristmk4, aristmk4_state, aristmk4, ROT0, "Aristocrat", "Enchanted Forest (4VXFC818, NSW)",          0, layout_aristmk4 ) // 10c, $1 = 10 credits
GAMEL( 1996, eforestb, eforest,  aristmk4, arimk4nz, aristmk4_state, aristmk4, ROT0, "Aristocrat", "Enchanted Forest (3VXFC5343, New Zealand)", 0, layout_arimk4nz ) // 5c, $2 = 40 credits
GAMEL( 1996, 3bagflvt, 0,        aristmk4, 3bagflvt, aristmk4_state, aristmk4, ROT0, "Aristocrat", "3 Bags Full (5VXFC790, Victoria)",          0, layout_3bagflvt ) // 5c, $1 = 20 credits
GAMEL( 1996, 3bagflnz, 3bagflvt, aristmk4, 3bagflnz, aristmk4_state, aristmk4, ROT0, "Aristocrat", "3 Bags Full (3VXFC5345, New Zealand)",      0, layout_3bagflnz ) // 5c, $2 = 40 credits
GAMEL( 1996, kgbird,   0,        aristmk4, kgbird,   aristmk4_state, aristmk4, ROT0, "Aristocrat", "K.G. Bird (4VXFC5341, New Zealand, 5c)",    0, layout_kgbird   ) // 5c, $2 = 40 credits
GAMEL( 1996, kgbirda,  kgbird,   aristmk4, kgbird,   aristmk4_state, aristmk4, ROT0, "Aristocrat", "K.G. Bird (4VXFC5341, New Zealand, 10c)",   0, layout_kgbird   ) // 10c, $2 = 20 credits
GAMEL( 1996, blkrhino, 0,        aristmk4, arimk4nz, aristmk4_state, aristmk4, ROT0, "Aristocrat", "Black Rhino (3VXFC5344, New Zealand)",      0, layout_arimk4nz ) // 5c, $2 = 40 credits
GAMEL( 1996, topgear,  0,        aristmk4, topgear,  aristmk4_state, aristmk4, ROT0, "Aristocrat", "Top Gear (4VXFC969, New Zealand)",          0, layout_topgear  ) // 10c, 1 coin = 1 credit
GAMEL( 1996, wtigernz, 0,        aristmk4, arimk4nz, aristmk4_state, aristmk4, ROT0, "Aristocrat", "White Tiger (3VXFC5342, New Zealand)",      0, layout_arimk4nz ) // 5c, $2 = 40 credits
GAMEL( 1998, phantomp, 0,        aristmk4, arimk4nz, aristmk4_state, aristmk4, ROT0, "Aristocrat", "Phantom Pays (4VXFC5431, New Zealand)",     0, layout_arimk4nz ) // 5c, $2 = 40 credits
GAMEL( 1998, ffortune, 0,        aristmk4, goldenc,  aristmk4_state, aristmk4, ROT0, "Aristocrat", "Fantasy Fortune (1VXFC5460, New Zealand)",  0, layout_goldenc  ) // 5c, $2 = 40 credits
GAMEL( 1998, swtht2nz, 0,        aristmk4, arimk4nz, aristmk4_state, aristmk4, ROT0, "Aristocrat", "Sweet Hearts II (1VXFC5461, New Zealand)",  0, layout_arimk4nz ) // 5c, $2 = 40 credits
GAMEL( 1996, goldenc,  0,        aristmk4, goldenc,  aristmk4_state, aristmk4, ROT0, "Aristocrat", "Golden Canaries (1VXFC5462, New Zealand)",  0, layout_goldenc  ) // 2c, $2 = 100 credits
GAMEL( 1999, autmoon,  0,        aristmk4, arimk4nz, aristmk4_state, aristmk4, ROT0, "Aristocrat", "Autumn Moon (1VXFC5488, New Zealand)",      0, layout_arimk4nz ) // 5c, $2 = 40 credits
GAMEL( 2000, coralr2,  0,        aristmk4, arimk4nz, aristmk4_state, aristmk4, ROT0, "Aristocrat", "Coral Riches II (1VXFC5472, New Zealand)",  0, layout_arimk4nz ) // 2c, $2 = 100 credits
GAMEL( 1995, cgold2,   0,        aristmk4, cgold2,   aristmk4_state, aristmk4, ROT0, "Aristocrat", "Caribbean Gold II (3XF5182H04, USA)",       0, layout_cgold2   )
GAMEL( 1996, fhunter,  0,        aristmk4, fhunter,  aristmk4_state, aristmk4, ROT0, "Aristocrat", "Fortune Hunter (2XF5196I01, USA)",          0, layout_fhunter  )
GAMEL( 1996, fhuntera, fhunter,  aristmk4, fhunter,  aristmk4_state, aristmk4, ROT0, "Aristocrat", "Fortune Hunter (2XF5196I02, USA)",          0, layout_fhunter  )
GAMEL( 1996, arcwins,  0,        aristmk4, arcwins,  aristmk4_state, aristmk4, ROT0, "Aristocrat", "Arctic Wins (4XF5227H03, USA)",             0, layout_arcwins  )
GAMEL( 1997, wildone,  0,  aristmk4_poker, wildone,  aristmk4_state, aristmk4, ROT0, "Aristocrat", "Wild One (4VXEC5357, New Zealand)",         0, layout_wildone  ) // 20c, $2 = 10 credits, video poker
GAMEL( 1993, gunnrose, 0,  aristmk4_poker, wildone,  aristmk4_state, aristmk4, ROT0, "Aristocrat", "Guns and Roses (C606191SMP, Australia)",    MACHINE_WRONG_COLORS, layout_topgear  )
GAMEL( 1986, gldnpkr,  0,  aristmk4_poker, gldnpkr,  aristmk4_state, aristmk4, ROT0, "Ainsworth Nominees P.L.", "Golden Poker (8VXEC037, New Zealand)", 0, layout_gldnpkr ) // possibly 20c, 1 coin = 1 credit, video poker
GAMEL( 1986, gtroppo,  0,        aristmk4, topgear,  aristmk4_state, aristmk4, ROT0, "Ainsworth Nominees P.L.", "Gone Troppo (1VXEC542, New Zealand)",  0, layout_topgear ) // possibly 20c, 1 coin = 1 credit
GAMEL( 1986, clkwise,  0,        aristmk4, topgear,  aristmk4_state, aristmk4, ROT0, "Ainsworth Nominees P.L.", "Clockwise (1VXEC534, New Zealand)",    MACHINE_NOT_WORKING, layout_topgear ) // 20c, 1 coin = 1 credit
GAMEL( 1986, cgold,    0,        aristmk4, topgear,  aristmk4_state, aristmk4, ROT0, "Ainsworth Nominees P.L.", "Caribbean Gold (3VXEC449, USA)",       0, layout_topgear ) // 25c, 1 coin = 1 credit
GAMEL( 1986, fvrpitch, 0,        aristmk4, fvrpitch, aristmk4_state, aristmk4, ROT0, "Ainsworth Nominees P.L.", "Fever Pitch? (2VXEC534, NSW, 90.36%)", 0, layout_fvrpitch  ) // 5c, $1 = 20 credits
