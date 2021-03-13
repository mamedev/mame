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
#include "video/jag_blitter.h"
#include "cdrom.h"
#include "imagedev/chd_cd.h"
#include "screen.h"
#include "emupal.h"

#ifndef ENABLE_SPEEDUP_HACKS
#define ENABLE_SPEEDUP_HACKS 1
#endif

/* CoJag and Jaguar have completely different XTALs, pixel clock in Jaguar is the same as the GPU one */
#define COJAG_PIXEL_CLOCK       XTAL(14'318'181)
#define JAGUAR_CLOCK            XTAL(26'590'906) // NTSC
// XTAL(26'593'900) PAL, TODO

class jaguar_state : public driver_device
{
public:
	jaguar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gpu(*this, "gpu")
		, m_blitter(*this, "blitter")
		, m_dsp(*this, "dsp")
		, m_ldac(*this, "ldac")
		, m_rdac(*this, "rdac")
		, m_nvram(*this, "nvram")
		, m_rom_base(*this, "mainrom")
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
		, m_joystick_data(0)
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
		, m_eeprom_bit_count(0)
		, m_protection_check(0)
		, m_eeprom(*this, "eeprom")
		, m_ide(*this, "ide")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
	{
	}

	void cojag68k(machine_config &config);
	void cojagr3k(machine_config &config);
	void cojagr3k_rom(machine_config &config);
	void jaguar(machine_config &config);

	void init_jaguar();
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

protected:
	void console_base_map(address_map &map);
	void console_base_gpu_map(address_map &map);

	// device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void sound_start() override;
	virtual void video_start() override;
	virtual void device_postload();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void video_config(machine_config &config, const XTAL clock);

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<jaguargpu_cpu_device> m_gpu;
	required_device<jag_blitter_device> m_blitter;
	required_device<jaguardsp_cpu_device> m_dsp;
	required_device<dac_word_interface> m_ldac;
	required_device<dac_word_interface> m_rdac;

	// memory
	optional_shared_ptr<uint32_t> m_nvram;        // not used on console
	optional_region_ptr<uint16_t> m_rom_base;
	optional_region_ptr<uint32_t> m_cart_base;    // not used in cojag
	required_shared_ptr<uint32_t> m_dsp_ram;
	required_region_ptr<uint16_t> m_wave_rom;
	required_shared_ptr<uint32_t> m_shared_ram;
	required_shared_ptr<uint32_t> m_gpu_ram;
	required_shared_ptr<uint32_t> m_gpu_clut;
	optional_memory_region        m_romboard_region;
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

	uint32_t m_joystick_data;

private:
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

	static void (jaguar_state::*const bitmap4[8])(uint16_t *, int32_t, int32_t, uint32_t *, int32_t, uint16_t *);
	static void (jaguar_state::*const bitmap8[8])(uint16_t *, int32_t, int32_t, uint32_t *, int32_t, uint16_t *);
	static void (jaguar_state::*const bitmap16[8])(uint16_t *, int32_t, int32_t, uint32_t *, int32_t);
	static void (jaguar_state::*const bitmap32[8])(uint16_t *, int32_t, int32_t, uint32_t *, int32_t);

	void eeprom_w(uint32_t data);
	uint32_t eeprom_clk();
	uint32_t eeprom_cs();
	uint32_t misc_control_r();
	void misc_control_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t gpuctrl_r(offs_t offset, uint32_t mem_mask = ~0);
	void gpuctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t dspctrl_r(offs_t offset, uint32_t mem_mask = ~0);
	void dspctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t joystick_r();
	void joystick_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void latch_w(uint32_t data);
	uint32_t eeprom_data_r(offs_t offset);
	void eeprom_enable_w(uint32_t data);
	void eeprom_data_w(offs_t offset, uint32_t data);
	void gpu_jump_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t gpu_jump_r();
	uint32_t cojagr3k_main_speedup_r();
	uint32_t main_gpu_wait_r();
	void area51_main_speedup_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void area51mx_main_speedup_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint16_t gpuctrl_r16(offs_t offset, uint16_t mem_mask = ~0);
	void gpuctrl_w16(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t blitter_r16(offs_t offset, uint16_t mem_mask = ~0);
	void blitter_w16(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t serial_r16(offs_t offset);
	void serial_w16(offs_t offset, uint16_t data);
	uint16_t dspctrl_r16(offs_t offset, uint16_t mem_mask = ~0);
	void dspctrl_w16(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t eeprom_cs16(offs_t offset);
	uint16_t eeprom_clk16(offs_t offset);
	void eeprom_w16(offs_t offset, uint16_t data);
	uint16_t joystick_r16(offs_t offset);
	void joystick_w16(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t shared_ram_r(offs_t offset);
	void shared_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t rom_base_r(offs_t offset);
	uint32_t wave_rom_r(offs_t offset);
	uint32_t dsp_ram_r(offs_t offset);
	void dsp_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t gpu_clut_r(offs_t offset);
	void gpu_clut_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t gpu_ram_r(offs_t offset);
	void gpu_ram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint16_t shared_ram_r16(offs_t offset);
	void shared_ram_w16(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t cart_base_r16(offs_t offset);
	uint16_t dsp_ram_r16(offs_t offset);
	void dsp_ram_w16(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t gpu_clut_r16(offs_t offset);
	void gpu_clut_w16(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t gpu_ram_r16(offs_t offset);
	void gpu_ram_w16(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// from audio/jaguar.cpp
	uint16_t jerry_regs_r(offs_t offset);
	void jerry_regs_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t serial_r(offs_t offset);
	void serial_w(offs_t offset, uint32_t data);
	void serial_update();

	// from video/jaguar.cpp
	uint32_t blitter_r(offs_t offset, uint32_t mem_mask = ~0);
	void blitter_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint16_t tom_regs_r(offs_t offset);
	void tom_regs_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t cojag_gun_input_r(offs_t offset);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void jagpal_ycc(palette_device &palette) const;

	DECLARE_WRITE_LINE_MEMBER( gpu_cpu_int );
	DECLARE_WRITE_LINE_MEMBER( dsp_cpu_int );
	DECLARE_WRITE_LINE_MEMBER( external_int );

	image_init_result quickload_cb(device_image_interface &image);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( cart_load );
	void cpu_space_map(address_map &map);
	void dsp_map(address_map &map);
	void dsp_rom_map(address_map &map);
	void gpu_map(address_map &map);
	void gpu_rom_map(address_map &map);
	void jag_gpu_dsp_map(address_map &map);
	void jaguar_map(address_map &map);
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

	void gpu_suspend() { m_gpu->suspend(SUSPEND_REASON_SPIN, 1); }
	void gpu_resume() { m_gpu->resume(SUSPEND_REASON_SPIN); }
	void dsp_suspend() { m_dsp->suspend(SUSPEND_REASON_SPIN, 1); }
	void dsp_resume() { m_dsp->resume(SUSPEND_REASON_SPIN); }

	void fix_endian( void *base, uint32_t size );
	void cojag_common_init(uint16_t gpu_jump_offs, uint16_t spin_pc);
	void init_freeze_common(offs_t main_speedup_addr);

	// from audio/jaguar.cpp
	void update_gpu_irq();
	void dsp_flags_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	// from video/jaguar.cpp
	void get_crosshair_xy(int player, int &x, int &y);
	int effective_hvalue(int value);
	bool adjust_object_timer(int vc);
	inline void trigger_host_cpu_irq(int level);
	inline void verify_host_cpu_irq();
	uint8_t *memory_base(uint32_t offset) { return reinterpret_cast<uint8_t *>(m_gpu->space(AS_PROGRAM).get_read_ptr(offset)); }
	void blitter_run();
	void scanline_update(int param);
	void set_palette(uint16_t vmode);

	/* from jagobj.cpp */
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

	/* from jagblit.cpp */
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
	required_device<palette_device> m_palette;
};

class jaguarcd_state : public jaguar_state
{
public:
	jaguarcd_state(const machine_config &mconfig, device_type type, const char *tag)
		: jaguar_state(mconfig, type, tag)
		, m_cdrom(*this, "cdrom")
		, m_cd_bios(*this, "cdbios")
	{
	}

	void jaguarcd(machine_config &config);

	void init_jaguarcd();

protected:
	virtual void machine_reset() override;

private:
	uint16_t butch_regs_r16(offs_t offset);
	void butch_regs_w16(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint32_t butch_regs_r(offs_t offset);
	void butch_regs_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t cd_bios_r(offs_t offset);

	void jaguarcd_map(address_map &map);
	void jagcd_gpu_dsp_map(address_map &map);

	// devices
	required_device<cdrom_image_device> m_cdrom;
	required_region_ptr<uint16_t> m_cd_bios;

	uint32_t m_butch_regs[0x40/4];
	uint32_t m_butch_cmd_response[0x102];
	uint8_t m_butch_cmd_index;
	uint8_t m_butch_cmd_size;

	cdrom_file  *m_cd_file;
	//const cdrom_toc*    m_toc;
};
