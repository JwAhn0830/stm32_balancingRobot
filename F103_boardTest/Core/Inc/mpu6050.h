
#include <stdio.h>
#include "i2c.h"
#include <stdint.h>

//define reg addr
#define MPU6050_ADDR 0xD0 // 0x68 << 1
#define WHO_AM_I_REG 0x75
#define PWR_MGMT_1_REG 0x6B
#define SMPRT_DIV_REG 0x19
#define GYRO_CONFIG_REG 0x1B
#define ACCEL_CONFIG_REG 0x1C

#define ACCEL_XOUT_H_REG 0x3B
#define GYRO_XOUT_H_REG 0x43


// err state code
typedef enum ERR {
	MPU6050_OK,
	WHO_AM_I_ERR,
	PWR_MGMT_1_ERR,
	SMPRT_DIV_ERR,
	GYRO_CONFIG_ERR,
	ACCEL_CONFIG_ERR
}ERR;

typedef struct {
    int16_t Accel_X_RAW;
    int16_t Accel_Y_RAW;
    int16_t Accel_Z_RAW;
    double Ax;
    double Ay;
    double Az;

    int16_t Gyro_X_RAW;
    int16_t Gyro_Y_RAW;
    int16_t Gyro_Z_RAW;
    double Gx;
    double Gy;
    double Gz;
}MPU6050;

typedef struct {
	uint8_t clockSource; // PWR_MGMT_1_REG
	uint8_t sampleRate; //  SMPRT_DIV_REG
	uint8_t gyroScaleRange; // GYRO_CONFIG_REG
	uint8_t accelScaleRange; // ACCEL_CONFIG_REG
}MPU6050_config;

void config_MPU6050_DEFAULT(MPU6050_config *config);
uint8_t init_MPU6050(I2C_HandleTypeDef *I2Cx, MPU6050_config *config);
void readAccel_MPU6050(I2C_HandleTypeDef *I2Cx, MPU6050* data);
void readGyro_MPU6050(I2C_HandleTypeDef *I2Cx, MPU6050* data);
