### Nigulp 0.9

Contains complete project and source code for building usefull application for applying Photoshop 8bf filters to images.
Project is written in Embarcadero Berlin 10.1 C++. It uses some 3rd party VCL components so if you want to build the project,
you need to replace componenets with your own.

Here is a list of 3rd party components used in this project. 

- ImageEn VCL library: used for image loading, saving and viewing. Available at www.imageen.com. Extremely valuable Delphi/C++ package when dealing with images. No, I do not endores ImageEn.  
- SigmaPi VCL library: used for sliders and check boxes. It's my component collection and it's C++ onl VCL package, not very usefull for Delphi projects. So, my advice is to get rid of sliders and checkboxes and
put standard ones.     
 
The reason for using 3rd pary libraries is to make a usefull applicataion and not another "Hallo World" sample project. 
Speaking about sample/demo side of this project is an enlegant (I hope so) way of using pspiHost API calls.
All pspiHost API call are in frmNigulpU.cpp unit and call-back functions are in TNhelper.cpp unit.

Before building the project, you must copy pspiGlobals.h and pspiHeader.h form pspiHost directory to project's pspiHeaders directory.
Also, you need to copy pspiHost.lib and pspiHost.a from Additional/OMFLibs to project's pspiLibs directory.

Good luck.      


