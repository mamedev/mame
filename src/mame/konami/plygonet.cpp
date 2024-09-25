// license:BSD-3-Clause
// copyright-holders:R. Belmont, Andrew Gardner, Ryan Holtz
/*
    Polygonet Commanders (Konami, 1993)
    Poly-Net Warriors (Konami, 1993)

    Preliminary driver by R. Belmont
    Additional work by Andrew Gardner
    DSP56156 fixes and K054009/K054010 span rendering by Ryan Holtz

    This is Konami's first 3D game!

    Hardware:
    68EC020 @ 16 MHz
    Motorola XC56156-40 DSP @ 40 MHz
    Z80 + K054539 for sound
    K056230 for network (up to four players)

    Video hardware:
    TTL text plane similar to Run and Gun.
    Konami K054009(x2) + K054010(x2) (polygon rasterizers)
    Konami K053936 "PSAC2" (3d roz plane, used for backgrounds)
    24.0 MHz crystal to drive the video hardware

    Driver includes:
    - 68020 memory map
    - Z80 + sound system
    - EEPROM
    - TTL text plane
    - Controls
    - Palettes

    Driver needs:
    - Network at 580800 (K056230)

    Tech info by Phil Bennett, from the schematics:

    68000 address map
    =================

    400000-43ffff = PSAC
    440000-47ffff = PSVR
    480000-4bffff = IO
    4c0000-4fffff = SYS
    500000-53ffff = DSP
    540000-57ffff = FIX
    580000-5bffff = OP1
    5c0000-5fffff = UNUSED


    DSP
    ===

    500000-503fff = HCOM     - 16kB shared RAM
    504000-504fff = CONTROL  - DSP/Host Control
                    D10? = COMBNK - Switch between 68k and DSP access to shared RAM
                    D08? = RESN   - Reset DSP
    506000-506fff = HEN      - DSP/Host interface

    Debugging notes:
    - Player's tank currently flickers on and off during the first in-game attract-mode segment.
    - Having bespoke code to trigger a breakpoint on frame 2300 gets close to the affected section.
*/

#include "emu.h"

#include "cpu/dsp56156/dsp56156.h"
#include "cpu/m68000/m68020.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "machine/k054321.h"
#include "machine/k056230.h"
#include "machine/watchdog.h"
#include "sound/k054539.h"
#include "video/k053936.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

#define LOG_DSP_AB0         (1U << 1)
#define LOG_DSP_A6          (1U << 2)
#define LOG_DSP_A7          (1U << 3)
#define LOG_DSP_A8          (1U << 4)
#define LOG_DSP_AC          (1U << 5)
#define LOG_DSP_AE          (1U << 6)
#define LOG_DSP_B6          (1U << 7)
#define LOG_DSP_B7          (1U << 8)
#define LOG_DSP_B8          (1U << 9)
#define LOG_68K_SHARED_RD   (1U << 10)
#define LOG_68K_SHARED_WR   (1U << 11)
#define LOG_DSP_HOST_INTF   (1U << 12)
#define LOG_DSP_CTRL        (1U << 13)
#define LOG_DSP_PORTC       (1U << 14)

#define LOG_ALL_DSP_A       (LOG_DSP_AB0 | LOG_DSP_A7 | LOG_DSP_A6 | LOG_DSP_A8 | LOG_DSP_AC | LOG_DSP_AE)
#define LOG_ALL_DSP_B       (LOG_DSP_AB0 | LOG_DSP_B6 | LOG_DSP_B7 | LOG_DSP_B8)

//#define VERBOSE (LOG_DSP_B6 | LOG_DSP_B7 | LOG_DSP_B8 | LOG_DSP_AE | LOG_DSP_HOST_INTF | LOG_DSP_CTRL | LOG_DSP_PORTC)
#define VERBOSE (0)
#include "logmacro.h"

namespace {

class polygonet_state : public driver_device
{
public:
	polygonet_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_dsp(*this, "dsp"),
		m_watchdog(*this, "watchdog"),
		m_eeprom(*this, "eeprom"),
		m_k056230(*this, "lanc"),
		m_k053936(*this, "k053936"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_ttl_vram(*this, "ttl_vram"),
		m_fix_regs(*this, "fix_regs"),
		m_roz_vram(*this, "roz_vram"),
		m_k054321(*this, "k054321"),
		m_sound_bank(*this, "bank1"),
		m_dsp_bank_a_8(*this, "dsp_bank_a_8"),
		m_dsp_bank_b_6(*this, "dsp_bank_b_6"),
		m_dsp_bank_b_7(*this, "dsp_bank_b_7"),
		m_dsp_bank_b_8(*this, "dsp_bank_b_8"),
		m_dsp_common(*this, "dsp_common"),
		m_dsp_share(*this, "dsp_share"),
		m_dsp_ab_0(*this, "dsp_ab_0"),
		m_dsp_a_6(*this, "dsp_a_6"),
		m_dsp_a_7(*this, "dsp_a_7"),
		m_dsp_a_e(*this, "dsp_a_e"),
		m_dsp_ram_a_8(*this, "dsp_ram_a_8", 8 * 0x4000U * 2, ENDIANNESS_BIG),
		m_dsp_ram_b_6(*this, "dsp_ram_b_6", 4 * 0x1000U * 2, ENDIANNESS_BIG),
		m_dsp_ram_b_7(*this, "dsp_ram_b_7", 4 * 0x1000U * 2, ENDIANNESS_BIG),
		m_dsp_ram_b_8(*this, "dsp_ram_b_8", 8 * 0x8000U * 2, ENDIANNESS_BIG),
		m_dsp_data_view(*this, "dspdata"),
		m_inputs(*this, "IN%u", 0U),
		m_eepromout(*this, "EEPROMOUT"),
		m_sys1(0),
		m_dsp_portc(0),
		m_ttl_gfx_index(0),
		m_ttl_tilemap(nullptr),
		m_roz_tilemap(nullptr),
		m_sound_ctrl(0),
		m_sound_intck(0)
	{ }

	void plygonet(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void dsp_program_map(address_map &map) ATTR_COLD;
	void dsp_data_map(address_map &map) ATTR_COLD;

	// Main-board handlers
	void sys_w(offs_t offset, u8 data);
	u8 inputs_r(offs_t offset);
	void sound_irq_w(u32 data);
	u32 dsp_host_interface_r(offs_t offset, u32 mem_mask = ~0);
	void dsp_host_interface_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 shared_ram_read(offs_t offset, u32 mem_mask = ~0);
	void shared_ram_write(offs_t offset, u32 data, u32 mem_mask = ~0);
	void dsp_w_lines(offs_t offset, u32 data, u32 mem_mask = ~0);

	// DSP handlers
	u16 dsp_bootload_r();
	void dsp_portc_write(u16 data);
	u16 dsp_ram_ab_0_read(offs_t offset);
	void dsp_ram_ab_0_write(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 dsp_ram_a_6_read(offs_t offset);
	void dsp_ram_a_6_write(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 dsp_ram_b_6_read(offs_t offset);
	void dsp_ram_b_6_write(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 dsp_ram_a_7_read(offs_t offset);
	void dsp_ram_a_7_write(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 dsp_ram_b_7_read(offs_t offset);
	void dsp_ram_b_7_write(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 dsp_ram_a_8_read(offs_t offset);
	void dsp_ram_a_8_write(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 dsp_ram_b_8_read(offs_t offset);
	void dsp_ram_b_8_write(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 dsp_ram_a_c_read(offs_t offset);
	void dsp_ram_a_c_write(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 dsp_ram_a_e_read(offs_t offset);
	void dsp_ram_a_e_write(offs_t offset, u16 data, u16 mem_mask = ~0);

	// Video handlers
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_interrupt);
	void ttl_vram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	u32 fix_regs_r(offs_t offset, u32 mem_mask = ~0);
	void fix_regs_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void roz_vram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	TILE_GET_INFO_MEMBER(ttl_get_tile_info);
	TILE_GET_INFO_MEMBER(roz_get_tile_info);

	// Sound handlers
	void sound_ctrl_w(u8 data);
	void update_sound_nmi();
	void k054539_nmi_gen(int state);

	template <int PolyPage> void process_polys();
	template <int PolyPage> void draw_poly(bitmap_rgb32 &bitmap, const u16 raw_color, const u16 span_ptr, const u16 raw_start, const u16 raw_end);

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<dsp56156_device> m_dsp;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<eeprom_serial_er5911_device> m_eeprom;

	required_device<k056230_device> m_k056230;

	required_device<k053936_device> m_k053936;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<u32> m_ttl_vram;
	required_shared_ptr<u32> m_fix_regs;
	required_shared_ptr<u32> m_roz_vram;
	required_device<k054321_device> m_k054321;
	required_memory_bank m_sound_bank;

	memory_bank_creator m_dsp_bank_a_8;
	memory_bank_creator m_dsp_bank_b_6;
	memory_bank_creator m_dsp_bank_b_7;
	memory_bank_creator m_dsp_bank_b_8;
	required_shared_ptr<u16> m_dsp_common;
	required_shared_ptr<u16> m_dsp_share;
	required_shared_ptr<u16> m_dsp_ab_0;
	required_shared_ptr<u16> m_dsp_a_6;
	required_shared_ptr<u16> m_dsp_a_7;
	required_shared_ptr<u16> m_dsp_a_e;
	memory_share_creator<u16> m_dsp_ram_a_8;
	memory_share_creator<u16> m_dsp_ram_b_6;
	memory_share_creator<u16> m_dsp_ram_b_7;
	memory_share_creator<u16> m_dsp_ram_b_8;
	memory_view m_dsp_data_view;

	required_ioport_array<4> m_inputs;
	required_ioport m_eepromout;

	// Main-board members
	u8 m_sys1;

	// DSP members
	u16 m_dsp_portc;

	// Video members
	int m_ttl_gfx_index;
	tilemap_t *m_ttl_tilemap;
	tilemap_t *m_roz_tilemap;

	// Sound members
	u8 m_sound_ctrl;
	int m_sound_intck;

	// Span drawer management
	bitmap_rgb32 m_pla_bitmaps[2];
	bitmap_rgb32 m_plb_bitmaps[2];
	u16 m_render_buf_idx[2];
	u16 m_display_buf_idx[2];
};

//-------------------------------------------------
//  Machine-related helpers
//-------------------------------------------------

void polygonet_state::machine_start()
{
	m_pla_bitmaps[0].allocate(384, 256);
	m_pla_bitmaps[1].allocate(384, 256);
	m_plb_bitmaps[0].allocate(384, 256);
	m_plb_bitmaps[1].allocate(384, 256);

	m_sound_bank->configure_entries(0, 8, memregion("audiocpu")->base(), 0x4000);

	// Initialize DSP banking
	m_dsp_bank_a_8->configure_entries(0, 8, m_dsp_ram_a_8.target(), 0x4000 * 2);
	m_dsp_bank_b_6->configure_entries(0, 4, m_dsp_ram_b_6.target(), 0x1000 * 2);
	m_dsp_bank_b_7->configure_entries(0, 4, m_dsp_ram_b_7.target(), 0x1000 * 2);
	m_dsp_bank_b_8->configure_entries(0, 8, m_dsp_ram_b_8.target(), 0x8000 * 2);
	m_dsp_data_view.select(0);

	// Register save states
	save_item(NAME(m_sys1));
	save_item(NAME(m_sound_ctrl));
	save_item(NAME(m_sound_intck));
	save_item(NAME(m_dsp_portc));
	save_item(NAME(m_render_buf_idx));
	save_item(NAME(m_display_buf_idx));
}

void polygonet_state::machine_reset()
{
	m_sound_bank->set_entry(0);

	m_sys1 = 0;
	m_sound_ctrl = 0;
	m_sound_intck = 0;
	m_dsp_portc = 0;

	std::fill(std::begin(m_render_buf_idx), std::end(m_render_buf_idx), 0);
	std::fill(std::begin(m_display_buf_idx), std::end(m_display_buf_idx), 0);

	// It's assumed the hardware has hard-wired operating mode 1 (MODA = 1, MODB = 0)
	m_dsp->set_input_line(DSP56156_IRQ_RESET, ASSERT_LINE);
	m_dsp->set_input_line(DSP56156_IRQ_MODA, ASSERT_LINE);
	m_dsp->set_input_line(DSP56156_IRQ_MODB, CLEAR_LINE);
}


//-------------------------------------------------
//  Input ports
//-------------------------------------------------

static INPUT_PORTS_START( polygonet )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x02, IP_ACTIVE_LOW )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) // Start 1, unused
	PORT_DIPNAME( 0x30, 0x00, "Player Color/Network ID" ) // 0x10(SW1), 0x20(SW2) is mapped on the JAMMA connector and plugs into an external switch mech.
	PORT_DIPSETTING(    0x00, "Red" )
	PORT_DIPSETTING(    0x10, "Yellow" )
	PORT_DIPSETTING(    0x20, "Green" )
	PORT_DIPSETTING(    0x30, "Blue" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, do_read)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, ready_read)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED ) // Start 2, unused
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, di_write)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, cs_write)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_er5911_device, clk_write)
INPUT_PORTS_END

static INPUT_PORTS_START( polynetw )
	PORT_INCLUDE( polygonet )

	PORT_MODIFY("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


//-------------------------------------------------
//  Machine-related handlers
//-------------------------------------------------

u8 polygonet_state::inputs_r(offs_t offset)
{
	return m_inputs[offset]->read();
}

void polygonet_state::sys_w(offs_t offset, u8 data)
{
	switch (offset)
	{
		case 0:
			// D28 = /FIXKILL     - Disable 'FIX' layer?
			// D27 = MUTE
			// D26 = EEPROM CLK
			// D25 = EEPROM CS
			// D24 = EEPROM DATA
			m_eepromout->write(data, 0xffff);
			break;

		case 1:
			// D23 = BRMAS        - 68k bus error mask
			// D22 = L7MAS        - L7 interrupt mask (unused - should always be '1')
			// D21 = /L5MAS       - L5 interrupt mask/acknowledge (vblank)
			// D20 = L3MAS        - L3 interrupt mask (056230)
			// D19 = VFLIP        - Flip video vertically
			// D18 = HFLIP        - Flip video horizontally
			// D17 = COIN2        - Coin counter 2
			// D16 = COIN1        - Coin counter 1
			machine().bookkeeping().coin_counter_w(0, data & 1);
			machine().bookkeeping().coin_counter_w(1, data & 2);
			if (BIT(~data, 5))
				m_maincpu->set_input_line(M68K_IRQ_5, CLEAR_LINE);

			m_sys1 = data;
			LOG("sys1 write: %02x\n", data);
			break;

		default:
			LOG("Unknown sys_w write: %08x = %02x\n", offset, data);
			break;
	}
}


// IRQs 3, 5, and 7 have valid vectors
//   IRQ 3 is network, currently disabled
//   IRQ 5 is vblank
//   IRQ 7 does nothing (it JSRs to a RTS and then RTE)

INTERRUPT_GEN_MEMBER(polygonet_state::vblank_interrupt)
{
	if (BIT(m_sys1, 5))
		device.execute().set_input_line(M68K_IRQ_5, ASSERT_LINE);
}

void polygonet_state::sound_irq_w(u32 data)
{
	// Auto-acknowledge interrupt
	m_audiocpu->set_input_line(0, HOLD_LINE);
}


//-------------------------------------------------
//  68k <-> DSP comms
//-------------------------------------------------

u32 polygonet_state::dsp_host_interface_r(offs_t offset, u32 mem_mask)
{
	offs_t hi_addr = 0;
	u32 value = 0;
	if (ACCESSING_BITS_8_15) // Low byte
	{
		hi_addr = (offset << 1) + 1;
		value = m_dsp->host_interface_read(hi_addr) << 8;
	}
	else if (ACCESSING_BITS_24_31) // High byte
	{
		hi_addr = offset << 1;
		value = m_dsp->host_interface_read(hi_addr) << 24;
	}

	LOGMASKED(LOG_DSP_HOST_INTF, "%s: 68k Reading DSP Host Interface address %04x: %08x & %08x\n", machine().describe_context(),
		hi_addr, value, mem_mask);

	return value;
}

void polygonet_state::dsp_host_interface_w(offs_t offset, u32 data, u32 mem_mask)
{
	offs_t hi_addr = 0;
	u8 hi_data = 0;
	if (ACCESSING_BITS_8_15) // Low byte
	{
		hi_addr = (offset << 1) + 1;
		hi_data = (u8)(data >> 8);
	}
	else if (ACCESSING_BITS_24_31) // High byte
	{
		hi_addr = offset << 1;
		hi_data = (u8)(data >> 24);
	}

	LOGMASKED(LOG_DSP_HOST_INTF, "%s: 68k Writing to DSP Host Interface address %04x (68k addr %08x) = %08x & %08x\n", machine().describe_context(),
		hi_addr, offset, data, mem_mask);

	m_dsp->host_interface_write(hi_addr, hi_data);
}

u32 polygonet_state::shared_ram_read(offs_t offset, u32 mem_mask)
{
	const u32 data = (m_dsp_share[offset << 1] << 16) | m_dsp_share[(offset << 1) + 1];
	LOGMASKED(LOG_68K_SHARED_RD, "%s: 68k Reading from shared DSP RAM[%04x]: %08x & %08x \n", machine().describe_context(), 0xc000 + (offset << 1), data, mem_mask);
	return data;
}

void polygonet_state::shared_ram_write(offs_t offset, u32 data, u32 mem_mask)
{
	// Write to the current DSP word
	if (ACCESSING_BITS_16_31)
	{
		m_dsp_share[offset << 1] = (u16)(data >> 16);
		LOGMASKED(LOG_68K_SHARED_WR, "%s: 68k Writing to shared DSP RAM[%04x] = %04x\n", machine().describe_context(), 0xc000 + (offset << 1), (u16)(data >> 16));
	}

	// Write to the next DSP word
	if (ACCESSING_BITS_0_15)
	{
		m_dsp_share[(offset << 1) + 1] = (u16)data;
		LOGMASKED(LOG_68K_SHARED_WR, "%s: 68k Writing to shared DSP RAM[%04x] = %04x\n", machine().describe_context(), 0xc000 + (offset << 1) + 1, (u16)data);
	}
}

void polygonet_state::dsp_w_lines(offs_t offset, u32 data, u32 mem_mask)
{
	LOGMASKED(LOG_DSP_CTRL, "%s: 68k writing to DSP control lines: %08x & %08x\n", machine().describe_context(), data, mem_mask);

	// 0x01000000 is the reset line, active-low
	m_dsp->set_input_line(DSP56156_IRQ_RESET, BIT(data, 24) ? CLEAR_LINE : ASSERT_LINE);

	// 0x04000000 is the COMBNK line - it switches who has access to the shared RAM - the dsp or the 68020
}



//-------------------------------------------------
//  DSP I/O management
//-------------------------------------------------

// It's believed this is hard-wired to return (at least) bit 15 as 0 - causes a host interface bootup
u16 polygonet_state::dsp_bootload_r()
{
	return 0x7fff;
}

// The DSP56156's Port C Data register (0xffe3):
// Program code (function 4e) configures it as general purpose output I/O pins (ffc1 = 0000 & ffc3 = 0fff).
//
//    XXXX ---- ---- ----  . Reserved bits
//    ---- ???- -?-- ----  . unknown
//    ---- ---- --x- ----  . [Bank Group A] Enable bit
//    ---- ---- ---x xx--  . [Group A bank control] Banks memory from 0x8000-0xbfff
//    ---- ---- ---- --x-  . [Bank Group B] Enable bit
//    ---- ---x x--- ---x  . [Group B bank control] Banks memory at 0x6000-0x6fff and 0x7000-0x7fff (bits 7,8 only), and 0x8000-0xffbf (bits 7,8,0)

void polygonet_state::dsp_portc_write(u16 data)
{
	LOGMASKED(LOG_DSP_PORTC, "%s: DSP Port C write: %04x\n", machine().describe_context(), data);
	m_dsp_portc = data;

	const u8 bank_a_8_num = bitswap<3>(m_dsp_portc, 4, 3, 2);
	m_dsp_bank_a_8->set_entry(bank_a_8_num);

	const u8 bank_b_67_num = bitswap<2>(m_dsp_portc, 7, 8);
	m_dsp_bank_b_6->set_entry(bank_b_67_num);
	m_dsp_bank_b_7->set_entry(bank_b_67_num);

	const u8 bank_b_8_num = bitswap<3>(m_dsp_portc, 7, 8, 0);
	m_dsp_bank_b_8->set_entry(bank_b_8_num);

	m_dsp_data_view.select(BIT(m_dsp_portc, 1));
}


//-------------------------------------------------
//  DSP RAM handlers
//-------------------------------------------------

u16 polygonet_state::dsp_ram_ab_0_read(offs_t offset)
{
	const u16 data = m_dsp_ab_0[offset];
	LOGMASKED(LOG_DSP_AB0, "%s: DSP Reading from Mapping A/B, 0xxx RAM[%04x]: %04x\n", machine().describe_context(), offset, data);
	return data;
}

void polygonet_state::dsp_ram_ab_0_write(offs_t offset, u16 data, u16 mem_mask)
{
	LOGMASKED(LOG_DSP_AB0, "%s: DSP Writing to Mapping A/B, 0xxx RAM[%04x] = %04x\n", machine().describe_context(), offset, data);
	COMBINE_DATA(&m_dsp_ab_0[offset]);
}


u16 polygonet_state::dsp_ram_a_6_read(offs_t offset)
{
	const u16 data = m_dsp_a_6[offset];
	LOGMASKED(LOG_DSP_A6, "%s: DSP Reading from Mapping A, 6xxx RAM[%04x]: %04x\n", machine().describe_context(), 0x6000 + offset, data);
	return data;
}

void polygonet_state::dsp_ram_a_6_write(offs_t offset, u16 data, u16 mem_mask)
{
	LOGMASKED(LOG_DSP_A6, "%s: DSP Writing to Mapping A, 6xxx RAM[%04x] = %04x\n", machine().describe_context(), 0x6000 + offset, data);
	COMBINE_DATA(&m_dsp_a_6[offset]);
}


u16 polygonet_state::dsp_ram_b_6_read(offs_t offset)
{
	const offs_t bank_b_6_offset = bitswap<2>(m_dsp_portc, 7, 8) * 0x1000;
	const u16 data = ((u16 *)m_dsp_bank_b_6->base())[offset];
	LOGMASKED(LOG_DSP_B6, "%s: DSP Reading from Mapping B, 6xxx RAM[%04x]: %04x (bank offset %04x)\n", machine().describe_context(), 0x6000 + offset, data, bank_b_6_offset + offset);
	return data;
}

void polygonet_state::dsp_ram_b_6_write(offs_t offset, u16 data, u16 mem_mask)
{
	const offs_t bank_b_6_offset = bitswap<2>(m_dsp_portc, 7, 8) * 0x1000;
	LOGMASKED(LOG_DSP_B6, "%s: DSP Writing to Mapping B, 6xxx RAM[%04x] = %04x (bank offset %04x)\n", machine().describe_context(), 0x6000 + offset, data, bank_b_6_offset + offset);
	COMBINE_DATA(((u16 *)m_dsp_bank_b_6->base()) + offset);
}


u16 polygonet_state::dsp_ram_a_7_read(offs_t offset)
{
	const u16 data = m_dsp_a_7[offset];
	LOGMASKED(LOG_DSP_A7, "%s: DSP Reading from Mapping A, 7xxx RAM[%04x]: %04x\n", machine().describe_context(), 0x7000 + offset, data);
	return data;
}

void polygonet_state::dsp_ram_a_7_write(offs_t offset, u16 data, u16 mem_mask)
{
	LOGMASKED(LOG_DSP_A7, "%s: DSP Writing to Mapping A, 7xxx RAM[%04x] = %04x\n", machine().describe_context(), 0x7000 + offset, data);
	COMBINE_DATA(&m_dsp_a_7[offset]);
	COMBINE_DATA(&m_dsp_common[offset]);
}


u16 polygonet_state::dsp_ram_b_7_read(offs_t offset)
{
	const offs_t bank_b_7_offset = bitswap<2>(m_dsp_portc, 7, 8) * 0x1000;
	const u16 data = ((u16 *)m_dsp_bank_b_7->base())[offset];
	LOGMASKED(LOG_DSP_B7, "%s: DSP Reading from Mapping B, 7xxx RAM[%04x]: %04x (bank offset %04x)\n", machine().describe_context(), 0x7000 + offset, data, bank_b_7_offset + offset);
	return data;
}

void polygonet_state::dsp_ram_b_7_write(offs_t offset, u16 data, u16 mem_mask)
{
	const offs_t bank_b_7_offset = bitswap<2>(m_dsp_portc, 7, 8) * 0x1000;
	LOGMASKED(LOG_DSP_B7, "%s: DSP Writing to Mapping B, 7xxx RAM[%04x] = %04x (bank offset %05x)\n", machine().describe_context(), 0x7000 + offset, data, bank_b_7_offset + offset);
	COMBINE_DATA(((u16 *)m_dsp_bank_b_7->base()) + offset);
}


u16 polygonet_state::dsp_ram_a_8_read(offs_t offset)
{
	const offs_t bank_a_8_offset = bitswap<3>(m_dsp_portc, 4, 3, 2) * 0x4000;
	const u16 data = ((u16 *)m_dsp_bank_a_8->base())[offset];
	LOGMASKED(LOG_DSP_A8, "%s: DSP Reading from Mapping A, 8xxx RAM[%04x]: %04x (bank offset %05x)\n", machine().describe_context(), 0x8000 + offset, data, bank_a_8_offset + offset);
	return data;
}

void polygonet_state::dsp_ram_a_8_write(offs_t offset, u16 data, u16 mem_mask)
{
	const offs_t bank_a_8_offset = bitswap<3>(m_dsp_portc, 4, 3, 2) * 0x4000;
	LOGMASKED(LOG_DSP_A8, "%s: DSP Writing to Mapping A, 8xxx RAM[%04x] = %04x (bank offset %05x)\n", machine().describe_context(), 0x8000 + offset, data, bank_a_8_offset + offset);
	COMBINE_DATA(((u16 *)m_dsp_bank_a_8->base()) + offset);
}


u16 polygonet_state::dsp_ram_b_8_read(offs_t offset)
{
	const offs_t bank_b_8_offset = bitswap<3>(m_dsp_portc, 7, 8, 0) * 0x8000;
	const u16 data = ((u16 *)m_dsp_bank_b_8->base())[offset];
	LOGMASKED(LOG_DSP_B8, "%s: DSP Reading from Mapping B, 8xxx RAM[%04x]: %04x (bank offset %05x)\n", machine().describe_context(), 0x8000 + offset, data, bank_b_8_offset + offset);
	return data;
}

void polygonet_state::dsp_ram_b_8_write(offs_t offset, u16 data, u16 mem_mask)
{
	const offs_t bank_b_8_offset = bitswap<3>(m_dsp_portc, 7, 8, 0) * 0x8000;
	LOGMASKED(LOG_DSP_B8, "%s: DSP Writing to Mapping B, 8xxx RAM[%04x] = %04x (bank offset %05x)\n", machine().describe_context(), 0x8000 + offset, data, bank_b_8_offset + offset);
	COMBINE_DATA(((u16 *)m_dsp_bank_b_8->base()) + offset);
}


u16 polygonet_state::dsp_ram_a_c_read(offs_t offset)
{
	const u16 data = m_dsp_share[offset];
	LOGMASKED(LOG_DSP_AC, "%s: DSP Reading from Mapping A, Cxxx RAM[%04x]: %04x\n", machine().describe_context(), 0xc000 + offset, data);
	return data;
}

void polygonet_state::dsp_ram_a_c_write(offs_t offset, u16 data, u16 mem_mask)
{
	LOGMASKED(LOG_DSP_AC, "%s: DSP Writing to Mapping A, Cxxx RAM[%04x] = %04x\n", machine().describe_context(), 0xc000 + offset, data);
	COMBINE_DATA(&m_dsp_share[offset]);
}


u16 polygonet_state::dsp_ram_a_e_read(offs_t offset)
{
	const u16 data = m_dsp_a_e[offset];
	LOGMASKED(LOG_DSP_AE, "%s: DSP Reading from Mapping A, Exxx RAM[%04x]: %04x\n", machine().describe_context(), 0xe000 + offset, data);
	return data;
}

void polygonet_state::dsp_ram_a_e_write(offs_t offset, u16 data, u16 mem_mask)
{
	LOGMASKED(LOG_DSP_AE, "%s: DSP Writing to Mapping A, Exxx RAM[%04x] = %04x\n", machine().describe_context(), 0xe000 + offset, data);
	const u16 old = m_dsp_a_e[offset];
	if (offset < 2 && BIT(old, 0) != BIT(data, 0))
	{
		m_render_buf_idx[offset] = BIT(data, 0);
		m_display_buf_idx[offset] = BIT(old, 0);
		if (offset == 0)
			process_polys<0>();
		else
			process_polys<1>();
	}
	COMBINE_DATA(&m_dsp_a_e[offset]);
}

template <int PolyPage>
void polygonet_state::process_polys()
{
	const u16 buf_idx = m_render_buf_idx[PolyPage];
	static const offs_t s_info_bank_offsets[2][2] = { { 0x0000, 0x2000 }, { 0x1000, 0x3000 } };
	const offs_t bank_offset = s_info_bank_offsets[PolyPage][buf_idx];
	const u16 *b6_data = (u16 *)m_dsp_ram_b_6.target() + bank_offset;
	const u16 *b7_data = (u16 *)m_dsp_ram_b_7.target() + bank_offset;

	if (b6_data[0] == 6 && b6_data[1] == 6)
		return;

	bitmap_rgb32 &bitmap = PolyPage ? m_plb_bitmaps[buf_idx] : m_pla_bitmaps[buf_idx];

	if (BIT(b7_data[1], 15))
		bitmap.fill(0x00000000);

	for (offs_t bank_idx = 0; BIT(b7_data[bank_idx + 1], 15) && bank_idx < 0x1000; bank_idx += 2)
	{
		const u16 raw_color = b7_data[bank_idx];
		const u16 span_ptr = b7_data[bank_idx + 1];
		const u16 raw_start = b6_data[bank_idx];
		const u16 raw_end = b6_data[bank_idx + 1];
		draw_poly<PolyPage>(bitmap, raw_color, span_ptr, raw_start, raw_end);
	}
}

template <int PolyPage>
void polygonet_state::draw_poly(bitmap_rgb32 &bitmap, const u16 raw_color, const u16 span_ptr, const u16 raw_start, const u16 raw_end)
{
	const u16 buf_idx = m_render_buf_idx[PolyPage];
	const u16 *span_data_buf = (u16 *)m_dsp_ram_b_8.target();
	const offs_t page_offset = PolyPage ? 0x10000 : 0x0000;
	const offs_t frame_offset = buf_idx ? 0x20000 : 0x00000;
	const offs_t start_offset = page_offset + frame_offset;
	const offs_t end_offset = page_offset + frame_offset + 0x8000U;

	const u8 r = ((raw_color >> 7) & 0xf8) | ((raw_color >> 12) & 7);
	const u8 g = ((raw_color >> 2) & 0xf8) | ((raw_color >> 7) & 7);
	const u8 b = ((raw_color << 3) & 0xf8) | ((raw_color >> 2) & 7);
	const u32 color888 = 0xff000000 | (r << 16) | (g << 8) | b;

	s16 y_start = (s16)raw_start >> 5;
	s16 y_end = (s16)raw_end >> 5;
	if (y_start == y_end)
	{
		return;
	}
	if (y_start > y_end)
	{
		std::swap(y_start, y_end);
	}

	offs_t start_addr = start_offset + (span_ptr & 0x7fff);
	offs_t end_addr = end_offset + (span_ptr & 0x7fff);

	for (s16 y = y_start; y < y_end; y++, start_addr++, end_addr++)
	{
		const u16 bitmap_y = u16(y + 1024) - 896;

		if (bitmap_y < 256)
		{
			const s16 x_start = (s16)span_data_buf[start_addr] >> 5;
			const s16 x_end = (s16)span_data_buf[end_addr] >> 5;
			u32 *dst = &bitmap.pix(bitmap_y);
			for (s16 x = x_start; x <= x_end; x++)
			{
				const u16 bitmap_x = u16(x + 1024) - 832;
				if (bitmap_x < 384 && (dst[bitmap_x] & 0xff000000) == 0)
				{
					dst[bitmap_x] = color888;
				}
			}
		}
	}
}

//-------------------------------------------------
//  Video hardware
//-------------------------------------------------

static GFXDECODE_START( gfx_plygonet )
	GFXDECODE_ENTRY( "gfx2", 0, gfx_16x16x4_packed_msb, 0x0000, 64 )
GFXDECODE_END

TILE_GET_INFO_MEMBER(polygonet_state::ttl_get_tile_info)
{
	const auto ttl_vram = util::big_endian_cast<const u16>(m_ttl_vram.target());
	const int code = ttl_vram[tile_index] & 0xfff;
	const int attr = ttl_vram[tile_index] >> 12;  // Is the palette in all 4 bits?

	tileinfo.set(m_ttl_gfx_index, code, attr, 0);
}

TILE_GET_INFO_MEMBER(polygonet_state::roz_get_tile_info)
{
	if (tile_index < 0x800)
	{
		const auto roz_vram = util::big_endian_cast<const u16>(m_roz_vram.target());
		const int code = roz_vram[tile_index] & 0x7ff;
		const int attr = (roz_vram[tile_index] >> 12) + 16; // ROZ base palette is palette index 16 onward
		tileinfo.set(0, code, attr, 0);
	}
	else
	{
		tileinfo.set(0, 0, 0, 0);
	}
}

void polygonet_state::ttl_vram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_ttl_vram[offset]);
	m_ttl_tilemap->mark_tile_dirty(offset << 1);
	m_ttl_tilemap->mark_tile_dirty((offset << 1) + 1);
}

void polygonet_state::fix_regs_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_fix_regs[offset]);
	LOG("fix_regs_w: %08x = %08x & %08x\n", offset, data, mem_mask);
}

u32 polygonet_state::fix_regs_r(offs_t offset, u32 mem_mask)
{
	const u32 data = m_fix_regs[offset];
	LOG("fix_regs_r: %08x: %08x & %08x\n", offset, data, mem_mask);
	return data;
}

void polygonet_state::roz_vram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_roz_vram[offset]);
	m_roz_tilemap->mark_tile_dirty(offset << 1);
	m_roz_tilemap->mark_tile_dirty((offset << 1) + 1);
}

void polygonet_state::video_start()
{
	static const gfx_layout charlayout =
	{
		8, 8,   // 8x8
		4096,   // # of tiles
		4,      // 4bpp
		{ 0, 1, 2, 3 }, // plane offsets
		{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 }, // X offsets
		{ 0*8*4, 1*8*4, 2*8*4, 3*8*4, 4*8*4, 5*8*4, 6*8*4, 7*8*4 }, // Y offsets
		8*8*4
	};

	// find first empty slot to decode gfx
	for (m_ttl_gfx_index = 0; m_ttl_gfx_index < MAX_GFX_ELEMENTS; m_ttl_gfx_index++)
		if (m_gfxdecode->gfx(m_ttl_gfx_index) == nullptr)
			break;

	assert(m_ttl_gfx_index != MAX_GFX_ELEMENTS);

	// Decode the TTL layer's graphics
	m_gfxdecode->set_gfx(m_ttl_gfx_index, std::make_unique<gfx_element>(m_palette, charlayout, memregion("gfx1")->base(), 0, m_palette->entries() / 16, 0));

	// Create the tilemap
	m_ttl_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(polygonet_state::ttl_get_tile_info)), TILEMAP_SCAN_ROWS,  8, 8, 64, 32);
	m_ttl_tilemap->set_transparent_pen(0);

	// Set up the ROZ tilemap
	m_roz_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(polygonet_state::roz_get_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 64, 64);
	m_roz_tilemap->set_transparent_pen(0);

	// Register save states
	save_item(NAME(m_ttl_gfx_index));
}

u32 polygonet_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0);
	bitmap.fill(m_palette->pens()[0x100], cliprect);

	m_k053936->zoom_draw(screen, bitmap, cliprect, m_roz_tilemap, 0, 0, 0);

	bitmap_rgb32 &bitmap_a = m_pla_bitmaps[m_display_buf_idx[0]];
	bitmap_rgb32 &bitmap_b = m_plb_bitmaps[m_display_buf_idx[1]];
	for (int y = 0; y < 256; y++)
	{
		u32 *dst = &bitmap.pix(y, cliprect.min_x);
		u32 *src_b = &bitmap_b.pix(y);
		u32 *src_a = &bitmap_a.pix(y);
		for (int x = 0; x < 384; x++)
		{
			u32 a_pix = *src_a++ & 0x00ffffff;
			u32 b_pix = *src_b++ & 0x00ffffff;
			if (a_pix)
			{
				*dst = a_pix | 0xff000000;
			}
			else if (b_pix)
			{
				*dst = b_pix | 0xff000000;
			}
			dst++;
		}
	}

	m_ttl_tilemap->draw(screen, bitmap, cliprect, 0, 1<<0);

	return 0;
}


//-------------------------------------------------
//  68k memory map
//-------------------------------------------------

void polygonet_state::main_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	map(0x200000, 0x21ffff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0x400000, 0x40001f).rw(m_k053936, FUNC(k053936_device::ctrl_r), FUNC(k053936_device::ctrl_w));
	map(0x440000, 0x440fff).ram().share(m_roz_vram).w(FUNC(polygonet_state::roz_vram_w));
	map(0x480000, 0x480003).r(FUNC(polygonet_state::inputs_r));
	map(0x4c0000, 0x4c0003).w(FUNC(polygonet_state::sys_w));
	map(0x500000, 0x503fff).ram().rw(FUNC(polygonet_state::shared_ram_read), FUNC(polygonet_state::shared_ram_write));
	map(0x504000, 0x504003).w(FUNC(polygonet_state::dsp_w_lines));
	map(0x506000, 0x50600f).rw(FUNC(polygonet_state::dsp_host_interface_r), FUNC(polygonet_state::dsp_host_interface_w));
	map(0x540000, 0x540fff).ram().share(m_ttl_vram).w(FUNC(polygonet_state::ttl_vram_w));
	map(0x541000, 0x54101f).ram().share(m_fix_regs).rw(FUNC(polygonet_state::fix_regs_r), FUNC(polygonet_state::fix_regs_w));
	map(0x580000, 0x5807ff).rw(m_k056230, FUNC(k056230_device::ram_r), FUNC(k056230_device::ram_w));
	map(0x580800, 0x580803).m(m_k056230, FUNC(k056230_device::regs_map));
	map(0x600000, 0x60000f).m(m_k054321, FUNC(k054321_device::main_map));
	map(0x640000, 0x640003).w(FUNC(polygonet_state::sound_irq_w));
	map(0x680000, 0x680003).w(m_watchdog, FUNC(watchdog_timer_device::reset32_w));
	map(0x700000, 0x73ffff).rom().region("gfx2", 0);
	map(0x780000, 0x79ffff).rom().region("gfx1", 0);
	map(0xff8000, 0xffffff).ram();
}


//-------------------------------------------------
//  DSP memory map
//-------------------------------------------------

void polygonet_state::dsp_program_map(address_map &map)
{
	map(0x7000, 0x7fff).ram().share(m_dsp_common);
	map(0x8000, 0x87ff).ram();
	map(0xc000, 0xc000).r(FUNC(polygonet_state::dsp_bootload_r));
}

void polygonet_state::dsp_data_map(address_map &map)
{
	map(0x0000, 0xffff).view(m_dsp_data_view);

	if ((VERBOSE & LOG_DSP_AB0) != 0) m_dsp_data_view[0](0x0000, 0x5fff).rw(FUNC(polygonet_state::dsp_ram_ab_0_read), FUNC(polygonet_state::dsp_ram_ab_0_write)).share(m_dsp_ab_0);
	else                              m_dsp_data_view[0](0x0000, 0x5fff).ram().share(m_dsp_ab_0);
	if ((VERBOSE & LOG_DSP_A6) != 0)  m_dsp_data_view[0](0x6000, 0x6fff).rw(FUNC(polygonet_state::dsp_ram_a_6_read), FUNC(polygonet_state::dsp_ram_a_6_write)).share(m_dsp_a_6);
	else                              m_dsp_data_view[0](0x6000, 0x6fff).ram().share(m_dsp_a_6);
	if ((VERBOSE & LOG_DSP_A7) != 0)  m_dsp_data_view[0](0x7000, 0x7fff).rw(FUNC(polygonet_state::dsp_ram_a_7_read), FUNC(polygonet_state::dsp_ram_a_7_write)).share(m_dsp_a_7);
	else                              m_dsp_data_view[0](0x7000, 0x7fff).ram().w(FUNC(polygonet_state::dsp_ram_a_7_write)).share(m_dsp_a_7);
	if ((VERBOSE & LOG_DSP_A8) != 0)  m_dsp_data_view[0](0x8000, 0xbfff).rw(FUNC(polygonet_state::dsp_ram_a_8_read), FUNC(polygonet_state::dsp_ram_a_8_write));
	else                              m_dsp_data_view[0](0x8000, 0xbfff).bankrw(m_dsp_bank_a_8);
	if ((VERBOSE & LOG_DSP_AC) != 0)  m_dsp_data_view[0](0xc000, 0xdfff).rw(FUNC(polygonet_state::dsp_ram_a_c_read), FUNC(polygonet_state::dsp_ram_a_c_write)).share(m_dsp_share);
	else                              m_dsp_data_view[0](0xc000, 0xdfff).ram().share(m_dsp_share);
	if ((VERBOSE & LOG_DSP_AE) != 0)  m_dsp_data_view[0](0xe000, 0xffff).rw(FUNC(polygonet_state::dsp_ram_a_e_read), FUNC(polygonet_state::dsp_ram_a_e_write)).share(m_dsp_a_e);
	else                              m_dsp_data_view[0](0xe000, 0xffff).ram().w(FUNC(polygonet_state::dsp_ram_a_e_write)).share(m_dsp_a_e);

	if ((VERBOSE & LOG_DSP_AB0) != 0) m_dsp_data_view[1](0x0000, 0x5fff).rw(FUNC(polygonet_state::dsp_ram_ab_0_read), FUNC(polygonet_state::dsp_ram_ab_0_write)).share(m_dsp_ab_0);
	else                              m_dsp_data_view[1](0x0000, 0x5fff).ram().share(m_dsp_ab_0);
	if ((VERBOSE & LOG_DSP_B6) != 0)  m_dsp_data_view[1](0x6000, 0x6fff).rw(FUNC(polygonet_state::dsp_ram_b_6_read), FUNC(polygonet_state::dsp_ram_b_6_write));
	else                              m_dsp_data_view[1](0x6000, 0x6fff).bankrw(m_dsp_bank_b_6);
	if ((VERBOSE & LOG_DSP_B7) != 0)  m_dsp_data_view[1](0x7000, 0x7fff).rw(FUNC(polygonet_state::dsp_ram_b_7_read), FUNC(polygonet_state::dsp_ram_b_7_write));
	else                              m_dsp_data_view[1](0x7000, 0x7fff).bankrw(m_dsp_bank_b_7);
	if ((VERBOSE & LOG_DSP_B8) != 0)  m_dsp_data_view[1](0x8000, 0xffbf).rw(FUNC(polygonet_state::dsp_ram_b_8_read), FUNC(polygonet_state::dsp_ram_b_8_write));
	else                              m_dsp_data_view[1](0x8000, 0xffbf).bankrw(m_dsp_bank_b_8);
}


//-------------------------------------------------
//  Sound memory map + handlers
//-------------------------------------------------

void polygonet_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xbfff).bankr(m_sound_bank);
	map(0xc000, 0xdfff).ram();
	map(0xe000, 0xe22f).rw("k054539", FUNC(k054539_device::read), FUNC(k054539_device::write));
	map(0xe230, 0xe3ff).ram();
	map(0xe400, 0xe62f).nopr().nopw(); // Second 054539 (not present)
	map(0xe630, 0xe7ff).ram();
	map(0xf000, 0xf003).m(m_k054321, FUNC(k054321_device::sound_map));
	map(0xf800, 0xf800).w(FUNC(polygonet_state::sound_ctrl_w));
}

void polygonet_state::sound_ctrl_w(u8 data)
{
	// .... .xxx - Sound bank
	// ...x .... - NMI clear (clocked?) (or NMI enable mask?)

	if ((m_sound_ctrl & 7) != (data & 7))
		m_sound_bank->set_entry(data & 7);

	m_sound_ctrl = data;
	update_sound_nmi();
}

void polygonet_state::update_sound_nmi()
{
	if (m_sound_intck) // checking m_sound_ctrl & 0x10 seems logical based on other Konami games, but polynetw doesn't like it
		m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	else
		m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void polygonet_state::k054539_nmi_gen(int state)
{
	m_sound_intck = state;
	update_sound_nmi();
}

//-------------------------------------------------
//  Machine configuration
//-------------------------------------------------

void polygonet_state::plygonet(machine_config &config)
{
	M68EC020(config, m_maincpu, XTAL(32'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &polygonet_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(polygonet_state::vblank_interrupt));

	DSP56156(config, m_dsp, XTAL(40'000'000));
	m_dsp->set_addrmap(AS_PROGRAM, &polygonet_state::dsp_program_map);
	m_dsp->set_addrmap(AS_DATA, &polygonet_state::dsp_data_map);
	m_dsp->portc_cb().set(FUNC(polygonet_state::dsp_portc_write));

	Z80(config, m_audiocpu, 8000000);
	m_audiocpu->set_addrmap(AS_PROGRAM, &polygonet_state::sound_map);

	config.set_maximum_quantum(attotime::from_hz(6000)); // occasional lockup in-game otherwise

	EEPROM_ER5911_8BIT(config, m_eeprom);

	WATCHDOG_TIMER(config, m_watchdog);

	// Networking hardware
	K056230(config, m_k056230);
	m_k056230->irq_cb().set_inputline(m_maincpu, M68K_IRQ_3);

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(64, 64+384-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(polygonet_state::screen_update));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_plygonet);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_888, 32768);

	K053936(config, m_k053936, 0);
	m_k053936->set_wrap(true);

	// Sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	K054321(config, m_k054321, "lspeaker", "rspeaker");

	k054539_device &k054539(K054539(config, "k054539", XTAL(18'432'000)));
	k054539.timer_handler().set(FUNC(polygonet_state::k054539_nmi_gen));
	k054539.add_route(0, "lspeaker", 0.75);
	k054539.add_route(1, "rspeaker", 0.75);
}


//-------------------------------------------------
//  ROM definitions
//-------------------------------------------------

ROM_START( plygonet )
	ROM_REGION( 0x200000, "maincpu", 0) // Main program
	ROM_LOAD32_BYTE( "305uaa01.4k", 0x000003, 512*1024, CRC(8bdb6c95) SHA1(e981833842f8fd89b9726901fbe2058444204792) ) // Boards exist without the "UA" in the label IE: 305a01, etc.
	ROM_LOAD32_BYTE( "305uaa02.2k", 0x000002, 512*1024, CRC(4d7e32b3) SHA1(25731526535036972577637d186f02ae467296bd) )
	ROM_LOAD32_BYTE( "305uaa03.2h", 0x000001, 512*1024, CRC(36e4e3fe) SHA1(e8fcad4f196c9b225a0fbe70791493ff07c648a9) )
	ROM_LOAD32_BYTE( "305uaa04.4h", 0x000000, 512*1024, CRC(d8394e72) SHA1(eb6bcf8aedb9ba5843204ab8aacb735cbaafb74d) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Z80 sound program
	ROM_LOAD("305b05.7b", 0x000000, 0x20000, CRC(2d3d9654) SHA1(784a409df47cee877e507b8bbd3610d161d63753) )

	ROM_REGION32_BE( 0x20000, "gfx1", 0 ) // TTL text plane tiles
	ROMX_LOAD( "305b06.18g", 0x000000, 0x20000, CRC(decd6e42) SHA1(4c23dcb1d68132d3381007096e014ee4b6007086), ROM_GROUPDWORD | ROM_REVERSE )

	ROM_REGION32_BE( 0x40000, "gfx2", 0 ) // '936 tiles
	ROM_LOAD( "305b07.20d", 0x000000, 0x40000, CRC(e4320bc3) SHA1(b0bb2dac40d42f97da94516d4ebe29b1c3d77c37) )

	ROM_REGION( 0x200000, "k054539", 0 ) // Sound data
	ROM_LOAD( "305b08.2e", 0x000000, 0x200000, CRC(874607df) SHA1(763b44a80abfbc355bcb9be8bf44373254976019) )

	ROM_REGION( 0x80, "eeprom", ROMREGION_ERASEFF )
	ROM_LOAD( "plygonet.nv", 0x0000, 0x0080, CRC(627748ac) SHA1(ea1b06739fee235b049ff8daffff7d43cb093112) )
ROM_END

ROM_START( polynetw )
	ROM_REGION( 0x200000, "maincpu", 0) // Main program
	ROM_LOAD32_BYTE( "305jaa01.4k", 0x000003, 0x080000, CRC(ea889bd9) SHA1(102e7c0f0c064662c0f6137ad5da97a9ccd49a97) )
	ROM_LOAD32_BYTE( "305jaa02.2k", 0x000002, 0x080000, CRC(d0710379) SHA1(cf0970d63e8d021edf2d404838c658a5b7cb8fb8) )
	ROM_LOAD32_BYTE( "305jaa03.2h", 0x000001, 0x080000, CRC(278b5928) SHA1(2ea96054e2ef637731cd64f2bef0b5b2bbe7e24f) )
	ROM_LOAD32_BYTE( "305jaa04.4h", 0x000000, 0x080000, CRC(b069353b) SHA1(12fbe2df09328bb7193e89a49d84a61eab5bfdcb) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // Z80 sound program
	ROM_LOAD( "305jaa05.7b", 0x000000, 0x020000, CRC(06053db6) SHA1(c7d43c2650d949ee552a49db93dece842c17e68d) )

	ROM_REGION32_BE( 0x20000, "gfx1", 0 ) // TTL text plane tiles
	ROMX_LOAD( "305a06.18g", 0x000000, 0x020000, CRC(4b9b7e9c) SHA1(8c3c0f1ec7e26fd9552f6da1e6bdd7ff4453ba57), ROM_GROUPDWORD | ROM_REVERSE )

	ROM_REGION32_BE( 0x40000, "gfx2", 0 ) // '936 tiles
	ROM_LOAD( "305a07.20d", 0x000000, 0x020000, CRC(0959283b) SHA1(482caf96e8e430b87810508b1a1420cd3b58f203) )

	ROM_REGION( 0x400000, "k054539", 0 ) // Sound data
	ROM_LOAD( "305a08.2e", 0x000000, 0x200000, CRC(7ddb8a52) SHA1(3199b347fc433ffe0de8521001df77672d40771e) )
	ROM_LOAD( "305a09.3e", 0x200000, 0x200000, CRC(6da1be58) SHA1(d63ac16ac551193ff8a6036724fb59e1d702e06b) )

	ROM_REGION( 0x80, "eeprom", ROMREGION_ERASEFF )
	ROM_LOAD( "polynetw.nv", 0x0000, 0x0080, CRC(8f39d644) SHA1(8733e1a288ba20c4b04b3aedde52801d05cebdf9) )
ROM_END

} // Anonymous namespace


//-------------------------------------------------
//  System definitions
//-------------------------------------------------

//    YEAR  NAME      PARENT   MACHINE   INPUT      STATE            INIT
GAME( 1993, plygonet, 0,       plygonet, polygonet, polygonet_state, empty_init, ROT90, "Konami", "Polygonet Commanders (ver UAA)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN )
GAME( 1993, polynetw, 0,       plygonet, polynetw,  polygonet_state, empty_init, ROT90, "Konami", "Poly-Net Warriors (ver JAA)",    MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN )
