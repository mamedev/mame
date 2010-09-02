/*----------- defined in drivers/wecleman.c -----------*/

extern int wecleman_selected_ip, wecleman_irqctrl;


/*----------- defined in video/wecleman.c -----------*/

extern UINT16 *wecleman_videostatus;
extern UINT16 *wecleman_pageram, *wecleman_txtram, *wecleman_roadram;
extern size_t wecleman_roadram_size;

WRITE16_HANDLER( hotchase_paletteram16_SBGRBBBBGGGGRRRR_word_w );
WRITE16_HANDLER( wecleman_paletteram16_SSSSBBBBGGGGRRRR_word_w );
WRITE16_HANDLER( wecleman_videostatus_w );
WRITE16_HANDLER( wecleman_pageram_w );
WRITE16_HANDLER( wecleman_txtram_w );
VIDEO_UPDATE( wecleman );
VIDEO_START( wecleman );
VIDEO_UPDATE( hotchase );
VIDEO_START( hotchase );

void hotchase_zoom_callback_0(running_machine *machine, int *code,int *color,int *flags);
void hotchase_zoom_callback_1(running_machine *machine, int *code,int *color,int *flags);
