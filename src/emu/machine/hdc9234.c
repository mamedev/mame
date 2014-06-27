/**************************************************************************

    HDC9234 Hard and Floppy Disk Controller
    Standard Microsystems Corporation (SMC)

    This controller handles MFM and FM encoded floppy disks and hard disks.
    A variant, the SMC9224, is used in some DEC systems.

    The HDC9234 is used in the Myarc HFDC card for the TI99/4A.

    References:
    [1] SMC HDC9234 preliminary data book (1988)

    The HDC9234 controller is also referred to as the "Universal Disk Controller" (UDC)
    by the data book

    Michael Zapf, June 2014

***************************************************************************/

#include "emu.h"
#include "hdc9234.h"

#define TRACE_REG 1
#define TRACE_ACT 1

/*
    Register names of the HDC. The left part is the set of write registers,
    while the right part are the read registers.
*/
enum
{
	// Write registers   |   Read registers
	//--------------------------------------
	DMA7_0=0,
	DMA15_8=1,
	DMA23_16=2,
	DESIRED_SECTOR=3,
	DESIRED_HEAD=4,         CURRENT_HEAD=4,
	DESIRED_CYLINDER=5,     CURRENT_CYLINDER=5,
	SECTOR_COUNT=6,         CURRENT_IDENT=6,
	RETRY_COUNT=7,          TEMP_STORAGE2=7,
	MODE=8,                 CHIP_STATUS=8,
	INT_COMM_TERM=9,        DRIVE_STATUS=9,
	DATA_DELAY=10,          DATA=10,
	COMMAND=11,             INT_STATUS=11
};

/*
    Definition of bits in the status register [1] p.7
*/
enum
{
	ST_INTPEND = 0x80 ,       // interrupt pending
	ST_DMAREQ  = 0x40 ,       // DMA request
	ST_DONE    = 0x20 ,       // command done
	ST_TERMCOD = 0x18 ,       // termination code (see below)
		TC_SUCCESS = 0x00 ,    // Successful completion
		TC_RDIDERR = 0x08 ,    // Error in READ-ID sequence
		TC_SEEKERR = 0x10 ,    // Error in SEEK sequence
		TC_DATAERR = 0x18 ,    // Error in DATA-TRANSFER seq.
	ST_RDYCHNG = 0x04 ,       // ready change
	ST_OVRUN   = 0x02 ,       // overrun/underrun
	ST_BADSECT = 0x01         // bad sector
};

/*
    Bits in the internal output registers. The registers are output via the
    auxiliary bus (AB)

    OUTPUT1
    AB7     drive select 3
    AB6     drive select 2
    AB5     drive select 1
    AB4     drive select 0
    AB3     programmable outputs
    AB2     programmable outputs
    AB1     programmable outputs
    AB0     programmable outputs

    OUTPUT2
    AB7     drive select 3* (active low, used for tape operations)
    AB6     reduce write current
    AB5     step direction
    AB4     step pulse
    AB3     desired head 3
    AB2     desired head 2
    AB1     desired head 1
    AB0     desired head 0
*/
enum
{
	OUT1_DRVSEL3    = 0x80,
	OUT1_DRVSEL2    = 0x40,
	OUT1_DRVSEL1    = 0x20,
	OUT1_DRVSEL0    = 0x10,
	OUT2_DRVSEL3I   = 0x80,
	OUT2_REDWRT     = 0x40,
	OUT2_STEPDIR    = 0x20,
	OUT2_STEPPULSE  = 0x10,
	OUT2_HEADSEL3   = 0x08,
	OUT2_HEADSEL2   = 0x04,
	OUT2_HEADSEL1   = 0x02,
	OUT2_HEADSEL0   = 0x01
};

enum
{
	TYPE_AT = 0x00,
	TYPE_HD = 0x01,
	TYPE_FLOPPY8 = 0x02,
	TYPE_FLOPPY5 = 0x03
};

hdc9234_device::hdc9234_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, HDC9234, "SMC HDC9234 Universal Disk Controller", tag, owner, clock, "hdc9234", __FILE__),
	m_out_intrq(*this),
	m_out_dip(*this),
	m_out_auxbus(*this),
	m_in_auxbus(*this),
	m_in_dma(*this),
	m_out_dma(*this)
{
}

/*
    Set or reset some bits.
*/
void hdc9234_device::set_bits(UINT8& byte, int mask, bool set)
{
	if (set) byte |= mask;
	else byte &= ~mask;
}

/*
    Set/clear INT

    Interupts are generated in the following occasions:
    - when the DONE bit is set to 1 in the ISR and ST_DONE is set to 1
    - when the READY_CHANGE bit is set to 1 in the ISR and ST_RDYCHNG is set to 1
    (ready change: 1->0 or 0->1)
*/
void hdc9234_device::set_interrupt(line_state intr)
{
	if (intr == ASSERT_LINE)
	{
		// Only if there is not already a pending interrupt
		if ((m_register_r[INT_STATUS] & ST_INTPEND) == 0)
		{
			m_register_r[INT_STATUS] |= ST_INTPEND;
			m_out_intrq(intr);
		}
	}
	else
	{
		// if there is a pending interrupt
		if ((m_register_r[INT_STATUS] & ST_INTPEND) != 0)
			m_out_intrq(intr);
	}
}

/*
    Process a command
*/
void hdc9234_device::process_command(UINT8 opcode)
{
	// Reset DONE and BAD_SECTOR [1], p.7
	set_bits(m_register_r[INT_STATUS], ST_DONE | ST_BADSECT, false);

/*
    // Reset interrupt line (not explicitly mentioned in spec, but seems reasonable
    set_interrupt(CLEAR_LINE);

    // Clear Interrupt Pending and Ready Change
    set_bits(m_register_r[INT_STATUS], ST_INTPEND | ST_RDYCHNG, false);
*/
	m_command = opcode;

	if (opcode == 0x00)
	{
		// RESET
		// same effect as the RST* pin being active
		if (TRACE_ACT) logerror("%s: Reset command\n", tag());
		device_reset();
	}
	else if (opcode == 0x01)
	{
		// DESELECT DRIVE
		// done when no drive is in use
		if (TRACE_ACT) logerror("%s: drdeselect command\n", tag());
		set_bits(m_output1, OUT1_DRVSEL3|OUT1_DRVSEL2|OUT1_DRVSEL1|OUT1_DRVSEL0, false);
		set_bits(m_output2, OUT2_DRVSEL3I, true); // inverted level
	}
	else if (opcode >= 0x20 && opcode <= 0x3f)
	{
		// DRIVE SELECT
		if (TRACE_ACT) logerror("%s: drselect command %02x\n", tag(), opcode);
		drive_select(opcode&0x1f);
	}
	else if (opcode >= 0x40 && opcode <= 0x4f)
	{
		// SETREGPTR
		if (TRACE_ACT) logerror("%s: setregptr command %02x\n", tag(), opcode);
		m_register_pointer = opcode & 0xf;
		// Spec does not say anything about the effect of setting an
		// invalid value (only "care should be taken")
		if (m_register_pointer > 10)
		{
			logerror("%s: set register pointer: Invalid register number: %d. Setting to 10.\n", tag(), m_register_pointer);
			m_register_pointer = 10;
		}
	}
}

void hdc9234_device::drive_select(int driveparm)
{
	// Command word
	//
	//        7     6     5     4     3     2     1     0
	//     +-----+-----+-----+-----+-----+-----+-----+-----+
	//     |  0  |  0  |  1  |Delay|    Type   |   Drive   |
	//     +-----+-----+-----+-----+-----+-----+-----+-----+
	//
	// [1] p.5: lower 4 bits of retry count register is put on OUTPUT1
	m_output1 = (0x10 << (driveparm & 0x03)) | (m_register_w[RETRY_COUNT]&0x0f);

	// The drive type is used to configure DMA burst mode ([1], p.12)
	// and to select the timing parameters
	m_selected_drive_type = (driveparm>>2) & 0x03;
	m_head_load_delay_enable = (driveparm>>4)&0x01;

/*
    // We need to store the type of the drive for the poll_drives command
    // to be able to correctly select the device (floppy or hard disk).
    m_types[driveparm&0x03] = m_selected_drive_type;
*/
	// Copy the DMA registers to registers CURRENT_HEAD, CURRENT_CYLINDER,
	// and CURRENT_IDENT. This is required during formatting ([1], p. 14)
	// as the format command reuses the registers for formatting parameters.
	m_register_r[CURRENT_HEAD] = m_register_r[DMA7_0];
	m_register_r[CURRENT_CYLINDER] = m_register_r[DMA15_8];
	m_register_r[CURRENT_IDENT] = m_register_r[DMA23_16];

	m_register_r[CHIP_STATUS] = (m_register_r[CHIP_STATUS] & 0xfc) | (driveparm & 0x03);

	sync_latches_out();
	sync_status_in();
}

/*
    This is pretty simple here, compared to wd17xx, because index and ready
    callbacks have to be tied to the controller board outside the chip.
*/
void hdc9234_device::connect_floppy_drive(floppy_image_device* floppy)
{
	m_floppy = floppy;
}

/*
    Get the state from outside and latch it in the register.
    There should be a bus driver on the PCB which provides the signals from
    both the hard and floppy drives during S0=S1=0 and STB*=0 times via the
    auxiliary bus.
*/
void hdc9234_device::sync_status_in()
{
	UINT8 prev = m_register_r[DRIVE_STATUS];
	m_register_r[DRIVE_STATUS] = m_in_auxbus(0);

	// Raise interrupt if ready changes.
	if (((m_register_r[DRIVE_STATUS] & HDC_DS_READY) != (prev & HDC_DS_READY))
		& (m_register_r[INT_STATUS] & ST_RDYCHNG))
	{
		set_interrupt(ASSERT_LINE);
	}
}

/*
    Push the output registers over the auxiliary bus. It is expected that
    the PCB contains latches to store the values.
    TODO: Timing (the spec is not clear at that point)
*/
void hdc9234_device::sync_latches_out()
{
	m_output1 = (m_output1 & 0xf0) | (m_register_w[RETRY_COUNT]&0x0f);
	m_out_auxbus((offs_t)HDC_OUTPUT_1, m_output1);
	m_out_auxbus((offs_t)HDC_OUTPUT_2, m_output2);
}

/*
    Read a byte of data from the controller
    The address (offset) encodes the C/D* line (command and /data)
*/
READ8_MEMBER( hdc9234_device::read )
{
	UINT8 reply = 0;

	if ((offset & 1) == 0)
	{
		// Data register
		reply = m_register_r[m_register_pointer];
		if (TRACE_REG) logerror("%s: register[%d] -> %02x\n", tag(), m_register_pointer, reply);

		// Autoincrement until DATA is reached.
		if (m_register_pointer < DATA)  m_register_pointer++;
	}
	else
	{
		// Status register
		reply = m_register_r[INT_STATUS];

		// "The interrupt pin is reset to its inactive state
		// when the UDC interrupt status register is read." [1] (p.3)
		if (TRACE_REG) logerror("%s: interrupt status register -> %02x\n", tag(), reply);
		set_interrupt(CLEAR_LINE);

		// Clear the bits due to interrupt status register read.
		m_register_r[INT_STATUS] &= ~(ST_INTPEND | ST_RDYCHNG);
	}
	return reply;
}

/*
    Write a byte to the controller
    The address (offset) encodes the C/D* line (command and /data)
*/
WRITE8_MEMBER( hdc9234_device::write )
{
//  logerror("%s: Write access to %04x: %d\n", tag(), offset & 0xffff, data);

	data &= 0xff;
	if ((offset & 1)==0)
	{
		// Writing data to registers

		// Data register
		if (TRACE_REG) logerror("%s: register[%d] <- %02x\n", tag(), m_register_pointer, data);
		m_register_w[m_register_pointer] = data;

		// The DMA registers and the sector register for read and
		// write are identical, so in that case we copy the contents
		if (m_register_pointer < DESIRED_HEAD)  m_register_r[m_register_pointer] = data;

		// Autoincrement until DATA is reached.
		if (m_register_pointer < DATA)  m_register_pointer++;
	}
	else
	{
		process_command(data);
	}
}

/*
    Reset the controller. Negative logic, but we use ASSERT_LINE.
*/
WRITE_LINE_MEMBER( hdc9234_device::reset )
{
	if (state == ASSERT_LINE)
	{
		logerror("%s: Reset via RST line\n", tag());
		device_reset();
	}
}

void hdc9234_device::device_start()
{
	logerror("%s: start\n", tag());
	m_out_intrq.resolve_safe();
	m_out_dip.resolve_safe();
	m_out_auxbus.resolve_safe();
	m_in_auxbus.resolve_safe(0);
	m_out_dma.resolve_safe();
	m_in_dma.resolve_safe(0);

	// allocate timers
}

void hdc9234_device::device_reset()
{
	m_command = 0;
	m_selected_drive_type = 0;
	m_head_load_delay_enable = false;
	m_register_pointer = 0;
}

const device_type HDC9234 = &device_creator<hdc9234_device>;
