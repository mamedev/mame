/*********************************************************************

    beta.h

    Implementation of Beta disk drive support for Spectrum and clones

    04/05/2008 Created by Miodrag Milanovic

*********************************************************************/
#include "emu.h"
#include "imagedev/flopdrv.h"
#include "formats/trd_dsk.h"
#include "machine/wd17xx.h"
#include "machine/beta.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct beta_disk_state
{
	UINT8 betadisk_status;
	UINT8 betadisk_active;

	device_t *wd179x;
};


/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/
INLINE beta_disk_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == BETA_DISK);

	return (beta_disk_state *)downcast<beta_disk_device *>(device)->token();
}


int betadisk_is_active(device_t *device)
{
	beta_disk_state *beta = get_safe_token(device);

	return beta->betadisk_active;
}

void betadisk_enable(device_t *device)
{
	beta_disk_state *beta = get_safe_token(device);

	beta->betadisk_active = 1;
}

void betadisk_disable(device_t *device)
{
	beta_disk_state *beta = get_safe_token(device);

	beta->betadisk_active = 0;
}

void betadisk_clear_status(device_t *device)
{
	beta_disk_state *beta = get_safe_token(device);

	beta->betadisk_status = 0;
}

static WRITE_LINE_DEVICE_HANDLER( betadisk_wd179x_intrq_w )
{
	beta_disk_state *beta = get_safe_token(device->owner());

	if (state)
		beta->betadisk_status |= (1<<7);
	else
		beta->betadisk_status &=~(1<<7);
}

static WRITE_LINE_DEVICE_HANDLER( betadisk_wd179x_drq_w )
{
	beta_disk_state *beta = get_safe_token(device->owner());

	if (state)
		beta->betadisk_status |= (1<<6);
	else
		beta->betadisk_status &=~(1<<6);
}

static const wd17xx_interface beta_wd17xx_interface =
{
	DEVCB_NULL,
	DEVCB_LINE(betadisk_wd179x_intrq_w),
	DEVCB_LINE(betadisk_wd179x_drq_w),
	{FLOPPY_0, FLOPPY_1, FLOPPY_2, FLOPPY_3}
};

READ8_DEVICE_HANDLER(betadisk_status_r)
{
	beta_disk_state *beta = get_safe_token(device);

	if (beta->betadisk_active==1) {
		return wd17xx_status_r(beta->wd179x, space, offset);
	} else {
		return 0xff;
	}
}

READ8_DEVICE_HANDLER(betadisk_track_r)
{
	beta_disk_state *beta = get_safe_token(device);

	if (beta->betadisk_active==1) {
		return wd17xx_track_r(beta->wd179x, space, offset);
	} else {
		return 0xff;
	}
}

READ8_DEVICE_HANDLER(betadisk_sector_r)
{
	beta_disk_state *beta = get_safe_token(device);

	if (beta->betadisk_active==1) {
		return wd17xx_sector_r(beta->wd179x, space, offset);
	} else {
		return 0xff;
	}
}

READ8_DEVICE_HANDLER(betadisk_data_r)
{

	beta_disk_state *beta = get_safe_token(device);

	if (beta->betadisk_active==1) {
		return wd17xx_data_r(beta->wd179x, space, offset);
	} else {
		return 0xff;
	}
}

READ8_DEVICE_HANDLER(betadisk_state_r)
{
	beta_disk_state *beta = get_safe_token(device);

	if (beta->betadisk_active==1) {
		return beta->betadisk_status;
	} else {
		return 0xff;
	}
}

WRITE8_DEVICE_HANDLER(betadisk_param_w)
{
	beta_disk_state *beta = get_safe_token(device);

	if (beta->betadisk_active==1) {
		wd17xx_set_drive(beta->wd179x, data & 3);
		wd17xx_set_side (beta->wd179x,(data & 0x10) ? 0 : 1 );
		wd17xx_dden_w(beta->wd179x, !BIT(data, 5));
		if ((data & 0x04) == 0) // reset
		{
			wd17xx_reset(beta->wd179x);
		}
		beta->betadisk_status = (data & 0x3f) | beta->betadisk_status;
	}
}

WRITE8_DEVICE_HANDLER(betadisk_command_w)
{
	beta_disk_state *beta = get_safe_token(device);

	if (beta->betadisk_active==1) {
		wd17xx_command_w(beta->wd179x, space, offset, data);
	}
}

WRITE8_DEVICE_HANDLER(betadisk_track_w)
{
	beta_disk_state *beta = get_safe_token(device);

	if (beta->betadisk_active==1) {
		wd17xx_track_w(beta->wd179x, space, offset, data);
	}
}

WRITE8_DEVICE_HANDLER(betadisk_sector_w)
{
	beta_disk_state *beta = get_safe_token(device);

	if (beta->betadisk_active==1) {
		wd17xx_sector_w(beta->wd179x, space, offset, data);
	}
}

WRITE8_DEVICE_HANDLER(betadisk_data_w)
{
	beta_disk_state *beta = get_safe_token(device);

	if (beta->betadisk_active==1) {
		wd17xx_data_w(beta->wd179x, space, offset, data);
	}
}

static const floppy_interface beta_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(trd),
	NULL,
	NULL
};

static MACHINE_CONFIG_FRAGMENT( beta_disk )
	MCFG_WD2793_ADD("wd179x", beta_wd17xx_interface ) // KR1818VG93
	MCFG_LEGACY_FLOPPY_4_DRIVES_ADD(beta_floppy_interface)
MACHINE_CONFIG_END

ROM_START( beta_disk )
	ROM_REGION( 0x60000, "beta", 0 )
	ROM_LOAD( "trd501.rom", 	0x00000, 0x4000, CRC(3e3cdd4c) SHA1(8303ba0cc79daa6c04cd1e6ce27e8b6886a3f0de))

	ROM_LOAD( "trd503zxvgs.rom",0x00000, 0x4000, CRC(b90ee684) SHA1(c78e1b36812fb2b3e7a62e66049c850d27de74a6))
	ROM_LOAD( "trd503m.rom",	0x00000, 0x4000, CRC(2f97fe06) SHA1(6b9decc07d5d7322c6c283f5fe950f530a42d70d))
	ROM_LOAD( "trd503[a].rom",	0x00000, 0x4000, CRC(c43d717f) SHA1(0a74bd34538a03d0e1d214b425d95c14ad10c8c4))
	ROM_LOAD( "trd503[a2].rom", 0x00000, 0x4000, CRC(121889b0) SHA1(c9d69cf3a0219f6e37e7eb5046961fa8fa8eb2c6))
	ROM_LOAD( "trd503[a3].rom", 0x00000, 0x4000, CRC(1c5a25b1) SHA1(4a6fbabd82a6a8f3986ac77e7157d1a2551ed40d))
	ROM_LOAD( "trd503beta3.rom",0x00000, 0x4000, CRC(561662f2) SHA1(4cea8a361eb848cf0ad59605c83d57c30521cfd5))
	ROM_LOAD( "trd503beta4.rom",0x00000, 0x4000, CRC(23dbc387) SHA1(74b80d3f07dc95e9f09b83c3df4724f063f47116))
    ROM_LOAD( "trd503all.rom",  0x00000, 0x4000, CRC(4c0187ab) SHA1(21f611e5e0bbf5d6450606e30b470f7c2c9640f1))
    ROM_LOAD( "trd503aut.rom",  0x00000, 0x4000, CRC(7ff90178) SHA1(2261144bdb5046cb8013f744a5b0721be8577dc7))
    ROM_LOAD( "trd503ext.rom",  0x00000, 0x4000, CRC(abb139e7) SHA1(3c1736857655a48375d30b3865963090ba635635))
    ROM_LOAD( "trd503kay.rom",  0x00000, 0x4000, CRC(77baccbb) SHA1(9c5fb89b57643f723246453809dad3f669bf59a3))
    ROM_LOAD( "trd503xbios.rom",0x00000, 0x4000, CRC(8be427cc) SHA1(78f423fd200e720aa6b857f00969fa1f8c7da20e))
	ROM_LOAD( "trd503[a4].rom", 0x00000, 0x4000, CRC(c2387608) SHA1(93e0b92a3f38b59cc006d22f9c9299f5377b15e5))


	ROM_LOAD( "trd504.rom", 	0x00000, 0x4000, CRC(ba310874) SHA1(05e55e37df8eee6c68601ba9cf6c92195852ce3f))
	ROM_LOAD( "trd504s.rom",	0x00000, 0x4000, CRC(c5ca0423) SHA1(cb2da2c4575bc54454e4f97073c5bb37505487fe))
	ROM_LOAD( "trd504s2.rom",   0x00000, 0x4000, CRC(1e9b59aa) SHA1(d6eb0eeb429c70d1f8f9a23ca4e2feb5f73d3120))
	ROM_LOAD( "trd504s3.rom",   0x00000, 0x4000, CRC(1fe1d003) SHA1(33703e97cc93b7edfcc0334b64233cf81b7930db))


	ROM_LOAD( "trd504tm.rom",	0x00000, 0x4000, CRC(2334b8c6) SHA1(2fd8944e067140324e998b59aab766f16eb5c7fb))
	ROM_LOAD( "trd504t.rom",	0x00000, 0x4000, CRC(e212d1e0) SHA1(745e9caf576e64a5386ad845256d28593d34cc40))
	ROM_LOAD( "trd504tb2.rom",  0x00000, 0x4000, CRC(8d943e6b) SHA1(bf6ab329791258f02c5f695b0c7448998b709435))
	ROM_LOAD( "trd504s4.rom",	0x00000, 0x4000, CRC(522ebbd6) SHA1(eede6a988ed453e506b27a8d09470e0a8d97d1aa))
	ROM_LOAD( "trd504f.rom",	0x00000, 0x4000, CRC(ab3100d8) SHA1(33646525ae1c579cddc0dd84d3218095950965b8))
	ROM_LOAD( "trd504em.rom",	0x00000, 0x4000, CRC(0d3f8b43) SHA1(d6783983a16b17b79337c22e9460e1cfde3744ae))
	ROM_LOAD( "trd504-1.rom",	0x00000, 0x4000, CRC(da170c65) SHA1(80f0fe79cbf393ac91ff971d31ff9eab0c9a959e))
	ROM_LOAD( "trd504m.rom",	0x00000, 0x4000, CRC(2f2cb630) SHA1(40b1f87f8be4e09630d7eb7c14561dde0b85c0c9))
	ROM_LOAD( "trd504em[a].rom",0x00000, 0x4000, CRC(fcbf11e8) SHA1(8050d371d7049e0c3e946964643396a39aa5ab0f))
	ROM_LOAD( "trd5043.rom",    0x00000, 0x4000, CRC(165d5ef8) SHA1(99d25234154e4a8b3ad5e06f260b6c41c253b333))

	ROM_LOAD( "trd505.rom", 	0x00000, 0x4000, CRC(fdff3810) SHA1(0a0e284d4764a542aa3e5d7c43d0291036e16c35))
	ROM_LOAD( "trd505[a].rom",	0x00000, 0x4000, CRC(03b76c8f) SHA1(ef2a07767d3b229aa4573dcfee905156a83bc32d))
	ROM_LOAD( "trd505h.rom",	0x00000, 0x4000, CRC(9ba15549) SHA1(5908784cdfb782066bde08f186f0bbb6f6b80545))
	ROM_LOAD( "trd505d.rom",	0x00000, 0x4000, CRC(31e4be08) SHA1(dd08bdea8b5caa35569f49770e380d16bb37502b))
    ROM_LOAD( "trd505[a2].rom", 0x00000, 0x4000, CRC(a102e726) SHA1(a3f0ef7b7d8b3022f306be8d9e8a51ae699097df))

	ROM_LOAD( "trd56661.rom",	0x00000, 0x4000, CRC(8528c789) SHA1(1332a01137bd537fee696ba7adddc0a15b3237c4))
	ROM_LOAD( "trd5666hte.rom", 0x00000, 0x4000, CRC(03841161) SHA1(4e523768231130947a81247e116fc049bd6da963))
	ROM_LOAD( "trd5613.rom",    0x00000, 0x4000, CRC(d66cda49) SHA1(dac3c8396fc4f85b5673076cc647afbe439ad17a))

	ROM_LOAD( "trd512.rom", 	0x00000, 0x4000, CRC(b615d6c4) SHA1(4abb90243f66df19caa84f82d0b880a6ce344eca))
	ROM_LOAD( "trd512f.rom",	0x00000, 0x4000, CRC(edb74f8c) SHA1(c5b2cd18926dde4a68bcd8c360ba33d6c11b8801))

	ROM_LOAD( "trd513f.rom",	0x00000, 0x4000, CRC(6b1c17f3) SHA1(5c14c0c389e29521cb59672d93d3e8a74f0184cc))
	ROM_LOAD( "trd513fm.rom",	0x00000, 0x4000, CRC(bad0c0a0) SHA1(da88e756302f83991774730012601ebf3c61aed9))
    ROM_LOAD( "trd513p.rom",    0x00000, 0x4000, CRC(eb3196fe) SHA1(7c85b9b562d288155677f20f5e41e679c073379b))

	ROM_LOAD( "trd604.rom", 	0x00000, 0x4000, CRC(d8882a8c) SHA1(282eb7bc819aad2a12fd954e76f7838a4e1a7929))
	ROM_LOAD( "trd604m.rom",	0x00000, 0x4000, CRC(e73394cb) SHA1(5b1336e8534513df0a71f8fedca38bc67174f03c))

	ROM_LOAD( "trd605r.rom",	0x00000, 0x4000, CRC(f8816a47) SHA1(98c9b63835e559f38f6a8d1344992f02cb899add))
	ROM_LOAD( "trd605e.rom",	0x00000, 0x4000, CRC(56d3c2db) SHA1(adc21492d216da1aca4119972f2110a23fdd23a7))
	ROM_LOAD( "trd605h.rom",    0x00000, 0x4000, CRC(53bb1b4a) SHA1(fc899901fd59c84c9aa6c402b67d76b225ab8e5b))
    ROM_LOAD( "trd605e-2.rom",  0x00000, 0x4000, CRC(a064b7f2) SHA1(46ac75e88375d313fdd5e873800bf82c179eb226))
    ROM_LOAD( "trd605e-3.rom",	0x00000, 0x4000, CRC(cff3d06b) SHA1(0fca9c61cb94a4cab78118c66ba54ce6bc6dcf65))

	ROM_LOAD( "trd606h.rom",	0x00000, 0x4000, CRC(6b44fdd7) SHA1(4ca7ea2bc0e62c98c9ba86a304aee3cbf88d6b09))

	ROM_LOAD( "trd607m.rom",	0x00000, 0x4000, CRC(5a062f03) SHA1(bbd68481c76e8a3f2f26847faa72c42df013b617))

	ROM_LOAD( "trd608.rom", 	0x00000, 0x4000, CRC(5c998d53) SHA1(c65a972e75e627336c44ea184cb9ae81dfa2069b))
	ROM_LOAD( "trd608-2.rom",	0x00000, 0x4000, CRC(3541b280) SHA1(2b93c1887f485639ec15860b5503dc7efb6d31d6))
	ROM_LOAD( "trd608-3.rom",	0x00000, 0x4000, CRC(d3e91d69) SHA1(5e367d3601d07bd2af1d6255561a5983c0e9fb5d))

	ROM_LOAD( "trd609.rom", 	0x00000, 0x4000, CRC(91028924) SHA1(fa5433f73aab93c58c891ceee2cdfed0a979dfbb))
    ROM_LOAD( "trd609e.rom",    0x00000, 0x4000, CRC(46312c8c) SHA1(0ce69fe76e17a875c287e3725f6209f1c59d8bd9))

	ROM_LOAD( "trd610e.rom",	0x00000, 0x4000, CRC(95395ca4) SHA1(8cda2efbf2360cd675ea8b04b43cf1746fb41f35))

	ROM_LOAD( "trd701.rom",		0x00000, 0x4000, CRC(47f39c0d) SHA1(53d8b8959c321618caaef5aeb270e530e8f8ada3))

	//Default for now
	ROM_LOAD( "trd503.rom", 	0x00000, 0x4000, CRC(10751aba) SHA1(21695e3f2a8f796386ce66eea8a246b0ac44810c))
ROM_END


/*-------------------------------------------------
    DEVICE_START( beta_disk )
-------------------------------------------------*/

static DEVICE_START( beta_disk )
{
	beta_disk_state *beta = get_safe_token(device);
	astring tempstring;

	/* validate arguments */
	assert(device->tag() != NULL);

	/* find our WD179x */
	tempstring.printf("%s:%s", device->tag(), "wd179x");
	beta->wd179x = device->machine().device(tempstring);
}

/*-------------------------------------------------
    DEVICE_RESET( beta_disk )
-------------------------------------------------*/

static DEVICE_RESET( beta_disk )
{
}

const device_type BETA_DISK = &device_creator<beta_disk_device>;

beta_disk_device::beta_disk_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, BETA_DISK, "Beta Disk Interface", tag, owner, clock)
{
	m_token = global_alloc_clear(beta_disk_state);
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void beta_disk_device::device_config_complete()
{
	m_shortname = "betadisk";
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void beta_disk_device::device_start()
{
	DEVICE_START_NAME( beta_disk )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void beta_disk_device::device_reset()
{
	DEVICE_RESET_NAME( beta_disk )(this);
}

//-------------------------------------------------
//  device_mconfig_additions - return a pointer to
//  the device's machine fragment
//-------------------------------------------------

machine_config_constructor beta_disk_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( beta_disk  );
}

//-------------------------------------------------
//  device_rom_region - return a pointer to the
//  the device's ROM definitions
//-------------------------------------------------

const rom_entry *beta_disk_device::device_rom_region() const
{
	return ROM_NAME(beta_disk );
}


