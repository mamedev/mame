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

/*----------- defined in machine/gdcrypt.c -----------*/

extern void naomi_game_decrypt(running_machine& machine, UINT64 key, UINT8* region, int length);


/*----------- defined in machine/naomi.c -----------*/







extern int jvsboard_type;
extern UINT16 actel_id;

void naomi_g1_irq(running_machine &machine);
