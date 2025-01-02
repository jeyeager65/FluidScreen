// Top Level
volatile int menuLevel = 0;
volatile int menuSelected = 0;
const int menuSize = 5;
const String menu[menuSize] = { "Main", "Jog", "Home", "Macro", "Other" };

// Second Level
volatile int jogMenuSelected = -1;
const int jogMenuSize = 5;
const String jogMenu[jogMenuSize] = { "Back", "XY", "X", "Y", "Z" };

volatile int homeMenuSelected = -1;
const int homeMenuSize = 5;
const String homeMenu[homeMenuSize] = { "Back", "All", "X", "Y", "Z" };

volatile int macroMenuSelected = -1;
const int macroMenuSize = 7;
const String macroMenu[macroMenuSize] = { "Back", "1", "2", "3", "4", "5", "6" };

volatile int otherMenuSelected = -1;
const int otherMenuSize = 4;
const String otherMenu[otherMenuSize] = { "Back", "Conn", "Setup", "Info" };

volatile int controlMenuSelected = -1;
const int controlMenuSize = 5;
const String controlMenu[controlMenuSize] = { "Back", "Hold", "Resume", "Unlock", "Reset"};