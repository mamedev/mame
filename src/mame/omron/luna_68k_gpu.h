// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay, O. Galibert
#ifndef MAME_OMRON_LUNA_68K_GPU_H
#define MAME_OMRON_LUNA_68K_GPU_H

#pragma once

#include "luna_68k_video.h"

#include "bus/rs232/rs232.h"
#include "cpu/m68000/m68020.h"
#include "machine/mc68681.h"
#include "machine/mc68901.h"
#include "video/bt45x.h"
#include "machine/nvram.h"
#include "screen.h"

class luna_68k_gpu_device : public device_t, public device_luna_68k_video_interface {
public:
	luna_68k_gpu_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock=0);

	virtual void vme_map(address_map &map) override;

public:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_device<m68020fpu_device> m_cpu;
	required_device<bt458_device> m_dac;
	required_device<mc68901_device> m_mfp;
	required_device<rs232_port_device> m_tty;
	required_device_array<mc68681_device, 2> m_duart;
	required_device<screen_device> m_screen;
	required_shared_ptr<u32> m_host;
	memory_share_creator<u16> m_fb;

	enum { CMD_SIZE_UNLIMITED = 42 };

	std::array<u32, 64> m_command;
	u32 m_size;
	u32 m_expected_size;
	u16 m_bx0, m_bx1, m_vx, m_by0, m_by1, m_vy;
	u16 m_blnk, m_plndsp, m_rop;
	u32 m_pnkmsk;
	u16 m_wplndat, m_wplnslct, m_rplnslct;

	void gpu_w(u32 data);
	void gencmd();
	void multiword_done();

	void buschng();
	void lintex();
	void filtex();
	void ltprst();
	void filpat();
	void pickmd();
	void pick();
	void wplndat();
	void wplnslct();
	void rplnslct();
	void spset();
	void wvecdat();
	void pnkmsk1();
	void pnkmsk2();
	void unk0206();
	void veconoff();
	void polonoff();
	void recoff();
	void boxend();
	void vmend();
	void polend();
	void linwid();
	void viewx0();
	void viewy0();
	void viewx1();
	void viewy1();
	void altviewx0();
	void altviewy0();
	void altviewx1();
	void altviewy1();
	void drawwd();
	void mod();
	void fbiocmd();
	void veccmd();
	void strcmd();
	void polcmd();
	void bitblt2();
	void clear();
	void bitblt1();
	void mata();
	void matb();
	void matx();
	void matc();
	void matd();
	void maty();

	u16 bx0_r();
	void bx0_w(u16 data);
	u16 bx1_r();
	void bx1_w(u16 data);
	u16 vx_r();
	void vx_w(u16 data);
	u16 by0_r();
	void by0_w(u16 data);
	u16 by1_r();
	void by1_w(u16 data);
	u16 vy_r();
	void vy_w(u16 data);

	u16 blnk_r();
	void blnk_w(u16 data);
	u16 plndsp_r();
	void plndsp_w(u16 data);
	u16 rop_r();
	void rop_w(u16 data);
	u16 state1_r();
	u16 state2_r();
	u16 state3_r();

	u32 fb_r(offs_t offset);
	void fb_w(offs_t offset, u32 data);
	void host_w(offs_t offset, u32 data, u32 mem_mask);

	void cpu_map(address_map &map) ATTR_COLD;
	void cpuspace_map(address_map &map) ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

DECLARE_DEVICE_TYPE(LUNA_68K_GPU, luna_68k_gpu_device)

#endif
