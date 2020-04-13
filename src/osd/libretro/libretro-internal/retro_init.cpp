#ifdef __GNUC__
#include <unistd.h>
#endif
#include <stdint.h>
#include <string.h>

#include "osdepend.h"
#include "emu.h"
#include "render.h"
#include "ui/uimain.h"
#include "uiinput.h"
#include "drivenum.h"

#include "libretro.h"
#include "options.h"
#include "osdepend.h"

#include "modules/lib/osdobj_common.h"
#include "modules/lib/osdlib.h"
#include "modules/osdmodule.h"
#include "modules/font/font_module.h"

#include "libretro_shared.h"

#define UINT16 uint16_t
#define INT16 int16_t
#define UINT32 uint32_t
#define INT32 int32_t
#define UINT8 uint8_t
#define INT8 int8_t

#ifdef _WIN32
char slash = '\\';
#else
char slash = '/';
#endif


/* Args for experimental_commandline */
static char ARGUV[32][1024];
static unsigned char ARGUC=0;

/* state */
int mame_reset = -1;

/* core options */
bool nobuffer_enable = false;
bool mouse_enable = false;
int  lightgun_mode = RETRO_SETTING_LIGHTGUN_MODE_DISABLED;
bool cheats_enable = false;
bool alternate_renderer = false;
bool boot_to_osd_enable = false;
bool boot_to_bios_enable = false;
bool experimental_cmdline = false;
bool softlist_enable = false;
bool softlist_auto = false;
bool write_config_enable = false;
bool read_config_enable = false;
bool auto_save_enable = false;
bool throttle_enable = false;
bool game_specific_saves_enable = false;
bool buttons_profiles = true;
bool mame_paths_enable = false;
bool mame_4way_enable = false;
char mame_4way_map[256];

bool res_43 = false;
bool video_changed = false;
// emu flags
static int tate = 0;
static int screenRot = 0;
int vertical,orient;
static bool arcade=false;
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
#define NB_OPTPATH 11//12

static const char *dir_name[NB_OPTPATH]= {
    "cfg","nvram"/*,"hi"*//*,"memcard"*/,"input",
    "states" ,"snaps","diff","samples",
    "artwork","cheat","ini","hash"
};

static const char *opt_name[NB_OPTPATH]= {
    "-cfg_directory","-nvram_directory"/*,"-hiscore_directory"*/,/*"-memcard_directory",*/"-input_directory",
    "-state_directory" ,"-snapshot_directory","-diff_directory","-samplepath",
    "-artpath","-cheatpath","-inipath","-hashpath"
};

int opt_type[NB_OPTPATH]={ // 0 for save_dir | 1 for system_dir
    0,0/*,0*/,0,
    0,0,0,1,
    1,1,1,1
};


//============================================================
//  main
//============================================================
static int parsePath(char* path, char* gamePath, char* gameName)
{
   int i;
   int slashIndex = -1;
   int dotIndex   = -1;
   int len        = strlen(path);

   if (len < 1)
      return 0;

   for (i = len - 1; i >= 0; i--)
   {
      if (path[i] == slash)
      {
         slashIndex = i;
         break;
      }
      else
         if (path[i] == '.')
            dotIndex = i;
   }
   if (slashIndex < 0 && dotIndex >0){
        strcpy(gamePath, ".\0");
        strncpy(gameName, path , dotIndex );
        gameName[dotIndex] = 0;
        return 1;
   }

   if (slashIndex < 0 || dotIndex < 0)
      return 0;

   strncpy(gamePath, path, slashIndex);
   gamePath[slashIndex] = 0;
   strncpy(gameName, path + (slashIndex + 1), dotIndex - (slashIndex + 1));
   gameName[dotIndex - (slashIndex + 1)] = 0;

   return 1;
}

static int parseSystemName(char* path, char* systemName)
{
   int i, j = 0;
   int slashIndex[2]={-1,-1};
   int len = strlen(path);

   if (len < 1)
      return 0;

   for (i = len - 1; i >=0; i--)
   {
      if (j<2)
      {
         if (path[i] == slash)
         {
            slashIndex[j] = i;
            j++;
         }
      }
      else
         break;
   }

   if (slashIndex[0] < 0 || slashIndex[1] < 0 )
      return 0;

   strncpy(systemName, path + (slashIndex[1] +1), slashIndex[0]-slashIndex[1]-1);
   return 1;
}

static int parseParentPath(char* path, char* parentPath)
{
   int i, j = 0;
   int slashIndex[2] = {-1,-1};
   int len = strlen(path);

   if (len < 1)
      return 0;

   for (i = len - 1; i >= 0; i--)
   {
      if (j<2)
      {
         if (path[i] == slash)
         {
            slashIndex[j] = i;
            j++;
         }
      }
      else
         break;
   }

   if (slashIndex[0] < 0 || slashIndex[1] < 0 )
      return 0;

   strncpy(parentPath, path, slashIndex[1]);
   return 1;
}

static int getGameInfo(char* gameName, int* rotation, int* driverIndex,bool *Arcade)
{
   int gameFound = 0;
   int num = driver_list::find(gameName);

   if (log_cb)
        log_cb(RETRO_LOG_DEBUG, "Searching for driver %s\n",gameName);

   if (num != -1)
   {
      if (driver_list::driver(num).flags & MACHINE_TYPE_ARCADE)
      {
         *Arcade=true;
         if (log_cb)
            log_cb(RETRO_LOG_DEBUG, "System type: ARCADE\n");
      }
      else if(driver_list::driver(num).flags& MACHINE_TYPE_CONSOLE)
      {
         if (log_cb)
            log_cb(RETRO_LOG_DEBUG, "System type: CONSOLE\n");
      }
      else if(driver_list::driver(num).flags& MACHINE_TYPE_COMPUTER)
      {
         if (log_cb)
            log_cb(RETRO_LOG_DEBUG, "System type: COMPUTER\n");
      }
      gameFound = 1;

      if (log_cb)
         log_cb(RETRO_LOG_INFO, "Game name: %s, Game description: %s\n",
               driver_list::driver(num).name,
               driver_list::driver(num).type.fullname());
   }
   else
   {
      if (log_cb)
         log_cb(RETRO_LOG_WARN, "Driver %s not found %i\n",gameName,num);
   }

   return gameFound;
}


void Extract_AllPath(char *srcpath)
{
   int result_value =0;

   /* Split the path to directory
    * and the name without the zip extension. */
   int result = parsePath(srcpath, MgamePath, MgameName);

   if (result == 0)
   {
      strcpy(MgameName,srcpath);
      result_value|=1;
      if (log_cb)
         log_cb(RETRO_LOG_ERROR, "Error parsing game path: %s\n",srcpath);
   }

   /* Split the path to directory and
    * the name without the zip extension. */
   result = parseSystemName(srcpath, MsystemName);
   if (result == 0)
   {
      strcpy(MsystemName,srcpath );
      result_value|=2;
      if (log_cb)
         log_cb(RETRO_LOG_ERROR, "Error parsing system name: %s\n",srcpath);
   }

   /* Get the parent path. */
   result = parseParentPath(srcpath, MparentPath);

   if (result == 0)
   {
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

static void Add_Option(const char* option)
{
   static int first = 0;

   if (first == 0)
   {
      PARAMCOUNT=0;
      first++;
   }

   sprintf(XARGV[PARAMCOUNT++], "%s", option);
}

static void Set_Default_Option(void)
{
   /* some hardcoded default options. */

   Add_Option(core);

   if(throttle_enable)
      Add_Option("-throttle");
   else
      Add_Option("-nothrottle");

   Add_Option("-joystick");
   Add_Option("-joystick_deadzone");
   Add_Option("0");
   Add_Option("-joystick_saturation");
   Add_Option("1");
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

   if ( lightgun_mode != RETRO_SETTING_LIGHTGUN_MODE_DISABLED )
      Add_Option("-lightgun");
   else
      Add_Option("-nolightgun");

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
      char option[50];
      Add_Option("-statename");
      sprintf(option,"%%g/%s",MgameName);
      Add_Option(option);
   }

   if(mame_4way_enable)
   {
      Add_Option("-joystick_map");
      Add_Option(mame_4way_map);
   }
}

static void Set_Path_Option(void)
{
   int i;
   char tmp_dir[256];

   /*Setup path option according to retro (save/system) directory,
    * or current if NULL. */

   for(i = 0; i < NB_OPTPATH; i++)
   {
      Add_Option((char*)(opt_name[i]));

      if(opt_type[i] == 0)
      {
         if (retro_save_directory)
            sprintf(tmp_dir, "%s%c%s%c%s", retro_save_directory, slash, core, slash,dir_name[i]);
         else
            sprintf(tmp_dir, "%s%c%s%c%s%c", ".", slash, core, slash,dir_name[i],slash);
      }
      else
      {
         if(retro_system_directory)
            sprintf(tmp_dir, "%s%c%s%c%s", retro_system_directory, slash, core, slash,dir_name[i]);
         else
            sprintf(tmp_dir, "%s%c%s%c%s%c", ".", slash, core, slash,dir_name[i],slash);
      }

      Add_Option((char*)(tmp_dir));
   }

}

//============================================================
//  main
//============================================================

static int execute_game(char* path)
{
   unsigned i;
   char tmp_dir[256];
   int gameRot=0;
   int driverIndex;

   FirstTimeUpdate = 1;

   screenRot = 0;

   for (i = 0; i < 64; i++)
      xargv_cmd[i]=NULL;

   Extract_AllPath(path);

   /* Find if the driver exists for MgameName.
    * If not, check if a driver exists for MsystemName.
    * Otherwise, exit. */
   if (getGameInfo(MgameName, &gameRot, &driverIndex,&arcade) == 0)
   {
      if (log_cb)
         log_cb(RETRO_LOG_ERROR, "Driver not found %s\n",MgameName);
      if (getGameInfo(MsystemName, &gameRot, &driverIndex,&arcade) == 0)
      {
         if (log_cb)
            log_cb(RETRO_LOG_ERROR, "System not found: %s\n",MsystemName);
         return -2;
      }
   }

   /* Handle case where Arcade game exists and game on a System also. */
   if(arcade == true)
   {
      if (log_cb)
         log_cb(RETRO_LOG_ERROR, "System not found: %s\n",MsystemName);

      // test system
      if (getGameInfo(MsystemName, &gameRot, &driverIndex,&arcade) != 0)
         arcade=false;
   }

   /* useless ? */
   if (tate)
   {
      /* horizontal game */
      if (gameRot == ROT0)
         screenRot = 1;
      else if (gameRot &  ORIENTATION_FLIP_X)
         screenRot = 3;
   }
   else
   {
      if (gameRot != ROT0)
      {
         screenRot = 1;
         if (gameRot &  ORIENTATION_FLIP_X)
            screenRot = 2;
      }
   }

   if (log_cb)
   {
      log_cb(RETRO_LOG_INFO, "Creating frontend for game: %s\n",MgameName);
      log_cb(RETRO_LOG_INFO, "Softlists: %d\n",softlist_enable);
   }

   Set_Default_Option();

   if(!mame_paths_enable)
      Set_Path_Option();

   /* useless ? */
   if (tate)
   {
      if (screenRot == 3)
         Add_Option((char*) "-rol");
   }
   else
   {
      if (screenRot == 2)
         Add_Option((char*)"-rol");
   }

   Add_Option((char*)("-rompath"));

   if(!boot_to_osd_enable)
   {
      sprintf(tmp_dir, "%s", MgamePath);
      Add_Option((char*)(tmp_dir));

      if(softlist_enable)
      {
         if(!arcade)
         {
            Add_Option(MsystemName);
            if(!boot_to_bios_enable)
            {
               if(!softlist_auto)
                  Add_Option((char*)mediaType);
               Add_Option((char*)MgameName);
            }
         }
         else
            Add_Option((char*)MgameName);
      }
      else
      {
         if (!strcmp(mediaType, "-rom"))
            Add_Option(MgameName);
         else
         {
            Add_Option(MsystemName);
            Add_Option((char*)mediaType);
            Add_Option((char*)gameName);
         }
      }
   }
   else
   {
      sprintf(tmp_dir, "%s;%s", MgamePath,MparentPath);
      Add_Option((char*)(tmp_dir));
   }

   return 0;
}

static void parse_cmdline(const char *argv)
{
   int c,c2;
   static char buffer[512*4];
   enum states
   {
      DULL,
      IN_WORD,
      IN_STRING
   } state = DULL;
   char *p  = NULL;
   char *p2 = NULL;
   char *start_of_word = NULL;

   strcpy(buffer,argv);
   strcat(buffer," \0");

   for (p = buffer; *p != '\0'; p++)
   {
      c = (unsigned char) *p; /* convert to unsigned char for is* functions */

      switch (state)
      {
         case DULL:
            /* not in a word, not in a double quoted string */

            if (isspace(c)) /* still not in a word, so ignore this char */
               continue;

            /* not a space -- if it's a double quote we go to IN_STRING, else to IN_WORD */
            if (c == '"')
            {
               state = IN_STRING;
               start_of_word = p + 1; /* word starts at *next* char, not this one */
               continue;
            }
            state = IN_WORD;
            start_of_word = p; /* word starts here */
            continue;

         case IN_STRING:
            /* we're in a double quoted string, so keep going until we hit a close " */
            if (c == '"')
            {
               /* word goes from start_of_word to p-1 */
               //... do something with the word ...
               for (c2=0, p2 = start_of_word; p2 < p; p2++, c2++)
                  ARGUV[ARGUC][c2] = (unsigned char)*p2;
               ARGUC++;

               state = DULL; /* back to "not in word, not in string" state */
            }
            continue; /* either still IN_STRING or we handled the end above */

         case IN_WORD:
            /* we're in a word, so keep going until we get to a space */
            if (isspace(c))
            {
               /* word goes from start_of_word to p-1 */
               /*... do something with the word ... */
               for (c2=0,p2 = start_of_word; p2 <p; p2++,c2++)
                  ARGUV[ARGUC][c2] = (unsigned char) *p2;
               ARGUC++;

               state = DULL; /* back to "not in word, not in string" state */
            }
            continue; /* either still IN_WORD or we handled the end above */
      }
   }
}

static int execute_game_cmd(char* path)
{
   unsigned i;
   int driverIndex;
   int gameRot=0;
   bool CreateConf = (!strcmp(ARGUV[0],"-cc") || !strcmp(ARGUV[0],"-createconfig")) ? 1 : 0;
   bool Only1Arg   = (ARGUC == 1) ? 1 : 0;
   bool Mamecmdopt = strcmp(ARGUV[0],core) == 0 ? 1: 0;

if(!Only1Arg)CreateConf = (!strcmp(ARGUV[1],"-cc") || !strcmp(ARGUV[1],"-createconfig")) ? 1 : 0;
if (log_cb)log_cb(RETRO_LOG_INFO,"ARGUV[0]=%s\n",ARGUV[0]);

   FirstTimeUpdate = 1;

   screenRot = 0;

   for (i = 0; i < 64; i++)
      xargv_cmd[i]=NULL;

   /* split the path to directory and the name without the zip extension */
   if (parsePath(Only1Arg?path:ARGUV[ARGUC-1], MgamePath, MgameName) == 0)
   {
      if (log_cb)
      log_cb(RETRO_LOG_ERROR, "parse path failed! path=%s.\n", path);
      strcpy(MgameName,path );
   }

   if(Only1Arg)
   {
      /* split the path to directory and the name without the zip extension */
      if (parseSystemName(path, MsystemName) ==0)
      {
         if (log_cb)
            log_cb(RETRO_LOG_ERROR, "parse systemname failed! path=%s\n", path);
         strcpy(MsystemName,path );
      }
   }

   /* Find the game info. Exit if game driver was not found. */
   if (getGameInfo(Only1Arg?MgameName:ARGUV[0], &gameRot, &driverIndex,&arcade) == 0)
   {
      /* handle -cc/-createconfig case */
      if(CreateConf)
      {
         if (log_cb)
            log_cb(RETRO_LOG_INFO, "Create an %s config\n", core);
      }
      else
      {
         if (log_cb)
            log_cb(RETRO_LOG_WARN, "Game not found: %s\n", MgameName);

         if(Only1Arg)
         {
            //test if system exist (based on parent path)
            if (getGameInfo(MsystemName, &gameRot, &driverIndex,&arcade) == 0)
            {
               if (log_cb)
                  log_cb(RETRO_LOG_ERROR, "Driver not found: %s\n", MsystemName);
                if(!Mamecmdopt)return -2;
            }
         }
         else
             if(!Mamecmdopt)return -2;
      }
   }

   if(Only1Arg)
   {
      /* handle case where Arcade game exist and game on a System also */
      if(arcade==true)
      {
         /* test system */
         if (getGameInfo(MsystemName, &gameRot, &driverIndex,&arcade) == 0)
         {
            if (log_cb)
               log_cb(RETRO_LOG_ERROR, "System not found: %s\n", MsystemName);
         }
         else
         {
            if (log_cb)
               log_cb(RETRO_LOG_INFO, "System found: %s\n", MsystemName);
            arcade=false;
         }
      }
   }

   Set_Default_Option();

   Add_Option("-mouse");

   Set_Path_Option();

   if(Only1Arg)
   {
      /* Assume arcade/mess rom with full path or -cc   */
      if(CreateConf)
         Add_Option((char*)"-createconfig");
      else
      {
         Add_Option((char*)"-rp");
         Add_Option((char*)g_rom_dir);
         log_cb(RETRO_LOG_DEBUG, "System: %s, game: %s\n", MsystemName, MgameName);
         int num = driver_list::find(MsystemName);

         if(!arcade && (num != -1) && (strcmp(MsystemName, MgameName) != 0))
            Add_Option(MsystemName);
         Add_Option(MgameName);
      }
   }
   else if (Mamecmdopt){
         for(i = 1;i < ARGUC; i++)
            Add_Option(ARGUV[i]);
   }
   else
   {
      /* Pass all cmdline args */
      for(i = 0;i < ARGUC; i++)
         Add_Option(ARGUV[i]);
   }

   return 0;
}

#include <fstream>
#include <string>
static char CMDFILE[512];

int loadcmdfile(char *argv)
{
  std::ifstream cmdfile(argv);
  std::string cmdstr;

  if(cmdfile.is_open()){

    std::getline(cmdfile, cmdstr);
    cmdfile.close();

    sprintf(CMDFILE, "%s", cmdstr.c_str());

    return 1;
  }

  return 0;
}


/*
#ifdef __cplusplus
extern "C"
#endif
*/
int mmain2(int argc, const char *argv)
{
   unsigned i=0;
   osd_options options;
   //cli_options MRoptions;
   int result = 0;

   strcpy(gameName,argv);

   // handle cmd file
   if (strlen(gameName) >= strlen("cmd")){
           if(!core_stricmp(&gameName[strlen(gameName)-strlen("cmd")], "cmd"))
                       i=loadcmdfile(gameName);
   }

   if(i==1)
   {
      parse_cmdline(CMDFILE);
      if (log_cb)
         log_cb(RETRO_LOG_INFO, "Starting game from command line:%s\n",CMDFILE);

      result = execute_game_cmd(ARGUV[ARGUC-1]);

   }
   else
   if(experimental_cmdline)
   {
      parse_cmdline(argv);
      if (log_cb)
         log_cb(RETRO_LOG_INFO, "Starting game from command line:%s\n",gameName);

      result = execute_game_cmd(ARGUV[ARGUC-1]);
   }
   else
   {
      if (log_cb)
         log_cb(RETRO_LOG_INFO, "Starting game:%s\n",gameName);
      result = execute_game(gameName);
   }

   if (result < 0)
      return result;

   if (log_cb)
      log_cb(RETRO_LOG_DEBUG, "Parameters:\n");

   for (i = 0; i < PARAMCOUNT; i++)
   {
      xargv_cmd[i] = (char*)(XARGV[i]);
      if (log_cb)
         log_cb(RETRO_LOG_DEBUG, " %s\n",XARGV[i]);
   }


   //launch mmain from retromain
   result=mmain(PARAMCOUNT, ( char **)xargv_cmd);

   xargv_cmd[PARAMCOUNT - 2] = NULL;

   return result/*==0?0:1*/;
}
