#include "VoskVoiceRecognition.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <vosk/vosk_api.h>

using namespace godot;

void VoskVoiceRecognition::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initVosk", "model_path"), &VoskVoiceRecognition::initVosk);
    ClassDB::bind_method(D_METHOD("setLogLevel", "log_level"), &VoskVoiceRecognition::setLogLevel);
    ClassDB::bind_method(D_METHOD("setWordsJson", "words_json"), &VoskVoiceRecognition::setWordsJson);

    ADD_SIGNAL(MethodInfo("vosk_ready_signal", PropertyInfo(Variant::BOOL, "ready"), PropertyInfo(Variant::STRING, "error_message")));
    ADD_SIGNAL(MethodInfo("vosk_model_loaded_signal", PropertyInfo(Variant::STRING, "model_path")));
    ADD_SIGNAL(MethodInfo("vosk_recognizer_ready_signal", PropertyInfo(Variant::BOOL, "ready"), PropertyInfo(Variant::STRING, "error_message")));
}

VoskVoiceRecognition::VoskVoiceRecognition() {
    if (Engine::get_singleton()->is_editor_hint()) {
        set_process_mode(Node::ProcessMode::PROCESS_MODE_DISABLED);
    }

    

    // set default log level
    vosk_set_log_level(vosk_log_level);
}

VoskVoiceRecognition::~VoskVoiceRecognition() {}

void VoskVoiceRecognition::initVosk(String p_model_path = "") {
    model = vosk_model_new(p_model_path.utf8().get_data());
    if (model == nullptr)
    {
        emit_signal("vosk_ready_signal", false, "Model" + p_model_path + " not found.");
        return;
    }

    emit_signal("vosk_model_loaded_signal", p_model_path);

    recognizer = vosk_recognizer_new(model, sample_rate);
    if (recognizer == nullptr)
    {
        emit_signal("vosk_ready_signal", false, "Model " + p_model_path + " not loaded.");
        emit_signal("vosk_recognizer_ready_signal", false, "Failed to initialize voice recognizer.");
        return;
    }
    emit_signal("vosk_ready_signal", true, "Model " + p_model_path + " loaded.");
    emit_signal("vosk_recognizer_ready_signal", true, "Voice recognizer is ready to use.");
    return;
}

void VoskVoiceRecognition::_process(double delta) {}

void VoskVoiceRecognition::setLogLevel(int p_log_level) {
    vosk_log_level = p_log_level;
    vosk_set_log_level(p_log_level);
}

void VoskVoiceRecognition::setWordsJson(String p_words_json = "[]")
{
    if (recognizer == nullptr)
    {
        return;
    }

    vosk_recognizer_set_grm(recognizer, p_words_json.utf8().get_data());
    return;
}

int VoskVoiceRecognition::acceptWaveform(PackedByteArray p_raw_data, int p_length) {
    if (recognizer == nullptr) {
        return -2;
    }

    const char *audioData = reinterpret_cast<const char *>(p_raw_data.ptr());


    int result = vosk_recognizer_accept_waveform(recognizer, audioData, p_length);
    
    return result;
}