#include "test.h"

using namespace godot;

void VoskVoiceRecognition::_bind_methods() {
    // ClassDB::bind_method(D_METHOD("initVosk", "model_path"), &VoskVoiceRecognition::initVosk);
    // ClassDB::bind_method(D_METHOD("p", "text"), &VoskVoiceRecognition::p);
    // ClassDB::bind_method(D_METHOD("cleanup"), &VoskVoiceRecognition::cleanup);
    // ClassDB::bind_method(D_METHOD("start", "model_path"), &VoskVoiceRecognition::start);
    // ClassDB::bind_method(D_METHOD("initialize", "model_path"), &VoskVoiceRecognition::initialize);
    

    // ADD_SIGNAL(MethodInfo("vosk_ready_signal", PropertyInfo(Variant::BOOL, "ready"), PropertyInfo(Variant::STRING, "error_message")));
    // ADD_SIGNAL(MethodInfo("vosk_model_loaded_signal", PropertyInfo(Variant::STRING, "model_path")));
    // ADD_SIGNAL(MethodInfo("vosk_recognizer_ready_signal", PropertyInfo(Variant::BOOL, "ready"), PropertyInfo(Variant::STRING, "error_message")));
}

VoskVoiceRecognition::VoskVoiceRecognition() {}


VoskVoiceRecognition::~VoskVoiceRecognition() {}
    
int audioCallback(const void *inputBuffer, void *outputBuffer,
                  unsigned long framesPerBuffer,
                  const PaStreamCallbackTimeInfo *timeInfo,
                  PaStreamCallbackFlags statusFlags,
                  void *userData) {
    const float *audioData = static_cast<const float*>(inputBuffer);

    VoskVoiceRecognition* vvr = static_cast<VoskVoiceRecognition*>(userData);

    if (vvr->is_listening) {
        // Process audio using Vosk
        vosk_recognizer_accept_waveform(vvr->recognizer, reinterpret_cast<const char*>(audioData), framesPerBuffer);

        // Retrieve partial result
        const char *partialResult = vosk_recognizer_partial_result(vvr->recognizer);
        if (partialResult) {
            vvr->p(partialResult);
        }
        
        // Retrieve final result
        const char *finalResult = vosk_recognizer_result(vvr->recognizer);
        if (finalResult) {
            vvr->p(finalResult);
        }
    }

    return paContinue;
}

// Method to initialize PortAudio and Vosk
bool VoskVoiceRecognition::initialize(const char p_model_path[]) {
    // Initialize PortAudio
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        p("PortAudio error: can't initialize");
        return false;
    }

    // Initialize Vosk recognizer
    model = vosk_model_new(p_model_path);

    if (!model) {
        p("Failed to load model");
        return false;
    }

    recognizer = vosk_recognizer_new(model, SAMPLE_RATE);
    if (!recognizer) {
        p("Failed to create recognizer");
        return false;
    }

    return true;
}

// Method to cleanup PortAudio and Vosk
void VoskVoiceRecognition::cleanup() {
    if (recognizer) {
        vosk_recognizer_free(recognizer);
        p("Recognizer cleaned up");
    }

    if (model) {
        vosk_model_free(model);
        p("Model cleaned up");
    }


    Pa_Terminate();
    p("PortAudio cleaned up");
}

bool VoskVoiceRecognition::start(const char p_model_path[]) {
    // Initialize PortAudio and Vosk
    if (!initialize(p_model_path)) {
        return false;
    }

    // Open audio stream
    PaStream *stream;
    PaError err = Pa_OpenDefaultStream(&stream, NUM_CHANNELS, 0, SAMPLE_FORMAT, SAMPLE_RATE,
                                        FRAMES_PER_BUFFER, audioCallback, this);
    if (err != paNoError) {
        p("PortAudio error: can't open stream");
        cleanup();
        return false;
    }

    // Start audio stream
    err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        Pa_CloseStream(stream);
        cleanup();
        return false;
    }

    is_listening = true;
    p("Listening for speech...");

    // Wait for is_listening to become false
    while (is_listening) {
        // std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Adjust the sleep duration as needed
        Sleep(100);
    }

    // Stop and close audio stream
    err = Pa_StopStream(stream);
    if (err != paNoError) {
        p("PortAudio error: can't stop stream");
    }
    err = Pa_CloseStream(stream);
    if (err != paNoError) {
        p("PortAudio error: can't close stream");
    }

    // Cleanup PortAudio and Vosk
    cleanup();

    return true;
}

void VoskVoiceRecognition::p(const char p_text[]) {
    std::cout << p_text << std::endl;

}


// main method
int main() {
    VoskVoiceRecognition voskVoiceRecognition;
    const char path[] = "../demo/bin/models/vosk-model-small-en-us-0.15";
    voskVoiceRecognition.start(path);
    return 0;
}