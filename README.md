ESP32_LVGL_InternetRadio ESP32 WROOM32U + PCM5102A CJMCU - 5102

ESP32 two cores with with 480x272 LCD IPS touch dispplay DAC PCM5102A CJMCU - 5102 internet radio player controlled through MQTT

<img width="454" height="339" alt="455274298-6e098ecd-b33a-498f-9400-6caf4f009150" src="https://github.com/user-attachments/assets/8df4de19-7f6b-48b1-964b-71d42b0f913d" />

Please note, that there is separate loop for second thread. If you have single core ESP32 you have to merge the threads.

Pinout: PCM5102A XMT = PCM5102A VCC

PCM5102A 3.3 = ESP32 3.3

PCM5102A GND = ESP32 GND

PCM5102A BCK = ESP32 4

PCM5102A DIN = ESP32 2

PCM5102A LCK = ESP32 15

I'm new in C++ so don't hesitate to give me advices. Cheers :)
