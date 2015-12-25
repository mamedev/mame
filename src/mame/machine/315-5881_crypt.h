// license:BSD-3-Clause
// copyright-holders:David Haywood

#pragma once

#ifndef __SEGA315_5881_CRYPT__
#define __SEGA315_5881_CRYPT__

typedef device_delegate<UINT16 (UINT32)> sega_m2_read_delegate;

extern const device_type SEGA315_5881_CRYPT;

#define MCFG_SET_READ_CALLBACK( _class, _method) \
	sega_315_5881_crypt_device::set_read_cb(*device, sega_m2_read_delegate(&_class::_method, #_class "::" #_method, NULL, (_class *)0));


class sega_315_5881_crypt_device :  public device_t
{
public:
	// construction/destruction
	sega_315_5881_crypt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);


	UINT16 do_decrypt(UINT8 *&base);
	void set_addr_low(UINT16 data);
	void set_addr_high(UINT16 data);
	void set_subkey(UINT16 data);

	sega_m2_read_delegate m_read;

	static void set_read_cb(device_t &device,sega_m2_read_delegate readcb)
	{
		sega_315_5881_crypt_device &dev = downcast<sega_315_5881_crypt_device &>(device);
		dev.m_read = readcb;
	}

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:

	enum {
//        BUFFER_SIZE = 32768, LINE_SIZE = 512,
		BUFFER_SIZE = 2, LINE_SIZE = 512,  // this should be a stream, without any 'BUFFER_SIZE' ? I guess the SH4 DMA implementation isn't on a timer tho?
		FLAG_COMPRESSED = 0x20000
	};

	UINT32 key;

	std::unique_ptr<UINT8[]> buffer;
	std::unique_ptr<UINT8[]> line_buffer;
	std::unique_ptr<UINT8[]> line_buffer_prev;
	UINT32 prot_cur_address;
	UINT16 subkey, dec_hist;
	UINT32 dec_header;

	bool enc_ready;

	int buffer_pos, line_buffer_pos, line_buffer_size, buffer_bit, buffer_bit2;
	UINT8 buffer2[2];
	UINT16 buffer2a;

	int block_size;
	int block_pos;
	int block_numlines;
	int done_compression;

	struct sbox {
		UINT8 table[64];
		int inputs[6];      // positions of the inputs bits, -1 means no input except from key
		int outputs[2];     // positions of the output bits
	};

	static const sbox fn1_sboxes[4][4];
	static const sbox fn2_sboxes[4][4];

	static const int FN1GK = 38;
	static const int FN2GK = 32;
	static const int fn1_game_key_scheduling[FN1GK][2];
	static const int fn2_game_key_scheduling[FN2GK][2];
	static const int fn1_sequence_key_scheduling[20][2];
	static const int fn2_sequence_key_scheduling[16];
	static const int fn2_middle_result_scheduling[16];

	static const UINT8 trees[9][2][32];

	int feistel_function(int input, const struct sbox *sboxes, UINT32 subkeys);
	UINT16 block_decrypt(UINT32 game_key, UINT16 sequence_key, UINT16 counter, UINT16 data);

	UINT16 get_decrypted_16();
	int get_compressed_bit();

	void enc_start();
	void enc_fill();
	void line_fill();

};

#endif
