#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include "esp_log.h"

// Define EEPROM_SIZE and the array to store the EEPROM values
#define EEPROM_SIZE 1024  // Example size, modify as needed

uint8_t eeprom_array[EEPROM_SIZE];  // EEPROM array to store data

// Define a structure to hold parsed data for each entry
typedef struct {
    uint32_t address;
    union {
        float float_value;
        int int_value;
        char string_value[256];  // Assuming max 256 chars for the string
    } value;
    uint8_t length;  // Length of the data (only for strings)
} eeprom_entry_t;

// Function to print EEPROM values for debugging
void print_eeprom_values() {
    ESP_LOGI("TAG", "Printing EEPROM values:");

    for (uint32_t addr = 0x1000; addr <= 0x100D; addr++) {
        if (eeprom_array[addr] != 0) {  // Check if the value is non-zero
            ESP_LOGI("TAG", "Address 0x%08" PRIx32 ": 0x%02X", addr, eeprom_array[addr]);
        }
    }
}

// Function to print all EEPROM values byte by byte
void print_all_eeprom_values() {
    ESP_LOGI("TAG", "Printing all EEPROM values byte by byte:");

    for (uint32_t addr = 0x1000; addr <= 0x100D; addr++) {
        ESP_LOGI("TAG", "Address 0x%08" PRIx32 ": 0x%02X", addr, eeprom_array[addr]);
    }
}

// Function to write float to EEPROM array
void write_float_to_eeprom(uint32_t address, float value) {
    uint8_t *p = (uint8_t *)&value;
    ESP_LOGI("TAG", "Writing float value 0x%08" PRIx32 " to address 0x%08" PRIx32, *(uint32_t *)&value, address);
    for (int i = 0; i < sizeof(float); i++) {
        eeprom_array[address + i] = p[i];
    }
}

// Function to write integer to EEPROM array
void write_int_to_eeprom(uint32_t address, int value) {
    uint8_t *p = (uint8_t *)&value;
    ESP_LOGI("TAG", "Writing int value 0x%08" PRIx32 " to address 0x%08" PRIx32, (uint32_t)value, address);
    for (int i = 0; i < sizeof(int); i++) {
        eeprom_array[address + i] = p[i];
    }
}

// Function to write string to EEPROM array
void write_string_to_eeprom(uint32_t address, char *value, uint8_t length) {
    ESP_LOGI("TAG", "Writing string '%s' to address 0x%08" PRIx32, value, address);
    for (int i = 0; i < length; i++) {
        eeprom_array[address + i] = value[i];
    }
}

// Function to parse a CSV line and store the values in the EEPROM array
void parse_and_store_csv_data(char *csv_line) {
    eeprom_entry_t entry;
    char type[10];  // To store the type (e.g., "FLOAT", "INT", "STRING")
    char value_str[256];  // To store the value as a string

    // Assuming CSV format: address,type,value,length
    int ret = sscanf(csv_line, "%8" PRIx32 ",%9[^,],%255[^,],%hhu", &entry.address, type, value_str, &entry.length);
    
    // Check if the line was parsed correctly
    if (ret != 4) {
        ESP_LOGE("TAG", "Failed to parse CSV line: %s", csv_line);
        return;
    }

    ESP_LOGI("TAG", "Parsed line - Address: 0x%08" PRIx32 ", Type: %s, Value: %s, Length: %d", entry.address, type, value_str, entry.length);

    if (strcmp(type, "FLOAT") == 0) {
        float value = strtof(value_str, NULL);
        write_float_to_eeprom(entry.address, value);
    } else if (strcmp(type, "INT") == 0) {
        int value = atoi(value_str);
        write_int_to_eeprom(entry.address, value);
    } else if (strcmp(type, "STRING") == 0) {
        write_string_to_eeprom(entry.address, value_str, entry.length);
    } else {
        ESP_LOGE("TAG", "Unknown type: %s", type);
    }
}

// Function to simulate reading CSV lines (this would typically come from a file or another source)
void simulate_csv_input() {
    // Simulate some CSV input lines
    char csv_data[][256] = {
        "1000,FLOAT,1.000,4",
        "1004,INT,42,4",
        "1008,STRING,Hello,5",
        "100D,INT,0,4"
    };

    for (int i = 0; i < 4; i++) {
        parse_and_store_csv_data(csv_data[i]);
    }
}

// Main application function
void app_main() {
    ESP_LOGI("TAG", "Started app_main");

    // Simulate CSV input and store data in EEPROM array
    simulate_csv_input();

    // Print out the stored values in EEPROM
    print_eeprom_values();

    // Print all EEPROM values byte by byte
    print_all_eeprom_values();

    ESP_LOGI("TAG", "Finished app_main");
}
