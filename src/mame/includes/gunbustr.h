struct tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int primask;
};

class gunbustr_state : public driver_device
{
public:
	gunbustr_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 m_coin_word;
	UINT32 *m_ram;
	struct tempsprite *m_spritelist;
	UINT32 m_mem[2];
	UINT32 *m_spriteram;
	size_t m_spriteram_size;
};


/*----------- defined in video/gunbustr.c -----------*/

VIDEO_START( gunbustr );
SCREEN_UPDATE( gunbustr );
