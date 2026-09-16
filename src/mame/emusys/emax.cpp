// license:BSD-3-Clause
// copyright-holders: AJR, Strikelesss
/***********************************************************************************************************************************

    Skeleton driver for E-mu Emax & Emax II samplers.
	
	- SCSI scan not working
	- Emax II does not boot to a bank due to "WARNING! Sound MemorySize Error" (digital proc./preset man. passes fail, which could be related)

***********************************************************************************************************************************/

#include "emu.h"

#include "cpu/ns32000/ns32000.h"

// hardware
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/ncr5380.h"
#include "machine/pit8253.h"
#include "machine/wd_fdc.h"
#include "machine/eepromser.h"

// scsi/rs232/midi
#include "bus/midi/midi.h"
#include "bus/rs232/rs232.h"
#include "bus/rs232/terminal.h"

// display
#include "video/hd44780.h"
#include "screen.h"
#include "emupal.h"

// disk
#include "machine/nscsi_bus.h"
#include "bus/nscsi/hd.h"
#include "imagedev/floppy.h"
#include "formats/pc_dsk.h"
#include "formats/hxchfe_dsk.h"

// logging

//#define LOG_OUTPUT_FUNC osd_printf_info
#define LOG_OUTPUT_FUNC logerror

#define LOG_INFO       (1U << 1)
#define LOG_WARN       (1U << 2)
#define LOG_DACMUX     (1U << 3)
#define LOG_ECHIP      (1U << 4)
#define LOG_HARD_DRIVE (1U << 5)
#define LOG_TIMER      (1U << 6)
#define LOG_LCD        (1U << 7)
#define LOG_BUS        (1U << 8)
#define LOG_SCN        (1U << 9)
#define LOG_LATCH      (1U << 10)
#define LOG_IRQ        (1U << 11)

#define VERBOSE_PRINTF 0
#define VERBOSE ( LOG_WARN | LOG_LCD | LOG_LATCH )
//#define VERBOSE ( LOG_WARN | LOG_TIMER | LOG_DACMUX | LOG_ECHIP | LOG_FLOPPY | LOG_HARD_DRIVE )

#define LOGEMAX(mask, ...) do { if (VERBOSE & (mask)) { if (VERBOSE_PRINTF >= 1) { printf(__VA_ARGS__); } LOGMASKED(mask, __VA_ARGS__); } } while (0)

#include "logmacro.h"

namespace {

class emax_state : public driver_device
{
public:
    emax_state(const machine_config &mconfig, device_type type, const char *tag)
        : driver_device(mconfig, type, tag)
        , m_maincpu(*this, "maincpu")
        , m_ctc(*this, "ctc")
        , m_fdc(*this, "fdc")
        , m_fdd(*this, "fdc:0:35dd")
        , m_hdc(*this, "hdc")
        , m_acia(*this, "acia1")
		, m_acia2(*this, "acia2")
        , m_rs422_port(*this, "rs422_port")
		, m_midi_out_port(*this, "midi_out_port")
        , m_lcdc(*this, "lcdc")
		, m_eeprom(*this, "eeprom")
    {
    }
	
	DECLARE_INPUT_CHANGED_MEMBER(scn_button);

    void emax(machine_config &config);
    void emaxp(machine_config &config);
    void emax2(machine_config &config);
    
	void swtime_w(int state)
	{
		LOGEMAX(LOG_IRQ, "swtime_w: %d \n", state);
		irq_w<STINT>(state);
	}
	
protected:
    virtual void machine_start() override ATTR_COLD;
    virtual void machine_reset() override ATTR_COLD;

private:
    // All active low
    enum class emaxi_ic21_latch_led_a : u8
    {
        DYNALLO_LED     = 0, // D0 (Bit 0)
        TRANSPOSE_LED   = 1, // D1 (Bit 1)
        PREDEF_LED      = 2, // D2 (Bit 2)
        ANAPROC_LED     = 3, // D3 (Bit 3)
        MASTER_LED      = 4, // D4 (Bit 4)
        SAMPLE_LED      = 5, // D5 (Bit 5)
        DIGPROC_LED     = 6, // D6 (Bit 6)
        PREMAN_LED      = 7, // D7 (Bit 7)
    };

    enum class emaxi_ic22_latch_led_b : u8
    {
        SCNDTA        = 0, // D0 (Bit 0) (active high)
        SCNCLK        = 1, // D1 (Bit 1) (active high)
        SYNC_TIME     = 2, // D2 (Bit 2) // SYNC (active high) TIME (active low)
        Unlabeled     = 3, // D3 (Bit 3)
        SIDE          = 4, // D4 (Bit 4) (active low)
        MTR           = 5, // D5 (Bit 5) (active low)
        SEQ_LED       = 6, // D6 (Bit 6) (active low)
        ENTER_LED     = 7, // D7 (Bit 7) (active low)
    };
    
    // All active low
    enum class emaxii_ic34_latch_led_a : u8
    {
        DRVSELECT_LED    = 0, // D0 (Bit 0)
        TRANSPOSE_LED    = 1, // D1 (Bit 1)
        PREDEF_LED       = 2, // D2 (Bit 2)
        DYNPROC_LED      = 3, // D3 (Bit 3)
        MASTER_LED       = 4, // D4 (Bit 4)
        SAMPLE_LED       = 5, // D5 (Bit 5)
        DIGPROC_LED      = 6, // D6 (Bit 6)
        PREMAN_LED       = 7, // D7 (Bit 7)
    };

    enum class emaxii_ic35_latch_led_b : u8
    {
        SCNDTA        = 0, // D0 (Bit 0) (active high)
        SCNCLK        = 1, // D1 (Bit 1) (active high)
        SYNC_TIME     = 2, // D2 (Bit 2) // SYNC (active high) TIME (active low)
        Unlabeled     = 3, // D3 (Bit 3)
        SIDE          = 4, // D4 (Bit 4) (active low)
        MTR           = 5, // D5 (Bit 5) (active low)
        SEQ_LED       = 6, // D6 (Bit 6) (active low)
        ENTER_LED     = 7, // D7 (Bit 7) (active low)
    };
    
    enum irq_number : unsigned
    {
        SCNINT    = 0, // scanner interrupt (active low)
        MIDINT    = 1, // MIDI/serial interrupt (active low)
        FDCINT    = 2, // floppy disk controller interrupt (active high)
        STINT     = 3, // software timer interrupt (active high)
        TGINT     = 4, // transient generation interrupt (active high)
        HDINT     = 5, // hard disk interrupt (active high)
                       // unused (tied high)
                       // unused (tied low)
    };
    
    template <irq_number IRQ> 
    void irq_w(int state)
    {
		LOGEMAX(LOG_IRQ, "IRQ %02x state=%d BEFORE latch=%02x\n",
             IRQ, state, m_irq_latch);
		
        if (state)
			m_irq_latch |= 1U << IRQ;
		else
			m_irq_latch &= ~(1U << IRQ);
		
		bool const irq_state = m_irq_latch != 0x43;

		LOGEMAX(LOG_IRQ, "IRQ %02x state=%d AFTER  latch=%02x -> CPU IRQ=%d\n",
             IRQ, state, m_irq_latch, irq_state);
		
		if (irq_state != m_irq_state)
		{
			m_irq_state = irq_state;
			m_maincpu->set_input_line(INPUT_LINE_IRQ0, m_irq_state);
		}
    }
	
	void update_acia_rxd()
	{
		int rxd = m_midi_off ? m_rs232_rxd : m_midi_rxd;
		m_acia->write_rxd(rxd);
	}
    
	void midi_rxd_w(int state)
	{
		LOGEMAX(LOG_BUS, "midi_rxd_w: %d \n", state);
		m_midi_rxd = state;
		update_acia_rxd();
	}

	void rs232_rxd_w(int state)
	{
		LOGEMAX(LOG_BUS, "rs232_rxd_w: %d \n", state);
		m_rs232_rxd = state;
		update_acia_rxd();
	}
	
    void emaxi_ic21_latch_led_a_w(u8 data);
    void emaxi_ic22_latch_led_b_w(u8 data);
	void emaxi_misc_latch_w(u8 data);
    void emaxii_ic34_latch_led_a_w(u8 data);
    void emaxii_ic35_latch_led_b_w(u16 data);
	void emaxii_misc_latch_w(u8 data);
	
	u16 emax_ii_lsi1_test_r(offs_t offset)
	{
		if (offset == 0) m_lsi1_test_count++;

		u32 val;
		switch (m_lsi1_test_count)
		{
			case 1:  val = 0x12345678; break;
			case 2:  val = 0x12345678; break;
			case 3:  val = 0xEDCBA987; break;
			case 4:  val = 0xEDCBA987; break;
			default: val = 0x12345678; break;
		}
	
		return offset == 0 ? (val & 0xFFFF) : (val >> 16);
	}

    HD44780_PIXEL_UPDATE(pixel_update);

    u8 hdc_r(offs_t offset);
    void hdc_w(offs_t offset, u8 data);
	
    u8 timer_r(offs_t offset);
    void timer_w(offs_t offset, u8 data);
	
    void mux_w(u8 data);
    void dac_w(u8 data);
    
    // These echip functions work to pass the 'bootprom diagnostics' test:
    void echip_cmd_w(u8 data);
    void echip_data_w(offs_t offset, u8 data);
    u8 echip_data_r();

    void palette_init(palette_device &palette);
    void scsihd(machine_config &config);

    void emax_periphs(address_map &map) ATTR_COLD;
    void emax_map(address_map &map) ATTR_COLD;
    void emaxp_map(address_map &map) ATTR_COLD;
    void emax2_map(address_map &map) ATTR_COLD;

    required_device<cpu_device> m_maincpu;
    required_device<pit8254_device> m_ctc;
    required_device<wd1772_device> m_fdc;
    required_device<floppy_image_device> m_fdd;
    optional_device<ncr5380_device> m_hdc;
    required_device<acia6850_device> m_acia;
	optional_device<acia6850_device> m_acia2;
    optional_device<rs232_port_device> m_rs422_port;
	optional_device<midi_port_device> m_midi_out_port;
    required_device<hd44780_device> m_lcdc;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
    
    // These echip variables work to (temporarily) pass the 'bootprom diagnostics' test:
    u8 m_echip_response_mode = 0;
    u8 m_echip_data = 0;
    u8 m_echip_read_index = 0;
    u8 m_echip_ram_test_byte = 0;
	
	u16 emaxii_lsi2_test_1_lo = 0;
	u16 emaxii_lsi2_test_1_hi = 0;
	u16 emaxii_lsi2_test_2_lo = 0;
	u16 emaxii_lsi2_test_2_hi = 0;
	int m_lsi1_test_count = 0;
	
	int m_midi_rxd = 0;
	int m_rs232_rxd = 0;
	bool m_midi_off = false;

    u8 m_irq_latch = 0;
    bool m_irq_state = false;
	
	u8 m_scn_shift = 0;
	u8 m_scn_bits = 0;
	bool m_scn_clk = false;
	
	u8 scanner_dta = 0x00;
    
    // Software Timer / PAL Latch tracking
    bool m_sync_time_mode = false; // true = SYNC (disabled), false = TIME (enabled)
};

INPUT_CHANGED_MEMBER(emax_state::scn_button)
{
	LOGEMAX(LOG_SCN, "SCN BUTTON: old=%d new=%d\n", oldval, newval);
    if (newval)
    {
		// does not work. need to figure out how cpu reads from scanner
		
		constexpr u8 MASTER_ROW = 1;
		constexpr u8 MASTER_COL = 1;
		constexpr u8 MASTER_SCANCODE = (MASTER_ROW << 4) | MASTER_COL;
		
        scanner_dta = 0x80 | MASTER_SCANCODE;
        irq_w<SCNINT>(0); // "I have data to send back to maincpu"
    }
}

static char const* led_state(u8 data, unsigned bit)
{
    return BIT(data, bit) ? "OFF" : "ON";
}

void emax_state::emaxi_ic21_latch_led_a_w(u8 data)
{
    LOGEMAX(LOG_LATCH, "[emax] emaxi_ic21_latch_led_a_w (%s): 0x%02X (DynAllo=%s Trans=%s PreDef=%s AnaProc=%s Mast=%s Samp=%s DigProc=%s PreMan=%s)\n",
        machine().describe_context().c_str(), data,
        led_state(data, (unsigned)emaxi_ic21_latch_led_a::DYNALLO_LED), led_state(data, (unsigned)emaxi_ic21_latch_led_a::TRANSPOSE_LED), 
        led_state(data, (unsigned)emaxi_ic21_latch_led_a::PREDEF_LED), led_state(data, (unsigned)emaxi_ic21_latch_led_a::ANAPROC_LED),
        led_state(data, (unsigned)emaxi_ic21_latch_led_a::MASTER_LED), led_state(data, (unsigned)emaxi_ic21_latch_led_a::SAMPLE_LED), 
        led_state(data, (unsigned)emaxi_ic21_latch_led_a::DIGPROC_LED), led_state(data, (unsigned)emaxi_ic21_latch_led_a::PREMAN_LED));
}

void emax_state::emaxi_ic22_latch_led_b_w(u8 data)
{
	const bool new_scn_dta = BIT(data, 0);
	const bool new_scn_clk = BIT(data, 1);

	LOGEMAX(LOG_SCN,
    "SCN RAW: data=%02X DTA=%d CLK=%d oldCLK=%d\n",
    data,
    BIT(data, 0),
    BIT(data, 1),
    m_scn_clk);

	// Detect rising edge of SCNCLK
	if (!m_scn_clk && new_scn_clk)
	{
		const u8 bit = new_scn_dta ? 1 : 0;

		m_scn_shift = (m_scn_shift << 1) | bit;
		m_scn_bits++;

		LOGEMAX(LOG_SCN,
			"SCN SERIAL: PC=%06X bit=%d bits=%d shift=%02X\n",
			m_maincpu->pc(),
			bit,
			m_scn_bits,
			m_scn_shift
		);

		if (m_scn_bits == 8)
		{
			LOGEMAX(LOG_SCN,
				"SCN SERIAL BYTE: %02X\n",
				m_scn_shift
			);

			m_scn_shift = 0;
			m_scn_bits = 0;
		}
	}

	m_scn_clk = new_scn_clk;
	
    LOGEMAX(LOG_LATCH, "[emax] emaxi_ic22_latch_led_b_w (%s): 0x%02X (ScnDta=%s ScnClk=%s SyncTime=%s Unlabeled=%s Side=%s Mtr=%s Seq=%s Enter=%s)\n",
        machine().describe_context().c_str(), data,
        BIT(data, (unsigned)emaxi_ic22_latch_led_b::SCNDTA) ? "ON" : "OFF", BIT(data, (unsigned)emaxi_ic22_latch_led_b::SCNCLK) ? "ON" : "OFF", 
        BIT(data, (unsigned)emaxi_ic22_latch_led_b::SYNC_TIME) ? "SYNC" : "TIME", led_state(data, (unsigned)emaxi_ic22_latch_led_b::Unlabeled),
        led_state(data, (unsigned)emaxi_ic22_latch_led_b::SIDE), led_state(data, (unsigned)emaxi_ic22_latch_led_b::MTR), 
        led_state(data, (unsigned)emaxi_ic22_latch_led_b::SEQ_LED), led_state(data, (unsigned)emaxi_ic22_latch_led_b::ENTER_LED));
    
    // Bit 2 controls +SYNC/-TIME.D (0 = TIME mode enabled, 1 = SYNC mode enabled)
    m_sync_time_mode = BIT(data, (unsigned)emaxi_ic22_latch_led_b::SYNC_TIME);
        
	m_eeprom->clk_write(!BIT(data, (unsigned)emaxi_ic22_latch_led_b::SIDE));
		
    m_fdd->ss_w(!BIT(data, (unsigned)emaxi_ic22_latch_led_b::SIDE));
    m_fdd->mon_w(BIT(data, (unsigned)emaxi_ic22_latch_led_b::MTR));
}

void emax_state::emaxi_misc_latch_w(u8 data)
{
	bool fdienb = BIT(data, 7);
	m_midi_off = !BIT(data, 6);
		
	update_acia_rxd();
		
	//LOGEMAX(LOG_LATCH, "cswewe: writing %02x with MIDIOF=%d \n", data, m_midi_off);
		
	m_eeprom->cs_write(fdienb);
	m_eeprom->di_write(m_midi_off);
}

void emax_state::emaxii_ic34_latch_led_a_w(u8 data)
{
    LOGEMAX(LOG_LATCH, "emaxii_ic34_latch_led_a_w (%s): 0x%02X (DrvSelect=%s Trans=%s PreDef=%s DynProc=%s Mast=%s Samp=%s DigProc=%s PreMan=%s)\n",
        machine().describe_context().c_str(), data,
        led_state(data, (unsigned)emaxii_ic34_latch_led_a::DRVSELECT_LED), led_state(data, (unsigned)emaxii_ic34_latch_led_a::TRANSPOSE_LED), 
        led_state(data, (unsigned)emaxii_ic34_latch_led_a::PREDEF_LED), led_state(data, (unsigned)emaxii_ic34_latch_led_a::DYNPROC_LED),
        led_state(data, (unsigned)emaxii_ic34_latch_led_a::MASTER_LED), led_state(data, (unsigned)emaxii_ic34_latch_led_a::SAMPLE_LED), 
        led_state(data, (unsigned)emaxii_ic34_latch_led_a::DIGPROC_LED), led_state(data, (unsigned)emaxii_ic34_latch_led_a::PREMAN_LED));
}

void emax_state::emaxii_ic35_latch_led_b_w(u16 data)
{
    LOGEMAX(LOG_LATCH, "emaxii_ic35_latch_led_b_w (%s): 0x%02X (ScnDta=%s ScnClk=%s SyncTime=%s Unlabeled=%s Side=%s Mtr=%s Seq=%s Enter=%s)\n",
        machine().describe_context().c_str(), data,
        BIT(data, (unsigned)emaxii_ic35_latch_led_b::SCNDTA) ? "ON" : "OFF", BIT(data, (unsigned)emaxii_ic35_latch_led_b::SCNCLK) ? "ON" : "OFF", 
        BIT(data, (unsigned)emaxii_ic35_latch_led_b::SYNC_TIME) ? "SYNC" : "TIME", led_state(data, (unsigned)emaxii_ic35_latch_led_b::Unlabeled),
        led_state(data, (unsigned)emaxii_ic35_latch_led_b::SIDE), led_state(data, (unsigned)emaxii_ic35_latch_led_b::MTR), 
        led_state(data, (unsigned)emaxii_ic35_latch_led_b::SEQ_LED), led_state(data, (unsigned)emaxii_ic35_latch_led_b::ENTER_LED));
    
    // Bit 2 controls +SYNC/-TIME.D (0 = TIME mode enabled, 1 = SYNC mode enabled)
    m_sync_time_mode = BIT(data, (unsigned)emaxii_ic35_latch_led_b::SYNC_TIME);
        
	m_eeprom->clk_write(!BIT(data, (unsigned)emaxii_ic35_latch_led_b::SIDE));
		
    m_fdd->ss_w(!BIT(data, (unsigned)emaxii_ic35_latch_led_b::SIDE));
    m_fdd->mon_w(BIT(data, (unsigned)emaxii_ic35_latch_led_b::MTR));
}

void emax_state::emaxii_misc_latch_w(u8 data)
{
	bool eece = BIT(data, 7);
	m_midi_off = BIT(data, 6);
		
	//LOGEMAX(LOG_LATCH, "%s: cswewe: %02x eece: %d midiof: %d \n", machine().describe_context().c_str(), data, eece, m_midi_off);
		
	m_eeprom->cs_write(eece);
	m_eeprom->di_write(m_midi_off);
}

void emax_state::machine_start()
{
    m_irq_latch = 0x43;
    m_irq_state = false;
    
    /*
    m_maincpu->space(AS_PROGRAM).install_write_tap(
        0x008000, 0x017fff, "ram_debug_tap",
        [this](offs_t offset, u8 &data, u8 mem_mask) {    
            LOGEMAX(LOG_WARN, "[emax] Write to RAM 0x%08X! Value: 0x%02X (PC: 0x%08X)\n", 
                            offset, data, m_maincpu->pc());
        });
		*/
        
		/*
        m_maincpu->space(AS_PROGRAM).install_read_tap(
        0x008000, 0x017fff, "ram_debug_tap",
        [this](offs_t offset, u8 &data, u8 mem_mask) {    
            LOGEMAX(LOG_WARN, "[emax] Reading from RAM 0x%08X! Value: 0x%02X (PC: 0x%08X)\n", 
                            offset, data, m_maincpu->pc());
        });
        */
}

void emax_state::machine_reset()
{
    m_fdc->set_floppy(m_fdd);
    m_fdc->dden_w(0);
}

HD44780_PIXEL_UPDATE(emax_state::pixel_update)
{
    if (x < 5 && y < 8 && line < 2 && pos < 16)
        bitmap.pix(line * 8 + y, pos * 6 + x) = state;
}

u8 emax_state::hdc_r(offs_t offset)
{
    u8 res = m_hdc->read(offset >> 1);
    LOGEMAX(LOG_HARD_DRIVE, "[emax] hdc_r: offset=%x reg=%x result=%02x pc=%s\n", offset, offset >> 1, res, machine().describe_context().c_str());
    return res;
}

void emax_state::hdc_w(offs_t offset, u8 data)
{
    LOGEMAX(LOG_HARD_DRIVE, "[emax] hdc_w: offset=%x reg=%x result=%02x pc=%s\n", offset, offset >> 1, data, machine().describe_context().c_str());
    m_hdc->write(offset >> 1, data);
}

u8 emax_state::timer_r(offs_t offset)
{
    u8 result = m_ctc->read(offset >> 1);
    LOGEMAX(LOG_TIMER, "[emax] timer_r: offset=%x reg=%x result=%02x pc=%s\n", offset, offset >> 1, result, machine().describe_context().c_str());
    return result;
}

void emax_state::timer_w(offs_t offset, u8 data)
{
    LOGEMAX(LOG_TIMER, "[emax] timer_w: offset=%x reg=%x data=%02x pc=%s\n", offset, offset >> 1, data, machine().describe_context().c_str());
    m_ctc->write(offset >> 1, data);
}

void emax_state::mux_w(u8 data)
{
    LOGEMAX(LOG_DACMUX, "[emax] mux_w: data=%02x pc=%s\n", data, machine().describe_context().c_str());
}

void emax_state::dac_w(u8 data)
{
    LOGEMAX(LOG_DACMUX, "[emax] dac_w: data=%02x pc=%s\n", data, machine().describe_context().c_str());
}

void emax_state::echip_cmd_w(u8 data)
{
    LOGEMAX(LOG_ECHIP, "[emax] (%s): E-CHIP CMD <- %02X\n", machine().describe_context().c_str(), data);

    switch (data)
    {
    case 0xc5:
        m_echip_response_mode = 1;
        m_echip_read_index = 0;
        break;

    case 0x7c:
        m_echip_ram_test_byte = m_echip_data;
        LOGEMAX(LOG_ECHIP, "[emax] (%s): E-CHIP RAM TEST WRITE <- %02X\n", machine().describe_context().c_str(), m_echip_ram_test_byte);
        break;

    case 0xfc:
        m_echip_response_mode = 2;
        m_echip_read_index = 0;
        break;

    default:
        break;
    }
}

void emax_state::echip_data_w(offs_t offset, u8 data)
{
    LOGEMAX(LOG_ECHIP, "[emax] (%s): E-CHIP DATA[%d] <- %02X\n", machine().describe_context().c_str(), offset, data);

    if (offset == 0)
        m_echip_data = data;
}

u8 emax_state::echip_data_r()
{
    u8 result = 0xff;

    switch (m_echip_response_mode)
    {
    case 1:
        switch (m_echip_read_index++)
        {
        case 0: result = 0x97; break;
        case 1: result = 0x45; break;
        case 2: result = 0x00; break;
        }
        break;

    case 2:
        result = m_echip_ram_test_byte;
        break;
    }

    LOGEMAX(LOG_ECHIP, "[emax] (%s): E-CHIP READ -> %02X\n", machine().describe_context().c_str(), result);
    return result;
}

void emax_state::emax_periphs(address_map &map)
{
    map(0x2c0000, 0x2c0000).select(6).rw(FUNC(emax_state::timer_r), FUNC(emax_state::timer_w));
    
    map(0x8E4000, 0x8E4000).w(FUNC(emax_state::emaxi_ic21_latch_led_a_w));
    map(0x8E4002, 0x8E4002).w(FUNC(emax_state::emaxi_ic22_latch_led_b_w));
	map(0xAA4000, 0xAA4000).w(FUNC(emax_state::emaxi_misc_latch_w));

	map(0x822000, 0x822000).w(m_fdc, FUNC(wd1772_device::cmd_w));
    map(0x822400, 0x822400).r(m_fdc, FUNC(wd1772_device::status_r));
    map(0x822800, 0x822800).w(m_fdc, FUNC(wd1772_device::track_w));
    map(0x822c00, 0x822c00).r(m_fdc, FUNC(wd1772_device::track_r));
    map(0x823000, 0x823000).w(m_fdc, FUNC(wd1772_device::sector_w));
    map(0x823400, 0x823400).r(m_fdc, FUNC(wd1772_device::sector_r));
    map(0x823800, 0x823800).w(m_fdc, FUNC(wd1772_device::data_w));
    map(0x823c00, 0x823c00).r(m_fdc, FUNC(wd1772_device::data_r));

    map(0x824004, 0x824004).w(FUNC(emax_state::mux_w));
    map(0x824006, 0x824006).w(FUNC(emax_state::dac_w));
	
    map(0x890000, 0x890000).w(m_lcdc, FUNC(hd44780_device::control_w));
    map(0x890002, 0x890002).r(m_lcdc, FUNC(hd44780_device::control_r));
	map(0x890004, 0x890004).lw8([this](u8 data) { LOGEMAX(LOG_LCD, "lcd update: '%c' \n", (char)data); 
		m_lcdc->data_w(data); }, "data_w");
    //map(0x890004, 0x890004).w(m_lcdc, FUNC(hd44780_device::data_w));
    map(0x890006, 0x890006).r(m_lcdc, FUNC(hd44780_device::data_r));

    // fffe00 = interrupt vector
    map(0xfffe00, 0xfffe00).lr8([this]() {
		LOGEMAX(LOG_IRQ, "read irq: %02x \n", m_irq_latch);
		return m_irq_latch;
        }, "irq_latch_r");
		
	map(0xfffe01, 0xfffe03).nopr(); // temporarily mute logging, unsure why this is occurring
    
	map(0x8E6000, 0x8E6000).lw8([this](u8 data) {
		LOGEMAX(LOG_IRQ, "stint off \n"); 
		irq_w<STINT>(0); }, "stint_w");
    map(0x8F0000, 0x8F0000).lw8([this](u8 data) {
		LOGEMAX(LOG_IRQ, "tgint off \n"); 
		irq_w<TGINT>(0); }, "tgint_w");
	
	map(0xAA6000, 0xAA6000).lr8([this]() {		
		LOGEMAX(LOG_SCN, "%s: csrscn: tmp=%02x SERIAL shift=%02x bits=%d\n", machine().describe_context().c_str(),
			scanner_dta, m_scn_shift, m_scn_bits);	
		return 0x00;
		}, "csrscn");
		
	// Temporary mappings to pass the bootprom diagnostics:
    map(0xaa2000, 0xaa2000).w(FUNC(emax_state::echip_cmd_w));
    map(0xaa2800, 0xaa2801).w(FUNC(emax_state::echip_data_w));
    map(0xaa2c00, 0xaa2c00).r(FUNC(emax_state::echip_data_r));

    //map(0xaa2000, 0xaa2000).select(0x800).w(FUNC(emax_state::echip_w));
    //map(0xaa2400, 0xaa2400).select(0x800).r(FUNC(emax_state::echip_r));
}

void emax_state::emax_map(address_map &map)
{
    map(0x000000, 0x000fff).rom().region("bootprom", 0);
    map(0x008000, 0x017fff).ram(); // DRAM, 64kb
	
    map(0x818028, 0x818028).w(m_acia, FUNC(acia6850_device::control_w));
    map(0x81802a, 0x81802a).r(m_acia, FUNC(acia6850_device::status_r));
    map(0x81802c, 0x81802c).w(m_acia, FUNC(acia6850_device::data_w));
    map(0x81802e, 0x81802e).r(m_acia, FUNC(acia6850_device::data_r));
	
    emax_periphs(map);
}

void emax_state::emaxp_map(address_map &map)
{
    map(0x000000, 0x001fff).rom().region("bootprom", 0);
    map(0x008000, 0x017fff).ram(); // DRAM, 64kb

    map(0x0f8000, 0x0f8000).select(0xe).rw(FUNC(emax_state::hdc_r), FUNC(emax_state::hdc_w));
	
    map(0x818028, 0x818028).w(m_acia, FUNC(acia6850_device::control_w));
    map(0x81802a, 0x81802a).r(m_acia, FUNC(acia6850_device::status_r));
    map(0x81802c, 0x81802c).w(m_acia, FUNC(acia6850_device::data_w));
    map(0x81802e, 0x81802e).r(m_acia, FUNC(acia6850_device::data_r));
	
    emax_periphs(map);
}

void emax_state::emax2_map(address_map &map)
{
    map(0x000000, 0x003fff).rom().region("bootprom", 0);
    
    map(0x008000, 0x087FFF).ram(); // DRAM?
		
	// fffe00 = interrupt vector
    map(0xfffe00, 0xfffe00).lr8([this]() {
        LOGEMAX(LOG_IRQ, "[emax] %s: interrupt vector port read = 0x%02X\n", machine().describe_context().c_str(), m_irq_latch);
        return m_irq_latch; 
        }, "irq_latch_r");
		
    map(0x7F8000, 0x7F8001).lw16([this](u16 data) { LOGEMAX(LOG_IRQ, "tgint/stint off \n"); irq_w<STINT>(0); irq_w<TGINT>(0); }, "rstint_w");
    
    map(0xCF0000, 0xCF0000).w(FUNC(emax_state::emaxii_ic34_latch_led_a_w));
    map(0xDF0000, 0xDF0003).w(FUNC(emax_state::emaxii_ic35_latch_led_b_w));
	map(0xEF0000, 0xEF0000).w(FUNC(emax_state::emaxii_misc_latch_w));
    
    map(0x0a8018, 0x0a8018).w(m_acia2, FUNC(acia6850_device::control_w));
    map(0x0a801a, 0x0a801a).r(m_acia2, FUNC(acia6850_device::status_r));
    map(0x0a801c, 0x0a801c).w(m_acia2, FUNC(acia6850_device::data_w));
    map(0x0a801e, 0x0a801e).r(m_acia2, FUNC(acia6850_device::data_r));
	
    map(0x0a8028, 0x0a8028).w(m_acia, FUNC(acia6850_device::control_w));
    map(0x0a802a, 0x0a802a).r(m_acia, FUNC(acia6850_device::status_r));
    map(0x0a802c, 0x0a802c).w(m_acia, FUNC(acia6850_device::data_w));
    map(0x0a802e, 0x0a802e).r(m_acia, FUNC(acia6850_device::data_r));
	
    map(0x0b0000, 0x0b0000).w(m_lcdc, FUNC(hd44780_device::control_w));
    map(0x0b0002, 0x0b0002).r(m_lcdc, FUNC(hd44780_device::control_r));
	map(0x0b0004, 0x0b0004).lw8([this](u8 data) { LOGEMAX(LOG_LCD, "lcd update: '%c' \n", (char)data); m_lcdc->data_w(data); }, "data_w");
    //map(0x0b0004, 0x0b0004).w(m_lcdc, FUNC(hd44780_device::data_w));
    map(0x0b0006, 0x0b0006).r(m_lcdc, FUNC(hd44780_device::data_r));
	
    map(0x1f8000, 0x1f800f).rw(m_hdc, FUNC(ncr5380_device::read), FUNC(ncr5380_device::write)).umask16(0x00ff);
    map(0x3f8000, 0x3f8007).rw(m_ctc, FUNC(pit8254_device::read), FUNC(pit8254_device::write)).umask16(0x00ff);
	
	map(0x698000, 0x698000).lr8([this]() {
		LOGEMAX(LOG_SCN, "%s: ccscan_r: tmp %02x \n", machine().describe_context().c_str(), scanner_dta); 	
		return scanner_dta;
		}, "ccscan_r");
		
	map(0x9E8800, 0x9E8803).lw16([this](offs_t offset, u16 data) {
		if (offset == 0) emaxii_lsi2_test_1_lo = data;
		else emaxii_lsi2_test_1_hi = data;
	}, "emax_ii_lsi2_test_1_w");
		
	map(0x9E8880, 0x9E8883).lw16([this](offs_t offset, u16 data) {
		if (offset == 0) emaxii_lsi2_test_2_lo = data;
		else emaxii_lsi2_test_2_hi = data;
	}, "emax_ii_lsi2_test_2_w");
		
	map(0x9EA800, 0x9EA803).lr16([this](offs_t offset) {
		return offset == 0 ? emaxii_lsi2_test_1_lo : emaxii_lsi2_test_1_hi;
		}, "emax_ii_lsi2_test_1_r");
		
	map(0x9EA880, 0x9EA883).lr16([this](offs_t offset) {
		return offset == 0 ? emaxii_lsi2_test_2_lo : emaxii_lsi2_test_2_hi;
	}, "emax_ii_lsi2_test_2_r");
	
	// LSI #1 passing 'hack'
	map(0x8e8000, 0x8e83ff).ram().mirror(0x0400).share("emax_ii_lsi1_ram");
	map(0x8e8700, 0x8e8703).r(FUNC(emax_state::emax_ii_lsi1_test_r));
    
    map(0xAE8000, 0xAE8000).w(m_fdc, FUNC(wd1772_device::cmd_w));
    map(0xAE8400, 0xAE8400).r(m_fdc, FUNC(wd1772_device::status_r));
    map(0xAE8800, 0xAE8800).w(m_fdc, FUNC(wd1772_device::track_w));
    map(0xAE8C00, 0xAE8C00).r(m_fdc, FUNC(wd1772_device::track_r));
    map(0xAE9000, 0xAE9000).w(m_fdc, FUNC(wd1772_device::sector_w));
    map(0xAE9400, 0xAE9400).r(m_fdc, FUNC(wd1772_device::sector_r));
    map(0xAE9800, 0xAE9800).w(m_fdc, FUNC(wd1772_device::data_w));
    map(0xAE9C00, 0xAE9C00).r(m_fdc, FUNC(wd1772_device::data_r));
}

static INPUT_PORTS_START(emax)
	PORT_START("ddt")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D) PORT_CHAR('d') PORT_CHAR('D') PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(emax_state::scn_button), 0)
INPUT_PORTS_END

static INPUT_PORTS_START(emax2)
INPUT_PORTS_END

void emax_state::palette_init(palette_device &palette)
{
    palette.set_pen_color(0, rgb_t(131, 136, 139));
    palette.set_pen_color(1, rgb_t( 92,  83,  88));
}

static void emu_scsi_devices(device_slot_interface &device)
{
	device.option_add("harddisk", NSCSI_HARDDISK);
}

void emax_state::scsihd(machine_config &config)
{
	auto &scsi(NSCSI_BUS(config, "scsi"));
    NSCSI_CONNECTOR(config, "scsi:1", emu_scsi_devices, "harddisk", false);
    NSCSI_CONNECTOR(config, "scsi:2", emu_scsi_devices, nullptr, false);
    NSCSI_CONNECTOR(config, "scsi:3", emu_scsi_devices, nullptr, false);
    NSCSI_CONNECTOR(config, "scsi:4", emu_scsi_devices, nullptr, false);
    NSCSI_CONNECTOR(config, "scsi:5", emu_scsi_devices, nullptr, false);
    NSCSI_CONNECTOR(config, "scsi:6", emu_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:7", emu_scsi_devices, nullptr, false);
	
	NCR5380(config, m_hdc);
	m_hdc->irq_handler().set(DEVICE_SELF, FUNC(emax_state::irq_w<HDINT>));
	
    scsi.set_external_device(0, m_hdc);
}

static void emax_floppies(device_slot_interface &device)
{
    device.option_add("35dd", FLOPPY_35_DD);
}

static void emax_rs232(device_slot_interface &device)
{
    device.option_add("terminal", SERIAL_TERMINAL);
}

static void add_formats(format_registration &fr)
{
    // TODO: other formats
    fr.add(FLOPPY_HFE_FORMAT);
}

void emax_state::emax(machine_config &config)
{
    NS32008(config, m_maincpu, 16_MHz_XTAL / 2); // NS32008D-8 + NS32C201D-10
    m_maincpu->set_addrmap(AS_PROGRAM, &emax_state::emax_map);

	EEPROM_93C06_16BIT(config, m_eeprom); // NMC93C06N
	m_eeprom->do_callback().set(*this, FUNC(emax_state::irq_w<MIDINT>));

    //R6500_11(config, "scannercpu", 16_MHz_XTAL / 4);

    PIT8254(config, m_ctc);
    m_ctc->set_clk<0>(16_MHz_XTAL / 2); // 8mhz line
    m_ctc->set_clk<1>(16_MHz_XTAL / 2); // 8mhz line
    m_ctc->set_clk<2>(16_MHz_XTAL / 32); // 500khz from IC4
    
    //m_ctc->out_handler<0>(); // +CSCF.D
    m_ctc->out_handler<1>().set(*this, FUNC(emax_state::irq_w<TGINT>)); // -TGTIME.D and +ADCCK.D
    m_ctc->out_handler<2>().set(*this, FUNC(emax_state::swtime_w)); // -SWTIME.D and -SMPL.D 

    WD1772(config, m_fdc, 16_MHz_XTAL / 2); // WD1772-PA
    m_fdc->intrq_wr_callback().set(*this, FUNC(emax_state::irq_w<FDCINT>));
    m_fdc->set_disable_motor_control(true);

    FLOPPY_CONNECTOR(config, "fdc:0", emax_floppies, "35dd", add_formats).enable_sound(false);

    ACIA6850(config, m_acia); // MC68A50P
	m_acia->irq_handler().set(*this, FUNC(emax_state::irq_w<MIDINT>)).invert();
	
    clock_device &acia_clock(CLOCK(config, "acia_clock", 8_MHz_XTAL / 16));
    acia_clock.signal_handler().set(m_acia, FUNC(acia6850_device::write_txc));
    acia_clock.signal_handler().append(m_acia, FUNC(acia6850_device::write_rxc));
    
	MIDI_PORT(config, m_midi_out_port, midiout_slot, "midiout");
	m_midi_out_port->rxd_handler().set(*this, FUNC(emax_state::midi_rxd_w));
	
    RS232_PORT(config, m_rs422_port, emax_rs232, nullptr);
    m_rs422_port->rxd_handler().set(*this, FUNC(emax_state::rs232_rxd_w));
    m_acia->rts_handler().set(m_rs422_port, FUNC(rs232_port_device::write_rts));
    m_acia->txd_handler().set(m_rs422_port, FUNC(rs232_port_device::write_txd));
	m_acia->txd_handler().append(m_midi_out_port, FUNC(midi_port_device::write_txd));

    screen_device &screen(SCREEN(config, "screen").set_lcd());
    screen.set_refresh_hz(50);
    screen.set_screen_update(m_lcdc, FUNC(hd44780_device::screen_update));
    screen.set_size(16*6, 16);
    screen.set_visarea(0, 16*6-1, 0, 16-1);
    screen.set_palette("palette");

    HD44780(config, m_lcdc, 270'000); // TODO: clock not measured, datasheet typical clock used
    m_lcdc->set_lcd_size(2, 16);
    m_lcdc->set_pixel_update_cb(FUNC(emax_state::pixel_update));

    PALETTE(config, "palette", FUNC(emax_state::palette_init), 2);

    //EMU_IM374(config, "echip", 16_MHz_XTAL / 2);
}

void emax_state::emaxp(machine_config &config)
{
    emax(config);
    m_maincpu->set_addrmap(AS_PROGRAM, &emax_state::emaxp_map);

    scsihd(config);
}

void emax_state::emax2(machine_config &config)
{
    NS32CG16(config, m_maincpu, 20_MHz_XTAL / 2); // NS32CG16V-10
    m_maincpu->set_addrmap(AS_PROGRAM, &emax_state::emax2_map);

    EEPROM_93C06_16BIT(config, m_eeprom); // NMC93C06N
	m_eeprom->do_callback().set(*this, FUNC(emax_state::irq_w<MIDINT>));

    PIT8254(config, m_ctc);
    m_ctc->set_clk<0>(16_MHz_XTAL / 2); // 8mhz line
    m_ctc->set_clk<1>(16_MHz_XTAL / 2); // 8mhz line
    m_ctc->set_clk<2>(16_MHz_XTAL / 32); // 500khz line
    
    //m_ctc->out_handler<0>(); // Unused
    m_ctc->out_handler<1>().set(*this, FUNC(emax_state::irq_w<TGINT>)); // -TGTIME.D
    m_ctc->out_handler<2>().set(*this, FUNC(emax_state::swtime_w)); // -SWTIME.D

    WD1772(config, m_fdc, 16_MHz_XTAL / 2);
    m_fdc->intrq_wr_callback().set(*this, FUNC(emax_state::irq_w<FDCINT>));
    m_fdc->set_disable_motor_control(true);

    FLOPPY_CONNECTOR(config, "fdc:0", emax_floppies, "35dd", add_formats).enable_sound(false);

    ACIA6850(config, m_acia); // IC3?
    m_acia->irq_handler().set(*this, FUNC(emax_state::irq_w<MIDINT>)).invert();
    
	clock_device &acia_clock(CLOCK(config, "acia_clock", 8_MHz_XTAL / 16));
    acia_clock.signal_handler().set(m_acia, FUNC(acia6850_device::write_txc));
    acia_clock.signal_handler().append(m_acia, FUNC(acia6850_device::write_rxc));
    
	MIDI_PORT(config, m_midi_out_port, midiout_slot, "midiout");
	m_midi_out_port->rxd_handler().set(*this, FUNC(emax_state::midi_rxd_w));
	
    RS232_PORT(config, m_rs422_port, emax_rs232, nullptr);
    m_rs422_port->rxd_handler().set(*this, FUNC(emax_state::rs232_rxd_w));
    m_acia->rts_handler().set(m_rs422_port, FUNC(rs232_port_device::write_rts));
    m_acia->txd_handler().set(m_rs422_port, FUNC(rs232_port_device::write_txd));
	m_acia->txd_handler().append(m_midi_out_port, FUNC(midi_port_device::write_txd));
	
    ACIA6850(config, m_acia2); // IC4? --- note from manual: "1C4 IS UNSTUFFED AND IS USED FOR DEBUGGING ONLY"
    m_acia2->irq_handler().set(*this, FUNC(emax_state::irq_w<MIDINT>)).invert();

    scsihd(config);

    screen_device &screen(SCREEN(config, "screen").set_lcd());
    screen.set_refresh_hz(50);
    screen.set_screen_update(m_lcdc, FUNC(hd44780_device::screen_update));
    screen.set_size(16*6, 16);
    screen.set_visarea(0, 16*6-1, 0, 16-1);
    screen.set_palette("palette");

    HD44780(config, m_lcdc, 270'000); // TODO: clock not measured, datasheet typical clock used
    m_lcdc->set_lcd_size(2, 16);
    m_lcdc->set_pixel_update_cb(FUNC(emax_state::pixel_update));

    PALETTE(config, "palette", FUNC(emax_state::palette_init), 2);

    // TODO: add other unknown peripherals
}

ROM_START(emax)
    ROM_REGION(0x1000, "bootprom", 0) // v2, Rev C mainboard, non-SE/HD version
    ROM_LOAD("emax.bin", 0x0000, 0x1000, CRC(b55210aa) SHA1(9b02dfc28700e07be5e044d53035041a54732927))

    ROM_REGION(0xc00, "scannercpu", 0)
    ROM_LOAD("im368-1_ba__r1129-11.ic7", 0x000, 0xc00, NO_DUMP)

    ROM_REGION(0x104, "cspal", 0)
    ROM_LOAD("ip345c.bin", 0x000, 0x104, CRC(7bae1347) SHA1(a49ab0bae41132e60c113d2117c5a042c2a1e44d)) // PAL16R4
ROM_END

ROM_START(emaxp)
    ROM_REGION(0x2000, "bootprom", 0) // SCSI upgrade
    ROM_LOAD("ip424a3089.bin", 0x0000, 0x2000, CRC(3abd3a16) SHA1(8d7ac39c8147bdc2ead9fedee463d1bbe94332c5))

    ROM_REGION(0xc00, "scannercpu", 0)
    ROM_LOAD("im368-1_ba__r1129-11.ic7", 0x000, 0xc00, NO_DUMP)

    ROM_REGION(0x104, "cspal", 0)
    ROM_LOAD("ip345c.bin", 0x000, 0x104, CRC(7bae1347) SHA1(a49ab0bae41132e60c113d2117c5a042c2a1e44d)) // PAL16R4

    ROM_REGION(0x104, "timpal", 0)
    ROM_LOAD("ip379a.bin", 0x000, 0x104, CRC(fb50f8bd) SHA1(5b8b7904736188c4cf8b36a4bf5ad685422ec760)) // PAL16R4
	
	ROM_REGION16_LE(0x20, "eeprom", 0)
    ROM_LOAD("93c06n.ic24", 0x00, 0x20, CRC(403ef05b) SHA1(893ef614127ac1898d8ac529521f87ff62207138))
ROM_END

ROM_START(emax2)
    ROM_REGION16_LE(0x4000, "bootprom", 0)
    ROM_LOAD16_BYTE("ip43aemu_3891.ic20", 0x0000, 0x2000, CRC(51fdccb8) SHA1(0cab6540ed5d03ba202569b8730e0ec6dce1a477)) // Am27C64-250DC
    ROM_LOAD16_BYTE("ip43bemu_4291.ic19", 0x0001, 0x2000, CRC(810160b3) SHA1(6f490f9014bc221e047ccd77428b002d0a3c3168)) // Am27C64-250DC

    ROM_REGION16_LE(0x20, "eeprom", 0)
    ROM_LOAD("93c06n.ic24", 0x00, 0x20, CRC(403ef05b) SHA1(893ef614127ac1898d8ac529521f87ff62207138))
ROM_END

} // anonymous namespace


SYST(1986, emax,  0,    0, emax,  emax,  emax_state, empty_init, "E-mu Systems", "Emax Digital Sampling Keyboard", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
SYST(198?, emaxp, emax, 0, emaxp, emax,  emax_state, empty_init, "E-mu Systems", "Emax Plus Digital Sampling Keyboard", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
SYST(1989, emax2, 0,    0, emax2, emax2, emax_state, empty_init, "E-mu Systems", "Emax II 16-Bit Digital Sound System", MACHINE_NO_SOUND | MACHINE_NOT_WORKING)