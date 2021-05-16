## Mono 12 stage Phaser effect

![alt text][pic1]  

### Modulation sources:  
* internal LFO, switched to if there is no modulation input signal provided (input index [1]). Internal LFO uses a hyperbolic waveform to produce smooth transistion over the whole frequency spectrum.
* external modulation signal fed into input [1]. Range has to be of int16_t (-32768 ... 32767) to cover the whole range.  

### Modulation scalling:  
Instead of a common approach of using two LFO cotrols: Rate and Depth, where the modulation waveform oscillates around the middle of the scale, this phaser uses two parameters to control the depth and range of the modulation: **top** and **bottom** values. Input modulation signal will be scaled and shifted to operate in range between these two values.

### API:  
  
```void lfo(float32_t f_Hz, float32_t top, float32_t bottom);```  
Controls the internal LFO: frequency in Hz, top level (0.0 ... 1.0) and bottom level (0.0 ... 1.0) 
Example:  
```phaser.lfo(0.5f, 0.0f, 1.0f);  // 0.5Hz, full scale```   

```void lfo_rate(float32_t f_Hz);```  
Controls the internal LFO frequency. Use if only the frequency update is required. 
Example:  
```phaser.lfo_rate(1.6f);  // internal LFO: 1.6Hz```   

```void depth(float32_t top, float32_t bottom);```  
Scales and offsets the modulation waveform (internal or external).  
Example:  
```phaser.depth(0.4f, 0.8f);  // modulation waveform between 0.4 and 0.8```  

```void feedback(float32_t value);```  
Controls the amount of feedback, range 0.0f to 1.0f.  
Example:  
```phaser.feedback(0.5f);  // set the feedback to 0.5```  

```void mix(float32_t value);```  
Dry / Wet mix ratio. Set to 0.5f for classic phaser sounds. Setting the feedback to 0.0 and the mix to 1.0 (full wet) will produce a vibrato effect.  
Example:  
```phaser.mix(0.3f);  // dry = 0.7, wet = 0.3```  

```void stages(uint8_t st);```  
Controls the number of phase shifter stagtes. Accepted values are: 2, 4, 6, 8, 10, 12. The more stages the more resonant notches are produced.
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

### Sound sample:  
https://soundcloud.com/hexeguitar/teensy-audio-12-stage-phaser

[pic1]: phaser_internal.png "Internal structure"