#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>

#define MPU6050_I2C_ADDR 0x68
#define PWR_MGMT_1 0x6B
#define GYRO_CONFIG 0x1B
#define REG_GYRO_XOUT_H 0x43
#define REG_GYRO_XOUT_L 0x44
#define REG_GYRO_YOUT_H 0x45
#define REG_GYRO_YOUT_L 0x46
#define REG_GYRO_ZOUT_H 0x47
#define REG_GYRO_ZOUT_L 0x48

void reverse (int number)
{
	printf("Reverse %d\n", number);
}

int file = -1;

int i2c_write (uint8_t reg_address, uint8_t value) {
	uint8_t buf[2];
	uint8_t write_value;
	buf[0] = reg_address;
	buf[1] = value;
	write_value = write(file,buf,2);
	if (write_value != 2) {
		printf("Error, unable to write to i2c device!\n");
		exit(1);
	}
	return 1;
}

uint8_t i2c_read(uint8_t reg_address, char *reg_value, uint8_t *i2c_gyro_write) {
	char buf[1] = {'\0'};
	uint8_t i2c_gyro_read = 0;
	buf[0] = reg_address;
	*i2c_gyro_write = write(file, buf, 1);
	i2c_gyro_read = read(file, buf, 1);
	*reg_value = buf[0];
	return i2c_gyro_read;
}

uint16_t merge_bytes( uint8_t LSB, uint8_t MSB) {
	return (uint16_t) (((LSB & 0xFF) << 8) | MSB);
}

int16_t two_complement_to_int( uint8_t LSB, uint8_t MSB) {
	int16_t signed_int = 0;
	uint16_t word;

	word = merge_bytes(LSB, MSB);

	if((word & 0x8000) == 0x8000) {
		signed_int = (int16_t) -(~word);
	} else {
		signed_int = (int16_t) (word & 0x7fff);
	}
	return signed_int;
}

int main() {
	const char * devName = "/dev/i2c-1";
	char gyro_x_h, gyro_x_l, gyro_y_h, gyro_y_l, gyro_z_h, gyro_z_l;
	gyro_x_h = 0;
	gyro_x_l = 0;
        gyro_y_h = 0;
        gyro_y_l = 0;
        gyro_z_h = 0;
        gyro_z_l = 0;
	int16_t x_gyro_int = 0;
	int16_t y_gyro_int = 0;
	int16_t z_gyro_int = 0;
	float x_gyro, y_gyro, z_gyro;

	file = open(devName, O_RDWR);
	if (file == -1) {
		perror(devName);
		exit(1);
	}

	if (ioctl(file, I2C_SLAVE, MPU6050_I2C_ADDR) < 0) {
		printf("Failed to acquire bus access and/or talk to slave!\n");
		exit(1);
	}

	if (i2c_write(PWR_MGMT_1, 0x01) != 1) {
		printf("Failed to write byte to power management!\n");
		exit(1);
	}
	if (i2c_write(GYRO_CONFIG, 0x00) != 1) {
		printf("Failed to write byte to gyro configuration!\n");
                exit(1);
	}

	while(1) {
		if (i2c_read(REG_GYRO_XOUT_H, &gyro_x_h) == -1) {
			printf("Something went wrong with read()! %s\n", strerror(errno));
			exit(1);
		}
                if (i2c_read(REG_GYRO_XOUT_L, &gyro_x_l) == -1) {
                        printf("Something went wrong with read()! %s\n", strerror(errno));
                        exit(1);
		}
                if (i2c_read(REG_GYRO_YOUT_H, &gyro_y_h) == -1) {
                        printf("Something went wrong with read()! %s\n", strerror(errno));
                        exit(1);
		}
                if (i2c_read(REG_GYRO_YOUT_L, &gyro_y_l) == -1) {
                        printf("Something went wrong with read()! %s\n", strerror(errno));
                        exit(1);
		}
                if (i2c_read(REG_GYRO_ZOUT_H, &gyro_z_h) == -1) {
                        printf("Something went wrong with read()! %s\n", strerror(errno));
                        exit(1);
		}
                if (i2c_read(REG_GYRO_ZOUT_L, &gyro_z_l) == -1) {
                        printf("Something went wrong with read()! %s\n", strerror(errno));
                        exit(1);
		}

		x_gyro_int = two_complement_to_int(gyro_x_h, gyro_x_l);
		y_gyro_int = two_complement_to_int(gyro_y_h, gyro_y_l);
		z_gyro_int = two_complement_to_int(gyro_z_h, gyro_z_l);

		x_gyro = ((float) x_gyro_int)/131;
		y_gyro = ((float) y_gyro_int)/131;
		z_gyro = ((float) z_gyro_int)/131;

		printf("x_gyro = %.3f d/s   y_gyro = %.3f d/s   z_gyro = %.3f d/s \r", x_gyro, y_gyro, z_gyro);
		usleep(10000);

	}

	return 0;
}
