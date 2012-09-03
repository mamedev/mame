
/*

    NEC uPD71071 DMA Controller
    Used on the Fujitsu FM-Towns

    Register description:

    0x00:   Initialise (Write-only)
            - bit 0: Reset
            - bit 1: 16-bit data bus

    0x01:   Channel Register
            On read:
            - bits 0-3: Selected channel
            - bit 4: Only base registers may be read or written
            On write:
            - bits 0-1: Select channel for programming count, address, and mode registers
            - bit 2: Only base registers can be read or written to

    0x02:
    0x03:   Count Register (16-bit)
            DMA Transfer counter

    0x04:
    0x05:
    0x06:
    0x07:   Address Register (32-bit)
            Self-explanatory, I hope. :)
            NOTE: Datasheet clearly shows this as 24-bit, with register 7 unused.
            But the FM-Towns definitely uses reg 7 as bits 24-31.

    0x08:
    0x09:   Device Control register (16-bit)
            bit 0: Enable memory-to-memory (MTM) transfers
            bit 1: Enable fixed address for channel 0 only (MTM only)
            bit 2: Disable DMA operation (stops HLDRQ signal to the CPU)
            bit 3: Use compressed timing
            bit 4: Rotational Priority
            bit 5: Extended Writing
            bit 6: DMARQ active level (1=active low)
            bit 7: DMAAK active level (1=active high)
            bit 8: Bus mode (0=bus release, 1=bus hold)
            bit 9: Wait Enable during Verify

    0x0a:   Mode Control register
            bit 0: Transfer size (1=16-bit, 0=8-bit,  16-bit data bus size only)
            bit 2-3: Transfer direction (ignored for MTM transfers)
                        00 = Verify
                        01 = I/O to memory
                        10 = memory to I/O
                        11 = invalid
            bit 4: Enable auto-initialise
            bit 5: Address direction (0=increment, 1=decrement, affects only current Address reg)
            bit 6-7: Transfer mode (ignored for MTM transfers)
                        00 = Demand
                        01 = Single
                        10 = Block
                        11 = Cascade

    0x0b:   Status register
            bit 0-3: Terminal count (per channel)
            bit 4-7: DMA request present (external hardware DMA only)

    0x0c:
    0x0d:   Temporary register (16-bit, read-only)
            Stores the last data transferred in an MTM transfer

    0x0e:   Request register
            bit 0-3: Software DMA request (1=set)
            bit 0 only in MTM transfers

    0x0f:   Mask register
            bit 0-3: DMARQ mask
            bits 1 and 0 only in MTM transfers


*/

#include "emu.h"
#include "machine/upd71071.h"

struct upd71071_reg
{
	UINT8 initialise;
	UINT8 channel;
	UINT16 count_current[4];
	UINT16 count_base[4];
	UINT32 address_current[4];
	UINT32 address_base[4];
	UINT16 device_control;
	UINT8 mode_control[4];
	UINT8 status;
	UINT8 temp_l;
	UINT8 temp_h;
	UINT8 request;
	UINT8 mask;
};

typedef struct _upd71071_t upd71071_t;
struct _upd71071_t
{
	struct upd71071_reg reg;
	int selected_channel;
	int buswidth;
	int dmarq[4];
	emu_timer* timer[4];
	int in_progress[4];
	int transfer_size[4];
	int base;
	const upd71071_intf* intf;
};

INLINE upd71071_t *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == UPD71071);

	return (upd71071_t*)downcast<upd71071_device *>(device)->token();
}

static TIMER_CALLBACK(dma_transfer_timer)
{
	// single byte or word transfer
	device_t* device = (device_t*)ptr;
	upd71071_t* dmac = get_safe_token(device);
	address_space* space = device->machine().device(dmac->intf->cputag)->memory().space(AS_PROGRAM);
	int channel = param;
	UINT16 data = 0;  // data to transfer

	switch(dmac->reg.mode_control[channel] & 0x0c)
	{
		case 0x00:  // Verify
			break;
		case 0x04:  // I/O -> memory
			if(dmac->intf->dma_read[channel])
				data = dmac->intf->dma_read[channel](device->machine());
			space->write_byte(dmac->reg.address_current[channel],data & 0xff);
			if(dmac->reg.mode_control[channel] & 0x20)  // Address direction
				dmac->reg.address_current[channel]--;
			else
				dmac->reg.address_current[channel]++;
			if(dmac->reg.count_current[channel] == 0)
			{
				if(dmac->reg.mode_control[channel] & 0x10)  // auto-initialise
				{
					dmac->reg.address_current[channel] = dmac->reg.address_base[channel];
					dmac->reg.count_current[channel] = dmac->reg.count_base[channel];
				}
				// TODO: send terminal count
			}
			else
				dmac->reg.count_current[channel]--;
			break;
		case 0x08:  // memory -> I/O
			data = space->read_byte(dmac->reg.address_current[channel]);
			if(dmac->intf->dma_read[channel])
				dmac->intf->dma_write[channel](device->machine(),data);
			if(dmac->reg.mode_control[channel] & 0x20)  // Address direction
				dmac->reg.address_current[channel]--;
			else
				dmac->reg.address_current[channel]++;
			if(dmac->reg.count_current[channel] == 0)
			{
				if(dmac->reg.mode_control[channel] & 0x10)  // auto-initialise
				{
					dmac->reg.address_current[channel] = dmac->reg.address_base[channel];
					dmac->reg.count_current[channel] = dmac->reg.count_base[channel];
				}
				// TODO: send terminal count
			}
			else
				dmac->reg.count_current[channel]--;
			break;
		case 0x0c:  // Invalid
			break;
	}
}

static void upd71071_soft_reset(device_t* device)
{
	upd71071_t* dmac = get_safe_token(device);
	int x;

	// Does not change base/current address, count, or buswidth
	dmac->selected_channel = 0;
	dmac->base = 0;
	for(x=0;x<4;x++)
		dmac->reg.mode_control[x] = 0;
	dmac->reg.device_control = 0;
	dmac->reg.temp_h = 0;
	dmac->reg.temp_l = 0;
	dmac->reg.mask = 0x0f;  // mask all channels
	dmac->reg.status &= ~0x0f;  // clears bits 0-3 only
	dmac->reg.request = 0;
}

int upd71071_dmarq(device_t* device, int state,int channel)
{
	upd71071_t* dmac = get_safe_token(device);

	if(state != 0)
	{
		if(dmac->reg.device_control & 0x0004)
			return 2;

		if(dmac->reg.mask & (1 << channel))  // is channel masked?
			return 1;

		dmac->dmarq[channel] = 1;  // DMARQ line is set
		dmac->reg.status |= (0x10 << channel);

		// start transfer
		switch(dmac->reg.mode_control[channel] & 0xc0)
		{
			case 0x00:  // Demand
				// TODO
				break;
			case 0x40:  // Single
				dmac->timer[channel]->adjust(attotime::from_hz(dmac->intf->clock),channel);
				break;
			case 0x80:  // Block
				// TODO
				break;
			case 0xc0:  // Cascade
				// TODO
				break;

		}
	}
	else
	{
		dmac->dmarq[channel] = 0;  // clear DMARQ line
		dmac->reg.status &= ~(0x10 << channel);
		dmac->reg.status |= (0x01 << channel);  // END or TC
	}
	return 0;
}

static DEVICE_START(upd71071)
{
	upd71071_t* dmac = get_safe_token(device);
	int x;

	dmac->intf = (const upd71071_intf*)device->static_config();
	for(x=0;x<4;x++)
	{
		dmac->timer[x] = device->machine().scheduler().timer_alloc(FUNC(dma_transfer_timer), (void*)device);
	}
	dmac->selected_channel = 0;
}

static READ8_DEVICE_HANDLER(upd71071_read)
{
	upd71071_t* dmac = get_safe_token(device);
	UINT8 ret = 0;

	logerror("DMA: read from register %02x\n",offset);
	switch(offset)
	{
		case 0x01:  // Channel
			ret = (1 << dmac->selected_channel);
			if(dmac->base != 0)
				ret |= 0x10;
			break;
		case 0x02:  // Count (low)
			if(dmac->base != 0)
				ret = dmac->reg.count_base[dmac->selected_channel] & 0xff;
			else
				ret = dmac->reg.count_current[dmac->selected_channel] & 0xff;
			break;
		case 0x03:  // Count (high)
			if(dmac->base != 0)
				ret = (dmac->reg.count_base[dmac->selected_channel] >> 8) & 0xff;
			else
				ret = (dmac->reg.count_current[dmac->selected_channel] >> 8) & 0xff;
			break;
		case 0x04:  // Address (low)
			if(dmac->base != 0)
				ret = dmac->reg.address_base[dmac->selected_channel] & 0xff;
			else
				ret = dmac->reg.address_current[dmac->selected_channel] & 0xff;
			break;
		case 0x05:  // Address (mid)
			if(dmac->base != 0)
				ret = (dmac->reg.address_base[dmac->selected_channel] >> 8) & 0xff;
			else
				ret = (dmac->reg.address_current[dmac->selected_channel] >> 8) & 0xff;
			break;
		case 0x06:  // Address (high)
			if(dmac->base != 0)
				ret = (dmac->reg.address_base[dmac->selected_channel] >> 16) & 0xff;
			else
				ret = (dmac->reg.address_current[dmac->selected_channel] >> 16) & 0xff;
			break;
		case 0x07:  // Address (highest)
			if(dmac->base != 0)
				ret = (dmac->reg.address_base[dmac->selected_channel] >> 24) & 0xff;
			else
				ret = (dmac->reg.address_current[dmac->selected_channel] >> 24) & 0xff;
			break;
		case 0x08:  // Device control (low)
			ret = dmac->reg.device_control & 0xff;
			break;
		case 0x09:  // Device control (high)
			ret = (dmac->reg.device_control >> 8) & 0xff;
			break;
		case 0x0a:  // Mode control
			ret = dmac->reg.mode_control[dmac->selected_channel];
			break;
		case 0x0b:  // Status
			ret = dmac->reg.status;
			dmac->reg.status &= ~0x0f;  // resets END/TC?
			break;
		case 0x0c:  // Temporary (low)
			ret = dmac->reg.temp_h;
			break;
		case 0x0d:  // Temporary (high)
			ret = dmac->reg.temp_l;
			break;
		case 0x0e:  // Request
			ret = dmac->reg.request;
			break;
		case 0x0f:  // Mask
			ret = dmac->reg.mask;
			break;
	}
	return ret;
}

static WRITE8_DEVICE_HANDLER(upd71071_write)
{
	upd71071_t* dmac = get_safe_token(device);

	switch(offset)
	{
		case 0x00:  // Initialise
			// TODO: reset (bit 0)
			dmac->buswidth = data & 0x02;
			if(data & 0x01)
				upd71071_soft_reset(device);
			logerror("DMA: Initialise [%02x]\n",data);
			break;
		case 0x01:  // Channel
			dmac->selected_channel = data & 0x03;
			dmac->base = data & 0x04;
			logerror("DMA: Channel selected [%02x]\n",data);
			break;
		case 0x02:  // Count (low)
			dmac->reg.count_base[dmac->selected_channel] =
				(dmac->reg.count_base[dmac->selected_channel] & 0xff00) | data;
			if(dmac->base == 0)
				dmac->reg.count_current[dmac->selected_channel] =
					(dmac->reg.count_current[dmac->selected_channel] & 0xff00) | data;
			logerror("DMA: Channel %i Counter set [%04x]\n",dmac->selected_channel,dmac->reg.count_base[dmac->selected_channel]);
			break;
		case 0x03:  // Count (high)
			dmac->reg.count_base[dmac->selected_channel] =
				(dmac->reg.count_base[dmac->selected_channel] & 0x00ff) | (data << 8);
			if(dmac->base == 0)
				dmac->reg.count_current[dmac->selected_channel] =
					(dmac->reg.count_current[dmac->selected_channel] & 0x00ff) | (data << 8);
			logerror("DMA: Channel %i Counter set [%04x]\n",dmac->selected_channel,dmac->reg.count_base[dmac->selected_channel]);
			break;
		case 0x04:  // Address (low)
			dmac->reg.address_base[dmac->selected_channel] =
				(dmac->reg.address_base[dmac->selected_channel] & 0xffffff00) | data;
			if(dmac->base == 0)
				dmac->reg.address_current[dmac->selected_channel] =
					(dmac->reg.address_current[dmac->selected_channel] & 0xffffff00) | data;
			logerror("DMA: Channel %i Address set [%08x]\n",dmac->selected_channel,dmac->reg.address_base[dmac->selected_channel]);
			break;
		case 0x05:  // Address (mid)
			dmac->reg.address_base[dmac->selected_channel] =
				(dmac->reg.address_base[dmac->selected_channel] & 0xffff00ff) | (data << 8);
			if(dmac->base == 0)
				dmac->reg.address_current[dmac->selected_channel] =
					(dmac->reg.address_current[dmac->selected_channel] & 0xffff00ff) | (data << 8);
			logerror("DMA: Channel %i Address set [%08x]\n",dmac->selected_channel,dmac->reg.address_base[dmac->selected_channel]);
			break;
		case 0x06:  // Address (high)
			dmac->reg.address_base[dmac->selected_channel] =
				(dmac->reg.address_base[dmac->selected_channel] & 0xff00ffff) | (data << 16);
			if(dmac->base == 0)
				dmac->reg.address_current[dmac->selected_channel] =
					(dmac->reg.address_current[dmac->selected_channel] & 0xff00ffff) | (data << 16);
			logerror("DMA: Channel %i Address set [%08x]\n",dmac->selected_channel,dmac->reg.address_base[dmac->selected_channel]);
			break;
		case 0x07:  // Address (highest)
			dmac->reg.address_base[dmac->selected_channel] =
				(dmac->reg.address_base[dmac->selected_channel] & 0x00ffffff) | (data << 24);
			if(dmac->base == 0)
				dmac->reg.address_current[dmac->selected_channel] =
					(dmac->reg.address_current[dmac->selected_channel] & 0x00ffffff) | (data << 24);
			logerror("DMA: Channel %i Address set [%08x]\n",dmac->selected_channel,dmac->reg.address_base[dmac->selected_channel]);
			break;
		case 0x08:  // Device control (low)
			dmac->reg.device_control = (dmac->reg.device_control & 0xff00) | data;
			logerror("DMA: Device control set [%04x]\n",dmac->reg.device_control);
			break;
		case 0x09:  // Device control (high)
			dmac->reg.device_control = (dmac->reg.device_control & 0x00ff) | (data << 8);
			logerror("DMA: Device control set [%04x]\n",dmac->reg.device_control);
			break;
		case 0x0a:  // Mode control
			dmac->reg.mode_control[dmac->selected_channel] = data;
			logerror("DMA: Channel %i Mode control set [%02x]\n",dmac->selected_channel,dmac->reg.mode_control[dmac->selected_channel]);
			break;
		case 0x0e:  // Request
			dmac->reg.request = data;
			logerror("DMA: Request set [%02x]\n",data);
			break;
		case 0x0f:  // Mask
			dmac->reg.mask = data;
			logerror("DMA: Mask set [%02x]\n",data);
			break;
	}
}

READ8_DEVICE_HANDLER(upd71071_r) { return upd71071_read(device,offset); }
WRITE8_DEVICE_HANDLER(upd71071_w) { upd71071_write(device,offset,data); }

const device_type UPD71071 = &device_creator<upd71071_device>;

upd71071_device::upd71071_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, UPD71071, "NEC uPD71071", tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(upd71071_t));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void upd71071_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void upd71071_device::device_start()
{
	DEVICE_START_NAME( upd71071 )(this);
}


