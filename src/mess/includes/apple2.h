// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    includes/apple2.h

    Include file to handle emulation of the Apple II series.

***************************************************************************/

#ifndef APPLE2_H_
#define APPLE2_H_

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
	UINT32 read_mem;
	read8_delegate *read_handler;
	UINT32 write_mem;
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
	UINT8 *auxmem;
	UINT32 auxmem_length;
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

	UINT32 m_flags, m_flags_mask;
	INT32 m_a2_cnxx_slot;
	UINT32 m_a2_mask;
	UINT32 m_a2_set;
	int m_a2_speaker_state;
	double m_joystick_x1_time;
	double m_joystick_y1_time;
	double m_joystick_x2_time;
	double m_joystick_y2_time;
	apple2_memmap_config m_mem_config;
	apple2_meminfo *m_current_meminfo;
	int m_fdc_diskreg;
	const UINT8 *m_a2_videoram, *m_a2_videoaux, *m_textgfx_data;
	UINT32 m_a2_videomask, m_textgfx_datalen;
	UINT32 m_old_a2;
	int m_fgcolor;
	int m_bgcolor;
	int m_flash;
	int m_alt_charset_value;
	UINT16 *m_hires_artifact_map;
	UINT16 *m_dhires_artifact_map;
	bool m_monochrome_dhr;
	int m_inh_slot;
	int m_reset_flag;

	UINT8 *m_rambase;

	UINT8 *m_rom, *m_slot_ram;
	UINT32 m_rom_length, m_slot_length;

	machine_type_t m_machinetype;

	device_a2eauxslot_card_interface *m_auxslotdevice;

	UINT16 m_lastchar, m_strobe;
	UINT8 m_transchar;

	READ8_MEMBER(apple2_c0xx_r);
	WRITE8_MEMBER(apple2_c0xx_w);
	READ8_MEMBER(apple2_c080_r);
	WRITE8_MEMBER(apple2_c080_w);

	READ8_MEMBER ( apple2_c00x_r );
	READ8_MEMBER ( apple2_c01x_r );
	READ8_MEMBER ( apple2_c02x_r );
	READ8_MEMBER ( apple2_c03x_r );
	READ8_MEMBER ( apple2_c05x_r );
	READ8_MEMBER ( apple2_c06x_r );
	READ8_MEMBER ( apple2_c07x_r );
	WRITE8_MEMBER ( apple2_c00x_w );
	WRITE8_MEMBER ( apple2_c01x_w );
	WRITE8_MEMBER ( apple2_c02x_w );
	WRITE8_MEMBER ( apple2_c03x_w );
	WRITE8_MEMBER ( apple2_c05x_w );
	WRITE8_MEMBER ( apple2_c07x_w );

	READ8_MEMBER ( apple2_mainram0000_r );
	READ8_MEMBER ( apple2_mainram0200_r );
	READ8_MEMBER ( apple2_mainram0400_r );
	READ8_MEMBER ( apple2_mainram0800_r );
	READ8_MEMBER ( apple2_mainram2000_r );
	READ8_MEMBER ( apple2_mainram4000_r );
	READ8_MEMBER ( apple2_mainramc000_r );
	READ8_MEMBER ( apple2_mainramd000_r );
	READ8_MEMBER ( apple2_mainrame000_r );
	READ8_MEMBER ( apple2_auxram0000_r );
	READ8_MEMBER ( apple2_auxram0200_r );
	READ8_MEMBER ( apple2_auxram0400_r );
	READ8_MEMBER ( apple2_auxram0800_r );
	READ8_MEMBER ( apple2_auxram2000_r );
	READ8_MEMBER ( apple2_auxram4000_r );
	READ8_MEMBER ( apple2_auxramc000_r );
	READ8_MEMBER ( apple2_auxramd000_r );
	READ8_MEMBER ( apple2_auxrame000_r );

	WRITE8_MEMBER ( apple2_mainram0000_w );
	WRITE8_MEMBER ( apple2_mainram0200_w );
	WRITE8_MEMBER ( apple2_mainram0400_w );
	WRITE8_MEMBER ( apple2_mainram0800_w );
	WRITE8_MEMBER ( apple2_mainram2000_w );
	WRITE8_MEMBER ( apple2_mainram4000_w );
	WRITE8_MEMBER ( apple2_mainramc000_w );
	WRITE8_MEMBER ( apple2_mainramd000_w );
	WRITE8_MEMBER ( apple2_mainrame000_w );
	WRITE8_MEMBER ( apple2_auxram0000_w );
	WRITE8_MEMBER ( apple2_auxram0200_w );
	WRITE8_MEMBER ( apple2_auxram0400_w );
	WRITE8_MEMBER ( apple2_auxram0800_w );
	WRITE8_MEMBER ( apple2_auxram2000_w );
	WRITE8_MEMBER ( apple2_auxram4000_w );
	WRITE8_MEMBER ( apple2_auxramc000_w );
	WRITE8_MEMBER ( apple2_auxramd000_w );
	WRITE8_MEMBER ( apple2_auxrame000_w );

	READ8_MEMBER ( apple2_c1xx_r );
	WRITE8_MEMBER ( apple2_c1xx_w );
	READ8_MEMBER ( apple2_c3xx_r );
	WRITE8_MEMBER ( apple2_c3xx_w );
	READ8_MEMBER ( apple2_c4xx_r );
	WRITE8_MEMBER ( apple2_c4xx_w );

	READ8_MEMBER ( apple2_c800_r );
	WRITE8_MEMBER ( apple2_c800_w );
	READ8_MEMBER ( apple2_ce00_r );
	WRITE8_MEMBER ( apple2_ce00_w );

	READ8_MEMBER ( apple2_inh_d000_r );
	WRITE8_MEMBER ( apple2_inh_d000_w );
	READ8_MEMBER ( apple2_inh_e000_r );
	WRITE8_MEMBER ( apple2_inh_e000_w );

	READ8_MEMBER(read_floatingbus);

	READ8_MEMBER(apple2_cfff_r);
	WRITE8_MEMBER(apple2_cfff_w);

	void apple2_refresh_delegates();
	int apple2_pressed_specialkey(UINT8 key);

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
	DECLARE_MACHINE_START(apple2orig);
	DECLARE_MACHINE_START(apple2e);
	DECLARE_MACHINE_START(apple2c);
	DECLARE_MACHINE_START(apple2cp);
	DECLARE_MACHINE_START(tk2000);
	DECLARE_MACHINE_START(tk3000);
	DECLARE_MACHINE_START(laser128);
	DECLARE_MACHINE_START(space84);
	DECLARE_MACHINE_START(laba2p);
	DECLARE_VIDEO_START(apple2);
	DECLARE_PALETTE_INIT(apple2);
	DECLARE_VIDEO_START(apple2p);
	DECLARE_VIDEO_START(apple2e);
	DECLARE_VIDEO_START(apple2c);
	UINT32 screen_update_apple2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(apple2_interrupt);
	DECLARE_WRITE_LINE_MEMBER(a2bus_irq_w);
	DECLARE_WRITE_LINE_MEMBER(a2bus_nmi_w);
	DECLARE_WRITE_LINE_MEMBER(a2bus_inh_w);
	DECLARE_READ_LINE_MEMBER(ay3600_shift_r);
	DECLARE_READ_LINE_MEMBER(ay3600_control_r);
	DECLARE_WRITE_LINE_MEMBER(ay3600_data_ready_w);
	DECLARE_WRITE_LINE_MEMBER(ay3600_iie_data_ready_w);
	void apple2_update_memory_postload();
	virtual void machine_reset();
	void apple2_setup_memory(const apple2_memmap_config *config);
	void apple2_update_memory();
	inline UINT32 effective_a2();
	UINT32 compute_video_address(int col, int row);
	void adjust_begin_and_end_row(const rectangle &cliprect, int *beginrow, int *endrow);
	inline void apple2_plot_text_character(bitmap_ind16 &bitmap, int xpos, int ypos, int xscale, UINT32 code,
		const UINT8 *textgfx_data, UINT32 textgfx_datalen, UINT32 my_a2);
	void apple2_text_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int page, int beginrow, int endrow);
	void apple2_lores_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int page, int beginrow, int endrow);
	void apple2_hires_draw(bitmap_ind16 &bitmap, const rectangle &cliprect, int page, int beginrow, int endrow);
	void apple2_video_start(const UINT8 *vram, const UINT8 *aux_vram, UINT32 ignored_softswitches, int hires_modulo);
	void apple2_setvar(UINT32 val, UINT32 mask);
	UINT8 apple2_getfloatingbusvalue();
	int apple2_fdc_has_35();
	int apple2_fdc_has_525();
	void apple2_iwm_setdiskreg(UINT8 data);
	void apple2_init_common();
	void apple2eplus_init_common(void *apple2cp_ce00_ram);
	INT8 apple2_slotram_r(address_space &space, int slotnum, int offset);
	int a2_no_ctrl_reset();

private:
	// Laser 128EX2 slot 5 Apple Memory Expansion emulation vars
	UINT8 m_exp_bankhior;
	int m_exp_addrmask;
	UINT8 m_exp_regs[0x10];
	UINT8 *m_exp_ram;
	int m_exp_wptr, m_exp_liveptr;
};
/*----------- defined in drivers/apple2.c -----------*/
INPUT_PORTS_EXTERN( apple2ep );
/*----------- defined in machine/apple2.c -----------*/
extern const applefdc_interface apple2_fdc_interface;

#endif /* APPLE2_H_ */
