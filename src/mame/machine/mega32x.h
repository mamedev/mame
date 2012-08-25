/* 32X */


// Fifa96 needs the CPUs swapped for the gameplay to enter due to some race conditions
// when using the DRC core.  Needs further investigation, the non-DRC core works either
// way
#define _32X_SWAP_MASTER_SLAVE_HACK
#define _32X_COMMS_PORT_SYNC 0
#define MAX_HPOSITION 480
/* need to make some pwm stuff part of device */
#define PWM_FIFO_SIZE m_pwm_tm_reg // guess, Marsch calls this register as FIFO width
#define PWM_CLOCK megadrive_region_pal ? ((MASTER_CLOCK_PAL*3) / 7) : ((MASTER_CLOCK_NTSC*3) / 7)



#define SH2_VRES_IRQ_LEVEL 14
#define SH2_VINT_IRQ_LEVEL 12
#define SH2_HINT_IRQ_LEVEL 10
#define SH2_CINT_IRQ_LEVEL 8
#define SH2_PINT_IRQ_LEVEL 6


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
	DECLARE_READ16_MEMBER( _32x_68k_m_hint_vector_r );
	DECLARE_WRITE16_MEMBER( _32x_68k_m_hint_vector_w );
	DECLARE_READ16_MEMBER( _32x_68k_MARS_r );
	DECLARE_READ16_MEMBER( _32x_68k_a15100_r );
	DECLARE_WRITE16_MEMBER( _32x_68k_a15100_w );
	DECLARE_READ16_MEMBER( _32x_68k_a15102_r );
	DECLARE_WRITE16_MEMBER( _32x_68k_a15102_w );
	DECLARE_READ16_MEMBER( _32x_68k_a15104_r );
	DECLARE_WRITE16_MEMBER( _32x_68k_a15104_w );
	DECLARE_READ16_MEMBER( _32x_68k_m_commsram_r );
	DECLARE_WRITE16_MEMBER( _32x_68k_m_commsram_w );
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
	
	UINT32* _32x_render_videobuffer_to_screenbuffer_helper(running_machine &machine, int scanline);
	int sh2_master_pwmint_enable, sh2_slave_pwmint_enable;

	void _32x_check_framebuffer_swap(void);
	void _32x_check_irqs(running_machine& machine);
	void _32x_scanline_cb0(running_machine& machine);
	void _32x_scanline_cb1();

	/* our current main rendering code needs to know this for mixing in */
	int m_32x_displaymode;
	int m_32x_videopriority;
	/* our main vblank handler resets this */
	int m_32x_hcount_compare_val;
	int m_sh2_are_running;
	int m_32x_240mode;
	UINT16 m_32x_a1518a_reg;


	UINT32 m_32x_linerender[320+258]; // tmp buffer (bigger than it needs to be to simplify RLE decode)


	void handle_pwm_callback(void);
	void calculate_pwm_timer(running_machine &machine);
	UINT16 m_pwm_ctrl,m_pwm_cycle,m_pwm_tm_reg;
	UINT16 m_cur_lch[0x10],m_cur_rch[0x10];
	UINT16 m_pwm_cycle_reg; //used for latching
	UINT8 m_pwm_timer_tick;
	UINT8 m_lch_index_r,m_rch_index_r,m_lch_index_w,m_rch_index_w;
	UINT16 m_lch_fifo_state,m_rch_fifo_state;


	UINT16 get_hposition(void);

	emu_timer *m_32x_pwm_timer;
protected:
	virtual void device_start();
	virtual void device_reset();

	// optional information overrides
//	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
private:
//	virtual void device_config_complete();
	int m_32x_adapter_enabled;
	int m_32x_access_auth;
	int m_32x_screenshift;

	UINT16 m_32x_68k_a15104_reg;
	int m_sh2_master_vint_enable, m_sh2_slave_vint_enable;
	int m_sh2_master_hint_enable, m_sh2_slave_hint_enable;
	int m_sh2_master_cmdint_enable, m_sh2_slave_cmdint_enable;
	int m_sh2_hint_in_vbl;
	int m_sh2_master_vint_pending;
	int m_sh2_slave_vint_pending;
	int m_32x_fb_swap;
	int m_32x_hcount_reg;

	UINT16 m_32x_autofill_length;
	UINT16 m_32x_autofill_address;
	UINT16 m_32x_autofill_data;
	UINT16 m_a15106_reg;
	UINT16 m_dreq_src_addr[2],m_dreq_dst_addr[2],m_dreq_size;
	UINT8 m_sega_tv;
	UINT16 m_hint_vector[2];
	UINT16 m_a15100_reg;
	int m_32x_68k_a15102_reg;

	UINT16 m_commsram[8];

	UINT16* m_32x_dram0;
	UINT16* m_32x_dram1;
	UINT16 *m_32x_display_dram, *m_32x_access_dram;
	UINT16* m_32x_palette;
	UINT16* m_32x_palette_lookup;
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
