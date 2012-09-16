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
#include "machine/nubus.h"
#include "machine/ncr539x.h"
#include "machine/ncr5380.h"
#include "machine/mackbd.h"
#include "sound/asc.h"
#include "sound/awacs.h"

#define MAC_SCREEN_NAME "screen"
#define MAC_539X_1_TAG "scsi:539x_1"
#define MAC_539X_2_TAG "scsi:539x_2"

// model helpers
#define ADB_IS_BITBANG	((mac->m_model == MODEL_MAC_SE || mac->m_model == MODEL_MAC_CLASSIC) || (mac->m_model >= MODEL_MAC_II && mac->m_model <= MODEL_MAC_IICI) || (mac->m_model == MODEL_MAC_SE30) || (mac->m_model == MODEL_MAC_QUADRA_700))
#define ADB_IS_BITBANG_CLASS	((m_model == MODEL_MAC_SE || m_model == MODEL_MAC_CLASSIC) || (m_model >= MODEL_MAC_II && m_model <= MODEL_MAC_IICI) || (m_model == MODEL_MAC_SE30) || (m_model == MODEL_MAC_QUADRA_700))
#define ADB_IS_EGRET	(mac->m_model >= MODEL_MAC_LC && mac->m_model <= MODEL_MAC_CLASSIC_II) || ((mac->m_model >= MODEL_MAC_IISI) && (mac->m_model <= MODEL_MAC_IIVI))
#define ADB_IS_CUDA	    ((mac->m_model >= MODEL_MAC_COLOR_CLASSIC && mac->m_model <= MODEL_MAC_LC_580) || ((mac->m_model >= MODEL_MAC_QUADRA_660AV) && (mac->m_model <= MODEL_MAC_QUADRA_630)) || (mac->m_model >= MODEL_MAC_POWERMAC_6100))
#define ADB_IS_PM_VIA1	(mac->m_model >= MODEL_MAC_PORTABLE && mac->m_model <= MODEL_MAC_PB100)
#define ADB_IS_PM_VIA2	(mac->m_model >= MODEL_MAC_PB140 && mac->m_model <= MODEL_MAC_PBDUO_270c)
#define ADB_IS_PM_VIA1_CLASS	(m_model >= MODEL_MAC_PORTABLE && m_model <= MODEL_MAC_PB100)
#define ADB_IS_PM_VIA2_CLASS	(m_model >= MODEL_MAC_PB140 && m_model <= MODEL_MAC_PBDUO_270c)
#define ADB_IS_PM_CLASS	((m_model >= MODEL_MAC_PORTABLE && m_model <= MODEL_MAC_PB100) || (m_model >= MODEL_MAC_PB140 && m_model <= MODEL_MAC_PBDUO_270c))

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
	MODEL_MAC_128K512K,	// 68000 machines
	MODEL_MAC_512KE,
	MODEL_MAC_PLUS,
	MODEL_MAC_SE,
	MODEL_MAC_CLASSIC,

    MODEL_MAC_PORTABLE, // Portable/PB100 are sort of hybrid classic and Mac IIs
	MODEL_MAC_PB100,

	MODEL_MAC_II,		// Mac II class 68020/030 machines
	MODEL_MAC_II_FDHD,
	MODEL_MAC_IIX,
	MODEL_MAC_IICX,
	MODEL_MAC_IICI,
	MODEL_MAC_IISI,
	MODEL_MAC_IIVX,
	MODEL_MAC_IIVI,
	MODEL_MAC_IIFX,
	MODEL_MAC_SE30,

	MODEL_MAC_LC,		// LC class 68030 machines, generally using a V8 or compatible gate array
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

	MODEL_MAC_PB140,	// 68030 PowerBooks.  140/145/145B/170 all have the same machine ID
	MODEL_MAC_PB160,    // 160/180/165 all have the same machine ID too
	MODEL_MAC_PB165c,
	MODEL_MAC_PB180c,
	MODEL_MAC_PB150,    // 150 is fairly radically different from the other 1x0s

	MODEL_MAC_PBDUO_210,	// 68030 PowerBook Duos
	MODEL_MAC_PBDUO_230,
	MODEL_MAC_PBDUO_250,
	MODEL_MAC_PBDUO_270c,

	MODEL_MAC_QUADRA_700,	// 68(LC)040 desktops
	MODEL_MAC_QUADRA_610,
	MODEL_MAC_QUADRA_650,
    MODEL_MAC_QUADRA_800,
	MODEL_MAC_QUADRA_900,
	MODEL_MAC_QUADRA_950,
	MODEL_MAC_QUADRA_660AV,
	MODEL_MAC_QUADRA_840AV,
	MODEL_MAC_QUADRA_605,
	MODEL_MAC_QUADRA_630,

	MODEL_MAC_PB550c,	// 68(LC)040 PowerBooks
	MODEL_MAC_PB520,
	MODEL_MAC_PB520c,
	MODEL_MAC_PB540,
	MODEL_MAC_PB540c,
	MODEL_MAC_PB190,
	MODEL_MAC_PB190cs,

	MODEL_MAC_POWERMAC_6100,	// NuBus PowerMacs
	MODEL_MAC_POWERMAC_7100,
	MODEL_MAC_POWERMAC_8100
};

// video parameters for classic Macs
#define MAC_H_VIS	(512)
#define MAC_V_VIS	(342)
#define MAC_H_TOTAL	(704)		// (512+192)
#define MAC_V_TOTAL	(370)		// (342+28)

/*----------- defined in machine/mac.c -----------*/

extern const via6522_interface mac_via6522_intf;
extern const via6522_interface mac_via6522_2_intf;
extern const via6522_interface mac_via6522_adb_intf;

void mac_scsi_irq(running_machine &machine, int state);
void mac_asc_irq(device_t *device, int state);
void mac_fdc_set_enable_lines(device_t *device, int enable_mask);






NVRAM_HANDLER( mac );

/*----------- defined in video/mac.c -----------*/












SCREEN_UPDATE_IND16( mac );
SCREEN_UPDATE_IND16( macse30 );
SCREEN_UPDATE_IND16( macprtb );
SCREEN_UPDATE_IND16( macpb140 );
SCREEN_UPDATE_IND16( macpb160 );
SCREEN_UPDATE_RGB32( macrbv );
SCREEN_UPDATE_RGB32( macrbvvram );
SCREEN_UPDATE_RGB32( macdafb );
SCREEN_UPDATE_RGB32( macpbwd );




/*----------- defined in audio/mac.c -----------*/

class mac_sound_device : public device_t,
                                  public device_sound_interface
{
public:
	mac_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~mac_sound_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type MAC_SOUND;


void mac_enable_sound( device_t *device, int on );
void mac_set_sound_buffer( device_t *device, int buffer );
void mac_set_volume( device_t *device, int volume );

void mac_sh_updatebuffer(device_t *device);

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
        m_screen(*this, MAC_SCREEN_NAME),
        m_539x_1(*this, MAC_539X_1_TAG),
        m_539x_2(*this, MAC_539X_2_TAG),
		m_ncr5380(*this, "scsi:ncr5380"),
        m_mackbd(*this, MACKBD_TAG),
		m_vram(*this,"vram"),
		m_vram16(*this,"vram16")
	 { }

	required_device<cpu_device> m_maincpu;
	required_device<via6522_device> m_via1;
	optional_device<via6522_device> m_via2;
	optional_device<asc_device> m_asc;
	optional_device<awacs_device> m_awacs;
    optional_device<egret_device> m_egret;
    optional_device<cuda_device> m_cuda;
	required_device<ram_device> m_ram;
    optional_device<screen_device> m_screen;
    optional_device<ncr539x_device> m_539x_1;
    optional_device<ncr539x_device> m_539x_2;
    optional_device<ncr5380_device> m_ncr5380;
    optional_device<mackbd_device> m_mackbd;

	virtual void machine_start();
	virtual void machine_reset();

	model_t m_model;

	UINT32 m_overlay;
	int m_drive_select;
	int m_scsiirq_enable;

	UINT32 m_via2_vbl;
	UINT32 m_se30_vbl_enable;
	UINT8 m_nubus_irq_state;

	/* used to store the reply to most keyboard commands */
	int m_keyboard_reply;

	/* Keyboard communication in progress? */
	int m_kbd_comm;
	int m_kbd_receive;
	/* timer which is used to time out inquiry */
	emu_timer *m_inquiry_timeout;

	int m_kbd_shift_reg;
	int m_kbd_shift_count;

	/* keyboard matrix to detect transition */
	int m_key_matrix[7];

	/* keycode buffer (used for keypad/arrow key transition) */
	int m_keycode_buf[2];
	int m_keycode_buf_index;

	int m_mouse_bit_x;
	int m_mouse_bit_y;
	int last_mx, last_my;
	int count_x, count_y;
	int m_last_was_x;
	int m_screen_buffer;

	int irq_count, ca1_data, ca2_data;

	/* state of rTCEnb and rTCClk lines */
	UINT8 m_rtc_rTCEnb;
	UINT8 m_rtc_rTCClk;

	/* serial transmit/receive register : bits are shifted in/out of this byte */
	UINT8 m_rtc_data_byte;
	/* serial transmitted/received bit count */
	UINT8 m_rtc_bit_count;
	/* direction of the current transfer (0 : VIA->RTC, 1 : RTC->VIA) */
	UINT8 m_rtc_data_dir;
	/* when rtc_data_dir == 1, state of rTCData as set by RTC (-> data bit seen by VIA) */
	UINT8 m_rtc_data_out;

	/* set to 1 when command in progress */
	UINT8 m_rtc_cmd;

	/* write protect flag */
	UINT8 m_rtc_write_protect;

	/* internal seconds register */
	UINT8 m_rtc_seconds[/*8*/4];
	/* 20-byte long PRAM, or 256-byte long XPRAM */
	UINT8 m_rtc_ram[256];
	/* current extended address and RTC state */
	UINT8 m_rtc_xpaddr;
	UINT8 m_rtc_state;

	// Mac ADB state
	INT32 m_adb_irq_pending, m_adb_waiting_cmd, m_adb_datasize, m_adb_buffer[257];
	INT32 m_adb_state, m_adb_command, m_adb_send, m_adb_timer_ticks, m_adb_extclock, m_adb_direction;
	INT32 m_adb_listenreg, m_adb_listenaddr, m_adb_last_talk, m_adb_srq_switch;
	INT32 m_adb_streaming, m_adb_stream_ptr;

	// Portable/PB100 Power Manager IC comms (chapter 4, "Guide to the Macintosh Family Hardware", second edition)
	UINT8 m_pm_data_send, m_pm_data_recv, m_pm_ack, m_pm_req, m_pm_cmd[32], m_pm_out[32], m_pm_dptr, m_pm_sptr, m_pm_slen, m_pm_state;
    UINT8 m_pmu_int_status, m_pmu_last_adb_command, m_pmu_poll;
	emu_timer *m_pmu_send_timer;

	// 60.15 Hz timer for RBV/V8/Sonora/Eagle/VASP/etc.
	emu_timer *m_6015_timer;

	// RBV and friends (V8, etc)
	UINT8 m_rbv_regs[256], m_rbv_ier, m_rbv_ifr, m_rbv_type, m_rbv_montype, m_rbv_vbltime;
	UINT32 m_rbv_colors[3], m_rbv_count, m_rbv_clutoffs, m_rbv_immed10wr;
	UINT32 m_rbv_palette[256];
	UINT8 m_sonora_vctl[8];
    emu_timer *m_vbl_timer, *m_cursor_timer;
    UINT16 m_cursor_line;
    UINT16 m_dafb_int_status;
    int m_dafb_scsi1_drq, m_dafb_scsi2_drq;
    UINT8 m_dafb_mode;
    UINT32 m_dafb_base, m_dafb_stride;

    // this is shared among all video setups with vram
	optional_shared_ptr<UINT32> m_vram;
	optional_shared_ptr<UINT16> m_vram16;

	// interrupts
	int m_scc_interrupt, m_via_interrupt, m_via2_interrupt, m_scsi_interrupt, m_asc_interrupt, m_last_taken_interrupt;

	// defined in machine/mac.c
	void v8_resize();
	void set_memory_overlay(int overlay);
	void scc_mouse_irq( int x, int y );
	void nubus_slot_interrupt(UINT8 slot, UINT32 state);
	void set_scc_interrupt(bool value);
	void set_via_interrupt(int value);
	void set_via2_interrupt(int value);
	void field_interrupts();
	void rtc_write_rTCEnb(int data);
	void rtc_shift_data(int data);
	void vblank_irq();
	void rtc_incticks();
	void adb_talk();
	void mouse_callback();
    void rbv_recalc_irqs();
	void pmu_exec();
	void mac_adb_newaction(int state);

	DECLARE_READ16_MEMBER ( mac_via_r );
	DECLARE_WRITE16_MEMBER ( mac_via_w );
	DECLARE_READ16_MEMBER ( mac_via2_r );
	DECLARE_WRITE16_MEMBER ( mac_via2_w );
	DECLARE_READ16_MEMBER ( mac_autovector_r );
	DECLARE_WRITE16_MEMBER ( mac_autovector_w );
	DECLARE_READ16_MEMBER ( mac_iwm_r );
	DECLARE_WRITE16_MEMBER ( mac_iwm_w );
	DECLARE_READ16_MEMBER ( mac_scc_r );
	DECLARE_WRITE16_MEMBER ( mac_scc_w );
	DECLARE_WRITE16_MEMBER ( mac_scc_2_w );
	DECLARE_READ16_MEMBER ( macplus_scsi_r );
	DECLARE_WRITE16_MEMBER ( macplus_scsi_w );
	DECLARE_WRITE16_MEMBER ( macii_scsi_w );
	DECLARE_READ32_MEMBER (macii_scsi_drq_r);
	DECLARE_WRITE32_MEMBER (macii_scsi_drq_w);

	DECLARE_READ32_MEMBER( rbv_ramdac_r );
	DECLARE_WRITE32_MEMBER( rbv_ramdac_w );
	DECLARE_WRITE32_MEMBER( ariel_ramdac_w );
	DECLARE_READ8_MEMBER( mac_sonora_vctl_r );
	DECLARE_WRITE8_MEMBER( mac_sonora_vctl_w );
	DECLARE_READ8_MEMBER ( mac_rbv_r );
	DECLARE_WRITE8_MEMBER ( mac_rbv_w );

	DECLARE_READ32_MEMBER(mac_read_id);

	DECLARE_READ16_MEMBER(mac_config_r);

    DECLARE_READ32_MEMBER(biu_r);
    DECLARE_WRITE32_MEMBER(biu_w);
    DECLARE_READ8_MEMBER(oss_r);
    DECLARE_WRITE8_MEMBER(oss_w);
    DECLARE_READ32_MEMBER(buserror_r);
    DECLARE_READ8_MEMBER(swimiop_r);
    DECLARE_WRITE8_MEMBER(swimiop_w);
    DECLARE_READ8_MEMBER(scciop_r);
    DECLARE_WRITE8_MEMBER(scciop_w);

    DECLARE_READ8_MEMBER(hmc_r);
    DECLARE_WRITE8_MEMBER(hmc_w);
    DECLARE_READ8_MEMBER(amic_dma_r);
    DECLARE_WRITE8_MEMBER(amic_dma_w);
    DECLARE_READ8_MEMBER(pmac_diag_r);

    DECLARE_READ8_MEMBER(mac_gsc_r);
    DECLARE_WRITE8_MEMBER(mac_gsc_w);

    DECLARE_READ8_MEMBER(mac_5396_r);
    DECLARE_WRITE8_MEMBER(mac_5396_w);

    DECLARE_READ32_MEMBER(dafb_r);
    DECLARE_WRITE32_MEMBER(dafb_w);
    DECLARE_READ32_MEMBER(dafb_dac_r);
    DECLARE_WRITE32_MEMBER(dafb_dac_w);

    DECLARE_READ32_MEMBER(macwd_r);
    DECLARE_WRITE32_MEMBER(macwd_w);

    DECLARE_WRITE_LINE_MEMBER(nubus_irq_9_w);
    DECLARE_WRITE_LINE_MEMBER(nubus_irq_a_w);
    DECLARE_WRITE_LINE_MEMBER(nubus_irq_b_w);
    DECLARE_WRITE_LINE_MEMBER(nubus_irq_c_w);
    DECLARE_WRITE_LINE_MEMBER(nubus_irq_d_w);
    DECLARE_WRITE_LINE_MEMBER(nubus_irq_e_w);

    DECLARE_WRITE_LINE_MEMBER(irq_539x_1_w);
    DECLARE_WRITE_LINE_MEMBER(drq_539x_1_w);

    DECLARE_WRITE_LINE_MEMBER(cuda_reset_w);

	DECLARE_DIRECT_UPDATE_MEMBER(overlay_opbaseoverride);
private:
	int has_adb();
	void rtc_init();
	void rtc_execute_cmd(int data);
	void adb_reset();
	void adb_vblank();
	int adb_pollkbd(int update);
	int adb_pollmouse();
	void adb_accummouse( UINT8 *MouseX, UINT8 *MouseY );
	void pmu_one_byte_reply(UINT8 result);
	void pmu_three_byte_reply(UINT8 result1, UINT8 result2, UINT8 result3);

    // wait states for accessing the VIA
    int m_via_cycles;

	// ADB mouse state
	int m_adb_mouseaddr;
	int m_adb_lastmousex, m_adb_lastmousey, m_adb_lastbutton, m_adb_mouse_initialized;

	// ADB keyboard state
	int m_adb_keybaddr;
	int m_adb_keybinitialized, m_adb_currentkeys[2], m_adb_modifiers;

    UINT8 m_oss_regs[0x400];

    // AMIC for x100 PowerMacs
    UINT8 m_amic_regs[0x200];

    // HMC for x100 PowerMacs
    UINT64 m_hmc_reg, m_hmc_shiftout;

public:
	emu_timer *m_scanline_timer;
	emu_timer *m_adb_timer;
	DECLARE_DRIVER_INIT(maclc2);
	DECLARE_DRIVER_INIT(maciifdhd);
	DECLARE_DRIVER_INIT(macse30);
	DECLARE_DRIVER_INIT(macprtb);
	DECLARE_DRIVER_INIT(maciivx);
	DECLARE_DRIVER_INIT(macpd210);
	DECLARE_DRIVER_INIT(macii);
	DECLARE_DRIVER_INIT(macclassic);
	DECLARE_DRIVER_INIT(macquadra700);
	DECLARE_DRIVER_INIT(macclassic2);
	DECLARE_DRIVER_INIT(maciifx);
	DECLARE_DRIVER_INIT(maclc);
	DECLARE_DRIVER_INIT(macpb160);
	DECLARE_DRIVER_INIT(macplus);
	DECLARE_DRIVER_INIT(macse);
	DECLARE_DRIVER_INIT(macpb140);
	DECLARE_DRIVER_INIT(mac128k512k);
	DECLARE_DRIVER_INIT(macpm6100);
	DECLARE_DRIVER_INIT(mac512ke);
	DECLARE_DRIVER_INIT(maclc520);
	DECLARE_DRIVER_INIT(maciici);
	DECLARE_DRIVER_INIT(maciix);
	DECLARE_DRIVER_INIT(maclrcclassic);
	DECLARE_DRIVER_INIT(maciisi);
	DECLARE_DRIVER_INIT(maciicx);
	DECLARE_DRIVER_INIT(maclc3);
	DECLARE_DRIVER_INIT(maclc3plus);
	DECLARE_DRIVER_INIT(macpm7100);
	DECLARE_DRIVER_INIT(macpm8100);
	DECLARE_DRIVER_INIT(macpb100);
	DECLARE_VIDEO_START(mac);
	DECLARE_PALETTE_INIT(mac);
	DECLARE_VIDEO_START(macprtb);
	DECLARE_PALETTE_INIT(macgsc);
	DECLARE_VIDEO_START(macsonora);
	DECLARE_VIDEO_RESET(macrbv);
	DECLARE_VIDEO_START(macdafb);
	DECLARE_VIDEO_RESET(macdafb);
	DECLARE_VIDEO_START(macv8);
	DECLARE_VIDEO_RESET(macsonora);
	DECLARE_VIDEO_RESET(maceagle);
	DECLARE_VIDEO_START(macrbv);
};

#endif /* MAC_H_ */


