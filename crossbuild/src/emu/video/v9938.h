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

void v9938_init (running_machine *, int, int, void (*callback)(int));
void v9938_reset (void);
int v9938_interrupt (void);
void v9938_set_sprite_limit (int);
void v9938_set_resolution (int);

extern PALETTE_INIT( v9938 );
extern PALETTE_INIT( v9958 );
extern WRITE8_HANDLER( v9938_palette_w );
extern WRITE8_HANDLER( v9938_vram_w );
extern  READ8_HANDLER( v9938_vram_r );
extern WRITE8_HANDLER( v9938_command_w );
extern  READ8_HANDLER( v9938_status_r );
extern WRITE8_HANDLER( v9938_register_w );

void v9938_update_mouse_state(int mx_delta, int my_delta, int button_state);
