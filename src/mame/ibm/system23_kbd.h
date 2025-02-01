#include "cpu/mcs48/mcs48.h"

class system23_kbd_device : public device_t
{
	public:
		system23_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
		void reset_w(int state);
		uint8_t read_keyboard();
		void t0_w(int state);

		auto scancode_export() {return m_bus_write.bind();};

    protected:
        virtual void device_start() override ATTR_COLD;
		virtual void device_reset() override ATTR_COLD;
		virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
		virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	private:
		required_device<i8048_device> m_mcu;

		devcb_write_line m_bus_write;

		uint8_t m_bus;

		int m_reset;
		int m_t0, m_t1;

		void bus_w(uint8_t data);
		int t0_r();
		int t1_r();
		void data_strobe(uint8_t data);

};

DECLARE_DEVICE_TYPE(SYSTEM23_KEYBOARD, system23_kbd_device)
