param(
    [Alias("p")]
    [string]$Port = "COM6",
    [Alias("b")]
    [int]$Baudrate = 3000000
)

$IDF_LOCATION='C:\Espressif\frameworks\esp-idf-stable'
Write-Host 'Exporting IDF tools from $IDF_LOCATION:'

& "$IDF_LOCATION\export.ps1"

$env:ESPTOOL_DIR = "$env:IDF_PATH\components\esptool_py\esptool"
$env:NVSTOOL_DIR = "$env:IDF_PATH\components\nvs_flash\nvs_partition_tool"
$env:NVSGEN_DIR = "$env:IDF_PATH\components\nvs_flash\nvs_partition_generator"
$env:PARTTOOL_DIR = "$env:IDF_PATH\components\partition_table"

Function run_esptool { python  "$env:ESPTOOL_DIR\esptool.py" $args}
Function run_espsecure { python  "$env:ESPTOOL_DIR\espsecure.py" $args}
Function run_espefuse { python  "$env:ESPTOOL_DIR\espefuse.py" $args}
Function run_parttool { python  "$env:PARTTOOL_DIR\parttool.py" $args}

Set-Alias -Name esptool -Value run_esptool
Set-Alias -Name espsecure -Value run_espsecure
Set-Alias -Name espefuse -Value run_espefuse
Set-Alias -Name parttool -Value run_parttool

Set-Alias -Name nvs_partition_gen -Value run_nvs_partition_gen
Set-Alias -Name nvs_tool -Value run_nvs_tool
Set-Alias -Name nvs_parser -Value run_nvs_parser
Set-Alias -Name nvs_logger -Value run_nvs_logger


Write-Host '  ---------------'
Write-Host 'Setting up esptool communication parameters:'
$env:ESPPORT = $Port
$env:ESPBAUD = $Baudrate
$env:IDF_BUILD_DIR = 'build'
Write-Host "  - ESPPORT      =$env:ESPPORT"
Write-Host "  - ESPBAUD      =$env:ESPBAUD"
Write-Host "  - IDF_BUILD_DIR=$env:IDF_BUILD_DIR"
