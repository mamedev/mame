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
     - 1-4 x DSDD 154 tracks, 8 sector/track, 1024 bytes/sector 1232KB

    Step/One service manuals: http://nivelleringslikaren.eu/stepone/

   TODO:
   - Verify FDC4710 as soon as we find a 160Kb floppy image
   - Put these into global ISA8 collection

********************************************************************************/

#include "emu.h"
#include "myb3k_fdc.h"
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
DEFINE_DEVICE_TYPE(ISA8_MYB3K_FDC4712, isa8_myb3k_fdc4712_device, "isa8_myb3k_fdc4712", "FDC4712 DSDD Floppy Disk Controller")

void isa8_myb3k_fdc471x_device_base::map(address_map &map)
{
	map(0x00, 0x03).r(FUNC(isa8_myb3k_fdc471x_device_base::myb3k_inv_fdc_data_r)).w(FUNC(isa8_myb3k_fdc471x_device_base::myb3k_inv_fdc_data_w));
	map(0x04, 0x04).w(FUNC(isa8_myb3k_fdc471x_device_base::myb3k_fdc_command));
	map(0x05, 0x05).r(FUNC(isa8_myb3k_fdc471x_device_base::myb3k_fdc_status));
}

FLOPPY_FORMATS_MEMBER( isa8_myb3k_fdc4710_device::myb3k_floppy_formats )
	FLOPPY_IMD_FORMAT
FLOPPY_FORMATS_END

FLOPPY_FORMATS_MEMBER( isa8_myb3k_fdc4711_device::myb3k_floppy_formats )
	FLOPPY_PC_FORMAT,
	FLOPPY_IMD_FORMAT
FLOPPY_FORMATS_END

FLOPPY_FORMATS_MEMBER( isa8_myb3k_fdc4712_device::myb3k_floppy_formats )
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

static void myb3k_8inch_floppies(device_slot_interface &device)
{
	device.option_add("8dsdd", FLOPPY_8_DSDD);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void isa8_myb3k_fdc471x_device_base::device_add_mconfig(machine_config &config)
{
	m_fdc->intrq_wr_callback().set(FUNC(isa8_myb3k_fdc471x_device_base::irq_w));
	m_fdc->drq_wr_callback().set(FUNC(isa8_myb3k_fdc471x_device_base::drq_w));
}

void isa8_myb3k_fdc4710_device::device_add_mconfig(machine_config &config)
{
	MB8876(config, m_fdc, XTAL(15'974'400) / 8); /* From StepOne schematics */
	FLOPPY_CONNECTOR(config, m_floppy_connectors[0], myb3k_sd_floppies, "525sd", isa8_myb3k_fdc4710_device::myb3k_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy_connectors[1], myb3k_sd_floppies, "525sd", isa8_myb3k_fdc4710_device::myb3k_floppy_formats);

	isa8_myb3k_fdc471x_device_base::device_add_mconfig(config);
}

/* Main difference from fdc4710 is that a Hitachi HA16632AP has replaced the discrete VFO enabling 720Kb disks */
void isa8_myb3k_fdc4711_device::device_add_mconfig(machine_config &config)
{
	FD1791(config, m_fdc, XTAL(15'974'400) / 16);
	FLOPPY_CONNECTOR(config, m_floppy_connectors[0], myb3k_qd_floppies, "525qd", isa8_myb3k_fdc4711_device::myb3k_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy_connectors[1], myb3k_qd_floppies, "525qd", isa8_myb3k_fdc4711_device::myb3k_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy_connectors[2], myb3k_qd_floppies, "525qd", isa8_myb3k_fdc4711_device::myb3k_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy_connectors[3], myb3k_qd_floppies, "525qd", isa8_myb3k_fdc4711_device::myb3k_floppy_formats);

	isa8_myb3k_fdc471x_device_base::device_add_mconfig(config);
}

void isa8_myb3k_fdc4712_device::device_add_mconfig(machine_config &config)
{
	MB8876(config, m_fdc, XTAL(15'974'400) / 8);
	FLOPPY_CONNECTOR(config, m_floppy_connectors[0], myb3k_8inch_floppies, "8dsdd", isa8_myb3k_fdc4712_device::myb3k_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy_connectors[1], myb3k_8inch_floppies, "8dsdd", isa8_myb3k_fdc4712_device::myb3k_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy_connectors[2], myb3k_8inch_floppies, "8dsdd", isa8_myb3k_fdc4712_device::myb3k_floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy_connectors[3], myb3k_8inch_floppies, "8dsdd", isa8_myb3k_fdc4712_device::myb3k_floppy_formats);

	isa8_myb3k_fdc471x_device_base::device_add_mconfig(config);
}

//**************************************************************************
//  LIVE DEVICES
//**************************************************************************
isa8_myb3k_fdc471x_device_base::isa8_myb3k_fdc471x_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	device_isa8_card_interface(mconfig, *this),
	m_fdc(*this, "fdc"),
	m_floppy_connectors(*this, "fdc:%u", 0)
{
}

isa8_myb3k_fdc4710_device::isa8_myb3k_fdc4710_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_myb3k_fdc471x_device_base(mconfig, ISA8_MYB3K_FDC4710, tag, owner, clock)
{
	has_motor_control = true;
	io_base = 0x20;
	dma_channel = 2;
}

isa8_myb3k_fdc4711_device::isa8_myb3k_fdc4711_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_myb3k_fdc471x_device_base(mconfig, ISA8_MYB3K_FDC4711, tag, owner, clock)
{
	has_motor_control = true;
	io_base = 0x20;
	dma_channel = 2;
}

isa8_myb3k_fdc4712_device::isa8_myb3k_fdc4712_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	isa8_myb3k_fdc471x_device_base(mconfig, ISA8_MYB3K_FDC4712, tag, owner, clock),
	selected_drive(0)
{
	has_motor_control = false;
	io_base = 0x520;
	dma_channel = 1;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------
void isa8_myb3k_fdc471x_device_base::device_start()
{
	LOG("%s\n", FUNCNAME);

	set_isa_device();
	m_isa->install_device(io_base, io_base + 7, *this, &isa8_myb3k_fdc471x_device_base::map);
	m_isa->set_dma_channel(dma_channel, this, true);
}

//-------------------------------------------------
//  irq_w - signal interrupt request to ISA bus
//-------------------------------------------------
WRITE_LINE_MEMBER( isa8_myb3k_fdc471x_device_base::irq_w )
{
	LOG("%s: %d\n", FUNCNAME, state);
	m_isa->irq6_w(state ? ASSERT_LINE : CLEAR_LINE);
}

//-------------------------------------------------
//  drq_w - signal dma request to ISA bus
//-------------------------------------------------
WRITE_LINE_MEMBER( isa8_myb3k_fdc471x_device_base::drq_w )
{
	LOG("%s: %d\n", FUNCNAME, state);

	switch (dma_channel)
	{
	case 1:
		m_isa->drq1_w(state);
	case 2:
		m_isa->drq2_w(state);
	default:
		break;
	}
}

//-------------------------------------------------
//  dack_r - return FDC data through DMA
//-------------------------------------------------
uint8_t isa8_myb3k_fdc471x_device_base::dack_r(int line)
{
	return ~(m_fdc->data_r());
}

//-------------------------------------------------
//  dack_w - write DMA data to FDC
//-------------------------------------------------
void isa8_myb3k_fdc471x_device_base::dack_w(int line, uint8_t data)
{
	return m_fdc->data_w(data);
}

#if 0 // eop/tc is used to for logic around multi sector transfers
void isa8_myb3k_fdc471x_device_base::eop_w(int state)
{
	m_fdc->tc_w(state == ASSERT_LINE);
}
#endif

//--------------------------------------------------------
//  myb3k_inv_fdc_data_r - a LS240 inverts databus for FDC
//--------------------------------------------------------
READ8_MEMBER( isa8_myb3k_fdc471x_device_base::myb3k_inv_fdc_data_r )
{
	uint8_t tmp = m_fdc->read(offset);
	LOGR("%s: %02x -> %02x\n", FUNCNAME, tmp, (~tmp) & 0xff);
	return ~tmp;
}

//--------------------------------------------------------
//  myb3k_inv_fdc_data_w - a LS240 inverts databus for FDC
//--------------------------------------------------------
WRITE8_MEMBER( isa8_myb3k_fdc471x_device_base::myb3k_inv_fdc_data_w )
{
	LOG("%s: %02x -> %02x\n", FUNCNAME, data, (~data) & 0xff);
	m_fdc->write(offset, (~data) & 0xff);
}

//-------------------------------------------------
//  myb3k_fdc_command - descrete fdc card features
//-------------------------------------------------

WRITE8_MEMBER( isa8_myb3k_fdc471x_device_base::myb3k_fdc_command )
{
	LOG("%s: %02x\n", FUNCNAME, data);

	auto selected_drive = data & FDC_DRIVE_SEL;
	auto selected_side = (data & FDC_SIDE_SEL) ? 1 : 0;
	bool motor_on = (data & FDC_MOTOR_ON) != 0;
	bool dden = (data & FDC_DDEN) != 0;

	LOGCMD(" - Drive %d\n", selected_drive);
	LOGCMD(" - Side  %d\n", selected_side);
	LOGCMD(" - Density %s\n", dden ? "MFM" : "FM");
	
	if (has_motor_control)
		LOGCMD(" - Motor %s\n", motor_on ? "ON" : "OFF");

	auto floppy_connector = m_floppy_connectors[selected_drive];
	floppy_image_device *floppy = nullptr;

	if (floppy_connector.found())
		floppy = floppy_connector->get_device();
	
	m_fdc->set_floppy(floppy);

	if (floppy != nullptr)
	{
		floppy->ss_w(selected_side);

		if (has_motor_control)
			floppy->mon_w(motor_on ? 0 : 1); // Active low and inverter on incoming data line
	} 

	m_fdc->dden_w(dden ? 0 : 1); // active low == MFM
}

WRITE8_MEMBER( isa8_myb3k_fdc4712_device::myb3k_fdc_command )
{
	selected_drive = data & FDC_DRIVE_SEL;
	isa8_myb3k_fdc471x_device_base::myb3k_fdc_command(space, offset, data, mem_mask);
}

//-------------------------------------------------
//  myb3k_fdc_status - discrete fdc card status
//-------------------------------------------------
#define FDC_MSM_END_IR 0x01

READ8_MEMBER( isa8_myb3k_fdc471x_device_base::myb3k_fdc_status )
{
	LOG("%s\n", FUNCNAME);

	// TODO: return the multi sector mode interrupt status
	return 0x00;
}

READ8_MEMBER( isa8_myb3k_fdc4712_device::myb3k_fdc_status )
{
	uint8_t status = isa8_myb3k_fdc471x_device_base::myb3k_fdc_status(space, offset, mem_mask);

	auto floppy_connector = m_floppy_connectors[selected_drive];
	floppy_image_device *floppy = nullptr;
	
	if (floppy_connector.found())
		floppy = floppy_connector->get_device();

	if (floppy != nullptr && !floppy->twosid_r())
		status |= FDC_STATUS_DOUBLE_SIDED;

	return status;
}
