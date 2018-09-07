:: ==============================  
:: code by junjun  
:: add: 2016.4.7  
:: ==============================  

::添加环境变量P2PChat
@echo off
echo *=====添加P2PChat环境变量======*
echo.
::设定环境变量 为当前路径
set EnvPath=%~dp0
set EnvName=P2PChat

setx P2PChat %EnvPath%
  
echo.  
echo 创建环境变量  
echo.  

pause>nul  
