extends Node2D

@onready var vosk_voice_recognition : VoskVoiceRecognition = $VoskVoiceRecognition

func _ready():
	vosk_voice_recognition.vosk_ready_signal.connect(_on_vosk_ready)
	
	vosk_voice_recognition.init()
	
func _on_vosk_ready(is_ready :bool):
	if is_ready: print("Vosk is ready.")
	else: print("Vosk not ready")
	
