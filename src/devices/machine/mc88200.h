// license:BSD-3-Clause
// copyright-holders:Patrick Mackinlay

#ifndef MAME_MACHINE_MC88200_H
#define MAME_MACHINE_MC88200_H

#pragma once

class mc88200_device
	: public device_t
{
public:
	mc88200_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock, u8 id = 0);

	template <typename T> void set_mbus(T &&tag, int spacenum) { m_mbus.set_tag(std::forward<T>(tag), spacenum); }

	bool translate(int intention, u32 &address, bool supervisor);

	template <typename T> std::optional<T> read(u32 virtual_address, bool supervisor);
	template <typename T> bool write(u32 virtual_address, T data, bool supervisor);
	void bus_error_w(int state) { if (!machine().side_effects_disabled()) m_bus_error = true; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void map(address_map &map) ATTR_COLD;

	u32 idr_r() { return m_idr; }
	u32 scr_r() { return m_scr; }
	u32 ssr_r() { return m_ssr; }
	u32 sar_r() { return m_sar; }
	u32 sctr_r() { return m_sctr; }

	u32 pfsr_r() { return m_pfsr; }
	u32 pfar_r() { return m_pfar; }
	u32 sapr_r() { return m_sapr; }
	u32 uapr_r() { return m_uapr; }

	u32 cdp_r(offs_t offset) { return m_cache[BIT(m_sar, 4, 8)].line[offset].data[BIT(m_sar, 2, 2)]; }
	u32 ctp_r(offs_t offset) { return m_cache[BIT(m_sar, 4, 8)].line[offset].tag; }
	u32 cssp_r() { return m_cache[BIT(m_sar, 4, 8)].status; }

	void idr_w(u32 data);
	void scr_w(u32 data);
	void ssr_w(u32 data);
	void sar_w(u32 data);
	void sctr_w(u32 data);

	void pfsr_w(u32 data);
	void pfar_w(u32 data);
	void sapr_w(u32 data);
	void uapr_w(u32 data);

	void bwp_w(offs_t offset, u32 data);
	void cdp_w(offs_t offset, u32 data);
	void ctp_w(offs_t offset, u32 data);
	void cssp_w(u32 data);

	struct translate_result
	{
		translate_result(u32 address, bool ci, bool g, bool wt)
			: address(address)
			, ci(ci)
			, g(g)
			, wt(wt)
		{}

		u32 const address;
		bool const ci; // cache inhibit
		bool const g;  // global
		bool const wt; // writethrough
	};
	std::optional<translate_result> translate(u32 virtual_address, bool supervisor, bool write);

	struct cache_set
	{
		void set_mru(unsigned const line);

		void set_unmodified(unsigned const line);
		void set_modified(unsigned const line);
		void set_shared(unsigned const line);
		void set_invalid(unsigned const line);

		bool modified(unsigned const line) const;
		bool shared(unsigned const line) const;
		bool invalid(unsigned const line) const;

		bool enabled(unsigned const line) const;

		u32 status;

		struct cache_line
		{
			bool match_segment(u32 const address) const;
			bool match_page(u32 const address) const;

			bool load_line(mc88200_device &cmmu, u32 const address);
			bool copy_back(mc88200_device &cmmu, u32 const address, bool const flush = false);

			u32 tag;
			u32 data[4];
		}
		line[4];
	};
	typedef bool (mc88200_device::cache_set::cache_line::* match_function)(u32 const) const;

	std::optional<unsigned> cache_replace(cache_set const &cs);
	void cache_flush(unsigned const start, unsigned const limit, match_function match, bool const copyback, bool const invalidate);

	template <typename T> std::optional<T> mbus_read(u32 address);
	template <typename T> bool mbus_write(u32 address, T data, bool flush = false);

private:
	required_address_space m_mbus;

	u32 m_idr;  // identification register
	u32 m_scr;  // system command register
	u32 m_ssr;  // system status register
	u32 m_sar;  // system address register
	u32 m_sctr; // system control register
	u32 m_pfsr; // p bus fault status register
	u32 m_pfar; // p bus fault address register
	u32 m_sapr; // supervisor area pointer register
	u32 m_uapr; // user area pointer register

	u32 m_batc[10]; // block address translation cache
	u64 m_patc[56]; // page address translation cache
	u32 m_patc_ptr;

	bool m_bus_error;

	static constexpr unsigned CACHE_SETS = 256;
	std::unique_ptr<cache_set[]> m_cache; // data cache

	u32 const m_id;
};

DECLARE_DEVICE_TYPE(MC88200, mc88200_device)

#endif // MAME_MACHINE_MC88200_H
