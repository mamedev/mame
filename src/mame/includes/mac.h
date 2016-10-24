// license:BSD-3-Clause
// copyright-holders:Nathan Woods, Raphael Nabet, R. Belmont
/*****************************************************************************
 *
 * includes/mac.h
 *
 * Macintosh driver declarations
 *
 ****************************************************************************/

#ifndef MAC_H_
#define MAC_H_

#include "machine/8530scc.h"
#include "machine/6522via.h"
#include "machine/ram.h"
#include "machine/egret.h"
#include "machine/cuda.h"
#include "bus/nubus/nubus.h"
#include "bus/macpds/macpds.h"
#include "machine/ncr539x.h"
#include "machine/ncr5380.h"
#include "machine/mackbd.h"
#include "machine/macrtc.h"
#include "sound/asc.h"
#include "sound/awacs.h"
#include "sound/dac.h"
#include "cpu/m68000/m68000.h"

#define MAC_SCREEN_NAME "screen"
#define MAC_539X_1_TAG "539x_1"
#define MAC_539X_2_TAG "539x_2"
#define MACKBD_TAG "mackbd"

// uncomment to run i8021 keyboard in original Mac/512(e)/Plus
//#define MAC_USE_EMULATED_KBD (1)

// model helpers
#define ADB_IS_BITBANG  ((mac->m_model == MODEL_MAC_SE || mac->m_model == MODEL_MAC_CLASSIC) || (mac->m_model >= MODEL_MAC_II && mac->m_model <= MODEL_MAC_IICI) || (mac->m_model == MODEL_MAC_SE30) || (mac->m_model == MODEL_MAC_QUADRA_700))
#define ADB_IS_BITBANG_CLASS    ((m_model == MODEL_MAC_SE || m_model == MODEL_MAC_CLASSIC) || (m_model >= MODEL_MAC_II && m_model <= MODEL_MAC_IICI) || (m_model == MODEL_MAC_SE30) || (m_model == MODEL_MAC_QUADRA_700))
#define ADB_IS_EGRET    (m_model >= MODEL_MAC_LC && m_model <= MODEL_MAC_CLASSIC_II) || ((m_model >= MODEL_MAC_IISI) && (m_model <= MODEL_MAC_IIVI))
#define ADB_IS_EGRET_NONCLASS   (mac->m_model >= MODEL_MAC_LC && mac->m_model <= MODEL_MAC_CLASSIC_II) || ((mac->m_model >= MODEL_MAC_IISI) && (mac->m_model <= MODEL_MAC_IIVI))
#define ADB_IS_CUDA     ((m_model >= MODEL_MAC_COLOR_CLASSIC && m_model <= MODEL_MAC_LC_580) || ((m_model >= MODEL_MAC_QUADRA_660AV) && (m_model <= MODEL_MAC_QUADRA_630)) || (m_model >= MODEL_MAC_POWERMAC_6100))
#define ADB_IS_CUDA_NONCLASS        ((mac->m_model >= MODEL_MAC_COLOR_CLASSIC && mac->m_model <= MODEL_MAC_LC_580) || ((mac->m_model >= MODEL_MAC_QUADRA_660AV) && (mac->m_model <= MODEL_MAC_QUADRA_630)) || (mac->m_model >= MODEL_MAC_POWERMAC_6100))
#define ADB_IS_PM_VIA1_CLASS    (m_model >= MODEL_MAC_PORTABLE && m_model <= MODEL_MAC_PB100)
#define ADB_IS_PM_VIA2_CLASS    (m_model >= MODEL_MAC_PB140 && m_model <= MODEL_MAC_PBDUO_270c)
#define ADB_IS_PM_CLASS ((m_model >= MODEL_MAC_PORTABLE && m_model <= MODEL_MAC_PB100) || (m_model >= MODEL_MAC_PB140 && m_model <= MODEL_MAC_PBDUO_270c))

/* for Egret and CUDA streaming MCU commands, command types */
enum mac_streaming_t
{
	MCU_STREAMING_NONE = 0,
	MCU_STREAMING_PRAMRD,
	MCU_STREAMING_PRAMWR,
	MCU_STREAMING_WRAMRD,
	MCU_STREAMING_WRAMWR
};

enum
{
	RBV_TYPE_RBV = 0,
	RBV_TYPE_V8,
	RBV_TYPE_SONORA,
	RBV_TYPE_DAFB
};

/* tells which model is being emulated (set by macxxx_init) */
enum model_t
{
	MODEL_MAC_128K512K, // 68000 machines
	MODEL_MAC_512KE,
	MODEL_MAC_PLUS,
	MODEL_MAC_SE,
	MODEL_MAC_CLASSIC,

	MODEL_MAC_PORTABLE, // Portable/PB100 are sort of hybrid classic and Mac IIs
	MODEL_MAC_PB100,

	MODEL_MAC_II,       // Mac II class 68020/030 machines
	MODEL_MAC_II_FDHD,
	MODEL_MAC_IIX,
	MODEL_MAC_IICX,
	MODEL_MAC_IICI,
	MODEL_MAC_IISI,
	MODEL_MAC_IIVX,
	MODEL_MAC_IIVI,
	MODEL_MAC_IIFX,
	MODEL_MAC_SE30,

	MODEL_MAC_LC,       // LC class 68030 machines, generally using a V8 or compatible gate array
	MODEL_MAC_LC_II,
	MODEL_MAC_LC_III,
	MODEL_MAC_LC_III_PLUS,
	MODEL_MAC_CLASSIC_II,
	MODEL_MAC_COLOR_CLASSIC,

	MODEL_MAC_LC_475,   // LC III clones with Cuda instead of Egret and 68LC040 on most models
	MODEL_MAC_LC_520,
	MODEL_MAC_LC_550,
	MODEL_MAC_TV,
	MODEL_MAC_LC_575,
	MODEL_MAC_LC_580,

	MODEL_MAC_PB140,    // 68030 PowerBooks.  140/145/145B/170 all have the same machine ID
	MODEL_MAC_PB160,    // 160/180/165 all have the same machine ID too
	MODEL_MAC_PB165c,
	MODEL_MAC_PB180c,
	MODEL_MAC_PB150,    // 150 is fairly radically different from the other 1x0s

	MODEL_MAC_PBDUO_210,    // 68030 PowerBook Duos
	MODEL_MAC_PBDUO_230,
	MODEL_MAC_PBDUO_250,
	MODEL_MAC_PBDUO_270c,

	MODEL_MAC_QUADRA_700,   // 68(LC)040 desktops
	MODEL_MAC_QUADRA_610,
	MODEL_MAC_QUADRA_650,
	MODEL_MAC_QUADRA_800,
	MODEL_MAC_QUADRA_900,
	MODEL_MAC_QUADRA_950,
	MODEL_MAC_QUADRA_660AV,
	MODEL_MAC_QUADRA_840AV,
	MODEL_MAC_QUADRA_605,
	MODEL_MAC_QUADRA_630,

	MODEL_MAC_PB550c,   // 68(LC)040 PowerBooks
	MODEL_MAC_PB520,
	MODEL_MAC_PB520c,
	MODEL_MAC_PB540,
	MODEL_MAC_PB540c,
	MODEL_MAC_PB190,
	MODEL_MAC_PB190cs,

	MODEL_MAC_POWERMAC_6100,    // NuBus PowerMacs
	MODEL_MAC_POWERMAC_7100,
	MODEL_MAC_POWERMAC_8100
};

// video parameters for classic Macs
#define MAC_H_VIS   (512)
#define MAC_V_VIS   (342)
#define MAC_H_TOTAL (704)       // (512+192)
#define MAC_V_TOTAL (370)       // (342+28)

/*----------- defined in machine/mac.c -----------*/

void mac_fdc_set_enable_lines(device_t *device, int enable_mask);

/* Mac driver data */

class mac_state : public driver_device
{
public:
	mac_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_via1(*this, "via6522_0"),
		m_via2(*this, "via6522_1"),
		m_asc(*this, "asc"),
		m_awacs(*this, "awacs"),
		m_egret(*this, EGRET_TAG),
		m_cuda(*this, CUDA_TAG),
		m_ram(*this, RAM_TAG),
		m_539x_1(*this, MAC_539X_1_TAG),
		m_539x_2(*this, MAC_539X_2_TAG),
		m_ncr5380(*this, "ncr5380"),
		m_mackbd(*this, MACKBD_TAG),
		m_rtc(*this,"rtc"),
		m_mouse0(*this, "MOUSE0"),
		m_mouse1(*this, "MOUSE1"),
		m_mouse2(*this, "MOUSE2"),
		m_keys(*this, "KEY%u", 0),
		m_montype(*this, "MONTYPE"),
		m_snd_enable(false),
		m_main_buffer(true),
		m_snd_vol(7),
		m_vram(*this,"vram"),
		m_vram16(*this,"vram16"),
		m_via2_ca1_hack(0),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_dac(*this, "dac")
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via1;
	optional_device<via6522_device> m_via2;
	optional_device<asc_device> m_asc;
	optional_device<awacs_device> m_awacs;
	optional_device<egret_device> m_egret;
	optional_device<cuda_device> m_cuda;
	required_device<ram_device> m_ram;
	optional_device<ncr539x_device> m_539x_1;
	optional_device<ncr539x_device> m_539x_2;
	optional_device<ncr5380_device> m_ncr5380;
	optional_device<mackbd_device> m_mackbd;
	optional_device<rtc3430042_device> m_rtc;

	required_ioport m_mouse0, m_mouse1, m_mouse2;
	optional_ioport_array<7> m_keys;
	optional_ioport m_montype;

	virtual void machine_start() override;
	virtual void machine_reset() override;

	model_t m_model;

	uint32_t m_overlay;
	int m_drive_select;
	int m_scsiirq_enable;

	uint32_t m_via2_vbl;
	uint32_t m_se30_vbl_enable;
	uint8_t m_nubus_irq_state;

	emu_timer *m_overlay_timeout;
	void overlay_timeout_func(void *ptr, int32_t param);
	uint32_t rom_switch_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);

	void mac_scanline(timer_device &timer, void *ptr, int32_t param);
	bool m_snd_enable;
	bool m_main_buffer;
	int m_snd_vol;

#ifndef MAC_USE_EMULATED_KBD
	/* used to store the reply to most keyboard commands */
	int m_keyboard_reply;

	/* Keyboard communication in progress? */
	int m_kbd_comm;
	int m_kbd_receive;
	/* timer which is used to time out inquiry */
	emu_timer *m_inquiry_timeout;

	int m_kbd_shift_reg;
	int m_kbd_shift_count;

	/* keycode buffer (used for keypad/arrow key transition) */
	int m_keycode_buf[2];
	int m_keycode_buf_index;
#endif

	/* keyboard matrix to detect transition - macadb needs to stop relying on this */
	int m_key_matrix[7];

	int m_mouse_bit_x;
	int m_mouse_bit_y;
	int last_mx, last_my;
	int count_x, count_y;
	int m_last_was_x;
	int m_screen_buffer;

	int irq_count, ca1_data, ca2_data;

	// Mac ADB state
	int32_t m_adb_irq_pending, m_adb_waiting_cmd, m_adb_datasize, m_adb_buffer[257];
	int32_t m_adb_state, m_adb_command, m_adb_send, m_adb_timer_ticks, m_adb_extclock, m_adb_direction;
	int32_t m_adb_listenreg, m_adb_listenaddr, m_adb_last_talk, m_adb_srq_switch;
	int32_t m_adb_streaming, m_adb_stream_ptr;
	int32_t m_adb_linestate;
	bool  m_adb_srqflag;
#define kADBKeyBufSize 32
		uint8_t m_adb_keybuf[kADBKeyBufSize];
		uint8_t m_adb_keybuf_start;
		uint8_t m_adb_keybuf_end;

	// Portable/PB100 Power Manager IC comms (chapter 4, "Guide to the Macintosh Family Hardware", second edition)
	uint8_t m_pm_data_send, m_pm_data_recv, m_pm_ack, m_pm_req, m_pm_cmd[32], m_pm_out[32], m_pm_dptr, m_pm_sptr, m_pm_slen, m_pm_state;
	uint8_t m_pmu_int_status, m_pmu_last_adb_command, m_pmu_poll;
	emu_timer *m_pmu_send_timer;

	// 60.15 Hz timer for RBV/V8/Sonora/Eagle/VASP/etc.
	emu_timer *m_6015_timer;

	// RBV and friends (V8, etc)
	uint8_t m_rbv_regs[256], m_rbv_ier, m_rbv_ifr, m_rbv_type, m_rbv_montype, m_rbv_vbltime;
	uint32_t m_rbv_colors[3], m_rbv_count, m_rbv_clutoffs, m_rbv_immed10wr;
	uint32_t m_rbv_palette[256];
	uint8_t m_sonora_vctl[8];
	emu_timer *m_vbl_timer, *m_cursor_timer;
	uint16_t m_cursor_line;
	uint16_t m_dafb_int_status;
	int m_dafb_scsi1_drq, m_dafb_scsi2_drq;
	uint8_t m_dafb_mode;
	uint32_t m_dafb_base, m_dafb_stride;

	// this is shared among all video setups with vram
	optional_shared_ptr<uint32_t> m_vram;
	optional_shared_ptr<uint16_t> m_vram16;

	// interrupts
	int m_scc_interrupt, m_via_interrupt, m_via2_interrupt, m_scsi_interrupt, m_asc_interrupt, m_last_taken_interrupt;

	// defined in machine/mac.c
	void v8_resize();
	void set_memory_overlay(int overlay);
	void scc_mouse_irq( int x, int y );
	void nubus_slot_interrupt(uint8_t slot, uint32_t state);
	void set_scc_interrupt(int state);
	void set_via_interrupt(int value);
	void set_via2_interrupt(int value);
	void field_interrupts();
	void vblank_irq();
	void adb_talk();
	void mouse_callback();
	void rbv_recalc_irqs();
	void pmu_exec();
	void mac_adb_newaction(int state);
	void set_adb_line(int linestate);
	void update_volume();

	uint16_t mac_via_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void mac_via_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t mac_via2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void mac_via2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t mac_autovector_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void mac_autovector_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t mac_iwm_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void mac_iwm_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t mac_scc_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void mac_scc_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void mac_scc_2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t macplus_scsi_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void macplus_scsi_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void macii_scsi_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint32_t macii_scsi_drq_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void macii_scsi_drq_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	uint32_t rbv_ramdac_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void rbv_ramdac_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void ariel_ramdac_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint8_t mac_sonora_vctl_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mac_sonora_vctl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mac_rbv_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mac_rbv_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint32_t mac_read_id(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);

	uint16_t mac_config_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);

	uint32_t biu_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void biu_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint8_t oss_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void oss_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint32_t buserror_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint8_t swimiop_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void swimiop_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t scciop_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void scciop_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t hmc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void hmc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t amic_dma_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void amic_dma_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t pmac_diag_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	uint8_t mac_gsc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mac_gsc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint8_t mac_5396_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mac_5396_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint32_t dafb_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void dafb_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t dafb_dac_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void dafb_dac_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	uint32_t macwd_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void macwd_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	void nubus_irq_9_w(int state);
	void nubus_irq_a_w(int state);
	void nubus_irq_b_w(int state);
	void nubus_irq_c_w(int state);
	void nubus_irq_d_w(int state);
	void nubus_irq_e_w(int state);

	void irq_539x_1_w(int state);
	void drq_539x_1_w(int state);

	void cuda_reset_w(int state);
	void adb_linechange_w(int state);

	void mac_scsi_irq(int state);
	void mac_asc_irq(int state);

private:
	int has_adb();
	void adb_reset();
	void adb_vblank();
	int adb_pollkbd(int update);
	int adb_pollmouse();
	void adb_accummouse( uint8_t *MouseX, uint8_t *MouseY );
	void pmu_one_byte_reply(uint8_t result);
	void pmu_three_byte_reply(uint8_t result1, uint8_t result2, uint8_t result3);

	// wait states for accessing the VIA
	int m_via_cycles;

	// ADB mouse state
	int m_adb_mouseaddr;
	int m_adb_lastmousex, m_adb_lastmousey, m_adb_lastbutton, m_adb_mouse_initialized;

	// ADB keyboard state
	int m_adb_keybaddr;
	int m_adb_keybinitialized, m_adb_currentkeys[2], m_adb_modifiers;

	// PRAM for ADB MCU HLEs (mostly unused now)
	uint8_t m_adb_pram[256];

	uint8_t m_oss_regs[0x400];

	// AMIC for x100 PowerMacs
	uint8_t m_amic_regs[0x200];

	// HMC for x100 PowerMacs
	uint64_t m_hmc_reg, m_hmc_shiftout;

	int m_via2_ca1_hack;
	optional_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;
	optional_device<dac_8bit_pwm_device> m_dac;

public:
	emu_timer *m_scanline_timer;
	emu_timer *m_adb_timer;
	void init_maclc2();
	void init_maciifdhd();
	void init_macse30();
	void init_macprtb();
	void init_maciivx();
	void init_macpd210();
	void init_macii();
	void init_macclassic();
	void init_macquadra700();
	void init_macclassic2();
	void init_maciifx();
	void init_maclc();
	void init_macpb160();
	void init_macplus();
	void init_macse();
	void init_macpb140();
	void init_mac128k512k();
	void init_macpm6100();
	void init_mac512ke();
	void init_maclc520();
	void init_maciici();
	void init_maciix();
	void init_maclrcclassic();
	void init_maciisi();
	void init_maciicx();
	void init_maclc3();
	void init_maclc3plus();
	void init_macpm7100();
	void init_macpm8100();
	void init_macpb100();
	void video_start_mac();
	void palette_init_mac(palette_device &palette);
	void video_start_macprtb();
	void palette_init_macgsc(palette_device &palette);
	void video_start_macsonora();
	void video_reset_macrbv();
	void video_start_macdafb();
	void video_reset_macdafb();
	void video_start_macv8();
	void video_reset_macsonora();
	void video_reset_maceagle();
	void video_start_macrbv();
	uint32_t screen_update_mac(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_macprtb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_macse30(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_macpb140(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_macpb160(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_macrbv(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_macdafb(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_macrbvvram(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_macv8(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_macsonora(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_macpbwd(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void mac_rbv_vbl(device_t &device);
#ifndef MAC_USE_EMULATED_KBD
	void kbd_clock(void *ptr, int32_t param);
	void inquiry_timeout_func(void *ptr, int32_t param);
#endif
	void mac_6015_tick(void *ptr, int32_t param);
	void mac_scanline_tick(void *ptr, int32_t param);
	void dafb_vbl_tick(void *ptr, int32_t param);
	void dafb_cursor_tick(void *ptr, int32_t param);
	void mac_adb_tick(void *ptr, int32_t param);    // macadb.c
	void mac_pmu_tick(void *ptr, int32_t param);    // macadb.c
	void mac_via_out_cb2(int state);
	void mac_adb_via_out_cb2(int state);
	uint8_t mac_via_in_a(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mac_via_in_b(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mac_via_out_a(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mac_via_out_b(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mac_via_in_a_pmu(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mac_via_in_b_pmu(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mac_via_out_a_pmu(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mac_via_out_b_pmu(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mac_via_out_b_bbadb(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mac_via_out_b_egadb(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mac_via_out_b_cdadb(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mac_via_in_b_via2pmu(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mac_via_out_b_via2pmu(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mac_via2_in_a(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mac_via2_in_b(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mac_via2_out_a(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mac_via2_out_b(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t mac_via2_in_a_pmu(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t mac_via2_in_b_pmu(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void mac_via2_out_a_pmu(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mac_via2_out_b_pmu(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mac_state_load();
	void mac_via_irq(int state);
	void mac_via2_irq(int state);
	void dafb_recalc_ints();
	void set_scc_waitrequest(int waitrequest);
	int scan_keyboard();
	void keyboard_init();
	void kbd_shift_out(int data);
	void keyboard_receive(int val);
	void mac_driver_init(model_t model);
	void mac_install_memory(offs_t memory_begin, offs_t memory_end,
		offs_t memory_size, void *memory_data, int is_rom, const char *bank);
	offs_t mac_dasm_override(device_t &device, char *buffer, offs_t pc, const uint8_t *oprom, const uint8_t *opram, int options);
};

#endif /* MAC_H_ */
