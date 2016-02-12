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

#ifndef ENABLE_SPEEDUP_HACKS
#define ENABLE_SPEEDUP_HACKS 1
#endif

/* CoJag and Jaguar have completely different XTALs, pixel clock in Jaguar is the same as the GPU one */
#define COJAG_PIXEL_CLOCK       XTAL_14_31818MHz
#define JAGUAR_CLOCK            XTAL_25_590906MHz // NTSC
// XTAL_25_593900MHz PAL, TODO

class jaguar_state : public driver_device
{
public:
	jaguar_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_gpu(*this, "gpu"),
			m_dsp(*this, "dsp"),
			m_dac1(*this, "dac1"),
			m_dac2(*this, "dac2"),
			m_cdrom(*this, "cdrom"),
			m_nvram(*this, "nvram"),
			m_rom_base(*this, "rom"),
			m_cart_base(*this, "cart"),
			m_dsp_ram(*this, "dspram"),
			m_wave_rom(*this, "waverom"),
			m_shared_ram(*this, "sharedram"),
			m_gpu_ram(*this, "gpuram"),
			m_gpu_clut(*this, "gpuclut"),
			m_romboard_region(*this, "romboard"),
			m_is_r3000(false),
			m_is_cojag(false),
			m_hacks_enabled(false),
			m_using_cart(false),
			m_misc_control_data(0),
			m_eeprom_enable(true),
			m_gpu_jump_address(nullptr),
			m_gpu_command_pending(false),
			m_gpu_spin_pc(0),
			m_main_speedup(nullptr),
			m_main_speedup_hits(0),
			m_main_speedup_last_cycles(0),
			m_main_speedup_max_cycles(0),
			m_main_gpu_wait(nullptr),
			m_joystick_data(0),
			m_eeprom_bit_count(0),
			m_protection_check(0) ,
		m_eeprom(*this, "eeprom"),
		m_ide(*this, "ide"),
		m_screen(*this, "screen")
	{
	}

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<jaguargpu_cpu_device> m_gpu;
	required_device<jaguardsp_cpu_device> m_dsp;
	required_device<dac_device> m_dac1;
	required_device<dac_device> m_dac2;
	optional_device<cdrom_image_device> m_cdrom;

	// memory
	optional_shared_ptr<UINT32> m_nvram;        // not used on console
	required_shared_ptr<UINT32> m_rom_base;
	optional_shared_ptr<UINT32> m_cart_base;    // not used in cojag
	required_shared_ptr<UINT32> m_dsp_ram;
	required_shared_ptr<UINT32> m_wave_rom;
	required_shared_ptr<UINT32> m_shared_ram;
	required_shared_ptr<UINT32> m_gpu_ram;
	required_shared_ptr<UINT32> m_gpu_clut;
	optional_memory_region      m_romboard_region;

	// configuration
	bool m_is_r3000;
	bool m_is_cojag;
	bool m_hacks_enabled;
	int m_pixel_clock;
	bool m_using_cart;
	bool m_is_jagcd;

	UINT32 m_misc_control_data;
	bool m_eeprom_enable;
	UINT32 *m_gpu_jump_address;
	bool m_gpu_command_pending;
	UINT32 m_gpu_spin_pc;
	UINT32 *m_main_speedup;
	int m_main_speedup_hits;
	UINT64 m_main_speedup_last_cycles;
	UINT64 m_main_speedup_max_cycles;
	UINT32 *m_main_gpu_wait;

	// driver data
	UINT32 m_joystick_data;
	UINT8 m_eeprom_bit_count;
	UINT8 m_protection_check;   /* 0 = check hasn't started yet; 1= check in progress; 2 = check is finished. */

	// audio data
	UINT16 m_dsp_regs[0x40/2];
	UINT16 m_serial_frequency;
	UINT8 m_gpu_irq_state;
	emu_timer *m_serial_timer;

	// blitter variables
	UINT32 m_blitter_regs[40];
	UINT16 m_gpu_regs[0x100/2];
	emu_timer *m_object_timer;
	UINT8 m_cpu_irq_state;
	bitmap_rgb32 m_screen_bitmap;
	UINT8 m_blitter_status;
	pen_t m_pen_table[65536];
	UINT8 m_blend_y[65536];
	UINT8 m_blend_cc[65536];
	UINT32 m_butch_regs[0x40/4];
	UINT32 m_butch_cmd_response[0x102];
	UINT8 m_butch_cmd_index;
	UINT8 m_butch_cmd_size;
	cdrom_file  *m_cd_file;
	const cdrom_toc*    m_toc;

	static void (jaguar_state::*const bitmap4[8])(UINT16 *, INT32, INT32, UINT32 *, INT32, UINT16 *);
	static void (jaguar_state::*const bitmap8[8])(UINT16 *, INT32, INT32, UINT32 *, INT32, UINT16 *);
	static void (jaguar_state::*const bitmap16[8])(UINT16 *, INT32, INT32, UINT32 *, INT32);
	static void (jaguar_state::*const bitmap32[8])(UINT16 *, INT32, INT32, UINT32 *, INT32);

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
	DECLARE_DRIVER_INIT(jaguar);
	DECLARE_DRIVER_INIT(jaguarcd);
	DECLARE_DRIVER_INIT(area51mx);
	DECLARE_DRIVER_INIT(maxforce);
	DECLARE_DRIVER_INIT(freezeat);
	DECLARE_DRIVER_INIT(fishfren);
	DECLARE_DRIVER_INIT(a51mxr3k);
	DECLARE_DRIVER_INIT(area51);
	DECLARE_DRIVER_INIT(freezeat4);
	DECLARE_DRIVER_INIT(freezeat5);
	DECLARE_DRIVER_INIT(freezeat6);
	DECLARE_DRIVER_INIT(vcircle);
	DECLARE_DRIVER_INIT(freezeat3);
	DECLARE_DRIVER_INIT(freezeat2);
	DECLARE_DRIVER_INIT(area51a);

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
	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE_LINE_MEMBER( gpu_cpu_int );
	DECLARE_WRITE_LINE_MEMBER( dsp_cpu_int );
	DECLARE_WRITE_LINE_MEMBER( external_int );

	int quickload(device_image_interface &image, const char *file_type, int quickload_size);
	void cart_start();
	IRQ_CALLBACK_MEMBER(jaguar_irq_callback);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER( jaguar_cart );
	DECLARE_QUICKLOAD_LOAD_MEMBER( jaguar );
protected:
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
	virtual void machine_reset() override;
	virtual void sound_start() override;
	virtual void video_start() override;
	virtual void device_postload();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void gpu_suspend() { m_gpu->suspend(SUSPEND_REASON_SPIN, 1); }
	void gpu_resume() { m_gpu->resume(SUSPEND_REASON_SPIN); }
	void dsp_suspend() { m_dsp->suspend(SUSPEND_REASON_SPIN, 1); }
	void dsp_resume() { m_dsp->resume(SUSPEND_REASON_SPIN); }

	void fix_endian( UINT32 addr, UINT32 size );
	void cojag_common_init(UINT16 gpu_jump_offs, UINT16 spin_pc);
	void init_freeze_common(offs_t main_speedup_addr);

	// from audio/jaguar.c
	void update_gpu_irq();
	DECLARE_WRITE32_MEMBER( dsp_flags_w );

	// from video/jaguar.c
	void get_crosshair_xy(int player, int &x, int &y);
	int effective_hvalue(int value);
	bool adjust_object_timer(int vc);
	void update_cpu_irq();
	UINT8 *memory_base(UINT32 offset) { return reinterpret_cast<UINT8 *>(m_gpu->space(AS_PROGRAM).get_read_ptr(offset)); }
	void blitter_run();
	void scanline_update(int param);
	void set_palette(UINT16 vmode);

	/* from jagobj.c */
	void jagobj_init();
	UINT32 *process_bitmap(UINT16 *scanline, UINT32 *objdata, int vc, int logit);
	UINT32 *process_scaled_bitmap(UINT16 *scanline, UINT32 *objdata, int vc, int logit);
	UINT32 *process_branch(UINT32 *objdata, int vc, int logit);
	void process_object_list(int vc, UINT16 *_scanline);
	void bitmap_4_draw(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos, UINT8 flags, INT32 dxpos, UINT16 *clutbase);
	void bitmap_4_0(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos, UINT16 *clutbase);
	void bitmap_4_1(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos, UINT16 *clutbase);
	void bitmap_4_2(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos, UINT16 *clutbase);
	void bitmap_4_3(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos, UINT16 *clutbase);
	void bitmap_4_4(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos, UINT16 *clutbase);
	void bitmap_4_5(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos, UINT16 *clutbase);
	void bitmap_4_6(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos, UINT16 *clutbase);
	void bitmap_4_7(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos, UINT16 *clutbase);
	void bitmap_8_draw(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos, UINT8 flags, INT32 dxpos, UINT16 *clutbase);
	void bitmap_8_0(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos, UINT16 *clutbase);
	void bitmap_8_1(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos, UINT16 *clutbase);
	void bitmap_8_2(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos, UINT16 *clutbase);
	void bitmap_8_3(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos, UINT16 *clutbase);
	void bitmap_8_4(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos, UINT16 *clutbase);
	void bitmap_8_5(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos, UINT16 *clutbase);
	void bitmap_8_6(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos, UINT16 *clutbase);
	void bitmap_8_7(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos, UINT16 *clutbase);
	void bitmap_16_draw(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos, UINT8 flags, INT32 dxpos);
	void bitmap_16_0(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos);
	void bitmap_16_1(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos);
	void bitmap_16_2(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos);
	void bitmap_16_3(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos);
	void bitmap_16_4(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos);
	void bitmap_16_5(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos);
	void bitmap_16_6(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos);
	void bitmap_16_7(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos);
	void bitmap_32_draw(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos, UINT8 flags, INT32 dxpos);
	void bitmap_32_0(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos);
	void bitmap_32_1(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos);
	void bitmap_32_2(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos);
	void bitmap_32_3(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos);
	void bitmap_32_4(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos);
	void bitmap_32_5(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos);
	void bitmap_32_6(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos);
	void bitmap_32_7(UINT16 *scanline, INT32 firstpix, INT32 iwidth, UINT32 *src, INT32 xpos);

	/* from jagblit.c */
	void generic_blitter(UINT32 command, UINT32 a1flags, UINT32 a2flags);
	void blitter_09800001_010020_010020(UINT32 command, UINT32 a1flags, UINT32 a2flags);
	void blitter_09800009_000020_000020(UINT32 command, UINT32 a1flags, UINT32 a2flags);
	void blitter_01800009_000028_000028(UINT32 command, UINT32 a1flags, UINT32 a2flags);
	void blitter_01800001_000018_000018(UINT32 command, UINT32 a1flags, UINT32 a2flags);
	void blitter_01c00001_000018_000018(UINT32 command, UINT32 a1flags, UINT32 a2flags);
	void blitter_00010000_xxxxxx_xxxxxx(UINT32 command, UINT32 a1flags, UINT32 a2flags);
	void blitter_01800001_xxxxxx_xxxxxx(UINT32 command, UINT32 a1flags, UINT32 a2flags);
	void blitter_x1800x01_xxxxxx_xxxxxx(UINT32 command, UINT32 a1flags, UINT32 a2flags);

	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	optional_device<vt83c461_device> m_ide;
	required_device<screen_device> m_screen;
};
