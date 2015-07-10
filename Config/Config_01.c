// ****************************************************************************************************************************************
// RFLink List of Plugins
// ****************************************************************************************************************************************
// Here are all plugins listed that are supported and used after compilation.
// When needed additional plugins can be added or selected plugins can be enabled/disabled.
// ****************************************************************************************************************************************
#define UNIT_NODO        1 // Nodo unit number after sending a [Reset] command

// Translation Plugin for oversized packets due to their breaks/pause being too short between packets
// Used for Flamingo FA500R and various others, do NOT exclude this plugin.
#define PLUGIN_001         // DO NOT CHANGE
#define PLUGIN_001_CORE    // DO NOT CHANGE 

// ------------------------------------------------------------------------
// -- Any of the following protocols can be excluded whenever not needed --
// ------------------------------------------------------------------------
// Kaku : Klik-Aan-Klik-Uit (with code wheel) aka ARC
#define PLUGIN_002
#define PLUGIN_002_CORE

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

// RGB Switch
#define PLUGIN_010
#define PLUGIN_010_CORE

// Home Confort
#define PLUGIN_011
#define PLUGIN_011_CORE

// Flamingo FA500R
#define PLUGIN_012
#define PLUGIN_012_CORE

// Home Easy EU 
#define PLUGIN_015
#define PLUGIN_015_CORE

// -------------------
// Weather sensors
// Alecto V1 (WS3500) 434 MHz.
#define PLUGIN_030
#define PLUGIN_030_CORE

// Alecto V3 (WS1100/WS1200/WSD-19) 433.9 MHz.
#define PLUGIN_031
#define PLUGIN_031_CORE

// Alecto V4
#define PLUGIN_032
#define PLUGIN_032_CORE

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
// Motion Sensors, include when needed
// -------------------
// Ajax Chubb Varel 433 MHz. motion sensors
#define PLUGIN_060
#define PLUGIN_060_CORE

// Chinese PIR motion door and window sensors 
#define PLUGIN_061
#define PLUGIN_061_CORE

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

// Deltronic doorbell
#define PLUGIN_073
#define PLUGIN_073_CORE

// RL02
#define PLUGIN_074
#define PLUGIN_074_CORE

// -------------------
// Smoke detectors
// -------------------
// Flamingo FA20 / KD101 smoke detector
#define PLUGIN_080
#define PLUGIN_080_CORE

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
#define PLUGIN_090
#define PLUGIN_090_CORE

// Debug to show unsupported packets 
#define PLUGIN_254
#define PLUGIN_254_CORE

// ****************************************************************************************************************************************
// RFLink List of Plugins that have TRANSMIT functionality
// ****************************************************************************************************************************************
// Here are all plugins listed that are supported and used after compilation.
// When needed additional plugins can be added or selected plugins can be enabled/disabled.
// ****************************************************************************************************************************************
// ------------------------------------------------------------------------
// -- Any of the following protocols can be excluded whenever not needed --
// ------------------------------------------------------------------------
// Kaku : Klik-Aan-Klik-Uit (with code wheel) aka ARC
#define PLUGIN_TX_002
#define PLUGIN_TX_002_CORE

// NewKAKU : Klik-Aan-Klik-Uit with automatic coding aka Intertechno.
#define PLUGIN_TX_004
#define PLUGIN_TX_004_CORE

// Eurodomest 
#define PLUGIN_TX_005
#define PLUGIN_TX_005_CORE

// Blyss
#define PLUGIN_TX_006
#define PLUGIN_TX_006_CORE

// Conrad RSL2
#define PLUGIN_TX_007
#define PLUGIN_TX_007_CORE

// Kambrook
#define PLUGIN_TX_008
#define PLUGIN_TX_008_CORE

// X10 RF
#define PLUGIN_TX_009
#define PLUGIN_TX_009_CORE

// Home Confort
#define PLUGIN_TX_011
#define PLUGIN_TX_011_CORE

// Flamingo FA500R
#define PLUGIN_TX_012
#define PLUGIN_TX_012_CORE

// Home Easy EU 
#define PLUGIN_TX_015
#define PLUGIN_TX_015_CORE

// -------------------
// Doorbells
// -------------------
// Select Plus (Action - Quhwa) 
#define PLUGIN_TX_070
#define PLUGIN_TX_070_CORE

// Byron SX doorbell
#define PLUGIN_TX_072
#define PLUGIN_TX_072_CORE

// Deltronic doorbell
#define PLUGIN_TX_073
#define PLUGIN_TX_073_CORE

// RL02
#define PLUGIN_TX_074
#define PLUGIN_TX_074_CORE
// -------------------
// Smoke detectors
// -------------------
// Flamingo FA20 / KD101 smoke detector
#define PLUGIN_TX_080
#define PLUGIN_TX_080_CORE

// Mertik Maxitrol / Dru fireplace
#define PLUGIN_TX_082
#define PLUGIN_TX_082_CORE
// -------------------
//       -=#=-
// -------------------
