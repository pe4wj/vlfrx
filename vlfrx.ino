/*
 * VLF receiver for SAQ at 17.2 kHz
 * Author: W.J. Ubbels PE4WJ
 */
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

int led = 13;

// GUItool: begin automatically generated code
AudioInputI2S            i2s1;           //xy=146,340
AudioSynthWaveformSine   sine1;          //xy=198.1666717529297,538.1666870117188
AudioSynthWaveformSine   sine2;          //xy=216.1666717529297,591.1666717529297
AudioEffectMultiply      multiply1;      //xy=299.1666564941406,338.1666564941406
AudioEffectMultiply      multiply2;      //xy=306.1666717529297,396.1666717529297
AudioFilterFIR           fir1;           //xy=457.16668701171875,335.16668701171875
AudioFilterFIR           fir2;           //xy=457.16668701171875,396.16668701171875
AudioSynthWaveformSine   sine4;          //xy=479.1666717529297,565.1666717529297
AudioSynthWaveformSine   sine5;          
AudioSynthWaveformSine   sine3;          //xy=485.1666717529297,516.1666717529297
AudioEffectFade          fade2;          //xy=597,1231
AudioEffectMultiply      multiply3;      //xy=615.1666717529297,333.1666717529297
AudioEffectMultiply      multiply4;      //xy=639.1666717529297,399.1666717529297
AudioMixer4              mixer1;         //xy=783.1666870117188,361.16668701171875
AudioOutputI2S           i2s2;           //xy=848,217
AudioConnection          patchCord1(i2s1, 0, multiply1, 0);
AudioConnection          patchCord2(i2s1, 0, multiply2, 0);
// AudioConnection          patchCord1(sine5, 0, multiply1, 0);
// AudioConnection          patchCord2(sine5, 0, multiply2, 0);

AudioConnection          patchCord3(sine1, 0, multiply1, 1);
AudioConnection          patchCord4(sine2, 0, multiply2, 1);
AudioConnection          patchCord5(multiply1, fir1);
AudioConnection          patchCord6(multiply2, fir2);
AudioConnection          patchCord7(fir1, 0, multiply3, 0);
AudioConnection          patchCord8(fir2, 0, multiply4, 0);
// AudioConnection          patchCord7(multiply1, 0, multiply3, 0);
// AudioConnection          patchCord8(multiply2, 0, multiply4, 0);

AudioConnection          patchCord9(sine4, 0, multiply4, 1);
AudioConnection          patchCord10(sine3, 0, multiply3, 1);
AudioConnection          patchCord11(multiply3, 0, mixer1, 0);
AudioConnection          patchCord12(multiply4, 0, mixer1, 1);
AudioConnection          patchCord13(mixer1, 0, i2s2, 0);
// AudioConnection          patchCord13(sine4, 0, i2s2, 0);
// GUItool: end automatically generated code
AudioControlSGTL5000     sgtl5000_1;


// const int myInput = AUDIO_INPUT_LINEIN; // use line input on the audio shield
const int myInput = AUDIO_INPUT_MIC; // use mic input on the audio shield


const float phaseshift = 90; // degrees phaseshift
const float freq = 19600; // receiver center freq in Hz
const float test_freq = 17200; // receiver center freq in Hz
const float bfo_freq = 600; // BFO freq in Hz

/*

FIR filter designed with
 http://t-filter.appspot.com

sampling frequency: 44100 Hz

fixed point precision: 16 bits

* 0 Hz - 250 Hz
  gain = 1
  desired ripple = 2 dB
  actual ripple = n/a

* 500 Hz - 22050 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = n/a

*/

const short FILTER_TAP_NUM = 200;

const short filter_taps[] = {
-365,
  -30,
  -31,
  -31,
  -32,
  -33,
  -34,
  -34,
  -34,
  -35,
  -35,
  -35,
  -34,
  -34,
  -33,
  -32,
  -31,
  -30,
  -28,
  -27,
  -25,
  -23,
  -20,
  -18,
  -15,
  -12,
  -8,
  -4,
  0,
  4,
  8,
  13,
  18,
  24,
  29,
  35,
  41,
  48,
  55,
  62,
  69,
  76,
  84,
  92,
  100,
  109,
  117,
  126,
  136,
  145,
  154,
  164,
  174,
  183,
  194,
  204,
  215,
  225,
  235,
  246,
  257,
  267,
  278,
  289,
  299,
  310,
  320,
  331,
  341,
  352,
  362,
  372,
  382,
  391,
  401,
  410,
  419,
  428,
  437,
  445,
  453,
  461,
  468,
  475,
  482,
  488,
  494,
  500,
  505,
  510,
  514,
  518,
  522,
  525,
  528,
  530,
  532,
  533,
  534,
  534,
  534,
  534,
  533,
  532,
  530,
  528,
  525,
  522,
  518,
  514,
  510,
  505,
  500,
  494,
  488,
  482,
  475,
  468,
  461,
  453,
  445,
  437,
  428,
  419,
  410,
  401,
  391,
  382,
  372,
  362,
  352,
  341,
  331,
  320,
  310,
  299,
  289,
  278,
  267,
  257,
  246,
  235,
  225,
  215,
  204,
  194,
  183,
  174,
  164,
  154,
  145,
  136,
  126,
  117,
  109,
  100,
  92,
  84,
  76,
  69,
  62,
  55,
  48,
  41,
  35,
  29,
  24,
  18,
  13,
  8,
  4,
  0,
  -4,
  -8,
  -12,
  -15,
  -18,
  -20,
  -23,
  -25,
  -27,
  -28,
  -30,
  -31,
  -32,
  -33,
  -34,
  -34,
  -35,
  -35,
  -35,
  -34,
  -34,
  -34,
  -33,
  -32,
  -31,
  -31,
  -30,
  -365
};


void setup() {
  // Audio connections require memory to work.
  AudioMemory(120);

  // Enable the audio shield, select input, and enable output
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.volume(0.8);
  sgtl5000_1.lineInLevel(15);
  sgtl5000_1.lineOutLevel(13);
  sgtl5000_1.micGain(63);

  // disable ADC High pass filter to reduce noise (as per http://openaudio.blogspot.com/2017/03/teensy-audio-board-self-noise.html?m=1)
  sgtl5000_1.adcHighPassFilterDisable();

  
  //setup everything
  // disable interrupts for a while to setup everything, and guarantee accurate phase quadrature
  AudioNoInterrupts();  // disable audio library momentarily

  
  
  // oscillators
  // ddc sin
  sine1.amplitude(1.0);
  sine1.frequency(freq);
  sine1.phase(0);
  // ddc cos
  sine2.amplitude(1.0);
  sine2.frequency(freq);
  sine2.phase(phaseshift);
  // bfo sin
  sine3.amplitude(1.0);
  sine3.frequency(bfo_freq);
  sine3.phase(phaseshift);
  // bfo cos
  sine4.amplitude(1.0);
  sine4.frequency(bfo_freq);
  sine4.phase(0);

  
  // testsig cos
  sine5.amplitude(1.0);
  sine5.frequency(test_freq);
  sine5.phase(phaseshift);
  AudioInterrupts();    // enable audio library
  
  
  // setup the delay
  //delay1.delay(0, delaytime);  // init gain to 1
  //amp1.gain(1);
  //fade1.fadeIn(10);
  

  // initialize the LED digital pin as an output.
  pinMode(led, OUTPUT);



//start FIR filter

  fir1.begin(filter_taps, FILTER_TAP_NUM);
  fir2.begin(filter_taps, FILTER_TAP_NUM);
}

void loop() {

  delay(200);  
  digitalWrite(led, HIGH);
  delay(200);  
  digitalWrite(led, LOW);
  
  
}
