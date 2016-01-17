// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * wpc.h
 *
 *  Created on: 7/10/2013
 */

#ifndef WPC_H_
#define WPC_H_

#include "emu.h"

/* A = Alpha-numeric
 * M = DMD
 * F = Fliptronics
 * D = DCS
 * S = Security
 * 9 = WPC-95
 */
/*                                  AMFDS9 */
/* DMD */
#define DMD_PAGE3200      (0x08)
#define DMD_PAGE3000      (0x09)
#define DMD_PAGE3600      (0x0a)
#define DMD_PAGE3400      (0x0b)
#define DMD_PAGE3A00      (0x0c)
#define DMD_FIRQLINE      (0x0d)
#define DMD_PAGE3800      (0x0e)
#define DMD_VISIBLEPAGE   (0x0f)

/* Printer board */
#define WPC_PRINTBUSY     (0x10) /* xxxxx  R: Printer ready ??? */
#define WPC_PRINTDATA     (0x11) /* xxxxx  W: send to printer */
#define WPC_PRINTDATAX    (0x12) /* xxxxx  W: 0: Printer data available */

/* Fliptronics flippers */
#define WPC_FLIPPERS      (0x24) /*   xxx  R: switches W: Solenoids */

/* Sound board */
#define WPC_SOUNDS11      (0x21) /* xxx    RW: R: Sound data availble, W: Reset soundboard ? */
#define WPC_SOUNDIF       (0x2c) /* xxx    RW: Sound board interface */
#define WPC_SOUNDBACK     (0x2d) /* xxx    RW: R: Sound data availble, W: Reset soundboard ? */

#define WPC_SOLENOID1     (0x30) /* xxxxxx W: Solenoid 25-28 */
#define WPC_SOLENOID2     (0x31) /* xxxxxx W: Solenoid  1- 8 */
#define WPC_SOLENOID3     (0x32) /* xxxxxx W: Solenoid 17-24 */
#define WPC_SOLENOID4     (0x33) /* xxxxxx W: Solenoid  9-16 */
#define WPC_LAMPROW       (0x34) /* xxxxxx W: Lamp row */
#define WPC_LAMPCOLUMN    (0x35) /* xxxxxx W: Lamp column enable */
#define WPC_GILAMPS       (0x36) /*        W: GI lights ?? */
#define WPC_DIPSWITCH     (0x37) /* xxxxxx R: CPU board dip-switches */
#define WPC_SWCOINDOOR    (0x38) /* xxxxxx W: Coin door switches */
#define WPC_SWROWREAD     (0x39) /* xxxx   R: Switch row read */
#define WPC_SWCOLSELECT   (0x3a) /* xxxx   W: Switch column enable */
#define WPC_ALPHAPOS      (0x3b) /* x      W: Select alphanumeric position */
#define WPC_ALPHA1LO      (0x3c) /* x      W: Display 1st row hi bits */
#define WPC_ALPHA1HI      (0x3d) /* x      W: Display 1st row lo bits */
#define WPC_ALPHA2LO      (0x3e) /* x      W: Display 2nd row hi bits */
#define WPC_ALPHA2HI      (0x3f) /* x      W:           b 2nd row lo bits */
#define WPC_LED           (0x42) /* xxxxxx W: CPU LED (bit 7) */
#define WPC_IRQACK        (0x43) /*        W: IRQ Ack ??? */
#define WPC_SHIFTADRH     (0x44) /* xxxxxx RW: See above */
#define WPC_SHIFTADRL     (0x45) /* xxxxxx RW: See above */
#define WPC_SHIFTBIT      (0x46) /* xxxxxx RW: See above */
#define WPC_SHIFTBIT2     (0x47) /* xxxxxx RW: See above */
#define WPC_FIRQSRC       (0x48) /*   xxxx R: bit 7 0=DMD, 1=SOUND? W: Clear FIRQ line */
#define WPC_RTCHOUR       (0x4a) /* xxxxxx RW: Real time clock: hour */
#define WPC_RTCMIN        (0x4b) /* xxxxxx RW: Real time clock: minute */
#define WPC_ROMBANK       (0x4c) /* xxxxxx W: Rombank switch */
#define WPC_PROTMEM       (0x4d) /* xxxxxx W: enabled/disable protected memory */
#define WPC_PROTMEMCTRL   (0x4e) /* xxxxxx W: Set protected memory area */
#define WPC_WATCHDOG      (0x4f) /* xxxxxx W: Watchdog */


#define MCFG_WMS_WPC_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, WPCASIC, 0);

#define MCFG_WPC_IRQ_ACKNOWLEDGE(_irq) \
	downcast<wpc_device *>(device)->set_irq_callback(DEVCB_##_irq);

#define MCFG_WPC_FIRQ_ACKNOWLEDGE(_firq) \
	downcast<wpc_device *>(device)->set_firq_callback(DEVCB_##_firq);

#define MCFG_WPC_SOUND_DATA(_sounddata_r,_sounddata_w) \
	downcast<wpc_device *>(device)->set_sound_data_read(DEVCB_##_sounddata_r); \
	downcast<wpc_device *>(device)->set_sound_data_write(DEVCB_##_sounddata_w);

#define MCFG_WPC_SOUND_CTRL(_soundctrl_r,_soundctrl_w) \
	downcast<wpc_device *>(device)->set_sound_ctrl_read(DEVCB_##_soundctrl_r); \
	downcast<wpc_device *>(device)->set_sound_ctrl_write(DEVCB_##_soundctrl_w);

#define MCFG_WPC_SOUND_S11C(_sounds11_w) \
	downcast<wpc_device *>(device)->set_sound_s11_write(DEVCB_##_sounds11_w);

#define MCFG_WPC_ROMBANK(_bank_w) \
	downcast<wpc_device *>(device)->set_bank_write(DEVCB_##_bank_w);

#define MCFG_WPC_DMDBANK(_dmdbank_w) \
	downcast<wpc_device *>(device)->set_dmdbank_write(DEVCB_##_dmdbank_w);

class wpc_device : public device_t
{
public:
	wpc_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	UINT16 get_memprotect_mask() { return m_memprotect_mask; }
	bool memprotect_active() { if(m_memprotect == 0xb4) return false; else return true; }
	UINT16 get_alphanumeric(UINT8 offset) { if(offset < 40) return m_alpha_data[offset]; else return 0; }
	void reset_alphanumeric() { memset(m_alpha_data,0,40*2); }
	UINT8 get_visible_page() { return m_dmd_visiblepage; }
	UINT8 get_dmd_firq_line() { return m_dmd_irqline; }
	void set_dmd_firq() { m_dmd_irqsrc = true; }
	void set_snd_firq() { m_snd_irqsrc = true; }

	// callbacks
	template<class _irq> void set_irq_callback(_irq irq) { m_irq_cb.set_callback(irq); }
	template<class _firq> void set_firq_callback(_firq firq) { m_firq_cb.set_callback(firq); }
	template<class _sounddata_r> void set_sound_data_read(_sounddata_r sounddata_r) { m_sounddata_r.set_callback(sounddata_r); }
	template<class _sounddata_w> void set_sound_data_write(_sounddata_w sounddata_w) { m_sounddata_w.set_callback(sounddata_w); }
	template<class _soundctrl_r> void set_sound_ctrl_read(_soundctrl_r soundctrl_r) { m_soundctrl_r.set_callback(soundctrl_r); }
	template<class _soundctrl_w> void set_sound_ctrl_write(_soundctrl_w soundctrl_w) { m_soundctrl_w.set_callback(soundctrl_w); }
	template<class _sounds11> void set_sound_s11_write(_sounds11 sounds11) { m_sounds11_w.set_callback(sounds11); }
	template<class _bank_w> void set_bank_write(_bank_w bank_w) { m_bank_w.set_callback(bank_w); }
	template<class _dmdbank_w> void set_dmdbank_write(_dmdbank_w dmdbank_w) { m_dmdbank_w.set_callback(dmdbank_w); }

	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	static const device_timer_id TIMER_IRQ = 1;
	static const device_timer_id TIMER_ZEROCROSS = 2;
protected:
	// overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	UINT8 m_shift_addr_high;
	UINT8 m_shift_addr_low;
	UINT8 m_shift_bit1;
	UINT8 m_shift_bit2;
	UINT8 m_memprotect;
	UINT16 m_memprotect_mask;
	UINT8 m_switch_col;  // select switch column
	UINT8 m_alpha_pos;  // selected LED position
	UINT16 m_alpha_data[40];
	bool m_zerocross;
	UINT32 m_irq_count;
	UINT8 m_dmd_visiblepage;
	bool m_dmd_irqsrc;
	bool m_snd_irqsrc;
	UINT8 m_dmd_irqline;
	emu_timer* m_zc_timer;

	devcb_write_line m_irq_cb;
	devcb_write_line m_firq_cb;
	devcb_read8 m_sounddata_r;
	devcb_write8 m_sounddata_w;
	devcb_read8 m_soundctrl_r;
	devcb_write8 m_soundctrl_w;
	devcb_write8 m_sounds11_w;
	devcb_write8 m_bank_w;
	devcb_write8 m_dmdbank_w;
};

extern const device_type WPCASIC;

#endif /* WPC_H_ */
