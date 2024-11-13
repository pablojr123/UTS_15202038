import paho.mqtt.client as mqtt
import json
from flask import Flask, jsonify
import threading

# MQTT Broker settings
broker = 'broker.emqx.io'
port = 1883
mqtt_topic = 'UTS_152022038/emqx/esp32'
username = 'daniiryanto'
password = 'akudaniiryanto1'

# Data yang akan disimpan dari pesan MQTT
mqtt_data = {}

# Callback function when the client receives a CONNACK response from the server
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT Broker!")
        client.subscribe(mqtt_topic)
    else:
        print(f"Failed to connect, return code {rc}")

# Callback function when a PUBLISH message is received from the server
def on_message(client, userdata, msg):
    
    global mqtt_data
    try:
        payload = msg.payload.decode('utf-8').strip()  # Pastikan tidak ada spasi atau karakter kosong
        print(f"Message received on topic '{msg.topic}': {payload}")
        
        # Validasi jika payload memang dalam format JSON
        if payload.startswith('{') and payload.endswith('}'):
            data = json.loads(payload)
            mqtt_data = data
        else:
            print("Invalid JSON format received.")
    except json.JSONDecodeError as e:
        print(f"Error decoding JSON: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")

# Flask app setup
app = Flask(__name__)

# Endpoint API untuk mengakses data MQTT
@app.route('/mqtt_data', methods=['GET'])
def get_mqtt_data():
    if mqtt_data:
        return jsonify(mqtt_data)  # Mengirimkan data MQTT dalam format JSON
    else:
        return jsonify({"error": "No data available"}), 404

# Fungsi untuk menjalankan client MQTT dalam thread terpisah
def mqtt_thread():
    client = mqtt.Client()
    client.username_pw_set(username, password)
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(broker, port, 60)
    client.loop_forever()

if __name__ == '__main__':
    # Jalankan client MQTT di thread terpisah
    threading.Thread(target=mqtt_thread, daemon=True).start()
    
    # Jalankan aplikasi Flask
    app.run(debug=True, host='0.0.0.0', port=5000)
