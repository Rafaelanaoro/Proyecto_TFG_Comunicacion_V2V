#include "vl53l0x_platform.h"
#include "stm32l4xx_hal.h"

#define VL53_TIMEOUT 100

VL53L0X_Error VL53L0X_WriteMulti(VL53L0X_DEV Dev, uint8_t index,
                                uint8_t *pdata, uint32_t count)
{
    return (HAL_I2C_Mem_Write(
        Dev->I2cHandle,
        Dev->I2cDevAddr,
        index,
        I2C_MEMADD_SIZE_8BIT,
        pdata,
        count,
        VL53_TIMEOUT
    ) == HAL_OK) ? VL53L0X_ERROR_NONE : VL53L0X_ERROR_CONTROL_INTERFACE;
}

VL53L0X_Error VL53L0X_ReadMulti(VL53L0X_DEV Dev, uint8_t index,
                               uint8_t *pdata, uint32_t count)
{
    return (HAL_I2C_Mem_Read(
        Dev->I2cHandle,
        Dev->I2cDevAddr,
        index,
        I2C_MEMADD_SIZE_8BIT,
        pdata,
        count,
        VL53_TIMEOUT
    ) == HAL_OK) ? VL53L0X_ERROR_NONE : VL53L0X_ERROR_CONTROL_INTERFACE;
}

VL53L0X_Error VL53L0X_WrByte(VL53L0X_DEV Dev, uint8_t index, uint8_t data)
{
    return VL53L0X_WriteMulti(Dev, index, &data, 1);
}

VL53L0X_Error VL53L0X_RdByte(VL53L0X_DEV Dev, uint8_t index, uint8_t *data)
{
    return VL53L0X_ReadMulti(Dev, index, data, 1);
}

VL53L0X_Error VL53L0X_PollingDelay(VL53L0X_DEV Dev)
{
    HAL_Delay(1);
    return VL53L0X_ERROR_NONE;
}

VL53L0X_Error VL53L0X_WrWord(VL53L0X_DEV Dev, uint8_t index, uint16_t data)
{
    uint8_t buf[2];
    buf[0] = (data >> 8) & 0xFF;
    buf[1] = data & 0xFF;
    return VL53L0X_WriteMulti(Dev, index, buf, 2);
}

VL53L0X_Error VL53L0X_RdWord(VL53L0X_DEV Dev, uint8_t index, uint16_t *data)
{
    uint8_t buf[2];
    VL53L0X_Error err = VL53L0X_ReadMulti(Dev, index, buf, 2);
    if (err == VL53L0X_ERROR_NONE)
        *data = ((uint16_t)buf[0] << 8) | buf[1];
    return err;
}

VL53L0X_Error VL53L0X_RdDWord(VL53L0X_DEV Dev, uint8_t index, uint32_t *data)
{
    uint8_t buf[4];
    VL53L0X_Error err = VL53L0X_ReadMulti(Dev, index, buf, 4);
    if (err == VL53L0X_ERROR_NONE)
        *data = ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) | ((uint32_t)buf[2] << 8) | buf[3];
    return err;
}

VL53L0X_Error VL53L0X_UpdateByte(VL53L0X_DEV Dev, uint8_t index, uint8_t AndData, uint8_t OrData)
{
    uint8_t data;
    VL53L0X_Error err = VL53L0X_RdByte(Dev, index, &data);
    if (err != VL53L0X_ERROR_NONE) return err;
    data = (data & AndData) | OrData;
    return VL53L0X_WrByte(Dev, index, data);
}

