## Mono to Stereo expander  
### 32bit float version for [OpenAudio_ArduinoLibrary](https://github.com/chipaudette/OpenAudio_ArduinoLibrary "OpenAudio_ArduinoLibrary")  

### Inputs:  
* mono in,  _AudioConnection_F32_ type  

### Outputs:
* left out, _AudioConnection_F32_ type 
* right out, _AudioConnection_F32_ type  

### Test patch:  
![alt text][pic1]  

### API:  

```void setSpread(float32_t val);```  
amount of stereo spread. Parameter range: 0.0f to 1.0f.  
Example:  
```monoToStereo.setSpread(1.0f);  // apply max stereo spread ```  

```void setPan(float32_t val);```  
panorama setting. Parameter range: -1.0f (max left) to 1.0f (max right), 0.0f = centre.  
Example:  
```monoToStereo.setPan(-0.5f);  // set the panorama to 25% left ```  

```void setBypass(bool state);```  
bypass setting: _false_ = effect **ON**, _true_ = effect **OFF**   

```void tglBypass(void);```  
toggles the bypass setting.   

```bool getBypass(void);```  
returns the current bypass setting.  

### Sound example:  

[![Teensy4 monoToStereo_F32](http://img.youtube.com/vi/y2SUNxpsVs0/0.jpg)](http://www.youtube.com/watch?v=y2SUNxpsVs0)



[pic1]: monoToStereo_F32_testPatch.png "Test patch"