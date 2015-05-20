/***************************************************************************

    mewui/datfile.c

    MEWUI data file.

***************************************************************************/

#include "emu.h"
#include "drivenum.h"
#include "mewui/datfile.h"
#include "mewui/utils.h"
#include <fstream>

//-------------------------------------------------
//  GLOBAL VARIABLES
//-------------------------------------------------

// Indices
tDatafileIndex *datfile_manager::hist_idx = NULL;
tDatafileIndex *datfile_manager::mame_idx = NULL;
tDatafileIndex *datfile_manager::mess_idx = NULL;
tDatafileIndex *datfile_manager::cmnd_idx = NULL;
tDatafileIndex *datfile_manager::sysi_idx = NULL;
tDatafileIndex *datfile_manager::story_idx = NULL;

sDataDrvIndex *datfile_manager::drv_idx = NULL;
sDataDrvIndex *datfile_manager::drvmess_idx = NULL;

tMenuIndex *datfile_manager::menu_idx = NULL;

std::vector<SoftwareListIndex> datfile_manager::sListIndex;

bool datfile_manager::m_history_file = false;
bool datfile_manager::m_mame_file = false;
bool datfile_manager::m_mess_file = false;
bool datfile_manager::m_command_file = false;
bool datfile_manager::m_sysinfo_file = false;
bool datfile_manager::m_story_file = false;

std::string datfile_manager::history_revision;
std::string datfile_manager::mame_revision;
std::string datfile_manager::mess_revision;
std::string datfile_manager::sysinfo_revision;
std::string datfile_manager::story_revision;

// Tags
static const char *TAG_BIO = "$bio";
static const char *TAG_INFO = "$info";
static const char *TAG_HISTORY_R = "## REVISION:";
static const char *TAG_MAME = "$mame";
static const char *TAG_COMMAND = "$cmd";
static const char *TAG_END = "$end";
static const char *TAG_DRIVER = "$drv";
static const char *TAG_STORY = "$story";
static const char *TAG_MAMEINFO_R = "# MAMEINFO.DAT";
static const char *TAG_MESSINFO_R = "#     MESSINFO.DAT";
static const char *TAG_SYSINFO_R = "# This file was generated on";
static const char *TAG_STORY_R = "# version";

//-------------------------------------------------
// ctor
//-------------------------------------------------
datfile_manager::datfile_manager(running_machine &machine) : m_machine(machine)
{
    m_mame_file = ParseOpen("mameinfo.dat");
    if (m_mame_file)
    {
        if (machine.options().mamefile())
            init_mameinfo();
        ParseClose();
    }

    m_command_file = ParseOpen("command.dat");
    if (m_command_file)
    {
        if (machine.options().cmndfile())
            init_command();
        ParseClose();
    }

    m_story_file = ParseOpen("story.dat");
    if (m_story_file)
    {
        if (machine.options().mamefile())
            init_storyinfo();
        ParseClose();
    }

    m_mess_file = ParseOpen("messinfo.dat");
    if (m_mess_file)
    {
        if (machine.options().mamefile())
            init_messinfo();
        ParseClose();
    }

    m_sysinfo_file = ParseOpen("sysinfo.dat");
    if (m_sysinfo_file)
    {
        if (machine.options().mamefile())
            init_sysinfo();
        ParseClose();
    }

    m_history_file = ParseOpen("history.dat");
    if (m_history_file)
    {
        if (machine.options().historyfile())
            init_history();
        ParseClose();
    }
}

//-------------------------------------------------
//  initialize sysinfo.dat index
//-------------------------------------------------

void datfile_manager::init_sysinfo()
{
    // check if already indexed
    if (!sysi_idx)
    {
        int swcount = 0;
        int count = index_datafile(&sysi_idx, swcount);
        osd_printf_verbose("Sysinfo.dat games found = %i\n", count);
        osd_printf_verbose("Rev = %s\n", sysinfo_revision.c_str());
    }
}

//-------------------------------------------------
//  initialize story.dat index
//-------------------------------------------------

void datfile_manager::init_storyinfo()
{
    // check if already indexed
    if (!story_idx)
    {
        int swcount = 0;
        int count = index_datafile(&story_idx, swcount);
        osd_printf_verbose("Story.dat games found = %i\n", count);
    }
}

//-------------------------------------------------
//  initialize history.dat index
//-------------------------------------------------

void datfile_manager::init_history()
{
    // check if already indexed
    if (!hist_idx)
    {
        int swcount = 0;
        int count = index_datafile(&hist_idx, swcount);
        osd_printf_verbose("History.dat games found = %i\n", count);
        osd_printf_verbose("History.dat softwares found = %i\n", swcount);
        osd_printf_verbose("Rev = %s\n", history_revision.c_str());
    }
}

//-------------------------------------------------
//  initialize mameinfo.dat index
//-------------------------------------------------

void datfile_manager::init_mameinfo()
{
    // check if already indexed
    if (!mame_idx || !drv_idx)
    {
        int drvcount = 0;
        int count = index_mame_mess_info(&mame_idx, &drv_idx, drvcount);
        osd_printf_verbose("Mameinfo.dat games found = %i\n", count);
        osd_printf_verbose("Mameinfo.dat drivers found = %d\n", drvcount);
        osd_printf_verbose("Rev = %s\n", mame_revision.c_str());
    }
}

//-------------------------------------------------
//  initialize messinfo.dat index
//-------------------------------------------------

void datfile_manager::init_messinfo()
{
    // check if already indexed
    if (!mess_idx || !drvmess_idx)
    {
        int drvcount = 0;
        int count = index_mame_mess_info(&mess_idx, &drvmess_idx, drvcount);
        osd_printf_verbose("Messinfo.dat games found = %i\n", count);
        osd_printf_verbose("Messinfo.dat drivers found = %d\n", drvcount);
        osd_printf_verbose("Rev = %s\n", mess_revision.c_str());
    }
}

//-------------------------------------------------
//  initialize command.dat index
//-------------------------------------------------

void datfile_manager::init_command()
{
    // check if already indexed
    if (!cmnd_idx)
    {
        int swcount = 0;
        int count = index_datafile(&cmnd_idx, swcount);
        osd_printf_verbose("Command.dat games found = %i\n", count);
    }
}

//-------------------------------------------------
//  load software info
//-------------------------------------------------

void datfile_manager::load_software_info(const char *soft_list, std::string &buffer, const char *soft_name)
{
    // Load history text
    if (ParseOpen("history.dat"))
    {
        if (!sListIndex.empty())
        {
            std::string readbuf;
            bool found = false;
            long s_offset = 0;

            // Find driver in datafile index
            for (int x = 0; !found && x < sListIndex.size(); x++)
                if (sListIndex[x].listname.compare(soft_list) == 0)
                    for (int y = 0; !found && y < sListIndex[x].item_list.size(); y++)
                        if (sListIndex[x].item_list[y].softname.compare(soft_name) == 0)
                        {
                            found = true;
                            s_offset = sListIndex[x].item_list[y].offset;
                        }

            if (!found)
            {
                ParseClose();
                return;
            }

            std::ifstream myfile(fp->fullpath());
            myfile.seekg(s_offset, myfile.beg);
            while (myfile.good())
            {
                // read from datafile
                std::getline(myfile, readbuf);

                // end entry when a end tag is encountered
                if (!core_strnicmp(TAG_END, readbuf.c_str(), strlen(TAG_END)))
                    break;

                // add this string to the buffer
                buffer.append(readbuf).append("\n");
            }
            myfile.close();
        }
        ParseClose();
    }
}

//-------------------------------------------------
//  load_data_info
//-------------------------------------------------

void datfile_manager::load_data_info(const game_driver *drv, std::string &buffer, int hm_type)
{
    tDatafileIndex *index_idx = NULL;
    sDataDrvIndex *driver_idx = NULL;
    const char *tag;
    std::string filename;

    switch (hm_type)
    {
        case MEWUI_HISTORY_LOAD:
            filename.assign("history.dat");
            tag = TAG_BIO;
            index_idx = hist_idx;
            break;
        case MEWUI_MAMEINFO_LOAD:
            filename.assign("mameinfo.dat");
            tag = TAG_MAME;
            index_idx = mame_idx;
            driver_idx = drv_idx;
            break;
        case MEWUI_SYSINFO_LOAD:
            filename.assign("sysinfo.dat");
            tag = TAG_BIO;
            index_idx = sysi_idx;
            break;
        case MEWUI_MESSINFO_LOAD:
            filename.assign("messinfo.dat");
            tag = TAG_MAME;
            index_idx = mess_idx;
            driver_idx = drvmess_idx;
            break;
        case MEWUI_STORY_LOAD:
            filename.assign("story.dat");
            tag = TAG_STORY;
            index_idx = story_idx;
            break;
    }

    if (ParseOpen(filename.c_str()))
    {
        // load game info
        if (index_idx)
            load_data_text(drv, buffer, index_idx, tag);

        // load driver info
        if (driver_idx)
            load_driver_text(drv, buffer, driver_idx, TAG_DRIVER);
        ParseClose();
    }
}

//-------------------------------------------------
//  load a game text into the buffer
//-------------------------------------------------

void datfile_manager::load_data_text(const game_driver *drv, std::string &buffer, tDatafileIndex *idx, const char *tag)
{
    tDatafileIndex *s_idx = idx;

    for (; s_idx->driver && s_idx->driver != drv; s_idx++) ;
    if (s_idx->driver == NULL)
    {
        int cloneof = driver_list::non_bios_clone(*drv);
        if (cloneof == -1)
            return;
        else
        {
            s_idx = idx;
            const game_driver *c_drv = &driver_list::driver(cloneof);
            for (; s_idx->driver && s_idx->driver != c_drv; s_idx++) ;
            if (s_idx->driver == NULL)
                return;
        }
    }

    std::string readbuf;
    std::ifstream myfile(fp->fullpath());

    if (!myfile.is_open())
        return;

    myfile.seekg(s_idx->offset, myfile.beg);
    while (myfile.good())
    {
        // read from datafile
        std::getline(myfile, readbuf);

        // end entry when a end tag is encountered
        if (!core_strnicmp(TAG_END, readbuf.c_str(), strlen(TAG_END)))
            break;

        // continue if a specific tag is encountered
        if (!core_strnicmp(tag, readbuf.c_str(), strlen(tag)))
            continue;

        // add this string to the buffer
        buffer.append(readbuf).append("\n");
    }
    myfile.close();
}

//-------------------------------------------------
//  load a driver name and offset into an
//  indexed array
//-------------------------------------------------
void datfile_manager::load_driver_text(const game_driver *drv, std::string &buffer, sDataDrvIndex *idx, const char *tag)
{
    std::string s;
    core_filename_extract_base(s, drv->source_file);
    for (; !idx->name.empty() && idx->name.compare(s.c_str()) != 0; idx++) ;

    // if driver not found, return
    if (idx->name.empty())
        return;

    std::string readbuf;
    std::ifstream myfile(fp->fullpath());

    if (!myfile.is_open())
        return;

    myfile.seekg(idx->offset, myfile.beg);
    buffer.append("--- DRIVER INFO ---\n\0").append("Driver: ").append(s).append("\n\n");
    while (myfile.good())
    {
        // read from datafile
        std::getline(myfile, readbuf);

        // end entry when a end tag is encountered
        if (!core_strnicmp(TAG_END, readbuf.c_str(), strlen(TAG_END)))
            break;

        // continue if a specific tag is encountered
        if (!core_strnicmp(tag, readbuf.c_str(), strlen(tag)))
            continue;

        // add this string to the buffer
        buffer.append(readbuf).append("\n");
    }
    myfile.close();
}

//-------------------------------------------------
//  load a game name and offset into an
//  indexed array (mameinfo)
//-------------------------------------------------
int datfile_manager::index_mame_mess_info(tDatafileIndex **_index, sDataDrvIndex **_index_drv, int &drvcount)
{
    tDatafileIndex  *idx;
    sDataDrvIndex   *idx_drv;
    int             count = 0;
    std::string     readbuf, name;
    int             t_mame = strlen(TAG_MAMEINFO_R);
    int             t_mess = strlen(TAG_MESSINFO_R);
    int             t_drv = strlen(TAG_DRIVER);
    int             t_tag = strlen(TAG_MAME);
    int             t_info = strlen(TAG_INFO);
    std::string     carriage("\r\n");

    // allocate index
    idx = *_index = global_alloc_array(tDatafileIndex, (driver_list::total() + 1));
    idx_drv = *_index_drv = global_alloc_array(sDataDrvIndex, (driver_list::total() + 1));

    std::ifstream myfile(fp->fullpath(), std::ifstream::binary);
    if (myfile.is_open())
    {
        // loop through datafile
        while (myfile.good())
        {
            std::getline(myfile, readbuf);

            if (mame_revision.empty() && readbuf.compare(0, t_mame, TAG_MAMEINFO_R) == 0)
            {
                size_t found = readbuf.find(" ", t_mame + 1);
                mame_revision.assign(readbuf.substr(t_mame + 1, found - t_mame));
            }

            else if (mess_revision.empty() && readbuf.compare(0, t_mess, TAG_MESSINFO_R) == 0)
            {
                size_t found = readbuf.find(" ", t_mess + 1);
                mess_revision.assign(readbuf.substr(t_mess + 1, found - t_mess));
            }

            // TAG_INFO
            else if (readbuf.compare(0, t_info, TAG_INFO) == 0)
            {
                std::string xid;
                std::getline(myfile, xid);

                size_t found = readbuf.find_last_not_of(carriage);
                if (found != std::string::npos)
                    name.assign(readbuf.substr(t_info + 1, found - t_info));
                else
                    name.assign(readbuf.substr(t_info + 1));

                if (xid.compare(0, t_tag, TAG_MAME) == 0)
                {
                    // validate driver
                    int game_index = driver_list::find(name.c_str());
                    if (game_index != -1)
                    {
                        idx->driver = &driver_list::driver(game_index);
                        idx->offset = myfile.tellg();
                        idx++;
                        count++;
                    }
                }

                else if (xid.compare(0, t_drv, TAG_DRIVER) == 0)
                {
                    idx_drv->name.assign(name);
                    idx_drv->offset = myfile.tellg();
                    idx_drv++;
                    drvcount++;
                }
            }
        }
        myfile.close();
    }

    // mark end of index
    idx->offset = 0L;
    idx->driver = NULL;
    idx_drv->offset = 0L;
    idx_drv->name.clear();
    return count;
}

//-------------------------------------------------
//  load a game name and offset into an
//  indexed array
//-------------------------------------------------
int datfile_manager::index_datafile(tDatafileIndex **_index, int &swcount)
{
    tDatafileIndex  *idx;
    int             count = 0;
    std::string     readbuf, name;
    int             t_hist = strlen(TAG_HISTORY_R);
    int             t_story = strlen(TAG_STORY_R);
    int             t_sysinfo = strlen(TAG_SYSINFO_R);
    int             t_info = strlen(TAG_INFO);
    int             t_bio = strlen(TAG_BIO);
    std::string     carriage("\r\n");

    // allocate index
    idx = *_index = global_alloc_array(tDatafileIndex, (driver_list::total() + 1));

    if (!idx)
        return 0;

    std::ifstream myfile(fp->fullpath(), std::ifstream::binary);
    if (myfile.is_open())
    {
        // loop through datafile
        while (myfile.good())
        {
            std::getline(myfile, readbuf);

            if (history_revision.empty() && readbuf.compare(0, t_hist, TAG_HISTORY_R) == 0)
            {
                size_t found = readbuf.find(" ", t_hist + 1);
                history_revision.assign(readbuf.substr(t_hist + 1, found - t_hist));
            }

            else if (sysinfo_revision.empty() && readbuf.compare(0, t_sysinfo, TAG_SYSINFO_R) == 0)
            {
                size_t found = readbuf.find(".", t_sysinfo + 1);
                sysinfo_revision.assign(readbuf.substr(t_sysinfo + 1, found - t_sysinfo));
            }

            else if (story_revision.empty() && readbuf.compare(0, t_story, TAG_STORY_R) == 0)
            {
                size_t found = readbuf.find(".", t_story + 1);
                story_revision.assign(readbuf.substr(t_story + 1, found - t_story));
            }

            // TAG_INFO identifies the driver
            else if (readbuf.compare(0, t_info, TAG_INFO) == 0)
            {
                int curpoint = t_info + 1;
                int ends = readbuf.length();
                while (curpoint < ends)
                {
                    // search for comma
                    size_t found = readbuf.find(",", curpoint);

                    // found it
                    if (found != std::string::npos)
                    {
                        // copy data and validate driver
                        int len = found - curpoint;
                        name.assign(readbuf.substr(curpoint, len));
                        strtrimspace(name);

                        // validate driver
                        int game_index = driver_list::find(name.c_str());

                        if (game_index != -1)
                        {
                            idx->driver = &driver_list::driver(game_index);
                            idx->offset = myfile.tellg();
                            idx++;
                            count++;
                        }

                        // update current point
                        curpoint = found + 1;
                    }

                    // if comma not found, copy data while until reach the end of string
                    else if (curpoint < ends)
                    {
                        name.assign(readbuf.substr(curpoint));
                        size_t found = name.find_last_not_of(carriage);
                        if (found != std::string::npos)
                            name.erase(found+1);

                        int game_index = driver_list::find(name.c_str());

                        if (game_index != -1)
                        {
                            idx->driver = &driver_list::driver(game_index);
                            idx->offset = myfile.tellg();
                            idx++;
                            count++;
                        }

                        // update current point
                        curpoint = ends;
                    }
                }
            }

            // search for software info
            else if (readbuf[0] == DATAFILE_TAG)
            {
                std::string readbuf_2;
                std::getline(myfile, readbuf_2);

                // TAG_BIO identifies software list
                if (readbuf_2.compare(0, t_bio, TAG_BIO) == 0)
                {
                    size_t eq_sign = readbuf.find("=");
                    std::string s_list(readbuf.substr(0, eq_sign));
                    std::string s_roms(readbuf.substr(eq_sign + 1));

                    s_list.erase(0, 1);
                    int ends = s_list.length();
                    int curpoint = 0;

                    while (curpoint < ends)
                    {
                        size_t found = s_list.find(",", curpoint);

                        // found it
                        if (found != std::string::npos)
                        {
                            int len = found - curpoint;
                            name.assign(s_list.substr(curpoint, len));
                            strtrimspace(name);
                            curpoint = found + 1;
                        }

                        else
                        {
                            name.assign(s_list);
                            strtrimspace(name);
                            curpoint = ends;
                        }

                        // search for a software list in the index, if not found then allocates
                        int list_index = find_or_allocate(name);
                        int cpoint = 0;
                        int cends = s_roms.length();

                        while (cpoint < cends)
                        {
                            // search for comma
                            size_t found = s_roms.find(",", cpoint);

                            // found it
                            if (found != std::string::npos)
                            {
                                // copy data
                                int len = found - cpoint;
                                name.assign(s_roms.substr(cpoint, len));
                                strtrimspace(name);

                                // add a SoftwareItem
                                SoftwareItem t_temp;
                                t_temp.softname.assign(name);
                                t_temp.offset = myfile.tellg();
                                sListIndex[list_index].item_list.push_back(t_temp);


                                // update current point
                                cpoint = found + 1;
                                swcount++;
                            }

                            else
                            {
                                // if reach the end, bail out
                                if (s_roms[cpoint] == CR || s_roms[cpoint] == LF)
                                    break;

                                // copy data
                                name.assign(s_roms.substr(cpoint));
                                size_t found = name.find_last_not_of(carriage);
                                if (found != std::string::npos)
                                    name.erase(found+1);

                                // add a SoftwareItem
                                SoftwareItem t_temp;
                                t_temp.softname.assign(name);
                                t_temp.offset = myfile.tellg();
                                sListIndex[list_index].item_list.push_back(t_temp);

                                // update current point
                                cpoint = cends;
                                count++;
                            }
                        }
                    }
                }
            }
        }
        myfile.close();
    }

    // mark end of index
    idx->offset = 0L;
    idx->driver = 0;
    return count;
}

//-------------------------------------------------
//  open up file for reading
//-------------------------------------------------

bool datfile_manager::ParseOpen(const char *filename)
{
    // Open file up in binary mode
    fp = global_alloc(emu_file(machine().options().history_path(), OPEN_FLAG_READ));

    if (fp->open(filename) != FILERR_NONE)
    {
        global_free(fp);
        fp = NULL;
        return FALSE;
    }
    return TRUE;
}

//-------------------------------------------------
//  closes the existing opened file (if any)
//-------------------------------------------------

void datfile_manager::ParseClose()
{
    // If the file is open, time for fclose.
    if (fp)
    {
        fp->close();
        global_free(fp);
    }

    fp = NULL;
}

//-------------------------------------------------
//  create the menu index
//-------------------------------------------------

int datfile_manager::index_menuidx(const game_driver *drv, tDatafileIndex *d_idx, tMenuIndex **_index)
{
    tMenuIndex *m_idx;
    const game_driver *gdrv;
    tDatafileIndex *gd_idx;
    int m_count = 0;
    std::string readbuf;

    gdrv = drv;

    do
    {
        gd_idx = d_idx;

        // find driver in datafile index
        for (; gd_idx->driver && gd_idx->driver != gdrv; gd_idx++) ;

        if (gd_idx->driver == gdrv)
            break;

        int cl = driver_list::clone(*gdrv);

        if (cl == -1)
            break;

        gdrv = &driver_list::driver(cl);

    }   while (!gd_idx->driver && gdrv);

    // driver not found in Data_file_index
    if (gdrv == 0)
        return 0;

    // seek to correct point in datafile
    std::ifstream myfile(fp->fullpath(), std::ifstream::binary);
    if (!myfile.is_open())
        return 0;

    myfile.seekg(gd_idx->offset, myfile.beg);

    // allocate index
    m_idx = *_index = auto_alloc_array(machine(), tMenuIndex, MAX_MENUIDX_ENTRIES);

    if (m_idx == NULL)
        return 0;

    // loop through between $cmd=
    while ((m_count < (MAX_MENUIDX_ENTRIES - 1)) && myfile.is_open() && myfile.good())
    {
        std::getline(myfile, readbuf);
        if (!core_strnicmp(TAG_INFO, readbuf.c_str(), strlen(TAG_INFO)))
            break;

        // TAG_COMMAND identifies the driver
        if (!core_strnicmp(TAG_COMMAND, readbuf.c_str(), strlen(TAG_COMMAND)))
        {
            std::getline(myfile, readbuf);
            m_idx->menuitem = readbuf;
            m_idx->offset = myfile.tellg();
            m_idx++;
            m_count++;
        }
    }

    myfile.close();

    // mark end of index
    m_idx->offset = 0L;
    m_idx->menuitem.clear();

    return m_count;
}

//-------------------------------------------------
//  free the menu index
//-------------------------------------------------

void datfile_manager::free_menuidx(tMenuIndex **_index)
{
    if (*_index)
    {
        auto_free(machine(), *_index);
        *_index = NULL;
    }
}

//-------------------------------------------------
//  load command text into the buffer
//-------------------------------------------------

int datfile_manager::load_command_text(std::string &buffer, tMenuIndex *m_idx, const int menu_sel)
{
    std::string readbuf;

    // open and seek to correct point in datafile
    std::ifstream myfile(fp->fullpath());

    if (!myfile.is_open())
        return 1;

    myfile.seekg((m_idx + menu_sel)->offset, myfile.beg);

    while (myfile.good())
    {
        // read from datafile
        std::getline(myfile, readbuf);

        // end entry when a tag is encountered
        if (!core_strnicmp(TAG_END, readbuf.c_str(), strlen(TAG_END)))
            break;

        // add this string to the buffer
        buffer.append(readbuf).append("\n");;
    }

    myfile.close();

    return 0;
}

//-------------------------------------------------
//  initializes the index
//-------------------------------------------------

bool datfile_manager::find_command(const game_driver *drv)
{
    // try to open command datafile
    if (ParseOpen("command.dat"))
    {
        // create menu_index
        int status = index_menuidx(drv, cmnd_idx, &menu_idx);

        ParseClose();

        if (!status)
        {
            free_menuidx(&menu_idx);
            return false;
        }

        return true;
    }

    return false;
}

//-------------------------------------------------
//  load command text
//-------------------------------------------------

void datfile_manager::load_command_info(std::string &buffer, const int menu_sel)
{
    // try to open command datafile
    if (ParseOpen("command.dat"))
    {
        load_command_text(buffer, menu_idx, menu_sel);
        ParseClose();
    }
}

//-------------------------------------------------
//  load submenu item for command.dat
//-------------------------------------------------

void datfile_manager::command_sub_menu(const game_driver *drv, std::vector<std::string> &menu_item)
{
    if (!find_command(drv))
        return;

    if (menu_idx)
    {
        tMenuIndex *m_idx = menu_idx;
        for (; !m_idx->menuitem.empty(); m_idx++)
            menu_item.push_back(m_idx->menuitem);
    }
}

//-------------------------------------------------
//  free datafile indices
//-------------------------------------------------

void datfile_manager::free_dat_index()
{
    // free command menu index
    if (menu_idx)
        free_menuidx(&menu_idx);

    // free history index
    if (hist_idx)
        global_free_array(hist_idx);

    // free mameinfo index
    if (mame_idx)
        global_free_array(mame_idx);

    // free sysinfo index
    if (sysi_idx)
        global_free_array(sysi_idx);

    // free messinfo index
    if (mess_idx)
        global_free_array(mess_idx);

    // free command index
    if (cmnd_idx)
        global_free_array(cmnd_idx);

    // free story index
    if (story_idx)
        global_free_array(story_idx);

    // free drivers index
    if (drv_idx)
        global_free_array(drv_idx);

    // free drivers index
    if (drvmess_idx)
        global_free_array(drvmess_idx);
}

int datfile_manager::find_or_allocate(std::string name)
{
    int x = 0;
    for (; x < sListIndex.size(); x++)
        if (sListIndex[x].listname.compare(name) == 0)
            return x;

    if (x == sListIndex.size())
    {
        SoftwareListIndex tmp;
        tmp.listname.assign(name);
        sListIndex.push_back(tmp);
    }

    return x;
}
