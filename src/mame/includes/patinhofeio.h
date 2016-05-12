// license:GPL2+
// copyright-holders:Felipe Sanches
#include "machine/teleprinter.h"

class patinho_feio_state : public driver_device
{
public:
        patinho_feio_state(const machine_config &mconfig, device_type type, const char *tag)
                : driver_device(mconfig, type, tag)
                , m_maincpu(*this, "maincpu")
                , m_decwriter(*this, "decwriter")
                , m_tty(*this, "teletype")
        { }

        DECLARE_DRIVER_INIT(patinho_feio);
        DECLARE_READ16_MEMBER(rc_r);
        DECLARE_READ8_MEMBER(buttons_r);

        DECLARE_WRITE8_MEMBER(decwriter_data_w);
        DECLARE_WRITE8_MEMBER(decwriter_kbd_input);
        TIMER_CALLBACK_MEMBER(decwriter_callback);

        DECLARE_WRITE8_MEMBER(teletype_data_w);
        DECLARE_WRITE8_MEMBER(teletype_kbd_input);
        TIMER_CALLBACK_MEMBER(teletype_callback);

        DECLARE_DEVICE_IMAGE_LOAD_MEMBER( patinho_tape );
        void load_tape(const char* name);
        void load_raw_data(const char* name, unsigned int start_address, unsigned int data_length);
        void update_panel(UINT8 ACC, UINT8 opcode, UINT8 mem_data, UINT16 mem_addr, UINT16 PC, UINT8 FLAGS, UINT16 RC);
        virtual void machine_start() override;

        required_device<patinho_feio_cpu_device> m_maincpu;
        required_device<teleprinter_device> m_decwriter;
        required_device<teleprinter_device> m_tty;
private:
        UINT8* paper_tape_data;
        UINT32 paper_tape_length;
        UINT32 paper_tape_address;

        emu_timer *m_decwriter_timer;
        emu_timer *m_teletype_timer;
	output_manager *m_out;
	UINT8 m_prev_ACC;
        UINT8 m_prev_opcode;
        UINT8 m_prev_mem_data;
        UINT16 m_prev_mem_addr;
        UINT16 m_prev_PC;
        UINT8 m_prev_FLAGS;
        UINT16 m_prev_RC;
};
