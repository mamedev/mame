

#define MCU_INITIAL_SEED	0x81


class chaknpop_state : public driver_device
{
public:
	chaknpop_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_mcu_ram(*this, "mcu_ram"),
		m_tx_ram(*this, "tx_ram"),
		m_attr_ram(*this, "attr_ram"),
		m_spr_ram(*this, "spr_ram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_mcu_ram;
	required_shared_ptr<UINT8> m_tx_ram;
	required_shared_ptr<UINT8> m_attr_ram;
	required_shared_ptr<UINT8> m_spr_ram;

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
	DECLARE_WRITE8_MEMBER(unknown_port_1_w);
	DECLARE_WRITE8_MEMBER(unknown_port_2_w);
	TILE_GET_INFO_MEMBER(chaknpop_get_tx_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_chaknpop(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
