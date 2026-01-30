#include "../inc/ek_hal_spi.h"
#include "../../utils/inc/ek_assert.h"
#include "../../utils/inc/ek_export.h"

#define EK_HAL_LOCK_ON(x)   ((x)->lock = true)
#define EK_HAL_LOCK_OFF(x)  ((x)->lock = false)
#define EK_HAL_LOCK_TEST(x) ((x)->lock == true)

#define STM32F429XX_TIMEOUT (100)

ek_list_node_t ek_hal_spi_head;

static void spi1_init(void);
static bool spi1_send(uint8_t *txdata, size_t size);
static bool spi1_recieve(uint8_t *rxdata, size_t size);
static bool spi1_write_recieve(uint8_t *txdata, uint8_t *rxdata, size_t size);

ek_hal_spi_t hal_drv_spi1 = {
    .idx = 1,
    .lock = false,

    .init = spi1_init,
    .read = spi1_recieve,
    .write = spi1_send,
    .write_read = spi1_write_recieve,
};

static void spi1_init(void)
{
    hal_drv_spi1.lock = false;

    ek_list_add_tail(&ek_hal_spi_head, &hal_drv_spi1.node);
}

static bool spi1_send(uint8_t *txdata, size_t size)
{
    if (EK_HAL_LOCK_TEST(&hal_drv_spi1)) return false;

    EK_HAL_LOCK_ON(&hal_drv_spi1);

    __UNUSED(txdata);
    __UNUSED(size);

    EK_HAL_LOCK_OFF(&hal_drv_spi1);

    return true;
}

static bool spi1_recieve(uint8_t *rxdata, size_t size)
{
    if (EK_HAL_LOCK_TEST(&hal_drv_spi1)) return false;

    EK_HAL_LOCK_ON(&hal_drv_spi1);

    __UNUSED(rxdata);
    __UNUSED(size);
    // 具体的spi底层

    EK_HAL_LOCK_OFF(&hal_drv_spi1);

    return true;
}

static bool spi1_write_recieve(uint8_t *txdata, uint8_t *rxdata, size_t size)
{
    if (EK_HAL_LOCK_TEST(&hal_drv_spi1)) return false;

    EK_HAL_LOCK_ON(&hal_drv_spi1);

    __UNUSED(txdata);
    __UNUSED(rxdata);
    __UNUSED(size);
    // 具体的spi底层

    EK_HAL_LOCK_OFF(&hal_drv_spi1);

    return true;
}

void ek_hal_spi_init(void)
{
    ek_list_node_t *p;
    ek_list_init(&ek_hal_spi_head);

    spi1_init();

    ek_list_iterate(p, &ek_hal_spi_head)
    {
        ek_hal_spi_t *spi = (ek_hal_spi_t *)ek_list_container(p, ek_hal_spi_t, node);
    }
}

EK_EXPORT(ek_hal_spi_init, 0);
