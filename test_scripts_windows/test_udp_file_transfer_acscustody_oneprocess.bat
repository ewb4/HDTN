@Echo off
SET HDTN_CONFIG_FILE_PARAM="--hdtn-config-file=%HDTN_SOURCE_ROOT%\tests\config_files\hdtn\hdtn_ingress1tcpcl_port4556_egress1udpfast_port4558flowid2_bpgencustodyxfer.json"
REM INITIALIZE ANACONDA PYTHON NEXT LINE (requires activate.bat in PATH)
REM CALL activate.bat
START "BpReceiveFile" /D "C:/hdtn_tmp" "cmd /k" "%HDTN_BUILD_ROOT%\common\bpcodec\apps\bpreceivefile.exe" "--acs-aware-bundle-agent" "--save-directory=.\received" "--my-uri-eid=ipn:2.1" "--inducts-config-file=%HDTN_SOURCE_ROOT%\tests\config_files\inducts\bpsink_one_udp_port4558_tinycb.json" "--custody-transfer-outducts-config-file=%HDTN_SOURCE_ROOT%\tests\config_files\outducts\bpsink_one_stcp_port4557.json"
timeout /t 3
START "HDTN One Process" /D "%HDTN_BUILD_ROOT%" "cmd /k" "%HDTN_BUILD_ROOT%\module\hdtn_one_process\hdtn-one-process.exe" "%HDTN_CONFIG_FILE_PARAM%"
timeout /t 6
START "Send Release ipn:1.0" /D "%HDTN_BUILD_ROOT%" "cmd /k" "%HDTN_BUILD_ROOT%\module\storage\hdtn-release-message-sender.exe" "--release-message-type=start" "--dest-uri-eid-to-release-or-stop=ipn:1.0" "--delay-before-send=3" "%HDTN_CONFIG_FILE_PARAM%"
timeout /t 6
REM ONE FILE TEST NEXT LINE
START "BpSendFile" /D "C:/hdtn_tmp" "cmd /k" "%HDTN_BUILD_ROOT%\common\bpcodec\apps\bpsendfile.exe" "--custody-transfer-use-acs" "--max-bundle-size-bytes=60000" "--file-or-folder-path=.\a.EXE" "--my-uri-eid=ipn:1.1" "--dest-uri-eid=ipn:2.1" "--outducts-config-file=%HDTN_SOURCE_ROOT%\tests\config_files\outducts\bpgen_one_tcpcl_port4556_allowrxcustodysignals.json"
START "Send Release ipn:2.1" /D "%HDTN_BUILD_ROOT%" "cmd /k" "%HDTN_BUILD_ROOT%\module\storage\hdtn-release-message-sender.exe" "--release-message-type=start" "--dest-uri-eid-to-release-or-stop=ipn:2.1" "--delay-before-send=12" "%HDTN_CONFIG_FILE_PARAM%"

REM DIRECTORY TEST NEXT LINE (directories not recursive)
REM START "BpSendFile" /D "C:/hdtn_tmp" "cmd /k" "%HDTN_BUILD_ROOT%\common\bpcodec\apps\bpsendfile.exe" "--max-bundle-size-bytes=4000000" "--file-or-folder-path=." "--my-uri-eid=ipn:1.1" "--dest-uri-eid=ipn:2.1" "--outducts-config-file=%HDTN_SOURCE_ROOT%\tests\config_files\outducts\bpgen_one_tcpcl_port4556.json"
