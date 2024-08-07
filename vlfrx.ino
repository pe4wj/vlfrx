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


#include <Rotary.h> // Ben Buxton's rotary library https://github.com/buxtronix/arduino/tree/master/libraries/Rotary

#define T4

// init LED pins
int led = 13; // builtin LED
int led_blue = 3; // RGB encoder LED
int led_green = 4; // RGB encoder LED
int led_red = 5; // RGB encoder LED


AudioInputI2S            i2s1;           //xy=146,340
AudioSynthWaveformSine   sine1;          //xy=198.1666717529297,538.1666870117188
AudioSynthWaveformSine   sine2;          //xy=216.1666717529297,591.1666717529297
AudioEffectMultiply      multiply1;      //xy=299.1666564941406,338.1666564941406
AudioEffectMultiply      multiply2;      //xy=306.1666717529297,396.1666717529297
AudioFilterFIR           fir1;           //xy=457.16668701171875,335.16668701171875
AudioFilterFIR           fir2;           //xy=457.16668701171875,396.16668701171875
AudioSynthWaveformSine   sine4;          //xy=479.1666717529297,565.1666717529297
AudioSynthWaveformSine   sine3;          //xy=485.1666717529297,516.1666717529297
AudioEffectFade          fade2;          //xy=597,1231
AudioEffectMultiply      multiply3;      //xy=615.1666717529297,333.1666717529297
AudioEffectMultiply      multiply4;      //xy=639.1666717529297,399.1666717529297
AudioMixer4              mixer1;         //xy=783.1666870117188,361.16668701171875
AudioOutputI2S           i2s2;           //xy=848,217

AudioEffectDelay         delay1;
//AudioAnalyzeFFT1024      FFT1;

//
//
//
AudioFilterStateVariable filt1;
AudioFilterStateVariable filt2;
AudioFilterStateVariable filt3;
AudioFilterStateVariable filt4;

AudioFilterStateVariable filt5;
AudioFilterStateVariable filt6;
AudioFilterStateVariable filt7;
AudioFilterStateVariable filt8;

// input high pass filter
AudioFilterStateVariable filt9;


// autodetect
AudioEffectFade          fade1;
AudioAnalyzeFFT1024    myFFT;



AudioConnection          patchCord1(i2s1, 0, filt9, 0);
// AudioConnection          patchCord2(i2s1, 0, filt9, 0);

AudioConnection          patchCord26(filt9, 2, multiply1, 0); // high pass output
AudioConnection          patchCord27(filt9, 2, multiply2, 0); // high pass output

AudioConnection          patchCord28(filt9, 2, myFFT, 0); // high pass output to FFT input

AudioConnection          patchCord3(sine1, 0, multiply1, 1);
AudioConnection          patchCord4(sine2, 0, multiply2, 1);
AudioConnection          patchCord5(multiply1, fir1);
AudioConnection          patchCord6(multiply2, fir2);
 // AudioConnection          patchCord7(fir1, 0, multiply3, 0);
 // AudioConnection          patchCord8(fir2, 0, multiply4, 0);

//// fir1 to filt1&2
AudioConnection          patchCord18(fir1, 0, filt1, 0); // I
AudioConnection          patchCord19(fir2, 0, filt2, 0); // Q
//
//// filt 1&2 to filt3&4
AudioConnection          patchCord20(filt1, 0, filt3, 0); // I
AudioConnection          patchCord21(filt2, 0, filt4, 0); // Q
//

//// filt3&4 to filt5&6
AudioConnection          patchCord22(filt3, 0, filt5, 0); // I
AudioConnection          patchCord23(filt4, 0, filt6, 0); // Q
//
//// filt 5&6 to filt7&8
AudioConnection          patchCord24(filt5, 0, filt7, 0); // I
AudioConnection          patchCord25(filt6, 0, filt8, 0); // Q
//


//// filt 7&8 to multiply3&4
AudioConnection          patchCord7(filt7, 0, multiply3, 0);
AudioConnection          patchCord8(filt8, 0, multiply4, 0);


AudioConnection          patchCord9(sine3, 0, multiply4, 1);
AudioConnection          patchCord10(sine4, 0, multiply3, 1);
AudioConnection          patchCord11(multiply3, 0, mixer1, 0);
AudioConnection          patchCord12(multiply4, 0, mixer1, 1);

AudioAnalyzePeak         peak;
AudioConnection          patchCord15(mixer1, 0, peak, 0);


AudioConnection          patchCord16(mixer1, 0, fade1, 0);

AudioConnection          patchCord29(fade1, 0, delay1, 0);

AudioConnection          patchCord13(delay1, 0, i2s2, 0);
AudioConnection          patchCord14(delay1, 0, i2s2, 1);
//AudioConnection          patchCord17(delay1, 0, FFT1, 0);


// GUItool: end automatically generated code
AudioControlSGTL5000     sgtl5000_1;

Bounce bouncer = Bounce(2,50);  // rotary push button

U8G2_SSD1306_64X32_1F_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

Rotary rotary = Rotary(1, 0);




const int myInput = AUDIO_INPUT_LINEIN; // use line input on the audio shield
// const int myInput = AUDIO_INPUT_MIC; // use mic input on the audio shield

// const int samplerate = 352800;// 44117.64706; //352800;// 
int samplerate;
float samplerate_factor; // = 44117.64706 / samplerate;

const float phaseshift = 90; // degrees phaseshift

// const float default_freq = 17200;
float freq;//  = default_freq; // receiver center freq in Hz
float dblevel = 2;
float avg_level = 0;

String mode;
String previousmode = "";


const int modeVoltThresh = 3150; // mode select voltage
const int analogPin = A0; // Analog input pin
const float V_ref = 3300; // ADC V_ref
const int n_avg = 10; // number of ADC averages
  


float bfo_freq;
float freqstep;
int filtfreq;
int hpffiltfreq; 

const int micgain = 63; // mic gain in dB
bool do_display = true;
float vol = 0;
const float alpha = 0.002; // averaging for AGC decay
const float alpha2 = 1; // averaging for peak value display in dBFS
const int delaytime = 10; // milliseconds

////FFT vars
//const int low_idx = 7;    // lower frequency limit is 300 Hz, this is the closest bin idx in the FFT for a 1024 point FFT
//const int high_idx = 21; // upper frequency limit is  900 Hz, this is the closest bin idx in the FFT for a 1024 point FFT
//const int n_bins = high_idx - low_idx + 1; // the number of bins to be processed in the FFT for a 1024 point FFT
//const int n_pixels = 64; // number of row pixels of the OLED display
//int pixel = 0;


// BAT detector autodetect vars
elapsedMillis hangcounter = 0; // counter which increments when the squelch hangs

const int FFT_SIZE = 1024;

const int low_idx = 44; // 15 kHz // lower frequency limit is approx 20000 Hz, this is the closest bin idx in the FFT for a 1024 point FFT
const int high_idx = 435; // upper frequency limit is approx 150000 Hz, this is the closest bin idx in the FFT for a 1024 point FFT
const int n_bins = high_idx - low_idx + 1; // the number of bins to be processed in the FFT for a 1024 point FFT

const float thresh = 0.1; // squelch threshold
const int hangtime = 2000; // hang time in milliseconds (monoflop)
const int fadeOutTime = 100; // fade out time in milliseconds

bool hang = 0; // boolean to indicate whether the squelch is hanging

float sum;
float mean;
float variance;
float relvar;
float relvar_minus_one;
float scaled_relvar_minus_one;
float actual_thresh = thresh;
bool is_open = false;

float peakfreq = 45678;
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



void autodetect() {
  int i;
  if (myFFT.available()) {

    // each time new FFT data is available

    // calculate the mean of the data
    sum = 0;
    float latest_max = 0;
    int latest_max_idx = 0;

    for (i = low_idx; i <= high_idx; i++) {
      // calculate the mean of the data
      sum += myFFT.read(i); // sum over all bins in the FFT

     // find the peak frequency - so that the heterodyne rx can be tuned to that freq (with a BFO freq of 1 kHz?)
    
     if (myFFT.read(i) > latest_max) {
      // store it as the new max
      latest_max = myFFT.read(i);
      latest_max_idx = i;
      peakfreq = (latest_max_idx/float(FFT_SIZE))*samplerate;
     }
    }
    mean = sum / float(n_bins); // divide the sum by the number of datapoints

    // calculate the sum of the squares of the differences from the mean
    sum = 0; // re-initialize sum
    for (i = low_idx; i <= high_idx; i++) {
      sum += pow((mean - myFFT.read(i)), 2); // sum over all bins in the FFT
    }

    
    

    // from that, calculate the variance
    variance = sum / float(n_bins); // divide the sum by the number of datapoints

    // now calculate the relative variance by dividing the variance by the mean squared
    relvar = variance / (mean * mean);

    // subtract 1 from the relative variance
    relvar_minus_one = relvar - 1;

    // scale the relative variance by the amount of datapoints
    scaled_relvar_minus_one = relvar_minus_one * sqrt(float(n_bins));

    // Serial.println(scaled_relvar_minus_one);

    //     determine if a signal is present, but only when the squelch is not hanging on a previous signal
    if (hang) {
      // do nothing except check whether the max hangtime has been obtained
      if (hangcounter > hangtime) {
        hangcounter = 0;
        hang = 0;
        displaystuff();
      }

    }
    else {
      // determine whether there is a signal
      if ((scaled_relvar_minus_one > actual_thresh) || is_open) {

        //start "hanging"
        hang = 1;
        
  // set the frequency to the peak detected frequency
  freq = peakfreq;
  setfreq();
  displaystuff();
  

 
        fade1.fadeIn(1);
        digitalWrite(led_green, LOW); // LED ON

      }
      else {

        fade1.fadeOut(fadeOutTime);
        digitalWrite(led_green, HIGH); // LED OFF
      }
    }

  }
}


void setmode() {
if (mode == "VLF") {
  samplerate = 96000;
  freq = 17200;
  bfo_freq = 600; // BFO freq in Hz
  freqstep = 100; // tuning step in Hz
  filtfreq = 250; // state variable filter cut off frequency (half the receiver bandwidth)
  hpffiltfreq = 500;
  
}
else if (mode == "BAT") { // simple heterodyne ulstrasonic receiver
  samplerate = 352800;
  freq = 40000;
  bfo_freq = 1000; // BFO freq in Hz - essentially turns it into a double sideband (heterodyne) receiver
  freqstep = 2000; // tuning step in Hz
  filtfreq = 8000; // state variable filter cut off frequency (half the receiver bandwidth)
  hpffiltfreq = 4000;//DEBUG4000;
}
// Serial.println(mode);
}

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
  // AudioNoInterrupts(); // disable audio library momentarily
  // set sample rate
//   setI2SFreq(samplerate);
  
  
  // oscillators
//  // ddc sin
//  sine1.amplitude(1.0);
//  sine1.frequency(freq * samplerate_factor);
//  sine1.phase(0);
//  // ddc cos
//  sine2.amplitude(1.0);
//  sine2.frequency(freq * samplerate_factor);
//  sine2.phase(phaseshift);
//  // bfo sin
//  sine3.amplitude(1.0);
//  sine3.frequency(bfo_freq * samplerate_factor);
//  sine3.phase(phaseshift);
//  // bfo cos
//  sine4.amplitude(1.0);
//  sine4.frequency(bfo_freq * samplerate_factor);
//  sine4.phase(0);

  
  // AudioInterrupts(); // enable audio interrupts again






  
  // initialize the builtin LED digital pin as an output.
  pinMode(led, OUTPUT);

  // encoder, switch and LED pins
  pinMode(0, INPUT_PULLUP); // encoder A
  pinMode(1, INPUT_PULLUP); // encoder B
  pinMode(2, INPUT); // encoder SW
  pinMode(led_blue, OUTPUT); // LED BLUE
  pinMode(led_green, OUTPUT); // LED GREEN
  pinMode(led_red, OUTPUT); // LED RED
  
  digitalWrite(led_blue, HIGH); // LED OFF
  digitalWrite(led_green, HIGH); // LED OFF
  digitalWrite(led_red, HIGH); // LED OFF

  // setup the delay
  delay1.delay(0, delaytime);


  // Configure the window algorithm to use for the FFT
//   FFT1.windowFunction(AudioWindowHanning1024);

  //start FIR filter

  fir1.begin(filter_taps, FILTER_TAP_NUM);
  fir2.begin(filter_taps, FILTER_TAP_NUM);

  Serial.begin(9600);
  attachInterrupt(0, rotate, CHANGE);
  attachInterrupt(1, rotate, CHANGE);
//  Serial.println("samplerate_factor:");
//  Serial.println(samplerate_factor);
  u8g2.begin();

  // checkmode (read ADC)
  checkmode();
  // set mode (set frequency and BFO values)
  setmode();
  setup();
  
// set the samplerate
setI2SFreq(samplerate);
samplerate_factor = 44117.64706 / samplerate;
  
  // init the filters
  setfilts();
  // init the frequency
  setfreq();

  // init the bfo frequency
  setbfofreq();
  // display the frequency
  displaystuff();


  
}


void checkmode() {
  
  long sumModeAdc = 0;
  for (int i = 0; i<n_avg; i++){
    sumModeAdc += analogRead(analogPin);
  }
  float modeAdc = sumModeAdc / n_avg;
  float modeVolt = (float(modeAdc)/1023)*V_ref;

// DEBUG
  // Serial.println(modeVolt);
  if (modeVolt > modeVoltThresh) {
    mode = "VLF"; // @@@DEBUG - change to VLF
    }
  else {
    mode = "BAT";
  }

  // DEBUG
  // mode = "VLF";
  if (mode != previousmode) { // if the mode has changed - then set it
    setmode();
    setfilts();
    setfreq();
    setbfofreq();
    displaystuff();
    previousmode = mode;
  }
}

void setfilts() {
  filt1.frequency(filtfreq);
  filt1.resonance(0.707);

  filt2.frequency(filtfreq);
  filt2.resonance(0.707);

  
  filt3.frequency(filtfreq);
  filt3.resonance(0.707);

  filt4.frequency(filtfreq);
  filt4.resonance(0.707);

  filt5.frequency(filtfreq);
  filt5.resonance(0.707);

  filt6.frequency(filtfreq);
  filt6.resonance(0.707);

  filt7.frequency(filtfreq);
  filt7.resonance(0.707);

  filt8.frequency(filtfreq);
  filt8.resonance(0.707);

  filt9.frequency(hpffiltfreq);
  filt9.resonance(0.707);

}


  void setfreq(){  
   
  AudioNoInterrupts(); // disable interrupts for a while to setup everything, and guarantee accurate phase quadrature
  // ddc sin
  sine1.amplitude(1.0);
  sine1.frequency(freq * samplerate_factor);
  sine1.phase(0);
  // ddc cos
  sine2.amplitude(1.0);
  sine2.frequency(freq * samplerate_factor);
  sine2.phase(phaseshift);  
    AudioInterrupts(); // enable audio interrupts again
  }
void setbfofreq(){
   AudioNoInterrupts(); // disable interrupts for a while to setup everything, and guarantee accurate phase quadrature
  // bfo sin
  sine3.amplitude(1.0);
  sine3.frequency(bfo_freq * samplerate_factor);
  sine3.phase(phaseshift);
  // bfo cos
  sine4.amplitude(1.0);
  sine4.frequency(bfo_freq * samplerate_factor);
  sine4.phase(0);
  AudioInterrupts(); // enable audio interrupts again
}


  void displaystuff(){
  u8g2.clearBuffer(); // clear the internal memory
  u8g2.setFont(u8g2_font_t0_11_tf);  // choose a suitable font
  // u8g2.drawStr(0, 10, "Freq (Hz):"); // write something to the internal memory
  u8g2.setCursor(0, 8); 
  if (mode == "BAT") {
    if (hang) {
    u8g2.print(freq);
    u8g2.print(" Hz");  
    }
    else {
      u8g2.print("-------");  
    }
  }
  else {
  u8g2.print(freq);
  u8g2.print(" Hz");
  }
  
  u8g2.setCursor(0, 20);  
  u8g2.print(dblevel);
  u8g2.print(" dBFS");
  u8g2.setCursor(0, 32); 
  u8g2.print("Mode: ");
  u8g2.print(mode);
  u8g2.sendBuffer(); // transfer internal memory to the display
//  u8g2.setCursor(pixel, 34);  
//  u8g2.print("|");
//  u8g2.sendBuffer(); // transfer internal memory to the display
  
  }

  void clear_display(){
  u8g2.clearBuffer(); // clear the internal memory
  u8g2.sendBuffer();  // transfer internal memory to the display
  }

// rotate is called anytime the rotary inputs change state.
void rotate() {
  unsigned char result = rotary.process();

//  if (digitalRead(2)) { // if rotary button is pressed
//    freqstep = 100; // set tuning step to 100 Hz
//    }
//  else {
//    freqstep = 25; // set tuning step to 25 Hz
//  }
  
  if (result == DIR_CW) {
    freq = freq + freqstep; // increment frequency

  } else if (result == DIR_CCW) {
    freq = freq - freqstep; // decrement ferquency

  }

  // frequency limits
  if (freq < 0) { // frequency lower than 0 Hz
    freq = 0;
    
  }

  if (freq > samplerate/2) {
    freq = floor(samplerate/2); // FIXME - make dependent on sample rate and floor, taking smallest tuning step into account
  }

    
  setfreq(); // set the current frequency
  // delay(100);
  // displaystuff();
// Serial.println(freq);
  }

// set samplerate code by Frank Bösing - taken from DD4WH's convolution SDR code: https://github.com/DD4WH/Teensy-ConvolutionSDR
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

  checkmode();

  if (mode == "BAT") {
    autodetect();
  }
  else {
      fade1.fadeIn(1);
  }

  if (digitalRead(2)) {
    displaystuff();
  }
  // very crude AGC - divide the amplitude by the measured peak, with an empirically determined threshold
  if (peak.available()) {
    float read_level = peak.read();
    float agc = 1 /(30*read_level);

  // cap the AGC value to prevent the receiver from muting completely upon thumbs
  float min_agc = 0.001;
  if (agc < min_agc)
  {
    agc = min_agc; 
  }

      // include a hang time
      if (agc < vol) { // fast attack (instantaneous, actually)
        vol = agc;
        }
        else{ // slow decay
        vol = (1-alpha)*vol + alpha*agc;  // simple 1st order IIR low pass filter
        } // @@@DEBUG - prevent peaks from blocking the AGC - 
      // }
    if (vol > 0.7) {
      vol = 0.7;
    }
    sgtl5000_1.volume(vol);
    

     avg_level = (1-alpha2)*avg_level + alpha2*read_level;
     dblevel = 20*log10(avg_level);
    
    //Serial.println(dblevel);
    // display the level
    // displaystuff();
    
//      if (FFT1.available()) {
//    
//      // each time new FFT data is available
//      // calculate the position of the max the data between the bins of interest
//        float maxval = 0;
//        int maxidx = 0;
//        for (int i = low_idx; i <= high_idx; i++) {
//          if (FFT1.read(i)> maxval){
//            // store a new max value
//            maxval = FFT1.read(i);
//            maxidx = i; // maxidx runs from 0:n_bins
//            
//            }
//        }
//        // map the maxidx to a display pixel, assuming 64 horizontal pixels
//        pixel = map(maxidx, low_idx, high_idx, 0, n_pixels-1);
//        // Serial.println(pixel);
//        
//      }
    
    }
  
  
//bouncer.update ( );
// int value = bouncer.risingEdge();//bouncer.read();
//
//  if (value == HIGH) { // encoder button is pressed
//    //displayfreq();
//    displaystuff();
//  
//
//  }
  
}
