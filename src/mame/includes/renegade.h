#define MCU_BUFFER_MAX 6

class renegade_state : public driver_device
{
public:
	renegade_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_videoram2(*this, "videoram2"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this,"maincpu"){ }

	UINT8 m_bank;
	int m_mcu_sim;
	int m_from_main;
	int m_from_mcu;
	int m_main_sent;
	int m_mcu_sent;
	UINT8 m_ddr_a;
	UINT8 m_ddr_b;
	UINT8 m_ddr_c;
	UINT8 m_port_a_out;
	UINT8 m_port_b_out;
	UINT8 m_port_c_out;
	UINT8 m_port_a_in;
	UINT8 m_port_b_in;
	UINT8 m_port_c_in;
	UINT8 m_mcu_buffer[MCU_BUFFER_MAX];
	UINT8 m_mcu_input_size;
	UINT8 m_mcu_output_byte;
	INT8 m_mcu_key;
	int m_mcu_checksum;
	const UINT8 *m_mcu_encrypt_table;
	int m_mcu_encrypt_table_len;
	int m_coin;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_videoram2;
	INT32 m_scrollx;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	required_shared_ptr<UINT8> m_spriteram;

	required_device<cpu_device> m_maincpu;
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_READ8_MEMBER(mcu_reset_r);
	DECLARE_WRITE8_MEMBER(mcu_w);
	DECLARE_READ8_MEMBER(mcu_r);
	DECLARE_WRITE8_MEMBER(bankswitch_w);
	DECLARE_WRITE8_MEMBER(renegade_coin_counter_w);
	void mcu_process_command();
	DECLARE_READ8_MEMBER(renegade_68705_port_a_r);
	DECLARE_WRITE8_MEMBER(renegade_68705_port_a_w);
	DECLARE_WRITE8_MEMBER(renegade_68705_ddr_a_w);
	DECLARE_READ8_MEMBER(renegade_68705_port_b_r);
	DECLARE_WRITE8_MEMBER(renegade_68705_port_b_w);
	DECLARE_WRITE8_MEMBER(renegade_68705_ddr_b_w);
	DECLARE_READ8_MEMBER(renegade_68705_port_c_r);
	DECLARE_WRITE8_MEMBER(renegade_68705_port_c_w);
	DECLARE_WRITE8_MEMBER(renegade_68705_ddr_c_w);
	DECLARE_WRITE8_MEMBER(renegade_videoram_w);
	DECLARE_WRITE8_MEMBER(renegade_videoram2_w);
	DECLARE_WRITE8_MEMBER(renegade_flipscreen_w);
	DECLARE_WRITE8_MEMBER(renegade_scroll0_w);
	DECLARE_WRITE8_MEMBER(renegade_scroll1_w);
	DECLARE_CUSTOM_INPUT_MEMBER(mcu_status_r);
	DECLARE_WRITE8_MEMBER(adpcm_play_w);
	DECLARE_DRIVER_INIT(kuniokun);
	DECLARE_DRIVER_INIT(kuniokunb);
	DECLARE_DRIVER_INIT(renegade);
	TILE_GET_INFO_MEMBER(get_bg_tilemap_info);
	TILE_GET_INFO_MEMBER(get_fg_tilemap_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_renegade(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
