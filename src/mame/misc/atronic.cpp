// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Atronic Video Fruit Machines */
/*
 From 1999? (documentation is dated August 99)

Atronic was an Austrian/German slot machine manufacturer. It later
merged with Spielo.

Atronic CashLine platform (~199x-2004?) - z80-based

Atronic CashLine platform is used in multiple cabinet form-factors:
* Regular/Upright (?)
* WBC - Wide Body Cabinet
* AST - Atronic Slant Top
* Titan (?)

* CashLine hardware description:

Backplane has room for 4 boards:
1. Master board [Z80]
2. Graphic Board [TMS34020]
3. Sound Board (optional?) [YMZ280B?]
4. Communications board (optional) [M68k]

CashLine PCB boards have EEPROM/EPROM slots for software installation:
* Main software         - Master board, socket U2
* Paytable software     - Master board, socket U6
* Security device (PLA) - Master board, socket U35 (PLA)
* Graphic software      - Graphic board, sockets U8 .. U15
* Sound software        - Sound board, sockets U18 .. U21
* Comm software         - Comm board, sockets U34 and U35

Atronic platforms following Z80-based CashLine:
* "Hi(!)bility" platform (2001) - two 68k-based circuit boards for game control and external
  communications running OSE (Enea?) operating system, and a customized PC mainboard
  based on Intel 815 chipset and Intel Celeron 566MHz CPU running Windows CE.
  Custom ATI Radeon Mobility M6 AGP graphics card is used for dual displays.
* "Oxygen" platform - 3 board system, with two PowerPC-based for game control and
  communications and customized PC mainboard based on Intel i855GM with integrated GPU
  and Intel Pentium M CPU at 1.6GHz. All boards use Linux.
* "Sensys" - single board system running Linux. Custom all-purpose board with Intel
  i915GM chipset with integrated GPU and Intel Core2Duo CPU at 2.2GHz.
* "Synergy" - direct successor of "Sensys" platform. Utilizing i965GME chipset (and possibly
  newer CPU).

CashLine-based slot machines were followed by Atronic Harmony, Atronic E-Motion and
Atronic e^2-Motion slot machines.

All product lines also featured a custom linked gaming products:
* e-Motion: Cash Fever, King Kong Cash, The Game of Life, Mystery Magic, Hot Link, etc)
* e^2-Motion: Mystery Magic, Hot Link

Atronic eMotion software is deployed via set of EEPROMs + CD + Security box (custom hw?).
Known eMotion games (code in braces taken verbatim from CDROM, CF - Cash Fever,
KKC - King Kong Cash, DDTE - ??):
* African Cash (HCDSTDAFCA-11.1.4.3 66009549 / 65023927)
* Angels & Devils (?)
* Bamboo Forest
* Bella Venezia (?)
* Big Boys Toys (HCD-STD-BBTO-11.1.1.2 2008)
* Blastin' Barrels (HCDUSAGOS2_3.2.2.48)
* Caribbean Queen (HCD-STD-CAQU-11.1.0.16 2008 SAP-NO. 66009928)
* Champagne & Roses (HCD_STD_CHRO_11.1.3.25 2007 SAP-NO. 66008329)
* China Town (HCD_STD_CHTO_11.1.3.15 2007 SAP-NO. 66007445)
* Creepy Cash (HCDSTDCRCA-4.1.5.1 66006639 NV# L06-2531-03 w/o security?)
* Crimson Fire (HCD-STD-CRFI-11.1.2.5 2007)
* Crimson Fire [KKC] (HCD_STD_KKCR_11.1.3.1 2008 SAP-NO. 66010132) [v11/v12]
* Crystal Lake
* Deal or No Deal The Show (HCDSTDDODB - 11.1.2.3 66008401 / 65023927)
* Diamond Mine (HCD-STD-DIMI1CG-12.0.0.12 2009 SAP-NO. 66011438)
* Diamond Mine [KKC] (HCD_STD_DIMI1KK_12.0.0.11 2010 SAP-NO. 66012065)
* Deep Diamonds
* Doggie Cash - Garden (HCDSTDTDWI1DC-12.0.4.2SIG 66012509 / 65023927 / Spielo)
* Doggie Cash - Glittering (HCDSTDTDPA1DC-12.0.2.2SIG 66012507 / 65023927 / Spielo)
* Doggie Cash - Sports (HCDSTDTDSP1DC-12.0.2.2SIG 66012508 / 65023927 / Spielo)
* Doggie Cash - Twins (HCDSTDTDMM1DC-12.0.4.2SIG 66012506 / 65023927 / Spielo)
* Eric Ring of Silver (HCD-STD-RIOS-11.1.0.16 2007)
* Frenzy Fruits [CF] (HCD-STD-FFCF-11.1.0.18 2007)
* Golden Adventure
* Golden Harp (HCD-STD-GOHA-11.1.0.33 2008)
* Golden Light (HCDSTDGOLI_11.1.2.1 66008710 / 65023927)
* Golden Light (HCD_STD_GOLI_11.1.1.3 2007 SAP-NO. 66008062)
* Granny's Trucking Co.
* Gypsy Fortune (HCDUSAGYPS-3.0.0.8 66003310)
* Gypsy Moon (HCD-STD-GYMO1CG-12.0.1.20 2009 SAP-NO. 66011154)
* Imperial Rome
* Jungle of Gold [KKC] (HCD_STD_KKJU_11.1.3.1 2008 SAP-NO. 66010168) [v11/v12]
* Knights of the Grail (HCD_STD_KNGR_11.1.0.46 2008 SAP-NO. 66009045)
* Kublai Khan [DDTE] (HCDSTDDLKU-11.1.3.1 66009322 / 65023927)
* Logic Goods
* Magic Academy (HCD_STD_MAAC_11.1.0.5 2006 SAP-NO. 66006963)
* Might Miner SP (HCDSTDMIMI-11.1.2.2 66010359 / 65023927)
* Mistress of the Sea (HCDSTDMIST-4.0.4.19 66004029 / 65023927)
* Money Island (HCDSTDMOIS-4.1.5.1 66005113 / 65023927 w/o security?)
* Mystical Journey (HCD-STD-MYJO-11.1.1.12 2007)
* Mystical Journey (HCDSTDMYJO-11.1.2.1 66008244 / 65023927)
* Mystical Journey [DDTE] (HCDSTDDLMY-11.1.1.1 66009192 / 65023927)
* Mystic Pearls (HCD_STD_MYPE_11.1.0.36 2008 SAP-NO. 66009819)
* Mystic Pearls [KKC] (HCD_STD_KKMY_12.0.0.17 2009 SAP-NO. 66011142)
* Passion Coast (HCDSTDPACO-4.1.5.1 6605996 / MS# M0-450-08-06-048 w/o security?)
* Passion Coast (HCDSTDPACO-4.1.5.1 6605996 / 65023927)
* Passion Coast (HCD_STD_PACO_11.1.0.7 2006 SAP-NO. 66006909)
* Princess of the Amazon [KKC] (HCD_STD_KKPO_12.0.0.11 2008 SAP-NO. 66010362)
* Scarab (HCDSTDSCAR-11.1.2.9 66009334 / 65023927)
* Sign of Zodiac (HCDSTDZODI_3.0.2.52 66001491 w/o security?)
* Sphinx Classic (?)
* Stargate Daniel Jackson (HCDSTDSGDJ-12.0.2.3 66011063 / 65023927)
* Stargate Jack O'Neill (HCDSTDSGJO-12.0.3.4 66011324 / 65023927)
* Stargate Samantha Carter (HCDSTDSGSC-12.0.2.4 66011??? / 65023927)
* Sun Spirit (HCDSTDGOSU-11.1.0.27 66007627 / 65023927)
* Sun Spirit [KKC] (HCD_STD_KKGO_11.1.3.1 2008 SAP-NO. 66010164) [v11/v12]
* Super Sphinx
* Star of Africa (HCDSTDSTA2-11.1.2 66008806 / 65023927 / Spielo)
* Tabby Cash
* The Crazy Nest [CF] (HCDSTDCRCF-4.0.5.1 66005543 / 65023927)
* The Game of Life - Career choices
* Toucan Treasures (HCD-STD-TOTR-???)
* Treasures of Venice (HCDSTDVENI_3.1.3.41 66003110)
* Tree Stooges Wild Stooges (HCDSTDTSWS-12.0.3.10 66011091 / 65023927 / Spielo)
* Tiger & Dragon [KKC] (HCD_STD_KKTI_11.1.4.1 2008 SAP-NO. 66010128) [v11/v12]
* Time for Money [CF] (HCD-STD-TICF-11.1.0.9 2007 SAP-NO. 66007383)
* Treasure Cats (HCD_STD_WICA_11.1.0.2 2007)
* Treasure Cats [KKC] (HCD_STD_KKWI_11.1.5.3 2009 SAP-NO. 66010696) [v11/v12]
* Wild Diamonds [KKC] (HCD_STD_WIDI1KK_12.0.0.4 2009 SAP-NO. 66011416)
* Wild Fangs (HCD_STD_WIFA_11.1.1.1 2007 SAP-NO. 66007957)
* Wild Fangs [CF] (HCD-STD-WFCF-11.1.1.1 2007)
* Wild Valley
* Xanadu - City of Luck (HCD_STD_XACL_11.1.1.2 2007 SAP-NO. 66007934)
* Xanadu - City of Luck (HCD_STD_XACL_11.1.2.4 66008982 / 65023927
* Xanadu - City of Luck [KKC] (HCD_STD_KKXA_11.1.2.1 2008 SAP-NO. 66010166) [v11/v12]

CashLine:
 There was PC software with these too, I think they're meant to connect to a PC for configuration?
 I've put what there was in an ISO, and converted it to a CHD for later inspection.

 Some of these are probably bad dumps (the ones with strange sized roms, castawaya, tajmahal, maybe magimush)

 Anybody is welcome to try and figure out what this is.

 setup uses multiple boards in a cage, different configurations are possible

 see parts list with IC Money description below

 some of these fill up the log with
 'BLMOVE with unaligned src and aligned dst' from the 34020
 which softlocks MAME as the PC no longer advances, see "void tms340x0_device::blmove(uint16_t op)" in 34010ops.hxx
 this appears to be valid code, just unsupported in the 34020 core

*/


/*---------------------------------------------------------------------------------------------------------------------------------------
    I C Money
-----------------------------------------------------------------------------------------------------------------------------------------

-- CPU board ---

Chips with data:
* U2: M27C4001@DIP32 ; I.C.Money \n ICM_-A-I-WL \n MB-U2 (6A28) ; dumped
* U6: M27C2001@DIP32 ; I.C.Money (EE) \n 200E / 1000S \n I591-I-A2-C (EE) \n MB-U6 (F2A7) ; dumped
* U22: GAL16V8D ; MB-U22-D ; dumped
* U32: GAL22V10D ; MB-U32-D ; dumped
* U35: GAL22V10D ; 65994077/ICM-S; dumped

General chips:

U1: Zilog Z8018010VSC
.
U3: Dallas DS1386-32k-150
U4: MC14052BCP
U5: 79L05 ?
.
U7: M74HC257N
U8: 74HC08N
U9: CD74HCT74E
U10: TL7705ACP
U11: LM393P
U12: Zilog Z85C3010VSC
U13: CD74HC541E
U14: CD74HC541E
U15: CD74HC541E
U16: M74HC245N
U17: PCF8584P
U18: ULN2003AN
U19: MC74HC257N
U20: MC74HC257N
U21: SN74HC273N
.
U23: CD74HCT74E
U24: 74HC374N
U25: M74HC257N
U26: OKI M6585   (sound)
U27: MC3403P
U28: TDA2030
U29: DS1488N
U30: SN75189AN
U31: MC78L05 ?
.
U33: 74HC138N
U34: MC74HC257N
.
U36: 74HC273N
U37: MC74HC257N
U38: DS1488N
U39: SN75189AN
U40: M74HC00N
U41: empty

OSC1: 18.432MHz

U41: unpopulated

Green PCB / Mainboard QC 2121
ATRONIC 6 470.5028 00.04


--- VIDEO board ---

Dumpable stuff:

* U2: Altera EPM7032S ; U2-A ; cannot dump, connected J2 10pin header? :(
* U3: Altera EPM7032S ; U3-A ; cannot dump, connected J3 10pin header? :(

U8: M27C801 ; I.C.Money \n IM5_A-03-B ; cannot dump
U9: M27C801 ; 65998223 GB U09 8M MX27C8000PC-10 \n I.C.MONEY \n IM5_-A-03-B \n A391 4U3F ; ok
U10: M27C801 ; I.C.Money \n IM5_A-03-B \n GB-U10 (2FC4) ; ok
U11: M27C801 ; 65998223 GB U11 8M MX27C8000PC-10 \n I.C.MONEY \n IM5_-A-03-B \n D12D 3795 ; ok
U12: M27C801 ; 65998223 GB U12 8M MX27C8000PC-10 \n I.C.MONEY \n IM5_-A-03-B \n 377D 3453 ; ok
U13: M27C801 ; I.C.Money \n IM5_A-03-B \n GB-U13 (F044) ; ok
U14: M27C801 ; 65998223 GB U14 8M MX27C8000PC-10 \n I.C.MONEY \n IM5_-A-03-B \n 2F4B CP9U ; ok
U15: M27C801 ; 65998223 GB U15 8M MX27C8000PC-10 \n I.C.MONEY \n IM5_-A-03-B \n 4A50 2076 ; ok

General chips:

U1: TMS34020APCM-40
.
U4: AHCT574
U5: AHCT574
U6: TL16C550CFN
U7: Bt481AKPJ110  (palette RAMDAC)
.
U16: TMS55160DGH-60
U17: TMS55160DGH-60
U18: 74HCT04D
U19: DS1488M
U20: DS1489M
U21: unpopulated
U22: MC74F373N
U23: MC74F373N
U24: SN74F374N
U25: SN74F374N
U26: 74F244PC
U27: 74F244PC

Oscillators:

OSZ1: 40.000MHz
OSZ2: 25.000MHz

GFX board: 21 / QC 8129
Color: Red
Markings bottom: 6 470.5020 00.21
*/


/*---------------------------------------------------------------------------------------------------------------------------------------
    Wild Thing
-----------------------------------------------------------------------------------------------------------------------------------------

-- CPU board --

U2: MB-U2 ; M27C4001@DIP32 ; Wild Thing (EE) \n 200€ / 1000S \n POKE-E-A-WV (EE) \n MB-U2 (5694)
U6: MB-U6 ; M27C1001@DIP32 ; Wild Thing (EE) \n 200€ / 1000S \n WILD-A-A3-D (EE) \n MB-U6 (19CA)
U22: MB-U22D ; GAL16V8D ; ??
U32: MB-U32I ; GAL22V10D ; ??
U35: POKE-S6368 ; GAL22V10D ; ??

Red PCB / Mainboard

-- alt CPU board ---

U2: M27C4001@DIP32 ; Wild Thing (EE) \n 200€ / 1000S \n POKE-E-A-WV (EE) \n MB-U2 (5694)
U6: M27C1001@DIP32 ; Wild Thing (EE) \n 200€ / 1000S \n WILD-A-A3-D (EE) \n MB-U6 (19CA)
U22: GAL16V8B ; [no markings] ; ?? cannot dump yet
U32: GAL16V8B ; [no markings] ; ?? cannot dump yet
U35: PALCE22V10H; POKE-S-6598 ; ?? cannot dump yet

PCB markings bottom: 6 470.5028 00.01

-- another alt CPU board ---

U2: M27C4001@DIP32 ; Wild Thing (EE) \n 200€ / 1000S \n POKE-E-A-WV (EE) \n MB-U2 (5694) ; (same as mb-1 ; mb-2)
U6: M27C1001@DIP32 ; Wild Thing (EE) \n 200€ / 1000S \n WILD-A-A3-D (EE) \n MB-U6 (19CA) ; (didn't dump anymore)
U22: GAL16V8D ; MB-U22-D
U32: GAL16V8D ; MB-U32-B
U35: PALCE22V10H; POKE-S-7266 ; ?? cannot dump yet

PCB markings bottom: 6 470.5028 00.01


--- VIDEO board ---

U8: M27C4001 ; 65971387/GB-U8 \n POKE-E-05-C \n E31872
U9: empty
U10: M27C4001 ; 65971387/GB-U10 \n POKE-E-05-C \n E32156
U11: empty
U12: M27C4001 ; 65971387/GB-U12 \n POKE-E-05-C \n E33068
U13: empty
U14: M27C4001 ; 65971387/GB-U14 \n POKE-E-05-C \n E33068
U15: empty

U1: TMS34020AGBL-32
U2: GAL16V8D ;  cannot dump
U3: GAL16B8B ;  cannot dump
U4: GAL16V8B ;  cannot dump
U5: GAL16V8B ;  cannot dump
U6: GAL16V8B ;  cannot dump
U7: GAL16V8B ;  cannot dump
U35: Bt477KPJ150  ;

GFX board: 05 / QC 2053
Markings bottom: 6 470.5020 00.05

*/

/*---------------------------------------------------------------------------------------------------------------------------------------
    Bonus Poker
-----------------------------------------------------------------------------------------------------------------------------------------

-- CPU Board ---

Chips with data:
* U2: M27C4001@DIP32 ; Bonus Poker (EE) \n 200€ / 1000S \n POKE-I-B-WJ (EE) \n MB-U2 (6A73) ; dumped
* U6: M27C4001@DIP32 ; Bonus Poker (EE) \n 200€ / 1000S \n BOPO-B-A1-C (EE) \n MB-U6 (EAA2) ; dumped
* U22: GAL16V8D ; MB-U22D ; dumped
* U32: GAL22V10D ; MB-U32I ; dumped
* U35: GAL22V10D ; POKE-S6366 ; dumped

General chips:

U1: Zilog Z8018010VSC
.
U3: Dallas DS1386-32k-120
U4: MC14052BCP
U5: 79L05 ?
.
U7: M74HC257B1
U8: 74HC08N
U9: MC74HCT7AN
U10: TL7705ACP
U11: LM393N
U12: Zilog Z85C3010VSC
U13: M74HC541B1
U14: M74HC541B1
U15: M74HC541B1
U16: M74HC245B1
U17: PCF8584P
U18: ULN2003AN
U19: M74HC257B1
U20: M74HC257B1
U21: 74HC273N
.
U23: MC74HCT7AN
U24: SN74HC374N
U25: M74HC257B1
U26: OKI M6585
U27: MC3403P
U28: TDA2030
U29: DS1488N
U30: SN75189N
U31: MC78L05 ?
.
U33: 74HC138N
U34: M74HC257B1
.
U36: 74HC273N
U37: M74HC257B1
U38: DS1488N
U39: SN75189N
U40: M74HC00B1
U41: empty

OSC1: 18.432MHz

U41: unpopulated

Red PCB / Mainboard QC 8124
ATRONIC 6 470.5028 00.21

-- Video board ---

Dumpable stuff:

* U2: GAL16V8D ; GB-U2-F ; ok
* U3: GAL16B8D ; GB-U3-I ; ok
* U4: GAL16V8D ; GB-U4-C ; ok
* U5: GAL16V8D ; GB-U5-A ; ok
* U6: GAL16V8D ; GB-U6-B ; ok
* U7: GAL16V8D ; GB-U7-A ; ok

* U8: M27C4001 ; 65971388/GB-U8 \n POKE-I-06-F \n E33478 ; ok
* U9: M27C4001 ; 65971388/GB-U9 \n POKE-I-06-F \n E33633 ; ok
* U10: M27C4001 ; 65971388/GB-U10 \n POKE-I-06-F \n E33854 ; ok
* U11: M27C4001 ; 65971388/GB-U11 \n POKE-I-06-F \n E34079 ; ok
* U12: M27C4001 ; 65971388/GB-U12 \n POKE-I-06-F \n E49638 ; ok
* U13: M27C4001 ; 65971388/GB-U13 \n POKE-I-06-F \n E49647 ; ok
* U14: M27C4001 ; 65971388/GB-U14 \n POKE-I-06-F \n E49679 ; ok
* U15: M27C4001 ; 65971388/GB-U15 \n POKE-I-06-F \n E49700 ; ok

General chips:

* U1: TMS34020AGBL-32

* U16: TMS55160DGH-60
* U17: TMS55160DGH-60
* U24: 74F374N
* U25: 74F374N
* U26: 74F374N
* U27: 74F374N
* U28: 74F244PC
* U29: 74F244PC
* U31: SN74F373N
* U32: SN74F373N
* U34: SN74F32N
* U35: Conexant Bt481AKPJ110 ;
* U36: empty

Oscillators:

OSZ1: 32MHz
OSZ2: 25MHz

GFX board: 07 / QC 2286
Markings bottom: 6 470.5020 00.07
*/

#include "emu.h"
#include "cpu/z180/z180.h"
#include "cpu/tms34010/tms34010.h"
#include "machine/ds1386.h"
#include "machine/pcf8584.h"
#include "machine/z80scc.h"
#include "emupal.h"
#include "screen.h"
#include "video/ramdac.h"

namespace {

class atronic_state : public driver_device
{
public:
	atronic_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_screen(*this, "screen"),
			m_palette(*this, "palette"),
			m_maincpu(*this, "maincpu"),
			m_videocpu(*this, "tms"),
			m_ramdac(*this, "ramdac"),
			m_vidram(*this, "vidram")
	{ }

	void atronic(machine_config &config);

private:
	[[maybe_unused]] u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	u8 serial_r();
	void serial_w(u8 data);

	void atronic_map(address_map &map) ATTR_COLD;
	void atronic_portmap(address_map &map) ATTR_COLD;

	void video_map(address_map &map) ATTR_COLD;

	void ramdac_map(address_map &map) ATTR_COLD;

	// devices
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<cpu_device> m_maincpu;
	required_device<tms34020_device> m_videocpu;
	required_device<ramdac_device> m_ramdac;

	required_shared_ptr<uint32_t> m_vidram;

	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg);
	TMS340X0_SCANLINE_RGB32_CB_MEMBER(scanline_update);

};

u32 atronic_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	return 0;
}


u8 atronic_state::serial_r()
{
	// bit 3 = serial input?
	return 0xff;
}

void atronic_state::serial_w(u8 data)
{
	// bits 7 & 6 = serial clock and data?
}


void atronic_state::atronic_map(address_map &map)
{
	map(0x00000, 0x7ffff).rom();
	map(0xf8000, 0xfffff).rw("timekpr", FUNC(ds1386_32k_device::data_r), FUNC(ds1386_32k_device::data_w));
}


void atronic_state::atronic_portmap(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x003f).noprw(); // internal registers
	map(0x0068, 0x0069).rw("i2cbc", FUNC(pcf8584_device::read), FUNC(pcf8584_device::write));
	map(0x0074, 0x0074).r(FUNC(atronic_state::serial_r));
	map(0x0078, 0x007b).rw("scc", FUNC(scc85c30_device::ab_dc_r), FUNC(scc85c30_device::ab_dc_w));
	map(0x0270, 0x0270).w(FUNC(atronic_state::serial_w));
}


static INPUT_PORTS_START( atronic )
INPUT_PORTS_END



TMS340X0_TO_SHIFTREG_CB_MEMBER(atronic_state::to_shiftreg)
{
	logerror("%s:to_shiftreg(%08X)\n", machine().describe_context(), address);
}


TMS340X0_FROM_SHIFTREG_CB_MEMBER(atronic_state::from_shiftreg)
{
	logerror("%s:from_shiftreg(%08X)\n", machine().describe_context(), address);
}


TMS340X0_SCANLINE_RGB32_CB_MEMBER(atronic_state::scanline_update)
{
	uint32_t fulladdr = ((params->rowaddr << 16) | params->coladdr) >> 5;
	uint32_t const *const bg0_base = &m_vidram[(fulladdr & 0x7fe00)]; // this probably isn't screen ram, but some temp gfx are copied on startup
	uint32_t *const dst = &bitmap.pix(scanline);
	int coladdr = fulladdr & 0x1ff;
	const pen_t *pens = m_palette->pens();

	for (int x = params->heblnk; x < params->hsblnk; x += 4, coladdr++)
	{
		uint32_t bg0pix = bg0_base[coladdr & 0x1ff];
		dst[x + 0] = pens[(bg0pix & 0x000000ff)];
		dst[x + 1] = pens[(bg0pix & 0x0000ff00) >> 8];
		dst[x + 2] = pens[(bg0pix & 0x00ff0000) >> 16];
		dst[x + 3] = pens[(bg0pix & 0xff000000) >> 24];
	}
}

void atronic_state::video_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).ram().share("vidram");

	map(0xa0000000, 0xa000001f).w(m_ramdac, FUNC(ramdac_device::index_w)).umask32(0x00ff0000);
	map(0xa0000020, 0xa000003f).w(m_ramdac, FUNC(ramdac_device::pal_w)).umask32(0x000000ff);
	map(0xa0000020, 0xa000003f).w(m_ramdac, FUNC(ramdac_device::mask_w)).umask32(0x00ff0000);

	map(0xfc000000, 0xffffffff).rom().region("user1", 0);
}


#define VIDEO_CLOCK     XTAL(40'000'000)
#define PIXEL_CLOCK     XTAL(25'000'000)

// CPU BOARD
// OSC1: 18.432MHz

// VIDEO BOARD
// OSZ1: 40.000MHz
// OSZ2: 25.000MHz

void atronic_state::ramdac_map(address_map &map)
{
	map(0x000, 0x2ff).rw(m_ramdac, FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb888_w));
}

void atronic_state::atronic(machine_config &config)
{
	/* basic machine hardware */
	Z80180(config, m_maincpu, 18.432_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &atronic_state::atronic_map);
	m_maincpu->set_addrmap(AS_IO, &atronic_state::atronic_portmap);
	m_maincpu->set_vblank_int("screen", FUNC(atronic_state::irq0_line_hold));

	DS1386_32K(config, "timekpr", 32768);

	PCF8584(config, "i2cbc", 3000000);

	SCC85C30(config, "scc", 5000000);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(VIDEO_CLOCK/2, 640, 0, 512, 257, 0, 224); // ??
	m_screen->set_screen_update("tms", FUNC(tms34020_device::tms340x0_rgb32));

	PALETTE(config, "palette").set_entries(256);
	RAMDAC(config, m_ramdac, 0, m_palette);
	m_ramdac->set_addrmap(0, &atronic_state::ramdac_map);

	TMS34020(config, m_videocpu, VIDEO_CLOCK);
	m_videocpu->set_addrmap(AS_PROGRAM, &atronic_state::video_map);
	m_videocpu->set_halt_on_reset(false);
	m_videocpu->set_pixel_clock(PIXEL_CLOCK/4);
	m_videocpu->set_pixels_per_clock(4);
	m_videocpu->set_scanline_rgb32_callback(FUNC(atronic_state::scanline_update));
	m_videocpu->set_shiftreg_in_callback(FUNC(atronic_state::to_shiftreg));
	m_videocpu->set_shiftreg_out_callback(FUNC(atronic_state::from_shiftreg));
}


ROM_START( atronic )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "atronic u2.bin", 0x0000, 0x080000, CRC(ddcfa9ed) SHA1(008ffaf56ccdb3eb60fa5a0ad2f14d1988c2fa5a) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "atronic u6.bin", 0x0000, 0x020000, CRC(9742b2d8) SHA1(9f5851c78f92055730b834de18f8dc7bd9b29a37) )

	ROM_REGION32_LE( 0x800000, "user1", ROMREGION_ERASE00 ) /* TMS34020APCM-40 code (34020) */
	ROM_REGION( 0x400000, "u18u21",ROMREGION_ERASE00 ) // sound
	ROM_REGION( 0x400000, "pals",ROMREGION_ERASE00 ) // pal (converted from JED)

	DISK_REGION( "cdrom" ) // some kind of PC based utlities for these games..
	DISK_IMAGE_READONLY_OPTIONAL( "atronic", 0,SHA1(3335e9f8f67f1b176e043f078456d2b13178b7ef) )
ROM_END

ROM_START( atronica )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "atronic u2.bin", 0x0000, 0x080000, CRC(ddcfa9ed) SHA1(008ffaf56ccdb3eb60fa5a0ad2f14d1988c2fa5a) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "atronic u6 std.bin", 0x0000, 0x020000, CRC(9ef7ae79) SHA1(3ed0ea056b23cee8829421c2369ff869b370ee80) )

	ROM_REGION32_LE( 0x800000, "user1", ROMREGION_ERASE00 ) /* TMS34020APCM-40 code (34020) */
	ROM_REGION( 0x400000, "u18u21",ROMREGION_ERASE00 ) // sound
	ROM_REGION( 0x400000, "pals",ROMREGION_ERASE00 ) // pal (converted from JED)
ROM_END



ROM_START( atlantca )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "u2.8 o-atla01-abaaa-ca-rus", 0x0000, 0x100000, CRC(c3f2aa47) SHA1(eda0088bfaea7a9a341dd63ae587c989742c6630) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "u6.1 atla01-a-zb-std-5-xx-xx-axx", 0x0000, 0x020000, CRC(5d09a4bf) SHA1(94aea5396a968ff659ac9e2f4879262c55eba2fe) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	ROM_LOAD32_BYTE( "u9.8 atla01-a-e-std-5",  0x000000, 0x100000, CRC(7f8210fa) SHA1(f71faee0d606c6aa06287f6ea31f41727e2a22d9) )
	ROM_LOAD32_BYTE( "u11.8 atla01-a-e-std-5", 0x000001, 0x100000, CRC(af648717) SHA1(8ab57dc9962ed47a8beb03dcfc686c57de326793) )
	ROM_LOAD32_BYTE( "u13.8 atla01-a-e-std-5", 0x000002, 0x100000, CRC(6e89bf2b) SHA1(0c3346a5da6c67bf2ef38cf657860dccb03a0461) )
	ROM_LOAD32_BYTE( "u15.8 atla01-a-e-std-5", 0x000003, 0x100000, CRC(157a7615) SHA1(6fbb506c716e99781a73922c98dc9173c5d61353) )
	ROM_LOAD32_BYTE( "u8.8 atla01-a-e-std-5",  0x400000, 0x100000, CRC(9e910565) SHA1(78e4e731d94b7e71db7cd9b15f9d0adfdd9f4e4f) )
	ROM_LOAD32_BYTE( "u10.8 atla01-a-e-std-5", 0x400001, 0x100000, CRC(e179fb20) SHA1(373f88ad001de48b415d9e2b2ca0b885c39080ac) )
	ROM_LOAD32_BYTE( "u12.8 atla01-a-e-std-5", 0x400002, 0x100000, CRC(6e37d906) SHA1(a5db448a5846f76ccf7b5297faddf310a1cc9fd6) )
	ROM_LOAD32_BYTE( "u14.8 atla01-a-e-std-5", 0x400003, 0x100000, CRC(50188375) SHA1(511af1a61c5259d4ed99fa7cb26697bc802a5dc6) )

	ROM_REGION( 0x400000, "u18u21", 0 ) // sound
	ROM_LOAD( "u18.8 atla01-aa-a-std", 0x000000, 0x100000, CRC(df8621e6) SHA1(4a91966a577dcc79d0b694482e7600ade1d4cbbc) )
	ROM_LOAD( "u19.8 atla01-aa-a-std", 0x100000, 0x100000, CRC(bb84f63d) SHA1(02990221a17657f2e46e0d42e2670158b4b0a7a6) )
	ROM_LOAD( "u20.8 atla01-aa-a-std", 0x200000, 0x100000, CRC(9f31c533) SHA1(eefa4d547aa5067b76686918564028593bb76c96) )
	ROM_LOAD( "u21.8 atla01-aa-a-std", 0x300000, 0x100000, CRC(a1bcd0a3) SHA1(0fd66c3bda92cead9457c35ce4b39f97293bb119) )

	ROM_REGION( 0x400000, "pals", 0 ) // pal (converted from JED)
	ROM_LOAD( "atlantica.bin", 0x0000, 0x0002dd, CRC(c3fdcd7d) SHA1(b56c859689e44689474142e537951c1cef40e46b) )
ROM_END


ROM_START( atlantcaa )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "u2-80.bin", 0x0000, 0x100000, CRC(e4553537) SHA1(c61e708511c7790f7d7a7955378b8ceb975c2c55) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "u6.1 atla01-a-zb-std-5-xx-xx-axx", 0x0000, 0x020000, CRC(5d09a4bf) SHA1(94aea5396a968ff659ac9e2f4879262c55eba2fe) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	ROM_LOAD32_BYTE( "u9-80.bin",  0x000000, 0x100000, CRC(1c51f9e1) SHA1(9300c80409f28ba55b94b93a3359fac732262b27) )
	ROM_LOAD32_BYTE( "u11-80.bin", 0x000001, 0x100000, CRC(b2b1f41f) SHA1(7551c7acc5c6c26b672e4a42d847ec9af79b50fe) )
	ROM_LOAD32_BYTE( "u13-80.bin", 0x000002, 0x100000, CRC(515820fa) SHA1(2f5def7145b45f8cd63d5463880a548e58e2b2d3) )
	ROM_LOAD32_BYTE( "u15-80.bin", 0x000003, 0x100000, CRC(12ca4c4f) SHA1(b2677d65e7283f75542db51d18c4c94baf487391) )
	ROM_LOAD32_BYTE( "u8-80.bin",  0x400000, 0x100000, CRC(465d05ed) SHA1(91e8edaaa7ef91a48e4718bc70318278c1cdfdb1) )
	ROM_LOAD32_BYTE( "u10-80.bin", 0x400001, 0x100000, CRC(dc010ce6) SHA1(d185546376057879e6874f4838f190f7e67a65f5) )
	ROM_LOAD32_BYTE( "u12-80.bin", 0x400002, 0x100000, CRC(1face659) SHA1(3a84e579739809716e1bb41730a8bae1e07558c7) )
	ROM_LOAD32_BYTE( "u14-80.bin", 0x400003, 0x100000, CRC(35db0cb6) SHA1(c237416ff63de639a5fdcbe581adf9ae9396929f) )

	ROM_REGION( 0x400000, "u18u21", 0 ) // sound
	ROM_LOAD( "u18.8 atla01-aa-a-std", 0x000000, 0x100000, CRC(df8621e6) SHA1(4a91966a577dcc79d0b694482e7600ade1d4cbbc) )
	ROM_LOAD( "u19.8 atla01-aa-a-std", 0x100000, 0x100000, CRC(bb84f63d) SHA1(02990221a17657f2e46e0d42e2670158b4b0a7a6) )
	ROM_LOAD( "u20.8 atla01-aa-a-std", 0x200000, 0x100000, CRC(9f31c533) SHA1(eefa4d547aa5067b76686918564028593bb76c96) )
	ROM_LOAD( "u21.8 atla01-aa-a-std", 0x300000, 0x100000, CRC(a1bcd0a3) SHA1(0fd66c3bda92cead9457c35ce4b39f97293bb119) )

	ROM_REGION( 0x400000, "pals", 0 ) // pal (converted from JED)
	ROM_LOAD( "atlantica.bin", 0x0000, 0x0002dd, CRC(c3fdcd7d) SHA1(b56c859689e44689474142e537951c1cef40e46b) )
ROM_END



ROM_START( baboshka )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "u2-80.bin", 0x0000, 0x100000, CRC(6084ca88) SHA1(608a23b4567271c89ed6a6b9e9a4999699a7b7a0) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "u6-10.bin", 0x0000, 0x020000, CRC(8b0ccfd2) SHA1(abdc59ebddc9e4fc3aa5b723a746de1419f7d6e7) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	ROM_LOAD32_BYTE( "u9-80.bin",  0x000000, 0x100000, CRC(1a5d8a4f) SHA1(ff8160f000ecb032831ef4320b686fdd37c19bc9) )
	ROM_LOAD32_BYTE( "u11-80.bin", 0x000001, 0x100000, CRC(713e18c9) SHA1(eb14213101c3ee09601bf01000631c3a2509e876) )
	ROM_LOAD32_BYTE( "u13-80.bin", 0x000002, 0x100000, CRC(dfbc8c2f) SHA1(1ae2dcd572fa5fc31be5cdb7d6de2bced06ff94e) )
	ROM_LOAD32_BYTE( "u15-80.bin", 0x000003, 0x100000, CRC(71cd1322) SHA1(9fcbbcf88ee2bf1c84d44af0da46976c16cca3c4) )
	ROM_LOAD32_BYTE( "u8-80.bin",  0x400000, 0x100000, CRC(4c0346e3) SHA1(a28e7b6203f9a73bc20568a13fbbd91b52ea7b6b) )
	ROM_LOAD32_BYTE( "u10-80.bin", 0x400001, 0x100000, CRC(39d736af) SHA1(8be121a93de87e00788ac9f382641b299a825f34) )
	ROM_LOAD32_BYTE( "u12-80.bin", 0x400002, 0x100000, CRC(a02d4ab8) SHA1(1817adedbb2a3e85cec6868f69dfe80caec9eb13) )
	ROM_LOAD32_BYTE( "u14-80.bin", 0x400003, 0x100000, CRC(b6343ede) SHA1(d19b2dc79c7b95cf09759709b422fb78008f5c37) )

	ROM_REGION( 0x400000, "u18u21", 0 ) // sound
	ROM_LOAD( "u18-80.bin", 0x0000, 0x100000, CRC(d26dfd1b) SHA1(7a1ddd4ac4429908997f14295d445586f2c9a26f) )
	ROM_LOAD( "u19-80.bin", 0x0000, 0x100000, CRC(273c7212) SHA1(0689fd7e3862d01f258fba9773f460ea4803d0a3) )
	ROM_LOAD( "u20-80.bin", 0x0000, 0x100000, CRC(a030de64) SHA1(fb3d73416e180dfc15c469eca499ee5060482f16) )
	ROM_LOAD( "u21-80.bin", 0x0000, 0x100000, CRC(ef05d889) SHA1(431f3a057aff474221f64a2fdee35ca328db42de) )
ROM_END


ROM_START( cfblue )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "u2.bin", 0x0000, 0x100000, CRC(0b5035d0) SHA1(f77ce0d16da39c259c0f764c23c23d0313166612) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "u6.bin", 0x0000, 0x020000, CRC(63690e7e) SHA1(9dcb3d64bae03556875185ead23d9b911773f5bd) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	ROM_LOAD32_BYTE( "u9.bin",  0x000000, 0x100000, CRC(d1c2ad08) SHA1(e53c7e91b2ab86e64f4ae753404aa86ae881becf) )
	ROM_LOAD32_BYTE( "u11.bin", 0x000001, 0x100000, CRC(42872aef) SHA1(54d7cf6a9f3d5d8b2b14fa381fd7b9db974525e1) )
	ROM_LOAD32_BYTE( "u13.bin", 0x000002, 0x100000, CRC(7da9415b) SHA1(aaa73465417dcf92838021b37cb412d52ccb4d85) )
	ROM_LOAD32_BYTE( "u15.bin", 0x000003, 0x100000, CRC(e0270268) SHA1(6bf5281eb5418903403873547690bdfa04597fea) )
	ROM_LOAD32_BYTE( "u8.bin",  0x400000, 0x100000, CRC(a870d32c) SHA1(0014b9b2a2b35ae8a10ed2910213ccea50f8ba61) )
	ROM_LOAD32_BYTE( "u10.bin", 0x400001, 0x100000, CRC(f99ae371) SHA1(b468a18eb7604f191198aae68e961db97cae0332) )
	ROM_LOAD32_BYTE( "u12.bin", 0x400002, 0x100000, CRC(2b2fcd96) SHA1(ce4a8d1267874e5d615e8b3abbb4d1b16630ae7a) )
	ROM_LOAD32_BYTE( "u14.bin", 0x400003, 0x100000, CRC(d66b735c) SHA1(6c4c1e5b5b21b60e950cf70d2d6ad72d5b22237a) )

	ROM_REGION( 0x400000, "u18u21", 0 ) // sound
	ROM_LOAD( "u18.bin", 0x0000, 0x100000, CRC(77d6c103) SHA1(667c4c77eeba3af9c8c772a9ffe2941f8f3df38f) )
	ROM_LOAD( "u19.bin", 0x0000, 0x100000, CRC(36371ef6) SHA1(83a454a71e01962937b23817419fe2e071f077ee) )
	ROM_LOAD( "u20.bin", 0x0000, 0x100000, CRC(d9548179) SHA1(12537373a6a3f79952d2c7c48d41e156fc578902) )
	ROM_LOAD( "u21.bin", 0x0000, 0x100000, CRC(3a620cc6) SHA1(1dced1a40c6b3d734ea463fe58bdd9ee9e3b8822) )
ROM_END

ROM_START( cfbluea )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "u2-80.bin", 0x0000, 0x100000, CRC(4ee3805e) SHA1(45d9438a26230f50013feda1b2c68ab2f8d4f419) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "u6-10.bin", 0x0000, 0x020000, CRC(0db0531d) SHA1(391e41b2dcd38669dcc24e938e9838feee972559) )
	ROM_LOAD( "u6low-10.bin", 0x0000, 0x020000, CRC(3cbad206) SHA1(d2a468d5bfd441b74ef85be088873d1f74d5c66e) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	ROM_LOAD32_BYTE( "u9-80.bin",  0x000000, 0x100000, CRC(37b3a499) SHA1(eb3252185596dd513d3cce95f3425241ca8513ab) )
	ROM_LOAD32_BYTE( "u11-80.bin", 0x000001, 0x100000, CRC(d98b2b1d) SHA1(414d300d113e9737d63efea09b358aeb8eeed7fc) )
	ROM_LOAD32_BYTE( "u13-80.bin", 0x000002, 0x100000, CRC(478bb4a5) SHA1(94304fe1477bfc66e8dcf2c2c91226754cb8c32a) )
	ROM_LOAD32_BYTE( "u15-80.bin", 0x000003, 0x100000, CRC(cfe9e4d4) SHA1(8cd4aadd885fc5500b0a2c1e41b1f096bd4cd2b5) )
	ROM_LOAD32_BYTE( "u8-80.bin",  0x400000, 0x100000, CRC(39670383) SHA1(cd78289377c75497f96dd6b76dc717b2ddc8d9c6) )
	ROM_LOAD32_BYTE( "u10-80.bin", 0x400001, 0x100000, CRC(f9d054ae) SHA1(244733f7ee6e82fef5d0245c3fd947d369b296f9) )
	ROM_LOAD32_BYTE( "u12-80.bin", 0x400002, 0x100000, CRC(5e95768f) SHA1(ba616bf41a2bb205d366e19e773ee5f0009be212) )
	ROM_LOAD32_BYTE( "u14-80.bin", 0x400003, 0x100000, CRC(89aaf76b) SHA1(6e731ba815c20b184e44495dd2231d9ae315a146) )

	ROM_REGION( 0x400000, "u18u21", 0 ) // sound
	ROM_LOAD( "u18-80.bin", 0x0000, 0x100000, CRC(77d6c103) SHA1(667c4c77eeba3af9c8c772a9ffe2941f8f3df38f) )
	ROM_LOAD( "u19-80.bin", 0x0000, 0x100000, CRC(36371ef6) SHA1(83a454a71e01962937b23817419fe2e071f077ee) )
	ROM_LOAD( "u20-80.bin", 0x0000, 0x100000, CRC(d9548179) SHA1(12537373a6a3f79952d2c7c48d41e156fc578902) )
	ROM_LOAD( "u21-80.bin", 0x0000, 0x100000, CRC(3a620cc6) SHA1(1dced1a40c6b3d734ea463fe58bdd9ee9e3b8822) )
ROM_END


ROM_START( cfgreen )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "u2-80.bin", 0x0000, 0x100000, CRC(2afda383) SHA1(8a1d1a780f710119cbf7ee6a53d5de91cfe120c2) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "u6-10.bin", 0x0000, 0x020000, CRC(3cbad206) SHA1(d2a468d5bfd441b74ef85be088873d1f74d5c66e) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	ROM_LOAD32_BYTE( "u9-80.bin",  0x000000, 0x100000, CRC(19a47a1b) SHA1(ae9ad2027fddf96062833345a5e2b9e7101b3380) )
	ROM_LOAD32_BYTE( "u11-80.bin", 0x000001, 0x100000, CRC(7d805f07) SHA1(0bb27a702e45d3d660363ac75c0f52f07248d40a) )
	ROM_LOAD32_BYTE( "u13-80.bin", 0x000002, 0x100000, CRC(104110dc) SHA1(9322598a94e3c71f546da3b42f137a22fc78a894) )
	ROM_LOAD32_BYTE( "u15-80.bin", 0x000003, 0x100000, CRC(c752e5b1) SHA1(98832603529c99d83885a9b72bf30aa5eb1eee93) )
	ROM_LOAD32_BYTE( "u8-80.bin",  0x400000, 0x100000, CRC(63e91841) SHA1(a9644b4ed37c2143273e782bd0e85906466c1173) )
	ROM_LOAD32_BYTE( "u10-80.bin", 0x400001, 0x100000, CRC(b198a826) SHA1(361f9a055633831f45b148ca5e23cbb9be97c95f) )
	ROM_LOAD32_BYTE( "u12-80.bin", 0x400002, 0x100000, CRC(9eb176c4) SHA1(73dea223338235a3ebd224c210df4923dbb01b56) )
	ROM_LOAD32_BYTE( "u14-80.bin", 0x400003, 0x100000, CRC(ad654d75) SHA1(31804c2fae178b2614759542cba1af34b82e5f12) )

	ROM_REGION( 0x400000, "u18u21", 0 ) // sound
	ROM_LOAD( "u18-80.bin", 0x0000, 0x100000, CRC(40a74e38) SHA1(6ea3458c449434353bbc7d03bbd7a83294584603) )
	ROM_LOAD( "u19-80.bin", 0x0000, 0x100000, CRC(6769a6c2) SHA1(5eddf6a86897b39a6c75462b5047d5175b543b18) )
	ROM_LOAD( "u20-80.bin", 0x0000, 0x100000, CRC(9c3288a0) SHA1(feb3b9fefd38052a5fd2fcee6a653c6043ff1759) )
	ROM_LOAD( "u21-80.bin", 0x0000, 0x100000, CRC(71367522) SHA1(0d82940ff87396e8722f8250cd4961d11dfa46a0) )
ROM_END


ROM_START( chicken )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "u2-80.bin", 0x0000, 0x100000, CRC(51430fa1) SHA1(cb4357cc0b5c05704c984c9ab373201612f7d340) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "u6-10.bin", 0x0000, 0x020000, CRC(bac68023) SHA1(fdc5d540ceb4a2d44013dfd59b46103ec6745dea) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	ROM_LOAD32_BYTE( "u9-80.bin",  0x000000, 0x100000, CRC(1109b7d6) SHA1(c0f6f5d56ee95982688b595894a2985ef53629e7) )
	ROM_LOAD32_BYTE( "u11-80.bin", 0x000001, 0x100000, CRC(5a1449f6) SHA1(3903858239223c37615f12a8db6a8e873722e34c) )
	ROM_LOAD32_BYTE( "u13-80.bin", 0x000002, 0x100000, CRC(e1081c7a) SHA1(dd6390d64cda9af93093092361ca24b551d82549) )
	ROM_LOAD32_BYTE( "u15-80.bin", 0x000003, 0x100000, CRC(2f3930db) SHA1(6dbc3b4c3d43fc6ecec6082dbb1e1d29df43d50e) )
	ROM_LOAD32_BYTE( "u8-80.bin",  0x400000, 0x100000, CRC(d560038c) SHA1(dece84d3e0691d53806382091dfd540dee1a3cdf) )
	ROM_LOAD32_BYTE( "u10-80.bin", 0x400001, 0x100000, CRC(5c6c3a8d) SHA1(43be71f50318d12e4d55b9e1df34b0bfdb719fdf) )
	ROM_LOAD32_BYTE( "u12-80.bin", 0x400002, 0x100000, CRC(a5a119e2) SHA1(decac8c7cea764224ed7e2da8af4f551f99739e6) )
	ROM_LOAD32_BYTE( "u14-80.bin", 0x400003, 0x100000, CRC(9008b8b3) SHA1(b4dd717f46018a7005eff5dc6655d3d473311c16) )

	ROM_REGION( 0x400000, "u18u21", 0 ) // sound
	ROM_LOAD( "u18-80.bin", 0x0000, 0x100000, CRC(12c922c1) SHA1(d463328a203667dad42a7cbfb6853289095fa4c9) )
	ROM_LOAD( "u19-80.bin", 0x0000, 0x100000, CRC(5e9c8810) SHA1(711c85a81dc61290fc43b56ccd955b4e46caee32) )
	ROM_LOAD( "u20-80.bin", 0x0000, 0x100000, CRC(4ce349e4) SHA1(4499e570211aeed44db93a7c3b7b5d0b4390b0ca) )
	ROM_LOAD( "u21-80.bin", 0x0000, 0x100000, CRC(6c7343cf) SHA1(28c282857f0c29198865444061fcf37e84697cf7) )
ROM_END


ROM_START( aclown )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "u2-80.bin", 0x0000, 0x100000, CRC(162915c4) SHA1(333d2ac8323eaaa0c7b85804b7d4ceef347118d1) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "u6-10.bin", 0x0000, 0x020000, CRC(ab86b3d4) SHA1(b0d32887674f971a3ccd482775ec3f978a2ea0c1) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	ROM_LOAD32_BYTE( "u9-80.bin",  0x000000, 0x100000, CRC(8bcbb27f) SHA1(d953268213580af11a2cc0dbd8bf1652f97f3929) )
	ROM_LOAD32_BYTE( "u11-80.bin", 0x000001, 0x100000, CRC(73fb3169) SHA1(8bbe5d8b8898e2d3368506e7b66d05b8f8ac7d02) )
	ROM_LOAD32_BYTE( "u13-80.bin", 0x000002, 0x100000, CRC(47580998) SHA1(37a6e409618aa3fe7d24bd3580fa93269895b059) )
	ROM_LOAD32_BYTE( "u15-80.bin", 0x000003, 0x100000, CRC(948e3737) SHA1(43225f114a3ae66caf95821a1e8a01f1a129e38d) )
	ROM_LOAD32_BYTE( "u8-80.bin",  0x400000, 0x100000, CRC(b4607b04) SHA1(81eff1246c68017e123fbfa46b4b8234808727d7) )
	ROM_LOAD32_BYTE( "u10-80.bin", 0x400001, 0x100000, CRC(df875d3b) SHA1(262ec7db996f13fa32d17c1c5d0c89c2f98ca1cc) )
	ROM_LOAD32_BYTE( "u12-80.bin", 0x400002, 0x100000, CRC(dbab3a76) SHA1(a85b76ade2d410cbbee7a62de96bed333ce23dc3) )
	ROM_LOAD32_BYTE( "u14-80.bin", 0x400003, 0x100000, CRC(f3a6bbd5) SHA1(5a9e81ee9ce533b3ab1aeaa4ca9185f5bf0b2a65) )

	ROM_REGION( 0x400000, "u18u21", 0 ) // sound
	ROM_LOAD( "u18-80.bin", 0x0000, 0x100000, CRC(c3efd917) SHA1(7f675c27d616a489c22544e98f726a62cfcb1bdf) )
	ROM_LOAD( "u19-80.bin", 0x0000, 0x100000, CRC(a6e90bbf) SHA1(4aa4746b3d474caf653396171b42b56a3e16caa3) )
	ROM_LOAD( "u20-80.bin", 0x0000, 0x100000, CRC(fef1ae8e) SHA1(efc5c289be052c56b5cb7976da25bd5bfacd97fc) )
	ROM_LOAD( "u21-80.bin", 0x0000, 0x100000, CRC(47108677) SHA1(864f767c54c0f9ff63fad3829e828abbd5e84f0b) )
ROM_END



ROM_START( goldglen )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "u2-80.bin", 0x0000, 0x100000, CRC(94c48e59) SHA1(b660d81f1659004e08df402ef9da61a1f4818b48) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "u6-10.bin", 0x0000, 0x020000, CRC(94409a39) SHA1(99af058e48147fc75a8c23e4f1a28484f3d5f625) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	ROM_LOAD32_BYTE( "u9-80.bin",  0x000000, 0x100000, CRC(01e69d2d) SHA1(a6e6974aec52931aeeb1f90d8f917ab85ebe843e) )
	ROM_LOAD32_BYTE( "u11-80.bin", 0x000001, 0x100000, CRC(6c39a180) SHA1(95f91ec10961d36c86dee5ce42fc7c8ab693e271) )
	ROM_LOAD32_BYTE( "u13-80.bin", 0x000002, 0x100000, CRC(e60a093f) SHA1(fa0af661f869f80e11097e101ec6100a75d1e63f) )
	ROM_LOAD32_BYTE( "u15-80.bin", 0x000003, 0x100000, CRC(9877e8aa) SHA1(2073d451446709d92700cf4afb68e5b04580c620) )
	ROM_LOAD32_BYTE( "u8-80.bin",  0x400000, 0x100000, CRC(631d617c) SHA1(44cbd9a8275537f2f8804b1d17645f74cd12fdb1) )
	ROM_LOAD32_BYTE( "u10-80.bin", 0x400001, 0x100000, CRC(cdd5e435) SHA1(659b29ab16d5a3878e4984599e83f8d1cf377ca7) )
	ROM_LOAD32_BYTE( "u12-80.bin", 0x400002, 0x100000, CRC(dcb50d70) SHA1(7f8c36d5dea9c3ce61c31aef7b310f546095d72e) )
	ROM_LOAD32_BYTE( "u14-80.bin", 0x400003, 0x100000, CRC(6c0277b1) SHA1(266c6eb08cb3d9e49cf397a7e01785627892200e) )

	ROM_REGION( 0x400000, "u18u21", 0 ) // sound
	ROM_LOAD( "u18-80.bin", 0x0000, 0x100000, CRC(2afed5cf) SHA1(633214458cd47666675464fb3621aaffbe0ca63a) )
	ROM_LOAD( "u19-80.bin", 0x0000, 0x100000, CRC(70279081) SHA1(15933d81af85b2c6f831e765f2a4e4f0e44fdc18) )
	ROM_LOAD( "u20-80.bin", 0x0000, 0x100000, CRC(0bcaa5b2) SHA1(c964b7ec99cfc641f91c1a483e68999fc3e23fa4) )
	ROM_LOAD( "u21-80.bin", 0x0000, 0x100000, CRC(e2aad0ee) SHA1(2678ae011a820da644e78e0c5d2a6af39c35ac4d) )
ROM_END


ROM_START( iccash )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "u2-80.bin", 0x0000, 0x100000, CRC(27b1c41e) SHA1(55a24301578b2d4e46948362aab8bfbb2918169a) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "u6-10.bin", 0x0000, 0x020000, CRC(5e7d8a05) SHA1(255355cf594c2818d358860e616b5b578a87e974) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	ROM_LOAD32_BYTE( "u9-80.bin",  0x000000, 0x100000, CRC(db77fe46) SHA1(2502c5c165a9720e5ff1196eaa17189281c3145c) )
	ROM_LOAD32_BYTE( "u11-80.bin", 0x000001, 0x100000, CRC(3a512c6c) SHA1(ba8592773d71e57b3dc6aaff7df1214a57429b10) )
	ROM_LOAD32_BYTE( "u13-80.bin", 0x000002, 0x100000, CRC(75fadda8) SHA1(5a968f10e582fbe74000f3de33dc1e2d07c3fec1) )
	ROM_LOAD32_BYTE( "u15-80.bin", 0x000003, 0x100000, CRC(10e5a8e7) SHA1(e91de378c9485dce080d2d00a923e75f8be30f9a) )
	ROM_LOAD32_BYTE( "u8-80.bin",  0x400000, 0x100000, CRC(0ac57377) SHA1(cbbac3434b5b46f30abe880990300bf0a0393557) )
	ROM_LOAD32_BYTE( "u10-80.bin", 0x400001, 0x100000, CRC(03c67464) SHA1(d5a1e657140a31c3f77c6490bcf35075d0546b71) )
	ROM_LOAD32_BYTE( "u12-80.bin", 0x400002, 0x100000, CRC(622992ef) SHA1(6a31212436dcda308f1f78abff714bbd97df71ef) )
	ROM_LOAD32_BYTE( "u14-80.bin", 0x400003, 0x100000, CRC(ce62e7f9) SHA1(af9eeb7bdb76870e914f1233c9fd496d6c33a615) )

	ROM_REGION( 0x400000, "u18u21", 0 ) // sound
	ROM_LOAD( "u18-80.bin", 0x0000, 0x100000, CRC(8e322415) SHA1(14685d1f426187d1fbe878713cd60ece177fdd1b) )
	ROM_LOAD( "u19-80.bin", 0x0000, 0x100000, CRC(8ce5ba46) SHA1(012d8686291d9078be8a489e21180681ace06b8b) )
	ROM_LOAD( "u20-80.bin", 0x0000, 0x100000, CRC(be59025d) SHA1(b64c707129c1418833c1b5601d0a194c2e29d9a8) )
	ROM_LOAD( "u21-80.bin", 0x0000, 0x100000, CRC(d6b79eae) SHA1(a128adbca125f3811edc8c7042bf41b45dc61083) )
ROM_END


ROM_START( shpinxii )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "u2-80.bin", 0x0000, 0x100000, CRC(943d35a7) SHA1(17ead3a7f084b5e384f99903f57360ff9e133026) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "u6-10.bin", 0x0000, 0x020000, CRC(4d37999a) SHA1(678dc788cfe00ab2599df08941660324793d7f6c) )
	ROM_LOAD( "sphinx ii.bin", 0x0000, 0x020000, CRC(7fae09a6) SHA1(5c26798337d3691d81f853ee447cb7119fce7b14) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	ROM_LOAD32_BYTE( "u9-80.bin",  0x000000, 0x100000, CRC(c54e4e07) SHA1(1249494773dae044a7bb4381b084e3d2e14367d7) )
	ROM_LOAD32_BYTE( "u11-80.bin", 0x000001, 0x100000, CRC(5c1e82ab) SHA1(f22ba1dc6799388e855d8f3064b96d568619a75b) )
	ROM_LOAD32_BYTE( "u13-80.bin", 0x000002, 0x100000, CRC(fb49ae3e) SHA1(bf0cb5815639ebc3db3333249ab2ed81d3bdc684) )
	ROM_LOAD32_BYTE( "u15-80.bin", 0x000003, 0x100000, CRC(ac741fb5) SHA1(a52eaa4a43cd522885d5d9b024c0646279dffe25) )
	ROM_LOAD32_BYTE( "u8-80.bin",  0x400000, 0x100000, CRC(ca4c1626) SHA1(6a883f713272ea70fd0757f9d0e07379925973a3) )
	ROM_LOAD32_BYTE( "u10-80.bin", 0x400001, 0x100000, CRC(b64deef1) SHA1(b3c4baef7137af5b25402cec474f92333d93e727) )
	ROM_LOAD32_BYTE( "u12-80.bin", 0x400002, 0x100000, CRC(cf5a97b7) SHA1(6cb490a5a0c9e908593beff3aee374eddef19a5f) )
	ROM_LOAD32_BYTE( "u14-80.bin", 0x400003, 0x100000, CRC(98730028) SHA1(86b782bea8caf33dab9656c93856fc345977f7cc) )

	ROM_REGION( 0x400000, "u18u21", 0 ) // sound
	ROM_LOAD( "u18-80.bin", 0x0000, 0x100000, CRC(52da6133) SHA1(51ba2c586ffeddca1d8e345c644525cbccffdba8) )
	ROM_LOAD( "u19-80.bin", 0x0000, 0x100000, CRC(3aed50bc) SHA1(c7abc91dbddf9bccac9cc9a5b73fbd9b22878ca9) )
	ROM_LOAD( "u20-80.bin", 0x0000, 0x100000, CRC(0a8ac239) SHA1(7d58abaff09a7e61d1380121d085a5601549e908) )
	ROM_LOAD( "u21-80.bin", 0x0000, 0x100000, CRC(d4621e8d) SHA1(6ec49c52b88e648dbe2fe3c47868946369d717cf) )
ROM_END





ROM_START( beachpt )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "u02_o_-wave-a-b-cb-rus_.bin", 0x0000, 0x100000, CRC(b26085fc) SHA1(19f350c46088b58438dfc234d4ac543105913286) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "u06_crp5bs1a.bin", 0x0000, 0x020000, CRC(0db0531d) SHA1(391e41b2dcd38669dcc24e938e9838feee972559) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	ROM_LOAD32_BYTE( "u09_bep5a01d.bin", 0x000000, 0x100000, CRC(0f4614de) SHA1(2181c552e9a3669fda5e87d0c596d5534d24d4b3) )
	ROM_LOAD32_BYTE( "u11_bep5a01d.bin", 0x000001, 0x100000, CRC(4f8c6fee) SHA1(2b75fe948bddda899969ef4a7663a52dc7b0eb81) )
	ROM_LOAD32_BYTE( "u13_bep5a01d.bin", 0x000002, 0x100000, CRC(ca9a24e5) SHA1(67276f680f3aedf480c54c666f0db1110cd77aee) )
	ROM_LOAD32_BYTE( "u15_bep5a01d.bin", 0x000003, 0x100000, CRC(ac904dc8) SHA1(165f66423a9c9231baa4e8b2e465d7f10f61202d) )
	ROM_LOAD32_BYTE( "u08_bep5a01d.bin", 0x400000, 0x100000, CRC(1f19cba6) SHA1(201975c7b440d2f53439e7383fb49b921015f22c) )
	ROM_LOAD32_BYTE( "u10_bep5a01d.bin", 0x400001, 0x100000, CRC(940bb1b2) SHA1(a57d6e5d3872787ff1821cbc80032269b378ecf0) )
	ROM_LOAD32_BYTE( "u12_bep5a01d.bin", 0x400002, 0x100000, CRC(07851214) SHA1(d088243f11ba51c79796986e99f0aa34d0d697d6) )
	ROM_LOAD32_BYTE( "u14_bep5a01d.bin", 0x400003, 0x100000, CRC(16a5ce9c) SHA1(3551e6eb7ff34f9ea70b7e6e940044ea1b4c59bb) )

	ROM_REGION( 0x400000, "u18u21", 0 ) // sound
	ROM_LOAD( "u18_bep_aa_a.bin", 0x0000, 0x100000, CRC(a0c6dafd) SHA1(9a09224b2d91cbf4efad5563a7633b973b0e5ce1) )
	ROM_LOAD( "u19_bep_aa_a.bin", 0x0000, 0x100000, CRC(69f1f267) SHA1(4fa837bf285670ed26ed0f0dada5e2a54ca7f142) )
	ROM_LOAD( "u20_bep_aa_a.bin", 0x0000, 0x100000, CRC(3dc030aa) SHA1(f01305fb187ae150b1264e8b72439e638772fbcc) )
	ROM_LOAD( "u21_bep_aa_a.bin", 0x0000, 0x100000, CRC(791c809a) SHA1(68af52cb2032a0c3f76030681baaaac8fb0bf51b) )
ROM_END



ROM_START( beetleup )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "u02_0-beet-a-a-cc.0def.bin", 0x0000, 0x100000, CRC(b5eedf40) SHA1(40a9baac99e9844cef5d3922c853f5e4903a7833) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "u06_n5b0-a-04-b.65aa.bin", 0x0000, 0x020000, CRC(d68d08e4) SHA1(548577d43f4136cf16266fe6855898a30fa49965) )
	ROM_LOAD( "u06_n5b0-a-05-b.648f.bin", 0x0000, 0x020000, CRC(2d2ff35f) SHA1(97759fbad4b6b30ca8f8ea74da74cfaa433a7fa2) )
	ROM_LOAD( "u06_n5b0-a-06-b.64 56.bin", 0x0000, 0x020000, CRC(7b4a6a97) SHA1(e3d54476730ca34a9f7214219cf991a220e15d5c) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	ROM_LOAD32_BYTE( "u09_7b88.bin", 0x000000, 0x100000, CRC(8443972b) SHA1(5f2eea84ba18a83502f36eeaa52cff49a1631668) )
	ROM_LOAD32_BYTE( "u11_1957.bin", 0x000001, 0x100000, CRC(36c7e5c5) SHA1(2bad0bb6b363af6a37f5b11c7ca8b3b674df4072) )
	ROM_LOAD32_BYTE( "u13_b661.bin", 0x000002, 0x100000, CRC(0e74726c) SHA1(3103d801a622315877fc09d9c99290b54b266885) )
	ROM_LOAD32_BYTE( "u15_fd57.bin", 0x000003, 0x100000, CRC(9eabc514) SHA1(ed89b068b381ad4e007352bddf3aebe10ebebf4a) )
	ROM_LOAD32_BYTE( "u08_5804.bin", 0x400000, 0x100000, CRC(65a020e3) SHA1(2bb781905338e2d444222095d8430137632fae3d) )
	ROM_LOAD32_BYTE( "u10_647c.bin", 0x400001, 0x100000, CRC(c46c064d) SHA1(6181cf0e1d53f0a79c266b4f87d3b0c32313c593) )
	ROM_LOAD32_BYTE( "u12_07f3.bin", 0x400002, 0x100000, CRC(cefdfdad) SHA1(2032e0942f52d025f8da225c31ca7f7121c7b7f8) )
	ROM_LOAD32_BYTE( "u14_fe27.bin", 0x400003, 0x100000, CRC(50cfd898) SHA1(8b881ae8c60f215bb0f75e14493ee4a0c9f2b364) )

	ROM_REGION( 0x400000, "u18u21", 0 ) // sound
	ROM_LOAD( "u18_f978.bin", 0x0000, 0x100000, CRC(afa9a1a8) SHA1(a06bdd776ca7ba9e9ceecc0935761e6d88cad90e) )
	ROM_LOAD( "u19_8766.bin", 0x0000, 0x100000, CRC(f63ed18c) SHA1(59d05582bbd125009a6bc226ec0ef2120c768694) )
	ROM_LOAD( "u20_26f0.bin", 0x0000, 0x100000, CRC(39f8e6d9) SHA1(59cd29d08610f601e3228364ae52a4e49e325f40) )
	ROM_LOAD( "u21_ea59.bin", 0x0000, 0x100000, CRC(0bd2f188) SHA1(f29a67c3cd36e7ee6fd7ef72a7724dc9df5b5657) )
ROM_END



ROM_START( bigblue )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "o_-bbbu01-adbaa-cb-std_.8mu02.bin", 0x0000, 0x100000, CRC(62d08d90) SHA1(fa563dd59eacd3021744863245aa7f82dea2c266) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "big blue bags.bin", 0x0000, 0x020000, CRC(4ec3fc1c) SHA1(7a081d370c54a6ea333957958b1341560458e845) )
	ROM_LOAD( "bbbu01-c-za-std_-5-xx-xx-axx.1mu06.bin", 0x0000, 0x020000, CRC(09e6df0b) SHA1(85961160f95cb8d223f73483d6edad79fa37d729) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	ROM_LOAD32_BYTE( "bbbu01-a_-a-std_-5_.8gu09.bin", 0x000000, 0x100000, CRC(6f11b908) SHA1(663382bc295615afbc3a9a39c7089470b8b55926) )
	ROM_LOAD32_BYTE( "bbbu01-a_-a-std_-5_.8gu11.bin", 0x000001, 0x100000, CRC(4cddcb5a) SHA1(e23354ab36f814b22c39564111558d4935fe8d70) )
	ROM_LOAD32_BYTE( "bbbu01-a_-a-std_-5_.8gu13.bin", 0x000002, 0x100000, CRC(3a6dd649) SHA1(0f2b6cdf4f10ded99adc4fe0b47e4fada4aa6643) )
	ROM_LOAD32_BYTE( "bbbu01-a_-a-std_-5_.8gu15.bin", 0x000003, 0x100000, CRC(2efd2269) SHA1(3923ec31e245c29786cd67a89d582bc051967580) )
	ROM_LOAD32_BYTE( "bbbu01-a_-a-std_-5_.8gu08.bin", 0x400000, 0x100000, CRC(32f01864) SHA1(f515cfa2ecdc239b441f6f5f7033516e88030ad6) )
	ROM_LOAD32_BYTE( "bbbu01-a_-a-std_-5_.8gu10.bin", 0x400001, 0x100000, CRC(97efddfd) SHA1(1303733596b725b705fa9dfd4150b4a6df9d4172) )
	ROM_LOAD32_BYTE( "bbbu01-a_-a-std_-5_.8gu12.bin", 0x400002, 0x100000, CRC(36c4f212) SHA1(1e7ea33114f1ca836849242d717023b1315a466e) )
	ROM_LOAD32_BYTE( "bbbu01-a_-a-std_-5_.8gu14.bin", 0x400003, 0x100000, CRC(bde8af9e) SHA1(095e567d35c45ae5e377cdf07ab77f6781e39cca) )

	ROM_REGION( 0x400000, "u18u21", 0 ) // sound
	ROM_LOAD( "bbbu01-ba-a-std_-_.8su18.bin", 0x0000, 0x100000, CRC(451687c9) SHA1(5165586a7ab69529396e4b387002e1dcbe3d892d) )
	ROM_LOAD( "bbbu01-ba-a-std_-_.8su19.bin", 0x0000, 0x100000, CRC(bb0029ec) SHA1(ab460d40ee46ee43b195b7a2ece42bcaaa043892) )
	ROM_LOAD( "bbbu01-ba-a-std_-_.8su20.bin", 0x0000, 0x100000, CRC(fb1bd294) SHA1(78dcaffc56f56d2b31d0c20d48d91696910be160) )
	ROM_LOAD( "bbbu01-ba-a-std_-_.8su21.bin", 0x0000, 0x100000, CRC(ec6d68ea) SHA1(91167b6379a6e1748e635c4b2b603a39fb6b049d) )
ROM_END

ROM_START( castaway )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "u2.8 o-cast-b-a-cc", 0x0000, 0x100000, CRC(8f103bb3) SHA1(65596aff9cfb2345a36a0e2a2b03a2b4310d421c) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "u6.1 c5bo-a-03-a", 0x0000, 0x020000, CRC(3917302a) SHA1(39b0672c36554712825a0e310522933be4b46d84) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	ROM_LOAD32_BYTE( "u9.8 cw5_b-03-b",  0x000000, 0x100000, CRC(c49aaf25) SHA1(5518312046208b4f912e9dee2ff24653a9976c6f) )
	ROM_LOAD32_BYTE( "u11.8 cw5_b-03-b", 0x000001, 0x100000, CRC(24267b4b) SHA1(9103923dd1bba0b01f6020f7c357ac9b7bef4951) )
	ROM_LOAD32_BYTE( "u13.8 cw5_b-03-b", 0x000002, 0x100000, CRC(3e606516) SHA1(5edad0a3099700bfeedff5a143591a85b3c4f582) )
	ROM_LOAD32_BYTE( "u15.8 cw5_b-03-b", 0x000003, 0x100000, CRC(7211abc5) SHA1(acaa9ad55abeb34e2d97b419f5213e44e80adde0) )
	ROM_LOAD32_BYTE( "u8.8 cw5_b-03-b",  0x400000, 0x100000, CRC(03839c9e) SHA1(56ad8843192ca47c1d467c69ab2d13189a19a905) )
	ROM_LOAD32_BYTE( "u10.8 cw5_b-03-b", 0x400001, 0x100000, CRC(cbe7399e) SHA1(da829dcab116b48a56526750546287e33d3de3c7) )
	ROM_LOAD32_BYTE( "u12.8 cw5_b-03-b", 0x400002, 0x100000, CRC(770e92de) SHA1(c358b0f528fc5c1efd2dd4a0563e958d90b55d64) )
	ROM_LOAD32_BYTE( "u14.8 cw5_b-03-b", 0x400003, 0x100000, CRC(dadd0d0b) SHA1(87750c506f6429f382149d205849663b622abda3) )

	ROM_REGION( 0x400000, "u18u21", 0 ) // sound
	ROM_LOAD( "u18.8 castaa_a", 0x0000, 0x100000, CRC(98b0a1f3) SHA1(2a6298a82dc549078857e43d60e692062b1cd022) )
	ROM_LOAD( "u19.8 castaa_a", 0x0000, 0x100000, CRC(2c7aa4a4) SHA1(7e495ce9e18ae759e9ecf21f55c6bc7c7b06a92d) )
	ROM_LOAD( "u20.8 castaa_a", 0x0000, 0x100000, CRC(cbb5824d) SHA1(626fd9c0f76942c7c040743519e2af867afed75a) )
	ROM_LOAD( "u21.8 castaa_a", 0x0000, 0x100000, CRC(31554b6b) SHA1(6af8dc72e0fcec7f73b54b728f8c61a51f5f0d48) )
ROM_END

ROM_START( castawaya ) // bad dump? (roms all look incorrect size to me)
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "u-2 m27c801.bin", 0x0000, 0x080000, CRC(55b61206) SHA1(abdbe887a6739dbc9f51838b31d23d3c8d8f03dd) )

	ROM_REGION( 0x080000, "u6", 0 ) // config?
	ROM_LOAD( "u-6 m27c801.bin", 0x0000, 0x080000, CRC(86538b30) SHA1(6b8d732b59af2cc1a6524989f8cf12a4d4dac484) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	// seem to be half size, doesn't have the TMS vectors
	ROM_LOAD32_BYTE( "u-9 m27c801.bin",  0x000000, 0x080000, BAD_DUMP CRC(4a5efe38) SHA1(23e82eeadccdd0224858686b1d96bd5d184904cb) )
	ROM_LOAD32_BYTE( "u-11 m27c801.bin", 0x000001, 0x080000, BAD_DUMP CRC(099e27e2) SHA1(4419ac8090ccab673e61f4f73c837971e341e7e2) )
	ROM_LOAD32_BYTE( "u-13 m27c801.bin", 0x000002, 0x080000, BAD_DUMP CRC(f65eb71f) SHA1(9e116cc2b6768c1525759735eecc05db5906f2dc) )
	ROM_LOAD32_BYTE( "u-15 m27c801.bin", 0x000003, 0x080000, BAD_DUMP CRC(319c8bb6) SHA1(8de3c66b375f0ff200ff240765c6f37609c4935e) )
	ROM_LOAD32_BYTE( "u-8 m27c801.bin",  0x400000, 0x080000, BAD_DUMP CRC(74e21aeb) SHA1(dce3b413c6efdc2d85357dd1f9e4e2808aef4f7e) )
	ROM_LOAD32_BYTE( "u-10 m27c801.bin", 0x400001, 0x080000, BAD_DUMP CRC(45294960) SHA1(1b5b33ef730c44a4800e80891369d4e21e1729d2) )
	ROM_LOAD32_BYTE( "u-12 m27c801.bin", 0x400002, 0x080000, BAD_DUMP CRC(be19a02c) SHA1(8968d4ec5ff58dcf50c3c9ac9601c6416399091d) )
	ROM_LOAD32_BYTE( "u-14 m27c801.bin", 0x400003, 0x080000, BAD_DUMP CRC(cda73f12) SHA1(dd5556d7e19ef1a9fec1b914a7464fbc4f97effe) )

	ROM_REGION( 0x400000, "u18u21", 0 ) // sound
	ROM_LOAD( "u-18 m27c801.bin", 0x0000, 0x080000, BAD_DUMP CRC(2e3d7181) SHA1(eed1a1594405f416efb379e134c12be89d495402) )
	ROM_LOAD( "u-19 m27c801.bin", 0x0000, 0x080000, BAD_DUMP CRC(a3c906f6) SHA1(25314d79f420c177424bf1de492ba8d9a928f643) )
	ROM_LOAD( "u-20 m27c801.bin", 0x0000, 0x080000, BAD_DUMP CRC(ec395eaa) SHA1(fa27b928971c039fb1586631f136bd7577be1c57) )
	ROM_LOAD( "u-21 m27c801.bin", 0x0000, 0x080000, BAD_DUMP CRC(324d0539) SHA1(71639e3fc40c09e07221524580046dc2447b43f1) )

	ROM_REGION( 0x400000, "others", 0 )
	ROM_LOAD( "ds1225y.bin", 0x0000, 0x002000, CRC(76af0395) SHA1(ce6aef5349b155f8e103bb4dd33933c501a490ae) )
	ROM_LOAD( "ds1386-32k.bin", 0x0000, 0x002000, CRC(4e28ebc9) SHA1(dfd60c53ffdd0b44b7f894e19b212bc88e81192f) )
ROM_END


ROM_START( dncsprt )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "o_-dasp01-adaaa-cc-std_.8mu02", 0x0000, 0x100000, CRC(744b40d7) SHA1(f7b3f507ccccb36ae55ac3b567956c45a83d3b63) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "dasp01-d-za-std_-5-xx-xx-axx.1mu06", 0x0000, 0x020000, CRC(2d5f7976) SHA1(77de321ba2f46726a0c26aa498a4c3deb7f8c421) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	ROM_LOAD32_BYTE( "dasp01-a_-c-std_-5_.8gu09", 0x000000, 0x100000, CRC(e6941863) SHA1(c9fe08bd070c9fac7b8c9089a6ecbff581265b3a) )
	ROM_LOAD32_BYTE( "dasp01-a_-c-std_-5_.8gu11", 0x000001, 0x100000, CRC(400b82ab) SHA1(5af6daf65e50b0c5ad27c43b0f3d4d8d24f38102) )
	ROM_LOAD32_BYTE( "dasp01-a_-c-std_-5_.8gu13", 0x000002, 0x100000, CRC(b425a8f6) SHA1(82d8e0d8602a81c6d4cf528b73f3c84ab5dde11b) )
	ROM_LOAD32_BYTE( "dasp01-a_-c-std_-5_.8gu15", 0x000003, 0x100000, CRC(f19ed793) SHA1(d157587ac47ba3237bfb4676a718b3c60fda5fd7) )
	ROM_LOAD32_BYTE( "dasp01-a_-c-std_-5_.8gu08", 0x400000, 0x100000, CRC(6b90f3a5) SHA1(392b9b77ba193625c51873139fb739f12420a853) )
	ROM_LOAD32_BYTE( "dasp01-a_-c-std_-5_.8gu10", 0x400001, 0x100000, CRC(824c4e39) SHA1(af9a90e8a86141cf9ae0f4d47bb3108042314bfd) )
	ROM_LOAD32_BYTE( "dasp01-a_-c-std_-5_.8gu12", 0x400002, 0x100000, CRC(29f4d1d2) SHA1(86de12c7686b1c6b43ebe7ed65b973cc9a3b208a) )
	ROM_LOAD32_BYTE( "dasp01-a_-c-std_-5_.8gu14", 0x400003, 0x100000, CRC(b6dd4982) SHA1(45ff8232eb965ad60e4cf066ab3addb532b39b56) )

	ROM_REGION( 0x400000, "u18u21", 0 ) // sound
	ROM_LOAD( "dasp01-aa-a-std_-_.8su18", 0x0000, 0x100000, CRC(65aeacb0) SHA1(97bbdb4e70e50c14ecb2cc18756c581a2069715f) )
	ROM_LOAD( "dasp01-aa-a-std_-_.8su19", 0x0000, 0x100000, CRC(0f81c833) SHA1(5568c92484c2b52b04034d99a7531a78bc1d5eb1) )
	ROM_LOAD( "dasp01-aa-a-std_-_.8su20", 0x0000, 0x100000, CRC(97302d05) SHA1(4a03d337a5e46f9b5686d1e16fa72f83cf4674f0) )
	ROM_LOAD( "dasp01-aa-a-std_-_.8su21", 0x0000, 0x100000, CRC(849e86f9) SHA1(cba1379b4cef793fc08c20607867d01c53d397a4) )
ROM_END

ROM_START( drmmake )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "u2-801", 0x0000, 0x100000, CRC(c809ecf0) SHA1(4ab641f9b805cd13d1fb860a3e9776505474a95d) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "u6-1001", 0x0000, 0x020000, CRC(7a00ad2a) SHA1(67d90b10b4f62922c4ed94bb8a0f77e474ee385d) )
	ROM_LOAD( "dream maker.bin", 0x0000, 0x020000, CRC(49c19eb3) SHA1(a55d4f9a0dd2b1db41fb28f475efa7e9f7c85be6) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	ROM_LOAD32_BYTE( "u9-801",  0x000000, 0x100000, CRC(e560eeff) SHA1(fe33927a91be7ecd2a283bb09b87f5f3d659cf09) )
	ROM_LOAD32_BYTE( "u11-801", 0x000001, 0x100000, CRC(693cec8e) SHA1(83d33603fa11aa4341a40b2ffc4862992307dcfc) )
	ROM_LOAD32_BYTE( "u13-801", 0x000002, 0x100000, CRC(8b78f6aa) SHA1(74804c44124b71f0f11446da342d0548130394f6) )
	ROM_LOAD32_BYTE( "u15-801", 0x000003, 0x100000, CRC(bdc064f5) SHA1(a6f1f200d3340bb4963dc435cb3262fc31ed557f) )
	ROM_LOAD32_BYTE( "u8-801",  0x400000, 0x100000, CRC(1cea4896) SHA1(9dfe38a7631c1d425f9f40fe801752a270000218) )
	ROM_LOAD32_BYTE( "u10-801", 0x400001, 0x100000, CRC(4d98843b) SHA1(349a6c41b626661e7807015c31dee6fca7e2be91) )
	ROM_LOAD32_BYTE( "u12-801", 0x400002, 0x100000, CRC(75978563) SHA1(fd73322d74b1ea0431962985d0ed94d148de8eba) )
	ROM_LOAD32_BYTE( "u14-801", 0x400003, 0x100000, CRC(416bcc36) SHA1(9e550f49ba63335b21f45786b87aa6e5b42f2acb) )

	ROM_REGION( 0x400000, "u18u21", 0 ) // sound
	ROM_LOAD( "u18-801", 0x0000, 0x100000, CRC(ee2dabf6) SHA1(8fbace51d38d354318d223f259e5b5ae9d922ec5) )
	ROM_LOAD( "u19-801", 0x0000, 0x100000, CRC(bb0029ec) SHA1(ab460d40ee46ee43b195b7a2ece42bcaaa043892) )
	ROM_LOAD( "u20-801", 0x0000, 0x100000, CRC(d9fe585c) SHA1(d5e6d407ce67e7d536a68fcfe834c66cf365716d) )
	ROM_LOAD( "u21-801", 0x0000, 0x100000, CRC(d5e81837) SHA1(ebbd5a3a73c3f440e518103da7e8db8c0818f351) )
ROM_END




ROM_START( jumpjkpt )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "o_-jujp01-abaaa-ca-std_u02.bin", 0x0000, 0x100000, CRC(d384b881) SHA1(0816fd33578b28c605978dd306b110c421bf5793) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "jujp01-a-za-std_-5_u06.bin", 0x0000, 0x020000, CRC(0f19b0c1) SHA1(c118215bcf502287277c34e6f389af70ab945674) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	ROM_LOAD32_BYTE( "jujp01-a_-b-std_-5_u09.bin", 0x000000, 0x100000, CRC(7d3cb293) SHA1(e9f102620f01309327678e115e206fd29dcffde6) )
	ROM_LOAD32_BYTE( "jujp01-a_-b-std_-5_u11.bin", 0x000001, 0x100000, CRC(d92c0c7e) SHA1(680032b81e76c74539ff56f8c5fc7d4d16fd4793) )
	ROM_LOAD32_BYTE( "jujp01-a_-b-std_-5_u13.bin", 0x000002, 0x100000, CRC(555ced70) SHA1(1ec115a2e2a1c171070775913a3eb831efc81dab) )
	ROM_LOAD32_BYTE( "jujp01-a_-b-std_-5_u15.bin", 0x000003, 0x100000, CRC(78d603e5) SHA1(eee3953acedcfedb0118328c37d8a13c72a222f3) )
	ROM_LOAD32_BYTE( "jujp01-a_-b-std_-5_u08.bin", 0x400000, 0x100000, CRC(3c96ccd3) SHA1(b3a05b5cd1200b177f2f9e5f0d0a4870efb9ce29) )
	ROM_LOAD32_BYTE( "jujp01-a_-b-std_-5_u10.bin", 0x400001, 0x100000, CRC(ae8f94ea) SHA1(bf747b63847bd60c3a387e723b4253e546d1eb80) )
	ROM_LOAD32_BYTE( "jujp01-a_-b-std_-5_u12.bin", 0x400002, 0x100000, CRC(cbc70102) SHA1(6079afafeea35a5d2b3a1b32076ebcd0c2ad4625) )
	ROM_LOAD32_BYTE( "jujp01-a_-b-std_-5_u14.bin", 0x400003, 0x100000, CRC(c4754688) SHA1(1bf172d28795a7313586e9874036be0f7864d7c2) )

	ROM_REGION( 0x400000, "u18u21", 0 ) // sound
	ROM_LOAD( "jujp01-aa-a-std_-_u18.bin", 0x0000, 0x100000, CRC(fe453a28) SHA1(ec39ac2bd8c7014f61a8db1a8896c771532b6d3b) )
	ROM_LOAD( "jujp01-aa-a-std_-_u19.bin", 0x0000, 0x100000, CRC(273c7212) SHA1(0689fd7e3862d01f258fba9773f460ea4803d0a3) )
	ROM_LOAD( "jujp01-aa-a-std_-_u20.bin", 0x0000, 0x100000, CRC(45290607) SHA1(f7af3cf323c7e9c3b1f86b230e73012c56fbf103) )
	ROM_LOAD( "jujp01-aa-a-std_-_u21.bin", 0x0000, 0x100000, CRC(ca8d5cf5) SHA1(17202eb6235ecbfd6301269fe36887c078fd6292) )
ROM_END



ROM_START( mushmagi )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "mb-u02.bin", 0x0000, 0x100000, CRC(bb36ee69) SHA1(3a9ce792941250277c5a8f53bd94f7c38b2e5130) )

	ROM_REGION( 0x080000, "u6", 0 ) // config?
	ROM_LOAD( "mb-u06.bin", 0x0000, 0x080000, CRC(10e3f3f6) SHA1(458f5be7ee01e361b4c31c099fc721521fbe3864) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	ROM_LOAD32_BYTE( "gb-u09.bin", 0x000000, 0x100000, CRC(f2d82cee) SHA1(78a15f757bbbb1f4f0a5ab889a9807d886a543a8) )
	ROM_LOAD32_BYTE( "gb-u11.bin", 0x000001, 0x100000, CRC(3ead238a) SHA1(21956bd6b24e3281db70b6d28d97ee7bbd9ae75f) )
	ROM_LOAD32_BYTE( "gb-u13.bin", 0x000002, 0x100000, CRC(58c191e6) SHA1(b0f86f407958de2b8e0f5e61288f8b6c7a2c0c2f) )
	ROM_LOAD32_BYTE( "gb-u15.bin", 0x000003, 0x100000, CRC(1861443a) SHA1(df7a86ee1655f92e9f81aba2f56b6529bdf227b9) )
	ROM_LOAD32_BYTE( "gb-u08.bin", 0x400000, 0x100000, CRC(9b85ef07) SHA1(c325fb8cc65a06d3aa0ac48ae539301ea42d45b6) )
	ROM_LOAD32_BYTE( "gb-u10.bin", 0x400001, 0x100000, CRC(e456c439) SHA1(d0dc09488ca66a9ca648c62b0711792682e2d015) )
	ROM_LOAD32_BYTE( "gb-u12.bin", 0x400002, 0x100000, CRC(972ffd8c) SHA1(b8ba4b4a49cdb27c5aaa9b2594db5a1981c70c13) )
	ROM_LOAD32_BYTE( "gb-u14.bin", 0x400003, 0x100000, CRC(952ada33) SHA1(976492f12db27cedc3219e29536a3256d7ae5675) )

	ROM_REGION( 0x400000, "u18u21", 0 ) // sound
	ROM_LOAD( "sb-u18.bin", 0x0000, 0x100000, CRC(3ee4c1bd) SHA1(6885585ca1f40790eafac3161de3bcca9a2117c7) )
	ROM_LOAD( "sb-u19.bin", 0x0000, 0x100000, CRC(70279081) SHA1(15933d81af85b2c6f831e765f2a4e4f0e44fdc18) )
	ROM_LOAD( "sb-u20.bin", 0x0000, 0x100000, CRC(f92276e5) SHA1(62628e9ef166607c42e873d116116dee6bf9b623) )
	ROM_LOAD( "sb-u21.bin", 0x0000, 0x100000, CRC(3c168653) SHA1(ce77851f64cf14e5b074f667d3a723097f78496f) )
ROM_END


ROM_START( splmastr )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "mb-u2 24e1.bin", 0x0000, 0x100000, CRC(e601e214) SHA1(7ff898245198350ea53ca1c3a71b491d55f60880) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "mb-u6 ebda.bin", 0x0000, 0x020000, CRC(7e73e9c7) SHA1(a8b00af9a3bf936e54391a96777ac78773b3cee0) )
	ROM_LOAD( "speel master.bin", 0x0000, 0x020000, CRC(04168ab7) SHA1(70a387599bf6629a9a8a6ff38ed0d40e92e54504) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	ROM_LOAD32_BYTE( "gb-u9 b408.bin",  0x000000, 0x100000, CRC(e7146c72) SHA1(21b143ae93a73dd59b652a0033ceaa9116575239) )
	ROM_LOAD32_BYTE( "gb-u11 abf6.bin", 0x000001, 0x100000, CRC(de54f849) SHA1(b628a69c8ad5f81543cd78c458dd9348226114a7) )
	ROM_LOAD32_BYTE( "gb-u13 6526.bin", 0x000002, 0x100000, CRC(e5744b4f) SHA1(8c36b087dc4fad6cd463abea5b1e7c0bd9c30074) )
	ROM_LOAD32_BYTE( "gb-u15 9714.bin", 0x000003, 0x100000, CRC(56ead7e0) SHA1(9e0f57b5ba3f299d0e47a40dccfe759fe3423451) )
	ROM_LOAD32_BYTE( "gb-u8 bf70.bin",  0x400000, 0x100000, CRC(30b86d06) SHA1(02026f680f827cc42d45092ca34ca2b63a764ef5) )
	ROM_LOAD32_BYTE( "gb-u10 e2cb.bin", 0x400001, 0x100000, CRC(1286c929) SHA1(6f69242224341d820d57c9381d613e59ef662622) )
	ROM_LOAD32_BYTE( "gb-u12 1737.bin", 0x400002, 0x100000, CRC(90a46172) SHA1(d8aebe2964b55a24169cf15af083edad92969f2d) )
	ROM_LOAD32_BYTE( "gb-u14 f911.bin", 0x400003, 0x100000, CRC(6d77b13c) SHA1(dab85782a98cc5c21f236044dea0c7617b40469b) )

	ROM_REGION( 0x400000, "u18u21", 0 ) // sound
	ROM_LOAD( "sb-u18 e1e5.bin", 0x0000, 0x100000, CRC(b0eb4ceb) SHA1(410990faeef6e371205a4344622b88d0db1e09e9) )
	ROM_LOAD( "sb-u19 7d08.bin", 0x0000, 0x100000, CRC(70279081) SHA1(15933d81af85b2c6f831e765f2a4e4f0e44fdc18) )
	ROM_LOAD( "sb-u20 06ad.bin", 0x0000, 0x100000, CRC(fe354878) SHA1(44abb1a6ba5234c4909ef9f0a2f8b353b0695ff0) )
	ROM_LOAD( "sb-u21 4f56.bin", 0x0000, 0x100000, CRC(54d631a2) SHA1(1fe4278642b5c01e863af5c2ac7ee38d7c94d776) )
ROM_END

ROM_START( tajmah )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "ovtddfxe.8u2", 0x0000, 0x100000, CRC(83639f76) SHA1(f982222f8ae635c34413b316fe55de76fdc8535e) )
	ROM_LOAD( "vtddfxd.801", 0x0000, 0x100000, CRC(b0eb5468) SHA1(09e8ceca4cf3bea6447b9c9c0ee5776cbd32f098) )

	ROM_REGION( 0x040000, "u6", 0 ) // config?
	ROM_LOAD( "590f4-d.u6", 0x0000, 0x040000, CRC(b346765d) SHA1(567ae4fa740a4bd26485b72f3fd0e57d7a18512e) )
	ROM_LOAD( "t510f14b.2u6", 0x0000, 0x040000, CRC(ff6add95) SHA1(b1fa169e61a774d1ce5c0c1b4f80baa289ca696e) )
	ROM_LOAD( "t595f03c.2u6", 0x0000, 0x040000, CRC(b173164c) SHA1(91390cc2568de1c61df6b427bec9060fa7b0829a) )
	ROM_LOAD( "t595f07b.u6", 0x0000, 0x040000, CRC(7ef35e2f) SHA1(d2b4d392d66784900bf8413d4adbd1e37de11a67) )

	ROM_REGION16_LE( 0x800000, "user2", 0 ) /* TMS34020APCM-40 code (34020) */
	ROM_LOAD32_BYTE( "t5_d01d.u9",  0x400000, 0x080000, CRC(d1a0e88b) SHA1(c68c9be2413725b698fccda74c120a732bc1b5ab) )
	ROM_LOAD32_BYTE( "t5_d01d.u11", 0x400001, 0x080000, CRC(e063b35e) SHA1(54fb8831e4529b85740f22ddb181649ebfad8f03) )
	ROM_LOAD32_BYTE( "t5_d01d.u13", 0x400002, 0x080000, CRC(9d3f04d2) SHA1(a6f6b564f7165aa992bc0dd8c090fa8cecd1953c) )
	ROM_LOAD32_BYTE( "t5_d01d.u15", 0x400003, 0x080000, CRC(f28beede) SHA1(a76e0fd9c71e171d4f861816e7944b008270ab9b) )
	ROM_LOAD32_BYTE( "t5_d01d.u8",  0x600000, 0x080000, CRC(744ee5f8) SHA1(6f211252042dff087327c4602b9294e7222254d3) )
	ROM_LOAD32_BYTE( "t5_d01d.u10", 0x600001, 0x080000, CRC(79448073) SHA1(b7108e048830587b67cc0dbd040bb09667b955a3) )
	ROM_LOAD32_BYTE( "t5_d01d.u12", 0x600002, 0x080000, CRC(c2ca0b17) SHA1(cfd553f1943552620f6d324f767fd1f1957e8e25) )
	ROM_LOAD32_BYTE( "t5_d01d.u14", 0x600003, 0x080000, CRC(0d512057) SHA1(f5f43dad25940193516d467725ecbe1989cc9003) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) - is this an alt set, or a 2nd video board? */
	ROM_LOAD32_BYTE( "u9",  0x400000, 0x080000, CRC(157b9860) SHA1(6e04f035a945a63617e32b196fa0c1b6fd26b281) )
	ROM_LOAD32_BYTE( "u11", 0x400001, 0x080000, CRC(6aa2cecc) SHA1(7b1d6bb81fed7413f69e926e7cefe1ee171453b4) )
	ROM_LOAD32_BYTE( "u13", 0x400002, 0x080000, CRC(5c091b7a) SHA1(d1c758a6d155bbc7359f3f46a29bac44d96ec4b1) )
	ROM_LOAD32_BYTE( "u15", 0x400003, 0x080000, CRC(ddbfc62c) SHA1(1884887930d9b3889aabd617b8339e9636c45e3e) )
	ROM_LOAD32_BYTE( "u8",  0x600000, 0x080000, CRC(235fd293) SHA1(48cd8773ceb46318be0aa2e9eb93c78d9d2b5f3d) )
	ROM_LOAD32_BYTE( "u10", 0x600001, 0x080000, CRC(c0db4621) SHA1(59454317bb0fa92c408ed14b3e0023aa2f63de4d) )
	ROM_LOAD32_BYTE( "u12", 0x600002, 0x080000, CRC(f0d055d2) SHA1(52457055db3b15b9031b0748e5d60dea2ad4707d) )
	ROM_LOAD32_BYTE( "u14", 0x600003, 0x080000, CRC(e85eebf4) SHA1(99e7ac920a61f69fb480e35b747fecfff4f6a3e3) )

	// sound (missing or not needed here? there is an OKI M6585 on the mainboard)
	/*
	ROM_REGION( 0x400000, "u18u21", 0 )
	ROM_LOAD( "u18", 0x0000, 0x100000, NO_DUMP )
	ROM_LOAD( "u19", 0x0000, 0x100000, NO_DUMP )
	ROM_LOAD( "u20", 0x0000, 0x100000, NO_DUMP )
	ROM_LOAD( "u21", 0x0000, 0x100000, NO_DUMP )
	*/
ROM_END


ROM_START( 3wishrd )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "3w-baca.8u2", 0x0000, 0x100000, CRC(14ca9f18) SHA1(8bf5eaa11ca70d14c7ed69a17c4610ecca6f76f8) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "3590a25a.1u6", 0x0000, 0x020000, CRC(8a10399e) SHA1(9658705821cdd39e1022a2b63dd61355d44c23f6) )
	ROM_LOAD( "3590a26a.1u6", 0x0000, 0x020000, CRC(86ec866d) SHA1(1fbcbfec49900e45ed7866857b0314de07020405) )
	ROM_LOAD( "35b0a03a.1u6", 0x0000, 0x020000, CRC(a25650d7) SHA1(72c1c58cf933c7b6fc85c071f48768e482e99ff7) )
	ROM_LOAD( "35b0a04a.1u6", 0x0000, 0x020000, CRC(36930b7c) SHA1(23d578450fc3185389058da367345ac20883b6f4) )
	ROM_LOAD( "three wishes.bin", 0x0000, 0x020000, CRC(37d85da7) SHA1(64db855e06dab5ea85c669bd72f1e8ee8856607a) )
	ROM_LOAD( "590a13a.1u6", 0x0000, 0x020000, CRC(3e674907) SHA1(ca933c416764ebf355d8e04f871f8421c9039078) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	ROM_LOAD32_BYTE( "tw5b01a.8u9", 0x000000, 0x100000, CRC(2410659a) SHA1(2bcd2539c0e3e7389c27c21e58d9199b9c7c742e) )
	ROM_LOAD32_BYTE( "tw5b01a.u11", 0x000001, 0x100000, CRC(44ca9ce1) SHA1(b1c6d83f749202c072c6ce99c0470a31cfab8986) )
	ROM_LOAD32_BYTE( "tw5b01a.u13", 0x000002, 0x100000, CRC(6c60097b) SHA1(f5ddb86b481b7b95d6ec151b37d662b583817813) )
	ROM_LOAD32_BYTE( "tw5b01a.u15", 0x000003, 0x100000, CRC(07f813c3) SHA1(02447ae735aa34451538abd6625061cea91672e9) )
	ROM_LOAD32_BYTE( "tw5b01a.8u8", 0x400000, 0x100000, CRC(0c58220a) SHA1(840e411b6baee6518df23920448ecde76102a9dd) )
	ROM_LOAD32_BYTE( "tw5b01a.u10", 0x400001, 0x100000, CRC(c4d871cc) SHA1(0a89f19401e3b14473db42d995e0879b9ffc973c) )
	ROM_LOAD32_BYTE( "tw5b01a.u12", 0x400002, 0x100000, CRC(248c0cdf) SHA1(033b01c5202709b709a4b1b59b8d257e52bcff66) )
	ROM_LOAD32_BYTE( "tw5b01a.u14", 0x400003, 0x100000, CRC(85f30657) SHA1(e055e1fdeeab2fd82bb23b80416b421eb9143967) )

	ROM_REGION( 0x400000, "u18u21", 0 ) // sound
	ROM_LOAD( "wishaaa.u18", 0x0000, 0x100000, CRC(c258c2c7) SHA1(900497e2e023e2ac1b62c5dabfaf95e5bf0b855b) )
	ROM_LOAD( "wishaaa.u19", 0x0000, 0x100000, CRC(09d186a9) SHA1(1d784fd3591583c99fac516a8c4cf47c3932d084) )
	ROM_LOAD( "wishaaa.u20", 0x0000, 0x100000, CRC(be187745) SHA1(c96458bbc9164a4ca3bca94f0ac5a4fe9f1b1dfa) )
	ROM_LOAD( "wishaaa.u21", 0x0000, 0x100000, CRC(466b34e1) SHA1(3f5236ea78bde8bfb998367e913f42d23b4c17f1) )
ROM_END


ROM_START( atrwild )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "mb-u2.bin", 0x0000, 0x80000, CRC(e72a2339) SHA1(ad191dbbd0ac1f3288c45e336f27f693877273a9) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "mb-u6.bin", 0x0000, 0x020000, CRC(f310e88d) SHA1(5d354e2a9de9eff27e66a4b1ca0925b19c6c86cc) )

	ROM_REGION( 0x117, "mbpals", 0 )
	ROM_LOAD( "mb-u22-d.bin", 0x000, 0x117, CRC(dc097847) SHA1(305294284d0ffd578f9115b836ef1f9e906c1599) )
	ROM_LOAD( "mb-u32-b.bin", 0x000, 0x117, CRC(78a9310b) SHA1(deb84d96b0411b05c54fb2c998bed020a37d5005) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	ROM_LOAD32_BYTE( "u8.bin",  0x600000, 0x080000, CRC(4acafd98) SHA1(d516c55ddce1470e4e19725b6d7dfd5f70ba1129) )
	ROM_LOAD32_BYTE( "u10.bin", 0x600001, 0x080000, CRC(804800be) SHA1(5fb2a5479c2a7073c2abd40e14a162fbf783eb70) )
	ROM_LOAD32_BYTE( "u12.bin", 0x600002, 0x080000, CRC(0845ff27) SHA1(5012569a79c9fcbee178a0cee45d25769a1cf9be) )
	ROM_LOAD32_BYTE( "u14.bin", 0x600003, 0x080000, CRC(81e06b01) SHA1(67f356670b1e409b139186d7898bbc470ddb770b) )

	// no dedicated sound ROM board present in cage, missing or not needed here? there is an OKI M6585 on the mainboard
ROM_END

ROM_START( atricmon )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "mb-u2.bin", 0x0000, 0x80000, CRC(0fc3f42f) SHA1(53d189205ede16bc0199a6163a11eb39c2e3f2f1) )

	ROM_REGION( 0x040000, "u6", 0 ) // config?
	ROM_LOAD( "mb-u6.bin", 0x0000, 0x040000, CRC(eb8d3b4f) SHA1(598fb921026685ccfdb0b1804d92b3c6b3313ff4) )

	ROM_REGION( 0x2e5, "mbpals", 0 )
	ROM_LOAD( "mb-u22-d.bin", 0x000, 0x117, CRC(dc097847) SHA1(305294284d0ffd578f9115b836ef1f9e906c1599) )
	ROM_LOAD( "mb-u32-d.bin", 0x000, 0x2e5, CRC(996854bc) SHA1(647d2f49b739f7ca55c0b85290b6a21256834fd8) )
	ROM_LOAD( "mb-u35-65994077_icm-s.bin", 0x000, 0x2e5, CRC(996854bc) SHA1(647d2f49b739f7ca55c0b85290b6a21256834fd8) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	ROM_LOAD32_BYTE( "gb-u9.bin",  0x000000, 0x100000, CRC(eff83f95) SHA1(743f2fb0dd84a83387919db70175aa62f5f6f858) )
	ROM_LOAD32_BYTE( "gb-u11.bin", 0x000001, 0x100000, CRC(3fc27ae9) SHA1(896da175c11b48fb28dbb0678849b8f167cf5f6e) )
	ROM_LOAD32_BYTE( "gb-u13.bin", 0x000002, 0x100000, CRC(6ad50f67) SHA1(b32781f06acc3e9929467d6d1212cf0dc757e5b3) )
	ROM_LOAD32_BYTE( "gb-u15.bin", 0x000003, 0x100000, CRC(6ae46bb3) SHA1(edc51f9a885c483283edb9b0873b980727205a91) )
	ROM_LOAD32_BYTE( "gb-u8.bin",  0x400000, 0x100000, CRC(7dee3392) SHA1(718333ad5552351702e95a76cc2b61f7c3bf14ac) )
	ROM_LOAD32_BYTE( "gb-u10.bin", 0x400001, 0x100000, CRC(db88f900) SHA1(83638b46fd7b6e4229fa5295479c9763c2f690c0) )
	ROM_LOAD32_BYTE( "gb-u12.bin", 0x400002, 0x100000, CRC(fcbada90) SHA1(3206409c9a689e196694831ff5e6ba0fd32d676a) )
	ROM_LOAD32_BYTE( "gb-u14.bin", 0x400003, 0x100000, CRC(055c7ed6) SHA1(8b8f537b8dfd898d2f9eb123303f9488a6ae4567) )

	// no dedicated sound ROM board present in cage, missing or not needed here? there is an OKI M6585 on the mainboard
ROM_END

ROM_START( atrbonpk )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "mb-u2.bin", 0x0000, 0x80000, CRC(7f9d9d3c) SHA1(501d3f1482b3c67cbc94a3af40ef31fb5a6e7921) )

	ROM_REGION( 0x080000, "u6", 0 ) // config?
	ROM_LOAD( "mb-u6.bin", 0x0000, 0x080000, CRC(0b8d47ba) SHA1(aa1d5b37c330f4f44c1af5caca24bbf670c0bbcb) )

	ROM_REGION( 0x2e5, "mbpals", 0 )
	ROM_LOAD( "mb-u22-d.bin", 0x000, 0x117, CRC(dc097847) SHA1(305294284d0ffd578f9115b836ef1f9e906c1599) )
	ROM_LOAD( "mb-u32-i.bin", 0x000, 0x2e5,  CRC(996854bc) SHA1(647d2f49b739f7ca55c0b85290b6a21256834fd8))
	ROM_LOAD( "mb-u35-poke-s6366.bin", 0x000, 0x2e5, CRC(996854bc) SHA1(647d2f49b739f7ca55c0b85290b6a21256834fd8) )

	ROM_REGION( 0x117, "gfxpals", 0 )
	ROM_LOAD( "u2-f.bin", 0x000, 0x117, CRC(eb5548c2) SHA1(7ccc6a7de0c0765e0da3563f3ee83dd99acc50bf) )
	ROM_LOAD( "u3-i.bin", 0x000, 0x117, CRC(769f7b32) SHA1(72df7c92367403a95c5bda3d6a643cc8fc24e153) )
	ROM_LOAD( "u4-c.bin", 0x000, 0x117, CRC(91e043ea) SHA1(d4d0c721c6c37cbe8babe148bc9887038cdf4820) )
	ROM_LOAD( "u5-a.bin", 0x000, 0x117, CRC(18ffc746) SHA1(e32dd74fb535fd4a754579173a788712b3e6ec30) )
	ROM_LOAD( "u6-b.bin", 0x000, 0x117, CRC(2750fb0a) SHA1(3814c4755a215073425a9d6bb048315498962c76) )
	ROM_LOAD( "u7-a.bin", 0x000, 0x117, CRC(adcb2789) SHA1(cc2ebd69abec73d66665faaec19b8706e539b34c) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	ROM_LOAD32_BYTE( "u9.bin",  0x400000, 0x080000, CRC(5b8450f1) SHA1(27fc771c3fb824cdb845237324984778fcd0a737) )
	ROM_LOAD32_BYTE( "u11.bin", 0x400001, 0x080000, CRC(c8c52bd1) SHA1(081b8b4c46f18d030329bf519a8ed50385f7c062) )
	ROM_LOAD32_BYTE( "u13.bin", 0x400002, 0x080000, CRC(23164a85) SHA1(e6de6aac28f1dac9ea908aaab9760b56ded1bb91) )
	ROM_LOAD32_BYTE( "u15.bin", 0x400003, 0x080000, CRC(aabbb4ff) SHA1(4a13475929141a4824b15347873cf330f7f7b0d0) )
	ROM_LOAD32_BYTE( "u8.bin",  0x600000, 0x080000, CRC(d6dfde87) SHA1(f3221adecb67ee593d52a1bbbdcee78dde497dbd) )
	ROM_LOAD32_BYTE( "u10.bin", 0x600001, 0x080000, CRC(d81a1f77) SHA1(c3ec3a06dacc3f528c9bcfa7a18e25e0126b1d85) )
	ROM_LOAD32_BYTE( "u12.bin", 0x600002, 0x080000, CRC(4b24dc03) SHA1(f875f09c6d44821b169e111cb1ea3d9716746d5f) )
	ROM_LOAD32_BYTE( "u14.bin", 0x600003, 0x080000, CRC(a6dc78ff) SHA1(96457601e7f90ce14a88765f70accc07d7236d30) )

	// no dedicated sound ROM board present in cage, missing or not needed here? there is an OKI M6585 on the mainboard
ROM_END


ROM_START( abigchs )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "o_-bigc01-afbaa-cc-rus_.8mu02", 0x0000, 0x100000, CRC(969082d8) SHA1(f3bcdc631ac1c346993a8d7300ba6687a32669f7) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "bigc21-d-zf-std_-5-xx-xx-axx.1mu06", 0x0000, 0x020000, CRC(0eb376fb) SHA1(34e1f28e71503ffb0e1e922bd3ba17bad0d37d99) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	// not valid TMS code, looks like some x86 drive image split into ROMs?
	ROM_LOAD( "bigc01-a_-f-rus_-5_-g101.wigu09", 0x000000, 0x100000, CRC(c87e6bb4) SHA1(387e2498625ff718fccaa7701dd595ee787b9a83) )
	ROM_LOAD( "bigc01-a_-f-rus_-5_-g101.wigu11", 0x100000, 0x100000, CRC(c9e9fa7f) SHA1(1698215845f21cfde0274e880d89c66fb3226f04) )
	ROM_LOAD( "bigc01-a_-f-rus_-5_-g101.wigu13", 0x200000, 0x100000, CRC(d5f5bb9a) SHA1(d8ecdb16ef4f18c200a1d3c3cdfe4db37292cf6f) )
	ROM_LOAD( "bigc01-a_-f-rus_-5_-g101.wigu15", 0x300000, 0x060600, CRC(b223662f) SHA1(0bc9cb6d33935d80365cc1e13869bea4fd98fbd1) )
	ROM_LOAD( "bigc01-a_-f-rus_-5_-g101.wigu08", 0x400000, 0x100000, CRC(f65c48a9) SHA1(4fec0fdcc13cf1fe26cf539518c4f0102ce4f2cb) )
	ROM_LOAD( "bigc01-a_-f-rus_-5_-g101.wigu10", 0x500000, 0x100000, CRC(b941669a) SHA1(70e13f8dff92f3821ef72789a7ef2e622c6c8ba3) )
	ROM_LOAD( "bigc01-a_-f-rus_-5_-g101.wigu12", 0x600000, 0x100000, CRC(e5c5ca4c) SHA1(4b1b4d73266a269697e54f0fada5c2c2e197c3be) )
	ROM_LOAD( "bigc01-a_-f-rus_-5_-g101.wigu14", 0x700000, 0x100000, CRC(5cf1c75a) SHA1(951289b5cad7ede582da93103acacc41af7622d9) )

	ROM_REGION( 0x400000, "u18u21", 0 ) // sound
	ROM_LOAD( "bigc01-ba-a-std_-_.8su18", 0x0000, 0x100000, CRC(79c99749) SHA1(bcdd61ff287877833ab6ca56a278b1d68e47608f) )
	ROM_LOAD( "bigc01-ba-a-std_-_.8su19", 0x0000, 0x100000, CRC(deb7a0b5) SHA1(d8526e42273003f8249007df2d8b6ba33b727324) )
	ROM_LOAD( "bigc01-ba-a-std_-_.8su20", 0x0000, 0x100000, CRC(f94998a7) SHA1(c0fced89584ce5b67ba68cd93f1f8348ac36fd26) )
	ROM_LOAD( "bigc01-ba-a-std_-_.8su21", 0x0000, 0x100000, CRC(08a34088) SHA1(009311d126eb78514133f0f6ef28548c42d50b1c) )
ROM_END

ROM_START( bearnec )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "u2_0_-bene01-afaaa-ce-rus_b178.bin", 0x0000, 0x100000, CRC(fc71f0b8) SHA1(6c124211614101ef151fe405bef0ee88277b8d2b) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "u6_bene21-e-zg-std_-5-xx-xx-axx_0f78.bin", 0x0000, 0x020000, CRC(d956484f) SHA1(d2d659a4350d7204666234a511ebd4dd7a021d89) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	// not valid TMS code, looks like some x86 drive image split into ROMs?
	ROM_LOAD( "u09_a632.bin", 0x000000, 0x100000, CRC(a671b6e8) SHA1(86b97ba98fdd09575a371b5b7f7d42bf2916fe17) )
	ROM_LOAD( "u11_947b.bin", 0x100000, 0x100000, CRC(3dc60963) SHA1(d824cd4fbe4116744727180762fbf0ffe22e6398) )
	ROM_LOAD( "u13_60e8.bin", 0x200000, 0x100000, CRC(11b25ede) SHA1(07d4901e985ed9b83d8630b748b82a408e26bac6) )
	ROM_LOAD( "u15_6c49.bin", 0x300000, 0x100000, CRC(f56e40ac) SHA1(7c5e0bb7a8bafea6ae57b5933c93230478ac74b2) )
	ROM_LOAD( "u08_f54f.bin", 0x400000, 0x100000, CRC(ce601695) SHA1(585b9e7c57072e7bff03fd34748fdadab6ff1b08) )
	ROM_LOAD( "u10_d622.bin", 0x500000, 0x100000, CRC(4ac9e636) SHA1(35d0bfd6c456e2cd5d1f950ea58edc8e92e05933) )
	ROM_LOAD( "u12_8bfa.bin", 0x600000, 0x100000, CRC(fa722d3f) SHA1(1275d0f6f06604e736a03b6031b7aeb46d7c6f07) )
	ROM_LOAD( "u14_d9c6.bin", 0x700000, 0x100000, CRC(fd9e0ebf) SHA1(0c6b2ddb397994ca62e80cd7c802a778fc287549) )

	ROM_REGION( 0x400000, "u18u21", 0 ) // sound
	ROM_LOAD( "u18_96c0.bin", 0x0000, 0x100000, CRC(0cf7eb95) SHA1(96e6f21b359198b0f893ed69d2bc23ad2db34f33) )
	ROM_LOAD( "u19_7c1c.bin", 0x0000, 0x100000, CRC(17ca92ee) SHA1(cdc4297c591db33a75ab716db7cf5620c13e8a84) )
	ROM_LOAD( "u20_3123.bin", 0x0000, 0x100000, CRC(0932857c) SHA1(a30c1e40811581230da72c384679c0c21cced4c2) )
	ROM_LOAD( "u21_8bb6.bin", 0x0000, 0x100000, CRC(c1a25921) SHA1(d54eb230c8ebde69f00bfab1088b7a39809e5ee2) )
ROM_END

ROM_START( goldcity )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "o_-goci01-afaaa-cd-rus_.8mu02", 0x0000, 0x100000, CRC(59c19539) SHA1(7c40eee8e534795a44b33140535284b2bc2a9ac5) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "goci21-e-zf-std_-5-xx-xx-axx.1mu06", 0x0000, 0x020000, CRC(73ab9c41) SHA1(0888923bdaede83f264979c0757894f5cb2e0ec8) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	// not valid TMS code, looks like some x86 drive image split into ROMs?
	ROM_LOAD( "goci01-a_-c-rus_-5_-g101.wigu09", 0x000000, 0x100000, CRC(72c9b584) SHA1(1345e7ea34a819fbc01b9a64e9f9c1a2de927dda) )
	ROM_LOAD( "goci01-a_-c-rus_-5_-g101.wigu11", 0x100000, 0x100000, CRC(2ebe1d71) SHA1(1b540c3bb9b232f475c3fe2b56c55f473d8c09ee) )
	ROM_LOAD( "goci01-a_-c-rus_-5_-g101.wigu13", 0x200000, 0x100000, CRC(26103a89) SHA1(322d377918e708b0e4ddda0d5cfb19eba8945219) )
	ROM_LOAD( "goci01-a_-c-rus_-5_-g101.wigu15", 0x300000, 0x100000, CRC(1346284a) SHA1(8975248a4566d0a14451aca71e337058676b6ae5) )
	ROM_LOAD( "goci01-a_-c-rus_-5_-g101.wigu08", 0x400000, 0x100000, CRC(4a0b3af2) SHA1(5fd2cb52ea5403347f6a0d54ce1d7c36bd012364) )
	ROM_LOAD( "goci01-a_-c-rus_-5_-g101.wigu10", 0x500000, 0x100000, CRC(9ae4af09) SHA1(6d85ab9a91308762171b7b3efccd42737d516ebd) )
	ROM_LOAD( "goci01-a_-c-rus_-5_-g101.wigu12", 0x600000, 0x100000, CRC(e60075f1) SHA1(5674a9bdbb3ce98a75f214620209973f3d42d5c7) )
	ROM_LOAD( "goci01-a_-c-rus_-5_-g101.wigu14", 0x700000, 0x100000, CRC(da4b5130) SHA1(872cdf862e1a989fbd451e10ea9cc46d8e129766) )

	// sound (missing or not needed here? there is an OKI M6585 on the mainboard)
	/*
	ROM_REGION( 0x400000, "u18u21", 0 )
	ROM_LOAD( "u18", 0x0000, 0x100000, NO_DUMP )
	ROM_LOAD( "u19", 0x0000, 0x100000, NO_DUMP )
	ROM_LOAD( "u20", 0x0000, 0x100000, NO_DUMP )
	ROM_LOAD( "u21", 0x0000, 0x100000, NO_DUMP )
	*/
ROM_END

ROM_START( santam )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* Z8018010VSC code (Z180) */
	ROM_LOAD( "u02_8m_m27c801-100f1_d6f7h.bin", 0x0000, 0x100000, CRC(8c0ed828) SHA1(e24cd0783a4290799db11ca8764b70cd380f2879) )

	ROM_REGION( 0x020000, "u6", 0 ) // config?
	ROM_LOAD( "u06_1m_m27c1001-10f_da21h.bin", 0x0000, 0x020000, CRC(51c0a380) SHA1(861c8b4f825f4bc11dd02ac03dcc2cc7e8c65129) )

	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* TMS34020APCM-40 code (34020) */
	// not valid TMS code, looks like some x86 drive image split into ROMs?
	ROM_LOAD( "gb_u09_8m_m27c801-100f1_9df6h.bin", 0x000000, 0x100000, CRC(470ccae5) SHA1(0521af7830cc59102edcc658df4d21a3d669d6db) )
	ROM_LOAD( "gb_u11_8m_m27c801-100f1_7621h.bin", 0x100000, 0x100000, CRC(8f9a1031) SHA1(1aca654b62e73f3005e627625bea2b4198c04a99) )
	ROM_LOAD( "gb_u13_8m_m27c801-100f1_be61h.bin", 0x200000, 0x100000, CRC(8b65f945) SHA1(40077a7d7ab13945df96f2483cb4f11ad8ae525c) )
	ROM_LOAD( "gb_u15_8m_m27c801-100f1_2857h.bin", 0x300000, 0x100000, CRC(8b688a66) SHA1(a343d8039b1c1c8edc717f3bf8a101a8d3069d23) )
	ROM_LOAD( "gb_u08_8m_m27c801-100f1_ac13h.bin", 0x400000, 0x100000, CRC(84a6f213) SHA1(296b8b8d97b0bc543b9e747c751a6e9f4a6dc26f) )
	ROM_LOAD( "gb_u10_8m_m27c801-100f1_03abh.bin", 0x500000, 0x100000, CRC(736acaa0) SHA1(1b2a4da15deba54dab76bf1fe88f0f4986217a31) )
	ROM_LOAD( "gb_u12_8m_m27c801-100f1_5953h.bin", 0x600000, 0x100000, CRC(cc0b9bce) SHA1(30e0594c6890deddb665b6169c47e640508c6b7f) )
	ROM_LOAD( "gb_u14_8m_m27c801-100f1_2a48h.bin", 0x700000, 0x100000, CRC(bd1fdca3) SHA1(b4bc73ff6900c14525d10fb10ca7f5371351a198) )

	ROM_REGION( 0x400000, "u18u21", 0 ) // sound
	ROM_LOAD( "sb_u18_8m_m27c801-100f1_a52eh.bin", 0x0000, 0x100000, CRC(95fe949d) SHA1(953f730a37d8d661cbc8c212c459db3769ac502b) )
	ROM_LOAD( "sb_u19_8m_m27c801-100f1_a71bh.bin", 0x0000, 0x100000, CRC(6eae31f3) SHA1(08fcd8c49b31de874906205a47035a71f87f12d6) )
	ROM_LOAD( "sb_u20_8m_m27c801-100f1_7870h.bin", 0x0000, 0x100000, CRC(27639c24) SHA1(7fdc7e5e684dfbef00450e4c8fa998d73c035895) )
	ROM_LOAD( "sb_u21_8m_m27c801-100f1_94cch.bin", 0x0000, 0x100000, CRC(c740b5be) SHA1(dafe80431197fe22cddd0fc295436edc37256603) )
ROM_END

} // anonymous namespace

/*
 Possible CashLine games:
 * Aphrodite
 * Babooshka? (Alternate spelling)
 * Break the Spell
 * Chickendales (Alternate spelling?)
 * Diver's Dream
 * Golden Glen (Alternate spelling?)
 * Happy Happy Hippy
 * Happy Safari
 * Ice Mondey (Alternate spelling?)
 * Isle of Fun
 * Joker Poker
 * Kismet
 * Mystery Game
 * Mystery Mask
 * Sign of Zodiac
 * Three Wishes
 * Xanadu Magic
 */

GAME( 1999, atronic,   0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Atronic SetUp/Clear Chips (Russia, set 1)", MACHINE_IS_SKELETON)
GAME( 1999, atronica,  atronic,  atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Atronic SetUp/Clear Chips (Russia, set 2)", MACHINE_IS_SKELETON)

GAME( 2002, atlantca,  0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Atlantica (Russia) (Atronic) (set 1)", MACHINE_IS_SKELETON)
GAME( 2002, atlantcaa, atlantca, atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Atlantica (Russia) (Atronic) (set 2)", MACHINE_IS_SKELETON)
GAME( 2002, baboshka,  0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Baboshka (Russia) (Atronic)", MACHINE_IS_SKELETON)
GAME( 2002, cfblue,    0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Crazy Fruits Blue (Russia) (Atronic) (set 1)", MACHINE_IS_SKELETON)
GAME( 2002, cfbluea,   cfblue,   atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Crazy Fruits Blue (Russia) (Atronic) (set 2)", MACHINE_IS_SKELETON)
GAME( 2002, cfgreen,   0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Crazy Fruits Green (Russia) (Atronic)", MACHINE_IS_SKELETON)
GAME( 2002, chicken,   0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Chicken (Russia) (Atronic)", MACHINE_IS_SKELETON)
GAME( 2002, aclown,    0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Clown (Russia) (Atronic)", MACHINE_IS_SKELETON)
GAME( 2002, goldglen,  0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Golden Glenn (Russia) (Atronic)", MACHINE_IS_SKELETON)
GAME( 2002, iccash,    0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "I C Cash (Russia) (Atronic)", MACHINE_IS_SKELETON)
GAME( 2002, shpinxii,  0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Sphinx II (Russia) (Atronic)", MACHINE_IS_SKELETON)
GAME( 2002, beachpt,   0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Beach Patrol (Russia) (Atronic)", MACHINE_IS_SKELETON)
GAME( 2002, beetleup,  0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Beetles Unplugged (Russia) (Atronic)", MACHINE_IS_SKELETON)
GAME( 2002, bigblue,   0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Big Blue (Russia) (Atronic)", MACHINE_IS_SKELETON)
GAME( 2002, castaway,  0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Castaway (Russia) (Atronic) (set 1)", MACHINE_IS_SKELETON)
GAME( 2002, castawaya, castaway, atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Castaway (Russia) (Atronic) (set 2)", MACHINE_IS_SKELETON)
GAME( 2002, dncsprt,   0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Dancing Spirit (Russia) (Atronic)", MACHINE_IS_SKELETON)
GAME( 2002, drmmake,   0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Dream Maker (Russia) (Atronic)", MACHINE_IS_SKELETON)
GAME( 2002, jumpjkpt,  0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Jumping Jackpots (Russia) (Atronic)", MACHINE_IS_SKELETON)
GAME( 2002, mushmagi,  0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Mushroom Magic (Russia) (Atronic)", MACHINE_IS_SKELETON)
GAME( 2002, splmastr,  0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Spell Master (Russia) (Atronic)", MACHINE_IS_SKELETON)
GAME( 2002, tajmah,    0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Tajmahal (Russia) (Atronic)", MACHINE_IS_SKELETON)
GAME( 2002, 3wishrd,   0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Three Wishes Red (Russia) (Atronic)", MACHINE_IS_SKELETON)
GAME( 200?, atrwild,   0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Wild Thing (Atronic)", MACHINE_IS_SKELETON)
GAME( 200?, atricmon,  0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "I C Money (Atronic)", MACHINE_IS_SKELETON) // related to I C Cash ?
GAME( 200?, atrbonpk,  0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Bonus Poker (Atronic)", MACHINE_IS_SKELETON)


// could be different hardware (or just bad dumps) they don't seem to have valid TMS code, instead the video ROMs seem to be some x86 drive image?
GAME( 2002, abigchs,   0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Big Cheese (Russia) (Atronic)", MACHINE_IS_SKELETON)
GAME( 2002, bearnec,   0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Bear Necessities (Russia) (Atronic)", MACHINE_IS_SKELETON)
GAME( 2002, goldcity,  0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Gold City (Russia) (Atronic)", MACHINE_IS_SKELETON)
GAME( 2002, santam,    0,        atronic, atronic, atronic_state, empty_init, ROT0, "Atronic", "Santa Maria (Russia) (Atronic)", MACHINE_IS_SKELETON)
