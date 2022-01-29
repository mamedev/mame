// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    DPB-7001 (c) 1981 Quantel

    Skeleton Driver

***************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6800/m6801.h"
#include "cpu/mcs48/mcs48.h"
#include "cpu/z8/z8.h"
#include "imagedev/floppy.h"
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
#include "emupal.h"
#include "screen.h"
#include <deque>

#define LOG_UNKNOWN         (1 << 0)
#define LOG_UCODE           (1 << 1)
#define LOG_MORE_UCODE      (1 << 2)
#define LOG_CSR             (1 << 3)
#define LOG_CTRLBUS         (1 << 4)
#define LOG_SYS_CTRL        (1 << 5)
#define LOG_FDC_CTRL        (1 << 6)
#define LOG_FDC_PORT        (1 << 7)
#define LOG_FDC_CMD         (1 << 8)
#define LOG_FDC_MECH        (1 << 9)
#define LOG_OUTPUT_TIMING   (1 << 10)
#define LOG_BRUSH_ADDR      (1 << 11)
#define LOG_STORE_ADDR      (1 << 12)
#define LOG_COMBINER        (1 << 13)
#define LOG_SIZE_CARD       (1 << 14)
#define LOG_FILTER_CARD     (1 << 15)
#define LOG_KEYBC           (1 << 16)
#define LOG_TDS             (1 << 17)
#define LOG_TABLET          (1 << 18)
#define LOG_ALL             (LOG_UNKNOWN | LOG_CSR | LOG_CTRLBUS | LOG_SYS_CTRL | LOG_FDC_CTRL | LOG_FDC_PORT | LOG_FDC_CMD | LOG_FDC_MECH | LOG_BRUSH_ADDR | \
							 LOG_STORE_ADDR | LOG_COMBINER | LOG_SIZE_CARD | LOG_FILTER_CARD | LOG_TABLET)

#define VERBOSE             (LOG_TABLET)
#include "logmacro.h"

static const uint16_t temp_pen_reads[4] = { 573, 840, 1320, 1360 };

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
		, m_diskseq(*this, "diskseq")
		, m_diskseq_ucode(*this, "diskseq_ucode")
		, m_diskseq_prom(*this, "diskseq_prom")
		, m_fddcpu(*this, "fddcpu")
		, m_fdd_serial(*this, "fddserial")
		, m_floppy0(*this, "0")
		, m_floppy(nullptr)
		, m_keybcpu(*this, "keybcpu")
		, m_keybc_cols(*this, "KEYB_COL%u", 0U)
		, m_tds_cpu(*this, "tds")
		, m_tds_duart(*this, "tds_duart")
		, m_tds_dips(*this, "TDSDIPS")
		, m_pen_switches(*this, "PENSW")
		, m_tablet_cpu(*this, "tablet")
		, m_tablet_dips(*this, "TABDIP%u", 0U)
		, m_pen_prox(*this, "PENPROX")
		, m_pen_x(*this, "PENX")
		, m_pen_y(*this, "PENY")
		, m_filter_signalprom(*this, "filter_signalprom")
		, m_filter_multprom(*this, "filter_multprom")
		, m_filter_signal(nullptr)
		, m_filter_mult(nullptr)
		, m_size_yl(*this, "filter_de")
		, m_size_yh(*this, "filter_df")
		, m_size_xl(*this, "filter_dg")
		, m_size_xh(*this, "filter_dh")
	{
	}

	void dpb7000(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(pen_prox_changed);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param) override;

	template <int StoreNum> uint32_t store_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	static constexpr device_timer_id TIMER_DISKSEQ_COMPLETE = 0;
	static constexpr device_timer_id TIMER_FIELD_IN = 1;
	static constexpr device_timer_id TIMER_FIELD_OUT = 2;
	static constexpr device_timer_id TIMER_TABLET_TX = 3;
	static constexpr device_timer_id TIMER_TABLET_IRQ = 4;

	void main_map(address_map &map);
	void fddcpu_map(address_map &map);
	void keybcpu_map(address_map &map);
	void tds_cpu_map(address_map &map);
	void tablet_program_map(address_map &map);
	void tablet_data_map(address_map &map);

	uint16_t bus_error_r(offs_t offset);
	void bus_error_w(offs_t offset, uint16_t data);

	void csr_w(uint8_t data);
	uint8_t csr_r();

	uint16_t cpu_ctrlbus_r();
	void cpu_ctrlbus_w(uint16_t data);

	DECLARE_WRITE_LINE_MEMBER(req_a_w);
	DECLARE_WRITE_LINE_MEMBER(req_b_w);

	void fdd_index_callback(floppy_image_device *floppy, int state);
	uint8_t fdd_ctrl_r();
	uint8_t fdd_cmd_r();
	void fddcpu_p1_w(uint8_t data);
	uint8_t fddcpu_p2_r();
	void fddcpu_p2_w(uint8_t data);
	DECLARE_WRITE_LINE_MEMBER(fddcpu_debug_rx);

	void handle_command(uint16_t data);
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

	void diskseq_y_w(uint16_t data);
	void diskseq_tick();

	void advance_line_count();
	void toggle_line_clock();
	void process_sample();
	void process_byte_from_disc(uint8_t data_byte);

	required_device<m68000_base_device> m_maincpu;
	required_device_array<acia6850_device, 3> m_acia;
	required_device<input_merger_device> m_p_int;
	required_device<com8116_device> m_brg;
	required_device_array<rs232_port_device, 2> m_rs232;
	required_device<sy6545_1_device> m_crtc;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint16_t> m_vdu_ram;
	required_memory_region m_vdu_char_rom;
	required_ioport m_baud_dip;
	required_ioport m_auto_start;

	required_ioport m_config_sw12;
	required_ioport m_config_sw34;

	required_device<am2910_device> m_diskseq;
	required_memory_region m_diskseq_ucode;
	required_memory_region m_diskseq_prom;

	required_device<m6803_cpu_device> m_fddcpu;
	required_device<rs232_port_device> m_fdd_serial;
	std::deque<int> m_fdd_debug_rx_bits;
	uint32_t m_fdd_debug_rx_bit_count;
	uint32_t m_fdd_debug_rx_byte_count;
	uint32_t m_fdd_debug_rx_recv_count;
	uint8_t m_fdd_ctrl;
	uint8_t m_fdd_port1;
	uint8_t m_fdd_track;
	uint8_t m_fdd_side;
	fdc_pll_t m_fdd_pll;

	required_device<floppy_connector> m_floppy0;
	floppy_image_device *m_floppy;

	emu_timer *m_diskseq_complete_clk;
	emu_timer *m_field_in_clk;
	emu_timer *m_field_out_clk;

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

	enum : uint8_t
	{
		DSEQ_STATUS_READY_BIT           = 0,    // C5
		DSEQ_STATUS_FAULT_BIT           = 1,    // C6
		DSEQ_STATUS_ONCYL_BIT           = 2,    // C7
		DSEQ_STATUS_SKERR_BIT           = 3,    // C8
		DSEQ_STATUS_INDEX_BIT           = 4,    // C9
		DSEQ_STATUS_SECTOR_BIT          = 5,    // C10
		DSEQ_STATUS_AMFND_BIT           = 6,    // C11
		DSEQ_STATUS_WTPROT_BIT          = 7,    // C12
		DSEQ_STATUS_SELECTED_BIT        = 8,    // C13
		DSEQ_STATUS_SEEKEND_BIT         = 9,    // C14
		DSEQ_STATUS_SYNC_DET_BIT        = 10,   // C15
		DSEQ_STATUS_RAM_ADDR_OVFLO_BIT  = 11,   // C16

		DSEQ_CTRLOUT_CK_SEL_1       = (1 << 0), // S40
		DSEQ_CTRLOUT_CK_SEL_0       = (1 << 1), // S41
		DSEQ_CTRLOUT_ADDR_W_PERMIT  = (1 << 2), // S42
		DSEQ_CTRLOUT_ZERO_RAM       = (1 << 3), // S43
		DSEQ_CTRLOUT_WRITE_RAM      = (1 << 4), // S44
		DSEQ_CTRLOUT_WORD_READ_RAM  = (1 << 5), // S45
		DSEQ_CTRLOUT_WRITE_SYNC     = (1 << 6), // S46
		DSEQ_CTRLOUT_SYNC_DET_EN    = (1 << 7), // S47

		DSEQ_CTRLOUT_WRITE_ZERO     = (1 << 5), // S53
		DSEQ_CTRLOUT_LINE_CK        = (1 << 6), // S54
		DSEQ_CTRLOUT_DISC_CLEAR     = (1 << 7), // S55
	};

	// Computer Card
	uint8_t m_csr;
	uint16_t m_sys_ctrl;

	// Disc Sequencer Card
	int m_diskseq_cp;
	bool m_diskseq_reset;
	bool m_diskseq_halt;
	uint8_t m_diskseq_line_cnt;         // EF/EE
	uint8_t m_diskseq_ed_cnt;           // ED
	uint8_t m_diskseq_head_cnt;         // EC
	uint16_t m_diskseq_cyl_from_cpu;    // AE/BH
	uint16_t m_diskseq_cmd_word_from_cpu; // DD/CC
	uint8_t m_diskseq_cmd;
	uint8_t m_diskseq_cyl_to_ctrl;
	uint8_t m_diskseq_cmd_to_ctrl;
	uint8_t m_diskseq_status_in;        // CG
	uint8_t m_diskseq_status_out;       // BC
	uint8_t m_diskseq_ucode_latch[7];   // GG/GF/GE/GD/GC/GB/GA
	uint8_t m_diskseq_cc_inputs[4];     // Inputs to FE/FD/FC/FB
	bool m_diskseq_cyl_read_pending;

	// Disc Data Buffer Card
	uint16_t m_diskbuf_ram_addr;
	uint8_t m_diskbuf_ram[14 * 0x800];
	uint16_t m_diskbuf_data_count;

	// Output Timing Card
	uint16_t m_cursor_origin_x;
	uint16_t m_cursor_origin_y;
	uint16_t m_cursor_size_x;
	uint16_t m_cursor_size_y;

	// Brush Address Card
	uint8_t m_line_clock;
	uint16_t m_line_count;
	uint16_t m_line_length;
	uint16_t m_brush_addr_func;
	uint8_t m_bif;
	uint8_t m_bixos;
	uint8_t m_biyos;
	uint16_t m_bxlen;
	uint16_t m_bylen;
	uint16_t m_bxlen_counter;
	uint16_t m_bylen_counter;
	uint8_t m_plum;
	uint8_t m_pchr;
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

	// Combiner Card
	uint8_t m_cursor_y;
	uint8_t m_cursor_u;
	uint8_t m_cursor_v;
	uint8_t m_invert_mask;
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
	DECLARE_READ_LINE_MEMBER(keyboard_t0_r);
	DECLARE_READ_LINE_MEMBER(keyboard_t1_r);
	uint8_t m_keybc_latched_bit;
	uint8_t m_keybc_p1_data;
	uint8_t m_keybc_tx;

	// TDS Box
	required_device<m6803_cpu_device> m_tds_cpu;
	required_device<scn2681_device> m_tds_duart;
	required_ioport m_tds_dips;
	required_ioport m_pen_switches;
	uint8_t tds_p1_r();
	uint8_t tds_p2_r();
	void tds_p1_w(uint8_t data);
	void tds_p2_w(uint8_t data);
	void tds_dac_w(uint8_t data);
	void tds_convert_w(uint8_t data);
	uint8_t tds_adc_r();
	uint8_t tds_pen_switches_r();
	DECLARE_WRITE_LINE_MEMBER(duart_b_w);
	void tablet_tx_tick();
	uint8_t m_tds_dac_value;
	uint8_t m_tds_adc_value;
	uint8_t m_tds_p1_data;

	// Tablet
	required_device<z8681_device> m_tablet_cpu;
	required_ioport_array<2> m_tablet_dips;
	required_ioport m_pen_prox;
	required_ioport m_pen_x;
	required_ioport m_pen_y;
	uint8_t tablet_p2_r(offs_t offset, uint8_t mem_mask);
	void tablet_p2_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t tablet_p3_r(offs_t offset, uint8_t mem_mask);
	void tablet_p3_w(offs_t offset, uint8_t data, uint8_t mem_mask);
	void tablet_irq_tick();
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
	bool m_tablet_pen_in_proximity;
	uint8_t m_tablet_state;

	// Filter Card
	required_memory_region m_filter_signalprom;
	required_memory_region m_filter_multprom;
	uint8_t *m_filter_signal;
	uint8_t *m_filter_mult;
	uint8_t m_filter_acbc[16];
	uint8_t m_filter_abbb[16];
	uint8_t m_incoming_lum;
	uint8_t m_incoming_chr;
	bool m_buffer_lum;

	// Size Card
	required_device<am2901b_device> m_size_yl;
	required_device<am2901b_device> m_size_yh;
	required_device<am2901b_device> m_size_xl;
	required_device<am2901b_device> m_size_xh;

	uint8_t m_size_h;
	uint8_t m_size_v;

	// Utility
	std::unique_ptr<uint32_t[]> m_yuv_lut;
};

void dpb7000_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom().region("monitor", 0);
	map(0xb00000, 0xb7ffff).rw(FUNC(dpb7000_state::bus_error_r), FUNC(dpb7000_state::bus_error_w));
	map(0xb80000, 0xbfffff).ram();
	map(0xffd000, 0xffd3ff).rw(FUNC(dpb7000_state::bus_error_r), FUNC(dpb7000_state::bus_error_w));
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
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N) PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K) PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J) PORT_CHAR('j') PORT_CHAR('J')

	PORT_START("KEYB_COL1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B) PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7 \' @") PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('\'') PORT_CHAR('@')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H) PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T) PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G) PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("KEYB_COL2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R) PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V) PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C) PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F) PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E) PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D')

	PORT_START("KEYB_COL3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P) PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(". : >") PORT_CODE(KEYCODE_STOP) PORT_CHAR ('.') PORT_CHAR(':') PORT_CHAR('>')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\" = #") PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\"') PORT_CHAR('=')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('d')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR('-')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED) // 0x16 - arrow key?
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O) PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L) PORT_CHAR('l') PORT_CHAR('L')

	PORT_START("KEYB_COL4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W) PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X) PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('\\')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z) PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2 \" _") PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('\"') PORT_CHAR('_')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q) PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A')

	PORT_START("KEYB_COL5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED) // 0x1b - arrow key?
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEYB_COL6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('^') PORT_CHAR('|')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE) PORT_CHAR('~')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED) // 0x1a - arrow key?
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('\x19') PORT_CHAR('*')

	PORT_START("KEYB_COL7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED) // 0x0b
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED) // 0x09
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Esc") PORT_CODE(KEYCODE_ESC) // 0x7f
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Delete") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR('\x08') // 0x08
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED) // 0x7f
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED) //PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\x0d')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CODE(KEYCODE_ENTER) PORT_CHAR('\x0d')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_UNUSED) // PORT_NAME("Carriage Return") PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR('\x0d')

	PORT_START("PENSW")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON2) PORT_NAME("Pen Button 1") PORT_CODE(MOUSECODE_BUTTON2)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_BUTTON3) PORT_NAME("Pen Button 2") PORT_CODE(MOUSECODE_BUTTON3)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON4) PORT_NAME("Pen Button 3") PORT_CODE(MOUSECODE_BUTTON4)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_BUTTON5) PORT_NAME("Pen Button 4") PORT_CODE(MOUSECODE_BUTTON5)
	PORT_BIT(0xf0, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("PENPROX")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("Pen Proximity") PORT_CODE(MOUSECODE_BUTTON1) PORT_CHANGED_MEMBER(DEVICE_SELF, dpb7000_state, pen_prox_changed, 0)
	PORT_BIT(0xfe, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("PENX")
	PORT_BIT( 0xffff, 5000, IPT_LIGHTGUN_X) PORT_NAME("Pen X") PORT_MINMAX(0, 10000) PORT_SENSITIVITY(50) PORT_CROSSHAIR(X, 1.0, 0.0, 0)

	PORT_START("PENY")
	PORT_BIT( 0xffff, 5000, IPT_LIGHTGUN_Y) PORT_NAME("Pen Y") PORT_MINMAX(0, 10000) PORT_SENSITIVITY(50) PORT_CROSSHAIR(Y, 1.0, 0.0, 0)

	PORT_START("TDSDIPS")
	PORT_DIPNAME(0x08, 0x00, "TDS Box Encoding")
	PORT_DIPSETTING(   0x08, "Binary")
	PORT_DIPSETTING(   0x00, "ASCII")
	PORT_DIPNAME(0x10, 0x00, "TDS Box Mode")
	PORT_DIPSETTING(   0x10, "Normal")
	PORT_DIPSETTING(   0x00, "Monitor")
	PORT_DIPNAME(0x40, 0x00, "TDS Box Standard")
	PORT_DIPSETTING(   0x40, "NTSC")
	PORT_DIPSETTING(   0x00, "PAL")
	PORT_BIT(0xa7, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("TABDIP0")
	PORT_DIPNAME(0x01, 0x00, "Tablet SW1:1");
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x01, DEF_STR( On ))
	PORT_DIPNAME(0x02, 0x00, "Tablet SW1:2");
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x02, DEF_STR( On ))
	PORT_DIPNAME(0x04, 0x00, "Tablet SW1:3");
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x04, DEF_STR( On ))
	PORT_DIPNAME(0x08, 0x00, "Tablet SW1:4");
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x08, DEF_STR( On ))
	PORT_DIPNAME(0x10, 0x00, "Tablet SW1:5");
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x10, DEF_STR( On ))
	PORT_DIPNAME(0x20, 0x00, "Tablet SW1:6");
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x20, DEF_STR( On ))
	PORT_DIPNAME(0x40, 0x00, "Tablet SW1:7");
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x40, DEF_STR( On ))
	PORT_DIPNAME(0x80, 0x00, "Tablet SW1:8");
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x80, DEF_STR( On ))

	PORT_START("TABDIP1")
	PORT_DIPNAME(0x01, 0x00, "Tablet SW2:1");
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x01, DEF_STR( On ))
	PORT_DIPNAME(0x02, 0x00, "Tablet SW2:2");
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x02, DEF_STR( On ))
	PORT_DIPNAME(0x04, 0x00, "Tablet SW2:3");
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x04, DEF_STR( On ))
	PORT_DIPNAME(0x08, 0x00, "Tablet SW2:4");
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x08, DEF_STR( On ))
	PORT_DIPNAME(0x10, 0x00, "Tablet SW2:5");
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x10, DEF_STR( On ))
	PORT_DIPNAME(0x20, 0x00, "Tablet SW2:6");
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x20, DEF_STR( On ))
	PORT_DIPNAME(0x40, 0x00, "Tablet SW2:7");
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x40, DEF_STR( On ))
	PORT_DIPNAME(0x80, 0x00, "Tablet SW2:8");
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x80, DEF_STR( On ))

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
	PORT_DIPNAME(0x0001, 0x0000, "Auto-Start")
	PORT_DIPSETTING(     0x0000, DEF_STR( Off ))
	PORT_DIPSETTING(     0x0001, DEF_STR( On ))
	PORT_BIT(0xfffe, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("CONFIGSW12")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "NTSC/PAL" )
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
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
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
	PORT_DIPNAME( 0x001c, 0x0018, "Disc Group 0 Type" )
	PORT_DIPSETTING(    0x001c, "None" )
	PORT_DIPSETTING(    0x0018, "160Mb Fujitsu" )
	PORT_DIPSETTING(    0x0014, "330Mb Fujitsu" )
	PORT_DIPSETTING(    0x0010, "80Mb Fujitsu" )
	PORT_DIPSETTING(    0x000c, "Floppy Disc" )
	PORT_DIPSETTING(    0x0008, "NTSC/Floppy/PAL/Conv" )
	PORT_DIPSETTING(    0x0004, "80Mb CDC Removable" )
	PORT_DIPSETTING(    0x0000, "160Mb CDC Fixed" )
	PORT_DIPNAME( 0x0060, 0x0060, "Disc Group 1 Count" )
	PORT_DIPSETTING(    0x0060, "One" )
	PORT_DIPSETTING(    0x0040, "Two" )
	PORT_DIPSETTING(    0x0020, "Three" )
	PORT_DIPSETTING(    0x0000, "Four" )
	PORT_DIPNAME( 0x0380, 0x0180, "Disc Group 1 Type" )
	PORT_DIPSETTING(    0x0380, "None" )
	PORT_DIPSETTING(    0x0300, "160Mb Fujitsu" )
	PORT_DIPSETTING(    0x0280, "330Mb Fujitsu" )
	PORT_DIPSETTING(    0x0200, "80Mb Fujitsu" )
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
	PORT_DIPSETTING(    0x4000, "80Mb Fujitsu" )
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

	m_field_in_clk = timer_alloc(TIMER_FIELD_IN);
	m_field_in_clk->adjust(attotime::never);

	m_field_out_clk = timer_alloc(TIMER_FIELD_OUT);
	m_field_out_clk->adjust(attotime::never);

	// Disc Sequencer Card
	save_item(NAME(m_diskseq_cp));
	save_item(NAME(m_diskseq_reset));
	save_item(NAME(m_diskseq_halt));
	save_item(NAME(m_diskseq_line_cnt));
	save_item(NAME(m_diskseq_ed_cnt));
	save_item(NAME(m_diskseq_head_cnt));
	save_item(NAME(m_diskseq_cyl_from_cpu));
	save_item(NAME(m_diskseq_cmd_word_from_cpu));
	save_item(NAME(m_diskseq_cmd));
	save_item(NAME(m_diskseq_cyl_to_ctrl));
	save_item(NAME(m_diskseq_cmd_to_ctrl));
	save_item(NAME(m_diskseq_status_in));
	save_item(NAME(m_diskseq_status_out));
	save_item(NAME(m_diskseq_ucode_latch));
	save_item(NAME(m_diskseq_cc_inputs));
	save_item(NAME(m_diskseq_cyl_read_pending));
	m_diskseq_complete_clk = timer_alloc(TIMER_DISKSEQ_COMPLETE);
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

	// Brush Address Card
	save_item(NAME(m_line_clock));
	save_item(NAME(m_line_count));
	save_item(NAME(m_line_length));
	save_item(NAME(m_brush_addr_func));
	save_item(NAME(m_bif));
	save_item(NAME(m_bixos));
	save_item(NAME(m_biyos));
	save_item(NAME(m_bxlen));
	save_item(NAME(m_bylen));
	save_item(NAME(m_bxlen_counter));
	save_item(NAME(m_bylen_counter));
	save_item(NAME(m_plum));
	save_item(NAME(m_pchr));

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
	save_item(NAME(m_invert_mask));
	save_item(NAME(m_select_matte));
	save_item(NAME(m_matte_ext));
	save_item(NAME(m_matte_y));
	save_item(NAME(m_matte_u));
	save_item(NAME(m_matte_v));

	// Size Card
	save_item(NAME(m_size_h));
	save_item(NAME(m_size_v));

	// Filter Card
	m_filter_signal = m_filter_signalprom->base();
	m_filter_mult = m_filter_multprom->base();
	save_item(NAME(m_filter_acbc));
	save_item(NAME(m_filter_abbb));
	save_item(NAME(m_incoming_lum));
	save_item(NAME(m_incoming_chr));
	save_item(NAME(m_buffer_lum));

	// Keyboard
	save_item(NAME(m_keybc_latched_bit));
	save_item(NAME(m_keybc_p1_data));
	save_item(NAME(m_keybc_tx));

	// TDS Box
	save_item(NAME(m_tds_dac_value));
	save_item(NAME(m_tds_adc_value));

	// Tablet
	save_item(NAME(m_tablet_p2_data));
	save_item(NAME(m_tablet_p3_data));
	save_item(NAME(m_tablet_dip_shifter));
	save_item(NAME(m_tablet_counter_latch));
	save_item(NAME(m_tablet_tx_bit));
	save_item(NAME(m_tablet_pen_in_proximity));
	save_item(NAME(m_tablet_state));
	m_tablet_tx_timer = timer_alloc(TIMER_TABLET_TX);
	m_tablet_tx_timer->adjust(attotime::never);
	m_tablet_irq_timer = timer_alloc(TIMER_TABLET_IRQ);
	m_tablet_irq_timer->adjust(attotime::never);

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
	// Computer Card
	m_brg->stt_w(m_baud_dip->read());
	m_csr = 0;
	m_sys_ctrl = SYSCTRL_REQ_B_IN;
	for (int i = 0; i < 3; i++)
	{
		m_acia[i]->write_cts(0);
		m_acia[i]->write_dcd(0);
	}

	m_field_in_clk->adjust(attotime::from_hz(59.94), 0, attotime::from_hz(59.94));
	m_field_out_clk->adjust(attotime::from_hz(59.94) + attotime::from_hz(15734.0 / 1.0), 0, attotime::from_hz(59.94));

	// Disc Sequencer Card
	m_diskseq_cp = 0;
	m_diskseq_reset = false;
	m_diskseq_halt = true;
	m_diskseq_line_cnt = 0;
	m_diskseq_ed_cnt = 0;
	m_diskseq_head_cnt = 0;
	m_diskseq_cyl_from_cpu = 0;
	m_diskseq_cmd_word_from_cpu = 0;
	m_diskseq_cmd = 0;
	m_diskseq_cyl_to_ctrl = 0;
	m_diskseq_cmd_to_ctrl = 0;
	m_diskseq_status_in = 0;
	m_diskseq_status_out = 0xf9;
	memset(m_diskseq_ucode_latch, 0, 7);
	memset(m_diskseq_cc_inputs, 0, 4);
	m_diskseq_cyl_read_pending = false;
	m_diskseq_complete_clk->adjust(attotime::never);

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

	// Brush Address Card
	m_line_clock = 0;
	m_line_count = 0;
	m_line_length = 0;
	m_brush_addr_func = 0;
	m_bif = 0;
	m_bixos = 0;
	m_biyos = 0;
	m_bxlen = 0;
	m_bylen = 0;
	m_bxlen_counter = 0;
	m_bylen_counter = 0;
	m_plum = 0;
	m_pchr = 0;

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
	memset(m_rhscr, 0, sizeof(uint16_t) * 2);
	memset(m_rvscr, 0, sizeof(uint16_t) * 2);
	memset(m_rzoom, 0, sizeof(uint16_t) * 2);
	memset(m_fld_sel, 0, sizeof(uint16_t) * 2);
	memset(m_window_enable, 0, sizeof(uint16_t) * 2);
	memset(m_cxpos, 0, sizeof(uint16_t) * 2);
	memset(m_cypos, 0, sizeof(uint16_t) * 2);
	m_ca0 = 0;

	// Combiner Card
	m_cursor_y = 0;
	m_cursor_u = 0;
	m_cursor_v = 0;
	m_invert_mask = 0;
	memset(m_select_matte, 0, 2);
	memset(m_matte_ext, 0, 2);
	memset(m_matte_y, 0, 2);
	memset(m_matte_u, 0, 2);
	memset(m_matte_v, 0, 2);

	// Filter Card
	memset(m_filter_acbc, 0, 16);
	memset(m_filter_abbb, 0, 16);
	m_incoming_lum = 0;
	m_incoming_chr = 0;
	m_buffer_lum = true;

	// Size Card
	m_size_h = 0;
	m_size_v = 0;

	// Keyboard
	m_keybc_latched_bit = 1;
	m_keybc_p1_data = 0;
	m_keybc_tx = 0;

	// TDS Box
	m_tds_dac_value = 0;
	m_tds_adc_value = 0;

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
	m_tablet_pen_in_proximity = false;
	m_tablet_state = 0;
}

void dpb7000_state::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if (id == TIMER_DISKSEQ_COMPLETE)
		req_b_w(1);
	else if (id == TIMER_FIELD_IN)
		req_a_w(1);
	else if (id == TIMER_FIELD_OUT)
		req_a_w(0);
	else if (id == TIMER_TABLET_TX)
		tablet_tx_tick();
	else if (id == TIMER_TABLET_IRQ)
		tablet_irq_tick();
}

MC6845_UPDATE_ROW(dpb7000_state::crtc_update_row)
{
	const pen_t *const pen = m_palette->pens();
	const uint8_t *const char_rom = m_vdu_char_rom->base();

	for (int column = 0; column < x_count; column++)
	{
		uint8_t code = (uint8_t)m_vdu_ram[((ma + column) & 0x7ff)];
		uint16_t addr = code << 4 | (ra & 0x0f);
		uint8_t data = char_rom[addr & 0xfff];

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

// NOTE: This function is not used, but is retained in the event we wish for low-level disk sequencer emulation.
void dpb7000_state::diskseq_y_w(uint16_t data)
{
	uint8_t old_prom_latch[7];
	memcpy(old_prom_latch, m_diskseq_ucode_latch, 7);

	const uint8_t *ucode_prom = m_diskseq_ucode->base();
	for (int i = 0; i < 7; i++)
	{
		m_diskseq_ucode_latch[i] = ucode_prom[data | (i << 8)];
	}

	if (m_diskseq_halt)
	{
		m_diskseq->i_w(0);
	}
	else
	{
		m_diskseq->i_w(m_diskseq_ucode_latch[1] & 0x0f);
	}
	m_diskseq->d_w(m_diskseq_ucode_latch[0]);

	if (!BIT(old_prom_latch[2], 1) && BIT(m_diskseq_ucode_latch[2], 1)) // S17: Line Counter Clock
		m_diskseq_line_cnt++;
	if (BIT(m_diskseq_ucode_latch[2], 2)) // S18: Line Counter Reset
		m_diskseq_line_cnt = 0;

	if (!BIT(old_prom_latch[2], 3) && BIT(m_diskseq_ucode_latch[2], 3)) // S19: ED Counter Clock
		m_diskseq_ed_cnt++;
	if (BIT(m_diskseq_ucode_latch[2], 4)) // S20: ED Counter Reset
		m_diskseq_ed_cnt = 0;

	if (!BIT(old_prom_latch[2], 5) && BIT(m_diskseq_ucode_latch[2], 5)) // S21: Auto-Head Clock
		m_diskseq_head_cnt++;
	if (BIT(m_diskseq_ucode_latch[2], 6)) // S22: Auto-Head Reset
		m_diskseq_head_cnt = 0;

	memset(m_diskseq_cc_inputs, 0, 4);
	m_diskseq_cc_inputs[0] |= m_diskseq_prom->base()[m_diskseq_line_cnt & 0x1f] & 3;
	m_diskseq_cc_inputs[0] |= BIT(m_diskseq_ed_cnt, 2) << 2;
	m_diskseq_cc_inputs[0] |= BIT(m_diskseq_head_cnt, 0) << 3;
	m_diskseq_cc_inputs[0] |= BIT(~m_diskseq_status_in, DSEQ_STATUS_READY_BIT) << 5;
	m_diskseq_cc_inputs[0] |= BIT(m_diskseq_status_in, DSEQ_STATUS_FAULT_BIT) << 6;
	m_diskseq_cc_inputs[0] |= BIT(m_diskseq_status_in, DSEQ_STATUS_ONCYL_BIT) << 7;

	m_diskseq_cc_inputs[1] |= BIT(m_diskseq_status_in, DSEQ_STATUS_SKERR_BIT);
	m_diskseq_cc_inputs[1] |= BIT(m_diskseq_status_in, DSEQ_STATUS_INDEX_BIT) << 1;
	m_diskseq_cc_inputs[1] |= BIT(m_diskseq_status_in, DSEQ_STATUS_SECTOR_BIT) << 2;
	m_diskseq_cc_inputs[1] |= BIT(m_diskseq_status_in, DSEQ_STATUS_AMFND_BIT) << 3;
	m_diskseq_cc_inputs[1] |= BIT(m_diskseq_status_in, DSEQ_STATUS_WTPROT_BIT) << 4;
	m_diskseq_cc_inputs[1] |= BIT(m_diskseq_status_in, DSEQ_STATUS_SELECTED_BIT) << 5;
	m_diskseq_cc_inputs[1] |= BIT(m_diskseq_status_in, DSEQ_STATUS_SEEKEND_BIT) << 6;
	m_diskseq_cc_inputs[1] |= BIT(m_diskseq_status_in, DSEQ_STATUS_SYNC_DET_BIT) << 7;

	m_diskseq_cc_inputs[2] |= BIT(m_diskseq_status_in, DSEQ_STATUS_RAM_ADDR_OVFLO_BIT);
	// C17..C19 tied low
	m_diskseq_cc_inputs[2] |= ~(m_diskseq_cmd_word_from_cpu & 0xf) << 4;

	m_diskseq_cc_inputs[3] = ~(m_diskseq_cmd_word_from_cpu >> 4) & 0xff;

	// S15, S16: Select which bank of 8 lines is treated as /CC input to Am2910
	const uint8_t fx_bank_sel = (BIT(m_diskseq_ucode_latch[2], 0) << 1) | BIT(m_diskseq_ucode_latch[1], 7);

	// S12, S13, S14: Select which bit from the bank is treated as /CC input to Am2910
	const uint8_t fx_bit_sel = (BIT(m_diskseq_ucode_latch[1], 6) << 2) | (BIT(m_diskseq_ucode_latch[1], 5) << 1) | BIT(m_diskseq_ucode_latch[1], 4);

	const int cc = BIT(m_diskseq_cc_inputs[fx_bank_sel], fx_bit_sel) ? 1 : 0;
	if (!m_diskseq_halt)
	{
#if (VERBOSE & (LOG_UCODE | LOG_MORE_UCODE))
		char debug_buf[1024];
		int buf_idx = 0;

		buf_idx += sprintf(debug_buf + buf_idx, "%02x: %02x%02x%02x%02x%02x%02x%02x ", data,
			m_diskseq_ucode_latch[6], m_diskseq_ucode_latch[5], m_diskseq_ucode_latch[4], m_diskseq_ucode_latch[3],
			m_diskseq_ucode_latch[2], m_diskseq_ucode_latch[1], m_diskseq_ucode_latch[0]);

		switch (m_diskseq_ucode_latch[1] & 0x0f)
		{
			case  0: buf_idx += sprintf(debug_buf + buf_idx, "JZ      ; "); break;
			case  1: buf_idx += sprintf(debug_buf + buf_idx, "CJS  %02x ; ", m_diskseq_ucode_latch[0]); break;
			case  2: buf_idx += sprintf(debug_buf + buf_idx, "JMAP    ; "); break;
			case  3: buf_idx += sprintf(debug_buf + buf_idx, "CJP  %02x ; ", m_diskseq_ucode_latch[0]); break;
			case  4: buf_idx += sprintf(debug_buf + buf_idx, "PUSH %02x ; ", m_diskseq_ucode_latch[0]); break;
			case  5: buf_idx += sprintf(debug_buf + buf_idx, "JSRP %02x ; ", m_diskseq_ucode_latch[0]); break;
			case  6: buf_idx += sprintf(debug_buf + buf_idx, "CJV  %02x ; ", m_diskseq_ucode_latch[0]); break;
			case  7: buf_idx += sprintf(debug_buf + buf_idx, "JRP  %02x ; ", m_diskseq_ucode_latch[0]); break;
			case  8: buf_idx += sprintf(debug_buf + buf_idx, "RFCT    ; "); break;
			case  9: buf_idx += sprintf(debug_buf + buf_idx, "RPCT    ; "); break;
			case 10: buf_idx += sprintf(debug_buf + buf_idx, "CRTN    ; "); break;
			case 11: buf_idx += sprintf(debug_buf + buf_idx, "CJPP %02x ; ", m_diskseq_ucode_latch[0]); break;
			case 12: buf_idx += sprintf(debug_buf + buf_idx, "LDCT %02x ; ", m_diskseq_ucode_latch[0]); break;
			case 13: buf_idx += sprintf(debug_buf + buf_idx, "LOOP    ; "); break;
			case 14: buf_idx += sprintf(debug_buf + buf_idx, "CONT    ; "); break;
			case 15: buf_idx += sprintf(debug_buf + buf_idx, "TWB  %02x ; ", m_diskseq_ucode_latch[0]); break;
		}

#if LOG_MORE_UCODE
		buf_idx += sprintf(debug_buf + buf_idx, "CCSel:%2d, CCx:%02x%02x%02x%02x, CC:%d, ", fx_bank_sel * 8 + fx_bit_sel, m_diskseq_cc_inputs[3], m_diskseq_cc_inputs[2], m_diskseq_cc_inputs[1], m_diskseq_cc_inputs[0], cc);
		buf_idx += sprintf(debug_buf + buf_idx, "FCyl:%d, FHD:%d, FCmd:%d, FSel:%d", BIT(m_diskseq_ucode_latch[2], 7), BIT(m_diskseq_ucode_latch[4], 2), BIT(m_diskseq_ucode_latch[4], 4), BIT(m_diskseq_ucode_latch[4], 5));
#endif
		LOGMASKED(LOG_UCODE, "%s\n", debug_buf);
#endif
	}
	m_diskseq->cc_w(cc);

	// S25: End of sequencer program
	if (BIT(m_diskseq_ucode_latch[3], 1))
	{
		m_diskseq_halt = true;
		req_b_w(1);
	}

	// S23: FCYL TAG
	// S26, S28..S34: Command word to push onto bus cable
	// S35: FHD TAG
	// S36: FCMD TAG
	// S37: FSEL TAG
	// S38: N/C
	// S39: N/C from PROM, contains D INDEX status bit
	// S40: CK SEL 0
	// S41: CK SEL 1
	// S42: ADDR W. PERMIT
	// S43: ZERO RAM
	// S44: WRITE RAM
	// S45: WORD READ RAM
	// S46: WRITE SYNC
	// S47: SYNC DET EN
	// S48: CPU Status Byte D0
	// S49: CPU Status Byte D1
	// S50: CPU Status Byte D2
	// C5 (D READY): CPU Status Byte D3
	// C12 (D WTPROT): CPU Status Byte D4
	// ED Bit 0: CPU Status Byte D5
	// ED Bit 1: CPU Status Byte D6
	// C6 (D FAULT): CPU Status Byte D7

	if (BIT(m_diskseq_ucode_latch[3], 3)) // S27: Push command word from S26, S28..S34 onto bus cable
	{
		uint8_t disk_cmd = BIT(m_diskseq_ucode_latch[4], 2);
		disk_cmd |= BIT(m_diskseq_ucode_latch[4], 1) << 1;
		disk_cmd |= BIT(m_diskseq_ucode_latch[4], 0) << 4;
		disk_cmd |= BIT(m_diskseq_ucode_latch[3], 7) << 5;
		disk_cmd |= BIT(m_diskseq_ucode_latch[3], 6) << 6;
		disk_cmd |= BIT(m_diskseq_ucode_latch[3], 5) << 7;
		disk_cmd |= BIT(m_diskseq_ucode_latch[3], 4) << 8;
		m_diskseq_cmd_to_ctrl = disk_cmd;
		m_fdd_ctrl |= 0x10;
	}

	if (BIT(m_diskseq_ucode_latch[3], 0)) // S24: Push cylinder number onto the bus cable
	{
		m_diskseq_cyl_to_ctrl = m_diskseq_cyl_from_cpu;
		m_fdd_ctrl |= 0x40;
	}

	if (BIT(m_diskseq_ucode_latch[6], 4))
	{
		m_diskseq_status_out = m_diskseq_ucode_latch[6] & 7;
		m_diskseq_status_out |= BIT(m_diskseq_status_in, DSEQ_STATUS_READY_BIT) << 3;
		m_diskseq_status_out |= BIT(m_diskseq_status_in, DSEQ_STATUS_WTPROT_BIT) << 4;
		m_diskseq_status_out |= (m_diskseq_ed_cnt & 3) << 5;
		m_diskseq_status_out |= 0x80;//BIT(m_diskseq_status_in, DSEQ_STATUS_FAULT_BIT) << 7;
	}
}

// NOTE: This function is not used, but is retained in the event we wish for low-level disk sequencer emulation.
void dpb7000_state::diskseq_tick()
{
	m_diskseq_cp = (m_diskseq_cp ? 0 : 1);
	m_diskseq->cp_w(m_diskseq_cp);

	if (m_diskseq_cp && m_diskseq_reset)
	{
		m_diskseq_reset = false;
	}
}

uint16_t dpb7000_state::bus_error_r(offs_t offset)
{
	if(!machine().side_effects_disabled())
	{
		m_maincpu->set_buserror_details(0xb00000 + offset*2, true, m_maincpu->get_fc());
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	}
	return 0xff;
}

void dpb7000_state::bus_error_w(offs_t offset, uint16_t data)
{
	if(!machine().side_effects_disabled())
	{
		m_maincpu->set_buserror_details(0xb00000 + offset*2, false, m_maincpu->get_fc());
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	}
}

void dpb7000_state::csr_w(uint8_t data)
{
	LOGMASKED(LOG_CSR, "%s: Card Select write: %02x\n", machine().describe_context(), data & 0x0f);
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
		LOGMASKED(LOG_CTRLBUS, "%s: CPU read from Control Bus, Brush Address Card status: %04x\n", machine().describe_context(), m_brush_addr_func);
		ret = m_brush_addr_func;
		break;
	case 1:
		LOGMASKED(LOG_CTRLBUS, "%s: CPU read from Control Bus, Disk Sequencer Card status: %02x\n", machine().describe_context(), m_diskseq_status_out);
		ret = m_diskseq_status_out;
		//req_b_w(0);
		break;
	case 7:
		ret = m_diskbuf_ram[m_diskbuf_ram_addr];
		LOGMASKED(LOG_CTRLBUS, "%s: CPU read from Control Bus, Disc Data Buffer Card RAM read: %04x = %02x\n", machine().describe_context(), m_diskbuf_ram_addr, ret);
		m_diskbuf_ram_addr++;
		break;
	case 12:
		ret = m_config_sw34->read();
		LOGMASKED(LOG_CTRLBUS, "%s: CPU read from Control Bus, Config Switches 1/2: %04x\n", machine().describe_context(), ret);
		break;
	case 14:
		ret = m_config_sw12->read();
		LOGMASKED(LOG_CTRLBUS, "%s: CPU read from Control Bus, Config Switches 3/4: %04x\n", machine().describe_context(), ret);
		break;
	default:
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
		break;
	case 1:
		LOGMASKED(LOG_STORE_ADDR, "%s: Store Address Card %d, set RVSCR: %03x\n", machine().describe_context(), card + 1, data & 0xfff);
		m_rvscr[card] = data & 0xfff;
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
		m_cxpos[card] = data & 0xfff;
		break;
	case 5:
		LOGMASKED(LOG_STORE_ADDR, "%s: Store Address Card %d, set CYPOS: %03x\n", machine().describe_context(), card + 1, data & 0xfff);
		m_cypos[card] = data & 0xfff;
		break;
	default:
		LOGMASKED(LOG_STORE_ADDR, "%s: Store Address Card %d, unknown register: %04x\n", machine().describe_context(), card + 1, data);
		break;
	}
}

void dpb7000_state::combiner_reg_w(uint16_t data)
{
	static const char* const s_const_names[16] = { "Y0", "CY", "CU", "CV", "ES", "EI", "EII", "Y7", "IS", "IY", "IU", "IV", "IIS", "IIY", "IIU", "IIV" };
	LOGMASKED(LOG_COMBINER, "%s: Combiner Card register write: %s = %02x\n", machine().describe_context(), s_const_names[(data >> 10) & 0xf], (uint8_t)data);
	switch ((data >> 10) & 0xf)
	{
		case 1: // CY
			m_cursor_y = (uint8_t)data;
			break;
		case 2: // CU
			m_cursor_u = (uint8_t)data;
			break;
		case 3: // CV
			m_cursor_v = (uint8_t)data;
			break;
		case 4: // ES
			m_invert_mask = (uint8_t)data;
			break;
		case 5: // EI
			m_matte_ext[0] = (uint8_t)data;
			break;
		case 6: // EII
			m_matte_ext[1] = (uint8_t)data;
			break;
		case 8: // IS
			m_select_matte[0] = BIT(data, 0);
			break;
		case 9: // IY
			m_matte_y[0] = (uint8_t)data;
			break;
		case 10: // IU
			m_matte_u[0] = (uint8_t)data;
			break;
		case 11: // IV
			m_matte_v[0] = (uint8_t)data;
			break;
		case 12: // IIS
			m_select_matte[1] = BIT(data, 0);
			break;
		case 13: // IIY
			m_matte_y[1] = (uint8_t)data;
			break;
		case 14: // IIU
			m_matte_u[1] = (uint8_t)data;
			break;
		case 15: // IIV
			m_matte_v[1] = (uint8_t)data;
			break;
	}
}

void dpb7000_state::handle_command(uint16_t data)
{
	//printf("handle_command %d, cxpos %d, cypos %d\n", (data >> 1) & 0xf, m_cxpos[1], m_cypos[1]);
	switch ((data >> 1) & 0xf)
	{
	case 0: // Live Video
		break;
	case 1: // Brush Store Read
		break;
	case 2: // Brush Store Write
		m_bxlen_counter = m_bxlen;
		m_bylen_counter = m_bylen;
		break;
	case 3: // Framestore Read
		break;
	case 4: // Framestore Write
		break;
	case 5: // Fast Wipe Video
		break;
	case 6: // Fast Wipe Brush Store
		break;
	case 7: // Fast Wipe Framestore
		for (int i = 0; i < 2; i++)
		{
			if (!BIT(data, 5 + i) && m_cxpos[i] < 800 && m_cypos[i] < 768)
			{
				m_bylen_counter = m_bylen;
				for (uint16_t y = m_cypos[i]; m_bylen_counter != 0x1000 && y < 768; m_bylen_counter++, y++)
				{
					uint8_t *lum = &m_framestore_lum[i][y * 800];
					uint8_t *chr = &m_framestore_chr[i][y * 800];
					m_bxlen_counter = m_bxlen;
					for (uint16_t x = m_cxpos[i]; m_bxlen_counter != 0x1000 && x < 800; m_bxlen_counter++, x++)
					{
						lum[x] = m_bs_y_latch;
						chr[x] = (x & 1) ? m_bs_v_latch : m_bs_u_latch;
					}
				}
			}
		}
		break;
	case 8: // Draw
		for (int i = 0; i < 2; i++)
		{
			if (!BIT(data, 5 + i) && m_cxpos[i] < 800 && m_cypos[i] < 768)
			{
				uint16_t bxlen = m_bxlen;//(((m_bxlen << 3) | (m_bixos & 7)) >> (m_bif & 3)) & 0xfff;
				uint16_t bylen = m_bylen;//(((m_bylen << 3) | (m_biyos & 7)) >> ((m_bif >> 2) & 3)) & 0xfff;
				for (uint16_t y = m_cypos[i], bly = bylen; bly != 0x1000 && y < 768; bly++, y++)
				{
					uint8_t *lum = &m_framestore_lum[i][y * 800];
					uint8_t *chr = &m_framestore_chr[i][y * 800];
					for (uint16_t x = m_cxpos[i], blx = bxlen; blx != 0x1000 && x < 800; blx++, x++)
					{
						if (BIT(data, 13)) // Fixed Colour Select
						{
							uint8_t y = m_bs_y_latch;
							uint8_t u = m_bs_u_latch;
							uint8_t v = m_bs_v_latch;
							if (!BIT(data, 12)) // Brush Zero
							{
								y = 0x00;
								u = 0x80;
								v = 0x80;
							}
							else if (!BIT(data, 9))
							{
								uint16_t bx = ((((blx - bxlen) << 3) | (m_bixos & 7)) >> (3 - (m_bif & 3))) & 0xfff;
								uint16_t by = ((((bly - bylen) << 3) | (m_biyos & 7)) >> ((3 - (m_bif >> 2)) & 3)) & 0xfff;

								// TODO: Actual Brush Processor functionality
								uint16_t lum_sum = lum[x] + m_brushstore_lum[by * 256 + bx];
								uint16_t chr_sum = chr[x] + m_brushstore_chr[by * 256 + bx];
								y = lum_sum > 0xff ? 0xff : (uint16_t)lum_sum;
								uint8_t chr_clamped = chr_sum > 0xff ? 0xff : (uint16_t)chr_sum;
								if (m_cxpos[i] & 1)
									v = chr_clamped;
								else
									u = chr_clamped;
							}
							lum[x] = y;
							chr[x] = (m_cxpos[i] & 1) ? v : u;
						}
					}
				}
			}
		}
		break;
	case 9: // Draw with Stencil I
		break;
	case 10: // Draw with Stencil II
		break;
	case 11: // Copy to Framestore
		break;
	case 12: // Copy to Brush Store
		break;
	case 13: // Paste with Stencil I
		break;
	case 14: // Paste with Stencil II
		break;
	case 15: // Copy to same Framestore (Invert)
		break;
	}
}

void dpb7000_state::cpu_ctrlbus_w(uint16_t data)
{
	switch (m_csr)
	{
	case 0: // Brush Address Card, function select
	{
		static const char* const s_func_names[16] =
		{
			"Live Video",           "Brush Store Read",     "Brush Store Write",        "Framestore Read",
			"Framestore Write",     "Fast Wipe Video",      "Fast Wipe Brush Store",    "Fast Wipe Framestore",
			"Draw",                 "Draw with Stencil I",  "Draw with Stencil II",     "Copy to Framestore",
			"Copy to Brush Store",  "Paste with Stencil I", "Paste with Stencil II",    "Copy to same Framestore (Invert)"
		};
		LOGMASKED(LOG_BRUSH_ADDR, "%s: Brush Address Card, Function Select: %04x\n", machine().describe_context(), data);
		LOGMASKED(LOG_BRUSH_ADDR, "                Function:           %s\n", s_func_names[(data >> 1) & 0xf]);
		LOGMASKED(LOG_BRUSH_ADDR, "                Store I:            %d\n", BIT(data, 5));
		LOGMASKED(LOG_BRUSH_ADDR, "                Store II:           %d\n", BIT(data, 6));
		LOGMASKED(LOG_BRUSH_ADDR, "                Luma Enable:        %d\n", BIT(data, 7));
		LOGMASKED(LOG_BRUSH_ADDR, "                Chroma Enable:      %d\n", BIT(data, 8));
		LOGMASKED(LOG_BRUSH_ADDR, "                Brush Select:       %d\n", BIT(data, 9));
		LOGMASKED(LOG_BRUSH_ADDR, "                Disc Enable:        %d\n", BIT(data, 10));
		LOGMASKED(LOG_BRUSH_ADDR, "                Brush Invert:       %d\n", BIT(data, 11));
		LOGMASKED(LOG_BRUSH_ADDR, "                Brush Zero:         %d\n", BIT(data, 12));
		LOGMASKED(LOG_BRUSH_ADDR, "                Fixed Color Select: %d\n", BIT(data, 13));
		LOGMASKED(LOG_BRUSH_ADDR, "                Go:                 %d\n", BIT(data, 0));
		m_brush_addr_func = data & ~1;
		if (BIT(data, 0))
		{
			handle_command(data);
		}
		break;
	}

	case 1: // Disk Sequencer Card, disc access
	{
		const uint8_t hi_nybble = data >> 12;
		if (hi_nybble == 0)
		{
			uint16_t old_cyl = m_diskseq_cyl_from_cpu;
			m_diskseq_cyl_from_cpu = data & 0x3ff;
			LOGMASKED(LOG_CTRLBUS, "%s: CPU write to Control Bus, Disk Sequencer Card, Cylinder Number: %04x\n", machine().describe_context(), m_diskseq_cyl_from_cpu);
			//printf("Cylinder %d\n", m_diskseq_cyl_from_cpu);
			if (old_cyl != m_diskseq_cyl_from_cpu && m_diskseq_cyl_from_cpu < 78 && m_floppy != nullptr)
			{
				if (m_diskseq_cyl_from_cpu < old_cyl)
				{
					m_floppy->dir_w(1);
					for (uint16_t i = m_diskseq_cyl_from_cpu; i < old_cyl; i++)
					{
						m_floppy->stp_w(1);
						m_floppy->stp_w(0);
						m_floppy->stp_w(1);
					}
				}
				else
				{
					m_floppy->dir_w(0);
					for (uint16_t i = old_cyl; i < m_diskseq_cyl_from_cpu; i++)
					{
						m_floppy->stp_w(1);
						m_floppy->stp_w(0);
						m_floppy->stp_w(1);
					}
				}
				LOGMASKED(LOG_CTRLBUS, "%s: New floppy cylinder: %04x\n", machine().describe_context(), m_floppy->get_cyl());
			}
		}
		else if (hi_nybble == 1)
		{
			LOGMASKED(LOG_CTRLBUS, "%s: CPU write to Control Bus, Disc Data Buffer Card, Preset RAM Address: %04x\n", machine().describe_context(), data & 0xfff);
			m_diskbuf_ram_addr = data & 0xfff;
		}
		else if (hi_nybble == 2)
		{
			m_diskseq_cmd_word_from_cpu = data & 0xfff;
			m_diskseq_cmd = (data >> 8) & 0xf;
			req_b_w(0); // Flag ourselves as in-use
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
				req_b_w(1);
				//m_diskseq_complete_clk->adjust(attotime::from_msec(1));
				break;
			case 0:
				LOGMASKED(LOG_CTRLBUS, "%s: Disk Sequencer Card Command: Read track to buffer RAM\n", machine().describe_context());
				if (!BIT(m_diskseq_status_out, 3))
				{
					m_diskseq_cyl_read_pending = true;
					m_fdd_side = 0;
				}
				break;
			case 2:
				LOGMASKED(LOG_CTRLBUS, "%s: Disk Sequencer Card Command: Read track, stride 2, to buffer RAM\n", machine().describe_context());
				if (!BIT(m_diskseq_status_out, 3))
				{
					m_diskseq_cyl_read_pending = true;
					m_fdd_side = 0;
				}
				break;
			case 4:
				LOGMASKED(LOG_CTRLBUS, "%s: Disk Sequencer Card Command: Read Track\n", machine().describe_context());
				if (!BIT(m_diskseq_status_out, 3))
				{
					m_diskbuf_data_count = 0x2700;
					m_line_count = 0;
					m_line_clock = 0;
					m_diskseq_cyl_read_pending = true;
					m_fdd_side = 0;
				}
				break;
			case 6:
				LOGMASKED(LOG_CTRLBUS, "%s: Disk Sequencer Card Command: Disc Clear, Read Track\n", machine().describe_context());
				if (!BIT(m_diskseq_status_out, 3))
				{
					m_diskbuf_data_count = 0x2700;
					m_diskseq_cyl_read_pending = true;
					m_fdd_side = 0;
				}
				break;
			case 8:
				LOGMASKED(LOG_CTRLBUS, "%s: Disk Sequencer Card Command: Write Track from Buffer RAM (not yet implemented)\n", machine().describe_context());
				req_b_w(1);
				//m_diskseq_complete_clk->adjust(attotime::from_msec(1));
				break;
			case 10:
				LOGMASKED(LOG_CTRLBUS, "%s: Disk Sequencer Card Command: Write Track, stride 2, from Buffer RAM (not yet implemented)\n", machine().describe_context());
				req_b_w(1);
				//m_diskseq_complete_clk->adjust(attotime::from_msec(1));
				break;
			case 12:
				LOGMASKED(LOG_CTRLBUS, "%s: Disk Sequencer Card Command: Write Track (not yet implemented)\n", machine().describe_context());
				req_b_w(1);
				//m_diskseq_complete_clk->adjust(attotime::from_msec(1));
				break;
			case 14:
				LOGMASKED(LOG_CTRLBUS, "%s: Disk Sequencer Card Command: Disc Clear, Write Track (not yet implemented)\n", machine().describe_context());
				req_b_w(1);
				//m_diskseq_complete_clk->adjust(attotime::from_msec(1));
				break;
			default:
				LOGMASKED(LOG_CTRLBUS, "%s: Unknown Disk Sequencer Card command.\n", machine().describe_context());
				m_diskseq_complete_clk->adjust(attotime::from_msec(1));
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
		store_address_w(1, data);
		if (BIT(data, 15))
			store_address_w(0, data);
		break;

	case 3: // Size Card, Y6/Y7
		if (BIT(data, 15))
		{
			LOGMASKED(LOG_CTRLBUS | LOG_SIZE_CARD, "%s:    Size Card Y7 (Flags): %04x\n", machine().describe_context(), data & 0x7fff);
		}
		else
		{
			LOGMASKED(LOG_CTRLBUS | LOG_SIZE_CARD, "%s:    Size Card Y6 (Coefficients): %04x\n", machine().describe_context(), data & 0x7fff);
		}
		break;

	case 5: // Filter Card
		if (BIT(data, 15))
		{
			LOGMASKED(LOG_CTRLBUS | LOG_FILTER_CARD, "%s: Coefficient(?) RAM B: Index %x = %02x\n", machine().describe_context(), (data >> 8) & 0xf, data & 0xff);
			m_filter_abbb[(data >> 8) & 0xf] = data & 0xff;
		}
		else
		{
			LOGMASKED(LOG_CTRLBUS | LOG_FILTER_CARD, "%s: Coefficient(?) RAM C: Index %x = %02x\n", machine().describe_context(), (data >> 8) & 0xf, data & 0xff);
			m_filter_acbc[(data >> 8) & 0xf] = data & 0xff;
		}
		break;

	case 8: // Brush Store Card color latches
		LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR, "%s: Brush Store Card color latches (%s): %04x\n", machine().describe_context(), m_ca0 ? "VY" : "UY", data);
		if (m_ca0)
		{
			m_bs_v_latch = (uint8_t)(data >> 8);
			m_bs_y_latch = (uint8_t)data;
		}
		else
		{
			m_bs_u_latch = (uint8_t)(data >> 8);
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
			LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR, "%s: Brush Address Card, Register Write: PLUM = %03x(?)\n", machine().describe_context(), data & 0xff);
			m_plum = data & 0xff;
			break;
		case 7:
			LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR, "%s: Brush Address Card, Register Write: PCHR = %03x(?)\n", machine().describe_context(), data & 0xff);
			m_pchr = data & 0xff;
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
			static const char* const s_reg_names[4] = { "Cursor X Origin", "Cursor Y Origin", "Cursor X Size", "Cursor Y Size" };
			const uint8_t reg = (data >> 12) & 3;
			LOGMASKED(LOG_CTRLBUS | LOG_OUTPUT_TIMING, "%s: CPU write to Output Timing Card: %s = %03x\n", machine().describe_context(), s_reg_names[reg], data & 0xfff);
			switch (reg)
			{
			case 0: // Cursor X Origin
				m_cursor_origin_x = data & 0xfff;
				break;
			case 1: // Cursor Y Origin
				m_cursor_origin_y = data & 0xfff;
				break;
			case 2: // Cursor X Size
				m_cursor_size_x = data & 0xfff;
				break;
			case 3: // Cursor Y Size
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
		m_diskseq_reset = true;
		m_diskseq_halt = true;
		break;
	default:
		LOGMASKED(LOG_CTRLBUS | LOG_UNKNOWN, "%s: CPU write to Control Bus, unknown CSR %d: %04x\n", machine().describe_context(), m_csr, data);
		break;
	}
}

WRITE_LINE_MEMBER(dpb7000_state::req_a_w)
{
	if (state)
		m_sys_ctrl |= SYSCTRL_REQ_A_IN;
	else
		m_sys_ctrl &= ~SYSCTRL_REQ_A_IN;

	update_req_irqs();
}

WRITE_LINE_MEMBER(dpb7000_state::req_b_w)
{
	if (state)
		m_sys_ctrl |= SYSCTRL_REQ_B_IN;
	else
		m_sys_ctrl &= ~SYSCTRL_REQ_B_IN;

	update_req_irqs();
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
	LOGMASKED(LOG_SYS_CTRL, "%s: CPU to Control Bus write: %04x\n", machine().describe_context(), data);
	m_sys_ctrl &= ~mask;
	m_sys_ctrl |= (data & mask);

	update_req_irqs();
}

void dpb7000_state::update_req_irqs()
{
	m_maincpu->set_input_line(5, (m_sys_ctrl & SYSCTRL_REQ_A_IN) && (m_sys_ctrl & SYSCTRL_REQ_A_EN) ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(4, (m_sys_ctrl & SYSCTRL_REQ_B_IN) && (m_sys_ctrl & SYSCTRL_REQ_B_EN) ? ASSERT_LINE : CLEAR_LINE);
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
	const uint16_t x = (m_bxlen_counter - m_bxlen);
	const uint16_t y = (m_bylen_counter - m_bylen);
	//printf("Processing sample %d,%d (%04x:%04x, %04x:%04x) LC:%d\n", x, y, m_bxlen_counter, m_bxlen, m_bylen_counter, m_bylen, m_line_count);
	if (BIT(m_brush_addr_func, 7))
		m_brushstore_lum[y * 256 + x] = m_incoming_lum;
	if (BIT(m_brush_addr_func, 8))
		m_brushstore_chr[y * 256 + x] = m_incoming_chr;

	m_bxlen_counter++;
	if (m_bxlen_counter == 0x1000)
	{
		m_bxlen_counter = m_bxlen;
		m_bylen_counter++;
		if (m_bylen_counter == 0x1000)
		{
			m_diskseq_cyl_read_pending = false;
		}
	}
}

void dpb7000_state::process_byte_from_disc(uint8_t data_byte)
{
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

void dpb7000_state::fdd_index_callback(floppy_image_device *floppy, int state)
{
	if (!state && m_diskseq_cyl_read_pending && m_floppy && m_fdd_side < 2)
	{
		//printf("Cylinder read is pending, index just passed by, we have a floppy. Let's go.\n");
		m_fdd_pll.read_reset(machine().time());

		static const uint16_t PREGAP_MARK = 0xaaaa;
		static const uint16_t SYNC_MARK = 0x9125;

		m_floppy->ss_w(m_fdd_side);

		bool seen_pregap = false;
		bool in_track = false;
		int curr_bit = -1;
		uint16_t curr_window = 0;
		uint16_t bit_idx = 0;

		//printf("Side %d, not seen pregap, not in the track, no current bit, no current window, no bit index.\n", m_fdd_side);
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
					//printf("\nFound pregap area.\n");
					bit_idx = 0;
					curr_window = 0;
				}
				else if (seen_pregap && !in_track && curr_window == SYNC_MARK)
				{
					in_track = true;
					//printf("\nOh hi, mark.\n");
					bit_idx = 0;
					curr_window = 0;
				}
				else if (seen_pregap && in_track && bit_idx == 16)
				{
					uint8_t data_byte = (uint8_t)bitswap<16>((uint16_t)curr_window, 15, 13, 11, 9, 7, 5, 3, 1, 14, 12, 10, 8, 6, 4, 2, 0);
					//printf("%02x ", data_byte);
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
							//printf("\nThe whole world has betrayed me!\n");
						}
						else if (m_fdd_side == 0 && m_diskbuf_data_count == 0)
						{
							curr_bit = -1;
							m_floppy->ss_w(1);
							m_fdd_side++;
							m_diskbuf_data_count = 0x2400;
							//printf("\nCatch you on the flip side!\n");
						}
						else if (m_fdd_side == 1 && m_diskbuf_data_count == 0)
						{
							curr_bit = -1;
							m_fdd_side++;
							//printf("\nYou're my favorite customer.\n");
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
							//printf("\nCatch you on the flip side!\n");
						}
						else if(m_diskbuf_ram_addr >= 0x4B00 && m_fdd_side == 1)
						{
							// If we've read the side 1 portion of the cylinder, yield out, we're done
							curr_bit = -1;
							m_fdd_side++;
							//printf("\nYou're my favorite customer.\n");
						}
						break;
					}
					bit_idx = 0;
					curr_window = 0;
				}
			}
		} while (curr_bit != -1);
		//printf("\n");
	}

	if (m_fdd_side == 2)
	{
		m_diskseq_cyl_read_pending = false;
		req_b_w(1);
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
		m_diskseq_status_out &= ~(1 << 3);
	else
		m_diskseq_status_out |= (1 << 3);
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

WRITE_LINE_MEMBER(dpb7000_state::fddcpu_debug_rx)
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

READ_LINE_MEMBER(dpb7000_state::keyboard_t0_r)
{
	LOGMASKED(LOG_KEYBC, "%s: T0 read\n", machine().describe_context());
	return 0;
}

READ_LINE_MEMBER(dpb7000_state::keyboard_t1_r)
{
	uint8_t data = m_keybc_latched_bit;
	//m_keybc_latched_bit = 0;
	LOGMASKED(LOG_KEYBC, "%s: T1 read: %d\n", machine().describe_context(), data);
	return data;
}

void dpb7000_state::tds_dac_w(uint8_t data)
{
	LOGMASKED(LOG_TDS, "%s: TDS DAC Write: %02x\n", machine().describe_context(), data);
	m_tds_dac_value = data;
}

void dpb7000_state::tds_convert_w(uint8_t data)
{
	LOGMASKED(LOG_TDS, "%s: TDS ADC Convert Start\n", machine().describe_context());
	m_tds_adc_value = m_tds_dac_value;
}

uint8_t dpb7000_state::tds_adc_r()
{
	LOGMASKED(LOG_TDS, "%s: TDS ADC Read: %02x\n", machine().describe_context(), m_tds_adc_value);
	return m_tds_adc_value;
}

uint8_t dpb7000_state::tds_pen_switches_r()
{
	uint8_t data = 0;//m_pen_switches->read() << 4;
	LOGMASKED(LOG_TDS, "%s: TDS Pen Switches Read: %02x\n", machine().describe_context(), data);
	return data;
}

void dpb7000_state::tablet_tx_tick()
{
	m_tds_duart->rx_b_w(m_tablet_tx_bit);
}

WRITE_LINE_MEMBER(dpb7000_state::duart_b_w)
{
	//printf("B%d ", state);
}

uint8_t dpb7000_state::tds_p1_r()
{
	const uint8_t latched = m_tds_p1_data & 0x82;
	const uint8_t adc_not_busy = (1 << 5);
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

INPUT_CHANGED_MEMBER(dpb7000_state::pen_prox_changed)
{
	m_tablet_pen_in_proximity = newval ? false : true;
	if (m_tablet_pen_in_proximity)
	{
		LOGMASKED(LOG_TABLET, "Triggering IRQ due to proximity\n");
		m_tablet_cpu->set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
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
		if (old_drq && !m_tablet_drq && m_tablet_pen_in_proximity) // DRQ transition to low
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
				m_tablet_irq_timer->adjust(attotime::from_ticks(temp_pen_reads[3 - m_tablet_mux], 1200000), 1);
				LOGMASKED(LOG_TABLET, "Setting up IRQ timer for read (continuous)\n");
			}
		}
	}
}

void dpb7000_state::tablet_irq_tick()
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
	//m_tablet_counter_latch = temp_pen_reads[3 - m_tablet_mux];
	uint8_t data = /*((~m_pen_switches->read() & 0x0f) << 4) |*/ (m_tablet_counter_latch >> 8);
	LOGMASKED(LOG_TABLET, "%s: Tablet RDH Read (Mux %d): %02x\n", machine().describe_context(), m_tablet_mux, data);
	return data;
}

uint8_t dpb7000_state::tablet_rdl_r()
{
	//if (m_tablet_mux == 3)
		//m_tablet_state = 0;

	m_tablet_counter_latch = rand() & 0xfff;//temp_pen_reads[m_tablet_mux];
	LOGMASKED(LOG_TABLET, "%s: Random latch: %04x\n", machine().describe_context(), m_tablet_counter_latch);
	uint8_t data = (uint8_t)m_tablet_counter_latch;
	LOGMASKED(LOG_TABLET, "%s: Tablet RDL Read (Mux %d): %02x\n", machine().describe_context(), m_tablet_mux, data);
	return data;
}

template <int StoreNum>
uint32_t dpb7000_state::store_screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int py = 0; py < 768; py++)
	{
		const uint8_t *src_lum = &m_framestore_lum[StoreNum][py * 800];
		const uint8_t *src_chr = &m_framestore_chr[StoreNum][py * 800];
		uint32_t *dst = &bitmap.pix(py);
		for (int px = 0; px < 800; px++)
		{
			const uint32_t u = *src_chr++ << 16;
			const uint32_t v = *src_chr++ << 8;
			*dst++ = m_yuv_lut[u | v | *src_lum++];
			*dst++ = m_yuv_lut[u | v | *src_lum++];
		}
	}

	if (StoreNum == 0)
	{
		for (int py = 512; py < 768; py++)
		{
			const uint8_t *src_lum = &m_brushstore_lum[(py - 512) * 256];
			const uint8_t *src_chr = &m_brushstore_chr[(py - 512) * 256];
			uint32_t *dst = &bitmap.pix(py);
			for (int px = 0; px < 256; px += 2)
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

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(696, 276);
	screen.set_visarea(56, 695, 36, 275);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	screen_device &store_screen1(SCREEN(config, "store_screen1", SCREEN_TYPE_RASTER));
	store_screen1.set_refresh_hz(60);
	store_screen1.set_size(800, 768);
	store_screen1.set_visarea(0, 799, 0, 767);
	store_screen1.set_screen_update(FUNC(dpb7000_state::store_screen_update<0>));

	screen_device &store_screen2(SCREEN(config, "store_screen2", SCREEN_TYPE_RASTER));
	store_screen2.set_refresh_hz(60);
	store_screen2.set_size(800, 768);
	store_screen2.set_visarea(0, 799, 0, 767);
	store_screen2.set_screen_update(FUNC(dpb7000_state::store_screen_update<1>));

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	// The 6545's clock is driven by the QD output of a 4-bit binary counter, which has its preset wired to 0010.
	// It therefore operates as a divide-by-six counter for the master clock of 22.248MHz.
	SY6545_1(config, m_crtc, 22.248_MHz_XTAL / 6);
	m_crtc->set_char_width(8);
	m_crtc->set_show_border_area(false);
	m_crtc->set_screen("screen");
	m_crtc->set_update_row_callback(FUNC(dpb7000_state::crtc_update_row));
	m_crtc->set_on_update_addr_change_callback(FUNC(dpb7000_state::crtc_addr_changed));

	// Disc Sequencer Card
	AM2910(config, m_diskseq, 0); // We drive the clock manually from the driver
	m_diskseq->ci_w(1);
	m_diskseq->rld_w(1);
	m_diskseq->ccen_w(0);
	m_diskseq->y().set(FUNC(dpb7000_state::diskseq_y_w));

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

	// Size Card
	AM2901B(config, m_size_yl);
	AM2901B(config, m_size_yh);
	AM2901B(config, m_size_xl);
	AM2901B(config, m_size_xh);

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

	SCN2681(config, m_tds_duart, 3.6864_MHz_XTAL);
	m_tds_duart->irq_cb().set_inputline(m_tds_cpu, M6803_IRQ_LINE);
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
	ROM_REGION16_BE(0x80000, "monitor", 0)
	ROM_LOAD16_BYTE("01616a-nad-4af2.bin", 0x00001, 0x8000, CRC(a42eace6) SHA1(78c629a8afb48a95fc0a86ca762cc5b84bd9929b))
	ROM_LOAD16_BYTE("01616a-ncd-58de.bin", 0x00000, 0x8000, CRC(f70fff2a) SHA1(a6f85d086a0c53d156eeeb157184ebcad4adecb3))
	ROM_LOAD16_BYTE("01616a-mad-f512.bin", 0x10001, 0x8000, CRC(4c3e39f6) SHA1(443095c56481fbcadd4dcec1757d889c8f78805d))
	ROM_LOAD16_BYTE("01616a-mcd-91f2.bin", 0x10000, 0x8000, CRC(4b6b6eb3) SHA1(1bef443d78197d33e44c708ead9604020881f67f))
	ROM_LOAD16_BYTE("01616a-lad-0059.bin", 0x20001, 0x8000, CRC(0daf670d) SHA1(2342a43054ed141de298a1c1a6867949297bb52a))
	ROM_LOAD16_BYTE("01616a-lcd-5639.bin", 0x20000, 0x8000, CRC(c8977d3f) SHA1(4ee9f3a883400b4771e6ae33c6e4edcd5c0b49e7))
	ROM_LOAD16_BYTE("01616a-kad-1d9b.bin", 0x30001, 0x8000, CRC(bda7e309) SHA1(377edf2675a6736fe7ec775894858967b0e9247e))
	ROM_LOAD16_BYTE("01616a-kcd-e51c.bin", 0x30000, 0x8000, CRC(aa05a5cc) SHA1(85dce335a72643f7640524b18cfe480a3c299f23))
	ROM_LOAD16_BYTE("01616a-jad-47a8.bin", 0x40001, 0x8000, CRC(60fff4c9) SHA1(ba60281c0dd8627dffe07e7ea66f4eb688e74001))
	ROM_LOAD16_BYTE("01616a-jcd-7825.bin", 0x40000, 0x8000, CRC(bb258ede) SHA1(ab8042391cd361bcd874b2f9d8fcaf20d4b2ebe7))
	ROM_LOAD16_BYTE("01616a-iad-73a8.bin", 0x50001, 0x8000, CRC(98709fd2) SHA1(bd0f4689600e9fc49dbd8f2f326e18f8d602825e))
	ROM_LOAD16_BYTE("01616a-icd-0562.bin", 0x50000, 0x8000, CRC(bab5274e) SHA1(3e51977da3dfe8fda089b9d2c3199acb4fed3212))
	ROM_LOAD16_BYTE("01616a-had-d6fa.bin", 0x60001, 0x8000, CRC(70d791c5) SHA1(c281e4f27404e58ad5a80d6de1c5583cd9f3fe0e))
	ROM_LOAD16_BYTE("01616a-hcd-9c0e.bin", 0x60000, 0x8000, CRC(938cb614) SHA1(ea7ea8a13e0ab1497691bab53090296ba51d271f))
	ROM_LOAD16_BYTE("01616a-gcd-3ab8.bin", 0x70001, 0x8000, CRC(e9c21438) SHA1(1784ab2de1bb6023565b2e27872a0fcda25e1b1f))
	ROM_LOAD16_BYTE("01616a-gad-397d.bin", 0x70000, 0x8000, CRC(0b95f9ed) SHA1(77126ee6c1f3dcdb8aa669ab74ff112e3f01918a))

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

	ROM_REGION(0x1200, "output_timing_proms", 0)
	ROM_LOAD("pb-037-17418-bea.bin", 0x0000, 0x400, CRC(644e82a3) SHA1(d7634e03809abe2db924571c05821c1b2aca051b))
	ROM_LOAD("pb-037-17418-bga.bin", 0x0400, 0x400, CRC(3b2c3635) SHA1(2038d616dd7f65ba55497bd037b0ad69aaa801ed))
	ROM_LOAD("pb-037-17418-cda.bin", 0x0800, 0x400, CRC(a31f3793) SHA1(4e74e528088c155e2c2592fa937e4cabfe6324c8))
	ROM_LOAD("pb-037-17418-dfa.bin", 0x0c00, 0x400, CRC(ca2ec308) SHA1(4232f44ea5bd3fa240eaf7c14e4b925140f90a1e))
	ROM_LOAD("pb-037-17418-bfa.bin", 0x1000, 0x200, CRC(db84a171) SHA1(ed63f384928a017dafd694c7bcf99e315af6bd3c))

	ROM_REGION(0x800, "keyboard", 0)
	ROM_LOAD("etc2716 63b2.bin", 0x000, 0x800, CRC(04614a50) SHA1(e547458f2c9cf29cf52f02b8824b32e5e91807fd))

	ROM_REGION(0x2000, "tds", 0)
	ROM_LOAD("hn482764g.bin", 0x0000, 0x2000, CRC(3626059c) SHA1(1a4f5c8b337f31c7b2b93096b59234ffbc2f1f00))

	ROM_REGION(0x2000, "tablet", 0)
	ROM_LOAD("nmc27c64q.bin", 0x0000, 0x2000, CRC(a453928f) SHA1(f4a25298fb446f0046c6f9f3ce70e7169dcebd01))

	ROM_REGION(0x200, "brushproc_prom", 0)
	ROM_LOAD("pb-02c-17593-baa.bin", 0x000, 0x200, CRC(a74cc1f5) SHA1(3b789d5a29c70c93dec56f44be8c14b41915bdef))

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
ROM_END

COMP( 1981, dpb7000, 0, 0, dpb7000, dpb7000, dpb7000_state, empty_init, "Quantel", "DPB-7000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
