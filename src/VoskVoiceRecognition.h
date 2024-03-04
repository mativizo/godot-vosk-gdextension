#ifndef VOSK_VOICE_RECOGNITION_H
#define VOSK_VOICE_RECOGNITION_H

#include <godot_cpp/classes/node2d.hpp>

namespace godot {
    class VoskVoiceRecognition : public Node2D {
        GDCLASS(VoskVoiceRecognition, Node2D);

        private:
            bool recognizing = false;

        protected:
            static void _bind_methods();

        public:
            // constructor
            VoskVoiceRecognition();

            // destructor
            ~VoskVoiceRecognition();
            void voice_recognition();

            void _process(double delta);
    };
}


#endif