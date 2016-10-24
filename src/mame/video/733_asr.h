// license:GPL-2.0+
// copyright-holders:Raphael Nabet

#define asr733_chr_region ":gfx1"

enum
{
	/* 8 bytes per character definition */
	asr733_single_char_len = 8,
	asr733_chr_region_len   = 128*asr733_single_char_len
};

class asr733_device : public device_t, public device_gfx_interface
{
public:
	asr733_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void palette_init_asr733(palette_device &palette);

	uint8_t cru_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void cru_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	template<class _Object> static devcb_base &static_set_keyint_callback(device_t &device, _Object object)
	{
		return downcast<asr733_device &>(device).m_keyint_line.set_callback(object);
	}
	template<class _Object> static devcb_base &static_set_lineint_callback(device_t &device, _Object object)
	{
		return downcast<asr733_device &>(device).m_lineint_line.set_callback(object);
	}

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	void device_start() override;
	void device_reset() override;
	machine_config_constructor device_mconfig_additions() const override;
	void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	ioport_constructor device_input_ports() const override;

private:
	// internal state
#if 0
	uint8_t m_OutQueue[ASROutQueueSize];
	int m_OutQueueHead;
	int m_OutQueueLen;
#endif

	void check_keyboard();
	void refresh(bitmap_ind16 &bitmap, int x, int y);

	void set_interrupt_line();
	void draw_char(int character, int x, int y, int color);
	void linefeed();
	void transmit(uint8_t data);

	emu_timer *m_line_timer;                // screen line timer

	uint8_t   m_recv_buf;
	uint8_t   m_xmit_buf;

	uint8_t   m_status;
	uint8_t   m_mode;
	uint8_t   m_last_key_pressed;
	int     m_last_modifier_state;

	unsigned char m_repeat_timer;
	int     m_new_status_flag;

	int     m_x;

	std::unique_ptr<bitmap_ind16>       m_bitmap;

	devcb_write_line                   m_keyint_line;
	devcb_write_line                   m_lineint_line;
};

extern const device_type ASR733;

#define MCFG_ASR733_KEYINT_HANDLER( _intcallb ) \
	devcb = &asr733_device::static_set_keyint_callback( *device, DEVCB_##_intcallb );

#define MCFG_ASR733_LINEINT_HANDLER( _intcallb ) \
	devcb = &asr733_device::static_set_lineint_callback( *device, DEVCB_##_intcallb );
