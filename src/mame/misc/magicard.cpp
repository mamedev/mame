// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Roberto Fresca, David Haywood, Paul Arnold
/******************************************************************************

  MAGIC CARD - IMPERA
  -------------------

  Driver by Roberto Fresca, David Haywood, Angelo Salese and Paul Arnold


  Games running on this hardware:

  * Magic Card (v2.01),                         Impera,  1994.
  * Magic Card (v1.10 14.09.94),                Impera,  1994.
  * Magic Card (v1.5 17.12.93, set 1),          Impera,  1993.
  * Magic Card (v1.5 17.12.93, set 2),          Impera,  1993.
  * Magic Card (v1.2 200/93, set 1),            Impera,  1994.
  * Magic Card (v1.2 200/93, set 2),            Impera,  1994.
  * Magic Card Export 94 (v2.11a, set 1),       Impera,  1994.
  * Magic Card Export 94 (v2.11a, set 2),       Impera,  1994.
  * Magic Card Export 94 (v2.11a, set 3),       Impera,  1994.
  * Magic Card Export 94 (v2.09a),              Impera,  1994.
  * Magic Card III Jackpot (V4.01 6/98),        Impera,  1998.
  * Magic Card III Jackpot (V4.01 7/98),        Impera,  1998.
  * Magic Lotto Export (5.03),                  Impera,  2001.
  * Hot Slots (6.00),                           Impera,  2002.
  * Quingo Export (5.00),                       Impera,  1999.
  * Bel Slots Export (5.01),                    Impera,  1999.
  * Big Deal Belgien (5.04),                    Impera,  2001.
  * Puzzle Me!,                                 Impera,  199?.
  * Lucky 7 (Impera, V04/91a, set 1),           Impera,  1991.
  * Lucky 7 (Impera, V04/91a, set 2),           Impera,  1991.
  * Dallas Poker (CZ/V1),                       AHF Automatentechnik, 1993.
  * Kajot Card (Version 1.01, Wien Euro),       Amatic,  1993.
  * Poker (Impera, V11/90b),                    Impera,  1991.
  * Simply the Best (CZ750, v1.0),              Kajot,   2001.


  TODO:
  - driver based off raw guesses (we don't have relevant key docs);
  - Verify RAM config on PCBs;
  - I/Os;
  - UART;
  - hook-up PIC16F84 when emulation available
  - hotslots, quingo: sets up 68070 timer chs 1 & 2, currently unsupported;
  - bigdeal0: locks with 68070 continually executing address error exception
  - determine what drives int1_w. (It's RTC on boards with RTC) (Not all games use this)
  - Correct memory map - at the moment ROM is mapped to several address spaces for
    all games. This is probably wrong.


*******************************************************************************


  *** Hardware Notes ***

  These are actually the specs of the Philips CD-i console.

  Identified:

  - CPU:  1x Philips SCC 68070 CCA84 (16 bits Microprocessor, PLCC) @ 15 MHz
  - VSC:  1x Philips SCC 66470 CAB (Video and System Controller, QFP)

  - Protection: 1x Dallas TimeKey DS1207-1 (for book-keeping protection)

  - Crystals:   1x 30.0000 MHz.
                1x 19.6608 MHz.

  - PLDs:       1x PAL16L8ACN
                1x PALCE18V8H-25


*******************************************************************************


  *** General Notes ***

  Impera released "Magic Card" in a custom 16-bits PCB.
  The hardware was so expensive and they never have reached the expected sales,
  so... they ported the game to Impera/Funworld 8bits boards, losing part of
  graphics and sound/music quality. The new product was named "Magic Card II".

  On first power up many games appear to lock with an error due to nvr not being
  initialised. The following procedures fix this...

  magicard shows STATIC MEMORY ERROR.
  Press OWNER BOOK KEEPING button and game will continue and finally show
  OUT OF RANGE ERROR. Press OWNER BOOK KEEPING again and game shows test menu
  with "!PRESS HOLD4 for 5sec   TO INIT MACHINE!".  Press HOLD 4 for 5 secs and
  after this game should start without error.

  magicrd1a/magicrd1b/magicrd1c/magicrd1/magicrd1d show !!ERROR!! CALL SERVICEMAN 4
  Press OWNER BOOK KEEPING. Game enters test Press HOLD3 for next test.
  Select PREFERENCES I and then RAM RESET.
  magicrd1a/magicrd1b/magicrd1d lock with TURN OFF!
  Quit and restart game.

  magicrde/magicrdea/magicrdeb
  Show STATIC MEMORY ERROR.
  Press OWNER BOOK KEEPING to get past this and again when error appears later.
  Use HOLD3 to enter menu and CLEAR to get hidden test. Press HOLD5 for 5 secs
  will initialise machine.

  magicrde/magicrdea alarm when credit is added and credit is then cleared so can't be played.
  Needs further investigation.

  Lucky7i shows !! FEHLER !! BITTE TECHNIKER RUFEN 4
  Press OWNER BOOK Keeping. Game enters test. Press HOLD 3 (Einstellungen)
  and then HOLD 5 (RAM RESET).
  Machine shows "GERAET JETZT AUS. UND WIEDER EINSTECKEN!!!

  magicrdec fails on OUT OF RANGE ERROR..
  magicrdj/magicrdja fail on ERROR IN SETTINGS

  kajotcrd gets stuck during initialisation. Possible eeprom contents issue ?
  dallaspk requires protection device (serial port ?)

  magicle/hotslots/quingo/belslots fail on I2C BUS ERROR - protection device missing.

  puzzleme - appears to work. Amount of credit for each coin input is set by values
             in the eeprom but there doesn't appear to be any tests to set these.
             Is something missing on this game ?

*******************************************************************************

  Impera boards...

  KNOWN REVS | Used with these games          | Differences to previews Revision
  ======================================================================================================
  V 1.04     | lucky7i                        | lowest known revision, does not have a socket for the PIC
  ------------------------------------------------------------------------------------------------------
  V 1.05     | magicrd1, magicrd1c            | PIC16C54 + XTAL got added
  ------------------------------------------------------------------------------------------------------
  V 2.1      | puzzleme                       | ESI1, 24C02, YM2149F, RTC added
  ------------------------------------------------------------------------------------------------------
  V 2.2      | magicrde                       |
  ------------------------------------------------------------------------------------------------------
  V 4.0      | magicrdja, magicrdeb, magicle, | ESI1 replaced by ALTERA MAX EPM7128SQC100
             | simpbest                       | YM2149F replaced by YMZ284-D, MX29F1610 added
  ------------------------------------------------------------------------------------------------------


*******************************************************************************

  For PCB layouts and extra info see the ROM Load of each game below.

*******************************************************************************

  The information below is purely to aid development and should be removed
  when complete.
  There are some mysteries - eg. magicrdeb appears to address an I2C address where
  there is no device but the game seems to work without ?

*******************************************************************************

                DS2401     DS1207   EP      PROTECT  PROT AVAIL     ELO TOUCH
  magicard      NO          YES[3]  24c02   YES         NO
  magicrd1      NO          YES[2]  24c02   16C56       YES
  magicrd1a     NO          YES[3]  24c02   YES         NO
  magicrd1b     NO          YES[2]  24c02?  YES         NO
  magicrd1c     NO ?        YES[2]  24c02?  16C54       YES
  magicrd1d     NO          YES[2]  24c02   YES         NO
  magicrde      YES         YES[1]  24c02   16C54       YES
  magicrdea     YES         YES[1]  24c02   16C54       YES
  magicrdeb     YES         NO      24c02   ?? *1
  magicrdec     YES         YES[1]  24c02?  YES         NO
  magicrdj      YES         NO      24c02?  YES         NO
  magicrdja     YES         NO ?    24c02   16F84       YES*
  magicle       ?           ?       24c04   16F84       YES*        YES
  hotslots      ?           ?       24c02   YES         NO
  quingo        ?           ?       24c04   YES         NO          YES
  belslots      ?           ?       24c04   YES         NO          YES
  bigdeal0      ?           ?       24c04   ?
  puzzleme      NO          NO      24c02   16C54       YES
  lucky7i                   YES[3]  24c02   NO
  dallaspk      NO          YES[4]  24c02   YES         NO          protection via serial port ?
  kajotcrd      YES         ??      24c02   YES         NO

  PIC16F84 emulation not available

  [1] Use same signature
  [2] Use same signature
  [3] Use unique signature
  [4] Use unique signature, 8 byte identification value isn't validated by the game.

  *1 Accesses I2C device 0x48 but fails. Game works anyway.

*******************************************************************************/

#include "emu.h"
#include "cpu/pic16c5x/pic16c5x.h"
#include "cpu/pic16x8x/pic16x8x.h"
#include "machine/ds1207.h"
#include "machine/ds2401.h"
#include "machine/i2cmem.h"
#include "machine/msm6242.h"
#include "machine/nvram.h"
#include "machine/scc66470.h"
#include "machine/scc68070.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "sound/saa1099.h"
#include "video/ramdac.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "magicard.lh"
#include "dallaspk.lh"
#include "pokeri.lh"
#include "simpbest.lh"

#define CLOCK_A XTAL(30'000'000)
#define CLOCK_B XTAL(8'000'000)
#define CLOCK_C XTAL(19'660'800)

namespace {

enum
{
	I2C_CPU = 0,
	I2C_PIC,
	i2C_MEM
};

class magicard_base_state : public driver_device
{
public:
	magicard_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_scc66470(*this,"scc66470")
		, m_i2cmem(*this, "sereeprom")
		, m_lamps(*this, "lamp%u", 1U)

	{ }

	void magicard_base(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

	uint32_t screen_update_magicard(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void ramdac_map(address_map &map);

	void dram_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t dram_r(offs_t offset, uint16_t mem_mask);
	void mcu_dram_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint16_t mcu_dram_r(offs_t offset, uint16_t mem_mask);
	uint8_t pic_portb_r();
	void pic_portb_w(offs_t offset, uint8_t data, uint8_t mask);
	void output_w(offs_t offset, uint16_t data);

	void cpu_int1(int state);

	void scc66470_irq(int state);
	void cpu_i2c_scl(int state);
	void cpu_i2c_sda_write(int state);
	int cpu_i2c_sda_read();

	void update_sda(uint8_t device, uint8_t state);
	void update_scl(uint8_t device, uint8_t state);

	required_device<scc68070_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<scc66470_device> m_scc66470;
	required_device<i2cmem_device> m_i2cmem;

	uint8_t m_sda_state;
	uint8_t m_scl_state;

private:
	output_finder<8> m_lamps;

	void scc66470_map(address_map &map);

	std::unique_ptr<uint16_t []> m_dram;

};

class magicard_state : public magicard_base_state
{
public:
	magicard_state(const machine_config &mconfig, device_type type, const char *tag)
		: magicard_base_state(mconfig, type, tag)
		, m_nvram(*this, "nvram")
		, m_ds1207(*this, "ds1207")
	{ }

	void magicard_pic54(machine_config &config);
	void magicard(machine_config &config);
	void magicard_pic56(machine_config &config);

	void init_dallaspk();

protected:
	virtual void machine_start() override;

private:
	void magicard_map(address_map &map);
	uint8_t nvram_r(offs_t offset);
	void nvram_w(offs_t offset, uint8_t data);
	uint8_t read_ds1207(offs_t offset);
	void write_ds1207(offs_t offset, uint8_t data);

	required_device<nvram_device> m_nvram;
	required_device<ds1207_device> m_ds1207;

	std::unique_ptr<uint8_t[]> m_nvram8;
};

class hotslots_state : public magicard_base_state
{
public:
	hotslots_state(const machine_config &mconfig, device_type type, const char *tag)
		: magicard_base_state(mconfig, type, tag)
		, m_ds2401(*this, "serial_id")
		, m_ds1207(*this, "ds1207")
		, m_rtc(*this, "rtc")
	{ }

	void hotslots_base(machine_config &config);
	void hotslots(machine_config &config);
	void hotslots_pic54(machine_config &config);
	void magicle(machine_config &config);
	void puzzleme(machine_config &config);
	void simpbest(machine_config &config);

private:
	void hotslots_map_base(address_map &map);
	void hotslots_map(address_map &map);
	void puzzleme_map(address_map &map);
	void simpbest_map(address_map &map);

	uint8_t read_ds1207_ds2401(offs_t offset);
	void write_ds1207_ds2401(offs_t offset, uint8_t data);
	//void output_w(offs_t offset, uint16_t data);

	void cpu_int1(int state);

	optional_device<ds2401_device> m_ds2401;
	optional_device<ds1207_device> m_ds1207;
	required_device<rtc72421_device> m_rtc;
};


void magicard_base_state::machine_start()
{
	m_lamps.resolve();
	m_dram = make_unique_clear<uint16_t []>(0x80000 / 2);
	save_pointer(NAME(m_dram), 0x80000 / 2);
	save_item(NAME(m_sda_state));
	save_item(NAME(m_scl_state));
}

void magicard_state::machine_start()
{
	magicard_base_state::machine_start();
	m_nvram8 = std::make_unique<uint8_t []>(16384);
	m_nvram->set_base(m_nvram8.get(), 16384);
}

void magicard_state::nvram_w(offs_t offset, uint8_t data)
{
	m_nvram8[ offset ] = data;
}

uint8_t magicard_state::nvram_r(offs_t offset)
{
	return m_nvram8[ offset ];
}

void magicard_base_state::mcu_dram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if(!machine().side_effects_disabled())
	{
		m_maincpu->eat_cycles(m_maincpu->attotime_to_cycles(attotime::from_ticks(m_scc66470->dram_dtack_cycles(), m_scc66470->clock())));
	}
	m_scc66470->dram_w(offset, data, mem_mask);
}

uint16_t magicard_base_state::mcu_dram_r(offs_t offset, uint16_t mem_mask)
{
	if(!machine().side_effects_disabled())
	{
		m_maincpu->eat_cycles(m_maincpu->attotime_to_cycles(attotime::from_ticks(m_scc66470->dram_dtack_cycles(), m_scc66470->clock())));
	}
	return m_scc66470->dram_r(offset);
}

void magicard_base_state::dram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_dram[offset]);
}

uint16_t magicard_base_state::dram_r(offs_t offset, uint16_t mem_mask)
{
	return m_dram[ offset ] & mem_mask;
}

void magicard_base_state::scc66470_map(address_map &map)
{
	map(0x00000, 0x7ffff).rw(FUNC(magicard_base_state::dram_r), FUNC(magicard_base_state::dram_w));
}

uint32_t magicard_base_state::screen_update_magicard(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if(cliprect.min_y == cliprect.max_y)
	{
		uint8_t buffer[768];
		m_scc66470->line(cliprect.min_y, buffer, sizeof(buffer));
		uint32_t *dest = &bitmap.pix(cliprect.min_y);

		for(int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			*dest++ = m_palette->pen(buffer[ x ]);
		}
	}
	return 0;
}


/*******************************************
*               R/W Handlers               *
*******************************************/

uint8_t magicard_state::read_ds1207(offs_t offset)
{
	return m_ds1207->read_dq() ? 0x08 : 0x00;
}

void magicard_state::write_ds1207(offs_t offset, uint8_t data)
{
	m_ds1207->write_rst(BIT(data, 0));
	m_ds1207->write_clk(BIT(data, 1));
	m_ds1207->write_dq(BIT(data, 3));
}

void magicard_base_state::output_w(offs_t offset, uint16_t data)
{
	// bit  0 - counter out
	// bit  1 - ??
	// bit  2 - ??
	// bit  3 - counter jackpot
	// bit  4 - counter checkout
	// bit  5 - ??
	// bit  6 - ??
	// bit  7 - hold 3 lamp
	// bit  8 - start lamp
	// bit  9 - hold 1 lamp
	// bit 10 - hold 5 lamp
	// bit 11 - hold 2 lamp
	// bit 12 - hold 4 lamp
	// bit 13 - clear lamp
	// bit 14 - hopper drive - hold 3 lamp (pokeri)
	// bit 15 - counter in

	m_lamps[0] = BIT(data, 9);      // Lamp 0 - HOLD 1
	m_lamps[1] = BIT(data, 11);     // Lamp 1 - HOLD 2
	m_lamps[3] = BIT(data, 12);     // Lamp 3 - HOLD 4
	m_lamps[4] = BIT(data, 10);     // Lamp 4 - HOLD 5
	m_lamps[5] = BIT(data, 13);     // Lamp 5 - CANCEL
	m_lamps[6] = BIT(data, 8);      // Lamp 6 - START

	m_lamps[2] = (BIT(data, 7) | BIT(data, 14));      // Lamp 2 - HOLD 3
}


uint8_t hotslots_state::read_ds1207_ds2401(offs_t offset)
{
	m_ds2401->write(true);
	return (m_ds2401->read() ? 0x10 : 0x00) | (m_ds1207->read_dq() ? 0x08 : 0x00);
}

void hotslots_state::write_ds1207_ds2401(offs_t offset, uint8_t data)
{
	m_ds2401->write(BIT(data, 4));
	m_ds1207->write_rst(BIT(data, 0));
	m_ds1207->write_clk(BIT(data, 1));
	m_ds1207->write_dq(BIT(data, 3));
}

/*
void hotslots_state::output_w(offs_t offset, uint16_t data)
{
    // bit  0 - counter out
    // bit  1 - counter key switch
    // bit  2 - ??
    // bit  3 - counter hopper refill
    // bit  4 - counter cashbox
    // bit  5 - ??
    // bit  6 - ??
    // bit  7 - hold 3 lamp
    // bit  8 - ??
    // bit  9 - hold 1 lamp
    // bit 10 - hold 5 lamp
    // bit 11 - hold 2 lamp
    // bit 12 - hold 4 lamp
    // bit 13 - clear lamp
    // bit 14 - hopper drive
    // bit 15 - counter in
}
*/

/*********************************************
*           Memory Map Information           *
*********************************************/

void magicard_state::magicard_map(address_map &map)
{
	map(0x00000000, 0x001fffff).m(m_scc66470, FUNC(scc66470_device::map));
	map(0x00000000, 0x0017ffff).rw(FUNC(magicard_state::mcu_dram_r), FUNC(magicard_state::mcu_dram_w));
	map(0x00180000, 0x001dffff).rom().region("maincpu", 0); // boot vectors point here
	map(0x001e0000, 0x001e7fff).rw(FUNC(magicard_state::nvram_r), FUNC(magicard_state::nvram_w)).umask16(0x00ff);
	map(0x00200000, 0x003fffff).rw(m_scc66470, FUNC(scc66470_device::ipa_r), FUNC(scc66470_device::ipa_w));
	/* 001ffc00-001ffdff System I/O */
	map(0x001ffc00, 0x001ffc01).portr("IN0");
	map(0x001ffc40, 0x001ffc41).portr("IN1");
	map(0x001ffc80, 0x001ffc81).w( FUNC(magicard_state::output_w));
	map(0x001ffd01, 0x001ffd01).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x001ffd03, 0x001ffd03).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x001ffd05, 0x001ffd05).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0x001ffd40, 0x001ffd43).w("saa", FUNC(saa1099_device::write)).umask16(0x00ff);
	map(0x001ffd80, 0x001ffd81).rw(FUNC(magicard_state::read_ds1207), FUNC(magicard_state::write_ds1207)).umask16(0x00ff);
	map(0x001fff80, 0x001fffbf).ram(); //DRAM I/O, not accessed by this game, CD buffer?
}

// Different PAL mapping?
void hotslots_state::hotslots_map_base(address_map &map)
{
	map(0x00000000, 0x001fffff).m(m_scc66470, FUNC(scc66470_device::map));
	map(0x00000000, 0x0017ffff).rw(FUNC(hotslots_state::mcu_dram_r), FUNC(hotslots_state::mcu_dram_w));
	map(0x00180000, 0x001ffbff).rom().region("maincpu", 0); // boot vectors point here
	map(0x00200000, 0x003fffff).rw(m_scc66470, FUNC(scc66470_device::ipa_r), FUNC(scc66470_device::ipa_w));
	map(0x00600000, 0x0067fbff).rom().region("maincpu", 0); // boot vectors point here
	map(0x00680000, 0x006ffbff).rom().region("maincpu", 0); // boot vectors point here
	map(0x00800000, 0x0087fbff).rom().region("maincpu", 0); // boot vectors point here
	map(0x001fff80, 0x001fffbf).ram(); //DRAM I/O, not accessed by this game, CD buffer?
	map(0x00400000, 0x0040ffff).ram().share("nvram");
	map(0x00411000, 0x00411001).portr("IN0");
	map(0x00412346, 0x00412347).portr("IN1");
	map(0x00413000, 0x00413001).w( FUNC(hotslots_state::output_w));
	map(0x00414001, 0x00414001).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x00414003, 0x00414003).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x00414005, 0x00414005).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0x00414007, 0x00414007).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x00415003, 0x00415003).r("ramdac", FUNC(ramdac_device::pal_r));
	map(0x00416001, 0x00416001).w("ssg", FUNC(ymz284_device::data_w));
	map(0x00417001, 0x00417001).w("ssg", FUNC(ymz284_device::address_w));
	map(0x00418000, 0x00418020).rw("rtc", FUNC(rtc72421_device::read), FUNC(rtc72421_device::write)).umask16(0x00ff);
	map(0x0041a000, 0x0041a001).nopw(); // supervisor?
}

void hotslots_state::hotslots_map(address_map &map)
{
	hotslots_map_base(map);
	map(0x00419566, 0x00419567).rw(FUNC(hotslots_state::read_ds1207_ds2401), FUNC(hotslots_state::write_ds1207_ds2401)).umask16(0x00ff);
}

void hotslots_state::puzzleme_map(address_map &map)
{
	hotslots_map_base(map);
}


void hotslots_state::simpbest_map(address_map &map)
{
	map(0x00000000, 0x001fffff).m(m_scc66470, FUNC(scc66470_device::map));
	map(0x00000000, 0x0017ffff).rw(FUNC(hotslots_state::mcu_dram_r), FUNC(hotslots_state::mcu_dram_w));
	map(0x001fff80, 0x001fffbf).ram(); //DRAM I/O

	map(0x00200000, 0x003fffff).rw(m_scc66470, FUNC(scc66470_device::ipa_r), FUNC(scc66470_device::ipa_w));
	map(0x00600000, 0x0067ffff).rom().region("maincpu", 0); // boot vectors point here

	map(0x00400000, 0x00400001).portr("IN0");
	map(0x00401000, 0x00401001).portr("IN1");
	map(0x00402000, 0x00402001).w( FUNC(hotslots_state::output_w));

	map(0x00403001, 0x00403001).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x00403003, 0x00403003).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x00403005, 0x00403005).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0x00403007, 0x00403007).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x00404003, 0x00404003).r("ramdac", FUNC(ramdac_device::pal_r));

	map(0x00405003, 0x00405003).w("ssg", FUNC(ymz284_device::data_w));
	map(0x00406001, 0x00406001).w("ssg", FUNC(ymz284_device::address_w));

	map(0x00407000, 0x00407020).rw("rtc", FUNC(rtc72421_device::read), FUNC(rtc72421_device::write)).umask16(0x00ff);

	map(0x00500000, 0x0050ffff).ram().share("nvram");

}


/*********************************************
*                Input Ports                 *
*********************************************/

static INPUT_PORTS_START( magicard )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN4 )            PORT_NAME("Remote 2")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )            PORT_NAME("Remote 1")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )      PORT_NAME("Rental Book Keeping")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE1 )         PORT_NAME("Owner Book Keeping")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )    PORT_NAME("Pay/Hopper Out")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_OTHER )            PORT_NAME("Hopper Count")    PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SERVICE2 )         PORT_NAME("Books3/Service")  PORT_CODE(KEYCODE_U)

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "Keyboard Test" )             PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Remote/Keyboard" )           PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, "Remote Switch" )
	PORT_DIPSETTING(    0x00, "Keyboard" )
	PORT_DIPNAME( 0x04, 0x04, "Swap Coin Inputs" )          PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x38, 0x38, "Setting" )                   PORT_DIPLOCATION("SW1:5,4,3")
	PORT_DIPSETTING(    0x38, "Austria 1" )
	PORT_DIPSETTING(    0x30, "Austria 2" )
	PORT_DIPSETTING(    0x18, "CSFR 1" )  // Czech Slovak Federal Republic
	PORT_DIPSETTING(    0x10, "CSFR 2" )  // Czech Slovak Federal Republic
	PORT_DIPSETTING(    0x28, "Germany 1" )
	PORT_DIPSETTING(    0x20, "Germany 2" )
	PORT_DIPSETTING(    0x08, "Hungary 1" )
	PORT_DIPSETTING(    0x00, "Hungary 2" )
	PORT_DIPNAME( 0x40, 0x40, "Hopper" )                    PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, "Coin B" )
	PORT_DIPSETTING(    0x00, "Coin A" )
	PORT_DIPNAME( 0x80, 0x80, "Hopper" )                    PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR )  PORT_NAME("Door Switch")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Reserve In 5")    PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Reserve In 6")    PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Reserve In 7")    PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Reserve In 8")    PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Reserve In 9")    PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Hopper Full")     PORT_CODE(KEYCODE_R)

INPUT_PORTS_END

static INPUT_PORTS_START( magicrde )
	PORT_INCLUDE( magicard )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON7 )     PORT_NAME("Alarm")           PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON6 )     PORT_NAME("Counter Control") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNUSED )      // PORT_NAME("N/C 1")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )      // PORT_NAME("N/C 2")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON5 )     PORT_NAME("Clear coinCard")  PORT_CODE(KEYCODE_H)

INPUT_PORTS_END

static INPUT_PORTS_START( puzzleme )
	PORT_START("IN0")

	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN3 )            PORT_NAME("Remote")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_MEMORY_RESET )     PORT_NAME("Clear")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE3 )         PORT_NAME("Show All Book Keeping")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )      PORT_NAME("Rental Book Keeping")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE1 )         PORT_NAME("Owner Book Keeping")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_CONFNAME( 0x1800, 0x1800, DEF_STR( Difficulty ) )
	PORT_CONFSETTING(      0x1800, DEF_STR( Normal ) )
	PORT_CONFSETTING(      0x0000, DEF_STR( Easy ) )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNUSED )

INPUT_PORTS_END

static INPUT_PORTS_START( lucky7i )
	PORT_INCLUDE( magicard )

	PORT_MODIFY("IN0")

//  PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON1 )          PORT_NAME("Win Plan Scroll/Collect")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )      PORT_NAME("Hold 5 / Bet (Einsatz)")
//  PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )           PORT_NAME("Start/Gamble")
//  PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )      PORT_NAME("Rental Book Keeping")
//  PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Remote 1" )                  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, "100" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x04, 0x04, "Munzer2" )                   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x08, 0x08, "Munzer1" )                   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Remote 2" )                  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, "100" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x40, 0x40, "Hopper-Wert" )               PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x80, 0x80, "Hopper" )                    PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

//  PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )    PORT_NAME("Attendant Collect")
//  PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNUSED )
//  PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

INPUT_PORTS_END


static INPUT_PORTS_START( dallaspk )
	PORT_INCLUDE( magicard )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_POKER_CANCEL )     PORT_NAME("Cancel / Take")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )      PORT_NAME("Hold 4 / Black")
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )      PORT_NAME("Hold 2 / Red")

	PORT_MODIFY("IN1")
	PORT_DIPNAME( 0x01, 0x01, "I/O Test" )                  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Remote 1" )                  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, "100" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x04, 0x04, "Munzer2" )                   PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x08, 0x08, "Munzer1" )                   PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Remote 2" )                  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, "100" )
	PORT_DIPSETTING(    0x00, "50" )
	PORT_DIPNAME( 0x40, 0x40, "Hopper-Wert" )               PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x80, 0x80, "Hopper" )                    PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END

static INPUT_PORTS_START( hotslots )
	PORT_INCLUDE( magicard )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON5 )     PORT_NAME("Alarm")           PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON6 )     PORT_NAME("Counter Control") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON7 )     PORT_NAME("Clear coinCard")  PORT_CODE(KEYCODE_H)

INPUT_PORTS_END


static INPUT_PORTS_START( pokeri )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN4 )            PORT_NAME("Remote 2")
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN3 )            PORT_NAME("Remote 1")
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )      PORT_NAME("Rental Book Keeping")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE1 )         PORT_NAME("Owner Book Keeping")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )    PORT_NAME("Pay/Hopper Out")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_OTHER )            PORT_NAME("Hopper Count")    PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SERVICE2 )         PORT_NAME("Books3/Service")  PORT_CODE(KEYCODE_U)

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "Service Test" )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Remote 1" )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, "100" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x04, 0x04, "Coin 2" )            PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x08, 0x08, "Coin 1" )            PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, "10" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, "Cards Back" )        PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, "Normal Clean" )
	PORT_DIPSETTING(    0x00, "Impera Logo" )
	PORT_DIPNAME( 0x20, 0x20, "Remote 2" )          PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, "100" )
	PORT_DIPSETTING(    0x00, "1M" )
	PORT_DIPNAME( 0x40, 0x40, "DSW 1:2, unknown" )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW 1:1, unknown" )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR )  PORT_NAME("Door Switch")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Reserve In 5")    PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Reserve In 6")    PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Reserve In 7")    PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Reserve In 8")    PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Reserve In 9")    PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Hopper Full")     PORT_CODE(KEYCODE_R)

INPUT_PORTS_END


static INPUT_PORTS_START( simpbest )

	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_POKER_CANCEL )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )      PORT_NAME("Rental Book Keeping")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_SERVICE1 )         PORT_NAME("Owner Book Keeping")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )    PORT_NAME("Pay/Hopper Out")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_OTHER )            PORT_NAME("Hopper Count")   PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_SERVICE2 )         PORT_NAME("Service")        PORT_CODE(KEYCODE_U)

	PORT_START("IN1")  // just for testing... the board lacks of them.
	PORT_DIPNAME( 0x01, 0x00, "DIP switches?" )             PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )          PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT ) PORT_NAME("Clear Credits")
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR )  PORT_NAME("Door Switch")
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Coin Card")      PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Not used")       PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Not used")       PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Counter Check")  PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Key Alarm")      PORT_CODE(KEYCODE_G)  // "A L A R A M" in the I/O test.
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_OTHER )        PORT_NAME("Hopper Full")    PORT_CODE(KEYCODE_R)

INPUT_PORTS_END


void magicard_base_state::machine_reset()
{
	uint16_t *const src = (uint16_t *)memregion("maincpu")->base();
	m_scc66470->set_vectors(src);

	m_sda_state = 0;
	m_scl_state = 0;
}


/*********************************************
*              Machine Drivers               *
*********************************************/

void magicard_base_state::scc66470_irq(int state)
{
	m_maincpu->int2_w(state);
}

void magicard_base_state::cpu_int1(int state)
{
	// TODO: is this used by games on magicard hardware ?
	m_maincpu->int1_w(1);
	m_maincpu->int1_w(0);
}

void hotslots_state::cpu_int1(int state)
{
	m_maincpu->int1_w(state);
}

void magicard_base_state::ramdac_map(address_map &map)
{
	map(0x0000, 0x03ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

void magicard_base_state::update_scl(uint8_t device, uint8_t state)
{
	if(state)
	{
		m_scl_state &= ~(1<<device);
	}
	else
	{
		m_scl_state |= (1<<device);
	}
	m_i2cmem->write_scl(m_scl_state ? 0 : 1);
	m_maincpu->write_scl(m_scl_state ? 0 : 1);
}

void magicard_base_state::update_sda(uint8_t device, uint8_t state)
{
	if(state)
	{
		m_sda_state &= ~(1<<device);
	}
	else
	{
		m_sda_state |= (1<<device);
	}
	m_i2cmem->write_sda(m_sda_state ? 0 : 1);
}

void magicard_base_state::cpu_i2c_scl(int state)
{
	update_scl(I2C_CPU, state);
}

void magicard_base_state::cpu_i2c_sda_write(int state)
{
	update_sda(I2C_CPU, state);
}

int magicard_base_state::cpu_i2c_sda_read()
{
	return (m_sda_state ? 0 : 1) & (m_i2cmem->read_sda() ? 1 : 0);
}

void magicard_base_state::magicard_base(machine_config &config)
{
	SCC68070(config, m_maincpu, CLOCK_C);  // SCC-68070 CCA84
	m_maincpu->i2c_scl_w().set(FUNC(magicard_base_state::cpu_i2c_scl));
	m_maincpu->i2c_sda_w().set(FUNC(magicard_base_state::cpu_i2c_sda_write));
	m_maincpu->i2c_sda_r().set(FUNC(magicard_base_state::cpu_i2c_sda_read));

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw( CLOCK_A/2, 960, 0, 768, 312, 32, 312);
	m_screen->set_video_attributes(VIDEO_UPDATE_SCANLINE);
	m_screen->set_screen_update(FUNC(magicard_base_state::screen_update_magicard));

	SCC66470(config,m_scc66470,CLOCK_A);
	m_scc66470->set_addrmap(0, &magicard_base_state::scc66470_map);
	m_scc66470->set_screen("screen");
	m_scc66470->irq().set(FUNC(magicard_base_state::scc66470_irq));

	PALETTE(config, m_palette).set_entries(0x100);
	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, m_palette));
	ramdac.set_addrmap(0, &magicard_state::ramdac_map);

	SPEAKER(config, "mono").front_center();

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

void magicard_state::magicard(machine_config &config)
{
	magicard_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &magicard_state::magicard_map);

	m_screen->screen_vblank().set(FUNC(magicard_state::cpu_int1));

	SAA1099(config, "saa", CLOCK_B).add_route(ALL_OUTPUTS, "mono", 1.0);

	I2C_24C02(config, m_i2cmem).set_e0(1);

	DS1207(config, "ds1207");
}

void magicard_state::magicard_pic54(machine_config &config)
{
	magicard(config);

	pic16c54_device &pic(PIC16C54(config, "pic16c54", 3686400));  // correct?
	pic.read_b().set(FUNC(magicard_state::pic_portb_r));
	pic.write_b().set(FUNC(magicard_state::pic_portb_w));
}

void magicard_state::magicard_pic56(machine_config &config)
{
	magicard(config);

	pic16c56_device &pic(PIC16C56(config, "pic16c56", 3686400));  // correct?
	pic.read_b().set(FUNC(magicard_state::pic_portb_r));
	pic.write_b().set(FUNC(magicard_state::pic_portb_w));
}

uint8_t magicard_base_state::pic_portb_r()
{
	return (m_sda_state ? 0 : 0x80) | (m_scl_state ? 0 : 0x40);
}

void magicard_base_state::pic_portb_w(offs_t offset, uint8_t data, uint8_t mask)
{
	update_scl(I2C_PIC,((data & mask) | (~mask & 0x40)) & 0x40);
	update_sda(I2C_PIC,((data & mask) | (~mask & 0x80)) & 0x80);
}

void hotslots_state::hotslots_base(machine_config &config)
{
	magicard_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &hotslots_state::hotslots_map);

	YMZ284(config, "ssg", 4000000).add_route(ALL_OUTPUTS, "mono", 1.0);

	RTC72421(config, m_rtc, XTAL(32'768)).out_int_handler().set(FUNC(hotslots_state::cpu_int1));

	DS2401(config, "serial_id");

	DS1207(config, "ds1207");
}

void hotslots_state::hotslots(machine_config &config)
{
	hotslots_base(config);

	I2C_24C02(config, m_i2cmem).set_e0(1);
}

void hotslots_state::hotslots_pic54(machine_config &config)
{
	hotslots(config);

	pic16c54_device &pic(PIC16C54(config, "pic16c54", 3686400));  // correct?
	pic.read_b().set(FUNC(hotslots_state::pic_portb_r));
	pic.write_b().set(FUNC(hotslots_state::pic_portb_w));
}

void hotslots_state::magicle(machine_config &config)
{
	hotslots_base(config);

	pic16f84_device &pic(PIC16F84(config, "pic16f84", 4000000)); 
	pic.set_config(0x3ffa); // No protect - No Watchdog - HS Clock
	pic.read_b().set(FUNC(hotslots_state::pic_portb_r));
	pic.write_b().set(FUNC(hotslots_state::pic_portb_w));

	I2C_24C04(config, m_i2cmem).set_e0(1);
}

void hotslots_state::puzzleme(machine_config &config)
{
	hotslots_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &hotslots_state::puzzleme_map);

	pic16c54_device &pic(PIC16C54(config, "pic16c54", 3686400));  // correct?
	pic.read_b().set(FUNC(hotslots_state::pic_portb_r));
	pic.write_b().set(FUNC(hotslots_state::pic_portb_w));

	I2C_24C02(config, m_i2cmem).set_e0(1);

	config.device_remove("serial_id");
	config.device_remove("ds1207");
}

void hotslots_state::simpbest(machine_config &config)
{
	hotslots_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &hotslots_state::simpbest_map);

//  m_screen->set_visarea(4*8, 88*8-1, 6*8, 37*8);
	m_screen->screen_vblank().set(FUNC(hotslots_state::cpu_int1));

	I2C_24C04(config, m_i2cmem).set_e0(1);

	config.device_remove("serial_id");
	config.device_remove("ds1207");
}


/*********************************************
*                  Rom Load                  *
*********************************************/

/*
  Magicard Ver 2.01
*/
ROM_START( magicard )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "magicorg.bin", 0x000000, 0x80000, CRC(810edf9f) SHA1(0f1638a789a4be7413aa019b4e198353ba9c12d9) )

	ROM_REGION( 0x0100, "sereeprom", 0 )  // Serial EPROM
	ROM_LOAD16_WORD_SWAP("mgorigee.bin",    0x0000, 0x0100, CRC(f09eb2b2) SHA1(2d6efcea6c0835ea754285e22354dff8f059fdf5) )

	ROM_REGION(0x4d, "ds1207", 0)  // timekey
	ROM_LOAD( "ds1207", 0x000000, 0x00004d, BAD_DUMP CRC(48528ccf) SHA1(182f5aa2938328bac59110eee1b340b3b4ea3e29) )  // created to match game

	ROM_REGION(0x4000, "nvram", 0)  // Default NVRAM
	ROM_LOAD( "magicard.nv", 0x0000, 0x4000, CRC(3b7d957e) SHA1(2c56b7f37a2166a99c9e6b05d90ace0a4dd179e2) )
ROM_END

/*
  Magicard 1.5 17.12.93
*/
ROM_START( magicrd1a )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "mcorigg2.bin", 0x00000, 0x20000, CRC(48546aa9) SHA1(23099a5e4c9f2c3386496f6d7f5bb7d435a6fb16) )
	ROM_RELOAD(                           0x40000, 0x20000 )
	ROM_LOAD16_WORD_SWAP( "mcorigg1.bin", 0x20000, 0x20000, CRC(c9e4a38d) SHA1(812e5826b27c7ad98142a0f52fbdb6b61a2e31d7) )
	ROM_RELOAD(                           0x40001, 0x20000 )

	ROM_REGION( 0x0100, "sereeprom", 0 )  // Serial EPROM
	ROM_LOAD("mgorigee.bin",    0x0000, 0x0100, CRC(f09eb2b2) SHA1(2d6efcea6c0835ea754285e22354dff8f059fdf5) )

	ROM_REGION(0x4d, "ds1207", 0)  // timekey
	ROM_LOAD( "ds1207", 0x000000, 0x00004d, BAD_DUMP CRC(4902b7c2) SHA1(6e6fe825cfcf39bae60ecc45ab0742772f87cf80) )  // created to match game

	ROM_REGION(0x4000, "nvram", 0)  // Default NVRAM
	ROM_LOAD( "magicrd1a.nv", 0x0000, 0x4000, CRC(4d78bbcc) SHA1(943344f03a69ee25526e2b1f2e74722ae2601c11) )
ROM_END

/*
  Magicard 1.5 17.12.93
*/
ROM_START( magicrd1b )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "mg_8.bin", 0x00000, 0x80000, CRC(f5499765) SHA1(63bcf40b91b43b218c1f9ec1d126a856f35d0844) )

	// bigger than the other sets?
	ROM_REGION( 0x20000, "other", 0 )  // unknown
	ROM_LOAD16_WORD_SWAP("mg_u3.bin",   0x00000, 0x20000, CRC(2116de31) SHA1(fb9c21ca936532e7c342db4bcaaac31c478b1a35) )

	ROM_REGION( 0x0100, "sereeprom", 0 )  // Serial EPROM
	ROM_LOAD("mgorigee.bin",    0x0000, 0x0100, CRC(fea8a821) SHA1(c744cac6af7621524fc3a2b0a9a135a32b33c81b) )

	ROM_REGION(0x4d, "ds1207", 0)  // timekey
	ROM_LOAD( "ds1207", 0x000000, 0x00004d, BAD_DUMP CRC(cbcc1a42) SHA1(4b577c85f5856192ce04051a2d305a9080192177) )  // created to match game

	ROM_REGION(0x4000, "nvram", 0)  // Default NVRAM
	ROM_LOAD( "magicrd1b.nv", 0x0000, 0x4000, CRC(4d78bbcc) SHA1(943344f03a69ee25526e2b1f2e74722ae2601c11) )
ROM_END

/*
   Magic Card Jackpot 4.01
  (Also Magic Lotto Export)
  -------------------------

  PCB Layout:
   __________________________________________________________________________________________________
  |                                                                                                  |
  |                       SERIAL NUMBER                                                              |___
  |                                                                                                  (A)_|
  |  __     _____             ___________        _________                                            ___|
  | |  |   /     \           |9524 GNN   |      |YMZ284-D |                                           ___|
  | |  |  |BATTERY|          |HM514270AJ8|      |_________|                                           ___|
  | |A |  |  +3V  |          |___________|                                                            ___|
  | |  |  |       |                                       __________________         ________     __  ___|
  | |  |   \_____/            ___________                |                  |       |ULN2803A|   |..| ___|
  | |__|                     |9524 GNN   |               |  MUSIC           |       |________|   |..| ___|
  |                          |HM514270AJ8|               |  TR9C1710-11PCA  |                    |..|(J)_|
  |                          |___________|               |  SA119X/9612     |       _________    |..||
  |    ___    ___                                        |__________________|      |74HC273N |   |..||
  |   |   |  |   |                                                                 |_________|   |..||
  |   |   |  |   |                                                                               |..||
  |   | B |  | B |                                     _________                    _________    |..||
  |   |   |  |   |                                    | 74HC04N |                  |74HC245N |   |__||
  |   |   |  |   |                                    |_________|                  |_________|       |___
  |   |___|  |___|                                                                                   A___|
  |                                                                                                   ___|
  |    _____                  ______________                                                          ___|
  |   |     |                |              |                                        ________         ___|
  |   |     |                |  PHILIPS     |     ___    ___________    ________    |ULN2803A|        ___|
  |   |     | EI79465--A/02  |  SCC66470CAB |    | D |  |PIC16F84-10|  |   E    |   |________|        ___|
  |   |  C  | LPL-CPU V4.0   |  172632=1/2  |    |___|  |___________|  |________|                     ___|
  |   |     | MULTI GAME     |  DfD0032I3   |                                       _________         ___|
  |   |     | 8603186        |              |                                      |74HC273N |        ___|
  |   |     |                |              |                                      |_________|        ___|
  |   |     |                |              |                                                         ___|
  |   |_____|                |______________|                                 _     _________         ___|
  |                                                                          |G|   |74HC245N |        ___|
  |                                                                          |_|   |_________|        ___|
  |   _______   _______                                                                               ___|
  |  |]     [| |IC21   |                                 ______             ___                       ___|
  |  |]  E  [| |       |                                |ALTERA|           |___|                __    ___|
  |  |]  M  [| |       |                                | MAX  |             F      _________  |..|   ___|
  |  |]  P  [| | MAGIC |     ________________           |      |                   |74HC245N | |..|   ___|
  |  |]  T  [| | CARD  |    |                |          |EPM712|                   |_________| |..|   ___|
  |  |]  Y  [| |JACKPOT|    |   PHILIPS      |          |8SQC10|                               |..|   ___|
  |  |]     [| |       |    |  SCC68070CCA84 |          |0-15  |                               |..|   ___|
  |  |]  S  [| |Version|    |  213140-1      |          |______|      X_TAL's                  |__|   ___|
  |  |]  O  [| |   4.01|    |  DfD0103V3     |                      _   _   _                         ___|
  |  |]  C  [| |       |    |                |    ALL RIGHTS       | | | | | |                        ___|
  |  |]  K  [| |Vnr.:  |    |                |    BY  IMPERA       |1| |2| |3|                        ___|
  |  |]  E  [| |11.7.98|    |                |                     |_| |_| |_|                        ___|
  |  |]  T  [| |       |    |                |                                                        ___|
  |  |]     [| |       |    |________________|                                                        ___|
  |  |]_____[| |27C4002|                                                                              ___|
  |  |_______| |_______|                               ___________                                    ___|
  |    ___________________                            |RTC2421 A  |                                   ___|
  |   |   :::::::::::::   |                           |___________|                                  Z___|
  |   |___________________|                                                                          |
  |__________________________________________________________________________________________________|

  Xtal 1: 30.000 MHz.  (66470)
  Xtal 2:  8.000 MHz.  (PIC?)
  Xtal 3: 19.660 MHz.  (68070)

  A = LT 0030 / LTC695CN / U18708
  B = NEC Japan / D43256BGU-70LL / 0008XD041
  C = MX B9819 / 29F1610MC-12C3 / M25685 / TAIWAN
  D = 24C02C / 24C04 (Serial I2C Bus EEPROM with User-Defined Block Write Protection).
  E = P0030SG / CD40106BCN
  F = 74HCU04D
  G = 74HC74D

  Silkscreened on the solder side:

  LEOTS.
  2800
  AT&S-F0 ML 94V-0

  IMPERA AUSTRIA          -------
  TEL: 0043/7242/27116     V 4.0
  FAX: 0043/7242/27053    -------

*/
ROM_START( magicrdja )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "27c4002.ic21", 0x00000, 0x80000, CRC(ab2ed583) SHA1(a2d7148b785a8dfce8cff3b15ada293d65561c98) ) // sldh

	ROM_REGION16_LE( 0x4280, "pic16f84", 0 )  // decapped and dumped
	ROM_LOAD("magicardj_4.01_pic16f84_code.bin",   0x0000, 0x0800, CRC(c6502436) SHA1(85c4126251bd60ec1f4e28615ec7f948ef8c088f) )
	/*
	{
	"conf_word": 0,
	"secure": true,
	"user_id0": 16256,
	"user_id1": 16262,
	"user_id2": 16265,
	"user_id3": 16264
	}
	*/
	// ID locations:
	ROM_FILL( 0x4000, 0x01, 0x80 )
	ROM_FILL( 0x4001, 0x01, 0x3f )
	ROM_FILL( 0x4002, 0x01, 0x86 )
	ROM_FILL( 0x4003, 0x01, 0x3f )
	ROM_FILL( 0x4004, 0x01, 0x89 )
	ROM_FILL( 0x4005, 0x01, 0x3f )
	ROM_FILL( 0x4006, 0x01, 0x88 )
	ROM_FILL( 0x4007, 0x01, 0x3f )
	// configuration word: all 0
	ROM_FILL( 0x400e, 0x01, 0x00 )
	ROM_FILL( 0x400f, 0x01, 0x00 )
	ROM_LOAD("magicardj_4.01_pic16f84_data.bin",   0x4200, 0x0080, CRC(40961fef) SHA1(8617ef78d50842ea89d81d4db3728b3f799d7530) )

	ROM_REGION( 0x200000, "other", 0 )  // unknown contents
	ROM_LOAD("29f1610mc.ic30",  0x000000, 0x200000, NO_DUMP )

	ROM_REGION( 0x0100, "sereeprom", 0 )  // Serial EPROM
	ROM_LOAD("24c02c.ic26", 0x0000, 0x0100, CRC(b5c86862) SHA1(0debc0f7e7c506e5a4e2cae152548d80ad72fc2e) )
ROM_END

/*
  Magic Card Export 94
  International Ver. 2.11a (set 1)
  Vnr.29.07.94    CHECKSUM: A63D

  1x Philips SCC66470CAB 383610
  1x Philips SCC68070 CCA84 347141
  1x ESI1 I9631
  1x MUSIC TR9C1710-11PCA SA121X/9617
  1x YAMAHA YM2149F 9614

  XTAL:

  Q1: 19.6608 Mhz        (68070)
  Q2: 30.000 Mhz         (66470)
  Q3: 3686.400  1Q08/95  (PIC?)

*/
ROM_START( magicrde )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "27c4002_v2.11a_a63d.ic21", 0x00000, 0x80000, CRC(b5f24412) SHA1(73ff05c19132932a419fef0d5dc985440ce70e83) )

	ROM_REGION( 0x2000, "pic16c54", 0 )  // decapped
	ROM_LOAD("pic16c54.ic29",   0x0000, 0x1fff, CRC(9c225a49) SHA1(249c12d23d1a85de828652c55a1a19ef8ec378ef) )

	ROM_REGION( 0x0100, "sereeprom", 0 )  // Serial EPROM
	ROM_LOAD("st24c02.ic26",    0x0000, 0x0100, CRC(427bcdc7) SHA1(0b1debf6aa2a50717fcf85dfb8d98ba70871beb9) )

	ROM_REGION(0x8, "serial_id", 0)  // serial number
	ROM_LOAD( "ds2401", 0x000000, 0x000008, BAD_DUMP CRC(3f87b999) SHA1(29649749d521ced9dc7ef1d0d6ddb9a8beea360f) )  // created to match game

	ROM_REGION(0x4d, "ds1207", 0)  // timekey
	ROM_LOAD( "ds1207", 0x000000, 0x00004d, BAD_DUMP CRC(b00bf924) SHA1(ab98b2955697765518d877d4e19dbe45de0d9503) )  // created to match game

	ROM_REGION(0x10000, "nvram", 0)  // Default NVRAM
	ROM_LOAD( "magicrde.nv", 0x0000, 0x10000, CRC(6b9f6abd) SHA1(fd171f465a16d3f2da9c19924ee31f6e56ee746c) )
ROM_END

/*
  Magic Card Export 94
  International Ver. 2.11a (set 2)
  Vnr.29.07.94    CHECKSUM: 9505

  1x Philips SCC66470CAB 383610
  1x Philips SCC68070 CCA84 347141
  1x ESI1 I9631
  1x MUSIC TR9C1710-11PCA SA121X/9617
  1x YAMAHA YM2149F 9614

  XTAL:

  Q1: 19.6608 Mhz         (68070)
  Q2: 30.000 Mhz          (66470)
  Q3: 3686.400  1Q08/95   (PIC?)

*/
ROM_START( magicrdea )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "27c4002_v2.11a_9505.ic21", 0x00000, 0x80000, CRC(24c69c01) SHA1(0928800b9cfc2ae358f90b3f79c08acd2b2aa7d8) )

	ROM_REGION( 0x2000, "pic16c54", 0 )  // decapped
	ROM_LOAD("pic16c54.ic29",   0x0000, 0x1fff, CRC(9c225a49) SHA1(249c12d23d1a85de828652c55a1a19ef8ec378ef) )

	ROM_REGION( 0x0100, "sereeprom", 0 )  // Serial EPROM
	ROM_LOAD("st24c02.ic26",    0x0000, 0x0100, CRC(427bcdc7) SHA1(0b1debf6aa2a50717fcf85dfb8d98ba70871beb9) )

	ROM_REGION(0x8, "serial_id", 0)  // serial number
	ROM_LOAD( "ds2401", 0x000000, 0x000008, BAD_DUMP CRC(3f87b999) SHA1(29649749d521ced9dc7ef1d0d6ddb9a8beea360f) )  // created to match game

	ROM_REGION(0x4d, "ds1207", 0)  // timekey
	ROM_LOAD( "ds1207", 0x000000, 0x00004d, BAD_DUMP CRC(b00bf924) SHA1(ab98b2955697765518d877d4e19dbe45de0d9503) )  // created to match game

	ROM_REGION(0x10000, "nvram", 0)  // Default NVRAM
	ROM_LOAD( "magicrdea.nv", 0x0000, 0x10000, CRC(a1043c84) SHA1(30c3bb43e91fc358a2592f9a6efbd146eec4e43c) )
ROM_END

/*
  Magic Card Export 94
  Clubversion Export v2.09a
  Vnr.02.08.94    CHECKSUM: 5B64

  1x Philips SCC66470CAB 383610
  1x Philips SCC68070 CCA84 347141

  Other components are unreadable
  in the PCB picture.

*/
ROM_START( magicrdec )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "27c4002_v2.9a_5b64.ic21", 0x00000, 0x80000, CRC(81ad0437) SHA1(117e2681541f786874cd0bce7f8bfb2bffb0b548) )

	// Serial EPROM undumped
	ROM_REGION( 0x0100, "sereeprom", 0 )  // Serial EPROM
	ROM_LOAD("st24c02.ic26",    0x0000, 0x0100, BAD_DUMP CRC(6c695dd7) SHA1(7fb56c449f7592e200204f38a4cbb4cf7f0f1665) )

	ROM_REGION(0x8, "serial_id", 0) /* serial number */
	ROM_LOAD( "ds2401", 0x000000, 0x000008, BAD_DUMP CRC(3f87b999) SHA1(29649749d521ced9dc7ef1d0d6ddb9a8beea360f) ) // created to match game

	ROM_REGION(0x4d, "ds1207", 0) /* timekey */
	ROM_LOAD( "ds1207", 0x000000, 0x00004d, BAD_DUMP CRC(b00bf924) SHA1(ab98b2955697765518d877d4e19dbe45de0d9503) ) // created to match game

	// PIC undumped

	ROM_REGION(0x10000, "nvram", 0) /* Default NVRAM */
	ROM_LOAD( "magicrdec.nv", 0x0000, 0x10000, CRC(f40bf542) SHA1(b73838f610dbf35099971d80e240abd672dd36e3) )
ROM_END

/*
  Magic Card Export
  Version 4.01
  Vnr.07.03.98    CHECKSUM: AF18

  1x Philips SCC66470CAB 383610
  1x Philips SCC68070 CCA84 347141

  Other components are unreadable
  in the PCB picture.

*/
ROM_START( magicrdj )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "27c4002_v4.01_af18.ic21", 0x00000, 0x80000, CRC(7700fd22) SHA1(0555c08c82f56e6399a89f6408e52d9d0beba2ac) )


	ROM_REGION(0x8, "serial_id", 0)  // serial number
	ROM_LOAD( "ds2401", 0x000000, 0x000008, BAD_DUMP CRC(3f87b999) SHA1(29649749d521ced9dc7ef1d0d6ddb9a8beea360f) )  // created to match game
	// PIC undumped
	// Serial EPROM undumped
ROM_END

/*
  Magic Card Export 94
  International Ver. 2.11a (set 3)

  1x Philips SCC66470CAB.
  1x Philips SCC68070 CCA84.
  1x MUSIC TR9C1710-11PCA.
  1x YAMAHA YMZ284-D.

  1x M27C4002 EPROM (dumped).
  1x 29F1610MC-12 Flash EEPROM (dumped).
  1x 24LC02 Serial EEPROM (dumped).

  1x Altera MAX EPM712xxxxx (unreadable).

  XTAL: 3x unknown frequency.

*/
ROM_START( magicrdeb )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "27c4002.ic21", 0x00000, 0x80000, CRC(098258c0) SHA1(5f5dfe376c980ec88e68b324ba912022091e2426) )

	ROM_REGION( 0x200000, "other", 0 )  // Flash EEPROM
	ROM_LOAD("mx29f1610.ic30",  0x000000, 0x200000, CRC(c8ba9820) SHA1(fcae1e200c718b549b91d1110025595ffd7bdd51) )

	ROM_REGION( 0x0100, "sereeprom", 0 ) // Serial EEPROM
	ROM_LOAD("24lc02b.ic26",    0x0000, 0x0100, CRC(5cb1b2b2) SHA1(84d4535e5491d9a4a9c658d39df16757bc572a4b) )

	ROM_REGION(0x8, "serial_id", 0)  // serial number
	ROM_LOAD( "ds2401", 0x000000, 0x000008, BAD_DUMP CRC(3f87b999) SHA1(29649749d521ced9dc7ef1d0d6ddb9a8beea360f) )  // created to match game

	ROM_REGION(0x10000, "nvram", 0)  // Default NVRAM
	ROM_LOAD( "magicrdeb.nv", 0x0000, 0x10000, CRC(8beb061b) SHA1(c29a2086dea30c98565e811d9686af35da42c9d9) )
ROM_END

/*
  Magic Card - Wien v1.2 200/93 set 1
  Sicherheitsversion 1.2

*/
ROM_START( magicrd1c )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "am27c4096.bin", 0x00000, 0x80000, CRC(d9e2a4ec) SHA1(b3000ded242fa25709c90b9b2541c9d1d5cabebb) )

	ROM_REGION( 0x1fff, "pic16c54", 0 )  // decapped
	ROM_LOAD("pic16c54a.bin",   0x0000, 0x1fff, CRC(e777e814) SHA1(e0440be76fa1f3c7ae7d31e1b29a2ba73552231c) )

	ROM_REGION(0x4d, "ds1207", 0)  // timekey
	ROM_LOAD( "ds1207", 0x000000, 0x00004d, BAD_DUMP CRC(ab0b75a2) SHA1(3a3c594d77936e671d25f526459355cc446a0991) )   // created to match game

	ROM_REGION(0x4000, "nvram", 0)  // Default NVRAM
	ROM_LOAD( "magicrd1c.nv", 0x0000, 0x4000, CRC(d6244455) SHA1(b6389574f1d4a4a64590d544c9bafe4892feb0a1) )

	ROM_REGION( 0x0100, "sereeprom", 0 ) // Serial EEPROM
	ROM_LOAD("24lc02b.ic26",    0x0000, 0x0100, CRC(fea8a821) SHA1(c744cac6af7621524fc3a2b0a9a135a32b33c81b) )
ROM_END


ROM_START( magicle )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "27c4002.ic21", 0x00000, 0x80000, CRC(73328346) SHA1(fca5f8a93f25377e659c2b291674d706ca37400e) )

	ROM_REGION16_LE( 0x4280, "pic16f84", 0 )  // decapped and dumped
	ROM_LOAD("magicle_5.03_pic16f84_code.bin",   0x0000, 0x0800, CRC(22965864) SHA1(c421a9e9fac7c9c5dc01adda620dc8f5f16d94ba) )
	/*
{
	"conf_word": 0,
	"secure": true,
	"user_id0": 16256,
	"user_id1": 16263,
	"user_id2": 16265,
	"user_id3": 16265
}
	*/
	// ID locations:
	ROM_FILL( 0x4000, 0x01, 0x80 )
	ROM_FILL( 0x4001, 0x01, 0x3f )
	ROM_FILL( 0x4002, 0x01, 0x87 )
	ROM_FILL( 0x4003, 0x01, 0x3f )
	ROM_FILL( 0x4004, 0x01, 0x89 )
	ROM_FILL( 0x4005, 0x01, 0x3f )
	ROM_FILL( 0x4006, 0x01, 0x89 )
	ROM_FILL( 0x4007, 0x01, 0x3f )
	// configuration word: all 0
	ROM_FILL( 0x400e, 0x01, 0x00 )
	ROM_FILL( 0x400f, 0x01, 0x00 )
	ROM_LOAD("magicle_5.03_pic16f84_data.bin",   0x4200, 0x0080, CRC(b3cdf90f) SHA1(0afec6f78320e5fe653073769cdeb32918da061b) )

	ROM_REGION( 0x200000, "other", 0 )  // unknown contents
	ROM_LOAD("29f1610mc.ic30",  0x000000, 0x200000, NO_DUMP )

	ROM_REGION( 0x0200, "sereeprom", 0 )  // Serial EPROM
	ROM_LOAD("24c04a.ic26", 0x0000, 0x0200, CRC(48c4f473) SHA1(5355313cc96f655096e13bfae78be3ba2dfe8a2d) )
ROM_END

/*
  Hot Slots Version 6.00

  Hardware PCB informations:
  E179465--A/02 LPL-CPU V4.0/MULTI GAME

  Eprom type AM27C4096
  Version 6.00
  vnr 15.04.02 Cksum (provided) 0D08

*/
ROM_START( hotslots )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "hot_slots_v600_15.04.02.bin", 0x00000, 0x80000, CRC(35677999) SHA1(7462eef3734b9b6087102901967a168a60ab7710) )

	ROM_REGION( 0x0100, "sereeprom", 0 )  // Serial EPROM
	ROM_LOAD16_WORD_SWAP("hot_slots_24c02.bin",          0x0000,  0x0100,  CRC(fcac71ad) SHA1(1bb31e9a2d847430dc0d011f672cf3726dc6280c) )
ROM_END

/*
  QUINGO EXPORT Version 5.00

  Hardware PCB informations : E179465--A/02 LPL-CPU V4.0/MULTI GAME 8603186

  Eprom type ST M27c4002
  Version 5.00
  vnr 27.07.99 Cksum (provided) 79C5

  Eeprom : 24c04A

  PCB Layout:
   __________________________________________________________________________________________________
  |                                                                                                  |
  |                       SERIAL NUMBER                                                              |___
  |                                                                                                  (A)_|
  |  __     _____             ___________        _________                                            ___|
  | |  |   /     \           |9524 GNN   |      |YMZ284-D |                                           ___|
  | |  |  |BATTERY|          |HM514270AJ8|      |_________|                                           ___|
  | |A |  |  +3V  |          |___________|                                                            ___|
  | |  |  |       |                                       __________________         ________     __  ___|
  | |  |   \_____/            ___________                |                  |       |ULN2803A|   |..| ___|
  | |__|                     |9524 GNN   |               |  MUSIC           |       |________|   |..| ___|
  |                          |HM514270AJ8|               |  TR9C1710-11PCA  |                    |..|(J)_|
  |                          |___________|               |  SA119X/9612     |       _________    |..||
  |    ___    ___                                        |__________________|      |74HC273N |   |..||
  |   |   |  |   |                                                                 |_________|   |..||
  |   |   |  |   |                                                                               |..||
  |   | B |  | B |                                     _________                    _________    |..||
  |   |   |  |   |                                    | 74HC04N |                  |74HC245N |   |__||
  |   |   |  |   |                                    |_________|                  |_________|       |___
  |   |___|  |___|                                                                                   A___|
  |                                                                                                   ___|
  |    _____                  ______________                                                          ___|
  |   |     |                |              |                                        ________         ___|
  |   |     |                |  PHILIPS     |     ___    ___________    ________    |ULN2803A|        ___|
  |   |     | EI79465--A/02  |  SCC66470CAB |    | D |  |PIC16F84-10|  |   E    |   |________|        ___|
  |   |  C  | LPL-CPU V4.0   |  172632=1/2  |    |___|  |___________|  |________|                     ___|
  |   |     | MULTI GAME     |  DfD0032I3   |                                       _________         ___|
  |   |     | 8603186        |              |                                      |74HC273N |        ___|
  |   |     |                |              |                                      |_________|        ___|
  |   |     |                |              |                                                         ___|
  |   |_____|                |______________|                                 _     _________         ___|
  |                                                                          |G|   |74HC245N |        ___|
  |                                                                          |_|   |_________|        ___|
  |   _______   _______                                                                               ___|
  |  |]     [| |IC21   |                                 ______             ___                       ___|
  |  |]  E  [| |       |                                |ALTERA|           |___|                __    ___|
  |  |]  M  [| |       |                                | MAX  |             F      _________  |..|   ___|
  |  |]  P  [| |QUINGO |     ________________           |      |                   |74HC245N | |..|   ___|
  |  |]  T  [| | EXPORT|    |                |          |EPM712|                   |_________| |..|   ___|
  |  |]  Y  [| |       |    |   PHILIPS      |          |8SQC10|                               |..|   ___|
  |  |]     [| |       |    |  SCC68070CCA84 |          |0-15  |                               |..|   ___|
  |  |]  S  [| |Version|    |  213140-1      |          |______|      X_TAL's                  |__|   ___|
  |  |]  O  [| |   5.00|    |  DfD0103V3     |                      _   _   _                         ___|
  |  |]  C  [| |       |    |                |    ALL RIGHTS       | | | | | |                        ___|
  |  |]  K  [| |Vnr.:  |    |                |    BY  IMPERA       |1| |2| |3|                        ___|
  |  |]  E  [| |270799 |    |                |                     |_| |_| |_|                        ___|
  |  |]  T  [| |       |    |                |                                                        ___|
  |  |]     [| |       |    |________________|                                                        ___|
  |  |]_____[| |27C4002|                                                                              ___|
  |  |_______| |_______|                             ___________                                      ___|
  |    ___________________                          |RTC2421 A  |   °°°°°                             ___|
  |   |   :::::::::::::   |                         |___________|    CON5                            Z___|
  |   |___________________|                                                                          |
  |__________________________________________________________________________________________________|

  Xtal 1: 30.000 MHz.  (66470)
  Xtal 2:  8.000 MHz.  (PIC?)
  Xtal 3: 19.660 MHz.  (68070)

  A = LT 0030 / LTC695CN / U18708
  B = NEC Japan / D43256BGU-70LL / 0008XD041
  C = MX B9819 / 29F1610MC-12C3 / M25685 / TAIWAN
  D = 24C02C / 24C04 (Serial I2C Bus EEPROM with User-Defined Block Write Protection).
  E = P0030SG / CD40106BCN
  F = 74HCU04D
  G = 74HC74D

*/
ROM_START( quingo )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "quingo_export_v500_27.07.99.bin", 0x00000, 0x80000, CRC(2cd89fe3) SHA1(bdd256d5114227166aff1c9f84b573e5f00530fd) )

	ROM_REGION( 0x0200, "sereeprom", 0 )  // Serial EPROM
	ROM_LOAD16_WORD_SWAP("quingo_24c04a.bin", 0x0000, 0x0200, BAD_DUMP CRC(d5e82b49) SHA1(7dbdf7d539cbd59a3ac546b6f50861c4958abb3a) )  // all AA & 55
	
	ROM_REGION16_LE( 0x4280, "pic16f84", 0 )  // borrowed from magicle to avoid I2C bus error
	ROM_LOAD("magicle_5.03_pic16f84_code.bin",   0x0000, 0x0800, BAD_DUMP CRC(22965864) SHA1(c421a9e9fac7c9c5dc01adda620dc8f5f16d94ba) )
ROM_END

/*
  BIG DEAL BELGIEN Version 5.04

  Hardware PCB informations : E179465--A/02 LPL-CPU V4.0/MULTI GAME 8603186
  Eprom type ST M27c4002
  Version 5.04
  vnr 21.05.01 Cksum (provided) C4B7

  Eeprom : 24c04A

  PCB Layout:
   __________________________________________________________________________________________________
  |                                                                                                  |
  |                       SERIAL NUMBER                                                              |___
  |                                                                                                  (A)_|
  |  __     _____             ___________        _________                                            ___|
  | |  |   /     \           |9524 GNN   |      |YMZ284-D |                                           ___|
  | |  |  |BATTERY|          |HM514270AJ8|      |_________|                                           ___|
  | |A |  |  +3V  |          |___________|                                                            ___|
  | |  |  |       |                                       __________________         ________     __  ___|
  | |  |   \_____/            ___________                |                  |       |ULN2803A|   |..| ___|
  | |__|                     |9524 GNN   |               |  MUSIC           |       |________|   |..| ___|
  |                          |HM514270AJ8|               |  TR9C1710-11PCA  |                    |..|(J)_|
  |                          |___________|               |  SA119X/9612     |       _________    |..||
  |    ___    ___                                        |__________________|      |74HC273N |   |..||
  |   |   |  |   |                                                                 |_________|   |..||
  |   |   |  |   |                                                                               |..||
  |   | B |  | B |                                     _________                    _________    |..||
  |   |   |  |   |                                    | 74HC04N |                  |74HC245N |   |__||
  |   |   |  |   |                                    |_________|                  |_________|       |___
  |   |___|  |___|                                                                                   A___|
  |                                                                                                   ___|
  |    _____                  ______________                                                          ___|
  |   |     |                |              |                                        ________         ___|
  |   |     |                |  PHILIPS     |     ___    ___________    ________    |ULN2803A|        ___|
  |   |     | EI79465--A/02  |  SCC66470CAB |    | D |  |PIC16F84-10|  |   E    |   |________|        ___|
  |   |  C  | LPL-CPU V4.0   |  172632=1/2  |    |___|  |___________|  |________|                     ___|
  |   |     | MULTI GAME     |  DfD0032I3   |                                       _________         ___|
  |   |     | 8603186        |              |                                      |74HC273N |        ___|
  |   |     |                |              |                                      |_________|        ___|
  |   |     |                |              |                                                         ___|
  |   |_____|                |______________|                                 _     _________         ___|
  |                                                                          |G|   |74HC245N |        ___|
  |                                                                          |_|   |_________|        ___|
  |   _______   _______                                                                               ___|
  |  |]     [| |IC21   |                                 ______             ___                       ___|
  |  |]  E  [| |       |                                |ALTERA|           |___|                __    ___|
  |  |]  M  [| |  BIG  |                                | MAX  |             F      _________  |..|   ___|
  |  |]  P  [| |  DEAL |     ________________           |      |                   |74HC245N | |..|   ___|
  |  |]  T  [| |BELGIEN|    |                |          |EPM712|                   |_________| |..|   ___|
  |  |]  Y  [| |       |    |   PHILIPS      |          |8SQC10|                               |..|   ___|
  |  |]     [| |       |    |  SCC68070CCA84 |          |0-15  |                               |..|   ___|
  |  |]  S  [| |Version|    |  213140-1      |          |______|      X_TAL's                  |__|   ___|
  |  |]  O  [| |   5.04|    |  DfD0103V3     |                      _   _   _                         ___|
  |  |]  C  [| |       |    |                |    ALL RIGHTS       | | | | | |                        ___|
  |  |]  K  [| |Vnr.:  |    |                |    BY  IMPERA       |1| |2| |3|                        ___|
  |  |]  E  [| |210501 |    |                |                     |_| |_| |_|                        ___|
  |  |]  T  [| |       |    |                |                                                        ___|
  |  |]     [| |       |    |________________|                                                        ___|
  |  |]_____[| |27C4002|                                                                              ___|
  |  |_______| |_______|                             ___________                                      ___|
  |    ___________________                          |RTC2421 A  |   °°°°°                             ___|
  |   |   :::::::::::::   |                         |___________|    CON5                            Z___|
  |   |___________________|                                                                          |
  |__________________________________________________________________________________________________|

  Xtal 1: 30.000 MHz. (66470)
  Xtal 2:  8.000 MHz. (PIC?)
  Xtal 3: 19.660 MHz. (68070)

  A = LT 0030 / LTC695CN / U18708
  B = NEC Japan / D43256BGU-70LL / 0008XD041
  C = MX B9819 / 29F1610MC-12C3 / M25685 / TAIWAN
  D = 24C02C / 24C04 (Serial I2C Bus EEPROM with User-Defined Block Write Protection).
  E = P0030SG / CD40106BCN
  F = 74HCU04D
  G = 74HC74D

*/
ROM_START( bigdeal0 )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "big_deal_belgien_v504_21.05.01.bin", 0x00000, 0x80000, CRC(3e3484db) SHA1(78bb655deacc57ad041a46de7ef153ce25922a8a) )

	ROM_REGION( 0x0200, "sereeprom", 0 )  // Serial EPROM
	ROM_LOAD16_WORD_SWAP("big_deal_24c04a.bin", 0x0000, 0x0200, BAD_DUMP CRC(d5e82b49) SHA1(7dbdf7d539cbd59a3ac546b6f50861c4958abb3a) )  // all AA & 55
	
	ROM_REGION16_LE( 0x4280, "pic16f84", 0 )  // borrowed from magicle to avoid I2C bus error
	ROM_LOAD("magicle_5.03_pic16f84_code.bin",   0x0000, 0x0800, BAD_DUMP CRC(22965864) SHA1(c421a9e9fac7c9c5dc01adda620dc8f5f16d94ba) )
ROM_END

/*
  BEL SLOTS EXP. Version 5.01

  Hardware PCB informations : E179465--A/02 LPL-CPU V4.0/MULTI GAME 8603186
  Eprom type ST M27c4002
  Version 5.01
  vnr 01.12.99 Cksum (provided) F718

  Eeprom : 24c04A

  PCB Layout:
   __________________________________________________________________________________________________
  |                                                                                                  |
  |                       SERIAL NUMBER                                                              |___
  |                                                                                                  (A)_|
  |  __     _____             ___________        _________                                            ___|
  | |  |   /     \           |9524 GNN   |      |YMZ284-D |                                           ___|
  | |  |  |BATTERY|          |HM514270AJ8|      |_________|                                           ___|
  | |A |  |  +3V  |          |___________|                                                            ___|
  | |  |  |       |                                       __________________         ________     __  ___|
  | |  |   \_____/            ___________                |                  |       |ULN2803A|   |..| ___|
  | |__|                     |9524 GNN   |               |  MUSIC           |       |________|   |..| ___|
  |                          |HM514270AJ8|               |  TR9C1710-11PCA  |                    |..|(J)_|
  |                          |___________|               |  SA119X/9612     |       _________    |..||
  |    ___    ___                                        |__________________|      |74HC273N |   |..||
  |   |   |  |   |                                                                 |_________|   |..||
  |   |   |  |   |                                                                               |..||
  |   | B |  | B |                                     _________                    _________    |..||
  |   |   |  |   |                                    | 74HC04N |                  |74HC245N |   |__||
  |   |   |  |   |                                    |_________|                  |_________|       |___
  |   |___|  |___|                                                                                   A___|
  |                                                                                                   ___|
  |    _____                  ______________                                                          ___|
  |   |     |                |              |                                        ________         ___|
  |   |     |                |  PHILIPS     |     ___    ___________    ________    |ULN2803A|        ___|
  |   |     | EI79465--A/02  |  SCC66470CAB |    | D |  |PIC16F84-10|  |   E    |   |________|        ___|
  |   |  C  | LPL-CPU V4.0   |  172632=1/2  |    |___|  |___________|  |________|                     ___|
  |   |     | MULTI GAME     |  DfD0032I3   |                                       _________         ___|
  |   |     | 8603186        |              |                                      |74HC273N |        ___|
  |   |     |                |              |                                      |_________|        ___|
  |   |     |                |              |                                                         ___|
  |   |_____|                |______________|                                 _     _________         ___|
  |                                                                          |G|   |74HC245N |        ___|
  |                                                                          |_|   |_________|        ___|
  |   _______   _______                                                                               ___|
  |  |]     [| |IC21   |                                 ______             ___                       ___|
  |  |]  E  [| |       |                                |ALTERA|           |___|                __    ___|
  |  |]  M  [| |  BEL  |                                | MAX  |             F      _________  |..|   ___|
  |  |]  P  [| | SLOTS |     ________________           |      |                   |74HC245N | |..|   ___|
  |  |]  T  [| |  EXP. |    |                |          |EPM712|                   |_________| |..|   ___|
  |  |]  Y  [| |       |    |   PHILIPS      |          |8SQC10|                               |..|   ___|
  |  |]     [| |       |    |  SCC68070CCA84 |          |0-15  |                               |..|   ___|
  |  |]  S  [| |Version|    |  213140-1      |          |______|      X_TAL's                  |__|   ___|
  |  |]  O  [| |   5.01|    |  DfD0103V3     |                      _   _   _                         ___|
  |  |]  C  [| |       |    |                |    ALL RIGHTS       | | | | | |                        ___|
  |  |]  K  [| |Vnr.:  |    |                |    BY  IMPERA       |1| |2| |3|                        ___|
  |  |]  E  [| |011299 |    |                |                     |_| |_| |_|                        ___|
  |  |]  T  [| |       |    |                |                                                        ___|
  |  |]     [| |       |    |________________|                                                        ___|
  |  |]_____[| |27C4002|                                                                              ___|
  |  |_______| |_______|                             ___________                                      ___|
  |    ___________________                          |RTC2421 A  |   °°°°°                             ___|
  |   |   :::::::::::::   |                         |___________|    CON5                            Z___|
  |   |___________________|                                                                          |
  |__________________________________________________________________________________________________|

  Xtal 1: 30.000 MHz.  (66470)
  Xtal 2:  8.000 MHz.  (PIC?)
  Xtal 3: 19.660 MHz.  (68070)

  A = LT 0030 / LTC695CN / U18708
  B = NEC Japan / D43256BGU-70LL / 0008XD041
  C = MX B9819 / 29F1610MC-12C3 / M25685 / TAIWAN
  D = 24C02C / 24C04 (Serial I2C Bus EEPROM with User-Defined Block Write Protection).
  E = P0030SG / CD40106BCN
  F = 74HCU04D
  G = 74HC74D

*/
ROM_START( belslots )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "bel_slots_exp_v501_01.12.99.bin", 0x00000, 0x80000, CRC(bd0b97ff) SHA1(9431359f91fd059c61441f4cb4924500889552a9) )

	ROM_REGION( 0x0200, "sereeprom", 0 )  // Serial EPROM
	ROM_LOAD16_WORD_SWAP("bel_slots_exp_24c04a.bin", 0x0000, 0x0200, BAD_DUMP CRC(d5e82b49) SHA1(7dbdf7d539cbd59a3ac546b6f50861c4958abb3a) )  // all AA & 55
	
	ROM_REGION16_LE( 0x4280, "pic16f84", 0 )  // borrowed from magicle to avoid I2C bus error
	ROM_LOAD("magicle_5.03_pic16f84_code.bin",   0x0000, 0x0800, BAD_DUMP CRC(22965864) SHA1(c421a9e9fac7c9c5dc01adda620dc8f5f16d94ba) )
ROM_END

/*
  Puzzle Me!
  Impera.

  PCB layout:
   ___________________________________________________________________________________________________________________________
  |                      ___    ___    ___    ___                                                                             |
  |    ___              |   |  |   |  |   |  |   |                                                                            |
  |   | B |             | A |  | A |  | A |  | A |    ______________________________________                                  |____
  |   |___|    _____    |   |  |   |  |   |  |   |   |                                      |                                   ___|
  |           |  _  |   |___|  |___|  |___|  |___|   |               YAMAHA                 |                                   ___|
  |  _______  |BATTE|                                |               YM2149F                |                                   ___|
  | |LTC695C| |RY   |    ___    ___    ___    ___    |                                  IC17|                                   ___|
  | |_______| |  +  |   | E |  | E |  | E |  | E |   |______________________________________|                                   ___|
  |           |_____|   | M |  | M |  | M |  | M |                                                                              ___|
  |                     | P |  | P |  | P |  | P |           ___________________________           ______________      _____    ___|
  |   ____    ____      | T |  | T |  | T |  | T |          |                           |         |   ULN2803A   |    | O O |   ___|
  |  |HY62|  |HY62|     | Y |  | Y |  | Y |  | Y |          |       KDA0476CN_66        |         |______________|    | O O |  ____|
  |  |64AL|  |64AL|     |___|  |___|  |___|  |___|          |       KOREA    219    IC20|        ________________     | O O | |
  |  |J_10|  |J_10|                                         |___________________________|       |    74HC273N    |    | O O | |
  |  |    |  |    |                                                                             |________________|    | O O | |
  |  |____|  |____|                                        ___________           ____________    ________________     | O O | |
  |                                                       | 74HC04AP  |         |EMPTY SOCKET|  |    74HC245N    |    | O O | |
  |  _______   _______                                    |___________|         |____________|  |________________|    | O O | |
  | |       | |       |   XTAL1                                                                                       |_____| |____
  | |       | |       |    _________________                 XTAL3                                 ______________        CON3   ___|
  | | EMPTY | | EMPTY |   |    IMPERA 8     |        _____    _____________     _____________     |  TD62083AP   |              ___|
  | | SOCKET| | SOCKET|   |     209751      |       |24C02|  |  PIC16C54   |   | HCF40106BE  |    |______________|              ___|
  | |       | |       |   |   DfD9227I3 Y   |       |_____|  |_____________|   |_____________|   ________________               ___|
  | |       | |       |   |                 |          IC26             IC29                    |    74HC273N    |              ___|
  | |       | |       |   |  SCC 66470 CAB  |                                                   |________________|              ___|
  | |       | |       |   |      317360     |                                                                                   ___|
  | |       | |       |   |   DfD9501I3 Y   |                                                    ________________               ___|
  | |  IC22A| |  IC21A|   |_________________|         __________________                        |    74HC245N    |              ___|
  | |_______| |_______|                  IC19        |                  |                       |________________|              ___|
  |  _______   _______                               |                  |                                                       ___|
  | |       | |       |                              |       ESI 1      |                                                       ___|
  | |       | |       |   XTAL2                      |       I9349      |                        ________________               ___|
  | |       | |       |    _________________         |                  |                       |    74HC245N    |              ___|
  | | EMPTY | |       |   |    IMPERA 7     |        |                  |                       |________________|              ___|
  | | SOCKET| |27C4002|   |     204440      |        |              IC25|                                                       ___|
  | |       | |       |   |   DfD9231V3 Y   |        |__________________|                                                       ___|
  | |       | |       |   |                 |                                                                                   ___|
  | |       | |       |   | SCC 68070 CCA84 |                                                                                   ___|
  | |       | |       |   |     324320      |                                                                                   ___|
  | |       | |       |   |   DfD9501V3 Y   |                                                                                   ___|
  | |       | |       |   |_________________|                                                                                   ___|
  | |   IC21| |   IC21|                  IC1               ______________    _________                                         ____|
  | |_______| |_______|                                   |  RTC 72421A  |  | DS1207  |                                       |
  | IMPERA BOARD REV V2.1                                 |______________|  |_________|                                       |
  |___________________________________________________________________________________________________________________________|

  XTAL1 = 30.000    (66470)
  XTAL2 = 19.6608   (68070)
  XTAL3 = 3686.400  (PIC?)

  A = KM44C256CJ_6
  B = TL7705ACP

*/
ROM_START( puzzleme )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "27c4002.ic21", 0x00000, 0x80000, CRC(cd3bc5a9) SHA1(682f62eba454f4f00212b2a8dabb05d6747f22fd) )

	ROM_REGION( 0x1fff, "pic16c54", 0 )  // decapped
	ROM_LOAD("pic16c54.ic29",   0x0000, 0x1fff, CRC(6dd2bd8e) SHA1(380f6b952ddd3183e9ab5404866c30be015b3773) )

	ROM_REGION( 0x0100, "sereeprom", 0 )  // Serial EPROM
	ROM_LOAD("x24c02p.ic26",    0x0000, 0x0100, CRC(bc940f53) SHA1(6b870019752ba5c446a5ad5155e4a81dfbf6e523) )
ROM_END


/*
  magicrd1
  Version 1.1 14.09.94

  PCB layout:
   ________________________________________________________________________________________________________________
  |                                                                                                                |
  |      __________                          _____________               ___________        ___                    |
  |     |  74LS04  |                        |LC324256BP-70|             |     C     |      |   |     ___           |__
  |     |__________|                        |_____________|             |___________|      |EMP|    |. .|           __|
  |                                          _____________                                 |TY |    |. .|           __|
  |            ____                         |LC324256BP-70|                                |   |    |. .|           __|
  |           | A  |                        |_____________|                                |SOC|    |. .|           __|
  |           |____|                         _____________                                 |KET|    |. .|           __|
  |                                         |LC324256BP-70|                                |___|    |. .|           __|
  |                                         |_____________|           __________________            |. .|           __|
  |                                          _____________           |   ADV476KN35E    |           |. .|           __|
  |                                         |LC324256BP-70|          |                  |           |___|           __|
  | _______                                 |_____________|          |     OF19802.3    |                          |
  ||DS1207 |                                                         |__________________|                          |
  ||_______|                                                                                                       |
  |         ___                       XTAL2                              __________                                |
  |        |   |        ________          ________________              | PIC16C54 |                               |
  |   ___  |PC7|       |        |        |                |             |__________|                               |__
  |  |   | |4HC|       |        |        |    IMPERA 8    |                   XTAL3                                 __|
  |  |HEF| |273|       |HYUNDAI |        |                |                                                         __|
  |  |400| |P  |       |        |        |                |                                                         __|
  |  |98B| |   |       |HY6264AL|        |     209751     |                                                         __|
  |  |P  | |   |       |P_10    |        |                |                                                         __|
  |  |   | |   |       |        |        |  DfD0922713 Y  |                                                         __|
  |  |___| |___|       |        |        |                |                                ___   ___                __|
  |                    | 9218A  |        |________________|              _____________    |   | |   |               __|
  |   _______          |        |                                       |  74HC245N   |   |PC7| |ULN|               __|
  |  |       |         | KOREA  |                                       |_____________|   |4HC| |280|               __|
  |  |BATTERY|         |________|                                                         |273| |3A |               __|
  |  |       |                                                                            |P  | |   |               __|
  |  |_______|     ________   ________                                                    |   | |   |               __|
  |               |        | |        |                                                   |   | |   |               __|
  |               |        | |        |                                  _____________    |___| |___|               __|
  |   ___         |        | |        |     XTAL1                       |  74HC245N   |    ___   ___                __|
  |  | B |        |        | |        |    __________________           |_____________|   |   | |   |               __|
  |  |___|        |        | |        |   |                  |                            |PC7| |ULN|               __|
  |               | EMPTY  | |        |   |    IMPERA 7      |                            |4HC| |280|               __|
  |               | SOCKET | |27C4002 |   |                  |                            |273| |3A |               __|
  |               |        | |        |   |     230031       |           _____________    |P  | |   |               __|
  |   ___   ___   |        | |        |   |                  |          |  74HC245N   |   |   | |   |               __|
  |  |   | |   |  |        | |        |   |   DfD9249V3 Y    |          |_____________|   |   | |   |               __|
  |  |PAL| |PAL|  |        | |        |   |                  |                            |___| |___|               __|
  |  |CE | |CE |  |        | |        |   |                  |                             __________               __|
  |  |   | |   |  |        | |        |   |                  |                            | CNY 74-4 |              __|
  |  |   | |   |  |        | |        |   |__________________|           _____________    |__________|              __|
  |  |   | |   |  |        | |        |                                 |  74HC245N   |                             __|
  |  |   | |   |  |        | |        |                                 |_____________|                             __|
  |  |___| |___|  |________| |________|                                                    ___________              __|
  |                                                                                       |   DIP 1   |            |
  | IMPERA BOARD REV V1.05                                                                |___________|            |
  |________________________________________________________________________________________________________________|

  A = TL7705ACP
  B = DS1210
  C = Cover scratched - unreadable

  XTAL1 = 19.6608 (68070)
  XTAL2 = 30.000  (66470)
  XTAL3 = 3.686JB (PIC?)

*/
ROM_START( magicrd1 )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "m27c4002.bin", 0x00000, 0x80000, CRC(229a504f) SHA1(8033e9b4cb55f2364bf4606375ef9ac05fc715fe) )

	ROM_REGION( 0x1fff, "pic16c56", 0 )  // decapped
	ROM_LOAD("pic16c56.bin",   0x0000, 0x1fff, CRC(b5655603) SHA1(d9126c36f3fca7e769ea60aaa711bb304b4b6a11) )

	ROM_REGION(0x4d, "ds1207", 0)  // timekey
	ROM_LOAD( "ds1207", 0x000000, 0x00004d, BAD_DUMP CRC(cbcc1a42) SHA1(4b577c85f5856192ce04051a2d305a9080192177) )  // created to match game

	ROM_REGION(0x4000, "nvram", 0)  // Default NVRAM
	ROM_LOAD( "magicrd1.nv", 0x0000, 0x4000, CRC(5b62f04a) SHA1(0cc6404e1bb66801a562ff7a1479859c17e9f209) )
ROM_END


/*

  Magic Card - Wien v1.2 200/93 set 2

  PCB layout:

  +---------------------------------------------------------------------------------------+
  |                                                                                       |
  |        +--------+                   +----------+          +---------+  +--+           |
  |        |74LS14N |                   |KM44C256CP|          |SAA1099P |  |  |           |
  | +--+   +--------+                   +----------+          +---------+  |E |           +---+
  | |GA|                                +----------+                       |  |             --|
  | |L |       +----+                   |KM44C256CP|                       |  |             --|
  | |16|       | A  |                   +----------+                       +--+             --|
  | |V8|       +----+                   +----------+                                        --|
  | |B |                                |KM44C256CP|       +--------------+                 --|
  | |  |                                +----------+       | KDA0476CN-66 |                 --|
  | +--+                                +----------+       | KOREA    219 |                 --|
  |   +------+                          |KM44C256CP|       |              |                 --|
  |   |DS1207|                          +----------+       +--------------+               +---+
  |   +------+                                                                            |
  |                                                                                       |
  |           +--+                    +-------------+        +---------+                  |
  | +--+ +--+ |  |   +------+         |SCC 66470 CAB|        |PIC16C58 |                  |
  | |  | |  | |  |   |LH5164|         |206880       |        +---------+                  +---+
  | |B | |C | |D |   |D-10L |         |DfD9210I3 Y  |                                       --|
  | |  | |  | |  |   |      |         |             |                                       --|
  | |  | |  | |  |   |      |         | PHILIPS 1988|                                       --|
  | +--+ +--+ |  |   |      |         |             |                     +--+              --|
  |           +--+   |      |         |             |        +----------+ |  | +--+         --|
  |                  |      |         |             |        |PC74HC245P| |  | |  |         --|
  |                  |      |         +-------------+        +----------+ |F | |G |         --|
  |                  |      |                                             |  | |  |         --|
  |                  +------+                                             |  | |  |         --|
  |               +------+ +------+                          +----------+ +--+ +--+         --|
  |               |      | |      |                          |PC74HC245P| +--+              --|
  |     +----+    |      | |  W   |    +-------------+       +----------+ |  | +--+         --|
  |     | H  |    |      | |      |    |SCC 68070 CCA|                    |  | |  |         --|
  |     +----+    |      | |      |    |           84|                    |F | |G |         --|
  |               |      | |      |    |268340       |       +----------+ |  | |  |         --|
  |     +--+ +--+ |EMPTY | |M     |    |DfD9349V3 Y  |       |PC74HC245P| |  | |  |         --|
  |+--+ |PA| |PA| |  SLOT| |2     |    |             |       +----------+ +--+ +--+         --|
  ||D | |L | |L | |      | |7     |    | PHILIPS 1988|                    +--------+        --|
  ||I | |  | |  | |      | |C     |    |             |                    | PC849  |        --|
  ||P | |16| |16| |      | |4     |    |             |       +----------+ +--------+        --|
  ||  | |L8| |L8| |      | |0     |    +-------------+       |PC74HC245P|                   --|
  ||2 | |  | |  | |      | |0     |                          +----------+                   --|
  |+--+ +--+ +--+ |      | |2     |                                                       +---+
  |               +------+ +------+                                       +---------+     |
  |                                                                       |  DIP 1  |     |
  |                                                                       +---------+     |
  +---------------------------------------------------------------------------------------+

  A: TL7705ACP
  B: CD4040BE
  C: HEF40098BP
  D: PC74HC273P
  E: LT1081CN
  F: PC74HC273P
  G: ULN2803A
  H: DS1210


  DIP 1:
  +-------------------------------+
  |O N                            |
  |+-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+|
  || | | | |#| |#| |#| |#| |#| |#||
  |+-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+|
  ||#| |#| | | | | | | | | | | | ||
  |+-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+|
  | 1   2   3   4   5   6   7   8 |
  +-------------------------------+

  DIP 2:
  +-------------------------------+
  |O N                            |
  |+-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+|
  ||#| |#| |#| |#| |#| | | |#| | ||
  |+-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+|
  || | | | | | | | | | |#| | | |#||
  |+-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+|
  | 1   2   3   4   5   6   7   8 |
  +-------------------------------+

*/
ROM_START( magicrd1d )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "w.bin", 0x00000, 0x80000, CRC(28300427) SHA1(83ea014a818246f476d769ad06cb2eba1ce699e8) )

	ROM_REGION(0x4d, "ds1207", 0)  // timekey
	ROM_LOAD( "ds1207", 0x000000, 0x00004d, BAD_DUMP CRC(cbcc1a42) SHA1(4b577c85f5856192ce04051a2d305a9080192177) )  // created to match game

	ROM_REGION(0x4000, "nvram", 0)  // Default NVRAM
	ROM_LOAD( "magicrd1d.nv", 0x0000, 0x4000, CRC(2d2e1082) SHA1(f288fa800da59dc89cdca02e528c94161b149f1c) )
ROM_END


/*
  Lucky 7
  Impera
  Version 04/91a

  PCB layout:
   ________________________________________________________________________________________________________________
  |                                                                                                                |
  |      __________                          _____________               ___________        ___                    |
  |     |  74LS04  |                        |HY51C4256S-10|             |     C     |      |904|     ___           |__
  |     |__________|                        |_____________|             |___________|      |9  |    |. .|           __|
  |                                          _____________                                 |   |    |. .|           __|
  |            ____                         |HY51C4256S-10|                                |LT1|    |. .|           __|
  |           | A  |                        |_____________|                                |081|    |. .|           __|
  |           |____|                         _____________                                 |CN |    |. .|           __|
  |                                         |HY51C4256S-10|                                |___|    |. .|           __|
  |                                         |_____________|           __________________            |. .|           __|
  |                                          _____________           |   ADV476KN35E    |           |. .|           __|
  |                                         |HY51C4256S-10|          |                  |           |___|           __|
  | _______                                 |_____________|          |     OF19802.3    |                          |
  ||DS1207 |                                                         |__________________|                          |
  ||_______|                                                                                                       |
  |         ___                       XTAL2                                                                        |
  |        |   |        ________          ________________                                                         |
  |   ___  |PC7|       |        |        |                |                                                        |__
  |  |   | |4HC|       |        |        |    IMPERA 8    |                                                         __|
  |  |HEF| |273|       |HYUNDAI |        |                |                                                         __|
  |  |400| |A  |       |        |        |                |                                                         __|
  |  |98B| |   |       |HY6264AL|        |     160710     |                                                         __|
  |  |P  | |   |       |P_10    |        |                |                                                         __|
  |  |   | |   |       |        |        |  DTD9105I1 Y   |                                                         __|
  |  |___| |___|       |        |        |                |                                ___   ___                __|
  |                    | 9218A  |        |________________|              _____________    |   | |   |               __|
  |   _______          |        |                                       |  74HC245AP  |   |PC7| |ULN|               __|
  |  |       |         | KOREA  |                                       |_____________|   |4HC| |280|               __|
  |  |BATTERY|         |________|                                                         |273| |3A |               __|
  |  |       |                                                                            |AP | |   |               __|
  |  |_______|     ________   ________                                                    |   | |   |               __|
  |               |        | |        |                                                   |   | |   |               __|
  |               |        | |        |                                  _____________    |___| |___|               __|
  |   ___         |        | |        |     XTAL1                       |  74HC245AP  |    ___   ___                __|
  |  | B |        |        | |        |    __________________           |_____________|   |   | |   |               __|
  |  |___|        |        | |        |   |                  |                            |PC7| |ULN|               __|
  |               |        | |        |   |    IMPERA 7      |                            |4HC| |280|               __|
  |               |D27C210 | |D27C210 |   |                  |                            |273| |3A |               __|
  |               |        | |        |   |     155200       |           _____________    |AP | |   |               __|
  |   ___   ___   |        | |        |   |                  |          |  74HC245AP  |   |   | |   |               __|
  |  |   | |   |  |GAME-ROM| |        |   |   DfD9101V3 Y    |          |_____________|   |   | |   |               __|
  |  |PAL| |PAL|  | Lucky 7| |        |   |                  |                            |___| |___|               __|
  |  |16L| |16L|  |        | |        |   |                  |                             __________               __|
  |  |8  | |8  |  |VNr03-07| |        |   |                  |                            |  PC849   |              __|
  |  |   | |   |  |Sum.D882| |        |   |__________________|           _____________    |__________|              __|
  |  |   | |   |  |        | |        |                                 |  74HC245AP  |                             __|
  |  |   | |   |  |        | |        |                                 |_____________|                             __|
  |  |___| |___|  |________| |________|                                                    ___________              __|
  |                                                                                       |   DIP 1   |            |
  | IMPERA BOARD REV V1.04                                                                |___________|            |
  |________________________________________________________________________________________________________________|

  A = TL7705ACP
  B = DS1210
  C = Cover scratched - unreadable

  XTAL1 = 19.6608 (68070)
  XTAL2 = 30.000  (66470)

*/
ROM_START( lucky7i )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "27c210.6", 0x00000, 0x20000, CRC(3a99e9f3) SHA1(b9b533378ce514662cbd85a37ee138a2df760ed4) )
	ROM_LOAD16_WORD_SWAP( "27c210.5", 0x20000, 0x20000, CRC(b4da8856) SHA1(a33158d75047561fa9674ceb6b22cc63b5b49aed) )

	ROM_REGION(0x4d, "ds1207", 0)  // timekey
	ROM_LOAD( "ds1207.bin", 0x000000, 0x00004d, BAD_DUMP CRC(7b838ea7) SHA1(5c22b789251becd20f56f944b76c5b779e5a8892) )  // created to match game

	ROM_REGION(0x4000, "nvram", 0)  // Default NVRAM
	ROM_LOAD( "lucky7i_nvram.bin", 0x0000, 0x4000, CRC(51960419) SHA1(ef7f9d7d9714fda0af23b311232194567887a264) )
ROM_END

/*
  Lucky 7 (alt)
  Ver 04/91a
  2 bytes of difference.

  Early board.

*/
ROM_START( lucky7x )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "nosticker_d27c210_6.bin", 0x00000, 0x20000, CRC(abff21e2) SHA1(88f8265114bbe9ed5004f97d4b3cdc7ae9c3d1e4) )
	ROM_LOAD16_WORD_SWAP( "nosticker_d27c210_5.bin", 0x20000, 0x20000, CRC(b4da8856) SHA1(a33158d75047561fa9674ceb6b22cc63b5b49aed) )

	ROM_REGION(0x4d, "ds1207", 0)  // timekey
	ROM_LOAD( "ds1207.bin", 0x000000, 0x00004d, BAD_DUMP CRC(7b838ea7) SHA1(5c22b789251becd20f56f944b76c5b779e5a8892) )  // created to match game

	ROM_REGION(0x4000, "nvram", 0)  // Default NVRAM
	ROM_LOAD( "lucky7x_nvram.bin", 0x0000, 0x4000, CRC(6f7ac5f1) SHA1(8a36bf38bf226a4001fa73b5243c5784f09063d6) )
ROM_END


/*
  Dallas Poker

  PCB layout:
  +------------------------------------------------------------------------------+
  |                                                           +-------------+    |
  | +---------+    XTAL1                                      |    +------+S|    |
  | |  EMPTY  |                                               |    |AHF   |u|    |
  | +---------+       +-------------+                         |    |Automa|b|    +---+
  | +---------+       |SCC 66470 CAB|                         |    |tentec|b|      --|
  | |P21014-07|       |466006       |       +---------+       |    |hnik  |o|      --|
  | +---------+       |IfD9205I3 Y  |       |SAA1099P |       | X  |      |a|      --|
  | +---------+       |             |       +---------+       | T  |8430 L|r|      --|
  | |  EMPTY  |       | PHILIPS 1988|                         | A  |eibnit|d|      --|
  | +---------+       |             |                         | L  |z     | |      --|
  | +---------+       |             |                         | 3  |      | |      --|
  | |P21014-07|       |             |       +---------------+ +--+ |Tel. 0| |      --|
  | +---------+       +-------------+       |  ADV476KN35E  |    | |3452/3| |    +---+
  | +---------+                             |   03-24 0S    |    | |249   | |    |
  | |  EMPTY  |                             |   0F19802.3   |    | +------+ |    |
  | +---------+                             +---------------+    +----------+    |
  | +---------+         XTAL2                                                    |
  | |P21014-07|       +-------------+                                            |
  | +---------+       |SCC 68070 CBA|      +----+                                |
  | +---------+       |           84|      | F  |                                +---+
  | |  EMPTY  |       |             |      +----+                                  --|
  | +---------+       |   203590    |                                              --|
  | +---------+       | DfD9218V3 Y |                       +---------+            --|
  | |P21014-07|       |             |                       |ULN 2803A|            --|
  | +---------+       | PHILIPS 1988|                       +---------+            --|
  |                   |             |      +----+                                  --|
  |+--+ +--+ +--+     +-------------+      | F  |             +----+               --|
  ||  | |A | |B |                          +----+             | G  |               --|
  ||C | +--+ +--+                                             +----+               --|
  ||  |                                                                            --|
  |+--++--+                                                                        --|
  |    |  |  +--+                          +----+                                  --|
  |    |D |  |  |                          | F  |           +---------+            --|
  |    |  |  |E |                          +----+           |ULN 2803A|            --|
  |    |  |  |  | +--------------------+                    +---------+            --|
  |    +--+  |  | |DALLAS POKER CZ/V1 P|                                           --|
  |          +--+ |VNR:19-09-93        |                      +----+               --|
  |               |SUM:8F8A/w   D27C210|                      | G  |               --|
  |    +--+  +--+ +--------------------+   +----+             +----+               --|
  |    |PA|  |PA|                          | F  |                                  --|
  |    |L |  |L | +--------------------+   +----+            +--------+            --|
  |    |16|  |16| |DALLAS POKER CZ/V1 B|                     | PC849  |            --|
  |    |L8|  |L8| |VNR:19-09-93        |                     +--------+          +---+
  |    |AC|  |AC| |SUM:EB91/w   D27C210|                     +---------+         |
  |    |  |  |  | +--------------------+                     |  DIP 1  |         |
  |    +--+  +--+                                            +---------+         |
  +------------------------------------------------------------------------------+

  XTAL1: 30.0000 (66470)
  XTAL2: 19.6608 (68070)
  XTAL3: 16.000  (PIC?)

  A: TL7705ACP
  B: DS1210
  C: DS1207
  D: HEF40098BP / 759690T / Hnn9210P3
  E: SN74LS14N
  F: HC245A
  G: PC74HC273T

  Under the "DALLAS POKER CZ/V1 B" chip is a PC74HC273T chip soldered on the PCB.
  Under the "DALLAS POKER CZ/V1 P" chip is a MB8464A-10L chip soldered on the PCB.

  Subboard: Looks like an 40PIN MCU or PIC...only four wires connect the subboard
  with the mainboard. (GND & VCC and PIN21 and PIN22 from the 40pin-MCU/PIC)


  DIP 1:
  +-------------------------------+
  |O N                            |
  |+-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+|
  || | |O| | | | | |O| |O| |O| |O||
  |+-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+|
  ||O| | | |O| |O| | | | | | | | ||
  |+-+ +-+ +-+ +-+ +-+ +-+ +-+ +-+|
  | 1   2   3   4   5   6   7   8 |
  +-------------------------------+

*/
ROM_START( dallaspk )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "cz-v1-p.bin", 0x00000, 0x20000, CRC(ad575e3f) SHA1(4e22957c42610fec0a96bd85f4b766422b020d88) )
	ROM_LOAD16_WORD_SWAP( "cz-v1-b.bin", 0x20000, 0x20000, CRC(2595d346) SHA1(34f09931d82b5376e4f3922222645c796dad0440) )

	ROM_REGION(0x4d, "ds1207", 0)  // timekey
	ROM_LOAD( "ds1207", 0x000000, 0x00004d, BAD_DUMP CRC(37adab02) SHA1(2b9859ae6cabfdb9c70f94ccc38a271caf6539aa) )  // created to match game

	ROM_REGION(0x4000, "nvram", 0)  // Default NVRAM
	ROM_LOAD( "dallaspk.nv", 0x0000, 0x4000, CRC(4886d292) SHA1(d06bbeb06c7bc407cb1cf6f2a6d266e578d359e2) )
ROM_END


/*
  Kajot Card
  Version 1.01, Wien Euro.

  Amatic.

  PCB layout:
   ___________________________________________________________________________________________________________________________
  |                      ___    ___    ___    ___                                                                             |
  |    ___              |   |  |   |  |   |  |   |                                                                            |
  |   | B |             | A |  | A |  | A |  | A |    ______________________________________                                  |____
  |   |___|    _____    |   |  |   |  |   |  |   |   |                                      |                                   ___|
  |           |  _  |   |___|  |___|  |___|  |___|   |               YAMAHA                 |                                   ___|
  |  _______  |BATTE|                                |               YM2149F                |                                   ___|
  | |LTC695C| |RY   |    ___    ___    ___    ___    |                                  IC17|                                   ___|
  | |_______| |  +  |   | E |  | E |  | E |  | E |   |______________________________________|                                   ___|
  |           |_____|   | M |  | M |  | M |  | M |                                                                              ___|
  |                     | P |  | P |  | P |  | P |           ___________________________           ______________      _____    ___|
  |   ____    ____      | T |  | T |  | T |  | T |          |                           |         |   ULN2803A   |    | O O |   ___|
  |  |HY62|  |HY62|     | Y |  | Y |  | Y |  | Y |          |       KDA0476CN_50        |         |______________|    | O O |  ____|
  |  |64AL|  |64AL|     |___|  |___|  |___|  |___|          |       KOREA   332B    IC20|        ________________     | O O | |
  |  |J_10|  |J_10|                                         |___________________________|       |    74HC273N    |    | O O | |
  |  |    |  |    |                                                                             |________________|    | O O | |
  |  |____|  |____|                                        ___________           ____________    ________________     | O O | |
  |                                                       | 74HC04AP  |         |EMPTY SOCKET|  |    74HC245N    |    | O O | |
  |  _______   _______                                    |___________|         |____________|  |________________|    | O O | |
  | |       | |       |   XTAL1                                                                                       |_____| |____
  | |       | |       |    _________________                 XTAL3                                 ______________        CON3   ___|
  | | EMPTY | | EMPTY |   |    IMPERA 8     |        _____    _____________     _____________     | EMPTY SOCKET |              ___|
  | | SOCKET| | SOCKET|   |     209751      |       |24C02|  | EMPTY SOCKET|   | EMPTY SOCKET|    |______________|              ___|
  | |       | |       |   |   DfD9227I3 Y   |       |_____|  |_____________|   |_____________|   ________________               ___|
  | |       | |       |   |                 |          IC26             IC29                    |    74HC273N    |              ___|
  | |       | |       |   |                 |                                                   |________________|              ___|
  | |       | |       |   |                 |                                                                                   ___|
  | |       | |       |   |                 |                                                    ________________               ___|
  | |  IC22A| |  IC21A|   |_________________|         __________________                        |    74HC245N    |              ___|
  | |_______| |_______|                  IC19        |                  |                       |________________|              ___|
  |  _______   _______                               |                  |                                                       ___|
  | |       | |       |                              |       ESI 1      |                                                       ___|
  | |  02   | |  01   |   XTAL2                      |       I9407      |                        ________________               ___|
  | |       | |       |    _________________         |                  |                       |    74HC245N    |              ___|
  | |       | |       |   |                 |        |                  |                       |________________|              ___|
  | |27C4002| |27C4002|   |                 |        |              IC25|                                                       ___|
  | |       | |       |   |                 |        |__________________|                                                       ___|
  | |       | |       |   |                 |                                                                                   ___|
  | |       | |       |   | SCC 68070 CCA84 |                                                                                   ___|
  | |       | |       |   |     288571      |                                                                                   ___|
  | |       | |       |   |   DfD9414V3 Y   |                                                                                   ___|
  | |       | |       |   |_________________|                                                                                   ___|
  | |   IC22| |   IC21|                  IC1               ______________    _________                                         ____|
  | |_______| |_______|                                   |  RTC 72421A  |  |  EMPTY  |                                       |
  | IMPERA BOARD REV V2.1                                 |______________|  |_________|                                       |
  |___________________________________________________________________________________________________________________________|

  XTAL1 = 30.000   (66470)
  XTAL2 = 19.6608  (68070)
  XTAL3 = 3686.400 (PIC?)

  A = KM44C256CJ-7
  B = TL7705ACP

*/
ROM_START( kajotcrd )
	ROM_REGION( 0x100000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "01.ic21", 0x00000, 0x80000, CRC(674aa36e) SHA1(483eb09950ff7c43a7147378f2e68d113c856905) )
	ROM_LOAD16_WORD_SWAP( "02.ic22", 0x80000, 0x80000, CRC(ae52803e) SHA1(27f917b0f8b302bdab930e304b4977a4b8192cd5) )

	ROM_REGION( 0x0100, "sereeprom", 0 )  // Serial EPROM
	ROM_LOAD("x24c02.ic26",    0x0000, 0x0100, CRC(0f143d6f) SHA1(c293728a997cd0868705dced55955072c6ebf5c0) )

	ROM_REGION(0x8, "serial_id", 0)  // serial number
	ROM_LOAD( "ds2401", 0x000000, 0x000008, BAD_DUMP CRC(3f87b999) SHA1(29649749d521ced9dc7ef1d0d6ddb9a8beea360f) )  // created to match game
ROM_END


/*
  Poker
  Impera.
  Ver 11/90b

  Early board.

*/
ROM_START( pokeri )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "g55_6__d27c210.bin", 0x00000, 0x20000, CRC(e208598b) SHA1(697b37e39025d31de6f37bd8bd59b35cee998e63) )
	ROM_LOAD16_WORD_SWAP( "g55_5__d27c210.bin", 0x20000, 0x20000, CRC(997d4de9) SHA1(47d46b4be99f4d62e23b78219c5f186476b93701) )

	ROM_REGION(0x4d, "ds1207", 0)  // timekey
	ROM_LOAD( "ds1207.bin", 0x000000, 0x00004d, BAD_DUMP CRC(e0fca9db) SHA1(51d92785fbcadd7e2e420d9f781446991dc72ee2) )  // created to match game

	ROM_REGION(0x4000, "nvram", 0)  // Default NVRAM
	ROM_LOAD( "pokeri_nvram.bin", 0x0000, 0x4000, CRC(6e2dbbf5) SHA1(fd693e466002ada1efa3bdf2a99a6ea26d484e79) )
ROM_END

/*-----------------------

  Simply the Best
  CZ750, V1.00
  2001, Kajot.

  -----------------------

  Hardware specs....

  IC1:
       PHILIPS
       SCC68070CCA84
       594180
       DfD9949V3

  IC5:
       PHILIPS
       SCC66470CAB
       595831
       DfD9948I3

  1x Altera EPM7128SQC100-10 CPLD MAX 7000S Family, 2.5K Gates 128 Macro Cells 100MHz CMOS Technology 5V 100-Pin PQFP.
  1x Philips S87C751-1N24 (IC13): 80C51 8-bit microcontroller family 2Kx64 OTP/ROM, I2C, low pin count NXP Semiconductors. Read protected.
  1x Microchip 24c04a: 4K 5.0V I2C Serial EEPROM.

  2x V62C518256: Mosel Vitelic 32K X 8 static RAM.
  1x HM514270D: 262,144-word x 16-bit Dynamic RAM.

  1x Yamaha YMZ284: Software-controlled Sound Generator (SSGL).

  1x RTC72421 (Real Time Clock).
  1x ADV476 (pin and software compatible RAM-DAC designed specifically for VGA and Personal System/2 color graphics).
  1x Linear Technology LTC695CN: Microprocessor Supervisory Circuits.

  Xtal 1 [Q1]: 30.000
  Xtal 2 [Q2]: 30.000
  Xtal 3 [Q5]: 11.0592
  Xtal 4 [Q6]: 19.6608


*/
ROM_START( simpbest )
	ROM_REGION( 0x80000, "maincpu", 0 )  // 68070 Code & GFX
	ROM_LOAD16_WORD_SWAP( "27c4002.ic23", 0x00000, 0x80000, CRC(ceae7862) SHA1(862baf3312c5076910d001a834661197ca45b766) )

	ROM_REGION( 0x0800, "mcu", 0 )  // S87C751 (2K x8 ROM) undumped
	ROM_LOAD("s87c751.ic13",   0x0000, 0x0800, NO_DUMP )

	ROM_REGION(0x10000, "nvram", 0)  // Default NVRAM
	ROM_LOAD( "simpbest_nvram.bin", 0x00000, 0x10000, CRC(21f4115f) SHA1(bee0755f151758cc591dddd4f41cca2ffab3ec0d) )

	ROM_REGION( 0x0200, "sereeprom", 0 )  // Serial EPROM
	ROM_LOAD16_WORD_SWAP("24c04a.ic27", 0x0000, 0x0200, CRC(3189844c) SHA1(cc017f44d9db92da85c96be750ccec7ee32e5972) )
ROM_END


/*********************************************
*                Driver Init                 *
*********************************************/

void magicard_state::init_dallaspk()
{
//  Dallas Poker...
//  NOP'ing to avoid the 68070 UART stuck...
	uint8_t *rom = memregion("maincpu")->base();

	rom[0x00482e] = 0x18;
	rom[0x00482f] = 0x60;
}

} // anonymous namespace


/*********************************************
*                Game Drivers                *
*********************************************/

//     YEAR  NAME        PARENT    MACHINE         INPUT      STATE           INIT           ROT     COMPANY                 FULLNAME                                     FLAGS

GAME(  1994, magicard,   0,        magicard,       magicard,  magicard_state, empty_init,    ROT0,  "Impera",               "Magic Card (v2.01)",                         MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAMEL( 1994, magicrd1,   0,        magicard_pic56, magicard,  magicard_state, empty_init,    ROT0,  "Impera",               "Magic Card (v1.10 14.09.94)",                MACHINE_SUPPORTS_SAVE,                        layout_magicard )
GAME(  1993, magicrd1a,  magicrd1, magicard,       magicard,  magicard_state, empty_init,    ROT0,  "Impera",               "Magic Card (v1.5 17.12.93, set 1)",          MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME(  1993, magicrd1b,  magicrd1, magicard,       magicard,  magicard_state, empty_init,    ROT0,  "Impera",               "Magic Card (v1.5 17.12.93, set 2)",          MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAMEL( 1993, magicrd1c,  magicrd1, magicard_pic54, magicard,  magicard_state, empty_init,    ROT0,  "Impera",               "Magic Card (v1.2 200/93, set 1)",            MACHINE_SUPPORTS_SAVE,                        layout_magicard )
GAME(  1993, magicrd1d,  magicrd1, magicard,       magicard,  magicard_state, empty_init,    ROT0,  "Impera",               "Magic Card (v1.2 200/93, set 2)",            MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME(  1994, magicrde,   0,        hotslots_pic54, magicrde,  hotslots_state, empty_init,    ROT0,  "Impera",               "Magic Card Export 94 (v2.11a, set 1)",       MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME(  1994, magicrdea,  magicrde, hotslots_pic54, magicrde,  hotslots_state, empty_init,    ROT0,  "Impera",               "Magic Card Export 94 (v2.11a, set 2)",       MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME(  1994, magicrdeb,  magicrde, hotslots,       magicrde,  hotslots_state, empty_init,    ROT0,  "Impera",               "Magic Card Export 94 (V2.11a, set 3)",       MACHINE_SUPPORTS_SAVE )
GAME(  1994, magicrdec,  magicrde, hotslots,       magicrde,  hotslots_state, empty_init,    ROT0,  "Impera",               "Magic Card Export 94 (v2.09a)",              MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME(  1998, magicrdj,   0,        hotslots,       magicrde,  hotslots_state, empty_init,    ROT0,  "Impera",               "Magic Card III Jackpot (V4.01 6/98)",        MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME(  1998, magicrdja,  magicrdj, hotslots,       magicrde,  hotslots_state, empty_init,    ROT0,  "Impera",               "Magic Card III Jackpot (V4.01 7/98)",        MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME(  2001, magicle,    0,        magicle,        hotslots,  hotslots_state, empty_init,    ROT0,  "Impera",               "Magic Lotto Export (5.03)",                  MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME(  2002, hotslots,   0,        hotslots,       hotslots,  hotslots_state, empty_init,    ROT0,  "Impera",               "Hot Slots (6.00)",                           MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME(  1999, quingo,     0,        magicle,        hotslots,  hotslots_state, empty_init,    ROT0,  "Impera",               "Quingo Export (5.00)",                       MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME(  1999, belslots,   0,        magicle,        hotslots,  hotslots_state, empty_init,    ROT0,  "Impera",               "Bel Slots Export (5.01)",                    MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME(  2001, bigdeal0,   0,        magicle,        magicard,  hotslots_state, empty_init,    ROT0,  "Impera",               "Big Deal Belgien (5.04)",                    MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME(  199?, puzzleme,   0,        puzzleme,       puzzleme,  hotslots_state, empty_init,    ROT0,  "Impera",               "Puzzle Me!",                                 MACHINE_SUPPORTS_SAVE )

GAME(  1991, lucky7i,    0,        magicard,       lucky7i,   magicard_state, empty_init,    ROT0,  "Impera",               "Lucky 7 (Impera, V04/91a, set 1)",           MACHINE_SUPPORTS_SAVE )
GAME(  1991, lucky7x,    lucky7i,  magicard,       lucky7i,   magicard_state, empty_init,    ROT0,  "Impera",               "Lucky 7 (Impera, V04/91a, set 2)",           MACHINE_SUPPORTS_SAVE )
GAMEL( 1993, dallaspk,   0,        magicard,       dallaspk,  magicard_state, init_dallaspk, ROT0,  "AHF Automatentechnik", "Dallas Poker (CZ/V1)",                       MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING,  layout_dallaspk )
GAME(  1993, kajotcrd,   0,        hotslots,       magicard,  hotslots_state, empty_init,    ROT0,  "Amatic",               "Kajot Card (Version 1.01, Wien Euro)",       MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAMEL( 1991, pokeri,     0,        magicard,       pokeri,    magicard_state, empty_init,    ROT0,  "Impera",               "Poker (Impera, V11/90b)",                    MACHINE_SUPPORTS_SAVE,                        layout_pokeri )

GAMEL( 2001, simpbest,   0,        simpbest,       simpbest,  hotslots_state, empty_init,    ROT0,  "Kajot",                "Simply the Best (CZ750, v1.0)",              MACHINE_SUPPORTS_SAVE | MACHINE_UNEMULATED_PROTECTION | MACHINE_NOT_WORKING,  layout_simpbest )
