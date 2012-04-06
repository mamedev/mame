

#define MCU_INITIAL_SEED	0x81


class chaknpop_state : public driver_device
{
public:
	chaknpop_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *  m_mcu_ram;
	UINT8 *  m_tx_ram;
	UINT8 *  m_spr_ram;
	UINT8 *  m_attr_ram;
	size_t   m_spr_ram_size;

	/* mcu-related */
	UINT8 m_mcu_seed;
	UINT8 m_mcu_select;
	UINT8 m_mcu_result;


	/* video-related */
	tilemap_t  *m_tx_tilemap;
	UINT8    *m_vram1;
	UINT8    *m_vram2;
	UINT8    *m_vram3;
	UINT8    *m_vram4;
	UINT8    m_gfxmode;
	UINT8    m_flip_x;
	UINT8    m_flip_y;
	DECLARE_WRITE8_MEMBER(coinlock_w);
	DECLARE_READ8_MEMBER(chaknpop_mcu_port_a_r);
	DECLARE_READ8_MEMBER(chaknpop_mcu_port_b_r);
	DECLARE_READ8_MEMBER(chaknpop_mcu_port_c_r);
	DECLARE_WRITE8_MEMBER(chaknpop_mcu_port_a_w);
	DECLARE_WRITE8_MEMBER(chaknpop_mcu_port_b_w);
	DECLARE_WRITE8_MEMBER(chaknpop_mcu_port_c_w);
	DECLARE_READ8_MEMBER(chaknpop_gfxmode_r);
	DECLARE_WRITE8_MEMBER(chaknpop_gfxmode_w);
	DECLARE_WRITE8_MEMBER(chaknpop_txram_w);
	DECLARE_WRITE8_MEMBER(chaknpop_attrram_w);
};



/*----------- defined in machine/chaknpop.c -----------*/



/*----------- defined in video/chaknpop.c -----------*/

PALETTE_INIT( chaknpop );
VIDEO_START( chaknpop );
SCREEN_UPDATE_IND16( chaknpop );

