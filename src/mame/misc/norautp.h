// license:BSD-3-Clause
// copyright-holders:Angelo Salese, Roberto Fresca
#ifndef MAME_MISC_NORAUTP_H
#define MAME_MISC_NORAUTP_H

#pragma once

#include "machine/i8255.h"
#include "sound/discrete.h"
#include "emupal.h"
#include "screen.h"


/* Discrete Sound Input Nodes */
#define NORAUTP_SND_EN                  NODE_01
#define NORAUTP_FREQ_DATA               NODE_02


class norautp_state : public driver_device
{
public:
	norautp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ppi8255(*this, "ppi8255_%u", 0),
		m_discrete(*this, "discrete"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void noraut_base(machine_config &config);
	void kimble(machine_config &config);
	void kimbldhl(machine_config &config);
	void norautp(machine_config &config);
	void norautu(machine_config &config);
	void norautx4(machine_config &config);
	void norautpl(machine_config &config);
	void newhilop(machine_config &config);
	void dphltest(machine_config &config);
	void nortest1(machine_config &config);
	void ssjkrpkr(machine_config &config);
	void dphl(machine_config &config);
	void dphla(machine_config &config);
	void drhl(machine_config &config);
	void norautxp(machine_config &config);
	void noraut3(machine_config &config);
	void cgidjp(machine_config &config);
	void cdrawpkr(machine_config &config);

	void init_ssa();
	void init_enc();
	void init_deb();

protected:
	virtual void machine_start() override { m_lamps.resolve(); }
	virtual void video_start() override ATTR_COLD;

private:
	void ppi2_obf_w(int state);
	TIMER_CALLBACK_MEMBER(ppi2_ack);
	uint8_t test2_r();
	void mainlamps_w(uint8_t data);
	void soundlamps_w(uint8_t data);
	void counterlamps_w(uint8_t data);
	void norautp_palette(palette_device &palette) const;
	uint32_t screen_update_norautp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void cgidjp_map(address_map &map) ATTR_COLD;
	void cgidjp_opcodes_map(address_map &map) ATTR_COLD;
	void decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void dphl_map(address_map &map) ATTR_COLD;
	void dphla_map(address_map &map) ATTR_COLD;
	void dphltest_map(address_map &map) ATTR_COLD;
	void drhl_map(address_map &map) ATTR_COLD;
	void kimbldhl_map(address_map &map) ATTR_COLD;
	void kimble_map(address_map &map) ATTR_COLD;
	void newhilop_map(address_map &map) ATTR_COLD;
	void noraut3_map(address_map &map) ATTR_COLD;
	void noraut3_decrypted_opcodes_map(address_map &map) ATTR_COLD;
	void norautp_map(address_map &map) ATTR_COLD;
	void norautp_portmap(address_map &map) ATTR_COLD;
	void norautx4_map(address_map &map) ATTR_COLD;
	void norautx8_map(address_map &map) ATTR_COLD;
	void norautxp_map(address_map &map) ATTR_COLD;
	void norautxp_portmap(address_map &map) ATTR_COLD;
	void nortest1_map(address_map &map) ATTR_COLD;
	void ssjkrpkr_map(address_map &map) ATTR_COLD;

	std::unique_ptr<uint16_t[]> m_np_vram;
	required_device<cpu_device> m_maincpu;
	required_device_array<i8255_device, 3> m_ppi8255;
	required_device<discrete_sound_device> m_discrete;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_shared_ptr<uint8_t> m_decrypted_opcodes;
	output_finder<12> m_lamps;
};

/*----------- defined in audio/norautp.c -----------*/
DISCRETE_SOUND_EXTERN( norautp_discrete );
DISCRETE_SOUND_EXTERN( dphl_discrete );
DISCRETE_SOUND_EXTERN( kimble_discrete );

#endif // MAME_MISC_NORAUTP_H
