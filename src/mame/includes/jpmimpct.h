// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    JPM IMPACT with Video hardware

****************************************************************************/
#include "machine/meters.h"
#include "machine/roc10937.h"
#include "machine/steppers.h"
#include "cpu/tms34010/tms34010.h"
#include "sound/upd7759.h"

struct duart_t
{
	uint8_t MR1A, MR2A;
	uint8_t SRA, CSRA;
	uint8_t CRA;
	uint8_t RBA, TBA;

	uint8_t IPCR;
	uint8_t ACR;
	uint8_t ISR, IMR;

	union
	{
		uint8_t CUR, CLR;
		uint16_t CR;
	};
	union
	{
		uint8_t CTUR, CTLR;
		uint16_t CT;
	};

	int tc;

	uint8_t MR1B, MR2B;
	uint8_t SRB, CSRB;
	uint8_t CRB;
	uint8_t RBB, TBB;

	uint8_t IVR;
	uint8_t IP;
	uint8_t OP;
	uint8_t OPR;
	uint8_t OPCR;
};

struct bt477_t
{
	uint8_t address;
	uint8_t addr_cnt;
	uint8_t pixmask;
	uint8_t command;
	rgb_t color;
};

class jpmimpct_state : public driver_device
{
public:
	jpmimpct_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_vfd(*this, "vfd"),
			m_vram(*this, "vram") ,
		m_maincpu(*this, "maincpu"),
		m_upd7759(*this, "upd"),
		m_palette(*this, "palette"),
		m_dsp(*this, "dsp"),
		m_reel0(*this, "reel0"),
		m_reel1(*this, "reel1"),
		m_reel2(*this, "reel2"),
		m_reel3(*this, "reel3"),
		m_reel4(*this, "reel4"),
		m_reel5(*this, "reel5"),
		m_meters(*this, "meters")
		{ }

	uint8_t m_tms_irq;
	uint8_t m_duart_1_irq;
	struct duart_t m_duart_1;
	uint8_t m_touch_cnt;
	uint8_t m_touch_data[3];
	int m_lamp_strobe;
	uint8_t m_Lamps[256];
	int m_optic_pattern;
	void reel0_optic_cb(int state) { if (state) m_optic_pattern |= 0x01; else m_optic_pattern &= ~0x01; }
	void reel1_optic_cb(int state) { if (state) m_optic_pattern |= 0x02; else m_optic_pattern &= ~0x02; }
	void reel2_optic_cb(int state) { if (state) m_optic_pattern |= 0x04; else m_optic_pattern &= ~0x04; }
	void reel3_optic_cb(int state) { if (state) m_optic_pattern |= 0x08; else m_optic_pattern &= ~0x08; }
	void reel4_optic_cb(int state) { if (state) m_optic_pattern |= 0x10; else m_optic_pattern &= ~0x10; }
	void reel5_optic_cb(int state) { if (state) m_optic_pattern |= 0x20; else m_optic_pattern &= ~0x20; }
	int m_payen;
	int m_alpha_clock;
	int m_hopinhibit;
	int m_slidesout;
	int m_hopper[3];
	int m_motor[3];
	optional_device<s16lf01_t> m_vfd;
	optional_shared_ptr<uint16_t> m_vram;
	struct bt477_t m_bt477;
	uint16_t duart_1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void duart_1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t duart_2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void duart_2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t inputs1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t unk_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void unk_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t jpmio_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void jpmio_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t inputs1awp_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t optos_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t prot_1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t prot_0_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void jpmioawp_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t ump_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void jpm_draw_lamps(int data, int lamp_strobe);
	void jpmimpct_bt477_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t jpmimpct_bt477_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void volume_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void upd7759_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t upd7759_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint8_t hopper_b_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t hopper_c_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void payen_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void display_c_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void tms_irq(int state);
	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg);
	TMS340X0_SCANLINE_RGB32_CB_MEMBER(scanline_update);

	void machine_start_jpmimpct();
	void machine_reset_jpmimpct();
	void video_start_jpmimpct();
	void machine_start_impctawp();
	void machine_reset_impctawp();
	void duart_1_timer_event(timer_device &timer, void *ptr, int32_t param);
	void update_irqs();
	required_device<cpu_device> m_maincpu;
	required_device<upd7759_device> m_upd7759;
	optional_device<palette_device> m_palette;
	optional_device<tms34010_device> m_dsp;
	optional_device<stepper_device> m_reel0;
	optional_device<stepper_device> m_reel1;
	optional_device<stepper_device> m_reel2;
	optional_device<stepper_device> m_reel3;
	optional_device<stepper_device> m_reel4;
	optional_device<stepper_device> m_reel5;
	required_device<meters_device> m_meters;
};
