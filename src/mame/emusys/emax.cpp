// license:BSD-3-Clause
// copyright-holders: AJR, Strikelesss
/***********************************************************************************************************************************

    Skeleton driver for E-mu Emax & Emax II samplers.
	
	All Emax's boot past 'loading software' to a blank screen (manual stating it is scanner related?)
	It currently loops at a TBITB, and loads only the OS-provided 'Untitled' bank if the TBITB is satisfied a few times, then reboots.

***********************************************************************************************************************************/

#include "emu.h"
#include "bus/nscsi/devices.h"
#include "cpu/ns32000/ns32000.h"
#include "machine/6850acia.h"
#include "machine/eepromser.h"
#include "machine/ncr5380.h"
#include "machine/pit8253.h"
#include "machine/wd_fdc.h"
#include "video/hd44780.h"
#include "emupal.h"
#include "screen.h"
#include "imagedev/floppy.h"
#include "formats/hxchfe_dsk.h"


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
		, m_lcdc(*this, "lcdc")
	{
	}

	void emax(machine_config &config);
	void emaxp(machine_config &config);
	void emax2(machine_config &config);
	
	DECLARE_INPUT_CHANGED_MEMBER(test_scint);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// All active low
	enum class emaxi_ic21_latch_led_a : u8
	{
		DYNALLO_LED		= 0, // D0 (Bit 0)
		TRANSPOSE_LED	= 1, // D1 (Bit 1)
		PREDEF_LED		= 2, // D2 (Bit 2)
		ANAPROC_LED		= 3, // D3 (Bit 3)
		MASTER_LED		= 4, // D4 (Bit 4)
		SAMPLE_LED		= 5, // D5 (Bit 5)
		DIGPROC_LED		= 6, // D6 (Bit 6)
		PREMAN_LED		= 7, // D7 (Bit 7)
	};

	enum class emaxi_ic22_latch_led_b : u8
	{
		SCNDTA		= 0, // D0 (Bit 0) (active high)
		SCNCLK		= 1, // D1 (Bit 1) (active high)
		SYNC_TIME	= 2, // D2 (Bit 2) // SYNC (active high) TIME (active low)
		Unlabeled	= 3, // D3 (Bit 3)
		SIDE		= 4, // D4 (Bit 4) (active low)
		MTR			= 5, // D5 (Bit 5) (active low)
		SEQ_LED		= 6, // D6 (Bit 6) (active low)
		ENTER_LED	= 7, // D7 (Bit 7) (active low)
	};
	
	// All active low
	enum class emaxii_ic34_latch_led_a : u8
	{
		DRVSELECT_LED	= 0, // D0 (Bit 0)
		TRANSPOSE_LED	= 1, // D1 (Bit 1)
		PREDEF_LED		= 2, // D2 (Bit 2)
		DYNPROC_LED		= 3, // D3 (Bit 3)
		MASTER_LED		= 4, // D4 (Bit 4)
		SAMPLE_LED		= 5, // D5 (Bit 5)
		DIGPROC_LED		= 6, // D6 (Bit 6)
		PREMAN_LED		= 7, // D7 (Bit 7)
	};

	enum class emaxii_ic35_latch_led_b : u8
	{
		SCNDTA		= 0, // D0 (Bit 0) (active high)
		SCNCLK		= 1, // D1 (Bit 1) (active high)
		SYNC_TIME 	= 2, // D2 (Bit 2) // SYNC (active high) TIME (active low)
		Unlabeled	= 3, // D3 (Bit 3)
		SIDE		= 4, // D4 (Bit 4) (active low)
		MTR			= 5, // D5 (Bit 5) (active low)
		SEQ_LED		= 6, // D6 (Bit 6) (active low)
		ENTER_LED	= 7, // D7 (Bit 7) (active low)
	};

	HD44780_PIXEL_UPDATE(pixel_update);

	void fdc_cmd_w(u8 data);
	u8 fdc_status_r();
	void fdc_track_w(u8 data);
	u8 fdc_track_r();
	void fdc_sector_w(u8 data);
	u8 fdc_sector_r();
	void fdc_data_w(u8 data);
	u8 fdc_data_r();

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
	
	//u8 echip_r(offs_t offset);
    //void echip_w(offs_t offset, u8 data);
	
	// Timer 2 output write-line callback
    void swtime_w(int state);

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
	required_device<hd44780_device> m_lcdc;
	
	enum irq_number : unsigned
	{
		SCNINT	= 0, // scanner interrupt (active high)
		MIDINT	= 1, // MIDI/serial interrupt (active low)
		FDCINT	= 2, // floppy disk controller interrupt (active high)
		STINT	= 3, // software timer interrupt (active high)
		TGINT	= 4, // timer generation interrupt (emax ii only, bit 4 unlabeled on emax i) (active high)
		HDINT	= 5, // hard disk interrupt (active high)
	};
	
	template <irq_number IRQ, bool ActiveLow = false>
    void irq_w(int state)
    {
        bool const active = ActiveLow ? !state : bool(state);

        logerror("irq_w: IRQ=%u state=%d active=%d latch_before=%02x pc=%s\n", (unsigned)IRQ, state, active, m_irq_latch, machine().describe_context().c_str());

        if (active)
            m_irq_latch |= 1U << IRQ;
        else
            m_irq_latch &= ~(1U << IRQ);

        bool const any_active = (m_irq_latch != 0);
        if (any_active != m_irq_state)
        {
            m_irq_state = any_active;
            logerror("irq_w: new state: %d (latch=%02x)\n", m_irq_state, m_irq_latch);
            m_maincpu->set_input_line(INPUT_LINE_IRQ0, m_irq_state);
        }
    } 

	u8 emaxi_vector_port_r()
	{
		//printf("%s: interrupt vector port read = %02x\n", machine().describe_context().c_str(), m_irq_latch);
		//logerror("%s: interrupt vector port read = %02x\n", machine().describe_context(), m_irq_latch);
		return m_irq_latch;
	}
	
	u16 emaxii_vector_port_r()
	{
		//printf("%s: interrupt vector port read = %02x\n", machine().describe_context().c_str(), m_irq_latch);
		//logerror("%s: interrupt vector port read = %02x\n", machine().describe_context(), m_irq_latch);
		return m_irq_latch;
	}
	
	// These echip variables work to pass the 'bootprom diagnostics' test:
	u8 m_echip_response_mode = 0;
	u8 m_echip_data = 0;
	u8 m_echip_read_index = 0;
	u8 m_echip_ram_test_byte = 0;

	u8 m_irq_latch = 0;
	bool m_irq_state = false;
	
	// Software Timer / PAL Latch tracking
    bool m_sync_time_mode = false; // true = SYNC (disabled), false = TIME (enabled)
    bool m_last_pal_clk = false; // Triggers PAL on rising edge
};

INPUT_CHANGED_MEMBER(emax_state::test_scint)
{
	// reboots if applied during blank screen
	irq_w<SCNINT>(newval);
}

void emax_state::fdc_cmd_w(u8 data)
{
	//logerror("%s: FDC cmd_w = %02x\n", machine().describe_context(), data);
	m_fdc->cmd_w(data);
}

u8 emax_state::fdc_status_r()
{
	u8 result = m_fdc->status_r();
	//logerror("%s: FDC status_r = %02x or %u \n", machine().describe_context(), result, result);
	return result;
}

void emax_state::fdc_track_w(u8 data)
{
	//logerror("%s: FDC track_w = %02x\n", machine().describe_context(), data);
	m_fdc->track_w(data);
}

u8 emax_state::fdc_track_r()
{
	u8 result = m_fdc->track_r();
	//logerror("%s: FDC track_r = %02x\n", machine().describe_context(), result);
	return result;
}

void emax_state::fdc_sector_w(u8 data)
{
	//logerror("%s: FDC sector_w = %02x\n", machine().describe_context(), data);
	m_fdc->sector_w(data);
}

u8 emax_state::fdc_sector_r()
{
	u8 result = m_fdc->sector_r();
	//logerror("%s: FDC sector_r = %02x\n", machine().describe_context(), result);
	return result;
}

void emax_state::fdc_data_w(u8 data)
{
	//logerror("%s: FDC data_w = %02x\n", machine().describe_context(), data);
	m_fdc->data_w(data);
}

u8 emax_state::fdc_data_r()
{
	u8 result = m_fdc->data_r();
	//logerror("%s: FDC data_r = %02x\n", machine().describe_context(), result);
	return result;
}

void emax_state::machine_start()
{
	m_irq_latch = 0;
	m_irq_state = false;
	
	/*
	m_maincpu->space(AS_PROGRAM).install_write_tap(
        0x008000, 0x017fff, "ram_debug_tap",
        [this](offs_t offset, u8 &data, u8 mem_mask) {	
            logerror("Write to RAM 0x%08X! Value: 0x%02X (PC: 0x%08X)\n", 
                            offset, data, m_maincpu->pc());
        });
		
		m_maincpu->space(AS_PROGRAM).install_read_tap(
        0x008000, 0x017fff, "ram_debug_tap",
        [this](offs_t offset, u8 &data, u8 mem_mask) {	
            logerror("Reading from RAM 0x%08X! Value: 0x%02X (PC: 0x%08X)\n", 
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
	//printf("hdc_r: offset=%x reg=%x result=%02x pc=%s\n", offset, offset >> 1, res, machine().describe_context().c_str());
	return res;
}

void emax_state::hdc_w(offs_t offset, u8 data)
{
	//printf("hdc_w: offset=%x reg=%x result=%02x pc=%s\n", offset, offset >> 1, data, machine().describe_context().c_str());
	m_hdc->write(offset >> 1, data);
}

u8 emax_state::timer_r(offs_t offset)
{
	u8 result = m_ctc->read(offset >> 1);
	//printf("timer_r: offset=%x reg=%x result=%02x pc=%s\n", offset, offset >> 1, result, machine().describe_context().c_str());
	return result;
}

void emax_state::timer_w(offs_t offset, u8 data)
{
	//printf("timer_w: offset=%x reg=%x data=%02x pc=%s\n", offset, offset >> 1, data, machine().describe_context().c_str());
	m_ctc->write(offset >> 1, data);
}

void emax_state::mux_w(u8 data)
{
	//logerror("muxw: %02x\n", data);
}

void emax_state::dac_w(u8 data)
{
	//logerror("dacw: %02x\n", data);
}

void emax_state::echip_cmd_w(u8 data)
{
	logerror("E-CHIP CMD <- %02X\n", data);

	switch (data)
	{
	case 0xc5:
		m_echip_response_mode = 1;
		m_echip_read_index = 0;
		break;

	case 0x7c:
		// 0x5A in AA2800 is the byte being written to E-Chip RAM.
		// Latch it now; subsequent AA2800 writes belong to the
		// following command sequence.
		m_echip_ram_test_byte = m_echip_data;
		logerror("E-CHIP RAM TEST WRITE <- %02X\n", m_echip_ram_test_byte);
		break;

	case 0xfc:
		// Read back the byte latched by the 7C operation.
		m_echip_response_mode = 2;
		m_echip_read_index = 0;
		break;

	default:
		break;
	}
}

void emax_state::echip_data_w(offs_t offset, u8 data)
{
	logerror("E-CHIP DATA[%d] <- %02X\n", offset, data);

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

	logerror("E-CHIP READ -> %02X\n", result);
	return result;
}

void emax_state::swtime_w(int state)
{
	// todo: reset (all untested, this function never hits)
	
    // If +SYNC/-TIME is HIGH (1), the 74HCT240 buffer is tri-stated/disabled,
    // blocking the timer clock from reaching IC49 (PAL16R4).
    if (m_sync_time_mode)
        return;

    // 74HCT240 inverts the active-low -SWTIME.D signal into Pin 1 (CLK) of IC49
    bool const pal_clk = !state;

    // On the rising clock edge, the PAL latches +STINT high
    if (pal_clk && !m_last_pal_clk)
    {
        irq_w<STINT>(1);
    }

    m_last_pal_clk = pal_clk;
}

static char const *led_state(u8 data, unsigned bit)
{
	return BIT(data, bit) ? "OFF" : "ON";
}

void emax_state::emax_periphs(address_map &map)
{
	map(0x2c0000, 0x2c0000).select(6).rw(FUNC(emax_state::timer_r), FUNC(emax_state::timer_w));
	
	map(0x8E4000, 0x8E4000).lw8(
    [this](u8 data)
    {
        logerror("led latch a (%s): 0x%02X (DynAllo=%s Trans=%s PreDef=%s AnaProc=%s Mast=%s Samp=%s DigProc=%s PreMan=%s)\n",
            machine().describe_context(), data,
            led_state(data, (unsigned)emaxi_ic21_latch_led_a::DYNALLO_LED), led_state(data, (unsigned)emaxi_ic21_latch_led_a::TRANSPOSE_LED), 
			led_state(data, (unsigned)emaxi_ic21_latch_led_a::PREDEF_LED), led_state(data, (unsigned)emaxi_ic21_latch_led_a::ANAPROC_LED),
            led_state(data, (unsigned)emaxi_ic21_latch_led_a::MASTER_LED), led_state(data, (unsigned)emaxi_ic21_latch_led_a::SAMPLE_LED), 
			led_state(data, (unsigned)emaxi_ic21_latch_led_a::DIGPROC_LED), led_state(data, (unsigned)emaxi_ic21_latch_led_a::PREMAN_LED));
    }, "ic21_latch_led_a");

	map(0x8E4002, 0x8E4002).lw8(
    [this](u8 data)
    {
        logerror("led latch b (%s): 0x%02X (ScnDta=%s ScnClk=%s SyncTime=%s Unlabeled=%s Side=%s Mtr=%s Seq=%s Enter=%s)\n",
            machine().describe_context(), data,
            led_state(data, (unsigned)emaxi_ic22_latch_led_b::SCNDTA), led_state(data, (unsigned)emaxi_ic22_latch_led_b::SCNCLK), 
			led_state(data, (unsigned)emaxi_ic22_latch_led_b::SYNC_TIME), led_state(data, (unsigned)emaxi_ic22_latch_led_b::Unlabeled),
            led_state(data, (unsigned)emaxi_ic22_latch_led_b::SIDE), led_state(data, (unsigned)emaxi_ic22_latch_led_b::MTR), 
			led_state(data, (unsigned)emaxi_ic22_latch_led_b::SEQ_LED), led_state(data, (unsigned)emaxi_ic22_latch_led_b::ENTER_LED));
		
		// Bit 2 controls +SYNC/-TIME.D (0 = TIME mode enabled, 1 = SYNC mode enabled)
		m_sync_time_mode = BIT(data, (unsigned)emaxi_ic22_latch_led_b::SYNC_TIME);
			
        m_fdd->ss_w(!BIT(data, (unsigned)emaxi_ic22_latch_led_b::SIDE));
        m_fdd->mon_w(BIT(data, (unsigned)emaxi_ic22_latch_led_b::MTR));
    }, "ic22_latch_led_b");

	map(0x822000, 0x822000).w(FUNC(emax_state::fdc_cmd_w));
	map(0x822400, 0x822400).r(FUNC(emax_state::fdc_status_r));
	map(0x822800, 0x822800).w(FUNC(emax_state::fdc_track_w));
	map(0x822c00, 0x822c00).r(FUNC(emax_state::fdc_track_r));
	map(0x823000, 0x823000).w(FUNC(emax_state::fdc_sector_w));
	map(0x823400, 0x823400).r(FUNC(emax_state::fdc_sector_r));
	map(0x823800, 0x823800).w(FUNC(emax_state::fdc_data_w));
	map(0x823c00, 0x823c00).r(FUNC(emax_state::fdc_data_r));
	
	/*
	map(0x822000, 0x822000).w(m_fdc, FUNC(wd1772_device::cmd_w));
	map(0x822400, 0x822400).r(m_fdc, FUNC(wd1772_device::status_r));
	map(0x822800, 0x822800).w(m_fdc, FUNC(wd1772_device::track_w));
	map(0x822c00, 0x822c00).r(m_fdc, FUNC(wd1772_device::track_r));
	map(0x823000, 0x823000).w(m_fdc, FUNC(wd1772_device::sector_w));
	map(0x823400, 0x823400).r(m_fdc, FUNC(wd1772_device::sector_r));
	map(0x823800, 0x823800).w(m_fdc, FUNC(wd1772_device::data_w));
	map(0x823c00, 0x823c00).r(m_fdc, FUNC(wd1772_device::data_r));
	*/

	map(0x824004, 0x824004).w(FUNC(emax_state::mux_w));
	map(0x824006, 0x824006).w(FUNC(emax_state::dac_w));
	map(0x890000, 0x890000).w(m_lcdc, FUNC(hd44780_device::control_w));
	map(0x890002, 0x890002).r(m_lcdc, FUNC(hd44780_device::control_r));
	map(0x890004, 0x890004).w(m_lcdc, FUNC(hd44780_device::data_w));
	map(0x890006, 0x890006).r(m_lcdc, FUNC(hd44780_device::data_r));

	// fffe00 = interrupt vector
	map(0xfffe00, 0xfffe00).r(FUNC(emax_state::emaxi_vector_port_r));
	
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
	map(0x818048, 0x818048).w("acia", FUNC(acia6850_device::control_w));
	map(0x81804a, 0x81804a).r("acia", FUNC(acia6850_device::status_r));
	map(0x81804c, 0x81804c).w("acia", FUNC(acia6850_device::data_w));
	map(0x81804e, 0x81804e).r("acia", FUNC(acia6850_device::data_r));
	emax_periphs(map);
}

void emax_state::emaxp_map(address_map &map)
{
	map(0x000000, 0x001fff).rom().region("bootprom", 0);
	map(0x008000, 0x017fff).ram(); // DRAM, 64kb

	map(0x0f8000, 0x0f8000).select(0xe).rw(FUNC(emax_state::hdc_r), FUNC(emax_state::hdc_w));
	map(0x818028, 0x818028).w("acia", FUNC(acia6850_device::control_w));
	map(0x81802a, 0x81802a).r("acia", FUNC(acia6850_device::status_r));
	map(0x81802c, 0x81802c).w("acia", FUNC(acia6850_device::data_w));
	map(0x81802e, 0x81802e).r("acia", FUNC(acia6850_device::data_r));
	emax_periphs(map);
}

void emax_state::emax2_map(address_map &map)
{
	map(0x000000, 0x003fff).rom().region("bootprom", 0);
	
	map(0x008000, 0x087FFF).ram(); // DRAM?
	
	// fffe00 = interrupt vector
	map(0xfffe00, 0xfffe01).r(FUNC(emax_state::emaxii_vector_port_r));
	
	map(0xCF0000, 0xCF0000).lw8(
    [this](u8 data)
    {
        logerror("led latch a (%s): 0x%02X (DrvSelect=%s Trans=%s PreDef=%s DynProc=%s Mast=%s Samp=%s DigProc=%s PreMan=%s)\n",
            machine().describe_context(), data,
            led_state(data, (unsigned)emaxii_ic34_latch_led_a::DRVSELECT_LED), led_state(data, (unsigned)emaxii_ic34_latch_led_a::TRANSPOSE_LED), 
			led_state(data, (unsigned)emaxii_ic34_latch_led_a::PREDEF_LED), led_state(data, (unsigned)emaxii_ic34_latch_led_a::DYNPROC_LED),
            led_state(data, (unsigned)emaxii_ic34_latch_led_a::MASTER_LED), led_state(data, (unsigned)emaxii_ic34_latch_led_a::SAMPLE_LED), 
			led_state(data, (unsigned)emaxii_ic34_latch_led_a::DIGPROC_LED), led_state(data, (unsigned)emaxii_ic34_latch_led_a::PREMAN_LED));
    }, "ic34_latch_led_a");

	map(0xDF0000, 0xDF0000).lw8(
    [this](u8 data)
    {
        logerror("led latch b (%s): 0x%02X (ScnDta=%s ScnClk=%s SyncTime=%s Unlabeled=%s Side=%s Mtr=%s Seq=%s Enter=%s)\n",
            machine().describe_context(), data,
            led_state(data, (unsigned)emaxii_ic35_latch_led_b::SCNDTA), led_state(data, (unsigned)emaxii_ic35_latch_led_b::SCNCLK), 
			led_state(data, (unsigned)emaxii_ic35_latch_led_b::SYNC_TIME), led_state(data, (unsigned)emaxii_ic35_latch_led_b::Unlabeled),
            led_state(data, (unsigned)emaxii_ic35_latch_led_b::SIDE), led_state(data, (unsigned)emaxii_ic35_latch_led_b::MTR), 
			led_state(data, (unsigned)emaxii_ic35_latch_led_b::SEQ_LED), led_state(data, (unsigned)emaxii_ic35_latch_led_b::ENTER_LED));
		
		// Bit 2 controls +SYNC/-TIME.D (0 = TIME mode enabled, 1 = SYNC mode enabled)
		m_sync_time_mode = BIT(data, (unsigned)emaxii_ic35_latch_led_b::SYNC_TIME);
			
        m_fdd->ss_w(!BIT(data, (unsigned)emaxii_ic35_latch_led_b::SIDE));
        m_fdd->mon_w(BIT(data, (unsigned)emaxii_ic35_latch_led_b::MTR));
    }, "ic35_latch_led_b");
	
	map(0x0a8018, 0x0a8018).w("acia1", FUNC(acia6850_device::control_w));
	map(0x0a801a, 0x0a801a).r("acia1", FUNC(acia6850_device::status_r));
	map(0x0a801c, 0x0a801c).w("acia1", FUNC(acia6850_device::data_w));
	map(0x0a801e, 0x0a801e).r("acia1", FUNC(acia6850_device::data_r));
	map(0x0a8028, 0x0a8028).w("acia2", FUNC(acia6850_device::control_w));
	map(0x0a802a, 0x0a802a).r("acia2", FUNC(acia6850_device::status_r));
	map(0x0a802c, 0x0a802c).w("acia2", FUNC(acia6850_device::data_w));
	map(0x0a802e, 0x0a802e).r("acia2", FUNC(acia6850_device::data_r));
	map(0x0b0000, 0x0b0000).w(m_lcdc, FUNC(hd44780_device::control_w));
	map(0x0b0002, 0x0b0002).r(m_lcdc, FUNC(hd44780_device::control_r));
	map(0x0b0004, 0x0b0004).w(m_lcdc, FUNC(hd44780_device::data_w));
	map(0x0b0006, 0x0b0006).r(m_lcdc, FUNC(hd44780_device::data_r));
	map(0x1f8000, 0x1f800f).rw(m_hdc, FUNC(ncr5380_device::read), FUNC(ncr5380_device::write)).umask16(0x00ff);
	map(0x3f8000, 0x3f8007).rw(m_ctc, FUNC(pit8254_device::read), FUNC(pit8254_device::write)).umask16(0x00ff);
	
	map(0x8e8000, 0x8e82ff).ram(); // ?
	
	map(0xAE8000, 0xAE8000).w(FUNC(emax_state::fdc_cmd_w));
	map(0xAE8400, 0xAE8400).r(FUNC(emax_state::fdc_status_r));
	map(0xAE8800, 0xAE8800).w(FUNC(emax_state::fdc_track_w));
	map(0xAE8C00, 0xAE8C00).r(FUNC(emax_state::fdc_track_r));
	map(0xAE9000, 0xAE9000).w(FUNC(emax_state::fdc_sector_w));
	map(0xAE9400, 0xAE9400).r(FUNC(emax_state::fdc_sector_r));
	map(0xAE9800, 0xAE9800).w(FUNC(emax_state::fdc_data_w));
	map(0xAE9C00, 0xAE9C00).r(FUNC(emax_state::fdc_data_r));
}


static INPUT_PORTS_START(emax)
	PORT_START("TEST")
    PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_OTHER) PORT_NAME("Test SCINT") PORT_CODE(KEYCODE_D) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(emax_state::test_scint), 0)
INPUT_PORTS_END

static INPUT_PORTS_START(emax2)
INPUT_PORTS_END

void emax_state::palette_init(palette_device &palette)
{
	palette.set_pen_color(0, rgb_t(131, 136, 139));
	palette.set_pen_color(1, rgb_t( 92,  83,  88));
}

void emax_state::scsihd(machine_config &config)
{
	auto &scsi(NSCSI_BUS(config, "scsi"));
	NSCSI_CONNECTOR(config, "scsi:0", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:1", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:2", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:3", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:4", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:5", default_scsi_devices, nullptr, false);
	NSCSI_CONNECTOR(config, "scsi:6", default_scsi_devices, "harddisk", false);

	NCR5380(config, m_hdc);
	scsi.set_external_device(7, m_hdc);
}

static void emax_floppies(device_slot_interface &device)
{
	device.option_add("35dd", FLOPPY_35_DD);
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

	//R6500_11(config, "scannercpu", 16_MHz_XTAL / 4);

	PIT8254(config, m_ctc);
	m_ctc->set_clk<0>(16_MHz_XTAL / 2); // 8mhz line
	m_ctc->set_clk<1>(16_MHz_XTAL / 2); // 8mhz line
	m_ctc->set_clk<2>(16_MHz_XTAL / 32); // 500khz from IC4
	
	//m_ctc->out_handler<0>(); // +CSCF.D
	//m_ctc->out_handler<1>(); // -TGTIME.D and +ADCCK.D
	m_ctc->out_handler<2>().set(*this, FUNC(emax_state::swtime_w)); // -SWTIME.D and -SMPL.D 

	WD1772(config, m_fdc, 16_MHz_XTAL / 2); // WD1772-PA
	m_fdc->intrq_wr_callback().set(*this, FUNC(emax_state::irq_w<FDCINT>));
	m_fdc->set_disable_motor_control(true);

	FLOPPY_CONNECTOR(config, "fdc:0", emax_floppies, "35dd", add_formats).enable_sound(false);

	ACIA6850(config, "acia"); // MC68A50P
	subdevice<acia6850_device>("acia")->irq_handler().set(*this, FUNC(emax_state::irq_w<MIDINT>));

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

	EEPROM_93C06_16BIT(config, "eeprom"); // NMC93C06N

	PIT8254(config, m_ctc);
	m_ctc->set_clk<0>(16_MHz_XTAL / 2); // 8mhz line
	m_ctc->set_clk<1>(16_MHz_XTAL / 2); // 8mhz line
	m_ctc->set_clk<2>(16_MHz_XTAL / 32); // 500khz line
	
	//m_ctc->out_handler<0>(); // RXC/TXC from IC4(6850)
	//m_ctc->out_handler<1>(); // -TGTIME.D
	m_ctc->out_handler<2>().set(*this, FUNC(emax_state::swtime_w)); // -SWTIME.D

	WD1772(config, m_fdc, 16_MHz_XTAL / 2);
	m_fdc->intrq_wr_callback().set(*this, FUNC(emax_state::irq_w<FDCINT>));
	m_fdc->set_disable_motor_control(true);

	FLOPPY_CONNECTOR(config, "fdc:0", emax_floppies, "35dd", add_formats).enable_sound(false);

	ACIA6850(config, "acia1"); // IC3?
	subdevice<acia6850_device>("acia1")->irq_handler().set(*this, FUNC(emax_state::irq_w<MIDINT>));
	
	ACIA6850(config, "acia2"); // IC4? --- note from manual: "1C4 IS UNSTUFFED AND IS USED FOR DEBUGGING ONLY"
	subdevice<acia6850_device>("acia2")->irq_handler().set(*this, FUNC(emax_state::irq_w<MIDINT>));

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