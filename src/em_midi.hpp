// Copyright (C) Paul Chase Dempsey
#pragma once
#ifndef EM_MIDI_HPP
#define EM_MIDI_HPP
#include "plugin.hpp"
using namespace ::rack;

// "G:\Reference\Haken\contctl.h"

namespace em_midi {
#if !defined U8
#define U8(arg) static_cast<uint8_t>(arg)
#endif

struct ISendMidi
{
    virtual void sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {}
    virtual void sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {}
    virtual void sendControlChange(uint8_t channel, uint8_t cc, uint8_t value) {}
    virtual void sendProgramChange(uint8_t channel, uint8_t program) {}
    virtual void sendKeyPressure(uint8_t channel, uint8_t note, uint8_t pressure) {}
    virtual void sendChannelPressure(uint8_t channel, uint8_t pressure) {}
    virtual void sendPitchBend(uint8_t channel, uint8_t bend_lo, uint8_t bend_hi) {}
    virtual bool readyToSend() { return true; }
};

constexpr const uint8_t MaxPresetName = 31; // Haken editor truncates to here on loading a preset.
constexpr const uint8_t MaxPresetNameSize = MaxPresetName + 1; // including zero termination

constexpr const uint8_t MidiStatus_NoteOff              = 0x80;
constexpr const uint8_t MidiStatus_NoteOn               = 0x90;
constexpr const uint8_t MidiStatus_PolyKeyPressure      = 0xa0;
constexpr const uint8_t MidiStatus_CC                   = 0xb0;
constexpr const uint8_t MidiStatus_ProgramChange        = 0xc0;
constexpr const uint8_t MidiStatus_ChannelPressure      = 0xd0;
constexpr const uint8_t MidiStatus_PitchBend            = 0xe0;
constexpr const uint8_t MidiStatus_SysEx                = 0xf0;
constexpr const uint8_t MidiStatus_SysExEnd             = 0xf7;

constexpr const uint8_t MidiStatus_SysRT_TimingClock    = 0xf8;
//constexpr const uint8_t MidiStatus_SysRT_Undefined    = 0xf9;
constexpr const uint8_t MidiStatus_SysRT_Start          = 0xfa;
constexpr const uint8_t MidiStatus_SysRT_Continue       = 0xfb;
constexpr const uint8_t MidiStatus_SysRT_Stop           = 0xfc;
//constexpr const uint8_t MidiStatus_SysRT_Undefined   = 0xfd;
constexpr const uint8_t MidiStatus_SysRT_ActiveSensing = 0xfe;
constexpr const uint8_t MidiStatus_SysRT_SystemReset   = 0xff;

constexpr const uint8_t RackStatus_NoteOff            = 0x08;
constexpr const uint8_t RackStatus_NoteOn             = 0x09;
constexpr const uint8_t RackStatus_PolyKeyPressure    = 0x0a;
constexpr const uint8_t RackStatus_CC                 = 0x0b;
constexpr const uint8_t RackStatus_ProgramChange      = 0x0c;
constexpr const uint8_t RackStatus_ChannelPressure    = 0x0d;
constexpr const uint8_t RackStatus_PitchBend          = 0x0e;
constexpr const uint8_t RackStatus_SysEx              = 0x0f;

constexpr const int   EM_Max14  = 0x3f80; // 16,256: maximum 14-bit controller value
constexpr const float EM_Max14f = 16256.f;

constexpr const uint8_t MidiCC_BankSelect       = 0;
constexpr const uint8_t MidiCC_ModWheel         = 1;
constexpr const uint8_t MidiCC_Breath           = 2;
// undefined
constexpr const uint8_t MidiCC_Foot             = 4;
constexpr const uint8_t MidiCC_PortamentoTime   = 5;
constexpr const uint8_t MidiCC_DataMSB          = 6;
constexpr const uint8_t MidiCC_Volume           = 7;
constexpr const uint8_t MidiCC_Balance          = 8;

constexpr const uint8_t MidiCC_Pan              = 10;
constexpr const uint8_t MidiCC_Expression       = 11;
constexpr const uint8_t MidiCC_Effect1          = 12;
constexpr const uint8_t MidiCC_Effect2          = 13;
// 14-15 Undefined
// 16-19 General Purpose 1-4
// 20-31 Undefined
// 32-63 LSBs for 0-31
constexpr const uint8_t MidiCC_Sustain          = 64;
constexpr const uint8_t MidiCC_PortamentoSwitch = 65;
constexpr const uint8_t MidiCC_Sostenuto        = 66;
constexpr const uint8_t MidiCC_Soft             = 67;
constexpr const uint8_t MidiCC_Legato           = 68;
constexpr const uint8_t MidiCC_Hold             = 69;
constexpr const uint8_t MidiCC_SoundVariation   = 70;
constexpr const uint8_t MidiCC_Intensity        = 71;
constexpr const uint8_t MidiCC_Release          = 72;
constexpr const uint8_t MidiCC_Attack           = 73;
constexpr const uint8_t MidiCC_Brightness       = 74;
//...
//constexpr const uint8_t MidiCC_Sound10        = 79;
// 80-83 General Purpose 5-8
constexpr const uint8_t MidiCC_PortamentoControl = 84;
//85-90 Undefined
//91-95 Effect depth
constexpr const uint8_t MidiCC_DataIncrement    = 96;
constexpr const uint8_t MidiCC_DataDecrement    = 97;
constexpr const uint8_t MidiCC_NRPN_LSB         = 98;
constexpr const uint8_t MidiCC_NRPN_MSB         = 99;
constexpr const uint8_t MidiCC_RPN_LSB          = 100;
constexpr const uint8_t MidiCC_RPN_MSB          = 101;

constexpr const uint8_t MidiCC_AllSoundOff  = 120;
constexpr const uint8_t MidiCC_Reset        = 121;
constexpr const uint8_t MidiCC_LocalControl = 122;
constexpr const uint8_t MidiCC_AllNotesOff  = 123;
constexpr const uint8_t MidiCC_OmniOff      = 124;
constexpr const uint8_t MidiCC_OmniOn       = 125;
constexpr const uint8_t MidiCC_Mono         = 126;
constexpr const uint8_t MidiCC_Poly         = 127;

// channel 16
constexpr const uint8_t EMCC_OctaveShift    = 8; // 48/60/72 = down, normal, up
constexpr const uint8_t EMCC_MonoSwitch     = 9;
constexpr const uint8_t EMCC_FineTune       = 10; // 4..124 = +- 60 cents
constexpr const uint8_t EMCC_i              = 12;
constexpr const uint8_t EMCC_ii             = 13;
constexpr const uint8_t EMCC_iii            = 14;
constexpr const uint8_t EMCC_iv             = 15;
constexpr const uint8_t EMCC_v              = 16;
constexpr const uint8_t EMCC_vi             = 17;
constexpr const uint8_t EMCC_PostLevel      = 18;
constexpr const uint8_t EMCC_AudioInLevel   = 19;
constexpr const uint8_t EMCC_R1             = 20;
constexpr const uint8_t EMCC_R2             = 21;
constexpr const uint8_t EMCC_R3             = 22;
constexpr const uint8_t EMCC_R4             = 23;
constexpr const uint8_t EMCC_RMIX           = 24;
constexpr const uint8_t EMCC_RoundRate 	    = 25;
constexpr const uint8_t EMCC_PreLevel       = 26;
constexpr const uint8_t EMCC_Attenuation    = 27;
constexpr const uint8_t EMCC_RoundInitial   = 28;
constexpr const uint8_t EMCC_Jack1          = 29;
constexpr const uint8_t EMCC_Jack2          = 30;
constexpr const uint8_t EMCC_Advance        = 31; //127 next, 64=next layer
constexpr const uint8_t EMCC_Category       = 32;
constexpr const uint8_t EMCC_ActionAesMenuRecirc = 33;

constexpr const uint8_t EMCC_Routing        = 36;
constexpr const uint8_t EMCC_PedalType      = 37;

constexpr const uint8_t EMCC_Polyphony      = 39;
constexpr const uint8_t EMCC_BendRange      = 40; //MPE_MIN=12, default|max=96
constexpr const uint8_t EMCC_YCC            = 41; //0=none, 127 = no_shelf
constexpr const uint8_t EMCC_ZCC            = 42; //0=none 11=default(Expression) 69=channel pressure, 70=MPE+, 127=MPE
constexpr const uint8_t EMCC_MiddleC        = 44;
constexpr const uint8_t EMCC_SplitPoint     = 45;
constexpr const uint8_t EMCC_MonoFunction   = 46;
//ctlReciCol
constexpr const uint8_t EMCC_MonoInterval   = 48;
constexpr const uint8_t EMCC_Priority       = 49;
constexpr const uint8_t EMCC_TuningGrid     = 51;
constexpr const uint8_t EMCC_Pedal1CC       = 52;
constexpr const uint8_t EMCC_Pedal2CC       = 53;
constexpr const uint8_t EMCC_OctaveShiftAmount = 54; //default=48
constexpr const uint8_t EMCC_Preserve       = 55;
constexpr const uint8_t EMCC_DataStream     = 56;
enum EM_StreamType {
    Name=0, ConText, Graph, GraphO1, GraphO2, GraphT0, GraphT1,
    Log, UpdF2, DemoAssort, Float, Kinet, BiqSin, Sys, Conv,
    DataEnd = 127
};
constexpr const uint8_t EMCC_DimMenu        = 59;
constexpr const uint8_t EMCC_TouchArea      = 60;
constexpr const uint8_t EMCC_Reverse_Rounding = 61;
constexpr const uint8_t EMCC_RecirculatorType = 62;
enum EM_Recirculator {
    Reverb = 0,
    ModDelay, SweptEcho, AnalogEcho, DigitalEchoLPF, DigitalEchoHPF,
    Mask    = 7,
    Extend  = 0x40,	// extended recirculator time (delay line half sample rate)
};
constexpr const uint8_t EMCC_Sustain        = 64;
constexpr const uint8_t EMCC_RoundEqual     = 65; //0=disable, 64=enable, 127=round equal tuning
constexpr const uint8_t EMCC_Sostenuto      = 66;
constexpr const uint8_t EMCC_HeadphoneLevel = 67;
constexpr const uint8_t EMCC_LineLevel      = 68;
constexpr const uint8_t EMCC_Sostenuto2     = 69;
constexpr const uint8_t EMCC_Actuation      = 70;
constexpr const uint8_t EMCC_PolyTrad		= 71; // ** Continuum out: total _TRAD polyphony  6.00
constexpr const uint8_t EMCC_PolyDsp		= 72; // ** Continuum out: total _DSP polyphony   6.00
constexpr const uint8_t EMCC_PolyCvc		= 73; // ** Continuum out: total _CVC polyphony   6.00
constexpr const uint8_t EMCC_Y              = 74;

constexpr const uint8_t EMCC_MinPedal1	    = 76;
constexpr const uint8_t EMCC_MaxPedal1	    = 77;
constexpr const uint8_t EMCC_MinPedal2	    = 78;
constexpr const uint8_t EMCC_MaxPedal2	    = 79;

constexpr const uint8_t EMCC_TiltEq          = 83;
constexpr const uint8_t EMCC_TiltEqFrequency = 84; // 0..127 = 120Hz..15kHz	
constexpr const uint8_t EMCC_TiltEqMix       = 85; // 0=dry .. 127=wet

// following 4 not just ch 16
constexpr const uint8_t EMCC_PedalFraction  = 86;
constexpr const uint8_t EMCC_Fraction       = 87; //bend/y/z
constexpr const uint8_t EMCC_SplitLow       = 88;
constexpr const uint8_t EMCC_SplitHigh      = 89;

constexpr const uint8_t EMCC_CompressorThreshold  = 90;
constexpr const uint8_t EMCC_CompressorAttack     = 91;
constexpr const uint8_t EMCC_CompressorRatio      = 92;
constexpr const uint8_t EMCC_CompressorMix        = 93;

constexpr const uint8_t EMCC_VersionHigh 	= 102;
constexpr const uint8_t EMCC_VersionLow 	= 103;
constexpr const uint8_t EMCC_CVCHi_Hardware = 104; // 5/hardware, 2/cvcSerialMsb (bits 15..14)
constexpr const uint8_t EMCC_CVCMid         = 105; // med 7 bits
constexpr const uint8_t EMCC_CVCLo          = 106; // low 7 bits
constexpr const uint8_t EMCC_Download       = 109;
constexpr const uint8_t EMCC_Info           = 110;
constexpr const uint8_t EMCC_Status         = 111;
constexpr const uint8_t EMCC_DSP            = 114; // 32*dsp# + 4%units (0-25)

constexpr const uint8_t EMCC_EditorPresent  = 116;
constexpr const uint8_t EMCC_LoopDetect     = 117;
constexpr const uint8_t EMCC_EditorReply    = 118;
constexpr const uint8_t EMCC_ArchiveCount   = 119; // count of CCs in .mid file

constexpr const uint8_t EMCC_MPE            = 127; // value = polyphony

constexpr const uint8_t EM_MasterChannel    = 0;
constexpr const uint8_t EM_MPE1Channel      = 1;
constexpr const uint8_t EM_MatrixChannel     = 14;
constexpr const uint8_t EM_SettingsChannel   = 15;

constexpr const uint8_t EM_UserCategopry    = 0;
constexpr const uint8_t EM_EditBuffer       = 126;
constexpr const uint8_t EM_SystemSlot       = 127;

// EMCC_CVCHi_Hardware values
enum EM_Hardware
{
    Unknown,
    hw_LightFull,   // light action full, not UP version
    hw_LightHalf,   // light action, not UP version
    hw_ClassicFull, // classic action full size
    hw_ClassicHalf, // classic action half size
    hw_Mini,        // ContinuuMini 
    hw_Osmose49,    // Osmose
    hw_Slim22,      // slim 22L6x
    hw_Slim46,      // slim 46L6x
    hw_Slim70,      // slim 70L6x
    hw_EMM,         // Eurorack EaganMatrix Module
    hw_LightFullUp, // light action full, UP triple-dsp board
    hw_LightHalfUp, // light action half, UP triple-dsp board
};

const char * HardwarePresetClass(EM_Hardware hw);
const char * ShortHardwareName(EM_Hardware hw);
const char * HardwareName(EM_Hardware hw);

// EMCC_Download values
// cc109 and cc110 use Editor's Message Bar: C = Center Display, L = Left Display, M = Max Log
enum EM_DownloadItem {
    downloadOkBoot 	= 0,	// C  ask user to turn off and back on again
    downloadFail 	= 1,	// C  center display: "Operation Failed.  Try again."
    downloadOkChained=2,	// C  center display: n/c (leave up "Download in Progress")
    //				= 3,	// LC available
    downloadOkData	= 4,	// C  center display: "Data download completed."
    archiveOk		= 5,	// C  center display: erase message bar; sent at very end of Preset Group
    archiveFail		= 6,	// C  center display: "Operation Failed.  Try again." ****
    queryKenton		= 7,	//    Editor will respond with ctlKHello to say if a Kenton is present
    curGloToFlash	= 8,	//    From Editor: write global/calib/sensorMap/current to flash
                            //	  Special case if Archive Retrieve: end of Archive Retrieve
    reduceGain		= 9,	// C  center display: "Reduce Gain"
    reducePoly		= 10,	// C  center display: "Reduce Polyphony"
    inFactCalib		= 11,	// C  center display: "Factory Calibration In Progress" [modified 6.38]
    eraseMessage	= 12,	// LC erase message bar
    aesSyncFail		= 13,	// C  center display: "AES3 Sync Failure"
    cvcPower		= 14,	// C  center display: "Turn On or Disconnect CVC"
    ceeMismatch		= 15,	// C  firmware version mismatch between CEEs
    configToMidi	= 16,	//    current config to Midi
    startFirmware 	= 17,	//    begin firmware download
    startData		= 18,	//    begin synthesis data download
    burnUser364 	= 19,	//    done with firmware 21364 download, burn user flash (if crc matched)
    endDataDownload = 20,	//    end data firmware/data download
    midiLoopback	= 21,	// C  Midi-loopback-detected error message
    //				= 22,	//
    //				= 23,	//
    begTxDsp		= 24,	// 	  begin CEE config send txDsp (from daisy=1 to 2,3)
    endTxDsp		= 25,	// 	  end CEE config send txDsp (from daisy=1 to 2,3)
    doneTxDsp		= 26,	// 	  handshake back: end of txDsp preset-sending process
    txDspFail		= 27,	// ?  config send txDsp failure - could try again?  logged in Max window
    doUpdF2			= 28,	// C  after Update File 1 reboot, tell user to do Update File 2
                            //	  This is used by Osmose.
    createLed		= 29,	//	  turn on yellow LED for archive create
    testBegin		= 30,	//    Editor to Continuum: begin Midi stress test
    testErr			= 31,	// ?  Continuum to Editor: error in Midi rx sequence
    userToMidi		= 32,	//    from Editor: preset names to Midi, then current config
    manualUpdate	= 33,	//    old preset needs manually-implemented update
    doResetCalib	= 34,	//	  set zero levels, like optCalib+0
    doRefineCalib	= 35,	//	  incorporate new zero levels, like optCalib+1
    midiTxFull		= 36,	//	  full midi transmission rate
    midiTxThird		= 37,	//	  one-third midi transmission rate
    midiTxTweenth	= 38,	//	  one-twentieth midi transmission rate
    sysToMidi		= 39,	//	  from Editor: preset names to Midi, then current config
    endSysNames		= 40,	//	  to Editor: end list of system presets
    doFactCalib		= 41,	//    do factory calibration
    doUpdate1		= 42,	// C  after recovery boot, let user know ready for update file 1
                            //	  This is used by Osmose to detect Recovery Mode.
    burnUser489 	= 43,	//    done with firmware 21489 download, burn user flash (if crc matched)
    dspReboot		= 44,	//	  reboot after Firmware File 1
    surfAlign		= 45,	//	  toggle slim Continuum's Surface Alignment mode
    addTrim			= 46,	//	  add currently-playing finger to Trim array
    delTrim			= 47,	//    remove trim point closes to currently-playing finger
    resetTrim		= 48,	//    remove all trim data
    beginSysNames	= 49,	//	  to Editor: begin list of system presets
    exitCombi		= 50,	//	  exit Combination Preset mode: edit all DSPs
    miniFactSetup	= 51,	//	  store calib/global/userPresets to continuuMini factory setup
    decPreset		= 52,	//	  to prev sysPreset
    incPreset		= 53,	//	  to next sysPreset
    beginUserNames	= 54,	//	  to Editor: begin list of user presets
    endUserNames	= 55,	//	  to Editor: end list of user presets
    sameSlot		= 56,	// C  save Combi preset to same slot or to disk
    preEraseUpdF1	= 57,	//    pre-erase flash Update File 1; echo when dsp1 done
    preEraseUpdF2	= 58,	//    pre-erase flash Update File 2; echo when dsp1 done
    preEraseUpdF3	= 59,	//    pre-erase flash Update File 3; echo when dsp1 done
    remakeSRMahl	= 60,	//	  remake synthesis-ready Mahling data
    doneFactProg	= 61,	//	  done copying to/from Factory Program Board
    failFactProg	= 62,	//	  failed copying to/from from Factory Program Board
    usbTxNoAck		= 63,	// M  Usb-Midi out from Mini did not get Ack
    rxOver 			= 64,	// M  Midi rx queue overflow
    txOver 			= 65,	// M  Midi tx queue overflow
    rxSynErr 		= 66,	// M  Midi rx syntax error
    rxBitErr 		= 67,	// M  Midi rx bad bit widths
    sensComm		= 68,	// M  serial sensors errors
    nanErr			= 69,	// M  output has nan
    ceeSeq			= 70,	// M  CEE comm glitch
    burnUserMini	= 71,	//	  ContinuuMini firmware has this at end
    doMidiLog0		= 72,	//	  end scrolling ascii log via Midi
    doMidiLog1		= 73,	//	  daisy=0,1 scrolling ascii log via Midi
    doMidiLog2		= 74,	//	  daisy=2 scrolling ascii log via Midi
    doMidiLog3		= 75,	//	  daisy=3 scrolling ascii log via Midi
    burnRecovery489 = 76,	//    done firmware 21489 download, burn recovery (done at factory only!)
    burnRecovery364 = 77,	//    done firmware 21364 download, burn recovery (done at factory only!)
    burnRecoveryMini= 78,	//    done firmware ContinuuMini download, burn recovery
    //				= 79,	//
    defFirstTuning	= 80,	//    80..87 begin loading user tuning grid; data follows using ctlDataLow
        // 80..87 are for downloading user tuning grids
    defLastTuning 	= 87,	//    80..87 begin loading user tuning grid; data follows using ctlDataLow
    numDecMat		= 88,	//	  decrement numeric matrix point
    numIncMat		= 89,	//	  increment numeric matrix point
    mendDisco		= 90,	//	  mend discontinuity at note (outlier to Sensor Map)
    rebootRecov		= 91,	//    reboot in Recovery Mode
    stageUp			= 92,	//	  host to dsp: upload monolithic update
    stageDown		= 93,	//	  host to dsp: download monolithic update
    stageDownOk1	= 94, stageDownOk2, stageDownOk3,		// dsp to host at end of StageDown
    stageDownFail1	= 97, stageDownFail2, stageDownFail3,	// dsp to host at end of StageDown
    rebootUser		= 100,	//    reboot in User (normal) Mode
    gridToFlash 	= 101,	//    save tuning grids to flash
    mendDivided		= 102,	//	  mend divided note (add flawed sensor to Sensor Map)
    startUpdF2		= 103,	//    from Editor: beginning of Update File 2
    notFirst		= 104,	//	  to Editor: not first preset in a combination
    firstOfTwo		= 105,	//	  to Editor: first preset in a dual combination
    firstofThree	= 106,	//	  to Editor: first preset in triple combination
    Demo1			= 107,	//	  Demo Assortment to first set-of-16 user presets.
        // 107..114 are for loading demo assortment into user presets
    Demo8			= 114,	//	  Demo Assortment to last set-of-16 user presets.
    Empty1			= 115,	//	  Empty preset to first set-of-16 user presets.
        // 115..122 are for emptying set-of-16 user presets
    Empty8			= 122,	//	  Empty preset to last set-of-16 user presets.
    burnUser593 	= 123,	//    done with firmware 21593 download, burn user flash (if crc matched)
    burnRecovery593 = 124,	//    done firmware 21593 download, burn recovery (done at factory only!)
};

// EMCC_Info (110) values
enum InfoItem {
    profileEnd		= 0,	// LC erase message bar, Editor puts up Save dialog
    percentFirst	= 1,	// L  left display "1%"
    percentLast		= 99,	// L  left display "99%"
    cfCreateArch0	= 100,	//    Save Preset: received from Editor, start *creation* archive
                            //	  value 100 is preset edit buffer (userPreset0)
    cfCreateArch1	= 101,	//    Save Preset: received from Editor, start *creation* archive
                            //	  value 101..116 is within set-of-16 user preset slots
    cfCreateArch16	= 116,	//    Save Preset: received from Editor, start *creation* archive
    inProgress		= 118,	// C  center display: "Download in progress. Please wait." 
    archiveNop		= 119,	// 	  no-op, used as padding
    edRecordArchive	= 120,	//	  sent to Editor when cfCreateArchive is received;
                            //	  Editor starts recording, but n/i edRecordArchive
    cfRetrieveArch  = 121,	//    sent to Editor along with edRecordArchive -- 
                            //	  played back from Editor at start of *retrieval* from archive;
                            // 	  Editor itself does not use this (except to save it)
                            //	  NB: Save-to-flash is part of cData processing.
    archiveEof		= 123,	//	  last data sent to Editor in archive create; Editor does not
                            //	  use this (except to store in file); echoed back from Editor as last
                            //    data in file during *retrieval* -- Continuum verifies message count
                            //	  (but does not exit ArchiveRetrieve until later at curGloToFlash)
    archiveToFile	= 124,	// 	  archive *creation*: Editor stops recording, save seq to file system
                            //	  archive retrieval: Continuum does nothing with this
    finalizing		= 125,	// L  left display "finalizing flash"
    //				= 126,	// L  left display "initializing" [no longer used, except in old archives]
    profileStart	= 127,	// C  center display: "Profile is being generated. Please wait."
};

// EMCC_Status (111) cvalues
enum StatusItem {
    sLedBits        = 0x0f,	//    select led color bits
    ledOff          = 0,	//	  led color
    ledBlue         = 1,	//	  led color
    ledRed          = 2,	//	  led color				 part of config
    ledBrightGreen  = 3,	//	  led color				 part of config
    ledGreen        = 4,	//	  led color				 part of config
    ledWhite        = 5,	//	  led color 			 calib		
    ledYellow       = 6,	//	  led color				 download, etc
    ledPurple       = 7,	//	  led color				 no aes sync, or flashing if cvc power off
    ledBlueGreen    = 8,	//	  led color  4.02		 [not used; retired from flawed sensor]

    sAesBits        = 0x70,	//    aes detected rate bits
    sAesShift		= 4,	//    shift for aes detected bits
    aesInputNone    = 0,	//	  no aes input stream
    aesInputNonStd  = 1,	//	  aes input nonstandard rate
    aesInput44p1    = 2,	//	  aes input  44.1 kHz	4.15 [Ottawa]
    aesInput48      = 3,	//	  aes input  48.0 kHz
    aesInput88p2    = 4,	//	  aes input  88.2 kHz
    aesInput96      = 5,	//	  aes input  96.0 kHz
    aesInput176p4   = 6,	//	  aes input 176.4 kHz
    aesInput192     = 7,	//	  aes input 192.0 kHz
    // StdRate 44100,48000,88200,96000,176400,192000
};
const char * RecirculatorName(EM_Recirculator r);
const char * RecirculatorParameterName(EM_Recirculator r_type, int r);
const char * DownloadMessage(int id);
inline bool Is14BitPedalCC(uint8_t cc) { return (EMCC_i <= cc) && (cc <= EMCC_vi); }

inline void SetNoteOn(midi::Message& msg, uint8_t channel, uint8_t note, uint8_t velocity = 127)
{
    msg.bytes = { U8(MidiStatus_NoteOn | channel), note, velocity };
}

inline void SetNoteOff(midi::Message& msg, uint8_t channel, uint8_t note, uint8_t velocity = 0)
{
    msg.bytes = { U8(MidiStatus_NoteOff | channel), note, velocity };
}

inline void SetCC(midi::Message& msg, uint8_t channel, uint8_t cc, uint8_t value)
{
    msg.bytes = { U8(MidiStatus_CC | channel), cc, value };
}

inline void SetProgramChange(midi::Message& msg, uint8_t channel, uint8_t program)
{
    msg.bytes = { U8(MidiStatus_ProgramChange | channel), program };
}

inline uint8_t GetCC(const midi::Message& msg) { return msg.bytes[1]; }
inline uint8_t GetRawStatus(const midi::Message& msg) { return msg.bytes[0] & 0xf0; }
const char * StatusName(uint8_t status);
const char * continuumCC(uint8_t cc);
const char * midiCC(uint8_t cc);
std::string ToFormattedString(const midi::Message& msg);

inline int MessageBytes(uint8_t status_byte)
{
    switch (status_byte & 0xf0) {
    case MidiStatus_NoteOff:
    case MidiStatus_NoteOn:
    case MidiStatus_PolyKeyPressure:
    case MidiStatus_CC:
        return 2;

    case MidiStatus_ProgramChange:
    case MidiStatus_ChannelPressure:
        return 1;

    case MidiStatus_PitchBend:
        return 2;

    default:
        return 0;
    }
}

union uMidiMessage {
    uint32_t data;
    uint8_t bytes[4];
    uMidiMessage() : data(0) {}
    uMidiMessage(uint64_t dw) : data(dw) {}
    uMidiMessage(uint8_t b1, uint8_t b2) : data(0) {
        bytes[0] = b1;
        bytes[1] = b2;
    }
    uMidiMessage(uint8_t b0, uint8_t b1, uint8_t b2) : data(0) {
        bytes[0] = b0;
        bytes[1] = b1;
        bytes[2] = b2;
    }
    uMidiMessage(const midi::Message& msg) : data(0) {
        size_t i = 0;
        for (auto b: msg.bytes) {
            bytes[i++] = b;
        }
    }
    void toMidiMessage(midi::Message& msg)
    {
        msg.bytes[0] = bytes[0];
        switch (MessageBytes(bytes[0])) {
            default:
            case 0:
                msg.setSize(1);
                break;
            case 1:
                msg.bytes[1] = bytes[1];
                msg.setSize(2);
                break;
            case 2:
                msg.bytes[1] = bytes[1];
                msg.bytes[2] = bytes[2];
                msg.setSize(3);
                break;
        }
    }
    midi::Message toMidiMessage() {
        midi::Message msg;
        toMidiMessage(msg);
        return msg;
    }
    void setStatus(uint8_t status) { bytes[0] = channel() | (status & 0xf0); }
    void setChannel(uint8_t channel) { bytes[0] = status() | (channel & 0x0f); }
    void setFirstByte(uint8_t b) { bytes[1] = b; }
    void setSecondByte(uint8_t b) { bytes[2] = b; }
	inline uint8_t status() { return bytes[0] & 0xf0; }
	inline uint8_t channel() { return bytes[0] & 0x0f; }
	inline uint8_t b1() { return bytes[1]; }
	inline uint8_t b2() { return bytes[2]; }
    inline uint64_t raw() { return data; }
};

inline uint8_t Status(uint64_t msg) { return uMidiMessage(msg).status(); }
inline uint8_t Channel(uint64_t msg) { return uMidiMessage(msg).channel(); }
inline uint8_t B1(uint64_t msg) { return uMidiMessage(msg).bytes[1]; }
inline uint8_t B2(uint64_t msg) { return uMidiMessage(msg).bytes[2]; }

}
#endif