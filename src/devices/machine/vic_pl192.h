// license:BSD-3-Clause
// copyright-holders:Devin Acker, Melissa Goad

// ARM PrimeCell PL910/PL192 VIC emulation

#ifndef MAME_MACHINE_VIC_PL192_H
#define MAME_MACHINE_VIC_PL192_H

class vic_pl190_device : public device_t, public device_memory_interface
{
public:
	vic_pl190_device(const machine_config &mconfig, const char* tag, device_t *owner, uint32_t clock = 0);

	auto out_irq_cb() { return m_out_irq_func.bind(); }
	auto out_fiq_cb() { return m_out_fiq_func.bind(); }

	template<unsigned IRQ>
	void irq_w(int state) { set_irq_line(IRQ, state); }

	void map(address_map &map) ATTR_COLD;

	u32 irq_status_r();
	u32 fiq_status_r();
	u32 raw_intr_r();

	u32 int_select_r();
	void int_select_w(u32 data);

	u32 int_enable_r();
	void int_enable_w(u32 data);
	void int_en_clear_w(u32 data);

	u32 soft_int_r();
	void soft_int_w(u32 data);
	void soft_int_clear_w(u32 data);

	u8 protection_r();
	void protection_w(u8 data);

	u32 cur_vect_addr_r();
	void cur_vect_addr_w(u32 data);

	u32 def_vect_addr_r();
	void def_vect_addr_w(u32 data);

	u32 vect_addr_r(offs_t offset);
	void vect_addr_w(offs_t offset, u32 data);

	u32 vect_ctl_r(offs_t offset);
	void vect_ctl_w(offs_t offset, u32 data);

protected:
	vic_pl190_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual space_config_vector memory_space_config() const override;

	virtual void update_vector();

	TIMER_CALLBACK_MEMBER(irq_sync_tick);

	u8 num_vectors; // number of available vectored interrupts

	u32 raw_intr, intr_select, intr_en, soft_intr;
	u32 vectaddr[32], vectctl[32], defaddress, vicaddress;
	int protection;

	u32 priority_mask; // mask for interrupts which can take priority over the current one
	u8 priority; // priority level of the current interrupt, if any
	u8 periph_id[4], pcell_id[4];

	emu_timer *m_irq_sync_timer;

private:
	void set_irq_line(int irq, int state);

	address_space_config m_mmio_config;

	devcb_write_line m_out_irq_func;
	devcb_write_line m_out_fiq_func;
};

class vic_upd800468_device : public vic_pl190_device
{
public:
	vic_upd800468_device(const machine_config &mconfig, const char* tag, device_t *owner, uint32_t clock = 0);

	void map(address_map &map) ATTR_COLD;

	void int_clear_w(u32 data);
};

class vic_pl192_device : public vic_pl190_device
{
public:
	vic_pl192_device(const machine_config &mconfig, const char* tag, device_t *owner, uint32_t clock = 0);

	void map(address_map &map) ATTR_COLD;

	u16 sw_priority_r();
	void sw_priority_w(u16 data);

	u8 daisy_priority_r();
	void daisy_priority_w(u8 data);

protected:

	virtual void update_vector() override;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	u16 sw_priority_mask;
	u8 daisy_priority;
};

DECLARE_DEVICE_TYPE(PL190_VIC, vic_pl190_device)
DECLARE_DEVICE_TYPE(UPD800468_VIC, vic_upd800468_device)
DECLARE_DEVICE_TYPE(PL192_VIC, vic_pl192_device)

#endif // MAME_MACHINE_VIC_PL192_H
