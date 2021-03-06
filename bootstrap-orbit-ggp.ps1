$conan = Get-Command -ErrorAction Ignore conan

if (!$conan) {
  Write-Host "Conan not found. Trying to install it via python-pip..."
	
  $pip3 = Get-Command -ErrorAction Ignore pip3
	
  if(!$pip3) {	
    Write-Error -ErrorAction Stop @"
It seems you don't have Python3 installed (pip3.exe not found).
Please install Python or make it available in the path.
Alternatively you could also just install conan manually and make
it available in the path.
"@
  }

  $result = Start-Process -FilePath $pip3.Path -ArgumentList "install","conan" -Wait -NoNewWindow -ErrorAction Stop -PassThru
  if ($result.ExitCode -ne 0) {
    Throw "Error while installing conan via pip3."
  }
    
  $conan = Get-Command -ErrorAction Ignore conan
  if (!$conan) {
    Write-Error -ErrorAction Stop @"
It seems we installed conan sucessfully, but it is not available
in the path. Please ensure that your Python user-executable folder is
in the path and call this script again.
You can call 'pip3 show -f conan' to figure out where conan.exe was placed.
"@
  }
} else {
  Write-Host "Conan found. Skipping installation..."
}

# Install conan config
$result = Start-Process -FilePath $conan.Path -ArgumentList "config","install","$PSScriptRoot\contrib\conan\configs\windows" -Wait -NoNewWindow -ErrorAction Stop -PassThru
if ($result.ExitCode -ne 0) {
  Throw "Error while installing conan config."
}

# Install Stadia GGP SDK
$search_result = & $conan.Path search ggp_sdk 2>&1
if (-not ($search_result -like "*orbitdeps*")) {
  if (-not (Test-Path -Path "$PSScriptRoot\contrib\conan\recipes\ggp_sdk" -PathType Container)) {
    $git = Get-Command -ErrorAction Stop git
    $result = Start-Process -FilePath $git.Path -Wait -NoNewWindow -ErrorAction Stop -PassThru -ArgumentList "clone","sso://user/hebecker/conan-ggp_sdk","$PSScriptRoot\contrib\conan\recipes\ggp_sdk"
    if ($result.ExitCode -ne 0) {
      Throw "Error in git clone command."
    }
  }

  $result = Start-Process -FilePath $conan.Path -Wait -NoNewWindow -ErrorAction Stop -PassThru -ArgumentList "export","$PSScriptRoot\contrib\conan\recipes\ggp_sdk","orbitdeps/stable"
  if ($result.ExitCode -ne 0) {
      Throw "Error in conan export command."
  }
} ELSE {
  Write-Host "ggp_sdk seems to be installed already. Skipping installation step..."
}

# Start build
if ($args) {
  & "$PSScriptRoot\build.ps1" $args
} else {
  & "$PSScriptRoot\build.ps1" "ggp_release"
}