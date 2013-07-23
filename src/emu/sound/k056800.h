/*********************************************************

    Konami 056800 MIRAC sound interface

*********************************************************/

#ifndef __K056800_H__
#define __K056800_H__


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*k056800_irq_cb)(running_machine &, int);


struct k056800_interface
{
	k056800_irq_cb       m_irq_cb;
};

class k056800_device : public device_t,
						public k056800_interface
{
public:
	k056800_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~k056800_device() {}

	enum
	{
		TIMER_TICK_SOUND_CPU
	};

	DECLARE_READ32_MEMBER( host_r );
	DECLARE_WRITE32_MEMBER( host_w );
	DECLARE_READ16_MEMBER( sound_r );
	DECLARE_WRITE16_MEMBER( sound_w );

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	// internal state
	UINT8                m_host_reg[8];
	UINT8                m_sound_reg[8];
	emu_timer            *m_sound_cpu_timer;
	UINT8                m_sound_cpu_irq1_enable;
	k056800_irq_cb       m_irq_cb_func;

	UINT8 host_reg_r( int reg );
	void host_reg_w( int reg, UINT8 data );
	UINT8 sound_reg_r( int reg );
	void sound_reg_w( int reg, UINT8 data );

};

extern const device_type K056800;



/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_K056800_ADD(_tag, _interface, _clock) \
	MCFG_DEVICE_ADD(_tag, K056800, _clock) \
	MCFG_DEVICE_CONFIG(_interface)


#endif /* __K056800_H__ */
