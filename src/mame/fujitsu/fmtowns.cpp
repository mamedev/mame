// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*

    Fujitsu FM-Towns
    driver by Barry Rodewald

    Japanese computer system released in 1989.

    CPU:  various AMD x86 CPUs, originally 80386DX (80387 available as an add-on).
          later models use 80386SX, 80486 and Pentium CPUs
    Sound:  Yamaha YM3438 (some later models are use YMF276; Low voltage variation of YM3438, needs External DAC)
            Ricoh RF5c68
            CD-DA
    Video:  Custom
            16 or 256 colours from a 24-bit palette, or 15-bit high-colour
            1024 sprites (16x16), rendered direct to VRAM
            16 colour text mode, rendered direct to VRAM

    Later models add an unknown single channel 16-bit PCM/ADPCM (FreshTV, SJ, MX), and CL-GD543x Windows accelerator chipsets (SJ)


    Fujitsu FM-Towns Marty

    Japanese console, based on the FM-Towns computer, using an AMD 80386SX CPU,
    released in 1993


    Issues: Video emulation is far from complete.

*/

/*

Regular models:

             Model              | Form factor |     CPU    |        Standard RAM          | Max RAM |      Standard FDDs     |         Standard HDD
FM Towns 1/2                    | Tower       | 386DX-16   | 1 MB (1) or 2 MB (2)         | 6 MB    | 1 or 2                 | No SCSI controller
FM Towns 1F/2F/1H/2H            | Tower       | 386DX-16   | 1 MB (1F/1H) or 2 MB (2F/2H) | 8 MB    | 1 (1F) or 2 (others)   | 20 or 40 MB (H models only)
FM Towns 10F/20F/40H/80H        | Tower       | 386DX-16   | 2 MB                         | 26 MB   | 1 (10F) or 2 (others)  | 40 or 85 MB (H models only)
FM Towns II CX10/20/40/100      | Tower       | 386DX-16   | 2 MB                         | 26 MB   | 1 (CX10) or 2 (others) | No (CX10/20), 40 or 100 MB
FM Towns II UX10/20/40          | All-in-one  | 386SX-16   | 2 MB                         | 10 MB   | 1 (UX10) or 2 (others) | No (UX10/20) or 40 MB
FM Towns II HG20/40/100         | Desktop     | 386DX-20   | 2 MB                         | 26 MB   | 2                      | No (HG20), 40 or 80 MB
FM Towns II HR20/100/200        | Desktop     | 486SX-20   | 4 MB                         | 28 MB   | 2                      | No (HR20) 100 or 200 MB
FM Towns II UG10/20/40/80       | All-in-one  | 386SX-20   | 2 MB                         | 10 MB   | 1 (UG10) or 2 (others) | No (UG10/20), 40 or 80 MB
FM Towns II UR20/40/80          | All-in-one  | 486SX-20   | 2 MB                         | 10 MB   | 2                      | No (UG20), 40 or 80 MB
FM Towns II ME20/170            | Desktop     | 486SX-25   | 2 MB                         | 66 MB   | 2                      | No (ME20) or 170 MB
FM Towns II MA170/340           | Desktop     | 486SX-33   | 4 MB                         | 100 MB  | 2                      | 170 or 340 MB
FM Towns II MX20/170/340        | Desktop     | 486DX2-66  | 4 MB                         | 100 MB  | 2                      | No (MX20), 170 or 340 MB
FM Towns II Fresh/MF20/MF170W   | Desktop     | 486SX-33   | 4 MB (MF20) or 6 MB (others) | 68 MB   | 2                      | No (MF20), or 170 (others)
FM Towns II MA170W/MA340W       | Desktop     | 486SX-33   | 8 MB                         | 100 MB  | 2                      | 170 or 340 MB
FM Towns II MX170W/MA340W       | Desktop     | 486DX2-66  | 8 MB                         | 100 MB  | 2                      | 170 or 340 MB
FM Towns II Fresh-TV            | Desktop     | 486SX-33   | 6 MB                         | 68 MB   | 2                      | 170 MB
FM Towns II Fresh-E             | Desktop     | 486DX2-66  | 8 MB                         | 72 MB   | 2                      | 260 MB
FM Towns II Fresh-T             | Desktop     | 486SX-33   | 8 MB                         | 72 MB   | 2                      | 260 MB
FM Towns II EA2                 | Desktop     | 486SX-33   | 4 MB                         | 68 MB   | 2                      | No
FM Towns II HA2/HA53            | Desktop     | 486DX2-66  | 4 MB (HA2) or 8 MB (HA53)    | 100 MB  | 2                      | No (HA2) or 530 MB
FM Towns II HB2/HB53/HB53M      | Desktop     | Pentium-60 | 8 MB                         | 136 MB  | 2                      | No (HB2) or 530 MB
FM Towns II Fresh-ES/Fresh-ET   | Desktop     | 486DX2-66  | 8 MB                         | 72 MB   | 1                      | 360 MB
FM Towns II HC53/HC53M          | Desktop     | Pentium-90 | 8 MB                         | 136 MB  | 1                      | 540 MB
FM Towns II Fresh-FS/Fresh-FT   | Desktop     | 486DX4-100 | 8 MB                         | 72 MB   | 1                      | 540 MB
FM Towns Marty/Marty 2/TC Marty | Console     | 386SX-16   | 2 MB                         | 4 MB    | 1                      | No SCSI controller
Car Marty                       | Car-mounted | 386SX-16   | 2 MB                         | ?       | No                     | No SCSI controller

Education models:

         Model          | Form factor |     CPU    |        Standard RAM          | Max RAM |      Standard FDDs     |         Standard HDD
FM Towns S1/S2          | Tower       | 386DX-16   | 1 MB (S1) or 2 MB (S2)       | 6 MB    | 1 or 2                 | No SCSI controller
FM Towns SF/SH          | Tower       | 386DX-16   | 1 MB (1F/1H) or 2 MB (2F/2H) | 8 MB    | 2                      | No (SF) or 40 MB
FM Towns SF2/SH2        | Tower       | 386DX-16   | 2 MB                         | 26 MB   | 2                      | No (SF2) or 40 MB
FM Towns II SG20/40     | Desktop     | 386DX-20   | 2 MB                         | 26 MB   | 2                      | No (SG20) or 40 MB
FM Towns II SR20/100    | Desktop     | 486SX-20   | 4 MB                         | 28 MB   | 2                      | No (SR20) or 100 MB
FM Towns II SE          | Desktop     | 486SX-25   | 2 MB                         | 66 MB   | 2                      | No
FM Towns II SA          | Desktop     | 486SX-33   | 4 MB                         | 100 MB  | 2                      | No
FM Towns II SF20/SF170W | Desktop     | 486SX-33   | 4 MB (SF20) or 6 MB (SF170W) | 100 MB  | 2                      | No (SF20) or 170 MB
FM Towns II SA170W      | Desktop     | 486SX-33   | 8 MB                         | 100 MB  | 2                      | 170 MB
FM Towns II SI2/SI26    | Desktop     | 486SX-33   | 4 MB (SI2) or 8 MB (SI26)    | 68 MB   | 2                      | No (SI2) or 260 MB
FM Towns II SJ2/SJ26    | Desktop     | 486DX2-66  | 4 MB (SJ2) or 8 MB (SJ26)    | 68 MB   | 2                      | No (SJ2) or 260 MB
FM Towns II SK53        | Desktop     | Pentium-60 | 8 MB                         | 136 MB  | 2                      | 530 MB
FM Towns II SN          | Laptop      | 486DX2-66  | 4 MB                         | 36 MB   | 1                      | 340 MB
FM Towns II SJ2A/SJ53   | Desktop     | 486DX2-66  | 4 MB (SJ2A) or 8 MB (SJ53)   | 68 MB   | 2                      | No (SJ2A) or 530 MB
FM Towns II SL53        | Desktop     | Pentium-90 | 8 MB                         | 136 MB  | 2                      | 530 MB

*/

/* I/O port map (incomplete, could well be incorrect too)
 *
 * 0x0000   : Master 8259 PIC
 * 0x0002   : Master 8259 PIC
 * 0x0010   : Slave 8259 PIC
 * 0x0012   : Slave 8259 PIC
 * 0x0020 RW: bit 0 = soft reset (read/write), bit 6 = power off (write), bit 7 = NMI vector protect
 * 0x0022  W: bit 7 = power off (write)
 * 0x0025 R : returns 0x00? (read)
 * 0x0026 R : timer?
 * 0x0028 RW: bit 0 = NMI mask (read/write)
 * 0x0030 R : Machine ID (low)
 * 0x0031 R : Machine ID (high)
 * 0x0032 RW: bit 7 = RESET, bit 6 = CLK, bit 0 = data (serial ROM)
 * 0x0040   : 8253 PIT counter 0
 * 0x0042   : 8253 PIT counter 1
 * 0x0044   : 8253 PIT counter 2
 * 0x0046   : 8253 PIT mode port
 * 0x0060   : 8253 PIT timer control
 * 0x006c RW: returns 0x00? (read) timer? (write)
 * 0x00a0-af: DMA controller 1 (uPD71071)
 * 0x00b0-bf: DMA controller 2 (uPD71071)
 * 0x0200-0f: Floppy controller (MB8877A)
 * 0x0400   : Video / CRTC (unknown)
 * 0x0404   : Disable VRAM, CMOS, memory-mapped I/O (everything in low memory except the BIOS)
 * 0x0440-5f: Video / CRTC
 * 0x0480 RW: bit 1 = disable BIOS ROM
 * 0x048a RW: JEIDA v3/v4(?) IC Memory card status
 * 0x0490 RW: JEIDA v4 IC Memory card page select
 * 0x0491 RW: JEIDA v4 IC Memory card
 * 0x04c0-cf: CD-ROM controller
 * 0x04d5   : Sound mute
 * 0x04d8   : YM3438 control port A / status
 * 0x04da   : YM3438 data port A / status
 * 0x04dc   : YM3438 control port B / status
 * 0x04de   : YM3438 data port B / status
 * 0x04e0-e3: volume ports
 * 0x04e9-ec: IRQ masks
 * 0x04f0-f8: RF5c68 registers
 * 0x05e8 R : RAM size in MB
 * 0x05ec RW: bit 0 = compatibility mode?
 * 0x0600 RW: Keyboard data port (8042)
 * 0x0602   : Keyboard control port (8042)
 * 0x0604   : (8042)
 * 0x0a00-0a: RS-232C interface (i8251)
 * 0x3000 - 0x3fff : CMOS RAM
 * 0xfd90-a0: CRTC / Video
 * 0xff81: CRTC / Video - returns value in RAM location 0xcff81?
 *
 * IRQ list
 *
 *      IRQ0 - PIT Timer IRQ
 *      IRQ1 - Keyboard
 *      IRQ2 - Serial Port
 *      IRQ6 - Floppy Disc Drive
 *      IRQ7 - PIC Cascade IRQ
 *      IRQ8 - SCSI controller
 *      IRQ9 - Built-in CD-ROM controller
 *      IRQ11 - VSync interrupt
 *      IRQ12 - Printer port
 *      IRQ13 - Sound (YM3438/RF5c68), Mouse
 *      IRQ15 - 16-bit PCM (expansion?)
 *
 * Machine ID list (I/O port 0x31)
 *
    1(01h)  FM-TOWNS 1/2
    2(02h)  FM-TOWNS 1F/2F/1H/2H
    3(03h)  FM-TOWNS 10F/20F/40H/80H
    4(04h)  FM-TOWNSII UX
    5(05h)  FM-TOWNSII CX
    6(06h)  FM-TOWNSII UG
    7(07h)  FM-TOWNSII HR
    8(08h)  FM-TOWNSII HG
    9(09h)  FM-TOWNSII UR
    11(0Bh) FM-TOWNSII MA
    12(0Ch) FM-TOWNSII MX
    13(0Dh) FM-TOWNSII ME
    14(0Eh) TOWNS Application Card (PS/V Vision)
    15(0Fh) FM-TOWNSII MF/Fresh/Fresh???TV
    16(10h) FM-TOWNSII SN
    17(11h) FM-TOWNSII HA/HB/HC
    19(13h) FM-TOWNSII EA/Fresh???T/Fresh???ET/Fresh???FT
    20(14h) FM-TOWNSII Fresh???E/Fresh???ES/Fresh???FS
    22(16h) FMV-TOWNS H/Fresh???GS/Fresh???GT/H2
    23(17h) FMV-TOWNS H20
    74(4Ah) FM-TOWNS MARTY
 */

/*

Fujitsu FM Towns Marty

PCB Layout
----------

CA20142-B21X

      CN10                                   CN1
   |-| |-| |-|                    |----------------------|
|--| |-| |-| |----|-|-------------|----------------------|------------|
|14577           CN11  SW2       |--------|  MROM.M37|--------|       |
|14576      TMS48C121DZ-80       |FUJITSU |          |FUJITSU |       |
|           TMS48C121DZ-80       |CG24243 |          |CS10501 |       |
|MB40968    TMS48C121DZ-80       |(QFP208)|  MROM.M36|(QFP160)|       |
||--------| TMS48C121DZ-80       |--------|          |--------| 62256 |
||FUJITSU | HM511664JP8    CN12                    FUJITSU            |
||CE31755 |                                        8451 (DIP8)        |
||(QFP160)|                                                           |
||--------|                                   4.9152MHz               |
|CN13            |--------|  28.63636MHz                            |-|
|                |FUJITSU |                  |----------------------|
|       RTC58323 |CG31553 |                  |                      |
|BATTERY         |(QFP208)|  32MHz   74LS00  |                      |
|                |--------|    |------|      |     CN2              |
|    LC7881            MB84256 |I386SX|      |     PCMCIA SLOT      |
|          TL084    CN4        | -16  |      |                      |
||--------|                    |------|      |                      |
||FUJITSU |    MB814400A-70PZ                |                      |
||CS09501 |    MB814400A-70PZ                |----------------------|
||(QFP120)|    MB814400A-70PZ              |--------|74LS14  74HC08 |-|
||--------|    MB814400A-70PZ              | RICOH  |      3771       |
|             YM3438                       |RU6101MF|                 |
|4560                           MB81C78    |(QFP44) |                 |
|             MB88505 |------|             |--------|                 |
|         CN3         |YM6063|  LED2                     LED1   LED3  |
|CN9  CN8     VOL     |(QFP80|      CN5        CN6    SW1          CN7|
|---------------------|------|----------------------------------------|
Notes:
      All IC's shown.
      Main CPU Intel 80386SX-16 running at 16.000MHz
      CN1   - Multi-pin connector possibly for cartridge or external peripheral connector?
      CN2   - PCMCIA slot
      CN3   - 24 pin connector for ?
      CN4   - CDROM connector? (multi-pin connector with internal slot for a thin cable)
      CN5/6 - DB9 connectors (joystick ports?)
      CN7   - Power input socket
      CN8/9 - Headphone out jacks?
      CN10  - Header with left/right RCA audio jacks and composite video output
      CN11  - S-VIDEO output
      CN12  - 8 pin connector (possibly power related for the CDROM?)
      CN13  - 2 pin connector (fan power?)
      SW1   - Reset Switch?
      SW2   - 2 position slide switch
      MROM* - Hitachi HN624116 16MBit SOP44 maskROM

*/

#include "emu.h"
#include "fmtowns.h"

#include "bus/scsi/scsi.h"
#include "bus/scsi/scsihd.h"

#include "screen.h"
#include "softlist.h"
#include "speaker.h"
#include <math.h>

#define LOG_SYS        (1U << 1)
#define LOG_CD         (1U << 2)
#define LOG_CD_UNKNOWN (1U << 3)

#define VERBOSE (LOG_GENERAL | LOG_CD_UNKNOWN)
#include "logmacro.h"


// CD controller IRQ types
#define TOWNS_CD_IRQ_MPU 1
#define TOWNS_CD_IRQ_DMA 2


enum
{
	MOUSE_START,
	MOUSE_SYNC,
	MOUSE_X_HIGH,
	MOUSE_X_LOW,
	MOUSE_Y_HIGH,
	MOUSE_Y_LOW
};


inline uint8_t towns_state::byte_to_bcd(uint8_t val)
{
	return ((val / 10) << 4) | (val % 10);
}

inline uint8_t towns_state::bcd_to_byte(uint8_t val)
{
	return (((val & 0xf0) >> 4) * 10) + (val & 0x0f);
}

inline uint32_t towns_state::msf_to_lbafm(uint32_t val)  // because the CDROM core doesn't provide this
{
	uint8_t m,s,f;
	f = bcd_to_byte(val & 0x0000ff);
	s = (bcd_to_byte((val & 0x00ff00) >> 8));
	m = (bcd_to_byte((val & 0xff0000) >> 16));
	return ((m * (60 * 75)) + (s * 75) + f) - 150;
}

void towns_state::init_serial_rom()
{
	// TODO: init serial ROM contents
	int x;
	static const uint8_t code[8] = { 0x04,0x65,0x54,0xA4,0x95,0x45,0x35,0x5F };
	uint8_t* srom = nullptr;

	if(m_serial)
		srom = m_serial->base();
	memset(m_towns_serial_rom.get(),0,256/8);

	if(srom)
	{
		memcpy(m_towns_serial_rom.get(),srom,32);
		m_towns_machine_id = (m_towns_serial_rom[0x18] << 8) | m_towns_serial_rom[0x17];
		logerror("Machine ID in serial ROM: %04x\n",m_towns_machine_id);
		return;
	}

	for(x=8;x<=21;x++)
		m_towns_serial_rom[x] = 0xff;

	for(x=0;x<=7;x++)
	{
		m_towns_serial_rom[x] = code[x];
	}

	// add Machine ID
	m_towns_machine_id = 0x0101;
	m_towns_serial_rom[0x17] = 0x01;
	m_towns_serial_rom[0x18] = 0x01;

	// serial number?
	m_towns_serial_rom[29] = 0x10;
	m_towns_serial_rom[28] = 0x6e;
	m_towns_serial_rom[27] = 0x54;
	m_towns_serial_rom[26] = 0x32;
	m_towns_serial_rom[25] = 0x10;
}

uint8_t towns_state::towns_system_r(offs_t offset)
{
	uint8_t ret = 0;

	switch(offset)
	{
		case 0x00:
			LOGMASKED(LOG_SYS, "SYS: port 0x20 read\n");
			return 0x00;
		case 0x05:
			LOGMASKED(LOG_SYS, "SYS: port 0x25 read\n");
			return 0x00;
/*      case 0x06:
            count = (m_towns_freerun_counter->elapsed() * ATTOSECONDS_TO_HZ(ATTOSECONDS_IN_USEC(1))).as_double();
            return count & 0xff;
        case 0x07:
            count = (m_towns_freerun_counter->elapsed() * ATTOSECONDS_TO_HZ(ATTOSECONDS_IN_USEC(1))).as_double();
            return (count >> 8) & 0xff;
*/      case 0x06:
			//LOGMASKED(LOG_SYS, "SYS: (0x26) timer read\n");
			return m_freerun_timer;
		case 0x07:
			return m_freerun_timer >> 8;
		case 0x08:
			//LOGMASKED(LOG_SYS, "SYS: (0x28) NMI mask read\n");
			return m_nmi_mask & 0x01;
		case 0x10:
			LOGMASKED(LOG_SYS, "SYS: (0x30) Machine ID read\n");
			return (m_towns_machine_id >> 8) & 0xff;
		case 0x11:
			LOGMASKED(LOG_SYS, "SYS: (0x31) Machine ID read\n");
			return m_towns_machine_id & 0xff;
		case 0x12:
			/* Bit 0 = data, bit 6 = CLK, bit 7 = RESET, bit 5 is always 1? */
			ret = (m_towns_serial_rom[m_towns_srom_position/8] & (1 << (m_towns_srom_position%8))) ? 1 : 0;
			ret |= m_towns_srom_clk;
			ret |= m_towns_srom_reset;
			//LOGMASKED(LOG_SYS, "SYS: (0x32) Serial ROM read [0x%02x, pos=%i]\n",ret,towns_srom_position);
			return ret;
		default:
			//LOGMASKED(LOG_SYS, "SYS: Unknown system port read (0x%02x)\n",offset+0x20);
			return 0x00;
	}
}

void towns_state::towns_system_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0x00:  // bit 7 = NMI vector protect, bit 6 = power off, bit 0 = software reset, bit 3 = A20 line?
//          space.m_maincpu->set_input_line(INPUT_LINE_A20,(data & 0x08) ? CLEAR_LINE : ASSERT_LINE);
			LOGMASKED(LOG_SYS, "SYS: port 0x20 write %02x\n",data);
			break;
		case 0x02:
			LOGMASKED(LOG_SYS, "SYS: (0x22) power port write %02x\n",data);
			break;
		case 0x08:
			//LOGMASKED(LOG_SYS, "SYS: (0x28) NMI mask write %02x\n",data);
			m_nmi_mask = data & 0x01;
			break;
		case 0x12:
			//LOGMASKED(LOG_SYS, "SYS: (0x32) Serial ROM write %02x\n",data);
			// clocks on low-to-high transition
			if((data & 0x40) && m_towns_srom_clk == 0) // CLK
			{  // advance to next bit
				m_towns_srom_position++;
			}
			if((data & 0x80) && m_towns_srom_reset == 0) // reset
			{  // reset to beginning
				m_towns_srom_position = 0;
			}
			m_towns_srom_clk = data & 0x40;
			m_towns_srom_reset = data & 0x80;
			break;
		default:
			LOGMASKED(LOG_SYS, "SYS: Unknown system port write 0x%02x (0x%02x)\n",data,offset);
			break;
	}
}

void towns_state::towns_intervaltimer2_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
	case 0x00:
		m_intervaltimer2_irqmask = data & 0x80;
		break;
	case 0x02:
		m_intervaltimer2_period = (m_intervaltimer2_period & 0xff00) | data;
		popmessage("Interval Timer 2 period changed to %04x",m_intervaltimer2_period);
		break;
	case 0x03:
		m_intervaltimer2_period = (data << 8) | (m_intervaltimer2_period & 0x00ff);
		popmessage("Interval Timer 2 period changed to %04x",m_intervaltimer2_period);
		break;
	}
}

uint8_t towns_state::towns_intervaltimer2_r(offs_t offset)
{
	uint8_t ret = 0;

	switch(offset)
	{
	case 0x00:
		if(m_intervaltimer2_timeout_flag != 0)
			ret |= 0x40;
		if(m_intervaltimer2_irqmask != 0)
			ret |= 0x80;
		m_intervaltimer2_timeout_flag = 0;  // flag reset on read
		return ret;
	case 0x02:
		return m_intervaltimer2_period & 0x00ff;
	case 0x03:
		return m_intervaltimer2_period >> 8;
	}
	return 0xff;
}

TIMER_CALLBACK_MEMBER(towns_state::freerun_inc)
{
	m_freerun_timer++;
}

TIMER_CALLBACK_MEMBER(towns_state::intervaltimer2_timeout)
{
	m_intervaltimer2_timeout_flag = 1;
}

TIMER_CALLBACK_MEMBER(towns_state::wait_end)
{
	m_maincpu->set_input_line(INPUT_LINE_HALT,CLEAR_LINE);
}

uint8_t towns_state::towns_sys6c_r()
{
	LOGMASKED(LOG_SYS, "SYS: (0x6c) Timer? read\n");
	return 0x00;
}

void towns_state::towns_sys6c_w(uint8_t data)
{
	// halts the CPU for 1 microsecond
	m_maincpu->set_input_line(INPUT_LINE_HALT,ASSERT_LINE);
	m_towns_wait_timer->adjust(attotime::from_usec(1),0,attotime::never);
}

template<int Chip>
uint8_t towns_state::towns_dma_r(offs_t offset)
{
	logerror("DMA#%01x: read register %i\n",Chip,offset);
	return m_dma[Chip]->read(offset);
}

template<int Chip>
void towns_state::towns_dma_w(offs_t offset, uint8_t data)
{
	logerror("DMA#%01x: wrote 0x%02x to register %i\n",Chip,data,offset);
	m_dma[Chip]->write(offset, data);
}

/*
 *  Floppy Disc Controller (MB8877A)
 */

void towns_state::mb8877a_irq_w(int state)
{
	if(m_towns_fdc_irq6mask == 0)
		state = 0;
	m_pic_master->ir6_w(state);  // IRQ6 = FDC
	if(IRQ_LOG) logerror("PIC: IRQ6 (FDC) set to %i\n",state);
}

void towns_state::mb8877a_drq_w(int state)
{
	m_dma[0]->dmarq(state, 0);
}

uint8_t towns_state::towns_floppy_r(offs_t offset)
{
	uint8_t ret;

	switch(offset)
	{
		case 0x00:
			return m_fdc->status_r();
		case 0x02:
			return m_fdc->track_r();
		case 0x04:
			return m_fdc->sector_r();
		case 0x06:
			return m_fdc->data_r();
		case 0x08:  // selected drive status?
			//logerror("FDC: read from offset 0x08\n");
			ret = 0x80;  // always set
			switch(m_towns_selected_drive)
			{
			case 1:
				ret |= 0x0c;
				if(m_flop[0]->get_device() && m_flop[0]->get_device()->exists())
					ret |= 0x03;
				break;
			case 2:
				ret |= 0x0c;
				if(m_flop[1]->get_device() && m_flop[1]->get_device()->exists())
					ret |= 0x03;
				break;
			case 3:
			case 4:
			case 0:
			default:
				break;
			}
			return ret;
		case 0x0e: // DRVCHG
			logerror("FDC: read from offset 0x0e\n");
			if(m_towns_selected_drive == 1)
				if (m_flop[0]->get_device())
					return m_flop[0]->get_device()->dskchg_r();
			if(m_towns_selected_drive == 2)
				if (m_flop[1]->get_device())
					return m_flop[1]->get_device()->dskchg_r();
			return 0x00;
		default:
			logerror("FDC: read from invalid or unimplemented register %02x\n",offset);
	}
	return 0xff;
}

void towns_state::towns_floppy_w(offs_t offset, uint8_t data)
{
	floppy_image_device* sel[4] = { m_flop[0]->get_device(), m_flop[1]->get_device(), nullptr, nullptr };

	switch(offset)
	{
		case 0x00:
			// Commands 0xd0 and 0xfe (Write Track) are apparently ignored?
			if(data == 0xd0)
				return;
			if(data == 0xfe)
				return;
			m_fdc->cmd_w(data);
			logerror("FDC: Command %02x\n",data);
			break;
		case 0x02:
			m_fdc->track_w(data);
			logerror("FDC: Track %02x\n",data);
			break;
		case 0x04:
			m_fdc->sector_w(data);
			logerror("FDC: Sector %02x\n",data);
			break;
		case 0x06:
			m_fdc->data_w(data);
			logerror("FDC: Data %02x\n",data);
			break;
		case 0x08:
		{
			// bit 5 - CLKSEL
			// docs are unclear about this but there's only one motor control line and turning on only the selected drive doesn't work properly.
			for(int i = 0; i < 4; i++)
			{
				if(sel[i] != nullptr)
				{
					sel[i]->mon_w((~data & 0x10)>>4);
					sel[i]->ss_w((data & 0x04)>>2);
				}
			}
			m_fdc->dden_w(BIT(~data, 1));

			m_towns_fdc_irq6mask = data & 0x01;
			//logerror("FDC: Config drive%i %02x\n",m_towns_selected_drive-1,data);

			break;
		}
		case 0x0c:  // drive select
			switch(data & 0x0f)
			{
				case 0x00:
					m_towns_selected_drive = 0;  // No drive selected
					break;
				case 0x01:
					m_towns_selected_drive = 1;
					if(sel[0] != nullptr)
						m_fdc->set_floppy(sel[0]);
					break;
				case 0x02:
					m_towns_selected_drive = 2;
					if(sel[1] != nullptr)
						m_fdc->set_floppy(sel[1]);
					break;
				case 0x04:
					m_towns_selected_drive = 3;
					if(sel[2] != nullptr)
						m_fdc->set_floppy(sel[2]);
					break;
				case 0x08:
					m_towns_selected_drive = 4;
					if(sel[3] != nullptr)
						m_fdc->set_floppy(sel[3]);
					break;
			}
			//logerror("FDC: drive select %02x\n",data);
			break;
		default:
			logerror("FDC: write %02x to invalid or unimplemented register %02x\n",data,offset);
	}
}

uint16_t towns_state::towns_fdc_dma_r()
{   uint16_t data = m_fdc->data_r();
	return data;
}

void towns_state::towns_fdc_dma_w(uint16_t data)
{
	m_fdc->data_w(data);
}

/*
 *  Port 0x600-0x607 - Keyboard controller (8042 MCU)
 *
 *  Sends two-byte code on each key press and release.
 *  First byte has the MSB set, and contains shift/ctrl/keyboard type flags
 *    Known bits:
 *      bit 7 = always 1
 *      bits 6-5 = keyboard type
 *        00 = thumb shift (NICOLA) keyboard
 *        01 = JIS keyboard
 *        10 = new JIS keyboard (with ALT key?)
 *        11 = extended use (?)
 *      bit 4 = key release
 *      bit 3 = ctrl
 *      bit 2 = shift
 *      bit 1 = left shift (thumb shift only)
 *      bit 0 = right shift (thumb shift only)
 *
 *  Second byte has the MSB reset, and contains the scancode of the key
 *  pressed or released.
 *      bit 7 = always 0
 *      bits 6-0 = key scancode
 */
void towns_state::kb_sendcode(uint8_t scancode, int release)
{
	switch(release)
	{
		case 0:  // key press
			m_towns_kb_output = 0xc0;
			m_towns_kb_extend = scancode & 0x7f;
			if (m_kb_ports[2]->read() & 0x00080000)
				m_towns_kb_output |= 0x04;
			if (m_kb_ports[2]->read() & 0x00040000)
				m_towns_kb_output |= 0x08;
			break;
		case 1:  // key release
			m_towns_kb_output = 0xd0;
			m_towns_kb_extend = scancode & 0x7f;
			if (m_kb_ports[2]->read() & 0x00080000)
				m_towns_kb_output |= 0x04;
			if (m_kb_ports[2]->read() & 0x00040000)
				m_towns_kb_output |= 0x08;
			break;
		case 2:  // extended byte
			m_towns_kb_output = scancode;
			m_towns_kb_extend = 0xff;
			break;
	}
	m_towns_kb_status |= 0x01;
	if(m_towns_kb_irq1_enable)
	{
		m_pic_master->ir1_w(1);
		if(IRQ_LOG) logerror("PIC: IRQ1 (keyboard) set high\n");
	}
	//logerror("KB: sending scancode 0x%02x\n",scancode);
}

TIMER_CALLBACK_MEMBER(towns_state::poll_keyboard)
{
	uint8_t scan = 0;
	for(int port = 0; port < 4; port++)
	{
		uint32_t portval = m_kb_ports[port]->read();
		for(int bit = 0; bit < 32; bit++)
		{
			if(BIT(portval, bit) != BIT(m_kb_prev[port], bit))
			{  // bit changed
				if(BIT(portval, bit) == 0)  // release
					kb_sendcode(scan, 1);
				else
					kb_sendcode(scan, 0);
			}
			scan++;
		}
		m_kb_prev[port] = portval;
	}
}

uint8_t towns_state::towns_keyboard_r(offs_t offset)
{
	uint8_t ret = 0x00;

	switch(offset)
	{
		case 0:  // scancode output
			ret = m_towns_kb_output;
			//logerror("KB: read keyboard output port, returning %02x\n",ret);
			m_pic_master->ir1_w(0);
			if(IRQ_LOG) logerror("PIC: IRQ1 (keyboard) set low\n");
			if(m_towns_kb_extend != 0xff)
			{
				kb_sendcode(m_towns_kb_extend,2);
			}
			else
				m_towns_kb_status &= ~0x01;
			return ret;
		case 1:  // status
			//logerror("KB: read status port, returning %02x\n",m_towns_kb_status);
			return m_towns_kb_status;
		default:
			logerror("KB: read offset %02x\n",offset);
	}
	return 0x00;
}

void towns_state::towns_keyboard_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0:  // command input
			m_towns_kb_status &= ~0x08;
			m_towns_kb_status |= 0x01;
			break;
		case 1:  // control
			m_towns_kb_status |= 0x08;
			break;
		case 2:  // IRQ1 enable
			m_towns_kb_irq1_enable = data & 0x01;
			break;
		default:
			logerror("KB: wrote 0x%02x to offset %02x\n",data,offset);
	}
}

/*
 *  Port 0x60 - PIT Timer control
 *  On read:    bit 0: Timer 0 output level
 *              bit 1: Timer 1 output level
 *              bits 4-2: Timer masks (timer 2 = beeper)
 *  On write:   bits 2-0: Timer mask set
 *              bit 7: Timer 0 output reset
 */
uint8_t towns_state::speaker_get_spk()
{
	return m_towns_spkrdata & m_pit_out2;
}


void towns_state::speaker_set_spkrdata(uint8_t data)
{
	m_towns_spkrdata = data ? 1 : 0;
	m_speaker->level_w(speaker_get_spk());
}


uint8_t towns_state::towns_port60_r()
{
	uint8_t val = 0x00;

	if (m_pit_out0)
		val |= 0x01;
	if (m_pit_out1)
		val |= 0x02;

	val |= (m_towns_timer_mask & 0x07) << 2;

	//logerror("PIT: port 0x60 read, returning 0x%02x\n",val);
	return val;
}

void towns_state::towns_port60_w(uint8_t data)
{
	if(data & 0x80)
	{
		//towns_pic_irq(dev,0);
		m_timer0 = 0;
		m_pic_master->ir0_w(m_timer0 || m_timer1);
	}
	m_towns_timer_mask = data & 0x07;

	speaker_set_spkrdata(data & 0x04);

	//logerror("PIT: wrote 0x%02x to port 0x60\n",data);
}

uint8_t towns_state::towns_sys5e8_r(offs_t offset)
{
	switch(offset)
	{
		case 0x00:
			LOGMASKED(LOG_SYS, "SYS: read RAM size port (%i)\n",m_ram->size());
			return m_ram->size()/1048576;
		case 0x02:
			LOGMASKED(LOG_SYS, "SYS: read port 5ec\n");
			return m_compat_mode & 0x01;
	}
	return 0x00;
}

void towns_state::towns_sys5e8_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0x00:
			LOGMASKED(LOG_SYS, "SYS: wrote 0x%02x to port 5e8\n",data);
			break;
		case 0x02:
			LOGMASKED(LOG_SYS, "SYS: wrote 0x%02x to port 5ec\n",data);
			m_compat_mode = data & 0x01;
			break;
	}
}

// Sound/LED control (I/O port 0x4e8-0x4ef)
// R/O  -- (0x4e9) FM IRQ flag (bit 0), PCM IRQ flag (bit 3)
// (0x4ea) PCM IRQ mask
// R/W  -- (0x4eb) PCM IRQ flag
// W/O  -- (0x4ec) LED control
uint8_t towns_state::towns_sound_ctrl_r(offs_t offset)
{
	uint8_t ret = 0;

	switch(offset)
	{
		case 0x00:
			ret = 1;
			break;
		case 0x01:
			if(m_towns_fm_irq_flag)
				ret |= 0x01;
			if(m_towns_pcm_irq_flag)
				ret |= 0x08;
			break;
		case 0x02:
			ret = m_towns_pcm_channel_mask;
			break;
		case 0x03:
			ret = m_towns_pcm_channel_flag;
			m_towns_pcm_channel_flag = 0;
			m_towns_pcm_irq_flag = 0;
			if(m_towns_fm_irq_flag == 0)
			{
				m_pic_slave->ir5_w(0);
				if(IRQ_LOG) logerror("PIC: IRQ13 (PCM) set low\n");
			}
			break;
//      default:
			//logerror("FM: unimplemented port 0x%04x read\n",offset + 0x4e8);
	}
	return ret;
}

void towns_state::towns_sound_ctrl_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0x02:  // PCM channel interrupt mask
			m_towns_pcm_channel_mask = data;
			break;
		default:
			logerror("FM: unimplemented port 0x%04x write %02x\n",offset + 0x4e8,data);
	}
}

// Controller ports
// Joysticks are multiplexed, with fire buttons available when bits 0 and 1 of port 0x4d6 are high. (bits 2 and 3 for second port)
uint8_t towns_state::towns_padport_r(offs_t offset)
{
	// Documentation indicates bit 7 is unused and should be ignored.
	// Tatsujin Ou expects it to read as zero to navigate menus.
	// Unclear whether it always reads as zero, or it's affected by something undocumented.
	unsigned const pad = BIT(offset, 1);
	return m_pad_ports[pad]->read() & (0x0f | (bitswap<3>(m_towns_pad_mask, pad + 4, (pad * 2) + 1, pad * 2) << 4));
}

void towns_state::towns_pad_mask_w(uint8_t data)
{
	m_towns_pad_mask = data;

	m_pad_ports[0]->pin_6_w(BIT(data, 0));
	m_pad_ports[0]->pin_7_w(BIT(data, 1));
	m_pad_ports[0]->pin_8_w(BIT(data, 4));

	m_pad_ports[1]->pin_6_w(BIT(data, 2));
	m_pad_ports[1]->pin_7_w(BIT(data, 3));
	m_pad_ports[1]->pin_8_w(BIT(data, 5));
}

uint8_t towns_state::towns_cmos_low_r(offs_t offset)
{
	if(m_towns_mainmem_enable != 0)
		return m_ram->pointer()[offset + 0xd8000];

	if(m_nvram)
		return m_nvram[offset >> 2] >> ((offset & 3) << 3);
	else
		return m_nvram16[offset >> 1] >> ((offset & 1) << 3);
}

void towns_state::towns_cmos_low_w(offs_t offset, uint8_t data)
{
	if(m_towns_mainmem_enable != 0)
		m_ram->pointer()[offset+0xd8000] = data;
	else
		if(m_nvram)
		{
			uint8_t shift = (offset & 3) << 3;
			m_nvram[offset >> 2] &= ~(0xff << shift);
			m_nvram[offset >> 2] |= (uint32_t)data << shift;
		}
		else
		{
			uint8_t shift = (offset & 1) << 3;
			m_nvram16[offset >> 1] &= ~(0xff << shift);
			m_nvram16[offset >> 1] |= (uint16_t)data << shift;
		}
}

uint8_t towns_state::towns_cmos_r(offs_t offset)
{
	if(m_nvram)
		return m_nvram[offset >> 2] >> ((offset & 3) << 3);
	else
		return m_nvram16[offset >> 1] >> ((offset & 1) << 3);
}

void towns_state::towns_cmos_w(offs_t offset, uint8_t data)
{
	if(m_nvram)
	{
		uint8_t shift = (offset & 3) << 3;
		m_nvram[offset >> 2] &= ~(0xff << shift);
		m_nvram[offset >> 2] |= (uint32_t)data << shift;
	}
	else
	{
		uint8_t shift = (offset & 1) << 3;
		m_nvram16[offset >> 1] &= ~(0xff << shift);
		m_nvram16[offset >> 1] |= (uint16_t)data << shift;
	}
}

void towns_state::towns_update_video_banks()
{
	uint8_t* ROM = m_user->base();

	if(m_towns_mainmem_enable != 0)  // first MB is RAM
	{
//      membank(1)->set_base(m_ram->pointer()+0xc0000);
//      membank(2)->set_base(m_ram->pointer()+0xc8000);
//      membank(3)->set_base(m_ram->pointer()+0xc9000);
//      membank(4)->set_base(m_ram->pointer()+0xca000);
//      membank(5)->set_base(m_ram->pointer()+0xca000);
//      membank(10)->set_base(m_ram->pointer()+0xca800);
		m_bank_cb000_r->set_base(m_ram->pointer()+0xcb000);
		m_bank_cb000_w->set_base(m_ram->pointer()+0xcb000);
		if(m_towns_system_port & 0x02)
			m_bank_f8000_r->set_base(m_ram->pointer()+0xf8000);
		else
			m_bank_f8000_r->set_base(ROM+0x238000);
		m_bank_f8000_w->set_base(m_ram->pointer()+0xf8000);
		return;
	}
	else  // enable I/O ports and VRAM
	{
//      membank(1)->set_base(towns_gfxvram+(towns_vram_rplane*0x8000));
//      membank(2)->set_base(towns_txtvram);
//      membank(3)->set_base(m_ram->pointer()+0xc9000);
//      if(towns_ankcg_enable != 0)
//          membank(4)->set_base(ROM+0x180000+0x3d000);  // ANK CG 8x8
//      else
//          membank(4)->set_base(towns_txtvram+0x2000);
//      membank(5)->set_base(towns_txtvram+0x2000);
//      membank(10)->set_base(m_ram->pointer()+0xca800);
		if(m_towns_ankcg_enable != 0)
			m_bank_cb000_r->set_base(ROM+0x180000+0x3d800);  // ANK CG 8x16
		else
			m_bank_cb000_r->set_base(m_ram->pointer()+0xcb000);
		m_bank_cb000_w->set_base(m_ram->pointer()+0xcb000);
		if(m_towns_system_port & 0x02)
			m_bank_f8000_r->set_base(m_ram->pointer()+0xf8000);
		else
			m_bank_f8000_r->set_base(ROM+0x238000);
		m_bank_f8000_w->set_base(m_ram->pointer()+0xf8000);
		return;
	}
}

uint8_t towns_state::towns_sys480_r()
{
	if(m_towns_system_port & 0x02)
		return 0x02;
	else
		return 0x00;
}

void towns_state::towns_sys480_w(uint8_t data)
{
	m_towns_system_port = data;
	m_towns_ram_enable = data & 0x02;
	towns_update_video_banks();
}

void towns_state::towns_video_404_w(uint8_t data)
{
	m_towns_mainmem_enable = data & 0x80;
	towns_update_video_banks();
}

uint8_t towns_state::towns_video_404_r()
{
	if(m_towns_mainmem_enable != 0)
		return 0x80;
	else
		return 0x00;
}

/*
 *  I/O ports 0x4c0-0x4cf
 *  CD-ROM driver (custom?)
 *
 *  0x4c0 - Status port (R/W)
 *    bit 7 - IRQ from sub MPU (reset when read)
 *    bit 6 - IRQ from DMA end (reset when read)
 *    bit 5 - Software transfer
 *    bit 4 - DMA transfer
 *    bit 1 - status read request
 *    bit 0 - ready
 *    Note: IRQ bits are only set high if the IRQ bit in the command byte is NOT set.
 *
 *  0x4c2 - Command port (R/W)
 *    On read, returns status byte (4 in total?)
 *    On write, performs specified command:
 *      bit 7 - command type
 *      bit 6 - IRQ
 *      bit 5 - status
 *      bits 4-0 - command
 *        Type=1:
 *          0 = set state
 *          1 = set state (CDDASET)
 *        Type=0:
 *          0 = Seek
 *          2 = Read (MODE1)
 *          5 = TOC Read
 *
 *  0x4c4 - Parameter port (R/W)
 *    Inserts a byte into an array of 8 bytes used for command parameters
 *    Writing to this port puts the byte at the front of the array, and
 *    pushes the other parameters back.
 *
 *  0x4c6 (W/O)
 *    bit 3 - software transfer mode
 *    bit 4 - DMA transfer mode
 *
 */
void towns_state::towns_cdrom_set_irq(int line,int state)
{
	switch(line)
	{
		case TOWNS_CD_IRQ_MPU:
			if(state != 0)
			{
				if(m_towns_cd.command & 0x40)
				{
//                  if(m_towns_cd.mpu_irq_enable)
					{
						m_towns_cd.status |= 0x80;
						m_pic_slave->ir1_w(1);
						if(IRQ_LOG) logerror("PIC: IRQ9 (CD-ROM) set high\n");
					}
				}
				else
					m_towns_cd.status |= 0x80;
			}
			else
			{
				m_towns_cd.status &= ~0x80;
				m_pic_slave->ir1_w(0);
				if(IRQ_LOG) logerror("PIC: IRQ9 (CD-ROM) set low\n");
			}
			break;
		case TOWNS_CD_IRQ_DMA:
			if(state != 0)
			{
				if(m_towns_cd.command & 0x40)
				{
//                  if(m_towns_cd.dma_irq_enable)
					{
						m_towns_cd.status |= 0x40;
						m_pic_slave->ir1_w(1);
						if(IRQ_LOG) logerror("PIC: IRQ9 (CD-ROM DMA) set high\n");
					}
				}
				else
					m_towns_cd.status |= 0x40;
			}
			else
			{
				m_towns_cd.status &= ~0x40;
				m_pic_slave->ir1_w(0);
				if(IRQ_LOG) logerror("PIC: IRQ9 (CD-ROM DMA) set low\n");
			}
			break;
	}
}

TIMER_CALLBACK_MEMBER(towns_state::towns_cd_status_ready)
{
	m_towns_cd.status |= 0x02;  // status read request
	m_towns_cd.status |= 0x01;  // ready
	m_towns_cd.cmd_status_ptr = 0;
	towns_cdrom_set_irq(TOWNS_CD_IRQ_MPU,1);
}

void towns_state::towns_cd_set_status(uint8_t st0, uint8_t st1, uint8_t st2, uint8_t st3)
{
	m_towns_cd.cmd_status[0] = st0;
	m_towns_cd.cmd_status[1] = st1;
	m_towns_cd.cmd_status[2] = st2;
	m_towns_cd.cmd_status[3] = st3;
	// wait a bit
	m_towns_status_timer->adjust(attotime::from_msec(1),0,attotime::never);
}

uint8_t towns_state::towns_cd_get_track()
{
	cdrom_image_device* cdrom = m_cdrom;
	uint32_t lba = m_cdda->get_audio_lba();
	uint8_t track;

	for(track=1;track<99;track++)
	{
		if(cdrom->get_track_start(track) > lba)
			break;
	}
	return track;
}

TIMER_CALLBACK_MEMBER(towns_state::towns_cdrom_read_byte)
{
	upd71071_device* device = m_dma_1.target();
	int masked;
	// TODO: support software transfers, for now DMA is assumed.

	if(m_towns_cd.buffer_ptr < 0) // transfer has ended
		return;

	masked = device->dmarq(param, 3);  // CD-ROM controller uses DMA1 channel 3
//  logerror("DMARQ: param=%i ret=%i bufferptr=%i\n",param,masked,m_towns_cd.buffer_ptr);
	if(param != 0)
	{
		m_towns_cd.read_timer->adjust(attotime::from_hz(300000));
	}
	else
	{
		if(masked != 0)  // check if the DMA channel is masked
		{
			m_towns_cd.read_timer->adjust(attotime::from_hz(300000),1);
			return;
		}
		if(m_towns_cd.buffer_ptr < 2048)
			m_towns_cd.read_timer->adjust(attotime::from_hz(300000),1);
		else
		{  // end of transfer
			m_towns_cd.status &= ~0x10;  // no longer transferring by DMA
			m_towns_cd.status &= ~0x20;  // no longer transferring by software
			LOGMASKED(LOG_CD, "DMA1: end of transfer (LBA=%08x)\n",m_towns_cd.lba_current);
			if(m_towns_cd.lba_current >= m_towns_cd.lba_last)
			{
				m_towns_cd.extra_status = 0;
				towns_cd_set_status(0x06,0x00,0x00,0x00);
				towns_cdrom_set_irq(TOWNS_CD_IRQ_DMA,1);
				m_towns_cd.buffer_ptr = -1;
				m_towns_cd.status |= 0x01;  // ready
			}
			else
			{
				m_towns_cd.extra_status = 0;
				towns_cd_set_status(0x22,0x00,0x00,0x00);
				towns_cdrom_set_irq(TOWNS_CD_IRQ_DMA,1);
				m_cdrom->read_data(++m_towns_cd.lba_current,m_towns_cd.buffer,cdrom_file::CD_TRACK_MODE1);
				m_towns_cd.read_timer->adjust(attotime::from_hz(300000),1);
				m_towns_cd.buffer_ptr = -1;
			}
		}
	}
}

uint8_t towns_state::towns_cdrom_read_byte_software()
{
	uint8_t ret;
	if(m_towns_cd.buffer_ptr < 0) // transfer has ended
		return 0x00;

	ret = m_towns_cd.buffer[m_towns_cd.buffer_ptr++];

	if(m_towns_cd.buffer_ptr >= 2048)
	{  // end of transfer
		m_towns_cd.status &= ~0x10;  // no longer transferring by DMA
		m_towns_cd.status &= ~0x20;  // no longer transferring by software
		LOGMASKED(LOG_CD, "CD: end of software transfer (LBA=%08x)\n",m_towns_cd.lba_current);
		if(m_towns_cd.lba_current >= m_towns_cd.lba_last)
		{
			m_towns_cd.extra_status = 0;
			towns_cd_set_status(0x06,0x00,0x00,0x00);
			towns_cdrom_set_irq(TOWNS_CD_IRQ_DMA,1);
			m_towns_cd.buffer_ptr = -1;
			m_towns_cd.status |= 0x01;  // ready
		}
		else
		{
			m_cdrom->read_data(++m_towns_cd.lba_current,m_towns_cd.buffer,cdrom_file::CD_TRACK_MODE1);
			m_towns_cd.extra_status = 0;
			towns_cd_set_status(0x21,0x00,0x00,0x00);
			towns_cdrom_set_irq(TOWNS_CD_IRQ_DMA,1);
			m_towns_cd.status &= ~0x10;
			m_towns_cd.status |= 0x20;
			m_towns_cd.buffer_ptr = -1;
		}
	}
	return ret;
}

void towns_state::towns_cdrom_read(cdrom_image_device* device)
{
	// MODE 1 read
	// load data into buffer to be sent via DMA1 channel 3
	// A set of status bytes is sent after each sector, and DMA is paused
	// so that the DMA controller than be set up again.
	// parameters:
	//          3 bytes: MSF of first sector to read
	//          3 bytes: MSF of last sector to read
	uint32_t lba1,lba2,track;

	lba1 = m_towns_cd.parameter[7] << 16;
	lba1 += m_towns_cd.parameter[6] << 8;
	lba1 += m_towns_cd.parameter[5];
	lba2 = m_towns_cd.parameter[4] << 16;
	lba2 += m_towns_cd.parameter[3] << 8;
	lba2 += m_towns_cd.parameter[2];
	m_towns_cd.lba_current = msf_to_lbafm(lba1);
	m_towns_cd.lba_last = msf_to_lbafm(lba2);

	track = device->get_track(m_towns_cd.lba_current);

	// parameter 7 = sector count?
	// lemmings 2 sets this to 4 but hates 4 extra sectors being read
//  if(m_towns_cd.parameter[1] != 0)
//      m_towns_cd.lba_last += m_towns_cd.parameter[1];

	LOGMASKED(LOG_CD, "CD: Mode 1 read from LBA next:%i last:%i track:%i\n",m_towns_cd.lba_current,m_towns_cd.lba_last,track);

	if(m_towns_cd.lba_current > m_towns_cd.lba_last)
	{
		m_towns_cd.extra_status = 0;
		towns_cd_set_status(0x01,0x00,0x00,0x00);
	}
	else
	{
		device->read_data(m_towns_cd.lba_current,m_towns_cd.buffer,cdrom_file::CD_TRACK_MODE1);
		if(m_towns_cd.software_tx)
		{
			m_towns_cd.status &= ~0x10;  // not a DMA transfer
			m_towns_cd.status |= 0x20;  // software transfer
		}
		else
		{
			m_towns_cd.status |= 0x10;  // DMA transfer begin
			m_towns_cd.status &= ~0x20;  // not a software transfer
		}
//      m_towns_cd.buffer_ptr = 0;
//      m_towns_cd.read_timer->adjust(attotime::from_hz(300000),1);
		if(m_towns_cd.command & 0x20)
		{
			m_towns_cd.extra_status = 2;
			towns_cd_set_status(0x00,0x00,0x00,0x00);
		}
		else
		{
			m_towns_cd.extra_status = 0;
			if(m_towns_cd.software_tx)
				towns_cd_set_status(0x21,0x00,0x00,0x00);
			else
				towns_cd_set_status(0x22,0x00,0x00,0x00);
		}
	}
}

void towns_state::towns_cdrom_play_cdda(cdrom_image_device* device)
{
	// PLAY AUDIO
	// Plays CD-DA audio from the specified MSF
	// Parameters:
	//          3 bytes: starting MSF of audio to play
	//          3 bytes: ending MSF of audio to play (can span multiple tracks)
	uint32_t lba1,lba2;

	lba1 = m_towns_cd.parameter[7] << 16;
	lba1 += m_towns_cd.parameter[6] << 8;
	lba1 += m_towns_cd.parameter[5];
	lba2 = m_towns_cd.parameter[4] << 16;
	lba2 += m_towns_cd.parameter[3] << 8;
	lba2 += m_towns_cd.parameter[2];
	m_towns_cd.cdda_current = msf_to_lbafm(lba1);
	m_towns_cd.cdda_length = msf_to_lbafm(lba2) - m_towns_cd.cdda_current + 1;

	m_cdda->start_audio(m_towns_cd.cdda_current,m_towns_cd.cdda_length);
	LOGMASKED(LOG_CD, "CD: CD-DA start from LBA:%i length:%i\n",m_towns_cd.cdda_current,m_towns_cd.cdda_length);
	if(m_towns_cd.command & 0x20)
	{
		m_towns_cd.extra_status = 1;
		towns_cd_set_status(0x00,0x03,0x00,0x00);
	}
}

TIMER_CALLBACK_MEMBER(towns_state::towns_delay_cdda)
{
	towns_cdrom_play_cdda(m_cdrom.target());
}

TIMER_CALLBACK_MEMBER(towns_state::towns_delay_seek)
{
	m_towns_cd.extra_status = 0;
	towns_cd_set_status(0x04,0x00,0x00,0x00);
}

void towns_state::towns_cdrom_execute_command(cdrom_image_device* device)
{
	towns_cdrom_set_irq(TOWNS_CD_IRQ_MPU,0); // TODO: this isn't sufficiently tested
	m_towns_seek_timer->adjust(attotime::never);
	if(!device->exists() && (m_towns_cd.command != 0xa0))
	{  // No CD in drive
		if(m_towns_cd.command & 0x20)
		{
			m_towns_cd.extra_status = 0;
			towns_cd_set_status(0x10,0x00,0x00,0x00);
		}
	}
	else
	{
		m_towns_cd.status &= ~0x02;
		switch(m_towns_cd.command & 0x9f)
		{
			case 0x00:  // Seek
				if(m_towns_cd.command & 0x20)
				{
					m_towns_cd.extra_status = 0;
					towns_cd_set_status(0x00,0x00,0x00,0x00);
					m_towns_seek_timer->adjust(attotime::from_msec(500));
				}
				LOGMASKED(LOG_CD, "CD: Command 0x00: SEEK\n");
				break;
			case 0x01:  // unknown
				if(m_towns_cd.command & 0x20)
				{
					m_towns_cd.extra_status = 0;
					towns_cd_set_status(0x00,0xff,0xff,0xff);
				}
				LOGMASKED(LOG_CD, "CD: Command 0x01: unknown\n");
				break;
			case 0x02:  // Read (MODE1)
				LOGMASKED(LOG_CD, "CD: Command 0x02: READ MODE1\n");
				towns_cdrom_read(device);
				break;
			case 0x04:  // Play Audio Track
				LOGMASKED(LOG_CD, "CD: Command 0x04: PLAY CD-DA\n");
				m_towns_cdda_timer->adjust(attotime::from_msec(1),0,attotime::never);
				break;
			case 0x05:  // Read TOC
				LOGMASKED(LOG_CD, "CD: Command 0x05: READ TOC\n");
				if(m_towns_cd.command & 0x20)
				{
					m_towns_cd.extra_status = 1;
					towns_cd_set_status(0x00,0x00,0x00,0x00);
				}
				else
				{
					m_towns_cd.extra_status = 2;
					towns_cd_set_status(0x16,0x00,0xa0,0x00);
				}
				break;
			case 0x06:  // Read CD-DA state?
				LOGMASKED(LOG_CD, "CD: Command 0x06: READ CD-DA STATE\n");
				m_towns_cd.extra_status = 1;
				towns_cd_set_status(0x00,0x00,0x00,0x00);
				break;
			case 0x1f:  // unknown
				LOGMASKED(LOG_CD, "CD: Command 0x1f: unknown\n");
				m_towns_cd.extra_status = 0;
				towns_cd_set_status(0x00,0x00,0x00,0x00);
				break;
			case 0x80:  // set state
				LOGMASKED(LOG_CD, "CD: Command 0x80: set state\n");
				if(m_towns_cd.command & 0x20)
				{
					m_towns_cd.extra_status = 0;
					if(m_cdda->audio_active() && !m_cdda->audio_paused())
						towns_cd_set_status(0x00,0x03,0x00,0x00);
					else
						towns_cd_set_status(0x00,0x01,0x00,0x00);

				}
				break;
			case 0x81:  // set state (CDDASET)
				if(m_towns_cd.command & 0x20)
				{
					m_towns_cd.extra_status = 0;
					towns_cd_set_status(0x00,0x00,0x00,0x00);
				}
				LOGMASKED(LOG_CD, "CD: Command 0x81: set state (CDDASET)\n");
				break;
			case 0x84:   // Stop CD audio track  -- generates no status output?
				if(m_towns_cd.command & 0x20)
				{
					m_towns_cd.extra_status = 1;
					towns_cd_set_status(0x00,0x00,0x00,0x00);
				}
				m_cdda->pause_audio(1);
				LOGMASKED(LOG_CD, "CD: Command 0x84: STOP CD-DA\n");
				break;
			case 0x85:   // Stop CD audio track (difference from 0x84?)
				if(m_towns_cd.command & 0x20)
				{
					m_towns_cd.extra_status = 1;
					towns_cd_set_status(0x00,0x00,0x00,0x00);
				}
				m_cdda->pause_audio(1);
				LOGMASKED(LOG_CD, "CD: Command 0x85: STOP CD-DA\n");
				break;
			case 0x87:  // Resume CD-DA playback
				if(m_towns_cd.command & 0x20)
				{
					m_towns_cd.extra_status = 1;
					towns_cd_set_status(0x00,0x03,0x00,0x00);
				}
				m_cdda->pause_audio(0);
				LOGMASKED(LOG_CD, "CD: Command 0x87: RESUME CD-DA\n");
				break;
			default:
				m_towns_cd.extra_status = 0;
				towns_cd_set_status(0x10,0x00,0x00,0x00);
				LOGMASKED(LOG_CD_UNKNOWN, "CD: Unknown or unimplemented command %02x\n",m_towns_cd.command);
				break;
		}
	}
}

uint16_t towns_state::towns_cdrom_dma_r()
{
	if(m_towns_cd.buffer_ptr >= 2048)
		return 0x00;
	return m_towns_cd.buffer[m_towns_cd.buffer_ptr++];
}

uint8_t towns_state::towns_cdrom_r(offs_t offset)
{
	uint32_t addr = 0;
	uint8_t ret = 0;

	ret = m_towns_cd.cmd_status[m_towns_cd.cmd_status_ptr];

	switch(offset)
	{
		case 0x00:  // status
			//LOGMASKED(LOG_CD, "CD: status read, returning %02x\n",towns_cd.status);
			return m_towns_cd.status;
		case 0x01:  // command status
			if(m_towns_cd.cmd_status_ptr >= 3)
			{
				m_towns_cd.status &= ~2;
				// check for more status bytes
				if(m_towns_cd.extra_status != 0)
				{
					switch(m_towns_cd.command & 0x9f)
					{
						case 0x02:  // read
							if(m_towns_cd.extra_status == 2)
								towns_cd_set_status(0x22,0x00,0x00,0x00);
							m_towns_cd.extra_status = 0;
							break;
						case 0x04:  // play cdda
							if(m_cdda->audio_ended())
								towns_cd_set_status(0x07,0x00,0x00,0x00);
							else
								towns_cd_set_status(0x00,0x00,0x03,0x00);
							m_towns_cd.status &= ~2;
							m_towns_cd.extra_status = 0;
							break;
						case 0x05:  // read toc
							switch(m_towns_cd.extra_status)
							{
								case 1:
									towns_cd_set_status(0x16,0x00,0xa0,0x00);
									m_towns_cd.extra_status++;
									break;
								case 2: // st1 = first track number (BCD)
									towns_cd_set_status(0x17,0x01,0x00,0x00);
									m_towns_cd.extra_status++;
									break;
								case 3:
									towns_cd_set_status(0x16,0x00,0xa1,0x00);
									m_towns_cd.extra_status++;
									break;
								case 4: // st1 = last track number (BCD)
									towns_cd_set_status(0x17,
										byte_to_bcd(m_cdrom->get_last_track()),
										0x00,0x00);
									m_towns_cd.extra_status++;
									break;
								case 5:
									towns_cd_set_status(0x16, 0x00, 0xa2, 0x00);
									m_towns_cd.extra_status++;
									break;
								case 6:  // st1/2/3 = address of track 0xaa? (BCD)
									addr = m_cdrom->get_track_start(0xaa);
									addr = cdrom_file::lba_to_msf(addr + 150);
									towns_cd_set_status(0x17,
										(addr & 0xff0000) >> 16,(addr & 0x00ff00) >> 8,addr & 0x0000ff);
									m_towns_cd.extra_status++;
									break;
								default:
									if(m_towns_cd.extra_status & 0x01)
									{
										towns_cd_set_status(0x16,
											((m_cdrom->get_adr_control((m_towns_cd.extra_status/2)-3) & 0x0f) << 4)
											| ((m_cdrom->get_adr_control((m_towns_cd.extra_status/2)-3) & 0xf0) >> 4),
											byte_to_bcd((m_towns_cd.extra_status/2)-2),0x00);
										m_towns_cd.extra_status++;
									}
									else
									{
										int track = (m_towns_cd.extra_status/2)-4;
										addr = m_cdrom->get_track_start(track);
										addr = cdrom_file::lba_to_msf(addr + 150);
										towns_cd_set_status(0x17,
											(addr & 0xff0000) >> 16,(addr & 0x00ff00) >> 8,addr & 0x0000ff);
										if(track >= m_cdrom->get_last_track())
										{
											m_towns_cd.extra_status = 0;
										}
										else
											m_towns_cd.extra_status++;
									}
									break;
							}
							break;
						case 0x06:  // read CD-DA state
							switch(m_towns_cd.extra_status)
							{
								case 1:  // st2 = track number
									towns_cd_set_status(0x18,
										0x00,towns_cd_get_track(),0x00);
									m_towns_cd.extra_status++;
									break;
								case 2:  // st0/1/2 = MSF from beginning of current track
									addr = m_cdda->get_audio_lba();
									addr = cdrom_file::lba_to_msf(addr - m_towns_cd.cdda_current);
									towns_cd_set_status(0x19,
										(addr & 0xff0000) >> 16,(addr & 0x00ff00) >> 8,addr & 0x0000ff);
									m_towns_cd.extra_status++;
									break;
								case 3:  // st1/2 = current MSF
									addr = m_cdda->get_audio_lba();
									addr = cdrom_file::lba_to_msf(addr);  // this data is incorrect, but will do until exact meaning is found
									towns_cd_set_status(0x19,
										0x00,(addr & 0xff0000) >> 16,(addr & 0x00ff00) >> 8);
									m_towns_cd.extra_status++;
									break;
								case 4:
									addr = m_cdda->get_audio_lba();
									addr = cdrom_file::lba_to_msf(addr);  // this data is incorrect, but will do until exact meaning is found
									towns_cd_set_status(0x20,
										addr & 0x0000ff,0x00,0x00);
									m_towns_cd.extra_status = 0;
									break;
							}
							break;
						case 0x84:
							towns_cd_set_status(0x11,0x00,0x00,0x00);
							m_towns_cd.extra_status = 0;
							break;
						case 0x85:
							towns_cd_set_status(0x12,0x00,0x00,0x00);
							m_towns_cd.extra_status = 0;
							break;
					}
				}
				else
					m_towns_cd.status &= ~0x02;
			}
			LOGMASKED(LOG_CD, "CD: reading command status port (%i), returning %02x\n",m_towns_cd.cmd_status_ptr,ret);
			m_towns_cd.cmd_status_ptr++;
			if(m_towns_cd.cmd_status_ptr > 3)
			{
				m_towns_cd.cmd_status_ptr = 0;
/*              if(m_towns_cd.extra_status != 0)
                {
                    towns_cdrom_set_irq(machine(),TOWNS_CD_IRQ_MPU,1);
                    m_towns_cd.status |= 0x02;
                }*/
			}
			return ret;
		case 0x02:  // data transfer (used in software transfers)
			if(m_towns_cd.software_tx)
			{
				return towns_cdrom_read_byte_software();
			}
			[[fallthrough]];
		default:
			return 0x00;
	}
}

void towns_state::towns_cdrom_w(offs_t offset, uint8_t data)
{
	int x;
	switch(offset)
	{
		case 0x00: // status
			if(data & 0x80)
				towns_cdrom_set_irq(TOWNS_CD_IRQ_MPU,0);
			if(data & 0x40)
				towns_cdrom_set_irq(TOWNS_CD_IRQ_DMA,0);
			if(data & 0x04)
				LOG("CD: sub MPU reset\n");
			m_towns_cd.mpu_irq_enable = data & 0x02;
			m_towns_cd.dma_irq_enable = data & 0x01;
			LOGMASKED(LOG_CD, "CD: status write %02x\n",data);
			break;
		case 0x01: // command
			m_towns_cd.command = data;
			towns_cdrom_execute_command(m_cdrom);
			LOGMASKED(LOG_CD, "CD: command %02x sent\n",data);
			LOGMASKED(LOG_CD, "CD: parameters: %02x %02x %02x %02x %02x %02x %02x %02x\n",
				m_towns_cd.parameter[7],m_towns_cd.parameter[6],m_towns_cd.parameter[5],
				m_towns_cd.parameter[4],m_towns_cd.parameter[3],m_towns_cd.parameter[2],
				m_towns_cd.parameter[1],m_towns_cd.parameter[0]);
			break;
		case 0x02: // parameter
			for(x=7;x>0;x--)
				m_towns_cd.parameter[x] = m_towns_cd.parameter[x-1];
			m_towns_cd.parameter[0] = data;
			LOGMASKED(LOG_CD, "CD: parameter %02x added\n",data);
			break;
		case 0x03:
			if(data & 0x08)  // software transfer
			{
				m_towns_cd.status &= ~0x10;  // no DMA transfer
				m_towns_cd.status |= 0x20;
				m_towns_cd.software_tx = true;
				m_towns_cd.buffer_ptr = 0;
			}
			if(data & 0x10)
			{
				m_towns_cd.status |= 0x10;  // DMA transfer begin
				m_towns_cd.status &= ~0x20;  // not a software transfer
				m_towns_cd.software_tx = false;
				if(m_towns_cd.buffer_ptr < 0)
				{
					m_towns_cd.buffer_ptr = 0;
					m_towns_cd.read_timer->adjust(attotime::from_hz(300000),1);
				}
			}
			LOGMASKED(LOG_CD, "CD: transfer mode write %02x\n",data);
			break;
		default:
			LOGMASKED(LOG_CD, "CD: write %02x to port %02x\n",data,offset*2);
			break;
	}
}


/* CMOS RTC
 * 0x70: Data port
 * 0x80: Register select
 */
uint8_t towns_state::towns_rtc_r()
{
	return (m_rtc_busy ? 0 : 0x80) | m_rtc_d;
}

void towns_state::towns_rtc_w(uint8_t data)
{
	m_rtc->d0_w(BIT(data, 0));
	m_rtc->d1_w(BIT(data, 1));
	m_rtc->d2_w(BIT(data, 2));
	m_rtc->d3_w(BIT(data, 3));
}

void towns_state::towns_rtc_select_w(uint8_t data)
{
	m_rtc->cs1_w(BIT(data, 7));
	m_rtc->cs2_w(BIT(data, 7));
	m_rtc->read_w(BIT(data, 2));
	m_rtc->write_w(BIT(data, 1));
	m_rtc->address_write_w(BIT(data, 0));
}

void towns_state::rtc_d0_w(int state)
{
	m_rtc_d = (m_rtc_d & ~1) | (state ? 1 : 0);
}

void towns_state::rtc_d1_w(int state)
{
	m_rtc_d = (m_rtc_d & ~2) | (state ? 2 : 0);
}

void towns_state::rtc_d2_w(int state)
{
	m_rtc_d = (m_rtc_d & ~4) | (state ? 4 : 0);
}

void towns_state::rtc_d3_w(int state)
{
	m_rtc_d = (m_rtc_d & ~8) | (state ? 8 : 0);
}

void towns_state::rtc_busy_w(int state)
{
	// active low output
	m_rtc_busy = !state;
}

// SCSI controller - I/O ports 0xc30 and 0xc32
void towns_state::towns_scsi_irq(int state)
{
	m_pic_slave->ir0_w(state);
	if(IRQ_LOG)
		logerror("PIC: IRQ8 (SCSI) set to %i\n",state);
}

void towns_state::towns_scsi_drq(int state)
{
	m_dma[0]->dmarq(state, 1);  // SCSI HDs use channel 1
}


// Volume ports - I/O ports 0x4e0-0x4e3
// 0x4e0 = input volume level
// 0x4e1 = input channel select
//         0 = Line in, left channel
//         1 = Line in, right channel
// 0x4e2 = output volume level
// 0x4e3 = output channel select
//         0 = CD-DA left channel
//         1 = CD-DA right channel
//         2 = MIC
//         3 = MODEM
uint8_t towns_state::towns_volume_r(offs_t offset)
{
	switch(offset)
	{
	case 2:
		return(m_towns_volume[m_towns_volume_select & 3]);
	case 3:
		return m_towns_volume_select;
	default:
		return 0;
	}
}

void towns_state::cdda_db_to_gain(float db)
{
	float gain = powf(10, db / 20.0f);
	int port = m_towns_volume_select & 3;
	if(port > 1)
		return;
	if(db > 0)
		gain = 0;
	m_cdda->set_output_gain(port, gain);
}

void towns_state::towns_volume_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
	case 2:
		m_towns_volume[m_towns_volume_select & 3] = data;
		if(!(m_towns_volume_select & 4) || (m_towns_volume_select & 0x18))
			return;
		cdda_db_to_gain((~data & 0x3f) * -0.5f);
		break;
	case 3:  // select channel
		m_towns_volume_select = data;
		if(!(data & 4))
			cdda_db_to_gain(1);
		else if(data & 8)
			cdda_db_to_gain(0);
		else if(data & 0x10)
			cdda_db_to_gain(-32.0f);
		break;
	default:
		logerror("SND: Volume port %i set to %02x\n",offset,data);
	}
}

uint8_t towns_state::unksnd_r()
{
	return 0;
}

// some unknown ports...
uint8_t towns_state::towns_41ff_r()
{
	logerror("I/O port 0x41ff read\n");
	return 0x01;
}

// YM3438 interrupt (IRQ 13)
void towns_state::towns_fm_irq(int state)
{
	if(state)
	{
		m_towns_fm_irq_flag = 1;
		m_pic_slave->ir5_w(1);
		if(IRQ_LOG) logerror("PIC: IRQ13 (FM) set high\n");
	}
	else
	{
		m_towns_fm_irq_flag = 0;
		if(m_towns_pcm_irq_flag == 0)
		{
			m_pic_slave->ir5_w(0);
			if(IRQ_LOG) logerror("PIC: IRQ13 (FM) set low\n");
		}
	}
}

// PCM interrupt (IRQ 13)
RF5C68_SAMPLE_END_CB_MEMBER(towns_state::towns_pcm_irq)
{
	if (m_towns_pcm_channel_mask & (1 << channel))
	{
		m_towns_pcm_irq_flag = 1;
		m_towns_pcm_channel_flag |= (1 << channel);
		m_pic_slave->ir5_w(1);
		if(IRQ_LOG) logerror("PIC: IRQ13 (PCM) set high (channel %i)\n",channel);
	}
}

void towns_state::towns_pit_out0_changed(int state)
{
	m_pit_out0 = state;

	if(m_towns_timer_mask & 0x01)
	{
		m_timer0 = state;
		if(IRQ_LOG) logerror("PIC: IRQ0 (PIT Timer ch0) set to %i\n",state);
	}
	else
		m_timer0 = 0;

	m_pic_master->ir0_w(m_timer0 || m_timer1);
}

void towns_state::towns_pit_out1_changed(int state)
{
	m_pit_out1 = state;

	if(m_towns_timer_mask & 0x02)
	{
		m_timer1 = state;
		if(IRQ_LOG) logerror("PIC: IRQ0 (PIT Timer ch1) set to %i\n",state);
	}
	else
		m_timer1 = 0;

	m_pic_master->ir0_w(m_timer0 || m_timer1);
}

void towns_state::pit_out2_changed(int state)
{
	m_pit_out2 = state ? 1 : 0;
	m_speaker->level_w(speaker_get_spk());
}

void towns_state::pit2_out1_changed(int state)
{
	m_i8251->write_rxc(state);
	m_i8251->write_txc(state);
}

void towns_state::towns_serial_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0:
		case 1:
			m_i8251->write(offset, data);
			break;
		case 4:
			m_serial_irq_enable = data;
			break;
		default:
			logerror("Invalid or unimplemented serial port write [offset=%02x, data=%02x]\n",offset,data);
	}
}

uint8_t towns_state::towns_serial_r(offs_t offset)
{
	switch(offset)
	{
		case 0:
		case 1:
			return m_i8251->read(offset);
		case 3:
			return m_serial_irq_source;
		default:
			logerror("Invalid or unimplemented serial port read [offset=%02x]\n",offset);
			return 0xff;
	}
}

void towns_state::towns_serial_irq(int state)
{
	m_serial_irq_source = state ? 0x01 : 0x00;
	m_pic_master->ir2_w(state);
	popmessage("Serial IRQ state: %i\n",state);
}

void towns_state::towns_rxrdy_irq(int state)
{
	if(m_serial_irq_enable & RXRDY_IRQ_ENABLE)
		towns_serial_irq(state);
}

void towns_state::towns_txrdy_irq(int state)
{
	if(m_serial_irq_enable & TXRDY_IRQ_ENABLE)
		towns_serial_irq(state);
}

void towns_state::towns_syndet_irq(int state)
{
	if(m_serial_irq_enable & SYNDET_IRQ_ENABLE)
		towns_serial_irq(state);
}


void towns_state::towns_mem(address_map &map)
{
	// memory map based on FM-Towns/Bochs (Bochs modified to emulate the FM-Towns)
	// may not be (and probably is not) correct
	map(0x00000000, 0x000bffff).ram();
	map(0x000c0000, 0x000c7fff).rw(FUNC(towns_state::towns_gfx_r), FUNC(towns_state::towns_gfx_w));
	map(0x000c8000, 0x000cafff).rw(FUNC(towns_state::towns_spriteram_low_r), FUNC(towns_state::towns_spriteram_low_w));
	map(0x000cb000, 0x000cbfff).bankr("bank_cb000_r").bankw("bank_cb000_w");
	map(0x000cc000, 0x000cff7f).ram();
	map(0x000cff80, 0x000cffff).rw(FUNC(towns_state::towns_video_cff80_mem_r), FUNC(towns_state::towns_video_cff80_mem_w));
	map(0x000d0000, 0x000d7fff).ram();
	map(0x000d8000, 0x000d9fff).rw(FUNC(towns_state::towns_cmos_low_r), FUNC(towns_state::towns_cmos_low_w)).share("nvram"); // CMOS? RAM
	map(0x000da000, 0x000effff).ram(); //READWRITE(SMH_BANK(11),SMH_BANK(11))
	map(0x000f0000, 0x000f7fff).ram(); //READWRITE(SMH_BANK(12),SMH_BANK(12))
	map(0x000f8000, 0x000fffff).bankr("bank_f8000_r").bankw("bank_f8000_w");
	map(0x80000000, 0x8007ffff).rw(FUNC(towns_state::towns_gfx_high_r), FUNC(towns_state::towns_gfx_high_w)).mirror(0x80000); // VRAM
	map(0x80100000, 0x8017ffff).rw(FUNC(towns_state::towns_gfx_packed_r), FUNC(towns_state::towns_gfx_packed_w)).mirror(0x80000); // VRAM
	map(0x81000000, 0x8101ffff).rw(FUNC(towns_state::towns_spriteram_r), FUNC(towns_state::towns_spriteram_w)); // Sprite RAM
	map(0xc0000000, 0xc0ffffff).rw(m_icmemcard, FUNC(fmt_icmem_device::static_mem_read), FUNC(fmt_icmem_device::static_mem_write));
	map(0xc1000000, 0xc1ffffff).rw(m_icmemcard, FUNC(fmt_icmem_device::mem_read), FUNC(fmt_icmem_device::mem_write));
	map(0xc2000000, 0xc207ffff).rom().region("user", 0x000000);  // OS ROM
	map(0xc2080000, 0xc20fffff).rom().region("user", 0x100000);  // DIC ROM
	map(0xc2100000, 0xc213ffff).rom().region("user", 0x180000);  // FONT ROM
	map(0xc2140000, 0xc2141fff).rw(FUNC(towns_state::towns_cmos_r), FUNC(towns_state::towns_cmos_w)); // CMOS (mirror?)
	map(0xc2180000, 0xc21fffff).rom().region("user", 0x080000);  // F20 ROM
	map(0xc2200000, 0xc2200fff).rw("pcm", FUNC(rf5c68_device::rf5c68_mem_r), FUNC(rf5c68_device::rf5c68_mem_w));  // WAVE RAM
	map(0xfffc0000, 0xffffffff).rom().region("user", 0x200000);  // SYSTEM ROM
}

void towns_state::marty_mem(address_map &map)
{
	map(0x00000000, 0x000bffff).ram();
	map(0x000c0000, 0x000c7fff).rw(FUNC(towns_state::towns_gfx_r), FUNC(towns_state::towns_gfx_w));
	map(0x000c8000, 0x000cafff).rw(FUNC(towns_state::towns_spriteram_low_r), FUNC(towns_state::towns_spriteram_low_w));
	map(0x000cb000, 0x000cbfff).bankr("bank_cb000_r").bankw("bank_cb000_w");
	map(0x000cc000, 0x000cff7f).ram();
	map(0x000cff80, 0x000cffff).rw(FUNC(towns_state::towns_video_cff80_mem_r), FUNC(towns_state::towns_video_cff80_mem_w));
	map(0x000d0000, 0x000d7fff).ram();
	map(0x000d8000, 0x000d9fff).rw(FUNC(towns_state::towns_cmos_low_r), FUNC(towns_state::towns_cmos_low_w)).share("nvram16"); // CMOS? RAM
	map(0x000da000, 0x000effff).ram(); //READWRITE(SMH_BANK(11),SMH_BANK(11))
	map(0x000f0000, 0x000f7fff).ram(); //READWRITE(SMH_BANK(12),SMH_BANK(12))
	map(0x000f8000, 0x000fffff).bankr("bank_f8000_r").bankw("bank_f8000_w");
	map(0x00600000, 0x0067ffff).rom().region("user", 0x000000);  // OS
	map(0x00680000, 0x0087ffff).rom().region("user", 0x280000);  // EX ROM
	map(0x00a00000, 0x00a7ffff).rw(FUNC(towns_state::towns_gfx_high_r), FUNC(towns_state::towns_gfx_high_w)).mirror(0x180000); // VRAM
	map(0x00b00000, 0x00b7ffff).rw(FUNC(towns_state::towns_gfx_packed_r), FUNC(towns_state::towns_gfx_packed_w)).mirror(0x80000); // VRAM
	map(0x00c00000, 0x00c1ffff).rw(FUNC(towns_state::towns_spriteram_r), FUNC(towns_state::towns_spriteram_w)); // Sprite RAM
	map(0x00d00000, 0x00dfffff).rw(m_icmemcard, FUNC(fmt_icmem_device::mem_read), FUNC(fmt_icmem_device::mem_write));
	map(0x00e80000, 0x00efffff).rom().region("user", 0x100000);  // DIC ROM
	map(0x00f00000, 0x00f7ffff).rom().region("user", 0x180000);  // FONT
	map(0x00f80000, 0x00f80fff).rw("pcm", FUNC(rf5c68_device::rf5c68_mem_r), FUNC(rf5c68_device::rf5c68_mem_w));  // WAVE RAM
	map(0x00fc0000, 0x00ffffff).rom().region("user", 0x200000);  // SYSTEM ROM
}

void towns_state::ux_mem(address_map &map)
{
	map(0x00000000, 0x000bffff).ram();
	map(0x000c0000, 0x000c7fff).rw(FUNC(towns_state::towns_gfx_r), FUNC(towns_state::towns_gfx_w));
	map(0x000c8000, 0x000cafff).rw(FUNC(towns_state::towns_spriteram_low_r), FUNC(towns_state::towns_spriteram_low_w));
	map(0x000cb000, 0x000cbfff).bankr("bank_cb000_r").bankw("bank_cb000_w");
	map(0x000cc000, 0x000cff7f).ram();
	map(0x000cff80, 0x000cffff).rw(FUNC(towns_state::towns_video_cff80_mem_r), FUNC(towns_state::towns_video_cff80_mem_w));
	map(0x000d0000, 0x000d7fff).ram();
	map(0x000d8000, 0x000d9fff).rw(FUNC(towns_state::towns_cmos_low_r), FUNC(towns_state::towns_cmos_low_w)).share("nvram16"); // CMOS? RAM
	map(0x000da000, 0x000effff).ram(); //READWRITE(SMH_BANK(11),SMH_BANK(11))
	map(0x000f0000, 0x000f7fff).ram(); //READWRITE(SMH_BANK(12),SMH_BANK(12))
	map(0x000f8000, 0x000fffff).bankr("bank_f8000_r").bankw("bank_f8000_w");
//  map(0x00680000, 0x0087ffff).rom().region("user",0x280000);  // EX ROM
	map(0x00a00000, 0x00a7ffff).rw(FUNC(towns_state::towns_gfx_high_r), FUNC(towns_state::towns_gfx_high_w)).mirror(0x180000); // VRAM
	map(0x00b00000, 0x00b7ffff).rw(FUNC(towns_state::towns_gfx_packed_r), FUNC(towns_state::towns_gfx_packed_w)).mirror(0x80000); // VRAM
	map(0x00c00000, 0x00c1ffff).rw(FUNC(towns_state::towns_spriteram_r), FUNC(towns_state::towns_spriteram_w)); // Sprite RAM
	map(0x00d00000, 0x00dfffff).rw(m_icmemcard, FUNC(fmt_icmem_device::mem_read), FUNC(fmt_icmem_device::mem_write));
	map(0x00e00000, 0x00e7ffff).rom().region("user", 0x000000);  // OS
	map(0x00e80000, 0x00efffff).rom().region("user", 0x100000);  // DIC ROM
	map(0x00f00000, 0x00f7ffff).rom().region("user", 0x180000);  // FONT
	map(0x00f80000, 0x00f80fff).rw("pcm", FUNC(rf5c68_device::rf5c68_mem_r), FUNC(rf5c68_device::rf5c68_mem_w));  // WAVE RAM
	map(0x00fc0000, 0x00ffffff).rom().region("user", 0x200000);  // SYSTEM ROM
}

void towns_state::towns_io(address_map &map)
{
	// I/O ports derived from FM Towns/Bochs, these are specific to the FM Towns
	// System ports
	map.unmap_value_high();
	map(0x0000, 0x0003).rw(m_pic_master, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask32(0x00ff00ff);
	map(0x0010, 0x0013).rw(m_pic_slave, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask32(0x00ff00ff);
	map(0x0020, 0x0033).rw(FUNC(towns_state::towns_system_r), FUNC(towns_state::towns_system_w));
	map(0x0040, 0x0047).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask32(0x00ff00ff);
	map(0x0050, 0x0057).rw("pit2", FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask32(0x00ff00ff);
	map(0x0060, 0x0060).rw(FUNC(towns_state::towns_port60_r), FUNC(towns_state::towns_port60_w));
	map(0x0068, 0x006b).rw(FUNC(towns_state::towns_intervaltimer2_r), FUNC(towns_state::towns_intervaltimer2_w));
	map(0x006c, 0x006c).rw(FUNC(towns_state::towns_sys6c_r), FUNC(towns_state::towns_sys6c_w));
	// 0x0070/0x0080 - CMOS RTC
	map(0x0070, 0x0070).rw(FUNC(towns_state::towns_rtc_r), FUNC(towns_state::towns_rtc_w));
	map(0x0080, 0x0080).w(FUNC(towns_state::towns_rtc_select_w));
	// DMA controllers (uPD71071)
	map(0x00a0, 0x00af).rw(FUNC(towns_state::towns_dma_r<0>), FUNC(towns_state::towns_dma_w<0>));
	map(0x00b0, 0x00bf).rw(FUNC(towns_state::towns_dma_r<1>), FUNC(towns_state::towns_dma_w<1>));
	// Floppy controller
	map(0x0200, 0x020f).rw(FUNC(towns_state::towns_floppy_r), FUNC(towns_state::towns_floppy_w));
	// CRTC / Video
	map(0x0400, 0x0400).r(FUNC(towns_state::towns_video_unknown_r));  // R/O (0x400)
	map(0x0404, 0x0404).rw(FUNC(towns_state::towns_video_404_r), FUNC(towns_state::towns_video_404_w));  // R/W (0x404)
	map(0x0440, 0x045f).rw(FUNC(towns_state::towns_video_440_r), FUNC(towns_state::towns_video_440_w));
	// System port
	map(0x0480, 0x0480).rw(FUNC(towns_state::towns_sys480_r), FUNC(towns_state::towns_sys480_w));  // R/W (0x480)
	// IC Memory Card
	map(0x048a, 0x048a).r(m_icmemcard, FUNC(fmt_icmem_device::status_r));
	map(0x0490, 0x0491).rw(m_icmemcard, FUNC(fmt_icmem_device::bank_r), FUNC(fmt_icmem_device::bank_w));
	// CD-ROM
	map(0x04c0, 0x04cf).rw(FUNC(towns_state::towns_cdrom_r), FUNC(towns_state::towns_cdrom_w)).umask32(0x00ff00ff);
	// Joystick / Mouse ports
	map(0x04d0, 0x04d3).r(FUNC(towns_state::towns_padport_r));
	map(0x04d6, 0x04d6).w(FUNC(towns_state::towns_pad_mask_w));
	// Sound (YM3438 [FM], RF5c68 [PCM])
	map(0x04d8, 0x04df).rw("fm", FUNC(ym3438_device::read), FUNC(ym3438_device::write)).umask32(0x00ff00ff);
	map(0x04e0, 0x04e3).rw(FUNC(towns_state::towns_volume_r), FUNC(towns_state::towns_volume_w));  // R/W  -- volume ports
	map(0x04e4, 0x04e7).r(FUNC(towns_state::unksnd_r));
	map(0x04e8, 0x04ef).rw(FUNC(towns_state::towns_sound_ctrl_r), FUNC(towns_state::towns_sound_ctrl_w));
	map(0x04f0, 0x04fb).w("pcm", FUNC(rf5c68_device::rf5c68_w));
	// CRTC / Video
	map(0x05c8, 0x05cb).rw(FUNC(towns_state::towns_video_5c8_r), FUNC(towns_state::towns_video_5c8_w));
	// System ports
	map(0x05e8, 0x05ef).rw(FUNC(towns_state::towns_sys5e8_r), FUNC(towns_state::towns_sys5e8_w)).umask32(0x00ff00ff);
	// Keyboard (8042 MCU)
	map(0x0600, 0x0607).rw(FUNC(towns_state::towns_keyboard_r), FUNC(towns_state::towns_keyboard_w)).umask32(0x00ff00ff);
	// RS-232C interface
	map(0x0a00, 0x0a0b).rw(FUNC(towns_state::towns_serial_r), FUNC(towns_state::towns_serial_w)).umask32(0x00ff00ff);
	// CMOS
	map(0x3000, 0x4fff).rw(FUNC(towns_state::towns_cmos_r), FUNC(towns_state::towns_cmos_w)).umask32(0x00ff00ff);
	// Something (MS-DOS wants this 0x41ff to be 1)
	//map(0x41fc,0x41ff).r(FUNC(towns_state::towns_41ff_r)).umask32(0xff000000);
	// CRTC / Video (again)
	map(0xfd90, 0xfda3).rw(FUNC(towns_state::towns_video_fd90_r), FUNC(towns_state::towns_video_fd90_w));
	map(0xff80, 0xffff).rw(FUNC(towns_state::towns_video_cff80_r), FUNC(towns_state::towns_video_cff80_w));
}

void towns_state::towns_1g_io(address_map &map)
{
	// For the first generation FM Towns with a SCSI card slot
	towns_io(map);
	map(0x0c30, 0x0c37).rw(m_scsi_slot, FUNC(fmt_scsi_slot_device::read), FUNC(fmt_scsi_slot_device::write)).umask32(0x00ff00ff);
}

void towns_state::towns2_io(address_map &map)
{
	// For FM Towns II models with integrated SCSI controller
	towns_io(map);
	map(0x0c30, 0x0c37).rw(m_scsi, FUNC(fmscsi_device::fmscsi_r), FUNC(fmscsi_device::fmscsi_w)).umask32(0x00ff00ff);
}

void towns_state::towns16_io(address_map &map)
{  // for the 386SX based systems
	// System ports
	map.unmap_value_high();
	map(0x0000, 0x0003).rw(m_pic_master, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x0010, 0x0013).rw(m_pic_slave, FUNC(pic8259_device::read), FUNC(pic8259_device::write)).umask16(0x00ff);
	map(0x0020, 0x0033).rw(FUNC(towns_state::towns_system_r), FUNC(towns_state::towns_system_w));
	map(0x0040, 0x0047).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	map(0x0050, 0x0057).rw("pit2", FUNC(pit8253_device::read), FUNC(pit8253_device::write)).umask16(0x00ff);
	map(0x0060, 0x0060).rw(FUNC(towns_state::towns_port60_r), FUNC(towns_state::towns_port60_w));
	map(0x0068, 0x006b).rw(FUNC(towns_state::towns_intervaltimer2_r), FUNC(towns_state::towns_intervaltimer2_w));
	map(0x006c, 0x006c).rw(FUNC(towns_state::towns_sys6c_r), FUNC(towns_state::towns_sys6c_w));
	// 0x0070/0x0080 - CMOS RTC
	map(0x0070, 0x0070).rw(FUNC(towns_state::towns_rtc_r), FUNC(towns_state::towns_rtc_w));
	map(0x0080, 0x0080).w(FUNC(towns_state::towns_rtc_select_w));
	// DMA controllers (uPD71071)
	map(0x00a0, 0x00af).rw(FUNC(towns_state::towns_dma_r<0>), FUNC(towns_state::towns_dma_w<0>));
	map(0x00b0, 0x00bf).rw(FUNC(towns_state::towns_dma_r<1>), FUNC(towns_state::towns_dma_w<1>));
	// Floppy controller
	map(0x0200, 0x020f).rw(FUNC(towns_state::towns_floppy_r), FUNC(towns_state::towns_floppy_w));
	// CRTC / Video
	map(0x0400, 0x0400).r(FUNC(towns_state::towns_video_unknown_r));  // R/O (0x400)
	map(0x0404, 0x0407).rw(FUNC(towns_state::towns_video_404_r), FUNC(towns_state::towns_video_404_w));  // R/W (0x404)
	map(0x0440, 0x045f).rw(FUNC(towns_state::towns_video_440_r), FUNC(towns_state::towns_video_440_w));
	// System port
	map(0x0480, 0x0480).rw(FUNC(towns_state::towns_sys480_r), FUNC(towns_state::towns_sys480_w));  // R/W (0x480)
	// IC Memory Card
	map(0x048a, 0x048a).r(m_icmemcard, FUNC(fmt_icmem_device::status_r));
	map(0x0490, 0x0491).rw(m_icmemcard, FUNC(fmt_icmem_device::bank_r), FUNC(fmt_icmem_device::bank_w));
	// CD-ROM
	map(0x04c0, 0x04cf).rw(FUNC(towns_state::towns_cdrom_r), FUNC(towns_state::towns_cdrom_w)).umask16(0x00ff);
	// Joystick / Mouse ports
	map(0x04d0, 0x04d3).r(FUNC(towns_state::towns_padport_r));
	map(0x04d6, 0x04d6).w(FUNC(towns_state::towns_pad_mask_w));
	// Sound (YM3438 [FM], RF5c68 [PCM])
	map(0x04d8, 0x04df).rw("fm", FUNC(ym3438_device::read), FUNC(ym3438_device::write)).umask16(0x00ff);
	map(0x04e0, 0x04e3).rw(FUNC(towns_state::towns_volume_r), FUNC(towns_state::towns_volume_w));  // R/W  -- volume ports
	map(0x04e4, 0x04e7).r(FUNC(towns_state::unksnd_r));
	map(0x04e8, 0x04ef).rw(FUNC(towns_state::towns_sound_ctrl_r), FUNC(towns_state::towns_sound_ctrl_w));
	map(0x04f0, 0x04fb).w("pcm", FUNC(rf5c68_device::rf5c68_w));
	// CRTC / Video
	map(0x05c8, 0x05cb).rw(FUNC(towns_state::towns_video_5c8_r), FUNC(towns_state::towns_video_5c8_w));
	// System ports
	map(0x05e8, 0x05ef).rw(FUNC(towns_state::towns_sys5e8_r), FUNC(towns_state::towns_sys5e8_w)).umask16(0x00ff);
	// Keyboard (8042 MCU)
	map(0x0600, 0x0607).rw(FUNC(towns_state::towns_keyboard_r), FUNC(towns_state::towns_keyboard_w)).umask16(0x00ff);
	// RS-232C interface
	map(0x0a00, 0x0a0b).rw(FUNC(towns_state::towns_serial_r), FUNC(towns_state::towns_serial_w)).umask16(0x00ff);
	// CMOS
	map(0x3000, 0x4fff).rw(FUNC(towns_state::towns_cmos_r), FUNC(towns_state::towns_cmos_w)).umask16(0x00ff);
	// Something (MS-DOS wants this 0x41ff to be 1)
	//map(0x41fc,0x41ff).r(FUNC(towns_state::towns_41ff_r)).umask32(0xff000000);
	// CRTC / Video (again)
	map(0xfd90, 0xfda3).rw(FUNC(towns_state::towns_video_fd90_r), FUNC(towns_state::towns_video_fd90_w));
	map(0xff80, 0xffff).rw(FUNC(towns_state::towns_video_cff80_r), FUNC(towns_state::towns_video_cff80_w));
}

void towns_state::townsux_io(address_map &map)
{
	// For FM Towns II UX
	towns16_io(map);
	map(0x0c30, 0x0c37).rw(m_scsi, FUNC(fmscsi_device::fmscsi_r), FUNC(fmscsi_device::fmscsi_w)).umask16(0x00ff);
}

void towns_state::pcm_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0xffff).ram();
}

/* Input ports */
static INPUT_PORTS_START( towns )
	// Keyboard
	PORT_START( "key1" )  // scancodes 0x00-0x1f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(27)
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1 ! \xE3\x81\xAC") PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2 \x22 \xE3\x81\xB5") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR(34)
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3 # \xE3\x81\x82 \xE3\x81\x81") PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4 $ \xE3\x81\x86 \xE3\x81\x85") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5 % \xE3\x81\x88 \xE3\x81\x87") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6 & \xE3\x81\x8A \xE3\x81\x89") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7 ´ \xE3\x82\x84 \xE3\x82\x83") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR(39)
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8 ( \xE3\x82\x86 \xE3\x82\x85") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9 ) \xE3\x82\x88 \xE3\x82\x87") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0 \xE3\x82\x8F \xE3\x82\x92") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("- = \xE3\x81\xBB") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('=')
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("^ \xE2\x80\xBE \xE3\x81\xB8") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^')
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xEF\xBF\xA5 | -") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(165) PORT_CHAR('|')
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q \xE3\x81\x9F") PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W \xE3\x81\xA6") PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E \xE3\x81\x84") PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R \xE3\x81\x99") PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T \xE3\x81\x8B") PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y \xE3\x82\x93") PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U \xE3\x81\xAA") PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I \xE3\x81\xAB") PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O \xE3\x82\x89") PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P \xE3\x81\x9B") PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("@ ` \xE2\x80\x9D") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@') PORT_CHAR('`')
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("[ { \xE3\x82\x9C \xE3\x80\x8C") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A \xE3\x81\xA1") PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S \xE3\x81\xA8") PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')

	PORT_START( "key2" )  // scancodes 0x20-0x3f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D \xE3\x81\x97") PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F \xE3\x81\xAF") PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G \xE3\x81\x8D") PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H \xE3\x81\x8F") PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J \xE3\x81\xBE") PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K \xE3\x81\xAE") PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L \xE3\x82\x8A") PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("; + \xE3\x82\x8C") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(": * \xE3\x81\x91") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("] } \xE3\x82\x80 \xE3\x80\x8D") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z \xE3\x81\xA4 \xE3\x81\xA3") PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X \xE3\x81\x95") PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C \xE3\x81\x9D") PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V \xE3\x81\xB2") PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B \xE3\x81\x93") PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N \xE3\x81\xBF") PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M \xE3\x82\x82") PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(", < \xE3\x81\xAD \xE3\x80\x81") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(". > \xE3\x82\x8B \xE3\x80\x82") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/ ? \xE3\x82\x81 \xE3\x83\xBB") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\x22 _ \xE3\x82\x8D") PORT_CHAR('"') PORT_CHAR('_')
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Space") PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey *") PORT_CODE(KEYCODE_ASTERISK)
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey /") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey +") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey -") PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 7") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 8") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 9") PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey =")
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 4") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 5") PORT_CODE(KEYCODE_5_PAD)

	PORT_START("key3")  // scancodes 0x40-0x5f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 6") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 1") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 2") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 3") PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey Enter") PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 0") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey .") PORT_CODE(KEYCODE_DEL_PAD)
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xE6\x8C\xBF\xE5\x85\xA5 (Insert) / DUP") PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 000") PORT_CODE(KEYCODE_000_PAD)
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xE5\x89\x8A\xE9\x99\xA4 (Delete) / EL") PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("HOME") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CAP") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xE3\x81\xB2\xE3\x82\x89\xE3\x81\x8C\xE3\x81\xAA (Hiragana) / \xE3\x83\xAD\xE3\x83\xBC\xE3\x83\x9E\xE5\xAD\x97 (Romaji)") PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xE7\x84\xA1\xE5\xA4\x89\xE6\x8F\x9B (Non-conversion)")
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xE5\xA4\x89\xE6\x8F\x9B (Conversion)")
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xE3\x81\x8B\xE3\x81\xAA\xE6\xBC\xA2\xE5\xAD\x97 (Kana Kanji)") PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xE3\x82\xAB\xE3\x82\xBF\xE3\x82\xAB\xE3\x83\x8A (Katakana)") PORT_CODE(KEYCODE_RWIN)
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF12") PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ALT") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF1") PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF2") PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF3") PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))

	PORT_START("key4")
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF4") PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF5") PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF6") PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF7") PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF8") PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF9") PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF10") PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF11") PORT_CODE(KEYCODE_F11) PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xE6\xBC\xA2\xE5\xAD\x97\xE8\xBE\x9E\xE6\x9B\xB8 (Kanji Dictionary)")
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xE5\x8D\x98\xE8\xAA\x9E\xE6\x8A\xB9\xE6\xB6\x88 (Word Deletion)")
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xE5\x8D\x98\xE8\xAA\x9E\xE7\x99\xBB\xE9\x8C\xB2 (Word Registration)")
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xE5\x89\x8D\xE8\xA1\x8C (Previous)")
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xE6\xAC\xA1\xE8\xA1\x8C (Next)")
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xE5\x8D\x8A\xE8\xA7\x92\xEF\xBC\x8F\xE5\x85\xA8\xE8\xA7\x92 (Half-width / Full-width)")
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xE5\x8F\x96\xE6\xB6\x88 (Cancel)")
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xE5\xAE\x9F\xE8\xA1\x8C (Execute)")
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF13")
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF14")
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF15")
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF16")
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF17")
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF18")
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF19")
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF20")
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("BREAK")
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("COPY")
INPUT_PORTS_END

static INPUT_PORTS_START( marty )
	PORT_INCLUDE(towns)
	// Consoles don't have keyboards...
	PORT_MODIFY("key1")
	PORT_BIT(0xffffffff,IP_ACTIVE_LOW,IPT_UNUSED)
	PORT_MODIFY("key2")
	PORT_BIT(0xffffffff,IP_ACTIVE_LOW,IPT_UNUSED)
	PORT_MODIFY("key3")
	PORT_BIT(0xffffffff,IP_ACTIVE_LOW,IPT_UNUSED)
	PORT_MODIFY("key4")
	PORT_BIT(0xffffffff,IP_ACTIVE_LOW,IPT_UNUSED)
INPUT_PORTS_END

void towns_state::driver_start()
{
	m_towns_vram = std::make_unique<uint32_t[]>(0x20000);
	m_towns_gfxvram = std::make_unique<uint8_t[]>(0x80000);
	m_towns_txtvram = std::make_unique<uint8_t[]>(0x20000);
	memset(m_towns_txtvram.get(), 0, sizeof(uint8_t)*0x20000);
	//towns_sprram = std::make_unique<uint8_t[]>(0x20000);
	m_towns_serial_rom = std::make_unique<uint8_t[]>(256/8);
	init_serial_rom();
	m_towns_kb_timer = timer_alloc(FUNC(towns_state::poll_keyboard), this);
	m_towns_wait_timer = timer_alloc(FUNC(towns_state::wait_end), this);
	m_towns_freerun_counter = timer_alloc(FUNC(towns_state::freerun_inc), this);
	m_towns_intervaltimer2 = timer_alloc(FUNC(towns_state::intervaltimer2_timeout), this);
	m_towns_status_timer = timer_alloc(FUNC(towns_state::towns_cd_status_ready), this);
	m_towns_cdda_timer = timer_alloc(FUNC(towns_state::towns_delay_cdda), this);
	m_towns_seek_timer = timer_alloc(FUNC(towns_state::towns_delay_seek), this);

	m_video = towns_video_controller();
	m_towns_cd = towns_cdrom_controller();
	m_towns_cd.status = 0x01;  // CDROM controller ready
	m_towns_cd.buffer_ptr = -1;
	m_towns_cd.read_timer = timer_alloc(FUNC(towns_state::towns_cdrom_read_byte), this);

	save_item(NAME(m_ftimer));
	save_item(NAME(m_freerun_timer));
	save_item(NAME(m_intervaltimer2_period));
	save_item(NAME(m_intervaltimer2_irqmask));
	save_item(NAME(m_intervaltimer2_timeout_flag));
	save_item(NAME(m_intervaltimer2_timeout_flag2));
	save_item(NAME(m_nmi_mask));
	save_item(NAME(m_compat_mode));
	save_item(NAME(m_towns_system_port));
	save_item(NAME(m_towns_ankcg_enable));
	save_item(NAME(m_towns_mainmem_enable));
	save_item(NAME(m_towns_ram_enable));
	save_pointer(NAME(m_towns_vram), 0x20000);
	save_pointer(NAME(m_towns_gfxvram), 0x80000);
	save_pointer(NAME(m_towns_txtvram), 0x20000);;
	save_item(NAME(m_towns_selected_drive));
	save_item(NAME(m_towns_fdc_irq6mask));
	save_pointer(NAME(m_towns_serial_rom), 256/8);
	save_item(NAME(m_towns_srom_position));
	save_item(NAME(m_towns_srom_clk));
	save_item(NAME(m_towns_srom_reset));
	save_item(NAME(m_towns_rtc_select));
	save_item(NAME(m_towns_rtc_data));
	save_item(NAME(m_towns_timer_mask));
	save_item(NAME(m_towns_kb_status));
	save_item(NAME(m_towns_kb_irq1_enable));
	save_item(NAME(m_towns_kb_output));  // key output
	save_item(NAME(m_towns_kb_extend));  // extended key output
	save_item(NAME(m_towns_fm_irq_flag));
	save_item(NAME(m_towns_pcm_irq_flag));
	save_item(NAME(m_towns_pcm_channel_flag));
	save_item(NAME(m_towns_pcm_channel_mask));
	save_item(NAME(m_towns_pad_mask));
	save_item(NAME(m_towns_volume));  // volume ports
	save_item(NAME(m_towns_volume_select));
	save_item(NAME(m_towns_scsi_control));
	save_item(NAME(m_towns_scsi_status));
	save_item(NAME(m_towns_spkrdata));
	save_item(NAME(m_pit_out0));
	save_item(NAME(m_pit_out1));
	save_item(NAME(m_pit_out2));
	save_item(NAME(m_serial_irq_source));

	save_item(NAME(m_kb_prev));
	save_item(NAME(m_prev_pad_mask));
	save_item(NAME(m_prev_x));
	save_item(NAME(m_prev_y));
	save_item(NAME(m_rtc_d));
	save_item(NAME(m_rtc_busy));
	save_item(NAME(m_vram_mask));
	save_item(NAME(m_vram_mask_addr));

	save_item(STRUCT_MEMBER(m_towns_cd, command));
	save_item(STRUCT_MEMBER(m_towns_cd, status));
	save_item(STRUCT_MEMBER(m_towns_cd, cmd_status));
	save_item(STRUCT_MEMBER(m_towns_cd, cmd_status_ptr));
	save_item(STRUCT_MEMBER(m_towns_cd, extra_status));
	save_item(STRUCT_MEMBER(m_towns_cd, parameter));
	save_item(STRUCT_MEMBER(m_towns_cd, mpu_irq_enable));
	save_item(STRUCT_MEMBER(m_towns_cd, dma_irq_enable));
	save_item(STRUCT_MEMBER(m_towns_cd, buffer));
	save_item(STRUCT_MEMBER(m_towns_cd, buffer_ptr));
	save_item(STRUCT_MEMBER(m_towns_cd, lba_current));
	save_item(STRUCT_MEMBER(m_towns_cd, lba_last));
	save_item(STRUCT_MEMBER(m_towns_cd, cdda_current));
	save_item(STRUCT_MEMBER(m_towns_cd, cdda_length));
	save_item(STRUCT_MEMBER(m_towns_cd, software_tx));

	save_item(STRUCT_MEMBER(m_video, towns_vram_wplane));
	save_item(STRUCT_MEMBER(m_video, towns_vram_rplane));
	save_item(STRUCT_MEMBER(m_video, towns_vram_page_sel));
	save_item(STRUCT_MEMBER(m_video, towns_palette_select));
	save_item(STRUCT_MEMBER(m_video, towns_palette_r));
	save_item(STRUCT_MEMBER(m_video, towns_palette_g));
	save_item(STRUCT_MEMBER(m_video, towns_palette_b));
	save_item(STRUCT_MEMBER(m_video, towns_degipal));
	save_item(STRUCT_MEMBER(m_video, towns_dpmd_flag));
	save_item(STRUCT_MEMBER(m_video, towns_crtc_mix));
	save_item(STRUCT_MEMBER(m_video, towns_crtc_sel));
	save_item(STRUCT_MEMBER(m_video, towns_crtc_reg));
	save_item(STRUCT_MEMBER(m_video, towns_video_sel));
	save_item(STRUCT_MEMBER(m_video, towns_video_reg));
	save_item(STRUCT_MEMBER(m_video, towns_sprite_sel));
	save_item(STRUCT_MEMBER(m_video, towns_sprite_reg));
	save_item(STRUCT_MEMBER(m_video, towns_sprite_flag));
	save_item(STRUCT_MEMBER(m_video, towns_sprite_page));
	save_item(STRUCT_MEMBER(m_video, towns_tvram_enable));
	save_item(STRUCT_MEMBER(m_video, towns_kanji_offset));
	save_item(STRUCT_MEMBER(m_video, towns_kanji_code_h));
	save_item(STRUCT_MEMBER(m_video, towns_kanji_code_l));
	save_item(STRUCT_MEMBER(m_video, towns_display_plane));
	save_item(STRUCT_MEMBER(m_video, towns_display_page_sel));
	save_item(STRUCT_MEMBER(m_video, towns_vblank_flag));
	save_item(STRUCT_MEMBER(m_video, towns_layer_ctrl));
	save_item(NAME(m_video.towns_crtc_layerscr[0].min_x));
	save_item(NAME(m_video.towns_crtc_layerscr[0].max_x));
	save_item(NAME(m_video.towns_crtc_layerscr[0].min_y));
	save_item(NAME(m_video.towns_crtc_layerscr[0].max_y));
	save_item(NAME(m_video.towns_crtc_layerscr[1].min_x));
	save_item(NAME(m_video.towns_crtc_layerscr[1].max_x));
	save_item(NAME(m_video.towns_crtc_layerscr[1].min_y));
	save_item(NAME(m_video.towns_crtc_layerscr[1].max_y));

	save_pointer(m_video.towns_crtc_reg,"CRTC registers",32);
	save_pointer(m_video.towns_video_reg,"Video registers",2);

	if (m_ram->size() > 0x100000)
		m_maincpu->space(AS_PROGRAM).install_ram(0x100000,m_ram->size()-1,m_ram->pointer() + 0x100000);
}

void marty_state::driver_start()
{
	towns_state::driver_start();
	if(m_towns_machine_id == 0x0101) // default if no serial ROM present
		m_towns_machine_id = 0x034a;
}

void towns_state::machine_start()
{
	if (m_flop[0]->get_device())
		m_flop[0]->get_device()->set_rpm(360);
	if (m_flop[1]->get_device())
		m_flop[1]->get_device()->set_rpm(360);

	// uninitialized PCM RAM filled with 0xff (fmtmarty chasehq relies on that)
	address_space &space = subdevice<rf5c68_device>("pcm")->space(0);
	for (int i = 0; i < 0x10000; i++)
		space.write_byte(i, 0xff);

	m_timer0 = 0;
	m_timer1 = 0;
	m_serial_irq_enable = 0;
}

void towns_state::machine_reset()
{
	m_ftimer = 0x00;
	m_freerun_timer = 0x00;
	m_nmi_mask = 0x00;
	m_compat_mode = 0x00;
	m_towns_ankcg_enable = 0x00;
	m_towns_mainmem_enable = 0x00;
	m_towns_system_port = 0x00;
	m_towns_ram_enable = 0x00;
	towns_update_video_banks();
	m_towns_kb_status = 0x18;
	m_towns_kb_irq1_enable = 0;
	m_towns_pad_mask = 0x7f;
	m_towns_volume_select = 0;
	m_intervaltimer2_period = 0;
	m_intervaltimer2_timeout_flag = 0;
	m_intervaltimer2_timeout_flag2 = 0;
	m_intervaltimer2_irqmask = 1;  // masked
	m_towns_kb_timer->adjust(attotime::zero,0,attotime::from_msec(10));
	m_towns_freerun_counter->adjust(attotime::zero,0,attotime::from_usec(1));
	m_serial_irq_source = 0;
	m_rtc_d = 0;
	m_rtc_busy = false;
	m_vram_mask_addr = 0;
	m_towns_pcm_channel_flag = 0;
	m_towns_pcm_channel_mask = 0xff;
	m_towns_pcm_irq_flag = 0;
	m_towns_fm_irq_flag = 0;
}

uint8_t towns_state::get_slave_ack(offs_t offset)
{
	if (offset==7) { // IRQ = 7
		return m_pic_slave->acknowledge();
	}
	return 0x00;
}

void towns_state::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_FMTOWNS_FORMAT);
}

static void towns_floppies(device_slot_interface &device)
{
	device.option_add("35hd", FLOPPY_35_HD);
}

static const gfx_layout fnt_chars_16x16 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	16*16
};

static const gfx_layout text_chars =
{
	8,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1), },
	{ STEP16(0,1) },
	8*16
};

static GFXDECODE_START( gfx_towns )
	GFXDECODE_ENTRY( "user",   0x180000 + 0x3d800, text_chars,  0, 16 )
	GFXDECODE_ENTRY( "user",   0x180000, fnt_chars_16x16,  0, 16 )
GFXDECODE_END

void towns_state::towns_base(machine_config &config)
{
	/* basic machine hardware */
	I386(config, m_maincpu, 16000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &towns_state::towns_mem);
	m_maincpu->set_addrmap(AS_IO, &towns_state::towns_1g_io);
	m_maincpu->set_vblank_int("screen", FUNC(towns_state::towns_vsync_irq));
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));
	//MCFG_MACHINE_RESET_OVERRIDE(towns_state,towns)

	/* pad ports */
	MSX_GENERAL_PURPOSE_PORT(config, m_pad_ports[0], msx_general_purpose_port_devices, "townspad");
	MSX_GENERAL_PURPOSE_PORT(config, m_pad_ports[1], msx_general_purpose_port_devices, "mouse");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_size(768,512);
	m_screen->set_visarea(0, 768-1, 0, 512-1);
	m_screen->set_screen_update(FUNC(towns_state::screen_update));

	GFXDECODE(config, "gfxdecode", m_palette16[0], gfx_towns);
	PALETTE(config, m_palette).set_entries(256);
	PALETTE(config, m_palette16[0]).set_entries(16);
	PALETTE(config, m_palette16[1]).set_entries(16);

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ym3438_device &fm(YM3438(config, "fm", 16000000 / 2)); // actual clock speed unknown
	fm.irq_handler().set(FUNC(towns_state::towns_fm_irq));
	fm.add_route(0, "lspeaker", 1.00);
	fm.add_route(1, "rspeaker", 1.00);

/*
    // Later model uses YMF276 for FM
    ymf276_device &fm(YMF276(config, "fm", 16000000 / 2)); // actual clock speed unknown
    fm.irq_handler().set(FUNC(towns_state::towns_fm_irq));
    fm.add_route(0, "lspeaker", 1.00);
    fm.add_route(1, "rspeaker", 1.00);
*/

	rf5c68_device &pcm(RF5C68(config, "pcm", 16000000 / 2));  // actual clock speed unknown
	pcm.set_end_callback(FUNC(towns_state::towns_pcm_irq));
	pcm.set_addrmap(0, &towns_state::pcm_mem);
	pcm.add_route(0, "lspeaker", 1.00);
	pcm.add_route(1, "rspeaker", 1.00);

	CDDA(config, m_cdda);
	m_cdda->add_route(0, "lspeaker", 0.30);
	m_cdda->add_route(1, "rspeaker", 0.30);
	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "lspeaker", 0.50);
	m_speaker->add_route(ALL_OUTPUTS, "rspeaker", 0.50);

	PIT8253(config, m_pit, 0);
	m_pit->set_clk<0>(307200);
	m_pit->out_handler<0>().set(FUNC(towns_state::towns_pit_out0_changed));
	m_pit->set_clk<1>(307200);
	m_pit->out_handler<1>().set(FUNC(towns_state::towns_pit_out1_changed));
	m_pit->set_clk<2>(307200);
	m_pit->out_handler<2>().set(FUNC(towns_state::pit_out2_changed));

	pit8253_device &pit2(PIT8253(config, "pit2", 0));
	pit2.set_clk<0>(307200); // reserved
	pit2.set_clk<1>(1228800); // RS-232
	pit2.out_handler<1>().set(FUNC(towns_state::pit2_out1_changed));
	pit2.set_clk<2>(307200); // reserved

	PIC8259(config, m_pic_master, 0);
	m_pic_master->out_int_callback().set_inputline(m_maincpu, 0);
	m_pic_master->in_sp_callback().set_constant(1);
	m_pic_master->read_slave_ack_callback().set(FUNC(towns_state::get_slave_ack));

	PIC8259(config, m_pic_slave, 0);
	m_pic_slave->out_int_callback().set(m_pic_master, FUNC(pic8259_device::ir7_w));
	m_pic_slave->in_sp_callback().set_constant(0);

	MB8877(config, m_fdc, 8'000'000 / 4);  // clock unknown
	m_fdc->intrq_wr_callback().set(FUNC(towns_state::mb8877a_irq_w));
	m_fdc->drq_wr_callback().set(FUNC(towns_state::mb8877a_drq_w));
	FLOPPY_CONNECTOR(config, m_flop[0], towns_floppies, "35hd", towns_state::floppy_formats);
	FLOPPY_CONNECTOR(config, m_flop[1], towns_floppies, "35hd", towns_state::floppy_formats);
	SOFTWARE_LIST(config, "fd_list_orig").set_original("fmtowns_flop_orig");
	SOFTWARE_LIST(config, "fd_list_cracked").set_original("fmtowns_flop_cracked");
	SOFTWARE_LIST(config, "fd_list_misc").set_original("fmtowns_flop_misc");

	CDROM(config, m_cdrom, 0).set_interface("fmt_cdrom");
	m_cdda->set_cdrom_tag(m_cdrom);
	SOFTWARE_LIST(config, "cd_list").set_original("fmtowns_cd");

	UPD71071(config, m_dma[0], 0);
	m_dma[0]->set_cpu_tag("maincpu");
	m_dma[0]->set_clock(4000000);
	m_dma[0]->dma_read_callback<0>().set(FUNC(towns_state::towns_fdc_dma_r));
	m_dma[0]->dma_read_callback<3>().set(FUNC(towns_state::towns_state::towns_cdrom_dma_r));
	m_dma[0]->dma_write_callback<0>().set(FUNC(towns_state::towns_fdc_dma_w));
	UPD71071(config, m_dma[1], 0);
	m_dma[1]->set_cpu_tag("maincpu");
	m_dma[1]->set_clock(4000000);
	m_dma[1]->dma_read_callback<0>().set(FUNC(towns_state::towns_fdc_dma_r));
	m_dma[1]->dma_read_callback<3>().set(FUNC(towns_state::towns_state::towns_cdrom_dma_r));
	m_dma[1]->dma_write_callback<0>().set(FUNC(towns_state::towns_fdc_dma_w));

	//MCFG_VIDEO_START_OVERRIDE(towns_state,towns)

	I8251(config, m_i8251, 0);
	m_i8251->rxrdy_handler().set(FUNC(towns_state::towns_rxrdy_irq));
	m_i8251->txrdy_handler().set(FUNC(towns_state::towns_txrdy_irq));
	m_i8251->syndet_handler().set(FUNC(towns_state::towns_syndet_irq));
	m_i8251->dtr_handler().set("rs232c", FUNC(rs232_port_device::write_dtr));
	m_i8251->rts_handler().set("rs232c", FUNC(rs232_port_device::write_rts));
	m_i8251->txd_handler().set("rs232c", FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232c(RS232_PORT(config, "rs232c", default_rs232_devices, nullptr));
	rs232c.rxd_handler().set(m_i8251, FUNC(i8251_device::write_rxd));
	rs232c.dsr_handler().set(m_i8251, FUNC(i8251_device::write_dsr));
	rs232c.cts_handler().set(m_i8251, FUNC(i8251_device::write_cts));

	FMT_ICMEM(config, m_icmemcard, 0);

	/* First-generation models: 1 MB onboard, 3 SIMM slots with 1 or 2 MB each, except slot 1 (limited to 1 MB).
	   Model 2 comes with a 1 MB SIMM preinstalled on slot 1, Model 1 doesn't. */
	RAM(config, m_ram).set_default_size("2M").set_extra_options("1M,3M,4M,5M,6M");

	MSM58321(config, m_rtc, 32768_Hz_XTAL);
	m_rtc->d0_handler().set(FUNC(towns_state::rtc_d0_w));
	m_rtc->d1_handler().set(FUNC(towns_state::rtc_d1_w));
	m_rtc->d2_handler().set(FUNC(towns_state::rtc_d2_w));
	m_rtc->d3_handler().set(FUNC(towns_state::rtc_d3_w));
	m_rtc->busy_handler().set(FUNC(towns_state::rtc_busy_w));
	m_rtc->set_year0(2000);
	m_rtc->set_default_24h(true);
}

void towns_state::towns(machine_config &config)
{
	towns_base(config);
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	FMT_SCSI_SLOT(config, m_scsi_slot, fmt_scsi_default_devices, nullptr);
	m_scsi_slot->irq_handler().set(FUNC(towns_state::towns_scsi_irq));
	m_scsi_slot->drq_handler().set(FUNC(towns_state::towns_scsi_drq));

	m_dma[0]->dma_read_callback<1>().set(m_scsi_slot, FUNC(fmt_scsi_slot_device::data_read));
	m_dma[0]->dma_write_callback<1>().set(m_scsi_slot, FUNC(fmt_scsi_slot_device::data_write));
	m_dma[1]->dma_read_callback<1>().set(m_scsi_slot, FUNC(fmt_scsi_slot_device::data_read));
	m_dma[1]->dma_write_callback<1>().set(m_scsi_slot, FUNC(fmt_scsi_slot_device::data_write));
}

void towns16_state::townsux(machine_config &config)
{
	towns_base(config);

	I386SX(config.replace(), m_maincpu, 16000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &towns16_state::ux_mem);
	m_maincpu->set_addrmap(AS_IO, &towns16_state::townsux_io);
	m_maincpu->set_vblank_int("screen", FUNC(towns_state::towns_vsync_irq));
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	scsi_port_device &scsi(SCSI_PORT(config, "scsi", 0));
	scsi.set_slot_device(1, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_0));
	scsi.set_slot_device(2, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_1));
	scsi.set_slot_device(3, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_2));
	scsi.set_slot_device(4, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_3));
	scsi.set_slot_device(5, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_4));

	FMSCSI(config, m_scsi, 0);
	m_scsi->set_scsi_port("scsi");
	m_scsi->irq_handler().set(FUNC(towns16_state::towns_scsi_irq));
	m_scsi->drq_handler().set(FUNC(towns16_state::towns_scsi_drq));

	m_dma[0]->dma_read_callback<1>().set(m_scsi, FUNC(fmscsi_device::fmscsi_data_r));
	m_dma[0]->dma_write_callback<1>().set(m_scsi, FUNC(fmscsi_device::fmscsi_data_w));
	m_dma[1]->dma_read_callback<1>().set(m_scsi, FUNC(fmscsi_device::fmscsi_data_r));
	m_dma[1]->dma_write_callback<1>().set(m_scsi, FUNC(fmscsi_device::fmscsi_data_w));

	// 2 MB onboard, one SIMM slot with 2-8 MB
	m_ram->set_default_size("2M").set_extra_options("4M,6M,10M");

	NVRAM(config, "nvram16", nvram_device::DEFAULT_ALL_0);
}

void towns_state::townssj(machine_config &config)
{
	towns_base(config);

	I486(config.replace(), m_maincpu, 66000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &towns_state::towns_mem);
	m_maincpu->set_addrmap(AS_IO, &towns_state::towns2_io);
	m_maincpu->set_vblank_int("screen", FUNC(towns_state::towns_vsync_irq));
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	scsi_port_device &scsi(SCSI_PORT(config, "scsi", 0));
	scsi.set_slot_device(1, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_0));
	scsi.set_slot_device(2, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_1));
	scsi.set_slot_device(3, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_2));
	scsi.set_slot_device(4, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_3));
	scsi.set_slot_device(5, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_4));

	FMSCSI(config, m_scsi, 0);
	m_scsi->set_scsi_port("scsi");
	m_scsi->irq_handler().set(FUNC(towns_state::towns_scsi_irq));
	m_scsi->drq_handler().set(FUNC(towns_state::towns_scsi_drq));

	m_dma[0]->dma_read_callback<1>().set(m_scsi, FUNC(fmscsi_device::fmscsi_data_r));
	m_dma[0]->dma_write_callback<1>().set(m_scsi, FUNC(fmscsi_device::fmscsi_data_w));
	m_dma[1]->dma_read_callback<1>().set(m_scsi, FUNC(fmscsi_device::fmscsi_data_r));
	m_dma[1]->dma_write_callback<1>().set(m_scsi, FUNC(fmscsi_device::fmscsi_data_w));

	// 4 MB (SJ2/SJ2A) or 8 MB (SJ26/SJ53) onboard, 2 SIMM slots with 4-32 MB each
	m_ram->set_default_size("8M").set_extra_options("4M,12M,16M,20M,24M,28M,32M,36M,40M,44M,48M,52M,56M,68M,72M");

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
}

void towns_state::townshr(machine_config &config)
{
	townssj(config);
	I486(config.replace(), m_maincpu, 20000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &towns_state::towns_mem);
	m_maincpu->set_addrmap(AS_IO, &towns_state::towns2_io);
	m_maincpu->set_vblank_int("screen", FUNC(towns_state::towns_vsync_irq));
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	// 4 MB onboard, 3 SIMM slots with 2-8 MB each
	m_ram->set_default_size("4M").set_extra_options("6M,8M,10M,12M,14M,16M,18M,20M,22M,24M,28M");
}

void towns_state::townsmx(machine_config &config)
{
	townssj(config);

	// 4 MB onboard, 3 SIMM slots with 2-32 MB each, MX170W/MX340W models come with a 4 MB SIMM preinstalled
	m_ram->set_default_size("8M").set_extra_options("4M,6M,8M,10M,12M,14M,16M,18M,20M,22M,24M,26M,28M,30M,32M,36M,38M,40M,42M,44M,46M,48M,52M,53M,54M,56M,60M,68M,70M,72M,76M,84M,100M");
}

void towns_state::townsftv(machine_config &config)
{
	townssj(config);
	I486(config.replace(), m_maincpu, 33000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &towns_state::towns_mem);
	m_maincpu->set_addrmap(AS_IO, &towns_state::towns2_io);
	m_maincpu->set_vblank_int("screen", FUNC(towns_state::towns_vsync_irq));
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	// 4 MB onboard, 2 SIMM slots with 2-32 MB each, one of them with a 2 MB SIMM preinstalled
	m_ram->set_default_size("6M").set_extra_options("4M,8M,10M,12M,14M,16M,20M,22M,24M,28M,36M,38M,40M,44M,52M,68M");
}

void marty_state::marty(machine_config &config)
{
	towns_base(config);

	I386SX(config.replace(), m_maincpu, 16000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &marty_state::marty_mem);
	m_maincpu->set_addrmap(AS_IO, &marty_state::towns16_io);
	m_maincpu->set_vblank_int("screen", FUNC(towns_state::towns_vsync_irq));
	m_maincpu->set_irq_acknowledge_callback("pic8259_master", FUNC(pic8259_device::inta_cb));

	m_pad_ports[0]->set_default_option("martypad");
	m_pad_ports[1]->set_default_option(nullptr);

	FLOPPY_CONNECTOR(config.replace(), m_flop[1], towns_floppies, nullptr, towns_state::floppy_formats);

	// 2 MB onboard, expandable to 4 MB with a Marty-only expansion card
	m_ram->set_default_size("2M").set_extra_options("4M");

	NVRAM(config, "nvram16", nvram_device::DEFAULT_ALL_0);
}

/* ROM definitions */

/* These ROMs were dumped from an FM Towns Model 2. Model 1 is assumed to use the same ROMs, since they were
   released at the same time, and the only differences are the amount of RAM and floppy drives.

   The ROM is physically contained in three 4 Mbit chips: two MB834200-20 (DIP40) and one MB834200-25 (QFP44) */
ROM_START( fmtowns )
	ROM_REGION32_LE( 0x280000, "user", 0)
	ROM_LOAD("fmt_dos.rom",  0x000000, 0x080000, CRC(112872ee) SHA1(57fd146478226f7f215caf63154c763a6d52165e) )
	ROM_LOAD("fmt_dic.rom",  0x100000, 0x080000, CRC(b314c659) SHA1(3959c4c6be540252cabea06847bcd408f1911cfb) )
	ROM_LOAD("fmt_fnt.rom",  0x180000, 0x040000, CRC(955c6b75) SHA1(fa5f7a18060afa35678dcbdc3589a1455aba26dc) )
	ROM_LOAD("fmt_sys.rom",  0x200000, 0x040000, CRC(53319e23) SHA1(15d9cc705f3534fe97a2386e4d4848a1602cc534) )
ROM_END

/* System ROM has a date of 91/07/09 and matches the UX set, but the dictionary ROM is completely different. It could be from an FM Towns II CX.
   Font ROM appears to be corrupt, though. */
ROM_START( fmtownsv03 )
	ROM_REGION32_LE( 0x280000, "user", 0)
	ROM_LOAD("fmt_dos_a.rom",  0x000000, 0x080000, CRC(22270e9f) SHA1(a7e97b25ff72b14121146137db8b45d6c66af2ae) )
	ROM_LOAD("fmt_f20_a.rom",  0x080000, 0x080000, CRC(75660aac) SHA1(6a521e1d2a632c26e53b83d2cc4b0edecfc1e68c) )
	ROM_LOAD("fmt_dic_a.rom",  0x100000, 0x080000, CRC(74b1d152) SHA1(f63602a1bd67c2ad63122bfb4ffdaf483510f6a8) )
	ROM_LOAD("fmt_fnt_a.rom",  0x180000, 0x040000, CRC(0108a090) SHA1(1b5dd9d342a96b8e64070a22c3a158ca419894e1) BAD_DUMP )
	ROM_LOAD("fmt_sys_a.rom",  0x200000, 0x040000, CRC(92f3fa67) SHA1(be21404098b23465d24c4201a81c96ac01aff7ab) )
ROM_END

/* 16MHz 80386SX, 2MB RAM expandable up to 10MB (due to the limited 24-bit address space of the CPU), dumped from a UX10 */
ROM_START( fmtownsux )
	ROM_REGION16_LE( 0x480000, "user", 0)
	ROM_LOAD("fmt_dos_a.rom",  0x000000, 0x080000, CRC(22270e9f) SHA1(a7e97b25ff72b14121146137db8b45d6c66af2ae) )
	// no F20 ROM
	ROM_LOAD("fmt_dic.rom",  0x100000, 0x080000, CRC(82d1daa2) SHA1(7564020dba71deee27184824b84dbbbb7c72aa4e) )
	ROM_LOAD("fmt_fnt.rom",  0x180000, 0x040000, CRC(dd6fd544) SHA1(a216482ea3162f348fcf77fea78e0b2e4288091a) )
	ROM_LOAD("fmt_sys_a.rom",  0x200000, 0x040000, CRC(92f3fa67) SHA1(be21404098b23465d24c4201a81c96ac01aff7ab) )
	ROM_REGION( 0x20, "serial", 0)
	ROM_LOAD("mytownsux.rom",  0x00, 0x20, CRC(5cc7e6bc) SHA1(e245f8086df57ce6e48853f0e13525f738e5c4d8) )
ROM_END

/* 20MHz 80486SX, 4MB RAM expandable up to 28MB, dumped from an HR20 */
ROM_START( fmtownshr )
	ROM_REGION32_LE( 0x280000, "user", 0)
	ROM_LOAD("fmt_dos.rom",  0x000000, 0x080000, CRC(112872ee) SHA1(57fd146478226f7f215caf63154c763a6d52165e) )
	// F20 ROM space appears to be all 0xFF on an HR, so it is assumed to be not present
//  ROM_LOAD("fmt_f20.rom",  0x080000, 0x080000, CRC(9f55a20c) SHA1(1920711cb66340bb741a760de187de2f76040b8c) )
	ROM_LOAD("fmt_dic.rom",  0x100000, 0x080000, CRC(82d1daa2) SHA1(7564020dba71deee27184824b84dbbbb7c72aa4e) )
	ROM_LOAD("fmt_fnt.rom",  0x180000, 0x040000, CRC(dd6fd544) SHA1(a216482ea3162f348fcf77fea78e0b2e4288091a) )
	ROM_LOAD("fmthr_sys.rom",0x200000, 0x040000, CRC(8aeff982) SHA1(a4ebf2e247a8e15a5f1ff003b657bbe3a67203d8) )
	ROM_REGION( 0x20, "serial", 0)
	ROM_LOAD("mytownshr.rom",  0x00, 0x20, CRC(c52f0e89) SHA1(634d3965606b18a99507f0a520553005661c41ff) )
ROM_END

/* 66MHz 80486DX2, 8MB RAM expandable up to 72MB, dumped from an SJ26 */
ROM_START( fmtownssj )
	ROM_REGION32_LE( 0x280000, "user", 0)
	// Assumed for now, only the serial ROM has been dumped successfully so far
	ROM_LOAD("fmt_dos.rom",  0x000000, 0x080000, CRC(112872ee) SHA1(57fd146478226f7f215caf63154c763a6d52165e) BAD_DUMP )
	ROM_LOAD("fmt_f20.rom",  0x080000, 0x080000, CRC(9f55a20c) SHA1(1920711cb66340bb741a760de187de2f76040b8c) BAD_DUMP )
	ROM_LOAD("fmt_dic.rom",  0x100000, 0x080000, CRC(82d1daa2) SHA1(7564020dba71deee27184824b84dbbbb7c72aa4e) BAD_DUMP )
	ROM_LOAD("fmt_fnt.rom",  0x180000, 0x040000, CRC(dd6fd544) SHA1(a216482ea3162f348fcf77fea78e0b2e4288091a) BAD_DUMP )
	ROM_LOAD("fmt_sys.rom",  0x200000, 0x040000, CRC(afe4ebcf) SHA1(4cd51de4fca9bd7a3d91d09ad636fa6b47a41df5) BAD_DUMP )
	ROM_REGION( 0x20, "serial", 0)
	ROM_LOAD("mytownssj.rom",  0x00, 0x20, CRC(d0ed8936) SHA1(bf9eeef25e9a1dc4a9ce1c70f4155ac973cae3f9) )
ROM_END

/* 66MHz 80486DX2, dumped from an FM-Towns II MX */
ROM_START( fmtownsmx )
	ROM_REGION32_LE( 0x280000, "user", 0)
	ROM_LOAD("fmtownsiimxbios.m79",  0x000000, 0x080000, CRC(f3fc636e) SHA1(a35a11ab56b1f11e3f69d70b61648ab83699a2df) )
	ROM_CONTINUE(0x180000,0x40000)
	ROM_CONTINUE(0x200000,0x40000)
	ROM_CONTINUE(0x080000,0x80000)
	ROM_CONTINUE(0x100000,0x80000)
	ROM_REGION( 0x20, "serial", 0)
	ROM_LOAD("mytownsmx.rom",  0x00, 0x20, CRC(16e78766) SHA1(38e8810bee9ee6b54c3999d27f499b89e4a4c33f) )
ROM_END

/* 33MHz 80486SX, 6MB RAM expandable up to 68MB, dumped from an FM Towns II Fresh TV */
ROM_START( fmtownsftv )
	ROM_REGION32_LE( 0x280000, "user", 0)
	ROM_LOAD("fmt_dos.rom",  0x000000, 0x080000, CRC(112872ee) SHA1(57fd146478226f7f215caf63154c763a6d52165e) )
	ROM_LOAD("fmt_f20.rom",  0x080000, 0x080000, CRC(9f55a20c) SHA1(1920711cb66340bb741a760de187de2f76040b8c) )
	ROM_LOAD("fmt_dic.rom",  0x100000, 0x080000, CRC(82d1daa2) SHA1(7564020dba71deee27184824b84dbbbb7c72aa4e) )
	ROM_LOAD("fmt_fnt.rom",  0x180000, 0x040000, CRC(dd6fd544) SHA1(a216482ea3162f348fcf77fea78e0b2e4288091a) )
	ROM_LOAD("fmt_sys.rom",  0x200000, 0x040000, CRC(afe4ebcf) SHA1(4cd51de4fca9bd7a3d91d09ad636fa6b47a41df5) )
	ROM_REGION( 0x20, "serial", 0)
	ROM_LOAD("mytownsftv.rom",  0x00, 0x20, CRC(a961aae7) SHA1(bba02edafdfaa6fb1b8122f623259a87a555c307) )
ROM_END

ROM_START( fmtmarty )
	ROM_REGION16_LE( 0x480000, "user", 0)
	ROM_LOAD("mrom.m36",  0x000000, 0x080000, CRC(9c0c060c) SHA1(5721c5f9657c570638352fa9acac57fa8d0b94bd) )
	ROM_CONTINUE(0x280000,0x180000)
	ROM_LOAD("mrom.m37",  0x400000, 0x080000, CRC(fb66bb56) SHA1(e273b5fa618373bdf7536495cd53c8aac1cce9a5) )
	ROM_CONTINUE(0x80000,0x100000)
	ROM_CONTINUE(0x180000,0x40000)
	ROM_CONTINUE(0x200000,0x40000)
ROM_END

ROM_START( fmtmarty2 )
	ROM_REGION16_LE( 0x480000, "user", 0)
	ROM_LOAD("fmt_dos.rom",  0x000000, 0x080000, CRC(2bc2af96) SHA1(99cd51c5677288ad8ef711b4ac25d981fd586884) )
	ROM_LOAD("fmt_dic.rom",  0x100000, 0x080000, CRC(82d1daa2) SHA1(7564020dba71deee27184824b84dbbbb7c72aa4e) )
	ROM_LOAD("fmt_fnt.rom",  0x180000, 0x040000, CRC(dd6fd544) SHA1(a216482ea3162f348fcf77fea78e0b2e4288091a) )
	ROM_LOAD("fmt_sys.rom",  0x200000, 0x040000, CRC(937311f6) SHA1(7b1c97fa778986134104a6964c8fe13e3654f52e) )
	ROM_LOAD("mar_ex0.rom",  0x280000, 0x080000, CRC(f67540f6) SHA1(1611fc4683dab72e56a5d0ee0f757e5878900b31) )
	ROM_LOAD("mar_ex1.rom",  0x300000, 0x080000, CRC(99938a4b) SHA1(167d8ae47312cdaa30b8597144f60f54ce9f74d3) )
	ROM_LOAD("mar_ex2.rom",  0x380000, 0x080000, CRC(c6783422) SHA1(b2ed2ba42b8132d139480484fe116ba7774e1604) )
	ROM_LOAD("mar_ex3.rom",  0x400000, 0x080000, CRC(4aa43e16) SHA1(19b669aa6488bdaf8569e89b1b1067e51246a768) )
	ROM_REGION( 0x20, "serial", 0)
	ROM_LOAD("mytownsm2.rom",  0x00, 0x20, CRC(44f2f076) SHA1(e4d3be54e66931c947993ded9ddbae716ad51ca5) )
ROM_END

ROM_START( carmarty )
	ROM_REGION16_LE( 0x480000, "user", 0)
	ROM_LOAD("fmt_dos.rom",  0x000000, 0x080000, CRC(2bc2af96) SHA1(99cd51c5677288ad8ef711b4ac25d981fd586884) )
	ROM_LOAD("fmt_dic.rom",  0x100000, 0x080000, CRC(82d1daa2) SHA1(7564020dba71deee27184824b84dbbbb7c72aa4e) )
	ROM_LOAD("fmt_fnt.rom",  0x180000, 0x040000, CRC(dd6fd544) SHA1(a216482ea3162f348fcf77fea78e0b2e4288091a) )
	ROM_LOAD("cmar_sys.rom",  0x200000, 0x040000, CRC(e1ff7ce1) SHA1(e6c359177e4e9fb5bbb7989c6bbf6e95c091fd88) )
	ROM_LOAD("cmar_ex0.rom",  0x280000, 0x080000, CRC(e248bfbd) SHA1(0ce89952a7901dd4d256939a6bc8597f87e51ae7) )
	ROM_LOAD("cmar_ex1.rom",  0x300000, 0x080000, CRC(ab2e94f0) SHA1(4b3378c772302622f8e1139ed0caa7da1ab3c780) )
	ROM_LOAD("cmar_ex2.rom",  0x380000, 0x080000, CRC(ce150ec7) SHA1(1cd8c39f3b940e03f9fe999ebcf7fd693f843d04) )
	ROM_LOAD("cmar_ex3.rom",  0x400000, 0x080000, CRC(582fc7fc) SHA1(a77d8014e41e9ff0f321e156c0fe1a45a0c5e58e) )
	ROM_REGION( 0x20, "serial", 0)
	ROM_LOAD("mytownscm.rom",  0x00, 0x20, CRC(bc58eba6) SHA1(483087d823c3952cc29bd827e5ef36d12c57ad49) )
ROM_END

/* Driver */

/*    YEAR  NAME        PARENT    COMPAT  MACHINE   INPUT  CLASS          INIT        COMPANY    FULLNAME                                   FLAGS */
COMP( 1989, fmtowns,    0,        0,      towns,    towns, towns_state,   empty_init, "Fujitsu", "FM-Towns (Model 1 / 2)",                  MACHINE_NOT_WORKING)
COMP( 1991, fmtownsv03, fmtowns,  0,      towns,    towns, towns_state,   empty_init, "Fujitsu", "FM-Towns (unknown, V03 L01 00 91/07/09)", MACHINE_NOT_WORKING)
COMP( 1991, fmtownsux,  fmtowns,  0,      townsux,  towns, towns16_state, empty_init, "Fujitsu", "FM-Towns II UX",                          MACHINE_NOT_WORKING)
COMP( 1992, fmtownshr,  fmtowns,  0,      townshr,  towns, towns_state,   empty_init, "Fujitsu", "FM-Towns II HR",                          MACHINE_NOT_WORKING)
COMP( 1993, fmtownsmx,  fmtowns,  0,      townsmx,  towns, towns_state,   empty_init, "Fujitsu", "FM-Towns II MX",                          MACHINE_NOT_WORKING)
COMP( 1994, fmtownsftv, fmtowns,  0,      townsftv, towns, towns_state,   empty_init, "Fujitsu", "FM-Towns II FreshTV",                     MACHINE_NOT_WORKING)
COMP( 19??, fmtownssj,  fmtowns,  0,      townssj,  towns, towns_state,   empty_init, "Fujitsu", "FM-Towns II SJ",                          MACHINE_NOT_WORKING)
CONS( 1993, fmtmarty,   0,        0,      marty,    marty, marty_state,   empty_init, "Fujitsu", "FM-Towns Marty",                          MACHINE_NOT_WORKING)
CONS( 1993, fmtmarty2,  fmtmarty, 0,      marty,    marty, marty_state,   empty_init, "Fujitsu", "FM-Towns Marty 2",                        MACHINE_NOT_WORKING)
CONS( 1994, carmarty,   fmtmarty, 0,      marty,    marty, marty_state,   empty_init, "Fujitsu", "FM-Towns Car Marty",                      MACHINE_NOT_WORKING)
