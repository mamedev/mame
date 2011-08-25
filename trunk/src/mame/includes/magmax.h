class magmax_state : public driver_device
{
public:
	magmax_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_videoram;
	UINT8 m_sound_latch;
	UINT8 m_LS74_clr;
	UINT8 m_LS74_q;
	UINT8 m_gain_control;
	emu_timer *m_interrupt_timer;
	UINT16 *m_vreg;
	UINT16 *m_scroll_x;
	UINT16 *m_scroll_y;
	int m_flipscreen;
	UINT32 *m_prom_tab;
	UINT16 *m_spriteram;
	size_t m_spriteram_size;
};


/*----------- defined in video/magmax.c -----------*/

PALETTE_INIT( magmax );
SCREEN_UPDATE( magmax );
VIDEO_START( magmax );
