CREATE TABLE [dbo].[tbInstance] (
    [InstanceKey]  INT           IDENTITY (1, 1) NOT NULL,
    [SeriesKey]    INT           NOT NULL,
    [InstanceUiid] NVARCHAR (64) NOT NULL
);

