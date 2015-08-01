// license:BSD-3
// copyright-holders:Andreas Naive

#ifndef _NS10CRYPT_H_
#define _NS10CRYPT_H_

class ns10_decrypter_device : public device_t
{
public:
	// this encodes the decryption logic, which varies per game 
	// and is probably hard-coded into the CPLD
	struct ns10_crypto_logic
	{
		UINT64 eMask[16];
		UINT64 dMask[16];
		UINT16 xMask;
		UINT16(*nonlinear_calculation)(UINT64, UINT64);  // preliminary encoding; need research
	};

	void activate();
	void deactivate();
	bool is_active()const;

	UINT16 decrypt(UINT16 cipherword);

protected:
	ns10_decrypter_device(
		device_type type, const ns10_decrypter_device::ns10_crypto_logic &logic,
		const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

private:
	UINT16 _mask;
	UINT64 _previous_cipherwords;
	UINT64 _previous_plainwords;
	bool _active;
	const ns10_crypto_logic& _logic;
	int _gf2Reduction[0x10000];

	void device_start();
	void init();
	int gf2_reduce(UINT64 num);
};

// game-specific devices

class konotako_decrypter_device : public ns10_decrypter_device
{
public:
	konotako_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};

class startrgn_decrypter_device : public ns10_decrypter_device
{
public:
	startrgn_decrypter_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


extern const device_type KONOTAKO_DECRYPTER;
extern const device_type STARTRGN_DECRYPTER;

#endif
