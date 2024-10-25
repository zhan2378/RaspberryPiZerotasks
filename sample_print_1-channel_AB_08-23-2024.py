## Pi and ADC library
import time
import board
import busio
import adafruit_ads1x15.ads1115 as ADS
from adafruit_ads1x15.analog_in import AnalogIn

## Timezone library import
import datetime
import pytz
import logging
import sys

## Timezone
timezone = pytz.timezone("US/Eastern")

## Create the I2C bus
i2c = busio.I2C(board.SCL, board.SDA)

## Create ADC object using I2C bus
ads = ADS.ADS1115(i2c)

##Create single-ended/differential input
chan_1 = AnalogIn(ads, ADS.P2, ADS.P3)

## Logging level
logging.basicConfig(level=logging.NOTSET)

## Sampling Rate (Hz), Sampling Duration (Min)
sampling_rate = 1
data_log_duration = 10

sampling_interval = 1/sampling_rate
total_number_of_samples = int(data_log_duration*60*(1/sampling_interval))

## Data reading code
samples_recorded = 1
data_read_start_time = datetime.datetime.now(timezone)
time_before_reading = data_read_start_time
print('Start')
while samples_recorded <= total_number_of_samples:

    ## Read data
    data_read1 = chan_1.voltage
    time_elapsed = time_before_reading - data_read_start_time
    sys.stdout.write("\t%s"%format(time_elapsed.total_seconds()*1000,))
    sys.stdout.write("\t%s"%data_read1)
    sys.stdout.write("\n")
    samples_recorded += 1

    ## Decide how long to sleep
    time.sleep(sampling_interval)

    ## Current Timestamp
    time_before_reading = datetime.datetime.now(timezone)
print('Done')
