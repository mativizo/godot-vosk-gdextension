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
            PaStream *stream;
            
            // Constructor
            VoskVoiceRecognition();
            ~VoskVoiceRecognition();
                       
            // Voice Recognition
            bool initialize(godot::String p_model_path);
            bool switch_model_and_init(godot::String model_path);            
            godot::Dictionary get_final_result();
            godot::Dictionary get_partial_result();
            void initialize_portaudio();
            void stop();
            void recognize_in_thread();
            bool start();
            
            // Status
            bool is_listening();
            int get_status();
            
            // Config
            void set_size_in_ms(int miliseconds);
            void set_logs(bool logs = false);
            void set_accumulate_audio_data(bool enable);
            void set_partial_words(bool value);
            void set_words(bool value);

            // Input devices
            godot::Dictionary get_current_input_device_info();
            bool set_input_device(int device_index);
            godot::Array get_input_devices();
            godot::Dictionary get_input_device_info_by_id(int device_index);

            // Utils
            void cleanup();
            void p(godot::String type, godot::String message, godot::String source);
            void accumulate_audio_data(const float* data, unsigned long num_samples);
            bool save_accumulated_audio();
            void write_word(std::ofstream& out, uint32_t word, int num_bytes);           
            godot::Dictionary convert_to_dict(std::string output);
            
    };
}
#endif