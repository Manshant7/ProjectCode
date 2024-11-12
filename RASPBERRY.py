import paho.mqtt.client as mqtt
import RPi.GPIO as GPIO
import time

# MQTT settings
mqtt_broker = "broker.hivemq.com"  # Public MQTT broker
soil_moisture_topic = "device/SoilMoisture"  # Topic for soil moisture readings
animal_alert_topic = "device/AnimalAlert"  # Topic for animal intrusion alerts
moisture_threshold = 300  # Threshold for soil moisture; adjust to your sensor's calibration

# GPIO setup for motor control
motor_pin = 17  # Define GPIO pin connected to motor control
GPIO.setmode(GPIO.BCM)  # Use Broadcom pin numbering
GPIO.setup(motor_pin, GPIO.OUT)  # Set motor pin as output

# Define MQTT client callbacks

def on_connect(client, userdata, flags, rc):
    """Callback function for MQTT connection event."""
    print("Connected to MQTT broker with result code", rc)
    # Subscribe to soil moisture and animal alert topics after connecting
    client.subscribe(soil_moisture_topic)
    client.subscribe(animal_alert_topic)

def on_message(client, userdata, msg):
    """Callback function for handling received MQTT messages."""
    # Handle soil moisture messages
    if msg.topic == soil_moisture_topic:
        try:
            moisture_level = int(msg.payload.decode())  # Convert message to integer
            print(f"Received soil moisture level: {moisture_level}")

            # Activate motor if moisture level is below threshold
            if moisture_level < moisture_threshold:
                print("Soil is dry. Activating motor...")
                GPIO.output(motor_pin, GPIO.HIGH)  # Turn motor ON
                time.sleep(5)  # Run motor for 5 seconds
                GPIO.output(motor_pin, GPIO.LOW)   # Turn motor OFF
                print("Motor deactivated")
            else:
                print("Soil moisture is sufficient.")
        except ValueError:
            print("Invalid soil moisture data received")

    # Handle animal detection messages
    elif msg.topic == animal_alert_topic:
        alert_message = msg.payload.decode()
        print(f"Alert: {alert_message}")

# MQTT client setup
client = mqtt.Client()
client.on_connect = on_connect  # Set callback for connect event
client.on_message = on_message  # Set callback for message event

# Connect to the MQTT broker
client.connect(mqtt_broker, 1883, 60)  # Port 1883 is default for MQTT

# Start MQTT loop to process network traffic and dispatch callbacks
client.loop_start()

try:
    while True:
        time.sleep(1)  # Keep script running
except KeyboardInterrupt:
    print("Exiting...")
finally:
    # Cleanup GPIO and MQTT on exit
    GPIO.cleanup()
    client.loop_stop()
    client.disconnect()
