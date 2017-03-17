// license:BSD-3-Clause
// copyright-holders:Andreas Naive

#ifndef _NS10CRYPT_H_
#define _NS10CRYPT_H_

#include <cstdint>

class gf2_reducer  // helper class
{
public:
	gf2_reducer();
	int gf2_reduce(uint64_t num)const;
private:
	int _gf2Reduction[0x10000];
};


class ns10_decrypter_device : public device_t
{
public:
	void activate(int iv);
	void deactivate();
	bool is_active()const;

	virtual uint16_t decrypt(uint16_t cipherword)=0;
	virtual ~ns10_decrypter_device();

protected:
	ns10_decrypter_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	virtual void init(int iv)=0;
	virtual void device_start()override=0;

	bool _active;
};

class ns10_type1_decrypter_device : public ns10_decrypter_device
{
public:
	// with just only type-1 game known, we cannot say which parts of the crypto_logic is common, if any,
	// and which is game-specific. In practice, this class is just an alias for the decrypter device of mrdrilr2
	uint16_t decrypt(uint16_t cipherword)override;

protected:
	ns10_type1_decrypter_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

private:
	uint16_t _mask;
	uint8_t _counter;
	static const int initSbox[16];

	void init(int iv)override;
	void device_start()override;
};


class ns10_type2_decrypter_device : public ns10_decrypter_device
{
public:
	// this encodes the decryption logic, which varies per game
	// and is probably hard-coded into the CPLD
	struct ns10_crypto_logic
	{
		uint64_t eMask[16];
		uint64_t dMask[16];
		uint16_t xMask;
		uint16_t(*nonlinear_calculation)(uint64_t, uint64_t, const gf2_reducer&);  // preliminary encoding; need research
	};

	uint16_t decrypt(uint16_t cipherword)override;

protected:
	ns10_type2_decrypter_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source,
		const ns10_type2_decrypter_device::ns10_crypto_logic &logic);

private:
	uint16_t _mask;
	uint64_t _previous_cipherwords;
	uint64_t _previous_plainwords;
	const ns10_crypto_logic& _logic;
	std::unique_ptr<const gf2_reducer>_reducer;
	static const int initSbox[16];

	void init(int iv)override;
	void device_start()override;
};



// game-specific devices

class mrdrilr2_decrypter_device : public ns10_type1_decrypter_device
{
public:
	mrdrilr2_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class chocovdr_decrypter_device : public ns10_type2_decrypter_device
{
public:
	chocovdr_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class gamshara_decrypter_device : public ns10_type2_decrypter_device
{
public:
	gamshara_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class gjspace_decrypter_device : public ns10_type2_decrypter_device
{
public:
	gjspace_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class knpuzzle_decrypter_device : public ns10_type2_decrypter_device
{
public:
	knpuzzle_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class konotako_decrypter_device : public ns10_type2_decrypter_device
{
public:
	konotako_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class nflclsfb_decrypter_device : public ns10_type2_decrypter_device
{
public:
	nflclsfb_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class startrgn_decrypter_device : public ns10_type2_decrypter_device
{
public:
	startrgn_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


extern const device_type MRDRILR2_DECRYPTER;
extern const device_type CHOCOVDR_DECRYPTER;
extern const device_type GAMSHARA_DECRYPTER;
extern const device_type  GJSPACE_DECRYPTER;
extern const device_type KNPUZZLE_DECRYPTER;
extern const device_type KONOTAKO_DECRYPTER;
extern const device_type NFLCLSFB_DECRYPTER;
extern const device_type STARTRGN_DECRYPTER;

#endif
