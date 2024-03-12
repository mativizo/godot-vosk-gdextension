#ifndef VOSK_VOICE_RECOGNITION_H
#define VOSK_VOICE_RECOGNITION_H

#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <string>

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
            int selected_input_device_id = -1;
            bool is_accumulating_audio = false;
            bool is_vosk_listening = false;
            bool logs_enabled = false;
            bool initialized = false;
            bool do_accumulate_audio_data = false;
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
            godot::Dictionary get_input_device_info_by_id(int device_index);
            bool set_input_device(int device_index);
            int get_status();
            bool is_listening();
            void set_partial_words(bool value);
            void set_words(bool value);
            
            void p(godot::String type, godot::String message, godot::String source);
            godot::Dictionary get_current_input_device_info();
            void set_logs(bool logs = false);
            void set_accumulate_audio_data(bool enable);
            bool switch_model_and_init(godot::String model_path);

            godot::String get_final_result();
            godot::String get_partial_result();
            void initialize_portaudio();
    };
}
#endif