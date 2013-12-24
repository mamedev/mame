
struct collision_info
{
		collision_info():
		x(0),
		y(0),
		min_x(0),
		min_y(0),
		max_x(0),
		max_y(0),
		hitbox(0),
		hitbox_x(0),
		hitbox_y(0) {}

	int x,y;
	INT16 min_x,min_y,max_x,max_y;
	UINT16 hitbox;
	UINT16 hitbox_x,hitbox_y;
};

class seibu_cop_legacy_device : public device_t
{
public:
seibu_cop_legacy_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ16_MEMBER( copdxbl_0_r );
	DECLARE_WRITE16_MEMBER( copdxbl_0_w );

	DECLARE_READ16_MEMBER( heatbrl_mcu_r );
	DECLARE_WRITE16_MEMBER( heatbrl_mcu_w );
	DECLARE_READ16_MEMBER( cupsoc_mcu_r );
	DECLARE_WRITE16_MEMBER( cupsoc_mcu_w );
	DECLARE_READ16_MEMBER( cupsocs_mcu_r );
	DECLARE_WRITE16_MEMBER( cupsocs_mcu_w );
	DECLARE_READ16_MEMBER( godzilla_mcu_r );
	DECLARE_WRITE16_MEMBER( godzilla_mcu_w );
	DECLARE_READ16_MEMBER( denjinmk_mcu_r );
	DECLARE_WRITE16_MEMBER( denjinmk_mcu_w );
	DECLARE_READ16_MEMBER( grainbow_mcu_r );
	DECLARE_WRITE16_MEMBER( grainbow_mcu_w );
	DECLARE_READ16_MEMBER( legionna_mcu_r );
	DECLARE_WRITE16_MEMBER( legionna_mcu_w );

	//DECLARE_READ16_MEMBER( raiden2_mcu_r );   unused
	//DECLARE_WRITE16_MEMBER( raiden2_mcu_w );  unused

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	UINT16 *m_cop_mcu_ram;
	UINT16 m_copd2_table[0x100];
	UINT16 m_copd2_table_2[0x100/8];
	UINT16 m_copd2_table_3[0x100/8];
	UINT16 m_copd2_table_4[0x100/8];
	UINT16 m_cop_438;
	UINT16 m_cop_43a;
	UINT16 m_cop_43c;
	UINT16 m_cop_dma_src[0x200];
	UINT16 m_cop_dma_size[0x200];
	UINT16 m_cop_dma_dst[0x200];
	UINT16 m_cop_dma_fade_table;
	UINT16 m_cop_dma_trigger;
	UINT16 m_cop_scale;
	UINT8 m_cop_rng_max_value;
	UINT16 m_copd2_offs;
	UINT32 m_cop_register[8];
	UINT16 m_seibu_vregs[0x50/2];
	UINT16 m_cop_status,m_cop_dist,m_cop_angle;
	UINT16 m_cop_hit_status;
	INT16 m_cop_hit_val_x,m_cop_hit_val_y,m_cop_hit_val_z,m_cop_hit_val_unk;
	UINT32 m_cop_sort_lookup,m_cop_sort_ram_addr,m_cop_sort_param;
	INT8 m_cop_angle_compare;
	INT8 m_cop_angle_mod_val;
	struct collision_info m_cop_collision_info[2];
	int m_r0, m_r1;
	UINT16 m_cop_rom_addr_lo,m_cop_rom_addr_hi,m_cop_rom_addr_unk;
	UINT16 m_u1,m_u2;
	UINT32 m_fill_val;
	UINT8 m_pal_brightness_val,m_pal_brightness_mode;
	UINT32 m_cop_sprite_dma_src;
	int m_cop_sprite_dma_abs_x,m_cop_sprite_dma_abs_y,m_cop_sprite_dma_size;
	UINT32 m_cop_sprite_dma_param;

	void copd2_set_tableoffset(UINT16 data);
	void copd2_set_tabledata(UINT16 data);
	DECLARE_WRITE16_MEMBER( seibu_common_video_regs_w );
	void cop_take_hit_box_params(UINT8 offs);
	UINT8 cop_calculate_collsion_detection();
	DECLARE_READ16_MEMBER( generic_cop_r );
	DECLARE_WRITE16_MEMBER( generic_cop_w );
};

extern const device_type SEIBU_COP_LEGACY;

#define MCFG_SEIBU_COP_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SEIBU_COP_LEGACY, 0)
