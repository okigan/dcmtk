CREATE TABLE [dbo].[tbSeries] (
    [SeriesKey]  INT           IDENTITY (1, 1) NOT NULL,
    [StudyKey]   INT           NOT NULL,
    [SeriesUiid] NVARCHAR (64) NOT NULL
);

