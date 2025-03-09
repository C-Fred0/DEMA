import os
import pandas as pd
import paho.mqtt.client as mqtt
import random

# Configuración del broker MQTT
broker = "broker.emqx.io"
port = 8083
topic = "test/in"

# Generar un ClientID aleatorio
client_id = f"mqttx_fdfb2d88_{random.randint(1000, 9999)}"

# Opciones de conexión
username = "xguax"
password = "xguax"

# Ruta para guardar el archivo CSV
output_dir = r"C:\Users\usuario\Downloads\Pistasdeblue"
os.makedirs(output_dir, exist_ok=True)

# Inicializar DataFrame
df = pd.DataFrame(columns=["X", "Y", "Z", "X1", "X2", "X3"])

# Contador de iteraciones
iteration_count = 0

def convert_to_list(value):
    """Convierte una cadena recibida en una lista de flotantes."""
    try:
        vals = value.strip().split(",")
        results = [float(val) for val in vals if val.strip()]
        return results
    except ValueError as e:
        print(f"Error al convertir los datos: {e}")
        return []

def convert_list_to_df(lst):
  #  """Convierte una lista de datos en un DataFrame con columnas específicas."""
  #  if len(lst) != 180:  # Verificar que se reciban exactamente 180 datos
  #      print("La longitud de los datos recibidos no es válida (se esperan 180 datos).")
  #      return pd.DataFrame()

    x = lst[0::6]
    y = lst[1::6]
    z = lst[2::6]
    x1 = lst[3::6]
    y1 = lst[4::6]
    z1 = lst[5::6]
    df = pd.DataFrame({'X': x, 'Y': y, 'Z': z, 'X1': x1, 'X2': y1, 'X3': z1})
    return df

def on_message(client, userdata, msg):
    """Callback que procesa los mensajes recibidos desde el broker MQTT."""
    global df
    data = msg.payload.decode("utf-8").strip()
    print(f"\n=== Mensaje recibido ===")
    print(f"Tema: {msg.topic}")
    print(f"Contenido: {data}")
    
    # Convertir datos a lista y luego a DataFrame
    lineList = convert_to_list(data)
    new_df = convert_list_to_df(lineList)

    if not new_df.empty:
        print(f"Nueva data adquirida:\n{new_df.describe()}")
        df = pd.concat([df, new_df], ignore_index=True)
    
    else:
        print("No se adquirieron datos válidos.")

def on_connect(client, userdata, flags, rc, properties=None):
    """Callback para manejar la conexión al broker."""
    if rc == 0:
        print(f"Conectado exitosamente al broker MQTT ({broker}:{port})")
        client.subscribe(topic)
        print(f"Suscrito al tema: {topic}")
    else:
        print(f"Error de conexión. Código de retorno: {rc}")

# Crear cliente MQTT
client = mqtt.Client(client_id=client_id, transport="websockets", protocol=mqtt.MQTTv5)
client.username_pw_set(username=username, password=password)

# Configurar los callbacks
client.on_connect = on_connect
client.on_message = on_message

# Solicitar letra y configurar el archivo de salida
letter = input('Por favor, inserte la letra para recolectar datos: ')
stride = 30
output_file = os.path.join(output_dir, f'letter_{letter}_stride_{stride}.csv')

# Conectar al broker y empezar el loop
try:
    print(f"Conectando al broker {broker} en el puerto {port}...")
    client.connect(broker, port)
    client.loop_start()  # Cambiar a loop_start para permitir interacción

    while True:
        user_input = input('1 - adquirir muestra, 2 - salir: ')
        if user_input == '1':
            iteration_count += 1  # Incrementar el contador de iteraciones
            print(f"Iteración actual: {iteration_count}")
            print("Adquiriendo muestras... Asegúrate de que el mensaje contenga 180 datos.")
        elif user_input == '2':
            if not df.empty:
                print('Guardando datos...')
                df.to_csv(output_file, index=False)
                print(f'Datos guardados en: {output_file}')
            else:
                print('No hay datos para guardar.')
            break
        else:
            print('Entrada no válida. Intente de nuevo.')

except KeyboardInterrupt:
    print("\nDesconexión del cliente MQTT.")
finally:
    client.loop_stop()  # Detener el loop del cliente MQTT
    client.disconnect()
