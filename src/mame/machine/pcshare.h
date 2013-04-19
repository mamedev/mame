class pcat_base_state : public driver_device
{
public:
	pcat_base_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu") { }

	IRQ_CALLBACK_MEMBER(irq_callback);

	required_device<cpu_device> m_maincpu;
};

ADDRESS_MAP_EXTERN(pcat32_io_common, 32);
MACHINE_CONFIG_EXTERN(pcat_common);
