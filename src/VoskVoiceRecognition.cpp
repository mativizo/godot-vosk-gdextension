#include "VoskVoiceRecognition.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>

using namespace godot;

void VoskVoiceRecognition::_bind_methods() {

}

VoskVoiceRecognition::VoskVoiceRecognition() {
    // Run only in scene
    if (Engine::get_singleton()->is_editor_hint()) {
        set_process_mode(Node::ProcessMode::PROCESS_MODE_DISABLED);
    }
}

VoskVoiceRecognition::~VoskVoiceRecognition() {
    // cleanup
}

void VoskVoiceRecognition::voice_recognition() {
    // do voice recognition here
}

void VoskVoiceRecognition::_process(double delta) {
    // do something every frame
}