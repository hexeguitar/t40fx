# t40fx
Teensy4.0 Audio Lib Components:  

## Stereo Plate Reverb
---
Fully stereo in/out reverb component for the standard 16bit Audio library.  

### Connections:  
Reverb requires stereo in and out connenctions.  
### API:  
  
```void size(float32_t n);```  
sets the reverb time. Parameter range: 0.0 to 1.0.  
Example:  
```reverb.size(1.0);  // set the reverb time to maximum```   


```void lowpass(float32_t n);```  
sets the reverb master lowpass filter. Parameter range: 0.0 to 1.0.  
Example:  
```reverb.lowpass(0.7);  // darken the reverb sound```  


```void hidamp(float32_t n);```  
sets the treble loss. Parameter range: 0.0 to 1.0.  
Example:  
```reverb.hidamp(1.0);  // max hi band dampening results in darker sound ```  


```void lodamp(float32_t n);```  
sets the bass cut. Parameter range: 0.0 to 1.0.  
Example:  
```reverb.lodamp(0.5);  // cut more bass in the reverb tail to make the sound brighter ```  

Audio connections used in the exmaple project:  
![alt text][pic1]  

### Additional config:  

by default the reverb places it's buffers into OCRAM/DMAMEM region.  
Comment out the  
```#define REVERB_USE_DMAMEM```  
line in the ```effect_platervbstereo.h``` file to place the variables into the DCTM ram region.
___

Copyright 12.2020 by Piotr Zapart  
www.hexefx.com


[pic1]: effect_platervbstereo/StereoPlateReverb.png "Stereo plate reverb connections"