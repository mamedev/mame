#ifndef TMP68301_H
#define TMP68301_H

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

/* TODO: frequency & hook it up with m68k */
#define MCFG_TMP68301_ADD(_tag, _config) \
	MCFG_DEVICE_ADD(_tag, TMP68301, 0) \
	MCFG_DEVICE_CONFIG(_config)

#define MCFG_TMP68301_MODIFY(_tag, _config) \
	MCFG_DEVICE_MODIFY(_tag) \
	MCFG_DEVICE_CONFIG(_config)


#define TMP68301_INTERFACE(name) \
	const tmp68301_interface (name) =


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mb_vcu_interface

struct tmp68301_interface
{
	devcb_read16         m_in_parallel_cb;
	devcb_write16        m_out_parallel_cb;
// 	TODO: serial ports
};

class tmp68301_device : public device_t,
						public device_memory_interface,
						public tmp68301_interface
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

	DECLARE_READ16_MEMBER(imr_r);
	DECLARE_WRITE16_MEMBER(imr_w);
	DECLARE_READ16_MEMBER(iisr_r);
	DECLARE_WRITE16_MEMBER(iisr_w);
	DECLARE_READ16_MEMBER(scr_r);
	DECLARE_WRITE16_MEMBER(scr_w);
	DECLARE_READ16_MEMBER(pdr_r);
	DECLARE_WRITE16_MEMBER(pdr_w);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;

	devcb_resolved_read16         m_in_parallel_func;
	devcb_resolved_write16        m_out_parallel_func;
private:
	// internal state
	UINT16 m_regs[0x400];

	UINT8 m_IE[3];        // 3 External Interrupt Lines
	emu_timer *m_tmp68301_timer[3];        // 3 Timers

	UINT16 m_irq_vector[8];

	TIMER_CALLBACK_MEMBER( timer_callback );
	void update_timer( int i );
	IRQ_CALLBACK_MEMBER(irq_callback);
	void update_irq_state();

	UINT16 m_imr;
	UINT16 m_iisr;
	UINT16 m_scr;

	inline UINT16 read_word(offs_t address);
	inline void write_word(offs_t address, UINT16 data);
	const address_space_config      m_space_config;
};

extern const device_type TMP68301;

#endif
