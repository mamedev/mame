#ifndef _VIDEO_TIA_H_
#define _VIDEO_TIA_H_

#define TIA_PALETTE_LENGTH				128 + 128 * 128
#define TIA_INPUT_PORT_ALWAYS_ON		0
#define TIA_INPUT_PORT_ALWAYS_OFF		0xffff
#define TIA_MAX_SCREEN_HEIGHT			342

struct tia_interface {
	read16_handler	read_input_port;
	read8_handler	databus_contents;
	write16_handler	vsync_callback;
};

PALETTE_INIT( tia_NTSC );
PALETTE_INIT( tia_PAL );

VIDEO_START( tia );
VIDEO_UPDATE( tia );

READ8_HANDLER( tia_r );
WRITE8_HANDLER( tia_w );

void tia_init(const struct tia_interface* ti);

#endif /* _VIDEO_TIA_H_ */
