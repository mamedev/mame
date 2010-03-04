/*************************************************************************

    DJ Boy

*************************************************************************/

#define PROT_OUTPUT_BUFFER_SIZE 8

class djboy_state
{
public:
	static void *alloc(running_machine &machine) { return auto_alloc_clear(&machine, djboy_state(machine)); }

	djboy_state(running_machine &machine) { }

	/* memory pointers */
	UINT8 *    videoram;
	UINT8 *    paletteram;

	/* video-related */
	tilemap_t    *background;
	UINT8      videoreg, scrollx, scrolly;

	/* Kaneko BEAST state */
	int        prot_busy_count;
	UINT8      prot_output_buffer[PROT_OUTPUT_BUFFER_SIZE];
	int        prot_available_data_count;
	int        prot_offs;		/* internal state */
	UINT8      prot_ram[0x80];	/* internal RAM */
	UINT8      prot_param[8];
	int        coin;
	int        complete;
	int        lives[2];
	int        addr;
	int        bankxor;
	int        mDjBoyState;
	int        prot_mode;

	/* devices */
	running_device *maincpu;
	running_device *cpu1;
	running_device *cpu2;
	running_device *pandora;
};


// mDjBoyState
enum
{
	eDJBOY_ATTRACT_HIGHSCORE = 0,
	eDJBOY_ATTRACT_TITLE,
	eDJBOY_ATTRACT_GAMEPLAY,
	eDJBOY_PRESS_P1_START,
	eDJBOY_PRESS_P1_OR_P2_START,
	eDJBOY_ACTIVE_GAMEPLAY
};

// prot_mode
enum
{
	ePROT_NORMAL = 0,
	ePROT_WRITE_BYTES,
	ePROT_WRITE_BYTE,
	ePROT_READ_BYTES,
	ePROT_WAIT_DSW1_WRITEBACK,
	ePROT_WAIT_DSW2_WRITEBACK,
	ePROT_STORE_PARAM
};

/*----------- defined in video/djboy.c -----------*/

WRITE8_HANDLER( djboy_scrollx_w );
WRITE8_HANDLER( djboy_scrolly_w );
WRITE8_HANDLER( djboy_videoram_w );
WRITE8_HANDLER( djboy_paletteram_w );

VIDEO_START( djboy );
VIDEO_UPDATE( djboy );
VIDEO_EOF( djboy );
