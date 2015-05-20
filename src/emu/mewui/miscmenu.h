/***************************************************************************

    mewui/miscmenu.h

    Internal MEWUI user interface.

***************************************************************************/

//-------------------------------------------------
//  class miscellaneous options menu
//-------------------------------------------------
class ui_menu_misc_options : public ui_menu
{
public:
    ui_menu_misc_options(running_machine &machine, render_container *container);
    virtual ~ui_menu_misc_options();
    virtual void populate();
    virtual void handle();
    virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

private:
    enum
    {
        REMEMBER_LAST_GAME = 1,
        ENLARGE_ARTS,
        HISTORY_ENABLED,
        MAMEINFO_ENABLED,
        COMMAND_ENABLED,
        CHEAT_ENABLED,
        MOUSE_ENABLED,
        CONFIRM_QUIT_ENABLED,
        SKIP_GAMEINFO_ENABLED,
        FORCED_4X3,
        USE_BGRND,
        LAST_MOPTION
    };

    bool m_options[LAST_MOPTION];
};
