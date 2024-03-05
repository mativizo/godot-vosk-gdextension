#include "VoskVoiceRecognition.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <vosk/vosk_api.h>

using namespace godot;

void VoskVoiceRecognition::_bind_methods() {
    ClassDB::bind_method(D_METHOD("init", "model_path"), &VoskVoiceRecognition::init);
    ClassDB::bind_method(D_METHOD("setLogLevel", "log_level"), &VoskVoiceRecognition::setLogLevel);
    ClassDB::bind_method(D_METHOD("voice_recognition"), &VoskVoiceRecognition::voice_recognition);

    ADD_SIGNAL(MethodInfo("vosk_ready_signal", PropertyInfo(Variant::BOOL, "ready"), PropertyInfo(Variant::STRING, "error")));
    ADD_SIGNAL(MethodInfo("vosk_model_loaded", PropertyInfo(Variant::STRING, "model_path")));
    ADD_SIGNAL(MethodInfo("vosk_recognizer_ready", PropertyInfo(Variant::BOOL, "ready"), PropertyInfo(Variant::STRING, "error")));
}

VoskVoiceRecognition::VoskVoiceRecognition() {
    if (Engine::get_singleton()->is_editor_hint()) {
        set_process_mode(Node::ProcessMode::PROCESS_MODE_DISABLED);
    }

    

    // set default log level
    vosk_set_log_level(voskLogLevel);
}

VoskVoiceRecognition::~VoskVoiceRecognition() {}

void VoskVoiceRecognition::init(String model_path = "") {
    model = vosk_model_new(model_path.utf8().get_data());
    if (model == nullptr) {
        emit_signal("vosk_ready_signal", false, "Model" + model_path + " not found.");
        return;
    }

    emit_signal("vosk_model_loaded", model_path);

    recognizer = vosk_recognizer_new(model, sample_rate);
    if (recognizer == nullptr) {
        emit_signal("vosk_ready_signal", false, "Model" + model_path + " not loaded.");
        emit_signal("vosk_recognizer_ready_signal", false, "Failed to initialize voice recognizer.");
        return;
    }
    emit_signal("vosk_ready_signal", true, "Model" +  model_path + " loaded.");
    emit_signal("vosk_recognizer_ready_signal", true, "Voice recognizer is ready to use.");
    return;
}

void VoskVoiceRecognition::voice_recognition() {}

void VoskVoiceRecognition::_process(double delta) {}

void VoskVoiceRecognition::setLogLevel(int log_level) {
    voskLogLevel = log_level;
    vosk_set_log_level(log_level);
}