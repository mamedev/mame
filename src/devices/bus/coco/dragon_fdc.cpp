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
#include "dragon_fdc.h"

#include "coco_fdc.h"
#include "imagedev/floppy.h"
#include "machine/wd_fdc.h"
#include "formats/dmk_dsk.h"
#include "formats/jvc_dsk.h"
#include "formats/vdk_dsk.h"
#include "formats/sdf_dsk.h"
#include "formats/os9_dsk.h"


/***************************************************************************
    PARAMETERS
***************************************************************************/

#define LOG_FDC (1U << 1)
#define VERBOSE (0)
#include "logmacro.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

namespace
{
	class dragon_fdc_device_base : public coco_family_fdc_device_base
	{
	protected:
		// construction/destruction
		dragon_fdc_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

		// device-level overrides
		virtual u8 cts_read(offs_t offset) override;
		virtual u8 scs_read(offs_t offset) override;
		virtual void scs_write(offs_t offset, u8 data) override;
		virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
		virtual void update_lines() override;

	private:
		// device references
		required_device<wd2797_device>              m_wd2797;
		required_device_array<floppy_connector, 4>  m_floppies;

		// methods
		void dskreg_w(u8 data);
	};

	class premier_fdc_device_base : public coco_family_fdc_device_base
	{
	protected:
		// construction/destruction
		premier_fdc_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

		// device-level overrides
		virtual u8 cts_read(offs_t offset) override;
		virtual u8 scs_read(offs_t offset) override;
		virtual void scs_write(offs_t offset, u8 data) override;
		virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
		virtual void update_lines() override;

	private:
		// device references
		required_device<wd2791_device>              m_wd2791;
		required_device_array<floppy_connector, 4>  m_floppies;

		// methods
		void dskreg_w(u8 data);
	};
}

/***************************************************************************
    LOCAL VARIABLES
***************************************************************************/

static void dragon_fdc_drives(device_slot_interface &device)
{
	device.option_add("qd", FLOPPY_525_QD);
}


void dragon_fdc_device_base::device_add_mconfig(machine_config &config)
{
	WD2797(config, m_wd2797, 4_MHz_XTAL / 4).set_force_ready(true);
	m_wd2797->intrq_wr_callback().set(FUNC(dragon_fdc_device_base::fdc_intrq_w));
	m_wd2797->drq_wr_callback().set(FUNC(dragon_fdc_device_base::fdc_drq_w));

	FLOPPY_CONNECTOR(config, m_floppies[0], dragon_fdc_drives, "qd", dragon_fdc_device_base::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[1], dragon_fdc_drives, "qd", dragon_fdc_device_base::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[2], dragon_fdc_drives, nullptr, dragon_fdc_device_base::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[3], dragon_fdc_drives, nullptr, dragon_fdc_device_base::floppy_formats).enable_sound(true);
}


void premier_fdc_device_base::device_add_mconfig(machine_config &config)
{
	WD2791(config, m_wd2791, 2_MHz_XTAL / 2).set_force_ready(true);

	FLOPPY_CONNECTOR(config, m_floppies[0], dragon_fdc_drives, "qd", dragon_fdc_device_base::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[1], dragon_fdc_drives, "qd", dragon_fdc_device_base::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[2], dragon_fdc_drives, nullptr, dragon_fdc_device_base::floppy_formats).enable_sound(true);
	FLOPPY_CONNECTOR(config, m_floppies[3], dragon_fdc_drives, nullptr, dragon_fdc_device_base::floppy_formats).enable_sound(true);
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  dragon_fdc_device_base - constructor
//-------------------------------------------------
dragon_fdc_device_base::dragon_fdc_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: coco_family_fdc_device_base(mconfig, type, tag, owner, clock)
	, m_wd2797(*this, "wd2797")
	, m_floppies(*this, "wd2797:%u", 0)
{
}


premier_fdc_device_base::premier_fdc_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: coco_family_fdc_device_base(mconfig, type, tag, owner, clock)
	, m_wd2791(*this, "wd2791")
	, m_floppies(*this, "wd2791:%u", 0)
{
}


//-------------------------------------------------
//  update_lines - Dragon specific disk
//  controller lines
//-------------------------------------------------

void dragon_fdc_device_base::update_lines()
{
	// set the NMI line
	set_line_value(line::NMI, intrq() && (dskreg() & 0x20));

	// set the CART line
	set_line_value(line::CART, drq());
}


void premier_fdc_device_base::update_lines()
{
}


//-------------------------------------------------
//  dskreg_w - function to write to
//  Dragon dskreg
//-------------------------------------------------

void dragon_fdc_device_base::dskreg_w(u8 data)
{
	LOGMASKED(LOG_FDC, "fdc_dragon_dskreg_w(): %c%c%c%c%c%c%c%c ($%02x)\n",
		BIT(data, 7) ? 'X' : 'x',
		BIT(data, 6) ? 'X' : 'x',
		BIT(data, 5) ? 'N' : 'n',
		BIT(data, 4) ? 'P' : 'p',
		BIT(data, 3) ? 'S' : 'D',
		BIT(data, 2) ? 'M' : 'm',
		BIT(data, 1) ? '1' : '0',
		BIT(data, 0) ? '1' : '0',
		data);

	// update the motor on each floppy
	for (int i = 0; i < 4; i++)
	{
		floppy_image_device *floppy = m_floppies[i]->get_device();
		if (floppy)
			floppy->mon_w(BIT(data,2) && (i == (data & 0x03)) ? CLEAR_LINE : ASSERT_LINE);
	}

	// manipulate the WD2797
	m_wd2797->set_floppy(m_floppies[data & 0x03]->get_device());
	m_wd2797->dden_w(BIT(data, 3));

	set_dskreg(data);
}


void premier_fdc_device_base::dskreg_w(u8 data)
{
	LOGMASKED(LOG_FDC, "fdc_premier_dskreg_w(): %c%c%c%c%c%c%c%c ($%02x)\n",
		BIT(data, 7) ? 'X' : 'x',
		BIT(data, 6) ? 'X' : 'x',
		BIT(data, 5) ? 'X' : 'x',
		BIT(data, 4) ? 'D' : 'S',
		BIT(data, 3) ? '8' : '5',
		BIT(data, 2) ? '1' : '0',
		BIT(data, 1) ? '1' : '0',
		BIT(data, 0) ? '1' : '0',
		data);
	floppy_image_device *floppy = nullptr;

	// update the motor on each floppy
	for (int i = 0; i < 4; i++)
	{
		floppy = m_floppies[i]->get_device();
		if (floppy)
			floppy->mon_w((i == (data & 0x03)) ? CLEAR_LINE : ASSERT_LINE );
	}
	floppy = m_floppies[data & 0x03]->get_device();

	// manipulate the WD2791
	m_wd2791->set_floppy(floppy);

	if (floppy)
		floppy->ss_w(BIT(data, 2));

	m_wd2791->dden_w(!BIT(data, 4));

	set_dskreg(data);
}


//-------------------------------------------------
//  cts_read
//-------------------------------------------------

u8 dragon_fdc_device_base::cts_read(offs_t offset)
{
	return memregion("eprom")->base()[offset];
}


u8 premier_fdc_device_base::cts_read(offs_t offset)
{
	return memregion("eprom")->base()[offset];
}


//-------------------------------------------------
//  scs_read
//-------------------------------------------------

u8 dragon_fdc_device_base::scs_read(offs_t offset)
{
	u8 result = 0;
	switch (offset & 0xef)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		result = m_wd2797->read(offset & 0xef);
		break;
	}
	return result;
}


u8 premier_fdc_device_base::scs_read(offs_t offset)
{
	u8 result = 0;
	switch (offset)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		result = m_wd2791->read(offset);
		break;
	}
	return result;
}


//-------------------------------------------------
//  scs_write
//-------------------------------------------------

void dragon_fdc_device_base::scs_write(offs_t offset, u8 data)
{
	switch (offset & 0xef)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		m_wd2797->write(offset & 0xef, data);
		break;
	case 8: case 9: case 10: case 11:
	case 12: case 13: case 14: case 15:
		dskreg_w(data);
		break;
	};
}


void premier_fdc_device_base::scs_write(offs_t offset, u8 data)
{
	switch (offset)
	{
	case 0:
	case 1:
	case 2:
	case 3:
		m_wd2791->write(offset, data);
		break;
	case 4:
		dskreg_w(data);
		break;
	};
}


//**************************************************************************
//              DRAGON FDC
//**************************************************************************

ROM_START(dragon_fdc)
	ROM_REGION(0x4000, "eprom", ROMREGION_ERASE00)
	ROM_LOAD("ddos10.rom", 0x0000, 0x2000, CRC(b44536f6) SHA1(a8918c71d319237c1e3155bb38620acb114a80bc))
ROM_END

namespace
{
	class dragon_fdc_device : public dragon_fdc_device_base
	{
	public:
		// construction/destruction
		dragon_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
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

DEFINE_DEVICE_TYPE_PRIVATE(DRAGON_FDC, device_cococart_interface, dragon_fdc_device, "dragon_fdc", "Dragon FDC")


//**************************************************************************
//              PREMIER FDC
//**************************************************************************

ROM_START(premier_fdc)
	ROM_REGION(0x4000, "eprom", ROMREGION_ERASE00)
	ROM_LOAD("deltados.rom", 0x0000, 0x2000, CRC(149eb4dd) SHA1(eb686ce6afe63e4d4011b333a882ca812c69307f))
ROM_END

namespace
{
	class premier_fdc_device : public premier_fdc_device_base
	{
	public:
		// construction/destruction
		premier_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
			: premier_fdc_device_base(mconfig, PREMIER_FDC, tag, owner, clock)
		{
		}

	protected:
		// optional information overrides
		virtual const tiny_rom_entry *device_rom_region() const override
		{
			return ROM_NAME(premier_fdc);
		}
	};
};

DEFINE_DEVICE_TYPE_PRIVATE(PREMIER_FDC, device_cococart_interface, premier_fdc_device, "premier_fdc", "Premier FDC")


//**************************************************************************
//              SDTANDY FDC
//**************************************************************************

ROM_START(sdtandy_fdc)
	ROM_REGION(0x4000, "eprom", ROMREGION_ERASE00)
	ROM_LOAD("sdtandy.rom", 0x0000, 0x2000, CRC(5d7779b7) SHA1(ca03942118f2deab2f6c8a89b8a4f41f2d0b94f1))
ROM_END

namespace
{
	class sdtandy_fdc_device : public dragon_fdc_device_base
	{
	public:
		// construction/destruction
		sdtandy_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
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

DEFINE_DEVICE_TYPE_PRIVATE(SDTANDY_FDC, device_cococart_interface, sdtandy_fdc_device, "sdtandy_fdc", "SDTANDY FDC")
