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

'''
## Functions to calculate sleep factor
def data_read_duration(sleep_factor, test_number_of_samples):
    samples_recorded = 1
    data_read_start_time = datetime.datetime.now(timezone)
    time_before_reading = data_read_start_time
    while samples_recorded <= test_number_of_samples+1:

        ## Read data
        data_read1 = chan_1.voltage
        data_read2 = chan_2.voltage
        time_elapsed = time_before_reading - data_read_start_time
        # print(f'{samples_recorded}\t',format(time_elapsed.total_seconds()*1000,'12g'),f'\t\t{data_read1},\t\t{data_read2}')
        print(f'{samples_recorded},',format(time_elapsed.total_seconds()*1000,),f',{data_read1}',f',{data_read2}')
        samples_recorded += 1

        ## Decide how long to sleep
        time_before_sleep = datetime.datetime.now(timezone)
        Intended_duration_to_sleep = time_before_sleep - time_before_reading
        if Intended_duration_to_sleep.total_seconds() < sampling_interval:
            time.sleep(sleep_factor*(sampling_interval-Intended_duration_to_sleep.total_seconds()))

        ## Current Timestamp
        time_before_reading = datetime.datetime.now(timezone)

    return time_elapsed.total_seconds()

def sleep_factor_calculator(sampling_interval,test_number_of_samples,offset_low,offset_high):
    sleep_factor_high = 1.0
    sleep_factor_low = 0.0
    read_duration_high = data_read_duration(sleep_factor_high,test_number_of_samples)
    read_duration_low = data_read_duration(sleep_factor_low,test_number_of_samples)
    while True:
        # print(f'with high factor {sleep_factor_high} duration is {read_duration_high},with low factor {sleep_factor_low} duration is {read_duration_low}')
        if sleep_factor_high == sleep_factor_low:
            sleep_factor = sleep_factor_high
            break

        if (read_duration_high >= (sampling_interval*test_number_of_samples)-offset_low and read_duration_high <= (sampling_interval*test_number_of_samples)+offset_high):
            sleep_factor = sleep_factor_high
            break
        elif (read_duration_low >= (sampling_interval*test_number_of_samples)-offset_low and read_duration_low <= (sampling_interval*test_number_of_samples)+offset_high):
            sleep_factor = sleep_factor_low
            break
        else:
            sleep_factor_mid = (sleep_factor_high+sleep_factor_low)/2
            read_duration_mid = data_read_duration(sleep_factor_mid,test_number_of_samples)
            if read_duration_mid < sampling_interval*test_number_of_samples:
                sleep_factor_low = sleep_factor_mid
                read_duration_low = read_duration_mid
            elif read_duration_mid > sampling_interval*test_number_of_samples:
                sleep_factor_high = sleep_factor_mid
                read_duration_high = read_duration_mid

    return sleep_factor
'''

## Timezone
timezone = pytz.timezone("US/Eastern")

## Create the I2C bus
i2c = busio.I2C(board.SCL, board.SDA)

## Create ADC object using I2C bus
ads = ADS.ADS1115(i2c,data_rate=475)

##Create single-ended/differential input
chan_1 = AnalogIn(ads, ADS.P1)

## Logging level
logging.basicConfig(level=logging.NOTSET)

## Sampling Rate (Hz), Sampling Duration (Min)
sampling_rate = 35
data_log_duration = 10

sampling_interval = 1/sampling_rate
total_number_of_samples = int(data_log_duration*60*(1/sampling_interval))

'''
## Sleep factor calculation: sleep_factor_function(sampling_interval,test_number_of_samples,offset_low,offset_high)
acceptable_offset = 0.315                 # Acceptable offset in seconds to read complete data
test_number_of_samples = 50               # Number of samples used to calculate sleep factor
offset_high = acceptable_offset * (test_number_of_samples/total_number_of_samples)           # Offset calculated to provide small variance for sleep factor calculation 
sleep_factor = sleep_factor_calculator(sampling_interval,test_number_of_samples,0.0,offset_high)
print(f'Sleep factor: {sleep_factor}')
'''


sleep_factor = 0.001
## Data reading code
samples_recorded = 1
data_read_start_time = datetime.datetime.now(timezone)
time_before_reading = data_read_start_time
print('Start')
while samples_recorded <= total_number_of_samples:

    ## Read data
    data_read1 = chan_1.voltage
    time_elapsed = time_before_reading - data_read_start_time
    # print(f'{samples_recorded}\t',format(time_elapsed.total_seconds()*1000,'12g'),f'\t\t{data_read1},\t\t{data_read2}')
    # print(f'{samples_recorded},',format(time_elapsed.total_seconds()*1000,),f',{data_read1}')
    # logging.info(f'{samples_recorded},',format(time_elapsed.total_seconds()*1000,),f',{data_read1}')
    #sys.stdout.write("%s"%samples_recorded)
    sys.stdout.write("\t%s"%format(time_elapsed.total_seconds()*1000,))
    sys.stdout.write("\t%s"%data_read1)
    sys.stdout.write("\n")
    samples_recorded += 1

    ## Decide how long to sleep
    time_before_sleep = datetime.datetime.now(timezone)
    Intended_duration_to_sleep = time_before_sleep - time_before_reading
    
    '''
    if Intended_duration_to_sleep.total_seconds() < sampling_interval:
        time.sleep(sleep_factor*(sampling_interval-Intended_duration_to_sleep.total_seconds()))
        '''

    ## Current Timestamp
    time_before_reading = datetime.datetime.now(timezone)
print('Done')
