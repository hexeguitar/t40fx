/**
   Example project for the Stereo Plate reverb audio component
   (c) 31.12.2020 by Piotr Zapart www-hexefx.com

   Attention!!! Works with Teensy 4.x only!

   The audio path follows a typical scheme used in mixing consoles
   where the reverb is put into aux path.
   Each source (like i2s or PlaySDWav etc) has a Reverb Send Level control
   The Stereo reverb output is mixed then with the dry signals using the output mixers.


*/

#include <Arduino.h>
#include "Audio.h"
#include "effect_platervbstereo.h"

#define I2S_REVERB_SEND_CH      0
#define SDWAV_REVERB_SEND_CH    1
#define REVERB_MIX_CH           1
#define I2S_MIX_CH              0

AudioPlaySdWav           playSdWav;
AudioInputI2S            i2s_in;
AudioMixer4              reverb_send_L;
AudioMixer4              reverb_send_R;
AudioEffectPlateReverb   reverb;
AudioMixer4              mixer_out_L;
AudioMixer4              mixer_out_R;
AudioOutputI2S           i2s_out;

AudioConnection          patchCord1(playSdWav, 0, reverb_send_L, 1);    // wav player L reverb send
AudioConnection          patchCord2(playSdWav, 0, mixer_out_L, 2);      // wav player L into output mixer
AudioConnection          patchCord3(playSdWav, 1, reverb_send_R, 1);    // wav player R reverb send
AudioConnection          patchCord4(playSdWav, 1, mixer_out_R, 2);      // wav player R into output mixer

AudioConnection          patchCord5(i2s_in, 0, mixer_out_L, 0);         // i2s out L into output mixer
AudioConnection          patchCord6(i2s_in, 1, mixer_out_R, 0);         // i2s out R into output mixer

AudioConnection          patchCord7(i2s_in, 0, reverb_send_L, 0);       // i2s out reverb send L
AudioConnection          patchCord8(i2s_in, 1, reverb_send_R, 0);      // i2s out reverb send R

AudioConnection          patchCord9(reverb_send_L, 0, reverb, 0);      // reverb inputs
AudioConnection          patchCord10(reverb_send_R, 0, reverb, 1);

AudioConnection          patchCord11(reverb, 0, mixer_out_L, 1);        // reverb out into output mixer
AudioConnection          patchCord12(reverb, 1, mixer_out_R, 1);

AudioConnection          patchCord13(mixer_out_L, 0, i2s_out, 0);       // output mixers -> codec DAC
AudioConnection          patchCord14(mixer_out_R, 0, i2s_out, 1);

AudioControlSGTL5000     codec;


uint32_t timeLast = 0, timeNow = 0;

void flexRamInfo(void);
void i2s_set_rev_send(float32_t lvl);
void reverb_set_volume(float32_t lvl);
void wav_set_rev_send(float32_t lvl);

void setup()
{
  Serial.begin(115200);
  //while(!Serial);
  delay(1000);
  Serial.println("--------------------------");
  Serial.println("T40_GFX - stereo plate reverb");
#ifdef REVERB_USE_DMAMEM
  Serial.println("DMAMEM is used for reverb buffers");
#endif
  AudioMemory(12);
  codec.enable();
  codec.volume(0.0); // headphones not used
  codec.inputSelect(AUDIO_INPUT_LINEIN);
  codec.lineInLevel(2);
  codec.lineOutLevel(31);
  flexRamInfo();

  i2s_set_rev_send(0.7);
  reverb_set_volume(0.6);

  reverb.size(1.0);     // max reverb length
  reverb.lowpass(0.3);  // sets the reverb master lowpass filter
  reverb.lodamp(0.1);   // amount of low end loss in the reverb tail
  reverb.hidamp(0.2);   // amount of treble loss in the reverb tail
  reverb.diffusion(1.0);  // 1.0 is the detault setting, lower it to create more "echoey" reverb

}

void loop()
{
  timeNow = millis();
  if (timeNow - timeLast > 1000)
  {
    Serial.print("Reverb CPU load = ");
    Serial.println(reverb.processorUsageMax());
    timeLast = timeNow;
  }

}


void flexRamInfo(void)
{ // credit to FrankB, KurtE and defragster !
#if defined(__IMXRT1052__) || defined(__IMXRT1062__)
  int itcm = 0;
  int dtcm = 0;
  int ocram = 0;
  Serial.print("FlexRAM-Banks: [");
  for (int i = 15; i >= 0; i--)
  {
    switch ((IOMUXC_GPR_GPR17 >> (i * 2)) & 0b11)
    {
      case 0b00:
        Serial.print(".");
        break;
      case 0b01:
        Serial.print("O");
        ocram++;
        break;
      case 0b10:
        Serial.print("D");
        dtcm++;
        break;
      case 0b11:
        Serial.print("I");
        itcm++;
        break;
    }
  }
  Serial.print("] ITCM: ");
  Serial.print(itcm * 32);
  Serial.print(" KB, DTCM: ");
  Serial.print(dtcm * 32);
  Serial.print(" KB, OCRAM: ");
  Serial.print(ocram * 32);
#if defined(__IMXRT1062__)
  Serial.print("(+512)");
#endif
  Serial.println(" KB");
  extern unsigned long _stext;
  extern unsigned long _etext;
  extern unsigned long _sdata;
  extern unsigned long _ebss;
  extern unsigned long _flashimagelen;
  extern unsigned long _heap_start;

  Serial.println("MEM (static usage):");
  Serial.println("RAM1:");

  Serial.print("ITCM = FASTRUN:      ");
  Serial.print((unsigned)&_etext - (unsigned)&_stext);
  Serial.print("   ");
  Serial.print((float)((unsigned)&_etext - (unsigned)&_stext) / ((float)itcm * 32768.0) * 100.0);
  Serial.print("%  of  ");
  Serial.print(itcm * 32);
  Serial.print("kb   ");
  Serial.print("  (");
  Serial.print(itcm * 32768 - ((unsigned)&_etext - (unsigned)&_stext));
  Serial.println(" Bytes free)");

  Serial.print("DTCM = Variables:    ");
  Serial.print((unsigned)&_ebss - (unsigned)&_sdata);
  Serial.print("   ");
  Serial.print((float)((unsigned)&_ebss - (unsigned)&_sdata) / ((float)dtcm * 32768.0) * 100.0);
  Serial.print("%  of  ");
  Serial.print(dtcm * 32);
  Serial.print("kb   ");
  Serial.print("  (");
  Serial.print(dtcm * 32768 - ((unsigned)&_ebss - (unsigned)&_sdata));
  Serial.println(" Bytes free)");

  Serial.println("RAM2:");
  Serial.print("OCRAM = DMAMEM:      ");
  Serial.print((unsigned)&_heap_start - 0x20200000);
  Serial.print("   ");
  Serial.print((float)((unsigned)&_heap_start - 0x20200000) / ((float)512 * 1024.0) * 100.0);
  Serial.print("%  of  ");
  Serial.print(512);
  Serial.print("kb");
  Serial.print("     (");
  Serial.print(512 * 1024 - ((unsigned)&_heap_start - 0x20200000));
  Serial.println(" Bytes free)");

  Serial.print("FLASH:               ");
  Serial.print((unsigned)&_flashimagelen);
  Serial.print("   ");
  Serial.print(((unsigned)&_flashimagelen) / (2048.0 * 1024.0) * 100.0);
  Serial.print("%  of  ");
  Serial.print(2048);
  Serial.print("kb");
  Serial.print("    (");
  Serial.print(2048 * 1024 - ((unsigned)&_flashimagelen));
  Serial.println(" Bytes free)");

#endif
}

void i2s_set_rev_send(float32_t lvl)
{
  lvl = constrain(lvl, 0.0, 1.0);
  reverb_send_L.gain(I2S_REVERB_SEND_CH, lvl);
  reverb_send_R.gain(I2S_REVERB_SEND_CH, lvl);
}


void reverb_set_volume(float32_t lvl)
{
  lvl = constrain(lvl, 0.0, 1.0);
  mixer_out_L.gain(REVERB_MIX_CH, lvl);
  mixer_out_R.gain(REVERB_MIX_CH, lvl);
}

void wav_set_rev_send(float32_t lvl)
{
  lvl = constrain(lvl, 0.0, 1.0);
  reverb_send_L.gain(SDWAV_REVERB_SEND_CH, lvl);
  reverb_send_R.gain(SDWAV_REVERB_SEND_CH, lvl); 
}
