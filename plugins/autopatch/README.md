This plugin will locate and apply an IPS file found that matches a software system and name.
Current only SNES and Mega Drive/Genesis are supported, and only IPS format patches are 
supported. Place IPS patches in a directory to match the software list item you are running.
For example, your autopatch directory should look something like this:

plugins/
plugins/autopatch/
plugins/autopatch/init.lua
plugins/autopatch/plugin.json
plugins/autopatch/genesis
plugins/autopatch/genesis/sonic.ips
plugins/autopatch/snes/
plugins/autopatch/snes/smwu.ips

Execute MAME using the -plugin autopatch command line arguments, or edit the plugin.json file to set 
the "start" variable to "true". If the IPS patching is successful there will be an indication on the 
command line showing which file was applied. The patch is applied only in memory, the original ROM or 
software list contents are not written to file. Note that the IPS format is simple and doesn't include 
any validation for the patch or the file it is being applied to.
