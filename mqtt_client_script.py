import tkinter as tk
import paho.mqtt.client as mqtt

mqtt_broker = "test.mosquitto.org"
mqtt_port = 1883
topic_temp = "326/grp19/sensors/DHT11/temp"
topic_humidity = "326/grp19/sensors/DHT11/humi"
topic_airquality = "326/grp19/sensors/MQ135/"
topic_LED = "326/grp19/Actuators/LED/"
topic_buzzer = "326/grp19/Actuators/Buzzer/"
topic = "326/grp19/"

def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT broker")
    client.subscribe(topic_temp)
    client.subscribe(topic_humidity)
    client.subscribe(topic_airquality)
    client.subscribe(topic_LED)
    client.subscribe(topic_buzzer)

def on_message(client, userdata, msg):
    print("Received message: " + msg.topic + " " + str(msg.payload))
    # Update the message in the UI
    received_message.set("Received message: " + msg.topic + " " + str(msg.payload))

def send_command():
    # Get the command from the input field
    command = command_entry.get()
    # Publish the command to the respective topic
    client.publish(topic_LED, command)  # Modify the topic as per your requirement
    # Clear the input field
    command_entry.delete(0, tk.END)

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(mqtt_broker, mqtt_port, 60)

# Create the Tkinter UI
root = tk.Tk()
root.title("MQTT Client")
root.geometry("400x300")

# Message label to display incoming messages
received_message = tk.StringVar()
message_label = tk.Label(root, textvariable=received_message)
message_label.pack()

# Command entry field
command_entry = tk.Entry(root, width=30)
command_entry.pack()

# Send button to send commands
send_button = tk.Button(root, text="Send", command=send_command)
send_button.pack()

# Start the MQTT client loop in a separate thread
client.loop_start()

# Run the Tkinter event loop
root.mainloop()

# Stop the MQTT client loop after closing the UI window
client.loop_stop()
