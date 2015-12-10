@ECHO OFF

REM ___________________________________
SET SAGA_VERSION=saga_2.2.3
SET SVN__VERSION=2-2-3

SET SAGA_ROOT=D:\saga\saga-code\trunk

SET ZIPEXE="C:\Program Files\7-Zip\7z.exe" a -r -y -mx5
SET ISETUP="C:\Program Files (x86)\Inno Setup 5\ISCC.exe"
SET SVNEXE=svn
SET DOXEXE=doxygen.exe
SET SWIGEXE="D:\libs\swigwin-3.0.7\swig.exe"
SET PYTHONDIR=D:\libs\Python-2.7
SET PYTHONVER=27


REM ___________________________________
REM ###################################
REM PRE-RELEASE STEPS
REM ###################################

ECHO __________________________________
ECHO ##################################
ECHO #
ECHO # MAKE SAGA RELEASE: %SVN__VERSION%
ECHO #
ECHO ##################################
ECHO.
ECHO Things you should have updated before:
ECHO - ArcSAGA Tools
ECHO - Translation Files
ECHO.
ECHO Enter 'y' to continue!
SET /P CONTINUE=
IF '%CONTINUE%' == 'y' GOTO CONTINUE
EXIT
:CONTINUE
PAUSE

REM ___________________________________
REM Create a branch
REM %SVNEXE% copy svn://svn.code.sf.net/p/saga-gis/code-0/trunk svn://svn.code.sf.net/p/saga-gis/code-0/branches/release-%SVN__VERSION% -m "branch release-%SVN__VERSION% created from trunk"

REM ___________________________________
REM Update saga_cmd parameter interface configuration
SET SAGA_TOOLS_PY=%SAGA_ROOT%\src\scripting\python\helpers\saga_cmd_param_interface\tools.py
"%SAGA_ROOT%\bin\saga_vc_Win32\saga_cmd.exe" dev_tools 6 -INPUT="%SAGA_TOOLS_PY%" -OUTPUT="%SAGA_TOOLS_PY%"


REM ___________________________________
REM ###################################
REM MAKE RELEASE
REM ###################################

REM ___________________________________
MKDIR "%SAGA_VERSION%"
PUSHD "%SAGA_VERSION%"

REM ___________________________________
REM win32 Binaries
SET SAGA_CONFIG=win32
MKDIR "%SAGA_VERSION%_%SAGA_CONFIG%"
PUSHD "%SAGA_VERSION%_%SAGA_CONFIG%"
XCOPY /C/S/Q/Y "%SAGA_ROOT%\saga-gis\bin\saga_vc_%SAGA_CONFIG%"
DEL /F saga_gui.cfg saga_gui.ini *.exp modules\*.exp modules\*.lib modules\dev_tools.*
RMDIR /S/Q _private
POPD
%ZIPEXE% "%SAGA_VERSION%_%SAGA_CONFIG%.zip" "%SAGA_VERSION%_%SAGA_CONFIG%"

COPY "%SAGA_ROOT%\saga_setup_readme.rtf" "%SAGA_VERSION%_%SAGA_CONFIG%"
COPY "%SAGA_ROOT%\saga_setup_%SAGA_CONFIG%.iss" "%SAGA_VERSION%_%SAGA_CONFIG%"
%ISETUP% "%SAGA_VERSION%_%SAGA_CONFIG%\saga_setup_%SAGA_CONFIG%.iss"
MOVE "%SAGA_VERSION%_%SAGA_CONFIG%\%SAGA_VERSION%_%SAGA_CONFIG%_setup.exe"

RMDIR /S/Q "%SAGA_VERSION%_%SAGA_CONFIG%"

REM ___________________________________
REM x64 Binaries
SET SAGA_CONFIG=x64
MKDIR "%SAGA_VERSION%_%SAGA_CONFIG%"
PUSHD "%SAGA_VERSION%_%SAGA_CONFIG%"
XCOPY /C/S/Q/Y "%SAGA_ROOT%\saga-gis\bin\saga_vc_%SAGA_CONFIG%"
DEL /F saga_gui.cfg saga_gui.ini *.exp modules\*.exp modules\*.lib modules\dev_tools.*
RMDIR /S/Q _private
POPD
%ZIPEXE% "%SAGA_VERSION%_%SAGA_CONFIG%.zip" "%SAGA_VERSION%_%SAGA_CONFIG%"

COPY "%SAGA_ROOT%\saga_setup_readme.rtf" "%SAGA_VERSION%_%SAGA_CONFIG%"
COPY "%SAGA_ROOT%\saga_setup_%SAGA_CONFIG%.iss" "%SAGA_VERSION%_%SAGA_CONFIG%"
%ISETUP% "%SAGA_VERSION%_%SAGA_CONFIG%\saga_setup_%SAGA_CONFIG%.iss"
MOVE "%SAGA_VERSION%_%SAGA_CONFIG%\%SAGA_VERSION%_%SAGA_CONFIG%_setup.exe"

RMDIR /S/Q "%SAGA_VERSION%_%SAGA_CONFIG%"

REM ___________________________________
REM SVN Source Code Repository
%SVNEXE% checkout svn://svn.code.sf.net/p/saga-gis/code-0/trunk %SAGA_VERSION%_src -q --non-interactive
PUSHD %SAGA_VERSION%_src
RMDIR /S/Q .svn
POPD
%ZIPEXE% %SAGA_VERSION%_src.zip %SAGA_VERSION%_src

REM ___________________________________
REM Doxygen API Documentation
PUSHD %SAGA_VERSION%_src
%DOXEXE% saga_api_Doxyfile
POPD
%ZIPEXE% "%SAGA_VERSION%_api_doc.zip" "%SAGA_VERSION%_api_doc"
RMDIR /S/Q "%SAGA_VERSION%_api_doc"

REM ___________________________________
REM SWIG/Python (win32)
SET WXWINLIB="%WXWIN%\lib\vc_dll"
SET SAGA="%SAGA_ROOT%\saga-gis\bin\saga_vc_Win32"
PUSHD "%SAGA_ROOT%\saga-gis\src\saga_core\saga_api"
%SWIGEXE% -c++ -python -includeall -I. -D_SAGA_PYTHON -D_SAGA_UNICODE saga_api.h
"%PYTHONDIR%\python.exe" saga_api_to_python_win.py install
MOVE saga_api.py "%PYTHONDIR%\Lib\site-packages\saga_api.py"
DEL /F saga_api_wrap.cxx
RMDIR /S/Q build
POPD
SET PYTHONOUT=Python%PYTHONVER%
XCOPY /C/Q/Y "%PYTHONDIR%\Lib\site-packages\*saga*.*" "%PYTHONOUT%\Lib\site-packages\"
COPY "%SAGA_ROOT%\saga-gis\src\scripting\python\saga_python_api.txt" "%PYTHONOUT%\Lib\site-packages\"
XCOPY /C/Q/Y "%SAGA_ROOT%\saga-gis\src\scripting\python\examples" "%PYTHONOUT%\Lib\site-packages\saga_api_examples\"
%ZIPEXE% %SAGA_VERSION%_win32_python%PYTHONVER%.zip "%PYTHONOUT%"
RMDIR /S/Q "%PYTHONOUT%"

REM ___________________________________
REM The End
RMDIR /S/Q %SAGA_VERSION%_src
POPD


REM ___________________________________
REM ###################################
REM POST-RELEASE STEPS
REM ###################################

ECHO __________________________________
ECHO ##################################
ECHO #
ECHO # What is left to do ?!
ECHO #
ECHO ##################################
ECHO.
ECHO - Don't forget to make the Linux tarball!
ECHO.    make dist
ECHO.
ECHO - Upload all release files
ECHO.    including an up-to-date 'readme.txt'
ECHO.
ECHO - Upload API Documentation to saga-gis.org
ECHO.
ECHO - Update version numbers in:
ECHO.    ./saga_setup_win32.iss
ECHO.    ./saga_setup_x64.iss
ECHO.    ./saga_api_Doxyfile
ECHO.    ./saga-gis/configure.ac
ECHO.    ./saga-gis/README
ECHO.    ./saga-gis/src/saga_core/saga_api/saga_api.h
ECHO.    ./saga-gis/src/scripting/helper/make_saga_release.bat (this file!)
ECHO.
ECHO - Create SAGA Module Reference Documentation
ECHO.    sagadoc-code: ./parse_modules.py
ECHO.    upload created version folder to saga-gis.org and update link
ECHO.
ECHO - Add new bug tracker milestone for next version
ECHO.    https://sourceforge.net/p/saga-gis/bugs/milestones
ECHO.
ECHO - Commit a comment like: SAGA version updated to %SVN__VERSION%

PAUSE
