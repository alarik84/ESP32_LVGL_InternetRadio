#pragma once

#include <Arduino.h>
#include <ArduinoJson.h> 
#include <SD.h>

class RadioStations {

    public:
        struct station {
            String name;
            String description;
            String sourceURL;
        };

        uint8_t getSourcesCount() {
            return radioSourcesCount;
        };

    private:
        static uint16_t constexpr jsonRadioSourceMaxSize = 4096;
        static uint8_t constexpr max_stations = 20;
        uint8_t radioSourcesCount = 0;
        String radioUrlsArray[max_stations];         // Stores the radio station URLs
        String radioNamesArray[max_stations];        // Stores the radio station names
        String radioDescriptionsArray[max_stations]; // Stores the radio station descriptions

    private:
        // Read the json radio source from the SD Card
        void readRadioSources(File file) {
            if (!file)
            {
                Serial.println("Failed to open radio.json");
                return;
            }

            size_t size = file.size();
            if (size > jsonRadioSourceMaxSize)
            {
                Serial.println("radio.json is too large");
                file.close();
                return;
            }

            std::unique_ptr<char[]> buf(new char[size + 1]);
            file.readBytes(buf.get(), size);
            buf[size] = '\0';
            file.close();
            
            //DynamicJsonDocument doc(jsonRadioSourceMaxSize);
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, buf.get());
            if (error)
            {
                Serial.print("deserializeJson() failed: ");
                Serial.println(error.f_str());
                return;
            }

            JsonArray sources = doc["radioSources"].as<JsonArray>();

            for (JsonObject src : sources)
            {
                const char *name = src["name"].as<const char *>();
                const char *url = src["url"].as<const char *>();
                const char *description = src["description"].as<const char *>();
                if (name && url && description && radioSourcesCount < max_stations)
                {
                    // Store the name, URL, and description in parallel arrays.
                    radioNamesArray[radioSourcesCount] = String(name);
                    radioUrlsArray[radioSourcesCount] = String(url);
                    radioDescriptionsArray[radioSourcesCount] = String(description);
                    radioSourcesCount ++;
                }
            }
        }
    public:
        void init(File sources) {
            readRadioSources(sources);
        }
        String StationsList() {
            String radioOptions;
            for (uint8_t i; i < radioSourcesCount; i++){
                radioOptions += radioNamesArray[i];
                radioOptions += "\n";
            }
            if (radioOptions.endsWith("\n"))
            {
                radioOptions.remove(radioOptions.length() - 1);
            }
            return radioOptions;
        }

        station GetStation(uint8_t stationIndex) {
            station station;
            station.name = radioNamesArray[stationIndex];
            station.description = radioDescriptionsArray[stationIndex];
            station.sourceURL = radioUrlsArray[stationIndex];
            return station;
        }
};