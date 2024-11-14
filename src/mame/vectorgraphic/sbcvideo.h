// license:BSD-3-Clause
// copyright-holders:Eric Anderson
#ifndef MAME_VECTORGRAPHIC_SBCVIDEO_H
#define MAME_VECTORGRAPHIC_SBCVIDEO_H

#pragma once

#include "machine/ram.h"
#include "video/mc6845.h"


class vector_sbc_video_device : public device_t
{
public:
	vector_sbc_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_buffer(T &&tag) { m_buffer.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_chrroml(T &&tag) { m_chrroml.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_chrromr(T &&tag) { m_chrromr.set_tag(std::forward<T>(tag)); }

	void spr_w(uint8_t data);
	void res320_mapping_ram_w(offs_t offset, uint8_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	MC6845_UPDATE_ROW(update_row);

	required_ioport m_io_sbc_video_conf;
	required_device<ram_device> m_buffer;
	required_region_ptr<uint8_t> m_chrroml;
	required_region_ptr<uint8_t> m_chrromr;
	uint8_t m_spr;
	uint8_t m_res320_ram[4];
};

DECLARE_DEVICE_TYPE(SBC_VIDEO, vector_sbc_video_device)

#endif // MAME_VECTORGRAPHIC_SBCVIDEO_H
