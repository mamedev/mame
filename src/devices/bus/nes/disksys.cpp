// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Disk System expansion


 Here we emulate the RAM expansion + Disk Drive which form the
 Famicom Disk System.

 Based on info from NESDev wiki ( http://wiki.nesdev.com/w/index.php/Family_Computer_Disk_System )

 TODO:
   - convert floppy drive + fds format to modern code!
   - add sound bits
   - stop IRQ from using HOLD_LINE

 ***********************************************************************************************************/


#include "emu.h"
#include "disksys.h"
#include "cpu/m6502/m6502.h"
#include "imagedev/flopdrv.h"
#include "formats/nes_dsk.h"

#ifdef NES_PCB_DEBUG
	#define VERBOSE 1
#else
	#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


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

static MACHINE_CONFIG_FRAGMENT( nes_disksys )
	MCFG_LEGACY_FLOPPY_DRIVE_ADD(FLOPPY_0, nes_floppy_interface)
MACHINE_CONFIG_END

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor nes_disksys_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( nes_disksys );
}



ROM_START( disksys )
	ROM_REGION(0x2000, "drive", 0)
	ROM_SYSTEM_BIOS( 0, "2c33a-01a", "Famicom Disk System Bios")
	ROMX_LOAD( "rp2c33a-01a.bin", 0x0000, 0x2000, CRC(5e607dcf) SHA1(57fe1bdee955bb48d357e463ccbf129496930b62), ROM_BIOS(1)) // newer, Nintendo logo has no shadow
	ROM_SYSTEM_BIOS( 1, "2c33-01", "Famicom Disk System Bios, older")
	ROMX_LOAD( "rp2c33-01.bin", 0x0000, 0x2000, CRC(1c7ae5d5) SHA1(af5af53f66982e749643fdf8b2acbb7d4d3ed229), ROM_BIOS(2)) // older, Nintendo logo has shadow
ROM_END

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *nes_disksys_device::device_rom_region() const
{
	return ROM_NAME( disksys );
}


void nes_disksys_device::load_proc(device_image_interface &image)
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

const device_type NES_DISKSYS = &device_creator<nes_disksys_device>;


nes_disksys_device::nes_disksys_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
					: nes_nrom_device(mconfig, NES_DISKSYS, "FC RAM Expansion + Disk System PCB", tag, owner, clock, "fc_disksys", __FILE__), m_2c33_rom(nullptr),
						m_fds_data(nullptr),
						m_disk(*this, FLOPPY_0), irq_timer(nullptr), m_irq_count(0), m_irq_count_latch(0), m_irq_enable(0), m_irq_transfer(0), m_fds_motor_on(0), m_fds_door_closed(0), m_fds_current_side(0), m_fds_head_position(0), m_fds_status0(0), m_read_mode(0), m_drive_ready(0),
						m_fds_sides(0), m_fds_last_side(0), m_fds_count(0)
				{
}


void nes_disksys_device::device_start()
{
	common_start();

	m_2c33_rom = (UINT8*)memregion("drive")->base();

	m_disk->floppy_install_load_proc(nes_disksys_device::load_proc);
	m_disk->floppy_install_unload_proc(nes_disksys_device::unload_proc);

	irq_timer = timer_alloc(TIMER_IRQ);
	irq_timer->adjust(attotime::zero, 0, machine().device<cpu_device>("maincpu")->cycles_to_attotime(1));

	save_item(NAME(m_fds_motor_on));
	save_item(NAME(m_fds_door_closed));
	save_item(NAME(m_fds_current_side));
	save_item(NAME(m_fds_head_position));
	save_item(NAME(m_fds_status0));
	save_item(NAME(m_read_mode));
	save_item(NAME(m_drive_ready));
	save_item(NAME(m_irq_enable));
	save_item(NAME(m_irq_transfer));
	save_item(NAME(m_irq_count));
	save_item(NAME(m_irq_count_latch));

	save_item(NAME(m_fds_last_side));
	save_item(NAME(m_fds_count));
}

void nes_disksys_device::pcb_reset()
{
	// read accesses in 0x6000-0xffff are always handled by
	// cutom code below, so no need to setup the prg...
	chr8(0, CHRRAM);
	set_nt_mirroring(PPU_MIRROR_VERT);

	m_fds_motor_on = 0;
	m_fds_door_closed = 0;
	m_fds_current_side = 1;
	m_fds_head_position = 0;
	m_fds_status0 = 0;
	m_read_mode = 0;
	m_drive_ready = 0;
	m_irq_count = 0;
	m_irq_count_latch = 0;
	m_irq_enable = 0;
	m_irq_transfer = 0;

	m_fds_count = 0;
	m_fds_last_side = 0;
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

WRITE8_MEMBER(nes_disksys_device::write_h)
{
	LOG_MMC(("Famicom Disk System write_h, offset %04x, data: %02x\n", offset, data));

	if (offset < 0x6000)
		m_prgram[offset + 0x2000] = data;
}

READ8_MEMBER(nes_disksys_device::read_h)
{
	LOG_MMC(("Famicom Disk System read_h, offset: %04x\n", offset));

	if (offset < 0x6000)
		return m_prgram[offset + 0x2000];
	else
		return m_2c33_rom[offset & 0x1fff];
}

WRITE8_MEMBER(nes_disksys_device::write_m)
{
	LOG_MMC(("Famicom Disk System write_m, offset: %04x, data: %02x\n", offset, data));
	m_prgram[offset] = data;
}

READ8_MEMBER(nes_disksys_device::read_m)
{
	LOG_MMC(("Famicom Disk System read_m, offset: %04x\n", offset));
	return m_prgram[offset];
}

void nes_disksys_device::hblank_irq(int scanline, int vblank, int blanked)
{
	if (m_irq_transfer)
		m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
}

WRITE8_MEMBER(nes_disksys_device::write_ex)
{
	LOG_MMC(("Famicom Disk System write_ex, offset: %04x, data: %02x\n", offset, data));

	if (offset >= 0x20 && offset < 0x60)
	{
		// wavetable
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
			m_irq_count = m_irq_count_latch;
			m_irq_enable = BIT(data, 1);
			break;
		case 0x03:
			// bit0 - Enable disk I/O registers
			// bit1 - Enable sound I/O registers
			break;
		case 0x04:
			// write data out to disk
			// TEST!
			if (m_fds_data && m_fds_current_side && !m_read_mode)
				m_fds_data[(m_fds_current_side - 1) * 65500 + m_fds_head_position++] = data;
			break;
		case 0x05:
			// $4025 - FDS Control
			// bit0 - Drive Motor Control (0: Stop motor; 1: Turn on motor)
			// bit1 - Transfer Reset (Set 1 to reset transfer timing to the initial state)
			// bit2 - Read / Write mode (0: write; 1: read)
			// bit3 - Mirroring (0: horizontal; 1: vertical)
			// bit4 - CRC control (set during CRC calculation of transfer)
			// bit5 - Always set to '1'
			// bit6 - Read/Write Start (Set to 1 when the drive becomes ready for read/write)
			// bit7 - Interrupt Transfer (0: Transfer without using IRQ; 1: Enable IRQ when
			//        the drive becomes ready)
			m_fds_motor_on = BIT(data, 0);

			if (BIT(data, 1))
				m_fds_head_position = 0;

			if (!(data & 0x40) && m_drive_ready && m_fds_head_position > 2)
				m_fds_head_position -= 2; // ??? is this some sort of compensation??

			m_read_mode = BIT(data, 2);
			set_nt_mirroring(BIT(data, 3) ? PPU_MIRROR_HORZ : PPU_MIRROR_VERT);
			m_drive_ready = data & 0x40;
			m_irq_transfer = BIT(data, 7);
			break;
		case 0x06:
			// external connector
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
			break;
	}
}

READ8_MEMBER(nes_disksys_device::read_ex)
{
	LOG_MMC(("Famicom Disk System read_ex, offset: %04x\n", offset));
	UINT8 ret;

	if (offset >= 0x20 && offset < 0x60)
	{
		// wavetable
	}

	switch (offset)
	{
		case 0x10:
			// $4030 - disk status 0
			// bit0 - Timer Interrupt (1: an IRQ occurred)
			// bit1 - Byte transfer flag (Set to 1 every time 8 bits have been transfered between
			//        the RAM adaptor & disk drive through $4024/$4031; Reset to 0 when $4024,
			//        $4031, or $4030 has been serviced)
			// bit4 - CRC control (0: CRC passed; 1: CRC error)
			// bit6 - End of Head (1 when disk head is on the most inner track)
			// bit7 - Disk Data Read/Write Enable (1 when disk is readable/writable)
			ret = m_fds_status0 | 0x80;
			// clear the disk IRQ detect flag
			m_fds_status0 &= ~0x01;
			break;
		case 0x11:
			// $4031 - data latch
			// don't read data if disk is unloaded
			if (!m_fds_data)
				ret = 0;
			else if (m_fds_current_side && m_read_mode)
			{
				ret = m_fds_data[(m_fds_current_side - 1) * 65500 + m_fds_head_position++];
				if (m_fds_head_position == 65500)
				{
					printf("end of disk reached!\n");
					m_fds_status0 |= 0x40;
					m_fds_head_position -= 2;
				}
			}
			else
				ret = 0;
			break;
		case 0x12:
			// $4032 - disk status 1:
			// bit0 - Disk flag  (0: Disk inserted; 1: Disk not inserted)
			// bit1 - Ready flag (0: Disk ready; 1: Disk not ready)
			// bit2 - Protect flag (0: Not write protected; 1: Write protected or disk ejected)
			if (!m_fds_data)
				ret = 1;
			else if (m_fds_last_side != m_fds_current_side)
			{
				// If we've switched disks, report "no disk" for a few reads
				ret = 1;
				m_fds_count++;
				if (m_fds_count == 50)
				{
					m_fds_last_side = m_fds_current_side;
					m_fds_count = 0;
				}
			}
			else
				ret = (m_fds_current_side == 0) ? 1 : 0; // 0 if a disk is inserted
			break;
		case 0x13:
			// $4033 - external connector (bits 0-6) + battery status (bit 7)
			ret = 0x80;
			break;
		case 0x70:  // $4090 - Volume gain - write through $4080
		case 0x72:  // $4092 - Mod gain - read through $4084
		default:
			ret = 0x00;
			break;
	}

	return ret;
}

//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void nes_disksys_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_IRQ)
	{
		if (m_irq_enable && m_irq_count)
		{
			m_irq_count--;
			if (!m_irq_count)
			{
				m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
				m_irq_enable = 0;
				m_fds_status0 |= 0x01;
				m_irq_count_latch = 0;  // used in Kaettekita Mario Bros
			}
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
		popmessage("Disk set to side %d", m_fds_current_side);
}



// Disk Loading / Unloading

void nes_disksys_device::load_disk(device_image_interface &image)
{
	int header = 0;
	m_fds_sides = 0;

	if (image.length() % 65500)
		header = 0x10;

	m_fds_sides = (image.length() - header) / 65500;

	if (!m_fds_data)
		m_fds_data = std::make_unique<UINT8[]>(m_fds_sides * 65500);

	// if there is an header, skip it
	image.fseek(header, SEEK_SET);
	image.fread(m_fds_data.get(), 65500 * m_fds_sides);
	return;
}

void nes_disksys_device::unload_disk(device_image_interface &image)
{
	/* TODO: should write out changes here as well */
	m_fds_sides =  0;
}
