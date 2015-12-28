// license:BSD-3-Clause
// copyright-holders:smf
/***************************************************************************

  Namco System 10 - Arcade PSX Hardware
  =====================================
  Driver by smf

Namco System 10 Hardware Overview
---------------------------------
Note! This document is a Work-In-Progress and will be updated from time to time when more dumps are available.

This document covers all the known Namco System 10 games, including....
*Drum Master                                     (C) Namco, 2001
*Drum Master 2                                   (C) Namco, 2001
*Drum Master 3                                   (C) Namco, 2002
*Drum Master 4                                   (C) Namco, 2003
*Drum Master 5                                   (C) Namco, 2003
*Drum Master 6                                   (C) Namco, 2004
*Drum Master 7                                   (C) Namco, 2005
*GAHAHA Ippatsu-dou                              (C) Namco/Metro, 2000
*GAHAHA Ippatsu-dou 2                            (C) Namco/Metro, 2001
Gamshara (10021 Ver.A)                           (C) Mitchell, 2003
Gekitoride-Jong Space (10011 Ver.A)              (C) Namco/Metro, 2001
*Golgo-13 3 : Juusei no Chinkonka                (C) Namco/8ing/Raizing, 2001
*Hard Puncher Hajime no Ippo 2 Ouja e no Chousen (C) Namco/Taito, 2002
*Honne Hakkenki                                  (C) Namco, 2001
Kotoba no Puzzle Mojipittan (KPM1 Ver.A)         (C) Namco, 2001
*Kurukuru Food                                   (C) Namco, 2002
Mr Driller 2 (DR21 Ver.A)                        (C) Namco, 2000
Mr Driller 2 (DR22 Ver.A)                        (C) Namco, 2000
Mr Driller G (DRG1 Ver.A)                        (C) Namco, 2001
NFL Classic Football (NCF3 Ver.A)                (C) Namco, 2003 - Has a noticeable red dot on the right of the version printed on PCB.  Something to determine region?
Panicuru Panekuru (PPA1 Ver.A)                   (C) Namco, 2001
*Photo Battole                                   (C) Namco, 2001
Point Blank 3 / Gunbalina (GNN2 Ver. A)          (C) Namco, 2000
*Ren-ai Quiz High School Angel                   (C) Namco, 2002
*Seishun Quiz Colorful High School               (C) Namco, 2002
Star Trigon (STT1 Ver.A)                         (C) Namco, 2002
*Taiko No Tatsujin                               (C) Namco, 2001
*Taiko No Tatsujin 2                             (C) Namco, 2001
*Taiko No Tatsujin 3                             (C) Namco, 2002
*Taiko No Tatsujin 4                             (C) Namco, 2003
*Taiko No Tatsujin 5                             (C) Namco, 2003
*Taiko No Tatsujin 6                             (C) Namco, 2004
*Tsukkomi Yousei Gips Nice Tsukkomi              (C) Namco/Metro, 2002
Uchuu Daisakusen : Chocovader Contactee (CVC1 Ver.A) (C) Namco, 2002

* - denotes not dumped yet.

The Namco System 10 system comprises 2 PCB's....
MAIN PCB - This is the mother board PCB. It holds the main CPU/GPU & SPU and all sound circuitry, program & video RAM,
           controller/input logic and video output circuitry. Basically everything except the ROMs.
           There are three known revisions of this PCB so far. The differences seem very minor. The 2nd and 3rd revision
           have an updated CPLD revision.
           The 3rd revision has an updated model Sony chip. The only other _noticeable_ difference is some component
           shuffling in the sound amplification section to accommodate two extra 1000uF capacitors and one 470uF capacitor
           has been replaced by a 1000uF capacitor. Everything else, including all the PLDs appears to be identical.
           Note there are no ROMs on the Main PCB and also no custom Namco chips on System10, which seem to have been
           phased out. Instead, they have been replaced by (custom programmed) CPLDs, probably due to cost-cutting
           measures within the company, or to reduce the cost of System10 to an entry-level no-frills development platform.
MEM PCB  - There are two known revisions of this PCB (so far). They're mostly identical except for the type/number of ROMs
           used and the contents of the CPLD.  The 2nd revision also has a RAM chip on it.
           Each game has a multi-letter code assigned to it which is printed on a small sticker and placed on the top side
           of the MEM PCB.
           This code is then proceeded by a number (only '1' & '2' seen so far), then 'Ver.' then A/B/C/D/E (only 'A' seen so far)
           which denotes the software revision, and in some cases a sub-revision such as 1 or 2 (usually only listed in the
           test mode).
           The first 1 denotes a Japanese version. Other numbers denote a World version.
           For World versions, only the main program changes, the rest of the (graphics) ROMs use the Japanese version ROMs.
           Any System 10 MEM PCB can be swapped to run on any System 10 Main PCB regardless of the main board revision.
           The high scores are stored on the MEM PCB (probably inside the main program EEPROMs/FlashROMs or maybe the CPLD?).
           Also, on all System 10 games, there is a sticker with a serial number on it and the program ROMs also contain
           that same serial number. I'm not sure why, they're not exactly _easily_ tracable and no one cares either way ;-)
           See the Main PCB and ROM Daughterboard PCB texts below for more details.


Main PCB Layout
---------------

Revision 1
SYSTEM10 MAIN PCB 8906960103 (8906970103)

Revision 2
SYSTEM10 MAIN PCB 8906960104 (8906970104)

Revision 3
SYSTEM10 MAIN PCB 8906962400 (8906972400)
  |----------------------------------------------------------|
  |   LA4705    VR1                     J201                 |
  |                           |----------------------|       |
  |           NJM3414         |----------------------|       |
|-|                                                          |
|       BA3121                                               |
|             NJM3414            54V25632      54V25632    J1|
|                     CXD1178Q                               |
|J JP4                          |---------|   |-------|      |
|A                              |         |   |       |      |
|M         CXA2067AS            |CXD8561CQ|   |CY37128|      |
|M                 53.693175MHz |         |   |VP160  |      |
|A                              |         |   |       |      |
|                               |---------|   |-------|      |
|                                                            |
|                            101.4912MHz                     |
|-|          MAX734  IS41LV16100                             |
  |                              |---------|                 |
  |   DSW1           IS41LV16100 |         |                 |
  |                              |CXD8606BQ|                 |
  |        GAL16V8D              |         |                 |
  |J5      |-|           *       |         |          PST592 |
  |        | |                   |---------| |--------|      |
  |        | |                               |        |      |
  |        | |           *                   |CXD2938Q|      |
  |J4      | |J202             IS41LV16256   |        |      |
  |        | |                               |        |      |
  |        | |      EPM3064                  |--------|      |
  |        | |                                               |
  |        |-|                 PQ30RV21                      |
  |                                                  J103    |
  |----------------------------------------------------------|
Notes:
------
      CXD8606BQ   : SONY CXD8606BQ Central Processing Unit / GTE     (QFP208)
                     - replaced by CXD8606CQ on Revision 3 Main PCB
      CXD8561CQ   : SONY CXD8561CQ Graphics Processsor Unit          (QFP208)
      CXD2938Q    : SONY CXD2938Q  Sound Processor Unit              (QFP208)
      CXD1178Q    : SONY CXD1178Q  8-bit RGB 3-channel D/A converter (QFP48)
      CXA2067AS   : SONY CXA2067AS TV/Video circuit RGB Pre-Driver   (SDIP30)
      CY37128VP160: CYPRESS CY37128VP160 Complex Programmable Logic Device (TQFP160, stamped 'S10MA1')
                     - replaced by an updated revision on Revision 2 & 3 Main PCB and stamped 'S10MA1B'
      EPM3064     : Altera MAX EPM3064ATC100-10 Complex Programmable Logic Device (TQFP100, stamped 'S10MA2A')
      GAL16V8D    : GAL16V8D PAL (PLCC20, stamped 'S10MA3A')
      IS41LV16100 : ISSI IS41LV16100S-50T 1M x16 EDO DRAM (x2, TSOP50(44) Type II)
      IS41LV16256 : ISSI IS41LV16256-50T 256k x16 EDO DRAM (TSOP44(40) Type II)
      54V25632    : OKI 54V25632 256K x32 SGRAM (x2, QFP100)
      PQ30RV31    : Sharp PQ30RV31 5 Volt to 3.3 Volt Voltage Regulator
      LA4705      : LA4705 15W 2-channel Power Amplifier (SIP18)
      MAX734      : MAX734 +12V 120mA Flash Memory Programming Supply Switching Regulator (SOIC8)
      PST592      : PST592J System Reset IC with 2.7V detection circuit (SOIC4)
      BA3121      : Ground Isolation Amplifier/ Noise Eliminator (SOIC8)
      NJM3414     : 70mA Dual Op Amp (x2, SOIC8)
      DSW1        : 8 position dip switch
      JP4         : 2 position jumper, set to NC, alt. position labelled SYNC (Note: changing the jumper position has
                    no visual effect)
      J1          : 40 Pin IDC connector for plugging of a flat 40-wire cable (not used, purpose unknown, possible
                    CDROM/DVD)
      J4          : 10 pin header for extra controls etc  / (note: custom Namco 48 pin edge connector is not on
                    System10 PCBs)
      J5          : 4 pin header for stereo sound out     \
      J201        : 100 pin custom Namco connector for mounting of MEM PCB. This connector is surface-mounted, not a
                    thru-hole type.
      J202        : 80 pin custom Namco connector for mounting of another board. This connector is surface-mounted,
                    not a thru-hole type.
                    (not used, purpose unknown)
      J103        : 6-pin JAMMA2 power plug (Note none of the other JAMMA2 standard connectors are present)
      VR1         : Volume potentiometer
      *           : Unpopulated position for IS41LV16100 1M x16 EDO DRAM

Additional Notes:
                1. In test mode (Display Test) the screen can be set to interlace or non-interlace mode. The graphics in
                   interlace mode are visually much smoother with noticeable screen flickering. Non-interlace modes gives
                   a much blockier graphic display (i.e. lower resolution) but without screen flickering.
                2. There is no dedicated highscore/options EEPROM present on the PCB, the game stores its settings on the
                   game board (probably in the program EEPROMs/FlashROMs or maybe the CPLD?).

ROM Daughterboard PCB
---------------------
This PCB holds all the ROMs.
There are two known types of ROM daughterboards used on S10 games (so far).
All of the PCBs are the same size (approx 5" x 5") containing one custom connector surface-mounted to the underside of
the PCB, some MASKROMs/FlashROMs, a CPLD (which seems to be the customary 'KEYCUS' chip, and on the 2nd type a RAM
chip also.

********
*Type 1*
********
System10 MEM(M) PCB 8906961000 (8906970700)
|-------------------------------------|
|                                     |
|                       |-------|     +-
|                       |       |     +-
|                       |CY37128|     +-
|                       |VP160  |     +-
|                       |       |     +-
|     7E     7D         |-------|     +-
|                                     |
|     6E     6D                       |
|                                     |
|     5E     5D                 5A    |
|                                     |
|     4E     4D                 4A    |
|                                     |
|     3E     3D                       |
|                                     |
|     2E     2D                 2A    |
|                                     |
|     1E     1D                 1A    |
|                                     |
|-------------------------------------|
Notes:
      CY37128VP160: CY37128VP160 Cypress Complex Programmable Logic Device (TQFP160)
      1A - 5A     : Intel Flash DA28F640J5 64MBit Flash EEPROM (SSOP56)
      1D - 7E     : Samsung Electronics K3N9V1000A-YC 128MBit MASK ROM (TSOP48) (see note 3)
      6 pin header: (purpose unknown, probably for programming the CPLD)

This PCB is used on:

              Software     MEM PCB
Game          Revision     Sticker      KEYCUS   ROMs Populated
------------------------------------------------------------------------------------
Mr Driller 2  DR21/VER.A3  DR21 Ver.A   KC001A   DR21VERA.1A, DR21MA1.1D, DR21MA2.2D
Mr Driller 2  DR22/VER.A3  DR22 Ver.A   KC001A   DR22VERA.1A, DR21MA1.1D, DR21MA2.2D

      Note
      1. The ROM PCB has locations for 4x 64MBit program ROMs, but only 1A is populated.
      2. The ROM PCB has locations for 14x 128MBit GFX ROMs (Total capacity = 2048MBits) but only 1D and 2D are populated.
      3. These ROMs are only 18mm long, dumping them requires a special custom adapter

********
*Type 2*
********
System10 MEM(N) PCB 8906961402 (8906971402)
|-------------------------------------|
|                                     |
|                    |---------|      +-
|                    |         |      +-
|                    |CY37256  |      +-
|     8E     8D      |VP208    |      +-
|                    |         |      +-
|     7E     7D      |---------|      +-
|                                     |
|     6E     6D                       |
|                                     |
|     5E     5D       CY7C1019        |
|                                     |
|     4E     4D                       |
|                                     |
|     3E     3D                       |
|                                     |
|     2E     2D                       |
|                                     |
|     1E     1D                       |
|                                     |
|-------------------------------------|
Notes:
      CY37256VP208: Cypress CY37256VP208 Complex Programmable Logic Device (TQFP208)
      CY7C1019    : Cypress CY7C1019BV33-15VC or Samsung Electronics K6R1008V1C-JC15 128k x8 bit 3.3V High Speed CMOS Static Ram (SOJ32)
      1D - 8E     : Samsung Electronics K9F2808U0B-YCBO 128MBit NAND Flash EEPROM (TSOP48)
      6 pin header: (purpose unknown, probably for programming the CPLD)

This PCB is used on:

                                      MEM PCB
Game                                  Sticker       KEYCUS   ROMs Populated
---------------------------------------------------------------------------
Gekitoride-Jong Space                 10011 Ver.A   KC003A   8E, 8D, 7E, 7D
Mr.Driller G                          DRG1  Ver.A   KC007A   8E, 8D, 7E
Kotoba no Puzzle Mojipittan           KPM1  Ver.A   KC012A   8E, 8D, 7E
Panicuru Panekuru                     PPA1  Ver.A   KC017A   8E, 8D, 7E
Star Trigon                           STT1  Ver.A   KC019A   8E, 8D
Gamshara                              10021 Ver.A   KC020A   8E, 8D
Utyuu Daisakusen Chocovader Contactee CVC1  Ver.A   KC022A   8E, 8D, 7E, 7D, 6E
Kono Tako                             10021 Ver.A   KC034A   8E, 8D

      Note
      1. The ROM PCB has locations for 16x 128MBit FlashROMs (Total capacity = 2048MBits) but usually only a few are populated.
*/

#include "emu.h"
#include "cpu/psx/psx.h"
#include "video/psx.h"
#include "machine/ns10crypt.h"

class namcos10_state : public driver_device
{
public:
	namcos10_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }

	// memm variant interface
	DECLARE_WRITE16_MEMBER(key_w);
	DECLARE_READ16_MEMBER(range_r);
	DECLARE_WRITE16_MEMBER(bank_w);

	// memn variant interface
	DECLARE_WRITE16_MEMBER(crypto_switch_w);
	DECLARE_READ16_MEMBER(nand_status_r);
	DECLARE_WRITE8_MEMBER(nand_address1_w);
	DECLARE_WRITE8_MEMBER(nand_address2_w);
	DECLARE_WRITE8_MEMBER(nand_address3_w);
	DECLARE_WRITE8_MEMBER(nand_address4_w);
	DECLARE_READ16_MEMBER(nand_data_r);
	DECLARE_WRITE16_MEMBER(nand_block_w);
	DECLARE_READ16_MEMBER(nand_block_r);

	DECLARE_READ16_MEMBER (control_r);
	DECLARE_WRITE16_MEMBER(control_w);

	DECLARE_READ16_MEMBER (i2c_clock_r);
	DECLARE_WRITE16_MEMBER(i2c_clock_w);
	DECLARE_READ16_MEMBER (i2c_data_r);
	DECLARE_WRITE16_MEMBER(i2c_data_w);

	DECLARE_READ16_MEMBER (sprot_r);
	DECLARE_WRITE16_MEMBER(sprot_w);

	UINT8 *nand_base;
	void nand_copy( UINT32 *dst, UINT32 address, int len );

private:
	enum {
		I2CP_IDLE,
		I2CP_RECIEVE_BYTE,
		I2CP_RECIEVE_ACK_1,
		I2CP_RECIEVE_ACK_0
	};
	UINT16 key;
	UINT8  cnt;
	UINT32 bank_base;
	UINT32 nand_address;
	UINT16 block[0x1ff];
	ns10_decrypter_device* decrypter;

	UINT16 i2c_host_clock, i2c_host_data, i2c_dev_clock, i2c_dev_data, i2c_prev_clock, i2c_prev_data;
	int i2cp_mode;
	UINT8 i2c_byte;
	int i2c_bit;

	int sprot_bit, sprot_byte;
	UINT16 nand_read( UINT32 address );
	UINT16 nand_read2( UINT32 address );

	void i2c_update();
public:
	DECLARE_DRIVER_INIT(knpuzzle);
	DECLARE_DRIVER_INIT(panikuru);
	DECLARE_DRIVER_INIT(mrdrilr2);
	DECLARE_DRIVER_INIT(startrgn);
	DECLARE_DRIVER_INIT(gunbalna);
	DECLARE_DRIVER_INIT(nflclsfb);
	DECLARE_DRIVER_INIT(gjspace);
	DECLARE_DRIVER_INIT(gamshara);
	DECLARE_DRIVER_INIT(mrdrilrg);
	DECLARE_DRIVER_INIT(chocovdr);
	DECLARE_DRIVER_INIT(konotako);
	DECLARE_MACHINE_RESET(namcos10);
	void memn_driver_init(  );
	required_device<cpu_device> m_maincpu;
};


static ADDRESS_MAP_START( namcos10_map, AS_PROGRAM, 32, namcos10_state )
	AM_RANGE(0x1f500000, 0x1f501fff) AM_RAM AM_SHARE("share3") /* ram? stores block numbers */
	AM_RANGE(0x9f500000, 0x9f501fff) AM_RAM AM_SHARE("share3") /* ram? stores block numbers */
	AM_RANGE(0xbf500000, 0xbf501fff) AM_RAM AM_SHARE("share3") /* ram? stores block numbers */

	AM_RANGE(0x1fba0000, 0x1fba0003) AM_READWRITE16(sprot_r, sprot_w, 0xffff0000)
	AM_RANGE(0x1fba0008, 0x1fba000b) AM_READWRITE16(i2c_clock_r, i2c_clock_w, 0x0000ffff)
	AM_RANGE(0x1fba0008, 0x1fba000b) AM_READWRITE16(i2c_data_r,  i2c_data_w,  0xffff0000)
	AM_RANGE(0x1fba0000, 0x1fba000f) AM_READWRITE16(control_r, control_w, 0xffffffff)
ADDRESS_MAP_END


// memm variant interface
//
// banked view with dynamic decryption over the flash.  Access to the
// nand is unknown, but may just be using the memn interface.  Won't
// know until the decryption is done.
//
// bios copies 62000-37ffff from the flash to 80012000 in ram through the
// decryption in range_r then jumps there (and dies horribly, of course)

WRITE16_MEMBER(namcos10_state::key_w )
{
	key = (data >> 15) | (data << 1);
	logerror("key_w %04x\n", key);
	cnt = 0;
}

WRITE16_MEMBER(namcos10_state::bank_w)
{
	bank_base = 0x80000 * offset;
}

READ16_MEMBER(namcos10_state::range_r)
{
	UINT32 d16 = ((const UINT16 *)(memregion("maincpu:rom")->base()))[offset+bank_base];

	/* This is not entirely correct, but not entirely incorrect either...
	   It's also specific to mrdriller2, it seems.
	*/

	UINT16 dd16 = d16 ^ key;

	key = d16;

	key =
		//((    BIT(d16,  3) ^ (BIT(cnt, 0) & !BIT(cnt, 2))) << 15) |
		((1 ^ BIT(key,  3) ^  BIT(d16, 0))                 << 15) |
		((1 ^ BIT(key, 13) ^  BIT(cnt, 0))                 << 14) |
		((1 ^ BIT(key, 11) ^  BIT(d16, 5) ^  BIT(d16, 2))  << 13) |
		((    BIT(key,  9) ^  BIT(cnt, 3))                 << 12) |
		((1 ^ BIT(key,  2))                                << 11) |
		((    BIT(key, 10) ^ (BIT(d16, 4) &  BIT(cnt, 1))) << 10) |
		((1 ^ BIT(key,  6) ^  BIT(cnt, 4))                 <<  9) |
		((1 ^ BIT(d16,  6) ^  BIT(key, 5))                 <<  8) |
		((    BIT(key,  1) ^ (BIT(d16, 5) |  BIT(d16, 4))) <<  7) |
		((    BIT(key, 15))                                <<  6) |
		((1 ^ BIT(key,  4) ^  BIT(cnt, 3) ^  BIT(d16, 2))  <<  5) |
		((1 ^ BIT(key,  7) ^  BIT(cnt, 5))                 <<  4) |
		((1 ^ BIT(key,  8) ^ (BIT(cnt, 7) |  BIT(d16, 3))) <<  3) |
		((    BIT(key, 14) ^ (BIT(cnt, 1) |  BIT(d16, 7))) <<  2) |
		((1 ^ BIT(key, 12) ^ (BIT(cnt, 7) &  BIT(d16, 7))) <<  1) |
		//((                   (BIT(cnt, 0) |  BIT(cnt, 2))) <<  0);
		((1 ^ BIT(key,  0) ^ BIT(cnt, 2))                  <<  0);

	cnt++;

	return dd16;
}

READ16_MEMBER(namcos10_state::control_r)
{
	logerror("control_r %d (%x)\n", offset, space.device().safe_pc());
	if(offset == 2)
		return 1^0xffff;
	return 0;
}

WRITE16_MEMBER(namcos10_state::control_w)
{
	logerror("control_w %d, %04x (%x)\n", offset, data, space.device().safe_pc());
}

WRITE16_MEMBER(namcos10_state::sprot_w)
{
	logerror("sprot_w %04x (%x)\n", data, space.device().safe_pc());
	sprot_bit = 7;
	sprot_byte = 0;
}

//   startrgn:
// 8004b6f8: jal 4b730, dies if v0!=0 (answers 1)
// flash access, unhappy with the results

// 800128d8: jal 37b58 (flash death)
// 800128e0: jal 1649c
// 800128e8: jal 2c47c

READ16_MEMBER(namcos10_state::sprot_r)
{
	// If line 3 has 0x30/0x31 in it, something happens.  That
	// something currently kills the system though.

	const static UINT8 prot[0x40] = {
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
		0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
		0x50, 0x51, 0x50, 0x51, 0x50, 0x51, 0x50, 0x51, 0x50, 0x51, 0x50, 0x51, 0x50, 0x51, 0x50, 0x51,
		0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
	};
	UINT16 res = sprot_byte >= 0x20 ? 0x3 :
		(((prot[sprot_byte     ] >> sprot_bit) & 1) ? 1 : 0) |
		(((prot[sprot_byte+0x20] >> sprot_bit) & 1) ? 2 : 0);

	sprot_bit--;
	if(sprot_bit == -1) {
		sprot_bit = 7;
		sprot_byte++;
	}
	return res;
}

READ16_MEMBER(namcos10_state::i2c_clock_r)
{
	UINT16 res = i2c_dev_clock & i2c_host_clock & 1;
	//  logerror("i2c_clock_r %d (%x)\n", res, space.device().safe_pc());
	return res;
}


WRITE16_MEMBER(namcos10_state::i2c_clock_w)
{
	COMBINE_DATA(&i2c_host_clock);
	//  logerror("i2c_clock_w %d (%x)\n", data, space.device().safe_pc());
	i2c_update();
}

READ16_MEMBER(namcos10_state::i2c_data_r)
{
	UINT16 res = i2c_dev_data & i2c_host_data & 1;
	//  logerror("i2c_data_r %d (%x)\n", res, space.device().safe_pc());
	return res;
}


WRITE16_MEMBER(namcos10_state::i2c_data_w)
{
	COMBINE_DATA(&i2c_host_data);
	//  logerror("i2c_data_w %d (%x)\n", data, space.device().safe_pc());
	i2c_update();
}

void namcos10_state::i2c_update()
{
	UINT16 clock = i2c_dev_clock & i2c_host_clock & 1;
	UINT16 data = i2c_dev_data & i2c_host_data & 1;

	if(i2c_prev_data == data && i2c_prev_clock == clock)
		return;

	switch(i2cp_mode) {
	case I2CP_IDLE:
		if(clock && !data) {
			logerror("i2c: start bit\n");
			i2c_byte = 0;
			i2c_bit = 7;
			i2cp_mode = I2CP_RECIEVE_BYTE;
		}
		break;
	case I2CP_RECIEVE_BYTE:
		if(clock && data && !i2c_prev_data) {
			logerror("i2c stop bit\n");
			i2cp_mode = I2CP_IDLE;
		} else if(clock && !i2c_prev_clock) {
			i2c_byte |= (data << i2c_bit);
			//          logerror("i2c_byte = %02x (%d)\n", i2c_byte, i2c_bit);
			i2c_bit--;
			if(i2c_bit < 0) {
				i2cp_mode = I2CP_RECIEVE_ACK_1;
				logerror("i2c recieved byte %02x\n", i2c_byte);
				i2c_dev_data = 0;
				data = 0;
			}
		}
		break;
	case I2CP_RECIEVE_ACK_1:
		if(clock && !i2c_prev_clock) {
			//          logerror("i2c ack on\n");
			i2cp_mode = I2CP_RECIEVE_ACK_0;
		}
		break;
	case I2CP_RECIEVE_ACK_0:
		if(!clock && i2c_prev_clock) {
			//          logerror("i2c ack off\n");
			i2c_dev_data = 1;
			data = i2c_host_data & 1;
			i2c_byte = 0;
			i2c_bit = 7;
			i2cp_mode = I2CP_RECIEVE_BYTE;
		}
		break;
	}
	i2c_prev_data = data;
	i2c_prev_clock = clock;
}

static ADDRESS_MAP_START( namcos10_memm_map, AS_PROGRAM, 32, namcos10_state )
	AM_RANGE(0x1f300000, 0x1f300003) AM_WRITE16(key_w, 0x0000ffff)
	AM_RANGE(0x1f400000, 0x1f5fffff) AM_READ16(range_r, 0xffffffff)
	AM_RANGE(0x1fb40000, 0x1fb4000f) AM_WRITE16(bank_w, 0xffffffff)

	AM_IMPORT_FROM(namcos10_map)
ADDRESS_MAP_END


// memn variant interface
//
// Block access to the nand.  Something strange is going on with the
// status port.  Interaction with the decryption is unclear at best.

WRITE16_MEMBER(namcos10_state::crypto_switch_w)
{
	printf("crypto_switch_w: %04x\n", data);
	if (decrypter == nullptr)
		return;

	if (BIT(data, 15) != 0)
		decrypter->activate(data & 0xf);
	else
		decrypter->deactivate();
}

READ16_MEMBER(namcos10_state::nand_status_r )
{
	return 0;
}

WRITE8_MEMBER(namcos10_state::nand_address1_w )
{
	logerror("nand_a1_w %08x (%08x)\n", data, space.device().safe_pc());
	//  nand_address = ( nand_address & 0x00ffffff ) | ( data << 24 );
}

WRITE8_MEMBER( namcos10_state::nand_address2_w )
{
	logerror("nand_a2_w %08x (%08x)\n", data, space.device().safe_pc());
	nand_address = ( nand_address & 0xffffff00 ) | ( data << 0 );
}

WRITE8_MEMBER( namcos10_state::nand_address3_w )
{
	logerror("nand_a3_w %08x (%08x)\n", data, space.device().safe_pc());
	nand_address = ( nand_address & 0xffff00ff ) | ( data <<  8 );
}

WRITE8_MEMBER( namcos10_state::nand_address4_w )
{
	nand_address = ( nand_address & 0xff00ffff ) | ( data << 16 );
	logerror("nand_a4_w %08x (%08x) -> %08x\n", data, space.device().safe_pc(), nand_address*2);
}

UINT16 namcos10_state::nand_read( UINT32 address )
{
	int index = ( ( address / 512 ) * 528 ) + ( address % 512 );
	return nand_base[ index ] | ( nand_base[ index + 1 ] << 8 );
}

UINT16 namcos10_state::nand_read2( UINT32 address )
{
	int index = ( ( address / 512 ) * 528 ) + ( address % 512 );
	return nand_base[ index + 1 ] | ( nand_base[ index ] << 8 );
}

READ16_MEMBER( namcos10_state::nand_data_r )
{
	UINT16 data = nand_read2( nand_address * 2 );

	//  logerror("read %08x = %04x\n", nand_address*2, data);
	// printf("read %08x = %04x\n", nand_address*2, data);


/*  printf( "data<-%08x (%08x)\n", data, nand_address ); */
	nand_address++;

	if (decrypter == nullptr)
		return data;

	if (decrypter->is_active())
		return decrypter->decrypt(data);
	else
		return data;
}

void namcos10_state::nand_copy( UINT32 *dst, UINT32 address, int len )
{
	while( len > 0 )
	{
		*( dst++ ) = nand_read( address ) | ( nand_read( address + 2 ) << 16 );
		address += 4;
		len -= 4;
	}
}

WRITE16_MEMBER(namcos10_state::nand_block_w)
{
	COMBINE_DATA( &block[offset] );
}

READ16_MEMBER(namcos10_state::nand_block_r)
{
	return block[ offset ];
}

static ADDRESS_MAP_START( namcos10_memn_map, AS_PROGRAM, 32, namcos10_state )
	AM_RANGE(0x1f300000, 0x1f300003) AM_WRITE16(crypto_switch_w, 0x0000ffff)
	AM_RANGE(0x1f380000, 0x1f380003) AM_WRITE16(crypto_switch_w, 0x0000ffff)
	AM_RANGE(0x1f400000, 0x1f400003) AM_READ16(nand_status_r, 0x0000ffff)
	AM_RANGE(0x1f410000, 0x1f410003) AM_WRITE8(nand_address1_w, 0x000000ff)
	AM_RANGE(0x1f420000, 0x1f420003) AM_WRITE8(nand_address2_w, 0x000000ff)
	AM_RANGE(0x1f430000, 0x1f430003) AM_WRITE8(nand_address3_w, 0x000000ff)
	AM_RANGE(0x1f440000, 0x1f440003) AM_WRITE8(nand_address4_w, 0x000000ff)
	AM_RANGE(0x1f450000, 0x1f450003) AM_READ16(nand_data_r, 0x0000ffff)
	AM_RANGE(0x1fb60000, 0x1fb60003) AM_READWRITE16(nand_block_r, nand_block_w, 0x0000ffff)

	AM_IMPORT_FROM(namcos10_map)
ADDRESS_MAP_END

void namcos10_state::memn_driver_init(  )
{
	UINT8 *BIOS = (UINT8 *)memregion( "maincpu:rom" )->base();
	nand_base = (UINT8 *)memregion( "user2" )->base();
	decrypter = static_cast<ns10_decrypter_device*>(machine().root_device().subdevice("decrypter"));

	nand_copy( (UINT32 *)( BIOS + 0x0000000 ), 0x08000, 0x001c000 );
	nand_copy( (UINT32 *)( BIOS + 0x0020000 ), 0x24000, 0x03e0000 );
}

static void decrypt_bios( running_machine &machine, const char *regionName, int start, int end, int b15, int b14, int b13, int b12, int b11, int b10, int b9, int b8,
	int b7, int b6, int b5, int b4, int b3, int b2, int b1, int b0 )
{
	memory_region *region = machine.root_device().memregion( regionName );
	UINT16 *BIOS = (UINT16 *)( region->base() + start );
	int len = (end - start) / 2;

	for( int i = 0; i < len; i++ )
	{
		BIOS[ i ] = BITSWAP16( BIOS[ i ] ^ 0xaaaa,
			b15, b14, b13, b12, b11, b10, b9, b8, b7, b6, b5, b4, b3, b2, b1, b0 );
	}
}

DRIVER_INIT_MEMBER(namcos10_state,mrdrilr2)
{
	int regSize = machine().root_device().memregion("user2")->bytes();
	decrypt_bios(machine(), "maincpu:rom", 0, regSize, 0xc, 0xd, 0xf, 0xe, 0xb, 0xa, 0x9, 0x8, 0x7, 0x6, 0x4, 0x1, 0x2, 0x5, 0x0, 0x3);
}

DRIVER_INIT_MEMBER(namcos10_state,gjspace)
{
	int regSize = machine().root_device().memregion("user2")->bytes();
	decrypt_bios(machine(), "user2", 0x0008400, 0x0029400, 0x0, 0x2, 0xe, 0xd, 0xf, 0x6, 0xc, 0x7, 0x5, 0x1, 0x9, 0x8, 0xa, 0x3, 0x4, 0xb);
	decrypt_bios(machine(), "user2", 0x0210000, 0x104e800, 0x0, 0x2, 0xe, 0xd, 0xf, 0x6, 0xc, 0x7, 0x5, 0x1, 0x9, 0x8, 0xa, 0x3, 0x4, 0xb);
	decrypt_bios(machine(), "user2", 0x1077c00, regSize, 0x0, 0x2, 0xe, 0xd, 0xf, 0x6, 0xc, 0x7, 0x5, 0x1, 0x9, 0x8, 0xa, 0x3, 0x4, 0xb);
	memn_driver_init();
}

DRIVER_INIT_MEMBER(namcos10_state,mrdrilrg)
{
	int regSize = machine().root_device().memregion("user2")->bytes();
	decrypt_bios(machine(), "user2", 0x8400, regSize, 0x6, 0x4, 0x7, 0x5, 0x2, 0x1, 0x0, 0x3, 0xc, 0xd, 0xe, 0xf, 0x8, 0x9, 0xb, 0xa);
	memn_driver_init();
}

DRIVER_INIT_MEMBER(namcos10_state,knpuzzle)
{
	int regSize = machine().root_device().memregion("user2")->bytes();
	decrypt_bios(machine(), "user2", 0x0008400, 0x0029400, 0x6, 0x7, 0x4, 0x5, 0x2, 0x0, 0x3, 0x1, 0xc, 0xd, 0xe, 0xf, 0x9, 0xb, 0x8, 0xa);
	decrypt_bios(machine(), "user2", 0x047ac00, 0x1042200, 0x6, 0x7, 0x4, 0x5, 0x2, 0x0, 0x3, 0x1, 0xc, 0xd, 0xe, 0xf, 0x9, 0xb, 0x8, 0xa);
	decrypt_bios(machine(), "user2", 0x104a600, regSize  , 0x6, 0x7, 0x4, 0x5, 0x2, 0x0, 0x3, 0x1, 0xc, 0xd, 0xe, 0xf, 0x9, 0xb, 0x8, 0xa);
	memn_driver_init();
}

DRIVER_INIT_MEMBER(namcos10_state,startrgn)
{
	int regSize = machine().root_device().memregion("user2")->bytes();
	decrypt_bios(machine(), "user2", 0x0008400, 0x0029400, 0x6, 0x5, 0x4, 0x7, 0x1, 0x3, 0x0, 0x2, 0xc, 0xd, 0xe, 0xf, 0x8, 0xb, 0xa, 0x9);
	decrypt_bios(machine(), "user2", 0x00b9a00, 0x105ae00, 0x6, 0x5, 0x4, 0x7, 0x1, 0x3, 0x0, 0x2, 0xc, 0xd, 0xe, 0xf, 0x8, 0xb, 0xa, 0x9);
	decrypt_bios(machine(), "user2", 0x1080000, regSize  , 0x6, 0x7, 0x4, 0x5, 0x0, 0x1, 0x3, 0x2, 0xd, 0xc, 0xf, 0xe, 0x8, 0x9, 0xb, 0xa);
	memn_driver_init();
}

DRIVER_INIT_MEMBER(namcos10_state,gamshara)
{
	int regSize = machine().root_device().memregion("user2")->bytes();
	decrypt_bios(machine(), "user2", 0x0008400, 0x0029400, 0x5, 0x4, 0x7, 0x6, 0x0, 0x1, 0x3, 0x2, 0xd, 0xf, 0xc, 0xe, 0x8, 0x9, 0xa, 0xb);
	decrypt_bios(machine(), "user2", 0x014e200, 0x105ae00, 0x5, 0x4, 0x7, 0x6, 0x0, 0x1, 0x3, 0x2, 0xd, 0xf, 0xc, 0xe, 0x8, 0x9, 0xa, 0xb);
	decrypt_bios(machine(), "user2", 0x1080000, regSize  , 0x5, 0x4, 0x7, 0x6, 0x0, 0x1, 0x3, 0x2, 0xd, 0xf, 0xc, 0xe, 0x8, 0x9, 0xa, 0xb);
	memn_driver_init();
}

DRIVER_INIT_MEMBER(namcos10_state,gunbalna)
{
	int regSize = machine().root_device().memregion("user2")->bytes();
	decrypt_bios(machine(), "user2", 0x8400, regSize, 0x5, 0x4, 0x7, 0x6, 0x0, 0x1, 0x3, 0x2, 0xd, 0xf, 0xc, 0xe, 0x9, 0x8, 0xa, 0xb);
	memn_driver_init();
}

DRIVER_INIT_MEMBER(namcos10_state,chocovdr)
{
	int regSize = machine().root_device().memregion("user2")->bytes();
	decrypt_bios(machine(), "user2", 0x0008400, 0x0029400, 0x5, 0x4, 0x6, 0x7, 0x1, 0x0, 0x2, 0x3, 0xc, 0xf, 0xe, 0xd, 0x8, 0xb, 0xa, 0x9);
	decrypt_bios(machine(), "user2", 0x01eae00, 0x105ae00, 0x5, 0x4, 0x6, 0x7, 0x1, 0x0, 0x2, 0x3, 0xc, 0xf, 0xe, 0xd, 0x8, 0xb, 0xa, 0x9);
	decrypt_bios(machine(), "user2", 0x1080000, regSize  , 0x5, 0x4, 0x6, 0x7, 0x1, 0x0, 0x2, 0x3, 0xc, 0xf, 0xe, 0xd, 0x8, 0xb, 0xa, 0x9);
	memn_driver_init();
}

DRIVER_INIT_MEMBER(namcos10_state,panikuru)
{
	int regSize = machine().root_device().memregion("user2")->bytes();
	decrypt_bios(machine(), "user2", 0x8400, regSize, 0x6, 0x4, 0x7, 0x5, 0x0, 0x1, 0x2, 0x3, 0xc, 0xf, 0xe, 0xd, 0x9, 0x8, 0xb, 0xa);
	memn_driver_init();
}

DRIVER_INIT_MEMBER(namcos10_state,nflclsfb)
{
	int regSize = machine().root_device().memregion("user2")->bytes();
	decrypt_bios(machine(), "user2", 0x0008400, 0x0029400, 0x6, 0x5, 0x4, 0x7, 0x1, 0x3, 0x0, 0x2, 0xc, 0xd, 0xe, 0xf, 0x8, 0xb, 0xa, 0x9);
	decrypt_bios(machine(), "user2", 0x0214200, 0x105ae00, 0x6, 0x5, 0x4, 0x7, 0x1, 0x3, 0x0, 0x2, 0xc, 0xd, 0xe, 0xf, 0x8, 0xb, 0xa, 0x9);
	decrypt_bios(machine(), "user2", 0x1080000, regSize  , 0x6, 0x5, 0x4, 0x7, 0x1, 0x3, 0x0, 0x2, 0xc, 0xd, 0xe, 0xf, 0x8, 0xb, 0xa, 0x9);
	memn_driver_init();
}

DRIVER_INIT_MEMBER(namcos10_state,konotako)
{
	int regSize = machine().root_device().memregion("user2")->bytes();
	decrypt_bios(machine(), "user2", 0x0008400, 0x0029400, 0x6, 0x7, 0x4, 0x5, 0x0, 0x1, 0x3, 0x2, 0xd, 0xc, 0xf, 0xe, 0x8, 0x9, 0xb, 0xa);
	decrypt_bios(machine(), "user2", 0x00b9a00, 0x105ae00, 0x6, 0x7, 0x4, 0x5, 0x0, 0x1, 0x3, 0x2, 0xd, 0xc, 0xf, 0xe, 0x8, 0x9, 0xb, 0xa);
	decrypt_bios(machine(), "user2", 0x1080000, regSize  , 0x6, 0x7, 0x4, 0x5, 0x0, 0x1, 0x3, 0x2, 0xd, 0xc, 0xf, 0xe, 0x8, 0x9, 0xb, 0xa);
	memn_driver_init();
}


MACHINE_RESET_MEMBER(namcos10_state,namcos10)
{
	i2c_dev_clock = i2c_dev_data = 1;
	i2c_host_clock = i2c_host_data = 1;
	i2c_prev_clock = i2c_prev_data = 1;
	i2cp_mode = I2CP_IDLE;
	i2c_byte = 0x00;
	i2c_bit = 0;
}

static MACHINE_CONFIG_START( namcos10_memm, namcos10_state )
	/* basic machine hardware */
	MCFG_CPU_ADD( "maincpu", CXD8606BQ, XTAL_101_4912MHz )
	MCFG_CPU_PROGRAM_MAP( namcos10_memm_map )

	// The bios first configures the rom window as 80000-big, then
	// switches to 400000.  If berr is active, the first configuration
	// wipes all handlers after 1fc80000, which kills the system
	// afterwards

	MCFG_PSX_DISABLE_ROM_BERR

	MCFG_RAM_MODIFY("maincpu:ram")
	MCFG_RAM_DEFAULT_SIZE("16M")

	MCFG_MACHINE_RESET_OVERRIDE(namcos10_state, namcos10 )

	/* video hardware */
	MCFG_PSXGPU_ADD( "maincpu", "gpu", CXD8561CQ, 0x200000, XTAL_53_693175MHz )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( namcos10_memn, namcos10_state )
	/* basic machine hardware */
	MCFG_CPU_ADD( "maincpu", CXD8606BQ, XTAL_101_4912MHz )
	MCFG_CPU_PROGRAM_MAP( namcos10_memn_map )

	// The bios first configures the rom window as 80000-big, then
	// switches to 400000.  If berr is active, the first configuration
	// wipes all handlers after 1fc80000, which kills the system
	// afterwards

	MCFG_PSX_DISABLE_ROM_BERR

	MCFG_RAM_MODIFY("maincpu:ram")
	MCFG_RAM_DEFAULT_SIZE("16M")

	MCFG_MACHINE_RESET_OVERRIDE(namcos10_state, namcos10 )

	/* video hardware */
	MCFG_PSXGPU_ADD( "maincpu", "gpu", CXD8561CQ, 0x200000, XTAL_53_693175MHz )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED(ns10_chocovdr, namcos10_memn)
/* decrypter device (CPLD in hardware?) */
MCFG_DEVICE_ADD("decrypter", CHOCOVDR_DECRYPTER, 0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED(ns10_gamshara, namcos10_memn)
/* decrypter device (CPLD in hardware?) */
MCFG_DEVICE_ADD("decrypter", GAMSHARA_DECRYPTER, 0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED(ns10_gjspace, namcos10_memn)
/* decrypter device (CPLD in hardware?) */
MCFG_DEVICE_ADD("decrypter", GJSPACE_DECRYPTER, 0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED(ns10_knpuzzle, namcos10_memn)
/* decrypter device (CPLD in hardware?) */
MCFG_DEVICE_ADD("decrypter", KNPUZZLE_DECRYPTER, 0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED(ns10_konotako, namcos10_memn)
/* decrypter device (CPLD in hardware?) */
MCFG_DEVICE_ADD("decrypter", KONOTAKO_DECRYPTER, 0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED(ns10_nflclsfb, namcos10_memn)
/* decrypter device (CPLD in hardware?) */
MCFG_DEVICE_ADD("decrypter", NFLCLSFB_DECRYPTER, 0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED(ns10_startrgn, namcos10_memn)
/* decrypter device (CPLD in hardware?) */
MCFG_DEVICE_ADD("decrypter", STARTRGN_DECRYPTER, 0)
MACHINE_CONFIG_END

static INPUT_PORTS_START( namcos10 )
	/* IN 0 */
	PORT_START("SYSTEM")
	PORT_SERVICE_NO_TOGGLE( 0x8000, IP_ACTIVE_HIGH )
	PORT_BIT( 0x7fff, IP_ACTIVE_HIGH, IPT_UNUSED )

	/* IN 1 */
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON5 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON6 )
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	/* IN 2 */
	PORT_START("P2")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_SERVICE2 )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	/* IN 3 */
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
INPUT_PORTS_END


ROM_START( mrdrilr2 )
	ROM_REGION32_LE( 0x800000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD( "dr21vera.1a",  0x000000, 0x800000, CRC(f93532a2) SHA1(8b72f2868978be1f0e0abd11425a3c8b2b0c4e99) )

	ROM_REGION( 0x4000000, "user2", 0 ) /* main prg */
	ROM_LOAD( "dr21ma1.1d", 0x0000000, 0x1000000, CRC(26dc6f55) SHA1(a9cedf547fa7a4d5850b9b3b867d46e577a035e0) )
	ROM_LOAD( "dr21ma2.2d", 0x1000000, 0x1000000, CRC(702556ff) SHA1(c95defd5fd6a9b406fc8d8f28ecfab732ef1ff42) )
ROM_END

ROM_START( mrdrlr2a )
	ROM_REGION32_LE( 0x800000, "maincpu:rom", 0 ) /* main prg */
	ROM_LOAD( "dr22vera.1a",  0x000000, 0x800000, CRC(f2633388) SHA1(42e56c9758ee833390003d4b41956f75f5a22760) )

	ROM_REGION( 0x4000000, "user2", 0 ) /* main prg */
	ROM_LOAD( "dr21ma1.1d", 0x0000000, 0x1000000, CRC(26dc6f55) SHA1(a9cedf547fa7a4d5850b9b3b867d46e577a035e0) )
	ROM_LOAD( "dr21ma2.2d", 0x1000000, 0x1000000, CRC(702556ff) SHA1(c95defd5fd6a9b406fc8d8f28ecfab732ef1ff42) )
ROM_END

ROM_START( gjspace )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x4200000, "user2", 0 ) /* main prg */
	ROM_LOAD( "10011a_0.bin", 0x0000000, 0x1080000, CRC(df862033) SHA1(4141357ed315adb4de636d7bf752354e953e8cbf) )
	ROM_LOAD( "10011a_1.bin", 0x1080000, 0x1080000, CRC(734c7ac0) SHA1(2f325236a4e4f2dba886682e9a7e8e243b5fbb3d) )
	ROM_LOAD( "10011a_2.bin", 0x2100000, 0x1080000, CRC(3bbbc0b7) SHA1(ad02ec2e5f401f0f5d40a413038649ebd25d5343) )
	ROM_LOAD( "10011a_3.bin", 0x3180000, 0x1080000, CRC(fb0de5ca) SHA1(50a462a52ff4a0bc112b9d89f2b2d032c60cf59c) )
ROM_END

ROM_START( mrdrilrg )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x3180000, "user2", 0 ) /* main prg */
	ROM_LOAD( "drg1a_0.bin",  0x0000000, 0x1080000, CRC(e0801878) SHA1(fbb771c1e76e0690f6dffed2287eb470b561ec20) )
	ROM_LOAD( "drg1a_1.bin",  0x1080000, 0x1080000, CRC(4d8cde73) SHA1(62a5fab8be8fd0a6bfeb101020d4cf58866a757c) )
	ROM_LOAD( "drg1a_2.bin",  0x2100000, 0x1080000, CRC(ccfabf7b) SHA1(0cbd91ce8abd6efca5d427b52279ce265f685aa9) )
ROM_END

ROM_START( mrdrilrga )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x3180000, "user2", 0 ) /* main prg */
	ROM_LOAD( "0.8e",         0x0000000, 0x1080000, CRC(def72bcd) SHA1(e243b7ef23b2b00612c185e01493cd01be51f154) )
	ROM_LOAD( "1.8d",         0x1080000, 0x1080000, CRC(c87b5e86) SHA1(b034210da30e1f2f7d04f77e00bf7724437e2024) )
	ROM_LOAD( "2.7e",         0x2100000, 0x1080000, CRC(e0a9339f) SHA1(4284e7233876cfaf8021440d78ccc8c70d00cc00) )
ROM_END

ROM_START( knpuzzle )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x3180000, "user2", 0 ) /* main prg */
	ROM_LOAD( "kpm1a_0.bin",  0x0000000, 0x1080000, CRC(b2947eb8) SHA1(fa941bf3598bb25d2c8f0a93154e32bf78a6507c) )
	ROM_LOAD( "kpm1a_1.bin",  0x1080000, 0x1080000, CRC(f3aa855a) SHA1(87b94e22db4bc4169324bbff93c4ea19c1d99b40) )
	ROM_LOAD( "kpm1a_2.bin",  0x2100000, 0x1080000, CRC(b297cc8d) SHA1(c3494e7a8a0b4e0c8c40b99121373effbfe848eb) )
ROM_END

ROM_START( startrgn )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x2100000, "user2", 0 ) /* main prg */
	ROM_LOAD( "stt1a_0.bin",  0x0000000, 0x1080000, CRC(1e090644) SHA1(a7a293e2bd9eea2eb64a492a47272d9d9ee2c724) )
	ROM_LOAD( "stt1a_1.bin",  0x1080000, 0x1080000, CRC(aa527694) SHA1(a25dcbeca58a1443070848b3487a24d51d41a34b) )
ROM_END

ROM_START( gamshara )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x2100000, "user2", 0 ) /* main prg */
	ROM_LOAD( "10021a.8e",    0x0000000, 0x1080000, CRC(6c0361fc) SHA1(7debf1f2e6bed31d59fb224a78a17a94fc573785) )
	ROM_LOAD( "10021a.8d",    0x1080000, 0x1080000, CRC(73669ff7) SHA1(eb8bbf931f1f8a049208d081d040512a3ffa9c00) )
ROM_END

ROM_START( ptblank3 )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x2100000, "user2", 0 ) /* main prg */
	// protection issues preventing the last NAND block to be dumped
	ROM_LOAD("gnn2a.8e", 0x0000000, 0x1080000, BAD_DUMP CRC(31b39221) SHA1(7fcb14aaa26c531928a6cd704e746d0e3ae3e031))
	ROM_LOAD( "gnn2a.8d",         0x1080000, 0x1080000, BAD_DUMP CRC(82d2cfb5) SHA1(4b5e713a55e74a7b32b1b9b5811892df2df86256) )
ROM_END

ROM_START( gunbalina )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x2100000, "user2", 0 ) /* main prg */
	// protection issues preventing the last NAND block to be dumped
	ROM_LOAD( "gnn1a.8e",         0x0000000, 0x1080000, BAD_DUMP CRC(981b03d4) SHA1(1c55458f1b2964afe2cf4e9d84548c0699808e9f) )
	ROM_LOAD( "gnn1a.8d",         0x1080000, 0x1080000, BAD_DUMP CRC(6cd343e0) SHA1(dcec44abae1504025895f42fe574549e5010f7d5) )
ROM_END

ROM_START( chocovdr )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x5280000, "user2", 0 ) /* main prg */
	ROM_LOAD( "0.8e",         0x0000000, 0x1080000, CRC(f36eebb5) SHA1(a0464186b247b28f37005ffd9e9b7370145f67ef) )
	ROM_LOAD( "1.8d",         0x1080000, 0x1080000, CRC(4aecd6fc) SHA1(31fe8f36e38020a92f15c44fd1a4b486636b40ce) )
	ROM_LOAD( "2.7e",         0x2100000, 0x1080000, CRC(ac212e5a) SHA1(f2d2e65a3249992730b8b90561b9bcf5eaaafb88) )
	ROM_LOAD( "3.7d",         0x3180000, 0x1080000, CRC(907d3d15) SHA1(20519d1f8bd9c6bc45b65e2d7444d588e922611d) )
	ROM_LOAD( "4.6e",         0x4200000, 0x1080000, CRC(1ed957dd) SHA1(bc8ce9f249fe496c130c6fe67b2260c4d0734ab9) )
ROM_END

ROM_START( panikuru )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x3180000, "user2", 0 ) /* main prg */
	ROM_LOAD( "0.8e",         0x0000000, 0x1080000, CRC(3aa66da4) SHA1(3f6ff164981e2c1825766c775442608fbf86d702) )
	ROM_LOAD( "1.8d",         0x1080000, 0x1080000, CRC(18e5135d) SHA1(a7b1533a1df71be5498718e301d1c9c548551fb4) )
	ROM_LOAD( "2.7e",         0x2100000, 0x1080000, CRC(cd3b25e0) SHA1(39dfebc59e71b8f1c28e718ee71032620f11440c) )
ROM_END

ROM_START( nflclsfb )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x4200000, "user2", 0 ) /* main prg */
	ROM_LOAD( "0.8e",         0x0000000, 0x1080000, CRC(b08d4270) SHA1(5f5dc1c2862292a9e597f6a21c0f9db2e5796ded) )
	ROM_LOAD( "1.8d",         0x1080000, 0x1080000, CRC(d3f519d8) SHA1(60d5f2fafd700e39245bed17e3cc6d608cc2c088) )
	ROM_LOAD( "2.7e",         0x2100000, 0x1080000, CRC(0c65fdc2) SHA1(fa5d41a7b10ae8f8d312b61cc6d34408123bda97) )
	ROM_LOAD( "3.7d",         0x3180000, 0x1080000, CRC(0a4e601d) SHA1(9c302a0b5aaf7046390982e62092b867c3a534a5) )
ROM_END

ROM_START( konotako )
	ROM_REGION32_LE( 0x400000, "maincpu:rom", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION16_LE( 0x4200000, "user2", 0 ) /* main prg */
	ROM_LOAD( "0.8e",         0x0000000, 0x1080000, CRC(63d23a0c) SHA1(31b54119f20827ff13ecf0cd87803a5e27eaafe7) )
	ROM_LOAD( "1.8d",         0x1080000, 0x1080000, CRC(bdbed53c) SHA1(5773069c43642e6f334cee185a6fb6908eedcf4a) )
ROM_END


GAME( 2000, mrdrilr2,  0,        namcos10_memm, namcos10, namcos10_state, mrdrilr2, ROT0, "Namco", "Mr. Driller 2 (Japan, DR21 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // PORT_4WAY joysticks
GAME( 2000, mrdrlr2a,  mrdrilr2, namcos10_memm, namcos10, namcos10_state, mrdrilr2, ROT0, "Namco", "Mr. Driller 2 (Asia, DR22 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // PORT_4WAY joysticks
GAME( 2000, ptblank3,  0,        namcos10_memn, namcos10, namcos10_state, gunbalna, ROT0, "Namco", "Point Blank 3 (Asia, GNN2 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2000, gunbalina, ptblank3, namcos10_memn, namcos10, namcos10_state, gunbalna, ROT0, "Namco", "Gunbalina (Japan, GNN1 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2001, gjspace,   0,        ns10_gjspace , namcos10, namcos10_state, gjspace,  ROT0, "Namco / Metro", "Gekitoride-Jong Space (10011 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2001, mrdrilrg,  0,        namcos10_memn, namcos10, namcos10_state, mrdrilrg, ROT0, "Namco", "Mr. Driller G (Japan, DRG1 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // PORT_4WAY joysticks
GAME( 2001, mrdrilrga, mrdrilrg, namcos10_memn, namcos10, namcos10_state, mrdrilrg, ROT0, "Namco", "Mr. Driller G ALT (Japan, DRG1 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // PORT_4WAY joysticks
GAME( 2001, knpuzzle,  0,        ns10_knpuzzle, namcos10, namcos10_state, knpuzzle, ROT0, "Namco", "Kotoba no Puzzle Mojipittan (Japan, KPM1 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2002, chocovdr,  0,        ns10_chocovdr, namcos10, namcos10_state, chocovdr, ROT0, "Namco", "Uchuu Daisakusen: Chocovader Contactee (Japan, CVC1 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2002, startrgn,  0,        ns10_startrgn, namcos10, namcos10_state, startrgn, ROT0, "Namco", "Star Trigon (Japan, STT1 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
GAME( 2002, panikuru,  0,        namcos10_memn, namcos10, namcos10_state, panikuru, ROT0, "Namco", "Panicuru Panekuru (Japan, PPA1 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2003, nflclsfb,  0,        ns10_nflclsfb, namcos10, namcos10_state, nflclsfb, ROT0, "Namco", "NFL Classic Football (US, NCF3 Ver.A.)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2003, gamshara,  0,        ns10_gamshara, namcos10, namcos10_state, gamshara, ROT0, "Mitchell", "Gamshara (10021 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2003, konotako,  0,        ns10_konotako, namcos10, namcos10_state, konotako, ROT0, "Mitchell", "Kono Tako (10021 Ver.A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
