// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_CARD_H
#define MAME_TVGAMES_XAVIX_IO_CARD_H

#pragma once

#include <array>
#include <utility>

// Shared timed event-token transport; derived readers retain their distinct physical encodings.
class xavix_card_reader_device : public device_t
{
public:
	template <typename... T> void set_event_callback(T &&... args) { m_event_out_cb.set(std::forward<T>(args)...); }
	void scan_card(int state);

protected:
	xavix_card_reader_device(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner,
			u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual u8 begin_scan() = 0;
	virtual attotime interval_after_event(u8 events_emitted) const = 0;

private:
	TIMER_CALLBACK_MEMBER(event_timer);

	device_delegate<void ()> m_event_out_cb;
	emu_timer *m_event_timer;
	u8 m_event_count;
	u8 m_event_index;
	bool m_scanning;
};

class xavix_epo_kabj_card_reader_device : public xavix_card_reader_device
{
public:
	xavix_epo_kabj_card_reader_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	virtual u8 begin_scan() override;
	virtual attotime interval_after_event(u8 events_emitted) const override;

	required_ioport m_code;
	u16 m_pattern;
};

class xavix_epo_es2j_card_reader_device : public xavix_card_reader_device
{
public:
	xavix_epo_es2j_card_reader_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	// Bits 0-23 hold the raw pattern (the decoder discards bit 0); bits 24-25 select its terminating edge count.
	static constexpr std::array<u32, 0x7c> CARD_STREAMS{{
		0x023a00a0, 0x02390050, 0x033c8000, 0x02378140, 0x02384028, 0x012f41f4, 0x0232c0f0, 0x0235c014,
		0x023421a4, 0x02352130, 0x012ea1f4, 0x0233a098, 0x02346098, 0x012d61f4, 0x0230e154, 0x012fe1e0,
		0x02381050, 0x02331168, 0x02329154, 0x012d91f4, 0x02365050, 0x012d51f4, 0x012cd1f4, 0x0231d188,
		0x02343098, 0x012d31f4, 0x0230b078, 0x0233b028, 0x02347014, 0x012b71f4, 0x0232f028, 0x012df0f0,
		0x023608c4, 0x02330968, 0x02348988, 0x012d89f4, 0x02344930, 0x012d49f4, 0x012cc9f4, 0x012fc8f0,
		0x023228b4, 0x02352828, 0x012ca9f4, 0x0233a850, 0x023468a0, 0x02376800, 0x012ee9a4, 0x0231e940,
		0x023618a0, 0x02351940, 0x02389800, 0x02339940, 0x02345814, 0x012b59f4, 0x0232d828, 0x012dd8f0,
		0x023038f0, 0x023138c4, 0x012ab9f4, 0x012fb898, 0x023078c4, 0x012979f4, 0x012ef988, 0x012bf968,
		0x023005f4, 0x02350530, 0x023284f0, 0x012d85f4, 0x02364428, 0x02334498, 0x012cc5f4, 0x012fc478,
		0x023225a4, 0x02352450, 0x012ea5d8, 0x012ba5f4, 0x02346450, 0x02316498, 0x012ae5f4, 0x0231e414,
		0x02361414, 0x012d15f4, 0x02349428, 0x012f94f0, 0x023054b4, 0x012b55f4, 0x0232d428, 0x0231d450,
		0x023034f0, 0x02333504, 0x012cb5d8, 0x0129b5f4, 0x012e74f0, 0x02317428, 0x0230f414, 0x0027f5f4,
		0x02340c98, 0x02310d54, 0x02308de0, 0x012b8df4, 0x02344c28, 0x012b4df4, 0x012ecde0, 0x0129cdf4,
		0x02322c98, 0x02312d88, 0x012aadf4, 0x0231ad04, 0x02306c98, 0x02316c14, 0x0128edf4, 0x012fec50,
		0x02341c50, 0x012f1da4, 0x012c9dd8, 0x01299df4, 0x012e5c78, 0x012f5c98, 0x0128ddf4, 0x012fdc28,
		0x012e3c78, 0x012d3cf0, 0x012cbc78, 0x0027bdf4
	}};

	virtual u8 begin_scan() override;
	virtual attotime interval_after_event(u8 events_emitted) const override;

	required_ioport m_card;
	u32 m_pattern;
};

DECLARE_DEVICE_TYPE(XAVIX_EPO_KABJ_CARD_READER, xavix_epo_kabj_card_reader_device)
DECLARE_DEVICE_TYPE(XAVIX_EPO_ES2J_CARD_READER, xavix_epo_es2j_card_reader_device)

#endif // MAME_TVGAMES_XAVIX_IO_CARD_H
