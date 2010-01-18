/***************************************************************************

    v9938 / v9958 emulation

***************************************************************************/

/* init functions */

#define MODEL_V9938	(0)
#define MODEL_V9958	(1)

/* resolutions */
#define RENDER_HIGH	(0)
#define RENDER_LOW	(1)
#define RENDER_AUTO	(2)

void v9938_init (running_machine *machine, int which, running_device *screen, bitmap_t *bitmap, int model, int vram_size, void (*callback)(running_machine *, int) );
void v9938_reset (int which);
int v9938_interrupt (running_machine *machine, int which);
void v9938_set_sprite_limit (int which, int);
void v9938_set_resolution (int which, int);
int v9938_get_transpen(int which);

extern PALETTE_INIT( v9938 );
extern PALETTE_INIT( v9958 );
extern WRITE8_HANDLER( v9938_0_palette_w );
extern WRITE8_HANDLER( v9938_1_palette_w );
extern WRITE8_HANDLER( v9938_0_vram_w );
extern WRITE8_HANDLER( v9938_1_vram_w );
extern  READ8_HANDLER( v9938_0_vram_r );
extern  READ8_HANDLER( v9938_1_vram_r );
extern WRITE8_HANDLER( v9938_0_command_w );
extern WRITE8_HANDLER( v9938_1_command_w );
extern  READ8_HANDLER( v9938_0_status_r );
extern  READ8_HANDLER( v9938_1_status_r );
extern WRITE8_HANDLER( v9938_0_register_w );
extern WRITE8_HANDLER( v9938_1_register_w );

void v9938_update_mouse_state(int which, int mx_delta, int my_delta, int button_state);
