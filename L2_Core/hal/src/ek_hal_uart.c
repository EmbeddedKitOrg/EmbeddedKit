#include "../inc/ek_hal_uart.h"
#include "../../utils/inc/ek_mem.h"
#include "../../utils/inc/ek_assert.h"
#include "../../utils/inc/ek_export.h"


#define EK_HAL_LOCK_ON(x)   ((x)->lock = true)
#define EK_HAL_LOCK_OFF(x)  ((x)->lock = false)
#define EK_HAL_LOCK_TEST(x) ((x)->lock == true)

#define UART_RX_BUFFER_SIZE 128

ek_list_node_t ek_hal_uart_head;

static uint8_t uart1_rx_buffer[UART_RX_BUFFER_SIZE]; // 建议通过串口 + 空闲终端 + DMA将数据搬运到这个数组
static void uart1_uart_init(void);
static bool uart1_uart_send(uint8_t *TxData, size_t Size);
static void uart1_uart_recieve_to_idle(void);

ek_hal_uart_t hal_drv_uart1 = {.idx = 1,
                               .baudrate = 115200,
                               .buf_size = UART_RX_BUFFER_SIZE,
                               .rxbuffer = uart1_rx_buffer,

                               .init = uart1_uart_init,
                               .write = uart1_uart_send,
                               .read = uart1_uart_recieve_to_idle};

static void uart1_uart_init(void)
{
    // uart外设初始化的底层
    // e.g. HAL_UART_Init(&huart1)

    hal_drv_uart1.lock = false;

    ek_list_add_tail(&ek_hal_uart_head, &hal_drv_uart1.node);
}

static bool uart1_uart_send(uint8_t *txdata, size_t size)
{
    if (EK_HAL_LOCK_TEST(&hal_drv_uart1)) return false;

    EK_HAL_LOCK_ON(&hal_drv_uart1);

    // uart发送数据的底层
    // e.g. HAL_UART_Transmit(&huart1, txdata, size, timeout)

    EK_HAL_LOCK_OFF(&hal_drv_uart1);

    return true;
}

static void uart1_uart_recieve_to_idle(void)
{
    // uart接收数据到空闲中断的底层
    // e.g. HAL_UARTEx_ReceiveToIdle(&huart1, rxdata, size, rx_data_len, timeout)
}

void ek_hal_uart_init(void)
{
    ek_list_node_t *p;
    ek_list_init(&ek_hal_uart_head);

    uart1_uart_init();

    ek_list_iterate(p, &ek_hal_uart_head)
    {
        ek_hal_uart_t *uart = (ek_hal_uart_t *)ek_list_container(p, ek_hal_uart_t, node);
        uart->read();
    }
}

EK_EXPORT(ek_hal_uart_init, 0);