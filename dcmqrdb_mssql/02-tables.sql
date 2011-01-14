USE [dcmqrdb_mssql]
GO

IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_tbInstance_tbSeries]') AND parent_object_id = OBJECT_ID(N'[dbo].[tbInstance]'))
ALTER TABLE [dbo].[tbInstance] DROP CONSTRAINT [FK_tbInstance_tbSeries]
GO

IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_tbSeries_tbStudy]') AND parent_object_id = OBJECT_ID(N'[dbo].[tbSeries]'))
ALTER TABLE [dbo].[tbSeries] DROP CONSTRAINT [FK_tbSeries_tbStudy]
GO

USE [dcmqrdb_mssql]
GO

/****** Object:  Table [dbo].[tbInstance]    Script Date: 01/12/2011 11:48:19 ******/
IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[tbInstance]') AND type in (N'U'))
DROP TABLE [dbo].[tbInstance]
GO

/****** Object:  Table [dbo].[tbSeries]    Script Date: 01/12/2011 11:48:19 ******/
IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[tbSeries]') AND type in (N'U'))
DROP TABLE [dbo].[tbSeries]
GO

/****** Object:  Table [dbo].[tbStudy]    Script Date: 01/12/2011 11:48:19 ******/
IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[tbStudy]') AND type in (N'U'))
DROP TABLE [dbo].[tbStudy]
GO

USE [dcmqrdb_mssql]
GO

/****** Object:  Table [dbo].[tbInstance]    Script Date: 01/12/2011 11:48:19 ******/
SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO

CREATE TABLE [dbo].[tbInstance](
	[InstanceKey] [int] IDENTITY(1,1) NOT NULL,
	[SeriesKey] [int] NOT NULL,
	[InstanceUiid] [nvarchar](64) NOT NULL,
 CONSTRAINT [PK_tbInstance] PRIMARY KEY CLUSTERED 
(
	[InstanceKey] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]

GO

USE [dcmqrdb_mssql]
GO

/****** Object:  Table [dbo].[tbSeries]    Script Date: 01/12/2011 11:48:19 ******/
SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO

CREATE TABLE [dbo].[tbSeries](
	[SeriesKey] [int] IDENTITY(1,1) NOT NULL,
	[StudyKey] [int] NOT NULL,
	[SeriesUiid] [nvarchar](64) NOT NULL,
 CONSTRAINT [PK_tbSeries] PRIMARY KEY CLUSTERED 
(
	[SeriesKey] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]

GO

USE [dcmqrdb_mssql]
GO

/****** Object:  Table [dbo].[tbStudy]    Script Date: 01/12/2011 11:48:19 ******/
SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO

CREATE TABLE [dbo].[tbStudy](
	[StudyKey] [int] IDENTITY(1,1) NOT NULL,
	[StudyUiid] [nvarchar](64) NOT NULL,
 CONSTRAINT [PK_tbStudy] PRIMARY KEY CLUSTERED 
(
	[StudyKey] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY]

GO

ALTER TABLE [dbo].[tbInstance]  WITH CHECK ADD  CONSTRAINT [FK_tbInstance_tbSeries] FOREIGN KEY([SeriesKey])
REFERENCES [dbo].[tbSeries] ([SeriesKey])
GO

ALTER TABLE [dbo].[tbInstance] CHECK CONSTRAINT [FK_tbInstance_tbSeries]
GO

ALTER TABLE [dbo].[tbSeries]  WITH CHECK ADD  CONSTRAINT [FK_tbSeries_tbStudy] FOREIGN KEY([StudyKey])
REFERENCES [dbo].[tbStudy] ([StudyKey])
GO

ALTER TABLE [dbo].[tbSeries] CHECK CONSTRAINT [FK_tbSeries_tbStudy]
GO


