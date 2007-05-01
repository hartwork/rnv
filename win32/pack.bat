@ECHO OFF
7z a -tzip -mx=9 -mm=Deflate rnv-1.7.9-win32bin.zip arx.exe expat.dll rnv.exe ..\COPYING ..\readme32.txt
PAUSE
