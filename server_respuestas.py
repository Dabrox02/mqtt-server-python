import paho.mqtt.client as mqtt
from flask import Flask, jsonify
from threading import Thread, Lock
from flask_cors import CORS

# Variables globales
app = Flask(__name__)
cors = CORS(app, resources={r"/*": {"origins": ["http://127.0.0.1:5001", "http://localhost:5001"]}})

data_hum = []
data_temp = []
data_lock = Lock()  # Lock para proteger el acceso a data

# Función de callback para cuando se recibe un mensaje MQTT
def on_message_hum(client, userdata, message):
    global data_hum
    with data_lock:
        message_rec = {
            "topic": message.topic,
            "payload": str(message.payload.decode("utf-8"))
        }
        data_hum.append(message_rec)
        print("message received: ", message_rec["payload"])

def on_message_temp(client, userdata, message):
    global data_temp
    with data_lock:
        message_rec = {
            "topic": message.topic,
            "payload": str(message.payload.decode("utf-8"))
        }
        data_temp.append(message_rec)
        print("message received: ", message_rec["payload"])

# Función para datos de humedad
def start_mqtt_humedad():
    client = mqtt.Client(client_id="ClienteHumedad", callback_api_version=mqtt.CallbackAPIVersion.VERSION2)
    client.on_message = on_message_hum
    client.connect("broker.emqx.io")
    client.subscribe("uts-iot/mqtt-jaider/hum")
    client.loop_start()

# Función para datos de temperatura
def start_mqtt_temperatura():
    client = mqtt.Client(client_id="ClienteTemperatura", callback_api_version=mqtt.CallbackAPIVersion.VERSION2)
    client.on_message = on_message_temp
    client.connect("broker.emqx.io")
    client.subscribe("uts-iot/mqtt-jaider/temp")
    client.loop_start()

# Ruta de Flask para mostrar los datos    
@app.route('/humedad', methods=['GET'])
def get_data_humedad():
    global data_hum
    with data_lock:
        return jsonify(data_hum)

# Ruta de Flask para mostrar los datos    
@app.route('/temperatura', methods=['GET'])
def get_data_temperatura():
    global data_temp
    with data_lock:
        return jsonify(data_temp)

# Iniciar MQTT en un hilo separado
thread_hum = Thread(target=start_mqtt_humedad)
thread_temp = Thread(target=start_mqtt_temperatura)
thread_hum.start()
thread_temp.start()

# Iniciar el servidor web
if __name__ == '__main__':
    app.run(port=5000)
