#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Created on Fri Sep 30 20:37:54 2022

@author: amitkumarbhuyan
"""

## Pi and ADC library
import time
import board
import busio
import adafruit_ads1x15.ads1115 as ADS
from adafruit_ads1x15.analog_in import AnalogIn
import ctypes
## CSV library import
import csv

## Firebase library import
import datetime
import pytz
import requests
## from firebase import firebase

## Threading library import
import threading

## Logging module
import logging

## Set logging level to info
logging.basicConfig(level=logging.INFO)

## Threading for firebase uploading
#def firebase_data_upload(data_upload_dictionary):
    ## Firebase data upload
    #logging.info("Firebase upload thread starts")
    #database = 'https://yoshi-8c738.firebaseio.com/'   
    #fb = firebase.FirebaseApplication(database, None)
    #result = fb.patch('/test_50hz_30min',data_upload_dictionary)
    #logging.info("Firebase upload thread ends")


## Firebase setup
## data_read = {}
####  database = 'https://upsdataloggingtest-default-rtdb.firebaseio.com/'

## Timezone
#timezone = pytz.timezone("US/Eastern")
#start_time = datetime.datetime.now(timezone)

## Create the I2C bus
#i2c = busio.I2C(board.SCL, board.SDA)

## Create ADC object using I2C bus
#ads = ADS.ADS1115(i2c)

##Create single-ended/differential input
#chan_1 = AnalogIn(ads, ADS.P1)

## logging.info("{:>5}\t{:>5}".format("Time", "Voltage"))
#sleep_factor = 0.1
## Sampling rate (Hz), Sampling Duration (Min)

sampling_rate = 860
#data_log_duration = 1.5

#sampling_interval = 1/sampling_rate
#total_data = int(data_log_duration*60*(1/sampling_interval))

#samples_recorded = 1
#while samples_recorded < total_data:

        ## Current Timestamp
    #time_before_reading = datetime.datetime.now(timezone)

    #data_read = chan_1.voltage
    #print(f'{samples_recorded}, {data_read}')
    # print(f'{data_read}')
    #samples_recorded += 1

    ## Decide how long to sleep
    #time_before_sleep = datetime.datetime.now(timezone)
    #Intended_duration_to_sleep = time_before_sleep - time_before_reading
    #if Intended_duration_to_sleep.total_seconds() < sampling_interval:
        #time.sleep(sleep_factor*(sampling_interval-Intended_duration_to_sleep.total_seconds()))
print("test")
lib = ctypes.CDLL('./libsampling.so')
lib.main.argtypes = (ctypes.c_int, ctypes.POINTER(ctypes.c_char_p))
lib.main.restype = None


args = (ctypes.c_char_p * 3)(b"./program", b"python_c_csv", b"1") #The third parameter '1' here is for 1 minute collection, it can be changed according to what you need
argc = len(args)

result = lib.main(argc, args)
