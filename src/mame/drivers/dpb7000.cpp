// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    DPB-7001 (c) 1981 Quantel

    Skeleton Driver

***************************************************************************/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68000.h"
#include "cpu/m6800/m6801.h"
#include "machine/6850acia.h"
#include "machine/am25s55x.h"
#include "machine/am2910.h"
#include "machine/com8116.h"
#include "machine/input_merger.h"
#include "machine/tdc1008.h"
#include "video/dpb_brushproc.h"
#include "video/dpb_brushstore.h"
#include "video/dpb_combiner.h"
#include "video/dpb_framestore.h"
#include "video/dpb_storeaddr.h"
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
#define LOG_OUTPUT_TIMING   (1 << 9)
#define LOG_BRUSH_ADDR      (1 << 10)
#define LOG_ALL             (LOG_UNKNOWN | LOG_UCODE | LOG_MORE_UCODE | LOG_CSR | LOG_CTRLBUS | LOG_SYS_CTRL | LOG_FDC_CTRL | LOG_FDC_PORT | LOG_FDC_CMD | \
							 LOG_OUTPUT_TIMING | LOG_BRUSH_ADDR)

#define VERBOSE             (LOG_ALL &~ LOG_FDC_CTRL)
#include "logmacro.h"

class dpb7000_state : public driver_device
{
public:
	dpb7000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_acia(*this, "fd%u", 0U)
		, m_p_int(*this, "p_int")
		, m_brg(*this, "brg")
		, m_rs232(*this, "rs232")
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
		, m_filter_cd(*this, "filter_cd")
		, m_filter_ce(*this, "filter_ce")
		, m_filter_cf(*this, "filter_cf")
		, m_filter_cg(*this, "filter_cg")
		, m_store_addr(*this, "store_addr%u", 0U)
		, m_brush_proc(*this, "brush_proc%u", 0U)
		, m_brush_store(*this, "brush_store")
		, m_combiner(*this, "combiner")
		, m_framestore(*this, "framestore%u", 0U)
	{
	}

	void dpb7000(machine_config &config);

private:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	static constexpr device_timer_id TIMER_DISKSEQ = 0;
	static constexpr device_timer_id TIMER_FIELD_IN = 1;
	static constexpr device_timer_id TIMER_FIELD_OUT = 2;

	void main_map(address_map &map);
	void fddcpu_map(address_map &map);

	DECLARE_READ16_MEMBER(bus_error_r);
	DECLARE_WRITE16_MEMBER(bus_error_w);

	DECLARE_WRITE8_MEMBER(csr_w);
	DECLARE_READ8_MEMBER(csr_r);

	DECLARE_READ16_MEMBER(cpu_ctrlbus_r);
	DECLARE_WRITE16_MEMBER(cpu_ctrlbus_w);

	DECLARE_WRITE_LINE_MEMBER(req_a_w);
	DECLARE_WRITE_LINE_MEMBER(req_b_w);

	DECLARE_READ8_MEMBER(fdd_ctrl_r);
	DECLARE_READ8_MEMBER(fdd_cmd_r);
	DECLARE_WRITE8_MEMBER(fddcpu_p1_w);
	DECLARE_READ8_MEMBER(fddcpu_p2_r);
	DECLARE_WRITE8_MEMBER(fddcpu_p2_w);
	DECLARE_WRITE_LINE_MEMBER(fddcpu_debug_rx);

	enum : uint16_t
	{
		SYSCTRL_AUTO_START      = 0x0001,
		SYSCTRL_REQ_B_EN        = 0x0020,
		SYSCTRL_REQ_A_EN        = 0x0040,
		SYSCTRL_REQ_A_IN        = 0x0080,
		SYSCTRL_REQ_B_IN        = 0x8000
	};

	DECLARE_READ16_MEMBER(cpu_sysctrl_r);
	DECLARE_WRITE16_MEMBER(cpu_sysctrl_w);
	void update_req_irqs();

	MC6845_UPDATE_ROW(crtc_update_row);
	MC6845_ON_UPDATE_ADDR_CHANGED(crtc_addr_changed);

	DECLARE_WRITE16_MEMBER(diskseq_y_w);
	void diskseq_tick();

	required_device<m68000_base_device> m_maincpu;
	required_device_array<acia6850_device, 3> m_acia;
	required_device<input_merger_device> m_p_int;
	required_device<com8116_device> m_brg;
	required_device<rs232_port_device> m_rs232;
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
	size_t m_fdd_debug_rx_bit_count;
	size_t m_fdd_debug_rx_byte_count;
	size_t m_fdd_debug_rx_recv_count;
	uint8_t m_fdd_ctrl;
	uint8_t m_fdd_port1;
	uint8_t m_fdd_track;

	required_device<tdc1008_device> m_filter_cd;
	required_device<tdc1008_device> m_filter_ce;
	required_device<tdc1008_device> m_filter_cf;
	required_device<tdc1008_device> m_filter_cg;

	required_device_array<dpb7000_storeaddr_card_device, 2> m_store_addr;
	required_device_array<dpb7000_brushproc_card_device, 2> m_brush_proc;
	required_device<dpb7000_brush_store_card_device> m_brush_store;
	required_device<dpb7000_combiner_card_device> m_combiner;
	required_device_array<dpb7000_framestore_card_device, 6> m_framestore;

	emu_timer *m_diskseq_clk;
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
	uint16_t m_diskseq_cmd_from_cpu;    // DD/CC
	uint8_t m_diskseq_cyl_to_ctrl;
	uint8_t m_diskseq_cmd_to_ctrl;
	uint8_t m_diskseq_status_in;        // CG
	uint8_t m_diskseq_status_out;       // BC
	uint8_t m_diskseq_ucode_latch[7];   // GG/GF/GE/GD/GC/GB/GA
	uint8_t m_diskseq_cc_inputs[4];     // Inputs to FE/FD/FC/FB

	// Output Timing Card
	uint16_t m_cursor_origin_x;
	uint16_t m_cursor_origin_y;
	uint16_t m_cursor_size_x;
	uint16_t m_cursor_size_y;

	// Brush Address Card
	uint16_t m_brush_addr_func;
	uint8_t m_bif;
	uint8_t m_bixos;
	uint8_t m_biyos;
	uint8_t m_bxlen;
	uint8_t m_bylen;
	uint8_t m_plum;
	uint8_t m_pchr;
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

static INPUT_PORTS_START( dpb7000 )
	PORT_START("BAUD")
	PORT_DIPNAME( 0x0f, 0x0e, "Baud Rate for Terminal")
	PORT_DIPSETTING(    0x00, "50")
	PORT_DIPSETTING(    0x01, "75")
	PORT_DIPSETTING(    0x02, "110")
	PORT_DIPSETTING(    0x03, "134.5")
	PORT_DIPSETTING(    0x04, "150")
	PORT_DIPSETTING(    0x05, "300")
	PORT_DIPSETTING(    0x06, "600")
	PORT_DIPSETTING(    0x07, "1200")
	PORT_DIPSETTING(    0x08, "1800")
	PORT_DIPSETTING(    0x09, "2000")
	PORT_DIPSETTING(    0x0a, "2400")
	PORT_DIPSETTING(    0x0b, "3600")
	PORT_DIPSETTING(    0x0c, "4800")
	PORT_DIPSETTING(    0x0e, "9600")
	PORT_DIPSETTING(    0x0f, "19200")

	PORT_START("AUTOSTART")
	PORT_DIPNAME( 0x0001, 0x0000, "Auto-Start" )
	PORT_DIPSETTING(    0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0001, DEF_STR( On ) )
	PORT_BIT( 0xfffe, IP_ACTIVE_HIGH, IPT_UNUSED )

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
	PORT_DIPNAME( 0x0003, 0x0003, "Disc Group 1 Count" )
	PORT_DIPSETTING(    0x0003, "One" )
	PORT_DIPSETTING(    0x0002, "Two" )
	PORT_DIPSETTING(    0x0001, "Three" )
	PORT_DIPSETTING(    0x0000, "Four" )
	PORT_DIPNAME( 0x001c, 0x001c, "Disc Group 0 Type" )
	PORT_DIPSETTING(    0x001c, "160Mb CDC Fixed" )
	PORT_DIPSETTING(    0x0018, "80Mb CDC Removable" )
	PORT_DIPSETTING(    0x0014, "NTSC/Floppy/PAL/Conv" )
	PORT_DIPSETTING(    0x0010, "Floppy Disc" )
	PORT_DIPSETTING(    0x000c, "80Mb Fujitsu" )
	PORT_DIPSETTING(    0x0008, "330Mb Fujitsu" )
	PORT_DIPSETTING(    0x0004, "160Mb Fujitsu" )
	PORT_DIPSETTING(    0x0000, "Unsupported" )
	PORT_DIPNAME( 0x0060, 0x0060, "Disc Group 1 Count" )
	PORT_DIPSETTING(    0x0060, "One" )
	PORT_DIPSETTING(    0x0040, "Two" )
	PORT_DIPSETTING(    0x0020, "Three" )
	PORT_DIPSETTING(    0x0000, "Four" )
	PORT_DIPNAME( 0x0380, 0x0380, "Disc Group 1 Type" )
	PORT_DIPSETTING(    0x0380, "160Mb CDC Fixed" )
	PORT_DIPSETTING(    0x0300, "80Mb CDC Removable" )
	PORT_DIPSETTING(    0x0280, "NTSC/Floppy/PAL/Conv" )
	PORT_DIPSETTING(    0x0200, "Floppy Disc" )
	PORT_DIPSETTING(    0x0180, "80Mb Fujitsu" )
	PORT_DIPSETTING(    0x0100, "330Mb Fujitsu" )
	PORT_DIPSETTING(    0x0080, "160Mb Fujitsu" )
	PORT_DIPSETTING(    0x0000, "Unsupported" )
	PORT_DIPNAME( 0x0c00, 0x0c00, "Disc Group 2 Count" )
	PORT_DIPSETTING(    0x0c00, "One" )
	PORT_DIPSETTING(    0x0800, "Two" )
	PORT_DIPSETTING(    0x0400, "Three" )
	PORT_DIPSETTING(    0x0000, "Four" )
	PORT_DIPNAME( 0x7000, 0x7000, "Disc Group 2 Type" )
	PORT_DIPSETTING(    0x7000, "160Mb CDC Fixed" )
	PORT_DIPSETTING(    0x6000, "80Mb CDC Removable" )
	PORT_DIPSETTING(    0x5000, "NTSC/Floppy/PAL/Conv" )
	PORT_DIPSETTING(    0x4000, "Floppy Disc" )
	PORT_DIPSETTING(    0x3000, "80Mb Fujitsu" )
	PORT_DIPSETTING(    0x2000, "330Mb Fujitsu" )
	PORT_DIPSETTING(    0x1000, "160Mb Fujitsu" )
	PORT_DIPSETTING(    0x0000, "Unsupported" )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
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
	save_item(NAME(m_diskseq_cmd_from_cpu));
	save_item(NAME(m_diskseq_cyl_to_ctrl));
	save_item(NAME(m_diskseq_cmd_to_ctrl));
	save_item(NAME(m_diskseq_status_in));
	save_item(NAME(m_diskseq_status_out));
	save_item(NAME(m_diskseq_ucode_latch));
	save_item(NAME(m_diskseq_cc_inputs));
	save_item(NAME(m_fdd_debug_rx_bit_count));
	save_item(NAME(m_fdd_debug_rx_byte_count));
	save_item(NAME(m_fdd_debug_rx_recv_count));
	save_item(NAME(m_fdd_ctrl));
	save_item(NAME(m_fdd_port1));
	save_item(NAME(m_fdd_track));

	m_diskseq_clk = timer_alloc(TIMER_DISKSEQ);
	m_diskseq_clk->adjust(attotime::never);

	// Output Timing Card
	save_item(NAME(m_cursor_origin_x));
	save_item(NAME(m_cursor_origin_y));
	save_item(NAME(m_cursor_size_x));
	save_item(NAME(m_cursor_size_y));

	// Brush Address Card
	save_item(NAME(m_brush_addr_func));
	save_item(NAME(m_bif));
	save_item(NAME(m_bixos));
	save_item(NAME(m_biyos));
	save_item(NAME(m_bxlen));
	save_item(NAME(m_bylen));
	save_item(NAME(m_plum));
	save_item(NAME(m_pchr));
}

void dpb7000_state::machine_reset()
{
	// Computer Card
	m_brg->stt_w(m_baud_dip->read());
	m_csr = 0;
	m_sys_ctrl = SYSCTRL_REQ_B_IN;

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
	m_diskseq_cmd_from_cpu = 0;
	m_diskseq_cyl_to_ctrl = 0;
	m_diskseq_cmd_to_ctrl = 0;
	m_diskseq_status_in = 0;
	m_diskseq_status_out = 0;
	memset(m_diskseq_ucode_latch, 0, 7);
	memset(m_diskseq_cc_inputs, 0, 4);

	m_diskseq_clk->adjust(attotime::from_hz(1000000), 0, attotime::from_hz(1000000));

	// Floppy Disc Controller
	m_fdd_debug_rx_bits.clear();
	m_fdd_debug_rx_bit_count = 0;
	m_fdd_debug_rx_byte_count = 0;
	m_fdd_debug_rx_recv_count = 0;
	m_fdd_ctrl = 0;
	m_fdd_port1 = 0;
	m_fdd_track = 20;

	// Output Timing Card
	m_cursor_origin_x = 0;
	m_cursor_origin_y = 0;
	m_cursor_size_x = 0;
	m_cursor_size_y = 0;

	// Store Address Card
	m_store_addr[0]->s_type_w(0);
	m_store_addr[1]->s_type_w(1);

	// Brush Address Card
	m_brush_addr_func = 0;
	m_bif = 0;
	m_bixos = 0;
	m_biyos = 0;
	m_bxlen = 0;
	m_bylen = 0;
	m_plum = 0;
	m_pchr = 0;
}

void dpb7000_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_DISKSEQ)
		diskseq_tick();
	else if (id == TIMER_FIELD_IN)
		req_a_w(1);
	else if (id == TIMER_FIELD_OUT)
		req_a_w(0);
}

MC6845_UPDATE_ROW(dpb7000_state::crtc_update_row)
{
	const pen_t *pen = m_palette->pens();
	const uint8_t *char_rom = m_vdu_char_rom->base();

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

			bitmap.pix32(y, x) = pen[color];

			data <<= 1;
		}
	}
}

MC6845_ON_UPDATE_ADDR_CHANGED(dpb7000_state::crtc_addr_changed)
{
}

WRITE16_MEMBER(dpb7000_state::diskseq_y_w)
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
	m_diskseq_cc_inputs[2] |= ~(m_diskseq_cmd_from_cpu & 0xf) << 4;

	m_diskseq_cc_inputs[3] = ~(m_diskseq_cmd_from_cpu >> 4) & 0xff;

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

void dpb7000_state::diskseq_tick()
{
	m_diskseq_cp = (m_diskseq_cp ? 0 : 1);
	m_diskseq->cp_w(m_diskseq_cp);

	if (m_diskseq_cp && m_diskseq_reset)
	{
		m_diskseq_reset = false;
	}
}

READ16_MEMBER(dpb7000_state::bus_error_r)
{
	if(!machine().side_effects_disabled())
	{
		m_maincpu->set_buserror_details(0xb00000 + offset*2, true, m_maincpu->get_fc());
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	}
	return 0xff;
}

WRITE16_MEMBER(dpb7000_state::bus_error_w)
{
	if(!machine().side_effects_disabled())
	{
		m_maincpu->set_buserror_details(0xb00000 + offset*2, false, m_maincpu->get_fc());
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	}
}

WRITE8_MEMBER(dpb7000_state::csr_w)
{
	LOGMASKED(LOG_CSR, "%s: Card Select write: %02x\n", machine().describe_context(), data & 0x0f);
	m_csr = data & 0x0f;
}

READ8_MEMBER(dpb7000_state::csr_r)
{
	LOGMASKED(LOG_CSR, "%s: Card Select read(?): %02x\n", machine().describe_context(), m_csr);
	return m_csr;
}

READ16_MEMBER(dpb7000_state::cpu_ctrlbus_r)
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

WRITE16_MEMBER(dpb7000_state::cpu_ctrlbus_w)
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
		LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR, "%s: Brush Address Card, Function Select: %04x\n", machine().describe_context(), data);
		LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR, "                Function:           %s\n", s_func_names[(data >> 1) & 0xf]);
		LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR, "                /Store I:           %d\n", BIT(data, 5));
		LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR, "                /Store II:          %d\n", BIT(data, 6));
		LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR, "                Luma Enable:        %d\n", BIT(data, 7));
		LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR, "                Chroma Enable:      %d\n", BIT(data, 8));
		LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR, "                Brush Select:       %d\n", BIT(data, 9));
		LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR, "                Disc Enable:        %d\n", BIT(data, 10));
		LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR, "                /Brush Invert:      %d\n", BIT(data, 11));
		LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR, "                Brush Zero:         %d\n", BIT(data, 12));
		LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR, "                Fixed Color Select: %d\n", BIT(data, 13));
		LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR, "                Go:                 %d\n", BIT(data, 0));
		m_brush_addr_func = data & ~1;
		break;
	}

	case 1: // Disk Sequencer Card, disc access
	{
		const uint8_t hi_nybble = data >> 12;
		if (hi_nybble == 0)
		{
			m_diskseq_cyl_from_cpu = data & 0x3ff;
			LOGMASKED(LOG_CTRLBUS, "%s: CPU write to Control Bus, Disk Sequencer Card, Cylinder Number: %04x\n", machine().describe_context(), m_diskseq_cyl_from_cpu);
		}
		else if (hi_nybble == 2)
		{
			m_diskseq_cmd_from_cpu = data & 0xfff;
			m_diskseq_halt = false;
			req_b_w(0);
			LOGMASKED(LOG_CTRLBUS, "%s: CPU write to Control Bus, Disk Sequencer Card, Command: %04x\n", machine().describe_context(), m_diskseq_cmd_from_cpu);
		}
		else
		{
			LOGMASKED(LOG_CTRLBUS | LOG_UNKNOWN, "%s: CPU write to Control Bus, Disk Sequencer Card, Unrecognized hi nybble: %04x\n", machine().describe_context(), data);
		}
		break;
	}

	case 2: // Store Address Card
		m_store_addr[1]->reg_w(data);
		if (BIT(data, 15))
			m_store_addr[0]->reg_w(data);
		break;

	case 8: // Brush Address Card, "Select 8" signal to PAL 16L8, BE
		LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR, "%s: Brush Address Card, Select 8\n", machine().describe_context());
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
			LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR, "%s: Brush Address Card, Register Write: BXLEN = %02x\n", machine().describe_context(), data & 0x3f);
			m_bxlen = data & 0x3f;
			break;
		case 5:
			LOGMASKED(LOG_CTRLBUS | LOG_BRUSH_ADDR, "%s: Brush Address Card, Register Write: BYLEN = %02x\n", machine().describe_context(), data & 0x3f);
			m_bylen = data & 0x3f;
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
			m_combiner->reg_w(data);
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

READ16_MEMBER(dpb7000_state::cpu_sysctrl_r)
{
	const uint16_t ctrl = m_sys_ctrl &~ SYSCTRL_AUTO_START;
	const uint16_t auto_start = m_auto_start->read() ? SYSCTRL_AUTO_START : 0;
	const uint16_t ret = ctrl | auto_start;
	LOGMASKED(LOG_SYS_CTRL, "%s: CPU read from System Control: %04x\n", machine().describe_context(), ret);
	return ret;
}

WRITE16_MEMBER(dpb7000_state::cpu_sysctrl_w)
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

READ8_MEMBER(dpb7000_state::fdd_ctrl_r)
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

WRITE8_MEMBER(dpb7000_state::fddcpu_p1_w)
{
	LOGMASKED(LOG_FDC_PORT, "%s: Floppy CPU Port 1 Write: %02x\n", machine().describe_context(), data);
	const uint8_t old_value = m_fdd_port1;
	m_fdd_port1 = data;
	if (!BIT(old_value, 0) && BIT(m_fdd_port1, 0))
	{
		if (BIT(m_fdd_port1, 1) && m_fdd_track < 40)
		{
			m_fdd_track++;
		}
		else if (!BIT(m_fdd_port1, 1) && m_fdd_track > 0)
		{
			m_fdd_track--;
		}

		if (m_fdd_track == 0)
			m_fdd_ctrl |= 4;
		else
			m_fdd_ctrl &= ~4;
	}
	if (BIT(m_fdd_port1, 7))
		m_diskseq_status_in |= (1 << DSEQ_STATUS_READY_BIT);
	else
		m_diskseq_status_in &= ~(1 << DSEQ_STATUS_READY_BIT);
}

WRITE8_MEMBER(dpb7000_state::fddcpu_p2_w)
{
	m_fdd_serial->write_txd(BIT(data, 4));
}

READ8_MEMBER(dpb7000_state::fddcpu_p2_r)
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

READ8_MEMBER(dpb7000_state::fdd_cmd_r)
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

void dpb7000_state::dpb7000(machine_config &config)
{
	// Computer Card 1 & 2
	M68000(config, m_maincpu, 16_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &dpb7000_state::main_map);

	INPUT_MERGER_ANY_HIGH(config, m_p_int).output_handler().set_inputline(m_maincpu, 3);

	ACIA6850(config, m_acia[0], 0);
	m_acia[0]->txd_handler().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_acia[0]->rts_handler().set(m_rs232, FUNC(rs232_port_device::write_rts));
	m_acia[0]->irq_handler().set_inputline(m_maincpu, 6);

	ACIA6850(config, m_acia[1], 0);
	m_acia[1]->irq_handler().set(m_p_int, FUNC(input_merger_device::in_w<0>));

	ACIA6850(config, m_acia[2], 0);
	m_acia[2]->irq_handler().set(m_p_int, FUNC(input_merger_device::in_w<1>));

	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_acia[0], FUNC(acia6850_device::write_rxd));
	m_rs232->dcd_handler().set(m_acia[0], FUNC(acia6850_device::write_dcd));
	m_rs232->cts_handler().set(m_acia[0], FUNC(acia6850_device::write_cts));

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

	PALETTE(config, m_palette, palette_device::MONOCHROME);

	// The 6545's clock is driven by the QD output of a 4-bit binary counter, which has its preset wired to 0010.
	// It therefore operates as a divide-by-six counter for the master clock of 22.248MHz.
	SY6545_1(config, m_crtc, 22.248_MHz_XTAL / 6);
	m_crtc->set_char_width(8);
	m_crtc->set_show_border_area(false);
	m_crtc->set_screen("screen");
	m_crtc->set_update_row_callback(FUNC(dpb7000_state::crtc_update_row), this);
	m_crtc->set_on_update_addr_change_callback(FUNC(dpb7000_state::crtc_addr_changed), this);

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

	RS232_PORT(config, m_fdd_serial, default_rs232_devices, nullptr);
	m_fdd_serial->rxd_handler().set(FUNC(dpb7000_state::fddcpu_debug_rx));

	config.m_perfect_cpu_quantum = subtag("fddcpu");

	// Filter Card
	TDC1008(config, m_filter_cd);
	TDC1008(config, m_filter_ce);
	TDC1008(config, m_filter_cf);
	TDC1008(config, m_filter_cg);

	// Store Address Cards
	DPB7000_STOREADDR(config, m_store_addr[0]);
	DPB7000_STOREADDR(config, m_store_addr[1]);

	// Brush Processor Cards
	DPB7000_BRUSHPROC(config, m_brush_proc[0]);
	DPB7000_BRUSHPROC(config, m_brush_proc[1]);

	// Brush Store Card
	DPB7000_BRUSHSTORE(config, m_brush_store);

	// Combiner Card
	DPB7000_COMBINER(config, m_combiner, 14.318181_MHz_XTAL);

	// Framestore Cards
	for (size_t i = 0; i < FRAMESTORE_COUNT; i++)
	{
		DPB7000_FRAMESTORE(config, m_framestore[i]);
	}

	for (size_t i = 0; i < 2; i++)
	{
		for (size_t j = 0; j < FRAMESTORE_COUNT; j++)
		{
			m_store_addr[i]->ipsel().set(m_framestore[j], FUNC(dpb7000_framestore_card_device::ipsel_w));
			m_store_addr[i]->csel().set(m_framestore[j], FUNC(dpb7000_framestore_card_device::csel_w));
			m_store_addr[i]->rck().set(m_framestore[j], FUNC(dpb7000_framestore_card_device::rck_w));
			m_store_addr[i]->cck().set(m_framestore[j], FUNC(dpb7000_framestore_card_device::cck_w));
			m_store_addr[i]->ra().set(m_framestore[j], FUNC(dpb7000_framestore_card_device::ra_w));
			m_store_addr[i]->opstr().set(m_framestore[j], FUNC(dpb7000_framestore_card_device::opstr_w));
			m_store_addr[i]->opwa().set(m_framestore[j], FUNC(dpb7000_framestore_card_device::opwa_w));
			m_store_addr[i]->opwb().set(m_framestore[j], FUNC(dpb7000_framestore_card_device::opwb_w));
			m_store_addr[i]->opra().set(m_framestore[j], FUNC(dpb7000_framestore_card_device::opra_w));
			m_store_addr[i]->oprb().set(m_framestore[j], FUNC(dpb7000_framestore_card_device::oprb_w));
			m_store_addr[i]->a().set(m_framestore[j], FUNC(dpb7000_framestore_card_device::a_w));
			m_store_addr[i]->ras().set(m_framestore[j], FUNC(dpb7000_framestore_card_device::ras_w));
			m_store_addr[i]->cas().set(m_framestore[j], FUNC(dpb7000_framestore_card_device::cas_w));
			m_store_addr[i]->write().set(m_framestore[j], FUNC(dpb7000_framestore_card_device::write_w));
		}
	}
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
ROM_END

COMP( 1981, dpb7000, 0, 0, dpb7000, dpb7000, dpb7000_state, empty_init, "Quantel", "DPB-7000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
