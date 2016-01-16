// license:BSD-3
// copyright-holders:Andreas Naive

#ifndef _NS10CRYPT_H_
#define _NS10CRYPT_H_

class gf2_reducer  // helper class
{
public:
	gf2_reducer();
	int gf2_reduce(UINT64 num)const;
private:
	int _gf2Reduction[0x10000];
};

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
		UINT16(*nonlinear_calculation)(UINT64, UINT64, const gf2_reducer&);  // preliminary encoding; need research
	};

	void activate(int iv);
	void deactivate();
	bool is_active()const;

	UINT16 decrypt(UINT16 cipherword);

protected:
	ns10_decrypter_device(
		device_type type, const ns10_decrypter_device::ns10_crypto_logic &logic,
		const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

private:
	UINT16 _mask;
	UINT64 _previous_cipherwords;
	UINT64 _previous_plainwords;
	bool _active;
	const ns10_crypto_logic& _logic;
	static const int initSbox[16];
	std::unique_ptr<const gf2_reducer>_reducer;

	void device_start() override;
	void init(int iv);
};



// game-specific devices

class chocovdr_decrypter_device : public ns10_decrypter_device
{
public:
	chocovdr_decrypter_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

class gamshara_decrypter_device : public ns10_decrypter_device
{
public:
	gamshara_decrypter_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

class gjspace_decrypter_device : public ns10_decrypter_device
{
public:
	gjspace_decrypter_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

class knpuzzle_decrypter_device : public ns10_decrypter_device
{
public:
	knpuzzle_decrypter_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

class konotako_decrypter_device : public ns10_decrypter_device
{
public:
	konotako_decrypter_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

class nflclsfb_decrypter_device : public ns10_decrypter_device
{
public:
	nflclsfb_decrypter_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};

class startrgn_decrypter_device : public ns10_decrypter_device
{
public:
	startrgn_decrypter_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
};


extern const device_type CHOCOVDR_DECRYPTER;
extern const device_type GAMSHARA_DECRYPTER;
extern const device_type  GJSPACE_DECRYPTER;
extern const device_type KNPUZZLE_DECRYPTER;
extern const device_type KONOTAKO_DECRYPTER;
extern const device_type NFLCLSFB_DECRYPTER;
extern const device_type STARTRGN_DECRYPTER;

#endif
