/*
 * am53cf96.c
 *
 * AMD/NCR/Symbios 53CF96 SCSI-2 controller.
 * Qlogic FAS-236 and Emulex ESP-236 are equivalents
 *
 * References:
 * AMD Am53CF96 manual
 *
 */

#include "emu.h"
#include "am53cf96.h"
#include "machine/scsidev.h"

static UINT8 scsi_regs[32], fifo[16], fptr = 0, xfer_state, last_id;
static const struct AM53CF96interface *intf;

static scsidev_device *devices[8];	// SCSI IDs 0-7

// 53CF96 register set
enum
{
	REG_XFERCNTLOW = 0,	// read = current xfer count lo byte, write = set xfer count lo byte
	REG_XFERCNTMID,		// read = current xfer count mid byte, write = set xfer count mid byte
	REG_FIFO,		// read/write = FIFO
	REG_COMMAND,		// read/write = command

	REG_STATUS,		// read = status, write = destination SCSI ID (4)
	REG_IRQSTATE,		// read = IRQ status, write = timeout         (5)
	REG_INTSTATE,		// read = internal state, write = sync xfer period (6)
	REG_FIFOSTATE,		// read = FIFO status, write = sync offset
	REG_CTRL1,		// read/write = control 1
	REG_CLOCKFCTR,		// clock factor (write only)
	REG_TESTMODE,		// test mode (write only)
	REG_CTRL2,		// read/write = control 2
	REG_CTRL3,		// read/write = control 3
	REG_CTRL4,		// read/write = control 4
	REG_XFERCNTHI,		// read = current xfer count hi byte, write = set xfer count hi byte
	REG_DATAALIGN		// data alignment (write only)
};

READ32_HANDLER( am53cf96_r )
{
	int reg, shift, rv;
	static const int states[] = { 0, 0, 1, 1, 2, 3, 4, 5, 6, 7, 0 };

	reg = offset * 2;
	if (mem_mask == 0x000000ff)
	{
		shift = 0;
	}
	else
	{
		reg++;
		shift = 16;
	}

	if (reg == REG_STATUS)
	{
		scsi_regs[REG_STATUS] &= ~0x7;
		scsi_regs[REG_STATUS] |= states[xfer_state];
		if (xfer_state < 10)
		{
			xfer_state++;
		}
	}

	rv = scsi_regs[reg]<<shift;

	if (reg == REG_FIFO)
	{
//      mame_printf_debug("53cf96: read FIFO PC=%x\n", cpu_get_pc(&space->device()));
		return 0;
	}

//  logerror("53cf96: read reg %d = %x (PC=%x)\n", reg, rv>>shift, cpu_get_pc(&space->device()));

	if (reg == REG_IRQSTATE)
	{
		scsi_regs[REG_STATUS] &= ~0x80;	// clear IRQ flag
	}

	return rv;
}

static TIMER_CALLBACK( am53cf96_irq )
{
	scsi_regs[REG_IRQSTATE] = 8;	// indicate success
	scsi_regs[REG_STATUS] |= 0x80;	// indicate IRQ
	intf->irq_callback(machine);
}

WRITE32_HANDLER( am53cf96_w )
{
	int reg, val/*, dma*/;

	reg = offset * 2;
	val = data;
	if (mem_mask == 0x000000ff)
	{
	}
	else
	{
		reg++;
		val >>= 16;
	}
	val &= 0xff;

//  logerror("53cf96: w %x to reg %d (ofs %02x data %08x mask %08x PC=%x)\n", val, reg, offset, data, mem_mask, cpu_get_pc(&space->device()));

	// if writing to the target ID, cache it off for later
	if (reg == REG_STATUS)
	{
		last_id = val;
	}

	if (reg == REG_XFERCNTLOW || reg == REG_XFERCNTMID || reg == REG_XFERCNTHI)
	{
		scsi_regs[REG_STATUS] &= ~0x10;	// clear CTZ bit
	}

	// FIFO
	if (reg == REG_FIFO)
	{
//      mame_printf_debug("%02x to FIFO @ %02d\n", val, fptr);
		fifo[fptr++] = val;
		if (fptr > 15)
		{
			fptr = 15;
		}
	}

	// command
	if (reg == REG_COMMAND)
	{
		//dma = (val & 0x80) ? 1 : 0;
		fptr = 0;
		switch (val & 0x7f)
		{
			case 0:	// NOP
				scsi_regs[REG_IRQSTATE] = 8;	// indicate success
				xfer_state = 0;
				break;
			case 2: // reset device
				scsi_regs[REG_IRQSTATE] = 8;	// indicate success

				logerror("53cf96: reset  target ID = %d (PC = %x)\n", last_id, cpu_get_pc(&space->device()));
				if (devices[last_id])
				{
					devices[last_id]->reset();
				}
				else
				{
					logerror("53cf96: reset request for unknown device SCSI ID %d\n", last_id);
				}

				xfer_state = 0;
				break;
			case 3:	// reset SCSI bus
				scsi_regs[REG_INTSTATE] = 4;	// command sent OK
				xfer_state = 0;
				space->machine().scheduler().timer_set( attotime::from_hz( 16384 ), FUNC(am53cf96_irq ));
				break;
			case 0x42:  	// select with ATN steps
				space->machine().scheduler().timer_set( attotime::from_hz( 16384 ), FUNC(am53cf96_irq ));
				if ((fifo[1] == 0) || (fifo[1] == 0x48) || (fifo[1] == 0x4b))
				{
					scsi_regs[REG_INTSTATE] = 6;
				}
				else
				{
					scsi_regs[REG_INTSTATE] = 4;
				}

				logerror("53cf96: command %x exec.  target ID = %d (PC = %x)\n", fifo[1], last_id, cpu_get_pc(&space->device()));
				if (devices[last_id])
				{
					int length;

					devices[last_id]->SetCommand( &fifo[1], 12 );
					devices[last_id]->ExecCommand( &length );
				}
				else
				{
					logerror("53cf96: request for unknown device SCSI ID %d\n", last_id);
				}
				xfer_state = 0;
				break;
			case 0x44:	// enable selection/reselection
				xfer_state = 0;
				break;
			case 0x10:	// information transfer (must not change xfer_state)
			case 0x11:	// second phase of information transfer
			case 0x12:	// message accepted
				space->machine().scheduler().timer_set( attotime::from_hz( 16384 ), FUNC(am53cf96_irq ));
				scsi_regs[REG_INTSTATE] = 6;	// command sent OK
				break;
			default:
				printf( "unsupported command %02x\n", val );
				break;
		}
	}

	// only update the register mirror if it's not a write-only reg
	if (reg != REG_STATUS && reg != REG_INTSTATE && reg != REG_IRQSTATE && reg != REG_FIFOSTATE)
	{
		scsi_regs[reg] = val;
	}
}

void am53cf96_init( running_machine &machine, const struct AM53CF96interface *interface )
{
	int i;

	// save interface pointer for later
	intf = interface;

	memset(scsi_regs, 0, sizeof(scsi_regs));
	memset(devices, 0, sizeof(devices));

	// try to open the devices
	for (i = 0; i < interface->scsidevs->devs_present; i++)
	{
		scsidev_device *device = machine.device<scsidev_device>( interface->scsidevs->devices[i].tag );
		devices[device->GetDeviceID()] = device;
	}

	state_save_register_global_array(machine, scsi_regs);
	state_save_register_global_array(machine, fifo);
	state_save_register_global(machine, fptr);
	state_save_register_global(machine, xfer_state);
	state_save_register_global(machine, last_id);
}

// retrieve data from the SCSI controller
void am53cf96_read_data(int bytes, UINT8 *pData)
{
	scsi_regs[REG_STATUS] |= 0x10;	// indicate DMA finished

	if (devices[last_id])
	{
		devices[last_id]->ReadData( pData, bytes );
	}
	else
	{
		logerror("53cf96: request for unknown device SCSI ID %d\n", last_id);
	}
}

// write data to the SCSI controller
void am53cf96_write_data(int bytes, UINT8 *pData)
{
//  int i;

	scsi_regs[REG_STATUS] |= 0x10;	// indicate DMA finished

	if (devices[last_id])
	{
		devices[last_id]->WriteData( pData, bytes );
	}
	else
	{
		logerror("53cf96: request for unknown device SCSI ID %d\n", last_id);
	}
}
