import serial
import serial.tools.list_ports
import json
import time
import requests

def find_esp32_port():
    esp32_identifiers = [
        (4292, 60000), # Standard CP210x/CH340 VIDs
        (6790, 29987)
    ]
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if (port.vid, port.pid) in esp32_identifiers:
            return port.device
    return None

def send_to_thingspeak(data):
    THINGSPEAK_API_KEY = "YOUR_API_KEY_HERE" # Mask this in public GitHub!
    url = f"https://api.thingspeak.com/update?api_key={THINGSPEAK_API_KEY}"
    url += f"&field1={data.get('temperature', 0)}"
    url += f"&field2={data.get('humidity', 0)}"
    url += f"&field3={data.get('eco2', 0)}"
    url += f"&field4={data.get('tvoc', 0)}"
    url += f"&field5={data.get('aqi', 0)}"
    
    try:
        response = requests.get(url, timeout=5)
        print(f"Data sent to ThingSpeak, status: {response.status_code}")
    except requests.exceptions.RequestException as e:
        print(f"ThingSpeak Error: {e}")

def main():
    esp32_port = find_esp32_port()

    if not esp32_port:
        print("Error: ESP32 not found. Check USB connection.")
        return

    try:
        ser = serial.Serial(esp32_port, 115200, timeout=2)
        print(f"Connected to {esp32_port}. Waiting for data...")
        time.sleep(2) 

        while True:
            line_bytes = ser.readline()
            if line_bytes:
                try:
                    json_string = line_bytes.decode('utf-8').strip()
                    if not json_string:
                        continue
                        
                    data_dict = json.loads(json_string)
                    print(f"Received -> Temp: {data_dict['temperature']:.1f}C, "
                          f"eCO2: {data_dict['eco2']}ppm, "
                          f"TVOC: {data_dict['tvoc']}ppb, "
                          f"AQI: {data_dict['aqi']}")
                    
                    send_to_thingspeak(data_dict)

                except (UnicodeDecodeError, json.JSONDecodeError) as e:
                    print(f"Invalid data received: {e}")
            
    except serial.SerialException as e:
        print(f"Serial connection error: {e}")
    except KeyboardInterrupt:
        print("\nProgram terminated.")
    finally:
        if 'ser' in locals() and ser.isOpen():
            ser.close()

if __name__ == "__main__":
    main()