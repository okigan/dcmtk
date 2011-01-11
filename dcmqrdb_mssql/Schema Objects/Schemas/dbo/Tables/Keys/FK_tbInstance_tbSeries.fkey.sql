ALTER TABLE [dbo].[tbInstance]
    ADD CONSTRAINT [FK_tbInstance_tbSeries] FOREIGN KEY ([SeriesKey]) REFERENCES [dbo].[tbSeries] ([SeriesKey]) ON DELETE NO ACTION ON UPDATE NO ACTION;

