@ECHO off
SET hostname=localhost
SET instance=SQLEXPRESS
SET dbinstance=%hostname%\%instance%
ECHO instance: %dbinstance%
ECHO %CD%

FOR /F %%g in (bootstrap.txt) DO (
	sqlcmd -E -b -S %dbinstance% -i %%g -o %%g.log
	if errorlevel 1 (
		ECHO FAILed on %%g
	) else (
		ECHO Processed %%g
	)
)
