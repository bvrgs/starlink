      SUBROUTINE PARSECON_SETACRDS ( ENTRY, STATUS )
*+
*  Name:
*     PARSECON_SETACRDS

*  Purpose:
*     Sets-up action menu position.

*  Language:
*     VAX Fortran

*  Invocation:
*     CALL PARSECON_SETACRDS ( ENTRY, STATUS )

*  Description:
*     Loads the coordinate for the most recently declared program
*     action into the ACTCOORDS store at the position indicated.

*  Arguments:
*     ENTRY=CHARACTER*(*) (given)
*        Numeric character string, indicating the position in the
*        menu for the action
*     STATUS=INTEGER

*  Algorithm:
*     The given string is converted to an integer which defines a
*     screen coordinate. If the X-coordinate hasn't been set yet, assume
*     the X-coordinate is being given. Otherwise, assume it is the
*     Y-coordinate.

*  Authors:
*     B.D.Kelly (REVAD::BDK)
*     A J Chipperfield (STARLINK)
*     {enter_new_authors_here}

*  History:
*     13.05.1986:  Original (REVAD::BDK)
*     16.10.1990:  Use CHR for conversion to integer
*        it's portable and stricter (RLVAD::AJC)
*     20.01.1992:  Renamed from _SETACOORDS (RLVAD::AJC)
*     25.02.1991:  Report errors (RLVAD::AJC)
*     24.03.1993:  Add DAT_PAR for SUBPAR_CMN
*     {enter_further_changes_here}

*  Bugs:
*     {note_any_bugs_here}

*-

*  Type Definitions:
      IMPLICIT NONE


*  Global Constants:
      INCLUDE 'SAE_PAR'
      INCLUDE 'DAT_PAR'
      INCLUDE 'PARSECON_ERR'


*  Arguments Given:
      CHARACTER*(*) ENTRY             ! the numeric string


*  Status:
      INTEGER STATUS


*  Global Variables:
      INCLUDE 'SUBPAR_CMN'


*  Local Variables:
      INTEGER NUMBER

*.


      IF ( STATUS .NE. SAI__OK ) RETURN

*   Convert the string to integer.
      CALL CHR_CTOI( ENTRY, NUMBER, STATUS )

      IF ( STATUS .EQ. SAI__ERROR ) THEN
         STATUS = PARSE__IVCOORD
         CALL EMS_REP ( 'PCN_SETACRDS1',
     :   'PARSECON: Action coordinates must be INTEGER',
     :    STATUS )

      ELSE
*      If Xcoord free, assume it is that. Otherwise, put it into Y.
         IF ( ACTCOORDS(1,ACTPTR) .EQ. -1 ) THEN
            ACTCOORDS(1,ACTPTR) = NUMBER
         ELSE
            ACTCOORDS(2,ACTPTR) = NUMBER
         ENDIF

      ENDIF

      END
