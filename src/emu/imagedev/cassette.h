/*********************************************************************

    cassette.h

    MESS interface to the cassette image abstraction code

*********************************************************************/

#ifndef CASSETTE_H
#define CASSETTE_H

#include "image.h"
#include "cassimg.h"


enum _cassette_state
{
	/* this part of the state is controlled by the UI */
	CASSETTE_STOPPED			= 0,
	CASSETTE_PLAY				= 1,
	CASSETTE_RECORD				= 2,

	/* this part of the state is controlled by drivers */
	CASSETTE_MOTOR_ENABLED		= 0,
	CASSETTE_MOTOR_DISABLED		= 4,
	CASSETTE_SPEAKER_ENABLED	= 0,
	CASSETTE_SPEAKER_MUTED		= 8,

	/* masks */
	CASSETTE_MASK_UISTATE		= 3,
	CASSETTE_MASK_MOTOR			= 4,
	CASSETTE_MASK_SPEAKER		= 8,
	CASSETTE_MASK_DRVSTATE		= 12
};

typedef enum _cassette_state cassette_state;


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct cassette_config_t	cassette_config;
struct cassette_config_t
{
	const struct CassetteFormat*	const *formats;
	const struct CassetteOptions	*create_opts;
	const cassette_state			default_state;
	const char *					interface;
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

cassette_state cassette_get_state(device_t *cassette);
void cassette_set_state(device_t *cassette, cassette_state state);
void cassette_change_state(device_t *cassette, cassette_state state, cassette_state mask);

double cassette_input(device_t *cassette);
void cassette_output(device_t *cassette, double value);

cassette_image *cassette_get_image(device_t *cassette);
double cassette_get_position(device_t *cassette);
double cassette_get_length(device_t *cassette);
void cassette_seek(device_t *cassette, double time, int origin);

DECLARE_LEGACY_IMAGE_DEVICE(CASSETTE, cassette);

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_CASSETTE_ADD(_tag, _config)	\
	MCFG_DEVICE_ADD(_tag, CASSETTE, 0)			\
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_CASSETTE_MODIFY(_tag, _config)	\
	MCFG_DEVICE_MODIFY(_tag)		\
	MCFG_DEVICE_CONFIG(_config)

extern const cassette_config default_cassette_config;

#endif /* CASSETTE_H */
