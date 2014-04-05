/**********************************************************************

 Corvus Concept expansion port emulation

 Copyright MESS Team.
 Visit http://mamedev.org for licensing and usage restrictions.


 FIXME: Concept expansion ports should just use the Apple II Bus device!
 The code below is outdated and inaccurate!

 **********************************************************************/

#include "machine/concept_exp.h"

// FDC controller
#include "imagedev/flopdrv.h"
#include "formats/basicdsk.h"

// HDC controller
#include "imagedev/harddriv.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

// device type definition
const device_type CONCEPT_EXP_PORT = &device_creator<concept_exp_port_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  concept_exp_port_device - constructor
//-------------------------------------------------

concept_exp_port_device::concept_exp_port_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
					device_t(mconfig, CONCEPT_EXP_PORT, "Corvus Concept expansion port", tag, owner, clock, "concept_exp_port", __FILE__),
					device_slot_interface(mconfig, *this)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void concept_exp_port_device::device_start()
{
	m_card = dynamic_cast<concept_exp_card_device *>(get_card_device());
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void concept_exp_port_device::device_reset()
{
}


READ8_MEMBER( concept_exp_port_device::reg_r )
{
	if (m_card)
		return m_card->reg_r(space, offset);

	return 0;
}

WRITE8_MEMBER( concept_exp_port_device::reg_w )
{
	if (m_card != NULL)
		m_card->reg_w(space, offset, data);
}

READ8_MEMBER( concept_exp_port_device::rom_r )
{
	if (m_card)
		return m_card->reg_r(space, offset);

	return 0;
}

WRITE8_MEMBER( concept_exp_port_device::rom_w )
{
	if (m_card != NULL)
		m_card->reg_w(space, offset, data);
}


//**************************************************************************
//  CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  concept_exp_card_device - constructor
//-------------------------------------------------

concept_exp_card_device::concept_exp_card_device(const machine_config &mconfig, device_t &device)
					: device_slot_card_interface(mconfig, device)
{
}


//**************************************************************************
//  STUB EMULATION OF CARD DEVICES
//**************************************************************************

// FIXME: clean these up and move them in separate sources if suitable...

const device_type CONCEPT_FDC = &device_creator<concept_fdc_device>;
const device_type CONCEPT_HDC = &device_creator<concept_hdc_device>;

concept_fdc_device::concept_fdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, CONCEPT_FDC, "Corvus Concept FDC controller", tag, owner, clock, "concept_fdc", __FILE__),
						concept_exp_card_device( mconfig, *this )
{
}

concept_hdc_device::concept_hdc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: device_t(mconfig, CONCEPT_HDC, "Corvus Concept HDC controller", tag, owner, clock, "concept_hdc", __FILE__),
						concept_exp_card_device( mconfig, *this ),
						m_hdc(*this, "hdc")
{
}


// FDC Controller

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

void concept_fdc_device::device_start()
{
	m_wd179x = subdevice<fd1793_device>("wd179x");

	save_item(NAME(m_fdc_local_status));
	save_item(NAME(m_fdc_local_command));
}


void concept_fdc_device::device_reset()
{
	m_fdc_local_status = 0;
	m_fdc_local_command = 0;
}


WRITE_LINE_MEMBER(concept_fdc_device::intrq_w)
{
	if (state)
		m_fdc_local_status |= LS_INT_mask;
	else
		m_fdc_local_status &= ~LS_INT_mask;
}

WRITE_LINE_MEMBER(concept_fdc_device::drq_w)
{
	if (state)
		m_fdc_local_status |= LS_DRQ_mask;
	else
		m_fdc_local_status &= ~LS_DRQ_mask;
}

const wd17xx_interface concept_wd17xx_interface =
{
	DEVCB_NULL,
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, concept_fdc_device, intrq_w),
	DEVCB_DEVICE_LINE_MEMBER(DEVICE_SELF_OWNER, concept_fdc_device, drq_w),
	{FLOPPY_0, FLOPPY_1, FLOPPY_2, FLOPPY_3}
};

READ8_MEMBER(concept_fdc_device::reg_r)
{
	switch (offset)
	{
		case  0:    // LOCAL STATUS REG
			return m_fdc_local_status;

		case  8:    // FDC STATUS REG
			return m_wd179x->status_r(space, offset);

		case  9:    // FDC TRACK REG
			return m_wd179x->track_r(space, offset);

		case 10:    // FDC SECTOR REG
			return m_wd179x->sector_r(space, offset);

		case 11:    // FDC DATA REG
			return m_wd179x->data_r(space, offset);
	}

	return 0;
}

WRITE8_MEMBER(concept_fdc_device::reg_w)
{
	int current_drive;

	switch (offset)
	{
		case 0:     // LOCAL COMMAND REG
			m_fdc_local_command = data;

			m_wd179x->set_side((data & LC_FLPSD1_mask) != 0);
			current_drive = ((data >> LC_DE0_bit) & 1) | ((data >> (LC_DE1_bit-1)) & 2);
			m_wd179x->set_drive(current_drive);
			/*motor_on = (data & LC_MOTOROF_mask) == 0;*/
			// floppy_drive_set_motor_state(floppy_get_device(machine(),  current_drive), (data & LC_MOTOROF_mask) == 0 ? 1 : 0);
			/*flp_8in = (data & LC_FLP8IN_mask) != 0;*/
			m_wd179x->dden_w(BIT(data, 7));
			floppy_get_device(machine(), current_drive)->floppy_drive_set_ready_state(1, 0);
			break;

		case  8:    // FDC COMMAMD REG
			m_wd179x->command_w(space, offset, data);
			break;

		case  9:    // FDC TRACK REG
			m_wd179x->track_w(space, offset, data);
			break;

		case 10:    // FDC SECTOR REG
			m_wd179x->sector_w(space, offset, data);
			break;

		case 11:    // FDC DATA REG
			m_wd179x->data_w(space, offset, data);
			break;
	}
}

READ8_MEMBER(concept_fdc_device::rom_r)
{
	static const UINT8 data[] = "CORVUS01";
	return (offset < 8) ? data[offset] : 0;
}


static LEGACY_FLOPPY_OPTIONS_START(concept)
#if 1
/* SSSD 8" */
LEGACY_FLOPPY_OPTION(concept, "img", "Corvus Concept 8\" SSSD disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
						HEADS([1])
						TRACKS([77])
						SECTORS([26])
						SECTOR_LENGTH([128])
						FIRST_SECTOR_ID([1]))
#elif 0
/* SSDD 8" (according to ROMs) */
LEGACY_FLOPPY_OPTION(concept, "img", "Corvus Concept 8\" SSDD disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
						HEADS([1])
						TRACKS([77])
						SECTORS([26])
						SECTOR_LENGTH([256])
						FIRST_SECTOR_ID([1]))
#elif 0
/* Apple II DSDD 5"1/4 (according to ROMs) */
LEGACY_FLOPPY_OPTION(concept, "img", "Corvus Concept Apple II 5\"1/4 DSDD disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
						HEADS([2])
						TRACKS([35])
						SECTORS([16])
						SECTOR_LENGTH([256])
						FIRST_SECTOR_ID([1]))
#elif 0
/* actual formats found */
LEGACY_FLOPPY_OPTION(concept, "img", "Corvus Concept 5\"1/4 DSDD disk image (256-byte sectors)", basicdsk_identify_default, basicdsk_construct_default, NULL,
						HEADS([2])
						TRACKS([80])
						SECTORS([16])
						SECTOR_LENGTH([256])
						FIRST_SECTOR_ID([1]))
#else
LEGACY_FLOPPY_OPTION(concept, "img", "Corvus Concept 5\"1/4 DSDD disk image (512-byte sectors)", basicdsk_identify_default, basicdsk_construct_default, NULL,
						HEADS([2])
						TRACKS([80])
						SECTORS([9])
						SECTOR_LENGTH([512])
						FIRST_SECTOR_ID([1]))
#endif
LEGACY_FLOPPY_OPTIONS_END

static const floppy_interface concept_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(concept),
	NULL,
	NULL
};


static MACHINE_CONFIG_FRAGMENT( fdc )
	MCFG_FD1793_ADD("wd179x", concept_wd17xx_interface )
	MCFG_LEGACY_FLOPPY_4_DRIVES_ADD(concept_floppy_interface)
MACHINE_CONFIG_END

machine_config_constructor concept_fdc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(fdc);
}


// HDC Controller

void concept_hdc_device::device_start()
{
}


void concept_hdc_device::device_reset()
{
}


// Handle reads against the Hard Disk Controller's onboard registers
READ8_MEMBER(concept_hdc_device::reg_r)
{
	switch (offset)
	{
		case 0:     // HDC Data Register
			return m_hdc->read(space, offset);

		case 1:     // HDC Status Register
			return m_hdc->status_r(space, offset);
	}

	return 0;
}

// Handle writes against the Hard Disk Controller's onboard registers
WRITE8_MEMBER(concept_hdc_device::reg_w)
{
	switch (offset)
	{
		case 0:     // HDC Data Register
			m_hdc->write(space, offset, data);
			break;
	}
}

// Handle reads agsint the Hard Disk Controller's onboard ROM
READ8_MEMBER(concept_hdc_device::rom_r)
{
	static const UINT8 data[8] = { 0xa9, 0x20, 0xa9, 0x00, 0xa9, 0x03, 0xa9, 0x3c };            /* Same as Apple II */
	return (offset < 8) ? data[offset] : 0;
}



static MACHINE_CONFIG_FRAGMENT( hdc )
	MCFG_DEVICE_ADD("hdc", CORVUS_HDC, 0)
	MCFG_HARDDISK_ADD( "harddisk1" )
MACHINE_CONFIG_END

machine_config_constructor concept_hdc_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(hdc);
}
