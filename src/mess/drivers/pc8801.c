// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/*************************************************************************************************************************************

    PC-8801 (c) 1981 NEC

    driver by Angelo Salese, original MESS PC-88SR driver by ???

    TODO:
    - implement proper i8214 routing, also add irq latch mechanism;
    - Fix up Floppy Terminal Count 0 / 1 writes properly, Castle Excellent (and presumably other games) is very picky about it.

    - add differences between various models;
    - implement proper upd3301 / i8257 support;
    - fix "jumps" in mouse support pointer (noticeable in Balance of Power);
    - Add limits for extend work RAM;
    - What happens to the palette contents when the analog/digital palette mode changes?
    - waitstates;
    - dipswitches needs to be controlled;
    - below notes states that plain PC-8801 doesn't have a disk CPU, but the BIOS clearly checks the floppy ports. Wrong info?
    - clean-ups, banking and video in particular (i.e. hook-ups with memory region should go away and device models should be used instead)

    per-game specific TODO:
    - 100yen Soft 8 Revival Special: tight loop with vblank bit, but vblank irq takes too much time to execute its code;
    - 177: gameplay is too fast (parent pc8801 only);
    - 1942: missing sound, enables a masked irq;
    - Acro Jet: hangs waiting for an irq (floppy issue);
    - Arcus: doesn't surpass Wolf Team logo;
    - Advanced Fantasian: garbage during gameplay (floppy?)
    - American Success: reads the light pen?
    - Attacker: resets after a bunch of animation frames;
    - Balance of Power: uses the SIO port for something ...
    - Belloncho Shintai Kensa: hangs
    - Bishoujo Baseball Gakuen: checks ym2608 after intro screen;
    - The Black Onyx: writes a katakana msg: "sono kata ha koko ni orimasen" then doesn't show up anything. (Needs user disk?)
    - Boukenshatachi: dies after the intro.
    - Campaign Ban Daisenryaku 2: Hangs at title screen?
    - Carigraph: inputs doesn't work?
    - Can Can Bunny: bitmap artifacts on intro, caused by a fancy usage of the attribute vram;
    - Can Can Bunny: no sound (regression);
    - Can Can Bunny Superior: black screen during the intro
    - Chou Bishoujo Densetsu CROQUIS: accesses ports 0xa0-0xa3 and 0xc2-0xc3
    - Combat: mono gfx mode enabled, but I don't see any noticeable quirk?
    - Cranston Manor (actually N88-Basic demo): no sound
    - Datenshi Kyouko: gfx garbage on the right edge?
    - Final Crisis: sound stuck with OPNA?
    - Fire Hawk: tries to r/w the opn ports (probably crashed due to floppy?)
    - Game Music Library: "Disk I/O error on 3040", starting from Sorcerian item
    - Gaudi - Barcelona no Kaze: fails PCM loading
    - GeGeGe no Kitarou: title screen text/bitmap contrast is pretty ugly (BTANB?);
    - Grobda: palette is ugly (parent pc8801 only);
    - Makaimura: after losing a life the game doesn't work properly anymore, copy protection?
    - Music Collection Vol. 2 - Final Fantasy Tokushuu: sound irq dies pretty soon
    - N-BASIC: cursor doesn't show up;
    - The Return of Ishtar: z80 exception after entering the name.
    - Star Cruiser: bad kanji data?
    - Star Cruiser: reads at i/o 0x8e?
    - Wanderers from Ys: user data disk looks screwed? It loads with everything as maximum as per now ...
    - WerDragon: no BGMs
    - Xevious: game is too fast (parent pc8801 only)

    list of games/apps that crashes due of floppy issues (* -> denotes games fixed with current floppy code, # -> regressed with current floppy code):
    * Agni no Ishi
    * Amazoness no Hihou (takes invalid data from floppy)
    - American Truck / American Truck SR (polls read deleted data command)
    * Ankokujou
    * Ao No Sekizou (fdc CPU irq doesn't fire anymore)
    * Arcus
    * Attacker
    - Autumn Park (BASIC error)
    * Battle Gorilla
    * Belloncho Shintai Kensa
    - Bishoujo Noriko Part I (writes to FDC CPU ROM then expects some strict values, taken from floppy image)
    * Blassty (attempts to read at 0x801b)
    - Bokosuka Wars (polls read ID command)
    * Boukenshatachi
    * Can Can Bunny Superior
    - Carmine
    - Castle Excellent (sets sector 0xf4? Jumps to 0xa100 and it shouldn't) (REGRESSED with current floppy code)
    - Card Game Pro 8.8k Plus Unit 1 (prints Disk i/o error 135 in vram, not visible for whatever reason)
    - Championship Lode Runner (fdc CPU irq doesn't fire anymore)
    - Change Vol. 1 (stops at PCM loading)
    - Chikyuu Boueigun (disk i/o error during "ESDF SYSTEM LOADING") (REGRESSED with current floppy code)
    * Chikyuu Senshi Rayieza (fdc CPU crashes)
    - Choplifter
    - Columns (code at 0x28c8, copy protection)
    - Corridor ("THIS SYSTEM NOT KOEI SYSTEM" printed on screen) (REGRESSED with current floppy code)
    # Craze (returns to basic after logo pops up, tries to self-modify program data via the window offset?)
    * Crimson
    * Crimson 3
    * Cuby Panic (copy protection routine at 0x911A)
    - Daidasso (prints "BOOT dekimasen" on screen -> can't boot)
    - Daikoukai Jidai (YSHELL.COM error)
    - Databox (app)
    - Day Dream ("Bad drive number at 570")
    - Demons Ring
    * Dennou Tsuushin
    - Door Door MK-2 (sets up TC in the middle of execution phase read then wants status bit 6 to be low PC=0x7050 of fdc cpu)
    * Dragon Slayer - The Legend of Heroes 2
    - Dungeon Buster
    * El Dorado Denki
    * Elevator Action
    - Emerald Densetsu (dies after few seconds of intro)
    - Emerald Dragon (it seems to miss a timer)
    - Emmy
    - Explosion (fails to load ADPCM data?)
    * F15 Strike Eagle
    - F2 Grand Prix ("Boot dekimasen")
    # Fangs - The Saga of Wolf Blood (Crashes at the first random battle)
    - Fantasian
    * Final Zone
    # Final Zone (demo) (REGRESSION: asserts at MESS boot)
    - Fruit Panic
    - FSD Sample Ongaku Shuu Vol. 1-7
    - Gaia no Kiba (Disk I/O error at 150)
    - Gaiflame
    - Gambler Jiko Chuushin ha
    - Gambler Jiko Chuushin ha 2
    - Gambler Jiko Chuushin ha 3
    - Gambler Jiko Chuushin ha 3 (demo)
    - Gambler Jiko Chuushin ha Mahjong Puzzle Collection
    - Gambler Jiko Chuushin ha Mahjong Puzzle Collection (demo)
    * Game Music Library
    * Gaudi - Barcelona no Kaze (bad Wolfteam logo then black screen)
    - GC-clusterz Music Disk Vol. 1-7
    * Genji
    * Gokuraku Tengoku
    - Grodius 3 (might not be floppy)
    - Gun Ship (at gameplay)
    (Hacker)

    - Harakiri
    - Kaseijin (app) (code snippet is empty at some point)
    - Lamia: fails to create an user disk (after character creation) -> disk write error
    * MakaiMura (attempts to r/w the sio ports, but it's clearly crashed)
    * Mugen Senshi Valis (at Telenet logo, it also appears to have a nasty copy protection when taking a specific item (untested))
    - Mr. Pro Yakyuu
    - Panorama Toh
    - PC-8034 (app)
    - PC-8037SR (app)
    - P1 (app)
    - Pattern Editor 88 (app)
    - Super Shunbo II (app) (Load error)
    - Super TII (app)
    * The Return of Ishtar
    - Tobira wo Akete (random crashes in parent pc8801 only)

    list of games that doesn't like i8214_irq_level == 5 in sound irq
    - 100yen Disk 2 / Jumper 2: Sound BGM dies pretty soon;
    - Alpha (demo): stuck note in title screen, doesn't seem to go further;
    - Ayumi: black screen after new game / load game screen;
    - Brunette: No sound, eventually hangs at gameplay;
    - Digital Devil Story Megami Tensei: hangs at gameplay (sound irq issue)
    - Double Face: hangs at logo (sound irq issue)

    games that needs to NOT have write-protect floppies (BTANBs):
    - Balance of Power
    - Blue Moon Story: moans with a kanji msg;
    - Mahjong Clinic Zoukangou
    - Tobira wo Akete (hangs at title screen)

    games that needs to HAVE write-protect floppies (BTANBs):
    - 100 Yen Disk 7: (doesn't boot in V2 mode)

    other BTANBs
    - Attack Hirokochan: returns to BASIC after an initial animation, needs BASIC V1:
    - Jark (needs PC-8801MC)
    - Kuronekosou Souzoku Satsujin Jiken: "Illegal function call in 105", needs BASIC V1;

    Notes:
    - BIOS disk ROM defines what kind of floppies you could load:
      * with 0x0800 ROM size you can load 2d floppies only;
      * with 0x2000 ROM size you can load 2d and 2hd floppies;
    - Later models have palette bugs with some games (Alphos, Tokyo Nampa Street).
      This is because you have to set up the V1 / V2 DIP-SW to V1 for those games (it's the BIOS that sets up to analog and never changes back otherwise).
    - Password for "AY-1: Fortress Solomon" is "123" then press enter, any other key pressed makes it to fail the check (you must soft reset the machine)
    - Pressing Home in Dennou Gakuen during gameplay makes it to show a fake DASM screen. That's supposed to be a panic button and it's also in the
      sequels (with different screens);

    Bankswitch Notes:
    - 0x31 - graphic banking
    - 0x32 - misc banking
    - 0x5c / 0x5f - VRAM banking
    - 0x70 - window offset (banking)
    - 0x71 - extra ROM banking
    - 0x78 - window offset (banking) increment (+ 0x100)
    - 0xe2 / 0xe3 - extra RAM banking
    - 0xf0 / 0xf1 = kanji banking

======================================================================================================================================

    PC-88xx Models (and similar machines like PC-80xx and PC-98DO)

    Model            | release |      CPU     |                      BIOS components                        |       |
                     |         |     clock    | N-BASIC | N88-BASIC | N88-BASIC Enh |  Sound  |  CD |  Dict |  Disk | Notes
    ==================================================================================================================================
    PC-8001          | 1979-03 |   z80A @ 4   |    X    |     -     |       -       |    -    |  -  |   -   |   -   |
    PC-8001A         |   ??    |   z80A @ 4   |    X    |     -     |       -       |    -    |  -  |   -   |   -   | (U)
    PC-8801          | 1981-11 |   z80A @ 4   |    X    |     X     |       -       |    -    |  -  |   -   |   -   | (KO)
    PC-8801A         |   ??    |   z80A @ 4   |    X    |     X     |       -       |    -    |  -  |   -   |   -   | (U)
    PC-8001 mkII     | 1983-03 |   z80A @ 4   |    X    |     -     |       -       |    -    |  -  |   -   |   -   | (GE),(KO)
    PC-8001 mkIIA    |   ??    |   z80A @ 4   |    X    |     -     |       -       |    -    |  -  |   -   |   -   | (U),(GE)
    PC-8801 mkII     | 1983-11 |   z80A @ 4   |    X    |     X     |       -       |    -    |  -  |   -   | (FDM) | (K1)
    PC-8001 mkII SR  | 1985-01 |   z80A @ 4   |    X    |     -     |       -       |    -    |  -  |   -   |   -   | (GE),(NE),(KO)
    PC-8801 mkII SR  | 1985-03 |   z80A @ 4   |    X    |     X     |       X       |    X    |  -  |   -   | (FDM) | (K1)
    PC-8801 mkII TR  | 1985-10 |   z80A @ 4   |    X    |     X     |       X       |    X    |  -  |   -   | (FD2) | (K1)
    PC-8801 mkII FR  | 1985-11 |   z80A @ 4   |    X    |     X     |       X       |    X    |  -  |   -   | (FDM) | (K1)
    PC-8801 mkII MR  | 1985-11 |   z80A @ 4   |    X    |     X     |       X       |    X    |  -  |   -   | (FDH) | (K2)
    PC-8801 FH       | 1986-11 |  z80H @ 4/8  |    X    |     X     |       X       |    X    |  -  |   -   | (FDM) | (K2)
    PC-8801 MH       | 1986-11 |  z80H @ 4/8  |    X    |     X     |       X       |    X    |  -  |   -   | (FDH) | (K2)
    PC-88 VA         | 1987-03 | z80H+v30 @ 8 |    -    |     X     |       X       |    X    |  -  |   X   | (FDH) | (K2)
    PC-8801 FA       | 1987-11 |  z80H @ 4/8  |    X    |     X     |       X       |    X    |  -  |   -   | (FD2) | (K2)
    PC-8801 MA       | 1987-11 |  z80H @ 4/8  |    X    |     X     |       X       |    X    |  -  |   X   | (FDH) | (K2)
    PC-88 VA2        | 1988-03 | z80H+v30 @ 8 |    -    |     X     |       X       |    X    |  -  |   X   | (FDH) | (K2)
    PC-88 VA3        | 1988-03 | z80H+v30 @ 8 |    -    |     X     |       X       |    X    |  -  |   X   | (FD3) | (K2)
    PC-8801 FE       | 1988-10 |  z80H @ 4/8  |    X    |     X     |       X       |    X    |  -  |   -   | (FD2) | (K2)
    PC-8801 MA2      | 1988-10 |  z80H @ 4/8  |    X    |     X     |       X       |    X    |  -  |   X   | (FDH) | (K2)
    PC-98 DO         | 1989-06 |   z80H @ 8   |    X    |     X     |       X       |    X    |  -  |   -   | (FDH) | (KE)
    PC-8801 FE2      | 1989-10 |  z80H @ 4/8  |    X    |     X     |       X       |    X    |  -  |   -   | (FD2) | (K2)
    PC-8801 MC       | 1989-11 |  z80H @ 4/8  |    X    |     X     |       X       |    X    |  X  |   X   | (FDH) | (K2)
    PC-98 DO+        | 1990-10 |   z80H @ 8   |    X    |     X     |       X       |    X    |  -  |   -   | (FDH) | (KE)

    info for PC-98 DO & DO+ refers to their 88-mode

    Disk Drive options:
    (FDM): there exist three model of this computer: Model 10 (base model, only optional floppy drive), Model 20
        (1 floppy drive for 5.25" 2D disks) and Model 30 (2 floppy drive for 5.25" 2D disks)
    (FD2): 2 floppy drive for 5.25" 2D disks
    (FDH): 2 floppy drive for both 5.25" 2D disks and 5.25" HD disks
    (FD3): 2 floppy drive for both 5.25" 2D disks and 5.25" HD disks + 1 floppy drive for 3.5" 2TD disks

    Notes:
    (U): US version
    (GE): Graphic Expansion for PC-8001
    (NE): N-BASIC Expansion for PC-8001 (similar to N88-BASIC Expansion for PC-88xx)
    (KO): Optional Kanji ROM
    (K1): Kanji 1st Level ROM
    (K2): Kanji 2nd Level ROM
    (KE): Kanji Enhanced ROM

    Memory mounting locations:
     * N-BASIC 0x0000 - 0x5fff, N-BASIC Expansion & Graph Enhhancement 0x6000 - 0x7fff
     * N-BASIC 0x0000 - 0x5fff, N-BASIC Expansion & Graph Enhhancement 0x6000 - 0x7fff
     * N88-BASIC 0x0000 - 0x7fff, N88-BASIC Expansion & Graph Enhhancement 0x6000 - 0x7fff
     * Sound BIOS: 0x6000 - 0x7fff
     * CD-ROM BIOS: 0x0000 - 0x7fff
     * Dictionary: 0xc000 - 0xffff (32 Banks)

    info from http://www.geocities.jp/retro_zzz/machines/nec/cmn_roms.html
    also, refer to http://www.geocities.jp/retro_zzz/machines/nec/cmn_vers.html for
        info about BASIC revisions in the various models (BASIC V2 is the BASIC
        Expansion, if I unerstood correctly)

*************************************************************************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "imagedev/cassette.h"
#include "bus/centronics/ctronics.h"
#include "machine/i8255.h"
#include "machine/upd1990a.h"
#include "machine/upd765.h"
#include "machine/i8214.h"
#include "machine/i8251.h"
#include "sound/2203intf.h"
#include "sound/2608intf.h"
#include "sound/beep.h"
//#include "includes/pc8801.h"

//#define USE_PROPER_I8214


#define IRQ_DEBUG       (0)
#define IRQ_LOG(x) do { if (IRQ_DEBUG) printf x; } while (0)

#define MASTER_CLOCK XTAL_4MHz
/* TODO: clocks of this */
#define PIXEL_CLOCK_15KHz XTAL_14_31818MHz
#define PIXEL_CLOCK_24KHz XTAL_21_4772MHz

#define I8214_TAG       "i8214"
#define UPD1990A_TAG    "upd1990a"
#define I8251_TAG       "i8251"

struct crtc_t
{
	UINT8 cmd,param_count,cursor_on,status,irq_mask;
	UINT8 param[8][5];
	UINT8 inverse;
};

struct mouse_t
{
	UINT8 phase;
	UINT8 x,y;
	attotime time;
};

class pc8801_state : public driver_device
{
public:
	pc8801_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_fdccpu(*this, "fdccpu"),
			m_pic(*this, I8214_TAG),
			m_rtc(*this, UPD1990A_TAG),
			m_cassette(*this, "cassette"),
			m_beeper(*this, "beeper"),
			m_opna(*this, "opna"),
			m_opn(*this, "opn"),
			m_palette(*this, "palette")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_fdccpu;
	optional_device<i8214_device> m_pic;
	required_device<upd1990a_device> m_rtc;
	required_device<cassette_image_device> m_cassette;
	required_device<beep_device> m_beeper;
	required_device<ym2608_device> m_opna;
	required_device<ym2203_device> m_opn;
	required_device<palette_device> m_palette;

	UINT8 *m_work_ram;
	UINT8 *m_hi_work_ram;
	UINT8 *m_ext_work_ram;
	UINT8 *m_gvram;
	UINT8 *m_n80rom;
	UINT8 *m_n88rom;
	UINT8 *m_kanji_rom;
	UINT8 *m_cg_rom;

	UINT8 m_i8255_0_pc;
	UINT8 m_i8255_1_pc;
	UINT8 m_fdc_irq_opcode;
	UINT8 m_ext_rom_bank;
	UINT8 m_gfx_ctrl;
	UINT8 m_vram_sel;
	UINT8 m_misc_ctrl;
	UINT8 m_device_ctrl_data;
	UINT8 m_window_offset_bank;
	UINT8 m_layer_mask;
	UINT16 m_dma_counter[4];
	UINT16 m_dma_address[4];
	UINT8 m_alu_reg[3];
	UINT8 m_dmac_mode;
	UINT8 m_alu_ctrl1;
	UINT8 m_alu_ctrl2;
	UINT8 m_extram_mode;
	UINT8 m_extram_bank;
	UINT8 m_txt_width;
	UINT8 m_txt_color;
#ifdef USE_PROPER_I8214
	UINT8 m_timer_irq_mask;
	UINT8 m_vblank_irq_mask;
	UINT8 m_sound_irq_mask;
	UINT8 m_int_state;
#else
	UINT8 m_i8214_irq_level;
	UINT8 m_vrtc_irq_mask;
	UINT8 m_vrtc_irq_latch;
	UINT8 m_timer_irq_mask;
	UINT8 m_timer_irq_latch;
	UINT8 m_sound_irq_mask;
	UINT8 m_sound_irq_latch;
	UINT8 m_sound_irq_pending;
#endif
	UINT8 m_has_clock_speed;
	UINT8 m_clock_setting;
	UINT8 m_baudrate_val;
	UINT8 m_has_dictionary;
	UINT8 m_dic_ctrl;
	UINT8 m_dic_bank;
	UINT8 m_has_cdrom;
	UINT8 m_cdrom_reg[0x10];
	crtc_t m_crtc;
	mouse_t m_mouse;
	struct { UINT8 r, g, b; } m_palram[8];
	UINT8 m_dmac_ff;
	UINT32 m_knj_addr[2];
	UINT32 m_extram_size;
	UINT8 m_has_opna;

	DECLARE_READ8_MEMBER(pc8801_alu_r);
	DECLARE_WRITE8_MEMBER(pc8801_alu_w);
	DECLARE_READ8_MEMBER(pc8801_wram_r);
	DECLARE_WRITE8_MEMBER(pc8801_wram_w);
	DECLARE_READ8_MEMBER(pc8801_ext_wram_r);
	DECLARE_WRITE8_MEMBER(pc8801_ext_wram_w);
	DECLARE_READ8_MEMBER(pc8801_nbasic_rom_r);
	DECLARE_READ8_MEMBER(pc8801_n88basic_rom_r);
	DECLARE_READ8_MEMBER(pc8801_gvram_r);
	DECLARE_WRITE8_MEMBER(pc8801_gvram_w);
	DECLARE_READ8_MEMBER(pc8801_high_wram_r);
	DECLARE_WRITE8_MEMBER(pc8801_high_wram_w);
	DECLARE_READ8_MEMBER(pc8801ma_dic_r);
	DECLARE_READ8_MEMBER(pc8801_cdbios_rom_r);
	DECLARE_READ8_MEMBER(pc8801_mem_r);
	DECLARE_WRITE8_MEMBER(pc8801_mem_w);
	DECLARE_READ8_MEMBER(pc8801_ctrl_r);
	DECLARE_WRITE8_MEMBER(pc8801_ctrl_w);
	DECLARE_READ8_MEMBER(pc8801_ext_rom_bank_r);
	DECLARE_WRITE8_MEMBER(pc8801_ext_rom_bank_w);
	DECLARE_WRITE8_MEMBER(pc8801_gfx_ctrl_w);
	DECLARE_READ8_MEMBER(pc8801_vram_select_r);
	DECLARE_WRITE8_MEMBER(pc8801_vram_select_w);
	DECLARE_WRITE8_MEMBER(i8214_irq_level_w);
	DECLARE_WRITE8_MEMBER(i8214_irq_mask_w);
	DECLARE_WRITE8_MEMBER(pc8801_irq_level_w);
	DECLARE_WRITE8_MEMBER(pc8801_irq_mask_w);
	DECLARE_READ8_MEMBER(pc8801_window_bank_r);
	DECLARE_WRITE8_MEMBER(pc8801_window_bank_w);
	DECLARE_WRITE8_MEMBER(pc8801_window_bank_inc_w);
	DECLARE_READ8_MEMBER(pc8801_misc_ctrl_r);
	DECLARE_WRITE8_MEMBER(pc8801_misc_ctrl_w);
	DECLARE_WRITE8_MEMBER(pc8801_bgpal_w);
	DECLARE_WRITE8_MEMBER(pc8801_palram_w);
	DECLARE_WRITE8_MEMBER(pc8801_layer_masking_w);
	DECLARE_READ8_MEMBER(pc8801_crtc_param_r);
	DECLARE_WRITE8_MEMBER(pc88_crtc_param_w);
	DECLARE_READ8_MEMBER(pc8801_crtc_status_r);
	DECLARE_WRITE8_MEMBER(pc88_crtc_cmd_w);
	DECLARE_READ8_MEMBER(pc8801_dmac_r);
	DECLARE_WRITE8_MEMBER(pc8801_dmac_w);
	DECLARE_READ8_MEMBER(pc8801_dmac_status_r);
	DECLARE_WRITE8_MEMBER(pc8801_dmac_mode_w);
	DECLARE_READ8_MEMBER(pc8801_extram_mode_r);
	DECLARE_WRITE8_MEMBER(pc8801_extram_mode_w);
	DECLARE_READ8_MEMBER(pc8801_extram_bank_r);
	DECLARE_WRITE8_MEMBER(pc8801_extram_bank_w);
	DECLARE_WRITE8_MEMBER(pc8801_alu_ctrl1_w);
	DECLARE_WRITE8_MEMBER(pc8801_alu_ctrl2_w);
	DECLARE_WRITE8_MEMBER(pc8801_pcg8100_w);
	DECLARE_WRITE8_MEMBER(pc8801_txt_cmt_ctrl_w);
	DECLARE_READ8_MEMBER(pc8801_kanji_r);
	DECLARE_WRITE8_MEMBER(pc8801_kanji_w);
	DECLARE_READ8_MEMBER(pc8801_kanji_lv2_r);
	DECLARE_WRITE8_MEMBER(pc8801_kanji_lv2_w);
	DECLARE_WRITE8_MEMBER(pc8801_dic_bank_w);
	DECLARE_WRITE8_MEMBER(pc8801_dic_ctrl_w);
	DECLARE_READ8_MEMBER(pc8801_cdrom_r);
	DECLARE_WRITE8_MEMBER(pc8801_cdrom_w);
	DECLARE_READ8_MEMBER(pc8801_cpuclock_r);
	DECLARE_READ8_MEMBER(pc8801_baudrate_r);
	DECLARE_WRITE8_MEMBER(pc8801_baudrate_w);
	DECLARE_WRITE8_MEMBER(pc8801_rtc_w);
	DECLARE_WRITE8_MEMBER(upd765_mc_w);
	DECLARE_READ8_MEMBER(upd765_tc_r);
	DECLARE_WRITE8_MEMBER(fdc_irq_vector_w);
	DECLARE_WRITE8_MEMBER(fdc_drive_mode_w);
	DECLARE_WRITE_LINE_MEMBER(txdata_callback);
	DECLARE_WRITE_LINE_MEMBER(rxrdy_w);
	DECLARE_READ8_MEMBER(pc8801_sound_board_r);
	DECLARE_WRITE8_MEMBER(pc8801_sound_board_w);
	DECLARE_READ8_MEMBER(pc8801_opna_r);
	DECLARE_WRITE8_MEMBER(pc8801_opna_w);
	DECLARE_READ8_MEMBER(pc8801_unk_r);
	DECLARE_WRITE8_MEMBER(pc8801_unk_w);

	UINT8 pc8801_pixel_clock(void);
	void pc8801_dynamic_res_change(void);
	void draw_bitmap_3bpp(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void draw_bitmap_1bpp(bitmap_ind16 &bitmap,const rectangle &cliprect);
	UINT8 calc_cursor_pos(int x,int y,int yi);
	UINT8 extract_text_attribute(UINT32 address,int x, UINT8 width, UINT8 &non_special);
	void pc8801_draw_char(bitmap_ind16 &bitmap,int x,int y,int pal,UINT8 gfx_mode,UINT8 reverse,UINT8 secret,
							UINT8 blink,UINT8 upper,UINT8 lower,int y_size,int width, UINT8 non_special);
	void draw_text(bitmap_ind16 &bitmap,int y_size, UINT8 width);
	void fdc_irq_w(bool state);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_PALETTE_INIT(pc8801);
protected:

	virtual void video_start();
	virtual void machine_start();
	virtual void machine_reset();
public:
	DECLARE_MACHINE_RESET(pc8801_clock_speed);
	DECLARE_MACHINE_RESET(pc8801_dic);
	DECLARE_MACHINE_RESET(pc8801_cdrom);
	INTERRUPT_GEN_MEMBER(pc8801_vrtc_irq);
	TIMER_CALLBACK_MEMBER(pc8801fd_upd765_tc_to_zero);
	TIMER_DEVICE_CALLBACK_MEMBER(pc8801_rtc_irq);
	DECLARE_READ8_MEMBER(cpu_8255_c_r);
	DECLARE_WRITE8_MEMBER(cpu_8255_c_w);
	DECLARE_READ8_MEMBER(fdc_8255_c_r);
	DECLARE_WRITE8_MEMBER(fdc_8255_c_w);
	DECLARE_WRITE_LINE_MEMBER(pic_int_w);
	DECLARE_WRITE_LINE_MEMBER(pic_enlg_w);
	DECLARE_READ8_MEMBER(opn_porta_r);
	DECLARE_READ8_MEMBER(opn_portb_r);
	IRQ_CALLBACK_MEMBER(pc8801_irq_callback);
	void pc8801_raise_irq(UINT8 irq,UINT8 state);
	DECLARE_WRITE_LINE_MEMBER(pc8801_sound_irq);
};


/*
CRTC command params:
0. CRTC reset

[0] *--- ---- <unknown>
[0] -xxx xxxx screen columns (+2)

[1] xx-- ---- blink speed (in frame unit) (+1, << 3)
[1] --xx xxxx screen lines (+1)

[2] x--- ---- "skip line"
[2] -x-- ---- cursor style (reverse on / underscore off)
[2] --x- ---- cursor blink on/off
[2] ---x xxxx lines per character (+1)

[3] xxx- ---- Vertical Retrace (+1)
[3] ---x xxxx Horizontal Retrace (+2)

[4] x--- ---- attribute not separate flag
[4] -x-- ---- attribute color flag
[4] --x- ---- attribute not special flag (invalidates next register)
[4] ---x xxxx attribute size (+1)
*/

#define screen_width ((m_crtc.param[0][0] & 0x7f) + 2) * 8

#define blink_speed ((((m_crtc.param[0][1] & 0xc0) >> 6) + 1) << 3)
#define screen_height ((m_crtc.param[0][1] & 0x3f) + 1)

#define lines_per_char ((m_crtc.param[0][2] & 0x1f) + 1)

#define vretrace (((m_crtc.param[0][3] & 0xe0) >> 5) + 1)
#define hretrace ((m_crtc.param[0][3] & 0x1f) + 2) * 8

#define text_color_flag ((m_crtc.param[0][4] & 0xe0) == 0x40)
//#define monitor_24KHz ((m_gfx_ctrl & 0x19) == 0x08) /* TODO: this is most likely to be WRONG */

void pc8801_state::video_start()
{
}

void pc8801_state::draw_bitmap_3bpp(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	int x,y,xi;
	UINT32 count;
	UINT16 y_size;
	UINT16 y_double;

	count = 0;

	y_double = (pc8801_pixel_clock());
	y_size = (y_double+1) * 200;

	for(y=0;y<y_size;y+=(y_double+1))
	{
		for(x=0;x<640;x+=8)
		{
			for(xi=0;xi<8;xi++)
			{
				int pen;

				pen = 0;

				/* note: layer masking doesn't occur in 3bpp mode, Bug Attack relies on this */
				pen |= ((m_gvram[count+0x0000] >> (7-xi)) & 1) << 0;
				pen |= ((m_gvram[count+0x4000] >> (7-xi)) & 1) << 1;
				pen |= ((m_gvram[count+0x8000] >> (7-xi)) & 1) << 2;

				if(y_double)
				{
					if(cliprect.contains(x+xi, y+0))
						bitmap.pix16(y+0, x+xi) = m_palette->pen(pen & 7);

					if(cliprect.contains(x+xi, y+1))
						bitmap.pix16(y+1, x+xi) = m_palette->pen(pen & 7);
				}
				else
				{
					if(cliprect.contains(x+xi, y+0))
						bitmap.pix16(y, x+xi) = m_palette->pen(pen & 7);
				}
			}

			count++;
		}
	}
}

void pc8801_state::draw_bitmap_1bpp(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	int x,y,xi;
	UINT32 count;
	UINT8 color;
	UINT8 is_cursor;

	count = 0;
	color = (m_gfx_ctrl & 1) ? 7 & ((m_layer_mask ^ 0xe) >> 1) : 7;
	is_cursor = 0;

	for(y=0;y<200;y++)
	{
		for(x=0;x<640;x+=8)
		{
			if(!(m_gfx_ctrl & 1))
				is_cursor = calc_cursor_pos(x/8,y/lines_per_char,y & (lines_per_char-1));

			for(xi=0;xi<8;xi++)
			{
				int pen;

				pen = ((m_gvram[count+0x0000] >> (7-xi)) & 1);
				if(is_cursor)
					pen^=1;

				if((m_gfx_ctrl & 1))
				{
					if(cliprect.contains(x+xi, y*2+0))
						bitmap.pix16(y*2+0, x+xi) = m_palette->pen(pen ? color : 0);

					if(cliprect.contains(x+xi, y*2+1))
						bitmap.pix16(y*2+1, x+xi) = m_palette->pen(pen ? color : 0);
				}
				else
				{
					if(cliprect.contains(x+xi, y))
						bitmap.pix16(y, x+xi) = m_palette->pen(pen ? color : 0);
				}
			}

			count++;
		}
	}

	if(!(m_gfx_ctrl & 1)) // 400 lines
	{
		count = 0;

		for(y=200;y<400;y++)
		{
			for(x=0;x<640;x+=8)
			{
				if(!(m_gfx_ctrl & 1))
					is_cursor = calc_cursor_pos(x/8,y/lines_per_char,y & (lines_per_char-1));

				for(xi=0;xi<8;xi++)
				{
					int pen;

					pen = ((m_gvram[count+0x4000] >> (7-xi)) & 1);
					if(is_cursor)
						pen^=1;

					if(cliprect.contains(x+xi, y))
						bitmap.pix16(y, x+xi) = m_palette->pen(pen ? 7 : 0);
				}

				count++;
			}
		}
	}
}

UINT8 pc8801_state::calc_cursor_pos(int x,int y,int yi)
{
	if(!(m_crtc.cursor_on)) // don't bother if cursor is off
		return 0;

	if(x == m_crtc.param[4][0] && y == m_crtc.param[4][1]) /* check if position matches */
	{
		/* don't pass through if we are using underscore */
		if((!(m_crtc.param[0][2] & 0x40)) && yi != 7)
			return 0;

		/* finally check if blinking is currently active high */
		if(!(m_crtc.param[0][2] & 0x20))
			return 1;

		if(((machine().first_screen()->frame_number() / blink_speed) & 1) == 0)
			return 1;

		return 0;
	}

	return 0;
}



UINT8 pc8801_state::extract_text_attribute(UINT32 address,int x, UINT8 width, UINT8 &non_special)
{
	UINT8 *vram = m_work_ram;
	int i;
	int fifo_size;
	int offset;

	non_special = 0;
	if(m_crtc.param[0][4] & 0x80)
	{
		popmessage("Using non-separate mode for text tilemap, contact MESSdev");
		return 0;
	}

	fifo_size = (m_crtc.param[0][4] & 0x20) ? 0 : ((m_crtc.param[0][4] & 0x1f) + 1);

	if(fifo_size == 0)
	{
		non_special = 1;
		return (text_color_flag) ? 0xe8 : 0;
	}

	/* TODO: correct or hack-ish? Certainly having 0 as a attribute X is weird in any case. */
	offset = (vram[address] == 0) ? 2 : 0;

	for(i=0;i<fifo_size;i++)
	{
		if(x < vram[address+offset])
		{
			return vram[address+1];
		}
		else
			address+=2;
	}

	return vram[address-3+offset];
}

void pc8801_state::pc8801_draw_char(bitmap_ind16 &bitmap,int x,int y,int pal,UINT8 gfx_mode,UINT8 reverse,UINT8 secret,UINT8 blink,UINT8 upper,UINT8 lower,int y_size,int width, UINT8 non_special)
{
	int xi,yi;
	UINT8 *vram = m_work_ram;
	UINT8 is_cursor;
	UINT8 y_height, y_double;
	UINT8 y_step;

	y_height = lines_per_char;
	y_double = (pc8801_pixel_clock());
	y_step = (non_special) ? 80 : 120; // trusted by Elthlead
	is_cursor = 0;

	for(yi=0;yi<y_height;yi++)
	{
		if(m_gfx_ctrl & 1)
			is_cursor = calc_cursor_pos(x,y,yi);

		for(xi=0;xi<8;xi++)
		{
			int res_x,res_y;
			int tile;
			int color;

			{
				tile = vram[x+(y*y_step)+m_dma_address[2]];

				res_x = x*8+xi*(width+1);
				res_y = y*y_height+yi;

				if(!machine().first_screen()->visible_area().contains(res_x, res_y))
					continue;

				if(gfx_mode)
				{
					UINT8 mask;

					mask = (xi & 4) ? 0x10 : 0x01;
					mask <<= ((yi & (0x6 << y_double)) >> (1+y_double));
					color = (tile & mask) ? pal : -1;
				}
				else
				{
					UINT8 char_data;
					UINT8 blink_mask;

					blink_mask = 0;
					if(blink && ((machine().first_screen()->frame_number() / blink_speed) & 3) == 1)
						blink_mask = 1;

					if(yi >= (1 << (y_double+3)) || secret || blink_mask)
						char_data = 0;
					else
						char_data = (m_cg_rom[tile*8+(yi >> y_double)] >> (7-xi)) & 1;

					if(yi == 0 && upper)
						char_data = 1;

					if(yi == y_height && lower)
						char_data = 1;

					if(is_cursor)
						char_data^=1;

					if(reverse)
						char_data^=1;

					color = char_data ? pal : -1;
				}

				if(color != -1)
				{
					bitmap.pix16(res_y, res_x) = m_palette->pen(color);
					if(width)
					{
						if(!machine().first_screen()->visible_area().contains(res_x+1, res_y))
							continue;

						bitmap.pix16(res_y, res_x+1) = m_palette->pen(color);
					}
				}
			}
		}
	}
}

void pc8801_state::draw_text(bitmap_ind16 &bitmap,int y_size, UINT8 width)
{
	int x,y;
	UINT8 attr;
	UINT8 reverse;
	UINT8 gfx_mode;
	UINT8 secret;
	UINT8 upper;
	UINT8 lower;
	UINT8 blink;
	int pal;
	UINT8 non_special;

	for(y=0;y<y_size;y++)
	{
		for(x=0;x<80;x++)
		{
			if(x & 1 && !width)
				continue;

			attr = extract_text_attribute((((y*120)+80+m_dma_address[2]) & 0xffff),(x),width,non_special);

			if(text_color_flag && (attr & 8)) // color mode
			{
				pal =  ((attr & 0xe0) >> 5);
				gfx_mode = (attr & 0x10) >> 4;
				reverse = 0;
				secret = 0;
				upper = 0;
				lower = 0;
				blink = 0;
				pal|=8; //text pal bank
			}
			else // monochrome
			{
				pal = 7; /* TODO: Bishoujo Baseball Gakuen Pasoket logo wants this to be black somehow ... */
				gfx_mode = (attr & 0x80) >> 7;
				reverse = (attr & 4) >> 2;
				secret = (attr & 1);
				upper = (attr & 0x10) >> 4;
				lower = (attr & 0x20) >> 5;
				blink = (attr & 2) >> 1;
				pal|=8; //text pal bank
				reverse ^= m_crtc.inverse;

				if(attr & 0x80)
					popmessage("Warning: mono gfx mode enabled, contact MESSdev");

			}

			pc8801_draw_char(bitmap,x,y,pal,gfx_mode,reverse,secret,blink,upper,lower,y_size,!width,non_special);
		}
	}
}

UINT32 pc8801_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->pen(0), cliprect);

//  popmessage("%04x %04x %02x",m_dma_address[2],m_dma_counter[2],m_dmac_mode);

	if(m_gfx_ctrl & 8)
	{
		if(m_gfx_ctrl & 0x10)
			draw_bitmap_3bpp(bitmap,cliprect);
		else
			draw_bitmap_1bpp(bitmap,cliprect);
	}

	//popmessage("%02x %02x %02x %02x %02x",state->m_layer_mask,state->m_dmac_mode,state->m_crtc.status,state->m_crtc.irq_mask,state->m_gfx_ctrl);

	if(!(m_layer_mask & 1) && m_dmac_mode & 4 && m_crtc.status & 0x10 && m_crtc.irq_mask == 3)
	{
		//popmessage("%02x %02x",m_crtc.param[0][0],m_crtc.param[0][4]);

		draw_text(bitmap,screen_height,m_txt_width);
	}

	return 0;
}

READ8_MEMBER(pc8801_state::pc8801_alu_r)
{
	int i;
	UINT8 b,r,g;

	/* store data to ALU regs */
	for(i=0;i<3;i++)
		m_alu_reg[i] = m_gvram[i*0x4000 + offset];

	b = m_gvram[offset + 0x0000];
	r = m_gvram[offset + 0x4000];
	g = m_gvram[offset + 0x8000];
	if(!(m_alu_ctrl2 & 1)) { b^=0xff; }
	if(!(m_alu_ctrl2 & 2)) { r^=0xff; }
	if(!(m_alu_ctrl2 & 4)) { g^=0xff; }

	return b & r & g;
}

WRITE8_MEMBER(pc8801_state::pc8801_alu_w)
{
	int i;

	switch(m_alu_ctrl2 & 0x30) // alu write mode
	{
		case 0x00: //logic operation
		{
			UINT8 logic_op;

			for(i=0;i<3;i++)
			{
				logic_op = (m_alu_ctrl1 & (0x11 << i)) >> i;

				switch(logic_op)
				{
					case 0x00: { m_gvram[i*0x4000 + offset] &= ~data; } break;
					case 0x01: { m_gvram[i*0x4000 + offset] |= data; } break;
					case 0x10: { m_gvram[i*0x4000 + offset] ^= data; } break;
					case 0x11: break; // NOP
				}
			}
		}
		break;

		case 0x10: // restore data from ALU regs
		{
			for(i=0;i<3;i++)
				m_gvram[i*0x4000 + offset] = m_alu_reg[i];
		}
		break;

		case 0x20: // swap ALU reg 1 into R GVRAM
			m_gvram[0x0000 + offset] = m_alu_reg[1];
			break;

		case 0x30: // swap ALU reg 0 into B GVRAM
			m_gvram[0x4000 + offset] = m_alu_reg[0];
			break;
	}
}


READ8_MEMBER(pc8801_state::pc8801_wram_r)
{
	return m_work_ram[offset];
}

WRITE8_MEMBER(pc8801_state::pc8801_wram_w)
{
	m_work_ram[offset] = data;
}

READ8_MEMBER(pc8801_state::pc8801_ext_wram_r)
{
	if(offset < m_extram_size)
		return m_ext_work_ram[offset];

	return 0xff;
}

WRITE8_MEMBER(pc8801_state::pc8801_ext_wram_w)
{
	if(offset < m_extram_size)
		m_ext_work_ram[offset] = data;
}

READ8_MEMBER(pc8801_state::pc8801_nbasic_rom_r)
{
	return m_n80rom[offset];
}

READ8_MEMBER(pc8801_state::pc8801_n88basic_rom_r)
{
	return m_n88rom[offset];
}

READ8_MEMBER(pc8801_state::pc8801_gvram_r)
{
	return m_gvram[offset];
}

WRITE8_MEMBER(pc8801_state::pc8801_gvram_w)
{
	m_gvram[offset] = data;
}

READ8_MEMBER(pc8801_state::pc8801_high_wram_r)
{
	return m_hi_work_ram[offset];
}

WRITE8_MEMBER(pc8801_state::pc8801_high_wram_w)
{
	m_hi_work_ram[offset] = data;
}

READ8_MEMBER(pc8801_state::pc8801ma_dic_r)
{
	UINT8 *dic_rom = memregion("dictionary")->base();

	return dic_rom[offset];
}

READ8_MEMBER(pc8801_state::pc8801_cdbios_rom_r)
{
	UINT8 *cdrom_bios = memregion("cdrom")->base();

	return cdrom_bios[offset];
}

READ8_MEMBER(pc8801_state::pc8801_mem_r)
{
	if(offset <= 0x7fff)
	{
		if(m_extram_mode & 1)
			return pc8801_ext_wram_r(space,offset | (m_extram_bank * 0x8000));

		if(m_gfx_ctrl & 2)
			return pc8801_wram_r(space,offset);

		if(m_has_cdrom && m_cdrom_reg[9] & 0x10)
			return pc8801_cdbios_rom_r(space,(offset & 0x7fff) | ((m_gfx_ctrl & 4) ? 0x8000 : 0x0000));

		if(m_gfx_ctrl & 4)
			return pc8801_nbasic_rom_r(space,offset);

		if(offset >= 0x6000 && offset <= 0x7fff && ((m_ext_rom_bank & 1) == 0))
			return pc8801_n88basic_rom_r(space,0x8000 + (offset & 0x1fff) + (0x2000 * (m_misc_ctrl & 3)));

		return pc8801_n88basic_rom_r(space,offset);
	}
	else if(offset >= 0x8000 && offset <= 0x83ff) // work RAM window
	{
		UINT32 window_offset;

		if(m_gfx_ctrl & 6) //wram read select or n basic select banks this as normal wram
			return pc8801_wram_r(space,offset);

		window_offset = (offset & 0x3ff) + (m_window_offset_bank << 8);

		if(((window_offset & 0xf000) == 0xf000) && (m_misc_ctrl & 0x10))
			printf("Read from 0xf000 - 0xffff window offset\n"); //accessed by Castle Excellent, no noticeable quirk

		if(((window_offset & 0xf000) == 0xf000) && (m_misc_ctrl & 0x10))
			return pc8801_high_wram_r(space,window_offset & 0xfff);

		return pc8801_wram_r(space,window_offset);
	}
	else if(offset >= 0x8400 && offset <= 0xbfff)
	{
		return pc8801_wram_r(space,offset);
	}
	else if(offset >= 0xc000 && offset <= 0xffff)
	{
		if(m_has_dictionary && m_dic_ctrl)
			return pc8801ma_dic_r(space,(offset & 0x3fff) + ((m_dic_bank & 0x1f) * 0x4000));

		if(m_misc_ctrl & 0x40)
		{
			if(!space.debugger_access())
				m_vram_sel = 3;

			if(m_alu_ctrl2 & 0x80)
				return pc8801_alu_r(space,offset & 0x3fff);
		}

		if(m_vram_sel == 3)
		{
			if(offset >= 0xf000 && offset <= 0xffff && (m_misc_ctrl & 0x10))
				return pc8801_high_wram_r(space,offset & 0xfff);

			return pc8801_wram_r(space,offset);
		}

		return pc8801_gvram_r(space,(offset & 0x3fff) + (0x4000 * m_vram_sel));
	}

	return 0xff;
}

WRITE8_MEMBER(pc8801_state::pc8801_mem_w)
{
	if(offset <= 0x7fff)
	{
		if(m_extram_mode & 0x10)
			pc8801_ext_wram_w(space,offset | (m_extram_bank * 0x8000),data);
		else
			pc8801_wram_w(space,offset,data);

		return;
	}
	else if(offset >= 0x8000 && offset <= 0x83ff)
	{
		if(m_gfx_ctrl & 6) //wram read select or n basic select banks this as normal wram
			pc8801_wram_w(space,offset,data);
		else
		{
			UINT32 window_offset;

			window_offset = (offset & 0x3ff) + (m_window_offset_bank << 8);

			if(((window_offset & 0xf000) == 0xf000) && (m_misc_ctrl & 0x10))
				printf("Write to 0xf000 - 0xffff window offset\n"); //accessed by Castle Excellent, no noticeable quirk

			if(((window_offset & 0xf000) == 0xf000) && (m_misc_ctrl & 0x10))
				pc8801_high_wram_w(space,window_offset & 0xfff,data);
			else
				pc8801_wram_w(space,window_offset,data);
		}

		return;
	}
	else if(offset >= 0x8400 && offset <= 0xbfff)
	{
		pc8801_wram_w(space,offset,data);
		return;
	}
	else if(offset >= 0xc000 && offset <= 0xffff)
	{
		if(m_misc_ctrl & 0x40)
		{
			if(!space.debugger_access())
				m_vram_sel = 3;

			if(m_alu_ctrl2 & 0x80)
			{
				pc8801_alu_w(space,offset & 0x3fff,data);
				return;
			}
		}

		if(m_vram_sel == 3)
		{
			if(offset >= 0xf000 && offset <= 0xffff && (m_misc_ctrl & 0x10))
			{
				pc8801_high_wram_w(space,offset & 0xfff,data);
				return;
			}

			pc8801_wram_w(space,offset,data);
			return;
		}

		pc8801_gvram_w(space,(offset & 0x3fff) + (0x4000 * m_vram_sel),data);
		return;
	}
}

static ADDRESS_MAP_START( pc8801_mem, AS_PROGRAM, 8, pc8801_state )
	AM_RANGE(0x0000, 0xffff) AM_READWRITE(pc8801_mem_r,pc8801_mem_w)
ADDRESS_MAP_END

READ8_MEMBER(pc8801_state::pc8801_ctrl_r)
{
	/*
	11-- ----
	--x- ---- vrtc
	---x ---- calendar CDO
	---- x--- fdc auto-boot DIP-SW
	---- -x-- (RS-232C related)
	---- --x- monitor refresh rate DIP-SW
	---- ---x (pbsy?)
	*/
	return ioport("CTRL")->read();
}

WRITE8_MEMBER(pc8801_state::pc8801_ctrl_w)
{
	/*
	x--- ---- SING (buzzer mask?)
	-x-- ---- mouse latch (JOP1, routes on OPN sound port A)
	--x- ---- beeper
	---x ---- ghs mode
	---- x--- crtc i/f sync mode
	---- -x-- upd1990a clock bit
	---- --x- upd1990a strobe bit
	---- ---x printer strobe
	*/

	m_rtc->stb_w((data & 2) >> 1);
	m_rtc->clk_w((data & 4) >> 2);

	if(((m_device_ctrl_data & 0x20) == 0x00) && ((data & 0x20) == 0x20))
		m_beeper->set_state(1);

	if(((m_device_ctrl_data & 0x20) == 0x20) && ((data & 0x20) == 0x00))
		m_beeper->set_state(0);

	if((m_device_ctrl_data & 0x40) != (data & 0x40))
	{
		attotime new_time = machine().time();

		if(m_mouse.phase == 0)
		{
			m_mouse.x = ioport("MOUSEX")->read();
			m_mouse.y = ioport("MOUSEY")->read();
		}

		if(data & 0x40 && (new_time - m_mouse.time) > attotime::from_hz(900))
		{
			m_mouse.phase = 0;
		}
		else
		{
			m_mouse.phase++;
			m_mouse.phase &= 3;
		}

		m_mouse.time = machine().time();
	}

	/* TODO: is SING a buzzer mask? Bastard Special relies on this ... */
	if(m_device_ctrl_data & 0x80)
		m_beeper->set_state(0);

	m_device_ctrl_data = data;
}

READ8_MEMBER(pc8801_state::pc8801_ext_rom_bank_r)
{
	return m_ext_rom_bank;
}

WRITE8_MEMBER(pc8801_state::pc8801_ext_rom_bank_w)
{
	m_ext_rom_bank = data;
}

UINT8 pc8801_state::pc8801_pixel_clock(void)
{
	int ysize = machine().first_screen()->height(); /* TODO: correct condition*/

	return (ysize >= 400);
}

void pc8801_state::pc8801_dynamic_res_change(void)
{
	rectangle visarea;
	int xsize,ysize,xvis,yvis;
	attoseconds_t refresh;;

	/* bail out if screen params aren't valid */
	if(!m_crtc.param[0][0] || !m_crtc.param[0][1] || !m_crtc.param[0][2] || !m_crtc.param[0][3])
		return;

	xvis = screen_width;
	yvis = screen_height * lines_per_char;
	xsize = screen_width + hretrace;
	ysize = screen_height * lines_per_char + vretrace * lines_per_char;

//  popmessage("H %d V %d (%d x %d) HR %d VR %d (%d %d)\n",xvis,yvis,screen_height,lines_per_char,hretrace,vretrace, xsize,ysize);

	visarea.set(0, xvis - 1, 0, yvis - 1);
	if(pc8801_pixel_clock())
		refresh = HZ_TO_ATTOSECONDS(PIXEL_CLOCK_24KHz) * (xsize) * ysize;
	else
		refresh = HZ_TO_ATTOSECONDS(PIXEL_CLOCK_15KHz) * (xsize) * ysize;

	machine().first_screen()->configure(xsize, ysize, visarea, refresh);
}

WRITE8_MEMBER(pc8801_state::pc8801_gfx_ctrl_w)
{
	/*
	--x- ---- ???
	---x ---- graphic color yes (1) / no (0)
	---- x--- graphic display yes (1) / no (0)
	---- -x-- Basic N (1) / N88 (0)
	---- --x- RAM select yes (1) / no (0)
	---- ---x VRAM 200 lines (1) / 400 lines (0) in 1bpp mode
	*/

	m_gfx_ctrl = data;

	pc8801_dynamic_res_change();
}

READ8_MEMBER(pc8801_state::pc8801_vram_select_r)
{
	return 0xf8 | ((m_vram_sel == 3) ? 0 : (1 << m_vram_sel));
}

WRITE8_MEMBER(pc8801_state::pc8801_vram_select_w)
{
	m_vram_sel = offset & 3;
}

#ifdef USE_PROPER_I8214

WRITE8_MEMBER(pc8801_state::i8214_irq_level_w)
{
	if(data & 8)
		m_pic->b_w(7);
	else
		m_pic->b_w(data & 0x07);
}

WRITE8_MEMBER(pc8801_state::i8214_irq_mask_w)
{
	m_timer_irq_mask = data & 1;
	m_vblank_irq_mask = data & 2;
}


#else
WRITE8_MEMBER(pc8801_state::pc8801_irq_level_w)
{
	if(data & 8)
		m_i8214_irq_level = 7;
	else
		m_i8214_irq_level = data & 7;

//  IRQ_LOG(("%02x LV\n",m_i8214_irq_level));
}


WRITE8_MEMBER(pc8801_state::pc8801_irq_mask_w)
{
	m_timer_irq_mask = data & 1;
	m_vrtc_irq_mask = data & 2;

	if(m_timer_irq_mask == 0)
		m_timer_irq_latch = 0;

	if(m_vrtc_irq_mask == 0)
		m_vrtc_irq_latch = 0;

	if(m_timer_irq_latch == 0 && m_vrtc_irq_latch == 0 && m_sound_irq_latch == 0)
		m_maincpu->set_input_line(0,CLEAR_LINE);

//  IRQ_LOG(("%02x MASK (%02x %02x)\n",data,m_timer_irq_latch,m_vrtc_irq_latch));

	//if(data & 4)
	//  printf("IRQ mask %02x\n",data);
}
#endif

READ8_MEMBER(pc8801_state::pc8801_window_bank_r)
{
	return m_window_offset_bank;
}

WRITE8_MEMBER(pc8801_state::pc8801_window_bank_w)
{
	m_window_offset_bank = data;
}

WRITE8_MEMBER(pc8801_state::pc8801_window_bank_inc_w)
{
	m_window_offset_bank++;
	m_window_offset_bank&=0xff;
}

READ8_MEMBER(pc8801_state::pc8801_misc_ctrl_r)
{
	return m_misc_ctrl;
}

WRITE8_MEMBER(pc8801_state::pc8801_misc_ctrl_w)
{
	/*
	x--- ---- sound irq mask, active low
	--x- ---- analog (1) / digital (0) palette select
	*/

	m_misc_ctrl = data;

	#ifdef USE_PROPER_I8214
	m_sound_irq_mask = ((data & 0x80) == 0);
	#else
	m_sound_irq_mask = ((data & 0x80) == 0);

	if(m_sound_irq_mask == 0)
		m_sound_irq_latch = 0;

	if(m_timer_irq_latch == 0 && m_vrtc_irq_latch == 0 && m_sound_irq_latch == 0)
		m_maincpu->set_input_line(0,CLEAR_LINE);

	if(m_sound_irq_mask && m_sound_irq_pending)
	{
		m_maincpu->set_input_line(0,HOLD_LINE);
		m_sound_irq_latch = 1;
		m_sound_irq_pending = 0;
	}

	#endif
}

WRITE8_MEMBER(pc8801_state::pc8801_bgpal_w)
{
	if(data)
		printf("BG Pal %02x\n",data);
}

WRITE8_MEMBER(pc8801_state::pc8801_palram_w)
{
	if(m_misc_ctrl & 0x20) //analog palette
	{
		if((data & 0x40) == 0)
		{
			m_palram[offset].b = data & 0x7;
			m_palram[offset].r = (data & 0x38) >> 3;
		}
		else
		{
			m_palram[offset].g = data & 0x7;
		}
	}
	else //digital palette
	{
		m_palram[offset].b = data & 1 ? 7 : 0;
		m_palram[offset].r = data & 2 ? 7 : 0;
		m_palram[offset].g = data & 4 ? 7 : 0;
	}

	m_palette->set_pen_color(offset, pal3bit(m_palram[offset].r), pal3bit(m_palram[offset].g), pal3bit(m_palram[offset].b));
}

WRITE8_MEMBER(pc8801_state::pc8801_layer_masking_w)
{
	/*
	---- x--- green gvram masked flag
	---- -x-- red gvram masked flag
	---- --x- blue gvram masked flag
	---- ---x text vram masked
	*/

	m_layer_mask = data;
}

READ8_MEMBER(pc8801_state::pc8801_crtc_param_r)
{
	printf("CRTC param reading\n");
	return 0xff;
}

WRITE8_MEMBER(pc8801_state::pc88_crtc_param_w)
{
	if(m_crtc.param_count < 5)
	{
		m_crtc.param[m_crtc.cmd][m_crtc.param_count] = data;
		if(m_crtc.cmd == 0)
			pc8801_dynamic_res_change();

		m_crtc.param_count++;
	}
}

READ8_MEMBER(pc8801_state::pc8801_crtc_status_r)
{
	/*
	---x ---- video enable
	---- x--- DMA is running
	---- -x-- special control character IRQ
	---- --x- indication end IRQ
	---- ---x light pen input
	*/

	return m_crtc.status;
}

#if 0
static const char *const crtc_command[] =
{
	"Reset / Stop Display",             // 0
	"Start Display",                    // 1
	"Set IRQ MASK",                     // 2
	"Read Light Pen",                   // 3
	"Load Cursor Position",             // 4
	"Reset IRQ",                        // 5
	"Reset Counters",                   // 6
	"Read Status"                       // 7
};
#endif

WRITE8_MEMBER(pc8801_state::pc88_crtc_cmd_w)
{
	m_crtc.cmd = (data & 0xe0) >> 5;
	m_crtc.param_count = 0;

	switch(m_crtc.cmd)
	{
		case 0:  // reset CRTC
			m_crtc.status &= (~0x16);
			break;
		case 1:  // start display
			m_crtc.status |= 0x10;
			m_crtc.status &= (~0x08);
			m_crtc.inverse = data & 1;

			if(data & 1) /* Ink Pot uses it, but I want another test case before removing this log */
				printf("CRTC inverse mode ON\n");
			break;
		case 2:  // set irq mask
			m_crtc.irq_mask = data & 3;
			break;
		case 3:  // read light pen
			m_crtc.status &= (~0x01);
			break;
		case 4:  // load cursor position ON/OFF
			m_crtc.cursor_on = data & 1;
			break;
		case 5:  // reset IRQ
		case 6:  // reset counters
			m_crtc.status &= (~0x06);
			break;
	}

	//if((data >> 5) != 4)
	//  printf("CRTC cmd %s polled %02x\n",crtc_command[data >> 5],data & 0x1f);
}

READ8_MEMBER(pc8801_state::pc8801_dmac_r)
{
	printf("DMAC R %08x\n",offset);
	return 0xff;
}

WRITE8_MEMBER(pc8801_state::pc8801_dmac_w)
{
	if(offset & 1)
		m_dma_counter[offset >> 1] = (m_dmac_ff) ? (m_dma_counter[offset >> 1]&0xff)|(data<<8) : (m_dma_counter[offset >> 1]&0xff00)|(data&0xff);
	else
		m_dma_address[offset >> 1] = (m_dmac_ff) ? (m_dma_address[offset >> 1]&0xff)|(data<<8) : (m_dma_address[offset >> 1]&0xff00)|(data&0xff);

	m_dmac_ff ^= 1;
}

READ8_MEMBER(pc8801_state::pc8801_dmac_status_r)
{
	printf("DMAC R STATUS\n");
	return 0xff;
}

WRITE8_MEMBER(pc8801_state::pc8801_dmac_mode_w)
{
	m_dmac_mode = data;
	m_dmac_ff = 0;

	if(data != 0xe4 && data != 0xa0 && data != 0xc4 && data != 0x80 && data != 0x00)
		printf("%02x DMAC mode\n",data);
}

READ8_MEMBER(pc8801_state::pc8801_extram_mode_r)
{
	return (m_extram_mode ^ 0x11) | 0xee;
}

WRITE8_MEMBER(pc8801_state::pc8801_extram_mode_w)
{
	/*
	---x ---- Write EXT RAM access at 0x0000 - 0x7fff
	---- ---x Read EXT RAM access at 0x0000 - 0x7fff
	*/

	m_extram_mode = data & 0x11;
}

READ8_MEMBER(pc8801_state::pc8801_extram_bank_r)
{
	return m_extram_bank;
}

WRITE8_MEMBER(pc8801_state::pc8801_extram_bank_w)
{
	m_extram_bank = data;
}

WRITE8_MEMBER(pc8801_state::pc8801_alu_ctrl1_w)
{
	m_alu_ctrl1 = data;
}

WRITE8_MEMBER(pc8801_state::pc8801_alu_ctrl2_w)
{
	m_alu_ctrl2 = data;
}

WRITE8_MEMBER(pc8801_state::pc8801_pcg8100_w)
{
	if(data)
		printf("Write to PCG-8100 %02x %02x\n",offset,data);
}

WRITE8_MEMBER(pc8801_state::pc8801_txt_cmt_ctrl_w)
{
	/* bits 2 to 5 are cmt related */

	m_txt_width = data & 1;
	m_txt_color = data & 2;

	m_cassette->change_state(BIT(data,3) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
}


READ8_MEMBER(pc8801_state::pc8801_kanji_r)
{
	if((offset & 2) == 0)
		return m_kanji_rom[m_knj_addr[0]*2+((offset & 1) ^ 1)];

	return 0xff;
}

WRITE8_MEMBER(pc8801_state::pc8801_kanji_w)
{
	if((offset & 2) == 0)
		m_knj_addr[0] = ((offset & 1) == 0) ? ((m_knj_addr[0]&0xff00)|(data&0xff)) : ((m_knj_addr[0]&0x00ff)|(data<<8));
}

READ8_MEMBER(pc8801_state::pc8801_kanji_lv2_r)
{
	if((offset & 2) == 0)
		return m_kanji_rom[m_knj_addr[1]*2+((offset & 1) ^ 1)];

	return 0xff;
}

WRITE8_MEMBER(pc8801_state::pc8801_kanji_lv2_w)
{
	if((offset & 2) == 0)
		m_knj_addr[1] = ((offset & 1) == 0) ? ((m_knj_addr[1]&0xff00)|(data&0xff)) : ((m_knj_addr[1]&0x00ff)|(data<<8));
}

WRITE8_MEMBER(pc8801_state::pc8801_dic_bank_w)
{
	printf("JISHO BANK = %02x\n",data);
	if(m_has_dictionary)
		m_dic_bank = data  & 0x1f;
}

WRITE8_MEMBER(pc8801_state::pc8801_dic_ctrl_w)
{
	printf("JISHO CTRL = %02x\n",data);
	if(m_has_dictionary)
		m_dic_ctrl = (data ^ 1) & 1;
}

READ8_MEMBER(pc8801_state::pc8801_cdrom_r)
{
	//printf("CD-ROM read [%02x]\n",offset);

	//if(m_has_cdrom)
	//  return m_cdrom_reg[offset];

	return 0xff;
}

WRITE8_MEMBER(pc8801_state::pc8801_cdrom_w)
{
	/*
	[9] ---x ---- CD-ROM BIOS bank
	    ---- ---x CD-ROM E-ROM bank (?)
	*/
	//printf("CD-ROM write %02x -> [%02x]\n",data,offset);

	if(m_has_cdrom)
		m_cdrom_reg[offset] = data;
}

READ8_MEMBER(pc8801_state::pc8801_cpuclock_r)
{
	if(m_has_clock_speed)
		return 0x10 | m_clock_setting;

	return 0xff;
}

READ8_MEMBER(pc8801_state::pc8801_baudrate_r)
{
	if(m_has_clock_speed)
		return 0xf0 | m_baudrate_val;

	return 0xff;
}

WRITE8_MEMBER(pc8801_state::pc8801_baudrate_w)
{
	if(m_has_clock_speed)
		m_baudrate_val = data & 0xf;
}

WRITE8_MEMBER(pc8801_state::pc8801_rtc_w)
{
	m_rtc->c0_w((data & 1) >> 0);
	m_rtc->c1_w((data & 2) >> 1);
	m_rtc->c2_w((data & 4) >> 2);
	m_rtc->data_in_w((data & 8) >> 3);

	/* TODO: remaining bits */
}

READ8_MEMBER(pc8801_state::pc8801_sound_board_r)
{
	if(m_has_opna)
		return m_opna->read(space, offset);

	return (offset & 2) ? 0xff : m_opn->read(space, offset);
}

WRITE8_MEMBER(pc8801_state::pc8801_sound_board_w)
{
	if(m_has_opna)
		m_opna->write(space, offset,data);
	else if((offset & 2) == 0)
		m_opn->write(space, offset, data);
}

READ8_MEMBER(pc8801_state::pc8801_opna_r)
{
	if(m_has_opna && (offset & 2) == 0)
		return m_opna->read(space, (offset & 1) | ((offset & 4) >> 1));

	return 0xff;
}

WRITE8_MEMBER(pc8801_state::pc8801_opna_w)
{
	if(m_has_opna && (offset & 2) == 0)
		m_opna->write(space, (offset & 1) | ((offset & 4) >> 1),data);
	else if(m_has_opna && offset == 2)
	{
		m_sound_irq_mask = ((data & 0x80) == 0);

		if(m_sound_irq_mask == 0)
			m_sound_irq_latch = 0;

		if(m_timer_irq_latch == 0 && m_vrtc_irq_latch == 0 && m_sound_irq_latch == 0)
			m_maincpu->set_input_line(0,CLEAR_LINE);

		if(m_sound_irq_mask && m_sound_irq_pending)
		{
			m_maincpu->set_input_line(0,HOLD_LINE);
			m_sound_irq_latch = 1;
			m_sound_irq_pending = 0;
		}
	}
}

READ8_MEMBER(pc8801_state::pc8801_unk_r)
{
	printf("Read port 0x33\n");
	return 0xff;
}

WRITE8_MEMBER(pc8801_state::pc8801_unk_w)
{
	printf("Write port 0x33\n");
}

static ADDRESS_MAP_START( pc8801_io, AS_IO, 8, pc8801_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x00) AM_READ_PORT("KEY0")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("KEY1")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("KEY2")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("KEY3")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("KEY4")
	AM_RANGE(0x05, 0x05) AM_READ_PORT("KEY5")
	AM_RANGE(0x06, 0x06) AM_READ_PORT("KEY6")
	AM_RANGE(0x07, 0x07) AM_READ_PORT("KEY7")
	AM_RANGE(0x08, 0x08) AM_READ_PORT("KEY8")
	AM_RANGE(0x09, 0x09) AM_READ_PORT("KEY9")
	AM_RANGE(0x0a, 0x0a) AM_READ_PORT("KEY10")
	AM_RANGE(0x0b, 0x0b) AM_READ_PORT("KEY11")
	AM_RANGE(0x0c, 0x0c) AM_READ_PORT("KEY12")
	AM_RANGE(0x0d, 0x0d) AM_READ_PORT("KEY13")
	AM_RANGE(0x0e, 0x0e) AM_READ_PORT("KEY14")
	AM_RANGE(0x0f, 0x0f) AM_READ_PORT("KEY15")
	AM_RANGE(0x00, 0x02) AM_WRITE(pc8801_pcg8100_w)
	AM_RANGE(0x10, 0x10) AM_WRITE(pc8801_rtc_w)
	AM_RANGE(0x20, 0x20) AM_MIRROR(0x0e) AM_DEVREADWRITE(I8251_TAG, i8251_device, data_r, data_w) /* RS-232C and CMT */
	AM_RANGE(0x21, 0x21) AM_MIRROR(0x0e) AM_DEVREADWRITE(I8251_TAG, i8251_device, status_r, control_w)
	AM_RANGE(0x30, 0x30) AM_READ_PORT("DSW1") AM_WRITE(pc8801_txt_cmt_ctrl_w)
	AM_RANGE(0x31, 0x31) AM_READ_PORT("DSW2") AM_WRITE(pc8801_gfx_ctrl_w)
	AM_RANGE(0x32, 0x32) AM_READWRITE(pc8801_misc_ctrl_r, pc8801_misc_ctrl_w)
	AM_RANGE(0x33, 0x33) AM_READWRITE(pc8801_unk_r,pc8801_unk_w)
	AM_RANGE(0x34, 0x34) AM_WRITE(pc8801_alu_ctrl1_w)
	AM_RANGE(0x35, 0x35) AM_WRITE(pc8801_alu_ctrl2_w)
	AM_RANGE(0x40, 0x40) AM_READWRITE(pc8801_ctrl_r, pc8801_ctrl_w)
	AM_RANGE(0x44, 0x47) AM_READWRITE(pc8801_sound_board_r,pc8801_sound_board_w) /* OPN / OPNA ports */
	AM_RANGE(0x50, 0x50) AM_READWRITE(pc8801_crtc_param_r, pc88_crtc_param_w)
	AM_RANGE(0x51, 0x51) AM_READWRITE(pc8801_crtc_status_r, pc88_crtc_cmd_w)
	AM_RANGE(0x52, 0x52) AM_WRITE(pc8801_bgpal_w)
	AM_RANGE(0x53, 0x53) AM_WRITE(pc8801_layer_masking_w)
	AM_RANGE(0x54, 0x5b) AM_WRITE(pc8801_palram_w)
	AM_RANGE(0x5c, 0x5c) AM_READ(pc8801_vram_select_r)
	AM_RANGE(0x5c, 0x5f) AM_WRITE(pc8801_vram_select_w)
	AM_RANGE(0x60, 0x67) AM_READWRITE(pc8801_dmac_r,pc8801_dmac_w)
	AM_RANGE(0x68, 0x68) AM_READWRITE(pc8801_dmac_status_r,pc8801_dmac_mode_w)
	AM_RANGE(0x6e, 0x6e) AM_READ(pc8801_cpuclock_r)
	AM_RANGE(0x6f, 0x6f) AM_READWRITE(pc8801_baudrate_r,pc8801_baudrate_w)
	AM_RANGE(0x70, 0x70) AM_READWRITE(pc8801_window_bank_r, pc8801_window_bank_w)
	AM_RANGE(0x71, 0x71) AM_READWRITE(pc8801_ext_rom_bank_r, pc8801_ext_rom_bank_w)
	AM_RANGE(0x78, 0x78) AM_WRITE(pc8801_window_bank_inc_w)
	AM_RANGE(0x90, 0x9f) AM_READWRITE(pc8801_cdrom_r,pc8801_cdrom_w)
//  AM_RANGE(0xa0, 0xa3) AM_NOP                                     /* music & network */
	AM_RANGE(0xa8, 0xad) AM_READWRITE(pc8801_opna_r,pc8801_opna_w)  /* second sound board */
//  AM_RANGE(0xb4, 0xb5) AM_NOP                                     /* Video art board */
//  AM_RANGE(0xc1, 0xc1) AM_NOP                                     /* (unknown) */
//  AM_RANGE(0xc2, 0xcf) AM_NOP                                     /* music */
//  AM_RANGE(0xd0, 0xd7) AM_NOP                                     /* music & GP-IB */
//  AM_RANGE(0xd8, 0xd8) AM_NOP                                     /* GP-IB */
//  AM_RANGE(0xdc, 0xdf) AM_NOP                                     /* MODEM */
	AM_RANGE(0xe2, 0xe2) AM_READWRITE(pc8801_extram_mode_r,pc8801_extram_mode_w)            /* expand RAM mode */
	AM_RANGE(0xe3, 0xe3) AM_READWRITE(pc8801_extram_bank_r,pc8801_extram_bank_w)            /* expand RAM bank */
#ifdef USE_PROPER_I8214
	AM_RANGE(0xe4, 0xe4) AM_WRITE(i8214_irq_level_w)
	AM_RANGE(0xe6, 0xe6) AM_WRITE(i8214_irq_mask_w)
#else
	AM_RANGE(0xe4, 0xe4) AM_WRITE(pc8801_irq_level_w)
	AM_RANGE(0xe6, 0xe6) AM_WRITE(pc8801_irq_mask_w)
#endif
//  AM_RANGE(0xe7, 0xe7) AM_NOP                                     /* Arcus writes here, almost likely to be a mirror of above */
	AM_RANGE(0xe8, 0xeb) AM_READWRITE(pc8801_kanji_r, pc8801_kanji_w)
	AM_RANGE(0xec, 0xef) AM_READWRITE(pc8801_kanji_lv2_r, pc8801_kanji_lv2_w)
	AM_RANGE(0xf0, 0xf0) AM_WRITE(pc8801_dic_bank_w)
	AM_RANGE(0xf1, 0xf1) AM_WRITE(pc8801_dic_ctrl_w)
//  AM_RANGE(0xf3, 0xf3) AM_NOP                                     /* DMA floppy (unknown) */
//  AM_RANGE(0xf4, 0xf7) AM_NOP                                     /* DMA 5'floppy (may be not released) */
//  AM_RANGE(0xf8, 0xfb) AM_NOP                                     /* DMA 8'floppy (unknown) */
	AM_RANGE(0xfc, 0xff) AM_DEVREADWRITE("d8255_master", i8255_device, read, write)
ADDRESS_MAP_END

READ8_MEMBER(pc8801_state::cpu_8255_c_r)
{
//  machine().scheduler().synchronize(); // force resync

	return m_i8255_1_pc >> 4;
}

WRITE8_MEMBER(pc8801_state::cpu_8255_c_w)
{
//  machine().scheduler().synchronize(); // force resync

	m_i8255_0_pc = data;
}


READ8_MEMBER(pc8801_state::fdc_8255_c_r)
{
//  machine().scheduler().synchronize(); // force resync

	return m_i8255_0_pc >> 4;
}

WRITE8_MEMBER(pc8801_state::fdc_8255_c_w)
{
//  machine().scheduler().synchronize(); // force resync

	m_i8255_1_pc = data;
}

static ADDRESS_MAP_START( pc8801fdc_mem, AS_PROGRAM, 8, pc8801_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x4000, 0x7fff) AM_RAM
ADDRESS_MAP_END

TIMER_CALLBACK_MEMBER(pc8801_state::pc8801fd_upd765_tc_to_zero)
{
	//printf("0\n");
	machine().device<upd765a_device>("upd765")->tc_w(false);
}

WRITE8_MEMBER(pc8801_state::upd765_mc_w)
{
	machine().device<floppy_connector>("upd765:0")->get_device()->mon_w(!(data & 1));
	machine().device<floppy_connector>("upd765:1")->get_device()->mon_w(!(data & 2));
}

READ8_MEMBER(pc8801_state::upd765_tc_r)
{
	//printf("%04x 1\n",m_fdccpu->pc());

	machine().device<upd765a_device>("upd765")->tc_w(true);
	//TODO: I'm not convinced that this works correctly with current hook-up ... 1000 usec is needed by Aploon, a bigger value breaks Alpha.
	//OTOH, 50 seems more than enough for the new upd...
	machine().scheduler().timer_set(attotime::from_usec(50), timer_expired_delegate(FUNC(pc8801_state::pc8801fd_upd765_tc_to_zero),this));
	return 0xff; // value is meaningless
}

WRITE8_MEMBER(pc8801_state::fdc_irq_vector_w)
{
	popmessage("Write to FDC IRQ vector I/O %02x, contact MESSdev\n",data);
	m_fdc_irq_opcode = data;
}

WRITE8_MEMBER(pc8801_state::fdc_drive_mode_w)
{
	logerror("FDC drive mode %02x\n", data);
	machine().device<floppy_connector>("upd765:0")->get_device()->set_rpm(data & 0x01 ? 360 : 300);
	machine().device<floppy_connector>("upd765:1")->get_device()->set_rpm(data & 0x02 ? 360 : 300);

	machine().device<upd765a_device>("upd765")->set_rate(data & 0x20 ? 500000 : 250000);
}

static ADDRESS_MAP_START( pc8801fdc_io, AS_IO, 8, pc8801_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xf0, 0xf0) AM_WRITE(fdc_irq_vector_w) // Interrupt Opcode Port
	AM_RANGE(0xf4, 0xf4) AM_WRITE(fdc_drive_mode_w) // Drive mode, 2d, 2dd, 2hd
	AM_RANGE(0xf7, 0xf7) AM_WRITENOP // printer port output
	AM_RANGE(0xf8, 0xf8) AM_READWRITE(upd765_tc_r,upd765_mc_w) // (R) Terminal Count Port (W) Motor Control Port
	AM_RANGE(0xfa, 0xfb) AM_DEVICE("upd765", upd765a_device, map )
	AM_RANGE(0xfc, 0xff) AM_DEVREADWRITE("d8255_slave", i8255_device, read, write)
ADDRESS_MAP_END

/* Input Ports */

/* 2008-05 FP:
Small note about the strange default mapping of function keys:
the top line of keys in PC8801 keyboard is as follows
[STOP][COPY]      [F1][F2][F3][F4][F5]      [ROLL UP][ROLL DOWN]
Therefore, in Full Emulation mode, "F1" goes to 'F3' and so on

Also, the Keypad has 16 keys, making impossible to map it in a satisfactory
way to a PC keypad. Therefore, default settings for these keys in Full
Emulation are currently based on the effect of the key rather than on
their real position

About natural keyboards: currently,
- "Keypad =" and "Keypad ," are not mapped
- "Stop" is mapped to 'Pause'
- "Copy" is mapped to 'Print Screen'
- "Kana" is mapped to 'F6'
- "Grph" is mapped to 'F7'
- "Roll Up" and "Roll Down" are mapped to 'Page Up' and 'Page Down'
- "Help" is mapped to 'F8'
 */

static INPUT_PORTS_START( pc8001 )
	PORT_START("KEY0")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)       PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)       PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)       PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)       PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)       PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)       PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)       PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)       PORT_CHAR(UCHAR_MAMEKEY(7_PAD))

	PORT_START("KEY1")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)       PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)       PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)    PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)    PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad =") PORT_CODE(KEYCODE_PGUP)
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad ,") PORT_CODE(KEYCODE_PGDN)
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)     PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(13)

	PORT_START("KEY2")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)   PORT_CHAR('@')
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)           PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)           PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)           PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)           PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)           PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)           PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)           PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("KEY3")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)           PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)           PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)           PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)           PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)           PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)           PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)           PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)           PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("KEY4")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)           PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)           PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)           PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)           PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)           PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)           PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)           PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)           PORT_CHAR('w') PORT_CHAR('W')

	PORT_START("KEY5")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)           PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)           PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)           PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2)  PORT_CHAR('\xA5') PORT_CHAR('|')
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)   PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)      PORT_CHAR('^')
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)       PORT_CHAR('-') PORT_CHAR('=')

	PORT_START("KEY6")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)           PORT_CHAR('0')
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)           PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)           PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)           PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)           PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)           PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)           PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)           PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("KEY7")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)           PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)           PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)       PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)       PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)       PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)        PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)       PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("  _") PORT_CODE(KEYCODE_DEL)            PORT_CHAR(0) PORT_CHAR('_')

	PORT_START("KEY8")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Clr Home") PORT_CODE(KEYCODE_HOME)      PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP)   PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Del Ins") PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(UCHAR_MAMEKEY(DEL)) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Grph") PORT_CODE(KEYCODE_LALT)  PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Kana") PORT_CODE(KEYCODE_LCONTROL) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RCONTROL)                        PORT_CHAR(UCHAR_SHIFT_2)

	PORT_START("KEY9")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Stop") PORT_CODE(KEYCODE_F1)            PORT_CHAR(UCHAR_MAMEKEY(PAUSE))
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)                              PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)                              PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)                              PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)                              PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)                              PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)                           PORT_CHAR(' ')
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)                             PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("KEY10")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)                             PORT_CHAR('\t')
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN)   PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT (0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT)   PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT (0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Help") PORT_CODE(KEYCODE_END)           PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT (0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Copy") PORT_CODE(KEYCODE_F2)            PORT_CHAR(UCHAR_MAMEKEY(PRTSCR))
	PORT_BIT (0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)                       PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD))
	PORT_BIT (0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)                       PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))
	PORT_BIT (0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Caps") PORT_CODE(KEYCODE_CAPSLOCK) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("KEY11")
	PORT_BIT (0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Roll Up") PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(PGUP))
	PORT_BIT (0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Roll Down") PORT_CODE(KEYCODE_F9)       PORT_CHAR(UCHAR_MAMEKEY(PGDN))
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY12")     /* port 0x0c */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY13")     /* port 0x0d */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY14")     /* port 0x0e */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY15")     /* port 0x0f */
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "BASIC" )
	PORT_DIPSETTING(    0x01, "N88-BASIC" )
	PORT_DIPSETTING(    0x00, "N-BASIC" )
	PORT_DIPNAME( 0x02, 0x02, "Terminal mode" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, "Text width" )
	PORT_DIPSETTING(    0x04, "40 chars/line" )
	PORT_DIPSETTING(    0x00, "80 chars/line" )
	PORT_DIPNAME( 0x08, 0x00, "Text height" )
	PORT_DIPSETTING(    0x08, "20 lines/screen" )
	PORT_DIPSETTING(    0x00, "25 lines/screen" )
	PORT_DIPNAME( 0x10, 0x10, "Enable S parameter" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Enable DEL code" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Memory wait" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Disable CMD SING" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Parity generate" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Parity type" )
	PORT_DIPSETTING(    0x00, "Even" )
	PORT_DIPSETTING(    0x02, "Odd" )
	PORT_DIPNAME( 0x04, 0x00, "Serial character length" )
	PORT_DIPSETTING(    0x04, "7 bits/char" )
	PORT_DIPSETTING(    0x00, "8 bits/char" )
	PORT_DIPNAME( 0x08, 0x08, "Stop bit length" )
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x10, 0x10, "Enable X parameter" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Duplex" )
	PORT_DIPSETTING(    0x20, "Half" )
	PORT_DIPSETTING(    0x00, "Full" )
	PORT_DIPNAME( 0xc0, 0x40, "Basic mode" )
	PORT_DIPSETTING(    0x80, "N88-BASIC (V1L)" )
	PORT_DIPSETTING(    0xc0, "N88-BASIC (V1H)" )
	PORT_DIPSETTING(    0x40, "N88-BASIC (V2)" )
//  PORT_DIPSETTING(    0x00, "N88-BASIC (V2)" )

	PORT_START("CTRL")
	PORT_DIPNAME( 0x02, 0x02, "Monitor Type" )
	PORT_DIPSETTING(    0x02, "15 KHz" )
	PORT_DIPSETTING(    0x00, "24 KHz" )
	PORT_DIPNAME( 0x08, 0x00, "Auto-boot floppy at start-up" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH,IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("upd1990a", upd1990a_device, data_out_r)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH,IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("CFG")       /* EXSWITCH */
	#if 0 // reference only, afaik there isn't a thing like this ...
	PORT_DIPNAME( 0x0f, 0x08, "Serial speed" )
	PORT_DIPSETTING(    0x01, "75bps" )
	PORT_DIPSETTING(    0x02, "150bps" )
	PORT_DIPSETTING(    0x03, "300bps" )
	PORT_DIPSETTING(    0x04, "600bps" )
	PORT_DIPSETTING(    0x05, "1200bps" )
	PORT_DIPSETTING(    0x06, "2400bps" )
	PORT_DIPSETTING(    0x07, "4800bps" )
	PORT_DIPSETTING(    0x08, "9600bps" )
	PORT_DIPSETTING(    0x09, "19200bps" )
	#endif
	PORT_DIPNAME( 0x40, 0x40, "Speed mode" )
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x40, DEF_STR( High ) )
	PORT_DIPNAME( 0x80, 0x80, "Main CPU clock" )
	PORT_DIPSETTING(    0x80, "4MHz" )
	PORT_DIPSETTING(    0x00, "8MHz" )

	PORT_START("OPN_PA")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("BOARD_CONFIG", 0x02, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("BOARD_CONFIG", 0x02, EQUALS, 0x00)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("BOARD_CONFIG", 0x02, EQUALS, 0x00)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1) PORT_CONDITION("BOARD_CONFIG", 0x02, EQUALS, 0x00)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("OPN_PB")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Joystick Button 1") PORT_CONDITION("BOARD_CONFIG", 0x02, EQUALS, 0x00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Joystick Button 2") PORT_CONDITION("BOARD_CONFIG", 0x02, EQUALS, 0x00)
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Mouse Button 1") PORT_CONDITION("BOARD_CONFIG", 0x02, EQUALS, 0x02)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Mouse Button 2") PORT_CONDITION("BOARD_CONFIG", 0x02, EQUALS, 0x02)
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("MOUSEX")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X ) PORT_RESET PORT_REVERSE PORT_SENSITIVITY(20) PORT_KEYDELTA(20) PORT_PLAYER(1) PORT_CONDITION("BOARD_CONFIG", 0x02, EQUALS, 0x02)

	PORT_START("MOUSEY")
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y ) PORT_RESET PORT_REVERSE PORT_SENSITIVITY(20) PORT_KEYDELTA(20) PORT_PLAYER(1) PORT_CONDITION("BOARD_CONFIG", 0x02, EQUALS, 0x02)

	PORT_START("MEM")
	PORT_CONFNAME( 0x0f, 0x0a, "Extension memory" )
	PORT_CONFSETTING(    0x00, DEF_STR( None ) )
	PORT_CONFSETTING(    0x01, "32KB (PC-8012-02 x 1)" )
	PORT_CONFSETTING(    0x02, "64KB (PC-8012-02 x 2)" )
	PORT_CONFSETTING(    0x03, "128KB (PC-8012-02 x 4)" )
	PORT_CONFSETTING(    0x04, "128KB (PC-8801-02N x 1)" )
	PORT_CONFSETTING(    0x05, "256KB (PC-8801-02N x 2)" )
	PORT_CONFSETTING(    0x06, "512KB (PC-8801-02N x 4)" )
	PORT_CONFSETTING(    0x07, "1M (PIO-8234H-1M x 1)" )
	PORT_CONFSETTING(    0x08, "2M (PIO-8234H-2M x 1)" )
	PORT_CONFSETTING(    0x09, "4M (PIO-8234H-2M x 2)" )
	PORT_CONFSETTING(    0x0a, "8M (PIO-8234H-2M x 4)" )
	PORT_CONFSETTING(    0x0b, "1.1M (PIO-8234H-1M x 1 + PC-8801-02N x 1)" )
	PORT_CONFSETTING(    0x0c, "2.1M (PIO-8234H-2M x 1 + PC-8801-02N x 1)" )
	PORT_CONFSETTING(    0x0d, "4.1M (PIO-8234H-2M x 2 + PC-8801-02N x 1)" )

	PORT_START("BOARD_CONFIG")
	PORT_CONFNAME( 0x01, 0x01, "Sound Board" ) /* TODO: is it possible to have BOTH sound chips in there? */
	PORT_CONFSETTING(    0x00, "OPN (YM2203)" )
	PORT_CONFSETTING(    0x01, "OPNA (YM2608)" )
	PORT_CONFNAME( 0x02, 0x00, "Port 1 Connection" )
	PORT_CONFSETTING(    0x00, "Joystick" )
	PORT_CONFSETTING(    0x02, "Mouse" )
INPUT_PORTS_END

static INPUT_PORTS_START( pc88sr )
	PORT_INCLUDE( pc8001 )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "BASIC" )
	PORT_DIPSETTING(    0x01, "N88-BASIC" )
	PORT_DIPSETTING(    0x00, "N-BASIC" )
INPUT_PORTS_END

/* Graphics Layouts */

static const gfx_layout char_layout =
{
	8, 8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout kanji_layout =
{
	16, 16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	16*16
};

/* debugging only */
static GFXDECODE_START( pc8801 )
	GFXDECODE_ENTRY( "cgrom", 0, char_layout,  0, 8 )
	GFXDECODE_ENTRY( "kanji", 0, kanji_layout, 0, 8 )
GFXDECODE_END

/* Floppy Configuration */

static SLOT_INTERFACE_START( pc88_floppies )
	SLOT_INTERFACE( "525hd", FLOPPY_525_HD )
SLOT_INTERFACE_END

#if 0
/* Cassette Configuration */

static const cassette_interface pc88_cassette_interface =
{
	cassette_default_formats,   // we need T88 format support!
	NULL,
	(cassette_state)(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED),
	"pc8801_cass"
};
#endif

#ifdef USE_PROPER_I8214
void pc8801_state::pc8801_raise_irq(UINT8 irq,UINT8 state)
{
	if(state)
	{
		drvm_int_state |= irq;

		drvm_pic->r_w(~irq);

		m_maincpu->set_input_line(0,ASSERT_LINE);
	}
	else
	{
		//drvm_int_state &= ~irq;

		//m_maincpu->set_input_line(0,CLEAR_LINE);
	}
}

WRITE_LINE_MEMBER(pc8801_state::pic_int_w)
{
	device_t *device = m_maincpu;
//  if (state == ASSERT_LINE)
//  {
//  }
}

WRITE_LINE_MEMBER(pc8801_state::pic_enlg_w)
{
	device_t *device = m_maincpu;
	//if (state == CLEAR_LINE)
	//{
	//}
}

static I8214_INTERFACE( pic_intf )
{
	DEVCB_DRIVER_LINE_MEMBER(pc8801_state,pic_int_w),
	DEVCB_DRIVER_LINE_MEMBER(pc8801_state,pic_enlg_w)
};

IRQ_CALLBACK_MEMBER(pc8801_state::pc8801_irq_callback)
{
	UINT8 vector = (7 - m_pic->a_r());

	m_int_state &= ~(1<<vector);
	m_maincpu->set_input_line(0,CLEAR_LINE);

	return vector << 1;
}

WRITE_LINE_MEMBER(pc8801_state::pc8801_sound_irq)
{
	if(m_sound_irq_mask && state)
		pc8801_raise_irq(machine(),1<<(4),1);
}

/*
TIMER_DEVICE_CALLBACK_MEMBER(pc8801_state::pc8801_rtc_irq)
{
    if(m_timer_irq_mask)
        pc8801_raise_irq(machine(),1<<(2),1);
}
*/

INTERRUPT_GEN_MEMBER(pc8801_state::pc8801_vrtc_irq)
{
	if(m_vblank_irq_mask)
		pc8801_raise_irq(machine(),1<<(1),1);
}

#else

#include "debugger.h"

IRQ_CALLBACK_MEMBER(pc8801_state::pc8801_irq_callback)
{
	if(m_sound_irq_latch)
	{
		m_sound_irq_latch = 0;
		return 4*2;
	}
	else if(m_vrtc_irq_latch)
	{
		m_vrtc_irq_latch = 0;
		return 1*2;
	}
	else if(m_timer_irq_latch)
	{
		m_timer_irq_latch = 0;
		return 2*2;
	}

	printf("IRQ triggered but no vector on the bus! %02x %02x %02x %02x\n",m_i8214_irq_level,m_sound_irq_latch,m_vrtc_irq_latch,m_timer_irq_latch);
	debugger_break(machine());

	return 4*2; //TODO: mustn't happen
}

WRITE_LINE_MEMBER(pc8801_state::pc8801_sound_irq)
{
//  printf("%02x %02x %02x\n",m_sound_irq_mask,m_i8214_irq_level,state);
	/* TODO: correct i8214 irq level? */
	if(state)
	{
		if(m_sound_irq_mask)
		{
			m_sound_irq_latch = 1;
			m_sound_irq_pending = 0;
			//IRQ_LOG(("sound\n"));
			m_maincpu->set_input_line(0,HOLD_LINE);
		}
		else
			m_sound_irq_pending = 1;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(pc8801_state::pc8801_rtc_irq)
{
	if(m_timer_irq_mask && m_i8214_irq_level >= 3)
	{
		m_timer_irq_latch = 1;
		//IRQ_LOG(("timer\n"));
		m_maincpu->set_input_line(0,HOLD_LINE);
	}
}

INTERRUPT_GEN_MEMBER(pc8801_state::pc8801_vrtc_irq)
{
	if(m_vrtc_irq_mask && m_i8214_irq_level >= 2)
	{
		m_vrtc_irq_latch = 1;
		//IRQ_LOG(("vrtc\n"));
		m_maincpu->set_input_line(0,HOLD_LINE);
	}
}
#endif

void pc8801_state::machine_start()
{
	machine().device<floppy_connector>("upd765:0")->get_device()->set_rpm(300);
	machine().device<floppy_connector>("upd765:1")->get_device()->set_rpm(300);
	machine().device<upd765a_device>("upd765")->set_rate(250000);

	m_rtc->cs_w(1);
	m_rtc->oe_w(1);

	m_work_ram = auto_alloc_array_clear(machine(), UINT8, 0x10000);
	m_hi_work_ram = auto_alloc_array_clear(machine(), UINT8, 0x1000);
	m_ext_work_ram = auto_alloc_array_clear(machine(), UINT8, 0x8000*0x100);
	m_gvram = auto_alloc_array_clear(machine(), UINT8, 0xc000);
	m_n80rom = memregion("n80rom")->base();
	m_n88rom = memregion("n88rom")->base();
	m_kanji_rom = memregion("kanji")->base();
	m_cg_rom = memregion("cgrom")->base();

	save_pointer(NAME(m_work_ram), 0x10000);
	save_pointer(NAME(m_hi_work_ram), 0x1000);
	save_pointer(NAME(m_ext_work_ram), 0x8000*0x100);
	save_pointer(NAME(m_gvram), 0xc000);
}

void pc8801_state::machine_reset()
{
	#define kB 1024
	#define MB 1024*1024
	const UINT32 extram_type[] = { 0*kB, 32*kB,64*kB,128*kB,128*kB,256*kB,512*kB,1*MB,2*MB,4*MB,8*MB,1*MB+128*kB,2*MB+128*kB,4*MB+128*kB, 0*kB, 0*kB };
	#undef kB
	#undef MB

	m_ext_rom_bank = 0xff;
	m_gfx_ctrl = 0x31;
	m_window_offset_bank = 0x80;
	m_misc_ctrl = 0x80;
	m_layer_mask = 0x00;
	m_vram_sel = 3;

//  pc8801_dynamic_res_change(machine());

	m_fdc_irq_opcode = 0; //TODO: copied from PC-88VA, could be wrong here ... should be 0x7f ld a,a in the latter case
	m_mouse.phase = 0;

	m_fdccpu->set_input_line_vector(0, 0);

	{
		m_txt_color = 2;
	}

	{
		int i;

		for(i=0;i<3;i++)
			m_alu_reg[i] = 0x00;
	}

	{
		m_crtc.param_count = 0;
		m_crtc.cmd = 0;
		m_crtc.status = 0;
	}

	m_beeper->set_frequency(2400);
	m_beeper->set_state(0);

	#ifdef USE_PROPER_I8214
	{
		/* initialize I8214 */
		m_pic->etlg_w(1);
		m_pic->inte_w(1);
	}
	#else
	{
		m_vrtc_irq_mask = 0;
		m_vrtc_irq_latch = 0;
		m_timer_irq_mask = 0;
		m_timer_irq_latch = 0;
		m_sound_irq_mask = 0;
		m_sound_irq_latch = 0;
		m_i8214_irq_level = 0;
		m_sound_irq_pending = 0;
	}
	#endif

	{
		m_dma_address[2] = 0xf300;
	}

	{
		m_extram_bank = 0;
		m_extram_mode = 0;
	}

	{
		int i;

		for(i=0;i<0x10;i++) //text + bitmap
			m_palette->set_pen_color(i, pal1bit(i >> 1), pal1bit(i >> 2), pal1bit(i >> 0));
	}

	m_has_clock_speed = 0;
	m_has_dictionary = 0;
	m_has_cdrom = 0;

	m_extram_size = extram_type[ioport("MEM")->read() & 0x0f];
	m_has_opna = ioport("BOARD_CONFIG")->read() & 1;
}

MACHINE_RESET_MEMBER(pc8801_state,pc8801_clock_speed)
{
	pc8801_state::machine_reset();
	m_has_clock_speed = 1;
	m_clock_setting = ioport("CFG")->read() & 0x80;

	m_maincpu->set_unscaled_clock(m_clock_setting ?  XTAL_4MHz : XTAL_8MHz);
	m_fdccpu->set_unscaled_clock(m_clock_setting ?  XTAL_4MHz : XTAL_8MHz); // correct?
	m_baudrate_val = 0;
}

MACHINE_RESET_MEMBER(pc8801_state,pc8801_dic)
{
	MACHINE_RESET_CALL_MEMBER( pc8801_clock_speed );
	m_has_dictionary = 1;
	m_dic_bank = 0;
	m_dic_ctrl = 0;
}

MACHINE_RESET_MEMBER(pc8801_state,pc8801_cdrom)
{
	MACHINE_RESET_CALL_MEMBER( pc8801_dic );
	m_has_cdrom = 1;

	{
		int i;

		for(i=0;i<0x10;i++)
			m_cdrom_reg[i] = 0;
	}
}

PALETTE_INIT_MEMBER(pc8801_state, pc8801)
{
	int i;

	for(i=0;i<0x10;i++) //text + bitmap
		palette.set_pen_color(i, pal1bit(i >> 1), pal1bit(i >> 2), pal1bit(i >> 0));
}

/* YM2203 Interface */

READ8_MEMBER(pc8801_state::opn_porta_r)
{
	if(ioport("BOARD_CONFIG")->read() & 2)
	{
		UINT8 shift,res;

		shift = (m_mouse.phase & 1) ? 0 : 4;
		res = (m_mouse.phase & 2) ? m_mouse.y : m_mouse.x;

//      printf("%d\n",m_mouse.phase);

		return ((res >> shift) & 0x0f) | 0xf0;
	}

	return ioport("OPN_PA")->read();
}
READ8_MEMBER(pc8801_state::opn_portb_r){ return ioport("OPN_PB")->read(); }

/* Cassette Configuration */
WRITE_LINE_MEMBER( pc8801_state::txdata_callback )
{
	//m_cass->output( (state) ? 0.8 : -0.8);
}

WRITE_LINE_MEMBER( pc8801_state::rxrdy_w )
{
	// ...
}

static MACHINE_CONFIG_START( pc8801, pc8801_state )
	/* main CPU */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK)        /* 4 MHz */
	MCFG_CPU_PROGRAM_MAP(pc8801_mem)
	MCFG_CPU_IO_MAP(pc8801_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", pc8801_state,  pc8801_vrtc_irq)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DRIVER(pc8801_state,pc8801_irq_callback)

	/* sub CPU(5 inch floppy drive) */
	MCFG_CPU_ADD("fdccpu", Z80, MASTER_CLOCK)       /* 4 MHz */
	MCFG_CPU_PROGRAM_MAP(pc8801fdc_mem)
	MCFG_CPU_IO_MAP(pc8801fdc_io)

	//MCFG_QUANTUM_TIME(attotime::from_hz(300000))
	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	MCFG_DEVICE_ADD("d8255_master", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(DEVREAD8("d8255_slave", i8255_device, pb_r))
	MCFG_I8255_IN_PORTB_CB(DEVREAD8("d8255_slave", i8255_device, pa_r))
	MCFG_I8255_IN_PORTC_CB(READ8(pc8801_state, cpu_8255_c_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(pc8801_state, cpu_8255_c_w))

	MCFG_DEVICE_ADD("d8255_slave", I8255, 0)
	MCFG_I8255_IN_PORTA_CB(DEVREAD8("d8255_master", i8255_device, pb_r))
	MCFG_I8255_IN_PORTB_CB(DEVREAD8("d8255_master", i8255_device, pa_r))
	MCFG_I8255_IN_PORTC_CB(READ8(pc8801_state, fdc_8255_c_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(pc8801_state, fdc_8255_c_w))

	MCFG_UPD765A_ADD("upd765", true, true)
	MCFG_UPD765_INTRQ_CALLBACK(INPUTLINE("fdccpu", INPUT_LINE_IRQ0))

	#ifdef USE_PROPER_I8214
	MCFG_I8214_ADD(I8214_TAG, MASTER_CLOCK, pic_intf)
	#endif
	MCFG_UPD1990A_ADD(UPD1990A_TAG, XTAL_32_768kHz, NULL, NULL)
	//MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED)

	MCFG_SOFTWARE_LIST_ADD("tape_list","pc8801_cass")

	MCFG_DEVICE_ADD(I8251_TAG, I8251, 0)
	MCFG_I8251_TXD_HANDLER(WRITELINE(pc8801_state, txdata_callback))
	MCFG_I8251_RTS_HANDLER(WRITELINE(pc8801_state, rxrdy_w))

	MCFG_FLOPPY_DRIVE_ADD("upd765:0", pc88_floppies, "525hd", floppy_image_device::default_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("upd765:1", pc88_floppies, "525hd", floppy_image_device::default_floppy_formats)
	MCFG_SOFTWARE_LIST_ADD("disk_list","pc8801_flop")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(PIXEL_CLOCK_24KHz,848,0,640,448,0,400)
	MCFG_SCREEN_UPDATE_DRIVER(pc8801_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", pc8801 )
	MCFG_PALETTE_ADD("palette", 0x10)
	MCFG_PALETTE_INIT_OWNER(pc8801_state, pc8801)

//  MCFG_VIDEO_START_OVERRIDE(pc8801_state,pc8801)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("opn", YM2203, MASTER_CLOCK)
	MCFG_YM2203_IRQ_HANDLER(WRITELINE(pc8801_state, pc8801_sound_irq))
	MCFG_AY8910_PORT_A_READ_CB(READ8(pc8801_state, opn_porta_r))
	MCFG_AY8910_PORT_B_READ_CB(READ8(pc8801_state, opn_portb_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_SOUND_ADD("opna", YM2608, MASTER_CLOCK*2)
	MCFG_YM2608_IRQ_HANDLER(WRITELINE(pc8801_state, pc8801_sound_irq))
	MCFG_AY8910_PORT_A_READ_CB(READ8(pc8801_state, opn_porta_r))
	MCFG_AY8910_PORT_B_READ_CB(READ8(pc8801_state, opn_portb_r))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_SOUND_ADD("beeper", BEEP, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.10)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("rtc_timer", pc8801_state, pc8801_rtc_irq, attotime::from_hz(600))
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pc8801fh, pc8801 )
	MCFG_MACHINE_RESET_OVERRIDE(pc8801_state, pc8801_clock_speed )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pc8801ma, pc8801 )
	MCFG_MACHINE_RESET_OVERRIDE(pc8801_state, pc8801_dic )
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pc8801mc, pc8801 )
	MCFG_MACHINE_RESET_OVERRIDE(pc8801_state, pc8801_cdrom )
MACHINE_CONFIG_END


/* TODO: clean this up */
#define PC8801_MEM_LOAD \
	ROM_REGION( 0x100000, "opna", ROMREGION_ERASE00 )


ROM_START( pc8801 )
	PC8801_MEM_LOAD

	ROM_REGION( 0x8000, "n80rom", ROMREGION_ERASEFF ) // 1.2
	ROM_LOAD( "n80.rom",   0x0000, 0x8000, CRC(5cb8b584) SHA1(063609dd518c124a4fc9ba35d1bae35771666a34) )

	ROM_REGION( 0x10000, "n88rom", ROMREGION_ERASEFF ) // 1.0
	ROM_LOAD( "n88.rom",   0x0000, 0x8000, CRC(ffd68be0) SHA1(3518193b8207bdebf22c1380c2db8c554baff329) )
	ROM_LOAD( "n88_0.rom", 0x8000, 0x2000, CRC(61984bab) SHA1(d1ae642aed4f0584eeb81ff50180db694e5101d4) )

	ROM_REGION( 0x10000, "fdccpu", 0)
	ROM_LOAD( "disk.rom", 0x0000, 0x0800, CRC(2158d307) SHA1(bb7103a0818850a039c67ff666a31ce49a8d516f) )

	ROM_REGION( 0x40000, "kanji", ROMREGION_ERASEFF)
	ROM_LOAD_OPTIONAL( "kanji1.rom", 0x00000, 0x20000, CRC(6178bd43) SHA1(82e11a177af6a5091dd67f50a2f4bafda84d6556) )

	ROM_REGION( 0x800, "cgrom", 0)
	ROM_LOAD( "font.rom", 0x0000, 0x0800, CRC(56653188) SHA1(84b90f69671d4b72e8f219e1fe7cd667e976cf7f) )
ROM_END

/* The dump only included "maincpu". Other roms arbitrariely taken from PC-8801 & PC-8801 MkIISR (there should be
at least 1 Kanji ROM). */
ROM_START( pc8801mk2 )
	PC8801_MEM_LOAD

	ROM_REGION( 0x8000, "n80rom", ROMREGION_ERASEFF ) // 1.4
	ROM_LOAD( "m2_n80.rom",   0x0000, 0x8000, CRC(91d84b1a) SHA1(d8a1abb0df75936b3fc9d226ccdb664a9070ffb1) )

	ROM_REGION( 0x10000, "n88rom", ROMREGION_ERASEFF ) //1.3
	ROM_LOAD( "m2_n88.rom",   0x0000, 0x8000, CRC(f35169eb) SHA1(ef1f067f819781d9fb2713836d195866f0f81501) )
	ROM_LOAD( "m2_n88_0.rom", 0x8000, 0x2000, CRC(5eb7a8d0) SHA1(95a70af83b0637a5a0f05e31fb0452bb2cb68055) )

	ROM_REGION( 0x10000, "fdccpu", 0)
	ROM_LOAD( "disk.rom", 0x0000, 0x0800, CRC(2158d307) SHA1(bb7103a0818850a039c67ff666a31ce49a8d516f) )

	ROM_REGION( 0x40000, "kanji", ROMREGION_ERASEFF)
	ROM_LOAD_OPTIONAL( "kanji1.rom", 0x00000, 0x20000, CRC(6178bd43) SHA1(82e11a177af6a5091dd67f50a2f4bafda84d6556) )

	ROM_REGION( 0x800, "cgrom", 0)
	ROM_COPY( "kanji", 0x1000, 0x0000, 0x800 )
ROM_END

ROM_START( pc8801mk2sr )
	PC8801_MEM_LOAD

	ROM_REGION( 0x8000, "n80rom", ROMREGION_ERASEFF ) // 1.5
	ROM_LOAD( "mk2sr_n80.rom",   0x0000, 0x8000, CRC(27e1857d) SHA1(5b922ed9de07d2a729bdf1da7b57c50ddf08809a) )

	ROM_REGION( 0x10000, "n88rom", ROMREGION_ERASEFF ) // 2.0
	ROM_LOAD( "mk2sr_n88.rom",   0x0000, 0x8000, CRC(a0fc0473) SHA1(3b31fc68fa7f47b21c1a1cb027b86b9e87afbfff) )
	ROM_LOAD( "mk2sr_n88_0.rom", 0x8000, 0x2000, CRC(710a63ec) SHA1(d239c26ad7ac5efac6e947b0e9549b1534aa970d) )
	ROM_LOAD( "n88_1.rom",       0xa000, 0x2000, CRC(c0bd2aa6) SHA1(8528eef7946edf6501a6ccb1f416b60c64efac7c) )
	ROM_LOAD( "n88_2.rom",       0xc000, 0x2000, CRC(af2b6efa) SHA1(b7c8bcea219b77d9cc3ee0efafe343cc307425d1) )
	ROM_LOAD( "n88_3.rom",       0xe000, 0x2000, CRC(7713c519) SHA1(efce0b51cab9f0da6cf68507757f1245a2867a72) )

	ROM_REGION( 0x10000, "fdccpu", 0)
	ROM_LOAD( "disk.rom", 0x0000, 0x0800, CRC(2158d307) SHA1(bb7103a0818850a039c67ff666a31ce49a8d516f) )

	/* No idea of the proper size: it has never been dumped */
	ROM_REGION( 0x2000, "audiocpu", 0)
	ROM_LOAD( "soundbios.rom", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x40000, "kanji", 0)
	ROM_LOAD( "kanji1.rom", 0x00000, 0x20000, CRC(6178bd43) SHA1(82e11a177af6a5091dd67f50a2f4bafda84d6556) )
	ROM_LOAD( "kanji2.rom", 0x20000, 0x20000, CRC(154803cc) SHA1(7e6591cd465cbb35d6d3446c5a83b46d30fafe95) )    // it should not be here

	ROM_REGION( 0x800, "cgrom", 0)
	ROM_COPY( "kanji", 0x1000, 0x0000, 0x800 )
ROM_END

ROM_START( pc8801mk2fr )
	PC8801_MEM_LOAD

	ROM_REGION( 0x8000, "n80rom", ROMREGION_ERASEFF ) // 1.5
	ROM_LOAD( "m2fr_n80.rom",   0x0000, 0x8000, CRC(27e1857d) SHA1(5b922ed9de07d2a729bdf1da7b57c50ddf08809a) )

	ROM_REGION( 0x10000, "n88rom", ROMREGION_ERASEFF ) // 2.1
	ROM_LOAD( "m2fr_n88.rom",   0x0000, 0x8000, CRC(b9daf1aa) SHA1(696a480232bcf8c827c7aeea8329db5c44420d2a) )
	ROM_LOAD( "m2fr_n88_0.rom", 0x8000, 0x2000, CRC(710a63ec) SHA1(d239c26ad7ac5efac6e947b0e9549b1534aa970d) )
	ROM_LOAD( "m2fr_n88_1.rom", 0xa000, 0x2000, CRC(e3e78a37) SHA1(85ecd287fe72b56e54c8b01ea7492ca4a69a7470) )
	ROM_LOAD( "m2fr_n88_2.rom", 0xc000, 0x2000, CRC(98c3a7b2) SHA1(fc4980762d3caa56964d0ae583424756f511d186) )
	ROM_LOAD( "m2fr_n88_3.rom", 0xe000, 0x2000, CRC(0ca08abd) SHA1(a5a42d0b7caa84c3bc6e337c9f37874d82f9c14b) )

	ROM_REGION( 0x10000, "fdccpu", 0)
	ROM_LOAD( "m2fr_disk.rom", 0x0000, 0x0800, CRC(2163b304) SHA1(80da2dee49d4307f00895a129a5cfeff00cf5321) )

	/* No idea of the proper size: it has never been dumped */
	ROM_REGION( 0x2000, "audiocpu", 0)
	ROM_LOAD( "soundbios.rom", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x40000, "kanji", 0)
	ROM_LOAD( "kanji1.rom", 0x00000, 0x20000, CRC(6178bd43) SHA1(82e11a177af6a5091dd67f50a2f4bafda84d6556) )

	ROM_REGION( 0x800, "cgrom", 0)
	ROM_COPY( "kanji", 0x1000, 0x0000, 0x800 )
ROM_END

ROM_START( pc8801mk2mr )
	PC8801_MEM_LOAD

	ROM_REGION( 0x8000, "n80rom", ROMREGION_ERASEFF ) // 1.8
	ROM_LOAD( "m2mr_n80.rom",   0x0000, 0x8000, CRC(f074b515) SHA1(ebe9cf4cf57f1602c887f609a728267f8d953dce) )

	ROM_REGION( 0x10000, "n88rom", ROMREGION_ERASEFF ) // 2.2
	ROM_LOAD( "m2mr_n88.rom",   0x0000, 0x8000, CRC(69caa38e) SHA1(3c64090237152ee77c76e04d6f36bad7297bea93) )
	ROM_LOAD( "m2mr_n88_0.rom", 0x8000, 0x2000, CRC(710a63ec) SHA1(d239c26ad7ac5efac6e947b0e9549b1534aa970d) )
	ROM_LOAD( "m2mr_n88_1.rom", 0xa000, 0x2000, CRC(e3e78a37) SHA1(85ecd287fe72b56e54c8b01ea7492ca4a69a7470) )
	ROM_LOAD( "m2mr_n88_2.rom", 0xc000, 0x2000, CRC(11176e0b) SHA1(f13f14f3d62df61498a23f7eb624e1a646caea45) )
	ROM_LOAD( "m2mr_n88_3.rom", 0xe000, 0x2000, CRC(0ca08abd) SHA1(a5a42d0b7caa84c3bc6e337c9f37874d82f9c14b) )

	ROM_REGION( 0x10000, "fdccpu", 0)
	ROM_LOAD( "m2mr_disk.rom", 0x0000, 0x2000, CRC(2447516b) SHA1(1492116f15c426f9796dc2bb6fcccf2656c0ca75) )

	/* No idea of the proper size: it has never been dumped */
	ROM_REGION( 0x2000, "audiocpu", 0)
	ROM_LOAD( "soundbios.rom", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x40000, "kanji", 0)
	ROM_LOAD( "kanji1.rom",      0x00000, 0x20000, CRC(6178bd43) SHA1(82e11a177af6a5091dd67f50a2f4bafda84d6556) )
	ROM_LOAD( "m2mr_kanji2.rom", 0x20000, 0x20000, CRC(376eb677) SHA1(bcf96584e2ba362218b813be51ea21573d1a2a78) )

	ROM_REGION( 0x800, "cgrom", 0)
	ROM_COPY( "kanji", 0x1000, 0x0000, 0x800 )
ROM_END

ROM_START( pc8801mh )
	PC8801_MEM_LOAD

	ROM_REGION( 0x8000, "n80rom", ROMREGION_ERASEFF ) // 1.8, but different BIOS code?
	ROM_LOAD( "mh_n80.rom",   0x0000, 0x8000, CRC(8a2a1e17) SHA1(06dae1db384aa29d81c5b6ed587877e7128fcb35) )

	ROM_REGION( 0x10000, "n88rom", ROMREGION_ERASEFF ) // 2.3
	ROM_LOAD( "mh_n88.rom",   0x0000, 0x8000, CRC(64c5d162) SHA1(3e0aac76fb5d7edc99df26fa9f365fd991742a5d) )
	ROM_LOAD( "mh_n88_0.rom", 0x8000, 0x2000, CRC(deb384fb) SHA1(5f38cafa8aab16338038c82267800446fd082e79) )
	ROM_LOAD( "mh_n88_1.rom", 0xa000, 0x2000, CRC(7ad5d943) SHA1(4ae4d37409ff99411a623da9f6a44192170a854e) )
	ROM_LOAD( "mh_n88_2.rom", 0xc000, 0x2000, CRC(6aa6b6d8) SHA1(2a077ab444a4fd1470cafb06fd3a0f45420c39cc) )
	ROM_LOAD( "mh_n88_3.rom", 0xe000, 0x2000, CRC(692cbcd8) SHA1(af452aed79b072c4d17985830b7c5dca64d4b412) )

	ROM_REGION( 0x10000, "fdccpu", 0)
	ROM_LOAD( "mh_disk.rom", 0x0000, 0x2000, CRC(a222ecf0) SHA1(79e9c0786a14142f7a83690bf41fb4f60c5c1004) )

	/* No idea of the proper size: it has never been dumped */
	ROM_REGION( 0x2000, "audiocpu", 0)
	ROM_LOAD( "soundbios.rom", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x40000, "kanji", 0)
	ROM_LOAD( "kanji1.rom",    0x00000, 0x20000, CRC(6178bd43) SHA1(82e11a177af6a5091dd67f50a2f4bafda84d6556) )
	ROM_LOAD( "mh_kanji2.rom", 0x20000, 0x20000, CRC(376eb677) SHA1(bcf96584e2ba362218b813be51ea21573d1a2a78) )

	ROM_REGION( 0x800, "cgrom", 0)
	ROM_COPY( "kanji", 0x1000, 0x0000, 0x0800 )
ROM_END

ROM_START( pc8801fa )
	PC8801_MEM_LOAD

	ROM_REGION( 0x8000, "n80rom", ROMREGION_ERASEFF ) // 1.8, but different BIOS code?
	ROM_LOAD( "fa_n80.rom",   0x0000, 0x8000, CRC(8a2a1e17) SHA1(06dae1db384aa29d81c5b6ed587877e7128fcb35) )

	ROM_REGION( 0x10000, "n88rom", ROMREGION_ERASEFF ) // 2.3 but different BIOS code?
	ROM_LOAD( "fa_n88.rom",   0x0000, 0x8000, CRC(73573432) SHA1(9b1346d44044eeea921c4cce69b5dc49dbc0b7e9) )
	ROM_LOAD( "fa_n88_0.rom", 0x8000, 0x2000, CRC(a72697d7) SHA1(5aedbc5916d67ef28767a2b942864765eea81bb8) )
	ROM_LOAD( "fa_n88_1.rom", 0xa000, 0x2000, CRC(7ad5d943) SHA1(4ae4d37409ff99411a623da9f6a44192170a854e) )
	ROM_LOAD( "fa_n88_2.rom", 0xc000, 0x2000, CRC(6aee9a4e) SHA1(e94278682ef9e9bbb82201f72c50382748dcea2a) )
	ROM_LOAD( "fa_n88_3.rom", 0xe000, 0x2000, CRC(692cbcd8) SHA1(af452aed79b072c4d17985830b7c5dca64d4b412) )

	ROM_REGION( 0x10000, "fdccpu", 0)
	ROM_LOAD( "fa_disk.rom", 0x0000, 0x0800, CRC(2163b304) SHA1(80da2dee49d4307f00895a129a5cfeff00cf5321) )

	/* No idea of the proper size: it has never been dumped */
	ROM_REGION( 0x2000, "audiocpu", 0)
	ROM_LOAD( "soundbios.rom", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x40000, "kanji", 0 )
	ROM_LOAD( "kanji1.rom",    0x00000, 0x20000, CRC(6178bd43) SHA1(82e11a177af6a5091dd67f50a2f4bafda84d6556) )
	ROM_LOAD( "fa_kanji2.rom", 0x20000, 0x20000, CRC(376eb677) SHA1(bcf96584e2ba362218b813be51ea21573d1a2a78) )

	ROM_REGION( 0x800, "cgrom", 0)
	ROM_COPY( "kanji", 0x1000, 0x0000, 0x0800 )
ROM_END

ROM_START( pc8801ma ) // newer floppy BIOS and Jisyo (dictionary) ROM
	PC8801_MEM_LOAD

	ROM_REGION( 0x8000, "n80rom", ROMREGION_ERASEFF ) // 1.8, but different BIOS code?
	ROM_LOAD( "ma_n80.rom",   0x0000, 0x8000, CRC(8a2a1e17) SHA1(06dae1db384aa29d81c5b6ed587877e7128fcb35) )

	ROM_REGION( 0x10000, "n88rom", ROMREGION_ERASEFF ) // 2.3 but different BIOS code?
	ROM_LOAD( "ma_n88.rom",   0x0000, 0x8000, CRC(73573432) SHA1(9b1346d44044eeea921c4cce69b5dc49dbc0b7e9) )
	ROM_LOAD( "ma_n88_0.rom", 0x8000, 0x2000, CRC(a72697d7) SHA1(5aedbc5916d67ef28767a2b942864765eea81bb8) )
	ROM_LOAD( "ma_n88_1.rom", 0xa000, 0x2000, CRC(7ad5d943) SHA1(4ae4d37409ff99411a623da9f6a44192170a854e) )
	ROM_LOAD( "ma_n88_2.rom", 0xc000, 0x2000, CRC(6aee9a4e) SHA1(e94278682ef9e9bbb82201f72c50382748dcea2a) )
	ROM_LOAD( "ma_n88_3.rom", 0xe000, 0x2000, CRC(692cbcd8) SHA1(af452aed79b072c4d17985830b7c5dca64d4b412) )

	ROM_REGION( 0x10000, "fdccpu", 0)
	ROM_LOAD( "ma_disk.rom", 0x0000, 0x2000, CRC(a222ecf0) SHA1(79e9c0786a14142f7a83690bf41fb4f60c5c1004) )

	/* No idea of the proper size: it has never been dumped */
	ROM_REGION( 0x2000, "audiocpu", 0)
	ROM_LOAD( "soundbios.rom", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x40000, "kanji", 0 )
	ROM_LOAD( "kanji1.rom",    0x00000, 0x20000, CRC(6178bd43) SHA1(82e11a177af6a5091dd67f50a2f4bafda84d6556) )
	ROM_LOAD( "ma_kanji2.rom", 0x20000, 0x20000, CRC(376eb677) SHA1(bcf96584e2ba362218b813be51ea21573d1a2a78) )

	ROM_REGION( 0x800, "cgrom", 0)
	ROM_COPY( "kanji", 0x1000, 0x0000, 0x0800 )

	/* 32 banks, to be loaded at 0xc000 - 0xffff */
	ROM_REGION( 0x80000, "dictionary", 0 )
	ROM_LOAD( "ma_jisyo.rom", 0x00000, 0x80000, CRC(a6108f4d) SHA1(3665db538598abb45d9dfe636423e6728a812b12) )
ROM_END

ROM_START( pc8801ma2 )
	PC8801_MEM_LOAD

	ROM_REGION( 0x8000, "n80rom", ROMREGION_ERASEFF ) // 1.8
	ROM_LOAD( "ma2_n80.rom",   0x0000, 0x8000, CRC(8a2a1e17) SHA1(06dae1db384aa29d81c5b6ed587877e7128fcb35) )

	ROM_REGION( 0x10000, "n88rom", ROMREGION_ERASEFF ) // 2.3 (2.31?)
	ROM_LOAD( "ma2_n88.rom",   0x0000, 0x8000, CRC(ae1a6ebc) SHA1(e53d628638f663099234e07837ffb1b0f86d480d) )
	ROM_LOAD( "ma2_n88_0.rom", 0x8000, 0x2000, CRC(a72697d7) SHA1(5aedbc5916d67ef28767a2b942864765eea81bb8) )
	ROM_LOAD( "ma2_n88_1.rom", 0xa000, 0x2000, CRC(7ad5d943) SHA1(4ae4d37409ff99411a623da9f6a44192170a854e) )
	ROM_LOAD( "ma2_n88_2.rom", 0xc000, 0x2000, CRC(1d6277b6) SHA1(dd9c3e50169b75bb707ef648f20d352e6a8bcfe4) )
	ROM_LOAD( "ma2_n88_3.rom", 0xe000, 0x2000, CRC(692cbcd8) SHA1(af452aed79b072c4d17985830b7c5dca64d4b412) )

	ROM_REGION( 0x10000, "fdccpu", 0)
	ROM_LOAD( "ma2_disk.rom", 0x0000, 0x2000, CRC(a222ecf0) SHA1(79e9c0786a14142f7a83690bf41fb4f60c5c1004) )

	/* No idea of the proper size: it has never been dumped */
	ROM_REGION( 0x2000, "audiocpu", 0)
	ROM_LOAD( "soundbios.rom", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x40000, "kanji", 0)
	ROM_LOAD( "kanji1.rom",     0x00000, 0x20000, CRC(6178bd43) SHA1(82e11a177af6a5091dd67f50a2f4bafda84d6556) )
	ROM_LOAD( "ma2_kanji2.rom", 0x20000, 0x20000, CRC(376eb677) SHA1(bcf96584e2ba362218b813be51ea21573d1a2a78) )

	ROM_REGION( 0x800, "cgrom", 0)
	ROM_COPY( "kanji", 0x1000, 0x0000, 0x0800 )

	ROM_REGION( 0x80000, "dictionary", 0 )
	ROM_LOAD( "ma2_jisyo.rom", 0x00000, 0x80000, CRC(856459af) SHA1(06241085fc1d62d4b2968ad9cdbdadc1e7d7990a) )
ROM_END

ROM_START( pc8801mc )
	PC8801_MEM_LOAD

	ROM_REGION( 0x08000, "n80rom", ROMREGION_ERASEFF ) // 1.8
	ROM_LOAD( "mc_n80.rom",   0x0000, 0x8000, CRC(8a2a1e17) SHA1(06dae1db384aa29d81c5b6ed587877e7128fcb35) )

	ROM_REGION( 0x10000, "n88rom", ROMREGION_ERASEFF ) // 2.3 (2.33?)
	ROM_LOAD( "mc_n88.rom",   0x0000, 0x8000, CRC(356d5719) SHA1(5d9ba80d593a5119f52aae1ccd61a1457b4a89a1) )
	ROM_LOAD( "mc_n88_0.rom", 0x8000, 0x2000, CRC(a72697d7) SHA1(5aedbc5916d67ef28767a2b942864765eea81bb8) )
	ROM_LOAD( "mc_n88_1.rom", 0xa000, 0x2000, CRC(7ad5d943) SHA1(4ae4d37409ff99411a623da9f6a44192170a854e) )
	ROM_LOAD( "mc_n88_2.rom", 0xc000, 0x2000, CRC(1d6277b6) SHA1(dd9c3e50169b75bb707ef648f20d352e6a8bcfe4) )
	ROM_LOAD( "mc_n88_3.rom", 0xe000, 0x2000, CRC(692cbcd8) SHA1(af452aed79b072c4d17985830b7c5dca64d4b412) )

	ROM_REGION( 0x10000, "fdccpu", 0)
	ROM_LOAD( "mc_disk.rom", 0x0000, 0x2000, CRC(a222ecf0) SHA1(79e9c0786a14142f7a83690bf41fb4f60c5c1004) )

	ROM_REGION( 0x10000, "cdrom", 0 )
	ROM_LOAD( "cdbios.rom", 0x0000, 0x10000, CRC(5c230221) SHA1(6394a8a23f44ea35fcfc3e974cf940bc8f84d62a) )

	/* No idea of the proper size: it has never been dumped */
	ROM_REGION( 0x2000, "audiocpu", 0)
	ROM_LOAD( "soundbios.rom", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x40000, "kanji", 0 )
	ROM_LOAD( "kanji1.rom",    0x00000, 0x20000, CRC(6178bd43) SHA1(82e11a177af6a5091dd67f50a2f4bafda84d6556) )
	ROM_LOAD( "mc_kanji2.rom", 0x20000, 0x20000, CRC(376eb677) SHA1(bcf96584e2ba362218b813be51ea21573d1a2a78) )

	ROM_REGION( 0x800, "cgrom", 0)
	ROM_COPY( "kanji", 0x1000, 0x0000, 0x0800 )

	ROM_REGION( 0x80000, "dictionary", 0 )
	ROM_LOAD( "mc_jisyo.rom", 0x00000, 0x80000, CRC(bd6eb062) SHA1(deef0cc2a9734ba891a6d6c022aa70ffc66f783e) )
ROM_END

/* System Drivers */

/*    YEAR  NAME            PARENT  COMPAT  MACHINE   INPUT   INIT  COMPANY FULLNAME */

COMP( 1981, pc8801,         0,      0,     pc8801,      pc88sr, driver_device,  0,    "Nippon Electronic Company",  "PC-8801", MACHINE_NOT_WORKING )
COMP( 1983, pc8801mk2,      pc8801, 0,     pc8801,      pc88sr, driver_device,  0,    "Nippon Electronic Company",  "PC-8801mkII", MACHINE_NOT_WORKING )
COMP( 1985, pc8801mk2sr,    pc8801, 0,     pc8801,      pc88sr, driver_device,  0,    "Nippon Electronic Company",  "PC-8801mkIISR", MACHINE_NOT_WORKING )
//COMP( 1985, pc8801mk2tr,  pc8801, 0,     pc8801,      pc88sr, driver_device,  0,    "Nippon Electronic Company",  "PC-8801mkIITR", MACHINE_NOT_WORKING )
COMP( 1985, pc8801mk2fr,    pc8801, 0,     pc8801,      pc88sr, driver_device,  0,    "Nippon Electronic Company",  "PC-8801mkIIFR", MACHINE_NOT_WORKING )
COMP( 1985, pc8801mk2mr,    pc8801, 0,     pc8801,      pc88sr, driver_device,  0,    "Nippon Electronic Company",  "PC-8801mkIIMR", MACHINE_NOT_WORKING )

//COMP( 1986, pc8801fh,     0,      0,     pc8801,      pc88sr, driver_device,  0,    "Nippon Electronic Company",  "PC-8801FH", MACHINE_NOT_WORKING )
COMP( 1986, pc8801mh,       pc8801, 0,     pc8801fh,    pc88sr, driver_device,  0,    "Nippon Electronic Company",  "PC-8801MH", MACHINE_NOT_WORKING )
COMP( 1987, pc8801fa,       pc8801, 0,     pc8801fh,    pc88sr, driver_device,  0,    "Nippon Electronic Company",  "PC-8801FA", MACHINE_NOT_WORKING )
COMP( 1987, pc8801ma,       pc8801, 0,     pc8801ma,    pc88sr, driver_device,  0,    "Nippon Electronic Company",  "PC-8801MA", MACHINE_NOT_WORKING )
//COMP( 1988, pc8801fe,     pc8801, 0,     pc8801,      pc88sr, driver_device,  0,    "Nippon Electronic Company",  "PC-8801FE", MACHINE_NOT_WORKING )
COMP( 1988, pc8801ma2,      pc8801, 0,     pc8801ma,    pc88sr, driver_device,  0,    "Nippon Electronic Company",  "PC-8801MA2", MACHINE_NOT_WORKING )
//COMP( 1989, pc8801fe2,    pc8801, 0,     pc8801,      pc88sr, driver_device,  0,    "Nippon Electronic Company",  "PC-8801FE2", MACHINE_NOT_WORKING )
COMP( 1989, pc8801mc,       pc8801, 0,     pc8801mc,    pc88sr, driver_device,  0,    "Nippon Electronic Company",  "PC-8801MC", MACHINE_NOT_WORKING )

//COMP( 1989, pc98do,       0,      0,     pc88va,   pc88sr, driver_device,  0,    "Nippon Electronic Company",  "PC-98DO", MACHINE_NOT_WORKING )
//COMP( 1990, pc98dop,      0,      0,     pc88va,   pc88sr, driver_device,  0,    "Nippon Electronic Company",  "PC-98DO+", MACHINE_NOT_WORKING )
