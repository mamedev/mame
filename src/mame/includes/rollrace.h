class rollrace_state : public driver_device
{
public:
	rollrace_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
	UINT8 *colorram;
	int ra_charbank[2];
	int ra_bkgpage;
	int ra_bkgflip;
	int ra_chrbank;
	int ra_bkgpen;
	int ra_bkgcol;
	int ra_flipy;
	int ra_flipx;
	int ra_spritebank;
};


/*----------- defined in video/rollrace.c -----------*/

PALETTE_INIT( rollrace );
SCREEN_UPDATE( rollrace );

WRITE8_HANDLER( rollrace_charbank_w );
WRITE8_HANDLER( rollrace_backgroundpage_w );
WRITE8_HANDLER( rollrace_backgroundcolor_w );
WRITE8_HANDLER( rollrace_bkgpen_w );
WRITE8_HANDLER( rollrace_flipy_w );
WRITE8_HANDLER( rollrace_spritebank_w );
WRITE8_HANDLER( rollrace_flipx_w );

