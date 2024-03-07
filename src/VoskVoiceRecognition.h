#ifndef VOSK_VOICE_RECOGNITION_H
#define VOSK_VOICE_RECOGNITION_H

#include <godot_cpp/classes/node2d.hpp>
#include <vosk/vosk_api.h>

namespace godot {
    class VoskVoiceRecognition : public Node2D {
        GDCLASS(VoskVoiceRecognition, Node2D);

        private:
            VoskRecognizer *recognizer;
            VoskModel *model;
            int vosk_log_level = 2;
            double sample_rate = 16000.0f;

        protected:
            static void _bind_methods();

        public:
            // constructor
            VoskVoiceRecognition();

            // destructor
            ~VoskVoiceRecognition();

            void initVosk(String p_model_path);
            void setLogLevel(int p_log_level);
            void setWordsJson(String p_words_json);
            int acceptWaveform(PackedVector2Array p_raw_data, float current_sample_rate);
            String getResults();
            String getPartialResults();

            void _convertToMono(PackedVector2Array &p_raw_data);
            bool _isStereo(const PackedVector2Array &p_raw_data);   

            void _resampleTo16k(PackedVector2Array &p_raw_data, float current_sample_rate);
                     
            void _process(double delta);
    };
}


#endif