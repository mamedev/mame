// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    AMD AM9517A/8237A Multimode DMA Controller emulation

****************************************************************************
                            _____   _____
                  _IOR   1 |*    \_/     | 40  A7
                  _IOW   2 |             | 39  A6
                 _MEMR   3 |             | 38  A5
                 _MEMW   4 |             | 37  A4
                     *   5 |             | 36  _EOP
                 READY   6 |             | 35  A3
                  HACK   7 |             | 34  A2
                 ADSTB   8 |             | 33  A1
                   AEN   9 |             | 32  A0
                  HREQ  10 |   AM9517A   | 31  Vcc
                   _CS  11 |    8237A    | 30  DB0
                   CLK  12 |             | 29  DB1
                 RESET  13 |             | 28  DB2
                 DACK2  14 |             | 27  DB3
                 DACK3  15 |             | 26  DB4
                 DREQ3  16 |             | 25  DACK0
                 DREQ2  17 |             | 24  DACK1
                 DREQ1  18 |             | 23  DB5
                 DREQ0  19 |             | 22  DB6
                   Vss  20 |_____________| 21  DB7

***************************************************************************/

#ifndef MAME_MACHINE_AM9517_H
#define MAME_MACHINE_AM9517_H

#pragma once



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> am9517a_device

class am9517a_device :  public device_t,
						public device_execute_interface
{
	friend class pcxport_dmac_device;

public:
	// construction/destruction
	am9517a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> devcb_base &set_out_hreq_callback(Object &&cb) { return m_out_hreq_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_eop_callback(Object &&cb) { return m_out_eop_cb.set_callback(std::forward<Object>(cb)); }

	template <class Object> devcb_base &set_in_memr_callback(Object &&cb) { return m_in_memr_cb.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_memw_callback(Object &&cb) { return m_out_memw_cb.set_callback(std::forward<Object>(cb)); }

	template <unsigned C, class Object> devcb_base &set_in_ior_callback(Object &&cb) { return m_in_ior_cb[C].set_callback(std::forward<Object>(cb)); }
	template <unsigned C, class Object> devcb_base &set_out_iow_callback(Object &&cb) { return m_out_iow_cb[C].set_callback(std::forward<Object>(cb)); }
	template <unsigned C, class Object> devcb_base &set_out_dack_callback(Object &&cb) { return m_out_dack_cb[C].set_callback(std::forward<Object>(cb)); }

	virtual DECLARE_READ8_MEMBER( read );
	virtual DECLARE_WRITE8_MEMBER( write );

	DECLARE_WRITE_LINE_MEMBER( hack_w );
	DECLARE_WRITE_LINE_MEMBER( ready_w );
	DECLARE_WRITE_LINE_MEMBER( eop_w );

	DECLARE_WRITE_LINE_MEMBER( dreq0_w );
	DECLARE_WRITE_LINE_MEMBER( dreq1_w );
	DECLARE_WRITE_LINE_MEMBER( dreq2_w );
	DECLARE_WRITE_LINE_MEMBER( dreq3_w );

protected:
	am9517a_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void execute_run() override;

	virtual void end_of_process();

	int m_icount;
	uint32_t m_address_mask;

	struct
	{
		uint32_t m_address;
		uint16_t m_count;
		uint32_t m_base_address;
		uint16_t m_base_count;
		uint8_t m_mode;
	} m_channel[4];

	int m_msb;
	int m_hreq;
	int m_hack;
	int m_ready;
	int m_eop;
	int m_state;
	int m_current_channel;
	int m_last_channel;
	uint8_t m_command;
	uint8_t m_mask;
	uint8_t m_status;
	uint16_t m_temp;
	uint8_t m_request;

private:
	inline void dma_request(int channel, int state);
	inline bool is_request_active(int channel);
	inline bool is_software_request_active(int channel);
	inline void set_hreq(int state);
	inline void set_dack();
	inline void set_eop(int state);
	inline int get_state1(bool msb_changed);
	inline void dma_read();
	inline void dma_write();
	inline void dma_advance();

	devcb_write_line   m_out_hreq_cb;
	devcb_write_line   m_out_eop_cb;

	devcb_read8        m_in_memr_cb;
	devcb_write8       m_out_memw_cb;

	devcb_read8        m_in_ior_cb[4];
	devcb_write8       m_out_iow_cb[4];
	devcb_write_line   m_out_dack_cb[4];
};


class upd71071_v53_device :  public am9517a_device
{
public:
	// construction/destruction
	upd71071_v53_device(const machine_config &mconfig,  const char *tag, device_t *owner, uint32_t clock);

	virtual DECLARE_READ8_MEMBER( read ) override;
	virtual DECLARE_WRITE8_MEMBER( write ) override;

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	int m_selected_channel;
	int m_base;
	uint8_t m_command_high;

};


class pcxport_dmac_device : public am9517a_device
{
public:
	// construction/destruction
	pcxport_dmac_device(const machine_config &mconfig,  const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_reset() override;

	virtual void end_of_process() override;
};



// device type definition
DECLARE_DEVICE_TYPE(AM9517A,      am9517a_device)
DECLARE_DEVICE_TYPE(V53_DMAU,     upd71071_v53_device)
DECLARE_DEVICE_TYPE(PCXPORT_DMAC, pcxport_dmac_device)


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_AM9517A_OUT_HREQ_CB(_devcb) \
	devcb = &downcast<am9517a_device &>(*device).set_out_hreq_callback(DEVCB_##_devcb);

#define MCFG_AM9517A_OUT_EOP_CB(_devcb) \
	devcb = &downcast<am9517a_device &>(*device).set_out_eop_callback(DEVCB_##_devcb);

#define MCFG_AM9517A_IN_MEMR_CB(_devcb) \
	devcb = &downcast<am9517a_device &>(*device).set_in_memr_callback(DEVCB_##_devcb);

#define MCFG_AM9517A_OUT_MEMW_CB(_devcb) \
	devcb = &downcast<am9517a_device &>(*device).set_out_memw_callback(DEVCB_##_devcb);

#define MCFG_AM9517A_IN_IOR_0_CB(_devcb) \
	devcb = &downcast<am9517a_device &>(*device).set_in_ior_callback<0>(DEVCB_##_devcb);

#define MCFG_AM9517A_IN_IOR_1_CB(_devcb) \
	devcb = &downcast<am9517a_device &>(*device).set_in_ior_callback<1>(DEVCB_##_devcb);

#define MCFG_AM9517A_IN_IOR_2_CB(_devcb) \
	devcb = &downcast<am9517a_device &>(*device).set_in_ior_callback<2>(DEVCB_##_devcb);

#define MCFG_AM9517A_IN_IOR_3_CB(_devcb) \
	devcb = &downcast<am9517a_device &>(*device).set_in_ior_callback<3>(DEVCB_##_devcb);

#define MCFG_AM9517A_OUT_IOW_0_CB(_devcb) \
	devcb = &downcast<am9517a_device &>(*device).set_out_iow_callback<0>(DEVCB_##_devcb);

#define MCFG_AM9517A_OUT_IOW_1_CB(_devcb) \
	devcb = &downcast<am9517a_device &>(*device).set_out_iow_callback<1>(DEVCB_##_devcb);

#define MCFG_AM9517A_OUT_IOW_2_CB(_devcb) \
	devcb = &downcast<am9517a_device &>(*device).set_out_iow_callback<2>(DEVCB_##_devcb);

#define MCFG_AM9517A_OUT_IOW_3_CB(_devcb) \
	devcb = &downcast<am9517a_device &>(*device).set_out_iow_callback<3>(DEVCB_##_devcb);

#define MCFG_AM9517A_OUT_DACK_0_CB(_devcb) \
	devcb = &downcast<am9517a_device &>(*device).set_out_dack_callback<0>(DEVCB_##_devcb);

#define MCFG_AM9517A_OUT_DACK_1_CB(_devcb) \
	devcb = &downcast<am9517a_device &>(*device).set_out_dack_callback<1>(DEVCB_##_devcb);

#define MCFG_AM9517A_OUT_DACK_2_CB(_devcb) \
	devcb = &downcast<am9517a_device &>(*device).set_out_dack_callback<2>(DEVCB_##_devcb);

#define MCFG_AM9517A_OUT_DACK_3_CB(_devcb) \
	devcb = &downcast<am9517a_device &>(*device).set_out_dack_callback<3>(DEVCB_##_devcb);

#define MCFG_I8237_OUT_HREQ_CB MCFG_AM9517A_OUT_HREQ_CB
#define MCFG_I8237_OUT_EOP_CB MCFG_AM9517A_OUT_EOP_CB
#define MCFG_I8237_IN_MEMR_CB MCFG_AM9517A_IN_MEMR_CB
#define MCFG_I8237_OUT_MEMW_CB MCFG_AM9517A_OUT_MEMW_CB
#define MCFG_I8237_IN_IOR_0_CB MCFG_AM9517A_IN_IOR_0_CB
#define MCFG_I8237_IN_IOR_1_CB MCFG_AM9517A_IN_IOR_1_CB
#define MCFG_I8237_IN_IOR_2_CB MCFG_AM9517A_IN_IOR_2_CB
#define MCFG_I8237_IN_IOR_3_CB MCFG_AM9517A_IN_IOR_3_CB
#define MCFG_I8237_OUT_IOW_0_CB MCFG_AM9517A_OUT_IOW_0_CB
#define MCFG_I8237_OUT_IOW_1_CB MCFG_AM9517A_OUT_IOW_1_CB
#define MCFG_I8237_OUT_IOW_2_CB MCFG_AM9517A_OUT_IOW_2_CB
#define MCFG_I8237_OUT_IOW_3_CB MCFG_AM9517A_OUT_IOW_3_CB
#define MCFG_I8237_OUT_DACK_0_CB MCFG_AM9517A_OUT_DACK_0_CB
#define MCFG_I8237_OUT_DACK_1_CB MCFG_AM9517A_OUT_DACK_1_CB
#define MCFG_I8237_OUT_DACK_2_CB MCFG_AM9517A_OUT_DACK_2_CB
#define MCFG_I8237_OUT_DACK_3_CB MCFG_AM9517A_OUT_DACK_3_CB

#endif // MAME_MACHINE_AM9517_H
