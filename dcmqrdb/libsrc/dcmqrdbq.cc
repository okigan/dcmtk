/*
 *
 *  Copyright (C) 1993-2010, OFFIS e.V.
 *  All rights reserved.  See COPYRIGHT file for details.
 *
 *  This software and supporting documentation were developed by
 *
 *    OFFIS e.V.
 *    R&D Division Health
 *    Escherweg 2
 *    D-26121 Oldenburg, Germany
 *
 *
 *  Module:  dcmqrdb
 *
 *  Author:  Marco Eichelberg
 *
 *  Purpose: classes DcmQueryRetrieveSQLDatabaseHandle,
 *                   DcmQueryRetrieveSQLDatabaseHandleFactory
 *
 *  Last Update:      $Author: $
 *  Update Date:      $Date: 2010-11-05 10:29:55 $
 *  CVS/RCS Revision: $Revision: 1.29 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */

#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */

BEGIN_EXTERN_C
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
END_EXTERN_C

#define INCLUDE_CCTYPE
#define INCLUDE_CSTDARG
#include "dcmtk/ofstd/ofstdinc.h"

#include "dcmtk/dcmqrdb/dcmqrdbs.h"
#include "dcmtk/dcmqrdb/dcmqrdbq.h"
#include "dcmtk/dcmqrdb/dcmqrcnf.h"

#include "dcmtk/dcmqrdb/dcmqridx.h"
#include "dcmtk/dcmnet/diutil.h"
#include "dcmtk/dcmdata/dcfilefo.h"
#include "dcmtk/ofstd/ofstd.h"

#ifdef WITH_SQL_DATABASE
//TODO: move these of this file

#include <atlbase.h>
#include "dblib/dblib.h"
#endif


const OFConditionConst DcmQRSqlDatabaseErrorC(OFM_imagectn, 0x001, OF_error, "DcmQR Index Database Error");
const OFCondition DcmQRSqlDatabaseError(DcmQRSqlDatabaseErrorC);

/* ========================= static data ========================= */

/**** The TbFindAttr table contains the description of tags (keys) supported
 **** by the DB Module.
 **** Tags described here have to be present in the Index Record file.
 **** The order is unsignificant.
 ****
 **** Each element of this table is described by
 ****           The tag value
 ****           The level of this tag (from patient to image)
 ****           The Key Type (only UNIQUE_KEY values is used)
 ****           The key matching type, specifiing which type of
 ****                   matching should be performed. The OTHER_CLASS
 ****                   value specifies that only strict comparison is applied.
 ****
 **** This table and the IndexRecord structure should contain at least
 **** all Unique and Required keys.
 ***/

static const DB_FindAttr TbFindAttr [] = {
        DB_FindAttr( DCM_PatientBirthDate,                      PATIENT_LEVEL,  OPTIONAL_KEY,   DATE_CLASS      ),
        DB_FindAttr( DCM_PatientSex,                            PATIENT_LEVEL,  OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_PatientName,                           PATIENT_LEVEL,  REQUIRED_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_PatientID,                             PATIENT_LEVEL,  UNIQUE_KEY,     STRING_CLASS    ),
        DB_FindAttr( DCM_PatientBirthTime,                      PATIENT_LEVEL,  OPTIONAL_KEY,   TIME_CLASS      ),
        DB_FindAttr( DCM_OtherPatientIDs,                       PATIENT_LEVEL,  OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_OtherPatientNames,                     PATIENT_LEVEL,  OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_EthnicGroup,                           PATIENT_LEVEL,  OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_PatientComments,                       PATIENT_LEVEL,  OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_NumberOfPatientRelatedStudies,         PATIENT_LEVEL,  OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_NumberOfPatientRelatedSeries,          PATIENT_LEVEL,  OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_NumberOfPatientRelatedInstances,       PATIENT_LEVEL,  OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_StudyDate,                             STUDY_LEVEL,    REQUIRED_KEY,   DATE_CLASS      ),
        DB_FindAttr( DCM_StudyTime,                             STUDY_LEVEL,    REQUIRED_KEY,   TIME_CLASS      ),
        DB_FindAttr( DCM_StudyID,                               STUDY_LEVEL,    REQUIRED_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_AccessionNumber,                       STUDY_LEVEL,    REQUIRED_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_ReferringPhysicianName,                STUDY_LEVEL,    OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_StudyDescription,                      STUDY_LEVEL,    OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_NameOfPhysiciansReadingStudy,          STUDY_LEVEL,    OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_StudyInstanceUID,                      STUDY_LEVEL,    UNIQUE_KEY,     UID_CLASS       ),
        DB_FindAttr( DCM_RETIRED_OtherStudyNumbers,             STUDY_LEVEL,    OPTIONAL_KEY,   OTHER_CLASS     ),
        DB_FindAttr( DCM_AdmittingDiagnosesDescription,         STUDY_LEVEL,    OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_PatientAge,                            STUDY_LEVEL,    OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_PatientSize,                           STUDY_LEVEL,    OPTIONAL_KEY,   OTHER_CLASS     ),
        DB_FindAttr( DCM_PatientWeight,                         STUDY_LEVEL,    OPTIONAL_KEY,   OTHER_CLASS     ),
        DB_FindAttr( DCM_Occupation,                            STUDY_LEVEL,    OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_AdditionalPatientHistory,              STUDY_LEVEL,    OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_NumberOfStudyRelatedSeries,            STUDY_LEVEL,    OPTIONAL_KEY,   OTHER_CLASS     ),
        DB_FindAttr( DCM_NumberOfStudyRelatedInstances,         STUDY_LEVEL,    OPTIONAL_KEY,   OTHER_CLASS     ),
        DB_FindAttr( DCM_SeriesNumber,                          SERIE_LEVEL,    REQUIRED_KEY,   OTHER_CLASS     ),
        DB_FindAttr( DCM_SeriesInstanceUID,                     SERIE_LEVEL,    UNIQUE_KEY,     UID_CLASS       ),
        DB_FindAttr( DCM_Modality,                              SERIE_LEVEL,    OPTIONAL_KEY,   STRING_CLASS    ),
        DB_FindAttr( DCM_InstanceNumber,                        IMAGE_LEVEL,    REQUIRED_KEY,   OTHER_CLASS     ),
        DB_FindAttr( DCM_SOPInstanceUID,                        IMAGE_LEVEL,    UNIQUE_KEY,     UID_CLASS       )
  };

/**** The NbFindAttr variable contains the length of the TbFindAttr table
 ***/

static int NbFindAttr = ((sizeof (TbFindAttr)) / (sizeof (TbFindAttr [0])));

/* ========================= static functions ========================= */
//TODO: remove ?
static char *DB_strdup(const char* str)
{
    if (str == NULL) return NULL;
    char* s = (char*)malloc(strlen(str)+1);
    strcpy(s, str);
    return s;
}

static DB_UidList* DB_UIDAddFound (
                DB_UidList *uidList,
                DB_LEVEL queryLevel, 
                IdxRecord *idxRec
                )
{
    DB_UidList *plist ;

    plist = (DB_UidList *) malloc (sizeof (DB_UidList)) ;
    if (plist == NULL) {
        DCMQRDB_ERROR("DB_UIDAddFound: out of memory");
        return NULL;
    }
    plist->next = uidList ;
    plist->patient = NULL ;
    plist->study = NULL ;
    plist->serie = NULL ;
    plist->image = NULL ;

    if ((int)queryLevel >= PATIENT_LEVEL)
        plist->patient = DB_strdup ((char *) idxRec->PatientID) ;
    if ((int)queryLevel >= STUDY_LEVEL)
        plist->study = DB_strdup ((char *) idxRec->StudyInstanceUID) ;
    if ((int)queryLevel >= SERIE_LEVEL)
        plist->serie = DB_strdup ((char *) idxRec->SeriesInstanceUID) ;
    if ((int)queryLevel >= IMAGE_LEVEL)
        plist->image = DB_strdup ((char *) idxRec->SOPInstanceUID) ;

    return plist ;
}

/************
 *      Initializes addresses in an IdxRecord
 */

static void DB_IdxInitRecord (IdxRecord *idx, int linksOnly)
{
    if (! linksOnly)
    {
        idx -> param[RECORDIDX_PatientBirthDate]. XTag = DCM_PatientBirthDate ;
        idx -> param[RECORDIDX_PatientBirthDate]. ValueLength = DA_MAX_LENGTH ;
        idx -> PatientBirthDate[0] = '\0' ;
        idx -> param[RECORDIDX_PatientSex]. XTag = DCM_PatientSex ;
        idx -> param[RECORDIDX_PatientSex]. ValueLength = CS_MAX_LENGTH ;
        idx -> PatientSex[0] = '\0' ;
        idx -> param[RECORDIDX_PatientName]. XTag = DCM_PatientName ;
        idx -> param[RECORDIDX_PatientName]. ValueLength = PN_MAX_LENGTH ;
        idx -> PatientName[0] = '\0' ;
        idx -> param[RECORDIDX_PatientID]. XTag = DCM_PatientID ;
        idx -> param[RECORDIDX_PatientID]. ValueLength = LO_MAX_LENGTH ;
        idx -> PatientID[0] = '\0' ;
        idx -> param[RECORDIDX_PatientBirthTime]. XTag = DCM_PatientBirthTime ;
        idx -> param[RECORDIDX_PatientBirthTime]. ValueLength = TM_MAX_LENGTH ;
        idx -> PatientBirthTime[0] = '\0' ;
        idx -> param[RECORDIDX_OtherPatientIDs]. XTag = DCM_OtherPatientIDs ;
        idx -> param[RECORDIDX_OtherPatientIDs]. ValueLength = LO_MAX_LENGTH ;
        idx -> OtherPatientIDs[0] = '\0' ;
        idx -> param[RECORDIDX_OtherPatientNames]. XTag = DCM_OtherPatientNames ;
        idx -> param[RECORDIDX_OtherPatientNames]. ValueLength = PN_MAX_LENGTH ;
        idx -> OtherPatientNames[0] = '\0' ;
        idx -> param[RECORDIDX_EthnicGroup]. XTag = DCM_EthnicGroup ;
        idx -> param[RECORDIDX_EthnicGroup]. ValueLength = SH_MAX_LENGTH ;
        idx -> EthnicGroup[0] = '\0' ;
        idx -> param[RECORDIDX_NumberofPatientRelatedStudies]. XTag = DCM_NumberOfPatientRelatedStudies ;
        idx -> param[RECORDIDX_NumberofPatientRelatedStudies]. ValueLength = IS_MAX_LENGTH ;
        idx -> NumberofPatientRelatedStudies[0] = '\0' ;
        idx -> param[RECORDIDX_NumberofPatientRelatedSeries]. XTag = DCM_NumberOfPatientRelatedSeries ;
        idx -> param[RECORDIDX_NumberofPatientRelatedSeries]. ValueLength = IS_MAX_LENGTH ;
        idx -> NumberofPatientRelatedSeries[0] = '\0' ;
        idx -> param[RECORDIDX_NumberofPatientRelatedInstances]. XTag = DCM_NumberOfPatientRelatedInstances ;
        idx -> param[RECORDIDX_NumberofPatientRelatedInstances]. ValueLength = IS_MAX_LENGTH ;
        idx -> NumberofPatientRelatedInstances[0] = '\0' ;
        idx -> param[RECORDIDX_StudyDate]. XTag = DCM_StudyDate ;
        idx -> param[RECORDIDX_StudyDate]. ValueLength = DA_MAX_LENGTH ;
        idx -> StudyDate[0] = '\0' ;
        idx -> param[RECORDIDX_StudyTime]. XTag = DCM_StudyTime ;
        idx -> param[RECORDIDX_StudyTime]. ValueLength = TM_MAX_LENGTH ;
        idx -> StudyTime[0] = '\0' ;
        idx -> param[RECORDIDX_StudyID]. XTag = DCM_StudyID ;
        idx -> param[RECORDIDX_StudyID]. ValueLength = CS_MAX_LENGTH ;
        idx -> StudyID[0] = '\0' ;
        idx -> param[RECORDIDX_StudyDescription]. XTag = DCM_StudyDescription ;
        idx -> param[RECORDIDX_StudyDescription]. ValueLength = LO_MAX_LENGTH ;
        idx -> StudyDescription[0] = '\0' ;
        idx -> param[RECORDIDX_NameOfPhysiciansReadingStudy]. XTag = DCM_NameOfPhysiciansReadingStudy ;
        idx -> param[RECORDIDX_NameOfPhysiciansReadingStudy]. ValueLength = PN_MAX_LENGTH ;
        idx -> NameOfPhysiciansReadingStudy[0] = '\0' ;
        idx -> param[RECORDIDX_AccessionNumber]. XTag = DCM_AccessionNumber ;
        idx -> param[RECORDIDX_AccessionNumber]. ValueLength = CS_MAX_LENGTH ;
        idx -> AccessionNumber[0] = '\0' ;
        idx -> param[RECORDIDX_ReferringPhysicianName]. XTag = DCM_ReferringPhysicianName ;
        idx -> param[RECORDIDX_ReferringPhysicianName]. ValueLength = PN_MAX_LENGTH ;
        idx -> ReferringPhysicianName[0] = '\0' ;
        idx -> param[RECORDIDX_ProcedureDescription]. XTag = DCM_StudyDescription ;
        idx -> param[RECORDIDX_ProcedureDescription]. ValueLength = LO_MAX_LENGTH ;
        idx -> ProcedureDescription[0] = '\0' ;
        idx -> param[RECORDIDX_AttendingPhysiciansName]. XTag = DCM_NameOfPhysiciansReadingStudy ;
        idx -> param[RECORDIDX_AttendingPhysiciansName]. ValueLength = PN_MAX_LENGTH ;
        idx -> AttendingPhysiciansName[0] = '\0' ;
        idx -> param[RECORDIDX_StudyInstanceUID]. XTag = DCM_StudyInstanceUID ;
        idx -> param[RECORDIDX_StudyInstanceUID]. ValueLength = UI_MAX_LENGTH ;
        idx -> StudyInstanceUID[0] = '\0' ;
        idx -> param[RECORDIDX_OtherStudyNumbers]. XTag = DCM_RETIRED_OtherStudyNumbers ;
        idx -> param[RECORDIDX_OtherStudyNumbers]. ValueLength = IS_MAX_LENGTH ;
        idx -> OtherStudyNumbers[0] = '\0' ;
        idx -> param[RECORDIDX_AdmittingDiagnosesDescription]. XTag = DCM_AdmittingDiagnosesDescription ;
        idx -> param[RECORDIDX_AdmittingDiagnosesDescription]. ValueLength = LO_MAX_LENGTH ;
        idx -> AdmittingDiagnosesDescription[0] = '\0' ;
        idx -> param[RECORDIDX_PatientAge]. XTag = DCM_PatientAge ;
        idx -> param[RECORDIDX_PatientAge]. ValueLength = AS_MAX_LENGTH ;
        idx -> PatientAge[0] = '\0' ;
        idx -> param[RECORDIDX_PatientSize]. XTag = DCM_PatientSize ;
        idx -> param[RECORDIDX_PatientSize]. ValueLength = DS_MAX_LENGTH ;
        idx -> PatientSize[0] = '\0' ;
        idx -> param[RECORDIDX_PatientWeight]. XTag = DCM_PatientWeight ;
        idx -> param[RECORDIDX_PatientWeight]. ValueLength = DS_MAX_LENGTH ;
        idx -> PatientWeight[0] = '\0' ;
        idx -> param[RECORDIDX_Occupation]. XTag = DCM_Occupation ;
        idx -> param[RECORDIDX_Occupation]. ValueLength = SH_MAX_LENGTH ;
        idx -> Occupation[0] = '\0' ;
        idx -> param[RECORDIDX_NumberofStudyRelatedSeries]. XTag = DCM_NumberOfStudyRelatedSeries ;
        idx -> param[RECORDIDX_NumberofStudyRelatedSeries]. ValueLength = IS_MAX_LENGTH ;
        idx -> NumberofStudyRelatedSeries[0] = '\0' ;
        idx -> param[RECORDIDX_NumberofStudyRelatedInstances]. XTag = DCM_NumberOfStudyRelatedInstances ;
        idx -> param[RECORDIDX_NumberofStudyRelatedInstances]. ValueLength = IS_MAX_LENGTH ;
        idx -> NumberofStudyRelatedInstances[0] = '\0' ;
        idx -> param[RECORDIDX_SeriesNumber]. XTag = DCM_SeriesNumber ;
        idx -> param[RECORDIDX_SeriesNumber]. ValueLength = IS_MAX_LENGTH ;
        idx -> SeriesNumber[0] = '\0' ;
        idx -> param[RECORDIDX_SeriesInstanceUID]. XTag = DCM_SeriesInstanceUID ;
        idx -> param[RECORDIDX_SeriesInstanceUID]. ValueLength = UI_MAX_LENGTH ;
        idx -> SeriesInstanceUID[0] = '\0' ;
        idx -> param[RECORDIDX_Modality]. XTag = DCM_Modality ;
        idx -> param[RECORDIDX_Modality]. ValueLength = CS_MAX_LENGTH ;
        idx -> ImageNumber[0] = '\0' ;
        idx -> param[RECORDIDX_ImageNumber]. XTag = DCM_InstanceNumber ;
        idx -> param[RECORDIDX_ImageNumber]. ValueLength = IS_MAX_LENGTH ;
        idx -> ImageNumber[0] = '\0' ;
        idx -> param[RECORDIDX_SOPInstanceUID]. XTag = DCM_SOPInstanceUID ;
        idx -> param[RECORDIDX_SOPInstanceUID]. ValueLength = UI_MAX_LENGTH ;
        idx -> SOPInstanceUID[0] = '\0' ;
        idx -> param[RECORDIDX_SeriesDate]. XTag = DCM_SeriesDate ;
        idx -> param[RECORDIDX_SeriesDate]. ValueLength = DA_MAX_LENGTH ;
        idx -> SeriesDate[0] = '\0' ;
        idx -> param[RECORDIDX_SeriesTime]. XTag = DCM_SeriesTime ;
        idx -> param[RECORDIDX_SeriesTime]. ValueLength = TM_MAX_LENGTH ;
        idx -> SeriesTime[0] = '\0' ;
        idx -> param[RECORDIDX_SeriesDescription]. XTag = DCM_SeriesDescription ;
        idx -> param[RECORDIDX_SeriesDescription]. ValueLength = LO_MAX_LENGTH ;
        idx -> SeriesDescription[0] = '\0' ;
        idx -> param[RECORDIDX_ProtocolName]. XTag = DCM_ProtocolName ;
        idx -> param[RECORDIDX_ProtocolName]. ValueLength = LO_MAX_LENGTH ;
        idx -> ProtocolName[0] = '\0' ;
        idx -> param[RECORDIDX_OperatorsName ]. XTag = DCM_OperatorsName ;
        idx -> param[RECORDIDX_OperatorsName ]. ValueLength = PN_MAX_LENGTH ;
        idx -> OperatorsName[0] = '\0' ;
        idx -> param[RECORDIDX_PerformingPhysicianName]. XTag = DCM_PerformingPhysicianName ;
        idx -> param[RECORDIDX_PerformingPhysicianName]. ValueLength = PN_MAX_LENGTH ;
        idx -> PerformingPhysicianName[0] = '\0' ;
        idx -> param[RECORDIDX_PresentationLabel]. XTag = DCM_ContentLabel ;
        idx -> param[RECORDIDX_PresentationLabel]. ValueLength = CS_LABEL_MAX_LENGTH ;
        idx -> PresentationLabel[0] = '\0' ;
    }
    idx -> param[RECORDIDX_PatientBirthDate]. PValueField = (char *)idx -> PatientBirthDate ;
    idx -> param[RECORDIDX_PatientSex]. PValueField = (char *)idx -> PatientSex ;
    idx -> param[RECORDIDX_PatientName]. PValueField = (char *)idx -> PatientName ;
    idx -> param[RECORDIDX_PatientID]. PValueField = (char *)idx -> PatientID ;
    idx -> param[RECORDIDX_PatientBirthTime]. PValueField = (char *)idx -> PatientBirthTime ;
    idx -> param[RECORDIDX_OtherPatientIDs]. PValueField = (char *)idx -> OtherPatientIDs ;
    idx -> param[RECORDIDX_OtherPatientNames]. PValueField = (char *)idx -> OtherPatientNames ;
    idx -> param[RECORDIDX_EthnicGroup]. PValueField = (char *)idx -> EthnicGroup ;
    idx -> param[RECORDIDX_NumberofPatientRelatedStudies]. PValueField = (char *)idx -> NumberofPatientRelatedStudies ;
    idx -> param[RECORDIDX_NumberofPatientRelatedSeries]. PValueField = (char *) idx -> NumberofPatientRelatedSeries ;
    idx -> param[RECORDIDX_NumberofPatientRelatedInstances]. PValueField = (char *) idx -> NumberofPatientRelatedInstances ;
    idx -> param[RECORDIDX_StudyDate]. PValueField = (char *) idx -> StudyDate ;
    idx -> param[RECORDIDX_StudyTime]. PValueField = (char *) idx -> StudyTime ;
    idx -> param[RECORDIDX_StudyID]. PValueField = (char *) idx -> StudyID ;
    idx -> param[RECORDIDX_StudyDescription]. PValueField = (char *) idx -> StudyDescription ;
    idx -> param[RECORDIDX_NameOfPhysiciansReadingStudy]. PValueField = (char *) idx ->NameOfPhysiciansReadingStudy;
    idx -> param[RECORDIDX_AccessionNumber]. PValueField = (char *) idx -> AccessionNumber ;
    idx -> param[RECORDIDX_ReferringPhysicianName]. PValueField = (char *) idx -> ReferringPhysicianName ;
    idx -> param[RECORDIDX_ProcedureDescription]. PValueField = (char *) idx -> ProcedureDescription ;
    idx -> param[RECORDIDX_AttendingPhysiciansName]. PValueField = (char *) idx -> AttendingPhysiciansName ;
    idx -> param[RECORDIDX_StudyInstanceUID]. PValueField = (char *) idx -> StudyInstanceUID ;
    idx -> param[RECORDIDX_OtherStudyNumbers]. PValueField = (char *) idx -> OtherStudyNumbers ;
    idx -> param[RECORDIDX_AdmittingDiagnosesDescription]. PValueField = (char *) idx -> AdmittingDiagnosesDescription ;
    idx -> param[RECORDIDX_PatientAge]. PValueField = (char *) idx -> PatientAge ;
    idx -> param[RECORDIDX_PatientSize]. PValueField = (char *) idx -> PatientSize ;
    idx -> param[RECORDIDX_PatientWeight]. PValueField = (char *) idx -> PatientWeight ;
    idx -> param[RECORDIDX_Occupation]. PValueField = (char *) idx -> Occupation ;
    idx -> param[RECORDIDX_NumberofStudyRelatedSeries]. PValueField = (char *) idx -> NumberofStudyRelatedSeries ;
    idx -> param[RECORDIDX_NumberofStudyRelatedInstances]. PValueField = (char *) idx -> NumberofStudyRelatedInstances ;
    idx -> param[RECORDIDX_SeriesNumber]. PValueField = (char *) idx -> SeriesNumber ;
    idx -> param[RECORDIDX_SeriesInstanceUID]. PValueField = (char *) idx -> SeriesInstanceUID ;
    idx -> param[RECORDIDX_Modality]. PValueField = (char *) idx -> Modality ;
    idx -> param[RECORDIDX_ImageNumber]. PValueField = (char *) idx -> ImageNumber ;
    idx -> param[RECORDIDX_SOPInstanceUID]. PValueField = (char *) idx -> SOPInstanceUID ;
    idx -> param[RECORDIDX_SeriesDate]. PValueField = (char *) idx -> SeriesDate ;
    idx -> param[RECORDIDX_SeriesTime]. PValueField = (char *) idx -> SeriesTime ;
    idx -> param[RECORDIDX_SeriesDescription]. PValueField = (char *) idx -> SeriesDescription ;
    idx -> param[RECORDIDX_ProtocolName]. PValueField = (char *) idx -> ProtocolName ;
    idx -> param[RECORDIDX_OperatorsName ]. PValueField = (char *) idx -> OperatorsName ;
    idx -> param[RECORDIDX_PerformingPhysicianName]. PValueField = (char *) idx -> PerformingPhysicianName ;
    idx -> param[RECORDIDX_PresentationLabel]. PValueField = (char *) idx -> PresentationLabel ;
}

/*******************
 *    Free an element List
 */
//TODO: remove ?
static OFCondition DB_FreeUidList (DB_UidList *lst)
{
    if (lst == NULL) return EC_Normal;

    OFCondition cond = DB_FreeUidList (lst -> next);
    if (lst -> patient)
        free (lst -> patient);
    if (lst -> study)
        free (lst -> study);
    if (lst -> serie)
        free (lst -> serie);
    if (lst -> image)
        free (lst -> image);
    free (lst);
    return (cond);
}


/*******************
 *    Free a UID List
 */
//TODO: remove?
static OFCondition DB_FreeElementList (DB_ElementList *lst)
{
    if (lst == NULL) return EC_Normal;

    OFCondition cond = DB_FreeElementList (lst -> next);
    if (lst->elem.PValueField != NULL) {
        free ((char *) lst -> elem. PValueField);
    }
    free (lst);
    return (cond);
}


/*******************
 *    Matches two strings
 */
//TODO: remove?
static int DB_StringUnify  (char *pmod, char *pstr)
{
    int uni;

    if (*pmod == '\0')
    return (*pstr == '\0');

    if (  *pmod == *pstr
      || (*pmod == '?' && *pstr != '\0')
    )
    return (DB_StringUnify (pmod + 1, pstr + 1));

    if (*pmod == '*') {
    if ( *(pmod + 1) == '\0' )
        return (OFTrue);
    while (  ( (uni = DB_StringUnify (pmod + 1, pstr)) == OFFalse )
         && (*pstr != '\0')
        )
        pstr++;
    return (uni);
    }
    else if (*pmod != *pstr)
    return (OFFalse);
    return OFFalse;
}

/*******************
 *    Is the specified tag supported
 */
//TODO:remove?
static int DB_TagSupported (DcmTagKey tag)
{
    int i;

    for (i = 0; i < NbFindAttr; i++)
    if (TbFindAttr[i]. tag == tag)
        return (OFTrue);

    return (OFFalse);

}

#if 1
/*******************
 *    Get UID tag of a specified level
 */

static OFCondition DB_GetUIDTag (DB_LEVEL level, DcmTagKey *tag)
{
    int i;

    for (i = 0; i < NbFindAttr; i++){
        if ((TbFindAttr[i]. level == level) && (TbFindAttr[i]. keyAttr == UNIQUE_KEY))
            break;
    }

    if (i < NbFindAttr) {
        *tag = TbFindAttr[i].tag;
        return (EC_Normal);
    }
    else{
        return (DcmQRSqlDatabaseError);
    }

}
#endif 
/*******************
 *    Get tag level of a specified tag
 */
#if 1
static OFCondition DB_GetTagLevel (DcmTagKey tag, DB_LEVEL *level)
{
    int i;

    for (i = 0; i < NbFindAttr; i++)
    if (TbFindAttr[i]. tag == tag)
        break;

    if (i < NbFindAttr) {
        *level = TbFindAttr[i]. level;
        return (EC_Normal);
    }
    else
    return (DcmQRSqlDatabaseError);
}

/*******************
 *    Get tag key attribute of a specified tag
 */

static OFCondition DB_GetTagKeyAttr (DcmTagKey tag, DB_KEY_TYPE *keyAttr)
{
    int i;

    for (i = 0; i < NbFindAttr; i++)
    if (TbFindAttr[i]. tag == tag)
        break;

    if (i < NbFindAttr) {
        *keyAttr = TbFindAttr[i]. keyAttr;
        return (EC_Normal);
    }
    else
    return (DcmQRSqlDatabaseError);
}

/*******************
 *    Get tag key attribute of a specified tag
 */

static OFCondition DB_GetTagKeyClass (DcmTagKey tag, DB_KEY_CLASS *keyAttr)
{
    int i;

    for (i = 0; i < NbFindAttr; i++)
    if (TbFindAttr[i]. tag == tag)
        break;

    if (i < NbFindAttr) {
        *keyAttr = TbFindAttr[i]. keyClass;
        return (EC_Normal);
    }
    else
    return (DcmQRSqlDatabaseError);
}


/*******************
 *    Remove spaces in a string
 */

static void DB_RemoveSpaces (char *string)
{
    char *pc1, *pc2;

    for (pc1 = pc2 = string; *pc2; pc2++) {
        if (*pc2 != ' ') {
            *pc1 = *pc2;
            pc1++;
        }
    }
    *pc1 = '\0';
}

/*******************
 *    Remove leading and trailing spaces in a string
 */

static void DB_RemoveEnclosingSpaces (char *string)
{
    char *pc1, *pc2;

    /** Find in pc2 the first non space character
    ** If not found, string is empty
    */

    for (pc2 = string; (*pc2 != '\0') && (*pc2 == ' '); pc2++);
    if (*pc2 == '\0') {
        string [0] = '\0';
        return;
    }

    /** Shift the string if necessary
     */

    if (pc2 != string) {
        for (pc1 = string; *pc2; pc1++, pc2++)
            *pc1 = *pc2;
        *pc1 = '\0';
    }

    /** Ship trailing spaces
     */

    for (pc2 = string + strlen (string) - 1; *pc2 == ' '; pc2--);
        pc2++;
    *pc2 = '\0';
}


/*******************
 *    Convert a date YYYYMMDD in a long
 */
//TODO: remove?
static long DB_DateToLong (char *date)
{
    char year [5];
    char month[3];
    char day  [3];

    strncpy (year, date, 4);
    year [4] = '\0';
    strncpy (month, date + 4, 2);
    month [2] = '\0';
    strncpy (day, date + 6, 2);
    day [2] = '\0';

    return ((atol(year) * 10000) + (atol(month) * 100) + atol(day));
}


/*******************
 *    Convert a time in a double
 */

static double DB_TimeToDouble (char *thetime)
{
    char t [20];
    char tmp [4];

    double result = 0.;
    char *pc;

    /*** Get fractionnal part if exists
    **/

    strcpy (t, thetime);
    if ((pc = strchr (t, '.')) != NULL) {
        double f;

        *pc = '\0';
        for (pc++, f = 1.; (*pc) && (isdigit (OFstatic_cast(unsigned char, *pc))); pc++) {
            f /= 10.;
            result += (*pc - '0') * f;
        }
    }

    /*** Add default values (mm ss) if necessary
    **/

    strcat (t, "0000");
    t [6] = '\0';

    /*** Get Hours, Minutes and Seconds
    **/

    strncpy (tmp, t, 2);
    tmp [3] = '\0';
    result += 3600. * OFStandard::atof(tmp);

    strncpy (tmp, t + 2, 2);
    tmp [3] = '\0';
    result += 60. * OFStandard::atof(tmp);

    strncpy (tmp, t + 4, 2);
    tmp [3] = '\0';
    result += OFStandard::atof(tmp);

    return result;
}
#endif

#if 1
/***********************
 *    Duplicate a dicom element
 *    dst space is supposed provided by the caller
 */

static void DB_DuplicateElement (DB_SmallDcmElmt *src, DB_SmallDcmElmt *dst)
{
    bzero( (char*)dst, sizeof (DB_SmallDcmElmt));
    dst -> XTag = src -> XTag;
    dst -> ValueLength = src -> ValueLength;

    if (src -> ValueLength == 0)
        dst -> PValueField = NULL;
    else {
        dst -> PValueField = (char *)malloc ((int) src -> ValueLength+1);
        bzero(dst->PValueField, (size_t)(src->ValueLength+1));
        if (dst->PValueField != NULL) {
            memcpy (dst -> PValueField,  src -> PValueField,
                (size_t) src -> ValueLength);
        } else {
            DCMQRDB_ERROR("DB_DuplicateElement: out of memory");
        }
    }
}
#endif

#if 1

/* ========================= FIND ========================= */

/************
**      Try to match Two Dates
**      The first one is the "model", the second one an element
**      Returns OFTrue if matching is ok, else returns OFFalse
 */

static int matchDate (DB_SmallDcmElmt *mod, DB_SmallDcmElmt *elt)
{
    char date [DBC_MAXSTRING] ;
    char modl [DBC_MAXSTRING] ;

    /*** Get elt and model data in strings
    **/

    memcpy (date, elt->PValueField, (size_t)(elt->ValueLength)) ;
    date [elt->ValueLength] = '\0' ;
    DB_RemoveSpaces (date) ;

    memcpy (modl, mod->PValueField, (size_t)(mod->ValueLength)) ;
    modl [mod->ValueLength] = '\0' ;
    DB_RemoveSpaces (modl) ;

    /*** If no '-' in date
    *** return strict comparison result
    **/

    if (strchr (modl, '-') == NULL)
        return (strcmp (modl, date) == 0) ;

    /*** If first char is -
    **/

    if (modl [0] == '-') {
        return DB_DateToLong (date) <= DB_DateToLong (modl+1) ;
    }

    /*** If last char is -
    **/

    else if (modl [strlen (modl) - 1] == '-') {
        modl [strlen (modl) - 1] = '\0' ;
        return DB_DateToLong (date) >= DB_DateToLong (modl) ;
    }
    else {
        char *pc ;
        long d ;

        d = DB_DateToLong (date) ;
        pc = strchr (modl, '-') ;
        *pc = '\0' ;

        return (d >= DB_DateToLong (modl)) && (d <= DB_DateToLong (pc+1)) ;

    }
}

/************
**      Try to match Two Times
**      The first one is the "model", the second one an element
**      Returns OFTrue if matching is ok, else returns OFFalse
 */

static int matchTime (DB_SmallDcmElmt *mod, DB_SmallDcmElmt *elt)
{
    char aTime [DBC_MAXSTRING] ;
    char modl [DBC_MAXSTRING] ;

    /*** Get elt and model data in strings
    **/

    memcpy (aTime, elt->PValueField, (size_t)(elt->ValueLength)) ;
    aTime [elt->ValueLength] = '\0' ;
    DB_RemoveSpaces (aTime) ;

    memcpy (modl, mod->PValueField, (size_t)(mod->ValueLength)) ;
    modl [mod->ValueLength] = '\0' ;
    DB_RemoveSpaces (modl) ;

    /*** If no '-' in time
    *** return strict comparison result
    **/

    if (strchr (modl, '-') == NULL)
        return (strcmp (modl, aTime) == 0) ;

    /*** If first char is -
    **/

    if (modl [0] == '-') {
        return DB_TimeToDouble (aTime) <= DB_TimeToDouble (modl+1) ;
    }

    /*** If last char is -
    **/

    else if (modl [strlen (modl) - 1] == '-') {
        modl [strlen (modl) - 1] = '\0' ;
        return DB_TimeToDouble (aTime) >= DB_TimeToDouble (modl) ;
    }
    else {
        char *pc ;
        double t ;

        t = DB_TimeToDouble (aTime) ;
        pc = strchr (modl, '-') ;
        *pc = '\0' ;

        return (t >= DB_TimeToDouble (modl)) && (t <= DB_TimeToDouble (pc+1)) ;

    }
}

/************
**      Try to match Two UID
**      The first one is the "model", the second one an element
**      Returns OFTrue if matching is ok, else returns OFFalse
 */

static int matchUID (DB_SmallDcmElmt *mod, DB_SmallDcmElmt *elt)
{
    int match ;
    char *uid ;
    char *modl ;
    char *pc ;
    unsigned int length ;

    /*** Get elt and model data in strings
    **/

    uid = (char *) malloc ((size_t)(elt->ValueLength + 1)) ;
    if (uid == NULL) {
        return 0 ;
    }
    memcpy (uid, elt->PValueField, (size_t)(elt->ValueLength)) ;
    uid [elt->ValueLength] = '\0' ;

    modl = (char *) malloc ((size_t)(mod->ValueLength + 1)) ;
    if (modl == NULL) {
        free (uid) ;
        return 0 ;
    }
    memcpy (modl, mod->PValueField, (size_t)(mod->ValueLength)) ;
    modl [mod->ValueLength] = '\0' ;

    /*** If no '\' in model
    *** return strict comparison result
    **/

#ifdef STRICT_COMPARE
#else
    /*** Suppress Leading and Trailing spaces in
    *** model and string
    **/

    DB_RemoveEnclosingSpaces (uid) ;
    DB_RemoveEnclosingSpaces (modl) ;
#endif

    if (strchr (modl, '\\') == NULL) {
        match = (strcmp (modl, uid) == 0) ;
        free (uid) ;
        free (modl) ;
        return (match) ;
    }

    /*** UID List comparison.
    *** Match is successful if uid is found in model
    **/

    match = OFFalse ;
    for (pc = modl ; *pc ; ) {

        /*** Calculate the length to the next '\' sign (if any).
        *** Otherwise the length of pc is returned.
        **/
        length = strcspn(pc, "\\") ;

        if ((length == strlen(uid)) && (strncmp (pc, uid, length) == 0)) {
            match = OFTrue ;
            break ;
        }
        else {
            pc = strchr (pc, '\\') ;
            if (pc == NULL)
                break ;
            else
                pc++ ;
        }
    }

    free (uid) ;
    free (modl) ;
    return (match) ;

}

/************
**      Try to match Two Strings
**      The first one is the "model", the second one an element
**      Returns OFTrue if matching is ok, else returns OFFalse
 */

static int matchStrings (DB_SmallDcmElmt *mod, DB_SmallDcmElmt *elt)
{
    int match ;
    char *string ;
    char *modl ;

    /*** Get elt and model data in strings
    **/

    string = (char *) malloc ((size_t)(elt->ValueLength + 1)) ;
    if (string == NULL) {
        return 0 ;
    }
    memcpy (string, elt->PValueField, (size_t)(elt->ValueLength)) ;
    string [elt->ValueLength] = '\0' ;

    modl = (char *) malloc ((size_t)(mod->ValueLength + 1)) ;
    if (modl == NULL) {
        free (string) ;
        return 0 ;
    }
    memcpy (modl, mod->PValueField, (size_t)(mod->ValueLength)) ;
    modl [mod->ValueLength] = '\0' ;

#ifdef STRICT_COMPARE
#else
    /*** Suppress Leading and Trailing spaces in
    *** model and string
    **/

    DB_RemoveEnclosingSpaces (string) ;
    DB_RemoveEnclosingSpaces (modl) ;
#endif

    /*** If no '*' and no '?' in model
    *** return strict comparison result
    **/

    if ((strchr (modl, '*') == NULL) && (strchr (modl, '?') == NULL))
        return (strcmp (modl, string) == 0) ;

    match = DB_StringUnify (modl, string) ;

    free (string) ;
    free (modl) ;
    return (match) ;

}

/************
**      Try to match Two Unknown elements
**      Strict comparaison is applied
**      The first one is the "model", the second one an element
**      Returns OFTrue if matching is ok, else returns OFFalse
 */

static int matchOther (DB_SmallDcmElmt *mod, DB_SmallDcmElmt *elt)
{
    if (mod->ValueLength != elt->ValueLength)
        return OFFalse ;

    return (memcmp (mod->PValueField, elt->PValueField, (size_t)(elt->ValueLength)) == 0) ;
}
#endif

//TODO: move all of the above match* methods to a common util

#if 1
/************
**      Try to match Two DB_SmallDcmElmts
**      The first one is the "model", the second one an element
**      Returns OFTrue if matching is ok, else returns OFFalse
 */

static int dbmatch (DB_SmallDcmElmt *mod, DB_SmallDcmElmt *elt)
{
    DB_KEY_CLASS keyClass = OTHER_CLASS;

    /*** If model length is 0
    *** Universal matching is applied : return always OFTrue
    **/

    if (mod->ValueLength == 0)
        return (OFTrue) ;

    /*** Get the key class of the element
    **/

    DB_GetTagKeyClass (elt->XTag, &keyClass) ;

    switch (keyClass) {

    case DATE_CLASS :
        return matchDate (mod, elt) ;

    case TIME_CLASS :
        return matchTime (mod, elt) ;

    case UID_CLASS :
        return matchUID  (mod, elt) ;

    case STRING_CLASS :
        return matchStrings (mod, elt) ;

    case OTHER_CLASS :
        return matchOther (mod, elt) ;

    }
    return OFFalse;
}

#endif 
/************
**      Create the response list in specified handle,
**      using informations found in an index record.
**      Old response list is supposed freed
**/

static DB_ElementList* makeResponseList (
                DB_ElementList  *findRequestList,
                IdxRecord               *idxRec
                )
{
    int i ;
    DB_ElementList *pRequestList = NULL;
    DB_ElementList *plist = NULL;
    DB_ElementList *last = NULL;

	DB_ElementList *findResponseList = NULL;
    /*** For each element in Request identifier
    **/

    for (pRequestList = findRequestList ; pRequestList ; pRequestList = pRequestList->next) {

        /*** Find Corresponding Tag in index record
        **/

        for (i = 0 ; i < NBPARAMETERS ; i++)
            if (idxRec->param [i]. XTag == pRequestList->elem. XTag)
                break ;

        /*** If Tag not found, skip the element
        **/

        if (i >= NBPARAMETERS)
            continue ;

        /*** Append index record element to response list
        **/

        plist = (DB_ElementList *) malloc (sizeof (DB_ElementList)) ;
        if (plist == NULL) {
            DCMQRDB_ERROR("makeResponseList: out of memory");
            return NULL;
        }
        plist->next = NULL ;

        DB_DuplicateElement(&idxRec->param[i], &plist->elem);

        if (findResponseList == NULL) {
            findResponseList = last = plist ;
        }
        else {
            last->next = plist ;
            last = plist ;
        }
    }

    return findResponseList;
}


#if 1
/************
**      Test a Find Request List
**      Returns EC_Normal if ok, else returns DcmQRSqlDatabaseError
 */

OFCondition DcmQueryRetrieveSQLDatabaseHandle::testFindRequestList (
                DB_ElementList  *findRequestList,
                DB_LEVEL        queryLevel,
                DB_LEVEL        infLevel,
                DB_LEVEL        lowestLevel
                )
{
    DB_ElementList *plist ;
    DB_LEVEL    XTagLevel = PATIENT_LEVEL; // DB_GetTagLevel() will set this correctly
    DB_KEY_TYPE XTagType  = OPTIONAL_KEY;  // DB_GetTagKeyAttr() will set this
    int level ;

    /**** Query level must be at least the infLevel
    ***/

    if (queryLevel < infLevel) {
        DCMQRDB_INFO("Level incompatible with Information Model (level " << queryLevel << ")");
        return DcmQRSqlDatabaseError ;
    }

    if (queryLevel > lowestLevel) {
        DCMQRDB_DEBUG("Level incompatible with Information Model (level " << queryLevel << ")");
        return DcmQRSqlDatabaseError ;
    }

    for (level = PATIENT_LEVEL ; level <= IMAGE_LEVEL ; level++) {

        /**** Manage exception due to StudyRoot Information Model :
        **** In this information model, queries may include Patient attributes
        **** but only if they are made at the study level
        ***/

        if ((level == PATIENT_LEVEL) && (infLevel == STUDY_LEVEL)) {
            /** In Study Root Information Model, accept only Patient Tags
            ** if the Query Level is the Study level
            */

            int atLeastOneKeyFound = OFFalse ;
            for (plist = findRequestList ; plist ; plist = plist->next) {
                DB_GetTagLevel (plist->elem. XTag, &XTagLevel) ;
                if (XTagLevel != level)
                    continue ;
                atLeastOneKeyFound = OFTrue ;
            }
            if (atLeastOneKeyFound && (queryLevel != STUDY_LEVEL)) {
                DCMQRDB_DEBUG("Key found in Study Root Information Model (level " << level << ")");
                return DcmQRSqlDatabaseError ;
            }
        }

        /**** If current level is above the QueryLevel
        ***/

        else if (level < queryLevel) {

            /** For this level, only unique keys are allowed
            ** Parse the request list elements reffering to
            ** this level.
            ** Check that only unique key attr are provided
            */

            int uniqueKeyFound = OFFalse ;
            for (plist = findRequestList ; plist ; plist = plist->next) {
                DB_GetTagLevel (plist->elem. XTag, &XTagLevel) ;
                if (XTagLevel != level)
                    continue ;
                DB_GetTagKeyAttr (plist->elem. XTag, &XTagType) ;
                if (XTagType != UNIQUE_KEY) {
                    DCMQRDB_DEBUG("Non Unique Key found (level " << level << ")");
                    return DcmQRSqlDatabaseError ;
                }
                else if (uniqueKeyFound) {
                    DCMQRDB_DEBUG("More than one Unique Key found (level " << level << ")");
                    return DcmQRSqlDatabaseError ;
                }
                else
                    uniqueKeyFound = OFTrue ;
            }
        }

        /**** If current level is the QueryLevel
        ***/

        else if (level == queryLevel) {

            /** For this level, all keys are allowed
            ** Parse the request list elements reffering to
            ** this level.
            ** Check that at least one key is provided
            */

            int atLeastOneKeyFound = OFFalse ;
            for (plist = findRequestList ; plist ; plist = plist->next) {
                DB_GetTagLevel (plist->elem. XTag, &XTagLevel) ;
                if (XTagLevel != level)
                    continue ;
                atLeastOneKeyFound = OFTrue ;
            }
            if (! atLeastOneKeyFound) {
                DCMQRDB_DEBUG("No Key found at query level (level " << level << ")");
                return DcmQRSqlDatabaseError ;
            }
        }

        /**** If current level beyond the QueryLevel
        ***/

        else if (level > queryLevel) {

            /** For this level, no key is allowed
            ** Parse the request list elements reffering to
            ** this level.
            ** Check that no key is provided
            */

            int atLeastOneKeyFound = OFFalse ;
            for (plist = findRequestList ; plist ; plist = plist->next) {
                DB_GetTagLevel (plist->elem. XTag, &XTagLevel) ;
                if (XTagLevel != level)
                    continue ;
                atLeastOneKeyFound = OFTrue ;
            }
            if (atLeastOneKeyFound) {
                DCMQRDB_DEBUG("Key found beyond query level (level " << level << ")");
                return DcmQRSqlDatabaseError ;
            }
        }

    }
    return EC_Normal ;
}
#endif

#if 1
/************
**      Hierarchical Search Algorithm
**      Returns OFTrue if matching is ok, else returns OFFalse
 */

OFCondition DcmQueryRetrieveSQLDatabaseHandle::hierarchicalCompare (
                DB_Private_Handle       *phandle,
                IdxRecord               *idxRec,
                DB_LEVEL                level,
                DB_LEVEL                infLevel,
                int                     *match)
{
    int                 i ;
    DcmTagKey   XTag ;
    DB_ElementList *plist ;
    DB_LEVEL    XTagLevel  = PATIENT_LEVEL; // DB_GetTagLevel() will set this correctly

    /**** If current level is above the QueryLevel
    ***/

    if (level < phandle->queryLevel) {

        /** Get UID Tag for current level */
        DB_GetUIDTag (level, &XTag) ;

        /** Find Element with this XTag in Identifier list*/
        for (plist = phandle->findRequestList ; plist ; plist = plist->next){
            if (plist->elem.XTag == XTag)
                break ;
        }

        /** Element not found */
        if (plist == NULL) {
            *match = OFFalse ;
            DCMQRDB_WARN("hierarchicalCompare : No UID Key found at level " << (int) level);
            return DcmQRSqlDatabaseError ;
        }

        /** Find element with the same XTag in index record */
        for (i = 0 ; i < NBPARAMETERS ; i++){
            if (idxRec->param[i]. XTag == XTag)
                break ;
        }

        /** Compare with Single value matching. If Match fails, return OFFalse */
        if (! dbmatch (&(plist->elem), &idxRec->param[i])) {
            *match = OFFalse ;
            return EC_Normal ;
        }

        /** Match succeeded. Try at next level */

        return hierarchicalCompare (phandle, idxRec, (DB_LEVEL)(level + 1), infLevel, match) ;
    }
    /**** If current level is the QueryLevel ***/
    else if (level == phandle->queryLevel) {

        /*** For each element in Identifier list **/
        for (plist = phandle->findRequestList ; plist ; plist = plist->next) {

            /** Get the Tag level of this element*/

            DB_GetTagLevel (plist->elem. XTag, &XTagLevel) ;

            /** If we are in the Study Root Information Model exception
            ** we must accept patients keys at the study level
            */
            if (  (XTagLevel == PATIENT_LEVEL)
                  && (phandle->queryLevel == STUDY_LEVEL)
                  && (infLevel == STUDY_LEVEL)
                )
                ;

            /** In other cases, only keys at the current level are
            ** taken into account. So skip this element.
            */
            else if (XTagLevel != level)
                continue ;

            /** Find element with the same XTag in index record
             */

            for (i = 0 ; i < NBPARAMETERS ; i++){
                if (idxRec->param [i]. XTag == plist->elem. XTag)
                    break ;
            }

            /** Compare with appropriate Matching. If Match fails, return OFFalse */
            if (! dbmatch (&(plist->elem), &idxRec->param[i])) {
                *match = OFFalse ;
                return EC_Normal ;
            }
        }

        /*** If we are here, all matches succeeded at the current level.
        *** Perhaps check that we have tried at least one match ??
        **/

        *match = OFTrue ;
        return EC_Normal ;

    }
    return DcmQRSqlDatabaseError;
}
#endif
/********************
**      Start find in Database
**/

OFCondition startFindRequestExtracted(
                                      DcmDataset *findRequestIdentifiers
                                      , DcmQueryRetrieveDatabaseStatus *status
                                      , DB_ElementList*& plist
                                      , DB_ElementList*& last
                                      , bool& qrLevelFound
                                      , DB_LEVEL& queryLevel
                                      , DB_ElementList *findRequestList)
{
    DB_SmallDcmElmt elem;
    
    int elemCount = (int)(findRequestIdentifiers->card());
    for (int elemIndex=0; elemIndex<elemCount; elemIndex++) {

        DcmElement* dcelem = findRequestIdentifiers->getElement(elemIndex);

        elem.XTag = dcelem->getTag().getXTag();
        if (elem.XTag == DCM_QueryRetrieveLevel || DB_TagSupported(elem.XTag)) {
            elem.ValueLength = dcelem->getLength();
            if (elem.ValueLength == 0) {
                elem.PValueField = NULL ;
            } else if ((elem.PValueField = (char*)malloc((size_t)(elem.ValueLength+1))) == NULL) {
                status->setStatus(STATUS_FIND_Refused_OutOfResources);
                return (DcmQRSqlDatabaseError) ;
            } else {
                /* only char string type tags are supported at the moment */
                char *s = NULL;
                dcelem->getString(s);
                strcpy(elem.PValueField, s);
            }
            /** If element is the Query Level, store it in handle
            */

            if (elem.XTag == DCM_QueryRetrieveLevel) {
                char *pc ;
                char level [50] ;

                strncpy(level, (char*)elem.PValueField,
                    (elem.ValueLength<50)? (size_t)(elem.ValueLength) : 49) ;

                /*** Skip this two lines if you want strict comparison
                **/

                for (pc = level ; *pc ; pc++)
                    *pc = ((*pc >= 'a') && (*pc <= 'z')) ? 'A' - 'a' + *pc : *pc ;

                if (strncmp(level, PATIENT_LEVEL_STRING, strlen(PATIENT_LEVEL_STRING)) == 0){
                    queryLevel = PATIENT_LEVEL ;
                }else if (strncmp(level, STUDY_LEVEL_STRING, strlen(STUDY_LEVEL_STRING)) == 0){
                    queryLevel = STUDY_LEVEL ;
                }else if (strncmp(level, SERIE_LEVEL_STRING, strlen(SERIE_LEVEL_STRING)) == 0){
                    queryLevel = SERIE_LEVEL ;
                }else if (strncmp(level, IMAGE_LEVEL_STRING, strlen(IMAGE_LEVEL_STRING)) == 0){
                    queryLevel = IMAGE_LEVEL ;
                }else {
                    if (elem.PValueField)
                        free(elem.PValueField) ;
#ifdef DEBUG
                    DCMQRDB_DEBUG("DB_startFindRequest () : Illegal query level (" << level << ")");
#endif
                    status->setStatus(STATUS_FIND_Failed_UnableToProcess);
                    return (DcmQRSqlDatabaseError) ;
                }
                qrLevelFound = OFTrue;
            } else {
                /** Else it is a query identifier. Append it to our RequestList if it is supported
                */
                if (DB_TagSupported (elem. XTag)) {
                    plist = (DB_ElementList *) malloc (sizeof (DB_ElementList)) ;
                    if (plist == NULL) {
                        status->setStatus(STATUS_FIND_Refused_OutOfResources);
                        return (DcmQRSqlDatabaseError) ;
                    }
                    plist->next = NULL ;
                    DB_DuplicateElement (&elem, &(plist->elem)) ;
                    if (findRequestList == NULL) {
                        findRequestList = last = plist ;
                    } else {
                        last->next = plist ;
                        last = plist ;
                    }
                }
            }

            if (elem.PValueField ) {
                free(elem.PValueField) ;
            }
        }
    }

    return EC_Normal;
}

int GetTagGroupElement(const DcmTagKey& dcmTagKey)
{
    //TODO: the behavior of hash() may !? change in future, add a new new function instead of it
    int tagNumber = dcmTagKey.hash();
    return tagNumber;
}

OFCondition DcmQueryRetrieveSQLDatabaseHandle::startFindRequest(
                const char      *SOPClassUID,
                DcmDataset      *findRequestIdentifiers,
                DcmQueryRetrieveDatabaseStatus  *status)
{
    //DB_SmallDcmElmt     elem ;
    DB_LEVEL            qLevel = PATIENT_LEVEL; // highest legal level for a query in the current model
    DB_LEVEL            lLevel = IMAGE_LEVEL;   // lowest legal level for a query in the current model
    OFCondition cond = EC_Normal;
    OFBool      qrLevelFound = OFFalse;
    DB_QUERY_CLASS rootLevel = PATIENT_ROOT;

    /**** Is SOPClassUID supported ? ***/

    if (strcmp( SOPClassUID, UID_FINDPatientRootQueryRetrieveInformationModel) == 0){
        rootLevel = PATIENT_ROOT ;
    }else if (strcmp( SOPClassUID, UID_FINDStudyRootQueryRetrieveInformationModel) == 0){
        rootLevel = STUDY_ROOT ;
#ifndef NO_PATIENTSTUDYONLY_SUPPORT
    }else if (strcmp( SOPClassUID, UID_RETIRED_FINDPatientStudyOnlyQueryRetrieveInformationModel) == 0){
        rootLevel = PATIENT_STUDY ;
#endif
    }else {
        status->setStatus(STATUS_FIND_Refused_SOPClassNotSupported);
        return (DcmQRSqlDatabaseError) ;
    }


    /**** Parse Identifiers in the Dicom Object
    **** Find Query Level and contruct a list
    **** of query identifiers
    ***/

    DB_ElementList  *findRequestList = NULL ;
    DB_LEVEL        queryLevel;
    DB_ElementList  *plist = NULL;
    DB_ElementList  *last = NULL;

    cond = startFindRequestExtracted(findRequestIdentifiers, status, plist, last, qrLevelFound, queryLevel, findRequestList);
    if (EC_Normal != cond)
        return cond;

    if (!qrLevelFound) {
        /* The Query/Retrieve Level is missing */
        status->setStatus(STATUS_FIND_Failed_IdentifierDoesNotMatchSOPClass);
        DCMQRDB_WARN("DB_startFindRequest(): missing Query/Retrieve Level");
        handle_->idxCounter = -1 ;
        DB_FreeElementList (handle_->findRequestList) ;
        handle_->findRequestList = NULL ;
        return (DcmQRSqlDatabaseError) ;
    }

    switch (rootLevel)
    {
      case PATIENT_ROOT :
        qLevel = PATIENT_LEVEL ;
        lLevel = IMAGE_LEVEL ;
        break ;
      case STUDY_ROOT :
        qLevel = STUDY_LEVEL ;
        lLevel = IMAGE_LEVEL ;
        break ;
      case PATIENT_STUDY:
        qLevel = PATIENT_LEVEL ;
        lLevel = STUDY_LEVEL ;
        break ;
    }

    /**** Test the consistency of the request list
    ***/


    if (doCheckFindIdentifier) {
        cond = testFindRequestList (findRequestList, queryLevel, qLevel, lLevel) ;
        if (cond != EC_Normal) {
            handle_->idxCounter = -1 ;
            DB_FreeElementList (handle_->findRequestList) ;
            handle_->findRequestList = NULL ;
#ifdef DEBUG
            DCMQRDB_DEBUG("DB_startFindRequest () : STATUS_FIND_Failed_IdentifierDoesNotMatchSOPClass - Invalid RequestList");
#endif
            status->setStatus(STATUS_FIND_Failed_IdentifierDoesNotMatchSOPClass);
            return (cond) ;
        }
    }
    
    int MatchFound = OFFalse ;
    cond = EC_Normal ;

    //TODO: move out of the scope of this function to persist across calls

    std::string tagList, valueList;

    //TODO: really hate this shoving of values into comma separated list (type safety goes out the window)
    //TODO: plus this is slow
    for(DB_ElementList* curr = plist; curr != NULL; curr = curr->next, tagList += ",", valueList += ","){
        int tagNumber = GetTagGroupElement(curr->elem.XTag);
        char tagNumberStringDec[16] = "\0";
        itoa(tagNumber, tagNumberStringDec, 10);
        tagList += tagNumberStringDec;
        valueList += curr->elem.PValueField;
    }

    BOOL bRes = FALSE;

    CAutoPtr<IDbCommand> pCmd(piDbSystem_->CreateCommand(piDbDatabase_));
    bRes = pCmd->Create(_T(
        "EXEC [dcmqrdb_mssql].[dbo].[spFindDcmInstance]"
        "  @tagList = ?"
        ", @valueList = ?"
        ";"
        ));
    bRes = pCmd->SetParam(0, CA2T(tagList.c_str()));
    bRes = pCmd->SetParam(1, CA2T(valueList.c_str()));
    CAutoPtr<IDbRecordset> pRec(piDbSystem_->CreateRecordset(piDbDatabase_));
    bRes = pCmd->Execute(pRec);

    DWORD dwCnt = pRec->GetRowCount();
    dwCnt;

    DWORD nFields = pRec->GetColumnCount();
    for(DWORD i = 0; i < nFields; i++){
        TCHAR name[128];
        pRec->GetColumnName(i, name, ARRAYSIZE(name));
    }

    long lInstanceKey = -1;

    if( !pRec->IsEOF() ) {
        piFindRecordSet_ = pRec.Detach();
        status->setStatus(STATUS_Pending);
        return (EC_Normal) ;
    }else{
        //nothing found
        status->setStatus(STATUS_Success);
        return (EC_Normal) ;
    }

    /*** Exit loop if error or matching OK **/

    /**** If an error occured in Matching function
    ****    return a failed status
    ***/

    if (cond != EC_Normal) {
        DB_FreeElementList (findRequestList) ;
        findRequestList = NULL ;
#ifdef DEBUG
        DCMQRDB_DEBUG("DB_startFindRequest () : STATUS_FIND_Failed_UnableToProcess");
#endif
        status->setStatus(STATUS_FIND_Failed_UnableToProcess);

        //DB_unlock();

        return (cond) ;
    }
}

/********************
**      Get next find response in Database
 */

OFCondition DcmQueryRetrieveSQLDatabaseHandle::nextFindResponse (
                DcmDataset      **findResponseIdentifiers,
                DcmQueryRetrieveDatabaseStatus  *status)
{
    BOOL bRes = FALSE;
    
    for(long lInstanceKey = -1; !piFindRecordSet_->IsEOF(); piFindRecordSet_->MoveNext()) {
        piFindRecordSet_->GetField(piFindRecordSet_->GetColumnIndex(_T("InstanceKey")), lInstanceKey);

        CAutoPtr<IDbCommand> pCmd2(piDbSystem_->CreateCommand(piDbDatabase_));
        bRes = pCmd2->Create(_T(
            "EXEC [dcmqrdb_mssql].[dbo].[spGetInstanceAttributes]"
            "  @instanceKey = ?"
            ";"
            ));
        bRes = pCmd2->SetParam(0, &lInstanceKey);
        CAutoPtr<IDbRecordset> pRec2(piDbSystem_->CreateRecordset(piDbDatabase_));
        bRes = pCmd2->Execute(pRec2);

        IdxRecord idxRec ;

        /**** If a matching image has been found, add index record to UID found list 
        prepare Response List in handle return status is pending
        ***/

        *findResponseIdentifiers = new DcmDataset();

        for(; !pRec2->IsEOF(); pRec2->MoveNext()){
            long tag = -1;
            TCHAR value[128];
            pRec2->GetField(pRec2->GetColumnIndex(_T("AttributeTag")), tag);
            pRec2->GetField(pRec2->GetColumnIndex(_T("Value")), value, ARRAYSIZE(value));

            DcmTag t(((tag >> 16) & 0xFFFF), tag & 0xFFFF);
            DcmElement *dce = newDicomElement(t);
            if (dce == NULL) {
                status->setStatus(STATUS_FIND_Refused_OutOfResources);
                return DcmQRSqlDatabaseError;
            }

            OFCondition ec = dce->putString(CT2A(value));
            if (ec != EC_Normal) {
                DCMQRDB_WARN("dbfind: DB_nextFindResponse: cannot put()");
                status->setStatus(STATUS_FIND_Failed_UnableToProcess);
                return DcmQRSqlDatabaseError;
            }

            ec = (*findResponseIdentifiers)->insert(dce, OFTrue /*replaceOld*/);
            if (ec != EC_Normal) {
                DCMQRDB_WARN("dbfind: DB_nextFindResponse: cannot insert()");
                status->setStatus(STATUS_FIND_Failed_UnableToProcess);
                return DcmQRSqlDatabaseError;
            }
        }
        status->setStatus(STATUS_Pending);
        return (EC_Normal) ;
    }
    //TODO: free up the response db dataset

    *findResponseIdentifiers = NULL ;
    status->setStatus(STATUS_Success);

    return (EC_Normal) ;

#if 0
    DB_ElementList      *plist = NULL;
    int                 MatchFound = OFFalse;
    IdxRecord           idxRec ;
    DB_LEVEL            qLevel = PATIENT_LEVEL;
    const char          *queryLevelString = NULL;
    OFCondition         cond = EC_Normal;

    if (handle_->findResponseList == NULL) {
#ifdef DEBUG
        DCMQRDB_DEBUG("DB_nextFindResponse () : STATUS_Success");
#endif
        *findResponseIdentifiers = NULL ;
        status->setStatus(STATUS_Success);

        DB_unlock();

        return (EC_Normal) ;
    }

    /***** Create the response (findResponseIdentifiers) using
    ***** the last find done and saved in handle findResponseList
    ****/

    *findResponseIdentifiers = new DcmDataset ;
    if ( *findResponseIdentifiers != NULL ) {

        /*** Put responses
        **/

        for ( plist = handle_->findResponseList ; plist != NULL ; plist = plist->next ) {
            DcmTag t(plist->elem.XTag);
            DcmElement *dce = newDicomElement(t);
            if (dce == NULL) {
                status->setStatus(STATUS_FIND_Refused_OutOfResources);
                return DcmQRIndexDatabaseError;
            }
            if (plist->elem.PValueField != NULL &&
                strlen(plist->elem.PValueField) > 0) {
                OFCondition ec = dce->putString(plist->elem.PValueField);
                if (ec != EC_Normal) {
                    DCMQRDB_WARN("dbfind: DB_nextFindResponse: cannot put()");
                    status->setStatus(STATUS_FIND_Failed_UnableToProcess);
                    return DcmQRIndexDatabaseError;
                }
            }
            OFCondition ec = (*findResponseIdentifiers)->insert(dce, OFTrue /*replaceOld*/);
            if (ec != EC_Normal) {
                DCMQRDB_WARN("dbfind: DB_nextFindResponse: cannot insert()");
                status->setStatus(STATUS_FIND_Failed_UnableToProcess);
                return DcmQRIndexDatabaseError;
            }
        }

        /*** Append the Query level
        **/

        switch (handle_->queryLevel) {
        case PATIENT_LEVEL :
            queryLevelString = PATIENT_LEVEL_STRING ;
            break ;
        case STUDY_LEVEL :
            queryLevelString = STUDY_LEVEL_STRING ;
            break ;
        case SERIE_LEVEL :
            queryLevelString = SERIE_LEVEL_STRING ;
            break ;
        case IMAGE_LEVEL :
            queryLevelString = IMAGE_LEVEL_STRING ;
            break ;
        }
        DU_putStringDOElement(*findResponseIdentifiers,
                              DCM_QueryRetrieveLevel, queryLevelString);
#ifdef DEBUG
        DCMQRDB_DEBUG("DB: findResponseIdentifiers:" << OFendl
            << DcmObject::PrintHelper(**findResponseIdentifiers));
#endif
    }
    else {

        DB_unlock();

        return (DcmQRIndexDatabaseError) ;
    }

    switch (handle_->rootLevel) {
    case PATIENT_ROOT : qLevel = PATIENT_LEVEL ;        break ;
    case STUDY_ROOT :   qLevel = STUDY_LEVEL ;          break ;
    case PATIENT_STUDY: qLevel = PATIENT_LEVEL ;        break ;
    }

    /***** Free the last response...
    ****/

    DB_FreeElementList (handle_->findResponseList) ;
    handle_->findResponseList = NULL ;

    /***** ... and find the next one
    ****/

    MatchFound = OFFalse ;
    cond = EC_Normal ;

    while (1) {

        /*** Exit loop if read error (or end of file)
        **/

        if (DB_IdxGetNext (&(handle_->idxCounter), &idxRec) != EC_Normal)
            break ;

        /*** If Response already found
        **/

        if (DB_UIDAlreadyFound (handle_, &idxRec))
            continue ;

        /*** Exit loop if error or matching OK
        **/

        cond = hierarchicalCompare (handle_, &idxRec, qLevel, qLevel, &MatchFound) ;
        if (cond != EC_Normal)
            break ;
        if (MatchFound)
            break ;

    }

    /**** If an error occured in Matching function
    ****    return status is pending
    ***/

    if (cond != EC_Normal) {
        handle_->idxCounter = -1 ;
        DB_FreeElementList (handle_->findRequestList) ;
        handle_->findRequestList = NULL ;
#ifdef DEBUG
        DCMQRDB_DEBUG("DB_nextFindResponse () : STATUS_FIND_Failed_UnableToProcess");
#endif
        status->setStatus(STATUS_FIND_Failed_UnableToProcess);

        DB_unlock();

        return (cond) ;
    }

    /**** If a matching image has been found
    ****    add index records UIDs in found UID list
    ****    prepare Response List in handle
    ***/

    if (MatchFound) {
        DB_UIDAddFound (handle_, &idxRec) ;
        makeResponseList (handle_, &idxRec) ;
#ifdef DEBUG
        DCMQRDB_DEBUG("DB_nextFindResponse () : STATUS_Pending");
#endif
        status->setStatus(STATUS_Pending);
        return (EC_Normal) ;
    }

    /**** else no matching image has been found,
    ****    free query identifiers list
    **** Response list is null, so next call will return STATUS_Success
    ***/

    else {
        handle_->idxCounter = -1 ;
        DB_FreeElementList (handle_->findRequestList) ;
        handle_->findRequestList = NULL ;
        DB_FreeUidList (handle_->uidList) ;
        handle_->uidList = NULL ;
    }

#ifdef DEBUG
    DCMQRDB_DEBUG("DB_nextFindResponse () : STATUS_Pending");
#endif
    status->setStatus(STATUS_Pending);
    return (EC_Normal) ;

#endif
}

/********************
**      Cancel find request
 */

OFCondition DcmQueryRetrieveSQLDatabaseHandle::cancelFindRequest (DcmQueryRetrieveDatabaseStatus *status)
{
  return EC_MemoryExhausted;

#if 0

    handle_->idxCounter = -1 ;
    DB_FreeElementList (handle_->findRequestList) ;
    handle_->findRequestList = NULL ;
    DB_FreeElementList (handle_->findResponseList) ;
    handle_->findResponseList = NULL ;
    DB_FreeUidList (handle_->uidList) ;
    handle_->uidList = NULL ;

    status->setStatus(STATUS_FIND_Cancel_MatchingTerminatedDueToCancelRequest);

    DB_unlock();

    return (EC_Normal) ;
#endif
}

/* ========================= MOVE ========================= */
#if 0
/************
 *      Test a Move Request List
 *      Returns EC_Normal if ok, else returns DcmQRIndexDatabaseError
 */

OFCondition DcmQueryRetrieveSQLDatabaseHandle::testMoveRequestList (
                DB_ElementList  *findRequestList,
                DB_LEVEL        queryLevel,
                DB_LEVEL        infLevel,
                DB_LEVEL        lowestLevel
                )
{
    DB_ElementList *plist ;
    DB_LEVEL    XTagLevel = PATIENT_LEVEL; // DB_GetTagLevel() will set this correctly
    DB_KEY_TYPE XTagType  = OPTIONAL_KEY;  // DB_GetTagKeyAttr() will set this
    int level ;

    /**** Query level must be at least the infLevel
    ***/

    if (queryLevel < infLevel) {
        DCMQRDB_DEBUG("Level incompatible with Information Model (level " << (int)queryLevel << ")");
        return DcmQRIndexDatabaseError ;
    }

    if (queryLevel > lowestLevel) {
        DCMQRDB_DEBUG("Level incompatible with Information Model (level " << (int)queryLevel << ")");
        return DcmQRIndexDatabaseError ;
    }

    for (level = PATIENT_LEVEL ; level <= IMAGE_LEVEL ; level++) {

        /**** Manage exception due to StudyRoot Information Model :
        **** In this information model, move may not include any
        **** Patient attributes.
        ***/

        if ((level == PATIENT_LEVEL) && (infLevel == STUDY_LEVEL)) {

            /** In Study Root Information Model, do not accept any
            ** Patient Tag
            */

            int atLeastOneKeyFound = OFFalse ;
            for (plist = findRequestList ; plist ; plist = plist->next) {
                DB_GetTagLevel (plist->elem. XTag, &XTagLevel) ;
                if (XTagLevel != level)
                    continue ;
                atLeastOneKeyFound = OFTrue ;
            }
            if (atLeastOneKeyFound) {
                DCMQRDB_DEBUG("Key found in Study Root Information Model (level " << level << ")");
                return DcmQRIndexDatabaseError ;
            }
        }

        /**** If current level is above or equal to the QueryLevel
        ***/

        else if (level <= queryLevel) {

            /** For these levels, only unique keys are allowed
            ** Parse the request list elements reffering to
            ** this level.
            ** Check that only unique key attr are provided
            */

            int uniqueKeyFound = OFFalse ;
            for (plist = findRequestList ; plist ; plist = plist->next) {
                DB_GetTagLevel (plist->elem. XTag, &XTagLevel) ;
                if (XTagLevel != level)
                    continue ;
                DB_GetTagKeyAttr (plist->elem. XTag, &XTagType) ;
                if (XTagType != UNIQUE_KEY) {
                    DCMQRDB_DEBUG("Non Unique Key found (level " << level << ")");
                    return DcmQRIndexDatabaseError ;
                }
                else if (uniqueKeyFound) {
                    DCMQRDB_DEBUG("More than one Unique Key found (level " << level << ")");
                    return DcmQRIndexDatabaseError ;
                }
                else
                    uniqueKeyFound = OFTrue ;
            }
            if (! uniqueKeyFound) {
                DCMQRDB_DEBUG("No Unique Key found (level " << level << ")");
                return DcmQRIndexDatabaseError ;
            }
        }

        /**** If current level beyond the QueryLevel
        ***/

        else if (level > queryLevel) {

            /** For this level, no key is allowed
            ** Parse the request list elements reffering to
            ** this level.
            ** Check that no key is provided
            */

            int atLeastOneKeyFound = OFFalse ;
            for (plist = findRequestList ; plist ; plist = plist->next) {
                DB_GetTagLevel (plist->elem. XTag, &XTagLevel) ;
                if (XTagLevel != level)
                    continue ;
                atLeastOneKeyFound = OFTrue ;
            }
            if (atLeastOneKeyFound) {
                DCMQRDB_DEBUG("Key found beyond query level (level " << level << ")");
                return DcmQRIndexDatabaseError ;
            }
        }

    }
    return EC_Normal ;
}

#endif

OFCondition DcmQueryRetrieveSQLDatabaseHandle::startMoveRequest(
        const char      *SOPClassUID,
        DcmDataset      *moveRequestIdentifiers,
        DcmQueryRetrieveDatabaseStatus  *status)
{
  return EC_MemoryExhausted;

#if 0
    DB_SmallDcmElmt     elem ;
    DB_ElementList      *plist = NULL;
    DB_ElementList      *last = NULL;
    DB_CounterList      *pidxlist = NULL;
    DB_CounterList      *lastidxlist = NULL;
    int                 MatchFound = OFFalse;
    IdxRecord           idxRec ;
    DB_LEVEL            qLevel = PATIENT_LEVEL; // highest legal level for a query in the current model
    DB_LEVEL            lLevel = IMAGE_LEVEL;   // lowest legal level for a query in the current model
    OFCondition         cond = EC_Normal;
    OFBool qrLevelFound = OFFalse;

    /**** Is SOPClassUID supported ?
    ***/

    if (strcmp( SOPClassUID, UID_MOVEPatientRootQueryRetrieveInformationModel) == 0)
        handle_->rootLevel = PATIENT_ROOT ;
    else if (strcmp( SOPClassUID, UID_MOVEStudyRootQueryRetrieveInformationModel) == 0)
        handle_->rootLevel = STUDY_ROOT ;
#ifndef NO_PATIENTSTUDYONLY_SUPPORT
    else if (strcmp( SOPClassUID, UID_RETIRED_MOVEPatientStudyOnlyQueryRetrieveInformationModel) == 0)
        handle_->rootLevel = PATIENT_STUDY ;
#endif
#ifndef NO_GET_SUPPORT
    /* experimental support for GET */
    else if (strcmp( SOPClassUID, UID_GETPatientRootQueryRetrieveInformationModel) == 0)
        handle_->rootLevel = PATIENT_ROOT ;
    else if (strcmp( SOPClassUID, UID_GETStudyRootQueryRetrieveInformationModel) == 0)
        handle_->rootLevel = STUDY_ROOT ;
#ifndef NO_PATIENTSTUDYONLY_SUPPORT
    else if (strcmp( SOPClassUID, UID_RETIRED_GETPatientStudyOnlyQueryRetrieveInformationModel) == 0)
        handle_->rootLevel = PATIENT_STUDY ;
#endif
#endif

    else {
        status->setStatus(STATUS_MOVE_Failed_SOPClassNotSupported);
        return (DcmQRIndexDatabaseError) ;
    }


    /**** Parse Identifiers in the Dicom Object
    **** Find Query Level and contruct a list
    **** of query identifiers
    ***/

    int elemCount = (int)(moveRequestIdentifiers->card());
    for (int elemIndex=0; elemIndex<elemCount; elemIndex++) {

        DcmElement* dcelem = moveRequestIdentifiers->getElement(elemIndex);

        elem.XTag = dcelem->getTag().getXTag();
        if (elem.XTag == DCM_QueryRetrieveLevel || DB_TagSupported(elem.XTag)) {
            elem.ValueLength = dcelem->getLength();
            if (elem.ValueLength == 0) {
                elem.PValueField = NULL ;
            } else if ((elem.PValueField = (char*)malloc((size_t)(elem.ValueLength+1))) == NULL) {
                status->setStatus(STATUS_MOVE_Failed_UnableToProcess);
                return (DcmQRIndexDatabaseError) ;
            } else {
                /* only char string type tags are supported at the moment */
                char *s = NULL;
                dcelem->getString(s);
                strcpy(elem.PValueField, s);
            }

            /** If element is the Query Level, store it in handle
             */

            if (elem. XTag == DCM_QueryRetrieveLevel) {
                char *pc ;
                char level [50] ;

                strncpy (level, (char *) elem. PValueField, (size_t)((elem. ValueLength < 50) ? elem. ValueLength : 49)) ;

                /*** Skip this two lines if you want strict comparison
                **/

                for (pc = level ; *pc ; pc++)
                    *pc = ((*pc >= 'a') && (*pc <= 'z')) ? 'A' - 'a' + *pc : *pc ;

                if (strncmp (level, PATIENT_LEVEL_STRING,
                             strlen (PATIENT_LEVEL_STRING)) == 0)
                    handle_->queryLevel = PATIENT_LEVEL ;
                else if (strncmp (level, STUDY_LEVEL_STRING,
                                  strlen (STUDY_LEVEL_STRING)) == 0)
                    handle_->queryLevel = STUDY_LEVEL ;
                else if (strncmp (level, SERIE_LEVEL_STRING,
                                  strlen (SERIE_LEVEL_STRING)) == 0)
                    handle_->queryLevel = SERIE_LEVEL ;
                else if (strncmp (level, IMAGE_LEVEL_STRING,
                                  strlen (IMAGE_LEVEL_STRING)) == 0)
                    handle_->queryLevel = IMAGE_LEVEL ;
                else {
#ifdef DEBUG
                    DCMQRDB_DEBUG("DB_startMoveRequest : STATUS_MOVE_Failed_UnableToProcess");
#endif
                    status->setStatus(STATUS_MOVE_Failed_UnableToProcess);
                    return (DcmQRIndexDatabaseError) ;
                }
                qrLevelFound = OFTrue;
            } else {
                /** Else it is a query identifier
                ** Append it to our RequestList
                */
                if (! DB_TagSupported (elem. XTag))
                    continue ;

                plist = (DB_ElementList *) malloc (sizeof( DB_ElementList ) ) ;
                if (plist == NULL) {
                    status->setStatus(STATUS_FIND_Refused_OutOfResources);
                    return (DcmQRIndexDatabaseError) ;
                }
                plist->next = NULL ;
                DB_DuplicateElement (&elem, & (plist->elem)) ;
                if (handle_->findRequestList == NULL) {
                    handle_->findRequestList = last = plist ;
                } else {
                    last->next = plist ;
                    last = plist ;
                }
            }

            if ( elem. PValueField ) {
                free (elem. PValueField) ;
            }
        }
    }

    if (!qrLevelFound) {
        /* The Query/Retrieve Level is missing */
        status->setStatus(STATUS_MOVE_Failed_IdentifierDoesNotMatchSOPClass);
        DCMQRDB_WARN("DB_startMoveRequest(): missing Query/Retrieve Level");
        handle_->idxCounter = -1 ;
        DB_FreeElementList (handle_->findRequestList) ;
        handle_->findRequestList = NULL ;
        return (DcmQRIndexDatabaseError) ;
    }

    switch (handle_->rootLevel)
    {
      case PATIENT_ROOT :
        qLevel = PATIENT_LEVEL ;
        lLevel = IMAGE_LEVEL ;
        break ;
      case STUDY_ROOT :
        qLevel = STUDY_LEVEL ;
        lLevel = IMAGE_LEVEL ;
        break ;
      case PATIENT_STUDY:
        qLevel = PATIENT_LEVEL ;
        lLevel = STUDY_LEVEL ;
        break ;
    }

    /**** Test the consistency of the request list
    ***/

    if (doCheckMoveIdentifier) {
        cond = testMoveRequestList (handle_->findRequestList,
                                    handle_->queryLevel, qLevel, lLevel) ;
        if (cond != EC_Normal) {
            handle_->idxCounter = -1 ;
            DB_FreeElementList (handle_->findRequestList) ;
            handle_->findRequestList = NULL ;
#ifdef DEBUG
            DCMQRDB_DEBUG("DB_startMoveRequest () : STATUS_MOVE_Failed_IdentifierDoesNotMatchSOPClass - Invalid RequestList");
#endif
            status->setStatus(STATUS_MOVE_Failed_IdentifierDoesNotMatchSOPClass);
            return (cond) ;
        }
    }

    /**** Goto the beginning of Index File
    **** Then find all matching images
    ***/

    MatchFound = OFFalse ;
    handle_->moveCounterList = NULL ;
    handle_->NumberRemainOperations = 0 ;

    /**** Find matching images
    ***/

    DB_lock(OFFalse);

    DB_IdxInitLoop (&(handle_->idxCounter)) ;
    while (1) {

        /*** Exit loop if read error (or end of file)
        **/

        if (DB_IdxGetNext (&(handle_->idxCounter), &idxRec) != EC_Normal)
            break ;

        /*** If matching found
        **/

        cond = hierarchicalCompare (handle_, &idxRec, qLevel, qLevel, &MatchFound) ;
        if (MatchFound) {
            pidxlist = (DB_CounterList *) malloc (sizeof( DB_CounterList ) ) ;
            if (pidxlist == NULL) {
                status->setStatus(STATUS_FIND_Refused_OutOfResources);
                return (DcmQRIndexDatabaseError) ;
            }

            pidxlist->next = NULL ;
            pidxlist->idxCounter = handle_->idxCounter ;
            handle_->NumberRemainOperations++ ;
            if ( handle_->moveCounterList == NULL )
                handle_->moveCounterList = lastidxlist = pidxlist ;
            else {
                lastidxlist->next = pidxlist ;
                lastidxlist = pidxlist ;
            }
        }
    }

    DB_FreeElementList (handle_->findRequestList) ;
    handle_->findRequestList = NULL ;

    /**** If a matching image has been found,
    ****    status is pending
    ***/

    if ( handle_->NumberRemainOperations > 0 ) {
#ifdef DEBUG
        DCMQRDB_DEBUG("DB_startMoveRequest : STATUS_Pending");
#endif
        status->setStatus(STATUS_Pending);
        return (EC_Normal) ;
    }

    /**** else no matching image has been found,
    ****    free query identifiers list
    ****    status is success
    ***/

    else {
        handle_->idxCounter = -1 ;
#ifdef DEBUG
        DCMQRDB_DEBUG("DB_startMoveRequest : STATUS_Success");
#endif
        status->setStatus(STATUS_Success);

        DB_unlock();

        return (EC_Normal) ;
    }

#endif

}

OFCondition DcmQueryRetrieveSQLDatabaseHandle::nextMoveResponse(
                char            *SOPClassUID,
                char            *SOPInstanceUID,
                char            *imageFileName,
                unsigned short  *numberOfRemainingSubOperations,
                DcmQueryRetrieveDatabaseStatus  *status)
{
  return EC_MemoryExhausted;

#if 0
    IdxRecord           idxRec ;
    DB_CounterList              *nextlist ;

    /**** If all matching images have been retrieved,
    ****    status is success
    ***/

    if ( handle_->NumberRemainOperations <= 0 ) {
        status->setStatus(STATUS_Success);

        DB_unlock();

        return (EC_Normal) ;
    }

    /**** Goto the next matching image number of Index File
    ***/

    if (DB_IdxRead (handle_->moveCounterList->idxCounter, &idxRec) != EC_Normal) {
#ifdef DEBUG
        DCMQRDB_DEBUG("DB_nextMoveResponse : STATUS_MOVE_Failed_UnableToProcess");
#endif
        status->setStatus(STATUS_MOVE_Failed_UnableToProcess);

        DB_unlock();

        return (DcmQRIndexDatabaseError) ;
    }

    strcpy (SOPClassUID, (char *) idxRec. SOPClassUID) ;
    strcpy (SOPInstanceUID, (char *) idxRec. SOPInstanceUID) ;
    strcpy (imageFileName, (char *) idxRec. filename) ;

    *numberOfRemainingSubOperations = --handle_->NumberRemainOperations ;

    nextlist = handle_->moveCounterList->next ;
    free (handle_->moveCounterList) ;
    handle_->moveCounterList = nextlist ;
    status->setStatus(STATUS_Pending);
#ifdef DEBUG
    DCMQRDB_DEBUG("DB_nextMoveResponse : STATUS_Pending");
#endif
    return (EC_Normal) ;

#endif
}



OFCondition DcmQueryRetrieveSQLDatabaseHandle::cancelMoveRequest (DcmQueryRetrieveDatabaseStatus *status)
{
  return EC_MemoryExhausted;

#if 0
    DB_CounterList *plist ;

    while (handle_->moveCounterList) {
        plist  = handle_->moveCounterList ;
        handle_->moveCounterList = handle_->moveCounterList->next ;
        free (plist) ;
    }

    status->setStatus(STATUS_MOVE_Cancel_SubOperationsTerminatedDueToCancelIndication);

    DB_unlock();

    return (EC_Normal) ;
#endif
}


/* ========================= STORE ========================= */


void DcmQueryRetrieveSQLDatabaseHandle::enableQuotaSystem(OFBool enable)
{
    quotaSystemEnabled = enable;
}


#if 0
/*
** Image file deleting
*/


OFCondition DcmQueryRetrieveSQLDatabaseHandle::deleteImageFile(char* imgFile)
{
    if (!quotaSystemEnabled) {
      DCMQRDB_WARN("file delete operations are disabled, keeping file: " << imgFile << " despite duplicate SOP Instance UID");
      return EC_Normal;
    } else {
      DCMQRDB_WARN("Deleting file: " << imgFile << " due to quota or duplicate SOP instance UID");
    }

#ifdef LOCK_IMAGE_FILES
    int lockfd;
#ifdef O_BINARY
    lockfd = open(imgFile, O_RDWR | O_BINARY, 0666);    /* obtain file descriptor */
#else
    lockfd = open(imgFile, O_RDWR, 0666);   /* obtain file descriptor */
#endif
    if (lockfd < 0) {
      DCMQRDB_WARN("DB ERROR: cannot open image file for deleting: " << imgFile);
      return DcmQRIndexDatabaseError;
    }
    if (dcmtk_flock(lockfd, LOCK_EX) < 0) { /* exclusive lock (blocking) */
      DCMQRDB_WARN("DB ERROR: cannot lock image file  for deleting: " << imgFile);
      dcmtk_plockerr("DB ERROR");
    }
#endif

    if (unlink(imgFile) < 0) {
        char buf[256];
        /* delete file */
        DCMQRDB_ERROR("DB ERROR: cannot delete image file: " << imgFile << OFendl
            << "DcmQRIndexDatabaseError: " << OFStandard::strerror(errno, buf, sizeof(buf)));
    }

#ifdef LOCK_IMAGE_FILES
    if (dcmtk_flock(lockfd, LOCK_UN) < 0) { /* unlock */
        DCMQRDB_WARN("DB ERROR: cannot unlock image file  for deleting: " << imgFile);
        dcmtk_plockerr("DB ERROR");
     }
    close(lockfd);              /* release file descriptor */
#endif

    return EC_Normal;
}


/*************************
**   Delete oldest study in database
 */

int DcmQueryRetrieveSQLDatabaseHandle::deleteOldestStudy(StudyDescRecord *pStudyDesc)
{

    int oldestStudy ;
    double OldestDate ;
    int s ;
    int n ;
    int idx = 0 ;
    IdxRecord idxRec ;

    oldestStudy = 0 ;
    OldestDate = 0.0 ;

#ifdef DEBUG
    DCMQRDB_DEBUG("deleteOldestStudy");
#endif

    for ( s = 0 ; s < handle_ -> maxStudiesAllowed ; s++ ) {
        if ( ( pStudyDesc[s]. NumberofRegistratedImages != 0 ) &&
            ( ( OldestDate == 0.0 ) || ( pStudyDesc[s]. LastRecordedDate < OldestDate ) ) ) {
            OldestDate = pStudyDesc[s]. LastRecordedDate ;
            oldestStudy = s ;
        }
    }

#ifdef DEBUG
    DCMQRDB_DEBUG("deleteOldestStudy oldestStudy = " << oldestStudy);
#endif

    n = strlen(pStudyDesc[oldestStudy].StudyInstanceUID) ;
    while ( DB_IdxRead (idx, &idxRec) == EC_Normal ) {

    if ( ! ( strncmp(idxRec. StudyInstanceUID, pStudyDesc[oldestStudy].StudyInstanceUID, n) ) ) {
        DB_IdxRemove (idx) ;
        deleteImageFile(idxRec.filename);
    }
    idx++ ;
    }

    pStudyDesc[oldestStudy].NumberofRegistratedImages = 0 ;
    pStudyDesc[oldestStudy].StudySize = 0 ;
    return(oldestStudy) ;
}

#endif


#if 0
/*************************
**   Delete oldest images in database
 */

OFCondition DcmQueryRetrieveSQLDatabaseHandle::deleteOldestImages(StudyDescRecord *pStudyDesc, int StudyNum, char *StudyUID, long RequiredSize)
{
  return EC_MemoryExhausted;


//    ImagesofStudyArray *StudyArray ;
//    IdxRecord idxRec ;
//    int nbimages = 0 , s = 0 , n ;
//    long DeletedSize ;
//
//#ifdef DEBUG
//    DCMQRDB_DEBUG("deleteOldestImages RequiredSize = " << RequiredSize);
//#endif
//    n = strlen(StudyUID) ;
//    StudyArray = (ImagesofStudyArray *)malloc(MAX_NUMBER_OF_IMAGES * sizeof(ImagesofStudyArray)) ;
//
//    if (StudyArray == NULL) {
//        DCMQRDB_WARN("deleteOldestImages: out of memory");
//        return DcmQRIndexDatabaseError;
//    }
//
//    /** Find all images having the same StudyUID
//     */
//
//    DB_IdxInitLoop (&(handle_ -> idxCounter)) ;
//    while ( DB_IdxGetNext(&(handle_ -> idxCounter), &idxRec) == EC_Normal ) {
//    if ( ! ( strncmp(idxRec. StudyInstanceUID, StudyUID, n) ) ) {
//
//        StudyArray[nbimages]. idxCounter = handle_ -> idxCounter ;
//        StudyArray[nbimages]. RecordedDate = idxRec. RecordedDate ;
//        StudyArray[nbimages++]. ImageSize = idxRec. ImageSize ;
//    }
//    }
//
//    /** Sort the StudyArray in order to have the oldest images first
//     */
//    qsort((char *)StudyArray, nbimages, sizeof(ImagesofStudyArray), DB_Compare) ;
//
//#ifdef DEBUG
//    {
//        int i ;
//        DCMQRDB_DEBUG("deleteOldestImages : Sorted images ref array");
//        for (i = 0 ; i < nbimages ; i++)
//            DCMQRDB_DEBUG("[" << STD_NAMESPACE setw(2) << i << "] :   Size " << StudyArray[i].ImageSize
//                << "   Date " << STD_NAMESPACE setw(20) << STD_NAMESPACE setprecision(3) << StudyArray[i].RecordedDate
//                << "   Ref " << StudyArray[i].idxCounter);
//        DCMQRDB_DEBUG("deleteOldestImages : end of ref array");
//    }
//#endif
//
//    s = 0 ;
//    DeletedSize = 0 ;
//
//    while ( DeletedSize < RequiredSize ) {
//
//    IdxRecord idxRemoveRec ;
//    DB_IdxRead (StudyArray[s]. idxCounter, &idxRemoveRec) ;
//#ifdef DEBUG
//    DCMQRDB_DEBUG("Removing file : " << idxRemoveRec. filename);
//#endif
//    deleteImageFile(idxRemoveRec.filename);
//
//    DB_IdxRemove (StudyArray[s]. idxCounter) ;
//    pStudyDesc[StudyNum].NumberofRegistratedImages -= 1 ;
//    pStudyDesc[StudyNum].StudySize -= StudyArray[s]. ImageSize ;
//    DeletedSize += StudyArray[s++]. ImageSize ;
//    }
//
//#ifdef DEBUG
//    DCMQRDB_DEBUG("deleteOldestImages DeletedSize = " << (int)DeletedSize);
//#endif
//    free(StudyArray) ;
//    return( EC_Normal ) ;

}
#endif 

#if 0
/*************************
 *   Verify if study UID already exists
 *   If the study UID exists, its index in the study descriptor is returned.
 *   If the study UID does not exist, the index of the first unused descriptor entry is returned.
 *   If no entries are free, maxStudiesAllowed is returned.
 */

int DcmQueryRetrieveSQLDatabaseHandle::matchStudyUIDInStudyDesc (StudyDescRecord *pStudyDesc, char *StudyUID, int maxStudiesAllowed)
{
    int s = 0 ;
    while  (s < maxStudiesAllowed)
    {
      if ((pStudyDesc[s].NumberofRegistratedImages > 0) && (0 == strcmp(pStudyDesc[s].StudyInstanceUID, StudyUID))) break;
      s++ ;
    }
    if (s==maxStudiesAllowed) // study uid does not exist, look for free descriptor
    {
      s=0;
      while  (s < maxStudiesAllowed)
      {
        if (pStudyDesc[s].NumberofRegistratedImages == 0) break;
        s++ ;
      }
    }
    return s;
}


/*************************
**  Check up storage rights in Study Desk record
 */

OFCondition DcmQueryRetrieveSQLDatabaseHandle::checkupinStudyDesc(StudyDescRecord *pStudyDesc, char *StudyUID, long imageSize)
{
    int         s ;
    long        RequiredSize ;

    s = matchStudyUIDInStudyDesc (pStudyDesc, StudyUID,
                     (int)(handle_ -> maxStudiesAllowed)) ;

    /** If Study already exists
     */

    if ( pStudyDesc[s]. NumberofRegistratedImages != 0 ) {

#ifdef DEBUG
    DCMQRDB_DEBUG("checkupinStudyDesc: study already exists : " << s) ;
#endif
    if ( ( pStudyDesc[s]. StudySize + imageSize )
         > handle_ -> maxBytesPerStudy ) {
        if ( imageSize > handle_ -> maxBytesPerStudy ) {
#ifdef DEBUG
            DCMQRDB_DEBUG("checkupinStudyDesc: imageSize = " << imageSize << " too large");
#endif
            return ( DcmQRIndexDatabaseError ) ;
        }

        RequiredSize = imageSize -
            ( handle_ -> maxBytesPerStudy - pStudyDesc[s]. StudySize ) ;
        deleteOldestImages(pStudyDesc, s, StudyUID, RequiredSize) ;
    }


    }
    else {
#ifdef DEBUG
    DCMQRDB_DEBUG("checkupinStudyDesc: study doesn't already exist");
#endif
    if ( imageSize > handle_ -> maxBytesPerStudy ) {
#ifdef DEBUG
        DCMQRDB_DEBUG("checkupinStudyDesc: imageSize = " << imageSize << " too large");
#endif
        return ( DcmQRIndexDatabaseError ) ;
    }
    if ( s > ( handle_ -> maxStudiesAllowed - 1 ) )
        s = deleteOldestStudy(pStudyDesc) ;

    }

    pStudyDesc[s]. StudySize += imageSize ;
#ifdef DEBUG
    DCMQRDB_DEBUG("checkupinStudyDesc: ~~~~~~~~ StudySize = " << pStudyDesc[s]. StudySize);
#endif

    /* we only have second accuracy */
    pStudyDesc[s]. LastRecordedDate =  (double) time(NULL);

    pStudyDesc[s]. NumberofRegistratedImages++ ;
    strcpy(pStudyDesc[s].StudyInstanceUID,StudyUID) ;

    if ( DB_StudyDescChange (pStudyDesc) == EC_Normal)
        return ( EC_Normal ) ;
    else
        return ( DcmQRIndexDatabaseError ) ;
}

/*
 * If the image is already stored remove it from the database.
 * hewett - Nov. 1, 93
 */
OFCondition DcmQueryRetrieveSQLDatabaseHandle::removeDuplicateImage(
    const char *SOPInstanceUID, const char *StudyInstanceUID,
    StudyDescRecord *pStudyDesc, const char *newImageFileName)
{

    int idx = 0;
    IdxRecord idxRec ;
    int studyIdx = 0;

    studyIdx = matchStudyUIDInStudyDesc (pStudyDesc, (char*)StudyInstanceUID,
                        (int)(handle_ -> maxStudiesAllowed)) ;

    if ( pStudyDesc[studyIdx].NumberofRegistratedImages == 0 ) {
    /* no study images, cannot be any old images */
    return EC_Normal;
    }

    while (DB_IdxRead(idx, &idxRec) == EC_Normal) {

    if (strcmp(idxRec.SOPInstanceUID, SOPInstanceUID) == 0) {

#ifdef DEBUG
        DCMQRDB_DEBUG("--- Removing Existing DB Image Record: " << idxRec.filename);
#endif
        /* remove the idx record  */
        DB_IdxRemove (idx);
        /* only remove the image file if it is different than that
         * being entered into the database.
         */
        if (strcmp(idxRec.filename, newImageFileName) != 0) {
            deleteImageFile(idxRec.filename);
        }
        /* update the study info */
        pStudyDesc[studyIdx].NumberofRegistratedImages--;
        pStudyDesc[studyIdx].StudySize -= idxRec.ImageSize;
    }
    idx++;
    }
    /* the study record should be written to file later */
    return EC_Normal;
}
#endif

OFCondition PopulateIdxRecFromImageFile(
                                 const char *imageFileName
                                 , IdxRecord &idxRec
                                 , DcmFileFormat& dcmff
                                 , DcmQueryRetrieveDatabaseStatus *status)
{
    if (dcmff.loadFile(imageFileName).bad())
    {
      char buf[256];
      DCMQRDB_WARN("DB: Cannot open file: " << imageFileName << ": "
          << OFStandard::strerror(errno, buf, sizeof(buf)));
      status->setStatus(STATUS_STORE_Error_CannotUnderstand);
      return (DcmQRSqlDatabaseError) ;
    }

    DcmDataset *dset = dcmff.getDataset();

    for (int i = 0 ; i < NBPARAMETERS ; i++ ) {
        OFCondition ec = EC_Normal;
        DB_SmallDcmElmt *se = idxRec.param + i;

        const char *strPtr = NULL;
        ec = dset->findAndGetString(se->XTag, strPtr);
        if ((ec != EC_Normal) || (strPtr == NULL)) {
            /* not found or empty */
            se->PValueField[0] = '\0';
            se->ValueLength = 0;
        } else {
            /* found and non-empty */
            strncpy(se->PValueField, strPtr, (size_t)(se->ValueLength));
            /* important: do not change the ValueLength field before the string is copied! */
            se->ValueLength = strlen(se->PValueField);
        }
    }

    return EC_Normal;
}

void ExtractInstanceDescription(char const*SOPClassUID, IdxRecord &idxRec, DcmDataset *dset, bool &useDescrTag, DcmTagKey &descrTag)
{
  if (SOPClassUID != NULL)
  {
    /* fill in value depending on SOP class UID (content might be improved) */
    if (strcmp(SOPClassUID, UID_GrayscaleSoftcopyPresentationStateStorage) == 0)
    {
      descrTag = DCM_ContentDescription;
    } else if (strcmp(SOPClassUID, UID_RETIRED_HardcopyGrayscaleImageStorage) == 0)
    {
      strcpy(idxRec.InstanceDescription, "Hardcopy Grayscale Image");
      useDescrTag = OFFalse;
    } else if ((strcmp(SOPClassUID, UID_BasicTextSRStorage) == 0) ||
      (strcmp(SOPClassUID, UID_EnhancedSRStorage) == 0) ||
      (strcmp(SOPClassUID, UID_ComprehensiveSRStorage) == 0) ||
      (strcmp(SOPClassUID, UID_ProcedureLogStorage) == 0) ||
      (strcmp(SOPClassUID, UID_MammographyCADSRStorage) == 0) ||
      (strcmp(SOPClassUID, UID_KeyObjectSelectionDocumentStorage) == 0) ||
      (strcmp(SOPClassUID, UID_ChestCADSRStorage) == 0) ||
      (strcmp(SOPClassUID, UID_ColonCADSRStorage) == 0) ||
      (strcmp(SOPClassUID, UID_XRayRadiationDoseSRStorage) == 0) ||
      (strcmp(SOPClassUID, UID_SpectaclePrescriptionReportStorage) == 0) ||
      (strcmp(SOPClassUID, UID_MacularGridThicknessAndVolumeReportStorage) == 0) ||
      (strcmp(SOPClassUID, UID_ImplantationPlanSRDocumentStorage) == 0))
    {
      OFString string;
      OFString description = "unknown SR";
      const char *name = dcmFindNameOfUID(SOPClassUID);
      if (name != NULL)
        description = name;
      if (dset->findAndGetOFString(DCM_VerificationFlag, string) == EC_Normal)
      {
        description += ", ";
        description += string;
      }
      if (dset->findAndGetOFString(DCM_CompletionFlag, string) == EC_Normal)
      {
        description += ", ";
        description += string;
      }
      if (dset->findAndGetOFString(DCM_CompletionFlagDescription, string) == EC_Normal)
      {
        description += ", ";
        description += string;
      }
      strncpy(idxRec.InstanceDescription, description.c_str(), DESCRIPTION_MAX_LENGTH);
      useDescrTag = OFFalse;
    } else if (strcmp(SOPClassUID, UID_RETIRED_StoredPrintStorage) == 0)
    {
      strcpy(idxRec.InstanceDescription, "Stored Print");
      useDescrTag = OFFalse;
    }
  }

    /* get description from attribute specified above */
    if (useDescrTag)
    {
        OFString string;
        /* return value is irrelevant */
        dset->findAndGetOFString(descrTag, string);
        strncpy(idxRec.InstanceDescription, string.c_str(), DESCRIPTION_MAX_LENGTH);
    }
    /* is dataset digitally signed? */
    if (strlen(idxRec.InstanceDescription) + 9 < DESCRIPTION_MAX_LENGTH)
    {
        DcmStack stack;
        if (dset->search(DCM_DigitalSignaturesSequence, stack, ESM_fromHere, OFTrue /* searchIntoSub */) == EC_Normal)
        {
            /* in principle it should be checked whether there is _any_ non-empty digital signatures sequence, but ... */
            if (((DcmSequenceOfItems *)stack.top())->card() > 0)
            {
                if (strlen(idxRec.InstanceDescription) > 0)
                    strcat(idxRec.InstanceDescription, " (Signed)");
                else
                    strcpy(idxRec.InstanceDescription, "Signed Instance");
            }
        }
    }
}
void PrintIdxRecord(IdxRecord &idxRec)
{
  DCMQRDB_DEBUG("-- BEGIN Parameters to Register in DB");
  for (int i = 0 ; i < NBPARAMETERS ; i++) {  /* new definition */
    DB_SmallDcmElmt *se = idxRec.param + i;
    const char* value = "";
    if (se->PValueField != NULL) value = se->PValueField;
    DcmTag tag(se->XTag);
    DCMQRDB_DEBUG(" " << tag.getTagName() << ": \"" << value << "\"");
  }
  DCMQRDB_DEBUG("-- END Parameters to Register in DB");
}

/*************************
**  Add data from imageFileName to database
 */

OFCondition DcmQueryRetrieveSQLDatabaseHandle::storeRequest (
    const char  *SOPClassUID,
    const char  * /*SOPInstanceUID*/,
    const char  *imageFileName,
    DcmQueryRetrieveDatabaseStatus *status,
    OFBool      isNew)
{
    IdxRecord idxRec ;
    bzero((char*)&idxRec, sizeof(idxRec));
    DB_IdxInitRecord(&idxRec, 0) ;

    strncpy(idxRec.filename, imageFileName, DBC_MAXSTRING);
#ifdef DEBUG
    DCMQRDB_DEBUG("DB_storeRequest () : storage request of file : " << idxRec.filename);
#endif
    strncpy (idxRec.SOPClassUID, SOPClassUID, UI_MAX_LENGTH);

    DcmFileFormat dcmff;
    PopulateIdxRecFromImageFile(imageFileName, idxRec, dcmff, status);
    DcmDataset *dset = dcmff.getDataset();

    /* InstanceStatus */
    idxRec.hstat = (isNew) ? DVIF_objectIsNew : DVIF_objectIsNotNew;

    /* InstanceDescription */
    OFBool useDescrTag = OFTrue;
    DcmTagKey descrTag = DCM_ImageComments;
    ExtractInstanceDescription(SOPClassUID, idxRec, dset, useDescrTag, descrTag);

#ifdef DEBUG
    PrintIdxRecord(idxRec);
#endif

    {
      BOOL bRes = FALSE;

      //TODO: maybe should use Table Value Parameter (TVP) in MS SQL2008 
      //but info to to scarce to find out how to actully use it
      //http://msdn.microsoft.com/en-us/library/bb510489.aspx

      CAutoPtr<IDbCommand> pCmd(piDbSystem_->CreateCommand(piDbDatabase_));
      bRes = pCmd->Create(_T(
          "EXEC [dcmqrdb_mssql].[dbo].[spRegisterDcmInstance]"
          "  @studyUiid = ?"
          ", @seriesUiid = ?"
          ", @instanceUiid = ?"
          ", @fileName = ?"
          ";"
          ));
      bRes = pCmd->SetParam(0, CA2T(idxRec.StudyInstanceUID));
      bRes = pCmd->SetParam(1, CA2T(idxRec.SeriesInstanceUID));
      bRes = pCmd->SetParam(2, CA2T(idxRec.SOPInstanceUID));
      bRes = pCmd->SetParam(3, CA2T(idxRec.filename));
      CAutoPtr<IDbRecordset> pRec(piDbSystem_->CreateRecordset(piDbDatabase_));
      bRes = pCmd->Execute(pRec);

      //TODO: this seems screwed up, a remote call to sql server for 
      //every tag!!??, but as noted above is is not clear how to TVP 
      //and hard coding every tag in stored proc seems even more stupid
      //TODO: at least make it work against an instance key (get from above)
      //to avoid another lookup in the database
      for(int i = 0; i < NBPARAMETERS; i++){
          //TODO: check turn code
          bRes = pCmd->Create(_T(
              "EXEC [dcmqrdb_mssql].[dbo].[spRegisterDcmTag]"
              "  @studyUiid = ?"
              ", @instanceUiid = ?"
              ", @AttributeTag =  ?"
              ", @attributeValue = ?"
              ));
          bRes = pCmd->SetParam(0, CA2T((idxRec).StudyInstanceUID));
          bRes = pCmd->SetParam(1, CA2T((idxRec).SOPInstanceUID));
          long tagGroupElement = GetTagGroupElement(idxRec.param[i].XTag);
          bRes = pCmd->SetParam(2, &tagGroupElement);
          bRes = pCmd->SetParam(3, CA2T(idxRec.param[i].PValueField));
          bRes = pCmd->Execute(pRec);
      }

      //TODO: check return status
      //TODO: have the stored procedure return the path for previously
      //registered file (if any) and remove that file
    }

    return EC_Normal;

#if 0
    DB_lock(OFTrue);

    bzero((char *)pStudyDesc, SIZEOF_STUDYDESC);
    DB_GetStudyDesc(pStudyDesc) ;

    stat(imageFileName, &stat_buf) ;
    idxRec. ImageSize = (int)(stat_buf. st_size) ;

    /* we only have second accuracy */
    idxRec. RecordedDate =  (double) time(NULL);

    /*
     * If the image is already stored remove it from the database.
     * hewett - Nov. 1, 93
     */

    removeDuplicateImage(idxRec.SOPInstanceUID,
                idxRec.StudyInstanceUID, pStudyDesc,
                imageFileName);


    if ( checkupinStudyDesc(pStudyDesc, idxRec. StudyInstanceUID, idxRec. ImageSize) != EC_Normal ) {
        free (pStudyDesc) ;
        status->setStatus(STATUS_STORE_Refused_OutOfResources);

        DB_unlock();

        return (DcmQRIndexDatabaseError) ;
    }

    free (pStudyDesc) ;

    if (DB_IdxAdd (handle_, &i, &idxRec) == EC_Normal)
    {
        status->setStatus(STATUS_Success);
        DB_unlock();
        return (EC_Normal) ;
    }
    else
    {
        status->setStatus(STATUS_STORE_Refused_OutOfResources);
        DB_unlock();
    }
    return DcmQRIndexDatabaseError;

#endif
}

/*
** Prune invalid DB records.
*/

OFCondition DcmQueryRetrieveSQLDatabaseHandle::pruneInvalidRecords()
{
  return EC_MemoryExhausted;

#if 0
  int idx = 0;
    IdxRecord idxRec ;
    StudyDescRecord *pStudyDesc;

    DB_lock(OFTrue);

    pStudyDesc = (StudyDescRecord *)malloc (SIZEOF_STUDYDESC) ;
    if (pStudyDesc == NULL) {
      DCMQRDB_WARN("DB_pruneInvalidRecords: out of memory");
      DB_unlock();
      return (DcmQRIndexDatabaseError) ;
    }

    for (int i = 0 ; i < handle_ -> maxStudiesAllowed ; i++ )
      pStudyDesc[i]. NumberofRegistratedImages = 0 ;

    DB_GetStudyDesc(pStudyDesc) ;

    while (DB_IdxRead(idx, &idxRec) == EC_Normal)
    {
      if (access(idxRec.filename, R_OK) < 0)
      {
#ifdef DEBUG
        DCMQRDB_DEBUG("*** Pruning Invalid DB Image Record: " << idxRec.filename);
#endif
        /* update the study info */
        int studyIdx = matchStudyUIDInStudyDesc(pStudyDesc, idxRec.StudyInstanceUID, (int)(handle_->maxStudiesAllowed)) ;
        if (studyIdx < handle_->maxStudiesAllowed)
        {
          if (pStudyDesc[studyIdx].NumberofRegistratedImages > 0)
          {
            pStudyDesc[studyIdx].NumberofRegistratedImages--;
          } else {
            pStudyDesc[studyIdx].NumberofRegistratedImages = 0;
            pStudyDesc[studyIdx].StudySize = 0;
            pStudyDesc[studyIdx].StudyInstanceUID[0] = '\0';
          }
          if (pStudyDesc[studyIdx].StudySize > idxRec.ImageSize)
          {
            pStudyDesc[studyIdx].StudySize -= idxRec.ImageSize;
          }
        }

        /* remove the idx record  */
        DB_IdxRemove (idx);
      }
      idx++;
    }

    DB_StudyDescChange (pStudyDesc);
    DB_unlock();
    free (pStudyDesc) ;
    return EC_Normal;

#endif 
}


/* ========================= INDEX ========================= */

#if 0
/************************
 *      Dump an index file
 */

void DcmQueryRetrieveSQLDatabaseHandle::printIndexFile (char *storeArea)
{
    int i ;
    int j ;
    IdxRecord           idxRec ;
    StudyDescRecord     *pStudyDesc;

    OFCondition result;
    DcmQueryRetrieveSQLDatabaseHandle handle(storeArea, -1, -1, result);
    if (result.bad()) return;

    pStudyDesc = (StudyDescRecord *)malloc (SIZEOF_STUDYDESC) ;
    if (pStudyDesc == NULL) {
        DCMQRDB_ERROR("printIndexFile: out of memory");
        return;
    }

    handle.DB_lock(OFFalse);

    handle.DB_GetStudyDesc(pStudyDesc);

    for (i=0; i<handle.handle_->maxStudiesAllowed; i++) {
        if (pStudyDesc[i].NumberofRegistratedImages != 0 ) {
            COUT << "******************************************************" << OFendl
                << "STUDY DESCRIPTOR: " << i << OFendl
                << "  Study UID: " << pStudyDesc[i].StudyInstanceUID << OFendl
                << "  StudySize: " << pStudyDesc[i].StudySize << OFendl
                << "  LastRecDate: " << pStudyDesc[i].LastRecordedDate << OFendl
                << "  NumOfImages: " << pStudyDesc[i].NumberofRegistratedImages << OFendl;
        }
    }

    handle.DB_IdxInitLoop (&j) ;
    while (1) {
        if (handle.DB_IdxGetNext(&j, &idxRec) != EC_Normal)
            break ;

        COUT << "*******************************************************" << OFendl;
        COUT << "RECORD NUMBER: " << j << OFendl << "  Status: ";
        if (idxRec.hstat == DVIF_objectIsNotNew)
            COUT << "is NOT new" << OFendl;
        else
            COUT << "is new" << OFendl;
        COUT << "  Filename: " << idxRec.filename << OFendl
             << "  ImageSize: " << idxRec.ImageSize << OFendl
             << "  RecordedDate: " << idxRec.RecordedDate << OFendl;
        for (i = 0 ; i < NBPARAMETERS ; i++) {  /* new definition */
            DB_SmallDcmElmt *se = idxRec.param + i;
            const char* value = "";
            if (se->PValueField != NULL) value = se->PValueField;
            DcmTag tag(se->XTag);
            COUT << "    " << tag.getTagName() << ": \"" << value << "\"" << OFendl;
        }
        COUT << "  InstanceDescription: \"" << idxRec.InstanceDescription << "\"" << OFendl;
    }
    COUT << "*******************************************************" << OFendl
         << "RECORDS IN THIS INDEXFILE: " << j << OFendl;

    handle.DB_unlock();

}
#endif

/* ========================= UTILS ========================= */
#if 0

const char *DcmQueryRetrieveSQLDatabaseHandle::getStorageArea() const
{
    return handle_->storageArea;
}

const char *DcmQueryRetrieveSQLDatabaseHandle::getIndexFilename() const
{
    return handle_->indexFilename;
}
#endif
void DcmQueryRetrieveSQLDatabaseHandle::setIdentifierChecking(OFBool checkFind, OFBool checkMove)
{

    doCheckFindIdentifier = checkFind;
    doCheckMoveIdentifier = checkMove;
}


/***********************
 *      Creates a handle
 */

DcmQueryRetrieveSQLDatabaseHandle::DcmQueryRetrieveSQLDatabaseHandle(
    const char *connectionString,
    const char *storageArea,
    long maxStudiesPerStorageArea,
    long maxBytesPerStudy,
    OFCondition& result)
    :   quotaSystemEnabled(OFTrue)
    , doCheckFindIdentifier(OFFalse)
    , doCheckMoveIdentifier(OFFalse)
    , fnamecreator()
{
    handle_ = NULL;

    //TODO: clean up the strcpy
    strcpy(storageArea_, storageArea);

    BOOL bRet = FALSE;
    bRet = OpenDbSystem(0, DB_SYSTEM_OLEDB, &piDbSystem_);

    CAutoPtr<IDbDatabase> pDb(piDbSystem_->CreateDatabase());
    BOOL bRes = pDb->Open(NULL, CA2T(connectionString), _T(""), _T(""), DB_OPEN_READ_ONLY);
    if( !bRes ) {
        TCHAR szMsg[256];
        pDb->GetErrors()->GetError(0)->GetMessage(szMsg, 256);
        ::MessageBox( NULL, szMsg, _T("Database Test"), MB_OK|MB_ICONERROR);
        //return EC_MemoryExhausted;
    }

    piDbDatabase_ = pDb.Detach();


#ifdef DEBUG
    DCMQRDB_DEBUG("DB_createHandle () : Handle created for " << storageArea);
    DCMQRDB_DEBUG("                     maxStudiesPerStorageArea: " << maxStudiesPerStorageArea
        << " maxBytesPerStudy: " << maxBytesPerStudy);
#endif

    if (maxStudiesPerStorageArea > DB_UpperMaxStudies) {
        DCMQRDB_WARN("maxStudiesPerStorageArea too large" << OFendl
            << "        setting to " << DB_UpperMaxStudies);
        maxStudiesPerStorageArea = DB_UpperMaxStudies;
    }
    if (maxStudiesPerStorageArea < 0) {
        maxStudiesPerStorageArea = DB_UpperMaxStudies;
    }
    if (maxBytesPerStudy < 0 || maxBytesPerStudy > DB_UpperMaxBytesPerStudy) {
        maxBytesPerStudy = DB_UpperMaxBytesPerStudy;
    }
}

/***********************
 *      Destroys a handle
 */

DcmQueryRetrieveSQLDatabaseHandle::~DcmQueryRetrieveSQLDatabaseHandle()
{
#if 0
    int closeresult;

    if (handle_)
    {
#ifndef _WIN32
      /* should not be necessary because we are closing the file handle anyway.
       * On Unix systems this does no harm, but on Windows the unlock fails
       * if the file was not locked before
       * and this gives an unnecessary error message on stderr.
       */
      DB_unlock();
#endif
      closeresult = close( handle_ -> pidx);

      /* Free lists */
      DB_FreeElementList (handle_ -> findRequestList);
      DB_FreeElementList (handle_ -> findResponseList);
      DB_FreeUidList (handle_ -> uidList);

      delete handle_;
    }
#endif
}

/**********************************
 *      Provides a storage filename
 */
//TODO: This is not applicable for a database, but 

OFCondition DcmQueryRetrieveSQLDatabaseHandle::makeNewStoreFileName(
                const char      *SOPClassUID,
                const char      * /* SOPInstanceUID */ ,
                char            *newImageFileName)
{
  OFString filename;
  char prefix[12];

  const char *m = dcmSOPClassUIDToModality(SOPClassUID);
  if (m==NULL) m = "XX";
  sprintf(prefix, "%s_", m);
  // unsigned int seed = fnamecreator.hashString(SOPInstanceUID);
  unsigned int seed = (unsigned int)time(NULL);
  newImageFileName[0]=0; // return empty string in case of error
  if (! fnamecreator.makeFilename(seed, this->storageArea_, prefix, ".dcm", filename)){
    return DcmQRSqlDatabaseError;
  }

  strcpy(newImageFileName, filename.c_str());
  return EC_Normal;

}


#if 0
OFCondition DcmQueryRetrieveSQLDatabaseHandle::instanceReviewed(int idx)
{
    // acquire shared lock and read record at index position
    OFCondition result = DB_lock(OFFalse);
    if (result.bad()) return result;
    IdxRecord record;
    result = DB_IdxRead(idx, &record);
    DB_unlock();

    if (result.good() && (record.hstat != DVIF_objectIsNotNew))
    {
      // acquire exclusive lock and update flag
      result = DB_lock(OFTrue);
      if (result.bad()) return result;

      record.hstat = DVIF_objectIsNotNew;
      DB_lseek(handle_->pidx, OFstatic_cast(long, SIZEOF_STUDYDESC + idx * SIZEOF_IDXRECORD), SEEK_SET);
      write(handle_->pidx, OFreinterpret_cast(char *, &record), SIZEOF_IDXRECORD);
      DB_lseek(handle_->pidx, 0L, SEEK_SET);
      DB_unlock();
    }

    return result;
}
#endif

DcmQueryRetrieveSQLDatabaseHandleFactory::DcmQueryRetrieveSQLDatabaseHandleFactory(const DcmQueryRetrieveConfig *config)
: DcmQueryRetrieveDatabaseHandleFactory()
, config_(config)
{
}

DcmQueryRetrieveSQLDatabaseHandleFactory::~DcmQueryRetrieveSQLDatabaseHandleFactory()
{
}

DcmQueryRetrieveDatabaseHandle *DcmQueryRetrieveSQLDatabaseHandleFactory::createDBHandle(
    const char * /* callingAETitle */,
    const char *calledAETitle,
    OFCondition& result) const
{
  return new DcmQueryRetrieveSQLDatabaseHandle(
//TODO: get from config
    //"Provider=SQLOLEDB;Data Source=localhost\\SQLEXPRESS;Persist Security Info=True;User ID=t;Password=t",
    //"Provider=SQLOLEDB;Data Source=localhost\\SQLEXPRESS;"
    config_->getConnectionString(calledAETitle),
    config_->getStorageArea(calledAETitle),
    config_->getMaxStudies(calledAETitle),
    config_->getMaxBytesPerStudy(calledAETitle), result);
}
