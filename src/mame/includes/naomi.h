/*

naomi.h -> NAOMI includes

*/

enum {
	JVSBD_DEFAULT = 0,
	JVSBD_ADSTICK,
	JVSBD_LIGHTGUN,
	JVSBD_MAHJONG,
	JVSBD_KEYBOARD
};

/*----------- defined in machine/naomi.c -----------*/

extern DRIVER_INIT( naomi );
extern DRIVER_INIT( naomi_mp );

extern DRIVER_INIT( ggxxsla );
extern DRIVER_INIT( ggxxrl );
extern DRIVER_INIT( ggxx );
extern UINT64 *naomi_ram64;

extern int jvsboard_type;
