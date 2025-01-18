#include "mpu6050.h"
#include "i2c.h"
#include <stdint.h>

static float accelDivider; //AFS_SEL
static float gyroDivider; //FS_SEL

/*
 *	use Internal 8MHz clock
 *	250 Full Scale Range for Gyro
 *	2g Full Scale Range for Accel
 *	1000kHz
 */
void config_MPU6050_DEFAULT(MPU6050_config *config) {
	config->clockSource = 0x00;
	config->accelScaleRange = 0x00;
	config->gyroScaleRange = 0x00;
	config->sampleRate = 0x07;
}

uint8_t init_MPU6050(I2C_HandleTypeDef *I2Cx, MPU6050_config *config){
	uint8_t check;
	uint8_t data;

	switch(config->accelScaleRange) {
	case 0:
		accelDivider = 16384;
		break;
	case 1:
		accelDivider = 8192;
		break;
	case 2:
		accelDivider = 4096;
		break;
	case 3:
		accelDivider = 2048;
		break;
	default :
		return ACCEL_CONFIG_ERR;
	}

	switch(config->gyroScaleRange) {
	case 0:
		gyroDivider = 131;
		break;
	case 1:
		gyroDivider = 65.5;
		break;
	case 2:
		gyroDivider = 32.8;
		break;
	case 3:
		gyroDivider = 16.4;
		break;
	default :
		return GYRO_CONFIG_ERR;
	}


	//check WHO_AM_I reg
	HAL_I2C_Mem_Read(I2Cx, MPU6050_ADDR, WHO_AM_I_REG, 1, &check, 1, 10);
	if (check != 104)
		return WHO_AM_I_ERR;

	//set PWR_MGMT reg
	data = config->clockSource;
	HAL_I2C_Mem_Write(I2Cx, MPU6050_ADDR, PWR_MGMT_1_REG, 1, &data, 1, 10);
	HAL_I2C_Mem_Read(I2Cx, MPU6050_ADDR, PWR_MGMT_1_REG, 1, &check, 1, 10);
	if (check != data)
		return PWR_MGMT_1_ERR;

	//set SMPRT_DIV_REG reg
	data = config->sampleRate;
	HAL_I2C_Mem_Write(I2Cx, MPU6050_ADDR, SMPRT_DIV_REG, 1, &data, 1, 10);
	HAL_I2C_Mem_Read(I2Cx, MPU6050_ADDR, SMPRT_DIV_REG, 1, &check, 1, 10);
	if (check != data)
		return SMPRT_DIV_ERR;

	//set ACCEL_CONFIG reg
	data = config->accelScaleRange;
	HAL_I2C_Mem_Write(I2Cx, MPU6050_ADDR, GYRO_CONFIG_REG, 1, &data, 1, 10);
	HAL_I2C_Mem_Read(I2Cx, MPU6050_ADDR, GYRO_CONFIG_REG, 1, &check, 1, 10);
	if (check != data)
		return GYRO_CONFIG_ERR;

	//set ACCEL_CONFIG reg
	data = config->gyroScaleRange;
	HAL_I2C_Mem_Write(I2Cx, MPU6050_ADDR, ACCEL_CONFIG_REG, 1, &data, 1, 10);
	HAL_I2C_Mem_Read(I2Cx, MPU6050_ADDR, ACCEL_CONFIG_REG, 1, &check, 1, 10);
	if (check != data)
		return ACCEL_CONFIG_ERR;

	//return 0 if everything goes well
	return MPU6050_OK;
}

void readAccel_MPU6050(I2C_HandleTypeDef *I2Cx, MPU6050* data) {

	uint8_t RAW_data[6];

    HAL_I2C_Mem_Read(I2Cx, MPU6050_ADDR, ACCEL_XOUT_H_REG, 1, RAW_data, 6, 10);

    data->Accel_X_RAW = (int16_t)(RAW_data[0] << 8 | RAW_data[1]);
    data->Accel_Y_RAW = (int16_t)(RAW_data[2] << 8 | RAW_data[3]);
    data->Accel_Z_RAW = (int16_t)(RAW_data[4] << 8 | RAW_data[5]);

    data->Ax = data->Accel_X_RAW / accelDivider;
    data->Ay = data->Accel_Y_RAW / accelDivider;
    data->Az = data->Accel_Z_RAW / accelDivider;

}
void readGyro_MPU6050(I2C_HandleTypeDef *I2Cx, MPU6050* data) {
	uint8_t RAW_data[6];

	HAL_I2C_Mem_Read(I2Cx, MPU6050_ADDR, GYRO_XOUT_H_REG, 1, RAW_data, 6, 10);

	data->Gyro_X_RAW = (int16_t)(RAW_data[0] << 8 | RAW_data[1]);
	data->Gyro_Y_RAW = (int16_t)(RAW_data[2] << 8 | RAW_data[3]);
	data->Gyro_Z_RAW = (int16_t)(RAW_data[4] << 8 | RAW_data[5]);

    data->Gx = data->Gyro_X_RAW / gyroDivider;
    data->Gy = data->Gyro_Y_RAW / gyroDivider;
    data->Gz = data->Gyro_Z_RAW / gyroDivider;
}


