// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Atari CAGE Audio Board

****************************************************************************/

#ifndef __ATARI_CAGE__
#define __ATARI_CAGE__

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
	atari_cage_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	atari_cage_device(const machine_config &mconfig, device_type type, std::string name, std::string tag, device_t *owner, UINT32 clock, std::string shortname, std::string source);

	static void static_set_speedup(device_t &device, offs_t speedup) { downcast<atari_cage_device &>(device).m_speedup = speedup; }
	template<class _Object> static devcb_base &set_irqhandler_callback(device_t &device, _Object object) { return downcast<atari_cage_device &>(device).m_irqhandler.set_callback(object); }

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

	void reset_w(int state);

	UINT16 main_r();
	void main_w(UINT16 data);

	UINT16 control_r();
	void control_w(UINT16 data);

	TIMER_DEVICE_CALLBACK_MEMBER( dma_timer_callback );
	void update_dma_state(address_space &space);
	TIMER_DEVICE_CALLBACK_MEMBER( cage_timer_callback );
	void update_timer(int which);
	void update_serial();
	READ32_MEMBER( tms32031_io_r );
	WRITE32_MEMBER( tms32031_io_w );
	void update_control_lines();
	READ32_MEMBER( cage_from_main_r );
	WRITE32_MEMBER( cage_from_main_ack_w );
	WRITE32_MEMBER( cage_to_main_w );
	READ32_MEMBER( cage_io_status_r );
	TIMER_CALLBACK_MEMBER( cage_deferred_w );
	WRITE32_MEMBER( speedup_w );

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	cpu_device *m_cpu;
	attotime m_cpu_h1_clock_period;

	UINT8 m_cpu_to_cage_ready;
	UINT8 m_cage_to_cpu_ready;

	devcb_write8 m_irqhandler;


	attotime m_serial_period_per_word;

	UINT8 m_dma_enabled;
	UINT8 m_dma_timer_enabled;
	timer_device *m_dma_timer;

	UINT8 m_timer_enabled[2];
	timer_device *m_timer[2];

	UINT32 m_tms32031_io_regs[0x100];
	UINT16 m_from_main;
	UINT16 m_control;

	UINT32 *m_speedup_ram;
	dmadac_sound_device *m_dmadac[4];

	offs_t m_speedup;
};


// device type definition
extern const device_type ATARI_CAGE;

class atari_cage_seattle_device : public atari_cage_device
{
public:
	// construction/destruction
	atari_cage_seattle_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

};

// device type definition
extern const device_type ATARI_CAGE_SEATTLE;

#endif // __ATARI_CAGE__
