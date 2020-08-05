# Photoshop-Plugin-Host
Plugin host for 8bf Photoshop filters

## pspiHost.dll
This dll engine enables loading and executing 8bf filters from user application.  
The code is written in VisualStudio 2017 C++ for Windows platform.  
Host engine can be used also from Embarcadero C++ with created libs (implib -a/ mkexp -p) and also from Delphi, but conversion of pspiHost.h and pspiGlobals.h to respective pas files needs to be done.

Engine can be built in 32/64-bit mode and it supports 32/64-bit filters respectively.
Before compiling psipHost, you must download Adobe Photoshop SDK and install it in photoshopapi directory.

Currently supported image formats:
- RGB
- BGR
- RGBA
- BGRA
- GRAYSCALE
- RGB + external alpha channel
- BGR + external alpha channel
- GRAYSCALE + external alpha channel.

Image can be passed to engine using single contiguous image buffer, or by adding scalines (for possibly non-contiguous image buffers). 

Currently not supported in host engine:
- masking (currently only for blending input and output)
- padding
- parameters saving/restoring
- filters requiring SuitePea
- descriptorParameters handling
- ICCProfile
- Big document support (prepared but not yet implemented)

### Required
- C++ compiler (tested on Visual Studio 2017) 
- Adobe Photoshop SDK

## capPspi
Console application for testing engine.

### Required
- C++ compiler (tested on Visual Studio 2017) 
- OpenCV







