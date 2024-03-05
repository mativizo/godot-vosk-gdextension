#ifndef VOSK_VOICE_RECOGNITION_H
#define VOSK_VOICE_RECOGNITION_H

#include <godot_cpp/classes/node2d.hpp>
#include <vosk/vosk_api.h>

namespace godot {
    class VoskVoiceRecognition : public Node2D {
        GDCLASS(VoskVoiceRecognition, Node2D);

        private:
            bool recognizing = false;
            VoskRecognizer *recognizer;
            VoskModel *model;
            int voskLogLevel = 2;
            double sample_rate = 16000;

        protected:
            static void _bind_methods();

        public:
            // constructor
            VoskVoiceRecognition();

            // destructor
            ~VoskVoiceRecognition();

            void init(String model_path);
            void setLogLevel(int log_level);
            void voice_recognition();

            void _process(double delta);
    };
}


#endif