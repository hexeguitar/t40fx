## Stereo Plate Reverb
Fully stereo in/out reverb component for the [OpenAudio_ArduinoLibrary](https://github.com/chipaudette/OpenAudio_ArduinoLibrary "OpenAudio_ArduinoLibrary").  

### Connections:  
Reverb requires stereo in and out connenctions.  
### API:  
  
```void size(float32_t n);```  
sets the reverb time. Parameter range: 0.0 to 1.0.  
Example:  
```reverb.size(1.0f);  // set the reverb time to maximum```   

```void lowpass(float32_t n);```  
sets the reverb master lowpass filter. Parameter range: 0.0 to 1.0.  
Example:  
```reverb.lowpass(0.7f);  // darken the reverb sound```  

```void hidamp(float32_t n);```  
sets the treble loss. Parameter range: 0.0 to 1.0.  
Example:  
```reverb.hidamp(1.0f);  // max hi band dampening results in darker sound ```  

```void lodamp(float32_t n);```  
sets the bass cut. Parameter range: 0.0 to 1.0.  
Example:  
```reverb.lodamp(0.5f);  // cut more bass in the reverb tail to make the sound brighter ```  

```void diffusion(float32_t n);```  
diffision controls the density of the reverb tail, lower values add more echo type reflections, higher values produce a lush rich reverb tail. Useful for creating a different type of reverbs: ie. with size set to 0 and diffusion to something in range 0.0f-0.3f the result will be like a ping-pong delay with room reverb.  
Example:  
```reverb.density(0.5f);  // alter the allpass coefficients to change the reverb sound ```  

```void freeze(bool state);```  
Cuts off the input signal and increases the reverb time coeff to 1.0, creating an infinite reverb. Combined with low diffusion settings might produce clicking, so use with caution.  
Example:  
```reverb.freeze(true);  // turn freeze on ```  

```bool freeze_tgl(void);```  
Toggles the freze state and retunrs the current state. 
Example:  
```Serial.printf("Freeze = %d", reverb.freeze_tgl());  // toggle the freeze and print the current state ```  

```bool freeze_get(void);```  
Returns the current freeze state. 

```void set_bypass(bool state);```  
Disables (true) or enables (false) the reverb engine.  
Example:  
```reverb.set_bypass(true);  // disable the reverb (saves CPU load) ```  

```bool get_bypass(void);```  
Returns the current reverb bypass status.

```void tgl_bypass(void);```  
Toggles the current reverb bypass status. 

Typical application using OpenAudio_ArduinoLibrary:  
![alt text][pic1]  

### Additional config:  

by default the reverb places it's buffers into OCRAM/DMAMEM region.  
Comment out the  
```#define REVERB_F32_USE_DMAMEM```  
line in the ```effect_platervbstereo_F32.h``` file to place the variables into the DCTM ram region.

[pic1]: plateReverb_schm.png "Stereo plate reverb connections"