      SUBROUTINE CONVERT( ACTION, STATUS )
*+
*  Name:
*     CONVERT

*  Purpose:
*     Top-level ADAM monolith routine for the CONVERT package.

*  Language:
*     Starlink Fortran 77

*  Invocation:
*     CALL CONVERT( ACTION, STATUS )

*  Description:
*     This routine interprets the action name passed to it and calls
*     the appropriate routine to perform the specified action. An error
*     will be reported and STATUS will be set if the action name is not
*     recognised.

*  Arguments:
*     ACTION = CHARACTER * ( * ) (Given and Returned)
*        The action name to be interpreted. The value given will be
*        forced to upper case by this routine.
*     STATUS = INTEGER (Given and Returned)
*        The global status.

*  Authors:
*     JM: Jo Murray (STARLINK)
*     MJC: Malcolm J. Currie (STARLINK)
*     {enter_new_authors_here}

*  History:
*     1991 Mar 11 (JM):
*        Original version.
*     1992 September 10 (MJC):
*        Minor tidying and comments added.
*     {enter_further_changes_here}

*  Bugs:
*     {note_any_bugs_here}

*-
      
*  Type Definitions:
      IMPLICIT NONE              ! No implicit typing

*  Global Constants:
      INCLUDE 'SAE_PAR'          ! Standard SAE constants

*  Arguments Given and Returned:
      CHARACTER * ( * ) ACTION

*  Status:
      INTEGER STATUS             ! Global status
*.

*  Check the inherited global status.
      IF ( STATUS .NE. SAI__OK ) RETURN

*  Convert the action name to upper case.
      CALL CHR_UCASE( ACTION )

*  Test the action name against each valid value in turn, calling the
*  appropriate routine.

*  Converts from an INTERIM BDF to an NDF.
      IF ( ACTION .EQ. 'BDF2NDF' ) THEN
         CALL BDF2NDF( STATUS )

*  Converts from a DIPSO file to an NDF.
      ELSE IF ( ACTION .EQ. 'DIPSO2NDF' ) THEN
         CALL DIPSO2NDF( STATUS )

*  Converts from a version 2 Figaro DST file to an NDF.
      ELSE IF ( ACTION .EQ. 'DST2NDF' ) THEN
         CALL DST2NDF( STATUS )

*  Converts from an NDF to an INTERIM BDF.
      ELSE IF ( ACTION .EQ. 'NDF2BDF' ) THEN
         CALL NDF2BDF( STATUS )

*  Converts from an NDF to a DIPSO file.
      ELSE IF ( ACTION .EQ. 'NDF2DIPSO' ) THEN
         CALL NDF2DIPSO ( STATUS )

*  Converts from an NDF to a FIgaro version 2 DST.
      ELSE IF ( ACTION .EQ. 'NDF2DST' ) THEN
         CALL NDF2DST( STATUS )

*  If the action name is not recognised, then report an error.
      ELSE
         STATUS = SAI__ERROR
         CALL MSG_SETC( 'ACTION', ACTION )
         CALL ERR_REP( 'CONVERT_ERR',
     :     'CONVERT: The action name ''^ACTION'' is not recognised by '/
     :     /'the CONVERT monolith.', STATUS )
      END IF

      END
