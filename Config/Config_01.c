// ****************************************************************************************************************************************
// Definities voor Nodo units : CONFIG.C
// ****************************************************************************************************************************************
// Here are all plugins listed that are supported and used after compilation.
// When needed additional plugins can be added or selected plugins can be disabled.
// ****************************************************************************************************************************************
#define UNIT_NODO        1 // Nodo unit number after sending a [Reset] command
#define CLOCK         true // true = include code for Real Time Clock into the compilation.
#define NODO_MEGA     true // DO NOT CHANGE

// Translation Plugin for oversized packets due to their breaks/pause being too short between packets
// Used for Flamingo FA500R and various others, do NOT exclude this plugin.
#define PLUGIN_001
#define PLUGIN_001_CORE

// ------------------------------------------------------------------------
// -- Any of the following protocols can be excluded whenever not needed --
// ------------------------------------------------------------------------
// Kaku : Klik-Aan-Klik-Uit (with code wheel) aka ARC
#define PLUGIN_002
#define PLUGIN_002_CORE

// Flamingo FA500R
#define PLUGIN_003
#define PLUGIN_003_CORE

// NewKAKU : Klik-Aan-Klik-Uit with automatic coding aka Intertechno.
#define PLUGIN_004
#define PLUGIN_004_CORE

// Eurodomest 
#define PLUGIN_005
#define PLUGIN_005_CORE

// Blyss
#define PLUGIN_006
#define PLUGIN_006_CORE

// Conrad RSL2
#define PLUGIN_007
#define PLUGIN_007_CORE

// Kambrook
#define PLUGIN_008
#define PLUGIN_008_CORE

// X10 RF
#define PLUGIN_009
#define PLUGIN_009_CORE

// Home Easy EU receive
#define PLUGIN_015
#define PLUGIN_015_CORE

// Home Easy EU transmit
#define PLUGIN_016
#define PLUGIN_016_CORE

// -------------------
// Weather sensors
// Alecto V1 (WS3500) 434 MHz.
#define PLUGIN_030
#define PLUGIN_030_CORE

// Alecto V3 (WS1100/WS1200/WSD-19) 433.9 MHz.
#define PLUGIN_031
#define PLUGIN_031_CORE

// Cresta 
#define PLUGIN_034
#define PLUGIN_034_CORE

// Mebus
#define PLUGIN_040
#define PLUGIN_040_CORE

// UPM/Esic
#define PLUGIN_042
#define PLUGIN_042_CORE

// LaCrosse
#define PLUGIN_043
#define PLUGIN_043_CORE

// Auriol v3
#define PLUGIN_044
#define PLUGIN_044_CORE

// Auriol
#define PLUGIN_045
#define PLUGIN_045_CORE

// Auriol v2 / Xiron
#define PLUGIN_046
#define PLUGIN_046_CORE

// Oregon V1/2/3
#define PLUGIN_048
#define PLUGIN_048_CORE

// -------------------
// PIR's
// -------------------
// Ajax Chubb Varel 433 MHz. motion sensors
//#define PLUGIN_060
//#define PLUGIN_060_CORE

// Chinese PIR motion sensors 
//#define PLUGIN_061
//#define PLUGIN_061_CORE

// -------------------
// Doorbells
// -------------------
// Select Plus (Action - Quhwa) 
#define PLUGIN_070
#define PLUGIN_070_CORE

// Plieger York
#define PLUGIN_071
#define PLUGIN_071_CORE

// Byron SX doorbell
#define PLUGIN_072
#define PLUGIN_072_CORE

// Lidl doorbell
//#define PLUGIN_073
//#define PLUGIN_073_CORE

// Byron MP001 doorbell
//#define PLUGIN_074
//#define PLUGIN_074_CORE

// -------------------
// Smoke detectors
// -------------------
// Flamingo FA20 / KD101 smoke detector receive
#define PLUGIN_080
#define PLUGIN_080_CORE

// Flamingo FA20 / KD101 smoke detector transmit
#define PLUGIN_081
#define PLUGIN_081_CORE

// Mertik Maxitrol / Dru fireplace
#define PLUGIN_082
#define PLUGIN_082_CORE

// -------------------
// 868 MHZ
// -------------------
// Alecto V2 (DKW2012/ACH2010) 868 MHz.  => PLANNED
#define PLUGIN_100
#define PLUGIN_100_CORE

// -------------------
// Housekeeping
// -------------------
// Nodo Slave conversion plugin
#define PLUGIN_250
#define PLUGIN_250_CORE

// Userevent conversion
//#define PLUGIN_251
//#define PLUGIN_251_CORE

// Debug to show unsupported packets 
#define PLUGIN_254
#define PLUGIN_254_CORE

//Sample plugin, do not compile => use it as template to make your own plugin
//#define PLUGIN_255
//#define PLUGIN_255_CORE
// -------------------
