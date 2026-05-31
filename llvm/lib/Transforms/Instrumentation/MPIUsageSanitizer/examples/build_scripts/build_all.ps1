# PowerShell Build Script for MPI Usage Sanitizer examples
# Compiles all C example programs with and without instrumentation natively on Windows.

$ErrorActionPreference = "Stop"

# Paths
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ExamplesDir = Resolve-Path (Join-Path $ScriptDir "..")
$BuildDir = Join-Path $ExamplesDir "build"
$InstrumentedDir = Join-Path $BuildDir "instrumented"
$UninstrumentedDir = Join-Path $BuildDir "uninstrumented"

# MS-MPI Paths
$MpiSdkDir = "C:\Program Files (x86)\Microsoft SDKs\MPI"
$MpiIncDir = Join-Path $MpiSdkDir "Include"

# Check MS-MPI
if (-not (Test-Path $MpiIncDir)) {
    Write-Warning "MS-MPI SDK not found at $MpiSdkDir"
    Write-Warning "Please install MS-MPI SDK using: winget install Microsoft.msmpisdk"
    exit 1
}

# Find Compiler (prefer clang, fallback to cl/MSVC, fallback to gcc/MinGW)
$Compiler = $null
$IsClang = $false
$IsGcc = $false

if (Get-Command "clang.exe" -ErrorAction SilentlyContinue) {
    $Compiler = "clang.exe"
    $IsClang = $true
} elseif (Get-Command "cl.exe" -ErrorAction SilentlyContinue) {
    $Compiler = "cl.exe"
} elseif (Get-Command "gcc.exe" -ErrorAction SilentlyContinue) {
    $Compiler = "gcc.exe"
    $IsGcc = $true
}

if ($null -eq $Compiler) {
    Write-Error "No C/C++ compiler (clang, cl, or gcc) was found in your PATH."
    exit 1
}

# Check architecture of the compiler to choose Lib\x64 vs Lib\x86
$MpiLibDir = Join-Path $MpiSdkDir "Lib\x64"
if ($IsGcc) {
    $TargetMachine = & $Compiler -dumpmachine
    if ($TargetMachine -match "i[3-6]86" -or $TargetMachine -match "mingw32" -or $TargetMachine -match "win32") {
        $MpiLibDir = Join-Path $MpiSdkDir "Lib\x86"
        Write-Host "Detected 32-bit GCC. Using 32-bit MS-MPI libraries from $MpiLibDir" -ForegroundColor Yellow
    }
}

Write-Host "Using compiler: $Compiler" -ForegroundColor Cyan

# Setup Build Directories
Write-Host "Setting up build directories..." -ForegroundColor Blue
if (Test-Path $BuildDir) {
    Remove-Item -Recurse -Force $BuildDir -ErrorAction SilentlyContinue
}

$Categories = @("basic", "collective", "point_to_point", "error_cases", "correct", "performance", "multi_language")
foreach ($cat in $Categories) {
    New-Item -ItemType Directory -Path (Join-Path $InstrumentedDir $cat) -Force | Out-Null
    New-Item -ItemType Directory -Path (Join-Path $UninstrumentedDir $cat) -Force | Out-Null
}
Write-Host "Build directories created successfully.`n" -ForegroundColor Green

# Function to compile a C file
function Compile-CFile {
    param (
        [string]$Category,
        [string]$FilePath
    )

    $Basename = [System.IO.Path]::GetFileNameWithoutExtension($FilePath)
    $UninstOut = Join-Path (Join-Path $UninstrumentedDir $Category) "$Basename.exe"
    $InstOut = Join-Path (Join-Path $InstrumentedDir $Category) "$Basename.exe"

    # 1. Compile Uninstrumented
    Write-Host "  Compiling $Basename (uninstrumented)..." -ForegroundColor Yellow
    try {
        if ($IsClang) {
            # Clang compilation
            & $Compiler -O2 -g -Wall "-I$MpiIncDir" "$FilePath" -o "$UninstOut" "-L$MpiLibDir" -lmsmpi
        } elseif ($IsGcc) {
            # GCC compilation (links against msmpi.lib directly on Windows)
            & $Compiler -O2 -g -Wall "-I$ScriptDir" "-I$MpiIncDir" "$FilePath" -o "$UninstOut" "-L$MpiLibDir" -lmsmpi
        } else {
            # MSVC cl compilation
            & $Compiler /I"$MpiIncDir" /O2 "$FilePath" /link "/LIBPATH:$MpiLibDir" msmpi.lib "/OUT:$UninstOut" | Out-Null
        }
        Write-Host "    $Basename (uninstrumented) compiled successfully" -ForegroundColor Green
    }
    catch {
        Write-Warning "    Failed to compile $Basename (uninstrumented)"
        return
    }

    # 2. Compile Instrumented (If pass is not available, we copy uninstrumented as a fallback)
    # Developers can load the pass using LLVM arguments if they have Clang and the pass plugin built.
    Write-Host "  Compiling $Basename (instrumented)..." -ForegroundColor Yellow
    $CompiledInstrumented = $false
    
    # Try compiling with experimental pass if Clang is available
    if ($IsClang) {
        $PassPlugin = Join-Path $ScriptDir "..\..\..\..\..\build\lib\MPIUsageSanitizer.dll"
        if (Test-Path $PassPlugin) {
            try {
                & $Compiler -O2 -g -Wall -fexperimental-new-pass-manager -fpass-plugin="$PassPlugin" "-I$MpiIncDir" "$FilePath" -o "$InstOut" "-L$MpiLibDir" -lmsmpi
                Write-Host "    $Basename (instrumented) compiled successfully with sanitizer pass" -ForegroundColor Green
                $CompiledInstrumented = $true
            }
            catch {
                Write-Warning "    Sanitizer compilation failed, falling back to copy"
            }
        }
    }

    if (-not $CompiledInstrumented) {
        # Fallback to copy the uninstrumented executable
        Copy-Item "$UninstOut" "$InstOut" -Force
        Write-Host "    $Basename (instrumented) copied from uninstrumented fallback" -ForegroundColor DarkGreen
    }
}

# Compile each category
foreach ($cat in $Categories) {
    $SrcDir = Join-Path $ExamplesDir $cat
    if (-not (Test-Path $SrcDir)) { continue }

    Write-Host "Compiling C examples in $cat..." -ForegroundColor Blue
    $CFiles = Get-ChildItem -Path $SrcDir -Filter "*.c"
    foreach ($file in $CFiles) {
        Compile-CFile -Category $cat -FilePath $file.FullName
    }
    Write-Host ""
}

Write-Host "Windows build process completed successfully!" -ForegroundColor Green
