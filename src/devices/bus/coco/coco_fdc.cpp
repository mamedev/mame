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
    cartridge, it is dealt with in the main coco.cpp file.

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

    ---------------------------------------------------------------------------

    Disto No Halt Extension

    The Disto Super Controller II includes "no halt" circuitry. Implemented
    by using a read and write cache.

    CachDat - Cache Data Register
        $FF74 & $FF75: Read/Write cache data.

    CachCtrl - Cache Controller
      $FF76: Read
        Bit 7 low indicates an interrupt request from the disk controller

      $FF76: Write:
        00000000 = Caching off
        00001000 = Tell cache controller to send interrupt when device is
                   ready to send/receive a buffer (seek done, etc.)
        00000111 = Read cache on - Get next 256 data bytes from controller
                   to cache
        00000100 = Write cache on - Next 256 bytes stored in cache are
                   sector
        00000110 = Copy Write cache to controller



*********************************************************************/

#include "emu.h"
#include "cococart.h"
#include "coco_fdc.h"
#include "imagedev/floppy.h"
#include "machine/msm6242.h"
#include "machine/ds1315.h"
#include "machine/wd_fdc.h"
#include "machine/ram.h"
#include "formats/dmk_dsk.h"
#include "formats/jvc_dsk.h"
#include "formats/vdk_dsk.h"
#include "formats/sdf_dsk.h"
#include "formats/os9_dsk.h"

//#define LOG_GENERAL   (1U << 0) //defined in logmacro.h already
#define LOG_WDFDC   (1U << 1) // Shows register setup
#define LOG_WDIO    (1U << 2) // Shows Data read and write

//#define VERBOSE (LOG_GENERAL | LOG_WDFDC)
#include "logmacro.h"

#define LOGWDFDC(...)   LOGMASKED(LOG_WDFDC,  __VA_ARGS__)
#define LOGWDIO(...)   LOGMASKED(LOG_WDIO,  __VA_ARGS__)

/***************************************************************************
    PARAMETERS
***************************************************************************/

#define WD_TAG                  "wd17xx"
#define WD2797_TAG              "wd2797"
#define DISTO_TAG               "disto"
#define CLOUD9_TAG              "cloud9"


template class device_finder<coco_family_fdc_device_base, false>;
template class device_finder<coco_family_fdc_device_base, true>;


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class coco_fdc_device_base : public coco_family_fdc_device_base
{
protected:
	// construction/destruction
	coco_fdc_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	enum class rtc_type
	{
		DISTO = 0x00,
		CLOUD9 = 0x01,
		NONE = 0xFF
	};

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// device-level overrides
	virtual u8 cts_read(offs_t offset) override;
	virtual u8 scs_read(offs_t offset) override;
	virtual void scs_write(offs_t offset, u8 data) override;
	virtual void device_add_mconfig(machine_config &config) override;

	// methods
	virtual void update_lines() override;
	void dskreg_w(u8 data);
	rtc_type real_time_clock();

	// devices
	required_device<wd1773_device>              m_wd17xx;
	required_device<ds1315_device>              m_ds1315;
	required_device_array<floppy_connector, 4>  m_floppies;

	// Disto RTC
	required_device<msm6242_device> m_disto_msm6242;        // 6242 RTC on Disto interface
	offs_t m_msm6242_rtc_address;
	optional_ioport m_rtc;

	// Protected
	u8 ff74_read(offs_t offset);
	void ff74_write(offs_t offset, u8 data);

private:
	// registers
	u8 m_cache_controler;
	u8 m_cache_pointer;
	required_device<ram_device>                 m_cache_buffer;
};



/***************************************************************************
    LOCAL VARIABLES
***************************************************************************/

void coco_family_fdc_device_base::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_DMK_FORMAT);
	fr.add(FLOPPY_JVC_FORMAT);
	fr.add(FLOPPY_VDK_FORMAT);
	fr.add(FLOPPY_SDF_FORMAT);
	fr.add(FLOPPY_OS9_FORMAT);
}

static void coco_fdc_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("35dd", FLOPPY_35_DD);
}

void coco_fdc_device_base::device_add_mconfig(machine_config &config)
{
	WD1773(config, m_wd17xx, 8_MHz_XTAL);
	m_wd17xx->intrq_wr_callback().set(FUNC(coco_fdc_device_base::fdc_intrq_w));
	m_wd17xx->drq_wr_callback().set(FUNC(coco_fdc_device_base::fdc_drq_w));
	m_wd17xx->set_disable_motor_control(true);
	m_wd17xx->set_force_ready(true);

	FLOPPY_CONNECTOR(config, m_floppies[0], coco_fdc_floppies, "525dd", coco_fdc_device_base::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[1], coco_fdc_floppies, "525dd", coco_fdc_device_base::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[2], coco_fdc_floppies, nullptr, coco_fdc_device_base::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[3], coco_fdc_floppies, nullptr, coco_fdc_device_base::floppy_formats).enable_sound(true);

	MSM6242(config, m_disto_msm6242, 32.768_kHz_XTAL);

	DS1315(config, CLOUD9_TAG, 0);

	RAM(config, "cachebuffer").set_default_size("256").set_default_value(0);
}


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

u8 *coco_family_fdc_device_base::get_cart_base()
{
	return memregion("eprom")->base();
}


//-------------------------------------------------
//  coco_family_fdc_device_base::get_cart_memregion
//-------------------------------------------------

memory_region *coco_family_fdc_device_base::get_cart_memregion()
{
	return memregion("eprom");
}


//***************************************************************************
//  COCO FDCs
//***************************************************************************

//-------------------------------------------------
//  coco_fdc_device_base - constructor
//-------------------------------------------------

coco_fdc_device_base::coco_fdc_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: coco_family_fdc_device_base(mconfig, type, tag, owner, clock)
	, m_wd17xx(*this, WD_TAG)
	, m_ds1315(*this, CLOUD9_TAG)
	, m_floppies(*this, WD_TAG ":%u", 0)
	, m_disto_msm6242(*this, DISTO_TAG)
	, m_msm6242_rtc_address(0)
	, m_rtc(*this, ":real_time_clock")
	, m_cache_buffer(*this, "cachebuffer")
{
}


//-------------------------------------------------
//  device_start - device-specific start
//-------------------------------------------------

void coco_fdc_device_base::device_start()
{
	coco_family_fdc_device_base::device_start();

	install_readwrite_handler(0xFF74, 0xFF76,
			read8sm_delegate(*this, FUNC(coco_fdc_device_base::ff74_read)),
			write8sm_delegate(*this, FUNC(coco_fdc_device_base::ff74_write)));

	save_item(NAME(m_cache_controler));
	save_item(NAME(m_cache_pointer));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void coco_fdc_device_base::device_reset()
{
	coco_family_fdc_device_base::device_reset();

	m_cache_controler = 0x80;
	m_cache_pointer = 0;
}


//-------------------------------------------------
//  ff74_read - no halt registers
//-------------------------------------------------

u8 coco_fdc_device_base::ff74_read(offs_t offset)
{
	u8 data = 0x0;

	switch(offset)
	{
		case 0x0:
			data = m_cache_buffer->read(m_cache_pointer++);
			LOG( "CachDat_A read: %2.2x\n", data );
			break;
		case 0x1:
			data = m_cache_buffer->read(m_cache_pointer++);
			LOG( "CachDat_B read: %2.2x\n", data );
			break;
		case 0x2:
			data = m_cache_controler;
			LOG( "CachCtrl read: %2.2x\n", data );
			break;
	}

	return data;
}


//-------------------------------------------------
//  ff74_write - no halt registers
//-------------------------------------------------

void coco_fdc_device_base::ff74_write(offs_t offset, u8 data)
{
	switch(offset)
	{
		case 0x0:
			LOG( "CachDat_A write: %2.2x\n", data );
			m_cache_buffer->write(m_cache_pointer++, data);
			break;
		case 0x1:
			LOG( "CachDat_B write: %2.2x\n", data );
			m_cache_buffer->write(m_cache_pointer++, data);
			break;
		case 0x2:
			LOG( "CachCtrl write: %2.2x\n", data );

			// reset static ram buffer pointer on any write
			m_cache_pointer = 0;

			if(data == 0)
			{
				// Clear interrupt when caching is turned off
				set_line_value(line::CART, CLEAR_LINE);
				m_cache_controler |= 0x80;
			}

			m_cache_controler = (m_cache_controler & 0x80) | (data & 0x7f);
			break;
	}
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
//  update_lines - CoCo specific disk
//  controller lines
//-------------------------------------------------

void coco_fdc_device_base::update_lines()
{
	if( (m_cache_controler & 0x7f) == 0) /* cache disabled */
	{
		// clear HALT enable under certain circumstances
		if (intrq() && (dskreg() & 0x20))
			set_dskreg(dskreg() & ~0x80);  // clear halt enable

		// set the NMI line
		set_line_value(line::NMI, intrq() && (dskreg() & 0x20));

		// set the HALT line
		set_line_value(line::HALT, !drq() && (dskreg() & 0x80));
	}
	else
	{
		if( drq() == ASSERT_LINE)
		{
			if( (m_cache_controler & 0x07) == 0x07) /* Read cache on */
			{
				u8 data = m_wd17xx->data_r();
				LOG("Cached drq read: %2.2x\n", data );
				m_cache_buffer->write(m_cache_pointer++, data);
			}
			else if( (m_cache_controler & 0x07) == 0x04 ) /* Write cache on */
			{
				u8 data = m_cache_buffer->read(m_cache_pointer++);
				LOG("Cached drq write: %2.2x\n", data );
				m_wd17xx->data_w(data);
			}
			else if( (m_cache_controler & 0x07) == 0x06 ) /* Copy Write cache to controller */
			{
				u8 data = m_cache_buffer->read(m_cache_pointer++);
				LOG("Cached copy drq write: %2.2x\n", data );
				m_wd17xx->data_w(data);
			}
			else
			{
				LOG("illegal DRQ cached assert mode\n" );
			}
		}

		if( (m_cache_controler & 0x08) == 0x08)
		{
			set_line_value(line::CART, intrq());
		}

		if( intrq() == ASSERT_LINE)
		{
			m_cache_controler &= 0x7f;
		}
		else
		{
			m_cache_controler |= 0x80;
		}
	}
}


//-------------------------------------------------
//  dskreg_w - function to write to CoCo dskreg
//-------------------------------------------------

void coco_fdc_device_base::dskreg_w(u8 data)
{
	u8 drive = 0;
	u8 head;

	LOG("fdc_coco_dskreg_w(): %c%c%c%c%c%c%c%c ($%02x)\n",
		data & 0x80 ? 'H' : 'h',
		data & 0x40 ? '3' : '.',
		data & 0x20 ? 'D' : 'S',
		data & 0x10 ? 'P' : 'p',
		data & 0x08 ? 'M' : 'm',
		data & 0x04 ? '2' : '.',
		data & 0x02 ? '1' : '.',
		data & 0x01 ? '0' : '.',
		data);

	// An email from John Kowalski informed me that if the DS3 is
	// high, and one of the other drive bits is selected (DS0-DS2), then the
	// second side of DS0, DS1, or DS2 is selected.  If multiple bits are
	// selected in other situations, then both drives are selected, and any
	// read signals get yucky.

	if (data & 0x04)
		drive = 2;
	else if (data & 0x02)
		drive = 1;
	else if (data & 0x01)
		drive = 0;
	else if (data & 0x40)
		drive = 3;

	// the motor is always turned on or off for all drives
	for (int i = 0; i < 4; i++)
	{
		floppy_image_device *floppy = m_floppies[i]->get_device();
		if (floppy)
			floppy->mon_w(BIT(data, 3) ? 0 : 1);
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
//  cts_read
//-------------------------------------------------

u8 coco_fdc_device_base::cts_read(offs_t offset)
{
	return memregion("eprom")->base()[offset];
}


//-------------------------------------------------
//  scs_read
//-------------------------------------------------

u8 coco_fdc_device_base::scs_read(offs_t offset)
{
	u8 result = 0;

	switch(offset & 0x1F)
	{
		case 8:
			result = m_wd17xx->status_r();
			LOGWDFDC("m_wd17xx->status_r: %2.2x\n", result );
			break;
		case 9:
			result = m_wd17xx->track_r();
			LOGWDFDC("m_wd17xx->track_r: %2.2x\n", result );
			break;
		case 10:
			result = m_wd17xx->sector_r();
			LOGWDFDC("m_wd17xx->sector_r: %2.2x\n", result );
			break;
		case 11:
			result = m_wd17xx->data_r();
			LOGWDIO("m_wd17xx->data_r: %2.2x\n", result );
			break;
	}

	/* other stuff for RTCs */
	switch (offset)
	{
		case 0x10:  /* FF50 */
			if (real_time_clock() == rtc_type::DISTO)
				result = m_disto_msm6242->read(m_msm6242_rtc_address);
			break;

		case 0x38:  /* FF78 */
			if (real_time_clock() == rtc_type::CLOUD9)
				m_ds1315->read_0();
			break;

		case 0x39:  /* FF79 */
			if (real_time_clock() == rtc_type::CLOUD9)
				m_ds1315->read_1();
			break;

		case 0x3C:  /* FF7C */
			if (real_time_clock() == rtc_type::CLOUD9)
				result = m_ds1315->read_data();
			break;
	}

	return result;
}


//-------------------------------------------------
//  scs_write
//-------------------------------------------------

void coco_fdc_device_base::scs_write(offs_t offset, u8 data)
{
	switch(offset & 0x1F)
	{
		case 0: case 1: case 2: case 3:
		case 4: case 5: case 6: case 7:
			dskreg_w(data);
			break;
		case 8:
			LOGWDFDC("m_wd17xx->cmd_w: %2.2x\n", data );
			m_wd17xx->cmd_w(data);
			break;
		case 9:
			LOGWDFDC("m_wd17xx->track_w: %2.2x\n", data );
			m_wd17xx->track_w(data);
			break;
		case 10:
			LOGWDFDC("m_wd17xx->sector_w: %2.2x\n", data );
			m_wd17xx->sector_w(data);
			break;
		case 11:
			LOGWDIO("m_wd17xx->data_w: %2.2x\n", data );
			m_wd17xx->data_w(data);
			break;
	};

	/* other stuff for RTCs */
	switch(offset)
	{
		case 0x10:  /* FF50 */
			if (real_time_clock() == rtc_type::DISTO)
				m_disto_msm6242->write(m_msm6242_rtc_address, data);
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
	ROM_LOAD("disk10.rom", 0x0000, 0x2000, CRC(b4f9968e) SHA1(04115be3f97952b9d9310b52f806d04f80b40d03))
ROM_END

namespace
{
	class coco_fdc_device : public coco_fdc_device_base
	{
	public:
		// construction/destruction
		coco_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
			: coco_fdc_device_base(mconfig, COCO_FDC, tag, owner, clock)
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

DEFINE_DEVICE_TYPE_PRIVATE(COCO_FDC, coco_family_fdc_device_base, coco_fdc_device, "coco_fdc", "CoCo FDC")


//**************************************************************************
//              COCO FDC v1.1
//**************************************************************************

ROM_START(coco_fdc_v11)
	ROM_REGION(0x8000, "eprom", ROMREGION_ERASE00)
	ROM_LOAD("disk11.rom", 0x0000, 0x2000, CRC(0b9c5415) SHA1(10bdc5aa2d7d7f205f67b47b19003a4bd89defd1))
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
		coco_fdc_v11_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
			: coco_fdc_device_base(mconfig, COCO_FDC_V11, tag, owner, clock)
		{
		}

	protected:
		// optional information overrides
		virtual const tiny_rom_entry *device_rom_region() const override
		{
			return ROM_NAME(coco_fdc_v11);
		}
	};
}

DEFINE_DEVICE_TYPE_PRIVATE(COCO_FDC_V11, coco_family_fdc_device_base, coco_fdc_v11_device, "coco_fdc_v11", "CoCo FDC v1.1")


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
		coco3_hdb1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
			: coco_fdc_device_base(mconfig, COCO3_HDB1, tag, owner, clock)
		{
		}

	protected:
		// optional information overrides
		virtual const tiny_rom_entry *device_rom_region() const override
		{
			return ROM_NAME(coco3_hdb1);
		}
	};
}

DEFINE_DEVICE_TYPE_PRIVATE(COCO3_HDB1, coco_family_fdc_device_base, coco3_hdb1_device, "coco3_hdb1", "CoCo3 HDB-DOS")

//**************************************************************************
//              COCO-2 HDB-DOS
//**************************************************************************

ROM_START(coco2_hdb1)
	ROM_REGION(0x8000, "eprom", ROMREGION_ERASE00)
	ROM_LOAD("hdbdw3bck.rom", 0x0000, 0x2000, CRC(867a3f42) SHA1(8fd64f1c246489e0bf2b3743ae76332ff324716a))
	ROM_RELOAD(0x2000, 0x2000)
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x6000, 0x2000)
ROM_END

namespace
{
	class coco2_hdb1_device : public coco_fdc_device_base
	{
	public:
		// construction/destruction
		coco2_hdb1_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
			: coco_fdc_device_base(mconfig, COCO2_HDB1, tag, owner, clock)
		{
		}

	protected:
		// optional information overrides
		virtual const tiny_rom_entry *device_rom_region() const override
		{
			return ROM_NAME(coco2_hdb1);
		}
	};
}

DEFINE_DEVICE_TYPE_PRIVATE(COCO2_HDB1, coco_family_fdc_device_base, coco2_hdb1_device, "coco2_hdb1", "CoCo2 HDB-DOS")

//**************************************************************************
//              Prológica CP-450 BASIC Disco V. 1.0 (1984)
//
//  There is a photo of the CP-450 disk controller unit at:
//  https://datassette.org/softwares/tandy-trs-color/cp-450-basic-disco-v-10
//  http://files.datassette.org/softwares/img/wp_20141212_22_08_26_pro.jpg
//
//**************************************************************************

ROM_START(cp450_fdc)
	ROM_REGION(0x4000, "eprom", ROMREGION_ERASE00)
	ROM_LOAD("cp450_basic_disco_v1.0.rom", 0x0000, 0x2000, CRC(e9ad60a0) SHA1(827697fa5b755f5dc1efb054cdbbeb04e405405b))
ROM_END

namespace
{
	class cp450_fdc_device : public coco_fdc_device_base
	{
	public:
		// construction/destruction
		cp450_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
			: coco_fdc_device_base(mconfig, CP450_FDC, tag, owner, clock)
		{
		}

	protected:
		// optional information overrides
		virtual const tiny_rom_entry *device_rom_region() const override
		{
			return ROM_NAME(cp450_fdc);
		}
	};
}

DEFINE_DEVICE_TYPE_PRIVATE(CP450_FDC, coco_family_fdc_device_base, cp450_fdc_device, "cp450_fdc", "Prológica CP-450 BASIC Disco V. 1.0 (1984)")

//**************************************************************************
//              Codimex CD-6809 FDC (1986)
//
// Seems to be a clone of the JFD-COCO originally manufactured by J&M
// More ifo at: http://amxproject.com/?p=2747
//**************************************************************************

ROM_START(cd6809_fdc)
	ROM_REGION(0x4000, "eprom", ROMREGION_ERASE00)
	ROM_LOAD("cd6809dsk.u16", 0x0000, 0x2000, CRC(3c35bda8) SHA1(9b2eec25188bed4326b84739a666435884e4ddf4))
ROM_END

namespace
{
	class cd6809_fdc_device : public coco_fdc_device_base
	{
	public:
		// construction/destruction
		cd6809_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
			: coco_fdc_device_base(mconfig, CD6809_FDC, tag, owner, clock)
		{
		}

	protected:
		// optional information overrides
		virtual const tiny_rom_entry *device_rom_region() const override
		{
			return ROM_NAME(cd6809_fdc);
		}
	};
}

DEFINE_DEVICE_TYPE_PRIVATE(CD6809_FDC, coco_family_fdc_device_base, cd6809_fdc_device, "cd6809_fdc", "Codimex CD-6809 Disk BASIC (1986)")
