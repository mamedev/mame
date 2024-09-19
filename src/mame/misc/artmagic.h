// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Nicola Salmoria
/*************************************************************************

    Art & Magic hardware

**************************************************************************/
#include "cpu/tms34010/tms34010.h"
#include "sound/okim6295.h"
#include "video/tlc34076.h"

class artmagic_state : public driver_device
{
public:
	artmagic_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_oki(*this, "oki")
		, m_tms(*this, "tms")
		, m_tlc34076(*this, "tlc34076")
		, m_control(*this, "control")
		, m_vram(*this, "vram%u", 0U)
		, m_blitter_base(*this, "gfx")
	{ }

	void init_shtstar();
	void init_cheesech();
	void init_ultennis();
	void init_stonebal();
	void cheesech(machine_config &config);
	void artmagic(machine_config &config);
	void shtstar(machine_config &config);
	void stonebal(machine_config &config);
	int prot_r();

private:
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_device<tms34010_device> m_tms;
	required_device<tlc34076_device> m_tlc34076;

	required_shared_ptr<uint16_t> m_control;
	required_shared_ptr_array<uint16_t, 2> m_vram;
	required_region_ptr<uint16_t> m_blitter_base;

	uint8_t m_tms_irq = 0U;
	uint8_t m_hack_irq = 0U;
	uint8_t m_prot_input[16]{};
	uint8_t m_prot_input_index = 0U;
	uint8_t m_prot_output[16]{};
	uint8_t m_prot_output_index = 0U;
	uint8_t m_prot_output_bit = 0U;
	uint8_t m_prot_bit_index = 0U;
	uint16_t m_prot_save = 0U;
	typedef void (artmagic_state::*prot_func)();
	prot_func m_protection_handler{};
	void ultennis_protection();
	void cheesech_protection();
	void stonebal_protection();

	int m_xor[16]{};
	int m_is_stoneball = 0;
	uint16_t m_blitter_data[8]{};
	uint8_t m_blitter_page = 0U;
	attotime m_blitter_busy_until{};
	emu_timer * m_irq_off_timer = nullptr;
	void control_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t ultennis_hack_r();
	void protection_bit_w(offs_t offset, uint16_t data);
	uint16_t blitter_r();
	void blitter_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void m68k_gen_int(int state);
	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg);
	TMS340X0_SCANLINE_RGB32_CB_MEMBER(scanline);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	void decrypt_cheesech();
	void decrypt_ultennis();
	void execute_blit();
	void update_irq_state();
	inline uint16_t *address_to_vram(offs_t *address);

	void main_map(address_map &map) ATTR_COLD;
	void shtstar_guncpu_io_map(address_map &map) ATTR_COLD;
	void shtstar_guncpu_map(address_map &map) ATTR_COLD;
	void shtstar_map(address_map &map) ATTR_COLD;
	void shtstar_subcpu_map(address_map &map) ATTR_COLD;
	void shtstar_subcpu_vector_map(address_map &map) ATTR_COLD;
	void stonebal_map(address_map &map) ATTR_COLD;
	void stonebal_tms_map(address_map &map) ATTR_COLD;
	void tms_map(address_map &map) ATTR_COLD;

protected:
	TIMER_CALLBACK_MEMBER(irq_off);
};
