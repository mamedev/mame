// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    includes/apple2.h

    Include file to handle emulation of the Apple II series.

***************************************************************************/

#ifndef APPLE2_H_
#define APPLE2_H_

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "cpu/m6502/m65c02.h"
#include "bus/a2bus/a2bus.h"
#include "bus/a2bus/a2eauxslot.h"
#include "machine/applefdc.h"
#include "machine/ram.h"
#include "imagedev/cassette.h"
#include "machine/kb3600.h"
#include "sound/speaker.h"
#include "machine/ram.h"
#include "bus/rs232/rs232.h"
#include "machine/mos6551.h"

#define AUXSLOT_TAG "auxbus"

#define IIC_ACIA1_TAG "acia1"
#define IIC_ACIA2_TAG "acia2"
#define IICP_IWM_TAG    "iwm"

#define LASER128_UDC_TAG "l128udc"

#define PRINTER_PORT_TAG "printer"
#define MODEM_PORT_TAG "modem"

/***************************************************************************
    SOFTSWITCH VALUES
***************************************************************************/

#define VAR_80STORE     0x000001
#define VAR_RAMRD       0x000002
#define VAR_RAMWRT      0x000004
#define VAR_INTCXROM    0x000008
#define VAR_ALTZP       0x000010
#define VAR_SLOTC3ROM   0x000020
#define VAR_80COL       0x000040
#define VAR_ALTCHARSET  0x000080
#define VAR_TEXT        0x000100
#define VAR_MIXED       0x000200
#define VAR_PAGE2       0x000400
#define VAR_HIRES       0x000800
#define VAR_AN0         0x001000
#define VAR_AN1         0x002000
#define VAR_AN2         0x004000
#define VAR_AN3         0x008000
#define VAR_LCRAM       0x010000
#define VAR_LCRAM2      0x020000
#define VAR_LCWRITE     0x040000
#define VAR_ROMSWITCH   0x080000
#define VAR_TK2000RAM   0x100000        // ROM/RAM switch for TK2000

#define VAR_DHIRES      VAR_AN3

/***************************************************************************
    OTHER
***************************************************************************/

/* -----------------------------------------------------------------------
 * New Apple II memory manager
 * ----------------------------------------------------------------------- */

#define APPLE2_MEM_AUX      0x40000000
#define APPLE2_MEM_SLOT     0x80000000
#define APPLE2_MEM_ROM      0xC0000000
#define APPLE2_MEM_FLOATING 0xFFFFFFFF
#define APPLE2_MEM_MASK     0x00FFFFFF

enum machine_type_t
{
	APPLE_II,           // Apple II/II+
	APPLE_IIE,          // Apple IIe with aux slots
	APPLE_IIGS,         // Apple IIgs
	APPLE_IIC,          // Apple IIc
	APPLE_IICPLUS,      // Apple IIc+
	TK2000,             // Microdigital TK2000
	TK3000,             // Microdigital TK3000
	LASER128,           // Laser 128/128EX/128EX2
	SPACE84,            // "Space 84" with flipped text mode
	LABA2P              // lab equipment (?) II Plus with flipped text mode
};

enum bank_disposition_t
{
	A2MEM_IO        = 0,    /* this is always handlers; never banked memory */
	A2MEM_MONO      = 1,    /* this is a bank where read and write are always in unison */
	A2MEM_DUAL      = 2     /* this is a bank where read and write can go different places */
};

struct apple2_meminfo
{
	uint32_t read_mem;
	read8_delegate *read_handler;
	uint32_t write_mem;
	write8_delegate *write_handler;
};

struct apple2_memmap_entry
{
	offs_t begin;
	offs_t end;
	void (*get_meminfo)(running_machine &machine, offs_t begin, offs_t end, apple2_meminfo *meminfo);
	bank_disposition_t bank_disposition;
};

struct apple2_memmap_config
{
	int first_bank;
	uint8_t *auxmem;
	uint32_t auxmem_length;
	const apple2_memmap_entry *memmap;
};

class apple2_state : public driver_device
{
public:
	apple2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ram(*this, RAM_TAG),
		m_ay3600(*this, "ay3600"),
		m_a2bus(*this, "a2bus"),
		m_a2eauxslot(*this, AUXSLOT_TAG),
		m_joy1x(*this, "joystick_1_x"),
		m_joy1y(*this, "joystick_1_y"),
		m_joy2x(*this, "joystick_2_x"),
		m_joy2y(*this, "joystick_2_y"),
		m_joybuttons(*this, "joystick_buttons"),
		m_kbdrom(*this, "keyboard"),
		m_kbspecial(*this, "keyb_special"),
		m_kbrepeat(*this, "keyb_repeat"),
		m_resetdip(*this, "reset_dip"),
		m_sysconfig(*this, "a2_config"),
		m_cassette(*this, "cassette"),
		m_acia1(*this, IIC_ACIA1_TAG),
		m_acia2(*this, IIC_ACIA2_TAG),
		m_laserudc(*this, LASER128_UDC_TAG),
		m_iicpiwm(*this, IICP_IWM_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<ay3600_device> m_ay3600;
	required_device<a2bus_device> m_a2bus;
	optional_device<a2eauxslot_device> m_a2eauxslot;

	optional_ioport m_joy1x, m_joy1y, m_joy2x, m_joy2y, m_joybuttons;
	optional_memory_region m_kbdrom;
	required_ioport m_kbspecial;
	optional_ioport m_kbrepeat;
	optional_ioport m_resetdip;
	optional_ioport m_sysconfig;
	optional_device<cassette_image_device> m_cassette;

	optional_device<mos6551_device> m_acia1, m_acia2;
	optional_device<applefdc_base_device> m_laserudc;
	optional_device<iwm_device> m_iicpiwm;

	uint32_t m_flags, m_flags_mask;
	int32_t m_a2_cnxx_slot;
	uint32_t m_a2_mask;
	uint32_t m_a2_set;
	int m_a2_speaker_state;
	double m_joystick_x1_time;
	double m_joystick_y1_time;
	double m_joystick_x2_time;
	double m_joystick_y2_time;
	apple2_memmap_config m_mem_config;
	std::unique_ptr<apple2_meminfo[]> m_current_meminfo;
	int m_fdc_diskreg;
	const uint8_t *m_a2_videoram, *m_a2_videoaux, *m_textgfx_data;
	uint32_t m_a2_videomask, m_textgfx_datalen;
	uint32_t m_old_a2;
	int m_fgcolor;
	int m_bgcolor;
	int m_flash;
	int m_alt_charset_value;
	std::unique_ptr<uint16_t[]> m_hires_artifact_map;
	std::unique_ptr<uint16_t[]> m_dhires_artifact_map;
	bool m_monochrome_dhr;
	int m_inh_slot;
	int m_reset_flag;

	uint8_t *m_rambase;

	uint8_t *m_rom, *m_slot_ram;
	uint32_t m_rom_length, m_slot_length;

	machine_type_t m_machinetype;

	device_a2eauxslot_card_interface *m_auxslotdevice;

	uint16_t m_lastchar, m_strobe;
	uint8_t m_transchar;

	uint8_t apple2_c0xx_r(address_space &space, offs_t offset, uint8_t mem_mask);
	void apple2_c0xx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t apple2_c080_r(address_space &space, offs_t offset, uint8_t mem_mask);
	void apple2_c080_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);

	uint8_t apple2_c00x_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint8_t apple2_c01x_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint8_t apple2_c02x_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint8_t apple2_c03x_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint8_t apple2_c05x_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint8_t apple2_c06x_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint8_t apple2_c07x_r(address_space &space, offs_t offset, uint8_t mem_mask);
	void apple2_c00x_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void apple2_c01x_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void apple2_c02x_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void apple2_c03x_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void apple2_c05x_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void apple2_c07x_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);

	uint8_t apple2_mainram0000_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint8_t apple2_mainram0200_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint8_t apple2_mainram0400_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint8_t apple2_mainram0800_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint8_t apple2_mainram2000_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint8_t apple2_mainram4000_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint8_t apple2_mainramc000_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint8_t apple2_mainramd000_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint8_t apple2_mainrame000_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint8_t apple2_auxram0000_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint8_t apple2_auxram0200_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint8_t apple2_auxram0400_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint8_t apple2_auxram0800_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint8_t apple2_auxram2000_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint8_t apple2_auxram4000_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint8_t apple2_auxramc000_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint8_t apple2_auxramd000_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint8_t apple2_auxrame000_r(address_space &space, offs_t offset, uint8_t mem_mask);

	void apple2_mainram0000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void apple2_mainram0200_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void apple2_mainram0400_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void apple2_mainram0800_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void apple2_mainram2000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void apple2_mainram4000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void apple2_mainramc000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void apple2_mainramd000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void apple2_mainrame000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void apple2_auxram0000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void apple2_auxram0200_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void apple2_auxram0400_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void apple2_auxram0800_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void apple2_auxram2000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void apple2_auxram4000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void apple2_auxramc000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void apple2_auxramd000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	void apple2_auxrame000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);

	uint8_t apple2_c1xx_r(address_space &space, offs_t offset, uint8_t mem_mask);
	void apple2_c1xx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t apple2_c3xx_r(address_space &space, offs_t offset, uint8_t mem_mask);
	void apple2_c3xx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t apple2_c4xx_r(address_space &space, offs_t offset, uint8_t mem_mask);
	void apple2_c4xx_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);

	uint8_t apple2_c800_r(address_space &space, offs_t offset, uint8_t mem_mask);
	void apple2_c800_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t apple2_ce00_r(address_space &space, offs_t offset, uint8_t mem_mask);
	void apple2_ce00_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);

	uint8_t apple2_inh_d000_r(address_space &space, offs_t offset, uint8_t mem_mask);
	void apple2_inh_d000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t apple2_inh_e000_r(address_space &space, offs_t offset, uint8_t mem_mask);
	void apple2_inh_e000_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);

	uint8_t read_floatingbus(address_space &space, offs_t offset, uint8_t mem_mask);

	uint8_t apple2_cfff_r(address_space &space, offs_t offset, uint8_t mem_mask);
	void apple2_cfff_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);

	void apple2_refresh_delegates();
	int apple2_pressed_specialkey(uint8_t key);

	read8_delegate read_delegates_master[4];
	write8_delegate write_delegates_master[3];
	write8_delegate write_delegates_0000[2];
	write8_delegate write_delegates_0200[2];
	write8_delegate write_delegates_0400[2];
	write8_delegate write_delegates_0800[2];
	write8_delegate write_delegates_2000[2];
	write8_delegate write_delegates_4000[2];
	write8_delegate write_delegates_c000[2];
	write8_delegate write_delegates_d000[2];
	write8_delegate write_delegates_e000[2];
	read8_delegate read_delegates_0000[2];
	read8_delegate read_delegates_0200[2];
	read8_delegate read_delegates_0400[2];
	read8_delegate read_delegates_0800[2];
	read8_delegate read_delegates_2000[2];
	read8_delegate read_delegates_4000[2];
	read8_delegate read_delegates_c000[2];
	read8_delegate read_delegates_d000[2];
	read8_delegate read_delegates_e000[2];
	read8_delegate rd_c000;
	write8_delegate wd_c000;
	read8_delegate rd_c080;
	write8_delegate wd_c080;
	read8_delegate rd_cfff;
	write8_delegate wd_cfff;
	read8_delegate rd_c800;
	write8_delegate wd_c800;
	read8_delegate rd_ce00;
	write8_delegate wd_ce00;
	read8_delegate rd_inh_d000;
	write8_delegate wd_inh_d000;
	read8_delegate rd_inh_e000;
	write8_delegate wd_inh_e000;
	void machine_start_apple2orig();
	void machine_start_apple2e();
	void machine_start_apple2c();
	void machine_start_apple2cp();
	void machine_start_tk2000();
	void machine_start_tk3000();
	void machine_start_laser128();
	void machine_start_space84();
	void machine_start_laba2p();
	void video_start_apple2();
	void palette_init_apple2(palette_device &palette);
	void video_start_apple2p();
	void video_start_apple2e();
	void video_start_apple2c();
	uint32_t screen_update_apple2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void apple2_interrupt(timer_device &timer, void *ptr, int32_t param);
	void a2bus_irq_w(int state);
	void a2bus_nmi_w(int state);
	void a2bus_inh_w(int state);
	int ay3600_shift_r();
	int ay3600_control_r();
	void ay3600_data_ready_w(int state);
	void ay3600_iie_data_ready_w(int state);
	void apple2_update_memory_postload();
	virtual void machine_reset() override;
	void apple2_setup_memory(const apple2_memmap_config *config);
	void apple2_update_memory();
	inline uint32_t effective_a2();
	uint32_t compute_video_address(int col, int row);
	void adjust_begin_and_end_row(const rectangle &cliprect, int *beginrow, int *endrow);
	inline void apple2_plot_text_character(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, uint32_t code,
		const uint8_t *textgfx_data, uint32_t textgfx_datalen, uint32_t my_a2);
	void apple2_text_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int page, int beginrow, int endrow);
	void apple2_lores_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int page, int beginrow, int endrow);
	void apple2_hires_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int page, int beginrow, int endrow);
	void apple2_video_start(const uint8_t *vram, const uint8_t *aux_vram, uint32_t ignored_softswitches, int hires_modulo);
	void apple2_setvar(uint32_t val, uint32_t mask);
	uint8_t apple2_getfloatingbusvalue();
	int apple2_fdc_has_35();
	int apple2_fdc_has_525();
	void apple2_iwm_setdiskreg(uint8_t data);
	void apple2_init_common();
	void apple2eplus_init_common(void *apple2cp_ce00_ram);
	int8_t apple2_slotram_r(address_space &space, int slotnum, int offset);
	int a2_no_ctrl_reset();

private:
	// Laser 128EX2 slot 5 Apple Memory Expansion emulation vars
	uint8_t m_exp_bankhior;
	int m_exp_addrmask;
	uint8_t m_exp_regs[0x10];
	std::unique_ptr<uint8_t[]> m_exp_ram;
	int m_exp_wptr, m_exp_liveptr;
};
/*----------- defined in drivers/apple2.c -----------*/
INPUT_PORTS_EXTERN( apple2ep );
/*----------- defined in machine/apple2.c -----------*/
extern const applefdc_interface apple2_fdc_interface;

#endif /* APPLE2_H_ */
