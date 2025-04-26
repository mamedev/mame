// license:BSD-3-Clause
// copyright-holders:Ville Linde
/*  Konami FireBeat

    Driver by Ville Linde



    Hardware overview:

    GQ972 PWB(A2) 0000070609 Main board
    -----------------------------------
        25.175 MHz clock (near GCUs and VGA connectors)
        16.6666 MHz clock (near GCUs and VGA connectors)
        OSC 66.0000MHz
        IBM PowerPC 403GCX at 66MHz
        (2x) Konami 0000057714 (2D object processor)
        Yamaha YMZ280B (ADPCM sound chip)
        Epson RTC65271 RTC/NVRAM
        National Semiconductor PC16552DV (dual UART)

    GQ974 PWB(A2) 0000070140 Extend board
    -------------------------------------
        ADC0808CCN
        FDC37C665GT (floppy disk controller)
        National Semiconductor PC16552DV (dual UART)
        ADM223LJR (RS-232 driver/receiver)

    GQ971 PWB(A2) 0000070254 SPU
    ----------------------------
        Motorola MC68HC000FN16 at 16MHz (?)
        Xilinx XC9572 CPLD
        Ricoh RF5c400 sound chip

    GQ971 PWB(B2) 0000067784 Backplane
    ----------------------------------
        3x PCB Slots with 2x DIN96S connectors (for main and extend PCBs)
        40-pin ATA connector for each slot

    GQ986 PWB(A1) 0000073015 Backplane
    ----------------------------------
        2x PCB Slots with 2x DIN96S connectors (for main and extend PCBs)
        40-pin ATA connector for each slot

    GQ972 PWB(D2) Controller interface on Beatmania III (?)
    GQ972 PWB(G1) Sound Amp (?)
    GQ971 PWB(C) Sound Amp


    Hardware configurations:

    Beatmania III
    -------------
        GQ971 Backplane
        GQ972 Main Board
        GQ974 Extend Board
        GQ971 SPU
        GQ972 Controller Interface
        GQ972 Sound Amp
        Hard drive in Slot 1
        DVD drive in Slot 2

    Keyboardmania
    -------------
        GQ971 Backplane
        GQ972 Main Board
        GQ974 Extend Board
        Yamaha XT446 board (for keyboard sounds) (the board layout matches the Yamaha MU100 Tone Generator)
        GQ971 Sound Amp
        CD-ROM drive in Slot 1
        CD-ROM drive in Slot 2

    ParaParaParadise
    ----------------
        GQ986 Backplane
        GQ972 Main Board
        2x CD-ROM drive in Slot 1

    Pop'n Music
    ------------
        GQ986 Backplane
        GQ971 SPU
        GQ972 Main Board
        CD-ROM drive in Slot 1
        DVD-ROM drive in Slot 2




    Games that run on this hardware:

    BIOS       Game ID        Year    Game
    ------------------------------------------------------------------
    GQ972      GQ972          2000    Beatmania III
    GQ972      GCA05          2000    Beatmania III Append Core Remix
    GCA21      GCA21          2001    Beatmania III Append 6th Mix
    GCA21      GCB07          2002    Beatmania III Append 7th Mix
    GCA21      GCC01          2003    Beatmania III The Final
    GQ974      GQ974          2000    Keyboardmania
    ???        GU974          2000    Keyboard Heaven
    GQ974      GCA01          2000    Keyboardmania 2nd Mix
    GQ974      GCA12          2001    Keyboardmania 3rd Mix
    GQ977      GQ977          2000    Para Para Paradise
    GQ977      GQ977          2000    Para Para Dancing
    GQ977      GC977          2000    Para Para Paradise 1.1
    GQ977      GQA11          2000    Para Para Paradise 1st Mix+
    GQA02(?)   GQ986          2000    Pop'n Music 4
    ???        G?A04          2000    Pop'n Music 5
    ???        GQA16          2001    Pop'n Music 6
    GQA02      GCB00          2001    Pop'n Music 7
    ???        GQB30          2002    Pop'n Music 8
    ???        GQ976          2000    Pop'n Music Mickey Tunes
    ???        GQ976          2000    Pop'n Music Mickey Tunes!
    ???        GQ987          2000    Pop'n Music Animelo
    ???        GEA02          2001    Pop'n Music Animelo 2

    TODO:
        - The external Yamaha MIDI sound board is not emulated (no keyboard sounds).


        - Notes on how the video is supposed to work from Ville / Ian Patterson:

        There are four "display contexts" that are set up via registers 20-4E. They are
        basically just raw framebuffers. 40-4E sets the base framebuffer pointer, 30-3E
        sets the size, 20-2E may set the minimum x and y coordinates but I haven't seen
        them set to something other than 0 yet. One context is set as the one the RAMDAC
        outputs to the monitor (not sure how this is selected yet, probably the lower
        bits of register 12). Thestartup test in the popn BIOS checks all of VRAM, so
        it moves the currentdisplay address around so you don't see crazy colors, which
        is very helpful in figuring out how this part works.

        The other new part is that there are two VRAM write ports, managed by registers
        60+68+70 and 64+6A+74, with status read from the lower bits of reg 7A. Each context
        can either write to VRAM as currently emulated, or the port can be switched in to
        "immediate mode" via registers 68/6A. Immedate mode can be used to run GCU commands
        at any point during the frame. It's mainly used to call display lists, which is where
        the display list addresses come from. Some games use it to send other commands, so
        it appears to be a 4-dword FIFO or something along those lines.

        // IRQs
        // IRQ 0: VBlank
        // IRQ 1: Extend board IRQ
        // IRQ 2: Main board UART
        // IRQ 3: SPU mailbox interrupt
        // IRQ 4: ATA
*/

#include "emu.h"
#include "k057714.h"
#include "midikbd.h"

#include "bus/ata/ataintf.h"
#include "bus/ata/atapicdr.h"
#include "bus/ata/hdd.h"
#include "cpu/m68000/m68000.h"
#include "cpu/powerpc/ppc.h"
#include "machine/fdc37c665gt.h"
#include "machine/ins8250.h"
#include "machine/intelfsh.h"
#include "machine/mb8421.h"
#include "machine/rtc65271.h"
#include "machine/timer.h"
#include "sound/cdda.h"
#include "sound/xt446.h"
#include "sound/rf5c400.h"
#include "sound/ymz280b.h"

#include "imagedev/floppy.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "osdcomm.h"

#include "wdlfft/fft.h"

#include <cmath>

#include "firebeat.lh"


/*****************************************************************************/
DECLARE_DEVICE_TYPE(KONAMI_FIREBEAT_EXTEND_SPECTRUM_ANALYZER, firebeat_extend_spectrum_analyzer_device)

class firebeat_extend_spectrum_analyzer_device : public device_t, public device_mixer_interface
{
public:
	firebeat_extend_spectrum_analyzer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read(offs_t offset);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_sound_interface-level overrides
	void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	enum {
		TOTAL_BUFFERS = 2,
		TOTAL_CHANNELS = 2,
		TOTAL_BARS = 6,

		FFT_LENGTH = 512
	};

	void update_fft();
	void apply_fft(uint32_t buf_index);

	float m_audio_buf[TOTAL_BUFFERS][TOTAL_CHANNELS][FFT_LENGTH]{};
	float m_fft_buf[TOTAL_CHANNELS][FFT_LENGTH]{};
	int m_audio_fill_index = 0;
	int m_audio_count[TOTAL_CHANNELS]{};

	int m_bars[TOTAL_CHANNELS][TOTAL_BARS]{};
};

firebeat_extend_spectrum_analyzer_device::firebeat_extend_spectrum_analyzer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, KONAMI_FIREBEAT_EXTEND_SPECTRUM_ANALYZER, tag, owner, clock),
	device_mixer_interface(mconfig, *this, 2)
{
}

void firebeat_extend_spectrum_analyzer_device::device_start()
{
	WDL_fft_init();
}

void firebeat_extend_spectrum_analyzer_device::device_reset()
{
	for (int ch = 0; ch < TOTAL_CHANNELS; ch++)
	{
		for (int i = 0; i < TOTAL_BUFFERS; i++)
			std::fill(std::begin(m_audio_buf[i][ch]), std::end(m_audio_buf[i][ch]), 0);

		std::fill(std::begin(m_fft_buf[ch]), std::end(m_fft_buf[ch]), 0);
		std::fill(std::begin(m_bars[ch]), std::end(m_bars[ch]), 0);

		m_audio_count[ch] = 0;
	}

	m_audio_fill_index = 0;
}

void firebeat_extend_spectrum_analyzer_device::sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs)
{
	device_mixer_interface::sound_stream_update(stream, inputs, outputs);

	for (int pos = 0; pos < outputs[0].samples(); pos++)
	{
		for (int ch = 0; ch < outputs.size(); ch++)
		{
			const float sample = outputs[ch].get(pos);
			m_audio_buf[m_audio_fill_index][ch][m_audio_count[m_audio_fill_index]] = sample;
		}

		update_fft();
	}

	// Bandpass filter + find peak
	// The results aren't accurate to the real circuit used on the PCB but the feature is very minor
	// and does not affect gameplay in any way.

	// Band values taken directly from NJU7507 data sheet.
	constexpr double NOTCHES[] = { 95, 240, 600, 1500, 3400, 8200, 18000 };
	constexpr int LAST_NOTCH = 7;

	auto srate = stream.sample_rate();
	auto order = WDL_fft_permute_tab(FFT_LENGTH / 2);
	for (int ch = 0; ch < TOTAL_CHANNELS; ch++)
	{
		double notch_max[TOTAL_BARS] = { -1, -1, -1, -1, -1, -1 };
		int cur_notch = 0;

		for (int i = 0; i <= FFT_LENGTH / 2; i++) {
			const double freq = (double)i / FFT_LENGTH * srate;

			if (freq < NOTCHES[cur_notch])
				continue;

			if (freq > NOTCHES[cur_notch+1])
				cur_notch++;

			if (cur_notch >= LAST_NOTCH) // Don't need to calculate anything above this frequency
				break;

			WDL_FFT_COMPLEX *bin = (WDL_FFT_COMPLEX*)m_fft_buf[ch] + order[i];

			const double re = bin->re;
			const double im = bin->im;
			const double mag = sqrt(re*re + im*im);

			if (notch_max[cur_notch] == -1 && freq >= NOTCHES[cur_notch] && freq < NOTCHES[cur_notch+1])
				notch_max[cur_notch] = mag;
		}

		for (int i = 0; i < TOTAL_BARS; i++)
		{
			double val = log10(notch_max[i] * 4096) * 20;
			val = std::max<double>(0, val);
			m_bars[ch][i] = uint32_t(std::min<double>(val, 255.0f));
		}
	}
}

void firebeat_extend_spectrum_analyzer_device::apply_fft(uint32_t buf_index)
{
	float *audio_l = m_audio_buf[buf_index][0];
	float *audio_r = m_audio_buf[buf_index][1];
	float *buf_l = m_fft_buf[0];
	float *buf_r = m_fft_buf[1];

	for (int i = 0; i < FFT_LENGTH; i++)
	{
		*buf_l++ = *audio_l++;
		*buf_r++ = *audio_r++;
	}

	for (int ch = 0; ch < TOTAL_CHANNELS; ch++)
	{
		WDL_real_fft((WDL_FFT_REAL *)m_fft_buf[ch], FFT_LENGTH, 0);

		for (int i = 0; i < FFT_LENGTH; i++)
			m_fft_buf[ch][i] /= (WDL_FFT_REAL)FFT_LENGTH;
	}
}

void firebeat_extend_spectrum_analyzer_device::update_fft()
{
	m_audio_count[m_audio_fill_index]++;
	if (m_audio_count[m_audio_fill_index] >= FFT_LENGTH)
	{
		apply_fft(m_audio_fill_index);

		m_audio_fill_index = 1 - m_audio_fill_index;
		m_audio_count[m_audio_fill_index] = 0;
	}
}

uint8_t firebeat_extend_spectrum_analyzer_device::read(offs_t offset)
{
	// Visible in the sound test menu and used for the spectral analyzer game skin
	// The actual data is coming from a circuit built on the extend board made up of NJU7507 x2 for each channel of audio.
	// The spectrum analyzer circuit is only populated on the extend board used by beatmania III but the footprints and
	// labels are still on the PCB for other games that use the extend board.

	// Return values notes:
	// - The values are based on the results of a chained NJU7507 which should give 14 bands worth of frequency data. However, only 6 bands are used in-game.
	// - Anything <= 8 will not display anything in-game and seems unused in memory
	// - It reads the upper and lower half of the register separately as bytes, but it the upper byte doesn't seem like it's actually used.
	// - In-game the skin shows up to +9 dB but it actually caps out at -3 dB on the skin

	int ch = offset >= 0x40; // 0 = Left, 1 = Right
	int notch = 6 - (((offset >> 2) & 0x0f) >> 1);
	int is_upper = BIT(offset, 2);

	auto val = (ch < TOTAL_CHANNELS && notch >= 0 && notch < TOTAL_BARS) ? m_bars[ch][notch] : 0;

	return (is_upper ? (val >> 8) : val) & 0xff;
}

DEFINE_DEVICE_TYPE(KONAMI_FIREBEAT_EXTEND_SPECTRUM_ANALYZER, firebeat_extend_spectrum_analyzer_device, "firebeat_spectrum_analyzer", "Firebeat Spectrum Analyzer")

/*****************************************************************************/
namespace {

struct IBUTTON_SUBKEY
{
	uint8_t identifier[8];
	uint8_t password[8];
	uint8_t data[0x30];
};

struct IBUTTON
{
	IBUTTON_SUBKEY subkey[3]{};
};

/*****************************************************************************/
static void firebeat_ata_devices(device_slot_interface &device)
{
	device.option_add("cdrom", ATAPI_CDROM);
	device.option_add("dvdrom", ATAPI_DVDROM);
	device.option_add("hdd", IDE_HARDDISK);
}

static void cdrom_config(device_t *device)
{
	device->subdevice<cdda_device>("cdda")->add_route(0, "^^lspeaker", 0.5);
	device->subdevice<cdda_device>("cdda")->add_route(1, "^^rspeaker", 0.5);
}

static void dvdrom_config(device_t *device)
{
	downcast<atapi_cdrom_device &>(*device).set_ultra_dma_mode(0x0102);
}

/*****************************************************************************/
class firebeat_state : public driver_device
{
public:
	firebeat_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_work_ram(*this, "work_ram"),
		m_ata(*this, "ata"),
		m_gcu(*this, "gcu"),
		m_gcu_sub(*this, "gcu_sub"),
		m_duart_com(*this, "duart_com"),
		m_status_leds(*this, "status_led_%u", 0U),
		m_io_inputs(*this, "IN%u", 0U)
	{ }

	void firebeat(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;

	uint32_t screen_update_firebeat_0(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void init_firebeat();

	void firebeat_map(address_map &map) ATTR_COLD;
	void ymz280b_map(address_map &map) ATTR_COLD;

	void init_lights(write32s_delegate out1, write32s_delegate out2, write32s_delegate out3);
	void lamp_output_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void lamp_output2_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void lamp_output3_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void ata_interrupt(int state);
	void gcu_interrupt(int state);
	void sound_irq_callback(int state);

	int m_cabinet_info = 0;

	uint8_t m_extend_board_irq_enable = 0;
	uint8_t m_extend_board_irq_active = 0;

	required_device<ppc4xx_device> m_maincpu;
	required_shared_ptr<uint32_t> m_work_ram;
	required_device<ata_interface_device> m_ata;
	required_device<k057714_device> m_gcu;
	optional_device<k057714_device> m_gcu_sub;

private:
	uint32_t cabinet_r(offs_t offset, uint32_t mem_mask = ~0);

	void set_ibutton(uint8_t *data);
	int ibutton_w(uint8_t data);
	void security_w(uint8_t data);

	uint8_t extend_board_irq_r(offs_t offset);
	void extend_board_irq_w(offs_t offset, uint8_t data);

	uint8_t input_r(offs_t offset);

	void control_w(offs_t offset, uint8_t data);

//  uint32_t comm_uart_r(offs_t offset, uint32_t mem_mask = ~ 0);
//  void comm_uart_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	IBUTTON m_ibutton;
	int m_ibutton_state = 0;
	int m_ibutton_read_subkey_ptr = 0;
	uint8_t m_ibutton_subkey_data[0x40]{};

	required_device<pc16552_device> m_duart_com;

	output_finder<8> m_status_leds;

	required_ioport_array<4> m_io_inputs;

	uint8_t m_control = 0;
};

class firebeat_spu_state : public firebeat_state
{
public:
	firebeat_spu_state(const machine_config &mconfig, device_type type, const char *tag) :
		firebeat_state(mconfig, type, tag),
		m_spuata(*this, "spu_ata"),
		m_rf5c400(*this, "rf5c400"),
		m_audiocpu(*this, "audiocpu"),
		m_dpram(*this, "spuram"),
		m_waveram(*this, "rf5c400_ram"),
		m_spu_status_leds(*this, "spu_status_led_%u", 0U)
	{ }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void device_resolve_objects() override ATTR_COLD;

	void firebeat_spu_base(machine_config &config);
	void firebeat_spu_map(address_map &map) ATTR_COLD;
	void spu_map(address_map &map) ATTR_COLD;
	void rf5c400_map(address_map &map) ATTR_COLD;

	void spu_ata_dmarq(int state);
	void spu_ata_interrupt(int state);
	TIMER_CALLBACK_MEMBER(spu_dma_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(spu_timer_callback);

	required_device<ata_interface_device> m_spuata;
	required_device<rf5c400_device> m_rf5c400;

private:
	void spu_status_led_w(uint16_t data);
	void spu_irq_ack_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void spu_ata_dma_low_w(uint16_t data);
	void spu_ata_dma_high_w(uint16_t data);
	void spu_wavebank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t firebeat_waveram_r(offs_t offset);
	void firebeat_waveram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	emu_timer *m_dma_timer = nullptr;
	bool m_sync_ata_irq = false;

	uint32_t m_spu_ata_dma = 0;
	int m_spu_ata_dmarq = 0;
	uint32_t m_wave_bank = 0;

	required_device<m68000_device> m_audiocpu;
	required_device<cy7c131_device> m_dpram;
	required_shared_ptr<uint16_t> m_waveram;

	output_finder<8> m_spu_status_leds;
};

/*****************************************************************************/

class firebeat_ppp_state : public firebeat_state
{
public:
	firebeat_ppp_state(const machine_config &mconfig, device_type type, const char *tag) :
		firebeat_state(mconfig, type, tag),
		m_stage_leds(*this, "stage_led_%u", 0U),
		m_top_leds(*this, "top_led_%u", 0U),
		m_lamps(*this, "lamp_%u", 0U),
		m_cab_led_left(*this, "left"),
		m_cab_led_right(*this, "right"),
		m_cab_led_door_lamp(*this, "door_lamp"),
		m_cab_led_ok(*this, "ok"),
		m_cab_led_slim(*this, "slim"),
		m_io_sensors(*this, "SENSOR%u", 1U)
	{ }

	void firebeat_ppp(machine_config &config);
	void init_ppp_base();
	void init_ppp_jp();
	void init_ppp_overseas();

private:
	virtual void device_resolve_objects() override ATTR_COLD;

	void firebeat_ppp_map(address_map &map) ATTR_COLD;

	uint16_t sensor_r(offs_t offset);

	void lamp_output_ppp_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void lamp_output2_ppp_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void lamp_output3_ppp_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	output_finder<8> m_stage_leds;
	output_finder<8> m_top_leds;
	output_finder<4> m_lamps;
	output_finder<> m_cab_led_left;
	output_finder<> m_cab_led_right;
	output_finder<> m_cab_led_door_lamp;
	output_finder<> m_cab_led_ok;
	output_finder<> m_cab_led_slim;

	required_ioport_array<4> m_io_sensors;
};

class firebeat_kbm_state : public firebeat_state
{
public:
	firebeat_kbm_state(const machine_config &mconfig, device_type type, const char *tag) :
		firebeat_state(mconfig, type, tag),
		m_duart_midi(*this, "duart_midi"),
		m_kbd(*this, "kbd%u", 0),
		m_lamps(*this, "lamp_%u", 1U),
		m_cab_led_door_lamp(*this, "door_lamp"),
		m_cab_led_start1p(*this, "start1p"),
		m_cab_led_start2p(*this, "start2p"),
		m_lamp_neon(*this, "neon"),
		m_io_wheels(*this, "WHEEL_P%u", 1U)
	{ }

	uint32_t screen_update_firebeat_1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void init_kbm_base();
	void init_kbm_jp();
	void init_kbm_overseas();
	void firebeat_kbm(machine_config &config);

private:
	virtual void device_resolve_objects() override ATTR_COLD;

	void firebeat_kbm_map(address_map &map) ATTR_COLD;

	void init_keyboard();

	uint8_t keyboard_wheel_r(offs_t offset);

	void lamp_output_kbm_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint8_t midi_uart_r(offs_t offset);
	void midi_uart_w(offs_t offset, uint8_t data);

//  TIMER_CALLBACK_MEMBER(keyboard_timer_callback);
	void midi_keyboard_right_irq_callback(int state);
	void midi_keyboard_left_irq_callback(int state);

//  emu_timer *m_keyboard_timer;
//  int m_keyboard_state[2];

	required_device<pc16552_device> m_duart_midi;
	required_device_array<midi_keyboard_device, 2> m_kbd;

	output_finder<3> m_lamps;
	output_finder<> m_cab_led_door_lamp;
	output_finder<> m_cab_led_start1p;
	output_finder<> m_cab_led_start2p;
	output_finder<> m_lamp_neon;

	required_ioport_array<2> m_io_wheels;
};

class firebeat_bm3_state : public firebeat_spu_state
{
public:
	firebeat_bm3_state(const machine_config &mconfig, device_type type, const char *tag) :
		firebeat_spu_state(mconfig, type, tag),
		m_fdc(*this, "fdc"),
		m_floppy(*this, "fdc:fdc:0"),
		m_spectrum_analyzer(*this, "spectrum_analyzer"),
		m_duart_midi(*this, "duart_midi"),
		m_io(*this, "IO%u", 1U),
		m_io_turntables(*this, "TURNTABLE_P%u", 1U),
		m_io_effects(*this, "EFFECT%u", 1U)
	{ }

	void firebeat_bm3(machine_config &config);
	void init_bm3();

private:
	void firebeat_bm3_map(address_map &map) ATTR_COLD;

	uint8_t spectrum_analyzer_r(offs_t offset);
	uint16_t sensor_r(offs_t offset);

	uint8_t midi_uart_r(offs_t offset);
	void midi_uart_w(offs_t offset, uint8_t data);

	void midi_st224_irq_callback(int state);

	required_device<fdc37c665gt_device> m_fdc;
	required_device<floppy_connector> m_floppy;
	required_device<firebeat_extend_spectrum_analyzer_device> m_spectrum_analyzer;
	required_device<pc16552_device> m_duart_midi;

	required_ioport_array<4> m_io;
	required_ioport_array<2> m_io_turntables;
	required_ioport_array<7> m_io_effects;

	void floppy_irq_callback(int state);
};

class firebeat_popn_state : public firebeat_spu_state
{
public:
	firebeat_popn_state(const machine_config &mconfig, device_type type, const char *tag) :
		firebeat_spu_state(mconfig, type, tag)
	{ }

	void firebeat_popn(machine_config &config);
	void init_popn_base();
	void init_popn_jp();
	void init_popn_rental();
};

/*****************************************************************************/

uint32_t firebeat_state::screen_update_firebeat_0(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return m_gcu->draw(screen, bitmap, cliprect); }
uint32_t firebeat_kbm_state::screen_update_firebeat_1(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect){ return m_gcu_sub->draw(screen, bitmap, cliprect); }

void firebeat_state::machine_start()
{
	/* set conservative DRC options */
	m_maincpu->ppcdrc_set_options(PPCDRC_COMPATIBLE_OPTIONS);

	/* configure fast RAM regions for DRC */
	m_maincpu->ppcdrc_add_fastram(0x00000000, 0x01ffffff, false, m_work_ram);
}

void firebeat_state::machine_reset()
{
	m_extend_board_irq_enable = 0x3f;
	m_extend_board_irq_active = 0x00;
	m_control = 0;
}

void firebeat_state::device_resolve_objects()
{
	m_status_leds.resolve();
}

void firebeat_state::init_firebeat()
{
	uint8_t *rom = memregion("user2")->base();
	set_ibutton(rom);

//  pc16552d_init(machine(), 0, 19660800, comm_uart_irq_callback, 0);     // Network UART

	// Set to defaults here, but overridden for most specific games. It represents various bits of
	// data, such as the firebeat's ability to play certain games at all, whether the firebeat is
	// meant as a rental unit or normal unit, and whether the unit is meant as a JP firebeat or
	// overseas firebeat. Unfortunately, some of the bits seem to change meaning depending on the
	// game series. Different dongles force games to check for different bits set in this info.
	// Specifics of the bitmask are documented in the various game series init functions.
	m_cabinet_info = 0;

	m_maincpu->ppc4xx_spu_set_tx_handler(write8smo_delegate(*this, FUNC(firebeat_state::security_w)));

	init_lights(write32s_delegate(*this), write32s_delegate(*this), write32s_delegate(*this));
}

void firebeat_state::firebeat(machine_config &config)
{
	/* basic machine hardware */
	PPC403GCX(config, m_maincpu, XTAL(66'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &firebeat_state::firebeat_map);

	RTC65271(config, "rtc", 0);

	FUJITSU_29F016A(config, "flash_main");
	FUJITSU_29F016A(config, "flash_snd1");
	FUJITSU_29F016A(config, "flash_snd2");

	ATA_INTERFACE(config, m_ata).options(firebeat_ata_devices, "cdrom", "cdrom", true);
	m_ata->irq_handler().set(FUNC(firebeat_state::ata_interrupt));
	m_ata->slot(1).set_option_machine_config("cdrom", cdrom_config);

	/* video hardware */
	PALETTE(config, "palette", palette_device::RGB_555);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 525, 0, 480);
	screen.set_screen_update(FUNC(firebeat_state::screen_update_firebeat_0));
	screen.set_palette("palette");
	screen.screen_vblank().set(m_gcu, FUNC(k057714_device::vblank_w));

	K057714(config, m_gcu, 0).set_screen("screen");
	m_gcu->irq_callback().set(FUNC(firebeat_state::gcu_interrupt));

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ymz280b_device &ymz(YMZ280B(config, "ymz", 16934400));
	ymz.irq_handler().set(FUNC(firebeat_state::sound_irq_callback));
	ymz.set_addrmap(0, &firebeat_state::ymz280b_map);
	ymz.add_route(1, "lspeaker", 1.0);
	ymz.add_route(0, "rspeaker", 1.0);

	PC16552D(config, "duart_com", 0);
	NS16550(config, "duart_com:chan0", XTAL(19'660'800));
	NS16550(config, "duart_com:chan1", XTAL(19'660'800));
}

void firebeat_state::firebeat_map(address_map &map)
{
	map(0x00000000, 0x01ffffff).ram().share("work_ram");
	map(0x70006000, 0x70006003).w(FUNC(firebeat_state::extend_board_irq_w));
	map(0x7000a000, 0x7000a003).r(FUNC(firebeat_state::extend_board_irq_r));
	map(0x74000000, 0x740003ff).noprw(); // SPU shared RAM
	map(0x7d000200, 0x7d00021f).r(FUNC(firebeat_state::cabinet_r));
	map(0x7d000400, 0x7d000401).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write));
	map(0x7d000500, 0x7d000501).w(FUNC(firebeat_state::control_w));
	map(0x7d000800, 0x7d000803).r(FUNC(firebeat_state::input_r));
	map(0x7d400000, 0x7d5fffff).rw("flash_main", FUNC(fujitsu_29f016a_device::read), FUNC(fujitsu_29f016a_device::write));
	map(0x7d800000, 0x7d9fffff).rw("flash_snd1", FUNC(fujitsu_29f016a_device::read), FUNC(fujitsu_29f016a_device::write));
	map(0x7da00000, 0x7dbfffff).rw("flash_snd2", FUNC(fujitsu_29f016a_device::read), FUNC(fujitsu_29f016a_device::write));
	map(0x7dc00000, 0x7dc0000f).rw(m_duart_com, FUNC(pc16552_device::read), FUNC(pc16552_device::write));
	map(0x7e000000, 0x7e00003f).rw("rtc", FUNC(rtc65271_device::rtc_r), FUNC(rtc65271_device::rtc_w));
	map(0x7e000100, 0x7e00013f).rw("rtc", FUNC(rtc65271_device::xram_r), FUNC(rtc65271_device::xram_w));
	map(0x7e800000, 0x7e8000ff).rw(m_gcu, FUNC(k057714_device::read), FUNC(k057714_device::write));
	map(0x7e800100, 0x7e8001ff).noprw(); // Secondary GCU, only used by Keyboardmania but is written to during the bootloader of other games
	map(0x7fe00000, 0x7fe0000f).rw(m_ata, FUNC(ata_interface_device::cs0_swap_r), FUNC(ata_interface_device::cs0_swap_w));
	map(0x7fe80000, 0x7fe8000f).rw(m_ata, FUNC(ata_interface_device::cs1_swap_r), FUNC(ata_interface_device::cs1_swap_w));
	map(0x7ff80000, 0x7fffffff).rom().region("user1", 0);       /* System BIOS */
}

void firebeat_state::ymz280b_map(address_map &map)
{
	map.global_mask(0x3fffff);
	map(0x000000, 0x1fffff).r("flash_snd1", FUNC(fujitsu_29f016a_device::read));
	map(0x200000, 0x3fffff).r("flash_snd2", FUNC(fujitsu_29f016a_device::read));
}

/*****************************************************************************/

uint32_t firebeat_state::cabinet_r(offs_t offset, uint32_t mem_mask)
{
//  printf("cabinet_r: %08X, %08X\n", offset, mem_mask);
	switch (offset)
	{
		case 0: return m_cabinet_info << 28;

		// These never seem to be anything other than 0?
		case 2: return 0;
		case 4: return 0;
	}

	return 0;
}

/*****************************************************************************/

/* Security dongle is a Dallas DS1411 RS232 Adapter with a DS1991 Multikey iButton */

/*
    Each DS1991 dongle has 3 secure enclaves. The first enclave is always the game
    serial number. This is a 9 digit alphanumeric ID. The first three characters are
    always the game's code, and the rest of the characters are all digits. The fourth
    character seems to be a region specifier and causes many games to check against
    values in the m_cabinet_info register to verify that the hardware matches. This was
    used to region lock JP and overseas data as well as specify that certain firebeats
    only accept e-Amusement dongles (Konami's rental service before it was an online network).

    Odd numbers in the 4th position correspond to JP data, with 1 and 3 being observed
    values in the wild. Some games also accept a 7 and a few games also accept a 5.
    Even numbers in the 4th position correspond to overseas data, with 4 being the only
    observed value. A 0 or 9 in the 4th position is game-specific (much like the handling of
    m_cabinet_info) but generally correspond to rental data.

    The second enclave is license data for some Pop'n Music games and specifies the length
    of time a dongle is valid for. The RTCRAM is used for this check which is why there is
    no operator menu to change the RTC. Instead, the time is set using the license check
    screen that appears on some series such as Pop'n Music and Firebeat. It is encoded in
    the password that is given to the operator to pass the check. For games which do not use
    extended license information, this enclave is left blank.

    The third enclave is a mode switch. Every game looks for some unique set of data here
    and will turn on manufacture/service mode if the right value is set. Some games also
    look for overseas and rental strings here and a few also have no hardware check dongles
    and debug dongles. In the case of normal retail dongles, this enclave is left blank.
*/

enum
{
	DS1991_STATE_NORMAL,
	DS1991_STATE_READ_SUBKEY
};

void firebeat_state::set_ibutton(uint8_t *data)
{
	int i, j;

	for (i=0; i < 3; i++)
	{
		// identifier
		for (j=0; j < 8; j++)
		{
			m_ibutton.subkey[i].identifier[j] = *data++;
		}

		// password
		for (j=0; j < 8; j++)
		{
			m_ibutton.subkey[i].password[j] = *data++;
		}

		// data
		for (j=0; j < 48; j++)
		{
			m_ibutton.subkey[i].data[j] = *data++;
		}
	}
}

int firebeat_state::ibutton_w(uint8_t data)
{
	int r = -1;

	switch (m_ibutton_state)
	{
		case DS1991_STATE_NORMAL:
		{
			switch (data)
			{
				//
				// DS2408B Serial 1-Wire Line Driver with Load Sensor
				//
				case 0xc1:          // DS2480B reset
				{
					r = 0xcd;
					break;
				}
				case 0xe1:          // DS2480B set data mode
				{
					break;
				}
				case 0xe3:          // DS2480B set command mode
				{
					break;
				}

				//
				// DS1991 MultiKey iButton
				//
				case 0x66:          // DS1991 Read SubKey
				{
					r = 0x66;
					m_ibutton_state = DS1991_STATE_READ_SUBKEY;
					m_ibutton_read_subkey_ptr = 0;
					break;
				}
				case 0xcc:          // DS1991 skip rom
				{
					r = 0xcc;
					m_ibutton_state = DS1991_STATE_NORMAL;
					break;
				}
				default:
				{
					fatalerror("ibutton: unknown normal mode cmd %02X\n", data);
				}
			}
			break;
		}

		case DS1991_STATE_READ_SUBKEY:
		{
			if (m_ibutton_read_subkey_ptr == 0)      // Read SubKey, 2nd command byte
			{
				int subkey = (data >> 6) & 0x3;
		//      printf("iButton SubKey %d\n", subkey);
				r = data;

				if (subkey < 3)
				{
					memcpy(&m_ibutton_subkey_data[0],  m_ibutton.subkey[subkey].identifier, 8);
					memcpy(&m_ibutton_subkey_data[8],  m_ibutton.subkey[subkey].password, 8);
					memcpy(&m_ibutton_subkey_data[16], m_ibutton.subkey[subkey].data, 0x30);
				}
				else
				{
					memset(&m_ibutton_subkey_data[0], 0, 0x40);
				}
			}
			else if (m_ibutton_read_subkey_ptr == 1) // Read SubKey, 3rd command byte
			{
				r = data;
			}
			else
			{
				r = m_ibutton_subkey_data[m_ibutton_read_subkey_ptr-2];
			}
			m_ibutton_read_subkey_ptr++;
			if (m_ibutton_read_subkey_ptr >= 0x42)
			{
				m_ibutton_state = DS1991_STATE_NORMAL;
			}
			break;
		}
	}

	return r;
}

void firebeat_state::security_w(uint8_t data)
{
	int r = ibutton_w(data);
	if (r >= 0)
		m_maincpu->ppc4xx_spu_receive_byte(r);
}

// Extend board IRQs
// 0x01: MIDI UART channel 2
// 0x02: MIDI UART channel 1
// 0x04: FDD
// 0x08: ?
// 0x10: ?
// 0x20: ?
// 0x40: Used by spectrum analyzer feature in beatmania III. Audio related?

uint8_t firebeat_state::extend_board_irq_r(offs_t offset)
{
	return ~m_extend_board_irq_active;
}

void firebeat_state::extend_board_irq_w(offs_t offset, uint8_t data)
{
//  printf("extend_board_irq_w: %08X, %08X\n", data, offset);

	auto is_fdd_irq_enabled = BIT(m_extend_board_irq_enable, 2);

	m_extend_board_irq_active &= ~(data & 0xff);
	m_extend_board_irq_enable = data & 0xff;

	if (BIT(m_extend_board_irq_enable, 2) != is_fdd_irq_enabled)
	{
		// Clearing the FDD IRQ here helps fix some issues with the FDD getting stuck
		m_maincpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
	}
}

/*****************************************************************************/

uint8_t firebeat_state::input_r(offs_t offset)
{
	switch (offset)
	{
		case 0: return (m_io_inputs[0]->read() & 0xff);
		case 1: return (m_io_inputs[1]->read() & 0xff);
		case 2: return (m_io_inputs[2]->read() & 0xff);
		case 3: return (m_io_inputs[3]->read() & 0xff);
	}

	return 0;
}

/*****************************************************************************/

void firebeat_state::control_w(offs_t offset, uint8_t data)
{
	// 0x01 - 31kHz (25.175 MHz)/24kHz (16.6666 MHz) clock switch
	// 0x02 - Unused?
	// 0x04 - Set to 1 by all games except beatmania III, usage unknown. Screen related?
	// 0x08 - Toggles screen mirroring when only one GCU is in use? Default 0
	// 0x80 - Used by ParaParaParadise and Keyboardmania. Set to 1 when doing YMZ flash initialization?

	if (BIT(data, 0) == 0 && BIT(m_control, 0) == 1)
	{
		// Set screen to 31kHz from 24kHz
		m_gcu->set_pixclock(25.175_MHz_XTAL);

		if (m_gcu_sub)
			m_gcu_sub->set_pixclock(25.175_MHz_XTAL);
	}
	else if (BIT(data, 0) == 1 && BIT(m_control, 0) == 0)
	{
		// Set screen to 24kHz from 31kHz
		m_gcu->set_pixclock(16.6666_MHz_XTAL);

		if (m_gcu_sub)
			m_gcu_sub->set_pixclock(16.6666_MHz_XTAL);
	}

	m_control = data;
}


/*****************************************************************************/

/*
uint32_t firebeat_state::comm_uart_r(offs_t offset, uint32_t mem_mask)
{
    uint32_t r = 0;

    if (ACCESSING_BITS_24_31)
    {
        r |= pc16552d_0_r((offset*4)+0) << 24;
    }
    if (ACCESSING_BITS_16_23)
    {
        r |= pc16552d_0_r((offset*4)+1) << 16;
    }
    if (ACCESSING_BITS_8_15)
    {
        r |= pc16552d_0_r((offset*4)+2) << 8;
    }
    if (ACCESSING_BITS_0_7)
    {
        r |= pc16552d_0_r((offset*4)+3) << 0;
    }

    return r;
}

void firebeat_state::comm_uart_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
    if (ACCESSING_BITS_24_31)
    {
        pc16552d_0_w((offset*4)+0, (data >> 24) & 0xff);
    }
    if (ACCESSING_BITS_16_23)
    {
        pc16552d_0_w((offset*4)+1, (data >> 16) & 0xff);
    }
    if (ACCESSING_BITS_8_15)
    {
        pc16552d_0_w((offset*4)+2, (data >> 8) & 0xff);
    }
    if (ACCESSING_BITS_0_7)
    {
        pc16552d_0_w((offset*4)+3, (data >> 0) & 0xff);
    }
}

static void comm_uart_irq_callback(running_machine &machine, int channel, int value)
{
    // TODO
    //m_maincpu->set_input_line(INPUT_LINE_IRQ2, ASSERT_LINE);
}
*/


/*****************************************************************************/

void firebeat_state::init_lights(write32s_delegate out1, write32s_delegate out2, write32s_delegate out3)
{
	if (out1.isnull()) out1 = write32s_delegate(*this, FUNC(firebeat_state::lamp_output_w));
	if (out2.isnull()) out2 = write32s_delegate(*this, FUNC(firebeat_state::lamp_output2_w));
	if (out3.isnull()) out3 = write32s_delegate(*this, FUNC(firebeat_state::lamp_output3_w));

	m_maincpu->space(AS_PROGRAM).install_write_handler(0x7d000804, 0x7d000807, out1);
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x7d000320, 0x7d000323, out2);
	m_maincpu->space(AS_PROGRAM).install_write_handler(0x7d000324, 0x7d000327, out3);
}

void firebeat_state::lamp_output_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// -------- -------- -------- xxxxxxxx   Status LEDs (active low)
	if (ACCESSING_BITS_0_7)
	{
		m_status_leds[0] = BIT(~data, 0);
		m_status_leds[1] = BIT(~data, 1);
		m_status_leds[2] = BIT(~data, 2);
		m_status_leds[3] = BIT(~data, 3);
		m_status_leds[4] = BIT(~data, 4);
		m_status_leds[5] = BIT(~data, 5);
		m_status_leds[6] = BIT(~data, 6);
		m_status_leds[7] = BIT(~data, 7);
	}

//  printf("lamp_output_w: %08X, %08X, %08X\n", data, offset, mem_mask);
}

void firebeat_state::lamp_output2_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
//  printf("lamp_output2_w: %08X, %08X, %08X\n", data, offset, mem_mask);
}

void firebeat_state::lamp_output3_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
//  printf("lamp_output3_w: %08X, %08X, %08X\n", data, offset, mem_mask);
}

/*****************************************************************************/

void firebeat_state::ata_interrupt(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ4, state);
}

void firebeat_state::gcu_interrupt(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, state);
}

void firebeat_state::sound_irq_callback(int state)
{
}

/*****************************************************************************/

void firebeat_spu_state::machine_start()
{
	firebeat_state::machine_start();
	m_dma_timer = timer_alloc(FUNC(firebeat_spu_state::spu_dma_callback), this);
}

void firebeat_spu_state::machine_reset()
{
	firebeat_state::machine_reset();
	m_spu_ata_dma = 0;
	m_spu_ata_dmarq = 0;
	m_wave_bank = 0;
	m_sync_ata_irq = 0;
}

void firebeat_spu_state::device_resolve_objects()
{
	firebeat_state::device_resolve_objects();
	m_spu_status_leds.resolve();
}

void firebeat_spu_state::firebeat_spu_base(machine_config &config)
{
	firebeat(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &firebeat_spu_state::firebeat_spu_map);

	M68000(config, m_audiocpu, 16000000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &firebeat_spu_state::spu_map);

	CY7C131(config, m_dpram);
	m_dpram->intl_callback().set_inputline(m_audiocpu, INPUT_LINE_IRQ4); // address 0x3fe triggers M68K interrupt
	m_dpram->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ3); // address 0x3ff triggers PPC interrupt

	RF5C400(config, m_rf5c400, XTAL(16'934'400));
	m_rf5c400->set_addrmap(0, &firebeat_spu_state::rf5c400_map);

	// Clean channel audio
	m_rf5c400->add_route(0, "lspeaker", 0.5);
	m_rf5c400->add_route(1, "rspeaker", 0.5);
}

void firebeat_spu_state::firebeat_spu_map(address_map &map)
{
	firebeat_map(map);
	map(0x74000000, 0x740003ff).rw(m_dpram, FUNC(cy7c131_device::right_r), FUNC(cy7c131_device::right_w)); // SPU shared RAM
}

void firebeat_spu_state::spu_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x100000, 0x13ffff).ram();
	map(0x200000, 0x200001).portr("SPU_DSW");
	map(0x220000, 0x220001).w(FUNC(firebeat_spu_state::spu_status_led_w));
	map(0x230000, 0x230001).w(FUNC(firebeat_spu_state::spu_irq_ack_w));
	map(0x240000, 0x240003).w(FUNC(firebeat_spu_state::spu_ata_dma_low_w)).nopr();
	map(0x250000, 0x250003).w(FUNC(firebeat_spu_state::spu_ata_dma_high_w)).nopr();
	map(0x260000, 0x260001).w(FUNC(firebeat_spu_state::spu_wavebank_w)).nopr();
	map(0x280000, 0x2807ff).rw(m_dpram, FUNC(cy7c131_device::left_r), FUNC(cy7c131_device::left_w)).umask16(0x00ff);
	map(0x300000, 0x30000f).rw(m_spuata, FUNC(ata_interface_device::cs0_r), FUNC(ata_interface_device::cs0_w));
	map(0x340000, 0x34000f).rw(m_spuata, FUNC(ata_interface_device::cs1_r), FUNC(ata_interface_device::cs1_w));
	map(0x400000, 0x400fff).rw(m_rf5c400, FUNC(rf5c400_device::rf5c400_r), FUNC(rf5c400_device::rf5c400_w));
	map(0x800000, 0xffffff).rw(FUNC(firebeat_spu_state::firebeat_waveram_r), FUNC(firebeat_spu_state::firebeat_waveram_w));
}

void firebeat_spu_state::rf5c400_map(address_map& map)
{
	map(0x0000000, 0x1ffffff).ram().share("rf5c400_ram");
}


/*  SPU board M68K IRQs

    IRQ1: Executes all commands stored in a buffer.
          The buffer can contain up to 8 commands.
          This seems to be unused.

    IRQ2: Executes one command stored in a different buffer from IRQ1.
          The buffer can contain up to 8 commands.
          The command index counter increments after each IRQ2 call.
          If there is no command in the slot at the current counter then it just increments without executing a command.

          Timing matters. In particular if the speed of the IRQ 2 calls is too fast then the volume and frequency animations will be wrong.
          The most common issue with bad timing is keysounds will be cut off.
          pop'n music Animelo 2 also has an issue when playing CHA-LA HEAD CHA-LA where one of the beginning keysounds will stay on a
          very high pitched frequency. The lower(?) the IRQ 2 frequency, the longer the keysound stays played it seems.

          For beatmania III:
            cmd[0] = nop
            cmd[1] = 0x91bc -> Send stop command for all rf5c400 channels that are done playing
            cmd[2] = 0x310a -> Error checking? Sending some kind of state to main CPU???
            cmd[3] = 0x29c6 -> Increment a timer for each running DMA(ATA command?)
                Each timer must count up to 0x02e8 (744) before it will move on to the next DMA, which I believe is the time out counter.

                In another part of the program (0x363c for a21jca03.bin) is the following code for determining when to start and stop the DMA:

                start_dma();
                while (get_dma_timer() < dma_max_timer)
                {
                    if (irq6_called_flag)
                        break;
                }
                end_dma();

                irq6_called_flag is set only when IRQ6 is called.
                get_dma_timer is the timer that is incremented by 0x29c6.
            cmd[4] = 0x94de -> Animates rf5c400 channel volumes
            cmd[5] = 0x7b2c -> Send some kind of buffer status flags to spu_status_led_w. Related to IRQ4 since commands come from PPC to set buffer data
            cmd[6] = 0x977e -> Animates rf5c400 channel frequencies
            cmd[7] = 0x9204 -> Sends current state of rf5c400 channels as well as a list (bitmask integer) of usable channels up to main CPU memory.
                               Also sends a flag to to spu_status_led_w that shows if there are available SE slots.
                               If there are no available SE slots then it will set bit 3 to .

    IRQ4: Dual-port RAM mailbox (when PPC writes to 0x3FE)
          Handles commands from PPC (bytes 0x00 and 0x01)

    IRQ6: ATA
*/

void firebeat_spu_state::spu_status_led_w(uint16_t data)
{
	// Verified with real hardware that the patterns match the status LEDs on the board

	// Set when clearing waveram memory during initialization:
	// uint16_t bank = ((~data) >> 6) & 3;
	// uint16_t offset = (!!((~data) & (1 << 5))) * 8192;
	// uint16_t verify = !!((~data) & (1 << 4)); // 0 = writing memory, 1 = verifying memory

	// For IRQ2:
	// Command 5 (0x7b2c):
	// if buffer[4] == 0 and buffer[0] < buffer[5], it writes what buffer(thread?) is currently busy(?)
	// There are 8 buffers/threads total
	//
	// Command 7 (0x9204):
	// If all 30 SE channels are in use, bit 3 will be set to 1

	// -------- -------- -------- xxxxxxxx   Status LEDs (active low)
	m_spu_status_leds[0] = BIT(~data, 0);
	m_spu_status_leds[1] = BIT(~data, 1);
	m_spu_status_leds[2] = BIT(~data, 2);
	m_spu_status_leds[3] = BIT(~data, 3);
	m_spu_status_leds[4] = BIT(~data, 4);
	m_spu_status_leds[5] = BIT(~data, 5);
	m_spu_status_leds[6] = BIT(~data, 6);
	m_spu_status_leds[7] = BIT(~data, 7);
}

void firebeat_spu_state::spu_irq_ack_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	if (ACCESSING_BITS_0_7)
	{
		if (BIT(data, 0))
			m_audiocpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
		if (BIT(data, 1))
			m_audiocpu->set_input_line(INPUT_LINE_IRQ2, CLEAR_LINE);
		if (BIT(data, 3))
			m_audiocpu->set_input_line(INPUT_LINE_IRQ6, CLEAR_LINE);
	}
}

void firebeat_spu_state::spu_ata_dma_low_w(uint16_t data)
{
	m_spu_ata_dma = (m_spu_ata_dma & ~0xffff) | data;
}

void firebeat_spu_state::spu_ata_dma_high_w(uint16_t data)
{
	m_spu_ata_dma = (m_spu_ata_dma & 0xffff) | ((uint32_t)data << 16);
}

void firebeat_spu_state::spu_wavebank_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_wave_bank = data * (4 * 1024 * 1024);
}

uint16_t firebeat_spu_state::firebeat_waveram_r(offs_t offset)
{
	return m_waveram[offset + m_wave_bank];
}

void firebeat_spu_state::firebeat_waveram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_waveram[offset + m_wave_bank]);
}

void firebeat_spu_state::spu_ata_dmarq(int state)
{
	if (m_spuata != nullptr && m_spu_ata_dmarq != state)
	{
		m_spu_ata_dmarq = state;

		if (m_spu_ata_dmarq)
		{
			m_spuata->write_dmack(ASSERT_LINE);
			m_dma_timer->adjust(attotime::zero);
		}
	}
}

TIMER_CALLBACK_MEMBER(firebeat_spu_state::spu_dma_callback)
{
	uint16_t data = m_spuata->read_dma();
	m_waveram[m_wave_bank+m_spu_ata_dma] = data;
	m_spu_ata_dma++;

	if (m_spu_ata_dmarq)
	{
		// This timer adjust value was picked because
		// it reduces stuttering issues/performance issues
		m_dma_timer->adjust(attotime::from_nsec(350));
	}
	else
	{
		m_spuata->write_dmack(CLEAR_LINE);
		m_dma_timer->enable(false);
	}
}

void firebeat_spu_state::spu_ata_interrupt(int state)
{
	if (state == 0)
		m_audiocpu->set_input_line(INPUT_LINE_IRQ2, state);

	m_sync_ata_irq = state;
}

TIMER_DEVICE_CALLBACK_MEMBER(firebeat_spu_state::spu_timer_callback)
{
	if (m_sync_ata_irq)
		m_audiocpu->set_input_line(INPUT_LINE_IRQ6, 1);
	else
		m_audiocpu->set_input_line(INPUT_LINE_IRQ2, 1);
}

/*****************************************************************************
* beatmania III
******************************************************************************/
static void pc_hd_floppies(device_slot_interface &device)
{
	device.option_add("35hd", FLOPPY_35_HD);
}

void firebeat_bm3_state::floppy_irq_callback(int state)
{
	if (BIT(m_extend_board_irq_enable, 2) == 0 && state)
	{
		m_extend_board_irq_active |= 0x04;
		m_maincpu->set_input_line(INPUT_LINE_IRQ1, state);
	}
}

void firebeat_bm3_state::firebeat_bm3(machine_config &config)
{
	firebeat_spu_base(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &firebeat_bm3_state::firebeat_bm3_map);

	ATA_INTERFACE(config, m_spuata).options(firebeat_ata_devices, "hdd", nullptr, true);
	m_spuata->irq_handler().set(FUNC(firebeat_bm3_state::spu_ata_interrupt));
	m_spuata->dmarq_handler().set(FUNC(firebeat_bm3_state::spu_ata_dmarq));
	m_spuata->slot(0).set_fixed(true);

	// 500 hz seems ok for beatmania III.
	// Any higher makes things act weird.
	// Lower doesn't have that huge of an effect compared to pop'n? (limited tested).
	TIMER(config, "spu_timer").configure_periodic(FUNC(firebeat_bm3_state::spu_timer_callback), attotime::from_hz(500));

	FDC37C665GT(config, m_fdc, 24_MHz_XTAL, upd765_family_device::mode_t::PS2);
	m_fdc->fintr().set(FUNC(firebeat_bm3_state::floppy_irq_callback));

	FLOPPY_CONNECTOR(config, m_floppy, pc_hd_floppies, "35hd", floppy_image_device::default_pc_floppy_formats);

	PC16552D(config, m_duart_midi, 0);
	NS16550(config, "duart_midi:chan0", XTAL(24'000'000));
	NS16550(config, "duart_midi:chan1", XTAL(24'000'000)).out_int_callback().set(FUNC(firebeat_bm3_state::midi_st224_irq_callback));

	// Effects audio channel, routed to ST-224's audio input
	m_rf5c400->add_route(2, "lspeaker", 0.5);
	m_rf5c400->add_route(3, "rspeaker", 0.5);

	KONAMI_FIREBEAT_EXTEND_SPECTRUM_ANALYZER(config, m_spectrum_analyzer, 0);
	m_rf5c400->add_route(0, m_spectrum_analyzer, 0.5, AUTO_ALLOC_INPUT, 0);
	m_rf5c400->add_route(1, m_spectrum_analyzer, 0.5, AUTO_ALLOC_INPUT, 1);
	m_rf5c400->add_route(2, m_spectrum_analyzer, 0.5, AUTO_ALLOC_INPUT, 0);
	m_rf5c400->add_route(3, m_spectrum_analyzer, 0.5, AUTO_ALLOC_INPUT, 1);
}

void firebeat_bm3_state::init_bm3()
{
	init_firebeat();
	init_lights(write32s_delegate(*this), write32s_delegate(*this), write32s_delegate(*this));

	// No cabinet info changes for BMIII are needed at this point. All sourced data is for JP region
	// non-rental firebeat stacks. So, no region-specific overrides are present for BMIII. If we
	// were to ever find rental or overseas data for BMIII, we should break this out and document
	// the bitfield as used by BMIII series.
	m_cabinet_info = 0x0;
}

void firebeat_bm3_state::firebeat_bm3_map(address_map &map)
{
	firebeat_spu_map(map);
	map(0x70000000, 0x70000fff).rw(FUNC(firebeat_bm3_state::midi_uart_r), FUNC(firebeat_bm3_state::midi_uart_w)).umask32(0xff000000);
	map(0x70001000, 0x70001fff).rw(m_fdc, FUNC(fdc37c665gt_device::read), FUNC(fdc37c665gt_device::write)).umask32(0xff000000);
	map(0x70008000, 0x7000807f).r(m_spectrum_analyzer, FUNC(firebeat_extend_spectrum_analyzer_device::read));
	map(0x7d000330, 0x7d00033f).nopw(); // ?
	map(0x7d000340, 0x7d00035f).r(FUNC(firebeat_bm3_state::sensor_r));
}

uint16_t firebeat_bm3_state::sensor_r(offs_t offset)
{
	switch (offset)
	{
		case 0: return m_io[0]->read() | 0x0100;
		case 1: return m_io[1]->read() | 0x0100;
		case 2: return m_io[2]->read() | 0x0100;
		case 3: return m_io[3]->read() | 0x0100;
		case 5: return (m_io_turntables[0]->read() >> 8) | 0x0100;
		case 6: return (m_io_turntables[0]->read() & 0xff) | 0x0100;
		case 7: return (m_io_turntables[1]->read() >> 8) | 0x0100;
		case 8: return (m_io_turntables[1]->read() & 0xff) | 0x0100;
		case 9: return m_io_effects[0]->read() | 0x0100;
		case 10: return m_io_effects[1]->read() | 0x0100;
		case 11: return m_io_effects[2]->read() | 0x0100;
		case 12: return m_io_effects[3]->read() | 0x0100;
		case 13: return m_io_effects[4]->read() | 0x0100;
		case 14: return m_io_effects[5]->read() | 0x0100;
		case 15: return m_io_effects[6]->read() | 0x0100;
	}

	return 0;
}

void firebeat_bm3_state::midi_st224_irq_callback(int state)
{
	if (BIT(m_extend_board_irq_enable, 0) == 0 && state != CLEAR_LINE)
	{
		m_extend_board_irq_active |= 0x01;
		m_maincpu->set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
	}
	else
		m_maincpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
}

uint8_t firebeat_bm3_state::midi_uart_r(offs_t offset)
{
	return m_duart_midi->read(offset >> 6);
}

void firebeat_bm3_state::midi_uart_w(offs_t offset, uint8_t data)
{
	m_duart_midi->write(offset >> 6, data);
}

/*****************************************************************************
* pop'n music
******************************************************************************/
void firebeat_popn_state::firebeat_popn(machine_config &config)
{
	firebeat_spu_base(config);

	ATA_INTERFACE(config, m_spuata).options(firebeat_ata_devices, "dvdrom", nullptr, true);
	m_spuata->irq_handler().set(FUNC(firebeat_popn_state::spu_ata_interrupt));
	m_spuata->dmarq_handler().set(FUNC(firebeat_popn_state::spu_ata_dmarq));
	m_spuata->slot(0).set_option_machine_config("dvdrom", dvdrom_config);
	m_spuata->slot(0).set_fixed(true);

	// 500 hz works best for pop'n music.
	// Any lower and sometimes you'll hear buzzing from certain keysounds, or fades take too long.
	// Any higher and keysounds get cut short.
	TIMER(config, "spu_timer").configure_periodic(FUNC(firebeat_popn_state::spu_timer_callback), attotime::from_hz(500));

	// Effects audio channel, routed back to main (no external processing)
	m_rf5c400->add_route(2, "lspeaker", 0.5);
	m_rf5c400->add_route(3, "rspeaker", 0.5);
}

void firebeat_popn_state::init_popn_base()
{
	init_firebeat();
	init_lights(write32s_delegate(*this), write32s_delegate(*this), write32s_delegate(*this));
}

void firebeat_popn_state::init_popn_jp()
{
	init_popn_base();

	// Non-rental, JP data.
	m_cabinet_info = 0x0;
}

void firebeat_popn_state::init_popn_rental()
{
	init_popn_base();

	// Like all other firebeat types, bit 0 is the JP/overseas switch. Pop'n seems to use
	// bits 1 and 2 to designate a rental cabinet. Rental data does not seem to care about
	// the region bit, so will accept value 0x6 or 0x7. Arbitrarily choose "JP".
	m_cabinet_info = 0x6;
}


/*****************************************************************************
* ParaParaParadise / ParaParaDancing
******************************************************************************/
void firebeat_ppp_state::device_resolve_objects()
{
	firebeat_state::device_resolve_objects();
	m_stage_leds.resolve();
	m_top_leds.resolve();
	m_lamps.resolve();
	m_cab_led_left.resolve();
	m_cab_led_right.resolve();
	m_cab_led_door_lamp.resolve();
	m_cab_led_ok.resolve();
	m_cab_led_slim.resolve();
}

void firebeat_ppp_state::firebeat_ppp(machine_config &config)
{
	firebeat(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &firebeat_ppp_state::firebeat_ppp_map);
}

void firebeat_ppp_state::init_ppp_base()
{
	init_firebeat();
	init_lights(write32s_delegate(*this, FUNC(firebeat_ppp_state::lamp_output_ppp_w)), write32s_delegate(*this, FUNC(firebeat_ppp_state::lamp_output2_ppp_w)), write32s_delegate(*this, FUNC(firebeat_ppp_state::lamp_output3_ppp_w)));
}

void firebeat_ppp_state::init_ppp_jp()
{
	init_ppp_base();

	// PPP Seems to use bit 3 to specify a PPP-compatible firebeat. It still uses
	// bit 0 to represent JP/overseas. We specify JP data here.
	m_cabinet_info = 0x8;
}

void firebeat_ppp_state::init_ppp_overseas()
{
	init_ppp_base();

	// We specify bit 3 set here as well to make this firebeat compatible with
	// PPP data. We also set the overseas bit for Korea/Asia region games.
	m_cabinet_info = 0x9;
}

void firebeat_ppp_state::firebeat_ppp_map(address_map &map)
{
	firebeat_map(map);
	map(0x7d000340, 0x7d00035f).r(FUNC(firebeat_ppp_state::sensor_r));
}

uint16_t firebeat_ppp_state::sensor_r(offs_t offset)
{
	switch (offset)
	{
		case 0: return m_io_sensors[0]->read() | 0x0100;
		case 1: return m_io_sensors[1]->read() | 0x0100;
		case 2: return m_io_sensors[2]->read() | 0x0100;
		case 3: return m_io_sensors[3]->read() | 0x0100;
	}

	return 0;
}

void firebeat_ppp_state::lamp_output_ppp_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	lamp_output_w(offset, data, mem_mask);

	// ParaParaParadise lamps (active high)
	// 0x00000100 Left
	// 0x00000200 Right
	// 0x00000400 Door Lamp
	// 0x00000800 OK
	// 0x00008000 Slim
	// 0x01000000 Stage LED 0
	// 0x02000000 Stage LED 1
	// 0x04000000 Stage LED 2
	// 0x08000000 Stage LED 3
	// 0x00010000 Stage LED 4
	// 0x00020000 Stage LED 5
	// 0x00040000 Stage LED 6
	// 0x00080000 Stage LED 7
	if (ACCESSING_BITS_8_15)
	{
		m_cab_led_left = BIT(data, 8);
		m_cab_led_right = BIT(data, 9);
		m_cab_led_door_lamp = BIT(data, 10);
		m_cab_led_ok = BIT(data, 11);
		m_cab_led_slim = BIT(data, 15);
	}
	if (ACCESSING_BITS_24_31)
	{
		m_stage_leds[0] = BIT(data, 24);
		m_stage_leds[1] = BIT(data, 25);
		m_stage_leds[2] = BIT(data, 26);
		m_stage_leds[3] = BIT(data, 27);
	}
	if (ACCESSING_BITS_16_23)
	{
		m_stage_leds[4] = BIT(data, 16);
		m_stage_leds[5] = BIT(data, 17);
		m_stage_leds[6] = BIT(data, 18);
		m_stage_leds[7] = BIT(data, 19);
	}
}

void firebeat_ppp_state::lamp_output2_ppp_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	lamp_output2_w(offset, data, mem_mask);

	// ParaParaParadise lamps (active high)
	// 0x00010000 Top LED 0
	// 0x00020000 Top LED 1
	// 0x00040000 Top LED 2
	// 0x00080000 Top LED 3
	// 0x00000001 Top LED 4
	// 0x00000002 Top LED 5
	// 0x00000004 Top LED 6
	// 0x00000008 Top LED 7
	if (ACCESSING_BITS_16_23)
	{
		m_top_leds[0] = BIT(data, 16);
		m_top_leds[1] = BIT(data, 17);
		m_top_leds[2] = BIT(data, 18);
		m_top_leds[3] = BIT(data, 19);
	}
	if (ACCESSING_BITS_0_7)
	{
		m_top_leds[4] = BIT(data, 0);
		m_top_leds[5] = BIT(data, 1);
		m_top_leds[6] = BIT(data, 2);
		m_top_leds[7] = BIT(data, 3);
	}
}

void firebeat_ppp_state::lamp_output3_ppp_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	lamp_output3_w(offset, data, mem_mask);

	// ParaParaParadise lamps (active high)
	// 0x00010000 Lamp 0
	// 0x00040000 Lamp 1
	// 0x00100000 Lamp 2
	// 0x00400000 Lamp 3
	if (ACCESSING_BITS_16_23)
	{
		m_lamps[0] = BIT(data, 16);
		m_lamps[1] = BIT(data, 18);
		m_lamps[2] = BIT(data, 20);
		m_lamps[3] = BIT(data, 22);
	}
}


/*****************************************************************************
* Keyboardmania
******************************************************************************/
void firebeat_kbm_state::device_resolve_objects()
{
	firebeat_state::device_resolve_objects();
	m_lamps.resolve();
	m_cab_led_door_lamp.resolve();
	m_cab_led_start1p.resolve();
	m_cab_led_start2p.resolve();
	m_lamp_neon.resolve();
}

void firebeat_kbm_state::init_kbm_base()
{
	init_firebeat();
	init_lights(write32s_delegate(*this, FUNC(firebeat_kbm_state::lamp_output_kbm_w)), write32s_delegate(*this), write32s_delegate(*this));
	init_keyboard();

//  pc16552d_init(machine(), 1, 24000000, midi_uart_irq_callback, 0);     // MIDI UART
}

void firebeat_kbm_state::init_kbm_jp()
{
	init_kbm_base();

	// KBM Seems to use bit 1 to specify a KBM-compatible firebeat. It still uses
	// bit 0 to represent JP/overseas. We specify JP data here.
	m_cabinet_info = 0x2;
}

void firebeat_kbm_state::init_kbm_overseas()
{
	init_kbm_base();

	// We specify bit 1 set here as well to make this firebeat compatible with
	// KBM data. We also set the overseas bit for Korea/Asia region games.
	m_cabinet_info = 0x3;
}

void firebeat_kbm_state::init_keyboard()
{
	// set keyboard timer
//  m_keyboard_timer = timer_alloc(FUNC(firebeat_state::keyboard_timer_callback), this);
//  m_keyboard_timer->adjust(attotime::from_msec(10), 0, attotime::from_msec(10));
}

void firebeat_kbm_state::firebeat_kbm(machine_config &config)
{
	/* basic machine hardware */
	PPC403GCX(config, m_maincpu, XTAL(66'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &firebeat_kbm_state::firebeat_kbm_map);

	RTC65271(config, "rtc", 0);

	FUJITSU_29F016A(config, "flash_main");
	FUJITSU_29F016A(config, "flash_snd1");
	FUJITSU_29F016A(config, "flash_snd2");

	ATA_INTERFACE(config, m_ata).options(firebeat_ata_devices, "cdrom", "cdrom", true);
	m_ata->irq_handler().set(FUNC(firebeat_kbm_state::ata_interrupt));
	m_ata->slot(1).set_option_machine_config("cdrom", cdrom_config);
	m_ata->slot(1).set_fixed(true);

	/* video hardware */
	PALETTE(config, "palette", palette_device::RGB_555);

	screen_device &lscreen(SCREEN(config, "lscreen", SCREEN_TYPE_RASTER));
	lscreen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 525, 0, 480);
	lscreen.set_screen_update(FUNC(firebeat_kbm_state::screen_update_firebeat_0));
	lscreen.set_palette("palette");
	lscreen.screen_vblank().set(m_gcu, FUNC(k057714_device::vblank_w));

	K057714(config, m_gcu, 0).set_screen("lscreen");
	m_gcu->irq_callback().set(FUNC(firebeat_kbm_state::gcu_interrupt));

	screen_device &rscreen(SCREEN(config, "rscreen", SCREEN_TYPE_RASTER));
	rscreen.set_raw(25.175_MHz_XTAL, 800, 0, 640, 525, 0, 480);
	rscreen.set_screen_update(FUNC(firebeat_kbm_state::screen_update_firebeat_1));
	rscreen.set_palette("palette");

	K057714(config, m_gcu_sub, 0).set_screen("rscreen");
	m_gcu_sub->irq_callback().set(FUNC(firebeat_kbm_state::gcu_interrupt));

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ymz280b_device &ymz(YMZ280B(config, "ymz", 16934400));
	ymz.irq_handler().set(FUNC(firebeat_kbm_state::sound_irq_callback));
	ymz.set_addrmap(0, &firebeat_kbm_state::ymz280b_map);
	ymz.add_route(1, "lspeaker", 1.0);
	ymz.add_route(0, "rspeaker", 1.0);

	// On the main PCB
	PC16552D(config, "duart_com", 0);
	NS16550(config, "duart_com:chan0", XTAL(19'660'800));
	NS16550(config, "duart_com:chan1", XTAL(19'660'800));

	// On the extend board
	PC16552D(config, m_duart_midi, 0);
	auto &midi_chan1(NS16550(config, "duart_midi:chan1", XTAL(24'000'000)));
	MIDI_KBD(config, m_kbd[0], 31250).tx_callback().set(midi_chan1, FUNC(ins8250_uart_device::rx_w));
	midi_chan1.out_int_callback().set(FUNC(firebeat_kbm_state::midi_keyboard_left_irq_callback));

	auto &midi_chan0(NS16550(config, "duart_midi:chan0", XTAL(24'000'000)));
	MIDI_KBD(config, m_kbd[1], 31250).tx_callback().set(midi_chan0, FUNC(ins8250_uart_device::rx_w));
	midi_chan0.out_int_callback().set(FUNC(firebeat_kbm_state::midi_keyboard_right_irq_callback));

	// Synth card
	auto &xt446(XT446(config, "xt446"));
	midi_chan1.out_tx_callback().set(xt446, FUNC(xt446_device::midi_w));
	xt446.add_route(0, "lspeaker", 1.0);
	xt446.add_route(1, "rspeaker", 1.0);
}

void firebeat_kbm_state::firebeat_kbm_map(address_map &map)
{
	firebeat_map(map);
	map(0x70000000, 0x70000fff).rw(FUNC(firebeat_kbm_state::midi_uart_r), FUNC(firebeat_kbm_state::midi_uart_w)).umask32(0xff000000);
	map(0x70008000, 0x7000800f).r(FUNC(firebeat_kbm_state::keyboard_wheel_r));
	map(0x7e800100, 0x7e8001ff).rw(m_gcu_sub, FUNC(k057714_device::read), FUNC(k057714_device::write));
}

uint8_t firebeat_kbm_state::keyboard_wheel_r(offs_t offset)
{
	switch (offset)
	{
		case 0: return m_io_wheels[0]->read(); // Keyboard Wheel (P1)
		case 8: return m_io_wheels[1]->read(); // Keyboard Wheel (P2)
	}

	return 0;
}

uint8_t firebeat_kbm_state::midi_uart_r(offs_t offset)
{
	return m_duart_midi->read(offset >> 6);
}

void firebeat_kbm_state::midi_uart_w(offs_t offset, uint8_t data)
{
	m_duart_midi->write(offset >> 6, data);
}

void firebeat_kbm_state::midi_keyboard_right_irq_callback(int state)
{
	if (BIT(m_extend_board_irq_enable, 1) == 0 && state != CLEAR_LINE)
	{
		m_extend_board_irq_active |= 0x02;
		m_maincpu->set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
	}
	else
		m_maincpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
}

void firebeat_kbm_state::midi_keyboard_left_irq_callback(int state)
{
	if (BIT(m_extend_board_irq_enable, 0) == 0 && state != CLEAR_LINE)
	{
		m_extend_board_irq_active |= 0x01;
		m_maincpu->set_input_line(INPUT_LINE_IRQ1, ASSERT_LINE);
	}
	else
		m_maincpu->set_input_line(INPUT_LINE_IRQ1, CLEAR_LINE);
}

/*
static const int keyboard_notes[24] =
{
    0x3c,   // C1
    0x3d,   // C1#
    0x3e,   // D1
    0x3f,   // D1#
    0x40,   // E1
    0x41,   // F1
    0x42,   // F1#
    0x43,   // G1
    0x44,   // G1#
    0x45,   // A1
    0x46,   // A1#
    0x47,   // B1
    0x48,   // C2
    0x49,   // C2#
    0x4a,   // D2
    0x4b,   // D2#
    0x4c,   // E2
    0x4d,   // F2
    0x4e,   // F2#
    0x4f,   // G2
    0x50,   // G2#
    0x51,   // A2
    0x52,   // A2#
    0x53,   // B2
};

TIMER_CALLBACK_MEMBER(firebeat_kbm_state::keyboard_timer_callback)
{
    static const int kb_uart_channel[2] = { 1, 0 };
    static const char *const keynames[] = { "KEYBOARD_P1", "KEYBOARD_P2" };
    int keyboard;
    int i;

    for (keyboard=0; keyboard < 2; keyboard++)
    {
        uint32_t kbstate = ioport(keynames[keyboard])->read();
        int uart_channel = kb_uart_channel[keyboard];

        if (kbstate != m_keyboard_state[keyboard])
        {
            for (i=0; i < 24; i++)
            {
                int kbnote = keyboard_notes[i];

                if ((m_keyboard_state[keyboard] & (1 << i)) != 0 && (kbstate & (1 << i)) == 0)
                {
                    // key was on, now off -> send Note Off message
                    pc16552d_rx_data(machine(), 1, uart_channel, 0x80);
                    pc16552d_rx_data(machine(), 1, uart_channel, kbnote);
                    pc16552d_rx_data(machine(), 1, uart_channel, 0x7f);
                }
                else if ((m_keyboard_state[keyboard] & (1 << i)) == 0 && (kbstate & (1 << i)) != 0)
                {
                    // key was off, now on -> send Note On message
                    pc16552d_rx_data(machine(), 1, uart_channel, 0x90);
                    pc16552d_rx_data(machine(), 1, uart_channel, kbnote);
                    pc16552d_rx_data(machine(), 1, uart_channel, 0x7f);
                }
            }
        }
        else
        {
            // no messages, send Active Sense message instead
            pc16552d_rx_data(machine(), 1, uart_channel, 0xfe);
        }

        m_keyboard_state[keyboard] = kbstate;
    }
}
*/

void firebeat_kbm_state::lamp_output_kbm_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	lamp_output_w(offset, data, mem_mask);

	if (ACCESSING_BITS_24_31)
	{
		m_cab_led_door_lamp = BIT(data, 28);
		m_cab_led_start1p = BIT(data, 24);
		m_cab_led_start2p = BIT(data, 25);
	}
	if (ACCESSING_BITS_8_15)
	{
		m_lamps[0] = BIT(data, 8);
		m_lamps[1] = BIT(data, 9);
		m_lamps[2] = BIT(data, 10);
		m_lamp_neon = BIT(data, 11);
	}
}

/*****************************************************************************/

static INPUT_PORTS_START( firebeat )
	PORT_START("IN3")
	// popn4/animelo:
	//   DIPSW 1 = Auto play mode
	// popn5:
	//   DIPSW 5? = Network communication debug messages
	// popn6/popn7/popn8/others?: Requires debug type dongle (not included in ROMs)
	//   DIPSW 8 = Auto play mode
	//   DIPSW 6 = Mute song BGM
	// kbm3rd: Set to 0xDE (UUdUUUUd) pattern to enable debug view mode
	PORT_DIPUNKNOWN_DIPLOC( 0x01, IP_ACTIVE_LOW, "DIP SW:8" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, IP_ACTIVE_LOW, "DIP SW:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, IP_ACTIVE_LOW, "DIP SW:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, IP_ACTIVE_LOW, "DIP SW:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "DIP SW:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, IP_ACTIVE_LOW, "DIP SW:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, IP_ACTIVE_LOW, "DIP SW:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, IP_ACTIVE_LOW, "DIP SW:1" )

	PORT_START("IN1")
	// Only read by pop'n music?
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

INPUT_PORTS_END

static INPUT_PORTS_START( firebeat_spu )
	PORT_START("SPU_DSW")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, IP_ACTIVE_LOW, "SPU DSW:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, IP_ACTIVE_LOW, "SPU DSW:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, IP_ACTIVE_LOW, "SPU DSW:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, IP_ACTIVE_LOW, "SPU DSW:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "SPU DSW:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, IP_ACTIVE_LOW, "SPU DSW:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, IP_ACTIVE_LOW, "SPU DSW:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, IP_ACTIVE_LOW, "SPU DSW:8" )
INPUT_PORTS_END

static INPUT_PORTS_START(ppp)
	PORT_INCLUDE( firebeat )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )            // Left
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )            // Right
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW)            // Test
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service") PORT_CODE(KEYCODE_7)      // Service
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )              // Coin
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )             // Start / Ok
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )             // Fixes booting in PPP with certain dongle types
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// ParaParaParadise has 24 sensors, grouped into groups of 3 for each sensor bar
	// Sensors 15...23 are only used by the Korean version of PPP, which has 8 sensor bars

	PORT_START("SENSOR1")
	PORT_BIT( 0x0007, IP_ACTIVE_HIGH, IPT_BUTTON3 )     // Sensor 0, 1, 2  (Sensor bar 1)
	PORT_BIT( 0x0038, IP_ACTIVE_HIGH, IPT_BUTTON4 )     // Sensor 3, 4, 5  (Sensor bar 2)
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_BUTTON5 )     // Sensor 6, 7, 8  (Sensor bar 3)

	PORT_START("SENSOR2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON5 )     // Sensor 6, 7, 8  (Sensor bar 3)
	PORT_BIT( 0x000e, IP_ACTIVE_HIGH, IPT_BUTTON6 )     // Sensor 9, 10,11 (Sensor bar 4)

	PORT_START("SENSOR3")
	PORT_BIT( 0x0007, IP_ACTIVE_HIGH, IPT_BUTTON7 )     // Sensor 12,13,14 (Sensor bar 5)
	PORT_BIT( 0x0038, IP_ACTIVE_HIGH, IPT_BUTTON8 )     // Sensor 15,16,17 (Sensor bar 6)   (unused by PPP)
	PORT_BIT( 0x00c0, IP_ACTIVE_HIGH, IPT_BUTTON9 )     // Sensor 18,19,20 (Sensor bar 7)   (unused by PPP)

	PORT_START("SENSOR4")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_BUTTON9 )     // Sensor 18,19,20 (Sensor bar 7)   (unused by PPP)
	PORT_BIT( 0x000e, IP_ACTIVE_HIGH, IPT_BUTTON10 )    // Sensor 21,22,23 (Sensor bar 8)   (unused by PPP)

INPUT_PORTS_END

static INPUT_PORTS_START(kbm)
	PORT_INCLUDE( firebeat )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )             // Start P1
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )             // Start P2
	PORT_SERVICE_NO_TOGGLE( 0x04, IP_ACTIVE_LOW)            // Test
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service") PORT_CODE(KEYCODE_7)      // Service
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )              // Coin
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )           // e-Amusement
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )            // e-Amusement (Keyboardmania)
	PORT_BIT( 0xde, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("WHEEL_P1")          // Keyboard modulation wheel (P1)
	PORT_BIT( 0xff, 0x80, IPT_PADDLE ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)

	PORT_START("WHEEL_P2")          // Keyboard modulation wheel (P2)
	PORT_BIT( 0xff, 0x80, IPT_PADDLE_V ) PORT_MINMAX(0x00, 0xff) PORT_SENSITIVITY(30) PORT_KEYDELTA(10)

/*
    PORT_START("KEYBOARD_P1")
    PORT_BIT( 0x000001, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 C1") PORT_CODE(KEYCODE_Q)
    PORT_BIT( 0x000002, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 C1#") PORT_CODE(KEYCODE_W)
    PORT_BIT( 0x000004, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 D1") PORT_CODE(KEYCODE_E)
    PORT_BIT( 0x000008, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 D1#") PORT_CODE(KEYCODE_R)
    PORT_BIT( 0x000010, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 E1") PORT_CODE(KEYCODE_T)
    PORT_BIT( 0x000020, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 F1") PORT_CODE(KEYCODE_Y)
    PORT_BIT( 0x000040, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 F1#") PORT_CODE(KEYCODE_U)
    PORT_BIT( 0x000080, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 G1") PORT_CODE(KEYCODE_I)
    PORT_BIT( 0x000100, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 G1#") PORT_CODE(KEYCODE_O)
    PORT_BIT( 0x000200, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 A1") PORT_CODE(KEYCODE_A)
    PORT_BIT( 0x000400, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 A1#") PORT_CODE(KEYCODE_S)
    PORT_BIT( 0x000800, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 B1") PORT_CODE(KEYCODE_D)
    PORT_BIT( 0x001000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 C2") PORT_CODE(KEYCODE_F)
    PORT_BIT( 0x002000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 C2#") PORT_CODE(KEYCODE_G)
    PORT_BIT( 0x004000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 D2") PORT_CODE(KEYCODE_H)
    PORT_BIT( 0x008000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 D2#") PORT_CODE(KEYCODE_J)
    PORT_BIT( 0x010000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 E2") PORT_CODE(KEYCODE_K)
    PORT_BIT( 0x020000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 F2") PORT_CODE(KEYCODE_L)
    PORT_BIT( 0x040000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 F2#") PORT_CODE(KEYCODE_Z)
    PORT_BIT( 0x080000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 G2") PORT_CODE(KEYCODE_X)
    PORT_BIT( 0x100000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 G2#") PORT_CODE(KEYCODE_C)
    PORT_BIT( 0x200000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 A2") PORT_CODE(KEYCODE_V)
    PORT_BIT( 0x400000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 A2#") PORT_CODE(KEYCODE_B)
    PORT_BIT( 0x800000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P1 B2") PORT_CODE(KEYCODE_N)

    PORT_START("KEYBOARD_P2")
    PORT_BIT( 0x000001, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 C1") PORT_CODE(KEYCODE_Q)
    PORT_BIT( 0x000002, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 C1#") PORT_CODE(KEYCODE_W)
    PORT_BIT( 0x000004, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 D1") PORT_CODE(KEYCODE_E)
    PORT_BIT( 0x000008, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 D1#") PORT_CODE(KEYCODE_R)
    PORT_BIT( 0x000010, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 E1") PORT_CODE(KEYCODE_T)
    PORT_BIT( 0x000020, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 F1") PORT_CODE(KEYCODE_Y)
    PORT_BIT( 0x000040, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 F1#") PORT_CODE(KEYCODE_U)
    PORT_BIT( 0x000080, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 G1") PORT_CODE(KEYCODE_I)
    PORT_BIT( 0x000100, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 G1#") PORT_CODE(KEYCODE_O)
    PORT_BIT( 0x000200, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 A1") PORT_CODE(KEYCODE_A)
    PORT_BIT( 0x000400, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 A1#") PORT_CODE(KEYCODE_S)
    PORT_BIT( 0x000800, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 B1") PORT_CODE(KEYCODE_D)
    PORT_BIT( 0x001000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 C2") PORT_CODE(KEYCODE_F)
    PORT_BIT( 0x002000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 C2#") PORT_CODE(KEYCODE_G)
    PORT_BIT( 0x004000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 D2") PORT_CODE(KEYCODE_H)
    PORT_BIT( 0x008000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 D2#") PORT_CODE(KEYCODE_J)
    PORT_BIT( 0x010000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 E2") PORT_CODE(KEYCODE_K)
    PORT_BIT( 0x020000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 F2") PORT_CODE(KEYCODE_L)
    PORT_BIT( 0x040000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 F2#") PORT_CODE(KEYCODE_Z)
    PORT_BIT( 0x080000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 G2") PORT_CODE(KEYCODE_X)
    PORT_BIT( 0x100000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 G2#") PORT_CODE(KEYCODE_C)
    PORT_BIT( 0x200000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 A2") PORT_CODE(KEYCODE_V)
    PORT_BIT( 0x400000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 A2#") PORT_CODE(KEYCODE_B)
    PORT_BIT( 0x800000, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("P2 B2") PORT_CODE(KEYCODE_N)
*/
INPUT_PORTS_END

static INPUT_PORTS_START(popn)
	PORT_INCLUDE( firebeat )
	PORT_INCLUDE( firebeat_spu )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 )            // Switch 1
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 )            // Switch 2
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 )            // Switch 3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 )            // Switch 4
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )            // Switch 5
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 )            // Switch 6
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 )            // Switch 7
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON8 )            // Switch 8

	PORT_MODIFY("IN1")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_UNKNOWN )            // vwatch, some kind of voltage check
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )            // When combined with a debug dongle, this will disable songs from working. Debug switch?

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON9 )            // Switch 9
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )              // Coin
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_SERVICE_NO_TOGGLE( 0x10, IP_ACTIVE_LOW)            // Test
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service") PORT_CODE(KEYCODE_7)      // Service
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

INPUT_PORTS_END

static INPUT_PORTS_START(bm3)
	PORT_INCLUDE( firebeat )
	PORT_INCLUDE( firebeat_spu )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE) PORT_NAME(DEF_STR(Test))              // Test
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Service")                // Service
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("A")                      // A
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("B")                      // B
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(1) PORT_NAME("P1 Foot") // P1 Foot Pedal
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(2) PORT_NAME("P2 Foot") // P2 Foot Pedal

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IO1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) // P1 Button 1
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) // P1 Button 2
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) // P1 Button 3
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1) // P1 Button 4
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1) // P1 Button 5
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START1 )                 // P1 Start Button

	PORT_START("IO2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 ) // Coin sensor

	PORT_START("IO3")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) // P2 Button 1
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) // P2 Button 2
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) // P2 Button 3
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2) // P2 Button 4
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2) // P2 Button 5
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_START2 )                 // P2 Start Button

	PORT_START("IO4")
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("TURNTABLE_P1")
	PORT_BIT( 0x03ff, 0x00, IPT_TRACKBALL_X) PORT_PLAYER(1) PORT_NAME("Turntable") PORT_MINMAX(0x00,0x3ff) PORT_SENSITIVITY(100) PORT_KEYDELTA(15)

	PORT_START("TURNTABLE_P2")
	PORT_BIT( 0x03ff, 0x00, IPT_TRACKBALL_X) PORT_PLAYER(2) PORT_NAME("Turntable") PORT_MINMAX(0x00,0x3ff) PORT_SENSITIVITY(100) PORT_KEYDELTA(15)

	PORT_START("EFFECT1")
	PORT_BIT( 0x001f, 0x00, IPT_DIAL) PORT_NAME("Effect 1") PORT_MINMAX(0x00,0x1f) PORT_SENSITIVITY(10) PORT_KEYDELTA(1)

	PORT_START("EFFECT2")
	PORT_BIT( 0x001f, 0x00, IPT_DIAL) PORT_NAME("Effect 2") PORT_MINMAX(0x00,0x1f) PORT_SENSITIVITY(10) PORT_KEYDELTA(1)

	PORT_START("EFFECT3")
	PORT_BIT( 0x001f, 0x00, IPT_DIAL) PORT_NAME("Effect 3") PORT_MINMAX(0x00,0x1f) PORT_SENSITIVITY(10) PORT_KEYDELTA(1)

	PORT_START("EFFECT4")
	PORT_BIT( 0x001f, 0x00, IPT_DIAL) PORT_NAME("Effect 4") PORT_MINMAX(0x00,0x1f) PORT_SENSITIVITY(10) PORT_KEYDELTA(1)

	PORT_START("EFFECT5")
	PORT_BIT( 0x001f, 0x00, IPT_DIAL) PORT_NAME("Effect 5") PORT_MINMAX(0x00,0x1f) PORT_SENSITIVITY(10) PORT_KEYDELTA(1)

	PORT_START("EFFECT6")
	PORT_BIT( 0x001f, 0x00, IPT_DIAL) PORT_NAME("Effect 6") PORT_MINMAX(0x00,0x1f) PORT_SENSITIVITY(10) PORT_KEYDELTA(1)

	PORT_START("EFFECT7")
	PORT_BIT( 0x001f, 0x00, IPT_DIAL) PORT_NAME("Effect 7") PORT_MINMAX(0x00,0x1f) PORT_SENSITIVITY(10) PORT_KEYDELTA(1)

INPUT_PORTS_END

/*****************************************************************************/

ROM_START( ppp )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("977jaa03.21e", 0x00000, 0x80000, CRC(7b83362a) SHA1(2857a93be58636c10a8d180dbccf2caeeaaff0e2))

	ROM_REGION(0xc8, "user2", 0)    // Security dongle
	ROM_LOAD("gq977", 0x00, 0xc8, CRC(1cf40267) SHA1(28269e30a05c7334955c02aaa8e233d7e53842d9))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "977jaa01", 0, BAD_DUMP SHA1(59c03d8eb366167feef741d42d9d8b54bfeb3c1e) )

	DISK_REGION( "ata:1:cdrom" ) // audio CD-ROM
	DISK_IMAGE_READONLY( "977jaa02", 0, SHA1(bd07c25ee3e1edc962997f6a5bb1700897231fb2) )
ROM_END

ROM_START( ppp1mp )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("977jaa03.21e", 0x00000, 0x80000, CRC(7b83362a) SHA1(2857a93be58636c10a8d180dbccf2caeeaaff0e2))

	ROM_REGION(0xc0, "user2", 0)    // Security dongle
	ROM_LOAD("gqa11-ja", 0x00, 0xc0, BAD_DUMP CRC(207a99b2) SHA1(d19788e1c377771141527660311ff84653039c32))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "a11jaa01", 0, SHA1(539ec6f1c1d198b0d6ce5543eadcbb4d9917fa42) )

	DISK_REGION( "ata:1:cdrom" ) // audio CD-ROM
	DISK_IMAGE_READONLY( "a11jaa02", 0, SHA1(575069570cb4a2b58b199a1329d45b189a20fcc9) )
ROM_END

ROM_START( ppd )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("977jaa03.21e", 0x00000, 0x80000, CRC(7b83362a) SHA1(2857a93be58636c10a8d180dbccf2caeeaaff0e2))

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gq977-ko", 0x00, 0xc0, BAD_DUMP CRC(b275de3c) SHA1(d23fcf0e87da2e561bc112851d26b3e78079f40a))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "977kaa01", 0, BAD_DUMP SHA1(7af9f4949ffa10ea5fc18b6c88c2abc710df3cf9) )

	DISK_REGION( "ata:1:cdrom" ) // audio CD-ROM
	DISK_IMAGE_READONLY( "977kaa02", 1, SHA1(0feb5ac56269ad4a8401fcfe3bb98b01a0169177) )
ROM_END

ROM_START( ppp11 )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("977jaa03.21e", 0x00000, 0x80000, CRC(7b83362a) SHA1(2857a93be58636c10a8d180dbccf2caeeaaff0e2))

	ROM_REGION(0xc8, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gq977", 0x00, 0xc8, CRC(1cf40267) SHA1(28269e30a05c7334955c02aaa8e233d7e53842d9))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "gc977jaa01", 0, SHA1(7ed1f4b55105c93fec74468436bfb1d540bce944) )

	DISK_REGION( "ata:1:cdrom" ) // audio CD-ROM
	DISK_IMAGE_READONLY( "gc977jaa02", 1, SHA1(74ce8c90575fd562807def7d561392d0f91f2bc6) )
ROM_END

ROM_START( kbm )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("974a03.21e", 0x00000, 0x80000, CRC(ef9a932d) SHA1(6299d3b9823605e519dbf1f105b59a09197df72f))

	ROM_REGION(0xc8, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gq974", 0x00, 0xc8, CRC(65e4886a) SHA1(afba0315f2532599c51e232f734c538c4d108d73))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "gq974-ja c01", 0, SHA1(975a4a59f842b8a7edad79b307e489cc88bef24d) )

	DISK_REGION( "ata:1:cdrom" ) // audio CD-ROM
	DISK_IMAGE_READONLY( "gq974-ja a02", 1, SHA1(80086676c00c9ca06ec14e305ea4523b6576e47f) )
ROM_END

ROM_START( kbh )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("974a03.21e", 0x00000, 0x80000, CRC(ef9a932d) SHA1(6299d3b9823605e519dbf1f105b59a09197df72f))

	ROM_REGION(0xc8, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gu974", 0x00, 0xc8, CRC(748b8476) SHA1(5d507fd46235c4315ad32599ce87aa4e06642eb5))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "gu974-ka a01", 0, SHA1(07d3d6abcb13b2c2a556f2eed7e89e3d11febf1b) )

	DISK_REGION( "ata:1:cdrom" ) // audio CD-ROM
	DISK_IMAGE_READONLY( "gu974-ka a02", 1, SHA1(9e358b0551b650a432e685ec82d3df2433e2aac3) )
ROM_END

ROM_START( kbm2nd )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("974a03.21e", 0x00000, 0x80000, CRC(ef9a932d) SHA1(6299d3b9823605e519dbf1f105b59a09197df72f))

	ROM_REGION(0xc8, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gca01ja_gca01aa", 0x00, 0xc8, CRC(27f977cf) SHA1(14739cb4edfc3c4453673d59f2bd0442eab71d6a))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "a01 ja a01", 0, SHA1(6a661dd737c83130febe771402a159859afeffba) )

	DISK_REGION( "ata:1:cdrom" ) // audio CD-ROM
	DISK_IMAGE_READONLY( "a01 ja a02", 1, SHA1(e1ffc0bd4ea169951ed9ceaf090dbb1511a46601) )
ROM_END

ROM_START( kbm3rd )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("974a03.21e", 0x00000, 0x80000, CRC(ef9a932d) SHA1(6299d3b9823605e519dbf1f105b59a09197df72f))

	ROM_REGION(0xc8, "user2", 0)    // Security dongle
	ROM_LOAD("gca12-ja_gca12-aa.bin", 0x00, 0xc8, CRC(96b12482) SHA1(199f20d9fa53108b3fe02d91d6793af1554b0f6f))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "a12jaa01", 0, SHA1(ea30bf1273bce772f09063bfc8a74df360c743a7) )

	DISK_REGION( "ata:1:cdrom" ) // audio CD-ROM
	DISK_IMAGE_READONLY( "a12jaa02", 0, SHA1(6bf7adbd637a0ce0c19b57187d3e46fabea99363) )
ROM_END

ROM_START( popn4 )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("a02jaa03.21e", 0x00000, 0x80000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599))

	ROM_REGION(0xc8, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gq986_gc986", 0x00, 0xc8, CRC(df7935c4) SHA1(ee0eafb801097f06b556a393d9b139aff9e5106d))

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a02jaa04.3q", 0x00000, 0x80000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "gq986jaa01", 0, SHA1(e5368ac029b0bdf29943ae66677b5521ae1176e1) )

	DISK_REGION( "spu_ata:0:dvdrom" ) // data DVD-ROM
	DISK_IMAGE( "gq986jaa02", 0, SHA1(c34ac216b3e0bef1d1813119469364c6403feaa4) )

	ROM_REGION(0x1038, "rtc", ROMREGION_ERASE00)    // Default unlocked RTC
	ROM_LOAD("rtc", 0x0000, 0x1038, CRC(4a5c946c) SHA1(9de6085d45c39ba91934cea3abaa37e1203888c7))
ROM_END

ROM_START( popn5 )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP( "a02jaa03.21e", 0x000000, 0x080000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599) )

	ROM_REGION(0xc8, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gea04-ja_gca04-ja_gca04-jb.bin", 0x00, 0xc8, CRC(f48f62f7) SHA1(e28b3fd41fa4136cf9c271967d0e68f21a67ba1a))

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP( "a02jaa04.3q",  0x000000, 0x080000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683) )

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "a04jaa01", 0, SHA1(87136ddad1d786b4d5f04381fcbf679ab666e6c9) )

	DISK_REGION( "spu_ata:0:dvdrom" ) // data DVD-ROM
	DISK_IMAGE_READONLY( "a04jaa02", 0, SHA1(058167a6ac910183a701920021cfbc0933428e97) )

	ROM_REGION(0x1038, "rtc", ROMREGION_ERASE00)    // Default unlocked RTC
	ROM_LOAD("rtc", 0x0000, 0x1038, CRC(adeba6fc) SHA1(a2266696bb0a68e2b70a07d580a3b471e72fa587))
ROM_END

ROM_START( popn6 )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("a02jaa03.21e", 0x00000, 0x80000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599))

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gqa16-ja", 0x00, 0xc0, BAD_DUMP CRC(f2094180) SHA1(36559307f73fe4d9e21533041e8ff8f9297773ed))

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a02jaa04.3q", 0x00000, 0x80000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "gqa16jaa01", 0, SHA1(7a7e475d06c74a273f821fdfde0743b33d566e4c) )

	DISK_REGION( "spu_ata:0:dvdrom" ) // data DVD-ROM
	DISK_IMAGE( "gqa16jaa02", 0, SHA1(18abf1a9dbf61faebd44c8dc1d6decbaaca826a2) )

	ROM_REGION(0x1038, "rtc", ROMREGION_ERASE00)    // Default unlocked RTC
	ROM_LOAD("rtc", 0x0000, 0x1038, CRC(9935427c) SHA1(f7095ea6360ca61d1e2914cf184e50e50777a168))
ROM_END

ROM_START( popn7 )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("a02jaa03.21e", 0x00000, 0x80000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599))

	ROM_REGION(0xc8, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gcb00", 0x00, 0xc8, CRC(fbc3f534) SHA1(914e68b85a8c8cab219909fbf88ab96bdee123db))

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a02jaa04.3q", 0x00000, 0x80000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "b00jab01", 0, SHA1(259c733ca4d30281205b46b7bf8d60c9d01aa818) )

	DISK_REGION( "spu_ata:0:dvdrom" ) // data DVD-ROM
	DISK_IMAGE_READONLY( "b00jaa02", 0, SHA1(43201334acb20f529baa50c24494b7f0a4bf3d0d) )

	ROM_REGION(0x1038, "rtc", ROMREGION_ERASE00)    // Default unlocked RTC
	ROM_LOAD("rtc", 0x0000, 0x1038, CRC(fce30919) SHA1(9f875f5fe6ab6591ec024afc0a91966befa73ede))
ROM_END

ROM_START( popn8 )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("a02jaa03.21e", 0x00000, 0x80000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599))

	ROM_REGION(0xc8, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gcb30-ja_gcb30-jb", 0x00, 0xc8, CRC(5bcd854f) SHA1(8dfd88ae3156a6d6c6eddd5fdc09256ee72425fa))

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a02jaa04.3q", 0x00000, 0x80000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "gqb30jaa01", 0, SHA1(0ff3e40e3717ce23337b3a2438bdaca01cba9e30) )

	DISK_REGION( "spu_ata:0:dvdrom" ) // data DVD-ROM
	DISK_IMAGE_READONLY( "gqb30jaa02", 0, SHA1(69d26af2bd85a5a510049fd2f6e36bcabee81fd1) )

	ROM_REGION(0x1038, "rtc", ROMREGION_ERASE00)    // Default unlocked RTC
	ROM_LOAD("rtc", 0x0000, 0x1038, CRC(1a91f33a) SHA1(510b5cbacb218e5588f3b725733e095b7914dcdb))
ROM_END

ROM_START( popnanm )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("a02jaa03.21e", 0x00000, 0x80000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599))

	ROM_REGION(0xc8, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gq987_gc987_forever", 0x00, 0xc8, CRC(ddd976b6) SHA1(91b49585886b8b1618401ca43ec3bde09896b782)) // Modified to set the period to 00/00 for forever license mode

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a02jaa04.3q", 0x00000, 0x80000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "gq987jaa01", 0, SHA1(ee1f9cf480c01ef356451cec30e5303d6c433758) )

	DISK_REGION( "spu_ata:0:dvdrom" ) // data DVD-ROM
	DISK_IMAGE_READONLY( "gq987jaa02", 0, SHA1(47f90cc940af50c8d91751ec27b45070a95d4d58) )

	ROM_REGION(0x1038, "rtc", ROMREGION_ERASE00)    // Default unlocked RTC
	ROM_LOAD("rtc", 0x0000, 0x1038, CRC(b08b454d) SHA1(33fc12ab148a379925b7b77016efba747f3b13cc))
ROM_END

ROM_START( popnanma )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("a02jaa03.21e", 0x00000, 0x80000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599))

	ROM_REGION(0xc8, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gq987_gc987", 0x00, 0xc8, CRC(c77bb0fc) SHA1(7228f334d6662f764c2b0417bfd415f30ac919d7))

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a02jaa04.3q", 0x00000, 0x80000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "gq987jaa01", 0, SHA1(ee1f9cf480c01ef356451cec30e5303d6c433758) )

	DISK_REGION( "spu_ata:0:dvdrom" ) // data DVD-ROM
	DISK_IMAGE_READONLY( "gq987jaa02", 0, SHA1(47f90cc940af50c8d91751ec27b45070a95d4d58) )

	ROM_REGION(0x1038, "rtc", ROMREGION_ERASE00)    // Default unlocked RTC
	ROM_LOAD("rtc", 0x0000, 0x1038, CRC(b08b454d) SHA1(33fc12ab148a379925b7b77016efba747f3b13cc))
ROM_END

ROM_START( popnanm2 )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("a02jaa03.21e", 0x00000, 0x80000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599))

	ROM_REGION(0xc8, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gca02ja_gca02jb_gea02ja_forever", 0x00, 0xc8, CRC(63b22ee0) SHA1(60f384140ea80e886e45a56a37811d86133674a4)) // Modified to set the period to 00/00 for forever license mode

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a02jaa04.3q", 0x00000, 0x80000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "a02jac01", 0, SHA1(e81203b6812336c4d00476377193340031ef11b1) )

	DISK_REGION( "spu_ata:0:dvdrom" ) // data DVD-ROM
	DISK_IMAGE_READONLY( "gea02jaa02", 0, SHA1(b482d0898cafeafcb020d81d40bd8915c0440f1e) )

	ROM_REGION(0x1038, "rtc", ROMREGION_ERASE00)    // Default unlocked RTC
	ROM_LOAD("rtc", 0x0000, 0x1038, CRC(90fcfeab) SHA1(f96e27e661259dc9e7f25a99bee9ffd6584fc1b8))
ROM_END

ROM_START( popnanm2a )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("a02jaa03.21e", 0x00000, 0x80000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599))

	ROM_REGION(0xc8, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gca02ja_gca02jb_gea02ja", 0x00, 0xc8, CRC(7910e8aa) SHA1(e296a50e846ad13a98953b6804e9e4c22cf3a389))

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a02jaa04.3q", 0x00000, 0x80000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "a02jac01", 0, SHA1(e81203b6812336c4d00476377193340031ef11b1) )

	DISK_REGION( "spu_ata:0:dvdrom" ) // data DVD-ROM
	DISK_IMAGE_READONLY( "gea02jaa02", 0, SHA1(b482d0898cafeafcb020d81d40bd8915c0440f1e) )

	ROM_REGION(0x1038, "rtc", ROMREGION_ERASE00)    // Default unlocked RTC
	ROM_LOAD("rtc", 0x0000, 0x1038, CRC(90fcfeab) SHA1(f96e27e661259dc9e7f25a99bee9ffd6584fc1b8))
ROM_END

ROM_START( popnanm2ja )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("a02jaa03.21e", 0x00000, 0x80000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599))

	ROM_REGION(0xc8, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gca02ja_gca02jb_gea02ja_forever", 0x00, 0xc8, CRC(63b22ee0) SHA1(60f384140ea80e886e45a56a37811d86133674a4)) // Modified to set the period to 00/00 for forever license mode

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a02jaa04.3q", 0x00000, 0x80000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "a02jaa01", 0, SHA1(9f66a62bbe49f77254f24fb8759f78d078250bbf) )

	DISK_REGION( "spu_ata:0:dvdrom" ) // data DVD-ROM
	DISK_IMAGE_READONLY( "gea02jaa02", 0, SHA1(b482d0898cafeafcb020d81d40bd8915c0440f1e) )

	ROM_REGION(0x1038, "rtc", ROMREGION_ERASE00)    // Default unlocked RTC
	ROM_LOAD("rtc", 0x0000, 0x1038, CRC(90fcfeab) SHA1(f96e27e661259dc9e7f25a99bee9ffd6584fc1b8))
ROM_END

ROM_START( popnanm2jaa )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("a02jaa03.21e", 0x00000, 0x80000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599))

	ROM_REGION(0xc8, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gca02ja_gca02jb_gea02ja", 0x00, 0xc8, CRC(7910e8aa) SHA1(e296a50e846ad13a98953b6804e9e4c22cf3a389))

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a02jaa04.3q", 0x00000, 0x80000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "a02jaa01", 0, SHA1(9f66a62bbe49f77254f24fb8759f78d078250bbf) )

	DISK_REGION( "spu_ata:0:dvdrom" ) // data DVD-ROM
	DISK_IMAGE_READONLY( "gea02jaa02", 0, SHA1(b482d0898cafeafcb020d81d40bd8915c0440f1e) )

	ROM_REGION(0x1038, "rtc", ROMREGION_ERASE00)    // Default unlocked RTC
	ROM_LOAD("rtc", 0x0000, 0x1038, CRC(90fcfeab) SHA1(f96e27e661259dc9e7f25a99bee9ffd6584fc1b8))
ROM_END

ROM_START( popnmt )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP( "a02jaa03.21e", 0x000000, 0x080000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599) )

	ROM_REGION(0xc8, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gq976", 0x00, 0xc8, CRC(c626cf9a) SHA1(a47a1a2f57a207223a3a08060b75e2744a036d13))

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP( "a02jaa04.3q",  0x000000, 0x080000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683) )

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "976jaa01", 0, SHA1(622a9350107e9fb17609ea1a234ca35489915da7) )

	DISK_REGION( "spu_ata:0:dvdrom" ) // data DVD-ROM
	DISK_IMAGE_READONLY( "976jaa02", 0, SHA1(8a5fda9d98fbf7c9d702bf650fb131a89925eb2b) )

	ROM_REGION(0x1038, "rtc", ROMREGION_ERASE00)    // Default unlocked RTC
	ROM_LOAD("rtc", 0x0000, 0x1038, CRC(a51bdc10) SHA1(99b759d9a575129abec556d381f3a041453d7136))
ROM_END

ROM_START( popnmt2 )
	// This is an updated version of popnmt released a few months later which has NET@MANIA
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP( "a02jaa03.21e", 0x000000, 0x080000, CRC(43ecc093) SHA1(637df5b546cf7409dd4752dc471674fe2a046599) )

	ROM_REGION(0xc8, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD("gq976", 0x00, 0xc8, CRC(c626cf9a) SHA1(a47a1a2f57a207223a3a08060b75e2744a036d13))

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP( "a02jaa04.3q",  0x000000, 0x080000, CRC(8c6000dd) SHA1(94ab2a66879839411eac6c673b25143d15836683) )

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "976jba01", 0, SHA1(f8a70ca0718dc222cebbef238b5954494503d315) )

	DISK_REGION( "spu_ata:0:dvdrom" ) // data DVD-ROM
	DISK_IMAGE_READONLY( "976jaa02", 0, SHA1(8a5fda9d98fbf7c9d702bf650fb131a89925eb2b) )

	ROM_REGION(0x1038, "rtc", ROMREGION_ERASE00)    // Default unlocked RTC
	ROM_LOAD("rtc", 0x0000, 0x1038, CRC(a51bdc10) SHA1(99b759d9a575129abec556d381f3a041453d7136))
ROM_END

ROM_START( bm3 )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("972maina01.21e", 0x00000, 0x80000, CRC(9de35bfd) SHA1(57290e0015ea24fa46efdfe1e8299003b7754a3b))

	ROM_REGION(0xc8, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD( "gq972", 0x000000, 0x0000c8, CRC(b74dc63b) SHA1(94daa3696ebea8dbfd53938e1a0ed96084413c48))

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("972spua01.3q", 0x00000, 0x80000, CRC(308dbcff) SHA1(87d11eb3e28cb4f3a8f88e3c57a28809dc429ccd))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "gc97201", 0, SHA1(216ced68f2082bf891dc3e89fb0663f559cc4915) )

	DISK_REGION( "spu_ata:0:hdd" ) // HDD
	DISK_IMAGE_READONLY( "gc97202", 0, SHA1(84049bab473d29eca3c6d536956ef20ae410967d) )

	ROM_REGION(0x1038, "rtc", ROMREGION_ERASE00)    // Default unlocked RTC
	ROM_LOAD("rtc", 0x0000, 0x1038, CRC(20eff14e) SHA1(7d652ed4d9e245f9574dd0fec60ee078dc73ba61))
ROM_END

ROM_START( bm3core )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("972maina01.21e", 0x00000, 0x80000, CRC(9de35bfd) SHA1(57290e0015ea24fa46efdfe1e8299003b7754a3b))

	ROM_REGION(0xc8, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD( "gca05-jc.bin", 0x00, 0xc8, CRC(a4c67c80) SHA1(a87609052fa879116350564353df7f5b70ef3ae5) )

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("972spua01.3q", 0x00000, 0x80000, CRC(308dbcff) SHA1(87d11eb3e28cb4f3a8f88e3c57a28809dc429ccd))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "a05jca01", 0, SHA1(b89eced8a1325b087e3f875d1a643bebe9bad5c0) )

	DISK_REGION( "spu_ata:0:hdd" ) // HDD
	DISK_IMAGE_READONLY( "a05jca02", 0, SHA1(1de7db35d20bbf728732f6a24c19315f9f4ad469) )

	ROM_REGION(0x1038, "rtc", ROMREGION_ERASE00)    // Default unlocked RTC
	ROM_LOAD("rtc", 0x0000, 0x1038, CRC(64bc48d3) SHA1(18ccba42545c7c11ea3b486a4469d7c599a41c80))
ROM_END

ROM_START( bm36th )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("972maina01.21e", 0x00000, 0x80000, CRC(9de35bfd) SHA1(57290e0015ea24fa46efdfe1e8299003b7754a3b))

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD( "gca21-jc", 0x000000, 0x0000c0, BAD_DUMP CRC(5cf45b41) SHA1(14f9ff701df79c621d47d20fe3e6b8b579975a1e) )

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a21jca03.3q", 0x00000, 0x80000, CRC(98ea367a) SHA1(f0b72bdfbba4d265e7b08d606cd82424947d97b9))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "a21jca01", 0, SHA1(d1b888379cc0b2c2ab58fa2c5be49258043c3ea1) )

	DISK_REGION( "spu_ata:0:hdd" ) // HDD
	DISK_IMAGE_READONLY( "a21jca02", 0, SHA1(8fa11848af40966e42b6304e37de92be5c1fe3dc) )

	ROM_REGION(0x1038, "rtc", ROMREGION_ERASE00)    // Default unlocked RTC
	ROM_LOAD("rtc", 0x0000, 0x1038, CRC(832ef42b) SHA1(b53b09a1287631d40caf456f13f09faa991ae38c))
ROM_END

ROM_START( bm37th )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("972maina01.21e", 0x00000, 0x80000, CRC(9de35bfd) SHA1(57290e0015ea24fa46efdfe1e8299003b7754a3b))

	ROM_REGION(0xc0, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD( "gcb07-jc", 0x000000, 0x0000c0, BAD_DUMP CRC(18b32076) SHA1(6f5a44a0c2ed033bc00e73e95f9fbd8301719054) )

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a21jca03.3q", 0x00000, 0x80000, CRC(98ea367a) SHA1(f0b72bdfbba4d265e7b08d606cd82424947d97b9))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "gcb07jca01", 0, SHA1(f906379bdebee314e2ca97c7756259c8c25897fd) )

	DISK_REGION( "spu_ata:0:hdd" ) // HDD
	DISK_IMAGE_READONLY( "gcb07jca02", 0, SHA1(6b8e17635825a6a43dc8d2721fe2eb0e0f39e940) )

	ROM_REGION(0x1038, "rtc", ROMREGION_ERASE00)    // Default unlocked RTC
	ROM_LOAD("rtc", 0x0000, 0x1038, CRC(6383ed31) SHA1(296f32d5c6619d1b9eb882d9c3cd6db23bf52054))
ROM_END

ROM_START( bm3final )
	ROM_REGION32_BE(0x80000, "user1", 0)
	ROM_LOAD16_WORD_SWAP("972maina01.21e", 0x00000, 0x80000, CRC(9de35bfd) SHA1(57290e0015ea24fa46efdfe1e8299003b7754a3b))

	ROM_REGION(0xc8, "user2", ROMREGION_ERASE00)    // Security dongle
	ROM_LOAD( "gcc01-jc", 0x000000, 0x0000c8, CRC(41209cf8) SHA1(5122579bd81d25dc6589a97a8c71a311fdffe47d))

	ROM_REGION(0x80000, "audiocpu", 0)          // SPU 68K program
	ROM_LOAD16_WORD_SWAP("a21jca03.3q", 0x00000, 0x80000, CRC(98ea367a) SHA1(f0b72bdfbba4d265e7b08d606cd82424947d97b9))

	DISK_REGION( "ata:0:cdrom" ) // program CD-ROM
	DISK_IMAGE_READONLY( "gcc01jca01", 0, SHA1(3e7af83670d791591ad838823422959987f7aab9) )

	DISK_REGION( "spu_ata:0:hdd" ) // HDD
	DISK_IMAGE_READONLY( "gcc01jca02", 0, SHA1(823e29bab11cb67069d822f5ffb2b90b9d3368d2) )

	ROM_REGION(0x1038, "rtc", ROMREGION_ERASE00)    // Default unlocked RTC
	ROM_LOAD("rtc", 0x0000, 0x1038, CRC(bf7079cd) SHA1(7cf8ce9794d97e1ed8b12339f78c8678e895cb19))
ROM_END

} // Anonymous namespace


/*****************************************************************************/

GAME( 2000, ppp,    0,   firebeat_ppp, ppp, firebeat_ppp_state, init_ppp_jp,       ROT0, "Konami", "ParaParaParadise", MACHINE_IMPERFECT_SOUND )
GAME( 2000, ppd,    0,   firebeat_ppp, ppp, firebeat_ppp_state, init_ppp_overseas, ROT0, "Konami", "ParaParaDancing", MACHINE_IMPERFECT_SOUND )
GAME( 2000, ppp11,  0,   firebeat_ppp, ppp, firebeat_ppp_state, init_ppp_jp,       ROT0, "Konami", "ParaParaParadise v1.1", MACHINE_IMPERFECT_SOUND )
GAME( 2000, ppp1mp, ppp, firebeat_ppp, ppp, firebeat_ppp_state, init_ppp_jp,       ROT0, "Konami", "ParaParaParadise 1st Mix Plus", MACHINE_IMPERFECT_SOUND )

// Keyboard sounds do not work: requires MU-100 emulation (ymu100.cpp) which is not in a fully working state yet
GAMEL( 2000, kbm,    0,   firebeat_kbm, kbm, firebeat_kbm_state, init_kbm_jp,       ROT270, "Konami", "Keyboardmania", MACHINE_IMPERFECT_SOUND, layout_firebeat )
GAMEL( 2000, kbh,    kbm, firebeat_kbm, kbm, firebeat_kbm_state, init_kbm_overseas, ROT270, "Konami", "Keyboardheaven (Korea)", MACHINE_IMPERFECT_SOUND, layout_firebeat )
GAMEL( 2000, kbm2nd, 0,   firebeat_kbm, kbm, firebeat_kbm_state, init_kbm_jp,       ROT270, "Konami", "Keyboardmania 2nd Mix", MACHINE_IMPERFECT_SOUND, layout_firebeat )
GAMEL( 2001, kbm3rd, 0,   firebeat_kbm, kbm, firebeat_kbm_state, init_kbm_jp,       ROT270, "Konami", "Keyboardmania 3rd Mix", MACHINE_IMPERFECT_SOUND, layout_firebeat )

GAME( 2000, popn4,       0,        firebeat_popn, popn, firebeat_popn_state, init_popn_jp,     ROT0, "Konami", "Pop'n Music 4", MACHINE_IMPERFECT_SOUND )
GAME( 2000, popn5,       0,        firebeat_popn, popn, firebeat_popn_state, init_popn_jp,     ROT0, "Konami", "Pop'n Music 5", MACHINE_IMPERFECT_SOUND )
GAME( 2001, popn6,       0,        firebeat_popn, popn, firebeat_popn_state, init_popn_jp,     ROT0, "Konami", "Pop'n Music 6", MACHINE_IMPERFECT_SOUND )
GAME( 2001, popn7,       0,        firebeat_popn, popn, firebeat_popn_state, init_popn_jp,     ROT0, "Konami", "Pop'n Music 7", MACHINE_IMPERFECT_SOUND )
GAME( 2002, popn8,       0,        firebeat_popn, popn, firebeat_popn_state, init_popn_jp,     ROT0, "Konami", "Pop'n Music 8", MACHINE_IMPERFECT_SOUND )
GAME( 2000, popnmt,      0,        firebeat_popn, popn, firebeat_popn_state, init_popn_rental, ROT0, "Konami", "Pop'n Music Mickey Tunes", MACHINE_IMPERFECT_SOUND )
GAME( 2000, popnmt2,     popnmt,   firebeat_popn, popn, firebeat_popn_state, init_popn_rental, ROT0, "Konami", "Pop'n Music Mickey Tunes!", MACHINE_IMPERFECT_SOUND )
GAME( 2000, popnanm,     0,        firebeat_popn, popn, firebeat_popn_state, init_popn_jp,     ROT0, "Konami", "Pop'n Music Animelo", MACHINE_IMPERFECT_SOUND )
GAME( 2000, popnanma,    popnanm,  firebeat_popn, popn, firebeat_popn_state, init_popn_jp,     ROT0, "Konami", "Pop'n Music Animelo (license expired)", MACHINE_IMPERFECT_SOUND )
GAME( 2001, popnanm2,    0,        firebeat_popn, popn, firebeat_popn_state, init_popn_jp,     ROT0, "Konami", "Pop'n Music Animelo 2 (JAC)", MACHINE_IMPERFECT_SOUND )
GAME( 2001, popnanm2a,   popnanm2, firebeat_popn, popn, firebeat_popn_state, init_popn_jp,     ROT0, "Konami", "Pop'n Music Animelo 2 (JAC, license expired)", MACHINE_IMPERFECT_SOUND )
GAME( 2001, popnanm2ja,  popnanm2, firebeat_popn, popn, firebeat_popn_state, init_popn_jp,     ROT0, "Konami", "Pop'n Music Animelo 2 (JAA)", MACHINE_IMPERFECT_SOUND )
GAME( 2001, popnanm2jaa, popnanm2, firebeat_popn, popn, firebeat_popn_state, init_popn_jp,     ROT0, "Konami", "Pop'n Music Animelo 2 (JAA, license expired)", MACHINE_IMPERFECT_SOUND )

// Requires ST-224 emulation for optional toggleable external effects, but otherwise is fully playable
GAME( 2000, bm3,      0, firebeat_bm3, bm3, firebeat_bm3_state, init_bm3, ROT0, "Konami", "Beatmania III", MACHINE_IMPERFECT_SOUND )
GAME( 2000, bm3core,  0, firebeat_bm3, bm3, firebeat_bm3_state, init_bm3, ROT0, "Konami", "Beatmania III Append Core Remix", MACHINE_IMPERFECT_SOUND )
GAME( 2001, bm36th,   0, firebeat_bm3, bm3, firebeat_bm3_state, init_bm3, ROT0, "Konami", "Beatmania III Append 6th Mix", MACHINE_IMPERFECT_SOUND )
GAME( 2002, bm37th,   0, firebeat_bm3, bm3, firebeat_bm3_state, init_bm3, ROT0, "Konami", "Beatmania III Append 7th Mix", MACHINE_IMPERFECT_SOUND )
GAME( 2003, bm3final, 0, firebeat_bm3, bm3, firebeat_bm3_state, init_bm3, ROT0, "Konami", "Beatmania III The Final", MACHINE_IMPERFECT_SOUND )
