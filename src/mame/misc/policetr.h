// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    P&P Marketing Police Trainer hardware

**************************************************************************/

#include "cpu/mips/mips1.h"
#include "machine/eepromser.h"
#include "sound/bsmt2000.h"
#include "video/ramdac.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class policetr_state : public driver_device
{
public:
	policetr_state(const machine_config &mconfig, device_type type, const char *tag)
		: policetr_state(mconfig, type, tag, 0x1fc028ac, 0x00000fc8)
	{ }

	void policetr(machine_config &config);

	int bsmt_status_r();

protected:
	policetr_state(const machine_config &mconfig, device_type type, const char *tag, uint32_t speedup_pc, uint32_t speedup_addr) :
		driver_device(mconfig, type, tag),
		m_srcbitmap(*this, "gfx"),
		m_rambase(*this, "rambase"),
		m_maincpu(*this, "maincpu"),
		m_bsmt(*this, "bsmt"),
		m_bsmt_region(*this, "bsmt"),
		m_lspeaker(*this, "lspeaker"),
		m_rspeaker(*this, "rspeaker"),
		m_eeprom(*this, "eeprom"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_ramdac(*this, "ramdac"),
		m_leds(*this, "leds%u", 0U),
		m_gun_x_io(*this, "GUNX%u", 1U),
		m_gun_y_io(*this, "GUNY%u", 1U),
		m_speedup_pc(speedup_pc),
		m_speedup_addr(speedup_addr) { }

	virtual void driver_start() override;
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	void mem(address_map &map) ATTR_COLD;

	void ramdac_map(address_map &map) ATTR_COLD;

	void control_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void speedup_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	void bsmt2000_reg_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void bsmt2000_data_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint8_t bsmt2000_data_r(offs_t offset);

	void video_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t video_r();
	void vblank(int state);
	void render_display_list(offs_t offset);
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_region_ptr<uint8_t> m_srcbitmap;
	required_shared_ptr<uint32_t> m_rambase;
	required_device<r3041_device> m_maincpu;
	required_device<bsmt2000_device> m_bsmt;
	required_region_ptr<uint8_t> m_bsmt_region;
	required_device<speaker_device> m_lspeaker;
	required_device<speaker_device> m_rspeaker;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<ramdac_device> m_ramdac;

	enum
	{
		LED_PCB_RED,
		LED_PCB_GREEN,
		LED_COIN1,
		LED_COIN2
	};

	output_finder<4> m_leds;

	required_ioport_array<2> m_gun_x_io;
	required_ioport_array<2> m_gun_y_io;

	uint32_t m_control_data = 0;
	uint32_t m_bsmt_data_bank = 0;
	uint32_t m_bsmt_data_offset = 0;
	uint32_t *m_speedup_data = nullptr;
	uint64_t m_last_cycles = 0;
	uint32_t m_loop_count = 0;
	offs_t m_speedup_pc;
	offs_t m_speedup_addr;
	rectangle m_render_clip;
	std::unique_ptr<bitmap_ind8> m_dstbitmap;
	uint16_t m_src_xoffs = 0;
	uint16_t m_src_yoffs = 0;
	uint16_t m_dst_xoffs = 0;
	uint16_t m_dst_yoffs = 0;
	uint8_t m_video_latch = 0;
	uint32_t m_srcbitmap_height_mask = 0;

	static constexpr uint32_t SRCBITMAP_WIDTH = 4096;
	static constexpr uint32_t SRCBITMAP_WIDTH_MASK = SRCBITMAP_WIDTH - 1;
	static constexpr uint32_t DSTBITMAP_WIDTH = 512;
	static constexpr uint32_t DSTBITMAP_HEIGHT = 256;
};

class sshooter_state : public policetr_state
{
public:
	sshooter_state(const machine_config &mconfig, device_type type, const char *tag)
		: sshooter_state(mconfig, type, tag, 0x1fc03440, 0x00018fd8)
	{ }

	void sshooter(machine_config &config);

protected:
	sshooter_state(const machine_config &mconfig, device_type type, const char *tag, uint32_t speedup_pc, uint32_t speedup_addr)
		: policetr_state(mconfig, type, tag, speedup_pc, speedup_addr)
	{ }

	void mem(address_map &map) ATTR_COLD;
};

class sshoot17_state : public sshooter_state
{
public:
	sshoot17_state(const machine_config &mconfig, device_type type, const char *tag)
		: sshooter_state(mconfig, type, tag, 0x1fc03470, 0x00018fd8)
	{ }
};

class sshoot12_state : public sshooter_state
{
public:
	sshoot12_state(const machine_config &mconfig, device_type type, const char *tag)
		: sshooter_state(mconfig, type, tag, 0x1fc033e0, 0x00018fd8)
	{ }
};

class sshoot11_state : public sshooter_state
{
public:
	sshoot11_state(const machine_config &mconfig, device_type type, const char *tag)
		: sshooter_state(mconfig, type, tag, 0x1fc032f8, 0x00018fd8)
	{ }
};

class plctr13b_state : public sshooter_state
{
public:
	plctr13b_state(const machine_config &mconfig, device_type type, const char *tag)
		: sshooter_state(mconfig, type, tag, 0x1fc028bc, 0x00000fc8)
	{ }
};

class polict10_state : public sshooter_state
{
public:
	polict10_state(const machine_config &mconfig, device_type type, const char *tag)
		: sshooter_state(mconfig, type, tag, 0x1fc028b4, 0x00000fc8)
	{ }
};
