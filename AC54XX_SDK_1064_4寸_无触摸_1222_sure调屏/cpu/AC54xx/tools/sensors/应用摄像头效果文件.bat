


::������������������ͷЧ���ļ���Ĵ˴�, ע��·����Ϣ��������

..\dv16_isp_cfg.exe -i gc2023\JL_JT_NIGHT.J3L -o ..\res\isp_cfg_0.bin
..\dv16_isp_cfg.exe -i gc2023\JL_JT_INDOOR.J3L -o 	..\res\isp_cfg_1.bin
..\dv16_isp_cfg.exe -i gc2023\JL_JT_OUTCLUDY.J3L -o ..\res\isp_cfg_2.bin
..\dv16_isp_cfg.exe -i gc2023\JL_JT_OUTDOOR.J3L -o ..\res\isp_cfg_3.bin

set /p  run="�Ƿ��������أ�(y / n)
echo %run%
if %run%==y  (
    cd ..
    call ���ش���.bat
)





