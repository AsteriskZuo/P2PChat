:: ==============================  
:: code by junjun  
:: add: 2016.4.7  
:: ==============================  

::��ӻ�������P2PChat
@echo off
echo *=====���P2PChat��������======*
echo.
::�趨�������� Ϊ��ǰ·��
set EnvPath=%~dp0
set EnvName=P2PChat

setx P2PChat %EnvPath%
  
echo.  
echo ������������  
echo.  

pause>nul  
