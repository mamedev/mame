// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    mpc3000.cpp - Akai / Roger Linn MPC-3000 music workstation
    Driver by R. Belmont

    Hardware:
        CPU: NEC V53 (33 MHz?)
             8086-compatible CPU
             8237-compatible DMA controller
             8254-compatible timer
             8259-compatible IRQ controller
        Floppy: uPD72069 (3000), uPD72068 (2000 & 2000XL)
        SCSI: MB89352
        LCD: LC7981
        Quad-UART: TE7774 (3000), 2x MB89371A (2000 & 2000XL)
        Panel controller CPU: NEC uPD78C10AGQ @ 12 MHz
        Sound DSP: L7A1045-L6048
            DSP's wavedata bus is 16 bits wide and has 24 address bits (32 MiB total sample space)

        DMA channel 0 is SCSI, 1 is floppy, 2 is IC31 (some sort of direct-audio stream?), and 3 is the L7A1045 DSP
        IRQ 3 is wire-OR of the 72069 FDC and 89352 SCSI
        IRQ 4 is wire-OR of all 4 TXRDYs on the TE7774
        IRQ 5 is wire-OR of RXRDY1, 2, and 3 on the TE7774
        IRQ 6 is the SMPTE sync in
        IRQ 7 is RXRDY4 on the TE7774

        TE7774 hookups: RXD1 is MIDI IN 1, RXD2 is MIDI IN 2, RXD3 and 4 are wire-ORed to the uPD7810's TX line.
                        TXD1-4 are MIDI OUTs 1, 2, 3, and 4.

    MPC2000 and 2000XL: (MPC2000 is often called "MPC2000 Classic" but apparently not by Akai)
        CPU: NEC V53
        Floppy: uPD72068
        SCSI: MB89352
        LCD:
        Dual UART: MB89371A
        (V53's 8251 is used for panel comms here)
        Panel controller CPU: NEC uPD7810 @ 12 MHz
        Sound DSP: L6048

MPCs on other hardware:

    MPC4000:
        CPU, LCD, panel controller: SA1110 StrongARM @ 176 MHz
        SCSI: FAS236 (same as NCR 53CF96)
        IDE, UART, DMA controller: FPGA XC2S100-5TQ144C
        USB Host: SL811HST
        USB function controller: NET2890
        DSP, MIDI: MB87L185PFVS-G-BND (gate array?)

    MPC1000:
        CPU, LCD, UART, panel controller, DSP: SH-3 7712 (HD6417712)

    MPC500:
        CPU, LCD, UART, panel controller, DSP: SH-3 7727 (HD6417727) @ 100 MHz

    MPC2500:
        CPU, LCD, UART, panel controller, DSP: SH-3 7727 (HD6417727) @ 160 MHz

    --------------------------------------------------------------------------
    MPC3000 key & LED matrices

    PB0-PB3 drum pad row select
    PB4-PB7 drum pad column sense input
    AN0-AN3 drum pad column velocity input
    PC1-PC3 row select output
    PC4-PC5 encoder knob quadrature input
    PA0-PA7 column sense input

    main buttons:
    Y0 row: PAD BANK    FULL LEVEL     keypad 7    keypad 8  keypad 9   DISK        PROGRAM/SOUNDS MIXER/EFFECTS
    Y1 row: 16 LEVELS   ASSIGN         keypad 4    keypad 5  keypad 6   MIDI        SONG           OTHER
    Y2 row: AFTER                      keypad 1    keypad 2  keypad 3   SEQ EDIT    STEP EDIT      EDIT LOOP
    Y3 row: SOFT KEY 1  SOFT KEY 3     keypad 0    keypad .  ENTER      TEMPO/SYNC  TRANSPOSE      SIMUL SEQ
    Y4 row: SOFT KEY 2  SOFT KEY 4                 <         <<         AUTO PUNCH  COUNT IN       WAIT FOR
    Y5 row:                            keypad -    keypad +  <- (LEFT)  ^ (UP)      \/ (DOWN)      > (RIGHT)
    Y6 row:                                        ERASE     TIMING C   TAP TEMPO   MAIN SCREEN    HELP
    Y7 row: STOP        PLAY           PLAY START  REC       OVER DUB   LOCATE      >              >>

    LEDs:
    LOLED x A1-A4 = EDIT LOOP, SIMUL SEQ, TRANSPOSE, WAIT FOR, COUNT IN, AUTO PUNCH, REC, OVER DUB
    HILED x A1-A4 = PLAY, BANK A, BANK B, BANK C, BANK D, FULL LEVEL, 16 LEVELS, AFTER

***************************************************************************/

#include "emu.h"
#include "cpu/nec/v5x.h"
#include "cpu/upd7810/upd7810.h"
#include "imagedev/floppy.h"
#include "sound/l7a1045_l6028_dsp_a.h"
#include "video/hd61830.h"
#include "bus/midi/midi.h"
#include "bus/nscsi/devices.h"
#include "formats/dfi_dsk.h"
#include "formats/hxchfe_dsk.h"
#include "formats/hxcmfm_dsk.h"
#include "formats/imd_dsk.h"
#include "formats/mfi_dsk.h"
#include "formats/td0_dsk.h"
#include "formats/dsk_dsk.h"
#include "formats/pc_dsk.h"
#include "formats/ipf_dsk.h"
#include "speaker.h"
#include "screen.h"
#include "emupal.h"
#include "machine/74259.h"
#include "machine/i8255.h"
#include "machine/input_merger.h"
#include "machine/mb87030.h"
#include "machine/nvram.h"
#include "machine/pit8253.h"
#include "machine/te7774.h"
#include "machine/upd765.h"

class mpc3000_state : public driver_device
{
public:
	mpc3000_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_subcpu(*this, "subcpu")
		, m_lcdc(*this, "lcdc")
		, m_dsp(*this, "dsp")
		, m_mdout(*this, "mdout")
		, m_fdc(*this, "fdc")
		, m_floppy(*this, "fdc:0")
		, m_sio(*this, "sio")
		, m_keys(*this, "Y%u", 0)
	{ }

	void mpc3000(machine_config &config);

	void init_mpc3000();

private:
	required_device<v53a_device> m_maincpu;
	required_device<upd7810_device> m_subcpu;
	required_device<hd61830_device> m_lcdc;
	required_device<l7a1045_sound_device> m_dsp;
	required_device<midi_port_device> m_mdout;
	required_device<upd72069_device> m_fdc;
	required_device<floppy_connector> m_floppy;
	required_device<te7774_device> m_sio;
	required_ioport_array<8> m_keys;

	static void floppies(device_slot_interface &device);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void mpc3000_map(address_map &map) ATTR_COLD;
	void mpc3000_io_map(address_map &map) ATTR_COLD;
	void mpc3000_sub_map(address_map &map) ATTR_COLD;

	uint8_t dma_memr_cb(offs_t offset);
	void dma_memw_cb(offs_t offset, uint8_t data);
	uint16_t dma_mem16r_cb(offs_t offset);
	void dma_mem16w_cb(offs_t offset, uint16_t data);
	void mpc3000_palette(palette_device &palette) const;

	uint8_t fdc_hc365_r();

	uint8_t subcpu_pa_r();
	uint8_t subcpu_pb_r();
	void subcpu_pb_w(uint8_t data);
	void subcpu_pc_w(uint8_t data);

	uint8_t m_key_scan_row;
};

void mpc3000_state::machine_start()
{
}

void mpc3000_state::machine_reset()
{
}

void mpc3000_state::mpc3000_map(address_map &map)
{
	map(0x000000, 0x07ffff).mirror(0x80000).rom().region("maincpu", 0);
	map(0x300000, 0x3fffff).ram();  // 2x HM658512 (512Kx8)
	map(0x500000, 0x500fff).share("nvram");
}

void mpc3000_state::mpc3000_io_map(address_map &map)
{
	map(0x0000, 0x0000).w("loledlatch", FUNC(hc259_device::write_nibble_d3));
	map(0x0020, 0x0020).w("hiledlatch", FUNC(hc259_device::write_nibble_d3));
	map(0x0060, 0x006f).m(m_dsp, FUNC(l7a1045_sound_device::map));
	map(0x0080, 0x0087).rw("dioexp", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
	map(0x00a0, 0x00bf).m("scsi:7:spc", FUNC(mb89352_device::map)).umask16(0x00ff);
	map(0x00c0, 0x00c7).rw("sio", FUNC(te7774_device::read_cs<0>), FUNC(te7774_device::write_cs<0>)).umask16(0x00ff);
	map(0x00c8, 0x00cf).rw("sio", FUNC(te7774_device::read_cs<1>), FUNC(te7774_device::write_cs<1>)).umask16(0x00ff);
	map(0x00d0, 0x00d7).rw("sio", FUNC(te7774_device::read_cs<2>), FUNC(te7774_device::write_cs<2>)).umask16(0x00ff);
	map(0x00d8, 0x00df).rw("sio", FUNC(te7774_device::read_cs<3>), FUNC(te7774_device::write_cs<3>)).umask16(0x00ff);
	map(0x00e0, 0x00e0).rw(m_lcdc, FUNC(hd61830_device::data_r), FUNC(hd61830_device::data_w)).umask16(0x00ff);
	map(0x00e2, 0x00e2).rw(m_lcdc, FUNC(hd61830_device::status_r), FUNC(hd61830_device::control_w)).umask16(0x00ff);
	map(0x00e8, 0x00eb).m(m_fdc, FUNC(upd72069_device::map)).umask16(0x00ff);
	map(0x00e8, 0x00eb).r(FUNC(mpc3000_state::fdc_hc365_r)).umask16(0xff00);
	map(0x00f0, 0x00f7).rw("synctmr", FUNC(pit8254_device::read), FUNC(pit8254_device::write)).umask16(0x00ff);
	map(0x00f8, 0x00ff).rw("adcexp", FUNC(i8255_device::read), FUNC(i8255_device::write)).umask16(0x00ff);
}

// bit 0 = ED   (no idea, disk eject?)
// bit 1 = /EDD (enhanced density if 0, DD/HD if 1?)
// bit 2 = /HDD (high density if 0, double density if 1)
// bits 3 & 4 = footswitches
uint8_t mpc3000_state::fdc_hc365_r()
{
	uint8_t rv = m_floppy->get_device()->floppy_is_hd() ? 0x00 : 0x04;
	return rv;
}

uint16_t mpc3000_state::dma_mem16r_cb(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_word(offset << 1);
}

void mpc3000_state::dma_mem16w_cb(offs_t offset, uint16_t data)
{
	m_maincpu->space(AS_PROGRAM).write_word(offset << 1, data);
}

uint8_t mpc3000_state::dma_memr_cb(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

void mpc3000_state::dma_memw_cb(offs_t offset, uint8_t data)
{
	m_maincpu->space(AS_PROGRAM).write_byte(offset, data);
}

void mpc3000_state::mpc3000_sub_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("subcpu", 0);
}

void mpc3000_state::mpc3000_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(138, 146, 148));
	palette.set_pen_color(1, rgb_t(92, 83, 88));
}

uint8_t mpc3000_state::subcpu_pa_r()
{
	return m_keys[7 - m_key_scan_row]->read();
}

uint8_t mpc3000_state::subcpu_pb_r()
{
	return 0;
}

// drum pad row select, active low
void mpc3000_state::subcpu_pb_w(uint8_t data)
{
}

// main buttons row select (PC1-PC3)
void mpc3000_state::subcpu_pc_w(uint8_t data)
{
	m_key_scan_row = ((data ^ 0xff) >> 1) & 7;
}

void mpc3000_state::floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
	device.option_add("35hd", FLOPPY_35_HD);
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

void mpc3000_state::mpc3000(machine_config &config)
{
	V53A(config, m_maincpu, 32_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mpc3000_state::mpc3000_map);
	m_maincpu->set_addrmap(AS_IO, &mpc3000_state::mpc3000_io_map);
	m_maincpu->out_hreq_cb().set(m_maincpu, FUNC(v53a_device::hack_w));
	m_maincpu->in_memr_cb().set(FUNC(mpc3000_state::dma_memr_cb));
	m_maincpu->out_memw_cb().set(FUNC(mpc3000_state::dma_memw_cb));
	m_maincpu->in_mem16r_cb().set(FUNC(mpc3000_state::dma_mem16r_cb));
	m_maincpu->out_mem16w_cb().set(FUNC(mpc3000_state::dma_mem16w_cb));
	m_maincpu->out_eop_cb().set("tc", FUNC(input_merger_device::in_w<0>));
	m_maincpu->in_ior_cb<0>().set("scsi:7:spc", FUNC(mb89352_device::dma_r));
	m_maincpu->out_iow_cb<0>().set("scsi:7:spc", FUNC(mb89352_device::dma_w));
	m_maincpu->in_ior_cb<1>().set(m_fdc, FUNC(upd72069_device::dma_r));
	m_maincpu->out_iow_cb<1>().set(m_fdc, FUNC(upd72069_device::dma_w));
	m_maincpu->in_io16r_cb<3>().set(m_dsp, FUNC(l7a1045_sound_device::dma_r16_cb));
	m_maincpu->out_io16w_cb<3>().set(m_dsp, FUNC(l7a1045_sound_device::dma_w16_cb));
	m_maincpu->set_tclk(4'000'000); // FIXME: DAWCK generated by DSP (also tied to V53 DSR input)
	m_maincpu->tout_handler<0>().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_maincpu->tout_handler<1>().set_inputline(m_maincpu, INPUT_LINE_IRQ1);
	m_maincpu->tout_handler<2>().set_inputline(m_maincpu, INPUT_LINE_IRQ2);

	constexpr XTAL V53_CLKOUT = 32_MHz_XTAL / 2;
	constexpr XTAL V53_PCLKOUT = 32_MHz_XTAL / 4;

	// HC02 gates
	INPUT_MERGER_ANY_HIGH(config, "intp3").output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ3);
	INPUT_MERGER_ALL_HIGH(config, "tc").output_handler().set(m_fdc, FUNC(upd72069_device::tc_line_w));

	hc259_device &loledlatch(HC259(config, "loledlatch"));
	loledlatch.q_out_cb<0>().set_output("led0").invert(); // Edit Loop
	loledlatch.q_out_cb<1>().set_output("led1").invert(); // Simul Seq
	loledlatch.q_out_cb<2>().set_output("led2").invert(); // Transpose
	loledlatch.q_out_cb<3>().set_output("led3").invert(); // Wait For
	loledlatch.q_out_cb<4>().set_output("led4").invert(); // Count In
	loledlatch.q_out_cb<5>().set_output("led5").invert(); // Auto Punch
	loledlatch.q_out_cb<6>().set_output("led6").invert(); // Rec
	loledlatch.q_out_cb<7>().set_output("led7").invert(); // Over Dub

	hc259_device &hiledlatch(HC259(config, "hiledlatch"));
	hiledlatch.q_out_cb<0>().set_output("led8").invert(); // Play
	hiledlatch.q_out_cb<1>().set_output("led9").invert(); // Bank A
	hiledlatch.q_out_cb<2>().set_output("led10").invert(); // Bank B
	hiledlatch.q_out_cb<3>().set_output("led11").invert(); // Bank C
	hiledlatch.q_out_cb<4>().set_output("led12").invert(); // Bank D
	hiledlatch.q_out_cb<5>().set_output("led13").invert(); // Full Level
	hiledlatch.q_out_cb<6>().set_output("led14").invert(); // 16 Levels
	hiledlatch.q_out_cb<7>().set_output("led15").invert(); // After

	UPD78C10(config, m_subcpu, 12_MHz_XTAL);
	m_subcpu->set_addrmap(AS_PROGRAM, &mpc3000_state::mpc3000_sub_map);
	m_subcpu->txd_func().set(m_sio, FUNC(te7774_device::rx_w<2>));      // 7810 TxD is wire-ORed to channels 2 & 3 RxD
	m_subcpu->txd_func().append(m_sio, FUNC(te7774_device::rx_w<3>));
	m_subcpu->pa_in_cb().set(FUNC(mpc3000_state::subcpu_pa_r));
	m_subcpu->pb_in_cb().set(FUNC(mpc3000_state::subcpu_pb_r));
	m_subcpu->pb_out_cb().set(FUNC(mpc3000_state::subcpu_pb_w));
	m_subcpu->pc_out_cb().set(FUNC(mpc3000_state::subcpu_pc_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(80);
	screen.set_screen_update("lcdc", FUNC(hd61830_device::screen_update));
	screen.set_size(240, 64);   //6x20, 8x8
	screen.set_visarea(0, 240-1, 0, 64-1);
	screen.set_palette("palette");

	PALETTE(config, "palette", FUNC(mpc3000_state::mpc3000_palette), 2);

	NVRAM(config, "nvram");     // LC3517 2048x8 SRAM with battery backup

	UPD72069(config, m_fdc, V53_CLKOUT); // TODO: upd72069 supports motor control
	m_fdc->intrq_wr_callback().set("intp3", FUNC(input_merger_device::in_w<0>));
	m_fdc->drq_wr_callback().set(m_maincpu, FUNC(v53a_device::dreq_w<1>));

	FLOPPY_CONNECTOR(config, m_floppy, mpc3000_state::floppies, "35hd", add_formats).enable_sound(true);

	pit8254_device &pit(PIT8254(config, "synctmr", 0)); // MB89254
	pit.set_clk<0>(V53_PCLKOUT);
	pit.set_clk<1>(V53_PCLKOUT);
	pit.set_clk<2>(V53_PCLKOUT);

	I8255(config, "adcexp"); // MB89255B
	I8255(config, "dioexp"); // MB89255B

	HD61830(config, m_lcdc, 4.9152_MHz_XTAL / 2 / 2); // LC7981

	// HC4072 gates
	INPUT_MERGER_ANY_HIGH(config, "intp4").output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ4);
	INPUT_MERGER_ANY_HIGH(config, "intp5").output_handler().set_inputline(m_maincpu, INPUT_LINE_IRQ5);

	TE7774(config, m_sio, V53_PCLKOUT);
	m_sio->txd_handler<0>().set(m_mdout, FUNC(midi_port_device::write_txd));
	m_sio->rxrdy_handler<0>().set("intp5", FUNC(input_merger_device::in_w<0>));
	m_sio->rxrdy_handler<1>().set("intp5", FUNC(input_merger_device::in_w<1>));
	m_sio->rxrdy_handler<2>().set("intp5", FUNC(input_merger_device::in_w<2>));
	m_sio->rxrdy_handler<3>().set_inputline(m_maincpu, INPUT_LINE_IRQ7);
	m_sio->txrdy_handler<0>().set("intp4", FUNC(input_merger_device::in_w<0>));
	m_sio->txrdy_handler<1>().set("intp4", FUNC(input_merger_device::in_w<1>));
	m_sio->txrdy_handler<2>().set("intp4", FUNC(input_merger_device::in_w<2>));
	m_sio->txrdy_handler<3>().set("intp4", FUNC(input_merger_device::in_w<3>));

	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	mdin.rxd_handler().set(m_sio, FUNC(te7774_device::rx_w<0>));

	midiout_slot(MIDI_PORT(config, "mdout"));

	NSCSI_BUS(config, "scsi");
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, nullptr);
	NSCSI_CONNECTOR(config, "scsi:7").option_set("spc", MB89352).machine_config(
		[this](device_t *device)
		{
			mb89352_device &spc = downcast<mb89352_device &>(*device);

			spc.set_clock(32_MHz_XTAL / 4); // PCLKOUT
			spc.out_irq_callback().set(":intp3", FUNC(input_merger_device::in_w<1>));
			spc.out_dreq_callback().set(m_maincpu, FUNC(v53a_device::dreq_w<0>));
		});

	SPEAKER(config, "speaker", 2).front();

	L7A1045(config, m_dsp, 33.8688_MHz_XTAL); // clock verified by schematic
	m_dsp->drq_handler_cb().set(m_maincpu, FUNC(v53a_device::dreq_w<3>));
	m_dsp->add_route(0, "speaker", 1.0, 0);
	m_dsp->add_route(1, "speaker", 1.0, 1);
}

static INPUT_PORTS_START( mpc3000 )
	PORT_START("Y0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Pad Bank") PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Full Level") PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Disk") PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Program/Sounds") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Mixer/Effects") PORT_CODE(KEYCODE_E)

	PORT_START("Y1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("16 Levels") PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Assign") PORT_CODE(KEYCODE_LSHIFT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("MIDI") PORT_CODE(KEYCODE_A)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Song") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Other") PORT_CODE(KEYCODE_D)

	PORT_START("Y2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("After") PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Seq Edit") PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Step Edit") PORT_CODE(KEYCODE_X)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Edit Loop") PORT_CODE(KEYCODE_C)

	PORT_START("Y3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Soft Key 1") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Soft Key 3") PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP) PORT_NAME(".")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tempo/Sync") PORT_CODE(KEYCODE_R)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Transpose") PORT_CODE(KEYCODE_T)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Simul Seq") PORT_CODE(KEYCODE_Y)

	PORT_START("Y4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Soft Key 2") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Soft Key 4") PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5) PORT_NAME("<")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6) PORT_NAME("<<")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Auto Punch") PORT_CODE(KEYCODE_F)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Count In") PORT_CODE(KEYCODE_G)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Wait For") PORT_CODE(KEYCODE_H)

	PORT_START("Y5")
	PORT_BIT(0x03, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("-")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("+")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("Left Arrow")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP) PORT_NAME("Up Arrow")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("Down Arrow")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("Right Arrow")

	PORT_START("Y6")
	PORT_BIT(0x07, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U) PORT_NAME("Erase")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Timing Correct") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Tap Tempo") PORT_CODE(KEYCODE_O)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Main Screen") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Help") PORT_CODE(KEYCODE_J)

	PORT_START("Y7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Stop") PORT_CODE(KEYCODE_K)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Play") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Play Start") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Rec") PORT_CODE(KEYCODE_B)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Over Dub") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Locate") PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(">") PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME(">>") PORT_CODE(KEYCODE_F9)
INPUT_PORTS_END

ROM_START( mpc3000 )
	ROM_REGION(0x80000, "maincpu", 0)   // V53 code
	ROM_SYSTEM_BIOS(0, "default", "ver 3.12")
	ROMX_LOAD( "mpc312ls.bin", 0x000000, 0x040000, CRC(d4fb6439) SHA1(555d388ed25f8b85638c325e7d9012eaa271ffa0), ROM_SKIP(1) | ROM_BIOS(0) )
	ROMX_LOAD( "mpc312ms.bin", 0x000001, 0x040000, CRC(80ee0ab9) SHA1(b8855118d59b8f73a3af5ff2e824cdaa0a9f564a), ROM_SKIP(1) | ROM_BIOS(0) )

	ROM_SYSTEM_BIOS(1, "vailixi", "ver 3.50")
	ROMX_LOAD( "3.50 vailixi lse.bin", 0x000000, 0x040000, CRC(9f3031b5) SHA1(c270a92f8ed273a1ede16388bb8f30c85ac1faab), ROM_SKIP(1) | ROM_BIOS(1) )
	ROMX_LOAD( "3.50 vailixi mso.bin", 0x000001, 0x040000, CRC(e62ebb26) SHA1(f6d080481de40bea2c94bbc96b222c5f9b7afaf4), ROM_SKIP(1) | ROM_BIOS(1) )

	ROM_SYSTEM_BIOS(2, "v311", "ver 3.11")
	ROMX_LOAD( "mpc311ls.bin", 0x000000, 0x040000, CRC(5a272061) SHA1(87cc8aef3233d95ad1febbd77b42f720d718baa3), ROM_SKIP(1) | ROM_BIOS(2) )
	ROMX_LOAD( "mpc311ms.bin", 0x000001, 0x040000, CRC(a8e177de) SHA1(b63a5c27066c03f7de56659aff183f15d95277c5), ROM_SKIP(1) | ROM_BIOS(2) )

	ROM_SYSTEM_BIOS(3, "v310", "ver 3.10")
	ROMX_LOAD( "mpc3000__ls__v3.10.am27c020__id0197.ic13_ls.bin", 0x000000, 0x040000, CRC(cbd1b3a6) SHA1(5464a57137549d9d9c47f9aafc2b89f4c0af8b31), ROM_SKIP(1) | ROM_BIOS(3) )
	ROMX_LOAD( "mpc3000__ms__v3.10.am27c020__id0197.ic14_ms.bin", 0x000001, 0x040000, CRC(e2ba1904) SHA1(27a9f047c63964fac2b453f2317b77834490983d), ROM_SKIP(1) | ROM_BIOS(3) )

	ROM_REGION(0x8000, "subcpu", 0)    // uPD78C10 panel controller code
	ROM_LOAD( "mp3000__op_v1.0.am27c256__id0110.ic602.bin", 0x000000, 0x008000, CRC(b0b783d3) SHA1(a60016184fc07ba00dcc19ba4da60e78aceff63c) )

	ROM_REGION( 0x2000000, "dsp", ROMREGION_ERASE00 )   // sample RAM
ROM_END

void mpc3000_state::init_mpc3000()
{
}

CONS( 1994, mpc3000, 0, 0, mpc3000, mpc3000, mpc3000_state, init_mpc3000, "Akai / Roger Linn", "MPC-3000", MACHINE_NOT_WORKING )
