USE [dcmqrdb_mssql]
GO

IF  EXISTS (SELECT * FROM sys.views WHERE object_id = OBJECT_ID(N'[dbo].[vwAttributes]'))
DROP VIEW [dbo].[vwAttributes]
GO

IF  EXISTS (SELECT * FROM sys.views WHERE object_id = OBJECT_ID(N'[dbo].[vwStudySeries]'))
DROP VIEW [dbo].[vwStudySeries]
GO

IF  EXISTS (SELECT * FROM sys.views WHERE object_id = OBJECT_ID(N'[dbo].[vwStudySeriesInstanceFileAttribute]'))
DROP VIEW [dbo].[vwStudySeriesInstanceFileAttribute]
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

CREATE VIEW [dbo].[vwAttributes]
AS
SELECT     AttributeKey, InstanceKey, DcmGroup, DcmElement, Value
FROM         dbo.tbAttribute

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

CREATE VIEW [dbo].[vwStudySeriesInstanceFileAttribute]
AS
SELECT     dbo.tbStudy.StudyKey, dbo.tbSeries.SeriesKey, dbo.tbInstance.InstanceKey, dbo.tbFile.FileKey, dbo.tbAttribute.AttributeKey, dbo.tbStudy.StudyUiid, 
                      dbo.tbSeries.SeriesUiid, dbo.tbInstance.InstanceUiid, dbo.tbFile.FilePath, dbo.tbAttribute.DcmGroup, dbo.tbAttribute.DcmElement, dbo.tbAttribute.Value
FROM         dbo.tbFile INNER JOIN
                      dbo.tbInstance ON dbo.tbFile.FileKey = dbo.tbInstance.FileKey INNER JOIN
                      dbo.tbAttribute ON dbo.tbInstance.InstanceKey = dbo.tbAttribute.InstanceKey INNER JOIN
                      dbo.tbSeries ON dbo.tbInstance.SeriesKey = dbo.tbSeries.SeriesKey INNER JOIN
                      dbo.tbStudy ON dbo.tbSeries.StudyKey = dbo.tbStudy.StudyKey

GO

SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO

CREATE VIEW [dbo].[vwStudySeriesIntance]
AS
SELECT     dbo.tbStudy.StudyKey, dbo.tbSeries.SeriesKey, dbo.tbInstance.InstanceKey, dbo.tbStudy.StudyUiid, dbo.tbInstance.InstanceUiid, dbo.tbSeries.SeriesUiid
FROM         dbo.tbStudy FULL OUTER JOIN
                      dbo.tbSeries ON dbo.tbStudy.StudyKey = dbo.tbSeries.StudyKey FULL OUTER JOIN
                      dbo.tbInstance ON dbo.tbSeries.SeriesKey = dbo.tbInstance.SeriesKey

GO


