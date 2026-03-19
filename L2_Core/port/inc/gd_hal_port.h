#ifndef GD_HAL_PORT_H
#define GD_HAL_PORT_H

#ifdef __cplusplus
extern "C"
{
#endif

void gd_gpio_drv_init(void);
void gd_uart_drv_init(void);
void gd_spi_drv_init(void);
void gd_i2c_drv_init(void);
void gd_tim_drv_init(void);
void gd_tick_drv_init(void);
void gd_dma_drv_init(void);
void gd_adc_drv_init(void);
void gd_dac_drv_init(void);
void gd_pwm_drv_init(void);

#ifdef __cplusplus
}
#endif

#endif // GD_HAL_PORT_H
