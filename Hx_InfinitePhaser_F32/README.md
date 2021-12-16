## Shepard/Barberpole Infinite phaser, F32 mono version for OpenAudio_ArduinoLibrary

![alt text][pic1]  

An interesting effect creating an illusion of infinite phasing up or down by running 6 parallel/interleaved 6 stage phaser units.  
Float32 mono version for use with [OpenAudio_ArduinoLibrary](https://github.com/chipaudette/OpenAudio_ArduinoLibrary "OpenAudio_ArduinoLibrary"). 


### Modulation scalling:  
Instead of a common approach of using two LFO cotrols: Rate and Depth, where the modulation waveform oscillates around the middle of the scale, this phaser uses two parameters to control the depth and range of the modulation: **top** and **bottom** values. Input modulation signal will be scaled and shifted to operate in range between these two values.  
Experiments show the most useful LFO rate range is well below 1Hz, hence i've scaled the _rate_ parameter to range from -1.0f (phasing down) to 1.0f (phasing up). Internally it's limited to 0.25Hz and can be changeg in teh header file if desired:  

```#define INFINITE_PHASER_MAX_LFO_HZ  (0.25f) // maximum LFO rate range```


### API:  
  
```void lfo(float32_t rate, float32_t top, float32_t bottom);```  
Controls the internal LFO frequency in scaled range from -1.0 to 1.0, top level (0.0 ... 1.0) and bottom level (0.0 ... 1.0) 
Example:  
```phaser.lfo(0.5f, 0.0f, 1.0f);  // forward 50% speed, full scale```   

```void lfo_rate(float32_t rate);```  
Controls the internal LFO modulation rate. Use if only the speed update is required. 
Example:  
```phaser.lfo_rate(-1.0f);  // full reverse throttle```   

```void depth(float32_t top, float32_t bottom);```  
Scales and offsets the modulation waveform (internal or external).  
Example:  
```phaser.depth(0.4f, 0.8f);  // modulation waveform between 0.4 and 0.8```  

```void depth_top(float32_t top);```  
Individually set the dept top parameter, range 0.0f to 1.0f.  

```void depth_btm(float32_t btm);```  
Individually set the dept bottom parameter, range 0.0f to 1.0f.  

```void feedback(float32_t value);```  
Controls the amount of feedback, range 0.0f to 1.0f.  
Example:  
```phaser.feedback(0.5f);  // set the feedback to 0.5```  

```void mix(float32_t value);```  
Dry / Wet mix ratio. Set to 0.5f for classic phaser sounds. Setting the feedback to 0.0 and the mix to 1.0 (full wet) will produce a vibrato effect.  
Example:  
```phaser.mix(0.3f);  // dry = 0.7, wet = 0.3```  

```void stages(uint8_t st);```  
Controls the number of phase shifter stagtes. Accepted values are: 2, 4, 6. The more stages the more resonant notches are produced.
Example:  
```phaser.stages(6);  // 6 stage phaser```  

```void set_bypass(bool state);```  
Disables (true) or enables (false) the phaser.  
Example:  
```phaser.set_bypass(true);  // disable the phaser (saves CPU load) ```  

```void tgl_bypass(void);```  
Toggles the current bypass status.  

```bool get_bypass(void);```  
Returns the current bypass status.  

### Sound example:  

[![Teensy4 InfinitePhaser_F32](http://img.youtube.com/vi/IxoAJXqS40E/0.jpg)](http://www.youtube.com/watch?v=IxoAJXqS40E)

[pic1]: InfinitePhaser_patch.png "Internal structure"