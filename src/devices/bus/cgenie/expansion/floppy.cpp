// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    EACA Colour Genie Floppy Disc Controller

    TODO:
    - How does it turn off the motor?
    - What's the source of the timer and the exact timings?

***************************************************************************/

#include "emu.h"
#include "floppy.h"
#include "formats/cgenie_dsk.h"
#include "bus/generic/carts.h"
#include "softlist.h"

//**************************************************************************
//  CONSTANTS/MACROS
//**************************************************************************

#define VERBOSE 0


//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(CGENIE_FDC, cgenie_fdc_device, "cgenie_fdc", "Colour Genie FDC")

void cgenie_fdc_device::mmio(address_map &map)
{
	map(0xe0, 0xe3).mirror(0x10).rw(FUNC(cgenie_fdc_device::irq_r), FUNC(cgenie_fdc_device::select_w));
	map(0xec, 0xec).mirror(0x10).r("wd2793", FUNC(wd2793_device::status_r)).w(FUNC(cgenie_fdc_device::command_w));
	map(0xed, 0xed).mirror(0x10).rw("wd2793", FUNC(wd2793_device::track_r), FUNC(wd2793_device::track_w));
	map(0xee, 0xee).mirror(0x10).rw("wd2793", FUNC(wd2793_device::sector_r), FUNC(wd2793_device::sector_w));
	map(0xef, 0xef).mirror(0x10).rw("wd2793", FUNC(wd2793_device::data_r), FUNC(wd2793_device::data_w));
}

FLOPPY_FORMATS_MEMBER( cgenie_fdc_device::floppy_formats )
	FLOPPY_CGENIE_FORMAT
FLOPPY_FORMATS_END

static void cgenie_floppies(device_slot_interface &device)
{
	device.option_add("sssd", FLOPPY_525_SSSD);
	device.option_add("sd",   FLOPPY_525_SD);
	device.option_add("ssdd", FLOPPY_525_SSDD);
	device.option_add("dd",   FLOPPY_525_DD);
	device.option_add("ssqd", FLOPPY_525_SSQD);
	device.option_add("qd",   FLOPPY_525_QD);
}

//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

ROM_START( cgenie_fdc )
	ROM_REGION(0x2000, "software", 0)
	ROM_DEFAULT_BIOS("unk1")

	// from cgemu
	ROM_SYSTEM_BIOS(0, "unk1", "Unknown 1")
	ROMX_LOAD("cgdos.rom", 0x0000, 0x2000, CRC(2a96cf74) SHA1(6dcac110f87897e1ee7521aefbb3d77a14815893), ROM_BIOS(0))

	// can't read label
	ROM_SYSTEM_BIOS(1, "unk2", "Unknown 2")
	ROMX_LOAD("cgdos_a.c", 0x0000, 0x1000, CRC(6164e9d1) SHA1(c42ab4e73173893918abc871d01b63a3030cf6cc), ROM_BIOS(1))
	ROMX_LOAD("cgdos_a.d", 0x1000, 0x1000, CRC(b09eb5d1) SHA1(80db78f665a488ad8cbffea696274b53fb90e492), ROM_BIOS(1))

	// typed in from source code
	ROM_SYSTEM_BIOS(2, "v2", "Version 2")
	ROMX_LOAD("cgdos-v2.rom", 0x0000, 0x2000, BAD_DUMP CRC(9dace9c1) SHA1(513fef9fea81bee8f5edcf831095e90941e7cd69), ROM_BIOS(2))
ROM_END

const tiny_rom_entry *cgenie_fdc_device::device_rom_region() const
{
	return ROM_NAME( cgenie_fdc );
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

MACHINE_CONFIG_START(cgenie_fdc_device::device_add_mconfig)
	TIMER(config, "timer").configure_periodic(FUNC(cgenie_fdc_device::timer_callback), attotime::from_msec(25));

	WD2793(config, m_fdc, 4_MHz_XTAL / 4);
	m_fdc->intrq_wr_callback().set(FUNC(cgenie_fdc_device::intrq_w));

	FLOPPY_CONNECTOR(config, "wd2793:0", cgenie_floppies, "ssdd", cgenie_fdc_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "wd2793:1", cgenie_floppies, "ssdd", cgenie_fdc_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "wd2793:2", cgenie_floppies, nullptr, cgenie_fdc_device::floppy_formats);
	FLOPPY_CONNECTOR(config, "wd2793:3", cgenie_floppies, nullptr, cgenie_fdc_device::floppy_formats);

//  SOFTWARE_LIST(config, "floppy_list").set_original("cgenie_flop");

	MCFG_GENERIC_SOCKET_ADD("socket", generic_plain_slot, "cgenie_flop_rom")
	MCFG_GENERIC_EXTENSIONS("bin,rom")
	MCFG_GENERIC_LOAD(cgenie_fdc_device, socket_load)

	SOFTWARE_LIST(config, "rom_list").set_original("cgenie_flop_rom");
MACHINE_CONFIG_END


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  cgenie_fdc_device - constructor
//-------------------------------------------------

cgenie_fdc_device::cgenie_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, CGENIE_FDC, tag, owner, clock),
	device_cg_exp_interface(mconfig, *this),
	m_fdc(*this, "wd2793"),
	m_floppy0(*this, "wd2793:0"),
	m_floppy1(*this, "wd2793:1"),
	m_floppy2(*this, "wd2793:2"),
	m_floppy3(*this, "wd2793:3"),
	m_socket(*this, "socket"),
	m_floppy(nullptr),
	m_irq_status(0)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void cgenie_fdc_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void cgenie_fdc_device::device_reset()
{
	// dos rom
	m_slot->m_program->install_rom(0xc000, 0xdfff, memregion("software")->base());

	// memory mapped i/o
	m_slot->m_program->install_device(0xff00, 0xffff, *this, &cgenie_fdc_device::mmio);

	// map extra socket
	if (m_socket->exists())
	{
		m_slot->m_program->install_read_handler(0xe000, 0xefff, read8sm_delegate(FUNC(generic_slot_device::read_rom), (generic_slot_device *) m_socket));
	}
}


//**************************************************************************
//  IMPLEMENTATION
//**************************************************************************

READ8_MEMBER( cgenie_fdc_device::irq_r )
{
	uint8_t data = m_irq_status;

	m_irq_status &= ~IRQ_TIMER;
	m_slot->int_w(m_irq_status ? ASSERT_LINE : CLEAR_LINE);

	return data;
}

TIMER_DEVICE_CALLBACK_MEMBER( cgenie_fdc_device::timer_callback )
{
	m_irq_status |= IRQ_TIMER;
	m_slot->int_w(ASSERT_LINE);
}

DEVICE_IMAGE_LOAD_MEMBER( cgenie_fdc_device, socket_load )
{
	uint32_t size = m_socket->common_get_size("rom");

	if (size > 0x1000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported ROM size");
		return image_init_result::FAIL;
	}

	m_socket->rom_alloc(0x1000, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_socket->common_load_rom(m_socket->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}

WRITE_LINE_MEMBER( cgenie_fdc_device::intrq_w )
{
	if (VERBOSE)
		logerror("cgenie_fdc_device::intrq_w: %d\n", state);

	if (state)
		m_irq_status |= IRQ_WDC;
	else
		m_irq_status &= ~IRQ_WDC;

	m_slot->int_w(m_irq_status ? ASSERT_LINE : CLEAR_LINE);
}

WRITE8_MEMBER( cgenie_fdc_device::select_w )
{
	if (VERBOSE)
		logerror("cgenie_fdc_device::motor_w: 0x%02x\n", data);

	m_floppy = nullptr;

	if (BIT(data, 0)) m_floppy = m_floppy0->get_device();
	if (BIT(data, 1)) m_floppy = m_floppy1->get_device();
	if (BIT(data, 2)) m_floppy = m_floppy2->get_device();
	if (BIT(data, 3)) m_floppy = m_floppy3->get_device();

	m_fdc->set_floppy(m_floppy);

	if (m_floppy)
	{
		m_floppy->ss_w(BIT(data, 4));
		m_floppy->mon_w(0);
	}
}

WRITE8_MEMBER( cgenie_fdc_device::command_w )
{
	// density select is encoded into this pseudo-command
	if ((data & 0xfe) == 0xfe)
		m_fdc->dden_w(!BIT(data, 0));

	// forward to the controller
	m_fdc->cmd_w(data);
}
