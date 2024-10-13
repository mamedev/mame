// license:BSD-3-Clause
// copyright-holders:Andreas Naive

#ifndef MAME_NAMCO_NS10CRYPT_H
#define MAME_NAMCO_NS10CRYPT_H

#pragma once

#include <array>
#include <cstdint>


class ns10_decrypter_device : public device_t
{
public:
	virtual ~ns10_decrypter_device();

	void activate(int iv);
	void deactivate();
	bool is_active() const { return m_active; }

	virtual uint16_t decrypt(uint16_t cipherword) = 0;

protected:
	ns10_decrypter_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void init(int iv) = 0;

	bool m_active;
};

class ns10_type1_decrypter_device : public ns10_decrypter_device
{
public:
	// with just only type-1 game known, we cannot say which parts of the crypto_logic is common, if any,
	// and which is game-specific. In practice, this class is just an alias for the decrypter device of mrdrilr2
	virtual uint16_t decrypt(uint16_t cipherword) override;

protected:
	ns10_type1_decrypter_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static const int INIT_SBOX[16];

	virtual void init(int iv) override;

	uint16_t m_mask;
	uint8_t m_counter;
};

class ns10_type2_decrypter_device : public ns10_decrypter_device
{
public:
	// this encodes the decryption logic, which varies per game and is probably hard-coded into the CPLD
	struct ns10_crypto_logic
	{
		using nonlinear_calculation_function = uint16_t (*)(uint64_t, uint64_t);
		using iv_calculation_function = uint64_t (*)(int);
		uint64_t eMask[16]{};
		uint64_t dMask[16]{};
		uint16_t xMask = 0;
		nonlinear_calculation_function nonlinear_calculation = nullptr; // preliminary encoding; need research
		iv_calculation_function iv_calculation = nullptr;
	};

	ns10_type2_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	ns10_type2_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, ns10_crypto_logic &&logic);

	virtual uint16_t decrypt(uint16_t cipherword) override;

	static int gf2_reduce(uint64_t num)
	{
		return population_count_64(num) & 1;
	}

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static const int INIT_SBOX[16];

	virtual void init(int iv) override;

	uint16_t m_mask;
	uint64_t m_previous_cipherwords;
	uint64_t m_previous_plainwords;

	const ns10_crypto_logic m_logic;

	bool m_logic_initialized;
};

class ns10_type2_decrypter_nonlinear_device : public ns10_decrypter_device
{
public:
	// this encodes the decryption logic, which varies per game and is probably hard-coded into the CPLD
	struct ns10_crypto_logic
	{
		using nonlinear_calculation_function = uint16_t (*)(uint16_t);
		using iv_calculation_function = uint64_t (*)(int);
		uint64_t eMask[16]{};
		uint64_t dMask[16]{};
		uint16_t xMask = 0;
		nonlinear_calculation_function nonlinear_calculation = nullptr; // preliminary encoding; need research
		iv_calculation_function iv_calculation = nullptr;
	};

	ns10_type2_decrypter_nonlinear_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	ns10_type2_decrypter_nonlinear_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, ns10_crypto_logic &&logic);

	virtual uint16_t decrypt(uint16_t cipherword) override;

	static int gf2_reduce(uint64_t num)
	{
		return population_count_64(num) & 1;
	}

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	static const int INIT_SBOX[16];

	virtual void init(int iv) override;

	uint16_t m_mask;
	uint64_t m_previous_cipherwords;
	uint64_t m_previous_plainwords;
	uint32_t m_nonlinear_count;

	required_memory_region m_nonlinear_region;

	const ns10_crypto_logic m_logic;

	bool m_logic_initialized;
};

class mrdrilr2_decrypter_device : public ns10_type1_decrypter_device
{
public:
	mrdrilr2_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(MRDRILR2_DECRYPTER, mrdrilr2_decrypter_device) // Type 1
DECLARE_DEVICE_TYPE(NS10_TYPE2_DECRYPTER, ns10_type2_decrypter_device)
DECLARE_DEVICE_TYPE(NS10_TYPE2_DECRYPTER_NONLINEAR, ns10_type2_decrypter_nonlinear_device)

#endif // MAME_NAMCO_NS10CRYPT_H
