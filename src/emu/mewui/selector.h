/***************************************************************************

    mewui/selector.h

    Internal MEWUI user interface.

***************************************************************************/

#pragma once

#ifndef __MSELECTOR_H__
#define __MSELECTOR_H__

enum
{
    SELECTOR_INIFILE = 1,
    SELECTOR_CATEGORY,
    SELECTOR_GAME,
    SELECTOR_SOFTWARE
};

//-------------------------------------------------
//  class selector menu
//-------------------------------------------------

class ui_menu_selector : public ui_menu
{
public:
    ui_menu_selector(running_machine &machine, render_container *container, std::string *_sel, UINT16 *_actual, int _total, int _category = 0, int _hover = 0);
    ui_menu_selector(running_machine &machine, render_container *container, std::vector<std::string> _sel, UINT16 *_actual, int _category = 0, int _hover = 0);
    virtual ~ui_menu_selector();
    virtual void populate();
    virtual void handle();
    virtual void custom_render(void *selectedref, float top, float bottom, float x, float y, float x2, float y2);

    virtual bool have_search() { return (search[0] != 0); }

private:
    enum { VISIBLE_GAMES_IN_SEARCH = 200 };
    char        search[40];
    UINT16      *selector;
    int         p_category, hover;
    bool        first_pass;
    std::vector<std::string> str_items;
    std::string *searchlist[VISIBLE_GAMES_IN_SEARCH + 1];

    void find_matches(const char *str, int count);
};

#endif /* __MSELECTOR_H__ */
