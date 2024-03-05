# godot-vosk-gdextension
[WIP] GDExtension for VOSK (Voice Recognition) library. This extension adds ability to convert speech to text in your Godot 4 game.


### State
This repository isn't ready to use in production/prototyping, because is work in progress. When extension will be ready I'll update this README.md accordingly.


### Development
1. `cd godot-cpp`.
2. `python -m SCons`.
3. `cd ..`
4. `python -m Scons`.
5. Run `/demo/godot.project` in Godot.


### Docs

```
# Available signals
signal vosk_ready_signal(is_ready : bool, message : String)

# Methods
vosk_voice_recognition.init(model_path : String = "bin/models/...") -> void # init Vosk with path to model dir
vosk_voice_recognition.setLogLevel(int log_level) -> void # set Vosk log level
...
```