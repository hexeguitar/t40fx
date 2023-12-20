## Stereo Plate Reverb
Fully stereo in/out guitar tone stack component for the [OpenAudio_ArduinoLibrary](https://github.com/chipaudette/OpenAudio_ArduinoLibrary "OpenAudio_ArduinoLibrary").  
Emulates 9 different EQ models from various guitar amplifiers.  
Code is based on implementation created by David Yeh and Tim Goetze.  

### Connections:  
ToneStack requires stereo in and out connenctions.  
### API:  
  
```void setModel(toneStack_presets_e m);```  
set one of the 9 available EQ models, use `TONESTACK_OFF` to bypass the module.  
Available models:  
* TONESTACK_OFF,
* TONESTACK_BASSMAN,
* TONESTACK_PRINCE,
* TONESTACK_MESA,
* TONESTACK_VOX,
* TONESTACK_JCM800,
* TONESTACK_TWIN,
* TONESTACK_HK,
* TONESTACK_JAZZ,
* TONESTACK_PIGNOSE  
  
Example:  
```tonestack.model(TONESTACK_BASSMAN);  // set EQ model to Fender Bassman```   

```const char *getName()``` 
returns a pointer to the currently used EQ model name as char array.  
Example:  
```Serial.println(tonestack.getName());  // print the preset name```  

```void setTone(float32_t b, float32_t m, float32_t t);```  
set Bass (b), Mid (m) and Treble (t) at once. Range 0.0f to 1.0f  
Example:  
```tonestack.setTone(0.5f, 0.5f, 0.5f);  // set all controls to 50% ```  

```void setBass(float32_t b);```  
set the bass control. Parameter range: 0.0 to 1.0.  
Example:  
```tonestack.setBass(0.7f);```  

```void setMid(float32_t m);```  
set the middle control. Parameter range: 0.0 to 1.0.  
Example:  
```tonestack.setMid(0.2f);```  

```void setTreble(float32_t t);```  
set the treble control. Parameter range: 0.0 to 1.0.  
Example:  
```tonestack.setTreble(0.3f);```  

```void setGain(float32_t g);```  
set the gain/volume.  
Example:  
```tonestack.setGain(1.5f);```  

Typical application using OpenAudio_ArduinoLibrary:  
![alt text][pic1]  

### Sound Sample  

https://www.youtube.com/watch?v=CsFPGtq5eNM

[pic1]: ToneStack_F32.png "Stereo Tone Stack connections"