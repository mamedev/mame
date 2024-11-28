// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"

#include "toaplipt.h"
#include "gp9001.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#include "cpu/m68000/m68000.h"
#include "cpu/nec/v25.h"
#include "cpu/z80/z80.h"
#include "machine/bankdev.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "sound/okim6295.h"
#include "sound/ymopm.h"
#include "sound/ymz280b.h"

// TODO: this is the old toaplan2.cpp header text, it could do with splitting between the new drivers

/*****************************************************************************

        ToaPlan      game hardware from 1991 - 1994
        Raizing/8ing game hardware from 1993 onwards
        -------------------------------------------------
        Driver by: Quench and Yochizo
        Original othldrby.c by Nicola Salmoria

   Raizing games and Truxton 2 are heavily dependent on the Raine source -
   many thanks to Richard Bush and the Raine team. [Yochizo]


Supported games:

    Name        Board No      Maker         Game name
    ----------------------------------------------------------------------------
    tekipaki    TP-020        Toaplan       Teki Paki
    tekipakit   TP-020        Toaplan       Teki Paki (location test)
    ghox        TP-021        Toaplan       Ghox (Spinner with single up/down axis control)
    ghoxj       TP-021        Toaplan       Ghox (8-Way Joystick controls)
    dogyuun     TP-022        Toaplan       Dogyuun
    dogyuuna    TP-022        Toaplan       Dogyuun (older)
    dogyuunt    TP-022        Toaplan       Dogyuun (location test)
    kbash       TP-023        Toaplan       Knuckle Bash
    kbash2      bootleg       Toaplan       Knuckle Bash 2
    truxton2    TP-024        Toaplan       Truxton 2 / Tatsujin Oh
    pipibibs    TP-025        Toaplan       Pipi & Bibis / Whoopee!! (set 1)
    pipibibsa   TP-025        Toaplan       Pipi & Bibis / Whoopee!! (set 2)
    pipibibsp   TP-025        Toaplan       Pipi & Bibis / Whoopee!! (Prototype)
    pipibibsbl  bootleg       Toaplan       Pipi & Bibis / Whoopee!! (based of the prototype)
    whoopee    *TP-025/TP-020 Toaplan       Pipi & Bibis / Whoopee!! (Teki Paki hardware)
    fixeight    TP-026        Toaplan       FixEight
    fixeightbl  bootleg       Toaplan       FixEight
    grindstm    TP-027        Toaplan       Grind Stormer (1992)
    grindstma   TP-027        Toaplan       Grind Stormer (1992) (older)
    vfive       TP-027        Toaplan       V-V (V-Five)  (1993 - Japan only)
    batsugun    TP-030        Toaplan       Batsugun
    batsuguna   TP-030        Toaplan       Batsugun (older)
    batsugunsp  TP-030        Toaplan       Batsugun (Special Version)
    enmadaio    TP-031        Toaplan       Enma Daio
    pwrkick     SW931201      Sunwise       Power Kick
    burgkids    SW931201      Sunwise       Burger Kids
    othldrby    S951060-VGP   Sunwise       Othello Derby
    snowbro2    TP-033        Hanafram      Snow Bros. 2 - With New Elves

    * This version of Whoopee!! is on a board labeled TP-020
      (same board number, and same hardware, as Teki Paki)
      but the ROMs are labeled TP-025.

    sstriker    RA-MA7893-01  Raizing       Sorcer Striker
    sstrikera   RA-MA7893-01  Raizing       Sorcer Striker (Unite Trading license)
    mahoudai    RA-MA7893-01  Raizing       Mahou Daisakusen (Japan)
    kingdmgp    RA-MA9402-03  Raizing/8ing  Kingdom Grandprix
    shippumd    RA-MA9402-03  Raizing/8ing  Shippu Mahou Daisakusen (Japan)
    bgaregga    RA9503        Raizing/8ing  Battle Garegga (World - Sat Feb 3 1996)
    bgareggat   RA9503        Raizing/8ing  Battle Garegga (location test - Wed Jan 17 1996)
    bgareggahk  RA9503        Raizing/8ing  Battle Garegga (Hong Kong (and Austria?) - Sat Feb 3 1996)
    bgareggatw  RA9503        Raizing/8ing  Battle Garegga (Taiwan (and Germany?) - Thu Feb 1 1996)
    bgaregganv  RA9503        Raizing/8ing  Battle Garegga - New Version (Hong Kong (and Austria?) - Sat Mar 2 1996)
    bgareggat2  RA9503        Raizing/8ing  Battle Garegga - Type 2 (World - Sat Mar 2 1996)
    bgareggacn  RA9503        Raizing/8ing  Battle Garegga - Type 2 (China (and Denmark?) - Tue Apr 2 1996)
    batrider    RA9704        Raizing/8ing  Armed Police Batrider (Europe - Fri Feb 13 1998)
    batrideru   RA9704        Raizing/8ing  Armed Police Batrider (USA - Fri Feb 13 1998)
    batriderc   RA9704        Raizing/8ing  Armed Police Batrider (China - Fri Feb 13 1998)
    batriderj   RA9704        Raizing/8ing  Armed Police Batrider - B Version (Japan - Fri Feb 13 1998)
    batriderk   RA9704        Raizing/8ing  Armed Police Batrider (Korea - Fri Feb 13 1998)
    batriderja  RA9704        Raizing/8ing  Armed Police Batrider (Japan - Mon Dec 22 1997)
    batridert   RA9704        Raizing/8ing  Armed Police Batrider (Taiwan - Mon Dec 22 1997)
    bbakraid    ET68-V99      8ing          Battle Bakraid - Unlimited Version (USA - Tue Jun 8th, 1999)
    bbakraidj   ET68-V99      8ing          Battle Bakraid - Unlimited Version (Japan - Tue Jun 8th, 1999)
    bbakraidja  ET68-V99      8ing          Battle Bakraid (Japan - Wed Apr 7th, 1999)

    SET NOTES:

    ghox     - The ghoxj set displays an English title screen when the jumpers are set for Japan/Taito,
               and fails to display the "Winners Don't Use Drugs" logo when set for USA/Taito (either
               Taito America or Taito Japan).

    dogyuun  - In the location test version, if you are hit while you have a bomb, the bomb explodes
               automatically and saves you from dying. In the final released version, the bomb explodes
               but you die anyway.
               The only difference between the dogyuun and dogyuuna sets is some of the region jumper
               settings; see the INPUT_PORTS definitions.

    truxton2 - Although the truxton2 PCB has only standard JAMMA mono audio output, and uses a YM3014B
               mono DAC, the YM2151 music is actually sequenced in stereo. In toaplan2.h, uncomment
               "#define TRUXTON2_STEREO" to hear the game's music the way it was originally composed.
               Difficulty is much lower when the region is set to Europe or USA than when set to any
               Asian region, independent of the "Difficulty" dipswitches. See the code beginning at
               1FE94 (RAM address 1002D6 contains 0 if region is an Asian region, 1 if Europe or USA)

    fixeight - The same program is used for all regions, and the region can be changed just by swapping
               EEPROMs. However, the V25 code also recognizes a secret input that rewrites the EEPROM to
               use any one of the 14 recognized regional licenses, using the state of the player 1 and
               player 2 button inputs held in conjunction with it as a 4-bit binary code:

               Region                      Button input
               ------------------------    ------------------------------------
               Korea, Taito license        No buttons
               Korea                       P1 button 1
               Hong Kong, Taito license    P1 button 2
               Hong Kong                   P1 buttons 1 & 2
               Taiwan, Taito license       P2 button 1
               Taiwan                      P1 button 1 + P2 button 1
               SE Asia, Taito license      P1 button 2 + P2 button 1
               Southeast Asia              P1 buttons 1 & 2 + P2 button 1
               Europe, Taito license       P2 button 2
               Europe                      P1 button 1 + P2 button 2
               USA, Taito license          P1 button 2 + P2 button 2
               USA                         P1 buttons 1 & 2 + P2 button 2
               (Invalid)                   P2 buttons 1 & 2
               (Invalid)                   P1 button 1 + P2 buttons 1 & 2
               Japan                       P1 button 2 + P2 buttons 1 & 2
               Japan, Taito license        P1 buttons 1 & 2 + P2 buttons 1 & 2

    grindstm - Code at 20A26 in vfive forces region to Japan. All sets have some NOPs at reset vector,
               and the NEC V25 CPU test that the other games do is skipped. Furthermore, all sets have
               a broken ROM checksum routine that reads address ranges that don't match the actual
               location or size of the ROM, and that has a hack at the end so it always passes.
               Normally you would expect to see code like this in a bootleg, but the NOPs and other
               oddities are identical among three different sets.

    batsugun - The Special Version has many changes to make the game easier: it adds an autofire button,
               replaces the regular bomb with the more powerful double bomb (which in the original version
               required both players in a two player game to press their bomb buttons at once), gives you
               a shield that can absorb one hit each time your ship "levels up", etc. It also changes the
               colors of the title screen, ship select screen, stages, and enemies.
               batsugun compared to batsuguna has code that looks more like the Special Version, but it
               doesn't have any of the Special Version features. All the differences between batsugun
               and batsuguna look like bug fixes that were carried over into the Special Version.

    sstriker - The mahoudai set reads the region jumpers, but the lookup tables in ROM that map jumper
               settings to copyright text, coinage settings, etc., contain identical values for every
               jumper setting, effectively ignoring the jumpers and forcing the region to Japan.
               On the other hand, sstriker has its title screen and all its text in English even when
               the region is set to Japan. This seems odd but has been verified correct on two boards.
               The only difference between sstriker and sstrikera is the copyright text displayed when
               the region is set to Korea.

    kingdmgp - The kingdmgp and shippumd sets have identical program ROMs but a different graphics ROM
               for the text layer. Setting the region to Japan with the kingdmgp ROM, or to anything other
               than Japan with the shippumd ROM, results in a corrupt title screen and unreadable text.
               In kingdmgp some of the tiles needed for the credits screen in attract mode have been
               stripped out, resulting in boxes where letters should be. It doesn't seem very professional
               but appears to be a genuine release. A lot of boards being sold as 'Kingdom Grand Prix' are
               in fact conversions using Neill Corlett's hack.

    bgaregga - The later versions change the small bullet-shaped enemy bullets to bright yellow balls,
               eliminate the flying metal debris from explosions, and require additional joystick input
               to access the Extended, Harder, Special, and Stage Edit hidden features.
               In addition to these changes, the bgareggat2 set uses only two buttons. Instead of being
               able to change the formation of your options with the third button, each of the selectable
               ships has a different, fixed option formation. However, the third button can still be used
               to select an alternate ship color and to enter the secret character and Stage Edit codes.

    batrider - Batrider was marketed as a two button game, and the regular ships all use only the first
               two buttons, but in the original version you need the third button in order to control the
               options of the hidden Battle Garegga ships.
               This problem was fixed in the B Version, which lets you change the Battle Garegga ships'
               option formation using Street Fighter style joystick commands (as well as by using the third
               button, if the cabinet has one)

    bbakraid - Because players managed to counter stop the original Battle Bakraid not long after release,
               the Unlimited Version, which can display more score digits, was released as a ROM upgrade.
               The upgrade also fixes the bug in the original version that prevented the unlocking of
               Team Edit mode from being saved in the EEPROM.


 ****************************************************************************
 * Battle Garegga and Armed Police Batrider have secret characters          *
 * and game features.                                                       *
 * Try to input the following commands to use them.                         *
 * ======================================================================== *
 * Battle Garegga                                                           *
 *       The button you use to select your ship not only determines its     *
 *       color, but affects its characteristics.                            *
 *           A: Default characteristics.                                    *
 *           B: Slightly higher speed than A type.                          *
 *           C: Slightly smaller hitbox than A type.                        *
 *       A+B+C: Same speed as B type and same hitbox as C type.             *
 *                                                                          *
 *       After inserting a coin (pushing a credit button), input            *
 *       UP  UP  DOWN  DOWN  LEFT  RIGHT  LEFT  RIGHT  A  B  C  START       *
 *       then you can use Mahou Daisakusen characters.                      *
 *                                                                          *
 * Note: In versions of the game dated Mar 2 1996 or later, you must        *
 *       hold the joystick RIGHT in addition to the specified button(s)     *
 *       when entering any of the following commands. Even if Stage Edit    *
 *       is enabled via dipswitch, you need to hold RIGHT to use it.        *
 *                                                                          *
 * EXTENDED:   After inserting a coin, hold A and press START.              *
 *             You play through all stages twice before the game ends.      *
 * HARDER:     After inserting a coin, hold B and press START.              *
 *             Difficulty is increased.                                     *
 * SPECIAL:    After inserting a coin, hold A and B and press START.        *
 *             Combination of EXTENDED and HARDER modes.                    *
 * STAGE EDIT: After inserting a coin, hold C and press START.              *
 *             You can choose what order to play Stage 2, 3 and 4 in,       *
 *             or even skip them.                                           *
 *                                                                          *
 * EXTENDED, HARDER, and SPECIAL modes each have their own high score list. *
 * ------------------------------------------------------------------------ *
 * Armed Police Batrider                                                    *
 *       The button you use to select your ship not only determines its     *
 *       color, but affects its characteristics.                            *
 *           A: High main shot power, low option shot power.                *
 *              Average speed. Default autofire rate is 15 Hz.              *
 *           B: Low main shot power, high option shot power. Slightly       *
 *              slower than A type. Default autofire rate is 12 Hz.         *
 *           C: High main shot and option shot power, but lowest speed.     *
 *              Default autofire rate is 20 Hz.                             *
 *       START: Low main shot and option shot power, but highest speed.     *
 *              Default autofire rate is 10 Hz.                             *
 *                                                                          *
 * Note: The following features can also be enabled via dipswitches.        *
 *                                                                          *
 * PLAYER SELECT: After inserting a coin, input                             *
 *       UP  UP  DOWN  DOWN  LEFT  RIGHT  LEFT  RIGHT  B  A                 *
 *       You can select a single character instead of a team.               *
 * GUEST PLAYERS: After inserting a coin, input                             *
 *       UP  UP  DOWN  DOWN  LEFT  RIGHT  LEFT  RIGHT  A  B                 *
 *       You can use Mahou Daisakusen and Battle Garegga characters.        *
 * SPECIAL COURSE: After inserting a coin, input                            *
 *       UP  DOWN  UP  DOWN  LEFT  RIGHT  LEFT  RIGHT  A  B                 *
 *       You can select the Special course, which consists of bosses only.  *
 * STAGE EDIT: When you select your course, press A and B simultaneously.   *
 *       You can choose what order to play Stage 2, 3 and 4 in,             *
 *       or even skip them.                                                 *
 ****************************************************************************


 ############################################################################
 # In Battle Bakraid, the button you use to select your ship not only       #
 # determines its color, but affects its characteristics.                   #
 #     A: Increased main shot power. Default autofire rate is 20 Hz.        #
 #     B: Increased bomb blast duration. Default autofire rate is 12 Hz.    #
 #     C: Increased side shot power. Default autofire rate is 15 Hz.        #
 # START: Increased speed. Default autofire rate is 10 Hz.                  #
 #                                                                          #
 # STAGE EDIT: When you select your course, press A and B simultaneously.   #
 #        You can choose what order to play Stage 2, 3, 4 and 5 in,         #
 #        or even skip them. Stage Edit can also be enabled via dipswitch.  #
 # ======================================================================== #
 # Battle Bakraid has unlocking codes to gain access to extra players       #
 # and game features. Once each feature is unlocked, it is saved in EEPROM  #
 # and remains unlocked until you erase the EEPROM from the service mode.   #
 # However, in the original (non-Unlimited) version, the unlocking of       #
 # Team Edit is not saved in EEPROM, apparently due to a bug.               #
 # Special thanks go to the 'R8ZING Shooter Tribute' page for finding       #
 # and publishing this info.                                                #
 # ======================================================================== #
 #      PLAYER SELECT: PHASE 2                                              #
 # Result:  3 more fighter planes available.                                #
 # Code:    UP  UP  DOWN  DOWN  LEFT  RIGHT  LEFT  RIGHT  A  B  Start       #
 # Conditions:                                                              #
 #      1. Start from the title screen                                      #
 #      2. Insert Coin                                                      #
 #      3. Watch the 20 sec. counter and enter each part of the code right  #
 #         between the counting.                                            #
 # Example: 12,up,11,up,10,down,9,down,8,left,7,right,6.left,5,r..          #
 # After entering the [B] button a chime should sound. Phase 2 unlocked!    #
 # ------------------------------------------------------------------------ #
 #      PLAYER SELECT: PHASE 3                                              #
 # Result:  2 more fighter planes available.                                #
 # Code:    UP  UP  DOWN  DOWN  LEFT  RIGHT  LEFT  RIGHT  B  A  Start       #
 # Conditions:                                                              #
 #      1. Unlock Player Select Phase 2 first                               #
 #      2. Insert Coin                                                      #
 #      3. Watch the 20 sec. counter and enter each part of the code right  #
 #         between the counting.                                            #
 # Important: The entering of this code has to be finished before the       #
 # counter passes 10 ! To do so, you will have to start after coin          #
 # insertion, right before it starts to count:                              #
 # Example: up,19,up,18,down,17,down,16,left,15,right,14.left,..            #
 # After entering the [A] button a chime should sound. Phase 3 unlocked!    #
 # ------------------------------------------------------------------------ #
 #      TEAM EDIT: ENABLE                                                   #
 # Result:  Unlocks the 'team edit' feature to select a team of different   #
 #          ships like in Batrider.                                         #
 # Code:    UP  DOWN  UP  DOWN  LEFT  RIGHT  LEFT  RIGHT  A  B  Start       #
 # Conditions:                                                              #
 #      1. Unlock Player Select Phase 2 and Phase 3 first                   #
 #      2. Insert Coin                                                      #
 #      3. Watch the 20 sec. counter and enter each part of the code right  #
 #         between the counting.                                            #
 # Important: This code hast to be entered so that the counter is at 0 when #
 # you press the final button [B]. To do so, start after second 9:          #
 # Example: 9,up,8,down,7,up,6,down,5,left,4,right,3,left,2,right,1,A,0,B   #
 # After entering the [B] button a chime should sound. Team edit unlocked!  #
 #                                                                          #
 # Note: In the Japan version, to use Team Edit after unlocking it,         #
 #       you must hold UP or DOWN  while selecting your course.             #
 #       In the USA version, if Team Edit is unlocked, the game asks you    #
 #       if you want to use it after you select your course.                #
 # ------------------------------------------------------------------------ #
 #      SPECIAL COURSE: ENABLE                                              #
 # Result:  Unlocks the Special course, a game mode where you fight the     #
 #          bosses only.                                                    #
 # Code:    UP  DOWN  UP  DOWN  LEFT  RIGHT  LEFT  RIGHT  B  A  Start       #
 # Conditions:                                                              #
 #      1. Start from the title screen                                      #
 #      2. Hold [C] button                                                  #
 #      3. Insert Coin                                                      #
 #      4. Watch the 20 sec. counter and enter each part of the code right  #
 #         between the counting.                                            #
 #      5. Release [C] button                                               #
 # After entering the [A] button a chime should sound. Special course       #
 # unlocked!                                                                #
 ############################################################################


Game status:

To Do / Unknowns:
    - Whoopee/Teki Paki sometimes tests bit 5 of the region jumper port
        just after testing for vblank. Why?
    - Priority problem on 2nd player side of selection screen in Fixeight bootleg.
    - Fixeight bootleg text in sound check mode does not display properly
        with the CPU set to 10MHz (ok at 16MHz). Possible error in video_count_r routine.
    - Need to sort out the video status register.
    - Find out how exactly how sound CPU communication really works in bgaregga/batrider/bbakraid
        current emulation seems to work (plays all sounds), but there are still some unknown reads/writes
    - Music timing is bit different on bbakraid.
        reference : https://www.youtube.com/watch?v=zjrWs0iHQ5A

*****************************************************************************/


class raizing_base_state : public driver_device
{
public:
	raizing_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_tx_videoram(*this, "tx_videoram")
		, m_tx_lineselect(*this, "tx_lineselect")
		, m_tx_linescroll(*this, "tx_linescroll")
		, m_tx_gfxram(*this, "tx_gfxram")
		, m_audiobank(*this, "audiobank")
		, m_raizing_okibank{
			{ *this, "raizing_okibank0_%u", 0U },
			{ *this, "raizing_okibank1_%u", 0U } }
		, m_shared_ram(*this, "shared_ram")
		, m_mainram(*this, "mainram")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_vdp(*this, "gp9001")
		, m_oki(*this, "oki%u", 1U)
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_soundlatch(*this, "soundlatch%u", 1U)
		, m_z80_rom(*this, "audiocpu")
		, m_oki_rom(*this, "oki%u", 1U)
	{ }

protected:
	u32 screen_update_truxton2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	virtual void device_post_load() override;
	virtual void machine_start() override ATTR_COLD;

	void bgaregga_common_video_start();

	// used by everything
	void create_tx_tilemap(int dx = 0, int dx_flipped = 0);

	void shippumd_coin_w(u8 data);
	void raizing_z80_bankswitch_w(u8 data);
	void raizing_oki_bankswitch_w(offs_t offset, u8 data);
	u8 bgaregga_E01D_r();


	// used by bgaregga and batrider
	void install_raizing_okibank(int chip);


	u32 screen_update_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(bbakraid_snd_interrupt);
	DECLARE_MACHINE_RESET(bgaregga);
	template<unsigned Chip> void raizing_oki(address_map &map) ATTR_COLD;

	u8 shared_ram_r(offs_t offset) { return m_shared_ram[offset]; }
	void shared_ram_w(offs_t offset, u8 data) { m_shared_ram[offset] = data; }

	u32 screen_update_toaplan2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	void tx_videoram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tx_linescroll_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	TILE_GET_INFO_MEMBER(get_text_tile_info);

	u8 m_sndirq_line = 0;        /* IRQ4 for batrider, IRQ2 for bbakraid */
	u8 m_z80_busreq = 0;


	tilemap_t *m_tx_tilemap = nullptr;    /* Tilemap for extra-text-layer */
	required_shared_ptr<u16> m_tx_videoram;
	optional_shared_ptr<u16> m_tx_lineselect;
	optional_shared_ptr<u16> m_tx_linescroll;
	optional_shared_ptr<u16> m_tx_gfxram;
	optional_memory_bank m_audiobank;
	optional_memory_bank_array<8> m_raizing_okibank[2];
	void coin_w(u8 data);
	void reset(int state);

	optional_shared_ptr<u8> m_shared_ram; // 8 bit RAM shared between 68K and sound CPU
	optional_shared_ptr<u16> m_mainram;

	required_device<m68000_base_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<gp9001vdp_device> m_vdp;
	optional_device_array<okim6295_device, 2> m_oki;
	optional_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device_array<generic_latch_8_device, 4> m_soundlatch; // tekipaki, batrider, bgaregga, batsugun
	optional_region_ptr<u8> m_z80_rom;
	optional_region_ptr_array<u8, 2> m_oki_rom;
	bitmap_ind8 m_custom_priority_bitmap;
};

class batrider_state : public raizing_base_state
{
public:
	batrider_state(const machine_config &mconfig, device_type type, const char *tag)
		: raizing_base_state(mconfig, type, tag)
		, m_dma_space(*this, "dma_space")
	{ }

	void batrider(machine_config &config);

	void init_batrider();

protected:
	virtual void video_start() override ATTR_COLD;

	void batrider_68k_mem(address_map &map) ATTR_COLD;
	void batrider_dma_mem(address_map &map) ATTR_COLD;
	void batrider_sound_z80_mem(address_map &map) ATTR_COLD;
	void batrider_sound_z80_port(address_map &map) ATTR_COLD;

	u16 batrider_z80_busack_r();
	void batrider_z80_busreq_w(u8 data);
	u16 batrider_z80rom_r(offs_t offset);
	void batrider_soundlatch_w(u8 data);
	void batrider_soundlatch2_w(u8 data);
	void batrider_unknown_sound_w(u16 data);
	void batrider_clear_sndirq_w(u16 data);
	void batrider_sndirq_w(u8 data);
	void batrider_clear_nmi_w(u8 data);
	void batrider_tx_gfxram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void batrider_textdata_dma_w(u16 data);
	void batrider_pal_text_dma_w(u16 data);
	void batrider_objectbank_w(offs_t offset, u8 data);
	void batrider_bank_cb(u8 layer, u32 &code);

	u16 m_gfxrom_bank[8]{};       /* Batrider object bank */

	// used by batrider, bbakraid, nprobowl
	optional_device<address_map_bank_device> m_dma_space;
};


class bbakraid_state : public batrider_state
{
public:
	bbakraid_state(const machine_config &mconfig, device_type type, const char *tag)
		: batrider_state(mconfig, type, tag)
		, m_eepromout(*this, "EEPROMOUT")
		, m_eeprom(*this, "eeprom")
	{ }

	void bbakraid(machine_config &config);

	void init_bbakraid();

protected:
private:
	void bbakraid_68k_mem(address_map &map) ATTR_COLD;
	void bbakraid_sound_z80_mem(address_map &map) ATTR_COLD;
	void bbakraid_sound_z80_port(address_map &map) ATTR_COLD;

	u16 bbakraid_eeprom_r();
	void bbakraid_eeprom_w(u8 data);

	optional_ioport m_eepromout;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
};


class nprobowl_state : public batrider_state
{
public:
	nprobowl_state(const machine_config &mconfig, device_type type, const char *tag)
		: batrider_state(mconfig, type, tag)
	{ }

	void nprobowl(machine_config &config);

protected:
private:
	void nprobowl_68k_mem(address_map &map) ATTR_COLD;
};

class bgaregga_state : public raizing_base_state
{
public:
	bgaregga_state(const machine_config &mconfig, device_type type, const char *tag)
		: raizing_base_state(mconfig, type, tag)
	{ }

	void bgaregga(machine_config &config);

	void init_bgaregga();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	void bgaregga_68k_mem(address_map &map) ATTR_COLD;
	void bgaregga_sound_z80_mem(address_map &map) ATTR_COLD;

};

class bgaregga_bootleg_state : public bgaregga_state
{
public:
	bgaregga_bootleg_state(const machine_config &mconfig, device_type type, const char *tag)
		: bgaregga_state(mconfig, type, tag)
	{ }

	void bgareggabl(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

};


class sstriker_state : public raizing_base_state
{
public:
	sstriker_state(const machine_config &mconfig, device_type type, const char *tag)
		: raizing_base_state(mconfig, type, tag)
	{ }

	void mahoudai(machine_config &config);
	void shippumd(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	void mahoudai_68k_mem(address_map &map) ATTR_COLD;
	void shippumd_68k_mem(address_map &map) ATTR_COLD;
	void raizing_sound_z80_mem(address_map &map) ATTR_COLD;
};


void raizing_base_state::reset(int state)
{
	if (m_audiocpu != nullptr)
		m_audiocpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
}

void raizing_base_state::coin_w(u8 data) // MOVE TO DEVICE!
{
	/* +----------------+------ Bits 7-5 not used ------+--------------+ */
	/* | Coin Lockout 2 | Coin Lockout 1 | Coin Count 2 | Coin Count 1 | */
	/* |     Bit 3      |     Bit 2      |     Bit 1    |     Bit 0    | */

	if (data & 0x0f)
	{
		machine().bookkeeping().coin_lockout_w(0, BIT(~data, 2));
		machine().bookkeeping().coin_lockout_w(1, BIT(~data, 3));
		machine().bookkeeping().coin_counter_w(0, BIT( data, 0));
		machine().bookkeeping().coin_counter_w(1, BIT( data, 1));
	}
	else
	{
		machine().bookkeeping().coin_lockout_global_w(1);    // Lock all coin slots
	}
	if (data & 0xf0)
	{
		logerror("Writing unknown upper bits (%02x) to coin control\n",data);
	}
}



TILE_GET_INFO_MEMBER(raizing_base_state::get_text_tile_info)
{
	const u16 attrib = m_tx_videoram[tile_index];
	const u32 tile_number = attrib & 0x3ff;
	const u32 color = attrib >> 10;
	tileinfo.set(0,
			tile_number,
			color,
			0);
}

void raizing_base_state::tx_videoram_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_tx_videoram[offset]);
	if (offset < 64*32)
		m_tx_tilemap->mark_tile_dirty(offset);
}

void raizing_base_state::tx_linescroll_w(offs_t offset, u16 data, u16 mem_mask)
{
	/*** Line-Scroll RAM for Text Layer ***/
	COMBINE_DATA(&m_tx_linescroll[offset]);

	m_tx_tilemap->set_scrollx(offset, m_tx_linescroll[offset]);
}



void raizing_base_state::device_post_load()
{
	if (m_tx_gfxram != nullptr)
		m_gfxdecode->gfx(0)->mark_all_dirty();
}


u32 raizing_base_state::screen_update_toaplan2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	m_custom_priority_bitmap.fill(0, cliprect);
	m_vdp->render_vdp(bitmap, cliprect);

	return 0;
}

void raizing_base_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		m_vdp->screen_eof();
	}
}

u32 raizing_base_state::screen_update_truxton2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen_update_toaplan2(screen, bitmap, cliprect);

	rectangle clip = cliprect;

	/* it seems likely that flipx can be set per line! */
	/* however, none of the games does it, and emulating it in the */
	/* MAME tilemap system without being ultra slow would be tricky */
	m_tx_tilemap->set_flip(m_tx_lineselect[0] & 0x8000 ? 0 : TILEMAP_FLIPX);

	/* line select is used for 'for use in' and '8ing' screen on bbakraid, 'Raizing' logo on batrider */
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		clip.min_y = clip.max_y = y;
		m_tx_tilemap->set_scrolly(0, m_tx_lineselect[y] - y);
		m_tx_tilemap->draw(screen, bitmap, clip, 0);
	}
	return 0;
}


void batrider_state::batrider_tx_gfxram_w(offs_t offset, u16 data, u16 mem_mask)
{
	/*** Dynamic GFX decoding for Batrider / Battle Bakraid ***/

	const u16 oldword = m_tx_gfxram[offset];

	if (oldword != data)
	{
		COMBINE_DATA(&m_tx_gfxram[offset]);
		m_gfxdecode->gfx(0)->mark_dirty(offset/16);
	}
}

void batrider_state::batrider_textdata_dma_w(u16 data)
{
	/*** Dynamic Text GFX decoding for Batrider ***/
	/*** Only done once during start-up ***/
	m_dma_space->set_bank(1);
	for (int i = 0; i < (0x8000 >> 1); i++)
	{
		m_dma_space->write16(i, m_mainram[i]);
	}
}

void batrider_state::batrider_pal_text_dma_w(u16 data)
{
	// FIXME: In batrider and bbakraid, the text layer and palette RAM
	// are probably DMA'd from main RAM by writing here at every vblank,
	// rather than being directly accessible to the 68K like the other games
	m_dma_space->set_bank(0);
	for (int i = 0; i < (0x3400 >> 1); i++)
	{
		m_dma_space->write16(i, m_mainram[i]);
	}
}

void batrider_state::batrider_objectbank_w(offs_t offset, u8 data)
{
	data &= 0xf;
	if (m_gfxrom_bank[offset] != data)
	{
		m_gfxrom_bank[offset] = data;
		m_vdp->set_dirty();
	}
}

void batrider_state::batrider_bank_cb(u8 layer, u32 &code)
{
	code = (m_gfxrom_bank[code >> 15] << 15) | (code & 0x7fff);
}

void raizing_base_state::create_tx_tilemap(int dx, int dx_flipped)
{
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(raizing_base_state::get_text_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);

	m_tx_tilemap->set_scroll_rows(8*32); /* line scrolling */
	m_tx_tilemap->set_scroll_cols(1);
	m_tx_tilemap->set_scrolldx(dx, dx_flipped);
	m_tx_tilemap->set_transparent_pen(0);
}

void raizing_base_state::bgaregga_common_video_start()
{
	m_screen->register_screen_bitmap(m_custom_priority_bitmap);
	m_vdp->custom_priority_bitmap = &m_custom_priority_bitmap;

	/* Create the Text tilemap for this game */
	create_tx_tilemap(0x1d4, 0x16b);
}

void bgaregga_state::video_start()
{
	bgaregga_common_video_start();
}

void sstriker_state::video_start()
{
	bgaregga_common_video_start();
}

void bgaregga_bootleg_state::video_start()
{
	m_screen->register_screen_bitmap(m_custom_priority_bitmap);
	m_vdp->custom_priority_bitmap = &m_custom_priority_bitmap;

	/* Create the Text tilemap for this game */
	create_tx_tilemap(4, 4);
}

void batrider_state::video_start()
{
	m_screen->register_screen_bitmap(m_custom_priority_bitmap);
	m_vdp->custom_priority_bitmap = &m_custom_priority_bitmap;

	m_vdp->disable_sprite_buffer(); // disable buffering on this game

	/* Create the Text tilemap for this game */
	m_gfxdecode->gfx(0)->set_source(reinterpret_cast<u8 *>(m_tx_gfxram.target()));

	create_tx_tilemap(0x1d4, 0x16b);

	/* Has special banking */
	save_item(NAME(m_gfxrom_bank));
}




u32 raizing_base_state::screen_update_bootleg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen_update_toaplan2(screen, bitmap, cliprect);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0);
	return 0;
}


/*****************************************************************************
    Input Port definitions
    The following commands are available when the Invulnerability dipswitch
    is set (or, in some games, also when the JAMMA Test switch is pressed):

    P2 start                 : pause
    P1 start                 : resume
    Hold P1 start & P2 start : slow motion

    In bgaregga, batrider and bbakraid, the commands are different:

    Tap P1 start             : pause/resume
    Hold P1 start            : slow motion

    Additional per-game test features are as follows:

    truxton2 - While playing in invulnerable mode, press button 3 to suicide.

    fixeight - While playing in invulnerable mode, press button 3 to suicide
               (player 1 and player 2 only)

    batsugun - While playing in invulnerable mode, press the following buttons
               to stage skip:

               P2 button 3 & P2 button 1 : Skip to end of stage 1
               P2 button 3 & P2 button 2 : Skip to end of stage 2
               P2 button 3               : Skip to end of stage 3

   sstriker - While playing in invulnerable mode as player 2, press
   /kingdmgp  P2 button 3 to skip to the end of the current stage.

   bgaregga - Press and hold P1 button 1, P1 button 2 and P1 button 3 while
              powering on in service mode to enter the special service mode.
              "OPTIONS" and "PLAY DATAS" are added to the menu, and the
              dipswitch display will show the region jumpers (normally hidden).
              Choose "GAME MODE" from the special service mode to enter the
              special game mode. In the special game mode, you can use pause
              and slow motion even when not playing in invulnerable mode.

   batrider - While playing in invulnerable mode, press P1 Start and P2 Start
              to skip directly to the ending scene.

   batrider - Press and hold P1 button 1, P1 button 2 and P1 button 3 while
   /bbakraid  powering on in service mode to enter the special service mode.
              You can change the game's region by pressing left/right.
              Choose "GAME MODE" from the special service mode to enter the
              special game mode. In the special game mode, you can use pause
              and slow motion even when not playing in invulnerable mode.
              While the game is paused in special mode, press button 3 to
              display debugging information.

*****************************************************************************/

static INPUT_PORTS_START( 2b )
	PORT_START("IN1")
	TOAPLAN_JOY_UDLR_2_BUTTONS( 1 )

	PORT_START("IN2")
	TOAPLAN_JOY_UDLR_2_BUTTONS( 2 )

	PORT_START("SYS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_TILT )
	TOAPLAN_TEST_SWITCH( 0x04, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSWA")
	TOAPLAN_MACHINE_NO_COCKTAIL_LOC(SW1)
	// Coinage on bit mask 0x00f0
	PORT_BIT( 0x00f0, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Modified below

	PORT_START("DSWB")
	TOAPLAN_DIFFICULTY_LOC(SW2)
	// Per-game features on bit mask 0x00fc
	PORT_BIT( 0x00fc, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Modified below
INPUT_PORTS_END


static INPUT_PORTS_START( 3b )
	PORT_INCLUDE( 2b )

	PORT_MODIFY("IN1")
	TOAPLAN_JOY_UDLR_3_BUTTONS( 1 )

	PORT_MODIFY("IN2")
	TOAPLAN_JOY_UDLR_3_BUTTONS( 2 )
INPUT_PORTS_END


static INPUT_PORTS_START( sstriker )
	PORT_INCLUDE( 3b )

	PORT_MODIFY("DSWA")
	PORT_DIPNAME( 0x0001,   0x0000, DEF_STR( Free_Play ) )  PORT_DIPLOCATION("SW1:!1")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0001, DEF_STR( On ) )
	// Various features on bit mask 0x000e - see above
	TOAPLAN_COINAGE_DUAL_LOC( JMPR, 0x0e, 0x04, SW1 )

	PORT_MODIFY("DSWB")
	// Difficulty on bit mask 0x0003 - see above
	PORT_DIPNAME( 0x000c,   0x0000, DEF_STR( Bonus_Life ) )     PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(        0x000c, DEF_STR( None ) )
	PORT_DIPSETTING(        0x0008, "200k only" )
	PORT_DIPSETTING(        0x0000, "Every 300k" )
	PORT_DIPSETTING(        0x0004, "200k and 500k" )
	PORT_DIPNAME( 0x0030,   0x0000, DEF_STR( Lives ) )          PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(        0x0030, "1" )
	PORT_DIPSETTING(        0x0020, "2" )
	PORT_DIPSETTING(        0x0000, "3" )
	PORT_DIPSETTING(        0x0010, "5" )
	PORT_DIPNAME( 0x0040,   0x0000, "Invulnerability (Cheat)" ) PORT_DIPLOCATION("SW2:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0040, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080,   0x0000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0080, DEF_STR( No ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( Yes ) )

	PORT_START("JMPR")
	PORT_CONFNAME( 0x0001,  0x0001, "FBI Logo" )        //PORT_CONFLOCATION("JP:!4")
	PORT_CONFSETTING(       0x0001, DEF_STR( Off ) )
	PORT_CONFSETTING(       0x0000, DEF_STR( On ) )
	PORT_CONFNAME( 0x000e,  0x0004, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!3,!2,!1")
	PORT_CONFSETTING(       0x0004, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x0002, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0006, DEF_STR( Southeast_Asia ) )
	PORT_CONFSETTING(       0x0008, DEF_STR( China ) )
	PORT_CONFSETTING(       0x000a, DEF_STR( Korea ) )
	PORT_CONFSETTING(       0x000c, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x000e, DEF_STR( Taiwan ) )
INPUT_PORTS_END


static INPUT_PORTS_START( sstrikerk ) // Although the region jumper is functional, it's a Korean board / version
	PORT_INCLUDE( sstriker )

	PORT_MODIFY("JMPR")
	PORT_CONFNAME( 0x000e,  0x000a, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!3,!2,!1")
	PORT_CONFSETTING(       0x0004, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x0002, DEF_STR( USA ) )
	PORT_CONFSETTING(       0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0006, DEF_STR( Southeast_Asia ) )
	PORT_CONFSETTING(       0x0008, DEF_STR( China ) )
	PORT_CONFSETTING(       0x000a, "Korea (Unite Trading)" )
	PORT_CONFSETTING(       0x000c, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x000e, DEF_STR( Taiwan ) )
INPUT_PORTS_END


static INPUT_PORTS_START( mahoudai )
	PORT_INCLUDE( sstriker )

	PORT_MODIFY("JMPR")
	// Effectively unused by this set - see notes
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( kingdmgp )
	PORT_INCLUDE( sstriker )

	// The code and lookup tables pertaining to the jumpers are almost identical to sstriker.
	// However, this set apparently lacks (reachable) code to display the FBI logo,
	// even though the logo itself is present in the gfx ROMs.
	PORT_MODIFY("JMPR")
	PORT_CONFNAME( 0x0001,  0x0000, DEF_STR( Unused ) ) //PORT_CONFLOCATION("JP:!4")
	PORT_CONFSETTING(       0x0000, DEF_STR( Off ) )
	PORT_CONFSETTING(       0x0001, DEF_STR( On ) )
	PORT_CONFNAME( 0x000e,  0x0004, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!3,!2,!1")
	PORT_CONFSETTING(       0x0004, DEF_STR( Europe ) )
	PORT_CONFSETTING(       0x0002, DEF_STR( USA ) )
//  PORT_CONFSETTING(        0x0000, DEF_STR( Japan ) )  // Corrupt title screen and text - use shippumd
	PORT_CONFSETTING(       0x0006, DEF_STR( Southeast_Asia ) )
	PORT_CONFSETTING(       0x0008, DEF_STR( China ) )
	PORT_CONFSETTING(       0x000a, "Korea (Unite Trading license)" )
	PORT_CONFSETTING(       0x000c, DEF_STR( Hong_Kong ) )
	PORT_CONFSETTING(       0x000e, DEF_STR( Taiwan ) )
INPUT_PORTS_END


static INPUT_PORTS_START( shippumd )
	PORT_INCLUDE( sstriker )

	PORT_MODIFY("JMPR")
	// Title screen and text are corrupt for anything but Japan
	PORT_BIT( 0xffff, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( bgaregga )
	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Unknown/Unused

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2) PORT_8WAY
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Unknown/Unused

	PORT_START("SYS")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	TOAPLAN_TEST_SWITCH( 0x04, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Unknown/Unused

	PORT_START("DSWA")
	PORT_SERVICE_DIPLOC(0x0001, IP_ACTIVE_HIGH, "SW1:!1")
	PORT_DIPNAME( 0x0002,   0x0000, "Credits to Start" )    PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(        0x0000, "1" )
	PORT_DIPSETTING(        0x0002, "2" )
	PORT_DIPNAME( 0x001c,   0x0000, DEF_STR( Coin_A ) )     PORT_DIPLOCATION("SW1:!3,!4,!5")
	PORT_DIPSETTING(        0x0018, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(        0x0014, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(        0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(        0x0004, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(        0x0008, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(        0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(        0x001c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x00e0,   0x0000, DEF_STR( Coin_B ) )     PORT_CONDITION("DSWA", 0x001c, NOTEQUALS, 0x001c)   PORT_DIPLOCATION("SW1:!6,!7,!8")
	PORT_DIPSETTING(        0x00c0, DEF_STR( 4C_1C ) )      PORT_CONDITION("DSWA", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x00a0, DEF_STR( 3C_1C ) )      PORT_CONDITION("DSWA", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x0080, DEF_STR( 2C_1C ) )      PORT_CONDITION("DSWA", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x0000, DEF_STR( 1C_1C ) )      PORT_CONDITION("DSWA", 0x001c, NOTEQUALS, 0x001c)
//  PORT_DIPSETTING(        0x00e0, DEF_STR( 1C_1C ) )      PORT_CONDITION("DSWA", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x0020, DEF_STR( 1C_2C ) )      PORT_CONDITION("DSWA", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x0040, DEF_STR( 1C_3C ) )      PORT_CONDITION("DSWA", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x0060, DEF_STR( 1C_4C ) )      PORT_CONDITION("DSWA", 0x001c, NOTEQUALS, 0x001c)
	// When Coin_A is set to Free_Play, Coin_A becomes Coin_A and Coin_B, and the following dips occur
	PORT_DIPNAME( 0x0020,   0x0000, "Joystick Mode" )       PORT_CONDITION("DSWA", 0x001c, EQUALS, 0x001c)  PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(        0x0000, "90 degrees ACW" )      PORT_CONDITION("DSWA", 0x001c, EQUALS, 0x001c)
	PORT_DIPSETTING(        0x0020, DEF_STR( Normal ) )     PORT_CONDITION("DSWA", 0x001c, EQUALS, 0x001c)
	PORT_DIPNAME( 0x0040,   0x0000, "Effect" )              PORT_CONDITION("DSWA", 0x001c, EQUALS, 0x001c)  PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )        PORT_CONDITION("DSWA", 0x001c, EQUALS, 0x001c)
	PORT_DIPSETTING(        0x0040, DEF_STR( On ) )         PORT_CONDITION("DSWA", 0x001c, EQUALS, 0x001c)
	PORT_DIPNAME( 0x0080,   0x0000, "Music" )               PORT_CONDITION("DSWA", 0x001c, EQUALS, 0x001c)  PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )        PORT_CONDITION("DSWA", 0x001c, EQUALS, 0x001c)
	PORT_DIPSETTING(        0x0080, DEF_STR( On ) )         PORT_CONDITION("DSWA", 0x001c, EQUALS, 0x001c)

	PORT_START("DSWB")
	PORT_DIPNAME( 0x0003,   0x0000, DEF_STR( Difficulty ) )     PORT_DIPLOCATION("SW2:!1,!2")
	PORT_DIPSETTING(        0x0001, DEF_STR( Easy ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(        0x0002, DEF_STR( Hard ) )
	PORT_DIPSETTING(        0x0003, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0004,   0x0000, DEF_STR( Flip_Screen ) )    PORT_DIPLOCATION("SW2:!3")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008,   0x0000, DEF_STR( Demo_Sounds ) )    PORT_DIPLOCATION("SW2:!4")
	PORT_DIPSETTING(        0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0070,   0x0000, DEF_STR( Lives ) )          PORT_DIPLOCATION("SW2:!5,!6,!7")
	PORT_DIPSETTING(        0x0030, "1" )
	PORT_DIPSETTING(        0x0020, "2" )
	PORT_DIPSETTING(        0x0000, "3" )
	PORT_DIPSETTING(        0x0010, "4" )
	PORT_DIPSETTING(        0x0040, "5" )
	PORT_DIPSETTING(        0x0050, "6" )
	PORT_DIPSETTING(        0x0060, DEF_STR( Infinite ) )
	PORT_DIPSETTING(        0x0070, "Invulnerability (Cheat)" )
	PORT_DIPNAME( 0x0080,   0x0000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0000, DEF_STR( None ) )       PORT_CONDITION("JMPR",0x0003,NOTEQUALS,0x0000)  // Non-Japan
	PORT_DIPSETTING(        0x0080, "Every 2000k" )         PORT_CONDITION("JMPR",0x0003,NOTEQUALS,0x0000)  // Non-Japan
	PORT_DIPSETTING(        0x0080, "1000k and 2000k" )     PORT_CONDITION("JMPR",0x0003,EQUALS,0x0000) // Japan
	PORT_DIPSETTING(        0x0000, "Every 1000k" )         PORT_CONDITION("JMPR",0x0003,EQUALS,0x0000) // Japan

	PORT_START("JMPR") // DSW3 and jumper
	PORT_DIPNAME( 0x0008,  0x0000, "Stage Edit" ) PORT_DIPLOCATION("SW3:!1")
	PORT_DIPSETTING(       0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004,  0x0000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW3:!2")
	PORT_DIPSETTING(       0x0004, DEF_STR( No ) )
	PORT_DIPSETTING(       0x0000, DEF_STR( Yes ) )
	PORT_CONFNAME( 0x0003,  0x0001, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!2,!1")
	PORT_CONFSETTING(       0x0001, "Europe (Tuning)" )
	PORT_CONFSETTING(       0x0002, "USA (Fabtek)" )
	PORT_CONFSETTING(       0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0003, DEF_STR( Asia ) )
INPUT_PORTS_END


static INPUT_PORTS_START( bgareggahk )
	PORT_INCLUDE( bgaregga )

	PORT_MODIFY("JMPR")
	PORT_CONFNAME( 0x0003,  0x0003, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!2,!1")
	PORT_CONFSETTING(       0x0001, "Austria (Tuning)" )
	// These two settings end up reporting ROM-0 as BAD
//  PORT_CONFSETTING(        0x0002, "USA (Fabtek)" )
//  PORT_CONFSETTING(        0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0003, "Hong Kong (Metrotainment)" )
INPUT_PORTS_END


static INPUT_PORTS_START( bgareggatw )
	PORT_INCLUDE( bgaregga )

	PORT_MODIFY("JMPR")
	PORT_CONFNAME( 0x0003,  0x0003, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!2,!1")
	PORT_CONFSETTING(       0x0001, "Germany (Tuning)" )
	// These two settings end up reporting ROM-0 as BAD
//  PORT_CONFSETTING(        0x0002, "USA (Fabtek)" )
//  PORT_CONFSETTING(        0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0003, "Taiwan (Liang Hwa)" )
INPUT_PORTS_END


static INPUT_PORTS_START( bgareggak )
	PORT_INCLUDE( bgaregga )

	PORT_MODIFY("JMPR")
	PORT_CONFNAME( 0x0003,  0x0003, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!2,!1")
	PORT_CONFSETTING(       0x0001, "Greece" )
	PORT_CONFSETTING(       0x0003, "Korea" )
INPUT_PORTS_END


static INPUT_PORTS_START( bgareggacn )
	PORT_INCLUDE( bgaregga )

	PORT_MODIFY("JMPR")
	PORT_CONFNAME( 0x0003,  0x0003, DEF_STR( Region ) ) //PORT_CONFLOCATION("JP:!2,!1")
	PORT_CONFSETTING(       0x0001, "Denmark (Tuning)" )
	// These two settings end up reporting ROM-0 as BAD
//  PORT_CONFSETTING(        0x0002, "USA (Fabtek)" )
//  PORT_CONFSETTING(        0x0000, DEF_STR( Japan ) )
	PORT_CONFSETTING(       0x0003, DEF_STR( China ) )
INPUT_PORTS_END


static INPUT_PORTS_START( bgareggabl )
	PORT_INCLUDE( bgaregga )

	PORT_MODIFY("DSWB") // Always uses bonus life settings from Japan region
	PORT_DIPNAME( 0x0080,   0x0000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!8")
	PORT_DIPSETTING(        0x0080, "1000k and 2000k" )
	PORT_DIPSETTING(        0x0000, "Every 1000k" )

	PORT_MODIFY("JMPR") // Region is hard-coded
	PORT_BIT( 0x0003, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( batrider )
	PORT_START("IN")        // Player Inputs
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x8080, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")       // DSWA and DSWB
	PORT_SERVICE_DIPLOC(0x0001, IP_ACTIVE_HIGH, "SW1:!1")
	PORT_DIPNAME( 0x0002,   0x0000, "Credits to Start" )    PORT_CONDITION("DSW", 0x001c, NOTEQUALS, 0x001c)    PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(        0x0000, "1" )                   PORT_CONDITION("DSW", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x0002, "2" )                   PORT_CONDITION("DSW", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPNAME( 0x0002,   0x0000, "Joystick Mode" )       PORT_CONDITION("DSW", 0x001c, EQUALS, 0x001c)       PORT_DIPLOCATION("SW1:!2")
	PORT_DIPSETTING(        0x0000, DEF_STR( Normal ) )     PORT_CONDITION("DSW", 0x001c, EQUALS, 0x001c)
	PORT_DIPSETTING(        0x0002, "90 degrees ACW" )      PORT_CONDITION("DSW", 0x001c, EQUALS, 0x001c)
	PORT_DIPNAME( 0x001c,   0x0000, DEF_STR( Coin_A ) )     PORT_DIPLOCATION("SW1:!3,!4,!5")
	PORT_DIPSETTING(        0x0018, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(        0x0014, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(        0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(        0x0004, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(        0x0008, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(        0x000c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(        0x001c, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x00e0,   0x0000, DEF_STR( Coin_B ) )     PORT_CONDITION("DSW", 0x001c, NOTEQUALS, 0x001c)    PORT_DIPLOCATION("SW1:!6,!7,!8")
	PORT_DIPSETTING(        0x00c0, DEF_STR( 4C_1C ) )      PORT_CONDITION("DSW", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x00a0, DEF_STR( 3C_1C ) )      PORT_CONDITION("DSW", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x0080, DEF_STR( 2C_1C ) )      PORT_CONDITION("DSW", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x0000, DEF_STR( 1C_1C ) )      PORT_CONDITION("DSW", 0x001c, NOTEQUALS, 0x001c)
//  PORT_DIPSETTING(        0x00e0, DEF_STR( 1C_1C ) )      PORT_CONDITION("DSW", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x0020, DEF_STR( 1C_2C ) )      PORT_CONDITION("DSW", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x0040, DEF_STR( 1C_3C ) )      PORT_CONDITION("DSW", 0x001c, NOTEQUALS, 0x001c)
	PORT_DIPSETTING(        0x0060, DEF_STR( 1C_4C ) )      PORT_CONDITION("DSW", 0x001c, NOTEQUALS, 0x001c)
	// When Coin_A is set to Free_Play, Coin_A becomes Coin_A and Coin_B, and the following dips occur
	PORT_DIPNAME( 0x0020,   0x0000, "Hit Score" )           PORT_CONDITION("DSW", 0x001c, EQUALS, 0x001c)   PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )        PORT_CONDITION("DSW", 0x001c, EQUALS, 0x001c)
	PORT_DIPSETTING(        0x0020, DEF_STR( On ) )         PORT_CONDITION("DSW", 0x001c, EQUALS, 0x001c)
	PORT_DIPNAME( 0x0040,   0x0000, "Sound Effect" )        PORT_CONDITION("DSW", 0x001c, EQUALS, 0x001c)   PORT_DIPLOCATION("SW1:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )        PORT_CONDITION("DSW", 0x001c, EQUALS, 0x001c)
	PORT_DIPSETTING(        0x0040, DEF_STR( On ) )         PORT_CONDITION("DSW", 0x001c, EQUALS, 0x001c)
	PORT_DIPNAME( 0x0080,   0x0000, "Music" )               PORT_CONDITION("DSW", 0x001c, EQUALS, 0x001c)   PORT_DIPLOCATION("SW1:!8")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )        PORT_CONDITION("DSW", 0x001c, EQUALS, 0x001c)
	PORT_DIPSETTING(        0x0080, DEF_STR( On ) )         PORT_CONDITION("DSW", 0x001c, EQUALS, 0x001c)
	PORT_DIPNAME( 0x0300,   0x0000, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:!1,!2")
	PORT_DIPSETTING(        0x0100, DEF_STR( Easy ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(        0x0200, DEF_STR( Hard ) )
	PORT_DIPSETTING(        0x0300, DEF_STR( Very_Hard ) )
	PORT_DIPNAME( 0x0c00,   0x0000, "Timer" )               PORT_DIPLOCATION("SW2:!3,!4")
	PORT_DIPSETTING(        0x0c00, DEF_STR( Highest ) )
	PORT_DIPSETTING(        0x0800, DEF_STR( High ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( Normal ) )
	PORT_DIPSETTING(        0x0400, DEF_STR( Low ) )
	PORT_DIPNAME( 0x3000,   0x0000, DEF_STR( Lives ) )      PORT_DIPLOCATION("SW2:!5,!6")
	PORT_DIPSETTING(        0x3000, "1" )
	PORT_DIPSETTING(        0x2000, "2" )
	PORT_DIPSETTING(        0x0000, "3" )
	PORT_DIPSETTING(        0x1000, "4" )
	PORT_DIPNAME( 0xc000,   0x0000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!7,!8")
	PORT_DIPSETTING(        0xc000, DEF_STR( None ) )
	PORT_DIPSETTING(        0x8000, "Every 2000k" )
	PORT_DIPSETTING(        0x0000, "Every 1500k" )
	PORT_DIPSETTING(        0x4000, "Every 1000k" )

	PORT_START("SYS-DSW")   // Coin/System and DSWC
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN)
	TOAPLAN_TEST_SWITCH( 0x0004, IP_ACTIVE_HIGH )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_DIPNAME( 0x0100,   0x0000, DEF_STR( Flip_Screen ) )    PORT_DIPLOCATION("SW3:!1")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0100, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200,   0x0000, DEF_STR( Demo_Sounds ) )    PORT_DIPLOCATION("SW3:!2")
	PORT_DIPSETTING(        0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400,   0x0000, "Stage Edit" )              PORT_DIPLOCATION("SW3:!3")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0400, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800,   0x0000, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SW3:!4")
	PORT_DIPSETTING(        0x0800, DEF_STR( No ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x1000,   0x0000, "Invulnerability (Cheat)" )         PORT_DIPLOCATION("SW3:!5")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x1000, DEF_STR( On ) )
	// These dips are shown only when Coin_A is set to Free_Play, but they work in normal play mode too
	PORT_DIPNAME( 0x2000,   0x0000, "Guest Players" )           PORT_DIPLOCATION("SW3:!6")
	PORT_DIPSETTING(        0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000,   0x0000, "Player Select" )           PORT_DIPLOCATION("SW3:!7")
	PORT_DIPSETTING(        0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000,   0x0000, "Special Course" )          PORT_DIPLOCATION("SW3:!8")
	PORT_DIPSETTING(        0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( batriderj )
	PORT_INCLUDE( batrider )

	PORT_MODIFY("SYS-DSW")  // Coin/System and DSWC
	// These dips are shown only when Coin_A is set to Free_Play, but they work in normal play mode too
	PORT_DIPNAME( 0x2000,   0x0000, "Guest Players" )           PORT_DIPLOCATION("SW3:!6")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x2000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000,   0x0000, "Player Select" )           PORT_DIPLOCATION("SW3:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000,   0x0000, "Special Course" )          PORT_DIPLOCATION("SW3:!8")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x8000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( bbakraid )
	PORT_INCLUDE( batrider )

	PORT_MODIFY("DSW")       // DSWA and DSWB
	PORT_DIPNAME( 0xc000,   0x0000, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:!7,!8")
	PORT_DIPSETTING(        0xc000, DEF_STR( None ) )
	PORT_DIPSETTING(        0x8000, "Every 4000k" )
	PORT_DIPSETTING(        0x4000, "Every 3000k" )
	PORT_DIPSETTING(        0x0000, "Every 2000k" )

	PORT_MODIFY("SYS-DSW")   // Coin/System and DSW-3
	PORT_DIPNAME( 0x2000,   0x0000, "Save Scores" )             PORT_DIPLOCATION("SW3:!6")
	PORT_DIPSETTING(        0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000,   0x0000, DEF_STR( Unused ) )         PORT_DIPLOCATION("SW3:!7")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x4000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000,   0x0000, DEF_STR( Unused ) )         PORT_DIPLOCATION("SW3:!8")
	PORT_DIPSETTING(        0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(        0x8000, DEF_STR( On ) )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write))
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::di_write))
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write))
INPUT_PORTS_END


static INPUT_PORTS_START( nprobowl )
	PORT_START("IN")        // Player Inputs
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_NAME("Set (Relay)")
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Gutter L")
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_NAME("Gutter R")
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_NAME("Ballout")
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_NAME("Ballpass")
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_NAME("Mot Home")
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // no effect in test mode
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // no effect in test mode

	PORT_START("DSW")       // SW0323 and SW0324
	PORT_SERVICE_DIPLOC(   0x0001, IP_ACTIVE_HIGH, "SW0323:!1")
	PORT_DIPUNKNOWN_DIPLOC(0x0002, 0x0000, "SW0323:!2")
	PORT_DIPNAME(          0x0004, 0x0004, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW0323:!3")
	PORT_DIPSETTING(       0x0004, DEF_STR( On ) )
	PORT_DIPSETTING(       0x0000, DEF_STR( Off ) )
	PORT_DIPUNKNOWN_DIPLOC(0x0008, 0x0000, "SW0323:!4")
	PORT_DIPNAME(          0x0070, 0x0000, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW0323:!5,!6,!7")
	PORT_DIPSETTING(       0x0070, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(       0x0060, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(       0x0050, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(       0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(       0x0000, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(       0x0010, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(       0x0020, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(       0x0030, DEF_STR( 1C_4C ) )
	PORT_DIPNAME(          0x0080, 0x0000, DEF_STR( Free_Play ) )  PORT_DIPLOCATION("SW0323:!8")
	PORT_DIPSETTING(       0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(       0x0080, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC(0x0100, 0x0000, "SW0324:!1")
	PORT_DIPUNKNOWN_DIPLOC(0x0200, 0x0000, "SW0324:!2")
	PORT_DIPUNKNOWN_DIPLOC(0x0400, 0x0000, "SW0324:!3")
	PORT_DIPUNKNOWN_DIPLOC(0x0800, 0x0000, "SW0324:!4")
	PORT_DIPUNKNOWN_DIPLOC(0x1000, 0x0000, "SW0324:!5")
	PORT_DIPUNKNOWN_DIPLOC(0x2000, 0x0000, "SW0324:!6")
	PORT_DIPUNKNOWN_DIPLOC(0x4000, 0x0000, "SW0324:!7")
	PORT_DIPUNKNOWN_DIPLOC(0x8000, 0x0000, "SW0324:!8")

	PORT_START("UNK")   // ??
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


void raizing_base_state::raizing_z80_bankswitch_w(u8 data)
{
	m_audiobank->set_entry(data & 0x0f);
}


// bgaregga and batrider don't actually have a NMK112, but rather a GAL
// programmed to bankswitch the sound ROMs in a similar fashion.
// it may not be a coincidence that the composer and sound designer for
// these two games, Manabu "Santaruru" Namiki, came to Raizing from NMK...

void raizing_base_state::raizing_oki_bankswitch_w(offs_t offset, u8 data)
{
	m_raizing_okibank[(offset & 4) >> 2][offset & 3]->set_entry(data & 0xf);
	m_raizing_okibank[(offset & 4) >> 2][4 + (offset & 3)]->set_entry(data & 0xf);
	offset++;
	data >>= 4;
	m_raizing_okibank[(offset & 4) >> 2][offset & 3]->set_entry(data & 0xf);
	m_raizing_okibank[(offset & 4) >> 2][4 + (offset & 3)]->set_entry(data & 0xf);
}


u8 raizing_base_state::bgaregga_E01D_r()
{
	// the Z80 reads this address during its IRQ routine,
	// and reads the soundlatch only if the lowest bit is clear.
	return m_soundlatch[0]->pending_r() ? 0 : 1;
}


u16 batrider_state::batrider_z80_busack_r()
{
	// Bit 0x01 returns the status of BUSAK from the Z80.
	// These accesses are made when the 68K wants to read the Z80
	// ROM code. Failure to return the correct status incurrs a Sound Error.

	return m_z80_busreq;    // Loop BUSRQ to BUSAK
}


void batrider_state::batrider_z80_busreq_w(u8 data)
{
	m_z80_busreq = (data & 0x01);   // see batrider_z80_busack_r above
}


u16 batrider_state::batrider_z80rom_r(offs_t offset)
{
	return m_z80_rom[offset];
}

// these two latches are always written together, via a single move.l instruction
void batrider_state::batrider_soundlatch_w(u8 data)
{
	m_soundlatch[0]->write(data & 0xff);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}


void batrider_state::batrider_soundlatch2_w(u8 data)
{
	m_soundlatch[1]->write(data & 0xff);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void batrider_state::batrider_unknown_sound_w(u16 data)
{
	// the 68K writes here when it wants a sound acknowledge IRQ from the Z80
	// for bbakraid this is on every sound command; for batrider, only on certain commands
}


void batrider_state::batrider_clear_sndirq_w(u16 data)
{
	// not sure whether this is correct
	// the 68K writes here during the sound IRQ handler, and nowhere else...
	m_maincpu->set_input_line(m_sndirq_line, CLEAR_LINE);
}


void batrider_state::batrider_sndirq_w(u8 data)
{
	// if batrider_clear_sndirq_w() is correct, should this be ASSERT_LINE?
	m_maincpu->set_input_line(m_sndirq_line, HOLD_LINE);
}


void batrider_state::batrider_clear_nmi_w(u8 data)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}


u16 bbakraid_state::bbakraid_eeprom_r()
{
	// Bit 0x01 returns the status of BUSAK from the Z80.
	// BUSRQ is activated via bit 0x10 on the EEPROM write port.
	// These accesses are made when the 68K wants to read the Z80
	// ROM code. Failure to return the correct status incurrs a Sound Error.

	u8 data;
	data  = ((m_eeprom->do_read() & 0x01) << 4);
	data |= ((m_z80_busreq >> 4) & 0x01);   // Loop BUSRQ to BUSAK

	return data;
}


void bbakraid_state::bbakraid_eeprom_w(u8 data)
{
	if (data & ~0x1f)
		logerror("CPU #0 PC:%06X - Unknown EEPROM data being written %02X\n",m_maincpu->pc(),data);

	m_eepromout->write(data, 0xff);

	m_z80_busreq = data & 0x10; // see bbakraid_eeprom_r above
}

void raizing_base_state::shippumd_coin_w(u8 data)
{
	coin_w(data & ~0x10);
	m_oki[0]->set_rom_bank(BIT(data, 4));
}

INTERRUPT_GEN_MEMBER(raizing_base_state::bbakraid_snd_interrupt)
{
	device.execute().set_input_line(0, HOLD_LINE);
}

MACHINE_RESET_MEMBER(raizing_base_state, bgaregga)
{
	driver_device::machine_reset();
	for (int chip = 0; chip < 2; chip++)
	{
		for (int i = 0; i < 8; i++)
		{
			if (m_raizing_okibank[chip][i] != nullptr)
				m_raizing_okibank[chip][i]->set_entry(0);
		}
	}
}

void raizing_base_state::machine_start()
{
	driver_device::machine_start();
	save_item(NAME(m_z80_busreq));
}


void sstriker_state::mahoudai_68k_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x218000, 0x21bfff).rw(FUNC(sstriker_state::shared_ram_r), FUNC(sstriker_state::shared_ram_w)).umask16(0x00ff);
	map(0x21c01d, 0x21c01d).w(FUNC(sstriker_state::coin_w));
	map(0x21c020, 0x21c021).portr("IN1");
	map(0x21c024, 0x21c025).portr("IN2");
	map(0x21c028, 0x21c029).portr("SYS");
	map(0x21c02c, 0x21c02d).portr("DSWA");
	map(0x21c030, 0x21c031).portr("DSWB");
	map(0x21c034, 0x21c035).portr("JMPR");
	map(0x21c03c, 0x21c03d).r(m_vdp, FUNC(gp9001vdp_device::vdpcount_r));
	map(0x300000, 0x30000d).rw(m_vdp, FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x401000, 0x4017ff).ram();                         // Unused palette RAM
	map(0x500000, 0x501fff).ram().w(FUNC(sstriker_state::tx_videoram_w)).share(m_tx_videoram);
	map(0x502000, 0x502fff).ram().share(m_tx_lineselect);
	map(0x503000, 0x5031ff).ram().w(FUNC(sstriker_state::tx_linescroll_w)).share(m_tx_linescroll);
	map(0x503200, 0x503fff).ram();
}


void sstriker_state::shippumd_68k_mem(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x218000, 0x21bfff).rw(FUNC(sstriker_state::shared_ram_r), FUNC(sstriker_state::shared_ram_w)).umask16(0x00ff);
//  map(0x21c008, 0x21c009).nopw();                    // ???
	map(0x21c01d, 0x21c01d).w(FUNC(sstriker_state::shippumd_coin_w)); // Coin count/lock + oki bankswitch
	map(0x21c020, 0x21c021).portr("IN1");
	map(0x21c024, 0x21c025).portr("IN2");
	map(0x21c028, 0x21c029).portr("SYS");
	map(0x21c02c, 0x21c02d).portr("DSWA");
	map(0x21c030, 0x21c031).portr("DSWB");
	map(0x21c034, 0x21c035).portr("JMPR");
	map(0x21c03c, 0x21c03d).r(m_vdp, FUNC(gp9001vdp_device::vdpcount_r));
	map(0x300000, 0x30000d).rw(m_vdp, FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x401000, 0x4017ff).ram();                         // Unused palette RAM
	map(0x500000, 0x501fff).ram().w(FUNC(sstriker_state::tx_videoram_w)).share(m_tx_videoram);
	map(0x502000, 0x502fff).ram().share(m_tx_lineselect);
	map(0x503000, 0x5031ff).ram().w(FUNC(sstriker_state::tx_linescroll_w)).share(m_tx_linescroll);
	map(0x503200, 0x503fff).ram();
}


void bgaregga_state::bgaregga_68k_mem(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x100000, 0x10ffff).ram();
	map(0x218000, 0x21bfff).rw(FUNC(bgaregga_state::shared_ram_r), FUNC(bgaregga_state::shared_ram_w)).umask16(0x00ff);
	map(0x21c01d, 0x21c01d).w(FUNC(bgaregga_state::coin_w));
	map(0x21c020, 0x21c021).portr("IN1");
	map(0x21c024, 0x21c025).portr("IN2");
	map(0x21c028, 0x21c029).portr("SYS");
	map(0x21c02c, 0x21c02d).portr("DSWA");
	map(0x21c030, 0x21c031).portr("DSWB");
	map(0x21c034, 0x21c035).portr("JMPR");
	map(0x21c03c, 0x21c03d).r(m_vdp, FUNC(gp9001vdp_device::vdpcount_r));
	map(0x300000, 0x30000d).rw(m_vdp, FUNC(gp9001vdp_device::read), FUNC(gp9001vdp_device::write));
	map(0x400000, 0x400fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x500000, 0x501fff).ram().w(FUNC(bgaregga_state::tx_videoram_w)).share(m_tx_videoram);
	map(0x502000, 0x502fff).ram().share(m_tx_lineselect);
	map(0x503000, 0x5031ff).ram().w(FUNC(bgaregga_state::tx_linescroll_w)).share(m_tx_linescroll);
	map(0x503200, 0x503fff).ram();
	map(0x600001, 0x600001).w(m_soundlatch[0], FUNC(generic_latch_8_device::write));
}


void batrider_state::batrider_dma_mem(address_map &map)
{
	map(0x0000, 0x1fff).ram().w(FUNC(batrider_state::tx_videoram_w)).share(m_tx_videoram);
	map(0x2000, 0x2fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");
	map(0x3000, 0x31ff).ram().share(m_tx_lineselect);
	map(0x3200, 0x33ff).ram().w(FUNC(batrider_state::tx_linescroll_w)).share(m_tx_linescroll);
	map(0x3400, 0x7fff).ram();
	map(0x8000, 0xffff).ram().w(FUNC(batrider_state::batrider_tx_gfxram_w)).share(m_tx_gfxram);
}


void batrider_state::batrider_68k_mem(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	// actually 200000 - 20ffff is probably all main RAM, and the text and palette RAM are written via DMA
	map(0x200000, 0x207fff).ram().share(m_mainram);
	map(0x208000, 0x20ffff).ram();
	map(0x300000, 0x37ffff).r(FUNC(batrider_state::batrider_z80rom_r));
	map(0x400000, 0x40000d).lrw16(
							NAME([this](offs_t offset, u16 mem_mask) { return m_vdp->read(offset ^ (0xc/2), mem_mask); }),
							NAME([this](offs_t offset, u16 data, u16 mem_mask) { m_vdp->write(offset ^ (0xc/2), data, mem_mask); }));
	map(0x500000, 0x500001).portr("IN");
	map(0x500002, 0x500003).portr("SYS-DSW");
	map(0x500004, 0x500005).portr("DSW");
	map(0x500006, 0x500007).r(m_vdp, FUNC(gp9001vdp_device::vdpcount_r));
	map(0x500009, 0x500009).r(m_soundlatch[2], FUNC(generic_latch_8_device::read));
	map(0x50000b, 0x50000b).r(m_soundlatch[3], FUNC(generic_latch_8_device::read));
	map(0x50000c, 0x50000d).r(FUNC(batrider_state::batrider_z80_busack_r));
	map(0x500011, 0x500011).w(FUNC(batrider_state::coin_w));
	map(0x500021, 0x500021).w(FUNC(batrider_state::batrider_soundlatch_w));
	map(0x500023, 0x500023).w(FUNC(batrider_state::batrider_soundlatch2_w));
	map(0x500024, 0x500025).w(FUNC(batrider_state::batrider_unknown_sound_w));
	map(0x500026, 0x500027).w(FUNC(batrider_state::batrider_clear_sndirq_w));
	map(0x500061, 0x500061).w(FUNC(batrider_state::batrider_z80_busreq_w));
	map(0x500080, 0x500081).w(FUNC(batrider_state::batrider_textdata_dma_w));
	map(0x500082, 0x500083).w(FUNC(batrider_state::batrider_pal_text_dma_w));
	map(0x5000c0, 0x5000cf).w(FUNC(batrider_state::batrider_objectbank_w)).umask16(0x00ff);
}


void bbakraid_state::bbakraid_68k_mem(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	// actually 200000 - 20ffff is probably all main RAM, and the text and palette RAM are written via DMA
	map(0x200000, 0x207fff).ram().share(m_mainram);
	map(0x208000, 0x20ffff).ram();
	map(0x300000, 0x33ffff).r(FUNC(bbakraid_state::batrider_z80rom_r));
	map(0x400000, 0x40000d).lrw16(
							NAME([this](offs_t offset, u16 mem_mask) { return m_vdp->read(offset ^ (0xc/2), mem_mask); }),
							NAME([this](offs_t offset, u16 data, u16 mem_mask) { m_vdp->write(offset ^ (0xc/2), data, mem_mask); }));
	map(0x500000, 0x500001).portr("IN");
	map(0x500002, 0x500003).portr("SYS-DSW");
	map(0x500004, 0x500005).portr("DSW");
	map(0x500006, 0x500007).r(m_vdp, FUNC(gp9001vdp_device::vdpcount_r));
	map(0x500009, 0x500009).w(FUNC(bbakraid_state::coin_w));
	map(0x500011, 0x500011).r(m_soundlatch[2], FUNC(generic_latch_8_device::read));
	map(0x500013, 0x500013).r(m_soundlatch[3], FUNC(generic_latch_8_device::read));
	map(0x500015, 0x500015).w(FUNC(bbakraid_state::batrider_soundlatch_w));
	map(0x500017, 0x500017).w(FUNC(bbakraid_state::batrider_soundlatch2_w));
	map(0x500018, 0x500019).r(FUNC(bbakraid_state::bbakraid_eeprom_r));
	map(0x50001a, 0x50001b).w(FUNC(bbakraid_state::batrider_unknown_sound_w));
	map(0x50001c, 0x50001d).w(FUNC(bbakraid_state::batrider_clear_sndirq_w));
	map(0x50001f, 0x50001f).w(FUNC(bbakraid_state::bbakraid_eeprom_w));
	map(0x500080, 0x500081).w(FUNC(bbakraid_state::batrider_textdata_dma_w));
	map(0x500082, 0x500083).w(FUNC(bbakraid_state::batrider_pal_text_dma_w));
	map(0x5000c0, 0x5000cf).w(FUNC(bbakraid_state::batrider_objectbank_w)).umask16(0x00ff);
}


void nprobowl_state::nprobowl_68k_mem(address_map &map) // TODO: verify everything, implement oki banking
{
	map(0x000000, 0x0fffff).rom();
	map(0x200000, 0x207fff).ram().share(m_mainram);
	map(0x208000, 0x20ffff).ram();
	map(0x400000, 0x40000d).lrw16(
							NAME([this](offs_t offset, u16 mem_mask) { return m_vdp->read(offset ^ (0xc/2), mem_mask); }),
							NAME([this](offs_t offset, u16 data, u16 mem_mask) { m_vdp->write(offset ^ (0xc/2), data, mem_mask); }));
	map(0x500000, 0x500001).portr("IN");
	map(0x500002, 0x500003).portr("UNK");
	map(0x500004, 0x500005).portr("DSW");
	//map(0x500010, 0x500011).w();
	//map(0x500012, 0x500013).w();
	map(0x500021, 0x500021).w(m_oki[0], FUNC(okim6295_device::write));
	//map(0x500040, 0x500041).w();
	//map(0x500042, 0x500043).w();
	map(0x500060, 0x500061).lr16(NAME([this] () -> u16 { return machine().rand(); })); // TODO: Hack, probably checks something in the mechanical part, verify
	map(0x500080, 0x500081).w(FUNC(nprobowl_state::batrider_textdata_dma_w));
	map(0x500082, 0x500083).w(FUNC(nprobowl_state::batrider_pal_text_dma_w));
}


void sstriker_state::raizing_sound_z80_mem(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xdfff).ram().share(m_shared_ram);
	map(0xe000, 0xe001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xe004, 0xe004).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xe00e, 0xe00e).w(FUNC(sstriker_state::coin_w));
}


void bgaregga_state::bgaregga_sound_z80_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_audiobank);
	map(0xc000, 0xdfff).ram().share(m_shared_ram);
	map(0xe000, 0xe001).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0xe004, 0xe004).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xe006, 0xe008).w(FUNC(bgaregga_state::raizing_oki_bankswitch_w));
	map(0xe00a, 0xe00a).w(FUNC(bgaregga_state::raizing_z80_bankswitch_w));
	map(0xe00c, 0xe00c).w(m_soundlatch[0], FUNC(generic_latch_8_device::acknowledge_w));
	map(0xe01c, 0xe01c).r(m_soundlatch[0], FUNC(generic_latch_8_device::read));
	map(0xe01d, 0xe01d).r(FUNC(bgaregga_state::bgaregga_E01D_r));
}


void batrider_state::batrider_sound_z80_mem(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_audiobank);
	map(0xc000, 0xdfff).ram();
}


void batrider_state::batrider_sound_z80_port(address_map &map)
{
	map.global_mask(0xff);
	map(0x40, 0x40).w(m_soundlatch[2], FUNC(generic_latch_8_device::write));
	map(0x42, 0x42).w(m_soundlatch[3], FUNC(generic_latch_8_device::write));
	map(0x44, 0x44).w(FUNC(batrider_state::batrider_sndirq_w));
	map(0x46, 0x46).w(FUNC(batrider_state::batrider_clear_nmi_w));
	map(0x48, 0x48).r(m_soundlatch[0], FUNC(generic_latch_8_device::read));
	map(0x4a, 0x4a).r(m_soundlatch[1], FUNC(generic_latch_8_device::read));
	map(0x80, 0x81).rw("ymsnd", FUNC(ym2151_device::read), FUNC(ym2151_device::write));
	map(0x82, 0x82).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x84, 0x84).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x88, 0x88).w(FUNC(batrider_state::raizing_z80_bankswitch_w));
	map(0xc0, 0xc6).w(FUNC(batrider_state::raizing_oki_bankswitch_w));
}


void bbakraid_state::bbakraid_sound_z80_mem(address_map &map)
{
	map(0x0000, 0xbfff).rom();     // No banking? ROM only contains code and data up to 0x28DC
	map(0xc000, 0xffff).ram();
}


void bbakraid_state::bbakraid_sound_z80_port(address_map &map)
{
	map.global_mask(0xff);
	map(0x40, 0x40).w(m_soundlatch[2], FUNC(generic_latch_8_device::write));
	map(0x42, 0x42).w(m_soundlatch[3], FUNC(generic_latch_8_device::write));
	map(0x44, 0x44).w(FUNC(bbakraid_state::batrider_sndirq_w));
	map(0x46, 0x46).w(FUNC(bbakraid_state::batrider_clear_nmi_w));
	map(0x48, 0x48).r(m_soundlatch[0], FUNC(generic_latch_8_device::read));
	map(0x4a, 0x4a).r(m_soundlatch[1], FUNC(generic_latch_8_device::read));
	map(0x80, 0x81).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write));
}

// similar as NMK112, but GAL-driven; NOT actual NMK112 is present
template<unsigned Chip>
void raizing_base_state::raizing_oki(address_map &map)
{
	map(0x00000, 0x000ff).bankr(m_raizing_okibank[Chip][0]);
	map(0x00100, 0x001ff).bankr(m_raizing_okibank[Chip][1]);
	map(0x00200, 0x002ff).bankr(m_raizing_okibank[Chip][2]);
	map(0x00300, 0x003ff).bankr(m_raizing_okibank[Chip][3]);
	map(0x00400, 0x0ffff).bankr(m_raizing_okibank[Chip][4]);
	map(0x10000, 0x1ffff).bankr(m_raizing_okibank[Chip][5]);
	map(0x20000, 0x2ffff).bankr(m_raizing_okibank[Chip][6]);
	map(0x30000, 0x3ffff).bankr(m_raizing_okibank[Chip][7]);
}

#define XOR(a) WORD_XOR_LE(a)
#define LOC(x) (x+XOR(0))

static const gfx_layout batrider_tx_tilelayout =
{
	8,8,    /* 8x8 characters */
	1024,   /* 1024 characters */
	4,      /* 4 bits per pixel */
	{ STEP4(0,1) },
	{ XOR(0)*4, XOR(1)*4, XOR(2)*4, XOR(3)*4, XOR(4)*4, XOR(5)*4, XOR(6)*4, XOR(7)*4 },
	{ STEP8(0,4*8) },
	8*8*4
};

static GFXDECODE_START( gfx_textrom )
	GFXDECODE_ENTRY( "text", 0, gfx_8x8x4_packed_msb, 64*16, 64 )
GFXDECODE_END

static GFXDECODE_START( gfx_batrider )
	GFXDECODE_ENTRY( nullptr, 0, batrider_tx_tilelayout, 64*16, 64 )
GFXDECODE_END


void sstriker_state::mahoudai(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 32_MHz_XTAL/2);   // 16MHz, 32MHz Oscillator
	m_maincpu->set_addrmap(AS_PROGRAM, &sstriker_state::mahoudai_68k_mem);
	m_maincpu->reset_cb().set(FUNC(sstriker_state::reset));

	Z80(config, m_audiocpu, 32_MHz_XTAL/8);     // 4MHz, 32MHz Oscillator
	m_audiocpu->set_addrmap(AS_PROGRAM, &sstriker_state::raizing_sound_z80_mem);

	config.set_maximum_quantum(attotime::from_hz(600));

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(sstriker_state::screen_update_truxton2));
	m_screen->screen_vblank().set(FUNC(sstriker_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_textrom);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, gp9001vdp_device::VDP_PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp, 27_MHz_XTAL);
	m_vdp->set_palette(m_palette);
	m_vdp->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2151(config, "ymsnd", 27_MHz_XTAL/8).add_route(ALL_OUTPUTS, "mono", 0.35);

	OKIM6295(config, m_oki[0], 32_MHz_XTAL/32, okim6295_device::PIN7_HIGH);
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 0.5);
}

void sstriker_state::shippumd(machine_config &config)
{
	mahoudai(config);
	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &sstriker_state::shippumd_68k_mem);
}

void bgaregga_state::bgaregga(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 32_MHz_XTAL/2);   // 16MHz, 32MHz Oscillator
	m_maincpu->set_addrmap(AS_PROGRAM, &bgaregga_state::bgaregga_68k_mem);
	m_maincpu->reset_cb().set(FUNC(bgaregga_state::reset));

	Z80(config, m_audiocpu, 32_MHz_XTAL/8);     // 4MHz, 32MHz Oscillator
	m_audiocpu->set_addrmap(AS_PROGRAM, &bgaregga_state::bgaregga_sound_z80_mem);

	config.set_maximum_quantum(attotime::from_hz(6000));

	MCFG_MACHINE_RESET_OVERRIDE(bgaregga_state,bgaregga)

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(bgaregga_state::screen_update_truxton2));
	m_screen->screen_vblank().set(FUNC(bgaregga_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_textrom);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, gp9001vdp_device::VDP_PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp, 27_MHz_XTAL);
	m_vdp->set_palette(m_palette);
	m_vdp->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_4);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch[0]);
	m_soundlatch[0]->data_pending_callback().set_inputline(m_audiocpu, 0);
	m_soundlatch[0]->set_separate_acknowledge(true);

	YM2151(config, "ymsnd", 32_MHz_XTAL/8).add_route(ALL_OUTPUTS, "mono", 0.3);

	OKIM6295(config, m_oki[0], 32_MHz_XTAL/16, okim6295_device::PIN7_HIGH);
	m_oki[0]->set_addrmap(0, &raizing_base_state::raizing_oki<0>);
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 0.6);
}


void bgaregga_bootleg_state::bgareggabl(machine_config &config)
{
	bgaregga(config);

	m_screen->set_screen_update(FUNC(bgaregga_bootleg_state::screen_update_bootleg));
}

void batrider_state::batrider(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 32_MHz_XTAL/2);   // 16MHz, 32MHz Oscillator (verified)
	m_maincpu->set_addrmap(AS_PROGRAM, &batrider_state::batrider_68k_mem);
	m_maincpu->reset_cb().set(FUNC(batrider_state::reset));

	Z80(config, m_audiocpu, 32_MHz_XTAL/6);     // 5.333MHz, 32MHz Oscillator (verified)
	m_audiocpu->set_addrmap(AS_PROGRAM, &batrider_state::batrider_sound_z80_mem);
	m_audiocpu->set_addrmap(AS_IO, &batrider_state::batrider_sound_z80_port);

	config.set_maximum_quantum(attotime::from_hz(600));

	MCFG_MACHINE_RESET_OVERRIDE(batrider_state,bgaregga)

	ADDRESS_MAP_BANK(config, m_dma_space, 0);
	m_dma_space->set_addrmap(0, &batrider_state::batrider_dma_mem);
	m_dma_space->set_endianness(ENDIANNESS_BIG);
	m_dma_space->set_data_width(16);
	m_dma_space->set_addr_width(16);
	m_dma_space->set_stride(0x8000);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(batrider_state::screen_update_truxton2));
	m_screen->screen_vblank().set(FUNC(batrider_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_batrider);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, gp9001vdp_device::VDP_PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp, 27_MHz_XTAL);
	m_vdp->set_palette(m_palette);
	m_vdp->set_tile_callback(FUNC(batrider_state::batrider_bank_cb));
	m_vdp->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_2);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	// these two latches are always written together, via a single move.l instruction
	GENERIC_LATCH_8(config, m_soundlatch[0]);
	GENERIC_LATCH_8(config, m_soundlatch[1]);
	GENERIC_LATCH_8(config, m_soundlatch[2]);
	GENERIC_LATCH_8(config, m_soundlatch[3]);

	YM2151(config, "ymsnd", 32_MHz_XTAL/8).add_route(ALL_OUTPUTS, "mono", 0.25); // 4MHz, 32MHz Oscillator (verified)

	OKIM6295(config, m_oki[0], 32_MHz_XTAL/10, okim6295_device::PIN7_HIGH);
	m_oki[0]->set_addrmap(0, &batrider_state::raizing_oki<0>);
	m_oki[0]->add_route(ALL_OUTPUTS, "mono", 0.5);

	OKIM6295(config, m_oki[1], 32_MHz_XTAL/10, okim6295_device::PIN7_LOW);
	m_oki[1]->set_addrmap(0, &batrider_state::raizing_oki<1>);
	m_oki[1]->add_route(ALL_OUTPUTS, "mono", 0.5);
}


void bbakraid_state::bbakraid(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 32_MHz_XTAL/2);   // 16MHz, 32MHz Oscillator
	m_maincpu->set_addrmap(AS_PROGRAM, &bbakraid_state::bbakraid_68k_mem);
	m_maincpu->reset_cb().set(FUNC(bbakraid_state::reset));

	Z80(config, m_audiocpu, XTAL(32'000'000)/6);     /* 5.3333MHz , 32MHz Oscillator */
	m_audiocpu->set_addrmap(AS_PROGRAM, &bbakraid_state::bbakraid_sound_z80_mem);
	m_audiocpu->set_addrmap(AS_IO, &bbakraid_state::bbakraid_sound_z80_port);
	m_audiocpu->set_periodic_int(FUNC(bbakraid_state::bbakraid_snd_interrupt), attotime::from_hz(XTAL(32'000'000) / 6 / 12000)); // sound CPU clock (divider unverified)

	config.set_maximum_quantum(attotime::from_hz(600));

	EEPROM_93C66_8BIT(config, m_eeprom);

	ADDRESS_MAP_BANK(config, m_dma_space, 0);
	m_dma_space->set_addrmap(0, &bbakraid_state::batrider_dma_mem);
	m_dma_space->set_endianness(ENDIANNESS_BIG);
	m_dma_space->set_data_width(16);
	m_dma_space->set_addr_width(16);
	m_dma_space->set_stride(0x8000);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(bbakraid_state::screen_update_truxton2));
	m_screen->screen_vblank().set(FUNC(bbakraid_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_batrider);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, gp9001vdp_device::VDP_PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp, 27_MHz_XTAL);
	m_vdp->set_palette(m_palette);
	m_vdp->set_tile_callback(FUNC(bbakraid_state::batrider_bank_cb));
	m_vdp->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_1);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	// these two latches are always written together, via a single move.l instruction
	GENERIC_LATCH_8(config, m_soundlatch[0]);
	GENERIC_LATCH_8(config, m_soundlatch[1]);
	GENERIC_LATCH_8(config, m_soundlatch[2]);
	GENERIC_LATCH_8(config, m_soundlatch[3]);

	YMZ280B(config, "ymz", 16.9344_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 1.0);
	// IRQ not used ???  Connected to a test pin (TP082)
}


void nprobowl_state::nprobowl(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 32_MHz_XTAL / 2);   // 32MHz Oscillator, divisor not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &nprobowl_state::nprobowl_68k_mem);
	m_maincpu->reset_cb().set(FUNC(nprobowl_state::reset));

	ADDRESS_MAP_BANK(config, m_dma_space, 0);
	m_dma_space->set_addrmap(0, &nprobowl_state::batrider_dma_mem);
	m_dma_space->set_endianness(ENDIANNESS_BIG);
	m_dma_space->set_data_width(16);
	m_dma_space->set_addr_width(16);
	m_dma_space->set_stride(0x8000);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_UPDATE_BEFORE_VBLANK);
	m_screen->set_raw(27_MHz_XTAL/4, 432, 0, 320, 262, 0, 240);
	m_screen->set_screen_update(FUNC(nprobowl_state::screen_update_truxton2));
	m_screen->screen_vblank().set(FUNC(nprobowl_state::screen_vblank));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_batrider);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, gp9001vdp_device::VDP_PALETTE_LENGTH);

	GP9001_VDP(config, m_vdp, 27_MHz_XTAL);
	m_vdp->set_palette(m_palette);
	m_vdp->vint_out_cb().set_inputline(m_maincpu, M68K_IRQ_2);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki[0], 32_MHz_XTAL/8, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // divisor not verified
	// TODO: banking
}


void raizing_base_state::install_raizing_okibank(int chip)
{
	assert(m_oki_rom[chip] && m_raizing_okibank[chip][0]);

	for (int i = 0; i < 4; i++)
	{
		m_raizing_okibank[chip][i]->configure_entries(0, 16, &m_oki_rom[chip][(i * 0x100)], 0x10000);
	}
	m_raizing_okibank[chip][4]->configure_entries(0, 16, &m_oki_rom[chip][0x400], 0x10000);
	for (int i = 5; i < 8; i++)
	{
		m_raizing_okibank[chip][i]->configure_entries(0, 16, &m_oki_rom[chip][0], 0x10000);
	}
}

void bgaregga_state::init_bgaregga()
{
	u8 *Z80 = memregion("audiocpu")->base();

	m_audiobank->configure_entries(0, 8, Z80, 0x4000); // Test mode only, Mirror of First 128KB Areas?
	m_audiobank->configure_entries(8, 8, Z80, 0x4000);
	install_raizing_okibank(0);
}


void batrider_state::init_batrider()
{
	u8 *Z80 = memregion("audiocpu")->base();

	m_audiobank->configure_entries(0, 16, Z80, 0x4000);
	install_raizing_okibank(0);
	install_raizing_okibank(1);
	m_sndirq_line = 4;
}


void bbakraid_state::init_bbakraid()
{
	m_sndirq_line = 2;
}

/* -------------------------- Raizing games ------------------------- */


/*
For the two sets of Sorcer Striker (World) the only differences
are 2 bytes plus a corrected checksum for each set:

File Offset     sstriker   sstrikera
  0x160            17         0B   <-- Rom checksum value
  0x161            79         6D   <-- Rom checksum value

  0x92C            18         0C   <-- Index of copyright strings to display for Korea
  0x92D            18         0C   <-- Index of copyright strings to display for Korea

0C points to the strings "LICENSED TO UNITE TRADING" / "FOR KOREA".
18 points to a pair of empty strings.

Printed labels for the eproms look like:

RA-MA-01
   01
RAIZING

Both English and Japanese sets use the same labels and numbers for the roms
even if the roms contain different code / data.
*/

ROM_START( sstriker )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "ra-ma_01_01.u65", 0x000000, 0x080000, CRC(708fd51d) SHA1(167186d4cf13af37ec0fa6a59c738c54dbbf3c7c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "ra-ma-01_02.u66", 0x00000, 0x10000, CRC(eabfa46d) SHA1(402c99ebf88f9025f74f0a28ced22b7882a65eb3) )

	ROM_REGION( 0x200000, "gp9001", 0 )
	ROM_LOAD( "ra-ma01-rom2.u2",  0x000000, 0x100000, CRC(54e2bd95) SHA1(341359dd46152615675bb90e8a184216c8feebff) )
	ROM_LOAD( "ra-ma01-rom3.u1",  0x100000, 0x100000, CRC(21cd378f) SHA1(e1695bccec949d18b1c03e9c42dca384554b0d7c) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "ra-ma-01_05.u81",  0x000000, 0x008000, CRC(88b58841) SHA1(1d16b538c11a291bd1f46a510bfbd6259b45a0b5) )

	ROM_REGION( 0x40000, "oki1", 0 )         /* ADPCM Samples */
	ROM_LOAD( "ra-ma01-rom1.u57", 0x00000, 0x40000, CRC(6edb2ab8) SHA1(e3032e8eda2686f30df4b7a088c5a4d4d45782ed) )
ROM_END


ROM_START( sstrikerk )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "ra-ma-01_01.u65", 0x000000, 0x080000, CRC(92259f84) SHA1(127e62e407d95efd360bfe2cac9577f326abf6ef) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "ra-ma-01_02.u66", 0x00000, 0x10000, CRC(eabfa46d) SHA1(402c99ebf88f9025f74f0a28ced22b7882a65eb3) )

	ROM_REGION( 0x200000, "gp9001", 0 )
	ROM_LOAD( "ra-ma01-rom2.u2",  0x000000, 0x100000, CRC(54e2bd95) SHA1(341359dd46152615675bb90e8a184216c8feebff) )
	ROM_LOAD( "ra-ma01-rom3.u1",  0x100000, 0x100000, CRC(21cd378f) SHA1(e1695bccec949d18b1c03e9c42dca384554b0d7c) )
	// also seen with 4 smaller ROMs instead of 2
	// 01.bin                  ra-ma01-rom2.u2 [even]     IDENTICAL
	// 02.bin                  ra-ma01-rom2.u2 [odd]      IDENTICAL
	// 03.bin                  ra-ma01-rom3.u1 [even]     IDENTICAL
	// 04.bin                  ra-ma01-rom3.u1 [odd]      IDENTICAL

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "ra-ma-01_05.u81",  0x000000, 0x008000, CRC(88b58841) SHA1(1d16b538c11a291bd1f46a510bfbd6259b45a0b5) )

	ROM_REGION( 0x40000, "oki1", 0 )         /* ADPCM Samples */
	ROM_LOAD( "ra-ma01-rom1.u57", 0x00000, 0x40000, CRC(6edb2ab8) SHA1(e3032e8eda2686f30df4b7a088c5a4d4d45782ed) )
ROM_END


ROM_START( mahoudai )
	ROM_REGION( 0x080000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "ra_ma_01_01.u65", 0x000000, 0x080000, CRC(970ccc5c) SHA1(c87cab83bde0284e631f02e50068407fee81d941) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "ra-ma-01_02.u66", 0x00000, 0x10000, CRC(eabfa46d) SHA1(402c99ebf88f9025f74f0a28ced22b7882a65eb3) )

	ROM_REGION( 0x200000, "gp9001", 0 )
	ROM_LOAD( "ra-ma01-rom2.u2",  0x000000, 0x100000, CRC(54e2bd95) SHA1(341359dd46152615675bb90e8a184216c8feebff) )
	ROM_LOAD( "ra-ma01-rom3.u1",  0x100000, 0x100000, CRC(21cd378f) SHA1(e1695bccec949d18b1c03e9c42dca384554b0d7c) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "ra_ma_01_05.u81",  0x000000, 0x008000, CRC(c00d1e80) SHA1(53e64c4c0c6309130b37597d13b44a9e95b717d8) )

	ROM_REGION( 0x40000, "oki1", 0 )         /* ADPCM Samples */
	ROM_LOAD( "ra-ma01-rom1.u57", 0x00000, 0x40000, CRC(6edb2ab8) SHA1(e3032e8eda2686f30df4b7a088c5a4d4d45782ed) )
ROM_END


ROM_START( kingdmgp )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "ma02rom1.bin", 0x000000, 0x080000, CRC(a678b149) SHA1(8c1a631e023dbba0a3fa6cd1b7d10dec1663213a) )
	ROM_LOAD16_BYTE( "ma02rom0.bin", 0x000001, 0x080000, CRC(f226a212) SHA1(526acf3d05fdc88054a772fbea3de2af532bf3d2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "ma02rom2.bin", 0x00000, 0x10000, CRC(dde8a57e) SHA1(f522a3f17e229c71512464349760a9e27778bf6a) )

	ROM_REGION( 0x400000, "gp9001", 0 )
	ROM_LOAD( "ma02rom3.bin",  0x000000, 0x200000, CRC(0e797142) SHA1(a480ccd151e49b886d3175a6deff56e1f2c26c3e) )
	ROM_LOAD( "ma02rom4.bin",  0x200000, 0x200000, CRC(72a6fa53) SHA1(ce92e65205b84361cfb90305a61e9541b5c4dc2f) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "ma02rom5.eng",  0x000000, 0x008000, CRC(8c28460b) SHA1(0aed170762f6044896a7e608df60bbd37c583a71) )

	ROM_REGION( 0x80000, "oki1", 0 )         /* ADPCM Samples */
	ROM_LOAD( "ma02rom6.bin", 0x00000, 0x80000, CRC(199e7cae) SHA1(0f5e13cc8ec42c80bb4bbff90aba29cdb15213d4) )
ROM_END


ROM_START( shippumd )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "ma02rom1.bin", 0x000000, 0x080000, CRC(a678b149) SHA1(8c1a631e023dbba0a3fa6cd1b7d10dec1663213a) )
	ROM_LOAD16_BYTE( "ma02rom0.bin", 0x000001, 0x080000, CRC(f226a212) SHA1(526acf3d05fdc88054a772fbea3de2af532bf3d2) )

	ROM_REGION( 0x10000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "ma02rom2.bin", 0x00000, 0x10000, CRC(dde8a57e) SHA1(f522a3f17e229c71512464349760a9e27778bf6a) )

	ROM_REGION( 0x400000, "gp9001", 0 )
	ROM_LOAD( "ma02rom3.bin",  0x000000, 0x200000, CRC(0e797142) SHA1(a480ccd151e49b886d3175a6deff56e1f2c26c3e) )
	ROM_LOAD( "ma02rom4.bin",  0x200000, 0x200000, CRC(72a6fa53) SHA1(ce92e65205b84361cfb90305a61e9541b5c4dc2f) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "ma02rom5.bin",  0x000000, 0x008000, CRC(116ae559) SHA1(4cc2d2a23cc0aefd457111b7990e47184e79204c) )

	ROM_REGION( 0x80000, "oki1", 0 )         /* ADPCM Samples */
	ROM_LOAD( "ma02rom6.bin", 0x00000, 0x80000, CRC(199e7cae) SHA1(0f5e13cc8ec42c80bb4bbff90aba29cdb15213d4) )
ROM_END

ROM_START( bgareggat )
	/* Dumped from a location test board, with some minor changes compared to the final.
	* All ROMs are socketed
	* All PAL/GALs are socketed
	* PLDs at U33, U125 are PALCE16V8H/4 instead of GAL16V8B as usual (no functional impact)
	* JP4 features four DIP switches, instead of two DIPs + two jumpers as in the final.

	The date codes are written referencing Heisei year 8 (1996).

	The program ROMs feature hand-written labels formatted like this:
	BATTLE GAREGGA
	     PRG 0
	    8.1.17.
	*/
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "battlegaregga-prg0-8-1-17.bin", 0x000000, 0x080000, CRC(c032176f) SHA1(799ba0424489361dd2f814afaf841326bc23300c) )
	ROM_LOAD16_BYTE( "battlegaregga-prg1-8-1-17.bin", 0x000001, 0x080000, CRC(3822f375) SHA1(a5a84cf48c86d8ac97f401280667658d7f451896) )

	/* Hand-written label that reads
	BATTLE GAREGGA
	      SND
	8.1.18. ロケVer.
	*/
	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "battlegaregga-snd-8-1-18-loke-ver.bin", 0x00000, 0x20000, CRC(f5ea56f7) SHA1(9db04069b378dbad6626fd29d3762e3361b9aa0d) )

	/* Stored on NEC ES23C16000W Mask ROMs with no Raizing/8ing custom markings.*/
	ROM_REGION( 0x800000, "gp9001", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	/* Hand-written label that reads
	BATTLE GAREGGA
	     TEXT
	8.1.17. 8AA6

	8AA6 is the checksum of the text ROM, which matches release.
	*/
	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	/* Stored on an NEC ES23C8001EJ Mask ROM with no Raizing/8ing custom markings.*/
	ROM_REGION( 0x100000, "oki1", 0 )        /* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END

ROM_START( bgaregga )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "prg0.bin", 0x000000, 0x080000, CRC(f80c2fc2) SHA1(a9aac5c7f5439b6fe8d1b3db1fb02a27cc28fdf6) )
	ROM_LOAD16_BYTE( "prg1.bin", 0x000001, 0x080000, CRC(2ccfdd1e) SHA1(7a9f11f851854f3f8389b9c3c0906ebb8dc28712) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x100000, "oki1", 0 )        /* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END


ROM_START( bgareggahk )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "prg_0.rom", 0x000000, 0x080000, CRC(26e0019e) SHA1(5197001f5d59246b137e19ed1952a8207b25d4c0) )
	ROM_LOAD16_BYTE( "prg_1.rom", 0x000001, 0x080000, CRC(2ccfdd1e) SHA1(7a9f11f851854f3f8389b9c3c0906ebb8dc28712) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x100000, "oki1", 0 )        /* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END


ROM_START( bgareggatw )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "garegga_prg0.u123", 0x000000, 0x080000, CRC(235b7405) SHA1(a2434801df4231a6b48f6c63f47c202d25a89e79) )
	ROM_LOAD16_BYTE( "garegga_prg1.u65",  0x000001, 0x080000, CRC(c29ccf6a) SHA1(38806e0b4ff852f4bfefd80c56ca23f71623e275) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x100000, "oki1", 0 )        /* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END


// only the program ROMs' dumps were provided for this set.
// According to the dumper: 'In the Korea Region Setting, DIP SWITCH's 'STAGE EDIT' does not work and
// the C button (formation change) function in the in-game is also deleted.'
ROM_START( bgareggak )
	ROM_REGION( 0x100000, "maincpu", 0 )            // Main 68K code
	ROM_LOAD16_BYTE( "prg0.bin", 0x000000, 0x080000, CRC(40a108a7) SHA1(cc3227dc87ffefb961dbcdff146e787dbfbdfc2c) )
	ROM_LOAD16_BYTE( "prg1.bin", 0x000001, 0x080000, CRC(45a6e48a) SHA1(f4d4158b8556b4261291ba9905b9731623b47e54) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            // Sound Z80 code + bank
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, BAD_DUMP CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, BAD_DUMP CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, BAD_DUMP CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, BAD_DUMP CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, BAD_DUMP CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, BAD_DUMP CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x100000, "oki1", 0 )        // ADPCM Samples
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, BAD_DUMP CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END


ROM_START( bgaregganv )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "prg_0.bin", 0x000000, 0x080000, CRC(951ecc07) SHA1(a82e4b59e4a974566e59f3ab2fbae1aec7d88a2b) )
	ROM_LOAD16_BYTE( "prg_1.bin", 0x000001, 0x080000, CRC(729a60c6) SHA1(cb6f5d138bb82c32910f42d8ee16fa573a23cef3) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x100000, "oki1", 0 )        /* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END


ROM_START( bgareggat2 )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "prg0", 0x000000, 0x080000, CRC(84094099) SHA1(49fc68a8bcdae4477e20eade9dd569de88b0b798) )
	ROM_LOAD16_BYTE( "prg1", 0x000001, 0x080000, CRC(46f92fe4) SHA1(62a02cc1dbdc3ac362339aebb62368eb89b06bad) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x100000, "oki1", 0 )        /* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END


ROM_START( bgareggacn )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_BYTE( "u123", 0x000000, 0x080000, CRC(88a4e66a) SHA1(ca97e564eed0c5e028b937312e55da56400d5c8c) )
	ROM_LOAD16_BYTE( "u65",  0x000001, 0x080000, CRC(5dea32a3) SHA1(59df6689e3eb5ea9e49a758604d21a64c65ca14d) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.u81", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x100000, "oki1", 0 )        /* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END

// 1945二代 (1945 Èr Dài)
ROM_START( bgareggabl )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "xt-8m.bin", 0x000000, 0x100000, CRC(4a6657cb) SHA1(1838956e7597eaa78ea5ee58d0ccc79cbbd7ded5) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001", 0 )
	ROM_LOAD( "6@-322",  0x000000, 0x400000, CRC(37fe48ed) SHA1(ded5d13c33b4582310cdfb3dd52c052f741c00c5) ) /* == rom4.bin+rom3.bin */
	ROM_LOAD( "5@-322",  0x400000, 0x400000, CRC(5a06c031) SHA1(ee241ff90117cec1f33ab163601a9d5c75609739) ) /* == rom2.bin+rom1.bin */

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "1@-256", 0x00000, 0x08000, CRC(760dcd14) SHA1(e151e5d7ca5557277f306b9484ec021f4edf1e07) )

	ROM_REGION( 0x010000, "user1", 0 ) // not graphics
	ROM_LOAD( "2@-256", 0x00000, 0x08000, CRC(456dd16e) SHA1(84779ee64d3ea33ba1ba4dee39b504a81c6811a1) )

	ROM_REGION( 0x100000, "oki1", 0 )        /* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END

// 雷神传 (Léishén Chuán)
ROM_START( bgareggabla )
	ROM_REGION( 0x100000, "maincpu", 0 )            /* Main 68K code */
	ROM_LOAD16_WORD_SWAP( "27c8100.mon-sys", 0x000000, 0x100000, CRC(d334e5aa) SHA1(41607b5630d7b92a96607ea95c5b55ad43745857) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001", 0 )
	ROM_LOAD( "rom4.bin",  0x000000, 0x200000, CRC(b333d81f) SHA1(5481465f1304334fd55798be2f44324c57c2dbcb) )
	ROM_LOAD( "rom3.bin",  0x200000, 0x200000, CRC(51b9ebfb) SHA1(30e0c326f5175aa436df8dba08f6f4e08130b92f) )
	ROM_LOAD( "rom2.bin",  0x400000, 0x200000, CRC(b330e5e2) SHA1(5d48e9d56f99d093b6390e0af1609fd796df2d35) )
	ROM_LOAD( "rom1.bin",  0x600000, 0x200000, CRC(7eafdd70) SHA1(7c8da8e86c3f9491719b1d7d5d285568d7614f38) )

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.bin", 0x00000, 0x08000, CRC(00d100bd) SHA1(fb6028e3519d6588a966d1b16d47453db2e51fd7))

	ROM_REGION( 0x010000, "user1", 0 ) // not graphics
	ROM_LOAD( "base.bin", 0x00000, 0x08000, CRC(456dd16e) SHA1(84779ee64d3ea33ba1ba4dee39b504a81c6811a1) )

	ROM_REGION( 0x100000, "oki1", 0 )        /* ADPCM Samples */
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END

ROM_START( bgareggablj ) // fixed on Japanese region
	ROM_REGION( 0x100000, "maincpu", 0 )            // Main 68K code
	ROM_LOAD16_WORD_SWAP( "sys.bin", 0x000000, 0x100000, CRC(b2a1225f) SHA1(dda00aa5e7ce3f39bb0eebbcd9500ef22a5d74d3) ) // 1ST AND 2ND HALF IDENTICAL
	ROM_IGNORE(                                0x100000 )

	ROM_REGION( 0x20000, "audiocpu", 0 )            // Sound Z80 code + bank
	ROM_LOAD( "snd.bin", 0x00000, 0x20000, CRC(68632952) SHA1(fb834db83157948e2b420b6051102a9c6ac3969b) )

	ROM_REGION( 0x800000, "gp9001", 0 )
	ROM_LOAD( "322_2.bin",  0x000000, 0x400000, CRC(37fe48ed) SHA1(ded5d13c33b4582310cdfb3dd52c052f741c00c5) ) // == rom4.bin+rom3.bin
	ROM_LOAD( "322_1.bin",  0x400000, 0x400000, CRC(5a06c031) SHA1(ee241ff90117cec1f33ab163601a9d5c75609739) ) // == rom2.bin+rom1.bin

	ROM_REGION( 0x008000, "text", 0 )
	ROM_LOAD( "text.bin", 0x00000, 0x08000, CRC(e67fd534) SHA1(987d0edffc2c243a13d4567319ea3d185eaadbf8) )

	ROM_REGION( 0x010000, "user1", 0 ) // not graphics
	ROM_LOAD( "base.bin", 0x00000, 0x08000, CRC(456dd16e) SHA1(84779ee64d3ea33ba1ba4dee39b504a81c6811a1) )

	ROM_REGION( 0x100000, "oki1", 0 )        // ADPCM Samples
	ROM_LOAD( "rom5.bin", 0x000000, 0x100000, CRC(f6d49863) SHA1(3a3c354852adad06e8a051511abfab7606bce382) )
ROM_END

/*
   The region of Batrider is controlled by the first byte of rom prg0.u22
   only sets which have been dumped from original PCBs are supported

   original ROM labels have no indication of the region.

   valid values are:
    ( * denotes that this set has been found on an original PCB )

   00 : Nippon *
   01 : USA *
   02 : Europe *
   03 : Asia
   04 : German (sic)
   05 : Austria
   06 : Belgium
   07 : Denmark
   08 : Finland
   09 : France
   0A : Great Britain
   0B : Greece
   0C : The Netherlands
   0D : Italy
   0E : Norway
   0F : Portugal
   10 : Spain
   11 : Sweden
   12 : Switzerland
   13 : Australia
   14 : New Zealand
   15 : Taiwan
   16 : Hong Kong
   17 : Korea *
   18 : China *
   19 : No Region?
   1A+: Invalid

   For future reference, that would mean the following

   ROM_LOAD16_BYTE( "prg0_nippon.u22",       0x000000, 0x080000, CRC(4f3fc729) SHA1(b32d51c254741b82171a86c271679522a7aefd34) )
   ROM_LOAD16_BYTE( "prg0_usa.u22",          0x000000, 0x080000, CRC(2049d007) SHA1(f2a43547a6fc5083b03c1d59a85abbf6e1ce4cd9) )
   ROM_LOAD16_BYTE( "prg0_europe.u22",       0x000000, 0x080000, CRC(91d3e975) SHA1(682885fc17f2424d475c282f239f42faf1aae076) )
   ROM_LOAD16_BYTE( "prg0_asia.u22",         0x000000, 0x080000, CRC(fea5fe5b) SHA1(0008336ecd3886485ab1d9678880b1a0bc788f40) )
   ROM_LOAD16_BYTE( "prg0_german.u22",       0x000000, 0x080000, CRC(29969dd0) SHA1(eb8ad84b772508b6befb35afb11a0d6193c6a060) )
   ROM_LOAD16_BYTE( "prg0_austria.u22",      0x000000, 0x080000, CRC(46e08afe) SHA1(a6f46581d0f7285704fbf1ac57476c96f4dcbec2) )
   ROM_LOAD16_BYTE( "prg0_belgium.u22",      0x000000, 0x080000, CRC(f77ab38c) SHA1(8be87175250345d3e31d95ec204805071eae81f6) )
   ROM_LOAD16_BYTE( "prg0_denmark.u22",      0x000000, 0x080000, CRC(980ca4a2) SHA1(4f29eaa5ba6b94d96c527f80188657abc8f4dcd0) )
   ROM_LOAD16_BYTE( "prg0_finland.u22",      0x000000, 0x080000, CRC(826d72db) SHA1(be4bca0143f43c13361fd56974eb9b1ce7bd1740) )
   ROM_LOAD16_BYTE( "prg0_france.u22",       0x000000, 0x080000, CRC(ed1b65f5) SHA1(1e08957c0f7ed65695fb1ceb961ab765f8a97c89) )
   ROM_LOAD16_BYTE( "prg0_greatbritain.u22", 0x000000, 0x080000, CRC(5c815c87) SHA1(dea89944cd9a3fa6991b214495dc7123a505d39b) )
   ROM_LOAD16_BYTE( "prg0_greece.u22",       0x000000, 0x080000, CRC(33f74ba9) SHA1(fe770415584b037152b37a75fe468d3c52dcb3cd) )
   ROM_LOAD16_BYTE( "prg0_netherlands.u22",  0x000000, 0x080000, CRC(e4c42822) SHA1(8bfd286c42d7f2b3c88757b9a8b818be90b73f48) )
   ROM_LOAD16_BYTE( "prg0_italy.u22",        0x000000, 0x080000, CRC(8bb23f0c) SHA1(b448bba312a8d583a981f6633cbc14af99fdbb06) )
   ROM_LOAD16_BYTE( "prg0_norway.u22",       0x000000, 0x080000, CRC(3a28067e) SHA1(9435e6ce90b8d740a545469e6edb35d1af11ceab) )
   ROM_LOAD16_BYTE( "prg0_portugal.u22",     0x000000, 0x080000, CRC(555e1150) SHA1(5c9ae898244a23a4184f9613f42d9aa9530468b9) )
   ROM_LOAD16_BYTE( "prg0_spain.u22",        0x000000, 0x080000, CRC(0eebaa8c) SHA1(e305e90434e7f322a33e42a642362f770d3eb0e5) )
   ROM_LOAD16_BYTE( "prg0_sweden.u22",       0x000000, 0x080000, CRC(619dbda2) SHA1(9e88ba104a5cffcced3b93ca711487a82b0fddde) )
   ROM_LOAD16_BYTE( "prg0_switzerland.u22",  0x000000, 0x080000, CRC(d00784d0) SHA1(0b809414ce910684ca39216086f7d26fd2adeded) )
   ROM_LOAD16_BYTE( "prg0_australia.u22",    0x000000, 0x080000, CRC(bf7193fe) SHA1(9af50fffc6ef23e300bf7b5e90b0dee6e4f4ad05) )
   ROM_LOAD16_BYTE( "prg0_newzealand.u22",   0x000000, 0x080000, CRC(6842f075) SHA1(125b303c064d2f0b539ecadcb205756e7fd1334e) )
   ROM_LOAD16_BYTE( "prg0_taiwan.u22",       0x000000, 0x080000, CRC(0734e75b) SHA1(17a8fb4f8fda3c234ed976490193ba308cac08fe) )
   ROM_LOAD16_BYTE( "prg0_hongkong.u22",     0x000000, 0x080000, CRC(b6aede29) SHA1(580f29db6a2c2cea43966413778362694992a675) )
   ROM_LOAD16_BYTE( "prg0_korea.u22",        0x000000, 0x080000, CRC(d9d8c907) SHA1(69c197f2a41f288913f042de9eb8274c0df3ac27) )
   ROM_LOAD16_BYTE( "prg0_china.u22",        0x000000, 0x080000, CRC(c3b91f7e) SHA1(6b2376c37808dccda296d90ccd7f577ccff4e4dc) )
   ROM_LOAD16_BYTE( "prg0_none.u22",         0x000000, 0x080000, CRC(accf0850) SHA1(d93e4e80443a40c3a9575dbf21927ef0d1a039b9) )
 */


ROM_START( batrider )
	ROM_REGION( 0x200000, "maincpu", 0 )            /* Main 68k code */
	ROM_LOAD16_BYTE( "prg0_europe.u22", 0x000000, 0x080000, CRC(91d3e975) SHA1(682885fc17f2424d475c282f239f42faf1aae076) )
	ROM_LOAD16_BYTE( "prg1b.u23", 0x000001, 0x080000, CRC(8e70b492) SHA1(f84f2039826ae815afb058d71c1dbd190f9d524d) )
	ROM_LOAD16_BYTE( "prg2.u21" , 0x100000, 0x080000, CRC(bdaa5fbf) SHA1(abd72ac633c0c8e7b4b1d7902c0d6e014ba995fe) )
	ROM_LOAD16_BYTE( "prg3.u24" , 0x100001, 0x080000, CRC(7aa9f941) SHA1(99bdbad7a96d461073b06a53c50fc57c2fd6fc6d) )

	ROM_REGION( 0x40000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.u77", 0x00000, 0x40000, CRC(56682696) SHA1(a372450d9a6d535123dfc31d8116074b168ab646) )

	ROM_REGION( 0x1000000, "gp9001", 0 )
	ROM_LOAD( "rom-1.bin", 0x000000, 0x400000, CRC(0df69ca2) SHA1(49670347ebd7e1067ff988cf842b275b7ee7b5f7) )
	ROM_LOAD( "rom-3.bin", 0x400000, 0x400000, CRC(60167d38) SHA1(fd2429808c59ef51fd5f5db84ea89a8dc504186e) )
	ROM_LOAD( "rom-2.bin", 0x800000, 0x400000, CRC(1bfea593) SHA1(ce06dc3097ae56b0df56d104bbf7efc9b5d968d4) )
	ROM_LOAD( "rom-4.bin", 0xc00000, 0x400000, CRC(bee03c94) SHA1(5bc1e6769c42857c03456426b502fcb86a114f19) )

	ROM_REGION( 0x100000, "oki1", 0 )       /* ADPCM Samples 1 */
	ROM_LOAD( "rom-5.bin", 0x000000, 0x100000, CRC(4274daf6) SHA1(85557b4707d529e5914f03c7a856864f5c24950e) )

	ROM_REGION( 0x100000, "oki2", 0 )       /* ADPCM Samples 2 */
	ROM_LOAD( "rom-6.bin", 0x000000, 0x100000, CRC(2a1c2426) SHA1(8abc3688ffc5ebb94b8d5118d4fa0908f07fe791) )
ROM_END


ROM_START( batrideru )
	ROM_REGION( 0x200000, "maincpu", 0 )            /* Main 68k code */
	ROM_LOAD16_BYTE( "prg0_usa.u22", 0x000000, 0x080000, CRC(2049d007) SHA1(f2a43547a6fc5083b03c1d59a85abbf6e1ce4cd9) )
	ROM_LOAD16_BYTE( "prg1b.u23", 0x000001, 0x080000, CRC(8e70b492) SHA1(f84f2039826ae815afb058d71c1dbd190f9d524d) )
	ROM_LOAD16_BYTE( "prg2.u21" , 0x100000, 0x080000, CRC(bdaa5fbf) SHA1(abd72ac633c0c8e7b4b1d7902c0d6e014ba995fe) )
	ROM_LOAD16_BYTE( "prg3.u24" , 0x100001, 0x080000, CRC(7aa9f941) SHA1(99bdbad7a96d461073b06a53c50fc57c2fd6fc6d) )

	ROM_REGION( 0x40000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.u77", 0x00000, 0x40000, CRC(56682696) SHA1(a372450d9a6d535123dfc31d8116074b168ab646) )

	ROM_REGION( 0x1000000, "gp9001", 0 )
	ROM_LOAD( "rom-1.bin", 0x000000, 0x400000, CRC(0df69ca2) SHA1(49670347ebd7e1067ff988cf842b275b7ee7b5f7) )
	ROM_LOAD( "rom-3.bin", 0x400000, 0x400000, CRC(60167d38) SHA1(fd2429808c59ef51fd5f5db84ea89a8dc504186e) )
	ROM_LOAD( "rom-2.bin", 0x800000, 0x400000, CRC(1bfea593) SHA1(ce06dc3097ae56b0df56d104bbf7efc9b5d968d4) )
	ROM_LOAD( "rom-4.bin", 0xc00000, 0x400000, CRC(bee03c94) SHA1(5bc1e6769c42857c03456426b502fcb86a114f19) )

	ROM_REGION( 0x100000, "oki1", 0 )       /* ADPCM Samples 1 */
	ROM_LOAD( "rom-5.bin", 0x000000, 0x100000, CRC(4274daf6) SHA1(85557b4707d529e5914f03c7a856864f5c24950e) )

	ROM_REGION( 0x100000, "oki2", 0 )       /* ADPCM Samples 2 */
	ROM_LOAD( "rom-6.bin", 0x000000, 0x100000, CRC(2a1c2426) SHA1(8abc3688ffc5ebb94b8d5118d4fa0908f07fe791) )
ROM_END


ROM_START( batriderc )
	ROM_REGION( 0x200000, "maincpu", 0 )            /* Main 68k code */
	ROM_LOAD16_BYTE( "prg0_china.u22", 0x000000, 0x080000, CRC(c3b91f7e) SHA1(6b2376c37808dccda296d90ccd7f577ccff4e4dc) )
	ROM_LOAD16_BYTE( "prg1b.u23", 0x000001, 0x080000, CRC(8e70b492) SHA1(f84f2039826ae815afb058d71c1dbd190f9d524d) )
	ROM_LOAD16_BYTE( "prg2.u21" , 0x100000, 0x080000, CRC(bdaa5fbf) SHA1(abd72ac633c0c8e7b4b1d7902c0d6e014ba995fe) )
	ROM_LOAD16_BYTE( "prg3.u24" , 0x100001, 0x080000, CRC(7aa9f941) SHA1(99bdbad7a96d461073b06a53c50fc57c2fd6fc6d) )

	ROM_REGION( 0x40000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.u77", 0x00000, 0x40000, CRC(56682696) SHA1(a372450d9a6d535123dfc31d8116074b168ab646) )

	ROM_REGION( 0x1000000, "gp9001", 0 )
	ROM_LOAD( "rom-1.bin", 0x000000, 0x400000, CRC(0df69ca2) SHA1(49670347ebd7e1067ff988cf842b275b7ee7b5f7) )
	ROM_LOAD( "rom-3.bin", 0x400000, 0x400000, CRC(60167d38) SHA1(fd2429808c59ef51fd5f5db84ea89a8dc504186e) )
	ROM_LOAD( "rom-2.bin", 0x800000, 0x400000, CRC(1bfea593) SHA1(ce06dc3097ae56b0df56d104bbf7efc9b5d968d4) )
	ROM_LOAD( "rom-4.bin", 0xc00000, 0x400000, CRC(bee03c94) SHA1(5bc1e6769c42857c03456426b502fcb86a114f19) )

	ROM_REGION( 0x100000, "oki1", 0 )       /* ADPCM Samples 1 */
	ROM_LOAD( "rom-5.bin", 0x000000, 0x100000, CRC(4274daf6) SHA1(85557b4707d529e5914f03c7a856864f5c24950e) )

	ROM_REGION( 0x100000, "oki2", 0 )       /* ADPCM Samples 2 */
	ROM_LOAD( "rom-6.bin", 0x000000, 0x100000, CRC(2a1c2426) SHA1(8abc3688ffc5ebb94b8d5118d4fa0908f07fe791) )
ROM_END


ROM_START( batriderj )
	ROM_REGION( 0x200000, "maincpu", 0 )            /* Main 68k code */
	ROM_LOAD16_BYTE( "prg0b.u22", 0x000000, 0x080000, CRC(4f3fc729) SHA1(b32d51c254741b82171a86c271679522a7aefd34) )
	ROM_LOAD16_BYTE( "prg1b.u23", 0x000001, 0x080000, CRC(8e70b492) SHA1(f84f2039826ae815afb058d71c1dbd190f9d524d) )
	ROM_LOAD16_BYTE( "prg2.u21" , 0x100000, 0x080000, CRC(bdaa5fbf) SHA1(abd72ac633c0c8e7b4b1d7902c0d6e014ba995fe) )
	ROM_LOAD16_BYTE( "prg3.u24" , 0x100001, 0x080000, CRC(7aa9f941) SHA1(99bdbad7a96d461073b06a53c50fc57c2fd6fc6d) )

	ROM_REGION( 0x40000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.u77", 0x00000, 0x40000, CRC(56682696) SHA1(a372450d9a6d535123dfc31d8116074b168ab646) )

	ROM_REGION( 0x1000000, "gp9001", 0 )
	ROM_LOAD( "rom-1.bin", 0x000000, 0x400000, CRC(0df69ca2) SHA1(49670347ebd7e1067ff988cf842b275b7ee7b5f7) )
	ROM_LOAD( "rom-3.bin", 0x400000, 0x400000, CRC(60167d38) SHA1(fd2429808c59ef51fd5f5db84ea89a8dc504186e) )
	ROM_LOAD( "rom-2.bin", 0x800000, 0x400000, CRC(1bfea593) SHA1(ce06dc3097ae56b0df56d104bbf7efc9b5d968d4) )
	ROM_LOAD( "rom-4.bin", 0xc00000, 0x400000, CRC(bee03c94) SHA1(5bc1e6769c42857c03456426b502fcb86a114f19) )

	ROM_REGION( 0x100000, "oki1", 0 )       /* ADPCM Samples 1 */
	ROM_LOAD( "rom-5.bin", 0x000000, 0x100000, CRC(4274daf6) SHA1(85557b4707d529e5914f03c7a856864f5c24950e) )

	ROM_REGION( 0x100000, "oki2", 0 )       /* ADPCM Samples 2 */
	ROM_LOAD( "rom-6.bin", 0x000000, 0x100000, CRC(2a1c2426) SHA1(8abc3688ffc5ebb94b8d5118d4fa0908f07fe791) )
ROM_END


ROM_START( batriderk )
	ROM_REGION( 0x200000, "maincpu", 0 )            /* Main 68k code */
	ROM_LOAD16_BYTE( "prg0_korea.u22", 0x000000, 0x080000, CRC(d9d8c907) SHA1(69c197f2a41f288913f042de9eb8274c0df3ac27) )
	ROM_LOAD16_BYTE( "prg1b.u23", 0x000001, 0x080000, CRC(8e70b492) SHA1(f84f2039826ae815afb058d71c1dbd190f9d524d) )
	ROM_LOAD16_BYTE( "prg2.u21" , 0x100000, 0x080000, CRC(bdaa5fbf) SHA1(abd72ac633c0c8e7b4b1d7902c0d6e014ba995fe) )
	ROM_LOAD16_BYTE( "prg3.u24" , 0x100001, 0x080000, CRC(7aa9f941) SHA1(99bdbad7a96d461073b06a53c50fc57c2fd6fc6d) )

	ROM_REGION( 0x40000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.u77", 0x00000, 0x40000, CRC(56682696) SHA1(a372450d9a6d535123dfc31d8116074b168ab646) )

	ROM_REGION( 0x1000000, "gp9001", 0 )
	ROM_LOAD( "rom-1.bin", 0x000000, 0x400000, CRC(0df69ca2) SHA1(49670347ebd7e1067ff988cf842b275b7ee7b5f7) )
	ROM_LOAD( "rom-3.bin", 0x400000, 0x400000, CRC(60167d38) SHA1(fd2429808c59ef51fd5f5db84ea89a8dc504186e) )
	ROM_LOAD( "rom-2.bin", 0x800000, 0x400000, CRC(1bfea593) SHA1(ce06dc3097ae56b0df56d104bbf7efc9b5d968d4) )
	ROM_LOAD( "rom-4.bin", 0xc00000, 0x400000, CRC(bee03c94) SHA1(5bc1e6769c42857c03456426b502fcb86a114f19) )

	ROM_REGION( 0x100000, "oki1", 0 )       /* ADPCM Samples 1 */
	ROM_LOAD( "rom-5.bin", 0x000000, 0x100000, CRC(4274daf6) SHA1(85557b4707d529e5914f03c7a856864f5c24950e) )

	ROM_REGION( 0x100000, "oki2", 0 )       /* ADPCM Samples 2 */
	ROM_LOAD( "rom-6.bin", 0x000000, 0x100000, CRC(2a1c2426) SHA1(8abc3688ffc5ebb94b8d5118d4fa0908f07fe791) )
ROM_END

/* older version, might have only been released in Japan, Hong Kong and Taiwan? */
ROM_START( batriderja )
	ROM_REGION( 0x200000, "maincpu", 0 )            /* Main 68k code */
	ROM_LOAD16_BYTE( "prg0.bin", 0x000000, 0x080000, CRC(f93ea27c) SHA1(41023c2ee1efd70b5aa9c70e1ddd9e5c3d51d68a) )
	ROM_LOAD16_BYTE( "prg1.u23", 0x000001, 0x080000, CRC(8ae7f592) SHA1(8a20ebf85eca621f578d2302c3a3988647b077a7) )
	ROM_LOAD16_BYTE( "prg2.u21", 0x100000, 0x080000, CRC(bdaa5fbf) SHA1(abd72ac633c0c8e7b4b1d7902c0d6e014ba995fe) )
	ROM_LOAD16_BYTE( "prg3.u24", 0x100001, 0x080000, CRC(7aa9f941) SHA1(99bdbad7a96d461073b06a53c50fc57c2fd6fc6d) )

	ROM_REGION( 0x40000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.u77", 0x00000, 0x40000, CRC(56682696) SHA1(a372450d9a6d535123dfc31d8116074b168ab646) )

	ROM_REGION( 0x1000000, "gp9001", 0 )
	ROM_LOAD( "rom-1.bin", 0x000000, 0x400000, CRC(0df69ca2) SHA1(49670347ebd7e1067ff988cf842b275b7ee7b5f7) )
	ROM_LOAD( "rom-3.bin", 0x400000, 0x400000, CRC(60167d38) SHA1(fd2429808c59ef51fd5f5db84ea89a8dc504186e) )
	ROM_LOAD( "rom-2.bin", 0x800000, 0x400000, CRC(1bfea593) SHA1(ce06dc3097ae56b0df56d104bbf7efc9b5d968d4) )
	ROM_LOAD( "rom-4.bin", 0xc00000, 0x400000, CRC(bee03c94) SHA1(5bc1e6769c42857c03456426b502fcb86a114f19) )

	ROM_REGION( 0x100000, "oki1", 0 )       /* ADPCM Samples 1 */
	ROM_LOAD( "rom-5.bin", 0x000000, 0x100000, CRC(4274daf6) SHA1(85557b4707d529e5914f03c7a856864f5c24950e) )

	ROM_REGION( 0x100000, "oki2", 0 )       /* ADPCM Samples 2 */
	ROM_LOAD( "rom-6.bin", 0x000000, 0x100000, CRC(2a1c2426) SHA1(8abc3688ffc5ebb94b8d5118d4fa0908f07fe791) )
ROM_END


ROM_START( batriderhk )
	ROM_REGION( 0x200000, "maincpu", 0 )            /* Main 68k code */
	ROM_LOAD16_BYTE( "prg0.u22", 0x000000, 0x080000, CRC(00afbb7c) SHA1(a4b6331e0fcab7d0c43fc43adb701f1248247b41) )
	ROM_LOAD16_BYTE( "prg1.u23", 0x000001, 0x080000, CRC(8ae7f592) SHA1(8a20ebf85eca621f578d2302c3a3988647b077a7) )
	ROM_LOAD16_BYTE( "prg2.u21", 0x100000, 0x080000, CRC(bdaa5fbf) SHA1(abd72ac633c0c8e7b4b1d7902c0d6e014ba995fe) )
	ROM_LOAD16_BYTE( "prg3.u24", 0x100001, 0x080000, CRC(7aa9f941) SHA1(99bdbad7a96d461073b06a53c50fc57c2fd6fc6d) )

	ROM_REGION( 0x40000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.u77", 0x00000, 0x40000, CRC(56682696) SHA1(a372450d9a6d535123dfc31d8116074b168ab646) )

	ROM_REGION( 0x1000000, "gp9001", 0 )
	ROM_LOAD( "rom-1.bin", 0x000000, 0x400000, CRC(0df69ca2) SHA1(49670347ebd7e1067ff988cf842b275b7ee7b5f7) )
	ROM_LOAD( "rom-3.bin", 0x400000, 0x400000, CRC(60167d38) SHA1(fd2429808c59ef51fd5f5db84ea89a8dc504186e) )
	ROM_LOAD( "rom-2.bin", 0x800000, 0x400000, CRC(1bfea593) SHA1(ce06dc3097ae56b0df56d104bbf7efc9b5d968d4) )
	ROM_LOAD( "rom-4.bin", 0xc00000, 0x400000, CRC(bee03c94) SHA1(5bc1e6769c42857c03456426b502fcb86a114f19) )

	ROM_REGION( 0x100000, "oki1", 0 )       /* ADPCM Samples 1 */
	ROM_LOAD( "rom-5.bin", 0x000000, 0x100000, CRC(4274daf6) SHA1(85557b4707d529e5914f03c7a856864f5c24950e) )

	ROM_REGION( 0x100000, "oki2", 0 )       /* ADPCM Samples 2 */
	ROM_LOAD( "rom-6.bin", 0x000000, 0x100000, CRC(2a1c2426) SHA1(8abc3688ffc5ebb94b8d5118d4fa0908f07fe791) )
ROM_END


ROM_START( batridert )
	ROM_REGION( 0x200000, "maincpu", 0 )            /* Main 68k code */
	ROM_LOAD16_BYTE( "u22.bin",  0x000000, 0x080000, CRC(b135820e) SHA1(c222887d18a0a3ea0fcc973b95b29d69c86f7ec3) )
	ROM_LOAD16_BYTE( "prg1.u23", 0x000001, 0x080000, CRC(8ae7f592) SHA1(8a20ebf85eca621f578d2302c3a3988647b077a7) )
	ROM_LOAD16_BYTE( "prg2.u21", 0x100000, 0x080000, CRC(bdaa5fbf) SHA1(abd72ac633c0c8e7b4b1d7902c0d6e014ba995fe) )
	ROM_LOAD16_BYTE( "prg3.u24", 0x100001, 0x080000, CRC(7aa9f941) SHA1(99bdbad7a96d461073b06a53c50fc57c2fd6fc6d) )

	ROM_REGION( 0x40000, "audiocpu", 0 )            /* Sound Z80 code + bank */
	ROM_LOAD( "snd.u77", 0x00000, 0x40000, CRC(56682696) SHA1(a372450d9a6d535123dfc31d8116074b168ab646) )

	ROM_REGION( 0x1000000, "gp9001", 0 )
	ROM_LOAD( "rom-1.bin", 0x000000, 0x400000, CRC(0df69ca2) SHA1(49670347ebd7e1067ff988cf842b275b7ee7b5f7) )
	ROM_LOAD( "rom-3.bin", 0x400000, 0x400000, CRC(60167d38) SHA1(fd2429808c59ef51fd5f5db84ea89a8dc504186e) )
	ROM_LOAD( "rom-2.bin", 0x800000, 0x400000, CRC(1bfea593) SHA1(ce06dc3097ae56b0df56d104bbf7efc9b5d968d4) )
	ROM_LOAD( "rom-4.bin", 0xc00000, 0x400000, CRC(bee03c94) SHA1(5bc1e6769c42857c03456426b502fcb86a114f19) )

	ROM_REGION( 0x100000, "oki1", 0 )       /* ADPCM Samples 1 */
	ROM_LOAD( "rom-5.bin", 0x000000, 0x100000, CRC(4274daf6) SHA1(85557b4707d529e5914f03c7a856864f5c24950e) )

	ROM_REGION( 0x100000, "oki2", 0 )       /* ADPCM Samples 2 */
	ROM_LOAD( "rom-6.bin", 0x000000, 0x100000, CRC(2a1c2426) SHA1(8abc3688ffc5ebb94b8d5118d4fa0908f07fe791) )
ROM_END



/*
Battle Bakraid
Raizing/8ing, 1999

PCB Layout
----------

ET68-V99
|-----------------------------------------------------|
|TA8201  16.93MHz     ROM-6                   6264    |
|  YAC516                                             |
|       YMZ280B-F     ROM-7               SND_U0720   |
|                                                     |
| VOL                 ROM-8                 Z80       |
|                                                     |
|                   341256                            |
|                                               93C66 |
|                   341256               XILINX       |
|J                                       XC95108      |
|A                  27MHz     32MHz                   |
|M                                                    |
|M          DIPSW1                      341256  341256|
|A                  XILINX    XILINK                  |
|           DIPSW2  XC95144   XC95108   341256  341256|
|                                                     |
|           DIPSW3                                    |
|                                MACH211    PRG1_U023 |
| TEST_SW            68000                            |
|                                           PRG0_U022 |
|                                                     |
|                                           PRG3_U024 |
|                             L7A0498                 |
|                             GP9001        PRG2_U021 |
| ROM-0       ROM-1           (QFP208)                |
|                                                     |
|                               6264       MN414260   |
| ROM-2       ROM-3                                   |
|                               6264       MN414260   |
|-----------------------------------------------------|
Notes:
      ROM-0 to ROM-3 - 32M DIP42
      ROM-6 to ROM-8 - 32M DIP42 Byte Mode
      68000 clock - 16.000MHz (32/2)
      Z80 clock - 5.33333MHz (32/6)
      VSync - 60Hz
      HSync - 15.39kHz
*/



ROM_START( bbakraid )
	ROM_REGION( 0x200000, "maincpu", 0 )            /* Main 68k code */
	ROM_LOAD16_BYTE( "prg0u022_usa.bin", 0x000000, 0x080000, CRC(95fb2ffd) SHA1(c7f502f3945249573b66226e8bacc6a9bc230693) )
	ROM_LOAD16_BYTE( "prg1u023.new", 0x000001, 0x080000, CRC(4ae9aa64) SHA1(45fdf72141c4c9f24a38d4218c65874799b9c868) )
	ROM_LOAD16_BYTE( "prg2u021.bin", 0x100000, 0x080000, CRC(ffba8656) SHA1(6526bb65fad3384de3f301a7d1095cbf03757433) )
	ROM_LOAD16_BYTE( "prg3u024.bin", 0x100001, 0x080000, CRC(834b8ad6) SHA1(0dd6223bb0749819ad29811eeb04fd08d937abb0) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "sndu0720.bin", 0x00000, 0x20000, CRC(e62ab246) SHA1(00d23689dd423ecd4024c58b5903d16e890f1dff) )

	ROM_REGION( 0x1000000, "gp9001", 0 )
	ROM_LOAD( "gfxu0510.bin", 0x000000, 0x400000, CRC(9cca3446) SHA1(1123f8b8bfbe59a2c572cdf61f1ad27ff37f0f0d) )
	ROM_LOAD( "gfxu0512.bin", 0x400000, 0x400000, CRC(a2a281d5) SHA1(d9a6623f9433ad682223f9780c26cd1523ebc5c5) )
	ROM_LOAD( "gfxu0511.bin", 0x800000, 0x400000, CRC(e16472c0) SHA1(6068d679a8b3b65e05acd58a7ce9ead90177049f) )
	ROM_LOAD( "gfxu0513.bin", 0xc00000, 0x400000, CRC(8bb635a0) SHA1(9064f1a2d8bb88ddbca702fb8556d0dfe6a5cadc) )

	ROM_REGION( 0x0c00000, "ymz", 0 )       /* YMZ280B Samples */
	ROM_LOAD( "rom6.829", 0x000000, 0x400000, CRC(8848b4a0) SHA1(e0dce136c5d5a4c1a92b863e57848cd5927d06f1) )
	ROM_LOAD( "rom7.830", 0x400000, 0x400000, CRC(d6224267) SHA1(5c9b7b13effbef9f707811f84bfe50ca85e605e3) )
	ROM_LOAD( "rom8.831", 0x800000, 0x400000, CRC(a101dfb0) SHA1(4b729b0d562e09df35438e9e6b457b8de2690a6e) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "eeprom-bbakraid-new.bin", 0x000, 0x200, CRC(35c9275a) SHA1(1282034adf3c7a24545fd273729867058dc93027) )
ROM_END


ROM_START( bbakraidc )
	ROM_REGION( 0x200000, "maincpu", 0 )            /* Main 68k code */
	ROM_LOAD16_BYTE( "prg0u022_china.bin", 0x000000, 0x080000, CRC(760be084) SHA1(096c8a2336492d7370ae25f3385faebf6e9c3eca) )
	ROM_LOAD16_BYTE( "prg1u023.new", 0x000001, 0x080000, CRC(4ae9aa64) SHA1(45fdf72141c4c9f24a38d4218c65874799b9c868) )
	ROM_LOAD16_BYTE( "prg2u021.bin", 0x100000, 0x080000, CRC(ffba8656) SHA1(6526bb65fad3384de3f301a7d1095cbf03757433) )
	ROM_LOAD16_BYTE( "prg3u024.bin", 0x100001, 0x080000, CRC(834b8ad6) SHA1(0dd6223bb0749819ad29811eeb04fd08d937abb0) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "sndu0720.bin", 0x00000, 0x20000, CRC(e62ab246) SHA1(00d23689dd423ecd4024c58b5903d16e890f1dff) )

	ROM_REGION( 0x1000000, "gp9001", 0 )
	ROM_LOAD( "gfxu0510.bin", 0x000000, 0x400000, CRC(9cca3446) SHA1(1123f8b8bfbe59a2c572cdf61f1ad27ff37f0f0d) )
	ROM_LOAD( "gfxu0512.bin", 0x400000, 0x400000, CRC(a2a281d5) SHA1(d9a6623f9433ad682223f9780c26cd1523ebc5c5) )
	ROM_LOAD( "gfxu0511.bin", 0x800000, 0x400000, CRC(e16472c0) SHA1(6068d679a8b3b65e05acd58a7ce9ead90177049f) )
	ROM_LOAD( "gfxu0513.bin", 0xc00000, 0x400000, CRC(8bb635a0) SHA1(9064f1a2d8bb88ddbca702fb8556d0dfe6a5cadc) )

	ROM_REGION( 0x0c00000, "ymz", 0 )       /* YMZ280B Samples */
	ROM_LOAD( "rom6.829", 0x000000, 0x400000, CRC(8848b4a0) SHA1(e0dce136c5d5a4c1a92b863e57848cd5927d06f1) )
	ROM_LOAD( "rom7.830", 0x400000, 0x400000, CRC(d6224267) SHA1(5c9b7b13effbef9f707811f84bfe50ca85e605e3) )
	ROM_LOAD( "rom8.831", 0x800000, 0x400000, CRC(a101dfb0) SHA1(4b729b0d562e09df35438e9e6b457b8de2690a6e) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "eeprom-bbakraid-new.bin", 0x000, 0x200, CRC(35c9275a) SHA1(1282034adf3c7a24545fd273729867058dc93027) )
ROM_END


ROM_START( bbakraidj )
	ROM_REGION( 0x200000, "maincpu", 0 )            /* Main 68k code */
	ROM_LOAD16_BYTE( "prg0u022.new", 0x000000, 0x080000, CRC(fa8d38d3) SHA1(aba91d87a8a62d3fe1139b4437b16e2f844264ad) )
	ROM_LOAD16_BYTE( "prg1u023.new", 0x000001, 0x080000, CRC(4ae9aa64) SHA1(45fdf72141c4c9f24a38d4218c65874799b9c868) )
	ROM_LOAD16_BYTE( "prg2u021.bin", 0x100000, 0x080000, CRC(ffba8656) SHA1(6526bb65fad3384de3f301a7d1095cbf03757433) )
	ROM_LOAD16_BYTE( "prg3u024.bin", 0x100001, 0x080000, CRC(834b8ad6) SHA1(0dd6223bb0749819ad29811eeb04fd08d937abb0) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "sndu0720.bin", 0x00000, 0x20000, CRC(e62ab246) SHA1(00d23689dd423ecd4024c58b5903d16e890f1dff) )

	ROM_REGION( 0x1000000, "gp9001", 0 )
	ROM_LOAD( "gfxu0510.bin", 0x000000, 0x400000, CRC(9cca3446) SHA1(1123f8b8bfbe59a2c572cdf61f1ad27ff37f0f0d) )
	ROM_LOAD( "gfxu0512.bin", 0x400000, 0x400000, CRC(a2a281d5) SHA1(d9a6623f9433ad682223f9780c26cd1523ebc5c5) )
	ROM_LOAD( "gfxu0511.bin", 0x800000, 0x400000, CRC(e16472c0) SHA1(6068d679a8b3b65e05acd58a7ce9ead90177049f) )
	ROM_LOAD( "gfxu0513.bin", 0xc00000, 0x400000, CRC(8bb635a0) SHA1(9064f1a2d8bb88ddbca702fb8556d0dfe6a5cadc) )

	ROM_REGION( 0x0c00000, "ymz", 0 )       /* YMZ280B Samples */
	ROM_LOAD( "rom6.829", 0x000000, 0x400000, CRC(8848b4a0) SHA1(e0dce136c5d5a4c1a92b863e57848cd5927d06f1) )
	ROM_LOAD( "rom7.830", 0x400000, 0x400000, CRC(d6224267) SHA1(5c9b7b13effbef9f707811f84bfe50ca85e605e3) )
	ROM_LOAD( "rom8.831", 0x800000, 0x400000, CRC(a101dfb0) SHA1(4b729b0d562e09df35438e9e6b457b8de2690a6e) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "eeprom-bbakraid-new.bin", 0x000, 0x200, CRC(35c9275a) SHA1(1282034adf3c7a24545fd273729867058dc93027) )
ROM_END


ROM_START( bbakraidja )
	ROM_REGION( 0x200000, "maincpu", 0 )            /* Main 68k code */
	ROM_LOAD16_BYTE( "prg0u022.bin", 0x000000, 0x080000, CRC(0dd59512) SHA1(c6a4e6aa49c6ac3b04ae62a0a4cc8084ae048381) )
	ROM_LOAD16_BYTE( "prg1u023.bin", 0x000001, 0x080000, CRC(fecde223) SHA1(eb5ac0eda49b4b0f3d25d8a8bb356e77a453d3a7) )
	ROM_LOAD16_BYTE( "prg2u021.bin", 0x100000, 0x080000, CRC(ffba8656) SHA1(6526bb65fad3384de3f301a7d1095cbf03757433) )
	ROM_LOAD16_BYTE( "prg3u024.bin", 0x100001, 0x080000, CRC(834b8ad6) SHA1(0dd6223bb0749819ad29811eeb04fd08d937abb0) )

	ROM_REGION( 0x20000, "audiocpu", 0 )            /* Sound Z80 code */
	ROM_LOAD( "sndu0720.bin", 0x00000, 0x20000, CRC(e62ab246) SHA1(00d23689dd423ecd4024c58b5903d16e890f1dff) )

	ROM_REGION( 0x1000000, "gp9001", 0 )
	ROM_LOAD( "gfxu0510.bin", 0x000000, 0x400000, CRC(9cca3446) SHA1(1123f8b8bfbe59a2c572cdf61f1ad27ff37f0f0d) )
	ROM_LOAD( "gfxu0512.bin", 0x400000, 0x400000, CRC(a2a281d5) SHA1(d9a6623f9433ad682223f9780c26cd1523ebc5c5) )
	ROM_LOAD( "gfxu0511.bin", 0x800000, 0x400000, CRC(e16472c0) SHA1(6068d679a8b3b65e05acd58a7ce9ead90177049f) )
	ROM_LOAD( "gfxu0513.bin", 0xc00000, 0x400000, CRC(8bb635a0) SHA1(9064f1a2d8bb88ddbca702fb8556d0dfe6a5cadc) )

	ROM_REGION( 0x0c00000, "ymz", 0 )       /* YMZ280B Samples */
	ROM_LOAD( "rom6.829", 0x000000, 0x400000, CRC(8848b4a0) SHA1(e0dce136c5d5a4c1a92b863e57848cd5927d06f1) )
	ROM_LOAD( "rom7.830", 0x400000, 0x400000, CRC(d6224267) SHA1(5c9b7b13effbef9f707811f84bfe50ca85e605e3) )
	ROM_LOAD( "rom8.831", 0x800000, 0x400000, CRC(a101dfb0) SHA1(4b729b0d562e09df35438e9e6b457b8de2690a6e) )

	ROM_REGION( 0x200, "eeprom", 0 )
	ROM_LOAD( "eeprom-bbakraid.bin", 0x000, 0x200, CRC(7f97d347) SHA1(3096c399019924dbb7d6673483f6a011f89467c6) )
ROM_END


// dedicated PCB marked Pro Bowl
ROM_START( nprobowl )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "newprobowl-prg0-u021", 0x000000, 0x080000, CRC(3a57f122) SHA1(cc361c295f23bc0479ba49eb15de2ec6ca535a56) ) // 11xxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "newprobowl-prg1-u024", 0x000001, 0x080000, CRC(9e9bb58a) SHA1(3d2159bde418dee5d89e3df9a248b4b1989e6ee9) ) // 11xxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200000, "gp9001", 0 )
	ROM_LOAD16_BYTE( "newprobowl-chr1-u0519", 0x000000, 0x80000, CRC(6700a9bf) SHA1(12a72aa0b91119fbbed994aec702a6869af6f287) )
	ROM_LOAD16_BYTE( "newprobowl-chr0-u0518", 0x000001, 0x80000, CRC(0736cccd) SHA1(5a4b691be1df697fef3847456c0f4bb3466c403f) )
	ROM_LOAD16_BYTE( "newprobowl-chr2-u0520", 0x100000, 0x80000, CRC(e5f6d0b6) SHA1(6e1a4792698be4b478118e8c82edb0cf3e2286f2) )
	ROM_LOAD16_BYTE( "newprobowl-chr3-u0521", 0x100001, 0x80000, CRC(00c21951) SHA1(922abde172fb82b504dce41b95227740f16208a7) )

	ROM_REGION( 0x100000, "oki1", 0 )
	ROM_LOAD( "newprobowl-adpcm0-u0834", 0x00000, 0x80000, CRC(3b40b161) SHA1(ff8ba38dd7e0dadbf72810470e3d9afb1cd983d2) )
	ROM_LOAD( "newprobowl-adpcm1-u0835", 0x80000, 0x80000, CRC(8c191e60) SHA1(f81c2849ffc553d921fc680cd50c2997b834c44a) )
ROM_END

ROM_START( probowl2 ) // identical to New Pro Bowl but for slight mods to the GFX ROMs to display the different title
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "probowl2.prg0.u021", 0x000000, 0x080000, CRC(3a57f122) SHA1(cc361c295f23bc0479ba49eb15de2ec6ca535a56) ) // 11xxxxxxxxxxxxxxxxx = 0xFF
	ROM_LOAD16_BYTE( "probowl2.prg1.u024", 0x000001, 0x080000, CRC(9e9bb58a) SHA1(3d2159bde418dee5d89e3df9a248b4b1989e6ee9) ) // 11xxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x200000, "gp9001", 0 )
	ROM_LOAD16_BYTE( "probowl2.chr1.u0519", 0x000000, 0x80000, CRC(66a5c9cd) SHA1(b8edc6a66e3929af2a1399e5fb6fc341b9789136) )
	ROM_LOAD16_BYTE( "probowl2.chr0.u0518", 0x000001, 0x80000, CRC(b4439673) SHA1(5ca19d9c3c3cce9485fb9bd5224f0b14abe07f49) )
	ROM_LOAD16_BYTE( "probowl2.chr2.u0520", 0x100000, 0x80000, CRC(e05be3a0) SHA1(922c6f094358cf3df790063e68d5dd4a32d46b06) )
	ROM_LOAD16_BYTE( "probowl2.chr3.u0521", 0x100001, 0x80000, CRC(55e2565f) SHA1(b6a570b736a9b30e26d2a59b53f218ef5cd6f0f6) )

	ROM_REGION( 0x100000, "oki1", 0 )
	ROM_LOAD( "probowl2.adpcm0.u0834", 0x00000, 0x80000, CRC(be2fb43f) SHA1(f3f9bc522f4668852b751f291cef0000bb86b779) )
	ROM_LOAD( "probowl2.adpcm1.u0835", 0x80000, 0x80000, CRC(8c191e60) SHA1(f81c2849ffc553d921fc680cd50c2997b834c44a) )
ROM_END




GAME( 1993, sstriker,    0,        mahoudai,   sstriker,   sstriker_state, empty_init,      ROT270, "Raizing",                         "Sorcer Striker",           MACHINE_SUPPORTS_SAVE ) // verified on two different PCBs
GAME( 1993, sstrikerk,   sstriker, mahoudai,   sstrikerk,  sstriker_state, empty_init,      ROT270, "Raizing (Unite Trading license)", "Sorcer Striker (Korea)",   MACHINE_SUPPORTS_SAVE ) // Although the region jumper is functional, it's a Korean board / version
GAME( 1993, mahoudai,    sstriker, mahoudai,   mahoudai,   sstriker_state, empty_init,      ROT270, "Raizing (Able license)",          "Mahou Daisakusen (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1994, kingdmgp,    0,        shippumd,   kingdmgp,   sstriker_state, empty_init,      ROT270, "Raizing / Eighting", "Kingdom Grandprix",               MACHINE_SUPPORTS_SAVE ) // from Korean board, missing letters on credits screen but this is correct
GAME( 1994, shippumd,    kingdmgp, shippumd,   shippumd,   sstriker_state, empty_init,      ROT270, "Raizing / Eighting", "Shippu Mahou Daisakusen (Japan)", MACHINE_SUPPORTS_SAVE )

GAME( 1996, bgaregga,    0,        bgaregga,   bgaregga,   bgaregga_state, init_bgaregga,   ROT270, "Raizing / Eighting", "Battle Garegga (Europe / USA / Japan / Asia) (Sat Feb 3 1996)",            MACHINE_SUPPORTS_SAVE )
GAME( 1996, bgareggat,   bgaregga, bgaregga,   bgaregga,   bgaregga_state, init_bgaregga,   ROT270, "Raizing / Eighting", "Battle Garegga (location test) (Wed Jan 17 1996)",                         MACHINE_SUPPORTS_SAVE )
GAME( 1996, bgareggahk,  bgaregga, bgaregga,   bgareggahk, bgaregga_state, init_bgaregga,   ROT270, "Raizing / Eighting", "Battle Garegga (Austria / Hong Kong) (Sat Feb 3 1996)",                    MACHINE_SUPPORTS_SAVE )
GAME( 1996, bgareggatw,  bgaregga, bgaregga,   bgareggatw, bgaregga_state, init_bgaregga,   ROT270, "Raizing / Eighting", "Battle Garegga (Taiwan / Germany) (Thu Feb 1 1996)",                       MACHINE_SUPPORTS_SAVE )
GAME( 1996, bgareggak,   bgaregga, bgaregga,   bgareggak,  bgaregga_state, init_bgaregga,   ROT270, "Raizing / Eighting", "Battle Garegga (Korea / Greece) (Wed Feb 7 1996)",                         MACHINE_SUPPORTS_SAVE )
GAME( 1996, bgaregganv,  bgaregga, bgaregga,   bgareggahk, bgaregga_state, init_bgaregga,   ROT270, "Raizing / Eighting", "Battle Garegga - New Version (Austria / Hong Kong) (Sat Mar 2 1996)",      MACHINE_SUPPORTS_SAVE ) // displays New Version only when set to HK
GAME( 1996, bgareggat2,  bgaregga, bgaregga,   bgaregga,   bgaregga_state, init_bgaregga,   ROT270, "Raizing / Eighting", "Battle Garegga - Type 2 (Europe / USA / Japan / Asia) (Sat Mar 2 1996)",   MACHINE_SUPPORTS_SAVE ) // displays Type 2 only when set to Europe
GAME( 1996, bgareggacn,  bgaregga, bgaregga,   bgareggacn, bgaregga_state, init_bgaregga,   ROT270, "Raizing / Eighting", "Battle Garegga - Type 2 (Denmark / China) (Tue Apr 2 1996)",               MACHINE_SUPPORTS_SAVE ) // displays Type 2 only when set to Denmark
GAME( 1998, bgareggabl,  bgaregga, bgareggabl, bgareggabl, bgaregga_bootleg_state, init_bgaregga,   ROT270, "bootleg (Melody)",   "1945 Er Dai / 1945 Part-2 (Chinese hack of Battle Garegga)",               MACHINE_SUPPORTS_SAVE ) // based on Thu Feb 1 1996 set, Region hardcoded to China
GAME( 1997, bgareggabla, bgaregga, bgareggabl, bgareggabl, bgaregga_bootleg_state, init_bgaregga,   ROT270, "bootleg (Melody)",   "Leishen Chuan / Thunder Deity Biography (Chinese hack of Battle Garegga)", MACHINE_SUPPORTS_SAVE ) // based on Thu Feb 1 1996 set, Region hardcoded to Asia
GAME( 1996, bgareggablj, bgaregga, bgareggabl, bgareggabl, bgaregga_bootleg_state, init_bgaregga,   ROT270, "bootleg",            "Battle Garegga (Japan, bootleg) (Sat Feb 3 1996)",                         MACHINE_SUPPORTS_SAVE )

// these are all based on Version B, even if only the Japan version states 'version B'
GAME( 1998, batrider,    0,        batrider,   batrider,   batrider_state, init_batrider,   ROT270, "Raizing / Eighting", "Armed Police Batrider (Europe) (Fri Feb 13 1998)",           MACHINE_SUPPORTS_SAVE )
GAME( 1998, batrideru,   batrider, batrider,   batrider,   batrider_state, init_batrider,   ROT270, "Raizing / Eighting", "Armed Police Batrider (USA) (Fri Feb 13 1998)",              MACHINE_SUPPORTS_SAVE )
GAME( 1998, batriderc,   batrider, batrider,   batrider,   batrider_state, init_batrider,   ROT270, "Raizing / Eighting", "Armed Police Batrider (China) (Fri Feb 13 1998)",            MACHINE_SUPPORTS_SAVE )
GAME( 1998, batriderj,   batrider, batrider,   batriderj,  batrider_state, init_batrider,   ROT270, "Raizing / Eighting", "Armed Police Batrider (Japan, B version) (Fri Feb 13 1998)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, batriderk,   batrider, batrider,   batrider,   batrider_state, init_batrider,   ROT270, "Raizing / Eighting", "Armed Police Batrider (Korea) (Fri Feb 13 1998)",            MACHINE_SUPPORTS_SAVE )
// older revision of the code
GAME( 1998, batriderja,  batrider, batrider,   batriderj,  batrider_state, init_batrider,   ROT270, "Raizing / Eighting", "Armed Police Batrider (Japan, older version) (Mon Dec 22 1997)", MACHINE_SUPPORTS_SAVE )
GAME( 1998, batriderhk,  batrider, batrider,   batrider,   batrider_state, init_batrider,   ROT270, "Raizing / Eighting", "Armed Police Batrider (Hong Kong) (Mon Dec 22 1997)",            MACHINE_SUPPORTS_SAVE )
GAME( 1998, batridert,   batrider, batrider,   batrider,   batrider_state, init_batrider,   ROT270, "Raizing / Eighting", "Armed Police Batrider (Taiwan) (Mon Dec 22 1997)",               MACHINE_SUPPORTS_SAVE )

// Battle Bakraid
// the 'unlimited' version is a newer revision of the code
GAME( 1999, bbakraid,    0,        bbakraid,   bbakraid,   bbakraid_state, init_bbakraid,   ROT270, "Eighting", "Battle Bakraid - Unlimited Version (USA) (Tue Jun 8 1999)",   MACHINE_SUPPORTS_SAVE )
GAME( 1999, bbakraidc,   bbakraid, bbakraid,   bbakraid,   bbakraid_state, init_bbakraid,   ROT270, "Eighting", "Battle Bakraid - Unlimited Version (China) (Tue Jun 8 1999)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, bbakraidj,   bbakraid, bbakraid,   bbakraid,   bbakraid_state, init_bbakraid,   ROT270, "Eighting", "Battle Bakraid - Unlimited Version (Japan) (Tue Jun 8 1999)", MACHINE_SUPPORTS_SAVE )
// older revision of the code
GAME( 1999, bbakraidja,  bbakraid, bbakraid,   bbakraid,   bbakraid_state, init_bbakraid,   ROT270, "Eighting", "Battle Bakraid (Japan) (Wed Apr 7 1999)", MACHINE_SUPPORTS_SAVE )

// dedicated PCB
GAME( 1996, nprobowl,    0,        nprobowl,   nprobowl,   nprobowl_state, empty_init,      ROT0,   "Zuck / Able Corp", "New Pro Bowl", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE ) // bad GFXs, no sound banking, controls, etc
GAME( 1996, probowl2,    nprobowl, nprobowl,   nprobowl,   nprobowl_state, empty_init,      ROT0,   "Zuck / Able Corp", "Pro Bowl 2",   MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_MECHANICAL | MACHINE_SUPPORTS_SAVE ) // bad GFXs, no sound banking, controls, etc
