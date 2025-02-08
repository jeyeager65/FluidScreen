#ifndef FLUID_SCREEN_SETTINGS_H
#define FLUID_SCREEN_SETTINGS_H

#include <Arduino.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <Preferences.h>

const char* prefNamespace = "FluidSettings";
Preferences preferences;

struct FluidScreenConnection {
    String SSID, Password, Address;
    uint16_t Port;
};

struct FluidScreenJogSettings {
    uint16_t MaxFeedrateXY, MaxFeedrateZ;
};

struct FluidScreenThemeColors {
    uint16_t Primary, Secondary, Background, BackgroundAlt, BackgroundPanel, 
             Border, Selected, Text, QrCode, QrCodeBg;
};

struct FluidScreenStatusColors {
    uint16_t Default, Idle, Jog, Run, Hold, Home, Alarm, Disconnect, Configuration;
};

struct FluidScreenSettings {
    bool IsInitialized;
    bool IsConfigMode;
    struct FluidScreenConnection Connection;
    struct FluidScreenJogSettings JogSettings;
    struct FluidScreenThemeColors ThemeColors;
    struct FluidScreenStatusColors StatusColors;  
};

struct FluidScreenSettings DefaultSettings, CurrentSettings, NewSettings;

void initDefaultSettings() {
    DefaultSettings.IsInitialized = true;
    DefaultSettings.IsConfigMode = true;
    
    DefaultSettings.Connection.SSID = "";
    DefaultSettings.Connection.Password = "";
    DefaultSettings.Connection.Address = "fluidnc.local";
    DefaultSettings.Connection.Port = 82;

    DefaultSettings.JogSettings.MaxFeedrateXY = 2400;
    DefaultSettings.JogSettings.MaxFeedrateZ = 800;

    DefaultSettings.ThemeColors.Primary         = 0x01EC;  // #003d63
    DefaultSettings.ThemeColors.Secondary       = 0x434F;  // #42697b
    DefaultSettings.ThemeColors.Background      = 0x0841;  // #080808
    DefaultSettings.ThemeColors.BackgroundAlt   = 0x39C7;  // #3a393a
    DefaultSettings.ThemeColors.BackgroundPanel = 0x2104;  // #212021
    DefaultSettings.ThemeColors.Border          = 0xBE19;  // #bdc2ce
    DefaultSettings.ThemeColors.Selected        = 0x0397;  // #0071bd
    DefaultSettings.ThemeColors.Text            = 0xFFFF;  // #ffffff
    DefaultSettings.ThemeColors.QrCode          = 0x0000;  // #000000
    DefaultSettings.ThemeColors.QrCodeBg        = 0xFFFF;  // #ffffff

    DefaultSettings.StatusColors.Default        = 0x7BEF;  // #7b7d7b
    DefaultSettings.StatusColors.Idle           = 0x7BEF;  // #7b7d7b
    DefaultSettings.StatusColors.Jog            = 0x000F;  // #00007b
    DefaultSettings.StatusColors.Run            = 0x03E0;  // #007d00
    DefaultSettings.StatusColors.Hold           = 0xE660;  // #e6ce00
    DefaultSettings.StatusColors.Home           = 0x03EF;  // #007d7b
    DefaultSettings.StatusColors.Alarm          = 0xF800;  // #ff0000
    DefaultSettings.StatusColors.Disconnect     = 0x780F;  // #7b007b
    DefaultSettings.StatusColors.Configuration  = 0xFB80;  // #ff7100
}

void debugSettings(FluidScreenSettings &settings) {
    Serial.printf("IsInitialized: %i\n", settings.IsInitialized);
    Serial.printf("IsConfigMode: %i\n", settings.IsConfigMode);
    Serial.printf("Connection.SSID: %s\n", settings.Connection.SSID);
    Serial.printf("Connection.Password: %s\n", settings.Connection.Password);
    Serial.printf("Connection.Address: %s\n", settings.Connection.Address);
    Serial.printf("Connection.Port: %i\n", settings.Connection.Port);
    Serial.printf("JogSettings.MaxFeedrateXY: %i\n", settings.JogSettings.MaxFeedrateXY);
    Serial.printf("JogSettings.MaxFeedrateZ: %i\n", settings.JogSettings.MaxFeedrateZ);
    Serial.printf("ThemeColors.Primary: %u\n", settings.ThemeColors.Primary);
    Serial.printf("ThemeColors.Secondary: %u\n", settings.ThemeColors.Secondary);
    Serial.printf("ThemeColors.Background: %u\n", settings.ThemeColors.Background);
    Serial.printf("ThemeColors.BackgroundAlt: %u\n", settings.ThemeColors.BackgroundAlt);
    Serial.printf("ThemeColors.BackgroundPanel: %u\n", settings.ThemeColors.BackgroundPanel);
    Serial.printf("ThemeColors.Border: %u\n", settings.ThemeColors.Border);
    Serial.printf("ThemeColors.Selected: %u\n", settings.ThemeColors.Selected);
    Serial.printf("ThemeColors.Text: %u\n", settings.ThemeColors.Text);
    Serial.printf("ThemeColors.QrCode: %u\n", settings.ThemeColors.QrCode);
    Serial.printf("ThemeColors.QrCodeBg: %u\n", settings.ThemeColors.QrCodeBg);
    Serial.printf("StatusColors.Default: %u\n", settings.StatusColors.Default);
    Serial.printf("StatusColors.Idle: %u\n", settings.StatusColors.Idle);
    Serial.printf("StatusColors.Jog: %u\n", settings.StatusColors.Jog);
    Serial.printf("StatusColors.Run: %u\n", settings.StatusColors.Run);
    Serial.printf("StatusColors.Hold: %u\n", settings.StatusColors.Hold);
    Serial.printf("StatusColors.Home: %u\n", settings.StatusColors.Home);
    Serial.printf("StatusColors.Alarm: %u\n", settings.StatusColors.Alarm);
    Serial.printf("StatusColors.Disconnect: %u\n", settings.StatusColors.Disconnect);
    Serial.printf("StatusColors.Configuration: %u\n", settings.StatusColors.Configuration);
}

void saveSettings(FluidScreenSettings &settings, bool checkForChange = false) {
    Serial.println("Saving Settings");
    if(preferences.begin(prefNamespace, false)) {
        
        if(!checkForChange || settings.IsInitialized != CurrentSettings.IsInitialized) { preferences.putBool("IsInitialized", settings.IsInitialized); }
        if(!checkForChange || settings.IsConfigMode != CurrentSettings.IsConfigMode) { preferences.putBool("IsConfigMode", settings.IsConfigMode); }
        if(!checkForChange || settings.Connection.SSID != CurrentSettings.Connection.SSID) { preferences.putString("SSID", settings.Connection.SSID); }
        if(!checkForChange || settings.Connection.Password != CurrentSettings.Connection.Password) { preferences.putString("Password", settings.Connection.Password); }
        if(!checkForChange || settings.Connection.Address != CurrentSettings.Connection.Address) { preferences.putString("Address", settings.Connection.Address); }
        if(!checkForChange || settings.Connection.Port != CurrentSettings.Connection.Port) { preferences.putUShort("Port", settings.Connection.Port); }
        
        if(!checkForChange || settings.JogSettings.MaxFeedrateXY != CurrentSettings.JogSettings.MaxFeedrateXY) { preferences.putUShort("MaxFeedrateXY", settings.JogSettings.MaxFeedrateXY); }
        if(!checkForChange || settings.JogSettings.MaxFeedrateZ != CurrentSettings.JogSettings.MaxFeedrateZ) { preferences.putUShort("MaxFeedrateZ", settings.JogSettings.MaxFeedrateZ); }

        if(!checkForChange || settings.ThemeColors.Primary != CurrentSettings.ThemeColors.Primary) { preferences.putUShort("Primary", settings.ThemeColors.Primary); }
        if(!checkForChange || settings.ThemeColors.Secondary != CurrentSettings.ThemeColors.Secondary) { preferences.putUShort("Secondary", settings.ThemeColors.Secondary); }
        if(!checkForChange || settings.ThemeColors.Background != CurrentSettings.ThemeColors.Background) { preferences.putUShort("Background", settings.ThemeColors.Background); }
        if(!checkForChange || settings.ThemeColors.BackgroundAlt != CurrentSettings.ThemeColors.BackgroundAlt) { preferences.putUShort("BackgroundAlt", settings.ThemeColors.BackgroundAlt); }
        if(!checkForChange || settings.ThemeColors.BackgroundPanel != CurrentSettings.ThemeColors.BackgroundPanel) { preferences.putUShort("BackgroundPanel", settings.ThemeColors.BackgroundPanel); }
        if(!checkForChange || settings.ThemeColors.Border != CurrentSettings.ThemeColors.Border) { preferences.putUShort("Border", settings.ThemeColors.Border); }
        if(!checkForChange || settings.ThemeColors.Selected != CurrentSettings.ThemeColors.Selected) { preferences.putUShort("Selected", settings.ThemeColors.Selected); }
        if(!checkForChange || settings.ThemeColors.Text != CurrentSettings.ThemeColors.Text) { preferences.putUShort("Text", settings.ThemeColors.Text); }
        if(!checkForChange || settings.ThemeColors.QrCode != CurrentSettings.ThemeColors.QrCode) { preferences.putUShort("QrCode", settings.ThemeColors.QrCode); }
        if(!checkForChange || settings.ThemeColors.QrCodeBg != CurrentSettings.ThemeColors.QrCodeBg) { preferences.putUShort("QrCodeBg", settings.ThemeColors.QrCodeBg); }

        if(!checkForChange || settings.StatusColors.Default != CurrentSettings.StatusColors.Default) { preferences.putUShort("Default", settings.StatusColors.Default); }
        if(!checkForChange || settings.StatusColors.Idle != CurrentSettings.StatusColors.Idle) { preferences.putUShort("Idle", settings.StatusColors.Idle); }
        if(!checkForChange || settings.StatusColors.Jog != CurrentSettings.StatusColors.Jog) { preferences.putUShort("Jog", settings.StatusColors.Jog); }
        if(!checkForChange || settings.StatusColors.Run != CurrentSettings.StatusColors.Run) { preferences.putUShort("Run", settings.StatusColors.Run); }
        if(!checkForChange || settings.StatusColors.Hold != CurrentSettings.StatusColors.Hold) { preferences.putUShort("Hold", settings.StatusColors.Hold); }
        if(!checkForChange || settings.StatusColors.Home != CurrentSettings.StatusColors.Home) { preferences.putUShort("Home", settings.StatusColors.Home); }
        if(!checkForChange || settings.StatusColors.Alarm != CurrentSettings.StatusColors.Alarm) { preferences.putUShort("Alarm", settings.StatusColors.Alarm); }
        if(!checkForChange || settings.StatusColors.Disconnect != CurrentSettings.StatusColors.Disconnect) { preferences.putUShort("Disconnect", settings.StatusColors.Disconnect); }
        if(!checkForChange || settings.StatusColors.Configuration != CurrentSettings.StatusColors.Configuration) { preferences.putUShort("Configuration", settings.StatusColors.Configuration); }

        preferences.end();
        Serial.println("Settings Saved");
    }
    else {
        Serial.println("Failed to Save Settings");
    }   
}

void setConfigMode(bool configMode) {
    Serial.println("Updating Config Mode");
    if(preferences.begin(prefNamespace, false)) {
        Serial.println("Namespace Opened or Created");
        preferences.putBool("IsConfigMode", configMode);
        preferences.end();
    }
}

void settingsSetup() {
    Serial.println("Setup Settings");

    initDefaultSettings();

    bool prefBegin = preferences.begin(prefNamespace, true);
    if(!prefBegin) {
        saveSettings(DefaultSettings, false);
    }

    // Check if initialized
    CurrentSettings.IsInitialized = preferences.isKey("IsInitialized");
    if(CurrentSettings.IsInitialized) { // Can also set to false to force reinitialization
        CurrentSettings.IsInitialized = preferences.getBool("IsInitialized", false);
    }

    if(!CurrentSettings.IsInitialized) {
        preferences.end();
        saveSettings(DefaultSettings, false);
        preferences.begin(prefNamespace, true);
    }

    CurrentSettings.IsConfigMode = preferences.getBool("IsConfigMode", true);

    CurrentSettings.Connection.SSID = preferences.getString("SSID", DefaultSettings.Connection.SSID);
    CurrentSettings.Connection.Password = preferences.getString("Password", DefaultSettings.Connection.Password);
    CurrentSettings.Connection.Address = preferences.getString("Address", DefaultSettings.Connection.Address);
    CurrentSettings.Connection.Port = preferences.getUShort("Port", DefaultSettings.Connection.Port);

    CurrentSettings.JogSettings.MaxFeedrateXY = preferences.getUShort("MaxFeedrateXY", DefaultSettings.JogSettings.MaxFeedrateXY);
    CurrentSettings.JogSettings.MaxFeedrateZ = preferences.getUShort("MaxFeedrateZ", DefaultSettings.JogSettings.MaxFeedrateZ);

    CurrentSettings.ThemeColors.Primary = preferences.getUShort("Primary", DefaultSettings.ThemeColors.Primary);
    CurrentSettings.ThemeColors.Secondary = preferences.getUShort("Secondary", DefaultSettings.ThemeColors.Secondary);
    CurrentSettings.ThemeColors.Background = preferences.getUShort("Background", DefaultSettings.ThemeColors.Background);
    CurrentSettings.ThemeColors.BackgroundAlt = preferences.getUShort("BackgroundAlt", DefaultSettings.ThemeColors.BackgroundAlt);
    CurrentSettings.ThemeColors.BackgroundPanel = preferences.getUShort("BackgroundPanel", DefaultSettings.ThemeColors.BackgroundPanel);
    CurrentSettings.ThemeColors.Border = preferences.getUShort("Border", DefaultSettings.ThemeColors.Border);
    CurrentSettings.ThemeColors.Selected = preferences.getUShort("Selected", DefaultSettings.ThemeColors.Selected);
    CurrentSettings.ThemeColors.Text = preferences.getUShort("Text", DefaultSettings.ThemeColors.Text);
    CurrentSettings.ThemeColors.QrCode = preferences.getUShort("QrCode", DefaultSettings.ThemeColors.QrCode);
    CurrentSettings.ThemeColors.QrCodeBg = preferences.getUShort("QrCodeBg", DefaultSettings.ThemeColors.QrCodeBg);

    CurrentSettings.StatusColors.Default = preferences.getUShort("Default", DefaultSettings.StatusColors.Default);
    CurrentSettings.StatusColors.Idle = preferences.getUShort("Idle", DefaultSettings.StatusColors.Idle);
    CurrentSettings.StatusColors.Jog = preferences.getUShort("Jog", DefaultSettings.StatusColors.Jog);
    CurrentSettings.StatusColors.Run = preferences.getUShort("Run", DefaultSettings.StatusColors.Run);
    CurrentSettings.StatusColors.Hold = preferences.getUShort("Hold", DefaultSettings.StatusColors.Hold);
    CurrentSettings.StatusColors.Home = preferences.getUShort("Home", DefaultSettings.StatusColors.Home);
    CurrentSettings.StatusColors.Alarm = preferences.getUShort("Alarm", DefaultSettings.StatusColors.Alarm);
    CurrentSettings.StatusColors.Disconnect = preferences.getUShort("Disconnect", DefaultSettings.StatusColors.Disconnect);
    CurrentSettings.StatusColors.Configuration = preferences.getUShort("Configuration", DefaultSettings.StatusColors.Configuration);

    // If not in config mode, but missing required settings, force config mode
    if(!CurrentSettings.IsConfigMode) {
        CurrentSettings.IsConfigMode =
           CurrentSettings.Connection.SSID.length() == 0
        || CurrentSettings.Connection.Password.length() == 0
        || CurrentSettings.Connection.Address.length() == 0
        || CurrentSettings.Connection.Port == 0;
    }

    preferences.end();
}

FluidScreenSettings getSettingsFromJson(JsonObject doc) {
    FluidScreenSettings settings;
    settings.IsInitialized = doc["IsInitialized"];
    settings.IsConfigMode = doc["IsConfigMode"];

    JsonObject connection = doc["Connection"];
    settings.Connection.SSID = (const char*)connection["SSID"];
    settings.Connection.Password = (const char*)connection["Password"];
    settings.Connection.Address = (const char*)connection["Address"];
    settings.Connection.Port = connection["Port"];

    JsonObject jogSettings = doc["JogSettings"];
    settings.JogSettings.MaxFeedrateXY = jogSettings["MaxFeedrateXY"];
    settings.JogSettings.MaxFeedrateZ = jogSettings["MaxFeedrateZ"];

    JsonObject themeColors = doc["ThemeColors"];
    settings.ThemeColors.Primary = themeColors["Primary"];
    settings.ThemeColors.Secondary = themeColors["Secondary"];
    settings.ThemeColors.Background = themeColors["Background"];
    settings.ThemeColors.BackgroundAlt = themeColors["BackgroundAlt"];
    settings.ThemeColors.BackgroundPanel = themeColors["BackgroundPanel"];
    settings.ThemeColors.Border = themeColors["Border"];
    settings.ThemeColors.Selected = themeColors["Selected"];
    settings.ThemeColors.Text = themeColors["Text"];
    settings.ThemeColors.QrCode = themeColors["QrCode"];
    settings.ThemeColors.QrCodeBg = themeColors["QrCodeBg"];

    JsonObject statusColors = doc["StatusColors"];
    settings.StatusColors.Default = statusColors["Default"];
    settings.StatusColors.Idle = statusColors["Idle"];
    settings.StatusColors.Jog = statusColors["Jog"];
    settings.StatusColors.Run = statusColors["Run"];
    settings.StatusColors.Hold = statusColors["Hold"];
    settings.StatusColors.Home = statusColors["Home"];
    settings.StatusColors.Alarm = statusColors["Alarm"];
    settings.StatusColors.Disconnect = statusColors["Disconnect"];
    settings.StatusColors.Configuration = statusColors["Configuration"];

    return settings;
}

String getJsonFromSettings(FluidScreenSettings& settings) {
    JsonDocument doc;

    doc["IsInitialized"] = settings.IsInitialized;
    doc["IsConfigMode"] = settings.IsConfigMode;

    JsonObject connection = doc["Connection"].to<JsonObject>();
    connection["SSID"] = settings.Connection.SSID;
    connection["Password"] = settings.Connection.Password;
    connection["Address"] = settings.Connection.Address;
    connection["Port"] = settings.Connection.Port;

    JsonObject jogSettings = doc["JogSettings"].to<JsonObject>();
    jogSettings["MaxFeedrateXY"] = settings.JogSettings.MaxFeedrateXY;
    jogSettings["MaxFeedrateZ"] = settings.JogSettings.MaxFeedrateZ;

    JsonObject themeColors = doc["ThemeColors"].to<JsonObject>();
    themeColors["Primary"] = settings.ThemeColors.Primary;
    themeColors["Secondary"] = settings.ThemeColors.Secondary;
    themeColors["Background"] = settings.ThemeColors.Background;
    themeColors["BackgroundAlt"] = settings.ThemeColors.BackgroundAlt;
    themeColors["BackgroundPanel"] = settings.ThemeColors.BackgroundPanel;
    themeColors["Border"] = settings.ThemeColors.Border;
    themeColors["Selected"] = settings.ThemeColors.Selected;
    themeColors["Text"] = settings.ThemeColors.Text;
    themeColors["QrCode"] = settings.ThemeColors.QrCode;
    themeColors["QrCodeBg"] = settings.ThemeColors.QrCodeBg;

    JsonObject statusColors = doc["StatusColors"].to<JsonObject>();
    statusColors["Default"] = settings.StatusColors.Default;
    statusColors["Idle"] = settings.StatusColors.Idle;
    statusColors["Jog"] = settings.StatusColors.Jog;
    statusColors["Run"] = settings.StatusColors.Run;
    statusColors["Hold"] = settings.StatusColors.Hold;
    statusColors["Home"] = settings.StatusColors.Home;
    statusColors["Alarm"] = settings.StatusColors.Alarm;
    statusColors["Disconnect"] = settings.StatusColors.Disconnect;
    statusColors["Configuration"] = settings.StatusColors.Configuration;

    String jsonString;
    serializeJson(doc, jsonString);

    return jsonString;
}

#endif