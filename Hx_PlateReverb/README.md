## Stereo Plate Reverb
Fully stereo in/out reverb component for the standard 16bit Audio library.  

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

```void set_bypass(bool state);```  
Disables (true) or enables (false) the reverb engine.  
Example:  
```reverb.set_bypass(true);  // disable the reverb (saves CPU load) ```  

```bool get_bypass(void);```  
Returns the current reverb bypass status.

```void tgl_bypass(void);```  
Toggles the current reverb bypass status. 

Audio connections used in the example project:  
![alt text][pic1]  

### Additional config:  

by default the reverb places it's buffers into OCRAM/DMAMEM region.  
Comment out the  
```#define REVERB_USE_DMAMEM```  
line in the ```effect_platervbstereo.h``` file to place the variables into the DCTM ram region.

[pic1]: StereoPlateReverb.png "Stereo plate reverb connections"