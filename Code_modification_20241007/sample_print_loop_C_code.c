#include <wiringPi.h>
#include <stdio.h>
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
#define PULSE_PIN 25 // WiringPi Pin 25

int fd;
// Note PCF8591 defaults to 0x48!
int asd_address = 0x48;
int16_t val;
uint8_t writeBuf[3];
uint8_t writeLo[3];
uint8_t writeHi[3];
uint8_t readBuf[2];
float myfloat;

const float VPS = 4.096 / 32768.0;
FILE *file;
struct timespec start, end, end1;
double time_taken;
double cpu_time_used;
int sample_count;
double previous;
bool conversion;


void pulseDetected() {
    if (conversion == false){
    return;
    }
    sample_count ++;
    // read conversion register
    if (read(fd, readBuf, 2) != 2) {
      perror("Read conversion");
      exit(-1);
    }

     //could also multiply by 256 then add readBuf[1]
    val = readBuf[0] << 8 | readBuf[1];

     //with +- LSB sometimes generates very low neg number.
    if (val < 0)   val = 0;

    myfloat = val * VPS; // convert to voltage

    clock_gettime(CLOCK_MONOTONIC, &end);
    time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    //cpu_time_used = ((double)(end - start))/ CLOCKS_PER_SEC;
    //printf("%d,%f,%f\n",sample_count,time_taken,myfloat);
    fprintf(file, "%d,%f,%f\n",sample_count,time_taken,myfloat);
    //printf("Time taken: %f seconds\n", time_taken);
    //printf("%.4f\n",myfloat);
    //fflush(file);
    previous = time_taken;
    //printf("%d\n",sample_count);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <time_second> <loop_count> \n", argv[0]);
        return 1;
    }
    conversion = true;
    sample_count  =  0;
    previous = 0;
    if ((fd = open("/dev/i2c-1", O_RDWR)) < 0) {
    printf("Error: Couldn't open device! %d\n", fd);
    exit (1);
    }
    int file_sn = 1;
    char filename[50];
    char current_time[20];
    time_t now = time(NULL);
    struct tm *local_time = localtime(&now);
    strftime(current_time, sizeof(current_time), "%Y%m%d_%H_%M_%S", local_time);
    sprintf(filename, "Data/%d_%dsec_%s.csv", file_sn,atoi(argv[1]),current_time);
    //printf(filename);
    file = fopen(filename, "w");
    if (file == NULL) {
        perror("Unable to open file1");
        return 1;
    }


  // connect to ADS1115 as i2c slave
    if (ioctl(fd, I2C_SLAVE, asd_address) < 0) {
    printf("Error: Couldn't find device on address!\n");
    exit (1);
    }

    writeBuf[0] = 1;
    writeBuf[1] = 0b11010010;
    writeBuf[2] = 0b11100100;
    writeLo[0]=2;
    writeLo[1]=0b00000000;
    writeLo[2]=0b00000000;
    writeHi[0]=3;
    writeHi[1]=0b10000000;
    writeHi[2]=0b00000000;


    // Initialize WiringPi
    if (wiringPiSetup() == -1) {
        printf("WiringPi setup failed!\n");
        return 1;
    }

    // Set the GPIO pin as input
    pinMode(PULSE_PIN, INPUT);


    //printf("Waiting for pulses...\n");

    if (write(fd, writeLo, 3) != 3) {
    perror("Write to register 2");
    exit (1);
    }

    if (write(fd, writeHi, 3) != 3) {
    perror("Write to register 3");
    exit (1);
    }



    if (write(fd, writeBuf, 3) != 3) {
    perror("Write to register 1");
    exit (1);
    }



    readBuf[0] = 0;
  if (write(fd, readBuf, 1) != 1) {
    perror("Write register select");
    exit(-1);
  }
    sleep(1);
    pullUpDnControl(PULSE_PIN, PUD_OFF);
    //printf("ASD1115 Demo will take five readings.\n");
    // Set up a callback to trigger on pulse detection (rising edge)
    int count =1;
    int loop_count = atoi(argv[2]);
    conversion = false;
    if (wiringPiISR(PULSE_PIN, INT_EDGE_RISING, &pulseDetected) < 0) {
        printf("ISR setup failed!\n");
        return 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &start);
    //file = fopen(filename, "w");
        //if (file == NULL) {
           //perror("Unable to open file1");
           //return 1;
            //}
    conversion = true;
    //clock_gettime(CLOCK_MONOTONIC, &end1);
    //clock_gettime(CLOCK_MONOTONIC, &start);

    // Loop to keep the program running

    while (1) {
        //delay(1000); // Sleep for a while to save CPU
        clock_gettime(CLOCK_MONOTONIC, &end1);
        if (((end1.tv_sec - start.tv_sec) + (end1.tv_nsec - start.tv_nsec) / 1e9)>= atoi(argv[1])){
            //wiringPiISR(PULSE_PIN, INT_EDGE_FALLING, NULL);
            conversion = false;
            //sleep(3);
            printf("\n");
            loop_count --;
            if (loop_count>0){
            file_sn ++;
            printf(filename);
            printf("\n");
            printf("%d\n",sample_count);
            sample_count = 0;
            clock_gettime(CLOCK_MONOTONIC, &start);
            fclose(file);
            now = time(NULL);
            struct tm *local_time = localtime(&now);
            strftime(current_time, sizeof(current_time), "%Y%m%d_%H_%M_%S", local_time);
            sprintf(filename, "Data/%d_%dsec_%s.csv", file_sn,atoi(argv[1]),current_time);

            file = fopen(filename, "w");
            if (file == NULL) {
                perror("Unable to open file2");
                return 1;
            }
            conversion = true;
            }else{
             //sample_count = 0;
             break;}
        }
        //count ++;
    }

    printf(filename);
    printf("\n");
    printf("%d\n",sample_count);
    writeBuf[0] = 1;    // config register is 1
    writeBuf[1] = 0b11000011; // bit 15-8 0xC3 single shot on
    writeBuf[2] = 0b10000111; // bits 7-0  0x85
    if (write(fd, writeBuf, 3) != 3) {
        perror("Write to register 1");
        exit (1);
    }
    fclose(file);
    close(fd);
    //printf("%d\n",sample_count);
    //wiringPiISR(PULSE_PIN, INT_EDGE_RISING, NULL);
    return 0;
}






sample_count = 0;
    int window_count =0;
    printf("Collection Ends, Generating Files.\n");
    printf(filenames[0]);
    file = fopen(filenames[window_count], "w");
                if (file == NULL) {
                    perror("Unable to open file2");
                    return 1;
                }
for (int i = 0; i < reading_count; i+=4) {
        sample_count ++;
        //printf("go");
        if (i==0){
            fprintf(file,"%d,%f,%f,%f,%f\n",(i/4)+1, accel_readings[i],accel_readings[i+1],accel_readings[i+2],accel_readings[i+3]);
        }
        if (accel_readings[i]-accel_readings[i-4] > 0 ){
            fprintf(file,"%d,%f,%f,%f,%f\n",(i/4)+1, accel_readings[i],accel_readings[i+1],accel_readings[i+2],accel_readings[i+3]);
        }else{
            //fclose(file);
            window_count ++;

            printf("\n");
            printf(filenames[window_count]);
            printf("    %d\n",sample_count);
            //file_sn ++;
            if (window_count+1<atoi(argv[2])){
                file = fopen(filenames[window_count], "w");
                if (file == NULL) {
                    perror("Unable to open file2");
                    return 1;
                }
                fprintf(file,"%d,%f,%f,%f,%f\n",(i/4)+1, accel_readings[i],accel_readings[i+1],accel_readings[i+2],accel_readings[i+3]);
            }




        }
        //printf("%d,%f,%f,%f,%f\n",(i/4)+1, accel_readings[i],accel_readings[i+1],accel_readings[i+2],accel_readings[i+3]);
    }
