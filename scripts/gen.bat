set SCRIPTS_DIR=%cd%
cd ..
mkdir project
cd project
cmake ..
cmake --build .
cd ..
xcopy shaders\ project\shaders\ /I /Y
cd %SCRIPTS_DIR%
pause
