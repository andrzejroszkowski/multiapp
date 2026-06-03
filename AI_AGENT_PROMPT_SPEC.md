# Specyfikacja Techniczna Projektu: Multiapp Framework dla ESP32 (ESP-IDF)

Niniejszy dokument jest kompletnym opisem założeń architektonicznych i technicznych modułowego szkieletu aplikacji dla układów ESP32. Służy jako kontekst wejściowy dla agentów AI w celu ponownego odtworzenia, rozwoju lub modyfikacji kodu źródłowego.

---

## 1. Założenia Architektoniczne i Środowisko
*   **System operacyjny / SDK:** ESP-IDF v5.5 lub v6.0+ (natywny FreeRTOS).
*   **Standard języka:** Obiektowy C++26. Ścisła kontrola typów, zakaz surowych wskaźników (`malloc`/`free`) na rzecz inteligentnych wskaźników (`std::unique_ptr`).
*   **Wielozadaniowość (Multitasking):** Podział rdzeni procesora:
    *   **Core 0 (System Core):** Protokoły sieciowe (Wi-Fi/BLE), system logów, asynchroniczny nadzorca błędów.
    *   **Core 1 (App Core):** Warstwa aplikacji użytkownika, silnik graficzny, pętle pomiarowe czujników.
*   **Zarządzanie pamięcią:** Dynamiczne tworzenie i niszczenie całych kontekstów aplikacji podczas przełączania ekranów w celu optymalizacji pamięci RAM (Heap Isolation).
*   **Bezpieczeństwo wątkowe (Thread Safety):** Każda operacja na współdzielonych zasobach (ekran, magistrale sprzętowe I2C/SPI) musi być zabezpieczona za pomocą muteksów lub blokad krytycznych (RAII / Critical Sections).

---

## 2. Drzewo Katalogów Projektu

```text
my_multi_app/
├── CMakeLists.txt                  # Główny plik konfiguracyjny CMake projektu
├── Kconfig.projbuild               # Definicje menuconfig dla aplikacji
├── sdkconfig.ci                    # Nadpisanie flag kompilacji dla testów i GCOV
├── nvs_mock_data.csv               # Tabela danych wejściowych partycji NVS
├── main/
│   ├── CMakeLists.txt              # Rejestracja i zależności modułu głównego
│   └── main.cpp                    # Punkt wejścia (app_main) i orkiestracja bootowania
├── components/
│   ├── app_interface/
│   │   └── include/
│   │       └── app_interface.hpp   # Interfejs polimorficzny dla sub-aplikacji
│   ├── msg_bus/
│   │   ├── CMakeLists.txt
│   │   ├── msg_bus.cpp             # Szyna danych (Kolejki FreeRTOS)
│   │   └── include/msg_bus.hpp
│   ├── ui_engine/
│   │   ├── CMakeLists.txt
│   │   ├── idf_component.yml       # Manifest zależności (LVGL v9.1.0)
│   │   ├── ui_engine.cpp           # Wątek graficzny i system blokad ekranu
│   │   └── include/ui_engine.hpp
│   ├── hw_locks/
│   │   ├── CMakeLists.txt
│   │   ├── hw_locks.cpp            # Bezpieczny dostęp do I2C/SPI (RAII Mutex)
│   │   └── include/hw_locks.hpp
│   ├── error_hub/
│   │   ├── CMakeLists.txt
│   │   ├── error_hub.cpp           # Centralny asynchroniczny rejestrator awarii
│   │   ├── include/error_hub.hpp
│   │   └── test/
│   │       └── test_error_hub.cpp  # Testy jednostkowe komponentu (Unity)
│   └── telemetry_app/
│       ├── CMakeLists.txt
│       ├── telemetry_app.cpp       # Przykład aplikacji (konsument danych + GUI)
│       └── include/telemetry_app.hpp
└── test/                           # Wyizolowany projekt testów jednostkowych
    ├── CMakeLists.txt
    └── main/
        ├── CMakeLists.txt
        └── test_main.cpp           # Interaktywne menu testów Unity (UART)
```

---

## 3. Definicje Komponentów i Interfejsów

### A. Cykl życia aplikacji (`app_interface`)
Każda sub-aplikacja musi dziedziczyć po interfejsie `AppInterface`. Metoda `start()` alokuje zasoby i uruchamia lokalny wątek roboczy. Metoda `stop()` niszczy wątki oraz zwalnia uchwyty widgetów graficznych z pamięci LVGL.

### B. Szyna Komunikacyjna (`msg_bus`)
Wzorzec Singletonu opakowujący `QueueHandle_t` z FreeRTOS. Służy do bezpiecznego przekazywania struktur danych (np. telemetrycznych) pomiędzy wątkiem sprzętowym (Core 0) a wątkiem aplikacji/GUI (Core 1).

### C. Silnik Graficzny (`ui_engine`)
Odpowiada za inicjalizację biblioteki **LVGL v9**. Uruchamia dedykowane zadanie `lvgl_task` o priorytecie `4` na rdzeniu `1`. Ponieważ LVGL nie jest domyślnie bezpieczny wątkowo, moduł udostępnia metody `lock_ui()` i `unlock_ui()` oparte o sekcje krytyczne (`taskENTER_CRITICAL`), które muszą być wywoływane przed każdą modyfikacją obiektów graficznych z poziomu zewnętrznych wątków.

### D. Warstwa Blokad Sprzętowych (`hw_locks`)
Implementuje wzorzec **RAII (Resource Acquisition Is Initialization)** przy użyciu muteksów binarnych FreeRTOS (`xSemaphoreCreateMutex`). Zapobiega kolizjom na liniach SDA/SCL (I2C) lub MOSI/MISO (SPI). Poprzez klasę pomocniczą `I2C0Guard`, zajęcie magistrali następuje w momencie konstrukcji obiektu, a automatyczne zwolnienie w momencie wyjścia ze skończonego bloku kodu (destruktor), co eliminuje ryzyko permanentnego zakleszczenia (deadlock) w przypadku wystąpienia błędów wewnątrz transakcji.

### E. Rejestrator Błędów (`error_hub`)
Asynchroniczny system raportowania awarii działający na najwyższym priorytecie planisty FreeRTOS (`6`) na rdzeniu `0`. Moduł przyjmuje zgłoszenia z dowolnego wątku w sposób nieblokujący (timeout `0`), pakując je do bezpiecznej kolejki. Wykrycie błędu o krytyczności `ErrorSeverity::CRITICAL` przerywa działanie frameworku, wymusza zapis/flush logów i wykonuje bezpieczny restart sprzętowy procesora poprzez `esp_restart()`.

---

## 4. Konfiguracja CI/CD i Emulacji (QEMU + GCOV)
Szkielet został przystosowany do pełnej automatyzacji w chmurze (np. GitHub Actions) bez dostępu do fizycznego mikrokontrolera:

1.  **Generowanie NVS:** Narzędzie `nvs_partition_gen.py` kompiluje plik `nvs_mock_data.csv` do postaci binarnej partycji.
2.  **Instrumentacja Kodu:** Flagi w `sdkconfig.ci` (`CONFIG_GCOV_ENABLE=y`) wymuszają na kompilatorze wygenerowanie plików `.gcno` oraz `.gcda` w celu śledzenia pokrycia kodu.
3.  **Konsolidacja obrazu:** Narzędzie `esptool.py merge_bin` składa bootloader, strukturę partycji, dane NVS oraz plik binarny testów w jeden plik `qemu_flash.bin`.
4.  **Emulacja:** Emulator `qemu-system-xtensa -machine esp32` uruchamia firmware bezgraficznie. Skrypt CI monitoruje potok UART, szukając asercji `"Tests Passed"` lub `"Tests Failed"` generowanych przez framework testowy Unity, wymuszając timeout po 45 sekundach w przypadku zawieszenia wątku.
5.  **Raportowanie:** Narzędzie `lcov` odfiltrowuje biblioteki wewnętrzne ESP-IDF i generuje raport pokrycia kodu wyłącznie dla autorskich komponentów biznesowych.
