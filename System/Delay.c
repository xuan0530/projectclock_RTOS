#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"

// 私有变量
static uint8_t delay_init_flag = 0;

/**
  * @brief  延时初始化函数（使用TIM2定时器）
  * @param  无
  * @retval 无
  */
static void Delay_Init(void)
{
    if(delay_init_flag) return;
    
    // 使能TIM2时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    // TIM2配置
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;          // 自动重装载值
    TIM_TimeBaseStructure.TIM_Prescaler = 72 - 1;           // 预分频器，72MHz/72 = 1MHz
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; // 时钟分频
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    
    // 启动定时器
    TIM_Cmd(TIM2, ENABLE);
    
    delay_init_flag = 1;
}

/**
  * @brief  微秒级延时
  * @param  xus 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_us(uint32_t xus)
{
    // 如果在FreeRTOS任务中且延时时间大于等于1000us，使用FreeRTOS延时
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING && xus >= 1000)
    {
        // 使用FreeRTOS的延时，但要注意最小单位是1ms
        uint32_t ticks = xus / 1000;
        if(ticks > 0)
        {
            vTaskDelay(pdMS_TO_TICKS(ticks));
        }
        
        // 处理剩余的微秒（小于1ms的部分）
        uint32_t remaining_us = xus % 1000;
        if(remaining_us > 0)
        {
            // 对于剩余的微秒，使用硬件定时器
            Delay_Init();
            
            uint32_t start_time = TIM_GetCounter(TIM2);
            while((TIM_GetCounter(TIM2) - start_time) < remaining_us)
            {
                // 等待
            }
        }
    }
    else
    {
        // 使用硬件定时器延时（在初始化阶段或中断中使用）
        Delay_Init();
        
        uint32_t start_time = TIM_GetCounter(TIM2);
        while((TIM_GetCounter(TIM2) - start_time) < xus)
        {
            // 等待，注意处理定时器溢出
            if(TIM_GetCounter(TIM2) < start_time)
            {
                // 定时器溢出了，重新计算
                start_time = TIM_GetCounter(TIM2);
            }
        }
    }
}

/**
  * @brief  毫秒级延时
  * @param  xms 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_ms(uint32_t xms)
{
    // 如果在FreeRTOS任务中，优先使用FreeRTOS延时
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
    {
        // 使用FreeRTOS的延时
        if(xms > 0)
        {
            vTaskDelay(pdMS_TO_TICKS(xms));
        }
    }
    else
    {
        // 在初始化阶段或中断中，使用硬件定时器
        while(xms--)
        {
            Delay_us(1000);
        }
    }
}

/**
  * @brief  秒级延时
  * @param  xs 延时时长，范围：0~4294967295
  * @retval 无
  */
void Delay_s(uint32_t xs)
{
    while(xs--)
    {
        Delay_ms(1000);
    }
}

/**
  * @brief  FreeRTOS专用毫秒延时（只能在任务中使用）
  * @param  xms 延时时长
  * @retval 无
  */
void Delay_ms_RTOS(uint32_t xms)
{
    if(xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
    {
        vTaskDelay(pdMS_TO_TICKS(xms));
    }
    else
    {
        Delay_ms(xms);  // 回退到普通延时
    }
}
