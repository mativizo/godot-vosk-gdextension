#include "VoskVoiceRecognition.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <portaudio.h> // Include PortAudio header
#include <algorithm>
#include <windows.h>
#include <streambuf>
#include <string>
#include <fstream>
#include <thread>

using namespace godot;

int audioCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                  const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags,
                  void *userData) {
    VoskVoiceRecognition* vvr = static_cast<VoskVoiceRecognition*>(userData);

    if (vvr->is_vosk_listening) {
        // Convert input buffer to the correct format
        const float *audioData = static_cast<const float*>(inputBuffer);
        const short *audioData_s = static_cast<const short*>(inputBuffer);

        
        short *convertedData = new short[framesPerBuffer];
        int16_t *convertedData_s = new int16_t[framesPerBuffer];
        short max_data = 0;
        for (unsigned long i = 0; i < framesPerBuffer; ++i) {
            convertedData[i] = static_cast<short>(audioData[i] * 32767.0f);
            convertedData_s[i] = static_cast<int16_t>(audioData[i] * 32767.0f);

            if (std::abs(convertedData[i]) > max_data) {
                max_data = std::abs(convertedData[i]);
            }
        }

        vvr->accumulate_audio_data(audioData, framesPerBuffer);

        // Process audio using Vosk
        vvr->status = vosk_recognizer_accept_waveform_s(vvr->recognizer, reinterpret_cast<const int16_t *>(audioData), framesPerBuffer * 2);
        

        delete[] convertedData;
    }

    return paContinue;
}

void VoskVoiceRecognition::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initialize", "model_path"), &VoskVoiceRecognition::initialize);
    ClassDB::bind_method(D_METHOD("start"), &VoskVoiceRecognition::start);
    ClassDB::bind_method(D_METHOD("stop"), &VoskVoiceRecognition::stop);
    ClassDB::bind_method(D_METHOD("set_size_in_ms", "miliseconds"), &VoskVoiceRecognition::set_size_in_ms);
    ClassDB::bind_method(D_METHOD("save_accumulated_audio"), &VoskVoiceRecognition::save_accumulated_audio);
    ClassDB::bind_method(D_METHOD("get_input_devices"), &VoskVoiceRecognition::get_input_devices);
    ClassDB::bind_method(D_METHOD("set_input_device", "device_index"), &VoskVoiceRecognition::set_input_device);
    ClassDB::bind_method(D_METHOD("get_partial"), &VoskVoiceRecognition::get_partial);
    ClassDB::bind_method(D_METHOD("get_final"), &VoskVoiceRecognition::get_final);
    ClassDB::bind_method(D_METHOD("get_status"), &VoskVoiceRecognition::get_status);
    ClassDB::bind_method(D_METHOD("is_listening"), &VoskVoiceRecognition::is_listening);
    ClassDB::bind_method(D_METHOD("set_partial_words", "value"), &VoskVoiceRecognition::set_partial_words);
    ClassDB::bind_method(D_METHOD("set_words", "value"), &VoskVoiceRecognition::set_words);

    //ADD_SIGNAL(MethodInfo("voice_status_signal", PropertyInfo(Variant::INT, "status"), PropertyInfo(Variant::STRING, "error_message")));
}

VoskVoiceRecognition::VoskVoiceRecognition() {
    if (Engine::get_singleton()->is_editor_hint()) {
        set_process_mode(Node::ProcessMode::PROCESS_MODE_DISABLED);
    }
}

VoskVoiceRecognition::~VoskVoiceRecognition() {}



bool VoskVoiceRecognition::initialize(godot::String p_model_path) {
    UtilityFunctions::print("Initializing....");
    
    // Initialize PortAudio
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        UtilityFunctions::print("PortAudio error: can't initialize");
        return false;
    } else {
        UtilityFunctions::print("Portaudio initialized!");
    }

    // Initialize Vosk recognizer
    model = vosk_model_new(p_model_path.utf8().get_data());

    if (!model) {
        UtilityFunctions::print("Failed to load model");
        return false;
    } else {
        UtilityFunctions::print("Model loaded");
    }

    recognizer = vosk_recognizer_new(model, SAMPLE_RATE);
    if (!recognizer) {
        godot::UtilityFunctions::print("Failed to create recognizer");
        return false;
    } else {
        UtilityFunctions::print("Recognizer created");
        
    }

    initialized = true;
    return true;
}

void VoskVoiceRecognition::cleanup() {
    if (recognizer) {
        vosk_recognizer_free(recognizer);
        godot::UtilityFunctions::print("Recognizer cleaned up");
    }

    if (model) {
        vosk_model_free(model);
        godot::UtilityFunctions::print("Model cleaned up");
    }

    Pa_Terminate();
    godot::UtilityFunctions::print("PortAudio cleaned up");
}

bool VoskVoiceRecognition::start() {
    // Initialize PortAudio and Vosk
    if (!initialized) {
        UtilityFunctions::print("VoskVoiceRecognition not initialized, can't start listening");
        return false;
    }

    // Start the recognition process in a separate thread
    if (!is_vosk_listening) {
        is_vosk_listening = true;
        recognition_thread = std::thread(&VoskVoiceRecognition::recognize_in_thread, this); // Start recognition in a separate thread
        recognition_thread.detach(); // Detach the thread
        return true;
    }

    return false; // Return false if already listening
}


void VoskVoiceRecognition::recognize_in_thread() {
    // Open audio stream
    PaStream *stream;
    PaError err = Pa_OpenDefaultStream(&stream, NUM_CHANNELS, 0, paInt16, SAMPLE_RATE, (SAMPLE_RATE * buffer_ms)/1000, audioCallback, this);
    if (err != paNoError) {
        godot::UtilityFunctions::print("PortAudio error: can't open stream");
        cleanup();
        return;
    }

    // Start audio stream
    err = Pa_StartStream(stream);
    if (err != paNoError) {
        godot::UtilityFunctions::print("PortAudio error: can't start stream");
        Pa_CloseStream(stream);
        cleanup();
        return;
    }

    UtilityFunctions::print("Listening for speech...");

    // Loop until is_listening becomes false
    while (is_vosk_listening) {
        // Sleep to avoid busy-waiting
        Sleep(buffer_ms);
    }

    // Stop and close audio stream
    err = Pa_StopStream(stream);
    if (err != paNoError) {
        UtilityFunctions::print("PortAudio error: can't stop stream");
    }
    err = Pa_CloseStream(stream);
    if (err != paNoError) {
        UtilityFunctions::print("PortAudio error: can't close stream");
    }

    // Cleanup PortAudio and Vosk
    cleanup();
}

void VoskVoiceRecognition::stop() {
    is_vosk_listening = false;
}

void VoskVoiceRecognition::set_size_in_ms(int miliseconds) {
    buffer_ms = miliseconds;
}

void VoskVoiceRecognition::accumulate_audio_data(const float* data, unsigned long num_samples) {
    //godot::UtilityFunctions::print("- - - - - - - ADDING DATA:");
    //godot::UtilityFunctions::print(data);
    //godot::UtilityFunctions::print("- - - - - - - END DATA:");
    accumulated_audio_data.insert(accumulated_audio_data.end(), data, data + num_samples);
}



bool VoskVoiceRecognition::save_accumulated_audio() {

    const std::string filename = "z_test.wav";

    if (accumulated_audio_data.empty()) {
        godot::UtilityFunctions::print("No audio data accumulated to save.");
        return false;
    }

    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        godot::UtilityFunctions::print("Failed to open output file for writing.");
        return false;
    }

    const int num_channels = NUM_CHANNELS;
    const int bits_per_sample = 16;
    const int data_size = accumulated_audio_data.size() * sizeof(short);

    // Write WAV file header
    file << "RIFF----WAVEfmt ";  // Chunk ID and format
    write_word(file, 16, 4);      // Subchunk1Size
    write_word(file, 1, 2);       // AudioFormat
    write_word(file, num_channels, 2);    // NumChannels
    write_word(file, SAMPLE_RATE, 4);     // SampleRate
    write_word(file, SAMPLE_RATE * num_channels * bits_per_sample / 8, 4);       // ByteRate
    write_word(file, num_channels * bits_per_sample / 8, 2);     // BlockAlign
    write_word(file, bits_per_sample, 2); // BitsPerSample

    file << "data----"; // Data chunk ID placeholder
    write_word(file, data_size, 4); // Data chunk size

    // Write audio data
    file.write(reinterpret_cast<const char*>(accumulated_audio_data.data()), data_size);

    // Update the data chunk size
    file.seekp(40, std::ios::beg);
    write_word(file, data_size, 4);

    // Close the file
    file.close();

    // Clear accumulated audio data
    accumulated_audio_data.clear();

    return true;
}

void VoskVoiceRecognition::write_word(std::ofstream& out, uint32_t word, int num_bytes) {
    for (int i = 0; i < num_bytes; ++i) {
        out.put(word & 0xFF);
        word >>= 8;
    }
}

godot::Array VoskVoiceRecognition::get_input_devices() {
    int num_devices = Pa_GetDeviceCount();
    if (num_devices < 0) {
        godot::UtilityFunctions::print("Error: Could not get device count.");
        return {};
    }

    godot::Array device_list;
    for (int i = 0; i < num_devices; ++i) {
        const PaDeviceInfo* device_info = Pa_GetDeviceInfo(i);
        if (device_info && device_info->maxInputChannels > 0) {
            device_list.push_back(device_info->name);
        }
    }

    return device_list;
}

bool VoskVoiceRecognition::set_input_device(int device_index) {
    int num_devices = Pa_GetDeviceCount();
    if (num_devices < 0) {
        godot::UtilityFunctions::print("Error: Could not get device count.");
        return false;
    }

    if (device_index < 0 || device_index >= num_devices) {
        godot::UtilityFunctions::print("Error: Invalid device index.");
        return false;
    }

    const PaDeviceInfo* device_info = Pa_GetDeviceInfo(device_index);
    if (!device_info || device_info->maxInputChannels == 0) {
        godot::UtilityFunctions::print("Error: Selected device is not an input device.");
        return false;
    }

    //input_device_index = device_index;
    return true;
}


godot::String VoskVoiceRecognition::get_partial() {
    // Retrieve partial result
    godot::String partialResult = vosk_recognizer_partial_result(recognizer);
    return partialResult;
}

godot::String VoskVoiceRecognition::get_final() {
    // Retrieve partial result
    godot::String finalResult = vosk_recognizer_final_result(recognizer);
    return finalResult;
}

int VoskVoiceRecognition::get_status() {
    return status;
}

bool VoskVoiceRecognition::is_listening() {
    return is_vosk_listening;
}

void VoskVoiceRecognition::set_partial_words(bool value) {
    if (recognizer == nullptr) {
        godot::UtilityFunctions::print("Recognizer is not initialized.");
        return;
    }
    vosk_recognizer_set_partial_words(recognizer, value);
    if (value) {
        godot::UtilityFunctions::print("Partial words: true");
    } else {
        godot::UtilityFunctions::print("Partial words: false");
    }
}


void VoskVoiceRecognition::set_words(bool value) {
    if (recognizer == nullptr) {
        godot::UtilityFunctions::print("Recognizer is not initialized.");
        return;
    }
    vosk_recognizer_set_words(recognizer, value);
    if (value) {
        godot::UtilityFunctions::print("Words: true");
    } else {
        godot::UtilityFunctions::print("Words: false");
    }
}
