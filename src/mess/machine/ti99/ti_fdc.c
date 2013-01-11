/****************************************************************************

    TI-99 Standard Floppy Disk Controller Card
    Based on WD1771
    Single Density, Double-sided

    Michael Zapf
    September 2010
    January 2012: rewritten as class (MZ)

****************************************************************************/

#include "emu.h"
#include "peribox.h"
#include "ti_fdc.h"
#include "machine/wd17xx.h"
#include "formats/ti99_dsk.h"

#define LOG logerror
#define VERBOSE 1

#define fdc_IRQ 1
#define fdc_DRQ 2

#define TI_FDC_TAG "ti_dssd_controller"

#define FDC_TAG "wd1771"

#define MOTOR_TIMER 1

const wd17xx_interface ti_wd17xx_interface =
{
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, ti_fdc_device, intrq_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, ti_fdc_device, drq_w),
	{ PFLOPPY_0, PFLOPPY_1, PFLOPPY_2, NULL }
};

ti_fdc_device::ti_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
: ti_expansion_card_device(mconfig, TI99_FDC, "TI-99 Standard DSSD Floppy Controller", tag, owner, clock)
{
	m_shortname = "ti99_fdc";
}

/*
    callback called at the end of DVENA pulse
*/
void ti_fdc_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_DVENA = CLEAR_LINE;
	handle_hold();
}

READ8Z_MEMBER(ti_fdc_device::readz)
{
	if (m_selected)
	{
		if ((offset & m_select_mask)==m_select_value)
		{
			// only use the even addresses from 1ff0 to 1ff6.
			// Note that data is inverted.
			// 0101 1111 1111 0xx0
			UINT8 reply = 0;

			if ((offset & 0x1ff9)==0x1ff0)
			{
				reply = wd17xx_r(m_controller, space, (offset >> 1)&0x03);
			}
			else
			{
				reply = m_dsrrom[offset & 0x1fff];
			}
			*value = reply;
			if (VERBOSE>5) LOG("ti_fdc: %04x -> %02x\n", offset & 0xffff, *value);
		}
	}
}

WRITE8_MEMBER(ti_fdc_device::write)
{
	if (m_selected)
	{
		if ((offset & m_select_mask)==m_select_value)
		{
			if (VERBOSE>5) LOG("ti_fdc: %04x <- %02x\n", offset & 0xffff, data);
			// only use the even addresses from 1ff8 to 1ffe.
			// Note that data is inverted.
			// 0101 1111 1111 1xx0
			if ((offset & 0x1ff9)==0x1ff8)
			{
				wd17xx_w(m_controller, space, (offset >> 1)&0x03, data);
			}
		}
	}
}

/*
    The CRU read handler.
    bit 0: HLD pin
    bit 1-3: drive n active
    bit 4: 0: motor strobe on
    bit 5: always 0
    bit 6: always 1
    bit 7: selected side
*/
void ti_fdc_device::crureadz(offs_t offset, UINT8 *value)
{
	if ((offset & 0xff00)==m_cru_base)
	{
		int addr = offset & 0x07;
		UINT8 reply = 0;
		if (addr == 0)
		{
			// deliver bits 0-7
			// TODO: HLD pin
			// The DVENA state is returned inverted
			if (m_DVENA==ASSERT_LINE) reply |= ((m_DSEL)<<1);
			else reply |= 0x10;
			reply |= 0x40;
			if (m_SIDSEL) reply |= 0x80;
		}
		*value = reply;
	}
}

void ti_fdc_device::cruwrite(offs_t offset, UINT8 data)
{
	int drive, drivebit;

	if ((offset & 0xff00)==m_cru_base)
	{
		int bit = (offset >> 1) & 0x07;
		switch (bit)
		{
		case 0:
			/* (De)select the card. Indicated by a LED on the board. */
			m_selected = (data!=0);
			if (VERBOSE>4) LOG("ti_fdc: Map DSR = %d\n", m_selected);
			break;
		case 1:
			/* Activate motor */
			if (data && !m_strobe_motor)
			{   /* on rising edge, set motor_running for 4.23s */
				m_DVENA = ASSERT_LINE;
				handle_hold();
				m_motor_on_timer->adjust(attotime::from_msec(4230));
			}
			m_strobe_motor = (data!=0);
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

		case 3:
			/* Load disk heads (HLT pin) (bit 3). Not implemented. */
			break;

		case 4:
		case 5:
		case 6:
			/* Select drive X (bits 4-6) */
			drive = bit-4;                  /* drive # (0-2) */
			drivebit = 1<<drive;

			if (data)
			{
				if ((m_DSEL & drivebit)!=0)         /* select drive */
				{
					if (m_DSEL != 0)
						LOG("ti_fdc: Multiple drives selected, %02x\n", m_DSEL);
					m_DSEL |= drivebit;
					wd17xx_set_drive(m_controller, drive);
					/*wd17xx_set_side(DSKside);*/
				}
			}
			else
				m_DSEL &= ~drivebit;
			break;

		case 7:
			/* Select side of disk (bit 7) */
			m_SIDSEL = data;
			wd17xx_set_side(m_controller, data);
			break;
		}
	}
}

/*
    Call this when the state of DSKhold or DRQ/IRQ or DVENA change

    Emulation is faulty because the CPU is actually stopped in the midst of
    instruction, at the end of the memory access

    TODO: This has to be replaced by the proper READY handling that is already
    prepared here. (Requires READY handling by the CPU.)
*/
void ti_fdc_device::handle_hold()
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
    Resets the drive geometry. This is required because the heuristic of
    the default implementation sets the drive geometry to the geometry
    of the medium.
*/
void ti_fdc_device::set_geometry(device_t *drive, floppy_type_t type)
{
	// This assertion may fail when the names of the floppy devices change.
	// Unfortunately, the wd17xx device assumes the floppy drives at root
	// level, so we use an explicitly qualified tag. See peribox.h.
	assert (drive!=NULL);
	floppy_drive_set_geometry(drive, type);
}

void ti_fdc_device::set_all_geometries(floppy_type_t type)
{
	set_geometry(machine().device(PFLOPPY_0), type);
	set_geometry(machine().device(PFLOPPY_1), type);
	set_geometry(machine().device(PFLOPPY_2), type);
}

/*
    Callback, called from the controller chip whenever DRQ/IRQ state change
*/
WRITE_LINE_MEMBER( ti_fdc_device::intrq_w )
{
	if (VERBOSE>8) LOG("ti_fdc: set irq = %02x\n", state);
	m_IRQ = (state==ASSERT_LINE);
	// Note that INTB is actually not used in the TI-99 family. But the
	// controller asserts the line nevertheless, probably intended for
	// use in another planned TI system
	m_slot->set_intb(state);

	handle_hold();
}

WRITE_LINE_MEMBER( ti_fdc_device::drq_w )
{
	if (VERBOSE>8) LOG("ti_fdc: set drq = %02x\n", state);
	m_DRQ = (state == ASSERT_LINE);
	handle_hold();
}

void ti_fdc_device::device_start(void)
{
	if (VERBOSE>5) LOG("ti_fdc: TI FDC start\n");
	m_dsrrom = memregion(DSRROM)->base();
	m_motor_on_timer = timer_alloc(MOTOR_TIMER);
	m_controller = subdevice(FDC_TAG);

	m_cru_base = 0x1100;
}

void ti_fdc_device::device_reset(void)
{
	if (VERBOSE>5) LOG("ti_fdc: TI FDC reset\n");
	m_DSEL = 0;
	m_SIDSEL = 0;
	m_DVENA = CLEAR_LINE;
	m_strobe_motor = false;
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
	m_DRQ = false;
	m_IRQ = false;
	m_hold = false;
	m_selected = false;

	ti99_set_80_track_drives(FALSE);
	floppy_type_t type = FLOPPY_STANDARD_5_25_DSDD_40;
	set_all_geometries(type);
}

MACHINE_CONFIG_FRAGMENT( ti_fdc )
	MCFG_FD1771_ADD(FDC_TAG, ti_wd17xx_interface )
MACHINE_CONFIG_END

ROM_START( ti_fdc )
	ROM_REGION(0x2000, DSRROM, 0)
	ROM_LOAD("disk.bin", 0x0000, 0x2000, CRC(8f7df93f) SHA1(ed91d48c1eaa8ca37d5055bcf67127ea51c4cad5)) /* TI disk DSR ROM */
ROM_END

machine_config_constructor ti_fdc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( ti_fdc );
}

const rom_entry *ti_fdc_device::device_rom_region() const
{
	return ROM_NAME( ti_fdc );
}

const device_type TI99_FDC = &device_creator<ti_fdc_device>;
