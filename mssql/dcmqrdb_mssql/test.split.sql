SELECT * FROM [dcmqrdb_mssql].[dbo].[Split] (
   'a,b'
  ,',')
GO

SELECT * FROM [dcmqrdb_mssql].[dbo].[Split] (
   'a,b,,'
  ,',')
GO

SELECT * FROM [dcmqrdb_mssql].[dbo].[Split] (
   ',,a,b,,'
  ,',')
GO


SELECT tbKey.Idx AS Idx, tbKey.Token AS [Key], tbValue.Token AS [Value]
FROM 
[dcmqrdb_mssql].[dbo].[Split] ('10,20,30',',') AS tbKey
JOIN 
[dcmqrdb_mssql].[dbo].[Split] ('a,b,c',',') AS tbValue
ON tbKey.Idx = tbValue.Idx
GO


select * from dbo.SplitAndJoin ('asdf,asdfasdf', 'n,m', ',')
GO

select * from 
dbo.SplitAndJoin ('10,20,30', 'a,b,c',',') AS a
JOIN
dbo.SplitAndJoin ('10,20,30', 'x,b,c',',') AS b
ON a.[Key] = b.[Key] AND a.[Value] = b.[Value]
GO

USE [dcmqrdb_mssql]
GO

DECLARE	@return_value int

EXEC	@return_value = [dbo].[spFindDcmInstance]
		@tagList = N'16,8',
		@valueList = N'F,20000216'

SELECT	'Return Value' = @return_value
