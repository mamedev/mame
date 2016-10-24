// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Nicola Salmoria
/*************************************************************************

    Art & Magic hardware

**************************************************************************/
#include "sound/okim6295.h"
#include "cpu/tms34010/tms34010.h"

class artmagic_state : public driver_device
{
public:
	enum
	{
		TIMER_IRQ_OFF
	};

	artmagic_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki"),
		m_oki_region(*this, "oki"),
		m_tms(*this, "tms"),
		m_tlc34076(*this, "tlc34076"),
		m_control(*this, "control"),
		m_vram0(*this, "vram0"),
		m_vram1(*this, "vram1") { }

	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;
	required_memory_region m_oki_region;
	required_device<tms34010_device> m_tms;
	required_device<tlc34076_device> m_tlc34076;

	required_shared_ptr<uint16_t> m_control;
	required_shared_ptr<uint16_t> m_vram0;
	required_shared_ptr<uint16_t> m_vram1;

	uint8_t m_tms_irq;
	uint8_t m_hack_irq;
	uint8_t m_prot_input[16];
	uint8_t m_prot_input_index;
	uint8_t m_prot_output[16];
	uint8_t m_prot_output_index;
	uint8_t m_prot_output_bit;
	uint8_t m_prot_bit_index;
	uint16_t m_prot_save;
	typedef void (artmagic_state::*prot_func)();
	prot_func m_protection_handler;
	void ultennis_protection();
	void cheesech_protection();
	void stonebal_protection();

	int m_xor[16];
	int m_is_stoneball;
	uint16_t *m_blitter_base;
	uint32_t m_blitter_mask;
	uint16_t m_blitter_data[8];
	uint8_t m_blitter_page;
	attotime m_blitter_busy_until;
	void control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t ultennis_hack_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void protection_bit_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t shtstar_unk_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t shtstar_unk_2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t artmagic_blitter_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void artmagic_blitter_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void m68k_gen_int(int state);
	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg);
	TMS340X0_SCANLINE_RGB32_CB_MEMBER(scanline);
	ioport_value prot_r(ioport_field &field, void *param);
	void init_shtstar();
	void init_cheesech();
	void init_ultennis();
	void init_stonebal();
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void decrypt_cheesech();
	void decrypt_ultennis();
	void execute_blit();
	void update_irq_state();
	inline uint16_t *address_to_vram(offs_t *address);

protected:
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
};
