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
		var voice_sample = record_effect.get_recording()
		
		if not voice_sample: return
		
		var data : PackedByteArray = []
		
		
		
		
		
		
		
		
