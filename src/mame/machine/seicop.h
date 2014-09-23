
#include "raiden2cop.h"

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

	DECLARE_READ16_MEMBER( generic_cop_r );
	DECLARE_WRITE16_MEMBER( generic_cop_w );
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

private:
	UINT16 *m_cop_mcu_ram;



	UINT16 m_copd2_offs;
	INT16 m_cop_hit_val_x,m_cop_hit_val_y,m_cop_hit_val_z,m_cop_hit_val_unk;
	INT8 m_legacycop_angle_compare;
	INT8 m_legacycop_angle_mod_val;
	struct collision_info m_cop_collision_info[2];
	int m_r0, m_r1;
	UINT16 m_cop_rom_addr_lo,m_cop_rom_addr_hi,m_cop_rom_addr_unk;

	UINT32 m_cop_sprite_dma_src;
	int m_cop_sprite_dma_abs_x,m_cop_sprite_dma_abs_y,m_cop_sprite_dma_size;
	UINT32 m_cop_sprite_dma_param;


	void cop_take_hit_box_params(UINT8 offs);
	UINT8 cop_calculate_collsion_detection();


	required_device<raiden2cop_device> m_raiden2cop;

};

extern const device_type SEIBU_COP_LEGACY;

#define MCFG_SEIBU_COP_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SEIBU_COP_LEGACY, 0)

