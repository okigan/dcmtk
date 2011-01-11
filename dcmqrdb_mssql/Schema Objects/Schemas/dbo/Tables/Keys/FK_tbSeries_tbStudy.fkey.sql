ALTER TABLE [dbo].[tbSeries]
    ADD CONSTRAINT [FK_tbSeries_tbStudy] FOREIGN KEY ([StudyKey]) REFERENCES [dbo].[tbStudy] ([StudyKey]) ON DELETE NO ACTION ON UPDATE NO ACTION;

