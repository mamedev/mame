/***************************************************************************

 Seibu Cop (Co-Processor) emulation
  (new implementation, based on Raiden 2 code)

***************************************************************************/

#ifndef RAIDEN2COP_H
#define RAIDEN2COP_H


#define MCFG_RAIDEN2COP_ADD(_tag ) \
	MCFG_DEVICE_ADD(_tag, RAIDEN2COP, 0)

#define MCFG_RAIDEN2COP_VIDEORAM_OUT_CB(_devcb) \
	devcb = &raiden2cop_device::set_m_videoramout_cb(*device, DEVCB_##_devcb);

#define MCFG_ITOA_UNUSED_DIGIT_VALUE(value) \
	raiden2cop_device::set_itoa_unused_digit_value(*device, value);


class raiden2cop_device : public device_t
{
public:
	raiden2cop_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// Command Table
	UINT16 cop_func_trigger[0x100/8];       /* function trigger */
	UINT16 cop_func_value[0x100/8];         /* function value (?) */
	UINT16 cop_func_mask[0x100/8];          /* function mask (?) */
	UINT16 cop_program[0x100];              /* program "code" */
	UINT16 cop_latch_addr, cop_latch_trigger, cop_latch_value, cop_latch_mask;

	DECLARE_WRITE16_MEMBER( cop_pgm_data_w );
	DECLARE_WRITE16_MEMBER( cop_pgm_addr_w );
	DECLARE_WRITE16_MEMBER( cop_pgm_value_w );
	DECLARE_WRITE16_MEMBER( cop_pgm_mask_w );
	DECLARE_WRITE16_MEMBER( cop_pgm_trigger_w );

	// these are used by legionna.c to find the command based on trigger value
	// legionna/seicop implementation then looks up the command based on hardcoded sequences in the driver rather than actually using the trigger value (should be changed)
	UINT16 get_func_value(int command) { return cop_func_value[command]; }
	UINT16 get_func_mask(int command) { return cop_func_mask[command]; }
	int find_trigger_match(UINT16 triggerval, UINT16 mask);
	int check_command_matches(int command, UINT16 seq0, UINT16 seq1, UINT16 seq2, UINT16 seq3, UINT16 seq4, UINT16 seq5, UINT16 seq6, UINT16 seq7, UINT16 _funcval_, UINT16 _funcmask_);

	// DMA
	UINT16 cop_dma_v1, cop_dma_v2, cop_dma_mode;
	UINT16 cop_dma_src[0x200], cop_dma_dst[0x200], cop_dma_size[0x200];
	UINT16 cop_dma_adr_rel;

	UINT16 pal_brightness_val;
	UINT16 pal_brightness_mode;

	DECLARE_WRITE16_MEMBER( cop_dma_v1_w );
	DECLARE_WRITE16_MEMBER( cop_dma_v2_w );

	DECLARE_WRITE16_MEMBER( cop_dma_adr_rel_w );
	DECLARE_WRITE16_MEMBER( cop_dma_src_w );
	DECLARE_WRITE16_MEMBER( cop_dma_size_w );
	DECLARE_WRITE16_MEMBER( cop_dma_dst_w );
	DECLARE_READ16_MEMBER( cop_dma_mode_r );
	DECLARE_WRITE16_MEMBER( cop_dma_mode_w );

	DECLARE_WRITE16_MEMBER( cop_pal_brightness_val_w );
	DECLARE_WRITE16_MEMBER( cop_pal_brightness_mode_w );

	DECLARE_WRITE16_MEMBER ( cop_dma_trigger_w );

	const UINT8 fade_table(int v);

	template<class _Object> static devcb_base &set_m_videoramout_cb(device_t &device, _Object object) { return downcast<raiden2cop_device &>(device).m_videoramout_cb.set_callback(object); }

	// Number Conversion

	DECLARE_WRITE16_MEMBER( cop_itoa_low_w );
	DECLARE_WRITE16_MEMBER( cop_itoa_high_w );
	DECLARE_WRITE16_MEMBER( cop_itoa_digit_count_w );
	DECLARE_READ16_MEMBER ( cop_itoa_digits_r );
	
	UINT32 cop_itoa;
	UINT16 cop_itoa_digit_count;
	UINT8 cop_itoa_digits[10];
	UINT8 m_cop_itoa_unused_digit_value;

	static void set_itoa_unused_digit_value(device_t &device, int value) { downcast<raiden2cop_device &>(device).m_cop_itoa_unused_digit_value = value; }
protected:
	// device-level overrides
	virtual void device_start();

private:
	// internal state
	devcb_write16       m_videoramout_cb;
	required_device<palette_device> m_palette;
};

extern const device_type RAIDEN2COP;

#endif  /* RAIDEN2COP_H */
