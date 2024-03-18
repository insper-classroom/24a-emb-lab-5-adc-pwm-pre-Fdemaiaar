#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>

#include "data.h"
QueueHandle_t xQueueData;

// não mexer! Alimenta a fila com os dados do sinal
void data_task(void *p) {
    vTaskDelay(pdMS_TO_TICKS(400));

    int data_len = sizeof(sine_wave_four_cycles) / sizeof(sine_wave_four_cycles[0]);
    for (int i = 0; i < data_len; i++) {
        xQueueSend(xQueueData, &sine_wave_four_cycles[i], 1000000);
    }

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void process_task(void *p) {
    int data = 0;
    int window[5] = {0}; // Janela para o filtro de média móvel
    int sum = 0; // Soma dos valores na janela
    int count = 0; // Contador para saber quando a janela está cheia
    int index = 0; // Índice para adicionar novos valores na janela

    while (true) {
        if (xQueueReceive(xQueueData, &data, portMAX_DELAY)) {
            // Adiciona o novo valor na janela, substituindo o mais antigo
            sum -= window[index]; // Subtrai o valor antigo da soma
            window[index] = data; // Atualiza com o novo valor
            sum += data; // Adiciona o novo valor à soma

            // Avança o índice e reseta se necessário
            index = (index + 1) % 5;

            // Aumenta o contador até que a janela esteja cheia
            if (count < 5) {
                count++;
            }

            // Calcula a média móvel se a janela estiver cheia
            if (count == 5) {
                int moving_average = sum / 5;
                printf("Média Móvel: %d\n", moving_average);
            }

            // Deixar esse delay!
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}


int main() {
    stdio_init_all();

    xQueueData = xQueueCreate(64, sizeof(int));

    xTaskCreate(data_task, "Data task ", 4096, NULL, 1, NULL);
    xTaskCreate(process_task, "Process task", 4096, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
