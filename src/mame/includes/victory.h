/***************************************************************************

    Victory system

****************************************************************************/


#define VICTORY_MAIN_CPU_CLOCK		(XTAL_8MHz / 2)

#define VICTORY_PIXEL_CLOCK				(XTAL_11_289MHz / 2)
#define VICTORY_HTOTAL					(0x150)
#define VICTORY_HBEND						(0x000)
#define VICTORY_HBSTART					(0x100)
#define VICTORY_VTOTAL					(0x118)
#define VICTORY_VBEND						(0x000)
#define VICTORY_VBSTART					(0x100)


/* microcode state */
struct micro_t
{
	UINT16		i;
	UINT16		pc;
	UINT8		r,g,b;
	UINT8		x,xp,y,yp;
	UINT8		cmd,cmdlo;
	emu_timer *	timer;
	UINT8		timer_active;
	attotime	endtime;
};

class victory_state : public driver_device
{
public:
	victory_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	UINT8 *charram;
	UINT16 paletteram[0x40];
	UINT8 *bgbitmap;
	UINT8 *fgbitmap;
	UINT8 *rram;
	UINT8 *gram;
	UINT8 *bram;
	UINT8 vblank_irq;
	UINT8 fgcoll;
	UINT8 fgcollx;
	UINT8 fgcolly;
	UINT8 bgcoll;
	UINT8 bgcollx;
	UINT8 bgcolly;
	UINT8 scrollx;
	UINT8 scrolly;
	UINT8 video_control;
	struct micro_t micro;
};


/*----------- defined in video/victory.c -----------*/

VIDEO_START( victory );
SCREEN_UPDATE( victory );
INTERRUPT_GEN( victory_vblank_interrupt );

READ8_HANDLER( victory_video_control_r );
WRITE8_HANDLER( victory_video_control_w );
WRITE8_HANDLER( victory_paletteram_w );
