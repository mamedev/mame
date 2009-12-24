/***************************************************************************

    Konami IC 056230 (LANC)

***************************************************************************/

#include "driver.h"
#include "k056230.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _k056230_state k056230_state;
struct _k056230_state
{
	UINT32  *    ram;
	int          is_thunderh;

	const device_config *cpu;
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE k056230_state *k056230_get_safe_token( const device_config *device )
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == K056230);

	return (k056230_state *)device->token;
}

INLINE const k056230_interface *k056230_get_interface( const device_config *device )
{
	assert(device != NULL);
	assert((device->type == K056230));
	return (const k056230_interface *) device->static_config;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

READ8_DEVICE_HANDLER( k056230_r )
{
	switch (offset)
	{
		case 0:		// Status register
		{
			return 0x08;
		}
	}

//  mame_printf_debug("k056230_r: %d at %08X\n", offset, cpu_get_pc(space->cpu));

	return 0;
}

static TIMER_CALLBACK( network_irq_clear )
{
	k056230_state *k056230 = (k056230_state *)ptr;
	cpu_set_input_line(k056230->cpu, INPUT_LINE_IRQ2, CLEAR_LINE);
}

WRITE8_DEVICE_HANDLER( k056230_w )
{
	k056230_state *k056230 = k056230_get_safe_token(device);

	switch (offset)
	{
		case 0:		// Mode register
		{
			break;
		}
		case 1:		// Control register
		{
			if (data & 0x20)
			{
				// Thunder Hurricane breaks otherwise...
				if (!k056230->is_thunderh)
				{
					cpu_set_input_line(k056230->cpu, INPUT_LINE_IRQ2, ASSERT_LINE);
					timer_set(device->machine, ATTOTIME_IN_USEC(10), k056230, 0, network_irq_clear);
				}
			}
//          else
//              cpu_set_input_line(k056230->cpu, INPUT_LINE_IRQ2, CLEAR_LINE);
			break;
		}
		case 2:		// Sub ID register
		{
			break;
		}
	}
//  mame_printf_debug("k056230_w: %d, %02X at %08X\n", offset, data, cpu_get_pc(space->cpu));
}

READ32_DEVICE_HANDLER( lanc_ram_r )
{
	k056230_state *k056230 = k056230_get_safe_token(device);

	//mame_printf_debug("LANC_RAM_r: %08X, %08X at %08X\n", offset, mem_mask, cpu_get_pc(space->cpu));
	return k056230->ram[offset & 0x7ff];
}

WRITE32_DEVICE_HANDLER( lanc_ram_w )
{
	k056230_state *k056230 = k056230_get_safe_token(device);

	//mame_printf_debug("LANC_RAM_w: %08X, %08X, %08X at %08X\n", data, offset, mem_mask, cpu_get_pc(space->cpu));
	COMBINE_DATA(k056230->ram + (offset & 0x7ff));
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( k056230 )
{
	k056230_state *k056230 = k056230_get_safe_token(device);
	const k056230_interface *intf = k056230_get_interface(device);

	k056230->cpu = devtag_get_device(device->machine, intf->cpu);
	k056230->is_thunderh = intf->is_thunderh;

	k056230->ram = auto_alloc_array(device->machine, UINT32, 0x2000);

	state_save_register_device_item_pointer(device, 0, k056230->ram, 0x2000);
}

/*-------------------------------------------------
    device definition
-------------------------------------------------*/

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID( p, s )	p##k056230##s
#define DEVTEMPLATE_FEATURES	      DT_HAS_START
#define DEVTEMPLATE_NAME		"Konami 056230"
#define DEVTEMPLATE_FAMILY		"Konami Network Board 056230"
#define DEVTEMPLATE_CLASS		DEVICE_CLASS_PERIPHERAL
#include "devtempl.h"
