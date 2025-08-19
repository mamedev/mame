#if !defined(SDLMAME_WIN32)

// MAME headers
#include "emu.h"
#include "osdepend.h"

//// MAMEOS headers
#include "input_common.h"
#include "modules/lib/osdobj_common.h"
#include "modules/osdmodule.h"

#include "render.h"

#include <libudev.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

namespace {

// state information for a lightgun
struct lightgun_state
{
	int32_t lX, lY;
	int32_t buttons[16];
};

struct event_udev_entry
{
   const char *devnode;
   struct udev_list_entry *item;
};

int event_isNumber(const char *s) {
  int n;

  if(strlen(s) == 0) {
    return 0;
  }

  for(n=0; n<strlen(s); n++) {
    if(!(s[n] == '0' || s[n] == '1' || s[n] == '2' || s[n] == '3' || s[n] == '4' ||
         s[n] == '5' || s[n] == '6' || s[n] == '7' || s[n] == '8' || s[n] == '9'))
      return 0;
  }
  return 1;
}

// compare /dev/input/eventX and /dev/input/eventY where X and Y are numbers
int event_strcmp_events(const char* x, const char* y) {

  // find a common string
  int n, common, is_number;
  int a, b;

  n=0;
  while(x[n] == y[n] && x[n] != '\0' && y[n] != '\0') {
    n++;
  }
  common = n;

  // check if remaining string is a number
  is_number = 1;
  if(event_isNumber(x+common) == 0) is_number = 0;
  if(event_isNumber(y+common) == 0) is_number = 0;

  if(is_number == 1) {
    a = atoi(x+common);
    b = atoi(y+common);

    if(a == b) return  0;
    if(a < b)  return -1;
    return 1;
  } else {
    return strcmp(x, y);
  }
}
    
/* Used for sorting devnodes to appear in the correct order */
static int sort_devnodes(const void *a, const void *b)
{
  const struct event_udev_entry *aa = (const struct event_udev_entry*)a;
  const struct event_udev_entry *bb = (const struct event_udev_entry*)b;
  return event_strcmp_events(aa->devnode, bb->devnode);
}
  
//============================================================
//  udev_input_device
//============================================================

using udev_input_device = event_based_device<struct input_event>;

//============================================================
//  udev_lightgun_device
//============================================================

class udev_lightgun_device : public udev_input_device {
public:
  lightgun_state m_lightgun;
  int m_minx, m_maxx, m_miny, m_maxy;
  int m_borderX, m_borderY;
  std::string m_devpath;
  int m_fd;
  running_machine* m_machine;

  udev_lightgun_device(std::string &&name, std::string &&id, input_module &module, std::string devpath, running_machine* machine) :
    udev_input_device(std::move(name), std::move(id), module), m_lightgun({0}) {
    m_devpath = devpath;
    m_fd = -1;

    m_minx = 0;
    m_maxx = 0;
    m_miny = 0;
    m_maxy = 0;
    m_borderX = -1;
    m_borderY = -1;
    m_machine = machine;
  }

  ~udev_lightgun_device() {
    if(m_fd != -1) {
      close(m_fd);
    }
  }

  virtual void poll(bool relative_reset) override {
    struct input_event input_events[32];
    int j, len;

    event_based_device<struct input_event>::poll(relative_reset);
    
    while ((len = read(m_fd, input_events, sizeof(input_events))) > 0) {
      len /= sizeof(*input_events);
      for (j = 0; j < len; j++) {
    	queue_events(&(input_events[j]), 1);
      }
    }
  }

  virtual void process_event(struct input_event const &event) override {
    switch (event.type) {
    case EV_KEY:
      switch (event.code) {
      case BTN_LEFT:
	m_lightgun.buttons[0] = (event.value == 1) ? 0x80 : 0;
	break;
      case BTN_RIGHT:
	m_lightgun.buttons[1] = (event.value == 1) ? 0x80 : 0;
	break;
      case BTN_MIDDLE:
	m_lightgun.buttons[2] = (event.value == 1) ? 0x80 : 0;
	break;
      case BTN_1:
	m_lightgun.buttons[3] = (event.value == 1) ? 0x80 : 0;
	break;
      case BTN_2:
	m_lightgun.buttons[4] = (event.value == 1) ? 0x80 : 0;
	break;
      case BTN_3:
	m_lightgun.buttons[5] = (event.value == 1) ? 0x80 : 0;
	break;
      case BTN_4:
	m_lightgun.buttons[6] = (event.value == 1) ? 0x80 : 0;
	break;
      case BTN_5:
	m_lightgun.buttons[7] = (event.value == 1) ? 0x80 : 0;
	break;
      case BTN_6:
	m_lightgun.buttons[8] = (event.value == 1) ? 0x80 : 0;
	break;
      case BTN_7:
	m_lightgun.buttons[9] = (event.value == 1) ? 0x80 : 0;
	break;
      case BTN_8:
	m_lightgun.buttons[10] = (event.value == 1) ? 0x80 : 0;
	break;
      case BTN_9:
	m_lightgun.buttons[11] = (event.value == 1) ? 0x80 : 0;
	break;
	//case BTN_TOUCH:
	//break;
      default:
	break;
      }
      break;

    //case EV_REL:
    //  switch (event.code) {
    //  case REL_X:
    //	break;
    //  case REL_Y:
    //	break;
    //  case REL_WHEEL:
    //	break;
    //  case REL_HWHEEL:
    //	break;
    //  }
    //  break;

    case EV_ABS:
      switch (event.code) {
      case ABS_X:
	// compute the border if not already done
    	if(m_borderX == -1 || m_borderY == -1) computeBorders(m_borderX, m_borderY);
	
    	if(m_borderX != -1 && m_borderY != -1) {
    	  if(event.value >= m_minx+m_borderX && event.value <= m_maxx-m_borderX) {
    	    // in game
    	    m_lightgun.lX = normalize_absolute_axis(event.value, m_minx+m_borderX, m_maxx-m_borderX);
    	  } else {
    	    // in borders
    	    if(event.value < m_minx+m_borderX) m_lightgun.lX = normalize_absolute_axis(m_minx+m_borderX, m_minx+m_borderX, m_maxx-m_borderX);
    	    if(event.value > m_maxx-m_borderX) m_lightgun.lX = normalize_absolute_axis(m_maxx-m_borderX, m_minx+m_borderX, m_maxx-m_borderX);
    	  }
    	}
    	break;
      case ABS_Y:
    	m_lightgun.lY = normalize_absolute_axis(event.value, m_miny, m_maxy);
    	break;
      }
      break;
    }
  }

  virtual void reset() override {
    memset(&m_lightgun, 0, sizeof(m_lightgun));
  }

  virtual void configure(osd::input_device &device) override {
    struct input_absinfo absx, absy;
 
    m_fd = open(m_devpath.c_str(), O_RDONLY | O_NONBLOCK);
    if (m_fd != -1) {
      for (int button = 0; button < 16; button++) {
    	input_item_id itemid = static_cast<input_item_id>(ITEM_ID_BUTTON1 + button);
    	device.add_item(default_button_name(button), std::string_view(), itemid, generic_button_get_state<std::int32_t>, &(m_lightgun.buttons[button]));
      }

      if (ioctl(m_fd, EVIOCGABS(ABS_X), &absx) >= 0) {
    	if (ioctl(m_fd, EVIOCGABS(ABS_Y), &absy) >= 0) {
    	  m_minx = absx.minimum;
    	  m_maxx = absx.maximum;
    	  m_miny = absy.minimum;
    	  m_maxy = absy.maximum;
    
    	  device.add_item("axis X", std::string_view(), ITEM_ID_XAXIS, generic_axis_get_state<std::int32_t>, &(m_lightgun.lX));
    	  device.add_item("axis Y", std::string_view(), ITEM_ID_YAXIS, generic_axis_get_state<std::int32_t>, &(m_lightgun.lY));
    	}
      }
    }
  }

  void computeBorders(int& borderX, int& borderY) {
    int w, h, neww;
    double game_ratio = 4.0/3.0; // assume 4/3 for the game itself (because i don't manage to find the information in mame structures)
    render_target* target = m_machine->render().first_target();



    // wait that the game is correctly set. not perfect.
    if(m_machine->paused()) {
      return;
    }

    w = target->width();
    h = target->height();
  
    neww = h * game_ratio;
    if(neww < w) { // w borders
      m_borderY = 0;
      m_borderX = (m_maxx-m_minx)*((w - neww)/2)/w;
    } else {
      // h borders - not handled case
      m_borderX = 0;
      m_borderY = 0;
    }

    osd_printf_verbose("Lightgun: borders size : %ix%i (resolution=%ix%i)\n", borderX, borderY, w, h);
  }
};

//============================================================
//  udev_lightgun_module
//============================================================

class udev_lightgun_module : public input_module_impl<udev_lightgun_device, osd_common_t>
{
private:
  struct udev *m_udev;

public:
  udev_lightgun_module() : input_module_impl<udev_lightgun_device, osd_common_t>(OSD_LIGHTGUNINPUT_PROVIDER, "udev")
  {
    m_udev = NULL;
  }

  ~udev_lightgun_module() {
    if (m_udev != NULL) udev_unref(m_udev);
  }

  virtual void exit() override
  {
    input_module_impl<udev_lightgun_device, osd_common_t>::exit();
  }

  void input_init(running_machine &machine) override {
    struct udev_enumerate *enumerate;
    struct udev_list_entry     *devs = NULL;
    struct udev_list_entry     *item = NULL;
    unsigned sorted_count = 0;
    struct event_udev_entry sorted[8]; // max devices
    unsigned int i;

   osd_printf_verbose("Lightgun: Begin udev initialization\n");

   input_module_impl<udev_lightgun_device, osd_common_t>::input_init(machine);

    m_udev = udev_new();
    if(m_udev == NULL) return;

    enumerate = udev_enumerate_new(m_udev);

    if (enumerate != NULL) {
      udev_enumerate_add_match_property(enumerate, "ID_INPUT_MOUSE", "1");
      udev_enumerate_add_match_subsystem(enumerate, "input");
      udev_enumerate_scan_devices(enumerate);
      devs = udev_enumerate_get_list_entry(enumerate);

      for (item = devs; item; item = udev_list_entry_get_next(item)) {
	const char         *name = udev_list_entry_get_name(item);
	struct udev_device  *dev = udev_device_new_from_syspath(m_udev, name);
	const char      *devnode = udev_device_get_devnode(dev);
	
	if (devnode != NULL && sorted_count < 8) {
	  sorted[sorted_count].devnode = devnode;
	  sorted[sorted_count].item = item;
	  sorted_count++;
	} else {
	  udev_device_unref(dev);
	}
      }

      /* Sort the udev entries by devnode name so that they are
       * created in the proper order */
      qsort(sorted, sorted_count,
	    sizeof(struct event_udev_entry), sort_devnodes);

      for (i = 0; i < sorted_count; i++) {
      	const char *name = udev_list_entry_get_name(sorted[i].item);

      	/* Get the filename of the /sys entry for the device
      	 * and create a udev_device object (dev) representing it. */
      	struct udev_device *dev = udev_device_new_from_syspath(m_udev, name);
      	const char *devnode     = udev_device_get_devnode(dev);
      	char devname[64];

      	if (devnode) {
      	  int fd = open(devnode, O_RDONLY | O_NONBLOCK);
      	  if (fd != -1) {
      	    if (ioctl(fd, EVIOCGNAME(sizeof(devname)), devname) < 0) {
      	      devname[0] = '\0';
      	    }
      	    close(fd);
      	    auto &devinfo = create_device<udev_lightgun_device>(DEVICE_CLASS_LIGHTGUN,
								std::string(devname),
								std::string(devname),
								std::string(devnode),
								&machine);
	    osd_printf_verbose("Lightgun: registered %s (%s)\n", devinfo.name(), devnode);
     	  }
      	}
      	udev_device_unref(dev);
      }
      udev_enumerate_unref(enumerate);
    }

    osd_printf_verbose("Lightgun: End udev initialization\n");
  }
  
};


} // anonymous namespace

#else // !defined(SDLMAME_WIN32)

MODULE_NOT_SUPPORTED(udev_lightgun_module, OSD_LIGHTGUNINPUT_PROVIDER, "udev")

#endif // !defined(SDLMAME_WIN32)

MODULE_DEFINITION(LIGHTGUN_UDEV, udev_lightgun_module)
