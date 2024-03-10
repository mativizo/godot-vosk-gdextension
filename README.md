# godot-vosk-gdextension
[WIP] GDExtension for VOSK (Voice Recognition) library. This extension adds ability to convert speech to text in your Godot 4 game.

1. [State](#state).
2. [Usage](#usage).
3. [Development](#development).
4. [Docs](#docs).
5. [Dependencies](#dependencies).
6. [Models](#models).
7. [Todo](#todo).



### State
Build can be used in debug projects.

**Tested with**:

| Godot Engine | OS |
| ------------ | -- |
| 4.3 dev4 | Windows 10 |


This repository isn't ready to use in production/prototyping, because is work in progress. When extension will be ready I'll update this README.md accordingly.

### Usage

Soon. Check [demo](https://github.com/mativizo/godot-vosk-gdextension/tree/main/demo) directory.


### Development
1. `cd godot-cpp`.
2. `python -m SCons`.
3. `cd ..`
4. `python -m Scons`.
5. Run `/demo/godot.project` in Godot.



### Docs

```gdscript
# Available signals
# Will be added soon.


var vvr : VoskVoiceRecognition
func _ready():
    vvr = VoskVoiceRecognition.new()
    add_child(vvr)


# Methods

# init Vosk with path to model dir
var init_status = vvr.initialize(model_path : String = "bin/models/...") -> bool
if init_status == true: # initialized successfully
else: # error occured, check logs

# enable partial words options
vvr.set_partial_words(value : bool) -> void

# enable final words options
vvr.set_words(value : bool) -> void

# set data chunk size in ms:
vvr.set_buffer_ms(miliseconds : float) -> void

# get available input devices
vvr.get_input_devices() -> godot::Array

# set prefered input device
vvr.set_input_device(device_index : int) -> bool

# example
var devices = vvr.get_input_devices()
vvr.set_input_device(devices[1])

# check if extension is listening
vvr.is_listening() -> bool

# save to wav file
vvr.save_accumulated_data() -> bool



# More functions will be implemented soon
...
```


### Dependencies

##### VOSK (Voice Recognition)
This project is created around [Vosk](https://alphacephei.com/vosk/). Vosk is a great speech recognition toolkit with support for 20+ languages. The best feature of Vosk is it's working offline..

##### PortAudio
To capture and manipulate the audio data I've used `PortAudio`.
Sadly, the `AudioEffectRecord` in Godot doesn't work for me (empty data) and the `AudioEffectCapture` was too hard to work with. All the logic was movent into C++ class.

### Models

Vosk models can be found on [Vosk website](https://alphacephei.com/vosk/). Check the [models list here](https://alphacephei.com/vosk/models).


### Todo

-  ✔ Create GDExtension.
-  ✔ Load Vosk & PortAudio library.
-  ✔ Method to load models.
- ❌ Method to switch/reload model. 
- ❌ Add ability to provide word list for accuracy.
-  ✔ Get partial text results.
-  ✔ Get full text results.
-  ✔ dd clean-up code.
- ❌ Auto detect models in dir.
- ❌ Finish the demo project code.
- ❌ Tests the extension.
