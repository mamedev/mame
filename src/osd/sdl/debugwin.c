//============================================================
//
//  debugwin.c - SDL/GTK+ debug window handling
//
//  Copyright (c) 1996-2010, Nicola Salmoria and the MAME Team.
//  Visit http://mamedev.org for licensing and usage restrictions.
//
//  SDLMAME by Olivier Galibert and R. Belmont
//
//============================================================

#if !defined(NO_DEBUGGER)

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "dview.h"
#include "debug-intf.h"
#include "debug-sup.h"
#include "debug-cb.h"

#if 0
#define LOG(x...)
#else
#define LOG(x) printf x
#endif

typedef struct _debug_private_data debug_private_data;
struct _debug_private_data
{
	running_machine *machine;
};

#define GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), EGG_TYPE_CLOCK_FACE, EggClockFacePrivate))

enum {
	WIN_TYPE_MAIN 		= 0x01,
	WIN_TYPE_MEMORY 	= 0x02,
	WIN_TYPE_DISASM 	= 0x04,
	WIN_TYPE_LOG 		= 0x08,
	WIN_TYPE_ALL 		= 0x0f
};

typedef struct hentry {
	struct hentry *h;
	char *e;
} hentry;

typedef struct {
	GtkEntry *edit_w;
	struct hentry *h, *ch;
	char *hold;
	int keep_last;
	void (*cb)(running_machine *,const char *, void *);
	void *cbp;
	running_machine *machine;
} edit;

typedef struct _win_i win_i;
struct _win_i {
	int 					type;
	win_i *					next;
	GtkWidget *				win;
	edit 					ed;
	running_machine *machine;	// machine
	debug_view *disasm_dv;
	struct {
		DView 					*console_w, *disasm_w, *registers_w;
		debug_view *console;
		debug_view *registers;
		running_device *cpu;	// current CPU
	} main;
	struct {
		DView *memory_w;
		edit ed;
		GtkComboBox *zone_w;
		debug_view *memory;
	} memory;
	struct {
		DView *disasm_w;
		edit ed;
		GtkComboBox *cpu_w;
	} disasm;
	struct {
		DView *log_w;
		debug_view *log;
	} log;
};

typedef struct memorycombo_item
{
	struct memorycombo_item *next;
	char					name[256];
	UINT8					prefsize;
	running_machine 			*machine;	// machine
	const memory_subview_item		*subview;	// subview
	running_device			*device;	// CPU device
} memorycombo_item;


static win_i *win_list;

static memorycombo_item *memorycombo;

static void debugmain_init(running_machine *machine);

static void run_func_on_win_list(void (*f)(GtkWidget *widget), int win_type_mask)
{
	win_i *p;
	for(p = win_list; p; p = p->next)
		if ((p->type & win_type_mask) != 0)
			f(p->win);
}

static win_i *get_win_i(GtkWidget *win, int win_type_mask)
{
	win_i *p;

	assert(win != NULL);

	LOG(("====== %p %x\n", win, win_type_mask));
	for(p = win_list; p != NULL; p = p->next)
	{
		LOG(("%p %p %x\n", p, p->win, p->type));
		if (((p->type & win_type_mask) != 0) && (win == p->win))
			return p;
	}
	assert(0);
	return NULL;
}

static running_machine *get_running_machine(GtkWidget *win)
{
	win_i *p = get_win_i(win, WIN_TYPE_ALL);
	return p->machine; /* fixme*/
}

static win_i *first_win_i(int win_type_mask)
{
	win_i *p;

	for(p = win_list; p; p = p->next)
	{
		if ((p->type & win_type_mask) != 0)
			return p;
	}
	return NULL;
}

static void add_win_i(win_i *win, int win_type)
{
	win->next = win_list;
	win_list = win;
	win->type = win_type;
}

static void remove_win_i(win_i *win)
{
	win_i **p = &win_list;
	while(*p != win)
		p = &(*p)->next;
	if(win->next)
		*p = win->next;
	else
		*p = NULL;
}

static void debugwin_show(int show)
{
	void (*f)(GtkWidget *widget) = show ? gtk_widget_show : gtk_widget_hide;

	run_func_on_win_list(f, WIN_TYPE_ALL);
}

static void edit_add_hist(edit *e, const char *text)
{
	if(e->ch)
		e->ch = 0;

	if(e->hold) {
		osd_free(e->hold);
		e->hold =0;
	}

	if(!e->h || (text[0] && strcmp(text, e->h->e))) {
		hentry *h = (hentry *) osd_malloc(sizeof(hentry));
		h->h = e->h;
		h->e = mame_strdup(text);
		e->h = h;
	}
}

static void edit_set_field(edit *e)
{
	if(e->keep_last) {
		gtk_entry_set_text(e->edit_w, e->h->e);
		gtk_editable_select_region(GTK_EDITABLE(e->edit_w), 0, -1);
	} else
		gtk_entry_set_text(e->edit_w, "");
}

static void edit_activate(GtkEntry *item, gpointer user_data)
{
	edit *e = (edit *) user_data;
	const char *text = gtk_entry_get_text(e->edit_w);
	edit_add_hist(e, text);
	e->cb(e->machine, text, e->cbp);
	edit_set_field(e);

}

static void edit_hist_back(edit *e)
{
	const char *text = gtk_entry_get_text(e->edit_w);
	if(e->ch) {
		if(!e->ch->h)
			return;
		e->ch = e->ch->h;
	} else {
		if(!e->h)
			return;
		if(!strcmp(text, e->h->e)) {
			if(!e->h->h)
				return;
			e->ch = e->h->h;
		} else {
			if(text[0])
				e->hold = mame_strdup(text);
			e->ch = e->h;
		}
	}
	gtk_entry_set_text(e->edit_w, e->ch->e);
	gtk_editable_select_region(GTK_EDITABLE(e->edit_w), 0, -1);
}

static void edit_hist_forward(edit *e)
{
	hentry *h, *hp;
	if(!e->ch)
		return;
	h = e->h;
	hp = 0;
	while(h != e->ch) {
		hp = h;
		h = h->h;
	}

	if(hp) {
		e->ch = hp;
		gtk_entry_set_text(e->edit_w, hp->e);
	} else if(e->hold) {
		gtk_entry_set_text(e->edit_w, e->hold);
		osd_free(e->hold);
		e->hold = 0;
		e->ch = 0;
	} else {
		gtk_entry_set_text(e->edit_w, "");
		e->ch = 0;
	}
	gtk_editable_select_region(GTK_EDITABLE(e->edit_w), 0, -1);
}

static gboolean edit_key(GtkWidget *item, GdkEventKey *k, gpointer user_data)
{
	edit *e = (edit *)user_data;

	switch(k->keyval) {
	case GDK_Up:
		edit_hist_back(e);
		return TRUE;
	case GDK_Down:
		edit_hist_forward(e);
		return TRUE;
	}
	return FALSE;
}

static void edit_init(running_machine *machine, edit *e, GtkWidget *w, const char *ft, int kl, void (*cb)(running_machine *, const char *, void *), void *cbp)
{
	e->edit_w = GTK_ENTRY(w);
	e->h = 0;
	e->ch = 0;
	e->hold = 0;
	e->keep_last = kl;
	e->cb = cb;
	e->cbp = cbp;
	e->machine = machine;
	if(ft)
		edit_add_hist(e, ft);
	edit_set_field(e);
	g_signal_connect(e->edit_w, "activate", G_CALLBACK(edit_activate), e);
	g_signal_connect(e->edit_w, "key-press-event", G_CALLBACK(edit_key), e);
}

static void edit_del(edit *e)
{
	hentry *h = e->h;
	while(h) {
		hentry *he = h->h;
		osd_free(h->e);
		osd_free(h);
		h = he;
	}
}

static void debugmain_update_checks(win_i *info)
{
	assert(info != NULL);
	assert(info->type == WIN_TYPE_MAIN);

	int rc = disasm_view_get_right_column(info->disasm_dv);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "raw_opcodes")), rc == DASM_RIGHTCOL_RAW);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "enc_opcodes")), rc == DASM_RIGHTCOL_ENCRYPTED);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "comments")), rc == DASM_RIGHTCOL_COMMENTS);
}

void on_raw_opcodes_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MAIN | WIN_TYPE_DISASM);
	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "raw_opcodes")))) {
		debug_view_begin_update(info->disasm_dv);
		disasm_view_set_right_column(info->disasm_dv, DASM_RIGHTCOL_RAW);
		debug_view_end_update(info->disasm_dv);
	}
}

void on_enc_opcodes_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MAIN | WIN_TYPE_DISASM);
	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "enc_opcodes")))) {
		debug_view_begin_update(info->disasm_dv);
		disasm_view_set_right_column(info->disasm_dv, DASM_RIGHTCOL_ENCRYPTED);
		debug_view_end_update(info->disasm_dv);
	}
}

void on_comments_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MAIN | WIN_TYPE_DISASM);
	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "comments")))) {
		debug_view_begin_update(info->disasm_dv);
		disasm_view_set_right_column(info->disasm_dv, DASM_RIGHTCOL_COMMENTS);
		debug_view_end_update(info->disasm_dv);
	}
}

static void debugmain_set_cpu(running_device *cpu)
{
	win_i *dmain = first_win_i(WIN_TYPE_MAIN);

	if (cpu != dmain->main.cpu)
	{
		char title[256];
		const registers_subview_item *regsubitem;
		const disasm_subview_item *dasmsubitem;

		dmain->main.cpu = cpu;

		// first set all the views to the new cpu
		for (dasmsubitem = disasm_view_get_subview_list(dmain->disasm_dv); dasmsubitem != NULL; dasmsubitem = dasmsubitem->next)
			if (dasmsubitem->space->cpu == cpu)
			{
				disasm_view_set_subview(dmain->disasm_dv, dasmsubitem->index);
				break;
			}

		for (regsubitem = registers_view_get_subview_list(dmain->main.registers); regsubitem != NULL; regsubitem = regsubitem->next)
			if (regsubitem->device == cpu)
			{
				registers_view_set_subview(dmain->main.registers, regsubitem->index);
				break;
			}

		// then update the caption
		snprintf(title, ARRAY_LENGTH(title), "Debug: %s - %s", cpu->machine->gamedrv->name, regsubitem->name.cstr());
		gtk_window_set_title(GTK_WINDOW(dmain->win), title);
		debugmain_update_checks(dmain);
	}
}

// The entry point

void osd_wait_for_debugger(running_device *device, int firststop)
{
	win_i *dmain = first_win_i(WIN_TYPE_MAIN);
	// create a console window
	if(!dmain)
	{
		// GTK init should probably be done earlier
		gtk_init(0, 0);
		debugmain_init(device->machine);
	}

	// update the views in the console to reflect the current CPU
	debugmain_set_cpu(device);

	debugwin_show(1);
	gtk_main_iteration();
}

void debugwin_update_during_game(running_machine *machine)
{
	win_i *dmain = first_win_i(WIN_TYPE_MAIN);
	if(dmain)
	{
		gtk_main_iteration_do(FALSE);
	}
}

static void debugmain_process_string(running_machine *machine, const char *str, void *dmp)
{
	if(!str[0])
		debug_cpu_single_step(machine, 1);
	else
		debug_console_execute_command(machine, str, 1);
}

static void debugmain_destroy(GtkObject *obj, gpointer user_data)
{
	win_i *dmain = first_win_i(WIN_TYPE_MAIN);

	mame_schedule_exit(dmain->machine);
}

static void debugmain_init(running_machine *machine)
{
	win_i *dmain = (win_i *) osd_malloc(sizeof(*dmain));
	memset(dmain, 0, sizeof(*dmain));
	dmain->win = create_debugmain();
	dmain->main.cpu = NULL;
	dmain->machine = machine;
	add_win_i(dmain, WIN_TYPE_MAIN);

	dmain->main.console_w   = DVIEW(lookup_widget(dmain->win, "console"));
	dmain->main.disasm_w    = DVIEW(lookup_widget(dmain->win, "disasm"));
	dmain->main.registers_w = DVIEW(lookup_widget(dmain->win, "registers"));

	dview_set_debug_view(dmain->main.console_w,   machine, DVT_CONSOLE, &dmain->main.console);
	dview_set_debug_view(dmain->main.disasm_w,    machine, DVT_DISASSEMBLY, &dmain->disasm_dv);
	dview_set_debug_view(dmain->main.registers_w, machine, DVT_REGISTERS, &dmain->main.registers);

	edit_init(machine, &dmain->ed, lookup_widget(dmain->win, "edit"), 0, 0, debugmain_process_string, &dmain);

	debug_view_begin_update(dmain->disasm_dv);
	disasm_view_set_expression(dmain->disasm_dv, "curpc");
//  debug_view_set_property_UINT32(dmain->disasm, DVP_DASM_TRACK_LIVE, 1);
	debug_view_end_update(dmain->disasm_dv);

	g_signal_connect(dmain->win, "destroy", G_CALLBACK(debugmain_destroy), dmain);

	gtk_widget_show_all(dmain->win);
}

static void memorywin_update_checks(win_i *info)
{
	int bpc = memory_view_get_bytes_per_chunk(info->memory.memory);
	int rev = memory_view_get_reverse(info->memory.memory);

	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "chunks_1")), bpc == 1);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "chunks_2")), bpc == 2);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "chunks_4")), bpc == 4);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "reverse")), rev);
}

static void memorywin_zone_changed(GtkComboBox *zone_w, win_i *mem)
{
	int sel = gtk_combo_box_get_active(mem->memory.zone_w);
	char title[256];
	const memory_subview_item *subview;

	assert(mem->type == WIN_TYPE_MEMORY);

	// update the subview
	memory_view_set_subview(mem->memory.memory, sel);

	// change the checkmarks in the menu
	memorywin_update_checks(mem);

	// update the window title
	subview = memory_view_get_current_subview(mem->memory.memory);
	sprintf(title, "Memory: %s", subview->name.cstr());
	gtk_window_set_title(GTK_WINDOW(mem->win), title);

}

static void memorywin_process_string(running_machine *machine, const char *str, void *memp)
{
	win_i *mem = (win_i *) memp;

	assert(mem->type == WIN_TYPE_MEMORY);

	memory_view_set_expression(mem->memory.memory, str);
}

static void memorywin_destroy(GtkObject *obj, gpointer user_data)
{
	win_i *mem = (win_i *) user_data;

	remove_win_i(mem);
	edit_del(&mem->ed);
	osd_free(mem);
}

static void memorywin_new(running_machine *machine)
{
	win_i *mem;
	int item, cursel;
	running_device *curcpu = debug_cpu_get_visible_cpu(machine);
	const memory_subview_item *subview;

	mem = (win_i *) osd_malloc(sizeof(*mem));
	memset(mem, 0, sizeof(*mem));
	add_win_i(mem, WIN_TYPE_MEMORY);

	mem->win = create_memorywin();
	mem->machine = machine;

	mem->memory.memory_w = DVIEW(lookup_widget(mem->win, "memoryview"));
	dview_set_debug_view(mem->memory.memory_w, machine, DVT_MEMORY, &mem->memory.memory);

	mem->memory.zone_w   = GTK_COMBO_BOX(lookup_widget(mem->win, "zone"));

	edit_init(machine, &mem->ed, lookup_widget(mem->win, "edit"), "0", 1, memorywin_process_string, mem);

	debug_view_begin_update(mem->memory.memory);
	memory_view_set_expression(mem->memory.memory, "0");
	debug_view_end_update(mem->memory.memory);

	// populate the combobox
	if (!memorycombo)
	{
		cursel = item = 0;

		for (subview = memory_view_get_subview_list(mem->memory.memory); subview != NULL; subview = subview->next)
		{
			gtk_combo_box_append_text(mem->memory.zone_w, subview->name);
			if (cursel == 0 && subview->space != NULL && subview->space->cpu == curcpu)
				cursel = item;

			item++;
		}

		memory_view_set_subview(mem->memory.memory, cursel);
		gtk_combo_box_set_active(mem->memory.zone_w, cursel);
	}

	g_signal_connect(mem->memory.zone_w, "changed", G_CALLBACK(memorywin_zone_changed), mem);

	g_signal_connect(mem->win, "destroy", G_CALLBACK(memorywin_destroy), mem);
	gtk_widget_show_all(mem->win);
}

static void disasmwin_update_checks(win_i *info)
{
	int rc = disasm_view_get_right_column(info->disasm_dv);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "raw_opcodes")), rc == DASM_RIGHTCOL_RAW);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "enc_opcodes")), rc == DASM_RIGHTCOL_ENCRYPTED);
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "comments")), rc == DASM_RIGHTCOL_COMMENTS);
}

static void disasmwin_cpu_changed(GtkComboBox *cpu_w, win_i *dis)
{
	char title[256];
	const disasm_subview_item *subview;

	disasm_view_set_subview(dis->disasm_dv, gtk_combo_box_get_active(dis->disasm.cpu_w));

	disasmwin_update_checks(dis);

	subview = disasm_view_get_current_subview(dis->disasm_dv);
	sprintf(title, "Disassembly: %s", subview->name.cstr());
	gtk_window_set_title(GTK_WINDOW(dis->win), title);
}


static void disasmwin_process_string(running_machine *machine, const char *str, void *disp)
{
	win_i *dis = (win_i *) disp;

	assert(dis->type == WIN_TYPE_DISASM);

	disasm_view_set_expression(dis->disasm_dv, str);
}

static void disasmwin_destroy(GtkObject *obj, gpointer user_data)
{
	win_i *dis = (win_i *) user_data;

	remove_win_i(dis);
	edit_del(&dis->ed);
	osd_free(dis);
}

static void disasmwin_new(running_machine *machine)
{
	win_i *dis;
	int item, cursel;
	running_device *curcpu = debug_cpu_get_visible_cpu(machine);
	const disasm_subview_item *subview;
	char title[256];

	dis = (win_i *)osd_malloc(sizeof(*dis));
	memset(dis, 0, sizeof(*dis));

	add_win_i(dis, WIN_TYPE_DISASM);

	dis->win = create_disasmwin();
	dis->machine = machine;

	dis->disasm.disasm_w = DVIEW(lookup_widget(dis->win, "disasmview"));

	dview_set_debug_view(dis->disasm.disasm_w, machine, DVT_DISASSEMBLY, &dis->disasm_dv);

	dis->disasm.cpu_w    = GTK_COMBO_BOX(lookup_widget(dis->win, "cpu"));

	edit_init(machine, &dis->ed, lookup_widget(dis->win, "edit"), "curpc", 1, disasmwin_process_string, dis);

	debug_view_begin_update(dis->disasm_dv);
	disasm_view_set_expression(dis->disasm_dv, "curpc");
//  debug_view_set_property_UINT32(dis->disasm, DVP_DASM_TRACK_LIVE, 1);
	debug_view_end_update(dis->disasm_dv);

	// populate the combobox
	cursel = item = 0;
	for (subview = disasm_view_get_subview_list(dis->disasm_dv); subview != NULL; subview = subview->next)
	{
		gtk_combo_box_append_text(dis->disasm.cpu_w, subview->name);
		if (cursel == 0 && subview->space->cpu == curcpu)
			cursel = item;

		item++;
	}

	gtk_combo_box_set_active(dis->disasm.cpu_w, cursel);
	disasm_view_set_subview(dis->disasm_dv, cursel);

	subview = disasm_view_get_current_subview(dis->disasm_dv);
	sprintf(title, "Disassembly: %s", subview->name.cstr());
	gtk_window_set_title(GTK_WINDOW(dis->win), title);

	g_signal_connect(dis->disasm.cpu_w, "changed", G_CALLBACK(disasmwin_cpu_changed), dis);

	//  g_signal_connect(dis->edit_w, "activate", G_CALLBACK(disasmwin_process_string), dis);
	g_signal_connect(dis->win, "destroy", G_CALLBACK(disasmwin_destroy), dis);
	gtk_widget_show_all(dis->win);
}

static void logwin_destroy(GtkObject *obj, gpointer user_data)
{
	win_i *log = (win_i *) user_data;

	remove_win_i(log);
	osd_free(log);
}

static void logwin_new(running_machine *machine)
{
	win_i *log;

	log = (win_i *) osd_malloc(sizeof(*log));
	memset(log, 0, sizeof(*log));

	add_win_i(log, WIN_TYPE_LOG);

	log->win = create_logwin();
	log->machine = machine;

	log->log.log_w = DVIEW(lookup_widget(log->win, "logview"));
	dview_set_debug_view(log->log.log_w, machine, DVT_LOG, &log->log.log);

	g_signal_connect(log->win, "destroy", G_CALLBACK(logwin_destroy), log);

	gtk_widget_show_all(log->win);
}

void on_new_mem_activate(GtkWidget *win)
{
	memorywin_new(get_running_machine(win));
}

void on_new_disasm_activate(GtkWidget *win)
{
	disasmwin_new(get_running_machine(win));
}

void on_new_errorlog_activate(GtkWidget *win)
{
	logwin_new(get_running_machine(win));
}

void on_run_activate(GtkWidget *win)
{
	debug_cpu_go(get_running_machine(win), ~0);
}

void on_run_h_activate(GtkWidget *win)
{
	debugwin_show(0);
	debug_cpu_go(get_running_machine(win), ~0);
}

void on_run_cpu_activate(GtkWidget *win)
{
	debug_cpu_next_cpu(get_running_machine(win));
}

void on_run_irq_activate(GtkWidget *win)
{
	debug_cpu_go_interrupt(get_running_machine(win), -1);
}

void on_run_vbl_activate(GtkWidget *win)
{
	debug_cpu_go_vblank(get_running_machine(win));
}

void on_step_into_activate(GtkWidget *win)
{
	debug_cpu_single_step(get_running_machine(win), 1);
}

void on_step_over_activate(GtkWidget *win)
{
	debug_cpu_single_step_over(get_running_machine(win), 1);
}

void on_step_out_activate(GtkWidget *win)
{
	debug_cpu_single_step_out(get_running_machine(win));
}

void on_hard_reset_activate(GtkWidget *win)
{
	mame_schedule_hard_reset(get_running_machine(win));
}

void on_soft_reset_activate(GtkWidget *win)
{
	mame_schedule_soft_reset(get_running_machine(win));
	debug_cpu_go(get_running_machine(win), ~0);
}

void on_exit_activate(GtkWidget *win)
{
	mame_schedule_exit(get_running_machine(win));
}

void on_chunks_1_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MEMORY);
	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "chunks_1")))) {
		debug_view_begin_update(info->memory.memory);
		memory_view_set_bytes_per_chunk(info->memory.memory, 1);
		debug_view_end_update(info->memory.memory);
	}
}

void on_chunks_2_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MEMORY);
	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "chunks_2")))) {
		debug_view_begin_update(info->memory.memory);
		memory_view_set_bytes_per_chunk(info->memory.memory, 2);
		debug_view_end_update(info->memory.memory);
	}
}

void on_chunks_4_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MEMORY);
	if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "chunks_4")))) {
		debug_view_begin_update(info->memory.memory);
		memory_view_set_bytes_per_chunk(info->memory.memory, 4);
		debug_view_end_update(info->memory.memory);
	}
}

void on_reverse_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MEMORY);
	debug_view_begin_update(info->memory.memory);
	memory_view_set_reverse(info->memory.memory, gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(lookup_widget(info->win, "reverse"))));

	debug_view_end_update(info->memory.memory);
}

void on_ibpl_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MEMORY);
	debug_view_begin_update(info->memory.memory);
	memory_view_set_chunks_per_row(info->memory.memory, memory_view_get_chunks_per_row(info->memory.memory) + 1);
	debug_view_end_update(info->memory.memory);
}

void on_dbpl_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_MEMORY);
	debug_view_begin_update(info->memory.memory);
	memory_view_set_chunks_per_row(info->memory.memory, memory_view_get_chunks_per_row(info->memory.memory) - 1);
	debug_view_end_update(info->memory.memory);
}

void on_run_to_cursor_activate(GtkWidget *win)
{
	win_i *info = get_win_i(win, WIN_TYPE_ALL);
	char command[64];

	if (debug_view_get_cursor_visible(info->disasm_dv))
	{
		const address_space *space = disasm_view_get_current_subview(info->disasm_dv)->space;
		if (debug_cpu_get_visible_cpu(info->machine) == space->cpu)
		{
			offs_t address = disasm_view_get_selected_address(info->disasm_dv);
			sprintf(command, "go %X", address);
			debug_console_execute_command(info->machine, command, 1);
		}
	}

}

gboolean
on_disasm_button_press_event           (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
	DView *info = (DView *) widget;
	LOG(("gef %f %f %p %p\n", event->x, event->y, info, widget));
	if (debug_view_get_cursor_supported(info->view))
	{
		DViewClass *dvc = DVIEW_GET_CLASS(info);
		debug_view_xy topleft = debug_view_get_visible_position(info->view);
		debug_view_xy newpos;
		newpos.x = topleft.x + event->x / dvc->fixedfont_width;
		newpos.y = topleft.y + event->y / dvc->fixedfont_height;
		debug_view_set_cursor_position(info->view, newpos);
		debug_view_set_cursor_visible(info->view, TRUE);
		gtk_widget_grab_focus(widget);
		//SetFocus(wnd);
	}
	return true;
}

gboolean
on_memoryview_button_press_event           (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
	return on_disasm_button_press_event(widget, event, user_data);
}

gboolean
on_memoryview_key_press_event             (GtkWidget   *widget,
                                                        GdkEventKey *event,
                                                        gpointer     user_data)
{
	DView *info = (DView *) widget;
	//printf("%s\n", event->string);
	//printf("The name of this keysym is `%s'\n",
	//			gdk_keyval_name(event->keyval));
	//if (/*waiting_for_debugger ||*/ !debugwin_seq_pressed(info->view-> owner->machine))
	switch (event->keyval)
	{
		case GDK_Up:
			debug_view_type_character(info->view, DCH_UP);
			break;

		case GDK_Down:
			debug_view_type_character(info->view, DCH_DOWN);
			break;

		case GDK_Left:
			if ((event->state & GDK_CONTROL_MASK) != 0)
				debug_view_type_character(info->view, DCH_CTRLLEFT);
			else
				debug_view_type_character(info->view, DCH_LEFT);
			break;

		case GDK_Right:
			if ((event->state & GDK_CONTROL_MASK) != 0)
				debug_view_type_character(info->view, DCH_CTRLRIGHT);
			else
				debug_view_type_character(info->view, DCH_RIGHT);
			break;

		case GDK_Prior:
			debug_view_type_character(info->view, DCH_PUP);
			break;

		case GDK_Next:
			debug_view_type_character(info->view, DCH_PDOWN);
			break;

		case GDK_Home:
			if ((event->state & GDK_CONTROL_MASK) != 0)
				debug_view_type_character(info->view, DCH_CTRLHOME);
			else
				debug_view_type_character(info->view, DCH_HOME);
			break;

		case GDK_End:
			if ((event->state & GDK_CONTROL_MASK) != 0)
				debug_view_type_character(info->view, DCH_CTRLEND);
			else
				debug_view_type_character(info->view, DCH_END);
			break;

		case GDK_Escape:
			//if (info->owner->focuswnd != NULL)
			//	SetFocus(info->owner->focuswnd);
			//info->owner->ignore_char_lparam = lparam >> 16;
			break;

		case GDK_Tab:
			//if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
			//	debugwin_view_prev_view(info->owner, info);
			//else
			//	debugwin_view_next_view(info->view, dvw);
			break;
		default:
			if (event->keyval >= 32 && event->keyval < 127 && debug_view_get_cursor_supported(info->view))
				debug_view_type_character(info->view, event->keyval);
	}

	return true;
}

#else

#include "emu.h"

// win32 stubs for linking
void osd_wait_for_debugger(running_device *device, int firststop)
{
}

void debugwin_update_during_game(running_machine *machine)
{
}

#endif
