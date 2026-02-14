#include "../../hal/inc/ek_hal_uart.h"
#include "../../utils/inc/ek_assert.h"
#include "../../utils/inc/ek_export.h"
#include "hal_usart.h"

#define EK_HAL_LOCK_ON(x)   ((x)->lock = true)
#define EK_HAL_LOCK_OFF(x)  ((x)->lock = false)
#define EK_HAL_LOCK_TEST(x) ((x)->lock == true)

#define UART_RX_BUFFER_SIZE 128
#define UART_TIMEOUT        (100000)  // 超时计数

// 硬件信息结构体
typedef struct
{
    uint32_t usart_periph;
} gd_uart_info;

// ops 实现
static void _init(ek_hal_uart_t *const dev);
static bool _write(ek_hal_uart_t *const dev, uint8_t *txdata, size_t size);
static bool _write_dma(ek_hal_uart_t *const dev, uint8_t *txdata, size_t size);
static void _read(ek_hal_uart_t *const dev);

static const ek_uart_ops_t gd_uart_ops = {
    .init = _init,
    .write = _write,
    .write_dma = _write_dma,
    .read = _read,
};

// 硬件信息
static gd_uart_info uart0_info = {
    .usart_periph = USART0,
};

// 接收缓冲区
static uint8_t uart0_rx_buf[UART_RX_BUFFER_SIZE];

// 设备实例
static ek_hal_uart_t drv_uart0 = {
    .baudrate = 115200,
    .buf_size = UART_RX_BUFFER_SIZE,
    .rxbuffer = uart0_rx_buf,
};

// 注册到 HAL
void gd_uart_drv_init(void)
{
    ek_hal_uart_register(&drv_uart0, "UART0", &gd_uart_ops, &uart0_info);
    // 注册后启动接收
    ek_hal_uart_read(&drv_uart0);
}

EK_EXPORT_HARDWARE(gd_uart_drv_init);

// 内部函数
static void _init(ek_hal_uart_t *const dev)
{
    ek_assert_param(dev != NULL);
    // 用户提到初始化已经实现，这里留空
    // GD32 的 USART 初始化已经在 BSP 层完成
}

static bool _write(ek_hal_uart_t *const dev, uint8_t *txdata, size_t size)
{
    ek_assert_param(dev != NULL);

    if (EK_HAL_LOCK_TEST(dev)) return false;

    gd_uart_info *info = (gd_uart_info *)dev->dev_info;

    EK_HAL_LOCK_ON(dev);

    HAL_USART_Transmite(info->usart_periph, txdata, size, UART_TIMEOUT);

    EK_HAL_LOCK_OFF(dev);
    return true;
}

static bool _write_dma(ek_hal_uart_t *const dev, uint8_t *txdata, size_t size)
{
    ek_assert_param(dev != NULL);

    if (EK_HAL_LOCK_TEST(dev)) return false;

    gd_uart_info *info = (gd_uart_info *)dev->dev_info;

    // TODO: UART DMA 发送需要配置 DMA 和传输完成中断
    // 中断处理函数示例：void USART_IRQHandler(void) { ... }
    HAL_USART_Transmite_DMA(info->usart_periph, txdata, size);

    return true;
}

static void _read(ek_hal_uart_t *const dev)
{
    ek_assert_param(dev != NULL);

    gd_uart_info *info = (gd_uart_info *)dev->dev_info;

    // TODO: UART DMA 接收需要配置 DMA 和空闲中断
    // 需要在中断处理函数中处理接收完成事件
    // 中断处理函数示例：
    // void USART_IRQHandler(void) {
    //     if (usart_interrupt_flag_get(USART0, USART_INT_FLAG_IDLE)) {
    //         usart_interrupt_flag_clear(USART0, USART_INT_FLAG_IDLE);
    //         // 处理接收到的数据
    //     }
    // }
    HAL_USART_ReceiveToIdle_DMA(info->usart_periph, dev->rxbuffer, dev->buf_size);
}
