extends Control


var vosk
@onready var partial_rich : RichTextLabel = $MarginContainer/VBoxContainer/HBoxContainer/VBoxContainer/ScrollContainer/PartialRich
@onready var final_rich : RichTextLabel = $MarginContainer/VBoxContainer/HBoxContainer/VBoxContainer2/ScrollContainer2/FinalRich

@onready var listen_button : Button = $MarginContainer/VBoxContainer/HBoxContainer2/Listen
@onready var model_path : LineEdit = $MarginContainer/VBoxContainer/HBoxContainer2/ModelPath


var is_listening = false
var elapsed_time = 0

func _ready():
	listen_button.pressed.connect(_on_listen_button)
	


func _process(_delta: float):
	if not is_listening: return
	#vosk.save_accumulated_audio()


func _on_listen_button():
	is_listening = !is_listening
	
	if is_listening:
		listen_button.text = "STOP LISTENING"
		if not vosk:
			vosk = VoskVoiceRecognition.new()
			add_child(vosk)
			var model_path = model_path.text
			if vosk.initialize(model_path):
				vosk.set_size_in_ms(1024)
				vosk.start()
			
	else:
		listen_button.text = "START LISTENING"
		if vosk:
			vosk.stop()
			vosk.queue_free()
	

func _on_partial_result_signal(partial_text : String):
	partial_rich.text += "\n" + partial_text
	
func _on_final_result_signal(final_text : String):
	final_rich.text += "\n" + final_text

