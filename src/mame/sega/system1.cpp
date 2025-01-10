// license:BSD-3-Clause
// copyright-holders:Jarek Parchanski, Nicola Salmoria, Mirko Buffoni
/******************************************************************************

Sega System 1 / System 2

driver by Jarek Parchanski, Nicola Salmoria, Mirko Buffoni

Up'n Down, Mister Viking, Flicky, SWAT, Water Match and Bull Fight are known
to run on IDENTICAL hardware (they were sold by Bally-Midway as ROM swaps).

TODO:
- fully understand nobb ports involved in the protection

*******************************************************************************

            Main  Snd   Gfx1  Gfx2            Max     Min
Game        ROMs  ROMs  ROMs  ROMs  Pal? Intf EPR#    EPR#
----------- ----- ----- ----- ----- ---- ---- ----    ----
starjack    6x8k  1x8k  6x8k  2x16k  no  ppi  5325(b) 5318
starjacks   6x8k  1x8k  6x8k  2x16k  no  ppi  license

upndown     6x8k  1x8k  6x8k  2x16k  no  ppi  5521    5514
upndownu    6x8k  1x8k  6x8k  2x16k  no  ppi  5684

regulus     6x8k  1x8k  6x8k  2x16k  no  ppi  5645(a) 5638
reguluso    6x8k  1x8k  6x8k  2x16k  no  ppi  5645
regulusu    6x8k  1x8k  6x8k  2x16k  no  ppi  5955

mrviking    6x8k  1x8k  6x8k  2x16k  no  ppi  5876    5749
mrvikingj   6x8k  1x8k  6x8k  2x16k  no  ppi  5756

swat        6x8k  1x8k  6x8k  2x16k  no  ppi  5812    5805

flickys1    4x8k  1x8k  6x8k  2x16k  no  ppi  ????    5855
flickyo     4x8k  1x8k  6x8k  2x16k  no  ppi  5860(a)
flicky      2x16k 1x8k  6x8k  2x16k  no  pio  5979(a)
flickys2    2x16k 1x8k  6x8k  2x16k  no  pio  6622

wmatch      6x8k  1x8k  6x8k  2x16k  no  ppi  ????    ????

bullfgt     6x8k  1x8k  6x8k  2x16k  no  ppi  ????    6069
thetogyu    3x16k 1x8k  6x8k  2x16k  no  pio  6073

spatter     3x16k 1x8k  6x8k  4x16k  no  pio  6394    6306
spattera    3x16k 1x8k  6x8k  4x16k  no  pio  6599    6306
ssanchan    3x16k 1x8k  6x8k  4x16k  no  pio  6312

pitfall2    3x16k 1x8k  6x8k  2x16k  no  pio  6458(a) 6454
pitfall2a   3x16k 1x8k  6x8k  2x16k  no  pio  6506
pitfall2u   3x16k 1x8k  6x8k  2x16k  no  pio  6625(a)

seganinj    3x16k 1x8k  6x8k  4x16k  no  pio  ????    6546
seganinju   3x16k 1x8k  6x8k  4x16k  no  pio  7150
nprinceso   3x16k 1x8k  6x8k  4x16k  no  pio  6552
nprincesb   3x16k 1x8k  6x8k  4x16k  no  pio  bootleg
ninja       3x16k 1x8k  6x8k  4x16k  no  pio  6595
nprinces    6x8k  1x8k  6x8k  4x16k  no  ppi  6617
nprincesu   6x8k  1x8k  6x8k  4x16k  no  ppi  6578

imsorry     3x16k 1x8k  6x8k  2x16k  no  pio  6678    6645
imsorryj    3x16k 1x8k  6x8k  2x16k  no  pio  6649

teddybb     3x16k 1x8k  6x8k  4x16k  no  pio  6770    6735
teddybbo    3x16k 1x8k  6x8k  4x16k  no  pio  6741
teddybbobl  3x16k 1x8k  3x16k 4x16k  no  pio  bootleg

hvymetal    3x32k 1x32k 6x16k 4x32k  yes      6790    6778

myhero      3x16k 1x8k  6x8k  4x16k  no  pio  6964    6921
sscandal    3x16k 1x8k  6x8k  4x16k  no  pio  6927
myherok     3x16k 1x8k  6x8k  4x16k  no  pio  bootleg?

4dwarrio    3x16k 1x8k  6x8k  4x16k  no  pio  ????    ????

shtngmst    3x32k 1x32k 3x32k 7x32k  yes      7102

choplift    3x32k 1x32k 3x32k 4x32k  yes      7126    7120
chopliftu   3x32k 1x32k 3x32k 4x32k  yes      7154
chopliftbl  3x32k 1x32k 3x32k 4x32k  yes      bootleg

raflesia    3x16k 1x8k  6x8k  4x16k  no  pio  7413    7408
raflesiau   6x8k  1x8k  6x8k  4x16k  no  ppi  ????    ????

wboy2       6x8k  1x8k  6x8k  4x16k  no  ppi  7592    7485
wboy2u      6x8k  1x8k  6x8k  4x16k  no  ppi  ????
wbdeluxe    6x8k  1x8k  6x8k  4x16k  no  ppi  ????
wboy        3x16k 1x8k  6x8k  4x16k  no  pio  7491
wboyo       3x16k 1x8k  6x8k  4x16k  no  pio  ????
wboy3       3x16k 1x8k  6x8k  4x16k  no  pio  ????
wboyu       3x16k 1x8k  6x8k  4x16k  no  pio  ????
wboy4       2x32k 1x32k 3x16k 2x32k  no       ????
wboysys2    2x32k 1x32k 3x32k 2x32k  yes      7580

gardia      3x32k 1x16k 3x16k 4x32k  yes      10255   10233
gardiab     3x32k 1x16k 3x16k 4x32k  yes      bootleg

brain       3x32k 1x32k 3x16k 3x32k  yes      ????    ????

tokisens    3x32k 1x32k 3x32k 4x32k  yes      10963   10957

wbml        3x32k 1x32k 3x32k 4x32k  yes      11033(a) 11027
wbmljo      3x32k 1x32k 3x32k 4x32k  yes      11033
wbmljb      3x64k 1x32k 3x32k 4x32k  yes      bootleg
wbmlb       3x64k 1x32k 3x32k 4x32k  yes      bootleg
wbmlg       3x64k 1x32k 3x32k 4x32k  yes      bootleg

dakkochn    2x32k 1x32k 3x32k 4x32k  yes      11225   11220

ufosensi    3x32k 1x32k 3x32k 4x32k  yes      11663   11657
ufosensib   3x64k 1x32k 3x32k 4x32k  yes      bootleg

blockgal    2x16k 1x8k  6x8k  4x16k  no               ????
blockgalb   1x64k 1x8k  6x8k  4x16k  no       bootleg

nob         3x32k 1x16k 3x32k 4x32k  yes
nobb        3x32k 1x16k 3x32k 4x32k  yes

*******************************************************************************

Spatter (315-5099)
Sega 1984

This game runs on Sega System 1 hardware.

834-5583-12 SPATTER (sticker)
834-5542 daughter board with 4 eproms (EPR6306, EPR6307, EPR6308, EPR6309)
834-5540 daughter board with logic ICs
315-5099 custom Z80 CPU w/security

*******************************************************************************

Chop Lifter, Sega 1985
Hardware info by Guru

This game runs on Sega System 2 hardware.

171-5303-01
834-5795-03 CHOP LIFTER (sticker)
|-----------------------------------------------------------------|
|DSW2 DSW1   315-5011   EPR-7120.86 CXK5808   Z80A(2)   20MHz     |
|TD62003   315-5012     EPR-7121.87 CXK5808            315-5152.10|
|                       EPR-7122.88                    315-5138.11|
|                       EPR-7123.89                       315-5049|
|                                                                 |
|                       EPR-7152.90                               |
|4                      EPR-7153.91                       M5M5165 |
|4                      EPR-7154.92                       M5M5165 |
|W                      M5M5165                                   |
|A   8255                             DIP40             EPR-7127.4|
|Y                                      315-5139.50     EPR-7128.5|
|     2148 2148 2148             TL7705                 EPR-7129.6|
|     2148 2148 2148                                   315-5025   |
|                                                      315-5025   |
|         8MHz                           PR5317.37     315-5025   |
|     Z80A(1)                                LED     PR7117.8     |
| VOL   76489(1)                                     PR7118.14    |
|       76489(2) EPR-7130.126  8147                  PR7119.20    |
|LA4460       MB8128           8147             MB8128            |
|-----------------------------------------------------------------|
Notes:
      315-5011   - Sega Custom IC (DIP40)
      315-5012   - Sega Custom IC (DIP48)
      315-5025   - Sega Custom IC (DIP18)
      315-5138   - PAL16R4 (DIP20)
      315-5139   - Signetics CK2605 (= PLS153) stamped '315-5139' (DIP20)
      315-5049   - Sega Custom IC (SDIP64)
      315-5152   - PAL16R4 (DIP20)
      DIP40      - DIP40 socket for 8751 MCU. Some games like this version of Chop Lifter use a small
                   DIP40-sized board plugged into the socket marked 'SEGA 839-0001'. The board contains
                   nothing. The bottom of the board may have tracks going to other pins but it's obscured
                   by the socket connector. The top of the board has no tracks on it.
      EPR-*      - All EPROMs are 27C256 (DIP28)
      MB8128     - Fujitsu MB8128 -10 2k x8 SRAM (DIP24)
      2148       - Intel P2148H-3 1k x4 SRAM (DIP18)
      2147       - Fujitsu MB2147-45 4k x1 SRAM (DIP18)
      TL7705     - Texas Instruments TL7705 Voltage Supply Supervisor and Master Reset IC (DIP8)
      LED        - Power LED
      8255       - NEC D8255 Programmable Peripheral Interface IC (DIP40)
      Z80A(1)    - Sharp LH0080A Z80A CPU, clock 4.000MHz [8/2]
      Z80A(2)    - Sharp LH0080A Z80A CPU, clock 4.000MHz [8/2]
      CXK5808    - Sony CXK5808 1kBx8-bit SRAM (NDIP22)
      M5M5165    - Mitsubishi M5M5165 8k x8 SRAM (DIP28)
      SN76489(1) - Texas Instruments SN76489 4-channel Programmable Sound Generator. Clock 4.000MHz [8/2] (DIP16)
      SN76489(2) - Texas Instruments SN76489 4-channel Programmable Sound Generator. Clock 2.000MHz [8/4] (DIP16)
      LA4460     - Sanyo LA4460 12W AF Power Amplifier (SIL10)
      TD62003    - Toshiba TD62003 7-channel Darlington Sink Driver (DIP16)
      PR5317     - Fujitsu MB7114 Bipolar PROM (DIP16)
      PR7117     - Fujitsu MB7114 Bipolar PROM (DIP16)
      PR7118     - MMI 63S141 Bipolar PROM (DIP16)
      PR7119     - Fujitsu MB7114 Bipolar PROM (DIP16)

      Measurements
      ------------
      OSC1  - 7.99992MHz
      OSC2  - 19.99982MHz
      VSync - 60.0952Hz
      HSync - 15.4442kHz

***************************************************************************

Pitfall II The Lost Caverns, Sega, 1984
Hardware info by Guru

This game runs on Sega System 1 hardware. The version documented here is
the not-encrypted version.
The same PCB runs a few other games, including some official Sega conversions.
For example: My Hero, Teddy Boy Blues, Sega Ninja, Ninja Princess and several others.
The bootleg Pitfall II PCB is an exact 1:1 copy, including using the same encrypted ROMs,
and the custom chips have been replaced with plug-in daughterboards.


Sega Game ID#: 834-5627-10 PITFALL II (sticker). Also seen: -11, -12 and -13 stickers.
PCB#: 171-5054-02 (seen on some PCBs with (C) 1984). The (C) 1985 PCB does not have a 171 number on the PCB.
|--------------------------------------------------------------------------------|
|            20MHz                    315-5063.IC67                              |
|                                                                            LED |
|Z80(1)    EPR-6623.IC116        D4168                 315-5062.IC41             |
|              EPR-6624A.IC109                                                   |
|                  EPR-6625.IC96                         25LS251  2148 2148 2148 |
|                                                                                |
|    EPR-6454A.IC117  X  X   EPR-6455.IC05                        2148 2148 2148 |
|                                                                                |
| TLP521-4(x6)     |--|  |--|                                                    |
|4                 |3 |  |3 |    2148                                            |
|4         DIPSW_B |1 |  |1 |    2148                                       2147 |
|W                 |5 |  |5 |    2148                                            |
|A         DIPSW_A || |  || |    2148           EPR-6473A.IC61                   |
|Y                 |5 |  |5 |                               315-5025             |
|                  |0 |  |0 |                   EPR-6474A.IC62    74S201         |
|    ULN2003       |1 |  |1 |      8128  8128               315-5025             |
|                  |2 |  |1 |                   EPR-6471A.IC63                   |
|                  |--|  |--|                               315-5025             |
|       Z80PIO     Z80        8128              EPR-6472A.IC64                   |
|      8MHz                                                                      |
|   VOL     EPR-6462.IC120                      EPR-6469A.IC65                   |
| LA4460 76489A(1)     8128                                                      |
|            76489A(2)             PR5317.IC76  EPR-6470A.IC66                   |
|--------------------------------------------------------------------------------|
Notes:
       315-5011 - Sega custom DIP40 IC \
       315-5012 - Sega custom DIP48 IC / The bootleg replaces these two chips with a plug-in daughterboard containing logic chips
         Z80(1) - Z80 CPU. Clock input measures 3.76992MHz on first power on and changes to 3.65950MHz or 3.80062MHz during game play.
                  Replaced with encrypted Z80 with sticker '315-5093' on the encrypted version.
                  Some factory conversions have been seen with a plug-in daughterboard containing Z80, PAL and PROM and some later games
                  or later releases of the same game use a stock Z80 without encryption.
       315-5025 - Sega custom IC. The bootleg replaces this with a plug-in daughterboard containing logic chips: 74LS299 (x6), 74LS273 (x3), 74LS157 (x3)
      76489A(1) - Texas Instruments SN76489 4-channel Programmable Sound Generator. Clock 4.000MHz [8/2]
      76489A(2) - Texas Instruments SN76489 4-channel Programmable Sound Generator. Clock 2.000MHz [8/4]
         LA4460 - Sanyo LA4460 12W AF Power Amplifier
         PR5317 - Fujitsu MB7114 Bipolar PROM (equivalent to 82S129)
            Z80 - Zilog Z8400A Z80A CPU or Sharp LH0080A CPU or NEC D780C-1 CPU. Clock input 4.000MHz
         Z80PIO - Zilog Z8420A Z80A-PIO or Sharp LH0081A Z80A-PIO. Clock input measures the same as the Z80(1) clock
          D4168 - NEC D4168 8kBx8-bit SRAM, equivalent to 6264
           8128 - Fujitsu MB8128 2kBx8-bit SRAM, equivalent to 6116
           2148 - Fujitsu MB2148 1kBx4-bit SRAM
           2147 - Fujitsu MB2147 4kBx1-bit SRAM
         74S201 - Texas Instruments 256bx1-bit SRAM
       315-5063 - National DMPAL16R4
       315-5062 - National DMPAL16R4
        25LS251 - AMD AM25LS251 8-Input Multiplexor
 EPR-6623.IC116 \
EPR-6624A.IC109  / 27128 16kBx8-bit EPROM (main program, not-encrypted version)
  EPR-6625.IC96 /
 EPR-6462.IC120 - 2764 8kBx8-bit EPROM (sound program)
 EPR-6473A.IC61 \
 EPR-6474A.IC62  \
 EPR-6471A.IC63   \
 EPR-6472A.IC64   / 2764 8kBx8-bit EPROM (background tiles)
 EPR-6469A.IC65  /
 EPR-6470A.IC66 /
EPR-6454A.IC117 \
  EPR-6455.IC05 / 27128 16kBx8-bit EPROM (sprites)
              X - Empty socket
       TLP521-4 - Toshiba TLP521-4 4-Channel Photocoupler
        ULN2003 - Texas Instruments ULN2003 or Toshiba TD62003 7-channel Darlington Sink Driver
      DIPSW_A,B - 8-position DIP switch
            LED - Power LED
          VSync - 60.0757Hz
          HSync - 15.2585kHz

***************************************************************************

Flicky sets version notes:

flicky, flickyo
---------------
They both seem to be very similar programs.  Difficulty is easier than the S1,S2 sets.
DIPs are also shared 100% with each other.

flickys1, flickys2
------------------
Very noticeably more difficult than the other two sets.  DIPs have changes (less lives
and bonus options).  There is no screen which shows the bonus lives values like the
other two sets, either.  flickys1 allows for DEMO SOUND which none of the others sets
seem to have access to.

******************************************************************************/

#include "emu.h"
#include "system1.h"

#include "machine/input_merger.h"
#include "machine/segacrpt_device.h"
#include "cpu/z80/mc8123.h"

#include "speaker.h"


#define MASTER_CLOCK    XTAL(20'000'000)
#define SOUND_CLOCK     XTAL(8'000'000)


/*************************************
 *
 *  Machine initialization
 *
 *************************************/

void system1_state::machine_start()
{
	const u32 numbanks = (m_maincpu_region->bytes() - 0x10000) / 0x4000;

	if (numbanks > 0)
		m_bank1->configure_entries(0, numbanks, m_maincpu_region->base() + 0x10000, 0x4000);
	else
		m_bank1->configure_entry(0, m_maincpu_region->base() + 0x8000);
	m_bank1->set_entry(0);

	if (m_banked_decrypted_opcodes)
	{
		m_bank0d->set_base(m_banked_decrypted_opcodes.get());
		m_bank1d->configure_entries(0, numbanks, m_banked_decrypted_opcodes.get() + 0x10000, 0x4000);
		m_bank1d->set_entry(0);
	}

	save_item(NAME(m_adjust_cycles));
	save_item(NAME(m_mcu_control));
	save_item(NAME(m_nob_maincpu_latch));
	save_item(NAME(m_nob_mcu_latch));
	save_item(NAME(m_nob_mcu_status));
	save_item(NAME(m_nobb_inport23_step));
}


/*************************************
 *
 *  Main CPU clocking
 *
 *************************************/

/*
    A 20MHz crystal clocks an LS161 which counts up from either 10 or 11 to 16 before
    carrying out and forcing a reload. The low bit of the reload value comes from the
    Z80's /M1 signal. When /M1 is low (an opcode is being fetched), the reload count
    is 10, which means the 20MHz clock is divided by 6. When /M1 is high, the reload
    count is 11, which means the clock is divided by 5.

    Since /M1 is low for 2 cycles during opcode fetch, this makes every opcode fetch
    take an extra 2 20MHz clocks, which is 2/5th cycles at 4MHz.
*/

void system1_state::adjust_cycles(u8 data)
{
	m_adjust_cycles = (m_adjust_cycles + 2) % 5;
	if (m_adjust_cycles <= 1)
		m_maincpu->adjust_icount(-1);
}


/*************************************
 *
 *  ROM banking
 *
 *************************************/

void system1_state::bank44_custom_w(u8 data)
{
	// bank bits are bits 6 and 2
	m_bank1->set_entry(((data & 0x40) >> 5) | ((data & 0x04) >> 2));
}


void system1_state::bank0c_custom_w(u8 data)
{
	// bank bits are bits 3 and 2
	m_bank1->set_entry((data & 0x0c) >> 2);
	if (m_bank1d)
		m_bank1d->set_entry((data & 0x0c) >> 2);
}


void system1_state::videomode_w(u8 data)
{
	// bits 0/1 are for the coin counters
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));

	// bit 6 is connected to the 8751 IRQ
	if (m_mcu != nullptr)
		m_mcu->set_input_line(MCS51_INT1_LINE, BIT(data, 6) ? CLEAR_LINE : ASSERT_LINE);

	// handle any custom banking or other stuff
	if (m_videomode_custom != nullptr)
		(this->*m_videomode_custom)(data);

	// remaining signals are video-related
	common_videomode_w(data);
}


/*************************************
 *
 *  Shooting Master gun input
 *
 *************************************/

void shtngmst_state::machine_start()
{
	system1_state::machine_start();

	m_gun_solenoid.resolve();

	save_item(NAME(m_gun_output));
	save_item(NAME(m_gun_trigger));
}


INPUT_CHANGED_MEMBER(shtngmst_state::gun_trigger)
{
	if (newval && BIT(m_gun_output, 0))
		m_gun_trigger = 1;
}


u8 shtngmst_state::gun_trigger_r()
{
	// bit 6 = gun trigger latch
	// bit 7 = light sensor?
	return ~(m_gun_trigger << 6);
}


void shtngmst_state::gun_output_w(u8 data)
{
	// bit 0 readies the gun?
	if (!BIT(data, 0))
		m_gun_trigger = 0;

	// bit 2 = gun solenoid
	m_gun_solenoid = BIT(data, 2);

	m_gun_output = data;
}


/*************************************
 *
 *  DakkoChan House custom inputs
 *
 *************************************/

void dakkochn_state::machine_start()
{
	system1_state::machine_start();

	save_item(NAME(m_mux_count));
	save_item(NAME(m_mux_clock));
}


void dakkochn_state::machine_reset()
{
	system1_state::machine_reset();
	m_mux_count = 0;
}


ioport_value dakkochn_state::mux_data_r()
{
	return m_keys[m_mux_count]->read();
}


ioport_value dakkochn_state::mux_status_r()
{
	// reads from here indicate which mux port is selected
	return 1 << m_mux_count;
}


void dakkochn_state::videomode_w(u8 data)
{
	// bit 1 toggling on clocks the mux
	if (BIT(data, 1) && !m_mux_clock)
		m_mux_count = (m_mux_count + 1) % 7;
	m_mux_clock = BIT(data, 1);

	// remaining stuff acts like system1 (bank0c)
	system1_state::videomode_w(data & ~0x02);
}


/*************************************
 *
 *  Sound I/O
 *
 *************************************/

void system1_state::sound_control_w(u8 data)
{
	// bit 0 = MUTE (inverted sense on System 2)
	machine().sound().system_mute(data & 1);

	// bit 6 = feedback from sound board that read occurred

	// bit 7 controls the sound CPU's NMI line
	m_soundcpu->set_input_line(INPUT_LINE_NMI, (data & 0x80) ? CLEAR_LINE : ASSERT_LINE);

	// remaining bits are used for video RAM banking
	videoram_bank_w(data);
}


u8 system1_state::sound_data_r()
{
	// if we have an 8255 PPI, get the data from the port and toggle the ack
	if (m_ppi8255 != nullptr)
	{
		m_ppi8255->pc6_w(0);
		m_ppi8255->pc6_w(1);
		return m_soundlatch->read();
	}

	// if we have a Z80 PIO, get the data from the port and toggle the strobe
	else if (m_pio != nullptr)
	{
		u8 data = m_pio->port_read(z80pio_device::PORT_A);
		m_pio->strobe(z80pio_device::PORT_A, false);
		m_pio->strobe(z80pio_device::PORT_A, true);
		return data;
	}

	return 0xff;
}


void system1_state::soundport_w(u8 data)
{
	// boost interleave when communicating with the sound CPU
	m_soundlatch->write(data);
	machine().scheduler().perfect_quantum(attotime::from_usec(100));
}


TIMER_DEVICE_CALLBACK_MEMBER(system1_state::soundirq_gen)
{
	// sound IRQ is generated on 32V, 96V, ... and auto-acknowledged
	m_soundcpu->set_input_line(0, HOLD_LINE);
}


/*************************************
 *
 *  MCU I/O
 *
 *************************************/

void system1_state::mcu_control_w(u8 data)
{
	/*
	    Bit 7 -> connects to TD62003 pins 5 & 6 @ IC151
	    Bit 6 -> via PLS153, when high, asserts the BUSRQ signal, halting the Z80
	    Bit 5 -> n/c
	    Bit 4 -> (with bit 3) Memory select: 0=Z80 program space, 1=banked ROM, 2=Z80 I/O space, 3=watchdog?
	    Bit 3 ->
	    Bit 2 -> n/c
	    Bit 1 -> n/c
	    Bit 0 -> Directly connected to Z80 /INT line
	*/

	// boost interleave to ensure that the MCU can break the Z80 out of BUSRQ
	if (!BIT(m_mcu_control, 6) && BIT(data, 6))
		machine().scheduler().perfect_quantum(attotime::from_usec(10));

	m_mcu_control = data;
	m_maincpu->set_input_line(Z80_INPUT_LINE_BUSRQ, (data & 0x40) ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(0, (data & 0x01) ? CLEAR_LINE : ASSERT_LINE);
}


void system1_state::mcu_io_w(offs_t offset, u8 data)
{
	switch ((m_mcu_control >> 3) & 3)
	{
		case 0:
			m_maincpu->space(AS_PROGRAM).write_byte(offset, data);
			break;

		case 2:
			m_maincpu->space(AS_IO).write_byte(offset, data);
			break;

		default:
			logerror("%03X: MCU movx write mode %02X offset %04X = %02X\n",
					m_mcu->pc(), m_mcu_control, offset, data);
			break;
	}
}


u8 system1_state::mcu_io_r(offs_t offset)
{
	switch ((m_mcu_control >> 3) & 3)
	{
		case 0:
			return m_maincpu->space(AS_PROGRAM).read_byte(offset);

		case 1:
			return m_maincpu_region->base()[offset + 0x10000];

		case 2:
			return m_maincpu->space(AS_IO).read_byte(offset);

		default:
			if (!machine().side_effects_disabled())
			{
				logerror("%03X: MCU movx read mode %02X offset %04X\n",
						m_mcu->pc(), m_mcu_control, offset);
			}
			return 0xff;
	}
}


IRQ_CALLBACK_MEMBER(system1_state::mcu_t0_callback)
{
	m_mcu->set_input_line(MCS51_T0_LINE, ASSERT_LINE);
	m_mcu->set_input_line(MCS51_T0_LINE, CLEAR_LINE);

	return 0xff;
}


/*************************************
 *
 *  nob MCU
 *
 *************************************/

u8 system1_state::nob_mcu_latch_r()
{
	return m_nob_mcu_latch;
}

void system1_state::nob_mcu_latch_w(u8 data)
{
	m_nob_mcu_latch = data;
}

void system1_state::nob_mcu_status_w(u8 data)
{
	m_nob_mcu_status = data;
}

void system1_state::nob_mcu_control_p2_w(u8 data)
{
	// bit 0 triggers a read from MCU port 0
	if (((m_mcu_control ^ data) & 0x01) && !(data & 0x01))
		m_nob_mcu_latch = m_nob_maincpu_latch;

	// bit 1 triggers a write from MCU port 0
	if (((m_mcu_control ^ data) & 0x02) && !(data & 0x02))
		m_nob_maincpu_latch = m_nob_mcu_latch;

	// bit 2 is toggled once near the end of an IRQ
	if (((m_mcu_control ^ data) & 0x04) && !(data & 0x04))
		m_mcu->set_input_line(MCS51_INT0_LINE, CLEAR_LINE);

	// bit 3 is toggled once at the start of an IRQ, and again at the end
	if (((m_mcu_control ^ data) & 0x08) && !(data & 0x08))
	{
		//logerror("MCU IRQ(8) toggle\n");
	}

	m_mcu_control = data;
}


u8 system1_state::nob_maincpu_latch_r()
{
	return m_nob_maincpu_latch;
}


void system1_state::nob_maincpu_latch_w(u8 data)
{
	m_nob_maincpu_latch = data;
	m_mcu->set_input_line(MCS51_INT0_LINE, ASSERT_LINE);
	machine().scheduler().perfect_quantum(attotime::from_usec(100));
}


u8 system1_state::nob_mcu_status_r()
{
	return m_nob_mcu_status;
}


/*************************************
 *
 *  nob bootleg protection
 *
 *************************************/

u8 system1_state::nobb_inport1c_r()
{
//  logerror("IN  $1c : pc = %04x - data = 0x80\n",m_maincpu->pc());
	return(0x80); // infinite loop (at 0x0fb3) until bit 7 is set
}

u8 system1_state::nobb_inport22_r()
{
//  logerror("IN  $22 : pc = %04x - data = %02x\n",m_maincpu->pc(),nobb_inport17_step);
	return(0); // nobb_inport17_step;
}

u8 system1_state::nobb_inport23_r()
{
//  logerror("IN  $23 : pc = %04x - step = %02x\n",m_maincpu->pc(),m_nobb_inport23_step);
	return(m_nobb_inport23_step);
}

void system1_state::nobb_outport24_w(u8 data)
{
//  logerror("OUT $24 : pc = %04x - data = %02x\n",m_maincpu->pc(),data);
	m_nobb_inport23_step = data;
}


/*************************************
 *
 *  Main CPU address maps
 *
 *************************************/

// main memory map
void system1_state::system1_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xc000, 0xcfff).ram().share("ram");
	map(0xd000, 0xd7ff).ram().share("spriteram");
	map(0xd800, 0xdfff).ram().w(FUNC(system1_state::paletteram_w)).share("paletteram");
	map(0xe000, 0xefff).rw(FUNC(system1_state::videoram_r), FUNC(system1_state::videoram_w));
	map(0xf000, 0xf3ff).rw(FUNC(system1_state::mixer_collision_r), FUNC(system1_state::mixer_collision_w));
	map(0xf400, 0xf7ff).w(FUNC(system1_state::mixer_collision_reset_w));
	map(0xf800, 0xfbff).rw(FUNC(system1_state::sprite_collision_r), FUNC(system1_state::sprite_collision_w));
	map(0xfc00, 0xffff).w(FUNC(system1_state::sprite_collision_reset_w));
}

void system1_state::decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().share("decrypted_opcodes");
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xc000, 0xcfff).ram().share("ram");
	map(0xd000, 0xd7ff).ram().share("spriteram");
	map(0xd800, 0xdfff).ram().w(FUNC(system1_state::paletteram_w)).share("paletteram");
}

void system1_state::banked_decrypted_opcodes_map(address_map &map)
{
	map(0x0000, 0x7fff).bankr("bank0d");
	map(0x8000, 0xbfff).bankr("bank1d");
	map(0xc000, 0xcfff).ram().share("ram");
	map(0xd000, 0xd7ff).ram().share("spriteram");
	map(0xd800, 0xdfff).ram().w(FUNC(system1_state::paletteram_w)).share("paletteram");
}

// same as normal System 1 except address map is shuffled (RAM/collision are swapped)
void system1_state::nobo_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr("bank1");
	map(0xc000, 0xc3ff).rw(FUNC(system1_state::mixer_collision_r), FUNC(system1_state::mixer_collision_w));
	map(0xc400, 0xc7ff).w(FUNC(system1_state::mixer_collision_reset_w));
	map(0xc800, 0xcbff).rw(FUNC(system1_state::sprite_collision_r), FUNC(system1_state::sprite_collision_w));
	map(0xcc00, 0xcfff).w(FUNC(system1_state::sprite_collision_reset_w));
	map(0xd000, 0xd7ff).ram().share("spriteram");
	map(0xd800, 0xdfff).ram().w(FUNC(system1_state::paletteram_w)).share("paletteram");
	map(0xe000, 0xefff).rw(FUNC(system1_state::videoram_r), FUNC(system1_state::videoram_w));
	map(0xf000, 0xffff).ram().share("ram");
}

// I/O map for systems with an 8255 PPI
void system1_state::system1_ppi_io_map(address_map &map)
{
	map.global_mask(0x1f);
	map(0x00, 0x00).mirror(0x03).portr("P1");
	map(0x04, 0x04).mirror(0x03).portr("P2");
	map(0x08, 0x08).mirror(0x03).portr("SYSTEM");
	map(0x0c, 0x0c).mirror(0x02).portr("SWA");    // DIP2
	map(0x0d, 0x0d).mirror(0x02).portr("SWB");    // DIP1 some games read it from here...
	map(0x10, 0x10).mirror(0x03).portr("SWB");    // DIP1 ... and some others from here but there are games which check BOTH!
	map(0x14, 0x17).rw(m_ppi8255, FUNC(i8255_device::read), FUNC(i8255_device::write));
}

void shtngmst_state::shtngmst_io_map(address_map &map)
{
	system1_ppi_io_map(map);
	map(0x10, 0x10).mirror(0x01).rw(FUNC(shtngmst_state::gun_output_r), FUNC(shtngmst_state::gun_output_w));
	map(0x12, 0x12).mirror(0x01).r(FUNC(shtngmst_state::gun_trigger_r));
	map(0x18, 0x18).mirror(0x03).portr("SW12");
	map(0x1c, 0x1c).mirror(0x02).portr("GUNX");
	map(0x1d, 0x1d).mirror(0x02).portr("GUNY");
}

// I/O map for systems with a Z80 PIO chip
void system1_state::system1_pio_io_map(address_map &map)
{
	map.global_mask(0x1f);
	map(0x00, 0x00).mirror(0x03).portr("P1");
	map(0x04, 0x04).mirror(0x03).portr("P2");
	map(0x08, 0x08).mirror(0x03).portr("SYSTEM");
	map(0x0c, 0x0c).mirror(0x02).portr("SWA");    // DIP2
	map(0x0d, 0x0d).mirror(0x02).portr("SWB");    // DIP1 some games read it from here...
	map(0x10, 0x10).mirror(0x03).portr("SWB");    // DIP1 ... and some others from here but there are games which check BOTH!
	map(0x18, 0x1b).rw("pio", FUNC(z80pio_device::read), FUNC(z80pio_device::write));
}

void system1_state::blockgal_io_map(address_map &map)
{
	system1_pio_io_map(map);
	map(0x0c, 0x0c).mirror(0x02).unmapr();
	map(0x0d, 0x0d).mirror(0x02).portr("SWA");
}


/*************************************
 *
 *  Sound CPU address maps
 *
 *************************************/

void system1_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0x87ff).mirror(0x1800).ram();
	map(0xa000, 0xa000).mirror(0x1fff).w("sn1", FUNC(sn76489a_device::write));
	map(0xc000, 0xc000).mirror(0x1fff).w("sn2", FUNC(sn76489a_device::write));
	map(0xe000, 0xe000).mirror(0x1fff).r(FUNC(system1_state::sound_data_r));
}


/*************************************
 *
 *  MCU address maps
 *
 *************************************/

void system1_state::mcu_io_map(address_map &map)
{
	map(0x0000, 0xffff).rw(FUNC(system1_state::mcu_io_r), FUNC(system1_state::mcu_io_w));
}


/*************************************
 *
 *  Generic port definitions
 *
 *************************************/

static INPUT_PORTS_START( system1_generic )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SWA")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SWA:1,2,3,4")
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x05, "2 Coins/1 Credit 5/3 6/4" )
	PORT_DIPSETTING(    0x04, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x02, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) ) Not allowed by mame coinage sorting, but valid
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SWA:5,6,7,8")
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x50, "2 Coins/1 Credit 5/3 6/4" )
	PORT_DIPSETTING(    0x40, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x20, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0x30, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
//  PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) ) Not allowed by mame coinage sorting, but valid

	PORT_START("SWB")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SWB:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SWB:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SWB:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SWB:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SWB:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SWB:7" )
	// If you don't like the description, feel free to change it
	PORT_DIPNAME( 0x80, 0x80, "SW 0 Read From" )        PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, "Port $0D" )
	PORT_DIPSETTING(    0x00, "Port $10" )
INPUT_PORTS_END



/*************************************
 *
 *  Game-specific port definitions
 *
 *************************************/

static INPUT_PORTS_START( starjack )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:2,3")
	PORT_DIPSETTING(    0x06, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x38, 0x30, DEF_STR (Bonus_Life ) )   PORT_DIPLOCATION("SWB:4,5,6")
	PORT_DIPSETTING(    0x38, "Every 20k" )
	PORT_DIPSETTING(    0x28, "Every 30k" )
	PORT_DIPSETTING(    0x18, "Every 40k" )
	PORT_DIPSETTING(    0x08, "Every 50k" )
	PORT_DIPSETTING(    0x30, "20k, then every 30k" )
	PORT_DIPSETTING(    0x20, "30k, then every 40k" )
	PORT_DIPSETTING(    0x10, "40k, then every 50k" )
	PORT_DIPSETTING(    0x00, "50k, then every 60k" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( starjacks )
	PORT_INCLUDE( starjack )

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x08, 0x08, "Ship" )                  PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Single ) )
	PORT_DIPSETTING(    0x00, "Multi" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x30, "30k, then every 40k" )
	PORT_DIPSETTING(    0x20, "40k, then every 50k" )
	PORT_DIPSETTING(    0x10, "50k, then every 60k" )
	PORT_DIPSETTING(    0x00, "60k, then every 70k" )
INPUT_PORTS_END


static INPUT_PORTS_START( regulus )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

// Same as 'regulus', but no Allow Continue Dip Switch
static INPUT_PORTS_START( reguluso )
	PORT_INCLUDE( regulus )

	PORT_MODIFY("SWB")
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SWB:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( upndown )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no button 2

	PORT_MODIFY("P2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no button 2

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:2,3")
	PORT_DIPSETTING(    0x06, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:4,5,6")
	PORT_DIPSETTING(    0x38, "10000" )
	PORT_DIPSETTING(    0x30, "20000" )
	PORT_DIPSETTING(    0x28, "30000" )
	PORT_DIPSETTING(    0x20, "40000" )
	PORT_DIPSETTING(    0x18, "50000" )
	PORT_DIPSETTING(    0x10, "60000" )
	PORT_DIPSETTING(    0x08, "70000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mrviking )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_MODIFY("P2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x02, 0x00, "Maximum Credits" )       PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, "9" )
	PORT_DIPSETTING(    0x00, "99" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x30, "10k, 30k then every 30k" )
	PORT_DIPSETTING(    0x20, "20k, 40k then every 30k" )
	PORT_DIPSETTING(    0x10, "30k, then every 30k" )
	PORT_DIPSETTING(    0x00, "40k, then every 30k" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
INPUT_PORTS_END

/* Same as 'mrviking', but no "Maximum Credits" Dip Switch and "Difficulty" Dip Switch is
   handled by bit 7 instead of bit 6 (so bit 6 is unused) */
static INPUT_PORTS_START( mrvikingj )
	PORT_INCLUDE( mrviking )

	PORT_MODIFY("SWB")
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SWB:2" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SWB:7" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
INPUT_PORTS_END

static INPUT_PORTS_START( swat )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:2,3")
	PORT_DIPSETTING(    0x06, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:4,5,6")
	PORT_DIPSETTING(    0x38, "30000" )
	PORT_DIPSETTING(    0x30, "40000" )
	PORT_DIPSETTING(    0x28, "50000" )
	PORT_DIPSETTING(    0x20, "60000" )
	PORT_DIPSETTING(    0x18, "70000" )
	PORT_DIPSETTING(    0x10, "80000" )
	PORT_DIPSETTING(    0x08, "90000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SWB:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( flicky )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no button 2
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // only 2way inputs
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // only 2way inputs
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY

	PORT_MODIFY("P2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) // no button 2
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // only 2way inputs
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // only 2way inputs
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x30, "30000 80000 160000" )
	PORT_DIPSETTING(    0x20, "30000 100000 200000" )
	PORT_DIPSETTING(    0x10, "40000 120000 240000" )
	PORT_DIPSETTING(    0x00, "40000 140000 280000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
INPUT_PORTS_END

static INPUT_PORTS_START( flickyb )
	PORT_INCLUDE( flicky )

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
INPUT_PORTS_END

static INPUT_PORTS_START( flickys1 )
	PORT_INCLUDE( flicky )

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x08, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "5 (Infinite)" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x30, "80000" )
	PORT_DIPSETTING(    0x20, "160000" )
	PORT_DIPSETTING(    0x10, "240000" )
	PORT_DIPSETTING(    0x00, "320000" )
INPUT_PORTS_END

static INPUT_PORTS_START( flickys2 )
	PORT_INCLUDE( flickys1 )

	PORT_MODIFY("SWB")
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x02, "SWB:2" )
INPUT_PORTS_END

static INPUT_PORTS_START( wmatch )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY

	PORT_MODIFY("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY PORT_COCKTAIL

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) // TURN P1
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL // TURN P2

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Time" )                  PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x08, "Fast" )
	PORT_DIPSETTING(    0x04, "Faster" )
	PORT_DIPSETTING(    0x00, "Fastest" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
INPUT_PORTS_END

static INPUT_PORTS_START( bullfgt )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x30, "30000" )
	PORT_DIPSETTING(    0x20, "50000" )
	PORT_DIPSETTING(    0x10, "70000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END

static INPUT_PORTS_START( spatter )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x30, "40k, 120k and 480k" )
	PORT_DIPSETTING(    0x20, "50k and 200k" )
	PORT_DIPSETTING(    0x10, "100k only" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, "Reset Timer/Objects On Life Loss" )  PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( pitfall2 )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, "20000 50000" )
	PORT_DIPSETTING(    0x00, "30000 70000" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, "Time" )                  PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x00, "2 Minutes" )
	PORT_DIPSETTING(    0x40, "3 Minutes" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SWB:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( pitfall2u )
	PORT_INCLUDE( pitfall2 )

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:2,3")
	PORT_DIPSETTING(    0x06, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x18, 0x18, "Starting Stage" )        PORT_DIPLOCATION("SWB:4,5")
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "4" )
INPUT_PORTS_END

static INPUT_PORTS_START( seganinj )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_MODIFY("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "240" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, "20k 70k 120k 170k" )
	PORT_DIPSETTING(    0x00, "50k 100k 150k 200k" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
INPUT_PORTS_END

static INPUT_PORTS_START( imsorry )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x30, "30000" )
	PORT_DIPSETTING(    0x20, "40000" )
	PORT_DIPSETTING(    0x10, "50000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END

static INPUT_PORTS_START( teddybb )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_MODIFY("P2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "252" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x30, "100k 400k" )
	PORT_DIPSETTING(    0x20, "200k 600k" )
	PORT_DIPSETTING(    0x10, "400k 800k" )
	PORT_DIPSETTING(    0x00, "600k" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
INPUT_PORTS_END

static INPUT_PORTS_START( hvymetal )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_MODIFY("P2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x30, "50000 100000" )
	PORT_DIPSETTING(    0x20, "60000 120000" )
	PORT_DIPSETTING(    0x10, "70000 150000" )
	PORT_DIPSETTING(    0x00, "100000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( myhero )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )

	PORT_MODIFY("P2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x30, "30000" )
	PORT_DIPSETTING(    0x20, "50000" )
	PORT_DIPSETTING(    0x10, "70000" )
	PORT_DIPSETTING(    0x00, "90000" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
INPUT_PORTS_END

static INPUT_PORTS_START( 4dwarrio )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:2,3")
	PORT_DIPSETTING(    0x06, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:4,5,6")
	PORT_DIPSETTING(    0x38, "30000" )
	PORT_DIPSETTING(    0x30, "40000" )
	PORT_DIPSETTING(    0x28, "50000" )
	PORT_DIPSETTING(    0x20, "60000" )
	PORT_DIPSETTING(    0x18, "70000" )
	PORT_DIPSETTING(    0x10, "80000" )
	PORT_DIPSETTING(    0x08, "90000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
INPUT_PORTS_END

static INPUT_PORTS_START( brain )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SWB:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SWB:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SWB:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SWB:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( gardia )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x30, "5k, 20k and 30k" )
	PORT_DIPSETTING(    0x20, "10k, 25k and 50k" )
	PORT_DIPSETTING(    0x10, "15k, 30k and 60k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SWB:8") // Manual states "Always On"
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( raflesia )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x30, "20k, 70k and 120k" )
	PORT_DIPSETTING(    0x20, "30k, 80k and 150k" )
	PORT_DIPSETTING(    0x10, "50k, 100k and 200k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
INPUT_PORTS_END

static INPUT_PORTS_START( wboy )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // down - unused
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // up - unused
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY

	PORT_MODIFY("P2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // down - unused
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // up - unused
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, "30k 100k 170k 240k" )
	PORT_DIPSETTING(    0x00, "30k 120k 210k 300k" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SWB:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( wboy3 )
	PORT_INCLUDE( wboy )

	PORT_MODIFY("SWB") // DSW0
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
INPUT_PORTS_END

// same as wboy, additional Energy Consumption switch
static INPUT_PORTS_START( wbdeluxe )
	PORT_INCLUDE( wboy )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // Has to be 0 otherwise the game resets if you die after level 1.

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x80, 0x00, "Energy Consumption" )    PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x00, "Slow" )
	PORT_DIPSETTING(    0x80, "Fast" )
INPUT_PORTS_END

static INPUT_PORTS_START( wboyu )
	PORT_INCLUDE( wboy )

	PORT_MODIFY("SWA")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x06, 0x06, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:2,3")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x06, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SWB:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SWB:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SWB:6" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SWB:8" )

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SWA:1,2,3")
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SWA:4" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SWA:6" )
	PORT_DIPNAME( 0xc0, 0xc0, "Mode" )                  PORT_DIPLOCATION("SWA:7,8")
	PORT_DIPSETTING(    0xc0, "Normal Game" )
	PORT_DIPSETTING(    0x80, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0x40, "Test Mode" )
	PORT_DIPSETTING(    0x00, "Endless Game" )
INPUT_PORTS_END

static INPUT_PORTS_START( nob )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) // shot
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) // fly

	PORT_MODIFY("P2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL // shot
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL // fly

	PORT_MODIFY("SWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SWA:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SWA:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x30, 0x20, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWA:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "99" )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x08, "40k, 80k, 120k and 160k" )
	PORT_DIPSETTING(    0x0c, "50k, 100k and 150k" )
	PORT_DIPSETTING(    0x04, "60k, 120k and 180k" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Free_Play ) )    PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SWB:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( choplift )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SWA:1,2,3,4")
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x05, "2 Coins/1 Credit 5/3 6/4" )
	PORT_DIPSETTING(    0x04, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x02, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0x03, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SWA:5,6,7,8")
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x50, "2 Coins/1 Credit 5/3 6/4" )
	PORT_DIPSETTING(    0x40, "2 Coins/1 Credit 4/3" )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, "1 Coin/1 Credit 2/3" )
	PORT_DIPSETTING(    0x20, "1 Coin/1 Credit 4/5" )
	PORT_DIPSETTING(    0x30, "1 Coin/1 Credit 5/6" )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )

	PORT_MODIFY("SWA")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, "20k 70k 120k 170k" )
	PORT_DIPSETTING(    0x00, "50k 100k 150k 200k" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SWB:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SWB:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( shtngmst )
	PORT_INCLUDE( choplift )

	PORT_MODIFY("P1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) // no 2P start

	PORT_MODIFY("SWA")
	PORT_DIPNAME( 0x01, 0x01, "Shots Per Second" )      PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Infinite ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x30, "100k, 500k" )
	PORT_DIPSETTING(    0x20, "150k, 600k" )
	PORT_DIPSETTING(    0x10, "200k, 700k" )
	PORT_DIPSETTING(    0x00, "300k, 800k" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START("TRIGGER")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(shtngmst_state::gun_trigger), 0)

	PORT_START("GUNX")
	PORT_BIT( 0xff, 0x7f, IPT_LIGHTGUN_X ) PORT_CROSSHAIR(X, 1.0, 0.0, 0) PORT_MINMAX(0x00, 0xfe) PORT_SENSITIVITY(48) PORT_KEYDELTA(8)

	PORT_START("GUNY")
	PORT_BIT( 0xff, 0x80, IPT_LIGHTGUN_Y ) PORT_CROSSHAIR(Y, -1.0, 0.0, 0) PORT_MINMAX(0x10, 0xf0) PORT_SENSITIVITY(64) PORT_KEYDELTA(8) PORT_REVERSE

	PORT_START("SW12") // 2 rotary switches
	PORT_CONFNAME( 0x0f, 0x08, "Gun Y Offset" ) // SW 1
	PORT_CONFSETTING(    0x00, "-16" )
	PORT_CONFSETTING(    0x01, "-14" )
	PORT_CONFSETTING(    0x02, "-12" )
	PORT_CONFSETTING(    0x03, "-10" )
	PORT_CONFSETTING(    0x04, "-8" )
	PORT_CONFSETTING(    0x05, "-6" )
	PORT_CONFSETTING(    0x06, "-4" )
	PORT_CONFSETTING(    0x07, "-2" )
	PORT_CONFSETTING(    0x08, "0" )
	PORT_CONFSETTING(    0x09, "+2" )
	PORT_CONFSETTING(    0x0a, "+4" )
	PORT_CONFSETTING(    0x0b, "+6" )
	PORT_CONFSETTING(    0x0c, "+8" )
	PORT_CONFSETTING(    0x0d, "+10" )
	PORT_CONFSETTING(    0x0e, "+12" )
	PORT_CONFSETTING(    0x0f, "+14" )
	PORT_CONFNAME( 0xf0, 0x60, "Gun X Offset" ) // SW 2
	PORT_CONFSETTING(    0x00, "-16" )
	PORT_CONFSETTING(    0x10, "-14" )
	PORT_CONFSETTING(    0x20, "-12" )
	PORT_CONFSETTING(    0x30, "-10" )
	PORT_CONFSETTING(    0x40, "-8" )
	PORT_CONFSETTING(    0x50, "-6" )
	PORT_CONFSETTING(    0x60, "-4" )
	PORT_CONFSETTING(    0x70, "-2" )
	PORT_CONFSETTING(    0x80, "0" )
	PORT_CONFSETTING(    0x90, "+2" )
	PORT_CONFSETTING(    0xa0, "+4" )
	PORT_CONFSETTING(    0xb0, "+6" )
	PORT_CONFSETTING(    0xc0, "+8" )
	PORT_CONFSETTING(    0xd0, "+10" )
	PORT_CONFSETTING(    0xe0, "+12" )
	PORT_CONFSETTING(    0xf0, "+14" )
INPUT_PORTS_END

static INPUT_PORTS_START( wboysys2 )
	PORT_INCLUDE( choplift )

	PORT_MODIFY("P1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // down - unused
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // up - unused
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY

	PORT_MODIFY("P2")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) // down - unused
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) // up - unused
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_COCKTAIL

	PORT_MODIFY("SWA")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, "30k 100k 170k 240k" )
	PORT_DIPSETTING(    0x00, "30k 120k 210k 300k" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
INPUT_PORTS_END

/* Notes about the bootleg version (as this is the only "working" one) :

Coinage is almost the same as the other Sega games.

However, when you set DSW1 to 00, you enter a pseudo "free play" mode :

  - When you insert a coin, or press the "Service" button, you are given 2 credits
    and this number is NEVER incremented nor decremented
  - You are given 3 lives at start and this number is NEVER decremented
    (it can however be incremented depending on the "Bonus Life" Dip Switch)

If only one nibble is set to 0, it will give a standard 1C_1C.


There is an ingame bug with the "Bonus Life" Dip Switch, but I don't know if it's only
a "feature" of the bootleg :

  - Check routine at 0x2366, and you'll notice that 0xc02d (player 1) and 0xc02e (player 2)
    act like a "pointer" to the bonus life table (0x5ab6 or 0x5abb)
  - Once you get enough points, 1 life is added, and the pointer is incremented
  - There is however NO test to the limit of this pointer ! So, once you've got your 5th
    extra life at 150k, the pointed value will be 3 (= extra life at 30k), and as your
    score is over this value, you'll be given another extra life ... and so on ...


Bits 2 and 6 of DSW0 aren't tested in the game (I can't tell about the "test mode")


Useful addresses:

  - 0xc040 : credits (0x00-0x09)
  - 0xc019 : player 1 lives
  - 0xc021 : player 2 lives
  - 0xc018 : player 1 level (0x01-0x14)
  - 0xc020 : player 2 level (0x01-0x14)
  - 0xc02d : player 1 bonus life "pointer"
  - 0xc02e : player 1 bonus life "pointer"
  - 0xc01a - 0xc01c : player 1 score (BCD coded - LSB first)
  - 0xc022 - 0xc024 : player 1 score (BCD coded - LSB first)

  - 0xc050 : when == 01, "free play" mode
  - 0xc00c : when == 01, you end the level

*/

static INPUT_PORTS_START( blockgal )
	PORT_INCLUDE( system1_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(60) PORT_KEYDELTA(15) PORT_REVERSE

	PORT_MODIFY("P2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(60) PORT_KEYDELTA(15) PORT_REVERSE PORT_COCKTAIL

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_MODIFY("SWB")
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, "10k 30k 60k 100k 150k" )
	PORT_DIPSETTING(    0x00, "30k 50k 100k 200k 300k" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SWB:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( blockgalb )
	PORT_INCLUDE( choplift )

	PORT_MODIFY("P1")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(60) PORT_KEYDELTA(15) PORT_REVERSE

	PORT_MODIFY("P2")
	PORT_BIT( 0xff, 0x00, IPT_DIAL ) PORT_SENSITIVITY(60) PORT_KEYDELTA(15) PORT_REVERSE PORT_COCKTAIL

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL

	PORT_MODIFY("SWA")
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, "10k 30k 60k 100k 150k" )
	PORT_DIPSETTING(    0x00, "30k 50k 100k 200k 300k" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SWB:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( tokisens )
	PORT_INCLUDE( choplift )

	PORT_MODIFY("SWA")
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SWB:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SWB:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SWB:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SWB:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( tokisensa )
	PORT_INCLUDE( tokisens )

	PORT_MODIFY("SWA")
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( wbml )
	PORT_INCLUDE( choplift )

	PORT_MODIFY("SWA")
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x0c, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) ) // starts with 2 coins inserted
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, "30000 100000 200000" )
	PORT_DIPSETTING(    0x00, "50000 150000 250000" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x40, 0x40, "Test Mode" )             PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SWB:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( dakkochn )
	PORT_INCLUDE( choplift )

	PORT_MODIFY("P1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(dakkochn_state::mux_data_r))

	PORT_MODIFY("P2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_CUSTOM_MEMBER(FUNC(dakkochn_state::mux_status_r))

	PORT_MODIFY("SYSTEM")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED ) // start 1 & 2 not connected.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )

	// TODO: Dip-Switches
	PORT_MODIFY("SWA")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SWB:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SWB:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SWB:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SWB:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SWB:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SWB:8" )

	PORT_MODIFY("SWB")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SWA:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SWA:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SWA:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SWA:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SWA:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SWA:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SWA:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SWA:8" )

	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY6")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/*

 To enter Test Mode all DIP Switches in DSW1 must be ON (example Difficulty = Easy)
 To get Infinite Lives all DIP Switches in DSW0 must be ON

*/
static INPUT_PORTS_START( ufosensi )
	PORT_INCLUDE( choplift )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_MODIFY("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL

	PORT_MODIFY("SWA")
	PORT_DIPNAME( 0x03, 0x01, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
//  PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Invulnerability" )       PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static GFXDECODE_START( gfx_system1 )
	GFXDECODE_ENTRY( "tiles", 0, charlayout, 0, 256 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

// original board with 64kbit ROMs and an 8255 PPI for outputs
void system1_state::sys1ppi(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, MASTER_CLOCK/5);
	m_maincpu->set_addrmap(AS_PROGRAM, &system1_state::system1_map);
	m_maincpu->set_addrmap(AS_IO, &system1_state::system1_ppi_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(system1_state::irq0_line_hold));
	m_maincpu->refresh_cb().set(FUNC(system1_state::adjust_cycles));

	Z80(config, m_soundcpu, SOUND_CLOCK/2);
	m_soundcpu->set_addrmap(AS_PROGRAM, &system1_state::sound_map);

	TIMER(config, "soundirq", 0).configure_scanline(FUNC(system1_state::soundirq_gen), "screen", 32, 64);

	config.set_maximum_quantum(attotime::from_hz(6000));

	I8255A(config, m_ppi8255);
	m_ppi8255->out_pa_callback().set(FUNC(system1_state::soundport_w));
	m_ppi8255->out_pb_callback().set(FUNC(system1_state::videomode_w));
	m_ppi8255->out_pc_callback().set(FUNC(system1_state::sound_control_w));
	m_ppi8255->tri_pa_callback().set_constant(0x00);
	m_ppi8255->tri_pb_callback().set_constant(0x40);
	m_ppi8255->tri_pc_callback().set_constant(0x80);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_video_attributes(VIDEO_ALWAYS_UPDATE); // needed for proper hardware collisions
	m_screen->set_raw(MASTER_CLOCK/2, 640, 0, 512, 260, 0, 224);
	m_screen->set_screen_update(FUNC(system1_state::screen_update_system1));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_system1);
	PALETTE(config, m_palette, FUNC(system1_state::system1_palette)).set_entries(2048, 256);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	// 1st SN audio output actually goes to 2nd SN audio input (pin 9),
	// with a resistor in between, so the volume is lowered a bit.
	SN76489A(config, m_sn[0], SOUND_CLOCK/4).add_route(ALL_OUTPUTS, "mono", 0.40);

	// 2nd SN's clock is selectable via jumper
	SN76489A(config, m_sn[1], SOUND_CLOCK/2).add_route(ALL_OUTPUTS, "mono", 0.60);

	input_merger_device &sn_ready(INPUT_MERGER_ANY_LOW(config, "sn_ready"));
	sn_ready.output_handler().set_inputline(m_soundcpu, Z80_INPUT_LINE_WAIT);

	m_sn[0]->ready_cb().set("sn_ready", FUNC(input_merger_device::in_w<0>));
	m_sn[1]->ready_cb().set("sn_ready", FUNC(input_merger_device::in_w<1>));
}

// reduced visible area for scrolling games
void system1_state::sys1ppis(machine_config &config)
{
	sys1ppi(config);
	m_screen->set_visarea(2*(0*8+8), 2*(32*8-1-8), 0*8, 28*8-1);
}

// revised board with 128kbit ROMs and a Z80 PIO for outputs
void system1_state::sys1pio(machine_config &config)
{
	sys1ppi(config);

	m_maincpu->set_addrmap(AS_IO, &system1_state::system1_pio_io_map);

	config.device_remove("ppi8255");

	Z80PIO(config, m_pio, MASTER_CLOCK);
	m_pio->out_pa_callback().set(FUNC(system1_state::soundport_w));
	m_pio->out_ardy_callback().set_inputline(m_soundcpu, INPUT_LINE_NMI);
	m_pio->out_pb_callback().set(FUNC(system1_state::videomode_w));
}

// reduced visible area for scrolling games
void system1_state::sys1pios(machine_config &config)
{
	sys1pio(config);
	m_screen->set_visarea(2*(0*8+8), 2*(32*8-1-8), 0*8, 28*8-1);
}

// this describes the additional 8751 MCU when present
void system1_state::mcu(machine_config &config)
{
	// basic machine hardware
	m_maincpu->remove_vblank_int();

	I8751(config, m_mcu, SOUND_CLOCK);
	m_mcu->set_addrmap(AS_IO, &system1_state::mcu_io_map);
	m_mcu->port_out_cb<1>().set(FUNC(system1_state::mcu_control_w));

	config.set_maximum_quantum(attotime::from_hz(m_maincpu->clock() / 16));

	// This interrupt is driven by pin 15 of a PAL16R4 (315-5138 on Choplifter), based on the vertical count.
	// The actual duty cycle likely differs from VBLANK, which is another output from the same PAL.
	m_screen->screen_vblank().set_inputline("mcu", MCS51_INT0_LINE);

	// bus controller INTACK pin clocks MCU T0
	m_maincpu->set_irq_acknowledge_callback(FUNC(system1_state::mcu_t0_callback));
}

// System 2 video
void system1_state::sys2(machine_config &config)
{
	sys1ppi(config);

	m_ppi8255->out_pc_callback().set(FUNC(system1_state::sound_control_w)).exor(0x01);

	// video hardware
	MCFG_VIDEO_START_OVERRIDE(system1_state,system2)
	m_screen->set_screen_update(FUNC(system1_state::screen_update_system2));
}

void system1_state::sys2x(machine_config &config)
{
	sys2(config);
	MC8123(config.replace(), m_maincpu, MASTER_CLOCK/5);
	encrypted_sys2_mc8123_maps(config);
}

// System 2 with rowscroll
void system1_state::sys2row(machine_config &config)
{
	sys2(config);
	m_screen->set_screen_update(FUNC(system1_state::screen_update_system2_rowscroll));
}

// address map configs
void system1_state::encrypted_sys1ppi_maps(machine_config &config)
{
	m_maincpu->set_addrmap(AS_PROGRAM, &system1_state::system1_map);
	m_maincpu->set_addrmap(AS_OPCODES, &system1_state::decrypted_opcodes_map);
	m_maincpu->set_addrmap(AS_IO, &system1_state::system1_ppi_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(system1_state::irq0_line_hold));
	m_maincpu->refresh_cb().set(FUNC(system1_state::adjust_cycles));
}

void system1_state::encrypted_sys1pio_maps(machine_config &config)
{
	m_maincpu->set_addrmap(AS_PROGRAM, &system1_state::system1_map);
	m_maincpu->set_addrmap(AS_OPCODES, &system1_state::decrypted_opcodes_map);
	m_maincpu->set_addrmap(AS_IO, &system1_state::system1_pio_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(system1_state::irq0_line_hold));
	m_maincpu->refresh_cb().set(FUNC(system1_state::adjust_cycles));
}

void system1_state::encrypted_sys2_mc8123_maps(machine_config &config)
{
	m_maincpu->set_addrmap(AS_PROGRAM, &system1_state::system1_map);
	m_maincpu->set_addrmap(AS_OPCODES, &system1_state::banked_decrypted_opcodes_map);
	m_maincpu->set_addrmap(AS_IO, &system1_state::system1_ppi_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(system1_state::irq0_line_hold));
	m_maincpu->refresh_cb().set(FUNC(system1_state::adjust_cycles));
}


// game-specific PPI-based System 1

void system1_state::starjack(machine_config &config)
{
	sys1ppis(config);

	// lower sound hw clocks
	m_soundcpu->set_clock(MASTER_CLOCK / 8);
	m_sn[0]->set_clock(MASTER_CLOCK / 8);
	m_sn[1]->set_clock(MASTER_CLOCK / 16);
}

void system1_state::upndown(machine_config &config)
{
	sys1ppi(config);
	segacrpt_z80_device &z80(SEGA_315_5098(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1ppi_maps(config);
	z80.set_decrypted_tag(":decrypted_opcodes");

	// lower sound hw clocks (does not apply to unencrypted set)
	m_soundcpu->set_clock(MASTER_CLOCK / 8);
	m_sn[0]->set_clock(MASTER_CLOCK / 8);
	m_sn[1]->set_clock(MASTER_CLOCK / 8);
}

void system1_state::regulus(machine_config &config)
{
	sys1ppi(config);
	segacrpt_z80_device &z80(SEGA_315_5033(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1ppi_maps(config);
	z80.set_decrypted_tag(":decrypted_opcodes");
}

void system1_state::mrviking(machine_config &config)
{
	sys1ppis(config);
	segacrpt_z80_device &z80(SEGA_315_5041(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1ppi_maps(config);
	z80.set_decrypted_tag(":decrypted_opcodes");
}

void system1_state::swat(machine_config &config)
{
	sys1ppi(config);
	segacrpt_z80_device &z80(SEGA_315_5048(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1ppi_maps(config);
	z80.set_decrypted_tag(":decrypted_opcodes");
}

void system1_state::flickyo(machine_config &config)
{
	sys1ppi(config);
	segacrpt_z80_device &z80(SEGA_315_5051(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1ppi_maps(config);
	z80.set_decrypted_tag(":decrypted_opcodes");
}

void system1_state::wmatch(machine_config &config)
{
	sys1ppis(config);
	segacrpt_z80_device &z80(SEGA_315_5064(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1ppi_maps(config);
	z80.set_decrypted_tag(":decrypted_opcodes");
}

void system1_state::bullfgt(machine_config &config)
{
	sys1ppi(config);
	segacrpt_z80_device &z80(SEGA_315_5065(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1ppi_maps(config);
	z80.set_decrypted_tag(":decrypted_opcodes");
}

void system1_state::wboy2(machine_config &config)
{
	sys1ppi(config);
	segacrp2_z80_device &z80(SEGA_315_5178(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1ppi_maps(config);
	z80.set_decrypted_tag(m_decrypted_opcodes);
}

void system1_state::wboy6(machine_config &config)
{
	sys1ppi(config);
	segacrp2_z80_device &z80(SEGA_315_5179(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1ppi_maps(config);
	z80.set_decrypted_tag(m_decrypted_opcodes);
}

void system1_state::nob(machine_config &config)
{
	nobb(config);

	// basic machine hardware
	I8751(config, m_mcu, SOUND_CLOCK);
	m_mcu->port_in_cb<0>().set(FUNC(system1_state::nob_mcu_latch_r));
	m_mcu->port_out_cb<0>().set(FUNC(system1_state::nob_mcu_latch_w));
	m_mcu->port_out_cb<1>().set(FUNC(system1_state::nob_mcu_status_w));
	m_mcu->port_out_cb<2>().set(FUNC(system1_state::nob_mcu_control_p2_w));
}

void system1_state::nobb(machine_config &config)
{
	sys1ppi(config);

	// alternate program map with RAM/collision swapped
	m_maincpu->set_addrmap(AS_PROGRAM, &system1_state::nobo_map);

	m_sn[1]->set_clock(SOUND_CLOCK / 4);
}


// game-specific PIO-based System 1

void system1_state::flicky(machine_config &config)
{
	sys1pio(config);
	segacrpt_z80_device &z80(SEGA_315_5051(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1pio_maps(config);
	z80.set_decrypted_tag(":decrypted_opcodes");
}

void system1_state::thetogyu(machine_config &config)
{
	sys1pio(config);
	segacrpt_z80_device &z80(SEGA_315_5065(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1pio_maps(config);
	z80.set_decrypted_tag(":decrypted_opcodes");
}

void system1_state::spatter(machine_config &config)
{
	sys1pios(config);
	segacrpt_z80_device &z80(SEGA_315_5096(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1pio_maps(config);
	z80.set_decrypted_tag(":decrypted_opcodes");
}

void system1_state::spattera(machine_config &config)
{
	sys1pios(config);
	segacrpt_z80_device &z80(SEGA_315_5099(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1pio_maps(config);
	z80.set_decrypted_tag(":decrypted_opcodes");
}
void system1_state::pitfall2(machine_config &config)
{
	sys1pio(config);
	segacrpt_z80_device &z80(SEGA_315_5093(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1pio_maps(config);
	z80.set_decrypted_tag(":decrypted_opcodes");
}

void system1_state::seganinj(machine_config &config)
{
	sys1pio(config);
	segacrpt_z80_device &z80(SEGA_315_5102(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1pio_maps(config);
	z80.set_decrypted_tag(":decrypted_opcodes");
}

void system1_state::seganinja(machine_config &config)
{
	sys1pio(config);
	segacrpt_z80_device &z80(SEGA_315_5133(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1pio_maps(config);
	z80.set_decrypted_tag(":decrypted_opcodes");
}

void system1_state::nprinceso(machine_config &config)
{
	sys1pio(config);
	segacrpt_z80_device &z80(SEGA_315_5098(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1pio_maps(config);
	z80.set_decrypted_tag(":decrypted_opcodes");
}

void system1_state::imsorry(machine_config &config)
{
	sys1pio(config);
	segacrpt_z80_device &z80(SEGA_315_5110(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1pio_maps(config);
	z80.set_decrypted_tag(":decrypted_opcodes");
}

void system1_state::teddybb(machine_config &config)
{
	sys1pio(config);
	segacrpt_z80_device &z80(SEGA_315_5155(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1pio_maps(config);
	z80.set_decrypted_tag(":decrypted_opcodes");
}

void system1_state::teddybboa(machine_config &config)
{
	sys1pio(config);
	segacrpt_z80_device &z80(SEGA_315_5111(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1pio_maps(config);
	z80.set_decrypted_tag(":decrypted_opcodes");
}

void system1_state::myheroj(machine_config &config)
{
	sys1pio(config);
	segacrpt_z80_device &z80(SEGA_315_5132(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1pio_maps(config);
	z80.set_decrypted_tag(":decrypted_opcodes");
}

void system1_state::_4dwarrio(machine_config &config)
{
	sys1pio(config);
	segacrp2_z80_device &z80(SEGA_315_5162(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1pio_maps(config);
	z80.set_decrypted_tag(m_decrypted_opcodes);
}

void system1_state::wboy(machine_config &config)
{
	sys1pio(config);
	segacrp2_z80_device &z80(SEGA_315_5177(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1pio_maps(config);
	z80.set_decrypted_tag(m_decrypted_opcodes);
}

void system1_state::wboyo(machine_config &config)
{
	sys1pio(config);
	segacrpt_z80_device &z80(SEGA_315_5135(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1pio_maps(config);
	z80.set_decrypted_tag(":decrypted_opcodes");
}

void system1_state::blockgal(machine_config &config)
{
	sys1pio(config);
	MC8123(config.replace(), m_maincpu, MASTER_CLOCK/5);
	encrypted_sys1pio_maps(config);
	m_maincpu->set_addrmap(AS_IO, &system1_state::blockgal_io_map);
}

void system1_state::gardia(machine_config &config)
{
	sys1pio(config);
	segacrp2_z80_device &z80(SEGA_317_0006(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1pio_maps(config);
	z80.set_decrypted_tag(m_decrypted_opcodes);

	m_sn[1]->set_clock(SOUND_CLOCK / 4);
}


// game-specific System 2

void system1_state::choplift(machine_config &config)
{
	sys2row(config);
	mcu(config);
}

void shtngmst_state::shtngmst(machine_config &config)
{
	sys2(config);
	mcu(config);
	m_maincpu->set_addrmap(AS_IO, &shtngmst_state::shtngmst_io_map);
}

void system1_state::gardiab(machine_config &config)
{
	sys2(config);
	segacrp2_z80_device &z80(SEGA_317_0007(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1ppi_maps(config);
	z80.set_decrypted_tag(m_decrypted_opcodes);

	m_sn[1]->set_clock(SOUND_CLOCK / 4);
}

void system1_state::gardiaj(machine_config &config)
{
	sys2(config);
	segacrp2_z80_device &z80(SEGA_317_0006(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1ppi_maps(config);
	z80.set_decrypted_tag(m_decrypted_opcodes);

	m_sn[1]->set_clock(SOUND_CLOCK / 4);
}

void system1_state::wboysys2(machine_config &config)
{
	sys2(config);
	segacrp2_z80_device &z80(SEGA_315_5177(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1ppi_maps(config);
	z80.set_decrypted_tag(m_decrypted_opcodes);
}

void system1_state::wboysys2a(machine_config &config)
{
	sys2(config);
	segacrp2_z80_device &z80(SEGA_315_5176(config.replace(), m_maincpu, MASTER_CLOCK/5));
	encrypted_sys1ppi_maps(config);
	z80.set_decrypted_tag(m_decrypted_opcodes);
}

void system1_state::wbmlb(machine_config &config)
{
	sys2(config);
	m_maincpu->set_addrmap(AS_OPCODES, &system1_state::banked_decrypted_opcodes_map);
}

void system1_state::blockgalb(machine_config &config)
{
	sys2(config);
	m_maincpu->set_addrmap(AS_OPCODES, &system1_state::decrypted_opcodes_map);
}

void system1_state::ufosensi(machine_config &config)
{
	sys2row(config);
	MC8123(config.replace(), m_maincpu, MASTER_CLOCK/5);
	encrypted_sys2_mc8123_maps(config);
}

void system1_state::ufosensib(machine_config &config)
{
	sys2row(config);
	m_maincpu->set_addrmap(AS_OPCODES, &system1_state::banked_decrypted_opcodes_map);
}



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

// Since the standard System 1 PROM has part # 5317, Star Jacker, whose first
// ROM is #5318, is probably the first or second System 1 game produced
ROM_START( starjack )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-5320b.129",  0x0000, 0x2000, CRC(7ab72ecd) SHA1(28d3f87851cccc94a86eb0217893de0baf8e62fd) )
	ROM_LOAD( "epr-5321a.130",  0x2000, 0x2000, CRC(38b99050) SHA1(79fd23bb7db577d1dbf1b50503085eccdd17b98c) )
	ROM_LOAD( "epr-5322a.131",  0x4000, 0x2000, CRC(103a595b) SHA1(6bb8a063279c93341ff472351b79c92795845f74) )
	ROM_LOAD( "epr-5323.132",   0x6000, 0x2000, CRC(46af0d58) SHA1(6cfa288e28e3b415db5ef3cef8e8849259234af9) )
	ROM_LOAD( "epr-5324.133",   0x8000, 0x2000, CRC(1e89efe2) SHA1(d36ef8f176d5e44884d3d0b9af81be6f7fbd0cde) )
	ROM_LOAD( "epr-5325.134",   0xa000, 0x2000, CRC(d6e379a1) SHA1(27362b3e10d9d4235647eadb9cd1404700a8fb49) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5332.3",     0x0000, 0x2000, CRC(7a72ab3d) SHA1(4a6ad09949a438562d7043532d628cefdbb5a2fe) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-5331.82",    0x0000, 0x2000, CRC(251d898f) SHA1(353067a75d583d5f53ce2f473b52a35dd912639f) )
	ROM_LOAD( "epr-5330.65",    0x2000, 0x2000, CRC(eb048745) SHA1(a2e90d20a07608f43273e84d1eb22f195b19626c) )
	ROM_LOAD( "epr-5329.81",    0x4000, 0x2000, CRC(3e8bcaed) SHA1(6d19543427b9c4d8d8f5ea0872cdf8cc4fe5018c) )
	ROM_LOAD( "epr-5328.64",    0x6000, 0x2000, CRC(9ed7849f) SHA1(cc30d144ff70539bbc82c848c154e156a33b638c) )
	ROM_LOAD( "epr-5327.80",    0x8000, 0x2000, CRC(79e92cb1) SHA1(03124ce123684b8469cf42be6ed5f0fffa64c480) )
	ROM_LOAD( "epr-5326.63",    0xa000, 0x2000, CRC(ba7e2b47) SHA1(bc7528456fe8dee9aa21212aa996fc347c5d55b4) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "epr-5318.86",    0x0000, 0x4000, CRC(6f2e1fd3) SHA1(326d538551245fce67d0fdba85975e27093b7a93) )
	ROM_LOAD( "epr-5319.93",    0x4000, 0x4000, CRC(ebee4999) SHA1(bb331be270dc1da63699533d9f02d73ce28f1bc5) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.106",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( starjacks )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "star_jacker_a1_ic29.129", 0x0000, 0x2000, CRC(59a22a1f) SHA1(4827b537f8df04429ed53c2119c67a32efcf04a2) )
	ROM_LOAD( "star_jacker_a1_ic30.130", 0x2000, 0x2000, CRC(7f4597dc) SHA1(7574853fc2e38880f8493cf628100a890f7aa7dc) )
	ROM_LOAD( "star_jacker_a1_ic31.131", 0x4000, 0x2000, CRC(6074c046) SHA1(5d2bd679d6a13a6c3b203662ce5496b801383db9) )
	ROM_LOAD( "star_jacker_a1_ic32.132", 0x6000, 0x2000, CRC(1c48a3fa) SHA1(4a2e7798600bc4a5fd68533083547212d148d347) )
	ROM_LOAD( "star_jacker_a1_ic33.133", 0x8000, 0x2000, CRC(7598bd51) SHA1(0c18b83dc7874aefcd94593ffbe2b50cc0329367) )
	ROM_LOAD( "star_jacker_a1_ic34.134", 0xa000, 0x2000, CRC(f66fa604) SHA1(d7a81920217fcf7a687ba5e2d10abad5c78085d2) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "star_jacker_a1_ic3.3",    0x0000, 0x2000, CRC(7a72ab3d) SHA1(4a6ad09949a438562d7043532d628cefdbb5a2fe) ) // same as EPR-5332

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "star_jacker_a1_ic82.82",  0x0000, 0x2000, CRC(251d898f) SHA1(353067a75d583d5f53ce2f473b52a35dd912639f) )
	ROM_LOAD( "star_jacker_a1_ic65.65",  0x2000, 0x2000, CRC(0ab1893c) SHA1(97877f5d8be7a7b80bbf9fe8dae2acd47c411d64) )
	ROM_LOAD( "epr-5456.81",             0x4000, 0x2000, CRC(3e8bcaed) SHA1(6d19543427b9c4d8d8f5ea0872cdf8cc4fe5018c) )
	ROM_LOAD( "epr-5455.64",             0x6000, 0x2000, CRC(7f628ae6) SHA1(f859a505b543382b42a478c8ae5cd90f3ea2bc2c) ) // Also found labeled as STAR JACKER A1 IC64
	ROM_LOAD( "epr-5454.80",             0x8000, 0x2000, CRC(79e92cb1) SHA1(03124ce123684b8469cf42be6ed5f0fffa64c480) )
	ROM_LOAD( "epr-5453.63",             0xa000, 0x2000, CRC(5bcb253e) SHA1(8c34a8377344940bcfb2495bfda3ffc6794f261b) ) // Also found labeled as STAR JACKER A1 IC63

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "star_jacker_a1_ic86.86",  0x0000, 0x4000, CRC(6f2e1fd3) SHA1(326d538551245fce67d0fdba85975e27093b7a93) )
	ROM_LOAD( "epr-5446.93",             0x4000, 0x4000, CRC(07987244) SHA1(8468b95684b1f62c6de6af1b1d405bfb333e4e26) ) // Also found labeled as STAR JACKER A1 IC93

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.106",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END


ROM_START( upndown )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr5516a.129",   0x0000, 0x2000, CRC(038c82da) SHA1(b7f403068ed9f97a4b960fb8863615892bb770ed) ) // encrypted
	ROM_LOAD( "epr5517a.130",   0x2000, 0x2000, CRC(6930e1de) SHA1(8a5564c76e1fd20c8e5d95e5f538980e13c41744) ) // encrypted
	ROM_LOAD( "epr-5518.131",   0x4000, 0x2000, CRC(2a370c99) SHA1(3d1b2f1cf0d5d2d6369a33e5b3b460a3113d6a3e) ) // encrypted
	ROM_LOAD( "epr-5519.132",   0x6000, 0x2000, CRC(9d664a58) SHA1(84f2d012dac63e8d0de3935a76f5202539423a74) ) // encrypted
	ROM_LOAD( "epr-5520.133",   0x8000, 0x2000, CRC(208dfbdf) SHA1(eff0c91ce6c2c1f6e191bcbf9ae83dd377cbb408) )
	ROM_LOAD( "epr-5521.134",   0xa000, 0x2000, CRC(e7b8d87a) SHA1(3419318bf6d87b902433bfe3b92baf5e5bad7df3) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5535.3",     0x0000, 0x2000, CRC(cf4e4c45) SHA1(d14a204a9966d37f4b9f3ea4c1d371c9d04e750a) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-5527.82",    0x0000, 0x2000, CRC(b2d616f1) SHA1(c079136a5d73e1d55ddbad6efb5e7067d0ff412b) )
	ROM_LOAD( "epr-5526.65",    0x2000, 0x2000, CRC(8a8b33c2) SHA1(db796d5c4ab3f749287133eaf05818f89dc2afb7) )
	ROM_LOAD( "epr-5525.81",    0x4000, 0x2000, CRC(e749c5ef) SHA1(2022cbd42ff0177cdd661bb00b1004459b6af83a) )
	ROM_LOAD( "epr-5524.64",    0x6000, 0x2000, CRC(8b886952) SHA1(6a9c909d10ccb03a8af6fa9d8067946d60b91592) )
	ROM_LOAD( "epr-5523.80",    0x8000, 0x2000, CRC(dede35d9) SHA1(6c47fa433e16ccc3fff9347a4fe8f0165d20a3d2) )
	ROM_LOAD( "epr-5522.63",    0xa000, 0x2000, CRC(5e6d9dff) SHA1(4f18274f5dc349b99b3daec517ccf5ccbb932d1c) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "epr-5514.86",    0x0000, 0x4000, CRC(fcc0a88b) SHA1(ca7db3df10deb6720096e6c50eddd9b74c47f0a0) )
	ROM_LOAD( "epr-5515.93",    0x4000, 0x4000, CRC(60908838) SHA1(aedff8ce07ab16942037e5aff212652e51c19e71) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.106",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( upndownu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-5679.129",   0x0000, 0x2000, CRC(c4f2f9c2) SHA1(7904ffb46a2c3ef69b9784f343ff37d81bbee11d) )
	ROM_LOAD( "epr-5680.130",   0x2000, 0x2000, CRC(837f021c) SHA1(14cc846f03b71e0922689388a6757955cfd88bd8) )
	ROM_LOAD( "epr-5681.131",   0x4000, 0x2000, CRC(e1c7ff7e) SHA1(440dc8c18183612c32486c617f5d7f38fd804f0e) )
	ROM_LOAD( "epr-5682.132",   0x6000, 0x2000, CRC(4a5edc1e) SHA1(71f06d1c4a580fed07ad32c6d1f2d37d47ed95b1) )
	ROM_LOAD( "epr-5520.133",   0x8000, 0x2000, CRC(208dfbdf) SHA1(eff0c91ce6c2c1f6e191bcbf9ae83dd377cbb408) ) // epr-5683.133
	ROM_LOAD( "epr-5684.133",   0xa000, 0x2000, CRC(32fa95da) SHA1(ebe87d28dde6b8356d40572e9f2cd35ec240075f) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5528.3",     0x0000, 0x2000, CRC(00cd44ab) SHA1(7f5385aa0773681329a4759b0fa6f975e3de6755) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-5527.82",    0x0000, 0x2000, CRC(b2d616f1) SHA1(c079136a5d73e1d55ddbad6efb5e7067d0ff412b) )
	ROM_LOAD( "epr-5526.65",    0x2000, 0x2000, CRC(8a8b33c2) SHA1(db796d5c4ab3f749287133eaf05818f89dc2afb7) )
	ROM_LOAD( "epr-5525.81",    0x4000, 0x2000, CRC(e749c5ef) SHA1(2022cbd42ff0177cdd661bb00b1004459b6af83a) )
	ROM_LOAD( "epr-5524.64",    0x6000, 0x2000, CRC(8b886952) SHA1(6a9c909d10ccb03a8af6fa9d8067946d60b91592) )
	ROM_LOAD( "epr-5523.80",    0x8000, 0x2000, CRC(dede35d9) SHA1(6c47fa433e16ccc3fff9347a4fe8f0165d20a3d2) )
	ROM_LOAD( "epr-5522.63",    0xa000, 0x2000, CRC(5e6d9dff) SHA1(4f18274f5dc349b99b3daec517ccf5ccbb932d1c) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "epr-5514.86",    0x0000, 0x4000, CRC(fcc0a88b) SHA1(ca7db3df10deb6720096e6c50eddd9b74c47f0a0) )
	ROM_LOAD( "epr-5515.93",    0x4000, 0x4000, CRC(60908838) SHA1(aedff8ce07ab16942037e5aff212652e51c19e71) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.106",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END


ROM_START( regulus )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-5640a.129",  0x0000, 0x2000, CRC(dafb1528) SHA1(9140c5507bd931df3f8ef8d2910bc74f737b1a5a) ) // encrypted
	ROM_LOAD( "epr-5641a.130",  0x2000, 0x2000, CRC(0fcc850e) SHA1(d2d6b06bf1e2dc404aa5451cc9f1b919fb5be0f5) ) // encrypted
	ROM_LOAD( "epr-5642a.131",  0x4000, 0x2000, CRC(4feffa17) SHA1(9d9f4227c4e60a5cc53c369e7c9ce59ea8df3553) ) // encrypted
	ROM_LOAD( "epr-5643a.132",  0x6000, 0x2000, CRC(b8ac7eb4) SHA1(f96bcde021060a8c1c5270b73487e24d1893e8e5) ) // encrypted
	ROM_LOAD( "epr-5644.133",   0x8000, 0x2000, CRC(ffd05b7d) SHA1(6fe471548d227d834c012d5d148b1ea1c12dfd00) )
	ROM_LOAD( "epr-5645a.134",  0xa000, 0x2000, CRC(6b4bf77c) SHA1(0200efb58b85a6873db44ffa70c3c14dbca958a6) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5652.3",     0x0000, 0x2000, CRC(74edcb98) SHA1(bc181c73a6009ca723e715650adb920b77bd311c) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-5651.82",    0x0000, 0x2000, CRC(f07f3e82) SHA1(f86acf2de639ac89f80cdf627d1d6b5f5e4f1557) )
	ROM_LOAD( "epr-5650.65",    0x2000, 0x2000, CRC(84c1baa2) SHA1(27ba8e2bb820913e58cb029da9c18d35e67728b8) )
	ROM_LOAD( "epr-5649.81",    0x4000, 0x2000, CRC(6774c895) SHA1(28f74bcf1e6bc06db0984dcf86dd527e301b0c01) )
	ROM_LOAD( "epr-5648.64",    0x6000, 0x2000, CRC(0c69e92a) SHA1(1ee18562250468f8f09a3062705422c28c740674) )
	ROM_LOAD( "epr-5647.80",    0x8000, 0x2000, CRC(9330f7b5) SHA1(2c1be04de6ec652ea8a566eb0eb1a9bcb4c90e66) )
	ROM_LOAD( "epr-5646.63",    0xa000, 0x2000, CRC(4dfacbbc) SHA1(e34d1e1aaf3ae7a138e75df5dedebfb4acd79340) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "epr-5638.86",    0x0000, 0x4000, CRC(617363dd) SHA1(c8024f541086a8a940e21219fa4522646aeb365a) )
	ROM_LOAD( "epr-5639.93",    0x4000, 0x4000, CRC(a4ec5131) SHA1(033bf46d2625f99544a784fe3fa299cc1b1b48e1) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.106",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( reguluso )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-5640.129",   0x0000, 0x2000, CRC(8324d0d4) SHA1(204713938bc85e8b62c161d8ae00d087ecc9089c) ) // encrypted
	ROM_LOAD( "epr-5641.130",   0x2000, 0x2000, CRC(0a09f5c7) SHA1(0d45bff29442908b9f4111c89baea0326f0a9ec9) ) // encrypted
	ROM_LOAD( "epr-5642.131",   0x4000, 0x2000, CRC(ff27b2f6) SHA1(fe294a53deffe2d46afa444fdae213e9d8763316) ) // encrypted
	ROM_LOAD( "epr-5643.132",   0x6000, 0x2000, CRC(0d867df0) SHA1(adccc78072c0772ec20c0178a0be3426759900bf) ) // encrypted
	ROM_LOAD( "epr-5644.133",   0x8000, 0x2000, CRC(ffd05b7d) SHA1(6fe471548d227d834c012d5d148b1ea1c12dfd00) )
	ROM_LOAD( "epr-5645.134",   0xa000, 0x2000, CRC(57a2b4b4) SHA1(9de8f5948c7993f1b6d8bf7032f7fc3d9dff5c77) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5652.3",     0x0000, 0x2000, CRC(74edcb98) SHA1(bc181c73a6009ca723e715650adb920b77bd311c) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-5651.82",    0x0000, 0x2000, CRC(f07f3e82) SHA1(f86acf2de639ac89f80cdf627d1d6b5f5e4f1557) )
	ROM_LOAD( "epr-5650.65",    0x2000, 0x2000, CRC(84c1baa2) SHA1(27ba8e2bb820913e58cb029da9c18d35e67728b8) )
	ROM_LOAD( "epr-5649.81",    0x4000, 0x2000, CRC(6774c895) SHA1(28f74bcf1e6bc06db0984dcf86dd527e301b0c01) )
	ROM_LOAD( "epr-5648.64",    0x6000, 0x2000, CRC(0c69e92a) SHA1(1ee18562250468f8f09a3062705422c28c740674) )
	ROM_LOAD( "epr-5647.80",    0x8000, 0x2000, CRC(9330f7b5) SHA1(2c1be04de6ec652ea8a566eb0eb1a9bcb4c90e66) )
	ROM_LOAD( "epr-5646.63",    0xa000, 0x2000, CRC(4dfacbbc) SHA1(e34d1e1aaf3ae7a138e75df5dedebfb4acd79340) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "epr-5638.86",    0x0000, 0x4000, CRC(617363dd) SHA1(c8024f541086a8a940e21219fa4522646aeb365a) )
	ROM_LOAD( "epr-5639.93",    0x4000, 0x4000, CRC(a4ec5131) SHA1(033bf46d2625f99544a784fe3fa299cc1b1b48e1) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.106",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( regulusu ) // Sega game ID# 834-5328-02 REGULUS
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-5950.129",   0x0000, 0x2000, CRC(3b047b67) SHA1(0164cb919a50013f23568f59caff19ff2d0bf11f) )
	ROM_LOAD( "epr-5951.130",   0x2000, 0x2000, CRC(d66453ab) SHA1(9e339c716c646bd02bedbe27096b75f633554e7c) )
	ROM_LOAD( "epr-5952.131",   0x4000, 0x2000, CRC(f3d0158a) SHA1(9b6d8b2e0a0bec45bfbb9f8ccc728e18e909685f) )
	ROM_LOAD( "epr-5953.132",   0x6000, 0x2000, CRC(a9ad4f44) SHA1(1e051595aff34db06186542bcfc3849bc88eb5d4) )
	ROM_LOAD( "epr-5644.133",   0x8000, 0x2000, CRC(ffd05b7d) SHA1(6fe471548d227d834c012d5d148b1ea1c12dfd00) )
	ROM_LOAD( "epr-5955.134",   0xa000, 0x2000, CRC(65ddb2a3) SHA1(4f94eaac900da5ca512289e2339776b1139e03e1) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5652.3",     0x0000, 0x2000, CRC(74edcb98) SHA1(bc181c73a6009ca723e715650adb920b77bd311c) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-5651.82",    0x0000, 0x2000, CRC(f07f3e82) SHA1(f86acf2de639ac89f80cdf627d1d6b5f5e4f1557) )
	ROM_LOAD( "epr-5650.65",    0x2000, 0x2000, CRC(84c1baa2) SHA1(27ba8e2bb820913e58cb029da9c18d35e67728b8) )
	ROM_LOAD( "epr-5649.81",    0x4000, 0x2000, CRC(6774c895) SHA1(28f74bcf1e6bc06db0984dcf86dd527e301b0c01) )
	ROM_LOAD( "epr-5648.64",    0x6000, 0x2000, CRC(0c69e92a) SHA1(1ee18562250468f8f09a3062705422c28c740674) )
	ROM_LOAD( "epr-5647.80",    0x8000, 0x2000, CRC(9330f7b5) SHA1(2c1be04de6ec652ea8a566eb0eb1a9bcb4c90e66) ) // PCB pic shows this should be rev A
	ROM_LOAD( "epr-5646.63",    0xa000, 0x2000, CRC(4dfacbbc) SHA1(e34d1e1aaf3ae7a138e75df5dedebfb4acd79340) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "epr-5638.86",    0x0000, 0x4000, CRC(617363dd) SHA1(c8024f541086a8a940e21219fa4522646aeb365a) )
	ROM_LOAD( "epr-5639.93",    0x4000, 0x4000, CRC(a4ec5131) SHA1(033bf46d2625f99544a784fe3fa299cc1b1b48e1) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.106",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END


ROM_START( mrviking )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-5873.129",   0x0000, 0x2000, CRC(14d21624) SHA1(70e185d03e782be908e6b5c6342cf6a7ebae618c) ) // encrypted
	ROM_LOAD( "epr-5874.130",   0x2000, 0x2000, CRC(6df7de87) SHA1(c2200e0c2f322a08af10e9c2e9191d1c595801a4) ) // encrypted
	ROM_LOAD( "epr-5875.131",   0x4000, 0x2000, CRC(ac226100) SHA1(11568db9fbca44013eeb0035c0a0a67d6dd18d00) ) // encrypted
	ROM_LOAD( "epr-5876.132",   0x6000, 0x2000, CRC(e77db1dc) SHA1(7b1aa19a16fb44f6c69cf053e2e10e5179416796) ) // encrypted
	ROM_LOAD( "epr-5755.133",   0x8000, 0x2000, CRC(edd62ae1) SHA1(9648f1ae3033c30ed8ab8d9c87b111756dab7b5e) )
	ROM_LOAD( "epr-5756.134",   0xa000, 0x2000, CRC(11974040) SHA1(a0904d19d06fb5ef5eb6da0dc4efe556bc29b33e) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5763.3",     0x0000, 0x2000, CRC(d712280d) SHA1(8393dfb57d9af22b3280ecaef736b6f9d856dbee) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-5762.82",    0x0000, 0x2000, CRC(4a91d08a) SHA1(4687ecc4061719fca5f85b2b290ebb7ced15ee5b) )
	ROM_LOAD( "epr-5761.65",    0x2000, 0x2000, CRC(f7d61b65) SHA1(a7a992f52406413e931945be60b35175f8aea6c2) )
	ROM_LOAD( "epr-5760.81",    0x4000, 0x2000, CRC(95045820) SHA1(d1848fc4f3d66603d0e8217373a37148aa2eeef5) )
	ROM_LOAD( "epr-5759.64",    0x6000, 0x2000, CRC(5f9bae4e) SHA1(6fff6086a96be6aa28bec05d1c94c257bb29ef1e) )
	ROM_LOAD( "epr-5758.80",    0x8000, 0x2000, CRC(808ee706) SHA1(d38ca7c6f36db6e35a3ce87bacdd70f293f23104) )
	ROM_LOAD( "epr-5757.63",    0xa000, 0x2000, CRC(480f7074) SHA1(c54a1fa02e312676658d7c5392a5a841bdb15d44) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "epr-5749.86",    0x0000, 0x4000, CRC(e24682cd) SHA1(3f626f3e5e2db486ccf727e9869ab488643b4a8c) )
	ROM_LOAD( "epr-5750.93",    0x4000, 0x4000, CRC(6564d1ad) SHA1(f246afee7e73bc30054b0e5dcb83fa0edd2d2164) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.106",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( mrvikingj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-5751.129",   0x0000, 0x2000, CRC(ae97a4c5) SHA1(12edd757bd5b00d42ada1e10c43817f71cfe77dc) ) // encrypted
	ROM_LOAD( "epr-5752.130",   0x2000, 0x2000, CRC(d48e6726) SHA1(934b5e7568c85005c5ec40d75e49727a18562d50) ) // encrypted
	ROM_LOAD( "epr-5753.131",   0x4000, 0x2000, CRC(28c60887) SHA1(9673335586221336c3373f5d7c8ae4fc11cc4b7f) ) // encrypted
	ROM_LOAD( "epr-5754.132",   0x6000, 0x2000, CRC(1f47ed02) SHA1(d1147cd29fb342111f4f20a1d1d03263dce478f3) ) // encrypted
	ROM_LOAD( "epr-5755.133",   0x8000, 0x2000, CRC(edd62ae1) SHA1(9648f1ae3033c30ed8ab8d9c87b111756dab7b5e) )
	ROM_LOAD( "epr-5756.134",   0xa000, 0x2000, CRC(11974040) SHA1(a0904d19d06fb5ef5eb6da0dc4efe556bc29b33e) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5763.3",     0x0000, 0x2000, CRC(d712280d) SHA1(8393dfb57d9af22b3280ecaef736b6f9d856dbee) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-5762.82",    0x0000, 0x2000, CRC(4a91d08a) SHA1(4687ecc4061719fca5f85b2b290ebb7ced15ee5b) )
	ROM_LOAD( "epr-5761.65",    0x2000, 0x2000, CRC(f7d61b65) SHA1(a7a992f52406413e931945be60b35175f8aea6c2) )
	ROM_LOAD( "epr-5760.81",    0x4000, 0x2000, CRC(95045820) SHA1(d1848fc4f3d66603d0e8217373a37148aa2eeef5) )
	ROM_LOAD( "epr-5759.64",    0x6000, 0x2000, CRC(5f9bae4e) SHA1(6fff6086a96be6aa28bec05d1c94c257bb29ef1e) )
	ROM_LOAD( "epr-5758.80",    0x8000, 0x2000, CRC(808ee706) SHA1(d38ca7c6f36db6e35a3ce87bacdd70f293f23104) )
	ROM_LOAD( "epr-5757.63",    0xa000, 0x2000, CRC(480f7074) SHA1(c54a1fa02e312676658d7c5392a5a841bdb15d44) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "epr-5749.86",    0x0000, 0x4000, CRC(e24682cd) SHA1(3f626f3e5e2db486ccf727e9869ab488643b4a8c) )
	ROM_LOAD( "epr-5750.93",    0x4000, 0x4000, CRC(6564d1ad) SHA1(f246afee7e73bc30054b0e5dcb83fa0edd2d2164) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.106",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END


ROM_START( swat ) // Sega game ID# 834-5388 SWAT
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr5807b.129",   0x0000, 0x2000, CRC(93db9c9f) SHA1(56e9d9a33f04b4d5971c0db24cc8719a52e64678) ) // encrypted
	ROM_LOAD( "epr-5808.130",   0x2000, 0x2000, CRC(67116665) SHA1(e8aa72f2835d38367be5e8a9313e51b64f452ee7) ) // encrypted
	ROM_LOAD( "epr-5809.131",   0x4000, 0x2000, CRC(fd792fc9) SHA1(a0b4f0c2e537bd16f7345590da00f2622947d7e4) ) // encrypted
	ROM_LOAD( "epr-5810.132",   0x6000, 0x2000, CRC(dc2b279d) SHA1(e740cbe239d379705fdffb3e500d6f5a2fece2e2) ) // encrypted
	ROM_LOAD( "epr-5811.133",   0x8000, 0x2000, CRC(093e3ab1) SHA1(abf1f23dc26a7518357d0c1749e869b539c3bbed) )
	ROM_LOAD( "epr-5812.134",   0xa000, 0x2000, CRC(5bfd692f) SHA1(adc8dcf643d8d0b0a1d0dda0494567263ea11a00) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5819.3",     0x0000, 0x2000, CRC(f6afd0fd) SHA1(06062648b9ebc70b4b5c30b043f537adc0052047) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-5818.82",    0x0000, 0x2000, CRC(b22033d9) SHA1(ad217cd8dad178f3f2f1fd44a58adcc4887fb6b7) )
	ROM_LOAD( "epr-5817.65",    0x2000, 0x2000, CRC(fd942797) SHA1(da7378e8d12cc2970df2efa075c944c79b3b74d2) )
	ROM_LOAD( "epr-5816.81",    0x4000, 0x2000, CRC(4384376d) SHA1(78ae13a38d6368e44ba95642cce7f5515a5b6022) )
	ROM_LOAD( "epr-5815.64",    0x6000, 0x2000, CRC(16ad046c) SHA1(a0b97e016e5cf43f223ecb6c5fe7dec7c8e9c098) )
	ROM_LOAD( "epr-5814.80",    0x8000, 0x2000, CRC(be721c99) SHA1(bbb0afe2b195d60418014c36acf3de95adfd90d8) )
	ROM_LOAD( "epr-5813.63",    0xa000, 0x2000, CRC(0d42c27e) SHA1(06b1d23cacfef3017e5951dc10e8471e9b3103d5) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "epr-5805.86",    0x0000, 0x4000, CRC(5a732865) SHA1(55c54e54f052187ddd957131e56400c9c432a6b2) )
	ROM_LOAD( "epr-5806.93",    0x4000, 0x4000, CRC(26ac258c) SHA1(e4e9f929ab8ae7da74f885481cf94335d7553a1c) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.106",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END


ROM_START( flicky ) // Sega game ID# 834-5411-04 FLICKY
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-5978a.116",  0x0000, 0x4000, CRC(296f1492) SHA1(52e2c63ce376ab8124b2c68bdfa432b6621cfa78) ) // encrypted
	ROM_LOAD( "epr-5979a.109",  0x4000, 0x4000, CRC(64b03ef9) SHA1(7519aa7f036bce6d52a5d4be2418139559f9a8a5) ) // encrypted

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5869.120",   0x0000, 0x2000, CRC(6d220d4e) SHA1(fe02a7a94a1ad046fc775a7f67f460c8d0f6dca6) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-5868.62",    0x0000, 0x2000, CRC(7402256b) SHA1(5bd660ac24a2d0d8ad983e948674a82a2d2e8b49) )
	ROM_LOAD( "epr-5867.61",    0x2000, 0x2000, CRC(2f5ce930) SHA1(4bc3bc6eb8f03926d3710c9f96fcc1b116e918d3) )
	ROM_LOAD( "epr-5866.64",    0x4000, 0x2000, CRC(967f1d9a) SHA1(652be7848526c6e61db4a502f75d1689d2ff2f59) )
	ROM_LOAD( "epr-5865.63",    0x6000, 0x2000, CRC(03d9a34c) SHA1(e158db3e0b86f2b8ad34cefc2714cb0a942efde7) )
	ROM_LOAD( "epr-5864.66",    0x8000, 0x2000, CRC(e659f358) SHA1(cf59f1fb0f9fb77d5ac36be52b6ee946ee85d6de) )
	ROM_LOAD( "epr-5863.65",    0xa000, 0x2000, CRC(a496ca15) SHA1(8c629a853486bbe049b1deecdc00f9e16b87698f) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "epr-5855.117",   0x0000, 0x4000, CRC(b5f894a1) SHA1(2c72dc16739dad155fcd572e1add067a7647f5bd) )
	ROM_LOAD( "epr-5856.110",   0x4000, 0x4000, CRC(266af78f) SHA1(dcbfce550d10a1f2b3ce3e7e081fc008cb575708) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( flickya ) // Sega game ID# 834-5411-11 FLICKY
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-5978a.116",  0x0000, 0x4000, CRC(296f1492) SHA1(52e2c63ce376ab8124b2c68bdfa432b6621cfa78) ) // encrypted
	ROM_LOAD( "epr-5979a.109",  0x4000, 0x4000, CRC(64b03ef9) SHA1(7519aa7f036bce6d52a5d4be2418139559f9a8a5) ) // encrypted

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5869.120",   0x0000, 0x2000, CRC(6d220d4e) SHA1(fe02a7a94a1ad046fc775a7f67f460c8d0f6dca6) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-6001.62",    0x0000, 0x4000, CRC(f1a75200) SHA1(47e57b5dbd687d0fa91de91f35f199e88d5a5d99) )
	ROM_LOAD( "epr-6000.64",    0x4000, 0x4000, CRC(299aefb7) SHA1(d0301f0bf706807891845f090e4e1f1c38dbbd54) )
	ROM_LOAD( "epr-5999.66",    0x8000, 0x4000, CRC(1ca53157) SHA1(46b4b9dac3f0506edc3957cee768e41c4754b0f4) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "epr-5855.117",   0x0000, 0x4000, CRC(b5f894a1) SHA1(2c72dc16739dad155fcd572e1add067a7647f5bd) )
	ROM_LOAD( "epr-5856.110",   0x4000, 0x4000, CRC(266af78f) SHA1(dcbfce550d10a1f2b3ce3e7e081fc008cb575708) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( flickyb ) // Sega game ID# 834-5411-11 FLICKY, only the two program ROMs differ from flickya. Legit or hack?
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "e-2_0.116",  0x0000, 0x4000, CRC(ec94fdbb) SHA1(a3289dd59a4cede1aeed8898cc38cfb82da3b778) ) // white, non original, handwritten label
	ROM_LOAD( "109",        0x4000, 0x4000, CRC(aa11b394) SHA1(538b28b194162c04ed7a46e3a4fa97760201405d) ) // blue, non original, blank label

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5869.120",   0x0000, 0x2000, CRC(6d220d4e) SHA1(fe02a7a94a1ad046fc775a7f67f460c8d0f6dca6) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-6001.62",    0x0000, 0x4000, CRC(f1a75200) SHA1(47e57b5dbd687d0fa91de91f35f199e88d5a5d99) )
	ROM_LOAD( "epr-6000.64",    0x4000, 0x4000, CRC(299aefb7) SHA1(d0301f0bf706807891845f090e4e1f1c38dbbd54) )
	ROM_LOAD( "epr-5999.66",    0x8000, 0x4000, CRC(1ca53157) SHA1(46b4b9dac3f0506edc3957cee768e41c4754b0f4) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "epr-5855.117",   0x0000, 0x4000, CRC(b5f894a1) SHA1(2c72dc16739dad155fcd572e1add067a7647f5bd) )
	ROM_LOAD( "epr-5856.110",   0x4000, 0x4000, CRC(266af78f) SHA1(dcbfce550d10a1f2b3ce3e7e081fc008cb575708) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( flickys2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6621.bin",   0x0000, 0x4000, CRC(b21ff546) SHA1(e1d5438eaf0efeaeb4687dcfc12bf325e804182f) )
	ROM_LOAD( "epr-6622.bin",   0x4000, 0x4000, CRC(133a8bf1) SHA1(e5e620797daace0843a680cb4572586b5e639ca0) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5869.120",   0x0000, 0x2000, CRC(6d220d4e) SHA1(fe02a7a94a1ad046fc775a7f67f460c8d0f6dca6) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-5868.62",    0x0000, 0x2000, CRC(7402256b) SHA1(5bd660ac24a2d0d8ad983e948674a82a2d2e8b49) )
	ROM_LOAD( "epr-5867.61",    0x2000, 0x2000, CRC(2f5ce930) SHA1(4bc3bc6eb8f03926d3710c9f96fcc1b116e918d3) )
	ROM_LOAD( "epr-5866.64",    0x4000, 0x2000, CRC(967f1d9a) SHA1(652be7848526c6e61db4a502f75d1689d2ff2f59) )
	ROM_LOAD( "epr-5865.63",    0x6000, 0x2000, CRC(03d9a34c) SHA1(e158db3e0b86f2b8ad34cefc2714cb0a942efde7) )
	ROM_LOAD( "epr-5864.66",    0x8000, 0x2000, CRC(e659f358) SHA1(cf59f1fb0f9fb77d5ac36be52b6ee946ee85d6de) )
	ROM_LOAD( "epr-5863.65",    0xa000, 0x2000, CRC(a496ca15) SHA1(8c629a853486bbe049b1deecdc00f9e16b87698f) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "epr-5855.117",   0x0000, 0x4000, CRC(b5f894a1) SHA1(2c72dc16739dad155fcd572e1add067a7647f5bd) )
	ROM_LOAD( "epr-5856.110",   0x4000, 0x4000, CRC(266af78f) SHA1(dcbfce550d10a1f2b3ce3e7e081fc008cb575708) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( flickys1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic129",  0x0000, 0x2000, CRC(7011275c) SHA1(69d9d1a66734bf859dbd0200b5a772110bd522c1) ) // encrypted
	ROM_LOAD( "ic130",  0x2000, 0x2000, CRC(e7ed012d) SHA1(7f378ad3e0b6721d7108b4ee10333422df92c039) ) // encrypted
	ROM_LOAD( "ic131",  0x4000, 0x2000, CRC(c5e98cd1) SHA1(ea8d97bebfce4e41242169d34bccbf430b094fd7) ) // encrypted
	ROM_LOAD( "ic132",  0x6000, 0x2000, CRC(0e5122c2) SHA1(cec34001d4eb8a983b3299462ec513049a3dab46) ) // encrypted

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5869.120",   0x0000, 0x2000, CRC(6d220d4e) SHA1(fe02a7a94a1ad046fc775a7f67f460c8d0f6dca6) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-5868.62",    0x0000, 0x2000, CRC(7402256b) SHA1(5bd660ac24a2d0d8ad983e948674a82a2d2e8b49) )
	ROM_LOAD( "epr-5867.61",    0x2000, 0x2000, CRC(2f5ce930) SHA1(4bc3bc6eb8f03926d3710c9f96fcc1b116e918d3) )
	ROM_LOAD( "epr-5866.64",    0x4000, 0x2000, CRC(967f1d9a) SHA1(652be7848526c6e61db4a502f75d1689d2ff2f59) )
	ROM_LOAD( "epr-5865.63",    0x6000, 0x2000, CRC(03d9a34c) SHA1(e158db3e0b86f2b8ad34cefc2714cb0a942efde7) )
	ROM_LOAD( "epr-5864.66",    0x8000, 0x2000, CRC(e659f358) SHA1(cf59f1fb0f9fb77d5ac36be52b6ee946ee85d6de) )
	ROM_LOAD( "epr-5863.65",    0xa000, 0x2000, CRC(a496ca15) SHA1(8c629a853486bbe049b1deecdc00f9e16b87698f) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "epr-5855.117",   0x0000, 0x4000, CRC(b5f894a1) SHA1(2c72dc16739dad155fcd572e1add067a7647f5bd) )
	ROM_LOAD( "epr-5856.110",   0x4000, 0x4000, CRC(266af78f) SHA1(dcbfce550d10a1f2b3ce3e7e081fc008cb575708) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( flickyo ) // Sega game ID# 834-5411  FLICKY
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-5857.ic129",  0x0000, 0x2000, CRC(a65ac88e) SHA1(1d1c276f7ffb33bc9f216b6b69517f1783d435a4) ) // encrypted
	ROM_LOAD( "epr-5858a.ic30",  0x2000, 0x2000, CRC(18b412f4) SHA1(6205dc2a6c1092f9bc7752672b7c06d5faf2f65e) ) // encrypted
	ROM_LOAD( "epr-5859.ic31",   0x4000, 0x2000, CRC(a5558d7e) SHA1(ca59c7e57ae45f960f769db9a04ffa5c870005dd) ) // encrypted
	ROM_LOAD( "epr-5860.ic32",   0x6000, 0x2000, CRC(1b35fef1) SHA1(53ca5361309c59a2b3490ea0037c6e58f07837d9) ) // encrypted

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5869.120",   0x0000, 0x2000, CRC(6d220d4e) SHA1(fe02a7a94a1ad046fc775a7f67f460c8d0f6dca6) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-5868.62",    0x0000, 0x2000, CRC(7402256b) SHA1(5bd660ac24a2d0d8ad983e948674a82a2d2e8b49) )
	ROM_LOAD( "epr-5867.61",    0x2000, 0x2000, CRC(2f5ce930) SHA1(4bc3bc6eb8f03926d3710c9f96fcc1b116e918d3) )
	ROM_LOAD( "epr-5866.64",    0x4000, 0x2000, CRC(967f1d9a) SHA1(652be7848526c6e61db4a502f75d1689d2ff2f59) )
	ROM_LOAD( "epr-5865.63",    0x6000, 0x2000, CRC(03d9a34c) SHA1(e158db3e0b86f2b8ad34cefc2714cb0a942efde7) )
	ROM_LOAD( "epr-5864.66",    0x8000, 0x2000, CRC(e659f358) SHA1(cf59f1fb0f9fb77d5ac36be52b6ee946ee85d6de) )
	ROM_LOAD( "epr-5863.65",    0xa000, 0x2000, CRC(a496ca15) SHA1(8c629a853486bbe049b1deecdc00f9e16b87698f) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "epr-5855.117",   0x0000, 0x4000, CRC(b5f894a1) SHA1(2c72dc16739dad155fcd572e1add067a7647f5bd) )
	ROM_LOAD( "epr-5856.110",   0x4000, 0x4000, CRC(266af78f) SHA1(dcbfce550d10a1f2b3ce3e7e081fc008cb575708) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

// Converted from and running on a Up n' Down board.
ROM_START( flickyup )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "2764-ic29",  0x0000, 0x2000, CRC(59ba3107) SHA1(2d37f00b8a81b97e45aa78ae75449663c35acece) ) // encrypted
	ROM_LOAD( "2764-ic30",  0x2000, 0x2000, CRC(5c84216f) SHA1(8147a51e5d5aee788f1aab8e9032a5813e5c43a1) ) // encrypted
	ROM_LOAD( "2764-ic31",  0x4000, 0x2000, CRC(106132fa) SHA1(669d9cbc1f18649a7d7a8cb462b76a6c34362ed3) ) // encrypted
	ROM_LOAD( "2764-ic32",  0x6000, 0x2000, CRC(c5ea7f58) SHA1(0adcbee77dad813fe168ad383e579c94bc31635f) ) // encrypted

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-5869.120",   0x0000, 0x2000, CRC(6d220d4e) SHA1(fe02a7a94a1ad046fc775a7f67f460c8d0f6dca6) ) // 2764-ic3

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-5868.62",    0x0000, 0x2000, CRC(7402256b) SHA1(5bd660ac24a2d0d8ad983e948674a82a2d2e8b49) ) // 2764-ic82
	ROM_LOAD( "epr-5867.61",    0x2000, 0x2000, CRC(2f5ce930) SHA1(4bc3bc6eb8f03926d3710c9f96fcc1b116e918d3) ) // 2764-ic65
	ROM_LOAD( "epr-5866.64",    0x4000, 0x2000, CRC(967f1d9a) SHA1(652be7848526c6e61db4a502f75d1689d2ff2f59) ) // 2764-ic81
	ROM_LOAD( "epr-5865.63",    0x6000, 0x2000, CRC(03d9a34c) SHA1(e158db3e0b86f2b8ad34cefc2714cb0a942efde7) ) // 2764-ic64
	ROM_LOAD( "epr-5864.66",    0x8000, 0x2000, CRC(e659f358) SHA1(cf59f1fb0f9fb77d5ac36be52b6ee946ee85d6de) ) // 2764-ic80
	ROM_LOAD( "epr-5863.65",    0xa000, 0x2000, CRC(a496ca15) SHA1(8c629a853486bbe049b1deecdc00f9e16b87698f) ) // 2764-ic63

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "epr-5855.117",   0x0000, 0x4000, CRC(b5f894a1) SHA1(2c72dc16739dad155fcd572e1add067a7647f5bd) ) // 27128-ic86
	ROM_LOAD( "epr-5856.110",   0x4000, 0x4000, CRC(266af78f) SHA1(dcbfce550d10a1f2b3ce3e7e081fc008cb575708) ) // 27128-ic93

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) // location ic106
ROM_END

// Converted from and running on an original Up n' Down board, with a bootleg piggyback board with standard Z80 + PROM and PAL
ROM_START( flickyupa )
	ROM_REGION( 0x10000, "maincpu", 0 ) // on piggyback board
	ROM_LOAD( "1", 0x0000, 0x2000, CRC(45391848) SHA1(9e4744ec6e88a7445b5dc7efc96cbac570403acb) )
	ROM_LOAD( "2", 0x2000, 0x2000, CRC(bf15cb82) SHA1(5e33aaac49f740441102f75187cb694f24b462a9) )
	ROM_LOAD( "3", 0x4000, 0x2000, CRC(6ee40df4) SHA1(b01f88636afbc2cb8011665d47c816f409fe311b) )
	ROM_LOAD( "4", 0x6000, 0x2000, CRC(fdecc2b5) SHA1(22b9f06645ed068012919f3c1bb5ee018b92ac49) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "13.120", 0x0000, 0x2000, CRC(6d220d4e) SHA1(fe02a7a94a1ad046fc775a7f67f460c8d0f6dca6) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "7.62",  0x0000, 0x2000, CRC(7402256b) SHA1(5bd660ac24a2d0d8ad983e948674a82a2d2e8b49) )
	ROM_LOAD( "10.61", 0x2000, 0x2000, CRC(2f5ce930) SHA1(4bc3bc6eb8f03926d3710c9f96fcc1b116e918d3) )
	ROM_LOAD( "6.64",  0x4000, 0x2000, CRC(967f1d9a) SHA1(652be7848526c6e61db4a502f75d1689d2ff2f59) )
	ROM_LOAD( "9.63",  0x6000, 0x2000, CRC(03d9a34c) SHA1(e158db3e0b86f2b8ad34cefc2714cb0a942efde7) )
	ROM_LOAD( "5.66",  0x8000, 0x2000, CRC(e659f358) SHA1(cf59f1fb0f9fb77d5ac36be52b6ee946ee85d6de) )
	ROM_LOAD( "8.65",  0xa000, 0x2000, CRC(a496ca15) SHA1(8c629a853486bbe049b1deecdc00f9e16b87698f) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "12.117", 0x0000, 0x4000, CRC(b5f894a1) SHA1(2c72dc16739dad155fcd572e1add067a7647f5bd) )
	ROM_LOAD( "11.110", 0x4000, 0x4000, CRC(266af78f) SHA1(dcbfce550d10a1f2b3ce3e7e081fc008cb575708) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76", 0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )

	ROM_REGION( 0x0100, "decryption_prom", 0 ) // on piggyback board
	ROM_LOAD( "n82s129n", 0x0000, 0x0100, NO_DUMP )

	ROM_REGION( 0x00cc, "plds", 0 )
	ROM_LOAD( "pal20l10cns", 0x0000, 0x00cc, NO_DUMP ) // on piggyback board
ROM_END


ROM_START( wmatch )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wm.129",         0x0000, 0x2000, CRC(b6db4442) SHA1(9f31b3b2d4b4a430f9de84141ebd66bdba063387) ) // encrypted
	ROM_LOAD( "wm.130",         0x2000, 0x2000, CRC(59a0a7a0) SHA1(a1707d08ba968d1ad01f3249c046a62dde8e2730) ) // encrypted
	ROM_LOAD( "wm.131",         0x4000, 0x2000, CRC(4cb3856a) SHA1(983f52bfb2f8e3871518137f424786a9a8e5c53d) ) // encrypted
	ROM_LOAD( "wm.132",         0x6000, 0x2000, CRC(e2e44b29) SHA1(53208666c1368887ab347ea1f261e692cc041d40) ) // encrypted
	ROM_LOAD( "wm.133",         0x8000, 0x2000, CRC(43a36445) SHA1(6cc5a6fa8319d4e2b454b326d8a908ff764fa65f) )
	ROM_LOAD( "wm.134",         0xa000, 0x2000, CRC(5624794c) SHA1(7cfb0a35b7fb8394e0e7efa6b63ba83bd5c9b8e7) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "wm.3",           0x0000, 0x2000, CRC(50d2afb7) SHA1(21b109d389d0b52d89cf635467c3213f6b24d7df) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "wm.82",          0x0000, 0x2000, CRC(540f0bf3) SHA1(3898dee3ed9e7382a9dfc3ee2af177c5b832ea84) )
	ROM_LOAD( "wm.65",          0x2000, 0x2000, CRC(92c1e39e) SHA1(a701a66ed75fbc0be4819751dabb86e51a1dbbc4) )
	ROM_LOAD( "wm.81",          0x4000, 0x2000, CRC(6a01ff2a) SHA1(f609fe9ec648dd428a6e2fc544585935d7adc562) )
	ROM_LOAD( "wm.64",          0x6000, 0x2000, CRC(aae6449b) SHA1(852d6c01420ea55e4215ec99adbb6896fa16a02d) )
	ROM_LOAD( "wm.80",          0x8000, 0x2000, CRC(fc3f0bd4) SHA1(887ff0d6c5fff0d1e631518fc89901d43a0d7088) )
	ROM_LOAD( "wm.63",          0xa000, 0x2000, CRC(c2ce9b93) SHA1(934f4dddf2f42a23f91385dd62fc166b117063b8) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "wm.86",          0x0000, 0x4000, CRC(238ae0e5) SHA1(af18cfe7f8103358a0ce2aef9bbd949fdc0bfbfc) )
	ROM_LOAD( "wm.93",          0x4000, 0x4000, CRC(a2f19170) SHA1(47dacc380b09c6365c737d320145cedad54ecedb) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.106",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END


ROM_START( bullfgt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-.129",       0x0000, 0x2000, CRC(29f19156) SHA1(86cca9601f63b9b3d3aaaf21c3a3e456a50ca6b8) ) // encrypted
	ROM_LOAD( "epr-.130",       0x2000, 0x2000, CRC(e37d2b95) SHA1(9d2523190e49c9d45a5832da912cbc0cd23e2496) ) // encrypted
	ROM_LOAD( "epr-.131",       0x4000, 0x2000, CRC(eaf5773d) SHA1(7db6a7c1c4d9e5f5b4de97b41ab5dd591e2e1548) ) // encrypted
	ROM_LOAD( "epr-.132",       0x6000, 0x2000, CRC(72c3c712) SHA1(1c1ac6d7248382228b99d2652f53fbe15246f253) ) // encrypted
	ROM_LOAD( "epr-.133",       0x8000, 0x2000, CRC(7d9fa4cd) SHA1(b6f0d86281c7e8de7a23b0c55c1991350d5bc9b1) )
	ROM_LOAD( "epr-.134",       0xa000, 0x2000, CRC(061f2797) SHA1(f13acd4c5b33ed85229a3907744283646e020867) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6077.120",   0x0000, 0x2000, CRC(02a37602) SHA1(1b67b0d80a228f7faf054bfd79aff120d92c8166) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-.82",        0x0000, 0x2000, CRC(b71c349f) SHA1(5a0e9b90c71708dadab201da09c71449e05268e1) )
	ROM_LOAD( "epr-.65",        0x2000, 0x2000, CRC(86deafa8) SHA1(b4b9d38bd4a47ce2e75ec0ef3d7507aef8a16858) )
	ROM_LOAD( "epr-6087.81",    0x4000, 0x2000, CRC(2677742c) SHA1(6a6154f1c2cc53b9d224fc73bab47e6deb7c505f) ) // epr-6087.81
	ROM_LOAD( "epr-.64",        0x6000, 0x2000, CRC(6f0a62be) SHA1(30c93c4d7f916f7b9a725f412a3a4a71f24c4f22) )
	ROM_LOAD( "epr-6085.80",    0x8000, 0x2000, CRC(9c3ddc62) SHA1(3332824de114836760a40133fb65d8f40474bc81) ) // epr-6085.80
	ROM_LOAD( "epr-.63",        0xa000, 0x2000, CRC(c0fce57c) SHA1(74f2c987f77e73b7069014d3bd6809d8bb3596c7) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "epr-6069.86",    0x0000, 0x4000, CRC(fe691e41) SHA1(90faf26685202e2a25bb3024750456014d0722b3) ) // epr-6069.86
	ROM_LOAD( "epr-6070.93",    0x4000, 0x4000, CRC(34f080df) SHA1(0e7d28e3325c8c3f06438fde29ea0ffe57fc325f) ) // epr-6070.93

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.106",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) // pr-5317.106
ROM_END

ROM_START( thetogyu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6071.116",   0x0000, 0x4000, CRC(96b57df9) SHA1(bfce24bf570961d3cfb449078e23e546fad7229e) ) // encrypted
	ROM_LOAD( "epr-6072.109",   0x4000, 0x4000, CRC(f7baadd0) SHA1(45a05b72561d47e4ac5475509fe2b57d870c89cd) ) // encrypted
	ROM_LOAD( "epr-6073.96",    0x8000, 0x4000, CRC(721af166) SHA1(0b345715227e70fa6857f5967f0c7da9577f8887) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6077.120",   0x0000, 0x2000, CRC(02a37602) SHA1(1b67b0d80a228f7faf054bfd79aff120d92c8166) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-6089.62",    0x0000, 0x2000, CRC(a183e5ff) SHA1(bb710377a8e88f530b669141ab46abd867c6cb83) )
	ROM_LOAD( "epr-6088.61",    0x2000, 0x2000, CRC(b919b4a6) SHA1(ca11a96bee2e2059552ac6cce6f8dead1965ef4b) )
	ROM_LOAD( "epr-6087.64",    0x4000, 0x2000, CRC(2677742c) SHA1(6a6154f1c2cc53b9d224fc73bab47e6deb7c505f) )
	ROM_LOAD( "epr-6086.63",    0x6000, 0x2000, CRC(76b5a084) SHA1(32fd23f0d6fc8f5c3b5aae9a20907191a6d70611) )
	ROM_LOAD( "epr-6085.66",    0x8000, 0x2000, CRC(9c3ddc62) SHA1(3332824de114836760a40133fb65d8f40474bc81) )
	ROM_LOAD( "epr-6084.65",    0xa000, 0x2000, CRC(90e1fa5f) SHA1(e37a7f872229a93a70e42615e6452aa608d53a93) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "epr-6069.117",   0x0000, 0x4000, CRC(fe691e41) SHA1(90faf26685202e2a25bb3024750456014d0722b3) )
	ROM_LOAD( "epr-6070.110",   0x4000, 0x4000, CRC(34f080df) SHA1(0e7d28e3325c8c3f06438fde29ea0ffe57fc325f) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END


ROM_START( spatter )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6392.116",   0x0000, 0x4000, CRC(329b4506) SHA1(8f71ffc3015c4fcf84a895bf53760830602f1040) ) // encrypted
	ROM_LOAD( "epr-6393.109",   0x4000, 0x4000, CRC(3b56e25f) SHA1(23f26f8632c8a370b5b3b7a3ec58f359cdf04f73) ) // encrypted
	ROM_LOAD( "epr-6394.96",    0x8000, 0x4000, CRC(647c1301) SHA1(5142abfcc63772fd1b47eb584ccda0bc3830e337) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6316.120",   0x0000, 0x2000, CRC(1df95511) SHA1(5780631c8c5a2c3fcd4085f217affa660d72a4e9) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-6328.62",    0x0000, 0x2000, CRC(a2bf2832) SHA1(5d7047a6a0c0588a4e98b6ce94d5fd0e6ab963f9) )
	ROM_LOAD( "epr-6397.61",    0x2000, 0x2000, CRC(c60d4471) SHA1(9e8130d575fa342485dfe093e086a4b48e51b904) )
	ROM_LOAD( "epr-6326.64",    0x4000, 0x2000, CRC(269fbb4c) SHA1(7b91f551360698195bf9ce8e32dd2e8fa17e9db8) )
	ROM_LOAD( "epr-6396.63",    0x6000, 0x2000, CRC(c15ccf3b) SHA1(14809ab81816eedb85cacda042e437d48cf9b31a) )
	ROM_LOAD( "epr-6324.66",    0x8000, 0x2000, CRC(8ab3b563) SHA1(6ede93b9f1593dbcbabd6c875bac8ec01a1b40a2) )
	ROM_LOAD( "epr-6395.65",    0xa000, 0x2000, CRC(3f083065) SHA1(cb17c8c2fe04baa58863c10cd8f359a58def3417) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-6306.04",    0x0000, 0x4000, CRC(e871e132) SHA1(55f7ab1a8c9a118911c64930452ea05f6ee37fc4) )
	ROM_LOAD( "epr-6308.117",   0x4000, 0x4000, CRC(99c2d90e) SHA1(5be54d931622892b7acc320e714d5b1cdce02d19) )
	ROM_LOAD( "epr-6307.05",    0x8000, 0x4000, CRC(0a5ad543) SHA1(5acada30c1affc4ffbebc8365a9ba4465f213d47) )
	ROM_LOAD( "epr-6309.110",   0xc000, 0x4000, CRC(7423ad98) SHA1(e19b4c64795f30e1491520160d315e4148d58df2) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.106",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) // pr-5317.106
ROM_END

ROM_START( spattera )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6597.116",   0x0000, 0x4000, CRC(fb928b9d) SHA1(0a9bede7a147009b9ebb8a0b73681359da665982) ) // encrypted
	ROM_LOAD( "epr-6598.109",   0x4000, 0x4000, CRC(5dff037a) SHA1(8e6f6b75a89609ab0498d317c11e6d653343ffbe) ) // encrypted
	ROM_LOAD( "epr-6599.96",    0x8000, 0x4000, CRC(7ba9de5b) SHA1(f18542c95e8241433ed995c213924ad1ce03cd5b) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6316.120",   0x0000, 0x2000, CRC(1df95511) SHA1(5780631c8c5a2c3fcd4085f217affa660d72a4e9) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-6328.62",    0x0000, 0x2000, CRC(a2bf2832) SHA1(5d7047a6a0c0588a4e98b6ce94d5fd0e6ab963f9) )
	ROM_LOAD( "epr-6397.61",    0x2000, 0x2000, CRC(c60d4471) SHA1(9e8130d575fa342485dfe093e086a4b48e51b904) )
	ROM_LOAD( "epr-6326.64",    0x4000, 0x2000, CRC(269fbb4c) SHA1(7b91f551360698195bf9ce8e32dd2e8fa17e9db8) )
	ROM_LOAD( "epr-6396.63",    0x6000, 0x2000, CRC(c15ccf3b) SHA1(14809ab81816eedb85cacda042e437d48cf9b31a) )
	ROM_LOAD( "epr-6324.66",    0x8000, 0x2000, CRC(8ab3b563) SHA1(6ede93b9f1593dbcbabd6c875bac8ec01a1b40a2) )
	ROM_LOAD( "epr-6395.65",    0xa000, 0x2000, CRC(3f083065) SHA1(cb17c8c2fe04baa58863c10cd8f359a58def3417) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-6306.04",    0x0000, 0x4000, CRC(e871e132) SHA1(55f7ab1a8c9a118911c64930452ea05f6ee37fc4) )
	ROM_LOAD( "epr-6308.117",   0x4000, 0x4000, CRC(99c2d90e) SHA1(5be54d931622892b7acc320e714d5b1cdce02d19) )
	ROM_LOAD( "epr-6307.05",    0x8000, 0x4000, CRC(0a5ad543) SHA1(5acada30c1affc4ffbebc8365a9ba4465f213d47) )
	ROM_LOAD( "epr-6309.110",   0xc000, 0x4000, CRC(7423ad98) SHA1(e19b4c64795f30e1491520160d315e4148d58df2) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.106",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) // pr-5317.106
ROM_END

ROM_START( ssanchan )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6310.116",   0x0000, 0x4000, CRC(26b43701) SHA1(e041bde10da12a3f698da09220f0a7cc2ee99abe) ) // encrypted
	ROM_LOAD( "epr-6311.109",   0x4000, 0x4000, CRC(cb2bc620) SHA1(ecc69360ad9fcc825b35955fbc29da9ea28b8846) ) // encrypted
	ROM_LOAD( "epr-6312.96",    0x8000, 0x4000, CRC(71b15b47) SHA1(7c955be049f9a8d7ca18d877183b698dd5ffe4da) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6316.120",   0x0000, 0x2000, CRC(1df95511) SHA1(5780631c8c5a2c3fcd4085f217affa660d72a4e9) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-6328.62",    0x0000, 0x2000, CRC(a2bf2832) SHA1(5d7047a6a0c0588a4e98b6ce94d5fd0e6ab963f9) )
	ROM_LOAD( "epr-6327.61",    0x2000, 0x2000, CRC(53298109) SHA1(75fd37034aee78d63939d8b4f584c1dc1042264b) )
	ROM_LOAD( "epr-6326.64",    0x4000, 0x2000, CRC(269fbb4c) SHA1(7b91f551360698195bf9ce8e32dd2e8fa17e9db8) )
	ROM_LOAD( "epr-6325.63",    0x6000, 0x2000, CRC(bf038745) SHA1(2fda2412f76b8971ba543ec10da07d4b0d1f2006) )
	ROM_LOAD( "epr-6324.66",    0x8000, 0x2000, CRC(8ab3b563) SHA1(6ede93b9f1593dbcbabd6c875bac8ec01a1b40a2) )
	ROM_LOAD( "epr-6323.65",    0xa000, 0x2000, CRC(0394673c) SHA1(fbee6a5cb37d0394db95781b9f165d766546eb33) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-6306.04",    0x0000, 0x4000, CRC(e871e132) SHA1(55f7ab1a8c9a118911c64930452ea05f6ee37fc4) )
	ROM_LOAD( "epr-6308.117",   0x4000, 0x4000, CRC(99c2d90e) SHA1(5be54d931622892b7acc320e714d5b1cdce02d19) )
	ROM_LOAD( "epr-6307.05",    0x8000, 0x4000, CRC(0a5ad543) SHA1(5acada30c1affc4ffbebc8365a9ba4465f213d47) )
	ROM_LOAD( "epr-6309.110",   0xc000, 0x4000, CRC(7423ad98) SHA1(e19b4c64795f30e1491520160d315e4148d58df2) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.106",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) // pr-5317.106
ROM_END


ROM_START( pitfall2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6456a.116",  0x0000, 0x4000, CRC(bcc8406b) SHA1(2e5c76886fce2c9863db7a914b85b088971aceef) ) // encrypted
	ROM_LOAD( "epr-6457a.109",  0x4000, 0x4000, CRC(a016fd2a) SHA1(866f82066466bc5eaf6ab1b6f85a1c173692a1f7) ) // encrypted
	ROM_LOAD( "epr-6458a.96",   0x8000, 0x4000, CRC(5c30b3e8) SHA1(9048091ebf054d0ba0c6a92520ddfac38a479034) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6462.120",   0x0000, 0x2000, CRC(86bb9185) SHA1(89add2e3784e8f5a20b895fb2c4466bdd6c34b0c) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-6474a.62",   0x0000, 0x2000, CRC(9f1711b9) SHA1(c652010a8b19828f81fd101aa1ea781e250c4ec2) )
	ROM_LOAD( "epr-6473a.61",   0x2000, 0x2000, CRC(8e53b8dd) SHA1(23e04589f2b523d6b8e46d16f40e59685e27f522) )
	ROM_LOAD( "epr-6472a.64",   0x4000, 0x2000, CRC(e0f34a11) SHA1(b7a96a1867f8bd3cc1251b5fd12991c406e62a37) )
	ROM_LOAD( "epr-6471a.63",   0x6000, 0x2000, CRC(d5bc805c) SHA1(520afa7617e8dfd09bf469c01ac606a4a3acdc5e) )
	ROM_LOAD( "epr-6470a.66",   0x8000, 0x2000, CRC(1439729f) SHA1(54ea6ef54be6dcc2a5d00f7f817fd8836a02b3b9) )
	ROM_LOAD( "epr-6469a.65",   0xa000, 0x2000, CRC(e4ac6921) SHA1(f95e3b368c2c6dbf8265fb314d73019fe7dcce22) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "epr-6454a.117",  0x0000, 0x4000, CRC(a5d96780) SHA1(e0571f6fd031bbe2d971c3be7b96a017b0ea4be9) )
	ROM_LOAD( "epr-6455.05",    0x4000, 0x4000, CRC(32ee64a1) SHA1(21743f78735fc9105fbbfac420bdaa2965b4b56f) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( pitfall2a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6505.116",   0x0000, 0x4000, CRC(b6769739) SHA1(e1b8401c20f77f8ec799b19d7bc94ae4f9ed702f) ) // encrypted
	ROM_LOAD( "epr-6506.109",   0x4000, 0x4000, CRC(1ce6aec4) SHA1(69b54c4569ccfb1166a901e7044ae1026db01a82) ) // encrypted
	ROM_LOAD( "epr-6458a.96",   0x8000, 0x4000, CRC(5c30b3e8) SHA1(9048091ebf054d0ba0c6a92520ddfac38a479034) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6462.120",   0x0000, 0x2000, CRC(86bb9185) SHA1(89add2e3784e8f5a20b895fb2c4466bdd6c34b0c) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-6474a.62",   0x0000, 0x2000, CRC(9f1711b9) SHA1(c652010a8b19828f81fd101aa1ea781e250c4ec2) )
	ROM_LOAD( "epr-6473a.61",   0x2000, 0x2000, CRC(8e53b8dd) SHA1(23e04589f2b523d6b8e46d16f40e59685e27f522) )
	ROM_LOAD( "epr-6472a.64",   0x4000, 0x2000, CRC(e0f34a11) SHA1(b7a96a1867f8bd3cc1251b5fd12991c406e62a37) )
	ROM_LOAD( "epr-6471a.63",   0x6000, 0x2000, CRC(d5bc805c) SHA1(520afa7617e8dfd09bf469c01ac606a4a3acdc5e) )
	ROM_LOAD( "epr-6470a.66",   0x8000, 0x2000, CRC(1439729f) SHA1(54ea6ef54be6dcc2a5d00f7f817fd8836a02b3b9) )
	ROM_LOAD( "epr-6469a.65",   0xa000, 0x2000, CRC(e4ac6921) SHA1(f95e3b368c2c6dbf8265fb314d73019fe7dcce22) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "epr-6454a.117",  0x0000, 0x4000, CRC(a5d96780) SHA1(e0571f6fd031bbe2d971c3be7b96a017b0ea4be9) )
	ROM_LOAD( "epr-6455.05",    0x4000, 0x4000, CRC(32ee64a1) SHA1(21743f78735fc9105fbbfac420bdaa2965b4b56f) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( pitfall2u )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6623.116",   0x0000, 0x4000, CRC(bcb47ed6) SHA1(d33421999f899c0a4dc0d4553614c1f5c7027257) )
	ROM_LOAD( "epr-6624a.109",  0x4000, 0x4000, CRC(6e8b09c1) SHA1(4869ca4d3f0b08cd3df4c82be9cfc774ddeb3010) )
	ROM_LOAD( "epr-6625.96",    0x8000, 0x4000, CRC(dc5484ba) SHA1(62fffff7d935c104def5f09e9dc4a26fa4ce4f94) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6462.120",   0x0000, 0x2000, CRC(86bb9185) SHA1(89add2e3784e8f5a20b895fb2c4466bdd6c34b0c) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-6474a.62",   0x0000, 0x2000, CRC(9f1711b9) SHA1(c652010a8b19828f81fd101aa1ea781e250c4ec2) )
	ROM_LOAD( "epr-6473a.61",   0x2000, 0x2000, CRC(8e53b8dd) SHA1(23e04589f2b523d6b8e46d16f40e59685e27f522) )
	ROM_LOAD( "epr-6472a.64",   0x4000, 0x2000, CRC(e0f34a11) SHA1(b7a96a1867f8bd3cc1251b5fd12991c406e62a37) )
	ROM_LOAD( "epr-6471a.63",   0x6000, 0x2000, CRC(d5bc805c) SHA1(520afa7617e8dfd09bf469c01ac606a4a3acdc5e) )
	ROM_LOAD( "epr-6470a.66",   0x8000, 0x2000, CRC(1439729f) SHA1(54ea6ef54be6dcc2a5d00f7f817fd8836a02b3b9) )
	ROM_LOAD( "epr-6469a.65",   0xa000, 0x2000, CRC(e4ac6921) SHA1(f95e3b368c2c6dbf8265fb314d73019fe7dcce22) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "epr-6454a.117",  0x0000, 0x4000, CRC(a5d96780) SHA1(e0571f6fd031bbe2d971c3be7b96a017b0ea4be9) )
	ROM_LOAD( "epr-6455.05",    0x4000, 0x4000, CRC(32ee64a1) SHA1(21743f78735fc9105fbbfac420bdaa2965b4b56f) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END


ROM_START( seganinj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6594a.116",   0x0000, 0x4000, CRC(a5d0c9d0) SHA1(b60caccab8269f40d4f6e7a50f3aa0d4901c1e57) ) // encrypted
	ROM_LOAD( "epr-6595a.109",   0x4000, 0x4000, CRC(b9e6775c) SHA1(f39e815c3c034015125b96de34a2a225b81392b5) ) // encrypted
	ROM_LOAD( "epr-6596a.96",    0x8000, 0x4000, CRC(f2eeb0d8) SHA1(1f0d1c73ba9eaa2887ffc596f0038b0af37ced49) ) // == epr-6552.96

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6559.120",   0x0000, 0x2000, CRC(5a1570ee) SHA1(fd9215e007b6687d057ea7aee01f6d3dcbc8f894) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-6558.62",    0x0000, 0x2000, CRC(2af9eaeb) SHA1(a8a472e9f156c34f1cfcf6d6be808da4303a2276) )
	ROM_LOAD( "epr-6592.61",    0x2000, 0x2000, CRC(7804db86) SHA1(8229781b8296d3ffdfa2f0901e2eed297cc3e160) )
	ROM_LOAD( "epr-6556.64",    0x4000, 0x2000, CRC(79fd26f7) SHA1(a7de0f21ccbcfda495a5c93237569a9b3919d2d5) )
	ROM_LOAD( "epr-6590.63",    0x6000, 0x2000, CRC(bf858cad) SHA1(1c18c4aa4b9a59f3c06aa459eab6bdd1b298d848) )
	ROM_LOAD( "epr-6554.66",    0x8000, 0x2000, CRC(5ac9d205) SHA1(c3094d10d1d6226bf9ad174d2dd1631b8d6ca33a) )
	ROM_LOAD( "epr-6588.65",    0xa000, 0x2000, CRC(dc931dbb) SHA1(4729b27843f226ba5861c3106f8418db70e7c47d) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-6546.117",   0x0000, 0x4000, CRC(a4785692) SHA1(95ce23076dc86c5d6d3a65274873d4c48e91cc06) )
	ROM_LOAD( "epr-6548.04",    0x4000, 0x4000, CRC(bdf278c1) SHA1(7ebe505f4f0434edb2cee17a6cbce6b900b29cc4) )
	ROM_LOAD( "epr-6547.110",   0x8000, 0x4000, CRC(34451b08) SHA1(ee8708f6c886b63f138bcc10dc2a053bfad96c37) )
	ROM_LOAD( "epr-6549a.05",   0xc000, 0x4000, CRC(7c51488c) SHA1(adc835d86e8b51ac47b8619655b3cc2c01aa8c7a) ) // note 'a' revision

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( seganinju )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-7149.116",   0x0000, 0x4000, CRC(cd9fade7) SHA1(958ef5c449df6ef5346b8634cb34a646950f706e) )
	ROM_LOAD( "epr-7150.109",   0x4000, 0x4000, CRC(c36351e2) SHA1(17734d3f410feb4cad617d1931b3356192b69ac0) )
	ROM_LOAD( "epr-7151.96",    0x8000, 0x4000, CRC(f2eeb0d8) SHA1(1f0d1c73ba9eaa2887ffc596f0038b0af37ced49) ) // == epr-6552.96

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6559.120",   0x0000, 0x2000, CRC(5a1570ee) SHA1(fd9215e007b6687d057ea7aee01f6d3dcbc8f894) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-6558.62",    0x0000, 0x2000, CRC(2af9eaeb) SHA1(a8a472e9f156c34f1cfcf6d6be808da4303a2276) )
	ROM_LOAD( "epr-6592.61",    0x2000, 0x2000, CRC(7804db86) SHA1(8229781b8296d3ffdfa2f0901e2eed297cc3e160) )
	ROM_LOAD( "epr-6556.64",    0x4000, 0x2000, CRC(79fd26f7) SHA1(a7de0f21ccbcfda495a5c93237569a9b3919d2d5) )
	ROM_LOAD( "epr-6590.63",    0x6000, 0x2000, CRC(bf858cad) SHA1(1c18c4aa4b9a59f3c06aa459eab6bdd1b298d848) )
	ROM_LOAD( "epr-6554.66",    0x8000, 0x2000, CRC(5ac9d205) SHA1(c3094d10d1d6226bf9ad174d2dd1631b8d6ca33a) )
	ROM_LOAD( "epr-6588.65",    0xa000, 0x2000, CRC(dc931dbb) SHA1(4729b27843f226ba5861c3106f8418db70e7c47d) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-6546.117",   0x0000, 0x4000, CRC(a4785692) SHA1(95ce23076dc86c5d6d3a65274873d4c48e91cc06) )
	ROM_LOAD( "epr-6548.04",    0x4000, 0x4000, CRC(bdf278c1) SHA1(7ebe505f4f0434edb2cee17a6cbce6b900b29cc4) )
	ROM_LOAD( "epr-6547.110",   0x8000, 0x4000, CRC(34451b08) SHA1(ee8708f6c886b63f138bcc10dc2a053bfad96c37) )
	ROM_LOAD( "epr-6549.05",    0xc000, 0x4000, CRC(d2057668) SHA1(ded2a04f7555eb3b1e4da57901ca00635de2c043) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( seganinja )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6879.116",   0x0000, 0x4000, CRC(cae7e51f) SHA1(de6aec8e83cfbe71ed2c52b8f5692aff5ef596a7) )
	ROM_LOAD( "epr-6880.109",   0x4000, 0x4000, CRC(7af85e01) SHA1(57d2a8662efc878ae132cd66de8d46d506ddd6e5) )
	ROM_LOAD( "epr-6881.96",    0x8000, 0x4000, CRC(f2eeb0d8) SHA1(1f0d1c73ba9eaa2887ffc596f0038b0af37ced49) ) // == epr-6552.96

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6559.120",   0x0000, 0x2000, CRC(5a1570ee) SHA1(fd9215e007b6687d057ea7aee01f6d3dcbc8f894) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-6558.62",    0x0000, 0x2000, CRC(2af9eaeb) SHA1(a8a472e9f156c34f1cfcf6d6be808da4303a2276) )
	ROM_LOAD( "epr-6592.61",    0x2000, 0x2000, CRC(7804db86) SHA1(8229781b8296d3ffdfa2f0901e2eed297cc3e160) )
	ROM_LOAD( "epr-6556.64",    0x4000, 0x2000, CRC(79fd26f7) SHA1(a7de0f21ccbcfda495a5c93237569a9b3919d2d5) )
	ROM_LOAD( "epr-6590.63",    0x6000, 0x2000, CRC(bf858cad) SHA1(1c18c4aa4b9a59f3c06aa459eab6bdd1b298d848) )
	ROM_LOAD( "epr-6554.66",    0x8000, 0x2000, CRC(5ac9d205) SHA1(c3094d10d1d6226bf9ad174d2dd1631b8d6ca33a) )
	ROM_LOAD( "epr-6588.65",    0xa000, 0x2000, CRC(dc931dbb) SHA1(4729b27843f226ba5861c3106f8418db70e7c47d) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-6546.117",   0x0000, 0x4000, CRC(a4785692) SHA1(95ce23076dc86c5d6d3a65274873d4c48e91cc06) )
	ROM_LOAD( "epr-6548.04",    0x4000, 0x4000, CRC(bdf278c1) SHA1(7ebe505f4f0434edb2cee17a6cbce6b900b29cc4) )
	ROM_LOAD( "epr-6547.110",   0x8000, 0x4000, CRC(34451b08) SHA1(ee8708f6c886b63f138bcc10dc2a053bfad96c37) )
	ROM_LOAD( "epr-6549.05",    0xc000, 0x4000, CRC(d2057668) SHA1(ded2a04f7555eb3b1e4da57901ca00635de2c043) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( nprinces )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6612.129",   0x0000, 0x2000, CRC(1b30976f) SHA1(f76b7f3d88985a5c190e7880c27ab057f102db31) ) // encrypted
	ROM_LOAD( "epr-6613.130",   0x2000, 0x2000, CRC(18281f27) SHA1(3fcf2fbd1fc13eda678b77c58c53aa881882286c) ) // encrypted
	ROM_LOAD( "epr-6614.131",   0x4000, 0x2000, CRC(69fc3d73) SHA1(287e6b252ae3cd23812b56afe23d4f239f3a76d5) ) // encrypted
	ROM_LOAD( "epr-6615.132",   0x6000, 0x2000, CRC(1d0374c8) SHA1(6d818470e294c03b51ec6db8a285d7b71ab2b61f) ) // encrypted
	ROM_LOAD( "epr-6616.133",   0x8000, 0x2000, CRC(73616e03) SHA1(429615ee1e041d3e14fc557ec39c380fea07de71) )
	ROM_LOAD( "epr-6617.134",   0xa000, 0x2000, CRC(20b6f895) SHA1(9c9cb3b0c33c4da2850a5756b63c3886634ec544) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6559.120",   0x0000, 0x2000, CRC(5a1570ee) SHA1(fd9215e007b6687d057ea7aee01f6d3dcbc8f894) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-6558.62",    0x0000, 0x2000, CRC(2af9eaeb) SHA1(a8a472e9f156c34f1cfcf6d6be808da4303a2276) ) // epr-6558.82
	ROM_LOAD( "epr-6557.61",    0x2000, 0x2000, CRC(6eb131d0) SHA1(27e6f7a3b6ed9a9a5aecfc9981202686b3a81cb4) ) // epr-6557.65
	ROM_LOAD( "epr-6556.64",    0x4000, 0x2000, CRC(79fd26f7) SHA1(a7de0f21ccbcfda495a5c93237569a9b3919d2d5) ) // epr-6556.81
	ROM_LOAD( "epr-6555.63",    0x6000, 0x2000, CRC(7f669aac) SHA1(24ad708112eb26bddf58a70a15273a267121e166) ) // epr-6555.64
	ROM_LOAD( "epr-6554.66",    0x8000, 0x2000, CRC(5ac9d205) SHA1(c3094d10d1d6226bf9ad174d2dd1631b8d6ca33a) ) // epr-6554.80
	ROM_LOAD( "epr-6553.65",    0xa000, 0x2000, CRC(eb82a8fe) SHA1(ec6a418ffbdc8563293d40617aae45382f68ecc2) ) // epr-6553.63

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-6546.117",   0x0000, 0x4000, CRC(a4785692) SHA1(95ce23076dc86c5d6d3a65274873d4c48e91cc06) ) // epr-6546.3
	ROM_LOAD( "epr-6548.04",    0x4000, 0x4000, CRC(bdf278c1) SHA1(7ebe505f4f0434edb2cee17a6cbce6b900b29cc4) ) // epr-6548.1
	ROM_LOAD( "epr-6547.110",   0x8000, 0x4000, CRC(34451b08) SHA1(ee8708f6c886b63f138bcc10dc2a053bfad96c37) ) // epr-6547.4
	ROM_LOAD( "epr-6549.05",    0xc000, 0x4000, CRC(d2057668) SHA1(ded2a04f7555eb3b1e4da57901ca00635de2c043) ) // epr-6549.2

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) // pr-5317.106
ROM_END

ROM_START( nprinceso )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6550.116",   0x0000, 0x4000, CRC(5f6d59f1) SHA1(e151bf22799c6507a167f83262e48fe2ba74dbd9) ) // encrypted
	ROM_LOAD( "epr-6551.109",   0x4000, 0x4000, CRC(1af133b2) SHA1(d3ff924782223ea0566d52ab8b45f17af433966e) ) // encrypted
	ROM_LOAD( "epr-6552.96",    0x8000, 0x4000, CRC(f2eeb0d8) SHA1(1f0d1c73ba9eaa2887ffc596f0038b0af37ced49) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6559.120",   0x0000, 0x2000, CRC(5a1570ee) SHA1(fd9215e007b6687d057ea7aee01f6d3dcbc8f894) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-6558.62",    0x0000, 0x2000, CRC(2af9eaeb) SHA1(a8a472e9f156c34f1cfcf6d6be808da4303a2276) )
	ROM_LOAD( "epr-6557.61",    0x2000, 0x2000, CRC(6eb131d0) SHA1(27e6f7a3b6ed9a9a5aecfc9981202686b3a81cb4) )
	ROM_LOAD( "epr-6556.64",    0x4000, 0x2000, CRC(79fd26f7) SHA1(a7de0f21ccbcfda495a5c93237569a9b3919d2d5) )
	ROM_LOAD( "epr-6555.63",    0x6000, 0x2000, CRC(7f669aac) SHA1(24ad708112eb26bddf58a70a15273a267121e166) )
	ROM_LOAD( "epr-6554.66",    0x8000, 0x2000, CRC(5ac9d205) SHA1(c3094d10d1d6226bf9ad174d2dd1631b8d6ca33a) )
	ROM_LOAD( "epr-6553.65",    0xa000, 0x2000, CRC(eb82a8fe) SHA1(ec6a418ffbdc8563293d40617aae45382f68ecc2) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-6546.117",   0x0000, 0x4000, CRC(a4785692) SHA1(95ce23076dc86c5d6d3a65274873d4c48e91cc06) )
	ROM_LOAD( "epr-6548.04",    0x4000, 0x4000, CRC(bdf278c1) SHA1(7ebe505f4f0434edb2cee17a6cbce6b900b29cc4) )
	ROM_LOAD( "epr-6547.110",   0x8000, 0x4000, CRC(34451b08) SHA1(ee8708f6c886b63f138bcc10dc2a053bfad96c37) )
	ROM_LOAD( "epr-6549.05",    0xc000, 0x4000, CRC(d2057668) SHA1(ded2a04f7555eb3b1e4da57901ca00635de2c043) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( nprincesu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6573.129",   0x0000, 0x2000, CRC(d2919c7d) SHA1(993fdde7dd8d4dbad42f8072829cfea794693a37) )
	ROM_LOAD( "epr-6574.130",   0x2000, 0x2000, CRC(5a132833) SHA1(c21cdca6062a6ea2ca306a8dd26b572b3be86321) )
	ROM_LOAD( "epr-6575.131",   0x4000, 0x2000, CRC(a94b0bd4) SHA1(068db579de3dbd545ae41f930a24f2997a2efedf) )
	ROM_LOAD( "epr-6576.132",   0x6000, 0x2000, CRC(27d3bbdb) SHA1(c7f729798c174de73b6582087f6fe2d4db848b6b) )
	ROM_LOAD( "epr-6577.133",   0x8000, 0x2000, CRC(73616e03) SHA1(429615ee1e041d3e14fc557ec39c380fea07de71) )
	ROM_LOAD( "epr-6578.134",   0xa000, 0x2000, CRC(ab68499f) SHA1(6c662a0ff827cc68bcdb26f6b9d48add4f8ef2e9) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6559.120",   0x0000, 0x2000, CRC(5a1570ee) SHA1(fd9215e007b6687d057ea7aee01f6d3dcbc8f894) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-6558.62",    0x0000, 0x2000, CRC(2af9eaeb) SHA1(a8a472e9f156c34f1cfcf6d6be808da4303a2276) ) // epr-6558.82
	ROM_LOAD( "epr-6557.61",    0x2000, 0x2000, CRC(6eb131d0) SHA1(27e6f7a3b6ed9a9a5aecfc9981202686b3a81cb4) ) // epr-6557.65
	ROM_LOAD( "epr-6556.64",    0x4000, 0x2000, CRC(79fd26f7) SHA1(a7de0f21ccbcfda495a5c93237569a9b3919d2d5) ) // epr-6556.81
	ROM_LOAD( "epr-6555.63",    0x6000, 0x2000, CRC(7f669aac) SHA1(24ad708112eb26bddf58a70a15273a267121e166) ) // epr-6555.64
	ROM_LOAD( "epr-6554.66",    0x8000, 0x2000, CRC(5ac9d205) SHA1(c3094d10d1d6226bf9ad174d2dd1631b8d6ca33a) ) // epr-6554.80
	ROM_LOAD( "epr-6553.65",    0xa000, 0x2000, CRC(eb82a8fe) SHA1(ec6a418ffbdc8563293d40617aae45382f68ecc2) ) // epr-6553.63

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-6546.117",   0x0000, 0x4000, CRC(a4785692) SHA1(95ce23076dc86c5d6d3a65274873d4c48e91cc06) ) // epr-6546.3
	ROM_LOAD( "epr-6548.04",    0x4000, 0x4000, CRC(bdf278c1) SHA1(7ebe505f4f0434edb2cee17a6cbce6b900b29cc4) ) // epr-6548.1
	ROM_LOAD( "epr-6547.110",   0x8000, 0x4000, CRC(34451b08) SHA1(ee8708f6c886b63f138bcc10dc2a053bfad96c37) ) // epr-6547.4
	ROM_LOAD( "epr-6549.05",    0xc000, 0x4000, CRC(d2057668) SHA1(ded2a04f7555eb3b1e4da57901ca00635de2c043) ) // epr-6549.2

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( nprincesb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "nprinces.001",   0x0000, 0x4000, CRC(e0de073c) SHA1(26aec99ddb080124225e0abf17aac4cc4aed1834) ) // encrypted
	ROM_LOAD( "nprinces.002",   0x4000, 0x4000, CRC(27219c7f) SHA1(3f4b0ea9b49907231d10a38d89e2f1803dc168c9) ) // encrypted
	ROM_LOAD( "epr-6552.96",    0x8000, 0x4000, CRC(f2eeb0d8) SHA1(1f0d1c73ba9eaa2887ffc596f0038b0af37ced49) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6559.120",   0x0000, 0x2000, CRC(5a1570ee) SHA1(fd9215e007b6687d057ea7aee01f6d3dcbc8f894) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-6558.62",    0x0000, 0x2000, CRC(2af9eaeb) SHA1(a8a472e9f156c34f1cfcf6d6be808da4303a2276) )
	ROM_LOAD( "epr-6557.61",    0x2000, 0x2000, CRC(6eb131d0) SHA1(27e6f7a3b6ed9a9a5aecfc9981202686b3a81cb4) )
	ROM_LOAD( "epr-6556.64",    0x4000, 0x2000, CRC(79fd26f7) SHA1(a7de0f21ccbcfda495a5c93237569a9b3919d2d5) )
	ROM_LOAD( "epr-6555.63",    0x6000, 0x2000, CRC(7f669aac) SHA1(24ad708112eb26bddf58a70a15273a267121e166) )
	ROM_LOAD( "epr-6554.66",    0x8000, 0x2000, CRC(5ac9d205) SHA1(c3094d10d1d6226bf9ad174d2dd1631b8d6ca33a) )
	ROM_LOAD( "epr-6553.65",    0xa000, 0x2000, CRC(eb82a8fe) SHA1(ec6a418ffbdc8563293d40617aae45382f68ecc2) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-6546.117",   0x0000, 0x4000, CRC(a4785692) SHA1(95ce23076dc86c5d6d3a65274873d4c48e91cc06) )
	ROM_LOAD( "epr-6548.04",    0x4000, 0x4000, CRC(bdf278c1) SHA1(7ebe505f4f0434edb2cee17a6cbce6b900b29cc4) )
	ROM_LOAD( "epr-6547.110",   0x8000, 0x4000, CRC(34451b08) SHA1(ee8708f6c886b63f138bcc10dc2a053bfad96c37) )
	ROM_LOAD( "epr-6549.05",    0xc000, 0x4000, CRC(d2057668) SHA1(ded2a04f7555eb3b1e4da57901ca00635de2c043) )

	ROM_REGION( 0x0220, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
	ROM_LOAD( "nprinces.129",   0x0100, 0x0100, CRC(ae765f62) SHA1(9434b5a23d118a9c62015b479719826b38269cd4) ) // decryption table (not used)
	ROM_LOAD( "nprinces.123",   0x0200, 0x0020, CRC(ed5146e9) SHA1(7044035c07636e4029f4b746c1a92e15173869e9) ) // decryption table (not used)
ROM_END

ROM_START( ninja )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6594.116",   0x0000, 0x4000, CRC(3ef0e5fc) SHA1(ba2d832aa33759c21582e728ca7e4a0ca03cb937) )
	ROM_LOAD( "epr-6595.109",   0x4000, 0x4000, CRC(b16f13cd) SHA1(e4649ce76393fdf8d2a1f53f1c25ee27ed35db45) )
	ROM_LOAD( "epr-6552.96",    0x8000, 0x4000, CRC(f2eeb0d8) SHA1(1f0d1c73ba9eaa2887ffc596f0038b0af37ced49) ) // epr-7151.96

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6559.120",   0x0000, 0x2000, CRC(5a1570ee) SHA1(fd9215e007b6687d057ea7aee01f6d3dcbc8f894) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-6558.62",    0x0000, 0x2000, CRC(2af9eaeb) SHA1(a8a472e9f156c34f1cfcf6d6be808da4303a2276) )
	ROM_LOAD( "epr-6592.61",    0x2000, 0x2000, CRC(88d0c7a1) SHA1(a649a56484f3cf466dbd4bc468d21220e638c5fe) )
	ROM_LOAD( "epr-6556.64",    0x4000, 0x2000, CRC(79fd26f7) SHA1(a7de0f21ccbcfda495a5c93237569a9b3919d2d5) )
	ROM_LOAD( "epr-6590.63",    0x6000, 0x2000, CRC(956e3b61) SHA1(47e797bcc39f3ef917848b64a3666e08f9498cc0) )
	ROM_LOAD( "epr-6554.66",    0x8000, 0x2000, CRC(5ac9d205) SHA1(c3094d10d1d6226bf9ad174d2dd1631b8d6ca33a) )
	ROM_LOAD( "epr-6588.65",    0xa000, 0x2000, CRC(023a14a3) SHA1(199bdf597ace496992f323c0eaa1e779920fb976) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-6546.117",   0x0000, 0x4000, CRC(a4785692) SHA1(95ce23076dc86c5d6d3a65274873d4c48e91cc06) )
	ROM_LOAD( "epr-6548.04",    0x4000, 0x4000, CRC(bdf278c1) SHA1(7ebe505f4f0434edb2cee17a6cbce6b900b29cc4) )
	ROM_LOAD( "epr-6547.110",   0x8000, 0x4000, CRC(34451b08) SHA1(ee8708f6c886b63f138bcc10dc2a053bfad96c37) )
	ROM_LOAD( "epr-6549.05",    0xc000, 0x4000, CRC(d2057668) SHA1(ded2a04f7555eb3b1e4da57901ca00635de2c043) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END


ROM_START( imsorry )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6676.116",   0x0000, 0x4000, CRC(eb087d7f) SHA1(b9bcc76bbdfa597d252e7db60fa0f7529e884cce) ) // encrypted
	ROM_LOAD( "epr-6677.109",   0x4000, 0x4000, CRC(bd244bee) SHA1(ad9c722fde08f48d8bc835b244450b01a3d747c2) ) // encrypted
	ROM_LOAD( "epr-6678.96",    0x8000, 0x4000, CRC(2e16b9fd) SHA1(3395fb769c79f048d099e2898bb7a15611b006c0) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6656.120",   0x0000, 0x2000, CRC(25e3d685) SHA1(a0267d6533af6ff5bf76b9858f2913821a915baf) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-6684.62",    0x0000, 0x2000, CRC(2c8df377) SHA1(abcabdecee0ce52000dab831ae1e50fe12c97066) )
	ROM_LOAD( "epr-6683.61",    0x2000, 0x2000, CRC(89431c48) SHA1(99c0d141eb5519c31b194693a1fe9be882cb03fd) )
	ROM_LOAD( "epr-6682.64",    0x4000, 0x2000, CRC(256a9246) SHA1(6aed392a5dd639c54bf54acd3651a77274c0a277) )
	ROM_LOAD( "epr-6681.63",    0x6000, 0x2000, CRC(6974d189) SHA1(57999a73511b2b3f52d7d6a32addc0641255d7b1) )
	ROM_LOAD( "epr-6680.66",    0x8000, 0x2000, CRC(10a629d6) SHA1(fa2c7df33c685e48020ccabcfba5830e7609e392) )
	ROM_LOAD( "epr-6674.65",    0xa000, 0x2000, CRC(143d883c) SHA1(e35f6fae7feb9a353321d8239ac8990bc773e60b) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "epr-6645.117",   0x0000, 0x4000, CRC(1ba167ee) SHA1(5a105cc3112f2533e7c5982233405d365402fba2) )
	ROM_LOAD( "epr-6646.04",    0x4000, 0x4000, CRC(edda7ad6) SHA1(eef7dcde632787283c4cb522380b138060018204) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( imsorryj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6647.116",   0x0000, 0x4000, CRC(cc5d915d) SHA1(1e2def1f7a03db3504177127dc784fe6c99a7440) ) // encrypted
	ROM_LOAD( "epr-6648.109",   0x4000, 0x4000, CRC(37574d60) SHA1(c7c8507b608976973e766956bd28dfb17222de35) ) // encrypted
	ROM_LOAD( "epr-6649.96",    0x8000, 0x4000, CRC(5f59bdee) SHA1(289ba35a7869a5b833c8aa4819e76fadde2d1ace) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6656.120",   0x0000, 0x2000, CRC(25e3d685) SHA1(a0267d6533af6ff5bf76b9858f2913821a915baf) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-6655.62",    0x0000, 0x2000, CRC(be1f762f) SHA1(abf7af29b1fe4003342fbb431541921433a1fc7c) )
	ROM_LOAD( "epr-6654.61",    0x2000, 0x2000, CRC(ed5f7fc8) SHA1(2e77e8292f644f5bbeebc807f193f20d4591f47a) )
	ROM_LOAD( "epr-6653.64",    0x4000, 0x2000, CRC(8b4845a7) SHA1(048efa9d8122d4a91f4d005d023261a5a5b8b046) )
	ROM_LOAD( "epr-6652.63",    0x6000, 0x2000, CRC(001d68cb) SHA1(c23b4bfbb09b7d3047e04b92d19b69d2ea550879) )
	ROM_LOAD( "epr-6651.66",    0x8000, 0x2000, CRC(4ee9b5e6) SHA1(821bdeefea03c5d3be6d83d0dd30841969d81bd4) )
	ROM_LOAD( "epr-6650.65",    0xa000, 0x2000, CRC(3fca4414) SHA1(d4c80e06bb7027dbc8aea42fb48c71d9fa08ca40) )

	ROM_REGION( 0x8000, "sprites", 0 )
	ROM_LOAD( "epr-6645.117",   0x0000, 0x4000, CRC(1ba167ee) SHA1(5a105cc3112f2533e7c5982233405d365402fba2) )
	ROM_LOAD( "epr-6646.04",    0x4000, 0x4000, CRC(edda7ad6) SHA1(eef7dcde632787283c4cb522380b138060018204) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END


ROM_START( teddybb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6768.116",   0x0000, 0x4000, CRC(5939817e) SHA1(84d78412d3e13da493d08a40deb2ff3fd51ff9f8) ) // encrypted
	ROM_LOAD( "epr-6769.109",   0x4000, 0x4000, CRC(14a98ddd) SHA1(197fa05fb476c02d64e9027cde5aaac26f59b5e8) ) // encrypted
	ROM_LOAD( "epr-6770.96",    0x8000, 0x4000, CRC(67b0c7c2) SHA1(b955719c954af5266e06ae7b04ff20f9dc414997) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr6748x.120",   0x0000, 0x2000, CRC(c2a1b89d) SHA1(55c5461640ccb26bed332c13adfbb99c27237bcb) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-6747.62",    0x0000, 0x2000, CRC(a0e5aca7) SHA1(e7d35ed5e1606a1ea8b29eeca3ca807ed163573b) ) // epr-6776.62
	ROM_LOAD( "epr-6746.61",    0x2000, 0x2000, CRC(cdb77e51) SHA1(590855f41b62fe9a84db51f90242697abb603c00) ) // epr-6775.61
	ROM_LOAD( "epr-6745.64",    0x4000, 0x2000, CRC(0cab75c3) SHA1(ef9b74c62fbd81db8942f0b7aa2569a8f4843e9d) ) // epr-6774.64
	ROM_LOAD( "epr-6744.63",    0x6000, 0x2000, CRC(0ef8d2cd) SHA1(cf9ebf8e3c1d0794b3d3377464f3908d4fcee6f7) ) // epr-6773.63
	ROM_LOAD( "epr-6743.66",    0x8000, 0x2000, CRC(c33062b5) SHA1(5845da895059ff0271a6ed6fd0fa1392be1ac223) ) // epr-6772.66
	ROM_LOAD( "epr-6742.65",    0xa000, 0x2000, CRC(c457e8c5) SHA1(3c1008ae8b054c198cfeb0a66534fb51beaee0f6) ) // epr-6771.65

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-6735.117",   0x0000, 0x4000, CRC(1be35a97) SHA1(7524cfa1a9c9a2e37753f119e7ac7aa3158621be) )
	ROM_LOAD( "epr-6737.04",    0x4000, 0x4000, CRC(6b53aa7a) SHA1(b1b3ff9460b2321e72b49befa63b61c9c36fedd9) )
	ROM_LOAD( "epr-6736.110",   0x8000, 0x4000, CRC(565c25d0) SHA1(5ae524ef01138c5042b223286d65eb9043c0f0d5) )
	ROM_LOAD( "epr-6738.05",    0xc000, 0x4000, CRC(e116285f) SHA1(b6fb50b02a981b3b23385200045ae537092d26d6) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( teddybbo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6739.116",   0x0000, 0x4000, CRC(81a37e69) SHA1(ddd0fd7ba5b3646c43ae4261f1e3fedd4184d92c) ) // encrypted
	ROM_LOAD( "epr-6740.109",   0x4000, 0x4000, CRC(715388a9) SHA1(5affc4ecb1e0d58b69093aed732b1e292b8d3118) ) // encrypted
	ROM_LOAD( "epr-6741.96",    0x8000, 0x4000, CRC(e5a74f5f) SHA1(ccf18b424d4aaeec0bae1e6f096b4c176f6ab554) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6748.120",   0x0000, 0x2000, CRC(9325a1cf) SHA1(555d137b1c974b144ebe6593b4c32c97b3bb5de9) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-6747.62",    0x0000, 0x2000, CRC(a0e5aca7) SHA1(e7d35ed5e1606a1ea8b29eeca3ca807ed163573b) )
	ROM_LOAD( "epr-6746.61",    0x2000, 0x2000, CRC(cdb77e51) SHA1(590855f41b62fe9a84db51f90242697abb603c00) )
	ROM_LOAD( "epr-6745.64",    0x4000, 0x2000, CRC(0cab75c3) SHA1(ef9b74c62fbd81db8942f0b7aa2569a8f4843e9d) )
	ROM_LOAD( "epr-6744.63",    0x6000, 0x2000, CRC(0ef8d2cd) SHA1(cf9ebf8e3c1d0794b3d3377464f3908d4fcee6f7) )
	ROM_LOAD( "epr-6743.66",    0x8000, 0x2000, CRC(c33062b5) SHA1(5845da895059ff0271a6ed6fd0fa1392be1ac223) )
	ROM_LOAD( "epr-6742.65",    0xa000, 0x2000, CRC(c457e8c5) SHA1(3c1008ae8b054c198cfeb0a66534fb51beaee0f6) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-6735.117",   0x0000, 0x4000, CRC(1be35a97) SHA1(7524cfa1a9c9a2e37753f119e7ac7aa3158621be) )
	ROM_LOAD( "epr-6737.04",    0x4000, 0x4000, CRC(6b53aa7a) SHA1(b1b3ff9460b2321e72b49befa63b61c9c36fedd9) )
	ROM_LOAD( "epr-6736.110",   0x8000, 0x4000, CRC(565c25d0) SHA1(5ae524ef01138c5042b223286d65eb9043c0f0d5) )
	ROM_LOAD( "epr-6738.05",    0xc000, 0x4000, CRC(e116285f) SHA1(b6fb50b02a981b3b23385200045ae537092d26d6) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( teddybboa ) // uses same program numbers of the new version but it's similar to the old one with different encryption module?
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6768.116",   0x0000, 0x4000, CRC(30437ff1) SHA1(117995ad21ba716152d3f45e3ac10310ea9930a0) ) // encrypted, SLDH
	ROM_LOAD( "epr-6769.109",   0x4000, 0x4000, CRC(984b47f1) SHA1(fe66aa7189c54be6f94434a19e8fadb4434b3583) ) // encrypted, SLDH
	ROM_LOAD( "epr-6770.96",    0x8000, 0x4000, CRC(e5a74f5f) SHA1(ccf18b424d4aaeec0bae1e6f096b4c176f6ab554) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6748.120",   0x0000, 0x2000, CRC(9325a1cf) SHA1(555d137b1c974b144ebe6593b4c32c97b3bb5de9) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-6747.62",    0x0000, 0x2000, CRC(a0e5aca7) SHA1(e7d35ed5e1606a1ea8b29eeca3ca807ed163573b) )
	ROM_LOAD( "epr-6746.61",    0x2000, 0x2000, CRC(cdb77e51) SHA1(590855f41b62fe9a84db51f90242697abb603c00) )
	ROM_LOAD( "epr-6745.64",    0x4000, 0x2000, CRC(0cab75c3) SHA1(ef9b74c62fbd81db8942f0b7aa2569a8f4843e9d) )
	ROM_LOAD( "epr-6744.63",    0x6000, 0x2000, CRC(0ef8d2cd) SHA1(cf9ebf8e3c1d0794b3d3377464f3908d4fcee6f7) )
	ROM_LOAD( "epr-6743.66",    0x8000, 0x2000, CRC(c33062b5) SHA1(5845da895059ff0271a6ed6fd0fa1392be1ac223) )
	ROM_LOAD( "epr-6742.65",    0xa000, 0x2000, CRC(c457e8c5) SHA1(3c1008ae8b054c198cfeb0a66534fb51beaee0f6) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-6735.117",   0x0000, 0x4000, CRC(1be35a97) SHA1(7524cfa1a9c9a2e37753f119e7ac7aa3158621be) )
	ROM_LOAD( "epr-6737.04",    0x4000, 0x4000, CRC(6b53aa7a) SHA1(b1b3ff9460b2321e72b49befa63b61c9c36fedd9) )
	ROM_LOAD( "epr-6736.110",   0x8000, 0x4000, CRC(565c25d0) SHA1(5ae524ef01138c5042b223286d65eb9043c0f0d5) )
	ROM_LOAD( "epr-6738.05",    0xc000, 0x4000, CRC(e116285f) SHA1(b6fb50b02a981b3b23385200045ae537092d26d6) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END


ROM_START( teddybbobl ) // data in romset is an exact match for teddybbo, including encryption
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.f2",   0x0000, 0x4000, CRC(81a37e69) SHA1(ddd0fd7ba5b3646c43ae4261f1e3fedd4184d92c) ) // == epr-6739.116 (encrypted
	ROM_LOAD( "2.j2",   0x4000, 0x4000, CRC(715388a9) SHA1(5affc4ecb1e0d58b69093aed732b1e292b8d3118) ) // == epr-6740.109
	ROM_LOAD( "3.k2",   0x8000, 0x4000, CRC(e5a74f5f) SHA1(ccf18b424d4aaeec0bae1e6f096b4c176f6ab554) ) // == epr-6741.96

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "6.e10",  0x0000, 0x2000, CRC(9325a1cf) SHA1(555d137b1c974b144ebe6593b4c32c97b3bb5de9) )

	ROM_REGION( 0xc000, "tiles", 0 ) // same as parent except 3x16k instead of 6x8k
	ROM_LOAD( "11.r7",  0x0000, 0x4000, CRC(55d7aaf7) SHA1(84041b665f91b515968aaa48d8ffe93c84c90c57) ) // epr-6747.62 + epr-6746.61
	ROM_LOAD( "10.r8",  0x4000, 0x4000, CRC(52a5083d) SHA1(33afef936ce21d49fc7bae3dd9b9d827e26e002f) ) // epr-6745.64 + epr-6744.63
	ROM_LOAD( "9.r10",  0x8000, 0x4000, CRC(8076d3a3) SHA1(ba7f136b9300a2b41f225a7696345c90a6fbd6a5) ) // epr-6753.66 + epr-6742.65

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "4.f3",   0x0000, 0x4000, CRC(1be35a97) SHA1(7524cfa1a9c9a2e37753f119e7ac7aa3158621be) )
	ROM_LOAD( "6.k3",   0x4000, 0x4000, CRC(6b53aa7a) SHA1(b1b3ff9460b2321e72b49befa63b61c9c36fedd9) )
	ROM_LOAD( "5.h3",   0x8000, 0x4000, CRC(565c25d0) SHA1(5ae524ef01138c5042b223286d65eb9043c0f0d5) )
	ROM_LOAD( "7.m3",   0xc000, 0x4000, CRC(e116285f) SHA1(b6fb50b02a981b3b23385200045ae537092d26d6) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )

	ROM_REGION( 0x0100, "promsbl", 0 )
	ROM_LOAD( "74s287.bin",     0x0000, 0x0100, CRC(de9af32c) SHA1(f999465bb4600a97179a9253e17413f6837703df) )
ROM_END


ROM_START( hvymetal )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "epr-6790a.1",   0x00000, 0x8000, CRC(59195bb9) SHA1(63dde673bd875dd23d445b152decb1d70c3750a4) ) // encrypted
	ROM_LOAD( "epr-6789a.2",   0x10000, 0x8000, CRC(83e1d18a) SHA1(07ef58ee2a5212e1e2800efc2bd48d2b2a9ed10d) )
	ROM_LOAD( "epr-6788a.3",   0x18000, 0x8000, CRC(6ecefd57) SHA1(3236313d5d826873d58af5ad80652c8d0ae0cc31) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6787.120",   0x0000, 0x8000, CRC(b64ac7f0) SHA1(2b16c2702d3230891b700714a66ece95f1a74b44) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "epr-6795.62",   0x00000, 0x4000, CRC(58a3d038) SHA1(9aabfad143748e2ec1b41fde72a1d533bac3f9d8) )
	ROM_LOAD( "epr-6796.61",   0x04000, 0x4000, CRC(d8b08a55) SHA1(cfa5370aa430947637bfe57a5a1f802f273b43f7) )
	ROM_LOAD( "epr-6793.64",   0x08000, 0x4000, CRC(487407c2) SHA1(9bb9fff24fe057fa17057ba9263d412905a0c036) )
	ROM_LOAD( "epr-6794.63",   0x0c000, 0x4000, CRC(89eb3793) SHA1(90a0cc81d917122c726238585eb802763d34884e) )
	ROM_LOAD( "epr-6791.66",   0x10000, 0x4000, CRC(a7dcd042) SHA1(d9bac10aa7ac591a20bfed4e391ec1669eadc32d) )
	ROM_LOAD( "epr-6792.65",   0x14000, 0x4000, CRC(d0be5e33) SHA1(1e61c6e14c3c736e74e6c2ff5cde71d1d20b99a4) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "epr-6778.117",  0x00000, 0x8000, CRC(0af61aee) SHA1(90879d4d1bef38714a39ca71c101bd103d250284) )
	ROM_LOAD( "epr-6777.110",  0x08000, 0x8000, CRC(91d7a197) SHA1(34c12b7de22169d369ff5b8a8d86da62404267f8) )
	ROM_LOAD( "epr-6780.4",    0x10000, 0x8000, CRC(55b31df5) SHA1(aa1ce0b1666e17db196bd1e079691fbe433a9226) )
	ROM_LOAD( "epr-6779.5",    0x18000, 0x8000, CRC(e03a2b28) SHA1(7e742c09e832d01f74fe4025d194cbc8d2f24b70) )

	ROM_REGION( 0x0300, "color_proms", 0 )
	ROM_LOAD( "pr7036.3",     0x0000, 0x0100, CRC(146f16fb) SHA1(0a2ac871383b115c16491b9ba5973f0d363eac49) ) // palette red component
	ROM_LOAD( "pr7035.2",     0x0100, 0x0100, CRC(50b201ed) SHA1(14c3a585c083dc387532d64bfd63e34f5220e6de) ) // palette green component
	ROM_LOAD( "pr7034.1",     0x0200, 0x0100, CRC(dfb5f139) SHA1(56cba261819fd5f2beab56ffd80bb3fd328efe3e) ) // palette blue component

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr5317p.4",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END


ROM_START( myhero )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6963b.116",  0x0000, 0x4000, CRC(4daf89d4) SHA1(6fd69964d4e0dcd5637920711361f1879fcf330e) )
	ROM_LOAD( "epr-6964a.109",  0x4000, 0x4000, CRC(c26188e5) SHA1(48d7871a9c63de774c48f1bd9dcaf84b4188f84f) )
	ROM_LOAD( "epr-6927.96",    0x8000, 0x4000, CRC(3cbbaf64) SHA1(fdb5f2ca38010729afa4ed24c087119cf398f27d) ) // epr-6965.96

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-69xx.120",   0x0000, 0x2000, CRC(0039e1e9) SHA1(ead2e8a8a518da5ac6ccd5cd6db4cf167ea47c76) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-6966.62",    0x0000, 0x2000, CRC(157f0401) SHA1(f07eb40de95054d6a2c2ebec0b251685e8931b37) )
	ROM_LOAD( "epr-6961.61",    0x2000, 0x2000, CRC(be53ce47) SHA1(de6073e7a00cba7e13aca0248c55126b16595d50) )
	ROM_LOAD( "epr-6960.64",    0x4000, 0x2000, CRC(bd381baa) SHA1(e160db821422232fb8f6b4f1c4ce0b61f7bed463) )
	ROM_LOAD( "epr-6959.63",    0x6000, 0x2000, CRC(bc04e79a) SHA1(df93f96aabde981fe9ecf32ef1f99dfebe968835) )
	ROM_LOAD( "epr-6958.66",    0x8000, 0x2000, CRC(714f2c26) SHA1(4696c9322d7b9b27f56309312fe498f14cb32827) )
	ROM_LOAD( "epr-6957.65",    0xa000, 0x2000, CRC(80920112) SHA1(745d029f99b6878efcca535885b9bf98bf8702f2) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-6921.117",   0x0000, 0x4000, CRC(f19e05a1) SHA1(98288ba2e96c03a4ab9c8235faa7e01bb376d021) )
	ROM_LOAD( "epr-6923.04",    0x4000, 0x4000, CRC(7988adc3) SHA1(4ee9e964c24234366660af4981566e8c45f46db9) )
	ROM_LOAD( "epr-6922.110",   0x8000, 0x4000, CRC(37f77a78) SHA1(01d8bd41303bd5e3a6f1cdafa4a1d682e4c659a2) )
	ROM_LOAD( "epr-6924.05",    0xc000, 0x4000, CRC(42bdc8f6) SHA1(f31d82641187a7cc77a4a19189b5a15d5168cbd7) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( sscandal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-6925b.116",  0x0000, 0x4000, CRC(ff54dcec) SHA1(634ba5c79dc20dc6ab3efd9597b9fb1e4f86f58f) ) // encrypted
	ROM_LOAD( "epr-6926a.109",  0x4000, 0x4000, CRC(5c41eea8) SHA1(6a060a9739ee85c5c3a3e205bfac46bff1ed0b91) ) // encrypted
	ROM_LOAD( "epr-6927.96",    0x8000, 0x4000, CRC(3cbbaf64) SHA1(fdb5f2ca38010729afa4ed24c087119cf398f27d) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6934.120",   0x0000, 0x2000, CRC(af467223) SHA1(d79a67e761fe483407cad645dd3b93d86e8790e3) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-6933.62",    0x0000, 0x2000, CRC(e7304036) SHA1(cff10b180832703ef472a6abd481f8433308d462) )
	ROM_LOAD( "epr-6932.61",    0x2000, 0x2000, CRC(f5cfbfda) SHA1(52044e3eb6f2e82c9490856410758c5223eb116b) )
	ROM_LOAD( "epr-6931.64",    0x4000, 0x2000, CRC(599d7f87) SHA1(c581001b45856447b2878dc5bdeb92bffb15086a) )
	ROM_LOAD( "epr-6930.63",    0x6000, 0x2000, CRC(cb6616c2) SHA1(84d4f65379cb9d5c9774d29bbad137529ab221a6) )
	ROM_LOAD( "epr-6929.66",    0x8000, 0x2000, CRC(27a16856) SHA1(1e386dfa5178a0902f5d5e64f4d0414593f2e801) )
	ROM_LOAD( "epr-6928.65",    0xa000, 0x2000, CRC(c0c9cfa4) SHA1(3a98f25beab2dcacf5ec4457501ecfde9bc6e8eb) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-6921.117",   0x0000, 0x4000, CRC(f19e05a1) SHA1(98288ba2e96c03a4ab9c8235faa7e01bb376d021) )
	ROM_LOAD( "epr-6923.04",    0x4000, 0x4000, CRC(7988adc3) SHA1(4ee9e964c24234366660af4981566e8c45f46db9) )
	ROM_LOAD( "epr-6922.110",   0x8000, 0x4000, CRC(37f77a78) SHA1(01d8bd41303bd5e3a6f1cdafa4a1d682e4c659a2) )
	ROM_LOAD( "epr-6924.05",    0xc000, 0x4000, CRC(42bdc8f6) SHA1(f31d82641187a7cc77a4a19189b5a15d5168cbd7) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END


ROM_START( myherobl )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.f2",   0x0000, 0x4000, CRC(c1d354dc) SHA1(0146dda49ccf5e0d1b507604095b75690e211f1b) )
	ROM_LOAD( "2.g2",   0x4000, 0x4000, CRC(688c9ede) SHA1(768f2e4bb797c2c85568d0ec2cda974e77efaff3) )
	ROM_LOAD( "3.h2",   0x8000, 0x4000, CRC(3cbbaf64) SHA1(fdb5f2ca38010729afa4ed24c087119cf398f27d) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "6.e10",   0x0000, 0x2000, CRC(af467223) SHA1(d79a67e761fe483407cad645dd3b93d86e8790e3) )

	ROM_REGION( 0xc000, "tiles", 0 ) // identical to original except for first half of B13.R10 ( has Coreland / Sega copyright tiles like the Japan set, not closer otherwise to the parent set )
	ROM_LOAD( "b13.r10",   0x0000, 0x4000, CRC(9a4861b1) SHA1(0b09556101a8d06f5dacb40970681113d493cbf5) ) // epr-6966.62             B13.R10      [1/2]      98.498535%
	ROM_LOAD( "b11.r7",    0x4000, 0x4000, CRC(0d6f248a) SHA1(18229745adc552c58a865a181ddad44dfd62bfad) )
	ROM_LOAD( "x.r8",      0x8000, 0x4000, CRC(24537709) SHA1(2afbefb41b6541d7e27ebef7339f7a26aa2c00c6) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "4.f4",   0x0000, 0x4000, CRC(f19e05a1) SHA1(98288ba2e96c03a4ab9c8235faa7e01bb376d021) )
	ROM_LOAD( "x.h4",   0x4000, 0x4000, CRC(7988adc3) SHA1(4ee9e964c24234366660af4981566e8c45f46db9) )
	ROM_LOAD( "x.g4",   0x8000, 0x4000, CRC(37f77a78) SHA1(01d8bd41303bd5e3a6f1cdafa4a1d682e4c659a2) )
	ROM_LOAD( "b7.k4",  0xc000, 0x4000, CRC(42bdc8f6) SHA1(f31d82641187a7cc77a4a19189b5a15d5168cbd7) )

	ROM_REGION( 0x0100, "lookup_proms", 0 ) // wasn't dumped here, but needed for priority
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )

	ROM_REGION( 0x0100, "promsbl", 0 ) // this is related to the bootleg reproduction of the encryption scheme
	ROM_LOAD( "prom.a2",     0x0000, 0x0100, CRC(4fcaf000) SHA1(c68592377373b157713b5e129b020feb6c866f91) )
	ROM_IGNORE(0x100)
ROM_END


ROM_START( myherok )
	ROM_REGION( 0x10000, "maincpu", 0 )
	// all the three program ROMs have bits 0-1 swapped
	// when decoded, they are identical to the Japanese version
	ROM_LOAD( "ry-11.rom",      0x0000, 0x4000, CRC(6f4c8ee5) SHA1(bbbb87a66be383d9d44ae3bb7f4d1ff56933fd57) ) // encrypted
	ROM_LOAD( "ry-09.rom",      0x4000, 0x4000, CRC(369302a1) SHA1(670bf97e401c0a665330d2264c126c275f4c5f8d) ) // encrypted
	ROM_LOAD( "ry-07.rom",      0x8000, 0x4000, CRC(b8e9922e) SHA1(f563fd415d5218c2c3e0071776c91b6250cacea3) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-6934.120",   0x0000, 0x2000, CRC(af467223) SHA1(d79a67e761fe483407cad645dd3b93d86e8790e3) )

	ROM_REGION( 0xc000, "tiles", 0 )
	// all three gfx ROMs have address lines A4 and A5 swapped, also #1 and #3
	// have data lines D0 and D6 swapped, while #2 has data lines D1 and D5 swapped.
	ROM_LOAD( "ry-04.rom",      0x0000, 0x4000, CRC(dfb75143) SHA1(b1943e0b8ca4439d5ef27abecd48e6fc806d3a0e) )
	ROM_LOAD( "ry-03.rom",      0x4000, 0x4000, CRC(cf68b4a2) SHA1(7f1607320943c452bcc30b4805e8e9c9d2a61955) )
	ROM_LOAD( "ry-02.rom",      0x8000, 0x4000, CRC(d100eaef) SHA1(d917a85c3560578cc7640bfcb4725b4217f0ed91) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-6921.117",   0x0000, 0x4000, CRC(f19e05a1) SHA1(98288ba2e96c03a4ab9c8235faa7e01bb376d021) )
	ROM_LOAD( "epr-6923.04",    0x4000, 0x4000, CRC(7988adc3) SHA1(4ee9e964c24234366660af4981566e8c45f46db9) )
	ROM_LOAD( "epr-6922.110",   0x8000, 0x4000, CRC(37f77a78) SHA1(01d8bd41303bd5e3a6f1cdafa4a1d682e4c659a2) )
	ROM_LOAD( "epr-6924.05",    0xc000, 0x4000, CRC(42bdc8f6) SHA1(f31d82641187a7cc77a4a19189b5a15d5168cbd7) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END


ROM_START( 4dwarrio )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "4d.116",       0x0000, 0x4000, CRC(546d1bc7) SHA1(724bb2f77a2b82fae85e535ae4a37820cfb323d0) ) // encrypted
	ROM_LOAD( "4d.109",       0x4000, 0x4000, CRC(f1074ec3) SHA1(bc368abeb6c0a7172e03bd7a1754cf4a6ecbb4f8) ) // encrypted
	ROM_LOAD( "4d.96",        0x8000, 0x4000, CRC(387c1e8f) SHA1(520ecbafd1c7271dad24410a68067dfd801fa6d6) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "4d.120",       0x0000, 0x2000, CRC(5241c009) SHA1(b7a21f95b63234f2496d5ea6e7dc8050ca1b39fc) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "4d.62",        0x0000, 0x2000, CRC(f31b2e09) SHA1(fdc288769495f4b0ca8c7594c9ab7dc0f29e57a4) )
	ROM_LOAD( "4d.61",        0x2000, 0x2000, CRC(5430e925) SHA1(55f92309223c41871175b1f54418c8b08339deb0) )
	ROM_LOAD( "4d.64",        0x4000, 0x2000, CRC(9f442351) SHA1(07076ef66e29c730050e38aecabdfbfced9f9bc4) )
	ROM_LOAD( "4d.63",        0x6000, 0x2000, CRC(633232bd) SHA1(c09c1df4f04608381d665a83776005607ad97ad4) )
	ROM_LOAD( "4d.66",        0x8000, 0x2000, CRC(52bfa2ed) SHA1(ea1c18d07957301f2006350b02fe40d13dbe2aa5) )
	ROM_LOAD( "4d.65",        0xa000, 0x2000, CRC(e9ba4658) SHA1(ba2581a52eb54e2d9f1e1bf30050280df3f5df1b) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "4d.117",       0x0000, 0x4000, CRC(436e4141) SHA1(2574d5c3b01c89d8a041c82af976147d3b87b36b) )
	ROM_LOAD( "4d.04",        0x4000, 0x4000, CRC(8b7cecef) SHA1(4851754cb56784ac248f699f0781646455dd556b) )
	ROM_LOAD( "4d.110",       0x8000, 0x4000, CRC(6ec5990a) SHA1(a26dbd470744c38a26a016e5d4792ac2f2b9bc4b) )
	ROM_LOAD( "4d.05",        0xc000, 0x4000, CRC(f31a1e6a) SHA1(f49dbc4b381e7096d5ffe3c16660dd63121dabf7) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr5317.76",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END


/*
    Shooting Master (SEGA)
    Year: 1985
    System 2

    Main Board        834-5719
    Light Gun Board?  834-5720


Offically licensed and manufactured in Italy, 100% identical code:

    Shooting Master (EVG)
    Year: 1985
    Manufacturer: E.V.G. SRL Milano made in Italy (Sega license)

    CPU
    1x Z8400AB1-Z80ACPU-Y28548 (main board)
    1x iC8751H-88-L5310039 (main board)
    1x AMD P8255A-8526YP (main board)
    1x SEGA 315-5012-8605P5 (main board)
    1x SEGA 315-5011-8549X5 (main board)
    1x SEGA 315-5049-8551PX (main board)
    1x SEGA 315-5139-8537-CK2605-V-J (main board)
    1x oscillator 20.000MHz (main board)
    1x SYS Z8400AB1-Z80ACPU-Y28535 (upper board)
    1x NEC D8255AC-2 (upper board)
    1x oscillator 4.9152MHz (upper board)

    ROMs
    1x HN27256G-25 (7043)(main board close to Z80)
    2x HN27256G-25 (7101-7102)(main board close to C8751)
    3x HN27256G-25 (7040-7041-7042)(main board close to 315-5049)
    2x PAL16R4A (315-5137 and 315-5138)
    1x HN27256G-25 (7100)(upper board close to oscillator)
    7x HN27256G-25 (7104 to 7110)(upper board close to Z80 and 8255)

*/
ROM_START( shtngmst )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "epr-7100.ic18", 0x00000, 0x8000, CRC(268ecb1d) SHA1(a9274c9718f7244235cc6df76331d6a0b7e4e4c8) ) // This rom is located on the daughter board.
	ROM_LOAD( "epr-7101.ic91", 0x10000, 0x8000, CRC(ebf5ff72) SHA1(13ae06e3a81cf00b80ec939d5baf30143d61d480) ) // These 2 roms are located on the main board.
	ROM_LOAD( "epr-7102.ic92", 0x18000, 0x8000, CRC(c890a4ad) SHA1(4b59d37902ace3a69b380ff40652ee37c85f0e9d) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-7043.ic126",  0x0000, 0x8000, CRC(99a368ab) SHA1(a9451f39ee2613e5c3e2791d4d8d837b4a3ab666) ) // This rom is located on the main board.

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "315-5159a.ic74", 0x00000, 0x1000, CRC(1f774912) SHA1(34d12756735514bea5a513fdf441ae93318747b2) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "epr-7040.ic4",    0x00000, 0x8000, CRC(f30769fa) SHA1(366c1fbe4e1c8943b209f6c831c9a6b7e4372105) ) // These roms are located on the main board.
	ROM_LOAD( "epr-7041.ic5",    0x08000, 0x8000, CRC(f3e273f9) SHA1(b8715c528299dc1e4f0c19c50d91ca9861a423a1) )
	ROM_LOAD( "epr-7042.ic6",    0x10000, 0x8000, CRC(6841c917) SHA1(6553843eea0131eb7b5a9aa29dddf641e41d8cc3) )

	ROM_REGION( 0x40000, "sprites", ROMREGION_ERASEFF )
	ROM_LOAD( "epr-7110.ic26",   0x00000, 0x8000, CRC(5d1a5048) SHA1(d1626ab1981080451c912df7e4ad7f76c0cb3459) ) // These roms are located on the daughter board.
	ROM_LOAD( "epr-7106.ic22",   0x08000, 0x8000, CRC(ae7ab7a2) SHA1(153691e468d29d21b95f1fbffb6896a3140d7e14) )
	ROM_LOAD( "epr-7108.ic24",   0x10000, 0x8000, CRC(816180ac) SHA1(a59670ec77d4359041ebf12dae5b74add55d82ac) )
	ROM_LOAD( "epr-7104.ic20",   0x18000, 0x8000, CRC(84a679c5) SHA1(19a21b1b33fc215f606093bfd61d597e4bd0b3d0) )
	ROM_LOAD( "epr-7109.ic25",   0x20000, 0x8000, CRC(097f7481) SHA1(4d93ea01b811af1cd3e136116625e4b8e06358a2) )
	ROM_LOAD( "epr-7105.ic21",   0x28000, 0x8000, CRC(13111729) SHA1(57ca2b945db36b056d0e40a39456fd8bf9d0a3ec) )
	ROM_LOAD( "epr-7107.ic23",   0x30000, 0x8000, CRC(8f50ea24) SHA1(781687e202dedca7b72c9bd5b97d9d46fcfd601c) )

	// These proms are located on the main board.
	ROM_REGION( 0x0300, "color_proms", 0 )
	ROM_LOAD( "epr-7113.ic20",   0x00000, 0x0100, CRC(5c0e1360) SHA1(2011b3eef2a58f9bd3f3b1bb9e6c201db85727c2) ) // palette red component
	ROM_LOAD( "epr-7112.ic14",   0x00100, 0x0100, CRC(46fbd351) SHA1(1fca7fbc5d5f8e13e58bbac735511bd0af392446) ) // palette green component
	ROM_LOAD( "epr-7111.ic8",    0x00200, 0x0100, CRC(8123b6b9) SHA1(fb2c5498f0603b5cd270402a738c891a85453666) ) // palette blue component - N82S129AN

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr5317.ic37",   0x00000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) // N82S129AN

	// These pld's are located on the main board.
	ROM_REGION( 0x1000, "plds", 0 )
	ROM_LOAD( "315-5137.ic10",   0x0000, 0x0104, CRC(6ffd9e6f) SHA1(a60a3a2ec5bc256b18bfff0fec0172ee2e4fd955) ) // TI PAL16R4A-2CN
	ROM_LOAD( "315-5138.ic11",   0x0200, 0x0104, CRC(dd223015) SHA1(8d70f91b118e8653dda1efee3eaea287ae63809f) ) // TI PAL16R4ACN
	ROM_LOAD( "315-5139.ic50",   0x0400, 0x00e7, CRC(943d91b0) SHA1(37c98085d580808aaeb01726a9f59705590378c4) ) // PLS153 CK2605
	// Note that IC7, IC13 and IC19 (315-5155) are not PLDs, but are a custom graphics shifter.
ROM_END

/*
    Choplifter (8751 315-5151)
    Year: 1985
    System 2

    Main Board 834-5795
*/
ROM_START( choplift )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "epr-7124.ic90",  0x00000, 0x8000, CRC(678d5c41) SHA1(7553979f78270c2ddc5b3f3ebf7817ead8e08de7) )
	ROM_LOAD( "epr-7125.ic91",  0x10000, 0x8000, CRC(f5283498) SHA1(1ad40f6d7b4cd18212ee56917240c0796f1a4ec2) )
	ROM_LOAD( "epr-7126.ic92",  0x18000, 0x8000, CRC(dbd192ab) SHA1(03d280c82599a14fc6a2065d57c6241cdc6f1143) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-7130.ic126", 0x0000, 0x8000, CRC(346af118) SHA1(ef579818a45b8ebb276d5832092b26e232d5a737) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "315-5151.ic74",  0x00000, 0x1000, CRC(1377a6ef) SHA1(b85acd7292e5480c98af1a0492b6b5d3f9b1716c) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "epr-7127.ic4",   0x00000, 0x8000, CRC(1e708f6d) SHA1(b975e13bdc44105e7a15c2694e3ec53b60e23e5e) )
	ROM_LOAD( "epr-7128.ic5",   0x08000, 0x8000, CRC(b922e787) SHA1(16087671ec7de25f749b5fd66409d48ef7b35820) )
	ROM_LOAD( "epr-7129.ic6",   0x10000, 0x8000, CRC(bd3b6e6e) SHA1(c66f21b98cb8fc61a9318041ac1812c13099d974) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "epr-7121.ic87",  0x00000, 0x8000, CRC(f2b88f73) SHA1(2b06da1beabbea82d502fbe12f6ec3ef26056edd) )
	ROM_LOAD( "epr-7120.ic86",  0x08000, 0x8000, CRC(517d7fd3) SHA1(3fb5c00224920c3f62fb86e82caf0fee2293e1e2) )
	ROM_LOAD( "epr-7123.ic89",  0x10000, 0x8000, CRC(8f16a303) SHA1(5f2465505f001dc052e9de4cf66bc1d53fc8c7da) )
	ROM_LOAD( "epr-7122.ic88",  0x18000, 0x8000, CRC(7c93f160) SHA1(6ab156cad7556808496070f8b02a708ce405c492) )

	ROM_REGION( 0x0300, "color_proms", 0 )
	ROM_LOAD( "pr7119.ic20",    0x0000, 0x0100, CRC(b2a8260f) SHA1(36c1debb4b3f2f190a25b18d533319d7380416de) ) // palette red component
	ROM_LOAD( "pr7118.ic14",    0x0100, 0x0100, CRC(693e20c7) SHA1(9ebf4bd2c30ddd9648bc4b41c7739cfdf80100da) ) // palette green component
	ROM_LOAD( "pr7117.ic8",     0x0200, 0x0100, CRC(4124307e) SHA1(cee28d891e6ce732c43a61acb5beeafd2200cf37) ) // palette blue component

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr5317.ic28",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )

	ROM_REGION( 0x0618, "plds", 0 )
	ROM_LOAD( "315-5152.ic10",   0x00000, 0x0104, CRC(2c9229b4) SHA1(9755013afcf89f99d7a399c7e223e027761cf89a) ) // PAL16R4A
	ROM_LOAD( "315-5138.ic11",   0x00000, 0x0104, CRC(dd223015) SHA1(8d70f91b118e8653dda1efee3eaea287ae63809f) ) // TI PAL16R4NC
	ROM_LOAD( "315-5139.ic50",   0x00000, 0x00e7, CRC(943d91b0) SHA1(37c98085d580808aaeb01726a9f59705590378c4) ) // CK2605
	ROM_LOAD( "315-5025.ic7",    0x00000, 0x0104, NO_DUMP )
	ROM_LOAD( "315-5025.ic13",   0x00000, 0x0104, NO_DUMP )
	ROM_LOAD( "315-5025.ic19",   0x00000, 0x0104, NO_DUMP )
ROM_END

/*
    Choplifter (Unprotected)
    Year: 1985
    System 2

    Main Board 834-5795-03
*/
ROM_START( chopliftu )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "epr-7152.ic90",  0x00000, 0x8000, CRC(fe49d83e) SHA1(307be38dd73ed37b275c1b464d266a752cb06132) )
	ROM_LOAD( "epr-7153.ic91",  0x10000, 0x8000, CRC(48697666) SHA1(0f4c6db9558272f5ceb347e742b284474f18b707) )
	ROM_LOAD( "epr-7154.ic92",  0x18000, 0x8000, CRC(56d6222a) SHA1(ad8ccf15fe7f1d6716f78490892da0167d79f678) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-7130.ic126", 0x0000, 0x8000, CRC(346af118) SHA1(ef579818a45b8ebb276d5832092b26e232d5a737) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "epr-7127.ic4",   0x00000, 0x8000, CRC(1e708f6d) SHA1(b975e13bdc44105e7a15c2694e3ec53b60e23e5e) )
	ROM_LOAD( "epr-7128.ic5",   0x08000, 0x8000, CRC(b922e787) SHA1(16087671ec7de25f749b5fd66409d48ef7b35820) )
	ROM_LOAD( "epr-7129.ic6",   0x10000, 0x8000, CRC(bd3b6e6e) SHA1(c66f21b98cb8fc61a9318041ac1812c13099d974) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "epr-7121.ic87",  0x00000, 0x8000, CRC(f2b88f73) SHA1(2b06da1beabbea82d502fbe12f6ec3ef26056edd) )
	ROM_LOAD( "epr-7120.ic86",  0x08000, 0x8000, CRC(517d7fd3) SHA1(3fb5c00224920c3f62fb86e82caf0fee2293e1e2) )
	ROM_LOAD( "epr-7123.ic89",  0x10000, 0x8000, CRC(8f16a303) SHA1(5f2465505f001dc052e9de4cf66bc1d53fc8c7da) )
	ROM_LOAD( "epr-7122.ic88",  0x18000, 0x8000, CRC(7c93f160) SHA1(6ab156cad7556808496070f8b02a708ce405c492) )

	ROM_REGION( 0x0300, "color_proms", 0 )
	ROM_LOAD( "pr7119.ic20",    0x0000, 0x0100, CRC(b2a8260f) SHA1(36c1debb4b3f2f190a25b18d533319d7380416de) ) // palette red component
	ROM_LOAD( "pr7118.ic14",    0x0100, 0x0100, CRC(693e20c7) SHA1(9ebf4bd2c30ddd9648bc4b41c7739cfdf80100da) ) // palette green component
	ROM_LOAD( "pr7117.ic8",     0x0200, 0x0100, CRC(4124307e) SHA1(cee28d891e6ce732c43a61acb5beeafd2200cf37) ) // palette blue component

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr5317.ic28",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )

	ROM_REGION( 0x0618, "plds", 0 )
	ROM_LOAD( "315-5152.ic10",   0x00000, 0x0104, CRC(2c9229b4) SHA1(9755013afcf89f99d7a399c7e223e027761cf89a) ) // PAL16R4A
	ROM_LOAD( "315-5138.ic11",   0x00000, 0x0104, CRC(dd223015) SHA1(8d70f91b118e8653dda1efee3eaea287ae63809f) ) // TI PAL16R4NC
	ROM_LOAD( "315-5139.ic50",   0x00000, 0x00e7, CRC(943d91b0) SHA1(37c98085d580808aaeb01726a9f59705590378c4) ) // CK2605
	ROM_LOAD( "315-5025.ic7",    0x00000, 0x0104, NO_DUMP )
	ROM_LOAD( "315-5025.1c13",   0x00000, 0x0104, NO_DUMP )
	ROM_LOAD( "315-5025.ic19",   0x00000, 0x0104, NO_DUMP )
ROM_END

/*
    Choplifter (Bootleg)
    Year: 1985
    System 2



    Small Daughterboard marked 600A

      |--------------------------------------------------------|
      |                                                        |
    A |  74ls244  74ls244  74ls669  74ls669  74ls669  74ls669  |
      |                                                        |
    B |  74ls240  74ls240  74ls283  74ls283  74ls283  74ls283  |
      |                                                        |
    C |  74ls10   74ls86   74ls157  74ls157  74ls157  74ls157  |
      |                                                        |
    D |  74ls157  74ls157  74ls157  74ls139  74ls74            |
      |                                                        |
    E |  pal16r4  pal16l8  74ls161  74ls161  74ls109           |
      |                                                        |
    F |  74ls27   74ls08   74ls04   74ls74   74ls00            |
      |                                                 600A   |
      |--------------------------------------------------------|
           1        2        3        4        5        6


    Small Daughterboard marked 600B

      |--------------------------------------|
      |                              600B    |
    A |  74ls74            74ls174  pal20r4  |
      |                                      |
    B |  pal16l8  pal16l8  74ls374  74ls374  |
      |                                      |
    C |  74ls283  pal16l8  pal16l8  74ls32   |
      |                                      |
    D |  74ls283  74ls283  74ls85   74ls283  |
      |                                      |
    E |  74ls04   74ls00   74ls00   74ls32   |
      |                                      |
      |--------------------------------------|
           1        2        3        4
*/
ROM_START( chopliftbl )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ep7124bl.90",    0x00000, 0x8000, CRC(71a37932) SHA1(72b6f8949d356b3adc5248fdaa13c2a1b9c0fa70) )
	ROM_LOAD( "epr-7125.91",    0x10000, 0x8000, CRC(f5283498) SHA1(1ad40f6d7b4cd18212ee56917240c0796f1a4ec2) )
	ROM_LOAD( "epr-7126.92",    0x18000, 0x8000, CRC(dbd192ab) SHA1(03d280c82599a14fc6a2065d57c6241cdc6f1143) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-7130.126",   0x0000, 0x8000, CRC(346af118) SHA1(ef579818a45b8ebb276d5832092b26e232d5a737) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "epr-7127.4",     0x00000, 0x8000, CRC(1e708f6d) SHA1(b975e13bdc44105e7a15c2694e3ec53b60e23e5e) )
	ROM_LOAD( "epr-7128.5",     0x08000, 0x8000, CRC(b922e787) SHA1(16087671ec7de25f749b5fd66409d48ef7b35820) )
	ROM_LOAD( "epr-7129.6",     0x10000, 0x8000, CRC(bd3b6e6e) SHA1(c66f21b98cb8fc61a9318041ac1812c13099d974) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "epr-7121.87",    0x00000, 0x8000, CRC(f2b88f73) SHA1(2b06da1beabbea82d502fbe12f6ec3ef26056edd) )
	ROM_LOAD( "epr-7120.86",    0x08000, 0x8000, CRC(517d7fd3) SHA1(3fb5c00224920c3f62fb86e82caf0fee2293e1e2) )
	ROM_LOAD( "epr-7123.89",    0x10000, 0x8000, CRC(8f16a303) SHA1(5f2465505f001dc052e9de4cf66bc1d53fc8c7da) )
	ROM_LOAD( "epr-7122.88",    0x18000, 0x8000, CRC(7c93f160) SHA1(6ab156cad7556808496070f8b02a708ce405c492) )

	ROM_REGION( 0x0300, "color_proms", 0 )
	ROM_LOAD( "pr7119.20",      0x0000, 0x0100, CRC(b2a8260f) SHA1(36c1debb4b3f2f190a25b18d533319d7380416de) ) // palette red component
	ROM_LOAD( "pr7118.14",      0x0100, 0x0100, CRC(693e20c7) SHA1(9ebf4bd2c30ddd9648bc4b41c7739cfdf80100da) ) // palette green component
	ROM_LOAD( "pr7117.8",       0x0200, 0x0100, CRC(4124307e) SHA1(cee28d891e6ce732c43a61acb5beeafd2200cf37) ) // palette blue component

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr5317.28",      0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )

	ROM_REGION( 0x0003, "plds_main", 0 )
	ROM_LOAD( "pal16r4.bin",    0x00000, 0x0001, NO_DUMP ) // PAL16R4 located at IC13.
	ROM_LOAD( "pal16r4.bin",    0x00000, 0x0001, NO_DUMP ) // PAL16R4 located at IC14.
	ROM_LOAD( "pal16l8.bin",    0x00000, 0x0001, NO_DUMP ) // PAL16L8 located at IC62.

	ROM_REGION( 0x0002, "plds_600a", 0 )
	ROM_LOAD( "pal16r4.bin",    0x00000, 0x0001, NO_DUMP ) // PAL16R4 located at E1.
	ROM_LOAD( "pal16l8.bin",    0x00000, 0x0001, NO_DUMP ) // PAL16L8 located at E2.

	ROM_REGION( 0x0005, "plds_600b", 0 )
	ROM_LOAD( "pal20r4.bin",    0x00000, 0x0001, NO_DUMP ) // PAL20R4 located at A4.
	ROM_LOAD( "pal16l8.bin",    0x00000, 0x0001, NO_DUMP ) // PAL16L8 located at B1.
	ROM_LOAD( "pal16l8.bin",    0x00000, 0x0001, NO_DUMP ) // PAL16L8 located at B2.
	ROM_LOAD( "pal16l8.bin",    0x00000, 0x0001, NO_DUMP ) // PAL16L8 located at C2.
	ROM_LOAD( "pal16l8.bin",    0x00000, 0x0001, NO_DUMP ) // PAL16L8 located at C3.

	ROM_REGION( 0x0410, "plds_unk", 0 )
	// Do any of these dumps match what's on the physical boards?
	ROM_LOAD( "pal16r4a.ic9",         0x0000, 0x0104, CRC(dd223015) SHA1(8d70f91b118e8653dda1efee3eaea287ae63809f) )
	ROM_LOAD( "pal16r4a.ic10",        0x0104, 0x0104, CRC(2c9229b4) SHA1(9755013afcf89f99d7a399c7e223e027761cf89a) )
	ROM_LOAD( "pal16r4a-chopbl1.bin", 0x0208, 0x0104, CRC(e1628a8e) SHA1(6b6df079cfadec71b38a53f107475f0dda428b00) )
	ROM_LOAD( "pal16l8a-chopbl2.bin", 0x030c, 0x0104, CRC(afa7425d) SHA1(09d8607b69ecfc0b12c8610751d489500b63c7d6) )
ROM_END


ROM_START( raflesia )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-7411.116",   0x0000, 0x4000, CRC(88a0c6c6) SHA1(1deaa8d8d607100966696e5e9dd5f799ba693af0) ) // encrypted
	ROM_LOAD( "epr-7412.109",   0x4000, 0x4000, CRC(d3b8cddf) SHA1(368c74d8ae46442cacdb67813dc1c039245da266) ) // encrypted
	ROM_LOAD( "epr-7413.96",    0x8000, 0x4000, CRC(b7e688b3) SHA1(ba5c6d5d19e7d51e41949fd5fa576fdae38f9c9c) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-7420.120",   0x0000, 0x2000, CRC(14387666) SHA1(9cb18e3002c32f658e4725707069f9cd2f496507) ) // epr-7420.3

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-7419.62",    0x0000, 0x2000, CRC(bfd5f34c) SHA1(78c4d380d5558212e535c3262223137447d64818) ) // epr-7419.82
	ROM_LOAD( "epr-7418.61",    0x2000, 0x2000, CRC(f8cbc9b6) SHA1(48be9337f704a11ac1fdeb64a3b3518c796bcdd0) ) // epr-7418.65
	ROM_LOAD( "epr-7417.64",    0x4000, 0x2000, CRC(e63501bc) SHA1(5cfd19241c54782c262bbb23c6f682534e77feb7) ) // epr-7417.81
	ROM_LOAD( "epr-7416.63",    0x6000, 0x2000, CRC(093e5693) SHA1(78bb1c4651bd63a9f776766d2eac4f1c09242ed5) ) // epr-7416.64
	ROM_LOAD( "epr-7415.66",    0x8000, 0x2000, CRC(1a8d6bd6) SHA1(b04ee35f603c6c9923ba888914eb43a8b7753d92) ) // epr-7415.80
	ROM_LOAD( "epr-7414.65",    0xa000, 0x2000, CRC(5d20f218) SHA1(bdc0185d133f7bbe287106882bacde846634ffa4) ) // epr-7414.63

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-7407.117",   0x0000, 0x4000, CRC(f09fc057) SHA1(c6f06144b708055b31fbcba9f38b63736db789d8) ) // epr-7407.3
	ROM_LOAD( "epr-7409.04",    0x4000, 0x4000, CRC(819fedb8) SHA1(e63f0422814423be91d8e1937a13d19693a1a5fc) ) // epr-7409.1
	ROM_LOAD( "epr-7408.110",   0x8000, 0x4000, CRC(3189f33c) SHA1(8476c2c01920f0492cf643929d4f023f3afe0164) ) // epr-7408.4
	ROM_LOAD( "epr-7410.05",    0xc000, 0x4000, CRC(ced74789) SHA1(d0ad845bfe83412ac8d43125e1c50d0581a5b47e) ) // epr-7410.2

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) // pr-5317.106
ROM_END

// Sega 834-5191 + 834-5540 + 834-5541. This set isn't exactly the same as the encrypted version (calls and jumps are moved around)
ROM_START( raflesiau )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-7433.129",   0x0000, 0x2000, CRC(6f4931b0) SHA1(4c02e052c01099478adb5bb595c035aff862dd45) )
	ROM_LOAD( "epr-7434.130",   0x2000, 0x2000, CRC(ec46e21b) SHA1(7bba493c8493611952e029dc8782916cc25db82a) )
	ROM_LOAD( "epr-7435.131",   0x4000, 0x2000, CRC(e035ff6b) SHA1(5bffdcdf543f47ccf2ed0cabf1ef9e42857d59f9) )
	ROM_LOAD( "epr-7436.132",   0x6000, 0x2000, CRC(6527aae7) SHA1(aecf33c777e8f4a7323d72dd3f2ca5d12cc798e7) )
	ROM_LOAD( "epr-7437.133",   0x8000, 0x2000, CRC(e13dd5e4) SHA1(a97654ff82ffa051ee14359733c2b9a952134d4e) )
	ROM_LOAD( "epr-7438.134",   0xa000, 0x2000, CRC(a0aa4729) SHA1(c353ed4d21661ea0721d8b85d6bfefe502373d7b) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-7420.120",   0x0000, 0x2000, CRC(14387666) SHA1(9cb18e3002c32f658e4725707069f9cd2f496507) ) // epr-7420.3

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-7419.82",    0x0000, 0x2000, CRC(bfd5f34c) SHA1(78c4d380d5558212e535c3262223137447d64818) )
	ROM_LOAD( "epr-7418.65",    0x2000, 0x2000, CRC(f8cbc9b6) SHA1(48be9337f704a11ac1fdeb64a3b3518c796bcdd0) )
	ROM_LOAD( "epr-7417.81",    0x4000, 0x2000, CRC(e63501bc) SHA1(5cfd19241c54782c262bbb23c6f682534e77feb7) )
	ROM_LOAD( "epr-7416.64",    0x6000, 0x2000, CRC(093e5693) SHA1(78bb1c4651bd63a9f776766d2eac4f1c09242ed5) )
	ROM_LOAD( "epr-7415.80",    0x8000, 0x2000, CRC(1a8d6bd6) SHA1(b04ee35f603c6c9923ba888914eb43a8b7753d92) )
	ROM_LOAD( "epr-7414.63",    0xa000, 0x2000, CRC(5d20f218) SHA1(bdc0185d133f7bbe287106882bacde846634ffa4) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-7407.3",     0x0000, 0x4000, CRC(f09fc057) SHA1(c6f06144b708055b31fbcba9f38b63736db789d8) )
	ROM_LOAD( "epr-7409.1",     0x4000, 0x4000, CRC(819fedb8) SHA1(e63f0422814423be91d8e1937a13d19693a1a5fc) )
	ROM_LOAD( "epr-7408.4",     0x8000, 0x4000, CRC(3189f33c) SHA1(8476c2c01920f0492cf643929d4f023f3afe0164) )
	ROM_LOAD( "epr-7410.2",     0xc000, 0x4000, CRC(ced74789) SHA1(d0ad845bfe83412ac8d43125e1c50d0581a5b47e) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.106",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( wboy )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-7489.116",   0x0000, 0x4000, CRC(130f4b70) SHA1(4a2ea5bc06f3a240c68813be3a9f9bef2bcf4e9c) ) // encrypted
	ROM_LOAD( "epr-7490.109",   0x4000, 0x4000, CRC(9e656733) SHA1(2233beb874b7cb48899afe603fef567932951a88) ) // encrypted
	ROM_LOAD( "epr-7491.96",    0x8000, 0x4000, CRC(1f7d0efe) SHA1(a1b4f8faf1614f4808df1292209c340f1490adbd) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-7498.120",   0x0000, 0x2000, CRC(78ae1e7b) SHA1(86032f443359b0bb2766e33024ed2e320aa9bc84) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-7497.62",    0x0000, 0x2000, CRC(08d609ca) SHA1(11799e9ef7e6942b304f132b404bff3ed44d524b) )
	ROM_LOAD( "epr-7496.61",    0x2000, 0x2000, CRC(6f61fdf1) SHA1(21826aebf5835b9f3d9c467c8647809c1bc0d01f) )
	ROM_LOAD( "epr-7495.64",    0x4000, 0x2000, CRC(6a0d2c2d) SHA1(8c21d7f0768e8dda2b7185f3c510cae4229a4a2e) )
	ROM_LOAD( "epr-7494.63",    0x6000, 0x2000, CRC(a8e281c7) SHA1(a88b80a7b94ab1401bbf28d7707fdf28a5505127) )
	ROM_LOAD( "epr-7493.66",    0x8000, 0x2000, CRC(89305df4) SHA1(7a5098624769a31e7512f56831e818bce6a18871) )
	ROM_LOAD( "epr-7492.65",    0xa000, 0x2000, CRC(60f806b1) SHA1(f91e5868a455dff2bce3c2891a7cfd648957cd73) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-7485.117",   0x0000, 0x4000, CRC(c2891722) SHA1(e4e11c0e9bd0dc121c25349493f2b13d2ff8c807) )
	ROM_LOAD( "epr-7487.04",    0x4000, 0x4000, CRC(2d3a421b) SHA1(d70440a8703ccface3212cd9544c950b36263e8c) )
	ROM_LOAD( "epr-7486.110",   0x8000, 0x4000, CRC(8d622c50) SHA1(9a76a50204c618347d3e8eee6cda841becd906eb) )
	ROM_LOAD( "epr-7488.05",    0xc000, 0x4000, CRC(007c2f1b) SHA1(c2f1376144a49d20cb35384648e06d06978474c1) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( wboyub ) // seems to be the same as 'wboy' but a different rom layout and external encryption handling
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin",        0x0000, 0x8000, CRC(07066b6f) SHA1(1ead373907fd5bd5f4cc003a97218aa582758a00) )
	ROM_LOAD( "epr-7491.96",  0x8000, 0x4000, CRC(1f7d0efe) SHA1(a1b4f8faf1614f4808df1292209c340f1490adbd) )

	ROM_REGION( 0x10000, "unk", 0 )
	ROM_LOAD( "0cpu.bin",     0xc000, 0x2000, CRC(a962e6af) SHA1(f46b01db38cdc9c8485d7fe0a344e9f6ed918925) ) // supposedly the encryption key

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-7498a.3",    0x0000, 0x2000, CRC(c198205c) SHA1(d2d5cd154ce6a5a3c6a099b4ab2ea7cc045ab0a1) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-7497.62",    0x0000, 0x2000, CRC(08d609ca) SHA1(11799e9ef7e6942b304f132b404bff3ed44d524b) )
	ROM_LOAD( "epr-7496.61",    0x2000, 0x2000, CRC(6f61fdf1) SHA1(21826aebf5835b9f3d9c467c8647809c1bc0d01f) )
	ROM_LOAD( "epr-7495.64",    0x4000, 0x2000, CRC(6a0d2c2d) SHA1(8c21d7f0768e8dda2b7185f3c510cae4229a4a2e) )
	ROM_LOAD( "epr-7494.63",    0x6000, 0x2000, CRC(a8e281c7) SHA1(a88b80a7b94ab1401bbf28d7707fdf28a5505127) )
	ROM_LOAD( "epr-7493.66",    0x8000, 0x2000, CRC(89305df4) SHA1(7a5098624769a31e7512f56831e818bce6a18871) )
	ROM_LOAD( "epr-7492.65",    0xa000, 0x2000, CRC(60f806b1) SHA1(f91e5868a455dff2bce3c2891a7cfd648957cd73) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "4.bin",        0x0000, 0x4000, CRC(8b3124e6) SHA1(e90deaa687128c1f0b7e9e6b6d767bd484c7fc61) )
	ROM_CONTINUE(0x8000,0x4000)
	ROM_LOAD( "5.bin",        0x4000, 0x4000, CRC(b75278e7) SHA1(5b7c519f32eac40dc46ca5bba03cec1e893b6fcd) )
	ROM_CONTINUE(0xc000,0x4000)

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END



/* Wonder Boy (Escape, Sega license)
PCB: 834-3984-09 WONDER BOY
CPU: 317-0003 (encrypted Z80 @ IC137)
PAL: 315-5063 (PAL @ IC67)
*/
ROM_START( wboyo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-7532.116",   0x0000, 0x4000, CRC(51d27534) SHA1(1cbc7201aacde89857f83b2600f309b514c5e758) ) // encrypted
	ROM_LOAD( "epr-7533.109",   0x4000, 0x4000, CRC(e29d1cd1) SHA1(f6ff4a6fffea77cc5706549bb2d8bf9e96ed0be0) ) // encrypted
	ROM_LOAD( "epr-7534.96",    0x8000, 0x4000, CRC(1f7d0efe) SHA1(a1b4f8faf1614f4808df1292209c340f1490adbd) ) // same contents as epr-7491.96

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-7498.120",   0x0000, 0x2000, CRC(78ae1e7b) SHA1(86032f443359b0bb2766e33024ed2e320aa9bc84) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-7497.62",    0x0000, 0x2000, CRC(08d609ca) SHA1(11799e9ef7e6942b304f132b404bff3ed44d524b) )
	ROM_LOAD( "epr-7496.61",    0x2000, 0x2000, CRC(6f61fdf1) SHA1(21826aebf5835b9f3d9c467c8647809c1bc0d01f) )
	ROM_LOAD( "epr-7495.64",    0x4000, 0x2000, CRC(6a0d2c2d) SHA1(8c21d7f0768e8dda2b7185f3c510cae4229a4a2e) )
	ROM_LOAD( "epr-7494.63",    0x6000, 0x2000, CRC(a8e281c7) SHA1(a88b80a7b94ab1401bbf28d7707fdf28a5505127) )
	ROM_LOAD( "epr-7493.66",    0x8000, 0x2000, CRC(89305df4) SHA1(7a5098624769a31e7512f56831e818bce6a18871) )
	ROM_LOAD( "epr-7492.65",    0xa000, 0x2000, CRC(60f806b1) SHA1(f91e5868a455dff2bce3c2891a7cfd648957cd73) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-7485.117",   0x0000, 0x4000, CRC(c2891722) SHA1(e4e11c0e9bd0dc121c25349493f2b13d2ff8c807) )
	ROM_LOAD( "epr-7487.04",    0x4000, 0x4000, CRC(2d3a421b) SHA1(d70440a8703ccface3212cd9544c950b36263e8c) )
	ROM_LOAD( "epr-7486.110",   0x8000, 0x4000, CRC(8d622c50) SHA1(9a76a50204c618347d3e8eee6cda841becd906eb) )
	ROM_LOAD( "epr-7488.05",    0xc000, 0x4000, CRC(007c2f1b) SHA1(c2f1376144a49d20cb35384648e06d06978474c1) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( wboy2 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-7587.129",   0x0000, 0x2000, CRC(1bbb7354) SHA1(e299979299c93981f5d28a1a614ad644506911dd) ) // encrypted
	ROM_LOAD( "epr-7588.130",   0x2000, 0x2000, CRC(21007413) SHA1(f45443a49e916465e5c8a8b348897ab426a897bd) ) // encrypted
	ROM_LOAD( "epr-7589.131",   0x4000, 0x2000, CRC(44b30433) SHA1(558d799c8f48f76c651f19e2a81160eb78ac6642) ) // encrypted
	ROM_LOAD( "epr-7590.132",   0x6000, 0x2000, CRC(bb525a0b) SHA1(5cd4731e0adfb5c660144eccda759e12a30ce78e) ) // encrypted
	ROM_LOAD( "epr-7591.133",   0x8000, 0x2000, CRC(8379aa23) SHA1(da47e0150b724a00878ef5f953fa6ac80bb27d8d) )
	ROM_LOAD( "epr-7592.134",   0xa000, 0x2000, CRC(c767a5d7) SHA1(a4e8d6a8278ac2227bde8c24d45aa7ab2a273579) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-7498.120",   0x0000, 0x2000, CRC(78ae1e7b) SHA1(86032f443359b0bb2766e33024ed2e320aa9bc84) ) // epr-7498.3

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-7497.62",    0x0000, 0x2000, CRC(08d609ca) SHA1(11799e9ef7e6942b304f132b404bff3ed44d524b) ) // epr-7497.82
	ROM_LOAD( "epr-7496.61",    0x2000, 0x2000, CRC(6f61fdf1) SHA1(21826aebf5835b9f3d9c467c8647809c1bc0d01f) ) // epr-7496.65
	ROM_LOAD( "epr-7495.64",    0x4000, 0x2000, CRC(6a0d2c2d) SHA1(8c21d7f0768e8dda2b7185f3c510cae4229a4a2e) ) // epr-7495.81
	ROM_LOAD( "epr-7494.63",    0x6000, 0x2000, CRC(a8e281c7) SHA1(a88b80a7b94ab1401bbf28d7707fdf28a5505127) ) // epr-7494.64
	ROM_LOAD( "epr-7493.66",    0x8000, 0x2000, CRC(89305df4) SHA1(7a5098624769a31e7512f56831e818bce6a18871) ) // epr-7493.80
	ROM_LOAD( "epr-7492.65",    0xa000, 0x2000, CRC(60f806b1) SHA1(f91e5868a455dff2bce3c2891a7cfd648957cd73) ) // epr-7492.63

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-7485.117",   0x0000, 0x4000, CRC(c2891722) SHA1(e4e11c0e9bd0dc121c25349493f2b13d2ff8c807) ) // epr-7485.3
	ROM_LOAD( "epr-7487.04",    0x4000, 0x4000, CRC(2d3a421b) SHA1(d70440a8703ccface3212cd9544c950b36263e8c) ) // epr-7487.1
	ROM_LOAD( "epr-7486.110",   0x8000, 0x4000, CRC(8d622c50) SHA1(9a76a50204c618347d3e8eee6cda841becd906eb) ) // epr-7486.4
	ROM_LOAD( "epr-7488.05",    0xc000, 0x4000, CRC(007c2f1b) SHA1(c2f1376144a49d20cb35384648e06d06978474c1) ) // epr-7488.2

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) // pr-5317.106
ROM_END

ROM_START( wboy2u )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic129_02.bin",   0x0000, 0x2000, CRC(32c4b709) SHA1(e57b7b6818f12fdd5f1600ed54c0b8a7f538aa71) )
	ROM_LOAD( "ic130_03.bin",   0x2000, 0x2000, CRC(56463ede) SHA1(c58c220aa0d0e194581646e6db2491075fdc37b9) )
	ROM_LOAD( "ic131_04.bin",   0x4000, 0x2000, CRC(775ed392) SHA1(073f8f70685913736eb04be8215a47b5253cb531) )
	ROM_LOAD( "ic132_05.bin",   0x6000, 0x2000, CRC(7b922708) SHA1(c2e1f67b756f558d6904fe82d6f5483cda5f9045) )
	ROM_LOAD( "epr-7591.133",   0x8000, 0x2000, CRC(8379aa23) SHA1(da47e0150b724a00878ef5f953fa6ac80bb27d8d) )
	ROM_LOAD( "epr-7592.134",   0xa000, 0x2000, CRC(c767a5d7) SHA1(a4e8d6a8278ac2227bde8c24d45aa7ab2a273579) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr7498a.3",     0x0000, 0x2000, CRC(c198205c) SHA1(d2d5cd154ce6a5a3c6a099b4ab2ea7cc045ab0a1) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-7497.62",    0x0000, 0x2000, CRC(08d609ca) SHA1(11799e9ef7e6942b304f132b404bff3ed44d524b) ) // epr-7497.82
	ROM_LOAD( "epr-7496.61",    0x2000, 0x2000, CRC(6f61fdf1) SHA1(21826aebf5835b9f3d9c467c8647809c1bc0d01f) ) // epr-7496.65
	ROM_LOAD( "epr-7495.64",    0x4000, 0x2000, CRC(6a0d2c2d) SHA1(8c21d7f0768e8dda2b7185f3c510cae4229a4a2e) ) // epr-7495.81
	ROM_LOAD( "epr-7494.63",    0x6000, 0x2000, CRC(a8e281c7) SHA1(a88b80a7b94ab1401bbf28d7707fdf28a5505127) ) // epr-7494.64
	ROM_LOAD( "epr-7493.66",    0x8000, 0x2000, CRC(89305df4) SHA1(7a5098624769a31e7512f56831e818bce6a18871) ) // epr-7493.80
	ROM_LOAD( "epr-7492.65",    0xa000, 0x2000, CRC(60f806b1) SHA1(f91e5868a455dff2bce3c2891a7cfd648957cd73) ) // epr-7492.63

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-7485.117",   0x0000, 0x4000, CRC(c2891722) SHA1(e4e11c0e9bd0dc121c25349493f2b13d2ff8c807) ) // epr-7485.3
	ROM_LOAD( "epr-7487.04",    0x4000, 0x4000, CRC(2d3a421b) SHA1(d70440a8703ccface3212cd9544c950b36263e8c) ) // epr-7487.1
	ROM_LOAD( "epr-7486.110",   0x8000, 0x4000, CRC(8d622c50) SHA1(9a76a50204c618347d3e8eee6cda841becd906eb) ) // epr-7486.4
	ROM_LOAD( "epr-7488.05",    0xc000, 0x4000, CRC(007c2f1b) SHA1(c2f1376144a49d20cb35384648e06d06978474c1) ) // epr-7488.2

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) // pr-5317.106
ROM_END

ROM_START( wboy3 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wb_1",           0x0000, 0x4000, CRC(bd6fef49) SHA1(6469a84cc1fd4ebf8c58b6efd3b255414bc86699) ) // encrypted
	ROM_LOAD( "wb_2",           0x4000, 0x4000, CRC(4081b624) SHA1(892fd347638ec900a7afc3d338b68e9d0a14f2b4) ) // encrypted
	ROM_LOAD( "wb_3",           0x8000, 0x4000, CRC(c48a0e36) SHA1(c9b9e51334e8b698be2195dda7701bb51760e502) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-7498.120",   0x0000, 0x2000, CRC(78ae1e7b) SHA1(86032f443359b0bb2766e33024ed2e320aa9bc84) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-7497.62",    0x0000, 0x2000, CRC(08d609ca) SHA1(11799e9ef7e6942b304f132b404bff3ed44d524b) )
	ROM_LOAD( "epr-7496.61",    0x2000, 0x2000, CRC(6f61fdf1) SHA1(21826aebf5835b9f3d9c467c8647809c1bc0d01f) )
	ROM_LOAD( "epr-7495.64",    0x4000, 0x2000, CRC(6a0d2c2d) SHA1(8c21d7f0768e8dda2b7185f3c510cae4229a4a2e) )
	ROM_LOAD( "epr-7494.63",    0x6000, 0x2000, CRC(a8e281c7) SHA1(a88b80a7b94ab1401bbf28d7707fdf28a5505127) )
	ROM_LOAD( "epr-7493.66",    0x8000, 0x2000, CRC(89305df4) SHA1(7a5098624769a31e7512f56831e818bce6a18871) )
	ROM_LOAD( "epr-7492.65",    0xa000, 0x2000, CRC(60f806b1) SHA1(f91e5868a455dff2bce3c2891a7cfd648957cd73) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-7485.117",   0x0000, 0x4000, CRC(c2891722) SHA1(e4e11c0e9bd0dc121c25349493f2b13d2ff8c807) )
	ROM_LOAD( "epr-7487.04",    0x4000, 0x4000, CRC(2d3a421b) SHA1(d70440a8703ccface3212cd9544c950b36263e8c) )
	ROM_LOAD( "epr-7486.110",   0x8000, 0x4000, CRC(8d622c50) SHA1(9a76a50204c618347d3e8eee6cda841becd906eb) )
	ROM_LOAD( "epr-7488.05",    0xc000, 0x4000, CRC(007c2f1b) SHA1(c2f1376144a49d20cb35384648e06d06978474c1) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

 // this bootleg by Tecfri has a small daughterboard with standard Z80, PAL and PROM. Code wise it's extremely similar to wboy3, with a small routine added at the end of ROM 3.
ROM_START( wboyblt )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.bin", 0x0000, 0x4000, CRC(67d5af52) SHA1(92d08d65b0e54f5c56dfa7a583650fd6b18e2c81) ) // encrypted
	ROM_LOAD( "2.bin", 0x4000, 0x4000, CRC(e0d36862) SHA1(5e25cee7f972c571edddb5b18682817fbf2e72b3) ) // encrypted
	ROM_LOAD( "3.bin", 0x8000, 0x4000, CRC(8dba0aad) SHA1(fd964ee39046e46f2685eb5f49144744001758e8) )

	// sound and GFX ROMs match the original wboy sets
	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "14.bin", 0x0000, 0x2000, CRC(78ae1e7b) SHA1(86032f443359b0bb2766e33024ed2e320aa9bc84) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "9.bin",  0x0000, 0x2000, CRC(08d609ca) SHA1(11799e9ef7e6942b304f132b404bff3ed44d524b) )
	ROM_LOAD( "8.bin",  0x2000, 0x2000, CRC(6f61fdf1) SHA1(21826aebf5835b9f3d9c467c8647809c1bc0d01f) )
	ROM_LOAD( "11.bin", 0x4000, 0x2000, CRC(6a0d2c2d) SHA1(8c21d7f0768e8dda2b7185f3c510cae4229a4a2e) )
	ROM_LOAD( "10.bin", 0x6000, 0x2000, CRC(a8e281c7) SHA1(a88b80a7b94ab1401bbf28d7707fdf28a5505127) )
	ROM_LOAD( "13.bin", 0x8000, 0x2000, CRC(89305df4) SHA1(7a5098624769a31e7512f56831e818bce6a18871) )
	ROM_LOAD( "12.bin", 0xa000, 0x2000, CRC(60f806b1) SHA1(f91e5868a455dff2bce3c2891a7cfd648957cd73) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "4.bin",  0x0000, 0x4000, CRC(c2891722) SHA1(e4e11c0e9bd0dc121c25349493f2b13d2ff8c807) )
	ROM_LOAD( "6.bin",  0x4000, 0x4000, CRC(2d3a421b) SHA1(d70440a8703ccface3212cd9544c950b36263e8c) )
	ROM_LOAD( "5.bin",  0x8000, 0x4000, CRC(8d622c50) SHA1(9a76a50204c618347d3e8eee6cda841becd906eb) )
	ROM_LOAD( "7.bin",  0xc000, 0x4000, CRC(007c2f1b) SHA1(c2f1376144a49d20cb35384648e06d06978474c1) )

	ROM_REGION( 0x0100, "lookup_proms", 0 ) // not dumped for this set, but should match
	ROM_LOAD( "pr-5317.76", 0x0000, 0x0100, BAD_DUMP CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )

	// the following are on the small daughterboard along with the Z80
	ROM_REGION( 0x0100, "decryption_prom", 0 )
	ROM_LOAD( "tbp24s10n", 0x0000, 0x0100, NO_DUMP )

	ROM_REGION( 0x00cc, "decryption_pal", 0 )
	ROM_LOAD( "pal20l10cns", 0x0000, 0x00cc, NO_DUMP )
ROM_END

/*
This wonderboy romset runs on a system1 1985 pcb with some flying wires.
Serial number of the pcb is 257

There are 2 piggyback boards:

The first is marked "SEGA 834-5764"  and it is placed on the socket of the sega sys1 protection chip and on a eprom socket.
there are IC1 and IC2 eproms (triple checked - can be easy to mis-read).  There is also a 40 pin socket in which they have
put an unknown 42 NEC cpu (they have scratched the codes) with pin 21 and 22 cut!

The second piggyback is marked "SEGA 834-5755" and it contains proms and some logic.
*/

ROM_START( wboy4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-7622.ic1",  0x0000, 0x8000, CRC(48b2c006) SHA1(35492330dae71d410712380466b4c09b81df8559) ) // encrypted
	ROM_LOAD( "epr-7621.ic2",  0x8000, 0x8000, CRC(466cae31) SHA1(e47e9084c83796a0a0dfeaa1f8f868cadd5f32c7) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-7583.126",  0x0000, 0x8000, CRC(99334b3c) SHA1(dfc09f63082b7666fa2152e22810c0455a7e5051) ) // epr7583.ic120

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-7610.ic62", 0x0000, 0x4000, CRC(1685d26a) SHA1(d30d08d61d789fd5a0eb7ef2998eb9728dabf4c9) )
	ROM_LOAD( "epr-7609.ic64", 0x4000, 0x4000, CRC(87ecba53) SHA1(b904d5af25e0c1f8c8ca8dc11a3bed508c868f19) )
	ROM_LOAD( "epr-7608.ic66", 0x8000, 0x4000, CRC(e812b3ec) SHA1(3eebeaf3480a0370aa5ee031c25768ada17ad8a2) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-7578.87",  0x00000, 0x8000, CRC(6ff1637f) SHA1(9a6ddbd7b8d53273b30c3529b028c1f28bf3c63b) ) // epr7577.ic110
	ROM_LOAD( "epr-7577.86",  0x08000, 0x8000, CRC(58b3705e) SHA1(1a8ff3f1765a3b21145bd1a6c85441f806f7b17d) ) // epr7576.ic117

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( wboy5 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wb1.ic116", 0x0000, 0x4000, CRC(6c67407c) SHA1(58d567ee46470cfdf7f1a539fabeb9f0e3c9e6ff) ) // encrypted
	ROM_LOAD( "wb_2",      0x4000, 0x4000, CRC(4081b624) SHA1(892fd347638ec900a7afc3d338b68e9d0a14f2b4) ) // encrypted
	ROM_LOAD( "wb_3",      0x8000, 0x4000, CRC(c48a0e36) SHA1(c9b9e51334e8b698be2195dda7701bb51760e502) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-7498.120", 0x0000, 0x2000, CRC(78ae1e7b) SHA1(86032f443359b0bb2766e33024ed2e320aa9bc84) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-7497.62", 0x0000, 0x2000, CRC(08d609ca) SHA1(11799e9ef7e6942b304f132b404bff3ed44d524b) )
	ROM_LOAD( "epr-7496.61", 0x2000, 0x2000, CRC(6f61fdf1) SHA1(21826aebf5835b9f3d9c467c8647809c1bc0d01f) )
	ROM_LOAD( "epr-7495.64", 0x4000, 0x2000, CRC(6a0d2c2d) SHA1(8c21d7f0768e8dda2b7185f3c510cae4229a4a2e) )
	ROM_LOAD( "epr-7494.63", 0x6000, 0x2000, CRC(a8e281c7) SHA1(a88b80a7b94ab1401bbf28d7707fdf28a5505127) )
	ROM_LOAD( "epr-7493.66", 0x8000, 0x2000, CRC(89305df4) SHA1(7a5098624769a31e7512f56831e818bce6a18871) )
	ROM_LOAD( "epr-7492.65", 0xa000, 0x2000, CRC(60f806b1) SHA1(f91e5868a455dff2bce3c2891a7cfd648957cd73) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-7485.117", 0x0000, 0x4000, CRC(c2891722) SHA1(e4e11c0e9bd0dc121c25349493f2b13d2ff8c807) )
	ROM_LOAD( "epr-7487.04",  0x4000, 0x4000, CRC(2d3a421b) SHA1(d70440a8703ccface3212cd9544c950b36263e8c) )
	ROM_LOAD( "epr-7486.110", 0x8000, 0x4000, CRC(8d622c50) SHA1(9a76a50204c618347d3e8eee6cda841becd906eb) )
	ROM_LOAD( "epr-7488.05",  0xc000, 0x4000, CRC(007c2f1b) SHA1(c2f1376144a49d20cb35384648e06d06978474c1) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76", 0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( wboy6 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "epr-7731.129",   0x0000, 0x2000, CRC(9776ceb6) SHA1(14a09cb397e99f2a503242c1feb7e13bd4394536) ) // encrypted
	ROM_LOAD( "epr-7732.130",   0x2000, 0x2000, CRC(74ff0918) SHA1(1a4124173f39bfbb471d5e33c8572c09a278965d) ) // encrypted
	ROM_LOAD( "epr-7733.131",   0x4000, 0x2000, CRC(07c8b494) SHA1(14b9bf9d97eb633378cee8964608f56ab63eb6d3) ) // encrypted
	ROM_LOAD( "epr-7734.132",   0x6000, 0x2000, CRC(8ebd648c) SHA1(03e5b19dc59a8118641a16b8a8fd583d51fcad28) ) // encrypted
	ROM_LOAD( "epr-7735.133",   0x8000, 0x2000, CRC(8379aa23) SHA1(da47e0150b724a00878ef5f953fa6ac80bb27d8d) )
	ROM_LOAD( "epr-7736.134",   0xa000, 0x2000, CRC(c767a5d7) SHA1(a4e8d6a8278ac2227bde8c24d45aa7ab2a273579) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-7498.120",   0x0000, 0x2000, CRC(78ae1e7b) SHA1(86032f443359b0bb2766e33024ed2e320aa9bc84) ) // epr-7498.3

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-7497.62",    0x0000, 0x2000, CRC(08d609ca) SHA1(11799e9ef7e6942b304f132b404bff3ed44d524b) ) // epr-7497.82
	ROM_LOAD( "epr-7496.61",    0x2000, 0x2000, CRC(6f61fdf1) SHA1(21826aebf5835b9f3d9c467c8647809c1bc0d01f) ) // epr-7496.65
	ROM_LOAD( "epr-7495.64",    0x4000, 0x2000, CRC(6a0d2c2d) SHA1(8c21d7f0768e8dda2b7185f3c510cae4229a4a2e) ) // epr-7495.81
	ROM_LOAD( "epr-7494.63",    0x6000, 0x2000, CRC(a8e281c7) SHA1(a88b80a7b94ab1401bbf28d7707fdf28a5505127) ) // epr-7494.64
	ROM_LOAD( "epr-7493.66",    0x8000, 0x2000, CRC(89305df4) SHA1(7a5098624769a31e7512f56831e818bce6a18871) ) // epr-7493.80
	ROM_LOAD( "epr-7492.65",    0xa000, 0x2000, CRC(60f806b1) SHA1(f91e5868a455dff2bce3c2891a7cfd648957cd73) ) // epr-7492.63

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-7485.117",   0x0000, 0x4000, CRC(c2891722) SHA1(e4e11c0e9bd0dc121c25349493f2b13d2ff8c807) ) // epr-7485.3
	ROM_LOAD( "epr-7487.04",    0x4000, 0x4000, CRC(2d3a421b) SHA1(d70440a8703ccface3212cd9544c950b36263e8c) ) // epr-7487.1
	ROM_LOAD( "epr-7486.110",   0x8000, 0x4000, CRC(8d622c50) SHA1(9a76a50204c618347d3e8eee6cda841becd906eb) ) // epr-7486.4
	ROM_LOAD( "epr-7488.05",    0xc000, 0x4000, CRC(007c2f1b) SHA1(c2f1376144a49d20cb35384648e06d06978474c1) ) // epr-7488.2

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) // pr-5317.106
ROM_END

ROM_START( wboyu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ic116_89.bin",   0x0000, 0x4000, CRC(73d8cef0) SHA1(a6f1f8de44a88f995836ce03b5a073306c56aaeb) )
	ROM_LOAD( "ic109_90.bin",   0x4000, 0x4000, CRC(29546828) SHA1(905d76bc2b212a161ad2f2bef144261bb73c94cb) )
	ROM_LOAD( "ic096_91.bin",   0x8000, 0x4000, CRC(c7145c2a) SHA1(0b2fd6f519a4b87bc27db5d03c489c7ff75e942a) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-7498.120",   0x0000, 0x2000, CRC(78ae1e7b) SHA1(86032f443359b0bb2766e33024ed2e320aa9bc84) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-7497.62",    0x0000, 0x2000, CRC(08d609ca) SHA1(11799e9ef7e6942b304f132b404bff3ed44d524b) ) // epr-7497.82
	ROM_LOAD( "epr-7496.61",    0x2000, 0x2000, CRC(6f61fdf1) SHA1(21826aebf5835b9f3d9c467c8647809c1bc0d01f) ) // epr-7496.65
	ROM_LOAD( "epr-7495.64",    0x4000, 0x2000, CRC(6a0d2c2d) SHA1(8c21d7f0768e8dda2b7185f3c510cae4229a4a2e) ) // epr-7495.81
	ROM_LOAD( "epr-7494.63",    0x6000, 0x2000, CRC(a8e281c7) SHA1(a88b80a7b94ab1401bbf28d7707fdf28a5505127) ) // epr-7494.64
	ROM_LOAD( "epr-7493.66",    0x8000, 0x2000, CRC(89305df4) SHA1(7a5098624769a31e7512f56831e818bce6a18871) ) // epr-7493.80
	ROM_LOAD( "epr-7492.65",    0xa000, 0x2000, CRC(60f806b1) SHA1(f91e5868a455dff2bce3c2891a7cfd648957cd73) ) // epr-7492.63

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "ic117_85.bin",   0x0000, 0x4000, CRC(1ee96ae8) SHA1(4e69b87e919894b961477e6cc5272f448495d847) )
	ROM_LOAD( "ic004_87.bin",   0x4000, 0x4000, CRC(119735bb) SHA1(001efa55d7fbcd2fdb6da17b136f295e5ea4a4c2) )
	ROM_LOAD( "ic110_86.bin",   0x8000, 0x4000, CRC(26d0fac4) SHA1(2e6a06f6850b2d19e3dd7dcdc6b700d0eda878cb) )
	ROM_LOAD( "ic005_88.bin",   0xc000, 0x4000, CRC(2602e519) SHA1(00e94ec7ae37b5063137d4d49af7806fb0357c4b) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.76",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END


ROM_START( wboysys2 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "epr-7580.90",  0x00000, 0x8000, CRC(d69927a5) SHA1(b633177146a83953131d4e03fa987416f222199a) ) // encrypted
	ROM_LOAD( "epr-7579.91",  0x10000, 0x8000, CRC(8a6f4b00) SHA1(2b1c26daa2e9c668292db73e28318257c62b175c) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-7583.126", 0x0000, 0x8000, CRC(99334b3c) SHA1(dfc09f63082b7666fa2152e22810c0455a7e5051) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "epr-7581.4",   0x00000, 0x8000, CRC(d95565fd) SHA1(25f1653cca1d6432171a7b391cbb76bc18ddfb06) )
	ROM_LOAD( "epr-7582.5",   0x08000, 0x8000, CRC(560cbac0) SHA1(851283e6d63e33d250840501dd22750b19772fb0) )
	ROM_LOAD( "epr-7607.6",   0x10000, 0x8000, CRC(bd36df03) SHA1(7f7efac2c71fae48dd1dcb4dcc849f07e8127f7d) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-7578.87",  0x00000, 0x8000, CRC(6ff1637f) SHA1(9a6ddbd7b8d53273b30c3529b028c1f28bf3c63b) )
	ROM_LOAD( "epr-7577.86",  0x08000, 0x8000, CRC(58b3705e) SHA1(1a8ff3f1765a3b21145bd1a6c85441f806f7b17d) )

	ROM_REGION( 0x0300, "color_proms", 0 )
	ROM_LOAD( "pr-7345.ic20", 0x0000, 0x0100, CRC(8eee0f72) SHA1(b5694c120f604a5f7cc95618a71ab16a1a6151ed) ) // palette red component
	ROM_LOAD( "pr-7344.ic14", 0x0100, 0x0100, CRC(3e7babd7) SHA1(d4f8790db4dce75e27156a4c6de2dcef2baf6d76) ) // palette green component
	ROM_LOAD( "pr-7343.ic8",  0x0200, 0x0100, CRC(371c44a6) SHA1(ac37458d1feb6566b09a795b20c21953d4ab109d) ) // palette blue component

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.28",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( wboysys2a )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "epr-7625.90",  0x00000, 0x8000, CRC(43b3d155) SHA1(40cc063bf64327250be7ae078ea85315ad4db794) ) // encrypted
	ROM_LOAD( "epr-7626.91",  0x10000, 0x8000, CRC(8a6f4b00) SHA1(2b1c26daa2e9c668292db73e28318257c62b175c) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-7583.126", 0x0000, 0x8000, CRC(99334b3c) SHA1(dfc09f63082b7666fa2152e22810c0455a7e5051) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "epr-7581.4",   0x00000, 0x8000, CRC(d95565fd) SHA1(25f1653cca1d6432171a7b391cbb76bc18ddfb06) )
	ROM_LOAD( "epr-7582.5",   0x08000, 0x8000, CRC(560cbac0) SHA1(851283e6d63e33d250840501dd22750b19772fb0) )
	ROM_LOAD( "epr-7607.6",   0x10000, 0x8000, CRC(bd36df03) SHA1(7f7efac2c71fae48dd1dcb4dcc849f07e8127f7d) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-7578.87",  0x00000, 0x8000, CRC(6ff1637f) SHA1(9a6ddbd7b8d53273b30c3529b028c1f28bf3c63b) )
	ROM_LOAD( "epr-7577.86",  0x08000, 0x8000, CRC(58b3705e) SHA1(1a8ff3f1765a3b21145bd1a6c85441f806f7b17d) )

	ROM_REGION( 0x0300, "color_proms", 0 )
	ROM_LOAD( "pr-7345.20", 0x0000, 0x0100, CRC(8eee0f72) SHA1(b5694c120f604a5f7cc95618a71ab16a1a6151ed) ) // palette red component
	ROM_LOAD( "pr-7344.14", 0x0100, 0x0100, CRC(3e7babd7) SHA1(d4f8790db4dce75e27156a4c6de2dcef2baf6d76) ) // palette green component
	ROM_LOAD( "pr-7343.8",  0x0200, 0x0100, CRC(371c44a6) SHA1(ac37458d1feb6566b09a795b20c21953d4ab109d) ) // palette blue component

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.28",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( wbdeluxe )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "wbd1.bin",       0x0000, 0x2000, CRC(a1bedbd7) SHA1(32d171847ca02b01a7ac810cac3185c81c923285) )
	ROM_LOAD( "ic130_03.bin",   0x2000, 0x2000, CRC(56463ede) SHA1(c58c220aa0d0e194581646e6db2491075fdc37b9) )
	ROM_LOAD( "wbd3.bin",       0x4000, 0x2000, CRC(6fcdbd4c) SHA1(4fb9a916c99bf267c0035cb80b16400732991f83) )
	ROM_LOAD( "ic132_05.bin",   0x6000, 0x2000, CRC(7b922708) SHA1(c2e1f67b756f558d6904fe82d6f5483cda5f9045) )
	ROM_LOAD( "wbd5.bin",       0x8000, 0x2000, CRC(f6b02902) SHA1(9a43b84d9537d70e9c0d75010a824bcaec57a50c) )
	ROM_LOAD( "wbd6.bin",       0xa000, 0x2000, CRC(43df21fe) SHA1(c1b88505942f48b0df2362bbb618689febe00d1f) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr7498a.3",     0x0000, 0x2000, CRC(c198205c) SHA1(d2d5cd154ce6a5a3c6a099b4ab2ea7cc045ab0a1) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-7497.62",    0x0000, 0x2000, CRC(08d609ca) SHA1(11799e9ef7e6942b304f132b404bff3ed44d524b) ) // epr-7497.82
	ROM_LOAD( "epr-7496.61",    0x2000, 0x2000, CRC(6f61fdf1) SHA1(21826aebf5835b9f3d9c467c8647809c1bc0d01f) ) // epr-7496.65
	ROM_LOAD( "epr-7495.64",    0x4000, 0x2000, CRC(6a0d2c2d) SHA1(8c21d7f0768e8dda2b7185f3c510cae4229a4a2e) ) // epr-7495.81
	ROM_LOAD( "epr-7494.63",    0x6000, 0x2000, CRC(a8e281c7) SHA1(a88b80a7b94ab1401bbf28d7707fdf28a5505127) ) // epr-7494.64
	ROM_LOAD( "epr-7493.66",    0x8000, 0x2000, CRC(89305df4) SHA1(7a5098624769a31e7512f56831e818bce6a18871) ) // epr-7493.80
	ROM_LOAD( "epr-7492.65",    0xa000, 0x2000, CRC(60f806b1) SHA1(f91e5868a455dff2bce3c2891a7cfd648957cd73) ) // epr-7492.63

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "epr-7485.117",   0x0000, 0x4000, CRC(c2891722) SHA1(e4e11c0e9bd0dc121c25349493f2b13d2ff8c807) ) // epr-7485.3
	ROM_LOAD( "epr-7487.04",    0x4000, 0x4000, CRC(2d3a421b) SHA1(d70440a8703ccface3212cd9544c950b36263e8c) ) // epr-7487.1
	ROM_LOAD( "epr-7486.110",   0x8000, 0x4000, CRC(8d622c50) SHA1(9a76a50204c618347d3e8eee6cda841becd906eb) ) // epr-7486.4
	ROM_LOAD( "epr-7488.05",    0xc000, 0x4000, CRC(007c2f1b) SHA1(c2f1376144a49d20cb35384648e06d06978474c1) ) // epr-7488.2

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.106",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) // pr-5317.106
ROM_END


ROM_START( gardia ) // Factory conversion, Sega game ID# 834-6119-04 GARDIA (CVT)
	ROM_REGION( 0x20000, "maincpu", 0 ) // These 3 EPROMs located on a Sega 834-5764 daughter card with a 315-5134 PAL?
	ROM_LOAD( "epr-10255.1",   0x00000, 0x8000, CRC(89282a6b) SHA1(f19e345e5fae6a518276cc1bd09d1e2083672b25) ) // encrypted
	ROM_LOAD( "epr-10254.2",   0x10000, 0x8000, CRC(2826b6d8) SHA1(de1faf33cca031b72052bf5244fcb0bd79d85659) )
	ROM_LOAD( "epr-10253.3",   0x18000, 0x8000, CRC(7911260f) SHA1(44196f0a6c4c2b22a68609ddfc75be6a7877a69a) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-10243.120", 0x0000, 0x4000, CRC(87220660) SHA1(3f2bfc03e0f1053a4aa0ec5ebb0d573f2e20964c) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "epr-10249.61",  0x0000, 0x4000, CRC(4e0ad0f2) SHA1(b76c155b674f3ad8938278d5dbb0452351c716a5) )
	ROM_LOAD( "epr-10248.64",  0x4000, 0x4000, CRC(3515d124) SHA1(39b28a103d8bfe702a376ebd880d6060e3d1ab30) )
	ROM_LOAD( "epr-10247.66",  0x8000, 0x4000, CRC(541e1555) SHA1(6660204c74a9f7e63b3ba08d99fb854aa863710e) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "epr-10234.117", 0x00000, 0x8000, CRC(8a6aed33) SHA1(044836885ace8294124b1be9b3a4828f772bb9ee) )
	ROM_LOAD( "epr-10233.110", 0x08000, 0x8000, CRC(c52784d3) SHA1(b37d7f261be12616dbe11dfa375eaf6878e4a0f3) )
	ROM_LOAD( "epr-10236.04",  0x10000, 0x8000, CRC(b35ab227) SHA1(616f6097afddffa9af89fe84d8b6df59c567c1e6) )
	ROM_LOAD( "epr-10235.5",   0x18000, 0x8000, CRC(006a3151) SHA1(a575f9d5c026e6b18e990720ec7520b6b5ae94e3) )

	ROM_REGION( 0x0300, "color_proms", 0 ) // BPROMs located on a Sega 834-5755 daughter card (N82S129AN or equivalent)
	ROM_LOAD( "pr-7345.3",      0x0000, 0x0100, CRC(8eee0f72) SHA1(b5694c120f604a5f7cc95618a71ab16a1a6151ed) ) // palette red component
	ROM_LOAD( "pr-7344.2",      0x0100, 0x0100, CRC(3e7babd7) SHA1(d4f8790db4dce75e27156a4c6de2dcef2baf6d76) ) // palette green component
	ROM_LOAD( "pr-7343.1",      0x0200, 0x0100, CRC(371c44a6) SHA1(ac37458d1feb6566b09a795b20c21953d4ab109d) ) // palette blue component

	ROM_REGION( 0x0100, "lookup_proms", 0 ) // BPROM located on a Sega 834-5755 daughter card
	ROM_LOAD( "pr5317.4",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( gardiab )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "gardiabl.5",   0x00000, 0x8000, CRC(207f9cbb) SHA1(647de15ac69a904344f3c18c9da8cefd626387db) ) // encrypted
	ROM_LOAD( "gardiabl.6",   0x10000, 0x8000, CRC(b2ed05dc) SHA1(c520bf7024c85dc759c27eccb0a31998f4d72b5f) )
	ROM_LOAD( "gardiabl.7",   0x18000, 0x8000, CRC(0a490588) SHA1(18df754ebdf062096f2d631a722b168901610345) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-10243.120", 0x0000, 0x4000, CRC(87220660) SHA1(3f2bfc03e0f1053a4aa0ec5ebb0d573f2e20964c) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "gardiabl.8",   0x0000, 0x4000, CRC(367c9a17) SHA1(bde7592ce94bbc6674c04b427c52e74207066f56) )
	ROM_LOAD( "gardiabl.9",   0x4000, 0x4000, CRC(1540fd30) SHA1(e2d134e0715231a428fd112be81493a0e2a2642f) )
	ROM_LOAD( "gardiabl.10",  0x8000, 0x4000, CRC(e5c9af10) SHA1(6bff5bbc0f339e84a8e31446dc9897c02600fbcf) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "epr-10234.117", 0x00000, 0x8000, CRC(8a6aed33) SHA1(044836885ace8294124b1be9b3a4828f772bb9ee) )
	ROM_LOAD( "epr-10233.110", 0x08000, 0x8000, CRC(c52784d3) SHA1(b37d7f261be12616dbe11dfa375eaf6878e4a0f3) )
	ROM_LOAD( "epr-10236.04",  0x10000, 0x8000, CRC(b35ab227) SHA1(616f6097afddffa9af89fe84d8b6df59c567c1e6) )
	ROM_LOAD( "epr-10235.5",   0x18000, 0x8000, CRC(006a3151) SHA1(a575f9d5c026e6b18e990720ec7520b6b5ae94e3) )

	ROM_REGION( 0x0300, "color_proms", 0 ) // BPROMs are N82S129AN or equivalent
	ROM_LOAD( "pr-7345.3",      0x0000, 0x0100, CRC(8eee0f72) SHA1(b5694c120f604a5f7cc95618a71ab16a1a6151ed) ) // palette red component
	ROM_LOAD( "pr-7344.2",      0x0100, 0x0100, CRC(3e7babd7) SHA1(d4f8790db4dce75e27156a4c6de2dcef2baf6d76) ) // palette green component
	ROM_LOAD( "pr-7343.1",      0x0200, 0x0100, CRC(371c44a6) SHA1(ac37458d1feb6566b09a795b20c21953d4ab109d) ) // palette blue component

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr5317.4",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( gardiaj ) // Sega game ID# 834-6119-02 GARDIA
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "epr-10250.ic90",   0x00000, 0x8000, CRC(c97943a7) SHA1(eb201987c7a78f7eb6838211c0af3394c0b2d95f) ) // encrypted
	ROM_LOAD( "epr-10251.ic91",   0x10000, 0x8000, CRC(b2ed05dc) SHA1(c520bf7024c85dc759c27eccb0a31998f4d72b5f) )
	ROM_LOAD( "epr-10252.ic92",   0x18000, 0x8000, CRC(0a490588) SHA1(18df754ebdf062096f2d631a722b168901610345) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-10243.ic126", 0x0000, 0x4000, CRC(87220660) SHA1(3f2bfc03e0f1053a4aa0ec5ebb0d573f2e20964c) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "epr-10240.ic4",   0x00000, 0x8000, CRC(998ce090) SHA1(78929f471c5aa8b32d1693e8af2ef3e86efd3d7d) )
	ROM_LOAD( "epr-10241.ic5",   0x08000, 0x8000, CRC(81ab0b07) SHA1(7f776dccd66ad097a1a906823786a52d31a8c4e8) )
	ROM_LOAD( "epr-10242.ic6",   0x10000, 0x8000, CRC(2dc4c4c7) SHA1(0347170b941a5c567eed114833656e8abd16a8ab) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "epr-10234.ic87", 0x00000, 0x8000, CRC(8a6aed33) SHA1(044836885ace8294124b1be9b3a4828f772bb9ee) )
	ROM_LOAD( "epr-10233.ic86", 0x08000, 0x8000, CRC(c52784d3) SHA1(b37d7f261be12616dbe11dfa375eaf6878e4a0f3) )
	ROM_LOAD( "epr-10236.ic89", 0x10000, 0x8000, CRC(b35ab227) SHA1(616f6097afddffa9af89fe84d8b6df59c567c1e6) )
	ROM_LOAD( "epr-10235.ic88", 0x18000, 0x8000, CRC(006a3151) SHA1(a575f9d5c026e6b18e990720ec7520b6b5ae94e3) )

	ROM_REGION( 0x0300, "color_proms", 0 ) // BPROMs are N82S129AN or equivalent
	ROM_LOAD( "pr-7345.ic20",      0x0000, 0x0100, CRC(8eee0f72) SHA1(b5694c120f604a5f7cc95618a71ab16a1a6151ed) ) // palette red component
	ROM_LOAD( "pr-7344.ic14",      0x0100, 0x0100, CRC(3e7babd7) SHA1(d4f8790db4dce75e27156a4c6de2dcef2baf6d76) ) // palette green component
	ROM_LOAD( "pr-7343.ic8",       0x0200, 0x0100, CRC(371c44a6) SHA1(ac37458d1feb6566b09a795b20c21953d4ab109d) ) // palette blue component

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr5317.ic28",     0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) // MB7114E

	ROM_REGION( 0x0400, "plds", 0 )
	ROM_LOAD( "315-5137.bin",   0x00000, 0x0104, CRC(6ffd9e6f) SHA1(a60a3a2ec5bc256b18bfff0fec0172ee2e4fd955) ) // TI PAL16R4A-2CN Located at IC10
	ROM_LOAD( "315-5138.bin",   0x00000, 0x0104, CRC(dd223015) SHA1(8d70f91b118e8653dda1efee3eaea287ae63809f) ) // TI PAL16R4ACN Located at IC11
ROM_END


ROM_START( brain )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "brain.1",      0x00000, 0x8000, CRC(2d2aec31) SHA1(02dfbb0e9ca01b864e3aa594cf38306fe82a4b5d) )
	ROM_LOAD( "brain.2",      0x10000, 0x8000, CRC(810a8ab5) SHA1(87cd39f5b1047f355e1d257c691ef11fc55824ca) )
	ROM_RELOAD(               0x08000, 0x8000 ) // there's code falling through from 7fff *so I have to copy the ROM there
	ROM_LOAD( "brain.3",      0x18000, 0x8000, CRC(9a225634) SHA1(9f137938592dd9c5ab2273864a11a682e0f7f783) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "brain.120",    0x0000, 0x8000, CRC(c7e50278) SHA1(9709a59004c6bc39173d0cb94f3602c358367976) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "brain.62",     0x0000, 0x4000, CRC(7dce2302) SHA1(ebf15da3aea36f6a831a5395b0e5fc253852a3ee) )
	ROM_LOAD( "brain.64",     0x4000, 0x4000, CRC(7ce03fd3) SHA1(11f037c75d606276cbf4ec76a2cfdde94a756493) )
	ROM_LOAD( "brain.66",     0x8000, 0x4000, CRC(ea54323f) SHA1(08a4d2543a75a1fbb6ef2c126e3aeb4945bf458f) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "brain.117",    0x00000, 0x8000, CRC(92ff71a4) SHA1(856646c595e0ef7bbcf18844ee34b04e05893ffa) )
	ROM_LOAD( "brain.110",    0x08000, 0x8000, CRC(a1b847ec) SHA1(d71664822b9b863bd2a37da71b4e0850893b9876) )
	ROM_LOAD( "brain.4",      0x10000, 0x8000, CRC(fd2ea53b) SHA1(c7f2d267f19d2c27a550120e003ebfcb10d8af89) )
	// 18000-1ffff empty

	ROM_REGION( 0x0300, "color_proms", 0 )
	ROM_LOAD( "bprom.3",      0x0000, 0x0100, BAD_DUMP CRC(8eee0f72) SHA1(b5694c120f604a5f7cc95618a71ab16a1a6151ed) ) // palette red component
	ROM_LOAD( "bprom.2",      0x0100, 0x0100, BAD_DUMP CRC(3e7babd7) SHA1(d4f8790db4dce75e27156a4c6de2dcef2baf6d76) ) // palette green component
	ROM_LOAD( "bprom.1",      0x0200, 0x0100, BAD_DUMP CRC(371c44a6) SHA1(ac37458d1feb6566b09a795b20c21953d4ab109d) ) // palette blue component

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr5317.76",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END


ROM_START( tokisens ) // Sega game ID#  834-6409 - Sega MC-8123, 317-0040
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "epr-10961.ic90",  0x00000, 0x8000, CRC(5c71c203) SHA1(65c3730d2255be5e09fda2f8eae1c7f3d245ce9b) )
	ROM_LOAD( "epr-10962.ic91",  0x10000, 0x8000, CRC(db9080e3) SHA1(591b1bd4ab694f45d472bb50483dadb980cd2f86) )
	ROM_LOAD( "epr-10963.ic92",  0x18000, 0x8000, CRC(d17ad93f) SHA1(870dbd3558a4c3a47f36d3d3c0c71c647baacf10) )

	ROM_REGION( 0x2000, "maincpu:key", 0 ) // MC8123 key
	ROM_LOAD( "317-0040.key",  0x0000, 0x2000, CRC(e2b67fd6) SHA1(4fcf457279dac317ccf700591cfaa9a4cff81b4a) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-10967.ic126", 0x0000, 0x8000, CRC(97966bf2) SHA1(b5a3d36afbb3d6e2e2e2c121609a30dc080ccf13) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "epr-10964.ic4",   0x00000, 0x8000, CRC(25af5c93) SHA1(da6e6244b14c9ad51cee012bf46d591928d13050) )
	ROM_LOAD( "epr-10965.ic5",   0x08000, 0x8000, CRC(cc8eb99a) SHA1(b66c2a786a3401021a05740f36103cf8e6129a85) )
	ROM_LOAD( "epr-10966.ic6",   0x10000, 0x8000, CRC(7ecf2459) SHA1(2dc6c4295d0e6f18efd26f8e15e0f31cf0a6820e) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "epr-10958.ic87",  0x00000, 0x8000, CRC(bb62dbc8) SHA1(f48aa1b38077d801521afe6cd43f1463a22b9431) )
	ROM_LOAD( "epr-10957.ic86",  0x08000, 0x8000, CRC(4ec56860) SHA1(9fd6ba8a68b4cb98183e8ac8643656c251f1c72d) )
	ROM_LOAD( "epr-10960.ic89",  0x10000, 0x8000, CRC(880e0d44) SHA1(2b2dc144807d1d048ffe81bfd33a77ccf618dd3e) )
	ROM_LOAD( "epr-10959.ic88",  0x18000, 0x8000, CRC(4deda48f) SHA1(12db2a69286f22cd8243be6faa9a075fafec1dfd) )

	ROM_REGION( 0x0300, "color_proms", 0 )
	ROM_LOAD( "pr10956.ic20", 0x0000, 0x0100, CRC(fd1bba8a) SHA1(4a38239d89f70291df71976b18be49fb24f071ca) ) // MMI 63S141AN - palette red component
	ROM_LOAD( "pr10955.ic14", 0x0100, 0x0100, CRC(72b35df7) SHA1(ef782fb7012c359ed7ca8f4ab42734c4994e473a) ) // MMI 63S141AN - palette green component
	ROM_LOAD( "pr10954.ic8",  0x0200, 0x0100, CRC(b7984867) SHA1(8a03cc98c33e4defe880d10a02a5d0108fa0c9da) ) // MMI 63S141AN - palette blue component

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.ic28",      0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) // BPROM type N82S129AN or compatible
ROM_END

ROM_START( tokisensa )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "ic90",  0x00000, 0x8000, CRC(1466b61d) SHA1(99f93813834d3a7c9f6228076d400f74d9b6dea9) )
	ROM_LOAD( "ic91",  0x10000, 0x8000, CRC(a8479f91) SHA1(0700746fb481fd2bd22ae82c9881aa61222a6379) )
	ROM_LOAD( "ic92",  0x18000, 0x8000, CRC(b7193b39) SHA1(d40fb8591b1ff83f3d56b955ac11a07496a0adbb) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-10967.ic126", 0x0000, 0x8000, CRC(97966bf2) SHA1(b5a3d36afbb3d6e2e2e2c121609a30dc080ccf13) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "ic4",   0x00000, 0x8000, CRC(9013b85c) SHA1(c27322245052ffc9d840fe683ed35965c61bf9e8) )
	ROM_LOAD( "ic5",   0x08000, 0x8000, CRC(e4755cc6) SHA1(33370d556a70e19edce5e0c7fa8b11453ccbe91b) )
	ROM_LOAD( "ic6",   0x10000, 0x8000, CRC(5bbfbdcc) SHA1(e7e679da874a79dfdda0be58d1352c192635296d) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "ic87",            0x00000, 0x8000, CRC(fc2bcbd7) SHA1(6b9007f2057e4c860ecae4ba5db4e02b8aaae8fd) )
	ROM_LOAD( "epr-10957.ic86",  0x08000, 0x8000, CRC(4ec56860) SHA1(9fd6ba8a68b4cb98183e8ac8643656c251f1c72d) )
	ROM_LOAD( "epr-10960.ic89",  0x10000, 0x8000, CRC(880e0d44) SHA1(2b2dc144807d1d048ffe81bfd33a77ccf618dd3e) )
	ROM_LOAD( "epr-10959.ic88",  0x18000, 0x8000, CRC(4deda48f) SHA1(12db2a69286f22cd8243be6faa9a075fafec1dfd) )

	ROM_REGION( 0x0300, "color_proms", 0 )
	ROM_LOAD( "ic20", 0x0000, 0x0100, CRC(8eee0f72) SHA1(b5694c120f604a5f7cc95618a71ab16a1a6151ed) ) // MMI 63S141AN - palette red component
	ROM_LOAD( "ic14", 0x0100, 0x0100, CRC(3e7babd7) SHA1(d4f8790db4dce75e27156a4c6de2dcef2baf6d76) ) // MMI 63S141AN - palette green component
	ROM_LOAD( "ic8",  0x0200, 0x0100, CRC(371c44a6) SHA1(ac37458d1feb6566b09a795b20c21953d4ab109d) ) // MMI 63S141AN - palette blue component

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.ic28",      0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END


ROM_START( wbml ) // Sega game ID# 834-6409 MONSTER LAND
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "epr-11031a.90", 0x00000, 0x8000, CRC(bd3349e5) SHA1(65cc16e5d3b08429388946df254b8122ad1da339) ) // encrypted
	ROM_LOAD( "epr-11032.91",  0x10000, 0x8000, CRC(9d03bdb2) SHA1(7dbab23e7c7972d9b51a0d3d046374720b7d6af5) ) // encrypted
	ROM_LOAD( "epr-11033.92",  0x18000, 0x8000, CRC(7076905c) SHA1(562fbd9bd60851f7e4e60b725193395b4f193479) ) // encrypted

	ROM_REGION( 0x2000, "maincpu:key", 0 ) // MC8123 key
	ROM_LOAD( "317-0043.key",  0x0000, 0x2000, CRC(e354abfc) SHA1(07b0d3c51301ebb25909234b6220a3ed20dbcc7d) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-11037.126", 0x0000, 0x8000, CRC(7a4ee585) SHA1(050436106cced5dcbf40a3d94d48202eedddc3ad) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "epr-11034.4",   0x00000, 0x8000, CRC(37a2077d) SHA1(57f032a98022bf03ff98cb2e563178ba97e4b63c) )
	ROM_LOAD( "epr-11035.5",   0x08000, 0x8000, CRC(cdf2a21b) SHA1(db2553866f21e03bd9d668c179be3352adbaf8a6) )
	ROM_LOAD( "epr-11036.6",   0x10000, 0x8000, CRC(644687fa) SHA1(d6c5bc95da4fc7e81091dcfe6205b6f47d54af76) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "epr-11028.87",  0x00000, 0x8000, CRC(af0b3972) SHA1(413825f66b84c7e45aa1855131482abead8f7f3b) )
	ROM_LOAD( "epr-11027.86",  0x08000, 0x8000, CRC(277d8f1d) SHA1(7854673503ed03d276abe971805a11f8c992f6d6) )
	ROM_LOAD( "epr-11030.89",  0x10000, 0x8000, CRC(f05ffc76) SHA1(f3dbb518240f86430840c3d4cda04bac79c20f69) )
	ROM_LOAD( "epr-11029.88",  0x18000, 0x8000, CRC(cedc9c61) SHA1(dbe5744f9b6f2a406b52b910dd4e133db7bce6b2) )

	ROM_REGION( 0x0300, "color_proms", 0 )
	ROM_LOAD( "pr11026.20",   0x0000, 0x0100, CRC(27057298) SHA1(654be7abb937bb0720263ee6512e31194662effe) )
	ROM_LOAD( "pr11025.14",   0x0100, 0x0100, CRC(41e4d86b) SHA1(a86e8bb0a465d01b04410edfbb82eb96f12b909f) )
	ROM_LOAD( "pr11024.8",    0x0200, 0x0100, CRC(08d71954) SHA1(df045dbfb3d669e4d42fbdba1e7191cd046f7b47) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr5317.37",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( wbmljo ) // Sega game ID# 834-6409 MONSTER LAND
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "epr-11031.90",  0x00000, 0x8000, CRC(497ebfb4) SHA1(d90872c7d5285c85b05879bc67638f640e0339d5) ) // encrypted
	ROM_LOAD( "epr-11032.91",  0x10000, 0x8000, CRC(9d03bdb2) SHA1(7dbab23e7c7972d9b51a0d3d046374720b7d6af5) ) // encrypted
	ROM_LOAD( "epr-11033.92",  0x18000, 0x8000, CRC(7076905c) SHA1(562fbd9bd60851f7e4e60b725193395b4f193479) ) // encrypted

	ROM_REGION( 0x2000, "maincpu:key", 0 ) // MC8123 key
	ROM_LOAD( "317-0043.key",  0x0000, 0x2000, CRC(e354abfc) SHA1(07b0d3c51301ebb25909234b6220a3ed20dbcc7d) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-11037.126", 0x0000, 0x8000, CRC(7a4ee585) SHA1(050436106cced5dcbf40a3d94d48202eedddc3ad) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "epr-11034.4",   0x00000, 0x8000, CRC(37a2077d) SHA1(57f032a98022bf03ff98cb2e563178ba97e4b63c) )
	ROM_LOAD( "epr-11035.5",   0x08000, 0x8000, CRC(cdf2a21b) SHA1(db2553866f21e03bd9d668c179be3352adbaf8a6) )
	ROM_LOAD( "epr-11036.6",   0x10000, 0x8000, CRC(644687fa) SHA1(d6c5bc95da4fc7e81091dcfe6205b6f47d54af76) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "epr-11028.87",  0x00000, 0x8000, CRC(af0b3972) SHA1(413825f66b84c7e45aa1855131482abead8f7f3b) )
	ROM_LOAD( "epr-11027.86",  0x08000, 0x8000, CRC(277d8f1d) SHA1(7854673503ed03d276abe971805a11f8c992f6d6) )
	ROM_LOAD( "epr-11030.89",  0x10000, 0x8000, CRC(f05ffc76) SHA1(f3dbb518240f86430840c3d4cda04bac79c20f69) )
	ROM_LOAD( "epr-11029.88",  0x18000, 0x8000, CRC(cedc9c61) SHA1(dbe5744f9b6f2a406b52b910dd4e133db7bce6b2) )

	ROM_REGION( 0x0300, "color_proms", 0 )
	ROM_LOAD( "pr11026.20",   0x0000, 0x0100, CRC(27057298) SHA1(654be7abb937bb0720263ee6512e31194662effe) )
	ROM_LOAD( "pr11025.14",   0x0100, 0x0100, CRC(41e4d86b) SHA1(a86e8bb0a465d01b04410edfbb82eb96f12b909f) )
	ROM_LOAD( "pr11024.8",    0x0200, 0x0100, CRC(08d71954) SHA1(df045dbfb3d669e4d42fbdba1e7191cd046f7b47) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr5317.37",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END


ROM_START( wbmld )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "decrypted_epr-11031a.90", 0x00000, 0x8000, CRC(aba42eb7) SHA1(d2bb16a52404ba867930140e71f981cfa225dd21) )
	ROM_LOAD( "decrypted_epr-11032.91",  0x10000, 0x8000, CRC(1b158845) SHA1(26360f4bc2884746fbcc5fd6dc21fd848a3d2157) )
	ROM_LOAD( "decrypted_epr-11033.92",  0x18000, 0x8000, CRC(39e07286) SHA1(70192f03e52dd34c9fe5698a5ec1c24d3c58543c) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-11037.126", 0x0000, 0x8000, CRC(7a4ee585) SHA1(050436106cced5dcbf40a3d94d48202eedddc3ad) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "epr-11034.4",   0x00000, 0x8000, CRC(37a2077d) SHA1(57f032a98022bf03ff98cb2e563178ba97e4b63c) )
	ROM_LOAD( "epr-11035.5",   0x08000, 0x8000, CRC(cdf2a21b) SHA1(db2553866f21e03bd9d668c179be3352adbaf8a6) )
	ROM_LOAD( "epr-11036.6",   0x10000, 0x8000, CRC(644687fa) SHA1(d6c5bc95da4fc7e81091dcfe6205b6f47d54af76) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "epr-11028.87",  0x00000, 0x8000, CRC(af0b3972) SHA1(413825f66b84c7e45aa1855131482abead8f7f3b) )
	ROM_LOAD( "epr-11027.86",  0x08000, 0x8000, CRC(277d8f1d) SHA1(7854673503ed03d276abe971805a11f8c992f6d6) )
	ROM_LOAD( "epr-11030.89",  0x10000, 0x8000, CRC(f05ffc76) SHA1(f3dbb518240f86430840c3d4cda04bac79c20f69) )
	ROM_LOAD( "epr-11029.88",  0x18000, 0x8000, CRC(cedc9c61) SHA1(dbe5744f9b6f2a406b52b910dd4e133db7bce6b2) )

	ROM_REGION( 0x0300, "color_proms", 0 )
	ROM_LOAD( "pr11026.20",   0x0000, 0x0100, CRC(27057298) SHA1(654be7abb937bb0720263ee6512e31194662effe) )
	ROM_LOAD( "pr11025.14",   0x0100, 0x0100, CRC(41e4d86b) SHA1(a86e8bb0a465d01b04410edfbb82eb96f12b909f) )
	ROM_LOAD( "pr11024.8",    0x0200, 0x0100, CRC(08d71954) SHA1(df045dbfb3d669e4d42fbdba1e7191cd046f7b47) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr5317.37",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( wbmljod )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "decrypted_epr-11031.90",  0x00000, 0x8000, CRC(940b35bf) SHA1(b0e3b494d17cfad3b4bb1de996931ee813e91f92) )
	ROM_LOAD( "decrypted_epr-11032.91",  0x10000, 0x8000, CRC(1b158845) SHA1(26360f4bc2884746fbcc5fd6dc21fd848a3d2157) )
	ROM_LOAD( "decrypted_epr-11033.92",  0x18000, 0x8000, CRC(39e07286) SHA1(70192f03e52dd34c9fe5698a5ec1c24d3c58543c) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-11037.126", 0x0000, 0x8000, CRC(7a4ee585) SHA1(050436106cced5dcbf40a3d94d48202eedddc3ad) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "epr-11034.4",   0x00000, 0x8000, CRC(37a2077d) SHA1(57f032a98022bf03ff98cb2e563178ba97e4b63c) )
	ROM_LOAD( "epr-11035.5",   0x08000, 0x8000, CRC(cdf2a21b) SHA1(db2553866f21e03bd9d668c179be3352adbaf8a6) )
	ROM_LOAD( "epr-11036.6",   0x10000, 0x8000, CRC(644687fa) SHA1(d6c5bc95da4fc7e81091dcfe6205b6f47d54af76) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "epr-11028.87",  0x00000, 0x8000, CRC(af0b3972) SHA1(413825f66b84c7e45aa1855131482abead8f7f3b) )
	ROM_LOAD( "epr-11027.86",  0x08000, 0x8000, CRC(277d8f1d) SHA1(7854673503ed03d276abe971805a11f8c992f6d6) )
	ROM_LOAD( "epr-11030.89",  0x10000, 0x8000, CRC(f05ffc76) SHA1(f3dbb518240f86430840c3d4cda04bac79c20f69) )
	ROM_LOAD( "epr-11029.88",  0x18000, 0x8000, CRC(cedc9c61) SHA1(dbe5744f9b6f2a406b52b910dd4e133db7bce6b2) )

	ROM_REGION( 0x0300, "color_proms", 0 )
	ROM_LOAD( "pr11026.20",   0x0000, 0x0100, CRC(27057298) SHA1(654be7abb937bb0720263ee6512e31194662effe) )
	ROM_LOAD( "pr11025.14",   0x0100, 0x0100, CRC(41e4d86b) SHA1(a86e8bb0a465d01b04410edfbb82eb96f12b909f) )
	ROM_LOAD( "pr11024.8",    0x0200, 0x0100, CRC(08d71954) SHA1(df045dbfb3d669e4d42fbdba1e7191cd046f7b47) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr5317.37",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END


ROM_START( wbmljb )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "wbml.01",      0x20000, 0x8000, CRC(66482638) SHA1(887f93015f0effa2d0fa1f1f59082f75ac072221) ) // Unencrypted opcodes
	ROM_CONTINUE(             0x00000, 0x8000 ) // Now load the operands in RAM
	ROM_LOAD( "m-6.bin",      0x30000, 0x8000, CRC(8c08cd11) SHA1(5103f3c887c213b09aee858c4a883f2869b9ffb5) ) // Unencrypted opcodes
	ROM_CONTINUE(             0x10000, 0x8000 )
	ROM_LOAD( "m-7.bin",      0x38000, 0x8000, CRC(11881703) SHA1(b5e4d477158e7653b0fef5a4806be7b4871e917d) ) // Unencrypted opcodes
	ROM_CONTINUE(             0x18000, 0x8000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-11037.126", 0x0000, 0x8000, CRC(7a4ee585) SHA1(050436106cced5dcbf40a3d94d48202eedddc3ad) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "epr-11034.4",   0x00000, 0x8000, CRC(37a2077d) SHA1(57f032a98022bf03ff98cb2e563178ba97e4b63c) )
	ROM_LOAD( "epr-11035.5",   0x08000, 0x8000, CRC(cdf2a21b) SHA1(db2553866f21e03bd9d668c179be3352adbaf8a6) )
	ROM_LOAD( "epr-11036.6",   0x10000, 0x8000, CRC(644687fa) SHA1(d6c5bc95da4fc7e81091dcfe6205b6f47d54af76) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "epr-11028.87",  0x00000, 0x8000, CRC(af0b3972) SHA1(413825f66b84c7e45aa1855131482abead8f7f3b) )
	ROM_LOAD( "epr-11027.86",  0x08000, 0x8000, CRC(277d8f1d) SHA1(7854673503ed03d276abe971805a11f8c992f6d6) )
	ROM_LOAD( "epr-11030.89",  0x10000, 0x8000, CRC(f05ffc76) SHA1(f3dbb518240f86430840c3d4cda04bac79c20f69) )
	ROM_LOAD( "epr-11029.88",  0x18000, 0x8000, CRC(cedc9c61) SHA1(dbe5744f9b6f2a406b52b910dd4e133db7bce6b2) )

	ROM_REGION( 0x0300, "color_proms", 0 )
	ROM_LOAD( "pr11026.20",   0x0000, 0x0100, CRC(27057298) SHA1(654be7abb937bb0720263ee6512e31194662effe) )
	ROM_LOAD( "pr11025.14",   0x0100, 0x0100, CRC(41e4d86b) SHA1(a86e8bb0a465d01b04410edfbb82eb96f12b909f) )
	ROM_LOAD( "pr11024.8",    0x0200, 0x0100, CRC(08d71954) SHA1(df045dbfb3d669e4d42fbdba1e7191cd046f7b47) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr5317.37",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( wbmlb )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "wbml.01",      0x20000, 0x8000, CRC(66482638) SHA1(887f93015f0effa2d0fa1f1f59082f75ac072221) ) // Unencrypted opcodes
	ROM_CONTINUE(             0x00000, 0x8000 ) // Now load the operands in RAM
	ROM_LOAD( "wbml.02",      0x30000, 0x8000, CRC(48746bb6) SHA1(a0049cba53e7548afa8d7b16a7e9494e628d2a0f) ) // Unencrypted opcodes
	ROM_CONTINUE(             0x10000, 0x8000 )
	ROM_LOAD( "wbml.03",      0x38000, 0x8000, CRC(d57ba8aa) SHA1(16f095cb78e31af5ce76d36c20fe4c3e0d027aea) ) // Unencrypted opcodes
	ROM_CONTINUE(             0x18000, 0x8000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-11037.126", 0x0000, 0x8000, CRC(7a4ee585) SHA1(050436106cced5dcbf40a3d94d48202eedddc3ad) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "wbml.08",      0x00000, 0x8000, CRC(bbea6afe) SHA1(ba56c6789a35eb57cd226296ebf57e9aa19ba625) )
	ROM_LOAD( "wbml.09",      0x08000, 0x8000, CRC(77567d41) SHA1(2ac501661522615859f8a1718dbb8451272d6931) )
	ROM_LOAD( "wbml.10",      0x10000, 0x8000, CRC(a52ffbdd) SHA1(609375112268b770a798186697ecab5853f29f89) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "epr-11028.87",  0x00000, 0x8000, CRC(af0b3972) SHA1(413825f66b84c7e45aa1855131482abead8f7f3b) )
	ROM_LOAD( "epr-11027.86",  0x08000, 0x8000, CRC(277d8f1d) SHA1(7854673503ed03d276abe971805a11f8c992f6d6) )
	ROM_LOAD( "epr-11030.89",  0x10000, 0x8000, CRC(f05ffc76) SHA1(f3dbb518240f86430840c3d4cda04bac79c20f69) )
	ROM_LOAD( "epr-11029.88",  0x18000, 0x8000, CRC(cedc9c61) SHA1(dbe5744f9b6f2a406b52b910dd4e133db7bce6b2) )

	ROM_REGION( 0x0300, "color_proms", 0 )
	ROM_LOAD( "pr11026.20",   0x0000, 0x0100, CRC(27057298) SHA1(654be7abb937bb0720263ee6512e31194662effe) )
	ROM_LOAD( "pr11025.14",   0x0100, 0x0100, CRC(41e4d86b) SHA1(a86e8bb0a465d01b04410edfbb82eb96f12b909f) )
	ROM_LOAD( "pr11024.8",    0x0200, 0x0100, CRC(08d71954) SHA1(df045dbfb3d669e4d42fbdba1e7191cd046f7b47) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr5317.37",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( wbmlb2 )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "m5.e8", 0x20000, 0x8000, CRC(66482638) SHA1(887f93015f0effa2d0fa1f1f59082f75ac072221) ) // Unencrypted opcodes
	ROM_CONTINUE(      0x00000, 0x8000 ) // Now load the operands in RAM
	ROM_LOAD( "m6.f8", 0x30000, 0x8000, CRC(c05e365a) SHA1(f8339af02538258a88ed348524c23a337fe152b6) ) // Unencrypted opcodes
	ROM_CONTINUE(      0x10000, 0x8000 )
	ROM_LOAD( "m7.g8", 0x38000, 0x8000, CRC(cb3ea856) SHA1(4d9eedecccccf2bf40cb65473a155f519803f6f9) ) // Unencrypted opcodes
	ROM_CONTINUE(      0x18000, 0x8000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "m8", 0x0000, 0x8000, BAD_DUMP CRC(7a4ee585) SHA1(050436106cced5dcbf40a3d94d48202eedddc3ad) ) // was removed on the PCB, using wbmlb's one for now

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "m11.g19", 0x00000, 0x8000, CRC(bbea6afe) SHA1(ba56c6789a35eb57cd226296ebf57e9aa19ba625) )
	ROM_LOAD( "m10.f19", 0x08000, 0x8000, BAD_DUMP CRC(77567d41) SHA1(2ac501661522615859f8a1718dbb8451272d6931) ) // rotten, but the other 2 ROMs match wbmlb so use that for now
	ROM_LOAD( "m9.e19",  0x10000, 0x8000, CRC(a52ffbdd) SHA1(609375112268b770a798186697ecab5853f29f89) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "m2.b8", 0x00000, 0x8000, CRC(af0b3972) SHA1(413825f66b84c7e45aa1855131482abead8f7f3b) )
	ROM_LOAD( "m1.a8", 0x08000, 0x8000, CRC(0cf576d1) SHA1(b9fe4a8405fc95ec08ab5f1f1029fa9c6ddd81be) )
	ROM_LOAD( "m4.d8", 0x10000, 0x8000, CRC(f05ffc76) SHA1(f3dbb518240f86430840c3d4cda04bac79c20f69) )
	ROM_LOAD( "m3.c8", 0x18000, 0x8000, CRC(cedc9c61) SHA1(dbe5744f9b6f2a406b52b910dd4e133db7bce6b2) )

	ROM_REGION( 0x0300, "color_proms", 0 )
	ROM_LOAD( "pr11026.20", 0x0000, 0x0100, CRC(27057298) SHA1(654be7abb937bb0720263ee6512e31194662effe) )
	ROM_LOAD( "pr11025.14", 0x0100, 0x0100, CRC(41e4d86b) SHA1(a86e8bb0a465d01b04410edfbb82eb96f12b909f) )
	ROM_LOAD( "pr11024.8",  0x0200, 0x0100, CRC(08d71954) SHA1(df045dbfb3d669e4d42fbdba1e7191cd046f7b47) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr5317.37", 0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( wbmlbg )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "galaxy.ic90",  0x20000, 0x8000, CRC(66482638) SHA1(887f93015f0effa2d0fa1f1f59082f75ac072221) ) // Unencrypted opcodes
	ROM_CONTINUE(             0x00000, 0x8000 ) // Now load the operands in RAM
	ROM_LOAD( "galaxy.ic91",  0x30000, 0x8000, CRC(89a8ab93) SHA1(11389604017e15aed9a8fcef60e42740acd79917) ) // Unencrypted opcodes
	ROM_CONTINUE(             0x10000, 0x8000 )
	ROM_LOAD( "galaxy.ic92",  0x38000, 0x8000, CRC(39e07286) SHA1(70192f03e52dd34c9fe5698a5ec1c24d3c58543c) ) // Unencrypted opcodes
	ROM_RELOAD(               0x18000, 0x8000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-11037.126", 0x0000, 0x8000, CRC(7a4ee585) SHA1(050436106cced5dcbf40a3d94d48202eedddc3ad) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "galaxy.ic4", 0x00000, 0x8000, CRC(ab75d056) SHA1(d90d9c723536d0ec21900dc70b51715300b01fe7) )
	ROM_LOAD( "galaxy.ic6", 0x08000, 0x8000, CRC(6bb5e601) SHA1(465d67dcde4e775d1b93640ef1a300e958cbe707) )
	ROM_LOAD( "galaxy.ic5", 0x10000, 0x8000, CRC(3c11d151) SHA1(7b0c6792ae919ac309a709ca0c89006487e1d6e9) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "epr-11028.87",  0x00000, 0x8000, CRC(af0b3972) SHA1(413825f66b84c7e45aa1855131482abead8f7f3b) )
	ROM_LOAD( "epr-11027.86",  0x08000, 0x8000, CRC(277d8f1d) SHA1(7854673503ed03d276abe971805a11f8c992f6d6) )
	ROM_LOAD( "epr-11030.89",  0x10000, 0x8000, CRC(f05ffc76) SHA1(f3dbb518240f86430840c3d4cda04bac79c20f69) )
	ROM_LOAD( "epr-11029.88",  0x18000, 0x8000, CRC(cedc9c61) SHA1(dbe5744f9b6f2a406b52b910dd4e133db7bce6b2) )

	ROM_REGION( 0x0300, "color_proms", 0 )
	ROM_LOAD( "pr11026.20",   0x0000, 0x0100, CRC(27057298) SHA1(654be7abb937bb0720263ee6512e31194662effe) )
	ROM_LOAD( "pr11025.14",   0x0100, 0x0100, CRC(41e4d86b) SHA1(a86e8bb0a465d01b04410edfbb82eb96f12b909f) )
	ROM_LOAD( "pr11024.8",    0x0200, 0x0100, CRC(08d71954) SHA1(df045dbfb3d669e4d42fbdba1e7191cd046f7b47) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr5317.37",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

// similar to above, apparently Gecas license, but clearly a bootleg
ROM_START( wbmlbge )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "3.k3",  0x20000, 0x8000, CRC(b4f90adc) SHA1(23b536acc70bbf8673be193a67d0423a87e2ff4d) ) // Unencrypted opcodes
	ROM_CONTINUE(      0x00000, 0x8000 ) // Now load the operands in RAM
	ROM_LOAD( "2.k4",  0x30000, 0x8000, CRC(1896c19b) SHA1(41d0429c65b172a5f6d0af92a5a2ae9178ceb550) ) // Unencrypted opcodes
	ROM_CONTINUE(      0x10000, 0x8000 )
	ROM_LOAD( "1.k4",  0x38000, 0x8000, CRC(0e827f13) SHA1(8d7fb996630beec48b5471a46be30b65e3ba6d8c) ) // Unencrypted opcodes
	ROM_CONTINUE(      0x18000, 0x8000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "11.d9", 0x00000, 0x8000, CRC(7a4ee585) SHA1(050436106cced5dcbf40a3d94d48202eedddc3ad) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "8.y6",  0x00000, 0x8000, CRC(ab75d056) SHA1(d90d9c723536d0ec21900dc70b51715300b01fe7) )
	ROM_LOAD( "9.y5",  0x08000, 0x8000, CRC(6bb5e601) SHA1(465d67dcde4e775d1b93640ef1a300e958cbe707) )
	ROM_LOAD( "10.y5", 0x10000, 0x8000, CRC(3c11d151) SHA1(7b0c6792ae919ac309a709ca0c89006487e1d6e9) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "5.k2",  0x00000, 0x8000, CRC(af0b3972) SHA1(413825f66b84c7e45aa1855131482abead8f7f3b) )
	ROM_LOAD( "4.k2",  0x08000, 0x8000, CRC(277d8f1d) SHA1(7854673503ed03d276abe971805a11f8c992f6d6) )
	ROM_LOAD( "7.k1",  0x10000, 0x8000, CRC(f05ffc76) SHA1(f3dbb518240f86430840c3d4cda04bac79c20f69) )
	ROM_LOAD( "6.k1",  0x18000, 0x8000, CRC(cedc9c61) SHA1(dbe5744f9b6f2a406b52b910dd4e133db7bce6b2) )

	ROM_REGION( 0x0300, "color_proms", 0 )
	ROM_LOAD( "3.z8",  0x0000, 0x0100, CRC(27057298) SHA1(654be7abb937bb0720263ee6512e31194662effe) )
	ROM_LOAD( "2.y8",  0x0100, 0x0100, CRC(41e4d86b) SHA1(a86e8bb0a465d01b04410edfbb82eb96f12b909f) )
	ROM_LOAD( "1.x8",  0x0200, 0x0100, CRC(08d71954) SHA1(df045dbfb3d669e4d42fbdba1e7191cd046f7b47) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr5317.37",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( wbmlvc )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "vc.ic90",  0x20000, 0x8000, CRC(093c4852) SHA1(8dfbfe89c5b27b381fc54610e1e262a0e1f1ec59) ) // Unencrypted opcodes
	ROM_CONTINUE(         0x00000, 0x8000 ) // Now load the operands in RAM
	ROM_LOAD( "vc.ic91",  0x30000, 0x8000, CRC(7e973ece) SHA1(bd98287d376c4333313432f4ddab45dae9fdcd93) ) // Unencrypted opcodes
	ROM_CONTINUE(         0x10000, 0x8000 )
	ROM_LOAD( "vc.ic92",  0x38000, 0x8000, CRC(32661e7e) SHA1(5e06735b7dcc529b142bf6aa311d0e9f389daedd) ) // Unencrypted opcodes
	ROM_RELOAD(           0x18000, 0x8000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-11037.126", 0x0000, 0x8000, CRC(7a4ee585) SHA1(050436106cced5dcbf40a3d94d48202eedddc3ad) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "vc.ic4",   0x00000, 0x8000, CRC(820bee59) SHA1(47afff58387eb67a8b0849d74023bd2c176a45e9) )
	ROM_LOAD( "vc.ic5",   0x08000, 0x8000, CRC(a9a1447e) SHA1(f7e55080c4fd6e1ff9e21a19b2f71dfd512d62c3) )
	ROM_LOAD( "vc.ic6",   0x10000, 0x8000, CRC(359026a0) SHA1(a20c801dbc758f172fcfc505a5083ddb76604243) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "epr-11028.87",  0x00000, 0x8000, CRC(af0b3972) SHA1(413825f66b84c7e45aa1855131482abead8f7f3b) )
	ROM_LOAD( "epr-11027.86",  0x08000, 0x8000, CRC(277d8f1d) SHA1(7854673503ed03d276abe971805a11f8c992f6d6) )
	ROM_LOAD( "epr-11030.89",  0x10000, 0x8000, CRC(f05ffc76) SHA1(f3dbb518240f86430840c3d4cda04bac79c20f69) )
	ROM_LOAD( "epr-11029.88",  0x18000, 0x8000, CRC(cedc9c61) SHA1(dbe5744f9b6f2a406b52b910dd4e133db7bce6b2) )

	ROM_REGION( 0x0300, "color_proms", 0 )
	ROM_LOAD( "pr11026.20",   0x0000, 0x0100, CRC(27057298) SHA1(654be7abb937bb0720263ee6512e31194662effe) )
	ROM_LOAD( "pr11025.14",   0x0100, 0x0100, CRC(41e4d86b) SHA1(a86e8bb0a465d01b04410edfbb82eb96f12b909f) )
	ROM_LOAD( "pr11024.8",    0x0200, 0x0100, CRC(08d71954) SHA1(df045dbfb3d669e4d42fbdba1e7191cd046f7b47) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr5317.37",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( wbmlvcd )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "wbmlvcd.ic90",  0x00000, 0x8000, CRC(f9c04c07) SHA1(736013b01451c38f7ede207f3154c7387e60ac29) )
	ROM_LOAD( "wbmlvcd.ic91",  0x10000, 0x8000, CRC(87167a57) SHA1(853e029e2875a2250471f3f405c906c9b5a4829d) )
	ROM_LOAD( "wbmlvcd.ic92",  0x18000, 0x8000, CRC(ffb69e82) SHA1(0d48ce1e3cc02a992c495a92fec4e2d03fc27193) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-11037.126", 0x0000, 0x8000, CRC(7a4ee585) SHA1(050436106cced5dcbf40a3d94d48202eedddc3ad) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "vc.ic4",   0x00000, 0x8000, CRC(820bee59) SHA1(47afff58387eb67a8b0849d74023bd2c176a45e9) )
	ROM_LOAD( "vc.ic5",   0x08000, 0x8000, CRC(a9a1447e) SHA1(f7e55080c4fd6e1ff9e21a19b2f71dfd512d62c3) )
	ROM_LOAD( "vc.ic6",   0x10000, 0x8000, CRC(359026a0) SHA1(a20c801dbc758f172fcfc505a5083ddb76604243) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "epr-11028.87",  0x00000, 0x8000, CRC(af0b3972) SHA1(413825f66b84c7e45aa1855131482abead8f7f3b) )
	ROM_LOAD( "epr-11027.86",  0x08000, 0x8000, CRC(277d8f1d) SHA1(7854673503ed03d276abe971805a11f8c992f6d6) )
	ROM_LOAD( "epr-11030.89",  0x10000, 0x8000, CRC(f05ffc76) SHA1(f3dbb518240f86430840c3d4cda04bac79c20f69) )
	ROM_LOAD( "epr-11029.88",  0x18000, 0x8000, CRC(cedc9c61) SHA1(dbe5744f9b6f2a406b52b910dd4e133db7bce6b2) )

	ROM_REGION( 0x0300, "color_proms", 0 )
	ROM_LOAD( "pr11026.20",   0x0000, 0x0100, CRC(27057298) SHA1(654be7abb937bb0720263ee6512e31194662effe) )
	ROM_LOAD( "pr11025.14",   0x0100, 0x0100, CRC(41e4d86b) SHA1(a86e8bb0a465d01b04410edfbb82eb96f12b909f) )
	ROM_LOAD( "pr11024.8",    0x0200, 0x0100, CRC(08d71954) SHA1(df045dbfb3d669e4d42fbdba1e7191cd046f7b47) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr5317.37",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

// found on 2 PCBs in different parts of Italy
// spotted differences:
// very low starting energy
// the time runs out super-fast
// speed and jump skills are dramatically increased since the start
ROM_START( wbmlh )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "6",  0x20000, 0x8000, CRC(1ace78a0) SHA1(0d373155829bc9789049ee38c3f5d6659d866243) ) // Unencrypted opcodes
	ROM_CONTINUE(   0x00000, 0x8000 ) // Now load the operands in RAM
	ROM_LOAD( "5",  0x30000, 0x8000, CRC(5aa6a908) SHA1(ef5a589a0e728f4a282d9db4254990532ad2c2bc) ) // Unencrypted opcodes
	ROM_CONTINUE(   0x10000, 0x8000 )
	ROM_LOAD( "4",  0x38000, 0x8000, CRC(cb3ea856) SHA1(4d9eedecccccf2bf40cb65473a155f519803f6f9) ) // Unencrypted opcodes
	ROM_CONTINUE(   0x18000, 0x8000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "11", 0x0000, 0x8000, CRC(7a4ee585) SHA1(050436106cced5dcbf40a3d94d48202eedddc3ad) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "3", 0x00000, 0x8000, CRC(ab75d056) SHA1(d90d9c723536d0ec21900dc70b51715300b01fe7) )
	ROM_LOAD( "2", 0x08000, 0x8000, CRC(6bb5e601) SHA1(465d67dcde4e775d1b93640ef1a300e958cbe707) )
	ROM_LOAD( "1", 0x10000, 0x8000, CRC(3c11d151) SHA1(7b0c6792ae919ac309a709ca0c89006487e1d6e9) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "9",  0x00000, 0x8000, CRC(af0b3972) SHA1(413825f66b84c7e45aa1855131482abead8f7f3b) )
	ROM_LOAD( "10", 0x08000, 0x8000, CRC(277d8f1d) SHA1(7854673503ed03d276abe971805a11f8c992f6d6) )
	ROM_LOAD( "7",  0x10000, 0x8000, CRC(f05ffc76) SHA1(f3dbb518240f86430840c3d4cda04bac79c20f69) )
	ROM_LOAD( "8",  0x18000, 0x8000, CRC(cedc9c61) SHA1(dbe5744f9b6f2a406b52b910dd4e133db7bce6b2) )

	ROM_REGION( 0x0300, "color_proms", 0 )
	ROM_LOAD( "pr11026.20",   0x0000, 0x0100, CRC(27057298) SHA1(654be7abb937bb0720263ee6512e31194662effe) )
	ROM_LOAD( "pr11025.14",   0x0100, 0x0100, CRC(41e4d86b) SHA1(a86e8bb0a465d01b04410edfbb82eb96f12b909f) )
	ROM_LOAD( "pr11024.8",    0x0200, 0x0100, CRC(08d71954) SHA1(df045dbfb3d669e4d42fbdba1e7191cd046f7b47) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr5317.37",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( dakkochn )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "epr-11224.ic90",  0x00000, 0x8000, CRC(9fb1972b) SHA1(1bb61c6ec2b5b8eb39f74f20d5bcd0f14501bd21) ) // encrypted
	ROM_LOAD( "epr-11225.ic91",  0x10000, 0x8000, CRC(c540f9e2) SHA1(dbda9355e8b796bcfaee2789714d248c4d7ad58c) ) // encrypted
	// 18000-1ffff empty

	ROM_REGION( 0x2000, "maincpu:key", 0 ) // MC8123B key
	ROM_LOAD( "317-5014.key",    0x0000, 0x2000, CRC(bb9df5ad) SHA1(7e7b7255149ae01d19883ecf4a88989f8a9bf4c6) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-11229.ic126", 0x0000, 0x8000, CRC(c11648d0) SHA1(c2df3d767d497c3365ae70748c4790f4ee394958) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "epr-11226.ic4",   0x00000, 0x8000, CRC(3dbc2f78) SHA1(f3f7ee2c0bedcc21c1c1f5394838af6d0a8833d8) )
	ROM_LOAD( "epr-11227.ic5",   0x08000, 0x8000, CRC(34156e8d) SHA1(e23d8604a3d5db413cf150f9891fca2b1e0163fa) )
	ROM_LOAD( "epr-11228.ic6",   0x10000, 0x8000, CRC(fdd5323f) SHA1(c47099c78207bb2258d34b98b48e3c04beb6407e) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "epr-11221.ic87",  0x00000, 0x8000, CRC(f9a44916) SHA1(9d9ba96146cff4c1ed18b7134ab19919e144d326) )
	ROM_LOAD( "epr-11220.ic86",  0x08000, 0x8000, CRC(84c8f6b2) SHA1(82b4c64b5b79dab9e3894e3e0bd12a05909af989) )
	ROM_LOAD( "epr-11223.ic89",  0x10000, 0x8000, CRC(538adc55) SHA1(542af53a56f580e5ab455aa6bed955ee5fd4a252) )
	ROM_LOAD( "epr-11222.ic88",  0x18000, 0x8000, CRC(33fab0b2) SHA1(eb3c08009315e46590c2c0df17fc3fa391034c66) )

	ROM_REGION( 0x0300, "color_proms", 0 )
	ROM_LOAD( "pr-11219.ic20",   0x0000, 0x0100, CRC(45e252d9) SHA1(92d8f1d0f1a9e65234521ce02d512f08b5e06d78) ) // palette red component
	ROM_LOAD( "pr-11218.ic14",   0x0100, 0x0100, CRC(3eda3a1b) SHA1(cc98c792521845259088eb163a150cd5bb603d5d) ) // palette green component
	ROM_LOAD( "pr-11217.ic8",    0x0200, 0x0100, CRC(49dbde88) SHA1(7057da5617de7e4775adf092cce1709135066129) ) // palette blue component

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr-5317.ic37",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )

	ROM_REGION( 0x00e7, "plds", 0 )
	ROM_LOAD( "315-5139.ic50",   0x0000, 0x00e7, CRC(943d91b0) SHA1(37c98085d580808aaeb01726a9f59705590378c4) )
ROM_END


ROM_START( ufosensi )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "epr-11661.90",  0x00000, 0x8000, CRC(f3e394e2) SHA1(a295a2aa80a164a548995822c46f32fd9fad7a0b) ) // encrypted
	ROM_LOAD( "epr-11662.91",  0x10000, 0x8000, CRC(0c2e4120) SHA1(d81fbefa95868e3efd29ef3bacf108329781ca17) ) // encrypted
	ROM_LOAD( "epr-11663.92",  0x18000, 0x8000, CRC(4515ebae) SHA1(9b823f10999746292762c2f0a1ca9039efa22506) ) // encrypted

	ROM_REGION( 0x2000, "maincpu:key", 0 ) // MC8123 key
	ROM_LOAD( "317-0064.key",  0x0000, 0x2000, CRC(da326f36) SHA1(0871b351379a094ac578e0eca5cb17797f9085aa) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-11667.126", 0x0000, 0x8000, CRC(110baba9) SHA1(e14cf5af11ac9691eca897bbae7c238665cd2a4d) )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "epr-11664.4",   0x00000, 0x8000, CRC(1b1bc3d5) SHA1(2a09e0dbe2d467c151dce705f249367df849eaeb) )
	ROM_LOAD( "epr-11665.5",   0x08000, 0x8000, CRC(3659174a) SHA1(176d2436abb45827a8d387241082854f55dc0314) )
	ROM_LOAD( "epr-11666.6",   0x10000, 0x8000, CRC(99dcc793) SHA1(ad1d0acb60e7c1a7016955e142ebca1cf07b4908) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "epr-11658.87",  0x00000, 0x8000, CRC(3b5a20f7) SHA1(03e0934b0913c3a2cadf1d28b8a700d70b80fbac) )
	ROM_LOAD( "epr-11657.86",  0x08000, 0x8000, CRC(010f81a9) SHA1(1b7ee05c80edfa403e32c216fa69387ca556895e) )
	ROM_LOAD( "epr-11660.89",  0x10000, 0x8000, CRC(e1e2e7c5) SHA1(434039a70049a6e74e2a2f48b60345f720e6b1af) )
	ROM_LOAD( "epr-11659.88",  0x18000, 0x8000, CRC(286c7286) SHA1(449a19ea9a9f9df47005e8dac1b8eacaebc515e7) )

	ROM_REGION( 0x0300, "color_proms", 0 )
	ROM_LOAD( "pr11656.20",   0x0000, 0x0100, CRC(640740eb) SHA1(9a601a3665f612d00c70019d33c7abd3cca9434b) ) // palette red component
	ROM_LOAD( "pr11655.14",   0x0100, 0x0100, CRC(a0c3fa77) SHA1(cdffa1de06d30ec421323145dfc3271803fc25d4) ) // palette green component
	ROM_LOAD( "pr11654.8",    0x0200, 0x0100, CRC(ba624305) SHA1(eb1d0dde60f81ff510ac8c1212e0ed5703febaf3) ) // palette blue component

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr5317.28",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( ufosensib )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "k1-08.ic18.3-4s", 0x20000, 0x8000, CRC(6b1d0955) SHA1(dbda145d40eaecd30c1d55a9675c58a2967c20c4) )
	ROM_CONTINUE(                0x00000, 0x8000 ) // Now load the operands in RAM
	ROM_LOAD( "k1-09.ic19.4s",   0x30000, 0x8000, CRC(fc543b26) SHA1(b9e1d2ca6f9811bf341edf104fe209dbf56e4b2d) )
	ROM_CONTINUE(                0x10000, 0x8000 )
	ROM_LOAD( "k1-10.ic20.4-5s", 0x38000, 0x8000, CRC(6ba2dc77) SHA1(09a65f55988ae28e285d402af9a2a1f1dc05a82c) )
	ROM_CONTINUE(                0x18000, 0x8000 )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "epr-11667.126", 0x0000, 0x8000, CRC(110baba9) SHA1(e14cf5af11ac9691eca897bbae7c238665cd2a4d) ) // label on chip is "k1-11.ic168.10v"

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "epr-11664.4",   0x00000, 0x8000, CRC(1b1bc3d5) SHA1(2a09e0dbe2d467c151dce705f249367df849eaeb) ) // label on chip is "k1-01.ic72.6e"
	ROM_LOAD( "epr-11665.5",   0x08000, 0x8000, CRC(3659174a) SHA1(176d2436abb45827a8d387241082854f55dc0314) ) // label on chip is "k1-02.ic73.6-7e"
	ROM_LOAD( "epr-11666.6",   0x10000, 0x8000, CRC(99dcc793) SHA1(ad1d0acb60e7c1a7016955e142ebca1cf07b4908) ) // label on chip is "k1-03.ic74.7e"

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "epr-11658.87",  0x00000, 0x8000, CRC(3b5a20f7) SHA1(03e0934b0913c3a2cadf1d28b8a700d70b80fbac) ) // label on chip is "k1-05.ic15.1-2s"
	ROM_LOAD( "epr-11657.86",  0x08000, 0x8000, CRC(010f81a9) SHA1(1b7ee05c80edfa403e32c216fa69387ca556895e) ) // label on chip is "k1-04.ic14.1s"
	ROM_LOAD( "epr-11660.89",  0x10000, 0x8000, CRC(e1e2e7c5) SHA1(434039a70049a6e74e2a2f48b60345f720e6b1af) ) // label on chip is "k1-07.ic17.2-3s"
	ROM_LOAD( "epr-11659.88",  0x18000, 0x8000, CRC(286c7286) SHA1(449a19ea9a9f9df47005e8dac1b8eacaebc515e7) ) // label on chip is "k1-06.ic16.2s"

	ROM_REGION( 0x0300, "color_proms", 0 )
	ROM_LOAD( "pr11656.20",   0x0000, 0x0100, CRC(640740eb) SHA1(9a601a3665f612d00c70019d33c7abd3cca9434b) ) // palette red component - label on chip is "74s287.ic134.9h"
	ROM_LOAD( "pr11655.14",   0x0100, 0x0100, CRC(a0c3fa77) SHA1(cdffa1de06d30ec421323145dfc3271803fc25d4) ) // palette green component - label on chip is "74s287.ic133.9f"
	ROM_LOAD( "pr11654.8",    0x0200, 0x0100, CRC(ba624305) SHA1(eb1d0dde60f81ff510ac8c1212e0ed5703febaf3) ) // palette blue component - label on chip is "74s287.ic132.9d"

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr5317.28",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) ) // label on chip is "74s287.ic116.8k"

	ROM_REGION( 0x2000, "plds", 0 )
	ROM_LOAD( "pal6l8.ic3.1c",   0x0000, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal6l8.ic32.2c",  0x0200, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal6l8.ic33.2d",  0x0400, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal6l8.ic4.1d",   0x0600, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal6r4.1",        0x0800, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal6r4.2",        0x0A00, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal6r4.ic34.2f",  0x0C00, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal6r4.ic5.1f",   0x0E00, 0x0104, NO_DUMP ) // PAL is read protected
	ROM_LOAD( "pal20r4.ic69.4c", 0x1000, 0x0144, NO_DUMP ) // PAL is read protected
ROM_END


ROM_START( blockgal )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "bg.116",       0x0000, 0x4000, CRC(a99b231a) SHA1(42ba45a4fd315255e9500bc3a0e8fe653c4c5a9c) ) // encrypted
	ROM_LOAD( "bg.109",       0x4000, 0x4000, CRC(a6b573d5) SHA1(33547a3895bbe65d5a6c40453eeb93e1fedad6de) ) // encrypted
	// 0x8000-0xbfff empty (was same as My Hero)

	ROM_REGION( 0x2000, "maincpu:key", 0 ) // MC8123 key
	ROM_LOAD( "317-0029.key",  0x0000, 0x2000, CRC(350d7f93) SHA1(7ef12d63b2c7150f8e74f65ec8340471d72b1c03) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "bg.120",       0x0000, 0x2000, CRC(d848faff) SHA1(5974cc0c3090800ca79f580a620f5b6615f5d039) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "bg.62",        0x0000, 0x2000, CRC(7e3ea4eb) SHA1(8bf020b083e2da12fe95ddae9ac7a385490525bc) )
	ROM_LOAD( "bg.61",        0x2000, 0x2000, CRC(4dd3d39d) SHA1(759fca021f8d59e861dc19543d5a184428a5e472) )
	ROM_LOAD( "bg.64",        0x4000, 0x2000, CRC(17368663) SHA1(e8f2ac6de0fddf08aefae07e693cac100cfb0db4) )
	ROM_LOAD( "bg.63",        0x6000, 0x2000, CRC(0c8bc404) SHA1(fc96fb682da3af6b7fc852cea6d8a957c4ce57e3) )
	ROM_LOAD( "bg.66",        0x8000, 0x2000, CRC(2b7dc4fa) SHA1(79d3677b24682cee0c08088433646800703be531) )
	ROM_LOAD( "bg.65",        0xa000, 0x2000, CRC(ed121306) SHA1(89f812b3954922e22fcf8d9cc4ee5ba295279cb6) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "bg.117",       0x0000, 0x4000, CRC(e99cc920) SHA1(b2b9199a9296e0c34fcf4dd20ffd3e8de08f42da) )
	ROM_LOAD( "bg.04",        0x4000, 0x4000, CRC(213057f8) SHA1(a872631aaa2b73e9198f2ad6cede2a889279e610) )
	ROM_LOAD( "bg.110",       0x8000, 0x4000, CRC(064c812c) SHA1(673790dc5131fd280333386a0e9915fb94e9f3e1) )
	ROM_LOAD( "bg.05",        0xc000, 0x4000, CRC(02e0b040) SHA1(fb626fc31dfe25bf9fac0c8d76d5041609b06e82) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr5317.76",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

ROM_START( blockgalb )
	ROM_REGION( 0x18000, "maincpu", 0 )
	ROM_LOAD( "ic62",         0x10000, 0x8000, CRC(65c47676) SHA1(bc283761e6f9ebf65fb405b1c8922c3c98c8d00e) ) // decrypted opcodes
	ROM_CONTINUE(             0x00000, 0x8000 ) // decrypted data

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "bg.120",       0x0000, 0x2000, CRC(d848faff) SHA1(5974cc0c3090800ca79f580a620f5b6615f5d039) )

	ROM_REGION( 0xc000, "tiles", 0 )
	ROM_LOAD( "bg.62",        0x0000, 0x2000, CRC(7e3ea4eb) SHA1(8bf020b083e2da12fe95ddae9ac7a385490525bc) )
	ROM_LOAD( "bg.61",        0x2000, 0x2000, CRC(4dd3d39d) SHA1(759fca021f8d59e861dc19543d5a184428a5e472) )
	ROM_LOAD( "bg.64",        0x4000, 0x2000, CRC(17368663) SHA1(e8f2ac6de0fddf08aefae07e693cac100cfb0db4) )
	ROM_LOAD( "bg.63",        0x6000, 0x2000, CRC(0c8bc404) SHA1(fc96fb682da3af6b7fc852cea6d8a957c4ce57e3) )
	ROM_LOAD( "bg.66",        0x8000, 0x2000, CRC(2b7dc4fa) SHA1(79d3677b24682cee0c08088433646800703be531) )
	ROM_LOAD( "bg.65",        0xa000, 0x2000, CRC(ed121306) SHA1(89f812b3954922e22fcf8d9cc4ee5ba295279cb6) )

	ROM_REGION( 0x10000, "sprites", 0 )
	ROM_LOAD( "bg.117",       0x0000, 0x4000, CRC(e99cc920) SHA1(b2b9199a9296e0c34fcf4dd20ffd3e8de08f42da) )
	ROM_LOAD( "bg.04",        0x4000, 0x4000, CRC(213057f8) SHA1(a872631aaa2b73e9198f2ad6cede2a889279e610) )
	ROM_LOAD( "bg.110",       0x8000, 0x4000, CRC(064c812c) SHA1(673790dc5131fd280333386a0e9915fb94e9f3e1) )
	ROM_LOAD( "bg.05",        0xc000, 0x4000, CRC(02e0b040) SHA1(fb626fc31dfe25bf9fac0c8d76d5041609b06e82) )

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "pr5317.76",    0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END


/*

Noboranka, Data East, 1986
Hardware info by Guru

PCB Layout
----------

Top

DE-0222-2                           /-Sub PCB on top
|-------------------------------|--/-----------|
| DSW2   PK-2                   |              |
| DSW1                          |              |
|                             6116             |
|                             6116             |
|        20MHz                  |              |
|J                            DM02             |
|A                            DM01-------------|
|M       8255                 DM00             |
|M  DM03 Z80A                                  |
|A  6116                                       |
|                                              |
|                                         DC-11|
|  76489                                       |
|  76489         *DM-12.IC3               PK-1 |
|MB3730 VOL 8MHz *DM-11       6116             |
|----------------------------------------------|
Notes:
      * - These parts below PCB on a small sub-board DE-0271-0
      PK1/PK2/DM-11 - PALs
      DC-11/DM-12 - 82S129 PROMs
      Z80A clock - 4.00MHz [20/5]
      76489 clock - 2.00MHz [8/4]
      VSync - 60.095Hz
      HSync - 15.444kHz


Sub PCB

DE-0272-0
|---------------|
|               |
|               |
| 8751H         |
|               |
|8MHz           |
|               |
|---------------|
8751 clock - 8.000MHz, labelled 'DM'


Bottom

DE-0223-2
|----------------------------------------------|
|          DECO_291-0  Z80B                    |
|                                              |
|    PK-3          TC15G008AP                  |
|                                       CXK5864|
|    PK-4      DM04        TMM2018             |
|                                         DM08 |
|              DM05        TMM2018             |
|                                         DM09 |
|              DM06                            |
|                                         DM10 |
|              DM07                            |
|                            8147       8147   |
|                                              |
|                   CXK5814  2148              |
|                   CXK5814  2148              |
|----------------------------------------------|
Notes:
      PK3/PK4 - PALS
      DECO_291-0 - Custom DIP28
      TC15G008AP - Gate Array DIP48
      Z80B clock - 4.00MHz [20/5]

*/
ROM_START( nob )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "dm08.1f", 0x00000, 0x8000, CRC(98d602d6) SHA1(a0f1e6d243f2e07703bb641434dce46d0ddc15ae) )
	ROM_LOAD( "dm10.1k", 0x10000, 0x8000, CRC(e7c06663) SHA1(8ae42b0875afe60ef672f2285aeb72da1c7e167b) )
	ROM_LOAD( "dm09.1h", 0x18000, 0x8000, CRC(dc4c872f) SHA1(aab85203cfd2463ffddfd48e87733fb8d6d8bcf6) )

	ROM_REGION( 0x1000, "mcu", 0 )
	ROM_LOAD( "dm.bin", 0x00000, 0x1000, CRC(6fde9dcb) SHA1(e1340644471a149b49a616c59445c85785e44fa4) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "dm03.9h", 0x0000, 0x4000, CRC(415adf76) SHA1(fbd6f8921aa3246702983ba81fa9ae53fa10c19d) )
	ROM_RELOAD(          0x4000, 0x4000 )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "dm02.13b", 0x08000, 0x8000, CRC(f12df039) SHA1(159de205f77fd74da30717054e6ddda2c0bb63d0) )
	ROM_LOAD( "dm01.12b", 0x00000, 0x8000, CRC(446fbcdd) SHA1(e3c8364eccfa6c8af7a57b599238b0e4ebe8cc59) )
	ROM_LOAD( "dm00.10b", 0x10000, 0x8000, CRC(35f396df) SHA1(ebf0a252513ae2b31ef012ac71d64fb20b8725cc) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "dm04.5f", 0x00000, 0x8000, CRC(2442b86d) SHA1(2eed80e1ff9cd782990142d0d73ca4fa13db4731) )
	ROM_LOAD( "dm06.5k", 0x08000, 0x8000, CRC(e33743a6) SHA1(56dce565523f19e673c9272992030386ca648e41) )
	ROM_LOAD( "dm05.5h", 0x10000, 0x8000, CRC(7fbba01d) SHA1(ded22806ae0d6642b45cd33c0ceab67390a6e319) )
	ROM_LOAD( "dm07.5l", 0x18000, 0x8000, CRC(85e7a29f) SHA1(0ca77c66599650f157450d703682ec114f0453cf) )

	ROM_REGION( 0x0300, "color_proms", 0 )
	// the first 2 proms were missing from the dump, but are clearly needed...
	ROM_LOAD( "nobo_pr.16d", 0x0000, 0x0100, CRC(95010ac2) SHA1(deaf84b408cd1f3396eb851ef04cc1654d5e9a46) ) // palette red component
	ROM_LOAD( "nobo_pr.15d", 0x0100, 0x0100, CRC(c55aac0c) SHA1(0f7f2d383a90e9f7f319626b4d5565805f44a1f9) ) // palette green component
	ROM_LOAD( "dm-12.ic3", 0x0200, 0x0100, CRC(de394cee) SHA1(511c53f22459e5e238b48685f85b10f5e15f2ac1) ) // palette blue component

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "dc-11.6a", 0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END

// the bootleg has different protection..
ROM_START( nobb )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "nobo-t.bin", 0x00000, 0x8000, CRC(176fd168) SHA1(f262521f07e5340f175019e2a06a54120a4aa3b7) )
	ROM_LOAD( "nobo-r.bin", 0x10000, 0x8000, CRC(d61cf3c9) SHA1(0f80011d713c51e67853810813ebba579ade0303) )
	ROM_LOAD( "nobo-s.bin", 0x18000, 0x8000, CRC(b0e7697f) SHA1(ad5394ca629152a8c73fb85d3fce8ea620ae6ff1) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "nobo-m.bin", 0x0000, 0x4000, CRC(415adf76) SHA1(fbd6f8921aa3246702983ba81fa9ae53fa10c19d) )
	ROM_RELOAD(             0x4000, 0x4000 )

	ROM_REGION( 0x18000, "tiles", 0 )
	ROM_LOAD( "nobo-j.bin", 0x08000, 0x8000, CRC(f12df039) SHA1(159de205f77fd74da30717054e6ddda2c0bb63d0) )
	ROM_LOAD( "nobo-k.bin", 0x00000, 0x8000, CRC(446fbcdd) SHA1(e3c8364eccfa6c8af7a57b599238b0e4ebe8cc59) )
	ROM_LOAD( "nobo-l.bin", 0x10000, 0x8000, CRC(35f396df) SHA1(ebf0a252513ae2b31ef012ac71d64fb20b8725cc) )

	ROM_REGION( 0x20000, "sprites", 0 )
	ROM_LOAD( "nobo-q.bin", 0x00000, 0x8000, CRC(2442b86d) SHA1(2eed80e1ff9cd782990142d0d73ca4fa13db4731) )
	ROM_LOAD( "nobo-o.bin", 0x08000, 0x8000, CRC(e33743a6) SHA1(56dce565523f19e673c9272992030386ca648e41) )
	ROM_LOAD( "nobo-p.bin", 0x10000, 0x8000, CRC(7fbba01d) SHA1(ded22806ae0d6642b45cd33c0ceab67390a6e319) )
	ROM_LOAD( "nobo-n.bin", 0x18000, 0x8000, CRC(85e7a29f) SHA1(0ca77c66599650f157450d703682ec114f0453cf) )

	ROM_REGION( 0x0300, "color_proms", 0 )
	ROM_LOAD( "nobo_pr.16d", 0x0000, 0x0100, CRC(95010ac2) SHA1(deaf84b408cd1f3396eb851ef04cc1654d5e9a46) ) // palette red component
	ROM_LOAD( "nobo_pr.15d", 0x0100, 0x0100, CRC(c55aac0c) SHA1(0f7f2d383a90e9f7f319626b4d5565805f44a1f9) ) // palette green component
	ROM_LOAD( "nobo_pr.14d", 0x0200, 0x0100, CRC(de394cee) SHA1(511c53f22459e5e238b48685f85b10f5e15f2ac1) ) // palette blue component

	ROM_REGION( 0x0100, "lookup_proms", 0 )
	ROM_LOAD( "nobo_pr.13a", 0x0000, 0x0100, CRC(648350b8) SHA1(c7986aa9127ef5b50b845434cb4e81dff9861cd2) )
ROM_END



/*************************************
 *
 *  Generic driver initialization
 *
 *************************************/

void system1_state::init_bank44()
{
	m_videomode_custom = &system1_state::bank44_custom_w;
}

void system1_state::init_bank0c()
{
	m_videomode_custom = &system1_state::bank0c_custom_w;
}


void system1_state::init_myherok()
{
	// extra layer of encryption applied BEFORE the usual CPU decryption
	// probably bootleg?

	int A;
	u8 *rom;

	// additionally to the usual protection, all the program ROMs have data lines D0 and D1 swapped.
	rom = m_maincpu_region->base();
	for (A = 0;A < 0xc000;A++)
		rom[A] = (rom[A] & 0xfc) | ((rom[A] & 1) << 1) | ((rom[A] & 2) >> 1);

	// the tile gfx ROMs are mangled as well:
	rom = memregion("tiles")->base();

	// the first ROM has data lines D0 and D6 swapped.
	for (A = 0x0000;A < 0x4000;A++)
		rom[A] = (rom[A] & 0xbe) | ((rom[A] & 0x01) << 6) | ((rom[A] & 0x40) >> 6);

	// the second ROM has data lines D1 and D5 swapped.
	for (A = 0x4000;A < 0x8000;A++)
		rom[A] = (rom[A] & 0xdd) | ((rom[A] & 0x02) << 4) | ((rom[A] & 0x20) >> 4);

	// the third ROM has data lines D0 and D6 swapped.
	for (A = 0x8000;A < 0xc000;A++)
		rom[A] = (rom[A] & 0xbe) | ((rom[A] & 0x01) << 6) | ((rom[A] & 0x40) >> 6);

	// also, all three ROMs have address lines A4 and A5 swapped.
	for (A = 0;A < 0xc000;A++)
	{
		int A1;
		u8 temp;

		A1 = (A & 0xffcf) | ((A & 0x0010) << 1) | ((A & 0x0020) >> 1);
		if (A < A1)
		{
			temp = rom[A];
			rom[A] = rom[A1];
			rom[A1] = temp;
		}
	}
}



void system1_state::init_blockgal()
{
	downcast<mc8123_device &>(*m_maincpu).decode(m_maincpu_region->base(), m_decrypted_opcodes, 0x8000);
}


void system1_state::init_wbml()
{
	init_bank0c();
	m_banked_decrypted_opcodes = std::make_unique<u8[]>(m_maincpu_region->bytes());
	downcast<mc8123_device &>(*m_maincpu).decode(m_maincpu_region->base(), m_banked_decrypted_opcodes.get(), m_maincpu_region->bytes());
}



u8 system1_state::nob_start_r()
{
	// in reality, it's likely some M1-dependent behavior
	return (m_maincpu->pc() <= 0x0003) ? 0x80 : m_maincpu_region->base()[1];
}

void system1_state::init_nob()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	address_space &iospace = m_maincpu->space(AS_IO);

	init_bank44();

	// hack to fix incorrect JMP at start, which should obviously be to $0080
	// patching the ROM causes errors in the self-test
	// in real-life, it could be some behavior dependent upon M1
	space.install_read_handler(0x0001, 0x0001, read8smo_delegate(*this, FUNC(system1_state::nob_start_r)));

	// install MCU communications
	iospace.install_readwrite_handler(0x18, 0x18, read8smo_delegate(*this, FUNC(system1_state::nob_maincpu_latch_r)), write8smo_delegate(*this, FUNC(system1_state::nob_maincpu_latch_w)));
	iospace.install_read_handler(0x1c, 0x1c, read8smo_delegate(*this, FUNC(system1_state::nob_mcu_status_r)));
}

void system1_state::init_nobb()
{
	// Patch to get PRG ROMS ('T', 'R' and 'S) status as "GOOD" in the "test mode" not really needed

//  u8 *ROM = m_maincpu_region->base();

//  ROM[0x3296] = 0x18;     // 'jr' instead of 'jr z' - 'T' (PRG Main ROM)
//  ROM[0x32be] = 0x18;     // 'jr' instead of 'jr z' - 'R' (Banked ROM 1)
//  ROM[0x32ea] = 0x18;     // 'jr' instead of 'jr z' - 'S' (Banked ROM 2)

	// Patch to avoid the internal checksum that will hang the game after an amount of time
	// (check code at 0x3313 in 'R' (banked ROM 1))

//  ROM[0x10000 + 0 * 0x8000 + 0x3347] = 0x18;  // 'jr' instead of 'jr z'

	init_bank44();

	address_space &iospace = m_maincpu->space(AS_IO);
	iospace.install_read_handler(0x1c, 0x1c, read8smo_delegate(*this, FUNC(system1_state::nobb_inport1c_r)));
	iospace.install_read_handler(0x02, 0x02, read8smo_delegate(*this, FUNC(system1_state::nobb_inport22_r)));
	iospace.install_read_handler(0x03, 0x03, read8smo_delegate(*this, FUNC(system1_state::nobb_inport23_r)));
	iospace.install_write_handler(0x04, 0x04, write8smo_delegate(*this, FUNC(system1_state::nobb_outport24_w)));
}


void system1_state::init_bootleg()
{
	memcpy(m_decrypted_opcodes, m_maincpu_region->base() + 0x10000, 0x8000);
}


void system1_state::init_bootsys2()
{
	init_bank0c();
	m_bank0d->set_base(m_maincpu_region->base() + 0x20000);
	m_bank1d->configure_entries(0, 4, m_maincpu_region->base() + 0x30000, 0x4000);
}

void system1_state::init_bootsys2d()
{
	init_bank0c();
	m_bank0d->set_base(m_maincpu_region->base());
	m_bank1d->configure_entries(0, 4, m_maincpu_region->base() + 0x10000, 0x4000);
}



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

// PPI-based System 1
GAME( 1983, starjack,   0,        starjack,  starjack,  system1_state, empty_init,     ROT270, "Sega", "Star Jacker (Sega)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, starjacks,  starjack, starjack,  starjacks, system1_state, empty_init,     ROT270, "Sega (Stern Electronics license)", "Star Jacker (Stern Electronics)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, upndown,    0,        upndown,   upndown,   system1_state, empty_init,     ROT270, "Sega", "Up'n Down (315-5030)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, upndownu,   upndown,  sys1ppi,   upndown,   system1_state, empty_init,     ROT270, "Sega", "Up'n Down (not encrypted)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, regulus,    0,        regulus,   regulus,   system1_state, empty_init,     ROT270, "Sega", "Regulus (315-5033, Rev A.)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, reguluso,   regulus,  regulus,   reguluso,  system1_state, empty_init,     ROT270, "Sega", "Regulus (315-5033)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, regulusu,   regulus,  sys1ppi,   regulus,   system1_state, empty_init,     ROT270, "Sega", "Regulus (not encrypted)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, mrviking,   0,        mrviking,  mrviking,  system1_state, empty_init,     ROT270, "Sega", "Mister Viking (315-5041)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, mrvikingj,  mrviking, mrviking,  mrvikingj, system1_state, empty_init,     ROT270, "Sega", "Mister Viking (315-5041, Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, swat,       0,        swat,      swat,      system1_state, empty_init,     ROT270, "Coreland / Sega", "SWAT (315-5048)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, flickyo,    flicky,   flickyo,   flicky,    system1_state, empty_init,     ROT0,   "Sega", "Flicky (64k Version, 315-5051, set 1)", MACHINE_SUPPORTS_SAVE ) // 84/5/24
GAME( 1984, flickys1,   flicky,   flickyo,   flickys1,  system1_state, empty_init,     ROT0,   "Sega", "Flicky (64k Version, 315-5051, set 2)", MACHINE_SUPPORTS_SAVE ) // 84/11/26
GAME( 1984, flickyup,   flicky,   flickyo,   flicky,    system1_state, empty_init,     ROT0,   "Sega", "Flicky (64k Version, on Up'n Down boardset, set 1)", MACHINE_SUPPORTS_SAVE ) // 84/5/17
GAME( 1984, flickyupa,  flicky,   flickyo,   flicky,    system1_state, empty_init,     ROT0,   "bootleg", "Flicky (64k Version, on Up'n Down boardset, set 2)", MACHINE_SUPPORTS_SAVE ) // 84/7/05
GAME( 1984, wmatch,     0,        wmatch,    wmatch,    system1_state, empty_init,     ROT270, "Sega", "Water Match (315-5064)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, bullfgt,    0,        bullfgt,   bullfgt,   system1_state, empty_init,     ROT0,   "Coreland / Sega", "Bullfight (315-5065)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, nprinces,   seganinj, flickyo,   seganinj,  system1_state, empty_init,     ROT0,   "bootleg?", "Ninja Princess (315-5051, 64k Ver. bootleg?)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, nprincesu,  seganinj, sys1ppi,   seganinj,  system1_state, empty_init,     ROT0,   "Sega", "Ninja Princess (64k Ver. not encrypted)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, wboy2,      wboy,     wboy2,     wboy,      system1_state, empty_init,     ROT0,   "Escape (Sega license)", "Wonder Boy (set 2, 315-5178)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, wboy2u,     wboy,     sys1ppi,   wboy,      system1_state, empty_init,     ROT0,   "Escape (Sega license)", "Wonder Boy (set 2, not encrypted)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, wboy6,      wboy,     wboy6,     wboy,      system1_state, empty_init,     ROT0,   "Escape (Sega license)", "Wonder Boy (set 6, 315-5179)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, wbdeluxe,   wboy,     sys1ppi,   wbdeluxe,  system1_state, empty_init,     ROT0,   "hack (Vision Electronics)", "Wonder Boy Deluxe", MACHINE_SUPPORTS_SAVE )
GAME( 1986, nob,        0,        nob,       nob,       system1_state, init_nob,       ROT270, "Coreland / Data East Corporation", "Noboranka (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, nobb,       nob,      nobb,      nob,       system1_state, init_nobb,      ROT270, "bootleg (Game Electronics)", "Noboranka (Japan, bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, raflesiau,  raflesia, sys1ppi,   raflesia,  system1_state, empty_init,     ROT270, "Coreland / Sega", "Rafflesia (not encrypted)", MACHINE_SUPPORTS_SAVE )

// PIO-based System 1
GAME( 1984, flicky,     0,        flicky,    flicky,    system1_state, empty_init,     ROT0,   "Sega", "Flicky (128k Version, 315-5051)", MACHINE_SUPPORTS_SAVE ) // 1984/05/24
GAME( 1984, flickya,    flicky,   flicky,    flicky,    system1_state, empty_init,     ROT0,   "Sega", "Flicky (128k Version, 315-5051, larger ROMs)", MACHINE_SUPPORTS_SAVE ) // 1984/05/24
GAME( 1984, flickyb,    flicky,   flicky,    flickyb,   system1_state, empty_init,     ROT0,   "Sega", "Flicky (128k Version, 315-5051, larger ROMs, newer)", MACHINE_SUPPORTS_SAVE ) // 1984/10/07
GAME( 1984, flickys2,   flicky,   sys1pio,   flickys2,  system1_state, empty_init,     ROT0,   "Sega", "Flicky (128k Version, not encrypted)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, thetogyu,   bullfgt,  thetogyu,  bullfgt,   system1_state, empty_init,     ROT0,   "Coreland / Sega", "The Togyu (315-5065, Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, spatter,    0,        spatter,   spatter,   system1_state, empty_init,     ROT0,   "Sega", "Spatter (315-5096)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, spattera,   spatter,  spattera,  spatter,   system1_state, empty_init,     ROT0,   "Sega", "Spatter (315-5099)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, ssanchan,   spatter,  spatter,   spatter,   system1_state, empty_init,     ROT0,   "Sega", "Sanrin San Chan (Japan, 315-5096)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, pitfall2,   0,        pitfall2,  pitfall2,  system1_state, empty_init,     ROT0,   "Sega", "Pitfall II (315-5093)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, pitfall2a,  pitfall2, pitfall2,  pitfall2,  system1_state, empty_init,     ROT0,   "Sega", "Pitfall II (315-5093, Flicky Conversion)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, pitfall2u,  pitfall2, sys1pio,   pitfall2u, system1_state, empty_init,     ROT0,   "Sega", "Pitfall II (not encrypted)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, seganinj,   0,        seganinj,  seganinj,  system1_state, empty_init,     ROT0,   "Sega", "Sega Ninja (315-5102)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, seganinju,  seganinj, sys1pio,   seganinj,  system1_state, empty_init,     ROT0,   "Sega", "Sega Ninja (not encrypted)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, seganinja,  seganinj, seganinja, seganinj,  system1_state, empty_init,     ROT0,   "Sega", "Sega Ninja (315-5113)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, ninja,      seganinj, seganinj,  seganinj,  system1_state, empty_init,     ROT0,   "Sega", "Ninja (315-5102)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, nprinceso,  seganinj, nprinceso, seganinj,  system1_state, empty_init,     ROT0,   "Sega", "Ninja Princess (315-5098, 128k Ver.)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, nprincesb,  seganinj, flicky,    seganinj,  system1_state, empty_init,     ROT0,   "bootleg?", "Ninja Princess (315-5051?, 128k Ver. bootleg?)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, imsorry,    0,        imsorry,   imsorry,   system1_state, empty_init,     ROT0,   "Coreland / Sega", "I'm Sorry (315-5110, US)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, imsorryj,   imsorry,  imsorry,   imsorry,   system1_state, empty_init,     ROT0,   "Coreland / Sega", "Gonbee no I'm Sorry (315-5110, Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, teddybb,    0,        teddybb,   teddybb,   system1_state, empty_init,     ROT0,   "Sega", "TeddyBoy Blues (315-5115, New Ver.)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, teddybbo,   teddybb,  teddybb,   teddybb,   system1_state, empty_init,     ROT0,   "Sega", "TeddyBoy Blues (315-5115, Old Ver.)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, teddybboa,  teddybb,  teddybboa, teddybb,   system1_state, empty_init,     ROT0,   "Sega", "TeddyBoy Blues (315-5111, Old Ver.)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, teddybbobl, teddybb,  teddybb,   teddybb,   system1_state, empty_init,     ROT0,   "bootleg", "TeddyBoy Blues (Old Ver. bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, myhero,     0,        sys1pio,   myhero,    system1_state, empty_init,     ROT0,   "Coreland / Sega", "My Hero (US, not encrypted)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, sscandal,   myhero,   myheroj,   myhero,    system1_state, empty_init,     ROT0,   "Coreland / Sega", "Seishun Scandal (315-5132, Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, myherobl,   myhero,   myheroj,   myhero,    system1_state, empty_init,     ROT0,   "bootleg", "My Hero (bootleg, 315-5132 encryption)", MACHINE_SUPPORTS_SAVE ) // cloned 315-5132 encryption? might be a direct copy of an undumped original set
GAME( 1985, myherok,    myhero,   myheroj,   myhero,    system1_state, init_myherok,   ROT0,   "Coreland / Sega", "Cheongchun Ilbeonji (Korea)", MACHINE_SUPPORTS_SAVE ) // possible bootleg, has extra encryption
GAME( 1985, 4dwarrio,   0,        _4dwarrio, 4dwarrio,  system1_state, empty_init,     ROT0,   "Coreland / Sega", "4-D Warriors (315-5162)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, raflesia,   0,        _4dwarrio, raflesia,  system1_state, empty_init,     ROT270, "Coreland / Sega", "Rafflesia (315-5162)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, wboy,       0,        wboy,      wboy,      system1_state, empty_init,     ROT0,   "Escape (Sega license)", "Wonder Boy (set 1, 315-5177)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, wboyo,      wboy,     wboyo,     wboy,      system1_state, empty_init,     ROT0,   "Escape (Sega license)", "Wonder Boy (set 2, 315-5135)", MACHINE_SUPPORTS_SAVE ) // aka 317-0003
GAME( 1986, wboy3,      wboy,     wboyo,     wboy3,     system1_state, empty_init,     ROT0,   "Escape (Sega license)", "Wonder Boy (set 3, 315-5135)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, wboy4,      wboy,     _4dwarrio, wboy,      system1_state, empty_init,     ROT0,   "Escape (Sega license)", "Wonder Boy (315-5162, 4-D Warriors Conversion)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, wboyu,      wboy,     sys1pio,   wboyu,     system1_state, empty_init,     ROT0,   "Escape (Sega license)", "Wonder Boy (prototype?)", MACHINE_SUPPORTS_SAVE ) // appears to be a very early / unfinished version.
GAME( 1986, wboy5,      wboy,     wboyo,     wboy3,     system1_state, empty_init,     ROT0,   "bootleg", "Wonder Boy (set 5, bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, wboyub,     wboy,     wboy,      wboy,      system1_state, empty_init,     ROT0,   "bootleg", "Wonder Boy (US bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, wboyblt,    wboy,     wboyo,     wboy3,     system1_state, empty_init,     ROT0,   "bootleg (Tecfri)", "Wonder Boy (Tecfri bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, blockgal,   0,        blockgal,  blockgal,  system1_state, init_blockgal,  ROT90,  "Sega / Vic Tokai", "Block Gal (MC-8123B, 317-0029)", MACHINE_SUPPORTS_SAVE)

// PIO-based System 1 with ROM banking
GAME( 1985, hvymetal,   0,        wboyo,     hvymetal,  system1_state, init_bank44,    ROT0,   "Sega", "Heavy Metal (315-5135)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, gardia,     0,        gardia,    gardia,    system1_state, init_bank44,    ROT270, "Coreland / Sega", "Gardia (317-0006)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE)
GAME( 1986, brain,      0,        sys1pio,   brain,     system1_state, init_bank44,    ROT0,   "Coreland / Sega", "Brain", MACHINE_SUPPORTS_SAVE )

// System 2
GAME( 1985, choplift,   0,        choplift,  choplift,  system1_state, init_bank0c,    ROT0,   "Sega", "Choplifter (8751 315-5151)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, chopliftu,  choplift, sys2row,   choplift,  system1_state, init_bank0c,    ROT0,   "Sega", "Choplifter (unprotected)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, chopliftbl, choplift, sys2row,   choplift,  system1_state, init_bank0c,    ROT0,   "bootleg", "Choplifter (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1985, shtngmst,   0,        shtngmst,  shtngmst,  shtngmst_state,init_bank0c,    ROT0,   "Sega", "Shooting Master (8751 315-5159a)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, gardiab,    gardia,   gardiab,   gardia,    system1_state, init_bank44,    ROT270, "bootleg", "Gardia (317-0007?, bootleg)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1986, gardiaj,    gardia,   gardiaj,   gardia,    system1_state, init_bank44,    ROT270, "Coreland / Sega", "Gardia (Japan, 317-0006)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1986, wboysys2,   wboy,     wboysys2,  wboysys2,  system1_state, init_bank0c,    ROT0,   "Escape (Sega license)", "Wonder Boy (system 2, set 1, 315-5177)", MACHINE_SUPPORTS_SAVE )
GAME( 1986, wboysys2a,  wboy,     wboysys2a, wboysys2,  system1_state, init_bank0c,    ROT0,   "Escape (Sega license)", "Wonder Boy (system 2, set 2, 315-5176)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, tokisens,   0,        sys2x,     tokisens,  system1_state, init_wbml,      ROT90,  "Sega", "Toki no Senshi - Chrono Soldier (MC-8123, 317-0040)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, tokisensa,  tokisens, sys2,      tokisensa, system1_state, init_bank0c,    ROT90,  "Sega", "Toki no Senshi - Chrono Soldier (prototype?)", MACHINE_SUPPORTS_SAVE ) // or bootleg?
GAME( 1987, wbml,       0,        sys2x,     wbml,      system1_state, init_wbml,      ROT0,   "Sega / Westone", "Wonder Boy: Monster Land (Japan New Ver., MC-8123, 317-0043)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, wbmljo,     wbml,     sys2x,     wbml,      system1_state, init_wbml,      ROT0,   "Sega / Westone", "Wonder Boy: Monster Land (Japan Old Ver., MC-8123, 317-0043)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, wbmljb,     wbml,     wbmlb,     wbml,      system1_state, init_bootsys2,  ROT0,   "bootleg", "Wonder Boy: Monster Land (Japan bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, wbmlb,      wbml,     wbmlb,     wbml,      system1_state, init_bootsys2,  ROT0,   "bootleg", "Wonder Boy: Monster Land (English bootleg set 1)", MACHINE_SUPPORTS_SAVE)
GAME( 1987, wbmlb2,     wbml,     wbmlb,     wbml,      system1_state, init_bootsys2,  ROT0,   "bootleg", "Wonder Boy: Monster Land (English bootleg set 4)", MACHINE_SUPPORTS_SAVE)
GAME( 1987, wbmlbg,     wbml,     wbmlb,     wbml,      system1_state, init_bootsys2,  ROT0,   "bootleg (Galaxy Electronics)", "Wonder Boy: Monster Land (English bootleg set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, wbmlbge,    wbml,     wbmlb,     wbml,      system1_state, init_bootsys2,  ROT0,   "bootleg (Gecas)", "Wonder Boy: Monster Land (English bootleg set 3)", MACHINE_SUPPORTS_SAVE )
GAME( 2009, wbmlvc,     wbml,     wbmlb,     wbml,      system1_state, init_bootsys2,  ROT0,   "Sega", "Wonder Boy: Monster Land (English, Virtual Console)", MACHINE_SUPPORTS_SAVE )
GAME( 2009, wbmlvcd,    wbml,     wbmlb,     wbml,      system1_state, init_bootsys2d, ROT0,   "bootleg (mpatou)", "Wonder Boy: Monster Land (decrypted bootleg of English, Virtual Console release)", MACHINE_SUPPORTS_SAVE ) // fully decrypted version
GAME( 1987, wbmld,      wbml,     wbmlb,     wbml,      system1_state, init_bootsys2d, ROT0,   "bootleg (mpatou)", "Wonder Boy: Monster Land (decrypted bootleg of Japan New Ver., MC-8123, 317-0043)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, wbmljod,    wbml,     wbmlb,     wbml,      system1_state, init_bootsys2d, ROT0,   "bootleg (mpatou)", "Wonder Boy: Monster Land (decrypted bootleg of Japan Old Ver., MC-8123, 317-0043)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, wbmlh,      wbml,     wbmlb,     wbml,      system1_state, init_bootsys2,  ROT0,   "bootleg", "Wonder Boy: Monster Land (English, difficulty hack)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, dakkochn,   0,        sys2x,     dakkochn,  dakkochn_state,init_wbml,      ROT0,   "White Board", "DakkoChan House (MC-8123B, 317-5014)", MACHINE_SUPPORTS_SAVE )
GAME( 1987, blockgalb,  blockgal, blockgalb, blockgalb, system1_state, init_bootleg,   ROT90,  "bootleg", "Block Gal (bootleg)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, ufosensi,   0,        ufosensi,  ufosensi,  system1_state, init_wbml,      ROT0,   "Sega", "Ufo Senshi Yohko Chan (MC-8123, 317-0064)", MACHINE_SUPPORTS_SAVE )
GAME( 1988, ufosensib,  ufosensi, ufosensib, ufosensi,  system1_state, init_bootsys2,  ROT0,   "bootleg", "Ufo Senshi Yohko Chan (bootleg, not encrypted)", MACHINE_SUPPORTS_SAVE )
