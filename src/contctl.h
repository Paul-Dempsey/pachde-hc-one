// Continuum Fingerboard Software by Lippold Haken, (C) Copyright 2001-2022.
// This program is proprietary and confidential.

#ifndef CONTCTL_H
#define CONTCTL_H

// This file contains:
// - Midi status byte definitions
// - Midi controller definitions for Continuum
// - Continuum configuration strip locations,
//   for configuration from playing surface


// Midi "status byte" definitions.
enum {
	keyOn 			= 0x90,		// default: Continuum uses constant 127 for keyOn velocity
	keyOff 			= 0x80,
		halfstepsPerOctave = 12,
		nnMiddleC 	= 60,
		nnMiddleA 	= 69,
		nnHi		= 119,		// half step below 10th octave (for display, good to avoid 10th oct)
		nnLowFull	= 15,		// lowest note number (without transpose) on full size
		nnHighFull	= 109,		// highest note number (without transpose) on full size
		nnLow70		= 28,		// nominal lowest note number (without transpose) on slim70
		nnLow70fsh	= 30,		// lowest f# on a slim70
		nnHigh70	= 96,		// nominal highest note number (without transpose) on slim70
		nnHigh70c	= 96,		// highest c on a slim70
		nnLow46		= 40,		// nominal lowest note number (without transpose) on slim46 and half size
		nnLow46fsh	= 42,		// lowest f# on a slim46
		nnHigh46	= 85,		// nominal highest note number (without transpose) on slim46 and half size
		nnHigh46c	= 84,		// highest c on a slim46
		nnLowMini	= 52,		// lowest note number (without transpose) on ContinuuMini
		nnHighMini	= 80,		// highest note number (without transpose) on ContinuuMini
		nnMidMini	= 66,		// (nnLowMini+nnHighMin)/2
	polyKeyPres 	= 0xa0,		// polyphonic key pressure (2 data bytes follow) for firmware update 16'.
								// Channel bits = 10XY:  X is msb for first data byte, Y for second.
	ctlChg 			= 0xb0,		// control change; (2 data bytes follow) controllers defined below  
		max14		  = 0x3f80,	// max for Continuum's 14-bit controllers (max msb, zero lsb: 127<<7)
	progChg 		= 0xc0,		// program change (1 data byte follows)
		progChgF		= 0xcf,	// ch16: preset select LSB; follows ctlBankH and ctlBankL.	9.70 [COVID19]
								//		 NB: Preset numbers start from 1 for internal catpre,
								//			 they are offset by 1 from progChgF/E Midi encoding.
								//		 NB: tx only ctlBankH = 127 catSSlot, 126 catEBuf, or 0 catUser
		progChgE		= 0xce,	// ch15: only for ctlBankH = 127 catSSlot, 126 catEBuf, or 0 catUser;
								// 		 rx (never tx) for store into flash slot.
		progChg1		= 0xc1,	// ch2:  rx (never tx) from user, alias for progChgF		 9.94
		progChg0		= 0xc0,	// ch1:  tx (never tx) for other synths at preset load (user-defined)
	chanPres 		= 0xd0,		// channel pressure (1 data byte); used by MPE Z encode and cData
		cData			= 0xdf,	// ch 16 chanPres: data stream for Continuum 				 5.40 [BigPine]
								//   - preceded by ctlStream stream identifier 				 6.16 [NAMM]
								//   - data stream terminated by ctlStream   				 8.81
								//	 - our MPE+ uses chanPres for Z [except ch 16 (cData) or ch 1 (user)]
								//     with ctlFrac supplying optional lsb
		ini_dB0			= 96,	// biqGraph amps: 0dB value in first biqGraph graph 		 5.61
		off_dB0			= 48,	// biqGraph amps: 0dB in mode offset graphs        			 5.61
	pitchWheel 		= 0xe0,		// pitch wheel (pitch bend) 14 bits follow for normal Midi,
								// we sometimes send additional 7 bits with preceding ctlFrac
		bendMax		= 0x3fff,	// max for encoding 14-bit normal Midi PitchWheel
		extBendMax 	= 0x1fffff,	// max for encoding 21-bit extended Midi PitchWheel using ctlFrac
		bendOffset 	= 0x2000,	// offset for encoding 14-bit normal Midi PitchWheel 
		#define inv_bendOffset    .0001220703125
		extBendOffset=0x100000,	// offset for encoding 21-bit extended PitchWheel (using ctlFrac)
		#define inv_extBendOffset .000000953674316

	// System Messages.
	sysMes 			= 0xf0,		// system messages   
	SysEx 			= 0xf0,		// sysex for old Preset files: now use cData  				 5.40 [BigPine]
	MidiTimingCode	= 0xf1,		// Midi Time Code quarter-frame message (together with Time Code SysEx)
	SongPosPtr 		= 0xf2,		// 2 bytes of data follow
	SongSelect 		= 0xf3,		// 1 byte of data follows
	//				= 0xf4,		//
	//				= 0xf5,		//
	TuneRequest	 	= 0xf6,		// retired touchCEE; no data follows						 7.72
	SysExEnd 		= 0xf7,		// end system exclusive; see also cDataEnd 					 5.40 [BigPine]
	TimingCk 		= 0xf8,		// Midi Clock: message 1 of 4 (real time, 24 per quarter note, no data)
	MidiTick		= 0xf9,		// 10 ms clock (rarely used, not part of Midi Clock)
	StartSeq 		= 0xfa,		// Midi Clock: message 2 of 4 (Midi Time Code is different, uses SysEx)
	ContinueSeq 	= 0xfb,		// Midi Clock: message 3 of 4 
	StopSeq 		= 0xfc,		// Midi Clock: message 4 of 4 
	//				= 0xfd,		// [undefined]
	actSense 		= 0xfe,		// active sensing (Continuum no longer transmits, as of 8.11)
	ResetAllReceivers = 0xff,

	// Masks.
	realTimeMask 	= 0xf8,		// real-time Midi bytes have these bits set
	statFlag 		= 0x80,		// top bit of status nybble indicating a status byte   
	statMask 		= 0xf0,		// status nybble   
	chanMask 		= 0x0f,		// channel nybble  

	// System controllers.
	ctlSoundOff		= 120,		// All Sound Off (stops playing all notes on channel)
	ctlDefault	 	= 121,		// All Controllers Default Value channel mode message
	ctlLocal		= 122,		// local control on/off
	ctlAllOff	 	= 123,		// All Notes Off channel mode message (keeps playing if sus pedal)
	ctlOmniOff		= 124,
	ctlOmniOn		= 125,
	ctlMonoOn		= 126,		// mono on (poly off)
	ctPolyOn		= 127,		// poly on (mono off); redefined by MPE
	ctlMpe			= 127,		// ... taken over by MPE: data is polyphony, or 0 to end MPE

	// Channel number.
	mpeMastChan		= 0,		// channel 1 is MPE's default "master" channel
								// channel 1 is also Utility channel for ContinuumUtility
	pChan			= 0,		// channel 1 pedals: tx always, rx *with* echo to Editor	 8.67
		ctlChgP = ctlChg + pChan,
	mpeFirstChan	= 1,		// channel 2 is MPE's default first channel for notes
	rChan			= 1,		// channel 2 pedals: relative rx *with* echo absolute		 8.67
								// channel 2 cc32/cc0/progChg: user preset select			 9.94
		ctlChgR = ctlChg + rChan,
	suChan	 		= 15,		// channel 16 cc (tx and rx): general Continuum setup
		ctlChgSU = ctlChg + suChan,
	emChan			= 14,		// channel 15 cc (tx and rx): EaganMatrix setup
		ctlChgEM = ctlChg + emChan,
	kentonChan		= 12,		// channel 13 cc (tx and rx): Kenton Killamix controller
};


// Midi controller numbers.   
//		** marks controllers used by Continuum only on channel 16 (general config).
//		$$ marks controllers used by Continuum only on channel 15 (EaganMatrix setup).
//		*$ marks controllers used for Editor rx and tx on ch16 [tx on ch1 if MPE+]
//					and rx ch1 echoed to Editor on ch16 [ch1 MPE+]
//					and rx relative value ch2 made absolute and echoed ch16 [ch1 MPE+].
//		## marks controllers used by Continuum on channels 1-n, where n is polyphony up to 16.
// For a list of controllers assignable to pedals or Kenton rotaries, see routine pedVal().
enum {



//  -------------- Continuum-specific controllers valid on all midi channels.  ----------------------------
	ctlMod			= 1,		// ## modulation depth		[assignable to y and z in Editor]
	ctlBreath		= 2,		// ## breath				[assignable to y and z in Editor]
	ctlUndef		= 3,		// ## undefined 			[assignable to y and z in Editor]
	ctlFoot			= 4,		// ## foot pedal			[assignable to y and z in Editor]
	ctlVol			= 7,		// ## volume 				[assignable to y and z in Editor]
	ctlExpres		= 11,		// ## expression			[assignable for y in Editor, default for z]
	ctlMpeZmsb		= 70,		//    14-bit MPE: Z msb; we transmit chanPres but also accept this as input
	//ctlMpeZlsb	= 102,		//    14-bit glitchy MPE:14-bit MPE: Z lsb [MPE+: use ctlFrac instead]
	ctlBrightness	= 74,		// ## brightness/cutoff 	[default for y]
	//ctlMpeYlsb	= 106,		//    14-bit glitchy MPE: brightness lsb [MPE+: use ctlFrac instead]
	ctlFracPed      = 86,		// ## 7' lsb for next i,ii,iii,iv,v,vi,jack1,jack2 [modified 8.81]
	ctlFrac			= 87,		// ## 7' lsb for next bend/y/z, or keyOn is ch1Note [modified 7.44, 8.24]
	ctlMultiLow		= 88,		// ## multi split: low nn for channel   3.38
	ctlMultiHigh	= 89,		// ## multi split: high nn for channel  3.38


//  -------------- Continuum-specific controllers on channel 16. General configuration. -------------------
	ctlBankH		= 0,		// ** category select; ctlBankL and progChgF/E follow.		 9.70 [COVID19]
								//    Also available for rx on channel 2 (user equipment).	 9.95
								//	  Categories 0..125 are intended for "general use,"
								//                      including local preset selection.
								//	  Categories 126, 127 are for Editor and program interfaces.
		catUser 		= 0,	//	  User Preset Slots are category 0; other categories ctlBankH 1..cats
								//		NB: Preset numbers start from 1 for internal "catpre" encoding,
								//			they are offset by 1 from progChgF/E Midi encoding.
		catEBuf 		= 126, 	//	  Only for Midi tx and rx, not used internally: Preset Edit Buffer.
								//  	NB: Internally, catUser with userPreset0 is used instead
								//	    NB: For Editor highlighting, source preset slot encoded as follows:
								//			0 no source preset slot ("from disk" or similar state)
								//			1..128 userSlot is source preset
								//			129+ sysSlot is source preset
		catSSlot 		= 127,	//    Only for Midi tx and rx, not used internally: sysSlot#
	//ctlMod		= 1,		// ## modulation depth		[assignable to y and z in Editor]
	//ctlBreath		= 2,		// ## breath				[assignable to y and z in Editor]
	//ctlUndef		= 3,		// ## undefined 			[assignable to y and z in Editor]
	//ctlFoot		= 4,		// ## foot pedal			[assignable to y and z in Editor]
	// 				= 5,		//    was ctlMute until 9.81
	ctlDataHi		= 6,		// ** data entry  tx: display, log, MPE bend range [7.39 Fruita]
		hiSensNo		= 0x00,	//    debug display: 2/0,5/ msbs of sensor number
		loSensNo_norm	= 0x20,	//    debug display: 2/1,5/ lsbs of sens# using normalized display
		loSensNo_dual	= 0x40,	//    debug display: 2/2,5/ lsbs of sens# using dual-rail display
		newScanHalf 	= 0x7e,	//    debug display: start of new scan for half-size fingerboard
		newScanFull		= 0x7f,	//    debug display: start of new scan for full-size or slim70
		newLogDump  	= 0x7d,	//    log dump: start of log dump over traditional Midi		
	//ctlVol		= 7,		// ## volume 				[assignable to y and z in Editor]
	ctlOctShift		= 8,		// *$ octave pedal  60=normal, 48/72=oct down/up  			 7.75 [JoshuaT]
	ctlMonoSwitch	= 9,		// *$ mono footswitch, enable mono interval  			1.71,1.84,3.21,6.15
	ctlFineTune		= 10,		// *$ fine tune  4..124 = -60..+60 cents   			4.32,6.15,9.08[COVID19]
	//ctlExpres		= 11,		// ## expression		 	[assignable for y in Editor, default for z]
	ctlI			= 12,		// *$ value for macro controller "i"   				 3.08, 6.15
	ctlII			= 13,		// *$ value for macro controller "ii"				 3.08, 6.15
	ctlIII			= 14,		// *$ value for macro controller "iii"				 3.08, 6.15
	ctlIV			= 15,		// *$ value for macro controller "iv"				 3.52, 6.15
	ctlV	 		= 16,		// *$ value for macro controller "iv" aka gen1		 9.74
	ctlVI	 		= 17,		// *$ value for macro controller "vi" aka gen2		 9.74
	ctlPost 		= 18,		// *$ samples generation output level after Master Section
	ctlAudIn		= 19,		// *$ audio in level  										 4.31, 6.15
	ctlReci1		= 20,		// *$ recirculator dial 1 							6.14[BigPine],6.15,7.35
	ctlReci2		= 21,		// *$ recirculator dial 2 (Midi pan position)		6.14[BigPine],6.15,7.35
	ctlReci3		= 22,		// *$ recirculator dial 3, in nn units (Midi balance) 		 6.15, 7.35
	ctlReci4		= 23,		// *$ recirculator dial 4  									 3.10, 7.36
	ctlReci5		= 24,		// *$ recirculator dial 5 (Mix) 							 3.10, 7.36
	ctlRoundRate 	= 25,		// *$ round rate  											(renumber 4.33)
	ctlPre 			= 26,		// *$ samples generation output level before Master Section
	ctlAtten		= 27,		// *$ output attenuation, only saved on Slim  				 9.10 [COVID19]
	ctlRndIni		= 28,		// *$ Round Initial
	ctlJack1		= 29,		// *$ kf_Midi's value from Pedal Jack 1 or Midi in 			 7.73,8.28,8.81
	//    NB: kentonChan uses cc30 and cc31 (ctlKAssign1)
	ctlJack2		= 30,		// *$ kf_Midi's value from Pedal Jack 2 or Midi in 			 7.73,8.28,8.8
	ctlAdvance		= 31,		// *$ 127 = next upreset, 64 = next layer					 7.70 [HockngH]
	ctlBankL		= 32,		// ** optional preset select MSB; between ctlBankH and progChgF/E
								//    Also available for rx on channel 2 (user equipment).	 9.95
	ctlActAes		= 33,		// ** select global options Action and aestx clock			 4.33
		bAction			= 0x03,	//	  bits to select action
		  ActnMiniClassic = 0,	//	  pre-CF600 or ContinuuMini
		  ActnMedium	  = 1,	//	  medium action (thresholds same as Classic)
		  ActnLight		  = 2, 	//	  light action for wide intervals
		  ActnLightN	  = 3, 	//	  light action for narrow intervals						 8.55
		bAes			= 0x0C,	//	  bits to select aestx
		  AES96k	 	  = 0,	//    aestx at standard 96k rate, internally clocked
		  AEShouse 		  = 1,	//    use aes3 house clock (aesrx clock) for xmit, if aesrx available
		  AES48k 		  = 2,	//    aestx at 48k clock rate, internally clocked
		  AES108k		  = 3,	// 	  this cannot be user-configured; used for CEE only   5.55
		bBigFontPop		= 0x10,	//	  large popup menus used in Editor (7.21 for after 7.3 release)
		//				= 0x20,	//    [8.23 was "Recirculator HP Only; retired 9.30 COVID19]
		bNoRecirc		= 0x40,	//	  recirculator "Disabled"  5.01, 5.12
	ctlAlg			= 34,		// ** Algorithm 0..18 for specialty, 19 for EaganMatrix.   7.77  
		//				= 0x60	//	  available bits
	ctlProgram 		= 35,		// ** Midi program number; 0 none, 1..20 (0..19)   [6.12]
	ctlRouting		= 36,		// ** data must be same order as config struct and context definition order
		bSurfaceTrad	= 0x01,	//	  bit routes playing surface to Midi Out
		bSurfaceDsp		= 0x02,	//	  bit routes playing surface to internal sounds
		bSurfaceCvc		= 0x04,	//	  bit routes playing surface to CVC
		bMidiInTrad		= 0x08,	//	  bit routes Midi In to Midi Out
		bMidiInDsp		= 0x10,	//	  bit routes Midi In to internal sounds
		bMidiInCvc		= 0x20,	//	  bit routes Midi In to CVC
		defaultRoute    = 63,	//	  default is to set all routing bits
	//					= 0x40	//	  bit is not used yet
	ctlPedType		= 37,		// ** type of pedals at pedal jacks, Kenton type (read only)
		bPedType0		= 0x07,	//	  bits specify type of pedal in jack 0, values below  [3bit 7.76]
		bPedType1		= 0x38,	//	  bits specify type of pedal in jack 1, values below
		sPedType1		= 3,	//	  shift value for bPedType1
		  unPlugged 	  = 0, 	//    no pedal plugged in
		  isSwitch		  = 1, 	//    Yamaha FC4/FC5   (two-value switch: 0 or 127)
		  isExpr		  = 2, 	//    Yamaha FC7 pedal (continuous-valued)
		  isDamper		  = 3,	//    Yamaha FC3 pedal (continuous-valued)
		  isTri		 	  = 4,	//	  Linemaster 476-S	   (TriValue switch: 0, 64, 127)	       [7.76]
		  isCV			  = 5,	//    Mini control voltages (Eurorack -2.5v..+2.5v via special cable) 8.68
		  isPot			  = 6,	//	  Slim Continuum pot (continuous-valued)
		bKenton		= 0x40, 	//	  Kenton Mini Controller detected 				[2bit until 7.76]
	ctlDataLow 		= 38, 		// ** log dumps, debug disp, cvc verify, custom tuning [re-add 6.19]
		maxGridEntries 	= 128,	//    tuning grid: fills one flash page [6.19 Gulf SP, AL]
		ctlsPerGridPoint= 6,	//    tuning grid: number of ctlDataLow messages per grid point
	ctlPoly		 	= 39,		// ** polyphony 
		baseBits		= 0x1f,	//	  bits specify base polyphony, values 1..16
		//				= 0x20,	//
		expPolyBit		= 0x40,	//	  bit set in ctlPoly if polyphony may expand for CEE   7.04
		maxMpePoly		= 13,	//	  max poly for MPE: not notes on mpeMastChan, emChan, suChan
								//	  NB: useChanPr uses maxMpePoly (real max poly maxMidiPoly-1?) 7.56  
		maxMidiPoly		= 16,	//    number of Midi channels limits number of voices
		emVoices		= 8, 	//	  max EaganMatrix voices computed on one dsp (all DSP versions!)
		maxPoly			= 24, 	//	  L6x has 8 voices per dsp; so does C3x, C4x, C5x   [8.81]
	ctlBendRange 	= 40, 		// ** bend range
		mpeMinBendRange	= 12,	//	  MPE minimum bend range   7.39 [Fruita]
		defBendRange	= 96,	//	  default bend range is max; good for surface display in Editor
		maxBendRange 	= 96,	//    max bend always used internally independent of ctlBendRange 
		#define ch1Bend(b) MAX(1,(b)-maxBendRange+1) //	> 96 selects MPE+ ch1's bend range  8.60
		#define inv_maxBendRange 0.0104166667
	ctlFrontBack 	= 41, 		// ** select controller number for front-back position
		xmitNoY			= 0,	//    do not transmit y info
		xmitNoShelf		= 127,	//	  avoid shelf on y, use defaultYcc
		#define defaultYcc ctlBrightness
	ctlPressure 	= 42, 		// ** select controller for finger pressure; 127 to use channel pressure
        xmitNoZ		 	= 0,	//    do not transmit z info [special value 127 chanPres until 5.40]
		defaultZcc      = 11,	//	  _DSP, _CVC, and snbn encoding for Z (this was ctlVol until 3.00)
		useChanPr		= 69,	//	  use channel pressure to encode Z (but do not use MPE)   7.56
		useMPEplus		= 70,	//|   MPE+: our 14-bit MPE, externally remap channels to n+1 [7.44]
								//	  chanPres, cc74 (Ymsb), cc87 (ctlFrac)	8.59 [Paris]
								//	  MPE+ data on ch1 channelized to DSP, supported by psCh1Note.    8.60
		useMPE		 	= 127,	//    MPE: channel pressure for Z, externally remap channels to n+1 [7.44]
	ctlNSplit		= 43,		// ** data bits for this must match order in config structure
		bNoteMode	 	= 0x07,	//    bit selects keyOn/keyOff processing for Midi out   5.12
		  MStatic		  = 0,	//	  always use Midi key velocity 127
		  MVeloc		  = 1,	//	  encode velocity in Midi keyOn message
		  MFormula		  = 2,	//	  encode formula V for velocity in Midi keyOn message  7.35
		  MNoNote		  = 3,	//	  do not output keyOn,keyOff,bends -- for CVC+Midi control of Voyager
		  MMidC			  = 4,	//	  use nn 60 and static velocity for all notes (Moog Theremin)  7.44
		  MAnnounce		  = 5,	//	  announce continuum presence for shall-not-be-named
		bSplitMode		= 0x78,	//	  bits select split mode 
		  SplitOff 		  = 0,	//    surface split disabled, and not a combination preset
		  SplitPoly 	  = 1,	//    equal number of channels below and above split point
		  SplitMonoL 	  = 2,	//    single channel (channel 0) below split point, all other chans above
		  SplitMonoH 	  = 3,	//    single channel (channel 0) above split point, all other chans below
		  SplitIntL 	  = 4,	//    internal sound below, all Midi above.
		  SplitIntH 	  = 5,	//    internal sound above split, all Midi below
		  SplitTArea	  = 6,	//	  Touch Area Extends Throughout Split Area						  9.28
		  SplitBelow 	  = 7,	//    Combination: 1st Preset Below Split, 2nd Preset Above Split 	  8.60
		  SplitAbove 	  = 8,	//    Combination: 1st Preset Above Split, 2nd Preset Below Split	  9.27
		  SplitFirst 	  = 9,	//    Combination: 1st Preset Uses Finger Data 						  8.60
		  SplitBoth		  = 10,	//    Combination: Both Presets Use Finger Data 					  9.27
		  SplitThree	  = 11,	//	  Combination: Three Presets Use Finger Data					  9.28
	ctlMiddleC 		= 44,		// ** middle C position; nnMiddleC = normal, 48 = sound octave lower
	ctlSplitPoint 	= 45, 		// ** note number for split [see also ctlMultiLow and ctlMultiHigh]
	ctlMonoFunc		= 46,		// ** mono function (type of transition)  [1.82, 5.13, 8.10]
		bMonoF		 	= 0x0F,	//    bits select mono function
		  MonoPortZ		  = 0,	//	  pressure-based mono functions (Mini uses MonoLegatoT instead 8.65)
		  MonoLegatoZ	  = 1,	//   								(Mini uses MonoLegatoT instead 8.65)
		  MonoRetrigZ	  = 2,	//									(Mini uses MonoLegatoT instead 8.65)
		  MonoLegatoT	  = 3,	//	  time-based mono functions, use most recent finger's Z
		  MonoRetrigT	  = 4,
		  MonoRetrigTL	  = 5,
		  MonoLegatoTZ	  = 6,	//	  time-based mono functions, use max finger's Z
		  MonoRetrigTZ	  = 7,
		  MonoRetrigTLZ	  = 8,
		  #define isRetrig(mf)  ((0x1b4 >> (mf)) & 1) 	// 110110100 8,7,5,4,2 are retrig  8.10
		  #define isRetrigL(mf) ((0x124 >> (mf)) & 1) 	// 100100100 8,5,2 are retrig on lift  8.10
		  #define isSmooth(mf)  ((0x04b >> (mf)) & 1)   // 001001011 6,3,1,0 are smooth transitions
	ctlReciCol		= 47,		// ** recirc columns 8.62 [DanielBoone] [ctlNsewMap til 8.28]
		bReciCol1		= 0x03,	//    select R1..R4 for Recirc column 1   6.14 [BigPine]
		bReciCol2		= 0x1C,	//    select R1..R5 for Recirc column 2   7.36
			rDial1=0,rDial2,rDial3,rDial4,rDial5, rDials,	// bReciCol bits select recirculator dial
	ctlMonoInterval = 48,		// ** set mono interval
	ctlPrio			= 49,		// ** [included roundInitial until 6.15]
		bOkIncComp		= 0x01,	//	  allow increased computation rate (doubleSR on !DSP364 only)
		//				= 0x02,	//	  
		bPrio			= 0x1C,	//	  bits select channel priority (aka "note priority")
		  LRU			  = 0,	//    least recently, used
		  LRR			  = 1,	//    least recently used with repeated pitch reuse  6.20
		  LCN			  = 2,	//	  lowest channel number
		  HIGH1			  = 3,	//    use only highest channel
		  HIGH2			  = 4,	//	  use only 2 highest channels
		  HIGH3			  = 5,	//    use only 3 highest channels
		  HIGH4			  = 6,	//	  use only 4 highest channels
		bToggleImmed	= 0x60,	//	  ctlOctShift in "toggle" or "immediate" mode 	   4.39, 7.73 [Conifer]
	//				= 50,		// **												 [ctlCfgExt until 9.70]
	ctlGrid 		= 51,		// ** tuning: 0 default,  1-50 n-tone equal, 60-71 just
		equalTempered 	= 0,	//    0 is "normal" equal tempered scale
		oneToneEqual 	= 1,	//    1..50 select n-tone equal scales
		fiftyToneEqual 	= 50,
		justC 			= 60,	//    60..71 select just tempered scales
		justB 			= 71,	//	  [80..87 downloadable tunings removed 6.13 Big Pine]
		firstUserTuning = 80,	//    80..87 are user grids [removed 6.13, re-add 6.19]
		lastUserTuning 	= 87,
		userTuningGrids = lastUserTuning - firstUserTuning + 1,
	ctlPedal1 		= 52,		// ** controller number for pedal jack 1; MINI: 0 means i2c instead
	ctlPedal2 		= 53,		// ** controller number for pedal jack 2
	ctlJackShift	= 54,		// ** amount to shift when pedal jack assigned to ctlOctShift
		jackShiftDefault = 48,	//    octave down is default (unfortunate default!)
	ctlPreserv		= 55,		// ** preserve and other global options
		bCfgOut			= 0x01,	//	  bit set to output config to Midi whenever it changes	5.40
		bPresEnc		= 0x02,	//    preserve Midi encoding (Bend, Ycc, Zcc, routing)  6.16
		bPresPed		= 0x04,	//	  preserve Pedal assignments (Ped1,Ped2,jackShift,min,max)  6.16
		bPresSurf		= 0x08,	//	  preserve surface processing (rounding, split, mono, etc)  6.16
		bNoLongerUsed	= 0x70,	//	  									   [to Line9 of midiports.txt 9.70]
	ctlStream		= 56,		// ** identify stream of cData bytes 						6.16 [NAMM]
		cDName=0,cDConText,cDGraph,cDGraphO1,cDGraphO2,cDGraphT0,cDGraphT1,
		cDLog=7,cDUpdF2,cDDemoAssort,cDFloat,cDKinet,cDBiqSin,cDSys,cDConv,
		cDataEnd = 127,			//    end of cData sequence    								8.81 [WarrenD]
	//				= 57,		// ** 												 [ctlFlashL until 9.70]
	//				= 58,		// ** 												 [ctlFlashH until 9.70]
	ctlDimMenu		= 59,		// ** dim and 4-button menu options for Mini and Slim Continuum;
								//    only sent, never received - detects loopback			9.08 [COVID19]
		bDim			= 0x07,	//    Mini dim = -6,-12,-18,-24dB
		bPedExt			= 0x18,	//	  set to use Mini's ped/ext jack for pedal / i2c / cv	8.59
			pedX=0,i2cX,cvX,	//	  bPedExt field values: jack is for pedal / i2c / cv		8.68
		bBrD			= 0x60,	//	  Mini display brightness
	ctlTArea	 	= 60, 		// ** note number for center of Touch Area; < nnLowFull if disabled  7.76
		nnTWid		= 2,		//    +/- width of tarea area (unless extended by split SplitTArea)
		nnTOff		= 16,		//    less or equal nnTOff means no Touch Area defined (use All Fingers)
		nnTMax		= 108,		//    highest Touch Area value
	ctlRvRo			= 61,		// ** data bits for this must match order in config structure
		bPitchReverse	= 0x01,	//	  bit selects reverse pitches
		bRKind			= 0x06,	//	  normal/release/viaY rounding   7.46 [Christmas 2015 TGG, Big Pine]
		  rNormal=0,rRelease,rViaY,rViaYinv,
		bAvailNow		= 0x38,	//
	ctlReciType		= 62,		// ** recirculator type, extend  [7.36, 7.44 moved & modified]
		R_rev = 0, 									// digital reverb [7.44]
		R_modDel,R_swepEcho, 						// 7.35 [Letchworth NY]
		R_anaEcho,R_digEchoLPF,R_digEchoHPF,		// 7.36
		R_mask = 7,									// mask for bits actually used
		bReciExt		= 0x40,	//	  extended recirculator time (delay line half sample rate)
	ctlCvcCfg		= 63,		// ** cvc standard voltage configuration (used if !is.EaganMatrix)
		cvcStdBits		= 0x07,	//	  cvc standard voltage range selection
		cvcZsqBit		= 0x08,	//	  cvc z squared select		7.44
		cvcLayerBit		= 0x10,	//	  cvc layer bit				7.44
		cvcBaseBits		= 0x60, // 	  cvc base voice bits		7.44
	ctlSus			= 64,		// *$ sustain pedal; down = b0 40 7f; up = b0 40 00  
	ctlRndEqual		= 65,		// *$ 0 = round disabled, 64 = rounding enabled,	
								//    127 = equal tuning with round 7.57, 7.70, 7.75
	//ctlNextTuning	=   ,		// *$ future? see also ctlRndEqual and Rob Schwimmer's VIWarp
	ctlSos			= 66,		// *$ sostenuto pedal, used as first sostenuto on Continuum
	ctlHpLevel		= 67,		// *$ headphone level  						8.12 [COVID19]
	ctlLineLevel	= 68,		// *$ line level  							8.12 [COVID19]
	ctlHold2		= 69,		// *$ hold2 pedal, used as second sostenuto 1.67
	ctlActuation	= 70,		//~** sensor scanning modifications			8.55, 9.31 [COVID19]
	ctlPolyTrad		= 71,		// ** Continuum out: total _TRAD polyphony  6.00
	ctlPolyDsp		= 72,		// ** Continuum out: total _DSP polyphony   6.00
	ctlPolyCvc		= 73,		// ** Continuum out: total _CVC polyphony   6.00
	//ctlBrightness	= 74,		// ## brightness/cutoff 	[default for y, assignable for z in Editor]
	ctlTest			= 75,		// ** for stress test of Midi to/from Editor  5.64  data is 64..127  
	ctlMin1			= 76,		// *$ minimum data value for pedal 1  4.15 (renumber 4.33) (sound ctlr 7)
	ctlMax1			= 77,		// *$ max data value for pedal 1      4.15 (renumber 4.33) (sound ctlr 8)
	ctlMin2			= 78,		// *$ minimum data value for pedal 2  4.15 (renumber 4.33) (sound ctlr 9)
	ctlMax2			= 79,		// *$ max data value for pedal 2      4.15 (renumber 4.33) (sound ctlr 10)
	ctlQBias		= 80,		// ** obsolete (saved but no longer does anything)			10.07
	//				= 81,		// ** 							[compLevel until 9.93] 										 9.80 [BigPine]
	//				= 82,		// ** 							[compTime until 9.93]
	ctlEqTilt		= 83,		// *$ Eq Tilt, 64=flat										 9.10 [COVID19]
	ctlEqFreq	    = 84,		// *$ Eq Frequency   0..127 = 120Hz..15kHz					 9.10 [COVID19]
	ctlEqMix		= 85,		// *$ Eq Mix, 0=dry, 127=wet								 9.10 [COVID19]
	//ctlFracPed    = 86,		// ## 7' lsb for next i,ii,iii,iv,v,vi,jack1,jack2 			 8.81
	//ctlFrac		= 87,		// ## 7' lsb for next bend/y/z, or keyOn is ch1Note   		 7.44, 8.24
	//ctlMultiLow	= 88,		// ## multi split: low nn for channel (write only, any Midi channel)  3.38
	//ctlMultiHigh	= 89,		// ## multi split: high nn for channel (write only, any Midi channel) 3.38
	ctlCompThr		= 90,		// *$ Compressor Threshold									 9.93
	ctlCompAtk		= 91,		// *$ Compressor Attack			[ctlTweak until 9.70]		 9.93
	ctlCompRat		= 92,		// *$ Compressor Ratio			[ctlTweakAreaLo 4.35..9.70]	 9.93
	ctlCompMix		= 93,		// *$ Compressor Mix			[ctlTweakAreaLo 5.29..9.70]	 9.93
	//				= 94,		// ** 		 (not currently isCtlSetup()/countInArchive(), ok for pedList[])
	//			    = 95,		// ** 		 (not currently isCtlSetup()/countInArchive(), ok for pedList[])
	//				= 96,		// ** 		 (not currently isCtlSetup()/countInArchive(), ok for pedList[])
	//			    = 97,		// ** 		 (not currently isCtlSetup()/countInArchive(), ok for pedList[])
	ctlNrpnlsb  	= 98,		//    MPE+: low 7' nrpn select								 9.08 [NAMM]
	ctlNrpnMsb  	= 99,		//    MPE+: high 7' nrpn select								 9.08 [NAMM]
	ctlRpnLsb		= 100,		//    MPE: low 7' selecting function of data entry			 7.38 [Fruita]
	ctlRpnMsb		= 101,		//    MPE: low 7' selecting function of data entry			 7.38 [Fruita]
		// cc102 through cc119: Not stored in config.
	ctlVersionHigh 	= 102,		// ** firmware version top 7 bits (read only)
	ctlVersionLow 	= 103,		// ** firmware version low 7 bits (read only)
	ctlCVCHigh 		= 104,		// ** 5/hardw,2/cvcSerialMsb (bits 15..14) (read only)
		hw_fL=1,hw_hL=2,		//    hardw field: light action full/half, not UP version    8.84
		hw_fC=3,hw_hC=4,		//	  hardw field: classic action full size and half size    8.84
		hw_Mini=5,hw_o49=6,		//	  hardw field: ContinuuMini and Osmose					 8.84
		hw_s22=7,hw_s46,hw_s70,	//	  hardw field: slim 22L6x, slim 46L6x, slim70L6x		 9.08
		hw_EMM=10,				//	  hardw field: Eurorack EaganMatrix Module				 9.70 [COVID19]
		hw_fU=11,hw_hU=12,		//    hardw field: light full/half, UP triple-dsp board      8.84
	ctlCVCMed 		= 105,		// ** cvc serial number med 7 bits (bits 13..7) (read only)
	ctlCVCLow 		= 106,		// ** cvc serial number low 7 bits (bits 6..0) (read only)
	ctlCont 		= 107,		// ** for Shall-not-be-named: once per second (read only)	 3.26, 3.39
		snbnModeBit		= 0x40,	//    data is polyphony, plus top data bit indicates ctlSNBN received	
	//				= 108,		// ** 											  [ctlUPreset 7.78 to 8.70]
	ctlDownload		= 109,		// ** download firmware/data/tuning
		// cc109 and cc110 use Editor's Message Bar: C = Center Display, L = Left Display, M = Max Log
		// NB: messages marked with C or L below sent using report() or postReport() 		 5.60
		// NB: "download" is firmware or data, "archive" is presets; 
		//     despite separate messages for the two cases here, Editor may use same text for both.
		// NB: ctlDownload not stored within a preset-on-disk's data; but some ctlDInfo are.
		downloadOkBoot 	= 0,	// C  ask user to turn off and back on again				 9.31 [COVID19]
		downloadFail 	= 1,	// C  center display: "Operation Failed.  Try again."
		downloadOkChained=2,	// C  center display: n/c (leave up "Download in Progress")  5.60
		//				= 3,	// LC available
		downloadOkData	= 4,	// C  center display: "Data download completed."
		archiveOk		= 5,	// C  center display: erase message bar; sent at very end of Preset Group
		archiveFail		= 6,	// C  center display: "Operation Failed.  Try again." ****
		queryKenton		= 7,	//    Editor will respond with ctlKHello to say if a Kenton is present
		curGloToFlash	= 8,	//    From Editor: write global/calib/sensorMap/current to flash	5.12
								//	  Special case if Archive Retrieve: end of Archive Retrieve
		reduceGain		= 9,	// C  center display: "Reduce Gain"
		reducePoly		= 10,	// C  center display: "Reduce Polyphony"
		inFactCalib		= 11,	// C  center display: "Factory Calibration In Progress" [modified 6.38]
		eraseMessage	= 12,	// LC erase message bar
		aesSyncFail		= 13,	// C  center display: "AES3 Sync Failure"  					 5.40
		cvcPower		= 14,	// C  center display: "Turn On or Disconnect CVC" 			 5.40
		ceeMismatch		= 15,	// C  firmware version mismatch between CEEs				 5.60
		configToMidi	= 16,	//    current config to Midi								 5.21 again
		startFirmware 	= 17,	//    begin firmware download
		startData		= 18,	//    begin synthesis data download
		burnUser364 	= 19,	//    done with firmware 21364 download, burn user flash (if crc matched)
		endDataDownload = 20,	//    end data firmware/data download
		midiLoopback	= 21,	// C  Midi-loopback-detected error message
		//				= 22,	//
		//				= 23,	//
		begTxDsp		= 24,	// 	  begin CEE config send txDsp (from daisy=1 to 2,3)  	 5.60
		endTxDsp		= 25,	// 	  end CEE config send txDsp (from daisy=1 to 2,3)  		 5.60
		doneTxDsp		= 26,	// 	  handshake back: end of txDsp preset-sending process
		txDspFail		= 27,	// ?  config send txDsp failure - could try again?  logged in Max window
		doUpdF2			= 28,	// C  after Update File 1 reboot, tell user to do Update File 2
								//	  This is used by Osmose.
		createLed		= 29,	//	  turn on yellow LED for archive create
		testBegin		= 30,	//    Editor to Continuum: begin Midi stress test
		testErr			= 31,	// ?  Continuum to Editor: error in Midi rx sequence
		userToMidi		= 32,	//    from Editor: preset names to Midi, then current config 6.19
		manualUpdate	= 33,	//    old preset needs manually-implemented update 			 6.20
		doResetCalib	= 34,	//	  set zero levels, like optCalib+0						 6.38
		doRefineCalib	= 35,	//	  incorporate new zero levels, like optCalib+1 			 6.38
		midiTxFull		= 36,	//	  full midi transmission rate							 6.41
		midiTxThird		= 37,	//	  one-third midi transmission rate 						6.41,7.57,8.55
		midiTxTweenth	= 38,	//	  one-twentieth midi transmission rate 					6.41,7.57,7.59
		sysToMidi		= 39,	//	  from Editor: preset names to Midi, then current config 6.19
		endSysNames		= 40,	//	  to Editor: end list of system presets				  	 9.70
		doFactCalib		= 41,	//    do factory calibration
		doUpdate1		= 42,	// C  after recovery boot, let user know ready for update file 1
								//	  This is used by Osmose to detect Recovery Mode.
		burnUser489 	= 43,	//    done with firmware 21489 download, burn user flash (if crc matched)
		dspReboot		= 44,	//	  reboot after Firmware File 1							 7.56
		surfAlign		= 45,	//	  toggle slim Continuum's Surface Alignment mode 		 9.08
		addTrim			= 46,	//	  add currently-playing finger to Trim array  			 7.57
		delTrim			= 47,	//    remove trim point closes to currently-playing finger   7.57
		resetTrim		= 48,	//    remove all trim data  								 7.57
		beginSysNames	= 49,	//	  to Editor: begin list of system presets 			 	 9.70
		exitCombi		= 50,	//	  exit Combination Preset mode: edit all DSPs
		miniFactSetup	= 51,	//	  store calib/global/userPresets to continuuMini factory setup
		decPreset		= 52,	//	  to prev sysPreset						  [tx7.78, rx8.59 SuperBooth]
		incPreset		= 53,	//	  to next sysPreset						  [tx7.78. rx8.59 SuperBooth]
		beginUserNames	= 54,	//	  to Editor: begin list of user presets 			 	 7.78
		endUserNames	= 55,	//	  to Editor: end list of user presets				  	 7.78, 9.70
		sameSlot		= 56,	// C  save Combi preset to same slot or to disk 			 8.11
		preEraseUpdF1	= 57,	//    pre-erase flash Update File 1; echo when dsp1 done	 9.74
		preEraseUpdF2	= 58,	//    pre-erase flash Update File 2; echo when dsp1 done	 9.74
		preEraseUpdF3	= 59,	//    pre-erase flash Update File 3; echo when dsp1 done	 9.74
		remakeSRMahl	= 60,	//	  remake synthesis-ready Mahling data					 9.83
		doneFactProg	= 61,	//	  done copying to/from Factory Program Board 			 9.91
		failFactProg	= 62,	//	  failed copying to/from from Factory Program Board 	 9.91
		usbTxNoAck		= 63,	// M  Usb-Midi out from Mini did not get Ack				 8.70
		rxOver 			= 64,	// M  Midi rx queue overflow								 8.50
		txOver 			= 65,	// M  Midi tx queue overflow								 8.50
		rxSynErr 		= 66,	// M  Midi rx syntax error									 8.50
		rxBitErr 		= 67,	// M  Midi rx bad bit widths								 8.50
		sensComm		= 68,	// M  serial sensors errors									 8.50
		nanErr			= 69,	// M  output has nan										 8.50
		ceeSeq			= 70,	// M  CEE comm glitch										 8.50
		burnUserMini	= 71,	//	  ContinuuMini firmware has this at end
		doMidiLog0		= 72,	//	  end scrolling ascii log via Midi						 7.40,8.81
		doMidiLog1		= 73,	//	  daisy=0,1 scrolling ascii log via Midi				 7.40,8.81
		doMidiLog2		= 74,	//	  daisy=2 scrolling ascii log via Midi 					 7.40,8.81
		doMidiLog3		= 75,	//	  daisy=3 scrolling ascii log via Midi 					 7.40,8.81
		burnRecovery489 = 76,	//    done firmware 21489 download, burn recovery (done at factory only!)
		burnRecovery364 = 77,	//    done firmware 21364 download, burn recovery (done at factory only!)
		burnRecoveryMini= 78,	//    done firmware ContinuuMini download, burn recovery
		//				= 79,	//
		defFirstTuning	= 80,	//    80..87 begin loading user tuning grid; data follows using ctlDataLow
			// 80..87 are for downloading user tuning grids					   [removed 6.13, re-add 6.19]
		defLastTuning 	= 87,	//    80..87 begin loading user tuning grid; data follows using ctlDataLow
		numDecMat		= 88,	//	  decrement numeric matrix point						 9.70
		numIncMat		= 89,	//	  increment numeric matrix point						 9.70
		mendDisco		= 90,	//	  mend discontinuity at note (outlier to Sensor Map)	 9.74
		rebootRecov		= 91,	//    reboot in Recovery Mode								 9.81
		stageUp			= 92,	//	  host to dsp: upload monolithic update					 9.82
		stageDown		= 93,	//	  host to dsp: download monolithic update				 9.82
		stageDownOk1	= 94, stageDownOk2, stageDownOk3,		// dsp to host at end of StageDown
		stageDownFail1	= 97, stageDownFail2, stageDownFail3,	// dsp to host at end of StageDown
		rebootUser		= 100,	//    reboot in User (normal) Mode							 9.82
		gridToFlash 	= 101,	//    save tuning grids to flash
		mendDivided		= 102,	//	  mend divided note (add flawed sensor to Sensor Map)	 9.74
		startUpdF2		= 103,	//    from Editor: beginning of Update File 2				 9.70 [COVID19]
		notFirst		= 104,	//	  to Editor: not first preset in a combination			 9.70 [COVID19]
		firstOfTwo		= 105,	//	  to Editor: first preset in a dual combination			 9.70 [COVID19]
		firstofThree	= 106,	//	  to Editor: first preset in triple combination			 9.70 [COVID19]
		Demo1			= 107,	//	  Demo Assortment to first set-of-16 user presets.		 9.70 [COVID19]
			// 107..114 are for loading demo assortment into user presets
		Demo8			= 114,	//	  Demo Assortment to last set-of-16 user presets.		 9.70 [COVID19]
		Empty1			= 115,	//	  Empty preset to first set-of-16 user presets.			 9.70 [COVID19]
			// 115..122 are for emptying set-of-16 user presets
		Empty8			= 122,	//	  Empty preset to last set-of-16 user presets.			 9.70 [COVID19]
		burnUser593 	= 123,	//    done with firmware 21593 download, burn user flash (if crc matched)
		burnRecovery593 = 124,	//    done firmware 21593 download, burn recovery (done at factory only!)
	ctlDInfo			= 110,	// ** for Continuum Editor: additional info for download;
		// cc109 and cc110 use Editor's Message Bar strip: C = Center Display, L = Left Display
		// NB: ctlDownload not stored within a preset-on-disk's data; but some ctlDInfo are.
		profileEnd		= 0,	// LC erase message bar, Editor puts up Save dialog
		percentFirst	= 1,	// L  left display "1%"
		percentLast		= 99,	// L  left display "99%"
		cfCreateArch0	= 100,	//    Save Preset: received from Editor, start *creation* archive  5.12
								//	  value 100 is preset edit buffer (userPreset0)
		cfCreateArch1	= 101,	//    Save Preset: received from Editor, start *creation* archive  5.12
								//	  value 101..116 is within set-of-16 user preset slots
		cfCreateArch16	= 116,	//    Save Preset: received from Editor, start *creation* archive  5.12
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
	ctlStatus		= 111,		// ** Continuum status info for Editor (read only)  3.47
		sLedBits		= 0x0f,	//    select led color bits
		  ledOff		  = 0,	//	  led color
		  ledBlue		  = 1,	//	  led color
		  ledRed		  = 2,	//	  led color				 part of config
		  ledBrightGreen  = 3,	//	  led color				 part of config
		  ledGreen		  = 4,	//	  led color				 part of config
		  ledWhite		  = 5,	//	  led color 			 calib		
		  ledYellow		  = 6,	//	  led color				 download, etc
		  ledPurple		  = 7,	//	  led color				 no aes sync, or flashing if cvc power off
		  ledBlueGreen	  = 8,	//	  led color  4.02		 [not used; retired from flawed sensor]
		sAesBits		= 0x70,	//    aes detected rate bits
		sAesShift		= 4,	//    shift for aes detected bits
		  aesInputNone	  = 0,	//	  no aes input stream
		  aesInputNonStd  = 1,	//	  aes input nonstandard rate
		  aesInput44p1	  = 2,	//	  aes input  44.1 kHz	4.15 [Ottawa]
		  aesInput48	  = 3,	//	  aes input  48.0 kHz
		  aesInput88p2	  = 4,	//	  aes input  88.2 kHz
		  aesInput96	  = 5,	//	  aes input  96.0 kHz
		  aesInput176p4	  = 6,	//	  aes input 176.4 kHz
		  aesInput192	  = 7,	//	  aes input 192.0 kHz
		  #define StdRate 44100,48000,88200,96000,176400,192000
	//				= 112,		// ** 												was ctlLastP until 9.70
	ctlSNBN 		= 113,		// ** Continuum input: Shall-not-be-named response to ctlCont          3.26
								//    this sets polyphony, multi-split, and Midi special encoding:
								//    high channel number hack, assume bend 96, default y z cc
	ctlPcntDisp 	= 114, 		// ** Tx: Processing percent used; Rx: Continuum monitor display mode
								//	  Tx: 32*dsp# + 4%units (0-25)							 9.08 [COVID19]
	ctlLogDump 		= 115, 		// ** begin or continue continuum log dump
	ctlEditor 		= 116,		// ** Continuum input: Continuum Editor is out there 				   3.33
	ctlLoopDetect	= 117,		// ** random pattern for loopback detect   also on ch1 MPE    8.62 [RoanMtn]
	ctlEditorReply	= 118,		// ** Respond to ctlEditor LightAct,doubleSR,3/ProcPow,SNBN,edCvcMatrix
		edsCvcMatrix	= 0,	//	  bit 0: cvc determined by EaganMatrix					 5.20,7.44,7.82
		edsSNBN			= 1,	//	  bit 1: Shall-not-be-named active
		edsProcPow		= 2,	//	  bits 2-4: 3-bit proccessingPowerMultiplier: 1x..6x, 7x DSP593    7.82
		  edmProcPow	= 0x1c,	//	  bits 2-4 mask
		edsDoubleSR		= 5,	//    bit 5: doubleSR active 8.24
		//edsLightAction= 6,	//	  bit 6: available (no longer used after 8.84, see ctlCVCHigh)
	ctlArchiveCount	= 119,		// ** count of controller messages in archive definition

	ctlCRC0			= 122,		// ** internal daisy=1 -> daisy=2,3 txDsp: 7 lsbs of config CRC		   6.20
	ctlCRC1			= 123,		// ** internal daisy=1 -> daisy=2,3 txDsp: next 7 bits of config CRC
	ctlCRC2			= 124,		// ** internal daisy=1 -> daisy=2,3 txDsp: next 7 bits of config CRC
	ctlCRC3			= 125,		// ** internal daisy=1 -> daisy=2,3 txDsp: next 7 bits of config CRC
	ctlCRC4			= 126,		// ** internal daisy=1 -> daisy=2,3 txDsp: 4 msbs of config CRC



// -------------- Continuum-specific controllers on channel 15. EaganMatrix configuration. ---------------
	//				= 0,		//
	//ctlMod		= 1,		// ## modulation depth		[assignable to y and z in Editor]
	//ctlBreath		= 2,		// ## breath				[assignable to y and z in Editor]
	//ctlUndef		= 3,		// ## undefined 			[assignable to y and z in Editor]
	//ctlFoot		= 4,		// ## foot pedal			[assignable to y and z in Editor]
	//				= 5,		//    
	//				= 6,		//    
	//ctlVol		= 7,		// ## volume 				[assignable to y and z in Editor]
	prLowFirst		= 8,		// *$ for deprecated echoed pedal updates to Editor on emChan  9.76
	//				= 9,		// *$ >>USE CH2 instead for these<<
	//				= 10,		// *$ for pedal updates echoed to Editor  5.40    
	//ctlExpres		= 11,		// ## expression 		 	[assignable for y in Editor, default for z]
	//				= 12,		// *$ for pedal updates echoed to Editor  5.40    
	//				= 13,		// *$ for pedal updates echoed to Editor  5.40    
	//				= 14,		// *$ for pedal updates echoed to Editor  5.40
	//				= 15,		// *$ for pedal updates echoed to Editor  5.40    
	//				= 16,		// *$ for pedal updates echoed to Editor  5.40
	//				= 17,		// *$ for pedal updates echoed to Editor  5.40
	//				= 18,		// *$ for pedal updates echoed to Editor  5.40
	//				= 19,		// *$ for pedal updates echoed to Editor  5.40
	//				= 20,		// *$ for pedal updates echoed to Editor  5.40
	//				= 21,		// *$ for pedal updates echoed to Editor  5.40
	//				= 22,		// *$ for pedal updates echoed to Editor  5.40
	//				= 23,		// *$ for pedal updates echoed to Editor  5.40
	//				= 24,		// *$ for pedal updates echoed to Editor  5.40
	//				= 25,		// *$ for pedal updates echoed to Editor  5.40
	//				= 26,		// *$ for pedal updates echoed to Editor  5.40
	//				= 27,		// *$ for pedal updates echoed to Editor  5.40
	//				= 28,		// *$ for pedal updates echoed to Editor  5.40
	//				= 29,		// *$ for pedal updates echoed to Editor  5.40
	//				= 29,		// *$ for pedal updates echoed to Editor  5.40
	//				= 30,		// *$ for pedal updates echoed to Editor  5.40
	prLowLast		= 31,		// *$ for legacy echoed pedal updates to Editor on emChan  9.76
	//				= 32,		//
	ctlThumbSel		= 33,		// $$ formula# for thumbnail, see ctlThumbForm and ctlThumbnail
		thFormBits		= 0x1f,	//	  formula#   [modified 6.17]
		thSelFriend		= 0x20,	//	  modulates/modulated-by/is selected formula  5.40 [Toledo]  5.41
		thDimEq			= 0x40,	//    bit to dim equation
	ctlFormSel		= 34,		// $$ formula number; see also ctlMatVal 
	ctlFormCopy		= 35,		// $$ matA..matDec formula to copy to current; 0 primary to secondary
	ctlThumbForm	= 36,		// $$ equation and shape generator number for ctlThumbnail
		thWBits			= 0x0f,	//	  values 1..5 for SG1..SG5  values 10..13 for Tap1..Tap4   5.41
		  thSg1=1,thSg2,thSg3,thSg4,thSg5,thTap1=10,thTap2,thTap3,thTap4,
		thEqBits		= 0x70,	//	  bits for thumbnail's equation; see ctlFormEq for values
		  thEqShift	  	= 4,	//	  shift for equation bits
	ctlThumbnail	= 37,		// $$ value for thumbnail [modified 6.17]
		thDimZ			= 0x01,	//    bit to dim W
		thDimY			= 0x02,	//    bit to dim X
		thDimX			= 0x04,	//    bit to dim Y
		thDimW			= 0x08,	//    bit to dim Z
		thMacrBits		= 0x70,	//	  macro controller selection used in formula  5.01
		  thMacrShift	  = 4,	//	  shift for macro selection in ctlThumbnail
		  thNone		  = 0,	//	  no macro used in this formula
		  thI			  = 1,	//	  macro i used in this formula
		  thII			  = 2,	//	  macro ii used in this formula
		  thIII			  = 3,	//	  macro iii used in this formula
		  thIV			  = 4,	//	  macro iv used in this formula
		  thV			  = 5,	//	  macro v (aka gen1) used in this formula
		  thVI			  = 6,	//	  macro vi (aka gen2) used in this formula
	// The controllers below specify details for one formula; before sending the
	// messages below, select which formula using ctlFormSel or matrix operation. 
	ctlFormEq		= 38,		// $$ equation type
		fEquationBits	= 0x07,	//    specify equation type
		  fAdd			  = 0,	//	  W+X+Y+Z
		  fWMul			  = 1,	//	  W*(X+Y+Z)
		  fZMul			  = 2,	//	  (W+X+Y)*Z
		  fPairMul		  = 3,	//    (W+X)*(Y+Z)
		  fPairAdd		  = 4,	//    (W*X)+(Y*Z)
		fFineInterp		= 0x08,	//	  fine mode for interpolation control
		//				= 0x70,	//
	ctlBlend		= 39,		// $$ select blend control for primary/secondary formula values  6.10
		ps0=0,ps1,psA=2,psV=23,psi=24,psiv=27,psv,psvi,psCh1Note,	// ctlBlend values
	// W subcomponent of the formula:
	ctlSgW			= 40,		// $$ W shape generator number, must precede ctlHeightW
		wSgBits			= 0x0F,	//    specify sg number
		  wSgConstant	  = 0,	//	  always 1 (ctlShapeW not used)
		  wSgGated		  = 1,	//	  1 if finger down (ctlShapeW not used)
		  wSg1			  = 2,	//	  Shape Generator 1 (ctlShapeW specifies SG shape)
		  wSg2			  = 3,	//	  SG 2
		  wSg3			  = 4,	//	  SG 3
		  wSg4			  = 5,	//	  SG 4
		  wSg5			  = 6,	//	  SG 5
		  wSg1b			  = 7,	//	  SG 1b  (for dualSG)   8.22 
		  wSg2b			  = 8,	//	  SG 2b
		  wSg3b			  = 9,	//	  SG 3b
		  wSg4b			  = 10,	//	  SG 4b
		  wSg5b			  = 11,	//	  SG 5b
		  wDelay		  = 12,	//	  FDelay tap  5.40 (ctlShapeW specifies tap#)
		  wRMS			  = 13,	//	  RMS value of matrix row (1 ms)   9.84
		wInitial		= 0x10,	//	  bit set if ctlHiW/ctlLoW based on initial pitch  5.25 [Interlochen]
		wAllOct			= 0x20,	//	  bit set if w Zone repeats every octave   8.10, moved 8.22
	ctlMultW		= 41,		// $$ multiplier for W
		wMulBits		= 0x07,
		  m0p001		  = 0,	//    multiply by 0.001
		  m0p01			  = 1,	//	  multiply by 0.01
		  m0p1			  = 2,	//    multiply by 0.1
		  m1p0			  = 3,	//    multiply by 1.0
		  m10p0			  = 4,	//	  multiply by 10.0
		  m100p0		  = 5,	//	  multiply by 100.0
		  m1000p0		  = 6,	//	  multiply by 1000.0
		  mavail		  = 7,	//    multiplier available (last one in 3-bit field)
		  #define FormMults .001,.01,.1,1,10,100,1000,1 // for ctlMultW,ctlMultY,ctlMultZ
		wScalerBits		= 0x78,
		  wNone			  = 0,	//	  no scaler (1.0)
		  wI			  = 1,	//	  scale with ctlI
		  wII			  = 2,	//	  scale with ctlII
		  wIII			  = 3,	//	  scale with ctlIII
		  wIV			  = 4,	//	  scale with ctlIV
		  wV			  = 5,	//	  scale with ctlV (aka gen1)
		  wVI			  = 6,	//	  scale with ctlVI (aka gen2)
		  w127xI		  = 7,	//	  scale with ctlI (0..127)  4.92
		  w127xII		  = 8,	//	  scale with ctlII (0..127)
		  w127xIII		  = 9,	//	  scale with ctlIII (0..127)
		  w127xIV		  = 10,	//	  scale with ctlIV (0..127)
		  w127xV		  = 11,	//	  scale with ctlV (aka gen1) (0..127)
		  wDivFings		  = 12,	//	  1 / fingersDown     8.73
		  wVoiceNum		  = 13, //	  scale by voice number (1..n)
		  wIfVoice1		  = 14, //	  1 if voice 1, else 0
		  wDivPoly		  = 15,	//    1 / thisDSPpoly   7.60
	ctlHeightW		= 42,		// $$ height for W			0..100; with wNeg -1..1 in 0.01 steps
	// X subcomponent of the formula:
	ctlHBegX		= 43,		// $$ beginning X position and multiplier
		xBegBits		= 0x0f,	//    beginning X (end of left shelf) 0..12 				 8.10 [Toledo]
		  hMax			= 12,	//    max value for hBegX,hEndx,hBegY,hEndY,hBegZ,hEndZ
		  #define inv_pMax 0.0833333333
		xMultBits		= 0x70,	//	  multiplier for X (same values as for W multiplier)
	ctlZeroX		= 44,		// $$ note number for zero X (60=middleC is default)
	ctlLeftX		= 45,		// $$ value for leftX   	0..100; with xNegLeft  -1..1 in 0.01 steps
	ctlRightX		= 46,		// $$ value for rightX  	0..100; with xNegRight -1..1 in 0.01 steps
	ctlFineX		= 47,		// $$ fine leftX, rightX	0..100; -0.0050..0.0050 in 0.0001 steps
	// Interpolation control.
	ctlInterp		= 48,		// $$ interpolation value 0..100  4.37 [20th Anniversary!]
		interpDef	 	= 40,	//    default interpolation
	// X end and initial control.
	ctlHEndX		= 49,		// $$ ending X position and X initial/continuous
		xEndBits		= 0x0f,	//	  ending X (start of right shelf) 0..12
		xInitBits		= 0x30,	//    cContinuous, cInitial, cDerivative, or cRelative  5.01, 7.19
		  cContinuous	  = 0,	//
		  cInitial		  = 1,
		  cDerivative	  = 2,	//    added 7.19
		  cRelative		  = 3,	//	  for formula's X and Y components only
		  cReleaseF		  = 3,	//	  for formula's Z component only [8.20] 
		  cReleaseP		  = 4,	//	  for formula's Z component only [8.21] 
	// W shape:
	ctlShapeW		= 50,		// $$ select shape generator shape, or wDelay tap 
		wLastTap		= 3,	//    clip value if sg shape is selecting tap; 4 FDelay taps total
		wRampUp			= 0,	//    sg shape: ramp 0..1
		wRampDn			= 1,	//    sg shape: ramp 1..0
		wPulse			= 2,	//	  sg shape: pulse (1 on, 0 off)
		wPulseEnd		= 3,	//    sg shape: pulse at end         7.80
		wTriangle		= 4,	//    sg shape: triangle (0..1..0)	   4.37 [20th Anniversary!]
		wHann			= 5,	//	  sg shape: Hann (raised cosine)    4.37 [20th Anniversary!]
		wGentleUp		= 6,	//    sg shape: S with gentle start  7.80
		wSteepUp		= 7,	//    sg shape: S with steep start   7.80
		wGentleDn		= 8,	//    sg shape: S with gentle start  7.80
		wSteepDn		= 9,	//    sg shape: S with steep start   7.80
		wSquare			= 10,	//	  sg shape: square (1 then -1)
		wSine			= 11,	//    sg shape: sine (-1..1)
		wSampHold		= 12,	//	  sg shape: sample&hold matrix row  4.36 [Sylvania]
								//	  Note: s&h n/a for dualSR; use contCycle or singleCycle.  8.22
		wSampHoldA		= 13,	//	  sg shape: sample&hold formula A  4.38 [Tifton, GA]
		wSampHoldV		= 34,	//	  sg shape: sample&hold formula V  4.38 [Tifton, GA]  6.17
		#define ENDHI(a) ((a)==wRampUp || (a)==wGentleUp || (a)==wSteepUp)
	// Y subcomponent of the formula:
	ctlVBegY		= 51,		// $$ beginning Y value		0..100; with yNegBeg -1..1 in 0.01 steps
	ctlVEndY		= 52,		// $$ ending Y value		0..100; with yNegEnd -1..1 in 0.01 steps
	ctlHBegY		= 53,		// $$ beginning Y position and multiplier
		yBegBits		= 0x0f,	//    beginning Y position (end of left shelf) 0..12
		yMultBits		= 0x70,	//	  multiplier for Y (same values as for W multiplier)
	ctlHEndY		= 54,		// $$ ending Y position and Y initial/continuous
		yEndBits		= 0x0f,	//	  ending Y position (start of right shelf) 0..12
		yInitBits		= 0x70,	//	  cContinuous, cInitial, cDerivative, cRelative  5.01,7.19, 8.21
	ctlTransY		= 55,		// $$ transfer function and polarity (see values below)
		  yzLinear=0,yzS,yzSquared,yzSqRoot,yzTwoStep,yzThreeStep,yzGraph,yzOff1,yzOff2,
	// Ancillary formula:
	ctlAncTyp		= 56,		// $$ specifies effect of ancillary formula  5.11, 7.04 
		fAncTBits		= 0x0f,	//	  type of ancillary formula, values listed below
		  aMul=0,aAbsMul,aInvMul,aDivide,
		  aAdd,aSub,aPow,aLog,
		  aMod,aQuant,
		  aMin,aMax,
		  aCross,
		fAncDBits		= 0x70,	//    domain of ancillary formula, values listed below
		  aWXYZ=0,aW,aX,aY,aZ,
	ctlAncVal		= 57,		// $$ selects ancillary formula, and kHz mode		  5.11, 527
		fAncVBits		= 0x3f,	//	  values listed below  7.04
		  a0p0=0,a0p25,a0p5,a1p0,a1p5,a2p0,a3p0,a4p0,a5p0,a10p0,aAncA,aAncV=31,aTimeCk,
		fAncNNBit		= 0x40,	//	  nn-to-kHz conversion bit   7.18
	// Z subcomponent of the formula:
	ctlVBegZ		= 58,		// $$ beginning Z value		0..100; with zNegBeg -1..1 in 0.01 steps
	ctlVEndZ		= 59,		// $$ ending Z value		0..100; with zNegEnd -1..1 in 0.01 steps
	ctlHBegZ		= 60,		// $$ beginning Z position and multiplier
		zBegBits		= 0x0f,	//    beginning Z position (end of left shelf) 0..12
		zMultBits		= 0x70,	//	  multiplier for Z (same values as for W multiplier)
	ctlHEndZ		= 61,		// $$ ending Z position and Z initial/continuous/derivative
		zEndBits		= 0x0f,	//	  ending Z position (start of right shelf) 0..12
		zInitBits		= 0x70,	//	  cContinuous, cInitial, cDerivative, cRelease   7.18,8.20,8.21
	ctlTransZ		= 62,		// $$ transfer function (see values defined at ctlTransY)
	// Formula persistence.
	ctlPersist		= 63,		// $$ formula persistence 0..100 							 4.36 [Sylvnia]
	// Manipulator.
	ctlSSet1		= 64,		// $$ HarMan 1 and ModMan 1: select Spectral Set  			 5.41 [BigPine]
	ctlSSet2		= 65,		// $$ HarMan 2 and ModMan 2: select Spectral Set 	 		 5.41 [BigPine]
		Live			= 0,	//	  special HarMan: granulate data recorded by VDelay/SDelay
	//				= 66,		// $$ [was convolution length until 10.01]
	//				= 67,		// $$
	//				= 68,		// $$
	//				= 69,		// $$
	// Biquad Banks.
	ctlBqExt		= 70,		// $$ size of biquad banks   5.63, 8.62 [DanielBoone]
		bExt1			= 0x07,	//    BiqGraph1 or biqBank1 extended (8..48 biquads) or bqVar
		bExt2			= 0x38,	//    BiqGraph2 or biqBank2 extended (8..48 biquads) or bqVar
		bqVar		= 6,		//	  bExt1/bExt2 0..5 means 8..48 modes; 6 means variable  		10.09
	// Secondary formula, sign bits.
	ctlSigns		= 71,		// $$ sign bits: msb W, then 2 each X,Y,Z (vertical sliders) 		 6.19
	ctlSigns2		= 72,		// $$ secondary sign bits 											 6.19
	ctlHeightW2		= 73,		// $$ secondary ctlHeightW											 6.19
	//ctlBrightness	= 74,		// ## brightness/cutoff 	[default for y, assignable for z in Editor]
	ctlLeftX2		= 75,		// $$ secondary ctlLeftX  											 6.19
	ctlRightX2		= 76,		// $$ secondary ctlRightX
	ctlVBegY2		= 77,		// $$ secondary ctlVBegY
	ctlVEndY2		= 78,		// $$ secondary ctlVEndY
	ctlVBegZ2		= 79,		// $$ secondary ctlVBegZ
	ctlVEndZ2		= 80,		// $$ secondary ctlVEndZ
	ctlPersist2		= 81,		// $$ secondary ctlPersist  
	ctlInterp2		= 82,		// $$ secondary ctlInterp
	// Oscillator/Filter type.
	ctlFilTyp1		= 83,		// $$ oscillator/filter type, order, extreme  6.19
		bFilTyp			= 0x0f,	//	   filter type bits    [SNois 7.58 Petit Jean, OSCj 9.10 COVID19]
		   OSCn=0,OSCi,OSCj,OSCp,SNois,LP,HP,BP,BR,AP,LS,HS,LP1,HP1,MUL, // values for bFilTyp
		bFilCasc		= 0x30,	//	   filter cascade bits: values 0..3 to cascade 1..4 filters
								//	   OSCj: power of postmultiply  OSCp: phase type 	9.10 [COVID19]
		bFilEx			= 0x40,	//	   audio-rate coef update for filter
	ctlFilTyp2		= 84,		// $$ oscillator/filter type, order, extreme  6.19
	// X Transfer Function.
	ctlTransX		= 85,		// $$ X transfer function
		  xOctaves=0,xKHz,xLinear,xS,xSquared,xSqRoot,xTwoStep,xThreeStep,xGraph,xOff1,xOff2,
	//ctlFracPed    = 86,		// ## 7' lsb for next i,ii,iii,iv,v,vi,jack1,jack2 [modified 8.81]
	//ctlFrac		= 87,		// ## 7' lsb for next bend/y/z, or keyOn is ch1Note [modified 7.44, 8.24]
	//ctlMultiLow	= 88,		// ## multi split: low nn for channel (write only, any Midi channel)  3.38
	//ctlMultiHigh	= 89,		// ## multi split: high nn for channel (write only, any Midi channel) 3.38
	// EaganMatrix parameters.
	ctlFilTyp3		= 90,		// $$ oscillator/filter type, order, extreme  6.19
	ctlBankA		= 91,		// $$ set type for bankA in matrix  4.89, moved here 5.41 [BigPine]
		BiqBank=0,BiqGraph,BiqMouth,SineBank,			// values 0..3 for ctlBankA,ctlBankB
		VDelay=0,SDelay,MDelay,FDelay,					// special 0..3 for ctlBankC
			SineSpray=4,WaveBank,HarMan,ModMan,			// values >3 for ctlBankA,ctlBankB,ctlBankC
			CvcBank=8,Kinetic,Additive,					// values >3 for ctlBankA,ctlBankB,ctlBankC
		#define IsGran(a) IN(SineSpray, a, HarMan)
	ctlBankB		= 92,		// $$ set type for bankB in matrix  4.89, moved here 5.41 [BigPine]
	ctlBankC		= 93,		// $$ set type for bankC in matrix  5.41 [BigPine]
	//				= 94,		// $$ [was convolution response until 10.01]
	//				= 95,		// $$
	//				= 96,		// $$
	//				= 97,		// $$
		nResp			= 9,	//	  number of convolution responses
	ctlFilTyp4		= 98,		// $$ oscillator/filter type, order, extreme  6.19
	ctlFilTyp5		= 99,		// $$ oscillator/filter type, order, extreme  6.19
	// Ancillary formula.
	ctlAncTyp2		= 100,		// $$ specifies 2nd effect of ancillary formula  9.73
		fAncT2Bits		= 0x0f,	//	  type of 2nd ancillary operation, same as fAncTBits values
		fLookupBits		= 0x70,	//	  graph lookup in formula? enum below 9.75
		 aLookNo=0,aLookGrph,aLook1st,aLook2nd,aLookGrph48,aLook1st48,aLook2nd48, // values for fLookupBits
		 #define LookGrph(a) (((a) - 1) % 3)  // 0 for Graph, 1 for Offset Graph 1, 2 Offset Graph 2
	ctlAncVal2		= 101,		// $$ selects 2nd ancillary operation, and source Touch Area  9.73
		fAncV2Bits		= 0x3f,	//	  values same as for fAncVBits
		fAncTAorAbit	= 0x40,	//	  formula-uses-Touch-Area-or-All-Fingers bit   9.73
	// EaganMatrix parameters.
	ctlRow			= 102,		// $$ row in matrix
		rNoise=0,
		rFilt1,rFilt2,rFilt3,rFilt4,rFilt5, // oscillator/filter rows
		rA,rB,rC,
		rAudioL,rAudioR,
		rSubL,rSubR,
		rDirect1,rDirect2,
		rInvert,rMultiply,
	     nRows,					//    number of rows in matrix
	ctlCol			= 103,		// $$ column in matrix
		cSL=0,cSR,				//    matrix column definitions: master
		cCnvI1,cCnvM1,
		cReciR1,cReciR2,
		cCnvI2,cCnvM2,
		cSM,
		cL,cR,
		cSep,					//    [11]
		cFiltIn1,cFiltF1,cFiltB1,//   oscillator/filter columns [12]
		cFiltIn2,cFiltF2,cFiltB2,
		cFiltIn3,cFiltF3,cFiltB3,//	  [18]
		cFiltIn4,cFiltF4,cFiltB4,
		cFiltIn5,cFiltF5,cFiltB5,
		cA1,cA2,cA3,cA4,		//	  bank A columns [27]
		cA5,cA6,cA7,cA8,
		cB1,cB2,cB3,cB4,		//	  bank B columns [35]
		cB5,cB6,cB7,cB8,
		cC1,cC2,cC3,cC4,		//	  bank C columns [43]
		cC5,cC6,cC7,cC8,
		cSgF1,cSgT1,			//    shape generator columns [51]
		cSgF2,cSgT2,
		cSgF3,cSgT3,
		cSgF4,cSgT4,
		cSgF5,cSgT5,
	     nCols,					//    number of columns in matrix [61]
		 nCnv			= 2,	//    number of convolutions (both in Master)  5.41 [BigPine]
		 oCnv			= 4,	//    number of matrix columns offset between convolutions
	     nFilt			= 5,	//    number of oscillators/filters
		 oFilt			= 3,	//	  number of matrix columns per filter
		 nBank			= 3,	// 	  number of multipurpose banks
	     oBank			= 8,	//    number of matrix columns per bank
	     nBiq			= 2,	//	  number of simultaneous biquad banks
	     nGran			= 2,	//	  number of simultaneous granulator banks
	     nSin			= 2,	//	  number of simultaneous sine banks
		 nKin			= 2,	//	  number of simultaneous kinetic banks
	     nSg			= 5,	//	  number of shape generators modules (2*nSg SG max with dualSG) 8.22 
		 oSg			= 2,	//    number of matrix columns per SG
	ctlMatVal		= 104,		// $$ value in matrix - optionally preceded by ctlMatVal1/ctlMatVal0
		matNC			= 0,	//    0 = no connection
		matA			= 1,	//	  "A" = first user-defined formula
		matV			= 22,	//	  "V" = last user-defined formula  6.17
		matW			= 23,	//    w gated W
		matX			= 24,	//	  x concert pitch
		matY			= 25,	//	  y
		matZZ			= 26,	//	  z squared
		 matInterps,			//	  number of interpolated matrix values
		matDec			= 27,	//	  0.0001<=value<=.9999, ctlMatVal1 and ctlMatVal0 have more data  6.17
		mat1			= 28,	//	  constant value 1
		mat9			= 36,	//	  constant value 9
		matSq			= 37,	//	  square row value
		matCu			= 38,	//	  cube row value
		matDisBit 		= 64, 	//    bit set if matrix point disabled (excluded from samMatList)   8.10
	ctlSgTyp2		= 105,		// $$ set shape generator type; see also ctlSgTyp1
		sgTyp4			= 0x03,	//    two bits per SG
		sgTyp5			= 0x0C,	//    two bits per SG
		bTimeSel		= 0x70,	//	  bits to select time delay   50 ms * 2^n    5.01, 5.40e, 6.19
	ctlBankCol		= 106,		// $$ column functions for banks
		bSF1			= 0x03,	//	  two bits for column function1 (bank A)
		bSF2			= 0xC0,	//	  two bits for column function1 (bank B)
			mmSF=0,mmQF,mmSA,	//	  ModMan column function1 choices
		bSB1			= 0x10,	//	  one bit for column function2 (Bank1)  ModMan qB, WaveBank oDuty
		bSB2			= 0x20,	//	  one bit for column function2 (Bank2)  ModMan qB, WaveBank oDuty
	ctlRowTyp		= 107,		// $$ select row types for matrix   5.41 [BigPine]
		rowTyp1			= 0x07,	//	  bits for row type (AudioLR rows)
			rt1Aud=0,rt1Tap,rt1AudTap,						// for rowTyp1
		rowTyp2			= 0x38,	//	  bits for row type (SubLR rows)
			rt2Sub=0,rt2Tap,rt2SubTap,rt2Aud,rt2AudTap,		// for rowTyp2
		aliasDelay		= 0x40,	//    bit set if ok to alias delay data (no nyquist filtering)
	ctlSgTyp1		= 108,		// $$ set shape generator type; see also ctlSgTyp2  5.51
		sgTyp1			= 0x03,	//    two bits per SG
		sgTyp2			= 0x0C,	//    two bits per SG
		sgTyp3			= 0x30,	//    two bits per SG
			contCycle=0,singleCycle,phaseInput,dualSG,		// values for sgTyp fields
	ctlBankParam		= 109,	// $$ bank parameter
		bqDefl1			= 0x07,	//    deflate (biq) or wave (WaveBank) for bank A
		bqDefl2			= 0x38,	//    deflate (biq) or wave (WaveBank) for bank B
			wave_saw = 0, wave_square, wave_triangle, wave_LeCaine,	// kind of waveBank
			defBankParam = 1,	//	  deflate=1 (biqBank avoid SpiralOfDeath) or square (waveBank) 7.60
		//				= 0x40
	ctlMatOp		= 110,		// $$ matrix operation
		// 				= 100..105 avail
		colShowVal		= 106,	//	  Editor <--> Continuum: Show column value 8.61 [DanielBoone]
		noShowVal		= 107,	//	  Editor <--> Continuum: Hide debug value  8.10
		formShowVal		= 108,	//	  Editor <--> Continuum: Show selected formula value  8.10
		//				= 109,	//    [no longer used 9.13 COVID19]
		nyqDis			= 110,	//	  Continuum to Editor: Delay's nyquist filter is disabled 5.41
		nyqOff			= 111,	//	  Continuum to Editor: Delay's nyquist filter is not in use 5.41
		nyqOn			= 112,	//	  Continuum to Editor: Delay's nyquist filter is in use 
		matSendArrays	= 114,	//	  Editor to Continuum: request to send graph/props data 9.75
		matSelPosAlt	= 115,	//    Editor to Continuum: matSelPos with Alt key down  5.13
		matBegPos		= 116,	//    Editor to Continuum: user mousedown in matrix  5.13
		matHiliteWide	= 117,	//	  Continuum to Editor: wide highlight current position 6.17
		matHilite		= 118,	//	  Continuum to Editor: normal highlight current position 6.17
		matNoHilite		= 119,	//	  Continuum to Editor: no highlighted position in matrix 6.17
		matForHilite	= 120,	//	  Continuum to Editor: next data is for highlit point 6.17
		matTogMute		= 121,	//	  Editor to Continuum: toggle mute (disab) on matrix point  8.10
		//				= 122,	//
		//				= 123,	//
		matRefresh		= 124,	//	  Refresh matrix, current formula, text
		matClear		= 125,	//    clear matrix (sent to Editor only when new sound loaded)
		matStart		= 126,	//    matrix data follows
		matSelPos		= 127,	//    Editor to Continuum: user mouseup in matrix, select it;
								//	  if matBegPos was elsewhere and valid, drag value to here 5.13
	ctlNeighborInd	= 111,		// $$ matrix index to right of matHiliteWide (if any) 6.17
	ctlQuantX		= 112,		// $$ x quantization
			#define Quant 0,1,2,3,4,5,7,12,24,36,48,96 // list of values for ctlQuantX
	ctlModMatVal	= 113,		// $$ numeric mod in matrix, followed by ctlModMatInd 6.01
	ctlModMatInd	= 114,		// $$ select index in matrix list (for mod of numeric in matrix) 6.01
	ctlMatVal0		= 115,		// $$ 1..100: thousandth and ten-thousandth digits  (for MatDec) 6.17
								//    precedes ctlMatVal, follows ctlMatVal1; omit if value 0
	ctlMatVal1		= 116,		// $$ 0..100: tenth and hundredth digits (for matDec) 6.17
								//	  precedes ctlMatVal0 (if ctlMatVal0 is present) and ctlMatVal
	// W shape generator modulation, nn low/high:
	ctlSgM			= 117,		// $$ sg phase modulation  5.26 [Toronto] 5.28
		bMod			= 0x1f,	//	  bits specify phase value modulating formula
			sg90=1,sg180,sg270,sgModA,sgModV=25,	// values for bMod bits
		//				= 0x60,	//	  bits avail
	ctlLoW			= 118,		// $$ low nn for W   5.25 [Interlochen]
	ctlHiW			= 119,  	// $$ high nn for W  5.25
	//				= 120,		//    ctlSoundOff is special, cannot use for data communications
	// Blend Transfer Function.																 9.74
	ctlHBegBl		= 121,		// $$ beginning Blend shelf, and part of Blend Transfer Function
		blTranBitHigh	= 0x01,	//	  high bit of 4-bit Blend Transfer Function
		blBegBits		= 0x1e,	//    beginning Blend shelf (end of left shelf) 0..12
		fLimBit			= 0x20,	//	  activate matrix column limiter (or leave unchanged)	 9.75
	ctlHEndBl		= 123,		// $$ ending Blend shelf, and part of Blend Transfer Function
		blEndBits		= 0x0f,	//    ending Blend shelf (start of right shelf) 0..12
		blTranBitsLow	= 0x70,	//	  low 3 bits of Blend Transfer Function
	//				= 122,		// $$ [was ctlCnvShif1 until 10.01]
	//				= 123,		//	  see ctlHEndBl above
	//				= 124,		// $$ [was ctlCnvShif2 until 10.01]
	//				= 125,		// $$
	//				= 126,		// $$
	ctlEMLast		= 126,
	//				= 127,		//    special for MPE, cannot use for data communications



// -------------- Continuum-specific controllers on channel 13.  Kenton configuration.  ------------------
	// cc number received from or sent to Editor, Kenton configuration
	// These were renumbered to avoid overlap with cc 74 and others.  [5.60  Fruita 2013]
	ctlKAssign1		= 30,		//    receive and send: data is cc assigned to rotary 1
	ctlKAssign9		= 38,		//    receive and send: data is cc assigned to rotary 9
		nKRots			= 9,	//	  number of Kenton rotaries
	ctlKAssignX		= 39,		//    receive and send: data is cc assigned to joystick x
	ctlKAssignY		= 40,		//    receive and send: data is cc assigned to joystick y
		nKAssigs		= 11,	//    9 Kenton rotaries plus Joystick X plus Joystick Y
	// cc numbers received from or sent to Kenton
	ctlKRot1		= 41,		//    receive: rotary new position    send: sets rotary level leds
	ctlKRot2		= 42,
	ctlKRot3		= 43,
	ctlKRot9		= 49,
	ctlKButton1		= 50,		//    receive: button pressed
	ctlKButton9		= 58,
	ctlKJoyRight	= 59,		//    receive: joystick right
	ctlKJoyUp		= 60,		//    receive: joystick up
	ctlKJoyLeft		= 61,		//    receive: joystick left
	ctlKJoyDown		= 62,		//    receive: joystick down
		#define isJoyY(cc)	   (cc & 1) // true if cc is for Y, when cc ctlKJoyRight..ctlKJoyDown
		#define JoyToDat(cc,v) ((cc >= ctlKJoyLeft) ? 64 - (v>>1) : 64 + (v>>1))
	ctlKHello		= 63,		//    new channel announcement, also generated by Editor
		edYesKenton 	= 100,	//	  Editor says it sees Kenton
		edNoKenton		= 101,  //    Editor says it does not see Kenton
		syVers2			= 126,	//    Kenton v2 SysEx response: can do rotary toggle and midi echo 
		syVers1			= 127,	//    Kenton v1 SysEx response: small memory Kenton 
	ctlKRotPress1	= 64,		//    receive: rotary pressed		 send: sets rotary select led (v2)
	ctlKRotPress2	= 65,
	ctlKRotPress3	= 66,
	ctlKRotPress4	= 67,
	ctlKRotPress5	= 68,
	ctlKRotPress6	= 69,
	ctlKRotPress7	= 70,
	ctlKRotPress8   = 71,
	ctlKRotPress9   = 72,
};




// Definitions for configuration from playing surface -- Midi note numbers
enum {

	// Internal sound parameter selection locations on the playing surface.
	optCalibrate = 48, optPost, optIntSound, optMidiProgram, optCVC,
	optRouting, optPoly, optChannelPrio, optBendRange, optY, optZ, optVelocity,
	optMiddleC, optSplitPoint, optSplitMode, optMonoFunc, optMonoInt, 
	optRoundInitial, optRoundRate, optTuning, optPedal1, optPedal2, 
	opti, optii, optiii, optOptions, optSend, optLoad, optStore,

	firstOpt = optCalibrate, lastOpt = optStore,

	// Data entry.
	optValue120 = 48, optValue10 = 59, optValue0 = 60, optValue16 = 76,

	// Options category.
	optReset = 0,					// reset options selections
	optMedAction = 1,				// medium action [was optSingChan until 5.13]
	optAesHouse = 2,				// allow slaving of aestx clock to aesrx rate
	optAes48k = 3,					// 48k internal clock for aestx
	optTwoOct = 4,					// octave pedal does two octaves (instead of normal 1 octave)
	optFourOct = 5,					// octave pedal does four octaves (instead of normal 1 octave)
	optReverse = 6,					// reverse pitches
	optNoRecirc = 7,				// avoid recirculator  4.40, renumbered 5.13, 6.11, 6.14
	optTxThird = 8,					// one-third Midi transmit rate  6.41, 7.57, 8.55
	optTxTweenth = 9,				// one-twentieth Midi transmit rate  6.41, 7.57  7.59
	optDemoAssort = 10,				// demo presets   7.00

	optResetAll = 127,				// reset all saved presets as well as current configuration

	// Send category.
	sendConfig	= 1,				// send current config over midi
};


// MINI: Indices into calibration arrays.
enum { L0 = 0, L1, R0, R1, rL, rR, calSiz=6, halls=4 };
enum { calMLo = 0, calMMid, calMHi, calPts=3 };



#endif
	
