// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 *  Data East Pinball CPU boards
 *
 *  Created on: 10/07/2013
 */

#ifndef DECOPINCPU_H_
#define DECOPINCPU_H_

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/6821pia.h"
#include "machine/nvram.h"

#define MCFG_DECOCPU_TYPE1_ADD(_tag, _clock, _region) \
	MCFG_DEVICE_ADD(_tag, DECOCPU1, _clock) \
	decocpu_type1_device::static_set_cpuregion(*device, _region);

#define MCFG_DECOCPU_TYPE2_ADD(_tag, _clock, _region) \
	MCFG_DEVICE_ADD(_tag, DECOCPU2, _clock) \
	decocpu_type1_device::static_set_cpuregion(*device, _region);

#define MCFG_DECOCPU_TYPE3_ADD(_tag, _clock, _region) \
	MCFG_DEVICE_ADD(_tag, DECOCPU3, _clock) \
	decocpu_type1_device::static_set_cpuregion(*device, _region);

#define MCFG_DECOCPU_TYPE3B_ADD(_tag, _clock, _region) \
	MCFG_DEVICE_ADD(_tag, DECOCPU3B, _clock) \
	decocpu_type1_device::static_set_cpuregion(*device, _region);

#define MCFG_DECOCPU_DISPLAY(_disp_r, _disp_w) \
	downcast<decocpu_type1_device *>(device)->set_display_read_callback(DEVCB_##_disp_r); \
	downcast<decocpu_type1_device *>(device)->set_display_write_callback(DEVCB_##_disp_w);

#define MCFG_DECOCPU_DMDSTATUS(_dmdstat_r) \
	downcast<decocpu_type1_device *>(device)->set_dmdstatus_read_callback(DEVCB_##_dmdstat_r);

#define MCFG_DECOCPU_SOUNDLATCH(_soundlatch_w) \
	downcast<decocpu_type1_device *>(device)->set_soundlatch_write_callback(DEVCB_##_soundlatch_w);

#define MCFG_DECOCPU_SWITCH(_switch_r, _switch_w) \
	downcast<decocpu_type1_device *>(device)->set_switch_read_callback(DEVCB_##_switch_r); \
	downcast<decocpu_type1_device *>(device)->set_switch_write_callback(DEVCB_##_switch_w);

#define MCFG_DECOCPU_LAMP(_lamp_w) \
	downcast<decocpu_type1_device *>(device)->set_lamp_write_callback(DEVCB_##_lamp_w);

#define MCFG_DECOCPU_SOLENOIDS(_sol_w) \
	downcast<decocpu_type1_device *>(device)->set_solenoid_write_callback(DEVCB_##_sol_w);


// 6808 CPU's input clock is 4MHz
// but because it has an internal /4 divider, its E clock runs at 1/4 that frequency
#define E_CLOCK (XTAL_4MHz/4)

// Length of time in cycles between IRQs on the main 6808 CPU
// This length is determined by the settings of the W14 and W15 jumpers
// It can be 0x300, 0x380, 0x700 or 0x780 cycles long.
// IRQ length is always 32 cycles
#define S11_IRQ_CYCLES 0x380

class decocpu_type1_device : public device_t
{
public:
	decocpu_type1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	decocpu_type1_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	required_device<cpu_device> m_cpu;
	required_device<pia6821_device> m_pia21;
	required_device<pia6821_device> m_pia24;
	required_device<pia6821_device> m_pia28;
	required_device<pia6821_device> m_pia2c;
	required_device<pia6821_device> m_pia30;
	required_device<pia6821_device> m_pia34;

	// callbacks
	template<class _disp_r> void set_display_read_callback(_disp_r disp_r) { m_read_display.set_callback(disp_r); }
	template<class _disp_w> void set_display_write_callback(_disp_w disp_w) { m_write_display.set_callback(disp_w); }
	template<class _dmdstat_r> void set_dmdstatus_read_callback(_dmdstat_r dmdstat_r) { m_read_dmdstatus.set_callback(dmdstat_r); }
	template<class _soundlatch_w> void set_soundlatch_write_callback(_soundlatch_w slatch_w) { m_write_soundlatch.set_callback(slatch_w); }
	template<class _switch_r> void set_switch_read_callback(_switch_r switch_r) { m_read_switch.set_callback(switch_r); }
	template<class _switch_w> void set_switch_write_callback(_switch_w switch_w) { m_write_switch.set_callback(switch_w); }
	template<class _lamp_w> void set_lamp_write_callback(_lamp_w lamp_w) { m_write_lamp.set_callback(lamp_w); }
	template<class _sol_w> void set_solenoid_write_callback(_sol_w sol_w) { m_write_solenoid.set_callback(sol_w); }

	DECLARE_WRITE_LINE_MEMBER(cpu_pia_irq);
	DECLARE_WRITE_LINE_MEMBER(pia21_ca2_w);
	DECLARE_WRITE8_MEMBER(lamp0_w);
	DECLARE_WRITE8_MEMBER(lamp1_w);
	DECLARE_READ8_MEMBER(display_strobe_r);
	DECLARE_WRITE8_MEMBER(display_strobe_w);
	DECLARE_WRITE8_MEMBER(display_out1_w);
	DECLARE_WRITE8_MEMBER(display_out2_w);
	DECLARE_WRITE8_MEMBER(display_out3_w);
	DECLARE_WRITE8_MEMBER(display_out4_w);
	DECLARE_READ8_MEMBER(display_in3_r);
	DECLARE_WRITE8_MEMBER(switch_w);
	DECLARE_READ8_MEMBER(switch_r);
	DECLARE_READ8_MEMBER(dmdstatus_r);
	DECLARE_WRITE8_MEMBER(sound_w);
	DECLARE_WRITE8_MEMBER(solenoid1_w);
	DECLARE_WRITE8_MEMBER(solenoid2_w);
	INPUT_CHANGED_MEMBER(main_nmi);
	INPUT_CHANGED_MEMBER(audio_nmi);

	static void static_set_cpuregion(device_t &device, const char *tag);

protected:
	// overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	static const device_timer_id TIMER_IRQ = 0;

private:
	const char* m_cputag;  // region for cpu board code and data
	emu_timer* m_irq_timer;
	bool m_irq_active;
	bool m_ca2;

	// callbacks
	devcb_read8 m_read_display;
	devcb_write8 m_write_display;
	devcb_read8 m_read_dmdstatus;
	devcb_write8 m_write_soundlatch;
	devcb_read8 m_read_switch;
	devcb_write8 m_write_switch;
	devcb_write8 m_write_lamp;
	devcb_write8 m_write_solenoid;
};

class decocpu_type2_device : public decocpu_type1_device
{
public:
	decocpu_type2_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	decocpu_type2_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
protected:
	// overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;
};

class decocpu_type3_device : public decocpu_type2_device
{
public:
	decocpu_type3_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	decocpu_type3_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

protected:
	// overrides
	virtual void device_start() override;
};

class decocpu_type3b_device : public decocpu_type3_device
{
public:
	decocpu_type3b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// overrides
	virtual void device_start() override;
};

extern const device_type DECOCPU1;
extern const device_type DECOCPU2;
extern const device_type DECOCPU3;
extern const device_type DECOCPU3B;

#endif /* DECOPINCPU_H_ */
