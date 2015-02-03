#include <unistd.h>
#include <stdint.h>
#include <string.h>

#include "osdepend.h"

#include "emu.h"
#include "clifront.h"
#include "render.h"
#include "ui/ui.h"
#include "uiinput.h"

#include "libretro.h"

#include "log.h"

#ifdef _WIN32
char slash = '\\';
#else
char slash = '/';
#endif

#if defined(WANT_MAME)
const char core[] = "mame";
#elif defined(WANT_MESS)
const char core[] = "mess";
#elif defined(WANT_UME)
const char core[] = "ume";
#endif

#if !defined(HAVE_GL) && !defined(HAVE_RGB32)
#define M16B
#endif

#include "render.c"
#ifndef HAVE_GL
#include "rendersw.inc"
#else 
static int init3d=1;
#endif
//============================================================
//  CONSTANTS
//============================================================
#define MAX_BUTTONS 16

#ifdef DEBUG_LOG
# define LOG(msg) fprintf(stderr, "%s\n", msg)
#else
# define LOG(msg)
#endif

//============================================================
//  GLOBALS
//============================================================

typedef struct joystate_t {
    int button[MAX_BUTTONS];
    int a1[2];
    int a2[2];
}Joystate;

// rendering target
static render_target *our_target = NULL;

// input device
static input_device *retrokbd_device; // KEYBD
static input_device *mouse_device;    // MOUSE
static input_device *joy_device[4];// JOY0/JOY1/JOY2/JOY3
static input_device *Pad_device[4];// PAD0/PAD1/PAD2/PAD3

// state
static UINT16 retrokbd_state[RETROK_LAST];
static int mouseLX,mouseLY;
static int mouseBUT[4];
static Joystate joystate[4];

static int ui_ipt_pushchar=-1;

static int mame_reset = -1;

// core options
bool hide_nagscreen = false;
bool hide_warnings = false;
bool nobuffer_enable = false;

static bool hide_gameinfo = false;
static bool mouse_enable = false;
static bool cheats_enable = false;
static bool alternate_renderer = false;
static bool boot_to_osd_enable = false;
static bool boot_to_bios_enable = false;
static bool experimental_cmdline = false;
static bool softlist_enable = false;
static bool softlist_auto = false;
static bool write_config_enable = false;
static bool read_config_enable = false;
static bool auto_save_enable = false;
static bool throttle_enable = false;
static bool game_specific_saves_enable = false;

// emu flags
static int tate = 0;
static int screenRot = 0;
int vertical,orient;
static bool arcade=FALSE;
static int FirstTimeUpdate = 1;

// rom file name and path
char g_rom_dir[1024];
char mediaType[10];
static char MgamePath[1024];
static char MparentPath[1024];
static char MgameName[512];
static char MsystemName[512];
static char gameName[1024];

// args for cores
static char XARGV[64][1024];
static const char* xargv_cmd[64];
int PARAMCOUNT=0;

// path configuration
#define NB_OPTPATH 12

static const char *dir_name[NB_OPTPATH]={
    "cfg","nvram","hi"/*,"memcard"*/,"input",
    "states" ,"snaps","diff","samples",
    "artwork","cheat","ini","hash"
};

static const char *opt_name[NB_OPTPATH]={
    "-cfg_directory","-nvram_directory","-hiscore_directory",/*"-memcard_directory",*/"-input_directory",
    "-state_directory" ,"-snapshot_directory","-diff_directory","-samplepath",
    "-artpath","-cheatpath","-inipath","-hashpath"
};

int opt_type[NB_OPTPATH]={ // 0 for save_dir | 1 for system_dir
    0,0,0,0,
    0,0,0,1,
    1,1,1,1
};

static void extract_basename(char *buf, const char *path, size_t size);
static void extract_directory(char *buf, const char *path, size_t size);

//============================================================
//  LIBCO
//============================================================
int pauseg=0;

#include <libco.h>

cothread_t mainThread;
cothread_t emuThread;

//============================================================
//  RETRO
//============================================================

#include "retromapper.c"
#include "retroinput.c"
#include "retroosd.c"

//============================================================
//  main
//============================================================

static void extract_basename(char *buf, const char *path, size_t size)
{
   const char *base = strrchr(path, '/');
   if (!base)
      base = strrchr(path, '\\');
   if (!base)
      base = path;

   if (*base == '\\' || *base == '/')
      base++;

   strncpy(buf, base, size - 1);
   buf[size - 1] = '\0';

   char *ext = strrchr(buf, '.');
   if (ext)
      *ext = '\0';
}

static void extract_directory(char *buf, const char *path, size_t size)
{
   strncpy(buf, path, size - 1);
   buf[size - 1] = '\0';

   char *base = strrchr(buf, '/');
   if (!base)
      base = strrchr(buf, '\\');

   if (base)
      *base = '\0';
   else
      buf[0] = '\0';
}

static int parsePath(char* path, char* gamePath, char* gameName) {
    int i;
    int slashIndex = -1;
    int dotIndex = -1;
    int len = strlen(path);

    if (len < 1) {
        return 0;
    }

    for (i = len - 1; i >=0; i--) {
        if (path[i] == slash) {
            slashIndex = i;
            break;
        } else
        if (path[i] == '.') {
            dotIndex = i;
        }
    }
    if (slashIndex < 0 || dotIndex < 0) {
        return 0;
    }

    strncpy(gamePath, path, slashIndex);
    gamePath[slashIndex] = 0;
    strncpy(gameName, path + (slashIndex + 1), dotIndex - (slashIndex + 1));
    gameName[dotIndex - (slashIndex + 1)] = 0;

    return 1;
}

static int parseSystemName(char* path, char* systemName) {
    int i;
    int j=0;
    int slashIndex[2]={-1,-1};
    int len = strlen(path);

    if (len < 1) {
        return 0;
    }

    for (i = len - 1; i >=0; i--) {
        if (j<2)
        {
           if (path[i] == slash) {
               slashIndex[j] = i;
               j++;
           }
        }
        else
           break;
    }
    if (slashIndex[0] < 0 || slashIndex[1] < 0 ) {
        return 0;
    }

    strncpy(systemName, path + (slashIndex[1] +1), slashIndex[0]-slashIndex[1]-1);
    return 1;
}

static int parseParentPath(char* path, char* parentPath) {
    int i;
    int j=0;
    int slashIndex[2]={-1,-1};
    int len = strlen(path);

    if (len < 1) {
        return 0;
    }

    for (i = len - 1; i >=0; i--) {
        if (j<2)
        {
           if (path[i] == slash) {
               slashIndex[j] = i;
               j++;
           }
        }
        else
           break;
    }
    if (slashIndex[0] < 0 || slashIndex[1] < 0 ) {
        return 0;
    }

    strncpy(parentPath, path, slashIndex[1]);
    return 1;
}

static int getGameInfo(char* gameName, int* rotation, int* driverIndex,bool *Arcade) {

    int gameFound = 0;
    int num=driver_list::find(gameName);
    log_cb(RETRO_LOG_DEBUG, "Searching for driver %s\n",gameName);

    if (num != -1){
        if(driver_list::driver(num).flags& GAME_TYPE_ARCADE)
        {
            *Arcade=TRUE;
            if (log_cb)
                log_cb(RETRO_LOG_DEBUG, "System type: ARCADE\n");
        }
        else if(driver_list::driver(num).flags& GAME_TYPE_CONSOLE)
        {
            if (log_cb)
                log_cb(RETRO_LOG_DEBUG, "System type: CONSOLE\n");
        }
        else if(driver_list::driver(num).flags& GAME_TYPE_COMPUTER)
        {
            if (log_cb)
                log_cb(RETRO_LOG_DEBUG, "System type: COMPUTER\n");
        }
        gameFound = 1;

        if (log_cb)
            log_cb(RETRO_LOG_INFO, "Game name: %s, Game description: %s\n",driver_list::driver(num).name, driver_list::driver(num).description);
    }
    else
    {
        if (log_cb)
            log_cb(RETRO_LOG_WARN, "Driver %s not found %i\n",gameName,num);
    }

    return gameFound;
}


void Extract_AllPath(char *srcpath){

    int result = 0, result_value =0;

    //split the path to directory and the name without the zip extension
    result = parsePath(srcpath, MgamePath, MgameName);
    if (result == 0) {
        strcpy(MgameName,srcpath);
        result_value|=1;
        if (log_cb)
            log_cb(RETRO_LOG_ERROR, "Error parsing game path: %s\n",srcpath);
    }

    //split the path to directory and the name without the zip extension
    result = parseSystemName(srcpath, MsystemName);
    if (result == 0) {
        strcpy(MsystemName,srcpath );
        result_value|=2;
        if (log_cb)
            log_cb(RETRO_LOG_ERROR, "Error parsing system name: %s\n",srcpath);
    }
    //get the parent path
    result = parseParentPath(srcpath, MparentPath);
    if (result == 0) {
        strcpy(MparentPath,srcpath );
        result_value|=4;
        if (log_cb)
            log_cb(RETRO_LOG_ERROR, "Error parsing parent path: %s\n",srcpath);
    }
    if (log_cb)
    {
        log_cb(RETRO_LOG_DEBUG, "Path extraction result: File name=%s\n",srcpath);
        log_cb(RETRO_LOG_DEBUG, "Path extraction result: Game name=%s\n",MgameName);
        log_cb(RETRO_LOG_DEBUG, "Path extraction result: System name=%s\n",MsystemName);
        log_cb(RETRO_LOG_DEBUG, "Path extraction result: Game path=%s\n",MgamePath);
        log_cb(RETRO_LOG_DEBUG, "Path extraction result: Parent path=%s\n",MparentPath);
    }

}

void Add_Option(const char* option){

    static int first=0;

    if(first==0){
        PARAMCOUNT=0;
        first++;
    }

    sprintf(XARGV[PARAMCOUNT++],"%s\0",option);
}

void Set_Default_Option(){

    //some hardcoded default Options

    Add_Option(core);
    if(throttle_enable)
        Add_Option("-throttle");
    else
        Add_Option("-nothrottle");
    Add_Option("-joystick");
    Add_Option("-samplerate");
    Add_Option("48000");
    if(cheats_enable)
       Add_Option("-cheat");
    else
       Add_Option("-nocheat");
    if(mouse_enable)
       Add_Option("-mouse");
    else
       Add_Option("-nomouse");
    if(hide_gameinfo)
       Add_Option("-skip_gameinfo");
    else
       Add_Option("-noskip_gameinfo");
    if(write_config_enable)
        Add_Option("-writeconfig");
    if(read_config_enable)
        Add_Option("-readconfig");
    else
        Add_Option("-noreadconfig");
    if(auto_save_enable)
        Add_Option("-autosave");
    if(game_specific_saves_enable)
    {
        Add_Option("-statename");
        char option[50];
        sprintf(option,"%%g/%s",MgameName);
        Add_Option(option);
    }
}

void Set_Path_Option(){

    char tmp_dir[256];

    //Setup path Option according to retro (save/system) directory or current if NULL
    for(int i=0;i<NB_OPTPATH;i++){

        Add_Option((char*)(opt_name[i]));

        if(opt_type[i]==0){
            if(retro_save_directory!=NULL)sprintf(tmp_dir, "%s%c%s%c%s", retro_save_directory, slash, core, slash,dir_name[i]);
            else sprintf(tmp_dir, "%s%c%s%c%s%c", ".", slash, core, slash,dir_name[i],slash);
        }
        else {
            if(retro_system_directory!=NULL)sprintf(tmp_dir, "%s%c%s%c%s", retro_system_directory, slash, core, slash,dir_name[i]);
            else sprintf(tmp_dir, "%s%c%s%c%s%c", ".", slash, core, slash,dir_name[i],slash);
        }

        Add_Option((char*)(tmp_dir));
    }

}

//============================================================
//  main
//============================================================

#include "retroexec.c"
#include "retroexpcmd.c"

#ifdef __cplusplus
extern "C"
#endif
int mmain(int argc, const char *argv)
{
    int result = 0;

    strcpy(gameName,argv);

    if(experimental_cmdline){
        parse_cmdline(argv);
            if (log_cb)
                log_cb(RETRO_LOG_INFO, "Starting game from command line:%s\n",gameName);

        result = executeGame_cmd(ARGUV[ARGUC-1]);
    }
    else
    {
        if (log_cb)
            log_cb(RETRO_LOG_INFO, "Starting game:%s\n",gameName);
        result = executeGame(gameName);
    }

    if(result<0)return result;
    if (log_cb)
        log_cb(RETRO_LOG_DEBUG, "Parameters:\n");
    for (int i = 0; i<PARAMCOUNT; i++){
        xargv_cmd[i] = (char*)(XARGV[i]);
        if (log_cb)
            log_cb(RETRO_LOG_DEBUG, " %s\n",XARGV[i]);
    }

    osd_options options;
    retro_osd_interface osd(options);
    cli_options MRoptions;
    osd.register_options();
    cli_frontend frontend(options, osd);

    result = frontend.execute(PARAMCOUNT, ( char **)xargv_cmd);

    xargv_cmd[PARAMCOUNT - 2] = NULL;


    return 1;
}

