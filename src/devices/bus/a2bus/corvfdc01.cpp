// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    corvfdc01.c

    Implemention of the Corvus Systems CORVUS01 floppy controller

    Boot PROM 0.8 fixes this at: 8", 500 blocks total, 128 bytes/block,
                                 26 sectors/track, 77 tracks.

*********************************************************************/

#include "corvfdc01.h"

/***************************************************************************
    PARAMETERS
***************************************************************************/

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type A2BUS_CORVFDC01 = &device_creator<a2bus_corvfdc01_device>;

#define FDC01_ROM_REGION    "fdc01_rom"
#define FDC01_FDC_TAG       "fdc01_fdc"

FLOPPY_FORMATS_MEMBER( a2bus_corvfdc01_device::corv_floppy_formats )
	FLOPPY_IMD_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( corv_floppies )
	SLOT_INTERFACE( "8sssd", FLOPPY_8_SSSD )
SLOT_INTERFACE_END

MACHINE_CONFIG_FRAGMENT( fdc01 )
	MCFG_FD1793_ADD(FDC01_FDC_TAG, XTAL_16MHz / 8)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(a2bus_corvfdc01_device, intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(a2bus_corvfdc01_device, drq_w))
	MCFG_FLOPPY_DRIVE_ADD(FDC01_FDC_TAG":0", corv_floppies, "8sssd", a2bus_corvfdc01_device::corv_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FDC01_FDC_TAG":1", corv_floppies, "8sssd", a2bus_corvfdc01_device::corv_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FDC01_FDC_TAG":2", corv_floppies, "8sssd", a2bus_corvfdc01_device::corv_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(FDC01_FDC_TAG":3", corv_floppies, "8sssd", a2bus_corvfdc01_device::corv_floppy_formats)
MACHINE_CONFIG_END

ROM_START( fdc01 )
	ROM_REGION(0x20, FDC01_ROM_REGION, 0)
	ROM_LOAD( "ff01.bin",     0x000000, 0x000020, CRC(ad3c1136) SHA1(b1e1e8a10618588b1b44b3be5d88857497f30b33) )
ROM_END

enum
{
	LS_DRQ_bit      = 0,    // DRQ
	LS_INT_bit      = 1,    // INT
	LS_SS_bit       = 4,    // 1 if single-sided (floppy or drive?)
	LS_8IN_bit      = 5,    // 1 if 8" floppy drive?
	LS_DSKCHG_bit   = 6,    // 0 if disk changed, 1 if not
	LS_SD_bit       = 7,    // 1 if single density

	LS_DRQ_mask     = (1 << LS_DRQ_bit),
	LS_INT_mask     = (1 << LS_INT_bit),
	LS_SS_mask      = (1 << LS_SS_bit),
	LS_8IN_mask     = (1 << LS_8IN_bit),
	LS_DSKCHG_mask  = (1 << LS_DSKCHG_bit),
	LS_SD_mask      = (1 << LS_SD_bit)
};

enum
{
	LC_FLPSD1_bit   = 0,    // 0 if side 0 , 1 if side 1
	LC_DE0_bit      = 1,    // drive select bit 0
	LC_DE1_bit      = 4,    // drive select bit 1
	LC_MOTOROF_bit  = 5,    // 1 if motor to be turned off
	LC_FLP8IN_bit   = 6,    // 1 to select 8", 0 for 5"1/4 (which I knew what it means)
	LC_FMMFM_bit    = 7,    // 1 to select single density, 0 for double

	LC_FLPSD1_mask  = (1 << LC_FLPSD1_bit),
	LC_DE0_mask     = (1 << LC_DE0_bit),
	LC_DE1_mask     = (1 << LC_DE1_bit),
	LC_MOTOROF_mask = (1 << LC_MOTOROF_bit),
	LC_FLP8IN_mask  = (1 << LC_FLP8IN_bit),
	LC_FMMFM_mask   = (1 << LC_FMMFM_bit)
};

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor a2bus_corvfdc01_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( fdc01 );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *a2bus_corvfdc01_device::device_rom_region() const
{
	return ROM_NAME( fdc01 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_corvfdc01_device::a2bus_corvfdc01_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source) :
	device_t(mconfig, type, name, tag, owner, clock, shortname, source),
	device_a2bus_card_interface(mconfig, *this),
	m_wdfdc(*this, FDC01_FDC_TAG),
	m_con1(*this, FDC01_FDC_TAG":0"),
	m_con2(*this, FDC01_FDC_TAG":1"),
	m_con3(*this, FDC01_FDC_TAG":2"),
	m_con4(*this, FDC01_FDC_TAG":3"), m_rom(nullptr), m_fdc_local_status(0), m_fdc_local_command(0), m_curfloppy(nullptr)
{
}

a2bus_corvfdc01_device::a2bus_corvfdc01_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, A2BUS_CORVFDC01, "Corvus Systems Floppy Controller", tag, owner, clock, "crvfdc01", __FILE__),
	device_a2bus_card_interface(mconfig, *this),
	m_wdfdc(*this, FDC01_FDC_TAG),
	m_con1(*this, FDC01_FDC_TAG":0"),
	m_con2(*this, FDC01_FDC_TAG":1"),
	m_con3(*this, FDC01_FDC_TAG":2"),
	m_con4(*this, FDC01_FDC_TAG":3"), m_rom(nullptr), m_fdc_local_status(0), m_fdc_local_command(0), m_curfloppy(nullptr)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_corvfdc01_device::device_start()
{
	// set_a2bus_device makes m_slot valid
	set_a2bus_device();

	m_rom = device().machine().root_device().memregion(this->subtag(FDC01_ROM_REGION).c_str())->base();

	save_item(NAME(m_fdc_local_status));
	save_item(NAME(m_fdc_local_command));
}

void a2bus_corvfdc01_device::device_reset()
{
	m_fdc_local_status = 0;
	m_fdc_local_command = 0;
	m_curfloppy = nullptr;
}

/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

UINT8 a2bus_corvfdc01_device::read_c0nx(address_space &space, UINT8 offset)
{
	switch (offset)
	{
		case 0: // local status
			if (m_curfloppy)
			{
				m_fdc_local_status &= ~LS_DSKCHG_mask;
				m_fdc_local_status |= m_curfloppy->dskchg_r() ? LS_DSKCHG_mask : 0;
			}
			return m_fdc_local_status | LS_8IN_mask;

		case  8:    // WD1793 at 8-11
			return m_wdfdc->status_r(space, offset);

		case  9:
			return m_wdfdc->track_r(space, offset);

		case 10:
			return m_wdfdc->sector_r(space, offset);

		case 11:
			return m_wdfdc->data_r(space, offset);
	}

	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_corvfdc01_device::write_c0nx(address_space &space, UINT8 offset, UINT8 data)
{
	int current_drive;
	floppy_image_device *floppy = nullptr;

	switch (offset)
	{
		case 0:     // LOCAL COMMAND REG
			m_fdc_local_command = data;

			current_drive = ((data >> LC_DE0_bit) & 1) | ((data >> (LC_DE1_bit-1)) & 2);
			switch (current_drive)
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

			if (floppy != m_curfloppy)
			{
				m_wdfdc->set_floppy(floppy);
			}

			if (m_curfloppy != nullptr)
			{
				// side select
				m_curfloppy->ss_w((data & LC_FLPSD1_mask) != 0);

				// motor control (active low)
				m_curfloppy->mon_w((data & LC_MOTOROF_mask) ? 1 : 0);
			}

			/*flp_8in = (data & LC_FLP8IN_mask) != 0;*/

			m_wdfdc->dden_w(BIT(data, LC_FMMFM_bit));
			break;

		case  8:    // FDC COMMAMD REG
			m_wdfdc->cmd_w(space, offset, data);
			break;

		case  9:    // FDC TRACK REG
			m_wdfdc->track_w(space, offset, data);
			break;

		case 10:    // FDC SECTOR REG
			m_wdfdc->sector_w(space, offset, data);
			break;

		case 11:    // FDC DATA REG
			m_wdfdc->data_w(space, offset, data);
			break;
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

UINT8 a2bus_corvfdc01_device::read_cnxx(address_space &space, UINT8 offset)
{
	return m_rom[offset & 0x1f];
}

WRITE_LINE_MEMBER(a2bus_corvfdc01_device::intrq_w)
{
	if (state)
		m_fdc_local_status |= LS_INT_mask;
	else
		m_fdc_local_status &= ~LS_INT_mask;
}

WRITE_LINE_MEMBER(a2bus_corvfdc01_device::drq_w)
{
	if (state)
		m_fdc_local_status |= LS_DRQ_mask;
	else
		m_fdc_local_status &= ~LS_DRQ_mask;
}
