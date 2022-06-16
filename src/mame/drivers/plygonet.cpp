// license:BSD-3-Clause
// copyright-holders:R. Belmont, Andrew Gardner, Ryan Holtz
/*
    Polygonet Commanders (Konami, 1993)
    Poly-Net Warriors (Konami, 1993)

    Preliminary driver by R. Belmont
    Additional work by Andrew Gardner

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
    - Polygon rasterization (K054009 + K054010)
    - Hook up PSAC2 (gfx decode for it is already present and correct)
    - Priorities.  From the original board it appears they're fixed, in front to back order:
      (all the way in front) TTL text layer -> polygons -> PSAC2 (all the way in back)

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

*/

#include "emu.h"

#include "cpu/dsp56156/dsp56156.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/eepromser.h"
#include "machine/k054321.h"
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
#define LOG_68K_SHARED      (1U << 10)
#define LOG_DSP_HOST_INTF   (1U << 11)
#define LOG_DSP_CTRL        (1U << 12)
#define LOG_DSP_PORTC       (1U << 13)

#define LOG_ALL_DSP_A       (LOG_DSP_AB0 | LOG_DSP_A7 | LOG_DSP_A6 | LOG_DSP_A8 | LOG_DSP_AC | LOG_DSP_AE)
#define LOG_ALL_DSP_B       (LOG_DSP_AB0 | LOG_DSP_B6 | LOG_DSP_B7 | LOG_DSP_B8)

//#define VERBOSE (LOG_ALL_DSP_A | LOG_ALL_DSP_B | LOG_68K_SHARED | LOG_DSP_HOST_INTF | LOG_DSP_CTRL | LOG_DSP_PORTC)
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
		m_eeprom(*this, "eeprom"),
		m_k053936(*this, "k053936"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_ttl_vram(*this, "ttl_vram"),
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
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

	void main_map(address_map &map);
	void sound_map(address_map &map);
	void dsp_program_map(address_map &map);
	void dsp_data_map(address_map &map);

	// Main-board handlers
	void sys_w(offs_t offset, uint8_t data);
	uint8_t inputs_r(offs_t offset);
	void sound_irq_w(uint32_t data);
	uint32_t dsp_host_interface_r(offs_t offset, uint32_t mem_mask = ~0);
	void dsp_host_interface_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t shared_ram_read(offs_t offset, uint32_t mem_mask = ~0);
	void shared_ram_write(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void dsp_w_lines(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t network_r();

	// DSP handlers
	uint16_t dsp_bootload_r();
	void dsp_portc_write(uint16_t data);
	uint16_t dsp_ram_ab_0_read(offs_t offset);
	void dsp_ram_ab_0_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dsp_ram_a_6_read(offs_t offset);
	void dsp_ram_a_6_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dsp_ram_b_6_read(offs_t offset);
	void dsp_ram_b_6_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dsp_ram_a_7_read(offs_t offset);
	void dsp_ram_a_7_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dsp_ram_b_7_read(offs_t offset);
	void dsp_ram_b_7_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dsp_ram_a_8_read(offs_t offset);
	void dsp_ram_a_8_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dsp_ram_b_8_read(offs_t offset);
	void dsp_ram_b_8_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dsp_ram_a_c_read(offs_t offset);
	void dsp_ram_a_c_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dsp_ram_a_e_read(offs_t offset);
	void dsp_ram_a_e_write(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// Video handlers
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_interrupt);
	DECLARE_WRITE_LINE_MEMBER(k054539_nmi_gen);
	void ttl_vram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void roz_vram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	TILE_GET_INFO_MEMBER(ttl_get_tile_info);
	TILE_GET_INFO_MEMBER(roz_get_tile_info);

	// Sound handlers
	void sound_ctrl_w(uint8_t data);

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<dsp56156_device> m_dsp;
	required_device<eeprom_serial_er5911_device> m_eeprom;
	required_device<k053936_device> m_k053936;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint32_t> m_ttl_vram;
	required_shared_ptr<uint32_t> m_roz_vram;
	required_device<k054321_device> m_k054321;
	required_memory_bank m_sound_bank;

	memory_bank_creator m_dsp_bank_a_8;
	memory_bank_creator m_dsp_bank_b_6;
	memory_bank_creator m_dsp_bank_b_7;
	memory_bank_creator m_dsp_bank_b_8;
	required_shared_ptr<uint16_t> m_dsp_common;
	required_shared_ptr<uint16_t> m_dsp_share;
	required_shared_ptr<uint16_t> m_dsp_ab_0;
	required_shared_ptr<uint16_t> m_dsp_a_6;
	required_shared_ptr<uint16_t> m_dsp_a_7;
	required_shared_ptr<uint16_t> m_dsp_a_e;
	memory_share_creator<uint16_t> m_dsp_ram_a_8;
	memory_share_creator<uint16_t> m_dsp_ram_b_6;
	memory_share_creator<uint16_t> m_dsp_ram_b_7;
	memory_share_creator<uint16_t> m_dsp_ram_b_8;
	memory_view m_dsp_data_view;

	required_ioport_array<4> m_inputs;
	required_ioport m_eepromout;

	// Main-board members
	uint8_t m_sys1;

	// DSP members
	uint16_t m_dsp_portc;

	// Video members
	int m_ttl_gfx_index;
	tilemap_t *m_ttl_tilemap;
	tilemap_t *m_roz_tilemap;

	// Sound members
	uint8_t m_sound_ctrl;
	uint8_t m_sound_intck;
};

//-------------------------------------------------
//  Machine-related helpers
//-------------------------------------------------

void polygonet_state::machine_start()
{
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
}

void polygonet_state::machine_reset()
{
	m_sound_bank->set_entry(0);

	m_sys1 = 0;
	m_sound_intck = 0;
	m_sound_ctrl = 0;

	m_dsp_portc = 0;

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

uint8_t polygonet_state::inputs_r(offs_t offset)
{
	return m_inputs[offset]->read();
}

void polygonet_state::sys_w(offs_t offset, uint8_t data)
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
			if (~data & 0x20)
				m_maincpu->set_input_line(M68K_IRQ_5, CLEAR_LINE);

			m_sys1 = data;
			break;

		default:
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

void polygonet_state::sound_irq_w(uint32_t data)
{
	// Auto-acknowledge interrupt
	m_audiocpu->set_input_line(0, HOLD_LINE);
}


//-------------------------------------------------
//  68k <-> DSP comms
//-------------------------------------------------

uint32_t polygonet_state::dsp_host_interface_r(offs_t offset, uint32_t mem_mask)
{
	offs_t hi_addr = 0;
	uint32_t value = 0;
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

void polygonet_state::dsp_host_interface_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	offs_t hi_addr = 0;
	uint8_t hi_data = 0;
	if (ACCESSING_BITS_8_15) // Low byte
	{
		hi_addr = (offset << 1) + 1;
		hi_data = (uint8_t)(data >> 8);
	}
	else if (ACCESSING_BITS_24_31) // High byte
	{
		hi_addr = offset << 1;
		hi_data = (uint8_t)(data >> 24);
	}

	LOGMASKED(LOG_DSP_HOST_INTF, "%s: 68k Writing to DSP Host Interface address %04x (68k addr %08x) = %08x & %08x\n", machine().describe_context(),
		hi_addr, offset, data, mem_mask);

	m_dsp->host_interface_write(hi_addr, hi_data);
}

uint32_t polygonet_state::shared_ram_read(offs_t offset, uint32_t mem_mask)
{
	const uint32_t data = (m_dsp_share[offset << 1] << 16) | m_dsp_share[(offset << 1) + 1];
	LOGMASKED(LOG_68K_SHARED, "%s: 68k Reading from shared DSP RAM[%04x]: %08x & %08x \n", machine().describe_context(), 0xc000 + (offset << 1), data, mem_mask);
	return data;
}

void polygonet_state::shared_ram_write(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	// Write to the current DSP word
	if (ACCESSING_BITS_16_31)
	{
		m_dsp_share[offset << 1] = (uint16_t)(data >> 16);
		LOGMASKED(LOG_68K_SHARED, "%s: 68k Writing to shared DSP RAM[%04x] = %04x\n", machine().describe_context(), 0xc000 + (offset << 1), (uint16_t)(data >> 16));
	}

	// Write to the next DSP word
	if (ACCESSING_BITS_0_15)
	{
		m_dsp_share[(offset << 1) + 1] = (uint16_t)data;
		LOGMASKED(LOG_68K_SHARED, "%s: 68k Writing to shared DSP RAM[%04x] = %04x\n", machine().describe_context(), 0xc000 + (offset << 1) + 1, (uint16_t)data);
	}
}

void polygonet_state::dsp_w_lines(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	LOGMASKED(LOG_DSP_CTRL, "%s: 68k writing to DSP control lines: %08x & %08x\n", machine().describe_context(), data, mem_mask);

	// 0x01000000 is the reset line, active-low
	m_dsp->set_input_line(DSP56156_IRQ_RESET, BIT(data, 24) ? CLEAR_LINE : ASSERT_LINE);

	// 0x04000000 is the COMBNK line - it switches who has access to the shared RAM - the dsp or the 68020
}

uint32_t polygonet_state::network_r()
{
	return 0x08000000;
}


//-------------------------------------------------
//  DSP I/O management
//-------------------------------------------------

// It's believed this is hard-wired to return (at least) bit 15 as 0 - causes a host interface bootup
uint16_t polygonet_state::dsp_bootload_r()
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

void polygonet_state::dsp_portc_write(uint16_t data)
{
	LOGMASKED(LOG_DSP_PORTC, "%s: DSP Port C write: %04x\n", machine().describe_context(), data);
	m_dsp_portc = data;

	const uint8_t bank_a_8_num = bitswap<3>(m_dsp_portc, 4, 3, 2);
	m_dsp_bank_a_8->set_entry(bank_a_8_num);

	const uint8_t bank_b_67_num = bitswap<2>(m_dsp_portc, 7, 8);
	m_dsp_bank_b_6->set_entry(bank_b_67_num);
	m_dsp_bank_b_7->set_entry(bank_b_67_num);

	const uint8_t bank_b_8_num = bitswap<3>(m_dsp_portc, 7, 8, 0);
	m_dsp_bank_b_8->set_entry(bank_b_8_num);

	m_dsp_data_view.select(BIT(m_dsp_portc, 1));
}


//-------------------------------------------------
//  DSP RAM handlers
//-------------------------------------------------

uint16_t polygonet_state::dsp_ram_ab_0_read(offs_t offset)
{
	const uint16_t data = m_dsp_ab_0[offset];
	LOGMASKED(LOG_DSP_AB0, "%s: DSP Reading from Mapping A/B, 0xxx RAM[%04x]: %04x\n", machine().describe_context(), offset, data);
	return data;
}

void polygonet_state::dsp_ram_ab_0_write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_DSP_AB0, "%s: DSP Writing to Mapping A/B, 0xxx RAM[%04x] = %04x\n", machine().describe_context(), offset, data);
	COMBINE_DATA(&m_dsp_ab_0[offset]);
}


uint16_t polygonet_state::dsp_ram_a_6_read(offs_t offset)
{
	const uint16_t data = m_dsp_a_6[offset];
	LOGMASKED(LOG_DSP_A6, "%s: DSP Reading from Mapping A, 6xxx RAM[%04x]: %04x\n", machine().describe_context(), 0x6000 + offset, data);
	return data;
}

void polygonet_state::dsp_ram_a_6_write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_DSP_A6, "%s: DSP Writing to Mapping A, 6xxx RAM[%04x] = %04x (bank offset %05x)\n", machine().describe_context(), 0x6000 + offset, data);
	COMBINE_DATA(&m_dsp_a_6[offset]);
}


uint16_t polygonet_state::dsp_ram_b_6_read(offs_t offset)
{
	const offs_t bank_b_6_offset = bitswap<2>(m_dsp_portc, 7, 8) * 0x1000;
	const uint16_t data = ((uint16_t *)m_dsp_bank_b_6->base())[offset];
	LOGMASKED(LOG_DSP_B6, "%s: DSP Reading from Mapping B, 6xxx RAM[%04x]: %04x (bank offset %05x)\n", machine().describe_context(), 0x6000 + offset, data, bank_b_6_offset + offset);
	return data;
}

void polygonet_state::dsp_ram_b_6_write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	const offs_t bank_b_6_offset = bitswap<2>(m_dsp_portc, 7, 8) * 0x1000;
	LOGMASKED(LOG_DSP_B6, "%s: DSP Writing to Mapping B, 6xxx RAM[%04x] = %04x (bank offset %05x)\n", machine().describe_context(), 0x6000 + offset, data, bank_b_6_offset + offset);
	COMBINE_DATA(((uint16_t *)m_dsp_bank_b_6->base()) + offset);
}


uint16_t polygonet_state::dsp_ram_a_7_read(offs_t offset)
{
	const uint16_t data = m_dsp_a_7[offset];
	LOGMASKED(LOG_DSP_A7, "%s: DSP Reading from Mapping A, 7xxx RAM[%04x]: %04x\n", machine().describe_context(), 0x7000 + offset, data);
	return data;
}

void polygonet_state::dsp_ram_a_7_write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_DSP_A7, "%s: DSP Writing to Mapping A, 7xxx RAM[%04x] = %04x\n", machine().describe_context(), 0x7000 + offset, data);
	COMBINE_DATA(&m_dsp_a_7[offset]);
	COMBINE_DATA(&m_dsp_common[offset]);
}


uint16_t polygonet_state::dsp_ram_b_7_read(offs_t offset)
{
	const offs_t bank_b_7_offset = bitswap<2>(m_dsp_portc, 7, 8) * 0x1000;
	const uint16_t data = ((uint16_t *)m_dsp_bank_b_7->base())[offset];
	LOGMASKED(LOG_DSP_B7, "%s: DSP Reading from Mapping B, 7xxx RAM[%04x]: %04x (bank offset %05x)\n", machine().describe_context(), 0x7000 + offset, data, bank_b_7_offset + offset);
	return data;
}

void polygonet_state::dsp_ram_b_7_write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	const offs_t bank_b_7_offset = bitswap<2>(m_dsp_portc, 7, 8) * 0x1000;
	LOGMASKED(LOG_DSP_B7, "%s: DSP Writing to Mapping B, 7xxx RAM[%04x] = %04x (bank offset %05x)\n", machine().describe_context(), 0x7000 + offset, data, bank_b_7_offset + offset);
	COMBINE_DATA(((uint16_t *)m_dsp_bank_b_7->base()) + offset);
}


uint16_t polygonet_state::dsp_ram_a_8_read(offs_t offset)
{
	const offs_t bank_a_8_offset = bitswap<3>(m_dsp_portc, 4, 3, 2) * 0x4000;
	const uint16_t data = ((uint16_t *)m_dsp_bank_a_8->base())[offset];
	LOGMASKED(LOG_DSP_A8, "%s: DSP Reading from Mapping A, 8xxx RAM[%04x]: %04x (bank offset %05x)\n", machine().describe_context(), 0x8000 + offset, data, bank_a_8_offset + offset);
	return data;
}

void polygonet_state::dsp_ram_a_8_write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	const offs_t bank_a_8_offset = bitswap<3>(m_dsp_portc, 4, 3, 2) * 0x4000;
	LOGMASKED(LOG_DSP_A8, "%s: DSP Writing to Mapping A, 8xxx RAM[%04x] = %04x (bank offset %05x)\n", machine().describe_context(), 0x8000 + offset, data, bank_a_8_offset + offset);
	COMBINE_DATA(((uint16_t *)m_dsp_bank_a_8->base()) + offset);
}


uint16_t polygonet_state::dsp_ram_b_8_read(offs_t offset)
{
	const offs_t bank_b_8_offset = bitswap<3>(m_dsp_portc, 7, 8, 0) * 0x8000;
	const uint16_t data = ((uint16_t *)m_dsp_bank_b_8->base())[offset];
	LOGMASKED(LOG_DSP_B8, "%s: DSP Reading from Mapping B, 8xxx RAM[%04x]: %04x (bank offset %05x)\n", machine().describe_context(), 0x8000 + offset, data, bank_b_8_offset + offset);
	return data;
}

void polygonet_state::dsp_ram_b_8_write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	const offs_t bank_b_8_offset = bitswap<3>(m_dsp_portc, 4, 3, 2) * 0x4000;
	LOGMASKED(LOG_DSP_B8, "%s: DSP Writing to Mapping B, 8xxx RAM[%04x] = %04x (bank offset %05x)\n", machine().describe_context(), 0x8000 + offset, data, bank_b_8_offset + offset);
	COMBINE_DATA(((uint16_t *)m_dsp_bank_b_8->base()) + offset);
}


uint16_t polygonet_state::dsp_ram_a_c_read(offs_t offset)
{
	const uint16_t data = m_dsp_share[offset];
	LOGMASKED(LOG_DSP_AC, "%s: DSP Reading from Mapping A, Cxxx RAM[%04x]: %04x\n", machine().describe_context(), 0xc000 + offset, data);
	return data;
}

void polygonet_state::dsp_ram_a_c_write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_DSP_AC, "%s: DSP Writing to Mapping A, Cxxx RAM[%04x] = %04x\n", machine().describe_context(), 0xc000 + offset, data);
	COMBINE_DATA(&m_dsp_share[offset]);
}


uint16_t polygonet_state::dsp_ram_a_e_read(offs_t offset)
{
	const uint16_t data = m_dsp_a_e[offset];
	LOGMASKED(LOG_DSP_AE, "%s: DSP Reading from Mapping A, Exxx RAM[%04x]: %04x\n", machine().describe_context(), 0xe000 + offset, data);
	return data;
}

void polygonet_state::dsp_ram_a_e_write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	LOGMASKED(LOG_DSP_AE, "%s: DSP Writing to Mapping A, Exxx RAM[%04x] = %04x\n", machine().describe_context(), 0xe000 + offset, data);
	COMBINE_DATA(&m_dsp_a_e[offset]);
}


//-------------------------------------------------
//  Video hardware
//-------------------------------------------------

static const gfx_layout bglayout =
{
	16,16,
	1024,
	4,
	{ 0, 1, 2, 3 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
		8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4, 8*4,
		9*4, 10*4, 11*4, 12*4, 13*4, 14*4, 15*4 },

	128*8
};

static GFXDECODE_START( gfx_plygonet )
	GFXDECODE_ENTRY( "gfx2", 0, bglayout, 0x0000, 64 )
GFXDECODE_END

TILE_GET_INFO_MEMBER(polygonet_state::ttl_get_tile_info)
{
	const auto ttl_vram = util::big_endian_cast<const uint16_t>(m_ttl_vram.target());
	const int code = ttl_vram[tile_index] & 0xfff;
	const int attr = ttl_vram[tile_index] >> 12;  // Is the palette in all 4 bits?

	tileinfo.set(m_ttl_gfx_index, code, attr, 0);
}

TILE_GET_INFO_MEMBER(polygonet_state::roz_get_tile_info)
{
	const auto roz_vram = util::big_endian_cast<const uint16_t>(m_roz_vram.target());
	const int code = roz_vram[tile_index] & 0x3ff;
	const int attr = (roz_vram[tile_index] >> 12) + 16; // ROZ base palette is palette index 16 onward

	tileinfo.set(0, code, attr, 0);
}

void polygonet_state::ttl_vram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
{
	COMBINE_DATA(&m_ttl_vram[offset]);
	m_ttl_tilemap->mark_tile_dirty(offset << 1);
	m_ttl_tilemap->mark_tile_dirty((offset << 1) + 1);
}

void polygonet_state::roz_vram_w(offs_t offset, uint32_t data, uint32_t mem_mask)
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
	m_roz_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(polygonet_state::roz_get_tile_info)), TILEMAP_SCAN_COLS, 16, 16, 32, 64);
	m_roz_tilemap->set_transparent_pen(0);

	// Register save states
	save_item(NAME(m_ttl_gfx_index));
}

uint32_t polygonet_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen.priority().fill(0);
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_k053936->zoom_draw(screen, bitmap, cliprect, m_roz_tilemap, 0, 0, 0);

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
	map(0x541000, 0x54101f).ram();
	map(0x580000, 0x5807ff).ram();
	map(0x580800, 0x580803).r(FUNC(polygonet_state::network_r)).nopw(); // Network RAM and registers?
	map(0x600000, 0x60000f).m(m_k054321, FUNC(k054321_device::main_map));
	map(0x640000, 0x640003).w(FUNC(polygonet_state::sound_irq_w));
	map(0x680000, 0x680003).w("watchdog", FUNC(watchdog_timer_device::reset32_w));
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
	map(0x0800, 0xffff).view(m_dsp_data_view);

	m_dsp_data_view[0](0x0000, 0x5fff).rw(FUNC(polygonet_state::dsp_ram_ab_0_read), FUNC(polygonet_state::dsp_ram_ab_0_write)).share(m_dsp_ab_0);
	m_dsp_data_view[0](0x6000, 0x6fff).rw(FUNC(polygonet_state::dsp_ram_a_6_read), FUNC(polygonet_state::dsp_ram_a_6_write)).share(m_dsp_a_6);
	m_dsp_data_view[0](0x7000, 0x7fff).rw(FUNC(polygonet_state::dsp_ram_a_7_read), FUNC(polygonet_state::dsp_ram_a_7_write)).share(m_dsp_a_7);
	m_dsp_data_view[0](0x8000, 0xbfff).rw(FUNC(polygonet_state::dsp_ram_a_8_read), FUNC(polygonet_state::dsp_ram_a_8_write));
	m_dsp_data_view[0](0xc000, 0xdfff).rw(FUNC(polygonet_state::dsp_ram_a_c_read), FUNC(polygonet_state::dsp_ram_a_c_write)).share(m_dsp_share);
	m_dsp_data_view[0](0xe000, 0xffff).rw(FUNC(polygonet_state::dsp_ram_a_e_read), FUNC(polygonet_state::dsp_ram_a_e_write)).share(m_dsp_a_e);

	m_dsp_data_view[1](0x0000, 0x5fff).rw(FUNC(polygonet_state::dsp_ram_ab_0_read), FUNC(polygonet_state::dsp_ram_ab_0_write)).share(m_dsp_ab_0);
	m_dsp_data_view[1](0x6000, 0x6fff).rw(FUNC(polygonet_state::dsp_ram_b_6_read), FUNC(polygonet_state::dsp_ram_b_6_write));
	m_dsp_data_view[1](0x7000, 0x7fff).rw(FUNC(polygonet_state::dsp_ram_b_7_read), FUNC(polygonet_state::dsp_ram_b_7_write));
	m_dsp_data_view[1](0x8000, 0xffbf).rw(FUNC(polygonet_state::dsp_ram_b_8_read), FUNC(polygonet_state::dsp_ram_b_8_write));
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

void polygonet_state::sound_ctrl_w(uint8_t data)
{
	// .... .xxx - Sound bank
	// ...x .... - NMI clear (clocked?)

	if ((m_sound_ctrl & 7) != (data & 7))
		m_sound_bank->set_entry(data & 7);

	// This behaves differently to the other games of this era
	if (!(m_sound_ctrl & 0x10) && (data & 0x10))
		m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	m_sound_ctrl = data;
}

WRITE_LINE_MEMBER(polygonet_state::k054539_nmi_gen)
{
	// Trigger interrupt on rising clock edge
	if (!m_sound_intck && state)
		m_audiocpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);

	m_sound_intck = state;
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

	config.set_perfect_quantum(m_maincpu); // TODO: TEMPORARY!  UNTIL A MORE LOCALIZED SYNC CAN BE MADE

	EEPROM_ER5911_8BIT(config, m_eeprom);

	WATCHDOG_TIMER(config, "watchdog");

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_plygonet);

	// Video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(64, 64+368-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(polygonet_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_888, 32768);

	K053936(config, m_k053936, 0);

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
	ROMX_LOAD( "305b07.20d", 0x000000, 0x40000, CRC(e4320bc3) SHA1(b0bb2dac40d42f97da94516d4ebe29b1c3d77c37), ROM_GROUPDWORD | ROM_REVERSE )

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
	ROMX_LOAD( "305a07.20d", 0x000000, 0x020000, CRC(0959283b) SHA1(482caf96e8e430b87810508b1a1420cd3b58f203), ROM_GROUPDWORD | ROM_REVERSE )

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
GAME( 1993, plygonet, 0,       plygonet, polygonet, polygonet_state, empty_init, ROT90, "Konami", "Polygonet Commanders (ver UAA)", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN )
GAME( 1993, polynetw, 0,       plygonet, polynetw,  polygonet_state, empty_init, ROT90, "Konami", "Poly-Net Warriors (ver JAA)",    MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE | MACHINE_NODEVICE_LAN )
