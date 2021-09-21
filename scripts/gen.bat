set SCRIPTS_DIR=%cd%
cd ..
mkdir project
cd project
cmake ..
cmake --build .
cd %SCRIPTS_DIR%
pause
