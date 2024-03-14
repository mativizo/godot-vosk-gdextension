extends Control

var vosk
@onready var partial_rich : RichTextLabel = $MarginContainer/VBoxContainer/HBoxContainer/VBoxContainer/ScrollContainer/PartialRich
@onready var final_rich : RichTextLabel = $MarginContainer/VBoxContainer/HBoxContainer/VBoxContainer2/ScrollContainer2/FinalRich

@onready var input_device_option : OptionButton = $MarginContainer/VBoxContainer/HBoxContainer2/InputDeviceOption

@onready var model_path : LineEdit = $MarginContainer/VBoxContainer/HBoxContainer2/ModelPath


var is_listening = false

var input_devices : Array = []
var partial_timer : Timer
var final_timer : Timer

func _ready():
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
	print(vosk.get_current_input_device_info())
		
	

func _process(_delta: float):
	if Input.is_action_pressed("push_to_talk"):
		if not vosk.is_listening():
			var final_result = vosk.get_final_result()
			if final_result != "": update_final_result(final_result)
			if vosk: vosk.cleanup()
			var model_path = model_path.text
			if vosk.switch_model_and_init(model_path):
				vosk.set_size_in_ms(1024)
				vosk.start()
	else:
		if vosk.is_listening():
			vosk.stop()
			var status = vosk.get_status()
		
			if status:
				var partial_result = vosk.get_partial_result()
				if partial_result != "": update_partial_result(partial_result)
				

func update_partial_result(partial_text : String):
	partial_rich.text += "\n" + partial_text

func update_final_result(final_text : String):
	final_rich.text += "\n" + final_text

func _on_switch_device(opt_id : int):
	vosk.set_input_device(input_devices[opt_id].id)
