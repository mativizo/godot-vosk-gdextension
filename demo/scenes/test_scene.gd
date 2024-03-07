extends Control

# vosk voice recognition node
@onready var vosk_voice_recognition : VoskVoiceRecognition = $VoskVoiceRecognition

# ui elements
@onready var loaded_model_label : Label = $MainMarginContainer/HBoxColumns/VBoxLeftColumn/LoadedModel
@onready var listening_button : Button = $MainMarginContainer/HBoxColumns/VBoxLeftColumn/StartListening
@onready var status_label : Label = $MainMarginContainer/HBoxColumns/VBoxLeftColumn/Status

@onready var partial_results_label : Label = $MainMarginContainer/HBoxColumns/VBoxRightColumn/PartialText
@onready var results_label : Label = $MainMarginContainer/HBoxColumns/VBoxRightColumn/FullText

var capture_bus_name : String = "Record"
var is_listening : bool = false
var is_ready : bool = false
var capture_effect : AudioEffectCapture

func _ready():
	listening_button.disabled = true
	listening_button.pressed.connect(_on_listening_button)
	
	var input_devices = AudioServer.get_input_device_list()
	var current_input_device = AudioServer.input_device
	print(input_devices)
	print("Current: "+str(current_input_device))
	AudioServer.set_input_device(input_devices[input_devices.size() - 1])
	print(AudioServer.input_device)
	
	# get record bus
	var capture_idx = AudioServer.get_bus_index("Capture")
	
	print("Capture idx %s" % [capture_idx])
	
	# get effect from bus
	capture_effect = AudioServer.get_bus_effect(capture_idx, 0)
	
	
	vosk_voice_recognition.vosk_ready_signal.connect(_on_vosk_ready)
	vosk_voice_recognition.vosk_model_loaded_signal.connect(_on_vosk_model_loaded)
	vosk_voice_recognition.vosk_recognizer_ready_signal.connect(_on_vosk_recognizer_ready)
	
	vosk_voice_recognition.setLogLevel(2)
	vosk_voice_recognition.initVosk("bin/models/vosk-model-small-en-us-0.15")
	
func _on_vosk_ready(_is_ready : bool, error_message : String):
	if not _is_ready:
		is_ready = false
		print("Vosk not ready. Message: " + str(error_message))
		status_label.text = "Vosk not ready, "+str(error_message)
		listening_button.disabled = true
		return
		
	print("Vosk is ready. Message: " + str(error_message))
	status_label.text = "Vosk is ready, "+str(error_message)
	is_ready = true
	listening_button.disabled = false
	
	
func _on_vosk_model_loaded(model_path : String):
	loaded_model_label.text = "Loaded model: "+str(model_path)

func _on_vosk_recognizer_ready(is_ready : bool, error_message : String):
	if not is_ready:
		print("Reconigzer isn't ready. Message: " + str(error_message))
		return
		
	print("Voice recognizer is ready! Message: "+str(error_message))
	
	# Start recognizing process
		
func _microphone_processing():
	if not capture_effect:
		print("Capture effect not ready, returning ")
		return
	
	var buffer_length_frames = capture_effect.get_buffer_length_frames()
	print("Buffer length: "+str(buffer_length_frames))
	if not capture_effect.can_get_buffer(buffer_length_frames / 2): return
	
	var data : PackedVector2Array = capture_effect.get_buffer(capture_effect.get_frames_available())
	var result = vosk_voice_recognition.acceptWaveform(data, 96000)
	
	if result == 1:
		var results = JSON.parse_string(vosk_voice_recognition.getResults())
		if results and results.has("text") and results.text != "":
			results_label.text = results.text
	elif result == 0:
		print("decoding continues")
		var partial_results = JSON.parse_string(vosk_voice_recognition.getPartialResults())
		if partial_results and partial_results.has("text") and partial_results.text != "":
			partial_results_label.text = partial_results.text
	elif result == -1: print("exception occured")
	elif result == -2: print("you must call initVosk first")
		

func _on_listening_button():
	if is_listening:
		is_listening = false
		status_label.text = "Listening stopped"
	else:
		is_listening = true
		status_label.text = "Listening..."

var every_x_frame = 0
var frame = 0
func _process(_delta : float):
	frame += 1
	if is_ready and is_listening:
		if every_x_frame <= frame:
			_microphone_processing()
			frame = 0
