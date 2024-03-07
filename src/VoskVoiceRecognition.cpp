#include "VoskVoiceRecognition.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <vosk/vosk_api.h>
#include <algorithm>


using namespace godot;

void VoskVoiceRecognition::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initVosk", "model_path"), &VoskVoiceRecognition::initVosk);
    ClassDB::bind_method(D_METHOD("setLogLevel", "log_level"), &VoskVoiceRecognition::setLogLevel);
    ClassDB::bind_method(D_METHOD("setWordsJson", "words_json"), &VoskVoiceRecognition::setWordsJson);
    ClassDB::bind_method(D_METHOD("acceptWaveform", "raw_data", "current_sample_rate"), &VoskVoiceRecognition::acceptWaveform);
    ClassDB::bind_method(D_METHOD("getResults"), &VoskVoiceRecognition::getResults);
    ClassDB::bind_method(D_METHOD("getPartialResults"), &VoskVoiceRecognition::getPartialResults);

    ADD_SIGNAL(MethodInfo("vosk_ready_signal", PropertyInfo(Variant::BOOL, "ready"), PropertyInfo(Variant::STRING, "error_message")));
    ADD_SIGNAL(MethodInfo("vosk_model_loaded_signal", PropertyInfo(Variant::STRING, "model_path")));
    ADD_SIGNAL(MethodInfo("vosk_recognizer_ready_signal", PropertyInfo(Variant::BOOL, "ready"), PropertyInfo(Variant::STRING, "error_message")));
}

VoskVoiceRecognition::VoskVoiceRecognition() {
    if (Engine::get_singleton()->is_editor_hint()) {
        set_process_mode(Node::ProcessMode::PROCESS_MODE_DISABLED);
    }

    vosk_set_log_level(vosk_log_level);
}

VoskVoiceRecognition::~VoskVoiceRecognition() {
    
}

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

int VoskVoiceRecognition::acceptWaveform(PackedVector2Array p_raw_data, float current_sample_rate = 16000.0f) {
    if (recognizer == nullptr) {
        return -2;
    }

    if (_isStereo(p_raw_data)) {
        if (vosk_log_level > 0) {
            UtilityFunctions::print("Input is stereo, converting.");
        }
        _convertToMono(p_raw_data);
    } else {
        if (vosk_log_level > 0) {
            UtilityFunctions::print("Input is mono.");
        }
    }

    if (vosk_log_level > 0) {
        UtilityFunctions::print("Current input sample rate: " + String::num(current_sample_rate, 2) + " Hz"); 
    }

    // Resample the audio data to 16kHz if needed
    if (current_sample_rate != 16000.0f) {
        if (vosk_log_level > 0) {
            UtilityFunctions::print("Resampling to 16kHz...");
        }
        _resampleTo16k(p_raw_data, current_sample_rate);
    }

    int length = p_raw_data.size();
    int16_t *buffer = new int16_t[length]; // Dynamically allocate memory for the buffer

    for (int i = 0; i < length; i++) {
        // Convert from floating point to 16-bit PCM
        buffer[i] = std::clamp( (short) (p_raw_data[i].x * 32768), (short) -32768, (short) 32767);
        //buffer[i] = static_cast<int16_t>(p_raw_data[i].x * 32767);
    }

    int result = vosk_recognizer_accept_waveform_s(recognizer, buffer, length);
    delete[] buffer;
    return result;
}

String VoskVoiceRecognition::getResults() {
    if (recognizer == nullptr) {
        return "";
    }

    return String(vosk_recognizer_result(recognizer));
}

String VoskVoiceRecognition::getPartialResults() {
    if (recognizer == nullptr) {
        return "";
    }

    return String(vosk_recognizer_partial_result(recognizer));
}


void VoskVoiceRecognition::_convertToMono(PackedVector2Array &p_raw_data) {
    for (int i = 0; i < p_raw_data.size(); i++) {
        p_raw_data[i].x = (p_raw_data[i].x + p_raw_data[i].y) / 2.0;
        p_raw_data[i].y = 0.0; // Set the second channel to 0
    }
}

// Resample to 16kHz
void VoskVoiceRecognition::_resampleTo16k(PackedVector2Array &p_raw_data, float current_sample_rate) {
    float target_sample_rate = 16000.0f;
    float ratio = target_sample_rate / current_sample_rate;

    for (int i = 0; i < p_raw_data.size(); i++) {
        p_raw_data[i].x *= ratio;
    }
}

bool VoskVoiceRecognition::_isStereo(const PackedVector2Array &p_raw_data) {
    if (p_raw_data.size() == 0) {
        return false;
    }

    for (int i = 0; i < p_raw_data.size(); i++) {
        if (p_raw_data[i].y != 0.0) {
            return true; // Found a non-zero y value, indicating stereo
        }
    }

    return false;
}