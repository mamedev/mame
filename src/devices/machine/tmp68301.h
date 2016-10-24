// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef TMP68301_H
#define TMP68301_H

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

/* TODO: serial ports, frequency & hook it up with m68k */
#define MCFG_TMP68301_IN_PARALLEL_CB(_devcb) \
	devcb = &tmp68301_device::set_in_parallel_callback(*device, DEVCB_##_devcb);

#define MCFG_TMP68301_OUT_PARALLEL_CB(_devcb) \
	devcb = &tmp68301_device::set_out_parallel_callback(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************



class tmp68301_device : public device_t,
						public device_memory_interface
{
public:
	tmp68301_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~tmp68301_device() {}

	template<class _Object> static devcb_base &set_in_parallel_callback(device_t &device, _Object object) { return downcast<tmp68301_device &>(device).m_in_parallel_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_parallel_callback(device_t &device, _Object object) { return downcast<tmp68301_device &>(device).m_out_parallel_cb.set_callback(object); }

	// Hardware Registers
	uint16_t regs_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void regs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	// Interrupts
	void external_interrupt_0();
	void external_interrupt_1();
	void external_interrupt_2();

	uint16_t imr_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void imr_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t iisr_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void iisr_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t scr_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void scr_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t pdr_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void pdr_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t pdir_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void pdir_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	int irq_callback(device_t &device, int irqline);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

private:
	devcb_read16         m_in_parallel_cb;
	devcb_write16        m_out_parallel_cb;

	// internal state
	uint16_t m_regs[0x400];

	uint8_t m_IE[3];        // 3 External Interrupt Lines
	emu_timer *m_tmp68301_timer[3];        // 3 Timers

	uint16_t m_irq_vector[8];

	void timer_callback(void *ptr, int32_t param);
	void update_timer( int i );
	void update_irq_state();

	uint16_t m_imr;
	uint16_t m_iisr;
	uint16_t m_scr;
	uint16_t m_pdir;
	uint16_t m_pdr;

	inline uint16_t read_word(offs_t address);
	inline void write_word(offs_t address, uint16_t data);
	const address_space_config      m_space_config;
};

extern const device_type TMP68301;

#endif
