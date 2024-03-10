extends Control


var vosk
@onready var partial_rich : RichTextLabel = $MarginContainer/VBoxContainer/HBoxContainer/VBoxContainer/ScrollContainer/PartialRich
@onready var final_rich : RichTextLabel = $MarginContainer/VBoxContainer/HBoxContainer/VBoxContainer2/ScrollContainer2/FinalRich

@onready var listen_button : Button = $MarginContainer/VBoxContainer/HBoxContainer2/Listen
@onready var model_path : LineEdit = $MarginContainer/VBoxContainer/HBoxContainer2/ModelPath


var is_listening = false
func _ready():
	listen_button.pressed.connect(_on_listen_button)
	

var check_for_results_every = 10

var elapsed_time = 0
func _process(_delta: float):
	if not is_listening: return
	
		
	elapsed_time += _delta
	if elapsed_time > check_for_results_every:
		elapsed_time = 0
		
		var partial = vosk.get_partial()
		partial_rich.text = partial + "\n" + partial_rich.text
		
		var final = vosk.get_final()
		final_rich.text = final + "\n" + final_rich.text
	
	#vosk.save_accumulated_audio()


func _on_listen_button():
	is_listening = !is_listening
	
	if is_listening:
		listen_button.text = "STOP LISTENING"
		if not vosk:
			vosk = VoskVoiceRecognition.new()
			get_parent().add_child(vosk)
			var model_path = model_path.text
			if vosk.initialize(model_path):
				vosk.set_size_in_ms(1024*4)
				vosk.start()
			
	else:
		listen_button.text = "START LISTENING"
		if vosk:
			vosk.stop()
			vosk.queue_free()
	
