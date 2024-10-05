@echo off

echo Exporting...

set name="jam-app"
set src_dir=.\bin\Debug
set dst_dir=.\export\%name%

set data_dir=.\run_tree\data
set libs_dir=.\deps\SDL2\lib\x64

RD /S /Q "%dst_dir%"

if NOT EXIST .\export mkdir export
if NOT EXIST .\export\version.txt echo 0 > .\export\version.txt

set /p BuildNum=<.\export\version.txt
set /A BuildNum=%BuildNum%+1
echo New Build: %BuildNum%
echo %BuildNum% > .\export\version.txt

set dst_dir=%dst_dir%-%BuildNum%
if NOT EXIST %dst_dir% mkdir %dst_dir%
if NOT EXIST %dst_dir%\data mkdir %dst_dir%\data

set ver_json={"Version": %BuildNum%}
echo %ver_json%>%dst_dir%\data\ver.json

copy %src_dir%\%name%.exe %dst_dir%\
rem copy %src_dir%\%name%.pdb %dst_dir%\

copy %libs_dir%\*.dll %dst_dir%\

xcopy %data_dir% %dst_dir%\data /h /i /c /k /e /r /y

rem VC_redist.exe (just in case?)
rem if NOT EXIST %dst_dir%\redist mkdir %dst_dir%\redist
rem copy ..\VC_redist.x64.exe %dst_dir%\redist

echo Zipping files...
if EXIST %dst_dir%.zip del %dst_dir%.zip
"C:\Program Files\7-Zip\7z.exe" a %dst_dir%.zip %dst_dir%
RD /S /Q "%dst_dir%"

echo Done.