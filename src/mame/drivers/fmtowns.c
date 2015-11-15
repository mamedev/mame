// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*

    Fujitsu FM-Towns
    driver by Barry Rodewald

    Japanese computer system released in 1989.

    CPU:  various AMD x86 CPUs, originally 80386DX (80387 available as an add-on).
          later models use 80386SX, 80486 and Pentium CPUs
    Sound:  Yamaha YM3438
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

#include "includes/fmtowns.h"
#include "bus/scsi/scsi.h"
#include "bus/scsi/scsihd.h"

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




inline UINT8 towns_state::byte_to_bcd(UINT8 val)
{
	return ((val / 10) << 4) | (val % 10);
}

inline UINT8 towns_state::bcd_to_byte(UINT8 val)
{
	return (((val & 0xf0) >> 4) * 10) + (val & 0x0f);
}

inline UINT32 towns_state::msf_to_lbafm(UINT32 val)  // because the CDROM core doesn't provide this
{
	UINT8 m,s,f;
	f = bcd_to_byte(val & 0x0000ff);
	s = (bcd_to_byte((val & 0x00ff00) >> 8));
	m = (bcd_to_byte((val & 0xff0000) >> 16));
	return ((m * (60 * 75)) + (s * 75) + f);
}

void towns_state::init_serial_rom()
{
	// TODO: init serial ROM contents
	int x;
	static const UINT8 code[8] = { 0x04,0x65,0x54,0xA4,0x95,0x45,0x35,0x5F };
	UINT8* srom = NULL;

	if(m_serial)
		srom = m_serial->base();
	memset(m_towns_serial_rom,0,256/8);

	if(srom)
	{
		memcpy(m_towns_serial_rom,srom,32);
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

void towns_state::init_rtc()
{
	system_time systm;

	machine().base_datetime(systm);

	// seconds
	m_towns_rtc_reg[0] = systm.local_time.second % 10;
	m_towns_rtc_reg[1] = systm.local_time.second / 10;
	// minutes
	m_towns_rtc_reg[2] = systm.local_time.minute % 10;
	m_towns_rtc_reg[3] = systm.local_time.minute / 10;
	// hours
	m_towns_rtc_reg[4] = systm.local_time.hour % 10;
	m_towns_rtc_reg[5] = systm.local_time.hour / 10;
	// weekday
	m_towns_rtc_reg[6] = systm.local_time.weekday;
	// day
	m_towns_rtc_reg[7] = systm.local_time.mday % 10;
	m_towns_rtc_reg[8] = systm.local_time.mday / 10;
	// month
	m_towns_rtc_reg[9] = systm.local_time.month % 10;
	m_towns_rtc_reg[10] = systm.local_time.month / 10;
	// year
	m_towns_rtc_reg[11] = (systm.local_time.year - 2000) % 10;
	m_towns_rtc_reg[12] = (systm.local_time.year - 2000) / 10;
}

READ8_MEMBER(towns_state::towns_system_r)
{
	UINT8 ret = 0;

	switch(offset)
	{
		case 0x00:
			logerror("SYS: port 0x20 read\n");
			return 0x00;
		case 0x05:
			logerror("SYS: port 0x25 read\n");
			return 0x00;
/*      case 0x06:
            count = (m_towns_freerun_counter->time_elapsed() * ATTOSECONDS_TO_HZ(ATTOSECONDS_IN_USEC(1))).as_double();
            return count & 0xff;
        case 0x07:
            count = (m_towns_freerun_counter->time_elapsed() * ATTOSECONDS_TO_HZ(ATTOSECONDS_IN_USEC(1))).as_double();
            return (count >> 8) & 0xff;
*/      case 0x06:
			//logerror("SYS: (0x26) timer read\n");
			return m_freerun_timer;
		case 0x07:
			return m_freerun_timer >> 8;
		case 0x08:
			//logerror("SYS: (0x28) NMI mask read\n");
			return m_nmi_mask & 0x01;
		case 0x10:
			logerror("SYS: (0x30) Machine ID read\n");
			return (m_towns_machine_id >> 8) & 0xff;
		case 0x11:
			logerror("SYS: (0x31) Machine ID read\n");
			return m_towns_machine_id & 0xff;
		case 0x12:
			/* Bit 0 = data, bit 6 = CLK, bit 7 = RESET, bit 5 is always 1? */
			ret = (m_towns_serial_rom[m_towns_srom_position/8] & (1 << (m_towns_srom_position%8))) ? 1 : 0;
			ret |= m_towns_srom_clk;
			ret |= m_towns_srom_reset;
			//logerror("SYS: (0x32) Serial ROM read [0x%02x, pos=%i]\n",ret,towns_srom_position);
			return ret;
		default:
			//logerror("SYS: Unknown system port read (0x%02x)\n",offset+0x20);
			return 0x00;
	}
}

WRITE8_MEMBER(towns_state::towns_system_w)
{
	switch(offset)
	{
		case 0x00:  // bit 7 = NMI vector protect, bit 6 = power off, bit 0 = software reset, bit 3 = A20 line?
//          space.m_maincpu->set_input_line(INPUT_LINE_A20,(data & 0x08) ? CLEAR_LINE : ASSERT_LINE);
			logerror("SYS: port 0x20 write %02x\n",data);
			break;
		case 0x02:
			logerror("SYS: (0x22) power port write %02x\n",data);
			break;
		case 0x08:
			//logerror("SYS: (0x28) NMI mask write %02x\n",data);
			m_nmi_mask = data & 0x01;
			break;
		case 0x12:
			//logerror("SYS: (0x32) Serial ROM write %02x\n",data);
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
			logerror("SYS: Unknown system port write 0x%02x (0x%02x)\n",data,offset);
	}
}

WRITE8_MEMBER(towns_state::towns_intervaltimer2_w)
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

READ8_MEMBER(towns_state::towns_intervaltimer2_r)
{
	UINT8 ret = 0;

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

void towns_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch(id)
	{
	case TIMER_RTC:
		rtc_second();
		break;
	case TIMER_FREERUN:
		freerun_inc();
		break;
	case TIMER_INTERVAL2:
		intervaltimer2_timeout();
		break;
	case TIMER_KEYBOARD:
		poll_keyboard();
		break;
	case TIMER_MOUSE:
		mouse_timeout();
		break;
	case TIMER_WAIT:
		wait_end();
		break;
	case TIMER_CDSTATUS:
		towns_cd_status_ready();
		break;
	case TIMER_CDDA:
		towns_delay_cdda((cdrom_image_device*)ptr);
		break;
	}
}
void towns_state::freerun_inc()
{
	m_freerun_timer++;
}

void towns_state::intervaltimer2_timeout()
{
	m_intervaltimer2_timeout_flag = 1;
}

void towns_state::wait_end()
{
	m_maincpu->set_input_line(INPUT_LINE_HALT,CLEAR_LINE);
}

READ8_MEMBER(towns_state::towns_sys6c_r)
{
	logerror("SYS: (0x6c) Timer? read\n");
	return 0x00;
}

WRITE8_MEMBER(towns_state::towns_sys6c_w)
{
	// halts the CPU for 1 microsecond
	m_maincpu->set_input_line(INPUT_LINE_HALT,ASSERT_LINE);
	m_towns_wait_timer->adjust(attotime::from_usec(1),0,attotime::never);
}

READ8_MEMBER(towns_state::towns_dma1_r)
{
//  logerror("DMA#1: read register %i\n",offset);
	return m_dma_1->read(space, offset);
}

WRITE8_MEMBER(towns_state::towns_dma1_w)
{
//  logerror("DMA#1: wrote 0x%02x to register %i\n",data,offset);
	m_dma_1->write(space, offset, data);
}

READ8_MEMBER(towns_state::towns_dma2_r)
{
	logerror("DMA#2: read register %i\n",offset);
	return m_dma_2->read(space, offset);
}

WRITE8_MEMBER(towns_state::towns_dma2_w)
{
	logerror("DMA#2: wrote 0x%02x to register %i\n",data,offset);
	m_dma_2->write(space, offset, data);
}

/*
 *  Floppy Disc Controller (MB8877A)
 */

WRITE_LINE_MEMBER( towns_state::mb8877a_irq_w )
{
	if(m_towns_fdc_irq6mask == 0)
		state = 0;
	m_pic_master->ir6_w(state);  // IRQ6 = FDC
	if(IRQ_LOG) logerror("PIC: IRQ6 (FDC) set to %i\n",state);
}

WRITE_LINE_MEMBER( towns_state::mb8877a_drq_w )
{
	m_dma_1->dmarq(state, 0);
}

READ8_MEMBER(towns_state::towns_floppy_r)
{
	UINT8 ret;

	switch(offset)
	{
		case 0x00:
			return m_fdc->status_r(space, 0);
		case 0x02:
			return m_fdc->track_r(space, 0);
		case 0x04:
			return m_fdc->sector_r(space, 0);
		case 0x06:
			return m_fdc->data_r(space, 0);
		case 0x08:  // selected drive status?
			//logerror("FDC: read from offset 0x08\n");
			ret = 0x80;  // always set
			switch(m_towns_selected_drive)
			{
			case 1:
				ret |= 0x0c;
				if(m_flop0->get_device()->exists())
					ret |= 0x03;
				break;
			case 2:
				ret |= 0x0c;
				if(m_flop1->get_device()->exists())
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
				return m_flop0->get_device()->dskchg_r();
			if(m_towns_selected_drive == 2)
				return m_flop1->get_device()->dskchg_r();
			return 0x00;
		default:
			logerror("FDC: read from invalid or unimplemented register %02x\n",offset);
	}
	return 0xff;
}

WRITE8_MEMBER(towns_state::towns_floppy_w)
{
	floppy_image_device* sel[4] = { m_flop0->get_device(), m_flop1->get_device(), NULL, NULL };

	switch(offset)
	{
		case 0x00:
			// Commands 0xd0 and 0xfe (Write Track) are apparently ignored?
			if(data == 0xd0)
				return;
			if(data == 0xfe)
				return;
			m_fdc->cmd_w(space, 0,data);
			logerror("FDC: Command %02x\n",data);
			break;
		case 0x02:
			m_fdc->track_w(space, 0,data);
			logerror("FDC: Track %02x\n",data);
			break;
		case 0x04:
			m_fdc->sector_w(space, 0,data);
			logerror("FDC: Sector %02x\n",data);
			break;
		case 0x06:
			m_fdc->data_w(space, 0,data);
			logerror("FDC: Data %02x\n",data);
			break;
		case 0x08:
			// bit 5 - CLKSEL
			if(m_towns_selected_drive != 0)
			{
				if(sel[m_towns_selected_drive-1] != NULL)
				{
					sel[m_towns_selected_drive-1]->mon_w((~data & 0x10)>>4);
					sel[m_towns_selected_drive-1]->ss_w((data & 0x04)>>2);
				}
			}
			m_fdc->dden_w(BIT(~data, 1));

			m_towns_fdc_irq6mask = data & 0x01;
			//logerror("FDC: Config drive%i %02x\n",m_towns_selected_drive-1,data);

			break;
		case 0x0c:  // drive select
			switch(data & 0x0f)
			{
				case 0x00:
					m_towns_selected_drive = 0;  // No drive selected
					break;
				case 0x01:
					m_towns_selected_drive = 1;
					if(sel[0] != NULL)
						m_fdc->set_floppy(sel[0]);
					break;
				case 0x02:
					m_towns_selected_drive = 2;
					if(sel[1] != NULL)
						m_fdc->set_floppy(sel[1]);
					break;
				case 0x04:
					m_towns_selected_drive = 3;
					if(sel[2] != NULL)
						m_fdc->set_floppy(sel[2]);
					break;
				case 0x08:
					m_towns_selected_drive = 4;
					if(sel[3] != NULL)
						m_fdc->set_floppy(sel[3]);
					break;
			}
			//logerror("FDC: drive select %02x\n",data);
			break;
		default:
			logerror("FDC: write %02x to invalid or unimplemented register %02x\n",data,offset);
	}
}

READ16_MEMBER(towns_state::towns_fdc_dma_r)
{   UINT16 data = m_fdc->data_r(generic_space(), 0);
	return data;
}

WRITE16_MEMBER(towns_state::towns_fdc_dma_w)
{
	m_fdc->data_w(generic_space(), 0,data);
}

/*
 *  Port 0x600-0x607 - Keyboard controller (8042 MCU)
 *
 *  Sends two-byte code on each key press and release.
 *  First byte has the MSB set, and contains shift/ctrl/alt/kana flags
 *    Known bits:
 *      bit 7 = always 1
 *      bit 4 = key release
 *      bit 3 = ctrl
 *      bit 2 = shift
 *
 *  Second byte has the MSB reset, and contains the scancode of the key
 *  pressed or released.
 *      bit 7 = always 0
 *      bits 6-0 = key scancode
 */
void towns_state::kb_sendcode(UINT8 scancode, int release)
{
	switch(release)
	{
		case 0:  // key press
			m_towns_kb_output = 0x80;
			m_towns_kb_extend = scancode & 0x7f;
			if(m_key3->read() & 0x00080000)
				m_towns_kb_output |= 0x04;
			if(m_key3->read() & 0x00040000)
				m_towns_kb_output |= 0x08;
			if(m_key3->read() & 0x06400000)
				m_towns_kb_output |= 0x20;
			break;
		case 1:  // key release
			m_towns_kb_output = 0x90;
			m_towns_kb_extend = scancode & 0x7f;
			if(m_key3->read() & 0x00080000)
				m_towns_kb_output |= 0x04;
			if(m_key3->read() & 0x00040000)
				m_towns_kb_output |= 0x08;
			if(m_key3->read() & 0x06400000)
				m_towns_kb_output |= 0x20;
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

void towns_state::poll_keyboard()
{
	ioport_port* kb_ports[4] = { m_key1, m_key2, m_key3, m_key4 };
	int port,bit;
	UINT8 scan;
	UINT32 portval;

	scan = 0;
	for(port=0;port<4;port++)
	{
		portval = kb_ports[port]->read();
		for(bit=0;bit<32;bit++)
		{
			if(((portval & (1<<bit))) != ((m_kb_prev[port] & (1<<bit))))
			{  // bit changed
				if((portval & (1<<bit)) == 0)  // release
					kb_sendcode(scan,1);
				else
					kb_sendcode(scan,0);
			}
			scan++;
		}
		m_kb_prev[port] = portval;
	}
}

READ8_MEMBER(towns_state::towns_keyboard_r)
{
	UINT8 ret = 0x00;

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

WRITE8_MEMBER(towns_state::towns_keyboard_w)
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
UINT8 towns_state::speaker_get_spk()
{
	return m_towns_spkrdata & m_pit_out2;
}


void towns_state::speaker_set_spkrdata(UINT8 data)
{
	m_towns_spkrdata = data ? 1 : 0;
	m_speaker->level_w(speaker_get_spk());
}


READ8_MEMBER(towns_state::towns_port60_r)
{
	UINT8 val = 0x00;

	if (m_pit_out0)
		val |= 0x01;
	if (m_pit_out1)
		val |= 0x02;

	val |= (m_towns_timer_mask & 0x07) << 2;

	//logerror("PIT: port 0x60 read, returning 0x%02x\n",val);
	return val;
}

WRITE8_MEMBER(towns_state::towns_port60_w)
{
	//device_t* dev = m_pic_master;

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

READ8_MEMBER(towns_state::towns_sys5e8_r)
{
	switch(offset)
	{
		case 0x00:
			logerror("SYS: read RAM size port (%i)\n",m_ram->size());
			return m_ram->size()/1048576;
		case 0x02:
			logerror("SYS: read port 5ec\n");
			return m_compat_mode & 0x01;
	}
	return 0x00;
}

WRITE8_MEMBER(towns_state::towns_sys5e8_w)
{
	switch(offset)
	{
		case 0x00:
			logerror("SYS: wrote 0x%02x to port 5e8\n",data);
			break;
		case 0x02:
			logerror("SYS: wrote 0x%02x to port 5ec\n",data);
			m_compat_mode = data & 0x01;
			break;
	}
}

// Sound/LED control (I/O port 0x4e8-0x4ef)
// R/O  -- (0x4e9) FM IRQ flag (bit 0), PCM IRQ flag (bit 3)
// (0x4ea) PCM IRQ mask
// R/W  -- (0x4eb) PCM IRQ flag
// W/O  -- (0x4ec) LED control
READ8_MEMBER(towns_state::towns_sound_ctrl_r)
{
	UINT8 ret = 0;

	switch(offset)
	{
		case 0x01:
			if(m_towns_fm_irq_flag)
				ret |= 0x01;
			if(m_towns_pcm_irq_flag)
				ret |= 0x08;
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

WRITE8_MEMBER(towns_state::towns_sound_ctrl_w)
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
// Joysticks are multiplexed, with fire buttons available when bits 0 and 1 of port 0x4d6 are high. (bits 2 and 3 for second port?)
void towns_state::mouse_timeout()
{
	m_towns_mouse_output = MOUSE_START;  // reset mouse data
}

READ8_MEMBER(towns_state::towns_padport_r)
{
	UINT8 ret = 0x00;
	UINT32 porttype = m_ctrltype->read();
	UINT8 extra1;
	UINT8 extra2;
	UINT32 state;

	if(offset == 0)
	{
		if((porttype & 0x0f) == 0x01)
		{
			extra1 = m_joy1_ex->read();

			if(m_towns_pad_mask & 0x10)
				ret |= (m_joy1->read() & 0x3f) | 0x40;
			else
				ret |= (m_joy1->read() & 0x0f) | 0x30;

			if(extra1 & 0x01) // Run button = left+right
				ret &= ~0x0c;
			if(extra1 & 0x02) // Select button = up+down
				ret &= ~0x03;

			if((extra1 & 0x10) && (m_towns_pad_mask & 0x01))
				ret &= ~0x10;
			if((extra1 & 0x20) && (m_towns_pad_mask & 0x02))
				ret &= ~0x20;
		}
		if((porttype & 0x0f) == 0x04)  // 6-button joystick
		{
			extra1 = m_6b_joy1_ex->read();

			if(m_towns_pad_mask & 0x10)
				ret |= 0x7f;
			else
				ret |= (m_6b_joy1->read() & 0x0f) | 0x70;

			if(!(m_towns_pad_mask & 0x10))
			{
				if(extra1 & 0x01) // Run button = left+right
					ret &= ~0x0c;
				if(extra1 & 0x02) // Select button = up+down
					ret &= ~0x03;
				if((extra1 & 0x04) && (m_towns_pad_mask & 0x01))
					ret &= ~0x10;
				if((extra1 & 0x08) && (m_towns_pad_mask & 0x02))
					ret &= ~0x20;
			}
			if(m_towns_pad_mask & 0x10)
			{
				if(extra1 & 0x10)
					ret &= ~0x08;
				if(extra1 & 0x20)
					ret &= ~0x04;
				if(extra1 & 0x40)
					ret &= ~0x02;
				if(extra1 & 0x80)
					ret &= ~0x01;
			}
		}
		if((porttype & 0x0f) == 0x02)  // mouse
		{
			switch(m_towns_mouse_output)
			{
				case MOUSE_X_HIGH:
					ret |= ((m_towns_mouse_x & 0xf0) >> 4);
					break;
				case MOUSE_X_LOW:
					ret |= (m_towns_mouse_x & 0x0f);
					break;
				case MOUSE_Y_HIGH:
					ret |= ((m_towns_mouse_y & 0xf0) >> 4);
					break;
				case MOUSE_Y_LOW:
					ret |= (m_towns_mouse_y & 0x0f);
					break;
				case MOUSE_START:
				case MOUSE_SYNC:
				default:
					if(m_towns_mouse_output < MOUSE_Y_LOW)
						ret |= 0x0f;
			}

			// button states are always visible
			state = m_mouse1->read();
			if(!(state & 0x01))
				ret |= 0x10;
			if(!(state & 0x02))
				ret |= 0x20;
			if(m_towns_pad_mask & 0x10)
				ret |= 0x40;
		}

	}
	if(offset == 1)  // second joystick port
	{
		if((porttype & 0xf0) == 0x10)
		{
			extra2 = m_joy2_ex->read();

			if(m_towns_pad_mask & 0x20)
				ret |= ((m_joy2->read() & 0x3f)) | 0x40;
			else
				ret |= ((m_joy2->read() & 0x0f)) | 0x30;

			if(extra2 & 0x01)
				ret &= ~0x0c;
			if(extra2 & 0x02)
				ret &= ~0x03;

			if((extra2 & 0x10) && (m_towns_pad_mask & 0x04))
				ret &= ~0x10;
			if((extra2 & 0x20) && (m_towns_pad_mask & 0x08))
				ret &= ~0x20;
		}
		if((porttype & 0xf0) == 0x40)  // 6-button joystick
		{
			extra2 = m_6b_joy2_ex->read();

			if(m_towns_pad_mask & 0x20)
				ret |= 0x7f;
			else
				ret |= ((m_6b_joy2->read() & 0x0f)) | 0x70;

			if(!(m_towns_pad_mask & 0x10))
			{
				if(extra2 & 0x01)
					ret &= ~0x0c;
				if(extra2 & 0x02)
					ret &= ~0x03;
				if((extra2 & 0x10) && (m_towns_pad_mask & 0x04))
					ret &= ~0x10;
				if((extra2 & 0x20) && (m_towns_pad_mask & 0x08))
					ret &= ~0x20;
			}
			if(m_towns_pad_mask & 0x20)
			{
				if(extra2 & 0x10)
					ret &= ~0x08;
				if(extra2 & 0x20)
					ret &= ~0x04;
				if(extra2 & 0x40)
					ret &= ~0x02;
				if(extra2 & 0x80)
					ret &= ~0x01;
			}
		}
		if((porttype & 0xf0) == 0x20)  // mouse
		{
			switch(m_towns_mouse_output)
			{
				case MOUSE_X_HIGH:
					ret |= ((m_towns_mouse_x & 0xf0) >> 4);
					break;
				case MOUSE_X_LOW:
					ret |= (m_towns_mouse_x & 0x0f);
					break;
				case MOUSE_Y_HIGH:
					ret |= ((m_towns_mouse_y & 0xf0) >> 4);
					break;
				case MOUSE_Y_LOW:
					ret |= (m_towns_mouse_y & 0x0f);
					break;
				case MOUSE_START:
				case MOUSE_SYNC:
				default:
					if(m_towns_mouse_output < MOUSE_Y_LOW)
						ret |= 0x0f;
			}

			// button states are always visible
			state = m_mouse1->read();
			if(!(state & 0x01))
				ret |= 0x10;
			if(!(state & 0x02))
				ret |= 0x20;
			if(m_towns_pad_mask & 0x20)
				ret |= 0x40;
		}
	}

	return ret;
}

WRITE8_MEMBER(towns_state::towns_pad_mask_w)
{
	UINT8 current_x,current_y;
	UINT32 type = m_ctrltype->read();

	m_towns_pad_mask = (data & 0xff);
	if((type & 0x0f) == 0x02)  // mouse
	{
		if((m_towns_pad_mask & 0x10) != 0 && (m_prev_pad_mask & 0x10) == 0)
		{
			if(m_towns_mouse_output == MOUSE_START)
			{
				m_towns_mouse_output = MOUSE_X_HIGH;
				current_x = m_mouse2->read();
				current_y = m_mouse3->read();
				m_towns_mouse_x = m_prev_x - current_x;
				m_towns_mouse_y = m_prev_y - current_y;
				m_prev_x = current_x;
				m_prev_y = current_y;
			}
			else
				m_towns_mouse_output++;
			m_towns_mouse_timer->adjust(attotime::from_usec(600),0,attotime::zero);
		}
		if((m_towns_pad_mask & 0x10) == 0 && (m_prev_pad_mask & 0x10) != 0)
		{
			if(m_towns_mouse_output == MOUSE_START)
			{
				m_towns_mouse_output = MOUSE_SYNC;
				current_x = m_mouse2->read();
				current_y = m_mouse3->read();
				m_towns_mouse_x = m_prev_x - current_x;
				m_towns_mouse_y = m_prev_y - current_y;
				m_prev_x = current_x;
				m_prev_y = current_y;
			}
			else
				m_towns_mouse_output++;
			m_towns_mouse_timer->adjust(attotime::from_usec(600),0,attotime::zero);
		}
		m_prev_pad_mask = m_towns_pad_mask;
	}
	if((type & 0xf0) == 0x20)  // mouse
	{
		if((m_towns_pad_mask & 0x20) != 0 && (m_prev_pad_mask & 0x20) == 0)
		{
			if(m_towns_mouse_output == MOUSE_START)
			{
				m_towns_mouse_output = MOUSE_X_HIGH;
				current_x = m_mouse2->read();
				current_y = m_mouse3->read();
				m_towns_mouse_x = m_prev_x - current_x;
				m_towns_mouse_y = m_prev_y - current_y;
				m_prev_x = current_x;
				m_prev_y = current_y;
			}
			else
				m_towns_mouse_output++;
			m_towns_mouse_timer->adjust(attotime::from_usec(600),0,attotime::zero);
		}
		if((m_towns_pad_mask & 0x20) == 0 && (m_prev_pad_mask & 0x20) != 0)
		{
			if(m_towns_mouse_output == MOUSE_START)
			{
				m_towns_mouse_output = MOUSE_SYNC;
				current_x = m_mouse2->read();
				current_y = m_mouse3->read();
				m_towns_mouse_x = m_prev_x - current_x;
				m_towns_mouse_y = m_prev_y - current_y;
				m_prev_x = current_x;
				m_prev_y = current_y;
			}
			else
				m_towns_mouse_output++;
			m_towns_mouse_timer->adjust(attotime::from_usec(600),0,attotime::zero);
		}
		m_prev_pad_mask = m_towns_pad_mask;
	}
}

READ8_MEMBER( towns_state::towns_cmos_low_r )
{
	if(m_towns_mainmem_enable != 0)
		return m_messram->pointer()[offset + 0xd8000];

	if(m_nvram)
		return m_nvram[offset];
	else
		return m_nvram16[offset];
}

WRITE8_MEMBER( towns_state::towns_cmos_low_w )
{
	if(m_towns_mainmem_enable != 0)
		m_messram->pointer()[offset+0xd8000] = data;
	else
		if(m_nvram)
			m_nvram[offset] = data;
		else
			m_nvram16[offset] = data;
}

READ8_MEMBER( towns_state::towns_cmos_r )
{
	if(m_nvram)
		return m_nvram[offset];
	else
		return m_nvram16[offset];
}

WRITE8_MEMBER( towns_state::towns_cmos_w )
{
	if(m_nvram)
		m_nvram[offset] = data;
	else
		m_nvram16[offset] = data;
}

void towns_state::towns_update_video_banks(address_space& space)
{
	UINT8* ROM;

	if(m_towns_mainmem_enable != 0)  // first MB is RAM
	{
		ROM = m_user->base();

//      membank(1)->set_base(m_messram->pointer()+0xc0000);
//      membank(2)->set_base(m_messram->pointer()+0xc8000);
//      membank(3)->set_base(m_messram->pointer()+0xc9000);
//      membank(4)->set_base(m_messram->pointer()+0xca000);
//      membank(5)->set_base(m_messram->pointer()+0xca000);
//      membank(10)->set_base(m_messram->pointer()+0xca800);
		membank("bank6")->set_base(m_messram->pointer()+0xcb000);
		membank("bank7")->set_base(m_messram->pointer()+0xcb000);
		if(m_towns_system_port & 0x02)
			membank("bank11")->set_base(m_messram->pointer()+0xf8000);
		else
			membank("bank11")->set_base(ROM+0x238000);
		membank("bank12")->set_base(m_messram->pointer()+0xf8000);
		return;
	}
	else  // enable I/O ports and VRAM
	{
		ROM = m_user->base();

//      membank(1)->set_base(towns_gfxvram+(towns_vram_rplane*0x8000));
//      membank(2)->set_base(towns_txtvram);
//      membank(3)->set_base(m_messram->pointer()+0xc9000);
//      if(towns_ankcg_enable != 0)
//          membank(4)->set_base(ROM+0x180000+0x3d000);  // ANK CG 8x8
//      else
//          membank(4)->set_base(towns_txtvram+0x2000);
//      membank(5)->set_base(towns_txtvram+0x2000);
//      membank(10)->set_base(m_messram->pointer()+0xca800);
		if(m_towns_ankcg_enable != 0)
			membank("bank6")->set_base(ROM+0x180000+0x3d800);  // ANK CG 8x16
		else
			membank("bank6")->set_base(m_messram->pointer()+0xcb000);
		membank("bank7")->set_base(m_messram->pointer()+0xcb000);
		if(m_towns_system_port & 0x02)
			membank("bank11")->set_base(m_messram->pointer()+0xf8000);
		else
			membank("bank11")->set_base(ROM+0x238000);
		membank("bank12")->set_base(m_messram->pointer()+0xf8000);
		return;
	}
}

READ8_MEMBER( towns_state::towns_sys480_r )
{
	if(m_towns_system_port & 0x02)
		return 0x02;
	else
		return 0x00;
}

WRITE8_MEMBER( towns_state::towns_sys480_w )
{
	m_towns_system_port = data;
	m_towns_ram_enable = data & 0x02;
	towns_update_video_banks(space);
}

WRITE8_MEMBER( towns_state::towns_video_404_w )
{
	m_towns_mainmem_enable = data & 0x80;
	towns_update_video_banks(space);
}

READ8_MEMBER( towns_state::towns_video_404_r )
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

void towns_state::towns_cd_status_ready()
{
	m_towns_cd.status |= 0x02;  // status read request
	m_towns_cd.status |= 0x01;  // ready
	m_towns_cd.cmd_status_ptr = 0;
	towns_cdrom_set_irq(TOWNS_CD_IRQ_MPU,1);
}

void towns_state::towns_cd_set_status(UINT8 st0, UINT8 st1, UINT8 st2, UINT8 st3)
{
	m_towns_cd.cmd_status[0] = st0;
	m_towns_cd.cmd_status[1] = st1;
	m_towns_cd.cmd_status[2] = st2;
	m_towns_cd.cmd_status[3] = st3;
	// wait a bit
	m_towns_status_timer->adjust(attotime::from_msec(1),0,attotime::never);
}

UINT8 towns_state::towns_cd_get_track()
{
	cdrom_image_device* cdrom = m_cdrom;
	UINT32 lba = m_cdda->get_audio_lba();
	UINT8 track;

	for(track=1;track<99;track++)
	{
		if(cdrom_get_track_start(cdrom->get_cdrom_file(),track) > lba)
			break;
	}
	return track;
}

TIMER_CALLBACK_MEMBER(towns_state::towns_cdrom_read_byte)
{
	upd71071_device* device = (upd71071_device* )ptr;
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
			logerror("DMA1: end of transfer (LBA=%08x)\n",m_towns_cd.lba_current);
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
				cdrom_read_data(m_cdrom->get_cdrom_file(),++m_towns_cd.lba_current,m_towns_cd.buffer,CD_TRACK_MODE1);
				m_towns_cd.read_timer->adjust(attotime::from_hz(300000),1);
				m_towns_cd.buffer_ptr = -1;
			}
		}
	}
}

UINT8 towns_state::towns_cdrom_read_byte_software()
{
	UINT8 ret;
	if(m_towns_cd.buffer_ptr < 0) // transfer has ended
		return 0x00;

	ret = m_towns_cd.buffer[m_towns_cd.buffer_ptr++];

	if(m_towns_cd.buffer_ptr >= 2048)
	{  // end of transfer
		m_towns_cd.status &= ~0x10;  // no longer transferring by DMA
		m_towns_cd.status &= ~0x20;  // no longer transferring by software
		logerror("CD: end of software transfer (LBA=%08x)\n",m_towns_cd.lba_current);
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
			cdrom_read_data(m_cdrom->get_cdrom_file(),++m_towns_cd.lba_current,m_towns_cd.buffer,CD_TRACK_MODE1);
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
	UINT32 lba1,lba2,track;

	lba1 = m_towns_cd.parameter[7] << 16;
	lba1 += m_towns_cd.parameter[6] << 8;
	lba1 += m_towns_cd.parameter[5];
	lba2 = m_towns_cd.parameter[4] << 16;
	lba2 += m_towns_cd.parameter[3] << 8;
	lba2 += m_towns_cd.parameter[2];
	m_towns_cd.lba_current = msf_to_lbafm(lba1);
	m_towns_cd.lba_last = msf_to_lbafm(lba2);

	// first track starts at 00:02:00 - this is hardcoded in the boot procedure
	track = cdrom_get_track(device->get_cdrom_file(),m_towns_cd.lba_current);
	if(track < 2)
	{  // recalculate LBA
		m_towns_cd.lba_current -= 150;
		m_towns_cd.lba_last -= 150;
	}

	// parameter 7 = sector count?
	if(m_towns_cd.parameter[1] != 0)
		m_towns_cd.lba_last += m_towns_cd.parameter[1];

	logerror("CD: Mode 1 read from LBA next:%i last:%i track:%i\n",m_towns_cd.lba_current,m_towns_cd.lba_last,track);

	if(m_towns_cd.lba_current > m_towns_cd.lba_last)
	{
		m_towns_cd.extra_status = 0;
		towns_cd_set_status(0x01,0x00,0x00,0x00);
	}
	else
	{
		cdrom_read_data(device->get_cdrom_file(),m_towns_cd.lba_current,m_towns_cd.buffer,CD_TRACK_MODE1);
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
	UINT32 lba1,lba2;

	lba1 = m_towns_cd.parameter[7] << 16;
	lba1 += m_towns_cd.parameter[6] << 8;
	lba1 += m_towns_cd.parameter[5];
	lba2 = m_towns_cd.parameter[4] << 16;
	lba2 += m_towns_cd.parameter[3] << 8;
	lba2 += m_towns_cd.parameter[2];
	m_towns_cd.cdda_current = msf_to_lbafm(lba1);
	m_towns_cd.cdda_length = msf_to_lbafm(lba2) - m_towns_cd.cdda_current;

	m_cdda->set_cdrom(device->get_cdrom_file());
	m_cdda->start_audio(m_towns_cd.cdda_current,m_towns_cd.cdda_length);
	logerror("CD: CD-DA start from LBA:%i length:%i\n",m_towns_cd.cdda_current,m_towns_cd.cdda_length);
	if(m_towns_cd.command & 0x20)
	{
		m_towns_cd.extra_status = 1;
		towns_cd_set_status(0x00,0x03,0x00,0x00);
	}
}

void towns_state::towns_delay_cdda(cdrom_image_device* dev)
{
	towns_cdrom_play_cdda(dev);
}

void towns_state::towns_cdrom_execute_command(cdrom_image_device* device)
{
	if(device->get_cdrom_file() == NULL)
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
					m_towns_cd.extra_status = 1;
					towns_cd_set_status(0x00,0x00,0x00,0x00);
				}
				logerror("CD: Command 0x00: SEEK\n");
				break;
			case 0x01:  // unknown
				if(m_towns_cd.command & 0x20)
				{
					m_towns_cd.extra_status = 0;
					towns_cd_set_status(0x00,0xff,0xff,0xff);
				}
				logerror("CD: Command 0x01: unknown\n");
				break;
			case 0x02:  // Read (MODE1)
				logerror("CD: Command 0x02: READ MODE1\n");
				towns_cdrom_read(device);
				break;
			case 0x04:  // Play Audio Track
				logerror("CD: Command 0x04: PLAY CD-DA\n");
				m_towns_cdda_timer->set_ptr(device);
				m_towns_cdda_timer->adjust(attotime::from_msec(1),0,attotime::never);
				break;
			case 0x05:  // Read TOC
				logerror("CD: Command 0x05: READ TOC\n");
				m_towns_cd.extra_status = 1;
				towns_cd_set_status(0x00,0x00,0x00,0x00);
				break;
			case 0x06:  // Read CD-DA state?
				logerror("CD: Command 0x06: READ CD-DA STATE\n");
				m_towns_cd.extra_status = 1;
				towns_cd_set_status(0x00,0x00,0x00,0x00);
				break;
			case 0x80:  // set state
				logerror("CD: Command 0x80: set state\n");
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
				logerror("CD: Command 0x81: set state (CDDASET)\n");
				break;
			case 0x84:   // Stop CD audio track  -- generates no status output?
				if(m_towns_cd.command & 0x20)
				{
					m_towns_cd.extra_status = 1;
					towns_cd_set_status(0x00,0x00,0x00,0x00);
				}
				m_cdda->pause_audio(1);
				logerror("CD: Command 0x84: STOP CD-DA\n");
				break;
			case 0x85:   // Stop CD audio track (difference from 0x84?)
				if(m_towns_cd.command & 0x20)
				{
					m_towns_cd.extra_status = 1;
					towns_cd_set_status(0x00,0x00,0x00,0x00);
				}
				m_cdda->pause_audio(1);
				logerror("CD: Command 0x85: STOP CD-DA\n");
				break;
			case 0x87:  // Resume CD-DA playback
				if(m_towns_cd.command & 0x20)
				{
					m_towns_cd.extra_status = 1;
					towns_cd_set_status(0x00,0x03,0x00,0x00);
				}
				m_cdda->pause_audio(0);
				logerror("CD: Command 0x87: RESUME CD-DA\n");
				break;
			default:
				m_towns_cd.extra_status = 0;
				towns_cd_set_status(0x10,0x00,0x00,0x00);
				logerror("CD: Unknown or unimplemented command %02x\n",m_towns_cd.command);
		}
	}
}

READ16_MEMBER(towns_state::towns_cdrom_dma_r)
{
	if(m_towns_cd.buffer_ptr >= 2048)
		return 0x00;
	return m_towns_cd.buffer[m_towns_cd.buffer_ptr++];
}

READ8_MEMBER(towns_state::towns_cdrom_r)
{
	UINT32 addr = 0;
	UINT8 ret = 0;

	ret = m_towns_cd.cmd_status[m_towns_cd.cmd_status_ptr];

	switch(offset)
	{
		case 0x00:  // status
			//logerror("CD: status read, returning %02x\n",towns_cd.status);
			return m_towns_cd.status;
		case 0x01:  // command status
			if(m_towns_cd.cmd_status_ptr >= 3)
			{
				m_towns_cd.status &= ~0x02;
				// check for more status bytes
				if(m_towns_cd.extra_status != 0)
				{
					switch(m_towns_cd.command & 0x9f)
					{
						case 0x00:  // seek
							towns_cd_set_status(0x04,0x00,0x00,0x00);
							m_towns_cd.extra_status = 0;
							break;
						case 0x02:  // read
							if(m_towns_cd.extra_status == 2)
								towns_cd_set_status(0x22,0x00,0x00,0x00);
							m_towns_cd.extra_status = 0;
							break;
						case 0x04:  // play cdda
							towns_cd_set_status(0x07,0x00,0x00,0x00);
							m_towns_cd.extra_status = 0;
							break;
						case 0x05:  // read toc
							switch(m_towns_cd.extra_status)
							{
								case 1:
								case 3:
									towns_cd_set_status(0x16,0x00,0x00,0x00);
									m_towns_cd.extra_status++;
									break;
								case 2: // st1 = first track number (BCD)
									towns_cd_set_status(0x17,0x01,0x00,0x00);
									m_towns_cd.extra_status++;
									break;
								case 4: // st1 = last track number (BCD)
									towns_cd_set_status(0x17,
										byte_to_bcd(cdrom_get_last_track(m_cdrom->get_cdrom_file())),
										0x00,0x00);
									m_towns_cd.extra_status++;
									break;
								case 5:  // st1 = control/adr of track 0xaa?
									towns_cd_set_status(0x16,
										cdrom_get_adr_control(m_cdrom->get_cdrom_file(),0xaa),
										0xaa,0x00);
									m_towns_cd.extra_status++;
									break;
								case 6:  // st1/2/3 = address of track 0xaa? (BCD)
									addr = cdrom_get_track_start(m_cdrom->get_cdrom_file(),0xaa);
									addr = lba_to_msf(addr);
									towns_cd_set_status(0x17,
										(addr & 0xff0000) >> 16,(addr & 0x00ff00) >> 8,addr & 0x0000ff);
									m_towns_cd.extra_status++;
									break;
								default:  // same as case 5 and 6, but for each individual track
									if(m_towns_cd.extra_status & 0x01)
									{
										towns_cd_set_status(0x16,
											((cdrom_get_adr_control(m_cdrom->get_cdrom_file(),(m_towns_cd.extra_status/2)-3) & 0x0f) << 4)
											| ((cdrom_get_adr_control(m_cdrom->get_cdrom_file(),(m_towns_cd.extra_status/2)-3) & 0xf0) >> 4),
											(m_towns_cd.extra_status/2)-3,0x00);
										m_towns_cd.extra_status++;
									}
									else
									{
										addr = cdrom_get_track_start(m_cdrom->get_cdrom_file(),(m_towns_cd.extra_status/2)-4);
										addr = lba_to_msf(addr);
										towns_cd_set_status(0x17,
											(addr & 0xff0000) >> 16,(addr & 0x00ff00) >> 8,addr & 0x0000ff);
										if(((m_towns_cd.extra_status/2)-3) >= cdrom_get_last_track(m_cdrom->get_cdrom_file()))
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
									addr = lba_to_msf(addr - m_towns_cd.cdda_current);
									towns_cd_set_status(0x19,
										(addr & 0xff0000) >> 16,(addr & 0x00ff00) >> 8,addr & 0x0000ff);
									m_towns_cd.extra_status++;
									break;
								case 3:  // st1/2 = current MSF
									addr = m_cdda->get_audio_lba();
									addr = lba_to_msf(addr);  // this data is incorrect, but will do until exact meaning is found
									towns_cd_set_status(0x19,
										0x00,(addr & 0xff0000) >> 16,(addr & 0x00ff00) >> 8);
									m_towns_cd.extra_status++;
									break;
								case 4:
									addr = m_cdda->get_audio_lba();
									addr = lba_to_msf(addr);  // this data is incorrect, but will do until exact meaning is found
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
			}
			logerror("CD: reading command status port (%i), returning %02x\n",m_towns_cd.cmd_status_ptr,ret);
			m_towns_cd.cmd_status_ptr++;
			if(m_towns_cd.cmd_status_ptr > 3)
			{
				m_towns_cd.cmd_status_ptr = 0;
/*              if(m_towns_cd.extra_status != 0)
                {
                    towns_cdrom_set_irq(space.machine(),TOWNS_CD_IRQ_MPU,1);
                    m_towns_cd.status |= 0x02;
                }*/
			}
			return ret;
		case 0x02:  // data transfer (used in software transfers)
			if(m_towns_cd.software_tx)
			{
				return towns_cdrom_read_byte_software();
			}
		default:
			return 0x00;
	}
}

WRITE8_MEMBER(towns_state::towns_cdrom_w)
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
				logerror("CD: sub MPU reset\n");
			m_towns_cd.mpu_irq_enable = data & 0x02;
			m_towns_cd.dma_irq_enable = data & 0x01;
			logerror("CD: status write %02x\n",data);
			break;
		case 0x01: // command
			m_towns_cd.command = data;
			towns_cdrom_execute_command(m_cdrom);
			logerror("CD: command %02x sent\n",data);
			logerror("CD: parameters: %02x %02x %02x %02x %02x %02x %02x %02x\n",
				m_towns_cd.parameter[7],m_towns_cd.parameter[6],m_towns_cd.parameter[5],
				m_towns_cd.parameter[4],m_towns_cd.parameter[3],m_towns_cd.parameter[2],
				m_towns_cd.parameter[1],m_towns_cd.parameter[0]);
			break;
		case 0x02: // parameter
			for(x=7;x>0;x--)
				m_towns_cd.parameter[x] = m_towns_cd.parameter[x-1];
			m_towns_cd.parameter[0] = data;
			logerror("CD: parameter %02x added\n",data);
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
			logerror("CD: transfer mode write %02x\n",data);
			break;
		default:
			logerror("CD: write %02x to port %02x\n",data,offset*2);
	}
}


/* CMOS RTC
 * 0x70: Data port
 * 0x80: Register select
 */
READ8_MEMBER(towns_state::towns_rtc_r)
{
	return 0x80 | m_towns_rtc_reg[m_towns_rtc_select];
}

WRITE8_MEMBER(towns_state::towns_rtc_w)
{
	m_towns_rtc_data = data;
}

WRITE8_MEMBER(towns_state::towns_rtc_select_w)
{
	if(data & 0x80)
	{
		if(data & 0x01)
			m_towns_rtc_select = m_towns_rtc_data & 0x0f;
	}
}

void towns_state::rtc_hour()
{
	m_towns_rtc_reg[4]++;
	if(m_towns_rtc_reg[4] > 4 && m_towns_rtc_reg[5] == 2)
	{
		m_towns_rtc_reg[4] = 0;
		m_towns_rtc_reg[5] = 0;
	}
	else if(m_towns_rtc_reg[4] > 9)
	{
		m_towns_rtc_reg[4] = 0;
		m_towns_rtc_reg[5]++;
	}
}

void towns_state::rtc_minute()
{
	m_towns_rtc_reg[2]++;
	if(m_towns_rtc_reg[2] > 9)
	{
		m_towns_rtc_reg[2] = 0;
		m_towns_rtc_reg[3]++;
		if(m_towns_rtc_reg[3] > 5)
		{
			m_towns_rtc_reg[3] = 0;
			rtc_hour();
		}
	}
}

void towns_state::rtc_second()
{
	// increase RTC time by one second
	m_towns_rtc_reg[0]++;
	if(m_towns_rtc_reg[0] > 9)
	{
		m_towns_rtc_reg[0] = 0;
		m_towns_rtc_reg[1]++;
		if(m_towns_rtc_reg[1] > 5)
		{
			m_towns_rtc_reg[1] = 0;
			rtc_minute();
		}
	}
}

// SCSI controller - I/O ports 0xc30 and 0xc32
READ16_MEMBER(towns_state::towns_scsi_dma_r)
{
	return m_scsi->fmscsi_data_r();
}

WRITE16_MEMBER(towns_state::towns_scsi_dma_w)
{
	m_scsi->fmscsi_data_w(data & 0xff);
}

WRITE_LINE_MEMBER(towns_state::towns_scsi_irq)
{
	m_pic_slave->ir0_w(state);
	if(IRQ_LOG)
		logerror("PIC: IRQ8 (SCSI) set to %i\n",state);
}

WRITE_LINE_MEMBER(towns_state::towns_scsi_drq)
{
	m_dma_1->dmarq(state, 1);  // SCSI HDs use channel 1
}


// Volume ports - I/O ports 0x4e0-0x4e3
// 0x4e0 = input volume level
// 0x4e1 = input channel select
//         4 = Line in, left channel
//         5 = Line in, right channel
// 0x4e2 = output volume level
// 0x4e3 = output channel select
//         2 = MIC
//         3 = MODEM
//         4 = CD-DA left channel
//         5 = CD-DA right channel
READ8_MEMBER(towns_state::towns_volume_r)
{
	switch(offset)
	{
	case 2:
		return(m_towns_volume[m_towns_volume_select]);
	case 3:
		return m_towns_volume_select;
	default:
		return 0;
	}
}

WRITE8_MEMBER(towns_state::towns_volume_w)
{
	switch(offset)
	{
	case 2:
		m_towns_volume[m_towns_volume_select] = data;
		if(m_towns_volume_select == 4)
			m_cdda->set_channel_volume(0,100.0f * (data / 64.0f));
		if(m_towns_volume_select == 5)
			m_cdda->set_channel_volume(1,100.0f * (data / 64.0f));
		break;
	case 3:  // select channel
		if(data < 8)
			m_towns_volume_select = data;
		break;
	default:
		logerror("SND: Volume port %i set to %02x\n",offset,data);
	}
}

// some unknown ports...
READ8_MEMBER(towns_state::towns_41ff_r)
{
	logerror("I/O port 0x41ff read\n");
	return 0x01;
}

// YM3438 interrupt (IRQ 13)
WRITE_LINE_MEMBER(towns_state::towns_fm_irq)
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

WRITE_LINE_MEMBER(towns_state::towns_pit_out0_changed)
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

WRITE_LINE_MEMBER(towns_state::towns_pit_out1_changed)
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

WRITE_LINE_MEMBER( towns_state::pit_out2_changed )
{
	m_pit_out2 = state ? 1 : 0;
	m_speaker->level_w(speaker_get_spk());
}

static ADDRESS_MAP_START(towns_mem, AS_PROGRAM, 32, towns_state)
	// memory map based on FM-Towns/Bochs (Bochs modified to emulate the FM-Towns)
	// may not be (and probably is not) correct
	AM_RANGE(0x00000000, 0x000bffff) AM_RAM
	AM_RANGE(0x000c0000, 0x000c7fff) AM_READWRITE8(towns_gfx_r,towns_gfx_w,0xffffffff)
	AM_RANGE(0x000c8000, 0x000cafff) AM_READWRITE8(towns_spriteram_low_r,towns_spriteram_low_w,0xffffffff)
	AM_RANGE(0x000cb000, 0x000cbfff) AM_READ_BANK("bank6") AM_WRITE_BANK("bank7")
	AM_RANGE(0x000cc000, 0x000cff7f) AM_RAM
	AM_RANGE(0x000cff80, 0x000cffff) AM_READWRITE8(towns_video_cff80_mem_r,towns_video_cff80_mem_w,0xffffffff)
	AM_RANGE(0x000d0000, 0x000d7fff) AM_RAM
	AM_RANGE(0x000d8000, 0x000d9fff) AM_READWRITE8(towns_cmos_low_r,towns_cmos_low_w,0xffffffff) AM_SHARE("nvram") // CMOS? RAM
	AM_RANGE(0x000da000, 0x000effff) AM_RAM //READWRITE(SMH_BANK(11),SMH_BANK(11))
	AM_RANGE(0x000f0000, 0x000f7fff) AM_RAM //READWRITE(SMH_BANK(12),SMH_BANK(12))
	AM_RANGE(0x000f8000, 0x000fffff) AM_READ_BANK("bank11") AM_WRITE_BANK("bank12")
//  AM_RANGE(0x00100000, 0x005fffff) AM_RAM  // some extra RAM
	AM_RANGE(0x80000000, 0x8007ffff) AM_READWRITE8(towns_gfx_high_r,towns_gfx_high_w,0xffffffff) AM_MIRROR(0x180000) // VRAM
	AM_RANGE(0x81000000, 0x8101ffff) AM_READWRITE8(towns_spriteram_r,towns_spriteram_w,0xffffffff) // Sprite RAM
	// 0xc0000000 - 0xc0ffffff  // IC Memory Card (static, first 16MB only)
	// 0xc1000000 - 0xc1ffffff  // IC Memory Card (banked, can show any of 4 banks), JEIDA v4 only (UX and later)
	AM_RANGE(0xc2000000, 0xc207ffff) AM_ROM AM_REGION("user",0x000000)  // OS ROM
	AM_RANGE(0xc2080000, 0xc20fffff) AM_ROM AM_REGION("user",0x100000)  // DIC ROM
	AM_RANGE(0xc2100000, 0xc213ffff) AM_ROM AM_REGION("user",0x180000)  // FONT ROM
	AM_RANGE(0xc2140000, 0xc2141fff) AM_READWRITE8(towns_cmos_r,towns_cmos_w,0xffffffff) // CMOS (mirror?)
	AM_RANGE(0xc2180000, 0xc21fffff) AM_ROM AM_REGION("user",0x080000)  // F20 ROM
	AM_RANGE(0xc2200000, 0xc220ffff) AM_DEVREADWRITE8("pcm", rf5c68_device, rf5c68_mem_r, rf5c68_mem_w, 0xffffffff)  // WAVE RAM
	AM_RANGE(0xfffc0000, 0xffffffff) AM_ROM AM_REGION("user",0x200000)  // SYSTEM ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(marty_mem, AS_PROGRAM, 16, towns_state)
	AM_RANGE(0x00000000, 0x000bffff) AM_RAM
	AM_RANGE(0x000c0000, 0x000c7fff) AM_READWRITE8(towns_gfx_r,towns_gfx_w,0xffff)
	AM_RANGE(0x000c8000, 0x000cafff) AM_READWRITE8(towns_spriteram_low_r,towns_spriteram_low_w,0xffff)
	AM_RANGE(0x000cb000, 0x000cbfff) AM_READ_BANK("bank6") AM_WRITE_BANK("bank7")
	AM_RANGE(0x000cc000, 0x000cff7f) AM_RAM
	AM_RANGE(0x000cff80, 0x000cffff) AM_READWRITE8(towns_video_cff80_mem_r,towns_video_cff80_mem_w,0xffff)
	AM_RANGE(0x000d0000, 0x000d7fff) AM_RAM
	AM_RANGE(0x000d8000, 0x000d9fff) AM_READWRITE8(towns_cmos_low_r,towns_cmos_low_w,0xffff) AM_SHARE("nvram16") // CMOS? RAM
	AM_RANGE(0x000da000, 0x000effff) AM_RAM //READWRITE(SMH_BANK(11),SMH_BANK(11))
	AM_RANGE(0x000f0000, 0x000f7fff) AM_RAM //READWRITE(SMH_BANK(12),SMH_BANK(12))
	AM_RANGE(0x000f8000, 0x000fffff) AM_READ_BANK("bank11") AM_WRITE_BANK("bank12")
//  AM_RANGE(0x00100000, 0x005fffff) AM_RAM  // some extra RAM - the Marty has 6MB RAM (not upgradable)
	AM_RANGE(0x00600000, 0x0067ffff) AM_ROM AM_REGION("user",0x000000)  // OS
	AM_RANGE(0x00680000, 0x0087ffff) AM_ROM AM_REGION("user",0x280000)  // EX ROM
	AM_RANGE(0x00a00000, 0x00a7ffff) AM_READWRITE8(towns_gfx_high_r,towns_gfx_high_w,0xffff) AM_MIRROR(0x180000) // VRAM
	AM_RANGE(0x00b00000, 0x00b7ffff) AM_ROM AM_REGION("user",0x180000)  // FONT
	AM_RANGE(0x00c00000, 0x00c1ffff) AM_READWRITE8(towns_spriteram_r,towns_spriteram_w,0xffff) // Sprite RAM
	AM_RANGE(0x00d00000, 0x00dfffff) AM_RAM // IC Memory Card (is this usable on the Marty?)
	AM_RANGE(0x00e80000, 0x00efffff) AM_ROM AM_REGION("user",0x100000)  // DIC ROM
	AM_RANGE(0x00f00000, 0x00f7ffff) AM_ROM AM_REGION("user",0x180000)  // FONT
	AM_RANGE(0x00f80000, 0x00f8ffff) AM_DEVREADWRITE8("pcm", rf5c68_device, rf5c68_mem_r, rf5c68_mem_w, 0xffff)  // WAVE RAM
	AM_RANGE(0x00fc0000, 0x00ffffff) AM_ROM AM_REGION("user",0x200000)  // SYSTEM ROM
	AM_RANGE(0xfffc0000, 0xffffffff) AM_ROM AM_REGION("user",0x200000)  // SYSTEM ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(ux_mem, AS_PROGRAM, 16, towns_state)
	AM_RANGE(0x00000000, 0x000bffff) AM_RAM
	AM_RANGE(0x000c0000, 0x000c7fff) AM_READWRITE8(towns_gfx_r,towns_gfx_w,0xffff)
	AM_RANGE(0x000c8000, 0x000cafff) AM_READWRITE8(towns_spriteram_low_r,towns_spriteram_low_w,0xffff)
	AM_RANGE(0x000cb000, 0x000cbfff) AM_READ_BANK("bank6") AM_WRITE_BANK("bank7")
	AM_RANGE(0x000cc000, 0x000cff7f) AM_RAM
	AM_RANGE(0x000cff80, 0x000cffff) AM_READWRITE8(towns_video_cff80_mem_r,towns_video_cff80_mem_w,0xffff)
	AM_RANGE(0x000d0000, 0x000d7fff) AM_RAM
	AM_RANGE(0x000d8000, 0x000d9fff) AM_READWRITE8(towns_cmos_low_r,towns_cmos_low_w,0xffff) AM_SHARE("nvram16") // CMOS? RAM
	AM_RANGE(0x000da000, 0x000effff) AM_RAM //READWRITE(SMH_BANK(11),SMH_BANK(11))
	AM_RANGE(0x000f0000, 0x000f7fff) AM_RAM //READWRITE(SMH_BANK(12),SMH_BANK(12))
	AM_RANGE(0x000f8000, 0x000fffff) AM_READ_BANK("bank11") AM_WRITE_BANK("bank12")
//  AM_RANGE(0x00680000, 0x0087ffff) AM_ROM AM_REGION("user",0x280000)  // EX ROM
	AM_RANGE(0x00a00000, 0x00a7ffff) AM_READWRITE8(towns_gfx_high_r,towns_gfx_high_w,0xffff) AM_MIRROR(0x180000) // VRAM
	AM_RANGE(0x00b00000, 0x00b7ffff) AM_ROM AM_REGION("user",0x180000)  // FONT
	AM_RANGE(0x00c00000, 0x00c1ffff) AM_READWRITE8(towns_spriteram_r,towns_spriteram_w,0xffff) // Sprite RAM
	AM_RANGE(0x00d00000, 0x00dfffff) AM_RAM // IC Memory Card
	AM_RANGE(0x00e00000, 0x00e7ffff) AM_ROM AM_REGION("user",0x000000)  // OS
	AM_RANGE(0x00e80000, 0x00efffff) AM_ROM AM_REGION("user",0x100000)  // DIC ROM
	AM_RANGE(0x00f00000, 0x00f7ffff) AM_ROM AM_REGION("user",0x180000)  // FONT
	AM_RANGE(0x00f80000, 0x00f8ffff) AM_DEVREADWRITE8("pcm", rf5c68_device, rf5c68_mem_r, rf5c68_mem_w, 0xffff)  // WAVE RAM
	AM_RANGE(0x00fc0000, 0x00ffffff) AM_ROM AM_REGION("user",0x200000)  // SYSTEM ROM
	AM_RANGE(0xfffc0000, 0xffffffff) AM_ROM AM_REGION("user",0x200000)  // SYSTEM ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( towns_io , AS_IO, 32, towns_state)
	// I/O ports derived from FM Towns/Bochs, these are specific to the FM Towns
	// System ports
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000,0x0003) AM_DEVREADWRITE8("pic8259_master", pic8259_device, read, write, 0x00ff00ff)
	AM_RANGE(0x0010,0x0013) AM_DEVREADWRITE8("pic8259_slave", pic8259_device, read, write, 0x00ff00ff)
	AM_RANGE(0x0020,0x0033) AM_READWRITE8(towns_system_r,towns_system_w, 0xffffffff)
	AM_RANGE(0x0040,0x0047) AM_DEVREADWRITE8("pit", pit8253_device, read, write, 0x00ff00ff)
	AM_RANGE(0x0050,0x0057) AM_DEVREADWRITE8("pit2", pit8253_device, read, write, 0x00ff00ff)
	AM_RANGE(0x0060,0x0063) AM_READWRITE8(towns_port60_r, towns_port60_w, 0x000000ff)
	AM_RANGE(0x0068,0x006b) AM_READWRITE8(towns_intervaltimer2_r, towns_intervaltimer2_w, 0xffffffff)
	AM_RANGE(0x006c,0x006f) AM_READWRITE8(towns_sys6c_r,towns_sys6c_w, 0x000000ff)
	// 0x0070/0x0080 - CMOS RTC
	AM_RANGE(0x0070,0x0073) AM_READWRITE8(towns_rtc_r,towns_rtc_w,0x000000ff)
	AM_RANGE(0x0080,0x0083) AM_WRITE8(towns_rtc_select_w,0x000000ff)
	// DMA controllers (uPD71071)
	AM_RANGE(0x00a0,0x00af) AM_READWRITE8(towns_dma1_r, towns_dma1_w, 0xffffffff)
	AM_RANGE(0x00b0,0x00bf) AM_READWRITE8(towns_dma2_r, towns_dma2_w, 0xffffffff)
	// Floppy controller
	AM_RANGE(0x0200,0x020f) AM_READWRITE8(towns_floppy_r, towns_floppy_w, 0xffffffff)
	// CRTC / Video
	AM_RANGE(0x0400,0x0403) AM_READ8(towns_video_unknown_r, 0x000000ff)  // R/O (0x400)
	AM_RANGE(0x0404,0x0407) AM_READWRITE8(towns_video_404_r, towns_video_404_w, 0x000000ff)  // R/W (0x404)
	AM_RANGE(0x0440,0x045f) AM_READWRITE8(towns_video_440_r, towns_video_440_w, 0xffffffff)
	// System port
	AM_RANGE(0x0480,0x0483) AM_READWRITE8(towns_sys480_r,towns_sys480_w,0x000000ff)  // R/W (0x480)
	// CD-ROM
	AM_RANGE(0x04c0,0x04cf) AM_READWRITE8(towns_cdrom_r,towns_cdrom_w,0x00ff00ff)
	// Joystick / Mouse ports
	AM_RANGE(0x04d0,0x04d3) AM_READ8(towns_padport_r, 0x00ff00ff)
	AM_RANGE(0x04d4,0x04d7) AM_WRITE8(towns_pad_mask_w, 0x00ff0000)
	// Sound (YM3438 [FM], RF5c68 [PCM])
	AM_RANGE(0x04d8,0x04df) AM_DEVREADWRITE8("fm", ym3438_device, read, write, 0x00ff00ff)
	AM_RANGE(0x04e0,0x04e3) AM_READWRITE8(towns_volume_r,towns_volume_w,0xffffffff)  // R/W  -- volume ports
	AM_RANGE(0x04e8,0x04ef) AM_READWRITE8(towns_sound_ctrl_r,towns_sound_ctrl_w,0xffffffff)
	AM_RANGE(0x04f0,0x04fb) AM_DEVWRITE8("pcm", rf5c68_device, rf5c68_w, 0xffffffff)
	// CRTC / Video
	AM_RANGE(0x05c8,0x05cb) AM_READWRITE8(towns_video_5c8_r, towns_video_5c8_w, 0xffffffff)
	// System ports
	AM_RANGE(0x05e8,0x05ef) AM_READWRITE8(towns_sys5e8_r, towns_sys5e8_w, 0x00ff00ff)
	// Keyboard (8042 MCU)
	AM_RANGE(0x0600,0x0607) AM_READWRITE8(towns_keyboard_r, towns_keyboard_w,0x00ff00ff)
	// SCSI controller
	AM_RANGE(0x0c30,0x0c37) AM_DEVREADWRITE8("fmscsi",fmscsi_device,fmscsi_r,fmscsi_w,0x00ff00ff)
	// CMOS
	AM_RANGE(0x3000,0x4fff) AM_READWRITE8(towns_cmos_r, towns_cmos_w,0x00ff00ff)
	// Something (MS-DOS wants this 0x41ff to be 1)
	//AM_RANGE(0x41fc,0x41ff) AM_READ8(towns_41ff_r,0xff000000)
	// CRTC / Video (again)
	AM_RANGE(0xfd90,0xfda3) AM_READWRITE8(towns_video_fd90_r, towns_video_fd90_w, 0xffffffff)
	AM_RANGE(0xff80,0xffff) AM_READWRITE8(towns_video_cff80_r,towns_video_cff80_w,0xffffffff)
ADDRESS_MAP_END

static ADDRESS_MAP_START( towns16_io , AS_IO, 16, towns_state)  // for the 386SX based systems
	// System ports
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000,0x0003) AM_DEVREADWRITE8("pic8259_master", pic8259_device, read, write, 0x00ff)
	AM_RANGE(0x0010,0x0013) AM_DEVREADWRITE8("pic8259_slave", pic8259_device, read, write, 0x00ff)
	AM_RANGE(0x0020,0x0033) AM_READWRITE8(towns_system_r,towns_system_w, 0xffff)
	AM_RANGE(0x0040,0x0047) AM_DEVREADWRITE8("pit", pit8253_device, read, write, 0x00ff)
	AM_RANGE(0x0050,0x0057) AM_DEVREADWRITE8("pit2", pit8253_device, read, write, 0x00ff)
	AM_RANGE(0x0060,0x0061) AM_READWRITE8(towns_port60_r, towns_port60_w, 0x00ff)
	AM_RANGE(0x0068,0x006b) AM_READWRITE8(towns_intervaltimer2_r, towns_intervaltimer2_w, 0xffff)
	AM_RANGE(0x006c,0x006d) AM_READWRITE8(towns_sys6c_r,towns_sys6c_w, 0x00ff)
	// 0x0070/0x0080 - CMOS RTC
	AM_RANGE(0x0070,0x0071) AM_READWRITE8(towns_rtc_r,towns_rtc_w,0x00ff)
	AM_RANGE(0x0080,0x0081) AM_WRITE8(towns_rtc_select_w,0x00ff)
	// DMA controllers (uPD71071)
	AM_RANGE(0x00a0,0x00af) AM_READWRITE8(towns_dma1_r, towns_dma1_w, 0xffff)
	AM_RANGE(0x00b0,0x00bf) AM_READWRITE8(towns_dma2_r, towns_dma2_w, 0xffff)
	// Floppy controller
	AM_RANGE(0x0200,0x020f) AM_READWRITE8(towns_floppy_r, towns_floppy_w, 0xffff)
	// CRTC / Video
	AM_RANGE(0x0400,0x0401) AM_READ8(towns_video_unknown_r, 0x00ff)  // R/O (0x400)
	AM_RANGE(0x0404,0x0407) AM_READWRITE8(towns_video_404_r, towns_video_404_w, 0xffff)  // R/W (0x404)
	AM_RANGE(0x0440,0x045f) AM_READWRITE8(towns_video_440_r, towns_video_440_w, 0xffff)
	// System port
	AM_RANGE(0x0480,0x0481) AM_READWRITE8(towns_sys480_r,towns_sys480_w,0x00ff)  // R/W (0x480)
	// CD-ROM
	AM_RANGE(0x04c0,0x04cf) AM_READWRITE8(towns_cdrom_r,towns_cdrom_w,0x00ff)
	// Joystick / Mouse ports
	AM_RANGE(0x04d0,0x04d3) AM_READ8(towns_padport_r, 0x00ff)
	AM_RANGE(0x04d6,0x04d7) AM_WRITE8(towns_pad_mask_w, 0x00ff)
	// Sound (YM3438 [FM], RF5c68 [PCM])
	AM_RANGE(0x04d8,0x04df) AM_DEVREADWRITE8("fm", ym3438_device, read, write, 0x00ff)
	AM_RANGE(0x04e0,0x04e3) AM_READWRITE8(towns_volume_r,towns_volume_w,0xffff)  // R/W  -- volume ports
	AM_RANGE(0x04e8,0x04ef) AM_READWRITE8(towns_sound_ctrl_r,towns_sound_ctrl_w,0xffff)
	AM_RANGE(0x04f0,0x04fb) AM_DEVWRITE8("pcm", rf5c68_device, rf5c68_w, 0xffff)
	// CRTC / Video
	AM_RANGE(0x05c8,0x05cb) AM_READWRITE8(towns_video_5c8_r, towns_video_5c8_w, 0xffff)
	// System ports
	AM_RANGE(0x05e8,0x05ef) AM_READWRITE8(towns_sys5e8_r, towns_sys5e8_w, 0x00ff)
	// Keyboard (8042 MCU)
	AM_RANGE(0x0600,0x0607) AM_READWRITE8(towns_keyboard_r, towns_keyboard_w,0x00ff)
	// SCSI controller
	AM_RANGE(0x0c30,0x0c37) AM_DEVREADWRITE8("fmscsi",fmscsi_device,fmscsi_r,fmscsi_w,0x00ff)
	// CMOS
	AM_RANGE(0x3000,0x4fff) AM_READWRITE8(towns_cmos_r, towns_cmos_w,0x00ff)
	// Something (MS-DOS wants this 0x41ff to be 1)
	//AM_RANGE(0x41fc,0x41ff) AM_READ8(towns_41ff_r,0xff000000)
	// CRTC / Video (again)
	AM_RANGE(0xfd90,0xfda3) AM_READWRITE8(towns_video_fd90_r, towns_video_fd90_w, 0xffff)
	AM_RANGE(0xff80,0xffff) AM_READWRITE8(towns_video_cff80_r,towns_video_cff80_w,0xffff)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( towns )
	PORT_START("ctrltype")
	PORT_CONFNAME(0x0f, 0x01, "Joystick port 1")
	PORT_CONFSETTING(0x00, "Nothing")
	PORT_CONFSETTING(0x01, "Standard 2-button joystick")
	PORT_CONFSETTING(0x02, "Mouse")
	PORT_CONFSETTING(0x04, "6-button joystick")
	PORT_CONFNAME(0xf0, 0x20, "Joystick port 2")
	PORT_CONFSETTING(0x00, "Nothing")
	PORT_CONFSETTING(0x10, "Standard 2-button joystick")
	PORT_CONFSETTING(0x20, "Mouse")
	PORT_CONFSETTING(0x40, "6-button joystick")

// Keyboard
	PORT_START( "key1" )  // scancodes 0x00-0x1f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ESC") PORT_CODE(KEYCODE_TILDE) PORT_CHAR(27)
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("-") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("^") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^')
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xEF\xBF\xA5") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Backspace") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tab") PORT_CODE(KEYCODE_TAB) PORT_CHAR(9)
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Q") PORT_CODE(KEYCODE_Q) PORT_CHAR('Q')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("W") PORT_CODE(KEYCODE_W) PORT_CHAR('W')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("R") PORT_CODE(KEYCODE_R) PORT_CHAR('R')
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("T") PORT_CODE(KEYCODE_T) PORT_CHAR('T')
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Y") PORT_CODE(KEYCODE_Y) PORT_CHAR('Y')
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("U") PORT_CODE(KEYCODE_U) PORT_CHAR('U')
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("I") PORT_CODE(KEYCODE_I) PORT_CHAR('I')
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("O") PORT_CODE(KEYCODE_O) PORT_CHAR('O')
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("P") PORT_CODE(KEYCODE_P) PORT_CHAR('P')
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("@") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('@')
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("[") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('[')
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(27)
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("S") PORT_CODE(KEYCODE_S) PORT_CHAR('S')

	PORT_START( "key2" )  // scancodes 0x20-0x3f
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("G") PORT_CODE(KEYCODE_G) PORT_CHAR('G')
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("H") PORT_CODE(KEYCODE_H) PORT_CHAR('H')
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("J") PORT_CODE(KEYCODE_J) PORT_CHAR('J')
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("K") PORT_CODE(KEYCODE_K) PORT_CHAR('K')
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("L") PORT_CODE(KEYCODE_L) PORT_CHAR('L')
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(";") PORT_CODE(KEYCODE_COLON) PORT_CHAR(';')
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(":") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':')
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("]") PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR(']')
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Z") PORT_CODE(KEYCODE_Z) PORT_CHAR('Z')
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("X") PORT_CODE(KEYCODE_X) PORT_CHAR('X')
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("V") PORT_CODE(KEYCODE_V) PORT_CHAR('V')
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("N") PORT_CODE(KEYCODE_N) PORT_CHAR('N')
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("M") PORT_CODE(KEYCODE_M) PORT_CHAR('M')
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(",") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',')
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME(".") PORT_CODE(KEYCODE_STOP) PORT_CHAR('.')
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("/") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/')
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("` _")
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
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("INS(?)") PORT_CODE(KEYCODE_INSERT)
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Tenkey 000")
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("DEL(?)/EL") PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("HOME") PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Down") PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Right") PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x00100000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00200000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("CAP") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT(0x00400000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xE3\x81\xB2\xE3\x82\x89\xE3\x81\x8C\xE3\x81\xAA (Hiragana)") PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x00800000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Key 0x57")
	PORT_BIT(0x01000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Key 0x58")
	PORT_BIT(0x02000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Kana???") PORT_CODE(KEYCODE_RALT)
	PORT_BIT(0x04000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("\xE3\x82\xAB\xE3\x82\xBF\xE3\x82\xAB\xE3\x83\x8A (Katakana)") PORT_CODE(KEYCODE_RWIN)
	PORT_BIT(0x08000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF12") PORT_CODE(KEYCODE_F12)
	PORT_BIT(0x10000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("ALT") PORT_CODE(KEYCODE_LALT)
	PORT_BIT(0x20000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x40000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x80000000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF3") PORT_CODE(KEYCODE_F3)

	PORT_START("key4")
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF5") PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x00000004,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF6") PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x00000008,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF7") PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF8") PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF9") PORT_CODE(KEYCODE_F9)
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF10") PORT_CODE(KEYCODE_F10)
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000100,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000200,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("PF11") PORT_CODE(KEYCODE_F11)
	PORT_BIT(0x00000400,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00000800,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Key 0x6b")
	PORT_BIT(0x00001000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Key 0x6c")
	PORT_BIT(0x00002000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Key 0x6d")
	PORT_BIT(0x00004000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Key 0x6e")
	PORT_BIT(0x00008000,IP_ACTIVE_HIGH,IPT_UNUSED)
	PORT_BIT(0x00010000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Key 0x70")
	PORT_BIT(0x00020000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Key 0x71")
	PORT_BIT(0x00040000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Key 0x72")
	PORT_BIT(0x00080000,IP_ACTIVE_HIGH,IPT_KEYBOARD) PORT_NAME("Key 0x73")
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

	PORT_START("joy1")
	PORT_BIT(0x00000001,IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x00000002,IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x00000004,IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x00000008,IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x00000010,IP_ACTIVE_LOW, IPT_UNUSED) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x00000020,IP_ACTIVE_LOW, IPT_UNUSED) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH, IPT_UNUSED) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH, IPT_UNUSED) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)

	PORT_START("joy1_ex")
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH, IPT_START) PORT_NAME("1P Run") PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("1P Select") PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x00000004,IP_ACTIVE_LOW, IPT_UNUSED) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x00000008,IP_ACTIVE_LOW, IPT_UNUSED) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH, IPT_UNUSED) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH, IPT_UNUSED) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x01)

	PORT_START("joy2")
	PORT_BIT(0x00000001,IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT(0x00000002,IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT(0x00000004,IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT(0x00000008,IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT(0x00000010,IP_ACTIVE_LOW, IPT_UNUSED) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT(0x00000020,IP_ACTIVE_LOW, IPT_UNUSED) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH, IPT_UNUSED) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH, IPT_UNUSED) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)

	PORT_START("joy2_ex")
	PORT_BIT(0x00000001,IP_ACTIVE_HIGH, IPT_START) PORT_NAME("2P Run") PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT(0x00000002,IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("2P Select") PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT(0x00000004,IP_ACTIVE_LOW, IPT_UNUSED) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT(0x00000008,IP_ACTIVE_LOW, IPT_UNUSED) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT(0x00000010,IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT(0x00000020,IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT(0x00000040,IP_ACTIVE_HIGH, IPT_UNUSED) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)
	PORT_BIT(0x00000080,IP_ACTIVE_HIGH, IPT_UNUSED) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x10)

	PORT_START("6b_joy1")
		PORT_BIT(0x00000001,IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x04)
		PORT_BIT(0x00000002,IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x04)
		PORT_BIT(0x00000004,IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x04)
		PORT_BIT(0x00000008,IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x04)
		PORT_BIT(0x00000010,IP_ACTIVE_LOW, IPT_UNUSED) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x04)
		PORT_BIT(0x00000020,IP_ACTIVE_LOW, IPT_UNUSED) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x04)
		PORT_BIT(0x00000040,IP_ACTIVE_HIGH, IPT_UNUSED) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x04)
		PORT_BIT(0x00000080,IP_ACTIVE_HIGH, IPT_UNUSED) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x04)

	PORT_START("6b_joy1_ex")
		PORT_BIT(0x00000001,IP_ACTIVE_HIGH, IPT_START) PORT_NAME("1P Run") PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x04)
		PORT_BIT(0x00000002,IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("1P Select") PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x04)
		PORT_BIT(0x00000004,IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x04)
		PORT_BIT(0x00000008,IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x04)
		PORT_BIT(0x00000010,IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x04)
		PORT_BIT(0x00000020,IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x04)
		PORT_BIT(0x00000040,IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x04)
		PORT_BIT(0x00000080,IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_PLAYER(1) PORT_CONDITION("ctrltype", 0x0f, EQUALS, 0x04)

	PORT_START("6b_joy2")
		PORT_BIT(0x00000001,IP_ACTIVE_LOW, IPT_JOYSTICK_UP) PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x40)
		PORT_BIT(0x00000002,IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN) PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x40)
		PORT_BIT(0x00000004,IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x40)
		PORT_BIT(0x00000008,IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x40)
		PORT_BIT(0x00000010,IP_ACTIVE_LOW, IPT_UNUSED) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x40)
		PORT_BIT(0x00000020,IP_ACTIVE_LOW, IPT_UNUSED) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x40)
		PORT_BIT(0x00000040,IP_ACTIVE_HIGH, IPT_UNUSED) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x40)
		PORT_BIT(0x00000080,IP_ACTIVE_HIGH, IPT_UNUSED) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x40)

	PORT_START("6b_joy2_ex")
		PORT_BIT(0x00000001,IP_ACTIVE_HIGH, IPT_START) PORT_NAME("2P Run") PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x40)
		PORT_BIT(0x00000002,IP_ACTIVE_HIGH, IPT_BUTTON7) PORT_NAME("2P Select") PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x40)
		PORT_BIT(0x00000004,IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x40)
		PORT_BIT(0x00000008,IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x40)
		PORT_BIT(0x00000010,IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x40)
		PORT_BIT(0x00000020,IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x40)
		PORT_BIT(0x00000040,IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x40)
		PORT_BIT(0x00000080,IP_ACTIVE_HIGH, IPT_BUTTON6) PORT_PLAYER(2) PORT_CONDITION("ctrltype", 0xf0, EQUALS, 0x40)

	PORT_START("mouse1")  // buttons
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_BUTTON1) PORT_NAME("Left mouse button") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Right mouse button") PORT_CODE(MOUSECODE_BUTTON2)

	PORT_START("mouse2")  // X-axis
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

	PORT_START("mouse3")  // Y-axis
	PORT_BIT( 0xff, 0x00, IPT_MOUSE_Y) PORT_SENSITIVITY(100) PORT_KEYDELTA(0) PORT_PLAYER(1)

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
	m_towns_vram = auto_alloc_array(machine(),UINT32,0x20000);
	m_towns_gfxvram = auto_alloc_array(machine(),UINT8,0x80000);
	m_towns_txtvram = auto_alloc_array(machine(),UINT8,0x20000);
	memset(m_towns_txtvram, 0, sizeof(UINT8)*0x20000);
	//towns_sprram = auto_alloc_array(machine(),UINT8,0x20000);
	m_towns_serial_rom = auto_alloc_array(machine(),UINT8,256/8);
	init_serial_rom();
	init_rtc();
	m_towns_rtc_timer = timer_alloc(TIMER_RTC);
	m_towns_kb_timer = timer_alloc(TIMER_KEYBOARD);
	m_towns_mouse_timer = timer_alloc(TIMER_MOUSE);
	m_towns_wait_timer = timer_alloc(TIMER_WAIT);
	m_towns_freerun_counter = timer_alloc(TIMER_FREERUN);
	m_towns_intervaltimer2 = timer_alloc(TIMER_INTERVAL2);
	m_towns_status_timer = timer_alloc(TIMER_CDSTATUS);
	m_towns_cdda_timer = timer_alloc(TIMER_CDDA);

	// CD-ROM init
	m_towns_cd.read_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(towns_state::towns_cdrom_read_byte),this), (void*)machine().device("dma_1"));

	m_maincpu->space(AS_PROGRAM).install_ram(0x100000,m_ram->size()-1,0xffffffff,0,NULL);
}

void marty_state::driver_start()
{
	towns_state::driver_start();
	if(m_towns_machine_id == 0x0101) // default if no serial ROM present
		m_towns_machine_id = 0x034a;
}

void towns_state::machine_start()
{
	m_flop0->get_device()->set_rpm(360);
	m_flop1->get_device()->set_rpm(360);
}

void towns_state::machine_reset()
{
	address_space &program = m_maincpu->space(AS_PROGRAM);
	m_messram = m_ram;
	m_cdrom = machine().device<cdrom_image_device>("cdrom");
	m_cdda = machine().device<cdda_device>("cdda");
	m_scsi = machine().device<fmscsi_device>("fmscsi");
	m_ftimer = 0x00;
	m_freerun_timer = 0x00;
	m_nmi_mask = 0x00;
	m_compat_mode = 0x00;
	m_towns_ankcg_enable = 0x00;
	m_towns_mainmem_enable = 0x00;
	m_towns_system_port = 0x00;
	m_towns_ram_enable = 0x00;
	towns_update_video_banks(program);
	m_towns_kb_status = 0x18;
	m_towns_kb_irq1_enable = 0;
	m_towns_pad_mask = 0x7f;
	m_towns_mouse_output = MOUSE_START;
	m_towns_cd.status = 0x01;  // CDROM controller ready
	m_towns_cd.buffer_ptr = -1;
	m_towns_volume_select = 0;
	m_intervaltimer2_period = 0;
	m_intervaltimer2_timeout_flag = 0;
	m_intervaltimer2_timeout_flag2 = 0;
	m_intervaltimer2_irqmask = 1;  // masked
	m_towns_rtc_timer->adjust(attotime::zero,0,attotime::from_hz(1));
	m_towns_kb_timer->adjust(attotime::zero,0,attotime::from_msec(10));
	m_towns_freerun_counter->adjust(attotime::zero,0,attotime::from_usec(1));
}

READ8_MEMBER(towns_state::get_slave_ack)
{
	if (offset==7) { // IRQ = 7
		return m_pic_slave->acknowledge();
	}
	return 0x00;
}

FLOPPY_FORMATS_MEMBER( towns_state::floppy_formats )
	FLOPPY_FMTOWNS_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( towns_floppies )
	SLOT_INTERFACE( "35hd", FLOPPY_35_HD )
SLOT_INTERFACE_END

static const gfx_layout fnt_chars_16x16 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,8,9,10,11,12,13,14,15 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	16*16
};

static const gfx_layout text_chars =
{
	8,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	8*16
};

static GFXDECODE_START( towns )
	GFXDECODE_ENTRY( "user",   0x180000 + 0x3d800, text_chars,  0, 16 )
	GFXDECODE_ENTRY( "user",   0x180000, fnt_chars_16x16,  0, 16 )
GFXDECODE_END

static MACHINE_CONFIG_FRAGMENT( towns_base )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I386, 16000000)
	MCFG_CPU_PROGRAM_MAP(towns_mem)
	MCFG_CPU_IO_MAP(towns_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", towns_state,  towns_vsync_irq)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259_master", pic8259_device, inta_cb)
	//MCFG_MACHINE_RESET_OVERRIDE(towns_state,towns)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(768,512)
	MCFG_SCREEN_VISIBLE_AREA(0, 768-1, 0, 512-1)
	MCFG_SCREEN_UPDATE_DRIVER(towns_state, screen_update)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", towns)
	MCFG_PALETTE_ADD("palette", 256)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("fm", YM3438, 16000000 / 2) // actual clock speed unknown
	MCFG_YM2612_IRQ_HANDLER(WRITELINE(towns_state, towns_fm_irq))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_RF5C68_ADD("pcm", 16000000 / 2)  // actual clock speed unknown
	MCFG_RF5C68_SAMPLE_END_CB(towns_state, towns_pcm_irq)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.50)
	MCFG_SOUND_ADD("cdda",CDDA,0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND,0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DEVICE_ADD("pit", PIT8253, 0)
	MCFG_PIT8253_CLK0(307200)
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE(towns_state, towns_pit_out0_changed))
	MCFG_PIT8253_CLK1(307200)
	MCFG_PIT8253_OUT1_HANDLER(WRITELINE(towns_state, towns_pit_out1_changed))
	MCFG_PIT8253_CLK2(307200)
	MCFG_PIT8253_OUT2_HANDLER(WRITELINE(towns_state, pit_out2_changed))

	MCFG_DEVICE_ADD("pit2", PIT8253, 0)
	MCFG_PIT8253_CLK0(307200) // reserved
	MCFG_PIT8253_CLK1(307200) // RS-232
	MCFG_PIT8253_CLK2(307200) // reserved

	MCFG_PIC8259_ADD( "pic8259_master", INPUTLINE("maincpu", 0), VCC, READ8(towns_state,get_slave_ack))

	MCFG_PIC8259_ADD( "pic8259_slave", DEVWRITELINE("pic8259_master", pic8259_device, ir7_w), GND, NULL)

	MCFG_MB8877_ADD("fdc",XTAL_8MHz/4)  // clock unknown
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(towns_state,mb8877a_irq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(towns_state,mb8877a_drq_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", towns_floppies, "35hd", towns_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", towns_floppies, "35hd", towns_state::floppy_formats)

	MCFG_CDROM_ADD("cdrom")
	MCFG_CDROM_INTERFACE("fmt_cdrom")
	MCFG_SOFTWARE_LIST_ADD("cd_list","fmtowns_cd")

	MCFG_DEVICE_ADD("scsi", SCSI_PORT, 0)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE1, "harddisk", SCSIHD, SCSI_ID_0)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE2, "harddisk", SCSIHD, SCSI_ID_1)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE3, "harddisk", SCSIHD, SCSI_ID_2)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE4, "harddisk", SCSIHD, SCSI_ID_3)
	MCFG_SCSIDEV_ADD("scsi:" SCSI_PORT_DEVICE5, "harddisk", SCSIHD, SCSI_ID_4)

	MCFG_FMSCSI_ADD("fmscsi")
	MCFG_LEGACY_SCSI_PORT("scsi")
	MCFG_FMSCSI_IRQ_HANDLER(WRITELINE(towns_state, towns_scsi_irq))
	MCFG_FMSCSI_DRQ_HANDLER(WRITELINE(towns_state, towns_scsi_drq))

	MCFG_DEVICE_ADD("dma_1", UPD71071, 0)
	MCFG_UPD71071_CPU("maincpu")
	MCFG_UPD71071_CLOCK(4000000)
	MCFG_UPD71071_DMA_READ_0_CB(READ16(towns_state, towns_fdc_dma_r))
	MCFG_UPD71071_DMA_READ_1_CB(READ16(towns_state, towns_scsi_dma_r))
	MCFG_UPD71071_DMA_READ_3_CB(READ16(towns_state, towns_cdrom_dma_r))
	MCFG_UPD71071_DMA_WRITE_0_CB(WRITE16(towns_state, towns_fdc_dma_w))
	MCFG_UPD71071_DMA_WRITE_1_CB(WRITE16(towns_state, towns_scsi_dma_w))
	MCFG_DEVICE_ADD("dma_2", UPD71071, 0)
	MCFG_UPD71071_CPU("maincpu")
	MCFG_UPD71071_CLOCK(4000000)
	MCFG_UPD71071_DMA_READ_0_CB(READ16(towns_state, towns_fdc_dma_r))
	MCFG_UPD71071_DMA_READ_1_CB(READ16(towns_state, towns_scsi_dma_r))
	MCFG_UPD71071_DMA_READ_3_CB(READ16(towns_state, towns_cdrom_dma_r))
	MCFG_UPD71071_DMA_WRITE_0_CB(WRITE16(towns_state, towns_fdc_dma_w))
	MCFG_UPD71071_DMA_WRITE_1_CB(WRITE16(towns_state, towns_scsi_dma_w))

	//MCFG_VIDEO_START_OVERRIDE(towns_state,towns)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("6M")
	MCFG_RAM_EXTRA_OPTIONS("2M,4M,8M,16M,32M,64M,96M")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( towns, towns_state )
	MCFG_FRAGMENT_ADD(towns_base)
	MCFG_NVRAM_ADD_0FILL("nvram")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( townsux, towns16_state )
	MCFG_FRAGMENT_ADD(towns_base)

	MCFG_CPU_REPLACE("maincpu",I386SX, 16000000)
	MCFG_CPU_PROGRAM_MAP(ux_mem)
	MCFG_CPU_IO_MAP(towns16_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", towns_state,  towns_vsync_irq)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259_master", pic8259_device, inta_cb)

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("2M")
	MCFG_RAM_EXTRA_OPTIONS("10M")

	MCFG_NVRAM_ADD_0FILL("nvram16")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( townssj, towns )

	MCFG_CPU_REPLACE("maincpu",PENTIUM, 66000000)
	MCFG_CPU_PROGRAM_MAP(towns_mem)
	MCFG_CPU_IO_MAP(towns_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", towns_state,  towns_vsync_irq)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259_master", pic8259_device, inta_cb)

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("8M")
	MCFG_RAM_EXTRA_OPTIONS("40M,72M")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( townshr, towns )
	MCFG_CPU_REPLACE("maincpu",I486, 20000000)
	MCFG_CPU_PROGRAM_MAP(towns_mem)
	MCFG_CPU_IO_MAP(towns_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", towns_state,  towns_vsync_irq)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259_master", pic8259_device, inta_cb)

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_EXTRA_OPTIONS("12M,20M,28M")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( townsftv, towns )
	MCFG_CPU_REPLACE("maincpu",I486, 33000000)
	MCFG_CPU_PROGRAM_MAP(towns_mem)
	MCFG_CPU_IO_MAP(towns_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", towns_state,  towns_vsync_irq)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259_master", pic8259_device, inta_cb)

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("6M")
	MCFG_RAM_EXTRA_OPTIONS("32M,68M")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( marty, marty_state )
	MCFG_FRAGMENT_ADD(towns_base)

	MCFG_CPU_REPLACE("maincpu",I386SX, 16000000)
	MCFG_CPU_PROGRAM_MAP(marty_mem)
	MCFG_CPU_IO_MAP(towns16_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", towns_state,  towns_vsync_irq)
	MCFG_CPU_IRQ_ACKNOWLEDGE_DEVICE("pic8259_master", pic8259_device, inta_cb)

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("6M")

	MCFG_NVRAM_ADD_0FILL("nvram16")
MACHINE_CONFIG_END

/* ROM definitions */
/* It is unknown exactly what model these ROM were dumped from, but it is certainly a newer model, as it won't
 * boot without the freerun timer, which was added in the FM-Towns II HG, HR, UG and later. */
ROM_START( fmtowns )
	ROM_REGION32_LE( 0x280000, "user", 0)
	ROM_LOAD("fmt_dos.rom",  0x000000, 0x080000, CRC(112872ee) SHA1(57fd146478226f7f215caf63154c763a6d52165e) )
	ROM_LOAD("fmt_f20.rom",  0x080000, 0x080000, CRC(9f55a20c) SHA1(1920711cb66340bb741a760de187de2f76040b8c) )
	ROM_LOAD("fmt_dic.rom",  0x100000, 0x080000, CRC(82d1daa2) SHA1(7564020dba71deee27184824b84dbbbb7c72aa4e) )
	ROM_LOAD("fmt_fnt.rom",  0x180000, 0x040000, CRC(dd6fd544) SHA1(a216482ea3162f348fcf77fea78e0b2e4288091a) )
	ROM_LOAD("fmt_sys.rom",  0x200000, 0x040000, CRC(afe4ebcf) SHA1(4cd51de4fca9bd7a3d91d09ad636fa6b47a41df5) )
ROM_END

/* likely an older set, as it runs without needing the freerun timer.  Font ROM appears to be corrupt, though. */
ROM_START( fmtownsa )
	ROM_REGION32_LE( 0x280000, "user", 0)
	ROM_LOAD("fmt_dos_a.rom",  0x000000, 0x080000, CRC(22270e9f) SHA1(a7e97b25ff72b14121146137db8b45d6c66af2ae) )
	ROM_LOAD("fmt_f20_a.rom",  0x080000, 0x080000, CRC(75660aac) SHA1(6a521e1d2a632c26e53b83d2cc4b0edecfc1e68c) )
	ROM_LOAD("fmt_dic_a.rom",  0x100000, 0x080000, CRC(74b1d152) SHA1(f63602a1bd67c2ad63122bfb4ffdaf483510f6a8) )
	ROM_LOAD("fmt_fnt_a.rom",  0x180000, 0x040000, CRC(0108a090) SHA1(1b5dd9d342a96b8e64070a22c3a158ca419894e1) )
	ROM_LOAD("fmt_sys_a.rom",  0x200000, 0x040000, CRC(92f3fa67) SHA1(be21404098b23465d24c4201a81c96ac01aff7ab) )
ROM_END

/* 16MHz 80386SX, 2MB RAM expandable up to 10MB (due to the limited 24-bit address space of the CPU), dumped from a UX10 */
ROM_START( fmtownsux )
	ROM_REGION32_LE( 0x480000, "user", 0)
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
	ROM_REGION32_LE( 0x480000, "user", 0)
	ROM_LOAD("mrom.m36",  0x000000, 0x080000, CRC(9c0c060c) SHA1(5721c5f9657c570638352fa9acac57fa8d0b94bd) )
	ROM_CONTINUE(0x280000,0x180000)
	ROM_LOAD("mrom.m37",  0x400000, 0x080000, CRC(fb66bb56) SHA1(e273b5fa618373bdf7536495cd53c8aac1cce9a5) )
	ROM_CONTINUE(0x80000,0x100000)
	ROM_CONTINUE(0x180000,0x40000)
	ROM_CONTINUE(0x200000,0x40000)
ROM_END

ROM_START( fmtmarty2 )
	ROM_REGION32_LE( 0x480000, "user", 0)
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
	ROM_REGION32_LE( 0x480000, "user", 0)
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

/*    YEAR  NAME    PARENT  COMPAT      MACHINE     INPUT    INIT    COMPANY      FULLNAME            FLAGS */
COMP( 1989, fmtowns,  0,        0,      towns,      towns, driver_device,    0,  "Fujitsu",   "FM-Towns",        MACHINE_NOT_WORKING)
COMP( 1989, fmtownsa, fmtowns,  0,      towns,      towns, driver_device,    0,  "Fujitsu",   "FM-Towns (alternate)", MACHINE_NOT_WORKING)
COMP( 1991, fmtownsux,fmtowns,  0,      townsux,    towns, driver_device,    0,  "Fujitsu",   "FM-Towns II UX", MACHINE_NOT_WORKING)
COMP( 1992, fmtownshr,fmtowns,  0,      townshr,    towns, driver_device,    0,  "Fujitsu",   "FM-Towns II HR", MACHINE_NOT_WORKING)
COMP( 1993, fmtownsmx,fmtowns,  0,      townshr,    towns, driver_device,    0,  "Fujitsu",   "FM-Towns II MX", MACHINE_NOT_WORKING)
COMP( 1994, fmtownsftv,fmtowns, 0,      townsftv,   towns, driver_device,    0,  "Fujitsu",   "FM-Towns II FreshTV", MACHINE_NOT_WORKING)
COMP( 19??, fmtownssj,fmtowns,  0,      townssj,    towns, driver_device,    0,  "Fujitsu",   "FM-Towns II SJ", MACHINE_NOT_WORKING)
CONS( 1993, fmtmarty, 0,        0,      marty,      marty, driver_device,    0,  "Fujitsu",   "FM-Towns Marty",  MACHINE_NOT_WORKING)
CONS( 1993, fmtmarty2,fmtmarty, 0,      marty,      marty, driver_device,    0,  "Fujitsu",   "FM-Towns Marty 2",  MACHINE_NOT_WORKING)
CONS( 1994, carmarty, fmtmarty, 0,      marty,      marty, driver_device,    0,  "Fujitsu",   "FM-Towns Car Marty",  MACHINE_NOT_WORKING)
