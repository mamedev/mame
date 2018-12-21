// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    JPM IMPACT with Video hardware

****************************************************************************/
#include "machine/meters.h"
#include "machine/roc10937.h"
#include "machine/steppers.h"
#include "machine/timer.h"
#include "cpu/tms34010/tms34010.h"
#include "sound/upd7759.h"
#include "emupal.h"

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
		: driver_device(mconfig, type, tag)
		, m_duart_1_timer(*this, "duart_1_timer")
		, m_vfd(*this, "vfd")
		, m_vram(*this, "vram")
		, m_maincpu(*this, "maincpu")
		, m_upd7759(*this, "upd")
		, m_palette(*this, "palette")
		, m_dsp(*this, "dsp")
		, m_reel(*this, "reel%u", 0U)
		, m_meters(*this, "meters")
		, m_digits(*this, "digit%u", 0U)
		, m_lamp_output(*this, "lamp%u", 0U)
	{ }

	void impctawp(machine_config &config);
	void jpmimpct(machine_config &config);

private:
	template <unsigned N> DECLARE_WRITE_LINE_MEMBER(reel_optic_cb) { if (state) m_optic_pattern |= (1 << N); else m_optic_pattern &= ~(1 << N); }
	DECLARE_READ16_MEMBER(duart_1_r);
	DECLARE_WRITE16_MEMBER(duart_1_w);
	DECLARE_READ16_MEMBER(duart_2_r);
	DECLARE_WRITE16_MEMBER(duart_2_w);
	DECLARE_READ16_MEMBER(inputs1_r);
	DECLARE_READ16_MEMBER(unk_r);
	DECLARE_WRITE16_MEMBER(unk_w);
	DECLARE_READ16_MEMBER(jpmio_r);
	DECLARE_WRITE16_MEMBER(jpmio_w);
	DECLARE_READ16_MEMBER(inputs1awp_r);
	DECLARE_READ16_MEMBER(optos_r);
	DECLARE_READ16_MEMBER(prot_1_r);
	DECLARE_READ16_MEMBER(prot_0_r);
	DECLARE_WRITE16_MEMBER(jpmioawp_w);
	DECLARE_READ16_MEMBER(ump_r);
	DECLARE_WRITE16_MEMBER(jpmimpct_bt477_w);
	DECLARE_READ16_MEMBER(jpmimpct_bt477_r);
	DECLARE_WRITE16_MEMBER(volume_w);
	DECLARE_WRITE16_MEMBER(upd7759_w);
	DECLARE_READ16_MEMBER(upd7759_r);
	DECLARE_READ8_MEMBER(hopper_b_r);
	DECLARE_READ8_MEMBER(hopper_c_r);
	DECLARE_WRITE8_MEMBER(payen_a_w);
	DECLARE_WRITE8_MEMBER(display_c_w);
	DECLARE_WRITE_LINE_MEMBER(tms_irq);
	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg);
	TMS340X0_SCANLINE_RGB32_CB_MEMBER(scanline_update);
	DECLARE_MACHINE_START(jpmimpct);
	DECLARE_MACHINE_RESET(jpmimpct);
	DECLARE_VIDEO_START(jpmimpct);
	DECLARE_MACHINE_START(impctawp);
	DECLARE_MACHINE_RESET(impctawp);
	TIMER_DEVICE_CALLBACK_MEMBER(duart_1_timer_event);
	void awp68k_program_map(address_map &map);
	void m68k_program_map(address_map &map);
	void tms_program_map(address_map &map);

	uint8_t m_tms_irq;
	uint8_t m_duart_1_irq;
	struct duart_t m_duart_1;
	uint8_t m_touch_cnt;
	uint8_t m_touch_data[3];
	int m_lamp_strobe;
	uint8_t m_Lamps[256];
	int m_optic_pattern;
	int m_payen;
	int m_alpha_clock;
	int m_hopinhibit;
	int m_slidesout;
	int m_hopper[3];
	int m_motor[3];
	struct bt477_t m_bt477;
	void jpm_draw_lamps(int data, int lamp_strobe);
	void update_irqs();

	required_device<timer_device> m_duart_1_timer;
	optional_device<s16lf01_device> m_vfd;
	optional_shared_ptr<uint16_t> m_vram;
	required_device<cpu_device> m_maincpu;
	required_device<upd7759_device> m_upd7759;
	optional_device<palette_device> m_palette;
	optional_device<tms34010_device> m_dsp;
	optional_device_array<stepper_device, 6> m_reel;
	required_device<meters_device> m_meters;
	output_finder<300> m_digits;
	output_finder<256> m_lamp_output;
};
