$SEWAS_ROOT=Get-Location

$VCPKG_ROOT="$SEWAS_ROOT/thirdparty/vcpkg/src"
$VCPKG_BUILD="$SEWAS_ROOT/thirdparty/vcpkg/build"

Write-Output "[START] Bootstraping"

$_vcpkg_not_found=!@(Test-Path "$VCPKG_ROOT/vcpkg.exe")
if ($_vcpkg_not_found)
{
    Write-Output "vcpkg has not been found"

    Write-Output "[START] install vcpkg"

    try
    {
        if (Test-Path "$VCPKG_BUILD")
        {
            Remove-Item -Recurse "$VCPKG_BUILD" -ErrorAction Stop
	}
        New-Item -Path "$VCPKG_BUILD" -ItemType "directory" -ErrorAction Stop
	
        Copy-Item "$SEWAS_ROOT/cmake/resources/vcpkg/CMakeLists.txt" -Destination "$VCPKG_BUILD" -ErrorAction Stop
	
        cd "$VCPKG_BUILD"
	cmake .
	cmake --build .
    }
    catch
    {
        throw "Failed to install vcpkg : $Error"
    }

    Write-Output "[STOP] install vcpkg"
}
else
{
    $_vcpkg="$VCPKG_ROOT/vcpkg.exe"
    
    Write-Output "vcpkg found : $_vcpkg"
    & $_vcpkg version
    & $_vcpkg update    
}

Write-Output "[STOP] Bootstraping"
