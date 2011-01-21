USE [dcmqrdb_mssql]
GO

IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[spFindDcmInstance]') AND type in (N'P', N'PC'))
DROP PROCEDURE [dbo].[spFindDcmInstance]
GO

IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[spFindMatchingAttibutes]') AND type in (N'P', N'PC'))
DROP PROCEDURE [dbo].[spFindMatchingAttibutes]
GO

IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[spGetInstanceAttributes]') AND type in (N'P', N'PC'))
DROP PROCEDURE [dbo].[spGetInstanceAttributes]
GO

IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[spRegisterDcmInstance]') AND type in (N'P', N'PC'))
DROP PROCEDURE [dbo].[spRegisterDcmInstance]
GO

IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[spRegisterDcmSeries]') AND type in (N'P', N'PC'))
DROP PROCEDURE [dbo].[spRegisterDcmSeries]
GO

IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[spRegisterDcmTag]') AND type in (N'P', N'PC'))
DROP PROCEDURE [dbo].[spRegisterDcmTag]
GO

IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[TVPTest]') AND type in (N'P', N'PC'))
DROP PROCEDURE [dbo].[TVPTest]
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
CREATE PROCEDURE [dbo].[spFindDcmInstance] 
	-- Add the parameters for the stored procedure here
	@tagList nvarchar(1024), 
	@valueList nvarchar(1024)
AS
BEGIN
	SET NOCOUNT ON;
	
	SELECT v.InstanceKey, v.InstanceUiid, v.FilePath, COUNT(v.AttributeKey) as MatchedAttributes
	FROM vwStudySeriesInstanceFileAttribute AS v 
	INNER JOIN
	dbo.SplitAndJoin(@tagList, @valueList, ',') AS q 
	ON v.AttributeTag = q.[Key] AND v.Value = q.Value
	GROUP BY v.InstanceKey, v.InstanceUiid, v.FilePath
	HAVING COUNT(v.AttributeKey) >= (SELECT COUNT(1) FROM  dbo.SplitAndJoin(@tagList, @valueList, ',') )
	ORDER BY MatchedAttributes DESC
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
CREATE PROCEDURE [dbo].[spFindMatchingAttibutes] 
	-- Add the parameters for the stored procedure here
	@tagList nvarchar(1024), 
	@valueList nvarchar(1024)
AS
BEGIN
	-- SET NOCOUNT ON added to prevent extra result sets from
	-- interfering with SELECT statements.
	SET NOCOUNT ON;

	SELECT v.InstanceKey, v.AttributeKey, v.InstanceUiid,  v.AttributeTag, v.Value
	FROM vwStudySeriesInstanceFileAttribute AS v 
	INNER JOIN
	dbo.SplitAndJoin(@tagList, @valueList, ',') AS q 
	ON v.AttributeTag = q.[Key] AND v.Value = q.Value

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
CREATE PROCEDURE [dbo].[spGetInstanceAttributes] 
	-- Add the parameters for the stored procedure here
	@instanceKey int
AS
BEGIN
	-- SET NOCOUNT ON added to prevent extra result sets from
	-- interfering with SELECT statements.
	SET NOCOUNT ON;

SELECT     v.*
FROM         vwAttributes AS v
WHERE     (InstanceKey = @instanceKey)
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
CREATE PROCEDURE [dbo].[spRegisterDcmInstance] 
	-- Add the parameters for the stored procedure here
	  @studyUiid nvarchar(64)
	, @seriesUiid nvarchar(64)
	, @instanceUiid nvarchar(64)
	, @fileName nvarchar(1024)
AS
BEGIN
	-- SET NOCOUNT ON added to prevent extra result sets from
	-- interfering with SELECT statements.
	SET NOCOUNT ON;

    DECLARE @fileKey INT;

    INSERT INTO dbo.tbFile (FilePath)
    VALUES (@fileName)
    
    SET @fileKey = SCOPE_IDENTITY();
	
	DECLARE @studyKey INT;
		
	SET @studyKey = (SELECT StudyKey FROM dbo.tbStudy WHERE StudyUiid = @studyUiid)

    IF @studyKey IS NULL
    BEGIN
		INSERT INTO dbo.tbStudy (StudyUiid)
		VALUES (@studyUiid);
		
		SET @StudyKey = SCOPE_IDENTITY();
    END

	DECLARE @seriesKey INT;
	
	SET @seriesKey = (SELECT SeriesKey FROM dbo.tbSeries WHERE StudyKey = @studyKey AND SeriesUiid = @seriesUiid) 
	
	IF @seriesKey IS NULL
	BEGIN
		INSERT INTO dbo.tbSeries (StudyKey, SeriesUiid)
		VALUES (@studyKey,@seriesUiid);
		
		SET @seriesKey =  SCOPE_IDENTITY();
    END
    
    DECLARE @instanceKey INT;

	SET @instanceKey = (SELECT InstanceKey FROM dbo.tbInstance WHERE SeriesKey = @seriesKey AND InstanceUiid = @instanceUiid) 
	
	IF @instanceKey IS NULL
	BEGIN
		INSERT INTO dbo.tbInstance (SeriesKey, InstanceUiid, FileKey)
		VALUES (@seriesKey,@instanceUiid, @FileKey);
		
		SET @instanceKey =  SCOPE_IDENTITY();
    END
    
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
CREATE PROCEDURE [dbo].[spRegisterDcmSeries] 
	-- Add the parameters for the stored procedure here
		@studyUiid nchar(64)
	,	@seriesUiid nchar(64)
AS
BEGIN
	-- SET NOCOUNT ON added to prevent extra result sets from
	-- interfering with SELECT statements.
	SET NOCOUNT ON;
	
	DECLARE @studyKey INT;
	
	SET @studyKey = (SELECT StudyKey FROM dbo.tbStudy WHERE StudyUiid = @studyUiid)

    IF @studyKey IS NULL
    BEGIN
		INSERT INTO dbo.tbStudy (StudyUiid)
		VALUES (@studyUiid);
		
		SET @StudyKey = SCOPE_IDENTITY();
    END

	DECLARE @seriesKey INT;
	
	SET @seriesKey = (SELECT SeriesKey FROM dbo.tbSeries WHERE StudyKey = @studyKey AND SeriesUiid = @seriesUiid) 
	
	IF @seriesKey IS NULL
	BEGIN
		INSERT INTO dbo.tbSeries (StudyKey, SeriesUiid)
		VALUES (@studyKey,@seriesUiid);
    END
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
CREATE PROCEDURE [dbo].[spRegisterDcmTag] 
	-- Add the parameters for the stored procedure here
	  @studyUiid nvarchar(64)
	, @instanceUiid nvarchar(64)
	, @AttributeTag int
	, @attributeValue nvarchar(1024)
AS
BEGIN
	-- SET NOCOUNT ON added to prevent extra result sets from
	-- interfering with SELECT statements.
	SET NOCOUNT ON;


	DECLARE @InstanceKey INT;
		
    SET @InstanceKey = (SELECT InstanceKey FROM dbo.vwStudySeriesIntance WHERE StudyUiid = @studyUiid AND InstanceUiid = @instanceUiid)
	
	INSERT INTO dbo.tbAttribute (InstanceKey, AttributeTag, Value)
	VALUES (@InstanceKey, @AttributeTag, @attributeValue)
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
CREATE PROCEDURE [dbo].[TVPTest] 
	-- Add the parameters for the stored procedure here
	@t myTVP READONLY
AS
BEGIN
	-- SET NOCOUNT ON added to prevent extra result sets from
	-- interfering with SELECT statements.
	SET NOCOUNT ON;

END

GO


