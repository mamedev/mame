// license:BSD-3-Clause
// copyright-holders:Roberto Fresca, Grull Osgo
/********************************************************************************

  Jack House.
  Chain Leisure Co., Ltd (1994)

  Driver by Roberto Fresca & Grull Osgo.

  This driver is based on an absolutely unknown hardware.
  Just developed the emulation reverse-engineering the game code and graphics ROMs.

  The game has an unknown video processor as blitter, that handles 28 registers,
  4 graphics modes, 2 layers (foreground and background), 6 video RAM segments
  (3 by layer, for tile offset, extended and attrib), and more complex things...

  Clocks and other stuff are educated guessings.


*********************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "machine/ram.h"
#include "machine/ticket.h"
#include "sound/ay8910.h"
#include "video/ramdac.h"
#include "screen.h"
#include "tilemap.h"
#include "speaker.h"

#include "jackhouse.lh"

namespace
{

class jackhouse_state : public driver_device
{
public:
	jackhouse_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_videoram(*this, "new_videoram%u", 0U, 0x0800U, ENDIANNESS_LITTLE),
		m_bgrom(*this, "bg_tiles"),
		m_gfx2rom(*this, "gfx2"),
		m_palette(*this, "palette"),
		m_hopper(*this, "hopper"),
		m_lamps(*this, "lamp%u", 0U)

	{ }

	void jackhouse(machine_config &config);

protected:
	virtual void machine_start() override { m_lamps.resolve(); };
	virtual void video_start() override;

private:

	void io_map(address_map &map);
	void program_map(address_map &map);
	void ramdac_map(address_map &map);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	template<u8 N> TILE_GET_INFO_MEMBER(get_tile_info);

	void videoram_w(u16 dst, u16 tile_code, u8 tile_color, u8 sel);

	// Video processor related stuff
	void video_proc_w(offs_t offs, u8 data);
	void vp_exec(u8 data);
	void vp_playfield(u8 data);

	// from IO ports
	void ram_sel(u8 data);
	void butt_lamps_w(u8 data);
	void outports_w(u8 data);

	// Devices
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	memory_share_array_creator<u8, 6> m_videoram;

	required_memory_region m_bgrom;
	required_memory_region m_gfx2rom;
	required_device<palette_device> m_palette;
	required_device<hopper_device> m_hopper;
	output_finder<8> m_lamps;

	tilemap_t *m_fg_tilemap = nullptr;
	tilemap_t *m_bg_tilemap = nullptr;

	offs_t vr_addr[6] = {0, 0, 0, 0, 0, 0};
	offs_t vp_vram_col[6] = {0, 0, 0, 0, 0, 0};
	offs_t vp_vram_row[6] = {0, 0, 0, 0, 0, 0};
	u8 vp_buff_addr = 0;
	u8 vp_buff[16];
	u8 vp_cmd_mode = 0;
	u8 vp_gfxA_src_low = 0;
	u8 vp_gfxA_src_high = 0;
	u8 vp_gfxB_src_low = 0;
	u8 vp_gfxB_src_high = 0;
	u8 vp_vram_dest_row = 0;
	u8 vp_vram_dest_col = 0;
	u8 vp_color_dst = 0;
	u8 vp_col_end = 0;
	u8 vp_row_end = 0;
	u8 m_ram_selector;
	u8 m_cont = 0;
};

#define MASTER_CLOCK    XTAL(6'000'000)


/***************************************
*        Memory map information        *
***************************************/

void jackhouse_state::program_map(address_map &map)
{
	map(0x0000, 0x9fff).rom();
	map(0xf800, 0xffff).ram().share("nvram");
}

void jackhouse_state::io_map(address_map &map)
{
	map.global_mask(0xff);

	map(0x0c, 0x0c).portr("IN3");  // unknown
	map(0x00, 0x1b).w(FUNC(jackhouse_state::video_proc_w));

	map(0x81, 0x81).r("aysnd", FUNC(ay8910_device::data_r));
	map(0x82, 0x83).w("aysnd", FUNC(ay8910_device::data_address_w));

	map(0x84, 0x87).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x88, 0x8b).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));

	map(0x90, 0x90).w(FUNC(jackhouse_state::butt_lamps_w));
	map(0x94, 0x94).w(FUNC(jackhouse_state::outports_w));

	map(0x98, 0x98).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x99, 0x99).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x9a, 0x9a).w("ramdac", FUNC(ramdac_device::mask_w));

	map(0x9c, 0x9c).w(FUNC(jackhouse_state::ram_sel));

}

void jackhouse_state::ramdac_map(address_map &map)
{
	map(0x000, 0x2ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

void jackhouse_state::ram_sel(u8 data)
{
	m_ram_selector = data;
	if (BIT(data, 0))
		m_bg_tilemap->enable(false);
	else
		m_bg_tilemap->enable(true);

	//logerror("Set Bit Selector: %02x\n", data);
}

void jackhouse_state::videoram_w(u16 dst, u16 tile_code, u8 tile_color, u8 sel)
{
	m_videoram[sel][dst] = tile_code & 0xff;
	m_videoram[sel + 1][dst] = (tile_code >> 8) & 0xff;
	m_videoram[sel + 2][dst] = tile_color;

	if (sel == 0)
		m_fg_tilemap->mark_tile_dirty(dst);
	else
		m_bg_tilemap->mark_tile_dirty(dst);
}

void jackhouse_state::vp_playfield(u8 data)
{
	u8 *ROM = m_bgrom->base();
	u16 tile_index = (((data & 0x03) - 1) * 28 * 64) * 2;
	for (u8 i = 2; i < 2 + 0x1c; i++)
	{
		for (u8 j = 0; j < 0x40; j++)
		{
			u16 dst = (i * 0x40 + j) & 0x7ff;
			u16 tile_code = (ROM[tile_index] | ( ROM[tile_index + 1] << 8));
			//logerror("vp: BG: tile_index:%04x - tile_code:%04x\n", tile_index, tile_code);
			tile_index += 2;
			videoram_w(dst, tile_code, data, 3);
		}
	}
}

void jackhouse_state::vp_exec(u8 data)
{
	switch( vp_cmd_mode )
	{
		// mode bit 0 - gfx data from vp_gfxB_src (ports 13,14)
		// mode bit 1 - gfx data from vp_gfxB_src (ports 13,14)
		case 0x01:
		case 0x02:
		{
			u16 tile_code = vp_gfxB_src_low | (vp_gfxB_src_high << 8);
			for (u8 i = vp_vram_dest_row; i < vp_vram_dest_row + vp_row_end + 1; i++)
			{
				for (u8 j = vp_vram_dest_col; j < vp_vram_dest_col + vp_col_end + 1; j++)
				{
					u16 dst = (i * 0x40 + j) & 0x7ff;
					videoram_w(dst, tile_code++, vp_color_dst, 0);
				}
			}
			break;
		}
		// mode bit 2 - gfx data from vp tables(ports 0f,10)
		case 0x04:
		{
			u8 *ROM = m_gfx2rom->base();
			u32 tile_index = ( vp_gfxA_src_low | (vp_gfxA_src_high << 8)) * 4;
			for (u8 i = vp_vram_dest_row; i < vp_vram_dest_row + vp_row_end + 1; i++)
			{
				for (u8 j = vp_vram_dest_col; j < vp_vram_dest_col + vp_col_end + 1; j++)
				{
					u16 dst = (i * 0x40 + j) & 0x7ff;
					u16 tile_code = ROM[tile_index] | (ROM[tile_index+1] << 8);
					//logerror("vp: FG:tile_index:%04x - tile_code:%04x\n", tile_index, tile_code);
					tile_index += 4;
					videoram_w(dst, tile_code, vp_color_dst, 0);
				}
			}
			break;
		}
		// mode bit 3 - Unknown
		case 0x08:
			break;
		default:
			logerror("vp: Illegal mode:%02x dst\n", vp_cmd_mode);
	}
}

void jackhouse_state::video_proc_w(offs_t offs, u8 data)
{
	// logerror("video_proc_w :offs:%04x data:%02x\n", offs, data);
	switch ( offs )
	{
		case 0x00:  // vram address register low (w/ autoincrement)
		{
			for (u8 i = 0; i < 6; i++)
				vp_vram_col[i] = data;
			break;
		}
		case 0x01:  // vram address register high (w/ autoincrement)
		{
			for (u8 i = 0; i < 6; i++)
			{
				vp_vram_row[i] = data;
				vr_addr[i]= 0x40 * vp_vram_row[i] + vp_vram_col[i];
				// logerror("%s: Acceso VideoRam: Set addr: Address = %04x\n", machine().describe_context(), vr_addr[0]);
			}
			break;
		}
		case 0x02:
		case 0x03:
		case 0x04:
		case 0x05:
		case 0x06:
		case 0x07:
		{
			// video ram write: main cpu sends data direct to video ram
			m_videoram[offs - 2][vr_addr[offs - 2]] = data;
			m_fg_tilemap->mark_tile_dirty(vr_addr[offs - 2]);
			// logerror("%s: VideoRam (%02x) Address:%02x - Write: Data = %04x\n", machine().describe_context(), offs - 2, vr_addr[offs - 2], m_videoram[offs - 2][vr_addr[offs - 2]] );
			vr_addr[offs - 2] += 1;  // autoincrement
			break;
		}
		case 0x08:
		{
			// logerror("%s:vgp: Offs:0x08: Unknown data:%02x - cont:%02x\n", machine().describe_context(), data, m_cont);
			m_cont++;
			if (m_cont == 0x3f)
				vp_playfield(vp_color_dst);
			break;
		}
		case 0x0f:
		{
			vp_gfxA_src_low = data;
			// logerror("vgp: Offs:0x0f: Set GFX's source address low:   data:%02x\n", vp_gfxA_src_low);
			break;
		}
		case 0x10:
		{
			vp_gfxA_src_high = data;
			// logerror("vgp: Offs:0x10: Set GFX's source address high:  data:%02x\n", vp_gfxA_src_high);
			break;
		}
		case 0x11:
		{
			// logerror("vgp: Offs:0x11: Extra data for Mode Bit 1  y Bit 3 :%02x\n", data);
			break;
		}
		case 0x12:
		{
			vp_color_dst = data;
			// logerror("vgp: Offs:0x12: Set Destination Color           data:%02x\n", vp_color_dst);
			break;
		}
		case 0x13:
		{
			vp_gfxB_src_low = data;
			// logerror("vgp: Offs:0x13: Set GFX's source address low:   data:%02x\n", vp_gfxB_src_low);
			break;
		}
		case 0x14:
		{
			vp_gfxB_src_high = data;
			// logerror("vgp: Offs:0x14: Set GFX's source address high:  data:%02x\n", vp_gfxB_src_high);
			break;
		}
		case 0x15:
		{
			vp_vram_dest_col = data;
			// logerror("vgp: Offs:0x15: Set vp_vram_dest_row dest address low:   data:%02x\n", vp_vram_dest_col);
			break;
		}
		case 0x16:
		{
			vp_vram_dest_row = data;
			// logerror("vgp: Offs:0x16: Set vp_vram_dest_col dest address high:  data:%02x\n", vp_vram_dest_row);
			break;
		}
		case 0x17:
		{
			// logerror("vgp: Offs:0x17: row_end: data:%02x\n", data);
			vp_col_end = data;
			break;
		}
		case 0x18:
		{
			vp_row_end = data;
			// logerror("vgp: Offs:0x18: col_end                  data:%02x\n", vp_col_end);
			vp_exec(data);
			break;
		}
		case 0x19:
		{
			vp_buff_addr = data & 0x0f;
			// logerror("%s:vgp: Offs:0x19: Set Buffer Address: data:%02x\n", machine().describe_context(), vp_buff_addr);
			break;
		}
		case 0x1a:
		{
			vp_buff[vp_buff_addr] = data;
			// logerror("%s:vgp: Offs:0x1a: Set Buffer Data: offs:%02x - data:%02x\n", machine().describe_context(), vp_buff_addr, vp_buff[vp_buff_addr]);
			m_cont = 0;
			break;
		}
		case 0x1b:
		{
			//select mode ( bit 0, bit 1, bit 2, bit 3 )
			vp_cmd_mode = data;
			// logerror("vgp: Offs:0x1b: Mode register: data:%02x\n", vp_cmd_mode);
			break;
		}
		default:
			logerror(" vgp: Unknown register (out of range): offs:%02x - data:%02x\n", offs, data);
	}
}


template<u8 N>
TILE_GET_INFO_MEMBER(jackhouse_state::get_tile_info)
{
// ram 0/3: tile code low | ram 1/4: tile code hi | ram 2/5: color
	u8 sel = N * 3;
	int offs = tile_index;
	int color = m_videoram[sel + 2][offs];
	int code =  m_videoram[sel][offs] | (m_videoram[sel + 1][offs] << 8);

	tileinfo.set(N, code, color, 0);
}

void jackhouse_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(jackhouse_state::get_tile_info<0>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(jackhouse_state::get_tile_info<1>)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
	m_fg_tilemap->set_transparent_pen(0);
}

uint32_t jackhouse_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


/***************************************
*           Graphics Layouts           *
***************************************/

static const gfx_layout tilelayout =
{
	8, 8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4*7, 4*6, 4*1, 4*0, 4*3, 4*2, 4*5, 4*4 },
	{ STEP8(0, 8*4) },
	8*8*4
};


/***************************************
*            Graphics Decode           *
***************************************/

static GFXDECODE_START( gfx_jackhouse )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 0, 16 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 0, 16 )
GFXDECODE_END


/***************************************
*                  Lamps               *
***************************************/

void jackhouse_state::butt_lamps_w(u8 data)
{
/* Port 0x90 - Lamps
bit 0 - lamp0 = Bet1/Big
bit 1 - lamp1 = Double-Up
bit 2 - lamp2 = Take/Hit
bit 3 - lamp3 = Bet3
bit 4 - lamp4 = Start/Stand
bit 5 - lamp5 = Bet2/Small/Split
*/
	for (u8 i = 0; i < 8; i++)
		m_lamps[i] = BIT(data, i);
	// logerror("Port 90h:Lamps?: data:%02x\n", data);
}

void jackhouse_state::outports_w(u8 data)
{
// Port 0x94
	machine().bookkeeping().coin_counter_w(0, BIT(data, 3));  // Coin 1
	machine().bookkeeping().coin_counter_w(1, BIT(data, 4));  // Coin 2
	machine().bookkeeping().coin_counter_w(2, BIT(data, 5));  // Remote
	machine().bookkeeping().coin_counter_w(3, BIT(data, 6));  // Coin 3
	machine().bookkeeping().coin_counter_w(4, BIT(data, 0));  // Hand Paid
	machine().bookkeeping().coin_counter_w(5, BIT(data, 2));  // Hopper Coin Out

	m_hopper->motor_w(BIT(data, 7));
}

/***************************************
*               Input ports            *
***************************************/

static INPUT_PORTS_START( jackhouse )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-1") PORT_CODE(KEYCODE_A)     // unknown
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("0-2") PORT_CODE(KEYCODE_S)     // unknown
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD1)  PORT_NAME("Bet 1 / Big")            // bet 1 / big
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP ) PORT_NAME("Double-Up")              // d-up
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE ) PORT_NAME("Take / Hit")             // take score / stand
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_POKER_HOLD3)  PORT_NAME("Bet 3")                  // bet 3
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD2)  PORT_NAME("Bet 2 / Small / Split")  // bet 2 / small / split
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START )       PORT_NAME("Start / Stand")          // start / hit

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-1") PORT_CODE(KEYCODE_D)  // unknown
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-2") PORT_CODE(KEYCODE_F)  // unknown
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-3") PORT_CODE(KEYCODE_G)  // unknown
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("1-4") PORT_CODE(KEYCODE_H)  // unknown
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_NAME("Coin A") PORT_IMPULSE(1)    // coin
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_NAME("Coin B") PORT_IMPULSE(1)    // coin
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN ) PORT_NAME("Remote")             // remote credits
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_NAME("Coin C") PORT_IMPULSE(1)    // coin

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-1") PORT_CODE(KEYCODE_J)              // unknown
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-2") PORT_CODE(KEYCODE_K)              // unknown
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_CUSTOM )  PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("2-4") PORT_CODE(KEYCODE_M)              // unknown
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )                                            // attendant pay key
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )                                            // hopper payout button
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE )         PORT_NAME("Confirm / Test Mode")  // confirm / test mode
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )            PORT_NAME("Record Mode")          // books-record / color test

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME(0x07, 0x07, "Main Chance Rate")    PORT_DIPLOCATION("DSW1:1,2,3")
	PORT_DIPSETTING(0x00, "84")
	PORT_DIPSETTING(0x01, "86")
	PORT_DIPSETTING(0x02, "88")
	PORT_DIPSETTING(0x03, "90")
	PORT_DIPSETTING(0x04, "92")
	PORT_DIPSETTING(0x05, "94")
	PORT_DIPSETTING(0x06, "96")
	PORT_DIPSETTING(0x07, "98")
	PORT_DIPNAME(0x18, 0x18, "Credit/Coin")         PORT_DIPLOCATION("DSW1:4,5")
	PORT_DIPSETTING(0x18, "1")
	PORT_DIPSETTING(0x10, "2")
	PORT_DIPSETTING(0x08, "5")
	PORT_DIPSETTING(0x00, "10")
	PORT_DIPNAME(0x60, 0x60, "Credit/Keyin")        PORT_DIPLOCATION("DSW1:6,7")
	PORT_DIPSETTING(0x60, "100")
	PORT_DIPSETTING(0x40, "200")
	PORT_DIPSETTING(0x20, "500")
	PORT_DIPSETTING(0x00, "1000")
	PORT_DIPNAME(0x80, 0x80, DEF_STR(Unknown))      PORT_DIPLOCATION("DSW1:8")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x80, DEF_STR(Off))

	PORT_START("DSW2")
	PORT_DIPNAME(0x07, 0x07, "D-Up Chance Rate")    PORT_DIPLOCATION("DSW2:1,2,3")
	PORT_DIPSETTING(0x00, "84")
	PORT_DIPSETTING(0x01, "86")
	PORT_DIPSETTING(0x02, "88")
	PORT_DIPSETTING(0x03, "90")
	PORT_DIPSETTING(0x04, "92")
	PORT_DIPSETTING(0x05, "94")
	PORT_DIPSETTING(0x06, "96")
	PORT_DIPSETTING(0x07, "98")
	PORT_DIPNAME(0x38, 0x38, "Bet Min/Max")         PORT_DIPLOCATION("DSW2:4,5,6")
	PORT_DIPSETTING(0x38, "Min 02 - Max 10")
	PORT_DIPSETTING(0x30, "Min 02 - Max 20")
	PORT_DIPSETTING(0x28, "Min 05 - Max 20")
	PORT_DIPSETTING(0x20, "Min 05 - Max 50")
	PORT_DIPSETTING(0x18, "Min 10 - Max 50")
	PORT_DIPSETTING(0x10, "Min 10 - Max 100")
	PORT_DIPSETTING(0x08, "Min 20 - Max 50")
	PORT_DIPSETTING(0x00, "Min 20 - Max 100")
	PORT_DIPNAME(0xc0, 0xc0, "Decks")               PORT_DIPLOCATION("DSW2:7,8")
	PORT_DIPSETTING(0x00, "6")
	PORT_DIPSETTING(0x40, "5")
	PORT_DIPSETTING(0x80, "4")
	PORT_DIPSETTING(0xc0, "3")

	PORT_START("DSW3")
	PORT_DIPNAME(0x01, 0x01, "Freeze")              PORT_DIPLOCATION("DSW3:1")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPNAME(0x0e, 0x0e, "Limit Over")          PORT_DIPLOCATION("DSW3:2,3,4")
	PORT_DIPSETTING(0x0e, "5000")
	PORT_DIPSETTING(0x0c, "10000")
	PORT_DIPSETTING(0x0a, "15000")
	PORT_DIPSETTING(0x08, "20000")
	PORT_DIPSETTING(0x06, "30000")
	PORT_DIPSETTING(0x04, "40000")
	PORT_DIPSETTING(0x02, "50000")
	PORT_DIPSETTING(0x00, "99999")
	PORT_DIPNAME(0x10, 0x10, "Credit/Keyout")       PORT_DIPLOCATION("DSW3:5")
	PORT_DIPSETTING(0x00, "100")
	PORT_DIPSETTING(0x10, "1")
	PORT_DIPNAME(0x20, 0x20, "Double Up")           PORT_DIPLOCATION("DSW3:6")
	PORT_DIPSETTING(0x00, DEF_STR(Off))
	PORT_DIPSETTING(0x20, DEF_STR(On))
	PORT_DIPNAME(0x40, 0x40, DEF_STR(Unknown))      PORT_DIPLOCATION("DSW3:7")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPNAME(0x80, 0x80, DEF_STR(Unknown))      PORT_DIPLOCATION("DSW3:8")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x80, DEF_STR(Off))

	PORT_START("DSW4")
	PORT_DIPNAME(0x01, 0x01, DEF_STR(Unknown))      PORT_DIPLOCATION("DSW4:1")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPNAME(0x02, 0x02, DEF_STR(Unknown))      PORT_DIPLOCATION("DSW4:2")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPNAME(0x04, 0x04, DEF_STR(Unknown))      PORT_DIPLOCATION("DSW4:3")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPNAME(0x08, 0x08, DEF_STR(Unknown))      PORT_DIPLOCATION("DSW4:4")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPNAME(0x10, 0x10, DEF_STR(Unknown))      PORT_DIPLOCATION("DSW4:5")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPNAME(0x20, 0x20, DEF_STR(Unknown))      PORT_DIPLOCATION("DSW4:6")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPNAME(0x40, 0x40, DEF_STR(Unknown))      PORT_DIPLOCATION("DSW4:7")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPNAME(0x80, 0x80, DEF_STR(Unknown))      PORT_DIPLOCATION("DSW4:8")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x80, DEF_STR(Off))

	PORT_START("DSW5")
	PORT_DIPNAME(0x01, 0x01, DEF_STR(Unknown))      PORT_DIPLOCATION("DSW5:1")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x01, DEF_STR(Off))
	PORT_DIPNAME(0x02, 0x02, DEF_STR(Unknown))      PORT_DIPLOCATION("DSW5:2")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x02, DEF_STR(Off))
	PORT_DIPNAME(0x04, 0x04, DEF_STR(Unknown))      PORT_DIPLOCATION("DSW5:3")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x04, DEF_STR(Off))
	PORT_DIPNAME(0x08, 0x08, DEF_STR(Unknown))      PORT_DIPLOCATION("DSW5:4")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x08, DEF_STR(Off))
	PORT_DIPNAME(0x10, 0x10, DEF_STR(Unknown))      PORT_DIPLOCATION("DSW5:5")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x10, DEF_STR(Off))
	PORT_DIPNAME(0x20, 0x20, DEF_STR(Unknown))      PORT_DIPLOCATION("DSW5:6")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x20, DEF_STR(Off))
	PORT_DIPNAME(0x40, 0x40, DEF_STR(Unknown))      PORT_DIPLOCATION("DSW5:7")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x40, DEF_STR(Off))
	PORT_DIPNAME(0x80, 0x80, DEF_STR(Unknown))      PORT_DIPLOCATION("DSW5:8")
	PORT_DIPSETTING(0x00, DEF_STR(On))
	PORT_DIPSETTING(0x80, DEF_STR(Off))

INPUT_PORTS_END


/***************************************
*            Machine Driver            *
***************************************/

void jackhouse_state::jackhouse(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, MASTER_CLOCK / 2);  /* 3 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &jackhouse_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &jackhouse_state::io_map);
	m_maincpu->set_periodic_int(FUNC(jackhouse_state::irq0_line_hold), attotime::from_hz(60));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	i8255_device &ppi0(I8255(config, "ppi8255_0"));
	ppi0.in_pa_callback().set_ioport("IN0");
	ppi0.in_pb_callback().set_ioport("IN1");
	ppi0.in_pc_callback().set_ioport("IN2");

	i8255_device &ppi1(I8255(config, "ppi8255_1"));
	ppi1.in_pa_callback().set_ioport("DSW4");
	ppi1.in_pb_callback().set_ioport("DSW5");
	ppi1.in_pc_callback().set_ioport("DSW3");

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(512, 256);
	screen.set_visarea(0*8, 64*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(jackhouse_state::screen_update));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(0x300);

	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, m_palette));
	ramdac.set_addrmap(0, &jackhouse_state::ramdac_map);
	ramdac.set_color_base(0);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_jackhouse);
	config.set_default_layout(layout_jackhouse);

	HOPPER(config, m_hopper, attotime::from_msec(100), TICKET_MOTOR_ACTIVE_HIGH, TICKET_STATUS_ACTIVE_HIGH);

	SPEAKER(config, "speaker").front_center();

	ay8910_device &aysnd(AY8910(config, "aysnd", MASTER_CLOCK / 4));
	aysnd.port_a_read_callback().set_ioport("DSW2");
	aysnd.port_b_read_callback().set_ioport("DSW1");
	aysnd.add_route(ALL_OUTPUTS, "speaker", 0.85);
}


/***************************************
*                Rom Load              *
***************************************/

ROM_START( jackhous )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jack_1.512", 0x0000, 0xd000, CRC(98dadfc2) SHA1(e87d43ec1a468e2195b6dc24f79a0f199ba436ba) )
	ROM_IGNORE(                     0x3000)

	ROM_REGION( 0x40000, "gfx1", 0 )
	ROM_LOAD( "jack_2.020", 0x00000, 0x40000, CRC(5f0d53d2) SHA1(9ea1bbde743d59851b47fcabebfe2f9ecc4bfda7) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "jack_3.010", 0x00000, 0x20000, CRC(b4c0c053) SHA1(089d684903fa61152ed026bca7921a3a0ea76122) )

	ROM_REGION( 0x04000, "bg_tiles", 0 )
	ROM_LOAD("bg_tiles.bin", 0x00000, 0x04000, BAD_DUMP CRC(91437a4d) SHA1(543cac1a3959d5e44a54164083fe5c99af7f484e) )
ROM_END
}  // anonymous namespace


/*********************************************
*                Game Drivers                *
*                                            *
*********************************************/

//   YEAR  NAME         PARENT  MACHINE    INPUT      CLASS            INIT        ROT    COMPANY                   FULLNAME     FLAGS
GAME(1994, jackhous,    0,      jackhouse, jackhouse, jackhouse_state, empty_init, ROT0, "Chain Leisure Co., Ltd", "Jack House", 0)
