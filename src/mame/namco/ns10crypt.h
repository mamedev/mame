// license:BSD-3-Clause
// copyright-holders:Andreas Naive

#ifndef MAME_NAMCO_NS10CRYPT_H
#define MAME_NAMCO_NS10CRYPT_H

#include <cstdint>

class ns10_decrypter_device : public device_t
{
public:
	void activate(int iv);
	void deactivate();
	bool is_active() const;

	virtual uint16_t decrypt(uint16_t cipherword) = 0;
	virtual ~ns10_decrypter_device();

protected:
	ns10_decrypter_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void init(int iv) = 0;
	virtual void device_start() override = 0;

	bool m_active;
};

class ns10_type1_decrypter_device : public ns10_decrypter_device
{
public:
	// with just only type-1 game known, we cannot say which parts of the crypto_logic is common, if any,
	// and which is game-specific. In practice, this class is just an alias for the decrypter device of mrdrilr2
	uint16_t decrypt(uint16_t cipherword) override;

protected:
	ns10_type1_decrypter_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

private:
	uint16_t m_mask = 0;
	uint8_t m_counter = 0;
	static const int initSbox[16];

	void init(int iv) override;
	void device_start() override;
};

class ns10_type2_decrypter_device : public ns10_decrypter_device
{
public:
	class gf2_reducer // helper class
	{
	public:
		gf2_reducer();
		int gf2_reduce(uint64_t num) const;

	private:
		int m_gf2Reduction[0x10000]{};
	};

	// this encodes the decryption logic, which varies per game
	// and is probably hard-coded into the CPLD
	struct ns10_crypto_logic
	{
		uint64_t eMask[16]{};
		uint64_t dMask[16]{};
		uint16_t xMask = 0;
		std::function<uint16_t(uint64_t, uint64_t, const gf2_reducer &)> nonlinear_calculation; // preliminary encoding; need research
	};

	ns10_type2_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	ns10_type2_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, const ns10_crypto_logic logic);

	uint16_t decrypt(uint16_t cipherword) override;

private:
	uint16_t m_mask = 0;
	uint64_t m_previous_cipherwords = 0;
	uint64_t m_previous_plainwords = 0;
	const ns10_crypto_logic m_logic;
	static const int initSbox[16];

	void init(int iv) override;
	void device_start() override;

	std::unique_ptr<const gf2_reducer> m_reducer;

	bool m_logic_initialized;
};

class mrdrilr2_decrypter_device : public ns10_type1_decrypter_device
{
public:
	mrdrilr2_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

DECLARE_DEVICE_TYPE(MRDRILR2_DECRYPTER, mrdrilr2_decrypter_device) // Type 1
DECLARE_DEVICE_TYPE(NS10_TYPE2_DECRYPTER, ns10_type2_decrypter_device)

#endif // MAME_NAMCO_NS10CRYPT_H
