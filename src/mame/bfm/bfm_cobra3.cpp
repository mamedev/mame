// license:BSD-3-Clause
// copyright-holders:David Haywood, James Wallace, blueonesarefaster

/* Bellfruit SWP (Skill With Prizes) Video hardware
    aka Cobra 3

   TODO: MPEG decoding currently not implemented
*/



#include "emu.h"
#include "bus/nscsi/cd.h"
#include "machine/68340.h"
#include "machine/meters.h"
#include "machine/ncr5380.h"
#include "machine/nscsi_bus.h"
#include "machine/nvram.h"
#include "video/ramdac.h"
#include "machine/scc66470.h"
#include "sound/ymz280b.h"
#include "screen.h"
#include "speaker.h"


namespace {

class bfm_cobra3_state : public driver_device
{
public:
	bfm_cobra3_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_cpuregion(*this, "maincpu"),
			m_nvram(*this, "nvram"),
			m_ymz(*this, "ymz280b"),
			m_palette(*this, "palette"),
			m_ramdac(*this, "ramdac"),
			m_scc66470(*this, "scc66470"),
			// m_strobein(*this, "STROBE%u", 0),
			m_meters(*this, "meters"),
			m_lamps(*this, "lamp%u", 0U),
			m_scsibus(*this, "scsi"),
			m_scsic(*this, "scsi:6:ncr5380")
	{ }

	std::unique_ptr<uint16_t[]> m_mainram;

	uint16_t bfm_cobra3_mem_r(offs_t offset, uint16_t mem_mask = ~0);
	void bfm_cobra3_mem_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void bfm_cobra3(machine_config &config);
	void ramdac_map(address_map &map);
	void bfm_cobra3_map(address_map &map) ATTR_COLD;
protected:

	// devices
	required_device<m68340_cpu_device> m_maincpu;
	required_region_ptr<uint16_t> m_cpuregion;
	required_device<nvram_device> m_nvram;
	required_device<ymz280b_device> m_ymz;
	required_device<palette_device> m_palette;
	required_device<ramdac_device> m_ramdac;
	required_device<scc66470_device> m_scc66470;
	// required_ioport_array<4> m_strobein;
	optional_device<meters_device> m_meters;
	output_finder<256> m_lamps;
	required_device<nscsi_bus_device> m_scsibus;
	required_device<ncr5380_device> m_scsic;


	virtual void machine_start() override ATTR_COLD;
	static void scsi_devices(device_slot_interface &device);
	void dma1_drq(int state);
	void scc66470_map(address_map &map);
	void scc66470_irq(int state);

};

uint16_t bfm_cobra3_state::bfm_cobra3_mem_r(offs_t offset, uint16_t mem_mask)
{
	int cs = m_maincpu->get_cs(offset * 2);

	switch ( cs )
	{
		case 1: // ROM
			return m_cpuregion[offset & 0x7ffff];
		case 2:// (NV)RAM
			return m_mainram[offset & 0x1fff];

		case 3: // I/O
			{
				offset &= 0x7ff;
				offs_t cs_addr_8_11 = (offset * 2) & 0xf00;

				switch(cs_addr_8_11)
				{
					case 0x300: //YMZ stereo sound accesses
						if(ACCESSING_BITS_0_7)
						{
							return m_ymz->read(offset & 1);
						}
						break;

					case 0x400:
						//input reads, haven't got far enough to trigger any
						logerror("%s maincpu read access offset %08x mem_mask %08x cs %d\n", machine().describe_context(), offset*4, mem_mask, cs);
						break;

					case 0x500: //SCSI DMA
						if(ACCESSING_BITS_8_15)
						{
							return m_scsic->dma_r() << 8;
						}
						break;

					case 0x600: //Looks like the RAMDAC hookup
						if(ACCESSING_BITS_0_7)
						{
							if((offset & 7) == 1)
							{
								return m_ramdac->pal_r();
							}
						}
						break;

					default:
						logerror("%s maincpu read access offset %08x mem_mask %08x cs %d\n", machine().describe_context(), offset*4, mem_mask, cs);
						break;
				}
			}
			break;

		case 4: // SCSI controller
				if(ACCESSING_BITS_8_15)
				{
					return(m_scsic->read(offset & 0x0f) << 8);
				}
				break;

		default:
				logerror("%s maincpu read access offset %08x mem_mask %08x cs %d\n", machine().describe_context(), offset*4, mem_mask, cs);
	}

	return 0x0000;
}

void bfm_cobra3_state::bfm_cobra3_mem_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	int cs = m_maincpu->get_cs(offset * 2);

	switch (cs)
	{
		case 1:// ROM, shouldn't write here?
			logerror("%sx maincpu write access(1) offset %08x data %08x mem_mask %08x cs %d\n", machine().describe_context(), offset*4, data, mem_mask, cs);
			break;

		case 2:// (NV)RAM
			COMBINE_DATA(&m_mainram[offset & 0x1fff]);
			break;

		case 3: // I/O
			{
				offset &= 0x7ff;
				offs_t cs_addr_8_11 = (offset * 2) & 0xf00;
				switch(cs_addr_8_11)
				{
					case 0x000:
						logerror("%s maincpu write access lamp drive io latch offset %08x mem_mask %08x cs %d\n", machine().describe_context(), offset*4, mem_mask, cs);
						// lamps;
						break;

					case 0x100:
						logerror("%s maincpu write access lockout latch offset %08x mem_mask %08x cs %d\n", machine().describe_context(), offset*4, mem_mask, cs);
						if(ACCESSING_BITS_8_15)
						{
							// coin lockout and optional vfd (debug only?)
						}
						break;

					case 0x200:
						logerror("%s maincpu write access io latch offset %08x mem_mask %08x cs %d\n", machine().describe_context(), offset*4, mem_mask, cs);
						// volume, watchdog and other stuff ? That's where it would be elsewhere
						break;

					case 0x300:
						if(ACCESSING_BITS_0_7)
						{
							m_ymz->write(offset & 1, data);
						}
						break;

					case 0x500: // SCSI DMA
						if(ACCESSING_BITS_8_15)
						{
							m_scsic->dma_w(data >> 8);
						}
						break;

					case 0x700: // RAMDAC for palettes
						if(ACCESSING_BITS_0_7)
						{
							offset &= 7;
							switch (offset)
							{
							case 0:
								m_ramdac->index_w(data);
								break;
							case 1:
								m_ramdac->pal_w(data);
								break;
							case 2:
								m_ramdac->mask_w(data);
								break;
							case 3:
								m_ramdac->index_r_w(data);
								break;
							}
						}
						break;

					default:
						// coin divert, hoppers, note validator must be somewhere
						logerror("%s maincpu write access(3) offset %08x data %08x mem_mask %08x cs %d\n", machine().describe_context(), offset*4, data, mem_mask, cs);
						break;
				}
			}
			break;

		case 4: // SCSI controller
			offset &= 0x0f;
			if(ACCESSING_BITS_8_15)
			{
				m_scsic->write(offset, data >> 8);
			}
			break;

		default:
			logerror("%s maincpu write access(0) offset %08x data %08x mem_mask %08x cs %d\n", machine().describe_context(), offset*4, data, mem_mask, cs);
			break;
	}
}


void bfm_cobra3_state::bfm_cobra3_map(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(bfm_cobra3_state::bfm_cobra3_mem_r), FUNC(bfm_cobra3_state::bfm_cobra3_mem_w));
	map(0x00800000, 0x009fffff).m(m_scc66470, FUNC(scc66470_device::map)).cswidth(16);
}

void bfm_cobra3_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

void bfm_cobra3_state::scc66470_map(address_map &map)
{
	map(0x00000, 0x7ffff).ram();
}

static INPUT_PORTS_START( bfm_cobra3 )
	// TODO: Fix inputs
	PORT_START("STROBE0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) PORT_IMPULSE(3)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_COIN4 ) PORT_IMPULSE(3)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN5 ) PORT_IMPULSE(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Green Test")
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE ) PORT_NAME("Red Test")
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("STROBE2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON5 ) // collect?
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON6 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON7 )

	PORT_START("STROBE3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON8 ) // service?
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON9 )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON10 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON11 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON12 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON13 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_BUTTON14 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_BUTTON15 )

INPUT_PORTS_END


void bfm_cobra3_state::machine_start()
{
	m_mainram = make_unique_clear<uint16_t[]>((1024 * 16) / 2);
	m_nvram->set_base(m_mainram.get(), 1024 * 16);
}


static void cobra_scsi_devices(device_slot_interface &device)
{
	device.option_add("cdrom", NSCSI_CDROM);
	device.option_add_internal("ncr53c80", NCR53C80);
}

void bfm_cobra3_state::dma1_drq(int state)
{
	// Triggers, but seems not to do anything, may be CPU bug related.
}

void bfm_cobra3_state::scc66470_irq(int state)
{
	m_maincpu->set_input_line(5, !state);
}

uint32_t bfm_cobra3_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if(m_scc66470->display_enabled())
	{
		if(cliprect.min_y == cliprect.max_y)
		{
			uint32_t *dest = &bitmap.pix(cliprect.min_y);
			uint8_t buffer[768];
			uint8_t *src = buffer;
			m_scc66470->line(cliprect.min_y, buffer, sizeof(buffer));

			src = buffer;

			if(*src == 254)
			{
				// Other implementations suggest leaving transparency / MPEG border colour here to ease blending
				src += 32;
			}
			else
			{
				dest = std::fill_n(dest, 32, m_palette->pen(*src));
				src += 32;
			}

			/* mpeg video has significant overscan, 4 lines either side.

			Just crop it out to fit, presume the chip does this IRL */

			for(int x = 0 ; x < 352 ; x++)
			{
				if(*src == 254)
				{
					*dest++ = 0; // Will allow MPEG to be drawn i.e. transparent
					src++;
				}
				else
				{
					*dest++ = m_palette->pen(*src++);
				}

				if(*src == 254)
				{
					*dest++ = 0; // This should be mpeg video pixel i.e. transparent
					src++;
				}
				else
				{
					*dest++ = m_palette->pen(*src++);
				}
			}
			if(*src == 254)
			{
				// finally 32 pixels of mpeg border colour when it's mixed in
			}
			else
			{
				std::fill_n(dest, 32, m_palette->pen(*src));
			}
		}
	}
	return 0;
}


void bfm_cobra3_state::bfm_cobra3(machine_config &config)
{
	M68340(config, m_maincpu, 16000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &bfm_cobra3_state::bfm_cobra3_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(15000000, 960, 0, 768, 312, 32, 312);
	screen.set_video_attributes(VIDEO_UPDATE_SCANLINE);
	screen.set_screen_update(FUNC(bfm_cobra3_state::screen_update));

	PALETTE(config, m_palette).set_entries(256);

	RAMDAC(config, m_ramdac, 0, m_palette); // MUSIC Semiconductor TR9C1710 RAMDAC
	m_ramdac->set_addrmap(0, &bfm_cobra3_state::ramdac_map);
	m_ramdac->set_split_read(1);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	YMZ280B(config, m_ymz, 16.9344_MHz_XTAL);
	m_ymz->add_route(0, "lspeaker", 1.0);
	m_ymz->add_route(1, "rspeaker", 1.0);

	SCC66470(config,m_scc66470,30000000);
	m_scc66470->set_addrmap(0, &bfm_cobra3_state::scc66470_map);
	m_scc66470->set_screen("screen");
	m_scc66470->irq().set(FUNC(bfm_cobra3_state::scc66470_irq));

	NSCSI_BUS(config, m_scsibus);

	NSCSI_CONNECTOR(config, "scsi:2", cobra_scsi_devices, "cdrom");
	NSCSI_CONNECTOR(config, "scsi:6").option_set("ncr5380", NCR5380).machine_config(
		[this] (device_t *device)
		{
			ncr53c80_device &adapter = downcast<ncr53c80_device &>(*device);
			adapter.drq_handler().set(*this, FUNC(bfm_cobra3_state::dma1_drq));
		});
}

ROM_START( c3_rtime )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "95400009.bin", 0x00001, 0x080000, CRC(a5e0a5ca) SHA1(e7063ddfb436152f15267fde2aa7695c8a262191) )
	ROM_LOAD16_BYTE( "95400010.bin", 0x00000, 0x080000, CRC(03fd5f72) SHA1(379cfc4ef5087f24989bc1f2246b6056e33fd472) )

	ROM_REGION( 0x100000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "95400063.lhs", 0x00001, 0x080000, CRC(eecb5f3b) SHA1(a1c6ad61a65c5361c38aaae2a064983a978c45ea) )
	ROM_LOAD16_BYTE( "95400064.rhs", 0x00000, 0x080000, CRC(251689f5) SHA1(4589a409c6b0f2869f99a08df8d76223e54d5b3c) )
	ROM_LOAD16_BYTE( "95401063.lhs", 0x00001, 0x080000, CRC(ea98c159) SHA1(6f665d80b71af57b31194fdc981707822e62053e) )
	ROM_LOAD16_BYTE( "95401064.rhs", 0x00000, 0x080000, CRC(bc125897) SHA1(a83fdb54349d3ea5d183754bf4b9fee1f0b73be3) )
	ROM_LOAD16_BYTE( "radtimes.lhs", 0x00001, 0x080000, CRC(c6574297) SHA1(bd9744c4b08f9ae35fe1523ebcd68c52a36a32e0) )
	ROM_LOAD16_BYTE( "radtimes.rhs", 0x00000, 0x080000, CRC(ed2c24f0) SHA1(5f06b2de7e2b2dccee7763ea0938849d67256ff2) )
	ROM_LOAD16_BYTE( "rt017.lhs", 0x00001, 0x080000, CRC(d2272c39) SHA1(f583fe39c153dca2e86e875ca39056a8756e0d2c) )
	ROM_LOAD16_BYTE( "rt018.rhs", 0x00000, 0x080000, CRC(52999d03) SHA1(21d1e9034a26f6f73109e9e83272dcff104993e5) )
	ROM_LOAD16_BYTE( "rtimesp1", 0x00001, 0x080000, CRC(f856d377) SHA1(a9fac7e2188bbd087f70c1c00cbf790bc52d573b) )
	ROM_LOAD16_BYTE( "rtimesp2", 0x00000, 0x080000, CRC(130d0864) SHA1(034d6c4fdec3acd4329d16315aeac43b1f1a5e91) )

	ROM_REGION( 0x1000000, "ymz280b", 0 )
	ROM_LOAD( "95004056.bin", 0x000000, 0x080000, CRC(24e8f9fb) SHA1(0d484a8f368b0f2140f148a1dc84db85a100af38) )
	ROM_LOAD( "95004057.bin", 0x080000, 0x080000, CRC(f73c92d6) SHA1(08c7db2baccb703f99efb81f618719a7789ca564) )

	DISK_REGION("scsi:2:cdrom")
	DISK_IMAGE_READONLY( "95100302", 0, SHA1(20accfe236a0c85108cd2a205399ed8959f1a638) )
ROM_END

ROM_START( c3_telly )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "95400021.p1",  0x00001, 0x080000, CRC(5c969746) SHA1(7458c613d7a3e7cf6a21e55f74dcdc052404f29c) )
	ROM_LOAD16_BYTE( "95400022.p2",  0x00000, 0x080000, CRC(fa1fdb7b) SHA1(eff87c197a62dba49d95810e8669026db2edb187) )

	ROM_REGION( 0x100000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "95401021.p1",  0x00001, 0x080000, CRC(24a334d3) SHA1(672f16cbd2ddf627213de71024b6fbaa28f526a5) )
	ROM_LOAD16_BYTE( "95401022.p2",  0x00000, 0x080000, CRC(90af3767) SHA1(e529ad7eef5e6d2a6951d46e77aaad2087890445) )
	ROM_LOAD16_BYTE( "tadd13lh",     0x00001, 0x080000, CRC(2d6ed08c) SHA1(efa39b9ff5605c2e29971fb5e874c9a0c178b1f0) )
	ROM_LOAD16_BYTE( "tadd14rh",     0x00000, 0x080000, CRC(26dd6ed6) SHA1(553f29017494b6f7ecc98940d527f498316ea55e) )
	ROM_LOAD16_BYTE( "telad.tl",     0x00001, 0x080000, CRC(e6906027) SHA1(20ca64417ea3795dc26adfea717cb3d724019c34) )
	ROM_LOAD16_BYTE( "telad.tr",     0x00000, 0x080000, CRC(38dbee05) SHA1(ee33cdaa7f817beb49a3cff49a5493a50d8d4504) )
	ROM_LOAD16_BYTE( "tasndl",       0x00001, 0x080000, CRC(3f0b9d2b) SHA1(6db3451c26a3e673204c316403e0bb7127191a1f) )
	ROM_LOAD16_BYTE( "tasndr",       0x00000, 0x080000, CRC(2dd9ebcf) SHA1(4d118d37e18266f82fb2acb37f5fd106e0f25a1f) )

	ROM_REGION( 0x1000000, "ymz280b", ROMREGION_ERASE00 )
	ROM_LOAD( "telsndl", 0x0000, 0x080000, CRC(74996fbd) SHA1(90e46130dccf47be1fcfaf549e548cdd4883e59d) )

	DISK_REGION("scsi:2:cdrom")
	DISK_IMAGE_READONLY( "95100300", 0, SHA1(98905cbff24c576c58210d1d003f710fa7064762) )
ROM_END


ROM_START( c3_tellyns )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "95400023.lhs", 0x00001, 0x080000, CRC(b79279b8) SHA1(010edf0c299b0b01ab43f52dce540ff0847fb4c5) )
	ROM_LOAD16_BYTE( "95400024.rhs", 0x00000, 0x080000, CRC(835d25fd) SHA1(6d780332f6016d6e1404922e0ac439a499211be3) )

	ROM_REGION( 0x100000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "95401023.lhs", 0x00001, 0x080000, CRC(85b95b56) SHA1(106e617fc92f95a6b3769db1fd4e5ab47c752c08) )
	ROM_LOAD16_BYTE( "95401024.rhs", 0x00000, 0x080000, CRC(835d25fd) SHA1(6d780332f6016d6e1404922e0ac439a499211be3) )

	ROM_REGION( 0x1000000, "ymz280b", ROMREGION_ERASE00 )
	ROM_LOAD( "telsndl", 0x0000, 0x080000, CRC(74996fbd) SHA1(90e46130dccf47be1fcfaf549e548cdd4883e59d) )

	DISK_REGION("scsi:2:cdrom")
	DISK_IMAGE_READONLY( "95100301", 0, SHA1(dbce040a6fb7916a240d24e2207cf6e1b3f572e7) )
ROM_END

ROM_START( c3_totp )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "95400101.lo", 0x00001, 0x080000, CRC(c95164c7) SHA1(7b2fada6a3208666219a53cba08f7acad015763d) )
	ROM_LOAD16_BYTE( "95400102.hi", 0x00000, 0x080000, CRC(5ebba159) SHA1(34bcf48140261cd87d81a32581e965d722f42f71) )

	ROM_REGION( 0x100000, "altrevs", 0 )
	ROM_LOAD16_BYTE( "95401101.lo", 0x00001, 0x080000, CRC(97d2d90a) SHA1(d4a2afd3cc551986e76f107beb66e8c660a6ee1d) )
	ROM_LOAD16_BYTE( "95401102.hi", 0x00000, 0x080000, CRC(3599427f) SHA1(16d915553b2b490a047888c64ebcf952714b3168) )

	ROM_REGION( 0x1000000, "ymz280b", ROMREGION_ERASE00 )
	ROM_LOAD( "totpsnd.lhs", 0x000000, 0x080000, CRC(56a73136) SHA1(10656ede18de9432a8a728cc59d000b5b1bf0150) )
	ROM_LOAD( "totpsnd.rhs", 0x080000, 0x080000, CRC(28d156ab) SHA1(ebf5c4e008015b9b56b3aa5228c05b8e298daa80) )

	DISK_REGION("scsi:2:cdrom")
	DISK_IMAGE_READONLY( "95100307", 0, SHA1(27ad1565f9a153fe71b72d9c597a6e3c3f13ded0) )
ROM_END

ROM_START( c3_ppays )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "95400687.hi", 0x00000, 0x080000, CRC(56080e1c) SHA1(49391059b5a758690d4972abad04d7e7aef23423) )
	ROM_LOAD16_BYTE( "95400687.lo", 0x00001, 0x080000, CRC(8b2c9c3d) SHA1(921c900447870f6ae51a4f3baeb60ce94e732291) )

	ROM_REGION( 0x1000000, "ymz280b", ROMREGION_ERASE00 )
	ROM_LOAD( "phrasesn.l", 0x0000, 0x080000, CRC(a436ccf8) SHA1(18c39aa2e68c32242e0de1347b25d4af44b84548) )

	DISK_REGION("scsi:2:cdrom")
	DISK_IMAGE_READONLY( "95100315", 0, SHA1(fc76d3ab5ff38c2dc4f06399f5399a1ae3c136e9) )
ROM_END

}

GAME( 1995, c3_telly,  0, bfm_cobra3, bfm_cobra3, bfm_cobra3_state, empty_init, ROT0, "BFM", "Telly Addicts (Bellfruit) (Cobra 3)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1995, c3_tellyns,0, bfm_cobra3, bfm_cobra3, bfm_cobra3_state, empty_init, ROT0, "BFM", "Telly Addicts (New Series) (Bellfruit) (Cobra 3)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1996, c3_rtime,  0, bfm_cobra3, bfm_cobra3, bfm_cobra3_state, empty_init, ROT0, "BFM", "Radio Times (Bellfruit) (Cobra 3)", MACHINE_IMPERFECT_GRAPHICS| MACHINE_NOT_WORKING )
GAME( 1997, c3_totp,   0, bfm_cobra3, bfm_cobra3, bfm_cobra3_state, empty_init, ROT0, "BFM", "Top of the Pops (Bellfruit) (Cobra 3?)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
GAME( 1998, c3_ppays,  0, bfm_cobra3, bfm_cobra3, bfm_cobra3_state, empty_init, ROT0, "BFM", "The Phrase That Pays (Bellfruit) (Cobra 3?)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NOT_WORKING )
