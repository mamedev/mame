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

#include "driver.h"
#include "deprecat.h"
#include "cpu/m68000/m68k.h"
#include "segas24.h"
#include "system16.h"
#include "video/segaic24.h"
#include "sound/ym2151.h"
#include "sound/dac.h"
#include "sound/2151intf.h"

UINT16* s24_mainram1;

extern void s24_fd1094_machine_init(void);
extern void s24_fd1094_driver_init(running_machine *machine);

VIDEO_START(system24);
VIDEO_UPDATE(system24);


// Floppy Fisk Controller

static int fdc_status, fdc_track, fdc_sector, fdc_data;
static int fdc_phys_track, fdc_irq, fdc_drq, fdc_span, fdc_index_count;
static UINT8 *fdc_pt;
static int track_size;

static void fdc_init(void)
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

static READ16_HANDLER( fdc_r )
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
			//          logerror("Read %02x (%d)\n", res, fdc_span);
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

static WRITE16_HANDLER( fdc_w )
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
				fdc_pt = memory_region(machine, REGION_USER2) + track_size*(2*fdc_phys_track+(data & 8 ? 1 : 0));
				fdc_span = track_size;
				fdc_status = 3;
				fdc_drq = 1;
				fdc_data = *fdc_pt;
				break;
			case 0xb:
				logerror("Write multiple [%02x] %d..%d side %d track %d\n", data, fdc_sector, fdc_sector+fdc_data-1, data & 8 ? 1 : 0, fdc_phys_track);
				fdc_pt = memory_region(machine, REGION_USER2) + track_size*(2*fdc_phys_track+(data & 8 ? 1 : 0));
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

static READ16_HANDLER( fdc_status_r )
{
	if(!track_size)
		return 0xffff;

	return 0x90 | (fdc_irq ? 2 : 0) | (fdc_drq ? 1 : 0) | (fdc_phys_track ? 0x40 : 0) | (fdc_index_count ? 0x20 : 0);
}

static WRITE16_HANDLER( fdc_ctrl_w )
{
	if(ACCESSING_BITS_0_7)
		logerror("FDC control %02x\n", data & 0xff);
}


// I/O Mappers

static UINT8 hotrod_io_r(running_machine *machine, int port)
{
	switch(port) {
	case 0:
		return input_port_read_indexed(machine, 0);
	case 1:
		return input_port_read_indexed(machine, 1);
	case 2:
		return 0xff;
	case 3:
		return 0xff;
	case 4:
		return input_port_read_indexed(machine, 2);
	case 5: // Dip switches
		return input_port_read_indexed(machine, 3);
	case 6:
		return input_port_read_indexed(machine, 4);
	case 7: // DAC
		return 0xff;
	}
	return 0x00;
}

static UINT8 dcclub_io_r(running_machine *machine, int port)
{
	switch(port) {
	case 0: {
		static const UINT8 pos[16] = { 0, 1, 3, 2, 6, 4, 12, 8, 9 };
		return (input_port_read_indexed(machine, 0) & 0xf) | ((~pos[input_port_read_indexed(machine, 5)>>4]<<4) & 0xf0);
	}
	case 1:
		return input_port_read_indexed(machine, 1);
	case 2:
		return 0xff;
	case 3:
		return 0xff;
	case 4:
		return input_port_read_indexed(machine, 2);
	case 5: // Dip switches
		return input_port_read_indexed(machine, 3);
	case 6:
		return input_port_read_indexed(machine, 4);
	case 7: // DAC
		return 0xff;
	}
	return 0x00;
}

static int cur_input_line;

static UINT8 mahmajn_io_r(running_machine *machine, int port)
{
	switch(port) {
	case 0:
		return ~(1 << cur_input_line);
	case 1:
		return 0xff;
	case 2:
		return input_port_read_indexed(machine, cur_input_line);
	case 3:
		return 0xff;
	case 4:
		return input_port_read_indexed(machine, 8);
	case 5: // Dip switches
		return input_port_read_indexed(machine, 9);
	case 6:
		return input_port_read_indexed(machine, 10);
	case 7: // DAC
		return 0xff;
	}
	return 0x00;
}

static UINT8 gground_io_r(running_machine *machine, int port)
{
	switch(port) {
	case 0:
		return input_port_read_indexed(machine, 0);
	case 1:
		return input_port_read_indexed(machine, 1);
	case 2: // P3 inputs
		return input_port_read_indexed(machine, 2);
	case 3:
		return 0xff;
	case 4:
		return input_port_read_indexed(machine, 3);
	case 5: // Dip switches
		return input_port_read_indexed(machine, 4);
	case 6:
		return input_port_read_indexed(machine, 5);
	case 7: // DAC
		return 0xff;
	}
	return 0x00;
}

static void mahmajn_io_w(running_machine *machine, int port, UINT8 data)
{
	switch(port) {
	case 3:
		if(data & 4)
			cur_input_line = (cur_input_line + 1) & 7;
		break;
	case 7: // DAC
		DAC_0_signed_data_w(machine, 0, data);
		break;
	default:
		fprintf(stderr, "Port %d : %02x\n", port, data & 0xff);
	}
}

static void hotrod_io_w(running_machine *machine, int port, UINT8 data)
{
	switch(port) {
	case 3: // Lamps
		break;
	case 7: // DAC
		DAC_0_signed_data_w(machine,0, data);
		break;
	default:
		fprintf(stderr, "Port %d : %02x\n", port, data & 0xff);
	}
}

static UINT8 hotrod_ctrl_cur;

static WRITE16_HANDLER( hotrod3_ctrl_w )
{
	if(ACCESSING_BITS_0_7) {
		data &= 3;
		hotrod_ctrl_cur = input_port_read_indexed(machine, 9+data);
	}
}

static READ16_HANDLER( hotrod3_ctrl_r )
{
	if(ACCESSING_BITS_0_7) {
		switch(offset) {
			// Steering dials
		case 0:
			return input_port_read_indexed(machine, 5) & 0xff;
		case 1:
			return input_port_read_indexed(machine, 5) >> 8;
		case 2:
			return input_port_read_indexed(machine, 6) & 0xff;
		case 3:
			return input_port_read_indexed(machine, 6) >> 8;
		case 4:
			return input_port_read_indexed(machine, 7) & 0xff;
		case 5:
			return input_port_read_indexed(machine, 7) >> 8;
		case 6:
			return input_port_read_indexed(machine, 8) & 0xff;
		case 7:
			return input_port_read_indexed(machine, 8) >> 8;

		case 8: { // Serial ADCs for the accel
			int v = hotrod_ctrl_cur & 0x80;
			hotrod_ctrl_cur <<= 1;
			return v ? 0xff : 0;
		}
		}
	}
	return 0;
}

static READ16_HANDLER( iod_r )
{
	logerror("IO daughterboard read %02x (%x)\n", offset, activecpu_get_pc());
	return 0xffff;
}

static WRITE16_HANDLER( iod_w )
{
	logerror("IO daughterboard write %02x, %04x & %04x (%x)\n", offset, data, mem_mask, activecpu_get_pc());
}


// Cpu #1 reset control

static UINT8 resetcontrol, prev_resetcontrol;

static void reset_reset(running_machine *machine)
{
	int changed = resetcontrol ^ prev_resetcontrol;
	if(changed & 2) {
		if(resetcontrol & 2) {
			cpunum_set_input_line(machine, 1, INPUT_LINE_HALT, CLEAR_LINE);
			cpunum_set_input_line(machine, 1, INPUT_LINE_RESET, PULSE_LINE);
//          mame_printf_debug("enable 2nd cpu!\n");
//          DEBUGGER_BREAK;
			s24_fd1094_machine_init();

		} else
			cpunum_set_input_line(machine, 1, INPUT_LINE_HALT, ASSERT_LINE);
	}
	if(changed & 4)
		sndti_reset(SOUND_YM2151, 0);
	prev_resetcontrol = resetcontrol;
}

static void resetcontrol_w(UINT8 data)
{
	resetcontrol = data;
	logerror("Reset control %02x (%x:%x)\n", resetcontrol, cpu_getactivecpu(), activecpu_get_pc());
	reset_reset(Machine);
}


// Rom board bank access

static UINT8 curbank;

static void reset_bank(running_machine *machine)
{
	if (memory_region(machine, REGION_USER1))
	{
		memory_set_bank(1, curbank & 15);
		memory_set_bank(2, curbank & 15);
	}
}

static READ16_HANDLER( curbank_r )
{
	return curbank;
}

static WRITE16_HANDLER( curbank_w )
{
	if(ACCESSING_BITS_0_7) {
		curbank = data & 0xff;
		reset_bank(machine);
	}
}


// YM2151

static READ16_HANDLER( ym_status_r )
{
	return YM2151_status_port_0_r(machine, 0);
}

static WRITE16_HANDLER( ym_register_w )
{
	if(ACCESSING_BITS_0_7)
		YM2151_register_port_0_w(machine, 0, data);
}

static WRITE16_HANDLER( ym_data_w )
{
	if(ACCESSING_BITS_0_7)
		YM2151_data_port_0_w(machine, 0, data);
}


// Protection magic latch

static const UINT8  mahmajn_mlt[8] = { 5, 1, 6, 2, 3, 7, 4, 0 };
static const UINT8 mahmajn2_mlt[8] = { 6, 0, 5, 3, 1, 4, 2, 7 };
static const UINT8      gqh_mlt[8] = { 3, 7, 4, 0, 2, 6, 5, 1 };
static const UINT8 bnzabros_mlt[8] = { 2, 4, 0, 5, 7, 3, 1, 6 };
static const UINT8   qrouka_mlt[8] = { 1, 6, 4, 7, 0, 5, 3, 2 };
static const UINT8 quizmeku_mlt[8] = { 0, 3, 2, 4, 6, 1, 7, 5 };
static const UINT8   dcclub_mlt[8] = { 4, 3, 7, 0, 2, 6, 1, 5 };

static UINT8 mlatch;
static const UINT8 *mlatch_table;

static READ16_HANDLER( mlatch_r )
{
	return mlatch;
}

static WRITE16_HANDLER( mlatch_w )
{
	if(ACCESSING_BITS_0_7) {
		int i;
		UINT8 mxor = 0;
		if(!mlatch_table) {
			logerror("Protection: magic latch accessed but no table loaded (%d:%x)\n", cpu_getactivecpu(), activecpu_get_pc());
			return;
		}

		data &= 0xff;

		if(data != 0xff) {
			for(i=0; i<8; i++)
				if(mlatch & (1<<i))
					mxor |= 1 << mlatch_table[i];
			mlatch = data ^ mxor;
			logerror("Magic latching %02x ^ %02x as %02x (%d:%x)\n", data & 0xff, mxor, mlatch, cpu_getactivecpu(), activecpu_get_pc());
		} else {
			logerror("Magic latch reset (%d:%x)\n", cpu_getactivecpu(), activecpu_get_pc());
			mlatch = 0x00;
		}
	}
}


// Timers and IRQs

enum {
	IRQ_YM2151 = 1,
	IRQ_TIMER  = 2,
	IRQ_VBLANK = 3,
	IRQ_SPRITE = 4
};

static UINT16 irq_timera;
static UINT8  irq_timerb;
static UINT8  irq_allow0, irq_allow1;
static int    irq_timer_pend0, irq_timer_pend1, irq_yms;
static emu_timer *irq_timer;

static TIMER_CALLBACK( irq_timer_cb )
{
	irq_timer_pend0 = irq_timer_pend1 = 1;
	if(irq_allow0 & (1 << IRQ_TIMER))
		cpunum_set_input_line(machine, 0, IRQ_TIMER+1, ASSERT_LINE);
	if(irq_allow1 & (1 << IRQ_TIMER))
		cpunum_set_input_line(machine, 1, IRQ_TIMER+1, ASSERT_LINE);
}

static void irq_init(void)
{
	irq_timera = 0;
	irq_timerb = 0;
	irq_allow0 = 0;
	irq_allow1 = 0;
	irq_timer_pend0 = 0;
	irq_timer_pend1 = 0;
	irq_timer = timer_alloc(irq_timer_cb, NULL);
}

static void irq_timer_reset(void)
{
	int freq = (irq_timerb << 12) | irq_timera;
	freq &= 0x1fff;

	timer_adjust_periodic(irq_timer, ATTOTIME_IN_HZ(freq), 0, ATTOTIME_IN_HZ(freq));
	logerror("New timer frequency: %0d [%02x %04x]\n", freq, irq_timerb, irq_timera);
}

static WRITE16_HANDLER(irq_w)
{
	switch(offset) {
	case 0: {
		UINT16 old_ta = irq_timera;
		COMBINE_DATA(&irq_timera);
		if(old_ta != irq_timera)
			irq_timer_reset();
		break;
	}
	case 1:
		if(ACCESSING_BITS_0_7) {
			UINT8 old_tb = irq_timerb;
			irq_timerb = data;
			if(old_tb != irq_timerb)
				irq_timer_reset();
		}
		break;
	case 2:
		irq_allow0 = data;
		cpunum_set_input_line(machine, 0, IRQ_TIMER+1, irq_timer_pend0 && (irq_allow0 & (1 << IRQ_TIMER)) ? ASSERT_LINE : CLEAR_LINE);
		cpunum_set_input_line(machine, 0, IRQ_YM2151+1, irq_yms && (irq_allow0 & (1 << IRQ_YM2151)) ? ASSERT_LINE : CLEAR_LINE);
		break;
	case 3:
		irq_allow1 = data;
		cpunum_set_input_line(machine, 1, IRQ_TIMER+1, irq_timer_pend1 && (irq_allow1 & (1 << IRQ_TIMER)) ? ASSERT_LINE : CLEAR_LINE);
		cpunum_set_input_line(machine, 1, IRQ_YM2151+1, irq_yms && (irq_allow1 & (1 << IRQ_YM2151)) ? ASSERT_LINE : CLEAR_LINE);
		break;
	}
}


static int ggground_kludge;
/* This IRQ needs to be generated before the others or the GFX
  don't get uploaded correctly and you see nothing */
static TIMER_CALLBACK( gground_generate_kludge_irq )
{
		cpunum_set_input_line(machine, 1, 5, HOLD_LINE);
}


static READ16_HANDLER(irq_r)
{
	/* These hacks are for Gain Ground */
	/* otherwise the interrupt occurs before the correct state has been
       set and the game crashes before booting */
	if (!strcmp(machine->gamedrv->name,"gground"))
	{

		if (activecpu_get_pc()==0x0084aa)
		{
			ggground_kludge = 1;
			return mame_rand(machine);

		}
		if (activecpu_get_pc()==0x084ba)
		{
			/* Clear IRQ line so IRQ doesn't happen too early */
			cpunum_set_input_line(machine, 1, 5, CLEAR_LINE);

			/* set a timer to generate an irq at the needed point */
			if (ggground_kludge == 1)
			{
				timer_set(ATTOTIME_IN_USEC(180000), NULL, 0, gground_generate_kludge_irq);
				ggground_kludge = 0;
			}
			return 1;
		}
	}

	if (!strcmp(machine->gamedrv->name,"ggroundj"))
	{

		if (activecpu_get_pc()==0x0084ac)
		{
			ggground_kludge = 1;
			return mame_rand(machine);

		}
		if (activecpu_get_pc()==0x084bc)
		{
			/* Clear IRQ line so IRQ doesn't happen too early */
			cpunum_set_input_line(machine, 1, 5, CLEAR_LINE);

			/* set a timer to generate an irq at the needed point */
			if (ggground_kludge == 1)
			{
				timer_set(ATTOTIME_IN_USEC(180000), NULL, 0, gground_generate_kludge_irq);
				ggground_kludge = 0;
			}
			return 1;
		}
	}

	switch(offset) {
	case 2:
		irq_timer_pend0 = 0;
		cpunum_set_input_line(machine, 0, IRQ_TIMER+1, CLEAR_LINE);
		break;
	case 3:
		irq_timer_pend1 = 0;
		cpunum_set_input_line(machine, 1, IRQ_TIMER+1, CLEAR_LINE);
		break;
	}
	return 0xffff;
}

static INTERRUPT_GEN(irq_vbl)
{
	int irq = cpu_getiloops() ? IRQ_SPRITE : IRQ_VBLANK;
	int mask = 1 << irq;

	if(irq_allow0 & mask)
		cpunum_set_input_line(machine, 0, 1+irq, HOLD_LINE);

	if(irq_allow1 & mask)
		cpunum_set_input_line(machine, 1, 1+irq, HOLD_LINE);

	if(!cpu_getiloops()) {
		// Ensure one index pulse every 20 frames
		// The is some code in bnzabros at 0x852 that makes it crash
		// if the pulse train is too fast
		fdc_index_count++;
		if(fdc_index_count >= 20)
			fdc_index_count = 0;
	}
}

static void irq_ym(running_machine *machine, int irq)
{
	irq_yms = irq;
	cpunum_set_input_line(machine, 0, IRQ_YM2151+1, irq_yms && (irq_allow0 & (1 << IRQ_YM2151)) ? ASSERT_LINE : CLEAR_LINE);
	cpunum_set_input_line(machine, 1, IRQ_YM2151+1, irq_yms && (irq_allow1 & (1 << IRQ_YM2151)) ? ASSERT_LINE : CLEAR_LINE);
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

static ADDRESS_MAP_START( system24_cpu1_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_MIRROR(0x140000) AM_ROM AM_SHARE(1)
	AM_RANGE(0x080000, 0x0fffff) AM_RAM AM_SHARE(2)
	AM_RANGE(0x200000, 0x20ffff) AM_READWRITE(sys24_tile_r, sys24_tile_w)
	AM_RANGE(0x220000, 0x220001) AM_WRITENOP		// Unknown, always 0
	AM_RANGE(0x240000, 0x240001) AM_WRITENOP		// Horizontal synchronization register
	AM_RANGE(0x260000, 0x260001) AM_WRITENOP		// Vertical synchronization register
	AM_RANGE(0x270000, 0x270001) AM_WRITENOP		// Video synchronization switch
	AM_RANGE(0x280000, 0x29ffff) AM_READWRITE(sys24_char_r, sys24_char_w)
	AM_RANGE(0x400000, 0x403fff) AM_RAM_WRITE(system24temp_sys16_paletteram1_w) AM_BASE(&paletteram16)
	AM_RANGE(0x404000, 0x40401f) AM_READWRITE(sys24_mixer_r, sys24_mixer_w)
	AM_RANGE(0x600000, 0x63ffff) AM_READWRITE(sys24_sprite_r, sys24_sprite_w)
	AM_RANGE(0x800000, 0x80007f) AM_READWRITE(system24temp_sys16_io_r, system24temp_sys16_io_w)
	AM_RANGE(0x800100, 0x800101) AM_WRITE(ym_register_w)
	AM_RANGE(0x800102, 0x800103) AM_READWRITE(ym_status_r, ym_data_w)
	AM_RANGE(0xa00000, 0xa00007) AM_READWRITE(irq_r, irq_w)
	AM_RANGE(0xb00000, 0xb00007) AM_READWRITE(fdc_r, fdc_w)
	AM_RANGE(0xb00008, 0xb0000f) AM_READWRITE(fdc_status_r, fdc_ctrl_w)
	AM_RANGE(0xb80000, 0xbbffff) AM_ROMBANK(1)
	AM_RANGE(0xbc0000, 0xbc0001) AM_READWRITE(curbank_r, curbank_w)
	AM_RANGE(0xbc0006, 0xbc0007) AM_READWRITE(mlatch_r, mlatch_w)
	AM_RANGE(0xc00000, 0xc00011) AM_READWRITE(hotrod3_ctrl_r, hotrod3_ctrl_w)
	AM_RANGE(0xc80000, 0xcbffff) AM_ROMBANK(2)
	AM_RANGE(0xcc0000, 0xcc0001) AM_READWRITE(curbank_r, curbank_w)
	AM_RANGE(0xcc0006, 0xcc0007) AM_READWRITE(mlatch_r, mlatch_w)
AM_RANGE(0xd00300, 0xd00301) AM_WRITE(SMH_NOP)
	AM_RANGE(0xf00000, 0xf3ffff) AM_RAM AM_SHARE(3)
	AM_RANGE(0xf40000, 0xf7ffff) AM_ROM AM_SHARE(1)
	AM_RANGE(0xf80000, 0xffffff) AM_RAM AM_SHARE(2)
ADDRESS_MAP_END

/*************************************
 *
 *  CPU 2 memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( system24_cpu2_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_RAM AM_SHARE(3) AM_BASE(&s24_mainram1)			// RAM here overrides the ROM mirror
	AM_RANGE(0x040000, 0x07ffff) AM_ROM AM_SHARE(1)
	AM_RANGE(0x080000, 0x0fffff) AM_RAM AM_SHARE(2)
	AM_RANGE(0x100000, 0x13ffff) AM_MIRROR(0x040000) AM_ROM AM_SHARE(1)
	AM_RANGE(0x200000, 0x20ffff) AM_READWRITE(sys24_tile_r, sys24_tile_w)
	AM_RANGE(0x220000, 0x220001) AM_WRITENOP		// Unknown, always 0
	AM_RANGE(0x240000, 0x240001) AM_WRITENOP		// Horizontal synchronization register
	AM_RANGE(0x260000, 0x260001) AM_WRITENOP		// Vertical synchronization register
	AM_RANGE(0x270000, 0x270001) AM_WRITENOP		// Video synchronization switch
	AM_RANGE(0x280000, 0x29ffff) AM_READWRITE(sys24_char_r, sys24_char_w)
	AM_RANGE(0x400000, 0x403fff) AM_RAM_WRITE(system24temp_sys16_paletteram1_w)
	AM_RANGE(0x404000, 0x40401f) AM_READWRITE(sys24_mixer_r, sys24_mixer_w)
	AM_RANGE(0x600000, 0x63ffff) AM_READWRITE(sys24_sprite_r, sys24_sprite_w)
	AM_RANGE(0x800000, 0x80007f) AM_READWRITE(system24temp_sys16_io_r, system24temp_sys16_io_w)
	AM_RANGE(0x800100, 0x800101) AM_WRITE(ym_register_w)
	AM_RANGE(0x800102, 0x800103) AM_READWRITE(ym_status_r, ym_data_w)
	AM_RANGE(0xa00000, 0xa00007) AM_READWRITE(irq_r, irq_w)
	AM_RANGE(0xb00000, 0xb00007) AM_READWRITE(fdc_r, fdc_w)
	AM_RANGE(0xb00008, 0xb0000f) AM_READWRITE(fdc_status_r, fdc_ctrl_w)
	AM_RANGE(0xb80000, 0xbbffff) AM_ROMBANK(1)
	AM_RANGE(0xbc0000, 0xbc0001) AM_READWRITE(curbank_r, curbank_w)
	AM_RANGE(0xbc0006, 0xbc0007) AM_READWRITE(mlatch_r, mlatch_w)
	AM_RANGE(0xc00000, 0xc00011) AM_READWRITE(hotrod3_ctrl_r, hotrod3_ctrl_w)
	AM_RANGE(0xc80000, 0xcbffff) AM_ROMBANK(2)
	AM_RANGE(0xcc0000, 0xcc0001) AM_READWRITE(curbank_r, curbank_w)
	AM_RANGE(0xcc0006, 0xcc0007) AM_READWRITE(mlatch_r, mlatch_w)
AM_RANGE(0xd00300, 0xd00301) AM_WRITE(SMH_NOP)
	AM_RANGE(0xf00000, 0xf3ffff) AM_RAM AM_SHARE(3)
	AM_RANGE(0xf40000, 0xf7ffff) AM_ROM AM_SHARE(1)
	AM_RANGE(0xf80000, 0xffffff) AM_RAM AM_SHARE(2)
ADDRESS_MAP_END


/*************************************
 *
 *  Game-specific driver inits
 *
 *************************************/

static DRIVER_INIT(qgh)
{
	system24temp_sys16_io_set_callbacks(hotrod_io_r, hotrod_io_w, resetcontrol_w, iod_r, iod_w);
	mlatch_table = gqh_mlt;
	track_size = 0;
}

static DRIVER_INIT(dcclub)
{
	system24temp_sys16_io_set_callbacks(dcclub_io_r, hotrod_io_w, resetcontrol_w, iod_r, iod_w);
	mlatch_table = dcclub_mlt;
	track_size = 0;
}

static DRIVER_INIT(qrouka)
{
	system24temp_sys16_io_set_callbacks(gground_io_r, hotrod_io_w, resetcontrol_w, iod_r, iod_w);
	mlatch_table = qrouka_mlt;
	track_size = 0;
}

static DRIVER_INIT(quizmeku)
{
	system24temp_sys16_io_set_callbacks(gground_io_r, hotrod_io_w, resetcontrol_w, iod_r, iod_w);
	mlatch_table = quizmeku_mlt;
	track_size = 0;
}

static DRIVER_INIT(mahmajn)
{

	system24temp_sys16_io_set_callbacks(mahmajn_io_r, mahmajn_io_w, resetcontrol_w, iod_r, iod_w);
	mlatch_table = mahmajn_mlt;
	track_size = 0;
	cur_input_line = 0;
}

static DRIVER_INIT(mahmajn2)
{

	system24temp_sys16_io_set_callbacks(mahmajn_io_r, mahmajn_io_w, resetcontrol_w, iod_r, iod_w);
	mlatch_table = mahmajn2_mlt;
	track_size = 0;
	cur_input_line = 0;
}

static DRIVER_INIT(hotrod)
{
	system24temp_sys16_io_set_callbacks(hotrod_io_r, hotrod_io_w, resetcontrol_w, iod_r, iod_w);
	mlatch_table = 0;

	// Sector  Size
	// 1       8192
	// 2       1024
	// 3       1024
	// 4       1024
	// 5        512
	// 6        256

	track_size = 0x2f00;
}

static DRIVER_INIT(bnzabros)
{
	system24temp_sys16_io_set_callbacks(hotrod_io_r, hotrod_io_w, resetcontrol_w, iod_r, iod_w);
	mlatch_table = bnzabros_mlt;

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

static DRIVER_INIT(sspirits)
{
	system24temp_sys16_io_set_callbacks(hotrod_io_r, hotrod_io_w, resetcontrol_w, iod_r, iod_w);
	mlatch_table = 0;
	track_size = 0x2d00;
	s24_fd1094_driver_init(machine);

}

static DRIVER_INIT(sspiritj)
{
	system24temp_sys16_io_set_callbacks(hotrod_io_r, hotrod_io_w, resetcontrol_w, iod_r, iod_w);
	mlatch_table = 0;
	track_size = 0x2f00;
	s24_fd1094_driver_init(machine);

}

static DRIVER_INIT(dcclubfd)
{
	system24temp_sys16_io_set_callbacks(dcclub_io_r, hotrod_io_w, resetcontrol_w, iod_r, iod_w);
	mlatch_table = dcclub_mlt;
	track_size = 0x2d00;
	s24_fd1094_driver_init(machine);

}




static DRIVER_INIT(sgmast)
{
	system24temp_sys16_io_set_callbacks(hotrod_io_r, hotrod_io_w, resetcontrol_w, iod_r, iod_w);
	mlatch_table = 0;
	track_size = 0x2d00;
	s24_fd1094_driver_init(machine);

}

static DRIVER_INIT(qsww)
{
	system24temp_sys16_io_set_callbacks(hotrod_io_r, hotrod_io_w, resetcontrol_w, iod_r, iod_w);
	mlatch_table = 0;
	track_size = 0x2d00;
	s24_fd1094_driver_init(machine);
}

static DRIVER_INIT(gground)
{
	system24temp_sys16_io_set_callbacks(gground_io_r, hotrod_io_w, resetcontrol_w, iod_r, iod_w);
	mlatch_table = 0;
	track_size = 0x2d00;
	s24_fd1094_driver_init(machine);
}

static DRIVER_INIT(crkdown)
{
	system24temp_sys16_io_set_callbacks(hotrod_io_r, hotrod_io_w, resetcontrol_w, iod_r, iod_w);
	mlatch_table = 0;
	track_size = 0x2d00;
	s24_fd1094_driver_init(machine);
}

static DRIVER_INIT(roughrac)
{
	system24temp_sys16_io_set_callbacks(hotrod_io_r, hotrod_io_w, resetcontrol_w, iod_r, iod_w);
	mlatch_table = 0;
	track_size = 0x2d00;
	s24_fd1094_driver_init(machine);
}

/*************************************
 *
 *  Generic driver initialization
 *
 *************************************/

static NVRAM_HANDLER(system24)
{
	if(!track_size || !file)
		return;
	if(read_or_write)
		mame_fwrite(file, memory_region(machine, REGION_USER2), 2*track_size);
	else
		mame_fread(file, memory_region(machine, REGION_USER2), 2*track_size);
}

static MACHINE_START( system24 )
{
	UINT8 *usr1 = memory_region(machine, REGION_USER1);
	if (usr1)
	{
		memory_configure_bank(1, 0, 16, usr1, 0x40000);
		memory_configure_bank(2, 0, 16, usr1, 0x40000);
	}
}

static MACHINE_RESET(system24)
{
	cpunum_set_input_line(machine, 1, INPUT_LINE_HALT, ASSERT_LINE);
	prev_resetcontrol = resetcontrol = 0x06;
	fdc_init();
	curbank = 0;
	reset_bank(machine);
	irq_init();
	mlatch = 0x00;
}

/*************************************
 *
 *  Generic port definitions
 *
 *************************************/

static INPUT_PORTS_START( system24_P1_P2 )
	PORT_START_TAG("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
 	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) //enabled with "Separate"
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)

	PORT_START_TAG("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE3 ) //enabled with "Separate"
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( system24_Service )
	PORT_START_TAG("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN4 )
INPUT_PORTS_END

static INPUT_PORTS_START( system24_DSW )
	PORT_START_TAG("COINAGE")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SWA:1,2,3,4")
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
	PORT_DIPSETTING(    0x00, "Free Play (if Coin B too) or 1/1" )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SWA:5,6,7,8")
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
	PORT_DIPSETTING(    0x00, "Free Play (if Coin A too) or 1/1" )

	PORT_START_TAG("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) ) PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) ) PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) ) PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) ) PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) ) PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) ) PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unused ) ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( system24_generic )
	PORT_INCLUDE( system24_P1_P2 )
	PORT_INCLUDE( system24_Service )
	PORT_INCLUDE( system24_DSW )
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
	PORT_DIPNAME( 0x20, 0x20, "Start Credit" ) PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x40, 0x40, "Coin Chute" ) PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, "Separate" )
	PORT_DIPSETTING(    0x00, "Common" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START
	PORT_BIT( 0xfff, 0x000, IPT_DIAL ) PORT_MINMAX(0x000,0xfff) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START
	PORT_BIT( 0xfff, 0x000, IPT_DIAL ) PORT_MINMAX(0x000,0xfff) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START
	PORT_BIT( 0xfff, 0x000, IPT_DIAL ) PORT_MINMAX(0x000,0xfff) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(3)

	PORT_START
	PORT_BIT( 0xfff, 0x000, IPT_DIAL ) PORT_MINMAX(0x000,0xfff) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(4)

	PORT_START
	PORT_BIT( 0xff, 0x01, IPT_PEDAL ) PORT_MINMAX(0x01,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START
	PORT_BIT( 0xff, 0x01, IPT_PEDAL ) PORT_MINMAX(0x01,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2)

	PORT_START
	PORT_BIT( 0xff, 0x01, IPT_PEDAL ) PORT_MINMAX(0x01,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(3)

	PORT_START
	PORT_BIT( 0xff, 0x01, IPT_PEDAL ) PORT_MINMAX(0x01,0xff) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(4)
INPUT_PORTS_END

static INPUT_PORTS_START( hotrodj )
	PORT_INCLUDE( hotrod )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x04, 0x04, "Start Credit" ) PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x04, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x08, 0x08, "Play Mode" ) PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x08, "4 Player" )
	PORT_DIPSETTING(    0x00, "2 Player" )
	PORT_DIPNAME( 0x10, 0x10, "Game Style" ) PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, "4 Sides" )
	PORT_DIPSETTING(    0x00, "2 Sides" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Language ) ) PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Japanese ) )
	PORT_DIPSETTING(    0x00, DEF_STR( English ) )
INPUT_PORTS_END

static INPUT_PORTS_START( bnzabros )
	PORT_INCLUDE( system24_generic )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Start Credit" ) PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x04, 0x04, "Coin Chute" ) PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x04, "Common" )
	PORT_DIPSETTING(    0x00, "Individual" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x80, "4" )
INPUT_PORTS_END

static INPUT_PORTS_START( crkdown )
	PORT_INCLUDE( system24_generic )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x40, 0x40, "Coin Chute" ) PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, "Common" )
	PORT_DIPSETTING(    0x00, "Separate" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( roughrac )
	PORT_INCLUDE( system24_generic )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Start Credit" ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Game Instruction" ) PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x40, 0x40, "Start Money" ) PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x40, "15000" )
	PORT_DIPSETTING(    0x00, "20000" )
	PORT_DIPNAME( 0x80, 0x80, "Start Intro" ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, "10" )
	PORT_DIPSETTING(    0x00, "15" )

	PORT_START
	PORT_BIT( 0xfff, 0x000, IPT_DIAL ) PORT_MINMAX(0x000,0xfff) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START
	PORT_BIT( 0xfff, 0x000, IPT_DIAL ) PORT_MINMAX(0x000,0xfff) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( sspirits )
	PORT_INCLUDE( system24_generic )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x20, "500K" )
	PORT_DIPSETTING(    0x30, "600K" )
	PORT_DIPSETTING(    0x10, "800K" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( qgh )
	PORT_INCLUDE( system24_generic )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x30, "5" )
INPUT_PORTS_END

static INPUT_PORTS_START( qsww )
	PORT_INCLUDE( system24_generic )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, "Start Credit" ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x30, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:7,8")
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
	PORT_DIPNAME( 0x01, 0x01, "Start Credit" ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Timing Meter" ) PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPNAME( 0x10, 0x10, "Initial Balls" ) PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPNAME( 0x20, 0x20, "Balls Limit" ) PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START
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
	PORT_DIPNAME( 0x01, 0x01, "Start Credit" ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x01, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Gameplay" ) PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x08, "1 Hole 1 Credit" )
	PORT_DIPSETTING(    0x00, "Lose Ball Game Over" )
	PORT_DIPNAME( 0xe0, 0xe0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:6,7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( Easier ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x60, "Little Hard" )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Harder ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( sgmastj )
	PORT_INCLUDE( sgmast )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) ) PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START
	PORT_BIT( 0xfff, 0x000, IPT_DIAL ) PORT_MINMAX(0x000,0xfff) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_PLAYER(2)
INPUT_PORTS_END

static INPUT_PORTS_START( quizmeku )
	PORT_INCLUDE( system24_P1_P2 )

	PORT_MODIFY("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)

	PORT_MODIFY("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START4 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(4)

	PORT_START_TAG("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)

	PORT_INCLUDE( system24_Service )
	PORT_INCLUDE( system24_DSW )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, "Play Mode" ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x01, "2 Player" )
	PORT_DIPSETTING(    0x00, "4 Player" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, "Answer Counts" ) PORT_DIPLOCATION("SWB:5")
	PORT_DIPSETTING(    0x10, "Every" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SWB:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( qrouka )
	PORT_INCLUDE( quizmeku )

	PORT_MODIFY("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Coin Chute" ) PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x08, "Common" )
	PORT_DIPSETTING(    0x00, "Separate" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x30, "4" )
	PORT_DIPSETTING(    0x20, "5" )
INPUT_PORTS_END

static INPUT_PORTS_START( mahmajn )
	PORT_START_TAG("MJ0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("A") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("B") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("C") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("D") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("MJ1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("E") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("F") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("G") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("H") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("MJ2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("I") PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("J") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("K") PORT_CODE(KEYCODE_K)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("L") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("MJ3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("M") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("N") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Chi") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Pon") PORT_CODE(KEYCODE_LALT)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("MJ4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("MJ5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Kan") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Reach") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Ron") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("P1")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START_TAG("P2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_INCLUDE( system24_Service )
	PORT_INCLUDE( system24_DSW )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Start Score" ) PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x00, "2000" )
	PORT_DIPSETTING(    0x02, "3000" )
	PORT_DIPNAME( 0x0c, 0x0c, "Difficulty (computer)" ) PORT_DIPLOCATION("SWB:3,4")
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x30, 0x30, "Difficulty (player)" ) PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x30, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( gground )
	PORT_INCLUDE( system24_P1_P2 )

	PORT_START_TAG("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE4 ) //enabled with "Separate"
 	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(3)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(3)

	PORT_INCLUDE( system24_Service )
	PORT_INCLUDE( system24_DSW )

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x1c, 0x1c, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SWB:3,4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Easier ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x18, "Little Easy" )
	PORT_DIPSETTING(    0x1c, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x0c, "Little Hard" )
	PORT_DIPSETTING(    0x14, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x60, 0x60, "Time Limit Per Stage" ) PORT_DIPLOCATION("SWB:6,7")
	PORT_DIPSETTING(    0x20, DEF_STR( Easiest ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x60, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, "Clock Of Time Limit" ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x80, "1.00 sec" )
	PORT_DIPSETTING(    0x00, "0.80 sec" )
INPUT_PORTS_END

/*************************************
 *
 *  Sound definitions
 *
 *************************************/

static const struct YM2151interface ym2151_interface =
{
	irq_ym
};

/*************************************
 *
 *  Generic machine drivers
 *
 *************************************/

static MACHINE_DRIVER_START( system24 )
	MDRV_CPU_ADD(M68000, 10000000)
	MDRV_CPU_PROGRAM_MAP(system24_cpu1_map, 0)
	MDRV_CPU_VBLANK_INT_HACK(irq_vbl, 2)

	MDRV_CPU_ADD(M68000, 10000000)
	MDRV_CPU_PROGRAM_MAP(system24_cpu2_map, 0)

	MDRV_INTERLEAVE(4)

	MDRV_MACHINE_START(system24)
	MDRV_MACHINE_RESET(system24)
	MDRV_NVRAM_HANDLER(system24)

	MDRV_VIDEO_ATTRIBUTES(VIDEO_UPDATE_AFTER_VBLANK)

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(58)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(100))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(62*8, 48*8)
	MDRV_SCREEN_VISIBLE_AREA(0*8, 62*8-1, 0*8, 48*8-1)

	MDRV_PALETTE_LENGTH(8192*2)

	MDRV_VIDEO_START(system24)
	MDRV_VIDEO_UPDATE(system24)

	MDRV_SPEAKER_STANDARD_STEREO("left", "right")

	MDRV_SOUND_ADD(YM2151, 4000000)
	MDRV_SOUND_CONFIG(ym2151_interface)
	MDRV_SOUND_ROUTE(0, "left", 0.50)
	MDRV_SOUND_ROUTE(1, "right", 0.50)

	MDRV_SOUND_ADD(DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "left", 0.50)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "right", 0.50)
MACHINE_DRIVER_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( hotrod )
	ROM_REGION( 0x100000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-11339.ic2", 0x000000, 0x20000, CRC(75130e73) SHA1(e079739f4a3da3807aac570442c5afef1a7d7b0e) )
	ROM_LOAD16_BYTE( "epr-11338.ic1", 0x000001, 0x20000, CRC(7d4a7ff3) SHA1(3d3af04d990d232ba0a8fe155de59bc632a0a461) )

	ROM_REGION( 0x1d6000, REGION_USER2, 0)
	ROM_LOAD( "ds3-5000-01d_3p_turbo.bin", 0x000000, 0x1d6000, CRC(627e8053) SHA1(d1a95f99078f5a29cccacfb1b30c3c9ead7b605c) )
ROM_END

ROM_START( hotroda )
	ROM_REGION( 0x100000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-11339.ic2", 0x000000, 0x20000, CRC(75130e73) SHA1(e079739f4a3da3807aac570442c5afef1a7d7b0e) )
	ROM_LOAD16_BYTE( "epr-11338.ic1", 0x000001, 0x20000, CRC(7d4a7ff3) SHA1(3d3af04d990d232ba0a8fe155de59bc632a0a461) )

	ROM_REGION( 0x1d6000, REGION_USER2, 0)
	ROM_LOAD( "ds3-5000-01d.bin", 0x000000, 0x1d6000, CRC(abf67b02) SHA1(f397435eaad691ff5a38d6d1d27840ed95a62df3) ) // World? 3 Playe TURBO
ROM_END

ROM_START( hotrodj )
	ROM_REGION( 0x100000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-11339.ic2", 0x000000, 0x20000, CRC(75130e73) SHA1(e079739f4a3da3807aac570442c5afef1a7d7b0e) )
	ROM_LOAD16_BYTE( "epr-11338.ic1", 0x000001, 0x20000, CRC(7d4a7ff3) SHA1(3d3af04d990d232ba0a8fe155de59bc632a0a461) )

	ROM_REGION( 0x1d6000, REGION_USER2, 0)
	ROM_LOAD( "ds3-5000-01a-revb.bin", 0x000000, 0x1d6000, CRC(a39a0c2d) SHA1(ea8104c2266c48f480837aa7679c0a6f0c5e5452) ) // Japanese 4 Player
ROM_END

ROM_START( qgh )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "16900b", 0x000000, 0x20000, CRC(20d7b7d1) SHA1(345b228c27e5f2fef9a2b8b5f619c59450a070f8) )
	ROM_LOAD16_BYTE( "16899b", 0x000001, 0x20000, CRC(397b3ba9) SHA1(1773212cd87dcff840f3953ec368be7e2394faf0) )

	ROM_REGION16_BE( 0x400000, REGION_USER1, 0)
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
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "14485", 0x000000, 0x20000, CRC(fc0085f9) SHA1(0250d1e17e19b541b85198ec4207e55bfbd5c32e) )
	ROM_LOAD16_BYTE( "14484", 0x000001, 0x20000, CRC(f51c641c) SHA1(3f2fd0be7d58c75e88565393da5e810655413b53) )

	ROM_REGION16_BE( 0x400000, REGION_USER1, 0)
	ROM_LOAD16_BYTE( "14482", 0x000000, 0x80000, CRC(7a13dd97) SHA1(bfe9950d2cd41f3f866520923c1ed7b8da1990ec) )
	ROM_LOAD16_BYTE( "14483", 0x100000, 0x80000, CRC(f3eb51a0) SHA1(6904830ff5e7aa5f016e115572fb6da678896ede) )
ROM_END

ROM_START( mahmajn )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr14813.bin", 0x000000, 0x20000, CRC(ea38cf4b) SHA1(118ab2e0ae20a4db5e619945dfbb3f200de3979c) )
	ROM_LOAD16_BYTE( "epr14812.bin", 0x000001, 0x20000, CRC(5a3cb4a7) SHA1(c0f21282140e8e6e927664f5f2b90525ae0207e9) )

	ROM_REGION16_BE( 0x400000, REGION_USER1, 0)
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
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr16799.bin", 0x000000, 0x20000, CRC(3a34cf75) SHA1(d22bf6334668af29167cf4244d18f9cd2e7ff7d6) )
	ROM_LOAD16_BYTE( "epr16798.bin", 0x000001, 0x20000, CRC(662923fa) SHA1(dcd3964d899d3f34dab22ffcd1a5af895804fae1) )

	ROM_REGION16_BE( 0x400000, REGION_USER1, 0)
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
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION16_BE( 0x400000, REGION_USER1, 0)
	ROM_LOAD16_BYTE( "mpr-13188-h.2",  0x000000, 0x80000, CRC(d3802294) SHA1(7608e71e8ef398ac24dbf851994253bca5ace625) )
	ROM_LOAD16_BYTE( "mpr-13187-h.1",  0x000001, 0x80000, CRC(e3d8c5f7) SHA1(5b1e8646debee2f2ef272ddd3320b0a17192fbbe) )
	ROM_LOAD16_BYTE( "mpr-13190.4",    0x100000, 0x40000, CRC(0b4df388) SHA1(340478bba82069ab745d6d8703e6801411fd2fc4) )
	ROM_RELOAD ( 0x180000, 0x40000)
	ROM_LOAD16_BYTE( "mpr-13189.3",    0x100001, 0x40000, CRC(5ea5a2f3) SHA1(514b5446303c50aeb1d6d10d0a3f210da2577e16) )
	ROM_RELOAD ( 0x180001, 0x40000)

	ROM_REGION( 0x1c2000, REGION_USER2, 0)
	ROM_LOAD( "ds3-5000-07d.img",        0x000000, 0x1c2000, CRC(ea7a3302) SHA1(5f92efb2e1135c1f3eeca38ba5789739a22dbd11) ) /* Region letter needs to be verfied */
ROM_END

ROM_START( bnzabrsj )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION16_BE( 0x400000, REGION_USER1, 0)
	ROM_LOAD16_BYTE( "mpr-13188-h.2",  0x000000, 0x80000, CRC(d3802294) SHA1(7608e71e8ef398ac24dbf851994253bca5ace625) )
	ROM_LOAD16_BYTE( "mpr-13187-h.1",  0x000001, 0x80000, CRC(e3d8c5f7) SHA1(5b1e8646debee2f2ef272ddd3320b0a17192fbbe) )
	ROM_LOAD16_BYTE( "mpr-13190.4",    0x100000, 0x40000, CRC(0b4df388) SHA1(340478bba82069ab745d6d8703e6801411fd2fc4) )
	ROM_RELOAD ( 0x180000, 0x40000)
	ROM_LOAD16_BYTE( "mpr-13189.3",    0x100001, 0x40000, CRC(5ea5a2f3) SHA1(514b5446303c50aeb1d6d10d0a3f210da2577e16) )
	ROM_RELOAD ( 0x180001, 0x40000)

	ROM_REGION( 0x1c2000, REGION_USER2, 0)
	ROM_LOAD( "ds3-5000-07b.img",        0x000000, 0x1c2000, CRC(efa7f2a7) SHA1(eb905bff88fa40324fb92b91ac8a5878648c26e5) )
ROM_END

ROM_START( quizmeku ) // Quiz Mekuromeku Story
	 ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	 ROM_LOAD16_BYTE( "epr15343.ic2", 0x000000, 0x20000, CRC(c72399a7) SHA1(bfbf0079ea63f89bca4ce9081aed5d5c1d9d169a) )
	 ROM_LOAD16_BYTE( "epr15342.ic1", 0x000001, 0x20000, CRC(0968ac84) SHA1(4e1170ac123adaec32819754b5075531ff1925fe) )

	 ROM_REGION16_BE( 0x400000, REGION_USER1, 0)
	 ROM_LOAD16_BYTE( "epr15345.ic5", 0x000000, 0x80000, CRC(88030b5d) SHA1(d2feeedb9a64c3dc8dd25716209f945d12fa9b53) )
	 ROM_LOAD16_BYTE( "epr15344.ic4", 0x000001, 0x80000, CRC(dd11b382) SHA1(2b0f49fb307a9aba0f295de64782ee095c557170) )
	 ROM_LOAD16_BYTE( "mpr15347.ic7", 0x100000, 0x80000, CRC(0472677b) SHA1(93ae57a2817b6b54c99814fca28ef51f7ff5e559) )
	 ROM_LOAD16_BYTE( "mpr15346.ic6", 0x100001, 0x80000, CRC(746d4d0e) SHA1(7863abe36126684772a4459d5b6f3b24670ec02b) )
	 ROM_LOAD16_BYTE( "mpr15349.ic9", 0x200000, 0x80000, CRC(770eecf1) SHA1(86cc5b4a325198dc1da1446ecd8e718415b7998a) )
	 ROM_LOAD16_BYTE( "mpr15348.ic8", 0x200001, 0x80000, CRC(7666e960) SHA1(f3f88d5c8318301a8c73141de60292f8349ac0ce) )
ROM_END

ROM_START( sspirits )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION( 0x1c2000, REGION_USER2, 0)
	ROM_LOAD( "ds3-5000-02-.img",         0x000000, 0x1c2000, CRC(cefbda69) SHA1(5b47ae0f1584ce1eb697246273ba761bd9e981c1)  )
ROM_END

ROM_START( sspiritj )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-11339.ic2", 0x000000, 0x20000, CRC(75130e73) SHA1(e079739f4a3da3807aac570442c5afef1a7d7b0e) )
	ROM_LOAD16_BYTE( "epr-11338.ic1", 0x000001, 0x20000, CRC(7d4a7ff3) SHA1(3d3af04d990d232ba0a8fe155de59bc632a0a461) )

	ROM_REGION( 0x1d6000, REGION_USER2, 0)
	ROM_LOAD( "ds3-5000-02-rev-a.img",         0x000000, 0x1d6000, CRC(0385470f) SHA1(62c1bfe3a88b2dee44629809e08b4b8a5770eaab)  )
ROM_END

ROM_START( sspirtfc )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION( 0x2000, REGION_USER3, 0 )	/* decryption key */
	ROM_LOAD( "317-0058-02c.key", 0x0000, 0x2000,  CRC(ebae170e) SHA1(b6d1e1b6943a35b96e98e426ecb39bb5a42fb643) )

	ROM_REGION( 0x1c2000, REGION_USER2, 0)
	ROM_LOAD( "ds3-5000-02c.img",         0x000000, 0x1c2000, NO_DUMP )
ROM_END

ROM_START( sgmast )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION( 0x2000, REGION_USER3, 0 )	/* decryption key */
	ROM_LOAD( "317-0058-05d.key", 0x0000, 0x2000, NO_DUMP )

	ROM_REGION( 0x1c2000, REGION_USER2, 0)
	/* not sure which of these images is best */
	ROM_LOAD( "ds3-5000-05d.img",      0x000000, 0x1c2000, CRC(e9a69f93) SHA1(dc15e47ed78373688c1fab72a9605528068ad702) )
	ROM_LOAD( "ds3-5000-05d_alt.img",  0x000000, 0x1c2000, CRC(e71a8ebf) SHA1(60feb0af1cfc0508c8d68c8572495eec1763dc93) )
	ROM_LOAD( "ds3-5000-05d_alt2.img", 0x000000, 0x1c2000, CRC(460bdcd5) SHA1(49b7384ac5742b45b7369f220f33f04ef955e992) )

ROM_END

ROM_START( sgmastc )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION( 0x2000, REGION_USER3, 0 )	/* decryption key */
	ROM_LOAD( "317-0058-05c.key", 0x0000, 0x2000, CRC(ae0eabe5) SHA1(692d7565bf9c5b32cc80bb4bd88c9193aa04cbb0) )

	ROM_REGION( 0x1c2000, REGION_USER2, 0)
	ROM_LOAD( "ds3-5000-05c.img",      0x000000, 0x1c2000, CRC(06c4f834) SHA1(5e178ed0edff7721c93f76da2e03ae188dc5efa4) )
ROM_END

ROM_START( sgmastj )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION( 0x2000, REGION_USER3, 0 )	/* decryption key */
	ROM_LOAD( "317-0058-05b.key", 0x0000, 0x2000, CRC(adc0c83b) SHA1(2328d82d5057062eeb0072fd57f0422218cf24fc) )

	ROM_REGION( 0x1c2000, REGION_USER2, 0)
	ROM_LOAD( "ds3-5000-05b.img",      0x000000, 0x1c2000,  CRC(a136668c) SHA1(7203f9d11023605a0a4b52a4be330785c8f7b623) )
ROM_END

ROM_START( qsww )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION( 0x2000, REGION_USER3, 0 )	/* decryption key */
	ROM_LOAD( "317-0058-08b.key", 0x0000, 0x2000,  CRC(fe0a336a) SHA1(f7a5b2c1a057d0bb8c1ae0453c58aa8f5fb731b9) )

	ROM_REGION( 0x1c2000, REGION_USER2, 0)
	ROM_LOAD( "ds3-5000-08b.img",    0x000000, 0x1c2000, CRC(5a886d38) SHA1(2e974a9ffe3534da4fb117c579b8b0e61a63542c) )
ROM_END

ROM_START( gground )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION( 0x2000, REGION_USER3, 0 )	/* decryption key */
	ROM_LOAD( "317-0058-03c.key", 0x0000, 0x2000,  CRC(e1785bbd) SHA1(b4bebb2829299f1c0815d6a5f317a2526b322f63) ) /* Region letter needs to be verfied */

	ROM_REGION( 0x1c2000, REGION_USER2, 0)
	ROM_LOAD( "ds3-5000-03c.img",         0x000000, 0x1c2000, CRC(5c5910f2) SHA1(9ed564a03c0d4ca4a207f3ecfb7336c6cbcaa70f) ) /* Region letter needs to be verfied */
ROM_END

ROM_START( ggroundj )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION( 0x2000, REGION_USER3, 0 )	/* decryption key */
	ROM_LOAD( "317-0058-03b.key", 0x0000, 0x2000,  CRC(84aecdba) SHA1(ceddf967359a6e76543fe1ab00be53d0a11fe1ab) )

	ROM_REGION( 0x1c2000, REGION_USER2, 0)
	ROM_LOAD( "ds3-5000-03b.img",         0x000000, 0x1c2000, CRC(7200dac9) SHA1(07cf33bf2a0da36e3852de409959f30128cdbf77) )
ROM_END

ROM_START( crkdown )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION( 0x2000, REGION_USER3, 0 )	/* decryption key */
	ROM_LOAD( "317-0058-04c.key", 0x0000, 0x2000,  CRC(16e978cc) SHA1(0e1482b5efa93b732d4cf0990919cb3fc903dca7) )

	ROM_REGION( 0x1c2000, REGION_USER2, 0)
	ROM_LOAD( "ds3-5000-04c.img",    0x000000, 0x1c2000, CRC(5edc01a5) SHA1(8eb1bf41f533d16c12930f5831f8bccd4d8de4f7) )
ROM_END

ROM_START( crkdownu )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION( 0x2000, REGION_USER3, 0 )	/* decryption key */
	ROM_LOAD( "317-0058-04d.key", 0x0000, 0x2000,  CRC(934ac358) SHA1(73418e22c9d201bc3fec5c63284858958c010e05) )

	ROM_REGION( 0x1c2000, REGION_USER2, 0)
	ROM_LOAD( "ds3-5000-04d.img",    0x000000, 0x1c2000, CRC(8679032c) SHA1(887b245a70652897fbda736b60e81a123866ec12) )
ROM_END

ROM_START( crkdownj )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION( 0x2000, REGION_USER3, 0 )	/* decryption key */
	ROM_LOAD( "317-0058-04b.key", 0x0000, 0x2000,  CRC(4a99a202) SHA1(d7375f09e7246ecd60ba0e48f049e9e252af92a8) )

	ROM_REGION( 0x1c2000, REGION_USER2, 0)
	ROM_LOAD( "ds3-5000-04b.img",    0x000000, 0x1c2000, CRC(5daa1a9a) SHA1(ce2f07b83b607bbbdb70f1ae344c1d897d601809) )
ROM_END

ROM_START( dcclub )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr13948.bin", 0x000000, 0x20000, CRC(d6a031c8) SHA1(45b7e3cd2c7412e24f547cd4185166199d3938d5) )
	ROM_LOAD16_BYTE( "epr13947.bin", 0x000001, 0x20000, CRC(7e3cff5e) SHA1(ff8cb776d2491796feeb8892c7e644e590438945) )

	ROM_REGION16_BE( 0x400000, REGION_USER1, 0)
	ROM_LOAD16_BYTE( "epr-15345.2",  0x000000, 0x80000, CRC(d9e120c2) SHA1(b18b76733078d8534c6f0d8950632ab51e6a10ab) )
	ROM_LOAD16_BYTE( "epr-15344.1",  0x000001, 0x80000, CRC(8f8b9f74) SHA1(de6b923118bea60197547ad016cb5d5e1a8f372b) )
	ROM_LOAD16_BYTE( "mpr-14097-t.4",0x100000, 0x80000, CRC(4bd74cae) SHA1(5aa90bd5d2b8e2338ef0fe41d1f794e8d51321e1) )
	ROM_LOAD16_BYTE( "mpr-14096-t.3",0x100001, 0x80000, CRC(38d96502) SHA1(c68b3c5c83fd0839c3f6f81189c310ec19bdf1c4) )
ROM_END

ROM_START( dcclubj )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr13948.bin", 0x000000, 0x20000, CRC(d6a031c8) SHA1(45b7e3cd2c7412e24f547cd4185166199d3938d5) )
	ROM_LOAD16_BYTE( "epr13947.bin", 0x000001, 0x20000, CRC(7e3cff5e) SHA1(ff8cb776d2491796feeb8892c7e644e590438945) )

	ROM_REGION16_BE( 0x400000, REGION_USER1, 0)
	ROM_LOAD16_BYTE( "epr-14095a.2", 0x000000, 0x80000, CRC(88d184e9) SHA1(519F3A22E1619DE9D5F13A45B85EBD249EBFA979) )
	ROM_LOAD16_BYTE( "epr-14094a.1", 0x000001, 0x80000, CRC(7dd2b7d4) SHA1(C7EAF9E2700E0C55F7E867F5CD3FFAA5AAE97956) )
	ROM_LOAD16_BYTE( "mpr-14097-t.4",0x100000, 0x80000, CRC(4bd74cae) SHA1(5aa90bd5d2b8e2338ef0fe41d1f794e8d51321e1) )
	ROM_LOAD16_BYTE( "mpr-14096-t.3",0x100001, 0x80000, CRC(38d96502) SHA1(c68b3c5c83fd0839c3f6f81189c310ec19bdf1c4) )
ROM_END

ROM_START( dcclubfd )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION( 0x2000, REGION_USER3, 0 )	/* decryption key */
	ROM_LOAD( "317-0058-09d.key", 0x0000, 0x2000, CRC(a91ebffb) SHA1(70b8b4272ca456491f254d115b434bb4ce73f049) )

	ROM_REGION( 0x1c2000, REGION_USER2, 0)
	ROM_LOAD( "ds3-5000-09d.img", 0x000000, 0x1c2000,  CRC(69870887) SHA1(e47a997c2c783bf6670ab213ebe2ee35492eba34) )
ROM_END

ROM_START( roughrac )
	ROM_REGION( 0x40000, REGION_CPU1, 0 ) /* FD1094 code */
	ROM_LOAD16_BYTE( "epr-12187.ic2", 0x000000, 0x20000, CRC(e83783f3) SHA1(4b3b32df7de85aef9cd77c8a4ffc17e10466b638) )
	ROM_LOAD16_BYTE( "epr-12186.ic1", 0x000001, 0x20000, CRC(ce76319d) SHA1(0ede61f0700f9161285c768fa97636f0e42b96f8) )

	ROM_REGION( 0x2000, REGION_USER3, 0 )	/* decryption key */
	ROM_LOAD( "317-0058-06b.key",    0x000000, 0x2000, CRC(6a5bf536) SHA1(3fc3e93ce8a47d7ee86da889efad2e7eca6e2ee9) )

	ROM_REGION( 0x1c2000, REGION_USER2, 0)
	ROM_LOAD( "ds3-5000-06b.img",    0x000000, 0x1c2000, CRC(a7fb2149) SHA1(c04266ae31700b085ab45606aed83019a563de70) )
ROM_END

/* I think the letter after the disk code is the region
           'b' = Japan
           'c' = europe?
           'd' = US
*/

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

/* Disk Based Games */
/* 01 */GAME( 1988, hotrod,   0,        system24, hotrod,   hotrod,   ROT0,   "Sega", "Hot Rod (World, 3 Players, Turbo set 1, Floppy Based)", 0 )
/* 01 */GAME( 1988, hotroda,  hotrod,   system24, hotrod,   hotrod,   ROT0,   "Sega", "Hot Rod (World, 3 Players, Turbo set 2, Floppy Based)", GAME_NO_SOUND )
/* 01 */GAME( 1988, hotrodj,  hotrod,   system24, hotrodj,  hotrod,   ROT0,   "Sega", "Hot Rod (Japan, 4 Players, Floppy Based)", GAME_NO_SOUND )
/* 02 */GAME( 1988, sspirits, 0,        system24, sspirits, sspirits, ROT270, "Sega", "Scramble Spirits (World, Floppy Based)", 0 )
/* 02 */GAME( 1988, sspiritj, sspirits, system24, sspirits, sspiritj, ROT270, "Sega", "Scramble Spirits (Japan, Floppy DS3-5000-02-REV-A Based)", 0 )
/* 02 */GAME( 1988, sspirtfc, sspirits, system24, sspirits, sspirits, ROT270, "Sega", "Scramble Spirits (World, Floppy Based, FD1094 317-0058-02c)",GAME_NOT_WORKING ) /* MISSING disk image */
/* 03 */GAME( 1988, gground,  0,        system24, gground,  gground,  ROT270, "Sega", "Gain Ground (World, 3 Players, Floppy Based, FD1094 317-0058-03c?)", 0 )
/* 03 */GAME( 1988, ggroundj, gground,  system24, gground,  gground,  ROT270, "Sega", "Gain Ground (Japan, 2 Players, Floppy Based, FD1094 317-0058-03b)", 0 )
/* 04 */GAME( 1989, crkdown,  0,        system24, crkdown,  crkdown,  ROT0,   "Sega", "Crack Down (World, Floppy Based, FD1094 317-0058-04c)", GAME_IMPERFECT_GRAPHICS ) // clipping probs / solid layer probs? (radar display)
/* 04 */GAME( 1989, crkdownu, crkdown,  system24, crkdown,  crkdown,  ROT0,   "Sega", "Crack Down (US, Floppy Based, FD1094 317-0058-04d)", GAME_IMPERFECT_GRAPHICS ) // clipping probs / solid layer probs? (radar display)
/* 04 */GAME( 1989, crkdownj, crkdown,  system24, crkdown,  crkdown,  ROT0,   "Sega", "Crack Down (Japan, Floppy Based, FD1094 317-0058-04b)", GAME_IMPERFECT_GRAPHICS ) // clipping probs / solid layer probs? (radar display)
/* 05 */GAME( 1989, sgmast,   0,        system24, sgmast,   sgmast,   ROT0,   "Sega", "Super Masters Golf (World?, Floppy Based, FD1094 317-0058-05d?)", GAME_NOT_WORKING|GAME_UNEMULATED_PROTECTION ) // NOT decrypted
/* 05 */GAME( 1989, sgmastc,  sgmast,   system24, sgmast,   sgmast,   ROT0,   "Sega", "Jumbo Ozaki Super Masters Golf (World, Floppy Based, FD1094 317-0058-05c)", GAME_IMPERFECT_GRAPHICS ) // some gfx offset / colour probs?
/* 05 */GAME( 1989, sgmastj,  sgmast,   system24, sgmastj,  sgmast,   ROT0,   "Sega", "Jumbo Ozaki Super Masters Golf (Japan, Floppy Based, FD1094 317-0058-05b)", GAME_IMPERFECT_GRAPHICS ) // some gfx offset / colour probs?
/* 06 */GAME( 1990, roughrac, 0,        system24, roughrac, roughrac, ROT0,   "Sega", "Rough Racer (Japan, Floppy Based, FD1094 317-0058-06b)", 0 )
/* 07 */GAME( 1990, bnzabros, 0,        system24, bnzabros, bnzabros, ROT0,   "Sega", "Bonanza Bros (US, Floppy DS3-5000-07d? Based)", 0 )
/* 07 */GAME( 1990, bnzabrsj, bnzabros, system24, bnzabros, bnzabros, ROT0,   "Sega", "Bonanza Bros (Japan, Floppy DS3-5000-07b Based)", 0 )
/* 08 */GAME( 1991, qsww,     0,        system24, qsww,     qsww,     ROT0,   "Sega", "Quiz Syukudai wo Wasuremashita (Japan, Floppy Based, FD1094 317-0058-08b)", GAME_IMPERFECT_GRAPHICS ) // wrong bg colour on title
/* 09 */GAME( 1991, dcclubfd, dcclub,   system24, dcclub,   dcclubfd, ROT0,   "Sega", "Dynamic Country Club (US, Floppy Based, FD1094 317-0058-09d)", 0 )

/* ROM Based */
GAME( 1991, dcclub,   0,        system24, dcclub,   dcclub,   ROT0,   "Sega", "Dynamic Country Club (World, ROM Based)", 0 )
GAME( 1991, dcclubj,  dcclub,   system24, dcclub,   dcclub,   ROT0,   "Sega", "Dynamic Country Club (Japan, ROM Based)", 0 )
GAME( 1992, mahmajn,  0,        system24, mahmajn,  mahmajn,  ROT0,   "Sega", "Tokoro San no MahMahjan (Japan, ROM Based)", 0 )
GAME( 1994, qgh,      0,        system24, qgh,      qgh,      ROT0,   "Sega", "Quiz Ghost Hunter (Japan, ROM Based)", 0 )
GAME( 1994, quizmeku, 0,        system24, quizmeku, quizmeku, ROT0,   "Sega", "Quiz Mekurumeku Story (Japan, ROM Based)", 0 )
GAME( 1994, qrouka,   0,        system24, qrouka,   qrouka,   ROT0,   "Sega", "Quiz Rouka Ni Tattenasai (Japan, ROM Based)", 0 )
GAME( 1994, mahmajn2, 0,        system24, mahmajn,  mahmajn2, ROT0,   "Sega", "Tokoro San no MahMahjan 2 (Japan, ROM Based)", 0 )
