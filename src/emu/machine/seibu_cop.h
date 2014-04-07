/***************************************************************************

    Seibu COP device

***************************************************************************/

#pragma once

#ifndef __SEIBU_COPDEV_H__
#define __SEIBU_COPDEV_H__



//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SEIBU_COP_IN_BYTE_CB(_devcb) \
	devcb = &seibu_cop_device::set_in_byte_callback(*device, DEVCB2_##_devcb);

#define MCFG_SEIBU_COP_IN_WORD_CB(_devcb) \
	devcb = &seibu_cop_device::set_in_word_callback(*device, DEVCB2_##_devcb);

#define MCFG_SEIBU_COP_IN_DWORD_CB(_devcb) \
	devcb = &seibu_cop_device::set_in_dword_callback(*device, DEVCB2_##_devcb);

#define MCFG_SEIBU_COP_OUT_BYTE_CB(_devcb) \
	devcb = &seibu_cop_device::set_out_byte_callback(*device, DEVCB2_##_devcb);

#define MCFG_SEIBU_COP_OUT_WORD_CB(_devcb) \
	devcb = &seibu_cop_device::set_out_word_callback(*device, DEVCB2_##_devcb);

#define MCFG_SEIBU_COP_OUT_DWORD_CB(_devcb) \
	devcb = &seibu_cop_device::set_out_dword_callback(*device, DEVCB2_##_devcb);



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> seibu_cop_device

class seibu_cop_device :    public device_t,
							public device_memory_interface
{
public:
	// construction/destruction
	seibu_cop_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb2_base &set_in_byte_callback(device_t &device, _Object object) { return downcast<seibu_cop_device &>(device).m_in_byte_cb.set_callback(object); }
	template<class _Object> static devcb2_base &set_in_word_callback(device_t &device, _Object object) { return downcast<seibu_cop_device &>(device).m_in_word_cb.set_callback(object); }
	template<class _Object> static devcb2_base &set_in_dword_callback(device_t &device, _Object object) { return downcast<seibu_cop_device &>(device).m_in_dword_cb.set_callback(object); }
	template<class _Object> static devcb2_base &set_out_byte_callback(device_t &device, _Object object) { return downcast<seibu_cop_device &>(device).m_out_byte_cb.set_callback(object); }
	template<class _Object> static devcb2_base &set_out_word_callback(device_t &device, _Object object) { return downcast<seibu_cop_device &>(device).m_out_word_cb.set_callback(object); }
	template<class _Object> static devcb2_base &set_out_dword_callback(device_t &device, _Object object) { return downcast<seibu_cop_device &>(device).m_out_dword_cb.set_callback(object); }

	// I/O operations
	DECLARE_WRITE16_MEMBER( write );
	DECLARE_READ16_MEMBER( read );
	DECLARE_WRITE16_MEMBER( dma_write_trigger_w );
	DECLARE_WRITE16_MEMBER(fill_val_lo_w);
	DECLARE_WRITE16_MEMBER(fill_val_hi_w);
	DECLARE_WRITE16_MEMBER(pal_brightness_val_w);
	DECLARE_WRITE16_MEMBER(pal_brightness_mode_w);
	DECLARE_WRITE16_MEMBER(dma_unk_param_w);
	DECLARE_WRITE16_MEMBER(dma_pal_fade_table_w);
	DECLARE_WRITE16_MEMBER(dma_src_w);
	DECLARE_WRITE16_MEMBER(dma_size_w);
	DECLARE_WRITE16_MEMBER(dma_dst_w);
	DECLARE_READ16_MEMBER(dma_trigger_r);
	DECLARE_WRITE16_MEMBER(dma_trigger_w);

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start();
	virtual void device_reset();
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

private:
	devcb2_read16       m_in_byte_cb;
	devcb2_read16       m_in_word_cb;
	devcb2_read16       m_in_dword_cb;
	devcb2_write16      m_out_byte_cb;
	devcb2_write16      m_out_word_cb;
	devcb2_write16      m_out_dword_cb;
	inline UINT16 read_word(offs_t address);
	inline void write_word(offs_t address, UINT16 data);

	UINT16 m_dma_unk_param, m_dma_pal_fade_table, m_dma_src[8], m_dma_dst[8], m_dma_size[8], m_dma_exec_param;
	UINT8 m_dma_trigger;
	UINT16 m_fill_val_lo,m_fill_val_hi;
	UINT32 m_fill_val;
	UINT16 m_pal_brightness_val, m_pal_brightness_mode;

	const address_space_config      m_space_config;

	const UINT8 fade_table(int v);
	void normal_dma_transfer(void);
	void palette_dma_transfer(void);
	void fill_word_transfer(void);
	void fill_dword_transfer(void);
};


// device type definition
extern const device_type SEIBU_COP;



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
