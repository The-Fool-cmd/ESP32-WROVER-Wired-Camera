import serial
import struct
import cv2
import numpy as np
import time

SERIAL_PORT = 'COM7'
BAUD_RATE = 6000000  # match the ESP32-S3 baud rate

def read_exactly(ser, length):
    data = b''
    while len(data) < length:
        more = ser.read(length - len(data))
        if not more:
            raise TimeoutError("Serial timeout")
        data += more
    return data

ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.5)
ser.reset_input_buffer()
time.sleep(0.5)

frame_count = 0
start = time.time()

while True:
    try:
        ser.write(b'c')  # Request a new frame

        marker = read_exactly(ser, 4)
        if marker != b'FRAM':
            raise ValueError(f"Invalid marker: {marker}")

        size_bytes = read_exactly(ser, 4)
        frame_len = struct.unpack('<I', size_bytes)[0]

        if not (1000 < frame_len < 200000):
            print(f"Invalid frame size: {frame_len}")
            continue

        jpeg = read_exactly(ser, frame_len)

        np_arr = np.frombuffer(jpeg, dtype=np.uint8)
        frame = cv2.imdecode(np_arr, cv2.IMREAD_COLOR)

        if frame is not None:
            frame_count += 1
            elapsed = time.time() - start
            if elapsed >= 1.0:
                print(f"FPS: {frame_count / elapsed:.2f}")
                frame_count = 0
                start = time.time()

            cv2.imshow("ESP32-S3 Camera", frame)
        else:
            print("Decode failed")

        if cv2.waitKey(1) == 27:
            break

    except Exception as e:
        print("Error:", e)
        ser.reset_input_buffer()

ser.close()
cv2.destroyAllWindows()
