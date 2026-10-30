// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    corvfdc01.c

    Implemention of the Corvus Systems CORVUS01 floppy controller

    Boot PROM 0.8 fixes this at: 8", 500 blocks total, 128 bytes/block,
                                 26 sectors/track, 77 tracks.

*********************************************************************/

#include "emu.h"
#include "corvfdc01.h"

#include "machine/wd_fdc.h"
#include "imagedev/floppy.h"


namespace {

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

#define FDC01_ROM_REGION    "fdc01_rom"
#define FDC01_FDC_TAG       "fdc01_fdc"

static void corv_floppies(device_slot_interface &device)
{
	device.option_add("8sssd", FLOPPY_8_SSSD);
}

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

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_corvfdc01_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_corvfdc01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_corvfdc01_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual void reset_from_bus() override;

	required_region_ptr<uint8_t> m_rom;
	required_device<fd1793_device> m_wdfdc;
	required_device_array<floppy_connector, 4> m_con;

private:
	void intrq_w(int state);
	void drq_w(int state);

	uint8_t m_fdc_local_status, m_fdc_local_command;
	floppy_image_device *m_curfloppy;
};


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void a2bus_corvfdc01_device::device_add_mconfig(machine_config &config)
{
	FD1793(config, m_wdfdc, 16_MHz_XTAL / 8);
	m_wdfdc->intrq_wr_callback().set(FUNC(a2bus_corvfdc01_device::intrq_w));
	m_wdfdc->drq_wr_callback().set(FUNC(a2bus_corvfdc01_device::drq_w));
	for (auto &con : m_con)
		FLOPPY_CONNECTOR(config, con, corv_floppies, "8sssd", floppy_image_device::default_fm_floppy_formats);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const tiny_rom_entry *a2bus_corvfdc01_device::device_rom_region() const
{
	return ROM_NAME( fdc01 );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

a2bus_corvfdc01_device::a2bus_corvfdc01_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_a2bus_card_interface(mconfig, *this),
	m_rom(*this, FDC01_ROM_REGION),
	m_wdfdc(*this, FDC01_FDC_TAG),
	m_con(*this, FDC01_FDC_TAG":%u", 0U),
	m_fdc_local_status(0), m_fdc_local_command(0), m_curfloppy(nullptr)
{
}

a2bus_corvfdc01_device::a2bus_corvfdc01_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	a2bus_corvfdc01_device(mconfig, A2BUS_CORVFDC01, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void a2bus_corvfdc01_device::device_start()
{
	save_item(NAME(m_fdc_local_status));
	save_item(NAME(m_fdc_local_command));
}

void a2bus_corvfdc01_device::device_reset()
{
	m_fdc_local_status = 0;
	m_fdc_local_command = 0;
	m_curfloppy = nullptr;
}

void a2bus_corvfdc01_device::reset_from_bus()
{
	m_wdfdc->reset();
}

/*-------------------------------------------------
    read_c0nx - called for reads from this card's c0nx space
-------------------------------------------------*/

uint8_t a2bus_corvfdc01_device::read_c0nx(uint8_t offset)
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
			return m_wdfdc->status_r();

		case  9:
			return m_wdfdc->track_r();

		case 10:
			return m_wdfdc->sector_r();

		case 11:
			return m_wdfdc->data_r();
	}

	return 0xff;
}


/*-------------------------------------------------
    write_c0nx - called for writes to this card's c0nx space
-------------------------------------------------*/

void a2bus_corvfdc01_device::write_c0nx(uint8_t offset, uint8_t data)
{
	int current_drive;
	floppy_image_device *floppy = nullptr;

	switch (offset)
	{
		case 0:     // LOCAL COMMAND REG
			m_fdc_local_command = data;

			current_drive = bitswap<2>(data, LC_DE1_bit, LC_DE0_bit);
			floppy = m_con[current_drive] ? m_con[current_drive]->get_device() : nullptr;

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
			m_wdfdc->cmd_w(data);
			break;

		case  9:    // FDC TRACK REG
			m_wdfdc->track_w(data);
			break;

		case 10:    // FDC SECTOR REG
			m_wdfdc->sector_w(data);
			break;

		case 11:    // FDC DATA REG
			m_wdfdc->data_w(data);
			break;
	}
}

/*-------------------------------------------------
    read_cnxx - called for reads from this card's cnxx space
-------------------------------------------------*/

uint8_t a2bus_corvfdc01_device::read_cnxx(uint8_t offset)
{
	return m_rom[offset & 0x1f];
}

void a2bus_corvfdc01_device::intrq_w(int state)
{
	if (state)
		m_fdc_local_status |= LS_INT_mask;
	else
		m_fdc_local_status &= ~LS_INT_mask;
}

void a2bus_corvfdc01_device::drq_w(int state)
{
	if (state)
		m_fdc_local_status |= LS_DRQ_mask;
	else
		m_fdc_local_status &= ~LS_DRQ_mask;
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(A2BUS_CORVFDC01, device_a2bus_card_interface, a2bus_corvfdc01_device, "crvfdc01", "Corvus Systems Floppy Controller")
