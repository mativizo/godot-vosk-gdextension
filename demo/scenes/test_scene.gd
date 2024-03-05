extends Control

# vosk voice recognition node
@onready var vosk_voice_recognition : VoskVoiceRecognition = $VoskVoiceRecognition

# ui elements
@onready var loaded_model_label : Label = $MainMarginContainer/HBoxColumns/VBoxLeftColumn/LoadedModel

func _ready():
	vosk_voice_recognition.vosk_ready_signal.connect(_on_vosk_ready)
	vosk_voice_recognition.vosk_model_loaded_signal.connect(_on_vosk_model_loaded)
	vosk_voice_recongition.vosk_recognizer_ready_signal.connect(_on_vosk_recognizer_ready)
	
	vosk_voice_recognition.setLogLevel(2)
	vosk_voice_recognition.init("bin/models/vosk-model-small-en-us-0.15")
	
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
		
