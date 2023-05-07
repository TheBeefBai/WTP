#ifndef _PULUI_
#define _PULUI_
#include <game/UI/Ctrl/UIControl.hpp>
#include <game/UI/Ctrl/Manipulator.hpp>

namespace Pulsar {
namespace UI {

void ChangeImage(LayoutUIControl& control, const char* paneName, const char* tplName);

//Simple enum of BMGs, making any change much easier as everything is centralized here
enum BMG {
    //vanilla
    BMG_SAVED_GHOST = 0x45b,
    BMG_FINISH = 0x4b5,
    BMG_TIME_TRIALS = 0xbb9,
    BMG_CHOOSE_GHOST_DATA = 0xd4f,
    BMG_REGCUPS = 0x23f0,
    BMG_REGS = 0x2454,
    BMG_BATTLE = 0x24B8,
    BMG_MII_NAME = 0x251d,

    //race
    BMG_INFO_DISPLAY = 0x2700,
    BMG_TROPHY_EARNED = 0x2701,
    BMG_GP_RACE = 0x2702,
    BMG_CHOOSE_NEXT = 0x2703, //TO ADD to race
    BMG_DEBUG = 0x2704,

    //Menu
    BMG_TT_MODE_BUTTONS = 0x2800,
    BMG_TT_MODE_BOTTOM_CUP = 0x2810,
    BMG_TT_MODE_BOTTOM_SINGLE = 0x2820,
    BMG_TT_BOTTOM_CUP = 0x2830,
    BMG_TT_BOTTOM_COURSE = 0x2831,
    BMG_NO_TROPHY = 0x2832,
    BMG_TROPHY = 0x2833,
    BMG_SELECT_GHOST = 0x2840,
    BMG_GHOST_SELECTED = 0x2841,
    BMG_GHOST_SELECTED_COUNTER = 0x2842,
    BMG_RANDOM_TROPHIES_MISSING = 0x2843,
    BMG_RANDOM_CUP = 0x2844,
    BMG_RANDOM_TROPHIES_MISSING_GCN = 0x2845,
    BMG_RANDOM_ALL_TROPHIES_GCN = 0x2846,
    BMG_DATE = 0x2847,
    BMG_NINTENDO = 0x2848,
    BMG_TEXT = 0x2849,
    BMG_TEAM_SELECT = 0x284a,


    BMG_TEAMS_BOTTOM = 0x2910,
    BMG_TEAMS_ENABLED = 0x2912,
    BMG_TEAMS_DISABLED = 0x2913,


    //Race
    BMG_SETTINGS_PAGE = 0x2f01,
    BMG_SETTINGS_BOTTOM = 0x2f10,
    BMG_SETTINGS_TITLE = 0x2f20,
    BMG_RADIO_SETTINGS = 0x3000,
    BMG_SCROLLER_SETTINGS = 0x3700,

    BMG_CUPS = 0x10000,
    BMG_TRACKS = 0x20000,
    BMG_AUTHORS = 0x30000
};

const char controlFolder[] = "control";
const char buttonFolder[] = "button";
const char raceFolder[] = "game_image";
const char bgFolder[] = "bg";

}//namespace UI
}//namespace Pulsar

#endif