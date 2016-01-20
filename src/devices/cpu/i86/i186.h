// license:BSD-3-Clause
// copyright-holders:Carl
#ifndef I186_H__
#define I186_H__

#include "emu.h"
#include "i86.h"

extern const device_type I80186;
extern const device_type I80188;

class i80186_cpu_device : public i8086_common_cpu_device
{
public:
	// construction/destruction
	i80186_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	i80186_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, int data_bus_size);

	template<class _Object> static devcb_base &static_set_read_slave_ack_callback(device_t &device, _Object object) { return downcast<i80186_cpu_device &>(device).m_read_slave_ack_func.set_callback(object); }
	template<class _Object> static devcb_base &static_set_chip_select_callback(device_t &device, _Object object) { return downcast<i80186_cpu_device &>(device).m_out_chip_select_func.set_callback(object); }
	template<class _Object> static devcb_base &static_set_tmrout0_handler(device_t &device, _Object object) { return downcast<i80186_cpu_device &>(device).m_out_tmrout0_func.set_callback(object); }
	template<class _Object> static devcb_base &static_set_tmrout1_handler(device_t &device, _Object object) { return downcast<i80186_cpu_device &>(device).m_out_tmrout1_func.set_callback(object); }

	IRQ_CALLBACK_MEMBER(int_callback);
	DECLARE_WRITE_LINE_MEMBER(drq0_w) { if(state) drq_callback(0); m_dma[0].drq_state = state; }
	DECLARE_WRITE_LINE_MEMBER(drq1_w) { if(state) drq_callback(1); m_dma[1].drq_state = state; }
	DECLARE_WRITE_LINE_MEMBER(tmrin0_w) { if(state && (m_timer[0].control & 0x8004) == 0x8004) { inc_timer(0); } }
	DECLARE_WRITE_LINE_MEMBER(tmrin1_w) { if(state && (m_timer[1].control & 0x8004) == 0x8004) { inc_timer(1); } }
	DECLARE_WRITE_LINE_MEMBER(int0_w) { external_int(0, state); }
	DECLARE_WRITE_LINE_MEMBER(int1_w) { external_int(1, state); }
	DECLARE_WRITE_LINE_MEMBER(int2_w) { external_int(2, state); }
	DECLARE_WRITE_LINE_MEMBER(int3_w) { external_int(3, state); }

	// device_memory_interface overrides
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override { return (spacenum == AS_PROGRAM) ? &m_program_config : ( (spacenum == AS_IO) ? &m_io_config : nullptr ); }

protected:
	// device_execute_interface overrides
	virtual UINT64 execute_clocks_to_cycles(UINT64 clocks) const override { return (clocks / 2); }
	virtual UINT64 execute_cycles_to_clocks(UINT64 cycles) const override { return (cycles * 2); }
	virtual void execute_run() override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual UINT32 execute_input_lines() const override { return 1; }
	virtual UINT8 fetch_op() override;
	virtual UINT8 fetch() override;
	UINT32 pc() { return m_pc = (m_sregs[CS] << 4) + m_ip; }

	virtual UINT8 read_port_byte(UINT16 port) override;
	virtual UINT16 read_port_word(UINT16 port) override;
	virtual void write_port_byte(UINT16 port, UINT8 data) override;
	virtual void write_port_word(UINT16 port, UINT16 data) override;

	static const UINT8 m_i80186_timing[200];

private:
	void update_interrupt_state();
	void handle_eoi(int data);
	void external_int(UINT16 intno, int state);
	void internal_timer_sync(int which);
	void internal_timer_update(int which, int new_count, int new_maxA, int new_maxB, int new_control);
	void update_dma_control(int which, int new_control);
	void drq_callback(int which);
	void inc_timer(int which);
	DECLARE_READ16_MEMBER(internal_port_r);
	DECLARE_WRITE16_MEMBER(internal_port_w);

	struct mem_state
	{
		UINT16      lower;
		UINT16      upper;
		UINT16      middle;
		UINT16      middle_size;
		UINT16      peripheral;
	};

	struct timer_state
	{
		UINT16      control;
		UINT16      maxA;
		UINT16      maxB;
		bool        active_count;
		UINT16      count;
		emu_timer   *int_timer;
	};

	struct dma_state
	{
		bool        drq_state;
		UINT32      source;
		UINT32      dest;
		UINT16      count;
		UINT16      control;
	};

	struct intr_state
	{
		UINT8       pending;
		UINT16      ack_mask;
		UINT16      priority_mask;
		UINT16      in_service;
		UINT16      request;
		UINT16      status;
		UINT16      poll_status;
		UINT16      timer;
		UINT16      dma[2];
		UINT16      ext[4];
		UINT8       ext_state;
	};

	timer_state     m_timer[3];
	dma_state       m_dma[2];
	intr_state      m_intr;
	mem_state       m_mem;

	static const device_timer_id TIMER_INT0 = 0;
	static const device_timer_id TIMER_INT1 = 1;
	static const device_timer_id TIMER_INT2 = 2;

	UINT16 m_reloc;

	address_space_config m_program_config;
	address_space_config m_io_config;

	devcb_read8 m_read_slave_ack_func;
	devcb_write16 m_out_chip_select_func;
	devcb_write_line m_out_tmrout0_func;
	devcb_write_line m_out_tmrout1_func;
};

class i80188_cpu_device : public i80186_cpu_device
{
public:
	// construction/destruction
	i80188_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

#define MCFG_80186_IRQ_SLAVE_ACK(_devcb) \
		devcb = &i80186_cpu_device::static_set_read_slave_ack_callback(*device, DEVCB_##_devcb);

#define MCFG_80186_CHIP_SELECT_CB(_devcb) \
		devcb = &i80186_cpu_device::static_set_chip_select_callback(*device, DEVCB_##_devcb);

#define MCFG_80186_TMROUT0_HANDLER(_devcb) \
		devcb = &i80186_cpu_device::static_set_tmrout0_handler(*device, DEVCB_##_devcb);

#define MCFG_80186_TMROUT1_HANDLER(_devcb) \
		devcb = &i80186_cpu_device::static_set_tmrout1_handler(*device, DEVCB_##_devcb);

#endif
