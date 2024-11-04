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
    on the Dragon. In addition, there is another register called DSKREG that
    controls the interface with the wd1793. DSKREG is detailed below:

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

    Disto Super Controller II
        Mini Expansion Bus
        No Halt Extension. Implemented by using a read and write cache.

            CachDat - Cache Data Register
                $FF74 & $FF75: Read/Write cache data.

                Normally cache size is 256 bytes. But a common upgrade is to change
                it to 512 bytes.

            CachCtrl - Cache Controller
                $FF76 & $FF77:
                    write
                        clears buffer counter

                    read bit:
                        7:  0 - indicates an interrupt request from the disk controller
                            1 - no interrupt
                        3:  0 - disable CART / FIRQ (masked)
                            1 - enable CART / FIRQ
                        2:  0 - normal NMI
                            1 - masked NMI
                        1:  0 - compatible I/O mode
                            1 - buffered I/O mode
                        0:  0 - buffered write operation
                            1 - buffered read operation

        There was a jumper to set the addresses to be FF58 or FF5B. Not implemented.

*********************************************************************/

#include "emu.h"
#include "coco_fdc.h"

#include "meb_intrf.h"

#include "imagedev/floppy.h"
#include "machine/ds1315.h"
#include "machine/input_merger.h"
#include "machine/msm6242.h"
#include "machine/wd_fdc.h"

#include "formats/dmk_dsk.h"
#include "formats/flex_dsk.h"
#include "formats/fs_coco_os9.h"
#include "formats/fs_coco_rsdos.h"
#include "formats/jvc_dsk.h"
#include "formats/os9_dsk.h"
#include "formats/sdf_dsk.h"
#include "formats/vdk_dsk.h"

#define LOG_WDFDC   (1U << 1) // Shows register setup
#define LOG_WDIO    (1U << 2) // Shows data read and write
#define LOG_WDSCII  (1U << 3) // Shows SCII register setup

//#define VERBOSE (LOG_GENERAL|LOG_WDFDC|LOG_WDSCII)
#include "logmacro.h"

#define LOGWDFDC(...)   LOGMASKED(LOG_WDFDC,    __VA_ARGS__)
#define LOGWDIO(...)    LOGMASKED(LOG_WDIO,     __VA_ARGS__)
#define LOGWDSCII(...)  LOGMASKED(LOG_WDSCII,   __VA_ARGS__)


/***************************************************************************
    PARAMETERS
***************************************************************************/

#define WD_TAG                  "wd17xx"
#define WD2797_TAG              "wd2797"
#define MEB_TAG                 "meb"

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

	// device-level overrides
	virtual u8 cts_read(offs_t offset) override;
	virtual u8 scs_read(offs_t offset) override;
	virtual void scs_write(offs_t offset, u8 data) override;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// methods
	virtual void update_lines() override;
	void dskreg_w(u8 data);

	// devices
	required_device<wd1773_device>              m_wd17xx;
	required_device_array<floppy_connector, 4>  m_floppies;
};


/***************************************************************************
    LOCAL VARIABLES
***************************************************************************/

void coco_family_fdc_device_base::floppy_formats(format_registration &fr)
{
	fr.add_mfm_containers();
	fr.add(FLOPPY_FLEX_FORMAT);
	fr.add(FLOPPY_DMK_FORMAT);
	fr.add(FLOPPY_SDF_FORMAT);
	fr.add(FLOPPY_JVC_FORMAT);
	fr.add(FLOPPY_VDK_FORMAT);
	fr.add(FLOPPY_OS9_FORMAT);
	fr.add(fs::COCO_RSDOS);
	fr.add(fs::COCO_OS9);
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
	, m_floppies(*this, WD_TAG ":%u", 0)
{
}


//-------------------------------------------------
//  update_lines - CoCo specific disk
//  controller lines
//-------------------------------------------------

void coco_fdc_device_base::update_lines()
{
	// clear HALT enable under certain circumstances
	if (intrq())
		set_dskreg(dskreg() & ~0x80);  // clear halt enable

	// set the NMI line
	set_line_value(line::NMI, intrq() && (dskreg() & 0x20));

	// set the HALT line
	set_line_value(line::HALT, !drq() && (dskreg() & 0x80));
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
	// second side of DS0, DS1, or DS2 is selected. If multiple bits are
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
			LOGWDFDC("m_wd17xx->status_r: %02x\n", result );
			break;
		case 9:
			result = m_wd17xx->track_r();
			LOGWDFDC("m_wd17xx->track_r: %02x\n", result );
			break;
		case 10:
			result = m_wd17xx->sector_r();
			LOGWDFDC("m_wd17xx->sector_r: %02x\n", result );
			break;
		case 11:
			result = m_wd17xx->data_r();
			LOGWDIO("m_wd17xx->data_r: %02x\n", result );
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
			LOGWDFDC("m_wd17xx->cmd_w: %02x\n", data );
			m_wd17xx->cmd_w(data);
			break;
		case 9:
			LOGWDFDC("m_wd17xx->track_w: %02x\n", data );
			m_wd17xx->track_w(data);
			break;
		case 10:
			LOGWDFDC("m_wd17xx->sector_w: %02x\n", data );
			m_wd17xx->sector_w(data);
			break;
		case 11:
			LOGWDIO("m_wd17xx->data_w: %02x\n", data );
			m_wd17xx->data_w(data);
			break;
	};
}


//**************************************************************************
//  CoCo DOS ROMs
//**************************************************************************

ROM_START(coco_fdc)
	ROM_REGION(0x8000, "eprom", ROMREGION_ERASE00)
	ROM_DEFAULT_BIOS("v11")

	ROM_SYSTEM_BIOS(0, "v10", "RSDOS v1.0")
	ROMX_LOAD("disk10.rom", 0x0000, 0x2000, CRC(b4f9968e) SHA1(04115be3f97952b9d9310b52f806d04f80b40d03), ROM_BIOS(0))
	ROM_RELOAD(0x2000,0x2000)
	ROM_RELOAD(0x4000,0x2000)
	ROM_RELOAD(0x6000,0x2000)

	ROM_SYSTEM_BIOS(1, "v11", "RSDOS v1.1")
	ROMX_LOAD("disk11.rom", 0x0000, 0x2000, CRC(0b9c5415) SHA1(10bdc5aa2d7d7f205f67b47b19003a4bd89defd1), ROM_BIOS(1))
	ROM_RELOAD(0x2000,0x2000)
	ROM_RELOAD(0x4000,0x2000)
	ROM_RELOAD(0x6000,0x2000)

	ROM_SYSTEM_BIOS(2, "ados", "ADOS v1.02 for CoCo 1/2")
	ROMX_LOAD("ados.rom", 0x0000, 0x2000, CRC(24e807cf) SHA1(a935ea11af4c600a771e4540b661cbb4258a21d6), ROM_BIOS(2))
	ROM_RELOAD(0x2000,0x2000)
	ROM_RELOAD(0x4000,0x2000)
	ROM_RELOAD(0x6000,0x2000)

	ROM_SYSTEM_BIOS(3, "ados2b", "ADOS v1.02 for CoCo 2B")
	ROMX_LOAD("ados2b.rom", 0x0000, 0x2000, CRC(47a59ad4) SHA1(66bd3cf08e7f1b318e82e8c4a9848233f38f1a56), ROM_BIOS(3))
	ROM_RELOAD(0x2000,0x2000)
	ROM_RELOAD(0x4000,0x2000)
	ROM_RELOAD(0x6000,0x2000)

	ROM_SYSTEM_BIOS(4, "ados3", "ADOS3 v1.01.01")
	ROMX_LOAD("ados3.rom", 0x0000, 0x2000, CRC(6f824cd1) SHA1(de602d8d219094f1237d37a4f80032c25808b358), ROM_BIOS(4))
	ROM_RELOAD(0x2000,0x2000)
	ROM_RELOAD(0x4000,0x2000)
	ROM_RELOAD(0x6000,0x2000)

	ROM_SYSTEM_BIOS(5, "ados340", "ADOS3 v1.01.01, 40 track disk drives")
	ROMX_LOAD("ados3-40.rom", 0x0000, 0x2000, CRC(8afe1a04) SHA1(a8dcc6fc0aa5589612cea2a318e7fae58d930c6c), ROM_BIOS(5))
	ROM_RELOAD(0x2000,0x2000)
	ROM_RELOAD(0x4000,0x2000)
	ROM_RELOAD(0x6000,0x2000)

	ROM_SYSTEM_BIOS(6, "ados380", "ADOS3 v1.01.01, 80 track disk drives")
	ROMX_LOAD("ados3-80.rom", 0x0000, 0x2000, CRC(859762f5) SHA1(957f7d5a10e61266193b636dbf642002bcedfaa3), ROM_BIOS(6))
	ROM_RELOAD(0x2000,0x2000)
	ROM_RELOAD(0x4000,0x2000)
	ROM_RELOAD(0x6000,0x2000)

	ROM_SYSTEM_BIOS(7, "rgbdos", "Hard Disk Basic for Emudsk")
	ROMX_LOAD("rgbdos_mess.rom", 0x0000, 0x2000, CRC(0b0e64db) SHA1(062ffab14dc788ec7744e528bf9bb425c3ec60ed), ROM_BIOS(7))
	ROM_RELOAD(0x2000,0x2000)
	ROM_RELOAD(0x4000,0x2000)
	ROM_RELOAD(0x6000,0x2000)

	ROM_SYSTEM_BIOS(8, "hdbk12", "Hard Disk Basic for Becker Port and DriveWire 3, CoCo 1/2")
	ROMX_LOAD("hdbdw3bck.rom", 0x0000, 0x2000, CRC(867a3f42) SHA1(8fd64f1c246489e0bf2b3743ae76332ff324716a), ROM_BIOS(8))
	ROM_RELOAD(0x2000,0x2000)
	ROM_RELOAD(0x4000,0x2000)
	ROM_RELOAD(0x6000,0x2000)

	ROM_SYSTEM_BIOS(9, "hdbk3", "Hard Disk Basic for Becker Port and DriveWire 3, CoCo 3")
	ROMX_LOAD("hdbdw3bc3.rom", 0x0000, 0x2000, CRC(309a9efd) SHA1(671605d61811953860466f771c1594bbade331f4), ROM_BIOS(9))
	ROM_RELOAD(0x2000,0x2000)
	ROM_RELOAD(0x4000,0x2000)
	ROM_RELOAD(0x6000,0x2000)
ROM_END

ROM_START(coco_scii)
	ROM_REGION(0x8000, "eprom", ROMREGION_ERASE00)
	ROM_DEFAULT_BIOS("cdos3")

	ROM_SYSTEM_BIOS(0, "cdos", "Disto C-DOS v4.0 for the CoCo 1/2")
	ROMX_LOAD("cdos 4 4-6-89 cc1.bin", 0x0000, 0x4000, CRC(9da6db28) SHA1(2cc6e275178ca8d8f281d845792fb0ae069aaeda), ROM_BIOS(0))
	ROM_RELOAD(0x4000,0x4000)

	ROM_SYSTEM_BIOS(1, "cdos3", "Disto C-DOS 3 v1.2 for the CoCo 3")
	ROMX_LOAD("cdos 1_2 3-30-89 cc3.bin", 0x0000, 0x4000, CRC(891c0094) SHA1(c1fa0fcbf1202a9b63aafd98dce777b502584230), ROM_BIOS(1))
	ROM_RELOAD(0x4000,0x4000)
ROM_END

ROM_START(cp450_fdc)
	ROM_REGION(0x4000, "eprom", ROMREGION_ERASE00)
	ROM_LOAD("cp450_basic_disco_v1.0.rom", 0x0000, 0x2000, CRC(e9ad60a0) SHA1(827697fa5b755f5dc1efb054cdbbeb04e405405b))
ROM_END

ROM_START(cd6809_fdc)
	ROM_REGION(0x4000, "eprom", ROMREGION_ERASE00)
	ROM_LOAD("cd6809dsk.u16", 0x0000, 0x2000, CRC(3c35bda8) SHA1(9b2eec25188bed4326b84739a666435884e4ddf4))
ROM_END


//**************************************************************************
//  COCO Floppy Disk Controller
//**************************************************************************

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
		// device_t implementation
		virtual const tiny_rom_entry *device_rom_region() const override
		{
			return ROM_NAME(coco_fdc);
		}
	};


//**************************************************************************
//              Disto / CRC Super Controller II Base
//**************************************************************************

	class coco_scii_device
		: public coco_fdc_device_base
	{
	public:
		// construction/destruction
			// construction/destruction
		coco_scii_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
			: coco_fdc_device_base(mconfig, COCO_SCII, tag, owner, clock)
			, m_slot(*this, MEB_TAG)
			, m_carts(*this, "cart_line")
		{
		}

	protected:
		// device_t implementation
		virtual void device_start() override ATTR_COLD;
		virtual void device_reset() override ATTR_COLD;
		virtual u8 scs_read(offs_t offset) override;
		virtual void scs_write(offs_t offset, u8 data) override;

		virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

		virtual const tiny_rom_entry *device_rom_region() const override
		{
			return ROM_NAME(coco_scii);
		}

		// methods
		virtual void update_lines() override;

		// Disto no halt registers
		u8 ff74_read(offs_t offset);
		void ff74_write(offs_t offset, u8 data);

		// device references
		required_device<distomeb_slot_device> m_slot;
		required_device<input_merger_device> m_carts;

	private:
		// registers
		std::unique_ptr<uint8_t[]> m_cache;
		u8 m_cache_pointer;
		u8 m_cache_controler;
	};


//-------------------------------------------------
//  device_start - device-specific start
//-------------------------------------------------

	void coco_scii_device::device_start()
	{
		coco_family_fdc_device_base::device_start();

		m_cache = std::make_unique<uint8_t[]>(0x200);

		install_readwrite_handler(0xFF74, 0xFF77,
				read8sm_delegate(*this, FUNC(coco_scii_device::ff74_read)),
				write8sm_delegate(*this, FUNC(coco_scii_device::ff74_write)));

		save_pointer(NAME(m_cache), 0x200);
		save_item(NAME(m_cache_pointer));
		save_item(NAME(m_cache_controler));
	}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

	void coco_scii_device::device_reset()
	{
		coco_family_fdc_device_base::device_reset();

		m_cache_controler = 0x80;
		m_cache_pointer = 0;
	}

	static void disto_meb_slot(device_slot_interface &device)
	{
		disto_meb_add_basic_devices(device);
	}


//-------------------------------------------------
//  device_add_mconfig - device-specific machine config
//-------------------------------------------------

	void coco_scii_device::device_add_mconfig(machine_config &config)
	{
		coco_fdc_device_base::device_add_mconfig(config);

		INPUT_MERGER_ANY_HIGH(config, m_carts).output_handler().set([this](int state) { set_line_value(line::CART, state); });

		DISTOMEB_SLOT(config, m_slot, DERIVED_CLOCK(1, 1), disto_meb_slot, "rtime");
		m_slot->cart_callback().set(m_carts, FUNC(input_merger_device::in_w<1>));
	}


//-------------------------------------------------
//  scs_read
//-------------------------------------------------

	u8 coco_scii_device::scs_read(offs_t offset)
	{
		if (offset > 0x0f && offset < 0x18)
			return m_slot->meb_read(offset - 0x10);

		return coco_fdc_device_base::scs_read(offset);
	}


//-------------------------------------------------
//  scs_write
//-------------------------------------------------

	void coco_scii_device::scs_write(offs_t offset, u8 data)
	{
		if (offset > 0x0f && offset < 0x18)
			m_slot->meb_write(offset - 0x10, data);
		else
		{
			coco_fdc_device_base::scs_write(offset, data);
		}
	}


//-------------------------------------------------
//  update_lines - SCII controller lines
//-------------------------------------------------

	void coco_scii_device::update_lines()
	{
		// clear HALT enable under certain circumstances
		if (intrq())
			set_dskreg(dskreg() & ~0x80);  // clear halt enable

		if ((m_cache_controler & 0x02) == 0) /* cache disabled */
		{
			// set the HALT line
			set_line_value(line::HALT, !drq() && (dskreg() & 0x80));
		}
		else
		{
			set_line_value(line::HALT, CLEAR_LINE);

			if (drq() == ASSERT_LINE)
			{
				if ((m_cache_controler & 0x01) == 0x01) /* Read cache on */
				{
					u8 data = m_wd17xx->data_r();
					LOGWDSCII("cache drq read: %02x\n", data );
					m_cache[m_cache_pointer++] = data;
				}
				else /* Write cache on */
				{
					u8 data = m_cache[m_cache_pointer++];
					LOGWDSCII("cache drq write: %02x\n", data );
					m_wd17xx->data_w(data);
				}

				m_cache_pointer &= 0x1ff;
			}
		}

		if ((m_cache_controler & 0x08) == 0x08)
		{
			m_carts->in_w<0>(intrq());
		}
		else
		{
			m_carts->in_w<0>(CLEAR_LINE);
		}

		if ((m_cache_controler & 0x04) == 0x00)
		{
			set_line_value(line::NMI, intrq() && (dskreg() & 0x20));
		}
		else
		{
			set_line_value(line::NMI, CLEAR_LINE);
		}
	}


//-------------------------------------------------
//  ff74_read - no halt registers
//-------------------------------------------------

	u8 coco_scii_device::ff74_read(offs_t offset)
	{
		u8 data = 0x0;

		switch(offset)
		{
			case 0x0:
			case 0x1:
				data = m_cache[m_cache_pointer++];
				LOGWDSCII("cache read: %04x = %02x\n", m_cache_pointer, data);
				m_cache_pointer &= 0x1ff;
				break;

			case 0x2:
			case 0x3:
				if (intrq() == ASSERT_LINE)
				{
					m_cache_controler &= 0x7f;
				}
				else
				{
					m_cache_controler |= 0x80;
				}

				data = m_cache_controler;
				LOGWDSCII("control read:  %02x\n", data);
				break;
		}

		return data;
	}


//-------------------------------------------------
//  ff74_write - no halt registers
//-------------------------------------------------

	void coco_scii_device::ff74_write(offs_t offset, u8 data)
	{
		switch(offset)
		{
			case 0x0:
			case 0x1:
				LOGWDSCII("cache write: %04x = %02x\n", m_cache_pointer, data);
				m_cache[m_cache_pointer++] = data;
				m_cache_pointer &= 0x1ff;
				break;
			case 0x2:
			case 0x3:
				LOGWDSCII("control write: %02x\n", data);

				// reset static ram buffer pointer on any write
				m_cache_pointer = 0;

				m_cache_controler = data;
				update_lines();
				break;
		}
	}


//**************************************************************************
//              Prológica CP-450 BASIC Disco V. 1.0 (1984)
//
//  There is a photo of the CP-450 disk controller unit at:
//  https://datassette.org/softwares/tandy-trs-color/cp-450-basic-disco-v-10
//  http://files.datassette.org/softwares/img/wp_20141212_22_08_26_pro.jpg
//
//**************************************************************************

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


//**************************************************************************
//              Codimex CD-6809 FDC (1986)
//
// Seems to be a clone of the JFD-COCO originally manufactured by J&M
// More ifo at: http://amxproject.com/?p=2747
//**************************************************************************

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
} // Anonymous namepace

DEFINE_DEVICE_TYPE_PRIVATE(COCO_FDC, coco_family_fdc_device_base, coco_fdc_device, "coco_fdc", "CoCo Floppy Disk Controller")
DEFINE_DEVICE_TYPE_PRIVATE(COCO_SCII, coco_family_fdc_device_base, coco_scii_device, "coco_scii", "Disto Super Controller II")
DEFINE_DEVICE_TYPE_PRIVATE(CP450_FDC, coco_family_fdc_device_base, cp450_fdc_device, "cp450_fdc", "Prológica CP-450 BASIC Disco V. 1.0 (1984)")
DEFINE_DEVICE_TYPE_PRIVATE(CD6809_FDC, coco_family_fdc_device_base, cd6809_fdc_device, "cd6809_fdc", "Codimex CD-6809 Disk BASIC (1986)")
