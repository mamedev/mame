// license:BSD-3-Clause
// copyright-holders:Brad Oliver,Stephane Humbert
/* This it the best way to allow game specific kludges until the system is fully understood */
enum {
	ARKUNK = 0,  /* unknown bootlegs for inclusion of possible new sets */
	ARKANGC,
	ARKANGC2,
	BLOCK2,
	ARKBLOCK,
	ARKBLOC2,
	ARKGCBL,
	PADDLE2
};

class arkanoid_state : public driver_device
{
public:
	enum
	{
		TIMER_68705_PRESCALER_EXPIRED,
	};

	arkanoid_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this,"videoram"),
		m_spriteram(*this,"spriteram"),
		m_protram(*this,"protram"),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_protram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	uint8_t    m_gfxbank;
	uint8_t    m_palettebank;

	/* input-related */
	uint8_t    m_paddle_select;   // selected by d008 bit 2

	/* bootleg related */
	int      m_bootleg_id;
	uint8_t    m_bootleg_cmd;

	/* mcu interface related */
	bool     m_Z80HasWritten;   // z80 has written to latch flag
	uint8_t    m_fromZ80;         // byte latch for z80->68705 comms
	bool     m_MCUHasWritten;   // 68705 has written to latch flag
	uint8_t    m_fromMCU;         // byte latch for 68705->z80 comms

	/* mcu internal related */
	uint8_t    m_portA_in;
	uint8_t    m_portA_out;
	uint8_t    m_ddrA;
	uint8_t    m_portC_internal;
	uint8_t    m_portC_out;
	uint8_t    m_ddrC;
	uint8_t    m_tdr;
	uint8_t    m_tcr;
	emu_timer *m_68705_timer;

	/* hexaa */
	uint8_t m_hexaa_from_main;
	uint8_t m_hexaa_from_sub;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	uint8_t arkanoid_Z80_mcu_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void arkanoid_Z80_mcu_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t arkanoid_68705_port_a_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void arkanoid_68705_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void arkanoid_68705_ddr_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t arkanoid_68705_port_c_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void arkanoid_68705_port_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void arkanoid_68705_ddr_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t arkanoid_68705_tdr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void arkanoid_68705_tdr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t arkanoid_68705_tcr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void arkanoid_68705_tcr_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t arkanoid_bootleg_f000_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t arkanoid_bootleg_f002_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void arkanoid_bootleg_d018_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t arkanoid_bootleg_d008_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void arkanoid_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void arkanoid_d008_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void tetrsark_d008_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void brixian_d008_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hexa_d008_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t hexaa_f000_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void hexaa_f000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hexaa_sub_80_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t hexaa_sub_90_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	ioport_value arkanoid_semaphore_input_r(ioport_field &field, void *param);
	ioport_value arkanoid_input_mux(ioport_field &field, void *param);
	void init_block2();
	void init_arkblock();
	void init_hexa();
	void init_hexaa();
	void init_paddle2();
	void init_tetrsark();
	void init_arkgcbl();
	void init_arkangc2();
	void init_arkbloc2();
	void init_arkangc();
	void init_brixian();
	void get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_arkanoid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_hexa(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void timer_68705_increment(void *ptr, int32_t param);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	void arkanoid_bootleg_init(  );

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
