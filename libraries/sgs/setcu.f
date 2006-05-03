      SUBROUTINE sgs_SETCU (X,Y)
*+
*  Name:
*     SETCU

*  Purpose:
*     Set cursor position.

*  Language:
*     Starlink Fortran 77

*  Arguments:
*     X = REAL (Given)
*         X cursor position in world coordinates
*     Y = REAL (Given)
*         Y   "        "     "   "        "

*  Authors:
*     PTW: P. T. Wallace (Starlink)
*     DLT: D. L. Terrett (Starlink)
*     {enter_new_authors_here}

*  History:
*     07-SEP-1991 (PTW/DLT):
*        Modified.
*     {enter_further_changes_here}

*  Bugs:
*     {note_any_bugs_here}

*  Externals:
*     sgs_ICUAV, sgs_IZONE, sgs_ICURW, sgs_1ERR, GQLCS, GINLC

*-

      IMPLICIT NONE

      REAL X,Y

      INCLUDE 'GKS_PAR'

      INCLUDE 'SGS_ERR'


      REAL X1,X2,Y1,Y2,XM,YM
      INTEGER IERR,MODE,IESW,ITNR,IPET,LDR,JSTAT,ISWKID
      REAL EAREA(4),RILPX,RILPY
      CHARACTER DATREC(1)*80,RNAME*5
      LOGICAL AVAIL
      PARAMETER (RNAME='SETCU')



*  Check that cursor exists
      CALL sgs_ICUAV(AVAIL)
      IF (AVAIL) THEN
         CALL sgs_IZONE(X1,X2,Y1,Y2,XM,YM)

*     Move cursor if position is inside the workstation window
         CALL sgs_ICURW(ISWKID)
         IF (X.GE.X1 .AND. X.LE.X2 .AND.
     :       Y.GE.Y1 .AND. Y.LE.Y2) THEN

*        Inquire current locator state
            CALL GQLCS(ISWKID,1,GSET,1,IERR,MODE,IESW,ITNR,
     :                                RILPX,RILPY,IPET,EAREA,LDR,DATREC)
            IF (IERR.NE.0) THEN
               CALL sgs_1ERR(SGS__INQER,RNAME,
     :                                  'Error returned by GQLCS',JSTAT)
               GO TO 999
            END IF

*        Set new position
            CALL GINLC(ISWKID,1,1,X,Y,IPET,
     :                    EAREA(1),EAREA(2),EAREA(3),EAREA(4),
     :                                                       LDR,DATREC)

         END IF
      END IF
  999 CONTINUE

      END
