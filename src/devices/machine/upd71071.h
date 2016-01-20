// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
#ifndef __UPD71071_H__
#define __UPD71071_H__

#include "emu.h"


struct upd71071_reg
{
	UINT8 initialise;
	UINT8 channel;
	UINT16 count_current[4];
	UINT16 count_base[4];
	UINT32 address_current[4];
	UINT32 address_base[4];
	UINT16 device_control;
	UINT8 mode_control[4];
	UINT8 status;
	UINT8 temp_l;
	UINT8 temp_h;
	UINT8 request;
	UINT8 mask;
};

class upd71071_device : public device_t
{
public:
	upd71071_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~upd71071_device() {}

	static void static_set_cpu_tag(device_t &device, std::string tag) { downcast<upd71071_device &>(device).m_cpu.set_tag(tag); }
	static void set_clock(device_t &device, int clock) { downcast<upd71071_device &>(device).m_upd_clock = clock; }
	template<class _Object> static devcb_base &set_out_hreq_callback(device_t &device, _Object object) { return downcast<upd71071_device &>(device).m_out_hreq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_eop_callback(device_t &device, _Object object) { return downcast<upd71071_device &>(device).m_out_eop_cb.set_callback(object); }

	template<class _Object> static devcb_base &set_dma_read_0_callback(device_t &device, _Object object) { return downcast<upd71071_device &>(device).m_dma_read_0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_dma_read_1_callback(device_t &device, _Object object) { return downcast<upd71071_device &>(device).m_dma_read_1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_dma_read_2_callback(device_t &device, _Object object) { return downcast<upd71071_device &>(device).m_dma_read_2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_dma_read_3_callback(device_t &device, _Object object) { return downcast<upd71071_device &>(device).m_dma_read_3_cb.set_callback(object); }

	template<class _Object> static devcb_base &set_dma_write_0_callback(device_t &device, _Object object) { return downcast<upd71071_device &>(device).m_dma_write_0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_dma_write_1_callback(device_t &device, _Object object) { return downcast<upd71071_device &>(device).m_dma_write_1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_dma_write_2_callback(device_t &device, _Object object) { return downcast<upd71071_device &>(device).m_dma_write_2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_dma_write_3_callback(device_t &device, _Object object) { return downcast<upd71071_device &>(device).m_dma_write_3_cb.set_callback(object); }

	template<class _Object> static devcb_base &set_out_dack_0_callback(device_t &device, _Object object) { return downcast<upd71071_device &>(device).m_out_dack_0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_dack_1_callback(device_t &device, _Object object) { return downcast<upd71071_device &>(device).m_out_dack_1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_dack_2_callback(device_t &device, _Object object) { return downcast<upd71071_device &>(device).m_out_dack_2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_dack_3_callback(device_t &device, _Object object) { return downcast<upd71071_device &>(device).m_out_dack_3_cb.set_callback(object); }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_WRITE_LINE_MEMBER(set_hreq);
	DECLARE_WRITE_LINE_MEMBER(set_eop);

	int dmarq(int state, int channel);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	// internal state
	void soft_reset();
	TIMER_CALLBACK_MEMBER(dma_transfer_timer);

	struct upd71071_reg m_reg;
	int m_selected_channel;
	int m_buswidth;
	int m_dmarq[4];
	emu_timer* m_timer[4];
	//int m_in_progress[4];
	//int m_transfer_size[4];
	int m_base;
	int m_upd_clock;
	devcb_write_line    m_out_hreq_cb;
	devcb_write_line    m_out_eop_cb;
	devcb_read16        m_dma_read_0_cb;
	devcb_read16        m_dma_read_1_cb;
	devcb_read16        m_dma_read_2_cb;
	devcb_read16        m_dma_read_3_cb;
	devcb_write16       m_dma_write_0_cb;
	devcb_write16       m_dma_write_1_cb;
	devcb_write16       m_dma_write_2_cb;
	devcb_write16       m_dma_write_3_cb;
	devcb_write_line   m_out_dack_0_cb;
	devcb_write_line   m_out_dack_1_cb;
	devcb_write_line   m_out_dack_2_cb;
	devcb_write_line   m_out_dack_3_cb;
	int m_hreq;
	int m_eop;
	optional_device<cpu_device> m_cpu;
};

extern const device_type UPD71071;

#define MCFG_UPD71071_CPU(_tag) \
	upd71071_device::static_set_cpu_tag(*device, "^" _tag);

#define MCFG_UPD71071_CLOCK(_clk) \
	upd71071_device::set_clock(*device, _clk);

#define MCFG_UPD71071_OUT_HREQ_CB(_devcb) \
	devcb = &upd71071_device::set_out_hreq_callback(*device, DEVCB_##_devcb);

#define MCFG_UPD71071_OUT_EOP_CB(_devcb) \
	devcb = &upd71071_device::set_out_eop_callback(*device, DEVCB_##_devcb);

#define MCFG_UPD71071_DMA_READ_0_CB(_devcb) \
	devcb = &upd71071_device::set_dma_read_0_callback(*device, DEVCB_##_devcb);

#define MCFG_UPD71071_DMA_READ_1_CB(_devcb) \
	devcb = &upd71071_device::set_dma_read_1_callback(*device, DEVCB_##_devcb);

#define MCFG_UPD71071_DMA_READ_2_CB(_devcb) \
	devcb = &upd71071_device::set_dma_read_2_callback(*device, DEVCB_##_devcb);

#define MCFG_UPD71071_DMA_READ_3_CB(_devcb) \
	devcb = &upd71071_device::set_dma_read_3_callback(*device, DEVCB_##_devcb);

#define MCFG_UPD71071_DMA_WRITE_0_CB(_devcb) \
	devcb = &upd71071_device::set_dma_write_0_callback(*device, DEVCB_##_devcb);

#define MCFG_UPD71071_DMA_WRITE_1_CB(_devcb) \
	devcb = &upd71071_device::set_dma_write_1_callback(*device, DEVCB_##_devcb);

#define MCFG_UPD71071_DMA_WRITE_2_CB(_devcb) \
	devcb = &upd71071_device::set_dma_write_2_callback(*device, DEVCB_##_devcb);

#define MCFG_UPD71071_DMA_WRITE_3_CB(_devcb) \
	devcb = &upd71071_device::set_dma_write_3_callback(*device, DEVCB_##_devcb);

#define MCFG_UPD71071_OUT_DACK_0_CB(_devcb) \
	devcb = &upd71071_device::set_out_dack_0_callback(*device, DEVCB_##_devcb);

#define MCFG_UPD71071_OUT_DACK_1_CB(_devcb) \
	devcb = &upd71071_device::set_out_dack_1_callback(*device, DEVCB_##_devcb);

#define MCFG_UPD71071_OUT_DACK_2_CB(_devcb) \
	devcb = &upd71071_device::set_out_dack_2_callback(*device, DEVCB_##_devcb);

#define MCFG_UPD71071_OUT_DACK_3_CB(_devcb) \
	devcb = &upd71071_device::set_out_dack_3_callback(*device, DEVCB_##_devcb);

#endif /*UPD71071_H_*/
