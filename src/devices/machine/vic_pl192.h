// license:BSD-3-Clause
// copyright-holders:Melissa Goad

// ARM PrimeCell PL192 VIC emulation

#ifndef MAME_MACHINE_VIC_PL192_H
#define MAME_MACHINE_VIC_PL192_H

class vic_pl192_device : public device_t
{
public:
	vic_pl192_device(const machine_config &mconfig, const char* tag, device_t *owner, uint32_t clock = 0);
	
	auto out_irq_cb() { return m_out_irq_func.bind(); }
	auto out_fiq_cb() { return m_out_fiq_func.bind(); }

	uint32_t read(offs_t offset);
	void write(offs_t offset, uint32_t data);
	
	template<unsigned IRQ>
	DECLARE_WRITE_LINE_MEMBER( irq_w ) { set_irq_line(IRQ, state); }

protected:
	vic_pl192_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	void set_irq_line(int irq, int state);

	devcb_write_line m_out_irq_func;
	devcb_write_line m_out_fiq_func;
	u32 raw_intr, intr_select, intr_en, soft_intr, vectaddr[32], vicaddress;
	int protection;
	u16 sw_priority_mask;
	u8 daisy_priority, vectprio[32];
	u8 periph_id[4], pcell_id[4];
};

DECLARE_DEVICE_TYPE(PL192_VIC, vic_pl192_device)

#endif // MAME_MACHINE_VIC_PL192_H