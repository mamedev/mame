
int executeGame(char* path)
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

#ifdef WANT_MAME	
   //find if the driver exists for MgameName, if not, exit
   if (getGameInfo(MgameName, &gameRot, &driverIndex,&arcade) == 0)
   {
      if (log_cb)
         log_cb(RETRO_LOG_ERROR, "Driver not found: %s\n",MgameName);
      return -2;
   }	
#else
   //find if the driver exists for MgameName, if not, check if a driver exists for MsystemName, if not, exit
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

   // handle case where Arcade game exist and game on a System also
   if(arcade==true)
   {
      if (log_cb)
         log_cb(RETRO_LOG_ERROR, "System not found: %s\n",MsystemName);	         	

      // test system
      if (getGameInfo(MsystemName, &gameRot, &driverIndex,&arcade) != 0) 
         arcade=false;
   }

#endif	

   // useless ?
   if (tate)
   {
      //horizontal game
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

   Set_Path_Option();

   // useless ?
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

#ifdef WANT_MAME
   sprintf(tmp_dir, "%s", MgamePath);
   Add_Option((char*)(tmp_dir));		   
   if(!boot_to_osd_enable)
      Add_Option(MgameName);

#else

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
         if (strcmp(mediaType, "-rom") == 0)
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



#endif 	 	 

   return 0;
} 

