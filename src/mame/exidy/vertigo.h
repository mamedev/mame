// license:BSD-3-Clause
// copyright-holders:Mathis Rosenhauer
/*************************************************************************

    Exidy Vertigo hardware

*************************************************************************/
#ifndef MAME_EXIDY_VERTIGO_H
#define MAME_EXIDY_VERTIGO_H

#pragma once

#include "exidy440_a.h"

#include "machine/74148.h"
#include "machine/adc0808.h"
#include "video/vector.h"

/*************************************
 *
 *  Typedefs
 *
 *************************************/

#define MC_LENGTH 512


class vertigo_state : public driver_device
{
public:
	vertigo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_custom(*this, "440audio"),
		m_ttl74148(*this, "74148"),
		m_vector(*this, "vector"),
		m_adc(*this, "adc"),
		m_vectorram(*this, "vectorram")
	{ }

	void vertigo(machine_config &config);

private:
	void adc_eoc_w(int state);
	uint16_t vertigo_io_convert(offs_t offset);
	uint16_t vertigo_coin_r();
	void vertigo_wsot_w(uint16_t data);
	void vertigo_audio_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t vertigo_sio_r();
	void vertigo_motor_w(uint16_t data);
	INTERRUPT_GEN_MEMBER(vertigo_interrupt);
	TIMER_CALLBACK_MEMBER(sound_command_w);
	void v_irq4_w(int state);
	void v_irq3_w(int state);
	void update_irq(uint8_t data);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	void exidy440_audio(machine_config &config);
	void vertigo_map(address_map &map) ATTR_COLD;
	void vertigo_motor(address_map &map) ATTR_COLD;
	void exidy440_audio_map(address_map &map) ATTR_COLD;

	struct am2901
	{
		uint32_t ram[16] = { };  // internal ram
		uint32_t d = 0;          // direct data D input
		uint32_t q = 0;          // Q register
		uint32_t f = 0;          // F ALU result
		uint32_t y = 0;          // Y output
	};

	class vector_generator
	{
	public:
		uint32_t sreg = 0;       // shift register
		uint32_t l1 = 0;         // latch 1 adder operand only
		uint32_t l2 = 0;         // latch 2 adder operand only
		uint32_t c_v = 0;        // vertical position counter
		uint32_t c_h = 0;        // horizontal position counter
		uint32_t c_l = 0;        // length counter
		uint32_t adder_s = 0;    // slope generator result and B input
		uint32_t adder_a = 0;    // slope generator A input
		uint32_t color = 0;      // color
		uint32_t intensity = 0;  // intensity
		uint32_t brez = 0;       // h/v-counters enable
		uint32_t vfin = 0;       // drawing yes/no
		uint32_t hud1 = 0;       // h-counter up or down (stored in L1)
		uint32_t hud2 = 0;       // h-counter up or down (stored in L2)
		uint32_t vud1 = 0;       // v-counter up or down (stored in L1)
		uint32_t vud2 = 0;       // v-counter up or down (stored in L2)
		uint32_t hc1 = 0;        // use h- or v-counter in L1 mode
		uint32_t ven = 0;        // vector intensity enable
	};

	struct microcode
	{
		uint32_t x = 0;
		uint32_t a = 0;
		uint32_t b = 0;
		uint32_t inst = 0;
		uint32_t dest = 0;
		uint32_t cn = 0;
		uint32_t mreq = 0;
		uint32_t rsel = 0;
		uint32_t rwrite = 0;
		uint32_t of = 0;
		uint32_t iif = 0;
		uint32_t oa = 0;
		uint32_t jpos = 0;
		uint32_t jmp = 0;
		uint32_t jcon = 0;
		uint32_t ma = 0;
	};

	struct vproc
	{
		uint16_t sram[64] = { }; // external sram
		uint16_t ramlatch = 0;   // latch between 2901 and sram
		uint16_t rom_adr = 0;    // vector ROM/RAM address latch
		uint32_t pc = 0;         // program counter
		uint32_t ret = 0;        // return address

	};

	void am2901x4(am2901 &bsp, microcode const &mc);
	void vertigo_vgen(vector_generator &vg);
	void vertigo_vproc_init();
	void vertigo_vproc_reset();
	void vertigo_vproc(int cycles, int irq4);
	void update_irq_encoder(int line, int state);

	required_device<cpu_device> m_maincpu;
	required_device<exidy440_sound_device> m_custom;
	required_device<ttl74148_device> m_ttl74148;
	required_device<vector_device> m_vector;
	required_device<adc0808_device> m_adc;
	required_shared_ptr<uint16_t> m_vectorram;
	attotime m_irq4_time;
	uint8_t m_irq_state = 0;
	vproc m_vs;
	am2901 m_bsp;
	vector_generator m_vgen;
	uint16_t *m_vectorrom = nullptr;
	microcode m_mc[MC_LENGTH];
};

#endif // MAME_EXIDY_VERTIGO_H
