#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>    // read/write usleep
#include <stdlib.h>    // exit function
#include <inttypes.h>  // uint8_t, etc
#include <linux/i2c-dev.h> // I2C bus definitions
#include <sys/ioctl.h>
#include <time.h>
#include <stdbool.h>
// Define the GPIO pin number (WiringPi pin numbering)
#define PULSE_PIN 23 // WiringPi Pin 25
// ISM330DHCX I2C address
#define ISM330DHCX_ADDR 0x6A

// Register addresses
#define WHO_AM_I_REG   0x0F
#define CTRL1_XL_REG   0x10
#define CTRL2_G_REG    0x11
#define CTRL3_C_REG    0x12
#define INT1_CTRL_REG  0x0D
#define CTRL9_XL_REG   0x18
#define CTRL10_C_REG   0x19
#define OUTX_L_A 0x28
#define OUTX_H_A 0x29
#define OUTY_L_A 0x2A
#define OUTY_H_A 0x2B
#define OUTZ_L_A 0x2C
#define OUTZ_H_A 0x2D
#define COUNTER_BDR_REG1 0x0B
#define FIFO_CTRL4 0x0A
float accxfloat;
float accyfloat;
float acczfloat;

const float VPS = 0.000061;
const int HALF_RANGE = 32768;
FILE *file;
struct timespec start, end, end1;
double time_taken;
double cpu_time_used;
int sample_count;
double previous;
bool conversion;
int acc_x_low;
int acc_x_high;
int acc_y_low;
int acc_y_high;
int acc_z_low;
int acc_z_high;
int acc_x;
int acc_y;
int acc_z;
int fd;
float *accel_readings = NULL;  // Dynamically allocated array to store readings
int reading_count = 0;  // Counter to keep track of the number of readings
int allocated_size = 0;  // Track how much memory is allocated
//void allocateMemoryForReadings(void);

void allocateMemoryForReadings() {
    // Allocate an initial size or double the size if more space is needed
    if (accel_readings == NULL) {
        allocated_size = 100;  // Start with space for 100 readings
        accel_readings = (float *)malloc(allocated_size * sizeof(float));
    } else if (reading_count+4 >= allocated_size) {
        allocated_size *= 2;  // Double the allocated size
        accel_readings = (float *)realloc(accel_readings, allocated_size * sizeof(float));
    }

    if (accel_readings == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }
}


void pulseDetected() {
    if (conversion == false){
    return;
    }
    sample_count ++;

    // Read accelerometer X axis data
    acc_x_low = wiringPiI2CReadReg8(fd, OUTX_L_A);
    acc_x_high = wiringPiI2CReadReg8(fd, OUTX_H_A);
    acc_x = (acc_x_high << 8) | acc_x_low;

        // Read accelerometer Y axis data
    acc_y_low = wiringPiI2CReadReg8(fd, OUTY_L_A);
    acc_y_high = wiringPiI2CReadReg8(fd, OUTY_H_A);
    acc_y = (acc_y_high << 8) | acc_y_low;

        // Read accelerometer Z axis data
    acc_z_low = wiringPiI2CReadReg8(fd, OUTZ_L_A);
    acc_z_high = wiringPiI2CReadReg8(fd, OUTZ_H_A);
    acc_z = (acc_z_high << 8) | acc_z_low;

     //with +- LSB sometimes generates very low neg number.
    //if ( < 0)   val = 0;

    accxfloat = (acc_x-HALF_RANGE) * VPS; // convert to g
    accyfloat = (acc_y-HALF_RANGE) * VPS;
    acczfloat = (acc_z-HALF_RANGE) * VPS;
    clock_gettime(CLOCK_MONOTONIC, &end);
    time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    allocateMemoryForReadings();
    accel_readings[reading_count] = time_taken;
    accel_readings[reading_count+1] = accxfloat;
    accel_readings[reading_count+2] = accyfloat;
    accel_readings[reading_count+3] = acczfloat;
    //printf("%d,%f,%f,%f,%f\n",reading_count,accel_readings[reading_count],accel_readings[reading_count+1],accel_readings[reading_count+2],accel_readings[reading_count+3]);
    reading_count += 4;
    //cpu_time_used = ((double)(end - start))/ CLOCKS_PER_SEC;
    //printf("%d,%f,%f,%f,%f\n",sample_count,time_taken,accxfloat,accyfloat,acczfloat);
    //printf("%d\n",sample_count);
    //fprintf(file, "%d,%f,%.4f,%f\n", sample_count, time_taken, (double)myfloat,time_taken - previous);
    //printf("Time taken: %f seconds\n", time_taken);
    //printf("%.4f\n",myfloat);
    //fflush(file);
    //previous = time_taken;
    //printf("%d\n",sample_count);
}



int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s <time_second> <file_number>  <activity>\n", argv[0]);
        return 1;
    }
    conversion = true;
    sample_count  =  0;
    previous = 0;
    if (wiringPiSetup() == -1) {
        printf("WiringPi setup failed!\n");
        return 1;
    }
    int CS_pin = 24;
    pinMode(CS_pin, OUTPUT);
    digitalWrite(CS_pin, HIGH);
    // Setup I2C communication
    fd = wiringPiI2CSetup(ISM330DHCX_ADDR);
    if (fd == -1) {
        printf("Failed to initiate I2C communication.\n");
        return -1;
    }
    printf("I2C communication successfully setup.\n");

    // Check device identity
    int who_am_i = wiringPiI2CReadReg8(fd, WHO_AM_I_REG);
    if (who_am_i == 0x6C || who_am_i == 0x6B) {  // Accept both 0x6C and 0x6B
        printf("ISM330DHCX detected! WHO_AM_I register: 0x%X\n", who_am_i);
    } else {
        printf("Device not detected. WHO_AM_I register: 0x%X\n", who_am_i);
        return -1;
    }

    // Configure accelerometer (CTRL1_XL) - 833Hz, 2g
    wiringPiI2CWriteReg8(fd, CTRL1_XL_REG, 0b01110000);  //
    int ctrl1_xl_val = wiringPiI2CReadReg8(fd, CTRL1_XL_REG);
    printf("CTRL1_XL register: 0x%X\n", ctrl1_xl_val);  // Should print 0xA0
    // Configure gyroscope (CTRL2_G) - 1.66 kHz, 2000 dps
    //wiringPiI2CWriteReg8(fd, CTRL2_G_REG, 0x80);   // 1.66 kHz, 2000 dps
// Enable data-ready interrupt on INT1
    wiringPiI2CWriteReg8(fd, FIFO_CTRL4, 0b00000110);
    wiringPiI2CWriteReg8(fd, COUNTER_BDR_REG1, 0b10000000);
    wiringPiI2CWriteReg8(fd, INT1_CTRL_REG, 0b00000001);  // INT1_DRDY_XL and INT1_DRDY_G enabled

    // Set normal mode, block data update enabled
    wiringPiI2CWriteReg8(fd, CTRL3_C_REG, 0b01000100);  // BDU enabled, Normal mode

    // Enable accelerometer axes
    //wiringPiI2CWriteReg8(fd, CTRL9_XL_REG, 0b00111000);  // Enable X, Y, Z accelerometer axes

    // Enable gyroscope axes
    //wiringPiI2CWriteReg8(fd, CTRL10_C_REG, 0b00111000);  // Enable X, Y, Z gyroscope axes

    printf("Sensor configuration completed.\n");
    //int rep = atoi(argv[2]);
    char filenames[atoi(argv[2])][50];
    int file_sn = 1;
    char filename[50];
    char current_time[20];



  // connect to ADS1115 as i2c slave


    // Set the GPIO pin as input
    pinMode(PULSE_PIN, INPUT);


    //printf("Waiting for pulses...\n");

    pullUpDnControl(PULSE_PIN, PUD_OFF);
    //printf("ASD1115 Demo will take five readings.\n");
    // Set up a callback to trigger on pulse detection (rising edge)
    if (wiringPiISR(PULSE_PIN, INT_EDGE_RISING, &pulseDetected) < 0) {
        printf("ISR setup failed!\n");
        return 1;
    }
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    strftime(current_time, sizeof(current_time), "%Y%m%d_%H_%M_%S", local_time);
    sprintf(filenames[0], "Data/%d_%dsec_%s_%s.csv", file_sn,atoi(argv[1]),current_time,argv[3]);
    printf(filenames[0]);
    printf("\n");
    //strcpy((char *)filenames[0], filename);
    //printf(filenames[0]);


    conversion = true;
    clock_gettime(CLOCK_MONOTONIC, &start);
    int count =1;
    int loop_count = atoi(argv[2]);
    //int loop_count = 1;
    // Loop to keep the program running
    while (1) {
        //sample_count ++;
        //acc_x_low = wiringPiI2CReadReg8(fd, OUTX_L_A);
        //acc_x_high = wiringPiI2CReadReg8(fd, OUTX_H_A);
        //acc_x = (acc_x_high << 8) | acc_x_low;

        // Read accelerometer Y axis data
        //acc_y_low = wiringPiI2CReadReg8(fd, OUTY_L_A);
        //acc_y_high = wiringPiI2CReadReg8(fd, OUTY_H_A);
        //acc_y = (acc_y_high << 8) | acc_y_low;

        // Read accelerometer Z axis data
        //acc_z_low = wiringPiI2CReadReg8(fd, OUTZ_L_A);
        //acc_z_high = wiringPiI2CReadReg8(fd, OUTZ_H_A);
        //acc_z = (acc_z_high << 8) | acc_z_low;

     //with +- LSB sometimes generates very low neg number.
    //if ( < 0)   val = 0;

        //accxfloat = (acc_x-HALF_RANGE) * VPS; // convert to g
        //accyfloat = (acc_y-HALF_RANGE) * VPS;
        //acczfloat = (acc_z-HALF_RANGE) * VPS;
        delay(1000); // Sleep for a while to save CPU
        clock_gettime(CLOCK_MONOTONIC, &end1);
        if (((end1.tv_sec - start.tv_sec) + (end1.tv_nsec - start.tv_nsec) / 1e9)>= atoi(argv[1])){
            //wiringPiISR(PULSE_PIN, INT_EDGE_FALLING, NULL);
            conversion = false;
            printf("\n");
            printf(filename);
            printf("\n");
            printf("%d\n",sample_count);
            loop_count --;
            if (loop_count>0){
            file_sn ++;
            sample_count = 0;
            clock_gettime(CLOCK_MONOTONIC, &start);
            //fclose(file);
            now = time(NULL);
            struct tm *local_time = localtime(&now);
            strftime(current_time, sizeof(current_time), "%Y%m%d_%H_%M_%S", local_time);
            sprintf(filenames[atoi(argv[2])-loop_count],  "Data/%d_%dsec_%s_%s.csv", file_sn,atoi(argv[1]),current_time,argv[3]);
            printf(filenames[atoi(argv[2])-loop_count]);
            conversion = true;
            }else{
             //sample_count = 0;
             break;}
        }
        //count ++;
    }
    sample_count = 0;
    printf("Collection Ends, Generating Files.\n");
    //printf("gogogo");
    int window_count = 0;
    file = fopen(filenames[window_count], "w");
    if (file == NULL) {
        perror("Unable to open file2");
        return 1;
    }

    for (int i = 0; i < reading_count; i += 4) {
        sample_count++;

        // Only access accel_readings[i-4] if i >= 4
        if (i == 0 || (i >= 4 && accel_readings[i] - accel_readings[i - 4] > 0)) {
            fprintf(file, "%d,%f,%f,%f,%f\n", sample_count, accel_readings[i], accel_readings[i + 1], accel_readings[i + 2], accel_readings[i + 3]);
        } else {
            // Close the current file and open the next file if available
            fclose(file);
            window_count++;
            sample_count = 0;
            if (window_count < atoi(argv[2])) {
                file = fopen(filenames[window_count], "w");
                if (file == NULL) {
                    perror("Unable to open file");
                    free(accel_readings);
                    return 1;
                }
            }
        }
    }



    //printf("\n");
    //printf(filenames[atoi(argv[2])-1]);
    //printf("    %d\n",sample_count);

    //fclose(file);
    wiringPiI2CWriteReg8(fd, FIFO_CTRL4, 0b00000000);
    wiringPiI2CWriteReg8(fd, COUNTER_BDR_REG1, 0b00000000);
    free(accel_readings);
    //free(filenames);
    //printf("%d\n",sample_count);
    //wiringPiISR(PULSE_PIN, INT_EDGE_RISING, NULL);
    close(fd);
    return 0;
}

