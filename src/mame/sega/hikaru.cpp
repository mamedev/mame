// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Sega Hikaru / 'Samurai' */

/*

Sega Hikaru Hardware Overview (last updated 5th August 2009 at 3:45pm)
-----------------------------

Note! This document will be updated from time to time when more dumps are available.

This document covers all the known Sega Hikaru games. The graphics are quite breathtaking
and this system is said to be one of the most expensive arcade boards developed by Sega.
The games on this system include....
Air Trix                     (C) Sega, 2001
Brave Fire Fighters          (C) Sega, 1999
*Cyber Troopers Virtual On 4 (C) Sega, 2001
Nascar Arcade                (C) Sega, 2000
Planet Harriers              (C) Sega, 2001
Star Wars Racer Arcade       (C) Sega, 2000

! - denotes secured but not fully dumped yet
* - denotes not dumped yet.

The Sega Hikaru system comprises the following PCBs.....
Main board     - 2 known versions exists. They're mostly the same. It contains many thin BGAs,
                 lots of RAM, 2x SH4 CPUs and 1x 16MBit boot EPROM. Because of the use of many thin BGA
                 ICs, the main board is VERY fragile and is mounted inside a metal box.
ROM PCB        - 2 known versions exist. They contain EPROMs, up to 16x TSOP48 flashROMs or SOP44
                 maskROMs, a small amount of SRAM and some security ICs that are also used on
                 NAOMI ROM carts (315-5881 - with a unique 317-xxxx number per game, a couple
                 of PLCC FPGAs and a X76F100 secured EEPROM). Plugs into mainboard into CONN3 and CONN4
Network PCB    - Contains a 68000, some SRAM, a few Sega ICs and a CPLD. Plugs into the top of the ROM board
AICA PCB       - Contains one RAM and one QFP100 IC. Plugs into the mainboard into CONN5
I/O PCB        - Uses a standard Sega JVS I/O board. Contains USB connectors type A & B, a
                 couple of long IDC cable connectors for controls and a Toshiba TMP90PH44 MCU
Filter PCB     - Contains external connectors for power, network, USB, audio and video. This board is mounted
                 to the metal box and plugs into the main board into CONN1 and CONN2


Main Board
----------
Version 1 - 837-13402 171-7639E SEGA 1998
Version 2 - 837-14097 171-7639F SEGA 1998
|------------------------------------------------------------------------------------|
|       MC33470.103            DSW(4)                           CONN4                |
|                                    93C46.115                                       |
|                          SH-4.13                          62256.72       93C46.78  |-|
|        SH-4.21                               PAL1         62256.73                 | |
|                        5264165FTTA60.15      (GAL16V8)                    3771     | |
|                        5264165FTTA60.16                      315-6146.74           | |
|    5264165FTTA60.22    5264165FTTA60.17S                               14.7456MHz  | |
|    5264165FTTA60.23    5264165FTTA60.18S                               32MHz       | |CONN1
|    5264165FTTA60.24S                                                               | |
|    5264165FTTA60.25S                                         3771 3771             | |
|                         315-6154.19                                                | |
|      315-6154.26                                                          PC910.102| |
|               33.3333MHz                                        CY37128.95         | |
|               CY2308SC.4                         CY2292SL.1     (=315-6202)        |-|
|HY57V161610.28                                                           ADM485.80  |
| PAL2                      CONN3                                 62256.91S          |
| (GAL16V8)                                                       62256.92S          |
|HY57V161610.35                                                                      |
|HY57V161610.36S                 315-6087.43  HY57V161610.44                         |-|
|HY57V161610.33 315-6083A.31                  HY57V161610.45S         EPR-23400(A).94| |
|HY57V161610.34S          D432232.32                   24.576MHz                     | |
|(PAL on V2 MB)                              HY57V161610.46   ADV7120      BATTERY   | |CONN2
|LED1                                        HY57V161610.47S  ADV7120                | |
|LED2                                                                                | |
|LED3                                                                                |-|
|LED4               315-6197.38                     315-6084.41  CONN5               |
|SW1                                315-6085.40                        K4S641632.98  |
|SW2                                                                                 |
|SW3    315-6086.37                                                   315-6232.96    |
|                                                   315-6084.42                      |
|                   315-6197.39     CY2308SC.2                  33.8688MHz           |
|(LED 1-4 moved here)          41.6666MHz                       25MHz                |
|(on V2 MB)                                                                          |
|------------------------------------------------------------------------------------|
Notes:
      In test mode, the ROM test will show only 'GOOD' for all ROMs. On test completion
      the screen will show 'PRESS TEST BUTTON TO EXIT'
      However if the SERVICE button is pressed, the screen will show the byte
      checksums of all ROMs.


ROM PCB
-------

Type 1 - 837-14140   171-8144B
|------------------------------------------------------------------------------------|
|                                                                                    |
|   IC45   IC41   IC37   IC50   IC46   IC42   IC38                                   |
|                                                                                    |
|                                                                        LATTICE     |
|   IC49   IC57   IC53   IC66   IC62   IC58   IC54                       M4A3-32/32  |
|                                                                        (315-6323)  |
|                                                                  X76F100           |
|   IC61                                                                             |
|                                                                         CY7C1399   |
|                                                                         CY7C1399   |
|   IC65                                                                             |
|           IC29  IC30  IC31  IC32  IC33  IC34  IC35  IC36                           |
|                                                                       28MHz        |
|                                                                            LATTICE |
|                                                                 315-5881   PLSI2032|
|                                                                          (315-6050)|
|------------------------------------------------------------------------------------|
ROM usage -                                                   CRC (from ROM test)
           Game       Sega Part No.     ROM Type              Byte   Word
           --------------------------------------------------------------
           Air Trix -
                      MPR-23573.IC37    128M TSOP48 mask ROM  B9A5   9E67
                      MPR-23577.IC38    "                     A52A   BCE0
                      MPR-23574.IC41    "                     DABB   B621
                      MPR-23578.IC42    "                     4BD4   5E6B
                      MPR-23575.IC45    "                     0D06   AD63
                      MPR-23579.IC46    "                     790F   A27E
                      MPR-23576.IC49    "                     BDBB   4F01
                      MPR-23580.IC50    "                     14A7   6A4E
                      IC53, IC54, IC57, \
                      IC58, IC61, IC62,  Not Used
                      IC65, IC66        /

                      EPR-23601A.IC29   27C322 EPROM          FEE8   C889
                      EPR-23602A.IC30   "                     97C2   620C
                      other EPROM sockets not used

                      315-5881 stamped 317-0294-COM


                                                              CRC (from ROM test)
           Game       Sega Part No.     ROM Type              Byte   Word
           --------------------------------------------------------------
           Planet Harriers -
                      MPR-23549.IC37    128M TSOP48 mask ROM  7F16   2C37
                      MPR-23553.IC38    "                     1F9F   AAE5
                      MPR-23550.IC41    "                     986C   8D7A
                      MPR-23554.IC42    "                     BD1D   5304
                      MPR-23551.IC45    "                     9784   B33D
                      MPR-23555.IC46    "                     CB75   B08B
                      MPR-23552.IC49    "                     5056   B3A9
                      MPR-23556.IC50    "                     CBDE   BE85
                      MPR-23557.IC53    "                     3D36   05BF
                      MPR-23561.IC54    "                     D629   8ED6
                      MPR-23558.IC57    "                     B9F5   0082
                      MPR-23562.IC58    "                     5875   8163
                      MPR-23559.IC61    "                     B19D   E7CC
                      MPR-23563.IC62    "                     158C   D180
                      MPR-23560.IC65    "                     34BC   677B
                      MPR-23564.IC66    "                     5524   349E

                      EPR-23565A.IC29   27C322 EPROM          8AC6
                      EPR-23566A.IC30   "                     1CC3
                      EPR-23567A.IC31   "                     388C   13B5
                      EPR-23568A.IC32   "                     D289   5910
                      EPR-23569A.IC33   "                     09A8   578F
                      EPR-23570A.IC34   "                     73E5   94F7
                      EPR-23571A.IC35   "                     B1DE   6DAD
                      EPR-23572A.IC36   "                     9019   6DD5
                      all EPROM sockets populated

                      315-5881 stamped 317-0297-COM


Type 2 - 837-13403   171-7640B
|------------------------------------------------------------------------------------|
|               LATTICE        CY7C199               X76F100                         |
|  315-5881     PLSI2032                                                             |
|                       28MHz  CY7C199               MACH111                         |
|                                                    (315-6203A)         IC30   IC32 |
|                                                                                    |
| IC37  IC39  IC41  IC43  IC45  IC47  IC49  IC51                                     |
|                                                                                    |
|                                                                                    |
| IC53  IC55  IC57  IC59  IC61  IC63  IC65  IC67  <-- these on other side of PCB     |
|                                                                                    |
|                                                                                    |
| IC38  IC40  IC42  IC44  IC46  IC48  IC50  IC52                                     |
|                                                                                    |
|                                                IC29  IC31  IC33  IC35  IC34  IC36  |
| IC54  IC56  IC58  IC60  IC62  IC64  IC66  IC68 <-- these on other side of PCB      |
|                                                                                    |
|------------------------------------------------------------------------------------|
ROM usage -                                                   CRC (from ROM test)
           Game       Sega Part No.     ROM Type              Byte   Word
           --------------------------------------------------------------
           Star Wars Racer Arcade
                      MPR-23086.IC37    64M SOP44 mask ROM    7993  8E18
                      MPR-23087.IC38    "                     4D44  D239
                      MPR-23088.IC39    "                     4135  BEAB
                      MPR-23089.IC40    "                     F0C8  04E2
                      MPR-23090.IC41    "                     9532  4C1C
                      MPR-23091.IC42    "                     925D  02FB
                      MPR-23092.IC43    "                     0809  7050
                      MPR-23093.IC44    "                     72BC  9311
                      MPR-23094.IC45    "                     DE84  9D8A
                      MPR-23095.IC46    "                     7A5C  E7FC
                      MPR-23096.IC47    "                     6806  1392
                      MPR-23097.IC48    "                     EDF1  7BD1
                      MPR-23098.IC49    "                     B82D  E114
                      MPR-23099.IC50    "                     5792  E5E5
                      MPR-23100.IC51    "                     3AF3  A97C
                      MPR-23101.IC52    "                     A8CC  721D
                      MPR-23102.IC53    "                     CED7  D3CF
                      MPR-23103.IC54    "                     6B67  FC76
                      MPR-23104.IC55    "                     586C  6954
                      MPR-23105.IC56    "                     13A0  DB38
                      MPR-23106.IC57    "                     4F03  42BF
                      MPR-23107.IC58    "                     8EA6  ADB6
                      MPR-23108.IC59    "                     8645  FC30
                      MPR-23109.IC60    "                     3847  CA6B
                      MPR-23110.IC61    "                     4140  01C4
                      MPR-23111.IC62    "                     EBE6  8085
                      MPR-23112.IC63    "                     B68B  7467
                      MPR-23113.IC64    "                     4715  4787
                      MPR-23114.IC65    "                     3CD6  144A
                      MPR-23115.IC66    "                     E5D3  BA35
                      MPR-23116.IC67    "                     E668  08ED
                      MPR-23117.IC68    "                     1FE8  C4A1

                      EPR-23174.IC29    27C322 EPROM          3B2E
                      EPR-23175.IC30    "                     F377
                      EPR-23176.IC31    "                     5F01   4174
                      EPR-23177.IC32    "                     3594   38B3
                      other EPROM sockets not used

                      315-5881 stamped 317-0277-COM


AICA PCB
--------------

837-13629  171-7911B  SEGA 1998
|------------------------|
|       K4S641632        |
|                        |
|                        |
|                        |
| C                      |
| N                    C |
| 1      315-6232      N |
|                      3 |
|                        |
|                        |
|      CN4         CN2   |
|------------------------|
Notes:
      K4S641632 - Samsung K4S641632 64Mbit SDRAM (1M x 16Bit x 4 Banks Synchronous DRAM)
      CN2       - AICA JTAG connector. Pinout:
                   VTref  1  8  GND
                   nTRST  2  9  GND
                   TDI    3 10  GND
                   TMS    4 11  GND
                   TCK    5 12  GND
                   TDO    6 13  RESET
                   VTref  7 14  GND

Network PCB
-----------

837-13404  171-7641B
|-----------------------------------|
| 40MHz      LATTICE     315-5917   |
|            PLSI2032               |
|            (315-5958A)    315-5917|
|   PAL                             |
|                          62256    |
|             315-5804     62256    |
|                          62256    |
|                          62256    |
|  68000          PAL               |
|  62256                            |
|  62256          PAL               |
|-----------------------------------|
Notes:
      62256 - 32k x8 SRAM


I/O PCB
-------

837-13551  171-7780D
|----------------------------|
|CN4  CN5     CN6     CN7 CN8|
| LED2  ADM485               |
|       TMP90PH44            |
|    14.745MHz          LED1 |
|CN1 CN2     CN3             |
|----------------------------|
Notes:
      TMP90PH44 - Toshiba TMP90PH44 TLCS-90 series microcontroller with
                  16k internal OTP PROM and 512 bytes internal RAM
            CN1 - 5 pin connector for +12v
            CN2 - 5 pin connector for +12v
            CN3 - 60 pin flat cable connector for digital controls & buttons
            CN4 - USB connector
            CN5 - USB connector
            CN6 - 26 pin flat cable connector for analog controls
            CN7 - 4 pin connector for +5v
            CN8 - 4 pin connector for +5v


Filter PCB
----------

839-1079  171-7789B SEGA 1998
|----------------------------------------------------|
|                CN6                CN11        CN9  |
|                               CN12   CN10          |
|                CN5  CN4   CN3           CN2   CN1  |
|                                                    |
|                                                    |
|  CNTX CNRX CN7       DN2               DN1         |
|----------------------------------------------------|
Notes:
      CN1  - 8 pin JVS power connector
      CN2  - 6 pin JVS power connector
      CN3  - 15 pin VGA connector 24k/31k
      CN4  - 15 pin VGA connector 24k/31k. Not populated
      CN5  - red/white RCA audio connectors
      CN6  - red/white RCA audio connectors
      CN7  - USB connector. Plugs into JVS I/O PCB using common A-B USB cable
      CN8  - Optical network connector, re-labelled CNTX. Sometimes not populated
      CN9  - Optical network connector, re-labelled CNTR. Sometimes not populated
      CN10 - Unknown. Not populated
      CN11 - RS422 connector. Not populated
      CN12 - Midi connector. Not populated
   DN1/DN2 - Connectors joining to main board. These connectors
             are on the other side of the PCB

*/

/*
    New hardware notes from Stefano Teso:

  - The master SH-4 port A: the bit layout for this thing is (MSB left)

    xxxx xxii iiee eexM

    x = unused
    i = IRQ causes (active low)
    e = Mainboard EEPROM (active low)
    M = connected to the slave SH-4 NMI pin; writing a specific bit
    pattern here (3 ones, 3 zeros, 7 ones) sends an NMI to the slave

    Bit 1 of the slave port A is used for master-slave communication as
    well, but I have yet to figure out how. It doesn't seem to be needed
    for emulation at this point though. Master-slave communication seems
    to work okay after emulating the M bit here.

  - There's an additional indirect-DMA-like device on the GPU at
    150000(0C,10,14). The table indicating where the data is, how long it
    is, and how to operate on it, is located at the bottom of GPU command
    RAM (14xxxxxx), defaulting at 143FC000 (whose bus address, specified
    by 1500000C is 483FC000.) It is used for (my guess) on-the-fly texture
    format conversion. Upon termination, it raises a GPU IRQ. It looks
    like airtrix first uploads the data to be converted somewhere using
    the memory controller's DMA, then instructs the GPU indirect DMA thing
    to read it and perform the conversion.

  - GPU IRQs: it looks like the GPU IRQs are as follows:

    15000088:

    bit 0: the GPU indirect-DMA-like device is done
    bit 1: vblank (or hblank)
    bit 2: the GPU at 15xxxxxx is done and needs feeding (means that
    commands are uploaded to CMDRAM and registers fiddled with, see bit 2
    below.)
    bit 8: any bit of 1A000018 is set

    1A000018:

    bit 1: vblank (or hblank)
    bit 2: GPU at 1Axxxxxx is done and needs feeding (same as bit 2
    above; note that both IRQs must be raised -- not necessarily at the
    same time -- for the whole "let's upload data to the GPU" routine to
    be performed.)

    My gut feeling tells me that there are two different GPUs: one
    processes the command stream and does the rendering on even frames,
    the other does the same for odd frames. Registers 1500007x may specify
    where the command stream start is (the initial GPU PC) and possibly
    the location of two distinct stack pointers (the GPU supports
    subroutines, after all.)

  - the serial device at 0080000(A|C) is not an EEPROM; it's likely some
    weird device that is used to query the hierarchy of the attached
    input/output devices. Probably the naomi has something similar, I
    haven't looked into it yet.

*/

#include "emu.h"
#include "cpu/sh/sh4.h"
#include "cpu/arm7/arm7.h"
#include "screen.h"
#include "speaker.h"
#include "315-6154.h"
#include "machine/eepromser.h"
#include "machine/x76f100.h"
#include "machine/nvram.h"
#include "machine/aicartc.h"
#include "machine/jvsdev.h"
#include "mie.h"
#include "jvs13551.h"
#include "sound/aica.h"
#include "m3comm.h"

namespace {

#define CPU_CLOCK (200000000)

class hikaru_state : public driver_device
{
public:
	hikaru_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_slave(*this, "slave")
		, m_315_6154(*this, "pci0:00.0")
		, s_315_6154(*this, "pci1:00.0")
		, m_mie(*this, "mie")
		, m_eeprom(*this, "main_eeprom")
		, m_rombd_eeprom(*this, "rombd_eeprom")
		, m_main_ram(*this, "main_ram")
		, m_slave_ram(*this, "slave_ram")
		, m_arm0(*this, "arm0")
		, m_aica0(*this, "aica0")
		, m_aica0_ram(*this, "aica0_ram")
	{ }

	void hikaru(machine_config &config);

private:
	virtual void video_start() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void sh4_map_main(address_map &map) ATTR_COLD;
	void sh4_io_main(address_map& map) ATTR_COLD;
	void sh4_map_slave(address_map &map) ATTR_COLD;
	void sh4_io_slave(address_map& map) ATTR_COLD;
	void pci_map(address_map& map) ATTR_COLD;
	void aica0_map(address_map& map) ATTR_COLD;
	void arm0_map(address_map& map) ATTR_COLD;

	uint32_t screen_update_hikaru(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<sh7091_device> m_maincpu;
	required_device<sh7091_device> m_slave;
	required_device<sega_315_6154_device> m_315_6154;
	required_device<sega_315_6154_device> s_315_6154;
	required_device<mie_device> m_mie;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<x76f100_device> m_rombd_eeprom;
	required_shared_ptr<uint64_t> m_main_ram;
	required_shared_ptr<uint64_t> m_slave_ram;
	required_device<cpu_device> m_arm0;
	required_device<aica_device> m_aica0;
	required_shared_ptr<uint16_t> m_aica0_ram;

	address_space* space_main_sh4;
	address_space* space_slave_sh4;
	address_space* space_6154;

	uint16_t aica0ram_r(offs_t offset);
	void aica0ram_w(offs_t offset, uint16_t data, uint16_t mem_mask);
	uint32_t arm0_aica_r(offs_t offset);
	void arm0_aica_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t aica0_reg_r(offs_t offset, uint32_t mem_mask);
	void aica0_reg_w(offs_t offset, uint32_t data, uint32_t mem_mask);
	void arm0_irq(int state);
	u8 m_arm0rst;

	uint64_t main_io_r();
	uint64_t slave_io_r();
	void main_io_w(uint64_t data);
	void slave_io_w(uint64_t data);

	void irq_update();
	//void gpu_irq(int state);
	void szk_irq(int state);

	u32 m_irq_pending;
	u32 m_irq_mask;
public:
	void init_hikaru();
	int mie_b_r();
	void mie_c_w(int state);
	void mie_f_w(int state);
};

void hikaru_state::video_start()
{
}

void hikaru_state::machine_start()
{
	space_main_sh4 = &m_maincpu->space(AS_PROGRAM);
	space_slave_sh4 = &m_slave->space(AS_PROGRAM);
	space_6154 = &m_315_6154->space(sega_315_6154_device::AS_PCI_MEMORY);
}

void hikaru_state::machine_reset()
{
	m_arm0rst = 1;
	m_arm0->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

uint32_t hikaru_state::screen_update_hikaru(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

// ugly trampolines, AICA needs to be devicefied
uint32_t hikaru_state::aica0_reg_r(offs_t offset, uint32_t mem_mask)
{
	if (offset == 0x2c00 / 4)
		return m_arm0rst;
	return m_aica0->read(offset * 2);
}
void hikaru_state::aica0_reg_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	if (offset == (0x2c00 / 4))
	{
		if (ACCESSING_BITS_0_7)
		{
			m_arm0rst = data & 1;
			m_arm0->set_input_line(INPUT_LINE_RESET, m_arm0rst ? ASSERT_LINE : CLEAR_LINE);
		}
	}
	m_aica0->write(offset * 2, data, 0xffff);
}
uint32_t hikaru_state::arm0_aica_r(offs_t offset)
{
	return m_aica0->read(offset * 2) & 0xffff;
}
void hikaru_state::arm0_aica_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_aica0->write(offset * 2, data, mem_mask & 0xffff);
}
uint16_t hikaru_state::aica0ram_r(offs_t offset)
{
	return m_aica0_ram[offset];
}
void hikaru_state::aica0ram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_aica0_ram[offset]);
}
void hikaru_state::arm0_irq(int state)
{
	m_arm0->set_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, state ? ASSERT_LINE : CLEAR_LINE);
}
void hikaru_state::arm0_map(address_map& map)
{
	map.unmap_value_high();
	map(0x00000000, 0x007fffff).rw(FUNC(hikaru_state::aica0ram_r), FUNC(hikaru_state::aica0ram_w));
	map(0x00800000, 0x00807fff).rw(FUNC(hikaru_state::arm0_aica_r), FUNC(hikaru_state::arm0_aica_w));
}
void hikaru_state::aica0_map(address_map& map)
{
	map.unmap_value_high();
	map(0x000000, 0x7fffff).ram().share("aica0_ram");
}
//

// main SH4 port A
// 0 - /IRL3 Slave SH4
// 1 - LED5 (active high)
// 2 - 93C46 DI / 76F100 SCL
// 3 - 93C46 SK / 76F100 SDA
// 4 - 93C46 CS / 76F100 RST
// 5 - 93C46 DO / 76F100 CS
// 6 - /INT0 ANT_INT
// 7 - /INT1 SZK_INT (AICA)
// 8 - /INT2 CN4_INT
// 9 - /INT4 CN3_INT1

uint64_t hikaru_state::main_io_r()
{
	return (m_eeprom->do_read() << 5) | (!BIT(m_irq_pending, 0) << 6) | (!BIT(m_irq_pending, 4) << 7) | (!BIT(m_irq_pending, 5) << 8) | (!BIT(m_irq_pending, 7) << 9);
}

void hikaru_state::main_io_w(uint64_t data)
{
	m_slave->set_input_line(SH4_IRL3, BIT(data, 0) ? CLEAR_LINE : ASSERT_LINE);

	m_eeprom->di_write(BIT(data,2));
	m_eeprom->cs_write(BIT(data, 4));
	m_eeprom->clk_write(BIT(data, 3));
}

// slave SH4 port A
// 0 - /IRL3 Main SH4
// 1 - LED6 (active high)
// 6-9 - DIPSW 4

uint64_t hikaru_state::slave_io_r()
{
	return ioport("DIPSW")->read() << 6;
}

void hikaru_state::slave_io_w(uint64_t data)
{
	m_maincpu->set_input_line(SH4_IRL3, BIT(data, 0) ? CLEAR_LINE : ASSERT_LINE);
}

void hikaru_state::irq_update()
{
	m_maincpu->set_input_line(SH4_IRL2, (m_irq_pending & m_irq_mask) ? ASSERT_LINE : CLEAR_LINE);
}

#if 0
void hikaru_state::gpu_irq(int state)
{
	if (state)
	{
		m_irq_pending |= 1;
		m_slave->set_input_line(SH4_IRL2, ASSERT_LINE);
	}
	else {
		m_irq_pending &= ~1;
		m_slave->set_input_line(SH4_IRL2, CLEAR_LINE);
	}
	irq_update();
}
#endif

void hikaru_state::szk_irq(int state)
{
	if (state)
		m_irq_pending |= 0x10;
	else
		m_irq_pending &= ~0x10;

	irq_update();
}

int hikaru_state::mie_b_r()
{
	return m_irq_pending;
}

void hikaru_state::mie_c_w(int state)
{
	m_irq_mask = state;
	irq_update();
}

void hikaru_state::mie_f_w(int state)
{
	m_irq_pending &= ~0xe;
	m_irq_pending |= state << 1;
	irq_update();
}

/* MIE GPIO ports:
    A not populated DIPSW 6
    B pending IRQs (active high)
      0 - INT0 ANT_INT
      1 - PortF 4
      2 - PortF 5
      3 - PortF 6
      4 - INT1 SZK_INT (AICA)
      5 - INT2 CN4_INT
      6 - INT3 CN3_INT0
      7 - INT4 CN3_INT1
    C INT_MASK, for main SH4 IRL2, 1=enabled,
      0 - INT0 (ANT_INT)
      1 - portF 4
      2 - portF 5
      3 - portF 6
      4 - INT1 SZK_INT (AICA)
      5 - INT2 CN4_INT
      6 - INT3 CN3_INT0
      7 - INT4 CN3_INT1
    D Reset (active low) and clock control
      0 - System Reset
      1 - GP_RES (all GPU ASICs reset)
      2 - Slave SH4 Reset
      3 - CN4_RES
      4 - PCI1_RES (Eurasia)
      5 - SND_RES
      6 - V S0, video clock PLL control
      7 - V S1, video clock PLL control
    E LEDs and GPU JTAG
      0-3 - LEDs 7-10
      4 - TRST
      5 - TDO
      6 - TMS
      7 - TCK
    F
      0 - 93C46 SK
      1 - 93C46 DI
      2 - 93C46 ORG
      3 - 93C46 CS
      4 - PortF 4 interrupt
      5 - PortF 5 interrupt
      6 - PortF 6 interrupt
      7 - GPU JTAG control, 0 - port E, 1 - external
    G
      0 - 93C46 DO
      1 -
      2 - /Test SW7
      3 - /Service SW8
      4 - FAN2 sensor
      5 - FAN1 sensor (not populated)
      6 - FAN0 sensor (not populated)
      7 - GPU JTAG chain TDO input
*/

static INPUT_PORTS_START( hikaru_common )
	PORT_START("MIE.0")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED ) // not populated DIPSW6, in theory may be used by some dev/debug code leftowers
	PORT_START("MIE.1")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_CUSTOM) PORT_READ_LINE_MEMBER(FUNC(hikaru_state::mie_b_r))
	PORT_START("MIE.2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_OUTPUT) PORT_WRITE_LINE_MEMBER(FUNC(hikaru_state::mie_c_w))

	PORT_START("MIE.5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("mie_eeprom", FUNC(eeprom_serial_93cxx_device::clk_write))
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("mie_eeprom", FUNC(eeprom_serial_93cxx_device::di_write))
//  PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("mie_eeprom", FUNC(eeprom_serial_93cxx_device::org_write)) // not implemented 8/16 bit mode switch
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("mie_eeprom", FUNC(eeprom_serial_93cxx_device::cs_write))
	PORT_BIT( 0x70, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_MEMBER(FUNC(hikaru_state::mie_f_w))

	PORT_START("MIE.6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("mie_eeprom", FUNC(eeprom_serial_93cxx_device::do_read))
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0xf2, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DIPSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW4:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( hikaru )
	PORT_INCLUDE( hikaru_common )

	PORT_START("TILT")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x7f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_START1 )
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
	PORT_BIT( 0x400f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("P2")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_START2 )
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
	PORT_BIT( 0x400f, IP_ACTIVE_HIGH, IPT_UNUSED )

	/* Dummy high bytes so we can easily get the analog ch # */
	PORT_START("A0")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A1")
	PORT_BIT( 0x01ff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A2")
	PORT_BIT( 0x02ff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A3")
	PORT_BIT( 0x03ff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A4")
	PORT_BIT( 0x04ff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A5")
	PORT_BIT( 0x05ff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A6")
	PORT_BIT( 0x06ff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("A7")
	PORT_BIT( 0x07ff, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

void hikaru_state::pci_map(address_map& map)
{
	// master bridge "Local Bus" area
//  map(0x00000000, 0x001fffff).rom();   // boot ROM
	map(0x00400000, 0x007fffff).mirror(0x80000000).noprw(); // r/w trigger "OKBCS" line to PLD between Antarctic and Africa, purpose unknown
	map(0x00800000, 0x0081ffff).mirror(0x80000000).m(m_mie, FUNC(mie_device::mie_port)).umask16(0x00ff);
	map(0x00820000, 0x0083ffff).mirror(0x80000000).rw(m_mie, FUNC(mie_device::mem_r), FUNC(mie_device::mem_w)).umask16(0x00ff);
	map(0x00c00000, 0x00c0ffff).mirror(0x80000000).ram().share("^nvram");
	map(0x02000000, 0x02ffffff).mirror(0x80000000).lrw32(
		NAME([this](offs_t offset) { return (!m_mie->busak_r() << 8) | 0x600; }),
		NAME([this](offs_t offset, u32 data) { m_mie->busrq_w(BIT(data, 8)); })
	);
//  map(0x0a000000, 0x0a0fffff).rw();    // optional ROMBD-specific security
	map(0x0c700000, 0x0c707fff).rw(tag(), FUNC(hikaru_state::aica0_reg_r), FUNC(hikaru_state::aica0_reg_w));
	map(0x0c710000, 0x0c71000f).rw("^aicartc0", FUNC(aicartc_device::read), FUNC(aicartc_device::write)).umask32(0x0000ffff);
	map(0x0c800000, 0x0cffffff).rw(tag(), FUNC(hikaru_state::aica0ram_r), FUNC(hikaru_state::aica0ram_w));
//  map(0x0d000000, 0x0dffffff).rw();    // optional AICA + RAM
//  map(0x0e000000, 0x0effffff).rw();    // optional Communication Board
	map(0x10000000, 0x13ffffff).mirror(0x80000000).rom().region("^user1", 0);
	map(0x14000000, 0x14ffffff).lrw32(
		NAME([this](offs_t offset) { return m_rombd_eeprom->read_sda(); }),
		NAME([this](offs_t offset, u32 data) {
			m_rombd_eeprom->write_cs(!BIT(data, 2));
			m_rombd_eeprom->write_rst(!BIT(data, 3));
			m_rombd_eeprom->write_scl(BIT(data, 1));
			m_rombd_eeprom->write_sda(BIT(data, 0));
			})
	);
	map(0x18000000, 0x1fffffff).nopr(); // suppress log, boot looking for ROM board 1st in this area
	map(0x20000000, 0x2fffffff).mirror(0x80000000).rom().region("^user2", 0);

// bridge mapped RAMs
	map(0x40000000, 0x41ffffff).lrw32(
		NAME([this](offs_t offset) { return space_slave_sh4->read_dword(0x0c000000 + (offset << 2)); }),
		NAME([this](offs_t offset, uint32_t data, uint32_t mem_mask) { space_slave_sh4->write_dword(0x0c000000 + (offset << 2), data, mem_mask); })
	);
	map(0x48000000, 0x483fffff).ram().share("^sharedram"); // slave bridge local RAM
	map(0x70000000, 0x71ffffff).lrw32(
		NAME([this](offs_t offset) { return space_main_sh4->read_dword(0x0c000000 + (offset << 2)); }),
		NAME([this](offs_t offset, uint32_t data, uint32_t mem_mask) { space_main_sh4->write_dword(0x0c000000 + (offset << 2), data, mem_mask); })
	);
//  78000000 master bridge local RAM, not present in Hikaru

// PCI

//  Antarctic ASIC 17C3:11DB (315-6083) 3D GPU Command Processor (CP), connected to slave bridge PCI port B (66MHz)
//  BAR0
//  map(0x00000000, 0x000000ff).rw();  // control/status registers, note: not clear how exactly it works as it clashing with boot ROM area
//  BAR1
//  map(0x04000000, 0x043fffff).ram(); // texture bank 0, IC41 Australia ASIC internal
//  map(0x06000000, 0x063fffff).ram(); // texture bank 1, IC42 Australia ASIC internal

//  Eurasia ASIC 17C7:11DB (315-6087) 2D GPU, connected to master bridge PCI port C (33MHz)
//  BAR0
//  map(0xf2000000, 0xf2000023).rw();  // control, interrupts, status registers
//  map(0xf2000080, 0xf20000d3).rw();  // CRTC config
//  map(0xf2000100, 0xf2000103).rw();  // display 0/1 enable
//  map(0xf2000180, 0xf20001bf).rw();  // display 0 layers registers
//  map(0xf2000200, 0xf200023f).rw();  // display 1 layers registers
//  BAR1
//  map(0xf2040000, 0xf204000f).rw();  // blitter/DMA registers
//  BAR2
//  map(0xf2080000, 0xf208000?);       // unused/unknown
//  BAR3
//  map(0xf3000000, 0xf37fffff).ram(); // video RAM (frame buffers and layers bitmaps)
}

void hikaru_state::sh4_io_main(address_map& map)
{
	map(0x00, 0x0f).rw(FUNC(hikaru_state::main_io_r), FUNC(hikaru_state::main_io_w));
}
void hikaru_state::sh4_io_slave(address_map& map)
{
	map(0x00, 0x0f).rw(FUNC(hikaru_state::slave_io_r), FUNC(hikaru_state::slave_io_w));
}

void hikaru_state::sh4_map_main(address_map &map)
{
	map(0x00000000, 0x03ffffff).rw(m_315_6154, FUNC(sega_315_6154_device::aperture_r<2>), FUNC(sega_315_6154_device::aperture_w<2>));
	map(0x00000000, 0x001fffff).rom().region("maincpu", 0);
	map(0x04000000, 0x040000ff).rw(m_315_6154, FUNC(sega_315_6154_device::registers_r), FUNC(sega_315_6154_device::registers_w));
	map(0x0c000000, 0x0dffffff).ram().share("main_ram");
	map(0x14000000, 0x17ffffff).rw(m_315_6154, FUNC(sega_315_6154_device::aperture_r<0>), FUNC(sega_315_6154_device::aperture_w<0>));
	map(0x18000000, 0x1bffffff).rw(m_315_6154, FUNC(sega_315_6154_device::aperture_r<1>), FUNC(sega_315_6154_device::aperture_w<1>));
}

void hikaru_state::sh4_map_slave(address_map &map)
{
	map(0x00000000, 0x03ffffff).rw(s_315_6154, FUNC(sega_315_6154_device::aperture_r<2>), FUNC(sega_315_6154_device::aperture_w<2>));
	map(0x00000000, 0x001fffff).rom().region("maincpu", 0);
	map(0x04000000, 0x040000ff).rw(s_315_6154, FUNC(sega_315_6154_device::registers_r), FUNC(sega_315_6154_device::registers_w));
	map(0x0c000000, 0x0dffffff).ram().share("slave_ram");
	map(0x10000000, 0x103fffff).lrw64(
		NAME([this](offs_t offset) { return space_6154->read_qword(0x48000000 + (offset << 3)); }),
		NAME([this](offs_t offset, uint64_t data, uint64_t mem_mask) { space_6154->write_qword(0x48000000 + (offset << 3), data, mem_mask); })
	);
	map(0x14000000, 0x17ffffff).rw(s_315_6154, FUNC(sega_315_6154_device::aperture_r<0>), FUNC(sega_315_6154_device::aperture_w<0>));
	map(0x18000000, 0x1bffffff).rw(s_315_6154, FUNC(sega_315_6154_device::aperture_r<1>), FUNC(sega_315_6154_device::aperture_w<1>));
}


void hikaru_state::hikaru(machine_config &config)
{
	/* basic machine hardware */
	SH7091(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_md(0, 1);
	m_maincpu->set_md(1, 0);
	m_maincpu->set_md(2, 1);
	m_maincpu->set_md(3, 0);
	m_maincpu->set_md(4, 0);
	m_maincpu->set_md(5, 1);
	m_maincpu->set_md(6, 0);
	m_maincpu->set_md(7, 1);
	m_maincpu->set_md(8, 0);
	m_maincpu->set_sh4_clock(CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &hikaru_state::sh4_map_main);
	m_maincpu->set_addrmap(AS_IO, &hikaru_state::sh4_io_main);
//  m_maincpu->set_force_no_drc(true);
//  m_maincpu->set_vblank_int("screen", FUNC(hikaru_state::vblank));

	SH7091(config, m_slave, CPU_CLOCK);
	m_slave->set_md(0, 1);
	m_slave->set_md(1, 0);
	m_slave->set_md(2, 1);
	m_slave->set_md(3, 0);
	m_slave->set_md(4, 0);
	m_slave->set_md(5, 1);
	m_slave->set_md(6, 0);
	m_slave->set_md(7, 1);
	m_slave->set_md(8, 0);
	m_slave->set_sh4_clock(CPU_CLOCK);
	m_slave->set_addrmap(AS_PROGRAM, &hikaru_state::sh4_map_slave);
	m_slave->set_addrmap(AS_IO, &hikaru_state::sh4_io_slave);
//  m_slave->set_force_no_drc(true);

	PCI_ROOT(config, "pci0", 0);
	SEGA315_6154(config, m_315_6154, 0);
	m_315_6154->set_addrmap(sega_315_6154_device::AS_PCI_MEMORY, &hikaru_state::pci_map);
	m_315_6154->set_mode(sega_315_6154_device::MODE_MASTER);
	PCI_ROOT(config, "pci1", 0);
	SEGA315_6154(config, s_315_6154, 0);
	s_315_6154->set_addrmap(sega_315_6154_device::AS_PCI_MEMORY, &hikaru_state::pci_map);
	s_315_6154->set_mode(sega_315_6154_device::MODE_SLAVE);

	MAPLE_DC(config, "maple", 0, m_maincpu); // TODO: get rid of this, there is no any Maple-host in Hikaru

	MIE(config, m_mie, 16000000, "maple", 0, "jvs");
	m_mie->set_gpio_name<0>("MIE.0");
	m_mie->set_gpio_name<1>("MIE.1");
	m_mie->set_gpio_name<2>("MIE.2");
	m_mie->set_gpio_name<5>("MIE.5");
	m_mie->set_gpio_name<6>("MIE.6");
	MIE_JVS(config, "jvs", 16000000);

	sega_837_13551_device& sega837(SEGA_837_13551(config, "837_13551", 0, "jvs"));
	sega837.set_port_tags("TILT", "P1", "P2", "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "OUTPUT");

	EEPROM_93C46_8BIT(config, m_eeprom);
	EEPROM_93C46_8BIT(config, "mie_eeprom");
	X76F100(config, m_rombd_eeprom);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(hikaru_state::screen_update_hikaru));

	SPEAKER(config, "speaker", 2).front();

	ARM7(config, m_arm0, ((XTAL(33'868'800) * 2) / 3) / 8);
	m_arm0->set_addrmap(AS_PROGRAM, &hikaru_state::arm0_map);

	AICA(config, m_aica0, (XTAL(33'868'800) * 2) / 3);
	m_aica0->irq().set(FUNC(hikaru_state::arm0_irq));
	m_aica0->main_irq().set(FUNC(hikaru_state::szk_irq));
	m_aica0->set_addrmap(0, &hikaru_state::aica0_map);
	m_aica0->add_route(0, "speaker", 1.0, 0);
	m_aica0->add_route(1, "speaker", 1.0, 1);

	AICARTC(config, "aicartc0", XTAL(32'768));
}

void hikaru_state::init_hikaru()
{
	m_maincpu->sh2drc_set_options(SH2DRC_STRICT_VERIFY | SH2DRC_STRICT_PCREL);
	m_maincpu->sh2drc_add_fastram(0x0c000000, 0x0dffffff, false, m_main_ram);
	m_slave->sh2drc_set_options(SH2DRC_STRICT_VERIFY | SH2DRC_STRICT_PCREL);
	m_slave->sh2drc_add_fastram(0x0c000000, 0x0dffffff, false, m_slave_ram);
}


#define ROM_LOAD_BIOS(bios,name,offset,length,hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_BIOS(bios))


#define HIKARU_BIOS \
	ROM_REGION( 0x200000, "maincpu", 0) \
	ROM_SYSTEM_BIOS( 0, "bios0", "epr23400a" ) \
	ROM_LOAD_BIOS( 0, "epr-23400a.ic94",  0x000000, 0x200000, CRC(2aa906a7) SHA1(098c9909b123ed6c338ac874f2ee90e3b2da4c02) ) \
	ROM_SYSTEM_BIOS( 1, "bios1", "epr23400" ) \
	ROM_LOAD_BIOS( 1, "epr-23400.ic94",   0x000000, 0x200000, CRC(3d557104) SHA1(d39879f5a1acbd54ad8ee4fbd412f870c9ff4aa5) ) \
	ROM_SYSTEM_BIOS( 2, "bios2", "epr21904" ) \
	ROM_LOAD_BIOS( 1, "epr-21904.ic94",   0x000000, 0x200000, CRC(d96298b6) SHA1(d10d837bc7d68eb7125c34beffe21a91305627b0) ) \
	ROM_SYSTEM_BIOS( 3, "bios3", "Development / prototype" ) \
	ROM_LOAD_BIOS( 1, "prot_bot.ic94",    0x000000, 0x200000, CRC(7cbf2fb6) SHA1(7384e3c9314add7d61f93c9edd9fb7788d08f423) )
// bios 0 is SAMURAI boot rom 0.96 / 2000/8/10
// bios 1 is SAMURAI boot rom 0.92 / 1999/7/22
// bios 2 is SAMURAI boot rom 0.84 / 1999/07/22
// bios 3 is SAMURAI boot rom 0.74 / 1999/5/01 Development version, have options to change country, SCSI ID, boot into debugger mode (will wait for commands from host via SCSI).

#define HIKARU_EEPROM \
	ROM_REGION( 0x80, "main_eeprom", 0) \
	ROM_LOAD( "93c46.ic115", 0, 0x80, CRC(737dc714) SHA1(852a90319ac2e787b94a4fc769424b79afbaaf9d) ) \
	ROM_REGION( 0x84, "rombd_eeprom", 0) \
	ROM_LOAD( "x76f100.ic85", 0, 0x84, BAD_DUMP CRC(194c6c18) SHA1(6c2de7dae32bef855d9bc556568c3a170f54caa5) )

ROM_START( hikaru )
	HIKARU_BIOS
	HIKARU_EEPROM

	ROM_REGION32_LE( 0x4000000, "user1", ROMREGION_ERASE00)
	ROM_REGION32_LE( 0x10000000, "user2", ROMREGION_ERASE00)
ROM_END


ROM_START( airtrix )
	HIKARU_BIOS
	HIKARU_EEPROM

	ROM_REGION32_LE( 0x4000000, "user1", ROMREGION_ERASE00)
	ROM_LOAD32_WORD( "epr-23601a.ic29", 0x0000000, 0x0400000, CRC(cd3ccc05) SHA1(49de32d3588511f37486aff900773453739d706d) )
	ROM_LOAD32_WORD( "epr-23602a.ic30", 0x0000002, 0x0400000, CRC(24f1bca9) SHA1(719dc4e003c1d13fcbb39604c156c89042c47dfd) )
	/* ic31 unpopulated */
	/* ic32 unpopulated */
	/* ic33 unpopulated */
	/* ic34 unpopulated */
	/* ic35 unpopulated */
	/* ic36 unpopulated */

	/* ROM board using 128M TSOP48 mask ROMs */
	ROM_REGION32_LE( 0x10000000, "user2", ROMREGION_ERASE00)
	ROM_LOAD32_WORD( "mpr-23573.ic37" , 0x0000000, 0x1000000, CRC(e22a0734) SHA1(fc06d5972d285d09473874aaeb1efed2d19c8f36) )
	ROM_LOAD32_WORD( "mpr-23577.ic38" , 0x0000002, 0x1000000, CRC(d007680d) SHA1(a795057c40b1851adb0e19e5dfb39e16206215bf) )
	ROM_LOAD32_WORD( "mpr-23574.ic41" , 0x2000000, 0x1000000, CRC(a77034a5) SHA1(e6e8e2f747e7a972144436103741acfd7030fe84) )
	ROM_LOAD32_WORD( "mpr-23578.ic42" , 0x2000002, 0x1000000, CRC(db612dd6) SHA1(e6813a1e16099094d67347027e058be582750ad7) )
	ROM_LOAD32_WORD( "mpr-23575.ic45" , 0x4000000, 0x1000000, CRC(fe660f06) SHA1(73916f67d852df719fd65b1ed0f8b977c0c33390) )
	ROM_LOAD32_WORD( "mpr-23579.ic46" , 0x4000002, 0x1000000, CRC(55e656d2) SHA1(5d0b26807cf915ab0ae5cc3a7c9dd6bec43da7b2) )
	ROM_LOAD32_WORD( "mpr-23576.ic49" , 0x6000000, 0x1000000, CRC(c01e0329) SHA1(df1a3c83f338925d69912af56f675197e14e1793) )
	ROM_LOAD32_WORD( "mpr-23580.ic50" , 0x6000002, 0x1000000, CRC(d260f39c) SHA1(e5cdf399defaaa7dbcee62f7ab64b898c28d8f7d) )
	/* ic53 unpopulated */
	/* ic54 unpopulated */
	/* ic57 unpopulated */
	/* ic58 unpopulated */
	/* ic61 unpopulated */
	/* ic62 unpopulated */
	/* ic65 unpopulated */
	/* ic66 unpopulated */

	// 834-14149   2000     317-0294-COM   Hikaru
	ROM_PARAMETER( ":rom_board:segam2crypt:key", "291b02c7" )
ROM_END

ROM_START( airtrixo )
	HIKARU_BIOS
	HIKARU_EEPROM

	ROM_REGION32_LE(0x4000000, "user1", ROMREGION_ERASE00)
	ROM_LOAD32_WORD( "epr-23601.ic29", 0x0000000, 0x0400000, CRC(e0c642cb) SHA1(f04f8e13cc46d462c79ecebcded7dee9b3500bdc) )
	ROM_LOAD32_WORD( "epr-23602.ic30", 0x0000002, 0x0400000, CRC(fac11d21) SHA1(70b48a7e1ac4268fc09d96d6845c5a5099d4e301) )

	/* ROM board using 128M TSOP48 mask ROMs */
	ROM_REGION32_LE( 0x10000000, "user2", ROMREGION_ERASE00)
	ROM_LOAD32_WORD( "mpr-23573.ic37" , 0x0000000, 0x1000000, CRC(e22a0734) SHA1(fc06d5972d285d09473874aaeb1efed2d19c8f36) )
	ROM_LOAD32_WORD( "mpr-23577.ic38" , 0x0000002, 0x1000000, CRC(d007680d) SHA1(a795057c40b1851adb0e19e5dfb39e16206215bf) )
	ROM_LOAD32_WORD( "mpr-23574.ic41" , 0x2000000, 0x1000000, CRC(a77034a5) SHA1(e6e8e2f747e7a972144436103741acfd7030fe84) )
	ROM_LOAD32_WORD( "mpr-23578.ic42" , 0x2000002, 0x1000000, CRC(db612dd6) SHA1(e6813a1e16099094d67347027e058be582750ad7) )
	ROM_LOAD32_WORD( "mpr-23575.ic45" , 0x4000000, 0x1000000, CRC(fe660f06) SHA1(73916f67d852df719fd65b1ed0f8b977c0c33390) )
	ROM_LOAD32_WORD( "mpr-23579.ic46" , 0x4000002, 0x1000000, CRC(55e656d2) SHA1(5d0b26807cf915ab0ae5cc3a7c9dd6bec43da7b2) )
	ROM_LOAD32_WORD( "mpr-23576.ic49" , 0x6000000, 0x1000000, CRC(c01e0329) SHA1(df1a3c83f338925d69912af56f675197e14e1793) )
	ROM_LOAD32_WORD( "mpr-23580.ic50" , 0x6000002, 0x1000000, CRC(d260f39c) SHA1(e5cdf399defaaa7dbcee62f7ab64b898c28d8f7d) )

	// 834-14149   2000     317-0294-COM   Hikaru
	ROM_PARAMETER( ":rom_board:segam2crypt:key", "291b02c7" )
ROM_END


ROM_START( pharrier )
	HIKARU_BIOS
	HIKARU_EEPROM

	ROM_REGION32_LE( 0x4000000, "user1", ROMREGION_ERASE00)
	ROM_LOAD32_WORD("epr-23565a.ic29", 0x0000000, 0x0400000, CRC(ca9af8a7) SHA1(e7d6badc03ec5833ee89e49dd389ee19b45da29c) )
	ROM_LOAD32_WORD("epr-23566a.ic30", 0x0000002, 0x0400000, CRC(aad0057c) SHA1(c18c0f1797432c74dc21bcd806cb5760916e4936) )
	ROM_LOAD32_WORD("epr-23567.ic31",  0x0800000, 0x0400000, CRC(f0e3dcdc) SHA1(422978a13e39f439da54e43a65dcad1a5b1f2f27) )
	ROM_LOAD32_WORD("epr-23568.ic32",  0x0800002, 0x0400000, CRC(6eee734c) SHA1(0941761b1690ad4eeac0bf682459992c6f38a930) )
	ROM_LOAD32_WORD("epr-23569.ic33",  0x1000000, 0x0400000, CRC(867c7064) SHA1(5cf0d88a1c739ba69b33f1ba3a0e5544331f63f3) )
	ROM_LOAD32_WORD("epr-23570.ic34",  0x1000002, 0x0400000, CRC(556ff58b) SHA1(7eb527aee823d037d1045d850427efa42d5da787) )
	ROM_LOAD32_WORD("epr-23571.ic35",  0x1800000, 0x0400000, CRC(5a75fa92) SHA1(b5e0c8c995ecc954b74d5eb36f3ae2a732a5986b) )
	ROM_LOAD32_WORD("epr-23572.ic36",  0x1800002, 0x0400000, CRC(46054067) SHA1(449800bdc2c40c76aed9bc5e7e8831d8f03ef286) )

	/* ROM board using 128M TSOP48 mask ROMs */
	ROM_REGION32_LE( 0x10000000, "user2", ROMREGION_ERASE00)
	ROM_LOAD32_WORD( "mpr-23549.ic37", 0x0000000, 0x1000000, CRC(ed764200) SHA1(ad840a40347345f72a443f284b1bb0ae2b37f7ac) )
	ROM_LOAD32_WORD( "mpr-23553.ic38", 0x0000002, 0x1000000, CRC(5e70ae78) SHA1(2ae6bdb5aa1434bb60b2b9bca7af12d6476cd35f) )
	ROM_LOAD32_WORD( "mpr-23550.ic41", 0x2000000, 0x1000000, CRC(841b4d3b) SHA1(d442078b6b4926e6e32b911d88a4408d20a8f0df) )
	ROM_LOAD32_WORD( "mpr-23554.ic42", 0x2000002, 0x1000000, CRC(5cce99de) SHA1(c39330e4bcfb4cec8b0b59ab184fad5093188765) )
	ROM_LOAD32_WORD( "mpr-23551.ic45", 0x4000000, 0x1000000, CRC(71f61d04) SHA1(6f24f82ddc5aaf9bbb41b8baddbcb855f1d37a16) )
	ROM_LOAD32_WORD( "mpr-23555.ic46", 0x4000002, 0x1000000, CRC(582e5453) SHA1(cf9fb8b52a169446b98630d67cdce745de917edc) )
	ROM_LOAD32_WORD( "mpr-23552.ic49", 0x6000000, 0x1000000, CRC(32487181) SHA1(a885747428c280f77dd861bf802d953da133ef59) )
	ROM_LOAD32_WORD( "mpr-23556.ic50", 0x6000002, 0x1000000, CRC(45002955) SHA1(85a27c86692ca79fc4e51a64af63a5e970b86cfa) )
	ROM_LOAD32_WORD( "mpr-23557.ic53", 0x8000000, 0x1000000, CRC(c20dff1b) SHA1(d90d3d85f4fddf39c109502c8f9e9f25a7fc43d1) )
	ROM_LOAD32_WORD( "mpr-23561.ic54", 0x8000002, 0x1000000, CRC(01237844) SHA1(7a8c6bfdea1d4db5e9f6850fdf1a03d703df3958) )
	ROM_LOAD32_WORD( "mpr-23558.ic57", 0xa000000, 0x1000000, CRC(e93cc8d7) SHA1(05fc23b8382daaca7ccd1ca80e7c5e93cbf2b6b1) )
	ROM_LOAD32_WORD( "mpr-23562.ic58", 0xa000002, 0x1000000, CRC(85e0816c) SHA1(28106404d1eef4c85dd425d3535a53c5d71e47a0) )
	ROM_LOAD32_WORD( "mpr-23559.ic61", 0xc000000, 0x1000000, CRC(1a7f2ba0) SHA1(e2a20138f21297f5313f5368ef9992da8fa23937) )
	ROM_LOAD32_WORD( "mpr-23563.ic62", 0xc000002, 0x1000000, CRC(e3dc328b) SHA1(d04ccc4025442c98b96f84c1b300671f3687ec6c) )
	ROM_LOAD32_WORD( "mpr-23560.ic65", 0xe000000, 0x1000000, CRC(24bb7072) SHA1(dad5135c89d292e4a1f96bd0ad28be6a17154be0) )
	ROM_LOAD32_WORD( "mpr-23564.ic66", 0xe000002, 0x1000000, CRC(255724b6) SHA1(1b382fad165831de3f2e39352c031146759dfc69) )

	// 834-14144   2001     317-0297-COM   Hikaru
	ROM_PARAMETER( ":rom_board:segam2crypt:key", "2912c68a" )
ROM_END

ROM_START( swracer )
	HIKARU_BIOS
	HIKARU_EEPROM

	ROM_REGION32_LE( 0x4000000, "user1", ROMREGION_ERASE00)
	ROM_LOAD32_WORD("epr-23174.ic29", 0x0000000, 0x0400000, CRC(eae62b46) SHA1(f1458072b002d64bbb7c43c582e3191e8031e19a) )
	ROM_LOAD32_WORD("epr-23175.ic30", 0x0000002, 0x0400000, CRC(b92da060) SHA1(dd9ecbd0977aef7629441ff45f4ad807b2408603) )
	ROM_LOAD32_WORD("epr-23176.ic31", 0x0800000, 0x0400000, CRC(2f2824a7) SHA1(a375719e3cababab5b33d00d8696c7cd62c5af30) )
	ROM_LOAD32_WORD("epr-23177.ic32", 0x0800002, 0x0400000, CRC(7a5e3f0f) SHA1(e8ca00cfaaa9be4f9d269e4d8f6bcbbd7de8f6d6) )
	/* ic33 unpopulated */
	/* ic34 unpopulated */
	/* ic35 unpopulated */
	/* ic36 unpopulated */

	/* ROM board using 64M SOP44 mask ROM */
	ROM_REGION32_LE( 0x10000000, "user2", ROMREGION_ERASE00)
	ROM_LOAD32_WORD("mpr-23086.ic37" ,  0x0000000, 0x0800000, CRC(ef6f20f1) SHA1(11fb66bf71223b4c6650d3adaea21e8709b8d67b))
	ROM_LOAD32_WORD("mpr-23087.ic38" ,  0x0000002, 0x0800000, CRC(54389822) SHA1(6357f0aa77ef0a5a08a751e085fa026d26ba47d1))
	ROM_LOAD32_WORD("mpr-23088.ic39" ,  0x1000000, 0x0800000, CRC(9f1a382e) SHA1(b846c3a091d04e49cc1e731237c9326ccac39a64))
	ROM_LOAD32_WORD("mpr-23089.ic40" ,  0x1000002, 0x0800000, CRC(6aae64fc) SHA1(392b6fba25d20bb41fd72be3a3a9ce95b2374065))
	ROM_LOAD32_WORD("mpr-23090.ic41" ,  0x2000000, 0x0800000, CRC(ba857872) SHA1(c07ff7955d3d07f2a60d9761b4bd692c0a9c9353))
	ROM_LOAD32_WORD("mpr-23091.ic42" ,  0x2000002, 0x0800000, CRC(66a73e27) SHA1(c4e7d190a80499225a78b7f788c2abc7ec4ebdca))
	ROM_LOAD32_WORD("mpr-23092.ic43" ,  0x3000000, 0x0800000, CRC(4f20a0f5) SHA1(0580feba6a6dd01a21d09ec2503ccf77030f8d2a))
	ROM_LOAD32_WORD("mpr-23093.ic44" ,  0x3000002, 0x0800000, CRC(e74d7d64) SHA1(c28e44319bf08aedd9aed625a12834ec76f1e5e0))
	ROM_LOAD32_WORD("mpr-23094.ic45" ,  0x4000000, 0x0800000, CRC(90f04c14) SHA1(b55846ea1edd920fd527e3257b13fea8df1f713f))
	ROM_LOAD32_WORD("mpr-23095.ic46" ,  0x4000002, 0x0800000, CRC(cc67cb5b) SHA1(85e99ec22d1c65139685a94f1ba0c52a0eb33a2e))
	ROM_LOAD32_WORD("mpr-23096.ic47" ,  0x5000000, 0x0800000, CRC(799ab79e) SHA1(c0ac85ad7f4cf46ff162f1ec2e85a3f22817de5e))
	ROM_LOAD32_WORD("mpr-23097.ic48" ,  0x5000002, 0x0800000, CRC(f68439de) SHA1(475d0f22e78e3c86431b742e37cbfd764ca8acee))
	ROM_LOAD32_WORD("mpr-23098.ic49" ,  0x6000000, 0x0800000, CRC(a1e2009c) SHA1(c6a600d47fd2a96d28c637631862150e6f303c3d))
	ROM_LOAD32_WORD("mpr-23099.ic50" ,  0x6000002, 0x0800000, CRC(ce36f642) SHA1(6cb2e69095efc7969255ebc637e2597c56442751))
	ROM_LOAD32_WORD("mpr-23100.ic51" ,  0x7000000, 0x0800000, CRC(0f966653) SHA1(1544af662188ea734e0a2e559e05e5f782fb292d))
	ROM_LOAD32_WORD("mpr-23101.ic52" ,  0x7000002, 0x0800000, CRC(2640fbaa) SHA1(59e9bd143734c71968beb9953122680d3350e69c))
	ROM_LOAD32_WORD("mpr-23102.ic53s" , 0x8000000, 0x0800000, CRC(080c5bcb) SHA1(0cf54348420ae9866edd64422cb82464990f1f2f))
	ROM_LOAD32_WORD("mpr-23103.ic54s" , 0x8000002, 0x0800000, CRC(19c7758f) SHA1(fed7f45dd91e1cb6bba7d8e80ed17dca27d92e43))
	ROM_LOAD32_WORD("mpr-23104.ic55s" , 0x9000000, 0x0800000, CRC(4ca74216) SHA1(0e65971359ba0e2b4fc032a26d1c10d8efadc205))
	ROM_LOAD32_WORD("mpr-23105.ic56s" , 0x9000002, 0x0800000, CRC(e2dd35ba) SHA1(2213e3195a49532a177086de8134ce8b753fc7ce))
	ROM_LOAD32_WORD("mpr-23106.ic57s" , 0xa000000, 0x0800000, CRC(dd325515) SHA1(8144e1a87f7d72a18791d1d452123a91cfb354dd))
	ROM_LOAD32_WORD("mpr-23107.ic58s" , 0xa000002, 0x0800000, CRC(a527a22a) SHA1(54c105b21797c9b0a2a6b2c7091de726c49a55e8))
	ROM_LOAD32_WORD("mpr-23108.ic59s" , 0xb000000, 0x0800000, CRC(47817d9a) SHA1(d2c6f1b2e800448eaf694d550733bba2280b6746))
	ROM_LOAD32_WORD("mpr-23109.ic60s" , 0xb000002, 0x0800000, CRC(8c61dec4) SHA1(25a1a5b236b3aed013fc94bd9695906ae5d7f305))
	ROM_LOAD32_WORD("mpr-23110.ic61s" , 0xc000000, 0x0800000, CRC(4ddae9f1) SHA1(d1c5e3f18932af806f166779cf14909ab17d052c))
	ROM_LOAD32_WORD("mpr-23111.ic62s" , 0xc000002, 0x0800000, CRC(c404cb1c) SHA1(e14855ec8a5a5ba243a2339c571928fdcc187157))
	ROM_LOAD32_WORD("mpr-23112.ic63s" , 0xd000000, 0x0800000, CRC(d001fe59) SHA1(ab395d2933b5d691259221168dfaa063cf9a4d1c))
	ROM_LOAD32_WORD("mpr-23113.ic64s" , 0xd000002, 0x0800000, CRC(f241cfd5) SHA1(5b85e8b50559becff7a76565c95487825bbd9351))
	ROM_LOAD32_WORD("mpr-23114.ic65s" , 0xe000000, 0x0800000, CRC(80049d7c) SHA1(56aab53e9317b1b5d10bd2af78fd83e7422d8939))
	ROM_LOAD32_WORD("mpr-23115.ic66s" , 0xe000002, 0x0800000, CRC(4fc540fe) SHA1(df580421d856566e067c2b319c8ac4671629682f))
	ROM_LOAD32_WORD("mpr-23116.ic67s" , 0xf000000, 0x0800000, CRC(9f567fce) SHA1(c35bcf968f139557e50ceafa9c6bad4deb87154f))
	ROM_LOAD32_WORD("mpr-23117.ic68s" , 0xf000002, 0x0800000, CRC(9d4d3529) SHA1(66008445629681ebf2f26b3f181d8524a8576d2f))

	// 834-14002   2001     317-0277-COM   Hikaru
	ROM_PARAMETER( ":rom_board:segam2crypt:key", "2903dad5" )
ROM_END

ROM_START( braveff )
	HIKARU_BIOS
	HIKARU_EEPROM

	ROM_REGION32_LE( 0x4000000, "user1", ROMREGION_ERASE00)
	ROM_LOAD32_WORD( "epr-21994.ic29", 0x0000000, 0x200000, CRC(31b0a754) SHA1(b49c998a15fbc790b780ed6665a56681d4edd369) )
	ROM_LOAD32_WORD( "epr-21995.ic30", 0x0000002, 0x200000, CRC(bcccb56b) SHA1(6e7a69934e5b47495ae8e90c57759573bc519d24) )
	ROM_LOAD32_WORD( "epr-21996.ic31", 0x0800000, 0x200000, CRC(a8f88e17) SHA1(dbbd2a73335c740bcf2ff9680c575841af29b340) )
	ROM_LOAD32_WORD( "epr-21997.ic32", 0x0800002, 0x200000, CRC(36641a7f) SHA1(37931bde1ddebef61fa6d8caca3cb67328fd0b90) )
	ROM_LOAD32_WORD( "epr-21998.ic33", 0x1000000, 0x200000, CRC(bd1df696) SHA1(fd937894763fab5cb50f33c40f8047e0d3adc93b) )
	ROM_LOAD32_WORD( "epr-21999.ic34", 0x1000002, 0x200000, CRC(9425eee0) SHA1(0f6a23163022bbd7ec54dd638094f3e317a87919) )
	/* ic35 unpopulated */
	/* ic36 unpopulated */

	/* ROM board using 64M SOP44 mask ROM */
	ROM_REGION32_LE( 0x10000000, "user2", ROMREGION_ERASE00)
	ROM_LOAD32_WORD( "mpr-22000.ic37",  0x0000000, 0x800000, CRC(53d641d6) SHA1(f47d7c77d0e36c4ec3b7171fd7a017f9f58ca5a0) )
	ROM_LOAD32_WORD( "mpr-22001.ic38",  0x0000002, 0x800000, CRC(234bc48f) SHA1(177c46884de0ba4bac1f9b778f99c905410a9345) )
	ROM_LOAD32_WORD( "mpr-22002.ic39",  0x1000000, 0x800000, CRC(d8f3aa9e) SHA1(f73208034fdd51fed086e912cb8580d2270122b6) )
	ROM_LOAD32_WORD( "mpr-22003.ic40",  0x1000002, 0x800000, CRC(2560fe98) SHA1(9bb5ffb6212ec6aa3f92e437eb424141f3b15e43) )
	ROM_LOAD32_WORD( "mpr-22004.ic41",  0x2000000, 0x800000, CRC(4e24d71d) SHA1(503344dd8cdd8e65ec7c801b0efae83b3f1f9ae2) )
	ROM_LOAD32_WORD( "mpr-22005.ic42",  0x2000002, 0x800000, CRC(2b96c97f) SHA1(707070c85f4b044236694daa13970c241b242d4d) )
	ROM_LOAD32_WORD( "mpr-22006.ic43",  0x3000000, 0x800000, CRC(f793a3ba) SHA1(80acd1d4f71cafd7328ff9b9ce30e5169b8f4f8c) )
	ROM_LOAD32_WORD( "mpr-22007.ic44",  0x3000002, 0x800000, CRC(62616e31) SHA1(dbe0d4b8fc085ed97884c105fd527af5cd8fbe79) )
	ROM_LOAD32_WORD( "mpr-22008.ic45",  0x4000000, 0x800000, CRC(e6905de8) SHA1(6bb4e43b1394788add15f0b78ccd5ab14f86516c) )
	ROM_LOAD32_WORD( "mpr-22009.ic46",  0x4000002, 0x800000, CRC(c37dfa5c) SHA1(5a3a5f2eb5a13831e36ca215147ec3c9740c50fc) )
	ROM_LOAD32_WORD( "mpr-22010.ic47",  0x5000000, 0x800000, CRC(b570b46c) SHA1(6e512fd1a2c8835f6aee307865b42d57ddf90ef5) )
	ROM_LOAD32_WORD( "mpr-22011.ic48",  0x5000002, 0x800000, CRC(d1f5fb58) SHA1(08a1282e00bda52d8d938225c65f67d22abfea05) )
	ROM_LOAD32_WORD( "mpr-22012.ic49",  0x6000000, 0x800000, CRC(3ab79029) SHA1(d4708446ba700d5f7c89827c80177ad2d1c0b222) )
	ROM_LOAD32_WORD( "mpr-22013.ic50",  0x6000002, 0x800000, CRC(42d8d00b) SHA1(ddce3c95258d8cf51792f2115f89ca658ffe97b6) )
	ROM_LOAD32_WORD( "mpr-22014.ic51",  0x7000000, 0x800000, CRC(0f49c00f) SHA1(877c654268edc9526ae3e21e21e3ecca706f300b) )
	ROM_LOAD32_WORD( "mpr-22015.ic52",  0x7000002, 0x800000, CRC(d3696e61) SHA1(247161c99c7061b8f391543af1812764a82399cb) )
	ROM_LOAD32_WORD( "mpr-22016.ic53s", 0x8000000, 0x800000, CRC(c1015e00) SHA1(f2ce2009d4f4f0f3cbfcce7a36fab2c54e738b07) )
	ROM_LOAD32_WORD( "mpr-22017.ic54s", 0x8000002, 0x800000, CRC(222a7cb0) SHA1(9f98ae3f13f85fae4596b671ea508b07c2116ab6) )
	ROM_LOAD32_WORD( "mpr-22018.ic55s", 0x9000000, 0x800000, CRC(f160e115) SHA1(ecf7f9f58fce6bff220568972ba7763537c9d7d7) )
	ROM_LOAD32_WORD( "mpr-22019.ic56s", 0x9000002, 0x800000, CRC(468b2f10) SHA1(f3fc0af7d4dd3f30ba84e684f3d9c217730564bb) )
	ROM_LOAD32_WORD( "mpr-22020.ic57s", 0xa000000, 0x800000, CRC(0c018d8a) SHA1(0447d7ad64061cca4c1231733e660ba51de5a216) )
	ROM_LOAD32_WORD( "mpr-22021.ic58s", 0xa000002, 0x800000, CRC(43b08604) SHA1(681142d8b95b2f9664d70b23262a64938774d4e3) )
	ROM_LOAD32_WORD( "mpr-22022.ic59s", 0xb000000, 0x800000, CRC(abd3d888) SHA1(9654c3a38feab46b4983a602831fb29cccdd0526) )
	ROM_LOAD32_WORD( "mpr-22023.ic60s", 0xb000002, 0x800000, CRC(07f00869) SHA1(92282d09d72d3e65a91128e06bb0d4426bb90be5) )

	ROM_PARAMETER( ":rom_board:segam2crypt:key", "-1" ) // 315-5881 not populated
ROM_END

ROM_START( sgnascar )
	HIKARU_BIOS
	HIKARU_EEPROM

	ROM_REGION32_LE( 0x4000000, "user1", ROMREGION_ERASE00)
	ROM_LOAD32_WORD( "epr-23485a.ic35", 0x000000, 0x400000, CRC(1072f531) SHA1(ca07a8bfb7247e4aec57e18cb091d24dcef666c1) )
	ROM_LOAD32_WORD( "epr-23486a.ic36", 0x000002, 0x400000, CRC(02d4aab6) SHA1(b1b0e07dc71dc124177e27dfd8b459444e8ae4d3) )

	/* ROM board using 128M TSOP48 mask ROMs */
	ROM_REGION32_LE( 0x10000000, "user2", ROMREGION_ERASE00)
	ROM_LOAD32_WORD( "mpr-23469.ic19", 0x0000000, 0x1000000, CRC(89cbad8d) SHA1(e4f103b96a3a842a90182172ddcf3bc5dfe6cca8) )
	ROM_LOAD32_WORD( "mpr-23473.ic20", 0x0000002, 0x1000000, CRC(977b87d6) SHA1(079eeebc6f9c60d0a016a46386bbe846d8a354da) )
	ROM_LOAD32_WORD( "mpr-23470.ic21", 0x2000000, 0x1000000, CRC(faf4940f) SHA1(72fee9ea5b78da260ed99ebe80ca6300f62cdbd7) )
	ROM_LOAD32_WORD( "mpr-23474.ic22", 0x2000002, 0x1000000, CRC(faf69ac5) SHA1(875c748151bf0e9cd73d86384665414b2f7b6f5a) )
	ROM_LOAD32_WORD( "mpr-23471.ic23", 0x4000000, 0x1000000, CRC(a3aad8ac) SHA1(afc8f3d1546e50afab4f540d59c87fe27cfb2cdd) )
	ROM_LOAD32_WORD( "mpr-23475.ic24", 0x4000002, 0x1000000, CRC(5f51597c) SHA1(02c0a5d463714082b7ebb2bec4d0f88aff186f82) )
	ROM_LOAD32_WORD( "mpr-23472.ic25", 0x6000000, 0x1000000, CRC(2495f678) SHA1(94b3160aabaea0596855c38ab1b63b16b20f2bae) )
	ROM_LOAD32_WORD( "mpr-23476.ic26", 0x6000002, 0x1000000, CRC(927cf31c) SHA1(7cab22a4113d92080a52e1d235bf075ce95f985f) )
	ROM_LOAD32_WORD( "mpr-23477.ic27", 0x8000000, 0x1000000, CRC(b4b7c477) SHA1(bcbfe081d509f0b87c6685b9b6617ae146987fe7) )
	ROM_LOAD32_WORD( "mpr-23481.ic28", 0x8000002, 0x1000000, CRC(27b8eb7d) SHA1(087b1ed13a3e2a0dbda82c454243214784429d24) )
	ROM_LOAD32_WORD( "mpr-23478.ic29", 0xa000000, 0x1000000, CRC(1fac431c) SHA1(2e3903c8cfd55d414555a1d23ba3a97c335991b3) )
	ROM_LOAD32_WORD( "mpr-23482.ic30", 0xa000002, 0x1000000, CRC(2e9a0420) SHA1(376d5f0b8274d741a702dc08da50ea5679991740) )
	ROM_LOAD32_WORD( "mpr-23479.ic31", 0xc000000, 0x1000000, CRC(9704e393) SHA1(0cb1403f4a268def3ce88db42e55d89ca913e2a0) )
	ROM_LOAD32_WORD( "mpr-23483.ic32", 0xc000002, 0x1000000, CRC(c37adebe) SHA1(e84f6d2cc364c743f7f3b73d8c8d0271952bb093) )
	ROM_LOAD32_WORD( "mpr-23480.ic33", 0xe000000, 0x1000000, CRC(f517b8b3) SHA1(c04740adb612473c4c9f8186e7e93d2f73d1bb1a) )
	ROM_LOAD32_WORD( "mpr-23484.ic34", 0xe000002, 0x1000000, CRC(2ebe1aa1) SHA1(16b39f7422da1a334dde27169c2949e1d95bddb3) )

	// 834-14125 317-0283-COM Actel A54SX32
	// ID 0x4252
	ROM_PARAMETER( ":rom_board:key", "56dedf33" )
ROM_END

ROM_START( sgnascaro )
	HIKARU_BIOS
	HIKARU_EEPROM

	ROM_REGION32_LE( 0x4000000, "user1", ROMREGION_ERASE00)
	ROM_LOAD32_WORD( "epr-23485.ic35", 0x000000, 0x400000, CRC(13b44fbf) SHA1(73416fa7b671ec5c96f0b084a427ff701bf6c399) )
	ROM_LOAD32_WORD( "epr-23486.ic36", 0x000002, 0x400000, CRC(ac3acd19) SHA1(1ec96be0bfceb2f1f808d78b07425d32056fbde0) )

	/* ROM board using 128M TSOP48 mask ROMs */
	ROM_REGION32_LE( 0x10000000, "user2", ROMREGION_ERASE00)
	ROM_LOAD32_WORD( "mpr-23469.ic19", 0x0000000, 0x1000000, CRC(89cbad8d) SHA1(e4f103b96a3a842a90182172ddcf3bc5dfe6cca8) )
	ROM_LOAD32_WORD( "mpr-23473.ic20", 0x0000002, 0x1000000, CRC(977b87d6) SHA1(079eeebc6f9c60d0a016a46386bbe846d8a354da) )
	ROM_LOAD32_WORD( "mpr-23470.ic21", 0x2000000, 0x1000000, CRC(faf4940f) SHA1(72fee9ea5b78da260ed99ebe80ca6300f62cdbd7) )
	ROM_LOAD32_WORD( "mpr-23474.ic22", 0x2000002, 0x1000000, CRC(faf69ac5) SHA1(875c748151bf0e9cd73d86384665414b2f7b6f5a) )
	ROM_LOAD32_WORD( "mpr-23471.ic23", 0x4000000, 0x1000000, CRC(a3aad8ac) SHA1(afc8f3d1546e50afab4f540d59c87fe27cfb2cdd) )
	ROM_LOAD32_WORD( "mpr-23475.ic24", 0x4000002, 0x1000000, CRC(5f51597c) SHA1(02c0a5d463714082b7ebb2bec4d0f88aff186f82) )
	ROM_LOAD32_WORD( "mpr-23472.ic25", 0x6000000, 0x1000000, CRC(2495f678) SHA1(94b3160aabaea0596855c38ab1b63b16b20f2bae) )
	ROM_LOAD32_WORD( "mpr-23476.ic26", 0x6000002, 0x1000000, CRC(927cf31c) SHA1(7cab22a4113d92080a52e1d235bf075ce95f985f) )
	ROM_LOAD32_WORD( "mpr-23477.ic27", 0x8000000, 0x1000000, CRC(b4b7c477) SHA1(bcbfe081d509f0b87c6685b9b6617ae146987fe7) )
	ROM_LOAD32_WORD( "mpr-23481.ic28", 0x8000002, 0x1000000, CRC(27b8eb7d) SHA1(087b1ed13a3e2a0dbda82c454243214784429d24) )
	ROM_LOAD32_WORD( "mpr-23478.ic29", 0xa000000, 0x1000000, CRC(1fac431c) SHA1(2e3903c8cfd55d414555a1d23ba3a97c335991b3) )
	ROM_LOAD32_WORD( "mpr-23482.ic30", 0xa000002, 0x1000000, CRC(2e9a0420) SHA1(376d5f0b8274d741a702dc08da50ea5679991740) )
	ROM_LOAD32_WORD( "mpr-23479.ic31", 0xc000000, 0x1000000, CRC(9704e393) SHA1(0cb1403f4a268def3ce88db42e55d89ca913e2a0) )
	ROM_LOAD32_WORD( "mpr-23483.ic32", 0xc000002, 0x1000000, CRC(c37adebe) SHA1(e84f6d2cc364c743f7f3b73d8c8d0271952bb093) )
	ROM_LOAD32_WORD( "mpr-23480.ic33", 0xe000000, 0x1000000, CRC(f517b8b3) SHA1(c04740adb612473c4c9f8186e7e93d2f73d1bb1a) )
	ROM_LOAD32_WORD( "mpr-23484.ic34", 0xe000002, 0x1000000, CRC(2ebe1aa1) SHA1(16b39f7422da1a334dde27169c2949e1d95bddb3) )

	// 834-14125 317-0283-COM Actel A54SX32
	// ID 0x4252
	ROM_PARAMETER( ":rom_board:key", "56dedf33" )
ROM_END

// HIKARU CHECK ROM BD, 837-13766
// Development ROM board type, looks same as Type 2/837-13403/171-7640B but have ZIF sockets instead of Mask ROMs and socketed 315-5881 chip.
ROM_START( hikcheck )
	HIKARU_BIOS
	HIKARU_EEPROM

	ROM_REGION32_LE( 0x4000000, "user1", 0)
	// Flash ROM module, label:
	// SEGA HIKARU
	// Manufacturer Test Use Program (Japanese)
	// 2000/5/19 IC 29 3199
	ROM_LOAD32_WORD("romc0.ic29", 0x0000000, 0x0400000, CRC(cb2b5b6c) SHA1(390807bc3a2c4832577abaf470eaa85168d68e51) )
	// Flash ROM module, label:
	// SEGA HIKARU
	// Manufacturer Test Use Program (Japanese)
	// 2000/5/19 IC 30 C750
	ROM_LOAD32_WORD("romc1.ic30", 0x0000002, 0x0400000, CRC(891c6b2b) SHA1(7cb2e40525dfb7c421c45fdd20f27f4e84ab0977) )
	// ic31 unpopulated
	// ic32 unpopulated
	// ic33 unpopulated
	// ic34 unpopulated
	// ic35 socket connected to "RTC BD HIKARU ROM IC35" PCB with M48T35Y Timekeeper
	ROM_LOAD32_WORD("m48t35y.ic35", 0x1800000, 0x0008000, CRC(1c25150c) SHA1(5ca4fee1515ade91677fe2c8ad7a4d5bc70a9958) )
	// ic36 unpopulated

	// SOP44 sockets, no ROMs populated
	ROM_REGION32_LE( 0x10000000, "user2", ROMREGION_ERASE00)

	// 315-5881 populated, have no 317-xxxx stamp, key is unknown.
	ROM_PARAMETER( ":rom_board:segam2crypt:key", "-1" )
ROM_END

} // anonymous namespace


GAME( 2000, hikaru,    0,        hikaru, hikaru, hikaru_state, init_hikaru, ROT0, "Sega",            "Hikaru BIOS", MACHINE_NO_SOUND|MACHINE_NOT_WORKING|MACHINE_IS_BIOS_ROOT )
GAME( 1999, braveff,   hikaru,   hikaru, hikaru, hikaru_state, init_hikaru, ROT0, "Sega",            "Brave Firefighters", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
GAME( 2000, airtrix,   hikaru,   hikaru, hikaru, hikaru_state, init_hikaru, ROT0, "Sega",            "Air Trix (Rev A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
GAME( 2000, airtrixo,  airtrix,  hikaru, hikaru, hikaru_state, init_hikaru, ROT0, "Sega",            "Air Trix (original)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
GAME( 2000, hikcheck,  hikaru,   hikaru, hikaru, hikaru_state, init_hikaru, ROT0, "Sega",            "Hikaru Check ROM Board", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
GAME( 2000, sgnascar,  hikaru,   hikaru, hikaru, hikaru_state, init_hikaru, ROT0, "Sega / Electronic Arts", "NASCAR Arcade (Rev A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
GAME( 2000, sgnascaro, sgnascar, hikaru, hikaru, hikaru_state, init_hikaru, ROT0, "Sega / Electronic Arts", "NASCAR Arcade (original)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
GAME( 2000, pharrier,  hikaru,   hikaru, hikaru, hikaru_state, init_hikaru, ROT0, "Sega",            "Planet Harriers (Rev A)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
GAME( 2000, swracer,   hikaru,   hikaru, hikaru, hikaru_state, init_hikaru, ROT0, "Sega",            "Star Wars: Racer Arcade", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
