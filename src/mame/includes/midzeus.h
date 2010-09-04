/*************************************************************************

    Driver for Midway Zeus games

**************************************************************************/

#define MIDZEUS_VIDEO_CLOCK		XTAL_66_6667MHz

class midzeus_state : public driver_device
{
public:
	midzeus_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  m_nvram(*this, "nvram") { }

	required_shared_ptr<UINT32>	m_nvram;
};


/*----------- defined in video/midzeus.c -----------*/

extern UINT32 *zeusbase;

VIDEO_START( midzeus );
VIDEO_UPDATE( midzeus );

READ32_HANDLER( zeus_r );
WRITE32_HANDLER( zeus_w );

/*----------- defined in video/midzeus2.c -----------*/

VIDEO_START( midzeus2 );
VIDEO_UPDATE( midzeus2 );

READ32_HANDLER( zeus2_r );
WRITE32_HANDLER( zeus2_w );

