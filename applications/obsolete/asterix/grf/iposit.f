*+  IPOSIT - set current position
      SUBROUTINE IPOSIT(STATUS)
*    Description :
*    Method :
*    Deficiencies :
*    Bugs :
*    Authors :
*    History :
*     22 Oct 90: V1.2-1 positions in various frames (RJV)
*      1 Jul 93: V1.2-2 GTR used (RJV)
*      7 Apr 95: V1.8-0 list entry and selection (RJV)
*     20 NOV 95: V2.0-0 GUI version (RJV)
*    Type Definitions :
      IMPLICIT NONE
*    Global constants :
      INCLUDE 'SAE_PAR'
      INCLUDE 'PAR_ERR'
*    Import :
*    Import-Export :
*    Export :
*    Status :
      INTEGER STATUS
*    Functions :
*    Local Constants :
*    Local variables :
      CHARACTER*12 OPTION
      INTEGER OID,NB
      LOGICAL ENTER,SEL,SHOW
*    Global Variables :
      INCLUDE 'IMG_CMN'
*    Version :
      CHARACTER*30 VERSION
      PARAMETER (VERSION='IPOSIT Version 2.0-0')
*-
      CALL USI_INIT()

      CALL MSG_PRNT(VERSION)

      IF (.NOT.I_OPEN) THEN
        CALL MSG_PRNT('AST_ERR: image processing system not active')
      ELSEIF (.NOT.I_DISP) THEN
        CALL MSG_PRNT('AST_ERR: no image currently displayed')
      ELSE

*  being run from a GUI - find out which mode
        IF (I_GUI) THEN
          CALL NBS_FIND_ITEM(I_NBID,'OPTIONS',OID,STATUS)
          CALL NBS_GET_CVALUE(OID,0,OPTION,NB,STATUS)
          IF (OPTION(1:1).EQ.'T') THEN
            CALL IPOSIT_GUI(STATUS)
          ENDIF
        ELSE
          OPTION=' '
        ENDIF

        IF (OPTION(1:1).NE.'T') THEN

*  see if dealing with multiple positions
          CALL USI_GET0L('ENTER',ENTER,STATUS)
          IF (ENTER) THEN

            CALL IPOSIT_ENTER(STATUS)

          ELSEIF (I_NPOS.GT.0) THEN
            CALL USI_GET0L('SHOW',SHOW,STATUS)
            IF (SHOW) THEN

              CALL IPOSIT_SHOW(STATUS)

            ELSE
              CALL USI_GET0L('SEL',SEL,STATUS)
              IF (SEL) THEN

                CALL IPOSIT_SELECT(STATUS)

              ENDIF
            ENDIF
          ENDIF

*  single position mode
          IF (.NOT.(ENTER.OR.SHOW.OR.SEL)) THEN

            CALL IPOSIT_SINGLE(STATUS)

          ENDIF

        ENDIF


      ENDIF

      CALL USI_CLOSE()

      END


*+  IPOSIT_SINGLE - set current position to given single position
      SUBROUTINE IPOSIT_SINGLE(STATUS)
*    Description :
*    Method :
*    Deficiencies :
*    Bugs :
*    Type Definitions :
      IMPLICIT NONE
*    Global constants :
      INCLUDE 'SAE_PAR'
      INCLUDE 'PAR_ERR'
*    Import :
*    Import-Export :
*    Export :
*    Status :
      INTEGER STATUS
*    Functions :
*    Local Constants :
*    Local variables :
      CHARACTER*1 CH
      CHARACTER*20 SRA,SDEC
      DOUBLE PRECISION RA,DEC,ELON,ELAT,B,L
      REAL X,Y,XPIX,YPIX
      INTEGER FRAME
*    Global Variables :
      INCLUDE 'IMG_CMN'
*-
      IF (STATUS.EQ.SAI__OK) THEN

*  ensure transformations correct
        CALL GTR_RESTORE(STATUS)

*  cursor mode
        IF (I_MODE.EQ.1) THEN
          CALL MSG_PRNT(' ')
          CALL MSG_PRNT('Select position...')

          CALL PGCURSE(X,Y,CH)

*  keyboard mode
        ELSE

*  get coordinate frame
          CALL USI_GET0I('FRAME',FRAME,STATUS)

          IF (FRAME.EQ.1) THEN
            CALL USI_GET0C('RA',SRA,STATUS)
            CALL USI_GET0C('DEC',SDEC,STATUS)
            CALL CONV_RADEC(SRA,SDEC,RA,DEC,STATUS)
            CALL IMG_CELTOWORLD(RA,DEC,X,Y,STATUS)

          ELSEIF (FRAME.EQ.2) THEN
            CALL USI_GET0D('ELON',ELON,STATUS)
            CALL USI_GET0D('ELAT',ELAT,STATUS)
            CALL IMG_ECLTOWORLD(ELON,ELAT,X,Y,STATUS)

          ELSEIF (FRAME.EQ.3) THEN
            CALL USI_GET0D('L',L,STATUS)
            CALL USI_GET0D('B',B,STATUS)
            CALL IMG_GALTOWORLD(L,B,X,Y,STATUS)

          ELSEIF (FRAME.EQ.4) THEN
            CALL USI_GET0R('X',X,STATUS)
            CALL USI_GET0R('Y',Y,STATUS)

          ELSEIF (FRAME.EQ.5) THEN
            CALL USI_GET0R('XPIX',XPIX,STATUS)
            CALL USI_GET0R('YPIX',YPIX,STATUS)
            CALL IMG_PIXTOWORLD(XPIX,YPIX,X,Y,STATUS)

          ENDIF

        ENDIF

        CALL IMG_SETPOS(X,Y,STATUS)

      ENDIF


      END



*+  IPOSIT_ENTER - enter a list of positions
      SUBROUTINE IPOSIT_ENTER(STATUS)
*    Description :
*    Method :
*    Deficiencies :
*    Bugs :
*    Type Definitions :
      IMPLICIT NONE
*    Global constants :
      INCLUDE 'SAE_PAR'
      INCLUDE 'PAR_ERR'
      INCLUDE 'FIO_ERR'
*    Import :
*    Import-Export :
*    Export :
*    Status :
      INTEGER STATUS
*    Functions :
      INTEGER CHR_LEN
*    Local Constants :
*    Local variables :
      CHARACTER*132 TEXT
      CHARACTER*80 REC
      CHARACTER*20 RAS,DECS
      CHARACTER*1 LC
      DOUBLE PRECISION RA,DEC
      REAL X,Y
      INTEGER IFD
      INTEGER RAPTR,DECPTR
      INTEGER ISRC,NSRC
      INTEGER L,SFID
      LOGICAL EXIST
      LOGICAL POK
      LOGICAL RADEC
      LOGICAL APPEND
      LOGICAL REPEAT
*    Global Variables :
      INCLUDE 'IMG_CMN'
*-
      IF (STATUS.EQ.SAI__OK) THEN



*  get group id for storing positions
        IF (I_NPOS.EQ.0) THEN
          CALL GRP_NEW('Source list',I_POS_ID,STATUS)
*  or append to or reset  existing one
        ELSE
          CALL USI_GET0L('APPEND',APPEND,STATUS)
          IF (.NOT.APPEND) THEN
            CALL GRP_SETSZ(I_POS_ID,0,STATUS)
            I_NPOS=0
          ENDIF
        ENDIF

*  get input
        CALL USI_GET0C('LIST',TEXT,STATUS)

        IF (STATUS.EQ.SAI__OK) THEN

*  see if direct RA/DEC entry
          CALL IPOSIT_PARSE(TEXT,RADEC,STATUS)

          IF (RADEC) THEN

            REPEAT=.TRUE.
            DO WHILE (REPEAT.AND.STATUS.EQ.SAI__OK)

*  check for continuation character
              L=CHR_LEN(TEXT)
              LC=TEXT(L:L)
              IF (LC.EQ.'~'.OR.LC.EQ.'-'.OR.LC.EQ.'\') THEN
                REPEAT=.TRUE.
                L=CHR_LEN(TEXT(:L-1))
              ELSE
                REPEAT=.FALSE.
              ENDIF

*  split text into ra and dec
              CALL CONV_SPLIT(TEXT(:L),RAS,DECS,STATUS)
*  parse
              CALL CONV_RADEC(RAS,DECS,RA,DEC,STATUS)
*  then convert back to a uniform format
              CALL CONV_DEGHMS(REAL(RA),RAS)
              CALL CONV_DEGDMS(REAL(DEC),DECS)
*  and store
              CALL GRP_PUT(I_POS_ID,1,RAS(:11)//DECS(:11),0,STATUS)
              I_NPOS=I_NPOS+1

*  if first one then make current position
              IF (I_NPOS.EQ.1) THEN
                CALL IMG_CELTOWORLD(RA,DEC,X,Y,STATUS)
                CALL IMG_SETPOS(X,Y,STATUS)
              ENDIF

              IF (REPEAT) THEN
                CALL USI_CANCL('LIST',STATUS)
                CALL USI_GET0C('LIST',TEXT,STATUS)
              ENDIF

            ENDDO

*  otherwise should be a file
          ELSE

*  see if file exists in form given
            INQUIRE(FILE=TEXT,EXIST=EXIST)
*  if it does - assume it to be text file
            IF (EXIST) THEN
              CALL FIO_OPEN(TEXT,'READ','NONE',0,IFD,STATUS)
              DO WHILE ( STATUS .EQ. SAI__OK )
                CALL FIO_READF(IFD,REC,STATUS)
                IF (STATUS.EQ.SAI__OK) THEN
*  ignore blank lines
                  IF (REC.GT.' ') THEN
*  remove leading blanks
                    CALL CHR_LDBLK( REC )

*  split record into ra and dec
                    CALL CONV_SPLIT(REC,RAS,DECS,STATUS)
*  parse
                    CALL CONV_RADEC(RAS,DECS,RA,DEC,STATUS)
*  then convert back to a uniform format
                    CALL CONV_DEGHMS(REAL(RA),RAS)
                    CALL CONV_DEGDMS(REAL(DEC),DECS)
*  and store
                    CALL GRP_PUT(I_POS_ID,1,RAS(:11)//DECS(:11),0,
     :                                                      STATUS)
                    I_NPOS=I_NPOS+1

*  make the first one the current position
                    IF (I_NPOS.EQ.1) THEN
                      CALL IMG_CELTOWORLD(RA,DEC,X,Y,STATUS)
                      CALL IMG_SETPOS(X,Y,STATUS)
                    ENDIF

                  ENDIF
                ENDIF
              ENDDO
              IF ( STATUS .EQ. FIO__EOF ) CALL ERR_ANNUL( STATUS )
              CALL FIO_CLOSE(IFD,STATUS)

*  otherwise assume HDS file in PSS format
            ELSE

              CALL ADI_FOPEN( TEXT, '*', 'READ', SFID, STATUS )
              IF (STATUS.EQ.SAI__OK) THEN

*  check if sources
                CALL SSO_INIT( STATUS )
                CALL SSI_VALID( SFID, POK, STATUS )
                CALL SSI_GETNSRC( SFID, NSRC, STATUS )
                IF (.NOT. POK .OR.NSRC.EQ.0 ) THEN
                  CALL MSG_PRNT('AST_ERR: No sources in this SSDS')
                ELSE
*  get RA DEC of sources
                  CALL SSI_MAPFLD( SFID, 'RA', '_DOUBLE', 'READ',
     :                                            RAPTR, STATUS )
                  CALL SSI_MAPFLD( SFID, 'DEC', '_DOUBLE', 'READ',
     :                                           DECPTR, STATUS )
                  DO ISRC=1,NSRC
*  get each position
                    CALL ARR_ELEM1D(RAPTR,NSRC,ISRC,RA,STATUS)
                    CALL ARR_ELEM1D(DECPTR,NSRC,ISRC,DEC,STATUS)

*  convert to a uniform format
                    CALL CONV_DEGHMS(REAL(RA),RAS)
                    CALL CONV_DEGDMS(REAL(DEC),DECS)

*  and store
                    CALL GRP_PUT(I_POS_ID,1,RAS(:11)//DECS(:11),0,
     :                                                      STATUS)
                    I_NPOS=I_NPOS+1

*  make the first one the current position
                    IF (I_NPOS.EQ.1) THEN
                      CALL IMG_CELTOWORLD(RA,DEC,X,Y,STATUS)
                      CALL IMG_SETPOS(X,Y,STATUS)
                    ENDIF

                  ENDDO

                ENDIF

                CALL SSI_RELEASE(SFID,STATUS)
                CALL ADI_FCLOSE(SFID,STATUS)

              ENDIF

            ENDIF

          ENDIF

        ENDIF

      ENDIF

      END




*+  IPOSIT_SELECT - select current position from list
      SUBROUTINE IPOSIT_SELECT(STATUS)
*    Description :
*    Method :
*    Deficiencies :
*    Bugs :
*    Type Definitions :
      IMPLICIT NONE
*    Global constants :
      INCLUDE 'SAE_PAR'
      INCLUDE 'PAR_ERR'
*    Import :
*    Import-Export :
*    Export :
*    Status :
      INTEGER STATUS
*    Functions :
*    Local Constants :
*    Local variables :
      CHARACTER*80 REC
      CHARACTER*20 RAS,DECS
      DOUBLE PRECISION RA,DEC
      REAL X,Y
      INTEGER NUM
*    Global Variables :
      INCLUDE 'IMG_CMN'
*-
      IF (STATUS.EQ.SAI__OK) THEN

*  get number
        CALL USI_GET0I('NUM',NUM,STATUS)
        IF (NUM.LE.0.OR.NUM.GT.I_NPOS) THEN
          CALL MSG_PRNT('AST_ERR: invalid position number')

        ELSE

*  get entry from list
          CALL GRP_GET(I_POS_ID,NUM,1,REC,STATUS)
*  split record into ra and dec
          CALL CONV_SPLIT(REC,RAS,DECS,STATUS)
*  parse
          CALL CONV_RADEC(RAS,DECS,RA,DEC,STATUS)
*  and store as current position
          CALL IMG_CELTOWORLD(RA,DEC,X,Y,STATUS)
          CALL IMG_SETPOS(X,Y,STATUS)

        ENDIF

      ENDIF

      END



*+  IPOSIT_SHOW - list positions
      SUBROUTINE IPOSIT_SHOW(STATUS)
*    Description :
*    Method :
*    Deficiencies :
*    Bugs :
*    Type Definitions :
      IMPLICIT NONE
*    Global constants :
      INCLUDE 'SAE_PAR'
*    Import :
*    Import-Export :
*    Export :
*    Status :
      INTEGER STATUS
*    Functions :
*    Local Constants :
*    Local variables :
      CHARACTER*40 REC/' '/
      INTEGER IPOS
*    Global Variables :
      INCLUDE 'IMG_CMN'
*-
      IF (STATUS.EQ.SAI__OK) THEN

        CALL MSG_BLNK()
        DO IPOS=1,I_NPOS
          CALL GRP_GET(I_POS_ID,IPOS,1,REC(6:),STATUS)
          WRITE(REC(:3),'(I3)') IPOS
          CALL MSG_PRNT(REC)
        ENDDO
        CALL MSG_BLNK()

      ENDIF

      END





*+  IPOSIT_PARSE - decide whether string is likely candidate for RA/DEC
      SUBROUTINE IPOSIT_PARSE(TEXT,RADEC,STATUS)
*    Description :
*    Method :
*    Deficiencies :
*    Bugs :
*    Type Definitions :
      IMPLICIT NONE
*    Global constants :
      INCLUDE 'SAE_PAR'
*    Import :
      CHARACTER*(*) TEXT
*    Import-Export :
*    Export :
      LOGICAL RADEC
*    Status :
      INTEGER STATUS
*    Functions :
      INTEGER CHR_LEN
*    Local Constants :
*    Local variables :
      CHARACTER*1 C
      INTEGER L
      INTEGER NDIGIT,NDELIM,NDOT,NMINUS,NPLUS
      INTEGER I
*-
      IF (STATUS.EQ.SAI__OK) THEN

        L=CHR_LEN(TEXT)
        C=TEXT(L:L)
*  ignore continuation character
        IF (C.EQ.'~'.OR.C.EQ.'-'.OR.C.EQ.'\') THEN
          L=L-1
        ENDIF

        NDIGIT=0
        NDELIM=0
        NDOT=0
        NMINUS=0
        NPLUS=0
        DO I=1,L
          C=TEXT(I:I)
          IF (C.GE.'0'.AND.C.LE.'9') THEN
            NDIGIT=NDIGIT+1
          ELSEIF (C.EQ.'.') THEN
            NDOT=NDOT+1
          ELSEIF (C.EQ.'-') THEN
            NMINUS=NMINUS+1
          ELSEIF (C.EQ.'+') THEN
            NPLUS=NPLUS+1
          ELSE
            IF (C.EQ.' '.OR.C.EQ.':'.OR.C.EQ.'h'.OR.C.EQ.'d'.OR.
     :                                  C.EQ.'m'.OR.C.EQ.'s') THEN
              NDELIM=NDELIM+1
            ENDIF
          ENDIF
        ENDDO

        RADEC=(NDIGIT.GE.4.AND.NDOT.LE.2.AND.NMINUS.LE.1.AND.
     :                       (NDIGIT+NDOT+NMINUS+NDELIM).EQ.L)

      ENDIF

      END




*+  IPOSIT_GUI
      SUBROUTINE IPOSIT_GUI(STATUS)
*    Description :
*    Method :
*    Deficiencies :
*    Bugs :
*    Authors :
*    History :
*    Type Definitions :
      IMPLICIT NONE
*    Global constants :
      INCLUDE 'SAE_PAR'
      INCLUDE 'DAT_PAR'
      INCLUDE 'PRM_PAR'
*    Import :
*    Import-Export :
*    Export :
*    Status :
      INTEGER STATUS
*    Functions :
*    Local Constants :
*    Local variables :
      REAL XP,YP
      CHARACTER*8 NAME
      CHARACTER*8 STRING
      CHARACTER*10 FMT
      CHARACTER*12 OPTIONS
      CHARACTER*12 RAS,DECS
      BYTE Q
      INTEGER IXP,IYP,IXPMAX,IYPMAX
      INTEGER IX,IY
      INTEGER I,J,II,JJ
      INTEGER I1,I2,J1,J2
      INTEGER ISTAT
      INTEGER XPID,YPID,XPMID,YPMID,FID,OID,XID,YID
      INTEGER RAID,DECID
      INTEGER FLAG
      INTEGER ISCALE
      INTEGER NB
      REAL XP1,XP2,YP1,YP2
      REAL XW,YW
      REAL XWOLD,YWOLD
      REAL XW1,XW2,YW1,YW2
      REAL SCVAL,VAL,VAL2
      REAL XC,YC,DX,DY
      REAL XSCALE,YSCALE
      DOUBLE PRECISION RA,DEC,ELON,ELAT,GLON,GLAT
      LOGICAL VAR,ERR,SIGNIF,QUAL
*    Global Variables :
      INCLUDE 'IMG_CMN'
*-

      IF (STATUS.EQ.SAI__OK) THEN

*  store current position
        XWOLD=I_X
        YWOLD=I_Y

*  locate noticeboard items
        CALL NBS_FIND_ITEM(I_NBID,'X',XID,STATUS)
        CALL NBS_FIND_ITEM(I_NBID,'Y',YID,STATUS)
        CALL NBS_FIND_ITEM(I_NBID,'XP',XPID,STATUS)
        CALL NBS_FIND_ITEM(I_NBID,'YP',YPID,STATUS)
        CALL NBS_FIND_ITEM(I_NBID,'XPMAX',XPMID,STATUS)
        CALL NBS_FIND_ITEM(I_NBID,'YPMAX',YPMID,STATUS)
        CALL NBS_FIND_ITEM(I_NBID,'RA',RAID,STATUS)
        CALL NBS_FIND_ITEM(I_NBID,'DEC',DECID,STATUS)
        NAME='DATA'
        CALL NBS_FIND_ITEM(I_NBID,'FLAG',FID,STATUS)
        CALL NBS_FIND_ITEM(I_NBID,'OPTIONS',OID,STATUS)

*  get device size
        CALL NBS_GET_VALUE(XPMID,0,VAL__NBI,IXPMAX,NB,STATUS)
        CALL NBS_GET_VALUE(YPMID,0,VAL__NBI,IYPMAX,NB,STATUS)

*  get plot window parameters
        CALL PGQVP(3,XP1,XP2,YP1,YP2)
        CALL PGQWIN(XW1,XW2,YW1,YW2)

        XSCALE=(XW2-XW1)/(XP2-XP1)
        YSCALE=(YW2-YW1)/(YP2-YP1)

        FLAG=0
        DO WHILE (FLAG.EQ.0)

*  get current cursor position in device coords
          CALL NBS_GET_VALUE(XPID,0,VAL__NBI,IXP,NB,STATUS)
          CALL NBS_GET_VALUE(YPID,0,VAL__NBI,IYP,NB,STATUS)
          IYP=IYPMAX-IYP

*  convert to world coords
          XW=XW1+(REAL(IXP)-XP1)*XSCALE
          YW=YW1+(REAL(IYP)-YP1)*YSCALE

*  convert to other frames
          CALL IMG_WORLDTOPIX(XW,YW,XP,YP,STATUS)
          CALL IMG_WORLDTOCEL(XW,YW,RA,DEC,STATUS)
          CALL CONV_DEGHMS(REAL(RA),RAS)
          CALL CONV_DEGDMS(REAL(DEC),DECS)
          CALL IMG_WORLDTOECL(XW,YW,ELON,ELAT,STATUS)
          CALL IMG_WORLDTOGAL(XW,YW,GLON,GLAT,STATUS)


          CALL NBS_PUT_VALUE(XID,0,VAL__NBR,XW,STATUS)
          CALL NBS_PUT_VALUE(YID,0,VAL__NBR,YW,STATUS)
          CALL NBS_PUT_CVALUE(RAID,0,RAS,STATUS)
          CALL NBS_PUT_CVALUE(DECID,0,DECS,STATUS)
          CALL NBS_GET_VALUE(FID,0,VAL__NBI,FLAG,NB,STATUS)

        ENDDO

        IF (FLAG.NE.1) THEN
          XW=XWOLD
          YW=YWOLD
        ENDIF

        CALL IMG_SETPOS(XW,YW,STATUS)


      ENDIF

      END
