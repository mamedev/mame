// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, David Haywood
#include "cpu/m68000/m68000.h"
#include "video/bufsprite.h"
#include "machine/74259.h"
#include "machine/eepromser.h"
#include "machine/timer.h"
#include "emupal.h"
#include "tilemap.h"

class gaelco2_state : public driver_device
{
public:
	gaelco2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_mainlatch(*this, "mainlatch"),
		m_spriteram(*this, "spriteram"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_vregs(*this, "vregs"),
		m_paletteram(*this, "paletteram"),
		m_shareram(*this, "shareram"),
		m_global_spritexoff(0)
	{ }

	void maniacsq_d5002fp(machine_config &config);
	void play2000(machine_config &config);
	void srollnd(machine_config &config);
	void alighunt(machine_config &config);
	void touchgo(machine_config &config);
	void alighunt_d5002fp(machine_config &config);
	void maniacsq(machine_config &config);
	void touchgo_d5002fp(machine_config &config);
	void saltcrdi(machine_config &config);

	void init_touchgo();
	void init_alighunt();
	void init_play2000();

	void coin1_counter_w(int state);
	void coin2_counter_w(int state);

protected:
	void vram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void vregs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void palette_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void wrally2_latch_w(offs_t offset, u16 data);

	void ROM16_split_gfx(const char *src_reg, const char *dst_reg, int start, int length, int dest1, int dest2);

	DECLARE_VIDEO_START(gaelco2);
	DECLARE_VIDEO_START(gaelco2_dual);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void mcu_hostmem_map(address_map &map) ATTR_COLD;

	required_device<m68000_device> m_maincpu;
	optional_device<ls259_device> m_mainlatch;
	required_device<buffered_spriteram16_device> m_spriteram;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<u16> m_vregs;
	required_shared_ptr<u16> m_paletteram;
	optional_shared_ptr<u16> m_shareram;

private:
	void shareram_w(offs_t offset, u8 data);
	u8 shareram_r(offs_t offset);
	void alighunt_coin_w(u16 data);
	void coin3_counter_w(int state);
	void coin4_counter_w(int state);
	template<unsigned Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	template<unsigned Layer> TILE_GET_INFO_MEMBER(get_tile_info_dual);
	int get_rowscrollmode_yscroll(bool first_screen);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int mask);
	u32 dual_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int index);

	void alighunt_map(address_map &map) ATTR_COLD;
	void maniacsq_map(address_map &map) ATTR_COLD;
	void play2000_map(address_map &map) ATTR_COLD;
	void touchgo_map(address_map &map) ATTR_COLD;
	void saltcrdi_map(address_map &map) ATTR_COLD;
	void srollnd_map(address_map &map) ATTR_COLD;

	// simulation
	u16 srollnd_share_sim_r(offs_t offset, u16 mem_mask = ~0);
	void srollnd_share_sim_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	u16 *m_videoram = nullptr;
	tilemap_t *m_pant[2]{};
	bool m_dual_monitor = false;
	int m_global_spritexoff;
};


class snowboar_state : public gaelco2_state
{
public:
	snowboar_state(const machine_config &mconfig, device_type type, const char *tag) :
		gaelco2_state(mconfig, type, tag),
		m_snowboar_protection(*this, "snowboar_prot")
	{ }

	void maniacsqs(machine_config &config);
	void snowboar(machine_config &config);

	void init_snowboara();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	u16 snowboar_protection_r();
	void snowboar_protection_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void snowboar_map(address_map &map) ATTR_COLD;

	required_shared_ptr<u16> m_snowboar_protection;

	u32 m_snowboard_latch = 0U;
};


class bang_state : public gaelco2_state
{
public:
	bang_state(const machine_config &mconfig, device_type type, const char *tag)
		: gaelco2_state(mconfig, type, tag)
		, m_light_x(*this, "LIGHT%u_X", 0U)
		, m_light_y(*this, "LIGHT%u_Y", 0U)
	{}

	void bang(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	template <unsigned Which> u16 gun_x();
	template <unsigned Which> u16 gun_y();
	void bang_clr_gun_int_w(u16 data);
	TIMER_DEVICE_CALLBACK_MEMBER(bang_irq);
	void bang_map(address_map &map) ATTR_COLD;

	required_ioport_array<2> m_light_x;
	required_ioport_array<2> m_light_y;

	bool m_clr_gun_int = false;
};


class wrally2_state : public gaelco2_state
{
public:
	wrally2_state(const machine_config &mconfig, device_type type, const char *tag)
		: gaelco2_state(mconfig, type, tag)
		, m_analog(*this, "ANALOG%u", 0U)
	{}

	void wrally2(machine_config &config);

	void init_wrally2();

	template <int N> int wrally2_analog_bit_r();

private:
	void wrally2_adc_clk(int state);
	void wrally2_adc_cs(int state);
	void wrally2_map(address_map &map) ATTR_COLD;

	required_ioport_array<2> m_analog;

	u8 m_analog_ports[2]{};
};
