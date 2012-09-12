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


    Fujitsu FM-Towns Marty

    Japanese console, based on the FM-Towns computer, using an AMD 80386SX CPU,
    released in 1993


    Issues: i386 protected mode is far from complete.
            Video emulation is far from complete.

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

#include "includes/fmtowns.h"
#include "machine/scsibus.h"
#include "machine/scsihd.h"

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


static WRITE_LINE_DEVICE_HANDLER( towns_pic_irq );

INLINE UINT8 byte_to_bcd(UINT8 val)
{
	return ((val / 10) << 4) | (val % 10);
}

INLINE UINT8 bcd_to_byte(UINT8 val)
{
	return (((val & 0xf0) >> 4) * 10) + (val & 0x0f);
}

INLINE UINT32 msf_to_lbafm(UINT32 val)  // because the CDROM core doesn't provide this
{
	UINT8 m,s,f;
	f = bcd_to_byte(val & 0x0000ff);
	s = (bcd_to_byte((val & 0x00ff00) >> 8));
	m = (bcd_to_byte((val & 0xff0000) >> 16));
	return ((m * (60 * 75)) + (s * 75) + f);
}

void towns_state::init_serial_rom(running_machine &machine)
{
	// TODO: init serial ROM contents
	int x;
	static const UINT8 code[8] = { 0x04,0x65,0x54,0xA4,0x95,0x45,0x35,0x5F };
	UINT8* srom = machine.root_device().memregion("serial")->base();

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

void towns_state::init_rtc(running_machine &machine)
{
	system_time systm;

	machine.base_datetime(systm);

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
*/		case 0x06:
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
//          space->machine().device("maincpu")->execute().set_input_line(INPUT_LINE_A20,(data & 0x08) ? CLEAR_LINE : ASSERT_LINE);
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
	device_set_input_line(m_maincpu,INPUT_LINE_HALT,CLEAR_LINE);
}

READ8_MEMBER(towns_state::towns_sys6c_r)
{
	logerror("SYS: (0x6c) Timer? read\n");
	return 0x00;
}

WRITE8_MEMBER(towns_state::towns_sys6c_w)
{
	// halts the CPU for 1 microsecond
	device_set_input_line(m_maincpu,INPUT_LINE_HALT,ASSERT_LINE);
	m_towns_wait_timer->adjust(attotime::from_usec(1),0,attotime::never);
}

READ8_MEMBER(towns_state::towns_dma1_r)
{
//  logerror("DMA#1: read register %i\n",offset);
	return upd71071_r(m_dma_1,offset);
}

WRITE8_MEMBER(towns_state::towns_dma1_w)
{
//  logerror("DMA#1: wrote 0x%02x to register %i\n",data,offset);
	upd71071_w(m_dma_1,offset,data);
}

READ8_MEMBER(towns_state::towns_dma2_r)
{
	logerror("DMA#2: read register %i\n",offset);
	return upd71071_r(m_dma_2,offset);
}

WRITE8_MEMBER(towns_state::towns_dma2_w)
{
	logerror("DMA#2: wrote 0x%02x to register %i\n",data,offset);
	upd71071_w(m_dma_2,offset,data);
}

/*
 *  Floppy Disc Controller (MB8877A)
 */

WRITE_LINE_MEMBER( towns_state::mb8877a_irq_w )
{
	if(m_towns_fdc_irq6mask == 0)
		state = 0;
	pic8259_ir6_w(m_pic_master, state);  // IRQ6 = FDC
	if(IRQ_LOG) logerror("PIC: IRQ6 (FDC) set to %i\n",state);
}

WRITE_LINE_MEMBER( towns_state::mb8877a_drq_w )
{
	upd71071_dmarq(m_dma_1, state, 0);
}

READ8_MEMBER(towns_state::towns_floppy_r)
{
	device_t* fdc = m_fdc;
	device_image_interface* image;
	UINT8 ret;

	switch(offset)
	{
		case 0x00:
			return wd17xx_status_r(fdc,offset/2);
		case 0x02:
			return wd17xx_track_r(fdc,offset/2);
		case 0x04:
			return wd17xx_sector_r(fdc,offset/2);
		case 0x06:
			return wd17xx_data_r(fdc,offset/2);
		case 0x08:  // selected drive status?
			//logerror("FDC: read from offset 0x08\n");
			ret = 0x80;  // always set
			switch(m_towns_selected_drive)
			{
			case 1:
				ret |= 0x0c;
				image = dynamic_cast<device_image_interface*>(space.machine().device("floppy0"));
				if(image->exists())
					ret |= 0x03;
				break;
			case 2:
				ret |= 0x0c;
				image = dynamic_cast<device_image_interface*>(space.machine().device("floppy1"));
				if(image->exists())
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
			return 0x00;
		default:
			logerror("FDC: read from invalid or unimplemented register %02x\n",offset);
	}
	return 0xff;
}

WRITE8_MEMBER(towns_state::towns_floppy_w)
{
	device_t* fdc = m_fdc;

	switch(offset)
	{
		case 0x00:
			// Commands 0xd0 and 0xfe (Write Track) are apparently ignored?
			if(data == 0xd0)
				return;
			if(data == 0xfe)
				return;
			wd17xx_command_w(fdc,offset/2,data);
			break;
		case 0x02:
			wd17xx_track_w(fdc,offset/2,data);
			break;
		case 0x04:
			wd17xx_sector_w(fdc,offset/2,data);
			break;
		case 0x06:
			wd17xx_data_w(fdc,offset/2,data);
			break;
		case 0x08:
			// bit 5 - CLKSEL
			if(m_towns_selected_drive != 0 && m_towns_selected_drive < 2)
			{
				floppy_mon_w(floppy_get_device(space.machine(), m_towns_selected_drive-1), !BIT(data, 4));
				floppy_drive_set_ready_state(floppy_get_device(space.machine(), m_towns_selected_drive-1), data & 0x10,0);
			}
			wd17xx_set_side(fdc,(data & 0x04)>>2);
			wd17xx_dden_w(fdc, BIT(~data, 1));

			m_towns_fdc_irq6mask = data & 0x01;
			logerror("FDC: write %02x to offset 0x08\n",data);
			break;
		case 0x0c:  // drive select
			switch(data & 0x0f)
			{
				case 0x00:
					m_towns_selected_drive = 0;  // No drive selected
					break;
				case 0x01:
					m_towns_selected_drive = 1;
					wd17xx_set_drive(fdc,0);
					break;
				case 0x02:
					m_towns_selected_drive = 2;
					wd17xx_set_drive(fdc,1);
					break;
				case 0x04:
					m_towns_selected_drive = 3;
					wd17xx_set_drive(fdc,2);
					break;
				case 0x08:
					m_towns_selected_drive = 4;
					wd17xx_set_drive(fdc,3);
					break;
			}
			logerror("FDC: drive select %02x\n",data);
			break;
		default:
			logerror("FDC: write %02x to invalid or unimplemented register %02x\n",data,offset);
	}
}

static UINT16 towns_fdc_dma_r(running_machine &machine)
{
	towns_state* state = machine.driver_data<towns_state>();
	device_t* fdc = state->m_fdc;
	return wd17xx_data_r(fdc,0);
}

static void towns_fdc_dma_w(running_machine &machine, UINT16 data)
{
	towns_state* state = machine.driver_data<towns_state>();
	device_t* fdc = state->m_fdc;
	wd17xx_data_w(fdc,0,data);
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
			if(ioport("key3")->read() & 0x00080000)
				m_towns_kb_output |= 0x04;
			if(ioport("key3")->read() & 0x00040000)
				m_towns_kb_output |= 0x08;
			if(ioport("key3")->read() & 0x06400000)
				m_towns_kb_output |= 0x20;
			break;
		case 1:  // key release
			m_towns_kb_output = 0x90;
			m_towns_kb_extend = scancode & 0x7f;
			if(ioport("key3")->read() & 0x00080000)
				m_towns_kb_output |= 0x04;
			if(ioport("key3")->read() & 0x00040000)
				m_towns_kb_output |= 0x08;
			if(ioport("key3")->read() & 0x06400000)
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
		pic8259_ir1_w(m_pic_master, 1);
		if(IRQ_LOG) logerror("PIC: IRQ1 (keyboard) set high\n");
	}
	logerror("KB: sending scancode 0x%02x\n",scancode);
}

void towns_state::poll_keyboard()
{
	static const char *const kb_ports[4] = { "key1", "key2", "key3", "key4" };
	int port,bit;
	UINT8 scan;
	UINT32 portval;

	scan = 0;
	for(port=0;port<4;port++)
	{
		portval = ioport(kb_ports[port])->read();
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
			pic8259_ir1_w(m_pic_master, 0);
			if(IRQ_LOG) logerror("PIC: IRQ1 (keyboard) set low\n");
			if(m_towns_kb_extend != 0xff)
			{
				kb_sendcode(m_towns_kb_extend,2);
			}
			else
				m_towns_kb_status &= ~0x01;
			return ret;
		case 1:  // status
			logerror("KB: read status port, returning %02x\n",m_towns_kb_status);
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
	return m_towns_spkrdata & m_towns_speaker_input;
}


void towns_state::speaker_set_spkrdata(UINT8 data)
{
	m_towns_spkrdata = data ? 1 : 0;
	speaker_level_w( m_speaker, speaker_get_spk() );
}


void towns_state::speaker_set_input(UINT8 data)
{
	m_towns_speaker_input = data ? 1 : 0;
	speaker_level_w( m_speaker, speaker_get_spk() );
}

READ8_MEMBER(towns_state::towns_port60_r)
{
	UINT8 val = 0x00;

	if ( pit8253_get_output(m_pit, 0 ) )
		val |= 0x01;
	if ( pit8253_get_output(m_pit, 1 ) )
		val |= 0x02;

	val |= (m_towns_timer_mask & 0x07) << 2;

	//logerror("PIT: port 0x60 read, returning 0x%02x\n",val);
	return val;
}

WRITE8_MEMBER(towns_state::towns_port60_w)
{
	device_t* dev = m_pic_master;

	if(data & 0x80)
	{
		//towns_pic_irq(dev,0);
		m_timer0 = 0;
		pic8259_ir0_w(dev, m_timer0 || m_timer1);
	}
	m_towns_timer_mask = data & 0x07;

	speaker_set_spkrdata(data & 0x04);

	//logerror("PIT: wrote 0x%02x to port 0x60\n",data);
}

READ32_MEMBER(towns_state::towns_sys5e8_r)
{
	switch(offset)
	{
		case 0x00:
			if(ACCESSING_BITS_0_7)
			{
				logerror("SYS: read RAM size port (%i)\n",m_ram->size());
				return m_ram->size()/1048576;
			}
			break;
		case 0x01:
			if(ACCESSING_BITS_0_7)
			{
				logerror("SYS: read port 5ec\n");
				return m_compat_mode & 0x01;
			}
			break;
	}
	return 0x00;
}

WRITE32_MEMBER(towns_state::towns_sys5e8_w)
{
	switch(offset)
	{
		case 0x00:
			if(ACCESSING_BITS_0_7)
			{
				logerror("SYS: wrote 0x%02x to port 5e8\n",data);
			}
			break;
		case 0x01:
			if(ACCESSING_BITS_0_7)
			{
				logerror("SYS: wrote 0x%02x to port 5ec\n",data);
				m_compat_mode = data & 0x01;
			}
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
				pic8259_ir5_w(m_pic_slave, 0);
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

READ32_MEMBER(towns_state::towns_padport_r)
{
	UINT32 ret = 0;
	UINT32 porttype = space.machine().root_device().ioport("ctrltype")->read();
	UINT8 extra1;
	UINT8 extra2;
	UINT32 state;

	if((porttype & 0x0f) == 0x00)
		ret |= 0x000000ff;
	if((porttype & 0xf0) == 0x00)
		ret |= 0x00ff0000;
	if((porttype & 0x0f) == 0x01)
	{
		extra1 = space.machine().root_device().ioport("joy1_ex")->read();

		if(m_towns_pad_mask & 0x10)
			ret |= (space.machine().root_device().ioport("joy1")->read() & 0x3f) | 0x00000040;
		else
			ret |= (space.machine().root_device().ioport("joy1")->read() & 0x0f) | 0x00000030;

		if(extra1 & 0x01) // Run button = left+right
			ret &= ~0x0000000c;
		if(extra1 & 0x02) // Select button = up+down
			ret &= ~0x00000003;

		if((extra1 & 0x10) && (m_towns_pad_mask & 0x01))
			ret &= ~0x00000010;
		if((extra1 & 0x20) && (m_towns_pad_mask & 0x02))
			ret &= ~0x00000020;
	}
	if((porttype & 0xf0) == 0x10)
	{
		extra2 = space.machine().root_device().ioport("joy2_ex")->read();

		if(m_towns_pad_mask & 0x20)
			ret |= ((space.machine().root_device().ioport("joy2")->read() & 0x3f) << 16) | 0x00400000;
		else
			ret |= ((space.machine().root_device().ioport("joy2")->read() & 0x0f) << 16) | 0x00300000;

		if(extra2 & 0x01)
			ret &= ~0x000c0000;
		if(extra2 & 0x02)
			ret &= ~0x00030000;

		if((extra2 & 0x10) && (m_towns_pad_mask & 0x04))
			ret &= ~0x00100000;
		if((extra2 & 0x20) && (m_towns_pad_mask & 0x08))
			ret &= ~0x00200000;
	}
	if((porttype & 0x0f) == 0x04)  // 6-button joystick
	{
		extra1 = space.machine().root_device().ioport("6b_joy1_ex")->read();

		if(m_towns_pad_mask & 0x10)
			ret |= 0x0000007f;
		else
			ret |= (space.machine().root_device().ioport("6b_joy1")->read() & 0x0f) | 0x00000070;

		if(!(m_towns_pad_mask & 0x10))
		{
			if(extra1 & 0x01) // Run button = left+right
				ret &= ~0x0000000c;
			if(extra1 & 0x02) // Select button = up+down
				ret &= ~0x00000003;
			if((extra1 & 0x04) && (m_towns_pad_mask & 0x01))
				ret &= ~0x00000010;
			if((extra1 & 0x08) && (m_towns_pad_mask & 0x02))
				ret &= ~0x00000020;
		}
		if(m_towns_pad_mask & 0x10)
		{
			if(extra1 & 0x10)
				ret &= ~0x00000008;
			if(extra1 & 0x20)
				ret &= ~0x00000004;
			if(extra1 & 0x40)
				ret &= ~0x00000002;
			if(extra1 & 0x80)
				ret &= ~0x00000001;
		}
	}
	if((porttype & 0xf0) == 0x40)  // 6-button joystick
	{
		extra2 = space.machine().root_device().ioport("6b_joy2_ex")->read();

		if(m_towns_pad_mask & 0x20)
			ret |= 0x007f0000;
		else
			ret |= ((space.machine().root_device().ioport("6b_joy2")->read() & 0x0f) << 16) | 0x00700000;

		if(!(m_towns_pad_mask & 0x10))
		{
			if(extra2 & 0x01)
				ret &= ~0x000c0000;
			if(extra2 & 0x02)
				ret &= ~0x00030000;
			if((extra2 & 0x10) && (m_towns_pad_mask & 0x04))
				ret &= ~0x00100000;
			if((extra2 & 0x20) && (m_towns_pad_mask & 0x08))
				ret &= ~0x00200000;
		}
		if(m_towns_pad_mask & 0x20)
		{
			if(extra2 & 0x10)
				ret &= ~0x00080000;
			if(extra2 & 0x20)
				ret &= ~0x00040000;
			if(extra2 & 0x40)
				ret &= ~0x00020000;
			if(extra2 & 0x80)
				ret &= ~0x00010000;
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
					ret |= 0x0000000f;
		}

		// button states are always visible
		state = space.machine().root_device().ioport("mouse1")->read();
		if(!(state & 0x01))
			ret |= 0x00000010;
		if(!(state & 0x02))
			ret |= 0x00000020;
		if(m_towns_pad_mask & 0x10)
			ret |= 0x00000040;
	}
	if((porttype & 0xf0) == 0x20)  // mouse
	{
		switch(m_towns_mouse_output)
		{
			case MOUSE_X_HIGH:
				ret |= ((m_towns_mouse_x & 0xf0) << 12);
				break;
			case MOUSE_X_LOW:
				ret |= ((m_towns_mouse_x & 0x0f) << 16);
				break;
			case MOUSE_Y_HIGH:
				ret |= ((m_towns_mouse_y & 0xf0) << 12);
				break;
			case MOUSE_Y_LOW:
				ret |= ((m_towns_mouse_y & 0x0f) << 16);
				break;
			case MOUSE_START:
			case MOUSE_SYNC:
			default:
				if(m_towns_mouse_output < MOUSE_Y_LOW)
					ret |= 0x000f0000;
		}

		// button states are always visible
		state = space.machine().root_device().ioport("mouse1")->read();
		if(!(state & 0x01))
			ret |= 0x00100000;
		if(!(state & 0x02))
			ret |= 0x00200000;
		if(m_towns_pad_mask & 0x20)
			ret |= 0x00400000;
	}

	return ret;
}

WRITE32_MEMBER(towns_state::towns_pad_mask_w)
{
	UINT8 current_x,current_y;
	UINT32 type = space.machine().root_device().ioport("ctrltype")->read();

	if(ACCESSING_BITS_16_23)
	{
		m_towns_pad_mask = (data & 0x00ff0000) >> 16;
		if((type & 0x0f) == 0x02)  // mouse
		{
			if((m_towns_pad_mask & 0x10) != 0 && (m_prev_pad_mask & 0x10) == 0)
			{
				if(m_towns_mouse_output == MOUSE_START)
				{
					m_towns_mouse_output = MOUSE_X_HIGH;
					current_x = space.machine().root_device().ioport("mouse2")->read();
					current_y = space.machine().root_device().ioport("mouse3")->read();
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
					current_x = space.machine().root_device().ioport("mouse2")->read();
					current_y = space.machine().root_device().ioport("mouse3")->read();
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
					current_x = space.machine().root_device().ioport("mouse2")->read();
					current_y = space.machine().root_device().ioport("mouse3")->read();
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
					current_x = space.machine().root_device().ioport("mouse2")->read();
					current_y = space.machine().root_device().ioport("mouse3")->read();
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
}

READ8_MEMBER( towns_state::towns_cmos_low_r )
{
	if(m_towns_mainmem_enable != 0)
		return m_messram->pointer()[offset + 0xd8000];

	return m_nvram[offset];
}

WRITE8_MEMBER( towns_state::towns_cmos_low_w )
{
	if(m_towns_mainmem_enable != 0)
		m_messram->pointer()[offset+0xd8000] = data;
	else
		m_nvram[offset] = data;
}

READ8_MEMBER( towns_state::towns_cmos_r )
{
	return m_nvram[offset];
}

WRITE8_MEMBER( towns_state::towns_cmos_w )
{
	m_nvram[offset] = data;
}

void towns_state::towns_update_video_banks(address_space& space)
{
	towns_state *state = space.machine().driver_data<towns_state>();
	UINT8* ROM;

	if(m_towns_mainmem_enable != 0)  // first MB is RAM
	{
		ROM = state->memregion("user")->base();

//      state->membank(1)->set_base(m_messram->pointer()+0xc0000);
//      state->membank(2)->set_base(m_messram->pointer()+0xc8000);
//      state->membank(3)->set_base(m_messram->pointer()+0xc9000);
//      state->membank(4)->set_base(m_messram->pointer()+0xca000);
//      state->membank(5)->set_base(m_messram->pointer()+0xca000);
//      state->membank(10)->set_base(m_messram->pointer()+0xca800);
		state->membank("bank6")->set_base(m_messram->pointer()+0xcb000);
		state->membank("bank7")->set_base(m_messram->pointer()+0xcb000);
		if(m_towns_system_port & 0x02)
			state->membank("bank11")->set_base(m_messram->pointer()+0xf8000);
		else
			state->membank("bank11")->set_base(ROM+0x238000);
		state->membank("bank12")->set_base(m_messram->pointer()+0xf8000);
		return;
	}
	else  // enable I/O ports and VRAM
	{
		ROM = state->memregion("user")->base();

//      state->membank(1)->set_base(towns_gfxvram+(towns_vram_rplane*0x8000));
//      state->membank(2)->set_base(towns_txtvram);
//      state->membank(3)->set_base(state->m_messram->pointer()+0xc9000);
//      if(towns_ankcg_enable != 0)
//          state->membank(4)->set_base(ROM+0x180000+0x3d000);  // ANK CG 8x8
//      else
//          state->membank(4)->set_base(towns_txtvram+0x2000);
//      state->membank(5)->set_base(towns_txtvram+0x2000);
//      state->membank(10)->set_base(state->m_messram->pointer()+0xca800);
		if(m_towns_ankcg_enable != 0)
			state->membank("bank6")->set_base(ROM+0x180000+0x3d800);  // ANK CG 8x16
		else
			state->membank("bank6")->set_base(m_messram->pointer()+0xcb000);
		state->membank("bank7")->set_base(m_messram->pointer()+0xcb000);
		if(m_towns_system_port & 0x02)
			state->membank("bank11")->set_base(m_messram->pointer()+0xf8000);
		else
			state->membank("bank11")->set_base(ROM+0x238000);
		state->membank("bank12")->set_base(m_messram->pointer()+0xf8000);
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

WRITE32_MEMBER( towns_state::towns_video_404_w )
{
	if(ACCESSING_BITS_0_7)
	{
		m_towns_mainmem_enable = data & 0x80;
		towns_update_video_banks(space);
	}
}

READ32_MEMBER( towns_state::towns_video_404_r )
{
	if(ACCESSING_BITS_0_7)
	{
		if(m_towns_mainmem_enable != 0)
			return 0x00000080;
	}
	return 0;
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
static void towns_cdrom_set_irq(running_machine &machine,int line,int state)
{
	towns_state* tstate = machine.driver_data<towns_state>();
	switch(line)
	{
		case TOWNS_CD_IRQ_MPU:
			if(state != 0)
			{
				if(tstate->m_towns_cd.command & 0x40)
				{
//                  if(tstate->m_towns_cd.mpu_irq_enable)
					{
						tstate->m_towns_cd.status |= 0x80;
						pic8259_ir1_w(tstate->m_pic_slave, 1);
						if(IRQ_LOG) logerror("PIC: IRQ9 (CD-ROM) set high\n");
					}
				}
				else
					tstate->m_towns_cd.status |= 0x80;
			}
			else
			{
				tstate->m_towns_cd.status &= ~0x80;
				pic8259_ir1_w(tstate->m_pic_slave, 0);
				if(IRQ_LOG) logerror("PIC: IRQ9 (CD-ROM) set low\n");
			}
			break;
		case TOWNS_CD_IRQ_DMA:
			if(state != 0)
			{
				if(tstate->m_towns_cd.command & 0x40)
				{
//                  if(tstate->m_towns_cd.dma_irq_enable)
					{
						tstate->m_towns_cd.status |= 0x40;
						pic8259_ir1_w(tstate->m_pic_slave, 1);
						if(IRQ_LOG) logerror("PIC: IRQ9 (CD-ROM DMA) set high\n");
					}
				}
				else
					tstate->m_towns_cd.status |= 0x40;
			}
			else
			{
				tstate->m_towns_cd.status &= ~0x40;
				pic8259_ir1_w(tstate->m_pic_slave, 0);
				if(IRQ_LOG) logerror("PIC: IRQ9 (CD-ROM DMA) set low\n");
			}
			break;
	}
}

static TIMER_CALLBACK( towns_cd_status_ready )
{
	towns_state* state = machine.driver_data<towns_state>();
	state->m_towns_cd.status |= 0x02;  // status read request
	state->m_towns_cd.status |= 0x01;  // ready
	state->m_towns_cd.cmd_status_ptr = 0;
	towns_cdrom_set_irq((running_machine&)machine,TOWNS_CD_IRQ_MPU,1);
}

static void towns_cd_set_status(running_machine &machine, UINT8 st0, UINT8 st1, UINT8 st2, UINT8 st3)
{
	towns_state* state = machine.driver_data<towns_state>();
	state->m_towns_cd.cmd_status[0] = st0;
	state->m_towns_cd.cmd_status[1] = st1;
	state->m_towns_cd.cmd_status[2] = st2;
	state->m_towns_cd.cmd_status[3] = st3;
	// wait a bit
	machine.scheduler().timer_set(attotime::from_msec(1), FUNC(towns_cd_status_ready), 0, &machine);
}

static UINT8 towns_cd_get_track(running_machine &machine)
{
	towns_state* state = machine.driver_data<towns_state>();
	cdrom_image_device* cdrom = state->m_cdrom;
	device_t* cdda = state->m_cdda;
	UINT32 lba = cdda_get_audio_lba(cdda);
	UINT8 track;

	for(track=1;track<99;track++)
	{
		if(cdrom_get_track_start(cdrom->get_cdrom_file(),track) > lba)
			break;
	}
	return track;
}

static TIMER_CALLBACK( towns_cdrom_read_byte )
{
	device_t* device = (device_t* )ptr;
	towns_state* state = machine.driver_data<towns_state>();
	int masked;
	// TODO: support software transfers, for now DMA is assumed.

	if(state->m_towns_cd.buffer_ptr < 0) // transfer has ended
		return;

	masked = upd71071_dmarq(device,param,3);  // CD-ROM controller uses DMA1 channel 3
//  logerror("DMARQ: param=%i ret=%i bufferptr=%i\n",param,masked,state->m_towns_cd.buffer_ptr);
	if(param != 0)
	{
		state->m_towns_cd.read_timer->adjust(attotime::from_hz(300000));
	}
	else
	{
		if(masked != 0)  // check if the DMA channel is masked
		{
			state->m_towns_cd.read_timer->adjust(attotime::from_hz(300000),1);
			return;
		}
		if(state->m_towns_cd.buffer_ptr < 2048)
			state->m_towns_cd.read_timer->adjust(attotime::from_hz(300000),1);
		else
		{  // end of transfer
			state->m_towns_cd.status &= ~0x10;  // no longer transferring by DMA
			state->m_towns_cd.status &= ~0x20;  // no longer transferring by software
			logerror("DMA1: end of transfer (LBA=%08x)\n",state->m_towns_cd.lba_current);
			if(state->m_towns_cd.lba_current >= state->m_towns_cd.lba_last)
			{
				state->m_towns_cd.extra_status = 0;
				towns_cd_set_status(device->machine(),0x06,0x00,0x00,0x00);
				towns_cdrom_set_irq(device->machine(),TOWNS_CD_IRQ_DMA,1);
				state->m_towns_cd.buffer_ptr = -1;
				state->m_towns_cd.status |= 0x01;  // ready
			}
			else
			{
				state->m_towns_cd.extra_status = 0;
				towns_cd_set_status(device->machine(),0x22,0x00,0x00,0x00);
				towns_cdrom_set_irq(device->machine(),TOWNS_CD_IRQ_DMA,1);
				cdrom_read_data(state->m_cdrom->get_cdrom_file(),++state->m_towns_cd.lba_current,state->m_towns_cd.buffer,CD_TRACK_MODE1);
				state->m_towns_cd.read_timer->adjust(attotime::from_hz(300000),1);
				state->m_towns_cd.buffer_ptr = -1;
			}
		}
	}
}

static void towns_cdrom_read(cdrom_image_device* device)
{
	// MODE 1 read
	// load data into buffer to be sent via DMA1 channel 3
	// A set of status bytes is sent after each sector, and DMA is paused
	// so that the DMA controller than be set up again.
	// parameters:
	//          3 bytes: MSF of first sector to read
	//          3 bytes: MSF of last sector to read
	towns_state* state = device->machine().driver_data<towns_state>();
	UINT32 lba1,lba2,track;

	lba1 = state->m_towns_cd.parameter[7] << 16;
	lba1 += state->m_towns_cd.parameter[6] << 8;
	lba1 += state->m_towns_cd.parameter[5];
	lba2 = state->m_towns_cd.parameter[4] << 16;
	lba2 += state->m_towns_cd.parameter[3] << 8;
	lba2 += state->m_towns_cd.parameter[2];
	state->m_towns_cd.lba_current = msf_to_lbafm(lba1);
	state->m_towns_cd.lba_last = msf_to_lbafm(lba2);

	// first track starts at 00:02:00 - this is hardcoded in the boot procedure
	track = cdrom_get_track(device->get_cdrom_file(),state->m_towns_cd.lba_current);
	if(track < 2)
	{  // recalculate LBA
		state->m_towns_cd.lba_current -= 150;
		state->m_towns_cd.lba_last -= 150;
	}

	// parameter 7 = sector count?
	if(state->m_towns_cd.parameter[1] != 0)
		state->m_towns_cd.lba_last += state->m_towns_cd.parameter[1];

	logerror("CD: Mode 1 read from LBA next:%i last:%i track:%i\n",state->m_towns_cd.lba_current,state->m_towns_cd.lba_last,track);

	if(state->m_towns_cd.lba_current > state->m_towns_cd.lba_last)
	{
		state->m_towns_cd.extra_status = 0;
		towns_cd_set_status(device->machine(),0x01,0x00,0x00,0x00);
	}
	else
	{
		cdrom_read_data(device->get_cdrom_file(),state->m_towns_cd.lba_current,state->m_towns_cd.buffer,CD_TRACK_MODE1);
		state->m_towns_cd.status |= 0x10;  // DMA transfer begin
		state->m_towns_cd.status &= ~0x20;  // not a software transfer
//      state->m_towns_cd.buffer_ptr = 0;
//      state->m_towns_cd.read_timer->adjust(attotime::from_hz(300000),1);
		if(state->m_towns_cd.command & 0x20)
		{
			state->m_towns_cd.extra_status = 2;
			towns_cd_set_status(device->machine(),0x00,0x00,0x00,0x00);
		}
		else
		{
			state->m_towns_cd.extra_status = 0;
			towns_cd_set_status(device->machine(),0x22,0x00,0x00,0x00);
		}
	}
}

static void towns_cdrom_play_cdda(cdrom_image_device* device)
{
	// PLAY AUDIO
	// Plays CD-DA audio from the specified MSF
	// Parameters:
	//          3 bytes: starting MSF of audio to play
	//          3 bytes: ending MSF of audio to play (can span multiple tracks)
	towns_state* state = device->machine().driver_data<towns_state>();
	UINT32 lba1,lba2;
	device_t* cdda = state->m_cdda;

	lba1 = state->m_towns_cd.parameter[7] << 16;
	lba1 += state->m_towns_cd.parameter[6] << 8;
	lba1 += state->m_towns_cd.parameter[5];
	lba2 = state->m_towns_cd.parameter[4] << 16;
	lba2 += state->m_towns_cd.parameter[3] << 8;
	lba2 += state->m_towns_cd.parameter[2];
	state->m_towns_cd.cdda_current = msf_to_lbafm(lba1);
	state->m_towns_cd.cdda_length = msf_to_lbafm(lba2) - state->m_towns_cd.cdda_current;

	cdda_set_cdrom(cdda,device->get_cdrom_file());
	cdda_start_audio(cdda,state->m_towns_cd.cdda_current,state->m_towns_cd.cdda_length);
	logerror("CD: CD-DA start from LBA:%i length:%i\n",state->m_towns_cd.cdda_current,state->m_towns_cd.cdda_length);
	if(state->m_towns_cd.command & 0x20)
	{
		state->m_towns_cd.extra_status = 1;
		towns_cd_set_status(device->machine(),0x00,0x03,0x00,0x00);
	}
}

static TIMER_CALLBACK(towns_delay_cdda)
{
	towns_cdrom_play_cdda((cdrom_image_device*)ptr);
}

static void towns_cdrom_execute_command(cdrom_image_device* device)
{
	towns_state* state = device->machine().driver_data<towns_state>();

	if(device->get_cdrom_file() == NULL)
	{  // No CD in drive
		if(state->m_towns_cd.command & 0x20)
		{
			state->m_towns_cd.extra_status = 0;
			towns_cd_set_status(device->machine(),0x10,0x00,0x00,0x00);
		}
	}
	else
	{
		state->m_towns_cd.status &= ~0x02;
		switch(state->m_towns_cd.command & 0x9f)
		{
			case 0x00:  // Seek
				if(state->m_towns_cd.command & 0x20)
				{
					state->m_towns_cd.extra_status = 1;
					towns_cd_set_status(device->machine(),0x00,0x00,0x00,0x00);
				}
				logerror("CD: Command 0x00: SEEK\n");
				break;
			case 0x01:  // unknown
				if(state->m_towns_cd.command & 0x20)
				{
					state->m_towns_cd.extra_status = 0;
					towns_cd_set_status(device->machine(),0x00,0xff,0xff,0xff);
				}
				logerror("CD: Command 0x01: unknown\n");
				break;
			case 0x02:  // Read (MODE1)
				logerror("CD: Command 0x02: READ MODE1\n");
				towns_cdrom_read(device);
				break;
			case 0x04:  // Play Audio Track
				logerror("CD: Command 0x04: PLAY CD-DA\n");
				device->machine().scheduler().timer_set(attotime::from_msec(1), FUNC(towns_delay_cdda), 0, device);
				break;
			case 0x05:  // Read TOC
				logerror("CD: Command 0x05: READ TOC\n");
				state->m_towns_cd.extra_status = 1;
				towns_cd_set_status(device->machine(),0x00,0x00,0x00,0x00);
				break;
			case 0x06:  // Read CD-DA state?
				logerror("CD: Command 0x06: READ CD-DA STATE\n");
				state->m_towns_cd.extra_status = 1;
				towns_cd_set_status(device->machine(),0x00,0x00,0x00,0x00);
				break;
			case 0x80:  // set state
				logerror("CD: Command 0x80: set state\n");
				if(state->m_towns_cd.command & 0x20)
				{
					state->m_towns_cd.extra_status = 0;
					if(cdda_audio_active(state->m_cdda) && !cdda_audio_paused(state->m_cdda))
						towns_cd_set_status(device->machine(),0x00,0x03,0x00,0x00);
					else
						towns_cd_set_status(device->machine(),0x00,0x01,0x00,0x00);

				}
				break;
			case 0x81:  // set state (CDDASET)
				if(state->m_towns_cd.command & 0x20)
				{
					state->m_towns_cd.extra_status = 0;
					towns_cd_set_status(device->machine(),0x00,0x00,0x00,0x00);
				}
				logerror("CD: Command 0x81: set state (CDDASET)\n");
				break;
			case 0x84:   // Stop CD audio track  -- generates no status output?
				if(state->m_towns_cd.command & 0x20)
				{
					state->m_towns_cd.extra_status = 1;
					towns_cd_set_status(device->machine(),0x00,0x00,0x00,0x00);
				}
				cdda_pause_audio(state->m_cdda,1);
				logerror("CD: Command 0x84: STOP CD-DA\n");
				break;
			case 0x85:   // Stop CD audio track (difference from 0x84?)
				if(state->m_towns_cd.command & 0x20)
				{
					state->m_towns_cd.extra_status = 1;
					towns_cd_set_status(device->machine(),0x00,0x00,0x00,0x00);
				}
				cdda_pause_audio(state->m_cdda,1);
				logerror("CD: Command 0x85: STOP CD-DA\n");
				break;
			case 0x87:  // Resume CD-DA playback
				if(state->m_towns_cd.command & 0x20)
				{
					state->m_towns_cd.extra_status = 1;
					towns_cd_set_status(device->machine(),0x00,0x03,0x00,0x00);
				}
				cdda_pause_audio(state->m_cdda,0);
				logerror("CD: Command 0x87: RESUME CD-DA\n");
				break;
			default:
				state->m_towns_cd.extra_status = 0;
				towns_cd_set_status(device->machine(),0x10,0x00,0x00,0x00);
				logerror("CD: Unknown or unimplemented command %02x\n",state->m_towns_cd.command);
		}
	}
}

static UINT16 towns_cdrom_dma_r(running_machine &machine)
{
	towns_state* state = machine.driver_data<towns_state>();
	if(state->m_towns_cd.buffer_ptr >= 2048)
		return 0x00;
	return state->m_towns_cd.buffer[state->m_towns_cd.buffer_ptr++];
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
							towns_cd_set_status(space.machine(),0x04,0x00,0x00,0x00);
							m_towns_cd.extra_status = 0;
							break;
						case 0x02:  // read
							if(m_towns_cd.extra_status == 2)
								towns_cd_set_status(space.machine(),0x22,0x00,0x00,0x00);
							m_towns_cd.extra_status = 0;
							break;
						case 0x04:  // play cdda
							towns_cd_set_status(space.machine(),0x07,0x00,0x00,0x00);
							m_towns_cd.extra_status = 0;
							break;
						case 0x05:  // read toc
							switch(m_towns_cd.extra_status)
							{
								case 1:
								case 3:
									towns_cd_set_status(space.machine(),0x16,0x00,0x00,0x00);
									m_towns_cd.extra_status++;
									break;
								case 2: // st1 = first track number (BCD)
									towns_cd_set_status(space.machine(),0x17,0x01,0x00,0x00);
									m_towns_cd.extra_status++;
									break;
								case 4: // st1 = last track number (BCD)
									towns_cd_set_status(space.machine(),0x17,
										byte_to_bcd(cdrom_get_last_track(m_cdrom->get_cdrom_file())),
										0x00,0x00);
									m_towns_cd.extra_status++;
									break;
								case 5:  // st1 = control/adr of track 0xaa?
									towns_cd_set_status(space.machine(),0x16,
										cdrom_get_adr_control(m_cdrom->get_cdrom_file(),0xaa),
										0xaa,0x00);
									m_towns_cd.extra_status++;
									break;
								case 6:  // st1/2/3 = address of track 0xaa? (BCD)
									addr = cdrom_get_track_start(m_cdrom->get_cdrom_file(),0xaa);
									addr = lba_to_msf(addr);
									towns_cd_set_status(space.machine(),0x17,
										(addr & 0xff0000) >> 16,(addr & 0x00ff00) >> 8,addr & 0x0000ff);
									m_towns_cd.extra_status++;
									break;
								default:  // same as case 5 and 6, but for each individual track
									if(m_towns_cd.extra_status & 0x01)
									{
										towns_cd_set_status(space.machine(),0x16,
											((cdrom_get_adr_control(m_cdrom->get_cdrom_file(),(m_towns_cd.extra_status/2)-3) & 0x0f) << 4)
											| ((cdrom_get_adr_control(m_cdrom->get_cdrom_file(),(m_towns_cd.extra_status/2)-3) & 0xf0) >> 4),
											(m_towns_cd.extra_status/2)-3,0x00);
										m_towns_cd.extra_status++;
									}
									else
									{
										addr = cdrom_get_track_start(m_cdrom->get_cdrom_file(),(m_towns_cd.extra_status/2)-4);
										addr = lba_to_msf(addr);
										towns_cd_set_status(space.machine(),0x17,
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
									towns_cd_set_status(space.machine(),0x18,
										0x00,towns_cd_get_track(space.machine()),0x00);
									m_towns_cd.extra_status++;
									break;
								case 2:  // st0/1/2 = MSF from beginning of current track
									addr = cdda_get_audio_lba(m_cdda);
									addr = lba_to_msf(addr - m_towns_cd.cdda_current);
									towns_cd_set_status(space.machine(),0x19,
										(addr & 0xff0000) >> 16,(addr & 0x00ff00) >> 8,addr & 0x0000ff);
									m_towns_cd.extra_status++;
									break;
								case 3:  // st1/2 = current MSF
									addr = cdda_get_audio_lba(m_cdda);
									addr = lba_to_msf(addr);  // this data is incorrect, but will do until exact meaning is found
									towns_cd_set_status(space.machine(),0x19,
										0x00,(addr & 0xff0000) >> 16,(addr & 0x00ff00) >> 8);
									m_towns_cd.extra_status++;
									break;
								case 4:
									addr = cdda_get_audio_lba(m_cdda);
									addr = lba_to_msf(addr);  // this data is incorrect, but will do until exact meaning is found
									towns_cd_set_status(space.machine(),0x20,
										addr & 0x0000ff,0x00,0x00);
									m_towns_cd.extra_status = 0;
									break;
							}
							break;
						case 0x84:
							towns_cd_set_status(space.machine(),0x11,0x00,0x00,0x00);
							m_towns_cd.extra_status = 0;
							break;
						case 0x85:
							towns_cd_set_status(space.machine(),0x12,0x00,0x00,0x00);
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
				towns_cdrom_set_irq(space.machine(),TOWNS_CD_IRQ_MPU,0);
			if(data & 0x40)
				towns_cdrom_set_irq(space.machine(),TOWNS_CD_IRQ_DMA,0);
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
			// TODO: software transfer mode (bit 3)
			if(data & 0x10)
			{
				m_towns_cd.status |= 0x10;  // DMA transfer begin
				m_towns_cd.status &= ~0x20;  // not a software transfer
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
READ32_MEMBER(towns_state::towns_rtc_r)
{
	if(ACCESSING_BITS_0_7)
		return 0x80 | m_towns_rtc_reg[m_towns_rtc_select];

	return 0x00;
}

WRITE32_MEMBER(towns_state::towns_rtc_w)
{
	if(ACCESSING_BITS_0_7)
		m_towns_rtc_data = data;
}

WRITE32_MEMBER(towns_state::towns_rtc_select_w)
{
	if(ACCESSING_BITS_0_7)
	{
		if(data & 0x80)
		{
			if(data & 0x01)
				m_towns_rtc_select = m_towns_rtc_data & 0x0f;
		}
	}
}

static void rtc_hour(running_machine &machine)
{
	towns_state* state = machine.driver_data<towns_state>();

	state->m_towns_rtc_reg[4]++;
	if(state->m_towns_rtc_reg[4] > 4 && state->m_towns_rtc_reg[5] == 2)
	{
		state->m_towns_rtc_reg[4] = 0;
		state->m_towns_rtc_reg[5] = 0;
	}
	else if(state->m_towns_rtc_reg[4] > 9)
	{
		state->m_towns_rtc_reg[4] = 0;
		state->m_towns_rtc_reg[5]++;
	}
}

static void rtc_minute(running_machine &machine)
{
	towns_state* state = machine.driver_data<towns_state>();

	state->m_towns_rtc_reg[2]++;
	if(state->m_towns_rtc_reg[2] > 9)
	{
		state->m_towns_rtc_reg[2] = 0;
		state->m_towns_rtc_reg[3]++;
		if(state->m_towns_rtc_reg[3] > 5)
		{
			state->m_towns_rtc_reg[3] = 0;
			rtc_hour(machine);
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
			rtc_minute(machine());
		}
	}
}

// SCSI controller - I/O ports 0xc30 and 0xc32
static UINT16 towns_scsi_dma_r(running_machine &machine)
{
	towns_state* state = machine.driver_data<towns_state>();
	return state->m_scsi->fmscsi_data_r();
}

static void towns_scsi_dma_w(running_machine &machine, UINT16 data)
{
	towns_state* state = machine.driver_data<towns_state>();
	state->m_scsi->fmscsi_data_w(data & 0xff);
}

static WRITE_LINE_DEVICE_HANDLER( towns_scsi_irq )
{
	towns_state* tstate = device->machine().driver_data<towns_state>();
	pic8259_ir0_w(tstate->m_pic_slave, state);
	if(IRQ_LOG)
		logerror("PIC: IRQ8 (SCSI) set to %i\n",state);
}

static WRITE_LINE_DEVICE_HANDLER( towns_scsi_drq )
{
	towns_state* tstate = device->machine().driver_data<towns_state>();
	upd71071_dmarq(tstate->m_dma_1,state,1);  // SCSI HDs use channel 1
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
	return 0;
}

WRITE8_MEMBER(towns_state::towns_volume_w)
{
	switch(offset)
	{
	case 2:
		m_towns_volume[m_towns_volume_select] = data;
		if(m_towns_volume_select == 4)
			cdda_set_channel_volume(m_cdda,0,100.0 * (data / 64.0f));
		if(m_towns_volume_select == 5)
			cdda_set_channel_volume(m_cdda,1,100.0 * (data / 64.0f));
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

static IRQ_CALLBACK( towns_irq_callback )
{
	towns_state* state = device->machine().driver_data<towns_state>();
	return pic8259_acknowledge(state->m_pic_master);
}

// YM3438 interrupt (IRQ 13)
static void towns_fm_irq(device_t* device, int irq)
{
	towns_state* state = device->machine().driver_data<towns_state>();
	device_t* pic = state->m_pic_slave;
	if(irq)
	{
		state->m_towns_fm_irq_flag = 1;
		pic8259_ir5_w(pic, 1);
		if(IRQ_LOG) logerror("PIC: IRQ13 (FM) set high\n");
	}
	else
	{
		state->m_towns_fm_irq_flag = 0;
		if(state->m_towns_pcm_irq_flag == 0)
		{
			pic8259_ir5_w(pic, 0);
			if(IRQ_LOG) logerror("PIC: IRQ13 (FM) set low\n");
		}
	}
}

// PCM interrupt (IRQ 13)
static void towns_pcm_irq(device_t* device, int channel)
{
	towns_state* state = device->machine().driver_data<towns_state>();
	device_t* pic = state->m_pic_slave;

	if(state->m_towns_pcm_channel_mask & (1 << channel))
	{
		state->m_towns_pcm_irq_flag = 1;
		state->m_towns_pcm_channel_flag |= (1 << channel);
		pic8259_ir5_w(pic, 1);
		if(IRQ_LOG) logerror("PIC: IRQ13 (PCM) set high (channel %i)\n",channel);
	}
}

static WRITE_LINE_DEVICE_HANDLER( towns_pic_irq )
{
	device->machine().device("maincpu")->execute().set_input_line(0, state ? HOLD_LINE : CLEAR_LINE);
//  logerror("PIC#1: set IRQ line to %i\n",interrupt);
}

static WRITE_LINE_DEVICE_HANDLER( towns_pit_out0_changed )
{
	towns_state* tstate = device->machine().driver_data<towns_state>();
	device_t* dev = tstate->m_pic_master;

	if(tstate->m_towns_timer_mask & 0x01)
	{
		tstate->m_timer0 = state;
		if(IRQ_LOG) logerror("PIC: IRQ0 (PIT Timer ch0) set to %i\n",state);
	}
	else
		tstate->m_timer0 = 0;

	pic8259_ir0_w(dev, tstate->m_timer0 || tstate->m_timer1);
}

static WRITE_LINE_DEVICE_HANDLER( towns_pit_out1_changed )
{
	towns_state* tstate = device->machine().driver_data<towns_state>();
	device_t* dev = tstate->m_pic_master;

	if(tstate->m_towns_timer_mask & 0x02)
	{
		tstate->m_timer1 = state;
		if(IRQ_LOG) logerror("PIC: IRQ0 (PIT Timer ch1) set to %i\n",state);
	}
	else
		tstate->m_timer1 = 0;

	pic8259_ir0_w(dev, tstate->m_timer0 || tstate->m_timer1);
}

WRITE_LINE_MEMBER( towns_state::pit_out2_changed )
{
	speaker_set_input(state);
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
  AM_RANGE(0xc2200000, 0xc220ffff) AM_DEVREADWRITE8_LEGACY("pcm",rf5c68_mem_r,rf5c68_mem_w,0xffffffff)  // WAVE RAM
  AM_RANGE(0xfffc0000, 0xffffffff) AM_ROM AM_REGION("user",0x200000)  // SYSTEM ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(marty_mem, AS_PROGRAM, 32, towns_state)
  ADDRESS_MAP_GLOBAL_MASK(0x00ffffff)  // 386SX has only a 24-bit address range
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
//  AM_RANGE(0x00100000, 0x005fffff) AM_RAM  // some extra RAM - the Marty has 6MB RAM (not upgradable)
  AM_RANGE(0x00600000, 0x0067ffff) AM_ROM AM_REGION("user",0x000000)  // OS
  AM_RANGE(0x00680000, 0x0087ffff) AM_ROM AM_REGION("user",0x280000)  // EX ROM
  AM_RANGE(0x00a00000, 0x00a7ffff) AM_READWRITE8(towns_gfx_high_r,towns_gfx_high_w,0xffffffff) AM_MIRROR(0x180000) // VRAM
  AM_RANGE(0x00b00000, 0x00b7ffff) AM_ROM AM_REGION("user",0x180000)  // FONT
  AM_RANGE(0x00c00000, 0x00c1ffff) AM_READWRITE8(towns_spriteram_r,towns_spriteram_w,0xffffffff) // Sprite RAM
  AM_RANGE(0x00d00000, 0x00dfffff) AM_RAM // IC Memory Card (is this usable on the Marty?)
  AM_RANGE(0x00e80000, 0x00efffff) AM_ROM AM_REGION("user",0x100000)  // DIC ROM
  AM_RANGE(0x00f00000, 0x00f7ffff) AM_ROM AM_REGION("user",0x180000)  // FONT
  AM_RANGE(0x00f80000, 0x00f8ffff) AM_DEVREADWRITE8_LEGACY("pcm",rf5c68_mem_r,rf5c68_mem_w,0xffffffff)  // WAVE RAM
  AM_RANGE(0x00fc0000, 0x00ffffff) AM_ROM AM_REGION("user",0x200000)  // SYSTEM ROM
  AM_RANGE(0xfffc0000, 0xffffffff) AM_ROM AM_REGION("user",0x200000)  // SYSTEM ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START(ux_mem, AS_PROGRAM, 32, towns_state)
  ADDRESS_MAP_GLOBAL_MASK(0x00ffffff)  // 386SX has only a 24-bit address range
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
//  AM_RANGE(0x00100000, 0x005fffff) AM_RAM  // some extra RAM - the Marty has 6MB RAM (not upgradable)
//  AM_RANGE(0x00680000, 0x0087ffff) AM_ROM AM_REGION("user",0x280000)  // EX ROM
  AM_RANGE(0x00a00000, 0x00a7ffff) AM_READWRITE8(towns_gfx_high_r,towns_gfx_high_w,0xffffffff) AM_MIRROR(0x180000) // VRAM
  AM_RANGE(0x00b00000, 0x00b7ffff) AM_ROM AM_REGION("user",0x180000)  // FONT
  AM_RANGE(0x00c00000, 0x00c1ffff) AM_READWRITE8(towns_spriteram_r,towns_spriteram_w,0xffffffff) // Sprite RAM
  AM_RANGE(0x00d00000, 0x00dfffff) AM_RAM // IC Memory Card
  AM_RANGE(0x00e00000, 0x00e7ffff) AM_ROM AM_REGION("user",0x000000)  // OS
  AM_RANGE(0x00e80000, 0x00efffff) AM_ROM AM_REGION("user",0x100000)  // DIC ROM
  AM_RANGE(0x00f00000, 0x00f7ffff) AM_ROM AM_REGION("user",0x180000)  // FONT
  AM_RANGE(0x00f80000, 0x00f8ffff) AM_DEVREADWRITE8_LEGACY("pcm",rf5c68_mem_r,rf5c68_mem_w,0xffffffff)  // WAVE RAM
  AM_RANGE(0x00fc0000, 0x00ffffff) AM_ROM AM_REGION("user",0x200000)  // SYSTEM ROM
  AM_RANGE(0xfffc0000, 0xffffffff) AM_ROM AM_REGION("user",0x200000)  // SYSTEM ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( towns_io , AS_IO, 32, towns_state)
  // I/O ports derived from FM Towns/Bochs, these are specific to the FM Towns
  // System ports
  ADDRESS_MAP_UNMAP_HIGH
  AM_RANGE(0x0000,0x0003) AM_DEVREADWRITE8_LEGACY("pic8259_master", pic8259_r, pic8259_w, 0x00ff00ff)
  AM_RANGE(0x0010,0x0013) AM_DEVREADWRITE8_LEGACY("pic8259_slave", pic8259_r, pic8259_w, 0x00ff00ff)
  AM_RANGE(0x0020,0x0033) AM_READWRITE8(towns_system_r,towns_system_w, 0xffffffff)
  AM_RANGE(0x0040,0x0047) AM_DEVREADWRITE8_LEGACY("pit",pit8253_r, pit8253_w, 0x00ff00ff)
  AM_RANGE(0x0050,0x0057) AM_DEVREADWRITE8_LEGACY("pit2",pit8253_r, pit8253_w, 0x00ff00ff)
  AM_RANGE(0x0060,0x0063) AM_READWRITE8(towns_port60_r, towns_port60_w, 0x000000ff)
  AM_RANGE(0x0068,0x006b) AM_READWRITE8(towns_intervaltimer2_r, towns_intervaltimer2_w, 0xffffffff)
  AM_RANGE(0x006c,0x006f) AM_READWRITE8(towns_sys6c_r,towns_sys6c_w, 0x000000ff)
  // 0x0070/0x0080 - CMOS RTC
  AM_RANGE(0x0070,0x0073) AM_READWRITE(towns_rtc_r,towns_rtc_w)
  AM_RANGE(0x0080,0x0083) AM_WRITE(towns_rtc_select_w)
  // DMA controllers (uPD71071)
  AM_RANGE(0x00a0,0x00af) AM_READWRITE8(towns_dma1_r, towns_dma1_w, 0xffffffff)
  AM_RANGE(0x00b0,0x00bf) AM_READWRITE8(towns_dma2_r, towns_dma2_w, 0xffffffff)
  // Floppy controller
  AM_RANGE(0x0200,0x020f) AM_READWRITE8(towns_floppy_r, towns_floppy_w, 0xffffffff)
  // CRTC / Video
  AM_RANGE(0x0400,0x0403) AM_READ(towns_video_unknown_r)  // R/O (0x400)
  AM_RANGE(0x0404,0x0407) AM_READWRITE(towns_video_404_r, towns_video_404_w)  // R/W (0x404)
  AM_RANGE(0x0440,0x045f) AM_READWRITE8(towns_video_440_r, towns_video_440_w, 0xffffffff)
  // System port
  AM_RANGE(0x0480,0x0483) AM_READWRITE8(towns_sys480_r,towns_sys480_w,0x000000ff)  // R/W (0x480)
  // CD-ROM
  AM_RANGE(0x04c0,0x04cf) AM_READWRITE8(towns_cdrom_r,towns_cdrom_w,0x00ff00ff)
  // Joystick / Mouse ports
  AM_RANGE(0x04d0,0x04d3) AM_READ(towns_padport_r)
  AM_RANGE(0x04d4,0x04d7) AM_WRITE(towns_pad_mask_w)
  // Sound (YM3438 [FM], RF5c68 [PCM])
  AM_RANGE(0x04d8,0x04df) AM_DEVREADWRITE8_LEGACY("fm",ym3438_r,ym3438_w,0x00ff00ff)
  AM_RANGE(0x04e0,0x04e3) AM_READWRITE8(towns_volume_r,towns_volume_w,0xffffffff)  // R/W  -- volume ports
  AM_RANGE(0x04e8,0x04ef) AM_READWRITE8(towns_sound_ctrl_r,towns_sound_ctrl_w,0xffffffff)
  AM_RANGE(0x04f0,0x04fb) AM_DEVWRITE8_LEGACY("pcm",rf5c68_w,0xffffffff)
  // CRTC / Video
  AM_RANGE(0x05c8,0x05cb) AM_READWRITE8(towns_video_5c8_r, towns_video_5c8_w, 0xffffffff)
  // System ports
  AM_RANGE(0x05e8,0x05ef) AM_READWRITE(towns_sys5e8_r, towns_sys5e8_w)
  // Keyboard (8042 MCU)
  AM_RANGE(0x0600,0x0607) AM_READWRITE8(towns_keyboard_r, towns_keyboard_w,0x00ff00ff)
  // SCSI controller
  AM_RANGE(0x0c30,0x0c37) AM_DEVREADWRITE8("scsi:fm",fmscsi_device,fmscsi_r,fmscsi_w,0x00ff00ff)
  // CMOS
  AM_RANGE(0x3000,0x4fff) AM_READWRITE8(towns_cmos_r, towns_cmos_w,0x00ff00ff)
  // Something (MS-DOS wants this 0x41ff to be 1)
  //AM_RANGE(0x41fc,0x41ff) AM_READ8(towns_41ff_r,0xff000000)
  // CRTC / Video (again)
  AM_RANGE(0xfd90,0xfda3) AM_READWRITE8(towns_video_fd90_r, towns_video_fd90_w, 0xffffffff)
  AM_RANGE(0xff80,0xffff) AM_READWRITE8(towns_video_cff80_r,towns_video_cff80_w,0xffffffff)

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
	m_pic_master = machine().device("pic8259_master");
	m_pic_slave = machine().device("pic8259_slave");
	m_towns_vram = auto_alloc_array(machine(),UINT32,0x20000);
	m_towns_gfxvram = auto_alloc_array(machine(),UINT8,0x80000);
	m_towns_txtvram = auto_alloc_array(machine(),UINT8,0x20000);
	//towns_sprram = auto_alloc_array(machine(),UINT8,0x20000);
	m_towns_serial_rom = auto_alloc_array(machine(),UINT8,256/8);
	init_serial_rom(machine());
	init_rtc(machine());
	m_towns_rtc_timer = timer_alloc(TIMER_RTC);
	m_towns_kb_timer = timer_alloc(TIMER_KEYBOARD);
	m_towns_mouse_timer = timer_alloc(TIMER_MOUSE);
	m_towns_wait_timer = timer_alloc(TIMER_WAIT);
	m_towns_freerun_counter = timer_alloc(TIMER_FREERUN);
	m_towns_intervaltimer2 = timer_alloc(TIMER_INTERVAL2);

	// CD-ROM init
	m_towns_cd.read_timer = machine().scheduler().timer_alloc(FUNC(towns_cdrom_read_byte), (void*)machine().device("dma_1"));

	device_set_irq_callback(machine().device("maincpu"), towns_irq_callback);
	machine().device("maincpu")->memory().space(AS_PROGRAM)->install_ram(0x100000,machine().device<ram_device>(RAM_TAG)->size()-1,0xffffffff,0,NULL);

}

void marty_state::driver_start()
{
	towns_state::driver_start();
	if(m_towns_machine_id == 0x0101) // default if no serial ROM present
		m_towns_machine_id = 0x034a;
}

void towns_state::machine_reset()
{
	address_space *program;

	m_maincpu = machine().device("maincpu");
	program = m_maincpu->memory().space(AS_PROGRAM);
	m_dma_1 = machine().device("dma_1");
	m_dma_2 = machine().device("dma_2");
	m_fdc = machine().device("fdc");
	m_pic_master = machine().device("pic8259_master");
	m_pic_slave = machine().device("pic8259_slave");
	m_pit = machine().device("pit");
	m_messram = machine().device<ram_device>(RAM_TAG);
	m_cdrom = machine().device<cdrom_image_device>("cdrom");
	m_cdda = machine().device("cdda");
	m_speaker = machine().device(SPEAKER_TAG);
	m_scsi = machine().device<fmscsi_device>("scsi:fm");
	m_ram = machine().device<ram_device>(RAM_TAG);
	m_ftimer = 0x00;
	m_freerun_timer = 0x00;
	m_nmi_mask = 0x00;
	m_compat_mode = 0x00;
	m_towns_ankcg_enable = 0x00;
	m_towns_mainmem_enable = 0x00;
	m_towns_system_port = 0x00;
	m_towns_ram_enable = 0x00;
	towns_update_video_banks(*program);
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

static const struct pit8253_config towns_pit8253_config =
{
	{
		{
			307200,
			DEVCB_NULL,
			DEVCB_LINE(towns_pit_out0_changed)
		},
		{
			307200,
			DEVCB_NULL,
			DEVCB_LINE(towns_pit_out1_changed)
		},
		{
			307200,
			DEVCB_NULL,
			DEVCB_DRIVER_LINE_MEMBER(towns_state,pit_out2_changed)
		}
	}
};

static const struct pit8253_config towns_pit8253_config_2 =
{
	{
		{
			307200,
			DEVCB_NULL,
			DEVCB_NULL  // reserved
		},
		{
			307200,
			DEVCB_NULL,
			DEVCB_NULL  // RS-232
		},
		{
			307200,
			DEVCB_NULL,
			DEVCB_NULL  // reserved
		}
	}
};

static READ8_DEVICE_HANDLER( get_slave_ack )
{
	towns_state* state = device->machine().driver_data<towns_state>();
	if (offset==7) { // IRQ = 7
		return pic8259_acknowledge(state->m_pic_slave);
	}
	return 0x00;
}
static const struct pic8259_interface towns_pic8259_master_config =
{
	DEVCB_LINE(towns_pic_irq),
	DEVCB_LINE_VCC,
	DEVCB_HANDLER(get_slave_ack)
};


static const struct pic8259_interface towns_pic8259_slave_config =
{
	DEVCB_DEVICE_LINE("pic8259_master", pic8259_ir7_w),
	DEVCB_LINE_GND,
	DEVCB_NULL
};

static const wd17xx_interface towns_mb8877a_interface =
{
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER( towns_state, mb8877a_irq_w),
	DEVCB_DRIVER_LINE_MEMBER( towns_state, mb8877a_drq_w),
	{FLOPPY_0,FLOPPY_1,0,0}
};

static LEGACY_FLOPPY_OPTIONS_START( towns )
	LEGACY_FLOPPY_OPTION( fmt_bin, "bin", "BIN disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([77])
		SECTORS([8])
		SECTOR_LENGTH([1024])
		FIRST_SECTOR_ID([1]))
LEGACY_FLOPPY_OPTIONS_END

static const floppy_interface towns_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(towns),
	NULL,
	NULL
};

static const upd71071_intf towns_dma_config =
{
	"maincpu",
	4000000,
	{ towns_fdc_dma_r, towns_scsi_dma_r, 0, towns_cdrom_dma_r },
	{ towns_fdc_dma_w, towns_scsi_dma_w, 0, 0 }
};

static const ym3438_interface ym3438_intf =
{
	towns_fm_irq
};

static const rf5c68_interface rf5c68_intf =
{
	towns_pcm_irq
};

static const FMSCSIinterface towns_scsi_config =
{
	DEVCB_LINE(towns_scsi_irq),
	DEVCB_LINE(towns_scsi_drq)
};

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

struct cdrom_interface towns_cdrom =
{
	NULL,
	NULL
};

static MACHINE_CONFIG_FRAGMENT( towns_base )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I386, 16000000)
	MCFG_CPU_PROGRAM_MAP(towns_mem)
	MCFG_CPU_IO_MAP(towns_io)
	MCFG_CPU_VBLANK_INT("screen", towns_vsync_irq)

	//    MCFG_MACHINE_RESET(towns)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(768,512)
	MCFG_SCREEN_VISIBLE_AREA(0, 768-1, 0, 512-1)
	MCFG_SCREEN_UPDATE_DRIVER(towns_state, screen_update)

	MCFG_GFXDECODE(towns)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("fm", YM3438, 53693100 / 7) // actual clock speed unknown
	MCFG_SOUND_CONFIG(ym3438_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SOUND_ADD("pcm", RF5C68, 53693100 / 7)  // actual clock speed unknown
	MCFG_SOUND_CONFIG(rf5c68_intf)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.50)
	MCFG_SOUND_ADD("cdda",CDDA,0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND,0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_PIT8253_ADD("pit",towns_pit8253_config)
	MCFG_PIT8253_ADD("pit2",towns_pit8253_config_2)

	MCFG_PIC8259_ADD( "pic8259_master", towns_pic8259_master_config )

	MCFG_PIC8259_ADD( "pic8259_slave", towns_pic8259_slave_config )

	MCFG_MB8877_ADD("fdc",towns_mb8877a_interface)
	MCFG_LEGACY_FLOPPY_4_DRIVES_ADD(towns_floppy_interface)

	MCFG_CDROM_ADD("cdrom",towns_cdrom)

	MCFG_SCSIBUS_ADD("scsi")
	MCFG_SCSIDEV_ADD("scsi:harddisk0", SCSIHD, SCSI_ID_0)
	MCFG_SCSIDEV_ADD("scsi:harddisk1", SCSIHD, SCSI_ID_1)
	MCFG_SCSIDEV_ADD("scsi:harddisk2", SCSIHD, SCSI_ID_2)
	MCFG_SCSIDEV_ADD("scsi:harddisk3", SCSIHD, SCSI_ID_3)
	MCFG_SCSIDEV_ADD("scsi:harddisk4", SCSIHD, SCSI_ID_4)
	MCFG_FMSCSI_ADD("scsi:fm",towns_scsi_config)

	MCFG_UPD71071_ADD("dma_1",towns_dma_config)
	MCFG_UPD71071_ADD("dma_2",towns_dma_config)

	MCFG_NVRAM_ADD_0FILL("nvram")

	//MCFG_VIDEO_START(towns)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("6M")
	MCFG_RAM_EXTRA_OPTIONS("2M,4M,8M,16M,32M,64M,96M")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( towns, towns_state )

	MCFG_FRAGMENT_ADD(towns_base)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( townsux, towns )

	MCFG_CPU_REPLACE("maincpu",I386, 16000000)
	MCFG_CPU_PROGRAM_MAP(ux_mem)
	MCFG_CPU_IO_MAP(towns_io)
	MCFG_CPU_VBLANK_INT("screen", towns_vsync_irq)

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("2M")
	MCFG_RAM_EXTRA_OPTIONS("10M")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( townssj, towns )

	MCFG_CPU_REPLACE("maincpu",I486, 66000000)
	MCFG_CPU_PROGRAM_MAP(towns_mem)
	MCFG_CPU_IO_MAP(towns_io)
	MCFG_CPU_VBLANK_INT("screen", towns_vsync_irq)

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("8M")
	MCFG_RAM_EXTRA_OPTIONS("40M,72M")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( townshr, towns )

	MCFG_CPU_REPLACE("maincpu",I486, 20000000)
	MCFG_CPU_PROGRAM_MAP(towns_mem)
	MCFG_CPU_IO_MAP(towns_io)
	MCFG_CPU_VBLANK_INT("screen", towns_vsync_irq)

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("4M")
	MCFG_RAM_EXTRA_OPTIONS("12M,20M,28M")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( marty, marty_state )
	MCFG_FRAGMENT_ADD(towns_base)

	MCFG_CPU_REPLACE("maincpu",I386, 16000000)
	MCFG_CPU_PROGRAM_MAP(marty_mem)
	MCFG_CPU_IO_MAP(towns_io)
	MCFG_CPU_VBLANK_INT("screen", towns_vsync_irq)

	MCFG_RAM_MODIFY(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("6M")
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

ROM_START( fmtmarty )
  ROM_REGION32_LE( 0x480000, "user", 0)
	ROM_LOAD("mrom.m36",  0x000000, 0x080000, CRC(9c0c060c) SHA1(5721c5f9657c570638352fa9acac57fa8d0b94bd) )
	ROM_CONTINUE(0x280000,0x180000)
	ROM_LOAD("mrom.m37",  0x400000, 0x080000, CRC(fb66bb56) SHA1(e273b5fa618373bdf7536495cd53c8aac1cce9a5) )
	ROM_CONTINUE(0x80000,0x100000)
	ROM_CONTINUE(0x180000,0x40000)
	ROM_CONTINUE(0x200000,0x40000)
ROM_END

ROM_START( carmarty )
  ROM_REGION32_LE( 0x480000, "user", 0)
	ROM_LOAD("fmt_dos.rom",  0x000000, 0x080000, CRC(2bc2af96) SHA1(99cd51c5677288ad8ef711b4ac25d981fd586884) )
	ROM_LOAD("fmt_dic.rom",  0x100000, 0x080000, CRC(82d1daa2) SHA1(7564020dba71deee27184824b84dbbbb7c72aa4e) )
	ROM_LOAD("fmt_fnt.rom",  0x180000, 0x040000, CRC(dd6fd544) SHA1(a216482ea3162f348fcf77fea78e0b2e4288091a) )
	ROM_LOAD("fmt_sys.rom",  0x200000, 0x040000, CRC(e1ff7ce1) SHA1(e6c359177e4e9fb5bbb7989c6bbf6e95c091fd88) )
	ROM_LOAD("mar_ex0.rom",  0x280000, 0x080000, CRC(e248bfbd) SHA1(0ce89952a7901dd4d256939a6bc8597f87e51ae7) )
	ROM_LOAD("mar_ex1.rom",  0x300000, 0x080000, CRC(ab2e94f0) SHA1(4b3378c772302622f8e1139ed0caa7da1ab3c780) )
	ROM_LOAD("mar_ex2.rom",  0x380000, 0x080000, CRC(ce150ec7) SHA1(1cd8c39f3b940e03f9fe999ebcf7fd693f843d04) )
	ROM_LOAD("mar_ex3.rom",  0x400000, 0x080000, CRC(582fc7fc) SHA1(a77d8014e41e9ff0f321e156c0fe1a45a0c5e58e) )
  ROM_REGION( 0x20, "serial", 0)
    ROM_LOAD("mytowns.rom",  0x00, 0x20, CRC(bc58eba6) SHA1(483087d823c3952cc29bd827e5ef36d12c57ad49) )
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT      MACHINE     INPUT    INIT    COMPANY      FULLNAME            FLAGS */
COMP( 1989, fmtowns,  0,    	0,		towns,		towns, driver_device,	 0,  "Fujitsu",   "FM-Towns",		 GAME_NOT_WORKING)
COMP( 1989, fmtownsa, fmtowns,	0,		towns,		towns, driver_device,	 0,  "Fujitsu",   "FM-Towns (alternate)", GAME_NOT_WORKING)
COMP( 1991, fmtownsux,fmtowns,	0,		townsux,	towns, driver_device,	 0,  "Fujitsu",   "FM-Towns II UX", GAME_NOT_WORKING)
COMP( 1992, fmtownshr,fmtowns,	0,		townshr,	towns, driver_device,	 0,  "Fujitsu",   "FM-Towns II HR", GAME_NOT_WORKING)
COMP( 19??, fmtownssj,fmtowns,	0,		townssj,	towns, driver_device,	 0,  "Fujitsu",   "FM-Towns II SJ", GAME_NOT_WORKING)
CONS( 1993, fmtmarty, 0,    	0,		marty,		marty, driver_device,	 0,  "Fujitsu",   "FM-Towns Marty",	 GAME_NOT_WORKING)
CONS( 1994, carmarty, fmtmarty,	0,		marty,		marty, driver_device,	 0,  "Fujitsu",   "FM-Towns Car Marty",	 GAME_NOT_WORKING)

