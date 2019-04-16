// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Nathan Woods,Angelo Salese, Robbbert
/*************************************************************************

    Atari Jaguar hardware

*************************************************************************/

#include "cpu/jaguar/jaguar.h"
#include "machine/nvram.h"
#include "sound/dac.h"
#include "machine/eepromser.h"
#include "machine/vt83c461.h"
#include "imagedev/snapquik.h"
#include "cdrom.h"
#include "imagedev/chd_cd.h"
#include "screen.h"

#ifndef ENABLE_SPEEDUP_HACKS
#define ENABLE_SPEEDUP_HACKS 1
#endif

/* CoJag and Jaguar have completely different XTALs, pixel clock in Jaguar is the same as the GPU one */
#define COJAG_PIXEL_CLOCK       XTAL(14'318'181)
#define JAGUAR_CLOCK            XTAL(25'590'906) // NTSC
// XTAL(25'593'900) PAL, TODO

class jaguar_state : public driver_device
{
public:
	jaguar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gpu(*this, "gpu")
		, m_dsp(*this, "dsp")
		, m_ldac(*this, "ldac")
		, m_rdac(*this, "rdac")
		, m_cdrom(*this, "cdrom")
		, m_nvram(*this, "nvram")
		, m_rom_base(*this, "rom")
		, m_cart_base(*this, "cart")
		, m_dsp_ram(*this, "dspram")
		, m_wave_rom(*this, "waverom")
		, m_shared_ram(*this, "sharedram")
		, m_gpu_ram(*this, "gpuram")
		, m_gpu_clut(*this, "gpuclut")
		, m_romboard_region(*this, "romboard")
		, m_mainram(*this, "mainram")
		, m_mainram2(*this, "mainram2")
		, m_maingfxbank(*this, "maingfxbank")
		, m_gpugfxbank(*this, "gpugfxbank")
		, m_mainsndbank(*this, "mainsndbank")
		, m_dspsndbank(*this, "dspsndbank")
		, m_config_io(*this, "CONFIG")
		, m_joy(*this, "JOY%u", 0U)
		, m_buttons(*this, "BUTTONS%u", 0U)
		, m_system(*this, "SYSTEM")
		, m_is_r3000(false)
		, m_is_cojag(false)
		, m_hacks_enabled(false)
		, m_using_cart(false)
		, m_misc_control_data(0)
		, m_eeprom_enable(true)
		, m_gpu_jump_address(nullptr)
		, m_gpu_command_pending(false)
		, m_gpu_spin_pc(0)
		, m_main_speedup(nullptr)
		, m_main_speedup_hits(0)
		, m_main_speedup_last_cycles(0)
		, m_main_speedup_max_cycles(0)
		, m_main_gpu_wait(nullptr)
		, m_joystick_data(0)
		, m_eeprom_bit_count(0)
		, m_protection_check(0)
		, m_eeprom(*this, "eeprom")
		, m_ide(*this, "ide")
		, m_screen(*this, "screen")
	{
	}

	void cojag68k(machine_config &config);
	void cojagr3k(machine_config &config);
	void cojagr3k_rom(machine_config &config);
	void jaguarcd(machine_config &config);
	void jaguar(machine_config &config);

	void init_jaguar();
	void init_jaguarcd();
	void init_area51mx();
	void init_maxforce();
	void init_freezeat();
	void init_fishfren();
	void init_a51mxr3k();
	void init_area51();
	void init_freezeat4();
	void init_freezeat5();
	void init_freezeat6();
	void init_vcircle();
	void init_freezeat3();
	void init_freezeat2();
	void init_area51a();

private:
	// devices
	required_device<cpu_device> m_maincpu;
	required_device<jaguargpu_cpu_device> m_gpu;
	required_device<jaguardsp_cpu_device> m_dsp;
	required_device<dac_word_interface> m_ldac;
	required_device<dac_word_interface> m_rdac;
	optional_device<cdrom_image_device> m_cdrom;

	// memory
	optional_shared_ptr<uint32_t> m_nvram;        // not used on console
	required_shared_ptr<uint32_t> m_rom_base;
	optional_shared_ptr<uint32_t> m_cart_base;    // not used in cojag
	required_shared_ptr<uint32_t> m_dsp_ram;
	required_shared_ptr<uint32_t> m_wave_rom;
	required_shared_ptr<uint32_t> m_shared_ram;
	required_shared_ptr<uint32_t> m_gpu_ram;
	required_shared_ptr<uint32_t> m_gpu_clut;
	optional_memory_region      m_romboard_region;
	optional_shared_ptr<uint32_t> m_mainram;
	optional_shared_ptr<uint32_t> m_mainram2;

	optional_memory_bank m_maingfxbank;
	optional_memory_bank m_gpugfxbank;
	optional_memory_bank m_mainsndbank;
	optional_memory_bank m_dspsndbank;
	optional_ioport m_config_io;
	optional_ioport_array<8> m_joy;
	optional_ioport_array<8> m_buttons;
	optional_ioport m_system;

	// configuration
	bool m_is_r3000;
	bool m_is_cojag;
	bool m_hacks_enabled;
	int m_pixel_clock;
	bool m_using_cart;
	bool m_is_jagcd;

	uint32_t m_misc_control_data;
	bool m_eeprom_enable;
	uint32_t *m_gpu_jump_address;
	bool m_gpu_command_pending;
	uint32_t m_gpu_spin_pc;
	uint32_t *m_main_speedup;
	int m_main_speedup_hits;
	uint64_t m_main_speedup_last_cycles;
	uint64_t m_main_speedup_max_cycles;
	uint32_t *m_main_gpu_wait;

	// driver data
	uint32_t m_joystick_data;
	uint8_t m_eeprom_bit_count;
	uint8_t m_protection_check;   /* 0 = check hasn't started yet; 1= check in progress; 2 = check is finished. */

	// audio data
	uint16_t m_dsp_regs[0x40/2];
	uint16_t m_serial_frequency;
	uint8_t m_gpu_irq_state;
	emu_timer *m_serial_timer;

	// blitter variables
	uint32_t m_blitter_regs[40];
	uint16_t m_gpu_regs[0x100/2];
	emu_timer *m_object_timer;
	uint8_t m_cpu_irq_state;
	bitmap_rgb32 m_screen_bitmap;
	uint8_t m_blitter_status;
	pen_t m_pen_table[65536];
	uint8_t m_blend_y[65536];
	uint8_t m_blend_cc[65536];
	uint32_t m_butch_regs[0x40/4];
	uint32_t m_butch_cmd_response[0x102];
	uint8_t m_butch_cmd_index;
	uint8_t m_butch_cmd_size;
	cdrom_file  *m_cd_file;
	const cdrom_toc*    m_toc;

	static void (jaguar_state::*const bitmap4[8])(uint16_t *, int32_t, int32_t, uint32_t *, int32_t, uint16_t *);
	static void (jaguar_state::*const bitmap8[8])(uint16_t *, int32_t, int32_t, uint32_t *, int32_t, uint16_t *);
	static void (jaguar_state::*const bitmap16[8])(uint16_t *, int32_t, int32_t, uint32_t *, int32_t);
	static void (jaguar_state::*const bitmap32[8])(uint16_t *, int32_t, int32_t, uint32_t *, int32_t);

	DECLARE_WRITE32_MEMBER(eeprom_w);
	DECLARE_READ32_MEMBER(eeprom_clk);
	DECLARE_READ32_MEMBER(eeprom_cs);
	DECLARE_READ32_MEMBER(misc_control_r);
	DECLARE_WRITE32_MEMBER(misc_control_w);
	DECLARE_READ32_MEMBER(gpuctrl_r);
	DECLARE_WRITE32_MEMBER(gpuctrl_w);
	DECLARE_READ32_MEMBER(dspctrl_r);
	DECLARE_WRITE32_MEMBER(dspctrl_w);
	DECLARE_READ32_MEMBER(joystick_r);
	DECLARE_WRITE32_MEMBER(joystick_w);
	DECLARE_WRITE32_MEMBER(latch_w);
	DECLARE_READ32_MEMBER(eeprom_data_r);
	DECLARE_WRITE32_MEMBER(eeprom_enable_w);
	DECLARE_WRITE32_MEMBER(eeprom_data_w);
	DECLARE_WRITE32_MEMBER(gpu_jump_w);
	DECLARE_READ32_MEMBER(gpu_jump_r);
	DECLARE_READ32_MEMBER(cojagr3k_main_speedup_r);
	DECLARE_READ32_MEMBER(main_gpu_wait_r);
	DECLARE_WRITE32_MEMBER(area51_main_speedup_w);
	DECLARE_WRITE32_MEMBER(area51mx_main_speedup_w);
	DECLARE_READ16_MEMBER(gpuctrl_r16);
	DECLARE_WRITE16_MEMBER(gpuctrl_w16);
	DECLARE_READ16_MEMBER(blitter_r16);
	DECLARE_WRITE16_MEMBER(blitter_w16);
	DECLARE_READ16_MEMBER(serial_r16);
	DECLARE_WRITE16_MEMBER(serial_w16);
	DECLARE_READ16_MEMBER(dspctrl_r16);
	DECLARE_WRITE16_MEMBER(dspctrl_w16);
	DECLARE_READ16_MEMBER(eeprom_cs16);
	DECLARE_READ16_MEMBER(eeprom_clk16);
	DECLARE_WRITE16_MEMBER(eeprom_w16);
	DECLARE_READ16_MEMBER(joystick_r16);
	DECLARE_WRITE16_MEMBER(joystick_w16);
	DECLARE_READ32_MEMBER(shared_ram_r);
	DECLARE_WRITE32_MEMBER(shared_ram_w);
	DECLARE_READ32_MEMBER(rom_base_r);
	DECLARE_WRITE32_MEMBER(rom_base_w);
	DECLARE_READ32_MEMBER(cart_base_r);
	DECLARE_WRITE32_MEMBER(cart_base_w);
	DECLARE_READ32_MEMBER(wave_rom_r);
	DECLARE_WRITE32_MEMBER(wave_rom_w);
	DECLARE_READ32_MEMBER(dsp_ram_r);
	DECLARE_WRITE32_MEMBER(dsp_ram_w);
	DECLARE_READ32_MEMBER(gpu_clut_r);
	DECLARE_WRITE32_MEMBER(gpu_clut_w);
	DECLARE_READ32_MEMBER(gpu_ram_r);
	DECLARE_WRITE32_MEMBER(gpu_ram_w);
	DECLARE_READ16_MEMBER(shared_ram_r16);
	DECLARE_WRITE16_MEMBER(shared_ram_w16);
	DECLARE_READ16_MEMBER(rom_base_r16);
	DECLARE_WRITE16_MEMBER(rom_base_w16);
	DECLARE_READ16_MEMBER(cart_base_r16);
	DECLARE_WRITE16_MEMBER(cart_base_w16);
	DECLARE_READ16_MEMBER(wave_rom_r16);
	DECLARE_WRITE16_MEMBER(wave_rom_w16);
	DECLARE_READ16_MEMBER(dsp_ram_r16);
	DECLARE_WRITE16_MEMBER(dsp_ram_w16);
	DECLARE_READ16_MEMBER(gpu_clut_r16);
	DECLARE_WRITE16_MEMBER(gpu_clut_w16);
	DECLARE_READ16_MEMBER(gpu_ram_r16);
	DECLARE_WRITE16_MEMBER(gpu_ram_w16);
	DECLARE_READ16_MEMBER(butch_regs_r16);
	DECLARE_WRITE16_MEMBER(butch_regs_w16);
	DECLARE_READ32_MEMBER(butch_regs_r);
	DECLARE_WRITE32_MEMBER(butch_regs_w);

	// from audio/jaguar.c
	DECLARE_READ16_MEMBER( jerry_regs_r );
	DECLARE_WRITE16_MEMBER( jerry_regs_w );
	DECLARE_READ32_MEMBER( serial_r );
	DECLARE_WRITE32_MEMBER( serial_w );
	void serial_update();

	// from video/jaguar.c
	DECLARE_READ32_MEMBER( blitter_r );
	DECLARE_WRITE32_MEMBER( blitter_w );
	DECLARE_READ16_MEMBER( tom_regs_r );
	DECLARE_WRITE16_MEMBER( tom_regs_w );
	DECLARE_READ32_MEMBER( cojag_gun_input_r );
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE_LINE_MEMBER( gpu_cpu_int );
	DECLARE_WRITE_LINE_MEMBER( dsp_cpu_int );
	DECLARE_WRITE_LINE_MEMBER( external_int );

	image_init_result quickload(device_image_interface &image, const char *file_type, int quickload_size);
	void cart_start();
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( jaguar_cart );
	DECLARE_QUICKLOAD_LOAD_MEMBER( jaguar );
	void cpu_space_map(address_map &map);
	void dsp_map(address_map &map);
	void dsp_rom_map(address_map &map);
	void gpu_map(address_map &map);
	void gpu_rom_map(address_map &map);
	void jag_dsp_map(address_map &map);
	void jag_gpu_map(address_map &map);
	void jagcd_dsp_map(address_map &map);
	void jagcd_gpu_map(address_map &map);
	void jaguar_map(address_map &map);
	void jaguarcd_map(address_map &map);
	void m68020_map(address_map &map);
	void r3000_map(address_map &map);
	void r3000_rom_map(address_map &map);

	// timer IDs
	enum
	{
		TID_SCANLINE,
		TID_BLITTER_DONE,
		TID_PIT,
		TID_SERIAL,
		TID_GPU_SYNC
	};

	// device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void sound_start() override;
	virtual void video_start() override;
	virtual void device_postload();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void gpu_suspend() { m_gpu->suspend(SUSPEND_REASON_SPIN, 1); }
	void gpu_resume() { m_gpu->resume(SUSPEND_REASON_SPIN); }
	void dsp_suspend() { m_dsp->suspend(SUSPEND_REASON_SPIN, 1); }
	void dsp_resume() { m_dsp->resume(SUSPEND_REASON_SPIN); }

	void fix_endian( uint32_t addr, uint32_t size );
	void cojag_common_init(uint16_t gpu_jump_offs, uint16_t spin_pc);
	void init_freeze_common(offs_t main_speedup_addr);

	// from audio/jaguar.c
	void update_gpu_irq();
	DECLARE_WRITE32_MEMBER( dsp_flags_w );

	// from video/jaguar.c
	void get_crosshair_xy(int player, int &x, int &y);
	int effective_hvalue(int value);
	bool adjust_object_timer(int vc);
	void update_cpu_irq();
	uint8_t *memory_base(uint32_t offset) { return reinterpret_cast<uint8_t *>(m_gpu->space(AS_PROGRAM).get_read_ptr(offset)); }
	void blitter_run();
	void scanline_update(int param);
	void set_palette(uint16_t vmode);

	/* from jagobj.c */
	void jagobj_init();
	uint32_t *process_bitmap(uint16_t *scanline, uint32_t *objdata, int vc, bool logit);
	uint32_t *process_scaled_bitmap(uint16_t *scanline, uint32_t *objdata, int vc, bool logit);
	uint32_t *process_branch(uint32_t *objdata, int vc, bool logit);
	void process_object_list(int vc, uint16_t *_scanline);
	void bitmap_4_draw(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint8_t flags, int32_t dxpos, uint16_t *clutbase);
	void bitmap_4_0(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase);
	void bitmap_4_1(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase);
	void bitmap_4_2(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase);
	void bitmap_4_3(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase);
	void bitmap_4_4(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase);
	void bitmap_4_5(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase);
	void bitmap_4_6(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase);
	void bitmap_4_7(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase);
	void bitmap_8_draw(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint8_t flags, int32_t dxpos, uint16_t *clutbase);
	void bitmap_8_0(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase);
	void bitmap_8_1(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase);
	void bitmap_8_2(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase);
	void bitmap_8_3(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase);
	void bitmap_8_4(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase);
	void bitmap_8_5(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase);
	void bitmap_8_6(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase);
	void bitmap_8_7(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint16_t *clutbase);
	void bitmap_16_draw(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint8_t flags, int32_t dxpos);
	void bitmap_16_0(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos);
	void bitmap_16_1(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos);
	void bitmap_16_2(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos);
	void bitmap_16_3(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos);
	void bitmap_16_4(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos);
	void bitmap_16_5(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos);
	void bitmap_16_6(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos);
	void bitmap_16_7(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos);
	void bitmap_32_draw(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos, uint8_t flags, int32_t dxpos);
	void bitmap_32_0(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos);
	void bitmap_32_1(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos);
	void bitmap_32_2(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos);
	void bitmap_32_3(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos);
	void bitmap_32_4(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos);
	void bitmap_32_5(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos);
	void bitmap_32_6(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos);
	void bitmap_32_7(uint16_t *scanline, int32_t firstpix, int32_t iwidth, uint32_t *src, int32_t xpos);

	/* from jagblit.c */
	void generic_blitter(uint32_t command, uint32_t a1flags, uint32_t a2flags);
	void blitter_09800001_010020_010020(uint32_t command, uint32_t a1flags, uint32_t a2flags);
	void blitter_09800009_000020_000020(uint32_t command, uint32_t a1flags, uint32_t a2flags);
	void blitter_01800009_000028_000028(uint32_t command, uint32_t a1flags, uint32_t a2flags);
	void blitter_01800001_000018_000018(uint32_t command, uint32_t a1flags, uint32_t a2flags);
	void blitter_01c00001_000018_000018(uint32_t command, uint32_t a1flags, uint32_t a2flags);
	void blitter_00010000_xxxxxx_xxxxxx(uint32_t command, uint32_t a1flags, uint32_t a2flags);
	void blitter_01800001_xxxxxx_xxxxxx(uint32_t command, uint32_t a1flags, uint32_t a2flags);
	void blitter_x1800x01_xxxxxx_xxxxxx(uint32_t command, uint32_t a1flags, uint32_t a2flags);

	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<vt83c461_device> m_ide;
	required_device<screen_device> m_screen;
};
