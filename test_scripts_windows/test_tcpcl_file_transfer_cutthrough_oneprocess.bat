@Echo off

CALL C:\Users\btomko\Anaconda3\Scripts\activate.bat C:\Users\btomko\Anaconda3
START "RegServer" /D "%HDTN_SOURCE_ROOT%\common\regsvr" "cmd /k" "python" "main.py"
timeout /t 3
START "BpReceiveFile" /D "C:/hdtn_tmp" "cmd /k" "%HDTN_BUILD_ROOT%\common\bpcodec\apps\bpreceivefile.exe" "--save-directory=.\received" "--my-uri-eid=ipn:2.1" "--inducts-config-file=%HDTN_SOURCE_ROOT%\tests\config_files\inducts\bpsink_one_tcpcl_port4558.json"
timeout /t 3
START "HDTN One Process" /D "%HDTN_BUILD_ROOT%" "cmd /k" "%HDTN_BUILD_ROOT%\module\hdtn_one_process\hdtn-one-process.exe" "--cut-through-only-test" "--hdtn-config-file=%HDTN_SOURCE_ROOT%\tests\config_files\hdtn\hdtn_ingress1tcpcl_port4556_egress1tcpcl_port4558flowid2.json"
timeout /t 6
REM ONE FILE TEST NEXT LINE
START "BpSendFile" /D "C:/hdtn_tmp" "cmd /k" "%HDTN_BUILD_ROOT%\common\bpcodec\apps\bpsendfile.exe" "--max-bundle-size-bytes=4000000" "--file-or-folder-path=.\a.EXE" "--my-uri-eid=ipn:1.1" "--dest-uri-eid=ipn:2.1" "--outducts-config-file=%HDTN_SOURCE_ROOT%\tests\config_files\outducts\bpgen_one_tcpcl_port4556.json"
REM DIRECTORY TEST NEXT LINE (directories not recursive)
REM START "BpSendFile" /D "C:/hdtn_tmp" "cmd /k" "%HDTN_BUILD_ROOT%\common\bpcodec\apps\bpsendfile.exe" "--max-bundle-size-bytes=4000000" "--file-or-folder-path=." "--my-uri-eid=ipn:1.1" "--dest-uri-eid=ipn:2.1" "--outducts-config-file=%HDTN_SOURCE_ROOT%\tests\config_files\outducts\bpgen_one_tcpcl_port4556.json"