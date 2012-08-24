/* 32X */


// Fifa96 needs the CPUs swapped for the gameplay to enter due to some race conditions
// when using the DRC core.  Needs further investigation, the non-DRC core works either
// way
#define _32X_SWAP_MASTER_SLAVE_HACK


#include "sound/dac.h"

#define _32X_MASTER_TAG (":sega32x:32x_master_sh2")
#define _32X_SLAVE_TAG (":sega32x:32x_slave_sh2")


class sega_32x_device : public device_t
{
public:
	sega_32x_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock, device_type type);

	required_device<dac_device> m_lch_pwm;
	required_device<dac_device> m_rch_pwm;
	
	DECLARE_READ32_MEMBER( _32x_sh2_master_4000_common_4002_r );
	DECLARE_READ32_MEMBER( _32x_sh2_slave_4000_common_4002_r );
	DECLARE_READ32_MEMBER( _32x_sh2_common_4004_common_4006_r );
	DECLARE_WRITE32_MEMBER( _32x_sh2_master_4000_common_4002_w );
	DECLARE_WRITE32_MEMBER( _32x_sh2_slave_4000_common_4002_w );
	DECLARE_WRITE32_MEMBER( _32x_sh2_common_4004_common_4006_w );
	DECLARE_WRITE32_MEMBER( _32x_sh2_master_4014_master_4016_w );
	DECLARE_WRITE32_MEMBER( _32x_sh2_master_4018_master_401a_w );
	DECLARE_WRITE32_MEMBER( _32x_sh2_master_401c_master_401e_w );
	DECLARE_WRITE32_MEMBER( _32x_sh2_slave_4014_slave_4016_w );
	DECLARE_WRITE32_MEMBER( _32x_sh2_slave_4018_slave_401a_w );
	DECLARE_WRITE32_MEMBER( _32x_sh2_slave_401c_slave_401e_w );
		

	DECLARE_READ16_MEMBER( _32x_68k_palette_r );
	DECLARE_WRITE16_MEMBER( _32x_68k_palette_w );
	DECLARE_READ16_MEMBER( _32x_68k_dram_r );
	DECLARE_WRITE16_MEMBER( _32x_68k_dram_w );
	DECLARE_READ16_MEMBER( _32x_68k_dram_overwrite_r );
	DECLARE_WRITE16_MEMBER( _32x_68k_dram_overwrite_w );
	DECLARE_READ16_MEMBER( _32x_68k_a15106_r );
	DECLARE_WRITE16_MEMBER( _32x_68k_a15106_w );
	DECLARE_READ16_MEMBER( _32x_dreq_common_r );
	DECLARE_WRITE16_MEMBER( _32x_dreq_common_w );
	DECLARE_READ16_MEMBER( _32x_68k_a1511a_r );
	DECLARE_WRITE16_MEMBER( _32x_68k_a1511a_w );
	DECLARE_READ16_MEMBER( _32x_68k_hint_vector_r );
	DECLARE_WRITE16_MEMBER( _32x_68k_hint_vector_w );
	DECLARE_READ16_MEMBER( _32x_68k_MARS_r );
	DECLARE_READ16_MEMBER( _32x_68k_a15100_r );
	DECLARE_WRITE16_MEMBER( _32x_68k_a15100_w );
	DECLARE_READ16_MEMBER( _32x_68k_a15102_r );
	DECLARE_WRITE16_MEMBER( _32x_68k_a15102_w );
	DECLARE_READ16_MEMBER( _32x_68k_a15104_r );
	DECLARE_WRITE16_MEMBER( _32x_68k_a15104_w );
	DECLARE_READ16_MEMBER( _32x_68k_commsram_r );
	DECLARE_WRITE16_MEMBER( _32x_68k_commsram_w );
	DECLARE_READ16_MEMBER( _32x_pwm_r );
	DECLARE_WRITE16_MEMBER( _32x_pwm_w );
	DECLARE_WRITE16_MEMBER( _32x_68k_pwm_w );
	DECLARE_READ16_MEMBER( _32x_common_vdp_regs_r );
	DECLARE_WRITE16_MEMBER( _32x_common_vdp_regs_w );
	DECLARE_READ16_MEMBER( _32x_sh2_master_4000_r );
	DECLARE_WRITE16_MEMBER( _32x_sh2_master_4000_w );
	DECLARE_READ16_MEMBER( _32x_sh2_slave_4000_r );
	DECLARE_WRITE16_MEMBER( _32x_sh2_slave_4000_w );
	DECLARE_READ16_MEMBER( _32x_sh2_common_4002_r );
	DECLARE_WRITE16_MEMBER( _32x_sh2_common_4002_w );
	DECLARE_READ16_MEMBER( _32x_sh2_common_4004_r );
	DECLARE_WRITE16_MEMBER( _32x_sh2_common_4004_w );
	DECLARE_READ16_MEMBER( _32x_sh2_common_4006_r );
	DECLARE_WRITE16_MEMBER( _32x_sh2_common_4006_w );
	DECLARE_WRITE16_MEMBER( _32x_sh2_master_4014_w );
	DECLARE_WRITE16_MEMBER( _32x_sh2_slave_4014_w );
	DECLARE_WRITE16_MEMBER( _32x_sh2_master_4016_w );
	DECLARE_WRITE16_MEMBER( _32x_sh2_slave_4016_w );
	DECLARE_WRITE16_MEMBER( _32x_sh2_master_4018_w );
	DECLARE_WRITE16_MEMBER( _32x_sh2_slave_4018_w ) ;
	DECLARE_WRITE16_MEMBER( _32x_sh2_master_401a_w );
	DECLARE_WRITE16_MEMBER( _32x_sh2_slave_401a_w );
	DECLARE_WRITE16_MEMBER( _32x_sh2_master_401c_w );
	DECLARE_WRITE16_MEMBER( _32x_sh2_slave_401c_w );
	DECLARE_WRITE16_MEMBER( _32x_sh2_master_401e_w );
	DECLARE_WRITE16_MEMBER( _32x_sh2_slave_401e_w );
	
	emu_timer *m_32x_pwm_timer;
protected:
	virtual void device_start();
	virtual void device_reset();

	// optional information overrides
//	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
private:
//	virtual void device_config_complete();
};


class sega_32x_ntsc_device : public sega_32x_device
{
	public:
		sega_32x_ntsc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

};

class sega_32x_pal_device : public sega_32x_device
{
	public:
		sega_32x_pal_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	protected:
		virtual machine_config_constructor device_mconfig_additions() const;
};


extern const device_type SEGA_32X_NTSC;
extern const device_type SEGA_32X_PAL;
