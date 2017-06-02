// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/*********************************************************************

    dragon_fdc.cpp

    Dragon Floppy Disk Controller

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
    Dragon ($FF48)

    Bit
    7 not used
    6 not used
    5 NMI enable flag
    4 write precompensation
    3 single density enable
    2 drive motor activation
    1 drive select high bit
    0 drive select low bit

    ---------------------------------------------------------------------------

    2007-02-22, P.Harvey-Smith

    Began implementing the Dragon Delta Dos controller, this was actually the first
    Dragon disk controller to market, beating Dragon Data's by a couple of months,
    it is based around the WD2791 FDC, which is compatible with the WD1793/WD2797 used
    by the standard CoCo and Dragon disk controllers except that it used an inverted
    data bus, which is the reason the read/write handlers invert the data. This
    controller like, the DragonDos WD2797 is mapped at $FF40-$FF43, in the normal
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
#include "cococart.h"
#include "coco_fdc.h"
#include "imagedev/flopdrv.h"
#include "machine/wd_fdc.h"
#include "formats/dmk_dsk.h"
#include "formats/jvc_dsk.h"
#include "formats/vdk_dsk.h"
#include "formats/sdf_dsk.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

#define LOG_FDC                 0
#define WD2797_TAG              "wd2797"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

namespace
{
	class dragon_fdc_device_base : public coco_family_fdc_device_base
	{
	protected:
		// construction/destruction
		dragon_fdc_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

		// device-level overrides
		virtual DECLARE_READ8_MEMBER(scs_read) override;
		virtual DECLARE_WRITE8_MEMBER(scs_write) override;
		virtual void device_add_mconfig(machine_config &config) override;
		virtual void update_lines() override;

	private:
		// device references
		required_device<wd2797_device>              m_wd2797;
		required_device_array<floppy_connector, 4>  m_floppies;

		// methods
		void dskreg_w(uint8_t data);
	};
}

/***************************************************************************
    LOCAL VARIABLES
***************************************************************************/

static SLOT_INTERFACE_START(dragon_fdc_device_base)
	SLOT_INTERFACE("qd", FLOPPY_525_QD)
SLOT_INTERFACE_END


MACHINE_CONFIG_MEMBER(dragon_fdc_device_base::device_add_mconfig)
	MCFG_WD2797_ADD(WD2797_TAG, XTAL_1MHz)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(dragon_fdc_device_base, fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(dragon_fdc_device_base, fdc_drq_w))

	MCFG_FLOPPY_DRIVE_ADD(WD2797_TAG ":0", dragon_fdc_device_base, "qd", dragon_fdc_device_base::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD(WD2797_TAG ":1", dragon_fdc_device_base, "qd", dragon_fdc_device_base::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD(WD2797_TAG ":2", dragon_fdc_device_base, "", dragon_fdc_device_base::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
	MCFG_FLOPPY_DRIVE_ADD(WD2797_TAG ":3", dragon_fdc_device_base, "", dragon_fdc_device_base::floppy_formats)
	MCFG_FLOPPY_DRIVE_SOUND(true)
MACHINE_CONFIG_END


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  dragon_fdc_device_base - constructor
//-------------------------------------------------
dragon_fdc_device_base::dragon_fdc_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: coco_family_fdc_device_base(mconfig, type, tag, owner, clock)
	, m_wd2797(*this, WD2797_TAG)
	, m_floppies(*this, WD2797_TAG ":%u", 0)
{
}


//-------------------------------------------------
//  update_lines - Dragon specific disk
//  controller lines
//-------------------------------------------------

void dragon_fdc_device_base::update_lines()
{
	// set the NMI line
	cart_set_line(cococart_slot_device::line::NMI, intrq() && (dskreg() & 0x20));

	// set the CART line
	cart_set_line(cococart_slot_device::line::CART, drq());
}


//-------------------------------------------------
//  dskreg_w - function to write to
//  Dragon dskreg
//-------------------------------------------------

void dragon_fdc_device_base::dskreg_w(uint8_t data)
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

	// update the motor on each floppy
	for (int i = 0; i < 4; i++)
	{
		floppy_image_device *floppy = m_floppies[i]->get_device();
		if (floppy)
			floppy->mon_w((data & 0x04) && (i == (data & 0x03)) ? CLEAR_LINE : ASSERT_LINE);
	}

	// manipulate the WD2797
	m_wd2797->set_floppy(m_floppies[data & 0x03]->get_device());
	m_wd2797->dden_w(BIT(data, 3));

	set_dskreg(data);
}


//-------------------------------------------------
//  scs_read
//-------------------------------------------------

READ8_MEMBER(dragon_fdc_device_base::scs_read)
{
	uint8_t result = 0;
	switch (offset & 0xEF)
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



//-------------------------------------------------
//  scs_write
//-------------------------------------------------

WRITE8_MEMBER(dragon_fdc_device_base::scs_write)
{
	switch (offset & 0xEF)
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
//              DRAGON FDC
//**************************************************************************

ROM_START(dragon_fdc)
	ROM_REGION(0x4000, "eprom", ROMREGION_ERASE00)
	ROM_LOAD_OPTIONAL("ddos10.rom", 0x0000, 0x2000, CRC(b44536f6) SHA1(a8918c71d319237c1e3155bb38620acb114a80bc))
ROM_END

namespace
{
	class dragon_fdc_device : public dragon_fdc_device_base
	{
	public:
		// construction/destruction
		dragon_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
			: dragon_fdc_device_base(mconfig, DRAGON_FDC, tag, owner, clock)
		{
		}

	protected:
		// optional information overrides
		virtual const tiny_rom_entry *device_rom_region() const override
		{
			return ROM_NAME(dragon_fdc);
		}
	};
}

DEFINE_DEVICE_TYPE(DRAGON_FDC, dragon_fdc_device, "dragon_fdc", "Dragon FDC")


//**************************************************************************
//              SDTANDY FDC
//**************************************************************************

ROM_START(sdtandy_fdc)
	ROM_REGION(0x4000, "eprom", ROMREGION_ERASE00)
	ROM_LOAD_OPTIONAL("sdtandy.rom", 0x0000, 0x2000, CRC(5d7779b7) SHA1(ca03942118f2deab2f6c8a89b8a4f41f2d0b94f1))
ROM_END

namespace
{
	class sdtandy_fdc_device : public dragon_fdc_device_base
	{
	public:
		// construction/destruction
		sdtandy_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
			: dragon_fdc_device_base(mconfig, SDTANDY_FDC, tag, owner, clock)
		{
		}

	protected:
		// optional information overrides
		virtual const tiny_rom_entry *device_rom_region() const override
		{
			return ROM_NAME(sdtandy_fdc);
		}
	};
}

DEFINE_DEVICE_TYPE(SDTANDY_FDC, sdtandy_fdc_device, "sdtandy_fdc", "SDTANDY FDC")
