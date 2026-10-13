// license:BSD-3-Clause
// copyright-holders:Alexandre Derumier
/******************************************************************************

    Philips CD-i Digital Video Cartridge slot, and the VMPEG cartridge.

    The slot is the expansion connector on the back of the player, which
    carries the bus, the interrupt, the video the cartridge produces and its
    audio.  A cartridge maps itself into the bus, so the machine only has to
    hand it the player's resources and take back what the slot carries.

    The VMPEG cartridge is the board that decodes MPEG-1: a Motorola MCD251
    for the video and a Motorola GSC38GG307 for the audio, and around them the
    OS-9 driver ROM, the register window they sit in, the transfers the
    SCC68070 performs for them, and the interrupt they share.  Other boards
    exist, notably IMPEG with a single MCD270 doing both, and they would be
    other cards in the same slot.

    Memory layout of the VMPEG cartridge as seen by the SCC68070:

        0xd00000..0xdfffff  1MB  additional system RAM
        0xe00000..0xe3ffff       register file (only A[15:1] is decoded, so
                                 the 64KB register window mirrors four times)
        0xe40000..0xe7ffff  256K OS-9 driver ROM
        0xe80000..0xefffff  512K picture DRAM of the video decoder, which does
                            not answer the bus until the driver has set the
                            cartridge up

    The audio decoder answers at 0xe03000, the video decoder at 0xe04000.

*******************************************************************************/

#ifndef MAME_PHILIPS_CDIDVC_H
#define MAME_PHILIPS_CDIDVC_H

#pragma once

#include "mcd212.h"

#include "machine/scc68070.h"
#include "sound/gsc38gg307.h"
#include "video/mcd251.h"

#include "screen.h"

#include <memory>


class cdi_dvc_slot_device;

// ======================> device_cdi_dvc_interface

// What the player needs from whatever cartridge is fitted.
class device_cdi_dvc_interface : public device_interface, public mcd212_ext_video_source
{
public:
	virtual ~device_cdi_dvc_interface();

	// the vector the cartridge supplies when its interrupt is acknowledged
	virtual uint8_t intack_r() = 0;

	// vertical timing from the player's screen
	virtual void screen_vblank(int state) = 0;

protected:
	device_cdi_dvc_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_pre_start() override;

	cdi_dvc_slot_device *m_slot;
};

// ======================> cdi_dvc_slot_device

class cdi_dvc_slot_device : public device_t, public device_single_card_slot_interface<device_cdi_dvc_interface>,
		public device_mixer_interface, public mcd212_ext_video_source
{
public:
	template <typename T>
	cdi_dvc_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: cdi_dvc_slot_device(mconfig, tag, owner, 0U)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	cdi_dvc_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// the player's side of the connector
	template <typename... T> void set_scc(T &&... args) { m_scc.set_tag(std::forward<T>(args)...); }
	template <typename... T> void set_screen(T &&... args) { m_screen.set_tag(std::forward<T>(args)...); }
	void set_pal(bool pal) { m_pal = pal; }
	auto intreq_callback() { return m_intreq_cb.bind(); }

	uint8_t intack_r();
	void screen_vblank(int state);
	void vblank_callback(screen_device &screen, bool state) { screen_vblank(state ? 1 : 0); }

	// mcd212_ext_video_source implementation, from the cartridge
	virtual bool ext_video_pixel(int x, int y, uint32_t &argb) const override;

	// the cartridge's side of the connector
	scc68070_device *scc() const { return m_scc; }
	screen_device *screen() const { return m_screen; }
	bool pal() const { return m_pal; }
	void intreq_w(int state) { m_intreq_cb(state); }

protected:
	virtual void device_start() override ATTR_COLD;

private:
	optional_device<scc68070_device> m_scc;
	optional_device<screen_device> m_screen;
	devcb_write_line m_intreq_cb;
	bool m_pal = true;
};

// ======================> cdi_dvc_vmpeg_device

class cdi_dvc_vmpeg_device : public device_t, public device_cdi_dvc_interface
{
public:
	cdi_dvc_vmpeg_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~cdi_dvc_vmpeg_device();

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_cdi_dvc_interface implementation
	virtual uint8_t intack_r() override;
	virtual void screen_vblank(int state) override;
	virtual bool ext_video_pixel(int x, int y, uint32_t &argb) const override;

private:
	// the bus
	uint16_t regs_r(offs_t offset, uint16_t mem_mask = ~0);
	void regs_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t rom_r(offs_t offset);
	uint16_t ram_r(offs_t offset, uint16_t mem_mask = ~0);
	void ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void ram_bus_error(offs_t offset, bool read);

	// a decoder asking for the next part of the system stream
	void fma_drq_w(int state);
	void fmv_drq_w(int state);
	void run_dma(bool for_fma);

	// either decoder raising or dropping its interrupt
	void chip_irq_w(int state);

	required_device<mcd251_device> m_fmv;
	required_device<gsc38gg307_device> m_fma;
	optional_region_ptr<uint16_t> m_rom;

	address_space *m_memory_space = nullptr;

	bool m_mpeg_ram_enabled = false;
	uint8_t m_mpeg_ram_enable_cnt = 0;

	// which decoder the transfer in progress belongs to
	bool m_transfer_for_fma = false;

	bool m_intreq_state = false;
};

void cdi_dvc_cards(device_slot_interface &device);

DECLARE_DEVICE_TYPE(CDI_DVC_SLOT, cdi_dvc_slot_device)
DECLARE_DEVICE_TYPE(CDI_DVC_VMPEG, cdi_dvc_vmpeg_device)

#endif // MAME_PHILIPS_CDIDVC_H
