// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari CAGE Audio Board

****************************************************************************/

#ifndef __ATARI_CAGE__
#define __ATARI_CAGE__

#include "machine/gen_latch.h"
#include "sound/dmadac.h"

#define CAGE_IRQ_REASON_DATA_READY      (1)
#define CAGE_IRQ_REASON_BUFFER_EMPTY    (2)

#define MCFG_ATARI_CAGE_IRQ_CALLBACK(_write) \
	devcb = &atari_cage_device::set_irqhandler_callback(*device, DEVCB_##_write);

#define MCFG_ATARI_CAGE_SPEEDUP(_speedup) \
	atari_cage_device::static_set_speedup(*device, _speedup);

class atari_cage_device : public device_t
{
public:
	// construction/destruction
	atari_cage_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	atari_cage_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	static void static_set_speedup(device_t &device, offs_t speedup) { downcast<atari_cage_device &>(device).m_speedup = speedup; }
	template<class _Object> static devcb_base &set_irqhandler_callback(device_t &device, _Object object) { return downcast<atari_cage_device &>(device).m_irqhandler.set_callback(object); }

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	void reset_w(int state);

	uint16_t main_r();
	void main_w(uint16_t data);

	uint16_t control_r();
	void control_w(uint16_t data);

	void dma_timer_callback(timer_device &timer, void *ptr, int32_t param);
	void update_dma_state(address_space &space);
	void cage_timer_callback(timer_device &timer, void *ptr, int32_t param);
	void update_timer(int which);
	void update_serial();
	uint32_t tms32031_io_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void tms32031_io_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	void update_control_lines();
	uint32_t cage_from_main_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void cage_from_main_ack_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	void cage_to_main_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);
	uint32_t cage_io_status_r(address_space &space, offs_t offset, uint32_t mem_mask);
	void cage_deferred_w(void *ptr, int32_t param);
	void speedup_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	required_shared_ptr<uint32_t> m_cageram;
	cpu_device *m_cpu;
	required_device<generic_latch_16_device> m_soundlatch;
	attotime m_cpu_h1_clock_period;

	uint8_t m_cpu_to_cage_ready;
	uint8_t m_cage_to_cpu_ready;

	devcb_write8 m_irqhandler;


	attotime m_serial_period_per_word;

	uint8_t m_dma_enabled;
	uint8_t m_dma_timer_enabled;
	timer_device *m_dma_timer;

	uint8_t m_timer_enabled[2];
	timer_device *m_timer[2];

	uint32_t m_tms32031_io_regs[0x100];
	uint16_t m_from_main;
	uint16_t m_control;

	uint32_t *m_speedup_ram;
	dmadac_sound_device *m_dmadac[4];

	offs_t m_speedup;
};


// device type definition
extern const device_type ATARI_CAGE;

class atari_cage_seattle_device : public atari_cage_device
{
public:
	// construction/destruction
	atari_cage_seattle_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

};

// device type definition
extern const device_type ATARI_CAGE_SEATTLE;

#endif // __ATARI_CAGE__
