// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/* V53 */

#include "nec.h"
#include "necpriv.h"

#include "machine/pit8253.h"
#include "machine/am9517a.h"
#include "machine/pic8259.h"
#include "machine/i8251.h"

// SCU

#define MCFG_V53_SCU_TXD_HANDLER(_devcb) \
	devcb = &v53_base_device::set_txd_handler(*device, DEVCB_##_devcb);

#define MCFG_V53_SCU_DTR_HANDLER(_devcb) \
	devcb = &v53_base_device::set_dtr_handler(*device, DEVCB_##_devcb);

#define MCFG_V53_SCU_RTS_HANDLER(_devcb) \
	devcb = &v53_base_device::set_rts_handler(*device, DEVCB_##_devcb);

#define MCFG_V53_SCU_RXRDY_HANDLER(_devcb) \
	devcb = &v53_base_device::set_rxrdy_handler(*device, DEVCB_##_devcb);

#define MCFG_V53_SCU_TXRDY_HANDLER(_devcb) \
	devcb = &v53_base_device::set_txrdy_handler(*device, DEVCB_##_devcb);

#define MCFG_V53_SCU_TXEMPTY_HANDLER(_devcb) \
	devcb = &v53_base_device::set_txempty_handler(*device, DEVCB_##_devcb);

#define MCFG_V53_SCU_SYNDET_HANDLER(_devcb) \
	devcb = &v53_base_device::set_syndet_handler(*device, DEVCB_##_devcb);

// TCU
#define MCFG_V53_TCU_CLK0(_clk) \
	v53_base_device::set_clk0(*device, _clk);

#define MCFG_V53_TCU_CLK1(_clk) \
	v53_base_device::set_clk1(*device, _clk);

#define MCFG_V53_TCU_CLK2(_clk) \
	v53_base_device::set_clk2(*device, _clk);

#define MCFG_V53_TCU_OUT0_HANDLER(_devcb) \
	devcb = &v53_base_device::set_out0_handler(*device, DEVCB_##_devcb);

#define MCFG_V53_TCU_OUT1_HANDLER(_devcb) \
	devcb = &v53_base_device::set_out1_handler(*device, DEVCB_##_devcb);

#define MCFG_V53_TCU_OUT2_HANDLER(_devcb) \
	devcb = &v53_base_device::set_out2_handler(*device, DEVCB_##_devcb);

// DMAU

#define MCFG_V53_DMAU_OUT_HREQ_CB(_devcb) \
	devcb = &v53_base_device::set_out_hreq_callback(*device, DEVCB_##_devcb);

#define MCFG_V53_DMAU_OUT_EOP_CB(_devcb) \
	devcb = &v53_base_device::set_out_eop_callback(*device, DEVCB_##_devcb);

#define MCFG_V53_DMAU_IN_MEMR_CB(_devcb) \
	devcb = &v53_base_device::set_in_memr_callback(*device, DEVCB_##_devcb);

#define MCFG_V53_DMAU_OUT_MEMW_CB(_devcb) \
	devcb = &v53_base_device::set_out_memw_callback(*device, DEVCB_##_devcb);

#define MCFG_V53_DMAU_IN_IOR_0_CB(_devcb) \
	devcb = &v53_base_device::set_in_ior_0_callback(*device, DEVCB_##_devcb);

#define MCFG_V53_DMAU_IN_IOR_1_CB(_devcb) \
	devcb = &v53_base_device::set_in_ior_1_callback(*device, DEVCB_##_devcb);

#define MCFG_V53_DMAU_IN_IOR_2_CB(_devcb) \
	devcb = &v53_base_device::set_in_ior_2_callback(*device, DEVCB_##_devcb);

#define MCFG_V53_DMAU_IN_IOR_3_CB(_devcb) \
	devcb = &v53_base_device::set_in_ior_3_callback(*device, DEVCB_##_devcb);

#define MCFG_V53_DMAU_OUT_IOW_0_CB(_devcb) \
	devcb = &v53_base_device::set_out_iow_0_callback(*device, DEVCB_##_devcb);

#define MCFG_V53_DMAU_OUT_IOW_1_CB(_devcb) \
	devcb = &v53_base_device::set_out_iow_1_callback(*device, DEVCB_##_devcb);

#define MCFG_V53_DMAU_OUT_IOW_2_CB(_devcb) \
	devcb = &v53_base_device::set_out_iow_2_callback(*device, DEVCB_##_devcb);

#define MCFG_V53_DMAU_OUT_IOW_3_CB(_devcb) \
	devcb = &v53_base_device::set_out_iow_3_callback(*device, DEVCB_##_devcb);

#define MCFG_V53_DMAU_OUT_DACK_0_CB(_devcb) \
	devcb = &v53_base_device::set_out_dack_0_callback(*device, DEVCB_##_devcb);

#define MCFG_V53_DMAU_OUT_DACK_1_CB(_devcb) \
	devcb = &v53_base_device::set_out_dack_1_callback(*device, DEVCB_##_devcb);

#define MCFG_V53_DMAU_OUT_DACK_2_CB(_devcb) \
	devcb = &v53_base_device::set_out_dack_2_callback(*device, DEVCB_##_devcb);

#define MCFG_V53_DMAU_OUT_DACK_3_CB(_devcb) \
	devcb = &v53_base_device::set_out_dack_3_callback(*device, DEVCB_##_devcb);



class v53_base_device : public nec_common_device
{
public:
	v53_base_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, offs_t fetch_xor, UINT8 prefetch_size, UINT8 prefetch_cycles, UINT32 chip_type);

	DECLARE_WRITE8_MEMBER(BSEL_w);
	DECLARE_WRITE8_MEMBER(BADR_w);
	DECLARE_WRITE8_MEMBER(BRC_w);
	DECLARE_WRITE8_MEMBER(WMB0_w);
	DECLARE_WRITE8_MEMBER(WCY1_w);
	DECLARE_WRITE8_MEMBER(WCY0_w);
	DECLARE_WRITE8_MEMBER(WAC_w);
	DECLARE_WRITE8_MEMBER(TCKS_w);
	DECLARE_WRITE8_MEMBER(SBCR_w);
	DECLARE_WRITE8_MEMBER(REFC_w);
	DECLARE_WRITE8_MEMBER(WMB1_w);
	DECLARE_WRITE8_MEMBER(WCY2_w);
	DECLARE_WRITE8_MEMBER(WCY3_w);
	DECLARE_WRITE8_MEMBER(WCY4_w);
	DECLARE_WRITE8_MEMBER(SULA_w);
	DECLARE_WRITE8_MEMBER(TULA_w);
	DECLARE_WRITE8_MEMBER(IULA_w);
	DECLARE_WRITE8_MEMBER(DULA_w);
	DECLARE_WRITE8_MEMBER(OPHA_w);
	DECLARE_WRITE8_MEMBER(OPSEL_w);
	DECLARE_WRITE8_MEMBER(SCTL_w);

	UINT8 m_SCTL;
	UINT8 m_OPSEL;

	UINT8 m_SULA;
	UINT8 m_TULA;
	UINT8 m_IULA;
	UINT8 m_DULA;
	UINT8 m_OPHA;

	// SCU
	DECLARE_READ8_MEMBER(scu_simk_r);
	DECLARE_WRITE8_MEMBER(scu_simk_w);
	UINT8 m_simk;
	template<class _Object> static devcb_base &set_txd_handler(device_t &device, _Object object) { return downcast<v53_base_device &>(device).m_txd_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_dtr_handler(device_t &device, _Object object) { return downcast<v53_base_device &>(device).m_dtr_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_rts_handler(device_t &device, _Object object) { return downcast<v53_base_device &>(device).m_rts_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_rxrdy_handler(device_t &device, _Object object) { return downcast<v53_base_device &>(device).m_rxrdy_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_txrdy_handler(device_t &device, _Object object) { return downcast<v53_base_device &>(device).m_txrdy_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_txempty_handler(device_t &device, _Object object) { return downcast<v53_base_device &>(device).m_txempty_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_syndet_handler(device_t &device, _Object object) { return downcast<v53_base_device &>(device).m_syndet_handler.set_callback(object); }
	DECLARE_WRITE_LINE_MEMBER(scu_txd_trampoline_cb) { m_txd_handler(state); }
	DECLARE_WRITE_LINE_MEMBER(scu_dtr_trampoline_cb) { m_dtr_handler(state); }
	DECLARE_WRITE_LINE_MEMBER(scu_rts_trampoline_cb) {  m_rts_handler(state); }
	DECLARE_WRITE_LINE_MEMBER(scu_rxrdy_trampoline_cb) { m_rxrdy_handler(state); } /* should we mask this here based on m_simk? it can mask the interrupt */
	DECLARE_WRITE_LINE_MEMBER(scu_txrdy_trampoline_cb) { m_txrdy_handler(state); } /* should we mask this here based on m_simk? it can mask the interrupt */
	DECLARE_WRITE_LINE_MEMBER(scu_txempty_trampoline_cb) {  m_txempty_handler(state); }
	DECLARE_WRITE_LINE_MEMBER(scu_syndet_trampoline_cb) { m_syndet_handler(state); }

	// TCU
	DECLARE_READ8_MEMBER(tmu_tst0_r);
	DECLARE_WRITE8_MEMBER(tmu_tct0_w);
	DECLARE_READ8_MEMBER(tmu_tst1_r);
	DECLARE_WRITE8_MEMBER(tmu_tct1_w);
	DECLARE_READ8_MEMBER(tmu_tst2_r);
	DECLARE_WRITE8_MEMBER(tmu_tct2_w);
	DECLARE_WRITE8_MEMBER(tmu_tmd_w);
//  static void set_clk0(device_t &device, double clk0) { downcast<v53_base_device &>(device).m_clk0 = clk0; }
//  static void set_clk1(device_t &device, double clk1) { downcast<v53_base_device &>(device).m_clk1 = clk1; }
//  static void set_clk2(device_t &device, double clk2) { downcast<v53_base_device &>(device).m_clk2 = clk2; }
	template<class _Object> static devcb_base &set_out0_handler(device_t &device, _Object object) { return downcast<v53_base_device &>(device).m_out0_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_out1_handler(device_t &device, _Object object) { return downcast<v53_base_device &>(device).m_out1_handler.set_callback(object); }
	template<class _Object> static devcb_base &set_out2_handler(device_t &device, _Object object) { return downcast<v53_base_device &>(device).m_out2_handler.set_callback(object); }
	DECLARE_WRITE_LINE_MEMBER(tcu_out0_trampoline_cb){ m_out0_handler(state); }
	DECLARE_WRITE_LINE_MEMBER(tcu_out1_trampoline_cb){ m_out1_handler(state); }
	DECLARE_WRITE_LINE_MEMBER(tcu_out2_trampoline_cb){ m_out2_handler(state); }

	// DMAU
	template<class _Object> static devcb_base &set_out_hreq_callback(device_t &device, _Object object) { return downcast<v53_base_device &>(device).m_out_hreq_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_eop_callback(device_t &device, _Object object) { return downcast<v53_base_device &>(device).m_out_eop_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_memr_callback(device_t &device, _Object object) { return downcast<v53_base_device &>(device).m_in_memr_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_memw_callback(device_t &device, _Object object) { return downcast<v53_base_device &>(device).m_out_memw_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_ior_0_callback(device_t &device, _Object object) { return downcast<v53_base_device &>(device).m_in_ior_0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_ior_1_callback(device_t &device, _Object object) { return downcast<v53_base_device &>(device).m_in_ior_1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_ior_2_callback(device_t &device, _Object object) { return downcast<v53_base_device &>(device).m_in_ior_2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_ior_3_callback(device_t &device, _Object object) { return downcast<v53_base_device &>(device).m_in_ior_3_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_iow_0_callback(device_t &device, _Object object) { return downcast<v53_base_device &>(device).m_out_iow_0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_iow_1_callback(device_t &device, _Object object) { return downcast<v53_base_device &>(device).m_out_iow_1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_iow_2_callback(device_t &device, _Object object) { return downcast<v53_base_device &>(device).m_out_iow_2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_iow_3_callback(device_t &device, _Object object) { return downcast<v53_base_device &>(device).m_out_iow_3_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_dack_0_callback(device_t &device, _Object object) { return downcast<v53_base_device &>(device).m_out_dack_0_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_dack_1_callback(device_t &device, _Object object) { return downcast<v53_base_device &>(device).m_out_dack_1_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_dack_2_callback(device_t &device, _Object object) { return downcast<v53_base_device &>(device).m_out_dack_2_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_dack_3_callback(device_t &device, _Object object) { return downcast<v53_base_device &>(device).m_out_dack_3_cb.set_callback(object); }
	DECLARE_WRITE_LINE_MEMBER(hreq_trampoline_cb) { m_out_hreq_cb(state); }
	DECLARE_WRITE_LINE_MEMBER(eop_trampoline_cb) { m_out_eop_cb(state); }
	DECLARE_READ8_MEMBER(dma_memr_trampoline_r) { return m_in_memr_cb(space, offset); }
	DECLARE_WRITE8_MEMBER(dma_memw_trampoline_w) {  m_out_memw_cb(space, offset, data); }
	DECLARE_READ8_MEMBER(dma_io_0_trampoline_r) { return m_in_ior_0_cb(space, offset); }
	DECLARE_READ8_MEMBER(dma_io_1_trampoline_r) { return m_in_ior_1_cb(space, offset); }
	DECLARE_READ8_MEMBER(dma_io_2_trampoline_r) { return m_in_ior_2_cb(space, offset); }
	DECLARE_READ8_MEMBER(dma_io_3_trampoline_r) { return m_in_ior_3_cb(space, offset); }
	DECLARE_WRITE8_MEMBER(dma_io_0_trampoline_w) { m_out_iow_0_cb(space, offset, data); }
	DECLARE_WRITE8_MEMBER(dma_io_1_trampoline_w) { m_out_iow_1_cb(space, offset, data); }
	DECLARE_WRITE8_MEMBER(dma_io_2_trampoline_w) { m_out_iow_2_cb(space, offset, data); }
	DECLARE_WRITE8_MEMBER(dma_io_3_trampoline_w) { m_out_iow_3_cb(space, offset, data); }
	DECLARE_WRITE_LINE_MEMBER(dma_dack0_trampoline_w) { m_out_dack_0_cb(state); }
	DECLARE_WRITE_LINE_MEMBER(dma_dack1_trampoline_w) { m_out_dack_1_cb(state); }
	DECLARE_WRITE_LINE_MEMBER(dma_dack2_trampoline_w) { m_out_dack_2_cb(state); }
	DECLARE_WRITE_LINE_MEMBER(dma_dack3_trampoline_w) { m_out_dack_3_cb(state); }


	DECLARE_WRITE_LINE_MEMBER(dreq0_w);
	DECLARE_WRITE_LINE_MEMBER(dreq1_w);
	DECLARE_WRITE_LINE_MEMBER(dreq2_w);
	DECLARE_WRITE_LINE_MEMBER(dreq3_w);
	DECLARE_WRITE_LINE_MEMBER(hack_w);



	void install_peripheral_io();

	const address_space_config m_io_space_config;

	const address_space_config *memory_space_config(address_spacenum spacenum) const override
	{
		switch (spacenum)
		{
			case AS_IO: return &m_io_space_config;
			default: return nec_common_device::memory_space_config(spacenum);
		}
	}




	DECLARE_READ8_MEMBER(get_pic_ack);
	DECLARE_WRITE_LINE_MEMBER(internal_irq_w);


protected:
	// device-level overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void execute_set_input(int inputnum, int state) override;

	required_device<pit8253_device> m_v53tcu;
	required_device<upd71071_v53_device> m_v53dmau;
	required_device<pic8259_device> m_v53icu;
	required_device<v53_scu_device> m_v53scu;

	// SCU
	devcb_write_line m_txd_handler;
	devcb_write_line m_dtr_handler;
	devcb_write_line m_rts_handler;
	devcb_write_line m_rxrdy_handler;
	devcb_write_line m_txrdy_handler;
	devcb_write_line m_txempty_handler;
	devcb_write_line m_syndet_handler;

	// TCU
//  double m_clk0;
//  double m_clk1;
//  double m_clk2;
	devcb_write_line m_out0_handler;
	devcb_write_line m_out1_handler;
	devcb_write_line m_out2_handler;


	// DMAU
	devcb_write_line   m_out_hreq_cb;
	devcb_write_line   m_out_eop_cb;
	devcb_read8        m_in_memr_cb;
	devcb_write8       m_out_memw_cb;
	devcb_read8        m_in_ior_0_cb;
	devcb_read8        m_in_ior_1_cb;
	devcb_read8        m_in_ior_2_cb;
	devcb_read8        m_in_ior_3_cb;
	devcb_write8       m_out_iow_0_cb;
	devcb_write8       m_out_iow_1_cb;
	devcb_write8       m_out_iow_2_cb;
	devcb_write8       m_out_iow_3_cb;
	devcb_write_line   m_out_dack_0_cb;
	devcb_write_line   m_out_dack_1_cb;
	devcb_write_line   m_out_dack_2_cb;
	devcb_write_line   m_out_dack_3_cb;



};


class v53_device : public v53_base_device
{
public:
	v53_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class v53a_device : public v53_base_device
{
public:
	v53a_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

extern const device_type V53;
extern const device_type V53A;
