#include "ddcci_vcp_slave.h"
I2C_HandleTypeDef hi2c = {0};

inline static void ddcci_push_byte(I2C_HandleTypeDef* hi2c, uint32_t xfer_opt) {
  uint8_t byte = 0x00;
  HAL_I2C_Slave_Seq_Receive_IT(hi2c, &byte, 1, xfer_opt);
  ddcci_write_byte(byte);
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef* hi2c, uint8_t TransferDirection,
                          uint16_t AddrMatchCode) {
  if (hi2c->Instance == I2C1) {
    if (TransferDirection == I2C_DIRECTION_TRANSMIT)
      HAL_I2C_Slave_Seq_Receive_IT(hi2c,
                                   ddcci_get_xfer_buf() + *ddcci_get_xfer_len(),
                                   1, I2C_FIRST_FRAME);
    else
      HAL_I2C_Slave_Seq_Transmit_IT(hi2c, ddcci_get_xfer_buf(),
                                    DDCCI_XFER_BUF_SIZE,
                                    I2C_FIRST_AND_LAST_FRAME);
  }
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef* hi2c) {
  HAL_I2C_EnableListen_IT(hi2c);
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef* hi2c) {
  (*ddcci_get_xfer_len())++;
  HAL_I2C_Slave_Seq_Receive_IT(
      hi2c, ddcci_get_xfer_buf() + *ddcci_get_xfer_len(), 1, I2C_NEXT_FRAME);
}

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef* hi2c) {
  ddcci_process_request();
  HAL_I2C_EnableListen_IT(hi2c);
}

int main() {
  HAL_Init();
  MX_GPIO_Init();
  MX_I2C_Init();
  HAL_I2C_EnableListen_IT(&hi2c);
  while (1) {
    printf("brightness = %d\n", ddcci_get_brightness());
    HAL_Delay(1000);
  }
}