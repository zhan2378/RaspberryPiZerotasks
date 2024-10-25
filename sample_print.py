## Pi and ADC library

import argparse
import ctypes
from time import sleep

'''
## Readme ##
This script is used for data sampling on Raspberry pi zero with ADS1115.
The c code script 'pulse_detection_sampling.c' is called in this script, and
it takes care of most of data sampling operations.

To run this script, you need to do the following steps firstly:
1. Install the wiringpi library:
        sudo apt-get update
        sudo apt-get install git
        git clone https://github.com/WiringPi/WiringPi.git
        cd WiringPi
        ./build debian
        cd debian-template
        sudo apt install ./wiringpi_3.10_armhf.deb
        cd
2. Connect the Alert pin on ADS1115 to Pi zero GPIO
    The data sampling is triggered by the interrupt signal from Alert pin on ADS1115.
    The GPIO position info can be printed out by the following code:
        gpio readall
    Find the GPIO pin with WPi number 25, which should be just above the bottom pin on the left column(GND).
    Connect this pin to the Alert pin on ADS1115 with a wire.
    
3. Compile the C code
    Put this script and 'pulse_detection_sampling.c' file under the same directory
    run the following code:
        gcc -shared -o sampling_reads.so -fPIC pulse_detection_sampling.c -l wiringPi
    
4. Now this script should be ready to run. 
    If we want to run the code through bash commands:
    
    Add the first parameter 'data_log_duration' to specify how many seconds of sampling duration you need in each round of collection.
    Add the second parameter 'loop_rounds' to specify how many rounds of collection you need.
    Example:
        If we are going to collect data for 2 rounds, 15 seconds for each round, we input:
        
        python sample_print.py 15 2
'''
parser = argparse.ArgumentParser(description="This script takes two arguments.")
parser.add_argument("data_duration_log", type=int, help="The first argument (an integer).")
parser.add_argument("loop_rounds", type=int, help="The second argument (an integer).")
py_args = parser.parse_args()


sampling_rate = 860
if not py_args.data_duration_log:
    data_log_duration =15 # secs , only integer accepted
else:
    data_log_duration = py_args.data_duration_log
if not py_args.loop_rounds:
    loop_rounds =1
else:
    loop_rounds = py_args.loop_rounds

lib = ctypes.CDLL('./sampling_reads.so')
lib.main.argtypes = (ctypes.c_int, ctypes.POINTER(ctypes.c_char_p))
lib.main.restype = ctypes.c_int


args = (ctypes.c_char_p * 3)(b"./program", str(data_log_duration).encode('utf-8'),str(loop_rounds).encode('utf-8'))
argc = len(args)


result = lib.main(argc, args)


