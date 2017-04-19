// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    coco_fdc.cpp

    CoCo Floppy Disk Controller

    The CoCo and Dragon both use the Western Digital floppy disk controllers.
    The CoCo uses either the WD1793 or the WD1773, the Dragon uses the WD2797,
    which mostly uses the same command set with some subtle differences, most
    notably the 2797 handles disk side select internally. The Dragon Alpha also
    uses the WD2797, however as this is a built in interface and not an external
    cartrige, it is dealt with in the main coco.cpp file.

    The wd's variables are mapped to $FF48-$FF4B on the CoCo and on $FF40-$FF43
    on the Dragon.  In addition, there is another register
    called DSKREG that controls the interface with the wd1793.  DSKREG is
    detailed below:  But they appear to be

    References:
        CoCo:   Disk Basic Unravelled
        Dragon: Inferences from the PC-Dragon source code
                DragonDos Controller, Disk and File Formats by Graham E Kinns

    ---------------------------------------------------------------------------

    DSKREG - the control register
    CoCo ($FF40)

    Bit
    7 halt enable flag
    6 drive select #3
    5 density (0=single, 1=double) and NMI enable flag
    4 write precompensation
    3 drive motor activation
    2 drive select #2
    1 drive select #1
    0 drive select #0

    Reading from $FF48-$FF4F clears bit 7 of DSKREG ($FF40)

*********************************************************************/

#include "emu.h"
#include "cococart.h"
#include "coco_fdc.h"
#include "imagedev/flopdrv.h"
#include "machine/msm6242.h"
#include "machine/ds1315.h"
#include "machine/wd_fdc.h"
#include "formats/dmk_dsk.h"
#include "formats/jvc_dsk.h"
#include "formats/vdk_dsk.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

#define LOG_FDC                 0
#define WD_TAG                  "wd17xx"
#define WD2797_TAG              "wd2797"
#define DISTO_TAG               "disto"
#define CLOUD9_TAG              "cloud9"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class coco_fdc_device_base : public coco_family_fdc_device_base
{
public:
	// construction/destruction
	coco_fdc_device_base(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

protected:
	enum class rtc_type
	{
		DISTO = 0x00,
		CLOUD9 = 0x01,
		NONE = 0xFF
	};

	// device-level overrides
	virtual DECLARE_READ8_MEMBER(read) override;
	virtual DECLARE_WRITE8_MEMBER(write) override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// methods
	virtual void update_lines() override;
	void dskreg_w(uint8_t data);
	rtc_type real_time_clock();

	// devices
	required_device<wd1773_t>                   m_wd17xx;
	required_device<ds1315_device>              m_ds1315;
	required_device_array<floppy_connector, 4>  m_floppies;

	// Disto RTC
	required_device<msm6242_device> m_disto_msm6242;        // 6242 RTC on Disto interface
	offs_t m_msm6242_rtc_address;
	optional_ioport m_rtc;
};



/***************************************************************************
    LOCAL VARIABLES
***************************************************************************/

FLOPPY_FORMATS_MEMBER( coco_family_fdc_device_base::floppy_formats )
	FLOPPY_DMK_FORMAT,
	FLOPPY_JVC_FORMAT,
	FLOPPY_VDK_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( coco_fdc_floppies )
	SLOT_INTERFACE("qd", FLOPPY_525_QD)
SLOT_INTERFACE_END

static MACHINE_CONFIG_FRAGMENT(coco_fdc)
	MCFG_WD1773_ADD(WD_TAG, XTAL_8MHz)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(coco_fdc_device_base, fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(coco_fdc_device_base, fdc_drq_w))

	MCFG_FLOPPY_DRIVE_ADD(WD_TAG ":0", coco_fdc_floppies, "qd", coco_fdc_device_base::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD(WD_TAG ":1", coco_fdc_floppies, "qd", coco_fdc_device_base::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD(WD_TAG ":2", coco_fdc_floppies, nullptr, coco_fdc_device_base::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD(WD_TAG ":3", coco_fdc_floppies, nullptr, coco_fdc_device_base::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)

	MCFG_DEVICE_ADD(DISTO_TAG, MSM6242, XTAL_32_768kHz)
	MCFG_DS1315_ADD(CLOUD9_TAG)
MACHINE_CONFIG_END


//***************************************************************************
//  COCO FAMILY FDCs - base class for CoCo/Dragon
//***************************************************************************

//-------------------------------------------------
//  coco_family_fdc_device_base::device_start
//-------------------------------------------------

void coco_family_fdc_device_base::device_start()
{
	save_item(NAME(m_dskreg));
	save_item(NAME(m_intrq));
	save_item(NAME(m_drq));
}


//-------------------------------------------------
//  coco_family_fdc_device_base::device_reset
//-------------------------------------------------

void coco_family_fdc_device_base::device_reset()
{
	m_dskreg = 0x00;
	m_intrq = false;
	m_drq = true;
}


//-------------------------------------------------
//  coco_family_fdc_device_base::get_cart_base
//-------------------------------------------------

uint8_t* coco_family_fdc_device_base::get_cart_base()
{
	return memregion("eprom")->base();
}


//***************************************************************************
//  COCO FDCs
//***************************************************************************

//-------------------------------------------------
//  coco_fdc_device_base - constructor
//-------------------------------------------------

coco_fdc_device_base::coco_fdc_device_base(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source)
	: coco_family_fdc_device_base(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_wd17xx(*this, WD_TAG)
	, m_ds1315(*this, CLOUD9_TAG)
	, m_floppies(*this, WD_TAG ":%u", 0)
	, m_disto_msm6242(*this, DISTO_TAG)
	, m_msm6242_rtc_address(0)
	, m_rtc(*this, ":real_time_clock")
{
}

//-------------------------------------------------
//  real_time_clock
//-------------------------------------------------

coco_fdc_device_base::rtc_type coco_fdc_device_base::real_time_clock()
{
	rtc_type result = (rtc_type) m_rtc.read_safe((ioport_value) rtc_type::NONE);

	// check to make sure we don't have any invalid values
	if (((result == rtc_type::DISTO) && (m_disto_msm6242 == nullptr))
		|| ((result == rtc_type::CLOUD9) && (m_ds1315 == nullptr)))
	{
		result = rtc_type::NONE;
	}

	return result;
}


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor coco_fdc_device_base::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(coco_fdc);
}


//-------------------------------------------------
//  update_lines - CoCo specific disk
//  controller lines
//-------------------------------------------------

void coco_fdc_device_base::update_lines()
{
	// clear HALT enable under certain circumstances
	if (intrq() && (dskreg() & 0x20))
		set_dskreg(dskreg() & ~0x80);  // clear halt enable

	// set the NMI line
	cart_set_line(cococart_slot_device::line::NMI, intrq() && (dskreg() & 0x20));

	// set the HALT line
	cart_set_line(cococart_slot_device::line::HALT, !drq() && (dskreg() & 0x80));
}


//-------------------------------------------------
//  dskreg_w - function to write to CoCo dskreg
//-------------------------------------------------

void coco_fdc_device_base::dskreg_w(uint8_t data)
{
	uint8_t drive = 0;
	uint8_t head;

	if (LOG_FDC)
	{
		logerror("fdc_coco_dskreg_w(): %c%c%c%c%c%c%c%c ($%02x)\n",
			data & 0x80 ? 'H' : 'h',
			data & 0x40 ? '3' : '.',
			data & 0x20 ? 'D' : 'S',
			data & 0x10 ? 'P' : 'p',
			data & 0x08 ? 'M' : 'm',
			data & 0x04 ? '2' : '.',
			data & 0x02 ? '1' : '.',
			data & 0x01 ? '0' : '.',
			data);
	}

	/* An email from John Kowalski informed me that if the DS3 is
	 * high, and one of the other drive bits is selected (DS0-DS2), then the
	 * second side of DS0, DS1, or DS2 is selected.  If multiple bits are
	 * selected in other situations, then both drives are selected, and any
	 * read signals get yucky.
	 */

	if (data & 0x04)
		drive = 2;
	else if (data & 0x02)
		drive = 1;
	else if (data & 0x01)
		drive = 0;
	else if (data & 0x40)
		drive = 3;

	for (int i = 0; i < 4; i++)
	{
		floppy_image_device *floppy = m_floppies[i]->get_device();
		if (floppy)
			floppy->mon_w(i == drive ? CLEAR_LINE : ASSERT_LINE);
	}

	head = ((data & 0x40) && (drive != 3)) ? 1 : 0;

	set_dskreg(data);

	update_lines();

	floppy_image_device *selected_floppy = m_floppies[drive]->get_device();
	m_wd17xx->set_floppy(selected_floppy);

	if (selected_floppy)
		selected_floppy->ss_w(head);

	m_wd17xx->dden_w(!BIT(dskreg(), 5));
}


//-------------------------------------------------
//  read
//-------------------------------------------------

READ8_MEMBER(coco_fdc_device_base::read)
{
	uint8_t result = 0;

	switch(offset & 0xEF)
	{
		case 8:
			result = m_wd17xx->status_r(space, 0);
			break;
		case 9:
			result = m_wd17xx->track_r(space, 0);
			break;
		case 10:
			result = m_wd17xx->sector_r(space, 0);
			break;
		case 11:
			result = m_wd17xx->data_r(space, 0);
			break;
	}

	/* other stuff for RTCs */
	switch(offset)
	{
		case 0x10:  /* FF50 */
			if (real_time_clock() == rtc_type::DISTO)
				result = m_disto_msm6242->read(space,m_msm6242_rtc_address);
			break;

		case 0x38:  /* FF78 */
			if (real_time_clock() == rtc_type::CLOUD9)
				m_ds1315->read_0(space, offset);
			break;

		case 0x39:  /* FF79 */
			if (real_time_clock() == rtc_type::CLOUD9)
				m_ds1315->read_1(space, offset);
			break;

		case 0x3C:  /* FF7C */
			if (real_time_clock() == rtc_type::CLOUD9)
				result = m_ds1315->read_data(space, offset);
			break;
	}
	return result;
}



//-------------------------------------------------
//  write
//-------------------------------------------------

WRITE8_MEMBER(coco_fdc_device_base::write)
{
	switch(offset & 0x1F)
	{
		case 0: case 1: case 2: case 3:
		case 4: case 5: case 6: case 7:
			dskreg_w(data);
			break;
		case 8:
			m_wd17xx->cmd_w(space, 0, data);
			break;
		case 9:
			m_wd17xx->track_w(space, 0, data);
			break;
		case 10:
			m_wd17xx->sector_w(space, 0, data);
			break;
		case 11:
			//printf("data w %02x\n", data);
			m_wd17xx->data_w(space, 0, data);
			break;
	};

	/* other stuff for RTCs */
	switch(offset)
	{
		case 0x10:  /* FF50 */
			if (real_time_clock() == rtc_type::DISTO)
				m_disto_msm6242->write(space,m_msm6242_rtc_address, data);
			break;

		case 0x11:  /* FF51 */
			if (real_time_clock() == rtc_type::DISTO)
				m_msm6242_rtc_address = data & 0x0f;
			break;
	}
}


//**************************************************************************
//  COCO FDC
//**************************************************************************

ROM_START(coco_fdc)
	ROM_REGION(0x4000, "eprom", ROMREGION_ERASE00)
	ROM_LOAD_OPTIONAL("disk10.rom", 0x0000, 0x2000, CRC(b4f9968e) SHA1(04115be3f97952b9d9310b52f806d04f80b40d03))
ROM_END

namespace
{
	class coco_fdc_device : public coco_fdc_device_base
	{
	public:
		// construction/destruction
		coco_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
			: coco_fdc_device_base(mconfig, COCO_FDC, "CoCo FDC", tag, owner, clock, "coco_fdc", __FILE__)
		{
		}

	protected:
		// optional information overrides
		virtual const tiny_rom_entry *device_rom_region() const override
		{
			return ROM_NAME(coco_fdc);
		}

	};

}
const device_type COCO_FDC = device_creator<coco_fdc_device>;


//**************************************************************************
//              COCO FDC v1.1
//**************************************************************************

ROM_START(coco_fdc_v11)
	ROM_REGION(0x8000, "eprom", ROMREGION_ERASE00)
	ROM_LOAD_OPTIONAL("disk11.rom", 0x0000, 0x2000, CRC(0b9c5415) SHA1(10bdc5aa2d7d7f205f67b47b19003a4bd89defd1))
	ROM_RELOAD(0x2000, 0x2000)
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x6000, 0x2000)
ROM_END

namespace
{
	class coco_fdc_v11_device : public coco_fdc_device_base
	{
	public:
		// construction/destruction
		coco_fdc_v11_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
			: coco_fdc_device_base(mconfig, COCO_FDC_V11, "CoCo FDC v1.1", tag, owner, clock, "coco_fdc_v11", __FILE__)
		{
		}

	protected:
		// optional information overrides
		virtual const tiny_rom_entry *device_rom_region() const override
		{
			return ROM_NAME(coco_fdc_v11);
		}
	};
};

const device_type COCO_FDC_V11 = device_creator<coco_fdc_v11_device>;


//**************************************************************************
//              COCO-3 HDB-DOS
//**************************************************************************

ROM_START(coco3_hdb1)
	ROM_REGION(0x8000, "eprom", ROMREGION_ERASE00)
	ROM_LOAD("hdbdw3bc3.rom", 0x0000, 0x2000, CRC(309a9efd) SHA1(671605d61811953860466f771c1594bbade331f4))
	ROM_RELOAD(0x2000, 0x2000)
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x6000, 0x2000)
ROM_END

namespace
{
	class coco3_hdb1_device : public coco_fdc_device_base
	{
	public:
		// construction/destruction
		coco3_hdb1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
			: coco_fdc_device_base(mconfig, COCO3_HDB1, "CoCo3 HDB-DOS", tag, owner, clock, "coco3_hdb1", __FILE__)
		{
		}

	protected:
		// optional information overrides
		virtual const tiny_rom_entry *device_rom_region() const override
		{
			return ROM_NAME(coco3_hdb1);
		}
	};
};

const device_type COCO3_HDB1 = device_creator<coco3_hdb1_device>;

//**************************************************************************
//              CP400 FDC
//**************************************************************************

ROM_START(cp400_fdc)
	ROM_REGION(0x4000, "eprom", ROMREGION_ERASE00)
	ROM_LOAD("cp400dsk.rom", 0x0000, 0x2000, CRC(e9ad60a0) SHA1(827697fa5b755f5dc1efb054cdbbeb04e405405b))
ROM_END

namespace
{
	class cp400_fdc_device : public coco_fdc_device_base
	{
	public:
		// construction/destruction
		cp400_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
			: coco_fdc_device_base(mconfig, CP400_FDC, "CP400 FDC", tag, owner, clock, "cp400_fdc", __FILE__)
		{
		}

	protected:
		// optional information overrides
		virtual const tiny_rom_entry *device_rom_region() const override
		{
			return ROM_NAME(cp400_fdc);
		}
	};
};

const device_type CP400_FDC = device_creator<cp400_fdc_device>;
