/*************************************************************************

    3dfx Voodoo Graphics SST-1 emulator

    driver by Aaron Giles

**************************************************************************/


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* enumeration specifying which model of Voodoo we are emulating */
enum
{
	VOODOO_1,
	VOODOO_2,
	VOODOO_BANSHEE,
	VOODOO_3,
	MAX_VOODOO_TYPES
};


#define STD_VOODOO_1_CLOCK			50000000
#define STD_VOODOO_2_CLOCK			90000000
#define STD_VOODOO_BANSHEE_CLOCK	90000000
#define STD_VOODOO_3_CLOCK			132000000


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*voodoo_vblank_func)(running_device *device, int state);
typedef void (*voodoo_stall_func)(running_device *device, int state);


typedef struct _voodoo_config voodoo_config;
struct _voodoo_config
{
	int					type;
	UINT8				fbmem;
	UINT8				tmumem0;
	UINT8				tmumem1;
	const char *		screen;
	const char *		cputag;
	voodoo_vblank_func	vblank;
	voodoo_stall_func	stall;
};



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_3DFX_VOODOO_ADD(_tag, _type, _clock, _fbmem, _screen) \
	MDRV_DEVICE_ADD(_tag, VOODOO_GRAPHICS, _clock) \
	MDRV_DEVICE_CONFIG_DATA32(voodoo_config, type, _type) \
	MDRV_DEVICE_CONFIG_DATA32(voodoo_config, fbmem, _fbmem) \
	MDRV_DEVICE_CONFIG_DATAPTR(voodoo_config, screen, _screen)

#define MDRV_3DFX_VOODOO_1_ADD(_tag, _clock, _fbmem, _screen) \
	MDRV_3DFX_VOODOO_ADD(_tag, VOODOO_1, _clock, _fbmem, _screen)

#define MDRV_3DFX_VOODOO_2_ADD(_tag, _clock, _fbmem, _screen) \
	MDRV_3DFX_VOODOO_ADD(_tag, VOODOO_2, _clock, _fbmem, _screen)

#define MDRV_3DFX_VOODOO_BANSHEE_ADD(_tag, _clock, _fbmem, _screen) \
	MDRV_3DFX_VOODOO_ADD(_tag, VOODOO_BANSHEE, _clock, _fbmem, _screen)

#define MDRV_3DFX_VOODOO_3_ADD(_tag, _clock, _fbmem, _screen) \
	MDRV_3DFX_VOODOO_ADD(_tag, VOODOO_3, _clock, _fbmem, _screen)

#define MDRV_3DFX_VOODOO_TMU_MEMORY(_tmu, _tmumem) \
	MDRV_DEVICE_CONFIG_DATA32(voodoo_config, tmumem##_tmu, _tmumem)

#define MDRV_3DFX_VOODOO_VBLANK(_vblank) \
	MDRV_DEVICE_CONFIG_DATAPTR(voodoo_config, vblank, _vblank)

#define MDRV_3DFX_VOODOO_STALL(_stall) \
	MDRV_DEVICE_CONFIG_DATAPTR(voodoo_config, stall, _stall)

#define MDRV_3DFX_VOODOO_CPU(_cputag) \
	MDRV_DEVICE_CONFIG_DATAPTR(voodoo_config, cputag, _cputag)

#define MDRV_3DFX_VOODOO_MODIFY(_tag) \
	MDRV_DEVICE_MODIFY(_tag)



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

int voodoo_update(running_device *device, bitmap_t *bitmap, const rectangle *cliprect);
int voodoo_get_type(running_device *device);
int voodoo_is_stalled(running_device *device);
void voodoo_set_init_enable(running_device *device, UINT32 newval);

READ32_DEVICE_HANDLER( voodoo_r );
WRITE32_DEVICE_HANDLER( voodoo_w );

READ32_DEVICE_HANDLER( banshee_r );
WRITE32_DEVICE_HANDLER( banshee_w );
READ32_DEVICE_HANDLER( banshee_fb_r );
WRITE32_DEVICE_HANDLER( banshee_fb_w );
READ32_DEVICE_HANDLER( banshee_io_r );
WRITE32_DEVICE_HANDLER( banshee_io_w );
READ32_DEVICE_HANDLER( banshee_rom_r );


/* ----- device interface ----- */

/* device get info callback */
#define VOODOO_GRAPHICS DEVICE_GET_INFO_NAME(voodoo)
DEVICE_GET_INFO( voodoo );
