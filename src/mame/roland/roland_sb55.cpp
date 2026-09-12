// license:BSD-3-Clause
// copyright-holders:superctr
/*************************************************************************************************

    Roland Sound Brush SB-55

    A MIDI file player from 1991, sold beside the SC-55: a 3.5" floppy drive, a three digit
    LED display and sixteen transport buttons, with no tone generator of its own.  Disks are
    MS-DOS format and hold Standard MIDI Files.  The infrared remote control is the SC-55's
    card remote, whose SONG, TEMPO and transport keys the Sound Brush answers.


    Main board (from the service notes, Apr. 1991, PCB 22925988 1/4):

    X1 16.000 MHz crystal (CPU), X2 455 kHz (remote decoder)
    IC2  uPD70320GJ-8-589  NEC V25, ROMless
    IC10 27C512            64K program EPROM
    IC9  HM65256BFP        32K SRAM, lithium battery backed
    IC8  BR2816A           2K x 8 parallel EEPROM
    IC18 HD63266F          floppy disk controller, uPD765 command set
    IC5  M54513P           LED segment driver, from port 0
    IC6  M54581P           LED common and switch row driver, from P14-P17
    IC7  74HC541           switch matrix read buffer onto the data bus
    IC4  BU3910F           remote control decoder, the same part as the SC-55's IC31
    IC3  PC410             photocoupler, MIDI IN 2 on the front
    IC21 M51953AL          reset

    The V25's A19 and A18 pick the chip, so every device is mirrored across its quarter of
    the 1 MB space: RAM at 00000, the EEPROM at 80000 and the EPROM at C0000, which is where
    the reset vector at FFFF0 sends the CPU.  A14 and A15 split the I/O space, which holds
    only the floppy controller at 0000 and the switch buffer at 4000.

    The panel is one matrix scanned from the CPU's ports.  P14-P17 drive four common lines
    through IC6, one low at a time; port 0 drives eight segment lines through IC5.  Commons
    0 to 2 carry the three digits of the display and common 3 the seven indicator lamps and,
    on the eighth segment, the STANDBY lamp of its own little board.  IC5 drives the display
    in its own order, a to g and then the point, from bit 7 down.

    The display is mounted upside down: its points stand above the digits rather than below,
    and its commons run from the right of the panel.  The firmware's font at C000:FBB0 is
    drawn the other way up to suit, and the layout turns the three digits over again.

    The same four commons drive the switch matrix, whose five returns come back through IC7,
    and the firmware numbers a key common * 5 + return.  The commons are the panel's columns
    taken right to left and the returns its rows taken bottom to top, so that the four keys
    of the bottom row are 0, 5, 10 and 15 and the top row's are 3, 8, 13 and 18.  Return 4
    is not part of the panel: it carries the drive's DISK CHANGE line on common 0, the
    footswitch jack on common 1 and the STANDBY switch on common 3.  IC11A gates DISK CHANGE
    with common 0 and pulls the return low through D3, so the machine reads the drive's own
    latch as a key, and that is how it notices a disk taken out or put in while it is in
    standby.  The footswitch is the one return the firmware inverts for itself, so an open
    jack reads low.

    The machine holds itself in standby at common 3 return 4, with the display blank and
    only the STANDBY lamp lit, and the same key takes it out again, as does the remote's
    POWER key.  From standby, holding SET, PROG and STOP and then pressing PLAY enters the
    service mode, which shows "tSt".

    The firmware will not take a combination of keys while anything else at all is down, and
    the one thing it excuses is the footswitch.

    MIDI IN 1 on the rear is the V25's serial channel 0, MIDI IN 2 on the front is channel 1,
    and MIDI OUT is channel 0's transmitter; MIDI THRU echoes IN 1 in hardware.  The floppy
    controller's interrupt is INTP0 and its DMA request drives DMARQ0, so sectors move through
    the CPU's own DMA channel 0 in one transfer mode.  P26 runs the drive motor, and the drive
    answers with READY, which is how the machine finds out whether it still holds a disk when
    it comes out of standby.  The controller's head load line is the drive's IN USE and, through
    Q1, the DISK lamp over the slot, so the lamp shows an access and not a disk.  The remote
    decoder puts a key number of 0 to 19 on port T and pulses INTP1.

    The machine reads its disk, walks the FAT and the directory, takes the title out of
    the Standard MIDI File and sends it to a Sound Canvas as a GS display sysex, and then
    plays the song out of MIDI OUT.  It takes only the timebases it was built for -- 24,
    48, 96, 192 and 384 ticks to the quarter note, and 60, 120, 240 and 480 -- and shows
    "noP" for any other.  A song number is shown on the two right hand digits, with the
    point of the middle one lit beside it.

    It records too, by the owner's manual's own combination: hold PAUSE and press REC, which
    takes the next free song number, and then PLAY.  STOP writes what came in at MIDI IN to
    the disk as a format 0 file at 96 ticks to the quarter note.

    TODO:
    - the service mode waits for a disk it then reads and writes, and what it wants on
      one is not known; the service notes' own test menu is not reached
    - P23 to P25 carry more drive lines the firmware drives directly and nothing here takes
    - MIDI THRU is not wired; MAME has no hardware echo of an input port

*************************************************************************************************/

#include "emu.h"

#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "cpu/nec/v25.h"
#include "imagedev/floppy.h"
#include "machine/nvram.h"
#include "machine/upd765.h"

#include "roland_sb55.lh"


namespace {

class sb55_state : public driver_device
{
public:
	sb55_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:0")
		, m_keys(*this, "KEY%u", 0U)
		, m_eeprom(*this, "eeprom", 0x800, ENDIANNESS_LITTLE)
		, m_digit(*this, "digit%u", 0U)
		, m_led(*this, "led%u", 0U)
		, m_led_disk(*this, "led_disk")
	{
	}

	void sb55(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(remote_key);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	u8 eeprom_r(offs_t offset) { return m_eeprom[offset]; }
	void eeprom_w(offs_t offset, u8 data) { m_eeprom[offset] = data; }

	void floppy_changed(floppy_image_device *floppy, bool loaded);
	void drive_w(u8 data);
	void update_ready();

	void head_load_w(int state);
	void fdc_irq_w(int state);
	TIMER_CALLBACK_MEMBER(head_unload) { m_head_load = false; m_led_disk = 0; }

	u8 switches_r();
	void segments_w(u8 data);
	void scan_w(u8 data);
	void update_display();

	required_device<v25_device> m_maincpu;
	required_device<hd63266f_device> m_fdc;
	required_device<floppy_connector> m_floppy;
	required_ioport_array<4> m_keys;
	memory_share_creator<u8> m_eeprom;
	output_finder<3> m_digit;
	output_finder<8> m_led;
	output_finder<> m_led_disk;

	emu_timer *m_unload_timer = nullptr;

	u8 m_scan = 0xff;
	u8 m_segments = 0;
	u8 m_remote_key = 0xff;
	bool m_disk = false;
	bool m_motor = false;
	bool m_head_load = false;
};


void sb55_state::machine_start()
{
	floppy_image_device *const floppy = m_floppy->get_device();
	m_fdc->set_floppy(floppy);
	m_fdc->set_rate(250000);
	if (floppy != nullptr)
	{
		floppy->setup_load_cb(floppy_image_device::load_cb(
				[this] (floppy_image_device *floppy) { floppy_changed(floppy, true); }));
		floppy->setup_unload_cb(floppy_image_device::unload_cb(
				[this] (floppy_image_device *floppy) { floppy_changed(floppy, false); }));
		m_disk = floppy->exists();
	}
	m_unload_timer = timer_alloc(FUNC(sb55_state::head_unload), this);
	update_ready();

	save_item(NAME(m_scan));
	save_item(NAME(m_segments));
	save_item(NAME(m_remote_key));
	save_item(NAME(m_disk));
	save_item(NAME(m_motor));
	save_item(NAME(m_head_load));
}

void sb55_state::machine_reset()
{
	m_scan = 0xff;
	m_segments = 0;
	m_remote_key = 0xff;
	m_motor = false;
	m_unload_timer->reset();
	m_head_load = false;
	m_led_disk = 0;
	update_ready();
}


// MAME's own ready comes up two index pulses after the motor, which is later than the
// firmware looks, so the controller takes it as an external line driven from here instead
void sb55_state::update_ready()
{
	m_fdc->ready_w(!(m_disk && m_motor));
}

// the image is still counted as present while its unload callback runs, so the drive says
// here which way it went
void sb55_state::floppy_changed(floppy_image_device *floppy, bool loaded)
{
	m_disk = loaded;
	update_ready();
}

// P26 runs the drive's motor
void sb55_state::drive_w(u8 data)
{
	if (m_motor == !BIT(data, 6))
		return;
	m_motor = !BIT(data, 6);
	update_ready();
}

// the controller loads the head for a transfer and drops it again a head unload time after
// the command ends, and the lamp goes out with it.  MAME's controller only ever loads the
// head, so the interrupt that ends the command starts that time here; the firmware leaves
// the unload time at the longest the chip has.
void sb55_state::head_load_w(int state)
{
	if (!state)
		return;
	m_unload_timer->reset();
	m_head_load = true;
	m_led_disk = 1;
}

void sb55_state::fdc_irq_w(int state)
{
	if (state && m_head_load)
		m_unload_timer->adjust(attotime::from_msec(256));
}


//-------------------------------------------------
//  the panel matrix
//-------------------------------------------------

// one common line is held low at a time; rows 0 to 2 are the digits, row 3 the indicators.
// The segments come off the bus the other way round from the order a seven segment element
// wants them, and the layout has the digits upside down as the panel does.
void sb55_state::update_display()
{
	for (int row = 0; row < 4; row++)
	{
		if (BIT(m_scan, 4 + row))
			continue;

		if (row < 3)
			m_digit[row] = bitswap<8>(m_segments, 0, 1, 2, 3, 4, 5, 6, 7);
		else
			for (int bit = 0; bit < 8; bit++)
				m_led[bit] = BIT(m_segments, bit);
	}
}

void sb55_state::segments_w(u8 data)
{
	m_segments = data;
	update_display();
}

void sb55_state::scan_w(u8 data)
{
	m_scan = data;
	update_display();
}

// IC7 puts the five switch returns on the low bits of the bus; the three it leaves open
// read back high through the pull-ups.  The drive's DISK CHANGE shares the fifth return
// with the footswitch and the standby switch, and holds it low from the moment a disk is
// taken out until the drive steps with one in again.
u8 sb55_state::switches_r()
{
	floppy_image_device *const floppy = m_floppy->get_device();
	u8 data = 0xff;
	for (int row = 0; row < 4; row++)
	{
		if (BIT(m_scan, 4 + row))
			continue;
		u8 keys = m_keys[row]->read();
		if (row == 0 && (floppy == nullptr || !floppy->dskchg_r()))
			keys &= ~0x10;
		data &= keys;
	}
	return data;
}


//-------------------------------------------------
//  the remote control decoder
//-------------------------------------------------

// IC4 decodes a key number onto port T and pulses INTP1; the firmware reads port T in the
// handler, so the number has to stand while the line is taken
INPUT_CHANGED_MEMBER(sb55_state::remote_key)
{
	if (!newval)
		return;

	m_remote_key = param;
	m_maincpu->set_input_line(NEC_INPUT_LINE_INTP1, ASSERT_LINE);
	m_maincpu->set_input_line(NEC_INPUT_LINE_INTP1, CLEAR_LINE);
}


//-------------------------------------------------
//  address maps
//-------------------------------------------------

void sb55_state::mem_map(address_map &map)
{
	map(0x00000, 0x07fff).mirror(0x38000).ram().share("nvram");
	map(0x80000, 0x807ff).mirror(0x3f800).rw(FUNC(sb55_state::eeprom_r), FUNC(sb55_state::eeprom_w));
	map(0xc0000, 0xcffff).mirror(0x30000).rom().region("progrom", 0);
}

void sb55_state::io_map(address_map &map)
{
	map(0x0000, 0x0002).mirror(0x3ffc).m(m_fdc, FUNC(hd63266f_device::map));
	map(0x4000, 0x4000).mirror(0x3fff).r(FUNC(sb55_state::switches_r));
}


//-------------------------------------------------
//  machine configuration
//-------------------------------------------------

static INPUT_PORTS_START( sb55 )
	// The four scan lines are the panel's columns, taken right to left, and the five returns
	// its rows, taken bottom to top; the firmware numbers a key scan * 5 + return.  The fifth
	// return carries the disk sensor, the footswitch jack and the standby switch.
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Fast Forward")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Repeat")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Clear")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Set")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED) // the drive's DISK CHANGE, driven below
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Rewind")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Single")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Random")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Program")
	// the firmware inverts this return, and only this one, so an open jack reads low
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Start/Stop Footswitch")
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Play")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Record")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Tempo >")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Song >")
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Stop")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Pause")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Tempo <")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Song <")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_NAME("Power")
	PORT_BIT(0xe0, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("REMOTE")
	PORT_BIT(0x000001, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Remote All")     PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(sb55_state::remote_key), 0)
	PORT_BIT(0x000002, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Remote Mute")    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(sb55_state::remote_key), 1)
	PORT_BIT(0x000004, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Remote Power")   PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(sb55_state::remote_key), 3)
	PORT_BIT(0x000008, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Remote Part <")  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(sb55_state::remote_key), 4)
	PORT_BIT(0x000010, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Remote Part >")  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(sb55_state::remote_key), 5)
	PORT_BIT(0x000020, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Remote Inst <")  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(sb55_state::remote_key), 6)
	PORT_BIT(0x000040, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Remote Inst >")  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(sb55_state::remote_key), 7)
	PORT_BIT(0x000080, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Remote Level <") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(sb55_state::remote_key), 8)
	PORT_BIT(0x000100, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Remote Level >") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(sb55_state::remote_key), 9)
	PORT_BIT(0x000200, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Remote Reverb <") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(sb55_state::remote_key), 10)
	PORT_BIT(0x000400, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Remote Reverb >") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(sb55_state::remote_key), 11)
	PORT_BIT(0x000800, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Remote Song <")  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(sb55_state::remote_key), 12)
	PORT_BIT(0x001000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Remote Song >")  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(sb55_state::remote_key), 13)
	PORT_BIT(0x002000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Remote Tempo <") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(sb55_state::remote_key), 14)
	PORT_BIT(0x004000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Remote Tempo >") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(sb55_state::remote_key), 15)
	PORT_BIT(0x008000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Remote Stop")    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(sb55_state::remote_key), 16)
	PORT_BIT(0x010000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Remote Play")    PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(sb55_state::remote_key), 17)
	PORT_BIT(0x020000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Remote Rewind")  PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(sb55_state::remote_key), 18)
	PORT_BIT(0x040000, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Remote Fast Forward") PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(sb55_state::remote_key), 19)
INPUT_PORTS_END

static void sb55_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
}

void sb55_state::sb55(machine_config &config)
{
	V25(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &sb55_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &sb55_state::io_map);
	m_maincpu->p0_out_cb().set(FUNC(sb55_state::segments_w));
	m_maincpu->p1_out_cb().set(FUNC(sb55_state::scan_w));
	m_maincpu->p2_out_cb().set(FUNC(sb55_state::drive_w));
	m_maincpu->pt_in_cb().set([this] () { return m_remote_key; });
	m_maincpu->dma0_read_cb().set(m_fdc, FUNC(hd63266f_device::dma_r));
	m_maincpu->dma0_write_cb().set(m_fdc, FUNC(hd63266f_device::dma_w));
	m_maincpu->tc_handler<0>().set(m_fdc, FUNC(hd63266f_device::tc_line_w));
	m_maincpu->txd_handler<0>().set("mdout", FUNC(midi_port_device::write_txd));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // HM65256 (IC9) + lithium battery
	// BR2816A (IC8).  A blank part reads as FF and the firmware then rejects its own
	// settings and stops at an error; zeroed, it writes its defaults over them instead.
	NVRAM(config, "eeprom", nvram_device::DEFAULT_ALL_0);

	HD63266F(config, m_fdc, 16_MHz_XTAL);
	m_fdc->intrq_wr_callback().set_inputline(m_maincpu, NEC_INPUT_LINE_INTP0);
	m_fdc->intrq_wr_callback().append(FUNC(sb55_state::fdc_irq_w));
	m_fdc->drq_wr_callback().set(m_maincpu, FUNC(v25_device::dmarq_w<0>));
	m_fdc->hdl_wr_callback().set(FUNC(sb55_state::head_load_w));
	m_fdc->set_ready_line_connected(false);
	m_fdc->set_select_lines_connected(false);

	FLOPPY_CONNECTOR(config, m_floppy, sb55_floppies, "35dd", floppy_image_device::default_pc_floppy_formats);

	midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	mdin.rxd_handler().set(m_maincpu, FUNC(v25_device::rxd_w<0>));
	midi_port_device &mdin2(MIDI_PORT(config, "mdin2", midiin_slot, "midiin"));
	mdin2.rxd_handler().set(m_maincpu, FUNC(v25_device::rxd_w<1>));
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	config.set_default_layout(layout_roland_sb55);
}

ROM_START( sb55 )
	ROM_REGION( 0x10000, "progrom", 0 )
	ROM_LOAD( "roland_sb-55_v1.03.ic10", 0x0000, 0x10000, CRC(c1798080) SHA1(5184c61132798e86b140c36329130fd14f3024ea) )
ROM_END

} // anonymous namespace


SYST( 1991, sb55, 0, 0, sb55, sb55, sb55_state, empty_init, "Roland", "Sound Brush SB-55", MACHINE_NO_SOUND_HW )
