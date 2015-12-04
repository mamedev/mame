// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    coco_fdc.c

    CoCo/Dragon FDC

    The CoCo and Dragon both use the Western Digital floppy disk controllers.
    The CoCo uses either the WD1793 or the WD1773, the Dragon uses the WD2797,
    which mostly uses the same command set with some subtle differences, most
    notably the 2797 handles disk side select internally. The Dragon Alpha also
    uses the WD2797, however as this is a built in interface and not an external
    cartrige, it is dealt with in the main coco.c file.

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
    CoCo ($FF40)                                    Dragon ($FF48)

    Bit                                              Bit
    7 halt enable flag                               7 not used
    6 drive select #3                                6 not used
    5 density (0=single, 1=double)                   5 NMI enable flag
        and NMI enable flag
    4 write precompensation                          4 write precompensation
    3 drive motor activation                         3 single density enable
    2 drive select #2                                2 drive motor activation
    1 drive select #1                                1 drive select high bit
    0 drive select #0                                0 drive select low bit

    Reading from $FF48-$FF4F clears bit 7 of DSKREG ($FF40)

    ---------------------------------------------------------------------------

    2007-02-22, P.Harvey-Smith

    Began implementing the Dragon Delta Dos controler, this was actually the first
    Dragon disk controler to market, beating Dragon Data's by a couple of months,
    it is based around the WD2791 FDC, which is compatible with the WD1793/WD2797 used
    by the standard CoCo and Dragon disk controlers except that it used an inverted
    data bus, which is the reason the read/write handlers invert the data. This
    controler like, the DragonDos WD2797 is mapped at $FF40-$FF43, in the normal
    register order.

    The Delta cart also has a register (74LS174 hex flipflop) at $FF44 encoded as
    follows :-

    Bit
    7 not used
    6 not used
    5 not used
    4 Single (0) / Double (1) density select
    3 5.25"(0) / 8"(1) Clock select
    2 Side select
    1 Drive select ms bit
    0 Drive select ls bit

*********************************************************************/

#include "emu.h"
#include "coco_fdc.h"
#include "imagedev/flopdrv.h"
#include "includes/coco.h"
#include "imagedev/flopdrv.h"
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

/***************************************************************************
    LOCAL VARIABLES
***************************************************************************/

FLOPPY_FORMATS_MEMBER( coco_fdc_device::floppy_formats )
	FLOPPY_DMK_FORMAT,
	FLOPPY_JVC_FORMAT,
	FLOPPY_VDK_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( coco_fdc_floppies )
	SLOT_INTERFACE("qd", FLOPPY_525_QD)
SLOT_INTERFACE_END


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/
/*-------------------------------------------------
    real_time_clock
-------------------------------------------------*/

coco_rtc_type_t coco_fdc_device::real_time_clock()
{
	coco_rtc_type_t result = coco_rtc_type_t(read_safe(machine().root_device().ioport("real_time_clock"), RTC_NONE));

	/* check to make sure we don't have any invalid values */
	if (((result == RTC_DISTO) && (m_disto_msm6242 == nullptr))
		|| ((result == RTC_CLOUD9) && (m_ds1315 == nullptr)))
	{
		result = RTC_NONE;
	}

	return result;
}
/*-------------------------------------------------
    fdc_intrq_w - callback from the FDC
-------------------------------------------------*/

WRITE_LINE_MEMBER( coco_fdc_device::fdc_intrq_w )
{
	set_intrq(state);
	update_lines();
}


/*-------------------------------------------------
    fdc_drq_w - callback from the FDC
-------------------------------------------------*/

WRITE_LINE_MEMBER( coco_fdc_device::fdc_drq_w )
{
	set_drq(state);
	update_lines();
}


//**************************************************************************
//              COCO FDC
//**************************************************************************

static MACHINE_CONFIG_FRAGMENT(coco_fdc)
	MCFG_WD1773_ADD(WD_TAG, XTAL_8MHz)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(coco_fdc_device, fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(coco_fdc_device, fdc_drq_w))

	MCFG_FLOPPY_DRIVE_ADD(WD_TAG ":0", coco_fdc_floppies, "qd", coco_fdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(WD_TAG ":1", coco_fdc_floppies, "qd", coco_fdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(WD_TAG ":2", coco_fdc_floppies, "", coco_fdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(WD_TAG ":3", coco_fdc_floppies, "", coco_fdc_device::floppy_formats)

	MCFG_DEVICE_ADD(DISTO_TAG, MSM6242, XTAL_32_768kHz)
	MCFG_DS1315_ADD(CLOUD9_TAG)
MACHINE_CONFIG_END

ROM_START( coco_fdc )
	ROM_REGION(0x4000,"eprom",ROMREGION_ERASE00)
	ROM_LOAD_OPTIONAL(  "disk10.rom",   0x0000, 0x2000, CRC(b4f9968e) SHA1(04115be3f97952b9d9310b52f806d04f80b40d03))
ROM_END

const device_type COCO_FDC = &device_creator<coco_fdc_device>;

//-------------------------------------------------
//  coco_fdc_device - constructor
//-------------------------------------------------
coco_fdc_device::coco_fdc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: device_t(mconfig, type, name, tag, owner, clock, shortname, source),
		device_cococart_interface( mconfig, *this ), m_owner(nullptr), m_dskreg(0), m_drq(0), m_intrq(0),
		m_wd17xx(*this, WD_TAG),
		m_wd2797(*this, WD2797_TAG),
		m_ds1315(*this, CLOUD9_TAG),
		m_disto_msm6242(*this, DISTO_TAG), m_msm6242_rtc_address(0)
{
}

coco_fdc_device::coco_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: device_t(mconfig, COCO_FDC, "CoCo FDC", tag, owner, clock, "coco_fdc", __FILE__),
		device_cococart_interface( mconfig, *this ), m_owner(nullptr), m_dskreg(0), m_drq(0), m_intrq(0),
		m_wd17xx(*this, WD_TAG),
		m_wd2797(*this, WD2797_TAG),
		m_ds1315(*this, CLOUD9_TAG),
		m_disto_msm6242(*this, DISTO_TAG), m_msm6242_rtc_address(0)
	{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void coco_fdc_device::device_start()
{
	m_owner = dynamic_cast<cococart_slot_device *>(owner());
	m_drq               = 1;
	m_dskreg            = 0x00;
	m_intrq             = 0;
	m_msm6242_rtc_address = 0;
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor coco_fdc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( coco_fdc );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *coco_fdc_device::device_rom_region() const
{
	return ROM_NAME( coco_fdc );
}

/*-------------------------------------------------
    get_cart_base
-------------------------------------------------*/

UINT8* coco_fdc_device::get_cart_base()
{
	return memregion("eprom")->base();
}

/*-------------------------------------------------
    update_lines - CoCo specific disk
    controller lines
-------------------------------------------------*/

void coco_fdc_device::update_lines()
{
	/* clear HALT enable under certain circumstances */
	if ((m_intrq != 0) && (m_dskreg & 0x20))
		m_dskreg &= ~0x80;  /* clear halt enable */

	/* set the NMI line */
	m_owner->cart_set_line(COCOCART_LINE_NMI,
		((m_intrq != 0) && (m_dskreg & 0x20)) ? COCOCART_LINE_VALUE_ASSERT : COCOCART_LINE_VALUE_CLEAR);

	/* set the HALT line */
	m_owner->cart_set_line(COCOCART_LINE_HALT,
		((m_drq == 0) && (m_dskreg & 0x80)) ? COCOCART_LINE_VALUE_ASSERT : COCOCART_LINE_VALUE_CLEAR);
}

/*-------------------------------------------------
    dskreg_w - function to write to CoCo
    dskreg
-------------------------------------------------*/

void coco_fdc_device::dskreg_w(UINT8 data)
{
	UINT8 drive = 0;
	UINT8 head = 0;

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

	floppy_image_device *floppy[4];

	floppy[0] = subdevice<floppy_connector>(WD_TAG ":0")->get_device();
	floppy[1] = subdevice<floppy_connector>(WD_TAG ":1")->get_device();
	floppy[2] = subdevice<floppy_connector>(WD_TAG ":2")->get_device();
	floppy[3] = subdevice<floppy_connector>(WD_TAG ":3")->get_device();

	for (int i = 0; i < 4; i++)
	{
		if (floppy[i])
			floppy[i]->mon_w(i == drive ? CLEAR_LINE : ASSERT_LINE);
	}

	head = ((data & 0x40) && (drive != 3)) ? 1 : 0;

	m_dskreg = data;

	update_lines();

	m_wd17xx->set_floppy(floppy[drive]);

	if (floppy[drive])
		floppy[drive]->ss_w(head);

	m_wd17xx->dden_w(!BIT(m_dskreg, 5));
}

/*-------------------------------------------------
    read
-------------------------------------------------*/

READ8_MEMBER(coco_fdc_device::read)
{
	UINT8 result = 0;

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
			if (real_time_clock() == RTC_DISTO)
				result = m_disto_msm6242->read(space,m_msm6242_rtc_address);
			break;

		case 0x38:  /* FF78 */
			if (real_time_clock() == RTC_CLOUD9)
				m_ds1315->read_0(space, offset);
			break;

		case 0x39:  /* FF79 */
			if (real_time_clock() == RTC_CLOUD9)
				m_ds1315->read_1(space, offset);
			break;

		case 0x3C:  /* FF7C */
			if (real_time_clock() == RTC_CLOUD9)
				result = m_ds1315->read_data(space, offset);
			break;
	}
	return result;
}



/*-------------------------------------------------
    write
-------------------------------------------------*/

WRITE8_MEMBER(coco_fdc_device::write)
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
			if (real_time_clock() == RTC_DISTO)
				m_disto_msm6242->write(space,m_msm6242_rtc_address, data);
			break;

		case 0x11:  /* FF51 */
			if (real_time_clock() == RTC_DISTO)
				m_msm6242_rtc_address = data & 0x0f;
			break;
	}
}


//**************************************************************************
//              DRAGON FDC
//**************************************************************************

static MACHINE_CONFIG_FRAGMENT(dragon_fdc)
	MCFG_WD2797_ADD(WD2797_TAG, XTAL_1MHz)

	MCFG_FLOPPY_DRIVE_ADD(WD2797_TAG ":0", coco_fdc_floppies, "qd", coco_fdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(WD2797_TAG ":1", coco_fdc_floppies, "qd", coco_fdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(WD2797_TAG ":2", coco_fdc_floppies, "", coco_fdc_device::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD(WD2797_TAG ":3", coco_fdc_floppies, "", coco_fdc_device::floppy_formats)
MACHINE_CONFIG_END


ROM_START( dragon_fdc )
	ROM_REGION(0x4000,"eprom",ROMREGION_ERASE00)
	ROM_LOAD_OPTIONAL(  "ddos10.rom",   0x0000,  0x2000, CRC(b44536f6) SHA1(a8918c71d319237c1e3155bb38620acb114a80bc))
ROM_END

const device_type DRAGON_FDC = &device_creator<dragon_fdc_device>;

//-------------------------------------------------
//  dragon_fdc_device - constructor
//-------------------------------------------------
dragon_fdc_device::dragon_fdc_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: coco_fdc_device(mconfig, type, name, tag, owner, clock, shortname, source)
{
}
dragon_fdc_device::dragon_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: coco_fdc_device(mconfig, DRAGON_FDC, "Dragon FDC", tag, owner, clock, "dragon_fdc", __FILE__)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void dragon_fdc_device::device_start()
{
	m_owner = dynamic_cast<cococart_slot_device *>(owner());
	m_drq               = 0;
	m_dskreg            = 0x00;
	m_intrq             = 0;
	m_msm6242_rtc_address = 0;
}

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor dragon_fdc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( dragon_fdc );
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *dragon_fdc_device::device_rom_region() const
{
	return ROM_NAME( dragon_fdc );
}



/*-------------------------------------------------
    update_lines - Dragon specific disk
    controller lines
-------------------------------------------------*/

void dragon_fdc_device::update_lines()
{
	/* set the NMI line */
	m_owner->cart_set_line(COCOCART_LINE_NMI,
		((m_intrq != 0) && (m_dskreg & 0x20)) ? COCOCART_LINE_VALUE_ASSERT : COCOCART_LINE_VALUE_CLEAR);

	/* set the CART line */
	m_owner->cart_set_line(COCOCART_LINE_CART,
		(m_drq != 0) ? COCOCART_LINE_VALUE_ASSERT : COCOCART_LINE_VALUE_CLEAR);
}


/*-------------------------------------------------
    dskreg_w - function to write to
    Dragon dskreg
-------------------------------------------------*/

void dragon_fdc_device::dskreg_w(UINT8 data)
{
	if (LOG_FDC)
	{
		logerror("fdc_dragon_dskreg_w(): %c%c%c%c%c%c%c%c ($%02x)\n",
			data & 0x80 ? 'X' : 'x',
			data & 0x40 ? 'X' : 'x',
			data & 0x20 ? 'N' : 'n',
			data & 0x10 ? 'P' : 'p',
			data & 0x08 ? 'S' : 'D',
			data & 0x04 ? 'M' : 'm',
			data & 0x02 ? '1' : '0',
			data & 0x01 ? '1' : '0',
			data);
	}

	floppy_image_device *floppy = nullptr;

	switch (data & 0x03)
	{
	case 0: floppy = subdevice<floppy_connector>(WD2797_TAG ":0")->get_device(); break;
	case 1: floppy = subdevice<floppy_connector>(WD2797_TAG ":1")->get_device(); break;
	case 2: floppy = subdevice<floppy_connector>(WD2797_TAG ":2")->get_device(); break;
	case 3: floppy = subdevice<floppy_connector>(WD2797_TAG ":3")->get_device(); break;
	}

	m_wd2797->set_floppy(floppy);

	m_wd2797->dden_w(BIT(data, 3));

	m_dskreg = data;
}



/*-------------------------------------------------
    read
-------------------------------------------------*/

READ8_MEMBER(dragon_fdc_device::read)
{
	UINT8 result = 0;
	switch(offset & 0xEF)
	{
		case 0:
			result = m_wd2797->status_r(space, 0);
			break;
		case 1:
			result = m_wd2797->track_r(space, 0);
			break;
		case 2:
			result = m_wd2797->sector_r(space, 0);
			break;
		case 3:
			result = m_wd2797->data_r(space, 0);
			break;
	}
	return result;
}



/*-------------------------------------------------
    write
-------------------------------------------------*/

WRITE8_MEMBER(dragon_fdc_device::write)
{
	switch(offset & 0xEF)
	{
		case 0:
			m_wd2797->cmd_w(space, 0, data);
			break;
		case 1:
			m_wd2797->track_w(space, 0, data);
			break;
		case 2:
			m_wd2797->sector_w(space, 0, data);
			break;
		case 3:
			m_wd2797->data_w(space, 0, data);
			break;
		case 8: case 9: case 10: case 11:
		case 12: case 13: case 14: case 15:
			dskreg_w(data);
			break;
	};
}

//**************************************************************************
//              SDTANDY FDC
//**************************************************************************

ROM_START( sdtandy_fdc )
	ROM_REGION(0x4000,"eprom",ROMREGION_ERASE00)
	ROM_LOAD_OPTIONAL(  "sdtandy.rom",   0x0000,  0x2000, CRC(5d7779b7) SHA1(ca03942118f2deab2f6c8a89b8a4f41f2d0b94f1))
ROM_END

const device_type SDTANDY_FDC = &device_creator<sdtandy_fdc_device>;

//-------------------------------------------------
//  sdtandy_fdc_device - constructor
//-------------------------------------------------

sdtandy_fdc_device::sdtandy_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: dragon_fdc_device(mconfig, SDTANDY_FDC, "SDTANDY FDC", tag, owner, clock, "sdtandy_fdc", __FILE__)
{
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *sdtandy_fdc_device::device_rom_region() const
{
	return ROM_NAME( sdtandy_fdc );
}

//**************************************************************************
//              COCO FDC v1.1
//**************************************************************************

ROM_START( coco_fdc_v11 )
	ROM_REGION(0x8000,"eprom",ROMREGION_ERASE00)
	ROM_LOAD_OPTIONAL(  "disk11.rom",   0x0000, 0x2000, CRC(0b9c5415) SHA1(10bdc5aa2d7d7f205f67b47b19003a4bd89defd1))
	ROM_RELOAD(0x2000, 0x2000)
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x6000, 0x2000)
ROM_END

const device_type COCO_FDC_V11 = &device_creator<coco_fdc_v11_device>;

//-------------------------------------------------
//  coco_fdc_v11_device - constructor
//-------------------------------------------------

coco_fdc_v11_device::coco_fdc_v11_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: coco_fdc_device(mconfig, COCO_FDC_V11, "CoCo FDC v1.1", tag, owner, clock, "coco_fdc_v11", __FILE__)
{
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *coco_fdc_v11_device::device_rom_region() const
{
	return ROM_NAME( coco_fdc_v11 );
}

//**************************************************************************
//              COCO-3 HDB-DOS
//**************************************************************************

ROM_START( coco3_hdb1 )
	ROM_REGION(0x8000,"eprom",ROMREGION_ERASE00)
	ROM_LOAD("hdbdw3bc3.rom",   0x0000, 0x2000, CRC(309a9efd) SHA1(671605d61811953860466f771c1594bbade331f4))
	ROM_RELOAD(0x2000, 0x2000)
	ROM_RELOAD(0x4000, 0x2000)
	ROM_RELOAD(0x6000, 0x2000)
ROM_END

const device_type COCO3_HDB1 = &device_creator<coco3_hdb1_device>;

coco3_hdb1_device::coco3_hdb1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: coco_fdc_device(mconfig, COCO3_HDB1, "CoCo3 HDB-DOS", tag, owner, clock, "coco3_hdb1", __FILE__)
{
}

const rom_entry *coco3_hdb1_device::device_rom_region() const
{
	return ROM_NAME( coco3_hdb1 );
}

//**************************************************************************
//              CP400 FDC
//**************************************************************************

ROM_START( cp400_fdc )
	ROM_REGION(0x4000,"eprom",ROMREGION_ERASE00)
	ROM_LOAD("cp400dsk.rom",  0x0000, 0x2000, CRC(e9ad60a0) SHA1(827697fa5b755f5dc1efb054cdbbeb04e405405b))
ROM_END

const device_type CP400_FDC = &device_creator<cp400_fdc_device>;

//-------------------------------------------------
//  cp400_fdc_device - constructor
//-------------------------------------------------

cp400_fdc_device::cp400_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
		: coco_fdc_device(mconfig, CP400_FDC, "CP400 FDC", tag, owner, clock, "cp400_fdc", __FILE__)
{
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *cp400_fdc_device::device_rom_region() const
{
	return ROM_NAME( cp400_fdc );
}
