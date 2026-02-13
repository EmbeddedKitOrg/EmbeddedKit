#include "../../hal/inc/ek_hal_uart.h"
#include "../../utils/inc/ek_assert.h"
#include "../../utils/inc/ek_export.h"
#include "usart.h"

#define EK_HAL_LOCK_ON(x)   ((x)->lock = true)
#define EK_HAL_LOCK_OFF(x)  ((x)->lock = false)
#define EK_HAL_LOCK_TEST(x) ((x)->lock == true)

#define UART_RX_BUFFER_SIZE 128
#define UART_TIMEOUT        (100)

// 硬件信息结构体
typedef struct
{
    UART_HandleTypeDef *huart;
} st_uart_info;

// ops 实现
static void _init(ek_hal_uart_t *const dev);
static bool _write(ek_hal_uart_t *const dev, uint8_t *txdata, size_t size);
static bool _write_dma(ek_hal_uart_t *const dev, uint8_t *txdata, size_t size);
static void _read(ek_hal_uart_t *const dev);

static const ek_uart_ops_t st_uart_ops = {
    .init = _init,
    .write = _write,
    .write_dma = _write_dma,
    .read = _read,
};

// 硬件信息
static st_uart_info uart1_info = {
    .huart = &huart1,
};

// 接收缓冲区
static uint8_t uart1_rx_buf[UART_RX_BUFFER_SIZE];

// 设备实例
static ek_hal_uart_t drv_uart1 = {
    .baudrate = 115200,
    .buf_size = UART_RX_BUFFER_SIZE,
    .rxbuffer = uart1_rx_buf,
};

// 注册到 HAL
void st_uart_drv_init(void)
{
    ek_hal_uart_register(&drv_uart1, "UART1", &st_uart_ops, &uart1_info);
    // 注册后启动接收
    ek_hal_uart_read(&drv_uart1);
}

EK_EXPORT_HARDWARE(st_uart_drv_init);

// 内部函数
static void _init(ek_hal_uart_t *const dev)
{
    ek_assert_param(dev != NULL);
    // CubeMX 已完成 UART 硬件初始化
}

static bool _write(ek_hal_uart_t *const dev, uint8_t *txdata, size_t size)
{
    ek_assert_param(dev != NULL);

    if (EK_HAL_LOCK_TEST(dev)) return false;

    st_uart_info *info = (st_uart_info *)dev->dev_info;

    EK_HAL_LOCK_ON(dev);
    HAL_StatusTypeDef ret = HAL_UART_Transmit(info->huart, txdata, size, UART_TIMEOUT);
    EK_HAL_LOCK_OFF(dev);

    return (ret == HAL_OK);
}

static bool _write_dma(ek_hal_uart_t *const dev, uint8_t *txdata, size_t size)
{
    ek_assert_param(dev != NULL);

    if (EK_HAL_LOCK_TEST(dev)) return false;

    st_uart_info *info = (st_uart_info *)dev->dev_info;

    EK_HAL_LOCK_ON(dev);
    HAL_StatusTypeDef ret = HAL_UART_Transmit_DMA(info->huart, txdata, size);
    EK_HAL_LOCK_OFF(dev);

    return (ret == HAL_OK);
}

static void _read(ek_hal_uart_t *const dev)
{
    ek_assert_param(dev != NULL);

    st_uart_info *info = (st_uart_info *)dev->dev_info;

    HAL_UARTEx_ReceiveToIdle_DMA(info->huart, dev->rxbuffer, dev->buf_size);
}

// UART 接收完成回调
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1)
    {
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart1_rx_buf, UART_RX_BUFFER_SIZE);
    }
}
