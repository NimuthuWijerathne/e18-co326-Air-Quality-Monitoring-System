import paho.mqtt.client as mqtt
import mysql.connector

# MQTT topics
topic_temp = "UoP/CO/326/E18/19/DHT11/temperature"
topic_humidity = "UoP/CO/326/E18/19/DHT11/humidity"
topic_airquality = "UoP/CO/326/E18/19/MQ135/AirQuality"
topic_LED = "UoP/CO/326/E18/19/LED"
topic_buzzer = "UoP/CO/326/E18/19/Buzzer"


# MQTT broker details
mqtt_broker = "test.mosquitto.org"
mqtt_port = 1883

# MySQL database details
mysql_host = "localhost"
mysql_user = "root"
mysql_password = ""
mysql_database = "326_project"


# MQTT topics
topics = {
    "temperature": "UoP/CO/326/E18/19/DHT11/temperature",
    "humidity": "UoP/CO/326/E18/19/DHT11/humidity",
    "air_quality": "UoP/CO/326/E18/19/MQ135/AirQuality",
    "LED": "UoP/CO/326/E18/19/LED",
    "buzzer": "UoP/CO/326/E18/19/Buzzer"
}

sensor_data = {
    "temperature": None,
    "humidity": None,
    "air_quality": None
}


def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT broker")
    # Subscribe to all topics
    for topic in topics.values():
        client.subscribe(topic)


def on_message(client, userdata, msg):
    print("Received message: " + msg.topic + " " + str(msg.payload))
    # Store sensor data
    store_sensor_data(msg.topic, msg.payload.decode())


def store_sensor_data(topic, payload):
    # Update the corresponding sensor value
    for sensor, topic_name in topics.items():
        if topic == topic_name:
            sensor_data[sensor] = float(payload)

    # If all sensor data is available, insert into the database
    if all(value is not None for value in sensor_data.values()):
        insert_data()


def insert_data():
    try:
        # Connect to MySQL database
        conn = mysql.connector.connect(
            host=mysql_host,
            user=mysql_user,
            password=mysql_password,
            database=mysql_database
        )
        cursor = conn.cursor()

        # Create SQL query to insert data into the table
        sql = "INSERT INTO sensor_data (temperature, humidity, air_quality) VALUES (%s, %s, %s)"
        values = tuple(sensor_data.values())

        # Execute the query
        cursor.execute(sql, values)
        conn.commit()

        # Reset sensor data
        for sensor in sensor_data:
            sensor_data[sensor] = None

        # Close the database connection
        cursor.close()
        conn.close()
    except mysql.connector.Error as error:
        print("Error while connecting to MySQL:", error)


# MQTT client setup
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# Connect to MQTT broker
client.connect(mqtt_broker, mqtt_port, 60)

# Start the MQTT client loop in a separate thread
client.loop_start()

# Run indefinitely
while True:
    pass

# Stop the MQTT client loop (will never reach this point)
client.loop_stop()
