echo off
for /r %%i in (*.xml) do "..\tools\bin\MakeN64Sprite.exe" %%i "..\files\%%~ni.spr"