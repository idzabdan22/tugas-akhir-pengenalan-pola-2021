from re import T
import time
from numpy import byte
import serial
import argparse
import pandas as pd

from numpy.random.mtrand import uniform
from sklearn import *

SERIAL_PORT = "/dev/ttyACM0"
BAUDRATE = 9600

# Init Serial Communication
ser = serial.Serial(SERIAL_PORT, BAUDRATE, timeout=1)

# Argument parser for n neighbours from terminal argument
ap = argparse.ArgumentParser()
ap.add_argument("-n", "--neighbours", type=int, default=3, help="set the number of neighbours for voting purpose")
arguments = vars(ap.parse_args())

# Variable for data
dep_data = 0
indep_data = 0
incoming_bytes = b''

def read_data():
    global indep_data, dep_data
    data_path = '/home/danidzz/Development/pengenalan_pola_project/python/asset/jeruk.csv'
    df = pd.read_csv(data_path)
    # memisahkan data independent (fitur) dan dependent (class)
    indep_data = df.iloc[:, 1:5]
    dep_data = df.iloc[:, 5]

def process_incoming_data(data):
    if(data == b's\r\n'):
        arduino_data = ""
        arr = []
        data = ser.readline().decode('ascii')
        while(not data):
            data = ser.readline().decode('ascii')
            pass
        data = data[0:len(data)-2]
        for x in data:
            arduino_data += x
        arr = arduino_data.split(",")
        for i in range(0, len(arr)):
            arr[i] = int(arr[i])
        print(arr)
        return knn_classifier(arr, arguments["neighbours"])
    else:
        print(data)
        while(not data):
            data = ser.readline().decode('ascii')
            pass
        return None 

def knn_classifier(data, neighbours):
    global indep_data, dep_data, output
    knn = neighbors.KNeighborsClassifier(n_neighbors=neighbours, weights="uniform")
    knn.fit(indep_data, dep_data)
    predict_knn = knn.predict([data])
    if predict_knn == "Kunci":
        time.sleep(1)
        ser.write(b'f')
        time.sleep(1)
        ser.write("k".encode())
        print("Jeruk Kunci")
    else:
        time.sleep(1)
        ser.write(b'f')
        time.sleep(1)
        ser.write("s".encode())
        print("Jeruk S Madu")
    time.sleep(1)
    
if __name__ == "__main__":
    read_data()
    time.sleep(2)
    ser.write([1])
    while True:
        incoming_bytes = process_incoming_data(ser.readline())
        print(incoming_bytes)
        
        
    