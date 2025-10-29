// license:BSD-3-Clause
// copyright-holders:AJR, R. Belmont
/***************************************************************************

    Skeleton driver for Akai MPC60 MIDI Production Center.

***************************************************************************/

#include "emu.h"

//#include "bus/midi/midi.h"
#include "cpu/i86/i186.h"
#include "cpu/upd7810/upd7810.h"
#include "formats/dfi_dsk.h"
#include "formats/dsk_dsk.h"
#include "formats/hxchfe_dsk.h"
#include "formats/hxcmfm_dsk.h"
#include "formats/imd_dsk.h"
#include "formats/ipf_dsk.h"
#include "formats/mfi_dsk.h"
#include "formats/pc_dsk.h"
#include "formats/td0_dsk.h"
#include "imagedev/floppy.h"
#include "machine/clock.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/timer.h"
#include "machine/upd765.h"
#include "video/hd61830.h"

#include "emupal.h"
#include "screen.h"

namespace {

static constexpr uint8_t BIT4 = (1 << 4);
static constexpr uint8_t BIT5 = (1 << 5);
class mpc60_state : public driver_device
{
public:
	mpc60_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_panelcpu(*this, "panelcpu")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:0")
		, m_ppi(*this, "ppi")
		, m_keys(*this, "Y%u", 0)
		, m_drums(*this, "PB%u", 0)
		, m_dataentry(*this, "DATAENTRY")
		, m_key_scan_row(0)
		, m_drum_scan_row(0)
		, m_variation_slider(0)
		, m_last_dial(0)
		, m_count_dial(0)
		, m_quadrature_phase(0)
	{
	}

	void mpc60(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(variation_changed);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u8 nvram_r(offs_t offset);
	void nvram_w(offs_t offset, u8 data);
	void fdc_tc_w(u8 data);

	static void floppies(device_slot_interface &device);

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void panel_map(address_map &map) ATTR_COLD;
	void lcd_map(address_map &map) ATTR_COLD;

	uint8_t subcpu_pa_r();
	uint8_t subcpu_pb_r();
	uint8_t subcpu_pc_r();
	void subcpu_pb_w(uint8_t data);
	void subcpu_pc_w(uint8_t data);
	uint8_t an0_r();
	uint8_t an1_r();
	uint8_t an2_r();
	uint8_t an3_r();
	uint8_t an4_r();

	uint8_t ppi_pb_r();
	void ppi_pc_w(uint8_t data);

	TIMER_DEVICE_CALLBACK_MEMBER(dial_timer_tick);

	required_device<cpu_device> m_maincpu;
	required_device<upd7810_device> m_panelcpu;
	required_device<upd72065_device> m_fdc;
	required_device<floppy_connector> m_floppy;
	required_device<i8255_device> m_ppi;
	required_ioport_array<8> m_keys;
	required_ioport_array<4> m_drums;
	required_ioport  m_dataentry;

	std::unique_ptr<u8[]> m_nvram_data;

	uint8_t m_key_scan_row, m_drum_scan_row, m_variation_slider;
	int m_last_dial, m_count_dial, m_quadrature_phase;
};

void mpc60_state::machine_start()
{
	m_nvram_data = make_unique_clear<u8[]>(0x800);
	subdevice<nvram_device>("nvram")->set_base(&m_nvram_data[0], 0x800);

	save_pointer(NAME(m_nvram_data), 0x800);
}

u8 mpc60_state::nvram_r(offs_t offset)
{
	return m_nvram_data[offset];
}

void mpc60_state::nvram_w(offs_t offset, u8 data)
{
	m_nvram_data[offset] = data;
}

void mpc60_state::fdc_tc_w(u8 data)
{
	m_fdc->tc_w(0);
	m_fdc->tc_w(1);
}

void mpc60_state::mem_map(address_map &map)
{
	map(0x00000, 0x7ffff).ram();
	map(0xbf000, 0xbffff).rw(FUNC(mpc60_state::nvram_r), FUNC(mpc60_state::nvram_w)).umask16(0x00ff);
	map(0xc0000, 0xfffff).rom().region("program", 0);
}

void mpc60_state::io_map(address_map &map)
{
	map(0x0080, 0x0083).m(m_fdc, FUNC(upd72065_device::map)).umask16(0x00ff);
	map(0x0090, 0x0090).w(FUNC(mpc60_state::fdc_tc_w));
	map(0x00a0, 0x00a0).rw(m_fdc, FUNC(upd72065_device::dma_r), FUNC(upd72065_device::dma_w));
	map(0x00b0, 0x00b0).r("lcdc", FUNC(hd61830_device::status_r));
	map(0x00b2, 0x00b2).r("lcdc", FUNC(hd61830_device::data_r));
	map(0x00b4, 0x00b4).w("lcdc", FUNC(hd61830_device::control_w));
	map(0x00b6, 0x00b6).w("lcdc", FUNC(hd61830_device::data_w));
	map(0x0200, 0x0207).rw(m_ppi, FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
}

void mpc60_state::panel_map(address_map &map)
{
	map(0x4000, 0x5fff).rom().region("panel", 0);
}

void mpc60_state::lcd_map(address_map &map)
{
	map.global_mask(0x07ff);
	map(0x0000, 0x07ff).ram();
}

uint8_t mpc60_state::subcpu_pa_r()
{
	return m_keys[7 - m_key_scan_row]->read();
}

uint8_t mpc60_state::subcpu_pb_r()
{
	return m_drums[m_drum_scan_row]->read();
}

uint8_t mpc60_state::subcpu_pc_r()
{
	uint8_t rv = 0;

	if (m_count_dial)
	{
		const bool negative = (m_count_dial < 0);

		switch (m_quadrature_phase >> 1)
		{
		case 0:
			rv = negative ? BIT5 : BIT4;
			break;

		case 1:
			rv = BIT4 | BIT5;
			break;

		case 2:
			rv = negative ? BIT4 : BIT5;
			break;

		case 3:
			rv = 0;
			break;
		}
		m_quadrature_phase++;
		m_quadrature_phase &= 7;

		// generate a complete 4-part pulse train for each single change in the position
		if (m_quadrature_phase == 0)
		{
			if (m_count_dial < 0)
			{
				m_count_dial++;
			}
			else
			{
				m_count_dial--;
			}
		}
	}

	return rv;
}

TIMER_DEVICE_CALLBACK_MEMBER(mpc60_state::dial_timer_tick)
{
	const int new_dial = m_dataentry->read();

	if (new_dial != m_last_dial)
	{
		int diff = new_dial - m_last_dial;
		if (diff > 0x80)
		{
			diff = 0x100 - diff;
		}
		if (diff < -0x80)
		{
			diff = -0x100 - diff;
		}

		m_count_dial += diff;
		m_last_dial = new_dial;
	}
}

// drum pad row select, active low
void mpc60_state::subcpu_pb_w(uint8_t data)
{
	m_drum_scan_row = (data & 0xf) ^ 0xf;
	if (m_drum_scan_row != 0)
	{
		// get a row number 0-3
		m_drum_scan_row = count_leading_zeros_32(m_drum_scan_row) - 28;
	}
}

uint8_t mpc60_state::an0_r()
{
	return 0xff;
}

uint8_t mpc60_state::an1_r()
{
	return 0xff;
}

uint8_t mpc60_state::an2_r()
{
	return 0xff;
}

uint8_t mpc60_state::an3_r()
{
	return 0xff;
}

uint8_t mpc60_state::an4_r()
{
	return m_variation_slider;
}

INPUT_CHANGED_MEMBER(mpc60_state::variation_changed)
{
	if (!oldval && newval)
	{
		m_variation_slider = newval;
	}
}

// main buttons row select (PC1-PC3)
void mpc60_state::subcpu_pc_w(uint8_t data)
{
	m_key_scan_row = ((data ^ 0xff) >> 1) & 7;
}

// PA0-6 = 7 bits of analog input from AD7523JN

// PB0 = FOOT 1     (Footswitch 1)
// PB1 = FOOT 2     (Footswitch 2)
// PB2 = DRV RDY    (FDC ready)
// PB3 = CLK WT
// PB4 = F/R        (I-0055 sync chip)
// PB5 = SMTINT     (I-0055 sync chip)
// PB6 = URE        (I-0055 sync chip)
// PB7 = ORE        (I-0055 sync chip)
uint8_t mpc60_state::ppi_pb_r()
{
	uint8_t rv = 0;

	rv |= m_floppy->get_device()->ready_r() ? 0x00 : 0x04;

	return rv;
}

// PC0 = FDCRST (REST pin on uPD766)
// PC1 = A input to 74HC153 at IC14
// PC2 = B input to 74HC153 at IC14
// PC3 = mute analog in?
// PC4 = TC/UB (I-0055 sync chip)
// PC5 = SMT/CLK (I-0055 sync chip)
// PC6 = FDC motor on
// PC7 = Something to do with the ADC
void mpc60_state::ppi_pc_w(uint8_t data)
{
	// TBD: when should we actually do this?
	if (BIT(data, 0) && m_floppy->get_device())
	{
		m_fdc->set_floppy(m_floppy->get_device());
	}

	m_fdc->reset_w(BIT(data, 0));
	m_floppy->get_device()->mon_w(BIT(data, 6) ^ 1);
}

void mpc60_state::floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

static void add_formats(format_registration &fr)
{
	fr.add(FLOPPY_DFI_FORMAT);
	fr.add(FLOPPY_MFM_FORMAT);
	fr.add(FLOPPY_TD0_FORMAT);
	fr.add(FLOPPY_IMD_FORMAT);
	fr.add(FLOPPY_DSK_FORMAT);
	fr.add(FLOPPY_PC_FORMAT);
	fr.add(FLOPPY_IPF_FORMAT);
	fr.add(FLOPPY_HFE_FORMAT);
}

static INPUT_PORTS_START(mpc60)
	PORT_START("Y0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Full Level") PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("16 Levels") PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Pad Bank") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("After") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("Y1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Soft Key 1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Soft Key 2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Soft Key 3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Soft Key 4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Help") PORT_CODE(KEYCODE_J)

	PORT_START("Y2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Disk") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tempo/Sync") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Drum Mix") PORT_CODE(KEYCODE_E)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Sounds") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Song Mode") PORT_CODE(KEYCODE_T)

	PORT_START("Y3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Edit") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Step Edit") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Edit Loop") PORT_CODE(KEYCODE_D)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MIDI") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Other") PORT_CODE(KEYCODE_G)

	PORT_START("Y4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_NAME(".")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Wait For Key") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Auto Punch") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_NAME("Up Arrow")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Transpose") PORT_CODE(KEYCODE_C)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Simul Seq") PORT_CODE(KEYCODE_V)

	PORT_START("Y5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("-")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("+")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Count In") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Left Arrow")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Down Arrow")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Right Arrow")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Main Screen") PORT_CODE(KEYCODE_P)

	PORT_START("Y6")
	PORT_BIT(0x07, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_NAME("<<")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_NAME("<")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Locate") PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(">") PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(">>") PORT_CODE(KEYCODE_F9)

	PORT_START("Y7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y) PORT_NAME("Erase")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Timing Correct") PORT_CODE(KEYCODE_U)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tap Tempo") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Rec") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Over Dub") PORT_CODE(KEYCODE_H)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Stop") PORT_CODE(KEYCODE_J)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Play") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Play Start") PORT_CODE(KEYCODE_L)

	PORT_START("PB0")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Crash") PORT_CODE(KEYCODE_PGUP)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Crash2") PORT_CODE(KEYCODE_NUMLOCK)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ride Cymbal") PORT_CODE(KEYCODE_SLASH_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Ride Bell") PORT_CODE(KEYCODE_ASTERISK)

	PORT_START("PB1")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("High Tom") PORT_CODE(KEYCODE_PGDN)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Mid Tom") PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Low Tom") PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Floor Tom") PORT_CODE(KEYCODE_9_PAD)

	PORT_START("PB2")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Alt Snare") PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Snare") PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Hihat Open") PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Hihat Pedal") PORT_CODE(KEYCODE_PLUS_PAD)

	PORT_START("PB3")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Side Stick") PORT_CODE(KEYCODE_0_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Bass") PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Hihat Closed") PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Hihat Loose") PORT_CODE(KEYCODE_3_PAD)

	PORT_START("VARIATION")
	PORT_ADJUSTER(100, "NOTE VARIATION") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(mpc60_state::variation_changed), 1)

	PORT_START("DATAENTRY")
	PORT_BIT( 0xff, 0x00, IPT_DIAL) PORT_SENSITIVITY(100) PORT_KEYDELTA(0)
INPUT_PORTS_END

void mpc60_state::mpc60(machine_config &config)
{
	I80186(config, m_maincpu, 20_MHz_XTAL); // MBL80186-10
	m_maincpu->set_addrmap(AS_PROGRAM, &mpc60_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &mpc60_state::io_map);

	CLOCK(config, "tmrck", 20_MHz_XTAL / 10).signal_handler().set(m_maincpu, FUNC(i80186_cpu_device::tmrin1_w)); // CPUCK output divided by 74HC390

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // CXK5816PN-15L + CL2020 battery

	UPD78C11(config, m_panelcpu, 12_MHz_XTAL);
	m_panelcpu->set_addrmap(AS_PROGRAM, &mpc60_state::panel_map);
	m_panelcpu->pa_in_cb().set(FUNC(mpc60_state::subcpu_pa_r));
	m_panelcpu->pb_in_cb().set(FUNC(mpc60_state::subcpu_pb_r));
	m_panelcpu->pb_out_cb().set(FUNC(mpc60_state::subcpu_pb_w));
	m_panelcpu->pc_in_cb().set(FUNC(mpc60_state::subcpu_pc_r));
	m_panelcpu->pc_out_cb().set(FUNC(mpc60_state::subcpu_pc_w));
	m_panelcpu->an0_func().set(FUNC(mpc60_state::an0_r));
	m_panelcpu->an1_func().set(FUNC(mpc60_state::an1_r));
	m_panelcpu->an2_func().set(FUNC(mpc60_state::an2_r));
	m_panelcpu->an3_func().set(FUNC(mpc60_state::an3_r));
	m_panelcpu->an4_func().set(FUNC(mpc60_state::an4_r));

	//MB89371(config, "sio01", 20_MHz_XTAL / 4);
	//MB89371(config, "sio23", 20_MHz_XTAL / 4);

	I8255(config, m_ppi); // MB89255A-P-C
	m_ppi->in_pb_callback().set(FUNC(mpc60_state::ppi_pb_r));
	m_ppi->out_pc_callback().set(FUNC(mpc60_state::ppi_pc_w));

	UPD72065(config, m_fdc, 16_MHz_XTAL / 4); // μPD72066C (clocked by SED9420CAC)
	m_fdc->set_ready_line_connected(false); // RDY tied to VDD (TODO: drive ready signal connected to PPI's PB2 instead)
	m_fdc->set_select_lines_connected(false);
	m_fdc->intrq_wr_callback().set(m_maincpu, FUNC(i80186_cpu_device::int0_w));
	m_fdc->drq_wr_callback().set(m_maincpu, FUNC(i80186_cpu_device::drq1_w)); // FIXME: delayed and combined with DRQAD

	FLOPPY_CONNECTOR(config, m_floppy, mpc60_state::floppies, "35dd", add_formats).enable_sound(true);

	hd61830_device &lcdc(HD61830(config, "lcdc", 0)); // LC7981
	lcdc.set_addrmap(0, &mpc60_state::lcd_map);
	lcdc.set_screen("screen");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(80);
	screen.set_screen_update("lcdc", FUNC(hd61830_device::screen_update));
	screen.set_size(240, 64);
	screen.set_visarea(0, 240-1, 0, 64-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	TIMER(config, "dialtimer").configure_periodic(FUNC(mpc60_state::dial_timer_tick), attotime::from_hz(60.0));

	//L4003(config, "voicelsi", 35.84_MHz_XTAL);
}

ROM_START(mpc60)
	ROM_REGION16_LE(0x40000, "program", 0)
	ROM_SYSTEM_BIOS(0, "v212", "v2.12") // V2.12 CPU ROMs (MBM27C512-20)
	ROMX_LOAD("mp6cpu2.ic2", 0x00000, 0x10000, CRC(e71b1acb) SHA1(b56ddfff1c546fc21341b1a614e18da9726312f4), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("mp6cpu3.ic3", 0x00001, 0x10000, CRC(f068838b) SHA1(42e815880d1c1a5b7d1c7933aad9c28410fc2627), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("mp6cpu1.ic4", 0x20000, 0x10000, CRC(1271bc73) SHA1(99fd6fa4c04e5bdf868e78072fec5b55c01350da), ROM_BIOS(0) | ROM_SKIP(1))
	ROMX_LOAD("mpc6cpu4.ic5", 0x20001, 0x10000, CRC(d922a66d) SHA1(0f4bc0522b9826d617f4af72382d75853515d7f5), ROM_BIOS(0) | ROM_SKIP(1))
	ROM_SYSTEM_BIOS(1, "v112", "v1.12")
	ROMX_LOAD("mpc60_v1-12_2.ic2", 0x00000, 0x10000, CRC(ddf26146) SHA1(987547198dc3984ab3dfa7f133ba7dca702cc269), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("mpc60_v1-12_4.ic3", 0x00001, 0x10000, CRC(9725d193) SHA1(6efda3d6760b3951c5036108106d446f6e128c59), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("mpc60_v1-12_1.ic4", 0x20000, 0x10000, CRC(f202dbb1) SHA1(6fd82224a99b52b6c414b88d5c920abda32ffa32), ROM_BIOS(1) | ROM_SKIP(1))
	ROMX_LOAD("mpc60_v1-12_3.ic5", 0x20001, 0x10000, CRC(ba5a1640) SHA1(1f9f49c49a3682b9a44d614ac411a7c043df399e), ROM_BIOS(1) | ROM_SKIP(1))

	ROM_REGION16_LE(0x10000, "waves", 0)
	ROM_LOAD16_BYTE("mpc60_voice_1_v1-0.ic17", 0x00000, 0x08000, CRC(b8fdfe3e) SHA1(c2f0e1d8813d4178d2f883a3f3e461e036b56229)) // lowest nibble is unused
	ROM_LOAD16_BYTE("mpc60_voice_2_v1-0.ic18", 0x00001, 0x08000, CRC(42f8e0a6) SHA1(a22dbefb9dafbb0c4095fd0bf4e63e67b5ec3b95))

	ROM_REGION(0x1000, "panelcpu", 0)
	ROM_LOAD("upd78c11g-044-36.ic1", 0x0000, 0x1000, NO_DUMP)
	ROM_FILL(0x0000, 1, 0x54) // dummy reset vector
	ROM_FILL(0x0001, 1, 0x00)
	ROM_FILL(0x0002, 1, 0x40)
	ROM_FILL(0x0018, 1, 0x54) // dummy interrupt vector
	ROM_FILL(0x0019, 1, 0x18)
	ROM_FILL(0x001a, 1, 0x40)
	ROM_FILL(0x0090, 1, 0xca) // dummy CALT vectors
	ROM_FILL(0x0091, 1, 0x41)
	ROM_FILL(0x0092, 1, 0xca)
	ROM_FILL(0x0093, 1, 0x41)
	ROM_FILL(0x0094, 1, 0xca)
	ROM_FILL(0x0095, 1, 0x41)
	ROM_FILL(0x0096, 1, 0xca)
	ROM_FILL(0x0097, 1, 0x41)
	ROM_FILL(0x0098, 1, 0xca)
	ROM_FILL(0x0099, 1, 0x41)
	ROM_FILL(0x009a, 1, 0xca)
	ROM_FILL(0x009b, 1, 0x41)

	ROM_REGION(0x2000, "panel", 0)
	ROM_LOAD("akai mpc60 panel eprom op v1-1 2764.ic2", 0x0000, 0x2000, CRC(f1332f47) SHA1(dd5e917d16941fce3db4bfe21d37f722d6262561))
ROM_END

} // anonymous namespace

SYST(1987, mpc60, 0, 0, mpc60, mpc60, mpc60_state, empty_init, "Akai Electric", "MPC60 MIDI Production Center", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
