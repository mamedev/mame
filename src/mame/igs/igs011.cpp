// license:BSD-3-Clause
// copyright-holders:Luca Elia, Olivier Galibert
/***************************************************************************

                      -= IGS011 (Blitter) Based Hardware =-

                    driver by   Luca Elia (l.elia@tin.it)
            code decrypted by   Olivier Galibert


CPU     :   68000
Sound   :   M6295 + Optional FM or ICS2115
Custom  :   IGS011 (blitter, protection)
            IGS012 (protection, optional)
            IGS003 (8255, protection)
NVRAM   :   Battery for main RAM

---------------------------------------------------------------------------
Year + Game                   PCB        Sound         Chips
---------------------------------------------------------------------------
95 Da Ban Cheng               NO-T0084-1 M6295         IGS011 8255
95 Long Hu Bang V033C         NO-T0093   M6295         IGS011 8255
95 Long Hu Bang V035C         ?
95 Mj Ryukobou                NO-T0094   M6295         IGS011 8255
95 Dragon World V010C         NO-0105-4  M6295 YM3812  IGS011 IGS003
95 Dragon World V011H (set 1) ?
95 Dragon World V011H (set 2) NO-T0105   M6295 YM3812  IGS011 IGS003  IGS012
95 Dragon World V020J         ?          M6295 YM3812  IGS011 IGS003  IGS012
95 Dragon World V021J         ?
95 Dragon World V021O         NO-0105-1  M6295 YM3812  IGS011 IGS003  IGS012
95 Dragon World V030O         NO-0105-1  M6295 YM3812  IGS011 IGS003
97 Dragon World V040O         NO-0105-5  M6295 YM3812  IGS011 IGS003c
97 Dragon World V040K         NO-0105-5  M6295 YM3812  IGS011 IGS003c IGS012
96 Virtua Bowling V100JCM     NO-0101-?  ICS2115       IGS011 IGS003e IGS012
96 Virtua Bowling V101XCM     NO-0101-1  ICS2115       IGS011 IGS003e IGS012
96 Virtua Bowling V101HJS     NO-0101-?  ICS2115       IGS011 IGS003e?IGS012
96 Long Hu Bang II V185H      NO-0115    M6295 YM2413  IGS011 8255
96 Te Yi Gong Neng            NO-0127-2  M6295 YM2413  IGS011 IGS003A
96 Wanli Changcheng           ?
96 Xingyun ManGuan            ?
98 Mj Nenrikishu SP V250J     NO-0115-5  M6295 YM2413  IGS011 8255
---------------------------------------------------------------------------

TODO:

- Interrupt controller at 838000 or a38000 (there's a preliminary
  implementation for lhb)

- vbowl, vbowlj: trackball glitches occasionally with "direct"
  orientation, jumping by exactly 181 in the direction orthogonal to
  the movement

- xymga: the sum of the program ROM long words computed by the game doesn't
  match the one stored at 7da4 (bad dump?), it is replaced in the ROM
  definition, otherwise the game stops with 'RECORD ERROR 3'

- lhb3, nkishusp, tygn: the serial port at a00000-a98000 is only a stub that
  loops the transmitted bytes back to the receiver, which is enough to pass
  the games' periodic echo check. The chip (no UART is visible on the PCBs),
  the baud rate, the meaning of the control bits and what is connected to
  the port on real hardware (another cabinet, a device, a loopback plug?)
  are unknown.

- tygn: payout and clear inputs shown in input test don't work,
  joystick left/right show incorrect names in input test

Notes:

- In most games, keep the Test button pressed during boot to access the
  input test and sound test.

Original game bugs that are emulated faithfully:

- dbc: The title screen background palette is wrong since the fade
  routine is called with incorrect argument values.

- drgnwrld: The copyright text on the title screen was updated to show
  "1997" (rather than "1995") by drawing the "7" over the top using the
  CPU rather than the blitter.  The font and background of the "7"
  appears mismatched.

- drgnwrld: If you change the Background DIP switch setting (SW2 2) to
  "Scene", you'll still see the "Girl" animation, albeit with incorrect
  colours, after completing some stages.

***************************************************************************/

#include "emu.h"

#include "igs003.h"
#include "igs011_video.h"
#include "igs012.h"
#include "igsmahjong.h"

#include "mahjong.h"

#include "cpu/m68000/m68000.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/timer.h"
#include "sound/ics2115.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class igs011_state_base : public driver_device
{
public:
	igs011_state_base(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_igs003(*this, "igs003")
		, m_igs011(*this, "igs011")
		, m_igs012(*this, "igs012")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_maincpu_region(*this, "maincpu")
		, m_io_in(*this, "IN%u", 0U)
		, m_io_dsw(*this, "DSW%u", 1U)
		, m_io_coin(*this, "COIN")
	{
	}

protected:
	virtual void machine_start() override ATTR_COLD;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<igs003_device> m_igs003;
	required_device<igs011_device> m_igs011;
	optional_device<igs012_device> m_igs012;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	/* memory regions */
	required_memory_region m_maincpu_region;

	optional_ioport_array<3> m_io_in;
	optional_ioport_array<5> m_io_dsw;
	optional_ioport m_io_coin;

	u16 m_dips_sel = 0;
	u8 m_prot = 0;

	void igs011_base(machine_config &config) ATTR_COLD;

	void dips_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	template<unsigned Num> u16 dips_r();
	void prot_reset_w(u8 data);
	u16 prot_reset_r();
	void prot_inc_w(u8 data);
	void prot_dec_w(u8 data);
	void drgnwrld_prot_swap_w(u8 data);
	u16 drgnwrldv40k_prot_r();
	u16 drgnwrldv21_prot_r();
	u16 drgnwrldv20j_prot_r();
	void igs003_index_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void igs003_data_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void igs012_regs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void sound_irq(int state);
	template <uint8_t Irq> TIMER_DEVICE_CALLBACK_MEMBER(timer_irq_cb);

};

// With Oki MSM6295 sound
class igs011_oki_state : public igs011_state_base
{
public:
	igs011_oki_state(const machine_config &mconfig, device_type type, const char *tag)
		: igs011_state_base(mconfig, type, tag)
		, m_oki(*this, "oki")
		, m_hopper(*this, "hopper")
		, m_io_key(*this, "KEY%u", 0U)
	{
	}

	void init_lhbv33c() ATTR_COLD;
	void init_drgnwrldv21j() ATTR_COLD;
	void init_wlcc() ATTR_COLD;
	void init_nkishusp() ATTR_COLD;
	void init_drgnwrldv21() ATTR_COLD;
	void init_dbc() ATTR_COLD;
	void init_lhb() ATTR_COLD;
	void init_drgnwrld() ATTR_COLD;
	void init_drgnwrldv30() ATTR_COLD;
	void init_drgnwrldv11h() ATTR_COLD;
	void init_lhb2() ATTR_COLD;
	void init_lhb2cpgs() ATTR_COLD;
	void init_lhb3() ATTR_COLD;
	void init_xymg() ATTR_COLD;
	void init_xymga() ATTR_COLD;
	void init_drgnwrldv10c() ATTR_COLD;
	void init_drgnwrldv20j() ATTR_COLD;
	void init_drgnwrldv40k() ATTR_COLD;
	void init_ryukobou() ATTR_COLD;
	void init_tygn() ATTR_COLD;
	void init_sdmg2641c() ATTR_COLD;

	void drgnwrld(machine_config &config) ATTR_COLD;
	void drgnwrld_igs012(machine_config &config) ATTR_COLD;
	void lhb(machine_config &config) ATTR_COLD;
	void wlcc(machine_config &config) ATTR_COLD;
	void xymg(machine_config &config) ATTR_COLD;
	void xymga(machine_config &config) ATTR_COLD;
	void sdmg2641c(machine_config &config) ATTR_COLD;
	void lhb2(machine_config &config) ATTR_COLD;
	void lhb2cpgs(machine_config &config) ATTR_COLD;

protected:
	required_device<okim6295_device> m_oki;
	optional_device<hopper_device> m_hopper;
	optional_ioport_array<5> m_io_key;

	u16 m_input_sel = 0;
	u16 m_lhb_irq_enable = 0;

	virtual void machine_start() override ATTR_COLD;

	void igs011_oki_base(machine_config &config) ATTR_COLD;
	void igs011_oki_opll_base(machine_config &config) ATTR_COLD;
	void xymg_base(machine_config &config) ATTR_COLD;

	u16 key_matrix_r();

	void lhb_prot_swap_w(u8 data);
	void nkishusp_prot_swap_w(u8 data);
	void xymga_prot_step1_w(u8 data);
	void xymga_prot_step2_w(u8 data);
	void wlcc_prot_swap_w(u8 data);
	u16 lhb_prot_r();
	u16 dbc_prot_r();
	u16 ryukobou_prot_r();
	u16 lhb2_prot_r();
	u16 lhb3_prot_r();
	u16 nkishusp_prot_r();
	template <u8 Which> u16 wlcc_prot_r();
	void drgnwrld_igs003_a_w(u8 data);
	void lhb_inputs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 lhb_inputs_r(offs_t offset);
	void igs003_inputs_w(u8 data);
	void lhb2_igs003_c_w(u8 data);
	void wlcc_igs003_c_w(u8 data);
	void lhb_irq_enable_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void lhb_okibank_w(u8 data);

	TIMER_DEVICE_CALLBACK_MEMBER(lhb_timer_irq_cb);

	INTERRUPT_GEN_MEMBER(lhb_vblank_irq);

	void wlcc_decrypt() ATTR_COLD;
	void lhb_decrypt() ATTR_COLD;
	void drgnwrld_type3_decrypt() ATTR_COLD;
	void drgnwrld_type2_decrypt() ATTR_COLD;
	void drgnwrld_type1_decrypt() ATTR_COLD;
	void drgnwrldv40k_decrypt() ATTR_COLD;
	void lhb2_decrypt() ATTR_COLD;
	void lhb3_decrypt() ATTR_COLD;
	void nkishusp_decrypt() ATTR_COLD;
	void dbc_decrypt() ATTR_COLD;
	void ryukobou_decrypt() ATTR_COLD;
	void tygn_decrypt() ATTR_COLD;

	void drgnwrld_mem(address_map &map) ATTR_COLD;
	void drgnwrld_igs012_mem(address_map &map) ATTR_COLD;
	void lhb_mem(address_map &map) ATTR_COLD;
	void lhb2_mem(address_map &map) ATTR_COLD;
	void lhb3_mem(address_map &map) ATTR_COLD;
	void nkishusp_mem(address_map &map) ATTR_COLD;
	void sdmg2641c_mem(address_map &map) ATTR_COLD;
	void wlcc_mem(address_map &map) ATTR_COLD;
	void xymg_base_mem(address_map &map) ATTR_COLD;
	void xymg_mem(address_map &map) ATTR_COLD;
	void xymga_mem(address_map &map) ATTR_COLD;
};

// With a serial port
class igs011_serial_state : public igs011_oki_state
{
public:
	igs011_serial_state(const machine_config &mconfig, device_type type, const char *tag)
		: igs011_oki_state(mconfig, type, tag)
	{
	}

	void lhb3(machine_config &config) ATTR_COLD;
	void nkishusp(machine_config &config) ATTR_COLD;
	void tygn(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	emu_timer *m_serial_timer = nullptr;
	u8 m_serial_ctrl = 0;
	u8 m_serial_tx = 0;
	u8 m_serial_rx = 0;
	u8 m_serial_status = 0;
	bool m_serial_rx_irq = false;

	void serial_tx_w(u8 data);
	void serial_ctrl_w(u8 data);
	u8 serial_rx_r();
	u8 serial_status_r();
	u8 serial_irq_r();
	u8 serial_irq_causes() const;
	TIMER_CALLBACK_MEMBER(serial_tx_done);
	void serial_update_irq();

	void serial_map(address_map &map) ATTR_COLD;
	void lhb3_serial_mem(address_map &map) ATTR_COLD;
	void nkishusp_serial_mem(address_map &map) ATTR_COLD;
};

// With ICS2115 sound and trackball inputs
class vbowl_state : public igs011_state_base
{
public:
	vbowl_state(const machine_config &mconfig, device_type type, const char *tag)
		: igs011_state_base(mconfig, type, tag)
		, m_ics(*this, "ics")
		, m_vbowl_trackball(*this, "vbowl_trackball")
		, m_io_an(*this, "AN%u", 0U)
		, m_io_conf(*this, "CONF")
		, m_trackball_count{ 0, 0 }
	{
	}

	void init_vbowl() ATTR_COLD;
	void init_vbowlj() ATTR_COLD;
	void init_vbowlhk() ATTR_COLD;

	void vbowl(machine_config &config) ATTR_COLD;
	void vbowlhk(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<ics2115_device> m_ics;
	required_shared_ptr<u16> m_vbowl_trackball;
	required_ioport_array<2> m_io_an;
	required_ioport m_io_conf;

	u16 m_trackball_count[2];

	void vbowl_prot_swap_w(u8 data);
	u16 vbowl_prot_r();
	u16 vbowlhk_prot_r();
	void vbowl_igs003_c_w(u8 data);
	u16 vbowl_unk_r();
	void vbowl_pen_hi_w(u8 data);
	void vbowl_link_0_w(u16 data);
	void vbowl_link_1_w(u16 data);
	void vbowl_link_2_w(u16 data);
	void vbowl_link_3_w(u16 data);
	void sound_irq(int state);

	void screen_vblank(int state);
	void vbowl_decrypt();
	void vbowlhk_decrypt();

	void vbowl_mem(address_map &map) ATTR_COLD;
	void vbowlhk_mem(address_map &map) ATTR_COLD;
};


/***************************************************************************

    Common functions

***************************************************************************/

void igs011_state_base::machine_start()
{
	save_item(NAME(m_dips_sel));
	save_item(NAME(m_prot));
}

void igs011_oki_state::machine_start()
{
	igs011_state_base::machine_start();

	m_lhb_irq_enable = 0;

	save_item(NAME(m_input_sel));
	save_item(NAME(m_lhb_irq_enable));
}

void igs011_serial_state::machine_start()
{
	igs011_oki_state::machine_start();

	m_serial_timer = timer_alloc(FUNC(igs011_serial_state::serial_tx_done), this);

	save_item(NAME(m_serial_ctrl));
	save_item(NAME(m_serial_tx));
	save_item(NAME(m_serial_rx));
	save_item(NAME(m_serial_status));
	save_item(NAME(m_serial_rx_irq));
}

void igs011_serial_state::machine_reset()
{
	igs011_oki_state::machine_reset();

	m_serial_timer->adjust(attotime::never);
	m_serial_ctrl = 0;
	m_serial_status = 0x10;
	m_serial_rx_irq = false;
	serial_update_irq();
}

void vbowl_state::machine_start()
{
	igs011_state_base::machine_start();

	save_item(NAME(m_trackball_count));
}

// Inputs

u16 igs011_oki_state::key_matrix_r()
{
	u16 ret = 0xffff;

	for (int i = 0; i < 5; i++)
	{
		if (BIT(~m_input_sel, i))
			ret &= m_io_key[i]->read();
	}

	return ret;
}

void igs011_state_base::dips_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_dips_sel);
}

template<unsigned Num>
u16 igs011_state_base::dips_r()
{
	// Games have 2 to 5 banks of DIP switches

	u16 ret = 0xff;
	for (int i = 0; i < Num; i++)
	{
		if (BIT(~m_dips_sel, i))
			ret &= m_io_dsw[i]->read();
	}

	return  (ret & 0xff) | (m_igs011->blitter_busy_r() ? 0x0100 : 0x0000);
}

/***************************************************************************

    Code Decryption

***************************************************************************/

void igs011_oki_state::wlcc_decrypt()
{
	u16 *src = (u16 *)m_maincpu_region->base();

	const int rom_size = 0x80000;

	for (int i=0; i<rom_size/2; i++)
	{
		u16 x = src[i];

		if ((i & 0x2000) == 0x0000 || (i & 0x0004) == 0x0000 || (i & 0x0090) == 0x0000)
			x ^= 0x0004;
		if ((i & 0x0100) == 0x0100 || (i & 0x0040) == 0x0040 || (i & 0x0012) == 0x0012)
			x ^= 0x0020;
		if ((i & 0x2400) == 0x0000 || (i & 0x4100) == 0x4100 || ((i & 0x2000) == 0x2000 && (i & 0x0c00) != 0x0000))
			x ^= 0x0200;
		if ((x & 0x0024) == 0x0004 || (x & 0x0024) == 0x0020)
			x ^= 0x0024;
		src[i] = x;
	}
}


void igs011_oki_state::lhb_decrypt()
{
	u16 *src = (u16 *)m_maincpu_region->base();

	const int rom_size = 0x80000;

	for (int i=0; i<rom_size/2; i++)
	{
		u16 x = src[i];

		if ((i & 0x1100) != 0x0100)
			x ^= 0x0200;

		if ((i & 0x0150) != 0x0000 && (i & 0x0152) != 0x0010)
			x ^= 0x0004;

		if ((i & 0x2084) != 0x2084 && (i & 0x2094) != 0x2014)
			x ^= 0x0020;

		src[i] = x;
	}
}


void igs011_oki_state::drgnwrld_type3_decrypt()
{
	u16 *src = (u16 *)m_maincpu_region->base();

	const int rom_size = 0x80000;

	for (int i=0; i<rom_size/2; i++)
	{
		u16 x = src[i];

		if ((i & 0x2000) == 0x0000 || (i & 0x0004) == 0x0000 || (i & 0x0090) == 0x0000)
			x ^= 0x0004;

		if ((i & 0x0100) == 0x0100 || (i & 0x0040) == 0x0040 || (i & 0x0012) == 0x0012)
			x ^= 0x0020;

		if ((((i & 0x1000) == 0x1000) ^ ((i & 0x0100) == 0x0100))
			|| (i & 0x0880) == 0x0800 || (i & 0x0240) == 0x0240)
				x ^= 0x0200;

		if ((x & 0x0024) == 0x0004 || (x & 0x0024) == 0x0020)
			x ^= 0x0024;

		src[i] = x;
	}
}

void igs011_oki_state::drgnwrld_type2_decrypt()
{
	u16 *src = (u16 *)m_maincpu_region->base();

	const int rom_size = 0x80000;

	for (int i=0; i<rom_size/2; i++)
	{
		u16 x = src[i];

		if (((i & 0x000090) == 0x000000) || ((i & 0x002004) != 0x002004))
			x ^= 0x0004;

		if ((((i & 0x000050) == 0x000000) || ((i & 0x000142) != 0x000000)) && ((i & 0x000150) != 0x000000))
			x ^= 0x0020;

		if (((i & 0x004280) == 0x004000) || ((i & 0x004080) == 0x000000))
			x ^= 0x0200;

		if ((i & 0x0011a0) != 0x001000)
			x ^= 0x0200;

		if ((i & 0x000180) == 0x000100)
			x ^= 0x0200;

		if ((x & 0x0024) == 0x0020 || (x & 0x0024) == 0x0004)
			x ^= 0x0024;

		src[i] = x;
	}
}

void igs011_oki_state::drgnwrld_type1_decrypt()
{
	u16 *src = (u16 *)m_maincpu_region->base();

	const int rom_size = 0x80000;

	for (int i=0; i<rom_size/2; i++)
	{
		u16 x = src[i];

		if ((i & 0x2000) == 0x0000 || (i & 0x0004) == 0x0000 || (i & 0x0090) == 0x0000)
			x ^= 0x0004;

		if ((i & 0x0100) == 0x0100 || (i & 0x0040) == 0x0040 || (i & 0x0012) == 0x0012)
			x ^= 0x0020;
/*
        if ((((i & 0x1000) == 0x1000) ^ ((i & 0x0100) == 0x0100))
            || (i & 0x0880) == 0x0800 || (i & 0x0240) == 0x0240)
                x ^= 0x0200;
*/
		if ((x & 0x0024) == 0x0004 || (x & 0x0024) == 0x0020)
			x ^= 0x0024;

		src[i] = x;
	}
}

void igs011_oki_state::drgnwrldv40k_decrypt()
{
	drgnwrld_type1_decrypt();

	u16 *src = (u16 *)m_maincpu_region->base();

	const int rom_size = 0x80000;

	for (int i = 0; i < rom_size / 2; i++)
	{
		u16 x = src[i];

		if ((i & 0x0800) != 0x0800)
			x ^= 0x0200;

		if (((i & 0x3a00) == 0x0a00) ^ ((i & 0x3a00) == 0x2a00))
			x ^= 0x0200;

		if (((i & 0x3ae0) == 0x0860) ^ ((i & 0x3ae0) == 0x2860))
			x ^= 0x0200;

		if (((i & 0x1c00) == 0x1800) ^ ((i & 0x1e00) == 0x1e00))
			x ^= 0x0200;

		if ((i & 0x1ee0) == 0x1c60)
			x ^= 0x0200;

		src[i] = x;
	}
}

void igs011_oki_state::lhb2_decrypt()
{
	const int rom_size = 0x80000;
	u16 *src = (u16 *)m_maincpu_region->base();
	std::unique_ptr<u16 []> result_data(new u16[rom_size / 2]);

	for (int i=0; i<rom_size/2; i++)
	{
		u16 x = src[i];

		if ((i & 0x0054) != 0x0000 && (i & 0x0056) != 0x0010)
			x ^= 0x0004;

		if ((i & 0x0204) == 0x0000)
			x ^= 0x0008;

		if ((i & 0x3080) != 0x3080 && (i & 0x3090) != 0x3010)
			x ^= 0x0020;

		const int j = bitswap<24>(i, 23,22,21,20,19,18,17,16,15,14,13, 8, 11,10, 9, 2, 7,6,5,4,3, 12, 1,0);

		result_data[j] = x;
	}

	memcpy(src,&result_data[0],rom_size);
}

void igs011_oki_state::lhb3_decrypt()
{
	const int rom_size = 0x80000;
	u16 *src = (u16 *)m_maincpu_region->base();
	std::unique_ptr<u16 []> result_data(new u16[rom_size / 2]);

	for (int i = 0; i < rom_size / 2; i++)
	{
		u16 x = src[i];

		// lhb2 address scrambling
		const int j = bitswap<24>(i, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 8, 11, 10, 9, 2, 7, 6, 5, 4, 3, 12, 1, 0);

		if ((j & 0x0100) || (j & 0x0040) || ((j & 0x0010)&&(j & 0x0002)))
			x ^= 0x00004;

		if ((j & 0x5000) == 0x1000)
			x ^= 0x0008;

		if (!(j & 0x0004) || !(j & 0x2000) || (!(j & 0x0080) && !(j & 0x0010)))
			x ^= 0x0020;

		result_data[j] = x;
	}

	memcpy(src, &result_data[0], rom_size);
}


// xor similar to ryukobou (both sets are Japan), address scrambling from lhb2
void igs011_oki_state::nkishusp_decrypt()
{
	const int rom_size = 0x80000;
	u16 *src = (u16 *)m_maincpu_region->base();
	std::unique_ptr<u16 []> result_data(new u16[rom_size / 2]);

	for (int i=0; i<rom_size/2; i++)
	{
		u16 x = src[i];

		// lhb2 address scrambling
		const int j = bitswap<24>(i, 23,22,21,20,19,18,17,16,15,14,13, 8, 11,10, 9, 2, 7,6,5,4,3, 12, 1,0);

		// ryukobou xor:

//      if ((j & 0x00100) && (j & 0x00400))
//          x ^= 0x0200;

		if (!(j & 0x00004) || !(j & 0x02000) || (!(j & 0x00080) && !(j & 0x00010)))
			x ^= 0x0020;

		if ((j & 0x00100) || (j & 0x00040) || ((j & 0x00010)&&(j & 0x00002)))
			x ^= 0x00004;

		// additional xor:

		if (!(j & 0x4000) && (j & 0x1000) && (j & 0x00200))
			x ^= 0x0008;

		result_data[j] = x;
	}

	memcpy(src,&result_data[0],rom_size);
}


void igs011_oki_state::tygn_decrypt()
{
	const int rom_size = 0x80000;
	u16 *src = (u16 *)m_maincpu_region->base();
	std::unique_ptr<u16 []> result_data(new u16[rom_size / 2]);

	for (int i = 0; i < rom_size / 2; i++)
	{
		u16 x = src[i];

		const int j = bitswap<24>(i, 23, 22, 21, 20, 19, 18, 17, 0, 2, 9, 5, 12, 14, 11, 8, 6, 3, 4, 16, 15, 13, 10, 7, 1);

		if ((j & 0x0280 / 2) || ((j & 0x0024 / 2) == 0x24 / 2))
			x ^= 0x0004;

		if (j & 0x4000 / 2)
		{
			if ((((j & 0x100 / 2) == 0x000 / 2) ^ ((j & 0x128 / 2) == 0x028 / 2)) || ((j & 0x108 / 2) == 0x100 / 2))
				x ^= 0x0020;
		}
		else
			x ^= 0x0020;

		if (!(j & 0x8000 / 2) && (j & 0x2000 / 2))
			x ^= 0x0008;

		result_data[j] = x;
	}

	memcpy(src, &result_data[0], rom_size);
}


void vbowl_state::vbowl_decrypt()
{
	u16 *src = (u16 *)m_maincpu_region->base();

	const int rom_size = 0x80000;

	for (int i=0; i<rom_size/2; i++)
	{
		u16 x = src[i];

		if ((i & 0x4100) == 0x0100)
			x ^= 0x0200;

		if ((i & 0x4000) == 0x4000 && (i & 0x0300) != 0x0100)
			x ^= 0x0200;

		if ((i & 0x5700) == 0x5100)
			x ^= 0x0200;

		if ((i & 0x5500) == 0x1000)
			x ^= 0x0200;

		if ((i & 0x0140) != 0x0000 || (i & 0x0012) == 0x0012)
			x ^= 0x0004;

		if ((i & 0x2004) != 0x2004 || (i & 0x0090) == 0x0000)
			x ^= 0x0020;

		src[i] = x;
	}
}


void vbowl_state::vbowlhk_decrypt()
{
	vbowl_decrypt();

	u16 *src = (u16 *)m_maincpu_region->base();

	const int rom_size = 0x80000;

	for (int i=0; i<rom_size/2; i++)
	{
		u16 x = src[i];

		// 00000-07fff, 2000 off, 800 off, 400 off, 200 on (2xx, 3xx)
		if ((i & 0xd700) == 0x0100)
			x ^= 0x0200;

		// 00000-07fff, 800 on, 400 off, 200 on (axx, bxx)
		if ((i & 0xc700) == 0x0500)
			x ^= 0x0200;

		// 08000-0ffff, 2000 off, 800 off, 200 off (0xx, 1xx, 4xx, 5xx)
		if ((i & 0xd500) == 0x4000)
			x ^= 0x0200;

		// 08000-0ffff, 800 on, 200 off (8xx, 9xx, cxx, dxx)
		if ((i & 0xc500) == 0x4400)
			x ^= 0x0200;

		// 10000-17fff, 2000 off, 200 off (0xx, 1xx, 4xx, 5xx, 8xx, 9xx, cxx, dxx)
		if ((i & 0xd100) == 0x8000)
			x ^= 0x0200;

		// 10000-17fff, 2000 on, 800 on, 400 off, 200 on (axx, bxx)
		if ((i & 0xd700) == 0x9500)
			x ^= 0x0200;

		// 18000-1ffff, 2000 off, 400 off, 200 on (2xx, 3xx, axx, bxx)
		if ((i & 0xd300) == 0xc100)
			x ^= 0x0200;

		// 18000-1ffff, 2000 on, 800 on, 200 off (8xx, 9xx, cxx, dxx)
		if ((i & 0xd500) == 0xd400)
			x ^= 0x0200;

		src[i] = x;
	}
}


void igs011_oki_state::dbc_decrypt()
{
	u16 *src = (u16 *)m_maincpu_region->base();

	const int rom_size = 0x80000;

	for (int i=0; i<rom_size/2; i++)
	{
		u16 x = src[i];

		if (i & 0x1000/2)
		{
			if (~i & 0x400/2)
				x ^= 0x0200;
		}

		if (i & 0x4000/2)
		{
			if (i & 0x100/2)
			{
				if (~i & 0x08/2)
					x ^= 0x0020;
			}
			else
			{
				if (~i & 0x28/2)
					x ^= 0x0020;
			}
		}
		else
		{
			x ^= 0x0020;
		}

		if (i & 0x200/2)
		{
			x ^= 0x0004;
		}
		else
		{
			if ((i & 0x80/2) == 0x80/2 || (i & 0x24/2) == 0x24/2)
				x ^= 0x0004;
		}

		src[i] = x;
	}
}


void igs011_oki_state::ryukobou_decrypt()
{
	u16 *src = (u16 *)m_maincpu_region->base();
	const int rom_size = 0x80000;

	for (int i=0; i<rom_size/2; i++)
	{
		u16 x = src[i];

		if ((i & 0x00100) && (i & 0x00400))
			x ^= 0x0200;

		if (!(i & 0x00004) || !(i & 0x02000) || (!(i & 0x00080) && !(i & 0x00010)))
			x ^= 0x0020;

		if ((i & 0x00100) || (i & 0x00040) || ((i & 0x00010)&&(i & 0x00002)))
			x ^= 0x00004;

		src[i] = x;
	}
}


/***************************************************************************

    IGS011 Protection: see igs/igs011_video.cpp

    Per-game protection ("CHECK PORT ERROR")

    This is probably not part of the IGS011 nor the IGS012, but a game specific protection
    similar to the IGS011 protection.

    The chip holds an internal value. It is manipulated by issuing commands,
    where each command is assigned a specific address range (fixed per game), and is
    triggered by writing a specific byte value to that range. Possible commands:

    - INC:   increment value
    - DEC:   decrement value
    - SWAP:  value = bitswap1(value). Bitswap1 is game specific.
    - RESET: value = 0

    The protection value is read from an additional address range:
    - READ:  read bitswap2(value). Only 1 bit is checked. Bitswap2 is game specific.

***************************************************************************/



// Prot2

// drgnwrld (33)
void igs011_state_base::prot_reset_w(u8 data)
{
	m_prot = 0x00;
}

// wlcc
u16 igs011_state_base::prot_reset_r()
{
	if (!machine().side_effects_disabled())
		m_prot = 0x00;

	return 0;
}


// lhb2 (55), lhb/dbc/ryukobou (33)
void igs011_state_base::prot_inc_w(u8 data)
{
//  if ((data & 0xff) == 0x55)
	{
		m_prot++;
	}
//  else
//      logerror("%s: warning, unknown prot_inc_w( %04x, %04x )\n", machine().describe_context(), offset, data);
}

// vbowl (33)
void igs011_state_base::prot_dec_w(u8 data)
{
//  if ((data & 0xff) == 0x33)
	{
		m_prot--;
	}
//  else
//      logerror("%s: warning, unknown prot_dec_w( %04x, %04x )\n", machine().describe_context(), offset, data);
}


void igs011_state_base::drgnwrld_prot_swap_w(u8 data)
{
//  if ((data & 0xff) == 0x33)
	{
		// (b3&b0) . b2 . (b0|b1) . (b2^!b4) . (!b1^b3)
		u8 const x = m_prot;
		m_prot = ((BIT(x,3)&BIT(x,0))<<4) | (BIT(x,2)<<3) | ((BIT(x,0)|BIT(x,1))<<2) | ((BIT(x,2)^BIT(x,4)^1)<<1) | (BIT(x,1)^1^BIT(x,3));
	}
//  else
//      logerror("%s: warning, unknown prot_swap_w( %04x, %04x )\n", machine().describe_context(), offset, data);
}

// lhb, xymg, lhb2, lhb2cpgs, lhb3, tygn
void igs011_oki_state::lhb_prot_swap_w(u8 data)
{
//  if ((data & 0xff) == 0x33)
	{
		// (!b0|b1) . b2 . (b0&b1)
		u8 const x = m_prot;
		m_prot = (((BIT(x,0)^1)|BIT(x,1))<<2) | (BIT(x,2)<<1) | (BIT(x,0)&BIT(x,1));
	}
//  else
//      logerror("%s: warning, unknown prot_swap_w( %04x, %04x )\n", machine().describe_context(), offset, data);
}

// nkishusp
void igs011_oki_state::nkishusp_prot_swap_w(u8 data)
{
	// (!b0|b1) . (!b2|b0) . (b2&!b1)
	u8 const x = m_prot;
	m_prot = (((BIT(x,0)^1)|BIT(x,1))<<2) | (((BIT(x,2)^1)|BIT(x,0))<<1) | (BIT(x,2)&(BIT(x,1)^1));
}

/*
    xymga has checks that xymg doesn't have. Besides the usual commands, it
    resets the value by writing to 11600-117ff, then steps it many times
    with writes to 10c00-10dff (or 10a00-10bff) and checks bit 9 of the
    usual read after each step. The expected sequences are 1,0,1,1,0,1,1
    (repeated) for 10c00 and 0,1,1,1 (repeated) for 10a00.

    The two step functions below are just one choice (the simplest found)
    that produces these sequences with the usual read bitswap, the real
    PAL equations are unknown.
*/
void igs011_oki_state::xymga_prot_step1_w(u8 data)
{
	// (b0^b1) . b2 . !b1
	u8 const x = m_prot;
	m_prot = ((BIT(x,0)^BIT(x,1))<<2) | (BIT(x,2)<<1) | (BIT(x,1)^1);
}

void igs011_oki_state::xymga_prot_step2_w(u8 data)
{
	// !b1 . b2 . 1
	u8 const x = m_prot;
	m_prot = ((BIT(x,1)^1)<<2) | (BIT(x,2)<<1) | 1;
}

// wlcc
void igs011_oki_state::wlcc_prot_swap_w(u8 data)
{
//  if ((data & 0xff) == 0x33)
	{
		// (b3 ^ b2) . (b2 ^ b1) . (b1 ^ b0) . !(b4 ^ b0) . !(b4 ^ b3)
		u8 const x = m_prot;
		m_prot = ((BIT(x,3)^BIT(x,2))<<4) | ((BIT(x,2)^BIT(x,1))<<3) | ((BIT(x,1)^BIT(x,0))<<2) | ((BIT(x,4)^BIT(x,0)^1)<<1) | (BIT(x,4)^BIT(x,3)^1);
	}
//  else
//      logerror("%s: warning, unknown prot_swap_w( %04x, %04x )\n", machine().describe_context(), offset, data);
}

// vbowl
void vbowl_state::vbowl_prot_swap_w(u8 data)
{
//  if ((data & 0xff) == 0x33)
	{
		// (b3 ^ b2) . (b2 ^ b1) . (b1 ^ b0) . (b4 ^ b0) . (b4 ^ b3)
		u8 const x = m_prot;
		m_prot = ((BIT(x,3)^BIT(x,2))<<4) | ((BIT(x,2)^BIT(x,1))<<3) | ((BIT(x,1)^BIT(x,0))<<2) | ((BIT(x,4)^BIT(x,0))<<1) | (BIT(x,4)^BIT(x,3));
	}
//  else
//      logerror("%s: warning, unknown prot_swap_w( %04x, %04x )\n", machine().describe_context(), offset, data);
}


// drgnwrld
u16 igs011_state_base::drgnwrldv40k_prot_r()
{
	// b9 = (!b4 & !b0) | ((b3 & !b2) & !(b1 ^ b0)
	u8 const x = m_prot;
	u8 const b9 = ((BIT(x, 4) ^ 1) & (BIT(x, 0) ^ 1)) | (((BIT(x, 3)) & ((BIT(x, 2)) ^ 1)) & (((BIT(x, 1)) ^ (BIT(x, 0))) ^ 1));
	return (b9 << 9);
}

u16 igs011_state_base::drgnwrldv21_prot_r()
{
	// b9 = (!b4) | (!b0 & b2) | (!(b3 ^ b1) & !(!(b4 & b0) | b2))
	u8 const x = m_prot;
	u8 const b9 = (BIT(x,4)^1) | ((BIT(x,0)^1) & BIT(x,2)) | ((BIT(x,3)^BIT(x,1)^1) & ((((BIT(x,4)^1) & BIT(x,0)) | BIT(x,2))^1) );
	return (b9 << 9);
}
u16 igs011_state_base::drgnwrldv20j_prot_r()
{
	// b9 = (!b4 | !b0) | !(b3 | b1) | !(b2 & b0)
	u8 const x = m_prot;
	u8 const b9 = ((BIT(x,4)^1) | (BIT(x,0)^1)) | ((BIT(x,3) | BIT(x,1))^1) | ((BIT(x,2) & BIT(x,0))^1);
	return (b9 << 9);
}

// lhb, xymg
u16 igs011_oki_state::lhb_prot_r()
{
	// b9 = !b2 | (b1 & b0)
	u8 const x = m_prot;
	u8 const b9 = (BIT(x,2)^1) | (BIT(x,1) & BIT(x,0));
	return (b9 << 9);
}

// dbc
u16 igs011_oki_state::dbc_prot_r()
{
	// b9 = !b1 | (!b0 & b2)
	u8 const x = m_prot;
	u8 const b9 = (BIT(x,1)^1) | ((BIT(x,0)^1) & BIT(x,2));
	return (b9 << 9);
}

// ryukobou
u16 igs011_oki_state::ryukobou_prot_r()
{
	// b9 = (!b1 | b2) & b0
	u8 const x = m_prot;
	u8 const b9 = ((BIT(x,1)^1) | BIT(x,2)) & BIT(x,0);
	return (b9 << 9);
}

// lhb2
u16 igs011_oki_state::lhb2_prot_r()
{
	// b3 = !b2 | !b1 | b0
	u8 const x = m_prot;
	u8 const b3 = (BIT(x,2)^1) | (BIT(x,1)^1) | BIT(x,0);
	return (b3 << 3);
}

// lhb2cpgs, lhb3, tygn
u16 igs011_oki_state::lhb3_prot_r()
{
	// b3 = b1 | (b0 & !b2)
	u8 const x = m_prot;
	u8 const b3 = BIT(x,1) | (BIT(x,0) & (BIT(x,2)^1));
	return (b3 << 3);
}

// nkishusp
u16 igs011_oki_state::nkishusp_prot_r()
{
	// b3 = b0 | (b1 & !b2)
	u8 const x = m_prot;
	u8 const b3 = BIT(x,0) | (BIT(x,1) & (BIT(x,2)^1));
	return (b3 << 3);
}

// wlcc: three read ranges with different bitswaps
template <u8 Which>
u16 igs011_oki_state::wlcc_prot_r()
{
	u8 const x = m_prot;
	u8 b9 = 0;

	switch (Which)
	{
		case 0: // b9 = (!b4 & b1) | (!b3 & !b0)
			b9 = ((BIT(x,4)^1) & BIT(x,1)) | ((BIT(x,3)^1) & (BIT(x,0)^1));
			break;

		case 1: // b9 = ((!b4 | !b2) & b3 & b0) | !(b1 & b0)
			b9 = (((BIT(x,4)^1) | (BIT(x,2)^1)) & BIT(x,3) & BIT(x,0)) | ((BIT(x,1) & BIT(x,0))^1);
			break;

		case 2: // b9 = (!b3 & !b0) | (b4 & b1 & !b2)
			b9 = ((BIT(x,3)^1) & (BIT(x,0)^1)) | (BIT(x,4) & BIT(x,1) & (BIT(x,2)^1));
			break;
	}

	return (b9 << 9);
}

// vbowl, vbowlj
u16 vbowl_state::vbowl_prot_r()
{
	// b9 = (!b4 & !b3) | !(b2 & b1) | !(b4 | b0)
	u8 const x = m_prot;
	u8 const b9 = ((BIT(x,4)^1) & (BIT(x,3)^1)) | ((BIT(x,2) & BIT(x,1))^1) | ((BIT(x,4) | BIT(x,0))^1);
	return (b9 << 9);
}

// vbowlhk
u16 vbowl_state::vbowlhk_prot_r()
{
	// b9 = (!b4 & !b3) | !(!b2 & !b1) | !(b4 | !b0)
	u8 const x = m_prot;
	u8 const b9 = ((BIT(x,4)^1) & (BIT(x,3)^1)) | (((BIT(x,2)^1) & (BIT(x,1)^1))^1) | ((BIT(x,4) | (BIT(x,0)^1))^1);
	return (b9 << 9);
}


/***************************************************************************

    IGS003 (8255 I/O + Protection)

***************************************************************************/

// the chip is on the low half of the data bus, the games set the port directions
// with byte writes to the even address, which the 68000 puts on both halves
void igs011_state_base::igs003_index_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_igs003->index_w(ACCESSING_BITS_0_7 ? (data & 0xff) : (data >> 8));
}

void igs011_state_base::igs003_data_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_igs003->data_w(ACCESSING_BITS_0_7 ? (data & 0xff) : (data >> 8));
}

void igs011_state_base::igs012_regs_w(offs_t offset, u16 data, u16 mem_mask)
{
	m_igs012->regs_w(offset, ACCESSING_BITS_0_7 ? (data & 0xff) : (data >> 8));
}

void igs011_oki_state::drgnwrld_igs003_a_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 1));
}

void igs011_oki_state::lhb_inputs_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_input_sel);

	if (ACCESSING_BITS_0_7)
	{
		machine().bookkeeping().coin_counter_w(0, BIT(data, 5)); // coin in
		machine().bookkeeping().coin_counter_w(1, BIT(data, 6)); // coin out
		m_hopper->motor_w(BIT(data, 7));
	}

	if (m_input_sel & (~0xff))
		logerror("%s: warning, unknown bits written in input_sel = %02x\n", machine().describe_context(), m_input_sel);

	//popmessage("sel2 %02x", m_input_sel & ~0x1f);
}

u16 igs011_oki_state::lhb_inputs_r(offs_t offset)
{
	switch (offset)
	{
		case 0: return m_input_sel;
		case 1: return key_matrix_r();
	}
	return 0;
}


// port A in lhb2, lhb2cpgs, lhb3, nkishusp, port B in xymg, sdmg2641c
void igs011_oki_state::igs003_inputs_w(u8 data)
{
	m_input_sel = data;

	machine().bookkeeping().coin_counter_w(0, BIT(data, 5)); // coin in
	machine().bookkeeping().coin_counter_w(1, BIT(data, 6)); // coin out
	m_hopper->motor_w(BIT(data, 7));
}

// port C in lhb2, lhb2cpgs, lhb3, nkishusp, tygn
void igs011_oki_state::lhb2_igs003_c_w(u8 data)
{
	m_igs011->blitter_pen_hi_w(data & 0x07);

	m_oki->set_rom_bank(BIT(data, 3));
}

void igs011_oki_state::wlcc_igs003_c_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0)); // coin in
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1)); // coin out

	m_oki->set_rom_bank(BIT(data, 4));
	m_hopper->motor_w(BIT(data, 5));
}

void vbowl_state::vbowl_igs003_c_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
}


/***************************************************************************

    Driver Inits

***************************************************************************/

// V0400O
void igs011_oki_state::init_drgnwrld()
{
	drgnwrld_type1_decrypt();
	m_igs011->drgnwrld_gfx_decrypt();
}

void igs011_oki_state::init_drgnwrldv30()
{
	drgnwrld_type1_decrypt();
	m_igs011->drgnwrld_gfx_decrypt();
}

void igs011_oki_state::init_drgnwrldv21()
{
	drgnwrld_type2_decrypt();
	m_igs011->drgnwrld_gfx_decrypt();
	m_maincpu->space(AS_PROGRAM).install_read_handler(0xd4c0, 0xd4ff, read16smo_delegate(*this, FUNC(igs011_oki_state::drgnwrldv21_prot_r)));
}

void igs011_oki_state::init_drgnwrldv21j()
{
	drgnwrld_type3_decrypt();
	m_igs011->drgnwrld_gfx_decrypt();
}

void igs011_oki_state::init_drgnwrldv20j()
{
	drgnwrld_type3_decrypt();
	m_igs011->drgnwrld_gfx_decrypt();
}

void igs011_oki_state::init_drgnwrldv40k()
{
	drgnwrldv40k_decrypt();
	m_igs011->drgnwrld_gfx_decrypt();

	m_maincpu->space(AS_PROGRAM).install_read_handler(0xd4c0, 0xd4ff, read16smo_delegate(*this, FUNC(igs011_oki_state::drgnwrldv40k_prot_r)));
}

void igs011_oki_state::init_drgnwrldv11h()
{
	drgnwrld_type1_decrypt();
	m_igs011->drgnwrld_gfx_decrypt();

	// the protection checks are already patched out in this set
}

void igs011_oki_state::init_drgnwrldv10c()
{
	drgnwrld_type1_decrypt();
	m_igs011->drgnwrld_gfx_decrypt();
}


void igs011_oki_state::init_lhb()
{
	lhb_decrypt();
}

void igs011_oki_state::init_lhbv33c()
{
	lhb_decrypt();
}

void igs011_oki_state::init_dbc()
{
	dbc_decrypt();

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x10600, 0x107ff, read16smo_delegate(*this, FUNC(igs011_oki_state::dbc_prot_r)));
}

void igs011_oki_state::init_ryukobou()
{
	ryukobou_decrypt();

	m_maincpu->space(AS_PROGRAM).install_read_handler(0x10600, 0x107ff, read16smo_delegate(*this, FUNC(igs011_oki_state::ryukobou_prot_r)));
}


void igs011_oki_state::init_xymg()
{
	lhb_decrypt();
}

void igs011_oki_state::init_xymga()
{
	u16 *src = (u16 *)m_maincpu_region->base();
	const int rom_size = 0x80000;

	for (int i = 0; i < rom_size / 2; i++)
	{
		u16 x = src[i];

		if ((i & 0x2300) == 0x2100 || ((i & 0x2000) == 0x0000 && ((i & 0x0300) != 0x0200) && ((i & 0x0300) != 0x0100)))
			x ^= 0x0200;

		if (!(i & 0x0004) || !(i & 0x2000) || (!(i & 0x0080) && !(i & 0x0010)))
			x ^= 0x0020;

		if ((i & 0x0100) || (i & 0x0040) || ((i & 0x0010) && (i & 0x0002)))
			x ^= 0x0004;

		src[i] = x;
	}
}

void igs011_oki_state::init_wlcc()
{
	wlcc_decrypt();
}


void igs011_oki_state::init_lhb2()
{
	lhb2_decrypt();
	m_igs011->lhb2_gfx_decrypt();
}

void igs011_oki_state::init_lhb2cpgs()
{
	lhb3_decrypt();
	m_igs011->lhb2_gfx_decrypt();
}

void igs011_oki_state::init_lhb3()
{
	lhb3_decrypt();
	m_igs011->lhb2_gfx_decrypt();
}

void igs011_oki_state::init_tygn()
{
	tygn_decrypt();

	m_igs011->lhb2_gfx_decrypt();
}

void vbowl_state::init_vbowl()
{
	u16 *rom = (u16 *)m_maincpu_region->base();

	vbowl_decrypt();
	m_igs011->vbowl_gfx_unpack();

	// Patch the bad dump so that it doesn't reboot at the end of a game (the patched value is from vbowlj)
	rom[0x080e0/2] = 0xe549;    // 0080E0: 0449 dc.w $0449; ILLEGAL
}

void vbowl_state::init_vbowlj()
{
	vbowl_decrypt();
	m_igs011->vbowl_gfx_unpack();
}


void vbowl_state::init_vbowlhk()
{
	vbowlhk_decrypt();
	m_igs011->vbowl_gfx_unpack();
}


void igs011_oki_state::init_nkishusp()
{
	nkishusp_decrypt();
	m_igs011->lhb2_gfx_decrypt();
}

void igs011_oki_state::init_sdmg2641c()
{
	u16 *src = (u16 *)m_maincpu_region->base();

	const int rom_size = 0x80000;

	for (int i = 0; i < rom_size/2; i++)
	{
		u16 x = src[i];

		if ((i & 0x0300) != 0x0100)   // lhb: 0x1100
			x ^= 0x0200;

		if ((i & 0x0150) != 0x0000 && (i & 0x0152) != 0x0010)
			x ^= 0x0004;

		if ((i & 0x2084) != 0x2084 && (i & 0x2094) != 0x2014)
			x ^= 0x0020;

		src[i] = x;
	}
}

/***************************************************************************

    Memory Maps

***************************************************************************/

void igs011_oki_state::drgnwrld_mem(address_map &map)
{
//  drgnwrld: IGS011 protection dynamically mapped at 1dd7x
//  map(0x01dd70, 0x01dd77).w(m_igs011, FUNC(igs011_device::prot_w)).umask16(0xff00);
//  map(0x01dd78, 0x01dd79).r(m_igs011, FUNC(igs011_device::prot_r));

	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram().share("nvram");
	map(0x200000, 0x200fff).rw(m_igs011, FUNC(igs011_device::priority_ram_r), FUNC(igs011_device::priority_ram_w));
	map(0x400000, 0x400fff).umask16(0x00ff).rw(m_palette, FUNC(palette_device::read8), FUNC(palette_device::write8)).share("palette");
	map(0x401000, 0x401fff).umask16(0x00ff).rw(m_palette, FUNC(palette_device::read8_ext), FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0x500000, 0x500001).portr("COIN");
	map(0x600001, 0x600001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x700000, 0x700003).umask16(0x00ff).w("ymsnd", FUNC(ym3812_device::write));

	map(0x800000, 0x800001).w(FUNC(igs011_oki_state::igs003_index_w));
	map(0x800002, 0x800003).w(FUNC(igs011_oki_state::igs003_data_w));
	map(0x800003, 0x800003).r(m_igs003, FUNC(igs003_device::data_r));

	map(0xa20000, 0xa20001).w(m_igs011, FUNC(igs011_device::priority_w));
	map(0xa40000, 0xa40001).w(FUNC(igs011_oki_state::dips_w));

	map(0xa50000, 0xa50001).w(m_igs011, FUNC(igs011_device::prot_addr_w));

	map(0xa58000, 0xa5cfff).m(m_igs011, FUNC(igs011_device::map));
	map(0xa88000, 0xa88001).r(FUNC(igs011_oki_state::dips_r<3>));
}

void igs011_oki_state::drgnwrld_igs012_mem(address_map &map)
{
	drgnwrld_mem(map);

	// IGS012
	map(0x001600, 0x00167f).mirror(0x01c000).m(m_igs012, FUNC(igs012_device::map));

	map(0x00d400, 0x00d43f).w(FUNC(igs011_oki_state::prot_dec_w));   // dec   (33)
	map(0x00d440, 0x00d47f).w(FUNC(igs011_oki_state::drgnwrld_prot_swap_w));   // swap  (33)
	map(0x00d480, 0x00d4bf).w(FUNC(igs011_oki_state::prot_reset_w));   // reset (33)
	map(0x00d4c0, 0x00d4ff).r(FUNC(igs011_oki_state::drgnwrldv20j_prot_r));   // read

	map(0x902000, 0x902fff).w(FUNC(igs011_oki_state::igs012_regs_w));
}


// Only values 0 and 7 are written (1 bit per irq source?)
void igs011_oki_state::lhb_irq_enable_w(offs_t offset, u16 data, u16 mem_mask)
{
	COMBINE_DATA(&m_lhb_irq_enable);
}

void igs011_oki_state::lhb_okibank_w(u8 data)
{
	m_oki->set_rom_bank(BIT(data, 1));

	if (data & (~0x2))
		logerror("%s: warning, unknown bits written in oki bank = %02x\n", machine().describe_context(), data);

//  popmessage("oki %04x",data);
}

/***************************************************************************

    Serial port (lhb3, nkishusp, tygn)

    The chip is not identified, the registers are deduced from the code:

    a00000  W  transmit data
    a08000  W  control: bit 0 = TX interrupt enable, bit 1 = RX interrupt enable,
               bit 5 low clears the error flags, bits 2, 3, 6 unknown
    a10000  W  only 0x81 is written at boot
    a80000  R  receive data
    a90000  R  status: bit 0 = RX ready, bits 1-3 = errors, bit 4 = TX ready
    a98000  R  interrupt cause: bit 2 = RX, bit 3 = TX

    The port raises IRQ 4, the handler reads a90000, a80000 and a98000 in
    this order, so the RX cause is latched until a98000 is read.

    At random times the games send a packet (0xfa, 0x04, random byte,
    checksum) and check, after a delay, that the received packet buffer holds
    the same random byte. Every mismatch increments an error counter (the RX
    error flags increment them too), and when a counter is above a random
    threshold the games corrupt their state on purpose and stop with
    SYSTEM ERROR.

    What answers on the real hardware is unknown (a device on the board,
    another cabinet, a loopback plug). Here every transmitted byte is looped
    back to the receiver, which is enough to pass the check, and there are
    never errors. The baud rate is unknown too, a character takes the time
    of 9600 baud 8N1.

    TODO: identify the chip and what is connected to it

***************************************************************************/

void igs011_serial_state::serial_update_irq()
{
	m_maincpu->set_input_line(4, serial_irq_causes() ? ASSERT_LINE : CLEAR_LINE);
}

void igs011_serial_state::serial_tx_w(u8 data)
{
	m_serial_tx = data;
	m_serial_status &= ~0x10;
	m_serial_timer->adjust(attotime::from_hz(9600 / 10));
	serial_update_irq();
}

TIMER_CALLBACK_MEMBER(igs011_serial_state::serial_tx_done)
{
	// loopback, see above
	m_serial_rx = m_serial_tx;
	m_serial_status |= 0x11;
	m_serial_rx_irq = true;
	serial_update_irq();
}

void igs011_serial_state::serial_ctrl_w(u8 data)
{
	m_serial_ctrl = data;
	serial_update_irq();
}

u8 igs011_serial_state::serial_rx_r()
{
	if (!machine().side_effects_disabled())
		m_serial_status &= ~0x01;

	return m_serial_rx;
}

u8 igs011_serial_state::serial_status_r()
{
	return m_serial_status;
}

u8 igs011_serial_state::serial_irq_causes() const
{
	// the TX cause follows the status, the RX cause is latched
	const bool tx = BIT(m_serial_ctrl, 0) && BIT(m_serial_status, 4);
	const bool rx = BIT(m_serial_ctrl, 1) && m_serial_rx_irq;

	return (rx ? 0x04 : 0x00) | (tx ? 0x08 : 0x00);
}

u8 igs011_serial_state::serial_irq_r()
{
	const u8 data = serial_irq_causes();

	if (!machine().side_effects_disabled())
	{
		m_serial_rx_irq = false;
		serial_update_irq();
	}

	return data;
}

void igs011_oki_state::lhb_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();

//  lhb: IGS011 protection dynamically mapped at 834x
//  map(0x008340, 0x008347).w(m_igs011, FUNC(igs011_device::prot_w)).umask16(0xff00);
//  map(0x008348, 0x008349).r(m_igs011, FUNC(igs011_device::prot_r));

	map(0x010000, 0x010000).w(FUNC(igs011_oki_state::lhb_okibank_w));

	map(0x010200, 0x0103ff).w(FUNC(igs011_oki_state::prot_inc_w));
	map(0x010400, 0x0105ff).w(FUNC(igs011_oki_state::lhb_prot_swap_w));
	map(0x010600, 0x0107ff).r(FUNC(igs011_oki_state::lhb_prot_r));
	// no reset

	map(0x100000, 0x103fff).ram().share("nvram");
	map(0x200000, 0x200fff).rw(m_igs011, FUNC(igs011_device::priority_ram_r), FUNC(igs011_device::priority_ram_w));
	map(0x300000, 0x3fffff).umask16(0x00ff).rw(m_igs011, FUNC(igs011_device::layers_r), FUNC(igs011_device::layers_w));
	map(0x400000, 0x400fff).umask16(0x00ff).rw(m_palette, FUNC(palette_device::read8), FUNC(palette_device::write8)).share("palette");
	map(0x401000, 0x401fff).umask16(0x00ff).rw(m_palette, FUNC(palette_device::read8_ext), FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0x600001, 0x600001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x700000, 0x700001).portr("COIN");
	map(0x700002, 0x700005).r(FUNC(igs011_oki_state::lhb_inputs_r));
	map(0x700002, 0x700003).w(FUNC(igs011_oki_state::lhb_inputs_w));
	map(0x820000, 0x820001).w(m_igs011, FUNC(igs011_device::priority_w));
	map(0x838000, 0x838001).w(FUNC(igs011_oki_state::lhb_irq_enable_w));
	map(0x840000, 0x840001).w(FUNC(igs011_oki_state::dips_w));

	map(0x850000, 0x850001).w(m_igs011, FUNC(igs011_device::prot_addr_w));

	map(0x858000, 0x85cfff).m(m_igs011, FUNC(igs011_device::map));
	map(0x888000, 0x888001).r(FUNC(igs011_oki_state::dips_r<5>));
}

void igs011_oki_state::xymg_base_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();

//  xymg: IGS011 protection dynamically mapped at 834x
//  map(0x008340, 0x008347).w(m_igs011, FUNC(igs011_device::prot_w)).umask16(0xff00);
//  map(0x008348, 0x008349).r(m_igs011, FUNC(igs011_device::prot_r));

	map(0x010000, 0x010000).w(FUNC(igs011_oki_state::lhb_okibank_w));

	map(0x010200, 0x0103ff).w(FUNC(igs011_oki_state::prot_inc_w));   // inc  (33)
	map(0x010400, 0x0105ff).w(FUNC(igs011_oki_state::lhb_prot_swap_w));   // swap (33)
	map(0x010600, 0x0107ff).r(FUNC(igs011_oki_state::lhb_prot_r));   // read
	// no reset

	map(0x100000, 0x103fff).ram();
	map(0x200000, 0x200fff).rw(m_igs011, FUNC(igs011_device::priority_ram_r), FUNC(igs011_device::priority_ram_w));
	map(0x300000, 0x3fffff).umask16(0x00ff).rw(m_igs011, FUNC(igs011_device::layers_r), FUNC(igs011_device::layers_w));
	map(0x400000, 0x400fff).umask16(0x00ff).rw(m_palette, FUNC(palette_device::read8), FUNC(palette_device::write8)).share("palette");
	map(0x401000, 0x401fff).umask16(0x00ff).rw(m_palette, FUNC(palette_device::read8_ext), FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0x600001, 0x600001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x820000, 0x820001).w(m_igs011, FUNC(igs011_device::priority_w));
	map(0x840000, 0x840001).w(FUNC(igs011_oki_state::dips_w));

	map(0x850000, 0x850001).w(m_igs011, FUNC(igs011_device::prot_addr_w));

	map(0x858000, 0x85cfff).m(m_igs011, FUNC(igs011_device::map));
	map(0x888000, 0x888001).r(FUNC(igs011_oki_state::dips_r<3>));
}

void igs011_oki_state::xymg_mem(address_map &map)
{
	xymg_base_mem(map);

	map(0x1f0000, 0x1f3fff).ram().share("nvram"); // extra RAM

	map(0x700000, 0x700001).w(FUNC(igs011_oki_state::igs003_index_w));
	map(0x700002, 0x700003).w(FUNC(igs011_oki_state::igs003_data_w));
	map(0x700003, 0x700003).r(m_igs003, FUNC(igs003_device::data_r));
}

void igs011_oki_state::xymga_mem(address_map &map)
{
	xymg_base_mem(map);

	// extra commands, only used by the checks that xymg doesn't have (written into the ROM space, reads are unaffected)
	map(0x010a00, 0x010bff).w(FUNC(igs011_oki_state::xymga_prot_step2_w));
	map(0x010c00, 0x010dff).w(FUNC(igs011_oki_state::xymga_prot_step1_w));
	map(0x011600, 0x0117ff).w(FUNC(igs011_oki_state::prot_reset_w));

	map(0x1f0000, 0x1fffff).ram().share("nvram"); // extra RAM, 2 x HM62H256

	map(0x700000, 0x700001).portr("COIN");
	map(0x700002, 0x700005).r(FUNC(igs011_oki_state::lhb_inputs_r));
	map(0x700002, 0x700003).w(FUNC(igs011_oki_state::lhb_inputs_w));
}

void igs011_oki_state::sdmg2641c_mem(address_map &map)
{
	xymg_base_mem(map);

	map(0x1f0000, 0x1f3fff).ram().share("nvram"); // extra RAM

	map(0x700000, 0x700001).w(FUNC(igs011_oki_state::igs003_index_w));
	map(0x700002, 0x700003).w(FUNC(igs011_oki_state::igs003_data_w));
	map(0x700003, 0x700003).r(m_igs003, FUNC(igs003_device::data_r));
	map(0x838000, 0x838001).w(FUNC(igs011_oki_state::lhb_irq_enable_w));
}

void igs011_oki_state::wlcc_mem(address_map &map)
{
//  wlcc: IGS011 protection dynamically mapped at 834x
//  map(0x008340, 0x008347).w(m_igs011, FUNC(igs011_device::prot_w)).umask16(0xff00);
//  map(0x008348, 0x008349).r(m_igs011, FUNC(igs011_device::prot_r));

	map(0x518000, 0x5181ff).w(FUNC(igs011_oki_state::prot_inc_w));   // inc   (33)
	map(0x518200, 0x5183ff).w(FUNC(igs011_oki_state::wlcc_prot_swap_w));   // swap  (33)
	map(0x518800, 0x5189ff).r(FUNC(igs011_oki_state::prot_reset_r));   // reset
	map(0x519000, 0x5191ff).r(FUNC(igs011_oki_state::wlcc_prot_r<0>));   // read
	map(0x519200, 0x5193ff).r(FUNC(igs011_oki_state::wlcc_prot_r<1>));   // read
	map(0x519400, 0x5195ff).r(FUNC(igs011_oki_state::wlcc_prot_r<2>));   // read

	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x103fff).ram().share("nvram");
	map(0x200000, 0x200fff).rw(m_igs011, FUNC(igs011_device::priority_ram_r), FUNC(igs011_device::priority_ram_w));
	map(0x300000, 0x3fffff).umask16(0x00ff).rw(m_igs011, FUNC(igs011_device::layers_r), FUNC(igs011_device::layers_w));
	map(0x400000, 0x400fff).umask16(0x00ff).rw(m_palette, FUNC(palette_device::read8), FUNC(palette_device::write8)).share("palette");
	map(0x401000, 0x401fff).umask16(0x00ff).rw(m_palette, FUNC(palette_device::read8_ext), FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0x520000, 0x520001).portr("COIN");
	map(0x600001, 0x600001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x800000, 0x800001).w(FUNC(igs011_oki_state::igs003_index_w));
	map(0x800002, 0x800003).w(FUNC(igs011_oki_state::igs003_data_w));
	map(0x800003, 0x800003).r(m_igs003, FUNC(igs003_device::data_r));
	map(0xa20000, 0xa20001).w(m_igs011, FUNC(igs011_device::priority_w));
	map(0xa40000, 0xa40001).w(FUNC(igs011_oki_state::dips_w));

	map(0xa50000, 0xa50001).w(m_igs011, FUNC(igs011_device::prot_addr_w));

	map(0xa58000, 0xa5cfff).m(m_igs011, FUNC(igs011_device::map));
	map(0xa88000, 0xa88001).r(FUNC(igs011_oki_state::dips_r<4>));
}


void igs011_oki_state::lhb2_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();

//  lhb2: IGS011 protection dynamically mapped at 1ff8x
//  map(0x01ff80, 0x01ff87).umask16(0xff00).w(m_igs011, FUNC(igs011_device::prot_w));
//  map(0x01ff88, 0x01ff89).r(m_igs011, FUNC(igs011_device::prot_r));

	map(0x020000, 0x0201ff).w(FUNC(igs011_oki_state::prot_inc_w));   // inc   (55)
	map(0x020200, 0x0203ff).w(FUNC(igs011_oki_state::lhb_prot_swap_w));   // swap  (33)
	map(0x020400, 0x0205ff).r(FUNC(igs011_oki_state::lhb2_prot_r));   // read
	map(0x020600, 0x0207ff).w(FUNC(igs011_oki_state::prot_reset_w));   // reset (55)

	map(0x100000, 0x103fff).ram().share("nvram");
	map(0x200001, 0x200001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x204000, 0x204003).umask16(0x00ff).w("ymsnd", FUNC(ym2413_device::write));
	map(0x208000, 0x208001).nopr().w(FUNC(igs011_oki_state::igs003_index_w));
	map(0x208002, 0x208003).w(FUNC(igs011_oki_state::igs003_data_w));
	map(0x208003, 0x208003).r(m_igs003, FUNC(igs003_device::data_r));
	map(0x20c000, 0x20cfff).rw(m_igs011, FUNC(igs011_device::priority_ram_r), FUNC(igs011_device::priority_ram_w));
	map(0x210000, 0x210fff).umask16(0x00ff).rw(m_palette, FUNC(palette_device::read8), FUNC(palette_device::write8)).share("palette");
	map(0x211000, 0x211fff).umask16(0x00ff).rw(m_palette, FUNC(palette_device::read8_ext), FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0x214000, 0x214001).portr("COIN");
	map(0x300000, 0x3fffff).umask16(0x00ff).rw(m_igs011, FUNC(igs011_device::layers_r), FUNC(igs011_device::layers_w));
	map(0xa20000, 0xa20001).w(m_igs011, FUNC(igs011_device::priority_w));
	map(0xa40000, 0xa40001).w(FUNC(igs011_oki_state::dips_w));

	map(0xa50000, 0xa50001).w(m_igs011, FUNC(igs011_device::prot_addr_w));

	map(0xa58000, 0xa5cfff).m(m_igs011, FUNC(igs011_device::map));
	map(0xa88000, 0xa88001).r(FUNC(igs011_oki_state::dips_r<3>));
}


void igs011_oki_state::lhb3_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();

	map(0x021000, 0x0211ff).w(FUNC(igs011_oki_state::prot_inc_w));   // inc   (55)
	map(0x021200, 0x0213ff).w(FUNC(igs011_oki_state::lhb_prot_swap_w));   // swap  (33)
	map(0x021400, 0x0215ff).r(FUNC(igs011_oki_state::lhb3_prot_r));   // read
	map(0x021600, 0x0217ff).w(FUNC(igs011_oki_state::prot_reset_w));   // reset (55)

	map(0x100000, 0x103fff).ram().share("nvram");
	map(0x200001, 0x200001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x204000, 0x204003).w("ymsnd", FUNC(ym2413_device::write)).umask16(0x00ff);
	map(0x208000, 0x208001).nopr().w(FUNC(igs011_oki_state::igs003_index_w));
	map(0x208002, 0x208003).w(FUNC(igs011_oki_state::igs003_data_w));
	map(0x208003, 0x208003).r(m_igs003, FUNC(igs003_device::data_r));
	map(0x20c000, 0x20cfff).rw(m_igs011, FUNC(igs011_device::priority_ram_r), FUNC(igs011_device::priority_ram_w));
	map(0x210000, 0x210fff).umask16(0x00ff).rw(m_palette, FUNC(palette_device::read8), FUNC(palette_device::write8)).share("palette");
	map(0x211000, 0x211fff).umask16(0x00ff).rw(m_palette, FUNC(palette_device::read8_ext), FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0x214000, 0x214001).portr("COIN");
	map(0x300000, 0x3fffff).umask16(0x00ff).rw(m_igs011, FUNC(igs011_device::layers_r), FUNC(igs011_device::layers_w));
	map(0xa20000, 0xa20001).w(m_igs011, FUNC(igs011_device::priority_w));
	map(0xa38000, 0xa38001).w(FUNC(igs011_oki_state::lhb_irq_enable_w));
	map(0xa40000, 0xa40001).w(FUNC(igs011_oki_state::dips_w));

	map(0xa50000, 0xa50001).w(m_igs011, FUNC(igs011_device::prot_addr_w));

	map(0xa58000, 0xa5cfff).m(m_igs011, FUNC(igs011_device::map));
	map(0xa88000, 0xa88001).r(FUNC(igs011_oki_state::dips_r<3>));
}


void igs011_oki_state::nkishusp_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();

//  nkishusp: IGS011 protection dynamically mapped at 1ff8x
//  map(0x01ff80, 0x01ff87).w(m_igs011, FUNC(igs011_device::prot_w)).umask16(0xff00);
//  map(0x01ff88, 0x01ff89).r(m_igs011, FUNC(igs011_device::prot_r));

	map(0x023000, 0x0231ff).w(FUNC(igs011_oki_state::prot_inc_w));   // inc   (55)
	map(0x023200, 0x0233ff).w(FUNC(igs011_oki_state::nkishusp_prot_swap_w));   // swap  (33)
	map(0x023400, 0x0235ff).r(FUNC(igs011_oki_state::nkishusp_prot_r));   // read
	map(0x023600, 0x0237ff).w(FUNC(igs011_oki_state::prot_reset_w));   // reset (55)

	map(0x100000, 0x103fff).ram().share("nvram");
	map(0x200001, 0x200001).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x204000, 0x204003).w("ymsnd", FUNC(ym2413_device::write)).umask16(0x00ff);
	map(0x208000, 0x208001).nopr().w(FUNC(igs011_oki_state::igs003_index_w));
	map(0x208002, 0x208003).w(FUNC(igs011_oki_state::igs003_data_w));
	map(0x208003, 0x208003).r(m_igs003, FUNC(igs003_device::data_r));
	map(0x20c000, 0x20cfff).rw(m_igs011, FUNC(igs011_device::priority_ram_r), FUNC(igs011_device::priority_ram_w));
	map(0x210000, 0x210fff).umask16(0x00ff).rw(m_palette, FUNC(palette_device::read8), FUNC(palette_device::write8)).share("palette");
	map(0x211000, 0x211fff).umask16(0x00ff).rw(m_palette, FUNC(palette_device::read8_ext), FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0x214000, 0x214001).portr("COIN");
	map(0x300000, 0x3fffff).umask16(0x00ff).rw(m_igs011, FUNC(igs011_device::layers_r), FUNC(igs011_device::layers_w));
	map(0xa20000, 0xa20001).w(m_igs011, FUNC(igs011_device::priority_w));
	map(0xa38000, 0xa38001).w(FUNC(igs011_oki_state::lhb_irq_enable_w));
	map(0xa40000, 0xa40001).w(FUNC(igs011_oki_state::dips_w));

	map(0xa50000, 0xa50001).w(m_igs011, FUNC(igs011_device::prot_addr_w));

	map(0xa58000, 0xa5cfff).m(m_igs011, FUNC(igs011_device::map));
	map(0xa88000, 0xa88001).r(FUNC(igs011_oki_state::dips_r<3>));
}

void igs011_serial_state::serial_map(address_map &map)
{
	map(0xa00001, 0xa00001).w(FUNC(igs011_serial_state::serial_tx_w));
	map(0xa08001, 0xa08001).w(FUNC(igs011_serial_state::serial_ctrl_w));
	map(0xa10000, 0xa10001).nopw(); // serial port mode? only 0x81 is written
	map(0xa80001, 0xa80001).r(FUNC(igs011_serial_state::serial_rx_r));
	map(0xa90001, 0xa90001).r(FUNC(igs011_serial_state::serial_status_r));
	map(0xa98001, 0xa98001).r(FUNC(igs011_serial_state::serial_irq_r));
}

void igs011_serial_state::lhb3_serial_mem(address_map &map)
{
	lhb3_mem(map);
	serial_map(map);
}

void igs011_serial_state::nkishusp_serial_mem(address_map &map)
{
	nkishusp_mem(map);
	serial_map(map);
}


u16 vbowl_state::vbowl_unk_r()
{
	return 0xffff;
}

void vbowl_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		m_vbowl_trackball[0] = m_vbowl_trackball[1];

		// trackball is rotated 45 degrees
		u8 const x = m_io_an[0]->read();
		u8 const y = m_io_an[1]->read();
		if (BIT(m_io_conf->read(), 0))
		{
			u8 const dx = x - m_trackball_count[0];
			u8 const dy = y - m_trackball_count[1];
			u8 const ax = (m_vbowl_trackball[1] & 0xff) + dx + dy;
			u8 const ay = (m_vbowl_trackball[1] >> 8) + dy - dx;
			m_vbowl_trackball[1] = (u16(ay) << 8) | u16(ax);
		}
		else
		{
			m_vbowl_trackball[1] = (u16(y) << 8) | u16(x);
		}

		m_trackball_count[0] = x;
		m_trackball_count[1] = y;
	}
}

void vbowl_state::vbowl_pen_hi_w(u8 data)
{
	m_igs011->blitter_pen_hi_w(data & 0x07);

	if (data & ~0x7)
		logerror("%s: warning, unknown bits written to pen_hi = %02x\n", machine().describe_context(), data);
}

void vbowl_state::vbowl_link_0_w(u16 data) { }
void vbowl_state::vbowl_link_1_w(u16 data) { }
void vbowl_state::vbowl_link_2_w(u16 data) { }
void vbowl_state::vbowl_link_3_w(u16 data) { }

void vbowl_state::vbowl_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();

//  vbowl: IGS011 protection dynamically mapped at 834x
//  map(0x008340, 0x008347).w(m_igs011, FUNC(vbowl_state::prot_w)).umask16(0xff00);
//  map(0x008348, 0x008349).r(m_igs011, FUNC(vbowl_state::prot_r));

	// IGS012
	map(0x001600, 0x00167f).m(m_igs012, FUNC(igs012_device::map)).mirror(0x01c000);

	map(0x00d400, 0x00d43f).w(FUNC(vbowl_state::prot_dec_w));   // dec   (33)
	map(0x00d440, 0x00d47f).w(FUNC(vbowl_state::drgnwrld_prot_swap_w));   // swap  (33)
	map(0x00d480, 0x00d4bf).w(FUNC(vbowl_state::prot_reset_w));   // reset (33)
	map(0x00d4c0, 0x00d4ff).r(FUNC(vbowl_state::drgnwrldv20j_prot_r));   // read

	map(0x50f000, 0x50f1ff).w(FUNC(vbowl_state::prot_dec_w));   // dec   (33)
	map(0x50f200, 0x50f3ff).w(FUNC(vbowl_state::vbowl_prot_swap_w));   // swap  (33)
	map(0x50f400, 0x50f5ff).w(FUNC(vbowl_state::prot_reset_w));   // reset (33)
	map(0x50f600, 0x50f7ff).r(FUNC(vbowl_state::vbowl_prot_r));   // read

	map(0x902000, 0x902fff).w(FUNC(vbowl_state::igs012_regs_w));

	map(0x100000, 0x103fff).ram().share("nvram");
	map(0x200000, 0x200fff).rw(m_igs011, FUNC(igs011_device::priority_ram_r), FUNC(igs011_device::priority_ram_w));
	map(0x300000, 0x3fffff).umask16(0x00ff).rw(m_igs011, FUNC(igs011_device::layers_r), FUNC(igs011_device::layers_w));
	map(0x400000, 0x400fff).umask16(0x00ff).rw(m_palette, FUNC(palette_device::read8), FUNC(palette_device::write8)).share("palette");
	map(0x401000, 0x401fff).umask16(0x00ff).rw(m_palette, FUNC(palette_device::read8_ext), FUNC(palette_device::write8_ext)).share("palette_ext");
	map(0x520000, 0x520001).portr("COIN");
	map(0x600000, 0x600007).rw(m_ics, FUNC(ics2115_device::word_r), FUNC(ics2115_device::word_w));
	map(0x700000, 0x700003).ram().share(m_vbowl_trackball);
	map(0x700005, 0x700005).w(FUNC(vbowl_state::vbowl_pen_hi_w));
	map(0x800000, 0x800001).w(FUNC(vbowl_state::igs003_index_w));
	map(0x800002, 0x800003).w(FUNC(vbowl_state::igs003_data_w));
	map(0x800003, 0x800003).r(m_igs003, FUNC(igs003_device::data_r));

	map(0xa00000, 0xa00001).w(FUNC(vbowl_state::vbowl_link_0_w));
	map(0xa08000, 0xa08001).w(FUNC(vbowl_state::vbowl_link_1_w));
	map(0xa10000, 0xa10001).w(FUNC(vbowl_state::vbowl_link_2_w));
	map(0xa18000, 0xa18001).w(FUNC(vbowl_state::vbowl_link_3_w));

	map(0xa20000, 0xa20001).w(m_igs011, FUNC(igs011_device::priority_w));
//  map(0xa38000, 0xa38001).w(FUNC(vbowl_state::lhb_irq_enable_w));
	map(0xa40000, 0xa40001).w(FUNC(vbowl_state::dips_w));

	map(0xa48000, 0xa48001).w(m_igs011, FUNC(igs011_device::prot_addr_w));

	map(0xa58000, 0xa5cfff).m(m_igs011, FUNC(igs011_device::map));

	map(0xa80000, 0xa80001).r(FUNC(vbowl_state::vbowl_unk_r)); // comm
	map(0xa88000, 0xa88001).r(FUNC(vbowl_state::dips_r<4>));
	map(0xa90000, 0xa90001).r(FUNC(vbowl_state::vbowl_unk_r)); // comm
	map(0xa98000, 0xa98001).r(FUNC(vbowl_state::vbowl_unk_r)); // comm
}


void vbowl_state::vbowlhk_mem(address_map &map)
{
	vbowl_mem(map);

	map(0x50f600, 0x50f7ff).r(FUNC(vbowl_state::vbowlhk_prot_r));   // read
}


/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( drgnwrld )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( Easy    ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal  ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard    ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNKNOWN( 0x20, 0x20 )                       PORT_DIPLOCATION("SW1:6")  // DIPS 6, 7 & 8 are unlisted in DIPSW sheet
	PORT_DIPUNKNOWN( 0x40, 0x40 )                       PORT_DIPLOCATION("SW1:7")
	PORT_DIPUNKNOWN( 0x80, 0x80 )                       PORT_DIPLOCATION("SW1:8")

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Strip Girl" )            PORT_DIPLOCATION("SW2:1")  // "Open Girl" in test mode
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Background" )            PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, "Girl" )
	PORT_DIPSETTING(    0x00, "Scene" )     // broken backgrounds with "Strip Girl" on (PCB does the same)
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Fever Game" )            PORT_DIPLOCATION("SW2:4")  // Bonus game after round such as hitting turtles etc
	PORT_DIPSETTING(    0x08, "One Kind Only" )
	PORT_DIPSETTING(    0x00, "Several Kinds" )
	PORT_DIPUNKNOWN( 0x10, 0x10 )                       PORT_DIPLOCATION("SW2:5")  // DIPS 5, 6, 7 & 8 are unlisted in DIPSW sheet
	PORT_DIPUNKNOWN( 0x20, 0x20 )                       PORT_DIPLOCATION("SW2:6")
	PORT_DIPUNKNOWN( 0x40, 0x40 )                       PORT_DIPLOCATION("SW2:7")
	PORT_DIPUNKNOWN( 0x80, 0x80 )                       PORT_DIPLOCATION("SW2:8")

	PORT_START("DSW3")
	// On some PCBs all of DSW3 is removed
	PORT_DIPUNKNOWN( 0x01, 0x01 )                       PORT_DIPLOCATION("SW3:1")
	PORT_DIPUNKNOWN( 0x02, 0x02 )                       PORT_DIPLOCATION("SW3:2")
	PORT_DIPUNKNOWN( 0x04, 0x04 )                       PORT_DIPLOCATION("SW3:3")
	PORT_DIPUNKNOWN( 0x08, 0x08 )                       PORT_DIPLOCATION("SW3:4")
	PORT_DIPUNKNOWN( 0x10, 0x10 )                       PORT_DIPLOCATION("SW3:5")
	PORT_DIPUNKNOWN( 0x20, 0x20 )                       PORT_DIPLOCATION("SW3:6")
	PORT_DIPUNKNOWN( 0x40, 0x40 )                       PORT_DIPLOCATION("SW3:7")
	PORT_DIPUNKNOWN( 0x80, 0x80 )                       PORT_DIPLOCATION("SW3:8")

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )   // keep pressed while booting
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )   // used?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE3 )   // used?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )    // press in girl test to pause, button 3 advances
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( drgnwrldc )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( Easy    ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal  ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard    ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNKNOWN( 0x20, 0x20 )                       PORT_DIPLOCATION("SW1:6")
	PORT_DIPUNKNOWN( 0x40, 0x40 )                       PORT_DIPLOCATION("SW1:7")
	PORT_DIPUNKNOWN( 0x80, 0x80 )                       PORT_DIPLOCATION("SW1:8")

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Strip Girl" )            PORT_DIPLOCATION("SW2:1")  // "Open Girl" in test mode
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "Sex Question" )          PORT_DIPLOCATION("SW2:2")  // "background" in test mode
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "Background" )            PORT_DIPLOCATION("SW2:3")  // "sex question" in test mode
	PORT_DIPSETTING(    0x04, "Girl" )
	PORT_DIPSETTING(    0x00, "Scene" )     // broken backgrounds with "Strip Girl" on (PCB does the same)
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Tiles" )                 PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "Mahjong" )
	PORT_DIPSETTING(    0x00, "Symbols" )
	PORT_DIPUNKNOWN( 0x20, 0x20 )                       PORT_DIPLOCATION("SW2:6")
	PORT_DIPNAME( 0x40, 0x40, "Fever Game" )            PORT_DIPLOCATION("SW2:7")  // Bonus game after round such as hitting turtles etc
	PORT_DIPSETTING(    0x40, "One Kind Only" )
	PORT_DIPSETTING(    0x00, "Several Kinds" )
	PORT_DIPUNKNOWN( 0x80, 0x80 )                       PORT_DIPLOCATION("SW2:8")

	PORT_START("DSW3")
	PORT_DIPUNKNOWN( 0x01, 0x01 )                       PORT_DIPLOCATION("SW3:1")
	PORT_DIPUNKNOWN( 0x02, 0x02 )                       PORT_DIPLOCATION("SW3:2")
	PORT_DIPUNKNOWN( 0x04, 0x04 )                       PORT_DIPLOCATION("SW3:3")
	PORT_DIPUNKNOWN( 0x08, 0x08 )                       PORT_DIPLOCATION("SW3:4")
	PORT_DIPUNKNOWN( 0x10, 0x10 )                       PORT_DIPLOCATION("SW3:5")
	PORT_DIPUNKNOWN( 0x20, 0x20 )                       PORT_DIPLOCATION("SW3:6")
	PORT_DIPUNKNOWN( 0x40, 0x40 )                       PORT_DIPLOCATION("SW3:7")
	PORT_DIPUNKNOWN( 0x80, 0x80 )                       PORT_DIPLOCATION("SW3:8")

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )   // keep pressed while booting
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )   // used?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE3 )   // used?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )    // press in girl test to pause, button 3 advances
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( drgnwrldj )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x18, DEF_STR( Easy    ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Normal  ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Hard    ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPUNKNOWN( 0x20, 0x20 )                       PORT_DIPLOCATION("SW1:6")
	PORT_DIPUNKNOWN( 0x40, 0x40 )                       PORT_DIPLOCATION("SW1:7")
	PORT_DIPUNKNOWN( 0x80, 0x80 )                       PORT_DIPLOCATION("SW1:8")

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "Background" )            PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, "Girl" )
	PORT_DIPSETTING(    0x00, "Scene" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Tiles" )                 PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "Mahjong" )
	PORT_DIPSETTING(    0x00, "Symbols" )
	PORT_DIPUNKNOWN( 0x08, 0x08 )                       PORT_DIPLOCATION("SW2:4")
	PORT_DIPUNKNOWN( 0x10, 0x10 )                       PORT_DIPLOCATION("SW2:5")
	PORT_DIPUNKNOWN( 0x20, 0x20 )                       PORT_DIPLOCATION("SW2:6")
	PORT_DIPUNKNOWN( 0x40, 0x40 )                       PORT_DIPLOCATION("SW2:7")
	PORT_DIPUNKNOWN( 0x80, 0x80 )                       PORT_DIPLOCATION("SW2:8")

	PORT_START("DSW3")
	PORT_DIPUNKNOWN( 0x01, 0x01 )                       PORT_DIPLOCATION("SW3:1")
	PORT_DIPUNKNOWN( 0x02, 0x02 )                       PORT_DIPLOCATION("SW3:2")
	PORT_DIPUNKNOWN( 0x04, 0x04 )                       PORT_DIPLOCATION("SW3:3")
	PORT_DIPUNKNOWN( 0x08, 0x08 )                       PORT_DIPLOCATION("SW3:4")
	PORT_DIPUNKNOWN( 0x10, 0x10 )                       PORT_DIPLOCATION("SW3:5")
	PORT_DIPUNKNOWN( 0x20, 0x20 )                       PORT_DIPLOCATION("SW3:6")
	PORT_DIPUNKNOWN( 0x40, 0x40 )                       PORT_DIPLOCATION("SW3:7")
	PORT_DIPUNKNOWN( 0x80, 0x80 )                       PORT_DIPLOCATION("SW3:8")

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )   // keep pressed while booting
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )   // used?
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE3 )   // used?
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )    // press in girl test to pause, button 3 advances
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
INPUT_PORTS_END


static INPUT_PORTS_START( lhb2 )
	// defaults based on manual for lhb2cpgs

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x04, "Payout Rate" )           PORT_DIPLOCATION("SW1:1,2,3") // 機率調整
	PORT_DIPSETTING(    0x07, "50%" )
	PORT_DIPSETTING(    0x06, "54%" )
	PORT_DIPSETTING(    0x05, "58%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "66%" )
	PORT_DIPSETTING(    0x02, "70%" )
	PORT_DIPSETTING(    0x01, "74%" )
	PORT_DIPSETTING(    0x00, "78%" )
	PORT_DIPNAME( 0x08, 0x00, "Odds Rate" )             PORT_DIPLOCATION("SW1:4")     // 倍數表
	PORT_DIPSETTING(    0x00, "1,2,3,4,5,6,7,8" )
	PORT_DIPSETTING(    0x08, "1,2,3,5,8,15,30,50" )
	PORT_DIPNAME( 0x10, 0x00, "Maximum Bet" )           PORT_DIPLOCATION("SW1:5")     // 最大押注
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPNAME( 0x60, 0x60, "Minimum Bet" )           PORT_DIPLOCATION("SW1:6,7")   // 最小押注
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x80, 0x80, "Credit Timer" )          PORT_DIPLOCATION("SW1:8")     // 自動清除  (clears credits after timeout if you don't start a game)
	PORT_DIPSETTING(    0x80, DEF_STR(Off) )                                          // 無
	PORT_DIPSETTING(    0x00, DEF_STR(On) )                                           // 有

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR(Coinage) )        PORT_DIPLOCATION("SW2:1,2")   // 投幣比率
	PORT_DIPSETTING(    0x00, DEF_STR(2C_1C) )
	PORT_DIPSETTING(    0x03, DEF_STR(1C_1C) )
	PORT_DIPSETTING(    0x02, DEF_STR(1C_2C) )
	PORT_DIPSETTING(    0x01, DEF_STR(1C_3C) )
	PORT_DIPNAME( 0x04, 0x04, "Key-in Rate" )           PORT_DIPLOCATION("SW2:3")     // 開分比率
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x08, 0x08, "Credit Limit" )          PORT_DIPLOCATION("SW2:4")     // 進分上限
	PORT_DIPSETTING(    0x08, "100" )
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPNAME( 0x10, 0x10, "Credit Mode" )           PORT_DIPLOCATION("SW2:5")     // 進分方式
	PORT_DIPSETTING(    0x10, "Coin Acceptor" )                                       // 投幣
	PORT_DIPSETTING(    0x00, "Key-In" )                                              // 開分
	PORT_DIPNAME( 0x20, 0x20, "Payout Mode" )           PORT_DIPLOCATION("SW2:6")     // 退分方式
	PORT_DIPSETTING(    0x20, "Key-Out" )                                             // 洗分
	PORT_DIPSETTING(    0x00, "Return Coins" )                                        // 退幣      (doesn't seem to work properly)
	PORT_DIPNAME( 0x40, 0x40, "Auto Reach" )            PORT_DIPLOCATION("SW2:7")     // 自動摸打  (automatically draws and discards tiles after reach)
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )                                          // 無
	PORT_DIPSETTING(    0x40, DEF_STR(On) )                                           // 有
	PORT_DIPNAME( 0x80, 0x80, DEF_STR(Demo_Sounds) )    PORT_DIPLOCATION("SW2:8")     // 示範音樂
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )                                          // 無
	PORT_DIPSETTING(    0x80, DEF_STR(On) )                                           // 有

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Jackpot Limit" )         PORT_DIPLOCATION("SW3:1,2")   // 破台限制
	PORT_DIPSETTING(    0x03, "500" )
	PORT_DIPSETTING(    0x02, "1000" )
	PORT_DIPSETTING(    0x01, "2000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )                                           // 無限制
	PORT_DIPNAME( 0x0c, 0x0c, "Gals" )                  PORT_DIPLOCATION("SW3:3,4")   // 美女
	PORT_DIPSETTING(    0x0c, DEF_STR(Off) )                                          // 無美女
	PORT_DIPSETTING(    0x08, "Clothed" )                                             // 保守
	PORT_DIPSETTING(    0x04, "Nudity" )                                              // 開放
	PORT_DIPSETTING(    0x00, "Nudity" )                                              // 開放
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW3:5" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW3:6" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW3:7" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW3:8" )                                     // (not shown in settings display)

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_CONDITION("DSW2", 0x10, EQUALS, 0x10)                  // 投幣
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_CONDITION("DSW2", 0x10, EQUALS, 0x00)                  // 投幣
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MEMORY_RESET )                                                               // 清除
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )                                                                   // 測試      (hold on start for input test)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r)) // 哈巴
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                                                                // 查帳
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  PORT_CONDITION("DSW2", 0x20, EQUALS, 0x20)                  // 洗分
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  PORT_CONDITION("DSW2", 0x20, EQUALS, 0x00)                  // 洗分
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_INCLUDE(mahjong_matrix_1p)

	PORT_MODIFY("KEY0")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // ? set to 0 both
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // ? and you can't start a game

	PORT_MODIFY("KEY1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) // doesn't use any other gambling keys
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("KEY2")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("KEY3")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("KEY4")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

INPUT_PORTS_START( lhb2cpgs )
	// The manual provides two sets of recommended settings for DIP SW.1:
	//       文台建議值                            武台建議值
	// SW 1   ON  ON OFF  ON  ON OFF OFF OFF        ON OFF  ON OFF OFF OFF OFF OFF

	//       建議調法
	// SW 2  OFF OFF OFF OFF OFF OFF OFF OFF
	// SW 3  OFF OFF OFF OFF OFF OFF OFF OFF

	PORT_INCLUDE( lhb2 )

	PORT_MODIFY("DSW3")
	// listed in manual but not shown in test mode - what does this actually do?
	PORT_DIPNAME( 0x70, 0x70, "Lottery Ticket Ratio" )  PORT_DIPLOCATION("SW3:5,6,7") // 彩票比例
	PORT_DIPSETTING(    0x70, "1:1" )
	PORT_DIPSETTING(    0x60, "1:2" )
	PORT_DIPSETTING(    0x50, "1:5" )
	PORT_DIPSETTING(    0x40, "1:6" )
	PORT_DIPSETTING(    0x30, "1:7" )
	PORT_DIPSETTING(    0x20, "1:8" )
	PORT_DIPSETTING(    0x10, "1:9" )
	PORT_DIPSETTING(    0x00, "1:10" )
INPUT_PORTS_END


static INPUT_PORTS_START( nkishusp )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x02, "Payout Rate" )           PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x07, "74%" )
	PORT_DIPSETTING(    0x06, "77%" )
	PORT_DIPSETTING(    0x05, "80%" )
	PORT_DIPSETTING(    0x04, "83%" )
	PORT_DIPSETTING(    0x03, "86%" )
	PORT_DIPSETTING(    0x02, "89%" )
	PORT_DIPSETTING(    0x01, "92%" )
	PORT_DIPSETTING(    0x00, "95%" )
	PORT_DIPNAME( 0x08, 0x00, "Odds Rate" )             PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, "1,2,3,4,5,6,7,8" )
	PORT_DIPSETTING(    0x08, "1,2,3,5,8,15,30,50" )
	PORT_DIPNAME( 0x10, 0x00, "Maximum Bet" )           PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x10, "10" )
	PORT_DIPNAME( 0x60, 0x60, "Minimum Bet" )           PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x60, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x80, 0x80, "Credit Timer" )          PORT_DIPLOCATION("SW1:8")     // (clears credits after timeout if you don't start a game)
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x04, 0x04, "Key-in Rate" )           PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x08, 0x08, "Credit Mode" )           PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "Coin Acceptor" )
	PORT_DIPSETTING(    0x00, "Key-In" )
	PORT_DIPNAME( 0x10, 0x10, "Auto Play" )             PORT_DIPLOCATION("SW2:5")     // (automatically draws and discards tiles after reach)
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Nudity" )                PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW2:8" )                                     // (not shown in settings display)

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Credit Limit" )          PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(    0x03, "500" )
	PORT_DIPSETTING(    0x02, "1000" )
	PORT_DIPSETTING(    0x01, "2000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )                                           // (seems to be limited to 9,999 trying to exceed this gives "RECORD ERROR 10")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW3:3" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW3:4" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW3:5" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW3:6" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW3:7" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW3:8" )                                     // (not shown in settings display)

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_CONDITION("DSW2", 0x08, EQUALS, 0x08)                  // 投幣
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_CONDITION("DSW2", 0x08, EQUALS, 0x00)                  // 投幣
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MEMORY_RESET )                                                               // 清除
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )                                                                   // 測試      (hold on start for input test)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r)) // 哈巴
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                                                                // 查帳
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )                                                              // 洗分
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN  )

	PORT_INCLUDE(mahjong_matrix_1p)


	PORT_MODIFY("KEY0")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // ? set to 0 both
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) // ? and you can't start a game

	PORT_MODIFY("KEY1")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) // doesn't use any other gambling keys
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("KEY2")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("KEY3")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_MODIFY("KEY4")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


// basically same game as lhb2 and nkishusp but with joystick controls
static INPUT_PORTS_START( tygn )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x02, "Payout Rate" )           PORT_DIPLOCATION("SW1:1,2,3")  // 機率調整
	PORT_DIPSETTING(    0x07, "50%" )
	PORT_DIPSETTING(    0x06, "54%" )
	PORT_DIPSETTING(    0x05, "58%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x03, "66%" )
	PORT_DIPSETTING(    0x02, "70%" )
	PORT_DIPSETTING(    0x01, "74%" )
	PORT_DIPSETTING(    0x00, "78%" )
	PORT_DIPNAME( 0x08, 0x08, "Minimum Bet" )           PORT_DIPLOCATION("SW1:4")      // 最小押注
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x00, "2" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR(Coinage) )        PORT_DIPLOCATION("SW1:5,6")    // 投幣比率
	PORT_DIPSETTING(    0x00, DEF_STR(2C_1C) )
	PORT_DIPSETTING(    0x30, DEF_STR(1C_1C) )
	PORT_DIPSETTING(    0x20, DEF_STR(1C_2C) )
	PORT_DIPSETTING(    0x10, DEF_STR(1C_3C) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR(Demo_Sounds) )    PORT_DIPLOCATION("SW1:7")      // 示範音樂
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )                                           // 無
	PORT_DIPSETTING(    0x40, DEF_STR(On) )                                            // 有
	PORT_DIPNAME( 0x80, 0x80, "Credit Timer" )          PORT_DIPLOCATION("SW1:8")      // 自動清除  (clears credits after timeout if you don't start a game)
	PORT_DIPSETTING(    0x80, DEF_STR(Off) )                                           // 無
	PORT_DIPSETTING(    0x00, DEF_STR(On) )                                            // 有

	PORT_START("DSW2") // not shown in test mode
	PORT_DIPUNKNOWN( 0x01, 0x01 ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPUNKNOWN( 0x02, 0x02 ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPUNKNOWN( 0x04, 0x04 ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPUNKNOWN( 0x08, 0x08 ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPUNKNOWN( 0x10, 0x10 ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPUNKNOWN( 0x20, 0x20 ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPUNKNOWN( 0x40, 0x40 ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPUNKNOWN( 0x80, 0x80 ) PORT_DIPLOCATION("SW2:8")

	PORT_START("DSW3") // only 2 out of 4 DIP switch banks physically present
	PORT_DIPNAME( 0xff, 0xff, DEF_STR( Unused ) )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )                                                                      // 投幣
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )                                                                   // 測試   (hold on start for test mode)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                                                                // 查帳   (stats)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r)) // 哈巴

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )                                                                     // 開始
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )                                                                // 上
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )                                                              // 下
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )                                                              // 捨     (name in input test looks wrong - caused by patches?)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )                                                             // 注     (name in input test looks wrong - caused by patches?)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )                                                                    // 摸/動  (selects input test in test mode)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )                                                                    // 押注   (selects graphics test in test mode)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 )                                                                    // 遊戲   (selects audio test in test mode)

	PORT_START("IN1") // probably unused
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2") // probably unused
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( xymg )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR(Coinage) )        PORT_DIPLOCATION("SW1:1,2")   // 投幣比率
	PORT_DIPSETTING(    0x03, DEF_STR(1C_1C) )
	PORT_DIPSETTING(    0x02, DEF_STR(1C_2C) )
	PORT_DIPSETTING(    0x01, DEF_STR(1C_3C) )
	PORT_DIPSETTING(    0x00, DEF_STR(1C_4C) )
	PORT_DIPNAME( 0x0c, 0x0c, "Key-in Rate" )           PORT_DIPLOCATION("SW1:3,4")   // 開分比率
	PORT_DIPSETTING(    0x0c, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x10, 0x10, "Credit Limit" )          PORT_DIPLOCATION("SW1:5")     // 進分上限
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x00, "Unlimited" )                                           // 無限制     (if you have 10,000 or more credits, further credits will be rejected)
	PORT_DIPNAME( 0x20, 0x20, "Credit Mode" )           PORT_DIPLOCATION("SW1:6")     // 進分方式
	PORT_DIPSETTING(    0x20, "Coin Acceptor" )                                       // 投幣
	PORT_DIPSETTING(    0x00, "Key-In" )                                              // 開分
	PORT_DIPNAME( 0x40, 0x40, "Payout Mode" )           PORT_DIPLOCATION("SW1:7")     // 退分方式
	PORT_DIPSETTING(    0x40, "Return Coins" )                                        // 退幣
	PORT_DIPSETTING(    0x00, "Key-Out" )                                             // 洗分
	PORT_DIPNAME( 0x80, 0x80, DEF_STR(Demo_Sounds) )  PORT_DIPLOCATION("SW1:8")       // 示範音樂
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )                                          // 無
	PORT_DIPSETTING(    0x80, DEF_STR(On) )                                           // 有

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Double Up Jackpot" )     PORT_DIPLOCATION("SW2:1,2")   // 比倍爆機
	PORT_DIPSETTING(    0x03, "1000" )
	PORT_DIPSETTING(    0x02, "1500" )
	PORT_DIPSETTING(    0x01, "2000" )
	PORT_DIPSETTING(    0x00, "3000" )
	PORT_DIPNAME( 0x0c, 0x0c, "Minimum Bet" )           PORT_DIPLOCATION("SW2:3,4")   // 最小押注
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, "Double Up Game" )        PORT_DIPLOCATION("SW2:5")     // 比倍遊戲
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )                                          // 無
	PORT_DIPSETTING(    0x10, DEF_STR(On) )                                           // 有
	PORT_DIPNAME( 0x20, 0x20, "Title Screen" )          PORT_DIPLOCATION("SW2:6" )    // (not shown in settings display)
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )                                          // (shows disclaimer and copyright message over black background)
	PORT_DIPSETTING(    0x20, DEF_STR(On) )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW2:7" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW2:8" )                                     // (not shown in settings display)

	// FIXME: SW3 doesn't respond in input test - are these hooked up correctly?
	PORT_START("DSW3")
	PORT_DIPUNKNOWN( 0x01, 0x01 )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	// TODO: hook up SW4 shown in input test but not settings display
	// TODO: hook up SW5 shown in input test but not settings display

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r)) // hopper switch
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )      // keep pressed while booting
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )   // stats
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_CONDITION("DSW1", 0x20, EQUALS, 0x20)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_CONDITION("DSW1", 0x20, EQUALS, 0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  PORT_CONDITION("DSW1", 0x40, EQUALS, 0x40)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  PORT_CONDITION("DSW1", 0x40, EQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_INCLUDE(igs_mahjong_matrix)
INPUT_PORTS_END


static INPUT_PORTS_START( wlcc )
	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW3:1" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW3:2" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW3:3" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW3:4" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW3:5" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW3:6" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW3:7" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW3:8" )                                     // (not shown in settings display)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, "Double Up Jackpot" )     PORT_DIPLOCATION("SW2:1,2")   // 比倍爆機
	PORT_DIPSETTING(    0x03, "1000" )
	PORT_DIPSETTING(    0x02, "1500" )
	PORT_DIPSETTING(    0x01, "2000" )
	PORT_DIPSETTING(    0x00, "3000" )
	PORT_DIPNAME( 0x0c, 0x0c, "Minimum Bet" )           PORT_DIPLOCATION("SW2:3,4")   // 最小押注
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x10, 0x10, "Double Up Game" )        PORT_DIPLOCATION("SW2:5")     // 比倍遊戲
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )                                          // 無
	PORT_DIPSETTING(    0x10, DEF_STR(On) )                                           // 有
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW2:6" )                                     // (not shown in settings display)
	PORT_DIPNAME( 0x40, 0x40, "Title Screen" )          PORT_DIPLOCATION("SW2:7" )    // (not shown in settings display)
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )                                          // (shows disclaimer and copyright message over black background)
	PORT_DIPSETTING(    0x40, DEF_STR(On) )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW2:8" )                                     // (not shown in settings display)

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR(Coinage) )        PORT_DIPLOCATION("SW1:1,2")   // 投幣比率
	PORT_DIPSETTING(    0x03, DEF_STR(1C_1C) )
	PORT_DIPSETTING(    0x02, DEF_STR(1C_2C) )
	PORT_DIPSETTING(    0x01, DEF_STR(1C_3C) )
	PORT_DIPSETTING(    0x00, DEF_STR(1C_4C) )
	PORT_DIPNAME( 0x0c, 0x0c, "Key-in Rate" )           PORT_DIPLOCATION("SW1:3,4")   // 開分比率
	PORT_DIPSETTING(    0x0c, "10" )
	PORT_DIPSETTING(    0x08, "20" )
	PORT_DIPSETTING(    0x04, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x10, 0x10, "Credit Limit" )          PORT_DIPLOCATION("SW1:5")     // 進分上限
	PORT_DIPSETTING(    0x10, "500" )
	PORT_DIPSETTING(    0x00, "Unlimited" )                                           // 無限制     (if you have 10,000 or more credits, further credits will be rejected)
	PORT_DIPNAME( 0x20, 0x20, "Credit Mode" )           PORT_DIPLOCATION("SW1:6")     // 進分方式
	PORT_DIPSETTING(    0x20, "Coin Acceptor" )                                       // 投幣
	PORT_DIPSETTING(    0x00, "Key-In" )                                              // 開分
	PORT_DIPNAME( 0x40, 0x40, "Payout Mode" )           PORT_DIPLOCATION("SW1:7")     // 退分方式
	PORT_DIPSETTING(    0x40, "Return Coins" )                                        // 退幣
	PORT_DIPSETTING(    0x00, "Key-Out" )                                             // 洗分
	PORT_DIPNAME( 0x80, 0x80, DEF_STR(Demo_Sounds) )    PORT_DIPLOCATION("SW1:8")     // 示範音樂
	PORT_DIPSETTING(    0x00, DEF_STR(Off) )                                          // 無
	PORT_DIPSETTING(    0x80, DEF_STR(On) )                                           // 有

	PORT_START("DSW4")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "SW4:1" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "SW4:2" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW4:3" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW4:4" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW4:5" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW4:6" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW4:7" )                                     // (not shown in settings display)
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW4:8" )                                     // (not shown in settings display)

	// TODO: hook up SW5 shown in input test but not settings display

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_CONDITION("DSW3", 0x20, EQUALS, 0x20)  // 投幣
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_CONDITION("DSW3", 0x20, EQUALS, 0x00)  // 投幣
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )                                                   // 測試
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )                                                // 査帳
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MEMORY_RESET )                                               // 清除
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  PORT_CONDITION("DSW3", 0x40, EQUALS, 0x40)  // 退幣
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  PORT_CONDITION("DSW3", 0x40, EQUALS, 0x00)  // 退幣
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r)) // hopper switch

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )                                                     // 開始/得分
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )                                                    // 摸/捨 大/雙比倍
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )                                                    // 押注 全比倍
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 )                                                    // 功能 小/半比倍
INPUT_PORTS_END


static INPUT_PORTS_START( lhb )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x02, "Payout Rate" )           // 機率調整
	PORT_DIPSETTING(    0x00, "50%" )
	PORT_DIPSETTING(    0x01, "53%" )
	PORT_DIPSETTING(    0x02, "56%" )
	PORT_DIPSETTING(    0x03, "59%" )
	PORT_DIPSETTING(    0x04, "62%" )
	PORT_DIPSETTING(    0x05, "65%" )
	PORT_DIPSETTING(    0x06, "68%" )
	PORT_DIPSETTING(    0x07, "71%" )
	PORT_DIPSETTING(    0x08, "75%" )
	PORT_DIPSETTING(    0x09, "78%" )
	PORT_DIPSETTING(    0x0a, "81%" )
	PORT_DIPSETTING(    0x0b, "84%" )
	PORT_DIPSETTING(    0x0c, "87%" )
	PORT_DIPSETTING(    0x0d, "90%" )
	PORT_DIPSETTING(    0x0e, "93%" )
	PORT_DIPSETTING(    0x0f, "96%" )
	PORT_DIPNAME( 0x30, 0x30, "YAKUMAN Point" )
	PORT_DIPSETTING(    0x30, "1" )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0xc0, 0x80, "Maximum Bet" )           // 最大押注
	PORT_DIPSETTING(    0xc0, "1" )
	PORT_DIPSETTING(    0x80, "5" )
	PORT_DIPSETTING(    0x40, "10" )
	PORT_DIPSETTING(    0x00, "20" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR(Coinage) )        // 投幣比率
	PORT_DIPSETTING(    0x00, DEF_STR(2C_1C) )
	PORT_DIPSETTING(    0x03, DEF_STR(1C_1C) )
	PORT_DIPSETTING(    0x02, DEF_STR(1C_2C) )
	PORT_DIPSETTING(    0x01, DEF_STR(1C_3C) )
	PORT_DIPNAME( 0x0c, 0x0c, "Minimum Bet" )           // 最小押注
	PORT_DIPSETTING(    0x0c, "1" )
	PORT_DIPSETTING(    0x08, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x70, 0x70, "DAI MANGUAN Cycle" )
	PORT_DIPSETTING(    0x70, "300" )
//  PORT_DIPSETTING(    0x60, "300" )
//  PORT_DIPSETTING(    0x50, "300" )
//  PORT_DIPSETTING(    0x40, "300" )
//  PORT_DIPSETTING(    0x30, "300" )
//  PORT_DIPSETTING(    0x20, "300" )
//  PORT_DIPSETTING(    0x10, "300" )
//  PORT_DIPSETTING(    0x00, "300" )
	PORT_DIPNAME( 0x80, 0x80, "DAI MANGUAN Times" )
	PORT_DIPSETTING(    0x80, "2" )
//  PORT_DIPSETTING(    0x00, "2" )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Score Limit" )           // 得分上限
	PORT_DIPSETTING(    0x03, "1000" )
	PORT_DIPSETTING(    0x02, "2000" )
	PORT_DIPSETTING(    0x01, "5000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x0c, 0x0c, "Credit Limit" )          // 進分上限
	PORT_DIPSETTING(    0x0c, "1000" )
	PORT_DIPSETTING(    0x08, "2000" )
	PORT_DIPSETTING(    0x04, "5000" )
	PORT_DIPSETTING(    0x00, "Unlimited" )
	PORT_DIPNAME( 0x10, 0x10, "CPU Strength" )
	PORT_DIPSETTING(    0x10, "Strong" )
	PORT_DIPSETTING(    0x00, "Weak" )
	PORT_DIPNAME( 0x20, 0x20, "Money Type" )
	PORT_DIPSETTING(    0x20, "Coins" )
	PORT_DIPSETTING(    0x00, "Notes" )
	PORT_DIPNAME( 0xc0, 0xc0, "Don Den Times" )         // 交換牌次數
	PORT_DIPSETTING(    0xc0, "0" )
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x40, "5" )
	PORT_DIPSETTING(    0x00, "8" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR(Demo_Sounds) )
	PORT_DIPSETTING(    0x01, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )
	PORT_DIPNAME( 0x02, 0x00, "In-Game Music" )
	PORT_DIPSETTING(    0x02, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )
	PORT_DIPNAME( 0x0c, 0x00, "Girls" )
	PORT_DIPSETTING(    0x0c, DEF_STR( No ) )
	PORT_DIPSETTING(    0x08, "Dressed" )
	PORT_DIPSETTING(    0x04, "Underwear" )
	PORT_DIPSETTING(    0x00, "Nude" )
	PORT_DIPNAME( 0x10, 0x10, "Key-In Rate" )           // 開分比率
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "10" )
	PORT_DIPNAME( 0x20, 0x20, "Payout Mode" )
	PORT_DIPSETTING(    0x20, "Key-Out" )
	PORT_DIPSETTING(    0x00, "Return Coins" )
	PORT_DIPNAME( 0x40, 0x40, "Credit Mode" )
	PORT_DIPSETTING(    0x40, "Coin Acceptor" )
	PORT_DIPSETTING(    0x00, "Key-In" )
	PORT_DIPNAME( 0x80, 0x80, "Last Chance" )
	PORT_DIPSETTING(    0x80, DEF_STR(Off) )
	PORT_DIPSETTING(    0x00, DEF_STR(On) )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x01, 0x01, "In-Game Bet" )
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN( 0x02, 0x02 )
	PORT_DIPUNKNOWN( 0x04, 0x04 )
	PORT_DIPUNKNOWN( 0x08, 0x08 )
	PORT_DIPUNKNOWN( 0x10, 0x10 )
	PORT_DIPUNKNOWN( 0x20, 0x20 )
	PORT_DIPUNKNOWN( 0x40, 0x40 )
	PORT_DIPUNKNOWN( 0x80, 0x80 )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r)) // hopper switch
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MEMORY_RESET )   // system reset
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW )       // keep pressed while booting
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )    // stats
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )          PORT_CONDITION("DSW4", 0x40, EQUALS, 0x40)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )   PORT_CONDITION("DSW4", 0x40, EQUALS, 0x00)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )  PORT_CONDITION("DSW4", 0x20, EQUALS, 0x20)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  PORT_CONDITION("DSW4", 0x20, EQUALS, 0x00)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )        // shows garbage in test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_INCLUDE(igs_mahjong_matrix)

	//PORT_MODIFY("KEY4")
	//PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2) // shows garbage in test mode
INPUT_PORTS_END


// vbowlhk: joystick only, no cabinet linking
static INPUT_PORTS_START( vbowlhk )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Sexy Interlude" )      PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x20, 0x20, "Open Picture" )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN_DIPLOC(0x40, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) )      PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Difficulty ) )      PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy   ) )   // 5
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )   // 7
	PORT_DIPSETTING(    0x01, DEF_STR( Medium ) )   // 9
	PORT_DIPSETTING(    0x00, DEF_STR( Hard   ) )   // 11
	PORT_DIPNAME( 0x04, 0x04, "Spares To Win (Frames 1-5)" )      PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x18, 0x18, "Points To Win (Frames 6-10)" )      PORT_DIPLOCATION("SW2:4,5")
	PORT_DIPSETTING(    0x18, "160" )
	PORT_DIPSETTING(    0x10, "170" )
	PORT_DIPSETTING(    0x08, "180" )
	PORT_DIPSETTING(    0x00, "190" )
	PORT_DIPUNKNOWN_DIPLOC(0x20, IP_ACTIVE_LOW, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC(0x40, IP_ACTIVE_LOW, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC(0x80, IP_ACTIVE_LOW, "SW2:8" )

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC(0x01, IP_ACTIVE_LOW, "SW3:1" )
	PORT_DIPUNKNOWN_DIPLOC(0x02, IP_ACTIVE_LOW, "SW3:2" )
	PORT_DIPUNKNOWN_DIPLOC(0x04, IP_ACTIVE_LOW, "SW3:3" )
	PORT_DIPUNKNOWN_DIPLOC(0x08, IP_ACTIVE_LOW, "SW3:4" )
	PORT_DIPUNKNOWN_DIPLOC(0x10, IP_ACTIVE_LOW, "SW3:5" )
	PORT_DIPUNKNOWN_DIPLOC(0x20, IP_ACTIVE_LOW, "SW3:6" )
	PORT_DIPUNKNOWN_DIPLOC(0x40, IP_ACTIVE_LOW, "SW3:7" )
	PORT_SERVICE_DIPLOC(0x80, IP_ACTIVE_LOW, "SW3:8" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x81, 0x81, "Protection & Comm Test" )      PORT_DIPLOCATION("SW4:1,8") // includes IGS011 and IGS012 protection tests
	PORT_DIPSETTING(    0x81, "No (0)" )
	PORT_DIPSETTING(    0x80, "No (1)" )
	PORT_DIPSETTING(    0x01, "No (2)" )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPUNKNOWN_DIPLOC(0x02, IP_ACTIVE_LOW, "SW4:2" )
	PORT_DIPUNKNOWN_DIPLOC(0x04, IP_ACTIVE_LOW, "SW4:3" )
	PORT_DIPUNKNOWN_DIPLOC(0x08, IP_ACTIVE_LOW, "SW4:4" )
	PORT_DIPUNKNOWN_DIPLOC(0x10, IP_ACTIVE_LOW, "SW4:5" )
	PORT_DIPUNKNOWN_DIPLOC(0x20, IP_ACTIVE_LOW, "SW4:6" )
	PORT_DIPUNKNOWN_DIPLOC(0x40, IP_ACTIVE_LOW, "SW4:7" )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)

	PORT_START("AN0")
	PORT_BIT( 0xff, 0xff, IPT_UNKNOWN )

	PORT_START("AN1")
	PORT_BIT( 0xff, 0xff, IPT_UNKNOWN )

	PORT_START("CONF")
INPUT_PORTS_END


// vbowl: add trackball and linked cabinets options
static INPUT_PORTS_START( vbowl )
	PORT_INCLUDE(vbowlhk)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Controls ) )      PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Joystick ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Trackball ) )

	PORT_MODIFY("DSW3")
	PORT_DIPNAME( 0x03, 0x03, "Cabinet ID" )      PORT_DIPLOCATION("SW3:1,2")
	PORT_DIPSETTING(    0x03, "1" )
	PORT_DIPSETTING(    0x02, "2" )
	PORT_DIPSETTING(    0x01, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x04, 0x04, "Linked Cabinets" )      PORT_DIPLOCATION("SW3:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("AN0")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1)

	PORT_MODIFY("AN1")
	PORT_BIT( 0xff, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(30) PORT_KEYDELTA(30) PORT_PLAYER(1)

	PORT_MODIFY("CONF")
	PORT_CONFNAME( 0x01, 0x01, "Trackball Orientation" )
	PORT_CONFSETTING(    0x00, "Direct" )
	PORT_CONFSETTING(    0x01, "Natural" )
INPUT_PORTS_END


// vbowlj: remove "open picture"
static INPUT_PORTS_START( vbowlj )
	PORT_INCLUDE(vbowl)

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Controls ) )      PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Joystick ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Trackball ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )       PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/***************************************************************************

    Machine Drivers

***************************************************************************/

// for debugging

#if 0
static const gfx_layout layout_16x16x1 =
{
	16,16,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP16(15,-1) },
	{ STEP16(0,16*1) },
	16*16*1
};

static GFXDECODE_START( gfx_igs011 )
	GFXDECODE_ENTRY( "igs011", 0, gfx_8x8x4_packed_lsb,   0, 0x80 )
	GFXDECODE_ENTRY( "igs011", 0, gfx_16x16x4_packed_lsb, 0, 0x80 )
	GFXDECODE_ENTRY( "igs011", 0, gfx_8x8x8_raw,          0, 0x08 )
	GFXDECODE_ENTRY( "igs011", 0, gfx_16x16x8_raw,        0, 0x08 )
GFXDECODE_END

static GFXDECODE_START( gfx_igs011_hi )
	GFXDECODE_ENTRY( "igs011", 0, gfx_8x8x4_packed_lsb,   0, 0x80 )
	GFXDECODE_ENTRY( "igs011", 0, gfx_16x16x4_packed_lsb, 0, 0x80 )
	GFXDECODE_ENTRY( "igs011", 0, gfx_8x8x8_raw,          0, 0x08 )
	GFXDECODE_ENTRY( "igs011", 0, gfx_16x16x8_raw,        0, 0x08 )
	GFXDECODE_ENTRY( "igs011:gfx_hi", 0, layout_16x16x1,      0, 0x80 )
GFXDECODE_END
#endif

void igs011_state_base::igs011_base(machine_config &config)
{
	M68000(config, m_maincpu, XTAL(22'000'000)/3);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	SCREEN(config, m_screen);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1282));
	m_screen->set_size(512, 260);
	m_screen->set_visarea(0, 512-1, 0, 240-1);
	m_screen->set_screen_update(m_igs011, FUNC(igs011_device::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x2000/4);
	m_palette->set_membits(8);

	IGS011(config, m_igs011, XTAL(22'000'000));
	m_igs011->set_palette(m_palette);
	m_igs011->set_host_space(m_maincpu, AS_PROGRAM);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
}

void igs011_oki_state::igs011_oki_base(machine_config &config)
{
	igs011_base(config);

	OKIM6295(config, m_oki, XTAL(22'000'000)/21, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "mono", 1.0);
}

void igs011_oki_state::igs011_oki_opll_base(machine_config &config)
{
	igs011_base(config);

	m_maincpu->set_vblank_int("screen", FUNC(igs011_state_base::irq6_line_hold));

	OKIM6295(config, m_oki, XTAL(22'000'000)/21, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.95);

	YM2413(config, "ymsnd", XTAL(3'579'545)).add_route(ALL_OUTPUTS, "mono", 1.9);
}

template <uint8_t Irq>
TIMER_DEVICE_CALLBACK_MEMBER( igs011_state_base::timer_irq_cb )
{
	m_maincpu->set_input_line(Irq, HOLD_LINE);
}

void igs011_oki_state::drgnwrld(machine_config &config)
{
	igs011_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &igs011_oki_state::drgnwrld_mem);
	m_maincpu->set_vblank_int("screen", FUNC(igs011_state_base::irq6_line_hold));

	TIMER(config, "timer_irq").configure_periodic(FUNC(igs011_oki_state::timer_irq_cb<5>), attotime::from_hz(240));

	IGS003(config, m_igs003); // IGS003 or IGS003c
	m_igs003->in_pa_callback().set_ioport("IN0");
	m_igs003->in_pb_callback().set_ioport("IN1");
	m_igs003->in_pc_callback().set_ioport("IN2");
	m_igs003->out_pa_callback().set(FUNC(igs011_oki_state::drgnwrld_igs003_a_w));
	// TODO: port B bits 0, 2, 4, 6 are outputs, purpose unknown

	OKIM6295(config, m_oki, XTAL(22'000'000)/21, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.95);

	YM3812(config, "ymsnd", XTAL(3'579'545)).add_route(ALL_OUTPUTS, "mono", 1.9);
}

void igs011_oki_state::drgnwrld_igs012(machine_config &config)
{
	drgnwrld(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &igs011_oki_state::drgnwrld_igs012_mem);

	IGS012(config, m_igs012);
}


INTERRUPT_GEN_MEMBER(igs011_oki_state::lhb_vblank_irq)
{
	if (!m_lhb_irq_enable)
		return;

	m_maincpu->set_input_line(6, HOLD_LINE);
}

TIMER_DEVICE_CALLBACK_MEMBER( igs011_oki_state::lhb_timer_irq_cb )
{
	if (!m_lhb_irq_enable)
		return;

	m_maincpu->set_input_line(5, HOLD_LINE);
}

void igs011_oki_state::lhb(machine_config &config)
{
	igs011_oki_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &igs011_oki_state::lhb_mem);
	m_maincpu->set_vblank_int("screen", FUNC(igs011_oki_state::lhb_vblank_irq));

	TIMER(config, "timer_irq").configure_periodic(FUNC(igs011_oki_state::lhb_timer_irq_cb), attotime::from_hz(120));
	// irq 3 points to an apparently unneeded routine

	HOPPER(config, m_hopper, attotime::from_msec(50));
}

void igs011_oki_state::wlcc(machine_config &config)
{
	igs011_oki_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &igs011_oki_state::wlcc_mem);
	m_maincpu->set_vblank_int("screen", FUNC(igs011_oki_state::irq6_line_hold));

	TIMER(config, "timer_irq").configure_periodic(FUNC(igs011_oki_state::timer_irq_cb<3>), attotime::from_hz(60));

	IGS003(config, m_igs003);
	m_igs003->in_pa_callback().set_ioport("IN0");
	m_igs003->out_pc_callback().set(FUNC(igs011_oki_state::wlcc_igs003_c_w));

	HOPPER(config, m_hopper, attotime::from_msec(50));
}


void igs011_oki_state::xymg_base(machine_config &config)
{
	igs011_oki_base(config);

	m_maincpu->set_vblank_int("screen", FUNC(igs011_oki_state::irq6_line_hold));

	TIMER(config, "timer_irq").configure_periodic(FUNC(igs011_oki_state::timer_irq_cb<3>), attotime::from_hz(60));

	HOPPER(config, m_hopper, attotime::from_msec(50));
}

void igs011_oki_state::xymg(machine_config &config)
{
	xymg_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &igs011_oki_state::xymg_mem);

	IGS003(config, m_igs003);
	m_igs003->in_pa_callback().set_ioport("COIN");
	m_igs003->out_pb_callback().set(FUNC(igs011_oki_state::igs003_inputs_w));
	m_igs003->in_pc_callback().set(FUNC(igs011_oki_state::key_matrix_r));
}


void igs011_oki_state::xymga(machine_config &config)
{
	xymg_base(config);

	// the code accesses an IGS003, but the socket is empty
	m_maincpu->set_addrmap(AS_PROGRAM, &igs011_oki_state::xymga_mem);
}

void igs011_oki_state::sdmg2641c(machine_config &config)
{
	xymg(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &igs011_oki_state::sdmg2641c_mem);

	// only mode 1 with x = 3 is checked (at boot), the other checks don't act on the result
	m_igs003->set_hold_taps(igs003_device::hold_taps::B);
	m_igs003->set_hold_xpos(3, 8, 10, 14);
}


void igs011_oki_state::lhb2(machine_config &config)
{
	igs011_oki_opll_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &igs011_oki_state::lhb2_mem);

	TIMER(config, "timer_irq").configure_periodic(FUNC(igs011_oki_state::timer_irq_cb<5>), attotime::from_hz(240)); // lev5 frequency drives the music tempo

	//GFXDECODE(config, "gfxdecode", m_palette, gfx_igs011_hi);

	IGS003(config, m_igs003); // marked 8255
	m_igs003->out_pa_callback().set(FUNC(igs011_oki_state::igs003_inputs_w));
	m_igs003->in_pb_callback().set(FUNC(igs011_oki_state::key_matrix_r));
	m_igs003->out_pc_callback().set(FUNC(igs011_oki_state::lhb2_igs003_c_w));
	// TODO: port C bits 4-7 are inputs, purpose unknown

	HOPPER(config, m_hopper, attotime::from_msec(50));
}


void igs011_serial_state::nkishusp(machine_config &config)
{
	igs011_oki_opll_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &igs011_serial_state::nkishusp_serial_mem);

	TIMER(config, "timer_irq").configure_periodic(FUNC(igs011_serial_state::timer_irq_cb<3>), attotime::from_hz(240)); // lev3 frequency drives the music tempo

	// VSync 60.0052Hz, HSync 15.620kHz

	//GFXDECODE(config, "gfxdecode", m_palette, gfx_igs011_hi);

	IGS003(config, m_igs003); // marked 8255
	m_igs003->out_pa_callback().set(FUNC(igs011_serial_state::igs003_inputs_w));
	m_igs003->in_pb_callback().set(FUNC(igs011_serial_state::key_matrix_r));
	m_igs003->out_pc_callback().set(FUNC(igs011_serial_state::lhb2_igs003_c_w));
	// TODO: port C bits 4-7 are inputs, purpose unknown
	m_igs003->set_hold_xpos(1, 6, 10, 14);

	HOPPER(config, m_hopper, attotime::from_msec(50));
}


void igs011_oki_state::lhb2cpgs(machine_config &config)
{
	lhb2(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &igs011_oki_state::lhb3_mem);

	m_igs003->set_hold_taps(igs003_device::hold_taps::B);
	m_igs003->set_hold_xpos(3, 8, 10, 14);
}

void igs011_serial_state::lhb3(machine_config &config)
{
	nkishusp(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &igs011_serial_state::lhb3_serial_mem);

	m_igs003->set_hold_taps(igs003_device::hold_taps::B);
	m_igs003->set_hold_xpos(3, 8, 10, 14);
}

void igs011_serial_state::tygn(machine_config &config)
{
	nkishusp(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &igs011_serial_state::lhb3_serial_mem);

	// IGS003A, ports A and B are inputs, port C is output
	m_igs003->in_pa_callback().set_ioport("IN0");
	m_igs003->in_pb_callback().set_ioport("IN1");
	m_igs003->out_pa_callback().set_nop();
}


void vbowl_state::sound_irq(int state)
{
	//m_maincpu->set_input_line(3, state);
}

void vbowl_state::vbowl(machine_config &config)
{
	igs011_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &vbowl_state::vbowl_mem);
	m_maincpu->set_vblank_int("screen", FUNC(vbowl_state::irq6_line_hold));

	TIMER(config, "timer_irq").configure_periodic(FUNC(vbowl_state::timer_irq_cb<3>), attotime::from_hz(240)); // lev3 frequency drives the music tempo
	// irq 5 points to a debug function (all routines are clearly patched out)
	// irq 4 points to an apparently unneeded routine

	IGS012(config, m_igs012);

	IGS003(config, m_igs003); // IGS003e
	m_igs003->in_pa_callback().set_ioport("IN0");
	m_igs003->in_pb_callback().set_ioport("IN1");
	m_igs003->out_pc_callback().set(FUNC(vbowl_state::vbowl_igs003_c_w));
	// TODO: port C bit 4 is an input, purpose unknown

	m_screen->screen_vblank().set(FUNC(vbowl_state::screen_vblank));
	//GFXDECODE(config, "gfxdecode", m_palette, gfx_igs011_hi);

	ICS2115(config, m_ics, 33.8688_MHz_XTAL);
	m_ics->irq().set(FUNC(vbowl_state::sound_irq));
	m_ics->add_route(ALL_OUTPUTS, "mono", 0.375);
}

void vbowl_state::vbowlhk(machine_config &config)
{
	vbowl(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &vbowl_state::vbowlhk_mem);

	m_igs003->set_hold_taps(igs003_device::hold_taps::B);
	m_igs003->set_hold_xpos(3, 8, 10, 14);
}


/***************************************************************************

    ROMs Loading

***************************************************************************/

/***************************************************************************

Dragon World (World, V040O)
(C) 1997 IGS

Chips:
  1x 68000 (main)
  1x AC0A26 (equivalent to OKI M6295)(sound)
  1x 6564L (equivalent to YM3812)(sound)
  1x custom IGS003c (marked on PCB as 8255)
  1x oscillator 22.0000MHz (main)
  1x oscillator 3.579545MHz (sound)
  1x custom IGS011 (FPGA?)

ROMs:
  1x MX27C4096 (u3)(main) (dumped)
  1x custom IGSD0301 (mask rom) (not dumped yet)
  1x NEC D27C2001D (IGSS0302)(sound) (not dumped yet)

Notes:
  1x JAMMA edge connector
  1x trimmer (volume)
  3x 8 switches dips
  PCB serial number is: 0105-5

***************************************************************************/

ROM_START( drgnwrld )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "chinadr-v0400.u3", 0x00000, 0x80000, CRC(a6daa2b8) SHA1(0cbfd001c1fd82a6385453d1c2a808add67746af) )

	ROM_REGION( 0x400000, "igs011", 0 )
	ROM_LOAD( "igs-d0301.u39", 0x000000, 0x400000, CRC(78ab45d9) SHA1(c326ee9f150d766edd6886075c94dea3691b606d) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "igs-s0302.u43", 0x00000, 0x40000, CRC(fde63ce1) SHA1(cc32d2cace319fe4d5d0aa96d7addb2d1def62f2) )
ROM_END

/***************************************************************************

Dragon World (World, V030O)
(C) 1995 IGS

Chips:
  1x MC68HC000P10           U2 (main)
  1x CUSTOM IGS011          U24
  1x oscillator 22.000MHz   U25
  1x oscillator 3.579545MHz X1

ROMs:
  1x MX27C4096              U3 (main)

Notes:
  1x JAMMA edge connector
  1x trimmer (volume)
  3x 8 switches dips
  PCB serial number is: 0105-1

***************************************************************************/

ROM_START( drgnwrldv30 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "chinadr-v0300.u3", 0x00000, 0x80000, CRC(5ac243e5) SHA1(50cccff0307239187ac2b65331ad2bcc666f8033) )

	ROM_REGION( 0x400000, "igs011", 0 )
	ROM_LOAD( "igs-d0301.u39", 0x000000, 0x400000, CRC(78ab45d9) SHA1(c326ee9f150d766edd6886075c94dea3691b606d) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "igs-s0302.u43", 0x00000, 0x40000, CRC(fde63ce1) SHA1(cc32d2cace319fe4d5d0aa96d7addb2d1def62f2) )
ROM_END

/***************************************************************************

Dragon World (World, V021O)
(C) 1995 IGS

Chips:
    1x MC68HC000P10             u2      16/32-bit Microprocessor - main
    1x AR17961-AP0642           u41     4-Channel Mixing ADCPM Voice Synthesis LSI - sound
    1x 6564L                    u40     FM Operator Type-L II (OPL II) - sound
    1x LS138S                   u42     sound
    1x LM7805CV                 u38     sound
    1x UPC1242H                 u46     sound
    1x IGS003                   u10     Programmable Peripheral Interface
    1x oscillator   22.0000MHz  u25
    1x oscillator   3.579545MHz x1

ROMs:
    1x  M27C4096                u3      (main) dumped
    1x  custom IGSD0301         u39     not dumped yet
    x   NEC D27C2001D           u43     (sound) dumped

RAMs:
    2x  UM6264                  u4,u5
    2x  CXK5863AP               u31,u32

PLDs:
    1x  custom IGS011 (FPGA?)   u24     not dumped
    1x  custom IGS012 (FPGA?)   u1      not dumped
    2x  PAL16L8A                u17,u18 read protected
    2x  ATF22V10B               u15,u45 read protected
    1x  ATV750                  u16     read protected

***************************************************************************/

ROM_START( drgnwrldv21 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "china-dr-v-0210.u3", 0x00000, 0x80000, CRC(60c2b018) SHA1(58563e3ccb51bd9d8362aa17c23743bb5a593c3b) )

	ROM_REGION( 0x400000, "igs011", 0 )
	ROM_LOAD( "igs-d0301.u39", 0x000000, 0x400000, CRC(78ab45d9) SHA1(c326ee9f150d766edd6886075c94dea3691b606d) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "china-dr-sp.u43", 0x00000, 0x40000, CRC(fde63ce1) SHA1(cc32d2cace319fe4d5d0aa96d7addb2d1def62f2) )
ROM_END

/***************************************************************************

中國龍/Chuugokuryuu (china dragon jpn ver.)
(c)IGS
Distributed by ALTA

MAIN CPU   : 68000
I/O        : IGS003 (=8255)
SOUND ?    : 6564L  (=OPL?)  , AR17961 (=M6295?)
CRTC ?     : IGS011
SOUND CPU? : IGSD0301 (DIP 42P)
OSC        : 22Mhz , 3.579545Mhz
DIPSW      : 8bitx 3 (SW3 is not used)
OTHER      : IGS012

MAIN PRG   : "CHINA DRAGON U020J" (japan)
SOUND PRG? : "CHINA DRAGON SP"
SOUND DATA?: "CHINA DRAGON U44"

***************************************************************************/

ROM_START( drgnwrldv20j )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "china_jp.v20", 0x00000, 0x80000, CRC(9e018d1a) SHA1(fe14e6344434cabf43685e50fd49c90f05f565be) )

	ROM_REGION( 0x420000, "igs011", 0 )
	// igs-d0301.u39 wasn't in this set
	ROM_LOAD( "igs-d0301.u39", 0x000000, 0x400000, CRC(78ab45d9) SHA1(c326ee9f150d766edd6886075c94dea3691b606d) )
	ROM_LOAD( "china.u44",     0x400000, 0x020000, CRC(10549746) SHA1(aebd83796679c85b43ad514b2771897f94e61294) ) // 1xxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "igs-s0302.u43", 0x00000, 0x40000, CRC(fde63ce1) SHA1(cc32d2cace319fe4d5d0aa96d7addb2d1def62f2) ) // original label: "sp"
ROM_END

ROM_START( drgnwrldv21j )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "v-021j", 0x00000, 0x80000, CRC(2f87f6e4) SHA1(d43065b078fdd9605c121988ad3092dce6cf0bf1) )

	ROM_REGION( 0x420000, "igs011", 0 )
	ROM_LOAD( "igs-d0301.u39", 0x000000, 0x400000, CRC(78ab45d9) SHA1(c326ee9f150d766edd6886075c94dea3691b606d) )
	ROM_LOAD( "cg",            0x400000, 0x020000, CRC(2dda0be3) SHA1(587b7cab747d4336515c98eb3365341bb6c7e5e4) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "igs-s0302.u43", 0x00000, 0x40000, CRC(fde63ce1) SHA1(cc32d2cace319fe4d5d0aa96d7addb2d1def62f2) ) // original label: "sp"
ROM_END

/***************************************************************************

  東方之珠/Dung1 Fong1 Zi1 Zyu1
  (Hong Kong version of 中國龍/Zhōngguó Lóng, V011H)

***************************************************************************/

ROM_START( drgnwrldv11h )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "c_drgn_hk.u3", 0x00000, 0x80000, CRC(182037ce) SHA1(141b698777533e57493e588d2526523d4bd3e17d) )

	ROM_REGION( 0x400000, "igs011", 0 )
	ROM_LOAD( "igs-d0301.u39", 0x000000, 0x400000, CRC(78ab45d9) SHA1(c326ee9f150d766edd6886075c94dea3691b606d) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "igs-s0302.u43", 0x00000, 0x40000, CRC(fde63ce1) SHA1(cc32d2cace319fe4d5d0aa96d7addb2d1def62f2) )
ROM_END

ROM_START( drgnwrldv11ha )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "u3", 0x00000, 0x80000, CRC(b68113c4) SHA1(618fbf8ef71fcab5cd887e2d06f08169aec9e59f) ) // label not readable

	ROM_REGION( 0x400000, "igs011", 0 )
	ROM_LOAD( "igs-d0301.u39", 0x000000, 0x400000, CRC(78ab45d9) SHA1(c326ee9f150d766edd6886075c94dea3691b606d) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "china-dr-sp.u43", 0x00000, 0x40000, CRC(fde63ce1) SHA1(cc32d2cace319fe4d5d0aa96d7addb2d1def62f2) )
ROM_END

/***************************************************************************

中國龍/Zhōngguó Lóng (China, V010C)
(C) 1995 IGS

Chips:
  CPU 1x MC68HC000P10 (main)
  1x AR17961-AP0642 (equivalent to OKI M6295)(sound)
  1x 6564L (equivalent to YM3812)(sound)
  1x LS138S (sound)
  1x LM7805CV (sound)
  1x UPC1242H (sound)
  1x custom IGS003 (marked on PCB as 8255)
  1x oscillator 22.0000MHz (main)
  1x oscillator 3.579545MHz (sound)
  1x custom IGS011 (FPGA?)

ROMs:
  1x maskrom 256x16 IGSD0303 (u3)(main)
  1x maskrom 2Mx16 UM23V32000 (IGSD0301)(u39)(gfx)
  1x empty socket for 27C040 (u44)
  1x maskrom NEC D27C2001D (IGSS0302)(u43)(sound)
  2x PAL16L8ACN (u17,u18)(read protected)
  2x PALATF22V10B (u15,u45)
  1x empty space for additional PALATV750 (u16)

Notes:
  1x JAMMA edge connector
  1x trimmer (volume)
  3x 8x2 switches DIP

The PCB is perfectly working, empty spaces and empty sockets are clearly intended to be empty.
25/07/2007 f205v Corrado Tomaselli Gnoppi

***************************************************************************/

ROM_START( drgnwrldv10c )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "igs-d0303.u3", 0x00000, 0x80000, CRC(3b3c29bb) SHA1(77b7e58104314303985c283cce3aec40bd7b9334) )

	ROM_REGION( 0x400000, "igs011", 0 )
	//ROM_LOAD( "igs-0301.u39", 0x000000, 0x400000, CRC(655ab941) SHA1(4bbefb27e8971446998508969661042c5111bc72) ) // bad dump
	ROM_LOAD( "igs-d0301.u39", 0x000000, 0x400000, CRC(78ab45d9) SHA1(c326ee9f150d766edd6886075c94dea3691b606d) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "igs-s0302.u43", 0x00000, 0x40000, CRC(fde63ce1) SHA1(cc32d2cace319fe4d5d0aa96d7addb2d1def62f2) )

	ROM_REGION( 0x40000, "plds", 0 )
	ROM_LOAD( "ccdu15.u15", 0x000, 0x2e5, CRC(a15fce69) SHA1(3e38d75c7263bfb36aebdbbd55ebbdd7ca601633) )
	//ROM_LOAD( "ccdu17.u17.bad.dump", 0x000, 0x104, CRC(e9cd78fb) SHA1(557d3e7ef3b25c1338b24722cac91bca788c02b8) )
	//ROM_LOAD( "ccdu18.u18.bad.dump", 0x000, 0x104, CRC(e9cd78fb) SHA1(557d3e7ef3b25c1338b24722cac91bca788c02b8) )
	ROM_LOAD( "ccdu45.u45", 0x000, 0x2e5, CRC(a15fce69) SHA1(3e38d75c7263bfb36aebdbbd55ebbdd7ca601633) )
ROM_END

/***************************************************************************

  東方之珠/dongbang jiju
  (Korea version of 中國龍/Zhōngguó Lóng)

***************************************************************************/

ROM_START( drgnwrldv40k )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "v-040k.u3", 0x00000, 0x80000, CRC(397404ef) SHA1(5228558760be7c103d4b9e2e2a31ca5619ca8055) )

	ROM_REGION( 0x400000, "igs011", 0 )
	ROM_LOAD( "igs-d0301.u39", 0x000000, 0x400000, CRC(78ab45d9) SHA1(c326ee9f150d766edd6886075c94dea3691b606d) )
	// u44 unpopulated

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "igs-s0302.u43", 0x00000, 0x40000, CRC(fde63ce1) SHA1(cc32d2cace319fe4d5d0aa96d7addb2d1def62f2) )

	ROM_REGION( 0x40000, "plds", 0 )
	ROM_LOAD( "ccdu15.u15", 0x000, 0x2e5, CRC(a15fce69) SHA1(3e38d75c7263bfb36aebdbbd55ebbdd7ca601633) )
	//ROM_LOAD( "ccdu17.u17.bad.dump", 0x000, 0x104, CRC(e9cd78fb) SHA1(557d3e7ef3b25c1338b24722cac91bca788c02b8) )
	//ROM_LOAD( "ccdu18.u18.bad.dump", 0x000, 0x104, CRC(e9cd78fb) SHA1(557d3e7ef3b25c1338b24722cac91bca788c02b8) )
	ROM_LOAD( "ccdu45.u45", 0x000, 0x2e5, CRC(a15fce69) SHA1(3e38d75c7263bfb36aebdbbd55ebbdd7ca601633) )
ROM_END

/***************************************************************************

    萬里長城/Wànlǐ Chángchéng (The Great Wall)

    Other files in the zip:

     5.942    16-6126.G16
    14.488    U3-9911.G22
    14.488    U4-82E6.G22
    14.488    U5-6C5E.G22

***************************************************************************/

ROM_START( wlcc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "wlcc4096.rom", 0x00000, 0x80000, CRC(3b16729f) SHA1(4ef4e5cbd6ccc65775e36c2c8b459bc1767d6574) )
	ROM_CONTINUE        (                 0x00000, 0x80000 ) // 1ST+2ND IDENTICAL

	ROM_REGION( 0x280000, "igs011", 0 )
	ROM_LOAD( "m0201-ig.160", 0x000000, 0x200000, CRC(ec54452c) SHA1(0ee7ffa3d4845af083944e64faf5a1c78247aaa2) )
	ROM_LOAD( "wlcc.gfx",     0x200000, 0x080000, CRC(1f7ad299) SHA1(ab0a8fb31906519b9352ba172def48456e8d565c) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "040-c3c2.snd", 0x00000, 0x80000, CRC(220949aa) SHA1(1e0dba168a0687d32aaaed42714ae24358f4a3e7) ) // 2 banks
	ROM_CONTINUE(             0x00000, 0x80000 ) // 1ST+2ND IDENTICAL
ROM_END

/***************************************************************************

    龙虎榜/Lóng Hǔ Bǎng (V035C)

    Other files in the zip:

     5.938    16V8.jed
    14.464    LHB-U33.jed
    14.488    LHB-U34.jed
    14.488    LHB-U35.jed

***************************************************************************/

ROM_START( lhb )
	ROM_REGION( 0x80000, "maincpu", 0 )
	// identical to LHB-4096
	ROM_LOAD16_WORD_SWAP( "v305j-409", 0x00000, 0x80000, CRC(701de8ef) SHA1(4a77160f642f4de02fa6fbacf595b75c0d4a505d) )

	ROM_REGION( 0x200000, "igs011", 0 )
	ROM_LOAD( "m0201-ig.160", 0x000000, 0x200000, CRC(ec54452c) SHA1(0ee7ffa3d4845af083944e64faf5a1c78247aaa2) )

	ROM_REGION( 0x80000, "oki", 0 )
	// identical to 040-c3c2.snd
	ROM_LOAD( "m0202.snd", 0x00000, 0x80000, CRC(220949aa) SHA1(1e0dba168a0687d32aaaed42714ae24358f4a3e7) ) // 2 banks
	ROM_CONTINUE(          0x00000, 0x80000 ) // 1ST+2ND IDENTICAL
ROM_END

/***************************************************************************

龙虎榜/Lóng hǔ bǎng (V033C)

PCB Layout
----------

IGS PCB NO-T0093
|---------------------------------------|
|uPD1242H     VOL       DSW5            |
|  IGS_M0202                            |
|             AR17961   DSW4            |
|                             CY7C185   |
|                       DSW3            |
|      8255             DSW2  CY7C185   |
|                       DSW1            |
|1                                      |
|8                           DIP32      |
|W             |-------|                |
|A             |       |     IGS_M0201  |
|Y  BATTERY    |IGS011 |                |
|              |       |          PAL   |
|              |-------|                |
|                         TC524256      |
|    MAJ_V-033C                         |
|                         TC524256      |
|1      6264                            |
|0                        TC524256      |
|W      6264    22.285MHz               |
|A         PAL            TC524256      |
|Y         PAL                          |
| SPDT_SW  PAL      68000               |
|---------------------------------------|
Notes:
      Uses common 10-way/18-way Mahjong pinout
      TC524256 - Toshiba TC524256BZ-80 256k x4 Dual Port VRAM (ZIP28)
      CY7C185  - Cypress CY7C185-20PC 8k x8 SRAM (DIP28)
      6264     - UT6264PC-70LL 8k x8 SRAM (DIP28)
      IGS011   - Custom IGS IC (QFP160)
      AR17961  - == OkiM6295 (QFP44)
      DIP32    - Empty socket, maybe a ROM missing, maybe not used?

      ROMs -
            MAJ_V-033C - Main Program (27C4096)
            IGS_M0201  - Graphics (16M maskROM)
            IGS_M0202  - OKI samples (4M maskROM)

***************************************************************************/

ROM_START( lhbv33c )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "maj_v-033c.u30", 0x00000, 0x80000, CRC(02a0b716) SHA1(cd0ee32ea69f66768196b0e9b4df0fae3af84ed3) )

	ROM_REGION( 0x200000, "igs011", 0 )
	ROM_LOAD( "igs_m0201.u15", 0x000000, 0x200000, CRC(ec54452c) SHA1(0ee7ffa3d4845af083944e64faf5a1c78247aaa2) )

	ROM_REGION( 0x80000, "oki", 0 )
	// identical to 040-c3c2.snd
	ROM_LOAD( "igs_m0202.u39", 0x00000, 0x80000, CRC(106ac5f7) SHA1(5796a880c3424e3d2251b2223a0e594957afecaf) ) // 2 banks

ROM_END

/***************************************************************************

大阪城/Daai6 Baan2 Sing4

PCB Layout
----------

IGS PCB NO-T0084-1
|---------------------------------------|
|uPD1242H     VOL       DSW5            |
|  IGS_M0202                            |
|             AR17961   DSW4            |
|                             CY7C185   |
|                       DSW3            |
|      8255             DSW2  CY7C185   |
|                       DSW1            |
|1                                      |
|8                           MAJ-H_CG   |
|W    PAL      |-------|                |
|A             |       |     IGS_M0201  |
|Y    PAL      |IGS011 |                |
|              |       |          PAL   |
|     PAL      |-------|                |
|                         TC524256      |
|     6264                              |
|                         TC524256      |
|1    6264                              |
|0           22.0994MHz   TC524256      |
|W    MAJ-H_V027H                       |
|A                        TC524256      |
|Y         BATTERY                      |
| SPDT_SW           68000               |
|---------------------------------------|
Notes:
      Uses common 10-way/18-way Mahjong pinout
      TC524256 - Toshiba TC524256BZ-80 256k x4 Dual Port VRAM (ZIP28)
      CY7C185  - Cypress CY7C185-20PC 8k x8 SRAM (DIP28)
      6264     - UT6264PC-70LL 8k x8 SRAM (DIP28)
      IGS011   - Custom IGS IC (QFP160)
      AR17961  - == OkiM6295 (QFP44)

      ROMs -
            MAJ-H_V027H- Main Program (27C4096)
            IGS_M0201  - Graphics (16M maskROM)
            IGS_M0202  - OKI samples (4M maskROM)
            MAJ-H_CG   - Graphics (27c4001 EPROM)

***************************************************************************/

ROM_START( dbc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "maj-h_v027h.u30", 0x00000, 0x80000, CRC(5d5ccd5b) SHA1(7a1223923f9a5825fd919ae9a36912284e705382) )

	ROM_REGION( 0x280000, "igs011", 0 )
	ROM_LOAD( "igs_m0201.u15", 0x000000, 0x200000, CRC(ec54452c) SHA1(0ee7ffa3d4845af083944e64faf5a1c78247aaa2) )
	ROM_LOAD( "maj-h_cg.u8",   0x200000, 0x080000, CRC(ee45cc46) SHA1(ed011f758a02026222994aaea0677a4e9580fbda) )   // 1xxxxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "igs_m0202.u39", 0x00000, 0x80000, CRC(106ac5f7) SHA1(5796a880c3424e3d2251b2223a0e594957afecaf) ) // 2 banks
ROM_END

/***************************************************************************

龍虎榜/Mahjong Ryukobou
Alta, 1995

PCB Layout
----------

IGS PCB NO-T0094
|------------------------------------|
|UPC1242  VOL  LM7805  DSW5          |
|   MAJ-J_SP           DSW4          |
|  TD62003     M6295   DSW3 CY7C185  |
|        8255          DSW2 CY7C185  |
|M                     DSW1 MAJ-J_CG |
|A                                   |
|H              IGS011      IGS_M0201|
|J                               PAL |
|O                                   |
|N  MAJ_V030J     22MHz              |
|G    6264              TC524256     |
|     6264              TC524256     |
|     PAL               TC524256     |
|     PAL      68000P10 TC524256     |
|     PAL                            |
|------------------------------------|
Notes:
      68000 - clock 7.33333MHz [22/3]
      M6295 - clock 1.04762MHz [22/21]
      VSync - 60.0078Hz
      HSync - 15.620kHz

***************************************************************************/

ROM_START( ryukobou )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "maj_v034j.u30", 0x000000, 0x80000, CRC(2ead9803) SHA1(159cc95df6cf409bd4c137de7ac876e5bc967c62) )

	ROM_REGION( 0x280000, "igs011", 0 )
	ROM_LOAD( "igs_m0201.u15", 0x000000, 0x200000, CRC(ec54452c) SHA1(0ee7ffa3d4845af083944e64faf5a1c78247aaa2) )
	ROM_LOAD( "maj-j_cg.u8",   0x200000, 0x080000, CRC(3c8de5d1) SHA1(51e43bfe7b5157112ded9a34b7032f73b7ffad11) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "maj-j_sp.u39", 0x000000, 0x80000, CRC(82f670f3) SHA1(d10dd67e40aee0e1c4f4b2c1217ce0935cb86406) )
ROM_END

ROM_START( ryukobouv30 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "maj_v030j.u30", 0x000000, 0x80000, CRC(186f2b4e) SHA1(380a3d94722d9f5fa5ec206ed0af6dbb8dd81715) )

	ROM_REGION( 0x280000, "igs011", 0 )
	ROM_LOAD( "igs_m0201.u15", 0x000000, 0x200000, CRC(ec54452c) SHA1(0ee7ffa3d4845af083944e64faf5a1c78247aaa2) )
	ROM_LOAD( "maj-j_cg.u8",   0x200000, 0x080000, CRC(3c8de5d1) SHA1(51e43bfe7b5157112ded9a34b7032f73b7ffad11) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "maj-j_sp.u39", 0x000000, 0x80000, CRC(82f670f3) SHA1(d10dd67e40aee0e1c4f4b2c1217ce0935cb86406) )
ROM_END

/***************************************************************************

龍虎榜/Lung4 Fu2 Bong2 II
IGS, 1996

PCB Layout
----------

IGS PCB NO-0115
|---------------------------------------------|
|                  M6295  IGSS0503.U38        |
|  UM3567  3.57945MHz                         |
|                          DSW3               |
|                          DSW2     PAL       |
| IGSM0502.U5              DSW1    6264       |
| IGSM0501.U7     PAL              6264       |
|                 PAL                         |
|                 PAL            IGS011       |
|                 PAL                         |
|                 PAL                         |
|                                             |
|   MC68HC000P10          22MHz  TC524258AZ-10|
|           6264         8255    TC524258AZ-10|
|    BATT   6264   MAJ2V185H.U29 TC524258AZ-10|
|                                TC524258AZ-10|
|---------------------------------------------|

Notes:
        68k clock: 7.3333MHz (i.e. 22/3)
      M6295 clock: 1.0476MHz (i.e. 22/21) \
         M6295 SS: HIGH                   / Therefore sampling freq = 7.936363636kHz (i.e. 1047600 / 132)
           UM3567: Compatible with YM2413, clock = 3.57945MHz
            HSync: 15.78kHz
            VSync: 60Hz

***************************************************************************/

ROM_START( lhb2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "maj2v185h.u29", 0x00000, 0x80000, CRC(2572d59a) SHA1(1d5362e209dadf8b21c10d1351d4bb038bfcaaef) )

	ROM_REGION( 0x200000, "igs011", 0 )
	ROM_LOAD( "igsm0501.u7", 0x00000, 0x200000, CRC(1c952bd6) SHA1(a6b6f1cdfb29647e81c032ffe59c94f1a10ceaf8) )

	ROM_REGION( 0x80000, "igs011:gfx_hi", 0 ) // high order bit of graphics (5th bit)
	/* these are identical ..seems OK as igs number is same, only ic changed */
	ROM_LOAD( "igsm0502.u4", 0x00000, 0x80000, CRC(5d73ae99) SHA1(7283aa3d6b15ceb95db80756892be46eb997ef15) )
	ROM_LOAD( "igsm0502.u5", 0x00000, 0x80000, CRC(5d73ae99) SHA1(7283aa3d6b15ceb95db80756892be46eb997ef15) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "igss0503.u38", 0x00000, 0x80000, CRC(c9609c9c) SHA1(f036e682b792033409966e84292a69275eaa05e5) )  // 2 banks
ROM_END

ROM_START( lhb2cpgs ) // 龍虎榜II 搓牌高手 (Lóng Hǔ Bǎng II Cuō Pái Gāoshǒu)
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "v127c.u29", 0x00000, 0x80000, CRC(d6025580) SHA1(1aa4e248380d1e70ef18d81073b5ab578d848f89) )

	ROM_REGION( 0x200000, "igs011", 0 )
	ROM_LOAD( "igsm0501.u7", 0x00000, 0x200000, CRC(1c952bd6) SHA1(a6b6f1cdfb29647e81c032ffe59c94f1a10ceaf8) )

	ROM_REGION( 0x80000, "igs011:gfx_hi", 0 )
	ROM_LOAD( "igsm0502.u4", 0x00000, 0x80000, CRC(5d73ae99) SHA1(7283aa3d6b15ceb95db80756892be46eb997ef15) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "igss0503.u38", 0x00000, 0x80000, CRC(c9609c9c) SHA1(f036e682b792033409966e84292a69275eaa05e5) )  // 2 banks
ROM_END

ROM_START( lhb3 ) // 龍虎榜III 搓牌高手 (Lóng Hǔ Bǎng III Cuō Pái Gāoshǒu) very similar to lhb2's, but not original?
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "rom.u29", 0x00000, 0x80000, CRC(c5985452) SHA1(f1f0c2b1b8c509b2a0a72a4f3387eccb0f25008a) )

	ROM_REGION( 0x200000, "igs011", 0 )
	ROM_LOAD( "rom.u7", 0x00000, 0x200000, CRC(1c952bd6) SHA1(a6b6f1cdfb29647e81c032ffe59c94f1a10ceaf8) )

	ROM_REGION( 0x80000, "igs011:gfx_hi", 0 )
	ROM_LOAD( "rom.u6", 0x00000, 0x80000, CRC(5d73ae99) SHA1(7283aa3d6b15ceb95db80756892be46eb997ef15) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "rom.u38", 0x00000, 0x80000, CRC(c9609c9c) SHA1(f036e682b792033409966e84292a69275eaa05e5) )  // 2 banks
ROM_END

/***************************************************************************

Mahjong Nenrikishu SP (V250J)
IGS/Alta, 1998

PCB Layout
----------

IGS PCB NO-0115-5
|----------------------------------|
|UPC1242H VOL M6295     SP         |
|   U3567 LM7805                   |
|   CG   3.579545MHz   DSW3        |
|  M0502               DSW2   PAL  |
|M                     DSW1  6264  |
|A M0501                     6264  |
|H              PAL        IGS011  |
|J              PAL                |
|O              PAL                |
|N              PAL                |
|G                                 |
|                   22MHz M5M442256|
|     68000P16            M5M442256|
|RESET    6264     8255   M5M442256|
| BATT    6264  V250J     M5M442256|
|----------------------------------|
Notes:
      68000 - Clock 7.3333MHz [22/2]
      U3567 - YM2413, clock 3.579545MHz
      M6295 - Clock 1.0476 [22/21]. pin 7 high
      VSync - 60.0052Hz
      HSync - 15.620kHz

***************************************************************************/

ROM_START( nkishusp )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "v250j.u29", 0x00000, 0x80000, CRC(500cb919) SHA1(76eed80b59c43e8cc1e258056cfe08b33a651852) )

	ROM_REGION( 0x800000, "igs011", ROMREGION_ERASEFF )
	ROM_LOAD( "m0501.u7", 0x000000, 0x200000, CRC(1c952bd6) SHA1(a6b6f1cdfb29647e81c032ffe59c94f1a10ceaf8) ) // Identical to igsm0501.u7
	// empty
	ROM_LOAD( "cg.u4",    0x400000, 0x080000, CRC(fe60f485) SHA1(d75e5f7a187161137a7f7b54d495d1cb3e1802a4) )
	// empty

	ROM_REGION( 0x200000, "igs011:gfx_hi", ROMREGION_ERASEFF ) // high order bit of graphics (5th bit)
	ROM_LOAD( "m0502.u6", 0x00000, 0x80000, CRC(5d73ae99) SHA1(7283aa3d6b15ceb95db80756892be46eb997ef15) ) // Identical to igsm0502.u4, igsm0502.u5
	// empty

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "sp.u38", 0x00000, 0x80000, CRC(d80e28e2) SHA1(c03441686e770227db6a2a41922fbb4284710571) )
ROM_END

/*********************************************************************************

Te Yi Gong Neng, IGS 1996

This is the joystick version of LHB2

PCB Layout
----------

IGS PCB NO- 0127-2
|-----------------------------------------|
|UPC1242          U3587   K668   S0503.U42|
|                     3.579545MHz         |
|                            SW1          |
|   M0502.U3                 SW2          |
|                            SW3          |
|                            SW4          |
|                                   6164  |
|J                                        |
|A  M0501.U7             PAL        6164  |
|M                       PAL              |
|M                       PAL              |
|A                       PAL      IGS011  |
|                        PAL              |
|                                         |
|     68000            22MHz   KM428C256  |
|           27C4096.U13                   |
|    T518B  IGS003A     6164   KM428C256  |
| SW5  BATTERY          6164              |
|-----------------------------------------|
Notes:
      68000 - Clock 7.33333MHz [22/3]
       K668 - Oki M6295 clone. Clock 1.047619048MHz [22/21]
      U3587 - This is a YM2412 clone. Clock input 3.579545MHz
  SW1/2/3/4 - 8-Position DIP Switch. SW3 and SW4 not populated
        SW5 - Reset / NVRAM Clear
       6164 - 8kBx8-bit SRAM
  KM428C256 - Dual-Port Video RAM with 256kBx8-bit DRAM and 512x8 SAM (Serial Access Memory).
              This is the same type of RAM used on Midway T-Unit hardware but 2X larger.

*********************************************************************************/

ROM_START( tygn ) // GFX ROMs same as lhb2 and nkishusp, sound ROM same as lhb2
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "27c4096.u13", 0x00000, 0x80000, CRC(25b1de1e) SHA1(7c3d0458b30db614e551fdf0db3f93fa44fcb758) )

	ROM_REGION( 0x800000, "igs011", ROMREGION_ERASEFF )
	ROM_LOAD( "m0501.u7", 0x000000, 0x200000, CRC(1c952bd6) SHA1(a6b6f1cdfb29647e81c032ffe59c94f1a10ceaf8) )

	ROM_REGION( 0x200000, "igs011:gfx_hi", ROMREGION_ERASEFF ) // high order bit of graphics (5th bit)
	ROM_LOAD( "m0502.u3", 0x00000, 0x80000, CRC(5d73ae99) SHA1(7283aa3d6b15ceb95db80756892be46eb997ef15) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "s0502.u42", 0x00000, 0x80000, CRC(c9609c9c) SHA1(f036e682b792033409966e84292a69275eaa05e5) )
ROM_END


/***************************************************************************

Virtua Bowling by IGS

PCB # 0101

U45  (27c240) is probably program
next to 68000 processor
U68,U69 probably images   (27c800 - mask)
U67, U66 sound and ????  (27c040)

ASIC chip used

SMD - custom chip IGS 011      F5XD  174
SMD - custom --near sound section - unknown -- i.d. rubbed off
SMD - custom  -- near inputs and 68000  IGS012    9441EK001

XTL near sound 33.868mhz
XTL near 68000  22.0000mhz

there are 4 banks of 8 dip switches

***************************************************************************/

ROM_START( vbowl )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "bowlingv101xcm.u45", 0x00000, 0x80000, BAD_DUMP CRC(ab8e3f1f) SHA1(69159e22559d6a26fe2afafd770aa640c192ba4b) )

	ROM_REGION( 0x400000 * 2, "igs011", 0)
	ROM_LOAD( "vrbowlng.u69", 0x000000, 0x400000, CRC(b0d339e8) SHA1(a26a5e0202a78e8cdc562b10d64e14eadfa4e115) )
	// extra space to expand every 4 bits to 8

	ROM_REGION( 0x100000, "igs011:gfx_hi", ROMREGION_INVERT )
	ROM_LOAD( "vrbowlng.u68", 0x000000, 0x100000, CRC(b0ce27e7) SHA1(6d3ef97edd606f384b1e05b152fbea12714887b7) )

	ROM_REGION( 0x400000, "ics", 0 )
	ROM_LOAD( "vrbowlng.u67", 0x00000, 0x80000, CRC(53000936) SHA1(e50c6216f559a9248c095bdfae05c3be4be79ff3) )  // 8 bit signed mono & u-law
	ROM_LOAD( "vrbowlng.u66", 0x80000, 0x80000, CRC(f62cf8ed) SHA1(c53e47e2c619ed974ad40ee4aaa4a35147ea8311) )  // 8 bit signed mono
	ROM_COPY( "ics", 0x000000, 0x100000,0x100000)
	ROM_COPY( "ics", 0x000000, 0x200000,0x100000)
	ROM_COPY( "ics", 0x000000, 0x300000,0x100000)
ROM_END

ROM_START( vbowlj )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "vrbowlng.u45", 0x00000, 0x80000, CRC(091c19c1) SHA1(5a7bfbee357122e9061b38dfe988c3853b0984b0) ) // second half all 00

	ROM_REGION( 0x400000 * 2, "igs011", 0)
	ROM_LOAD( "vrbowlng.u69", 0x000000, 0x400000, CRC(b0d339e8) SHA1(a26a5e0202a78e8cdc562b10d64e14eadfa4e115) )
	// extra space to expand every 4 bits to 8

	ROM_REGION( 0x100000, "igs011:gfx_hi", ROMREGION_INVERT )
	ROM_LOAD( "vrbowlng.u68", 0x000000, 0x100000, CRC(b0ce27e7) SHA1(6d3ef97edd606f384b1e05b152fbea12714887b7) )

	ROM_REGION( 0x400000, "ics", 0 )
	ROM_LOAD( "vrbowlng.u67", 0x00000, 0x80000, CRC(53000936) SHA1(e50c6216f559a9248c095bdfae05c3be4be79ff3) )  // 8 bit signed mono & u-law
	ROM_LOAD( "vrbowlng.u66", 0x80000, 0x80000, CRC(f62cf8ed) SHA1(c53e47e2c619ed974ad40ee4aaa4a35147ea8311) )  // 8 bit signed mono
	ROM_COPY( "ics", 0x000000, 0x100000,0x100000)
	ROM_COPY( "ics", 0x000000, 0x200000,0x100000)
	ROM_COPY( "ics", 0x000000, 0x300000,0x100000)
ROM_END

ROM_START( vbowlhk )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "bowlingv101hjs.u45", 0x00000, 0x80000, CRC(92fbfa72) SHA1(824b666ea2247ef5a974675925bcef94dbd1a992) ) // second half all 00

	ROM_REGION( 0x400000 * 2, "igs011", 0)
	ROM_LOAD( "vrbowlng.u69", 0x000000, 0x400000, CRC(b0d339e8) SHA1(a26a5e0202a78e8cdc562b10d64e14eadfa4e115) )
	// extra space to expand every 4 bits to 8

	ROM_REGION( 0x100000, "igs011:gfx_hi", ROMREGION_INVERT )
	ROM_LOAD( "vrbowlng.u68", 0x000000, 0x100000, CRC(b0ce27e7) SHA1(6d3ef97edd606f384b1e05b152fbea12714887b7) )

	ROM_REGION( 0x400000, "ics", 0 )
	ROM_LOAD( "vrbowlng.u67", 0x00000, 0x80000, CRC(53000936) SHA1(e50c6216f559a9248c095bdfae05c3be4be79ff3) )  // 8 bit signed mono & u-law
	ROM_LOAD( "vrbowlng.u66", 0x80000, 0x80000, CRC(f62cf8ed) SHA1(c53e47e2c619ed974ad40ee4aaa4a35147ea8311) )  // 8 bit signed mono
	ROM_COPY( "ics", 0x000000, 0x100000,0x100000)
	ROM_COPY( "ics", 0x000000, 0x200000,0x100000)
	ROM_COPY( "ics", 0x000000, 0x300000,0x100000)
ROM_END

/***************************************************************************

    幸運滿貫 (Xìngyùn Mǎn Guàn)

    Other files in the zip:

    14.484 U33-82E6.jed
    14.484 U34-1.jed
    14.484 U35-7068.jed

***************************************************************************/

ROM_START( xymg )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "u30-ebac.rom", 0x00000, 0x80000, CRC(7d272b6f) SHA1(15fd1be23cabdc77b747541f5cd9fed6b08be4ad) )

	ROM_REGION( 0x280000, "igs011", 0 )
	ROM_LOAD( "m0201-ig.160", 0x000000, 0x200000, CRC(ec54452c) SHA1(0ee7ffa3d4845af083944e64faf5a1c78247aaa2) )
	ROM_LOAD( "ygxy-u8.rom",  0x200000, 0x080000, CRC(56a2706f) SHA1(98bf4b3153eef53dd449e2538b4b7ff2cc2fe6fa) )

	ROM_REGION( 0x80000, "oki", 0 )
	// identical to 040-c3c2.snd
	ROM_LOAD( "m0202.snd", 0x00000, 0x80000, CRC(220949aa) SHA1(1e0dba168a0687d32aaaed42714ae24358f4a3e7) ) // 2 banks
	ROM_CONTINUE(          0x00000, 0x80000 ) // 1ST+2ND IDENTICAL
ROM_END

// this is very similar to the ryukobou PCB type. Below the Oki there's an empty space for an IGS003. Only SW1 and SW2 are populated.
ROM_START( xymga )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "rom.u30", 0x000000, 0x80000, BAD_DUMP CRC(ecc871fb) SHA1(b5a0e5ef9e6097548c5b26a4638b8618900a37ff) )
	// HACK: the sum of the program ROM long words that the game checks doesn't match the one stored at 7da4 (bad dump?).
	// Replace the stored sum (encrypted) with the one the game computes in MAME, which includes the protection reads.
	ROM_FILL( 0x7da4, 1, 0x6b )
	ROM_FILL( 0x7da5, 1, 0x2f )
	ROM_FILL( 0x7da6, 1, 0x7d )
	ROM_FILL( 0x7da7, 1, 0xd2 )

	ROM_REGION( 0x280000, "igs011", 0 )
	ROM_LOAD( "rom.u15",     0x000000, 0x200000, CRC(ec54452c) SHA1(0ee7ffa3d4845af083944e64faf5a1c78247aaa2) )
	ROM_LOAD( "igs_0203.u8", 0x200000, 0x080000, CRC(56a2706f) SHA1(98bf4b3153eef53dd449e2538b4b7ff2cc2fe6fa) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "igs_s0202.u39", 0x000000, 0x80000, CRC(106ac5f7) SHA1(5796a880c3424e3d2251b2223a0e594957afecaf) ) // same as xymg, only without 1st and 2nd half identical
ROM_END

ROM_START( sdmg2641c ) // IGS PCB N0-0093-8
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "igs_m2403.u30", 0x00000, 0x80000, CRC(e5f197fe) SHA1(d17bf7f37251083645cf235e148cb819d1bd995d) )

	ROM_REGION( 0x280000, "igs011", 0 )
	ROM_LOAD( "rom.u15",      0x000000, 0x200000, CRC(ec54452c) SHA1(0ee7ffa3d4845af083944e64faf5a1c78247aaa2) )
	ROM_LOAD( "igs_l2404.u8", 0x200000, 0x080000, CRC(47dd4df5) SHA1(74e2fdb6a7052290b631fc13d31bc03261410e84) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "igs_s0202.u39", 0x000000, 0x80000, CRC(106ac5f7) SHA1(5796a880c3424e3d2251b2223a0e594957afecaf) ) // not dumped, but ROM code matches xymga's
ROM_END


} // anonymous namespace


/***************************************************************************

    Game Drivers

***************************************************************************/

GAME( 1997, drgnwrld,      0,        drgnwrld,        drgnwrld,  igs011_oki_state, init_drgnwrld,     ROT0, "IGS",                     "Dragon World (World, V040O)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1995, drgnwrldv40k,  drgnwrld, drgnwrld_igs012, drgnwrldc, igs011_oki_state, init_drgnwrldv40k, ROT0, "IGS",                     "Dongbang Jiju (Korea, V040K)",                     MACHINE_SUPPORTS_SAVE )
GAME( 1995, drgnwrldv30,   drgnwrld, drgnwrld,        drgnwrld,  igs011_oki_state, init_drgnwrldv30,  ROT0, "IGS",                     "Dragon World (World, V030O)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1995, drgnwrldv21,   drgnwrld, drgnwrld_igs012, drgnwrld,  igs011_oki_state, init_drgnwrldv21,  ROT0, "IGS",                     "Dragon World (World, V021O)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1995, drgnwrldv21j,  drgnwrld, drgnwrld_igs012, drgnwrldj, igs011_oki_state, init_drgnwrldv21j, ROT0, "IGS / Alta",              "Chuugokuryuu (Japan, V021J)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1995, drgnwrldv20j,  drgnwrld, drgnwrld_igs012, drgnwrldj, igs011_oki_state, init_drgnwrldv20j, ROT0, "IGS / Alta",              "Chuugokuryuu (Japan, V020J)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1995, drgnwrldv11h,  drgnwrld, drgnwrld,        drgnwrldc, igs011_oki_state, init_drgnwrldv11h, ROT0, "IGS",                     "Dung Fong Zi Zyu (Hong Kong, V011H, set 1)",       MACHINE_SUPPORTS_SAVE )
GAME( 1995, drgnwrldv11ha, drgnwrld, drgnwrld_igs012, drgnwrldc, igs011_oki_state, init_drgnwrldv40k, ROT0, "IGS",                     "Dung Fong Zi Zyu (Hong Kong, V011H, set 2)",       MACHINE_SUPPORTS_SAVE ) // different encryption and with IGS012
GAME( 1995, drgnwrldv10c,  drgnwrld, drgnwrld,        drgnwrldc, igs011_oki_state, init_drgnwrldv10c, ROT0, "IGS",                     "Zhongguo Long (China, V010C)",                     MACHINE_SUPPORTS_SAVE )
GAME( 1995, lhb,           0,        lhb,             lhb,       igs011_oki_state, init_lhb,          ROT0, "IGS",                     "Long Hu Bang (China, V035C)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1995, lhbv33c,       lhb,      lhb,             lhb,       igs011_oki_state, init_lhbv33c,      ROT0, "IGS",                     "Long Hu Bang (China, V033C)",                      MACHINE_SUPPORTS_SAVE )
GAME( 1995, dbc,           lhb,      lhb,             lhb,       igs011_oki_state, init_dbc,          ROT0, "IGS",                     "Daai Baan Sing (Hong Kong, V027H)",                MACHINE_SUPPORTS_SAVE )
GAME( 1995, ryukobou,      lhb,      lhb,             lhb,       igs011_oki_state, init_ryukobou,     ROT0, "IGS / Alta",              "Mahjong Ryukobou (Japan, V034J)",                  MACHINE_SUPPORTS_SAVE )
GAME( 1995, ryukobouv30,   lhb,      lhb,             lhb,       igs011_oki_state, init_ryukobou,     ROT0, "IGS / Alta",              "Mahjong Ryukobou (Japan, V030J)",                  MACHINE_SUPPORTS_SAVE )
GAME( 1996, lhb2,          0,        lhb2,            lhb2,      igs011_oki_state, init_lhb2,         ROT0, "IGS",                     "Lung Fu Bong II (Hong Kong, V185H)",               MACHINE_SUPPORTS_SAVE )
GAME( 1996, lhb2cpgs,      lhb2,     lhb2cpgs,        lhb2cpgs,  igs011_oki_state, init_lhb2cpgs,     ROT0, "IGS",                     "Long Hu Bang II: Cuo Pai Gaoshou (China, V127C)",  MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1996, tygn,          lhb2,     tygn,            tygn,      igs011_serial_state, init_tygn,      ROT0, "IGS",                     "Te Yi Gong Neng (China, V632C)",                   MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1996, lhb3,          lhb2,     lhb3,            lhb2,      igs011_serial_state, init_lhb3,      ROT0, "IGS",                     "Long Hu Bang III: Cuo Pai Gaoshou (China, V242C)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
GAME( 1996, xymg,          0,        xymg,            xymg,      igs011_oki_state, init_xymg,         ROT0, "IGS",                     "Xingyun Manguan (China, V651C, set 1)",            MACHINE_SUPPORTS_SAVE )
GAME( 1996, xymga,         xymg,     xymga,           xymg,      igs011_oki_state, init_xymga,        ROT0, "IGS",                     "Xingyun Manguan (China, V651C, set 2)",            MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING ) // different encryption and without IGS003
GAME( 1996, wlcc,          xymg,     wlcc,            wlcc,      igs011_oki_state, init_wlcc,         ROT0, "IGS",                     "Wanli Changcheng (China, V638C)",                  MACHINE_SUPPORTS_SAVE )
GAME( 1996, vbowl,         0,        vbowl,           vbowl,     vbowl_state,      init_vbowl,        ROT0, "IGS",                     "Virtua Bowling (World, V101XCM)",                  MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1996, vbowlj,        vbowl,    vbowl,           vbowlj,    vbowl_state,      init_vbowlj,       ROT0, "IGS / Alta",              "Virtua Bowling (Japan, V100JCM)",                  MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1996, vbowlhk,       vbowl,    vbowlhk,         vbowlhk,   vbowl_state,      init_vbowlhk,      ROT0, "IGS / Tai Tin Amusement", "Virtua Bowling (Hong Kong, V101HJS)",              MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND )
GAME( 1997, sdmg2641c,     sdmg2,    sdmg2641c,       xymg,      igs011_oki_state, init_sdmg2641c,    ROT0, "IGS",                     "Chaoji Da Manguan II (China, V641C)",              MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING ) // inputs / outputs / protection need verifying
GAME( 1998, nkishusp,      lhb2,     nkishusp,        nkishusp,  igs011_serial_state, init_nkishusp,  ROT0, "IGS / Alta",              "Mahjong Nenrikishu SP (Japan, V250J)",             MACHINE_SUPPORTS_SAVE )
