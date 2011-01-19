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


