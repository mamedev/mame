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
	asr733_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_PALETTE_INIT(asr733);

	DECLARE_READ8_MEMBER(cru_r);
	DECLARE_WRITE8_MEMBER(cru_w);

	template<class _Object> static devcb_base &static_set_keyint_callback(device_t &device, _Object object)
	{
		return downcast<asr733_device &>(device).m_keyint_line.set_callback(object);
	}
	template<class _Object> static devcb_base &static_set_lineint_callback(device_t &device, _Object object)
	{
		return downcast<asr733_device &>(device).m_lineint_line.set_callback(object);
	}

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

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
	UINT8 m_OutQueue[ASROutQueueSize];
	int m_OutQueueHead;
	int m_OutQueueLen;
#endif

	void check_keyboard();
	void refresh(bitmap_ind16 &bitmap, int x, int y);

	void set_interrupt_line();
	void draw_char(int character, int x, int y, int color);
	void linefeed();
	void transmit(UINT8 data);

	emu_timer *m_line_timer;                // screen line timer

	UINT8   m_recv_buf;
	UINT8   m_xmit_buf;

	UINT8   m_status;
	UINT8   m_mode;
	UINT8   m_last_key_pressed;
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
