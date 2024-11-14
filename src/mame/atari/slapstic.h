// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*************************************************************************

    Atari Slapstic decoding helper

**************************************************************************

    For more information on the slapstic, see slapstic.html, or go to
    http://www.aarongiles.com/slapstic.html

*************************************************************************/

#ifndef MAME_ATARI_SLAPSTIC_H
#define MAME_ATARI_SLAPSTIC_H

#pragma once


DECLARE_DEVICE_TYPE(SLAPSTIC, atari_slapstic_device)

/*************************************
 *
 *  Structure of slapstic params
 *
 *************************************/

class atari_slapstic_device :  public device_t
{
public:
	// construction/destruction
	atari_slapstic_device(const machine_config &mconfig, const char *tag, device_t *owner, int chipnum)
		: atari_slapstic_device(mconfig, tag, owner, u32(0))
	{
		m_chipnum = chipnum;
	}

	atari_slapstic_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename T> void set_bank(T &&tag) { m_bank.set_tag(std::forward<T>(tag)); }
	void set_view(memory_view &view) { m_view = &view; }

	template <typename T> void set_range(T &&tag, int index, offs_t start, offs_t end, offs_t mirror) {
		m_space.set_tag(std::forward<T>(tag), index);
		m_start = start;
		m_end = end;
		m_mirror = mirror;
	}

	void set_chipnum(int chipnum) { m_chipnum = chipnum; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_pre_save() override;
	virtual void device_post_load() override;

private:
	struct mask_value {
		u16 mask, value;
	};


	struct slapstic_data {
		u8 bankstart;
		u16 bank[4];

		mask_value alt1;
		mask_value alt2;
		mask_value alt3;
		mask_value alt4;
		int altshift;

		mask_value bit1;
		mask_value bit2;
		mask_value bit3c0;
		mask_value bit3s0;
		mask_value bit3c1;
		mask_value bit3s1;
		mask_value bit4;

		mask_value add1;
		mask_value add2;
		mask_value addplus1;
		mask_value addplus2;
		mask_value add3;
	};

	static const slapstic_data slapstic101;
	// 102 has never been encountered
	static const slapstic_data slapstic103;
	static const slapstic_data slapstic104;
	static const slapstic_data slapstic105;
	static const slapstic_data slapstic106;
	static const slapstic_data slapstic107;
	static const slapstic_data slapstic108;
	static const slapstic_data slapstic109;
	static const slapstic_data slapstic110;
	static const slapstic_data slapstic111;
	static const slapstic_data slapstic112;
	static const slapstic_data slapstic113;
	static const slapstic_data slapstic114;
	static const slapstic_data slapstic115;
	static const slapstic_data slapstic116;
	static const slapstic_data slapstic117;
	static const slapstic_data slapstic118;

	static const slapstic_data *const slapstic_table[];

	struct test {
		offs_t m_m, m_v;

		test() : m_m(0), m_v(0) {}
		test(offs_t m, offs_t v) : m_m(m), m_v(v) {}
		bool operator()(offs_t a) const { return (a & m_m) == m_v; }
	};

	struct checker {
		offs_t m_range_mask, m_range_value, m_shift, m_input_mask;

		checker(offs_t start, offs_t end, offs_t mirror, int data_width, int address_lines);

		test test_in(const mask_value &mv) const;
		test test_any(const mask_value &mv) const;

		test test_inside() const;

		test test_reset() const;
		test test_bank(u16 b) const;
	};

	struct state {
		atari_slapstic_device *m_sl;

		state(atari_slapstic_device *sl) : m_sl(sl) {}
		virtual ~state() = default;

		virtual void test(offs_t addr) const = 0;
		virtual u8 state_id() const = 0;
	};

	struct idle : public state {
		struct test m_reset;

		idle(atari_slapstic_device *sl, const checker &check, const slapstic_data *data);
		virtual void test(offs_t addr) const override;
		virtual u8 state_id() const override;
	};

	struct active_101_102 : public state {
		struct test m_bank[4], m_alt, m_bit;

		active_101_102(atari_slapstic_device *sl, const checker &check, const slapstic_data *data);
		virtual void test(offs_t addr) const override;
		virtual u8 state_id() const override;
	};

	struct active_103_110 : public state {
		struct test m_bank[4], m_alt, m_bit;

		active_103_110(atari_slapstic_device *sl, const checker &check, const slapstic_data *data);
		virtual void test(offs_t addr) const override;
		virtual u8 state_id() const override;
	};

	struct active_111_118 : public state {
		struct test m_bank[4], m_alt, m_add;

		active_111_118(atari_slapstic_device *sl, const checker &check, const slapstic_data *data);
		virtual void test(offs_t addr) const override;
		virtual u8 state_id() const override;
	};

	struct alt_valid_101_102 : public state {
		struct test m_reset, m_inside, m_valid;

		alt_valid_101_102(atari_slapstic_device *sl, const checker &check, const slapstic_data *data);
		virtual void test(offs_t addr) const override;
		virtual u8 state_id() const override;
	};

	struct alt_valid_103_110 : public state {
		struct test m_reset, m_valid;

		alt_valid_103_110(atari_slapstic_device *sl, const checker &check, const slapstic_data *data);
		virtual void test(offs_t addr) const override;
		virtual u8 state_id() const override;
	};

	struct alt_valid_111_118 : public state {
		struct test m_reset, m_valid, m_add;

		alt_valid_111_118(atari_slapstic_device *sl, const checker &check, const slapstic_data *data);
		virtual void test(offs_t addr) const override;
		virtual u8 state_id() const override;
	};

	struct alt_select_101_110 : public state {
		struct test m_reset, m_select;
		int m_shift;

		alt_select_101_110(atari_slapstic_device *sl, const checker &check, const slapstic_data *data, int shift);
		virtual void test(offs_t addr) const override;
		virtual u8 state_id() const override;
	};

	struct alt_select_111_118 : public state {
		struct test m_reset, m_select;
		int m_shift;

		alt_select_111_118(atari_slapstic_device *sl, const checker &check, const slapstic_data *data, int shift);
		virtual void test(offs_t addr) const override;
		virtual u8 state_id() const override;
	};

	struct alt_commit : public state {
		struct test m_reset, m_commit;

		alt_commit(atari_slapstic_device *sl, const checker &check, const slapstic_data *data);
		virtual void test(offs_t addr) const override;
		virtual u8 state_id() const override;
	};

	struct bit_load : public state {
		struct test m_reset, m_load;

		bit_load(atari_slapstic_device *sl, const checker &check, const slapstic_data *data);
		virtual void test(offs_t addr) const override;
		virtual u8 state_id() const override;
	};

	struct bit_set : public state {
		struct test m_reset, m_set0, m_clear0, m_set1, m_clear1, m_commit;
		bool m_is_odd;

		bit_set(atari_slapstic_device *sl, const checker &check, const slapstic_data *data, bool odd);
		virtual void test(offs_t addr) const override;
		virtual u8 state_id() const override;
	};

	struct add_load : public state {
		struct test m_reset, m_load;

		add_load(atari_slapstic_device *sl, const checker &check, const slapstic_data *data);
		virtual void test(offs_t addr) const override;
		virtual u8 state_id() const override;
	};

	struct add_set : public state {
		struct test m_reset, m_add1, m_add2, m_end;

		add_set(atari_slapstic_device *sl, const checker &check, const slapstic_data *data);
		virtual void test(offs_t addr) const override;
		virtual u8 state_id() const override;
	};



	int m_chipnum;

	optional_memory_bank m_bank;
	memory_view *m_view;
	optional_address_space m_space;
	offs_t m_start, m_end, m_mirror;

	std::unique_ptr<state> m_s_idle;
	std::unique_ptr<state> m_s_active;

	std::unique_ptr<state> m_s_alt_valid;
	std::unique_ptr<state> m_s_alt_select;
	std::unique_ptr<state> m_s_alt_commit;

	std::unique_ptr<state> m_s_bit_load;
	std::unique_ptr<state> m_s_bit_set_odd;
	std::unique_ptr<state> m_s_bit_set_even;

	std::unique_ptr<state> m_s_add_load;
	std::unique_ptr<state> m_s_add_set;

	const state *m_state;

	enum { S_IDLE, S_ACTIVE, S_ALT_VALID, S_ALT_SELECT, S_ALT_COMMIT, S_BIT_LOAD, S_BIT_SET_ODD, S_BIT_SET_EVEN, S_ADD_LOAD, S_ADD_SET };
	u8 m_saved_state;

	u8 m_current_bank;
	u8 m_loaded_bank;

	void change_bank(int bank);
	void commit_bank();
};

#endif // MAME_ATARI_SLAPSTIC_H
