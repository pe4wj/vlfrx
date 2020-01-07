# vlfrx
VLF receiver for the Teensy MCU + audio shield.

Having visited Grimeton radio station (SAQ) in Sweden in August 2019, I came up with the idea to build a dedicated receiver.

Inspired by SAQRX by SM6LKM (https://sites.google.com/site/sm6lkm/saqrx/) but I did not want to use the PC for this. Instead, I opted for using a Teensy 4.0 MCU and its companion audio shield.

Instead of using complex low pass filters as done in SAQRX, the complex multiplication is done after the I and Q filters. This is essentially Weaver's method of single sideband demodulation.

![Optional Text](../master/vlfrx.png)

The signal processing code has been generated by the excellent Teensy Audio System Design tool (https://www.pjrc.com/teensy/gui/) - note that the AGC was added later in code and is not shown here.

![Optional Text](../master/audio_design_tool.png)

The Teensy audio library contains a wealth of signal processing functions. There is no direct function for generation of a cosine wave, however phase can be specified. Therefore, the cosine signals are made by generating a sine wave with a phase of 90 degrees. A very crude AGC has been included to protect the listener's ears. This basically divides the signal amplitude by the measured RMS value over a number of samples (by using an RMS detector from the audio library).

The Teensy + audio shield was built into a small enclosure including a TDA2822 audio amplifier, OLED display and rotary encoder. The small box to the right is an active ("E-field probe") antenna based on a BF245 JFET which is biased by the microphone bias voltage coming from the audio shield. The actual antenna is a wire of approx 25 cm formed into a loop and soldered into a 2 mm banana socket. The antenna contains a "ground" terminal for attaching a ground wire. Normally, holding the antenna in the air while touching the metal audio connector shield provides plenty of signal.

![Optional Text](../master/vlfrx.jpg)
