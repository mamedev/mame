// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Palindrome, FraSher, Roberto Fresca
/**************************************************************************************************

    Driver: aristmk4

    Manufacturer: Aristocrat Leisure Industries ( aka Ainsworth Nominees P.L. )
    Platform: Aristocrat 540 Video ( MK 2.5 Video / MK IV )
    Driver by Palindrome & FraSher

    original 86lions.c driver by Chris Hardy, Angelo Salese & Roberto Fresca


    ***************** INITIALISATION **************************************************************

    Method 1 :
    * Key in with the Jackpot Key followed by the Audit Key (F1 then F2 keys)
    * Hit UP (W key) twice (for gunnrose only)
    * Press PB4, PB5 and PB6 keys simultaneously (Z+X+C keys by default)
    * A value (displayed below) will appear next to RF/AMT on the right of the screen
    * Key out both the Jackpot and Audit Keys

    This method works with the following games:
    3bagfull   200
    3bagfullnz 200
    autmoon    200
    blkrhino   200
    blkrhinonz 200
    cgold      500
    coralr2    200
    eforest    200
    eforestnz  200
    ffortune   200
    fvrpitch   200
    gldnpkr    400
    goldenc    200
    grnlizrd   200
    gtroppo    500
    kgbirdnz   200
    kgbirdnza  200
    phantomp   200
    swtht2nz   200
    teqsun     200
    wildone    200
    wtigernz   200
    gunnrose   N/A (no hopper)

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
    topgear    500

    Method 3 :
    * Key in with the Jackpot Key followed by the Audit Key.
    * Press PB4, PB5 and PB6 keys simultaneously (Z+X+C keys by default)
    * Press Service (default A) 4 times until you are in the Setup Screen, showing Printer Pay Limit etc.
    * Press Bet 2 (default D) to change the Jackpot Win Limit. A higher value is better (3000 max)
    * Key out both the Jackpot and Audit Keys

    This method works with the following games:
    3bagfullu
    arcwins
    arcwinsa
    cgold2
    cgold2a
    dblagent
    eforestu
    gambler
    fhunter
    fhuntera
    kgbird
    letsgof
    thundhrt
    trktreat
    wtiger

    Note: cgold2a, fhunter, fhuntera and wtiger must have DIP labeled "5201-5" switch to on at all times.
       This allows setup procedure to complete properly and game to play (if disabled, the games don't accept inputs).
        - The other US games (excluding cgold) all run slower when 5201-5 is off.

***************************************************************************************************

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

    The AY8910 named ay1 has writes on PORT B to the ZN434 DA converter.
    The AY8910 named ay2 has writes to lamps and the light tower on Port A and B. These are implemented via the layout.


***************************************************************************************************

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
    Implement Port '5005' for eforestu. Changing this only works after performing memory reset.(delete nvram file)
    First 3 bits
    000 = $100 / Credit
    001 = 50c  / Credit
    010 = $5   / Credit
    011 = 10c  / Credit
    100 = $10  / Credit
    101 = 25c  / Credit
    110 = $1   / Credit
    111 = 5c   / Credit (default)
    Implement Bill Acceptor for eforestu to add credits.
    arcwins and eforestu are now working.

    21/02/2013 - Heihachi_73
    Added new game Caribbean Gold (cgold), however it is not a straight swap as it has slightly different input locations:
     - With unmodified 0.148 source, game complains about logic door being open,
        which is seemingly tied to the current coin input.
     - When HOPCO2 is toggled off/on quickly (default is on otherwise it will
        cause a note acceptor error), the note acceptor works, adding 4 credits ($1?).
        This is seemingly a quarter slot (25c). Not sure if other notes are possible.
     - Same gameplay as Gone Troppo, one interesting thing about this game is that
        the KQJ symbols have actual faces instead of plain letters. These symbols were also found on some mechanical reel slots.

    08/03/2013 - Heihachi_73
    Cleaned up comments and erroneous ROM names (e.g. graphics ROMs named after the program ROM).
    Caribbean Gold II - copied cgold graphics ROMs u8+u11 (aka u20+u45) to cgold2, game now playable.
     Tiles 0x64 and 0x65 are used to show the game's denomination (credit value), however cgold does
     not use these tiles (there are seemingly unused line/bet/number tiles in this location), this
     causes a graphics glitch on the $/cent signs.
    Promoted Fortune Hunter and clone to working status, as they were in fact working for quite a while.
    Fixed ROM names for kgbirdnz/kgbirdnza; 5c and 10c variants were mixed up.

    11/12/2014 - Lord-Data
    Added hopper and meter outputs.

    27/03/2014
    Added new game: Guns and Roses Poker - gunnrose

    13/11/2015 - Roberto Fresca
    Added new game: Fever Pitch (2VXEC534, NSW, 90.36%).

    26/04/2017 - Heihachi_73
    Added button labels and layout to gunnrose, position of buttons is guesswork at this stage.
    This game does not have a cashout button. Credits are paid out using the jackpot key.
    Changed jackpot and audit keys to F1 and F2 respectively.
    Renamed nodump program ROM in clkwise as it had the wrong EPROM number.

    28/11/2024 - Brian Troha and Heihachi_73
    Added ten new US region dumps: 3 Bags Full, Arctic Wins, Caribbean Gold II, Double Agent,
     K.G. Bird, Let's Go Fishing, The Gambler, Thunder Heart, Trick or Treat and White Tiger.
    The new Caribbean Gold II dump has good graphics ROMs, fixing the existing set, however the color PROM is still missing.
    Trick or Treat has a bad dump of the U21 ROM resulting in garbled graphics.
    White Tiger US version is now the parent set.
     - White Tiger NZ has had its bad U20 graphics ROM repaired by hand where possible using the US version, however the ROMs are not an exact match.
       The remaining graphics ROMs all seemed fine, although there is a stray dot in the heart card graphic caused by a suspicious byte at location 0x0B0C in U46.

***************************************************************************************************

    When the games first power on (or when reset), they will display a TILT message on the screen.
    This doesn't affect gameplay, and if there are no pending errors the game should coin up and/or play immediately.

    The tilt message will also appear when an error code is displayed, such as the main door being opened/closed, or
    a hardware error/fault (such as hopper empty, coin yoyo, printer errors; none of which should happen in MAME however).

    The tilt message will disappear if you turn the Audit Key on and off, or after you start playing.
    Despite the name, there is no 'tilt' mechanism in the machine and it is nothing to worry about.

    These games do not feature a backup mechanism in case of power faults or system crashes requiring a reboot.
    If the player was in the middle of a spin or watching a win count up, any credits won on that spin will be voided.
    On the machine's artwork, this is reflected with text reading 'Malfunction voids all pays and plays', of which
    the text has also been carried onto later machines. The Aristocrat MK5 and later systems however feature backup
    mechanisms and will repeat the last game (including free game features and/or gamble selection) when powered on,
    to where the player had left off.

    Some games have grahpical issues when DIP SW7 is set to any value other than off/off.
    It is unknown whether this occurs on original hardware or whether it is an emulation bug.

    cgold/2/2a, gtroppo, topgear and trktreat require DIP SW7 to be set to off/off or else the second screen bonus will be broken.

    In wildone, the dollar sign on the Insert $2 graphic is the wrong colour on other settings as well. It only appears
    correctly when SW7 is off/off. This is probably a bug in the original game, where the graphic designers have used the
    wrong palette for the background of the dollar sign.

    From these findings, it is noted that the off/off setting may in fact be the default background setting of all games.

    cgold, gtroppo and topgear are non-multiplier, 5 payline games, therefore, you cannot bet higher than 5 credits on these machines.

    cgold can be set to credit play or coin play by toggling SW1-5. If SW1-5 is on, game is in credit mode; if SW1-5 is off,
    wins and remaining credits will be automatically paid out as coins.

    Non-US games can enable/disable the double up (gamble) option by toggling the SW1-8 switch. Turning SW1-8 off will enable
    the double up option (default); turning SW1-8 on will disable double up and enable auto-spin on some games.
    So far, only 3bagfull, blkrhino, eforest and grnlizrd allow automatic play; other games simply ignore the buttons.
    The games respond slightly faster between games with double up disabled.

    3bagfull/nz/u, cgold/2/2a, fvrpitch, fhunter/a, gtroppo, letsgof, topgear, trktreat and wtiger do not have a double up option,
    and US-based games ignore this switch setting (double up is always enabled on the US games which support it).

    New Zealand games will end the current spin and void any remaining free games upon reaching the maximum win of $500.00.

    Aristocrat games made prior to 1993 have a default 1986 Ainsworth copyright string even though the games may be as late as 1992.

    US versions of 3 Bags Full, Fortune Hunter and Let's Go Fishing all have the same program ID of 5196, however the version number differs between games.
    Likewise, US versions of Enchanted Forest, The Gambler and Thunder Heart all have the same program ID of 5289, again with different version numbers.
     - 3bagfullu and fhunter/a display THREE BAGS FULL FEATURE at the top of the screen in the bonus feature, letsgof has the text removed.
     - eforestu and gambler display ENCHANTED WIN when a wild substitutes, thndrhrt shows BONUS WIN.

    TODO:

    1. ROMs need redumping for the following games:
     - trktreat and wtigernz have bad graphics ROMs.
     - clkwise needs its program ROM redumped, original dump was 32K of 0xFF's. Graphics and video/sound ROM are OK.
     - U71/U40 PROM dumps are needed for cgold/cgold2 (1CM12), clkwise (2CM18), dblagent (unknown), fvrpitch (unknown)
        gldnpkr (unknown), gunnrose (unknown), teqsun (unknown), and topgear (2CM33).
        - fvrpitch and teqsun may actually have the correct PROMs as their colours match photos.

    2. Video poker and Keno button panels needed. 06/06/2011: Video poker panels done, however they need confirmation
       with a real machine.

    3. Extend the driver to use the keno keyboard input for keno games (no MK2.5/MKIV Keno games dumped yet as of 28/02/2010).

    4. Provide complete cashcade emulation.

    5. Look into what the hopper probe signal is for.

    6. Investigate issues with the Poker style games as described below.

    7. DIP switches need verifying as the descriptions don't match between games.

    8. When DIP SW7 is set to off/off, emulation speed is reduced.

    9. Rewrite video emulation by using custom drawing code.

    10. Fix coin input for the US games. Currently, only the note acceptor works. The reverse is true for cgold.

    11. Fix RTC as the hour is buggy e.g. 11PM shows 91:00:00.

    12. Fix 86 Lions (pre-Aristocrat MK4 HW, without PROM and dunno what else).

    13. Hook up native/Jubilee MK4 games which have larger capacity ROMs (no native MK4 games are dumped yet - all games are MK2.5 format).


    ***************** POKER GAMES *****************************************************************

    wildone & gldnpkr have a problem where the second branch condition is always true, see assembler below for
    example of wildone.

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

    06/12/2024: Is this code and comment still needed? The code is from 2010 when the driver was first added using an MC6845 instead of R6545.
     - All three video poker games seem to run OK without the aristmk4_poker setting. - Heihachi_73


**************************************************************************************************/

#define MAIN_CLOCK  12_MHz_XTAL

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
#include "machine/timer.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

// Button panel and lamps
#include "aristmk4.lh"   // AU 90cr with gamble
#include "arimk4nz.lh"   // NZ 45cr with double up
#include "3bagfull.lh"   // AU 90cr without gamble
#include "3bagfullnz.lh" // NZ 45cr without gamble
#include "arcwins.lh"    // US 25cr with gamble
#include "cgold2.lh"     // US 25cr without gamble
#include "eforestu.lh"   // US 45cr with gamble
#include "fhunter.lh"    // US 45cr without gamble
#include "fvrpitch.lh"   // AU 25cr without gamble
#include "goldenc.lh"    // NZ 90cr with double up
#include "grnlizrd.lh"   // AU 50cr with gamble
#include "kgbirdnz.lh"   // NZ 25cr with double up
#include "teqsun.lh"     // AU 25cr with gamble, 12-button panel with no bet 4/play 4 buttons
#include "topgear.lh"    // NZ 5 line without gamble
#include "gldnpkr.lh"    // Video poker
#include "gunnrose.lh"   // Video poker
#include "wildone.lh"    // Video poker


namespace {

class aristmk4_state : public driver_device
{
public:
	aristmk4_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_rtc(*this, "rtc"),
		m_ay1(*this, "ay1"),
		m_ay2(*this, "ay2"),
		m_samples(*this, "samples"),
		m_mkiv_vram(*this, "mkiv_vram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_credit_spend_meter(*this, "creditspendmeter"),
		m_credit_out_meter(*this, "creditoutmeter"),
		m_hopper_motor_out(*this, "hopper_motor"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void aristmk4_poker(machine_config &config);
	void aristmk4(machine_config &config);
	void _86lions(machine_config &config);

	void init_aristmk4();

private:
	required_device<cpu_device> m_maincpu;
	required_device<mc146818_device> m_rtc;
	required_device<ay8910_device> m_ay1;
	required_device<ay8910_device> m_ay2;
	required_device<samples_device> m_samples;

	required_shared_ptr<uint8_t> m_mkiv_vram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	output_finder<> m_credit_spend_meter;
	output_finder<> m_credit_out_meter;
	output_finder<> m_hopper_motor_out;
	output_finder<21> m_lamps;

	int m_rtc_address_strobe = 0;
	int m_rtc_data_strobe = 0;
	uint8_t *m_shapeRomPtr = nullptr;
	uint8_t m_shapeRom[0xc000]{};
	std::unique_ptr<uint8_t[]> m_nvram{};
	uint8_t m_psg_data = 0;
	int m_ay8910_1 = 0;
	int m_ay8910_2 = 0;
	int m_u3_p0_w = 0;
	uint8_t m_cgdrsw = 0;
	uint8_t m_ripple = 0;
	int m_hopper_motor = 0;
	int m_inscrd = 0;
	int m_insnote = 0;
	int m_cashcade_c = 0;
	int m_printer_motor = 0;
	emu_timer *m_power_timer = nullptr;
	emu_timer *m_note_reset_timer = nullptr;
	emu_timer *m_coin_reset_timer = nullptr;
	emu_timer *m_hopper_reset_timer = nullptr;

	uint8_t ldsw();
	uint8_t cgdrr();
	void cgdrw(uint8_t data);
	void u3_p0(uint8_t data);
	uint8_t u3_p2();
	uint8_t u3_p3();
	uint8_t bv_p0();
	uint8_t bv_p1();
	uint8_t mkiv_pia_ina();
	void mkiv_pia_outa(uint8_t data);
	void mlamps(uint8_t data);
	uint8_t cashcade_r();
	void mk4_printer_w(uint8_t data);
	uint8_t mk4_printer_r();
	void mkiv_pia_ca2(int state);
	void mkiv_pia_cb2(int state);
	void mkiv_pia_outb(uint8_t data);
	uint8_t via_a_r();
	uint8_t via_b_r();
	void via_a_w(uint8_t data);
	void via_b_w(uint8_t data);
	void via_ca2_w(int state);
	void via_cb2_w(int state);
	void pblp_out(uint8_t data);
	void pbltlp_out(uint8_t data);
	void zn434_w(uint8_t data);
	uint8_t pa1_r();
	uint8_t pb1_r();
	uint8_t pc1_r();

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	void aristmk4_palette(palette_device &palette) const;
	void lions_palette(palette_device &palette) const;
	uint32_t screen_update_aristmk4(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(note_input_reset);
	TIMER_CALLBACK_MEMBER(coin_input_reset);
	TIMER_CALLBACK_MEMBER(hopper_reset);
	TIMER_CALLBACK_MEMBER(power_fail);
	inline void uBackgroundColour();

	void slots_mem(address_map &map) ATTR_COLD;
	void poker_mem(address_map &map) ATTR_COLD;
};

/* Partial Cashcade protocol */
static const uint8_t cashcade_p[] ={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xf0};

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

uint32_t aristmk4_state::screen_update_aristmk4(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
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

uint8_t aristmk4_state::ldsw()
{
	int U3_p2_ret= ioport("5002")->read();
	if(U3_p2_ret & 0x1)
	{
	return 0;
	}
	return m_cgdrsw = ioport("5005")->read();
}

uint8_t aristmk4_state::cgdrr()
{
	if(m_cgdrsw) // is the LC closed
	{
	return m_ripple; // return a positive value from the ripple counter
	}
	return 0x0; // otherwise the counter outputs are set low.
}

void aristmk4_state::cgdrw(uint8_t data)
{
	m_ripple = data;
}

void aristmk4_state::u3_p0(uint8_t data)
{
	m_u3_p0_w = data;

	if ((data&0x80)==0) // Printer Motor Off
	{
		m_printer_motor = 1; // Set this so the next read of u3_p3 returns PTRHOM as OFF.
	}

	//logerror("u3_p0_w: %02X\n",m_u3_p0_w);
}

uint8_t aristmk4_state::u3_p2()
{
	int u3_p2_ret= ioport("5002")->read();
	int u3_p3_ret= ioport("5003")->read();

	m_lamps[19] = BIT(u3_p2_ret, 4);
	m_lamps[20] = BIT(u3_p3_ret, 2);

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
	{
		u3_p2_ret=u3_p2_ret^0x02; // CBOPT2
		m_inscrd++; // increment so that coin input can go to next phase in aristmk4_state::via_b_r() below
	}

	return u3_p2_ret;
}

uint8_t aristmk4_state::u3_p3()
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
	m_insnote=0; // Reset note input after 150msec
}

uint8_t aristmk4_state::bv_p0()
{
	int bv_p0_ret=0x00;

	switch(m_insnote)
	{
	case 0x01:
		bv_p0_ret=ioport("NS")->read()+0x81; // Check note selector
		m_insnote++;
		break;
	case 0x02:
		bv_p0_ret=0x89;
		m_insnote++;
		m_note_reset_timer->adjust(attotime::from_msec(150));
		break;
	default:
		break; // Timer will reset the input
	}

	return bv_p0_ret;

}

uint8_t aristmk4_state::bv_p1()
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
uint8_t aristmk4_state::mkiv_pia_ina()
{
	/* uncomment this code once RTC is fixed */

	//return m_rtc->data_r(); // Shows wrong PM hour
	return 0;
	// Note: Aussie boards have no RTC fitted, so this is technically valid
}

//output a
void aristmk4_state::mkiv_pia_outa(uint8_t data)
{
	if(m_rtc_data_strobe)
	{
		m_rtc->data_w(data);
		//logerror("rtc protocol write data: %02X\n",data);
	}
	else
	{
		m_rtc->address_w(data);
		//logerror("rtc protocol write address: %02X\n",data);
	}
}

//output ca2
void aristmk4_state::mkiv_pia_ca2(int state)
{
	m_rtc_address_strobe = state;
	// logerror("address strobe %02X\n", address_strobe);
}

//output cb2
void aristmk4_state::mkiv_pia_cb2(int state)
{
	m_rtc_data_strobe = state;
	//logerror("data strobe: %02X\n", data);
}

//output b
void aristmk4_state::mkiv_pia_outb(uint8_t data)
{
	uint8_t emet[5];
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
					m_credit_spend_meter = emet[i];
					break;
				case 5:
					m_credit_out_meter = emet[i];
					break;
				default:
					//Uncomment when adding new games to check for unhandled mech meter pulse
					//printf("Unhandled Mechanical meter %d pulse: %02d\n",i+1, emet[i]);
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
					m_credit_spend_meter = 0;
					break;
				case 5:
					m_credit_out_meter = 0;
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
	"*3bagfull",
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
	m_hopper_motor_out = 1;
}

// Port A read (SW1)
uint8_t aristmk4_state::via_a_r()
{
	int psg_ret=0;

	if (m_ay8910_1&0x03) // SW1 read.
	{
		psg_ret = m_ay1->data_r();
		//logerror("PSG porta ay1 returned %02X\n",psg_ret);
	}

	else if (m_ay8910_2&0x03) //i don't think we read anything from Port A on ay2, Can be removed once game works ok.
	{
		psg_ret = m_ay2->data_r();
		//logerror("PSG porta ay2 returned %02X\n",psg_ret);
	}
	return psg_ret;
}

uint8_t aristmk4_state::via_b_r()
{
	int ret=ioport("via_port_b")->read();

/*
    Not expecting to read anything from port B on the AY8910's ( controls BC1, BC2 and BDIR )
    However there are extra 4 bits not going to the AY8910's on the schematics, which get read from here.
    OPTA1  - Bit4 - Coin optics - A
    OPTB1  - Bit5 - Coin optics - B
    HOPCO1 - Bit6 - Hopper counter
    CBOPT1 - Bit7 - Cash box optics

    Coin input... CBOPT2 goes LOW, then the optic detectors OPTA1 / OPTB1 detect the coin passing
    The timer causes one credit, per 150ms or so...

    US games also need CBOPT1 to go high after OPTB1 for coins to register
    This may also be why some non-US games trigger a coin diverter error when CBOPT is held low
*/

	switch(m_inscrd)
	{
	case 0x00:
		break;
//  case 0x01 is CBOPT2 aka 5002-1, not via_port_b
	case 0x02: // Coin Optic A
		ret=ret^0x10;
		m_inscrd++;
		break;
	case 0x03: // Coin Optic B
		ret=ret^0x20;
		m_inscrd++;
		m_coin_reset_timer->adjust(attotime::from_msec(150));
		break;
	//case 0x04: // CBOPT1
			//Uncomment when CBOPT2 is hooked up to coin input
		//ret=ret^0x80;
		//m_inscrd++;
		//break;
	default:
		break; //timer will reset the input
	}

// if user presses collect.. send coins to hopper.

	switch(m_hopper_motor)
	{
	case 0x00:
		ret=ret^0x40; // HOPCO1
		m_hopper_reset_timer->adjust(attotime::from_msec(175));
		m_hopper_motor = 0x02;
		m_hopper_motor_out = 2;
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

void aristmk4_state::via_a_w(uint8_t data)
{
	//logerror("VIA port A write %02X\n",data);
	m_psg_data = data;
}

void aristmk4_state::via_b_w(uint8_t data)
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
		m_ay1->data_w(m_psg_data);
		//logerror("VIA Port A write data ay1: %02X\n",m_psg_data);
		break;
	}
	case 0x07:  //LATCH Address (set register)
	{
		m_ay1->address_w(m_psg_data);
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
		m_ay2->data_w(m_psg_data);
		//logerror("VIA Port A write data ay2: %02X\n",m_psg_data);
		break;
	}
	case 0x07:  //LATCH Address (set register)
	{
		m_ay2->address_w(m_psg_data);
		//logerror("VIA Port B write register ay2: %02X\n",m_psg_data);
		break;
	}
		default:
		//logerror("Unknown PSG state on ay2: %02X\n",m_ay8910_2);
		break;
	}
}

void aristmk4_state::via_ca2_w(int state)
{
	// CA2 is connected to CDSOL1 on schematics ?
	//logerror("VIA Port CA2 write %02X\n",data) ;
}

void aristmk4_state::via_cb2_w(int state)
{
	// CB2 = hopper motor (HOPMO1). When it is 0x01, it is not running (active low)
	// when it goes to 0, we're expecting to coins to be paid out, handled in via_b_r
	// as soon as it is 1, HOPCO1 to remain 'ON'

	if (state == 0x01)
		m_hopper_motor = state;
	else if (m_hopper_motor < 0x02)
		m_hopper_motor = state;

	m_hopper_motor_out = m_hopper_motor;
}

// Lamp output

void aristmk4_state::pblp_out(uint8_t data)
{
	m_lamps[ 1] = BIT(data, 0);
	m_lamps[ 5] = BIT(data, 1);
	m_lamps[ 9] = BIT(data, 2);
	m_lamps[11] = BIT(data, 3);
	m_lamps[ 3] = BIT(data, 4);
	m_lamps[ 4] = BIT(data, 5);
	m_lamps[ 2] = BIT(data, 6);
	m_lamps[10] = BIT(data, 7);
	//logerror("Lights port A %02X\n",data);
}

void aristmk4_state::pbltlp_out(uint8_t data)
{
	m_lamps[ 8] = BIT(data, 0);
	m_lamps[12] = BIT(data, 1);
	m_lamps[ 6] = BIT(data, 2);
	m_lamps[ 7] = BIT(data, 3);
	m_lamps[14] = BIT(data, 4); // light tower
	m_lamps[15] = BIT(data, 5); // light tower
	m_lamps[16] = BIT(data, 6); // light tower
	m_lamps[17] = BIT(data, 7); // light tower
	//logerror("Lights port B: %02X\n",data);
}

void aristmk4_state::mlamps(uint8_t data)
{
	/* TAKE WIN AND GAMBLE LAMPS */
	m_lamps[18] = BIT(data, 5);
	m_lamps[13] = BIT(data, 6);
}

void aristmk4_state::zn434_w(uint8_t data)
{
	// Introduced to prevent warning in log for write to AY1 PORT B
	// this is a write to the ZN434 DA converters..
}


uint8_t aristmk4_state::cashcade_r()
{
	/* work around for cashcade games */
	return cashcade_p[(m_cashcade_c++)%15];
}

void aristmk4_state::mk4_printer_w(uint8_t data)
{
	//logerror("Printer: %c %d\n",data,data);
}

uint8_t aristmk4_state::mk4_printer_r()
{
	return 0;
}

/******************************************************************************

ADDRESS MAP - SLOT GAMES

******************************************************************************/

void aristmk4_state::slots_mem(address_map &map)
{
	map(0x0000, 0x17ff).ram().share("mkiv_vram"); // video RAM - chips U49 / U50
	map(0x1800, 0x1800).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x1801, 0x1801).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x1c00, 0x1cff).w(FUNC(aristmk4_state::mk4_printer_w));
	map(0x1900, 0x19ff).r(FUNC(aristmk4_state::mk4_printer_r));
	map(0x2000, 0x3fff).rom();  // graphics ROM map
	map(0x4000, 0x4fff).ram().share("nvram");

	map(0x5000, 0x5000).w(FUNC(aristmk4_state::u3_p0));
	map(0x5002, 0x5002).r(FUNC(aristmk4_state::u3_p2));
	map(0x5003, 0x5003).r(FUNC(aristmk4_state::u3_p3));
	map(0x5005, 0x5005).r(FUNC(aristmk4_state::ldsw));
	map(0x500d, 0x500d).portr("500d");
	map(0x500e, 0x500e).portr("500e");
	map(0x500f, 0x500f).portr("500f");
	map(0x5010, 0x501f).m("via6522_0", FUNC(via6522_device::map));
	map(0x5200, 0x5200).r(FUNC(aristmk4_state::cashcade_r));
	map(0x5201, 0x5201).portr("5201");
	map(0x52c0, 0x52c0).r(FUNC(aristmk4_state::bv_p0));
	map(0x52c1, 0x52c1).r(FUNC(aristmk4_state::bv_p1));
	map(0x527f, 0x5281).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x5300, 0x5300).portr("5300");
	map(0x5380, 0x5383).rw("pia6821_0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));  // RTC data - PORT A, mechanical meters - PORT B ??
	map(0x5440, 0x5440).w(FUNC(aristmk4_state::mlamps)); // Take Win and Gamble lamps
	map(0x5468, 0x5468).rw(FUNC(aristmk4_state::cgdrr), FUNC(aristmk4_state::cgdrw)); // 4020 ripple counter outputs
	map(0x6000, 0xffff).rom();  // game ROMs
}

/******************************************************************************

ADDRESS MAP - VIDEO POKER GAMES

******************************************************************************/

/*
Poker card style games seem to have different address mapping

The graphics rom is mapped from 0x4000 - 0x4fff

The U87 personality rom is not required, therefore game rom code mapping is from 0x8000-0xffff
*/

void aristmk4_state::poker_mem(address_map &map)
{
	map(0x0000, 0x17ff).ram().share("mkiv_vram"); // video RAM - chips U49 / U50
	map(0x1800, 0x1800).rw("crtc", FUNC(mc6845_device::status_r), FUNC(mc6845_device::address_w));
	map(0x1801, 0x1801).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x1c00, 0x1cff).w(FUNC(aristmk4_state::mk4_printer_w));
	map(0x1900, 0x19ff).r(FUNC(aristmk4_state::mk4_printer_r));
	map(0x4000, 0x4fff).ram().share("nvram");

	map(0x5000, 0x5000).w(FUNC(aristmk4_state::u3_p0));
	map(0x5002, 0x5002).r(FUNC(aristmk4_state::u3_p2));
	map(0x5003, 0x5003).portr("5003");
	map(0x5005, 0x5005).r(FUNC(aristmk4_state::ldsw));
	map(0x500d, 0x500d).portr("500d");
	map(0x500e, 0x500e).portr("500e");
	map(0x500f, 0x500f).portr("500f");
	map(0x5010, 0x501f).m("via6522_0", FUNC(via6522_device::map));
	map(0x5200, 0x5200).r(FUNC(aristmk4_state::cashcade_r));
	map(0x5201, 0x5201).portr("5201");
	map(0x52c0, 0x52c0).r(FUNC(aristmk4_state::bv_p0));
	map(0x52c1, 0x52c1).r(FUNC(aristmk4_state::bv_p1));
	map(0x527f, 0x5281).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x5300, 0x5300).portr("5300");
	map(0x5380, 0x5383).rw("pia6821_0", FUNC(pia6821_device::read), FUNC(pia6821_device::write));  // RTC data - PORT A, mechanical meters - PORT B ??
	map(0x5440, 0x5440).w(FUNC(aristmk4_state::mlamps)); // Take Win and Gamble lamps
	map(0x5468, 0x5468).rw(FUNC(aristmk4_state::cgdrr), FUNC(aristmk4_state::cgdrw)); // 4020 ripple counter outputs
	map(0x6000, 0x7fff).rom();  // graphics ROM map
	map(0x8000, 0xffff).rom();  // game ROMs
}

/******************************************************************************

INPUT PORTS

******************************************************************************/

static INPUT_PORTS_START(aristmk4)

	PORT_START("via_port_b")
	PORT_DIPNAME( 0x10, 0x00, "Coin Optic 1" ) // COIN-IN PHOTO-OPTIC A                                                     "22 - COIN IN FAULT" if held high
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("AY:1")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Coin Optic 2" ) // COIN-IN PHOTO-OPTIC B                                                     "22 - COIN IN FAULT" if held high
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("AY:2")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Hopper Coin Release") PORT_CODE(KEYCODE_BACKSLASH)        // "ILLEGAL COIN PAID"
	PORT_DIPNAME( 0x80, 0x80, "CBOPT1" ) // Cash Box Optic 1, enable for 3bagfull, blkrhino, eforest and grnlizrd otherwise they will give a coin diverter error on the 5th coin, turn off for US games
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) // When toggled on/off quickly, increments coin drop in US games             "34 - COIN VALIDATOR FAULT" if held high in US games
	PORT_DIPSETTING(    0x80, DEF_STR( On ) ) PORT_DIPLOCATION("AY:4")

	PORT_START("5002")
	PORT_DIPNAME( 0x01, 0x00, "HOPCO2") // coins out hopper 2, BILL VALIDATOR DOOR in cgold2, eforestu
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5002:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "CBOPT2") // Cash Box Optic 2 - coin in cash box 2, LOGIC DOOR in cgold                       "34 - COIN VALIDATOR FAULT" if held low
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5002:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "HOPHI2") // hopper 2 full, CASH DOOR in cgold2, eforestu
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5002:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "DOPTI") // photo optic door        DOOR OPEN SENSE SWITCH                                    "61 - MAIN DOOR SWITCH FAULT" if held low
	PORT_DIPSETTING(    0x00, DEF_STR( On ) ) PORT_DIPLOCATION("5002:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN ) PORT_NAME("Audit Key") PORT_TOGGLE PORT_CODE(KEYCODE_F2) // AUDTSW
	PORT_DIPNAME( 0x20, 0x00, "HOPLO1") // hopper 1 low
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5002:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "HOPLO2") // hopper 2 low, CASH-BOX DOOR in cgold
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5002:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON11 ) PORT_NAME("Robot Test / Hopper Reset") PORT_CODE(KEYCODE_Z) // PB6

	PORT_START("5003")
	PORT_DIPNAME( 0x01, 0x00, "OPTAUI") // opto audit in
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5003:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR ) PORT_NAME("Main Door") PORT_TOGGLE PORT_CODE(KEYCODE_M) // DSWDT
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN ) PORT_NAME("Jackpot Key") PORT_TOGGLE PORT_CODE(KEYCODE_F1) // JKPTSW
	PORT_DIPNAME( 0x08, 0x08, "HOPHI1") // hopper 1 full, HOPPER PROBE in robot test
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5003:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "OPTA2") // coin in a2, HANDLE A in robot test
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5003:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "OPTB2") // coin in b2, HANDLE B in robot test
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5003:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "PTRTAC") // printer taco
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5003:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "PTRHOM") // printer home - must be on
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5003:8")
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("5005")
	PORT_DIPNAME( 0x07, 0x07, "CREDIT SELECT") // US games (except cgold) use this for setting up the denomination
	PORT_DIPSETTING(    0x07, "$0.05" ) PORT_DIPLOCATION("5005:1,2,3")
	PORT_DIPSETTING(    0x03, "$0.10" )
	PORT_DIPSETTING(    0x05, "$0.25" )
	PORT_DIPSETTING(    0x01, "$0.50" )
	PORT_DIPSETTING(    0x06, "$1.00" )
	PORT_DIPSETTING(    0x02, "$5.00" )
	PORT_DIPSETTING(    0x04, "$10.00" )
	PORT_DIPSETTING(    0x00, "$100.00" )
	PORT_DIPNAME( 0x08, 0x00, "5005-4") // Logic door (cgold2)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5005:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "CGDRSW") // Logic door (Security Cage)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5005:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
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
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5300:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "5300-2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5300:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "5300-3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5300:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "5300-4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5300:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "5300-5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5300:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Bill Validator")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5300:6") // bill validator d/c, must be on for US games
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "5300-7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5300:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Mechanical Meters")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5300:8") // must be on
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("500d")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("5 Credits Per Line") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("10 Credits Per Line") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Collect") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Reserve") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("Gamble") PORT_CODE(KEYCODE_U)
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
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MEMORY_RESET ) PORT_CODE(KEYCODE_C) // PB4
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON14 ) PORT_NAME("Hopper Test") PORT_CODE(KEYCODE_V) // PB3
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON15 ) PORT_NAME("Print Data") PORT_CODE(KEYCODE_B) // PB2
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON16 ) PORT_NAME("Clock Init") PORT_CODE(KEYCODE_N) // PB1

	PORT_START("5200")
	PORT_DIPNAME( 0x01, 0x00, "5200-1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5200:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "5200-2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5200:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "5200-3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5200:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "5200-4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5200:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "5200-5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5200:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "5200-6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5200:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "5200-7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5200:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "5200-8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5200:8")
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("5201")
	PORT_DIPNAME( 0x01, 0x00, "5201-1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5201:1")
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "5201-2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5201:2")
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "5201-3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5201:3")
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "5201-4") // fixes link offline error
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5201:4")
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, "5201-5") // causes cgold2, fhunter/a and wtiger to freeze when low
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5201:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "5201-6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5201:6")
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, "5201-7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5201:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "5201-8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5201:8")
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
	PORT_DIPSETTING(    0x00, "3 MHz" )
	PORT_DIPSETTING(    0x10, "1.5 MHz" )

	/********************************* Dip switch for background color *************************************************/

	PORT_START("SW7")
	PORT_DIPNAME( 0x01, 0x00, "SW7 - U22 BG COLOR" )
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
	PORT_DIPNAME( 0x40, 0x00, "DSW1 - Link Jackpot - S2" ) // Cash credit option in 3bagfull/blkrhino/eforest/fvrpitch/grnlizrd/teqsun
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DSW1 - Auto spin" ) // Disables double up, only blkrhino, eforest and grnlizrd support auto spin with double up disabled
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "DSW2 - Maximum credit - S1" ) // Cash credit option in topgear
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

static INPUT_PORTS_START(3bagfull)
	PORT_INCLUDE(aristmk4)

	PORT_MODIFY("500d")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_NAME("Max Bet") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Auto Play") PORT_CODE(KEYCODE_J)

	PORT_MODIFY("500e")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Play 1 Line") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Play 9 Lines") PORT_CODE(KEYCODE_H)
INPUT_PORTS_END

static INPUT_PORTS_START(3bagfullnz)
	PORT_INCLUDE(3bagfull)

	PORT_MODIFY("500d")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("4 Credits Per Line") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("5 Credits Per Line") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-5 UNUSED")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-6 UNUSED")
INPUT_PORTS_END

static INPUT_PORTS_START(eforestu)
	PORT_INCLUDE(aristmk4)

	PORT_MODIFY("via_port_b")
	PORT_DIPNAME( 0x80, 0x00, "CBOPT1" ) // turn off for US games
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) ) PORT_DIPLOCATION("AY:4")

	PORT_MODIFY("500d")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Play 7 Lines") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Play 9 Lines / Black") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Cashout") PORT_CODE(KEYCODE_Q)
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

	PORT_MODIFY("5201")
	PORT_DIPNAME( 0x10, 0x10, "5201-5") // freezes cgold2, fhunter/a and wtiger if off
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) ) PORT_DIPLOCATION("5201:5")
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START(wtiger)
	PORT_INCLUDE(eforestu)

	PORT_MODIFY("500d")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Play 9 Lines") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-5 UNUSED")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-6 UNUSED")

	PORT_MODIFY("500e")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Play 1 Line") PORT_CODE(KEYCODE_W)
INPUT_PORTS_END

static INPUT_PORTS_START(thundhrt)
	PORT_INCLUDE(eforestu)

	PORT_MODIFY("500d")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Play 9 Lines") PORT_CODE(KEYCODE_Y)

	PORT_MODIFY("500e")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Play 1 Line") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("1 Credit Per Line / Red") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("5 Credits Per Line / Black") PORT_CODE(KEYCODE_H)
INPUT_PORTS_END

static INPUT_PORTS_START(arcwins)
	PORT_INCLUDE(eforestu)

	PORT_MODIFY("500d")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Play 4 Lines") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Play 5 Lines / Black") PORT_CODE(KEYCODE_Y)

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

static INPUT_PORTS_START(grnlizrd)
	PORT_INCLUDE(aristmk4)

	PORT_MODIFY("500e")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Play 5 Lines / Black") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Play 4 Lines") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_NAME("Play 3 Lines") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("Play 2 Lines") PORT_CODE(KEYCODE_D)
INPUT_PORTS_END

static INPUT_PORTS_START(kgbirdnz)
	PORT_INCLUDE(grnlizrd)

	PORT_MODIFY("500d")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("4 Credits Per Line") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("5 Credits Per Line") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_CODE(KEYCODE_U)
INPUT_PORTS_END

static INPUT_PORTS_START(fvrpitch)
	PORT_INCLUDE(kgbirdnz)

	PORT_MODIFY("500d")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-7 UNUSED")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-8 UNUSED")

	PORT_MODIFY("500e")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("Play 1 Line") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Play 5 Lines") PORT_CODE(KEYCODE_H)
INPUT_PORTS_END

static INPUT_PORTS_START(teqsun)
	PORT_INCLUDE(kgbirdnz)

	PORT_MODIFY("500d")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("Gamble") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("5 Credits Per Line") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-5 UNUSED")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-6 UNUSED")

	PORT_MODIFY("500e")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON10 ) PORT_NAME("Play 5 Lines / Black") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take Win") PORT_CODE(KEYCODE_H)
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

static INPUT_PORTS_START(gunnrose)
	PORT_INCLUDE(aristmk4)

	PORT_MODIFY("500d")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Red") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Draw") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Bet 2 / Hold 2") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Bet 3 / Hold 3") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-5 UNUSED")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-6 UNUSED")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-7 UNUSED")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-8 UNUSED")

	PORT_MODIFY("500e")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("Half Gamble") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Reserve") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("Full Gamble") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Bet 1 / Hold 1") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Bet 5 / Hold 5") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Bet 4 / Hold 4") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Black") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take Win") PORT_CODE(KEYCODE_A)
INPUT_PORTS_END

static INPUT_PORTS_START(wildone)
	PORT_INCLUDE(aristmk4)

	PORT_MODIFY("500d")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Bet 2 / Hold 2") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Bet 3 / Hold 3") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-5 UNUSED")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-6 UNUSED")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-7 UNUSED")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-8 UNUSED")

	PORT_MODIFY("500e")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Collect") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Black") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Red") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Bet 1 / Hold 1") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("High 5 / Hold 5") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Big 5 / Hold 4") PORT_CODE(KEYCODE_T) // no bet 4 button
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take Win") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Draw") PORT_CODE(KEYCODE_K)
INPUT_PORTS_END

static INPUT_PORTS_START(gldnpkr)
	PORT_INCLUDE(wildone)

	PORT_MODIFY("500d")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_NAME("0-1 UNUSED")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("Gamble") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_CODE(KEYCODE_F)

	PORT_MODIFY("500e")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Change") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / Red") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Black") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take Win") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Draw") PORT_CODE(KEYCODE_R)
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

static GFXDECODE_START(gfx_aristmk4)
	GFXDECODE_ENTRY("tile_gfx",0x0,layout8x8x6, 0, 8 )
GFXDECODE_END

/* read m/c number */

uint8_t aristmk4_state::pa1_r()
{
	return (ioport("SW3")->read() << 4) + ioport("SW4")->read();
}

uint8_t aristmk4_state::pb1_r()
{
	return (ioport("SW5")->read() << 4) + ioport("SW6")->read();
}

uint8_t aristmk4_state::pc1_r()
{
	return 0;
}

/* same as Casino Winner HW */
void aristmk4_state::aristmk4_palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	for (int i = 0; i < palette.entries(); i++)
	{
		const uint8_t data = color_prom[i];
		const int b = 0x4f * BIT(data, 0) + 0xa8 * BIT(data, 1);
		const int g = 0x21 * BIT(data, 2) + 0x47 * BIT(data, 3) + 0x97 * BIT(data, 4);
		const int r = 0x21 * BIT(data, 5) + 0x47 * BIT(data, 6) + 0x97 * BIT(data, 7);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void aristmk4_state::init_aristmk4()
{
	m_shapeRomPtr = (uint8_t *)memregion("tile_gfx")->base();
	memcpy(m_shapeRom,m_shapeRomPtr,sizeof(m_shapeRom)); // back up
	m_nvram = std::make_unique<uint8_t[]>(0x1000);
}

void aristmk4_state::machine_start()
{
	save_pointer(NAME(m_nvram), 0x1000); // m_nvram
	m_credit_spend_meter.resolve();
	m_credit_out_meter.resolve();
	m_hopper_motor_out.resolve();
	m_lamps.resolve();
	m_power_timer = timer_alloc(FUNC(aristmk4_state::power_fail), this);
	m_note_reset_timer = timer_alloc(FUNC(aristmk4_state::note_input_reset), this);
	m_coin_reset_timer = timer_alloc(FUNC(aristmk4_state::coin_input_reset), this);
	m_hopper_reset_timer = timer_alloc(FUNC(aristmk4_state::hopper_reset), this);
}

void aristmk4_state::machine_reset()
{
	/* MK4 has a link on the motherboard to switch between 1.5MHz and 3MHz clock speed */
	switch(ioport("LK13")->read())  // CPU speed control... 3MHz or 1.5MHz
	{
	case 0x00:
		m_maincpu->set_unscaled_clock(MAIN_CLOCK/4);  // 3 MHz
		break;
	case 0x10:
		m_maincpu->set_unscaled_clock(MAIN_CLOCK/8);  // 1.5 MHz
		break;
	}

	m_power_timer->adjust(attotime::from_hz(1), 0, attotime::from_hz(1));
}

TIMER_CALLBACK_MEMBER(aristmk4_state::power_fail)
{
	/*
	IRQ generator pulses the NMI signal to CPU in the event of power down or power failure.
	This event is recorded in NVRAM to facilitate the Robot Test.

	Would be ideal to use this in our add_exit_callback instead of using a timer but it doesn't seem to
	save the power down state in nvram. Is there a cleaner way to do this?

	To enter the robot test

	1. Open the main door
	2. Trigger powerfail / NMI by pressing ',' for at least 1 second, the game will freeze.
	3. Press F3 ( reset ) whilst holding down robot/hopper test button ( Z )

	Note: The use of 1 Hz in the timer is to avoid unintentional triggering the NMI ( ie.. hold down ',' for at least 1 second )
	*/

	if(ioport("powerfail")->read()) // send NMI signal if L pressed
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}
}

void aristmk4_state::aristmk4(machine_config &config)
{
	/* basic machine hardware */
	MC6809E(config, m_maincpu, MAIN_CLOCK/8); // M68B09E @ 1.5 MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &aristmk4_state::slots_mem);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(320, 256);
	screen.set_visarea(0, 304-1, 0, 216-1);    /* from the crtc registers... updated by crtc */
	screen.set_screen_update(FUNC(aristmk4_state::screen_update_aristmk4));
	screen.screen_vblank().set_inputline(m_maincpu, M6809_IRQ_LINE, HOLD_LINE);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_aristmk4);
	PALETTE(config, m_palette, FUNC(aristmk4_state::aristmk4_palette), 512);

	i8255_device &ppi(I8255A(config, "ppi8255_0"));
	ppi.in_pa_callback().set(FUNC(aristmk4_state::pa1_r));
	ppi.in_pb_callback().set(FUNC(aristmk4_state::pb1_r));
	ppi.in_pc_callback().set(FUNC(aristmk4_state::pc1_r));

	via6522_device &via(R65C22(config, "via6522_0", MAIN_CLOCK/8)); // R65C22P2
	via.readpa_handler().set(FUNC(aristmk4_state::via_a_r));
	via.readpb_handler().set(FUNC(aristmk4_state::via_b_r));
	via.writepa_handler().set(FUNC(aristmk4_state::via_a_w));
	via.writepb_handler().set(FUNC(aristmk4_state::via_b_w));
	via.ca2_handler().set(FUNC(aristmk4_state::via_ca2_w));
	via.cb2_handler().set(FUNC(aristmk4_state::via_cb2_w));
	via.irq_handler().set_inputline(m_maincpu, M6809_FIRQ_LINE);
	// CA1 is connected to +5V, CB1 is not connected.

	pia6821_device &pia(PIA6821(config, "pia6821_0"));
	pia.readpa_handler().set(FUNC(aristmk4_state::mkiv_pia_ina));
	pia.writepa_handler().set(FUNC(aristmk4_state::mkiv_pia_outa));
	pia.writepb_handler().set(FUNC(aristmk4_state::mkiv_pia_outb));
	pia.ca2_handler().set(FUNC(aristmk4_state::mkiv_pia_ca2));
	pia.cb2_handler().set(FUNC(aristmk4_state::mkiv_pia_cb2));

	mc6845_device &crtc(C6545_1(config, "crtc", MAIN_CLOCK/8)); // TODO: type is unknown
	crtc.set_screen("screen");
	/* in fact is a mc6845 driving 4 pixels by memory address.
	 that's why the big horizontal parameters */
	crtc.set_show_border_area(false);
	crtc.set_char_width(4);

	MC146818(config, m_rtc, 4.194304_MHz_XTAL);

	SPEAKER(config, "mono").front_center();

	// the Mark IV has X 2 AY8910 sound chips which are tied to the VIA
	AY8910(config, m_ay1, MAIN_CLOCK/8);
	m_ay1->port_a_read_callback().set_ioport("DSW1");
	m_ay1->port_b_write_callback().set(FUNC(aristmk4_state::zn434_w)); // Port write to set Vout of the DA convertors ( 2 x ZN434 )
	m_ay1->add_route(ALL_OUTPUTS, "mono", 0.40);

	AY8910(config, m_ay2, MAIN_CLOCK/8);
	m_ay2->port_a_write_callback().set(FUNC(aristmk4_state::pblp_out));   // Port A write - goes to lamps on the buttons x8
	m_ay2->port_b_write_callback().set(FUNC(aristmk4_state::pbltlp_out));  // Port B write - goes to lamps on the buttons x4 and light tower x4
	m_ay2->add_route(ALL_OUTPUTS, "mono", 0.40);

	SAMPLES(config, m_samples);
	m_samples->set_channels(5);  /* one for each meter - can pulse simultaneously */
	m_samples->set_samples_names(meter_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "mono", 0.05);
}

void aristmk4_state::aristmk4_poker(machine_config &config)
{
	aristmk4(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &aristmk4_state::poker_mem);
}

/* same as Aristocrat Mark-IV HW color offset 7 */
void aristmk4_state::lions_palette(palette_device &palette) const
{
	for (int i = 0; i < palette.entries(); i++)
	{
		const int b = 0x4f * BIT(i, 0) + 0xa8 * BIT(i, 1);
		const int g = 0x4f * BIT(i, 2) + 0xa8 * BIT(i, 3);
		const int r = 0x4f * BIT(i, 4) + 0xa8 * BIT(i, 5);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

void aristmk4_state::_86lions(machine_config &config)
{
	aristmk4(config);
	m_palette->set_init(FUNC(aristmk4_state::lions_palette));
}


// 3 Bags Full (5VXFC790, Victoria)
// 90.018%
// Unhandled Mechanical meter 3 pulse: 08
ROM_START( 3bagfull )
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


// 3 Bags Full (3VXFC5345, New Zealand)
// 88.22%
// Unhandled Mechanical meter 2 pulse: 04
ROM_START( 3bagfullnz )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59",  0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("3vxfc5345.u87", 0x06000, 0x2000, CRC(ba97a469) SHA1(fee56fe7116d1f1aab2b0f2526101d4eb87f0bf1))
	ROM_LOAD("3vxfc5345.u86", 0x08000, 0x8000, CRC(c632c7c7) SHA1(f3090d037f71a0cf099bb55abbc509cf95f0cbba))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("1vlsh224.u20", 0x00000, 0x2000, CRC(b02d4ce8) SHA1(eace41f870bfbc253124efd72f1c7d6021f2e99f)) // original graphics ROMs were bad,
	ROM_LOAD("1vlsh224.u21", 0x02000, 0x2000, CRC(06218c95) SHA1(cbda8e50fd4e9c8a3c51a006921a85d4bfaa6f78)) // using 3bagfull ROMs for now although
	ROM_LOAD("1vlsh224.u22", 0x04000, 0x2000, CRC(191e73f1) SHA1(e6d510b155f9cd3427a70346e5ff28969309be4e)) // some unused tiles differ between sets
	ROM_LOAD("1vlsh224.u45", 0x06000, 0x2000, CRC(054c55cb) SHA1(3df1893095f867220f3d6a52a40bcdffbfc8b529))
	ROM_LOAD("1vlsh224.u46", 0x08000, 0x2000, CRC(f33970b3) SHA1(8814a4d29383545c7c48e5b44f16a53e38b67fc3))
	ROM_LOAD("1vlsh224.u47", 0x0a000, 0x2000, CRC(609ecf9e) SHA1(9d819bb71f62eb4dd1b3d71748e87c7d77e2afe6))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("1cm48.u71", 0x0000, 0x0200, CRC(81daeeb0) SHA1(7dfe198c6def5c4ae4ecac488d65c2911fb3a890))
ROM_END


// 3 Bags Full (4XF5196I02, US)
// 92.047%
// Unhandled Mechanical meter 1 pulse: 01 > Insert note
// Unhandled Mechanical meter 3 pulse: 08 > Cashout
ROM_START( 3bagfullu )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59",  0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("4xf5196i02.u87", 0x06000, 0x2000, CRC(147aca84) SHA1(28309d04ffe727c79f75146f2dd3737a6cb8a53c))
	ROM_LOAD("4xf5196.u86",    0x08000, 0x8000, CRC(c8c33c7a) SHA1(8aab8bfc4a29d08ac3c2a2b2db7b773e2f0fbbd1))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("2vlsh224.u20", 0x00000, 0x2000, CRC(50676d2b) SHA1(e100debce42bcff093faf4dddc082655fcb26eba))
	ROM_LOAD("2vlsh224.u21", 0x02000, 0x2000, CRC(6e487671) SHA1(3959e7a6acdcf24055a4b6a98317bd75fcef421d))
	ROM_LOAD("2vlsh224.u22", 0x04000, 0x2000, CRC(8ad3ebd5) SHA1(e307a363fa806ed2db586aa9b4a6262046606242))
	ROM_LOAD("2vlsh224.u45", 0x06000, 0x2000, CRC(e6d1210f) SHA1(443dd2748decd45b98ebd14532e3ba80fa4b5b10))
	ROM_LOAD("2vlsh224.u46", 0x08000, 0x2000, CRC(444c8a8b) SHA1(401cd998195415c789712421c46427fa652fafa7))
	ROM_LOAD("2vlsh224.u47", 0x0a000, 0x2000, CRC(7dc3b85c) SHA1(05bded5f52fedf619ec755aec8c90feb71aad6ad))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("1cm48.u71", 0x0000, 0x0200, CRC(81daeeb0) SHA1(7dfe198c6def5c4ae4ecac488d65c2911fb3a890))
ROM_END


// Arctic Wins (4XF5227H04, US)
// 92.501%
// Unhandled Mechanical meter 1 pulse: 01 > Insert note
// Unhandled Mechanical meter 3 pulse: 08 > Cashout
ROM_START( arcwins )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("4xf5227h04.u87", 0x06000, 0x2000, CRC(4411b6e8) SHA1(ad960e9ac75d2d6b9465e3002626e8ac97e3d545))
	ROM_LOAD("4xf5227.u86",    0x08000, 0x8000, CRC(4e2b955a) SHA1(66202e1c7fe52f706c809d6aa8aa649b54dca4d2))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("4xf5227.u20", 0x00000, 0x2000, CRC(f0438b40) SHA1(cead72e988e8973d95312d62ffd45cb51c982551)) // unknown EPROM names, should contain VLSH or VL/SH letters on label
	ROM_LOAD("4xf5227.u21", 0x02000, 0x2000, CRC(0e4c817c) SHA1(dc142d4cf5227496d1e6b82368a8fa186b6372c7))
	ROM_LOAD("4xf5227.u22", 0x04000, 0x2000, CRC(fef65b79) SHA1(38562221ff0513ab973ac96a6ff1e70f0d4e6436))
	ROM_LOAD("4xf5227.u45", 0x06000, 0x2000, CRC(bf7bf9e2) SHA1(32cc8428281f57280ba7aeb7b9a30c51b3a5bec8))
	ROM_LOAD("4xf5227.u46", 0x08000, 0x2000, CRC(c4b2ec7c) SHA1(db0bef392e83a1fb9b1d2255b36a3ec12e73ee1c))
	ROM_LOAD("4xf5227.u47", 0x0a000, 0x2000, CRC(6608d05a) SHA1(7a4014d4dbc8ec6b3dcf14df5a5149696c7ce45e))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("1cm29.u71", 0x0000, 0x0200, CRC(ef25f5cc) SHA1(51d12f4b8b8712cbd18ec97ec04e1340cd85fc67))
ROM_END


// Arctic Wins (4XF5227H03, US)
// 90.361%
// Unhandled Mechanical meter 1 pulse: 01 > Insert note
// Unhandled Mechanical meter 1 pulse: 08 > Cashout
ROM_START( arcwinsa )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("4xf5227h03.u87", 0x06000, 0x2000, CRC(eec47dcf) SHA1(9d9d56310fc2c69c56aee961d1881328e3aa32d2))
	ROM_LOAD("4xf5227.u86",    0x08000, 0x8000, CRC(4e2b955a) SHA1(66202e1c7fe52f706c809d6aa8aa649b54dca4d2))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("4xf5227.u20", 0x00000, 0x2000, CRC(f0438b40) SHA1(cead72e988e8973d95312d62ffd45cb51c982551)) // unknown EPROM names, should contain VLSH or VL/SH letters on label
	ROM_LOAD("4xf5227.u21", 0x02000, 0x2000, CRC(0e4c817c) SHA1(dc142d4cf5227496d1e6b82368a8fa186b6372c7))
	ROM_LOAD("4xf5227.u22", 0x04000, 0x2000, CRC(fef65b79) SHA1(38562221ff0513ab973ac96a6ff1e70f0d4e6436))
	ROM_LOAD("4xf5227.u45", 0x06000, 0x2000, CRC(bf7bf9e2) SHA1(32cc8428281f57280ba7aeb7b9a30c51b3a5bec8))
	ROM_LOAD("4xf5227.u46", 0x08000, 0x2000, CRC(c4b2ec7c) SHA1(db0bef392e83a1fb9b1d2255b36a3ec12e73ee1c))
	ROM_LOAD("4xf5227.u47", 0x0a000, 0x2000, CRC(6608d05a) SHA1(7a4014d4dbc8ec6b3dcf14df5a5149696c7ce45e))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("1cm29.u71", 0x0000, 0x0200, CRC(ef25f5cc) SHA1(51d12f4b8b8712cbd18ec97ec04e1340cd85fc67))
ROM_END


// Autumn Moon (1VXFC5488, New Zealand)
// 87.27%
// Unhandled Mechanical meter 2 pulse: 04
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

// Black Rhino (4VXFC830, NSW)
// 87.836%
// Unhandled Mechanical meter 3 pulse: 08
ROM_START( blkrhino )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe)) // 2VA/S004/M VIDEO SOUND 1/1 Tatts, carnaval 8K © 1995 Aristocrat P U59

		/* GAME EPROMs */
	ROM_LOAD("4vxfc830.u87", 0x06000, 0x2000, CRC(5c61e1bc) SHA1(2d3ae1b998ab65588c0867d308c55fd3364c03f9)) // 4VXFC830 MS BLACK RHINO 1/2 87.836 569/3 20cpc 8K © 1995 Aristocrat S U87
	ROM_LOAD("4vxfc830.u86", 0x08000, 0x8000, CRC(2f50f8a6) SHA1(b5fb81bfe8cfdd52247629be75de6ea8302c7a85)) // 4VXFC830 BLACK RHINO 2/2 569/3 32K © 1995 Aristocrat S U86

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("1vlsh236.u20", 0x00000, 0x2000, CRC(0559fe98) SHA1(2ffb7b3ce3b7ba3bd846cae514b66b1c1a3be91f)) // 1VL/SH236 BLACK RHINO 1/6 8K © 1995 Aristocrat P U20
	ROM_LOAD("1vlsh236.u21", 0x02000, 0x2000, CRC(c0b94f7b) SHA1(8fc3bc53c532407b77682e5e9ac6a625081d22a3)) // 1VL/SH236 BLACK RHINO 3/6 8K © 1995 Aristocrat P U21
	ROM_LOAD("1vlsh236.u22", 0x04000, 0x2000, CRC(2f4f0fe5) SHA1(b6c75bd3b6281a2de7bfea8162c39d58b0e8fa32)) // 1VL/SH236 BLACK RHINO 5/6 8K © 1995 Aristocrat P U22
	ROM_LOAD("1vlsh236.u45", 0x06000, 0x2000, CRC(e483b4cd) SHA1(1cb3f77e7d470d7dcd8e50a0f59298d5546e8b58)) // 1VL/SH236 BLACK RHINO 2/6 8K © 1995 Aristocrat P U45
	ROM_LOAD("1vlsh236.u46", 0x08000, 0x2000, CRC(4a0ce91d) SHA1(e2f853c69fb256870c9809cdfbba2b40b47a0004)) // 1VL/SH236 BLACK RHINO 4/6 8K © 1995 Aristocrat P U46
	ROM_LOAD("1vlsh236.u47", 0x0a000, 0x2000, CRC(b265276e) SHA1(8fc0b7a0c12549b4138c51eb91b74f13282909dd)) // 1VL/SH236 BLACK RHINO 6/6 8K © 1995 Aristocrat P U47

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("2cm34.u71", 0x0000, 0x0200, CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f)) // 2CM34 WHITE TIGER 1/1 0.5K © 1995 Aristocrat P U71
ROM_END


// Black Rhino (3VXFC5344, New Zealand)
// 91.96%
// Unhandled Mechanical meter 2 pulse: 04
ROM_START( blkrhinonz )
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


// Caribbean Gold (3VXEC449, US)
// 90.350%
// Unhandled Mechanical meter 3 pulse: 08
ROM_START( cgold ) // MK2.5 board
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("3vas003.u7", 0x06000, 0x2000, CRC(fe7d0ea4) SHA1(3f3f4809534065c33eca2cfff0d1d2a3e3992406)) // 3VA/S003/MEM.or 7 Video Sound CARNAVAL
		/* This chip was physically broken in half on arrival, the sticker was the only thing holding it together; luckily this ROM is already dumped */

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
	ROM_REGION(0x200, "proms", 0 )                                      // Using gtroppo's PROM until 1CM12 is dumped; EPROM reader couldn't identify chip
	ROM_LOAD("1cm12.u40", 0x0000, 0x0200, BAD_DUMP CRC(918cb0ab) SHA1(2ec37abae2ecae2f0f525daf6fafd03789fca20b)) // 1CM12/40 MV2033*CGOLD/TISLE 943D-GLI 8/93
ROM_END


// Caribbean Gold II (4XF5182H04, US)
// 92.858%
// Unhandled Mechanical meter 1 pulse: 01 > Insert note
// Unhandled Mechanical meter 3 pulse: 08 > Cashout
ROM_START( cgold2 )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("4xf5182h04.u87", 0x06000, 0x2000, CRC(ef5cf758) SHA1(5b0b013b079ec7aa11cb54ff44e0f77781c76fe6)) // Torn labels
	ROM_LOAD("4xf5182.u86",    0x08000, 0x8000, CRC(37fd539e) SHA1(4996f112dbc15238c0e96fdb77d7de8b2487ef50))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("5vlsh076.u20", 0x00000, 0x2000, CRC(6e81d96c) SHA1(90877e2e7400e9820b0d3e99380f7452069bef07)) // C. GOLD BLU-L 5VL/SH076 U20/8  2P49/85HA/BAC8
	ROM_LOAD("5vlsh076.u21", 0x02000, 0x2000, CRC(8bf50f7c) SHA1(17705de695d43fa4fa6f1e7afc5c19ecf6f75e35)) // C. GOLD GRN-L 5VL/SH076 U21/10 7F17/P923/4071
	ROM_LOAD("5vlsh076.u22", 0x04000, 0x2000, CRC(ec08e24b) SHA1(9dce6952e92c8d10a1722ec0a394b93be6bc7cea)) // C. GOLD RED-L 5VL/SH076 U22/12 9578/UA95/764F
	ROM_LOAD("5vlsh076.u45", 0x06000, 0x2000, CRC(442599ff) SHA1(073c8354b0e0895092ce5c45c8cdd2d1b46e5fe3)) // C. GOLD BLU-M 5VL/SH076 U45/9  F94H/2AU7/B5E7
	ROM_LOAD("5vlsh076.u46", 0x08000, 0x2000, CRC(9580c2c2) SHA1(8a010fb9e349c066e1af53ed9aa659dbf7dbf17e)) // C. GOLD GRN-M 5VL/SH076 U46/11 PFC7/AH41/3AED
	ROM_LOAD("5vlsh076.u47", 0x0a000, 0x2000, CRC(f3cb845a) SHA1(288f7fe991bb60194a9ef9e8c9b2b18ebbd3b49c)) // C. GOLD RED-M 5VL/SH076 U47/13 85VA/1CA1/2BB3

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )                                      // Using gtroppo's PROM until 1CM12 is dumped
	ROM_LOAD("1cm12.u71", 0x0000, 0x0200, BAD_DUMP CRC(918cb0ab) SHA1(2ec37abae2ecae2f0f525daf6fafd03789fca20b)) // C.G./TREASURE 1CM12 943D U71/40
ROM_END


// Caribbean Gold II (3XF5182H04, US)
// 92.858%
// Unhandled Mechanical meter 1 pulse: 01
ROM_START( cgold2a )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("3xf5182h04.u87", 0x06000, 0x2000, CRC(070a02b2) SHA1(872621275e51c5dca371861a9b9f3038f0dbc8aa))
	ROM_LOAD("3xf5182.u86",    0x08000, 0x8000, CRC(5ac1d424) SHA1(42bb8b5eb163a04054621bbcba5cf8203a661baf))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("5vlsh076.u20", 0x00000, 0x2000, CRC(6e81d96c) SHA1(90877e2e7400e9820b0d3e99380f7452069bef07)) // C. GOLD BLU-L 5VL/SH076 U20/8  2P49/85HA/BAC8
	ROM_LOAD("5vlsh076.u21", 0x02000, 0x2000, CRC(8bf50f7c) SHA1(17705de695d43fa4fa6f1e7afc5c19ecf6f75e35)) // C. GOLD GRN-L 5VL/SH076 U21/10 7F17/P923/4071
	ROM_LOAD("5vlsh076.u22", 0x04000, 0x2000, CRC(ec08e24b) SHA1(9dce6952e92c8d10a1722ec0a394b93be6bc7cea)) // C. GOLD RED-L 5VL/SH076 U22/12 9578/UA95/764F
	ROM_LOAD("5vlsh076.u45", 0x06000, 0x2000, CRC(442599ff) SHA1(073c8354b0e0895092ce5c45c8cdd2d1b46e5fe3)) // C. GOLD BLU-M 5VL/SH076 U45/9  F94H/2AU7/B5E7
	ROM_LOAD("5vlsh076.u46", 0x08000, 0x2000, CRC(9580c2c2) SHA1(8a010fb9e349c066e1af53ed9aa659dbf7dbf17e)) // C. GOLD GRN-M 5VL/SH076 U46/11 PFC7/AH41/3AED
	ROM_LOAD("5vlsh076.u47", 0x0a000, 0x2000, CRC(f3cb845a) SHA1(288f7fe991bb60194a9ef9e8c9b2b18ebbd3b49c)) // C. GOLD RED-M 5VL/SH076 U47/13 85VA/1CA1/2BB3

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )                                      // Using gtroppo's PROM until 1CM12 is dumped
	ROM_LOAD("1cm12.u71", 0x0000, 0x0200, BAD_DUMP CRC(918cb0ab) SHA1(2ec37abae2ecae2f0f525daf6fafd03789fca20b)) // C.G./TREASURE 1CM12 943D U71/40
ROM_END


// Clockwise (New Zealand, unknown ID)
ROM_START( clkwise ) // MK2.5 board
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("3vas003.u7", 0x06000, 0x2000, CRC(fe7d0ea4) SHA1(3f3f4809534065c33eca2cfff0d1d2a3e3992406))

		/* GAME EPROMs */
	ROM_LOAD("clkwise.u9", 0x08000, 0x8000, NO_DUMP) // dead on arrival or blank chip (0xFFFFFFFF throughout ROM)

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
	ROM_LOAD("2cm18.u40", 0x0000, 0x0200, BAD_DUMP CRC(918cb0ab) SHA1(2ec37abae2ecae2f0f525daf6fafd03789fca20b)) // Using gtroppo's PROM until 2CM18 is dumped
ROM_END


// Coral Riches II (1VXFC5472, New Zealand)
// 87.13%
// Unhandled Mechanical meter 2 pulse: 04
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


// Double Agent (3XF5287H04, US)
// 91.977%
// Unhandled Mechanical meter 1 pulse: 01 > Insert note
// Unhandled Mechanical meter 3 pulse: 08 > Cashout
ROM_START( dblagent )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("3xf5287h04.u87", 0x06000, 0x2000, CRC(c1356048) SHA1(bff581f0dfcc189d5404f231b1a127225fb9abfc))
	ROM_LOAD("3xf5287.u86",    0x08000, 0x8000, CRC(57526edc) SHA1(309295e0c278de370961541cdeb6650555868e83))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("4vlsh116.u20", 0x00000, 0x2000, CRC(3c82d320) SHA1(79c4846c95e9d7701d51577aee47c070f407df9d))
	ROM_LOAD("4vlsh116.u21", 0x02000, 0x2000, CRC(fcfa8602) SHA1(f9007ce61f00ea81e26143d8a37d31a3c1c42650))
	ROM_LOAD("4vlsh116.u22", 0x04000, 0x2000, CRC(8d5e0f9e) SHA1(e00abaea53d134addfefa4830269accb93f5fdf8))
	ROM_LOAD("4vlsh116.u45", 0x06000, 0x2000, CRC(f13b14da) SHA1(ad5f412a87c21fe5879cc96393e100efe8c5f4ee))
	ROM_LOAD("4vlsh116.u46", 0x08000, 0x2000, CRC(ccb5b2fd) SHA1(8100037904a83668ef749b28777fdb1730044c44))
	ROM_LOAD("4vlsh116.u47", 0x0a000, 0x2000, CRC(18b0abcc) SHA1(965e69621f6c73bbacd54099dce0222c79d4f9e3))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("1cm29.u71", 0x0000, 0x0200, CRC(ef25f5cc) SHA1(51d12f4b8b8712cbd18ec97ec04e1340cd85fc67)) // Using kgbird's 1CM29 PROM until original PROM is verified
ROM_END


// Enchanted Forest (4VXFC818, NSW)
// 90.483%
// Unhandled Mechanical meter 3 pulse: 08
ROM_START( eforest )
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


// Enchanted Forest (3VXFC5343, New Zealand)
// 88.43%
// Unhandled Mechanical meter 2 pulse: 04
ROM_START( eforestnz )
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


// Enchanted Forest (12XF528902, US)
// 92.778%
// Unhandled Mechanical meter 1 pulse: 01 > Insert note
ROM_START( eforestu )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("12xf528902.u87", 0x06000, 0x2000, CRC(b2f79725) SHA1(66842130b49276bda91e211514af0ab074d2c283))
	ROM_LOAD("12xf5289.u86",   0x08000, 0x8000, CRC(547207f3) SHA1(aedae50abb4cffa0434abfe606a11fbbba037197))

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


// Fantasy Fortune (1VXFC5460, New Zealand)
// 87.90%
// Unhandled Mechanical meter 2 pulse: 04
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


// Fever Pitch (2VXEC534, NSW)
// 90.360%
// Unhandled Mechanical meter 2 pulse: 04 --> Payout pulse.
ROM_START( fvrpitch ) // MK2.5 board
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("3vas003.u7", 0x06000, 0x2000, CRC(fe7d0ea4) SHA1(3f3f4809534065c33eca2cfff0d1d2a3e3992406))

		/* GAME EPROMS */
	ROM_LOAD("2vxec534.u9", 0x08000, 0x8000, CRC(6f8780e8) SHA1(ebf1bfdf2ad727caa2fee34a6ae645ddba42f1cb))

		/* SHAPE EPROMS */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("1vlbh1299.u8",  0x00000, 0x2000, CRC(8d6294d2) SHA1(819ab872a3ea99801350dd7bdf07011cbc7689e0)) // is it VLBH instead of VLSH or a typo?
	ROM_LOAD("1vlbh1299.u10", 0x02000, 0x2000, CRC(939b30af) SHA1(0253c6b1d336ad589322ee9058c1da68ac1e714a))
	ROM_LOAD("1vlbh1299.u12", 0x04000, 0x2000, CRC(81913322) SHA1(4ed8b678e38784a41c1a46809a5ecb14256b4c75))
	ROM_LOAD("1vlbh1299.u9",  0x06000, 0x2000, CRC(e0937d74) SHA1(19f567620e095b10f1d4f2a524331737bfa628b7))
	ROM_LOAD("1vlbh1299.u11", 0x08000, 0x2000, CRC(bfa3bb9e) SHA1(610de284004906af5a5b594256e7d7ec846afff2))
	ROM_LOAD("1vlbh1299.u13", 0x0a000, 0x2000, CRC(6d8fb9a6) SHA1(1d8b667eea57f5a4ce173af55f58b9bf56aaa05e))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("1cm29.u40", 0x0000, 0x0200, CRC(ef25f5cc) SHA1(51d12f4b8b8712cbd18ec97ec04e1340cd85fc67)) // Using kgbird's 1CM29 PROM until original PROM is verified
ROM_END


// Fortune Hunter (2XF5196I01, US)
// 90.018%
// Unhandled Mechanical meter 1 pulse: 01
ROM_START( fhunter )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe)) // VIDEO SOUND 2VA/S004 U59/7 1AAA/FP34/8E00
											// Alternate label:       VIDEO SOUND 2VA/S004/MEM/59/7 FP34/8E00
		/* GAME EPROMs */
	ROM_LOAD("2xf5196i01.u87", 0x06000, 0x2000, CRC(f9e6b760) SHA1(af7f16727e84ba8f07400f7f02302862e02d1af4))
	ROM_LOAD("2xf5196.u86",    0x08000, 0x8000, CRC(6971ccee) SHA1(1292cfa8125cbaec3bcd9d136cb385a3574bfa4a))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("1vlsh293.u20", 0x00000, 0x2000, CRC(96c81134) SHA1(e5e75e8b4897ee7cd9c27b0546fe4006cf384cba)) // F. HUNTER BLU-L 1VL/SH293 U20/8  516C/3780/8F37
	ROM_LOAD("1vlsh293.u21", 0x02000, 0x2000, CRC(ad7bc6a0) SHA1(145e9a094212841e8a684136ea813bd1bea070fb)) // F. HUNTER GRN-L 1VL/SH293 U21/10 PC01/0649/05A6
	ROM_LOAD("1vlsh293.u22", 0x04000, 0x2000, CRC(450d47bb) SHA1(219a0eeca3989da8cec68405466c9a20f2ee9bfa)) // F. HUNTER RED-L 1VL/SH293 U22/12 2H12/6AH8/EA7A
	ROM_LOAD("1vlsh293.u45", 0x06000, 0x2000, CRC(560b2417) SHA1(1ed26ceaff87150d2f0115825f952348e34e0414)) // F. HUNTER BLU-M 1VL/SH293 U45/9  A0C8/0A1F/88C5
	ROM_LOAD("1vlsh293.u46", 0x08000, 0x2000, CRC(7704c13f) SHA1(4cfca6ee9e2e543714e8bf0c6de4d9e9406ce250)) // F. HUNTER GRN-M 1VL/SH293 U46/11 2648/8H31/CEDD
	ROM_LOAD("1vlsh293.u47", 0x0a000, 0x2000, CRC(a9e6da98) SHA1(3b7d8920d3ef4ae17a55d2e1968318eb3c70264d)) // F. HUNTER RED-M 1VL/SH293 U47/13 FFP8/AA7A/F5AD

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("1cm48.u71", 0x0000, 0x0200, CRC(81daeeb0) SHA1(7dfe198c6def5c4ae4ecac488d65c2911fb3a890)) // FORTUNE HUNTER 1CM48 8CFA U71/40
											// Alternate label:    FH/3BF/LGF 1CM48 8CFA U71/40
ROM_END


// Fortune Hunter (2XF5196I02, US)
// 92.047%
// Unhandled Mechanical meter 1 pulse: 01
ROM_START( fhuntera )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe)) // VIDEO SOUND 2VA/S004 U59/7 1AAA/FP34/8E00
											// Alternate label:       VIDEO SOUND 2VA/S004/MEM/59/7 FP34/8E00
		/* GAME EPROMs */
	ROM_LOAD("2xf5196i02.u87", 0x06000, 0x2000, CRC(4b532a14) SHA1(98d1753ad1d0d041f81a535947ed501d0eb1d85c))
	ROM_LOAD("2xf5196.u86",    0x08000, 0x8000, CRC(6971ccee) SHA1(1292cfa8125cbaec3bcd9d136cb385a3574bfa4a))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("1vlsh293.u20", 0x00000, 0x2000, CRC(96c81134) SHA1(e5e75e8b4897ee7cd9c27b0546fe4006cf384cba)) // F. HUNTER BLU-L 1VL/SH293 U20/8 516C/3780/8F37
	ROM_LOAD("1vlsh293.u21", 0x02000, 0x2000, CRC(ad7bc6a0) SHA1(145e9a094212841e8a684136ea813bd1bea070fb)) // F. HUNTER GRN-L 1VL/SH293 U21/10 PC01/0649/05A6
	ROM_LOAD("1vlsh293.u22", 0x04000, 0x2000, CRC(450d47bb) SHA1(219a0eeca3989da8cec68405466c9a20f2ee9bfa)) // F. HUNTER RED-L 1VL/SH293 U22/12 2H12/6AH8/EA7A
	ROM_LOAD("1vlsh293.u45", 0x06000, 0x2000, CRC(560b2417) SHA1(1ed26ceaff87150d2f0115825f952348e34e0414)) // F. HUNTER BLU-M 1VL/SH293 U45/9 A0C8/0A1F/88C5
	ROM_LOAD("1vlsh293.u46", 0x08000, 0x2000, CRC(7704c13f) SHA1(4cfca6ee9e2e543714e8bf0c6de4d9e9406ce250)) // F. HUNTER GRN-M 1VL/SH293 U46/11 2648/8H31/CEDD
	ROM_LOAD("1vlsh293.u47", 0x0a000, 0x2000, CRC(a9e6da98) SHA1(3b7d8920d3ef4ae17a55d2e1968318eb3c70264d)) // F. HUNTER RED-M 1VL/SH293 U47/13 FFP8/AA7A/F5AD

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("1cm48.u71", 0x0000, 0x0200, CRC(81daeeb0) SHA1(7dfe198c6def5c4ae4ecac488d65c2911fb3a890)) // FORTUNE HUNTER 1CM48 8CFA U71/40
											// Alternate label:    FH/3BF/LGF 1CM48 8CFA U71/40
ROM_END


// The Gambler (11XF528902, US)
// 92.778%
// Unhandled Mechanical meter 1 pulse: 01 > Insert note
// Unhandled Mechanical meter 3 pulse: 08 > Cashout
ROM_START( gambler )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("11xf528902.u87", 0x06000, 0x2000, CRC(aaa18f7a) SHA1(23765f49fef21bae7e9af4057fee1d4ac949352c))
	ROM_LOAD("11xf5289.u86",   0x08000, 0x8000, CRC(db3dca8a) SHA1(72e00213be2406fcd7a6d93da2190624de140b1e))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("11xf5289.u20", 0x00000, 0x2000, CRC(03e2029b) SHA1(c211e1af5921e00d37438870fd00bc6990a4b248)) // unknown EPROM names, should contain VLSH or VL/SH letters on label
	ROM_LOAD("11xf5289.u21", 0x02000, 0x2000, CRC(33a269ba) SHA1(0569a8370e8027e735f67a9d58fb1d327ecdd0e9))
	ROM_LOAD("11xf5289.u22", 0x04000, 0x2000, CRC(b0ab6535) SHA1(65d9cd62eb3c821bae4ab41a93aff66534ad4824))
	ROM_LOAD("11xf5289.u45", 0x06000, 0x2000, CRC(2972c767) SHA1(737a8f9a6ef6e5fee8ac9e16066d133a1b0916af))
	ROM_LOAD("11xf5289.u46", 0x08000, 0x2000, CRC(f63a608e) SHA1(51ce463b7d914539136c15ade029e96cda449b9b))
	ROM_LOAD("11xf5289.u47", 0x0a000, 0x2000, CRC(80c7f6fd) SHA1(0c5e40a10488183a9e0ddc368887b6b830d1da82))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("2cm34.u71", 0x0000, 0x0200, CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END


// Golden Canaries (1VXFC5462, New Zealand)
// 87.30%
// Unhandled Mechanical meter 2 pulse: 04
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


// Gone Troppo (1VXEC542, New Zealand)
// 87.138%
// Unhandled Mechanical meter 3 pulse: 08
ROM_START( gtroppo ) // MK2.5 board
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


// Green Lizard (4VXFC811, NSW)
// 90.445%
// Unhandled Mechanical meter 3 pulse: 08
ROM_START( grnlizrd )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe)) // 2VAS004 VIDEO SOUND 1/1 8K © 1996 Aristocrat S U7

		/* GAME EPROMs */
	ROM_LOAD("4vxfc811.u87", 0x06000, 0x2000, CRC(e2fc0fc6) SHA1(b154c820a40702f1b14a6f9f000e31a289feadf8)) // 4VXFC811 MS GREEN LIZARD 1/2 90.445 572 20CPC 8K © 1995 Aristocrat S U87
	ROM_LOAD("4vxfc811.u86", 0x08000, 0x8000, CRC(bacb0cdf) SHA1(54c579c0337e73bc9cef1f0f7eda0eb1bc168c2a)) // 4VXFC811 GREEN LIZARD 2/2 32K © 1995 Aristocrat S U86

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("1vlsh227.u20", 0x00000, 0x2000, CRC(3b76772a) SHA1(05b0f8ff25e4f476096d07bc098ee3271615eb7e)) // 1VLSH227 GREEN LIZARD 1/6 8K © 1995 Aristocrat S U20
	ROM_LOAD("1vlsh227.u21", 0x02000, 0x2000, CRC(5ae618cd) SHA1(bc890d9355f953c7bc86988db0303eef65434083)) // 1VLSH227 GREEN LIZARD 3/6 8K © 1995 Aristocrat S U21
	ROM_LOAD("1vlsh227.u22", 0x04000, 0x2000, CRC(9744c2af) SHA1(627f4224266ed86e63ce1feea36471978d78b544)) // 1VLSH227 GREEN LIZARD 5/6 8K © 1995 Aristocrat S U22
	ROM_LOAD("1vlsh227.u45", 0x06000, 0x2000, CRC(a85c75d7) SHA1(c0ac08a50946fcd08af325f4d8c7d5b8e4508f5c)) // 1VLSH227 GREEN LIZARD 2/6 8K © 1995 Aristocrat S U45
	ROM_LOAD("1vlsh227.u46", 0x08000, 0x2000, CRC(916e09f8) SHA1(f4c49cf051d904db9abfd01a64169a692be9119a)) // 1VLSH227 GREEN LIZARD 4/6 8K © 1995 Aristocrat S U46
	ROM_LOAD("1vlsh227.u47", 0x0a000, 0x2000, CRC(dccf6409) SHA1(68394a04da2b9cfc92813c0a8caff7fdb2fa91b5)) // 1VLSH227 GREEN LIZARD 6/6 8K © 1995 Aristocrat S U47

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("1cm29.u71", 0x0000, 0x0200, CRC(ef25f5cc) SHA1(51d12f4b8b8712cbd18ec97ec04e1340cd85fc67)) // 1CM29 1/1 0.5K © 1995 Aristocrat P U40
ROM_END


// K.G. Bird (3XF5264H04, US)
// 91.488%
// Unhandled Mechanical meter 1 pulse: 01 > Insert note
// Unhandled Mechanical meter 3 pulse: 08 > Cashout
ROM_START( kgbird )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("3xf5264h04.u87", 0x06000, 0x2000, CRC(2ce4166c) SHA1(fd2fb2d24cf5670bc402cd25ad3cbe0117f9b618))
	ROM_LOAD("3xf5264.u86",    0x08000, 0x8000, CRC(bf66c11b) SHA1(56bc361bc8e2782ca90ddf129804759d8a86a076))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("2vlsh159.u20", 0x00000, 0x2000, CRC(a285e78d) SHA1(627019e736333af15e38ca1bb11b290926a19b20))
	ROM_LOAD("2vlsh159.u21", 0x02000, 0x2000, CRC(e5ba4fbd) SHA1(4b10fb8524af9f118c888639b17476ae2bef0100))
	ROM_LOAD("2vlsh159.u22", 0x04000, 0x2000, CRC(5614ee55) SHA1(3eb3872aa8d2b8c2bd798fd46cc715c64bf35714))
	ROM_LOAD("2vlsh159.u45", 0x06000, 0x2000, CRC(cf496cf2) SHA1(cf097835b5f3d5a656ff84063c54d6b1d40703cd))
	ROM_LOAD("2vlsh159.u46", 0x08000, 0x2000, CRC(9e183d34) SHA1(9f45f6373baf309b0e7c7a14ec3fa3a5d3e70f3e))
	ROM_LOAD("2vlsh159.u47", 0x0a000, 0x2000, CRC(f91b66ba) SHA1(4f5d0f0562c6a6029ad6d76507091a159983d6f4))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("1cm29.u71", 0x0000, 0x0200, CRC(ef25f5cc) SHA1(51d12f4b8b8712cbd18ec97ec04e1340cd85fc67))
ROM_END

// K.G. Bird (4VXFC5341, New Zealand, 5c)
// 87.98%
// Unhandled Mechanical meter 2 pulse: 04
ROM_START( kgbirdnz )
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
	ROM_LOAD("1vlsh159.u22", 0x04000, 0x2000, CRC(5614ee55) SHA1(3eb3872aa8d2b8c2bd798fd46cc715c64bf35714)) // Same as kgbird
	ROM_LOAD("1vlsh159.u45", 0x06000, 0x2000, CRC(cf496cf2) SHA1(cf097835b5f3d5a656ff84063c54d6b1d40703cd)) // Same as kgbird
	ROM_LOAD("1vlsh159.u46", 0x08000, 0x2000, CRC(e155c8d4) SHA1(9c50152dd420d545a88eaea98c2dd2ef49cf056a))
	ROM_LOAD("1vlsh159.u47", 0x0a000, 0x2000, CRC(f91b66ba) SHA1(4f5d0f0562c6a6029ad6d76507091a159983d6f4)) // Same as kgbird

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("1cm29.u71", 0x0000, 0x0200, CRC(ef25f5cc) SHA1(51d12f4b8b8712cbd18ec97ec04e1340cd85fc67))
ROM_END


// K.G. Bird (4VXFC5341, New Zealand, 10c)
// 91.97%
// Unhandled Mechanical meter 2 pulse: 04
ROM_START( kgbirdnza )
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


// Let's Go Fishing (5XF5196I02, US)
// 92.047%
// Unhandled Mechanical meter 1 pulse: 01 > Insert note
// Unhandled Mechanical meter 3 pulse: 08 > Cashout
ROM_START( letsgof )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59",  0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("5xf5196i02.u87", 0x06000, 0x2000, CRC(6f13f4a3) SHA1(95f860df94fe26aa709d335a01ae0681bc14f7da))
	ROM_LOAD("5xf5196.u86",    0x08000, 0x8000, CRC(810a2834) SHA1(3ff1f135968b5cc1052ff9b7be398d5b2adbd736))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("4vlsh293.u20", 0x00000, 0x2000, CRC(b45770cd) SHA1(cd97079070a353450d9c3e436c6ece0952cba121))
	ROM_LOAD("4vlsh293.u21", 0x02000, 0x2000, CRC(d79dc05a) SHA1(b2080ba12408e7b80512f049b17261cca537d333))
	ROM_LOAD("4vlsh293.u22", 0x04000, 0x2000, CRC(03ff1077) SHA1(0280b54d5932c9a9fe1ac7018614ba8767736072))
	ROM_LOAD("4vlsh293.u45", 0x06000, 0x2000, CRC(f4bdb0d1) SHA1(91a0eead9f398d789233b478fd2e8659d27297af))
	ROM_LOAD("4vlsh293.u46", 0x08000, 0x2000, CRC(7ece5615) SHA1(5fc5fc0a0898182a81957d5583cd992af06d9eae))
	ROM_LOAD("4vlsh293.u47", 0x0a000, 0x2000, CRC(533874dd) SHA1(520255db3a1f9538e70e90f6c96b4ba6ece488c7))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("1cm48.u71", 0x0000, 0x0200, CRC(81daeeb0) SHA1(7dfe198c6def5c4ae4ecac488d65c2911fb3a890))
ROM_END


// Phantom Pays (4VXFC5431, New Zealand)
// 91.95%
// Unhandled Mechanical meter 2 pulse: 04
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


// Sweethearts II (1VXFC5461, New Zealand)
// 87.13%
// Unhandled Mechanical meter 2 pulse: 04
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


// Tequila Sunrise (1VXFC613, NSW)
// 90.527%
// Unhandled Mechanical meter 3 pulse: 08
ROM_START( teqsun ) // MK2.5 board
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("1vas004.u7", 0x02000, 0x2000, CRC(fd0576ce) SHA1(437ab6c8cc1d83a5ac774c704491634273fde52e)) // 1VA/S004.MEM.or 7 Video Sound CARNAVAL (Tatts.)

		/* GAME EPROMs */
	ROM_LOAD("1vxfc613.u9", 0x06000, 0x2000, CRC(c8baf2d6) SHA1(e1aac9dbf2c29553cac152ae91718474985edf30)) // 1VXF/C613/9/MS 90.52% TEQUILA SUNRISE
	ROM_LOAD("1vxfc613.u8", 0x08000, 0x8000, CRC(f94c8bb0) SHA1(791495241552586b68aded3d8514ba82681c484d)) // 1VXF/C613/8/MS 90.52%-D/537 20CPC- TEQUILA SUNRISE

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("1vlsh151.u8",  0x00000, 0x2000, CRC(90df2210) SHA1(de62def572615a16ccf7783fc7a32571acf50661)) // 1VL/SH151/8 TEQUILA SUNRISE
	ROM_LOAD("1vlsh151.u10", 0x02000, 0x2000, CRC(dc6afd9f) SHA1(5c80018835f4299c419f8f19b6c54702dde8b1e2)) // 1VL/SH151/10 TEQUILA SUNRISE
	ROM_LOAD("1vlsh151.u12", 0x04000, 0x2000, CRC(aeef27b9) SHA1(ae3bb575ddf2a71d2ccb09f914b5327a0df58b25)) // 1VL/SH151/12 TEQUILA SUNRISE
	ROM_LOAD("1vlsh151.u9",  0x06000, 0x2000, CRC(94ecac89) SHA1(48c0024717e1645b391b4b2ed286d9bd84271ce2)) // 1VL/SH151/9 TEQUILA SUNRISE
	ROM_LOAD("1vlsh151.u11", 0x08000, 0x2000, CRC(ebea52fc) SHA1(a99f4b58812f2a1a1c1f7845e918ff572a105b56)) // 1VL/SH151/11 TEQUILA SUNRISE
	ROM_LOAD("1vlsh151.u13", 0x0a000, 0x2000, CRC(8a2ba54c) SHA1(4f2d4267c28c91965e1bdbaf166615ab82b70c3f)) // 1VL/SH151/13 TEQUILA SUNRISE

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("1cm29.u40", 0x0000, 0x0200, CRC(ef25f5cc) SHA1(51d12f4b8b8712cbd18ec97ec04e1340cd85fc67)) // Using kgbird's 1CM29 PROM until original PROM is verified
ROM_END


// Thunder Heart (13XF528902, US)
// 92.268%
// Unhandled Mechanical meter 1 pulse: 01 > Insert note
// Unhandled Mechanical meter 3 pulse: 08 > Cashout
ROM_START( thundhrt )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("13xf528902.u87", 0x06000, 0x2000, CRC(81297478) SHA1(70f1cc54649adec088b5e3bab677e908f6441b1f))
	ROM_LOAD("13xf5289.u86",   0x08000, 0x8000, CRC(2dc5c84f) SHA1(2b8b529470cc6e78cb988b247308532f364ca7b4))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("13xf5289.u20", 0x00000, 0x2000, CRC(7ee21aaf) SHA1(38c3fc4ed42e93b75ad7b4937284324f2f3f9c1e)) // unknown EPROM names, should contain VLSH or VL/SH letters on label
	ROM_LOAD("13xf5289.u21", 0x02000, 0x2000, CRC(3c961484) SHA1(902636c402629054bf7bf7e32bdeecdc4a61620c))
	ROM_LOAD("13xf5289.u22", 0x04000, 0x2000, CRC(bd2e1cfb) SHA1(048edef79ba66cd09adc316401a14504ba87698b))
	ROM_LOAD("13xf5289.u45", 0x06000, 0x2000, CRC(9909ce2f) SHA1(8ad886ec2de0e54bf0f9b2e40ff4e304e811681d))
	ROM_LOAD("13xf5289.u46", 0x08000, 0x2000, CRC(019bf600) SHA1(b92c5dc432084313480e56def1ca1af2f0fa9381))
	ROM_LOAD("13xf5289.u47", 0x0a000, 0x2000, CRC(f861f64d) SHA1(e033ebcc0cfb160fb04752c067febb47f0735041))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("2cm34.u71", 0x0000, 0x0200, CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END


// Top Gear (4VXFC969, New Zealand)
// 87.471%
// Unhandled Mechanical meter 3 pulse: 08
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
	ROM_LOAD("2cm33.u71", 0x0000, 0x0200, BAD_DUMP CRC(ef25f5cc) SHA1(51d12f4b8b8712cbd18ec97ec04e1340cd85fc67)) // Using kgbird's 1CM29 PROM until 2CM33 is dumped
ROM_END


// Trick or Treat (7XF5183H04, US)
// 92.066%
// Unhandled Mechanical meter 1 pulse: 01 > Insert note
// Unhandled Mechanical meter 3 pulse: 08 > Cashout
ROM_START( trktreat )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59",  0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("7xf5183h04.u87", 0x06000, 0x2000, CRC(e3582474) SHA1(db5ac7275fa96bd19be89208948a3fe51ad70711))
	ROM_LOAD("7xf5183.u86",    0x08000, 0x8000, CRC(2175ec42) SHA1(eb7808da9d5492f6d568e8bf0d11fd60dbcfabf8))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("7xf5183.u20", 0x00000, 0x2000, CRC(bc3fcf32) SHA1(1261fd76a8dc55b339d454fc33cc84446a7b9f6b)) // unknown EPROM names, should contain VLSH or VL/SH letters on label
	ROM_LOAD("7xf5183.u21", 0x02000, 0x2000, BAD_DUMP CRC(e8c3be84) SHA1(ce45609c1646639d60c0691db0d2714778b10833))
	ROM_LOAD("7xf5183.u22", 0x04000, 0x2000, CRC(c7c240e2) SHA1(4ba20491626b899b57a8f97a2e6c25c99c619d20))
	ROM_LOAD("7xf5183.u45", 0x06000, 0x2000, CRC(d744dc68) SHA1(e17389ac2cfe955cfc3c89443ef93dc0b8ab971e))
	ROM_LOAD("7xf5183.u46", 0x08000, 0x2000, CRC(8ffe4651) SHA1(e41a7ce44b5b313dd207d046a28f745c1b73f5ff))
	ROM_LOAD("7xf5183.u47", 0x0a000, 0x2000, CRC(22ef3968) SHA1(0bfd11c2b9446670447807b6891bfa1db9204c52))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("trktreat.u71", 0x0000, 0x0200, CRC(6309e6a0) SHA1(7b2bc4ced79d070e241331dc56270fe9542aa4ce))
ROM_END


// White Tiger (4XF5139I08, US)
// 90.082%
// Unhandled Mechanical meter 1 pulse: 01 > Insert note
// Unhandled Mechanical meter 3 pulse: 08 > Cashout
ROM_START( wtiger )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("4xf5139i08.u87", 0x06000, 0x2000, CRC(fa8b3167) SHA1(0c69a8f071a694d18b7e05d55327802da567a487))
	ROM_LOAD("4xf5139.u86",    0x08000, 0x8000, CRC(fd498a87) SHA1(83cd7c6296f2624a97ce11da64768e06f3244a1f))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("3vlsh157.u20", 0x00000, 0x2000, CRC(b8a5d26a) SHA1(328d5df637f95883d8249897493495f1b83eca4a))
	ROM_LOAD("3vlsh157.u21", 0x02000, 0x2000, CRC(9f090dac) SHA1(fd5f6bf45c1160cddc10229b6a84d9a7e3339da3))
	ROM_LOAD("3vlsh157.u22", 0x04000, 0x2000, CRC(19a6e4f2) SHA1(9af51152706006f09ba434d07bd9663676becd75))
	ROM_LOAD("3vlsh157.u45", 0x06000, 0x2000, CRC(65f42ea7) SHA1(3953684cd47d319d77963a6ed203f41c1e91edf9))
	ROM_LOAD("3vlsh157.u46", 0x08000, 0x2000, CRC(c0e85f97) SHA1(0e55bff8d5c9400475a8eab922613098126f1e6d))
	ROM_LOAD("3vlsh157.u47", 0x0a000, 0x2000, CRC(8d922142) SHA1(efe471b6c6f1f1dbaa8b26f62e36ece96e69a26a))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("2cm34.u71", 0x0000, 0x0200, CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END


// White Tiger (3VXFC5342, New Zealand)
// 91.99%
// Unhandled Mechanical meter 2 pulse: 04
ROM_START( wtigernz )
	ROM_REGION(0x10000, "maincpu", 0 )
		/* VIDEO AND SOUND EPROM */
	ROM_LOAD("2vas004.u59", 0x02000, 0x2000, CRC(84226547) SHA1(df9c2c01a7ac4d930c06a8c4863853ddb1a2adbe))

		/* GAME EPROMs */
	ROM_LOAD("3vxfc5342.u87", 0x06000, 0x2000, CRC(9492b242) SHA1(26bb14cba8e8c3cdbcb4b4903da9592b0a1f8cb3))
	ROM_LOAD("3vxfc5342.u86", 0x08000, 0x8000, CRC(f639ef56) SHA1(5d49deee95df29cd4f5c69fea01bb752aaf2ce99))

		/* SHAPE EPROMs */
	ROM_REGION(0xc000, "tile_gfx", 0 )
	ROM_LOAD("1vlsh157.u20", 0x00000, 0x2000, BAD_DUMP CRC(857f2a6e) SHA1(c59656874dd778f876f30cd7371be81053981088))
	ROM_LOAD("1vlsh157.u21", 0x02000, 0x2000, CRC(4bce2fa1) SHA1(8c25cd51ea61a4a9ff1238d1617e38b2cd298c53))
	ROM_LOAD("1vlsh157.u22", 0x04000, 0x2000, CRC(da141f20) SHA1(e0ebeeff2e085a30032d29748f5aa6116428aaa8))
	ROM_LOAD("1vlsh157.u45", 0x06000, 0x2000, CRC(13783f87) SHA1(662f6afdd027c3d139d7dfcd45a4a2a5a2bf2101))
	ROM_LOAD("1vlsh157.u46", 0x08000, 0x2000, CRC(7dfd06ec) SHA1(51fbc3d24e270edb8de432a99ca28695e42e72a6))
	ROM_LOAD("1vlsh157.u47", 0x0a000, 0x2000, CRC(177a45ea) SHA1(6b044f88c79de571a007fb71ff2f99587babe474))

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 )
	ROM_LOAD("2cm34.u71", 0x0000, 0x0200, CRC(75814247) SHA1(9d123dadba3b5a1fd1c7f0100b255c4dd4f7e04f))
ROM_END


/* Video poker games */


// Golden Poker (8VXEC037, New Zealand)
// Unhandled Mechanical meter 3 pulse: 08
ROM_START( gldnpkr ) // MK2.5 board
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


// Guns & Roses (C606191SMP, NSW)
// Unhandled Mechanical meter 2 pulse: 04
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
	ROM_LOAD("gnr.u9t", 0x06000, 0x2000, CRC(73a0c2cd) SHA1(662056d570eaa069483d378b77efcfb42eff6d0d))  // 1VL/SH136 RED AND BLACK
	ROM_LOAD("gnr.u11", 0x08000, 0x2000, CRC(c50adffe) SHA1(a7c4a3cdd4d5d31a1420e47859408caa75ce2636))  // 1VL/SH136 RED AND BLACK
	ROM_LOAD("gnr.u13", 0x0a000, 0x2000, CRC(e0a6bfc5) SHA1(07e4c8191503f0ea2de4f7ce18fe6290d20ef80e))  // 1VL/SH136 RED AND BLACK

		/* COLOR PROM */
	ROM_REGION(0x200, "proms", 0 ) /* are either of these correct?  They are taken from different games */
	//ROM_LOAD("2cm07.u40", 0x0000, 0x0200, CRC(1e3f402a) SHA1(f38da1ad6607df38add10c69febf7f5f8cd21744)) // Using 2CM07 until a correct PROM is confirmed
	ROM_LOAD("1cm48.u40", 0x0000, 0x0200, BAD_DUMP CRC(81daeeb0) SHA1(7dfe198c6def5c4ae4ecac488d65c2911fb3a890))
ROM_END

// Wild One (4VXEC5357, New Zealand)
// 88.00%
// Unhandled Mechanical meter 2 pulse: 04
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


/* Microstar hardware? */


// 86 Lions
// Shows "Hi-Roller" on title screen
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

} // anonymous namespace


GAMEL( 1996, 3bagfull,   0,        aristmk4,       3bagfull,   aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "3 Bags Full (5VXFC790, Victoria)",          0, layout_3bagfull   ) // 5c, $1 = 20 credits
GAMEL( 1996, 3bagfullnz, 3bagfull, aristmk4,       3bagfullnz, aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "3 Bags Full (3VXFC5345, New Zealand)",      0, layout_3bagfullnz ) // 5c, $2 = 40 credits
GAMEL( 1998, 3bagfullu,  3bagfull, aristmk4,       wtiger,     aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "3 Bags Full (4XF5196I02, US)",              0, layout_fhunter    ) // Multi-denomination
GAMEL( 1996, arcwins,    0,        aristmk4,       arcwins,    aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "Arctic Wins (4XF5227H04, US)",              0, layout_arcwins    ) // Multi-denomination
GAMEL( 1996, arcwinsa,   arcwins,  aristmk4,       arcwins,    aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "Arctic Wins (4XF5227H03, US)",              0, layout_arcwins    ) // Multi-denomination
GAMEL( 1999, autmoon,    0,        aristmk4,       arimk4nz,   aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "Autumn Moon (1VXFC5488, New Zealand)",      0, layout_arimk4nz   ) // 5c, $2 = 40 credits
GAMEL( 1995, blkrhino,   0,        aristmk4,       aristmk4,   aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "Black Rhino (4VXFC830, NSW)",               0, layout_aristmk4   ) // 5c, $1 = 20 credits
GAMEL( 1996, blkrhinonz, blkrhino, aristmk4,       arimk4nz,   aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "Black Rhino (3VXFC5344, New Zealand)",      0, layout_arimk4nz   ) // 5c, $2 = 40 credits
GAMEL( 1986, cgold,      0,        aristmk4,       topgear,    aristmk4_state, init_aristmk4, ROT0, "Ainsworth Nominees P.L.", "Caribbean Gold (3VXEC449, US)",             0, layout_topgear    ) // 25c, 25c = 1 credit
GAMEL( 1996, cgold2,     0,        aristmk4,       cgold2,     aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "Caribbean Gold II (4XF5182H04, US)",        0, layout_cgold2     ) // Multi-denomination
GAMEL( 1995, cgold2a,    cgold2,   aristmk4,       cgold2,     aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "Caribbean Gold II (3XF5182H04, US)",        0, layout_cgold2     ) // Multi-denomination
GAMEL( 1986, clkwise,    0,        aristmk4,       topgear,    aristmk4_state, init_aristmk4, ROT0, "Ainsworth Nominees P.L.", "Clockwise (1VXEC534, New Zealand)",         MACHINE_NOT_WORKING, layout_topgear )
GAMEL( 2000, coralr2,    0,        aristmk4,       arimk4nz,   aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "Coral Riches II (1VXFC5472, New Zealand)",  0, layout_arimk4nz   ) // 2c, $2 = 100 credits
GAMEL( 1996, dblagent,   0,        aristmk4,       arcwins,    aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "Double Agent (3XF5287H04, US)",             0, layout_arcwins    ) // Multi-denomination
GAMEL( 1995, eforest,    0,        aristmk4,       aristmk4,   aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "Enchanted Forest (4VXFC818, NSW)",          0, layout_aristmk4   ) // 10c, $1 = 10 credits
GAMEL( 1996, eforestnz,  eforest,  aristmk4,       arimk4nz,   aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "Enchanted Forest (3VXFC5343, New Zealand)", 0, layout_arimk4nz   ) // 5c, $2 = 40 credits
GAMEL( 1996, eforestu,   eforest,  aristmk4,       eforestu,   aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "Enchanted Forest (12XF528902, US)",         0, layout_eforestu   ) // Multi-denomination
GAMEL( 1998, ffortune,   0,        aristmk4,       goldenc,    aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "Fantasy Fortune (1VXFC5460, New Zealand)",  0, layout_goldenc    ) // 5c, $2 = 40 credits
GAMEL( 1996, fhunter,    0,        aristmk4,       wtiger,     aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "Fortune Hunter (2XF5196I01, US)",           0, layout_fhunter    ) // Multi-denomination
GAMEL( 1996, fhuntera,   fhunter,  aristmk4,       wtiger,     aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "Fortune Hunter (2XF5196I02, US)",           0, layout_fhunter    ) // Multi-denomination
GAMEL( 1986, fvrpitch,   0,        aristmk4,       fvrpitch,   aristmk4_state, init_aristmk4, ROT0, "Ainsworth Nominees P.L.", "Fever Pitch (2VXEC534, NSW)",               0, layout_fvrpitch   ) // 5c, $1 = 20 credits
GAMEL( 1996, gambler,    0,        aristmk4,       eforestu,   aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "The Gambler (11XF528902, US)",              0, layout_eforestu   ) // Multi-denomination
GAMEL( 1996, goldenc,    0,        aristmk4,       goldenc,    aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "Golden Canaries (1VXFC5462, New Zealand)",  0, layout_goldenc    ) // 2c, $2 = 100 credits
GAMEL( 1986, gldnpkr,    0,        aristmk4_poker, gldnpkr,    aristmk4_state, init_aristmk4, ROT0, "Ainsworth Nominees P.L.", "Golden Poker (8VXEC037, New Zealand)",      0, layout_gldnpkr    ) // 20c, 20c = 1 credit
GAMEL( 1986, gtroppo,    0,        aristmk4,       topgear,    aristmk4_state, init_aristmk4, ROT0, "Ainsworth Nominees P.L.", "Gone Troppo (1VXEC542, New Zealand)",       0, layout_topgear    ) // 20c, 20c = 1 credit
GAMEL( 1994, grnlizrd,   0,        aristmk4,       grnlizrd,   aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "Green Lizard (4VXFC811, NSW)",              0, layout_grnlizrd   ) // 5c, $1 = 20 credits
GAMEL( 1993, gunnrose,   0,        aristmk4_poker, gunnrose,   aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "Guns and Roses (C606191SMP, NSW)",          MACHINE_WRONG_COLORS, layout_gunnrose ) // 20c, $1 = 5 credits
GAMEL( 1996, kgbird,     0,        aristmk4,       arcwins,    aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "K.G. Bird (3XF5264H04, US)",                0, layout_arcwins    ) // Multi-denomination
GAMEL( 1996, kgbirdnz,   kgbird,   aristmk4,       kgbirdnz,   aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "K.G. Bird (4VXFC5341, New Zealand, 5c)",    0, layout_kgbirdnz   ) // 5c, $2 = 40 credits
GAMEL( 1996, kgbirdnza,  kgbird,   aristmk4,       kgbirdnz,   aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "K.G. Bird (4VXFC5341, New Zealand, 10c)",   0, layout_kgbirdnz   ) // 10c, $2 = 20 credits
GAMEL( 1998, letsgof,    0,        aristmk4,       wtiger,     aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "Let's Go Fishing (5XF5196I02, US)",         0, layout_fhunter    ) // Multi-denomination
GAMEL( 1998, phantomp,   0,        aristmk4,       arimk4nz,   aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "Phantom Pays (4VXFC5431, New Zealand)",     0, layout_arimk4nz   ) // 5c, $2 = 40 credits
GAMEL( 1998, swtht2nz,   0,        aristmk4,       arimk4nz,   aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "Sweethearts II (1VXFC5461, New Zealand)",   0, layout_arimk4nz   ) // 5c, $2 = 40 credits
GAMEL( 1993, teqsun,     0,        aristmk4,       teqsun,     aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "Tequila Sunrise (1VXFC613, NSW)",           0, layout_teqsun     ) // 5c, $1 = 20 credits
GAMEL( 1998, thundhrt,   0,        aristmk4,       thundhrt,   aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "Thunder Heart (13XF528902, US)",            0, layout_eforestu   ) // Multi-denomination
GAMEL( 1996, trktreat,   0,        aristmk4,       cgold2,     aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "Trick or Treat (7XF5183H04, US)",           MACHINE_IMPERFECT_GRAPHICS, layout_cgold2     ) // Multi-denomination
GAMEL( 1996, topgear,    0,        aristmk4,       topgear,    aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "Top Gear (4VXFC969, New Zealand)",          0, layout_topgear    ) // 10c, 10c = 1 credit
GAMEL( 1995, wtiger,     0,        aristmk4,       wtiger,     aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "White Tiger (4XF5139I08, US)",              0, layout_fhunter    ) // Multi-denomination
GAMEL( 1996, wtigernz,   wtiger,   aristmk4,       arimk4nz,   aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "White Tiger (3VXFC5342, New Zealand)",      0, layout_arimk4nz   ) // 5c, $2 = 40 credits
GAMEL( 1997, wildone,    0,        aristmk4_poker, wildone,    aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "Wild One (4VXEC5357, New Zealand)",         0, layout_wildone    ) // 20c, $2 = 10 credits

GAMEL( 1985, 86lions,    0,        _86lions,       aristmk4,   aristmk4_state, init_aristmk4, ROT0, "Aristocrat",              "86 Lions",                                  MACHINE_NOT_WORKING, layout_topgear )
