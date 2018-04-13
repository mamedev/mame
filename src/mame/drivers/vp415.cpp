// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
/***************************************************************************

    Skeleton driver for Philips VP415 LV ROM Player

	List of Modules:
		A - Audio Processor
		B - RGB
		C - Video Processor
		D - Ref Source
		E - Slide Drive
		F - Motor+Sequence
		G - Gen Lock
		H - ETBC B
		I - ETBC C
		J - Focus
		K - HF Processor
		L - Video Dropout Correction
		M - Radial
		N - Display Keyboard
		P - Front Loader
		Q - RC5 Mirror
		R - Drive Processor
		S - Control
		T - Supply
		U - Analog I/O
		V - Module Carrier
		W - CPU Datagrabber
		X - LV ROM
		Y - Vid Mix
		Z - Deck Electronics

	TODO:
	- Module W and Module S both have a NEC D8041AHC which has not yet
	  been dumped. Dumps will be necessary to properly emulate the
	  player.
	- Anyone who has information on the proper voltage with which to
	  dump a mask-type 8041 manufactured by NEC circa 1987, please
	  contact Ryan Holtz.
	- Driver currently fails the initial self-test with code 025. Per
	  the service manual, code 25 means "no REFV pulse at system
	  start-up".

***************************************************************************/

#include "emu.h"
#include "screen.h"

#include "cpu/mcs51/mcs51.h"
#include "cpu/z80/z80.h"

#include "machine/i8155.h"
#include "machine/i8255.h"
#include "machine/saa1043.h"
#include "video/mb88303.h"

class vp415_state : public driver_device
{
public:
	vp415_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, Z80CPU_TAG)
		, m_drivecpu(*this, DRIVECPU_TAG)
		, m_chargen(*this, CHARGEN_TAG)
		, m_mainram(*this, Z80RAM_TAG)
		, m_ctrlram(*this, CTRLRAM_TAG)
		, m_switches(*this, SWITCHES_TAG)
	{ }

	void vp415(machine_config &config);

	virtual void machine_reset() override;
	virtual void machine_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE8_MEMBER(ncr5385_w);
	DECLARE_READ8_MEMBER(ncr5385_r);
	DECLARE_WRITE8_MEMBER(sel34_w);
	DECLARE_READ8_MEMBER(sel37_r);
	DECLARE_WRITE8_MEMBER(ctrl_regs_w);
	DECLARE_READ8_MEMBER(ctrl_regs_r);

	DECLARE_READ8_MEMBER(drive_i8155_pb_r);
	DECLARE_READ8_MEMBER(drive_i8155_pc_r);

	DECLARE_WRITE8_MEMBER(drive_i8255_pa_w);
	DECLARE_WRITE8_MEMBER(drive_i8255_pb_w);
	DECLARE_READ8_MEMBER(drive_i8255_pc_r);
	DECLARE_WRITE8_MEMBER(drive_cpu_port1_w);

	DECLARE_WRITE_LINE_MEMBER(refv_w);

	static const char* Z80CPU_TAG;
	static const char* Z80RAM_TAG;
	static const char* CTRLCPU_TAG;
	static const char* CTRLRAM_TAG;
	static const char* DRIVECPU_TAG;
	static const char* DESCRAMBLE_ROM_TAG;
	static const char* SYNC_ROM_TAG;
	static const char* DRIVE_ROM_TAG;
	static const char* CONTROL_ROM_TAG;
	static const char* SWITCHES_TAG;
	static const char* I8155_TAG;
	static const char* I8255_TAG;
	static const char* CHARGEN_TAG;
	static const char* SYNCGEN_TAG;

protected:
	enum
	{
		SEL34_INTR_N = 0x01,
		SEL34_RES    = 0x20,
		SEL34_ERD    = 0x40,
		SEL34_ENW    = 0x80,
		SEL34_INTR_N_BIT = 0,
		SEL34_RES_BIT    = 5,
		SEL34_ERD_BIT    = 6,
		SEL34_ENW_BIT    = 7,
	};

	enum
	{
		SEL37_ID0   = 0x01,
		SEL37_ID1   = 0x02,
		SEL37_BRD   = 0x10,
		SEL37_MON_N = 0x20,
		SEL37_SK1c  = 0x40,
		SEL37_SK1d  = 0x40,

		SEL37_ID0_BIT   = 0,
		SEL37_ID1_BIT   = 1,
		SEL37_BRD_BIT   = 4,
		SEL37_MON_N_BIT = 5,
		SEL37_SK1c_BIT  = 6,
		SEL37_SK1d_BIT  = 7,
	};

	enum
	{
		DIAG_TURN_MISCOMPARE_INITIAL = 0x08,
		DIAG_TURN_MISCOMPARE_FINAL   = 0x10,
		DIAG_TURN_GOOD_PARITY        = 0x18,
		DIAG_TURN_BAD_PARITY         = 0x20,
		DIAG_COMPLETE = 0x80,

		DIAG_COMPLETE_BIT = 7,
	};

	enum
	{
		INT_FUNC_COMPLETE = 0x01,
		INT_INVALID_CMD   = 0x40,

		INT_FUNC_COMPLETE_BIT = 0,
		INT_INVALID_CMD_BIT   = 6,
	};

	enum
	{
		AUX_STATUS_TC_ZERO    = 0x02,
		AUX_STATUS_PAUSED     = 0x04,
		AUX_STATUS_PARITY_ERR = 0x40,
		AUX_STATUS_DATA_FULL  = 0x80,

		AUX_STATUS_TC_ZERO_BIT    = 1,
		AUX_STATUS_PAUSED_BIT     = 2,
		AUX_STATUS_PARITY_ERR_BIT = 6,
		AUX_STATUS_DATA_FULL_BIT  = 7,
	};

	enum
	{
		STATE_IDLE,
		STATE_DIAGNOSTIC_GOOD_PARITY,
		STATE_DIAGNOSTIC_BAD_PARITY,
	};

	enum
	{
		CTRL_SELECT   = 0x01,
		CTRL_RESELECT = 0x02,
		CTRL_PARITY   = 0x04,

		CTRL_SELECT_BIT   = 0,
		CTRL_RESELECT_BIT = 1,
		CTRL_PARITY_BIT   = 2,
	};

	enum
	{
		I8255PB_COMM1    = 0x01,
		I8255PB_COMM2    = 0x02,
		I8255PB_COMM3    = 0x04,
		I8255PB_COMM4    = 0x08,
		I8255PB_RLS_N    = 0x10,
		I8255PB_SL_PWR   = 0x20,
		I8255PB_RAD_FS_N = 0x40,
		I8255PB_STR1     = 0x80,

		I8255PB_COMM1_BIT    = 0,
		I8255PB_COMM2_BIT    = 1,
		I8255PB_COMM3_BIT    = 2,
		I8255PB_COMM4_BIT    = 3,
		I8255PB_RLS_N_BIT    = 4,
		I8255PB_SL_PWR_BIT   = 5,
		I8255PB_RAD_FS_N_BIT = 6,
		I8255PB_STR1_BIT     = 7,
	};

	virtual void video_start() override;

	void z80_program_map(address_map &map);
	void z80_io_map(address_map &map);
	void set_intn_line(uint8_t line, uint8_t value);

	void ctrl_program_map(address_map &map);
	void ctrl_io_map(address_map &map);
	void sd_w(uint8_t data);
	uint8_t sd_r();

	void drive_program_map(address_map &map);
	void drive_io_map(address_map &map);

	required_device<z80_device> m_maincpu;
	required_device<i8031_device> m_drivecpu;
	required_device<mb88303_device> m_chargen;
	required_shared_ptr<uint8_t> m_mainram;
	required_shared_ptr<uint8_t> m_ctrlram;
	required_ioport m_switches;

	uint8_t m_sel34;
	uint8_t m_sel37;

	uint32_t m_state;
	uint8_t m_ctrl_reg;
	uint8_t m_int_reg;
	uint8_t m_aux_status_reg;
	uint8_t m_diag_status_reg;
	uint8_t m_intn_lines[2];

	uint8_t m_refv;
};

/*static*/ const char* vp415_state::Z80CPU_TAG = "z80cpu";
/*static*/ const char* vp415_state::Z80RAM_TAG = "z80ram";
/*static*/ const char* vp415_state::CTRLCPU_TAG = "ctrlcpu";
/*static*/ const char* vp415_state::CTRLRAM_TAG = "ctrlram";
/*static*/ const char* vp415_state::DRIVECPU_TAG = "drivecpu";
/*static*/ const char* vp415_state::DESCRAMBLE_ROM_TAG = "descramblerom";
/*static*/ const char* vp415_state::SYNC_ROM_TAG = "syncrom";
/*static*/ const char* vp415_state::DRIVE_ROM_TAG = "driverom";
/*static*/ const char* vp415_state::CONTROL_ROM_TAG = "controlrom";
/*static*/ const char* vp415_state::SWITCHES_TAG = "SWITCHES";
/*static*/ const char* vp415_state::I8155_TAG = "i8155";
/*static*/ const char* vp415_state::I8255_TAG = "i8255";
/*static*/ const char* vp415_state::CHARGEN_TAG = "mb88303";
/*static*/ const char* vp415_state::SYNCGEN_TAG = "saa1043";

void vp415_state::machine_reset()
{
	m_state = STATE_IDLE;
	m_int_reg = 0;
	m_ctrl_reg = 0;
	m_aux_status_reg = AUX_STATUS_TC_ZERO;
	m_diag_status_reg = DIAG_COMPLETE;

	m_sel34 = 0;
	m_sel37 = SEL37_BRD | SEL37_MON_N | SEL37_SK1c | SEL37_SK1d;
	m_intn_lines[0] = 1;
	m_intn_lines[1] = 1;
}

void vp415_state::machine_start()
{
}

WRITE_LINE_MEMBER(vp415_state::refv_w)
{
	m_refv = state;
	m_drivecpu->set_input_line(MCS51_INT0_LINE, m_refv ? ASSERT_LINE : CLEAR_LINE);
}

// CPU Datagrabber Module (W)

WRITE8_MEMBER(vp415_state::ncr5385_w)
{
	switch (offset)
	{
		case 0x0: // Data Register
			switch (m_state)
			{
				case STATE_DIAGNOSTIC_GOOD_PARITY:
					m_aux_status_reg &= ~AUX_STATUS_PARITY_ERR;
					m_aux_status_reg |= AUX_STATUS_DATA_FULL;
					m_int_reg = INT_FUNC_COMPLETE;
					m_diag_status_reg = DIAG_COMPLETE | DIAG_TURN_GOOD_PARITY;
					m_state = STATE_IDLE;
					set_intn_line(1, 0);
					logerror("%s: ncr5385_w: data=%02x (diagnostic w/ good parity)\n", machine().describe_context(), data);
					break;
				case STATE_DIAGNOSTIC_BAD_PARITY:
					m_aux_status_reg |= AUX_STATUS_PARITY_ERR | AUX_STATUS_DATA_FULL;
					m_int_reg = INT_FUNC_COMPLETE;
					m_diag_status_reg = DIAG_COMPLETE | DIAG_TURN_BAD_PARITY;
					m_state = STATE_IDLE;
					set_intn_line(1, 0);
					logerror("%s: ncr5385_w: data=%02x (diagnostic w/ bad parity)\n", machine().describe_context(), data);
					break;
				default:
					logerror("%s: ncr5385_w: data=%02x\n", machine().describe_context(), data);
					break;
			}
			break;
		case 0x1: // Command Register
			switch (data & 0x3f)
			{
				case 0x00: // Chip Reset
					logerror("%s: ncr5385_w: command: reset\n", machine().describe_context());
					m_state = STATE_IDLE;
					m_int_reg = 0;
					m_aux_status_reg = AUX_STATUS_TC_ZERO;
					m_diag_status_reg = DIAG_COMPLETE;
					set_intn_line(1, 1);
					break;
				case 0x0b: // Diagnostic
					logerror("%s: ncr5385_w: command: diagnostic (%s parity)\n", machine().describe_context(), BIT(data, 6) ? "bad" : "good");
					if (BIT(data, 6))
						m_state = STATE_DIAGNOSTIC_BAD_PARITY;
					else
						m_state = STATE_DIAGNOSTIC_GOOD_PARITY;
					break;
				default:
					logerror("%s: ncr5385_w: command: %02x\n", machine().describe_context(), data);
					break;
			}
			break;
		case 0x2: // Control Register
			m_ctrl_reg = data & 0x07;
			logerror("%s: ncr5385_w: control: parity_en=%d, reselect_en=%d, select_en=%d\n", machine().describe_context(), BIT(data, CTRL_PARITY_BIT), BIT(data, CTRL_RESELECT_BIT), BIT(data, CTRL_SELECT_BIT));
			break;
		default:
			logerror("%s: ncr5385_w: %x=%02x\n", machine().describe_context(), offset, data);
			break;
	}
}

READ8_MEMBER(vp415_state::ncr5385_r)
{
	switch (offset)
	{
		case 0x2:
			logerror("%s: ncr5385_r: control (%02x)\n", machine().describe_context(), m_ctrl_reg);
			return m_ctrl_reg;
		case 0x4:
			logerror("%s: ncr5385_r: aux status (%02x)\n", machine().describe_context(), m_aux_status_reg);
			return m_aux_status_reg;
		case 0x6:
			logerror("%s: ncr5385_r: interrupt (%02x)\n", machine().describe_context(), m_int_reg);
			set_intn_line(1, 1);
			return m_int_reg;
		case 0x9:
			logerror("%s: ncr5385_r: diagnostic status (%02x)\n", machine().describe_context(), m_diag_status_reg);
			return m_diag_status_reg;
		default:
			logerror("%s: ncr5385_r: %x (%02x)\n", machine().describe_context(), offset, 0);
			return 0;
	}
}

WRITE8_MEMBER(vp415_state::sel34_w)
{
	logerror("%s: sel34: /INTR=%d, RES=%d, ERD=%d, ENW=%d\n", machine().describe_context(), BIT(data, SEL34_INTR_N_BIT), BIT(data, SEL34_RES_BIT), BIT(data, SEL34_ERD_BIT), BIT(data, SEL34_ENW_BIT));
	m_sel34 = data;

	if (!BIT(data, SEL34_INTR_N_BIT))
	{
		m_sel37 &= ~(SEL37_ID0 | SEL37_ID1);
	}
}

READ8_MEMBER(vp415_state::sel37_r)
{
	logerror("%s: sel37: ID0=%d, ID1=%d\n", machine().describe_context(), BIT(m_sel37, SEL37_ID0_BIT), BIT(m_sel37, SEL37_ID1_BIT));
	return m_sel37;
}

void vp415_state::set_intn_line(uint8_t line, uint8_t value)
{
	if (value == 0)
	{
		m_sel37 |= line ? SEL37_ID0 : SEL37_ID1;
	}

	m_intn_lines[line] = value;
	if (m_intn_lines[0] && m_intn_lines[1])
	{
		m_maincpu->set_input_line(0, CLEAR_LINE);
	}
	else
	{
		m_maincpu->set_input_line(0, ASSERT_LINE);
	}
}

void vp415_state::z80_program_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region(Z80CPU_TAG, 0);
	map(0xa000, 0xfeff).ram().share(Z80RAM_TAG);
}

void vp415_state::z80_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x0f).rw(this, FUNC(vp415_state::ncr5385_r), FUNC(vp415_state::ncr5385_w));
	// 0x20, 0x21: Connected to A0 + D0..D7 of SLAVE i8041
	map(0x34, 0x34).w(this, FUNC(vp415_state::sel34_w));
	map(0x37, 0x37).r(this, FUNC(vp415_state::sel37_r));
}



// Control Module (S)

WRITE8_MEMBER(vp415_state::ctrl_regs_w)
{
	const uint8_t handler_index = (offset & 0x0c00) >> 10;
	switch (handler_index)
	{
		case 0: // WREN
			logerror("%s: ctrl_regs_w: WREN: %02x\n", machine().describe_context(), data);
			sd_w(data);
			break;
		case 1: // WR3
			logerror("%s: ctrl_regs_w: WR3 (UPI-41, not yet dumped/implemented): %02x\n", machine().describe_context(), data);
			break;
		case 2:
			logerror("%s: ctrl_regs_w: N.C. write %02x\n", machine().describe_context(), data);
			break;
		case 3:
			logerror("%s: ctrl_regs_w: output buffer: VP=%d, NPL=%d, WR-CLK=%d, DB/STAT=%d, RD-STRT=%d, V/C-TXT=%d\n", machine().describe_context(), data & 0x7, BIT(data, 4), BIT(data, 3), BIT(data, 5), BIT(data, 6), BIT(data, 7));
			break;
	}
}

READ8_MEMBER(vp415_state::ctrl_regs_r)
{
	const uint8_t handler_index = (offset & 0x0c00) >> 10;
	uint8_t value = 0;
	switch (handler_index)
	{
		case 0: // RDEN
			value = sd_r();
			logerror("%s: ctrl_regs_r: RDEN: %02x\n", machine().describe_context(), value);
			break;
		case 1: // RD3
			logerror("%s: ctrl_regs_r: RD3 (UPI-41, not yet dumped/implemented): %02x\n", machine().describe_context(), 0);
			break;
		case 2: // RD2
			logerror("%s: ctrl_regs_r: N.C. read\n", machine().describe_context());
			break;
		case 3:
			value = m_switches->read();
			logerror("%s: ctrl_regs_r: RD1 (DIP switches): %02x\n", machine().describe_context(), value);
			break;
	}
	return value;
}

uint8_t vp415_state::sd_r()
{
	return 0;
}

void vp415_state::sd_w(uint8_t data)
{
}

void vp415_state::ctrl_program_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region(CONTROL_ROM_TAG, 0);
}

void vp415_state::ctrl_io_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share(CTRLRAM_TAG);
	map(0xe000, 0xffff).rw(this, FUNC(vp415_state::ctrl_regs_r), FUNC(vp415_state::ctrl_regs_w)).mask(0x1c00);
}

// Drive Processor Module (R)

READ8_MEMBER(vp415_state::drive_i8155_pb_r)
{
	logerror("%s: drive_i8155_pb_r: %02x\n", machine().describe_context(), 0);
	return 0;
}

READ8_MEMBER(vp415_state::drive_i8155_pc_r)
{
	logerror("%s: drive_i8155_pc_r: %02x\n", machine().describe_context(), 0);
	return 0;
}

WRITE8_MEMBER(vp415_state::drive_i8255_pa_w)
{
	logerror("%s: drive_i8255_pa_w: %02x\n", machine().describe_context(), data);
}

WRITE8_MEMBER(vp415_state::drive_i8255_pb_w)
{
	logerror("%s: drive_i8255_pb_w: COMM-1:%d, COMM-2:%d, COMM-3:%d, COMM-4:%d, /RLS:%d, SL-PWR:%d, /RAD-FS:%d, STR1:%d\n"
		, machine().describe_context()
		, BIT(data, I8255PB_COMM1_BIT)
		, BIT(data, I8255PB_COMM2_BIT)
		, BIT(data, I8255PB_COMM3_BIT)
		, BIT(data, I8255PB_COMM4_BIT)
		, BIT(data, I8255PB_RLS_N_BIT)
		, BIT(data, I8255PB_SL_PWR_BIT)
		, BIT(data, I8255PB_RAD_FS_N_BIT)
		, BIT(data, I8255PB_STR1_BIT));
}

READ8_MEMBER(vp415_state::drive_i8255_pc_r)
{
	logerror("%s: drive_i8255_pc_r: %02x\n", machine().describe_context(), 0);
	return 0;
}

WRITE8_MEMBER(vp415_state::drive_cpu_port1_w)
{
	//logerror("%s: drive_cpu_port1_w: %02x\n", machine().describe_context(), data);
	m_chargen->ldi_w(BIT(data, 2));
}

void vp415_state::drive_program_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region(DRIVE_ROM_TAG, 0);
}

void vp415_state::drive_io_map(address_map &map)
{
	map(0x0000, 0x0003).mirror(0xfbfc).rw(I8255_TAG, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x0400, 0x04ff).mirror(0xf800).rw(I8155_TAG, FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
	map(0x0500, 0x0507).mirror(0xf8f8).rw(I8155_TAG, FUNC(i8155_device::io_r), FUNC(i8155_device::io_w));
}

void vp415_state::video_start()
{
}

uint32_t vp415_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_chargen->update_bitmap(bitmap, cliprect);
	return 0;
}

static INPUT_PORTS_START( vp415 )
	PORT_START(vp415_state::SWITCHES_TAG)
		PORT_BIT( 0x03, IP_ACTIVE_HIGH, IPT_UNUSED )
		PORT_DIPNAME( 0x04, 0x00, "BRA (unknown purpose)" )
		PORT_DIPSETTING(    0x04, "On" )
		PORT_DIPSETTING(    0x00, "Off" )
		PORT_DIPNAME( 0x08, 0x00, "BRB (unknown purpose)" )
		PORT_DIPSETTING(    0x08, "On" )
		PORT_DIPSETTING(    0x00, "Off" )
		PORT_DIPNAME( 0x10, 0x00, "DUMP (unknown purpose)" )
		PORT_DIPSETTING(    0x10, "On" )
		PORT_DIPSETTING(    0x00, "Off" )
		PORT_DIPNAME( 0x20, 0x00, "Remote Control IR/Euro" )
		PORT_DIPSETTING(    0x20, "IR" )
		PORT_DIPSETTING(    0x00, "Euro" )
		PORT_DIPNAME( 0x40, 0x00, "REPLAY (unknown purpose)" )
		PORT_DIPSETTING(    0x40, "On" )
		PORT_DIPSETTING(    0x00, "Off" )
		PORT_DIPNAME( 0x80, 0x00, "RESI (unknown purpose)" )
		PORT_DIPSETTING(    0x80, "On" )
		PORT_DIPSETTING(    0x00, "Off" )
INPUT_PORTS_END

MACHINE_CONFIG_START(vp415_state::vp415)
	MCFG_CPU_ADD(Z80CPU_TAG, Z80, XTAL(8'000'000)/2) // 8MHz through a /2 flip-flop divider, per schematic
	MCFG_CPU_PROGRAM_MAP(z80_program_map)
	MCFG_CPU_IO_MAP(z80_io_map)

	MCFG_CPU_ADD(CTRLCPU_TAG, I8031, XTAL(11'059'200)) // 12MHz, per schematic
	MCFG_CPU_PROGRAM_MAP(ctrl_program_map)
	MCFG_CPU_IO_MAP(ctrl_io_map)

	MCFG_CPU_ADD(DRIVECPU_TAG, I8031, XTAL(12'000'000)) // 12MHz, per schematic
	MCFG_MCS51_PORT_P1_OUT_CB(WRITE8(vp415_state, drive_cpu_port1_w));
	MCFG_CPU_PROGRAM_MAP(drive_program_map)
	MCFG_CPU_IO_MAP(drive_io_map)

	MCFG_DEVICE_ADD(I8155_TAG, I8155, 0)
	MCFG_I8155_OUT_PORTA_CB(DEVWRITE8(CHARGEN_TAG, mb88303_device, da_w))
	MCFG_I8155_IN_PORTB_CB(READ8(vp415_state, drive_i8155_pb_r))
	MCFG_I8155_IN_PORTC_CB(READ8(vp415_state, drive_i8155_pc_r))

	MCFG_DEVICE_ADD(I8255_TAG, I8255, 0)
	MCFG_I8255_OUT_PORTA_CB(WRITE8(vp415_state, drive_i8255_pa_w))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(vp415_state, drive_i8255_pb_w))
	MCFG_I8255_IN_PORTC_CB(READ8(vp415_state, drive_i8255_pc_r))

	MCFG_DEVICE_ADD(CHARGEN_TAG, MB88303, 0)

	MCFG_DEVICE_ADD(SYNCGEN_TAG, SAA1043, XTAL(5'000'000))
	MCFG_SAA1043_V2_CALLBACK(WRITELINE(vp415_state, refv_w))

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_UPDATE_DRIVER(vp415_state, screen_update)
	MCFG_SCREEN_SIZE(320, 240)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
MACHINE_CONFIG_END

ROM_START(vp415)
	/* Module R */
	ROM_REGION(0x4000, vp415_state::DRIVE_ROM_TAG, 0) // Version 1.7
	ROM_LOAD( "r.3104 103 6803.6_drive.ic4", 0x0000, 0x4000, CRC(02e4273e) SHA1(198f7ac8f2a880f38046c9d9075ce32f3b730bd4) )

	/* Module S */
	ROM_REGION(0x10000, vp415_state::CONTROL_ROM_TAG, 0) // Version 1.8
	ROM_LOAD( "s.3104 103 6804.9_control.ic2", 0x0000, 0x10000, CRC(10564765) SHA1(8eb6cff7ca7cbfcb3db8b04b697cdd7e364be805) )

	/* Module W */
	ROM_REGION(0x8000, vp415_state::Z80CPU_TAG, 0)
	ROM_LOAD( "w.3104 103 6805.3_cpu", 0x0000, 0x4000, CRC(c2cf4f25) SHA1(e55e1ac917958eb42244bff17a0016b74627c8fa) ) // Version 1.3
	ROM_LOAD( "w.3104 103 6806.3_cpu", 0x4000, 0x4000, CRC(14a45ea0) SHA1(fa028d01094be91e3480c9ad35d46b5546f9ff0f) ) // Version 1.4

	ROM_REGION(0x4000, vp415_state::DESCRAMBLE_ROM_TAG, 0)
	ROM_LOAD( "w.3104 103 6807.0_cpu", 0x0000, 0x4000, CRC(19c2bc87) SHA1(152f8c645588be9fc0dbc840368ab33a13a04e62) ) // Version 1.0

	ROM_REGION(0x4000, vp415_state::SYNC_ROM_TAG, 0)
	ROM_LOAD( "w.3104 103 6808.0_cpu", 0x0000, 0x4000, CRC(bdb601e0) SHA1(4f769aa62b756b157ba9ac8d3ae8bd1228821ff9) ) // Version 1.0
ROM_END

CONS( 1983, vp415, 0, 0, vp415, vp415, vp415_state, 0, "Philips", "VP415", MACHINE_IS_SKELETON )

