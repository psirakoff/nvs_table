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

    for (uint32_t addr = 0x1000; addr <= 0x1020; addr++) {
        if (eeprom_array[addr] != 0) {  // Check if the value is non-zero
            ESP_LOGI("TAG", "Address 0x%08" PRIx32 ": 0x%02X", addr, eeprom_array[addr]);
        }
    }
}

// Function to print all EEPROM values byte by byte
void print_all_eeprom_values() {
    ESP_LOGI("TAG", "Printing all EEPROM values byte by byte:");

    for (uint32_t addr = 0x1000; addr <= 0x1020; addr++) {
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
    eeprom_array[address + length] = 0x00;  // Add null terminator
}

// Function to write hex to EEPROM array
void write_hex_to_eeprom(uint32_t address, uint32_t value, uint8_t length, uint8_t endianness) {
    uint8_t *p = (uint8_t *)&value;
    ESP_LOGI("TAG", "Writing hex value 0x%08" PRIx32 " to address 0x%08" PRIx32, value, address);
    for (int i = 0; i < length; i++) {
        eeprom_array[address + i] = endianness ? p[length - 1 - i] : p[i];
    }
}

// Function to swap endianness of a 32-bit value
uint32_t swap_endianness(uint32_t value) {
    return ((value >> 24) & 0x000000FF) |
           ((value >> 8) & 0x0000FF00) |
           ((value << 8) & 0x00FF0000) |
           ((value << 24) & 0xFF000000);
}

// Function to calculate checksum of previous N bytes
uint8_t calculate_checksum(uint32_t address, uint8_t length) {
    uint16_t checksum = 0;  // Use 16-bit to handle overflow
    for (uint32_t addr = address - length; addr < address; addr++) {
        checksum += eeprom_array[addr];
    }
    return (uint8_t)(checksum & 0xFF);  // Return LSByte if sum > 0xFF
}

// Function to parse a CSV line and store the values in the EEPROM array
void parse_and_store_csv_data(char *csv_line) {
    eeprom_entry_t entry;
    char reg_name[10];  // To store the register name
    char type[10];  // To store the type (e.g., "FLOAT", "INT", "STRING", "HEX")
    char value_str[256];  // To store the value as a string
    uint8_t endianness;

    // Assuming CSV format: reg_name,address,type,value,length,endianness
    int ret = sscanf(csv_line, "%9[^,],%8" PRIx32 ",%9[^,],%255[^,],%hhu,%hhu", reg_name, &entry.address, type, value_str, &entry.length, &endianness);
    
    // Check if the line was parsed correctly
    if (ret != 6) {
        ESP_LOGE("TAG", "Failed to parse CSV line: %s", csv_line);
        return;
    }

    ESP_LOGI("TAG", "Parsed line - Register: %s, Address: 0x%08" PRIx32 ", Type: %s, Value: %s, Length: %d, Endianness: %d", reg_name, entry.address, type, value_str, entry.length, endianness);

    if (strcmp(type, "FLOAT") == 0) {
        float value = strtof(value_str, NULL);
        write_float_to_eeprom(entry.address, value);
    } else if (strcmp(type, "INT") == 0) {
        int value = atoi(value_str);
        write_int_to_eeprom(entry.address, value);
    } else if (strcmp(type, "STRING") == 0) {
        write_string_to_eeprom(entry.address, value_str, entry.length);
    } else if (strcmp(type, "HEX") == 0) {
        uint32_t value = strtoul(value_str, NULL, 16);
        if (endianness) {
            value = swap_endianness(value);
        }
        write_hex_to_eeprom(entry.address, value, entry.length, endianness);
    } else if (strcmp(type, "CS") == 0) {
        uint8_t checksum = calculate_checksum(entry.address, entry.length);
        ESP_LOGI("TAG", "Calculated checksum 0x%02X for address 0x%08" PRIx32, checksum, entry.address);
        eeprom_array[entry.address] = checksum;
    } else {
        ESP_LOGE("TAG", "Unknown type: %s", type);
    }
}

// Function to simulate reading CSV lines (this would typically come from a file or another source)
void simulate_csv_input() {
    // Simulate some CSV input lines
    char csv_data[][256] = {
        "reg1,1000,FLOAT,1.000,4,0",
        "reg2,1004,INT,42,4,0",
        "reg3,1008,STRING,Hello,6,0",
        "reg4,100E,HEX,0x11223344,4,1",
        "CS,1012,CS,x,4,0",  // Updated type to "CS"
        "reg5,1013,HEX,0xFFAABBCC,4,0",
        "CS1,1017,CS,x,4,0"
    };

    for (int i = 0; i < sizeof(csv_data) / sizeof(csv_data[0]); i++) {
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
