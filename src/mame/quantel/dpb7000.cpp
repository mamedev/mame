// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    DPB-7001 (c) 1981 Quantel

    Known issues/possible improvements:
    - Disk Sequencer Card is currently implemented via HLE.
    - Tablet is currently implemented via HLE.
    - Blanking PAL is currently disabled, which results in on-screen
      garbage when scrolling.
    - Size Card and Filter Card functionality is not yet implemented.
    - Video input functionality is not yet implemented.
    - There are numerous Brush Processor Card functions that are not yet
      implemented.

    To create a hard disk image compatible with the "330Mb Fujitsu" DIP
    setting:
    - chdman createhd -o [output name] -chs 1024,16,80 -ss 256 -c none

***************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6800/m6801.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/z8/z8.h"
#include "imagedev/floppy.h"
#include "imagedev/harddriv.h"
#include "machine/6850acia.h"
#include "machine/am25s55x.h"
#include "machine/am2901b.h"
#include "machine/am2910.h"
#include "machine/com8116.h"
#include "machine/fdc_pll.h"
#include "machine/input_merger.h"
#include "machine/mc68681.h"
#include "machine/tdc1008.h"
#include "video/mc6845.h"

#include "screen.h"
#include "emupal.h"

#include <algorithm>
#include <deque>

#define LOG_UNKNOWN         (1U << 1)
#define LOG_UCODE           (1U << 2)
#define LOG_MORE_UCODE      (1U << 3)
#define LOG_CSR             (1U << 4)
#define LOG_CTRLBUS         (1U << 5)
#define LOG_SYS_CTRL        (1U << 6)
#define LOG_FDC_CTRL        (1U << 7)
#define LOG_FDC_PORT        (1U << 8)
#define LOG_FDC_CMD         (1U << 9)
#define LOG_FDC_MECH        (1U << 10)
#define LOG_OUTPUT_TIMING   (1U << 11)
#define LOG_BRUSH_ADDR      (1U << 12)
#define LOG_STORE_ADDR      (1U << 13)
#define LOG_COMBINER        (1U << 14)
#define LOG_SIZE_CARD       (1U << 15)
#define LOG_FILTER_CARD     (1U << 16)
#define LOG_KEYBC           (1U << 17)
#define LOG_TDS             (1U << 18)
#define LOG_TABLET          (1U << 19)
#define LOG_COMMANDS        (1U << 20)
#define LOG_HDD             (1U << 21)
#define LOG_FDD             (1U << 22)
#define LOG_DDB             (1U << 23)
#define LOG_IRQ             (1U << 24)
#define LOG_BRUSH_LATCH     (1U << 25)
#define LOG_BRUSH_DRAWS     (1U << 26)
#define LOG_BRUSH_WRITES    (1U << 27)
#define LOG_STORE_READS     (1U << 28)
#define LOG_ALL             (LOG_UNKNOWN | LOG_CSR | LOG_CTRLBUS | LOG_SYS_CTRL | LOG_BRUSH_ADDR | \
							 LOG_STORE_ADDR | LOG_COMBINER | LOG_SIZE_CARD | LOG_FILTER_CARD | LOG_COMMANDS | LOG_OUTPUT_TIMING | \
							 LOG_BRUSH_LATCH | LOG_FDC_PORT | LOG_FDC_CMD | LOG_FDC_MECH | LOG_BRUSH_WRITES | LOG_STORE_READS)

//#define VERBOSE             (LOG_CSR | LOG_CTRLBUS | LOG_STORE_ADDR | LOG_COMBINER | LOG_SIZE_CARD | LOG_FILTER_CARD | LOG_BRUSH_ADDR | LOG_COMMANDS | LOG_OUTPUT_TIMING)
#define VERBOSE (0)
#include "logmacro.h"

namespace
{

class dpb7000_state : public driver_device
{
public:
	dpb7000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_acia(*this, "fd%u", 0U)
		, m_p_int(*this, "p_int")
		, m_brg(*this, "brg")
		, m_rs232(*this, "rs232%u", 0U)
		, m_crtc(*this, "crtc")
		, m_palette(*this, "palette")
		, m_vdu_ram(*this, "vduram")
		, m_vdu_char_rom(*this, "vduchar")
		, m_baud_dip(*this, "BAUD")
		, m_auto_start(*this, "AUTOSTART")
		, m_config_sw12(*this, "CONFIGSW12")
		, m_config_sw34(*this, "CONFIGSW34")
		, m_fddcpu(*this, "fddcpu")
		, m_fdd_serial(*this, "fddserial")
		, m_floppy0(*this, "0")
		, m_hdd(*this, "hdd")
		, m_floppy(nullptr)
		, m_output_cursor(*this, "output_timing_cursor")
		, m_output_hlines(*this, "output_timing_hlines")
		, m_output_hflags(*this, "output_timing_hflags")
		, m_output_vlines(*this, "output_timing_vlines")
		, m_output_vflags(*this, "output_timing_vflags")
		, m_brushaddr_pal(*this, "brushaddr_pal")
		, m_storeaddr_protx(*this, "storeaddr_prom_protx")
		, m_storeaddr_proty(*this, "storeaddr_prom_proty")
		, m_storeaddr_xlnib(*this, "storeaddr_prom_xlnib")
		, m_storeaddr_xmnib(*this, "storeaddr_prom_xmnib")
		, m_storeaddr_xhnib(*this, "storeaddr_prom_xhnib")
		, m_storeaddr_pal_blank(*this, "storeaddr_pal_blank")
		, m_keybcpu(*this, "keybcpu")
		, m_keybc_cols(*this, "KEYB_COL%u", 0U)
		, m_tds_cpu(*this, "tds")
		, m_tds_duart(*this, "tds_duart")
		, m_tds_dips(*this, "TDSDIPS")
		, m_pen_switches(*this, "PENSW")
		, m_tablet_cpu(*this, "tablet")
		, m_pen_prox(*this, "PENPROX")
		, m_pen_x(*this, "PENX")
		, m_pen_y(*this, "PENY")
		, m_pen_press(*this, "PENPRESS")
		, m_brushproc_prom(*this, "brushproc_prom")
		, m_brushproc_pal(*this, "brushproc_pal")
	{
	}

	void dpb7000(machine_config &config);

	//DECLARE_INPUT_CHANGED_MEMBER(pen_prox_changed);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	template <int StoreNum> uint32_t store_debug_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t stencil_debug_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t brush_debug_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t combined_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void fddcpu_map(address_map &map) ATTR_COLD;
	void keybcpu_map(address_map &map) ATTR_COLD;
	void tds_cpu_map(address_map &map) ATTR_COLD;
	void tablet_program_map(address_map &map) ATTR_COLD;
	void tablet_data_map(address_map &map) ATTR_COLD;

	void csr_w(uint8_t data);
	uint8_t csr_r();

	uint16_t cpu_ctrlbus_r();
	void cpu_ctrlbus_w(uint16_t data);

	TIMER_CALLBACK_MEMBER(req_a_w);
	TIMER_CALLBACK_MEMBER(req_b_w);
	TIMER_CALLBACK_MEMBER(cmd_done);
	TIMER_CALLBACK_MEMBER(execute_hdd_command);

	bool is_disk_group_fdd(int group_index);
	bool is_disk_group_hdd(int group_index);
	int get_heads_for_disk_group(int group_index);
	void fdd_index_callback(floppy_image_device *floppy, int state);
	void seek_fdd_to_cylinder();
	uint8_t fdd_ctrl_r();
	uint8_t fdd_cmd_r();
	void fddcpu_p1_w(uint8_t data);
	uint8_t fddcpu_p2_r();
	void fddcpu_p2_w(uint8_t data);
	void fddcpu_debug_rx(int state);

	bool handle_command(uint16_t data);
	void store_address_w(uint8_t card, uint16_t data);
	void combiner_reg_w(uint16_t data);

	enum : uint16_t
	{
		SYSCTRL_AUTO_START      = 0x0001,
		SYSCTRL_REQ_B_EN        = 0x0020,
		SYSCTRL_REQ_A_EN        = 0x0040,
		SYSCTRL_REQ_A_IN        = 0x0080,
		SYSCTRL_REQ_B_IN        = 0x8000
	};

	uint16_t cpu_sysctrl_r();
	void cpu_sysctrl_w(uint16_t data);
	void update_req_irqs();

	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_addr_changed);

	void advance_line_count();
	void toggle_line_clock();
	void process_sample();
	void process_byte_from_disc(uint8_t data_byte);
	uint8_t process_byte_to_disc();

	required_device<m68000_device> m_maincpu;
	required_device_array<acia6850_device, 3> m_acia;
	required_device<input_merger_device> m_p_int;
	required_device<com8116_device> m_brg;
	required_device_array<rs232_port_device, 2> m_rs232;
	required_device<sy6545_1_device> m_crtc;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint16_t> m_vdu_ram;
	required_region_ptr<uint8_t> m_vdu_char_rom;
	required_ioport m_baud_dip;
	required_ioport m_auto_start;

	required_ioport m_config_sw12;
	required_ioport m_config_sw34;

	required_device<m6803_cpu_device> m_fddcpu;
	required_device<rs232_port_device> m_fdd_serial;
	required_device<floppy_connector> m_floppy0;
	required_device<harddisk_image_device> m_hdd;

	// Floppy Drive
	floppy_image_device *m_floppy;
	std::deque<int> m_fdd_debug_rx_bits;
	uint32_t m_fdd_debug_rx_bit_count;
	uint32_t m_fdd_debug_rx_byte_count;
	uint32_t m_fdd_debug_rx_recv_count;
	uint8_t m_fdd_ctrl;
	uint8_t m_fdd_port1;
	uint8_t m_fdd_track;
	uint8_t m_fdd_side;
	fdc_pll_t m_fdd_pll;

	// Timers
	emu_timer *m_diskseq_complete_clk;
	emu_timer *m_field_in_clk;
	emu_timer *m_field_out_clk;
	emu_timer *m_cmd_done_timer;
	emu_timer *m_hdd_command_timer;

	enum : size_t
	{
		FRAMESTORE_A0 = 0,
		FRAMESTORE_A1,
		FRAMESTORE_A2,
		FRAMESTORE_B0,
		FRAMESTORE_B1,
		FRAMESTORE_B2,
		FRAMESTORE_COUNT
	};

	enum : uint16_t
	{
		DGROUP_TYPE_INVALID0        = 0,
		DGROUP_TYPE_80MB_CDC        = 1,
		DGROUP_TYPE_INVALID1        = 2,
		DGROUP_TYPE_FLOPPY          = 3,
		DGROUP_TYPE_INVALID2        = 4,
		DGROUP_TYPE_330MB_FUJITSU   = 5,
		DGROUP_TYPE_160MB_FUJITSU   = 6,
		DGROUP_TYPE_NONE            = 7,

		DGROUP_0_SHIFT              = 2,
		DGROUP_1_SHIFT              = 7,
		DGROUP_2_SHIFT              = 12
	};

	// Computer Card
	uint8_t m_csr;
	uint16_t m_sys_ctrl;

	// Disc Sequencer Card
	uint16_t m_diskseq_cyl_from_cpu;    // AE/BH
	uint16_t m_diskseq_cmd_word_from_cpu; // DD/CC
	uint8_t m_diskseq_cmd;
	uint8_t m_diskseq_cmd_to_ctrl;
	uint8_t m_diskseq_status;       // BC
	bool m_diskseq_cyl_read_pending;
	bool m_diskseq_cyl_write_pending;
	bool m_diskseq_use_hdd_pending;
	uint16_t m_diskseq_command_stride;

	// Disc Data Buffer Card
	uint16_t m_diskbuf_ram_addr;
	uint8_t m_diskbuf_ram[14 * 0x800];
	uint16_t m_diskbuf_data_count;

	// Output Timing Card
	uint16_t m_cursor_origin_x;
	uint16_t m_cursor_origin_y;
	uint16_t m_cursor_size_x;
	uint16_t m_cursor_size_y;
	uint8_t m_output_csflags;
	uint8_t m_output_cpflags;
	required_region_ptr<uint8_t> m_output_cursor;
	required_region_ptr<uint8_t> m_output_hlines;
	required_region_ptr<uint8_t> m_output_hflags;
	required_region_ptr<uint8_t> m_output_vlines;
	required_region_ptr<uint8_t> m_output_vflags;

	// Brush Address Card
	required_region_ptr<uint8_t> m_brushaddr_pal;
	uint8_t m_line_clock;
	uint16_t m_line_count;
	uint16_t m_line_length;
	uint16_t m_brush_addr_func;
	uint8_t m_brush_addr_cmd;
	uint8_t m_bif;
	uint8_t m_bixos;
	uint8_t m_biyos;
	uint16_t m_bxlen;
	uint16_t m_bylen;
	uint16_t m_bxlen_counter;
	uint16_t m_bylen_counter;
	uint8_t m_brush_press_lum;
	uint8_t m_brush_press_chr;
	std::unique_ptr<uint8_t[]> m_framestore_chr[2];
	std::unique_ptr<uint8_t[]> m_framestore_lum[2];
	std::unique_ptr<uint8_t[]> m_framestore_ext[2];

	// Brush Store Card
	uint8_t m_bs_y_latch;
	uint8_t m_bs_u_latch;
	uint8_t m_bs_v_latch;
	std::unique_ptr<uint8_t[]> m_brushstore_chr;
	std::unique_ptr<uint8_t[]> m_brushstore_lum;
	std::unique_ptr<uint8_t[]> m_brushstore_ext;

	// Store Address Card
	uint16_t m_rhscr[2];
	uint16_t m_rvscr[2];
	uint16_t m_rzoom[2];
	uint16_t m_fld_sel[2];
	uint16_t m_window_enable[2];
	uint16_t m_cxpos[2];
	uint16_t m_cypos[2];
	uint16_t m_ca0;
	required_region_ptr<uint8_t> m_storeaddr_protx;
	required_region_ptr<uint8_t> m_storeaddr_proty;
	required_region_ptr<uint8_t> m_storeaddr_xlnib;
	required_region_ptr<uint8_t> m_storeaddr_xmnib;
	required_region_ptr<uint8_t> m_storeaddr_xhnib;
	required_region_ptr<uint8_t> m_storeaddr_pal_blank;

	// Combiner Card
	uint8_t m_cursor_y;
	uint8_t m_cursor_u;
	uint8_t m_cursor_v;
	uint8_t m_ext_store_flags;
	bool m_select_matte[2];
	uint8_t m_matte_ext[2];
	uint8_t m_matte_y[2];
	uint8_t m_matte_u[2];
	uint8_t m_matte_v[2];

	// Keyboard
	required_device<i8039_device> m_keybcpu;
	required_ioport_array<8> m_keybc_cols;
	uint8_t keyboard_p1_r();
	uint8_t keyboard_p2_r();
	void keyboard_p1_w(uint8_t data);
	void keyboard_p2_w(uint8_t data);
	int keyboard_t0_r();
	int keyboard_t1_r();
	uint8_t m_keybc_latched_bit;
	uint8_t m_keybc_p1_data;
	uint8_t m_keybc_tx;

	// TDS Box
	enum tds_press_state : uint32_t
	{
		STATE_IDLE,
		STATE_PRESS_IN,
		STATE_PRESSED,
		STATE_PRESS_OUT
	};

	required_device<m6803_cpu_device> m_tds_cpu;
	required_device<scn2681_device> m_tds_duart;
	required_ioport m_tds_dips;
	required_ioport m_pen_switches;
	uint8_t tds_p1_r();
	uint8_t tds_p2_r();
	uint8_t tds_p3_r();
	uint8_t tds_p4_r();
	void tds_p1_w(uint8_t data);
	void tds_p2_w(uint8_t data);
	void tds_dac_w(uint8_t data);
	void tds_convert_w(uint8_t data);
	uint8_t tds_adc_r();
	uint8_t tds_pen_switches_r();
	void duart_b_w(int state);
	TIMER_CALLBACK_MEMBER(tablet_tx_tick);
	TIMER_CALLBACK_MEMBER(tds_adc_tick);
	TIMER_CALLBACK_MEMBER(tds_press_tick);
	uint8_t m_tds_dac_value;
	uint8_t m_tds_dac_offset;
	float m_tds_dac_percent;
	bool m_tds_adc_busy;
	uint8_t m_tds_adc_value;
	uint8_t m_tds_p1_data;
	uint32_t m_tds_press_state;
	uint8_t m_tds_pressure_shift;
	emu_timer *m_tds_adc_timer;
	emu_timer *m_tds_press_timer;
	bool m_tds_in_proximity;

	// Tablet
	required_device<z8681_device> m_tablet_cpu;
	required_ioport m_pen_prox;
	required_ioport m_pen_x;
	required_ioport m_pen_y;
	required_ioport m_pen_press;
	uint8_t tablet_p2_r(offs_t offset, uint8_t mem_mask);
	void tablet_p2_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t tablet_p3_r(offs_t offset, uint8_t mem_mask);
	void tablet_p3_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	TIMER_CALLBACK_MEMBER(tablet_irq_tick);
	uint8_t tablet_rdl_r();
	uint8_t tablet_rdh_r();
	uint8_t m_tablet_p2_data;
	uint8_t m_tablet_p3_data;
	uint8_t m_tablet_mux;
	uint8_t m_tablet_drq;
	uint16_t m_tablet_dip_shifter;
	uint16_t m_tablet_counter_latch;
	emu_timer *m_tablet_tx_timer;
	emu_timer *m_tablet_irq_timer;
	uint8_t m_tablet_tx_bit;
	uint16_t m_tablet_pen_x;
	uint16_t m_tablet_pen_y;
	uint16_t m_tablet_pen_switches;
	uint8_t m_tablet_state;

	// Table HLE
	TIMER_CALLBACK_MEMBER(tablet_hle_tick);
	std::deque<int> m_tablet_hle_tx_bits;
	emu_timer *m_tablet_hle_timer;

	// Filter Card
	int8_t m_filter_x_coeffs[12];
	int8_t m_filter_y_coeffs[8];
	uint8_t m_incoming_lum;
	uint8_t m_incoming_chr;
	bool m_buffer_lum;

	// Size Card
	uint32_t m_size_dsx_ddx;
	uint32_t m_size_dsy_ddx;
	uint32_t m_size_dsx_ddy;
	uint32_t m_size_dsy_ddy;
	uint8_t m_size_frac_origin_sx_dx;
	uint8_t m_size_frac_origin_sy_dx;
	uint8_t m_size_frac_origin_sx_dy;
	uint8_t m_size_frac_origin_sy_dy;
	int16_t m_size_dxdx_counter;
	int16_t m_size_dydy_counter;
	uint16_t m_size_dest_x;
	uint16_t m_size_dest_y;

	// Brush Processor Card
	required_region_ptr<uint8_t> m_brushproc_prom;
	required_region_ptr<uint16_t> m_brushproc_pal;

	// Utility
	std::unique_ptr<uint32_t[]> m_yuv_lut;
};

void dpb7000_state::main_map(address_map &map)
{
	map(0x000000, 0x09ffff).rom().region("monitor", 0);
	map(0x0006aa, 0x0006ab).nopw();
	map(0xb00000, 0xb7ffff).rw(m_maincpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
	map(0xb80000, 0xbfffff).ram();
	//map(0xb00000, 0xbfffff).rw(m_maincpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
	//map(0xfc0000, 0xffd3ff).ram();
	map(0xffd000, 0xffd3ff).rw(m_maincpu, FUNC(m68000_device::berr_r), FUNC(m68000_device::berr_w));
	map(0xffe000, 0xffefff).ram().share("vduram").umask16(0x00ff);
	map(0xfff801, 0xfff801).rw(m_crtc, FUNC(sy6545_1_device::status_r), FUNC(sy6545_1_device::address_w)).cswidth(16);
	map(0xfff803, 0xfff803).rw(m_crtc, FUNC(sy6545_1_device::register_r), FUNC(sy6545_1_device::register_w)).cswidth(16);
	map(0xfff805, 0xfff805).rw(m_acia[0], FUNC(acia6850_device::status_r), FUNC(acia6850_device::control_w)).cswidth(16);
	map(0xfff807, 0xfff807).rw(m_acia[0], FUNC(acia6850_device::data_r), FUNC(acia6850_device::data_w)).cswidth(16);
	map(0xfff808, 0xfff808).r(FUNC(dpb7000_state::csr_r)).cswidth(16);
	map(0xfff809, 0xfff809).w(FUNC(dpb7000_state::csr_w)).cswidth(16);
	map(0xfff80a, 0xfff80b).rw(FUNC(dpb7000_state::cpu_ctrlbus_r), FUNC(dpb7000_state::cpu_ctrlbus_w));
	map(0xfff80c, 0xfff80d).rw(FUNC(dpb7000_state::cpu_sysctrl_r), FUNC(dpb7000_state::cpu_sysctrl_w));
	map(0xfff811, 0xfff811).rw(m_acia[1], FUNC(acia6850_device::status_r), FUNC(acia6850_device::control_w)).cswidth(16);
	map(0xfff813, 0xfff813).rw(m_acia[1], FUNC(acia6850_device::data_r), FUNC(acia6850_device::data_w)).cswidth(16);
	map(0xfff815, 0xfff815).rw(m_acia[2], FUNC(acia6850_device::status_r), FUNC(acia6850_device::control_w)).cswidth(16);
	map(0xfff817, 0xfff817).rw(m_acia[2], FUNC(acia6850_device::data_r), FUNC(acia6850_device::data_w)).cswidth(16);
}

void dpb7000_state::fddcpu_map(address_map &map)
{
	map(0x4000, 0x4000).r(FUNC(dpb7000_state::fdd_ctrl_r));
	map(0x4001, 0x4001).r(FUNC(dpb7000_state::fdd_cmd_r));
	map(0xf800, 0xffff).rom().region("fddprom", 0);
}

void dpb7000_state::keybcpu_map(address_map &map)
{
	map(0x000, 0x7ff).rom().region("keyboard", 0);
}

void dpb7000_state::tds_cpu_map(address_map &map)
{
	map(0x2000, 0x2000).w(FUNC(dpb7000_state::tds_dac_w));
	map(0x2001, 0x2001).w(FUNC(dpb7000_state::tds_convert_w));
	map(0x2003, 0x2003).r(FUNC(dpb7000_state::tds_adc_r));
	map(0x2007, 0x2007).r(FUNC(dpb7000_state::tds_pen_switches_r));
	map(0x4000, 0x400f).rw(m_tds_duart, FUNC(scn2681_device::read), FUNC(scn2681_device::write));
	map(0xe000, 0xffff).rom().region("tds", 0);
}

void dpb7000_state::tablet_program_map(address_map &map)
{
	map(0x0000, 0x1fff).mirror(0xe000).rom().region("tablet", 0);
}

void dpb7000_state::tablet_data_map(address_map &map)
{
	map(0x4000, 0x4000).mirror(0x1fff).r(FUNC(dpb7000_state::tablet_rdl_r));
	map(0x6000, 0x6000).mirror(0x1fff).r(FUNC(dpb7000_state::tablet_rdh_r));
}

static INPUT_PORTS_START( dpb7000 )
	PORT_START("KEYB_COL0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I) PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M) PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"9  )  \u2013") PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')') // – (en dash)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"8  (  \u2014") PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(') // — (em dash)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')

	PORT_START("KEYB_COL1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"7  \u2019  \u2018") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'') // ’ ‘
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"6  &  »") PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("KEYB_COL2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5  %  >") PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%') PORT_CHAR('>')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4  $  <") PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$') PORT_CHAR('<')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')

	PORT_START("KEYB_COL3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8".  :  æ") PORT_CODE(KEYCODE_STOP) PORT_CHAR ('.') PORT_CHAR(':')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"¹  =  ~") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('=')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8",  ;") PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR(';')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('-')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\ufb02  +  Æ") PORT_CODE(KEYCODE_COLON) // ﬂ
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')

	PORT_START("KEYB_COL4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"3  £  «") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2  \"") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')

	PORT_START("KEYB_COL5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!') PORT_CHAR('#')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEYB_COL6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"`  `  Å") PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8" \ua788  ˆ  ø") PORT_CODE(KEYCODE_EQUALS) // modifier letter low circumflex accent - leave the preceding space
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"/ ?  œ") PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8" \u0324  ¨  Ø") PORT_CODE(KEYCODE_TILDE) PORT_CHAR('~') // combining diaeresis below - leave the preceding space
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"ß  ¸  å") PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"´  ´  ~") PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(u8"\ufb01  *  ϒ") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('*') // ﬁ

	PORT_START("KEYB_COL7")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) // 0x7f
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Delete") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(0x08) // 0x08
	PORT_BIT(0x30, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(0x0d)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("PENSW")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Pen Button 1") PORT_CODE(MOUSECODE_BUTTON1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Pen Button 2") PORT_CODE(MOUSECODE_BUTTON3)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Pen Button 3") PORT_CODE(MOUSECODE_BUTTON4)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Pen Button 4") PORT_CODE(MOUSECODE_BUTTON5)
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("PENPROX")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Pen Proximity") PORT_CODE(MOUSECODE_BUTTON2)
	PORT_BIT(0xfe, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("PENX")
	PORT_BIT( 0xffff, 3800, IPT_LIGHTGUN_X) PORT_NAME("Pen X") PORT_MINMAX(500, 4599) PORT_SENSITIVITY(50) PORT_CROSSHAIR(X, 1.385, -0.17, 0)

	PORT_START("PENY")
	PORT_BIT( 0xffff, 2048, IPT_LIGHTGUN_Y) PORT_NAME("Pen Y") PORT_MINMAX(1000, 4399) PORT_SENSITIVITY(50) PORT_CROSSHAIR(Y, 1.53, -0.26, 0)

	PORT_START("PENPRESS")
	PORT_BIT( 0xff, 0x02, IPT_PEDAL ) PORT_MINMAX(0x02,0xaf) PORT_SENSITIVITY(100) PORT_KEYDELTA(10)

	PORT_START("TDSDIPS")
	PORT_DIPNAME(0x08, 0x08, "TDS Box Encoding")
	PORT_DIPSETTING(   0x08, "Binary")
	PORT_DIPSETTING(   0x00, "ASCII")
	PORT_DIPNAME(0x10, 0x10, "TDS Box Mode")
	PORT_DIPSETTING(   0x10, "Normal")
	PORT_DIPSETTING(   0x00, "Monitor")
	PORT_DIPNAME(0x40, 0x00, "TDS Box Standard")
	PORT_DIPSETTING(   0x40, "NTSC")
	PORT_DIPSETTING(   0x00, "PAL")
	PORT_BIT(0xa7, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("BAUD")
	PORT_DIPNAME(0x0f, 0x0e, "Baud Rate for Terminal")
	PORT_DIPSETTING(   0x00, "50")
	PORT_DIPSETTING(   0x01, "75")
	PORT_DIPSETTING(   0x02, "110")
	PORT_DIPSETTING(   0x03, "134.5")
	PORT_DIPSETTING(   0x04, "150")
	PORT_DIPSETTING(   0x05, "300")
	PORT_DIPSETTING(   0x06, "600")
	PORT_DIPSETTING(   0x07, "1200")
	PORT_DIPSETTING(   0x08, "1800")
	PORT_DIPSETTING(   0x09, "2000")
	PORT_DIPSETTING(   0x0a, "2400")
	PORT_DIPSETTING(   0x0b, "3600")
	PORT_DIPSETTING(   0x0c, "4800")
	PORT_DIPSETTING(   0x0e, "9600")
	PORT_DIPSETTING(   0x0f, "19200")

	PORT_START("AUTOSTART")
	PORT_DIPNAME(0x0001, 0x0001, "Auto-Start")
	PORT_DIPSETTING(     0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(     0x0001, DEF_STR( On ))
	PORT_BIT(0xfffe, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("CONFIGSW12")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0000, "NTSC/PAL" )
	PORT_DIPSETTING(    0x0002, "NTSC" )
	PORT_DIPSETTING(    0x0000, "PAL" )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Diagnostic Mode" )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_START("CONFIGSW34")
	PORT_DIPNAME( 0x0003, 0x0003, "Disc Group 0 Count" )
	PORT_DIPSETTING(    0x0003, "One" )
	PORT_DIPSETTING(    0x0002, "Two" )
	PORT_DIPSETTING(    0x0001, "Three" )
	PORT_DIPSETTING(    0x0000, "Four" )
	PORT_DIPNAME( 0x001c, 0x000c, "Disc Group 0 Type" )
	PORT_DIPSETTING(    0x001c, "None" )
	PORT_DIPSETTING(    0x0018, "160Mb Fujitsu" )
	PORT_DIPSETTING(    0x0014, "330Mb Fujitsu" )
	PORT_DIPSETTING(    0x000c, "Floppy Disc" )
	PORT_DIPSETTING(    0x0008, "NTSC/Floppy/PAL/Conv" )
	PORT_DIPSETTING(    0x0004, "80Mb CDC Removable" )
	PORT_DIPSETTING(    0x0000, "160Mb CDC Fixed" )
	PORT_DIPNAME( 0x0060, 0x0060, "Disc Group 1 Count" )
	PORT_DIPSETTING(    0x0060, "One" )
	PORT_DIPSETTING(    0x0040, "Two" )
	PORT_DIPSETTING(    0x0020, "Three" )
	PORT_DIPSETTING(    0x0000, "Four" )
	PORT_DIPNAME( 0x0380, 0x0380, "Disc Group 1 Type" )
	PORT_DIPSETTING(    0x0380, "None" )
	PORT_DIPSETTING(    0x0300, "160Mb Fujitsu" )
	PORT_DIPSETTING(    0x0280, "330Mb Fujitsu" )
	PORT_DIPSETTING(    0x0180, "Floppy Disc" )
	PORT_DIPSETTING(    0x0100, "NTSC/Floppy/PAL/Conv" )
	PORT_DIPSETTING(    0x0080, "80Mb CDC Removable" )
	PORT_DIPSETTING(    0x0000, "160Mb CDC Fixed" )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Disc Group 2 Count" )
	PORT_DIPSETTING(    0x0c00, "One" )
	PORT_DIPSETTING(    0x0800, "Two" )
	PORT_DIPSETTING(    0x0400, "Three" )
	PORT_DIPSETTING(    0x0000, "Four" )
	PORT_DIPNAME( 0x7000, 0x7000, "Disc Group 2 Type" )
	PORT_DIPSETTING(    0x7000, "None" )
	PORT_DIPSETTING(    0x6000, "160Mb Fujitsu" )
	PORT_DIPSETTING(    0x5000, "330Mb Fujitsu" )
	PORT_DIPSETTING(    0x3000, "Floppy Disc" )
	PORT_DIPSETTING(    0x2000, "NTSC/Floppy/PAL/Conv" )
	PORT_DIPSETTING(    0x1000, "80Mb CDC Removable" )
	PORT_DIPSETTING(    0x0000, "160Mb CDC Fixed" )
	PORT_DIPNAME( 0x8000, 0x8000, "Start Up In Dialogue" )
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
INPUT_PORTS_END

void dpb7000_state::machine_start()
{
	// Computer Card
	save_item(NAME(m_csr));
	save_item(NAME(m_sys_ctrl));

	m_field_in_clk = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(dpb7000_state::req_a_w), this));
	m_field_in_clk->adjust(attotime::never);

	m_field_out_clk = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(dpb7000_state::req_a_w), this));
	m_field_out_clk->adjust(attotime::never);

	m_cmd_done_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(dpb7000_state::cmd_done), this));
	m_cmd_done_timer->adjust(attotime::never);

	m_hdd_command_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(dpb7000_state::execute_hdd_command), this));
	m_hdd_command_timer->adjust(attotime::never);

	// Disc Sequencer Card
	save_item(NAME(m_diskseq_cyl_from_cpu));
	save_item(NAME(m_diskseq_cmd_word_from_cpu));
	save_item(NAME(m_diskseq_cmd));
	save_item(NAME(m_diskseq_cmd_to_ctrl));
	save_item(NAME(m_diskseq_status));
	save_item(NAME(m_diskseq_cyl_read_pending));
	save_item(NAME(m_diskseq_cyl_write_pending));
	save_item(NAME(m_diskseq_use_hdd_pending));
	save_item(NAME(m_diskseq_command_stride));
	m_diskseq_complete_clk = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(dpb7000_state::req_b_w), this));
	m_diskseq_complete_clk->adjust(attotime::never);

	// Floppy Disc Controller Card
	save_item(NAME(m_fdd_debug_rx_bit_count));
	save_item(NAME(m_fdd_debug_rx_byte_count));
	save_item(NAME(m_fdd_debug_rx_recv_count));
	save_item(NAME(m_fdd_ctrl));
	save_item(NAME(m_fdd_port1));
	save_item(NAME(m_fdd_track));
	save_item(NAME(m_fdd_side));

	// Disc Data Buffer Card
	save_item(NAME(m_diskbuf_ram_addr));
	save_item(NAME(m_diskbuf_ram));
	save_item(NAME(m_diskbuf_data_count));

	// Output Timing Card
	save_item(NAME(m_cursor_origin_x));
	save_item(NAME(m_cursor_origin_y));
	save_item(NAME(m_cursor_size_x));
	save_item(NAME(m_cursor_size_y));
	save_item(NAME(m_output_csflags));
	save_item(NAME(m_output_cpflags));

	// Brush Address Card
	save_item(NAME(m_line_clock));
	save_item(NAME(m_line_count));
	save_item(NAME(m_line_length));
	save_item(NAME(m_brush_addr_func));
	save_item(NAME(m_brush_addr_cmd));
	save_item(NAME(m_bif));
	save_item(NAME(m_bixos));
	save_item(NAME(m_biyos));
	save_item(NAME(m_bxlen));
	save_item(NAME(m_bylen));
	save_item(NAME(m_bxlen_counter));
	save_item(NAME(m_bylen_counter));
	save_item(NAME(m_brush_press_lum));
	save_item(NAME(m_brush_press_chr));

	// Frame Store Cards, 640x1024
	for (int i = 0; i < 2; i++)
	{
		m_framestore_chr[i] = std::make_unique<uint8_t[]>(10 * 0x10000);
		m_framestore_lum[i] = std::make_unique<uint8_t[]>(10 * 0x10000);
		m_framestore_ext[i] = std::make_unique<uint8_t[]>(10 * 0x10000);
	}
	save_pointer(&m_framestore_chr[0][0], "m_framestore_chr[0]", 10 * 0x10000);
	save_pointer(&m_framestore_lum[0][0], "m_framestore_lum[0]", 10 * 0x10000);
	save_pointer(&m_framestore_ext[0][0], "m_framestore_ext[0]", 10 * 0x10000);
	save_pointer(&m_framestore_chr[1][0], "m_framestore_chr[1]", 10 * 0x10000);
	save_pointer(&m_framestore_lum[1][0], "m_framestore_lum[1]", 10 * 0x10000);
	save_pointer(&m_framestore_ext[1][0], "m_framestore_ext[1]", 10 * 0x10000);

	// Brush Store Card
	save_item(NAME(m_bs_y_latch));
	save_item(NAME(m_bs_u_latch));
	save_item(NAME(m_bs_v_latch));
	m_brushstore_chr = std::make_unique<uint8_t[]>(0x10000);
	m_brushstore_lum = std::make_unique<uint8_t[]>(0x10000);
	m_brushstore_ext = std::make_unique<uint8_t[]>(0x10000);
	save_pointer(&m_brushstore_chr[0], "m_brushstore_chr", 0x10000);
	save_pointer(&m_brushstore_lum[0], "m_brushstore_lum", 0x10000);
	save_pointer(&m_brushstore_ext[0], "m_brushstore_ext", 0x10000);

	// Store Address Cards
	save_item(NAME(m_rhscr));
	save_item(NAME(m_rvscr));
	save_item(NAME(m_rzoom));
	save_item(NAME(m_fld_sel));
	save_item(NAME(m_window_enable));
	save_item(NAME(m_cxpos));
	save_item(NAME(m_cypos));
	save_item(NAME(m_ca0));

	// Combiner Card
	save_item(NAME(m_cursor_y));
	save_item(NAME(m_cursor_u));
	save_item(NAME(m_cursor_v));
	save_item(NAME(m_ext_store_flags));
	save_item(NAME(m_select_matte));
	save_item(NAME(m_matte_ext));
	save_item(NAME(m_matte_y));
	save_item(NAME(m_matte_u));
	save_item(NAME(m_matte_v));

	// Size Card
	save_item(NAME(m_size_dsx_ddx));
	save_item(NAME(m_size_dsy_ddx));
	save_item(NAME(m_size_dsx_ddy));
	save_item(NAME(m_size_dsy_ddy));
	save_item(NAME(m_size_frac_origin_sx_dx));
	save_item(NAME(m_size_frac_origin_sy_dx));
	save_item(NAME(m_size_frac_origin_sx_dy));
	save_item(NAME(m_size_frac_origin_sy_dy));
	save_item(NAME(m_size_dxdx_counter));
	save_item(NAME(m_size_dydy_counter));
	save_item(NAME(m_size_dest_x));
	save_item(NAME(m_size_dest_y));

	// Filter Card
	save_item(NAME(m_filter_x_coeffs));
	save_item(NAME(m_filter_y_coeffs));
	save_item(NAME(m_incoming_lum));
	save_item(NAME(m_incoming_chr));
	save_item(NAME(m_buffer_lum));

	// Keyboard
	save_item(NAME(m_keybc_latched_bit));
	save_item(NAME(m_keybc_p1_data));
	save_item(NAME(m_keybc_tx));

	// TDS Box
	save_item(NAME(m_tds_dac_value));
	save_item(NAME(m_tds_dac_offset));
	save_item(NAME(m_tds_dac_percent));
	save_item(NAME(m_tds_adc_busy));
	save_item(NAME(m_tds_adc_value));
	save_item(NAME(m_tds_press_state));
	save_item(NAME(m_tds_pressure_shift));
	save_item(NAME(m_tds_in_proximity));
	m_tds_adc_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(dpb7000_state::tds_adc_tick), this));
	m_tds_adc_timer->adjust(attotime::never);
	m_tds_press_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(dpb7000_state::tds_press_tick), this));
	m_tds_press_timer->adjust(attotime::never);

	// Tablet
	save_item(NAME(m_tablet_p2_data));
	save_item(NAME(m_tablet_p3_data));
	save_item(NAME(m_tablet_dip_shifter));
	save_item(NAME(m_tablet_counter_latch));
	save_item(NAME(m_tablet_tx_bit));
	save_item(NAME(m_tablet_pen_x));
	save_item(NAME(m_tablet_pen_y));
	save_item(NAME(m_tablet_pen_switches));
	save_item(NAME(m_tablet_state));
	m_tablet_tx_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(dpb7000_state::tablet_tx_tick), this));
	m_tablet_tx_timer->adjust(attotime::never);
	m_tablet_irq_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(dpb7000_state::tablet_irq_tick), this));
	m_tablet_irq_timer->adjust(attotime::never);

	// Tablet HLE
	m_tablet_hle_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(dpb7000_state::tablet_hle_tick), this));
	m_tablet_hle_timer->adjust(attotime::never);

	m_yuv_lut = std::make_unique<uint32_t[]>(0x1000000);
	for (uint16_t u = 0; u < 256; u++)
	{
		for (uint16_t v = 0; v < 256; v++)
		{
			for (uint16_t y = 0; y < 256; y++)
			{
				float fr = y + 1.4075f * (v - 128);
				float fg = y - 0.3455f * (u - 128) - (0.7169f * (v - 128));
				float fb = y + 1.7790f * (u - 128);
				uint8_t r = (fr < 0.0f) ? 0 : (fr > 255.0f ? 255 : (uint8_t)fr);
				uint8_t g = (fg < 0.0f) ? 0 : (fg > 255.0f ? 255 : (uint8_t)fg);
				uint8_t b = (fb < 0.0f) ? 0 : (fb > 255.0f ? 255 : (uint8_t)fb);
				m_yuv_lut[(u << 16) | (v << 8) | y] = 0xff000000 | (r << 16) | (g << 8) | b;
			}
		}
	}
}

void dpb7000_state::machine_reset()
{
	m_cmd_done_timer->adjust(attotime::never);

	// Computer Card
	m_brg->stt_w(m_baud_dip->read());
	m_csr = 0;
	m_sys_ctrl = 0;
	for (int i = 0; i < 3; i++)
	{
		m_acia[i]->write_cts(0);
		m_acia[i]->write_dcd(0);
	}

	m_field_in_clk->adjust(attotime::from_hz(59.94), 0, attotime::from_hz(59.94));
	m_field_out_clk->adjust(attotime::from_hz(59.94) + attotime::from_hz(15734.0 / 1.0), 1, attotime::from_hz(59.94));

	// Disc Sequencer Card
	m_diskseq_cyl_from_cpu = 0;
	m_diskseq_cmd_word_from_cpu = 0;
	m_diskseq_cmd = 0;
	m_diskseq_cmd_to_ctrl = 0;
	m_diskseq_status = 0xf9;
	m_diskseq_cyl_read_pending = false;
	m_diskseq_cyl_write_pending = false;
	m_diskseq_use_hdd_pending = false;
	m_diskseq_command_stride = 1;
	m_diskseq_complete_clk->adjust(attotime::never);
	m_hdd_command_timer->adjust(attotime::never);

	// Floppy Disc Controller Card
	m_fdd_debug_rx_bits.clear();
	m_fdd_debug_rx_bit_count = 0;
	m_fdd_debug_rx_byte_count = 0;
	m_fdd_debug_rx_recv_count = 0;
	m_fdd_ctrl = 0;
	m_fdd_port1 = 0;
	m_fdd_track = 20;
	m_fdd_side = 0;
	m_fdd_pll.set_clock(attotime::from_hz(1000000));
	m_fdd_pll.reset(machine().time());
	m_floppy = nullptr;

	// Disc Data Buffer Card
	m_diskbuf_ram_addr = 0;
	memset(m_diskbuf_ram, 0, 14 * 0x800);
	m_diskbuf_data_count = 0;

	// Output Timing Card
	m_cursor_origin_x = 0;
	m_cursor_origin_y = 0;
	m_cursor_size_x = 0;
	m_cursor_size_y = 0;
	m_output_csflags = 0;
	m_output_cpflags = 0;

	// Brush Address Card
	m_line_clock = 0;
	m_line_count = 0;
	m_line_length = 0;
	m_brush_addr_func = 0;
	m_brush_addr_cmd = 0;
	m_bif = 0;
	m_bixos = 0;
	m_biyos = 0;
	m_bxlen = 0;
	m_bylen = 0;
	m_bxlen_counter = 0;
	m_bylen_counter = 0;
	m_brush_press_lum = 0;
	m_brush_press_chr = 0;

	// Frame Store Cards, 640x1024
	for (int i = 0; i < 2; i++)
	{
		memset(&m_framestore_chr[i][0], 0, 10 * 0x10000);
		memset(&m_framestore_lum[i][0], 0, 10 * 0x10000);
		memset(&m_framestore_ext[i][0], 0, 10 * 0x10000);
	}

	// Brush Store Card
	m_bs_y_latch = 0;
	m_bs_u_latch = 0;
	m_bs_v_latch = 0;
	memset(&m_brushstore_chr[0], 0, 0x10000);
	memset(&m_brushstore_lum[0], 0, 0x10000);
	memset(&m_brushstore_ext[0], 0, 0x10000);

	// Store Address Card
	memset(m_rhscr, 0, sizeof(m_rhscr));
	memset(m_rvscr, 0, sizeof(m_rvscr));
	memset(m_rzoom, 0, sizeof(m_rzoom));
	memset(m_fld_sel, 0, sizeof(m_fld_sel));
	memset(m_window_enable, 0, sizeof(m_window_enable));
	memset(m_cxpos, 0, sizeof(m_cxpos));
	memset(m_cypos, 0, sizeof(m_cypos));
	m_ca0 = 0;

	// Combiner Card
	m_cursor_y = 0;
	m_cursor_u = 0;
	m_cursor_v = 0;
	m_ext_store_flags = 0;
	memset(m_select_matte, 0, 2);
	memset(m_matte_ext, 0, 2);
	memset(m_matte_y, 0, 2);
	memset(m_matte_u, 0, 2);
	memset(m_matte_v, 0, 2);

	// Filter Card
	memset(m_filter_x_coeffs, 0, sizeof(m_filter_x_coeffs));
	memset(m_filter_y_coeffs, 0, sizeof(m_filter_y_coeffs));
	m_incoming_lum = 0;
	m_incoming_chr = 0;
	m_buffer_lum = true;

	// Size Card
	m_size_dsx_ddx = 0;
	m_size_dsy_ddx = 0;
	m_size_dsx_ddy = 0;
	m_size_dsy_ddy = 0;
	m_size_frac_origin_sx_dx = 0;
	m_size_frac_origin_sy_dx = 0;
	m_size_frac_origin_sx_dy = 0;
	m_size_frac_origin_sy_dy = 0;
	m_size_dxdx_counter = 0;
	m_size_dydy_counter = 0;
	m_size_dest_x = 0;
	m_size_dest_y = 0;

	// Keyboard
	m_keybc_latched_bit = 1;
	m_keybc_p1_data = 0;
	m_keybc_tx = 0;

	// TDS Box
	m_tds_dac_value = 0;
	m_tds_dac_offset = 0;
	m_tds_dac_percent = 0.0f;
	m_tds_adc_busy = false;
	m_tds_adc_value = 0x00;
	m_tds_pressure_shift = 0;
	m_tds_in_proximity = false;
	m_tds_press_state = STATE_IDLE;
	m_tds_adc_timer->adjust(attotime::never);
	m_tds_press_timer->adjust(attotime::from_hz(120), 0, attotime::from_hz(120));

	// Tablet
	m_tablet_p2_data = 0;
	m_tablet_p3_data = 0;
	m_tablet_dip_shifter = 0;
	m_tablet_mux = 0;
	m_tablet_drq = 0;
	m_tablet_counter_latch = 0;
	m_tablet_tx_timer->adjust(attotime::from_hz(9600), 0, attotime::from_hz(9600));
	m_tablet_irq_timer->adjust(attotime::never);
	m_tablet_tx_bit = 1;
	m_tablet_pen_x = 0;
	m_tablet_pen_y = 0;
	m_tablet_pen_switches = 0;
	m_tablet_state = 0;

	// Tablet HLE
	m_tablet_hle_tx_bits.clear();
	m_tablet_hle_timer->adjust(attotime::from_hz(40), 0, attotime::from_hz(40));
}

MC6845_UPDATE_ROW(dpb7000_state::crtc_update_row)
{
	const pen_t *const pen = m_palette->pens();

	for (int column = 0; column < x_count; column++)
	{
		uint8_t code = (uint8_t)m_vdu_ram[((ma + column) & 0x7ff)];
		uint16_t addr = code << 4 | (ra & 0x0f);
		uint8_t data = m_vdu_char_rom[addr & 0xfff];

		if (column == cursor_x)
		{
			data = 0xff;
		}

		for (int bit = 0; bit < 8; bit++)
		{
			int x = (column * 8) + bit;
			int color = BIT(data, 7) && de;

			bitmap.pix(y, x) = pen[color];

			data <<= 1;
		}
	}
}

MC6845_ON_UPDATE_ADDR_CHANGED(dpb7000_state::crtc_addr_changed)
{
}

void dpb7000_state::csr_w(uint8_t data)
{
	LOGMASKED(LOG_CSR, "%s: Card Select write: %02x (%04x)\n", machine().describe_context(), data & 0x0f, data);
	m_csr = data & 0x0f;
}

uint8_t dpb7000_state::csr_r()
{
	LOGMASKED(LOG_CSR, "%s: Card Select read(?): %02x\n", machine().describe_context(), m_csr);
	return m_csr;
}

uint16_t dpb7000_state::cpu_ctrlbus_r()
{
	uint16_t ret = 0;
	switch (m_csr)
	{
	case 0:
		if (!machine().side_effects_disabled())
			LOGMASKED(LOG_CTRLBUS, "%s: CPU read from Control Bus, Brush Address Card status: %04x\n", machine().describe_context(), m_brush_addr_func);
		ret = m_brush_addr_func;
		break;
	case 1:
		if (!machine().side_effects_disabled())
		{
			LOGMASKED(LOG_CTRLBUS, "%s: CPU read from Control Bus, Disk Sequencer Card status: %02x\n", machine().describe_context(), m_diskseq_status);
		}
		ret = m_diskseq_status;
		break;
	case 7:
		ret = m_diskbuf_ram[m_diskbuf_ram_addr];
		LOGMASKED(LOG_DDB, "%s: CPU read from Control Bus, Disc Data Buffer Card RAM read: %04x = %02x\n", machine().describe_context(), m_diskbuf_ram_addr, ret);
		m_diskbuf_ram_addr++;
		break;
	case 8:
	{
		if (m_brush_addr_cmd == 3) // Framestore Read
		{
			const uint16_t x = m_cxpos[1] + (m_bxlen_counter - m_bxlen);
			const uint16_t y = m_cypos[1] + (m_bylen_counter - m_bylen);

			uint32_t pix_idx = y * 800 + x;
			uint8_t &chr = m_ca0 ? m_bs_v_latch : m_bs_u_latch;
			if (!BIT(m_brush_addr_func, 5))
			{
				if (BIT(m_brush_addr_func, 7))
					m_bs_y_latch = m_framestore_lum[0][pix_idx];
				if (BIT(m_brush_addr_func, 8))
					chr = m_framestore_chr[0][pix_idx];
				if (BIT(m_brush_addr_func, 9))
					m_bs_y_latch = m_framestore_ext[0][pix_idx];
			}
			if (!BIT(m_brush_addr_func, 6))
			{
				if (BIT(m_brush_addr_func, 7))
					m_bs_y_latch = m_framestore_lum[1][pix_idx];
				if (BIT(m_brush_addr_func, 8))
					chr = m_framestore_chr[1][pix_idx];
				if (BIT(m_brush_addr_func, 9))
					m_bs_y_latch = m_framestore_ext[1][pix_idx];
			}

			m_bxlen_counter++;
			if (m_bxlen_counter == 0x1000)
			{
				m_bxlen_counter = m_bxlen;
				m_bylen_counter++;
				if (m_bylen_counter == 0x1000)
				{
					cmd_done(0);
				}
			}
		}

		uint16_t data = (uint16_t)m_bs_y_latch;
		if (m_ca0)
		{
			data |= (uint16_t)m_bs_v_latch << 8;
			LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR | LOG_BRUSH_LATCH, "%s: Brush Store Card color latches read (VY): %04x\n\n", machine().describe_context(), data);
		}
		else
		{
			data |= (uint16_t)m_bs_u_latch << 8;
			LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR | LOG_BRUSH_LATCH, "%s: Brush Store Card color latches read (UY): %04x\n", machine().describe_context(), data);
		}
		m_ca0 = 1 - m_ca0;
		return data;
	}
	case 12:
		ret = m_config_sw34->read();
		if (!machine().side_effects_disabled())
			LOGMASKED(LOG_CTRLBUS, "%s: CPU read from Control Bus, Config Switches 3/4: %04x\n", machine().describe_context(), ret);
		break;
	case 14:
		ret = m_config_sw12->read();
		if (!machine().side_effects_disabled())
			LOGMASKED(LOG_CTRLBUS, "%s: CPU read from Control Bus, Config Switches 1/2: %04x\n", machine().describe_context(), ret);
		break;
	default:
		if (!machine().side_effects_disabled())
			LOGMASKED(LOG_CTRLBUS | LOG_UNKNOWN, "%s: CPU read from Control Bus, unknown CSR %d\n", machine().describe_context(), m_csr);
		break;
	}
	return ret;
}

void dpb7000_state::store_address_w(uint8_t card, uint16_t data)
{
	switch ((data >> 12) & 7)
	{
	case 0:
		LOGMASKED(LOG_STORE_ADDR, "%s: Store Address Card %d, set RHSCR: %03x\n", machine().describe_context(), card + 1, data & 0xfff);
		m_rhscr[card] = data & 0xfff;
		if (!card)
		{
			LOGMASKED(LOG_STORE_ADDR, "%s: Setting Store Address Card 2 RHSCR also: %03x\n", machine().describe_context(), data & 0xfff);
			m_rhscr[1] = data & 0xfff;
		}
		break;
	case 1:
		LOGMASKED(LOG_STORE_ADDR, "%s: Store Address Card %d, set RVSCR: %03x\n", machine().describe_context(), card + 1, data & 0xfff);
		m_rvscr[card] = data & 0xfff;
		if (!card)
		{
			LOGMASKED(LOG_STORE_ADDR, "%s: Setting Store Address Card 2 RVSCR also: %03x\n", machine().describe_context(), data & 0xfff);
			m_rvscr[1] = data & 0xfff;
		}
		break;
	case 2:
		LOGMASKED(LOG_STORE_ADDR, "%s: Store Address Card %d, set R ZOOM: %03x\n", machine().describe_context(), card + 1, data & 0xfff);
		m_rzoom[card] = data & 0xf;
		break;
	case 3:
		LOGMASKED(LOG_STORE_ADDR, "%s: Store Address Card %d, set FLDSEL: %03x\n", machine().describe_context(), card + 1, data & 0xfff);
		m_fld_sel[card] = data & 0xf;
		m_window_enable[card] = BIT(m_fld_sel[card], 2);
		break;
	case 4:
		LOGMASKED(LOG_STORE_ADDR, "%s: Store Address Card %d, set CXPOS: %03x\n", machine().describe_context(), card + 1, data & 0xfff);
		m_cxpos[0] = data & 0xfff;
		m_cxpos[1] = data & 0xfff;
		//m_ca0 = data & 1;
		break;
	case 5:
		LOGMASKED(LOG_STORE_ADDR, "%s: Store Address Card %d, set CYPOS: %03x\n", machine().describe_context(), card + 1, data & 0xfff);
		m_cypos[0] = data & 0xfff;
		m_cypos[1] = data & 0xfff;
		break;
	default:
		LOGMASKED(LOG_STORE_ADDR, "%s: Store Address Card %d, unknown register: %04x\n", machine().describe_context(), card + 1, data);
		break;
	}
}

void dpb7000_state::combiner_reg_w(uint16_t data)
{
	const uint8_t reg_idx = (data >> 10) & 0xf;
	switch (reg_idx)
	{
		case 0: // Output Timing CS Flags
			LOGMASKED(LOG_OUTPUT_TIMING, "%s: Output Timing Card CS Register write: %02x\n", machine().describe_context(), (uint8_t)data);
			m_output_csflags = (uint8_t)(data & 0x0003);
			break;
		case 1: // CY
			LOGMASKED(LOG_COMBINER, "%s: Combiner Card Constant Y write: %02x\n", machine().describe_context(), (uint8_t)data);
			m_cursor_y = (uint8_t)data;
			break;
		case 2: // CV
			LOGMASKED(LOG_COMBINER, "%s: Combiner Card Constant V write: %02x\n", machine().describe_context(), (uint8_t)data);
			m_cursor_v = (uint8_t)data;
			break;
		case 3: // CU
			LOGMASKED(LOG_COMBINER, "%s: Combiner Card Constant U write: %02x\n", machine().describe_context(), (uint8_t)data);
			m_cursor_u = (uint8_t)data;
			break;
		case 4: // ES
			LOGMASKED(LOG_COMBINER, "%s: Combiner Card Ext. Store Flags (ES) write: %02x\n", machine().describe_context(), (uint8_t)data);
			m_ext_store_flags = (uint8_t)data;
			break;
		case 5: // EI
			LOGMASKED(LOG_COMBINER, "%s: Combiner Card Ext I Matte (EI) write: %02x\n", machine().describe_context(), (uint8_t)data);
			m_matte_ext[0] = (uint8_t)data;
			break;
		case 6: // EII
			LOGMASKED(LOG_COMBINER, "%s: Combiner Card Ext II Matte (EII) write: %02x\n", machine().describe_context(), (uint8_t)data);
			m_matte_ext[1] = (uint8_t)data;
			break;
		case 7: // Output Timing CP Flags
			LOGMASKED(LOG_OUTPUT_TIMING, "%s: Output Timing Card CP Register write: %02x\n", machine().describe_context(), (uint8_t)data);
			m_output_cpflags = (uint8_t)(data & 0x001f);
			break;
		case 8: // IS
			LOGMASKED(LOG_COMBINER, "%s: Combiner Card Matte I Select (IS) write: %02x\n", machine().describe_context(), (uint8_t)data);
			m_select_matte[0] = BIT(data, 0);
			break;
		case 9: // IY
			LOGMASKED(LOG_COMBINER, "%s: Combiner Card Matte I Y (IY) write: %02x\n", machine().describe_context(), (uint8_t)data);
			m_matte_y[0] = (uint8_t)data;
			break;
		case 10: // IU
			LOGMASKED(LOG_COMBINER, "%s: Combiner Card Matte I U (IU) write: %02x\n", machine().describe_context(), (uint8_t)data);
			m_matte_u[0] = (uint8_t)data;
			break;
		case 11: // IV
			LOGMASKED(LOG_COMBINER, "%s: Combiner Card Matte I V (IV) write: %02x\n", machine().describe_context(), (uint8_t)data);
			m_matte_v[0] = (uint8_t)data;
			break;
		case 12: // IIS
			LOGMASKED(LOG_COMBINER, "%s: Combiner Card Matte II Select (IIS) write: %02x\n", machine().describe_context(), (uint8_t)data);
			m_select_matte[1] = BIT(data, 0);
			break;
		case 13: // IIY
			LOGMASKED(LOG_COMBINER, "%s: Combiner Card Matte II Y (IIY) write: %02x\n", machine().describe_context(), (uint8_t)data);
			m_matte_y[1] = (uint8_t)data;
			break;
		case 14: // IIU
			LOGMASKED(LOG_COMBINER, "%s: Combiner Card Matte II U (IIU) write: %02x\n", machine().describe_context(), (uint8_t)data);
			m_matte_u[1] = (uint8_t)data;
			break;
		case 15: // IIV
			LOGMASKED(LOG_COMBINER, "%s: Combiner Card Matte II V (IIV) write: %02x\n", machine().describe_context(), (uint8_t)data);
			m_matte_v[1] = (uint8_t)data;
			break;
	}
}

bool dpb7000_state::handle_command(uint16_t data)
{
	//printf("handle_command %d, cxpos %d, cypos %d\n", m_brush_addr_cmd, m_cxpos[1], m_cypos[1]);
	switch (m_brush_addr_cmd)
	{
	case 0: // Live Video
		LOGMASKED(LOG_COMMANDS, "Unsupported command: Live Video\n");
		break;
	case 1: // Brush Store Read
		LOGMASKED(LOG_COMMANDS, "Unsupported command: Brush Store Read\n");
		break;
	case 2: // Brush Store Write
	case 3: // Framestore Read
	case 4: // Framestore Write
		m_bxlen_counter = m_bxlen;
		m_bylen_counter = m_bylen;
		return false;
	case 5: // Fast Wipe Video
		LOGMASKED(LOG_COMMANDS, "Unsupported command: Fast Wipe Video\n");
		break;
	case 6: // Fast Wipe Brush Store
		LOGMASKED(LOG_COMMANDS, "Unsupported command: Fast Wipe Brush Store\n");
		break;
	/*case 7: // Fast Wipe Framestore
	    for (int i = 0; i < 2; i++)
	    {
	        if (!BIT(data, 5 + i))
	        {
	            m_bylen_counter = m_bylen;
	            for (uint16_t y = m_cypos[1]; m_bylen_counter != 0x1000; m_bylen_counter++, y = (y + 1) & 0xfff)
	            {
	                if (y >= 768)
	                    continue;

	                uint8_t *lum = &m_framestore_lum[i][y * 800];
	                uint8_t *chr = &m_framestore_chr[i][y * 800];
	                uint8_t *ext = &m_framestore_ext[i][y * 800];
	                m_bxlen_counter = m_bxlen;
	                for (uint16_t x = m_cxpos[1]; m_bxlen_counter != 0x1000; m_bxlen_counter++, x = (x + 1) & 0xfff)
	                {
	                    if (x >= 800)
	                        continue;

	                    lum[x] = m_bs_y_latch;
	                    chr[x] = (x & 1) ? m_bs_v_latch : m_bs_u_latch;
	                    if (BIT(data, 9))
	                        ext[x] = m_bs_y_latch;
	                }
	            }
	        }
	    }
	    break;*/
	case 7: // Fast Wipe Framestore
	case 8: // Draw
	{
		const bool use_brush = (m_brush_addr_cmd == 8);
		const bool lum_en = BIT(data, 7);
		const bool chr_en = BIT(data, 8);
		const bool ext_en = BIT(data, 9);
		bool const write_en[2] = { lum_en, chr_en };

		const bool s1_disable = BIT(data, 5);
		const bool s2_disable = BIT(data, 6);

		const uint16_t sel_1 = (uint16_t)s1_disable << 5;
		const uint16_t sel_2 = (uint16_t)s2_disable << 4;
		const uint16_t sel_ext = ext_en << 6;
		const uint16_t fcs = BIT(data, 13);

		const uint16_t prom_addr = (fcs << 8) | (use_brush ? 0x80 : 0x00) | sel_ext | sel_1 | sel_2 | bitswap<4>(data, 1, 2, 3, 4);
		const uint8_t prom_data = m_brushproc_prom[prom_addr];

		const bool brush_zero = BIT(data, 12);
		const bool oe1 = BIT(prom_data, 0);
		const bool oe2 = BIT(prom_data, 1);
		const bool oe3 = BIT(prom_data, 2);
		const bool oe4 = BIT(prom_data, 7);

		const bool use_s1_data = (oe3 == oe4);
		const bool use_s2_data = !oe4;
		const bool use_ext_data = !oe3;
		const bool mask_sel = BIT(prom_data, 3);
		const bool word_width = BIT(prom_data, 4);
		const bool use_k_data = BIT(~prom_data, 6);
		const bool brush_invert = BIT(~data, 11);
		const uint16_t pal_addr_extra = (BIT(prom_data, 5) << 8) | (brush_invert << 9);

		const uint16_t brushaddr_pal_addr = (uint16_t)bitswap<4>(data, 1, 2, 3, 4) | (1 << 4) | (1 << 5) | (0 << 7) | (BIT(data, 5) << 8) | (1 << 9) | (1 << 10);
		const uint8_t brushaddr_pal_val = m_brushaddr_pal[brushaddr_pal_addr];

		int ext_idx = BIT(brushaddr_pal_val, 1) ? 1 : 0;

		uint8_t *store_1[2] = { &m_framestore_lum[0][0], &m_framestore_chr[0][0] };
		uint8_t *store_2[2] = { &m_framestore_lum[1][0], &m_framestore_chr[1][0] };
		uint8_t pressure[2] = { m_brush_press_lum, m_brush_press_chr };

		uint16_t bxlen = m_bxlen;//(((m_bxlen << 3) | (m_bixos & 7)) >> (m_bif & 3)) & 0xfff;
		uint16_t bylen = m_bylen;//(((m_bylen << 3) | (m_biyos & 7)) >> ((m_bif >> 2) & 3)) & 0xfff;
		//uint16_t width = 0x1000 - bxlen;
		//if (m_brush_addr_cmd == 8)
		m_ca0 = 0;
		//printf("\n%sing %dx%d brush at %d,%d (data %04x) (Store %d)\n", m_brush_addr_cmd == 8 ? "Draw" : "Wip", 0x1000 - m_bxlen, 0x1000 - m_bylen, m_cxpos[1], m_cypos[1], data, !s1_disable ? 1 : (!s2_disable ? 2 : 0));
		LOGMASKED(LOG_BRUSH_DRAWS, "%sing %dx%d brush at %d,%d (Store %d)\n", m_brush_addr_cmd == 8 ? "Draw" : "Wip", 0x1000 - m_bxlen, 0x1000 - m_bylen, m_cxpos[1], m_cypos[1], !s1_disable ? 1 : (!s2_disable ? 2 : 0));
		if ((m_cxpos[1] == 0x16 && m_cypos[1] == 0xda) || (m_cxpos[1] == 0x216 && m_cypos[1] == 0x58) || data == 0x39d1)
		{
			LOGMASKED(LOG_BRUSH_DRAWS, "    BAddrPALAddr:%03x BAddrPALVal:%02x ExtIdx:%d\n", brushaddr_pal_addr, brushaddr_pal_val, ext_idx);
			LOGMASKED(LOG_BRUSH_DRAWS, "    /S1:%d /S2:%d LUM:%d CHR:%d EXT:%d BZ:%d /BI:%d FCS:%d\n", s1_disable, s2_disable, lum_en, chr_en, BIT(data, 9), brush_zero, brush_invert, fcs);
			LOGMASKED(LOG_BRUSH_DRAWS, "    PROMAddr:%03x PROMVal:%02x OE1:%d OE2:%d OE3:%d OE4:%d MSEL:%d 16BIT:%d\n", prom_addr, prom_data, oe1, oe2, oe3, oe4, mask_sel, word_width);
		}

		for (uint16_t y = m_cypos[1], bly = bylen; bly != 0x1000; bly++, y = (y + 1) & 0xfff)
		{
			if (y >= 768)
				continue;

			uint32_t line_idx = y * 800;
			for (uint16_t x = m_cxpos[1], blx = bxlen; blx != 0x1000; blx++, x = (x + 1) & 0xfff)
			{
				if (x >= 800)
					continue;

				if (m_output_cpflags & 0x0c)
				{
					const uint16_t x_nib_addr = (x >> 1) & 0x3ff;
					uint16_t x_xlnib = m_storeaddr_xlnib[x_nib_addr] & 0xe;
					uint16_t x_xmnib = (m_storeaddr_xmnib[x_nib_addr] & 0x0f) << 4;
					uint16_t x_xhnib = (m_storeaddr_xhnib[x_nib_addr] & 0x0f) << 8;
					uint16_t remapped_x = x_xhnib | x_xmnib | x_xlnib | (x & 1);

					uint16_t prot_addr = (BIT(~m_output_cpflags, 2) << 8) | (BIT(~m_output_cpflags, 3) << 9);
					uint16_t prot_x_addr = (remapped_x >> 4) & 0xff;
					uint16_t prot_y_addr = ((y - 8) >> 2) & 0xff;
					const bool prot_x = m_storeaddr_protx[prot_addr | prot_x_addr] & 1;
					const bool prot_y = m_storeaddr_proty[prot_addr | prot_y_addr] & 1;

					if (prot_x || prot_y)
					{
						LOGMASKED(LOG_CSR, "Prot Reject: %03x, %03x (%d, %d)\n", prot_addr | prot_x_addr, prot_addr | prot_y_addr, prot_x, prot_y);
						continue;
					}
				}

				uint32_t pix_idx = line_idx + x;

				uint8_t uv_bsel = x & 1;

				uint8_t fixed_chr = (uv_bsel ? m_bs_v_latch : m_bs_u_latch);
				uint8_t brush_values[2] = { m_bs_y_latch, fixed_chr };
				uint8_t brush_ext = 0xff;

				if ((m_cxpos[1] == 0x16 && m_cypos[1] == 0xda) || (m_cxpos[1] == 0x216 && m_cypos[1] == 0x58) || data == 0x39d1)
				{
					LOGMASKED(LOG_BRUSH_DRAWS, "    Out x,y: %d,%d\n", x, y);
				}
				if (use_brush)
				{
					uint16_t bx = ((((blx - bxlen) << 3) | (m_bixos & 7)) >> (3 - (m_bif & 3))) & 0xfff;
					uint16_t by = ((((bly - bylen) << 3) | (m_biyos & 7)) >> ((3 - (m_bif >> 2)) & 3)) & 0xfff;
					uint16_t uv_bx = ((x & 1) ? (bx | 1) : (bx & ~1));
					uint8_t brush_lum = m_brushstore_lum[by * 256 + bx];
					uint8_t brush_chr = m_brushstore_chr[by * 256 + uv_bx];
					brush_ext = ext_en ? m_brushstore_ext[by * 256 + bx] : brush_lum;
					brush_values[0] = (fcs || !lum_en) ? m_bs_y_latch : brush_lum;
					brush_values[1] = (fcs || !chr_en) ? fixed_chr : brush_chr;

					if ((m_cxpos[1] == 0x16 && m_cypos[1] == 0xda) || (m_cxpos[1] == 0x216 && m_cypos[1] == 0x58) || data == 0x39d1)
					{
						LOGMASKED(LOG_BRUSH_DRAWS, "    Brush pixel %d,%d (bl %03x,%03x) (blen %03x,%03x) (store %d,%d), Brush LCE %02x %02x %02x:\n", 0x1000 - blx, 0x1000 - bly, blx, bly, bxlen, bylen, bx, by, brush_values[0], brush_values[1], brush_ext);
					}
				}
				else if (s1_disable && !s2_disable && x == 22 && y == 8)
				{
					LOGMASKED(LOG_BRUSH_DRAWS, "    Fixed brush value LCE %02x %02x %02x (YUV %02x %02x %02x)\n\n", brush_values[0], brush_values[1], brush_ext, m_bs_y_latch, m_bs_u_latch, m_bs_v_latch);
				}

				for (int ch = 0; ch < 2; ch++)
				{
					uint8_t s1_data = store_1[ch][pix_idx];
					uint8_t s2_data = store_2[ch][pix_idx];
					//uint8_t sext_data = m_framestore_ext[!s2_disable ? 1 : 0][pix_idx];
					uint8_t sext_data = m_framestore_ext[ext_idx][pix_idx];

					uint8_t brush_data = brush_values[ch];
					uint8_t brush_ext_data = use_k_data ? brush_ext : 0xff;
					uint8_t other_data = 0x00;
					uint8_t pressure_data = pressure[ch];
					uint8_t press_product = (uint8_t)((brush_ext_data * pressure_data + 0x0080) >> 8);
					if ((m_cxpos[1] == 0x16 && m_cypos[1] == 0xda) || (m_cxpos[1] == 0x216 && m_cypos[1] == 0x58) || data == 0x39d1)
					{
						LOGMASKED(LOG_BRUSH_DRAWS, "        brush ext (%02x) * pressure (%02x) = %02x\n", brush_ext_data, pressure_data, press_product);
					}

					uint8_t mask_multiplicand = 0x00;
					if (brush_zero)
						mask_multiplicand = (mask_sel ? sext_data : 0xff);

					uint8_t press_result = (uint8_t)((press_product * mask_multiplicand + 0x0080) >> 8);
					if ((m_cxpos[1] == 0x16 && m_cypos[1] == 0xda) || (m_cxpos[1] == 0x216 && m_cypos[1] == 0x58) || data == 0x39d1)
					{
						LOGMASKED(LOG_BRUSH_DRAWS, "        press product (%02x) * mask multiplicand (%02x) = %02x\n\n", press_product, mask_multiplicand, press_result);
					}
					uint16_t pal_addr = (uint8_t)bitswap<8>(press_result, 0, 1, 2, 3, 4, 5, 6, 7) | pal_addr_extra;
					uint16_t pal_data = m_brushproc_pal[pal_addr];
					uint8_t pal_value = (uint8_t)pal_data;
					bool pal_sel = BIT(pal_data, 8);
					bool pal_invert = BIT(pal_data, 9);

					if ((m_cxpos[1] == 0x16 && m_cypos[1] == 0xda) || (m_cxpos[1] == 0x216 && m_cypos[1] == 0x58) || data == 0x39d1)
					{
						LOGMASKED(LOG_BRUSH_DRAWS, "        Brush %s data: %02x\n", ch ? "chr" : "lum", brush_data);
						LOGMASKED(LOG_BRUSH_DRAWS, "        Bext:%02x, PressDat:%02x, PressProd:%02x, MaskMult:%02x, PressRes:%02x, PALAddr:%03x, PALData:%03x, PALVal:%02x\n", brush_ext_data, pressure_data, press_product, mask_multiplicand, press_result, pal_addr, pal_data, pal_value);
						LOGMASKED(LOG_BRUSH_DRAWS, "        S1:%02x S2:%02x SE:%02x\n", s1_data, s2_data, sext_data);
					}

					if (use_s1_data)
					{
						other_data = s1_data;
						if ((m_cxpos[1] == 0x16 && m_cypos[1] == 0xda) || (m_cxpos[1] == 0x216 && m_cypos[1] == 0x58) || data == 0x39d1) LOGMASKED(LOG_BRUSH_DRAWS, "        Using Store I for other data, %02x\n", other_data);
					}
					else if (use_s2_data)
					{
						other_data = s2_data;
						if ((m_cxpos[1] == 0x16 && m_cypos[1] == 0xda) || (m_cxpos[1] == 0x216 && m_cypos[1] == 0x58) || data == 0x39d1) LOGMASKED(LOG_BRUSH_DRAWS, "        Using Store II for other data, %02x\n", other_data);
					}
					else if (use_ext_data)
					{
						other_data = sext_data;
						if ((m_cxpos[1] == 0x16 && m_cypos[1] == 0xda) || (m_cxpos[1] == 0x216 && m_cypos[1] == 0x58) || data == 0x39d1) LOGMASKED(LOG_BRUSH_DRAWS, "        Using Stencil for other data, %02x\n", other_data);
					}

					if (!oe1)
					{
						brush_data = s1_data;
						if ((m_cxpos[1] == 0x16 && m_cypos[1] == 0xda) || (m_cxpos[1] == 0x216 && m_cypos[1] == 0x58) || data == 0x39d1) LOGMASKED(LOG_BRUSH_DRAWS, "        Using Store I for brush data, %02x\n", other_data);
					}
					else if (!oe2)
					{
						brush_data = s2_data;
						if ((m_cxpos[1] == 0x16 && m_cypos[1] == 0xda) || (m_cxpos[1] == 0x216 && m_cypos[1] == 0x58) || data == 0x39d1) LOGMASKED(LOG_BRUSH_DRAWS, "        Using Store II for brush data, %02x\n", other_data);
					}

					if ((m_cxpos[1] == 0x16 && m_cypos[1] == 0xda) || (m_cxpos[1] == 0x216 && m_cypos[1] == 0x58) || data == 0x39d1)
					{
						LOGMASKED(LOG_BRUSH_DRAWS, "        Before-comp brush data %02x, other data %02x\n", brush_data, other_data);
					}

					uint8_t comp_a = brush_data;
					uint8_t comp_b = other_data;

					uint8_t alu_out = 0x00;
					bool final_add = false;
					if (comp_a < comp_b)
					{
						if ((m_cxpos[1] == 0x16 && m_cypos[1] == 0xda) || (m_cxpos[1] == 0x216 && m_cypos[1] == 0x58) || data == 0x39d1)
						{
							LOGMASKED(LOG_BRUSH_DRAWS, "        comp_a %02x < comp_b %02x, final ALU pass should subtract\n", comp_a, comp_b);
						}
						alu_out = other_data - brush_data;
					}
					else
					{
						if ((m_cxpos[1] == 0x16 && m_cypos[1] == 0xda) || (m_cxpos[1] == 0x216 && m_cypos[1] == 0x58) || data == 0x39d1)
						{
							LOGMASKED(LOG_BRUSH_DRAWS, "        comp_a %02x > comp_b %02x, final ALU pass should add, calculating %02x - %02x = %02x\n", comp_a, comp_b, comp_a, comp_b, comp_a - comp_b);
						}
						alu_out = brush_data - other_data;
						final_add = true;
					}

					if (pal_invert)
						brush_data = ~brush_data;

					uint16_t out_product = (uint16_t)alu_out * (uint16_t)pal_value;
					if ((m_cxpos[1] == 0x16 && m_cypos[1] == 0xda) || (m_cxpos[1] == 0x216 && m_cypos[1] == 0x58) || data == 0x39d1)
					{
						LOGMASKED(LOG_BRUSH_DRAWS, "        ALU out %02x, out product %04x (%02x * %02x), PALSel:%d, PALInv:%d\n", alu_out, out_product, alu_out, pal_value, pal_sel, pal_invert);
					}
					uint8_t final_msb = 0;
					uint8_t final_lsb = 0;
					if (!word_width)
					{
						uint16_t alu_a = (other_data << 8) | s2_data;
						uint16_t alu_b = out_product;
						uint16_t final_alu_out = (final_add ? (alu_a + alu_b) : (alu_a - alu_b));

						final_msb = (pal_sel ? (final_alu_out >> 8) : brush_data);
						final_lsb = final_msb;

						if ((m_cxpos[1] == 0x16 && m_cypos[1] == 0xda) || (m_cxpos[1] == 0x216 && m_cypos[1] == 0x58) || data == 0x39d1)
						{
							if (final_add)
							{
								LOGMASKED(LOG_BRUSH_DRAWS, "        8bpp, S1D %02x, S2D %02x, ALUA %04x, ALUB %04x, ALU final %04x + %04x = %04x, MSB %02x\n", s1_data, s2_data, alu_a, alu_b, alu_a, alu_b, final_alu_out, final_msb);
							}
							else
							{
								LOGMASKED(LOG_BRUSH_DRAWS, "        8bpp, S1D %02x, S2D %02x, ALUA %04x, ALUB %04x, ALU final %04x - %04x = %04x, MSB %02x\n", s1_data, s2_data, alu_a, alu_b, alu_a, alu_b, final_alu_out, final_msb);
							}
						}
					}
					else
					{
						uint16_t alu_a = (other_data << 8) | s2_data;
						uint16_t alu_b = out_product;
						uint16_t final_alu_out = (final_add ? (alu_a + alu_b) : (alu_a - alu_b));
						final_msb = (pal_sel ? (final_alu_out >> 8) : brush_data);
						final_lsb = (uint8_t)final_alu_out;

						if ((m_cxpos[1] == 0x16 && m_cypos[1] == 0xda) || (m_cxpos[1] == 0x216 && m_cypos[1] == 0x58) || data == 0x39d1)
						{
							if (final_add)
							{
								LOGMASKED(LOG_BRUSH_DRAWS, "        16bpp, ALUA %04x, ALUB %04x, %04x+%04x=%04x, MSB %02x, LSB %02x\n", alu_a, alu_b, alu_a, alu_b, final_alu_out, final_msb, final_lsb);
							}
							else
							{
								LOGMASKED(LOG_BRUSH_DRAWS, "        16bpp, ALUA %04x, ALUB %04x, %04x-%04x=%04x, MSB %02x, LSB %02x\n", alu_a, alu_b, alu_a, alu_b, final_alu_out, final_msb, final_lsb);
							}
						}
					}

					if (write_en[ch])
					{
						if (!s1_disable)
						{
							if ((m_cxpos[1] == 0x16 && m_cypos[1] == 0xda) || (m_cxpos[1] == 0x216 && m_cypos[1] == 0x58) || data == 0x39d1)
							{
								LOGMASKED(LOG_BRUSH_DRAWS, "        Storing %02x in Store 1 %s\n", final_msb, ch ? "chr" : "lum");
							}
							store_1[ch][pix_idx] = s2_disable ? final_lsb : final_msb;
						}
						if (!s2_disable)
						{
							store_2[ch][pix_idx] = final_lsb;
							if ((m_cxpos[1] == 0x16 && m_cypos[1] == 0xda) || (m_cxpos[1] == 0x216 && m_cypos[1] == 0x58) || data == 0x39d1)
							{
								LOGMASKED(LOG_BRUSH_DRAWS, "        Storing %02x in Store 2 %s\n", final_lsb, ch ? "chr" : "lum");
							}
						}
					}

					if (ch == 0 && ext_en)
					{
						if (!s1_disable)
						{
							if ((m_cxpos[1] == 0x16 && m_cypos[1] == 0xda) || (m_cxpos[1] == 0x216 && m_cypos[1] == 0x58) || data == 0x39d1)
							{
								LOGMASKED(LOG_BRUSH_DRAWS, "        Storing %02x in EXT 1\n", final_msb);
							}
							m_framestore_ext[0][pix_idx] = final_msb;
						}
						if (!s2_disable)
						{
							if ((m_cxpos[1] == 0x16 && m_cypos[1] == 0xda) || (m_cxpos[1] == 0x216 && m_cypos[1] == 0x58) || data == 0x39d1)
							{
								LOGMASKED(LOG_BRUSH_DRAWS, "        Storing %02x in EXT 2\n", final_lsb);
							}
							m_framestore_ext[1][pix_idx] = final_lsb;
						}
					}
				}
			}
		}
		break;
	}
	case 9: // Draw with Stencil I
		LOGMASKED(LOG_COMMANDS, "Unsupported command: Draw with Stencil I\n");
		break;
	case 10: // Draw with Stencil II
		LOGMASKED(LOG_COMMANDS, "Unsupported command: Draw with Stencil II\n");
		break;
	case 11: // Copy to Framestore
		LOGMASKED(LOG_COMMANDS, "Unsupported command: Copy to Framestore\n");
		break;
	case 12: // Copy to Brush Store
	{
		uint8_t *store_lum = !BIT(data, 5) ? &m_framestore_lum[0][0] : (!BIT(data, 6) ? &m_framestore_lum[1][0] : nullptr);
		uint8_t *store_chr = !BIT(data, 5) ? &m_framestore_chr[0][0] : (!BIT(data, 6) ? &m_framestore_chr[1][0] : nullptr);
		uint8_t *store_ext = !BIT(data, 5) ? &m_framestore_ext[0][0] : (!BIT(data, 6) ? &m_framestore_ext[1][0] : nullptr);
		if (!BIT(data, 7) || true)
			store_lum = nullptr;
		if (!BIT(data, 8))
			store_chr = nullptr;
		if (!BIT(data, 9))
			store_ext = nullptr;

		m_ca0 = 0;
		for (uint32_t y = m_cypos[1], bly = m_bylen; bly != 0x1000; bly++, y = (y + 1) & 0xfff)
		{
			if (y >= 256)
				continue;

			const uint32_t line_idx = y * 800;
			for (uint32_t x = m_cxpos[1], blx = m_bxlen; blx != 0x1000; blx++, x = (x + 1) & 0xfff)
			{
				if (x >= 256)
					continue;

				const uint32_t pix_idx = line_idx + x;
				const uint16_t bx = x;//((((blx - m_bxlen) << 3) | (m_bixos & 7)) >> (3 - (m_bif & 3))) & 0xfff;
				const uint16_t by = y;//((((bly - m_bylen) << 3) | (m_biyos & 7)) >> ((3 - (m_bif >> 2)) & 3)) & 0xfff;
				const uint16_t uv_bx = ((x & 1) ? (bx | 1) : (bx & ~1));
				if (store_lum)
					m_brushstore_lum[by * 256 + bx] = store_lum[pix_idx];
				if (store_chr)
					m_brushstore_chr[by * 256 + uv_bx] = store_chr[pix_idx];
				if (store_ext)
					m_brushstore_ext[by * 256 + bx] = store_ext[pix_idx];
			}
		}
		break;
	}
	case 13: // Paste with Stencil I
		LOGMASKED(LOG_COMMANDS, "Unsupported command: Paste with Stencil I\n");
		break;
	case 14: // Paste with Stencil II
		LOGMASKED(LOG_COMMANDS, "Unsupported command: Paste with Stencil II\n");
		break;
	case 15: // Copy to same Framestore (Invert)
		LOGMASKED(LOG_COMMANDS, "Unsupported command: Copy to Framestore (Invert)\n");
		break;
	}
	return true;
}

void dpb7000_state::cpu_ctrlbus_w(uint16_t data)
{
	switch (m_csr)
	{
	case 0: // Brush Address Card, function select
	{
		static const char* const func_names[16] =
		{
			"Live Video",           "Brush Store Read",     "Brush Store Write",        "Framestore Read",
			"Framestore Write",     "Fast Wipe Video",      "Fast Wipe Brush Store",    "Fast Wipe Framestore",
			"Draw",                 "Draw with Stencil I",  "Draw with Stencil II",     "Copy to Framestore",
			"Copy to Brush Store",  "Paste with Stencil I", "Paste with Stencil II",    "Copy to same Framestore (Invert)"
		};
		LOGMASKED(LOG_BRUSH_ADDR, "%s: Brush Address Card, Function Select: %04x\n", machine().describe_context(), data);
		LOGMASKED(LOG_BRUSH_ADDR, "                Function:           %s\n", func_names[(data >> 1) & 0xf]);
		LOGMASKED(LOG_BRUSH_ADDR, "                /S1:                %d\n", BIT(data, 5));
		LOGMASKED(LOG_BRUSH_ADDR, "                /S2:                %d\n", BIT(data, 6));
		LOGMASKED(LOG_BRUSH_ADDR, "                LUMEN:              %d\n", BIT(data, 7));
		LOGMASKED(LOG_BRUSH_ADDR, "                CHREN:              %d\n", BIT(data, 8));
		LOGMASKED(LOG_BRUSH_ADDR, "                KSEL:               %d\n", BIT(data, 9));
		LOGMASKED(LOG_BRUSH_ADDR, "                DISCEN:             %d\n", BIT(data, 10));
		LOGMASKED(LOG_BRUSH_ADDR, "                /KINV:              %d\n", BIT(data, 11));
		LOGMASKED(LOG_BRUSH_ADDR, "                /KZERO:             %d\n", BIT(data, 12));
		LOGMASKED(LOG_BRUSH_ADDR, "                FCS:                %d\n", BIT(data, 13));
		LOGMASKED(LOG_BRUSH_ADDR, "                Go:                 %d\n", BIT(data, 0));
		m_brush_addr_func = data & ~1;
		if (BIT(data, 0))
		{
			m_brush_addr_func |= 0x8000;
			m_brush_addr_cmd = (data >> 1) & 0xf;
			if (handle_command(data)) {
				m_cmd_done_timer->adjust(attotime::from_usec(500));
			}
		}
		break;
	}

	case 1: // Disk Sequencer Card, disk access
	{
		const uint8_t hi_nybble = data >> 12;
		if (hi_nybble == 0)
		{
			m_diskseq_cyl_from_cpu = data & 0x3ff;
			LOGMASKED(LOG_CTRLBUS, "%s: CPU write to Control Bus, Disk Sequencer Card, Cylinder Number: %04x (%04x)\n", machine().describe_context(), m_diskseq_cyl_from_cpu, data);
			if (m_diskseq_cyl_from_cpu == 0)
				m_diskseq_status |= 0x04;
			else
				m_diskseq_status &= ~0x04;
		}
		else if (hi_nybble == 1)
		{
			LOGMASKED(LOG_CTRLBUS, "%s: CPU write to Control Bus, Disc Data Buffer Card, Preset RAM Address: %04x\n", machine().describe_context(), data & 0xfff);
			m_diskbuf_ram_addr = (data & 0xfff) << 3;
		}
		else if (hi_nybble == 2)
		{
			m_diskseq_cmd_word_from_cpu = data & 0xfff;
			m_diskseq_cmd = (data >> 8) & 0xf;
			int group = (int)(data >> 5) & 3;
			LOGMASKED(LOG_CTRLBUS, "%s: CPU write to Control Bus, Disk Sequencer Card, Command: %x (%04x)\n", machine().describe_context(), (data >> 8) & 0xf, data);
			LOGMASKED(LOG_CTRLBUS, "%s                                                    Head: %x\n", machine().describe_context(), data & 0xf);
			LOGMASKED(LOG_CTRLBUS, "%s                                                   Drive: %x\n", machine().describe_context(), (data >> 5) & 7);
			switch (m_diskseq_cmd)
			{
			case 1:
			case 5:
			case 9:
			case 13:
				LOGMASKED(LOG_CTRLBUS, "%s: Disk Sequencer Card Command: No command\n", machine().describe_context());
				break;

			case 0:
				LOGMASKED(LOG_CTRLBUS, "%s: Disk Sequencer Card Command: Read track to buffer RAM\n", machine().describe_context());
				if (!BIT(m_diskseq_status, 3))
				{
					req_b_w(0); // Flag ourselves as in-use
					m_diskseq_cyl_read_pending = true;
					m_diskseq_command_stride = 1;
					if (is_disk_group_fdd(group))
					{
						m_fdd_side = 0;
						seek_fdd_to_cylinder();
					}
					else if (is_disk_group_hdd(group))
					{
						m_diskseq_use_hdd_pending = true;
						m_hdd_command_timer->adjust(attotime::from_double((double)19200 / 1012000));
					}
				}
				break;

			case 2:
				LOGMASKED(LOG_CTRLBUS, "%s: Disk Sequencer Card Command: Read track, stride 2, to buffer RAM\n", machine().describe_context());
				if (!BIT(m_diskseq_status, 3))
				{
					req_b_w(0); // Flag ourselves as in-use
					m_diskseq_cyl_read_pending = true;
					m_diskseq_command_stride = 2;
					if (is_disk_group_fdd(group))
					{
						m_fdd_side = 0;
						seek_fdd_to_cylinder();
					}
					else if (is_disk_group_hdd(group))
					{
						m_diskseq_use_hdd_pending = true;
						m_hdd_command_timer->adjust(attotime::from_double((double)19200 / 1012000));
					}
				}
				break;

			case 3:
				LOGMASKED(LOG_CTRLBUS, "%s: Disk Sequencer Card Command: Restore\n", machine().describe_context());
				m_diskseq_cyl_from_cpu = 0;
				req_b_w(0); // Flag ourselves as in-use
				m_diskseq_complete_clk->adjust(attotime::from_msec(1), 1);
				break;

			case 4:
				LOGMASKED(LOG_CTRLBUS, "%s: Disk Sequencer Card Command: Read Track\n", machine().describe_context());
				if (!BIT(m_diskseq_status, 3))
				{
					req_b_w(0); // Flag ourselves as in-use
					m_line_count = 0;
					m_line_clock = 0;

					m_diskseq_cyl_read_pending = true;
					m_diskseq_command_stride = 1;
					if (is_disk_group_fdd(group))
					{
						m_diskbuf_data_count = 0x2700;
						m_fdd_side = 0;
						seek_fdd_to_cylinder();
					}
					else if (is_disk_group_hdd(group))
					{
						m_diskbuf_data_count = 0x4b00;
						m_diskseq_use_hdd_pending = true;
						m_hdd_command_timer->adjust(attotime::from_double((double)19200 / 1012000));
					}
				}
				break;

			case 6:
				LOGMASKED(LOG_CTRLBUS, "%s: Disk Sequencer Card Command: Disc Clear, Read Track\n", machine().describe_context());
				if (!BIT(m_diskseq_status, 3))
				{
					m_size_dxdx_counter = 0;
					m_size_dydy_counter = 0;
					m_size_dest_x = (m_brush_addr_cmd != 2) ? m_cxpos[1] : 0;
					m_size_dest_y = (m_brush_addr_cmd != 2) ? m_cypos[1] : 0;

					req_b_w(0); // Flag ourselves as in-use
					m_diskseq_cyl_read_pending = true;
					m_diskseq_command_stride = 1;
					if (is_disk_group_fdd(group))
					{
						m_diskbuf_data_count = 0x2700;
						m_fdd_side = 0;
						seek_fdd_to_cylinder();
					}
					else if (is_disk_group_hdd(group))
					{
						m_diskbuf_data_count = 0x4b00;
						m_diskseq_use_hdd_pending = true;
						m_hdd_command_timer->adjust(attotime::from_double((double)19200 / 1012000));
					}
				}
				break;

			case 8:
				LOGMASKED(LOG_CTRLBUS, "%s: Disk Sequencer Card Command: Write Track from Buffer RAM\n", machine().describe_context());
				if (!BIT(m_diskseq_status, 3))
				{
					req_b_w(0); // Flag ourselves as in-use
					m_diskseq_cyl_write_pending = true;
					m_diskseq_command_stride = 1;
					if (is_disk_group_fdd(group))
					{
						m_diskbuf_data_count = 0x2700;
						m_fdd_side = 0;
						seek_fdd_to_cylinder();
					}
					else if (is_disk_group_hdd(group))
					{
						m_diskbuf_data_count = 0x4b00;
						m_diskseq_use_hdd_pending = true;
						m_hdd_command_timer->adjust(attotime::from_double((double)19200 / 1012000));
					}
				}
				break;

			case 10:
				LOGMASKED(LOG_CTRLBUS, "%s: Disk Sequencer Card Command: Write Track, stride 2, from Buffer RAM\n", machine().describe_context());
				if (!BIT(m_diskseq_status, 3))
				{
					req_b_w(0); // Flag ourselves as in-use
					m_diskseq_cyl_write_pending = true;
					m_diskseq_command_stride = 2;
					if (is_disk_group_fdd(group))
					{
						m_diskbuf_data_count = 0x2700;
						m_fdd_side = 0;
						seek_fdd_to_cylinder();
					}
					else if (is_disk_group_hdd(group))
					{
						m_diskbuf_data_count = 0x4b00;
						m_diskseq_use_hdd_pending = true;
						m_hdd_command_timer->adjust(attotime::from_double((double)19200 / 1012000));
					}
				}
				break;

			case 12:
				LOGMASKED(LOG_CTRLBUS, "%s: Disk Sequencer Card Command: Write Track\n", machine().describe_context());
				if (!BIT(m_diskseq_status, 3) && is_disk_group_hdd(group))
				{
					req_b_w(0); // Flag ourselves as in-use

					m_diskseq_cyl_write_pending = true;
					m_diskseq_command_stride = 1;

					m_diskbuf_data_count = 0x4b00;
					m_diskseq_use_hdd_pending = true;
					m_hdd_command_timer->adjust(attotime::from_double((double)19200 / 1012000));
				}
				break;

			case 14:
				LOGMASKED(LOG_CTRLBUS, "%s: Disk Sequencer Card Command: Disc Clear, Write Track\n", machine().describe_context());
				if (!BIT(m_diskseq_status, 3) && is_disk_group_hdd(group))
				{
					req_b_w(0); // Flag ourselves as in-use
					m_line_count = 0;
					m_line_clock = 0;

					m_diskseq_cyl_write_pending = true;
					m_diskseq_command_stride = 1;

					m_diskbuf_data_count = 0x4b00;
					m_diskseq_use_hdd_pending = true;
					m_hdd_command_timer->adjust(attotime::from_double((double)19200 / 1012000));
				}
				break;

			default:
				LOGMASKED(LOG_CTRLBUS, "%s: Unknown Disk Sequencer Card command.\n", machine().describe_context());
				req_b_w(0); // Flag ourselves as in-use
				m_diskseq_complete_clk->adjust(attotime::from_msec(1), 1);
				break;
			}
		}
		else
		{
			LOGMASKED(LOG_CTRLBUS | LOG_UNKNOWN, "%s: CPU write to Control Bus, Disk Sequencer Card, Unrecognized hi nybble: %04x\n", machine().describe_context(), data);
		}
		break;
	}

	case 2: // Store Address Card
		if (BIT(data, 15))
			store_address_w(0, data);
		else
			store_address_w(1, data);
		break;

	case 3: // Size Card, Y6/Y7
		if (BIT(data, 15))
		{
			LOGMASKED(LOG_CTRLBUS | LOG_SIZE_CARD, "%s:    Size Card Y7 (Flags): %04x\n", machine().describe_context(), data & 0x7fff);
		}
		else
		{
			switch (data & 0x3300)
			{
			case 0x1000: // Change in source X per change in dest X, MSB
				LOGMASKED(LOG_CTRLBUS | LOG_SIZE_CARD, "%s:    Size Card delta source X per delta dest X, MSB: %02x (%04x)\n", machine().describe_context(), data & 0xff, data);
				m_size_dsx_ddx &= 0x00ff;
				m_size_dsx_ddx |= (data & 0x00ff) << 8;
				break;
			case 0x0000: // Change in source X per change in dest X, LSB
				LOGMASKED(LOG_CTRLBUS | LOG_SIZE_CARD, "%s:    Size Card delta source X per delta dest X, LSB: %02x (%04x)\n", machine().describe_context(), data & 0xff, data);
				m_size_dsx_ddx &= 0xff00;
				m_size_dsx_ddx |= data & 0x00ff;
				break;
			case 0x3000: // Change in source Y per change in dest X, MSB
				LOGMASKED(LOG_CTRLBUS | LOG_SIZE_CARD, "%s:    Size Card delta source Y per delta dest X, MSB: %02x (%04x)\n", machine().describe_context(), data & 0xff, data);
				m_size_dsy_ddx &= 0x00ff;
				m_size_dsy_ddx |= (data & 0x00ff) << 8;
				break;
			case 0x2000: // Change in source Y per change in dest X, LSB
				LOGMASKED(LOG_CTRLBUS | LOG_SIZE_CARD, "%s:    Size Card delta source Y per delta dest X, LSB: %02x (%04x)\n", machine().describe_context(), data & 0xff, data);
				m_size_dsy_ddx &= 0xff00;
				m_size_dsy_ddx |= data & 0x00ff;
				break;
			case 0x1100: // Change in source X per change in dest Y, MSB
				LOGMASKED(LOG_CTRLBUS | LOG_SIZE_CARD, "%s:    Size Card delta source X per delta dest Y, MSB: %02x (%04x)\n", machine().describe_context(), data & 0xff, data);
				m_size_dsx_ddy &= 0x00ff;
				m_size_dsx_ddy |= (data & 0x00ff) << 8;
				break;
			case 0x0100: // Change in source X per change in dest Y, LSB
				LOGMASKED(LOG_CTRLBUS | LOG_SIZE_CARD, "%s:    Size Card delta source X per delta dest Y, LSB: %02x (%04x)\n", machine().describe_context(), data & 0xff, data);
				m_size_dsx_ddy &= 0xff00;
				m_size_dsx_ddy |= data & 0x00ff;
				break;
			case 0x3100: // Change in source Y per change in dest Y, MSB
				LOGMASKED(LOG_CTRLBUS | LOG_SIZE_CARD, "%s:    Size Card delta source Y per delta dest Y, MSB: %02x (%04x)\n", machine().describe_context(), data & 0xff, data);
				m_size_dsy_ddy &= 0x00ff;
				m_size_dsy_ddy |= (data & 0x00ff) << 8;
				break;
			case 0x2100: // Change in source Y per change in dest Y, LSB
				LOGMASKED(LOG_CTRLBUS | LOG_SIZE_CARD, "%s:    Size Card delta source Y per delta dest Y, LSB: %02x (%04x)\n", machine().describe_context(), data & 0xff, data);
				m_size_dsy_ddy &= 0xff00;
				m_size_dsy_ddy |= data & 0x00ff;
				break;
			case 0x1300: // Origin, source X / dest X
				LOGMASKED(LOG_CTRLBUS | LOG_SIZE_CARD, "%s:    Size Card origin, source X / dest X: %02x (%04x)\n", machine().describe_context(), data & 0xff, data);
				m_size_frac_origin_sx_dx = data & 0x00ff;
				break;
			case 0x0300: // Origin, source X / dest Y
				LOGMASKED(LOG_CTRLBUS | LOG_SIZE_CARD, "%s:    Size Card origin, source X / dest Y: %02x (%04x)\n", machine().describe_context(), data & 0xff, data);
				m_size_frac_origin_sx_dy = data & 0x00ff;
				break;
			case 0x3300: // Origin, source Y / dest X
				LOGMASKED(LOG_CTRLBUS | LOG_SIZE_CARD, "%s:    Size Card origin, source Y / dest X: %02x (%04x)\n", machine().describe_context(), data & 0xff, data);
				m_size_frac_origin_sy_dx = data & 0x00ff;
				break;
			case 0x2300: // Origin, source Y / dest Y
				LOGMASKED(LOG_CTRLBUS | LOG_SIZE_CARD, "%s:    Size Card origin, source Y / dest Y: %02x (%04x)\n", machine().describe_context(), data & 0xff, data);
				m_size_frac_origin_sy_dy = data & 0x00ff;
				break;
			}
		}
		break;

	case 5: // Filter Card
		if (BIT(data, 15))
		{
			static const char *const coeff_names[8] = { "Y-3", "Y-2", "Y-1", "Y (LSB)", "Y (MSB)", "Y+1", "Y+2", "Y+3" };
			const uint16_t coeff_idx = ((data >> 8) & 0xf) - 2;
			switch ((data >> 8) & 0xf)
			{
			case 2: // Y-3
			case 3: // Y-2
			case 4: // Y-1
			case 5: // Y (LSB)
			case 6: // Y (MSB)
			case 7: // Y+1
			case 8: // Y+2
			case 9: // Y+3
				LOGMASKED(LOG_CTRLBUS | LOG_FILTER_CARD, "%s: Filter Card %s Coefficient Write: %02x\n", machine().describe_context(), coeff_names[coeff_idx], data & 0xff);
				m_filter_y_coeffs[coeff_idx] = data & 0xff;
				break;
			default: // Ignored
				LOGMASKED(LOG_CTRLBUS | LOG_FILTER_CARD, "%s: Filter Card Coefficient (ignored Y) Write: %04x\n", machine().describe_context(), data);
				break;
			}
		}
		else
		{
			static const char *const coeff_names[12] = { "X-5", "X-4", "X-3", "X-2", "X-1", "X (LSB)", "X (MSB)", "X+1", "X+2", "X+3", "X+4", "X+5" };
			const uint16_t coeff_idx = ((data >> 8) & 0xf) - 1;
			switch ((data >> 8) & 0xf)
			{
			case 1: // X-5
			case 2: // X-4
			case 3: // X-3
			case 4: // X-2
			case 5: // X-1
			case 6: // X (LSB)
			case 7: // X (MSB)
			case 8: // X+1
			case 9: // X+2
			case 10: // X+3
			case 11: // X+4
			case 12: // X+5
				LOGMASKED(LOG_CTRLBUS | LOG_FILTER_CARD, "%s: Filter Card %s Coefficient Write: %02x\n", machine().describe_context(), coeff_names[coeff_idx], data & 0xff);
				m_filter_x_coeffs[coeff_idx] = data & 0xff;
				break;
			default: // Ignored
				LOGMASKED(LOG_CTRLBUS | LOG_FILTER_CARD, "%s: Filter Card Coefficient (ignored X) Write: %04x\n", machine().describe_context(), data);
				break;
			}
		}
		break;

	case 7: // Disk Data Buffer RAM
		LOGMASKED(LOG_DDB, "%s: Disc Data Buffer Card RAM write: %04x = %04x\n", machine().describe_context(), m_diskbuf_ram_addr, data);
		m_diskbuf_ram[m_diskbuf_ram_addr++] = (uint8_t)data;
		break;

	case 8: // Brush Store Card color latches
		m_bs_y_latch = (uint8_t)data;
		if (m_ca0)
		{
			LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR | LOG_BRUSH_LATCH, "%s: Brush Store Card color latches (VY): %04x\n\n", machine().describe_context(), data);
			m_bs_v_latch = (uint8_t)(data >> 8);
		}
		else
		{
			LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR | LOG_BRUSH_LATCH, "%s: Brush Store Card color latches (UY): %04x\n", machine().describe_context(), data);
			m_bs_u_latch = (uint8_t)(data >> 8);
		}
		if (m_brush_addr_cmd == 4) // Framestore Write
		{
			const uint8_t chr = m_ca0 ? m_bs_v_latch : m_bs_u_latch;
			if (!BIT(m_brush_addr_func, 5))
			{
				const uint16_t x = m_cxpos[0] + (m_bxlen_counter - m_bxlen);
				const uint16_t y = m_cypos[0] + (m_bylen_counter - m_bylen);
				uint32_t pix_idx = y * 800 + x;

				if (BIT(m_brush_addr_func, 7))
				{
					LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR | LOG_BRUSH_LATCH, "%s: Writing %02x to Store 1 luma at %d,%d\n", machine().describe_context(), m_bs_y_latch, x, y);
					m_framestore_lum[0][pix_idx] = m_bs_y_latch;
				}
				if (BIT(m_brush_addr_func, 8))
				{
					LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR | LOG_BRUSH_LATCH, "%s: Writing %02x to Store 1 chroma at %d,%d\n", machine().describe_context(), chr, x, y);
					m_framestore_chr[0][pix_idx] = chr;
				}
				if (BIT(m_brush_addr_func, 9))
				{
					LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR | LOG_BRUSH_LATCH, "%s: Writing %02x to Store 1 stencil at %d,%d\n", machine().describe_context(), m_bs_y_latch, x, y);
					m_framestore_ext[0][pix_idx] = m_bs_y_latch;
				}
			}
			if (!BIT(m_brush_addr_func, 6))
			{
				const uint16_t x = m_cxpos[1] + (m_bxlen_counter - m_bxlen);
				const uint16_t y = m_cypos[1] + (m_bylen_counter - m_bylen);
				uint32_t pix_idx = y * 800 + x;

				if (BIT(m_brush_addr_func, 7))
				{
					LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR | LOG_BRUSH_LATCH, "%s: Writing %02x to Store 2 luma at %d,%d\n", machine().describe_context(), m_bs_y_latch, x, y);
					m_framestore_lum[1][pix_idx] = m_bs_y_latch;
				}
				if (BIT(m_brush_addr_func, 8))
				{
					LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR | LOG_BRUSH_LATCH, "%s: Writing %02x to Store 2 chroma at %d,%d\n", machine().describe_context(), chr, x, y);
					m_framestore_chr[1][pix_idx] = chr;
				}
				if (BIT(m_brush_addr_func, 9))
				{
					LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR | LOG_BRUSH_LATCH, "%s: Writing %02x to Store 2 stencil at %d,%d\n", machine().describe_context(), m_bs_y_latch, x, y);
					m_framestore_ext[1][pix_idx] = m_bs_y_latch;
				}
			}

			m_bxlen_counter++;
			if (m_bxlen_counter == 0x1000)
			{
				m_bxlen_counter = m_bxlen;
				m_bylen_counter++;
				if (m_bylen_counter == 0x1000)
				{
					cmd_done(0);
				}
			}
		}
		m_ca0 = 1 - m_ca0;
		break;

	case 9: // Brush Address Card, register write/select
		switch (data >> 12)
		{
		case 1:
			LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR, "%s: Brush Address Card, Register Write: BIF = %02x\n", machine().describe_context(), data & 0xff);
			m_bif = data & 0xff;
			break;
		case 2:
			LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR, "%s: Brush Address Card, Register Write: BIXOS = %d\n", machine().describe_context(), data & 0x7);
			m_bixos = data & 0x7;
			break;
		case 3:
			LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR, "%s: Brush Address Card, Register Write: BIYOS = %d\n", machine().describe_context(), data & 0x7);
			m_biyos = data & 0x7;
			break;
		case 4:
			LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR, "%s: Brush Address Card, Register Write: BXLEN = %03x\n", machine().describe_context(), data & 0xfff);
			m_bxlen = data & 0xfff;
			m_bxlen_counter = data & 0xfff;
			break;
		case 5:
			LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR, "%s: Brush Address Card, Register Write: BYLEN = %03x\n", machine().describe_context(), data & 0xfff);
			m_bylen = data & 0xfff;
			m_bylen_counter = data & 0xfff;
			break;
		case 6:
			LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR | LOG_BRUSH_LATCH, "%s: Brush Address Card, Register Write: PLUM = %03x\n", machine().describe_context(), data & 0xff);
			m_brush_press_lum = data & 0xff;
			break;
		case 7:
			LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR | LOG_BRUSH_LATCH, "%s: Brush Address Card, Register Write: PCHR = %03x\n", machine().describe_context(), data & 0xff);
			m_brush_press_chr = data & 0xff;
			break;
		default:
			LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR, "%s: Brush Address Card, Register Write: Unknown (%04x)\n", machine().describe_context(), data);
			break;
		}
		break;

	case 10: // Output Timing Card - cursor registers; Combiner Card
	{
		const uint8_t hi_bits = (data >> 14) & 3;
		if (hi_bits == 0) // Cursor Parameters
		{
			const uint8_t reg = (data >> 12) & 3;
			switch (reg)
			{
			case 0: // Cursor X Origin
				LOGMASKED(LOG_CTRLBUS | LOG_OUTPUT_TIMING, "%s: CPU write to Output Timing Card: Cursor Origin X = %03x\n", machine().describe_context(), data & 0xfff);
				m_cursor_origin_x = data & 0xfff;
				break;
			case 1: // Cursor Y Origin
				LOGMASKED(LOG_CTRLBUS | LOG_OUTPUT_TIMING, "%s: CPU write to Output Timing Card: Cursor Origin Y = %03x\n", machine().describe_context(), data & 0xfff);
				m_cursor_origin_y = data & 0xfff;
				break;
			case 2: // Cursor X Size
				LOGMASKED(LOG_CTRLBUS | LOG_OUTPUT_TIMING, "%s: CPU write to Output Timing Card: Cursor Size X = %03x\n", machine().describe_context(), data & 0xfff);
				m_cursor_size_x = data & 0xfff;
				break;
			case 3: // Cursor Y Size
				LOGMASKED(LOG_CTRLBUS | LOG_OUTPUT_TIMING, "%s: CPU write to Output Timing Card: Cursor Size Y = %03x\n", machine().describe_context(), data & 0xfff);
				m_cursor_size_y = data & 0xfff;
				break;
			}
		}
		else if (hi_bits == 3) // Combiner Card, constant registers
		{
			combiner_reg_w(data);
		}
		else
		{
			LOGMASKED(LOG_CTRLBUS | LOG_OUTPUT_TIMING | LOG_UNKNOWN, "%s: CPU write to Output Timing Card, unknown select value: %04x\n", machine().describe_context(), data);
		}
		break;
	}

	case 15: // Disk Sequencer Card, panic reset
		LOGMASKED(LOG_CTRLBUS, "%s: CPU write to Control Bus, Disk Sequencer Card, panic reset\n", machine().describe_context());
		break;

	default:
		LOGMASKED(LOG_CTRLBUS | LOG_UNKNOWN, "%s: CPU write to Control Bus, unknown CSR %d: %04x\n", machine().describe_context(), m_csr, data);
		break;
	}
}

TIMER_CALLBACK_MEMBER(dpb7000_state::req_a_w)
{
	//if (machine().input().code_pressed(KEYCODE_LALT))
	{
		if (param)
		{
			m_sys_ctrl |= SYSCTRL_REQ_A_IN;
		}
		else
		{
			m_sys_ctrl &= ~SYSCTRL_REQ_A_IN;
		}

		update_req_irqs();
	}
	/*else
	{
	    m_sys_ctrl &= ~SYSCTRL_REQ_A_IN;
	    update_req_irqs();
	}*/
}

TIMER_CALLBACK_MEMBER(dpb7000_state::req_b_w)
{
	if (param)
	{
		m_sys_ctrl |= SYSCTRL_REQ_B_IN;
	}
	else
	{
		m_sys_ctrl &= ~SYSCTRL_REQ_B_IN;
	}

	update_req_irqs();
}

TIMER_CALLBACK_MEMBER(dpb7000_state::cmd_done)
{
	m_brush_addr_func &= ~0x8000;
}

TIMER_CALLBACK_MEMBER(dpb7000_state::execute_hdd_command)
{
	constexpr int SECTORS_PER_TRACK = 20480 / 256;
	int group = (int)(m_diskseq_cmd_word_from_cpu >> 5) & 3;
	int head_count = get_heads_for_disk_group(group);
	int head_index = m_diskseq_cmd_word_from_cpu & 0xf;
	int image_lba = SECTORS_PER_TRACK * head_count * (int)m_diskseq_cyl_from_cpu + SECTORS_PER_TRACK * head_index;

	if (m_hdd->exists())
	{
		if (m_diskseq_cyl_write_pending)
		{
			unsigned char sector_buffer[256];
			int start_sector = m_diskbuf_ram_addr >> 8;
			uint16_t ram_addr = m_diskbuf_ram_addr;
			if (m_diskseq_command_stride != 1)
			{
				for (int sector = start_sector; sector < 19200 / 256; sector++, image_lba++)
				{
					m_hdd->read(image_lba, sector_buffer);
					for (int stride_idx = 0; stride_idx < 256; stride_idx += 2)
					{
						sector_buffer[stride_idx] = m_diskbuf_ram[ram_addr];
						ram_addr += 2;
					}
					LOGMASKED(LOG_HDD, "Performing write to LBA %d: Cylinder %03x, head %x, command word %03x, Stride 2 (RAM address %04x, offset %04x)\n", image_lba, m_diskseq_cyl_from_cpu, head_index, m_diskseq_cmd_word_from_cpu, m_diskbuf_ram_addr, sector * 256);
					m_hdd->write(image_lba, sector_buffer);
				}
			}
			else
			{
				if (m_diskseq_cmd == 12 || m_diskseq_cmd == 14)
				{
					for (int sector = 0; sector < 19200 / 256; sector++, image_lba++)
					{
						LOGMASKED(LOG_HDD, "Performing write to LBA %d: Cylinder %03x, head %x, command word %03x\n", image_lba, m_diskseq_cyl_from_cpu, head_index, m_diskseq_cmd_word_from_cpu);
						for (int i = 0; i < 256; i += 2)
						{
							sector_buffer[i + 0] = process_byte_to_disc();
							sector_buffer[i + 1] = process_byte_to_disc();
						}
						m_hdd->write(image_lba, sector_buffer);
					}
				}
				else
				{
					for (int sector = start_sector; sector < 19200 / 256; sector++, image_lba++)
					{
						LOGMASKED(LOG_HDD, "Performing write to LBA %d: Cylinder %03x, head %x, command word %03x (RAM address %04x, offset %04x)\n", image_lba, m_diskseq_cyl_from_cpu, head_index, m_diskseq_cmd_word_from_cpu, m_diskbuf_ram_addr, sector * 256);
						m_hdd->write(image_lba, m_diskbuf_ram + sector * 256);
					}
				}
			}
			m_diskseq_cyl_write_pending = false;
		}
		else if (m_diskseq_cyl_read_pending)
		{
			unsigned char sector_buffer[256];
			if (m_diskseq_command_stride != 1)
			{
				for (int sector = 0; sector < 19200 / 256; sector++, image_lba++)
				{
					LOGMASKED(LOG_HDD, "Performing read of LBA %d: Cylinder %03x, head %x, command word %03x\n", image_lba, m_diskseq_cyl_from_cpu, head_index, m_diskseq_cmd_word_from_cpu);
					m_hdd->read(image_lba, sector_buffer);
					for (int clear_idx = 0; clear_idx < 256; clear_idx += 2)
					{
						sector_buffer[clear_idx] = 0;
					}
					if (m_diskseq_cmd == 4 || m_diskseq_cmd == 6)
					{
						for (int i = 0; i < 256; i++)
						{
							process_byte_from_disc(sector_buffer[i]);
						}
					}
					else
					{
						memcpy(m_diskbuf_ram + sector * 256, sector_buffer, 256);
					}
				}
			}
			else
			{
				int start_sector = BIT(m_diskseq_cmd, 2) ? 0 : (m_diskbuf_ram_addr >> 8);
				int partial_bytes = m_diskbuf_ram_addr & 0x00ff;
				if (partial_bytes && !BIT(m_diskseq_cmd, 2))
				{
					LOGMASKED(LOG_HDD, "Performing partial read of sector into disk buffer address %04x\n", m_diskbuf_ram_addr);
					m_hdd->read(image_lba, sector_buffer);
					memcpy(m_diskbuf_ram + m_diskbuf_ram_addr, sector_buffer + partial_bytes, 0x100 - partial_bytes);
					m_diskbuf_ram_addr += 0x100;
					m_diskbuf_ram_addr &= 0xff00;
					image_lba += start_sector;
				}

				for (int sector = start_sector; sector < 19200 / 256; sector++, image_lba++)
				{
					LOGMASKED(LOG_HDD, "Performing read of LBA %d: Cylinder %03x, head %x, command word %03x\n", image_lba, m_diskseq_cyl_from_cpu, head_index, m_diskseq_cmd_word_from_cpu);
					if (BIT(m_diskseq_cmd, 2))
					{
						m_hdd->read(image_lba, sector_buffer);
						for (int i = 0; i < 256; i++)
						{
							process_byte_from_disc(sector_buffer[i]);
						}
					}
					else
					{
						m_hdd->read(image_lba, m_diskbuf_ram + sector * 256);
					}
				}
			}
			m_diskseq_cyl_read_pending = false;
		}
		m_diskseq_command_stride = 2;
	}

	m_diskseq_use_hdd_pending = false;
	req_b_w(1);
}

uint16_t dpb7000_state::cpu_sysctrl_r()
{
	const uint16_t ctrl = m_sys_ctrl &~ SYSCTRL_AUTO_START;
	const uint16_t auto_start = m_auto_start->read() ? SYSCTRL_AUTO_START : 0;
	const uint16_t ret = ctrl | auto_start;
	//LOGMASKED(LOG_SYS_CTRL, "%s: CPU read from System Control: %04x\n", machine().describe_context(), ret);
	return ret;
}

void dpb7000_state::cpu_sysctrl_w(uint16_t data)
{
	const uint16_t mask = (SYSCTRL_REQ_A_EN | SYSCTRL_REQ_B_EN);
	LOGMASKED(LOG_IRQ, "%s: CPU to Control Bus write: %04x\n", machine().describe_context(), data);
	m_sys_ctrl &= ~mask;
	m_sys_ctrl |= (data & mask);

	update_req_irqs();
}

void dpb7000_state::update_req_irqs()
{
	const bool take_irq_a = (m_sys_ctrl & SYSCTRL_REQ_A_IN) && (m_sys_ctrl & SYSCTRL_REQ_A_EN);
	const bool take_irq_b = (m_sys_ctrl & SYSCTRL_REQ_B_IN) && (m_sys_ctrl & SYSCTRL_REQ_B_EN);
	if (take_irq_a)
	{
		LOGMASKED(LOG_IRQ, "Flagging to take IRQ A\n");
	}
	m_maincpu->set_input_line(5, take_irq_a ? ASSERT_LINE : CLEAR_LINE);
	if (take_irq_b)
	{
		LOGMASKED(LOG_IRQ, "Flagging to take IRQ B\n");
	}
	m_maincpu->set_input_line(4, take_irq_b ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t dpb7000_state::fdd_ctrl_r()
{
	// D4: Command Tag Flag
	// D5: Restore Flag
	// D6: Cylinder Tag Flag
	// D7: Debug Switch
	const uint8_t ret = m_fdd_ctrl;
	m_fdd_ctrl &= ~0x70;
	LOGMASKED(LOG_FDC_CTRL, "%s: Floppy CPU Ctrl Read: %02x\n", machine().describe_context(), ret);
	return ret;
}

void dpb7000_state::advance_line_count()
{
	m_line_length = 0;
	m_line_count++;
	toggle_line_clock();
}

void dpb7000_state::toggle_line_clock()
{
	m_line_clock ^= 1;
}

void dpb7000_state::process_sample()
{
	const uint16_t x = m_size_dest_x;
	const uint16_t y = m_size_dest_y;

	//m_size_dsx_ddx = 0x100;
	//m_size_dsy_ddy = 0x100;

	//if (m_size_dsx_ddx != 0x100) printf("Processing sample %d,%d (%04x:%04x, %04x:%04x) LC:%d dxdx:%06x, dydy:%06x\n", x, y, m_bxlen_counter, m_bxlen, m_bylen_counter, m_bylen, m_line_count, m_size_dxdx_counter, m_size_dydy_counter);
	bool process_sample = m_size_dxdx_counter <= 0 && m_size_dydy_counter <= 0;
	if (process_sample)
	{
		switch (m_brush_addr_cmd)
		{
			case 2: // Brush Store Write
				LOGMASKED(LOG_BRUSH_WRITES, "Processing %02x,%02x into Brush Store L/C/E at %d, %d\n", m_incoming_lum, m_incoming_chr, x, y);
				if (BIT(m_brush_addr_func, 7))
					m_brushstore_lum[y * 256 + x] = m_incoming_lum;
				if (BIT(m_brush_addr_func, 8))
					m_brushstore_chr[y * 256 + x] = m_incoming_chr;
				if (BIT(m_brush_addr_func, 9))
					m_brushstore_ext[y * 256 + x] = m_incoming_lum;
				break;

			case 4: // Framestore Write
			{
				uint32_t pix_idx = y * 800 + x;
				if (!BIT(m_brush_addr_func, 5))
				{
					if (BIT(m_brush_addr_func, 7))
						m_framestore_lum[0][pix_idx] = m_incoming_lum;
					if (BIT(m_brush_addr_func, 8))
						m_framestore_chr[0][pix_idx] = m_incoming_chr;
					if (BIT(m_brush_addr_func, 9))
						m_framestore_ext[0][pix_idx] = m_incoming_lum;
				}
				if (!BIT(m_brush_addr_func, 6))
				{
					if (BIT(m_brush_addr_func, 7))
						m_framestore_lum[1][pix_idx] = m_incoming_lum;
					if (BIT(m_brush_addr_func, 8))
						m_framestore_chr[1][pix_idx] = m_incoming_chr;
					if (BIT(m_brush_addr_func, 9))
						m_framestore_ext[1][pix_idx] = m_incoming_lum;
				}
				break;
			}
		}
	}

	if (m_size_dxdx_counter <= 0)
	{
		m_size_dxdx_counter += 0x100;//(int16_t)(m_size_dsx_ddx & 0x7fff);
		m_size_dest_x++;
	}
	m_size_dxdx_counter -= 0x100;

	m_bxlen_counter++;
	if (m_bxlen_counter == 0x1000)
	{
		//if (m_size_dsx_ddx != 0x100) printf("Advancing to next line due to bxlen counter elapsing\n");
		m_bxlen_counter = m_bxlen;
		m_bylen_counter++;

		if (m_size_dydy_counter <= 0)
		{
			m_size_dydy_counter += 0x100;//(int16_t)(m_size_dsy_ddy & 0x7fff);
			m_size_dest_y++;
			//if (m_size_dsx_ddx != 0x100) printf("dydy counter is <= 0, resetting dydy counter to %04x, dxdx counter to 0, dest X to %d, dest Y to %d\n", m_size_dydy_counter, m_size_dest_x, m_size_dest_y);
		}
		m_size_dydy_counter -= 0x100;

		m_size_dxdx_counter = 0;
		m_size_dest_x = (m_brush_addr_cmd != 2) ? m_cxpos[1] : 0;

		//if (m_size_dsx_ddx != 0x100) printf("At end of line, dydy counter is now %04x\n", m_size_dydy_counter);

		if (m_bylen_counter == 0x1000)
		{
			m_diskseq_cyl_read_pending = false;
			cmd_done(0);
		}
	}
}

uint8_t dpb7000_state::process_byte_to_disc()
{
	if (!m_diskseq_cyl_write_pending)
		return 0;

	uint8_t lum = 0;
	uint8_t chr = 0;
	switch (m_brush_addr_cmd)
	{
		case 3: // Framestore Read
		{
			if (!BIT(m_brush_addr_func, 5))
			{
				const uint16_t x = m_cxpos[0] + (m_bxlen_counter - m_bxlen);
				const uint16_t y = m_cypos[0] + (m_bylen_counter - m_bylen);
				uint32_t pix_idx = y * 800 + x;

				if (BIT(m_brush_addr_func, 9))
				{
					lum = m_framestore_ext[0][pix_idx];
					LOGMASKED(LOG_STORE_READS, "Reading %02x from Store 1 Stencil at %d, %d to disk [%03x %03x]\n", lum, x, y, m_bxlen_counter, m_bylen_counter);
				}
				if (BIT(m_brush_addr_func, 8))
				{
					chr = m_framestore_chr[0][pix_idx];
					LOGMASKED(LOG_STORE_READS, "Reading %02x from Store 1 Chroma at %d, %d to disk [%03x %03x]\n", chr, x, y, m_bxlen_counter, m_bylen_counter);
				}
				if (BIT(m_brush_addr_func, 7))
				{
					lum = m_framestore_lum[0][pix_idx];
					LOGMASKED(LOG_STORE_READS, "Reading %02x from Store 1 Luma at %d, %d to disk [%03x %03x]\n", lum, x, y, m_bxlen_counter, m_bylen_counter);
				}
			}

			if (!BIT(m_brush_addr_func, 6))
			{
				const uint16_t x = m_cxpos[1] + (m_bxlen_counter - m_bxlen);
				const uint16_t y = m_cypos[1] + (m_bylen_counter - m_bylen);
				uint32_t pix_idx = y * 800 + x;

				if (BIT(m_brush_addr_func, 9))
				{
					lum = m_framestore_ext[1][pix_idx];
					LOGMASKED(LOG_STORE_READS, "Reading %02x from Store 2 Stencil at %d, %d to disk [%03x %03x]\n", lum, x, y, m_bxlen_counter, m_bylen_counter);
				}
				if (BIT(m_brush_addr_func, 8))
				{
					chr = m_framestore_chr[1][pix_idx];
					LOGMASKED(LOG_STORE_READS, "Reading %02x from Store 2 Chroma at %d, %d to disk [%03x %03x]\n", chr, x, y, m_bxlen_counter, m_bylen_counter);
				}
				if (BIT(m_brush_addr_func, 7))
				{
					lum = m_framestore_lum[1][pix_idx];
					LOGMASKED(LOG_STORE_READS, "Reading %02x from Store 2 Luma at %d, %d to disk [%03x %03x]\n", lum, x, y, m_bxlen_counter, m_bylen_counter);
				}
			}
			break;
		}
	}

	const uint8_t ret = m_buffer_lum ? lum : chr;

	m_buffer_lum = !m_buffer_lum;
	if (m_buffer_lum)
	{
		m_line_length++;
		if (m_line_length == 0x300)
		{
			advance_line_count();
		}

		m_bxlen_counter++;
		if (m_bxlen_counter == 0x1000)
		{
			m_bxlen_counter = m_bxlen;
			m_bylen_counter++;
			if (m_bylen_counter == 0x1000)
			{
				if (m_brush_addr_cmd == 3)
				{
					LOGMASKED(LOG_STORE_READS, "Done with Store Read command\n");
				}
				m_diskseq_cyl_write_pending = false;
				cmd_done(0);
			}
		}
	}
	return ret;
}

void dpb7000_state::process_byte_from_disc(uint8_t data_byte)
{
	if (!m_diskseq_cyl_read_pending)
		return;

	if (m_buffer_lum)
	{
		m_incoming_lum = data_byte;
	}
	else
	{
		m_incoming_chr = data_byte;
		process_sample();
	}

	m_buffer_lum = !m_buffer_lum;
	m_line_length++;
	if (m_line_length == 0x300)
	{
		advance_line_count();
	}
}

bool dpb7000_state::is_disk_group_fdd(int group_index)
{
	constexpr uint16_t GROUP_SHIFTS[3] = { DGROUP_0_SHIFT, DGROUP_1_SHIFT, DGROUP_2_SHIFT };
	uint16_t val = (m_config_sw34->read() >> GROUP_SHIFTS[group_index]) & 7;
	return val == DGROUP_TYPE_FLOPPY;
}

bool dpb7000_state::is_disk_group_hdd(int group_index)
{
	constexpr uint16_t GROUP_SHIFTS[3] = { DGROUP_0_SHIFT, DGROUP_1_SHIFT, DGROUP_2_SHIFT };
	uint16_t val = (m_config_sw34->read() >> GROUP_SHIFTS[group_index]) & 7;
	return val == DGROUP_TYPE_80MB_CDC || val == DGROUP_TYPE_160MB_FUJITSU || val == DGROUP_TYPE_330MB_FUJITSU;
}

int dpb7000_state::get_heads_for_disk_group(int group_index)
{
	constexpr uint16_t GROUP_SHIFTS[3] = { DGROUP_0_SHIFT, DGROUP_1_SHIFT, DGROUP_2_SHIFT };
	uint16_t val = (m_config_sw34->read() >> GROUP_SHIFTS[group_index]) & 7;
	switch (val)
	{
		case DGROUP_TYPE_FLOPPY:
			return 1;
		case DGROUP_TYPE_80MB_CDC:
			return 5;
		case DGROUP_TYPE_160MB_FUJITSU:
			return 10;
		case DGROUP_TYPE_330MB_FUJITSU:
			return 16;
		default:
			return 0;
	}
}

void dpb7000_state::fdd_index_callback(floppy_image_device *floppy, int state)
{
	if (m_diskseq_use_hdd_pending)
		return;

	if (!state && m_diskseq_cyl_read_pending && m_floppy && m_fdd_side < 2)
	{
		LOGMASKED(LOG_COMMANDS, "Reading cylinder from floppy\n");
		m_fdd_pll.read_reset(machine().time());

		constexpr uint16_t PREGAP_MARK = 0xaaaa;
		constexpr uint16_t SYNC_MARK = 0x9125;

		m_floppy->ss_w(m_fdd_side);

		bool seen_pregap = false;
		bool in_track = false;
		int curr_bit = -1;
		uint16_t curr_window = 0;
		uint16_t bit_idx = 0;

		attotime tm = machine().time();
		attotime limit = machine().time() + attotime::from_ticks(1, 6); // One revolution at 360rpm on a Shugart SA850
		do
		{
			curr_bit = m_fdd_pll.get_next_bit(tm, m_floppy, limit);
			if (curr_bit < 0)
			{
				LOGMASKED(LOG_FDC_MECH, "Warning: Unable to retrieve full track %d side %d, curr_bit returned -1\n", m_floppy->get_cyl(), m_fdd_side);
			}
			else
			{
				curr_window <<= 1;
				curr_window |= curr_bit;
				bit_idx++;

				if (!seen_pregap && curr_window == PREGAP_MARK)
				{
					seen_pregap = true;
					bit_idx = 0;
					curr_window = 0;
				}
				else if (seen_pregap && !in_track && curr_window == SYNC_MARK)
				{
					in_track = true;
					bit_idx = 0;
					curr_window = 0;
				}
				else if (seen_pregap && in_track && bit_idx == 16)
				{
					uint8_t data_byte = (uint8_t)bitswap<16>((uint16_t)curr_window, 15, 13, 11, 9, 7, 5, 3, 1, 14, 12, 10, 8, 6, 4, 2, 0);
					switch (m_diskseq_cmd)
					{
					case 4: // Read Track
					case 6: // Disc Clear, Read Track
						m_diskbuf_data_count--;
						process_byte_from_disc(data_byte);
						if (!m_diskseq_cyl_read_pending)
						{
							curr_bit = -1;
							m_fdd_side = 2;
						}
						else if (m_fdd_side == 0 && m_diskbuf_data_count == 0)
						{
							curr_bit = -1;
							m_floppy->ss_w(1);
							m_fdd_side++;
							m_diskbuf_data_count = 0x2400;
						}
						else if (m_fdd_side == 1 && m_diskbuf_data_count == 0)
						{
							curr_bit = -1;
							m_fdd_side++;
						}
						break;
					case 0: // Read Track to Buffer RAM
					case 2: // Read Track, stride 2, to Buffer RAM
						if (BIT(m_diskseq_cmd, 1))
						{
							if (!BIT(m_diskbuf_ram_addr, 0))
							{
								m_diskbuf_ram[m_diskbuf_ram_addr >> 1] = data_byte;
							}
							m_diskbuf_ram_addr++;
						}
						else
						{
							m_diskbuf_ram[m_diskbuf_ram_addr] = data_byte;
							m_diskbuf_ram_addr++;
						}

						if (m_diskbuf_ram_addr >= 0x2700 && m_fdd_side == 0)
						{
							// If we've read the side 0 portion of the cylinder, yield out and wait for the next index pulse
							curr_bit = -1;
							m_floppy->ss_w(1);
							m_fdd_side++;
						}
						else if(m_diskbuf_ram_addr >= 0x4b00 && m_fdd_side == 1)
						{
							// If we've read the side 1 portion of the cylinder, yield out, we're done
							curr_bit = -1;
							m_fdd_side++;
						}
						break;
					}
					bit_idx = 0;
					curr_window = 0;
				}
			}
		} while (curr_bit != -1);
	}

	if (m_fdd_side == 2)
	{
		m_diskseq_cyl_read_pending = false;
		req_b_w(1);
	}
}

void dpb7000_state::seek_fdd_to_cylinder()
{
	uint16_t floppy_cyl = (uint16_t)m_floppy->get_cyl();
	if (floppy_cyl != m_diskseq_cyl_from_cpu && m_diskseq_cyl_from_cpu < 78 && m_floppy != nullptr)
	{
		if (m_diskseq_cyl_from_cpu < floppy_cyl)
		{
			m_floppy->dir_w(1);
			for (uint16_t i = m_diskseq_cyl_from_cpu; i < floppy_cyl; i++)
			{
				m_floppy->stp_w(1);
				m_floppy->stp_w(0);
				m_floppy->stp_w(1);
			}
		}
		else
		{
			m_floppy->dir_w(0);
			for (uint16_t i = floppy_cyl; i < m_diskseq_cyl_from_cpu; i++)
			{
				m_floppy->stp_w(1);
				m_floppy->stp_w(0);
				m_floppy->stp_w(1);
			}
		}
		LOGMASKED(LOG_FDD, "%s: New floppy cylinder: %04x\n", machine().describe_context(), floppy_cyl);
	}
}

void dpb7000_state::fddcpu_p1_w(uint8_t data)
{
	LOGMASKED(LOG_FDC_PORT, "%s: Floppy CPU Port 1 Write: %02x\n", machine().describe_context(), data);
	const uint8_t old_value = m_fdd_port1;
	m_fdd_port1 = data;

	floppy_image_device *newflop = m_floppy0->get_device();
	if (newflop != m_floppy)
	{
		if (m_floppy)
		{
			m_floppy->mon_w(1);
			m_floppy->setup_index_pulse_cb(floppy_image_device::index_pulse_cb());
		}
		if (newflop)
		{
			newflop->set_rpm(360);
			newflop->mon_w(BIT(old_value, 2));
			newflop->setup_index_pulse_cb(floppy_image_device::index_pulse_cb(&dpb7000_state::fdd_index_callback, this));
			m_fdd_track = newflop->get_cyl();
		}
		m_floppy = newflop;
	}

	if (m_floppy)
	{
		m_floppy->mon_w(BIT(m_fdd_port1, 2));
		m_floppy->dir_w(1 - BIT(m_fdd_port1, 1));
		m_floppy->stp_w(1 - BIT(m_fdd_port1, 0));
		m_fdd_track = m_floppy->get_cyl();

		if (m_fdd_track == 0)
		{
			m_fdd_ctrl |= 0x04; // On Cylinder
		}
		else
		{
			m_fdd_ctrl &= ~0x04;
		}
	}
	else
	{
		m_fdd_ctrl &= ~0x04;
	}

	// C5 D READY
	if (BIT(m_fdd_port1, 7))
	{
		m_diskseq_status &= ~(1 << 3);
		m_diskseq_status |= 0x04;
	}
	else
	{
		m_diskseq_status |= (1 << 3);
	}
}

void dpb7000_state::fddcpu_p2_w(uint8_t data)
{
	m_fdd_serial->write_txd(BIT(data, 4));
}

uint8_t dpb7000_state::fddcpu_p2_r()
{
	uint8_t ret = 0;
	if (m_fdd_debug_rx_byte_count)
	{
		ret |= m_fdd_debug_rx_bits[0] << 3;
		m_fdd_debug_rx_bits.pop_front();

		m_fdd_debug_rx_recv_count++;
		if (m_fdd_debug_rx_recv_count == 10)
		{
			m_fdd_debug_rx_recv_count = 0;
			m_fdd_debug_rx_byte_count--;
		}
	}
	else
	{
		ret |= 1 << 3;
	}
	return ret;
}

uint8_t dpb7000_state::fdd_cmd_r()
{
	LOGMASKED(LOG_FDC_CMD, "%s: Floppy CPU command read: %02x\n", m_diskseq_cmd_to_ctrl);
	return m_diskseq_cmd_to_ctrl;
}

void dpb7000_state::fddcpu_debug_rx(int state)
{
	if (m_fdd_debug_rx_bit_count < 10)
	{
		m_fdd_debug_rx_bits.push_back(state);
		m_fdd_debug_rx_bit_count++;
	}
	else
	{
		m_fdd_debug_rx_bit_count = 0;
		m_fdd_debug_rx_byte_count++;
	}
}

uint8_t dpb7000_state::keyboard_p1_r()
{
	LOGMASKED(LOG_KEYBC, "%s: Port 1 read\n", machine().describe_context());
	return 0;
}

uint8_t dpb7000_state::keyboard_p2_r()
{
	LOGMASKED(LOG_KEYBC, "%s: Port 2 read\n", machine().describe_context());
	return 0;
}

void dpb7000_state::keyboard_p1_w(uint8_t data)
{
	LOGMASKED(LOG_KEYBC, "%s: Port 1 write: %02x\n", machine().describe_context(), data);
	const uint8_t old_data = m_keybc_p1_data;
	m_keybc_p1_data = data;
	const uint8_t col = data & 0x0f;
	const uint8_t lowered = old_data & ~data;
	if (BIT(lowered, 7))
	{
		m_keybc_latched_bit = BIT(m_keybc_cols[col]->read(), (data >> 4) & 7);
	}
	else
	{
		m_keybc_latched_bit = 1;
	}
}

void dpb7000_state::keyboard_p2_w(uint8_t data)
{
	LOGMASKED(LOG_KEYBC, "%s: Port 2 write: %02x\n", machine().describe_context(), data);
	m_keybc_tx = BIT(~data, 7);
}

int dpb7000_state::keyboard_t0_r()
{
	LOGMASKED(LOG_KEYBC, "%s: T0 read\n", machine().describe_context());
	return 0;
}

int dpb7000_state::keyboard_t1_r()
{
	uint8_t data = m_keybc_latched_bit;
	//m_keybc_latched_bit = 0;
	LOGMASKED(LOG_KEYBC, "%s: T1 read: %d\n", machine().describe_context(), data);
	return data;
}

void dpb7000_state::tds_dac_w(uint8_t data)
{
	m_tds_dac_value = ~data;
	uint16_t offset = 0x67 * m_tds_dac_value;
	m_tds_dac_offset = (uint8_t)(offset >> 8);
	m_tds_dac_percent = std::clamp(m_tds_dac_value / 255.0f, 0.0f, 1.0f);
	LOGMASKED(LOG_TDS, "%s: TDS DAC Write: %02x (%f)\n", machine().describe_context(), data, m_tds_dac_percent);
}

TIMER_CALLBACK_MEMBER(dpb7000_state::tds_press_tick)
{
	m_tds_in_proximity = !m_pen_prox->read();

	switch (m_tds_press_state)
	{
	case STATE_IDLE:
		if (m_tds_in_proximity)
		{
			m_tds_press_state = STATE_PRESS_IN;
			m_tds_pressure_shift = 6;
		}
		break;
	case STATE_PRESS_IN:
		if (!m_tds_in_proximity)
			m_tds_press_state = STATE_PRESS_OUT;
		else if (m_tds_pressure_shift > 0)
			m_tds_pressure_shift--;
		else
			m_tds_press_state = STATE_PRESSED;
		break;
	case STATE_PRESSED:
		if (!m_tds_in_proximity)
			m_tds_press_state = STATE_PRESS_OUT;
		break;
	case STATE_PRESS_OUT:
		if (m_tds_in_proximity)
			m_tds_press_state = STATE_PRESS_IN;
		else if (m_tds_pressure_shift < 6)
			m_tds_pressure_shift++;
		else
			m_tds_press_state = STATE_IDLE;
		break;
	}
}

TIMER_CALLBACK_MEMBER(dpb7000_state::tds_adc_tick)
{
	m_tds_adc_busy = false;

	uint16_t value = m_tds_dac_offset;
	if (m_tds_press_state != STATE_IDLE)
	{
		value += (m_pen_press->read() >> m_tds_pressure_shift);
	}
	m_tds_adc_value = (value >= 0xf8 ? 0xf7 : (uint8_t)value);
}

void dpb7000_state::tds_convert_w(uint8_t data)
{
	LOGMASKED(LOG_TDS, "%s: TDS ADC Convert Start: %02x\n", machine().describe_context(), data);

	m_tds_adc_busy = true;
	m_tds_adc_value = 0x80;
	m_tds_adc_timer->adjust(attotime::from_ticks(8, 120000));
}

uint8_t dpb7000_state::tds_adc_r()
{
	LOGMASKED(LOG_TDS, "%s: TDS ADC Read: %02x (DAC value %02x)\n", machine().describe_context(), m_tds_adc_value, m_tds_dac_value);
	return m_tds_adc_value;
}

uint8_t dpb7000_state::tds_pen_switches_r()
{
	uint8_t data = 0;//m_pen_switches->read() << 4;
	LOGMASKED(LOG_TDS, "%s: TDS Pen Switches Read: %02x\n", machine().describe_context(), data);
	return data;
}

TIMER_CALLBACK_MEMBER(dpb7000_state::tablet_tx_tick)
{
	//m_tds_duart->rx_b_w(m_tablet_tx_bit);
	if (!m_tablet_hle_tx_bits.empty())
	{
		m_tds_duart->rx_b_w(m_tablet_hle_tx_bits[0]);
		m_tablet_hle_tx_bits.pop_front();
	}
	else
	{
		m_tds_duart->rx_b_w(1);
	}
}

void dpb7000_state::duart_b_w(int state)
{
	//printf("B%d ", state);
}

uint8_t dpb7000_state::tds_p1_r()
{
	const uint8_t latched = m_tds_p1_data & 0x80;
	const uint8_t adc_not_busy = m_tds_adc_busy ? 0x00 : 0x20;
	uint8_t data = m_tds_dips->read() | adc_not_busy | latched;
	LOGMASKED(LOG_TDS, "%s: TDS Port 1 Read, %02x\n", machine().describe_context(), data);
	return data;
}

uint8_t dpb7000_state::tds_p2_r()
{
	uint8_t data = 0x02 | (m_keybc_tx << 3);
	LOGMASKED(LOG_TDS, "%s: TDS Port 2 Read, %02x\n", machine().describe_context(), data);
	return data;
}

uint8_t dpb7000_state::tds_p3_r()
{
	LOGMASKED(LOG_TDS, "%s: TDS Port 3 Read, %02x\n", machine().describe_context(), 0);
	return 0;
}

uint8_t dpb7000_state::tds_p4_r()
{
	LOGMASKED(LOG_TDS, "%s: TDS Port 4 Read, %02x\n", machine().describe_context(), 0);
	return 0;
}

void dpb7000_state::tds_p1_w(uint8_t data)
{
	LOGMASKED(LOG_TDS, "%s: TDS Port 1 Write = %02x\n", machine().describe_context(), data);
	const uint8_t old = m_tds_p1_data;
	m_tds_p1_data = data;
	if (BIT(old & ~data, 0))
	{
		m_tds_duart->reset();
		m_tablet_tx_timer->adjust(attotime::from_hz(9600), 0, attotime::from_hz(9600));
	}
}

void dpb7000_state::tds_p2_w(uint8_t data)
{
	LOGMASKED(LOG_TDS, "%s: TDS Port 2 Write = %02x\n", machine().describe_context(), data);
}

/*INPUT_CHANGED_MEMBER(dpb7000_state::pen_prox_changed)
{
    m_tds_in_proximity = newval ? false : true;
    if (m_tds_in_proximity)
    {
        LOGMASKED(LOG_TABLET, "Triggering IRQ due to proximity\n");
        m_tablet_cpu->set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
    }
}*/

TIMER_CALLBACK_MEMBER(dpb7000_state::tablet_hle_tick)
{
	if (m_tds_press_state != STATE_IDLE || (m_pen_switches->read() == 1))
	{
		m_tablet_pen_x = m_pen_x->read();
		m_tablet_pen_y = 4799 - m_pen_y->read();
		m_tablet_pen_switches = m_pen_switches->read();
		uint8_t data[5] =
		{
			(uint8_t)(0x40 | (BIT(m_tablet_pen_switches, 1) << 2) | (BIT(m_tablet_pen_y, 12) << 1) | BIT(m_tablet_pen_x, 12)),
			(uint8_t)(m_tablet_pen_x & 0x003f),
			(uint8_t)((m_tablet_pen_x & 0x0fc0) >> 6),
			(uint8_t)(m_tablet_pen_y & 0x003f),
			(uint8_t)((m_tablet_pen_y & 0x0fc0) >> 6)
		};

		for (int i = 0; i < 5; i++)
		{
			data[i] |= (population_count_32(data[i]) & 1) ? 0x80 : 0x00;
			m_tablet_hle_tx_bits.push_back(0);
			for (int bit = 0; bit < 8; bit++)
			{
				m_tablet_hle_tx_bits.push_back(BIT(data[i], bit));
			}
			m_tablet_hle_tx_bits.push_back(1);
		}
	}
}

uint8_t dpb7000_state::tablet_p2_r(offs_t offset, uint8_t mem_mask)
{
	const uint8_t dip_bit = BIT(m_tablet_dip_shifter, 15) << 7;
	const uint8_t led_bit = (BIT(~m_tablet_p3_data, 5) << 6);
	const uint8_t pen_prox = m_pen_prox->read() << 5;
	uint8_t data = dip_bit | led_bit | pen_prox | (m_tablet_p2_data & 0x1f);

	LOGMASKED(LOG_TABLET, "%s: Tablet Port 2 Read = %02x & %02x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void dpb7000_state::tablet_p2_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	LOGMASKED(LOG_TABLET, "%s: Tablet Port 2 Write: %02x & %02x\n", machine().describe_context(), data, mem_mask);

	m_tablet_p2_data = data;

	if (mem_mask & 0x06)
	{
		m_tablet_mux = (data & 0x06) >> 1;
	}

	if (BIT(mem_mask, 4))
	{
		uint8_t old_drq = m_tablet_drq;
		m_tablet_drq = BIT(data, 4);
		LOGMASKED(LOG_TABLET, "Tablet DRQ: %d\n", m_tablet_drq);
		LOGMASKED(LOG_TABLET, "Clearing tablet CPU IRQ\n");
		m_tablet_cpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
		if (old_drq && !m_tablet_drq && m_tds_in_proximity) // DRQ transition to low
		{
			if (m_tablet_state == 0) // We're idle
			{
				m_tablet_state = 1; // We're reading
				m_tablet_cpu->set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
				LOGMASKED(LOG_TABLET, "Triggering IRQ for read (initial)\n");
			}
		}
		if (!old_drq && m_tablet_drq) // DRQ transition back to high
		{
			if (m_tablet_state == 1)
			{
				m_tablet_irq_timer->adjust(attotime::from_ticks(1, 1200000), 1);
				LOGMASKED(LOG_TABLET, "Setting up IRQ timer for read (continuous)\n");
			}
		}
	}
}

TIMER_CALLBACK_MEMBER(dpb7000_state::tablet_irq_tick)
{
	LOGMASKED(LOG_TABLET, "Triggering tablet CPU IRQ\n");
	m_tablet_cpu->set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
	m_tablet_cpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
}

uint8_t dpb7000_state::tablet_p3_r(offs_t offset, uint8_t mem_mask)
{
	uint8_t data = m_tablet_p3_data;
	LOGMASKED(LOG_TABLET, "%s: Tablet Port 3 Read = %02x & %02x\n", machine().describe_context(), data, mem_mask);
	return data;
}

void dpb7000_state::tablet_p3_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	LOGMASKED(LOG_TABLET, "%s: Tablet Port 3 Write: %02x & %02x\n", machine().describe_context(), data, mem_mask);

	if (BIT(mem_mask, 7))
	{
		m_tablet_tx_bit = BIT(data, 7);
	}

	const uint8_t old = m_tablet_p3_data;
	m_tablet_p3_data = data;

	const uint8_t lowered = old & ~data;
	const uint8_t raised = ~old & data;

	if (BIT(lowered, 5))
	{
		// HACK: We hard-code the DIPs to the config from a known tablet setup, as the meaning of the individual
		// DIP switches is undocumented.
		m_tablet_dip_shifter = 0x6e1e; // (m_tablet_dips[0]->read() << 8) | m_tablet_dips[1]->read();
	}

	if (BIT(raised, 6))
	{
		m_tablet_dip_shifter <<= 1;
	}
}

uint8_t dpb7000_state::tablet_rdh_r()
{
	uint8_t data = (m_tablet_counter_latch >> 8);
	LOGMASKED(LOG_TABLET, "%s: Tablet RDH Read (Mux %d): %02x\n", machine().describe_context(), m_tablet_mux, data);
	return data;
}

uint8_t dpb7000_state::tablet_rdl_r()
{
	//if (m_tablet_mux == 3)
		//m_tablet_state = 0;

	m_tablet_counter_latch = machine().rand() & 0xfff;
	LOGMASKED(LOG_TABLET, "%s: Random latch: %04x\n", machine().describe_context(), m_tablet_counter_latch);
	uint8_t data = (uint8_t)m_tablet_counter_latch;
	LOGMASKED(LOG_TABLET, "%s: Tablet RDL Read (Mux %d): %02x\n", machine().describe_context(), m_tablet_mux, data);
	return data;
}

uint32_t dpb7000_state::combined_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const uint16_t upper_flag_addr = (uint16_t)((m_output_cpflags & 3) << 8);

	// TODO: Implement the Store Address Card in a more accurate manner, and implement framestore scan-out and flag handling accordingly.
	int32_t y_addr[2] = { m_rvscr[0], m_rvscr[1] };
	int32_t dest_y = 0;
	uint8_t matte_chr[2][2] = { { m_matte_u[0], m_matte_v[0] }, { m_matte_u[1], m_matte_v[1] } };
	bool seen_rvr = false;
	bool started_scanout = false;
	for (int32_t y = 0; y < 625; y++)
	{
		const uint8_t vlines = m_output_vlines[y + 0x18f] & 0x0f;

		const uint8_t hlines = m_output_hlines[0xa0 + 70] & 0x0f;
		const uint16_t flag_addr = (hlines & 0x0f) | ((vlines & 0x0f) << 4);
		const uint8_t hflags = m_output_hflags[flag_addr | upper_flag_addr] & 0x0f;
		const bool palette = BIT(hflags, 3);

		if (!BIT(hflags, 1))
		{
			seen_rvr = true;
			if (palette)
			{
				y_addr[0] = 0;
				y_addr[1] = 0;
			}
			else
			{
				y_addr[0] = m_rvscr[0];
				y_addr[1] = m_rvscr[1];
			}
		}
		else if (!started_scanout && seen_rvr)
		{
			started_scanout = true;
			dest_y = 0;
		}

		// TODO: Enabling the blanking PAL causes strangeness when bringing up either the menu or the palette.
		// Most likely scenario is that assuming a horizontal coordinate of 0 coming into the PAL puts us in a blanking region.
		//const bool blank_1_q = BIT(m_storeaddr_pal_blank[((y_addr[0] << 3) & 0x1800) | ((y_addr[0] << 2) & 0x3f0)], 0);
		//const bool blank_2_q = BIT(m_storeaddr_pal_blank[((y_addr[1] << 3) & 0x1800) | ((y_addr[1] << 2) & 0x3f0)], 0);
		const bool blank_1_q = true;
		const bool blank_2_q = true;
		const bool use_store1_matte = !palette && (!blank_1_q || m_select_matte[0]);
		const bool use_store2_matte = !palette && (!blank_2_q || m_select_matte[1]);
		const bool use_ext_store1_matte = !(blank_1_q && !BIT(m_ext_store_flags, 0));
		const bool use_ext_store2_matte = !(blank_2_q && !BIT(m_ext_store_flags, 1));
		const bool invert_a = palette ? BIT(~m_ext_store_flags, 3) : BIT(m_ext_store_flags, 2);
		const bool invert_b = !invert_a;
		const uint8_t ext_src_invert = BIT(m_ext_store_flags, 4) ? 0xff : 0x00;
		const uint8_t ext_a_mask = (invert_a ? 0xff : 0x00);
		const uint8_t ext_b_mask = (invert_b ? 0xff : 0x00);

		const int32_t y_line1 = y_addr[0] * 800;
		const int32_t y_line2 = y_addr[1] * 800;

		uint32_t *d = &bitmap.pix(dest_y);
		int32_t x_addr[2] = { m_rhscr[0] & ~1, m_rhscr[1] & ~1 };
		for (int32_t x = 0; x < 800; x++, x_addr[0] = (x_addr[0] + 1) % 800, x_addr[1] = (x_addr[1] + 1) % 800)
		{
			const uint8_t uv_sel1 = x_addr[0] & 1;
			const uint8_t uv_sel2 = x_addr[1] & 1;

			const uint32_t pix_idx1 = y_line1 + x_addr[0];
			const uint32_t pix_idx2 = y_line2 + x_addr[1];

			const uint16_t lum1 = use_store1_matte ? m_matte_y[0] : m_framestore_lum[0][pix_idx1];
			const uint16_t chr1 = use_store1_matte ? matte_chr[0][uv_sel1] : m_framestore_chr[0][pix_idx1];
			const uint16_t ext1 = use_ext_store1_matte ? m_matte_ext[0] : (m_framestore_ext[0][pix_idx1] ^ ext_src_invert);

			const uint16_t lum2 = use_store2_matte ? m_matte_y[1] : m_framestore_lum[1][pix_idx2];
			const uint16_t chr2 = use_store2_matte ? matte_chr[0][uv_sel2] : m_framestore_chr[1][pix_idx2];
			const uint16_t ext2 = use_ext_store2_matte ? m_matte_ext[1] : (m_framestore_ext[1][pix_idx2] ^ ext_src_invert);

			const uint8_t ext_product = (uint8_t)(((ext1 * ext2) + 0x0080) >> 8);
			const uint8_t ext_a = ext_product ^ ext_a_mask;
			const uint8_t ext_b = ext_product ^ ext_b_mask;

			const uint16_t lum1_product = ((lum1 * ext_a) + 0x0080) >> 8;
			const uint16_t lum2_product = ((lum2 * ext_b) + 0x0080) >> 8;
			const uint16_t chr1_product = ((chr1 * ext_a) + 0x0080) >> 8;
			const uint16_t chr2_product = ((chr2 * ext_b) + 0x0080) >> 8;

			const uint8_t lum_sum = (uint8_t)std::min<uint16_t>(lum1_product + lum2_product, 255);
			const uint8_t chr_sum = (uint8_t)std::min<uint16_t>(chr1_product + chr2_product, 255);

			*d++ = 0xff000000 | (lum_sum << 8) | chr_sum;
		}

		if (BIT(hflags, 1))
		{
			dest_y++;

			y_addr[0]++;
			if (y_addr[0] == 4096)
				y_addr[0] = 0;

			y_addr[1]++;
			if (y_addr[1] == 4096)
				y_addr[1] = 0;
		}
	}

	const uint32_t cursor_rgb = m_yuv_lut[(m_cursor_u << 16) | (m_cursor_v << 8) | m_cursor_y];
	uint16_t cursor_origin_y_counter = m_cursor_origin_y;
	uint16_t cursor_y_size = m_cursor_size_y;
	uint16_t cursor_y_index = 0;

	for (int32_t dst_y = 0; dst_y < 625; dst_y++)
	{
		uint16_t cursor_origin_x_counter = m_cursor_origin_x;
		uint16_t cursor_x_size = m_cursor_size_x;
		uint16_t cursor_x_index = 0;

		uint32_t *d = &bitmap.pix(dst_y);
		for (int32_t x = 0; x < 800; x += 2)
		{
			const uint32_t u = (d[x] << 16) & 0xff0000;
			const uint32_t v = (d[x | 1] << 8) & 0x00ff00;
			const uint32_t y0 = (d[x] >> 8) & 0x0000ff;
			const uint32_t y1 = (d[x | 1] >> 8) & 0x0000ff;

			uint32_t combined[2] = { m_yuv_lut[u | v | y0], m_yuv_lut[u | v | y1] };
			for (int32_t h = 0; h < 2; h++)
			{
				if (m_output_csflags)
				{
					const bool cursor_in_window = (cursor_origin_x_counter == 0x1000 && cursor_origin_y_counter == 0x1000);
					const bool cursor_enable = cursor_in_window && (cursor_x_index < 0x20 && cursor_y_index < 0x20);
					if (cursor_enable)
					{
						const uint16_t cursor_addr_x = cursor_x_index ^ 0x10;
						const uint16_t cursor_addr_y = cursor_y_index ^ 0x10;
						const uint16_t cursor_addr = (cursor_addr_y << 5) | cursor_addr_x;

						const uint8_t cursor_data = m_output_cursor[cursor_addr];

						bool cursor_bit = true;
						bool cursor_colored = true;
						switch (m_output_csflags)
						{
						case 1:
							cursor_bit = BIT(cursor_data, 0);
							cursor_colored = BIT(cursor_data, 1);
							break;
						case 2:
							cursor_bit = BIT(cursor_data, 2);
							cursor_colored = BIT(cursor_data, 3);
							break;
						case 3:
							cursor_bit = ((cursor_addr_x & cursor_addr_y) & 0x10) ? 0 : 1;
							break;
						default:
							// Cursor inactive
							break;
						}

						if (!cursor_bit)
						{
							combined[h] = cursor_colored ? cursor_rgb : 0xff000000;
						}
					}

					if (cursor_origin_x_counter < 0x1000)
					{
						cursor_origin_x_counter++;
					}
					else if (cursor_x_size < 0x1000)
					{
						cursor_x_size++;
						cursor_x_index++;
					}
					else if (cursor_x_index < 0x20)
					{
						cursor_x_index++;
					}
				}

				d[x + h] = combined[h];
			}
		}

		if (m_output_csflags)
		{
			if (cursor_origin_y_counter < 0x1000)
			{
				cursor_origin_y_counter++;
			}
			else if (cursor_y_size < 0x1000)
			{
				cursor_y_size++;
				cursor_y_index++;
			}
			else if (cursor_y_index < 0x20)
			{
				cursor_y_index++;
			}
		}
	}

	return 0;
}

template <int StoreNum>
uint32_t dpb7000_state::store_debug_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (0)
	{
		for (int py = 0; py < 768; py++)
		{
			const uint8_t *src_lum = &m_framestore_lum[StoreNum][py * 800];
			const uint8_t *src_chr = &m_framestore_chr[StoreNum][py * 800];
			uint32_t *dst = &bitmap.pix(py);
			for (int px = 0; px < 800; px += 2)
			{
				const uint32_t u = *src_chr++ << 16;
				const uint32_t v = *src_chr++ << 8;
				*dst++ = m_yuv_lut[u | v | *src_lum++];
				*dst++ = m_yuv_lut[u | v | *src_lum++];
			}
		}
	}
	return 0;
}

uint32_t dpb7000_state::stencil_debug_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (0)
	{
		for (int py = 0; py < 768; py++)
		{
			const uint8_t *src_ext1 = &m_framestore_ext[0][py * 800];
			const uint8_t *src_ext2 = &m_framestore_ext[1][py * 800];
			uint32_t *dst = &bitmap.pix(py);
			for (int px = 0; px < 800; px++, src_ext1++, src_ext2++)
			{
				const uint16_t h = (uint8_t)px >> 4;
				const uint16_t pal_blank_addr = ((h << 12) & 0xe000) | ((py << 3) & 0x1800) | (BIT(h, 0) << 10) | ((py << 2) & 0x3f0) | ((h >> 4) & 0xf);
				const uint8_t blank_val = m_storeaddr_pal_blank[pal_blank_addr];
				const uint32_t blank_color = ((blank_val & 3) != 3) ? 0x000000ff : 0;
				*dst++ = 0xff000000 | (*src_ext1 << 16) | (*src_ext2 << 8) | blank_color;
			}
		}
	}
	return 0;
}

uint32_t dpb7000_state::brush_debug_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if (0)
	{
		for (int y = 0; y < 256; y++)
		{
			const uint8_t *src_lum = &m_brushstore_lum[y * 256];
			const uint8_t *src_chr = &m_brushstore_chr[y * 256];
			const uint8_t *src_ext = &m_brushstore_ext[y * 256];
			uint32_t *dst_lc = &bitmap.pix(y);
			uint32_t *dst_ext = &bitmap.pix(y + 256);
			for (int x = 0; x < 256; x++)
			{
				dst_lc[x] = (0xff << 24) | (src_lum[x] << 16) | (src_lum[x] << 8) | src_lum[x];
				dst_lc[x + 256] = (0xff << 24) | (src_chr[x] << 16) | (src_chr[x] << 8) | src_chr[x];
				dst_ext[x] = (0xff << 24) | (src_ext[x] << 16) | (src_ext[x] << 8) | src_ext[x];
			}
		}
	}
	return 0;
}

static void dpb7000_floppies(device_slot_interface &device)
{
	device.option_add("8", FLOPPY_8_DSDD);
}

void dpb7000_state::dpb7000(machine_config &config)
{
	// Computer Card 1 & 2
	M68000(config, m_maincpu, 16_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &dpb7000_state::main_map);

	INPUT_MERGER_ANY_HIGH(config, m_p_int).output_handler().set_inputline(m_maincpu, 3);

	ACIA6850(config, m_acia[0], 0);
	m_acia[0]->txd_handler().set(m_rs232[0], FUNC(rs232_port_device::write_txd));
	m_acia[0]->rts_handler().set(m_rs232[0], FUNC(rs232_port_device::write_rts));
	m_acia[0]->irq_handler().set_inputline(m_maincpu, 6);

	ACIA6850(config, m_acia[1], 0);
	m_acia[1]->txd_handler().set(m_tds_duart, FUNC(scn2681_device::rx_a_w));
	m_acia[1]->irq_handler().set(m_p_int, FUNC(input_merger_device::in_w<0>));

	ACIA6850(config, m_acia[2], 0);
	m_acia[2]->txd_handler().set(m_rs232[1], FUNC(rs232_port_device::write_txd));
	m_acia[2]->rts_handler().set(m_rs232[1], FUNC(rs232_port_device::write_rts));
	m_acia[2]->irq_handler().set(m_p_int, FUNC(input_merger_device::in_w<1>));

	RS232_PORT(config, m_rs232[0], default_rs232_devices, nullptr);
	m_rs232[0]->rxd_handler().set(m_acia[0], FUNC(acia6850_device::write_rxd));
	m_rs232[0]->dsr_handler().set(m_acia[0], FUNC(acia6850_device::write_dcd));
	m_rs232[0]->cts_handler().set(m_acia[0], FUNC(acia6850_device::write_cts));

	RS232_PORT(config, m_rs232[1], default_rs232_devices, nullptr);
	m_rs232[1]->rxd_handler().set(m_acia[2], FUNC(acia6850_device::write_rxd));
	m_rs232[1]->dcd_handler().set(m_acia[2], FUNC(acia6850_device::write_dcd));
	m_rs232[1]->cts_handler().set(m_acia[2], FUNC(acia6850_device::write_cts));

	COM8116(config, m_brg, 5.0688_MHz_XTAL);   // K1355A/B
	m_brg->ft_handler().set(m_acia[0], FUNC(acia6850_device::write_txc));
	m_brg->ft_handler().append(m_acia[0], FUNC(acia6850_device::write_rxc));
	m_brg->ft_handler().append(m_acia[1], FUNC(acia6850_device::write_txc));
	m_brg->ft_handler().append(m_acia[1], FUNC(acia6850_device::write_rxc));
	m_brg->ft_handler().append(m_acia[2], FUNC(acia6850_device::write_txc));
	m_brg->ft_handler().append(m_acia[2], FUNC(acia6850_device::write_rxc));

	screen_device &combined_screen(SCREEN(config, "combined_screen", SCREEN_TYPE_RASTER));
	combined_screen.set_refresh_hz(50);
	combined_screen.set_size(800, 625);
	combined_screen.set_visarea(0, 709, 0, 574);
	combined_screen.set_screen_update(FUNC(dpb7000_state::combined_screen_update));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_size(696, 276);
	screen.set_visarea(56, 695, 36, 275);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	if (0)
	{
		screen_device &store_screen1(SCREEN(config, "store_screen1", SCREEN_TYPE_RASTER));
		store_screen1.set_refresh_hz(50);
		store_screen1.set_size(800, 768);
		store_screen1.set_visarea(0, 799, 0, 767);
		store_screen1.set_screen_update(FUNC(dpb7000_state::store_debug_screen_update<0>));

		screen_device &store_screen2(SCREEN(config, "store_screen2", SCREEN_TYPE_RASTER));
		store_screen2.set_refresh_hz(50);
		store_screen2.set_size(800, 768);
		store_screen2.set_visarea(0, 799, 0, 767);
		store_screen2.set_screen_update(FUNC(dpb7000_state::store_debug_screen_update<1>));

		screen_device &ext_screen(SCREEN(config, "ext_screen", SCREEN_TYPE_RASTER));
		ext_screen.set_refresh_hz(50);
		ext_screen.set_size(800, 768);
		ext_screen.set_visarea(0, 799, 0, 767);
		ext_screen.set_screen_update(FUNC(dpb7000_state::stencil_debug_screen_update));

		screen_device &brush_screen(SCREEN(config, "brush_screen", SCREEN_TYPE_RASTER));
		brush_screen.set_refresh_hz(50);
		brush_screen.set_size(512, 512);
		brush_screen.set_visarea(0, 511, 0, 511);
		brush_screen.set_screen_update(FUNC(dpb7000_state::brush_debug_screen_update));
	}

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	// The 6545's clock is driven by the QD output of a 4-bit binary counter, which has its preset wired to 0010.
	// It therefore operates as a divide-by-six counter for the master clock of 22.248MHz.
	SY6545_1(config, m_crtc, 22.248_MHz_XTAL / 6);
	m_crtc->set_char_width(8);
	m_crtc->set_show_border_area(false);
	m_crtc->set_screen("screen");
	m_crtc->set_update_row_callback(FUNC(dpb7000_state::crtc_update_row));
	m_crtc->set_on_update_addr_change_callback(FUNC(dpb7000_state::crtc_addr_changed));

	// Floppy Disc Unit
	M6803(config, m_fddcpu, 4.9152_MHz_XTAL);
	m_fddcpu->set_addrmap(AS_PROGRAM, &dpb7000_state::fddcpu_map);
	m_fddcpu->out_p1_cb().set(FUNC(dpb7000_state::fddcpu_p1_w));
	m_fddcpu->in_p2_cb().set(FUNC(dpb7000_state::fddcpu_p2_r));
	m_fddcpu->out_p2_cb().set(FUNC(dpb7000_state::fddcpu_p2_w));

	FLOPPY_CONNECTOR(config, m_floppy0, dpb7000_floppies, "8", floppy_image_device::default_mfm_floppy_formats);

	RS232_PORT(config, m_fdd_serial, default_rs232_devices, nullptr);
	m_fdd_serial->rxd_handler().set(FUNC(dpb7000_state::fddcpu_debug_rx));

	config.set_perfect_quantum(m_fddcpu);

	// Hard Disk
	HARDDISK(config, "hdd", 0);

	// Keyboard
	I8039(config, m_keybcpu, 4.608_MHz_XTAL);
	m_keybcpu->set_addrmap(AS_PROGRAM, &dpb7000_state::keybcpu_map);
	m_keybcpu->p1_in_cb().set(FUNC(dpb7000_state::keyboard_p1_r));
	m_keybcpu->p2_in_cb().set(FUNC(dpb7000_state::keyboard_p2_r));
	m_keybcpu->p1_out_cb().set(FUNC(dpb7000_state::keyboard_p1_w));
	m_keybcpu->p2_out_cb().set(FUNC(dpb7000_state::keyboard_p2_w));
	m_keybcpu->t0_in_cb().set(FUNC(dpb7000_state::keyboard_t0_r));
	m_keybcpu->t1_in_cb().set(FUNC(dpb7000_state::keyboard_t1_r));

	// TDS Box
	M6803(config, m_tds_cpu, 4.9152_MHz_XTAL);
	m_tds_cpu->set_addrmap(AS_PROGRAM, &dpb7000_state::tds_cpu_map);
	m_tds_cpu->in_p1_cb().set(FUNC(dpb7000_state::tds_p1_r));
	m_tds_cpu->out_p1_cb().set(FUNC(dpb7000_state::tds_p1_w));
	m_tds_cpu->in_p2_cb().set(FUNC(dpb7000_state::tds_p2_r));
	m_tds_cpu->out_p2_cb().set(FUNC(dpb7000_state::tds_p2_w));
	m_tds_cpu->in_p3_cb().set(FUNC(dpb7000_state::tds_p3_r));
	m_tds_cpu->in_p4_cb().set(FUNC(dpb7000_state::tds_p4_r));

	SCN2681(config, m_tds_duart, 3.6864_MHz_XTAL);
	m_tds_duart->irq_cb().set_inputline(m_tds_cpu, M6803_IRQ1_LINE);
	m_tds_duart->a_tx_cb().set(m_acia[1], FUNC(acia6850_device::write_rxd));
	m_tds_duart->b_tx_cb().set(FUNC(dpb7000_state::duart_b_w));

	// Tablet
	Z8681(config, m_tablet_cpu, 7.3728_MHz_XTAL);
	m_tablet_cpu->set_addrmap(AS_PROGRAM, &dpb7000_state::tablet_program_map);
	m_tablet_cpu->set_addrmap(AS_DATA, &dpb7000_state::tablet_data_map);
	m_tablet_cpu->p2_in_cb().set(FUNC(dpb7000_state::tablet_p2_r));
	m_tablet_cpu->p2_out_cb().set(FUNC(dpb7000_state::tablet_p2_w));
	m_tablet_cpu->p3_in_cb().set(FUNC(dpb7000_state::tablet_p3_r));
	m_tablet_cpu->p3_out_cb().set(FUNC(dpb7000_state::tablet_p3_w));
}

ROM_START( dpb7000 )
	ROM_REGION16_BE(0xa0000, "monitor", 0)
	ROM_SYSTEM_BIOS( 0, "v406", "V4.06A" )
	ROMX_LOAD("01616a-nad-4af2.bin", 0x00001,  0x8000, CRC(a42eace6) SHA1(78c629a8afb48a95fc0a86ca762cc5b84bd9929b), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("01616a-ncd-58de.bin", 0x00000,  0x8000, CRC(f70fff2a) SHA1(a6f85d086a0c53d156eeeb157184ebcad4adecb3), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("01616a-mad-f512.bin", 0x10001,  0x8000, CRC(4c3e39f6) SHA1(443095c56481fbcadd4dcec1757d889c8f78805d), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("01616a-mcd-91f2.bin", 0x10000,  0x8000, CRC(4b6b6eb3) SHA1(1bef443d78197d33e44c708ead9604020881f67f), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("01616a-lad-0059.bin", 0x20001,  0x8000, CRC(0daf670d) SHA1(2342a43054ed141de298a1c1a6867949297bb52a), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("01616a-lcd-5639.bin", 0x20000,  0x8000, CRC(c8977d3f) SHA1(4ee9f3a883400b4771e6ae33c6e4edcd5c0b49e7), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("01616a-kad-1d9b.bin", 0x30001,  0x8000, CRC(bda7e309) SHA1(377edf2675a6736fe7ec775894858967b0e9247e), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("01616a-kcd-e51c.bin", 0x30000,  0x8000, CRC(aa05a5cc) SHA1(85dce335a72643f7640524b18cfe480a3c299f23), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("01616a-jad-47a8.bin", 0x40001,  0x8000, CRC(60fff4c9) SHA1(ba60281c0dd8627dffe07e7ea66f4eb688e74001), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("01616a-jcd-7825.bin", 0x40000,  0x8000, CRC(bb258ede) SHA1(ab8042391cd361bcd874b2f9d8fcaf20d4b2ebe7), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("01616a-iad-73a8.bin", 0x50001,  0x8000, CRC(98709fd2) SHA1(bd0f4689600e9fc49dbd8f2f326e18f8d602825e), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("01616a-icd-0562.bin", 0x50000,  0x8000, CRC(bab5274e) SHA1(3e51977da3dfe8fda089b9d2c3199acb4fed3212), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("01616a-had-d6fa.bin", 0x60001,  0x8000, CRC(70d791c5) SHA1(c281e4f27404e58ad5a80d6de1c5583cd9f3fe0e), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("01616a-hcd-9c0e.bin", 0x60000,  0x8000, CRC(938cb614) SHA1(ea7ea8a13e0ab1497691bab53090296ba51d271f), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("01616a-gad-397d.bin", 0x70001,  0x8000, CRC(0b95f9ed) SHA1(77126ee6c1f3dcdb8aa669ab74ff112e3f01918a), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("01616a-gcd-3ab8.bin", 0x70000,  0x8000, CRC(e9c21438) SHA1(1784ab2de1bb6023565b2e27872a0fcda25e1b1f), ROM_BIOS(0) | ROM_SKIP(1))
	ROM_SYSTEM_BIOS( 1, "v411", "V4.11A" )
	ROMX_LOAD("01993c-naa-0608.bin", 0x00001, 0x10000, CRC(9e4cfd96) SHA1(faae2ba849d30c894d9f929bae371ae76e05bdac), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("01993c-nca-3964.bin", 0x00000, 0x10000, CRC(04d75a15) SHA1(79f706296ba0c399b4e2d3acf947cae7706f9d63), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("01993c-maa-fa58.bin", 0x20001, 0x10000, CRC(88dc8ee0) SHA1(a2f6b80022ddb5adf687b678c621c8f5db1f3575), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("01993c-mca-b2b0.bin", 0x20000, 0x10000, CRC(16bf9db4) SHA1(a4696ee5a68b8a304c2433f16a6d57d7108f6406), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("01993c-laa-6be3.bin", 0x40001, 0x10000, CRC(56e693cc) SHA1(b4f3e0f8b635fe2b3fa975ed57f966e8ade7fbb1), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("01993c-lca-7c0d.bin", 0x40000, 0x10000, CRC(9540fa68) SHA1(79e46c4b6e86f6e53e3f5fea72e3be8c64de03af), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("01993c-kaa-3b12.bin", 0x60001, 0x10000, CRC(41e2ec71) SHA1(37112f878b3db24b198e878f5dba90fcbb4175d2), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("01993c-kca-f705.bin", 0x60000, 0x10000, CRC(00bfbd62) SHA1(17a5f2cbc91cabf1113c7d6e26124b67e5ece848), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("01993c-jaa-3cc9.bin", 0x80001, 0x10000, CRC(9e477845) SHA1(b89e5d2b69a6043452a56685a1227df86c83a328), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("01993c-jca-0b41.bin", 0x80000, 0x10000, CRC(58e8f302) SHA1(b37953046d81027b069b7a7a1834cd8c94fd9dca), ROM_BIOS(1) | ROM_SKIP(1))
	ROM_SYSTEM_BIOS( 2, "diags", "Diagnostics" )
	ROMX_LOAD("cat7000_diagnostic_rom_merged.bin", 0x00000, 0x20000, CRC(666767d6) SHA1(1dad9747321302bca7e03728c63034070f9ebf13), ROM_BIOS(2) )

	ROM_REGION(0x1000, "vduchar", 0)
	ROM_LOAD("bw14char.ic1",  0x0000, 0x1000, BAD_DUMP CRC(f9dd68b5) SHA1(50132b759a6d84c22c387c39c0f57535cd380411))

	ROM_REGION(0x700, "diskseq_ucode", 0)
	ROM_LOAD("17704a-hga-256.bin", 0x000, 0x100, CRC(ab59d8fd) SHA1(6dcbbb48838a5e370e22a708cea44e3db80d6505))
	ROM_LOAD("17704a-hfa-256.bin", 0x100, 0x100, CRC(f2475396) SHA1(b525ba58d37128cadf1e7285fb421c14e2495587))
	ROM_LOAD("17704a-hea-256.bin", 0x200, 0x100, CRC(0a19cc11) SHA1(13b72368d7cb5b7f685adfa07f07029026426b80))
	ROM_LOAD("17704a-hda-256.bin", 0x300, 0x100, CRC(a431cdf6) SHA1(027eea87ccafde727208277b40e3a3f88433343a))
	ROM_LOAD("17704a-hca-256.bin", 0x400, 0x100, CRC(610ef462) SHA1(f495647cc8420b7470ad68bccc069370f8f2af93))
	ROM_LOAD("17704a-hba-256.bin", 0x500, 0x100, CRC(94c16baf) SHA1(13baef1359bf92a8ebb9a0c39f4223e810c3cdc1))
	ROM_LOAD("17704a-haa-256.bin", 0x600, 0x100, CRC(0080f2a9) SHA1(63c7b31e5f65cc6e2c5fc67883c84284652cb4a7))

	ROM_REGION(0x700, "diskseq_prom", 0)
	ROM_LOAD("17704a-dfa-32.bin", 0x00, 0x20, CRC(ce5b8b46) SHA1(49a1e619f52101b6078e2f51d82ce5947fe2c011))

	ROM_REGION(0x800, "fddprom", 0)
	ROM_LOAD("17446a-gd-m2716.bin", 0x000, 0x800, CRC(a0be00ca) SHA1(48c4f8c07b9f6bc9b68698e1e326782e0b01e1b0))

	ROM_REGION(0x400, "output_timing_cursor", 0)
	ROM_LOAD("pb-037-17418-cda.bin", 0x000, 0x400, CRC(a31f3793) SHA1(4e74e528088c155e2c2592fa937e4cabfe6324c8))

	ROM_REGION(0x400, "output_timing_hlines", 0)
	ROM_LOAD("pb-037-17418-bga.bin", 0x000, 0x400, CRC(3b2c3635) SHA1(2038d616dd7f65ba55497bd037b0ad69aaa801ed))

	ROM_REGION(0x400, "output_timing_hflags", 0)
	ROM_LOAD("pb-037-17418-bea.bin", 0x000, 0x400, CRC(644e82a3) SHA1(d7634e03809abe2db924571c05821c1b2aca051b))

	ROM_REGION(0x400, "output_timing_vlines", 0)
	ROM_LOAD("pb-037-17418-dfa.bin", 0x000, 0x400, CRC(ca2ec308) SHA1(4232f44ea5bd3fa240eaf7c14e4b925140f90a1e))

	ROM_REGION(0x200, "output_timing_vflags", 0)
	ROM_LOAD("pb-037-17418-bfa.bin", 0x000, 0x100, CRC(a4486aac) SHA1(3834d550da3f1865b921807300a94612149f69d4))

	ROM_REGION(0x800, "keyboard", 0)
	ROM_LOAD("etc2716 63b2.bin", 0x000, 0x800, CRC(04614a50) SHA1(e547458f2c9cf29cf52f02b8824b32e5e91807fd))

	ROM_REGION(0x2000, "tds", 0)
	ROM_LOAD("hn482764g.bin", 0x0000, 0x2000, CRC(3626059c) SHA1(1a4f5c8b337f31c7b2b93096b59234ffbc2f1f00))

	ROM_REGION(0x2000, "tablet", 0)
	ROM_LOAD("nmc27c64q.bin", 0x0000, 0x2000, CRC(a453928f) SHA1(f4a25298fb446f0046c6f9f3ce70e7169dcebd01))

	ROM_REGION(0x800, "brushaddr_pal", 0)
	ROM_LOAD("pb-029-17419a-bea.bin", 0x000, 0x800, CRC(d30015c3) SHA1(fb5856df3ace452dfb1c1882b2adb6562e3a6800))

	ROM_REGION(0x200, "brushproc_prom", 0)
	ROM_LOAD("pb-02c-17593-baa.bin", 0x000, 0x200, CRC(6e31339f) SHA1(72a92d97412be19c884c3b854f7d9831435391a5))

	ROM_REGION16_BE(0x800, "brushproc_pal", 0)
	ROMX_LOAD("pb-02c-17593-hba.bin", 0x000, 0x800, CRC(76018e4f) SHA1(73d995e2e78410676061d45857756d5305a9984a), ROM_GROUPWORD)

	ROM_REGION(0x100, "brushstore_pal", 0)
	ROM_LOAD("pb-02a-17421-ada.bin", 0x000, 0x100, CRC(84bf7029) SHA1(9d58322994f6f7e99a9c6478577559c8171670ed))

	ROM_REGION(0x400, "framestore_back_pal", 0)
	ROM_LOAD("pb-02f-01748a-aba.bin", 0x000, 0x400, CRC(24b31494) SHA1(f9185a00e5470ec95d234a76c15acbf33cfb285d))

	ROM_REGION(0x400, "framestore_front_pal", 0)
	ROM_LOAD("pb-02f-01748a-bba.bin", 0x000, 0x400, CRC(8f06b632) SHA1(233b841c3957a6df229f3a693f9288cb8feec58c))

	ROM_REGION(0x100, "filter_signalprom", 0)
	ROM_LOAD("pb-027-17427a-dab.bin", 0x000, 0x100, CRC(6deac5cc) SHA1(184c34267e1b390bad0ca4a94befceece979e677))

	ROM_REGION(0x200, "filter_multprom", 0)
	ROM_LOAD("pb-027-17427a-afa-bfa.bin", 0x000, 0x200, CRC(58164a20) SHA1(c32997350bae22be075a8eee7fe4d1bdd8a34fd2))

	ROM_REGION(0x100, "size_pal", 0)
	ROM_LOAD("pb-012-17428a-aca.bin", 0x000, 0x100, CRC(0003b9ec) SHA1(8484e4c3b57069c5166201a7cb43d028c87e06cc))

	ROM_REGION(0x200, "size_prom_yh", 0)
	ROM_LOAD("pb-012-17428a-fda.bin", 0x000, 0x200, CRC(159418f4) SHA1(4e4c9be6d2e3ccdfdf8edfcb5cad106887e03fd2))

	ROM_REGION(0x200, "size_prom_yl", 0)
	ROM_LOAD("pb-012-17428a-gda.bin", 0x000, 0x200, CRC(1207291a) SHA1(ea44cf510169fdba2cfb0570a508fb3b0c93b06e))

	ROM_REGION(0x200, "size_prom_xh", 0)
	ROM_LOAD("pb-012-17428a-ecb.bin", 0x000, 0x200, CRC(1e324bf1) SHA1(efd88294dcb9fba58b2bb2b8d4715ba2c16f511d))

	ROM_REGION(0x200, "size_prom_xl", 0)
	ROM_LOAD("pb-012-17428a-gca.bin", 0x000, 0x200, CRC(ec7d26f3) SHA1(8bdbec33218f903294c135554d61271fa18d1a8d))

	ROM_REGION(0x10000, "storeaddr_pal_blank", 0)
	ROM_LOAD("pb-032-17425b-igb.bin", 0x00000, 0x10000, CRC(cdd80590) SHA1(fecb64695b61e8ec740af1480240088d5447688d))

	ROM_REGION(0x400, "storeaddr_prom_xlnib", 0)
	ROM_LOAD("pb-032-17425b-bbb.bin", 0x000, 0x400, CRC(2051a6e4) SHA1(3bd8a9015e77b034a94fe072a9753649b76f9f69))

	ROM_REGION(0x400, "storeaddr_prom_xmnib", 0)
	ROM_LOAD("pb-032-17425b-bcb.bin", 0x000, 0x400, CRC(01aaa6f7) SHA1(e31bff0c68f74996368443bfb58a3524a838f270))

	ROM_REGION(0x400, "storeaddr_prom_xhnib", 0)
	ROM_LOAD("pb-032-17425b-bdb.bin", 0x000, 0x400, CRC(20e2fb9e) SHA1(c4c77ec02ab6d3a1a28edf5543e57235a64a9d8d))

	ROM_REGION(0x400, "storeaddr_prom_protx", 0)
	ROM_LOAD("pb-032-17425b-deb.bin", 0x000, 0x400, CRC(faeb44dd) SHA1(3eaf981245824332d216e97095bdc02ff04e4800))

	ROM_REGION(0x400, "storeaddr_prom_proty", 0)
	ROM_LOAD("pb-032-17425b-edb.bin", 0x000, 0x400, CRC(83585876) SHA1(7c244adcd365f7b2ec347255100fa3597857905c))
ROM_END

} // anonymous namespace

COMP( 1981, dpb7000, 0, 0, dpb7000, dpb7000, dpb7000_state, empty_init, "Quantel", "DPB-7000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
