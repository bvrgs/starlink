      SUBROUTINE sgs_CLRZ
*+
*  Name:
*     CLRZ

*  Purpose:
*     Clear current zone, even if this means clearing the whole screen.

*  Language:
*     Starlink Fortran 77

*  Authors:
*     PTW: P. T. Wallace (Starlink)
*     DLT: D. L. Terrett (Starlink)
*     {enter_new_authors_here}

*  History:
*     14-SEP-1991 (PTW/DLT):
*        Modified.
*     {enter_further_changes_here}

*  Bugs:
*     {note_any_bugs_here}

*  Constants From GKS_PAR:
*     GCONDI   i       clear workstation conditionally
*     GPOSTP   i       regeneration postponed
*     GALWAY   i       clear workstation always

*  Externals:
*     sgs_CLRBL, sgs_OPOLY, sgs_OTEXT, GCLRWK, GUWK

*  Read From Common:
*     IZTW     i()     zone table - SGS workstation ID
*     IWTID    i()     workstation table - GKS workstation ID
*     ISZID    i       current zone ID
*     IBLKCL   i()     workstation descrition table - block clear mechanism
*     ZTW      r()     zone table - window
*     NTEXT    i       text counter
*     NPOLY    i       polyline counter
*     WSNRCL   l()     workstation not really clear

*  Written To Common:
*     WSNRCL   l()     workstation not really clear

*-

      IMPLICIT NONE

      INCLUDE 'sgscom'

      INCLUDE 'GKS_PAR'




*  Flush all pending plotting
      IF (NTEXT.GT.0) CALL sgs_OTEXT
      IF (NPOLY.GT.1) CALL sgs_OPOLY

*  If the current zone is a base zone clear the whole workstation
      IF (IZTW(ISZID).LT.0) THEN
         IF (WSNRCL(ABS(IZTW(ISZID)))) THEN
            CALL GCLRWK(IWTID(ABS(IZTW(ISZID))),GALWAY)
            WSNRCL(ABS(IZTW(ISZID))) = .FALSE.
         ELSE
            CALL GCLRWK(IWTID(ABS(IZTW(ISZID))),GCONDI)
         END IF
      ELSE

*     If the workstation supports block erase and it isn't spooled
         IF (IBLKCL(IZTW(ISZID)).GE.1 .AND.
     :       ISPOOL(IZTW(ISZID)).EQ.0) THEN

*        Clear zone with block erase
            CALL sgs_CLRBL(ZTW(1,ISZID),ZTW(2,ISZID),
     :                     ZTW(3,ISZID),ZTW(4,ISZID))

*        and update the workstation
            CALL GUWK(IWTID(IZTW(ISZID)),GPOSTP)
         ELSE

*        Have to clear the whole workstation
            IF (WSNRCL(ABS(IZTW(ISZID)))) THEN
               CALL GCLRWK(IWTID(ABS(IZTW(ISZID))),GALWAY)
               WSNRCL(ABS(IZTW(ISZID))) = .FALSE.
            ELSE
               CALL GCLRWK(IWTID(ABS(IZTW(ISZID))),GCONDI)
            END IF
         END IF
      END IF

      END
