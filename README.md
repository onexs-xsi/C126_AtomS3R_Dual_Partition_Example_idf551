# AtomS3R 双分区示例 (ESP-IDF 5.5.1)

本项目演示如何在 ESP-IDF 中使用**多个 SPIFFS 分区**来存储和读取文件数据。适用于 M5Stack AtomS3R 或其他 ESP32-S3 设备。

## 📋 目录

- [项目简介](#项目简介)
- [项目结构](#项目结构)
- [分区配置详解](#分区配置详解)
- [快速开始](#快速开始)
- [修改分区后的维护指南](#修改分区后的维护指南)
- [添加第三个分区 (part_c)](#添加第三个分区-part_c)
- [常见问题](#常见问题)

---

## 项目简介

本示例展示了：
- 如何配置自定义分区表
- 如何创建多个 SPIFFS 分区镜像
- 如何在代码中挂载和读取多个 SPIFFS 分区中的文件

**工作原理**：程序启动后会依次挂载 `partitions_a` 和 `partitions_b` 两个 SPIFFS 分区，并遍历读取其中的 `.txt` 文件内容。

---

## 项目结构

```
C126_AtomS3R_Dual_Partition_Example_idf551/
├── CMakeLists.txt          # 项目主构建配置（包含分区镜像生成命令）
├── partitions.csv          # 自定义分区表
├── sdkconfig.defaults      # 默认SDK配置
├── sdkconfig               # 当前SDK配置（自动生成）
├── main/
│   ├── CMakeLists.txt      # main组件构建配置
│   └── main.cpp            # 主程序代码
├── part_a/                 # partitions_a 分区的源文件目录
│   ├── this_is_part_a_1.txt
│   ├── this_is_part_a_2.txt
│   └── this_is_part_a_3.txt
└── part_b/                 # partitions_b 分区的源文件目录
    ├── this_is_part_b_1.txt
    ├── this_is_part_b_2.txt
    └── this_is_part_b_3.txt
```

---

## 分区配置详解

### 分区表文件 (`partitions.csv`)

```csv
# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     ,        0x4000,
phy_init, data, phy,     ,        0x1000,
factory,  app,  factory, ,        3M,
partitions_a,   data, spiffs,  ,        2M,
partitions_b,   data, spiffs,  ,        2M,
```

| 分区名 | 类型 | 子类型 | 大小 | 说明 |
|--------|------|--------|------|------|
| `nvs` | data | nvs | 16KB | 非易失性存储，用于WiFi、BLE配置等 |
| `phy_init` | data | phy | 4KB | PHY初始化数据 |
| `factory` | app | factory | 3MB | 应用程序 |
| `partitions_a` | data | spiffs | 2MB | SPIFFS文件系统分区A |
| `partitions_b` | data | spiffs | 2MB | SPIFFS文件系统分区B |

> ⚠️ **注意**：本配置基于 **8MB Flash**。所有分区大小总和不能超过 Flash 容量。

### SPIFFS 镜像生成配置 (`CMakeLists.txt`)

```cmake
spiffs_create_partition_image(partitions_a part_a FLASH_IN_PROJECT)
spiffs_create_partition_image(partitions_b part_b FLASH_IN_PROJECT)
```

| 参数 | 说明 |
|------|------|
| 第1个参数 | 分区名（必须与 `partitions.csv` 中的名称一致） |
| 第2个参数 | 源文件目录（项目根目录下的文件夹名） |
| `FLASH_IN_PROJECT` | 编译时自动烧录到设备 |

### SDK配置 (`sdkconfig.defaults`)

```ini
CONFIG_ESPTOOLPY_FLASHSIZE_8MB=y           # Flash大小设为8MB
CONFIG_PARTITION_TABLE_CUSTOM=y             # 启用自定义分区表
CONFIG_PARTITION_TABLE_CUSTOM_FILENAME="partitions.csv"  # 指定分区表文件
```

---

## 快速开始

### 环境要求

- ESP-IDF v5.5.1 或更高版本
- 支持的硬件：M5Stack AtomS3R 或其他 ESP32-S3 设备（8MB Flash）

### 编译和烧录

```bash
# 1. 设置 ESP-IDF 环境
. $HOME/esp/esp-idf/export.sh    # Linux/macOS
# 或
%IDF_PATH%\export.bat            # Windows CMD
# 或
.$env:IDF_PATH\export.ps1        # Windows PowerShell

# 2. 设置目标芯片
idf.py set-target esp32s3

# 3. 配置项目（可选，使用默认配置可跳过）
idf.py menuconfig

# 4. 编译项目
idf.py build

# 5. 烧录到设备
idf.py -p COM3 flash    # Windows，替换为实际端口
idf.py -p /dev/ttyUSB0 flash    # Linux

# 6. 查看串口输出
idf.py -p COM3 monitor
```

### 预期输出

```
I (xxx) dual_part_example: Initializing SPIFFS for partitions_a
I (xxx) dual_part_example: Partition partitions_a size: total: 2031616, used: xxx
I (xxx) dual_part_example: Listing files in /part_a
I (xxx) dual_part_example: Found file: this_is_part_a_1.txt
I (xxx) dual_part_example: Reading content of /part_a/this_is_part_a_1.txt:
I (xxx) dual_part_example:   (文件内容)
...
I (xxx) dual_part_example: Initializing SPIFFS for partitions_b
I (xxx) dual_part_example: Partition partitions_b size: total: 2031616, used: xxx
I (xxx) dual_part_example: Listing files in /part_b
...
```

---

## 修改分区后的维护指南

当你需要修改分区配置时，请按以下步骤操作：

### 步骤 1：修改分区表 (`partitions.csv`)

例如，将 `partitions_b` 从 2MB 改为 1MB：

```csv
partitions_b,   data, spiffs,  ,        1M,
```

### 步骤 2：清理并重新编译

```bash
# 必须执行 fullclean，否则旧的分区信息可能残留
idf.py fullclean

# 重新编译
idf.py build
```

### 步骤 3：擦除 Flash 并重新烧录

```bash
# 擦除整个Flash（重要！分区变化后必须执行）
idf.py -p COM3 erase-flash

# 重新烧录
idf.py -p COM3 flash
```

### ⚠️ 重要注意事项

| 操作 | 是否需要 `fullclean` | 是否需要 `erase-flash` |
|------|---------------------|------------------------|
| 修改分区大小 | ✅ 是 | ✅ 是 |
| 添加/删除分区 | ✅ 是 | ✅ 是 |
| 仅修改分区内文件 | ❌ 否 | ❌ 否 |
| 修改应用代码 | ❌ 否 | ❌ 否 |

### 常见错误及解决方案

| 错误信息 | 原因 | 解决方法 |
|----------|------|----------|
| `Failed to find SPIFFS partition` | 分区名不匹配或分区表未更新 | 检查分区名，执行 `fullclean` |
| `Partition table checksum mismatch` | Flash 中分区表与编译不一致 | 执行 `erase-flash` |
| `Failed to mount or format filesystem` | SPIFFS 损坏或大小不匹配 | 执行 `erase-flash` |

---

## 添加第三个分区 (part_c)

按照以下 4 个步骤添加 `partitions_c` 分区：

### 步骤 1：创建源文件目录

在项目根目录下创建 `part_c` 文件夹，并添加你的文件：

```bash
mkdir part_c
echo "This is part C file 1" > part_c/this_is_part_c_1.txt
echo "This is part C file 2" > part_c/this_is_part_c_2.txt
```

项目结构变为：
```
├── part_a/
├── part_b/
└── part_c/          # 新增
    ├── this_is_part_c_1.txt
    └── this_is_part_c_2.txt
```

### 步骤 2：修改分区表 (`partitions.csv`)

添加新分区（注意调整其他分区大小以适应Flash容量）：

```csv
# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     ,        0x4000,
phy_init, data, phy,     ,        0x1000,
factory,  app,  factory, ,        3M,
partitions_a,   data, spiffs,  ,        1M,
partitions_b,   data, spiffs,  ,        1M,
partitions_c,   data, spiffs,  ,        1M,
```

> 💡 **容量计算**：8MB = 8192KB。上述配置使用约 6MB（nvs 16KB + phy 4KB + factory 3MB + 3×1MB），仍有约 2MB 余量。

### 步骤 3：修改 CMakeLists.txt

在项目根目录的 `CMakeLists.txt` 中添加新分区的镜像生成命令：

```cmake
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(C126_AtomS3R_Dual_Partition_Example_idf551)

spiffs_create_partition_image(partitions_a part_a FLASH_IN_PROJECT)
spiffs_create_partition_image(partitions_b part_b FLASH_IN_PROJECT)
spiffs_create_partition_image(partitions_c part_c FLASH_IN_PROJECT)  # 新增
```

### 步骤 4：修改主程序 (`main/main.cpp`)

在 `app_main()` 函数中添加挂载和处理新分区的代码：

```cpp
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

    // Mount and process part_c (新增)
    if(mount_partition("partitions_c", "/part_c") == ESP_OK) {
        process_partition("/part_c");
    }
}
```

### 步骤 5：编译并烧录

```bash
# 清理旧构建
idf.py fullclean

# 编译
idf.py build

# 擦除Flash并烧录
idf.py -p COM3 erase-flash
idf.py -p COM3 flash

# 查看输出
idf.py -p COM3 monitor
```

---

## 常见问题

### Q1：如何计算分区大小？

分区大小总和不能超过 Flash 容量。计算公式：

```
总大小 = nvs + phy_init + factory + partitions_a + partitions_b + ...
       ≤ Flash 容量 (如 8MB)
```

建议预留 256KB~512KB 作为安全余量。

### Q2：分区大小必须是特定值吗？

- 对于 SPIFFS 分区，大小应为 **4KB 的倍数**（Flash 扇区大小）
- 可以使用 `K`、`M` 后缀表示 KB 和 MB

### Q3：如何查看当前分区信息？

```bash
# 查看分区表
idf.py partition-table

# 或使用 esptool
esptool.py --port COM3 read_flash 0x8000 0xC00 partition_table.bin
python $IDF_PATH/components/partition_table/parttool.py partition_table.bin
```

### Q4：SPIFFS 和 FAT 文件系统哪个更好？

| 特性 | SPIFFS | FAT |
|------|--------|-----|
| 磨损均衡 | ✅ 内置 | ❌ 需额外实现 |
| 目录支持 | ❌ 扁平结构 | ✅ 支持 |
| 内存占用 | 较低 | 较高 |
| 适用场景 | 配置文件、小文件 | SD卡、大文件 |

### Q5：如何单独烧录某个分区？

```bash
# 只烧录 partitions_a 分区
parttool.py -p COM3 write_partition --partition-name=partitions_a --input=build/partitions_a.bin
```

---

## 参考资料

- [ESP-IDF SPIFFS 文档](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/storage/spiffs.html)
- [ESP-IDF 分区表文档](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-guides/partition-tables.html)
- [M5Stack AtomS3R 官方文档](https://docs.m5stack.com/)

---

## License

MIT License
