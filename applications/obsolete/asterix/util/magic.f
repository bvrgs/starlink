*+  MAGIC - sets magic values according to QUALITY
      SUBROUTINE MAGIC(STATUS)
*    Description :
*    Authors :
*      BHVAD::RJV
*    History :
*    Type definitions :
      IMPLICIT NONE
*    Global constants :
      INCLUDE 'SAE_PAR'
      INCLUDE 'DAT_PAR'
      INCLUDE 'PAR_ERR'
      INCLUDE 'PRM_PAR'
*    Status :
      INTEGER STATUS
*    Function declarations :
*    Local constants :
*    Local variables :
      CHARACTER*(DAT__SZLOC) ILOC
      CHARACTER*(DAT__SZLOC) OLOC
      CHARACTER*(DAT__SZTYP) TYPE
      CHARACTER*8 MSTR
      REAL MAGVAL
      INTEGER IDIM,NDIM,DIMS(DAT__MXDIM),LEN
      INTEGER DPTR,QPTR
      BYTE MASK
      LOGICAL OVER
      LOGICAL PRIM
      LOGICAL DOK,QOK

*    Version :
      CHARACTER*30 VERSION
      PARAMETER (VERSION = 'MAGIC Version 1.2-0')
*-
      CALL MSG_PRNT(VERSION)

      CALL AST_INIT()

      CALL PAR_GET0L('OVER',OVER,STATUS)

*  overwrite case - input becomes output
      IF (OVER) THEN
        CALL USI_ASSOCI('INP','UPDATE',OLOC,PRIM,STATUS)
      ELSE
*  otherwise provided not primitive
        CALL USI_ASSOCI('INP','READ',ILOC,PRIM,STATUS)
        IF (.NOT.PRIM) THEN
*  make copy of input - then release it
          CALL DAT_TYPE(ILOC,TYPE,STATUS)
          CALL USI_ASSOCO('OUT',TYPE,OLOC,STATUS)
          CALL HDX_COPY(ILOC,OLOC,STATUS)
          CALL BDA_RELEASE(ILOC,STATUS)
        ELSE
          CALL MSG_PRNT('! input is primitive')
          STATUS=SAI__ERROR
        ENDIF
      ENDIF

      CALL BDA_CHKDATA(OLOC,DOK,NDIM,DIMS,STATUS)
      CALL BDA_CHKQUAL(OLOC,QOK,NDIM,DIMS,STATUS)

      LEN=1
      DO IDIM=1,NDIM
        LEN=LEN*DIMS(IDIM)
      ENDDO

      IF (STATUS.EQ.SAI__OK.AND.DOK.AND.QOK) THEN

        CALL PAR_GET0R('MAGIC',MAGVAL,STATUS)
        IF (STATUS.EQ.PAR__NULL) THEN
          CALL ERR_ANNUL( STATUS )
          MAGVAL=VAL__BADR
        ENDIF

        CALL BDA_GETMASK(OLOC,MASK,STATUS)
        CALL STR_BTOC(MASK,MSTR,STATUS)
        CALL PAR_DEF0C('MASK',MSTR,STATUS)
        CALL PAR_GET0C('MASK',MSTR,STATUS)
        CALL STR_CTOB(MSTR,MASK,STATUS)

        CALL BDA_MAPDATA(OLOC,'U',DPTR,STATUS)
        CALL BDA_MAPQUAL(OLOC,'R',QPTR,STATUS)

        CALL MAGIC_WRITE(OLOC,MASK,MAGVAL,LEN,%VAL(QPTR),%VAL(DPTR),
     :                                                       STATUS)


      ENDIF

      CALL AST_CLOSE()

      END



*+  MAGIC_WRITE
      SUBROUTINE MAGIC_WRITE(LOC,MASK,MAGIC,LEN,Q,D,STATUS)
*    Description :
*    Authors :
*      BHVAD::RJV
*    History :
*    Type definitions :
      IMPLICIT NONE
*    Global constants :
      INCLUDE 'SAE_PAR'
      INCLUDE 'DAT_PAR'
*    Import :
      CHARACTER*(DAT__SZLOC) LOC
      BYTE MASK
      REAL MAGIC
      INTEGER LEN
      BYTE Q(LEN)
      REAL D(LEN)
*    Export :
*    Status :
      INTEGER STATUS
*    Function declarations :
*    Local constants :
      BYTE GOOD
      PARAMETER (GOOD=0)
*    Local variables :
      LOGICAL FLAG
      INTEGER I
*-
      IF (STATUS.EQ.SAI__OK) THEN

        FLAG=.FALSE.

        DO I=1,LEN
          IF ((Q(I).AND.MASK).NE.GOOD) THEN
            D(I)=MAGIC
            FLAG=.TRUE.
          ENDIF
        ENDDO

        IF (FLAG) THEN
          CALL BDA_PUTMAGIC(LOC,FLAG,STATUS)
        ENDIF

        IF (STATUS.NE.SAI__OK) THEN
          CALL ERR_REP(' ','! from MAGIC_WRITE',STATUS)
        ENDIF
      ENDIF
      END
