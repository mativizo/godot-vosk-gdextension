#include "VoskVoiceRecognition.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <vosk/vosk_api.h>

using namespace godot;

void VoskVoiceRecognition::_bind_methods() {
    ADD_SIGNAL(MethodInfo("vosk_ready_signal", PropertyInfo(Variant::BOOL, "ready"), PropertyInfo(Variant::STRING, "error")));
    ClassDB::bind_method(D_METHOD("init", "model_name", "sample_rate"), &VoskVoiceRecognition::init);
    ClassDB::bind_method(D_METHOD("voice_recognition"), &VoskVoiceRecognition::voice_recognition);
}

VoskVoiceRecognition::VoskVoiceRecognition() {
    if (Engine::get_singleton()->is_editor_hint()) {
        set_process_mode(Node::ProcessMode::PROCESS_MODE_DISABLED);
    }
}

VoskVoiceRecognition::~VoskVoiceRecognition() {}

void VoskVoiceRecognition::init(String model_name = "", double sample_rate = 16000.0f) {
    String model_path = "res://models/" + model_name;
    VOSK_API_H::VoskModel *model = vosk_model_new(model_path.utf8().get_data());
    if (model == nullptr) {
        emit_signal("vosk_ready_signal", false, "Model" + model_name + "(" + model_path + ") not found.");
        return;
    }
    recognizer = vosk_recognizer_new(model, sample_rate);
    if (recognizer == nullptr) {
        emit_signal("vosk_ready_signal", false, "Model" + model_name + "(" + model_path + ") not loaded.");
        return;
    }
    emit_signal("vosk_ready_signal", true, "Model" + model_name + "(" + model_path + ") loaded.");
    return;
}

void VoskVoiceRecognition::voice_recognition() {}

void VoskVoiceRecognition::_process(double delta) {}