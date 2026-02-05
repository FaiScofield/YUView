# PowerShell脚本：从CMakeLists.txt中提取版本号

# 获取脚本所在目录的父目录（项目根目录）
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectRoot = Split-Path -Parent $scriptDir
$cmakeFile = Join-Path $projectRoot "CMakeLists.txt"
$content = Get-Content $cmakeFile

foreach ($line in $content) {
    if ($line -match 'project\([^)]*VERSION\s+([0-9]+\.[0-9]+\.[0-9]+)') {
        $version = $matches[1]
        Write-Output $version
        exit 0
    }
}

# 如果没找到版本号，则返回默认值
Write-Output "1.0.0"
exit 1