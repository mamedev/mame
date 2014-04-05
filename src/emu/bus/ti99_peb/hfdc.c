// license:MAME|LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Myarc Hard and Floppy Disk Controller
    based on SMC9234
    High Density, Double-sided

    FIXME: HFDC does not work at CRU addresses other than 1100
    (test shows
        hfdc: reado 41f5 (00): 00
        hfdc: reado 41f4 (00): 00 -> wrong rom page? (should be (02)))

    Michael Zapf

    February 2012: Rewritten as class

*****************************************************************************/

#include "hfdc.h"
#include "imagedev/flopdrv.h"
#include "formats/ti99_dsk.h"       // Format

#define BUFFER "ram"
#define FDC_TAG "hdc9234"
#define CLOCK_TAG "mm58274c"

#define MOTOR_TIMER 1

#define HFDC_MAX_FLOPPY 4
#define HFDC_MAX_HARD 4

#define TAPE_ADDR   0x0fc0
#define HDC_R_ADDR  0x0fd0
#define HDC_W_ADDR  0x0fd2
#define CLK_ADDR    0x0fe0
#define RAM_ADDR    0x1000

#define VERBOSE 1
#define LOG logerror

myarc_hfdc_device::myarc_hfdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ti_expansion_card_device(mconfig, TI99_HFDC, "Myarc Hard and Floppy Disk Controller", tag, owner, clock, "ti99_hfdc", __FILE__),
		m_hdc9234(*this, FDC_TAG),
		m_clock(*this, CLOCK_TAG)
{
}

/*
    read a byte in disk DSR space
    HFDC manual, p. 44
    Memory map as seen by the 99/4A PEB

    0x4000 - 0x4fbf one of four possible ROM pages
    0x4fc0 - 0x4fcf tape select
    0x4fd0 - 0x4fdf disk controller select
    0x4fe0 - 0x4fff time chip select

    0x5000 - 0x53ff static RAM page 0x10
    0x5400 - 0x57ff static RAM page any of 32 pages
    0x5800 - 0x5bff static RAM page any of 32 pages
    0x5c00 - 0x5fff static RAM page any of 32 pages
*/
READ8Z_MEMBER(myarc_hfdc_device::readz)
{
	if (m_selected && ((offset & m_select_mask)==m_select_value))
	{
		/* DSR region */
		if ((offset & 0x1000)==0x0000)
		{
			if ((offset & 0x0fc0)==0x0fc0)
			{
				// Tape: 4fc0...4fcf
				if ((offset & 0x1ff0)==TAPE_ADDR)
				{
					if (VERBOSE>0) LOG("ti99: HFDC: Tape support not available (access to address %04x)\n", offset&0xffff);
					return;
				}

				// HDC9234: 4fd0..4fdf / read: 4fd0,4 (mirror 8,c)
				// read: 0100 1111 1101 xx00
				if ((offset & 0x1ff3)==HDC_R_ADDR)
				{
					if (!space.debugger_access()) *value = m_hdc9234->read(space, (offset>>2)&1, mem_mask);
					if (VERBOSE>7) LOG("hfdc: read %04x: %02x\n",  offset & 0xffff, *value);
					return;
				}

				if ((offset & 0x1fe1)==CLK_ADDR)
				{
					if (!space.debugger_access()) *value = m_clock->read(space, (offset & 0x001e) >> 1);
					if (VERBOSE>7) LOG("hfdc: read from clock address %04x: %02x\n", offset & 0xffff, *value);
					return;
				}
			}
			else
			{
				// ROM
				*value = m_dsrrom[(m_rom_page << 12) | (offset & 0x0fff)];
				if (VERBOSE>7) LOG("hfdc: reado %04x (%02x): %02x\n",  offset & 0xffff, m_rom_page, *value);
				return;
			}
		}

		// RAM: 0101 xxxx xxxx xxxx
		if ((offset & 0x1000)==RAM_ADDR)
		{
			// 0101 00xx xxxx xxxx  static 0x08
			// 0101 01xx xxxx xxxx  bank 1
			// 0101 10xx xxxx xxxx  bank 2
			// 0101 11xx xxxx xxxx  bank 3
			int bank = (offset & 0x0c00) >> 10;
			*value = m_buffer_ram[(m_ram_page[bank]<<10) | (offset & 0x03ff)];
			if (VERBOSE>7) LOG("hfdc: read %04x (%02x): %02x\n", offset & 0xffff, m_ram_page[bank], *value);
		}
	}
}

/*
    Write a byte to the controller.
*/
WRITE8_MEMBER( myarc_hfdc_device::write )
{
	if (m_selected && ((offset & m_select_mask)==m_select_value))
	{
		// Tape: 4fc0...4fcf
		if ((offset & 0x1ff0)==TAPE_ADDR)
		{
			if (VERBOSE>0) LOG("ti99: HFDC: Tape support not available (access to address %04x)\n", offset & 0xffff);
			return;
		}

		// HDC9234: 4fd0..4fdf / write: 4fd2,6 (mirror a,e)
		// write: 0100 1111 1101 xx10
		if ((offset & 0x1ff3)==HDC_W_ADDR)
		{
			if (VERBOSE>7) LOG("hfdc: write to controller address %04x: %02x\n", offset & 0xffff, data);
			if (!space.debugger_access()) m_hdc9234->write(space, (offset>>2)&1, data, mem_mask);
			return;
		}

		if ((offset & 0x1fe1)==CLK_ADDR)
		{
			if (VERBOSE>7) LOG("hfdc: write to clock address %04x: %02x\n", offset & 0xffff, data);
			if (!space.debugger_access()) m_clock->write(space, (offset & 0x001e) >> 1, data);
			return;
		}

		// RAM: 0101 xxxx xxxx xxxx
		if ((offset & 0x1000)==RAM_ADDR)
		{
			// 0101 00xx xxxx xxxx  static 0x08
			// 0101 01xx xxxx xxxx  bank 1
			// 0101 10xx xxxx xxxx  bank 2
			// 0101 11xx xxxx xxxx  bank 3
			int bank = (offset & 0x0c00) >> 10;
			if (VERBOSE>7) LOG("hfdc: WRITE %04x (%02x): %02x\n", (offset & 0xffff), m_ram_page[bank], data);
			m_buffer_ram[(m_ram_page[bank]<<10) | (offset & 0x03ff)] = data;
		}
	}
}

READ8Z_MEMBER(myarc_hfdc_device::crureadz)
{
	UINT8 reply;
	if ((offset & 0xff00)==m_cru_base)
	{
		if ((offset & 0x00ff)==0)  /* CRU bits 0-7 */
		{
			/* CRU bits */
			if (m_cru_select)
			{
				// DIP switches.  Logic levels are inverted (on->0, off->1).  CRU
				// bit order is the reverse of DIP-switch order, too (dip 1 -> bit 7,
				// dip 8 -> bit 0).
				// MZ: 00 should not be used since there is a bug in the
				// DSR of the HFDC which causes problems with SD disks
				// (controller tries DD and then fails to fall back to SD) */
				reply = ~(ioport("HFDCDIP")->read());
			}
			else
			{
				reply = 0;

				if (m_irq)          reply |= 0x01;
				if (m_dip)          reply |= 0x02;
				if (m_motor_running)    reply |= 0x04;
			}
			*value = reply;
		}
		else /* CRU bits 8+ */
			*value = 0;
		if (VERBOSE>7) LOG("hfdc: reading from CRU %04x: %02x\n", offset, *value);
	}
}

/*
    Write disk CRU interface
    HFDC manual p. 43

    CRU rel. address    Definition                      Active
    00                  Device Service Routine Select   HIGH
    02                  Reset to controller chip        LOW
    04                  Floppy Motor on / CD0           HIGH
    06                  ROM page select 0 / CD1
    08                  ROM page select 1 / CRU input select
    12/4/6/8/A          RAM page select at 0x5400
    1C/E/0/2/4          RAM page select at 0x5800
    26/8/A/C/E          RAM page select at 0x5C00

    RAM bank select: bank 0..31; 12 = LSB (accordingly for other two areas)
    ROM bank select: bank 0..3; 06 = MSB, 07 = LSB
    Bit number = (CRU_rel_address - base_address)/2
    CD0 and CD1 are Clock Divider selections for the Floppy Data Separator (FDC9216)
*/
WRITE8_MEMBER(myarc_hfdc_device::cruwrite)
{
	if ((offset & 0xff00)==m_cru_base)
	{
		if (VERBOSE>7) LOG("hfdc: writing to CRU %04x: %02x\n", offset, data);
		int bit = (offset >> 1) & 0x1f;

		if (bit >= 9 && bit < 24)
		{
			if (data)
				// we leave index 0 unchanged; modify indices 1-3
				m_ram_page[(bit-4)/5] |= 1 << ((bit-9)%5);
			else
				m_ram_page[(bit-4)/5] &= ~(1 << ((bit-9)%5));

			return;
		}

		switch (bit)
		{
		case 0:
			m_selected = (data!=0);
			if (VERBOSE>7) LOG("hfdc: selected = %d\n", m_selected);
			break;

		case 1:
			if (data==0) m_hdc9234->reset();  // active low
			break;

		case 2:
			m_CD0 = data;
			if (data != 0) m_motor_running = true;
			else
			{
				if (m_motor_running)
				{
					m_motor_on_timer->adjust(attotime::from_msec(4230));
				}
			}
			break;

		case 3:
			m_CD1 = data;
			if (data!=0) m_rom_page |= 2;
			else m_rom_page &= 0xfd;
			break;

		case 4:
			m_cru_select = (data!=0);
			if (data!=0) m_rom_page |= 1;
			else m_rom_page &= 0xfe;
			break;

		default:
			if (VERBOSE>1) LOG("ti99: HFDC: Attempt to set undefined CRU bit %d\n", bit);
		}
	}
}


int myarc_hfdc_device::slog2(int value)
{
	int i=-1;
	while (value!=0)
	{
		value >>= 1;
		i++;
	}
	return i;
}

READ8_MEMBER( myarc_hfdc_device::auxbus_in )
{
	UINT8 reply = 0;
	int index = 0;

	if ((m_output1_latch & 0xf0)==0)
	{
		if (VERBOSE>4) LOG("hfdc: no drive selected, returning 0\n");
		// No HD and no floppy
		return 0; /* is that the correct default value? */
	}

	// If a floppy is selected, we have one line set among the four programmable outputs.
	// Floppy selected <=> latch & 0x10 != 0; floppy number in last four bits
	if ((m_output1_latch & 0x10)!=0)
	{
		index = slog2(m_output1_latch & 0x0f);
		if (index==-1) return 0;

		/* Get floppy status. */
		if (m_floppy_unit[index]->floppy_drive_get_flag_state(FLOPPY_DRIVE_INDEX) == FLOPPY_DRIVE_INDEX)
			reply |= DS_INDEX;
		if (m_floppy_unit[index]->floppy_tk00_r() == CLEAR_LINE)
			reply |= DS_TRK00;
		if (m_floppy_unit[index]->floppy_wpt_r() == CLEAR_LINE)
			reply |= DS_WRPROT;

		/* if (image_exists(disk_img)) */

		reply |= DS_READY;  /* Floppies don't have a READY line; line is pulled up */
		reply |= DS_SKCOM;  /* Same here. */
		if (VERBOSE>4) LOG("hfdc: floppy selected, returning status %02x\n", reply);
	}
	else // one of the first three lines must be selected
	{
		UINT8 state;
		index = slog2((m_output1_latch>>4) & 0x0f)-1;
		mfm_harddisk_device *hd = m_harddisk_unit[index];
		state = hd->get_status();

		if (state & MFMHD_TRACK00)      reply |= DS_TRK00;
		if (state & MFMHD_SEEKCOMP)     reply |= DS_SKCOM;
		if (state & MFMHD_WRFAULT)      reply |= DS_WRFAULT;
		if (state & MFMHD_INDEX)        reply |= DS_INDEX;
		if (state & MFMHD_READY)        reply |= DS_READY;
		if (VERBOSE>4) LOG("hfdc: hd selected, returning status %02x\n", reply);
	}

	return reply;
}

WRITE8_MEMBER( myarc_hfdc_device::auxbus_out )
{
	int index;
	switch (offset)
	{
	case INPUT_STATUS:
		if (VERBOSE>1) LOG("ti99: HFDC: Invalid operation: S0=S1=0, but tried to write (expected: read drive status)\n");
		break;

	case OUTPUT_DMA_ADDR:
		/* Value is dma address byte. Shift previous contents to the left. */
		m_dma_address = ((m_dma_address << 8) + (data&0xff))&0xffffff;
		break;

	case OUTPUT_OUTPUT1:
		// value is output1
		// The HFDC interprets the value as follows:
		// WDS = Winchester Drive System, old name for hard disk
		// +------+------+------+------+------+------+------+------+
		// | WDS3 | WDS2 | WDS1 | DSKx | x=4  | x=3  | x=2  | x=1  |
		// +------+------+------+------+------+------+------+------+
		// Accordingly, drive 0 is always the floppy; selected by the low nibble
		if (data & 0x10)
		{
			// Floppy selected
			index = slog2(data & 0x0f);
			if (index>=0) m_hdc9234->connect_floppy_drive(m_floppy_unit[index]);
		}
		else
		{
			// HD selected
			index = slog2((data>>4) & 0x0f);
			if (index>=0) m_hdc9234->connect_hard_drive(m_harddisk_unit[index-1]);
		}

		m_output1_latch = data;
		break;

	case OUTPUT_OUTPUT2:
		/* value is output2 */
		m_output2_latch = data;
		break;
	}
}

/*
    Read a byte from buffer in DMA mode
*/
READ8_MEMBER( myarc_hfdc_device::read_buffer )
{
	UINT8 value = m_buffer_ram[m_dma_address & 0x7fff];
	m_dma_address++;
	return value;
}

/*
    Write a byte to buffer in DMA mode
*/
WRITE8_MEMBER( myarc_hfdc_device::write_buffer )
{
	m_buffer_ram[m_dma_address & 0x7fff] = data;
	m_dma_address++;
}

/*
    Called whenever the state of the sms9234 interrupt pin changes.
*/
WRITE_LINE_MEMBER( myarc_hfdc_device::intrq_w )
{
	m_irq = state;

	// Set INTA*
	// Signal from SMC is active high, INTA* is active low; board inverts signal
	// Anyway, we keep with ASSERT_LINE and CLEAR_LINE
	m_slot->set_inta(state);
}


/*
    Called whenever the state of the sms9234 DMA in progress changes.
*/
WRITE_LINE_MEMBER( myarc_hfdc_device::dip_w )
{
	m_dip = state;
}

/*
    Callback called at the end of DVENA pulse
*/
void myarc_hfdc_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_motor_running = false;
	if (VERBOSE>6) LOG("hfdc: motor off\n");
}

const smc92x4_interface ti99_smc92x4_interface =
{
	FALSE,      /* do not use the full track layout */
};

static const mm58274c_interface floppy_mm58274c_interface =
{
	1,  /*  mode 24*/
	0   /*  first day of week */
};

MACHINE_CONFIG_FRAGMENT( ti99_hfdc )
	MCFG_SMC92X4_ADD(FDC_TAG, ti99_smc92x4_interface )
	MCFG_SMC92X4_INTRQ_CALLBACK(WRITELINE(myarc_hfdc_device, intrq_w))
	MCFG_SMC92X4_DIP_CALLBACK(WRITELINE(myarc_hfdc_device, dip_w))
	MCFG_SMC92X4_AUXBUS_OUT_CALLBACK(WRITE8(myarc_hfdc_device, auxbus_out))
	MCFG_SMC92X4_AUXBUS_IN_CALLBACK(READ8(myarc_hfdc_device, auxbus_in))
	MCFG_SMC92X4_DMA_IN_CALLBACK(READ8(myarc_hfdc_device, read_buffer))
	MCFG_SMC92X4_DMA_OUT_CALLBACK(WRITE8(myarc_hfdc_device, write_buffer))
	MCFG_MM58274C_ADD(CLOCK_TAG, floppy_mm58274c_interface)
MACHINE_CONFIG_END

ROM_START( ti99_hfdc )
	ROM_REGION(0x4000, DSRROM, 0)
	ROM_LOAD("hfdc.bin", 0x0000, 0x4000, CRC(66fbe0ed) SHA1(11df2ecef51de6f543e4eaf8b2529d3e65d0bd59)) /* HFDC disk DSR ROM */
	ROM_REGION(0x8000, BUFFER, 0)  /* HFDC RAM buffer 32 KiB */
	ROM_FILL(0x0000, 0x8000, 0x00)
ROM_END

INPUT_PORTS_START( ti99_hfdc )
	PORT_START( "CRUHFDC" )
	PORT_DIPNAME( 0x1f00, 0x1100, "HFDC CRU base" )
		PORT_DIPSETTING( 0x1000, "1000" )
		PORT_DIPSETTING( 0x1100, "1100" )
		PORT_DIPSETTING( 0x1200, "1200" )
		PORT_DIPSETTING( 0x1300, "1300" )
		PORT_DIPSETTING( 0x1400, "1400" )
		PORT_DIPSETTING( 0x1500, "1500" )
		PORT_DIPSETTING( 0x1600, "1600" )
		PORT_DIPSETTING( 0x1700, "1700" )
		PORT_DIPSETTING( 0x1800, "1800" )
		PORT_DIPSETTING( 0x1900, "1900" )
		PORT_DIPSETTING( 0x1a00, "1A00" )
		PORT_DIPSETTING( 0x1b00, "1B00" )
		PORT_DIPSETTING( 0x1c00, "1C00" )
		PORT_DIPSETTING( 0x1d00, "1D00" )
		PORT_DIPSETTING( 0x1e00, "1E00" )
		PORT_DIPSETTING( 0x1f00, "1F00" )

	PORT_START( "HFDCDIP" )
	PORT_DIPNAME( 0xff, 0x55, "HFDC drive config" )
		PORT_DIPSETTING( 0x00, "40 track, 16 ms")
		PORT_DIPSETTING( 0xaa, "40 track, 8 ms")
		PORT_DIPSETTING( 0x55, "80 track, 2 ms")
		PORT_DIPSETTING( 0xff, "80 track HD, 2 ms")

	PORT_START( "DRVSPD" )
	PORT_CONFNAME( 0x01, 0x01, "Floppy and HD speed" )
		PORT_CONFSETTING( 0x00, "No delay")
		PORT_CONFSETTING( 0x01, "Realistic")
INPUT_PORTS_END

void myarc_hfdc_device::device_start()
{
	if (VERBOSE>5) LOG("hfdc: start\n");
	m_dsrrom = memregion(DSRROM)->base();
	m_buffer_ram = memregion(BUFFER)->base();

	m_motor_on_timer = timer_alloc(MOTOR_TIMER);

	// The HFDC does not use READY; it has on-board RAM for DMA
}

void myarc_hfdc_device::device_reset()
{
	if (VERBOSE>5) LOG("hfdc: reset\n");
	if (m_genmod)
	{
		m_select_mask = 0x1fe000;
		m_select_value = 0x174000;
	}
	else
	{
		m_select_mask = 0x7e000;
		m_select_value = 0x74000;
	}

	m_cru_base = ioport("CRUHFDC")->read();

	// Resetting values
	m_rom_page = 0;

	m_ram_page[0] = 0x08;   // static page 0x08
	for (int i=1; i < 4; i++) m_ram_page[i] = 0;
	m_dma_address = 0;
	m_output1_latch = m_output2_latch = 0;
	m_dip = m_irq = false;
	m_cru_select = false;
	m_CD0 = m_CD1 = 0;
	m_motor_running = false;
	m_selected = false;

	// Find the floppies and hard disks
	m_floppy_unit[0] = static_cast<legacy_floppy_image_device *>(m_slot->get_drive(FLOPPY_0));
	m_floppy_unit[1] = static_cast<legacy_floppy_image_device *>(m_slot->get_drive(FLOPPY_1));
	m_floppy_unit[2] = static_cast<legacy_floppy_image_device *>(m_slot->get_drive(FLOPPY_2));
	m_floppy_unit[3] = static_cast<legacy_floppy_image_device *>(m_slot->get_drive(FLOPPY_3));

	m_harddisk_unit[0] = static_cast<mfm_harddisk_device *>(m_slot->get_drive(MFMHD_0));
	m_harddisk_unit[1] = static_cast<mfm_harddisk_device *>(m_slot->get_drive(MFMHD_1));
	m_harddisk_unit[2] = static_cast<mfm_harddisk_device *>(m_slot->get_drive(MFMHD_2));

	if (ioport("HFDCDIP")->read()&0x55)
		ti99_set_80_track_drives(TRUE);
	else
		ti99_set_80_track_drives(FALSE);

	m_hdc9234->set_timing(ioport("DRVSPD")->read()!=0);

	floppy_type_t floptype = FLOPPY_STANDARD_5_25_DSHD;
	// Again, need to re-adjust the floppy geometries
	for (int i=0; i < HFDC_MAX_FLOPPY; i++)
	{
		if (m_floppy_unit[i] != NULL)
		{
			//  floppy_drive_set_controller(card->floppy_unit[i], device);
			//  floppy_drive_set_index_pulse_callback(floppy_unit[i], smc92x4_index_pulse_callback);
			m_floppy_unit[i]->floppy_drive_set_geometry(floptype);
		}
	}

	if (VERBOSE>2) LOG("hfdc: CRU base = %04x\n", m_cru_base);
	// TODO: Check how to make use of   floppy_mon_w(w->drive, CLEAR_LINE);
}

machine_config_constructor myarc_hfdc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ti99_hfdc );
}

const rom_entry *myarc_hfdc_device::device_rom_region() const
{
	return ROM_NAME( ti99_hfdc );
}

ioport_constructor myarc_hfdc_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ti99_hfdc);
}

const device_type TI99_HFDC = &device_creator<myarc_hfdc_device>;
