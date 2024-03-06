# godot-vosk-gdextension
[WIP] GDExtension for VOSK (Voice Recognition) library. This extension adds ability to convert speech to text in your Godot 4 game.

1. [State](#state).
2. [Development](#development).
3. [Docs](#docs).
4. [Dependencies](#dependencies).
5. [Models](#models).
6. [Todo](#todo).



### State
This repository isn't ready to use in production/prototyping, because is work in progress. When extension will be ready I'll update this README.md accordingly.

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
vosk_voice_recognition.initVosk(p_model_path : String = "bin/models/...") -> void

# set Vosk log level
vosk_voice_recognition.setLogLevel(p_log_level : int) -> void

# set grammar json string
vosk_voice_recognition.setWordsJson(p_words_json : String = "[]") -> void

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
- 🚧 Get audio buffer to work with.
- 🚧 Pass audio converted audio buffer or convert it on the GDE side.
- ❌ Get partial text results.
- ❌ Get full text results.