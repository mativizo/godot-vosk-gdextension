extends Control


var vosk
@onready var partial_rich : RichTextLabel = $MarginContainer/VBoxContainer/HBoxContainer/VBoxContainer/ScrollContainer/PartialRich
@onready var final_rich : RichTextLabel = $MarginContainer/VBoxContainer/HBoxContainer/VBoxContainer2/ScrollContainer2/FinalRich
@onready var status_label : Label = $MarginContainer/VBoxContainer/Status

@onready var input_device_option : OptionButton = $MarginContainer/VBoxContainer/HBoxContainer2/InputDeviceOption

@onready var listen_button : Button = $MarginContainer/VBoxContainer/HBoxContainer2/Listen
@onready var model_path : LineEdit = $MarginContainer/VBoxContainer/HBoxContainer2/ModelPath


var is_listening = false
var elapsed_time = 0

var input_devices : Array = []
var partial_timer : Timer
var final_timer : Timer
var json

func _ready():
	json = JSON.new()
	listen_button.pressed.connect(_on_listen_button)
	input_device_option.item_selected.connect(_on_switch_device)
	
	partial_timer = Timer.new()
	partial_timer.set_wait_time(0.5)
	partial_timer.set_autostart(true)
	
	vosk = VoskVoiceRecognition.new()
	vosk.set_logs(true)
	add_child(vosk)
	
	input_devices = vosk.get_input_devices()
	for input_device_info in input_devices:
		input_device_option.add_item(str(input_device_info.id) + ":" + input_device_info.name)
	

var call_every_x_seconds = 2
var time_elapsed = 0
func _process(_delta: float):
	time_elapsed += _delta
	if call_every_x_seconds < time_elapsed:
		var status = vosk.get_status()
		status_label.text = "Status: " +str(status)
		
		if status == 1:
			update_partial_result(vosk.get_partial_result())
			#update_final_result(vosk.get_final_result())
	
		
	
	#vosk.save_accumulated_audio()

func _on_listen_button():
	is_listening = !is_listening
	
	if is_listening:
		listen_button.text = "STOP LISTENING"
		
		var model_path = model_path.text
		if vosk.switch_model_and_init(model_path):
			vosk.set_size_in_ms(1024)
			vosk.start()
			
	else:
		listen_button.text = "START LISTENING"
		if vosk: vosk.stop()
	


func update_partial_result(partial_text : Dictionary):
	if "partial" in partial_text and partial_text.partial != "":
		partial_rich.text = partial_text.partial + "\n" + partial_rich.text
	
func update_final_result(final_text : Dictionary):
	if "text" in final_text and final_text.text != "":
		final_rich.text = final_text.text + "\n" + final_rich.text
	

func _on_switch_device(opt_id : int):
	vosk.set_input_device(input_devices[opt_id].id)
