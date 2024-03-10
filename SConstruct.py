import os
import sys

env = SConscript("godot-cpp/SConstruct")

env.Append(CPPPATH=["src/"])
env.Append(LIBPATH=["src/vosk/", "src/portaudio/lib/x64/Release"])
env.Append(LIBS=["libvosk", "portaudio_x64"])


sources = Glob("src/*.cpp")

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "demo/bin/VoskVoiceRecognition.{}.{}.framework/libgdexample.{}.{}".format(
            env["platform"], env["target"], env["platform"], env["target"]
        ),
        source=sources,
    )
else:
    library = env.SharedLibrary(
        "demo/bin/VoskVoiceRecognition{}{}".format(env["suffix"], env["SHLIBSUFFIX"]),
        source=sources,
    )

Default(library)