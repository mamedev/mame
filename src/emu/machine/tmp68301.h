#ifndef TMP68301_H
#define TMP68301_H

class tmp68301_device : public device_t
{
public:
	tmp68301_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~tmp68301_device() {}

	// Hardware Registers
	DECLARE_READ16_MEMBER( regs_r );
	DECLARE_WRITE16_MEMBER( regs_w );

	// Interrupts
	void external_interrupt_0();
	void external_interrupt_1();
	void external_interrupt_2();

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	
private:
	// internal state
	UINT16 m_regs[0x400];

	UINT8 m_IE[3];        // 3 External Interrupt Lines
	emu_timer *m_tmp68301_timer[3];        // 3 Timers

	int m_irq_vector[8];
	
	TIMER_CALLBACK_MEMBER( timer_callback );
	void update_timer( int i );
	IRQ_CALLBACK_MEMBER(irq_callback);
	void update_irq_state();
};

extern const device_type TMP68301;

#define MCFG_TMP68301_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, TMP68301, 0)

#endif
