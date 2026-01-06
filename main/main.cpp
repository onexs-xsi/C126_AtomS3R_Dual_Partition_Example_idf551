#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "dirent.h"

static const char *TAG = "dual_part_example";

void read_txt_file(const char *filepath) {
    FILE *f = fopen(filepath, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading: %s", filepath);
        return;
    }

    char line[128];
    ESP_LOGI(TAG, "Reading content of %s:", filepath);
    while (fgets(line, sizeof(line), f) != NULL) {
        // Remove trailing newline character if present
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        ESP_LOGI(TAG, "  %s", line);
    }
    fclose(f);
}

void process_partition(const char *mount_point) {
    ESP_LOGI(TAG, "Listing files in %s", mount_point);
    DIR *dir = opendir(mount_point);
    if (dir == NULL) {
        ESP_LOGE(TAG, "Failed to open directory %s", mount_point);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        ESP_LOGI(TAG, "Found file: %s", entry->d_name);

        size_t len = strlen(entry->d_name);
        if (len > 4 && strcmp(entry->d_name + len - 4, ".txt") == 0) {
            char filepath[512];
            snprintf(filepath, sizeof(filepath), "%s/%s", mount_point, entry->d_name);
            read_txt_file(filepath);
        }
    }
    closedir(dir);
}

esp_err_t mount_partition(const char *partition_label, const char *mount_point) {
    ESP_LOGI(TAG, "Initializing SPIFFS for %s", partition_label);

    esp_vfs_spiffs_conf_t conf = {
      .base_path = mount_point,
      .partition_label = partition_label,
      .max_files = 5,
      .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partition %s size: total: %d, used: %d", partition_label, total, used);
    }
    return ESP_OK;
}

extern "C" void app_main(void)
{
    // Mount and process part_a
    if(mount_partition("partitions_a", "/part_a") == ESP_OK) {
        process_partition("/part_a");
    }

    // Mount and process part_b
    if(mount_partition("partitions_b", "/part_b") == ESP_OK) {
        process_partition("/part_b");
    }
}
