/*
 * VLF receiver for SAQ at 17.2 kHz
 * Author: W.J. Ubbels PE4WJ
 */
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Encoder.h>
#include <U8g2lib.h>
#include <Bounce.h>


#include <Rotary.h>


#define T4

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

AudioConnection          patchCord9(sine3, 0, multiply4, 1);
AudioConnection          patchCord10(sine4, 0, multiply3, 1);
AudioConnection          patchCord11(multiply3, 0, mixer1, 0);
AudioConnection          patchCord12(multiply4, 0, mixer1, 1);
AudioConnection          patchCord13(mixer1, 0, i2s2, 0);
AudioConnection          patchCord14(mixer1, 0, i2s2, 1);
// AudioConnection          patchCord13(sine4, 0, i2s2, 0);

AudioAnalyzeRMS      rms;
AudioConnection          patchCord15(mixer1, 0, rms, 0);
// GUItool: end automatically generated code
AudioControlSGTL5000     sgtl5000_1;

Bounce bouncer = Bounce( 2,50);  // rotary push button

U8G2_SSD1306_64X32_1F_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

Rotary rotary = Rotary(1, 0);

// const int myInput = AUDIO_INPUT_LINEIN; // use line input on the audio shield
const int myInput = AUDIO_INPUT_MIC; // use mic input on the audio shield


const int samplerate = 44117.64706; //352800; //192000;
const float samplerate_factor = 44117.64706 / samplerate;

const float phaseshift = 90; // degrees phaseshift

const float default_freq = 17200;
float freq = default_freq; // receiver center freq in Hz
const float test_freq = 17200; // receiver center freq in Hz
const float bfo_freq = 600; // BFO freq in Hz
float freqstep = 25; // tuning step in Hz
const int micgain = 63; // mic gain in dB
bool do_display = true;

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
  sgtl5000_1.volume(0);
  sgtl5000_1.lineInLevel(15);
  sgtl5000_1.lineOutLevel(13);
  sgtl5000_1.micGain(micgain);

  // disable ADC High pass filter to reduce noise (as per http://openaudio.blogspot.com/2017/03/teensy-audio-board-self-noise.html?m=1)
  sgtl5000_1.adcHighPassFilterDisable();

  
  //setup everything
  // disable interrupts for a while to setup everything, and guarantee accurate phase quadrature
  AudioNoInterrupts();  // disable audio library momentarily
  // set sample rate
  setI2SFreq(samplerate);
  
  
  // oscillators
  // ddc sin
  sine1.amplitude(1.0);
  sine1.frequency(freq * samplerate_factor);
  sine1.phase(0);
  // ddc cos
  sine2.amplitude(1.0);
  sine2.frequency(freq * samplerate_factor);
  sine2.phase(phaseshift);
  // bfo sin
  sine3.amplitude(1.0);
  sine3.frequency(bfo_freq * samplerate_factor);
  sine3.phase(phaseshift);
  // bfo cos
  sine4.amplitude(1.0);
  sine4.frequency(bfo_freq * samplerate_factor);
  sine4.phase(0);

  
  // testsig cos
  sine5.amplitude(1.0);
  sine5.frequency(test_freq * samplerate_factor);
  sine5.phase(phaseshift);
  AudioInterrupts();    // enable audio library
  
  
  // setup the delay
  //delay1.delay(0, delaytime);  // init gain to 1
  //amp1.gain(1);
  //fade1.fadeIn(10);
  

  // initialize the LED digital pin as an output.
  pinMode(led, OUTPUT);

  // encoder, switch and LED pins
  pinMode(0, INPUT_PULLUP); // encoder A
  pinMode(1, INPUT_PULLUP); // encoder B
  pinMode(2, INPUT); // encoder SW
  pinMode(3, OUTPUT); // LED BLUE
  pinMode(4, OUTPUT); // LED GREEN
  pinMode(5, OUTPUT); // LED RED
  
  digitalWrite(3, HIGH); // LED OFF
  digitalWrite(4, HIGH); // LED OFF
  digitalWrite(5, HIGH); // LED OFF



//start FIR filter

  fir1.begin(filter_taps, FILTER_TAP_NUM);
  fir2.begin(filter_taps, FILTER_TAP_NUM);

  Serial.begin(9600);
  attachInterrupt(0, rotate, CHANGE);
  attachInterrupt(1, rotate, CHANGE);
  Serial.println("samplerate_factor:");
  Serial.println(samplerate_factor);
  u8g2.begin();

  // init the frequency
  setfreq();
  // display the frequency
  displayfreq();



  
}


  void setfreq(){  
// Serial.println(freq);
 
 

    
   AudioNoInterrupts();    // enable audio library
      // ddc sin
  sine1.amplitude(1.0);
  sine1.frequency(freq * samplerate_factor);
  sine1.phase(0);
  // ddc cos
  sine2.amplitude(1.0);
  sine2.frequency(freq * samplerate_factor);
  sine2.phase(phaseshift);
    AudioInterrupts();    // enable audio library
  }

  void displayfreq(){
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_t0_11_tf);  // choose a suitable font
  u8g2.drawStr(0, 10, "Freq (Hz):");  // write something to the internal memory
  u8g2.setCursor(0, 24);
  u8g2.print(freq);
  u8g2.sendBuffer();          // transfer internal memory to the display
  }
  void clear_display(){
  u8g2.clearBuffer();          // clear the internal memory
//  u8g2.setFont(u8g2_font_t0_11_tf);  // choose a suitable font
//  // u8g2.drawStr(0, 10, "Hello,");  // write something to the internal memory
//  u8g2.setCursor(0, 24);
//  u8g2.print(freq);
  u8g2.sendBuffer();          // transfer internal memory to the display
  }

// rotate is called anytime the rotary inputs change state.
void rotate() {
  unsigned char result = rotary.process();

if (digitalRead(2)) {
  freqstep = 1000;
  }
else {
  freqstep = 25;
}
  
  if (result == DIR_CW) {
    freq = freq + freqstep;

  } else if (result == DIR_CCW) {
    freq = freq - freqstep;

  }

// frequency limits
    if (freq < 1000) {
      freq = 1000;
      
    }
    else {
      
    }
    if (freq > samplerate/2) {
      freq = 22050;//floor(samplerate/2);
      
    }
    else {
      
    }

    
  setfreq();
  if (do_display) {
    displayfreq();
    }
//    else {
//      clear_display();
//      }

  }

// set samplerate code by Frank Bösing
void setI2SFreq(int freq) {
#if defined(T4) 
  // PLL between 27*24 = 648MHz und 54*24=1296MHz
  int n1 = 4; //SAI prescaler 4 => (n1*n2) = multiple of 4
  int n2 = 1 + (24000000 * 27) / (freq * 256 * n1);
  double C = ((double)freq * 256 * n1 * n2) / 24000000;

  int nfact = C;
  int ndiv = 10000;
  int nmult = C * ndiv - (nfact * ndiv);

  CCM_ANALOG_PLL_AUDIO = 0;
  CCM_ANALOG_PLL_AUDIO |= CCM_ANALOG_PLL_AUDIO_ENABLE
                          | CCM_ANALOG_PLL_AUDIO_POST_DIV_SELECT(2) // 2: 1/4; 1: 1/2; 0: 1/1
                          | CCM_ANALOG_PLL_AUDIO_DIV_SELECT(nfact);

  CCM_ANALOG_PLL_AUDIO_NUM   = nmult & CCM_ANALOG_PLL_AUDIO_NUM_MASK;
  CCM_ANALOG_PLL_AUDIO_DENOM = ndiv & CCM_ANALOG_PLL_AUDIO_DENOM_MASK;
  while (!(CCM_ANALOG_PLL_AUDIO & CCM_ANALOG_PLL_AUDIO_LOCK)) {}; //Wait for pll-lock

  const int div_post_pll = 1; // other values: 2,4
  CCM_ANALOG_MISC2 &= ~(CCM_ANALOG_MISC2_DIV_MSB | CCM_ANALOG_MISC2_DIV_LSB);
  if (div_post_pll > 1) CCM_ANALOG_MISC2 |= CCM_ANALOG_MISC2_DIV_LSB;
  if (div_post_pll > 3) CCM_ANALOG_MISC2 |= CCM_ANALOG_MISC2_DIV_MSB;

  CCM_CSCMR1 = (CCM_CSCMR1 & ~(CCM_CSCMR1_SAI1_CLK_SEL_MASK))
               | CCM_CSCMR1_SAI1_CLK_SEL(2); // &0x03 // (0,1,2): PLL3PFD0, PLL5, PLL4
  CCM_CS1CDR = (CCM_CS1CDR & ~(CCM_CS1CDR_SAI1_CLK_PRED_MASK | CCM_CS1CDR_SAI1_CLK_PODF_MASK))
               | CCM_CS1CDR_SAI1_CLK_PRED(n1 - 1) // &0x07
               | CCM_CS1CDR_SAI1_CLK_PODF(n2 - 1); // &0x3f

  IOMUXC_GPR_GPR1 = (IOMUXC_GPR_GPR1 & ~(IOMUXC_GPR_GPR1_SAI1_MCLK1_SEL_MASK))
                    | (IOMUXC_GPR_GPR1_SAI1_MCLK_DIR | IOMUXC_GPR_GPR1_SAI1_MCLK1_SEL(0));  //Select MCLK

  //Serial.printf("n1:%d n2:%d C:%f c0:%d c1:%d c2:%d\n",  n1, n2, C, nfact, nmult, ndiv);
#else
  typedef struct {
    uint8_t mult;
    uint16_t div;
  } tmclk;

  const int numfreqs = 20;
  //  const int samplefreqs[numfreqs] = { 8000, 11025, 16000, 22050, 32000, 44100, (int)44117.64706 , 48000, 88200, (int)44117.64706 * 2, 96000, 100000, 176400, (int)44117.64706 * 4, 192000};
  const int samplefreqs[numfreqs] = { 8000, 11025, 16000, 22050, 32000, 44100, (int)44117.64706 , 48000, 50223, 88200, (int)44117.64706 * 2,
                                      96000, 100000, 100466, 176400, (int)44117.64706 * 4, 192000, 234375, 281000, 352800
                                    };

#if (F_PLL==180000000)
  //{ 8000, 11025, 16000, 22050, 32000, 44100, (int)44117.64706 , 48000,
  const tmclk clkArr[numfreqs] = {{46, 4043}, {49, 3125}, {73, 3208}, {98, 3125}, {183, 4021}, {196, 3125}, {16, 255}, {128, 1875},
    // 50223, 88200, (int)44117.64706 * 2, 96000, 100466, 176400, (int)44117.64706 * 4, 192000, 234375, 281000, 352800};
    {1, 14}, {107, 853}, {32, 255}, {219, 1604}, {224, 1575}, {1, 7}, {214, 853}, {64, 255}, {219, 802}, {1, 3}, {2, 5} , {1, 2}
  };
#endif


  for (int f = 0; f < numfreqs; f++) {
    if ( freq == samplefreqs[f] ) {
      while (I2S0_MCR & I2S_MCR_DUF) ;
      I2S0_MDR = I2S_MDR_FRACT((clkArr[f].mult - 1)) | I2S_MDR_DIVIDE((clkArr[f].div - 1));
      return;
    }
  }
#endif //Teensy4
}


void loop() {

//crude AGC
if (rms.available()) {
      
      float agc = 0.5 /rms.read();
      if (agc > 0.7) {
        agc = 0.7;
      }
      sgtl5000_1.volume(agc);
      Serial.println(agc);
}

  
bouncer.update ( );
 int value = bouncer.risingEdge();//bouncer.read();
//  long newPosition = myEnc.read();
//  if (newPosition != oldPosition) {
//    oldPosition = newPosition;
    

//  delay(200);  
//  digitalWrite(led, HIGH);
//  // digitalWrite(5, LOW); // LED ON
//  delay(200);  
//  digitalWrite(led, LOW);
  // digitalWrite(5, HIGH); // LED OFF
//  
// if (digitalRead(2)) {
//   digitalWrite(4, LOW); // LED ON
//   // freq = default_freq;
//   // setfreq();
//   // freqstep = 1000;
//   
// } else {
//   digitalWrite(4, HIGH); // LED OFF
//   // freqstep = 25;
//   
// }

if (value == HIGH) {

  if (do_display){
    do_display = false;
    digitalWrite(4, HIGH); // LED OFF 
    digitalWrite(5, HIGH); // LED OFF 
    
       

  }
  else {
    do_display = true;
    digitalWrite(4, LOW); // LED ON
    digitalWrite(5, LOW); // LED ON

  }
}
  
  
  
}
