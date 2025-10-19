#include "Common.h"

extern EK_CoroSemHanlder_t SerialSemDMA_OK;

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == USER_HUART)
    {
        bool need_yield = false;
        EK_bSemBinaryGive_FromISR(SerialSemDMA_OK, &need_yield);
        EK_vKernelYield_From_ISR(need_yield);
    }
}
