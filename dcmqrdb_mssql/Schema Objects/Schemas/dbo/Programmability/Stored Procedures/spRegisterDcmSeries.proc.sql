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


