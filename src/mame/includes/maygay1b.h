// license:BSD-3-Clause
// copyright-holders:David Haywood



#define VERBOSE 0
#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)

#define M1_MASTER_CLOCK (XTAL_8MHz)
#define M1_DUART_CLOCK  (XTAL_3_6864MHz)

#include "cpu/m6809/m6809.h"
#include "machine/i8279.h"

#include "video/awpvid.h"       //Fruit Machines Only
#include "machine/6821pia.h"
#include "machine/mc68681.h"
#include "machine/meters.h"
#include "machine/roc10937.h"   // vfd
#include "machine/steppers.h"   // stepper motor
#include "sound/ay8910.h"
#include "sound/ym2413.h"
#include "sound/okim6376.h"
#include "machine/nvram.h"
#include "sound/upd7759.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/okim6295.h"

class maygay1b_state : public driver_device
{
public:
	maygay1b_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mcu(*this, "mcu"),
		m_vfd(*this, "vfd"),
		m_ay(*this, "aysnd"),
		m_msm6376(*this, "msm6376"),
		m_upd7759(*this, "upd"),
		m_okim6295(*this, "oki"),
		m_duart68681(*this, "duart68681"),
		m_sw1_port(*this, "SW1"),
		m_sw2_port(*this, "SW2"),
		m_kbd_ports(*this, { "SW1", "SW2", "STROBE2", "STROBE3", "STROBE4", "STROBE5", "STROBE6", "STROBE7", }),
		m_bank1(*this, "bank1"),
		m_reel0(*this, "reel0"),
		m_reel1(*this, "reel1"),
		m_reel2(*this, "reel2"),
		m_reel3(*this, "reel3"),
		m_reel4(*this, "reel4"),
		m_reel5(*this, "reel5"),
		m_meters(*this, "meters"),
		m_oki_region(*this, "msm6376")
	{}

	required_device<cpu_device> m_maincpu;
	required_device<i80c51_device> m_mcu;
	optional_device<s16lf01_t> m_vfd;
	required_device<ay8910_device> m_ay;
	optional_device<okim6376_device> m_msm6376;
	optional_device<upd7759_device> m_upd7759;
	optional_device<okim6295_device> m_okim6295;
	required_device<mc68681_device> m_duart68681;
	required_ioport m_sw1_port;
	required_ioport m_sw2_port;
	required_ioport_array<8> m_kbd_ports;
	required_memory_bank m_bank1;
	required_device<stepper_device> m_reel0;
	required_device<stepper_device> m_reel1;
	required_device<stepper_device> m_reel2;
	required_device<stepper_device> m_reel3;
	required_device<stepper_device> m_reel4;
	required_device<stepper_device> m_reel5;
	required_device<meters_device> m_meters;
	optional_region_ptr<uint8_t> m_oki_region;

	uint8_t m_lamppos;
	int m_lamp_strobe;
	int m_old_lamp_strobe;
	int m_lamp_strobe2;
	int m_old_lamp_strobe2;
	int m_RAMEN;
	int m_ALARMEN;
	int m_PSUrelay;
	bool m_Vmm;
	int m_WDOG;
	int m_NMIENABLE;
	int m_meter;
	void maygay1b_nmitimer_callback(timer_device &timer, void *ptr, int32_t param);
	uint8_t m_Lamps[256];
	int m_optic_pattern;
	void reel0_optic_cb(int state) { if (state) m_optic_pattern |= 0x01; else m_optic_pattern &= ~0x01; }
	void reel1_optic_cb(int state) { if (state) m_optic_pattern |= 0x02; else m_optic_pattern &= ~0x02; }
	void reel2_optic_cb(int state) { if (state) m_optic_pattern |= 0x04; else m_optic_pattern &= ~0x04; }
	void reel3_optic_cb(int state) { if (state) m_optic_pattern |= 0x08; else m_optic_pattern &= ~0x08; }
	void reel4_optic_cb(int state) { if (state) m_optic_pattern |= 0x10; else m_optic_pattern &= ~0x10; }
	void reel5_optic_cb(int state) { if (state) m_optic_pattern |= 0x20; else m_optic_pattern &= ~0x20; }
	void scanlines_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void scanlines_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lamp_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lamp_data_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t kbd_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void reel12_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void reel34_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void reel56_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m1_latch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void latch_ch2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t latch_st_hi(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t latch_st_lo(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void m1ab_no_oki_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m1_pia_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m1_pia_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m1_lockout_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void m1_meter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t m1_meter_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t m1_firq_clr_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t m1_firq_trg_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t m1_firq_nec_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t nec_reset_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void nec_bank0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void nec_bank1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void duart_irq_handler(int state);
	uint8_t m1_duart_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mcu_port0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_port1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_port2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_port3_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mcu_port0_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mcu_port2_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void main_to_mcu_0_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void main_to_mcu_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t m_main_to_mcu;

	void init_m1();
	void init_m1common();
	void init_m1nec();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	void cpu0_firq(int data);
	void cpu0_nmi();
};

MACHINE_CONFIG_EXTERN( maygay_m1 );
MACHINE_CONFIG_EXTERN( maygay_m1_nec );
MACHINE_CONFIG_EXTERN( maygay_m1_no_oki );
MACHINE_CONFIG_EXTERN( maygay_m1_empire );
