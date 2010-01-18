/***************************************************************************

    Konami IC 033906 (PCI bridge)

***************************************************************************/

#include "emu.h"
#include "k033906.h"
#include "video/voodoo.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _k033906_state k033906_state;
struct _k033906_state
{
	UINT32 *     reg;
	UINT32 *     ram;

	int          reg_set;	// 1 = access reg / 0 = access ram

	running_device *voodoo;
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k033906_state *k033906_get_safe_token( running_device *device )
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == K033906);

	return (k033906_state *)device->token;
}

INLINE const k033906_interface *k033906_get_interface( running_device *device )
{
	assert(device != NULL);
	assert((device->type == K033906));
	return (const k033906_interface *) device->baseconfig().static_config;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

void k033906_set_reg( running_device *device, int state )
{
	k033906_state *k033906 = k033906_get_safe_token(device);
	k033906->reg_set = state;
}

static UINT32 k033906_reg_r( running_device *device, int reg )
{
	k033906_state *k033906 = k033906_get_safe_token(device);

	switch (reg)
	{
		case 0x00:		return 0x0001121a;			// PCI Vendor ID (0x121a = 3dfx), Device ID (0x0001 = Voodoo)
		case 0x02:		return 0x04000000;			// Revision ID
		case 0x04:		return k033906->reg[0x04];		// memBaseAddr
		case 0x0f:		return k033906->reg[0x0f];		// interrupt_line, interrupt_pin, min_gnt, max_lat

		default:
			fatalerror("%s: k033906_reg_r: %08X", cpuexec_describe_context(device->machine), reg);
	}
	return 0;
}

static void k033906_reg_w( running_device *device, int reg, UINT32 data )
{
	k033906_state *k033906 = k033906_get_safe_token(device);

	switch (reg)
	{
		case 0x00:
			break;

		case 0x01:		// command register
			break;

		case 0x04:		// memBaseAddr
		{
			if (data == 0xffffffff)
				k033906->reg[0x04] = 0xff000000;
			else
				k033906->reg[0x04] = data & 0xff000000;
			break;
		}

		case 0x0f:		// interrupt_line, interrupt_pin, min_gnt, max_lat
		{
			k033906->reg[0x0f] = data;
			break;
		}

		case 0x10:		// initEnable
		{
			voodoo_set_init_enable(k033906->voodoo, data);
			break;
		}

		case 0x11:		// busSnoop0
		case 0x12:		// busSnoop1
			break;

		case 0x38:		// ???
			break;

		default:
			fatalerror("%s:K033906_w: %08X, %08X", cpuexec_describe_context(device->machine), data, reg);
	}
}

READ32_DEVICE_HANDLER( k033906_r )
{
	k033906_state *k033906 = k033906_get_safe_token(device);

	if (k033906->reg_set)
		return k033906_reg_r(device, offset);
	else
		return k033906->ram[offset];
}

WRITE32_DEVICE_HANDLER( k033906_w )
{
	k033906_state *k033906 = k033906_get_safe_token(device);

	if (k033906->reg_set)
		k033906_reg_w(device, offset, data);
	else
		k033906->ram[offset] = data;
}


/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( k033906 )
{
	k033906_state *k033906 = k033906_get_safe_token(device);
	const k033906_interface *intf = k033906_get_interface(device);

	k033906->voodoo = device->machine->device(intf->voodoo);

	k033906->reg = auto_alloc_array(device->machine, UINT32, 256);
	k033906->ram = auto_alloc_array(device->machine, UINT32, 32768);

	k033906->reg_set = 0;

	state_save_register_device_item_pointer(device, 0, k033906->reg, 256);
	state_save_register_device_item_pointer(device, 0, k033906->ram, 32768);
	state_save_register_device_item(device, 0, k033906->reg_set);
}

/*-------------------------------------------------
    device definition
-------------------------------------------------*/

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID( p, s )	p##k033906##s
#define DEVTEMPLATE_FEATURES	      DT_HAS_START
#define DEVTEMPLATE_NAME		"Konami 033906"
#define DEVTEMPLATE_FAMILY		"Konami PCI Bridge 033906"
#define DEVTEMPLATE_CLASS		DEVICE_CLASS_PERIPHERAL
#include "devtempl.h"
