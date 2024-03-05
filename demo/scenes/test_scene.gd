extends Control

@onready var vosk_voice_recognition : VoskVoiceRecognition = $VoskVoiceRecognition
@onready var loaded_model_label : Label = $MainMarginContainer/HBoxColumns/VBoxLeftColumn/LoadedModel

func _ready():
	vosk_voice_recognition.vosk_ready_signal.connect(_on_vosk_ready)
	vosk_voice_recognition.vosk_model_loaded.connect(_on_vosk_model_loaded)
	
	vosk_voice_recognition.setLogLevel(2)
	vosk_voice_recognition.init("bin/models/vosk-model-small-en-us-0.15")
	
func _on_vosk_ready(is_ready : bool, message : String):
	if is_ready: print("Vosk is ready. Message: "+str(message))
	else: print("Vosk not ready. Message: " + str(message))
	
func _on_vosk_model_loaded(model_path : String):
	loaded_model_label.text = "Loaded model: "+str(model_path)
