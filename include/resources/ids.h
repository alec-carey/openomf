#ifndef _IDS_H
#define _IDS_H

enum RESOURCE_ID {
    BK_INTRO = 0,
    BK_MENU,
    BK_END,
    BK_END1,
    BK_END2,
    BK_CREDITS,
    BK_MECHLAB,
    BK_VS,
    BK_MELEE,
    BK_NEWSROOM,
    BK_ARENA0,
    BK_ARENA1,
    BK_ARENA2,
    BK_ARENA3,
    BK_ARENA4,
    BK_NORTHAM,
    BK_KATUSHAI,
    BK_WAR,
    BK_WORLD,
    AF_JAGUAR,
    AF_SHADOW,
    AF_THORN,
    AF_PYROS,
    AF_ELECTRA,
    AF_KATANA,
    AF_SHREDDER,
    AF_FLAIL,
    AF_GARGOYLE,
    AF_CHRONOS,
    AF_NOVA,
    PSM_END,
    PSM_MENU,
    PSM_ARENA0,
    PSM_ARENA1,
    PSM_ARENA2,
    PSM_ARENA3,
    PSM_ARENA4,
    DAT_SOUNDS,
    DAT_ENGLISH,
    DAT_GRAPHCHR,
    DAT_CHARSMAL,
    DAT_ALTPALS,
    NUMBER_OF_RESOURCES
};

const char* get_resource_file(unsigned int id);
const char* get_resource_name(unsigned int id);
char* get_path_by_id(int id);

// returns 0 if there are no missing files, otherwise 1 is returned
int validate_resource_path(char **missingfile);

int is_arena(unsigned int id);
int is_scene(unsigned int id);
int is_har(unsigned int id);
int is_music(unsigned int id);

int rand_arena();

#endif // _IDS_H
