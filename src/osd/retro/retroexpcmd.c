
//Args for experimental_cmdline
static char ARGUV[32][1024];
static unsigned char ARGUC=0;

void parse_cmdline( const char *argv ){

	static char buffer[512*4];
	
	strcpy(buffer,argv);
	strcat(buffer," \0");
	
	char *p,*p2,*start_of_word;
	int c,c2;
	enum states { DULL, IN_WORD, IN_STRING } state = DULL;

	for (p = buffer; *p != '\0'; p++) {
		c = (unsigned char) *p; /* convert to unsigned char for is* functions */
		switch (state) {
			case DULL: /* not in a word, not in a double quoted string */
				if (isspace(c)) {
					/* still not in a word, so ignore this char */
				continue;
			}
			/* not a space -- if it's a double quote we go to IN_STRING, else to IN_WORD */
			if (c == '"') {
				state = IN_STRING;
				start_of_word = p + 1; /* word starts at *next* char, not this one */
				continue;
			}
			state = IN_WORD;
			start_of_word = p; /* word starts here */
			continue;

		case IN_STRING:
			/* we're in a double quoted string, so keep going until we hit a close " */
			if (c == '"') {
				/* word goes from start_of_word to p-1 */
				//... do something with the word ...
				for (c2=0,p2 = start_of_word; p2 < p; p2++,c2++)ARGUV[ARGUC][c2] = (unsigned char) *p2;
				ARGUC++; 
				
				state = DULL; /* back to "not in word, not in string" state */
			}
			continue; /* either still IN_STRING or we handled the end above */

		case IN_WORD:
			/* we're in a word, so keep going until we get to a space */
			if (isspace(c)) {
				/* word goes from start_of_word to p-1 */
				//... do something with the word ...
				for (c2=0,p2 = start_of_word; p2 <p; p2++,c2++)ARGUV[ARGUC][c2] = (unsigned char) *p2;
				ARGUC++; 
				
				state = DULL; /* back to "not in word, not in string" state */
			}
			continue; /* either still IN_WORD or we handled the end above */
		}	
	}
	
}

int executeGame_cmd(char* path) {

	int gameRot=0;
	int driverIndex;

	bool CreateConf = ( strcmp(ARGUV[0],"-cc") == 0 || strcmp(ARGUV[0],"-createconfig") == 0 )?1:0;
	bool Only1Arg =   ( ARGUC==1 )?1:0;

	FirstTimeUpdate = 1;

	screenRot = 0;

	for (int i = 0; i<64; i++)xargv_cmd[i]=NULL;

	//split the path to directory and the name without the zip extension
	if (parsePath(Only1Arg?path:ARGUV[ARGUC-1], MgamePath, MgameName) == 0) {
		write_log("parse path failed! path=%s\n", path);
		strcpy(MgameName,path );
	}

	if(Only1Arg){
	//split the path to directory and the name without the zip extension
		if (parseSystemName(path, MsystemName) ==0){
			write_log("parse systemname failed! path=%s\n", path);
			strcpy(MsystemName,path );		
		}
	}

	//Find the game info. Exit if game driver was not found.
	if (getGameInfo(Only1Arg?MgameName:ARGUV[0], &gameRot, &driverIndex,&arcade) == 0) {

		// handle -cc/-createconfig case
		if(CreateConf){
			write_log("create an %s config\n", core);
		}
		else {
			write_log("game not found: %s\n", MgameName);
			if(Only1Arg){
				//test if system exist (based on parent path)
		   		if (getGameInfo(MsystemName, &gameRot, &driverIndex,&arcade) == 0) {
		      			write_log("driver not found: %s\n", MsystemName);
   		         		return -2;
	       			}
			}
			else return -2;
		}
	}


	if(Only1Arg){

		// handle case where Arcade game exist and game on a System also
		if(arcade==true){
			// test system
		   	if (getGameInfo(MsystemName, &gameRot, &driverIndex,&arcade) == 0) {
		      		write_log("System not found: %s\n", MsystemName);   		         	
	       		}
			else {
				write_log("System found: %s\n", MsystemName);   
				arcade=false;
			}
		} 
	
	} 

	Set_Default_Option();

        Add_Option("-mouse");

	Set_Path_Option();

	if(Only1Arg){// Assume arcade/mess rom with full path or -cc  

		if(CreateConf)
			Add_Option((char*)"-createconfig");
		else {
			Add_Option((char*)"-rp");	
			Add_Option((char*)g_rom_dir);	
			if(!arcade)Add_Option(MsystemName);
			Add_Option(MgameName);
		}
	}
	else { // Pass all cmdline args
		for(int i=0;i<ARGUC;i++)
			Add_Option(ARGUV[i]);
	}

	
	return 0;
}

