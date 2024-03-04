#include "VoskVoiceRecognition.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <vosk-win64-0.3.45/vosk_api.h>

using namespace godot;

void VoskVoiceRecognition::_bind_methods() {
    ADD_SIGNAL(MethodInfo("vosk_ready_signal", PropertyInfo(Variant::BOOL, "ready")));
    ClassDB::bind_method(D_METHOD("init"), &VoskVoiceRecognition::init);
    ClassDB::bind_method(D_METHOD("voice_recognition"), &VoskVoiceRecognition::voice_recognition);
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

void VoskVoiceRecognition::init() {
    emit_signal("vosk_ready_signal", true);
}

void VoskVoiceRecognition::voice_recognition() {
    // do voice recognition here
}

void VoskVoiceRecognition::_process(double delta) {
    // do something every frame
}