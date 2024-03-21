#include <Arduino.h>
#include "lvgl_hal.h"
#include "pinout.h"

#include "ui/ui.h"

// Hardware init
#ifdef I2C0_SUPPORT
TwoWire i2c0 = TwoWire(0);
// TwoWire i2c1 = TwoWire(1);
#endif

void lvgl_ui_task(void *parameter);
void lis2dw12_task(void *parameter);
void wifi_task(void *parameter);
void moonraker_task(void *parameter);
void setup()
{
    Serial.begin(115200);
    while (!Serial)
        delay(10);
    printf("ESP32 Partition table:\n\n");

    printf("| Type | Sub |  Offset  |   Size   |       Label      |\n");
    printf("| ---- | --- | -------- | -------- | ---------------- |\n");

    esp_partition_iterator_t pi = esp_partition_find(ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);
    if (pi != NULL)
    {
        do
        {
            const esp_partition_t *p = esp_partition_get(pi);
            printf("|  %02x  | %02x  | 0x%06X | 0x%06X | %-16s |\r\n",
                   p->type, p->subtype, p->address, p->size, p->label);
        } while (pi = (esp_partition_next(pi)));
    }

    Serial.print("free heap size: ");
    Serial.println(esp_get_free_heap_size());
    Serial.println("\r\n\r\n------------- Knomi startup -----------\r\n");
    Serial.println("SPI Flash: ");
    Serial.print("  Size: ");
    Serial.println(ESP.getFlashChipSize());
    Serial.print("  Speed: ");
    Serial.println(ESP.getFlashChipSpeed());
    Serial.print("  Mode: ");
    Serial.println(ESP.getFlashChipMode());
    Serial.println("SPI PSRAM: ");
    Serial.print("  Found: ");
    Serial.println(psramFound());
    Serial.print("  Size: ");
    Serial.println(ESP.getPsramSize());
    Serial.println("\r\n\r\n------------------------------------------\r\n");

#ifdef I2C0_SUPPORT
    i2c0.begin(I2C0_SDA_PIN, I2C0_SCL_PIN, I2C0_SPEED);
    // i2c1.begin(I2C1_SDA_PIN, I2C1_SCL_PIN, I2C1_SPEED);
#endif

    xTaskCreate(lvgl_ui_task, "lvgl ui",
                4096, // Stack size (bytes)
                NULL, // Parameter to pass
                10,   // Task priority
                NULL  // Task handle
    );

#ifdef LIS2DW_SUPPORT
    xTaskCreate(lis2dw12_task, "lis2dw12",
                4096, // Stack size (bytes)
                NULL, // Parameter to pass
                9,    // Task priority
                NULL  // Task handle
    );
#endif

    xTaskCreate(wifi_task, "wifi",
        4096,  // Stack size (bytes)
        NULL,  // Parameter to pass
        8,     // Task priority
        NULL   // Task handle
        );

    xTaskCreate(moonraker_task, "moonraker",
        4096,  // Stack size (bytes)
        NULL,  // Parameter to pass
        7,     // Task priority
        NULL   // Task handle
        );
}

void loop()
{
}
