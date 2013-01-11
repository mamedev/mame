/*******************************************************************************
    SNUG BwG Disk Controller
    Based on WD1773
    Double Density, Double-sided

    * Supports Double Density.
    * As this card includes its own RAM, it does not need to allocate a portion
      of VDP RAM to store I/O buffers.
    * Includes a MM58274C RTC.
    * Support an additional floppy drive, for a total of 4 floppies.

    Reference:
    * BwG Disketten-Controller: Beschreibung der DSR (Description of the DSR)
        <http://home.t-online.de/home/harald.glaab/snug/bwg.pdf>

    +------------------------+
    |   32 KiB EPROM         | --- 1 of 4 pages--> 4000  +------------------+
    |                        |                           |   DSR space      |
    +------------------------+                           |   (Driver)       |
    |   2 KiB  RAM           | --- 1 of 2 pages--> 5c00  +------------------+
    +------------------------+                           |   RAM buffer     |
                                                   5fe0  +------------------+
                                                         |   RTC or WD1773  |
                                                   5fff  +------------------+

    Michael Zapf, September 2010
    January 2012: rewritten as class (MZ)
*******************************************************************************/

#include "emu.h"
#include "peribox.h"
#include "bwg.h"
#include "machine/wd17xx.h"
#include "formats/ti99_dsk.h"
#include "imagedev/flopdrv.h"
#include "machine/mm58274c.h"

#define LOG logerror
#define VERBOSE 1

#define MOTOR_TIMER 1
#define FDC_TAG "wd1773"
#define CLOCK_TAG "mm58274c"

#define BUFFER "ram"

snug_bwg_device::snug_bwg_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: ti_expansion_card_device(mconfig, TI99_BWG, "SNUG BwG Floppy Controller", tag, owner, clock)
{
	m_shortname = "ti99_bwg";
}

/*
    Callback called at the end of DVENA pulse
*/
void snug_bwg_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_DVENA = CLEAR_LINE;
	handle_hold();
}

/*
    Call this when the state of DSKhold or DRQ/IRQ or DVENA change
    Also see ti_fdc.

    Emulation is faulty because the CPU is actually stopped in the midst of
    instruction, at the end of the memory access

    TODO: This has to be replaced by the proper READY handling that is already
    prepared here. (Requires READY handling by the CPU.)
*/
void snug_bwg_device::handle_hold()
{
	line_state state;

	if (m_hold && !m_DRQ && !m_IRQ && (m_DVENA==ASSERT_LINE))
		state = ASSERT_LINE;
	else
		state = CLEAR_LINE;

	m_slot->set_ready((state==CLEAR_LINE)? ASSERT_LINE : CLEAR_LINE);
//  machine().device("maincpu")->execute().set_input_line(INPUT_LINE_HALT, state);
}

/*
    Callback, called from the controller chip whenever DRQ/IRQ state change
*/
WRITE_LINE_MEMBER( snug_bwg_device::intrq_w )
{
	if (VERBOSE>8) LOG("bwg: set irq = %02x\n", state);
	m_IRQ = (state==ASSERT_LINE);

	// Note that INTB is actually not used in the TI-99 family. But the
	// controller asserts the line nevertheless, probably intended for
	// use in another planned TI system
	m_slot->set_intb(state);

	handle_hold();
}

WRITE_LINE_MEMBER( snug_bwg_device::drq_w )
{
	if (VERBOSE>8) LOG("bwg: set drq = %02x\n", state);
	m_DRQ = (state==ASSERT_LINE);
	handle_hold();
}

/*
    Read a byte
    4000 - 5bff: ROM (4 banks)

    rtc disabled:
    5c00 - 5fef: RAM
    5ff0 - 5fff: Controller (f0 = status, f2 = track, f4 = sector, f6 = data)

    rtc enabled:
    5c00 - 5fdf: RAM
    5fe0 - 5fff: Clock (even addr)
*/
READ8Z_MEMBER(snug_bwg_device::readz)
{
	if (m_selected)
	{
		if ((offset & m_select_mask)==m_select_value)
		{
			// 010x xxxx xxxx xxxx
			if ((offset & 0x1c00)==0x1c00)
			{
				// ...1 11xx xxxx xxxx
				if (m_rtc_enabled)
				{
					if ((offset & 0x03e1)==0x03e0)
					{
						// .... ..11 111x xxx0
						*value = mm58274c_r(m_clock, space, (offset & 0x001e) >> 1);
					}
					else
					{
						*value = m_buffer_ram[(m_ram_page<<10) | (offset & 0x03ff)];
					}
				}
				else
				{
					if ((offset & 0x03f9)==0x03f0)
					{
						// .... ..11 1111 0xx0
						// Note that the value is inverted again on the board,
						// so we can drop the inversion
						*value = wd17xx_r(m_controller, space, (offset >> 1)&0x03);
					}
					else
					{
						*value = m_buffer_ram[(m_ram_page<<10) | (offset & 0x03ff)];
					}
				}
			}
			else
			{
				*value = m_dsrrom[(m_rom_page<<13) | (offset & 0x1fff)];
				if (VERBOSE>7) LOG("bwg read dsr: %04x -> %02x\n", offset, *value);
			}
		}
	}
}

/*
    Resets the drive geometry. This is required because the heuristic of
    the default implementation sets the drive geometry to the geometry
    of the medium.
*/
void snug_bwg_device::set_geometry(device_t *drive, floppy_type_t type)
{
	// This assertion may fail when the names of the floppy devices change.
	// Unfortunately, the wd17xx device assumes the floppy drives at root
	// level, so we use an explicitly qualified tag. See peribox.h.
	assert(drive != NULL);
	floppy_drive_set_geometry(drive, type);
}

void snug_bwg_device::set_all_geometries(floppy_type_t type)
{
	set_geometry(machine().device(PFLOPPY_0), type);
	set_geometry(machine().device(PFLOPPY_1), type);
	set_geometry(machine().device(PFLOPPY_2), type);
}

/*
    Write a byte
    4000 - 5bff: ROM, ignore write (4 banks)

    rtc disabled:
    5c00 - 5fef: RAM
    5ff0 - 5fff: Controller (f8 = command, fa = track, fc = sector, fe = data)

    rtc enabled:
    5c00 - 5fdf: RAM
    5fe0 - 5fff: Clock (even addr)
*/
WRITE8_MEMBER(snug_bwg_device::write)
{
	if (m_selected)
	{
		if ((offset & m_select_mask)==m_select_value)
		{
			// 010x xxxx xxxx xxxx
			if ((offset & 0x1c00)==0x1c00)
			{
				// ...1 11xx xxxx xxxx
				if (m_rtc_enabled)
				{
					if ((offset & 0x03e1)==0x03e0)
					{
						// .... ..11 111x xxx0
						mm58274c_w(m_clock, space, (offset & 0x001e) >> 1, data);
					}
					else
					{
						m_buffer_ram[(m_ram_page<<10) | (offset & 0x03ff)] = data;
					}
				}
				else
				{
					if ((offset & 0x03f9)==0x03f8)
					{
						// .... ..11 1111 1xx0
						// Note that the value is inverted again on the board,
						// so we can drop the inversion
						wd17xx_w(m_controller, space, (offset >> 1)&0x03, data);
					}
					else
					{
						m_buffer_ram[(m_ram_page<<10) | (offset & 0x03ff)] = data;
					}
				}
			}
		}
	}
}

/*
    CRU read handler. *=inverted.
    bit 0: DSK4 connected*
    bit 1: DSK1 connected*
    bit 2: DSK2 connected*
    bit 3: DSK3 connected*
    bit 4: Dip 1
    bit 5: Dip 2
    bit 6: Dip 3
    bit 7: Dip 4
*/
void snug_bwg_device::crureadz(offs_t offset, UINT8 *value)
{
	UINT8 reply = 0;

	if ((offset & 0xff00)==m_cru_base)
	{
		if ((offset & 0x00ff)==0)
		{
			// Assume that we have 4 drives connected
			// If we want to do that properly, we need to check the actually
			// available drives (not the images!). But why should we connect less?
			reply = 0x00;

			// DIP switches. Note that a closed switch means 0
			// xx01 1111   11 = only dsk1; 10 = 1+2, 01=1/2/3, 00=1-4

			if (m_dip1 != 0) reply |= 0x10;
			if (m_dip2 != 0) reply |= 0x20;
			reply |= (m_dip34 << 6);
			*value = ~reply;
		}
		else
			*value = 0;
	}
}

void snug_bwg_device::cruwrite(offs_t offset, UINT8 data)
{
	int drive, drivebit;

	if ((offset & 0xff00)==m_cru_base)
	{
		int bit = (offset >> 1) & 0x0f;
		switch (bit)
		{
		case 0:
			/* (De)select the card. Indicated by a LED on the board. */
			m_selected = (data != 0);
			if (VERBOSE>4) LOG("bwg: Map DSR = %d\n", m_selected);
			break;

		case 1:
			/* Activate motor */
			if (data && !m_strobe_motor)
			{   /* on rising edge, set motor_running for 4.23s */
				m_DVENA = ASSERT_LINE;
				handle_hold();
				m_motor_on_timer->adjust(attotime::from_msec(4230));
			}
			m_strobe_motor = (data != 0);
			break;

		case 2:
			/* Set disk ready/hold (bit 2) */
			// 0: ignore IRQ and DRQ
			// 1: TMS9900 is stopped until IRQ or DRQ are set
			// OR the motor stops rotating - rotates for 4.23s after write
			// to CRU bit 1
			// This is not emulated and could cause the TI99 to lock up
			m_hold = (data != 0);
			handle_hold();
			break;

		case 4:
		case 5:
		case 6:
		case 8:
			/* Select drive 0-2 (DSK1-DSK3) (bits 4-6) */
			/* Select drive 3 (DSK4) (bit 8) */
			drive = (bit == 8) ? 3 : (bit - 4);     /* drive # (0-3) */
			drivebit = 1<<drive;

			if (data != 0)
			{
				if ((m_DSEL & drivebit) == 0)           /* select drive */
				{
					if (m_DSEL != 0)
						logerror("bwg: Multiple drives selected, %02x\n", m_DSEL);
					m_DSEL |= drivebit;
					wd17xx_set_drive(m_controller, drive);
				}
			}
			else
				m_DSEL &= ~drivebit;
			break;

		case 7:
			/* Select side of disk (bit 7) */
			m_SIDE = data;
			wd17xx_set_side(m_controller, m_SIDE);
			break;

		case 10:
			/* double density enable (active low) */
			wd17xx_dden_w(m_controller, (data != 0) ? ASSERT_LINE : CLEAR_LINE);
			break;

		case 11:
			/* EPROM A13 */
			if (data != 0)
				m_rom_page |= 1;
			else
				m_rom_page &= 0xfe;  // 11111110
			break;

		case 13:
			/* RAM A10 */
			m_ram_page = data;
			break;

		case 14:
			/* Override FDC with RTC (active high) */
			m_rtc_enabled = (data != 0);
			break;

		case 15:
			/* EPROM A14 */
			if (data != 0)
				m_rom_page |= 2;
			else
				m_rom_page &= 0xfd; // 11111101
			break;

		case 3:
		case 9:
		case 12:
			/* Unused (bit 3, 9 & 12) */
			break;
		}
	}
}

void snug_bwg_device::device_start(void)
{
	if (VERBOSE>5) LOG("bwg: BWG start\n");
	m_dsrrom = memregion(DSRROM)->base();
	m_buffer_ram = memregion(BUFFER)->base();
	m_motor_on_timer = timer_alloc(MOTOR_TIMER);
	m_controller = subdevice(FDC_TAG);
	m_clock = subdevice(CLOCK_TAG);
	m_cru_base = 0x1100;
}

void snug_bwg_device::device_reset()
{
	if (VERBOSE>5) LOG("bwg: BWG reset\n");

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

	m_strobe_motor = false;
	m_DVENA = CLEAR_LINE;
	ti99_set_80_track_drives(FALSE);
	floppy_type_t type = FLOPPY_STANDARD_5_25_DSDD_40;
	set_all_geometries(type);
	m_DRQ = false;
	m_IRQ = false;
	m_hold = false;
	m_rtc_enabled = false;
	m_selected = false;

	m_dip1 = ioport("BWGDIP1")->read();
	m_dip2 = ioport("BWGDIP2")->read();
	m_dip34 = ioport("BWGDIP34")->read();

	m_rom_page = 0;
	m_ram_page = 0;
}

const wd17xx_interface bwg_wd17xx_interface =
{
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, snug_bwg_device, intrq_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, snug_bwg_device, drq_w),
	{ PFLOPPY_0, PFLOPPY_1, PFLOPPY_2, PFLOPPY_3 }
};

static const mm58274c_interface floppy_mm58274c_interface =
{
	1,  /*  mode 24*/
	0   /*  first day of week */
};

INPUT_PORTS_START( bwg_fdc )
	PORT_START( "BWGDIP1" )
	PORT_DIPNAME( 0x01, 0x00, "BwG step rate" )
		PORT_DIPSETTING( 0x00, "6 ms")
		PORT_DIPSETTING( 0x01, "20 ms")

	PORT_START( "BWGDIP2" )
	PORT_DIPNAME( 0x01, 0x00, "BwG date/time display" )
		PORT_DIPSETTING( 0x00, "Hide")
		PORT_DIPSETTING( 0x01, "Show")

	PORT_START( "BWGDIP34" )
	PORT_DIPNAME( 0x03, 0x00, "BwG drives" )
		PORT_DIPSETTING( 0x00, "DSK1 only")
		PORT_DIPSETTING( 0x01, "DSK1-DSK2")
		PORT_DIPSETTING( 0x02, "DSK1-DSK3")
		PORT_DIPSETTING( 0x03, "DSK1-DSK4")
INPUT_PORTS_END

MACHINE_CONFIG_FRAGMENT( bwg_fdc )
	MCFG_WD1773_ADD(FDC_TAG, bwg_wd17xx_interface )
	MCFG_MM58274C_ADD(CLOCK_TAG, floppy_mm58274c_interface)
MACHINE_CONFIG_END

ROM_START( bwg_fdc )
	ROM_REGION(0x8000, DSRROM, 0)
	ROM_LOAD("bwg.bin", 0x0000, 0x8000, CRC(06f1ec89) SHA1(6ad77033ed268f986d9a5439e65f7d391c4b7651)) /* BwG disk DSR ROM */
	ROM_REGION(0x0800, BUFFER, 0)  /* BwG RAM buffer */
	ROM_FILL(0x0000, 0x0400, 0x00)
ROM_END

machine_config_constructor snug_bwg_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( bwg_fdc );
}

const rom_entry *snug_bwg_device::device_rom_region() const
{
	return ROM_NAME( bwg_fdc );
}

ioport_constructor snug_bwg_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(bwg_fdc);
}

const device_type TI99_BWG = &device_creator<snug_bwg_device>;
