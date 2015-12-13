// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
/*************************************************************************

    Exidy Vertigo hardware

*************************************************************************/

#include "audio/exidy440.h"
#include "machine/pit8253.h"
#include "machine/74148.h"
#include "video/vector.h"

/*************************************
 *
 *  Typedefs
 *
 *************************************/

#define MC_LENGTH 512

struct am2901
{
	UINT32 ram[16];   /* internal ram */
	UINT32 d;         /* direct data D input */
	UINT32 q;         /* Q register */
	UINT32 f;         /* F ALU result */
	UINT32 y;         /* Y output */
};

class vector_generator
{
public:
	UINT32 sreg;      /* shift register */
	UINT32 l1;        /* latch 1 adder operand only */
	UINT32 l2;        /* latch 2 adder operand only */
	UINT32 c_v;       /* vertical position counter */
	UINT32 c_h;       /* horizontal position counter */
	UINT32 c_l;       /* length counter */
	UINT32 adder_s;   /* slope generator result and B input */
	UINT32 adder_a;   /* slope generator A input */
	UINT32 color;     /* color */
	UINT32 intensity; /* intensity */
	UINT32 brez;      /* h/v-counters enable */
	UINT32 vfin;      /* drawing yes/no */
	UINT32 hud1;      /* h-counter up or down (stored in L1) */
	UINT32 hud2;      /* h-counter up or down (stored in L2) */
	UINT32 vud1;      /* v-counter up or down (stored in L1) */
	UINT32 vud2;      /* v-counter up or down (stored in L2) */
	UINT32 hc1;       /* use h- or v-counter in L1 mode */
	UINT32 ven;       /* vector intensity enable */

private:
};

struct microcode
{
	UINT32 x;
	UINT32 a;
	UINT32 b;
	UINT32 inst;
	UINT32 dest;
	UINT32 cn;
	UINT32 mreq;
	UINT32 rsel;
	UINT32 rwrite;
	UINT32 of;
	UINT32 iif;
	UINT32 oa;
	UINT32 jpos;
	UINT32 jmp;
	UINT32 jcon;
	UINT32 ma;
};

struct vproc
{
	UINT16 sram[64]; /* external sram */
	UINT16 ramlatch; /* latch between 2901 and sram */
	UINT16 rom_adr;  /* vector ROM/RAM address latch */
	UINT32 pc;       /* program counter */
	UINT32 ret;      /* return address */

};


class vertigo_state : public driver_device
{
public:
	vertigo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_audiocpu(*this, "audiocpu"),
			m_pit(*this, "pit8254"),
			m_custom(*this, "custom"),
			m_ttl74148(*this, "74148"),
			m_vector(*this, "vector"),
			m_vectorram(*this, "vectorram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<pit8254_device> m_pit;
	required_device<exidy440_sound_device> m_custom;
	required_device<ttl74148_device> m_ttl74148;
	required_device<vector_device> m_vector;
	required_shared_ptr<UINT16> m_vectorram;
	attotime m_irq4_time;
	UINT8 m_irq_state;
	UINT8 m_adc_result;
	vproc m_vs;
	am2901 m_bsp;
	vector_generator m_vgen;
	UINT16 *m_vectorrom;
	microcode m_mc[MC_LENGTH];
	DECLARE_READ16_MEMBER(vertigo_io_convert);
	DECLARE_READ16_MEMBER(vertigo_io_adc);
	DECLARE_READ16_MEMBER(vertigo_coin_r);
	DECLARE_WRITE16_MEMBER(vertigo_wsot_w);
	DECLARE_WRITE16_MEMBER(vertigo_audio_w);
	DECLARE_READ16_MEMBER(vertigo_sio_r);
	DECLARE_WRITE16_MEMBER(vertigo_motor_w);
	DECLARE_READ16_MEMBER(vertigo_pit8254_lsb_r);
	DECLARE_WRITE16_MEMBER(vertigo_pit8254_lsb_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	INTERRUPT_GEN_MEMBER(vertigo_interrupt);
	TIMER_CALLBACK_MEMBER(sound_command_w);
	DECLARE_WRITE_LINE_MEMBER(v_irq4_w);
	DECLARE_WRITE_LINE_MEMBER(v_irq3_w);
	TTL74148_OUTPUT_CB(update_irq);

	void vertigo_vproc_init();
	void vertigo_vproc_reset();
	void am2901x4 (am2901 *bsp, microcode *mc);
	void vertigo_vgen (vector_generator *vg);
	void vertigo_vproc(int cycles, int irq4);
	void update_irq_encoder(int line, int state);
};
