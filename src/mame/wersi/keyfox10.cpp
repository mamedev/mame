// license:BSD-3-Clause
// copyright-holders:

#include "emu.h"
#include "cpu/mcs51/i80c52.h"
#include "bus/midi/midi.h"
#include "sound/sam8905.h"
#include "speaker.h"
#include "debugger.h"

#define LOG_SERIAL (1U << 1)
#define LOG_PORT   (1U << 2)
#define LOG_SAM    (1U << 3)
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

    void midi_rxd_w(int state) { m_midi_rxd = state; }

    required_device<i80c32_device> m_maincpu;
    required_device<sam8905_device> m_sam_snd;
    required_device<sam8905_device> m_sam_fx;
    required_device<midi_port_device> m_midi_out;
    required_region_ptr<u8> m_rom;
    required_region_ptr<u8> m_samples_rom;  // IC17 "Samples" 128KB
    required_region_ptr<u8> m_rhythms_rom;  // IC14 "Rhythmen" 128KB
    u8 m_port0 = 0xff;
    u8 m_port1 = 0xff;
    u8 m_port2 = 0xff;
    u8 m_port3 = 0xff;
    u8 m_midi_rxd = 1;  // MIDI RX bit (idle high)

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
    // P1.7 (SENSE) is input - directly driven low based on MODE
    // When MODE=0, SENSE should be low to indicate "data ready"
    u8 data = m_port1 & 0x7f;  // Clear SENSE bit
    // Return SENSE low (0) to prevent stuck in scanning loop
    // TODO: implement proper switch matrix scanning
    LOGMASKED(LOG_PORT, "P1 read: 0x%02X (SENSE=%d)\n", data, 0);
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
    u8 changed = m_port1 ^ data;
    u8 rising = changed & data;

    if (changed)
    {
        LOGMASKED(LOG_IO, "P1 write: 0x%02X [%s%s%s%s%s%s%s]\n",
            data,
            (changed & P1_CLK_SW)   ? ((data & P1_CLK_SW)   ? "CLK_SW↑ "   : "CLK_SW↓ ")   : "",
            (changed & P1_CLK_BEAT) ? ((data & P1_CLK_BEAT) ? "CLK_BEAT↑ " : "CLK_BEAT↓ ") : "",
            (changed & P1_CLK_LED)  ? ((data & P1_CLK_LED)  ? "CLK_LED↑ "  : "CLK_LED↓ ")  : "",
            (changed & P1_CLK_DISP) ? ((data & P1_CLK_DISP) ? "CLK_DISP↑ " : "CLK_DISP↓ ") : "",
            (changed & P1_ENABLE)   ? ((data & P1_ENABLE)   ? "ENABLE↑ "   : "ENABLE↓ ")   : "",
            (changed & P1_MODE)     ? ((data & P1_MODE)     ? "MODE↑ "     : "MODE↓ ")     : "",
            (changed & P1_DATA)     ? ((data & P1_DATA)     ? "DATA↑ "     : "DATA↓ ")     : "");
    }

    // CLK_DISP rising edge: shift DATA bit into display shift registers
    if (rising & P1_CLK_DISP)
    {
        disp_shift_clock();
    }

    m_port1 = data;
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

    // Get carry bits from each register before shifting
    u8 ic7_carry = BIT(m_disp_sr[0], 7);
    u8 ic6_carry = BIT(m_disp_sr[1], 7);

    // Shift all registers left, insert carry/data into LSB
    m_disp_sr[2] = (m_disp_sr[2] << 1) | ic6_carry;  // IC10: receives from IC6
    m_disp_sr[1] = (m_disp_sr[1] << 1) | ic7_carry;  // IC6: receives from IC7
    m_disp_sr[0] = (m_disp_sr[0] << 1) | data_in;    // IC7: receives DATA

    LOGMASKED(LOG_IO, "DISP shift: DATA=%d -> IC7=0x%02X IC6=0x%02X IC10=0x%02X\n",
        data_in, m_disp_sr[0], m_disp_sr[1], m_disp_sr[2]);
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
#ifdef ENABLE_SAM_FX
    // T1=1: SAM chip (only first 8 bytes decoded)
    if (offset < 8)
        return sam8905_r(1, offset);
    return 0xff;  // Unmapped
#else
    return 0;
#endif
}

void keyfox10_state::sam_fx_w(offs_t offset, u8 data)
{
#ifdef ENABLE_SAM_FX
    // T1=0: ROM (writes ignored)
    // T1=1: SAM FX chip access
    if (BIT(m_port3, 5) && offset < 8)
        sam8905_w(1, offset, data);
#else
    (void)offset;
    (void)data;
#endif
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
    m_gal_log = fopen("gal_inputs.log", "w");
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

    // Save state
    save_item(NAME(m_port0));
    save_item(NAME(m_port1));
    save_item(NAME(m_port2));
    save_item(NAME(m_port3));
    save_item(NAME(m_midi_rxd));
    save_item(NAME(m_disp_sr));
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

u16 keyfox10_state::sam_fx_waveform_r(offs_t offset)
{
    // FX chip uses same ROM mapping as SND: (WAVE[6:0] << 10) | PHI[11:2]
    uint32_t wave = (offset >> 12) & 0x7F;   // WAVE[6:0] - 7 bits
    uint32_t phi_int = ((offset & 0xFFF) >> 2) & 0x3FF;  // PHI[11:2] - 10 bits
    uint32_t rom_addr = (wave << 10) | phi_int;
    bool use_rhythm = BIT(offset, 19);

    u8 sample;
    if (use_rhythm)
        sample = m_rhythms_rom[rom_addr];
    else
        sample = m_samples_rom[rom_addr];

    return (int16_t)(int8_t)sample << 4;
}

static INPUT_PORTS_START(keyfox10)
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
    SAM8905(config, m_sam_snd, 22'579'200);
    m_sam_snd->waveform_read_callback().set(FUNC(keyfox10_state::sam_snd_waveform_r));
    m_sam_snd->add_route(0, "lspeaker", 1.0);
    m_sam_snd->add_route(1, "rspeaker", 1.0);

    // SAM8905 FX - effects processor (at 0xE000 when T1=1)
    SAM8905(config, m_sam_fx, 22'579'200);
    m_sam_fx->waveform_read_callback().set(FUNC(keyfox10_state::sam_fx_waveform_r));
    m_sam_fx->add_route(0, "lspeaker", 1.0);
    m_sam_fx->add_route(1, "rspeaker", 1.0);
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
