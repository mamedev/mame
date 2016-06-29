
// ALL games use this - tilemap DMA (RAM -> private buffer)
void raiden2cop_device::dma_tilemap_buffer()
{
	int src = cop_dma_src[cop_dma_mode] << 6;
	if (src == 0xcfc0) src = 0xd000; // R2, why?? everything else sets the right pointer (it also sets up odd size / dest regs, they probably counteract this)

	for (int i = 0; i < 0x2800 / 2; i++)
	{
		UINT16 tileval = m_host_space->read_word(src);
		src += 2;
		m_videoramout_cb(i, tileval, 0xffff);
	}
}

 // ALL games use this - palette DMA (RAM -> private buffer)
void raiden2cop_device::dma_palette_buffer()
{
	int src = cop_dma_src[cop_dma_mode] << 6;

	for (int i = 0; i < 0x1000 / 2; i++) // todo, use length register
	{
		UINT16 palval = m_host_space->read_word(src);
		src += 2;
		m_palette->set_pen_color(i, pal5bit(palval >> 0), pal5bit(palval >> 5), pal5bit(palval >> 10));
	}
}

// these are typically used to transfer palette data from one RAM buffer to another, applying fade values to it prior to the 0x15 transfer
void raiden2cop_device::dma_palette_brightness()
{
	UINT32 src, dst, size, i;

	/*
	Apparently all of those are just different DMA channels, brightness effects are done through a RAM table and the pal_brightness_val / mode
	0x80 is used by Legionnaire
	0x81 is used by SD Gundam and Godzilla
	0x82 is used by Zero Team and X Se Dae
	0x86 is used by Seibu Cup Soccer
	0x87 is used by Denjin Makai

	TODO:
	- Denjin Makai mode 4 is totally guessworked.
	- SD Gundam doesn't fade colors correctly, it should have the text layer / sprites with normal gradient and the rest dimmed in most cases,
	presumably bad RAM table or bad algorithm
	*/

	//if(dma_trigger != 0x87)
	//printf("SRC: %08x %08x DST:%08x SIZE:%08x TRIGGER: %08x %02x %02x\n",cop_dma_src[cop_dma_mode] << 6,cop_dma_adr_rel * 0x400,cop_dma_dst[cop_dma_mode] << 6,cop_dma_size[cop_dma_mode] << 5,cop_dma_mode,pal_brightness_val,pal_brightness_mode);

	src = (cop_dma_src[cop_dma_mode] << 6);
	dst = (cop_dma_dst[cop_dma_mode] << 6);
	size = ((cop_dma_size[cop_dma_mode] << 5) - (cop_dma_dst[cop_dma_mode] << 6) + 0x20) / 2;

	for (i = 0; i < size; i++)
	{
		UINT16 pal_val;
		int r, g, b;
		int rt, gt, bt;

		if (pal_brightness_mode == 5)
		{
			bt = ((m_host_space->read_word(src + (cop_dma_adr_rel * 0x400))) & 0x7c00) >> 5;
			bt = fade_table(bt | (pal_brightness_val ^ 0));
			b = ((m_host_space->read_word(src)) & 0x7c00) >> 5;
			b = fade_table(b | (pal_brightness_val ^ 0x1f));
			pal_val = ((b + bt) & 0x1f) << 10;
			gt = ((m_host_space->read_word(src + (cop_dma_adr_rel * 0x400))) & 0x03e0);
			gt = fade_table(gt | (pal_brightness_val ^ 0));
			g = ((m_host_space->read_word(src)) & 0x03e0);
			g = fade_table(g | (pal_brightness_val ^ 0x1f));
			pal_val |= ((g + gt) & 0x1f) << 5;
			rt = ((m_host_space->read_word(src + (cop_dma_adr_rel * 0x400))) & 0x001f) << 5;
			rt = fade_table(rt | (pal_brightness_val ^ 0));
			r = ((m_host_space->read_word(src)) & 0x001f) << 5;
			r = fade_table(r | (pal_brightness_val ^ 0x1f));
			pal_val |= ((r + rt) & 0x1f);
		}
		else if (pal_brightness_mode == 4) //Denjin Makai
		{
			UINT16 targetpaldata = m_host_space->read_word(src + (cop_dma_adr_rel * 0x400));
			UINT16 paldata = m_host_space->read_word(src); // ^1 !!! (why?)

			bt = (targetpaldata & 0x7c00) >> 10;
			b = (paldata & 0x7c00) >> 10;
			gt = (targetpaldata & 0x03e0) >> 5;
			g = (paldata & 0x03e0) >> 5;
			rt = (targetpaldata & 0x001f) >> 0;
			r = (paldata & 0x001f) >> 0;

			if (pal_brightness_val == 0x10)
				pal_val = bt << 10 | gt << 5 | rt << 0;
			else if (pal_brightness_val == 0xff) // TODO: might be the back plane or it still doesn't do any mod, needs PCB tests
				pal_val = 0;
			else
			{
				bt = fade_table(bt << 5 | ((pal_brightness_val * 2) ^ 0));
				b = fade_table(b << 5 | ((pal_brightness_val * 2) ^ 0x1f));
				pal_val = ((b + bt) & 0x1f) << 10;
				gt = fade_table(gt << 5 | ((pal_brightness_val * 2) ^ 0));
				g = fade_table(g << 5 | ((pal_brightness_val * 2) ^ 0x1f));
				pal_val |= ((g + gt) & 0x1f) << 5;
				rt = fade_table(rt << 5 | ((pal_brightness_val * 2) ^ 0));
				r = fade_table(r << 5 | ((pal_brightness_val * 2) ^ 0x1f));
				pal_val |= ((r + rt) & 0x1f);
			}
		}
		else
		{
			printf("Warning: palette DMA used with mode %02x!\n", pal_brightness_mode);
			pal_val = m_host_space->read_word(src);
		}

		m_host_space->write_word(dst, pal_val);
		src += 2;
		dst += 2;
	}
}

void raiden2cop_device::dma_fill()
{
	UINT32 length, address;
	int i;
	if (cop_dma_dst[cop_dma_mode] != 0x0000) // Invalid?
		return;

	address = (cop_dma_src[cop_dma_mode] << 6);
	length = (cop_dma_size[cop_dma_mode] + 1) << 5;

	//printf("%08x %08x\n",address,length);

	for (i = address; i < address + length; i += 4)
		m_host_space->write_dword(i, (cop_dma_v1) | (cop_dma_v2 << 16));

	/*
	UINT32 length, address;
	int i;
	if(cop_dma_dst[cop_dma_mode] != 0x0000) // Invalid?
	return;

	address = (cop_dma_src[cop_dma_mode] << 6);
	length = (cop_dma_size[cop_dma_mode]+1) << 5;

	//printf("%08x %08x\n",address,length);

	for (i=address;i<address+length;i+=4)
	{
	m_host_space->write_dword(i, m_fill_val);
	}
	*/
}

void raiden2cop_device::dma_zsorting(UINT16 data)
{
	struct sort_entry {
		INT16 sorting_key;
		UINT16 val;
	};

	std::vector<sort_entry> entries(data);
	for(int i=0; i<data; i++) {
		sort_entry &e = entries[i];
		e.val = m_host_space->read_word(cop_sort_lookup + 2*i);
		e.sorting_key = m_host_space->read_word(cop_sort_ram_addr + e.val);
	}
	switch(cop_sort_param) {
	case 1:
		std::sort(entries.begin(), entries.end(), [](const auto &a, const auto &b){ return a.sorting_key > b.sorting_key; });
		break;
	case 2:
		std::sort(entries.begin(), entries.end(), [](const auto &a, const auto &b){ return a.sorting_key < b.sorting_key; });
		break;
	}

	for(int i=0; i<data; i++)
		m_host_space->write_word(cop_sort_lookup + 2*i, entries[i].val);
}


