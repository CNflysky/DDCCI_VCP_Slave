#include "ddcci_vcp_slave.h"
// add this library to your project: https://github.com/dgatf/I2C-slave-multi-address-RP2040
#include "i2c_multi.h"

static PIO pio = pio0;
uint8_t pio_i2c_sda = 18;

void i2c_receive_handler(uint8_t data, bool is_address) {
  if (!is_address) ddcci_write_byte(data);
}

void i2c_stop_handler(uint8_t length) { ddcci_process_request(); }
int main() {
  stdio_init_all();
  i2c_multi_init(pio, pio_i2c_sda);
  i2c_multi_enable_address(DDCCI_I2C_ADDRESS);
  i2c_multi_set_receive_handler(i2c_receive_handler);
  i2c_multi_set_stop_handler(i2c_stop_handler);
  i2c_multi_set_write_buffer(ddcci_get_xfer_buf());

  while (1) {
    printf("brightness = %d\n", ddcci_get_brightness());
    sleep_ms(1000);
  }
}
