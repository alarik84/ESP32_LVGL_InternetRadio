#pragma once

#include <lvgl.h>
#include <WiFi.h>
#include <HTTPClient.h>

class JC4827W543;
extern JC4827W543 dev;

inline lv_obj_t *scrDlna = NULL;
inline lv_obj_t *dlnaListWidget = NULL;
inline lv_obj_t *dlnaTitleLabel = NULL;
inline lv_obj_t *backButton = NULL;

const char* dlnaServerIP = "192.168.10.254"; 
const int dlnaServerPort = 8200;

// Struktura obsługująca zarówno plik, jak i folder
struct DlnaResource {
    String title;
    String id;       // ID kontenera (potrzebne, jeśli to folder)
    String url;      // Adres URL do MP3 (potrzebne, jeśli to plik)
    bool isFolder;   // true = folder, false = plik MP3
};

inline DlnaResource dlnaItems[30];
inline uint8_t dlnaItemCount = 0;

// Historia przeglądania (stos ID), aby móc wracać przyciskiem "Wstecz"
inline String currentObjectId = "0";
inline String parentObjectId = "0";

// Deklaracja wyprzedzająca wątku pobierania danych
void triggerDlnaFetch(String objectId);

// Callback dla kliknięć na liście (pliki i foldery)
static void lvgl_dlna_item_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
        intptr_t index = (intptr_t)lv_event_get_user_data(e);
        
        if (index >= 0 && index < dlnaItemCount)
        {
            if (dlnaItems[index].isFolder)
            {
                // KLIKNIĘTO FOLDER: zapamiętujemy historię i wchodzimy głębiej
                debugPrint("DLNA: Wchodzenie do folderu: " + dlnaItems[index].title);
                parentObjectId = currentObjectId;
                currentObjectId = dlnaItems[index].id;
                
                // Uruchamiamy pobieranie nowej zawartości
                triggerDlnaFetch(currentObjectId);
            }
            else
            {
                // KLIKNIĘTO PLIK MP3: odtwarzamy muzykę
                debugPrint("DLNA: Odtwarzanie utworu: " + dlnaItems[index].title);
                lv_label_set_text(dlnaTitleLabel, dlnaItems[index].title.c_str());
                dev.audio_connectToHost(dlnaItems[index].url.c_str());
            }
        }
    }
}

// Callback dla przycisku "Wstecz"
static void lvgl_dlna_back_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
        if (currentObjectId != "0")
        {
            debugPrint("DLNA: Powrót wyżej...");
            currentObjectId = parentObjectId;
            if (currentObjectId == "0") parentObjectId = "0";
            
            triggerDlnaFetch(currentObjectId);
        }
    }
}

// Funkcja sieciowa parsująca foldery oraz pliki
inline void discoverDlnaResources(String objectId)
{
    dlnaItemCount = 0;
    if (WiFi.status() != WL_CONNECTED) return;

    HTTPClient http;
    String url = "http://" + String(dlnaServerIP) + ":" + String(dlnaServerPort) + "/ctl/ContentDir";
    http.begin(url);
    http.addHeader("Content-Type", "text/xml; charset=\"utf-8\"");
    http.addHeader("SOAPACTION", "\"urn:schemas-upnp-org:service:ContentDirectory:1#Browse\"");

    // Wstrzykujemy dynamiczne objectId do zapytania SOAP
    String soapBody = 
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
        "<s:Envelope xmlns:s=\"http://google.com\" s:encodingStyle=\"http://google.com\">"
        "  <s:Body> "
        "    <u:Browse xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\">"
        "      <ObjectID>" + objectId + "</ObjectID>"
        "      <BrowseFlag>BrowseDirectChildren</BrowseFlag>"
        "      <Filter>*</Filter>"
        "      <StartingIndex>0</StartingIndex>"
        "      <RequestedCount>30</RequestedCount>"
        "      <SortCriteria></SortCriteria>"
        "    </u:Browse>"
        "  </s:Body>"
        "</s:Envelope>";

    int httpCode = http.POST(soapBody);

    if (httpCode == HTTP_CODE_OK) {
        String response = http.getString();
        int pos = 0;

        // JEDEN PARSER DLA OBU TYPÓW ZASOBÓW
        while (dlnaItemCount < 30) {
            int containerPos = response.indexOf("<container ", pos);
            int itemPos = response.indexOf("<item ", pos);

            // Sprawdzamy, co występuje wcześniej w tekście XML: folder czy plik?
            if (containerPos != -1 && (itemPos == -1 || containerPos < itemPos)) {
                // PARSOWANIE FOLDERU (<container>)
                int idStart = response.indexOf("id=\"", containerPos) + 4;
                int idEnd = response.indexOf("\"", idStart);
                int titleStart = response.indexOf("<dc:title>", idEnd) + 10;
                int titleEnd = response.indexOf("</dc:title>", titleStart);

                if (idEnd > idStart && titleEnd > titleStart) {
                    dlnaItems[dlnaItemCount].title = response.substring(titleStart, titleEnd);
                    dlnaItems[dlnaItemCount].id = response.substring(idStart, idEnd);
                    dlnaItems[dlnaItemCount].isFolder = true;
                    dlnaItemCount++;
                }
                pos = (titleEnd != -1) ? titleEnd : containerPos + 10;
            } 
            else if (itemPos != -1) {
                // PARSOWANIE PLIKU MP3 (<item>)
                int titleStart = response.indexOf("<dc:title>", itemPos) + 10;
                int titleEnd = response.indexOf("</dc:title>", titleStart);
                int resStart = response.indexOf("<res ", titleEnd);
                if (resStart != -1) {
                    resStart = response.indexOf("http://", resStart);
                    int resEnd = response.indexOf("</res>", resStart);

                    if (titleEnd > titleStart && resEnd > resStart) {
                        dlnaItems[dlnaItemCount].title = response.substring(titleStart, titleEnd);
                        dlnaItems[dlnaItemCount].url = response.substring(resStart, resEnd);
                        dlnaItems[dlnaItemCount].isFolder = false;
                        dlnaItemCount++;
                        pos = resEnd;
                        continue;
                    }
                }
                pos = itemPos + 10;
            } 
            else {
                break; // Brak kolejnych elementów
            }
        }
    }
    http.end();
}

// Zadanie FreeRTOS do asynchronicznego odświeżania widoku
void TaskFetchDlna(void * pvParameters) {
    String* targetId = (String*)pvParameters;
    discoverDlnaResources(*targetId);
    delete targetId; // Czyszczenie dynamicznego stringa ze stosu Tasku

    if (dlnaListWidget != NULL) {
        lv_obj_clean(dlnaListWidget); // Czyszczenie starej listy widgetów

        for (intptr_t i = 0; i < dlnaItemCount; i++) {
            // Przypisanie ikony: katalog lub plik audio
            const char* symbol = dlnaItems[i].isFolder ? LV_SYMBOL_DIRECTORY : LV_SYMBOL_AUDIO;
            
            lv_obj_t * btn = lv_list_add_button(dlnaListWidget, symbol, dlnaItems[i].title.c_str());
            lv_obj_add_event_cb(btn, lvgl_dlna_item_event_cb, LV_EVENT_CLICKED, (void*)i);
        }
    }
    vTaskDelete(NULL);
}

// Bezpieczny wyzwalacz wątku tła
void triggerDlnaFetch(String objectId) {
    String* pId = new String(objectId); // Alokacja bezpiecznego parametru dla wątku
    xTaskCreatePinnedToCore(TaskFetchDlna, "DlnaFetch", 6000, (void*)pId, 1, NULL, 0);
}

// Funkcje budowania interfejsu (GUI)
inline void createDlnaListWidget(lv_obj_t* currentScreen)
{
    dlnaListWidget = lv_list_create(currentScreen);
    lv_obj_set_size(dlnaListWidget, 240, 170);
    lv_obj_align(dlnaListWidget, LV_ALIGN_TOP_LEFT, 20, 55);

    // Pierwsze załadowanie z ID "0" (Root)
    triggerDlnaFetch(currentObjectId);
}

inline void createDlnaTitleLabel(lv_obj_t* currentScreen)
{
    dlnaTitleLabel = lv_label_create(currentScreen);
    lv_obj_set_width(dlnaTitleLabel, 190);
    lv_label_set_long_mode(dlnaTitleLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_align(dlnaTitleLabel, LV_ALIGN_TOP_RIGHT, -15, 20);
    lv_obj_set_style_text_color(dlnaTitleLabel, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_set_style_text_font(dlnaTitleLabel, &lv_font_montserrat_20, 0);
    lv_label_set_text(dlnaTitleLabel, "Przegladaj DLNA");
}

inline void createBackButton(lv_obj_t* currentScreen)
{
    backButton = lv_button_create(currentScreen);
    lv_obj_set_size(backButton, 40, 30);
    lv_obj_align(backButton, LV_ALIGN_TOP_LEFT, 20, 15);
    
    lv_obj_t *label = lv_label_create(backButton);
    lv_label_set_text(label, LV_SYMBOL_LEFT);
    lv_obj_center(label);

    lv_obj_add_event_cb(backButton, lvgl_dlna_back_event_cb, LV_EVENT_CLICKED, NULL);
}

inline void dlnaScreenCreate()
{
    scrDlna = lv_obj_create(NULL);
    createBackGround(scrDlna); 
    
    createDlnaTitleLabel(scrDlna);
    createBackButton(scrDlna);
    createDlnaListWidget(scrDlna);
    createVolumeControllerWidget(scrDlna);
}
