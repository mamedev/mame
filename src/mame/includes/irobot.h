/*************************************************************************

    Atari I, Robot hardware

*************************************************************************/

#define IR_TIMING				1		/* try to emulate MB and VG running time */

typedef struct irmb_ops
{
	const struct irmb_ops *nxtop;
	UINT32 func;
	UINT32 diradd;
	UINT32 latchmask;
	UINT32 *areg;
	UINT32 *breg;
	UINT8 cycles;
	UINT8 diren;
	UINT8 flags;
	UINT8 ramsel;
} irmb_ops;


class irobot_state : public driver_device
{
public:
	irobot_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  m_nvram(*this, "nvram") { }

	required_shared_ptr<UINT8>	m_nvram;
	UINT8 *videoram;
	UINT8 vg_clear;
	UINT8 bufsel;
	UINT8 alphamap;
	UINT8 *combase;
	UINT8 irvg_vblank;
	UINT8 irvg_running;
	UINT8 irmb_running;
#if IR_TIMING
	timer_device *irvg_timer;
	timer_device *irmb_timer;
#endif
	UINT8 *comRAM[2];
	UINT8 *mbRAM;
	UINT8 *mbROM;
	UINT8 control_num;
	UINT8 statwr;
	UINT8 out0;
	UINT8 outx;
	UINT8 mpage;
	UINT8 *combase_mb;
	irmb_ops *mbops;
	const irmb_ops *irmb_stack[16];
	UINT32 irmb_regs[16];
	UINT32 irmb_latch;
	UINT8 *polybitmap1;
	UINT8 *polybitmap2;
	int ir_xmin;
	int ir_ymin;
	int ir_xmax;
	int ir_ymax;
};

/*----------- defined in machine/irobot.c -----------*/

DRIVER_INIT( irobot );
MACHINE_RESET( irobot );

TIMER_DEVICE_CALLBACK( irobot_irvg_done_callback );
TIMER_DEVICE_CALLBACK( irobot_irmb_done_callback );

READ8_HANDLER( irobot_status_r );
WRITE8_HANDLER( irobot_statwr_w );
WRITE8_HANDLER( irobot_out0_w );
WRITE8_HANDLER( irobot_rom_banksel_w );
READ8_HANDLER( irobot_control_r );
WRITE8_HANDLER( irobot_control_w );
READ8_HANDLER( irobot_sharedmem_r );
WRITE8_HANDLER( irobot_sharedmem_w );


/*----------- defined in video/irobot.c -----------*/

PALETTE_INIT( irobot );
VIDEO_START( irobot );
SCREEN_UPDATE( irobot );

WRITE8_HANDLER( irobot_paletteram_w );

void irobot_poly_clear(running_machine *machine);
void irobot_run_video(running_machine *machine);
