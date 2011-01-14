@ECHO off
SET hostname=localhost
SET instance=SQLEXPRESS
SET dbinstance=%hostname%\%instance%
ECHO instance: %dbinstance%
SET currdir=%CD%

SET sqlfile=bootstrap.sql
sqlcmd -E -S %dbinstance% -i %sqlfile% -o %sqlfile%.log

if errorlevel 1 (
	ECHO FAILed on %sqlfile%
) else (
	ECHO Processed %sqlfile%
)