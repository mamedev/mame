/*************************************************************************

    Naomi plug-in board

    driver by Samuele Zannoli

**************************************************************************/

#include "devlegcy.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

/* enumeration specifying which model of board we are emulating */
enum
{
	ROM_BOARD,
	DIMM_BOARD,
	AW_ROM_BOARD,
	MAX_NAOMIBD_TYPES
};


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*naomibd_interrupt_func)(device_t *device, int state);

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

#define MCFG_NAOMIBD_ADD(_tag, _type) \
	MCFG_DEVICE_ADD(_tag, NAOMI_BOARD, 0) \
	MCFG_DEVICE_CONFIG_DATA32(naomibd_config, type, _type)

#define MCFG_NAOMI_ROM_BOARD_ADD(_tag, _region) \
	MCFG_NAOMIBD_ADD(_tag, ROM_BOARD) \
	MCFG_NAOMIBD_REGION(_region)

#define MCFG_AW_ROM_BOARD_ADD(_tag, _region) \
	MCFG_NAOMIBD_ADD(_tag, AW_ROM_BOARD) \
	MCFG_NAOMIBD_REGION(_region)

#define MCFG_NAOMI_DIMM_BOARD_ADD(_tag, _gdrom, _region, _pic) \
	MCFG_NAOMIBD_ADD(_tag, DIMM_BOARD) \
	MCFG_NAOMIBD_REGION(_region) \
	MCFG_NAOMIBD_GDROM_REGION(_gdrom) \
	MCFG_NAOMIBD_PIC_REGION(_pic)

#define MCFG_NAOMIBD_REGION(_region) \
	MCFG_DEVICE_CONFIG_DATAPTR(naomibd_config, regiontag, _region)

#define MCFG_NAOMIBD_GDROM_REGION(_region) \
	MCFG_DEVICE_CONFIG_DATAPTR(naomibd_config, gdromregiontag, _region)

#define MCFG_NAOMIBD_PIC_REGION(_region) \
	MCFG_DEVICE_CONFIG_DATAPTR(naomibd_config, picregiontag, _region)

#define MCFG_NAOMIBD_MODIFY(_tag) \
	MCFG_DEVICE_MODIFY(_tag)

/*#define MCFG_NAOMIBD_TMU_MEMORY(_tmu, _tmumem) \
    MCFG_DEVICE_CONFIG_DATA32(naomibd_config, tmumem##_tmu, _tmumem)*/


/*----------- defined in machine/naomibd.c -----------*/

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

int naomibd_interrupt_callback(device_t *device, naomibd_interrupt_func callback);
int naomibd_get_type(device_t *device);
void *naomibd_get_memory(device_t *device);
offs_t naomibd_get_dmaoffset(device_t *device);

READ64_DEVICE_HANDLER( naomibd_r );
WRITE64_DEVICE_HANDLER( naomibd_w );

/* ----- device interface ----- */

DECLARE_LEGACY_NVRAM_DEVICE(NAOMI_BOARD, naomibd);
