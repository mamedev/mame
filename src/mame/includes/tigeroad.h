// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino

#include "video/tigeroad_spr.h"

#include "cpu/m68000/m68000.h"
#include "cpu/m6805/m68705.h"
#include "cpu/z80/z80.h"
#include "cpu/mcs51/mcs51.h"

#include "machine/gen_latch.h"

#include "sound/2203intf.h"
#include "sound/msm5205.h"

#include "video/bufsprite.h"

#include "emupal.h"
#include "tilemap.h"


class tigeroad_state : public driver_device
{
public:
	tigeroad_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_spriteram(*this, "spriteram")
		, m_videoram(*this, "videoram")
		, m_ram16(*this, "ram16")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_msm(*this, "msm")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_spritegen(*this, "spritegen")
		, m_soundlatch(*this, "soundlatch")
		, m_has_coinlock(true)
	{ }

	void toramich(machine_config &config);
	void tigeroad(machine_config &config);
	void f1dream_comad(machine_config &config);

protected:
	required_device<buffered_spriteram16_device> m_spriteram;
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_ram16;
	int m_bgcharbank;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	DECLARE_WRITE16_MEMBER(tigeroad_soundcmd_w);
	DECLARE_WRITE16_MEMBER(tigeroad_videoram_w);
	DECLARE_WRITE16_MEMBER(tigeroad_videoctrl_w);
	DECLARE_WRITE16_MEMBER(tigeroad_scroll_w);
	DECLARE_WRITE8_MEMBER(msm5205_w);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILEMAP_MAPPER_MEMBER(tigeroad_tilemap_scan);
	virtual void video_start() override;
	uint32_t screen_update_tigeroad(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<msm5205_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<tigeroad_spr_device> m_spritegen;
	required_device<generic_latch_8_device> m_soundlatch;

	void comad_sound_io_map(address_map &map);
	void comad_sound_map(address_map &map);
	void main_map(address_map &map);
	void sample_map(address_map &map);
	void sample_port_map(address_map &map);
	void sound_map(address_map &map);
	void sound_port_map(address_map &map);
	/* misc */
	bool m_has_coinlock;
};


class pushman_state : public tigeroad_state
{
public:
	pushman_state(const machine_config &mconfig, device_type type, const char *tag)
		: tigeroad_state(mconfig, type, tag)
		, m_mcu(*this, "mcu")
		, m_host_semaphore(false)
		, m_mcu_semaphore(false)
		, m_host_latch(0xffff)
		, m_mcu_latch(0xffff)
		, m_mcu_output(0xffff)
		, m_mcu_latch_ctl(0xff)
	{
		m_has_coinlock = false;
	}

	void pushman(machine_config &config);
	void bballs(machine_config &config);

private:
	DECLARE_READ16_MEMBER(mcu_comm_r);
	DECLARE_WRITE16_MEMBER(pushman_mcu_comm_w);
	DECLARE_WRITE16_MEMBER(bballs_mcu_comm_w);

	DECLARE_WRITE8_MEMBER(mcu_pa_w);
	DECLARE_WRITE8_MEMBER(mcu_pb_w);
	DECLARE_WRITE8_MEMBER(mcu_pc_w);

	void bballs_map(address_map &map);
	void pushman_map(address_map &map);

	virtual void machine_start() override;

	required_device<m68705u_device> m_mcu;

	bool    m_host_semaphore, m_mcu_semaphore;
	u16     m_host_latch, m_mcu_latch;
	u16     m_mcu_output;
	u8      m_mcu_latch_ctl;
};

class f1dream_state : public tigeroad_state
{
public:
	f1dream_state(const machine_config &mconfig, device_type type, const char *tag)
		: tigeroad_state(mconfig, type, tag)
		, m_mcu(*this, "mcu")
		, m_old_p3(0xff)
	{
	}

	void f1dream(machine_config &config);

private:
	void f1dream_map(address_map &map);
	void f1dream_mcu_io(address_map &map);

	DECLARE_WRITE8_MEMBER(out1_w);
	DECLARE_WRITE8_MEMBER(out3_w);

	DECLARE_READ8_MEMBER(mcu_shared_r);
	DECLARE_WRITE8_MEMBER(mcu_shared_w);

	DECLARE_WRITE16_MEMBER(blktiger_to_mcu_w);

	required_device<i8751_device> m_mcu;
	uint8_t m_old_p3;
};
