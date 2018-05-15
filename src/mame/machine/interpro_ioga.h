// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_INTERPRO_IOGA_H
#define MAME_MACHINE_INTERPRO_IOGA_H

#pragma once

#define MCFG_INTERPRO_IOGA_NMI_CB(_out_nmi) \
	devcb = &downcast<interpro_ioga_device &>(*device).set_out_nmi_callback(DEVCB_##_out_nmi);

#define MCFG_INTERPRO_IOGA_IRQ_CB(_out_irq) \
	devcb = &downcast<interpro_ioga_device &>(*device).set_out_irq_callback(DEVCB_##_out_irq);

#define MCFG_INTERPRO_IOGA_IVEC_CB(_out_ivec) \
	devcb = &downcast<interpro_ioga_device &>(*device).set_out_ivec_callback(DEVCB_##_out_ivec);

#define MCFG_INTERPRO_IOGA_DMA_CB(_channel, _dma_r, _dma_w) \
	devcb = &downcast<interpro_ioga_device &>(*device).set_dma_r_callback(_channel, DEVCB_##_dma_r); \
	devcb = &downcast<interpro_ioga_device &>(*device).set_dma_w_callback(_channel, DEVCB_##_dma_w);

#define MCFG_INTERPRO_IOGA_SERIAL_DMA_CB(_channel, _dma_r, _dma_w) \
	devcb = &downcast<interpro_ioga_device &>(*device).set_serial_dma_r_callback(_channel, DEVCB_##_dma_r); \
	devcb = &downcast<interpro_ioga_device &>(*device).set_serial_dma_w_callback(_channel, DEVCB_##_dma_w);

#define MCFG_INTERPRO_IOGA_FDCTC_CB(_tc) \
	devcb = &downcast<interpro_ioga_device &>(*device).set_fdc_tc_callback(DEVCB_##_tc);

#define MCFG_INTERPRO_IOGA_ETH_CA_CB(_ca) \
	devcb = &downcast<interpro_ioga_device &>(*device).set_eth_ca_callback(DEVCB_##_ca);

#define MCFG_INTERPRO_IOGA_MEMORY(_tag, _spacenum) \
	downcast<interpro_ioga_device &>(*device).set_memory(_tag, _spacenum);

class interpro_ioga_device : public device_t
{
protected:
	interpro_ioga_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	enum interrupt_type
	{
		INT_HARD_IN, // hardware internal
		INT_HARD_EX, // hardware external
		INT_SOFT_LO, // soft low type
		INT_SOFT_HI  // soft high type
	};

	enum interrupt_number
	{
		IRQ_TIMER2   =  0, // internal int  3 (5c)
		IRQ_TIMER3   =  1, // internal int  4 (5e)
		IRQ_SCSI     =  2, // external int  0 (60)
		IRQ_FLOPPY   =  3, // external int  1 (62)
		IRQ_PLOTTER  =  4, // external int  2 (64)
		IRQ_SRXCBUS0 =  5, // external int  3 (66)
		IRQ_SRXCBUS1 =  6, // external int  4 (68)
		IRQ_SRXCBUS2 =  7, // external int  5 (6a)
		IRQ_VB       =  8, // external int  6 (6c)
		IRQ_9        =  9, // external int  7 (6e)
		IRQ_CBUS3    = 10, // external int  8 (70)
		IRQ_RTC      = 11, // external int  9 (72)
		IRQ_60HZ     = 12, // external int 10 (74)
		IRQ_MOUSE    = 13, // internal int  0 (76)
		IRQ_TIMER0   = 14, // internal int  1 (78)
		IRQ_TIMER1   = 15, // internal int  2 (7a)
		IRQ_SERDMA   = 16, // internal int  5 (7c)
		IRQ_SERIAL   = 17, // external int 11 (7e)
		IRQ_ETHERNET = 18, // external int 12 (80)

		// soft interrupts (low type)
		IRQ_SOFT0 = 0,
		IRQ_SOFT1 = 1,
		IRQ_SOFT2 = 2,
		IRQ_SOFT3 = 3,
		IRQ_SOFT4 = 4,
		IRQ_SOFT5 = 5,
		IRQ_SOFT6 = 6,
		IRQ_SOFT7 = 7,

		// soft interrupts (high type)
		IRQ_SOFT8  = 0,
		IRQ_SOFT9  = 1,
		IRQ_SOFT10 = 2,
		IRQ_SOFT11 = 3,
		IRQ_SOFT12 = 4,
		IRQ_SOFT13 = 5,
		IRQ_SOFT14 = 6,
		IRQ_SOFT15 = 7
	};

	struct interrupt_data_t
	{
		const interrupt_type type;
		const interrupt_number number;
		const u16 mask;

		const char *const name;
		const char *const source;
	};

	enum dma_channel
	{
		DMA_PLOTTER = 0,
		DMA_SCSI    = 1,
		DMA_FLOPPY  = 2
	};

public:
	template <class Object> devcb_base &set_out_nmi_callback(Object &&cb) { return m_out_nmi_func.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_irq_callback(Object &&cb) { return m_out_irq_func.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_out_ivec_callback(Object &&cb) { return m_out_ivec_func.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_dma_r_callback(int channel, Object &&cb) { return m_dma_channel[channel].device_r.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_dma_w_callback(int channel, Object &&cb) { return m_dma_channel[channel].device_w.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_serial_dma_r_callback(int channel, Object &&cb) { return m_serial_dma_channel[channel].device_r.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_serial_dma_w_callback(int channel, Object &&cb) { return m_serial_dma_channel[channel].device_w.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_fdc_tc_callback(Object &&cb) { return m_fdc_tc_func.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_eth_ca_callback(Object &&cb) { return m_eth_ca_func.set_callback(std::forward<Object>(cb)); }

	void set_memory(const char *const tag, const int spacenum)
	{
		m_memory_tag = tag;
		m_memory_spacenum = spacenum;
	}

	virtual void map(address_map &map) = 0;

	// interrupt request lines
	DECLARE_WRITE_LINE_MEMBER(ir0_w) { set_int_line(INT_HARD_EX, IRQ_SCSI, state); }
	DECLARE_WRITE_LINE_MEMBER(ir1_w) { set_int_line(INT_HARD_EX, IRQ_FLOPPY, state); }
	DECLARE_WRITE_LINE_MEMBER(ir2_w) { set_int_line(INT_HARD_EX, IRQ_PLOTTER, state); }
	DECLARE_WRITE_LINE_MEMBER(ir3_w) { set_int_line(INT_HARD_EX, IRQ_SRXCBUS0, state); }
	DECLARE_WRITE_LINE_MEMBER(ir4_w) { set_int_line(INT_HARD_EX, IRQ_SRXCBUS1, state); }
	DECLARE_WRITE_LINE_MEMBER(ir5_w) { set_int_line(INT_HARD_EX, IRQ_SRXCBUS2, state); }
	DECLARE_WRITE_LINE_MEMBER(ir6_w) { set_int_line(INT_HARD_EX, IRQ_VB, state); }
	DECLARE_WRITE_LINE_MEMBER(ir7_w) { set_int_line(INT_HARD_EX, IRQ_9, state); }
	DECLARE_WRITE_LINE_MEMBER(ir8_w) { set_int_line(INT_HARD_EX, IRQ_CBUS3, state); }
	// FIXME: mc146818 inverts the normal irq state convention
	DECLARE_WRITE_LINE_MEMBER(ir9_w) { set_int_line(INT_HARD_EX, IRQ_RTC, !state); }
	DECLARE_WRITE_LINE_MEMBER(ir10_w) { set_int_line(INT_HARD_EX, IRQ_60HZ, state); }
	DECLARE_WRITE_LINE_MEMBER(ir11_w) { set_int_line(INT_HARD_EX, IRQ_SERIAL, state); }
	DECLARE_WRITE_LINE_MEMBER(ir12_w) { set_int_line(INT_HARD_EX, IRQ_ETHERNET, state); }

	IRQ_CALLBACK_MEMBER(acknowledge_interrupt);

	// interrupt control
	enum icr_mask
	{
		IRQ_NUMBER          = 0x000f,
		IRQ_LEVEL           = 0x00f0,
		IRQ_VECTOR          = 0x00ff,
		IRQ_FLAGS           = 0xff00,

		IRQ_PENDING         = 0x0100,
		IRQ_ENABLE_EXTERNAL = 0x0200,
		IRQ_EDGE            = 0x0400,
		IRQ_NEGPOL          = 0x0800,
		IRQ_ENABLE_INTERNAL = 0x1000
	};
	DECLARE_READ16_MEMBER(icr_r) { return m_hwicr[offset]; }
	DECLARE_WRITE16_MEMBER(icr_w);
	DECLARE_READ16_MEMBER(icr18_r) { return icr_r(space, 18, mem_mask); }
	DECLARE_WRITE16_MEMBER(icr18_w) { icr_w(space, 18, data, mem_mask); }

	enum nmictrl_mask
	{
		NMI_ALL     = 0x01,
		NMI_ENABLE1 = 0x02,
		NMI_EDGE    = 0x04,
		NMI_NEGPOL  = 0x08,
		NMI_ENABLE2 = 0x10,

		NMI_IE      = NMI_ENABLE1 | NMI_ENABLE2
	};
	DECLARE_READ8_MEMBER(nmictrl_r) { return m_nmictrl; }
	DECLARE_WRITE8_MEMBER(nmictrl_w);

	DECLARE_READ8_MEMBER(softint_r) { return m_softint; }
	DECLARE_WRITE8_MEMBER(softint_w);
	DECLARE_READ16_MEMBER(softint_vector_r) { return m_swicr[offset]; }
	DECLARE_WRITE16_MEMBER(softint_vector_w);

	// dma request lines
	DECLARE_WRITE_LINE_MEMBER(drq_plotter) { drq(state, DMA_PLOTTER); }
	DECLARE_WRITE_LINE_MEMBER(drq_scsi) { drq(state, DMA_SCSI); }
	DECLARE_WRITE_LINE_MEMBER(drq_floppy) { drq(state, DMA_FLOPPY); }
	DECLARE_WRITE_LINE_MEMBER(drq_serial0) { serial_drq(state, 0); }
	DECLARE_WRITE_LINE_MEMBER(drq_serial1) { serial_drq(state, 1); }
	DECLARE_WRITE_LINE_MEMBER(drq_serial2) { serial_drq(state, 2); }

	// dma control
	enum dma_ctrl_mask
	{
		DMA_CTRL_TCZERO  = 0x00000001, // transfer count zero
		DMA_CTRL_TAG     = 0x00000e00, // bus tag
		DMA_CTRL_BERR    = 0x00400000, // bus error
		DMA_CTRL_ERR     = 0x00800000, // checked for in scsi isr

		DMA_CTRL_ENABLE  = 0x01000000, // transfer enabled
		DMA_CTRL_VIRTUAL = 0x02000000, // virtual address translation required
		DMA_CTRL_DOUBLE  = 0x04000000, // double transfer size (double or quad quad)

		DMA_CTRL_WRITE   = 0x40000000, // memory to device transfer
		DMA_CTRL_QUAD    = 0x80000000, // select quad transfer size (quad quad when combined with double)

		DMA_CTRL_WMASK   = 0xfd000e00  // writable fields (quad not writable for floppy/plotter?)
	};
	DECLARE_READ32_MEMBER(dma_plotter_r) { return dma_r(space, offset, mem_mask, DMA_PLOTTER); }
	DECLARE_WRITE32_MEMBER(dma_plotter_w) { dma_w(space, offset, data, mem_mask, DMA_PLOTTER); }
	DECLARE_READ32_MEMBER(dma_scsi_r) { return dma_r(space, offset, mem_mask, DMA_SCSI); }
	DECLARE_WRITE32_MEMBER(dma_scsi_w) { dma_w(space, offset, data, mem_mask, DMA_SCSI); }
	DECLARE_READ32_MEMBER(dma_floppy_r) { return dma_r(space, offset, mem_mask, DMA_FLOPPY); }
	DECLARE_WRITE32_MEMBER(dma_floppy_w) { dma_w(space, offset, data, mem_mask, DMA_FLOPPY); }

	DECLARE_READ32_MEMBER(serial_dma0_addr_r) { return serial_dma_addr_r(space, offset, mem_mask, 0); }
	DECLARE_WRITE32_MEMBER(serial_dma0_addr_w) { serial_dma_addr_w(space, offset, data, mem_mask, 0); }
	DECLARE_READ32_MEMBER(serial_dma0_ctrl_r) { return serial_dma_ctrl_r(space, offset, mem_mask, 0); }
	DECLARE_WRITE32_MEMBER(serial_dma0_ctrl_w) { serial_dma_ctrl_w(space, offset, data, mem_mask, 0); }

	DECLARE_READ32_MEMBER(serial_dma1_addr_r) { return serial_dma_addr_r(space, offset, mem_mask, 1); }
	DECLARE_WRITE32_MEMBER(serial_dma1_addr_w) { serial_dma_addr_w(space, offset, data, mem_mask, 1); }
	DECLARE_READ32_MEMBER(serial_dma1_ctrl_r) { return serial_dma_ctrl_r(space, offset, mem_mask, 1); }
	DECLARE_WRITE32_MEMBER(serial_dma1_ctrl_w) { serial_dma_ctrl_w(space, offset, data, mem_mask, 1); }

	DECLARE_READ32_MEMBER(serial_dma2_addr_r) { return serial_dma_addr_r(space, offset, mem_mask, 2); }
	DECLARE_WRITE32_MEMBER(serial_dma2_addr_w) { serial_dma_addr_w(space, offset, data, mem_mask, 2); }
	DECLARE_READ32_MEMBER(serial_dma2_ctrl_r) { return serial_dma_ctrl_r(space, offset, mem_mask, 2); }
	DECLARE_WRITE32_MEMBER(serial_dma2_ctrl_w) { serial_dma_ctrl_w(space, offset, data, mem_mask, 2); }

	DECLARE_READ32_MEMBER(dma_plotter_eosl_r) { return m_dma_plotter_eosl; }
	DECLARE_WRITE32_MEMBER(dma_plotter_eosl_w) { m_dma_plotter_eosl = data ^ 0xffff0000; }

	// bus arbitration and control
	enum arbctl_mask
	{
		ARBCTL_BGR_ETHC = 0x0001,
		ARBCTL_BGR_SCSI = 0x0002,
		ARBCTL_BGR_PLOT = 0x0004,
		ARBCTL_BGR_FDC  = 0x0008,
		ARBCTL_BGR_SER0 = 0x0010,
		ARBCTL_BGR_SER1 = 0x0020,
		ARBCTL_BGR_SER2 = 0x0040,
		ARBCTL_BGR_ETHB = 0x0080,
		ARBCTL_BGR_ETHA = 0x0100
	};
	DECLARE_READ16_MEMBER(arbctl_r) { return m_arbctl; }
	DECLARE_WRITE16_MEMBER(arbctl_w);

	enum error_businfo_mask
	{
		BINFO_CT     = 0x003f,
		BINFO_TAG    = 0x01c0,
		BINFO_BG     = 0x0e00,
		BINFO_BERR   = 0x1000,
		BINFO_MMBE   = 0x2000,
		BINFO_SNAPOK = 0x4000,
		BINFO_MSBE   = 0x8000
	};
	enum error_businfo_bg_mask
	{
		BINFO_BG_IOD      = 0x0200,
		BINFO_BG_ICAMMU   = 0x0400,
		BINFO_BG_DCAMMU   = 0x0800,
		BINFO_BG_SRMASTER = 0x0c00
	};
	DECLARE_READ32_MEMBER(error_businfo_r);
	DECLARE_READ32_MEMBER(error_address_r) { return m_error_address; }
	DECLARE_READ32_MEMBER(bus_timeout_r) { return m_bus_timeout; }
	DECLARE_WRITE32_MEMBER(bus_timeout_w) { m_bus_timeout = data; }

	DECLARE_WRITE32_MEMBER(bus_error) { m_error_address = data; m_error_businfo = offset; }

	// timers
	DECLARE_READ32_MEMBER(timer0_r);
	DECLARE_WRITE32_MEMBER(timer0_w);

	enum timer1_mask
	{
		TIMER1_COUNT   = 0x0000ffff,
		TIMER1_START   = 0x00010000,
		TIMER1_EXPIRED = 0x00020000
	};
	DECLARE_READ32_MEMBER(timer1_r);
	DECLARE_WRITE32_MEMBER(timer1_w);

	DECLARE_READ32_MEMBER(timer2_count_r);
	DECLARE_WRITE32_MEMBER(timer2_count_w);
	DECLARE_READ32_MEMBER(timer2_value_r);
	DECLARE_WRITE32_MEMBER(timer2_value_w);

	enum timer3_mask
	{
		TIMER3_COUNT   = 0x3fffffff,
		TIMER3_START   = 0x40000000,
		TIMER3_EXPIRED = 0x80000000
	};
	DECLARE_READ32_MEMBER(timer3_r);
	DECLARE_WRITE32_MEMBER(timer3_w);

	DECLARE_READ32_MEMBER(prescaler_r);
	DECLARE_WRITE32_MEMBER(prescaler_w);

	// mouse
	enum mouse_status_mask
	{
		MOUSE_YPOS    = 0x000000ff,
		MOUSE_XPOS    = 0x0000ff00,
		MOUSE_LBUTTON = 0x00010000,
		MOUSE_MBUTTON = 0x00020000,
		MOUSE_RBUTTON = 0x00040000,

		MOUSE_BUTTONS = 0x00070000
	};
	DECLARE_READ32_MEMBER(mouse_status_r);
	DECLARE_WRITE32_MEMBER(mouse_status_w);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	const char *m_memory_tag;
	int m_memory_spacenum;
	address_space *m_memory_space;

	// callbacks
	devcb_write_line m_out_nmi_func;
	devcb_write_line m_out_irq_func;
	devcb_write8 m_out_ivec_func;
	devcb_write_line m_fdc_tc_func;
	devcb_write_line m_eth_ca_func;

	void set_nmi_line(int state);
	void set_int_line(interrupt_type type, int number, int state);

	TIMER_CALLBACK_MEMBER(interrupt_check);
	TIMER_CALLBACK_MEMBER(set_ivec);
	TIMER_CALLBACK_MEMBER(dma);
	TIMER_CALLBACK_MEMBER(serial_dma);
	TIMER_CALLBACK_MEMBER(timer0);
	TIMER_CALLBACK_MEMBER(timer1);
	TIMER_CALLBACK_MEMBER(timer2) {}
	TIMER_CALLBACK_MEMBER(timer3);
	TIMER_CALLBACK_MEMBER(timer_60hz);
	TIMER_CALLBACK_MEMBER(mouse_timer);

	virtual TIMER_CALLBACK_MEMBER(eth_reset) = 0;
	emu_timer *m_eth_reset_timer;

private:
	bool nmi(int state);
	bool irq(int state, u8 ivec);
	u16 get_icr(interrupt_type type, int number) const;
	void set_pending(interrupt_type type, int number, bool pending);

	void drq(int state, int channel);
	void serial_drq(int state, int channel);

	u32 dma_r(address_space &space, offs_t offset, u32 mem_mask, dma_channel channel) const;
	void dma_w(address_space &space, offs_t offset, u32 data, u32 mem_mask, dma_channel channel);

	enum serial_dma_ctrl_mask
	{
		SDMA_COUNT   = 0x0000ffff,
		SDMA_TCZERO  = 0x00200000,
		SDMA_SEND    = 0x04000000, // transfer from memory to device
		SDMA_CONTROL = 0xffff0000,
	};
	u32 serial_dma_addr_r(address_space &space, offs_t offset, u32 mem_mask, int channel) const { return m_serial_dma_channel[channel].address; }
	void serial_dma_addr_w(address_space &space, offs_t offset, u32 data, u32 mem_mask, int channel);
	u32 serial_dma_ctrl_r(address_space &space, offs_t offset, u32 mem_mask, int channel) const { return m_serial_dma_channel[channel].control; }
	void serial_dma_ctrl_w(address_space &space, offs_t offset, u32 data, u32 mem_mask, int channel);

	// interrupt state
	static const interrupt_data_t m_interrupt_data[];
	interrupt_data_t const *m_active_interrupt;
	int m_nmi_state;
	int m_irq_state;
	u8 m_ivec;
	u32 m_hwint_forced;
	u32 m_line_state;

	// interrupt control
	static const int INTERRUPT_COUNT = 19;
	u16 m_hwicr[INTERRUPT_COUNT];
	u8 m_softint;
	u8 m_nmictrl;
	u16 m_swicr[8];

	// dma state
	static const int DMA_CHANNEL_COUNT = 3;
	struct dma_channel_t
	{
		u32 real_address;
		u32 virtual_address;
		u32 transfer_count;
		u32 control;

		int drq_state;
		devcb_read8 device_r;
		devcb_write8 device_w;

		const u16 arb_mask;
		const dma_channel channel;
		const char *const name;
	} m_dma_channel[DMA_CHANNEL_COUNT];
	u32 m_dma_plotter_eosl;

	// serial dma state
	static const int SERIAL_DMA_CHANNEL_COUNT = 3;
	struct serial_dma_channel_t
	{
		u32 address;
		u32 control;

		int drq_state;
		devcb_read8 device_r;
		devcb_write8 device_w;

		const u16 arb_mask;
		const int channel;
		const char *const name;
	} m_serial_dma_channel[SERIAL_DMA_CHANNEL_COUNT];

	// timers
	u8 m_timer0_count;
	u16 m_timer1_count;
	u32 m_timer2_count;
	u32 m_timer2_value;
	u32 m_timer3_count;
	u32 m_prescaler;

	emu_timer *m_interrupt_timer;
	emu_timer *m_dma_timer;
	emu_timer *m_serial_dma_timer;
	emu_timer *m_timer0;
	emu_timer *m_timer1;
	emu_timer *m_timer2;
	emu_timer *m_timer3;
	emu_timer *m_timer_60hz;

	// bus arbitration and control
	u16 m_arbctl;
	u32 m_error_address;
	u32 m_error_businfo;
	u32 m_bus_timeout;

	// mouse
	u32 m_mouse_status;
	emu_timer *m_mouse_timer;
};

class turquoise_ioga_device : public interpro_ioga_device
{
public:
	turquoise_ioga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;

	DECLARE_WRITE16_MEMBER(eth_w);
	DECLARE_READ16_MEMBER(eth_r);

protected:
	virtual TIMER_CALLBACK_MEMBER(eth_reset) override;

	enum eth_base_mask
	{
		ETH_BASE_MASK = 0xffe00000
	};
	DECLARE_READ32_MEMBER(eth_base_r) { return m_eth_base; }
	DECLARE_WRITE32_MEMBER(eth_base_w);

	enum eth_control_mask
	{
		ETH_CA    = 0x0001, // channel attention
		ETH_FLUSH = 0x0002,
		ETH_BUF   = 0x0004,
		ETH_QUAD  = 0x0008,
		ETH_BERR  = 0x0010, // bus error
		ETH_PERR  = 0x0020, // parity error
		ETH_RESET = 0x0040,
		ETH_WTAG  = 0x0600,
		ETH_RTAG  = 0x3000,

		//ETH_MASK     = 0x4ff2  // channel attention and error bits not persistent
	};
	DECLARE_READ16_MEMBER(eth_control_r) { return m_eth_control; }
	DECLARE_WRITE16_MEMBER(eth_control_w);

private:
	u32 m_eth_base;
	u16 m_eth_control;
};

class sapphire_ioga_device : public interpro_ioga_device
{
public:
	sapphire_ioga_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void map(address_map &map) override;

	DECLARE_WRITE16_MEMBER(eth_w);
	DECLARE_READ16_MEMBER(eth_r);

protected:
	virtual TIMER_CALLBACK_MEMBER(eth_reset) override;

	enum eth_remap_mask
	{
		ETH_CHA_FLUSH  = 0x00000008,
		ETH_CHA_BUF    = 0x00000010,
		ETH_CHA_QUAD   = 0x00000020,
		ETH_CHA_WTAG   = 0x000001c0,
		ETH_CHA_RTAG   = 0x00000e00,
		ETH_REMAP_ADDR = 0xfffff000
	};
	DECLARE_READ32_MEMBER(eth_remap_r) { return m_eth_remap; }
	DECLARE_WRITE32_MEMBER(eth_remap_w);

	enum eth_mappg_mask
	{
		ETH_CHB_FLUSH = 0x00000008,
		ETH_CHB_BUF   = 0x00000010,
		ETH_CHB_QUAD  = 0x00000020,
		ETH_CHB_WTAG  = 0x000001c0,
		ETH_CHB_RTAG  = 0x00000e00,
		ETH_MAPPG     = 0xfffff000
	};
	DECLARE_READ32_MEMBER(eth_mappg_r) { return m_eth_mappg; }
	DECLARE_WRITE32_MEMBER(eth_mappg_w);

	enum eth_control_mask
	{
		ETH_CA       = 0x00000001, // channel attention
		ETH_MAPEN    = 0x00000002,
		ETH_CHC_BUF  = 0x00000010,
		ETH_CHC_QUAD = 0x00000020,
		ETH_CHC_WTAG = 0x000001c0,
		ETH_CHC_RTAG = 0x00000e00,
		ETH_BERR     = 0x00001000,
		ETH_MMBE     = 0x00002000,
		ETH_RESET    = 0x00004000,

		ETH_MASK     = 0x00004ff2  // channel attention and error bits not persistent
	};
	DECLARE_READ32_MEMBER(eth_control_r);
	DECLARE_WRITE32_MEMBER(eth_control_w);

private:
	u32 m_eth_remap;
	u32 m_eth_mappg;
	u32 m_eth_control;
};

// device type definition
DECLARE_DEVICE_TYPE(TURQUOISE_IOGA, turquoise_ioga_device)
DECLARE_DEVICE_TYPE(SAPPHIRE_IOGA, sapphire_ioga_device)

#endif // MAME_MACHINE_INTERPRO_IOGA_H
