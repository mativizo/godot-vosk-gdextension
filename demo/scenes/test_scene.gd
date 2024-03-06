extends Control

# vosk voice recognition node
@onready var vosk_voice_recognition : VoskVoiceRecognition = $VoskVoiceRecognition

# ui elements
@onready var loaded_model_label : Label = $MainMarginContainer/HBoxColumns/VBoxLeftColumn/LoadedModel
@onready var listening_button : Button = $MainMarginContainer/HBoxColumns/VBoxRightColumn/StartListening

var record_bus_name : String = "Record"
var is_listening : bool = false
var timeout : int = 12000
var record_effect : AudioEffectRecord




func _ready():
	# get record bus
	var record_idx = AudioServer.get_bus_index("Record")
	
	# get effect from bus
	record_effect = AudioServer.get_bus_effect(record_idx, 0)
	
	
	vosk_voice_recognition.vosk_ready_signal.connect(_on_vosk_ready)
	vosk_voice_recognition.vosk_model_loaded_signal.connect(_on_vosk_model_loaded)
	vosk_voice_recognition.vosk_recognizer_ready_signal.connect(_on_vosk_recognizer_ready)
	
	vosk_voice_recognition.setLogLevel(2)
	vosk_voice_recognition.initVosk("bin/models/vosk-model-small-en-us-0.15")
	
func _on_vosk_ready(is_ready : bool, error_message : String):
	if is_ready: print("Vosk is ready. Message: " + str(error_message))
	else: print("Vosk not ready. Message: " + str(error_message))
	
func _on_vosk_model_loaded(model_path : String):
	loaded_model_label.text = "Loaded model: "+str(model_path)

func _on_vosk_recognizer_ready(is_ready : bool, error_message : String):
	if not is_ready:
		print("Reconigzer isn't ready. Message: " + str(error_message))
		return
		
	print("Voice recognizer is ready! Message: "+str(error_message))
	
	# Start recognizing process
		
func _microphone_processing():
	if not record_effect: return
	
	# recording...
	if record_effect.is_recording_active():
		var voice_sample : AudioStreamWAV = record_effect.get_recording()
		
		if not voice_sample: return
		
		var data : Array = []
		if voice_sample.stereo: data = _stereo_to_mono(voice_sample.data)
		else: data = voice_sample.data
		
		var apply_results = vosk_voice_recognition.applyWaveform(data, data.size())
		if apply_results == 1: print("silence occured")
		elif apply_results == 0: print("decoding continues")
		elif apply_results == -1: print("exception occured")
		elif apply_results == -2: print("you must call initVosk first")
		

func _stereo_to_mono(input_data : PackedByteArray) -> PackedByteArray:
	# check if real stereo
	if input_data.size() % 4 != 0: return []
	var output_data : PackedByteArray = []
	var output_index : int = 0
	
	# mix channels
	for n in range(0, input_data.size(), 4):
		var left_channel : int = input_data[n] + (input_data[n + 1] << 8)
		var right_channel: int = input_data[n + 2] + (input_data[n + 3] << 8)
		var mixed_channels: int = (left_channel + right_channel) / 2
		output_data.append(mixed_channels)
	return output_data
			
		
		
		
		
		
