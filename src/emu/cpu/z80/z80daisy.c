/***************************************************************************

    z80daisy.c

    Z80/180 daisy chaining support functions.

***************************************************************************/

#include "emu.h"
#include "z80daisy.h"


struct _z80_daisy_state
{
	z80_daisy_state *		next;			/* next device */
	running_device *	device;			/* associated device */
	z80_daisy_irq_state		irq_state;		/* IRQ state callback */
	z80_daisy_irq_ack		irq_ack;		/* IRQ ack callback */
	z80_daisy_irq_reti		irq_reti;		/* IRQ reti callback */
};


z80_daisy_state *z80daisy_init(running_device *cpudevice, const z80_daisy_chain *daisy)
{
	astring tempstring;
	z80_daisy_state *head = NULL;
	z80_daisy_state **tailptr = &head;

	/* create a linked list of devices */
	for ( ; daisy->devname != NULL; daisy++)
	{
		*tailptr = auto_alloc(cpudevice->machine, z80_daisy_state);
		(*tailptr)->next = NULL;
		(*tailptr)->device = cpudevice->machine->device(cpudevice->siblingtag(tempstring, daisy->devname));
		if ((*tailptr)->device == NULL)
			fatalerror("Unable to locate device '%s'", daisy->devname);
		(*tailptr)->irq_state = (z80_daisy_irq_state)(*tailptr)->device->get_config_fct(DEVINFO_FCT_IRQ_STATE);
		(*tailptr)->irq_ack = (z80_daisy_irq_ack)(*tailptr)->device->get_config_fct(DEVINFO_FCT_IRQ_ACK);
		(*tailptr)->irq_reti = (z80_daisy_irq_reti)(*tailptr)->device->get_config_fct(DEVINFO_FCT_IRQ_RETI);
		tailptr = &(*tailptr)->next;
	}

	return head;
}


void z80daisy_reset(z80_daisy_state *daisy)
{
	/* loop over all devices and call their reset function */
	for ( ; daisy != NULL; daisy = daisy->next)
		daisy->device->reset();
}


int z80daisy_update_irq_state(z80_daisy_state *daisy)
{
	/* loop over all devices; dev[0] is highest priority */
	for ( ; daisy != NULL; daisy = daisy->next)
	{
		int state = (*daisy->irq_state)(daisy->device);

		/* if this device is asserting the INT line, that's the one we want */
		if (state & Z80_DAISY_INT)
			return ASSERT_LINE;

		/* if this device is asserting the IEO line, it blocks everyone else */
		if (state & Z80_DAISY_IEO)
			return CLEAR_LINE;
	}

	return CLEAR_LINE;
}


int z80daisy_call_ack_device(z80_daisy_state *daisy)
{
	/* loop over all devices; dev[0] is the highest priority */
	for ( ; daisy != NULL; daisy = daisy->next)
	{
		int state = (*daisy->irq_state)(daisy->device);

		/* if this device is asserting the INT line, that's the one we want */
		if (state & Z80_DAISY_INT)
			return (*daisy->irq_ack)(daisy->device);
	}

	logerror("z80daisy_call_ack_device: failed to find an device to ack!\n");
	return 0;
}


void z80daisy_call_reti_device(z80_daisy_state *daisy)
{
	/* loop over all devices; dev[0] is the highest priority */
	for ( ; daisy != NULL; daisy = daisy->next)
	{
		int state = (*daisy->irq_state)(daisy->device);

		/* if this device is asserting the IEO line, that's the one we want */
		if (state & Z80_DAISY_IEO)
		{
			(*daisy->irq_reti)(daisy->device);
			return;
		}
	}

	logerror("z80daisy_call_reti_device: failed to find an device to reti!\n");
}
