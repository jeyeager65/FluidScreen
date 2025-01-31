// Top Level
volatile int menuLevel = 0;
volatile int menuSelected = 0;
const int menuSize = 5;
const String menu[menuSize] = { "Main", "Jog", "Home", "Macro", "Other" };
const String defaultMessage = "Select a menu item to access the submenu.";

// Second Level
volatile int jogMenuSelected = -1;
const int jogMenuSize = 5;
const String jogMenu[jogMenuSize] = { "Back", "XY", "X", "Y", "Z" };
const String jogMessage = "Highlight a Jog Mode to enable joystick control.";

volatile int homeMenuSelected = -1;
const int homeMenuSize = 5;
const String homeMenu[homeMenuSize] = { "Back", "All", "X", "Y", "Z" };
const String homeMessage = "Select an Axis option to home.";

volatile int macroMenuSelected = -1;
const int macroMenuSize = 7;
const String macroMenu[macroMenuSize] = { "Back", "1", "2", "3", "4", "5", "6" };
const String macroMessage = "Highlight a Macro number to view details. Select to execute.";

volatile int otherMenuSelected = -1;
const int otherMenuSize = 4;
const String otherMenu[otherMenuSize] = { "Back", "Conn", "Setup", "Info" };
const String otherMessage = "Highlight an option to view details.";
const String otherConnMessage = "Connect to WiFi and FluidNC if currently disconnected.";
const String otherSetupMessage = "Enter Setup Mode to configure connection settings and theme colors.";
const String otherInfoMessage = "View current connection details.";

volatile int controlMenuSelected = -1;
const int controlMenuSize = 5;
const String controlMenu[controlMenuSize] = { "Back", "Hold", "Resume", "Unlock", "Reset"};
const String controlMenuMessage = "Select an Action to perform.";
const String controlHoldMessage = "Select to perform a Feed Hold command.";
const String controlResumeMessage = "Select to perform a Cycle Start/Resume command.";
const String controlUnlockMessage = "Select to perform an Unlock to clear the alarm state.";
const String controlResetMessage = "Select to perform a Soft Reset. This will raise an alarm if in motion.";