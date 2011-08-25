/***************************************************************************

    tlc34076.h

    Basic implementation of the TLC34076 palette chip and similar
    compatible chips.

***************************************************************************/

#define TLC34076_6_BIT		0
#define TLC34076_8_BIT		1

const pen_t *tlc34076_get_pens(device_t *device);


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _tlc34076_config tlc34076_config;
struct _tlc34076_config
{
	int res_sel;
};

DECLARE_LEGACY_DEVICE(TLC34076, tlc34076);


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_TLC34076_ADD(_tag, _res_sel) \
	MCFG_DEVICE_ADD(_tag, TLC34076, 0) \
	MCFG_DEVICE_CONFIG_DATA32(tlc34076_config, res_sel, _res_sel)


/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

WRITE8_DEVICE_HANDLER( tlc34076_w );
READ8_DEVICE_HANDLER( tlc34076_r );
