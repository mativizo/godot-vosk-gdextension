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
#include <mutex>


using namespace godot;

int audioCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                  const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags,
                  void *userData)
{
    VoskVoiceRecognition *vvr = static_cast<VoskVoiceRecognition *>(userData);

    vvr->status = -2;

    if (vvr->is_vosk_listening)
    {
        // Convert input buffer to the correct format
        const float *audioData = static_cast<const float *>(inputBuffer);
        
        if (vvr->do_accumulate_audio_data) {
            vvr->accumulate_audio_data(audioData, framesPerBuffer);
        }

        vvr->status = vosk_recognizer_accept_waveform_s(vvr->recognizer, reinterpret_cast<const int16_t *>(audioData), framesPerBuffer * 2);

    } else {
        vvr->p("ERROR", "Vosk Voice not listening", "audioCallback()");
    }

    return paContinue;
}

void VoskVoiceRecognition::_bind_methods()
{

    // main methods
    ClassDB::bind_method(D_METHOD("initialize", "model_path"), &VoskVoiceRecognition::initialize);
    ClassDB::bind_method(D_METHOD("switch_model_and_init", "model_path"), &VoskVoiceRecognition::switch_model_and_init);
    ClassDB::bind_method(D_METHOD("start"), &VoskVoiceRecognition::start);
    ClassDB::bind_method(D_METHOD("stop"), &VoskVoiceRecognition::stop);
    ClassDB::bind_method(D_METHOD("get_partial_result"), &VoskVoiceRecognition::get_partial_result);
    ClassDB::bind_method(D_METHOD("get_final_result"), &VoskVoiceRecognition::get_final_result);

    // config
    ClassDB::bind_method(D_METHOD("set_accumulate_audio_data", "value"), &VoskVoiceRecognition::set_accumulate_audio_data);
    ClassDB::bind_method(D_METHOD("set_logs", "logs"), &VoskVoiceRecognition::set_logs);
    ClassDB::bind_method(D_METHOD("set_partial_words", "value"), &VoskVoiceRecognition::set_partial_words);
    ClassDB::bind_method(D_METHOD("set_words", "value"), &VoskVoiceRecognition::set_words);
    ClassDB::bind_method(D_METHOD("set_size_in_ms", "miliseconds"), &VoskVoiceRecognition::set_size_in_ms);

    // input devices
    ClassDB::bind_method(D_METHOD("get_input_device_info_by_id", "input_device_id"), &VoskVoiceRecognition::get_input_device_info_by_id);
    ClassDB::bind_method(D_METHOD("get_input_devices"), &VoskVoiceRecognition::get_input_devices);
    ClassDB::bind_method(D_METHOD("set_input_device", "device_index"), &VoskVoiceRecognition::set_input_device);
    ClassDB::bind_method(D_METHOD("get_current_input_device_info"), &VoskVoiceRecognition::get_current_input_device_info);

    // cleaning up
    ClassDB::bind_method(D_METHOD("cleanup"), &VoskVoiceRecognition::cleanup);

    // utils
    ClassDB::bind_method(D_METHOD("p", "type", "message", "source"), &VoskVoiceRecognition::p);
    ClassDB::bind_method(D_METHOD("get_status"), &VoskVoiceRecognition::get_status);
    ClassDB::bind_method(D_METHOD("is_listening"), &VoskVoiceRecognition::is_listening);
    ClassDB::bind_method(D_METHOD("save_accumulated_audio"), &VoskVoiceRecognition::save_accumulated_audio);

    // signals
    ADD_SIGNAL(MethodInfo("final_result_signal", PropertyInfo(Variant::STRING, "final_result")));
    ADD_SIGNAL(MethodInfo("partial_result_signal", PropertyInfo(Variant::STRING, "partial_result")));
}

VoskVoiceRecognition::VoskVoiceRecognition()
{
    if (Engine::get_singleton()->is_editor_hint())
    {
        set_process_mode(Node::ProcessMode::PROCESS_MODE_DISABLED);
    }
    initialize_portaudio();
}


VoskVoiceRecognition::~VoskVoiceRecognition()
{
    cleanup();
    p("Info", "Class cleaned up and destroyed.", "~VoskVoiceRecognition() - destructor.");
}

void VoskVoiceRecognition::initialize_portaudio()
{
    PaError err = Pa_Initialize();
    if (err != paNoError)
    {
        p("PortAudio Error", "Can't initialize.", "initialize()");
    }
    else
    {
        // if not specified, get default
        if (selected_input_device_id == -1)
        {
            selected_input_device_id = Pa_GetDefaultInputDevice();
        }
        p("PortAudio Info", "Portaudio initialized!", "initialize()");
    }
}

bool VoskVoiceRecognition::initialize(godot::String p_model_path)
{
    is_vosk_listening = false;

    p("Vosk Info", "Initializing....", "initialize()");

    // Initialize PortAudio
    initialize_portaudio();

    // Initialize Vosk recognizer
    model = vosk_model_new(p_model_path.utf8().get_data());

    if (!model)
    {
        p("Vosk Info", "Failed to load model", "initialize()");
        return false;
    }
    else
    {
        p("Vosk Info", "Model loaded", "initialize()");
    }

    recognizer = vosk_recognizer_new(model, SAMPLE_RATE);
    if (!recognizer)
    {
        p("Vosk Info", "Failed to create recognizer.", "initialize()");
        return false;
    }
    else
    {
        p("Vosk Info", "Recognizer created.", "initialize()");
    }

    initialized = true;
    return true;
}

bool VoskVoiceRecognition::switch_model_and_init(godot::String model_path)
{
    p("Vosk Info", "Switching model...", "switch_model()");
    
    cleanup();

    return initialize(model_path);
}

void VoskVoiceRecognition::cleanup()
{
    if (recognizer)
    {
        vosk_recognizer_free(recognizer);
        p("Vosk Info", "Recognizer cleaned up", "cleanup()");
        recognizer = nullptr;
    }

    if (model)
    {
        vosk_model_free(model);
        p("Vosk Info", "Model cleaned up", "cleanup()");
        model = nullptr;
    }

    Pa_Terminate();
    p("PortAudio Info", "PortAudio cleaned up", "cleanup()");
}

bool VoskVoiceRecognition::start()
{
    p("Info", "Starting....", "start()");

    // Initialize PortAudio and Vosk
    if (!initialized)
    {
        p("Vosk Error", "VoskVoiceRecognition not initialized, can't start listening", "start()");
        return false;
    }
    else
    {
        p("Vosk Info", "Initialization test: passed.", "start()");
    }

    // Start the recognition process in a separate thread
    if (!is_vosk_listening)
    {
        p("Vosk Info", "Starting recognition", "start()");
        is_vosk_listening = true;
        // Open audio stream
        PaStreamParameters *inputParameters = new PaStreamParameters();
        inputParameters->device = selected_input_device_id;
        inputParameters->channelCount = NUM_CHANNELS;
        inputParameters->sampleFormat = paInt16;
        inputParameters->suggestedLatency = Pa_GetDeviceInfo(inputParameters->device)->defaultLowInputLatency;
        inputParameters->hostApiSpecificStreamInfo = NULL;

        std::string message = "Opening stream on " + std::to_string(selected_input_device_id);
        p("PortAudio Info", message.c_str(), "audioCallback()");


        
        // PaError err = Pa_OpenDefaultStream(&stream, NUM_CHANNELS, 0, paInt16, SAMPLE_RATE, (SAMPLE_RATE * buffer_ms)/1000, audioCallback, this);
        PaError err = Pa_OpenStream(&stream, inputParameters, NULL, SAMPLE_RATE, (SAMPLE_RATE * buffer_ms) / 1000, paClipOff, audioCallback, this);
        if (err != paNoError)
        {
            std::string message = "Can't open stream, error: " + (std::string) Pa_GetErrorText(err);
            p("PortAudio Error", message.c_str(), "recognize_in_thread()");
            cleanup();
            return false;
        }

        // Start audio stream
        err = Pa_StartStream(stream);
        if (err != paNoError)
        {
            std::string message = "Can't start stream, error: " + (std::string) Pa_GetErrorText(err);
            p("PortAudio Error", message.c_str(), "recognize_in_thread()");
            Pa_CloseStream(stream);
            cleanup();
            return false;
        }
        recognition_thread = std::thread(&VoskVoiceRecognition::recognize_in_thread, this); // Start recognition in a separate thread
        recognition_thread.detach();                                                        // Detach the thread
        p("Vosk Info", "Moved to thread, detached.", "start()");
        return true;
    }

    p("Vosk Info", "Already listening", "start()");
    return false;
}

void VoskVoiceRecognition::recognize_in_thread()
{
    // Loop until is_listening becomes false
    while (is_vosk_listening)
    {
        // Sleep to avoid busy-waiting
        Sleep(buffer_ms);
    }
}


void VoskVoiceRecognition::stop()
{
    p("Vosk Info", "Stopping recognition.", "stop()");
    is_vosk_listening = false;

    // Stop and close audio stream
    PaError err = Pa_StopStream(stream);
    if (err != paNoError)
    {
        std::string message = "Can't stop stream, error: " + (std::string) Pa_GetErrorText(err);
        p("PortAudio Error", message.c_str(), "recognize_in_thread()");
    }
    
    err = Pa_CloseStream(stream);
    if (err != paNoError)
    {
        p("PortAudio Error", "Can't close stream", "recognize_in_thread()");
    }
}

void VoskVoiceRecognition::set_size_in_ms(int miliseconds)
{
    std::string message = "Buffer size: " + std::to_string(miliseconds) + " ms";
    p("PortAudio Info", message.c_str(), "set_size_in_ms()");
    buffer_ms = miliseconds;
}

void VoskVoiceRecognition::accumulate_audio_data(const float *data, unsigned long num_samples)
{
    // accumulated_audio_data.insert(accumulated_audio_data.end(), data, data + num_samples);

    size_t newSize = accumulated_audio_data.size() + num_samples;
    accumulated_audio_data.reserve(newSize);
    accumulated_audio_data.insert(accumulated_audio_data.end(), data, data + num_samples);

}


bool VoskVoiceRecognition::save_accumulated_audio()
{
    const std::string filename = "z_test.wav";
    if (accumulated_audio_data.empty())
    {
        p("PortAudio Error", "No audio data accumulated to save.", "save_accumulated_audio()");
        return false;
    }

    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open())
    {
        p("PortAudio Error", "Failed to open output file for writing.", "save_accumulated_audio()");
        return false;
    }

    const int num_channels = NUM_CHANNELS;
    const int bits_per_sample = 16;
    const int data_size = accumulated_audio_data.size() * sizeof(short);

    // Write WAV file header
    file << "RIFF----WAVEfmt ";                                            // Chunk ID and format
    write_word(file, 16, 4);                                               // Subchunk1Size
    write_word(file, 1, 2);                                                // AudioFormat
    write_word(file, num_channels, 2);                                     // NumChannels
    write_word(file, SAMPLE_RATE, 4);                                      // SampleRate
    write_word(file, SAMPLE_RATE * num_channels * bits_per_sample / 8, 4); // ByteRate
    write_word(file, num_channels * bits_per_sample / 8, 2);               // BlockAlign
    write_word(file, bits_per_sample, 2);                                  // BitsPerSample

    file << "data----";             // Data chunk ID placeholder
    write_word(file, data_size, 4); // Data chunk size

    // Write audio data
    file.write(reinterpret_cast<const char *>(accumulated_audio_data.data()), data_size);

    // Update the data chunk size
    file.seekp(40, std::ios::beg);
    write_word(file, data_size, 4);

    // Close the file
    file.close();

    // Clear accumulated audio data
    accumulated_audio_data.clear();

    // create string "Saved audio data to: " + filename
    std::string message = "Saved audio data to: " + filename;
    p("[PortAudio Info]", message.c_str(), "save_accumulated_audio()");

    return true;
}

void VoskVoiceRecognition::write_word(std::ofstream &out, uint32_t word, int num_bytes)
{
    for (int i = 0; i < num_bytes; ++i)
    {
        out.put(word & 0xFF);
        word >>= 8;
    }
}

godot::Array VoskVoiceRecognition::get_input_devices()
{
    int num_devices = Pa_GetDeviceCount();
    if (num_devices < 0)
    {
        p("PortAudio Error", "Could not get device count.", "get_input_devices()");
        return {};
    }

    godot::Array device_list;
    for (int i = 0; i < num_devices; ++i)
    {
        const PaDeviceInfo *device_info = Pa_GetDeviceInfo(i);
        // && device_info->hostApi == 1
        if (device_info && device_info->maxInputChannels > 0)
        {
            godot::Dictionary di = get_input_device_info_by_id(i);
            device_list.push_back(di);
        }
    }

    return device_list;
}

bool VoskVoiceRecognition::set_input_device(int device_index)
{
    int num_devices = Pa_GetDeviceCount();
    if (num_devices < 0)
    {
        p("PortAudio Error", "Could not get device count.", "set_input_device()");
        return false;
    }

    if (device_index < 0 || device_index >= num_devices)
    {
        p("PortAudio Error", "Error: Invalid device index.", "set_input_device()");
        return false;
    }

    const PaDeviceInfo *device_info = Pa_GetDeviceInfo(device_index);
    if (!device_info || device_info->maxInputChannels == 0)
    {
        p("PortAudio Error", "Selected device is not an input device.", "set_input_device()");
        return false;
    }
    else
    {
        std::string message = "Selected input device: " + (std::string)device_info->name;
        p("PortAudio Info", message.c_str(), "set_input_device()");
    }

    selected_input_device_id = device_index;
    std::string message = "Device set to " + std::to_string(device_index) + " " + device_info->name;
    p("PortAudio Info", message.c_str(), "set_input_device()");
    return true;
}

int VoskVoiceRecognition::get_status()
{
    return status;
}

bool VoskVoiceRecognition::is_listening()
{
    return is_vosk_listening;
}

void VoskVoiceRecognition::set_partial_words(bool value)
{
    if (recognizer == nullptr)
    {
        p("Vosk Error", "Recognizer is not initialized.", "set_partial_words()");
        return;
    }
    vosk_recognizer_set_partial_words(recognizer, value);
    if (value)
    {
        p("Vosk Settings", "Partial words: true", "set_partial_words()");
    }
    else
    {
        p("Vosk Settings", "Partial words: false", "set_partial_words()");
    }
}

void VoskVoiceRecognition::set_words(bool value)
{
    if (recognizer == nullptr)
    {
        p("Vosk Error", "Recognizer is not initialized.", "set_words()");
        return;
    }
    vosk_recognizer_set_words(recognizer, value);
    if (value)
    {
        p("Vosk Settings", "Words: true", "set_words()");
    }
    else
    {
        p("Vosk Settings", "Words: false", "set_words()");
    }
}

godot::Dictionary VoskVoiceRecognition::get_input_device_info_by_id(int device_index)
{
    godot::Dictionary device_info;

    const PaDeviceInfo *device_info_pa = Pa_GetDeviceInfo(device_index);
    if (!device_info_pa)
    {
        p("PortAudio Error", "Failed to get device info.", "get_input_device_info_by_id()");
        return device_info;
    }

    device_info["name"] = device_info_pa->name;
    device_info["max_input_channels"] = device_info_pa->maxInputChannels;
    device_info["max_output_channels"] = device_info_pa->maxOutputChannels;
    device_info["default_sample_rate"] = device_info_pa->defaultSampleRate;
    device_info["default_low_input_latency"] = device_info_pa->defaultLowInputLatency;
    device_info["default_low_output_latency"] = device_info_pa->defaultLowOutputLatency;
    device_info["default_high_input_latency"] = device_info_pa->defaultHighInputLatency;
    device_info["default_high_output_latency"] = device_info_pa->defaultHighOutputLatency;
    device_info["id"] = device_index;

    return device_info;
}

godot::Dictionary VoskVoiceRecognition::get_current_input_device_info()
{
    int defaultDeviceIndex = Pa_GetDefaultInputDevice();
    if (selected_input_device_id < 0) {
        defaultDeviceIndex = selected_input_device_id;
    }

    if (!defaultDeviceIndex)
    {
        p("PortAudio Error", "Failed to get default input device.", "get_current_input_device_info()");
    }
    return get_input_device_info_by_id(defaultDeviceIndex);
}

void VoskVoiceRecognition::set_logs(bool logs)
{
    logs_enabled = logs;
}

void VoskVoiceRecognition::p(godot::String type, godot::String message, godot::String source)
{
    // if log enabled
    if (!logs_enabled)
    {
        return;
    }

    UtilityFunctions::print("[GVGEX]: [", type, "] ", message, " (", source, ")");
}

void VoskVoiceRecognition::set_accumulate_audio_data(bool enable)
{
    do_accumulate_audio_data = enable;
}


godot::Dictionary VoskVoiceRecognition::get_partial_result()
{
    godot::Dictionary dict = {};
    if (is_vosk_listening) {
        std::string res = (std::string) vosk_recognizer_partial_result(recognizer);
        dict = convert_to_dict(res);
        return dict;
        // dict["test"] = res.c_str();
        // return dict;
    }

    return dict;
}

godot::Dictionary VoskVoiceRecognition::get_final_result()
{
    godot::Dictionary dict = {};
    if (is_vosk_listening) {
        std::string res = (std::string) vosk_recognizer_result(recognizer);
        godot::Dictionary dict = convert_to_dict(res);
        return dict;
    }
    return dict;
}

godot::Dictionary VoskVoiceRecognition::convert_to_dict(std::string output) {
    godot::Dictionary result;

    std::string cleaned_output = output;

    // Remove commas after lines
    size_t pos = 0;
    while ((pos = cleaned_output.find(",\n", pos)) != std::string::npos) {
        cleaned_output.replace(pos, 2, "\n");
    }

    size_t start_pos = 0;
    while (true) {
        // Find the next key-value pair
        size_t key_pos = cleaned_output.find('"', start_pos);
        if (key_pos == std::string::npos) break; // No more key-value pairs
        
        size_t end_key_pos = cleaned_output.find('"', key_pos + 1);
        if (end_key_pos == std::string::npos) break; // Invalid format

        godot::String key = cleaned_output.substr(key_pos + 1, end_key_pos - key_pos - 1).c_str();
        
        // Find the value
        size_t value_pos = cleaned_output.find(':', end_key_pos);
        if (value_pos == std::string::npos) break; // Invalid format
        
        char first_char = cleaned_output[value_pos + 1];
        std::string value_str;
        size_t value_end_pos;
        if (first_char == '"' || first_char == '\'') {
            // String value
            size_t end_quote_pos = cleaned_output.find(first_char, value_pos + 2);
            if (end_quote_pos == std::string::npos) break; // Invalid format
            value_str = cleaned_output.substr(value_pos + 2, end_quote_pos - value_pos - 2);
            value_end_pos = end_quote_pos;
        } else {
            // Numeric or boolean value
            size_t end_value_pos = cleaned_output.find_first_not_of("0123456789.-+eE", value_pos + 1);
            value_str = cleaned_output.substr(value_pos + 1, end_value_pos - value_pos - 1);
            value_end_pos = end_value_pos;
        }

        // Convert the value to Godot Variant type
        // print to console out
        

        godot::String value = (godot::String) value_str.c_str();
        result[key] = value;

        // Move start_pos to the next key-value pair
        start_pos = cleaned_output.find(',', value_end_pos);
        if (start_pos == std::string::npos) break;
        start_pos++;
    }

    return result;
}