// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/**********************************************************************

    ISA 8 bit Floppy Disk Controller for Matsushita MyBrain 3000,
    Panasonic JB-3000 and Ericsson Step/One

    FDC4710 - 5.25" FDD4730
     - 1-2 x SSDD 40 tracks, 8 sectors/track, 512 bytes/sector 160KB

    FDC4711 - 5.25" FDD4731/FDD4732 (FDD4732 has external power needed for drive 3-4)
     - 1-4 x DSDD 80 tracks, 8 sectors/track, 512 bytes/sector 720KB

    FDC4712 - 8" FDD4733
     - 1-2 x DSDD 154 tracks, 8 sector/track, 1024 bytes/sector 1232KB

    Step/One service manuals: http://nivelleringslikaren.eu/stepone/

   TODO:
   - Verify FDC4710 as soon as we find a 160Kb floppy image
   - Add FDC4712 8" as soon as we get a visual or schematics on it
   - Reduce code duplication by introducing a base class, once all are emulated
   - Put these into global ISA8 collection

********************************************************************************/

#include "emu.h"
#include "myb3k_fdc.h"
#include "imagedev/flopdrv.h"
#include "formats/pc_dsk.h"
#include "formats/imd_dsk.h"

//#define LOG_GENERAL (1U << 0) //defined in logmacro.h already
#define LOG_READ    (1U << 1)
#define LOG_CMD     (1U << 2)

//#define VERBOSE (LOG_GENERAL | LOG_CMD)
//#define LOG_OUTPUT_STREAM std::cout

#include "logmacro.h"

#define LOGR(...)   LOGMASKED(LOG_READ, __VA_ARGS__)
#define LOGCMD(...) LOGMASKED(LOG_CMD,  __VA_ARGS__)

#ifdef _MSC_VER
#define FUNCNAME __func__
#else
#define FUNCNAME __PRETTY_FUNCTION__
#endif

DEFINE_DEVICE_TYPE(ISA8_MYB3K_FDC4710, isa8_myb3k_fdc4710_device, "isa8_myb3k_fdc4710", "FDC4710 SSDD Floppy Disk Controller")
DEFINE_DEVICE_TYPE(ISA8_MYB3K_FDC4711, isa8_myb3k_fdc4711_device, "isa8_myb3k_fdc4711", "FDC4711 DSDD Floppy Disk Controller")

void isa8_myb3k_fdc4710_device::map(address_map &map)
{
//  AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("fdc", mb8876_device, read, write) AM_MIRROR(0x500)
	map(0x00, 0x03).r(this, FUNC(isa8_myb3k_fdc4710_device::myb3k_inv_fdc_data_r)).w(this, FUNC(isa8_myb3k_fdc4710_device::myb3k_inv_fdc_data_w)).mirror(0x500);
	map(0x04, 0x04).w(this, FUNC(isa8_myb3k_fdc4710_device::myb3k_fdc_command)).mirror(0x500);
	map(0x05, 0x05).r(this, FUNC(isa8_myb3k_fdc4710_device::myb3k_fdc_status)).mirror(0x500);
}

void isa8_myb3k_fdc4711_device::map(address_map &map)
{
//  AM_RANGE(0x00, 0x03) AM_DEVREADWRITE("fdc", fd1791_device, read, write) AM_MIRROR(0x500)
	map(0x00, 0x03).r(this, FUNC(isa8_myb3k_fdc4711_device::myb3k_inv_fdc_data_r)).w(this, FUNC(isa8_myb3k_fdc4711_device::myb3k_inv_fdc_data_w)).mirror(0x500);
	map(0x04, 0x04).w(this, FUNC(isa8_myb3k_fdc4711_device::myb3k_fdc_command)).mirror(0x500);
	map(0x05, 0x05).r(this, FUNC(isa8_myb3k_fdc4711_device::myb3k_fdc_status)).mirror(0x500);
}

FLOPPY_FORMATS_MEMBER( isa8_myb3k_fdc4710_device::myb3k_floppy_formats )
	FLOPPY_IMD_FORMAT
FLOPPY_FORMATS_END

FLOPPY_FORMATS_MEMBER( isa8_myb3k_fdc4711_device::myb3k_floppy_formats )
	FLOPPY_PC_FORMAT,
	FLOPPY_IMD_FORMAT
FLOPPY_FORMATS_END

static void myb3k_sd_floppies(device_slot_interface &device)
{
	device.option_add("525sd", FLOPPY_525_SD);
}

static void myb3k_qd_floppies(device_slot_interface &device)
{
	device.option_add("525qd", FLOPPY_525_QD);
}

#if 0
static void myb3k_8inch_floppies(device_slot_interface &device)
{
	device.option_add("8dsdd", FLOPPY_8_DSDD);
}
#endif

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------
/*  */
MACHINE_CONFIG_START(isa8_myb3k_fdc4710_device::device_add_mconfig)
	MCFG_DEVICE_ADD("fdc", MB8876, XTAL(15'974'400) / 8) /* From StepOne schematics */
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(*this, isa8_myb3k_fdc4710_device, irq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(*this, isa8_myb3k_fdc4710_device, drq_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", myb3k_sd_floppies, "525sd", isa8_myb3k_fdc4710_device::myb3k_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", myb3k_sd_floppies, "525sd", isa8_myb3k_fdc4710_device::myb3k_floppy_formats)
MACHINE_CONFIG_END

/* Main difference from fdc4710 is that a Hitachi HA16632AP has replaced the descrete VFO enabling 720Kb disks */
MACHINE_CONFIG_START(isa8_myb3k_fdc4711_device::device_add_mconfig)
	MCFG_DEVICE_ADD("fdc", FD1791, XTAL(15'974'400) / 16)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(*this, isa8_myb3k_fdc4711_device, irq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(*this, isa8_myb3k_fdc4711_device, drq_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", myb3k_qd_floppies, "525qd", isa8_myb3k_fdc4711_device::myb3k_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", myb3k_qd_floppies, "525qd", isa8_myb3k_fdc4711_device::myb3k_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:2", myb3k_qd_floppies, "525qd", isa8_myb3k_fdc4711_device::myb3k_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:3", myb3k_qd_floppies, "525qd", isa8_myb3k_fdc4711_device::myb3k_floppy_formats)
MACHINE_CONFIG_END

#if 0
MACHINE_CONFIG_START(isa8_myb3k_fdc4712_device::device_add_mconfig)
	MCFG_DEVICE_ADD("fdc", FD1791, XTAL(15'974'400) / 8)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(*this, isa8_myb3k_fdc4712_device, irq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(*this, isa8_myb3k_fdc4712_device, drq_w))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", myb3k_8inch_floppies, "8dsdd", isa8_myb3k_fdc4712_device::myb3k_floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", myb3k_8inch_floppies, "8dsdd", isa8_myb3k_fdc4712_device::myb3k_floppy_formats)
MACHINE_CONFIG_END
#endif

//**************************************************************************
//  LIVE DEVICES
//**************************************************************************
isa8_myb3k_fdc4710_device::isa8_myb3k_fdc4710_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_fdc(*this, "fdc")
	, m_fdd0(*this, "fdc:0")
	, m_fdd1(*this, "fdc:1")
{
}

isa8_myb3k_fdc4710_device::isa8_myb3k_fdc4710_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : isa8_myb3k_fdc4710_device(mconfig, ISA8_MYB3K_FDC4710, tag, owner, clock)
{
}

isa8_myb3k_fdc4711_device::isa8_myb3k_fdc4711_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock)
	, device_isa8_card_interface(mconfig, *this)
	, m_fdc(*this, "fdc")
	, m_fdd0(*this, "fdc:0")
	, m_fdd1(*this, "fdc:1")
	, m_fdd2(*this, "fdc:2")
	, m_fdd3(*this, "fdc:3")
{
}

isa8_myb3k_fdc4711_device::isa8_myb3k_fdc4711_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : isa8_myb3k_fdc4711_device(mconfig, ISA8_MYB3K_FDC4711, tag, owner, clock)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void isa8_myb3k_fdc4710_device::device_start()
{
	LOG("%s\n", FUNCNAME);

	set_isa_device();
	m_isa->set_dma_channel(2, this, true);
	m_isa->install_device(0x020, 0x027, *this, &isa8_myb3k_fdc4710_device::map);
}

void isa8_myb3k_fdc4711_device::device_start()
{
	LOG("%s\n", FUNCNAME);

	set_isa_device();
	m_isa->install_device(0x020, 0x027, *this, &isa8_myb3k_fdc4711_device::map);
	m_isa->set_dma_channel(2, this, true);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------
void isa8_myb3k_fdc4710_device::device_reset()
{
	LOG("%s\n", FUNCNAME);
}

void isa8_myb3k_fdc4711_device::device_reset()
{
	LOG("%s\n", FUNCNAME);
}

//-------------------------------------------------
//  irq_w - signal interrupt request to ISA bus
//-------------------------------------------------
WRITE_LINE_MEMBER( isa8_myb3k_fdc4710_device::irq_w )
{
	LOG("%s: %d\n", FUNCNAME, state);
	m_isa->irq6_w(state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER( isa8_myb3k_fdc4711_device::irq_w )
{
	LOG("%s: %d\n", FUNCNAME, state);
	m_isa->irq6_w(state ? ASSERT_LINE : CLEAR_LINE);
}

//-------------------------------------------------
//  drq_w - signal dma request to ISA bus
//-------------------------------------------------
WRITE_LINE_MEMBER( isa8_myb3k_fdc4710_device::drq_w )
{
	LOG("%s: %d\n", FUNCNAME, state);
	m_isa->drq2_w(state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER( isa8_myb3k_fdc4711_device::drq_w )
{
	LOG("%s: %d\n", FUNCNAME, state);
	m_isa->drq2_w(state ? ASSERT_LINE : CLEAR_LINE);
}

//-------------------------------------------------
//  dack_r - return FDC data trough DMA
//-------------------------------------------------
uint8_t isa8_myb3k_fdc4710_device::dack_r(int line)
{
	return ~(m_fdc->data_r());
}

uint8_t isa8_myb3k_fdc4711_device::dack_r(int line)
{
	return ~(m_fdc->data_r());
}

//-------------------------------------------------
//  dack_w - write DMA data to FDC
//-------------------------------------------------
void isa8_myb3k_fdc4710_device::dack_w(int line, uint8_t data)
{
	return m_fdc->data_w(data);
}

void isa8_myb3k_fdc4711_device::dack_w(int line, uint8_t data)
{
	return m_fdc->data_w(data);
}

#if 0 // eop/tc is used to for logic around multi sector transfers
void isa8_myb3k_fdc4710_device::eop_w(int state)
{
	m_fdc->tc_w(state == ASSERT_LINE);
}

void isa8_myb3k_fdc4711_device::eop_w(int state)
{
	m_fdc->tc_w(state == ASSERT_LINE);
}
#endif

//--------------------------------------------------------
//  myb3k_inv_fdc_data_r - a LS240 inverts databus for FDC
//--------------------------------------------------------
READ8_MEMBER( isa8_myb3k_fdc4710_device::myb3k_inv_fdc_data_r )
{
	uint8_t tmp = m_fdc->read(space, offset);
	LOGR("%s: %02x -> %02x\n", FUNCNAME, tmp, (~tmp) & 0xff);
	return ~tmp;
}

READ8_MEMBER( isa8_myb3k_fdc4711_device::myb3k_inv_fdc_data_r )
{
	uint8_t tmp = m_fdc->read(space, offset);
	LOGR("%s: %02x -> %02x\n", FUNCNAME, tmp, (~tmp) & 0xff);
	return ~tmp;
}

//--------------------------------------------------------
//  myb3k_inv_fdc_data_w - a LS240 inverts databus for FDC
//--------------------------------------------------------
WRITE8_MEMBER( isa8_myb3k_fdc4710_device::myb3k_inv_fdc_data_w )
{
	LOG("%s: %02x -> %02x\n", FUNCNAME, data, (~data) & 0xff);
	m_fdc->write(space, offset, (~data) & 0xff);
}

WRITE8_MEMBER( isa8_myb3k_fdc4711_device::myb3k_inv_fdc_data_w )
{
	LOG("%s: %02x -> %02x\n", FUNCNAME, data, (~data) & 0xff);
	m_fdc->write(space, offset, (~data) & 0xff);
}

//-------------------------------------------------
//  myb3k_fdc_command - descrete fdc card features
//-------------------------------------------------
WRITE8_MEMBER( isa8_myb3k_fdc4710_device::myb3k_fdc_command )
{
	data = ~data;
	LOG("%s: %02x\n", FUNCNAME, data);
	LOGCMD(" - Drive %d\n", ~data & FDC_DRIVE_SEL);
	LOGCMD(" - Side  %d\n", (data & FDC_SIDE_SEL) ? 0 : 1);
	LOGCMD(" - Motor %s\n", (data & FDC_MOTOR_ON) ? "OFF" : "ON");
	LOGCMD(" - Density %s\n", (data & FDC_DDEN) ? "FM" : "MFM");

	floppy_image_device *floppy = nullptr;

	switch(~data & FDC_DRIVE_SEL) // Y0-Y3 on a '139 maps to drive 4 to 1 respectively
	{
	case 0:floppy = m_fdd0->get_device(); break;
	case 1:floppy = m_fdd1->get_device(); break;
	}

	if (floppy)
	{
		LOGCMD(" - Floppy found\n");
		m_fdc->set_floppy(floppy);
		floppy->ss_w((data & FDC_SIDE_SEL) ? 0 : 1);
		floppy->mon_w((data & FDC_MOTOR_ON) ? 1 : 0); // Active low and inverter on incomming data line
	}
	else
	{
		LOGCMD(" - Floppy not found\n");
	}
	m_fdc->dden_w((data & FDC_DDEN) ? 1 : 0); // active low == MFM
}

WRITE8_MEMBER( isa8_myb3k_fdc4711_device::myb3k_fdc_command )
{
	data = ~data;
	LOG("%s: %02x\n", FUNCNAME, data);
	LOGCMD(" - Drive %d\n", ~data & FDC_DRIVE_SEL);
	LOGCMD(" - Side  %d\n", (data & FDC_SIDE_SEL) ? 0 : 1);
	LOGCMD(" - Motor %s\n", (data & FDC_MOTOR_ON) ? "OFF" : "ON");
	LOGCMD(" - Density %s\n", (data & FDC_DDEN) ? "FM" : "MFM");

	floppy_image_device *floppy = nullptr;

	switch(~data & FDC_DRIVE_SEL) // Y0-Y3 on a '139 maps to drive 4 to 1 respectively
	{
	case 0:floppy = m_fdd0->get_device(); break;
	case 1:floppy = m_fdd1->get_device(); break;
	case 2:floppy = m_fdd2->get_device(); break;
	case 3:floppy = m_fdd3->get_device(); break;
	}

	if (floppy)
	{
		LOGCMD(" - Floppy found\n");
		m_fdc->set_floppy(floppy);
		floppy->ss_w((data & FDC_SIDE_SEL) ? 0 : 1);
		floppy->mon_w((data & FDC_MOTOR_ON) ? 1 : 0); // Active low and inverter on incomming data line
	}
	else
	{
		LOGCMD(" - Floppy not found\n");
	}
	m_fdc->dden_w((data & FDC_DDEN) ? 1 : 0); // active low == MFM
}

//-------------------------------------------------
//  myb3k_fdc_status - descrete fdc card status
//-------------------------------------------------
#define FDC_MSM_END_IR 0x01
READ8_MEMBER( isa8_myb3k_fdc4710_device::myb3k_fdc_status )
{
	LOG("%s\n", FUNCNAME);
	// TODO: return the multi sector mode interrupt status
	return 0x00;
}

READ8_MEMBER( isa8_myb3k_fdc4711_device::myb3k_fdc_status )
{
	LOG("%s\n", FUNCNAME);
	// TODO: return the multi sector mode interrupt status
	return 0x00;
}
