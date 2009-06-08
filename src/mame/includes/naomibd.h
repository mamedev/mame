/*************************************************************************

    Naomi plug-in board

    driver by Samuele Zannoli

**************************************************************************/


/***************************************************************************
    CONSTANTS
***************************************************************************/

/* enumeration specifying which model of board we are emulating */
enum
{
	ROM_BOARD,
	DIMM_BOARD,
	MAX_NAOMIBD_TYPES
};

enum
{
	DEVINFO_INT_DMAOFFSET = DEVINFO_INT_DEVICE_SPECIFIC+0,	/* R/O: value of dma_offset register */
	DEVINFO_PTR_MEMORY = DEVINFO_PTR_DEVICE_SPECIFIC+0	/* R/O: pointer to memory inside the board containing game data */
};


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*naomibd_interrupt_func)(const device_config *device, int state);

typedef struct _naomibd_config naomibd_config;
struct _naomibd_config
{
	int						type;
	const char *			regiontag;
	const char *			gdromregiontag;
	const char *			picregiontag;
	naomibd_interrupt_func	interrupt;
};



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MDRV_NAOMIBD_ADD(_tag, _type) \
	MDRV_DEVICE_ADD(_tag, NAOMI_BOARD, 0) \
	MDRV_DEVICE_CONFIG_DATA32(naomibd_config, type, _type)

#define MDRV_NAOMI_ROM_BOARD_ADD(_tag, _region) \
	MDRV_NAOMIBD_ADD(_tag, ROM_BOARD) \
	MDRV_NAOMIBD_REGION(_region)

#define MDRV_NAOMI_DIMM_BOARD_ADD(_tag, _gdrom, _region, _pic) \
	MDRV_NAOMIBD_ADD(_tag, DIMM_BOARD) \
	MDRV_NAOMIBD_REGION(_region) \
	MDRV_NAOMIBD_GDROM_REGION(_gdrom) \
	MDRV_NAOMIBD_PIC_REGION(_pic)

#define MDRV_NAOMIBD_REGION(_region) \
	MDRV_DEVICE_CONFIG_DATAPTR(naomibd_config, regiontag, _region)

#define MDRV_NAOMIBD_GDROM_REGION(_region) \
	MDRV_DEVICE_CONFIG_DATAPTR(naomibd_config, gdromregiontag, _region)

#define MDRV_NAOMIBD_PIC_REGION(_region) \
	MDRV_DEVICE_CONFIG_DATAPTR(naomibd_config, picregiontag, _region)

#define MDRV_NAOMIBD_MODIFY(_tag) \
	MDRV_DEVICE_MODIFY(_tag)

/*#define MDRV_NAOMIBD_TMU_MEMORY(_tmu, _tmumem) \
    MDRV_DEVICE_CONFIG_DATA32(naomibd_config, tmumem##_tmu, _tmumem)*/


/*----------- defined in machine/naomibd.c -----------*/

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

int naomibd_interrupt_callback(const device_config *device, naomibd_interrupt_func callback);
int naomibd_get_type(const device_config *device);

READ64_DEVICE_HANDLER( naomibd_r );
WRITE64_DEVICE_HANDLER( naomibd_w );


/* ----- device interface ----- */

/* device get info callback */
#define NAOMI_BOARD DEVICE_GET_INFO_NAME(naomibd)
DEVICE_GET_INFO( naomibd );
