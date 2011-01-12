@ECHO off
SET hostname=localhost
SET instance=SQLEXPRESS
SET dbinstance=%hostname%\%instance%
ECHO instance: %dbinstance%
SET currdir=%CD%

sqlcmd -E -S %dbinstance% -i bootstrap.sql -o bootstrap.sql.log
