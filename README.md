# Photoshop-Plugin-Host
Plugin host for 8bf Photoshop filters

## pspiHost.dll
This dll engine enables loading and executing 8bf filters from user application. The code is written in VisualStudio 2017 C++ for Windows platform.
Host engine can be used also from Embarcadero C++ with created libs (implib -a/ mkexp -p) and also from Delphi, but conversion of pspiHost.h and pspiGlobals.h to respective pas files needs to be done.

Engine can be built in 32/64-bit mode and it supports 32/64-bit filters respectively.

Currently supported image formats: RGB, BGR, RGBA, BGRA, GRAYSCALE, RGB + external alpha channel, BGR + external alpha channel, GRAYSCALE + external alpha channel.
Image can be passed to engine using single contiguous image buffer, or by adding scalines (for possibly non-contiguous image buffers). 

Currently not supported in host engine:
masking (although mask can be set - will be available within few days), padding, parameter saving/restoring, filters requiring SuitePea, descriptors handling.

## capPspi
Console application for testing engine. Requires OpenCV as image container.






