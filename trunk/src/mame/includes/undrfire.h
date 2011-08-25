struct tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int primask;
};

class undrfire_state : public driver_device
{
public:
	undrfire_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 m_coin_word;
	UINT16 m_port_sel;
	int m_frame_counter;
	UINT32 *m_ram;
	UINT32 *m_shared_ram;
	struct tempsprite *m_spritelist;
	UINT16 m_rotate_ctrl[8];
	UINT8 m_dislayer[6];
	UINT32 *m_spriteram;
	size_t m_spriteram_size;
};


/*----------- defined in video/undrfire.c -----------*/

VIDEO_START( undrfire );
SCREEN_UPDATE( undrfire );
SCREEN_UPDATE( cbombers );
