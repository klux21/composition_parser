@setlocal
@echo off
if not exist "%APPDATA%\notepad++\userDefineLangs" echo Notepad++ add-on directory "%APPDATA%\notepad++\userDefineLangs" is missing!
if not exist "%APPDATA%\notepad++\userDefineLangs" goto EXIT

copy /B /Y data_composition_format.xml "%APPDATA%\notepad++\userDefineLangs\data_composition_format.xml"

:EXIT
