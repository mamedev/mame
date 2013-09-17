#include "sound/2151intf.h"
#include "cpu/z80/z80.h"

#define T5182COINPORT "T5182_COIN"

class t5182_device : public device_t

{
public:
	t5182_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~t5182_device() {}

	enum
	{
		VECTOR_INIT,
		YM2151_ASSERT,
		YM2151_CLEAR,
		YM2151_ACK,
		CPU_ASSERT,
		CPU_CLEAR
	};

	DECLARE_WRITE8_MEMBER(sound_irq_w );
	DECLARE_READ8_MEMBER(sharedram_semaphore_snd_r);
	DECLARE_WRITE8_MEMBER(sharedram_semaphore_main_acquire_w);
	DECLARE_WRITE8_MEMBER(sharedram_semaphore_main_release_w);
	DECLARE_WRITE8_MEMBER(sharedram_semaphore_snd_acquire_w);
	DECLARE_WRITE8_MEMBER(sharedram_semaphore_snd_release_w);
	DECLARE_READ8_MEMBER(sharedram_semaphore_main_r);
	DECLARE_READ8_MEMBER(sharedram_r);
	DECLARE_WRITE8_MEMBER(sharedram_w);
	DECLARE_WRITE_LINE_MEMBER(ym2151_irq_handler);
	DECLARE_WRITE8_MEMBER(ym2151_irq_ack_w);
	DECLARE_WRITE8_MEMBER(cpu_irq_ack_w);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

private:
	// internal state
	cpu_device *m_ourcpu;
	UINT8 *m_t5182_sharedram;
	int m_irqstate;
	int m_semaphore_main;
	int m_semaphore_snd;

	TIMER_CALLBACK_MEMBER( setirq_callback );
};

extern const device_type T5182;

ADDRESS_MAP_EXTERN( t5182_map, 8 );
ADDRESS_MAP_EXTERN( t5182_io, 8 );

MACHINE_CONFIG_EXTERN( t5182 );

#define MCFG_T5182_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, T5182, 0)
