#include <vector>
#include <algorithm>

#include "emu.h"
#include "video/segaic24.h"
#include "includes/segas24.h"


namespace {
	struct layer_sort {
		layer_sort(segas24_mixer *_mixer) { mixer = _mixer; }

		bool operator()(int l1, int l2) {
			static const int default_pri[12] = { 0, 1, 2, 3, 4, 5, 6, 7, -4, -3, -2, -1 };
			int p1 = mixer->get_reg(l1) & 7;
			int p2 = mixer->get_reg(l2) & 7;
			if(p1 != p2)
				return p1 - p2 < 0;
			return default_pri[l2] - default_pri[l1] < 0;
		}

		segas24_mixer *mixer;
	};
};

SCREEN_UPDATE(system24)
{
	segas24_state *state = screen->machine().driver_data<segas24_state>();

	if(state->vmixer->get_reg(13) & 1) {
		bitmap_fill(bitmap, 0, get_black_pen(screen->machine()));
		return 0;
	}

	bitmap_fill(screen->machine().priority_bitmap, 0, 0);
	bitmap_fill(bitmap, cliprect, 0);

	std::vector<int> order;
	order.resize(12);
	for(int i=0; i<12; i++)
		order[i] = i;

	std::sort(order.begin(), order.end(), layer_sort(state->vmixer));

	int spri[4];
	int level = 0;
	for(int i=0; i<12; i++)
		if(order[i] < 8)
			state->vtile->draw(bitmap, cliprect, order[i], level, 0);
		else {
			spri[order[i]-8] = level;
			level++;
		}

	state->vsprite->draw(bitmap, cliprect, spri);
	return 0;
}
