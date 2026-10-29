// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Brian H. Gilbert
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Disk System expansion

 iNES mappers: none (iNES mapper 20 is reserved for emulator use for FDS emulation)


 Here we emulate the RAM expansion + Disk Drive which form the
 Famicom Disk System.

 Based on info from NESDev wiki ( https://www.nesdev.org/wiki/Family_Computer_Disk_System )

 TODO:
   - convert floppy drive + fds format to modern code!

 ***********************************************************************************************************/


#include "emu.h"
#include "disksys.h"
#include "imagedev/flopdrv.h"
#include "formats/nes_dsk.h"
#include "speaker.h"

#ifdef NES_PCB_DEBUG
#define VERBOSE (LOG_GENERAL)
#else
#define VERBOSE (0)
#endif
#include "logmacro.h"


//-----------------------------------------------
//
//  Disk drive implementation
//
//-----------------------------------------------

static const floppy_interface nes_floppy_interface =
{
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(nes_only),
	"floppy_5_25"
};


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void nes_disksys_device::device_add_mconfig(machine_config &config)
{
	LEGACY_FLOPPY(config, m_disk, &nes_floppy_interface);

	RP2C33_SOUND(config, m_sound, XTAL(21'477'272)/12); // clock driven from motherboard?
	// level measured with nes-audio-tests db_fds against the hardware survey (+6.6 dB vs a full-volume APU square)
	m_sound->add_route(0, DEVICE_SELF_OWNER, 0.227);
}


ROM_START( disksys )
	ROM_REGION(0x2000, "drive", 0)
	ROM_SYSTEM_BIOS( 0, "2c33a-01a", "Famicom Disk System BIOS")
	ROMX_LOAD( "rp2c33a-01a.bin", 0x0000, 0x2000, CRC(5e607dcf) SHA1(57fe1bdee955bb48d357e463ccbf129496930b62), ROM_BIOS(0)) // newer, Nintendo logo has no shadow
	ROM_SYSTEM_BIOS( 1, "2c33-01", "Famicom Disk System BIOS, older")
	ROMX_LOAD( "rp2c33-01.bin", 0x0000, 0x2000, CRC(1c7ae5d5) SHA1(af5af53f66982e749643fdf8b2acbb7d4d3ed229), ROM_BIOS(1)) // older, Nintendo logo has shadow
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *nes_disksys_device::device_rom_region() const
{
	return ROM_NAME( disksys );
}


void nes_disksys_device::load_proc(device_image_interface &image, bool is_created)
{
	nes_disksys_device *disk_sys = static_cast<nes_disksys_device *>(image.device().owner());
	disk_sys->load_disk(image);
}

void nes_disksys_device::unload_proc(device_image_interface &image)
{
	nes_disksys_device *disk_sys = static_cast<nes_disksys_device *>(image.device().owner());
	disk_sys->unload_disk(image);
}


//------------------------------------------------
//
//  RAM expansion cart implementation
//
//------------------------------------------------

DEFINE_DEVICE_TYPE(NES_DISKSYS, nes_disksys_device, "fc_disksys", "FC RAM Expansion + Disk System PCB")


nes_disksys_device::nes_disksys_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: nes_nrom_device(mconfig, NES_DISKSYS, tag, owner, clock)
	, m_2c33_rom(*this, "drive")
	, m_fds_data(nullptr)
	, m_write_started(false), m_write_pending_valid(false), m_write_pending(0)
	, m_disk(*this, "floppy0")
	, m_sound(*this, "rp2c33snd")
	, irq_timer(nullptr), transfer_timer(nullptr)
	, m_irq_count(0), m_irq_count_latch(0), m_irq_enable(false), m_irq_repeat(false), m_irq_transfer(false), m_disk_reg_enable(false), m_sound_en(false)
	, m_fds_scan(0), m_fds_door_closed(0), m_fds_current_side(0), m_fds_head_position(0), m_scan_bytes(0), m_fds_status0(0), m_read_mode(0), m_drive_ready(0)
	, m_fds_sides(0), m_fds_last_side(0), m_fds_count(0)
{
}


void nes_disksys_device::device_start()
{
	common_start();

	m_disk->floppy_install_load_proc(nes_disksys_device::load_proc);
	m_disk->floppy_install_unload_proc(nes_disksys_device::unload_proc);

	m_fds_data = std::make_unique<uint8_t[]>(MAX_SIDES * SIDE_SIZE);
	m_fds_loaded = std::make_unique<uint8_t[]>(MAX_SIDES * SIDE_SIZE);
	std::fill_n(m_fds_data.get(), MAX_SIDES * SIDE_SIZE, 0);
	save_pointer(NAME(m_fds_data), MAX_SIDES * SIDE_SIZE);
	save_item(NAME(m_fds_sides));

	transfer_timer = timer_alloc(FUNC(nes_disksys_device::transfer_tick), this);
	transfer_timer->adjust(attotime::zero, 0, clocks_to_attotime(TRANSFER_CYCLES));

	save_item(NAME(m_fds_scan));
	save_item(NAME(m_fds_door_closed));
	save_item(NAME(m_fds_current_side));
	m_fds_current_side = 1;   // power-on: side A is in the drive
	save_item(NAME(m_scan_bytes));
	save_item(NAME(m_fds_head_position));
	save_item(NAME(m_fds_status0));
	save_item(NAME(m_read_mode));
	save_item(NAME(m_drive_ready));
	save_item(NAME(m_4025));
	save_item(NAME(m_4026));
	save_item(NAME(m_read_data));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_repeat));
	save_item(NAME(m_irq_transfer));
	save_item(NAME(m_write_started));
	save_item(NAME(m_write_pending_valid));
	save_item(NAME(m_write_pending));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));
	save_item(NAME(m_disk_reg_enable));
	save_item(NAME(m_sound_en));
	save_item(NAME(m_wd_timer));
	save_item(NAME(m_wd_refresh));
	save_item(NAME(m_wd_irq_enable));

	save_item(NAME(m_fds_last_side));
	save_item(NAME(m_fds_count));
}

void nes_disksys_device::pcb_reset()
{
	// read accesses in 0x6000-0xffff are always handled by
	// cutom code below, so no need to setup the prg...
	chr8(0, CHRRAM);
	set_nt_mirroring(PPU_MIRROR_VERT);

	m_write_started = m_write_pending_valid = false;
	m_fds_scan = 0;
	m_fds_door_closed = 0;
	m_fds_head_position = 0;
	m_scan_bytes = 0;
	m_fds_status0 = 0;
	m_read_mode = 1;
	m_drive_ready = 0;
	m_4025 = 0x06;      // the disk registers are disabled: $4025 = $06 and $4026 = $7F (NESdev, Disk I/O reset)
	m_4026 = 0x7f;
	m_read_data = 0;
	m_irq_count = 0;
	m_irq_count_latch = 0;
	m_irq_enable = false;
	m_irq_repeat = false;
	m_irq_transfer = false;
	m_disk_reg_enable = false;
	m_sound_en = false;
	m_wd_timer = 0xfff;
	m_wd_refresh = 0;
	m_wd_irq_enable = false;

	// the reset button does not move the disk: whatever side is in the drive is still reported as inserted (the
	// "no disk" reads after a side change only follow disk_flip_side())
	m_fds_count = 0;
	m_fds_last_side = m_fds_current_side;
}


/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 RAM is in 0x6000-0xdfff (32K)
 ROM is in 0xe000-0xffff (8K)

 registers + disk drive are accessed in
 0x4020-0x403f (read_ex/write_ex below)

 -------------------------------------------------*/

void nes_disksys_device::write_h(offs_t offset, uint8_t data)
{
	LOG("Famicom Disk System write_h, offset %04x, data: %02x\n", offset, data);

	if (offset < 0x6000)
		m_prgram[offset + 0x2000] = data;
}

uint8_t nes_disksys_device::read_h(offs_t offset)
{
	LOG("Famicom Disk System read_h, offset: %04x\n", offset);

	if (offset < 0x6000)
		return m_prgram[offset + 0x2000];
	else
		return m_2c33_rom[offset & 0x1fff];
}

void nes_disksys_device::write_m(offs_t offset, uint8_t data)
{
	LOG("Famicom Disk System write_m, offset: %04x, data: %02x\n", offset, data);
	m_prgram[offset] = data;
}

uint8_t nes_disksys_device::read_m(offs_t offset)
{
	LOG("Famicom Disk System read_m, offset: %04x\n", offset);
	return m_prgram[offset];
}

TIMER_CALLBACK_MEMBER(nes_disksys_device::transfer_tick)
{
	// fires at the disk's real ~96.4 kHz/149-CPU-cycle byte transfer rate (see header note on TRANSFER_CYCLES).
	// NESdev ($4030, Byte Transfer Flag): the flag is set once a byte would have been transferred whether or not its
	// IRQ is enabled, as long as CRC is enabled ($4025.D6 = 1) and $4025.D5 = 1, and in read mode ($4025.D2 = 1) only
	// while the motor runs ($4025.D1 = 0); $4025.D7 only decides whether it raises an IRQ (update_irq). Tonkachi Editor
	// polls $4030.D7 with the IRQ off. Reading $4030 does not clear it: $4024 writes and $4031 reads do.
	// the head moves across the disk in time while the drive scans, whether or not the CPU reads the bytes; when it has
	// passed a whole side the drive reports not ready ($4032.D1) until the head is rewound
	if (m_fds_scan && !BIT(m_4025, 1) && (m_scan_bytes < SIDE_SIZE))
		m_scan_bytes++;

	const bool runs = BIT(m_4025, 6) && BIT(m_4025, 5) && (!BIT(m_4025, 2) || !BIT(m_4025, 1));
	if (runs)
	{
		m_fds_status0 |= 0x80;
		update_irq();
	}
}

// /IRQ is the NOR of the three sources (NESdev): the timer ($4030.D0), the DRAM refresh watchdog ($4030.D1) and the byte
// transfer flag ($4030.D7) while $4025.D7 enables its interrupt
void nes_disksys_device::update_irq()
{
	const bool irq = BIT(m_fds_status0, 0) || BIT(m_fds_status0, 1) || (BIT(m_fds_status0, 7) && m_irq_transfer);
	set_irq_line(irq ? ASSERT_LINE : CLEAR_LINE);
}

// One CPU cycle of the DRAM refresh watchdog (NESdev, Family Computer Disk System, DRAM Refresh Watchdog): the timer
// decrements every cycle and reloads with 3245 when the refresh counter overflows, the refresh counter advances on every
// cycle that does not access PRG-RAM, and an underflow of the timer raises the IRQ while $4023.D7 = 1. Both counters keep
// running while the IRQ is asserted; while $4023.D0 = 0 they are held at 4095 and 0. The order (timer, then refresh) is
// MesenCE's (TakuikaNinja, matched to his FDS-4030D1-Addr and FDS-4030D1-Micro tests on a Twin Famicom).
void nes_disksys_device::watchdog_tick(offs_t address)
{
	if (!m_disk_reg_enable)
		return;

	if (m_wd_timer == 0)
	{
		m_wd_timer = 0xfff;
		if (m_wd_irq_enable && !BIT(m_fds_status0, 1))
		{
			m_fds_status0 |= 0x02;
			update_irq();
		}
	}
	else
		m_wd_timer--;

	if ((address < 0x6000) || (address >= 0xe000))
	{
		if (m_wd_refresh == 0x7f)
		{
			m_wd_refresh = 0;
			m_wd_timer = 3245;
		}
		else
			m_wd_refresh++;
	}
}

// $4025 - FDS Control (NESdev)
void nes_disksys_device::write_4025(uint8_t data)
{
	// bit0 - Scan disk (0: reset, 1: scan)
	// bit1 - Drive motor control (0: start, 1: stop)
	// bit2 - Transfer mode (0: write; 1: read)
	// bit3 - Nametable arrangement (0: horizontal, i.e. vertical mirroring; 1: vertical)
	// bit4 - CRC transfer control (1: transfer the CRC value)
	// bit5 - Unknown, always written as 1 (the byte transfer flag needs it)
	// bit6 - CRC enabled (0: disable/reset; 1: enable; the byte transfer flag only runs while it is set)
	// bit7 - Interrupt enabled (1: an IRQ every time the byte transfer flag is raised)
	// bit0 is /reset ("scan media") and bit1 /motor on on the drive cable (NESdev, FDS RAM adaptor cable pinout,
	// pins 12 and 3): while bit0 is 0 the media pointer is reset and stays reset at the beginning of the disk,
	// so the head is no longer on the innermost track ($4030.D6); bit1 only starts and stops the motor, it does
	// not move the pointer. With the motor stopped nothing passes under the head (see transfer_tick, $4031).
	m_fds_scan = BIT(data, 0);
	if (!m_fds_scan)
	{
		m_fds_head_position = m_scan_bytes = 0;
		m_fds_status0 &= ~0x40;
	}

	// Stopping after a read: the BIOS has also read the 2 CRC bytes the disk image does not hold, so the head is 2
	// bytes too far. After a write nothing was transferred for the CRC (the drive writes it itself), so the head
	// is already where the next block starts.
	if (!(data & 0x40) && m_drive_ready && m_read_mode && m_fds_head_position > 2)
		m_fds_head_position -= 2;

	m_read_mode = BIT(data, 2);
	// reading, or the CRC control bit (the drive appends the CRC and the next block starts with a new gap) ends
	// the block being written
	if (m_read_mode || BIT(data, 4))
		m_write_started = m_write_pending_valid = false;
	if (BIT(data, 3))
		m_fds_status0 |= 0x08;
	else
		m_fds_status0 &= ~0x08;
	set_nt_mirroring(BIT(m_fds_status0, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
	m_drive_ready = data & 0x40;
	m_irq_transfer = BIT(data, 7);
	m_4025 = data;
	// a $4025 write restarts the drive's transfer timing, so a byte transfer IRQ that was raised but not yet
	// serviced goes away with it. Otherwise the BIOS (which turns the transfer IRQ off with a $4025 write right
	// after it has read the last byte it needs) gets one more interrupt after it stopped expecting any, and
	// its handler unwinds the caller's stack for it.
	m_fds_status0 &= ~0x80;
	update_irq();
}

void nes_disksys_device::write_ex(offs_t offset, uint8_t data)
{
	LOG("Famicom Disk System write_ex, offset: %04x, data: %02x\n", offset, data);

	if (offset >= 0x20 && offset < 0x60)
	{
		// wavetable
		if (m_sound_en)
			m_sound->wave_w(offset - 0x20, data);
	}

	switch (offset)
	{
		case 0x00:
			m_irq_count_latch = (m_irq_count_latch & 0xff00) | data;
			break;
		case 0x01:
			m_irq_count_latch = (m_irq_count_latch & 0x00ff) | (data << 8);
			break;
		case 0x02:
			if (m_disk_reg_enable)
			{
				m_irq_repeat = BIT(data, 0);
				m_irq_enable = BIT(data, 1);
				if (m_irq_enable)
					m_irq_count = m_irq_count_latch;
				else
				{
					// stopping the counter acknowledges a pending timer IRQ (NESdev)
					m_fds_status0 &= ~0x01;
					update_irq();
				}
			}
			break;
		case 0x03:
			// bit0 - Enable disk I/O registers
			// bit1 - Enable sound I/O registers
			// bit7 - Enable DRAM refresh watchdog interrupts: clearing it acknowledges a pending one but leaves the
			//        watchdog's counters alone (NESdev)
			m_disk_reg_enable = BIT(data, 0);
			m_wd_irq_enable = BIT(data, 7);
			if (!m_wd_irq_enable)
				m_fds_status0 &= ~0x02;
			if (!m_disk_reg_enable)
			{
				// disabling the disk registers disables and acknowledges the timer, watchdog and byte transfer IRQs and
				// holds the watchdog's timer at 4095 and its refresh counter at 0 (NESdev, Disk I/O reset)
				m_irq_enable = 0;
				m_fds_status0 &= ~0x83;
				m_wd_timer = 0xfff;
				m_wd_refresh = 0;
				// disabling the disk registers indirectly sets $4025 = $06 on hardware, which also resets the nametable
				// arrangement and clears $4030.D3 (TakuikaNinja's FDS Mirroring Test, verified on a Sharp Twin Famicom)
				write_4025(0x06);
				m_4026 = 0x7f;   // "$4026 is set to $7F" (NESdev, Disk I/O reset)
			}
			update_irq();
			m_sound_en = BIT(data, 1);
			break;
		case 0x04:
			if (!m_disk_reg_enable)   // write-only disk register: held in reset while $4023.D0 is 0
				break;
			// write data out to disk. A block is written as a gap of zeros, the start mark ($80), the block's bytes and
			// then the drive adds the CRC itself; the disk image holds only the block's bytes, so the gap and the mark
			// are not stored. A byte written to $4024 is only clocked onto the disk at the next transfer: the dummy byte
			// the BIOS writes after the last one of a block never gets there, as the CRC control bit replaces it.
			if (m_fds_sides && m_fds_current_side && !m_read_mode)
			{
				if (!m_write_started)
				{
					if (data == 0x80)
					{
						m_write_started = true;
						m_write_pending_valid = false;
					}
				}
				else
				{
					if (m_write_pending_valid && (m_fds_head_position < SIDE_SIZE))
						m_fds_data[(m_fds_current_side - 1) * SIDE_SIZE + m_fds_head_position++] = m_write_pending;
					m_write_pending = data;
					m_write_pending_valid = true;
				}
			}
			// clear the byte transfer flag
			m_fds_status0 &= ~0x80;
			update_irq();
			break;
		case 0x05:
			// "Enable disk I/O registers (1: enable, 0: reset & disable writes)" (NESdev, $4023): the write-only disk
			// registers stay in their reset state while $4023.D0 is 0
			if (m_disk_reg_enable)
				write_4025(data);
			break;
		case 0x06:
			// $4026 - external connector output (open collector, read back through $4033)
			if (m_disk_reg_enable)
				m_4026 = data;
			break;
		case 0x60:  // $4080 - Volume envelope - read through $4090
		case 0x62:  // $4082 - Frequency low
		case 0x63:  // $4083 - Frequency high
		case 0x64:  // $4084 - Mod envelope - read through $4092
		case 0x65:  // $4085 - Mod counter
		case 0x66:  // $4086 - Mod frequency low
		case 0x67:  // $4087 - Mod frequency high
		case 0x68:  // $4088 - Mod table write
		case 0x69:  // $4089 - Wave write / master volume
		case 0x6a:  // $408a - Envelope speed
			if (m_sound_en)
				m_sound->write(offset - 0x60, data);
			break;
	}
}

uint8_t nes_disksys_device::read_ex(offs_t offset)
{
	LOG("Famicom Disk System read_ex, offset: %04x\n", offset);
	uint8_t ret = 0x00;

	if (offset >= 0x20 && offset < 0x60)
	{
		// wavetable
		if (m_sound_en)
			ret = m_sound->wave_r(offset - 0x20);
	}

	switch (offset)
	{
		case 0x10:
			// $4030 - disk status (NESdev)
			// bit0 - Timer Interrupt (1: an IRQ occurred)
			// bit1 - DRAM Refresh Watchdog (1: an IRQ occurred)
			// bit3 - Nametable arrangement (from $4025.D3)
			// bit4 - CRC control (0: CRC passed; 1: CRC error)
			// bit6 - End of Head (1 when disk head is on the most inner track)
			// bit7 - Byte transfer flag (set every time 8 bits have been transferred; cleared by $4024 writes and $4031
			//        reads, not by reading $4030)
			ret = m_fds_status0;
			if (!machine().side_effects_disabled())
			{
				// reading acknowledges the timer and watchdog IRQs, not the byte transfer flag
				m_fds_status0 &= ~0x03;
				update_irq();
			}
			break;
		case 0x11:
			// $4031 - data latch
			// don't read data if disk is unloaded
			if (!m_fds_sides)
				ret = 0;
			else if (m_fds_current_side && m_read_mode)
			{
				// the register holds the last byte transferred (NESdev: it "is loaded with the contents of an internal
				// shift register every time the byte transfer flag raises"). The media pointer only advances while
				// the drive scans ($4025.D0 = 1) with its motor running ($4025.D1 = 0): held in reset or stopped,
				// no new byte arrives and the register keeps its value. (This model moves the pointer when the CPU
				// takes a byte, as the disk image holds no gaps to pass in time.)
				if (!m_fds_scan || BIT(m_4025, 1))
					ret = m_read_data;
				else
					ret = m_fds_data[(m_fds_current_side - 1) * SIDE_SIZE + m_fds_head_position];
				if (!machine().side_effects_disabled() && m_fds_scan && !BIT(m_4025, 1))
				{
					m_read_data = ret;
					m_fds_head_position++;
					if (m_fds_head_position == SIDE_SIZE)
					{
						logerror("%s: end of disk reached!\n", machine().describe_context());
						m_fds_status0 |= 0x40;
						m_fds_head_position -= 2;
					}
				}
			}
			else
				ret = 0;
			// clear the byte transfer flag
			if (!machine().side_effects_disabled())
			{
				m_fds_status0 &= ~0x80;
				update_irq();
			}
			break;
		case 0x12:
			// $4032 - disk status 1:
			// bit0 - Disk flag  (0: Disk inserted; 1: Disk not inserted)
			// bit1 - Ready flag (0: Disk ready; 1: Disk not ready)
			// bit2 - Protect flag (0: Not write protected; 1: Write protected or disk ejected)
			if (!m_fds_sides)
				ret = 1;
			else if (m_fds_last_side != m_fds_current_side)
			{
				// If we've switched disks, report "no disk" for a few reads
				ret = 1;
				if (!machine().side_effects_disabled())
				{
					m_fds_count++;
					if (m_fds_count == 50)
					{
						m_fds_last_side = m_fds_current_side;
						m_fds_count = 0;
					}
				}
			}
			else
			{
				ret = (m_fds_current_side == 0) ? 1 : 0; // 0 if a disk is inserted
				// bit1, /ready (NESdev, FDS RAM adaptor cable pinout): active (0) once the drive has acknowledged
				// "/scan media" with the head at the start of the disk; inactive (1) when the head reaches the end of
				// the disk (most inner track) or "/scan media" goes inactive. Here: a disk is in, the drive is
				// scanning ($4025.D0) and the head has not run off the end of the side
				// ($4030.D6) or, in time, passed a whole side (m_scan_bytes: one byte per byte transfer period while
				// scanning; the disk image holds no gaps, so this end comes about as late as the image's last byte
				// would, an approximation of the real disk's). Kosodate Gokko waits for it to go inactive after the BIOS
				// has loaded it, with the drive still scanning.
				if (!m_fds_current_side || !m_fds_scan || BIT(m_fds_status0, 6) || (m_scan_bytes >= SIDE_SIZE))
					ret |= 0x02;
			}
			break;
		case 0x13:
			// $4033 - external connector (bits 0-6) + drive power/battery status (bit 7). NESdev: the $4026 outputs are
			// open collector with pull-ups and shared with these inputs, so a bit that is clear in $4026 reads back as
			// 0 (nothing is connected to the port here, so the others read 1); the battery bit reads good (1) only
			// while the motor is on ($4025.D1 = 0), "otherwise it will always be read as 0".
			ret = (m_4026 & 0x7f) | (BIT(m_4025, 1) ? 0x00 : 0x80);
			break;
		case 0x70:  // $4090 - Volume gain - write through $4080
		case 0x71:  // $4091 - Wave accumulator
		case 0x72:  // $4092 - Mod gain - read through $4084
		case 0x73:  // $4093 - Mod table address accumulator
		case 0x74:  // $4094 - Mod counter * gain result
		case 0x75:  // $4095 - Mod counter increment
		case 0x76:  // $4096 - Wavetable value
		case 0x77:  // $4097 - Mod counter value
			if (m_sound_en)
				ret = m_sound->read(offset - 0x60);
			break;
		default:
			// (the wavetable, $4040-$407F, was read above)
			break;
	}

	return ret;
}

//-------------------------------------------------
//  irq_timer_tick - handle IRQ timer
//-------------------------------------------------

TIMER_CALLBACK_MEMBER(nes_disksys_device::irq_timer_tick)
{
	if (m_irq_enable)
	{
		if (m_irq_count)
			m_irq_count--;
		else
		{
			m_irq_count = m_irq_count_latch;
			if (!m_irq_repeat)
				m_irq_enable = 0;
			m_fds_status0 |= 0x01;
			update_irq();
		}
	}
}


// Hacky helper to allow user to switch disk side with a simple key

void nes_disksys_device::disk_flip_side()
{
	m_fds_current_side++;
	if (m_fds_current_side > m_fds_sides)
		m_fds_current_side = 0;

	if (m_fds_current_side == 0)
		popmessage("No disk inserted.");
	else
		popmessage("Disk set to disk %d, side %c", (m_fds_current_side + 1) / 2, ((m_fds_current_side - 1) & 1) ? 'B' : 'A');
}

// the drive's own key for turning the disk over (one turn per press)
static INPUT_PORTS_START( nes_disksys )
	PORT_START("FLIPDISK")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("Change Disk Side") PORT_CODE(KEYCODE_SPACE) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(nes_disksys_device::flip_side_changed), 0)
INPUT_PORTS_END

ioport_constructor nes_disksys_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nes_disksys );
}

INPUT_CHANGED_MEMBER(nes_disksys_device::flip_side_changed)
{
	if (newval)
		disk_flip_side();
}



// Disk Loading / Unloading

// A .qd side holds the blocks of the disk in order, each followed by its 2 byte CRC and with no gaps (NESdev, FDS disk
// format): the disk info block (56 bytes with its block code), the file amount block (2 bytes) and a file header block
// (16 bytes) and file data block (the block code and the file's size, which is in the header block) for each file, hidden
// ones included. Copy them without the CRCs, as the .fds layout the drive works on has them, and leave the rest of the
// side zero. A side that does not start with the disk info block, or whose blocks end early or do not fit, ends there
void nes_disksys_device::qd_side_to_fds(const uint8_t *qd, uint8_t *fds)
{
	unsigned src = 0, dst = 0;
	auto const copy_block = [&] (unsigned length)
	{
		if (((src + length + 2) > QD_SIDE_SIZE) || ((dst + length) > SIDE_SIZE))
			return false;
		std::memcpy(&fds[dst], &qd[src], length);
		src += length + 2;
		dst += length;
		return true;
	};

	if ((qd[0] != 0x01) || !copy_block(56))
		return;
	if ((src >= QD_SIDE_SIZE) || (qd[src] != 0x02) || !copy_block(2))
		return;
	while ((src < QD_SIDE_SIZE) && (qd[src] == 0x03) && ((src + 16) <= QD_SIDE_SIZE))
	{
		unsigned const size = qd[src + 13] | (unsigned(qd[src + 14]) << 8);
		if (!copy_block(16))
			return;
		if ((src >= QD_SIDE_SIZE) || (qd[src] != 0x04) || !copy_block(1 + size))
			return;
	}
}

void nes_disksys_device::load_disk(device_image_interface &image)
{
	int header = 0;
	m_fds_sides = 0;

	if (image.is_filetype("qd"))
	{
		unsigned const sides = image.length() / QD_SIDE_SIZE;
		if (!sides || (sides > MAX_SIDES) || (image.length() % QD_SIDE_SIZE))
		{
			osd_printf_error("%s: not a whole number of Quick Disk sides of %u bytes, at most %u\n", image.basename(), QD_SIDE_SIZE, MAX_SIDES);
			return;
		}
		m_fds_sides = sides;

		std::fill_n(m_fds_data.get(), MAX_SIDES * SIDE_SIZE, 0);
		std::unique_ptr<uint8_t []> qd = std::make_unique<uint8_t []>(QD_SIDE_SIZE);
		for (unsigned side = 0; side < sides; side++)
		{
			image.fseek(uint64_t(side) * QD_SIDE_SIZE, SEEK_SET);
			image.fread(qd.get(), QD_SIDE_SIZE);
			qd_side_to_fds(qd.get(), &m_fds_data[side * SIDE_SIZE]);
		}

		// what the games wrote to the disk in earlier sessions: as for a .fds image, in the .fds layout
		size_t const size = size_t(sides) * SIDE_SIZE;
		std::memcpy(m_fds_loaded.get(), m_fds_data.get(), size);
		image.battery_load(m_fds_data.get(), size, m_fds_loaded.get());
		std::memcpy(m_fds_loaded.get(), m_fds_data.get(), size);
		return;
	}

	if (image.length() % SIDE_SIZE)
		header = 0x10;

	unsigned const sides = (image.length() - header) / SIDE_SIZE;
	if (sides > MAX_SIDES)
	{
		osd_printf_error("%s: disk image has %u sides, at most %u are supported\n", image.basename(), sides, MAX_SIDES);
		return;
	}
	m_fds_sides = sides;

	const size_t size = size_t(m_fds_sides) * SIDE_SIZE;
	std::fill_n(m_fds_data.get(), MAX_SIDES * SIDE_SIZE, 0);
	if (!size)
		return;

	// if there is an header, skip it
	image.fseek(header, SEEK_SET);
	image.fread(m_fds_data.get(), size);

	// what the games wrote to the disk in earlier sessions is kept apart from the disk image, like the save RAM of a
	// cartridge: the saved copy replaces the disk image's data when there is one
	std::memcpy(m_fds_loaded.get(), m_fds_data.get(), size);
	image.battery_load(m_fds_data.get(), size, m_fds_loaded.get());
	std::memcpy(m_fds_loaded.get(), m_fds_data.get(), size);
}

void nes_disksys_device::unload_disk(device_image_interface &image)
{
	// write out what the games changed on the disk (only when something was; the disk image itself is never modified)
	const size_t size = size_t(m_fds_sides) * SIDE_SIZE;
	if (size && std::memcmp(m_fds_data.get(), m_fds_loaded.get(), size))
	{
		image.battery_save(m_fds_data.get(), size);
		std::memcpy(m_fds_loaded.get(), m_fds_data.get(), size);
	}
	m_fds_sides =  0;
}
