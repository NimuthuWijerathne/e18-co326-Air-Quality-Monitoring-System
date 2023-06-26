import paho.mqtt.client as mqtt
from pymongo import MongoClient

mqtt_broker = "test.mosquitto.org"
mqtt_port = 1883
topic_temp = "326/grp19/sensors/DHT11/temp"
topic_humidity = "326/grp19/sensors/DHT11/humi"
topic_airquality = "326/grp19/sensors/MQ135/"
topic_LED = "326/grp19/Actuators/LED/"
topic_buzzer = "326/grp19/Actuators/Buzzer/"
topic = "326/grp19/"

# MongoDB connection details
mongo_host = 'localhost'
mongo_port = 27017
mongo_database = 'sensor_data'
mongo_collection = 'sensors'

def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT broker")
    client.subscribe(topic_temp)
    client.subscribe(topic_humidity)
    client.subscribe(topic_airquality)

def on_message(client, userdata, msg):
    print("Received message: " + msg.topic + " " + str(msg.payload))
    # Save sensor data to MongoDB
    data = {
        'topic': msg.topic,
        'payload': str(msg.payload)
    }
    save_to_mongodb(data)

def save_to_mongodb(data):
    client = MongoClient(mongo_host, mongo_port)
    db = client[mongo_database]
    collection = db[mongo_collection]
    collection.insert_one(data)
    client.close()

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(mqtt_broker, mqtt_port, 60)

# Start the MQTT client loop in a separate thread
client.loop_start()

# Run indefinitely
while True:
    pass

# Stop the MQTT client loop (will never reach this point)
client.loop_stop()
