@echo off

dv16_isd_sdr.exe isd_tools_flash-new.cfg -f uboot.boot ui.apu sdram.apu -resource res audlogo 32 -aline 4096 -bfumode5 

del upgrade\JL_AC*.bfu upgrade\*.bin

file_package.exe -file sdram.bin -dir upgrade -o package
copy /B JL_AC51.bfu+package upgrade\JL_AC54.bfu

del JL_AC51.bfu package
echo.
echo �����ļ���upgradeĿ¼�£���upgradeĿ¼�µ������ļ�copy��SD���ĸ�Ŀ¼���忨�ϵ缴������
echo.
pause