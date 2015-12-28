// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*
 * Sega System 24
 *
 * Kudos to Charles MacDonald (http://cgfm2.emuviews.com) for his
 * very useful research
 *
 * 3 Player controls for Gain Ground added by 'Unknown'
 */

/* Missing:
   - linescroll in special modes (qgh title, mahmajn2/qrouka attract mode)
   - screen flipping (mix register 13 & 2)
   - FRC timer IRQ is currently in a slight off-beat (timer should be resetted every time
     that the mode changes, but current MAME framework doesn't seem to like it)
*/

/*
Sega System24 Overview
Sega, 1987-1994

PCB Layout
----------

Note: The differences in the revisions is just the use of different sized RAMs
and the quantities of RAM used.

(1st Revision)
837-6442 SYSTEM 24 (C)SEGA 1987
|-------------------------------------------------------------------------------|
| YM2151 DSW1 EPRxxxxx.IC1           --------------------------| TMM41464-10    |
|          DSW2         EPRxxxxx.IC2 |-------------------------- TMM41464-10    |
|                                                CN2             TMM41464-10    |
|                 68000               |---------|                TMM41464-10    |
|   |---------|  |--------------|     | 315-5295|                TMM41464-10    |
|   | 315-5296|  |HITACHI FD1094|     |(QFP100) |                TMM41464-10    |
|-| |(QFP100) |  |317-0xxx-0xx  |     |         |                TMM41464-10    |
  | |         |  |--------------|     |---------|                TMM41464-10    |
|-| |---------|                                                  MB81C466-10    |
|                                                                MB81C466-10    |
|                                     |---------|                MB81C466-10    |
|S                        |---------| | 315-5295|                MB81C466-10    |
|E                 20MHz  | 315-5295| |(QFP100) |                MB81C466-10    |
|G                        |(QFP100) | |         |                MB81C466-10    |
|A                        |         | |---------|                MB81C466-10    |
|                         |---------|                            MB81C466-10    |
|5                                                               |---------|    |
|6                       |---------|  |---------|                | 315-5293|    |
|                        | 315-5294|  | 315-5292|                | (QFP160)|    |
|                        |(QFP100) |  | (QFP160)|    32MHz       |         |    |
|            MB81C78A-45 |         |  |         |                |---------|    |
|-|          MB81C78A-45 |---------|  |---------|M5M4464-12 x4   MB81461-12 x6  |
  |                                                                             |
|-|      YM3012                                                                 |
|            |--------|                                                         |
|            |315-5242| HM65256 HM65256 HM65256  M5M4464-12 x4   MB81461-12 x6  |
|            |(QFP44) | HM65256 HM65256 HM65256                                 |
|------------|--------|---------------------------------------------------------|


(2nd Revision)
837-6442-01 SYSTEM 24 (C) SEGA 1987
|-------------------------------------------------------------------------------|
| YM2151 DSW1 EPRxxxxx.IC1           --------------------------| TMM41464-10    |
|          DSW2         EPRxxxxx.IC2 |-------------------------- TMM41464-10    |
|                                                CN2             TMM41464-10    |
|                 68000               |---------|                TMM41464-10    |
|   |---------|  |--------------|     | 315-5295|                TMM41464-10    |
|   | 315-5296|  |HITACHI FD1094|     |(QFP100) |                TMM41464-10    |
|-| |(QFP100) |  |317-0xxx-0xx  |     |         |                TMM41464-10    |
  | |         |  |--------------|     |---------|                TMM41464-10    |
|-| |---------|                                                  TMM41464-10    |
|                                                                TMM41464-10    |
|                                     |---------|                TMM41464-10    |
|S                        |---------| | 315-5295|                TMM41464-10    |
|E                 20MHz  | 315-5295| |(QFP100) |                TMM41464-10    |
|G                        |(QFP100) | |         |                TMM41464-10    |
|A                        |         | |---------|                TMM41464-10    |
|                         |---------|                            TMM41464-10    |
|5                                                               |---------|    |
|6                       |---------|  |---------|                | 315-5293|    |
|                        | 315-5294|  | 315-5292|                | (QFP160)|    |
|                        |(QFP100) |  | (QFP160)|    32MHz       |         |    |
|            MB81C78A-45 |         |  |         |                |---------|    |
|-|          MB81C78A-45 |---------|  |---------|M5M4464-12 x4   MB81461-12 x6  |
  |                                                                             |
|-|      YM3012                                (*)                              |
|            |--------|                                                         |
|            |315-5242| HM65256 HM65256 HM65256  M5M4464-12 x4   MB81461-12 x6  |
|            |(QFP44) | HM65256 HM65256 HM65256                                 |
|------------|--------|---------------------------------------------------------|
(*) Unpopulated Sockets for MB81C4256 x4


(3rd Revision)
837-6442-02 SYSTEM 24 (C) SEGA 1987
|-------------------------------------------------------------------------------|
| YM2151 DSW1 EPRxxxxx.IC1           --------------------------| TC514256-10    |
|          DSW2         EPRxxxxx.IC2 |-------------------------- TC514256-10    |
|                                                CN2             TC514256-10    |
|                 68000               |---------|                TC514256-10    |
|   |---------|  |--------------|     | 315-5295|                TC514256-10    |
|   | 315-5296|  |HITACHI FD1094|     |(QFP100) |                TC514256-10    |
|-| |(QFP100) |  |317-0xxx-0xx  |     |         |                TC514256-10    |
  | |         |  |--------------|     |---------|                TC514256-10    |
|-| |---------|                                                                 |
|                                                                               |
|                                     |---------|                               |
|S                        |---------| | 315-5295|                               |
|E                 20MHz  | 315-5295| |(QFP100) |                               |
|G                        |(QFP100) | |         |                               |
|A                        |         | |---------|                               |
|                         |---------|                                           |
|5                                                               |---------|    |
|6                       |---------|  |---------|                | 315-5293|    |
|                        | 315-5294|  | 315-5292|                | (QFP160)|    |
|                        |(QFP100) |  | (QFP160)|    32MHz       |         |    |
|            MB81C78A-45 |         |  |         |                |---------|    |
|-|          MB81C78A-45 |---------|  |---------|                MB81461-12 x6  |
  |                                                                             |
|-|      YM3012                                                                 |
|            |--------|                                                         |
|            |315-5242| HM65256 HM65256 TC51832  TC514256-10 x4  MB81461-12 x6  |
|            |(QFP44) | HM65256 HM65256 TC51832                                 |
|------------|--------|---------------------------------------------------------|
Notes:
               315-5292: Custom Sega IC (QFP160, Tilemap Generator)
               315-5293: Custom Sega IC (QFP160, Object Generator)
               315-5294: Custom Sega IC (QFP100, Mixer Chip)
               315-5295: Custom Sega IC (QFP100, Object Generator)
               315-5296: Custom Sega IC (QFP100, I/O Chip)
               315-5242: Custom Sega IC (QFP44, Colour Encoder)
            68000 clock: 10.000MHz
   Hitachi FD1094 clock: 10.000MHz
           YM2151 clock: 4.000MHz
                  VSync: 58Hz
                  HSync: 24.33kHz (All System24 games require 24kHz monitor)
        CN2 (Above PCB): Connector for ROM Board (Not used for floppy-based games)
        CN2 (Below PCB): Connector for Floppy Controller Board (Not used for ROM-based games)
        Main PCB Pinout same as System 16
        Floppy Drive is a standard 1.44 High Density drive, but the controller is custom and
        the floppy disk format is custom. The floppy disk can be read with "Anadisk"
        depending on the PC being used and it's floppy controller. Most clone PC's can't read the
        System 24 floppies even with "Anadisk"[1]. But many brand-name PC's can. It's likely due to the
        propietry nature of the components used in brand-name PC's. Generally the older and crappier
        the PC is, the better chance you have of being able to read the floppy ;-)

        [1] Actually, most can _except_ for the Hotrod disks.  Those 8K sectors are deadly.


Floppy Controller PCB Layout
----------------------------

837-6443         ___________
|---------------|    DATA   |-- ||||---|
|                               PWR    |
|     74LS367 74LS05                   |
|     74LS367 74LS174                  |
|                     MB4107   74LS05  |
|                                      |
|     MB89311         74LS139   8MHz   |
|                CN1                   |
|     --------------------------|      |
|     |--------------------------      |
|--------------------------------------|


I/O Controller PCB Layout
-------------------------

834-6510
(sticker 834-6510-01)   ___________
|----------------------|  IDC 34   |---|
|                                      |
|                                      |
|  D4701          %2                   |
|                                      |
|                 %3                   |
|    %1                       74LS139  |
|                CN1                   |
|     --------------------------|      |
|     |--------------------------      |
|--------------------------------------|
Notes:
      - %1 - Unpopulated position for D4701 IC
        %2 - Unpopulated position for MSM6253RS IC
        %3 - Unpopulated position for MSM6253RS IC

      - CN1 is shown for completeness, it's actually underneath the I/O PCB.

      - The custom I/O board plugs into the top connector on the main board and
        is used for player steering controls. The wiring diagram has 8 pins coming from
        the connector. Four are labelled 'JST4P 1P (L)', the other four are labelled
        'JST4P 2P (R)'. I assume this is for an analog wheel or similar for player 1
        (being on the left) and player 2 (being on the right)
        The wiring diagram also lists some buttons for both PL1 and PL2 coming from
        the regular 56-way edge connector....  'Nitro' and 'Accel'
        The manual shows a controller that looks like a spinner which they call
        a 'sensor board' with part 839-0138 and has a photo-sensor disc, which looks
        like it's used for steering.
        This board is used on Hot Rod, Rough Racer and the Golf games only.


ROM Board Layout
----------------

837-7187-01
171-5875-01B
(SYSTEM24) (C)SEGA 1989 . 1991
|------------------------------------------|
|                                          |
|                                          |
|                                  J1      |
|CN1    ROM1.IC4      ROM2.IC5     J2      |
||-|                               J3      |
|| |                               J4      |
|| |    ROM3.IC6      ROM4.IC7             |
|| |                               J5      |
|| |                               J6      |
|| |    ROM5.IC8      ROM6.IC9     J7      |
|| |                               J8      |
|| |                               J9      |
|| |    ROM7.IC10     ROM8.IC11            |
|| |                                       |
|| |                  74HC4040  74LS139    |
||-|    EPM5032DC.IC2                      |
|------------------------------------------|
Notes:
      CN1: Connector for joining ROM board to Main Board. This connector is above and below the PCB to allow for chaining of several boards.

      The ROM Daughter board has a protection device on it at location IC2. It's an Erasable Programmable Logic Device; EPLD, like a normal PLD,
      but uses an EPROM and is erasable and re-programmable. It's type is Altera EPM5032DC. It's part of the Altera MAX 5000 Programmable Logic
      Device Family and (generally) has it's protection bit set at the factory.

      All jumpers are 2 pins, they're either shorted or not shorted. All ROMs are configured as the same type.
      See the table below for details.
      Jumpers to set ROM sizes:
                               2M (J1, J3, J6, J8)
                               4M (J2, J3, J7, J9)
                               8M (J2, J4, J5, J9)


FD1094/Floppy/ROM details
-------------------------

Game                   FD1094         Floppy          ROMs                                                          Other
-------------------------------------------------------------------------------------------------------------------------------------------------------------
Hot Rod (3p Turbo)     none           DS3-5000-01D    Main Board:     EPR11338.IC1, EPR11339.IC2                    I/O Controller PCB, Floppy Controller PCB

Hot Rod (4p Rev C)     none           DS3-5000-01A    Main Board:     EPR11338.IC1, EPR11339.IC2                    I/O Controller PCB, Floppy Controller PCB
                                      Rev C

Hot Rod (4p Japanese   none           DS3-5000-01A    Main Board:     EPR11338.IC1, EPR11339.IC2                    I/O Controller PCB, Floppy Controller PCB
         Rev B)                       Rev B

Scramble Spirits       none           DS3-5000-02?    Main Board:     EPR12186.IC1, EPR12187.IC2                    Floppy Controller PCB

Scramble Spirits       317-0058-02D   DS3-5000-02D    Main Board:     EPR12186.IC1, EPR12187.IC2                    Floppy Controller PCB
(Encrypted Version)

Gain Ground (Japan)    317-0058-03B   DS3-5000-03B    Main Board:     EPR12186.IC1, EPR12187.IC2                    Floppy Controller PCB

Gain Ground            317-0058-03D   DS3-5000-03D    Main Board:     EPR12186.IC1, EPR12187.IC2                    Floppy Controller PCB

Crack Down (Japan)     317-0058-04B   DS3-5000-04B    Main Board:     EPR12186.IC1, EPR12187.IC2                    Floppy Controller PCB
                                      Rev A

Crack Down             317-0058-04D   DS3-5000-04D    Main Board:     EPR12186.IC1, EPR12187.IC2                    Floppy Controller PCB

Super Masters Golf     317-0058-05B   DS3-5000-05B    Main Board:     EPR12186.IC1, EPR12187.IC2                    I/O Controller PCB, Floppy Controller PCB
Rev B

Super Masters Golf     317-0058-05D   DS3-5000-05D    Main Board:     EPR12186.IC1, EPR12187.IC2                    I/O Controller PCB, Floppy Controller PCB
Rev D

Rough Racer            317-0058-06B   DS3-5000-06B    Main Board:     EPR12186.IC1, EPR12187.IC2                    I/O Controller PCB, Floppy Controller PCB

Bonanza Bros           none           DS3-5000-07D    Main Board:     EPR12186.IC1, EPR12187.IC2                    Floppy Controller PCB
                                                      Daughter Board: MPR13187.IC4, MPR13188.IC5, MPR13189.IC6
                                                                      MPR13190.IC7
                                                                      EPLD: 317-0156

Quiz Syukudai wo       317-0058-08B   DS3-5000-08B    Main Board:     EPR12186.IC1, EPR12187.IC2                    Floppy Controller PCB
Wasuremashita

Dynamic Country Club   317-0058-09D   DS3-5000-09D    Main Board:     EPR13947.IC1, EPR13948.IC2                    I/O Controller PCB, Floppy Controller PCB
(Disk version)

Dynamic Country Club   none           none            Main Board:     EPR13947.IC1, EPR13948.IC2
(ROM version)                                         Daughter Board: EPR15345.IC4  EPR15344.IC5, MPR14097.IC6
                                                                      MPR14096.IC7
                                                                      EPLD: 317-0177

Quiz Mekuromeku Story  none           none            Main Board:     EPR15342.IC1, EPR15343.IC2
                                                      Daughter Board: EPR15344.IC4, EPR15345.IC5, EPR15346.IC6
                                                                      EPR15347.IC7, EPR15348.IC8, EPR15349.IC9
                                                                      EPLD: 317-0205

Tokoro San no          none           none            Main Board:     EPR14812.IC1, EPR14813.IC2
MahMahjan                                             Daughter Board: MPR14819.IC4, MPR14820.IC5, MPR14821.IC6
                                                                      MPR14822IC.7, MPR14823.IC8, MPR14824.IC9
                                                                      MPR14825.IC10, MPR14826.IC11
                                                                      EPLD: 317-0200

Quiz Rouka Ni          none           none            Main Board:     EPR14484.IC1, EPR14485.IC2
Tattenasai                                            Daughter Board: EPR14482.IC5, EPR14483.IC7
                                                                      EPLD: 317-0191

Tokoro San no          none           none            Main Board:     EPR16798.IC1, EPR16799.IC2
MahMahjan 2                                           Daughter Board: MPR16800.IC4, MPR16801.IC5, MPR16802.IC6
                                                                      MPR16803.IC7, MPR16804.IC8, MPR16805.IC9
                                                                      MPR16806.IC10, MPR16807.IC11
                                                                      EPLD: 317-0220

Quiz Ghost Hunter      none           none            Main Board:     EPR16899B.IC1, EPR16900B.IC2
                                                      Daughter Board: MPR16901A.IC4, MPR16902A.IC5, MPR16903.IC6
                                                                      MPR16904.IC7,  MPR16905.IC8,  MPR16906.IC9
                                                                      MPR16907.IC10,  MPR16908.IC11
                                                                      EPLD: 317-0226

Notes:
      A regular 68000-10 is used in position IC3. If used, the Hitachi CPU sits at position IC4.
      Where FD1094 is listed as 'none', a standard 68000 CPU is used instead at IC4 on the PCB.

      The last 3 digits of the floppy number is also the last part of the Hitachi CPU number....they're a matching pair.
      So if one had the Hitachi part number and a floppy without a label, it's possible to create the correct floppy disk name from
      the last 3 digits of the Hitachi FD1094 part number.

*/


/* system24temp_ functions / variables are from shared rewrite files,
   once the rest of the rewrite is complete they can be removed, I
   just made a copy & renamed them for now to avoid any conflicts
*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/ym2151.h"
#include "sound/dac.h"
#include "sound/2151intf.h"
#include "machine/fd1094.h"
#include "machine/nvram.h"
#include "video/segaic24.h"
#include "includes/segas24.h"
#include "includes/segaipt.h"

#define MASTER_CLOCK        XTAL_20MHz
#define VIDEO_CLOCK         XTAL_32MHz
#define TIMER_CLOCK         (VIDEO_CLOCK/4)
#define HSYNC_CLOCK         (VIDEO_CLOCK/2/656.0)
/* TODO: understand why divisors doesn't match at all with the reference */
#define FRC_CLOCK_MODE0     (MASTER_CLOCK/2)/24 // /16 according to Charles
#define FRC_CLOCK_MODE1     (MASTER_CLOCK/2)/1536 // /1024 according to Charles, but /1536 sounds better

enum {
	IRQ_YM2151 = 1,
	IRQ_TIMER  = 2,
	IRQ_VBLANK = 3,
	IRQ_SPRITE = 4,
	IRQ_FRC = 5
};

// Floppy Disk Controller

void segas24_state::fdc_init()
{
	fdc_status = 0;
	fdc_track = 0;
	fdc_sector = 0;
	fdc_data = 0;
	fdc_phys_track = 0;
	fdc_irq = 0;
	fdc_drq = 0;
	fdc_index_count = 0;
}

READ16_MEMBER( segas24_state::fdc_r )
{
	if(!track_size)
		return 0xffff;

	switch(offset) {
	case 0:
		fdc_irq = 0;
		return fdc_status;
	case 1:
		return fdc_track;
	case 2:
		return fdc_sector;
	case 3:
	default: {
		int res = fdc_data;
		if(fdc_drq) {
			fdc_span--;
			// logerror("Read %02x (%d)\n", res, fdc_span);
			if(fdc_span) {
				fdc_pt++;
				fdc_data = *fdc_pt;
			} else {
				logerror("FDC: transfert complete\n");
				fdc_drq = 0;
				fdc_status = 0;
				fdc_irq = 1;
			}
		} else
			logerror("FDC: data read with drq down\n");
		return res;
	}
	}
}

WRITE16_MEMBER( segas24_state::fdc_w )
{
	if(!track_size)
		return;

	if(ACCESSING_BITS_0_7) {
		data &= 0xff;
		switch(offset) {
		case 0:
			fdc_irq = 0;
			switch(data >> 4) {
			case 0x0:
				logerror("FDC: Restore\n");
				fdc_phys_track = fdc_track = 0;
				fdc_irq = 1;
				fdc_status = 4;
				break;
			case 0x1:
				logerror("FDC: Seek %d\n", fdc_data);
				fdc_phys_track = fdc_track = fdc_data;
				fdc_irq = 1;
				fdc_status = fdc_track ? 0 : 4;
				break;
			case 0x9:
				logerror("Read multiple [%02x] %d..%d side %d track %d\n", data, fdc_sector, fdc_sector+fdc_data-1, data & 8 ? 1 : 0, fdc_phys_track);
				fdc_pt = memregion("floppy")->base() + track_size*(2*fdc_phys_track+(data & 8 ? 1 : 0));
				fdc_span = track_size;
				fdc_status = 3;
				fdc_drq = 1;
				fdc_data = *fdc_pt;
				break;
			case 0xb:
				logerror("Write multiple [%02x] %d..%d side %d track %d\n", data, fdc_sector, fdc_sector+fdc_data-1, data & 8 ? 1 : 0, fdc_phys_track);
				fdc_pt = memregion("floppy")->base() + track_size*(2*fdc_phys_track+(data & 8 ? 1 : 0));
				fdc_span = track_size;
				fdc_status = 3;
				fdc_drq = 1;
				break;
			case 0xd:
				logerror("FDC: Forced interrupt\n");
				fdc_span = 0;
				fdc_drq = 0;
				fdc_irq = data & 1;
				fdc_status = 0;
				break;
			case 0xf:
				if(data == 0xfe)
					logerror("FDC: Assign mode %02x\n", fdc_data);
				else if(data == 0xfd)
					logerror("FDC: Assign parameter %02x\n", fdc_data);
				else
					logerror("FDC: Unknown command %02x\n", data);
				break;
			default:
				logerror("FDC: Unknown command %02x\n", data);
				break;
			}
			break;
		case 1:
			logerror("FDC: Track register %02x\n", data);
			fdc_track = data;
			break;
		case 2:
			logerror("FDC: Sector register %02x\n", data);
			fdc_sector = data;
			break;
		case 3:
			if(fdc_drq) {
				//              logerror("Write %02x (%d)\n", data, fdc_span);
				*fdc_pt++ = data;
				fdc_span--;
				if(!fdc_span) {
					logerror("FDC: transfert complete\n");
					fdc_drq = 0;
					fdc_status = 0;
					fdc_irq = 1;
				}
			} else
				logerror("FDC: Data register %02x\n", data);
			fdc_data = data;
			break;
		}
	}
}

READ16_MEMBER( segas24_state::fdc_status_r )
{
	if(!track_size)
		return 0xffff;

	return 0x90 | (fdc_irq ? 2 : 0) | (fdc_drq ? 1 : 0) | (fdc_phys_track ? 0x40 : 0) | (fdc_index_count ? 0x20 : 0);
}

WRITE16_MEMBER( segas24_state::fdc_ctrl_w )
{
	if(ACCESSING_BITS_0_7)
		logerror("FDC control %02x\n", data & 0xff);
}


// I/O Mappers

UINT8 segas24_state::hotrod_io_r(UINT8 port)
{
	switch(port)
	{
	case 0:
		return ioport("P1")->read();
	case 1:
		return ioport("P2")->read();
	case 2:
		return read_safe(ioport("P3"), 0xff);
	case 3:
		return 0xff;
	case 4:
		return ioport("SERVICE")->read();
	case 5: // Dip switches
		return ioport("COINAGE")->read();
	case 6:
		return ioport("DSW")->read();
	case 7: // DAC
		return 0xff;
	}
	return 0x00;
}

UINT8 segas24_state::dcclub_io_r(UINT8 port)
{
	switch(port)
	{
	case 0:
	{
		static const UINT8 pos[16] = { 0, 1, 3, 2, 6, 4, 12, 8, 9 };
		return (ioport("P1")->read() & 0xf) | ((~pos[ioport("PADDLE")->read()>>4]<<4) & 0xf0);
	}
	case 1:
		return ioport("P2")->read();
	case 2:
		return 0xff;
	case 3:
		return 0xff;
	case 4:
		return ioport("SERVICE")->read();
	case 5: // Dip switches
		return ioport("COINAGE")->read();
	case 6:
		return ioport("DSW")->read();
	case 7: // DAC
		return 0xff;
	}
	return 0x00;
}


UINT8 segas24_state::mahmajn_io_r(UINT8 port)
{
	static const char *const keynames[] = { "MJ0", "MJ1", "MJ2", "MJ3", "MJ4", "MJ5", "P1", "P2" };

	switch(port)
	{
	case 0:
		return ~(1 << cur_input_line);
	case 1:
		return 0xff;
	case 2:
		return ioport(keynames[cur_input_line])->read();
	case 3:
		return 0xff;
	case 4:
		return ioport("SERVICE")->read();
	case 5: // Dip switches
		return ioport("COINAGE")->read();
	case 6:
		return ioport("DSW")->read();
	case 7: // DAC
		return 0xff;
	}
	return 0x00;
}

void segas24_state::mahmajn_io_w(UINT8 port, UINT8 data)
{
	switch(port)
	{
	case 3:
		if(data & 4)
			cur_input_line = (cur_input_line + 1) & 7;
		break;
	case 7: // DAC
		m_dac->write_signed8(data);
		break;
	default:
		fprintf(stderr, "Port %d : %02x\n", port, data & 0xff);
	}
}

void segas24_state::hotrod_io_w(UINT8 port, UINT8 data)
{
	switch(port)
	{
	case 3: // Lamps
		break;
	case 7: // DAC
		m_dac->write_signed8(data);
		break;
	default:
		fprintf(stderr, "Port %d : %02x\n", port, data & 0xff);
	}
}


WRITE16_MEMBER( segas24_state::hotrod3_ctrl_w )
{
	static const char *const portnames[] = { "PEDAL1", "PEDAL2", "PEDAL3", "PEDAL4" };

	if(ACCESSING_BITS_0_7)
	{
		data &= 3;
		hotrod_ctrl_cur = read_safe(ioport(portnames[data]), 0);
	}
}

READ16_MEMBER( segas24_state::hotrod3_ctrl_r )
{
	if(ACCESSING_BITS_0_7)
	{
		switch(offset)
		{
			// Steering dials
			case 0:
				return read_safe(ioport("DIAL1"), 0) & 0xff;
			case 1:
				return read_safe(ioport("DIAL1"), 0) >> 8;
			case 2:
				return read_safe(ioport("DIAL2"), 0) & 0xff;
			case 3:
				return read_safe(ioport("DIAL2"), 0) >> 8;
			case 4:
				return read_safe(ioport("DIAL3"), 0) & 0xff;
			case 5:
				return read_safe(ioport("DIAL3"), 0) >> 8;
			case 6:
				return read_safe(ioport("DIAL4"), 0) & 0xff;
			case 7:
				return read_safe(ioport("DIAL4"), 0) >> 8;

			case 8:
			{
				// Serial ADCs for the accel
				int v = hotrod_ctrl_cur & 0x80;
				hotrod_ctrl_cur <<= 1;
				return v ? 0xff : 0;
			}
		}
	}
	return 0;
}

READ16_MEMBER( segas24_state::iod_r )
{
	logerror("IO daughterboard read %02x (%x)\n", offset, space.device().safe_pc());
	return 0xffff;
}

WRITE16_MEMBER( segas24_state::iod_w )
{
	logerror("IO daughterboard write %02x, %04x & %04x (%x)\n", offset, data, mem_mask, space.device().safe_pc());
}


// Cpu #1 reset control


void segas24_state::reset_reset()
{
	int changed = resetcontrol ^ prev_resetcontrol;
	if(changed & 2) {
		if(resetcontrol & 2) {
			m_subcpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
			m_subcpu->set_input_line(INPUT_LINE_RESET, PULSE_LINE);
//          osd_printf_debug("enable 2nd cpu!\n");
//          debugger_break(machine);

		} else
			m_subcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}
	if(changed & 4)
		machine().device("ymsnd")->reset();
	prev_resetcontrol = resetcontrol;
}

void segas24_state::reset_control_w(UINT8 data)
{
	resetcontrol = data;
	reset_reset();
}


// Rom board bank access


void segas24_state::reset_bank()
{
	if (memregion("romboard")->base())
	{
		membank("bank1")->set_entry(curbank & 15);
		membank("bank2")->set_entry(curbank & 15);
	}
}

READ16_MEMBER( segas24_state::curbank_r )
{
	return curbank;
}

WRITE16_MEMBER( segas24_state::curbank_w )
{
	if(ACCESSING_BITS_0_7) {
		curbank = data & 0xff;
		reset_bank();
	}
}

READ8_MEMBER( segas24_state::frc_mode_r )
{
	return frc_mode & 1;
}

WRITE8_MEMBER( segas24_state::frc_mode_w )
{
	/* reset frc if a write happens here */
	frc_cnt_timer->reset();
	frc_mode = data & 1;
}

READ8_MEMBER( segas24_state::frc_r )
{
	INT32 result = (frc_cnt_timer->time_elapsed() * (frc_mode ? FRC_CLOCK_MODE1 : FRC_CLOCK_MODE0)).as_double();

	result %= ((frc_mode) ? 0x67 : 0x100);

	return result;
}

WRITE8_MEMBER( segas24_state::frc_w )
{
	/* Undocumented behaviour, Bonanza Bros. seems to use this for irq ack'ing ... */
	m_maincpu->set_input_line(IRQ_FRC+1, CLEAR_LINE);
	m_subcpu->set_input_line(IRQ_FRC+1, CLEAR_LINE);
}


// Protection magic latch

const UINT8  segas24_state::mahmajn_mlt[8] = { 5, 1, 6, 2, 3, 7, 4, 0 };
const UINT8 segas24_state::mahmajn2_mlt[8] = { 6, 0, 5, 3, 1, 4, 2, 7 };
const UINT8      segas24_state::qgh_mlt[8] = { 3, 7, 4, 0, 2, 6, 5, 1 };
const UINT8 segas24_state::bnzabros_mlt[8] = { 2, 4, 0, 5, 7, 3, 1, 6 };
const UINT8   segas24_state::qrouka_mlt[8] = { 1, 6, 4, 7, 0, 5, 3, 2 };
const UINT8 segas24_state::quizmeku_mlt[8] = { 0, 3, 2, 4, 6, 1, 7, 5 };
const UINT8   segas24_state::dcclub_mlt[8] = { 4, 7, 3, 0, 2, 6, 5, 1 };


READ16_MEMBER( segas24_state::mlatch_r )
{
	return mlatch;
}

WRITE16_MEMBER( segas24_state::mlatch_w )
{
	if(ACCESSING_BITS_0_7) {
		int i;
		UINT8 mxor = 0;
		if(!mlatch_table) {
			logerror("Protection: magic latch accessed but no table loaded (%s:%x)\n", space.device().tag(), space.device().safe_pc());
			return;
		}

		data &= 0xff;

		if(data != 0xff) {
			for(i=0; i<8; i++)
				if(mlatch & (1<<i))
					mxor |= 1 << mlatch_table[i];
			mlatch = data ^ mxor;
			logerror("Magic latching %02x ^ %02x as %02x (%s:%x)\n", data & 0xff, mxor, mlatch, space.device().tag(), space.device().safe_pc());
		} else {
			logerror("Magic latch reset (%s:%x)\n", space.device().tag(), space.device().safe_pc());
			mlatch = 0x00;
		}
	}
}


// Timers and IRQs

void segas24_state::irq_timer_sync()
{
	attotime ctime = machine().time();

	switch(irq_tmode) {
	case 0:
		break;
	case 1: {
		// Don't remove the floor(), the value may be slightly negative
		int ppos = floor((irq_synctime - irq_vsynctime).as_double() * HSYNC_CLOCK);
		int cpos = floor((ctime - irq_vsynctime).as_double() * HSYNC_CLOCK);
		irq_tval += cpos-ppos;
		break;
	}
	case 2: {
		fatalerror("segas24_state::irq_timer_sync - case 2\n");
	}
	case 3: {
		int ppos = floor((irq_synctime - irq_vsynctime).as_double() * TIMER_CLOCK);
		int cpos = floor((ctime - irq_vsynctime).as_double() * TIMER_CLOCK);
		irq_tval += cpos-ppos;
		break;
	}
	}

	irq_synctime = ctime;
}

void segas24_state::irq_timer_start(int old_tmode)
{
	switch(irq_tmode) {
	case 0:
		if(old_tmode) {
			irq_tval++;
			if(irq_tval == 0x1000)
				irq_timer->adjust(attotime::zero);
			else
				irq_timer->enable(false);
		}
		break;
	case 1: {
		int count = 0x1000 - irq_tval;
		irq_timer->adjust(attotime::from_hz(HSYNC_CLOCK)*count);
		break;
	}
	case 2:
		fatalerror("segas24_state::irq_timer_start - case 2\n");
	case 3: {
		int count = 0x1000 - irq_tval;
		irq_timer->adjust(attotime::from_hz(TIMER_CLOCK)*count);
		break;
	}
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(segas24_state::irq_timer_cb)
{
	irq_timer_sync();

	if(irq_tval != 0x1000)
		fprintf(stderr, "Error: timer desync %x != 1000\n", irq_tval);

	irq_tval = irq_tdata;
	irq_timer_start(irq_tmode);

	irq_timer_pend0 = irq_timer_pend1 = 1;
	if(irq_allow0 & (1 << IRQ_TIMER))
		m_maincpu->set_input_line(IRQ_TIMER+1, ASSERT_LINE);
	if(irq_allow1 & (1 << IRQ_TIMER))
		m_subcpu->set_input_line(IRQ_TIMER+1, ASSERT_LINE);

	if (irq_tmode == 1 || irq_tmode == 2)
	{
	//  m_screen->update_now();
		m_screen->update_partial(m_screen->vpos());
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(segas24_state::irq_timer_clear_cb)
{
	irq_sprite = irq_vblank = 0;
	m_maincpu->set_input_line(IRQ_VBLANK+1, CLEAR_LINE);
	m_maincpu->set_input_line(IRQ_SPRITE+1, CLEAR_LINE);
	m_subcpu->set_input_line(IRQ_VBLANK+1, CLEAR_LINE);
	m_subcpu->set_input_line(IRQ_SPRITE+1, CLEAR_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER(segas24_state::irq_frc_cb)
{
	if(irq_allow0 & (1 << IRQ_FRC) && frc_mode == 1)
		m_maincpu->set_input_line(IRQ_FRC+1, ASSERT_LINE);

	if(irq_allow1 & (1 << IRQ_FRC) && frc_mode == 1)
		m_subcpu->set_input_line(IRQ_FRC+1, ASSERT_LINE);
}

void segas24_state::irq_init()
{
	irq_tdata = 0;
	irq_tmode = 0;
	irq_allow0 = 0;
	irq_allow1 = 0;
	irq_timer_pend0 = 0;
	irq_timer_pend1 = 0;
	irq_vblank = 0;
	irq_sprite = 0;
	irq_timer = machine().device<timer_device>("irq_timer");
	irq_timer_clear = machine().device<timer_device>("irq_timer_clear");
	irq_tval = 0;
	irq_synctime = attotime::zero;
	irq_vsynctime = attotime::zero;
}

WRITE16_MEMBER(segas24_state::irq_w)
{
	switch(offset) {
	case 0: {
		irq_timer_sync();
		COMBINE_DATA(&irq_tdata);
		irq_tdata &= 0xfff;
		irq_timer_start(irq_tmode);
		break;
	}
	case 1:
		if(ACCESSING_BITS_0_7) {
			UINT8 old_tmode = irq_tmode;
			irq_timer_sync();
			irq_tmode = data & 3;
			irq_timer_start(old_tmode);
		}
		break;
	case 2:
		irq_allow0 = data & 0x3f;
		irq_timer_pend0 = 0;
		m_maincpu->set_input_line(IRQ_TIMER+1, CLEAR_LINE);
		m_maincpu->set_input_line(IRQ_YM2151+1, irq_yms && (irq_allow0 & (1 << IRQ_YM2151)) ? ASSERT_LINE : CLEAR_LINE);
		m_maincpu->set_input_line(IRQ_VBLANK+1, irq_vblank && (irq_allow0 & (1 << IRQ_VBLANK)) ? ASSERT_LINE : CLEAR_LINE);
		m_maincpu->set_input_line(IRQ_SPRITE+1, irq_sprite && (irq_allow0 & (1 << IRQ_SPRITE)) ? ASSERT_LINE : CLEAR_LINE);
		//m_maincpu->set_input_line(IRQ_FRC+1, irq_frc && (irq_allow0 & (1 << IRQ_FRC)) ? ASSERT_LINE : CLEAR_LINE);
		break;
	case 3:
		irq_allow1 = data & 0x3f;
		irq_timer_pend1 = 0;
		m_subcpu->set_input_line(IRQ_TIMER+1, CLEAR_LINE);
		m_subcpu->set_input_line(IRQ_YM2151+1, irq_yms && (irq_allow1 & (1 << IRQ_YM2151)) ? ASSERT_LINE : CLEAR_LINE);
		m_subcpu->set_input_line(IRQ_VBLANK+1, irq_vblank && (irq_allow1 & (1 << IRQ_VBLANK)) ? ASSERT_LINE : CLEAR_LINE);
		m_subcpu->set_input_line(IRQ_SPRITE+1, irq_sprite && (irq_allow1 & (1 << IRQ_SPRITE)) ? ASSERT_LINE : CLEAR_LINE);
		//m_subcpu->set_input_line(IRQ_FRC+1, irq_frc && (irq_allow1 & (1 << IRQ_FRC)) ? ASSERT_LINE : CLEAR_LINE);
		break;
	}
}

// gground bp at 849c (84a4)
// cmp.w $a00000, d0   | 16
// dbeq d1, 84b4       | 10
// 26 cycles/tour
// 0x200 counts
// 656 cycles/ligne
// 656*0x200/26 = 12918 (3276) -> cd89 (got e087, delta = 1f78, 8056)
// 410 cycles/ligne
// 410*0x200/26 = 8073

READ16_MEMBER(segas24_state::irq_r)
{
	switch(offset) {
	case 2:
		irq_timer_pend0 = 0;
		m_maincpu->set_input_line(IRQ_TIMER+1, CLEAR_LINE);
		break;
	case 3:
		irq_timer_pend1 = 0;
		m_subcpu->set_input_line(IRQ_TIMER+1, CLEAR_LINE);
		break;
	}
	irq_timer_sync();
	return irq_tval & 0xfff;
}

TIMER_DEVICE_CALLBACK_MEMBER(segas24_state::irq_vbl)
{
	int irq, mask;
	int scanline = param;

	/* TODO: perhaps vblank irq happens at 400, sprite IRQ certainly don't at 0! */
	if(scanline == 0) { irq = IRQ_SPRITE; irq_sprite = 1; }
	else if(scanline == 384) { irq = IRQ_VBLANK; irq_vblank = 1; }
	else
		return;

	irq_timer_clear->adjust(attotime::from_hz(HSYNC_CLOCK));

	mask = 1 << irq;

	if(irq_allow0 & mask)
		m_maincpu->set_input_line(1+irq, ASSERT_LINE);

	if(irq_allow1 & mask)
		m_subcpu->set_input_line(1+irq, ASSERT_LINE);

	if(scanline == 384) {
		// Ensure one index pulse every 20 frames
		// The is some code in bnzabros at 0x852 that makes it crash
		// if the pulse train is too fast
		fdc_index_count++;
		if(fdc_index_count >= 20)
			fdc_index_count = 0;
	}

	irq_timer_sync();
	irq_vsynctime = machine().time();
}

WRITE_LINE_MEMBER(segas24_state::irq_ym)
{
	irq_yms = state;
	m_maincpu->set_input_line(IRQ_YM2151+1, irq_yms && (irq_allow0 & (1 << IRQ_YM2151)) ? ASSERT_LINE : CLEAR_LINE);
	m_subcpu->set_input_line(IRQ_YM2151+1, irq_yms && (irq_allow1 & (1 << IRQ_YM2151)) ? ASSERT_LINE : CLEAR_LINE);
}


READ16_MEMBER ( segas24_state::sys16_io_r )
{
	//  logerror("IO read %02x (%s:%x)\n", offset, space.device().tag(), space.device().safe_pc());
	if(offset < 8)
		return (this->*io_r)(offset);
	else if (offset < 0x20) {
		switch(offset) {
		case 0x8:
			return 'S';
		case 0x9:
			return 'E';
		case 0xa:
			return 'G';
		case 0xb:
			return 'A';
		case 0xe:
			return io_cnt;
		case 0xf:
			return io_dir;
		default:
			logerror("IO control read %02x (%s:%x)\n", offset, space.device().tag(), space.device().safe_pc());
			return 0xff;
		}
	} else
		return iod_r(space, offset & 0x1f, mem_mask);
}

WRITE16_MEMBER( segas24_state::sys16_io_w )
{
	if(ACCESSING_BITS_0_7) {
		if(offset < 8) {
			if(!(io_dir & (1 << offset))) {
				logerror("IO port write on input-only port (%d, [%02x], %02x, %s:%x)\n", offset, io_dir, data & 0xff, space.device().tag(), space.device().safe_pc());
				return;
			}
			(this->*io_w)(offset, data);
		} else if (offset < 0x20) {
			switch(offset) {
			case 0xe:
				io_cnt = data;
				reset_control_w(data & 7);
				break;
			case 0xf:
				io_dir = data;
				break;
			default:
				logerror("IO control write %02x, %02x (%s:%x)\n", offset, data & 0xff, space.device().tag(), space.device().safe_pc());
			}
		}
	}
	if(offset >= 0x20)
		iod_w(space, offset & 0x1f, data, mem_mask);
}

// 315-5242

READ16_MEMBER( segas24_state::sys16_paletteram_r )
{
	return m_generic_paletteram_16[offset];
}

WRITE16_MEMBER( segas24_state::sys16_paletteram_w )
{
	int r, g, b;
	COMBINE_DATA (m_generic_paletteram_16 + offset);
	data = m_generic_paletteram_16[offset];

	r = (data & 0x00f) << 4;
	if(data & 0x1000)
		r |= 8;

	g = data & 0x0f0;
	if(data & 0x2000)
		g |= 8;

	b = (data & 0xf00) >> 4;
	if(data & 0x4000)
		b |= 8;

	r |= r >> 5;
	g |= g >> 5;
	b |= b >> 5;

	m_palette->set_pen_color(offset, rgb_t(r, g, b));

	if(data & 0x8000) {
		r = 255-0.6*(255-r);
		g = 255-0.6*(255-g);
		b = 255-0.6*(255-b);
	} else {
		r = 0.6*r;
		g = 0.6*g;
		b = 0.6*b;
	}
	m_palette->set_pen_color(offset+m_palette->entries()/2, rgb_t(r, g, b));
}


/*
CPU1:
00-03 ROM (1)
04-07 ROM
08-0f ramlo (2)
10-13 ROM
14-17 ROM
f0-f3 ramprg (3)
fc-ff ramhi (4)

CPU2:
00-03 ramprg
04-07 ROM
08-0f ramlo
10-13 ROM
14-17 ROM
f0-f3 ramprg
fc-ff ramhi
*/

/*************************************
 *
 *  CPU 1 memory handlers
 *
 *************************************/

/*
 Both CPUs have the same memory map, except for the first 512K where
 CPU A has ROM and CPU B has work RAM mapped there.

 The memory map can be broken down into the following regions:

 000000-07FFFF : BIOS ROM (for CPU A only)
                 CPU B work RAM (for CPU B only)
 080000-0FFFFF : CPU A work RAM
 100000-1FFFFF : BIOS ROM
 200000-3FFFFF : Tilemap generator
 400000-5FFFFF : Mixer chip
 600000-7FFFFF : Sprite generator
 800000-9FFFFF : I/O chip
 A00000-AFFFFF : Timer and interrupt controller chip
 B00000-EFFFFF : Extra chip selects for expansion
 F00000-F7FFFF : CPU B work RAM
 F80000-FFFFFF : CPU A work RAM

 The BIOS ROM mirror at $100000 is common to both CPUs.
*/


static ADDRESS_MAP_START( system24_cpu1_map, AS_PROGRAM, 16, segas24_state )
	AM_RANGE(0x000000, 0x03ffff) AM_MIRROR(0x040000) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x080000, 0x0bffff) AM_MIRROR(0x040000) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x100000, 0x13ffff) AM_MIRROR(0x0c0000) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x200000, 0x20ffff) AM_MIRROR(0x110000) AM_DEVREADWRITE("tile", segas24_tile, tile_r, tile_w)
	AM_RANGE(0x220000, 0x220001) AM_MIRROR(0x11fffe) AM_WRITENOP        /* Horizontal split position (ABSEL) */
	AM_RANGE(0x240000, 0x240001) AM_MIRROR(0x11fffe) AM_WRITENOP        /* Scanline trigger position (XHOUT) */
	AM_RANGE(0x260000, 0x260001) AM_MIRROR(0x10fffe) AM_WRITENOP        /* Frame trigger position (XVOUT) */
	AM_RANGE(0x270000, 0x270001) AM_MIRROR(0x10fffe) AM_WRITENOP        /* Synchronization mode */
	AM_RANGE(0x280000, 0x29ffff) AM_MIRROR(0x160000) AM_DEVREADWRITE("tile", segas24_tile, char_r, char_w)
	AM_RANGE(0x400000, 0x403fff) AM_MIRROR(0x1f8000) AM_READWRITE(sys16_paletteram_r, sys16_paletteram_w) AM_SHARE("paletteram")
	AM_RANGE(0x404000, 0x40401f) AM_MIRROR(0x1fbfe0) AM_DEVREADWRITE("mixer", segas24_mixer, read, write)
	AM_RANGE(0x600000, 0x63ffff) AM_MIRROR(0x180000) AM_DEVREADWRITE("sprite", segas24_sprite, read, write)
	AM_RANGE(0x800000, 0x80007f) AM_MIRROR(0x1ffe00) AM_READWRITE(sys16_io_r, sys16_io_w)
	AM_RANGE(0x800100, 0x800103) AM_MIRROR(0x1ffe00) AM_DEVREADWRITE8("ymsnd", ym2151_device, read, write, 0x00ff)
	AM_RANGE(0xa00000, 0xa00007) AM_MIRROR(0x0ffff8) AM_READWRITE(irq_r, irq_w)
	AM_RANGE(0xb00000, 0xb00007) AM_MIRROR(0x07fff0) AM_READWRITE(fdc_r, fdc_w)
	AM_RANGE(0xb00008, 0xb0000f) AM_MIRROR(0x07fff0) AM_READWRITE(fdc_status_r, fdc_ctrl_w)
	AM_RANGE(0xb80000, 0xbbffff) AM_ROMBANK("bank1")
	AM_RANGE(0xbc0000, 0xbc0001) AM_MIRROR(0x03fff8) AM_READWRITE(curbank_r, curbank_w)
	AM_RANGE(0xbc0002, 0xbc0003) AM_MIRROR(0x03fff8) AM_READWRITE8(frc_mode_r, frc_mode_w,0x00ff)
	AM_RANGE(0xbc0004, 0xbc0005) AM_MIRROR(0x03fff8) AM_READWRITE8(frc_r, frc_w,0x00ff)
	AM_RANGE(0xbc0006, 0xbc0007) AM_MIRROR(0x03fff8) AM_READWRITE(mlatch_r, mlatch_w)
	AM_RANGE(0xc00000, 0xc00011) AM_MIRROR(0x07ffe0) AM_READWRITE(hotrod3_ctrl_r, hotrod3_ctrl_w)
	AM_RANGE(0xc80000, 0xcbffff) AM_ROMBANK("bank2")
	AM_RANGE(0xcc0000, 0xcc0001) AM_MIRROR(0x03fff8) AM_READWRITE(curbank_r, curbank_w)
	AM_RANGE(0xcc0002, 0xcc0003) AM_MIRROR(0x03fff8) AM_READWRITE8(frc_mode_r, frc_mode_w,0x00ff)
	AM_RANGE(0xcc0004, 0xcc0005) AM_MIRROR(0x03fff8) AM_READWRITE8(frc_r, frc_w,0x00ff)
	AM_RANGE(0xcc0006, 0xcc0007) AM_MIRROR(0x03fff8) AM_READWRITE(mlatch_r, mlatch_w)
	AM_RANGE(0xf00000, 0xf3ffff) AM_MIRROR(0x040000) AM_RAM AM_SHARE("subcpu")
	AM_RANGE(0xf80000, 0xfbffff) AM_MIRROR(0x040000) AM_RAM AM_SHARE("share1")
ADDRESS_MAP_END



/*************************************
 *
 *  CPU 2 memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( system24_cpu2_map, AS_PROGRAM, 16, segas24_state )
	AM_RANGE(0x000000, 0x03ffff) AM_MIRROR(0x040000) AM_RAM AM_SHARE("subcpu")
	AM_RANGE(0x080000, 0x0bffff) AM_MIRROR(0x040000) AM_RAM AM_SHARE("share1")
	AM_RANGE(0x100000, 0x13ffff) AM_MIRROR(0x0c0000) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x200000, 0x20ffff) AM_MIRROR(0x110000) AM_DEVREADWRITE("tile", segas24_tile, tile_r, tile_w)
	AM_RANGE(0x220000, 0x220001) AM_MIRROR(0x11fffe) AM_WRITENOP        /* Horizontal split position (ABSEL) */
	AM_RANGE(0x240000, 0x240001) AM_MIRROR(0x11fffe) AM_WRITENOP        /* Scanline trigger position (XHOUT) */
	AM_RANGE(0x260000, 0x260001) AM_MIRROR(0x10fffe) AM_WRITENOP        /* Frame trigger position (XVOUT) */
	AM_RANGE(0x270000, 0x270001) AM_MIRROR(0x10fffe) AM_WRITENOP        /* Synchronization mode */
	AM_RANGE(0x280000, 0x29ffff) AM_MIRROR(0x160000) AM_DEVREADWRITE("tile", segas24_tile, char_r, char_w)
	AM_RANGE(0x400000, 0x403fff) AM_MIRROR(0x1f8000) AM_READWRITE(sys16_paletteram_r, sys16_paletteram_w) AM_SHARE("paletteram")
	AM_RANGE(0x404000, 0x40401f) AM_MIRROR(0x1fbfe0) AM_DEVREADWRITE("mixer", segas24_mixer, read, write)
	AM_RANGE(0x600000, 0x63ffff) AM_MIRROR(0x180000) AM_DEVREADWRITE("sprite", segas24_sprite, read, write)
	AM_RANGE(0x800000, 0x80007f) AM_MIRROR(0x1ffe00) AM_READWRITE(sys16_io_r, sys16_io_w)
	AM_RANGE(0x800100, 0x800103) AM_MIRROR(0x1ffe00) AM_DEVREADWRITE8("ymsnd", ym2151_device, read, write, 0x00ff)
	AM_RANGE(0xa00000, 0xa00007) AM_MIRROR(0x0ffff8) AM_READWRITE(irq_r, irq_w)
	AM_RANGE(0xb00000, 0xb00007) AM_MIRROR(0x07fff0) AM_READWRITE(fdc_r, fdc_w)
	AM_RANGE(0xb00008, 0xb0000f) AM_MIRROR(0x07fff0) AM_READWRITE(fdc_status_r, fdc_ctrl_w)
	AM_RANGE(0xb80000, 0xbbffff) AM_ROMBANK("bank1")
	AM_RANGE(0xbc0000, 0xbc0001) AM_MIRROR(0x03fff8) AM_READWRITE(curbank_r, curbank_w)
	AM_RANGE(0xbc0002, 0xbc0003) AM_MIRROR(0x03fff8) AM_READWRITE8(frc_mode_r, frc_mode_w,0x00ff)
	AM_RANGE(0xbc0004, 0xbc0005) AM_MIRROR(0x03fff8) AM_READWRITE8(frc_r, frc_w,0x00ff)
	AM_RANGE(0xbc0006, 0xbc0007) AM_MIRROR(0x03fff8) AM_READWRITE(mlatch_r, mlatch_w)
	AM_RANGE(0xc00000, 0xc00011) AM_MIRROR(0x07ffe0) AM_READWRITE(hotrod3_ctrl_r, hotrod3_ctrl_w)
	AM_RANGE(0xc80000, 0xcbffff) AM_ROMBANK("bank2")
	AM_RANGE(0xcc0000, 0xcc0001) AM_MIRROR(0x03fff8) AM_READWRITE(curbank_r, curbank_w)
	AM_RANGE(0xcc0002, 0xcc0003) AM_MIRROR(0x03fff8) AM_READWRITE8(frc_mode_r, frc_mode_w,0x00ff)
	AM_RANGE(0xcc0004, 0xcc0005) AM_MIRROR(0x03fff8) AM_READWRITE8(frc_r, frc_w,0x00ff)
	AM_RANGE(0xcc0006, 0xcc0007) AM_MIRROR(0x03fff8) AM_READWRITE(mlatch_r, mlatch_w)
	AM_RANGE(0xf00000, 0xf3ffff) AM_MIRROR(0x040000) AM_RAM AM_SHARE("subcpu")
	AM_RANGE(0xf80000, 0xfbffff) AM_MIRROR(0x040000) AM_RAM AM_SHARE("share1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 16, segas24_state )
	AM_RANGE(0x00000, 0xfffff) AM_ROMBANK("fd1094_decrypted_opcodes")
ADDRESS_MAP_END

/*************************************
 *
 *  Generic driver initialization
 *
 *************************************/

void segas24_state::machine_start()
{
	if (track_size)
		machine().device<nvram_device>("floppy_nvram")->set_base(memregion("floppy")->base(), 2*track_size);

	UINT8 *usr1 = memregion("romboard")->base();
	if (usr1)
	{
		membank("bank1")->configure_entries(0, 16, usr1, 0x40000);
		membank("bank2")->configure_entries(0, 16, usr1, 0x40000);
	}

	vtile = machine().device<segas24_tile>("tile");
	vsprite = machine().device<segas24_sprite>("sprite");
	vmixer = machine().device<segas24_mixer>("mixer");
}

void segas24_state::machine_reset()
{
	m_subcpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	prev_resetcontrol = resetcontrol = 0x06;
	fdc_init();
	curbank = 0;
	reset_bank();
	irq_init();
	mlatch = 0x00;
	frc_mode = 0;
	frc_cnt_timer = machine().device<timer_device>("frc_timer");
	frc_cnt_timer->reset();
}

/*************************************
 *
 *  Generic port definitions
 *
 *************************************/

static INPUT_PORTS_START( system24_generic )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) //enabled with "Separate"
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE3 ) //enabled with "Separate"
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)

	PORT_START("P3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 )

	PORT_START("COINAGE")
	SEGA_COINAGE_LOC(SW1)

	PORT_START("DSW")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW2:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW2:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END


/*************************************
 *
 *  Game-specific port definitions
 *
 *************************************/

static INPUT_PORTS_START( hotrod )
	PORT_INCLUDE( system24_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN6 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN7 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN8 )

	PORT_MODIFY("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE4 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW")
	//"SW2:1" unused
	//"SW2:2" unused
	//"SW2:3" unused
	//"SW2:4" unused
	//"SW2:5" unused
	PORT_DIPNAME( 0x20, 0x20, "Start Credit" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x40, 0x40, "Coin Chute" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, "Separate" )
	PORT_DIPSETTING(    0x00, "Common" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DIAL1")
	PORT_BIT( 0xfff, 0x000, IPT_DIAL ) PORT_MINMAX(0x000,0xfff) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("DIAL2")
	PORT_BIT( 0xfff, 0x000, IPT_DIAL ) PORT_MINMAX(0x000,0xfff) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("DIAL3")
	PORT_BIT( 0xfff, 0x000, IPT_DIAL ) PORT_MINMAX(0x000,0xfff) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(3)

	PORT_START("DIAL4")
	PORT_BIT( 0xfff, 0x000, IPT_DIAL ) PORT_MINMAX(0x000,0xfff) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(4)

	PORT_START("PEDAL1")
	PORT_BIT( 0xff, 0x01, IPT_PEDAL ) PORT_MINMAX(0x01,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("PEDAL2")
	PORT_BIT( 0xff, 0x01, IPT_PEDAL ) PORT_MINMAX(0x01,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START("PEDAL3")
	PORT_BIT( 0xff, 0x01, IPT_PEDAL ) PORT_MINMAX(0x01,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(3)

	PORT_START("PEDAL4")
	PORT_BIT( 0xff, 0x01, IPT_PEDAL ) PORT_MINMAX(0x01,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(4)
INPUT_PORTS_END

static INPUT_PORTS_START( hotrodj )
	PORT_INCLUDE( hotrod )

	PORT_MODIFY("DSW")
	//"SW2:1" unused
	//"SW2:2" unused
	PORT_DIPNAME( 0x04, 0x04, "Start Credit" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x08, 0x08, "Play Mode" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "4 Player" )
	PORT_DIPSETTING(    0x00, "2 Player" )
	PORT_DIPNAME( 0x10, 0x10, "Game Style" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "4 Sides" )
	PORT_DIPSETTING(    0x00, "2 Sides" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Japanese ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
	//"SW2:7" not divert from "hotrod"
	//"SW2:8" not divert from "hotrod"
INPUT_PORTS_END

static INPUT_PORTS_START( bnzabros )
	PORT_INCLUDE( system24_generic )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Start Credit" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x04, 0x04, "Coin Chute" ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "Common" )
	PORT_DIPSETTING(    0x00, "Individual" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x80, "4" )
INPUT_PORTS_END

static INPUT_PORTS_START( crkdown )
	PORT_INCLUDE( system24_generic )

	PORT_MODIFY("DSW")
	//"SW2:1" unused
	//"SW2:2" unused
	//"SW2:3" unused
	//"SW2:4" unused
	//"SW2:5" unused
	//"SW2:6" unused
	PORT_DIPNAME( 0x40, 0x40, "Coin Chute" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, "Common" ) // Japanese manual says "Single"
	PORT_DIPSETTING(    0x00, "Separate" ) // Japanese manual says "Twin"
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( roughrac )
	PORT_INCLUDE( system24_generic )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Start Credit" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Game Instruction" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x40, "Start Money" ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, "15000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPNAME( 0x80, 0x80, "Start Intro" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, "10" )
	PORT_DIPSETTING(    0x00, "15" )

	PORT_START("DIAL1")
	PORT_BIT( 0xfff, 0x000, IPT_DIAL ) PORT_MINMAX(0x000,0xfff) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("DIAL2")
	PORT_BIT( 0xfff, 0x000, IPT_DIAL ) PORT_MINMAX(0x000,0xfff) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( sspirits )
	PORT_INCLUDE( system24_generic )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, "500K" )
	PORT_DIPSETTING(    0x30, "600K" )
	PORT_DIPSETTING(    0x10, "800K" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( qgh )
	PORT_INCLUDE( system24_generic )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x30, "5" )
	//"SW2:7" unused
	//"SW2:8" unused
INPUT_PORTS_END

static INPUT_PORTS_START( qsww )
	PORT_INCLUDE( system24_generic )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Start Credit" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	//"SW2:4" unused
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x30, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( dcclub ) /* In the Japan set missing angle input */
	PORT_INCLUDE( system24_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Start Credit" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Timing Meter" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPNAME( 0x10, 0x10, "Initial Balls" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPNAME( 0x20, 0x20, "Balls Limit" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START("PADDLE")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE ) PORT_MINMAX(0x00,0x8f) PORT_SENSITIVITY(64) PORT_KEYDELTA(64) PORT_PLAYER(1)
INPUT_PORTS_END

static INPUT_PORTS_START( sgmast )
	PORT_INCLUDE( system24_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	/* For select the power shot rotate the stick from up-left (max power) to up (minimum power) and relese */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Start Credit" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Gameplay" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "1 Hole 1 Credit" )
	PORT_DIPSETTING(    0x00, "Lose Ball Game Over" )
	//"SW2:5" unused
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:6,7,8") // MeterWidth WindVelocity MeterSpeed
	PORT_DIPSETTING(    0x80, DEF_STR( Easiest ) )                                  //        64%          30%        50%
	PORT_DIPSETTING(    0xa0, DEF_STR( Easier ) )                                   //        64%          50%        75%
	PORT_DIPSETTING(    0xc0, DEF_STR( Easy ) )                                     //       100%          75%        75%
	PORT_DIPSETTING(    0xe0, DEF_STR( Normal ) )                                   //       100%         100%       100%
	PORT_DIPSETTING(    0x60, "Little Hard" )                                       //       118%          75%       100%
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )                                     //       118%         100%       100%
	PORT_DIPSETTING(    0x20, DEF_STR( Harder ) )                                   //       136%          75%       100%
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )                                  //       136%         100%       100%
INPUT_PORTS_END

static INPUT_PORTS_START( sgmastj )
	PORT_INCLUDE( sgmast )

	PORT_MODIFY("DSW")
	//"SW2:1" not divert from "sgmast"
	//"SW2:2" not divert from "sgmast"
	//"SW2:3" not divert from "sgmast"
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW2:4" )
	//"SW2:5" unused
	//"SW2:6" not divert from "sgmast"
	//"SW2:7" not divert from "sgmast"
	//"SW2:8" not divert from "sgmast"

	PORT_START("DIAL1")
	PORT_BIT( 0xfff, 0x000, IPT_DIAL ) PORT_MINMAX(0x000,0xfff) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( quizmeku )
	PORT_INCLUDE( system24_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)

	PORT_MODIFY("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)

	PORT_MODIFY("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Play Mode" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "2 Player" )
	PORT_DIPSETTING(    0x00, "4 Player" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, "Answer Counts" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "Every" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( qrouka )
	PORT_INCLUDE( system24_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)

	PORT_MODIFY("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)

	PORT_MODIFY("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN4 ) PORT_CONDITION("DSW",0x08,EQUALS,0x00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_CONDITION("DSW",0x08,EQUALS,0x00)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CONDITION("DSW",0x08,EQUALS,0x00)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CONDITION("DSW",0x08,EQUALS,0x00)
	/* alt coin mode */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CONDITION("DSW",0x08,EQUALS,0x08)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_CONDITION("DSW",0x08,EQUALS,0x08)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("DSW",0x08,EQUALS,0x08)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CONDITION("DSW",0x08,EQUALS,0x08)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Play Mode" ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "2 Player" )
	PORT_DIPSETTING(    0x00, "4 Player" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	//"SW2:3" unused
	PORT_DIPNAME( 0x08, 0x08, "Coin Chute" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "Common" )
	PORT_DIPSETTING(    0x00, "Separate" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x30, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( mahmajn )
	PORT_INCLUDE( system24_generic )

	PORT_MODIFY("P1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("P3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("MJ0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("MJ1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("MJ2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("MJ3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("MJ4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("MJ5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Start Score" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, "2000" )
	PORT_DIPSETTING(    0x02, "3000" )
	PORT_DIPNAME( 0x0c, 0x0c, "Difficulty (computer)" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x30, "Difficulty (player)" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	//"SW2:7" unused
	//"SW2:8" unused
INPUT_PORTS_END

static INPUT_PORTS_START( gground )
	PORT_INCLUDE( system24_generic )

	PORT_MODIFY("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE4 ) //enabled with "Separate"
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)

	PORT_MODIFY("DSW")
	// Here is known problem.
	// Service mode works as I hoped.
	// But game mode doesn't. It works as all dipswitches are On.
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:3,4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easier ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x18, "Little Easy" )
	PORT_DIPSETTING(    0x1c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x0c, "Little Hard" )
	PORT_DIPSETTING(    0x14, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x60, 0x60, "Time Limit Per Stage" ) PORT_DIPLOCATION("SW2:6,7")
	PORT_DIPSETTING(    0x20, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, "Clock Of Time Limit" ) PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, "1.00 sec" )
	PORT_DIPSETTING(    0x00, "0.80 sec" )
INPUT_PORTS_END


/*************************************
 *
 *  Generic machine drivers
 *
 *************************************/

static MACHINE_CONFIG_START( system24, segas24_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, MASTER_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(system24_cpu1_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", segas24_state, irq_vbl, "screen", 0, 1)

	MCFG_CPU_ADD("subcpu", M68000, MASTER_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(system24_cpu2_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_TIMER_DRIVER_ADD("irq_timer", segas24_state, irq_timer_cb)
	MCFG_TIMER_DRIVER_ADD("irq_timer_clear", segas24_state, irq_timer_clear_cb)
	MCFG_TIMER_ADD_NONE("frc_timer")
	MCFG_TIMER_DRIVER_ADD_PERIODIC("irq_frc", segas24_state, irq_frc_cb, attotime::from_hz(FRC_CLOCK_MODE1))

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", empty)
	MCFG_S24TILE_DEVICE_ADD("tile", 0xfff)
	MCFG_S24TILE_DEVICE_GFXDECODE("gfxdecode")
	MCFG_S24TILE_DEVICE_PALETTE("palette")
	MCFG_S24SPRITE_DEVICE_ADD("sprite")
	MCFG_S24MIXER_DEVICE_ADD("mixer")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK)
	MCFG_SCREEN_RAW_PARAMS(VIDEO_CLOCK/2, 656, 0/*+69*/, 496/*+69*/, 424, 0/*+25*/, 384/*+25*/)
	MCFG_SCREEN_UPDATE_DRIVER(segas24_state, screen_update_system24)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 8192*2)

	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_YM2151_ADD("ymsnd", 4000000)
	MCFG_YM2151_IRQ_HANDLER(WRITELINE(segas24_state,irq_ym))
	MCFG_SOUND_ROUTE(0, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(1, "rspeaker", 0.50)

	MCFG_DAC_ADD("dac")
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.50)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.50)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( system24_floppy, system24 )
	MCFG_NVRAM_ADD_NO_FILL("floppy_nvram")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( system24_floppy_fd1094, system24_floppy )
	MCFG_CPU_REPLACE("subcpu", FD1094, MASTER_CLOCK/2)
	MCFG_CPU_PROGRAM_MAP(system24_cpu2_map)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( hotrod )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-11339.ic2", 0x000000, 0x20000, CRC(75130e73) SHA1(e079739f4a3da3807aac570442c5afef1a7d7b0e) )
	ROM_LOAD16_BYTE( "epr-11338.ic1", 0x000001, 0x20000, CRC(7d4a7ff3) SHA1(3d3af04d990d232ba0a8fe155de59bc632a0a461) )

	ROM_REGION( 0x1d6000, "floppy", 0)
	ROM_LOAD( "ds3-5000-01d_3p_turbo.img", 0x000000, 0x1d6000, CRC(842006fd) SHA1(d5432f58c0fb39f2bf62786a0d842bdd469ab2cb) )
ROM_END

ROM_START( hotroda )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-11339.ic2", 0x000000, 0x20000, CRC(75130e73) SHA1(e079739f4a3da3807aac570442c5afef1a7d7b0e) )
	ROM_LOAD16_BYTE( "epr-11338.ic1", 0x000001, 0x20000, CRC(7d4a7ff3) SHA1(3d3af04d990d232ba0a8fe155de59bc632a0a461) )

	ROM_REGION( 0x1d6000, "floppy", 0)
	ROM_LOAD( "ds3-5000-01d.img", 0x000000, 0x1d6000, CRC(e25c6b63) SHA1(fbf86d2ebccd8053b990939f63f5497907d18321) )
ROM_END

ROM_START( hotrodja )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-11339.ic2", 0x000000, 0x20000, CRC(75130e73) SHA1(e079739f4a3da3807aac570442c5afef1a7d7b0e) )
	ROM_LOAD16_BYTE( "epr-11338.ic1", 0x000001, 0x20000, CRC(7d4a7ff3) SHA1(3d3af04d990d232ba0a8fe155de59bc632a0a461) )

	ROM_REGION( 0x1d6000, "floppy", 0)
	ROM_LOAD( "ds3-5000-01a-rev-b.img", 0x000000, 0x1d6000, CRC(c18f6dca) SHA1(6f2b5a9567a340324a5f3fb57a3b744de0924a23) )
ROM_END

ROM_START( hotrodj )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-11339.ic2", 0x000000, 0x20000, CRC(75130e73) SHA1(e079739f4a3da3807aac570442c5afef1a7d7b0e) )
	ROM_LOAD16_BYTE( "epr-11338.ic1", 0x000001, 0x20000, CRC(7d4a7ff3) SHA1(3d3af04d990d232ba0a8fe155de59bc632a0a461) )

	ROM_REGION( 0x1d6000, "floppy", 0)
	ROM_LOAD( "ds3-5000-01a-rev-c.img", 0x000000, 0x1d6000, CRC(852f9b5f) SHA1(159e161f55beed0f90cce8a73b0aeb4564d6af90) )
ROM_END

ROM_START( qgh )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "16900b", 0x000000, 0x20000, CRC(20d7b7d1) SHA1(345b228c27e5f2fef9a2b8b5f619c59450a070f8) )
	ROM_LOAD16_BYTE( "16899b", 0x000001, 0x20000, CRC(397b3ba9) SHA1(1773212cd87dcff840f3953ec368be7e2394faf0) )

	ROM_REGION16_BE( 0x400000, "romboard", 0)
	ROM_LOAD16_BYTE( "16902a", 0x000000, 0x80000, CRC(d35b7706) SHA1(341bca0af6b6d3f326328a88cdc69c7897b83a0d) )
	ROM_LOAD16_BYTE( "16901a", 0x000001, 0x80000, CRC(ab4bcb33) SHA1(8acd73096eb485c6dc83da6adfcc47d5d0f5b7f3) )
	ROM_LOAD16_BYTE( "16904",  0x100000, 0x80000, CRC(10987c88) SHA1(66f893690565aed613427421958ebe225a20ad0f) )
	ROM_LOAD16_BYTE( "16903",  0x100001, 0x80000, CRC(c19f9e46) SHA1(f1275674a8b44957428d79402f240ca21a34f48d) )
	ROM_LOAD16_BYTE( "16906",  0x200000, 0x80000, CRC(99c6773e) SHA1(568570b607d2cbbedb39ceae5bbc479478fae4ca) )
	ROM_LOAD16_BYTE( "16905",  0x200001, 0x80000, CRC(3922bbe3) SHA1(4378ca900f96138b5e33265ddac56af7b45afbc8) )
	ROM_LOAD16_BYTE( "16908",  0x300000, 0x80000, CRC(407ec20f) SHA1(c8a909d8e9ba024a37a5af6b7920fe7023f80d49) )
	ROM_LOAD16_BYTE( "16907",  0x300001, 0x80000, CRC(734b0a82) SHA1(d3fb31c55e79b99040beb7c49faaf2e17b95aa87) )
ROM_END

ROM_START( qrouka )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "14485", 0x000000, 0x20000, CRC(fc0085f9) SHA1(0250d1e17e19b541b85198ec4207e55bfbd5c32e) )
	ROM_LOAD16_BYTE( "14484", 0x000001, 0x20000, CRC(f51c641c) SHA1(3f2fd0be7d58c75e88565393da5e810655413b53) )

	ROM_REGION16_BE( 0x400000, "romboard", 0)
	ROM_LOAD16_BYTE( "14482", 0x000000, 0x80000, CRC(7a13dd97) SHA1(bfe9950d2cd41f3f866520923c1ed7b8da1990ec) )
	ROM_LOAD16_BYTE( "14483", 0x100000, 0x80000, CRC(f3eb51a0) SHA1(6904830ff5e7aa5f016e115572fb6da678896ede) )
ROM_END

ROM_START( mahmajn )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr14813.bin", 0x000000, 0x20000, CRC(ea38cf4b) SHA1(118ab2e0ae20a4db5e619945dfbb3f200de3979c) )
	ROM_LOAD16_BYTE( "epr14812.bin", 0x000001, 0x20000, CRC(5a3cb4a7) SHA1(c0f21282140e8e6e927664f5f2b90525ae0207e9) )

	ROM_REGION16_BE( 0x400000, "romboard", 0)
	ROM_LOAD16_BYTE( "mpr14820.bin", 0x000000, 0x80000, CRC(8d2a03d3) SHA1(b3339bcd101bcfe042e2a1cfdc8baef0a86624df) )
	ROM_LOAD16_BYTE( "mpr14819.bin", 0x000001, 0x80000, CRC(e84c4827) SHA1(54741295e1bdca7d0c78eb795a68b92212d43b2e) )
	ROM_LOAD16_BYTE( "mpr14822.bin", 0x100000, 0x80000, CRC(7c3dcc51) SHA1(a199c2c71cda44a2c8755074c1007d83c8d45d2d) )
	ROM_LOAD16_BYTE( "mpr14821.bin", 0x100001, 0x80000, CRC(bd8dc543) SHA1(fd50b14fa73307a62dc0b522cfedb8b3332c407e) )
	ROM_LOAD16_BYTE( "mpr14824.bin", 0x200000, 0x80000, CRC(38311933) SHA1(237d9a8ffe14ba9ec371bb571d7c9e74a93fe1f3) )
	ROM_LOAD16_BYTE( "mpr14823.bin", 0x200001, 0x80000, CRC(4c8d4550) SHA1(be8717d4080ce932fc8272ebe54e2b0a60b20edd) )
	ROM_LOAD16_BYTE( "mpr14826.bin", 0x300000, 0x80000, CRC(c31b8805) SHA1(b446388c83af2e14300b0c4248470d3a8c504f2c) )
	ROM_LOAD16_BYTE( "mpr14825.bin", 0x300001, 0x80000, CRC(191080a1) SHA1(407c1c5fa4c76732e8a444860094542e90a1e8e8) )
ROM_END

ROM_START( mahmajn2 )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr16799.bin", 0x000000, 0x20000, CRC(3a34cf75) SHA1(d22bf6334668af29167cf4244d18f9cd2e7ff7d6) )
	ROM_LOAD16_BYTE( "epr16798.bin", 0x000001, 0x20000, CRC(662923fa) SHA1(dcd3964d899d3f34dab22ffcd1a5af895804fae1) )

	ROM_REGION16_BE( 0x400000, "romboard", 0)
	ROM_LOAD16_BYTE( "mpr16801.bin", 0x000000, 0x80000, CRC(74855a17) SHA1(d2d8e7da7b261e7cb64605284d2c78fbd1465b69) )
	ROM_LOAD16_BYTE( "mpr16800.bin", 0x000001, 0x80000, CRC(6dbc1e02) SHA1(cce5734243ff171759cecb5c05c12dc743a25c1d) )
	ROM_LOAD16_BYTE( "mpr16803.bin", 0x100000, 0x80000, CRC(9b658dd6) SHA1(eaaae289a3555aa6a92f57eea964dbbf48c5c2a4) )
	ROM_LOAD16_BYTE( "mpr16802.bin", 0x100001, 0x80000, CRC(b4723225) SHA1(acb8923c7d9908b1112f8d1f2512f18236915e5d) )
	ROM_LOAD16_BYTE( "mpr16805.bin", 0x200000, 0x80000, CRC(d15528df) SHA1(bda1dd5c98867c2e7666380bca0bc7eef8022fbc) )
	ROM_LOAD16_BYTE( "mpr16804.bin", 0x200001, 0x80000, CRC(a0de08e2) SHA1(2c36b66e74b88fb076e2eaa250c6d06ee0b4ac88) )
	ROM_LOAD16_BYTE( "mpr16807.bin", 0x300000, 0x80000, CRC(816188bb) SHA1(76b2690a6156766a1af94f01f6de1209b7756b2c) )
	ROM_LOAD16_BYTE( "mpr16806.bin", 0x300001, 0x80000, CRC(54b353d3) SHA1(40632e4571b44ee215b5a1f7aab9d89c460a5c9e) )
ROM_END

ROM_START( bnzabros )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION16_BE( 0x400000, "romboard", 0)
	ROM_LOAD16_BYTE( "mpr-13188-h.2",  0x000000, 0x80000, CRC(d3802294) SHA1(7608e71e8ef398ac24dbf851994253bca5ace625) )
	ROM_LOAD16_BYTE( "mpr-13187-h.1",  0x000001, 0x80000, CRC(e3d8c5f7) SHA1(5b1e8646debee2f2ef272ddd3320b0a17192fbbe) )
	ROM_LOAD16_BYTE( "mpr-13190.4",    0x100000, 0x40000, CRC(0b4df388) SHA1(340478bba82069ab745d6d8703e6801411fd2fc4) )
	ROM_RELOAD ( 0x180000, 0x40000)
	ROM_LOAD16_BYTE( "mpr-13189.3",    0x100001, 0x40000, CRC(5ea5a2f3) SHA1(514b5446303c50aeb1d6d10d0a3f210da2577e16) )
	ROM_RELOAD ( 0x180001, 0x40000)

	ROM_REGION( 0x1c2000, "floppy", 0)
	ROM_LOAD( "ds3-5000-07d.img", 0x000000, 0x1c2000, CRC(2e70251f) SHA1(1c2616dfa5cc15e8ebf1424012f2dd66f3a001a1) ) /* Region letter needs to be verfied */
ROM_END

ROM_START( bnzabrosj )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION16_BE( 0x400000, "romboard", 0)
	ROM_LOAD16_BYTE( "mpr-13188-h.2",  0x000000, 0x80000, CRC(d3802294) SHA1(7608e71e8ef398ac24dbf851994253bca5ace625) )
	ROM_LOAD16_BYTE( "mpr-13187-h.1",  0x000001, 0x80000, CRC(e3d8c5f7) SHA1(5b1e8646debee2f2ef272ddd3320b0a17192fbbe) )
	ROM_LOAD16_BYTE( "mpr-13190.4",    0x100000, 0x40000, CRC(0b4df388) SHA1(340478bba82069ab745d6d8703e6801411fd2fc4) )
	ROM_RELOAD ( 0x180000, 0x40000)
	ROM_LOAD16_BYTE( "mpr-13189.3",    0x100001, 0x40000, CRC(5ea5a2f3) SHA1(514b5446303c50aeb1d6d10d0a3f210da2577e16) )
	ROM_RELOAD ( 0x180001, 0x40000)

	ROM_REGION( 0x1c2000, "floppy", 0)
	ROM_LOAD( "ds3-5000-07b.img", 0x000000, 0x1c2000, CRC(efa7f2a7) SHA1(eb905bff88fa40324fb92b91ac8a5878648c26e5) )
ROM_END

ROM_START( quizmeku ) // Quiz Mekuromeku Story
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr15343.ic2", 0x000000, 0x20000, CRC(c72399a7) SHA1(bfbf0079ea63f89bca4ce9081aed5d5c1d9d169a) )
	ROM_LOAD16_BYTE( "epr15342.ic1", 0x000001, 0x20000, CRC(0968ac84) SHA1(4e1170ac123adaec32819754b5075531ff1925fe) )

	ROM_REGION16_BE( 0x400000, "romboard", 0)
	ROM_LOAD16_BYTE( "epr15345.ic5", 0x000000, 0x80000, CRC(88030b5d) SHA1(d2feeedb9a64c3dc8dd25716209f945d12fa9b53) )
	ROM_LOAD16_BYTE( "epr15344.ic4", 0x000001, 0x80000, CRC(dd11b382) SHA1(2b0f49fb307a9aba0f295de64782ee095c557170) )
	ROM_LOAD16_BYTE( "mpr15347.ic7", 0x100000, 0x80000, CRC(0472677b) SHA1(93ae57a2817b6b54c99814fca28ef51f7ff5e559) )
	ROM_LOAD16_BYTE( "mpr15346.ic6", 0x100001, 0x80000, CRC(746d4d0e) SHA1(7863abe36126684772a4459d5b6f3b24670ec02b) )
	ROM_LOAD16_BYTE( "mpr15349.ic9", 0x200000, 0x80000, CRC(770eecf1) SHA1(86cc5b4a325198dc1da1446ecd8e718415b7998a) )
	ROM_LOAD16_BYTE( "mpr15348.ic8", 0x200001, 0x80000, CRC(7666e960) SHA1(f3f88d5c8318301a8c73141de60292f8349ac0ce) )
ROM_END

ROM_START( sspirits )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION( 0x1c2000, "floppy", 0)
	ROM_LOAD( "ds3-5000-02-.img", 0x000000, 0x1c2000, CRC(179b98e9) SHA1(f6fc52c599c336d5c6f7aa199515268b4b3218a8) )
ROM_END

ROM_START( sspiritj )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-11339.ic2", 0x000000, 0x20000, CRC(75130e73) SHA1(e079739f4a3da3807aac570442c5afef1a7d7b0e) )
	ROM_LOAD16_BYTE( "epr-11338.ic1", 0x000001, 0x20000, CRC(7d4a7ff3) SHA1(3d3af04d990d232ba0a8fe155de59bc632a0a461) )

	ROM_REGION( 0x1d6000, "floppy", 0)
	ROM_LOAD( "ds3-5000-02-rev-a.img", 0x000000, 0x1d6000, CRC(0385470f) SHA1(62c1bfe3a88b2dee44629809e08b4b8a5770eaab)  )
ROM_END

ROM_START( sspirtfc )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION( 0x2000, "subcpu:key", 0 )   /* decryption key */
	ROM_LOAD( "317-0058-02c.key", 0x0000, 0x2000,  CRC(ebae170e) SHA1(b6d1e1b6943a35b96e98e426ecb39bb5a42fb643) )

	ROM_REGION( 0x1c2000, "floppy", 0)
	ROM_LOAD( "ds3-5000-02c.img", 0x000000, 0x1c2000, NO_DUMP )
ROM_END

ROM_START( sgmast )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION( 0x2000, "subcpu:key", 0 )   /* decryption key */
	// reconstructed key; some of the RNG-independent bits could be incorrect
	ROM_LOAD( "317-0058-05d.key", 0x0000, 0x2000, BAD_DUMP  CRC(c779738d) SHA1(f65355c20fbcb22781816d24633a509f13bec170) )

	ROM_REGION( 0x1c2000, "floppy", 0)
	ROM_LOAD( "ds3-5000-05d.img",      0x000000, 0x1c2000, CRC(e9a69f93) SHA1(dc15e47ed78373688c1fab72a9605528068ad702) )
ROM_END

ROM_START( sgmastc )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION( 0x2000, "subcpu:key", 0 )   /* decryption key */
	ROM_LOAD( "317-0058-05c.key", 0x0000, 0x2000, CRC(ae0eabe5) SHA1(692d7565bf9c5b32cc80bb4bd88c9193aa04cbb0) )

	ROM_REGION( 0x1c2000, "floppy", 0)
	ROM_LOAD( "ds3-5000-05c.img", 0x000000, 0x1c2000, CRC(63a6ef3a) SHA1(f39fe0bf8930de994b1a77e0ba787d249d73c5e5) )
ROM_END

ROM_START( sgmastj )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION( 0x2000, "subcpu:key", 0 )   /* decryption key */
	ROM_LOAD( "317-0058-05b.key", 0x0000, 0x2000, CRC(adc0c83b) SHA1(2328d82d5057062eeb0072fd57f0422218cf24fc) )

	ROM_REGION( 0x1c2000, "floppy", 0)
	ROM_LOAD( "ds3-5000-05b.img", 0x000000, 0x1c2000,  CRC(a136668c) SHA1(7203f9d11023605a0a4b52a4be330785c8f7b623) )
ROM_END

ROM_START( qsww )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION( 0x2000, "subcpu:key", 0 )   /* decryption key */
	ROM_LOAD( "317-0058-08b.key", 0x0000, 0x2000,  CRC(fe0a336a) SHA1(f7a5b2c1a057d0bb8c1ae0453c58aa8f5fb731b9) )

	ROM_REGION( 0x1c2000, "floppy", 0)
	ROM_LOAD( "ds3-5000-08b.img", 0x000000, 0x1c2000, CRC(5a886d38) SHA1(2e974a9ffe3534da4fb117c579b8b0e61a63542c) )
ROM_END

ROM_START( gground )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION( 0x2000, "subcpu:key", 0 )   /* decryption key */
	ROM_LOAD( "317-0058-03d.key", 0x0000, 0x2000,  CRC(e1785bbd) SHA1(b4bebb2829299f1c0815d6a5f317a2526b322f63) ) /* Also labeled "rev-A" but is it different? */

	ROM_REGION( 0x1c2000, "floppy", 0)
	ROM_LOAD( "ds3-5000-03d-rev-a.img", 0x000000, 0x1c2000, CRC(5c5910f2) SHA1(9ed564a03c0d4ca4a207f3ecfb7336c6cbcaa70f) )
ROM_END

ROM_START( ggroundj )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION( 0x2000, "subcpu:key", 0 )   /* decryption key */
	ROM_LOAD( "317-0058-03b.key", 0x0000, 0x2000,  CRC(84aecdba) SHA1(ceddf967359a6e76543fe1ab00be53d0a11fe1ab) )

	ROM_REGION( 0x1c2000, "floppy", 0)
	ROM_LOAD( "ds3-5000-03b.img", 0x000000, 0x1c2000, CRC(7200dac9) SHA1(07cf33bf2a0da36e3852de409959f30128cdbf77) )
ROM_END

ROM_START( crkdown )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION( 0x2000, "subcpu:key", 0 )   /* decryption key */
	ROM_LOAD( "317-0058-04c.key", 0x0000, 0x2000,  CRC(16e978cc) SHA1(0e1482b5efa93b732d4cf0990919cb3fc903dca7) )

	ROM_REGION( 0x1c2000, "floppy", 0)
	ROM_LOAD( "ds3-5000-04c.img", 0x000000, 0x1c2000, CRC(7d97ba5e) SHA1(43f98bd04d031dd435bbd1bf8e6688bc57ce9666) )
ROM_END

ROM_START( crkdownu )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION( 0x2000, "subcpu:key", 0 )   /* decryption key */
	ROM_LOAD( "317-0058-04d.key", 0x0000, 0x2000,  CRC(934ac358) SHA1(73418e22c9d201bc3fec5c63284858958c010e05) )

	ROM_REGION( 0x1c2000, "floppy", 0)
	ROM_LOAD( "ds3-5000-04d.img", 0x000000, 0x1c2000, CRC(8679032c) SHA1(887b245a70652897fbda736b60e81a123866ec12) )
ROM_END

ROM_START( crkdownj )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION( 0x2000, "subcpu:key", 0 )   /* decryption key */
	ROM_LOAD( "317-0058-04b.key", 0x0000, 0x2000,  CRC(4a99a202) SHA1(d7375f09e7246ecd60ba0e48f049e9e252af92a8) ) /* Also labeled "rev-A" but is it different? */

	ROM_REGION( 0x1c2000, "floppy", 0)
	ROM_LOAD( "ds3-5000-04b-rev-a.img", 0x000000, 0x1c2000, CRC(5daa1a9a) SHA1(ce2f07b83b607bbbdb70f1ae344c1d897d601809) )
ROM_END

ROM_START( dcclub )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr13948.bin", 0x000000, 0x20000, CRC(d6a031c8) SHA1(45b7e3cd2c7412e24f547cd4185166199d3938d5) )
	ROM_LOAD16_BYTE( "epr13947.bin", 0x000001, 0x20000, CRC(7e3cff5e) SHA1(ff8cb776d2491796feeb8892c7e644e590438945) )

	ROM_REGION16_BE( 0x400000, "romboard", 0)
	ROM_LOAD16_BYTE( "epr-15345.2",  0x000000, 0x80000, CRC(d9e120c2) SHA1(b18b76733078d8534c6f0d8950632ab51e6a10ab) )
	ROM_LOAD16_BYTE( "epr-15344.1",  0x000001, 0x80000, CRC(8f8b9f74) SHA1(de6b923118bea60197547ad016cb5d5e1a8f372b) )
	ROM_LOAD16_BYTE( "mpr-14097-t.4",0x100000, 0x80000, CRC(4bd74cae) SHA1(5aa90bd5d2b8e2338ef0fe41d1f794e8d51321e1) )
	ROM_LOAD16_BYTE( "mpr-14096-t.3",0x100001, 0x80000, CRC(38d96502) SHA1(c68b3c5c83fd0839c3f6f81189c310ec19bdf1c4) )
ROM_END

ROM_START( dcclubj )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr13948.bin", 0x000000, 0x20000, CRC(d6a031c8) SHA1(45b7e3cd2c7412e24f547cd4185166199d3938d5) )
	ROM_LOAD16_BYTE( "epr13947.bin", 0x000001, 0x20000, CRC(7e3cff5e) SHA1(ff8cb776d2491796feeb8892c7e644e590438945) )

	ROM_REGION16_BE( 0x400000, "romboard", 0)
	ROM_LOAD16_BYTE( "epr-14095a.2", 0x000000, 0x80000, CRC(88d184e9) SHA1(519f3a22e1619de9d5f13a45b85ebd249ebfa979) )
	ROM_LOAD16_BYTE( "epr-14094a.1", 0x000001, 0x80000, CRC(7dd2b7d4) SHA1(c7eaf9e2700e0c55f7e867f5cd3ffaa5aae97956) )
	ROM_LOAD16_BYTE( "mpr-14097-t.4",0x100000, 0x80000, CRC(4bd74cae) SHA1(5aa90bd5d2b8e2338ef0fe41d1f794e8d51321e1) )
	ROM_LOAD16_BYTE( "mpr-14096-t.3",0x100001, 0x80000, CRC(38d96502) SHA1(c68b3c5c83fd0839c3f6f81189c310ec19bdf1c4) )
ROM_END

ROM_START( dcclubfd )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION( 0x2000, "subcpu:key", 0 )   /* decryption key */
	ROM_LOAD( "317-0058-09d.key", 0x0000, 0x2000, CRC(a91ebffb) SHA1(70b8b4272ca456491f254d115b434bb4ce73f049) )

	ROM_REGION( 0x1c2000, "floppy", 0)
	ROM_LOAD( "ds3-5000-09d.img", 0x000000, 0x1c2000,  CRC(69870887) SHA1(e47a997c2c783bf6670ab213ebe2ee35492eba34) )
ROM_END

ROM_START( roughrac )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION( 0x2000, "subcpu:key", 0 )   /* decryption key */
	ROM_LOAD( "317-0058-06b.key",    0x000000, 0x2000, CRC(6a5bf536) SHA1(3fc3e93ce8a47d7ee86da889efad2e7eca6e2ee9) )

	ROM_REGION( 0x1c2000, "floppy", 0)
	ROM_LOAD( "ds3-5000-06b.img",    0x000000, 0x1c2000, CRC(a7fb2149) SHA1(c04266ae31700b085ab45606aed83019a563de70) )
ROM_END

/* I think the letter after the disk code is the region
           'b' = Japan
           'c' = Europe?
           'd' = US
*/


/*************************************
 *
 *  Game-specific driver inits
 *
 *************************************/

DRIVER_INIT_MEMBER(segas24_state,qgh)
{
	io_r = &segas24_state::hotrod_io_r;
	io_w = &segas24_state::hotrod_io_w;
	mlatch_table = segas24_state::qgh_mlt;
	track_size = 0;
}

DRIVER_INIT_MEMBER(segas24_state,dcclub)
{
	io_r = &segas24_state::dcclub_io_r;
	io_w = &segas24_state::hotrod_io_w;
	mlatch_table = segas24_state::dcclub_mlt;
	track_size = 0;
}

DRIVER_INIT_MEMBER(segas24_state,qrouka)
{
	io_r = &segas24_state::hotrod_io_r;
	io_w = &segas24_state::hotrod_io_w;
	mlatch_table = segas24_state::qrouka_mlt;
	track_size = 0;
}

DRIVER_INIT_MEMBER(segas24_state,quizmeku)
{
	io_r = &segas24_state::hotrod_io_r;
	io_w = &segas24_state::hotrod_io_w;
	mlatch_table = segas24_state::quizmeku_mlt;
	track_size = 0;
}

DRIVER_INIT_MEMBER(segas24_state,mahmajn)
{
	io_r = &segas24_state::mahmajn_io_r;
	io_w = &segas24_state::mahmajn_io_w;
	mlatch_table = segas24_state::mahmajn_mlt;
	track_size = 0;
	cur_input_line = 0;
}

DRIVER_INIT_MEMBER(segas24_state,mahmajn2)
{
	io_r = &segas24_state::mahmajn_io_r;
	io_w = &segas24_state::mahmajn_io_w;
	mlatch_table = segas24_state::mahmajn2_mlt;
	track_size = 0;
	cur_input_line = 0;
}

DRIVER_INIT_MEMBER(segas24_state,hotrod)
{
	io_r = &segas24_state::hotrod_io_r;
	io_w = &segas24_state::hotrod_io_w;
	mlatch_table = nullptr;

	// Sector  Size
	// 1       8192
	// 2       1024
	// 3       1024
	// 4       1024
	// 5        512
	// 6        256

	track_size = 0x2f00;
}

DRIVER_INIT_MEMBER(segas24_state,bnzabros)
{
	io_r = &segas24_state::hotrod_io_r;
	io_w = &segas24_state::hotrod_io_w;
	mlatch_table = segas24_state::bnzabros_mlt;

	// Sector  Size
	// 1       2048
	// 2       2048
	// 3       2048
	// 4       2048
	// 5       2048
	// 6       1024
	// 7        256

	track_size = 0x2d00;
}

DRIVER_INIT_MEMBER(segas24_state,sspirits)
{
	io_r = &segas24_state::hotrod_io_r;
	io_w = &segas24_state::hotrod_io_w;
	mlatch_table = nullptr;
	track_size = 0x2d00;
}

DRIVER_INIT_MEMBER(segas24_state,sspiritj)
{
	io_r = &segas24_state::hotrod_io_r;
	io_w = &segas24_state::hotrod_io_w;
	mlatch_table = nullptr;
	track_size = 0x2f00;
}

DRIVER_INIT_MEMBER(segas24_state,dcclubfd)
{
	io_r = &segas24_state::dcclub_io_r;
	io_w = &segas24_state::hotrod_io_w;
	mlatch_table = segas24_state::dcclub_mlt;
	track_size = 0x2d00;
}


DRIVER_INIT_MEMBER(segas24_state,sgmast)
{
	io_r = &segas24_state::hotrod_io_r;
	io_w = &segas24_state::hotrod_io_w;
	mlatch_table = nullptr;
	track_size = 0x2d00;
}

DRIVER_INIT_MEMBER(segas24_state,qsww)
{
	io_r = &segas24_state::hotrod_io_r;
	io_w = &segas24_state::hotrod_io_w;
	mlatch_table = nullptr;
	track_size = 0x2d00;
}

DRIVER_INIT_MEMBER(segas24_state,gground)
{
	io_r = &segas24_state::hotrod_io_r;
	io_w = &segas24_state::hotrod_io_w;
	mlatch_table = nullptr;
	track_size = 0x2d00;
}

DRIVER_INIT_MEMBER(segas24_state,crkdown)
{
	io_r = &segas24_state::hotrod_io_r;
	io_w = &segas24_state::hotrod_io_w;
	mlatch_table = nullptr;
	track_size = 0x2d00;
}

DRIVER_INIT_MEMBER(segas24_state,roughrac)
{
	io_r = &segas24_state::hotrod_io_r;
	io_w = &segas24_state::hotrod_io_w;
	mlatch_table = nullptr;
	track_size = 0x2d00;
}


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

//            YEAR, NAME,      PARENT,   MACHINE,                INPUT,    INIT,                    MONITOR,COMPANY,FULLNAME,FLAGS
/* Disk Based Games */
/* 01 */GAME( 1988, hotrod,    0,        system24_floppy,        hotrod,   segas24_state, hotrod,   ROT0,   "Sega", "Hot Rod (World, 3 Players, Turbo set 1, Floppy Based)", 0 )
/* 01 */GAME( 1988, hotroda,   hotrod,   system24_floppy,        hotrod,   segas24_state, hotrod,   ROT0,   "Sega", "Hot Rod (World, 3 Players, Turbo set 2, Floppy Based)", 0 )
/* 01 */GAME( 1988, hotrodj,   hotrod,   system24_floppy,        hotrodj,  segas24_state, hotrod,   ROT0,   "Sega", "Hot Rod (Japan, 4 Players, Floppy Based, Rev C)", 0 )
/* 01 */GAME( 1988, hotrodja,  hotrod,   system24_floppy,        hotrodj,  segas24_state, hotrod,   ROT0,   "Sega", "Hot Rod (Japan, 4 Players, Floppy Based, Rev B)", 0 )
/* 02 */GAME( 1988, sspirits,  0,        system24_floppy,        sspirits, segas24_state, sspirits, ROT270, "Sega", "Scramble Spirits (World, Floppy Based)", 0 )
/* 02 */GAME( 1988, sspiritj,  sspirits, system24_floppy,        sspirits, segas24_state, sspiritj, ROT270, "Sega", "Scramble Spirits (Japan, Floppy DS3-5000-02-REV-A Based)", 0 )
/* 02 */GAME( 1988, sspirtfc,  sspirits, system24_floppy_fd1094, sspirits, segas24_state, sspirits, ROT270, "Sega", "Scramble Spirits (World, Floppy Based, FD1094 317-0058-02c)", MACHINE_NOT_WORKING ) /* MISSING disk image */
/* 03 */GAME( 1988, gground,   0,        system24_floppy_fd1094, gground,  segas24_state, gground,  ROT270, "Sega", "Gain Ground (World, 3 Players, Floppy Based, FD1094 317-0058-03d Rev A)", 0 )
/* 03 */GAME( 1988, ggroundj,  gground,  system24_floppy_fd1094, gground,  segas24_state, gground,  ROT270, "Sega", "Gain Ground (Japan, 2 Players, Floppy Based, FD1094 317-0058-03b)", 0 )
/* 04 */GAME( 1989, crkdown,   0,        system24_floppy_fd1094, crkdown,  segas24_state, crkdown,  ROT0,   "Sega", "Crack Down (World, Floppy Based, FD1094 317-0058-04c)", MACHINE_IMPERFECT_GRAPHICS ) // clipping probs / solid layer probs? (radar display)
/* 04 */GAME( 1989, crkdownu,  crkdown,  system24_floppy_fd1094, crkdown,  segas24_state, crkdown,  ROT0,   "Sega", "Crack Down (US, Floppy Based, FD1094 317-0058-04d)", MACHINE_IMPERFECT_GRAPHICS ) // clipping probs / solid layer probs? (radar display)
/* 04 */GAME( 1989, crkdownj,  crkdown,  system24_floppy_fd1094, crkdown,  segas24_state, crkdown,  ROT0,   "Sega", "Crack Down (Japan, Floppy Based, FD1094 317-0058-04b Rev A)", MACHINE_IMPERFECT_GRAPHICS ) // clipping probs / solid layer probs? (radar display)
/* 05 */GAME( 1989, sgmast,    0,        system24_floppy_fd1094, sgmast,   segas24_state, sgmast,   ROT0,   "Sega", "Super Masters Golf (World?, Floppy Based, FD1094 317-0058-05d?)", 0 )
/* 05 */GAME( 1989, sgmastc,   sgmast,   system24_floppy_fd1094, sgmast,   segas24_state, sgmast,   ROT0,   "Sega", "Jumbo Ozaki Super Masters Golf (World, Floppy Based, FD1094 317-0058-05c)", MACHINE_IMPERFECT_GRAPHICS ) // some gfx offset / colour probs?
/* 05 */GAME( 1989, sgmastj,   sgmast,   system24_floppy_fd1094, sgmastj,  segas24_state, sgmast,   ROT0,   "Sega", "Jumbo Ozaki Super Masters Golf (Japan, Floppy Based, FD1094 317-0058-05b)", MACHINE_IMPERFECT_GRAPHICS ) // some gfx offset / colour probs?
/* 06 */GAME( 1990, roughrac,  0,        system24_floppy_fd1094, roughrac, segas24_state, roughrac, ROT0,   "Sega", "Rough Racer (Japan, Floppy Based, FD1094 317-0058-06b)", 0 )
/* 07 */GAME( 1990, bnzabros,  0,        system24_floppy,        bnzabros, segas24_state, bnzabros, ROT0,   "Sega", "Bonanza Bros (US, Floppy DS3-5000-07d? Based)", 0 )
/* 07 */GAME( 1990, bnzabrosj, bnzabros, system24_floppy,        bnzabros, segas24_state, bnzabros, ROT0,   "Sega", "Bonanza Bros (Japan, Floppy DS3-5000-07b Based)", 0 )
/* 08 */GAME( 1991, qsww,      0,        system24_floppy_fd1094, qsww,     segas24_state, qsww,     ROT0,   "Sega", "Quiz Syukudai wo Wasuremashita (Japan, Floppy Based, FD1094 317-0058-08b)", MACHINE_IMPERFECT_GRAPHICS ) // wrong bg colour on title
/* 09 */GAME( 1991, dcclubfd,  dcclub,   system24_floppy_fd1094, dcclub,   segas24_state, dcclubfd, ROT0,   "Sega", "Dynamic Country Club (US, Floppy Based, FD1094 317-0058-09d)", 0 )

//    YEAR, NAME,     PARENT,   MACHINE,  INPUT,    INIT,                    MONITOR,COMPANY,FULLNAME,FLAGS
/* ROM Based */
GAME( 1991, dcclub,   0,        system24, dcclub,   segas24_state, dcclub,   ROT0,   "Sega", "Dynamic Country Club (World, ROM Based)", 0 )
GAME( 1991, dcclubj,  dcclub,   system24, dcclub,   segas24_state, dcclub,   ROT0,   "Sega", "Dynamic Country Club (Japan, ROM Based)", 0 )
GAME( 1991, qrouka,   0,        system24, qrouka,   segas24_state, qrouka,   ROT0,   "Sega", "Quiz Rouka Ni Tattenasai (Japan, ROM Based)", 0 )
GAME( 1992, quizmeku, 0,        system24, quizmeku, segas24_state, quizmeku, ROT0,   "Sega", "Quiz Mekurumeku Story (Japan, ROM Based)", 0 ) /* Released in 05.1993 */
GAME( 1992, mahmajn,  0,        system24, mahmajn,  segas24_state, mahmajn,  ROT0,   "Sega", "Tokoro San no MahMahjan (Japan, ROM Based)", 0 )
GAME( 1994, qgh,      0,        system24, qgh,      segas24_state, qgh,      ROT0,   "Sega", "Quiz Ghost Hunter (Japan, ROM Based)", 0 )
GAME( 1994, mahmajn2, 0,        system24, mahmajn,  segas24_state, mahmajn2, ROT0,   "Sega", "Tokoro San no MahMahjan 2 (Japan, ROM Based)", 0 )
