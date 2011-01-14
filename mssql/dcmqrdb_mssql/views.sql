USE [dcmqrdb_mssql]
GO


IF  EXISTS (SELECT * FROM sys.views WHERE object_id = OBJECT_ID(N'[dbo].[vwStudySeries]'))
DROP VIEW [dbo].[vwStudySeries]
GO


IF  EXISTS (SELECT * FROM sys.views WHERE object_id = OBJECT_ID(N'[dbo].[vwStudySeriesIntance]'))
DROP VIEW [dbo].[vwStudySeriesIntance]
GO

USE [dcmqrdb_mssql]
GO


SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO

CREATE VIEW [dbo].[vwStudySeries]
AS
SELECT dbo.tbStudy.StudyKey, dbo.tbStudy.StudyUiid, dbo.tbSeries.SeriesKey, dbo.tbSeries.SeriesUiid
FROM  dbo.tbStudy FULL OUTER JOIN
               dbo.tbSeries ON dbo.tbStudy.StudyKey = dbo.tbSeries.StudyKey
GO


SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO

CREATE VIEW [dbo].[vwStudySeriesIntance]
AS
SELECT dbo.tbStudy.StudyUiid, dbo.tbSeries.SeriesUiid, dbo.tbInstance.InstanceUiid
FROM  dbo.tbStudy FULL OUTER JOIN
               dbo.tbSeries ON dbo.tbStudy.StudyKey = dbo.tbSeries.StudyKey FULL OUTER JOIN
               dbo.tbInstance ON dbo.tbSeries.SeriesKey = dbo.tbInstance.SeriesKey
GO



