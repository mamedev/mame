// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
#include "sound/ym2151.h"
#include "cpu/z80/z80.h"

class t5182_device : public device_t

{
public:
	t5182_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
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

	enum
	{
		SETIRQ_CB
	};

	void sound_irq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sharedram_semaphore_snd_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sharedram_semaphore_main_acquire_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sharedram_semaphore_main_release_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sharedram_semaphore_snd_acquire_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sharedram_semaphore_snd_release_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t sharedram_semaphore_main_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t sharedram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void sharedram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ym2151_irq_handler(int state);
	void ym2151_irq_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void cpu_irq_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

private:
	// internal state
	required_device<cpu_device> m_ourcpu;
	required_shared_ptr<uint8_t> m_sharedram;
	int m_irqstate;
	int m_semaphore_main;
	int m_semaphore_snd;
	emu_timer *m_setirq_cb;
	void setirq_callback(void *ptr, int32_t param);
};

extern const device_type T5182;
