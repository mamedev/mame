// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    corvfdc02.c

    Implemention of the Corvus Systems CORVUS02 floppy controller
    aka the "Buffered Floppy Controller"

    Boot PROM 0.8 says 8" SSDD or 5.25" DSDD; we stick with 5.25" here
    and let the FDC01 handle 8".

*********************************************************************/

#include "corvfdc02.h"
#include "formats/concept_dsk.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_CORVFDC02 = &device_creator<a2bus_corvfdc02_device>;

#define FDC02_ROM_REGION    "fdc02_rom"
#define FDC02_FDC_TAG       "fdc02_fdc"

FLOPPY_FORMATS_MEMBER( a2bus_corvfdc02_device::corv_floppy_formats )
	FLOPPY_CONCEPT_525DSDD_FORMAT,
	FLOPPY_IMD_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( corv_floppies )
	SLOT_INTERFACE( "525dsqd", FLOPPY_525_QD )
SLOT_INTERFACE_END


MACHINE_CONFIG_FRAGMENT( fdc02 )
	MCFG_UPD765A_ADD(FDC02_FDC_TAG, true, false)
	MCFG_UPD765_INTRQ_CALLBACK(WRITELINE(a2bus_corvfdc02_device, intrq_w))
	MCFG_UPD765_DRQ_CALLBACK(WRITELINE(a2bus_corvfdc02_device, drq_w))
	MCFG_FLOPPY_DRIVE_ADD(FDC02_FDC_TAG":0", corv_floppies, "525dsqd", a2bus_corvfdc02_device::corv_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FDC02_FDC_TAG":1", corv_floppies, "525dsqd", a2bus_corvfdc02_device::corv_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FDC02_FDC_TAG":2", corv_floppies, "525dsqd", a2bus_corvfdc02_device::corv_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FDC02_FDC_TAG":3", corv_floppies, "525dsqd", a2bus_corvfdc02_device::corv_floppy_formats)
MACHINE_CONFIG_END

ROM_START( fdc02 )
	ROM_REGION(0x20, FDC02_ROM_REGION, 0)
	ROM_LOAD( "bfc00.bin", 0x000000, 0x000020, CRC(98d1a765) SHA1(d27c3c6921e1bb3778a3f78decf106275bc0add1) )
ROM_END

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_corvfdc02_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( fdc02 );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *a2bus_corvfdc02_device::device_rom_region() const
{
	return ROM_NAME( fdc02 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_corvfdc02_device::a2bus_corvfdc02_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_a2bus_card_interface(mconfig, *this),
	m_fdc(*this, FDC02_FDC_TAG),
	m_con1(*this, FDC02_FDC_TAG":0"),
	m_con2(*this, FDC02_FDC_TAG":1"),
	m_con3(*this, FDC02_FDC_TAG":2"),
	m_con4(*this, FDC02_FDC_TAG":3"), m_rom(nullptr), m_fdc_local_status(0), m_fdc_local_command(0), m_bufptr(0), m_curfloppy(nullptr), m_in_drq(false), m_timer(nullptr)
{
}

a2bus_corvfdc02_device::a2bus_corvfdc02_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, A2BUS_CORVFDC02, "Corvus Systems Buffered Floppy Controller", tag, owner, clock, "crvfdc02", __FILE__),
	device_a2bus_card_interface(mconfig, *this),
	m_fdc(*this, FDC02_FDC_TAG),
	m_con1(*this, FDC02_FDC_TAG":0"),
	m_con2(*this, FDC02_FDC_TAG":1"),
	m_con3(*this, FDC02_FDC_TAG":2"),
	m_con4(*this, FDC02_FDC_TAG":3"), m_rom(nullptr), m_fdc_local_status(0), m_fdc_local_command(0), m_bufptr(0), m_curfloppy(nullptr), m_in_drq(false), m_timer(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_corvfdc02_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();

	m_rom = device().machine().root_device().memregion(this->subtag(FDC02_ROM_REGION).c_str())->base();

	m_timer = timer_alloc(0);

	save_item(NAME(m_fdc_local_status));
	save_item(NAME(m_fdc_local_command));
	save_item(NAME(m_bufptr));
	save_item(NAME(m_buffer));
}

void a2bus_corvfdc02_device::device_reset()
{
	m_fdc_local_status = 2;
	m_fdc_local_command = 0;
	m_curfloppy = nullptr;
	m_in_drq = false;
	m_timer->adjust(attotime::never);
}

void a2bus_corvfdc02_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	m_fdc->tc_w(true);
	m_fdc->tc_w(false);
}

/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

UINT8 a2bus_corvfdc02_device::read_c0nx(address_space &space, UINT8 offset)
{
	switch (offset)
	{
		case 0: // 765 FIFO
			return m_fdc->fifo_r(space, 0);

		case 1: // 765 MSR
			return m_fdc->msr_r(space, 0);

		case 2: // buffer address
			return (m_bufptr>>1) & 0xff;

		case 3:
//          printf("Read buffer @ %x = %02x\n", m_bufptr, m_buffer[m_bufptr]);
			return m_buffer[m_bufptr--];

		case 4: // local status
			if (m_curfloppy)
			{
				m_fdc_local_status &= ~(1 | 0x40);
				m_fdc_local_status |= m_curfloppy->dskchg_r() ? 1 : 0;
				m_fdc_local_status |= m_curfloppy->ready_r() ? 0x40 : 0;
			}
			return m_fdc_local_status;
			break;
	}

	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_corvfdc02_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
	floppy_image_device *floppy = nullptr;

	switch (offset)
	{
		case 0:    // FDC FIFO write
			m_fdc->fifo_w(space, offset, data);
			break;

		case 1:    // FDC ???
			break;

		case 2: // buffer address
			m_bufptr = (data << 1) | (data & 1);
//          printf("%02x to buffer address yields %x\n", data, m_bufptr);
			break;

		case 3: // buffer write
//          printf("%02x to buffer[%x]\n", data, m_bufptr);
			m_buffer[m_bufptr--] = data;
			break;

		case 4:     // LOCAL COMMAND REG
			m_fdc_local_command = data;

			// drive select enabled?
			if (data & 4)
			{
				switch (data & 3)
				{
					case 0:
						floppy = m_con1 ? m_con1->get_device() : nullptr;
						break;
					case 1:
						floppy = m_con2 ? m_con2->get_device() : nullptr;
						break;
					case 2:
						floppy = m_con3 ? m_con3->get_device() : nullptr;
						break;
					case 3:
						floppy = m_con4 ? m_con4->get_device() : nullptr;
						break;
				}

				logerror("corvfdc02: selecting drive %d: %p\n", data & 3, (void *) floppy);

				if (floppy != m_curfloppy)
				{
					m_fdc->set_floppy(floppy);
					m_curfloppy = floppy;
				}
			}

			if (m_curfloppy != nullptr)
			{
				// motor control (active low)
				m_curfloppy->mon_w((data & 8) ? 1 : 0);
//              printf("Cur drive %p motor %s\n", m_curfloppy, (data & 8) ? "OFF" : "ON");
			}

			if (data & 0x80)
			{
//              printf("Reset NEC765\n");
				m_fdc->reset();
			}
			break;
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

UINT8 a2bus_corvfdc02_device::read_cnxx(address_space &space, UINT8 offset)
{
	return m_rom[offset & 0x1f];
}

WRITE_LINE_MEMBER(a2bus_corvfdc02_device::intrq_w)
{
	if (state)
	{
		m_fdc_local_status &= ~2;   // indicate IRQ occurred
		if (m_fdc_local_command & 0x20)
		{
			raise_slot_irq();
		}
	}
	else
	{
		m_fdc_local_status |= 2;    // clear IRQ
		lower_slot_irq();
	}
}

WRITE_LINE_MEMBER(a2bus_corvfdc02_device::drq_w)
{
	if (state)
	{
		// pseudo-DMA direction?
		if (m_fdc_local_command & 0x40)
		{
			m_buffer[m_bufptr] = m_fdc->dma_r();
//          printf("DMA %02x to buffer[%x]\n", m_buffer[m_bufptr], m_bufptr);

			if (!m_bufptr)
			{
				m_timer->adjust(attotime::zero);
			}

			m_bufptr--;
			m_bufptr &= 0x1ff;
		}
		else
		{
			m_fdc->dma_w(m_buffer[m_bufptr++]);
			m_bufptr &= 0x1ff;
		}
	}
}
