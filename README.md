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
This repository isn't ready to use in production/prototyping, because is work in progress. When extension will be ready I'll update this README.md accordingly.

### Usage

1. Add Godot VOSK GDExtension to your project (use releases if available or build by yourself).
2. Enable Audio Input in `Project Settings -> Audio -> Driver -> Enable Input` (Advanced Settings).
3. Add `Capture` bus by navigating to bottom section of the editor, then click the `Audio` tab and the `Add bus` button.
4. Under `Capture` bus click the `Add Effect` button and select `Capture` effect. You can also route `Capture` bus to another muted bus to prevent hearing yourself.
5. Add the `AudioStreamPlayer` node to your scene. Set `Stream` as `new AudioStreamMicrophone` and set `AutoPlay` property to `true`.
6. *More info will be added then the demo code will be done.*


### Development
1. `cd godot-cpp`.
2. `python -m SCons`.
3. `cd ..`
4. `python -m Scons`.
5. Run `/demo/godot.project` in Godot.



### Docs

```gdscript
# Available signals
signal vosk_ready_signal(is_ready : bool, error_message : String)
signal vosk_model_loaded_signal(model_path : String)
signal vosk_recognizer_ready_signal(is_ready : bool, error_message : String)



# Methods

# init Vosk with path to model dir
vvr.initVosk(p_model_path : String = "bin/models/...") -> void

# set Vosk log level
vvr.setLogLevel(p_log_level : int) -> void

# set grammar json string
vvr.setWordsJson(p_words_json : String = "[]") -> void

# apply waveform data:
vvr.applyWaveform(p_raw_data : PackedByteArray, p_length : int) -> int

## returns:
# 1 if silence is occured and you can retrieve a new utterance with result method 
# 0 if decoding continues
# -1 if exception occured
# -2 if recognizer isn't instantiated

# get partial results from recognizer
vvr.getPartialResults() -> JSON String

# get full results from recognizer
vvr.getResults() -> JSON String

...
```


### Dependencies

This project is created around [Vosk](https://alphacephei.com/vosk/). Vosk is a great speech recognition toolkit with support for 20+ languages. The best feature of Vosk is it's working offline..

### Models

Vosk models can be found on [Vosk website](https://alphacephei.com/vosk/). Check the [models list here](https://alphacephei.com/vosk/models).


### Todo

-  ✔ Create GDExtension.
-  ✔ Load Vosk library.
-  ✔ Method to load models.
-  ✔ Add ability to provide word list for accuracy.
-  ✔ Get audio with AudioEffectRecord (see `demo`).
-  ✔ Mix to mono if needed (see `demo`).
-  ✔ Pass audio data with `applyWaveform` method.
-  ✔ Get partial text results.
-  ✔ Get full text results.
- ❌ Add clean up code.
- ❌ Add ability to reload model.
- ❌ Auto detect models in dir.
- ❌ Finish the demo project code.
- ❌ Tests the extension.
