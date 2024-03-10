#ifndef VOSK_VOICE_RECOGNITION_H
#define VOSK_VOICE_RECOGNITION_H

#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <string>
#include <iostream>
#include <algorithm>
#include <streambuf>
#include <fstream>
#include <windows.h>
#include "vosk_api.h"
#include <portaudio.h>
#include <thread>

namespace godot {

    
    

    class VoskVoiceRecognition : public Node2D {
        GDCLASS(VoskVoiceRecognition, Node2D);

        private:
            const int SAMPLE_RATE = 48000;
            const int FRAMES_PER_BUFFER = 512;
            const int NUM_CHANNELS = 1;
            const PaSampleFormat SAMPLE_FORMAT = paFloat32;
            

        protected:
            static void _bind_methods();

        public:
            std::vector<float> accumulated_audio_data;
            bool is_accumulating_audio = false;
            bool is_vosk_listening = false;
            bool initialized = false;
            int status = -2;
            VoskModel *model = nullptr;
            VoskRecognizer *recognizer = nullptr;
            std::thread recognition_thread; // Thread for voice recognition
            int buffer_ms = 100;

            
            VoskVoiceRecognition();
            ~VoskVoiceRecognition();
           
            void cleanup();


            bool initialize(godot::String p_model_path);
            bool start();
            void recognize_in_thread();
            void stop();
            void set_size_in_ms(int miliseconds);

            void accumulate_audio_data(const float* data, unsigned long num_samples);
            bool save_accumulated_audio();
            void write_word(std::ofstream& out, uint32_t word, int num_bytes);
            godot::Array get_input_devices();
            bool set_input_device(int device_index);
            godot::String get_partial();
            godot::String get_final();
            int get_status();
            bool is_listening();
            void set_partial_words(bool value);
            void set_words(bool value);
    };
}
#endif