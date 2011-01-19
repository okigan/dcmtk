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


SELECT * 
FROM 
[dcmqrdb_mssql].[dbo].[Split] ('10,20,30',',') AS A
JOIN 
[dcmqrdb_mssql].[dbo].[Split] ('a,b,c',',') AS B
ON A.Idx = B.Idx
GO



