#ifndef _SPRITEBUTTON_H
#define _SPRITEBUTTON_H

#include "game/gui/component.h"
#include "game/gui/text_render.h"
#include "video/surface.h"

enum {
    VALIGN_TOP = 0,
    VALIGN_MIDDLE,
    VALIGN_BOTTOM,
};

enum {
    HALIGN_LEFT = 0,
    HALIGN_CENTER,
    HALIGN_RIGHT
};

typedef void (*spritebutton_click_cb)(component *c, void *userdata);

component* spritebutton_create(const font *font, const char *text, surface *img, int disabled, spritebutton_click_cb cb, void *userdata);

void spritebutton_set_text_style(component *c, text_settings *set);
text_settings* spritebutton_get_text_style(component *c);

#endif // _SPRITEBUTTON_H
