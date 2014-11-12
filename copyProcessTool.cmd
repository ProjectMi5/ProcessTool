@echo off
if not exist "y:\" net use y: "\\ltsp300\Users\ow\ProcessToolDebug" /user:ow
if exist "y:\" xcopy /y /e "C:\MI5_OPCUA\Process_Handling\Mi5_ProcessTool_Git\Win32\Debug\Mi5*" "y:"