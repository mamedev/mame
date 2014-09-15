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


class raiden2cop_device : public device_t
{
public:
	raiden2cop_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

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
