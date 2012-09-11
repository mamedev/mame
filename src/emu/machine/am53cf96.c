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

READ8_MEMBER( am53cf96_device::read )
{
	int rv;
	static const int states[] = { 0, 0, 1, 1, 2, 3, 4, 5, 6, 7, 0 };

	if (offset == REG_STATUS)
	{
		scsi_regs[REG_STATUS] &= ~0x7;
		scsi_regs[REG_STATUS] |= states[xfer_state];
		if (xfer_state < 10)
		{
			xfer_state++;
		}
	}

	rv = scsi_regs[offset];

	if (offset == REG_FIFO)
	{
//      mame_printf_debug("53cf96: read FIFO PC=%x\n", space.device().safe_pc());
		return 0;
	}

//  logerror("53cf96: read reg %d = %x (PC=%x)\n", reg, rv>>shift, space.device().safe_pc());

	if (offset == REG_IRQSTATE)
	{
		scsi_regs[REG_STATUS] &= ~0x80;	// clear IRQ flag
	}

	return rv;
}

void am53cf96_device::device_timer(emu_timer &timer, device_timer_id tid, int param, void *ptr)
{
	scsi_regs[REG_IRQSTATE] = 8;	// indicate success
	scsi_regs[REG_STATUS] |= 0x80;	// indicate IRQ
	irq_callback(machine());
}

WRITE8_MEMBER( am53cf96_device::write )
{
//  logerror("53cf96: w %x to reg %d (PC=%x)\n", data, offset, space.device().safe_pc());

	// if writing to the target ID, cache it off for later
	if (offset == REG_STATUS)
	{
		last_id = data;
	}

	if (offset == REG_XFERCNTLOW || offset == REG_XFERCNTMID || offset == REG_XFERCNTHI)
	{
		scsi_regs[REG_STATUS] &= ~0x10;	// clear CTZ bit
	}

	// FIFO
	if (offset == REG_FIFO)
	{
//      mame_printf_debug("%02x to FIFO @ %02d\n", data, fptr);
		fifo[fptr++] = data;
		if (fptr > 15)
		{
			fptr = 15;
		}
	}

	// command
	if (offset == REG_COMMAND)
	{
		//dma = (data & 0x80) ? 1 : 0;
		fptr = 0;
		switch (data & 0x7f)
		{
			case 0:	// NOP
				scsi_regs[REG_IRQSTATE] = 8;	// indicate success
				xfer_state = 0;
				break;
			case 2: // reset device
				scsi_regs[REG_IRQSTATE] = 8;	// indicate success

				logerror("53cf96: reset  target ID = %d (PC = %x)\n", last_id, space.device().safe_pc());
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
				m_transfer_timer->adjust( attotime::from_hz( 16384 ) );
				break;
			case 0x42:  	// select with ATN steps
				m_transfer_timer->adjust( attotime::from_hz( 16384 ) );
				if ((fifo[1] == 0) || (fifo[1] == 0x48) || (fifo[1] == 0x4b))
				{
					scsi_regs[REG_INTSTATE] = 6;
				}
				else
				{
					scsi_regs[REG_INTSTATE] = 4;
				}

				logerror("53cf96: command %x exec.  target ID = %d (PC = %x)\n", fifo[1], last_id, space.device().safe_pc());
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
				m_transfer_timer->adjust( attotime::from_hz( 16384 ) );
				scsi_regs[REG_INTSTATE] = 6;	// command sent OK
				break;
			default:
				printf( "unsupported command %02x\n", data );
				break;
		}
	}

	// only update the register mirror if it's not a write-only reg
	if (offset != REG_STATUS && offset != REG_INTSTATE && offset != REG_IRQSTATE && offset != REG_FIFOSTATE)
	{
		scsi_regs[offset] = data;
	}
}

am53cf96_device::am53cf96_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
    : device_t(mconfig, AM53CF96, "53CF96 SCSI", tag, owner, clock)
{
}

void am53cf96_device::device_config_complete()
{
	// inherit a copy of the static data
	const AM53CF96interface *intf = reinterpret_cast<const AM53CF96interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<AM53CF96interface *>(this) = *intf;
	}
}

void am53cf96_device::device_start()
{
	memset(scsi_regs, 0, sizeof(scsi_regs));
	memset(devices, 0, sizeof(devices));

	// try to open the devices
	for( device_t *device = owner()->first_subdevice(); device != NULL; device = device->next() )
	{
		scsidev_device *scsidev = dynamic_cast<scsidev_device *>(device);
		if( scsidev != NULL )
		{
			devices[scsidev->GetDeviceID()] = scsidev;
		}
	}

	fptr = 0;
	xfer_state = 0;

	save_item( NAME( scsi_regs ) );
	save_item( NAME( fifo ) );
	save_item( NAME( fptr ) );
	save_item( NAME( xfer_state ) );
	save_item( NAME( last_id ) );

	m_transfer_timer = timer_alloc( TIMER_TRANSFER );
}

// retrieve data from the SCSI controller
void am53cf96_device::dma_read_data(int bytes, UINT8 *pData)
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
void am53cf96_device::dma_write_data(int bytes, UINT8 *pData)
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

const device_type AM53CF96 = &device_creator<am53cf96_device>;
