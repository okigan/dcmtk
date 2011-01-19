USE [dcmqrdb_mssql]
GO

IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[Split]') AND type in (N'FN', N'IF', N'TF', N'FS', N'FT'))
DROP FUNCTION [dbo].[Split]
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
RETURNS @TokenTable TABLE ([Token] nvarchar(128))
BEGIN
	DECLARE @token NVARCHAR(128)
	DECLARE @pos INT
	DECLARE @nextPos INT
	DECLARE @strLen INT
	
	SET @pos = 1
	SET @nextPos = charindex(@delim, @str)
	SET @strLen = LEN(@str)
	
	WHILE (@pos <= @strLen)  
	BEGIN
		SET @token = substring(@str, @Pos, @nextPos - @pos);
		
		INSERT INTO @TokenTable ( [Token]) VALUES (@token)
		SET @Pos = @NextPos + 1
		SET @NextPos = charindex(@delim, @str, @Pos)
		
		IF @nextPos = 0
		BEGIN
			SET @nextPos = @strLen + 1
		END
	END	
	RETURN
END

GO


