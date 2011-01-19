USE [dcmqrdb_mssql]
GO

IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Split]') AND type in (N'FN', N'IF', N'TF', N'FS', N'FT'))
DROP FUNCTION [dbo].[Split]
GO

IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[SplitAndJoin]') AND type in (N'FN', N'IF', N'TF', N'FS', N'FT'))
DROP FUNCTION [dbo].[SplitAndJoin]
GO

USE [dcmqrdb_mssql]
GO

SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO

-- =============================================
-- Author:		Igor Okulist
-- Create date: 
-- Description:	
-- =============================================
CREATE FUNCTION [dbo].[Split] 
(	
	-- Add the parameters for the function here
	@str nvarchar(MAX), 
	@delim char
)
RETURNS @TokenTable TABLE (Idx INT, Token nvarchar(128))
BEGIN
	DECLARE @index INT
	DECLARE @token NVARCHAR(128)
	DECLARE @pos INT
	DECLARE @nextPos INT
	DECLARE @strLen INT
	
	SET @index = 0
	SET @pos = 1
	SET @nextPos = charindex(@delim, @str)
	SET @strLen = LEN(@str)
	
	WHILE (@pos <= @strLen)  
	BEGIN
		SET @token = substring(@str, @Pos, @nextPos - @pos);
		
		INSERT INTO @TokenTable (Idx, Token) VALUES (@index, @token)
		SET @Pos = @NextPos + 1
		SET @NextPos = charindex(@delim, @str, @Pos)
		
		IF @nextPos = 0
		BEGIN
			SET @nextPos = @strLen + 1
		END
		
		SET @index = @index + 1
	END	
	RETURN
END

GO

SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO

-- =============================================
-- Author:		Igor Okulist
-- Create date: 
-- Description:	
-- =============================================
CREATE FUNCTION [dbo].[SplitAndJoin] 
(	
	-- Add the parameters for the function here
	@keys nvarchar(MAX), 
	@values nvarchar(MAX), 
	@delim char
)
RETURNS TABLE AS RETURN
(
SELECT tbKey.Idx AS Idx, tbKey.Token AS [Key], tbValue.Token AS [Value]
FROM 
[dcmqrdb_mssql].[dbo].[Split] (@keys, @delim) AS tbKey
JOIN 
[dcmqrdb_mssql].[dbo].[Split] (@values, @delim) AS tbValue
ON tbKey.Idx = tbValue.Idx

)
GO


