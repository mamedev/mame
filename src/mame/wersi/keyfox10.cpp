// license:BSD-3-Clause
// copyright-holders:

#include "emu.h"
#include "cpu/mcs51/i80c52.h"
#include "bus/midi/midi.h"
#include "sound/sam8905.h"
#include "speaker.h"
#include "debugger.h"

#include "keyfox10.lh"

//#define LOG_SERIAL (1U << 1)
#define LOG_SERIAL 0
//#define LOG_PORT   (1U << 2)
#define LOG_PORT   0
//#define LOG_SAM    (1U << 3)
#define LOG_SAM    0
//#define LOG_IO     (1U << 4)
#define LOG_IO     0
#define VERBOSE (LOG_SERIAL | LOG_SAM | LOG_IO)
#include "logmacro.h"

namespace {

// SAM8905 DSP state
// Control byte: [X X X X | SSR | IDL | SEL | WR]
//   SSR: 0=44.1kHz, 1=22.05kHz sample rate
//   IDL: 0=normal, 1=all 16 tones output
//   SEL: 0=D-RAM, 1=A-RAM
//   WR:  0=read, 1=write
struct sam8905_state
{
    u8 ctrl = 0;
    u8 addr = 0;
    u16 aram[8 * 32] = {};   // A-RAM: 8 slots × 32 words × 15 bits
    u32 dram[16 * 16] = {};  // D-RAM: 16 slots × 16 words × 19 bits
};

class keyfox10_state : public driver_device
{
public:
    keyfox10_state(const machine_config &mconfig, device_type type, const char *tag)
        : driver_device(mconfig, type, tag)
        , m_maincpu(*this, "maincpu")
        , m_sam_snd(*this, "sam_snd")
        , m_sam_fx(*this, "sam_fx")
        , m_midi_out(*this, "mdout")
        , m_rom(*this, "program")
        , m_samples_rom(*this, "samples")
        , m_rhythms_rom(*this, "rhythms")
        , m_panel_io(*this, "PANEL%u", 0U)
        , m_kbd_io(*this, "KBDMTX%u", 0U)
        , m_digits(*this, "digit%u", 0U)
        , m_digit_dp(*this, "dp%u", 0U)
        , m_beat_leds(*this, "beat%u", 0U)
        , m_panel_leds(*this, "panel%u_%u", 0U, 0U)
    { }

    ~keyfox10_state()
    {
        // Close GAL log
        if (m_gal_log)
        {
            fclose(m_gal_log);
            m_gal_log = nullptr;
        }
        // Dump SAM memory on exit
        dump_sam_memory();
    }

    void keyfox10(machine_config &config);

private:
    void program_map(address_map &map) ATTR_COLD;
    void data_map(address_map &map) ATTR_COLD;

    void port0_w(u8 data);
    void port1_w(u8 data);
    void port2_w(u8 data);
    void port3_w(u8 data);
    u8 port0_r();
    u8 port1_r();
    u8 port2_r();
    u8 port3_r();

    // SAM8905 DSP handlers (SND at 0x8000, FX at 0xE000)
    u8 sam_snd_r(offs_t offset);
    void sam_snd_w(offs_t offset, u8 data);
    u8 sam_fx_r(offs_t offset);
    void sam_fx_w(offs_t offset, u8 data);
    u8 sam8905_r(int chip, offs_t offset);
    void sam8905_w(int chip, offs_t offset, u8 data);

    // SAM8905 external waveform ROM handlers
    u16 sam_snd_waveform_r(offs_t offset);
    u16 sam_fx_waveform_r(offs_t offset);

    // SAM8905 inter-chip audio callback (sound SAM -> FX SAM)
    void sam_snd_sample_out(uint32_t data);

    // FX SAM external SRAM write callback
    void sam_fx_waveform_w(offs_t offset, u16 data);

    void midi_rxd_w(int state) { m_midi_rxd = state; }

    required_device<i80c32_device> m_maincpu;
    required_device<sam8905_device> m_sam_snd;
    required_device<sam8905_device> m_sam_fx;
    required_device<midi_port_device> m_midi_out;
    required_region_ptr<u8> m_rom;
    required_region_ptr<u8> m_samples_rom;  // IC17 "Samples" 128KB
    required_region_ptr<u8> m_rhythms_rom;  // IC14 "Rhythmen" 128KB
    required_ioport_array<10> m_panel_io;   // 10x 8-bit button panels
    required_ioport_array<2> m_kbd_io;      // 2x 8-bit keyboard matrix (IC4, IC11)
    output_finder<3> m_digits;              // 7-segment display outputs (digit0-digit2)
    output_finder<3> m_digit_dp;            // Decimal point outputs (dp0-dp2)
    output_finder<8> m_beat_leds;           // Beat position LEDs (beat0-beat7)
    output_finder<10, 8> m_panel_leds;      // Panel button LEDs (10 ICs x 8 bits = 80 LEDs)
    u8 m_port0 = 0xff;
    u8 m_port1 = 0xff;
    u8 m_port2 = 0xff;
    u8 m_port3 = 0xff;
    u8 m_midi_rxd = 1;  // MIDI RX bit (idle high)

    // Current input sample for FX chip (from SND chip via 74HC595 shift registers)
    uint16_t m_fx_input_sample = 0;

    // FX SAM 32KB SRAM for delay/reverb buffers
    std::unique_ptr<uint8_t[]> m_fx_sram;
    static constexpr size_t FX_SRAM_SIZE = 32768;  // 32KB = 32768 8-bit words

    // SAM8905 DSP - two chips selected via T1 (P3.5)
    // T1=1: Sound generation SAM (index 0)
    // T1=0: Effects SAM (index 1)
    sam8905_state m_sam[2];

    int sam_chip_select() const { return BIT(m_port3, 5) ? 0 : 1; }

    // Debug watches - set these values directly in code to enable
    // D-RAM addr = slot*16 + word (e.g., 0x0F = slot 0 word 15)
    // A-RAM addr = alg*32 + word (e.g., 0x00 = algorithm 0 word 0)
    // Set to -1 to disable, or set address to watch
    s16 m_sam_watch_dram = 0;  // D-RAM address to watch (0-255, -1=off)
    s16 m_sam_watch_aram = 0;  // A-RAM address to watch (0-255, -1=off)
    s32 m_rom_watch_addr = 0;  // ROM address to watch (0x10000-0x1FFFF, -1=off)
    bool m_watch_break = false; // true = break to debugger on watch hit

    // P1 pin definitions:
    // P1.0: CLK_SW   (out) - switch scan clock
    // P1.1: CLK_BEAT (out) - beat LED clock
    // P1.2: CLK_LED  (out) - LED clock
    // P1.3: CLK_DISP (out) - display shift register clock
    // P1.4: ENABLE   (out)
    // P1.5: MODE     (out)
    // P1.6: DATA     (out) - serial data for shift registers
    // P1.7: SENSE    (in)  - input from switches
    static constexpr u8 P1_CLK_SW   = 0x01;
    static constexpr u8 P1_CLK_BEAT = 0x02;
    static constexpr u8 P1_CLK_LED  = 0x04;
    static constexpr u8 P1_CLK_DISP = 0x08;
    static constexpr u8 P1_ENABLE   = 0x10;
    static constexpr u8 P1_MODE     = 0x20;
    static constexpr u8 P1_DATA     = 0x40;
    static constexpr u8 P1_SENSE    = 0x80;

    // 3x 74HC164 shift registers for 7-segment display (active low segments)
    // IC7 -> DISPLAY1, IC6 -> DISPLAY2, IC10 -> DISPLAY3
    u8 m_disp_sr[3] = {0xff, 0xff, 0xff};  // Shift registers (active low, init to all off)

    void disp_shift_clock();
    void update_display();    // Update MAME display outputs from shift registers

    // 10x 74HC574 shift registers for button panel (80 buttons with LEDs)
    // Chain: IC8 -> IC12 -> IC13 -> IC15 -> IC14 -> IC16 -> IC17 -> IC1 -> IC3 -> IC2
    // Uses CLK_LED for both LED display and button scanning
    // 74HC574 has separate shift register and latched outputs:
    // - Shift register: continuously shifted via CLK_LED
    // - Latched outputs: captured from shift register on ENABLE rising edge
    u8 m_panel_sr[10] = {};        // Shift register chain (80 bits) - for shifting
    u8 m_panel_latch[10] = {};     // Latched outputs (80 bits) - drives LEDs
    u8 m_btn_state[10] = {};       // Current button states from inputs (80 buttons)

    void panel_shift_clock();      // CLK_LED rising edge handler
    void panel_latch();            // ENABLE rising edge - latch shift reg to outputs
    void update_panel_leds();      // Update MAME panel LED outputs from latched state

    // CLK_SW keyboard matrix: 4x 74HC175 + 2x 74HC251 multiplexers
    // 74HC175 shift register chain provides select address for 74HC251 muxes
    // IC4 (74HC251): RHYTHM-/+, ACC+/-, TEMPO+/-, START/S
    // IC11 (74HC251): UPPER+/-, LOWER-/+, BASS-/+
    u16 m_kbd_sr = 0;           // 4x 74HC175 = 16-bit shift register
    u8 m_kbd_state[2] = {};     // Current keyboard matrix button states (IC4, IC11)

    void kbd_shift_clock();     // CLK_SW rising edge handler
    u8 read_kbd_mux() const;    // Read selected switch via 74HC251 multiplexers

    // CLK_BEAT beat LED: IC9 - 74HC164 shift register
    // 8-bit shift register for beat position indicator LEDs
    // Outputs QA-QH drive LEDs via diodes D1-D7
    u8 m_beat_sr = 0;           // Beat LED shift register (8 bits)

    void beat_shift_clock();    // CLK_BEAT rising edge handler

    // GAL16V8 input logger - captures inputs for hardware replay
    // Inputs: ~RD, ~WR, ~PSEN, A13, A14, A15, T0, T1
    virtual void machine_start() override ATTR_COLD;
    void log_gal_inputs(bool rd, bool wr, bool psen, u16 addr);
    FILE *m_gal_log = nullptr;
    u32 m_gal_seen[8] = {};  // Bitmap for 256 possible input combinations

    // SAM8905 memory dump (called from destructor)
    void dump_sam_memory() const;
};

void keyfox10_state::program_map(address_map &map)
{
    // 80C32 has 16-bit address space; ROM banking for 128KB ROM TBD
    map(0x0000, 0xffff).rom().region("program", 0);
}

void keyfox10_state::data_map(address_map &map)
{
    // Memory map controlled by GAL16V8, depends on T1 (P3.5) state:
    // T1=0: Full ROM data access (upper 64KB at 0x10000-0x1FFFF)
    // T1=1: RAM + SAM chips
    //
    // During MOVX read, ~RD is inverted to ROM A16, so data reads access ROM upper 64KB
    map(0x0000, 0x1fff).ram();  // 8KB RAM (active when T1=1 or for writes)
    map(0x2000, 0x7fff).rom().region("program", 0x12000);  // ROM data (always accessible)
    // 0x8000-0xDFFF: ROM data when T1=0, SAM SND at 0x8000-0x8007 when T1=1
    map(0x8000, 0xdfff).rw(FUNC(keyfox10_state::sam_snd_r), FUNC(keyfox10_state::sam_snd_w));
    // 0xE000-0xFFFF: ROM data when T1=0, SAM FX at 0xE000-0xE007 when T1=1
    map(0xe000, 0xffff).rw(FUNC(keyfox10_state::sam_fx_r), FUNC(keyfox10_state::sam_fx_w));
}

u8 keyfox10_state::port0_r()
{
    LOGMASKED(LOG_PORT, "P0 read: 0x%02X\n", m_port0);
    return m_port0;
}

u8 keyfox10_state::port1_r()
{
    // P1.7 (SENSE) combines LED panel sense and keyboard matrix sense via NAND gates:
    //   SENSE = NAND(NAND(~MODE, LED_SENSE), SW_SENSE)
    //         = (~MODE & LED_SENSE) | ~SW_SENSE
    //
    // Hardware behavior:
    // - MODE (P1.5) controls Q2 (BC640 PNP): MODE=0 -> LEDs powered, MODE=1 -> LEDs off
    // - ~MODE goes to IC5 (74HC175) ~CLR: MODE=0 -> kbd matrix enabled, MODE=1 -> cleared
    // - LED_SENSE is active-high (button pressed connects shift reg output to sense line)
    // - SW_SENSE is active-low (button pressed pulls mux output low)
    //
    // With 74HC299 MODE polarity (MODE=0 = shift, MODE=1 = parallel load):
    // - MODE=0 (shift mode): SENSE = panel_sense || kbd_sense (button scanning active)
    // - MODE=1 (parallel load): SENSE = kbd_sense only

    u8 data = m_port1 & 0x7f;  // Clear SENSE bit

    bool mode = (m_port1 & P1_MODE) != 0;

    // Read current button state from panel input ports
    for (int i = 0; i < 10; i++)
    {
        m_btn_state[i] = m_panel_io[i]->read();
    }

    // Read current keyboard matrix state from input ports
    for (int i = 0; i < 2; i++)
    {
        m_kbd_state[i] = m_kbd_io[i]->read();
    }

    // LED_SENSE: Wired-OR of all (Q output AND button pressed) through diodes
    // - When shift_reg bit = 1 AND button pressed: diode conducts, LEDSENSE goes high
    // - When shift_reg bit = 0 OR button not pressed: no current, LEDSENSE stays low
    //
    // Scanning sequence:
    // 1. Parallel load all 1s (MODE=1 + CLK) - if any button pressed, SENSE = high
    // 2. Shift 0 through (MODE=0, DATA=0, CLK) - SENSE stays high until 0 reaches pressed button
    // 3. When 0 reaches pressed button position, that bit no longer contributes to SENSE
    // 4. If only one button pressed, SENSE goes low when 0 reaches it
    //bool led_sense = false;
    bool led_sense = false;
    static int cnt_btn;

    if ((m_port1 & P1_ENABLE)) {
        // compare with panel_sr
        for (int i = 0; i < 10; i++)
        {
            //if (m_panel_sr[i] & m_btn_state[i] == 0)
            if ((m_panel_sr[i]) & m_btn_state[i])
            {
                led_sense = true;
                break;
            }
        }
        //LOGMASKED(LOG_IO, "CHECK %d - %d\n", led_sense, cnt_btn);
        cnt_btn++;
    } else {
        // floating - check if any button is pressed
        for (int i = 0; i < 10; i++)
        {
            if (m_btn_state[i])
            {
                led_sense = true;
                cnt_btn = 0;
                LOGMASKED(LOG_IO, "DETECTED %d, %u\n", i, m_btn_state[i], mode);
                break;
            }
        }
    }

    // SW_SENSE: Check keyboard matrix via 74HC251 muxes
    // read_kbd_mux() returns active-low: 0 = button pressed, 1 = no button
    // Note: When MODE=1, IC5 is cleared so mux address = 0
    //u8 sw_sense = read_kbd_mux();  // Active-low hardware signal
    u8 sw_sense = read_kbd_mux();  // Active-low hardware signal

    // SENSE = NAND(NAND(~MODE, LED_SENSE), SW_SENSE)
    //       = (~MODE & LED_SENSE) | ~SW_SENSE
    // ~SW_SENSE: invert the active-low signal (0->1 when button pressed)
    // With MODE polarity swap: MODE=0 means shift/sense mode
    //bool sense = (!mode && led_sense) || (mode && !sw_sense);

    bool sense = (mode && led_sense) || (!mode && !sw_sense);

    //bool sense = (! (!(mode && led_sense)) && (sw_sense));

    if (sense)
        data |= P1_SENSE;

    LOGMASKED(LOG_IO, "P1 read: 0x%02X (SENSE=%d MODE=%d led=%d sw=%d) %d\n",
        data, sense ? 1 : 0,
              mode ? 1 : 0,
              led_sense ? 1 : 0,
              sw_sense ? 1 : 0, cnt_btn);

    return data;
}

u8 keyfox10_state::port2_r()
{
    LOGMASKED(LOG_PORT, "P2 read: 0x%02X\n", m_port2);
    return m_port2;
}

u8 keyfox10_state::port3_r()
{
    // P3.0 = MIDI RXD input
    u8 data = (m_port3 & 0xfe) | (m_midi_rxd & 0x01);
    LOGMASKED(LOG_PORT, "P3 read: 0x%02X\n", data);
    return data;
}

void keyfox10_state::port0_w(u8 data)
{
    if (m_port0 != data)
        LOGMASKED(LOG_PORT, "P0 write: 0x%02X\n", data);
    m_port0 = data;
}

void keyfox10_state::port1_w(u8 data)
{
    //data ^= (P1_CLK_LED | P1_CLK_SW);
    u8 changed = m_port1 ^ data;
    u8 rising = changed & data;

    // if (changed)
    // {
    //     LOGMASKED(LOG_IO, "P1 write: 0x%02X [%s%s%s%s%s%s%s]\n",
    //         data,
    //         (changed & P1_CLK_SW)   ? ((data & P1_CLK_SW)   ? "CLK_SW↑ "   : "CLK_SW↓ ")   : "",
    //         (changed & P1_CLK_BEAT) ? ((data & P1_CLK_BEAT) ? "CLK_BEAT↑ " : "CLK_BEAT↓ ") : "",
    //         (changed & P1_CLK_LED)  ? ((data & P1_CLK_LED)  ? "CLK_LED↑ "  : "CLK_LED↓ ")  : "",
    //         (changed & P1_CLK_DISP) ? ((data & P1_CLK_DISP) ? "CLK_DISP↑ " : "CLK_DISP↓ ") : "",
    //         (changed & P1_ENABLE)   ? ((data & P1_ENABLE)   ? "ENABLE↑ "   : "ENABLE↓ ")   : "",
    //         (changed & P1_MODE)     ? ((data & P1_MODE)     ? "MODE↑ "     : "MODE↓ ")     : "",
    //         (changed & P1_DATA)     ? ((data & P1_DATA)     ? "DATA↑ "     : "DATA↓ ")     : "");
    // }

    // CLK_DISP rising edge: shift DATA bit into display shift registers
    if (rising & P1_CLK_DISP)
    {
        disp_shift_clock();
    }

    // CLK_LED rising edge: shift DATA bit into button panel shift registers
    // Used for both LED display and button scanning
    if (rising & P1_CLK_LED)
    {
        //if (0)
            panel_shift_clock();
    }

    // CLK_SW rising edge: keyboard matrix address shift register
    // 4x 74HC175 chain provides address for 74HC251 multiplexers
    if (rising & P1_CLK_SW)
    {
        kbd_shift_clock();
    }

    // CLK_BEAT rising edge: beat LED indicator shift register
    // IC9 - 74HC164 provides beat position for rhythm display
    if (rising & P1_CLK_BEAT)
    {
        beat_shift_clock();
    }

    // ENABLE rising edge: latch panel shift register to output latches
    //lib/ Must happen before m_port1 update so we check 'rising' correctly
    //if (rising & P1_ENABLE)
    if (rising && (data & P1_ENABLE) == 0 && (data & P1_MODE) == 0)
    {
        panel_latch();
    }

    // Update m_port1 before checking MODE-dependent updates
    m_port1 = data;

    // Update LED display when ENABLE latches new data or MODE changes
    // (LEDs only light when MODE=0)
    if ((rising & P1_ENABLE) || (changed & P1_MODE))
    {
        update_panel_leds();
    }
}

void keyfox10_state::port2_w(u8 data)
{
    if (m_port2 != data)
    {
        LOGMASKED(LOG_PORT, "P2 write: 0x%02X\n", data);
        // Log P2 changes specifically since it's used for external memory address high byte
        LOGMASKED(LOG_SAM, "P2 = 0x%02X (ext addr A8-A15)\n", data);
    }
    m_port2 = data;
}

void keyfox10_state::port3_w(u8 data)
{
    if (m_port3 != data)
        LOGMASKED(LOG_PORT, "P3 write: 0x%02X\n", data);
    // P3.1 = TXD output to MIDI
    if ((m_port3 ^ data) & 0x02)
        m_midi_out->write_txd(BIT(data, 1));
    m_port3 = data;
}

void keyfox10_state::disp_shift_clock()
{
    // 74HC164 shift registers are daisy-chained:
    // DATA -> IC7 -> IC6 -> IC10 (bit 7 of each feeds into bit 0 of next)
    // On rising CLK_DISP edge, shift all registers left by 1 bit

    u8 data_in = (m_port1 & P1_DATA) ? 1 : 0;  // DATA bit to shift in
    //u8 data_in = (m_port1 & P1_DATA) ? 0 : 1;  // DATA bit to shift in

    // Get carry bits from each register before shifting
    u8 ic7_carry = BIT(m_disp_sr[0], 7);
    u8 ic6_carry = BIT(m_disp_sr[1], 7);

    // Shift all registers left, insert carry/data into LSB
    m_disp_sr[2] = (m_disp_sr[2] << 1) | ic6_carry;  // IC10: receives from IC6
    m_disp_sr[1] = (m_disp_sr[1] << 1) | ic7_carry;  // IC6: receives from IC7
    m_disp_sr[0] = (m_disp_sr[0] << 1) | data_in;    // IC7: receives DATA

    LOGMASKED(LOG_IO, "DISP shift: DATA=%d -> IC7=0x%02X IC6=0x%02X IC10=0x%02X\n",
        data_in, m_disp_sr[0], m_disp_sr[1], m_disp_sr[2]);

    // Update visual display outputs
    update_display();
}

void keyfox10_state::update_display()
{
    // Convert shift register values to MAME 7-segment format
    // SE67F is common-cathode: 1 = segment ON, 0 = segment OFF
    // MAME led7seg: bit set = segment ON (same convention)
    for (int i = 0; i < 3; i++)
    {
        u8 segs = m_disp_sr[i];       // No inversion needed for common-cathode
        m_digits[i] = segs & 0x7f;    // bits 0-6 = segments a-g
        m_digit_dp[i] = BIT(segs, 7); // bit 7 = decimal point
    }

    // Update beat LEDs from beat shift register
    for (int i = 0; i < 8; i++)
    {
        m_beat_leds[i] = BIT(m_beat_sr, i);
    }
}

void keyfox10_state::panel_latch()
{
    // 74HC574 ENABLE rising edge: latch shift register to outputs
    // This captures the current shift register state for LED display
    for (int i = 0; i < 10; i++)
    {
        m_panel_latch[i] = ~m_panel_sr[i]; // FIX
    }
    LOGMASKED(LOG_IO, "Panel latch: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
        m_panel_latch[0], m_panel_latch[1], m_panel_latch[2], m_panel_latch[3], m_panel_latch[4],
        m_panel_latch[5], m_panel_latch[6], m_panel_latch[7], m_panel_latch[8], m_panel_latch[9]);
}

void keyfox10_state::update_panel_leds()
{
    // Update panel LEDs from LATCHED outputs (not shift register)
    // 10 ICs × 8 bits = 80 LEDs
    //
    // Hardware: LEDs only physically light when MODE=0 (Q2 PNP powers LEDs)
    // MODE toggles rapidly for button scanning. Only update LED outputs when
    // MODE=0 to avoid flickering - keeps previous state during sense mode.
    //
    // Position mapping: Due to shift direction, firmware LED bit N ends up at
    // physical position (79-N). Button scan position matches physical position.
    // Invert the output mapping so button position matches displayed LED position.
    bool mode = (m_port1 & P1_MODE) != 0;

    if (mode)
         return;  // Don't update during sense mode - keep previous LED state

    for (int ic = 0; ic < 10; ic++)
    {
        for (int bit = 0; bit < 8; bit++)
        {
            // Invert: m_panel_latch[0] bit 0 (shift position 79) → display position 79
            // m_panel_latch[ic] bit b → display position (79 - (ic*8 + bit)) = (9-ic)*8 + (7-bit)
            //m_panel_leds[9 - ic][7 - bit] = BIT(m_panel_latch[ic], bit);
            m_panel_leds[ic][bit] = BIT(m_panel_latch[ic], bit);
        }
    }
}

void keyfox10_state::panel_shift_clock()
{
    // 10x 74HC299 bidirectional shift registers for button panel:
    // DATA -> KF2:4 -> KF2:3 -> KF2:2 -> KF2:1 -> KF1:12 -> KF1:11 -> KF1:10 -> KF1:2 -> KF1:1 -> KF3:1
    //
    // 74HC299 with S1=HIGH, S0 directly connected to MODE pin:
    // - MODE=0: S1=1, S0=0 -> Shift right on CLK edge
    // - MODE=1: S1=1, S0=1 -> Parallel load on CLK edge (H inputs tied high -> 0xFF)
    //
    // Note: Schematic labels this as "~MODE" but firmware behavior shows MODE=0 = shift
    // All 74HC299 operations are triggered on CLK rising edge

    if ((m_port1 & P1_ENABLE) == 0 && (m_port1 & P1_MODE))
    {
        // MODE=1: Parallel load from H inputs (tied high -> all 1s)
        for (int i = 0; i < 10; i++)
            m_panel_sr[i] = 0xff;

        LOGMASKED(LOG_IO, "P1 Panel parallel load: all 0xFF\n");
        return;
    }

    // MODE=0: Shift right - DATA enters MSB of first IC, LSB of each IC feeds MSB of next
    u8 data_in = (m_port1 & P1_DATA) ? 1 : 0;
    //u8 data_in = (m_port1 & P1_DATA) ? 0 : 1;

    // Shift chain right: bit 0 of IC[i] feeds bit 7 of IC[i+1]
    for (int i = 9; i > 0; i--)
    {
        m_panel_sr[i] = (m_panel_sr[i] >> 1) | (BIT(m_panel_sr[i-1], 0) << 7);
    }
    m_panel_sr[0] = (m_panel_sr[0] >> 1) | (data_in << 7);

    LOGMASKED(LOG_IO, "P1 Panel shift right: DATA=%d -> SR[0]=0x%02X SR[9]=0x%02X\n",
        data_in, m_panel_sr[0], m_panel_sr[9]);
}

void keyfox10_state::kbd_shift_clock()
{
    // 4x 74HC175 quad D-flip-flops for keyboard matrix address generation
    // DATA -> IC5[0] -> IC5[1] -> IC5[2] -> IC5[3]
    // On rising CLK_SW edge, shift DATA into 16-bit chain
    // Outputs provide SELA, SELB, SELC (3-bit address) for 74HC251 muxes

    u8 data_in = (m_port1 & P1_DATA) ? 1 : 0;
    //u8 data_in = (m_port1 & P1_DATA) ? 0 : 1;

    // Shift left, insert new bit at LSB
    m_kbd_sr = (m_kbd_sr << 1) | data_in;

    LOGMASKED(LOG_IO, "Kbd shift: DATA=%d -> SR=0x%04X (addr=%d)\n",
        data_in, m_kbd_sr, m_kbd_sr & 0x07);
}

u8 keyfox10_state::read_kbd_mux() const
{
    // 74HC251 8:1 multiplexers select button based on address from shift register
    // Bits 0-2 (SELA, SELB, SELC): Select which of 8 inputs on the mux
    // Bit 3 -> STRB (~OE): Select which mux is enabled
    //   Bit 3 = 0: IC4 enabled (STRB=0), IC11 disabled (STRB=1)
    //   Bit 3 = 1: IC4 disabled (STRB=1), IC11 enabled (STRB=0)
    //
    // IC4: RHYTHM-/+, ACC+/-, TEMPO+/-, START/S (D0-D6)
    // IC11: UPPER+/-, LOWER-/+, BASS-/+ (D0-D5)
    //
    // Returns active-low: 0 = button pressed, 1 = no button
    // (buttons have 10K pull-ups, pressing pulls to GND)

    u8 addr = m_kbd_sr & 0x07;        // 3-bit mux address (SELA, SELB, SELC)
    bool ic11_sel = BIT(m_kbd_sr, 3); // Bit 3 selects mux (0=IC4, 1=IC11)

    bool button_pressed;
    if (ic11_sel)
    {
        // IC11 enabled: UPPER+/-, LOWER-/+, BASS-/+
        button_pressed = BIT(m_kbd_state[1], addr);
    }
    else
    {
        // IC4 enabled: RHYTHM-/+, ACC+/-, TEMPO+/-, START/S
        button_pressed = BIT(m_kbd_state[0], addr);
    }

    // Return active-low (0 = pressed, 1 = not pressed)
    return button_pressed ? 0 : 1;
}

void keyfox10_state::beat_shift_clock()
{
    // IC9 - 74HC164 8-bit shift register for beat indicator LEDs
    // On rising CLK_BEAT edge, shift DATA into register
    // Outputs QA-QH drive beat position LEDs

    u8 data_in = (m_port1 & P1_DATA) ? 1 : 0;
    //u8 data_in = (m_port1 & P1_DATA) ? 0 : 1;

    // Shift left, insert new bit at LSB
    m_beat_sr = (m_beat_sr << 1) | data_in;

    LOGMASKED(LOG_IO, "Beat shift: DATA=%d -> SR=0x%02X\n", data_in, m_beat_sr);

    // Update beat LED outputs
    update_display();
}

u8 keyfox10_state::sam_snd_r(offs_t offset)
{
    // T1=0: ROM data mode (0x8000-0xDFFF -> ROM 0x18000-0x1DFFF)
    // T1=1: SAM SND chip access (only 0x8000-0x8007 active)
    if (!BIT(m_port3, 5))  // T1=0: ROM data
    {
        u32 rom_offset = 0x18000 + offset;
        if (m_rom_watch_addr >= 0) // && rom_offset == u32(m_rom_watch_addr))
        {
            logerror("ROM watch: PC=%04X read ROM[%05X] = %02X\n",
                m_maincpu->pc(), rom_offset, m_rom[rom_offset]);
            if (m_watch_break)
                machine().debug_break();
        }
        return m_rom[rom_offset];
    }
    // T1=1: SAM chip (only first 8 bytes decoded)
    if (offset < 8)
        return sam8905_r(0, offset);
    return 0xff;  // Unmapped
}

void keyfox10_state::sam_snd_w(offs_t offset, u8 data)
{
    // T1=0: ROM (writes ignored)
    // T1=1: SAM SND chip access
    if (BIT(m_port3, 5) && offset < 8)
        sam8905_w(0, offset, data);
}

u8 keyfox10_state::sam_fx_r(offs_t offset)
{
    // T1=0: ROM data mode (0xE000-0xFFFF -> ROM 0x1E000-0x1FFFF)
    // T1=1: SAM FX chip access (only 0xE000-0xE007 active)
    if (!BIT(m_port3, 5))  // T1=0: ROM data
    {
        u32 rom_offset = 0x1E000 + offset;
        if (m_rom_watch_addr >= 0) // && rom_offset == u32(m_rom_watch_addr))
        {
            logerror("ROM watch: PC=%04X read ROM[%05X] = %02X\n",
                m_maincpu->pc(), rom_offset, m_rom[rom_offset]);
            if (m_watch_break)
                machine().debug_break();
        }
        return m_rom[rom_offset];
    }
    // T1=1: SAM chip (only first 8 bytes decoded)
    if (offset < 8)
        return sam8905_r(1, offset);
    return 0xff;  // Unmapped
}

void keyfox10_state::sam_fx_w(offs_t offset, u8 data)
{
    // T1=0: ROM (writes ignored)
    // T1=1: SAM FX chip access
    if (BIT(m_port3, 5) && offset < 8)
        sam8905_w(1, offset, data);
}

u8 keyfox10_state::sam8905_r(int chip, offs_t offset)
{
    sam8905_device &sam_dev = chip ? *m_sam_fx : *m_sam_snd;
    const char *chip_name = chip ? "FX" : "SND";

    // Pass through to actual SAM8905 device
    u8 data = sam_dev.read(offset);
    LOGMASKED(LOG_SAM, "SAM[%s] read [%d] = 0x%02X PC=%04X\n", chip_name, offset, data, m_maincpu->pc());

    return data;
}

void keyfox10_state::sam8905_w(int chip, offs_t offset, u8 data)
{
    sam8905_state &sam = m_sam[chip];
    sam8905_device &sam_dev = chip ? *m_sam_fx : *m_sam_snd;
    const char *chip_name = chip ? "FX" : "SND";

    // Pass through to actual SAM8905 device
    sam_dev.write(offset, data);

    u8 a2 = BIT(offset, 2);
    u8 a1 = BIT(offset, 1);
    u8 a0 = BIT(offset, 0);
    bool sel_aram = BIT(sam.ctrl, 1);  // SEL bit

    if (a2)
    {
        // A2=1: Write control byte
        sam.ctrl = data;
        LOGMASKED(LOG_SAM, "SAM[%s] ctrl = 0x%02X (SSR=%d IDL=%d SEL=%s WR=%s) PC=%04X\n",
            chip_name, data,
            BIT(data, 3),  // SSR
            BIT(data, 2),  // IDL
            BIT(data, 1) ? "A-RAM" : "D-RAM",  // SEL
            BIT(data, 0) ? "write" : "read",  // WR
            m_maincpu->pc());
        return;
    }

    if (a1 == 0 && a0 == 0)
    {
        // A2=0, A1=0, A0=0: Write address selection byte
        sam.addr = data;
        LOGMASKED(LOG_SAM, "SAM[%s] addr = 0x%02X PC=%04X\n", chip_name, data, m_maincpu->pc());
        return;
    }

    // Data write to A-RAM or D-RAM
    if (sel_aram)
    {
        // A-RAM: 15-bit words, accessed via 2 bytes
        if (a1 == 0 && a0 == 1)
        {
            // LSB
            sam.aram[sam.addr % 256] = (sam.aram[sam.addr % 256] & 0x7f00) | data;
            LOGMASKED(LOG_SAM, "SAM[%s] A-RAM[0x%02X] write LSB = 0x%02X\n", chip_name, sam.addr, data);
        }
        else if (a1 == 1 && a0 == 0)
        {
            // MSB (bit 7 undefined, mask to 7 bits)
            sam.aram[sam.addr % 256] = (sam.aram[sam.addr % 256] & 0x00ff) | ((data & 0x7f) << 8);
            LOGMASKED(LOG_SAM, "SAM[%s] A-RAM[0x%02X] write MSB = 0x%02X -> word = 0x%04X PC=%04X\n",
                chip_name, sam.addr, data, sam.aram[sam.addr % 256], m_maincpu->pc());
            // Watch check
            if (m_sam_watch_aram >= 0 && (sam.addr % 256) == u8(m_sam_watch_aram))
            {
                logerror("A-RAM watch: PC=%04X SAM[%s] A-RAM[%02X] = %04X (alg %d, word %d)\n",
                    m_maincpu->pc(), chip_name, sam.addr, sam.aram[sam.addr % 256],
                    sam.addr / 32, sam.addr % 32);
                if (m_watch_break)
                    machine().debug_break();
            }
        }
    }
    else
    {
        // D-RAM: 19-bit words, accessed via 3 bytes
        if (a1 == 0 && a0 == 1)
        {
            // LSB
            sam.dram[sam.addr % 256] = (sam.dram[sam.addr % 256] & 0x07ff00) | data;
            LOGMASKED(LOG_SAM, "SAM[%s] D-RAM[0x%02X] write LSB = 0x%02X\n", chip_name, sam.addr, data);
        }
        else if (a1 == 1 && a0 == 0)
        {
            // NSB
            sam.dram[sam.addr % 256] = (sam.dram[sam.addr % 256] & 0x0700ff) | (data << 8);
            LOGMASKED(LOG_SAM, "SAM[%s] D-RAM[0x%02X] write NSB = 0x%02X\n", chip_name, sam.addr, data);
        }
        else if (a1 == 1 && a0 == 1)
        {
            // MSB (5 high bits undefined, mask to 3 bits)
            sam.dram[sam.addr % 256] = (sam.dram[sam.addr % 256] & 0x00ffff) | ((data & 0x07) << 16);
            LOGMASKED(LOG_SAM, "SAM[%s] D-RAM[0x%02X] write MSB = 0x%02X -> word = 0x%05X PC=%04X\n",
                chip_name, sam.addr, data, sam.dram[sam.addr % 256], m_maincpu->pc());
            // Watch check
            if (m_sam_watch_dram >= 0) // && (sam.addr % 256) == u8(m_sam_watch_dram))
            {
                logerror("D-RAM watch: PC=%04X SAM[%s] D-RAM[%02X] = %05X (slot %d, word %d)\n",
                    m_maincpu->pc(), chip_name, sam.addr, sam.dram[sam.addr % 256],
                    sam.addr / 16, sam.addr % 16);
                if (m_watch_break)
                    machine().debug_break();
            }
        }
    }
}

void keyfox10_state::log_gal_inputs(bool rd, bool wr, bool psen, u16 addr)
{
    // Build GAL input byte:
    // bit 0: ~RD   (IN1) - active low
    // bit 1: ~WR   (IN2) - active low
    // bit 2: ~PSEN (IN3) - active low
    // bit 3: A13   (IN4)
    // bit 4: A14   (IN5)
    // bit 5: A15   (IN6)
    // bit 6: T0    (IN7) - P3.4
    // bit 7: T1    (IN8) - P3.5
    // IN9, IN10 always LOW (not captured)

    u8 inputs = 0;
    inputs |= rd   ? 0x00 : 0x01;     // ~RD active low
    inputs |= wr   ? 0x00 : 0x02;     // ~WR active low
    inputs |= psen ? 0x00 : 0x04;     // ~PSEN active low
    inputs |= BIT(addr, 13) << 3;     // A13
    inputs |= BIT(addr, 14) << 4;     // A14
    inputs |= BIT(addr, 15) << 5;     // A15
    inputs |= BIT(m_port3, 4) << 6;   // T0
    inputs |= BIT(m_port3, 5) << 7;   // T1

    // Check if we've seen this combination before
    if (!BIT(m_gal_seen[inputs >> 5], inputs & 0x1f))
    {
        m_gal_seen[inputs >> 5] |= 1U << (inputs & 0x1f);

        // Log to file
        if (m_gal_log)
        {
            const char *access_type = !psen ? "PSEN" : (!rd ? "RD" : "WR");
            fprintf(m_gal_log, "%d %d %d %d %d %d %d %d | 0x%02X | %s 0x%04X\n",
                BIT(inputs, 0), BIT(inputs, 1), BIT(inputs, 2),
                BIT(inputs, 5), BIT(inputs, 4), BIT(inputs, 3),  // A15 A14 A13 order
                BIT(inputs, 6), BIT(inputs, 7),
                inputs, access_type, addr);
            fflush(m_gal_log);
        }
    }
}

void keyfox10_state::machine_start()
{
    // Open GAL log file
    //m_gal_log = fopen("gal_inputs.log", "w");
    if (m_gal_log)
    {
        fprintf(m_gal_log, "# Keyfox10 GAL16V8 input capture\n");
        fprintf(m_gal_log, "# Format: ~RD ~WR ~PSEN A15 A14 A13 T0 T1 | hex | access\n");
        fprintf(m_gal_log, "# 0=active, 1=inactive for active-low signals\n");
    }

    // Memory tap for slot state not working with shared RAM
    // Will monitor slot state via D-RAM watch instead

    // Install memory taps to capture all accesses
    m_maincpu->space(AS_PROGRAM).install_read_tap(0x0000, 0xffff, "gal_psen",
        [this](offs_t offset, u8 &data, u8 mem_mask) {
            log_gal_inputs(true, true, false, offset);  // PSEN active
        });

    m_maincpu->space(AS_DATA).install_read_tap(0x0000, 0xffff, "gal_rd",
        [this](offs_t offset, u8 &data, u8 mem_mask) {
            log_gal_inputs(false, true, true, offset);  // RD active
        });

    m_maincpu->space(AS_DATA).install_write_tap(0x0000, 0xffff, "gal_wr",
        [this](offs_t offset, u8 &data, u8 mem_mask) {
            log_gal_inputs(true, false, true, offset);  // WR active
        });

    // Initialize FX SRAM (32KB for delay/reverb buffers)
    m_fx_sram = std::make_unique<uint8_t[]>(FX_SRAM_SIZE);
    std::fill_n(m_fx_sram.get(), FX_SRAM_SIZE, 0);

    // Initialize FX input sample (SND -> FX sample transfer)
    m_fx_input_sample = 0;

    // Set FX chip to slave mode - it runs synchronized with SND chip
    m_sam_fx->set_slave_mode(true);

    // Save state
    save_item(NAME(m_port0));
    save_item(NAME(m_port1));
    save_item(NAME(m_port2));
    save_item(NAME(m_port3));
    save_item(NAME(m_midi_rxd));
    save_item(NAME(m_disp_sr));
    save_item(NAME(m_fx_input_sample));
    save_pointer(NAME(m_fx_sram), FX_SRAM_SIZE);
    save_item(NAME(m_panel_sr));
    save_item(NAME(m_panel_latch));
    save_item(NAME(m_btn_state));
    save_item(NAME(m_kbd_sr));
    save_item(NAME(m_kbd_state));
    save_item(NAME(m_beat_sr));

    // Resolve display outputs and initialize
    m_digits.resolve();
    m_digit_dp.resolve();
    m_beat_leds.resolve();
    m_panel_leds.resolve();
    update_display();
    update_panel_leds();
}

void keyfox10_state::dump_sam_memory() const
{
    const char *chip_names[2] = {"snd", "fx"};

    for (int chip = 0; chip < 2; chip++)
    {
        const sam8905_state &sam = m_sam[chip];
        char filename[64];

        // Dump D-RAM: 16 slots × 16 words × 19 bits
        snprintf(filename, sizeof(filename), "sam_%s_dram.log", chip_names[chip]);
        FILE *f = fopen(filename, "w");
        if (f)
        {
            fprintf(f, "# SAM8905 %s D-RAM dump\n", chip == 0 ? "SND" : "FX");
            fprintf(f, "# 16 slots × 16 words × 19 bits\n");
            fprintf(f, "# Format: slot.word = value (hex/decimal)\n\n");

            for (int slot = 0; slot < 16; slot++)
            {
                fprintf(f, "Slot %d:\n", slot);
                for (int word = 0; word < 16; word++)
                {
                    int addr = slot * 16 + word;
                    u32 val = sam.dram[addr];
                    fprintf(f, "  [%02X] %2d.%02d = 0x%05X (%6d)\n",
                        addr, slot, word, val, val);
                }
                fprintf(f, "\n");
            }
            fclose(f);
        }

        // Dump A-RAM: 8 slots × 32 words × 15 bits (44kHz mode)
        // or 4 slots × 64 words (22kHz mode)
        snprintf(filename, sizeof(filename), "sam_%s_aram.log", chip_names[chip]);
        f = fopen(filename, "w");
        if (f)
        {
            fprintf(f, "# SAM8905 %s A-RAM dump\n", chip == 0 ? "SND" : "FX");
            fprintf(f, "# 44kHz mode: 8 slots × 32 words × 15 bits\n");
            fprintf(f, "# 22kHz mode: 4 slots × 64 words × 15 bits\n");
            fprintf(f, "# Format: slot.word = value (hex/decimal)\n\n");

            // Show as 8 slots × 32 words (44kHz interpretation)
            fprintf(f, "=== 44kHz mode (8 slots × 32 words) ===\n\n");
            for (int slot = 0; slot < 8; slot++)
            {
                fprintf(f, "Slot %d:\n", slot);
                for (int word = 0; word < 32; word++)
                {
                    int addr = slot * 32 + word;
                    u16 val = sam.aram[addr];
                    fprintf(f, "  [%02X] %d.%02d = 0x%04X (%5d)\n",
                        addr, slot, word, val, val);
                }
                fprintf(f, "\n");
            }

            // Also show as 4 slots × 64 words (22kHz interpretation)
            fprintf(f, "=== 22kHz mode (4 slots × 64 words) ===\n\n");
            for (int slot = 0; slot < 4; slot++)
            {
                fprintf(f, "Slot %d:\n", slot);
                for (int word = 0; word < 64; word++)
                {
                    int addr = slot * 64 + word;
                    u16 val = sam.aram[addr];
                    fprintf(f, "  [%02X] %d.%02d = 0x%04X (%5d)\n",
                        addr, slot, word, val, val);
                }
                fprintf(f, "\n");
            }
            fclose(f);
        }
    }

    // Dump entire FX SRAM as WAV file (32KB as 16-bit samples)
    // Raw dump - each consecutive byte pair forms a 16-bit sample (high, low)
    if (m_fx_sram)
    {
        FILE *f = fopen("fx_sram_dump.wav", "wb");
        if (f)
        {
            // 32KB SRAM = 16K 16-bit samples (consecutive byte pairs)
            uint32_t num_samples = FX_SRAM_SIZE / 2;

            // WAV header for mono 16-bit 22050Hz
            uint32_t sample_rate = 22050;
            uint16_t num_channels = 1;
            uint16_t bits_per_sample = 16;
            uint32_t byte_rate = sample_rate * num_channels * bits_per_sample / 8;
            uint16_t block_align = num_channels * bits_per_sample / 8;
            uint32_t data_size = num_samples * 2;
            uint32_t chunk_size = 36 + data_size;

            // RIFF header
            fwrite("RIFF", 1, 4, f);
            fwrite(&chunk_size, 4, 1, f);
            fwrite("WAVE", 1, 4, f);

            // fmt chunk
            fwrite("fmt ", 1, 4, f);
            uint32_t fmt_size = 16;
            fwrite(&fmt_size, 4, 1, f);
            uint16_t audio_format = 1;  // PCM
            fwrite(&audio_format, 2, 1, f);
            fwrite(&num_channels, 2, 1, f);
            fwrite(&sample_rate, 4, 1, f);
            fwrite(&byte_rate, 4, 1, f);
            fwrite(&block_align, 2, 1, f);
            fwrite(&bits_per_sample, 2, 1, f);

            // data chunk
            fwrite("data", 1, 4, f);
            fwrite(&data_size, 4, 1, f);

            // Write SRAM as 16-bit samples (consecutive byte pairs: high, low)
            for (size_t i = 0; i < FX_SRAM_SIZE; i += 2)
            {
                int8_t hi_byte = static_cast<int8_t>(m_fx_sram[i]);
                uint8_t lo_byte = m_fx_sram[i + 1];
                int16_t sample = (static_cast<int16_t>(hi_byte) << 8) | lo_byte;
                fwrite(&sample, 2, 1, f);
            }

            fclose(f);
            printf("Wrote FX SRAM dump to fx_sram_dump.wav (%u 16-bit samples, 32KB)\n", num_samples);
        }
    }
}

// SAM8905 external waveform ROM read handlers
// Address format: WA[19:0] = { WAVE[7:0], PHI[11:0] }
// Per SAM8905 programmer's guide: "8 bits + 12 upper phase bits" = 20-bit address
// Hardware mapping: WA19=ROM select (WAVE[7]), WA0-WA1=skipped (fractional)
// Testing 1024 samples/wave: ROM = (WAVE[6:0] << 10) | PHI[11:2]
// Data: 8-bit ROM → sign-extended to 12-bit
u16 keyfox10_state::sam_snd_waveform_r(offs_t offset)
{
    // offset = 20-bit address from SAM8905: { WAVE[7:0], PHI[11:0] }
    // Per programmer's guide: 8 bits WAVE + 12 bits PHI = 20 bits
    // After skipping WA[1:0] (fractional): 17 bits for ROM
    // Option A (512 samples/wave): ROM = (WAVE[7:0] << 9) | PHI[10:2]
    // Option B (1024 samples/wave): ROM = (WAVE[6:0] << 10) | PHI[11:2]
    // Testing Option B based on "12 upper phase bits" = 10 bits after skipping 2
    uint32_t wave = (offset >> 12) & 0x7F;   // WAVE[6:0] - 7 bits (128 wave slots)
    uint32_t phi = offset & 0xFFF;           // PHI[11:0]
    uint32_t phi_int = (phi >> 2) & 0x3FF;   // PHI[11:2] - 10 bits (1024 samples/wave)
    uint32_t rom_addr = (wave << 10) | phi_int;  // 17-bit ROM address
    bool use_rhythm = BIT(offset, 19);  // WA19 = WAVE[7] selects ROM

    static int log_count = 0;
    // Only log when WAVE is non-zero (actual waveform being played)
    if (wave != 0 && log_count < 200) {
        LOGMASKED(LOG_SAM, "SAM waveform: offset=%05X WAVE=%02X PHI=%03X phi_int=%03X rom=%05X\n",
                  offset, wave, phi, phi_int, rom_addr);
        log_count++;
    }

    u8 sample;
    if (use_rhythm)
        sample = m_rhythms_rom[rom_addr];
    else
        sample = m_samples_rom[rom_addr];

    // Sign-extend 8-bit to 12-bit (shift to upper bits, then sign-extend)
    return (int16_t)(int8_t)sample << 4;
}

// Callback from sound SAM: captures L/R samples for FX SAM input
// Stores current sample for FX chip to read via waveform callback
void keyfox10_state::sam_snd_sample_out(uint32_t data)
{
    // Unpack L/R from 32-bit value: upper 16 = L, lower 16 = R
    // For now we only use R channel (mono reverb input)
    uint16_t sample_r = uint16_t(data & 0xFFFF);
    //uint16_t sample_l = int16_t(data >> 16);

    // Byte swap for proper endianness
    //int16_t sample = ((sample_l >> 8) & 0xFF) | ((sample_l << 8) & 0xFF00);
    uint16_t sample = ((sample_r >> 8) & 0xFF) | ((sample_r << 8) & 0xFF00);

    // Store current sample for FX chip waveform reads
    m_fx_input_sample = sample;

    // FX chip runs at 22kHz, SND at 44kHz - call process_frame every other sample
    static int snd_sample_count = 0;
    if ((snd_sample_count++ & 1) == 0) {
        int32_t fx_l, fx_r;
        m_sam_fx->process_frame(fx_l, fx_r);
    }
}

// FX SAM external SRAM write callback
// Hardware note: Real SRAM is 8-bit (IC19), connected to WD3-WD10.
// WD0-2 are pulled to GND. WD11 is generated by logic: WDH11 = (~WAH0) AND WDH10
// This provides sign extension when WAH0=0. Only bits 10:3 are stored in SRAM.
// We store full 12-bit for slightly better audio quality than real hardware.
void keyfox10_state::sam_fx_waveform_w(offs_t offset, u16 data)
{
    // offset is 15-bit address (0-32767)
    if (offset < FX_SRAM_SIZE) {
        // Hardware: SRAM stores only 8 bits (bits 10:3 of 12-bit SAM data)
        // SAM outputs 12-bit data, SRAM captures D7:D0 from WDH10:WDH3
        // IMPORTANT: Store as UNSIGNED to preserve bit pattern (0xFF = 255, not -1)
        uint8_t data_8bit = (data >> 3) & 0xFF;
        m_fx_sram[offset] = data_8bit;

        // Debug: trace significant SRAM writes (values > threshold)
        static int sram_write_trace = 0;
        if (data_8bit != 0 && data_8bit > 10 && sram_write_trace < 100) {
            logerror("FX SRAM write: addr=%04X 12bit=%d 8bit=%u\n",
                offset, (int16_t)data, data_8bit);
            sram_write_trace++;
        }
    } else {
        abort();
    }
}

u16 keyfox10_state::sam_fx_waveform_r(offs_t offset)
{
    // Check WA19 (bit 19) - if set, return input from sound SAM
    // WA[19:0] = { WAVE[7:0], PHI[11:0] }
    // When WA19=1 (WAVE[7]=1, i.e., WF >= 0x80), reads from input sample
    // PHI[0] selects byte: 0=high byte, 1=low byte of 16-bit sample
    if (offset & 0x80000)  // WA19 set - read input sample
    {
        // Read current input sample (synchronized via process_frame)
        uint16_t sample = m_fx_input_sample;

        // PHI[0] selects byte: 0=high byte, 1=low byte
        uint8_t sample_8bit;
        if (offset & 1) {
            // Low byte
            sample_8bit = sample & 0xFF;
        }
        else
        {
            // High byte
            sample_8bit = (sample >> 8) & 0xFF;
        }

        // Place 8-bit value at bits 10:3, bits 2:0 are grounded (always 0)
        uint16_t result = ((uint16_t)sample_8bit) << 3;

        // Sign extension for high byte read
        if ((offset & 1) == 0 && (sample_8bit & 0x80)) {
            result |= 0x800;
        }

        return result & 0xFFF;
    }

    // Check for SRAM access: WF < 0x80 means external SRAM (not input shift registers)
    // SRAM WA0-WA14 connects to lower 15 bits of SAM's 20-bit address bus
    // 15-bit SRAM address = (WF[2:0] << 12) | PHI[11:0]
    // WAH0 = PHI[0] for sign extension control
    uint32_t wave = (offset >> 12) & 0xFF;   // WAVE[7:0]
    if (wave < 0x80) {
        // SRAM access: lower 15 bits of (WF << 12 | PHI)
        uint32_t sram_addr = ((wave & 0x7) << 12) | (offset & 0xFFF);
        if (sram_addr < FX_SRAM_SIZE && m_fx_sram) {
            // Hardware: SRAM stores 8 bits, mapped to WDH10:WDH3
            // WDH0-2 are grounded (always 0)
            // WDH11 sign extension depends on WAH0 (=PHI[0])
            // IMPORTANT: Read as UNSIGNED to preserve bit pattern
            uint8_t sram_8bit = m_fx_sram[sram_addr] & 0xFF;
            uint16_t result = ((uint16_t)sram_8bit) << 3;  // Place at bits 10:3 (result is 0-2040)

            // WAH0 = PHI[4] = (offset >> 4) & 1
            //bool wah0 = (offset >> 4) & 1;
            bool wah0 = (offset & 1);

            if (!wah0) {
                // WAH0=0: WDH11 = WDH10 (sign extension from bit 10)
                // Bit 10 of result = bit 7 of sram_8bit
                if (sram_8bit & 0x80)
                    result |= 0x800;
            }
            // WAH0=1: WDH11 = 0 (already 0 from unsigned shift)

            // Debug: trace SRAM reads (limited)
            static int sram_read_trace = 0;
            if (sram_read_trace < 30) {
                logerror("FX SRAM read: addr=%04X wah0=%d raw=%u result=%d\n",
                    sram_addr, wah0, sram_8bit, (int16_t)result);
                sram_read_trace++;
            }
            return result & 0xFFF;
        }
    }

    // ROM access fallback: (WAVE[6:0] << 10) | PHI[11:2]
    //uint32_t phi_int = ((offset & 0xFFF) >> 2) & 0x3FF;  // PHI[11:2] - 10 bits
    //uint32_t rom_addr = ((wave & 0x7F) << 10) | phi_int;

    //u8 sample = m_samples_rom[rom_addr];
    //return (int16_t)(int8_t)sample << 4;

    abort();
    return 0;
}

static INPUT_PORTS_START(keyfox10)
    // 10x 74HC299 bidirectional shift registers for button panel (80 buttons)
    // Chain order: KF2:4 -> KF2:3 -> KF2:2 -> KF2:1 -> KF1:12 -> KF1:11 -> KF1:10 -> KF1:2 -> KF1:1 -> KF3:1

    PORT_START("PANEL0")  // KF2:4 - Upper Solo Controls
    PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("PRESET 1")      PORT_CODE(KEYCODE_1)
    PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("PRESET 2")      PORT_CODE(KEYCODE_2)
    PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("REV.MODE")      PORT_CODE(KEYCODE_3)
    PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("SOLO")          PORT_CODE(KEYCODE_4)
    PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("PORTAMENTO")
    PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("WERSI-CHORD")
    PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("SUSTAIN U")
    PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("MEM-CARD U")

    PORT_START("PANEL1")  // KF2:3 - Upper Solo Voices 2
    PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("SYN.BRASS")
    PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("MUSETTE")
    PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("JAZZ-FLUTE")
    PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("SYNTHE")
    PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("JAZZ-GUIT")
    PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("VOCAL")
    PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("HAWAI-GUIT")
    PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("STRINGS")

    PORT_START("PANEL2")  // KF2:2 - Upper Solo Voices 1
    PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("VIBES")
    PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("SAXOPHON")
    PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("HONKY-TONK")
    PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("CLARINET")
    PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("E-PIANO")
    PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("TRUMPET")
    PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("PIANO")
    PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("TROMBONE")

    PORT_START("PANEL3")  // KF2:1 - Upper Drawbars
    PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("WV.SLOW")
    PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("ENSEMBLE")
    PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("WV.FAST")
    PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("CHURCH")
    PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("PERC 2/3")
    PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("FULL ORGAN")
    PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("FLUTE 4 U")
    PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("JAZZ 2")

    PORT_START("PANEL4")  // KF1:12 - Mixed Upper + Lower controls
    PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("FLUTE 16")
    PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("FLUTE 8 U")
    PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("DYNAMIC")
    PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("JAZZ 1")
    PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("SUSTAIN L")
    PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("SPLIT")
    PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("DRUMS")
    PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("MEM-CARD BL")

    PORT_START("PANEL5")  // KF1:11 - Lower Section
    PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("GUITAR")
    PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("SYN.STRINGS")
    PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("FLUTE 2")
    PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("STRINGS L")
    PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("FLUTE 4 L")
    PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("E-PIANO L")
    PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("FLUTE 8 L")
    PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("PIANO L")

    PORT_START("PANEL6")  // KF1:10 - Bass Section
    PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("SUSTAIN B")
    PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("AUTO BASS")
    PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("SLAP BASS")
    PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("BASS 2")
    PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("SYN.BASS")
    PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("BASS 1")
    PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("TRANSPOSE")
    PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("MIDI")

    PORT_START("PANEL7")  // KF1:2 - Rhythm Section 2
    PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("2ND BANK")
    PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("TUNE")
    PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("MARCH")
    PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("MEM-CARD R")
    PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("WALTZ")
    PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("BEGUINE")
    PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("SAMBA")
    PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("BOSSA")

    PORT_START("PANEL8")  // KF1:1 - Rhythm Section 1
    PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("RHYTHM")
    PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("BALLAD")
    PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("BIG BAND")
    PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("JAZZ ROCK")
    PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("FOXTROT")
    PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("8 BEAT")
    PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("ROCK'N'ROLL")
    PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("DISCO")

    PORT_START("PANEL9")  // KF3:1 - Accompaniment Controls
    PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("ACC. 3")
    PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("MEM-CARD S")
    PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("ACC. 2")
    PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("SYNC START")
    PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("ACC. 1")
    PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("FILL")
    PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("ACC. BASS")
    PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("INTRO/END")

    // Keyboard matrix: 4x 74HC175 + 2x 74HC251 multiplexers
    // Increment/decrement buttons for level controls
    // Scanned via CLK_SW, read via SENSE (P1.7)

    PORT_START("KBDMTX0")  // IC4 - Level controls 1
    //PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)  // D0 unused
    PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("UNKNWON")
    PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("RHYTHM-")   PORT_CODE(KEYCODE_Q)
    PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("RHYTHM+")   PORT_CODE(KEYCODE_W)
    PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("ACC+")      PORT_CODE(KEYCODE_E)
    PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("ACC-")      PORT_CODE(KEYCODE_R)
    PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("TEMPO+")    PORT_CODE(KEYCODE_A)
    PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("TEMPO-")    PORT_CODE(KEYCODE_S)
    PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("START/S")   PORT_CODE(KEYCODE_D)

    PORT_START("KBDMTX1")  // IC11 - Level controls 2
    PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("UPPER+")    PORT_CODE(KEYCODE_H)
    PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("UPPER-")    PORT_CODE(KEYCODE_J)
    PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("LOWER-")    PORT_CODE(KEYCODE_K)
    PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("LOWER+")    PORT_CODE(KEYCODE_L)
    PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("BASS-")     PORT_CODE(KEYCODE_F)
    PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("BASS+")     PORT_CODE(KEYCODE_G)
    PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("SWELL0")
    PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("SWELL1")
    //PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)  // D6 unused
    //PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNUSED)  // D7 unused
INPUT_PORTS_END

void keyfox10_state::keyfox10(machine_config &config)
{
    I80C32(config, m_maincpu, 16_MHz_XTAL);
    m_maincpu->set_addrmap(AS_PROGRAM, &keyfox10_state::program_map);
    m_maincpu->set_addrmap(AS_DATA, &keyfox10_state::data_map);
    m_maincpu->port_in_cb<0>().set(FUNC(keyfox10_state::port0_r));
    m_maincpu->port_in_cb<1>().set(FUNC(keyfox10_state::port1_r));
    m_maincpu->port_in_cb<2>().set(FUNC(keyfox10_state::port2_r));
    m_maincpu->port_in_cb<3>().set(FUNC(keyfox10_state::port3_r));
    m_maincpu->port_out_cb<0>().set(FUNC(keyfox10_state::port0_w));
    m_maincpu->port_out_cb<1>().set(FUNC(keyfox10_state::port1_w));
    m_maincpu->port_out_cb<2>().set(FUNC(keyfox10_state::port2_w));
    m_maincpu->port_out_cb<3>().set(FUNC(keyfox10_state::port3_w));

    // MIDI
    midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
    mdin.rxd_handler().set(FUNC(keyfox10_state::midi_rxd_w));
    MIDI_PORT(config, "mdout", midiout_slot, "midiout");

    // Sound - two SAM8905 DSP chips
    SPEAKER(config, "lspeaker").front_left();
    SPEAKER(config, "rspeaker").front_right();

    // SAM8905 SND - sound generation (at 0x8000 when T1=1)
    // Clock: 22.5792 MHz / 1024 = 22.05 kHz sample rate
    SAM8905(config, m_sam_snd, 22'579'200 * 2, 1024);
    m_sam_snd->waveform_read_callback().set(FUNC(keyfox10_state::sam_snd_waveform_r));
    m_sam_snd->sample_output_callback().set(FUNC(keyfox10_state::sam_snd_sample_out));
    m_sam_snd->add_route(0, "lspeaker", 1.0);  // Dry L - SILENCED FOR DEBUG
    m_sam_snd->add_route(1, "rspeaker", 1.0);  // Dry R - SILENCED FOR DEBUG

    // SAM8905 FX - effects processor (at 0xE000 when T1=1)
    // FX chip has 32KB SRAM for delay/reverb buffers
    // Runs in slave mode - triggered by SND chip after each frame (set in machine_start)
    SAM8905(config, m_sam_fx, 22'579'200, 1024);
    m_sam_fx->waveform_read_callback().set(FUNC(keyfox10_state::sam_fx_waveform_r));
    m_sam_fx->waveform_write_callback().set(FUNC(keyfox10_state::sam_fx_waveform_w));
    m_sam_fx->add_route(0, "lspeaker", 1.0);  // Wet L
    m_sam_fx->add_route(1, "rspeaker", 1.0);  // Wet R

    // 7-segment display layout
    config.set_default_layout(layout_keyfox10);
}

ROM_START(keyfox10)
    ROM_REGION(0x20000, "program", 0)
    ROM_LOAD("kf10_ic27_v2.bin", 0x0000, 0x20000, CRC(c04e40a9) SHA1(668d40761658f1863ab028ed317141839d1075ac))

    ROM_REGION(0x20000, "samples", 0)  // IC17 "Samples" 128KB
    ROM_LOAD("kf10_ic17.bin", 0x0000, 0x20000, CRC(fa8f2632) SHA1(c252ebd54de7f14c4b11014e5bebdb440cd54690))

    ROM_REGION(0x20000, "rhythms", 0)  // IC14 "Rhythmen" 128KB
    ROM_LOAD("max1_ic14.bin", 0x0000, 0x20000, CRC(6b8c4eb8) SHA1(097e5612fd75dd82efc18ffc66694bf7f31cfe5a))
ROM_END

} // anonymous namespace

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY  FULLNAME      FLAGS
SYST( 1990, keyfox10, 0,      0,      keyfox10, keyfox10, keyfox10_state, empty_init, "Wersi", "Keyfox 10",  MACHINE_NOT_WORKING )
