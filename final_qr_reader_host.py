import cv2
import numpy as np
import pyzbar.pyzbar as pyzbar
import urllib.request
from http.server import BaseHTTPRequestHandler, HTTPServer
import requests
import threading

# QR code processing variables
font = cv2.FONT_HERSHEY_PLAIN
url = 'http://192.168.62.74'

cv2.namedWindow("live transmission", cv2.WINDOW_AUTOSIZE)
prev_data = ""

# HTTP server variables
hostName = "localhost"
hostPort = 9000

# HTTP server handler
class MyServer(BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()

        inputValue = self.path.split("=")[-1]
        print("Received inputValue:", inputValue)

        # You can add your own logic here to handle the received data
        self.wfile.write(bytes("<html><head><title>QR Data Received</title></head>", "utf-8"))
        self.wfile.write(bytes("<body>", "utf-8"))
        self.wfile.write(bytes(f"<p>Received input value: {inputValue}</p>", "utf-8"))
        self.wfile.write(bytes("</body></html>", "utf-8"))

# Start HTTP server in a separate thread
def start_http_server():
    webServer = HTTPServer((hostName, hostPort), MyServer)
    print(f"Server started http://{hostName}:{hostPort}")

    try:
        webServer.serve_forever()
    except KeyboardInterrupt:
        pass

    webServer.server_close()
    print("Server stopped.")

# QR code processing and HTTP request
def process_qr_code():
    global prev_data

    while True:
        img_resp = urllib.request.urlopen(url + '/cam-mid.jpg')
        imgnp = np.array(bytearray(img_resp.read()), dtype=np.uint8)
        frame = cv2.imdecode(imgnp, -1)

        decodedObjects = pyzbar.decode(frame)
        for obj in decodedObjects:
            data = obj.data.decode('utf-8')  # Decode bytes to string
            if prev_data == data:
                pass
            else:
                print("Data: ", data)
                print("http://192.168.62.9/setServos?inputValue={0}".format(data))
                # Make HTTP request
                try:
                    response = requests.get("http://192.168.62.9/setServos", params={"inputValue": data})
                    print("HTTP Response:", response.text)
                except requests.exceptions.RequestException as e:
                    print("Error making HTTP request:", e)
                    
                prev_data = data
            
            cv2.putText(frame, data, (50, 50), font, 2, (255, 0, 0), 3)

        cv2.imshow("live transmission", frame)

        key = cv2.waitKey(1)
        if key == 27:  # Esc key
            break

    cv2.destroyAllWindows()

# Main function
if __name__ == "__main__":
    # Start HTTP server in a separate thread
    http_server_thread = threading.Thread(target=start_http_server)
    http_server_thread.start()

    # Start QR code processing
    process_qr_code()

    # Wait for HTTP server thread to complete
    http_server_thread.join()

