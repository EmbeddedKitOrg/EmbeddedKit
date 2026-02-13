#ifndef ST_HAL_PORT_H
#define ST_HAL_PORT_H

void st_gpio_drv_init(void);
void st_spi_drv_init(void);
void st_uart_drv_init(void);
void st_i2c_drv_init(void);
void st_tim_drv_init(void);
void st_dma2d_drv_init(void);
void st_ltdc_drv_init(void);
void st_tick_drv_init(void);

#endif // ST_HAL_PORT_H
