/*
* Linear Technology / Analog Devices, Ismaning Design Center
*
* DC2732A_681X.ino
*  Linduino Sketch for LTC2949 Demo Board (DC2732A) - Isolated Battery Meter for EV / HEV
*  and LTC681x-1 Demo Board (DC2259A, DC2350A...) - Multicell Battery Monitor
*
* Hardware required:
*  LTC6820 demoboard (DC1941D or DC2617A or DC2792)
*  Optional: Two LTC6820 boards or one DC2792 (dual LTC6820 demoboard)
*              The two LTC6820 are selected via individual chip selects via Arduino pins 9 and 10. See notes below
*              The software will also work with only one LTC6820
*              Two LTC6820 are necessary to showcase reversible isoSPI fault recovery functionality
*  One or more LTC681x Demo Board (DC2259A, DC2350A), default (see define LTCDEF_CELL_MONITOR_COUNT in code below) is 2 boards in the daisychain
*  the number of cells to be measured can be set via LTCDEF_CELLS_PER_CELL_MONITOR_COUNT, default is 12 (LTC6811)
*  Linduino (DC2026)
*
* Hardware setup (e.g. PARALLEL TO DAISYCHAIN):
*  DC2732A: Enable isoSPI mode (JP3-JP6), disable isoSPI termination (JP1)
*  DC1941D: Set IBIAS, VCMP jumpers to VTH2
*  DC2259A: Set ISOMD jumpers to 1 to enable isoSPI
*  :
*  - Connect Linduino to LTC6820 demoboard
*  - Connect LTC6820 demoboard via RJ45 cable to LTC2949 demoboard (J1)
*  - Connect LTC2949 demoboard (J2) via RJ45 cable to first LTC6811-1 demoboard (J3 / port A)
*  - Connect LTC6811-1 demoboard (J4 / port B) to 2nd LTC6811-1 demoboard (J3 / port A)
*  - Connect as many LTC6811-1 demoboards in the daisychain as required
*  - Connect power supply to LTC2949 demoboard (turrets VCC, LGND)
*  - Connect power supply / batteries to LTC6811-1 demoboard(s) (+ CELL12, - CELL0)
*
* equivalent setups are possible e.g. with LTC6813 (DC2350A)
*
* Software setup:
*  - Set LTCDEF_CELL_MONITOR_COUNT to the number of LTC681x connected in the daisychain (default 2)
*  - Set LTCDEF_CELLS_PER_CELL_MONITOR_COUNT to the number of cells per cell monitor
*  -
* Hardware setup (ON TOP OF DAISYCHAIN):
*  - same as above, but:
*    - DC2732A: Enable isoSPI termination (JP1)
*    - Connect LTC2949 demoboard (J2) via RJ45 cable to last LTC6811-1 demoboard (J4 / port B)
*    - Connect LTC6820 demoboard via RJ45 cable to first LTC6811-1 demoboard (J3 / port A)
*
* See demo manuals of the related demoboards for more details on hardware setup.
*
* Description:
*  LTC2949 will do fast single shot measurements of I2 and BAT (CH2 + P2ASV) synchronous to cell
*  voltage measurements done by the cell monitor(s) LTC6811-1 (and compatible cell monitors of the LTC68xx family)
*  LTC2949 is connected in parallel to the daisychain of cell monitors (meaning at the bottom of the daisychain
*  parallel to the LTC6820) as default.
*  The DC2732A_681X sketch also allows to connect LTC2949 on top of the daisychain (after the last cell monitor). It
*  will automatically detect which configuration is setup and switch between "PARALLEL TO DAISYCHAIN" and
*  "ON TOP OF DAISYCHAIN". This also demonstrates how LTC2949 can benefit from a redundant communication path when used
*  with reversible isoSPI ready cell monitors like the LTC6813.
*    The reversible isoSPI fault recovery feature can be demonstrated when having two LTC6820 connected to the Linduino
*    Any of the two LTC6820 can be disconnected and the software will automatically find a new communication path.

*  Additionally LTC2949 does following measurements with its slow channel (100ms update time)
*   NTC1: SLOT1 measures temp via NTC (NTC connected between V1 and GND)
*   continuous reading and reporting of delta-TB1, I1, P1, BAT, NTC1 / IC temp. from slow channel.
*  Reporting of fast I2, BAT together with cell voltage conversion results

*  Example output: (see the INIT string, that always shows the configuration used, CS tells the used chip select = one for up to two LTC6820)
INIT,PARALLEL TO DAISYCHAIN,CS:10,STAT:0x10000000000000000000,OK
0.3,0.1,0.2,0.3,0.1,0.2,0x000000000000000000000000,0x040000000000040000000000,6.6,6.6,6.6,6.6,6.6,6.6,0.3,0.1,0.2,0.3,0.1,0.2,OK
tDut,I1,P1,BAT,Tntc,TIC,C1V,C2V,C3V,C7V,C8V,C9V,C4V,C5V,C6V,C10V,C11V,C12V,fI2,fBAT,fT,OK/ERR
0,-69785,-13063,0.19,-37.6,25.0,0.3,0.1,0.2,0.3,0.1,0.2,0.4,0.1,1.3,0.3,0.5,1.4,-69817,0.19,1064,OK
15,,,,,,0.3,0.1,0.1,0.3,0.1,0.2,0.0,0.1,1.3,0.3,0.5,1.4,-69817,0.19,1072,OK
10,,,,,,0.3,0.1,0.1,0.3,0.1,0.2,0.4,0.1,1.3,0.3,0.5,1.4,-69817,0.19,1068,OK
...
9,,,,,,6.6,6.6,6.6,6.6,6.6,6.6,6.6,6.6,6.6,6.6,6.6,6.6,-8,-0.00,832,ERR:0x83
<<<<<<=====================================================================>>>>>>>
<<<<<<========= error injected by unplugging one isoSPI cable =============>>>>>>>
<<<<<<=====================================================================>>>>>>>
ERR:0x1
INIT,PARALLEL TO DAISYCHAIN,CS:10,STAT:0xFFFFFFFFFFFFFFFFFFFF,ERR:0x9
6.6,6.6,6.6,6.6,6.6,6.6,0xFFFFFFFFFFFFFFFFFFFFFFFF,0xFFFFFFFFFFFFFFFFFFFFFFFF,6.6,6.6,6.6,6.6,6.6,6.6,6.6,6.6,6.6,6.6,6.6,6.6,ERR:0xB
tDut,I1,P1,BAT,Tntc,TIC,C1V,C2V,C3V,C7V,C8V,C9V,C4V,C5V,C6V,C10V,C11V,C12V,fI2,fBAT,fT,OK/ERR
ERR:0x1
INIT,PARALLEL TO DAISYCHAIN,CS:9,STAT:0xFFFFFFFFFFFFFFFFFFFF,ERR:0x9
0.3,0.1,0.2,0.3,0.1,0.2,0x040000000000040000000000,0x040000000000040000000000,6.6,6.6,6.6,6.6,6.6,6.6,0.3,0.1,0.2,0.3,0.1,0.2,ERR:0x9
tDut,I1,P1,BAT,Tntc,TIC,C1V,C2V,C3V,C7V,C8V,C9V,C4V,C5V,C6V,C10V,C11V,C12V,fI2,fBAT,fT,OK/ERR
ERR:0x1
INIT,PARALLEL TO DAISYCHAIN,CS:9,STAT:0xFFFFFFFFFFFFFFFFFFFF,ERR:0x9
0.3,0.1,0.2,0.3,0.1,0.2,0x040000000000040000000000,0x040000000000040000000000,6.6,6.6,6.6,6.6,6.6,6.6,0.3,0.1,0.2,0.3,0.1,0.2,ERR:0x9
tDut,I1,P1,BAT,Tntc,TIC,C1V,C2V,C3V,C7V,C8V,C9V,C4V,C5V,C6V,C10V,C11V,C12V,fI2,fBAT,fT,OK/ERR
ERR:0x1
INIT,ON TOP OF DAISYCHAIN,CS:10,STAT:0xFFFFFFFFFFFFFFFFFFFF,ERR:0xB
6.6,6.6,6.6,6.6,6.6,6.6,0xFFFFFFFFFFFFFFFFFFFFFFFF,0xFFFFFFFFFFFFFFFFFFFFFFFF,6.6,6.6,6.6,6.6,6.6,6.6,6.6,6.6,6.6,6.6,6.6,6.6,ERR:0xB
tDut,I1,P1,BAT,Tntc,TIC,C1V,C2V,C3V,C7V,C8V,C9V,C4V,C5V,C6V,C10V,C11V,C12V,fI2,fBAT,fT,OK/ERR
ERR:0x3
INIT,ON TOP OF DAISYCHAIN,CS:10,STAT:0xFFFFFFFFFFFFFFFFFFFF,ERR:0xB
6.6,6.6,6.6,6.6,6.6,6.6,0xFFFFFFFFFFFFFFFFFFFFFFFF,0xFFFFFFFFFFFFFFFFFFFFFFFF,6.6,6.6,6.6,6.6,6.6,6.6,6.6,6.6,6.6,6.6,6.6,6.6,ERR:0xB
tDut,I1,P1,BAT,Tntc,TIC,C1V,C2V,C3V,C7V,C8V,C9V,C4V,C5V,C6V,C10V,C11V,C12V,fI2,fBAT,fT,OK/ERR
ERR:0x3
<<<<<<=====================================================================>>>>>>>
<<<<<<========= new functional communication path found ===================>>>>>>>
<<<<<<=====================================================================>>>>>>>
INIT,ON TOP OF DAISYCHAIN,CS:9,STAT:0x10000000000000000000,OK
0.3,0.1,0.2,0.3,0.1,0.2,0x040000000000040000000000,0x040000000000040000000000,6.6,6.6,6.6,6.6,6.6,6.6,0.3,0.1,0.2,0.3,0.1,0.1,OK
tDut,I1,P1,BAT,Tntc,TIC,C1V,C2V,C3V,C7V,C8V,C9V,C4V,C5V,C6V,C10V,C11V,C12V,fI2,fBAT,fT,OK/ERR
0,-69786,-13045,0.19,-37.6,25.0,0.3,0.1,0.2,0.3,0.1,0.1,0.3,0.5,1.4,0.1,0.1,1.3,-69825,0.19,1460,OK
27,,,,,,0.3,0.1,0.2,0.3,0.1,0.1,0.3,0.5,1.4,0.4,0.1,1.3,-69817,0.19,1464,OK
...
*
*  Notes:
*   tDut: delta TB1 in ms (typically 100, as values are reported for every slow conversion cycle which lasts 100 ms)
*   I1: slow / high precision I1 current measurement in V (x 1/Rsns for current in A)
*   P1: slow / high precision P1 power measurement in V*V (x 1/Rsns for power in W)
*   BAT: slow AUX channel BAT measurement in V
*   Tntc: slow AUX channel SLOT1 measurement. SLOT1 is configured for temperature measurement via NTC in degree Celsius
*   TIC:  slow AUX channel IC temperature measurement in degree Celsius
*   CxV: Voltage of cell x
*   fI2: latest CH2 current fast measurement in V (x 1/Rsns for current in A)
*   fBAT: latest CH2 voltage (P2 as voltage) fast BAT measurement in V
*   fT: cell monitor fast measurement cycle time in milliseconds
*   OK/ERR: Tag indicating all measurements / communication was successful (OK) or some failure occurred (ERR)
*
****************************************************************************************************************
****************************************************************************************************************
****************************************************************************************************************
****************************************************************************************************************
* NEWLY ADDED:
* - supports two LTC6820 isoSPI masters, selection is done with chip select on Linduino pin 10 (SS) and 9
* - The DC2792 dual LTC6820 can be used for that purpose without any s/w changes. It has option for ribbon or
*  ‘shield’ connection and an LED for each CS line so that traffic & direction can be visualized.
* - Alternatively two DC1641 can be used:
*  - the connection of two DC1641 demoboards to one Linduino can be done with a custom flat ribbon
*      Linduino’s QuikEval connector (14pin connector J1) has a GPIO on pin 14 that can be used as a 2nd
*      chip select to the second DC1941. A simple way of making the connection is to use a 14pin flat
*      ribbon cable, crimp a third connector to it and swap wires 6 and 14 between the two connectors
*      going to the two DC1941 boards. The Linduino sketch switches then between the two possible chip
*      selects (Arduino pin 10 and 9) to switch between the two communication path.
*
* created: Patrick Wilhelm (PW)
*/

#include <ltcmuc_tools.h>
#include <SPI.h>
#include <LTC2949.h>
#include <SD.h>
#include <EEPROM.h>
#include <FlexCAN_T4.h>
FlexCAN_T4<CAN3 , RX_SIZE_256, TX_SIZE_16> can3;//can
FlexCAN_T4<CAN2, RX_SIZE_256, TX_SIZE_16> Can2; // Initialize CAN2
CAN_message_t send_msg,receive_msg;
CAN_message_t msg;//CAN message strut
#define send_id 0x1806E5F4 
#define receive_id 0x18FF50E5 

//  int max_voltage = 3950;                  //1900 coresponds to 190.0V
// int max_current_without_decimal = 100  ; // 100 corresponds to 10.0 Amp
//  float output_voltage=0, output_current=0;
//  bool hw_failure=0, temp_charger=0, input_voltage=0, starting_state=0, comm=0;
 

// IMPORTANT MODES OF CODE

// number of cell monitors
#define LTCDEF_CELL_MONITOR_COUNT 6

#define seventhSlave
#undef seventhSlave// undef only if u dont need seventh slave

#define dynamicCooling
//#undef dynamicCooling //undef only if u dont need to turn fans on 

// CHARGER SECTION

#define charger_active
#undef charger_active //undef in case u are not charging

#define charger_softEnabled
#undef charger_softEnabled



//CHARGER SECTION ENDS

#define Interrupt_Debug
#undef Interrupt_Debug //undef if u want the code to run 

//GUI SECTION
#define GUI_Enabled
#undef GUI_Enabled

//SD CARD SECTION

#define startLogging
//#undef startLogging

#define initialiseEEPROM
#undef initialiseEEPROM

//SOC section
#define startSOC
//#undef startSOC

// circular daisy chain 
#define circular
//#undef circular 

// number of cells per cell monitor (e.g. LTC6811: 12, LTC6813: 18)
// !!!must be multiple of 3!!!
// Its always possible to set less, if not all voltages are of interest 
// for debugging/evaluation. So here we set 6 only.
#define LTCDEF_CELLS_PER_CELL_MONITOR_COUNT 18

// define this for usage of LTC681X only, without LTC2949
#define LTCDEF_LTC681X_ONLY
#undef  LTCDEF_LTC681X_ONLY

//IMORTANT MODES OF CODE ENDS


#define LTCDEF_TIMING_DEBUG
#undef LTCDEF_TIMING_DEBUG

// serial baudrate
//Note: on Arduino Due, 250000 baud seems to be the maximum
#define LTCDEF_BAUDRATE 250000

// default chip select of the LTC6820
// Note: SS is mapped differently on Arduino Zero / DUE
//       thus direct pin number (10) should be used instead
#define LTCDEF__CS 10
#define LTCDEF__CS2 37
// chip select of the optional 2nd LTC6820
#define LTCDEF_GPO 37

// set sense resistor here
#define LTCDEF_SENSE_RESISTOR 1000e-6
// significant digits
// voltages
#define LTCDEF_DIGITS_BATSLOW     3
#define LTCDEF_DIGITS_BATFAST     2
#define LTCDEF_DIGITS_BATFIFO_AVG 2
#define LTCDEF_DIGITS_CELL        4
// currents
#define LTCDEF_DIGITS_I1SLOW      3
#define LTCDEF_DIGITS_I2FAST      0
#define LTCDEF_DIGITS_I2FIFO_AVG  0
// power
#define LTCDEF_DIGITS_P1SLOW      3
// temperature
#define LTCDEF_DIGITS_TEMPSLOW    1

#ifdef LTCDEF_LTC681X_ONLY
// polling not supported in case of LTCDEF_LTC681X_ONLY!!!
#define LTCDEF_POLL_EOC false
#else
// true for making ADCV command and poll EOC
// false for making ADCV command and wait until EOC
#define LTCDEF_POLL_EOC true
#endif

// define LTCDEF_IGNORE_PEC_ERRORS 
// quick work around for new 6815 style devices with PEC+CMDcnt 
// that is currently not supported by this source
#define LTCDEF_IGNORE_PEC_ERRORS
// #undef LTCDEF_IGNORE_PEC_ERRORS
//
#ifdef LTCDEF_IGNORE_PEC_ERRORS
// ignore all PEC errors
inline bool err_detected(byte error) { return error > 0x3; }
// or just ignore single PEC errors
//inline bool err_detected(byte error) { return (ALLOW_ONE_PEC_ERROR ? ((error) > 1) : ((error) != 0)); }
#else
inline bool err_detected(byte error) { return error != 0; }
#endif

// NTC: NTCALUG01A104F/TDK (SMD)
#define NTC_STH_A  9.85013754e-4
#define NTC_STH_B  1.95569870e-4
#define NTC_STH_C  7.69918797e-8
#define NTC_RREF   100e3

// fast channel configuration: fast single shot, channel 2: I1, BAT (via P2 as voltage see below)
#define LTCDEF_FACTRL_CONFIG LTC2949_BM_FACTRL_FACH2
// ADC configuration (SLOT1 measures temperature via NTC, P2 measures voltage)
#define LTCDEF_ADCCFG_CONFIG (LTC2949_BM_ADCCONF_NTC1 | LTC2949_BM_ADCCONF_P2ASV)

#define LTCDEF_ERR_RETRIES 1


// defined by me 
#define BMS_FLT_3V3 18              // PIN 18 ( A4 ) selected as BMS ERROR pin on teensy can check on master
#define FAN_MBED 19                 // FAN mbed signal to ACU
#define CH_EN_MBED 22               // charger enable signal to ACU
#define PEC_ERR_PIN 9               // to switch error state led on master
#define VOLT_ERR_PIN 7              // to switch error state led on master
#define TEMP_ERR_PIN 8              // to switch error state led on master

#define voltTimer 400               // Time for voltage loop to go on in its error checking mode
#define tempTimer 900                // Time for temperature loop to go on in its error checking mode
#define chargeTimer 5000

#define voltage 0
#define temp 1

#define UI_BUFFER_SIZE 64

#define RES 30000                   // Resistor divider for BATP and BATM
#define RES_SUM 6520000
#define POT_DIV_BPM 76.5   // Voltage Multiplier for BPM

//CELL COUNT OF STACK
int cellsPerStack[7]={15,15,15,15,15,15,6};


//NORMALIZATION SECTION

const uint8_t slavesToNormalize = 0x33; // S1 , S3, S6, S7 ->The least significant bit(first from right) represents the first slave in daisy chain and so on...
//                                     // In binary system if it is 1 it means that slave needs normalize convert the binary to hex and put the value  here
//                                     // For ex. if I have slave 1 and 3 to normalize the binary equivalent will be 00000101 convert binary to hex and proceed

// const uint16_t voltChannelToNormalize[LTCDEF_CELL_MONITOR_COUNT] = { 0x00,0x00,0x00,0x00,0x00,0x00,0x00};//
const uint16_t voltChannelToNormalize[LTCDEF_CELL_MONITOR_COUNT] = { 0x00,0x00,0x00,0x00,0x00,0x00};
// //const uint16_t voltChannelToNormalize[LTCDEF_CELL_MONITOR_COUNT] = { 0x00,0x00,0x00,0x00,0x00};
// // const uint16_t voltChannelToNormalize[LTCDEF_CELL_MONITOR_COUNT] = { 0x00,0x00,0x00,0x00};
// // const uint16_t voltChannelToNormalize[LTCDEF_CELL_MONITOR_COUNT] = { 0x00,0x00,0x00};
// // const uint16_t voltChannelToNormalize[LTCDEF_CELL_MONITOR_COUNT] = { 0x00,0x00};
//const uint16_t voltChannelToNormalize[LTCDEF_CELL_MONITOR_COUNT] = { 0x00};

// const uint16_t tempChannelToNormalize[LTCDEF_CELL_MONITOR_COUNT] = { 0x40,0x00,0x00,0x00,0x00,0x05,0x00};
const uint16_t tempChannelToNormalize[LTCDEF_CELL_MONITOR_COUNT] = { 0x201,0x00,0x00,0x00,0x20,0x00};
// //const uint16_t tempChannelToNormalize[LTCDEF_CELL_MONITOR_COUNT] = { 0x00,0x00,0x00,0x00,0x00};
// // const uint16_t tempChannelToNormalize[LTCDEF_CELL_MONITOR_COUNT] = { 0x00,0x00,0x00,0x00};
// // const uint16_t tempChannelToNormalize[LTCDEF_CELL_MONITOR_COUNT] = { 0x00,0x00,0x00};
// // const uint16_t tempChannelToNormalize[LTCDEF_CELL_MONITOR_COUNT] = { 0x00,0x00};
//const uint16_t tempChannelToNormalize[LTCDEF_CELL_MONITOR_COUNT] = { 0x00};

// //NORMALIZATION SECTION ENDS

//SPLIT SECTION

const uint8_t slavesOnSplit = 0x01;// Least significant bit is first slave 

//const uint16_t voltChannelOnSplit[LTCDEF_CELL_MONITOR_COUNT] = { 0x00}; // if SPlit between ch-1 and ch-2 pass "ch-2" here
const uint16_t voltChannelOnSplit[LTCDEF_CELL_MONITOR_COUNT] = { 0x80,0x00,0x00,0x00,0x00,0x00 };
//const uint16_t voltChannelOnSplit[LTCDEF_CELL_MONITOR_COUNT] = { 0x02,0x00,0x00,0x00,0x00 };
//const uint16_t voltChannelOnSplit[LTCDEF_CELL_MONITOR_COUNT] = { 0x02,0x00,0x00,0x00 };
//const uint16_t voltChannelOnSplit[LTCDEF_CELL_MONITOR_COUNT] = { 0x02,0x00,0x00};
//const uint16_t voltChannelOnSplit[LTCDEF_CELL_MONITOR_COUNT] = { 0x02,0x00 };
//const uint16_t voltChannelOnSplit[LTCDEF_CELL_MONITOR_COUNT] = { 0x0 };

// Normalize all channels
// const uint8_t slavesToNormalize = 0xff; // S3 and S6
// const uint16_t voltChannelToNormalize[LTCDEF_CELL_MONITOR_COUNT] = { 0xffff, 0xffff,0xffff,0xffff,0xffff,0xffff,0xffff };
// const uint16_t tempChannelToNormalize[LTCDEF_CELL_MONITOR_COUNT] = { 0xffff, 0xffff,0xffff,0xffff,0xffff,0xffff,0xffff };

//const uint8_t slavesToNormalize = 0x00;
//const uint16_t voltChannelToNormalize[LTCDEF_CELL_MONITOR_COUNT] = { 0x00};
//const uint16_t tempChannelToNormalize[LTCDEF_CELL_MONITOR_COUNT] = { 0x00};

//SPLIT SECTION ENDS

// THRESHOLD SECTION

const double overVoltageThreshold = 4.200;
const double underVoltageThreshold = 2.8000;
const double overTempThreshold = 45.000;
const double dynamicCoolingThreshold = 35.000;

//CHARGER THRESHOLD
int max_voltage = 3600;                  //1900 coresponds to 190.0V
int max_current_without_decimal = 100  ; // 100 corresponds to 10.0 Amp
float output_voltage=0, output_current=0;
bool hw_failure=0, temp_charger=0, input_voltage=0, starting_state=0, comm=0;






//THRESHOLD SECTION ENDS

const bool fanMbed = true;
const bool chargerEnMbed = false;
const bool amsMasterToACUsignals[2] = { fanMbed , chargerEnMbed };
int flag_fan =0;


// SD Card logging 
int chgrDelay = 1000;
const int chipSelect = BUILTIN_SDCARD;
File dataFile;
bool BPM_ready;
bool enteredChecking=false;
bool chargingStarted_=true;
bool chargingFlag=false;

//SOC estimation
bool SOC_init_flag=false;

float EnergyAvailable = 0;

float tf=0;

double cellVoltages[LTCDEF_CELL_MONITOR_COUNT][15];     // cell voltages data
int8_t voltageErrorLoc[LTCDEF_CELL_MONITOR_COUNT][15];  // gives location of voltage error (-1 in case of error / 0 in case of no error)
int8_t voltageNormalize[LTCDEF_CELL_MONITOR_COUNT][15];
int8_t voltageSplit[LTCDEF_CELL_MONITOR_COUNT][15];
double cellTemperatures[LTCDEF_CELL_MONITOR_COUNT][15]; // cell temp data
int8_t tempErrorLoc[LTCDEF_CELL_MONITOR_COUNT][15];  // gives location of voltage error (-1 in case of error / 0 in case of no error)
int8_t tempNormalize[LTCDEF_CELL_MONITOR_COUNT][15];
float batVoltage_num;
float batCurrPower_num[1];
String batVoltage;                                  // TS voltage
String batCurrPower[2];                             // 0:TS_curr ; 1:TS_power

String CellData = "";
String CellData_GUI="";

// circular daisy chain 
bool loopcount = true;
float tolerance = 1;
double cells[LTCDEF_CELL_MONITOR_COUNT][18];
double cells1[LTCDEF_CELL_MONITOR_COUNT][18];
double cells2[LTCDEF_CELL_MONITOR_COUNT][18];

double auxHigh1[LTCDEF_CELL_MONITOR_COUNT][12];
double auxLow1[LTCDEF_CELL_MONITOR_COUNT][12];
double auxHigh2[LTCDEF_CELL_MONITOR_COUNT][12];
double auxLow2[LTCDEF_CELL_MONITOR_COUNT][12];
double auxHigh[LTCDEF_CELL_MONITOR_COUNT][12];
double auxLow[LTCDEF_CELL_MONITOR_COUNT][12];

struct maxMinParameters
{
  double val;
  uint8_t slaveLoc;
  uint8_t cellLoc;
};

struct maxMinParameters maxVoltage;
struct maxMinParameters minVoltage;
struct maxMinParameters maxTemp;

int8_t errorFlag[9] = {0};
bool bmsFlag;
bool voltFlag;
bool tempFlag;
bool chargerFlag;
char ui_buffer[UI_BUFFER_SIZE];

void sendAllDataToECU(void);
void sendDataToECU(float voltage, float temperature);
void cellVoltageLoop(unsigned long timeBuffer);    
void cellsVoltSort(void);  
void printCells(void);
void voltagePlausibilityCheck(void);
void cellTempLoop(unsigned long timeBuffer);  
void printAux(void);
void tempPlausibilityCheck(void);
void tempConvertSort(bool muxSelect);
void batLoop(bool slowChannelReady);
void checkError(void); 
void pull_3V3_high(void);             
void clearFlags(void);   
void clearFlagsV(void);
void clearFlagsT(void);
void transferT(uint16_t * data, uint8_t nic, bool muxSelect, uint8_t auxReg);            
void transferV(uint16_t * data, uint8_t nic, uint8_t cellReg);                       
void circularVoltdef(void);
void circularTempdef(bool muxSelect);
void chechVoltageFlag(void);
void checkTempFlag(void);
void errorCheckingMode(void);
void triggerError(void);
void triggerInterrupt(void);
void printErrLocV(void);
void printErrLocT(void);
void Initialisation(void);
void switchErrorLed(void);
void initialiseNormalizeChannel(void);
void setNormalizeChannels(uint8_t ic, uint16_t * vChannels, uint16_t * tChannels );
void setNormalizeFlag(uint8_t nic, uint16_t voltChannels, uint16_t tempChannels);
void checkVoltNormalize(void);
void checkTempNormalize(void);
void initialiseSplitChannel(void);
void setVoltSplitChannels(uint8_t ic, uint16_t * channels );
void setVoltSplitFlag(uint8_t nic, uint16_t channels );
void checkVoltSplit(void);
void checkACUsignalStatus(void);
void findMax(double array[LTCDEF_CELL_MONITOR_COUNT][15], uint8_t type );
void findMin(double array[LTCDEF_CELL_MONITOR_COUNT][15]);
void printMaxMinParameters(void);
void performDynamicCooling(void);
void chargerLoop(void);
void triggerchargerError(void);
void printchargerError(void);
void cellsLogging(void);
void auxLogging(void);
void maxMinLogging(void);
void SDcardLogging(void);
void initialiseSDcard(void);
float InitialiseEnergy(float min_voltage, float thr_voltage);
float CalculateEnergy(float TS_voltage, float TS_current, float &Energy_available, float time_previous);
float linear_interpolate(float x0, float y0, float x1, float y1, float x);

// define to do a reset every xxx milliseconds
//#define LTCDEF_DO_RESET_TEST 1000

uint16_t cellMonDat[LTCDEF_CELL_MONITOR_COUNT * 3]; // = LTCDEF_CELL_MONITOR_COUNT * 6 bytes
byte * buffer = (byte *)cellMonDat; // buffer and cellMonDat can share the same memory
byte  error = 0;  

#ifndef LTCDEF_LTC681X_ONLY
	int16_t fastData2949[LTC2949_RDFASTDATA_LENGTH];
#endif

#ifndef LTCDEF_LTC681X_ONLY
	// store the last TBx value that was read (to be able to calc time difference later...)
	uint32_t deltaT = LTC2949_GetLastTBxInt();
#endif

unsigned long mcuTime;
uint8_t retries = LTCDEF_ERR_RETRIES;

/*!*********************************************************************
\brief prints the CSV header of the measurement output
***********************************************************************/
void PrintCSVHeader()
{
	// the order the cell voltages are reported is little bit awkward,
	// its not just 1,2,3,4,5,6.... so we can't print the header
	// like this:
	//Serial.print(F("tDut,I1,P1,BAT,Tntc,TIC,C1V..,C"));
	//Serial.print(LTCDEF_CELL_MONITOR_COUNT * 3 * 4);
	//Serial.println(F("V,fI2,fBAT,OK/ERR"));
#ifndef LTCDEF_LTC681X_ONLY
if(loopcount){
	Serial.print(F("tDut,I1,P1,BAT,Tntc,TIC,"));
} 
#else

	Serial.print(F("tDut,"));
#endif
	for (uint8_t j = 0; j < LTCDEF_CELL_MONITOR_COUNT; j++) // number of cell monitors
	{
		for (uint8_t n = 0; n < LTCDEF_CELLS_PER_CELL_MONITOR_COUNT; n++)// += 3) // 3 per RDCV
		{
			Serial.print((char)('a' + j));
			Serial.print('C');
			Serial.print(n);
			PrintComma();
		}
	}
#ifdef LTCDEF_LTC681X_ONLY
	Serial.println(F("eErr,OK/ERR"));
#else
if(loopcount){
	Serial.println(F("eErr,fI2,fBAT,fT,OK/ERR"));
}
#endif

	/*
	!!!!
	!!!! Initially we assume to be parallel to the daisychain, but here we started on top of daisychain
	!!!!
	tDut,I1,P1,BAT,Tntc,TIC,C1V,C2V,C3V,C13V,C14V,C15V,C4V,C5V,C6V,C16V,C17V,C18V,C7V,C8V,C9V,C19V,C20V,C21V,C10V,C11V,C12V,C22V,C23V,C24V,fI2,fBAT,OK/ERR
	STAT:0xFFFFFFFFFFFFFFFFFFFF,ERR:0x9
	0.0,0.9,0.9,0.9,0.9,0.9,0xDA00000000009A0000000000,0x1E0000000000960000000000,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,0.0,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,ERR:0x9
	INIT,PARALLEL TO DAISYCHAIN,0xFFFFFFFFFFFFFFFFFFFF,STAT:0xFFFFFFFFFFFFFFFFFFFF,ERR:0x9
	ERR:0x9
	tDut,I1,P1,BAT,Tntc,TIC,C1V,C2V,C3V,C13V,C14V,C15V,C4V,C5V,C6V,C16V,C17V,C18V,C7V,C8V,C9V,C19V,C20V,C21V,C10V,C11V,C12V,C22V,C23V,C24V,fI2,fBAT,OK/ERR
	STAT:0x10000000000000000000,OK
	0.0,0.9,0.9,0.9,0.9,0.9,0x9E0000000000DE0000000000,0x9E00000000009E0000000000,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,0.0,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,OK
	INIT,ON TOP OF DAISYCHAIN,0x10000000000000000000,STAT:0x10000000000000000000,OK
	100,0,0,-0.00,25.8,28.0,0.0,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.0,0,-0.00,1220,OK
	100,-1,6,-0.00,25.8,28.0,0.0,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.0,0,-0.00,1216,OK
	100,0,0,-0.00,25.8,28.0,0.0,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.0,0,-0.00,1220,OK
	100,0,6,-0.00,25.8,28.0,0.0,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.0,0,-0.00,1216,OK
	...
	...
	!!!! normal operation
	...
	...
	100,0,0,-0.00,25.8,28.2,0.0,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.0,0,-0.00,1216,OK
	100,0,0,-0.00,25.8,28.2,0.0,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.0,0,-0.00,1228,OK
	100,-1,0,-0.00,25.8,28.2,0.0,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.0,0,-0.00,1224,OK
	!!!!
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	!!!! We interrupt the isoSPI interface here!!!!!
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	!!!!
	!!!! First we try again with the current configuration (ON TOP OF DAISYCHAIN)
	!!!!
	INIT,ON TOP OF DAISYCHAIN,0x10000000000000000000,STAT:0xFFFFFFFFFFFFFFFFFFFF,ERR:0xB
	ERR:0xB
	tDut,I1,P1,BAT,Tntc,TIC,C1V,C2V,C3V,C13V,C14V,C15V,C4V,C5V,C6V,C16V,C17V,C18V,C7V,C8V,C9V,C19V,C20V,C21V,C10V,C11V,C12V,C22V,C23V,C24V,fI2,fBAT,OK/ERR
	STAT:0xFFFFFFFFFFFFFFFFFFFF,ERR:0x9
	-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,0xFFFFFFFFFFFFFFFFFFFFFFFF,0xFFFFFFFFFFFFFFFFFFFFFFFF,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,ERR:0xB
	!!!!
	!!!! Didn't work so we try with the opposite configuration (PARALLEL TO DAISYCHAIN)
	!!!!
	INIT,PARALLEL TO DAISYCHAIN,0xFFFFFFFFFFFFFFFFFFFF,STAT:0xFFFFFFFFFFFFFFFFFFFF,ERR:0x9
	ERR:0x9
	tDut,I1,P1,BAT,Tntc,TIC,C1V,C2V,C3V,C13V,C14V,C15V,C4V,C5V,C6V,C16V,C17V,C18V,C7V,C8V,C9V,C19V,C20V,C21V,C10V,C11V,C12V,C22V,C23V,C24V,fI2,fBAT,OK/ERR
	STAT:0xFFFFFFFFFFFFFFFFFFFF,ERR:0xB
	-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,0xFFFFFFFFFFFFFFFFFFFFFFFF,0xFFFFFFFFFFFFFFFFFFFFFFFF,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,ERR:0xB
	!!!!
	!!!! Didn't work so we try with the opposite configuration (ON TOP OF DAISYCHAIN)
	!!!!
	!!!! (we keep doing this, until we find a functional configuration)
	!!!!
	INIT,ON TOP OF DAISYCHAIN,0xFFFFFFFFFFFFFFFFFFFF,STAT:0xFFFFFFFFFFFFFFFFFFFF,ERR:0xB
	ERR:0xB
	tDut,I1,P1,BAT,Tntc,TIC,C1V,C2V,C3V,C13V,C14V,C15V,C4V,C5V,C6V,C16V,C17V,C18V,C7V,C8V,C9V,C19V,C20V,C21V,C10V,C11V,C12V,C22V,C23V,C24V,fI2,fBAT,OK/ERR
	STAT:0xFFFFFFFFFFFFFFFFFFFF,ERR:0x9
	-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,0xFFFFFFFFFFFFFFFFFFFFFFFF,0xFFFFFFFFFFFFFFFFFFFFFFFF,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,ERR:0xB
	...
	...
	!!!!
	!!!! we keep doing this, until we find a functional configuration
	!!!!
	...
	...
	tDut,I1,P1,BAT,Tntc,TIC,C1V,C2V,C3V,C13V,C14V,C15V,C4V,C5V,C6V,C16V,C17V,C18V,C7V,C8V,C9V,C19V,C20V,C21V,C10V,C11V,C12V,C22V,C23V,C24V,fI2,fBAT,OK/ERR
	STAT:0x00000000000000000000,ERR:0xA
	0.0,0.0,0.0,0.0,0.0,0.0,0x000000000000000000000000,0x000000000000000000000000,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,ERR:0xA
	INIT,ON TOP OF DAISYCHAIN,0xCFB7FF03000001000007,STAT:0x00000000000000000000,ERR:0xA
	ERR:0xB
	...
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	!!!!!We connect LTC2949 parallel to daisychain !!!
	!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	...
	tDut,I1,P1,BAT,Tntc,TIC,C1V,C2V,C3V,C13V,C14V,C15V,C4V,C5V,C6V,C16V,C17V,C18V,C7V,C8V,C9V,C19V,C20V,C21V,C10V,C11V,C12V,C22V,C23V,C24V,fI2,fBAT,OK/ERR
	STAT:0x10000000000000000000,OK
	0.0,0.9,0.9,0.9,0.9,0.9,0x060000000000060000000000,0x060000000000060000000000,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,0.0,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,-0.0,OK
	INIT,PARALLEL TO DAISYCHAIN,0x10000000000000000000,STAT:0x10000000000000000000,OK
	!!!!
	!!!! found new functional configuration (PARALLEL TO DAISYCHAIN)
	!!!!
	100,0,0,-0.00,25.8,28.0,0.0,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.0,0,-0.00,1228,OK
	100,1,6,-0.00,25.8,28.0,0.0,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.0,0,-0.00,1224,OK
	99,0,0,-0.00,25.8,28.0,0.0,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.0,0,-0.00,2476,OK
	100,0,6,-0.00,25.8,28.0,0.0,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.9,0.0,0,-0.00,1220,OK
	...
	...
	!!!! normal operation
	...
	...
	*/
}

void setup()
{
	//Initialize serial and wait for port to open:
	Serial.begin(LTCDEF_BAUDRATE);
	// wait for serial port to connect. Needed for native USB port only
	// while (!Serial);
	// disable drivers on all SPI pins
	// we do this, as to allow compatibility of Arduino Due
	// with existing Arduino shields that expect SPI signals 
	// on those pins, by routing the SPI header of DUE to
	// those pins.
  delay(50);
	pinMode(11, INPUT);
	pinMode(12, INPUT);
	pinMode(13, INPUT);

	digitalWrite(LTCDEF_GPO, HIGH);
	pinMode(LTCDEF_GPO, OUTPUT);

  pinMode( PEC_ERR_PIN , OUTPUT );
  pinMode( VOLT_ERR_PIN , OUTPUT );
  pinMode( TEMP_ERR_PIN , OUTPUT );
  pinMode( BMS_FLT_3V3 , OUTPUT );     // Bpin defined for sending signal to ACU for BMS error
  pinMode( FAN_MBED , OUTPUT );
  pinMode( CH_EN_MBED , OUTPUT );

  
  initialiseSDcard();

  initialiseNormalizeChannel();
  setNormalizeChannels(slavesToNormalize, voltChannelToNormalize, tempChannelToNormalize );

  initialiseSplitChannel();
  setVoltSplitChannels(slavesOnSplit, voltChannelOnSplit );

  digitalWriteFast( BMS_FLT_3V3 , LOW);
  checkACUsignalStatus();
  delay(10000);
	// configure SPI, also done in LTC2949.cpp:
	// also used for LTC681x
	// LTC2949_SPISettings = SPISettings(LTC2949_MAX_SPIFREQU, MSBFIRST, LTC2949_DEFAULT_SPIMODE);
  LTC2949_SPISettings = SPISettings(1000000, MSBFIRST, LTC2949_DEFAULT_SPIMODE);
  #ifndef circular
	Init(LTCDEF__CS, false);
  #endif

  initialiseFlags();//initialises BMS,VOLT,TEMP Flags to False
  initialiseCAN();//enables CAN communication for charger
  initCAN();//enables CAN communication for ECU

  for (int i = 0 ; i < LTCDEF_CELL_MONITOR_COUNT ; i++){
    for (int j = 0 ; j<18 ; j++){
      cells2[i][j]=6;
    }
  }
  
}


void Init(uint8_t selCS, boolean ltc2949onTopOfDaisychain)
{
#ifdef LTCDEF_LTC681X_ONLY
	ltc2949onTopOfDaisychain = true;
#endif

	LTC2949_CS = selCS;
	//Initialize LTC2949 library
	LTC2949_init_lib(
		/*byte cellMonitorCount,			*/LTCDEF_CELL_MONITOR_COUNT,
		/*boolean ltc2949onTopOfDaisychain, */ltc2949onTopOfDaisychain,
		/*boolean debugEnable				*/false
	);
	LTC2949_init_device_state();
	// 
	Serial.print(F("INIT,"));
#ifndef LTCDEF_LTC681X_ONLY
	if (LTC2949_onTopOfDaisychain)
		Serial.print(F("ON TOP OF"));
	else
		Serial.print(F("PARALLEL TO"));
	Serial.print(F(" DAISYCHAIN,"));
#endif

	Serial.print(F("CS:"));
	Serial.print(LTC2949_CS);
	PrintComma();
	//
	delay(LTC2949_TIMING_BOOTUP);
	byte error = 0;
#ifdef LTCDEF_LTC681X_ONLY
	mcuTime = millis();
	error |= CellMonitorInit();
#else
if(loopcount){
	error = WakeUpReportStatus();
  if(error)
  {
    digitalWriteFast(BMS_FLT_3V3,LOW);
  }
	error |= Cont(false);
	mcuTime = millis();
	delay(LTC2949_TIMING_CONT_CYCLE);
  //Serial.println("code entering CellMonitorInit");
	error |= CellMonitorInit();
	error |= Cont(true);
}
#endif
  //PrintComma();
	//PrintOkErr(error);
	//PrintCSVHeader();
}

void PrintCellVoltages(uint16_t * cellMonDat, boolean init)
{
	for (uint8_t i = 0; i < LTCDEF_CELL_MONITOR_COUNT * 3; i++)
	{
    if (cellMonDat[i] != 0xFFFFU) // don't print non-existing cells
			//Serial.print(cellMonDat[i] * 100e-6, LTCDEF_DIGITS_CELL);
		if (init)
			cellMonDat[i] = 0xFFFFU; // this allows to check if next cell voltages were read correctly
		//PrintComma();
	}
}

void PrintAuxVoltages(uint16_t * cellMonDat, boolean init)
{
	for (uint8_t i = 0; i < LTCDEF_CELL_MONITOR_COUNT * 3; i++)
	{
		if (cellMonDat[i] != 0xFFFFU) // don't print non-existing cells
			//Serial.print(cellMonDat[i] * 100e-6, LTCDEF_DIGITS_CELL);
		if (init)
			cellMonDat[i] = 0xFFFFU; // this allows to check if next cell voltages were read correctly
		//PrintComma();
	}
}

String CellVoltagesToString(uint16_t * cellMonDat, uint8_t nic)
{
	String str = "";
	uint8_t i = nic * 3;
	uint8_t lasti = i + 3;
	for (; i < lasti; i++)
	{
		if (cellMonDat[i] != 0xFFFFU)
			str += String(cellMonDat[i] * 100e-6, LTCDEF_DIGITS_CELL);
      //Serial.println(cellMonDat[i] * 100e-6, LTCDEF_DIGITS_CELL);
		str += ',';
	}
  // Serial.println("CellData");
  // Serial.println(cellMonDat[0] * 100e-6, LTCDEF_DIGITS_CELL);

	return str;
}

void loop()
{
  #ifdef circular
   if(loopcount){
    Init(LTCDEF__CS,false);
    Serial.println("");
    Serial.println("Entered normal daisy chain");
  }
  else{
    Init(LTCDEF__CS2,false);
    Serial.println("");
    Serial.println("Entered backward daisy chain");
  }
  #endif

  #ifndef GUI_Enabled
	Serial.print("Start of the code ");
  #endif
  float ti = millis();
  String str = "";

	unsigned long timeBuffer = millis();
	unsigned long timerus = micros();


#ifdef LTCDEF_DO_RESET_TEST
	if (deltaT > LTCDEF_DO_RESET_TEST / 1.0e3 / LTC2949_LSB_TB1)
	{
		LTC2949_reset();
		delay(LTC2949_TIMING_AUTO_SLEEP_MAX*1.5);
		Init(LTC2949_CS, LTC2949_onTopOfDaisychain);
		return;
  }
#endif // LTCDEF_DO_RESET_TEST

	// LTC2949_ChkUpdate checks for changed in TBx (default is to check TB4)
	// this tells if we have updated values available in the slow channel registers
#ifdef LTCDEF_LTC681X_ONLY
	const boolean slowChannelReady = false;
#else
  boolean slowChannelReady = true;
  if(loopcount){
	boolean slowChannelReady = LTC2949_ChkUpdate(&error);
	if (slowChannelReady || LTC_TIMEOUT_CHECK(timeBuffer, mcuTime + LTC2949_TIMING_CONT_CYCLE))
	{
		// in case of any error below we will also enter here! (see last delay(LTC2949_TIMING_IDLE2CONT2UPDATE))
		error |= ChkDeviceStatCfg();
		slowChannelReady = true;
	}
  }
#endif

	// new high precision results available
	// calc difference between last TBx and current TBx, report milliseconds
#ifndef LTCDEF_LTC681X_ONLY
if(loopcount){
	if (slowChannelReady){
		str += String((unsigned int)((LTC2949_GetLastTBxInt() - deltaT) * (1.0e3 * LTC2949_LSB_TB1))); }//Serial.print((unsigned int)((LTC2949_GetLastTBxInt() - deltaT) * (1.0e3 * LTC2949_LSB_TB1)));
	else{}
}
#endif
		str += String(timeBuffer - mcuTime); //Serial.print(timeBuffer - mcuTime);
	mcuTime = timeBuffer;
	str += ',';//PrintComma();

#ifndef LTCDEF_LTC681X_ONLY
if(loopcount){
	// read high precision current I1
	if (slowChannelReady)
	{
		error |= LTC2949_READ(LTC2949_VAL_I1, 3, buffer);
		str += String(LTC_3BytesToInt32(buffer) * LTC2949_LSB_I1 / LTCDEF_SENSE_RESISTOR, LTCDEF_DIGITS_I1SLOW); //Serial.print(LTC_3BytesToInt32(buffer) * LTC2949_LSB_I1 / LTCDEF_SENSE_RESISTOR, LTCDEF_DIGITS_I1SLOW);
    batCurrPower_num[0] = LTC_3BytesToInt32(buffer) * LTC2949_LSB_I1 / LTCDEF_SENSE_RESISTOR, LTCDEF_DIGITS_I1SLOW;
    batCurrPower[0] = String(batCurrPower_num[0]);
    
    //if (batCurrPower[0]>-1){
    //  batCurrPower[0] = 0;
    //}

	}
	str += ',';//PrintComma();

	// read high precision power P1
	if (slowChannelReady)
	{
		error |= LTC2949_READ(LTC2949_VAL_P1, 3, buffer);
		str += String(LTC_3BytesToInt32(buffer) * LTC2949_LSB_P1 / LTCDEF_SENSE_RESISTOR, LTCDEF_DIGITS_P1SLOW); //Serial.print(LTC_3BytesToInt32(buffer) * LTC2949_LSB_P1 / LTCDEF_SENSE_RESISTOR, LTCDEF_DIGITS_P1SLOW);
	  batCurrPower[1] = String(LTC_3BytesToInt32(buffer) * LTC2949_LSB_P1 / LTCDEF_SENSE_RESISTOR * 13.75786, LTCDEF_DIGITS_P1SLOW);
  }
	str += ',';//PrintComma();

	// read voltage BAT
	if (slowChannelReady)
	{
		error |= LTC2949_READ(LTC2949_VAL_BAT, 2, buffer);
    str += " BATV ";
		str += String(LTC_2BytesToInt16(buffer) * LTC2949_LSB_BAT * POT_DIV_BPM, LTCDEF_DIGITS_BATSLOW); //Serial.print(LTC_2BytesToInt16(buffer) * LTC2949_LSB_BAT, LTCDEF_DIGITS_BATSLOW);
	  batVoltage_num = LTC_2BytesToInt16(buffer) * LTC2949_LSB_BAT * POT_DIV_BPM, LTCDEF_DIGITS_BATSLOW;
    batVoltage = String(batVoltage_num);
   // batVoltage = String(LTC_2BytesToInt16(buffer) * LTC2949_LSB_BAT * POT_DIV_BPM, LTCDEF_DIGITS_BATSLOW);
  }
	str += ',';//PrintComma();

	// read temperature via SLOT1
	if (slowChannelReady)
	{
		error |= LTC2949_READ(LTC2949_VAL_SLOT1, 2, buffer);
		str += String(LTC_2BytesToInt16(buffer) * LTC2949_LSB_TEMP, LTCDEF_DIGITS_TEMPSLOW); //Serial.print(LTC_2BytesToInt16(buffer) * LTC2949_LSB_TEMP, LTCDEF_DIGITS_TEMPSLOW);
	}
	str += ',';//PrintComma();

	// read internal temperature
	if (slowChannelReady)
	{
		error |= LTC2949_READ(LTC2949_VAL_TEMP, 2, buffer);
		str += String(LTC_2BytesToInt16(buffer) * LTC2949_LSB_TEMP, LTCDEF_DIGITS_TEMPSLOW); //Serial.print(LTC_2BytesToInt16(buffer) * LTC2949_LSB_TEMP, LTCDEF_DIGITS_TEMPSLOW);
	}
	str += ',';//PrintComma();
}
#endif

#ifdef LTCDEF_TIMING_DEBUG
	str += String(micros() - timerus);
	str += ',';
#endif

	////////////////////////////////////////////////////////////////////
	// fast synchronous cell voltage and current measurement
	////////////////////////////////////////////////////////////////////
	// clear old cell voltage conversion results
  
  if(loopcount){
	error |= LTC2949_68XX_ClrCells();
  error |= LTC2949_68XX_ClrAux();

	error |= CellMonitorCFGA((byte*)cellMonDat, false);
  error |= CellMonitorCFGB((byte*)cellMonDat, false , false);

	if (err_detected(error))
	{
		str = "";
		// not yet initialized or communication error or.... (e.g. ON TOP OF instead of PARALLEL TO DAISYCHAIN)
		PrintOkErr(error);
		delay(100); // we wait 0.1 second, just avoid too many trials in case of error.

		if (retries--)
		{
			Init(LTC2949_CS, LTC2949_onTopOfDaisychain);
		}
		else
		{
			// lets try to toggle isoSPI bus configuration and LTC6820 master (four possible combinations)
			// Note: here we treat all possible combinations to be equally likely. This way the boards
			// can be connected together in any way.
			// In a real system only two combinations would make sense.
			switch (LTC2949_CS + (LTC2949_onTopOfDaisychain ? (uint8_t)128 : (uint8_t)0))
			{
			case (LTCDEF__CS + 0):
				Init(LTCDEF_GPO, false);
				break;
			case (LTCDEF_GPO + 0):
				Init(LTCDEF__CS, true);
				break;
			case (LTCDEF__CS + 128):
				Init(LTCDEF_GPO, true);
				break;
			case (LTCDEF_GPO + 128):
			default:
				Init(LTCDEF__CS, false);
				break;
			}
			retries = LTCDEF_ERR_RETRIES;
		}
		return;
	}
  }

	if (LTCDEF_POLL_EOC)
		timeBuffer = micros();

	// trigger measurement (broadcast command will trigger cell voltage and current measurement)
  BPM_ready=slowChannelReady;
  pull_3V3_high();
  switchErrorLed();
  clearFlags();
  cellVoltageLoop(timeBuffer);
  #ifndef GUI_Enabled
  Serial.println();
  #endif
  
 
  cellTempLoop(timeBuffer);

  

  findMax(cellVoltages, voltage);
  findMax(cellTemperatures, temp);
  findMin(cellVoltages);
   #ifndef GUI_Enabled
   printMaxMinParameters();
   #endif

  #ifdef dynamicCooling
  performDynamicCooling();
  #endif

  checkError();
  #ifndef GUI_Enabled
  triggerError();
  #endif
  #ifdef GUI_Enabled
  triggerError_GUI();
  #endif

  
  batLoop(slowChannelReady);

  // #ifdef GUI_Enabled
  // Serial.print(CellData);
  //   CellData = "";
  // #endif

  

  #ifdef charger_active
  sendAlldatatoecu();
  #endif


  #ifdef charger_active
  if (!(voltFlag || tempFlag))
  {

  chargerLoop();
  triggerchargerError();
  }
  #endif


  #ifndef GUI_Enabled
  triggerInterrupt();
  #endif

  #ifdef GUI_Enabled
  triggerInterrupt_GUI();
  #endif


  // #ifdef GUI_Enabled
  // Serial.println(CellData);
  //   CellData = "";
  // #endif

  

  

#ifdef LTCDEF_TIMING_DEBUG
	str += String(micros() - timerus);
	str += ',';
#endif


#ifndef LTCDEF_LTC681X_ONLY
if(loopcount){
	// print fast I2
	str += String(fastData2949[LTC2949_RDFASTDATA_I2] * LTC2949_LSB_FIFOI2 / LTCDEF_SENSE_RESISTOR, LTCDEF_DIGITS_I2FAST); //Serial.print(fastData2949[LTC2949_RDFASTDATA_I2] * LTC2949_LSB_FIFOI2 / LTCDEF_SENSE_RESISTOR, LTCDEF_DIGITS_I2FAST);
	str += ',';//PrintComma();
	// print fast BAT
	str += String(fastData2949[LTC2949_RDFASTDATA_BAT] * LTC2949_LSB_FIFOBAT, LTCDEF_DIGITS_BATFAST); //Serial.print(fastData2949[LTC2949_RDFASTDATA_BAT] * LTC2949_LSB_FIFOBAT, LTCDEF_DIGITS_BATFAST);
	// clear the EOC by reading again if necessary:
	if (!LTC2949_FASTSSHT_HS_CLR(fastData2949))
		error |= LTC2949_RdFastData(fastData2949);
	if (!LTC2949_FASTSSHT_HS_CLR(fastData2949)) // for sure HS bytes must be cleared now
		error |= LTC2949_ERRCODE_OTHER;
	str += ',';//PrintComma();
	str += String(deltaT); //Serial.print(deltaT);
	str += ',';//PrintComma();
}
#endif
	// Serial.print(str);
  // Print
  // Serial.println(myCvs[0]);
  // Serial.print("BPM : ");
  // Serial.println(bpmDat[2]);
	str = "";
	// PrintOkErr(error);
	if (err_detected(error)) // in case of error we sleep to avoid too many error reports and also to make sure we call ChkDeviceStatCfg in the next loop 
		delay(LTC2949_TIMING_IDLE2CONT2UPDATE);

  #ifdef GUI_Enabled
  
  Serial.println();
  
  float tf = millis() - ti;
  
  CellData_GUI = "";
  CellData_GUI += String(tf,2);
  CellData_GUI += ",";
  Serial.print(CellData_GUI);
  

  

  CellData_GUI = "";
  
  Serial.println();
  
  #endif


  #ifdef startSOC
  if (SOC_init_flag==false)
  {
    EnergyAvailable = InitialiseEnergy(minVoltage.val, underVoltageThreshold);
    Serial.print("Energy Available :");
    Serial.print(EnergyAvailable);
    Serial.print("Kwh");
    SOC_init_flag=true;
  }
  else
  {

    EnergyAvailable = CalculateEnergy(batVoltage_num, batCurrPower_num[0], EnergyAvailable, tf, chgrDelay);
    Serial.print("Energy Available in Kwh:");
    Serial.print(EnergyAvailable);
    Serial.print("Kwh");
  }

  Serial.println();
  Serial.print("SoC: ");
  Serial.print(EnergyAvailable*1000*1000*100/(4200*5.5*3.7*90));
  Serial.print("%");
  #endif
  
  #ifndef GUI_Enabled
  Serial.println();
  tf = millis() - ti;
  Serial.print("LOOP TIME : ");
  Serial.print(tf);
  Serial.println("ms");
  #endif

  #ifdef charger_active
  delay(chgrDelay);
  #endif

  #ifdef circular
  loopcount=!loopcount ;
  #endif
}

void cellVoltageLoop (unsigned long timeBuffer)
{
  	byte  error = 0;
    error |= LTC2949_ADxx(
		/*byte md = MD_NORMAL     : */MD_FAST,
		/*byte ch = CELL_CH_ALL   : */CELL_CH_ALL,
		/*byte dcp = DCP_DISABLED : */DCP_DISABLED,
		/*uint8_t pollTimeout = 0 : */LTCDEF_POLL_EOC ? LTC2949_68XX_GETADCVTIMEOUT16US(LTC2949_68XX_T6C_27KHZ_US) : 0
	);


#ifdef LTCDEF_TIMING_DEBUG
	str += String(micros() - timerus);
	str += ',';
#endif


	cellMonDat[0] = 0xFFFFU; // will be used later

#ifdef LTCDEF_LTC681X_ONLY
	timeBuffer = micros() + LTC2949_68XX_T6C_27KHZ_US;
	// for sure we poll for HS of LTC2949's rdcv later, so we do not have to wait here!
	while (!LTC_TIMEOUT_CHECK(micros(), timeBuffer))
		; // wait for all cell voltage measurements to be completed.
	error |= LTC2949_68XX_RdCells(LTC2949_68XX_CMD_RDCVA, cellMonDat);

#ifdef LTCDEF_TIMING_DEBUG
	str += String(micros() - timerus);
	str += ',';
#endif

#else
if(loopcount){
	// if (!LTCDEF_POLL_EOC)
	// {
	// 	timeBuffer = micros();
	// 	// for sure we poll for HS of LTC2949's rdcv later, so we do not have to wait here!
	// 	while (false && !LTC_TIMEOUT_CHECK(micros(), timeBuffer + LTC2949_68XX_T6C_27KHZ_US))
	// 		; // wait for all cell voltage measurements to be completed.
	// }
	fastData2949[LTC2949_RDFASTDATA_HS] = 0; // clear the HS bytes
	// poll LTC2949 for conversion done
	error |= LTC2949_RdFastData(
		fastData2949,
		cellMonDat,
		LTC2949_68XX_CMD_RDCVA,
    // LTC2949_68XX_CMD_RDAUXA,
		LTC2949_68XX_GETADCVTIMEOUT16US(LTC2949_FASTSSHT_RDY_TIME_US));
	// calc cycle time (ADCV to "results ready")
	deltaT = micros() - timeBuffer;
	// check if we already read valid data from LTC2949
	if (LTC2949_FASTSSHT_HS_OK(fastData2949))
	{
		; // all fine, nothing to do.
	}
	else if (LTC2949_FASTSSHT_HS_CLR(fastData2949))
	{
		// we polled for HS==0x0F before, so it cannot be all HS are zero! something went wrong (e.g. timeout)
		error |= LTC2949_ERRCODE_OTHER;
	}
	else if (LTC2949_FASTSSHT_HS_LAST_OK(fastData2949)) // first HS != 0x0F, last HS == 0x0F
	{
		// we have to read data from LTC2949 again, as only the next RDCVx will report the final conversion results
		// also cell voltages have to be read again, as also those most probably were not updated
		// note: here we must not poll HS! (it must be zero now!)
		error |= LTC2949_RdFastData(
			fastData2949,
			cellMonDat,
			LTC2949_68XX_CMD_RDCVA);
      // LTC2949_68XX_CMD_RDAUXA);

		if (!LTC2949_FASTSSHT_HS_CLR(fastData2949)) // HS must be cleared now
			error |= LTC2949_ERRCODE_OTHER; // this must never happen in case of fast single shot events
	}
	else
	{
		// Unexpected HS bytes, something went wrong
		error |= LTC2949_ERRCODE_OTHER;
	}
} 
#endif

	char errorExt = error > 1 ? 'X' : '_';
	if (!LTC2949_onTopOfDaisychain || (cellMonDat[0] == 0xFFFFU))
	{
		if (errorExt == '_') errorExt = 'Y';

		// we have to read cell voltages group A (again)
		// for sure we have to read in case LTC2949 is not on top of daisychain!
		error |= LTC2949_68XX_RdCells(LTC2949_68XX_CMD_RDCVA, cellMonDat);
	}

		String cvs[LTCDEF_CELL_MONITOR_COUNT];

		for (uint8_t i = 0; i < LTCDEF_CELL_MONITOR_COUNT; i++)
			cvs[i] = "";

		for (uint8_t rdcvi = 0; ; rdcvi++)
		{
			for (uint8_t i = 0; i < LTCDEF_CELL_MONITOR_COUNT; i++)
			{
        cvs[i] += CellVoltagesToString(cellMonDat, i);
        transferV(cellMonDat,i,rdcvi);
      }

			if ((rdcvi > 4) || (rdcvi >= (LTCDEF_CELLS_PER_CELL_MONITOR_COUNT / 3 - 1)))
				break;

			switch (rdcvi)
			{
			case 0:
				error |= LTC2949_68XX_RdCells(LTC2949_68XX_CMD_RDCVB, cellMonDat);
				if (error > 1 && errorExt == '_') errorExt = 'b';
				break;
			case 1:
				error |= LTC2949_68XX_RdCells(LTC2949_68XX_CMD_RDCVC, cellMonDat);
				if (error > 1 && errorExt == '_') errorExt = 'c';
				break;
			case 2:
				error |= LTC2949_68XX_RdCells(LTC2949_68XX_CMD_RDCVD, cellMonDat);
				if (error > 1 && errorExt == '_') errorExt = 'd';
				break;
			case 3:
				error |= LTC2949_68XX_RdCells(LTC2949_68XX_CMD_RDCVE, cellMonDat);
				if (error > 1 && errorExt == '_') errorExt = 'e';
				break;
			case 4:
				error |= LTC2949_68XX_RdCells(LTC2949_68XX_CMD_RDCVF, cellMonDat);
				if (error > 1 && errorExt == '_') errorExt = 'f';
				break;
			
		  }
		// for (uint8_t i = 0; i < LTCDEF_CELL_MONITOR_COUNT; i++)
		// {
    //   str += cvs[i]; //Serial.print(cvs[i]);
		// 	cvs[i] = "";
		// }
		// str += errorExt; //Serial.print(errorExt);
		// str += ',';//PrintComma();
	  }
  circularVoltdef();
  cellsVoltSort();
  checkVoltNormalize();
  checkVoltSplit();
  cellsLogging();
  #ifndef GUI_Enabled
  printCells();
  #endif
  

}


void cellTempLoop(unsigned long timeBuffer)
{
  for ( int muxSelect = 1; muxSelect >= 0; muxSelect--)
  {
     byte  error = 0;
     error |= CellMonitorCFGB((byte*)cellMonDat, false , muxSelect); 	
        
        error |= LTC2949_ADAX(
		/*byte md = MD_NORMAL     : */MD_FAST,
		/*byte ch = CELL_CH_ALL   : */CELL_CH_ALL,
		/*byte dcp = DCP_DISABLED : */DCP_DISABLED,
		/*uint8_t pollTimeout = 0 : */LTCDEF_POLL_EOC ? LTC2949_68XX_GETADCVTIMEOUT16US(LTC2949_68XX_T6C_27KHZ_US) : 0
	);


#ifdef LTCDEF_TIMING_DEBUG
	str += String(micros() - timerus);
	str += ',';
#endif


	cellMonDat[0] = 0xFFFFU; // will be used later

#ifdef LTCDEF_LTC681X_ONLY
	timeBuffer = micros() + LTC2949_68XX_T6C_27KHZ_US;
	// for sure we poll for HS of LTC2949's rdcv later, so we do not have to wait here!
	while (!LTC_TIMEOUT_CHECK(micros(), timeBuffer))
		; // wait for all cell voltage measurements to be completed.
	error |= LTC2949_68XX_RdAux(LTC2949_68XX_CMD_RDAUXA, cellMonDat);

#ifdef LTCDEF_TIMING_DEBUG
	str += String(micros() - timerus);
	str += ',';
#endif

#else
if(loopcount){
	if (!LTCDEF_POLL_EOC)
	{
		timeBuffer = micros();
		// for sure we poll for HS of LTC2949's rdcv later, so we do not have to wait here!
		while (false && !LTC_TIMEOUT_CHECK(micros(), timeBuffer + LTC2949_68XX_T6C_27KHZ_US))
			; // wait for all cell voltage measurements to be completed.
	}
	fastData2949[LTC2949_RDFASTDATA_HS] = 0; // clear the HS bytes
	// poll LTC2949 for conversion done
	error |= LTC2949_RdFastData(
		fastData2949,
		cellMonDat,
		// LTC2949_68XX_CMD_RDCVA,
    LTC2949_68XX_CMD_RDAUXA,
		LTC2949_68XX_GETADCVTIMEOUT16US(LTC2949_FASTSSHT_RDY_TIME_US));
	// calc cycle time (ADCV to "results ready")
	deltaT = micros() - timeBuffer;
	// check if we already read valid data from LTC2949
	if (LTC2949_FASTSSHT_HS_OK(fastData2949))
	{
		; // all fine, nothing to do.
	}
	else if (LTC2949_FASTSSHT_HS_CLR(fastData2949))
	{
		// we polled for HS==0x0F before, so it cannot be all HS are zero! something went wrong (e.g. timeout)
		error |= LTC2949_ERRCODE_OTHER;
	}
	else if (LTC2949_FASTSSHT_HS_LAST_OK(fastData2949)) // first HS != 0x0F, last HS == 0x0F
	{
		// we have to read data from LTC2949 again, as only the next RDCVx will report the final conversion results
		// also cell voltages have to be read again, as also those most probably were not updated
		// note: here we must not poll HS! (it must be zero now!)
		error |= LTC2949_RdFastData(
			fastData2949,
			cellMonDat,
			// LTC2949_68XX_CMD_RDCVA);
      LTC2949_68XX_CMD_RDAUXA);

		if (!LTC2949_FASTSSHT_HS_CLR(fastData2949)) // HS must be cleared now
			error |= LTC2949_ERRCODE_OTHER; // this must never happen in case of fast single shot events
	}
	else
	{
		// Unexpected HS bytes, something went wrong
		error |= LTC2949_ERRCODE_OTHER;
	}
}
#endif

	char errorExt = error > 1 ? 'X' : '_';
	if (!LTC2949_onTopOfDaisychain || (cellMonDat[0] == 0xFFFFU))
	{
		if (errorExt == '_') errorExt = 'Y';

		// we have to read cell voltages group A (again)
		// for sure we have to read in case LTC2949 is not on top of daisychain!
		error |= LTC2949_68XX_RdAux(LTC2949_68XX_CMD_RDAUXA, cellMonDat);
	}
	
		String cvs[LTCDEF_CELL_MONITOR_COUNT];

		for (uint8_t i = 0; i < LTCDEF_CELL_MONITOR_COUNT; i++)
			cvs[i] = "";

		for (uint8_t rdcvi = 0; rdcvi < 3; rdcvi++)
		{
			for (uint8_t i = 0; i < LTCDEF_CELL_MONITOR_COUNT; i++)
      {
				cvs[i] += CellVoltagesToString(cellMonDat, i);
        transferT(cellMonDat,i,muxSelect,rdcvi);
      }
			// if ((rdcvi > 4) || (rdcvi >= (LTCDEF_CELLS_PER_CELL_MONITOR_COUNT / 3 - 1)))
			// 	break;

			switch (rdcvi)
			{
			case 0:
				error |= LTC2949_68XX_RdAux(LTC2949_68XX_CMD_RDAUXB, cellMonDat);
				if (error > 1 && errorExt == '_') errorExt = 'b';
				break;
			case 1:
				error |= LTC2949_68XX_RdAux(LTC2949_68XX_CMD_RDAUXC, cellMonDat);
				if (error > 1 && errorExt == '_') errorExt = 'c';
				break;
			case 2:
				error |= LTC2949_68XX_RdAux(LTC2949_68XX_CMD_RDAUXD, cellMonDat);
				if (error > 1 && errorExt == '_') errorExt = 'd';
				break;
			}
		}
		// for (uint8_t i = 0; i < LTCDEF_CELL_MONITOR_COUNT; i++)
		// {
    //   str += cvs[i]; //Serial.print(cvs[i]);
		// 	cvs[i] = "";
		// }
		// str += errorExt; //Serial.print(errorExt);
		// str += ',';//PrintComma();

	circularTempdef(muxSelect);
  tempConvertSort(muxSelect);

  }
  
  checkTempNormalize();
  #ifndef GUI_Enabled
  printAux();
  #endif
  
  auxLogging();

 
}

void batLoop(bool slowChannelReady )
{
  if (slowChannelReady)
  {
  #ifndef GUI_Enabled
  Serial.println();
  Serial.print("BATTERY VOLTAGE : ");
  Serial.print( batVoltage );
  Serial.println();
  Serial.print("TS CURRENT : ");
  Serial.print( batCurrPower[0] );
  Serial.println();
  Serial.print("TS POWER : ");
  Serial.print( batCurrPower[1] );
  Serial.println();
  #endif

  CellData += batVoltage;
  CellData += ",";
  CellData += batCurrPower[0];
  CellData += ",";
  CellData += batCurrPower[1];
  CellData += ",";
  
  #ifdef GUI_Enabled
  CellData_GUI += batVoltage;
  CellData_GUI += ",";
  CellData_GUI += batCurrPower[0];
  CellData_GUI += ",";
  CellData_GUI += batCurrPower[1];
  CellData_GUI += ",";

  Serial.println(CellData_GUI);
  CellData_GUI="";
  #endif


  
  }
  #ifdef startLogging
  SDcardLogging();
  #endif
}

void sendDataToECU(float voltage, float temperature) {
    CAN_message_t msg;

    // Use unions to convert float to byte arrays
    union {
        float f;
        uint8_t bytes[4];
    } voltageData, tempData;

    voltageData.f = voltage;
    tempData.f = temperature;

    // Prepare CAN message (8 bytes: 4 for voltage, 4 for temperature)
    msg.id = 0x100;  // Example message ID (change as needed)
    msg.len = 8;     // Data length is 8 bytes

    // Pack the voltage and temperature into the CAN message
    for (int i = 0; i < 4; i++) {
        msg.buf[i] = voltageData.bytes[i];
        msg.buf[i + 4] = tempData.bytes[i];
    }

    // Send the CAN message
    Can2.write(msg);
}


void sendAllDataToECU() {
    for (int i = 0; i < 15; i++) {
        sendDataToECU(voltageDataArray[i], temperatureDataArray[i]);  // Send each pair
        delay(10);  // Small delay between messages to avoid bus overload
    }


void chargerLoop(void)
{
     send_msg.flags.extended = 1;
     receive_msg.flags.extended = 1;
     send_msg.id=send_id;
    if(chargerFlag == false)
    {
    send_msg.buf[0]= (max_voltage)>>8;
    send_msg.buf[1]= (max_voltage)%256;
    send_msg.buf[2]= (max_current_without_decimal)>>8;
    send_msg.buf[3]= (max_current_without_decimal)%256;
    send_msg.buf[4]= 0;
    Serial.println("Set control bit to 0: Charger OK");
    }
    else{
    send_msg.buf[0]= (max_voltage)>>8;
    send_msg.buf[1]= (max_voltage)%256;
    send_msg.buf[2]= (max_current_without_decimal)>>8;
    send_msg.buf[3]= (max_current_without_decimal)%256;
    send_msg.buf[4]= 1;
    Serial.println("Set control bit to 1: Do not charge");
    }

    output_voltage = (receive_msg.buf[0]*256 + receive_msg.buf[1] + 1.5)/10;
    output_current = (receive_msg.buf[2]*256 + receive_msg.buf[3])/10;
    //need to add the fifth control byte also
    //Serial.println(send_msg.buf[2]);
    //Serial.println(send_msg.buf[3]);
  #ifndef GUI_Enabled
  Serial.println("Entered Charger loop");
  #endif
  // #ifdef charger_softEnabled

  //  if ((max_voltage<100))
  //  {
  //if ((can3.write(send_msg)!=1))

  //{
  // double ti = millis(),delT = 0.0;
  // char input='0';
  // #ifndef GUI_Enabled
  // while( delT < chargeTimer )
  // {
    
  //   delT = millis() - ti ;
  //   if (input=='c')
  //   { 
  //     send_msg.flags.extended = 1;
  //    receive_msg.flags.extended = 1;
  //    send_msg.id=send_id;
 
  //     max_voltage=3950;
  //     max_current_without_decimal=100;
  //     send_msg.buf[0]= (max_voltage)>>8;
  //   send_msg.buf[1]= (max_voltage)%256;
  //   send_msg.buf[2]= (max_current_without_decimal)>>8;
  //   send_msg.buf[3]= (max_current_without_decimal)%256;

  //     Serial.println("Charger Enable Received");
  //     can3.write(send_msg);
  //     Serial.println(can3.write(send_msg));
  //     break;

  //   }
  //   else
  //   { max_voltage=0;
  //     max_current_without_decimal=0;
  //     send_msg.buf[0]= (max_voltage)>>8;
  //   send_msg.buf[1]= (max_voltage)%256;
  //   send_msg.buf[2]= (max_current_without_decimal)>>8;
  //   send_msg.buf[3]= (max_current_without_decimal)%256;
  //     Serial.println("BMS OK....Waiting for charger Enable");
  //     can3.write(send_msg);
  //     Serial.println(can3.write(send_msg));
  //   }
  //   if (Serial.available() > 0)
  //   {
  //     input = read_char();
  //   } 
  //   delay(100); 
  // }
  // #endif
  // #ifdef GUI_Enabled
  // while( delT < chargeTimer )
  // {
    
  //   delT = millis() - ti ;
  //   if (input=='c')
  //   {
  //     send_msg.flags.extended = 1;
  //    receive_msg.flags.extended = 1;
  //    send_msg.id=send_id;
 
  //     max_voltage=3950;
  //     max_current_without_decimal=100;
  //     send_msg.buf[0]= (max_voltage)>>8;
  //   send_msg.buf[1]= (max_voltage)%256;
  //   send_msg.buf[2]= (max_current_without_decimal)>>8;
  //   send_msg.buf[3]= (max_current_without_decimal)%256;

      
  //     can3.write(send_msg);
      
  //     break;

  //   }
  //   else
  //   {  max_voltage=0;
  //     max_current_without_decimal=0;
  //     send_msg.buf[0]= (max_voltage)>>8;
  //   send_msg.buf[1]= (max_voltage)%256;
  //   send_msg.buf[2]= (max_current_without_decimal)>>8;
  //   send_msg.buf[3]= (max_current_without_decimal)%256;
  //   can3.write(send_msg);

  //     Serial.println("0,0,0,0,0,0,0,0,0,");
      
  //   }
  //   if (Serial.available() > 0)
  //   {
  //     input = read_char();
  //   } 
  //   delay(100); 
  // }
  //     Serial.println("1,1,1,1,1,1,1,1,1,1,1,");
  //     Serial.println("1,1,1,1,1,1,1,1,1,1,1,");
  //     Serial.println("1,1,1,1,1,1,1,1,1,1,1,");
  // #endif
  
  
  
  
  // }
  // #endif
  //can3.write(send_msg);
  #ifndef GUI_Enabled
  //if(chargerFlag == false){
    if(can3.write(send_msg) == 1){
      Serial.println("CAN Message sent successfully");
    }
    else{
      Serial.println("Failed to send CAN message");
     //Interrupt();
    }
  //}
  #endif
  #ifdef GUI_Enabled
  
    output_voltage = (receive_msg.buf[0]*256 + receive_msg.buf[1] + 1.5)/10;
    output_current = (receive_msg.buf[2]*256 + receive_msg.buf[3])/10;
    Serial.print(output_voltage);
    Serial.print(",");
    Serial.print(output_current);
    Serial.println(",");
    for (int i = 4; i >= 0; i--) {
            int bit = (receive_msg.buf[4] >> i) & 1;
            Serial.print(bit);
            Serial.print(",");
        }

    Serial.println("");
  #endif

 
  //Serial.println(msg.id);
  #ifndef GUI_Enabled
  if(can3.read(receive_msg)){
    //Serial.print("ID: 0x"); Serial.print(msg.id, HEX );
    //Serial.println(receive_msg.id, HEX);
    //Serial.print("LEN: "); Serial.println(receive_msg.len);
    Serial.println("CAN communication with charger successful");
    output_voltage = (receive_msg.buf[0]*256 + receive_msg.buf[1] + 1.5)/10;
    output_current = (receive_msg.buf[2]*256 + receive_msg.buf[3])/10;
    //Serial.print("Output Voltage [CHARGER] : "); Serial.println(output_voltage);
    //Serial.print("Output Current [CHARGER] : "); Serial.println(output_current);
    //Serial.print("Error Byte :");
    //Serial.println(receive_msg.buf[4]);
    }
    else{
      Serial.println("Received no message from charger");
      chargerFlag = true;
    }
  #endif
}

void transferV( uint16_t * data, uint8_t nic, uint8_t cellReg)
{
 if (loopcount) {
    uint8_t counter = 0;
    for (uint8_t i = 3 * nic; i < 3 * (nic + 1); i++) {
      cells1[nic][counter + 3 * cellReg] = data[i] * 100e-6;
      counter++;
    }
  } else {
    uint8_t counter = 0;
    for (uint8_t i = 3 * nic; i < 3 * (nic + 1); i++) {
      cells2[nic][counter + 3 * cellReg] = data[i] * 100e-6;
      counter++;
    }
  }
}

void circularVoltdef(void){

  for (int c_ic = 0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++) {
    for (int i = 0; i < 18; i++) {
      if( abs( cells1[c_ic][i]-cells2[LTCDEF_CELL_MONITOR_COUNT- c_ic -1][i]) < tolerance){
        cells[c_ic][i]=cells1[c_ic][i];

      }else{

         if(cells1[c_ic][i] > 0.5 && cells2[LTCDEF_CELL_MONITOR_COUNT-c_ic-1][i] > 0.5) //add some range
         {
          cells[c_ic][i]=cells1[c_ic][i];
          if(cells[c_ic][i]>cells2[LTCDEF_CELL_MONITOR_COUNT-c_ic-1][i]){
            cells[c_ic][i]=cells2[LTCDEF_CELL_MONITOR_COUNT-c_ic-1][i];
          }
        }if(cells1[c_ic][i] < 0.5 && cells1[c_ic][i]>0){
          cells[c_ic][i]=cells2[LTCDEF_CELL_MONITOR_COUNT-c_ic-1][i];
        }if(cells2[LTCDEF_CELL_MONITOR_COUNT-c_ic-1][i] < 0.5 && cells2[LTCDEF_CELL_MONITOR_COUNT-c_ic-1][i] >0){
          cells[c_ic][i]=cells1[c_ic][i];
        }

      }
    }
  }
}

void cellsVoltSort(void)
{
  for ( int c_ic = 0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
  {
    #ifndef seventhSlave
      for ( int i=0; i < 18; i++ )
      {
        if ( (i != 5) && (i != 11) && (i != 17) )       // these pins are GNDed in the brd file 
        {
          if ( i < 5 )
          {
            cellVoltages[c_ic][i] = cells[c_ic][i];
          }
          else if ( (i < 11) && (i > 5) )
          {
            cellVoltages[c_ic][i-1] = cells[c_ic][i];
          }
          else 
          {
            cellVoltages[c_ic][i-2] = cells[c_ic][i];
          }
        }
      }
    #else
      if ( c_ic < 6 )
      {
        for ( int i=0; i < 18; i++ )
        {
        if ( (i != 5) && (i != 11) && (i != 17) )       // these pins are GNDed in the brd file 
          {
            if ( i < 5 )
            {
              cellVoltages[c_ic][i] = cells[c_ic][i];
            }
            else if ( (i < 11) && (i > 5) )
            {
              cellVoltages[c_ic][i-1] = cells[c_ic][i];
            }
            else 
            {
              cellVoltages[c_ic][i-2] = cells[c_ic][i];
            }
          }
        }
      }

      else if ( c_ic == 6 )
      {
        for ( uint8_t i=0; i<18; i++ )
        {
          if ( (i==0) || (i==1) || (i==6) || (i==7) || (i==12) || (i==13) )
          {
            if ( i < 2 )
            {
              cellVoltages[c_ic][i] = cells[c_ic][i];
            }
            else if ( (i < 8) && ( i > 5) )
            {
              cellVoltages[c_ic][i-4] = cells[c_ic][i];
            }
            else
            {
              cellVoltages[c_ic][i-8] = cells[c_ic][i];
            }
          }
          // else
          // {
          //   if ( i < 15 )
          //   {
          //     cellVoltages[c_ic][i] = 3.5000;
          //   }
          // }
        }
        for ( uint8_t i=6; i<15; i++ )
        {
          cellVoltages[c_ic][i] = 3.5000;
        }
      }
    #endif
  }
}

void printCells(void)
{
  for (uint8_t c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
  {
    #ifndef seventhSlave
      Serial.println();
      Serial.print("IC : ");
      Serial.print(c_ic+1);

      for (uint8_t i=0; i < 15; i++ )
      {
        Serial.print(" C");
        Serial.print(i+1);
        Serial.print(":");
        Serial.print(cellVoltages[c_ic][i], 4);
        Serial.print(",");
      }
    #else
      if ( c_ic < 6 )
      {
        Serial.println();
        Serial.print("IC : ");
        Serial.print(c_ic+1);

        for (uint8_t i=0; i < 15; i++ )
        {
          Serial.print(" C");
          Serial.print(i+1);
          Serial.print(":");
          Serial.print(cellVoltages[c_ic][i], 4);
          Serial.print(",");
        }
      }

      else if ( c_ic == 6 )
      {
        Serial.println();
        Serial.print("IC : ");
        Serial.print(c_ic+1);

        for (uint8_t i=0; i < 6; i++ )
        {
          Serial.print(" C");
          Serial.print(i+1);
          Serial.print(":");
          Serial.print(cellVoltages[c_ic][i], 4);
          Serial.print(",");
        }
      }
    #endif
  }
}

void cellsLogging(void)
{
  
  if(BPM_ready)
  {
    for (uint8_t c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
    {
      #ifndef seventhSlave
        for (uint8_t i=0; i < 15; i++ )
        {
          CellData += String(cellVoltages[c_ic][i], 4);
          CellData += ",";
        }
      #else
        if ( c_ic < 6 )
        {
          for (uint8_t i=0; i < 15; i++ )
          {
            CellData += String(cellVoltages[c_ic][i], 4);
            CellData += ",";
          }
        }

        else if ( c_ic == 6 )
        {
          for (uint8_t i=0; i < 6; i++ )
          {
            CellData += String(cellVoltages[c_ic][i], 4);
            CellData += ",";
          }
        }
      #endif
    }
  }
  #ifdef LTCDEF_LTC681X_ONLY
  {
    for (uint8_t c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
    {
      #ifndef seventhSlave
        for (uint8_t i=0; i < 15; i++ )
        {
          CellData += String(cellVoltages[c_ic][i], 4);
          CellData += ",";
        }
      #else
        if ( c_ic < 6 )
        {
          for (uint8_t i=0; i < 15; i++ )
          {
            CellData += String(cellVoltages[c_ic][i], 4);
            CellData += ",";
          }
        }

        else if ( c_ic == 6 )
        {
          for (uint8_t i=0; i < 6; i++ )
          {
            CellData += String(cellVoltages[c_ic][i], 4);
            CellData += ",";
          }
        }
      #endif
    }
  }
  #endif
    for (uint8_t c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
    {
      #ifndef seventhSlave
        for (uint8_t i=0; i < 15; i++ )
        {
          
          CellData_GUI += String(cellVoltages[c_ic][i], 4);
          CellData_GUI += ",";
        }
      #else
        if ( c_ic < 6 )
        {
          for (uint8_t i=0; i < 15; i++ )
          {
            CellData_GUI += String(cellVoltages[c_ic][i], 4);
            CellData_GUI += ",";
            
          }
        }

        else if ( c_ic == 6 )
        {
          for (uint8_t i=0; i < 6; i++ )
          {
            
            CellData_GUI += String(cellVoltages[c_ic][i], 4);
            CellData_GUI += ",";
          }
        }
      #endif
    }
     #ifdef GUI_Enabled
     Serial.println(CellData_GUI);
      // Serial.println();
     CellData_GUI = "";
     #endif

  
}

float linear_interpolate(float x0, float y0, float x1, float y1, float x) {
    float y = y0 + (x - x0) * (y1 - y0) / (x1 - x0);
    return y;
}

float InitialiseEnergy(float min_voltage, float thr_voltage) {
    float cap_mapped = 0.0;
    float cap_mapped_thr = 0.0;
    float SOC_VoltApprox = -1;
    float SOC_ThrVoltApprox = -1;

    float cell_capacity_lookup[87] = 
    {
        10.0, 78.54084795, 121.7597832, 171.4230513, 225.0389946, 280.0221137, 
        338.1715796, 387.3510778, 441.212951, 492.8865338, 548.4655855, 
        602.2419923, 656.0061216, 717.2497924, 770.9822287, 824.5884643, 
        879.4615621, 934.3346599, 992.9889292, 1046.572513, 1101.468263, 
        1156.364012, 1209.934652, 1268.689235, 1323.633523, 1375.96966, 
        1427.093946, 1483.288917, 1540.724863, 1598.141393, 1654.412408, 
        1707.971722, 1761.587666, 1813.946454, 1866.350546, 1919.943838, 
        1973.569489, 2025.952547, 2080.820791, 2131.893302, 2184.274742, 
        2237.890685, 2295.284564, 2348.939338, 2403.88039, 2454.744185, 
        2509.961908, 2563.500189, 2618.457421, 2667.135351, 2724.483927, 
        2782.746003, 2829.330942, 2885.525912, 2946.781861, 3008.07664, 
        3061.731414, 3117.887554, 3171.5488, 3230.199833, 3283.796361, 
        3337.392889, 3391.008832, 3443.377329, 3498.318381, 3551.934324, 
        3613.180565, 3666.767385, 3716.474338, 3766.009787, 3813.381701, 
        3852.360288, 3900.213706, 3944.288866, 3988.391207, 4022.84064, 
        4056.234516, 4100.31194, 4142.094775, 4164.440414, 4190.76138, 
        4213.12967, 4222.824484, 4238.782419, 4254.672399, 4260.459841, 
        4260.154046
    };
    float cell_voltage_lookup[87] = {
        
        4.20, 4.142899317, 4.114316039, 4.090057769, 4.076679267, 4.07170303, 
        4.051197819, 4.05428974, 4.064581985, 4.0490339, 4.03982197, 
        4.04188808, 4.042772486, 4.024432319, 4.022266276, 4.007953401, 
        3.992387619, 3.976821837, 3.95562859, 3.939135515, 3.925749933, 
        3.912364352, 3.894625448, 3.883087374, 3.87437365, 3.861002226, 
        3.854178482, 3.842654565, 3.827386083, 3.810248858, 3.806044185, 
        3.787215181, 3.773836678, 3.762645454, 3.755814631, 3.740255928, 
        3.727811797, 3.718956502, 3.702923534, 3.691116474, 3.682105451, 
        3.668726948, 3.649409522, 3.639768506, 3.630743325, 3.623484956, 
        3.616451686, 3.59559821, 3.588130316, 3.592221731, 3.568543904, 
        3.558877405, 3.558619745, 3.547095828, 3.529937367, 3.516516393, 
        3.506875376, 3.491613973, 3.482595871, 3.461091166, 3.44584392, 
        3.430596674, 3.417218171, 3.406961319, 3.397936138, 3.384557635, 
        3.366464802, 3.350283185, 3.330229587, 3.293668758, 3.270606767, 
        3.228092858, 3.202100338, 3.157377913, 3.11527173, 3.067986154, 
        3.017653961, 2.948511876, 2.904892293, 2.838268764, 2.784705202, 
        2.720261874, 2.667880434, 2.602382398, 2.53034376, 2.471442955, 
        2.442010248
    };

    int SOC_lookup_index=0;
    do{
      SOC_lookup_index++;
      SOC_VoltApprox = min_voltage - cell_voltage_lookup[SOC_lookup_index];
    }
    while(SOC_VoltApprox<0 || SOC_lookup_index>86);
  
    cap_mapped = linear_interpolate(cell_voltage_lookup[SOC_lookup_index],cell_capacity_lookup[SOC_lookup_index] , cell_voltage_lookup[SOC_lookup_index-1], cell_capacity_lookup[SOC_lookup_index-1], min_voltage);

    SOC_lookup_index=0;
    do{
      SOC_lookup_index++;
      SOC_ThrVoltApprox = thr_voltage - cell_voltage_lookup[SOC_lookup_index];
    }
    while(SOC_ThrVoltApprox<0 || SOC_lookup_index>86);

    cap_mapped_thr = linear_interpolate(cell_voltage_lookup[SOC_lookup_index],cell_capacity_lookup[SOC_lookup_index] , cell_voltage_lookup[SOC_lookup_index-1], cell_capacity_lookup[SOC_lookup_index-1], thr_voltage);


    // Calculate Energy Available
    float Energy_available = (cap_mapped_thr - cap_mapped) * 5.5 * min_voltage * 1.05 * 90 / 1000000.0;
  return Energy_available;
}
// Function to update Energy Available
float CalculateEnergy(float TS_voltage, float TS_current, float EnergyAvailable, float time_previous, float chargerDelay) {
    
    float Power = abs(TS_current * TS_voltage / 1000.0);
    // If TS_current is positive, return Energy_available and Power as 0
    #ifdef charger_active
    if (TS_current < 1) {
        Power = 0.0;
        return;
    }
    #else
    if (TS_current > -1) {
        Power = 0.0;
        return;
    }
    #endif
    // Calculate Power and make it positive
    // Power in KW
    #ifndef charger_active
      chargerDelay=0;
    #endif
    // Calculate Energy Consumed
    float Energy_consumed = (Power * (time_previous + chargerDelay)) / (1000.0 * 3600.0); // Energy consumed in kWh

    // Update Energy Available
    EnergyAvailable += Energy_consumed;
    return EnergyAvailable;

}


void transferT( uint16_t * data, uint8_t nic, bool muxSelect , uint8_t auxReg)
{
   if (loopcount)
  {
      uint8_t counter = 0;
    if (muxSelect)
    {
      for (uint8_t i = 3 * nic; i < 3 * (nic + 1); i++) 
      {
        auxHigh1[nic][counter + 3 * auxReg] = data[i] * 100e-6;
        counter++;
      }
   } 
   else 
   {
      for (uint8_t i = 3 * nic; i < 3 * (nic + 1); i++) 
      {
        auxLow1[nic][counter + 3 * auxReg] = data[i] * 100e-6;
        counter++;
      }
    }
  }
  else
  {
      uint8_t counter = 0;
    if (muxSelect) 
    {
      for (uint8_t i = 3 * nic; i < 3 * (nic + 1); i++) 
      {
        auxHigh2[nic][counter + 3 * auxReg] = data[i] * 100e-6;
        counter++;
      }
    } 
    else 
    {
      for (uint8_t i = 3 * nic; i < 3 * (nic + 1); i++) 
      {
        auxLow2[nic][counter + 3 * auxReg] = data[i] * 100e-6;
        counter++;
      }
    }
}
}

void circularTempdef(bool muxSelect){
  if(loopcount)
  {
    double auxRefH, auxRefL;
    for (uint8_t c_ic = 0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++) 
    {
      if (muxSelect) 
      {
        auxRefH = auxHigh1[c_ic][5];
      } 
      else 
      {
        auxRefL = auxLow1[c_ic][5];
      }
      for (uint8_t i = 0; i < 12; i++){
        if (muxSelect) {
          auxHigh1[c_ic][i] = voltToTemp(auxHigh1[c_ic][i], auxRefH);
        } else {
          auxLow1[c_ic][i] = voltToTemp(auxLow1[c_ic][i], auxRefL);
        }
      }
    }
  } 
  else
  {
      double auxRefH, auxRefL;
    for (uint8_t c_ic = 0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++) {
      if (muxSelect) {
        auxRefH = auxHigh2[c_ic][5];
      } else {
        auxRefL = auxLow2[c_ic][5];
      }
      for (uint8_t i = 0; i < 12; i++) 
      {
        if (muxSelect) 
        {
          auxHigh2[c_ic][i] = voltToTemp(auxHigh2[c_ic][i], auxRefH);
        } 
        else 
        {
          auxLow2[c_ic][i] = voltToTemp(auxLow2[c_ic][i], auxRefL);
        }
      }
    }
  }
  for (uint8_t c_ic = 0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++) {
    for (uint8_t i = 0; i < 12; i++) {
        if (muxSelect) {
          if( abs(auxHigh1[c_ic][i]-auxHigh2[LTCDEF_CELL_MONITOR_COUNT- c_ic -1][i]) < tolerance){
            auxHigh[c_ic][i]=auxHigh1[c_ic][i];
          }else{
            auxHigh[c_ic][i]=auxHigh1[c_ic][i];
          if(auxHigh[c_ic][i]<auxHigh2[LTCDEF_CELL_MONITOR_COUNT-c_ic-1][i]){
            auxHigh[c_ic][i]=auxHigh2[LTCDEF_CELL_MONITOR_COUNT-c_ic-1][i];
            }
          } 
        } else 
        {
          if( abs(auxLow1[c_ic][i]-auxLow2[LTCDEF_CELL_MONITOR_COUNT- c_ic -1][i]) < tolerance){
            auxLow[c_ic][i]=auxLow1[c_ic][i];
          }
          else
          {
            auxLow[c_ic][i]=auxLow1[c_ic][i];
          if(auxLow[c_ic][i]<auxLow2[LTCDEF_CELL_MONITOR_COUNT-c_ic-1][i])
          {
            auxLow[c_ic][i]=auxLow2[LTCDEF_CELL_MONITOR_COUNT-c_ic-1][i];
          }
        }
      }
    }
  }
}

void tempConvertSort(bool muxSelect)
{
  #ifndef seventhSlave
    if (muxSelect)
    {
      for ( uint8_t c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
      {
        for ( uint8_t i=0; i < 8; i++ )
        {
          if ( i < 5 )
          {
            cellTemperatures[c_ic][i+8] = auxHigh[c_ic][i];
          }
          else if ( i > 5 )
          {
            cellTemperatures[c_ic][i+7] = auxHigh[c_ic][i];
          }
        }
      }
    }
    else
    {
      for (uint8_t c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
      {
        for ( uint8_t i=0; i < 9; i++ )
        {
          if ( i < 5)
          {
            cellTemperatures[c_ic][i] = auxLow[c_ic][i];
          }
          else if ( i > 5 )
          {
            cellTemperatures[c_ic][i-1] = auxLow[c_ic][i];
          }
        }
      }
    }
  #else
    if (muxSelect)
    {
      for ( uint8_t c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
      {
        if ( c_ic < 6 )
        {
          for ( uint8_t i=0; i < 8; i++ )
          {
            if ( i < 5 )
            {
              cellTemperatures[c_ic][i+8] = auxHigh[c_ic][i];
            }
            else if ( i > 5 )
            {
              cellTemperatures[c_ic][i+7] = auxHigh[c_ic][i];
            }
          }
        }

        else if ( c_ic == 6 )
        {
          for ( uint8_t i=0; i < 8; i++ )
          {
            if ( i < 5 )
            {
              cellTemperatures[c_ic][i+8] = 30.000;
            }
            else if ( i > 5 )
            {
              cellTemperatures[c_ic][i+7] = 30.000;
            }
          }
        }
      }
    }
    else
    {
      for (uint8_t c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
      {
        if ( c_ic < 6 )
        {
          for ( uint8_t i=0; i < 9; i++ )
          {
            if ( i < 5)
            {
              cellTemperatures[c_ic][i] = auxLow[c_ic][i];
            }
            else if ( i > 5 )
            {
              cellTemperatures[c_ic][i-1] = auxLow[c_ic][i];
            }
          }
        }

        else if ( c_ic == 6 )
        {
          for ( uint8_t i=0; i < 9; i++ )
          {
            if ( i < 5)
            {
              cellTemperatures[c_ic][i] = auxLow[c_ic][i];
            }
            else if ( i == 6 )
            {
              cellTemperatures[c_ic][i-1] = auxLow[c_ic][i];
            }
            else if ( i > 6 )
            {
              cellTemperatures[c_ic][i-1] = 30.000;  // dummy values to normalize error checks
            }
          }
        }
      }
    }
  #endif
}

void printAux(void)
{
  for (uint8_t c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
  {
    #ifndef seventhSlave
      Serial.println();
      Serial.print("IC : ");
      Serial.print(c_ic+1);

      for (uint8_t i=0; i < 15; i++ )
      {
        Serial.print(" T");
        Serial.print(i+1);
        Serial.print(":");
        Serial.print(cellTemperatures[c_ic][i], 3);
        Serial.print(",");
      }
    #else
      if ( c_ic < 6 )
      {
        Serial.println();
        Serial.print("IC : ");
        Serial.print(c_ic+1);

        for (uint8_t i=0; i < 15; i++ )
        {
          Serial.print(" T");
          Serial.print(i+1);
          Serial.print(":");
          Serial.print(cellTemperatures[c_ic][i], 3);
          Serial.print(",");
        } 
      }

      else if ( c_ic == 6 )
      {
        Serial.println();
        Serial.print("IC : ");
        Serial.print(c_ic+1);

        for (uint8_t i=0; i < 6; i++ )
        {
          Serial.print(" T");
          Serial.print(i+1);
          Serial.print(":");
          Serial.print(cellTemperatures[c_ic][i], 3);
          Serial.print(",");
        }
      }
    #endif  
  }
}

void auxLogging(void)
{
  if(BPM_ready)
  {
    for (uint8_t c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
    {
      #ifndef seventhSlave
        for (uint8_t i=0; i < 15; i++ )
        {
          CellData += String(cellTemperatures[c_ic][i], 4);
          CellData += ",";
        }
      #else
        if ( c_ic < 6 )
        {
          for (uint8_t i=0; i < 15; i++ )
          {
            CellData += String(cellTemperatures[c_ic][i], 4);
            CellData += ",";
          }
        }

        else if ( c_ic == 6 )
        {
          for (uint8_t i=0; i < 6; i++ )
          {
            CellData += String(cellTemperatures[c_ic][i], 4);
            CellData += ",";
          }
        }
      #endif
    }
  }
  #ifdef LTCDEF_LTC681X_ONLY
  for (uint8_t c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
    {
      #ifndef seventhSlave
        for (uint8_t i=0; i < 15; i++ )
        {
          CellData += String(cellTemperatures[c_ic][i], 4);
          CellData += ",";
        }
      #else
        if ( c_ic < 6 )
        {
          for (uint8_t i=0; i < 15; i++ )
          {
            CellData += String(cellTemperatures[c_ic][i], 4);
            CellData += ",";
          }
        }

        else if ( c_ic == 6 )
        {
          for (uint8_t i=0; i < 6; i++ )
          {
            CellData += String(cellTemperatures[c_ic][i], 4);
            CellData += ",";
          }
        }
      #endif
    }
    #endif

  
  
    for (uint8_t c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
    {
      #ifndef seventhSlave
        for (uint8_t i=0; i < 15; i++ )
        {
          
          CellData_GUI += String(cellTemperatures[c_ic][i], 4);
          CellData_GUI += ",";
        }
      #else
        if ( c_ic < 6 )
        {
          for (uint8_t i=0; i < 15; i++ )
          {
            
            CellData_GUI += String(cellTemperatures[c_ic][i], 4);
            CellData_GUI += ",";
            
          }
        }

        else if ( c_ic == 6 )
        {
          for (uint8_t i=0; i < 6; i++ )
          {
            
            CellData_GUI += String(cellTemperatures[c_ic][i], 4);
            CellData_GUI += ",";
          }
        }
      #endif
    }
    #ifdef GUI_Enabled
    CellData_GUI += "0,0,0,";
    Serial.println(CellData_GUI);
    CellData_GUI="";
    #endif
  
}


double voltToTemp( double auxVal, double auxRef)
{
  if ( auxVal != auxRef )
  {
  double R = 10000.0;
  double Rt;
  double B = 3420.50726961, C = B/293.15;   // coefficients are calculated using datasheet 103JT thermistor
  
  Rt = ( R * auxVal )/( auxRef - auxVal );
  Rt =  Rt * 0.001 ;
  
  if( Rt > 0 )
  {
    double logR = log(Rt/12.11),T;
    T = B/(logR + C);
    T = T - 273.15;
    return T;
  }
  else
  {
    return 0.0;
  }

  }
  else
  {
    return 0.0;
  }
}

void checkError (void)
{
  checkVoltageFlag();
  checkTempFlag();
}

void checkVoltageFlag(void)
{
  for ( uint8_t c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
  {
    for ( uint8_t i=0; i < 15; i++ )
    {
      if ( cellVoltages[c_ic][i] < 0.5 )
      {
        errorFlag[0] = -1;
        voltageErrorLoc[c_ic][i] = -1;
      }

      if ( cellVoltages[c_ic][i] > 6.0 )
      {
        errorFlag[1] = -1;
        voltageErrorLoc[c_ic][i] = -1;
      }

      if ( (cellVoltages[c_ic][i] < underVoltageThreshold) && (cellVoltages[c_ic][i] > 0.5 ) )
      {
        errorFlag[2] = -1;
        voltageErrorLoc[c_ic][i] = -1;
      }

      if ( (cellVoltages[c_ic][i] > overVoltageThreshold) && (cellVoltages[c_ic][i] < 6.0 ) )
      {
        errorFlag[3] = -1;
        voltageErrorLoc[c_ic][i] = -1; 
      }
    }
  }
}

void checkTempFlag(void)
{
  for ( uint8_t c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
  {
    for ( uint8_t i=0; i < 15; i++ )
    {
      if ( cellTemperatures[c_ic][i] < 0.0 )
      {
        errorFlag[4] = -1;
        tempErrorLoc[c_ic][i] = -1;
      }

      if ( cellTemperatures[c_ic][i] == 0.0 )
      {
        errorFlag[5] = -1;
        tempErrorLoc[c_ic][i] = -1;
      }

      if ( (cellTemperatures[c_ic][i] > overTempThreshold) )
      {
        errorFlag[6] = -1;
        tempErrorLoc[c_ic][i] = -1; 
      }
    }
  }
}

void triggerchargerError(void)
{
  output_voltage = (receive_msg.buf[0]*256 + receive_msg.buf[1])/10;
  output_current = (receive_msg.buf[2]*256 + receive_msg.buf[3])/10;
 
  if(output_voltage>250 && chargingStarted_)
  {
    chargingStarted_=false;
    chargingFlag=true;
    delay(chgrDelay);
    if(can3.read(receive_msg)){
    //Serial.print("ID: 0x"); Serial.print(msg.id, HEX );
    Serial.println(receive_msg.id, HEX);
    Serial.print("LEN: "); Serial.println(receive_msg.len);
    output_voltage = (receive_msg.buf[0]*256 + receive_msg.buf[1] + 1.5)/10;
    output_current = (receive_msg.buf[2]*256 + receive_msg.buf[3])/10;
    Serial.println("Attempting to start charging");
    Serial.print("Output Voltage [CHARGER] : "); Serial.println(output_voltage);
    Serial.print("Output Current [CHARGER] : "); Serial.println(output_current);
    Serial.print("Error Byte :");
    Serial.println(receive_msg.buf[4]);}
    else{
      Serial.println("Received no message from charger(2)");
      chargerFlag = true;
    }
  }
  if(output_voltage < 250 && (receive_msg.buf[4] == 8 || receive_msg.buf[4] == 24) && output_current < 1)
  {
   chargerFlag=false;
   #ifndef GUI_Enabled
   Serial.println("Connect battery");
   printchargerError();
   #endif
  }
  else if(receive_msg.buf[4] != 0){
    chargerFlag=true;
    #ifndef GUI_Enabled
    Serial.println("Received some error from charger check error byte");
    printchargerError();
    #endif
  }
  else if (output_voltage > 250 && receive_msg.buf[4] == 0 && output_current < 0.1)
  {
    chargerFlag= true;
    // if(chargingFlag==true)
    // {
    // // send_msg.flags.extended = 1;
    // //  receive_msg.flags.extended = 1;
    // //  send_msg.id=send_id;
    // //  max_voltage = 0;
    // // send_msg.buf[0]= (max_voltage)>>8;
    // // send_msg.buf[1]= (max_voltage)%256;
    // // send_msg.buf[2]= (max_current_without_decimal)>>8;
    // // send_msg.buf[3]= (max_current_without_decimal)%256;
    // // can3.write(send_msg);
    // }
    #ifndef GUI_Enabled
    Serial.println("Charger is ready | Voltage detected by charger | No current detected");
    printchargerError();
    #endif
  }
  else if (output_voltage > 250 && receive_msg.buf[4] == 0 && output_current > 1)
  {
    chargerFlag=false;
    #ifndef GUI_Enabled
    Serial.println("Charging ... Charging ... Charging ...");
    #endif
  }
  else if (output_voltage > 250 && receive_msg.buf[4] != 0)
  {
    chargerFlag=true;
    #ifndef GUI_Enabled
    Serial.println("Charging stopped due to charger error");
    printchargerError();
    #endif
  }
  else {
    chargerFlag=true;
    #ifndef GUI_Enabled
    Serial.println("Entered confusing state");
    printchargerError();
    #endif
  }


}

void triggerError(void)
{
  for ( uint8_t i=0; i<7; i++ )
  {
    if ( errorFlag[i] == -1 )
    { 
      errorCheckingMode();
      break;
    }
  }
}

void triggerError_GUI(void)
{
  for ( uint8_t i=0; i<7; i++ )
  {
    if ( errorFlag[i] == -1 )
    { if(enteredChecking==false){
      errorCheckingMode();
      break;}
      if ( errorFlag[0] == -1 || errorFlag[1] == -1 || errorFlag[2] == -1 || errorFlag[3] == -1 )
      {
            voltFlag = true;
            
      }

      if ( errorFlag[4] == -1 || errorFlag[5] == -1 || errorFlag[6] == -1 )
      {
           tempFlag = true;
      }
    }
  }
}


void errorCheckingMode(void)
{ 

  enteredChecking=true;
  if ( errorFlag[0] == -1 || errorFlag[1] == -1 || errorFlag[2] == -1 || errorFlag[3] == -1 )
  {
    voltagePlausibilityCheck();
  }

  if ( errorFlag[4] == -1 || errorFlag[5] == -1 || errorFlag[6] == -1 )
  {
    tempPlausibilityCheck();
  }
}

void voltagePlausibilityCheck(void)
{
  double ti = millis(),delT = 0.0;
  while( delT < voltTimer )
  {
    clearFlagsV();
    cellVoltageLoop(0);
    checkVoltageFlag();

    if ( errorFlag[0] == -1 || errorFlag[1] == -1 || errorFlag[2] == -1 || errorFlag[3] == -1 )
    {
      delT = millis() - ti ;
      voltFlag = true;
    }
    else
    { 
      voltFlag = false;
      break;
    }
  }
}

void tempPlausibilityCheck(void)
{
  double ti = millis(),delT = 0.0;
  while( delT < tempTimer )
  {
    clearFlagsT();
    cellTempLoop(0);
    checkTempFlag();

    if ( errorFlag[4] == -1 || errorFlag[5] == -1 || errorFlag[6] == -1 )
    {
      delT = millis() - ti ;
      tempFlag = true;
    }
    else
    {
      tempFlag = false;
      break;
    }
  }
}

void triggerInterrupt(void)
{
  bmsFlag = voltFlag || tempFlag || chargerFlag;
  
  if ( bmsFlag || chargerFlag )
  { 
    if ( voltFlag && tempFlag && chargerFlag )
    {
      Serial.print('\n');
      Serial.print(" Interrupted due to voltage,temperature and charger flag ");
      printErrLocV();
      printErrLocT();
      //printchargerError();
    }

    else if ( voltFlag && tempFlag )
    {
      Serial.print('\n');
      Serial.print(" Interrupted due to voltage and temperature flag ");
      printErrLocT();
      printErrLocV();
      
    }

    else if ( voltFlag && chargerFlag )
    {
      Serial.print('\n');
      Serial.print(" Interrupted due to voltage and charger flag ");
      printErrLocV();
      //printchargerError();
    }

    else if ( chargerFlag && tempFlag )
    {
      Serial.print('\n');
      Serial.print(" Interrupted due to temperature and charger flag ");
      printErrLocT();
      //printchargerError();
    }

    else if (  tempFlag )
    {
      Serial.print('\n');
      Serial.print(" Interrupted due to temperature flag only ");
      printErrLocT();
      
    }

    else if ( chargerFlag  )
    {
      Serial.print('\n');
      Serial.print(" Interrupted due to charger flag only ");
      
      printchargerError();
    }

    else if ( voltFlag  )
    {
      Serial.print('\n');
      Serial.print(" Interrupted due to voltage flag only ");
      
      printErrLocV();
    }

    #ifdef Interrupt_Debug
    Interrupt_for_Debug();
    #endif

    #ifndef Interrupt_Debug
    Interrupt();
    #endif
  }
}

void triggerInterrupt_GUI(void)
{ 
  bmsFlag = voltFlag || tempFlag;
  if(voltFlag && tempFlag)
  {
    Serial.println("0,0,1,0,");
  }
  else if(voltFlag)
  {
    Serial.println("1,0,0,0,");
  }
  else if(tempFlag)
  {
    Serial.println("0,1,0,0,");
  }
  else
  {
    Serial.println("0,0,0,0,");
  }

  if(bmsFlag || chargerFlag)
  {

    #ifdef Interrupt_Debug
    Interrupt_for_Debug();
    #endif

    #ifndef Interrupt_Debug
    Interrupt();
    #endif  
  }
}


void showError(void)
{
  if ( voltFlag && tempFlag )
    {
      Serial.print('\n');
      Serial.print(" Interrupted due to voltage and temperature flag ");
      printErrLocT();
      printErrLocV();
      
    }

  
  else if (  tempFlag )
    {
      Serial.print('\n');
      Serial.print(" Interrupted due to temperature flag only ");
      printErrLocT();
      
    }

  else if ( voltFlag  )
    {
      Serial.print('\n');
      Serial.print(" Interrupted due to voltage flag only ");
      
      printErrLocV();
    }  
  
}

void Interrupt_for_Debug(void)
{
  bool error_received = false;
  char input = 0;
  while (input != 'r' )
  {
    digitalWriteFast( BMS_FLT_3V3 , LOW );               
    switchErrorLed();
    if (input == 's' && (error_received==false))
    {
      printCells();
      printAux();
      showError();
      error_received =true;
    }
    if (Serial.available() > 0)
    {
      input = read_char();
    } 
    delay(100); 
  }
}

void Interrupt(void)
{
  digitalWriteFast( BMS_FLT_3V3 , LOW );               
  switchErrorLed();
}

void switchErrorLed(void)
{
  for ( uint8_t flags = 0; flags < 3; flags++ )
  {
    switch(flags)
    {
      case 0:
        if( voltFlag )
        {
          digitalWriteFast(VOLT_ERR_PIN,HIGH);
        }
        else
        {
          digitalWriteFast(VOLT_ERR_PIN,LOW);
        }
        break;
      case 1:
        if ( tempFlag )
        {
          digitalWriteFast(TEMP_ERR_PIN,HIGH);
        }
        else
        {
          digitalWriteFast(TEMP_ERR_PIN,LOW);
        }
        break;
      case 2:
        if ( voltFlag && tempFlag )
        {
          digitalWriteFast(VOLT_ERR_PIN,HIGH);
          digitalWriteFast(TEMP_ERR_PIN,HIGH);
        }
        break;
    }
  }
}

void pull_3V3_high(void)
{
  if(bmsFlag==false)
  {
  digitalWriteFast(BMS_FLT_3V3,HIGH);
  }
}

void clearFlags(void)
{
  bmsFlag = false;
  voltFlag = false;
  tempFlag = false;
  //chargerFlag = false;

  //digitalWriteFast(BMS_FLT_3V3,HIGH);
  
  for ( uint8_t i=0; i < 7; i++ )
  {
    errorFlag[i] = 0;
  }

  for ( uint8_t c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
  {
    for ( uint8_t i=0; i < 15; i++ )
    {
      voltageErrorLoc[c_ic][i] = 0;
      tempErrorLoc[c_ic][i] = 0;
    }
  }
}

void clearFlagsV(void)
{
  for ( uint8_t i=0; i < 4; i++ )
  {
    errorFlag[i] = 0;
  }

  for ( uint8_t c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
  {
    for ( uint8_t i=0; i < 15; i++ )
    {
      voltageErrorLoc[c_ic][i] = 0;
    }
  }
}

void clearFlagsT(void)
{
  for ( uint8_t i=4; i < 7; i++ )
  {
    errorFlag[i] = 0;
  }

  for ( uint8_t c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
  {
    for ( uint8_t i=0; i < 15; i++ )
    {
      tempErrorLoc[c_ic][i] = 0;
    }
  }
}

void printErrLocV(void)
{
  for (uint8_t c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
  {
    Serial.println();
    Serial.print("IC : ");
    Serial.print(c_ic+1);

    for (uint8_t i=0; i < 15; i++ )
    {
      Serial.print(" C");
      Serial.print(i+1);
      Serial.print(":");
      Serial.print(voltageErrorLoc[c_ic][i]);
      Serial.print(",");
    }
  }
}

void printErrLocT(void)
{
  for (uint8_t c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
  {
    Serial.println();
    Serial.print("IC : ");
    Serial.print(c_ic+1);

    for (uint8_t i=0; i < 15; i++ )
    {
      Serial.print(" T");
      Serial.print(i+1);
      Serial.print(":");
      Serial.print(tempErrorLoc[c_ic][i]);
      Serial.print(",");
    }
  }
}

void printchargerError(void)
{
  Serial.println("");
  Serial.print("Charger Error Byte:");
  Serial.println(receive_msg.buf[4]);
      if (receive_msg.buf[4] == 1)
      Serial.println("Status: Hardware Failure"); 
      if (receive_msg.buf[4] == 2)
      Serial.println("Status: Charger overheated"); 
      if (receive_msg.buf[4] == 4)
      Serial.println("Status: Incorrect Input Voltage"); 
      if (receive_msg.buf[4] == 8)
      Serial.println("Status: Battery Disconnected"); 
      if (receive_msg.buf[4] == 16)
      Serial.println("Status: Communication failed");
      if (receive_msg.buf[4] == 12)
      Serial.println("Status: Battery disconnected and incorrect input voltage");
      if (receive_msg.buf[4] == 24)
      Serial.println("Status: Battery disconnected and communication failed");
}

void initialiseNormalizeChannel(void)
{
  for (uint8_t c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
  {
    for (uint8_t i=0; i < 15; i++ )
    {
      voltageNormalize[c_ic][i] = 0;
      tempNormalize[c_ic][i] = 0;
    }
  }
}

void setNormalizeChannels(uint8_t ic , uint16_t * vChannels, uint16_t * tChannels )
{
  for (int c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
  {
    if ( (ic >> c_ic) & 1 ) 
    {
      setNormalizeFlag(c_ic, vChannels[c_ic], tChannels[c_ic]);
    }
  }  
}

void setNormalizeFlag( uint8_t nic, uint16_t voltChannels, uint16_t tempChannels )
{
  for ( int i=0; i < 15; i++ )
  {
    if( (voltChannels >> i) & 1 )
    {
      voltageNormalize[nic][i] = -1;
    }
  }

  for ( int i=0; i < 15; i++ )
  {
    if( (tempChannels >> i) & 1 )
    {
      tempNormalize[nic][i] = -1;
    }
  }
}

void checkVoltNormalize(void)
{
  for (int c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
  {
    for ( int i=0; i<15; i++ )
    {
      if ( voltageNormalize[c_ic][i] == -1 )
      {
        cellVoltages[c_ic][i] = 3.5000;
      }
    }
  }
}

void checkTempNormalize(void)
{
  for (int c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
  {
    for ( int i=0; i<15; i++ )
    {
      if ( tempNormalize[c_ic][i] == -1 )
      {
        cellTemperatures[c_ic][i] = 30.000;
      }
    }
  }
}

void initialiseSplitChannel(void)
{
  for ( uint8_t c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
  {
    for ( uint8_t i=0; i<15; i++ )
    {
      voltageSplit[c_ic][i] = 0;
    }
  }
}

void setVoltSplitChannels(uint8_t ic, uint16_t * channels )
{
  for ( int c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
  {
    if ( (ic >> c_ic) & 1 )
    {
      setVoltSplitFlag(c_ic, channels[c_ic]);
    }
  }
}

void setVoltSplitFlag( uint8_t nic, uint16_t channels )
{
  for ( int i=0; i < 15; i++ )
  {
    if( (channels >> i) & 1 )              
    {
      voltageSplit[nic][i] = -1;
    }
  }
}

void checkVoltSplit(void)
{
  for ( uint8_t c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
  {
    for ( uint8_t i=0; i<15; i++ )
    {
      if( voltageSplit[c_ic][i] == -1 )      // set split flag for right channel among the two channels of a split
      {
        float splitVal = avg(cellVoltages[c_ic][i-1],cellVoltages[c_ic][i]);
        cellVoltages[c_ic][i-1] = splitVal;
        cellVoltages[c_ic][i] = splitVal;
      }
    }
  }
}

float avg( float a, float b)
{
  return (a+b)/2;
}

void checkACUsignalStatus(void)
{
  if( amsMasterToACUsignals[0] )
  {
    digitalWriteFast(FAN_MBED, HIGH);
  }
  if( amsMasterToACUsignals[1] )
  {
    digitalWriteFast(CH_EN_MBED, HIGH);
  }
}

void performDynamicCooling(void)
{

if (flag_fan == 0) {
    if (maxTemp.val <= 35)
        digitalWriteFast(FAN_MBED, LOW);
    else {
        digitalWriteFast(FAN_MBED, HIGH);
        flag_fan = 1;
    }
}
else {
    if (maxTemp.val > 33)
        digitalWriteFast(FAN_MBED, HIGH);
    else {
        digitalWriteFast(FAN_MBED, LOW);
        flag_fan = 0;
    }
}

}

void findMax(double array[LTCDEF_CELL_MONITOR_COUNT][15], uint8_t type)
{
  double max = array[0][0];
  uint8_t sLoc=0, cLoc=0;
   if ( type == voltage ){
  for ( uint8_t c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
  {
    for ( uint8_t i=0; i<cellsPerStack[c_ic]; i++ )
    {
      if(voltageNormalize[c_ic][i]!=-1){
      if ( array[c_ic][i] > max )
      {
        max = array[c_ic][i];
        sLoc = c_ic;
        cLoc = i;
      }
    }
  }}

  
    maxVoltage.val = max;
    maxVoltage.slaveLoc = sLoc+1;
    maxVoltage.cellLoc = cLoc+1;
  }
  if ( type == temp )
  {for ( uint8_t c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
  {
    for ( uint8_t i=0; i<cellsPerStack[c_ic]; i++ )
    {
      if(tempNormalize[c_ic][i]!=-1){
      if ( array[c_ic][i] > max )
      {
        max = array[c_ic][i];
        sLoc = c_ic;
        cLoc = i;
      }
    }
  }}

    maxTemp.val = max;
    maxTemp.slaveLoc = sLoc+1;
    maxTemp.cellLoc = cLoc+1;
  }
}

void findMin(double array[LTCDEF_CELL_MONITOR_COUNT][15])
{
  double min = array[0][0];
  uint8_t sLoc=0, cLoc=0;
  for ( uint8_t c_ic=0; c_ic < LTCDEF_CELL_MONITOR_COUNT; c_ic++ )
  {
    for ( uint8_t i=0; i<cellsPerStack[c_ic]; i++ )
    {if(voltageNormalize[c_ic][i]!=-1){
      if ( array[c_ic][i] < min )
      {
        min = array[c_ic][i];
        sLoc = c_ic;
        cLoc = i;
      }
    }}
  }

  minVoltage.val = min;
  minVoltage.slaveLoc = sLoc+1;
  minVoltage.cellLoc = cLoc+1;

}

void printMaxMinParameters(void)
{
  Serial.println();

  Serial.print(" Max Voltage : ");
  Serial.print(maxVoltage.val , 4);
  Serial.print("  Location --> ");
  Serial.print("S");
  Serial.print(maxVoltage.slaveLoc);
  Serial.print("C");
  Serial.print(maxVoltage.cellLoc);
  
  Serial.println();
  
  Serial.print(" Max Temperature : ");
  Serial.print(maxTemp.val , 4);
  Serial.print("  Location --> ");
  Serial.print("S");
  Serial.print(maxTemp.slaveLoc);
  Serial.print("C");
  Serial.print(maxTemp.cellLoc);

  Serial.println();
  
  Serial.print(" Min Voltage : ");
  Serial.print(minVoltage.val , 4);
  Serial.print("  Location --> ");
  Serial.print("S");
  Serial.print(minVoltage.slaveLoc);
  Serial.print("C");
  Serial.print(minVoltage.cellLoc);
}


void SDcardLogging(void)
{
  dataFile.println(CellData);
  // Serial.println();
  // Serial.println(CellData);
  // Serial.println();
  dataFile.flush();

  CellData = "";
}

void initialiseSDcard(void)
{
  #ifdef initialiseEEPROM
  EEPROM.write(0,0);
  #endif
  
  int eeprom_value = EEPROM.read(0);
  eeprom_value++;
  Serial.println(eeprom_value);
  EEPROM.write(0,eeprom_value);
  #ifndef GUI_Enabled
  
   Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) 
  {
     Serial.println("Card failed, or not present");
    // don't do anything more:
    // while (1) ;
  }
  // Serial.println("card initialized.");
  #endif


  String fileName = "AMSCellData" + String(eeprom_value) + ".txt";
  // Open up the file we're going to log to!
  dataFile = SD.open(fileName.c_str(), FILE_WRITE);
  #ifndef GUI_Enabled
  if (! dataFile) 
  {
    Serial.println("error opening file");
    // Wait forever since we cant write data
    // while (1) ;
  }
  #endif
}
void initCAN(void) {
    Can2.begin();
    Can2.setBaudRate(250000);  // Set baud rate to 500kbps
}

void initialiseCAN(void)
{
  can3.begin();
  can3.setBaudRate(250000);
//  can.enableFIFO(); //enabling FIFO
//  can.enableFIFOInterrupt(); //all FIFO interrupt enabled
//  can.onReceive(read_update);
}

void initialiseFlags(void)
{
  bmsFlag = false;
  voltFlag = false;
  tempFlag = false;
  chargerFlag = false;
}


int8_t read_char()
{
  read_data();
  return(ui_buffer[0]);
}

uint8_t read_data()
{
  uint8_t index = 0; //index to hold current location in ui_buffer
  int c; // single character used to store incoming keystrokes
  while (index < UI_BUFFER_SIZE-1)
  {
    c = Serial.read(); //read one character
    if (((char) c == '\r') || ((char) c == '\n')) break; // if carriage return or linefeed, stop and return data
    if ( ((char) c == '\x7F') || ((char) c == '\x08') )   // remove previous character (decrement index) if Backspace/Delete key pressed      index--;
    {
      if (index > 0) index--;
    }
    else if (c >= 0)
    {
      ui_buffer[index++]=(char) c; // put character into ui_buffer
    }
  }
  ui_buffer[index]='\0';  // terminate string with NULL

  if ((char) c == '\r')    // if the last character was a carriage return, also clear linefeed if it is next characterde
  {
    delay(10);  // allow 10ms for linefeed to appear on serial pins
    if (Serial.peek() == '\n') Serial.read(); // if linefeed appears, read it and throw it away
  }

  return index; // return number of characters, not including null terminator
}

byte ReadPrintCellVoltages(uint16_t rdcv, uint16_t * cellMonDat)
{
	// in case LTC2949 is parallel to the daisychain, we now read only the cell voltages
	byte error = LTC2949_68XX_RdCells(rdcv, cellMonDat);
	PrintCellVoltages(cellMonDat, true);
	return error;
}

byte ReadPrintAuxVoltages(uint16_t rdaux, uint16_t * cellMonDat)
{
	// in case LTC2949 is parallel to the daisychain, we now read only the cell voltages
	byte error = LTC2949_68XX_RdAux(rdaux, cellMonDat);
	PrintAuxVoltages(cellMonDat, true);
	return error;
}

#ifndef LTCDEF_LTC681X_ONLY
/*!*********************************************************************
\brief Checks if the device as awake and in slow and fast continuous mode
This will restart the whole measurement e.g. in case the device was
powered off or there is a severe communication issue.
This can be demonstrated by just powering off the device or unplugging
the isoSPI interface while measurement is running.
***********************************************************************/
byte ChkDeviceStatCfg()
{
	byte error;
	byte data[10];
	byte dataOthers;
	boolean expChkFailed;

	// check STATUS (EXT)FAULTS, ALERT registers
	error = LTC2949_ReadChkStatusFaults(
		/*boolean lockMemAndClr:    */ false,
		/*boolean printResult:      */ false,
		/*byte len:                 */ 10,
		/*byte * statFaultsExpAndRd:*/ data,
		/*boolean * expChkFailed:   */ &expChkFailed,
		/*byte expDefaultSet):      */ LTC2949_STATFAULTSCHK_IGNORE_STATUPD | LTC2949_STATFAULTSCHK_DFLT_AFTER_CLR);

	if (err_detected(error))
		return error;

	if (expChkFailed)
	{
		error = LTC2949_ERRCODE_OTHER; // STATUS (EXT)FAULTS, ALERT check failed
		SerialPrintByteArrayHex(data, 10, true); // report the status
		PrintComma();
		return LTC2949_ERRCODE_OTHER;
	}

	// check BRCEN bit
	if (err_detected(error = LTC2949_READ(LTC2949_REG_REGSCTRL, 1, &dataOthers)))
		return error; // PEC error
	if (bitMaskSetClrChk(dataOthers, LTC2949_BM_REGSCTRL_BCREN, !LTC2949_onTopOfDaisychain))
		return LTC2949_ERRCODE_OTHER; // BRCEN != LTC2949_onTopOfDaisychain

	if (err_detected(error = LTC2949_READ(LTC2949_REG_OPCTRL, 1, &dataOthers)))
		return error; // PEC error
	if (dataOthers != LTC2949_BM_OPCTRL_CONT)
		return LTC2949_ERRCODE_OTHER; // not in continuous mode

	if (err_detected(error = LTC2949_READ(LTC2949_REG_FACTRL, 1, &dataOthers)))
		return error; // PEC error
	if (dataOthers != LTCDEF_FACTRL_CONFIG)
		return LTC2949_ERRCODE_OTHER;  // not or wrong fast mode

	if (err_detected(error = LTC2949_ADCConfigRead(&dataOthers)))
		return error; // PEC error
	if (dataOthers != LTCDEF_ADCCFG_CONFIG)
		return LTC2949_ERRCODE_OTHER; // wrong ADC configuration

	return 0;
}

/*!*********************************************************************
\brief Wakeup LTC2949, report and clear all status / alert registers
***********************************************************************/
byte WakeUpReportStatus()
{
	byte  error = LTC2949_WakeupAndAck();
	error |= LTC2949_ReadChkStatusFaults(true, true);
	PrintComma();
	PrintOkErr(error);
	return error;
}

/*!*********************************************************************
\brief Activate / deactivate measurement in LTC2949
- slow continuous mode
- fast continuous CH2 mode
***********************************************************************/
byte Cont(boolean enable)
{
	if (enable)
	{
		byte error = 0;
#ifdef LTCDEF_READ_FROM_EEPROM
		error = LTC2949_EEPROMRead();
#else
		// fast slot not used, still we configure something
		LTC2949_SlotFastCfg(3, 2);
		// SLOT1 measures temperature via NTC between V1 and GND. SLOT2 not used, still we configure something
		LTC2949_SlotsCfg(1, 0, 4, 5);
		// enable NTC temperature measurement via SLOT1
		NtcCfgWrite(1, NTC_RREF, NTC_STH_A, NTC_STH_B, NTC_STH_C);
#endif

#ifdef LTCDEF_WRITE_TO_EEPROM
		error = LTC2949_EEPROMWrite();
#endif

		// read & clear status
		error |= LTC2949_ReadChkStatusFaults(true, false);

		// enable measurement
		return error | LTC2949_GoCont(
			/*cfgFast:      */ LTCDEF_FACTRL_CONFIG,
			/*byte adcCfg:  */ LTCDEF_ADCCFG_CONFIG);
	}
	LTC2949_WriteFastCfg(0);
	LTC2949_OpctlIdle();
	return 0;
}

void NtcCfgWrite(int ntc1or2, float rref, float a, float b, float c)
{
	byte data[3];
	LTC2949_FloatToF24Bytes(rref, data);
	LTC2949_WRITE(ntc1or2 == 2 ? LTC2949_VAL_RREF2 : LTC2949_VAL_RREF1, 3, data);
	LTC2949_FloatToF24Bytes(a, data);
	LTC2949_WRITE(ntc1or2 == 2 ? LTC2949_VAL_NTC2A : LTC2949_VAL_NTC1A, 3, data);
	LTC2949_FloatToF24Bytes(b, data);
	LTC2949_WRITE(ntc1or2 == 2 ? LTC2949_VAL_NTC2B : LTC2949_VAL_NTC1B, 3, data);
	LTC2949_FloatToF24Bytes(c, data);
	LTC2949_WRITE(ntc1or2 == 2 ? LTC2949_VAL_NTC2C : LTC2949_VAL_NTC1C, 3, data);
}
#endif


/* 

  REG  |  BIT_7  |  BIT_6  |  BIT_5  |  BIT_4  |  BIT_3  |  BIT_2  |  BIT_1  |  BIT_0  |
----------------------------------------------------------------------------------------
CFGAR0 |  GPIO5  |  GPIO4  |  GPIO3  |  GPIO2  |  GPIO1  |  REFON  |   DTEN  | ADC_OPT |

here in the code cellMonDat[i * 6 + 0] represents the CFGRA0 registers byte of the (i + 1)th slaves
 
*/

byte CellMonitorCFGA(byte * cellMonDat, bool verbose)
{
	// read configuration and print
	byte error = LTC2949_68XX_RdCfg(cellMonDat);
	if (verbose)
	{
		//SerialPrintByteArrayHex(cellMonDat, LTCDEF_CELL_MONITOR_COUNT * 6, true);
		//PrintComma();
	}
	// set REFON & GPIO in configuration registers
	for (uint8_t i = 0; i < LTCDEF_CELL_MONITOR_COUNT; i++)
	{
		cellMonDat[i * 6 + 0] = 0xFC;
    // for ( uint8_t j = 2; j < 8; j++ )
    // {
    //   cellMonDat[i * 6 + 0] |= (1 << j) ; // REFON and GPIO[1-5] as 1
    // }

    // cellMonDat[i * 6 + 1] = 0; //clear UV & OV
    // cellMonDat[i * 6 + 2] = 0; //clear UV & OV
    // cellMonDat[i * 6 + 3] = 0; //clear UV & OV
		cellMonDat[i * 6 + 4] = 0; //clear all DCC
		cellMonDat[i * 6 + 5] = 0; //clear all DCC
	}
	// write configuration registers
	error |= LTC2949_68XX_WrCfg(cellMonDat);
	return error;
}


/* 

  REG  |  BIT_7  |  BIT_6  |  BIT_5  |  BIT_4  |  BIT_3  |  BIT_2  |  BIT_1  |  BIT_0  |
----------------------------------------------------------------------------------------
CFGBR0 |  DCC16  |  DCC15  |  DCC14  |  DCC13  |  GPIO9  |  GPIO8  |  GPIO7  |  GPIO6  |

here in the code cellMonDat[i * 6 + 0] represents the CFGBR0 registers byte of the (i + 1)th slaves

*/

byte CellMonitorCFGB(byte * cellMonDat, bool verbose, bool muxSelect)
{
	// read configuration and print
	byte error = LTC2949_68XX_RdCfgb(cellMonDat);
	if (verbose)
	{
		//SerialPrintByteArrayHex(cellMonDat, LTCDEF_CELL_MONITOR_COUNT * 6, true);
		//PrintComma();
	}
	// set GPIO[6-9] in configuration registers
  uint8_t k = muxSelect ? 4 : 3;
	for (uint8_t i = 0; i < LTCDEF_CELL_MONITOR_COUNT; i++)
	{
		cellMonDat[i * 6 + 0] = 0;
    for ( uint8_t j = 0; j < k; j++ )
    {
      cellMonDat[i * 6 + 0] |= (1 << j) ; // set GPIO[6-9] as 1
    }

    cellMonDat[i * 6 + 1] = 0;
    cellMonDat[i * 6 + 2] = 0;
    cellMonDat[i * 6 + 3] = 0;
		cellMonDat[i * 6 + 4] = 0; 
		cellMonDat[i * 6 + 5] = 0; 
	}
	// write configuration registers
	error |= LTC2949_68XX_WrCfgb(cellMonDat);
	return error;
}

byte CellMonitorInit()
{
	byte cellMonDat[LTCDEF_CELL_MONITOR_COUNT * 6];
	LTC2949_68XX_RdCfg(cellMonDat); // dummy read
	// dummy read of cell voltage group A
	byte error = ReadPrintCellVoltages(LTC2949_68XX_CMD_RDCVA, (uint16_t*)cellMonDat);
	// clear all cell voltage groups
	error |= LTC2949_68XX_ClrCells();
  error |= LTC2949_68XX_ClrAux();

	error |= CellMonitorCFGA(cellMonDat, true);

	// // read configuration and print
	error |= LTC2949_68XX_RdCfg(cellMonDat);
  // for ( uint8_t i = 0; i < LTCDEF_CELL_MONITOR_COUNT; i++ )
  // {
  //   Serial.println();
  //   for (uint8_t j = 0; j < 6; j++ )
  //   {
  //     Serial.println(cellMonDat[i*6+j],BIN);
  //   }
  // }
	//SerialPrintByteArrayHex(cellMonDat, LTCDEF_CELL_MONITOR_COUNT * 6, true);
	//PrintComma();
  
  error |= CellMonitorCFGB(cellMonDat, true, false);
  error |= LTC2949_68XX_RdCfgb(cellMonDat);
	//SerialPrintByteArrayHex(cellMonDat, LTCDEF_CELL_MONITOR_COUNT * 6, true);
	//PrintComma();
  
	// error |= ReadPrintCellVoltages(LTC2949_68XX_CMD_RDCVA, (uint16_t*)cellMonDat);
  // error |= ReadPrintAuxVoltages(LTC2949_68XX_CMD_RDAUXA, (uint16_t*)cellMonDat);

	// // trigger cell voltage measurement of all cells in fast mode
  //Serial.println("Start ADCV");
	error |= LTC2949_ADxx(
		/*byte md = MD_NORMAL     : */MD_FAST,
		/*byte ch = CELL_CH_ALL   : */CELL_CH_ALL,
		/*byte dcp = DCP_DISABLED : */DCP_DISABLED,
		/*uint8_t pollTimeout = 0 : */0
	);
	delay(2); // wait for conversion results ready to be read
	// print all conversion results
  //Serial.println("end ADCV");
	error |= ReadPrintCellVoltages(LTC2949_68XX_CMD_RDCVA, (uint16_t*)cellMonDat);
	// // this is just for debugging anyway, so we don't print all values here. See main loop for the actual cyclic measurements
	// //error |= ReadPrintCellVoltages(LTC2949_68XX_CMD_RDCVB, (uint16_t*)cellMonDat);
	// //error |= ReadPrintCellVoltages(LTC2949_68XX_CMD_RDCVC, (uint16_t*)cellMonDat);
	// //error |= ReadPrintCellVoltages(LTC2949_68XX_CMD_RDCVD, (uint16_t*)cellMonDat);
	// //error |= ReadPrintCellVoltages(LTC2949_68XX_CMD_RDCVE, (uint16_t*)cellMonDat);
	// //error |= ReadPrintCellVoltages(LTC2949_68XX_CMD_RDCVF, (uint16_t*)cellMonDat);
  
  //Serial.println("Start ADAX ");
  error |= LTC2949_ADAX(
		/*byte md = MD_NORMAL     : */MD_FAST,
		/*byte ch = CELL_CH_ALL   : */CELL_CH_ALL,
		/*byte dcp = DCP_DISABLED : */DCP_DISABLED,
		/*uint8_t pollTimeout = 0 : */0
	);
  delay(2);  // wait for conversion results ready to be read
  //Serial.println("end ADAX");
	// print all conversion results
  error |= ReadPrintAuxVoltages(LTC2949_68XX_CMD_RDAUXA, (uint16_t*)cellMonDat);

	return error;
}









// New functions


// static inline byte LTC2949_68XX_ClrAux()
// {
// 	return LTC2949_68XX_CMD(
// 		/*uint8_t addr:       */ 0, // broadcast is mandatory in case of reading data from cell monitors
// 		/*uint16_t cmd:       */ LTC2949_68XX_CMD_CLRAUX
// 	);
// }


// // The new function to be added is the below one
// static inline byte LTC2949_68XX_RdCfgb(byte* data)
// {
// 	return LTC2949_68XX_CMD(
// 		/*uint8_t addr:       */ 0, // broadcast is mandatory in case of reading data from cell monitors
// 		/*uint16_t cmd:       */ LTC2949_68XX_CMD_RDCFGB,
// 		/*byte *data:         */ (byte*)data,
// 		/*uint16_t len:       */ LTC2949_CellMonitorCount * 6U
// 	);
// }

// // The new function to be added is the below one
// static inline byte LTC2949_68XX_WrCfgb(byte* data)
// {
// 	return LTC2949_68XX_CMD(
// 		/*uint8_t addr:       */ 0, // broadcast is mandatory in case of reading data from cell monitors
// 		/*uint16_t cmd:       */ LTC2949_68XX_CMD_WRCFGB,
// 		/*byte *data:         */ (byte*)data,
// 		/*uint16_t len:       */ LTC2949_CellMonitorCount * 6U
// 	);
// }


/**** This function is for the ADAX command ****/

// static inline byte LTC2949_ADax(
// 	byte md = MD_NORMAL,
// 	byte ch = CELL_CH_ALL,
// 	byte dcp = DCP_DISABLED
// #ifdef LTCDEF_SPI_POLL_SUPPORT
// 	, uint8_t pollTimeout16us = 0
// #else
// #endif
// ) {
// 	// Note: ADCV will always be send as broadcast command
// 	byte error = LTC2949_68XX_CMD(
// 		/*uint8_t addr:           */ 0,
// 		/*uint16_t cmd:           */ LTC2949_GetADAXCmd(md, dcp, ch
// #ifdef LTCDEF_SPI_POLL_SUPPORT
// 			, pollTimeout16us > 0
// #else
// #endif
// 		),
// 		/*byte *data:             */ NULL,
// 		/*uint16_t len:           */ 0,
// 		/*byte *data2949:         */ NULL,
// 		/*uint16_t len2949:       */ 0
// #ifdef LTCDEF_SPI_POLL_SUPPORT
// 		,/*uint8_t pollTimeout16us:*/ pollTimeout16us
// #else
// #endif
// 	);
// 	return error;
// }


// static inline uint16_t LTC2949_GetADAXCmd(
// 	byte MD, //ADC Mode
// 	byte DCP, //Discharge Permit
// 	byte CH //Cell Channels to be measured
// #ifdef LTCDEF_SPI_POLL_SUPPORT
// 	, boolean poll // true for poll end of conversion, false for send ADCV only
// #else
// #endif
// )
// {
// 	return
// 		/*CMD0 / MSB:*/ (((MD & 0x02U) >> 1) | 0x02U) << 9 |
// 		/*CMD1 / LSB:*/ ((MD & 0x01U) << 7) | 0x60U | (DCP << 4) | CH |
// #ifdef LTCDEF_SPI_POLL_SUPPORT
// 		(poll ? LTC2949_68XX_CMDTYPE_PL : LTC2949_68XX_CMDTYPE_WR)
// #else
// 		LTC2949_68XX_CMDTYPE_WR
// #endif
// 		;
// }

// static inline byte LTC2949_68XX_RdAux(uint16_t rdaux, uint16_t* data)
// {
// 	byte error = LTC2949_68XX_CMD(
// 		/*uint8_t addr:       */ 0, // broadcast is mandatory in case of reading data from cell monitors
// 		/*uint16_t cmd:       */ rdaux,
// 		/*byte *data:         */ (byte*)data,
// 		/*uint16_t len:       */ LTC2949_CellMonitorCount * 6U
// 	);
// 	LTC2949_ReorderData(data, LTC2949_CellMonitorCount * 3U);
// 	return error;
// }
