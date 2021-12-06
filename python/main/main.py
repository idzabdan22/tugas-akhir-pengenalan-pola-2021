import csv
from re import T
import time
import serial
import sys
import argparse
import pandas as pd

from numpy.random.mtrand import uniform
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score
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
        return knn_classifier(arr, arguments["neighbours"])
    else:
        while(not data):
            data = ser.readline()
            pass
        return None 

def knn_classifier(data, neighbours):
    global indep_data, dep_data
    knn = neighbors.KNeighborsClassifier(n_neighbors=neighbours, weights="uniform")
    knn.fit(indep_data, dep_data)
    predict_knn = knn.predict([data])
    if predict_knn == "Kunci":
        output = "K" # Kelas Jeruk Kunci
    else:
        output = "S" # Kelas Jeruk Santang Madu
    time.sleep(1)
    ser.write(bytes(output, 'utf-8'))

if __name__ == "__main__":
    read_data()
    time.sleep(2)
    ser.write([1])
    while True:
        incoming_bytes = process_incoming_data(ser.readline())
        print(incoming_bytes)
        
        
    