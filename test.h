#ifndef TEST_H
#define TEST_H

#include <string>
#include <iostream>
#include <algorithm>
#include <streambuf>
#include <fstream>
#include <windows.h>
#include <vosk_api.h>
#include <portaudio.h>

namespace godot {
    class VoskVoiceRecognition {
        //GDCLASS(VoskVoiceRecognition, Node2D);

        private:
            const int SAMPLE_RATE = 16000;
            const int FRAMES_PER_BUFFER = 1024;
            const int NUM_CHANNELS = 1;
            const PaSampleFormat SAMPLE_FORMAT = paFloat32;
            

        protected:
            static void _bind_methods();

        public:
            bool is_listening = false;
            VoskModel *model = nullptr;
            VoskRecognizer *recognizer = nullptr;
            
            
            // constructor
            VoskVoiceRecognition();

            // destructor
            ~VoskVoiceRecognition();



            bool start(const char p_model_path[]);
            void p(const char p_text[]);
            void cleanup();
            bool initialize(const char p_model_path[]);
            // int audioCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData);

    };
}


#endif