/*************************************************************************

    Malzak

*************************************************************************/


typedef struct _malzak_state malzak_state;
struct _malzak_state
{
	/* misc */
//  int playfield_x[256];
//  int playfield_y[256];
	int playfield_code[256];
	int malzak_x;
	int malzak_y;
	int collision_counter;

	/* devices */
	const device_config *s2636_0;
	const device_config *s2636_1;
	const device_config *saa5050;
};


/*----------- defined in video/malzak.c -----------*/

WRITE8_HANDLER( malzak_playfield_w );

VIDEO_UPDATE( malzak );
