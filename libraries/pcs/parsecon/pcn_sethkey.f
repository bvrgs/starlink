      SUBROUTINE PARSECON_SETHKEY ( ENTRY, STATUS )
*+
*  Name:
*     PARSECON_SETHKEY

*  Purpose:
*     Sets-up parameter full-help specifier.

*  Language:
*     VAX Fortran

*  Invocation:
*     CALL PARSECON_SETHKEY( ENTRY, STATUS )

*  Description:
*     Loads the provided string into the helpkey store for the most
*     recently declared program parameter.

*  Arguments:
*     ENTRY=CHARACTER*(*) (given)
*        helpkey string
*     STATUS=INTEGER

*  Algorithm:
*     Superfluous quotes are removed from the given string, it is
*     concatenated with any declared 'helplib' specifier and the
*     result is put into the array holding helpkey.

*  Authors:
*     A. J. Chipperfield (RLVAD::AJC)
*     A J Chipperfield (STARLINK)
*     {enter_new_authors_here}

*  History:
*     15.05.1990:  Original (RLVAD::AJC)
*     17.10.1990:  define QUOTE portably (RLVAD::AJC)
*     24.03.1993:  Add DAT_PAR for SUBPAR_CMN (RLVAD::AJC)
*     16.02.1994:  Use used length of ACNAME  (RLVAD::AJC)
*     {enter_further_changes_here}

*  Bugs:
*     {note_any_bugs_here}

*-

*  Type Definitions:
      IMPLICIT NONE


*  Global Constants:
      INCLUDE 'SAE_PAR'
      INCLUDE 'DAT_PAR'


*  Arguments Given:
      CHARACTER*(*) ENTRY             ! the help string


*  Status:
      INTEGER STATUS

*    External routines:
      INTEGER CHR_LEN
      EXTERNAL CHR_LEN


*  Global Variables:
      INCLUDE 'SUBPAR_CMN'
      INCLUDE 'PARSECON2_CMN'
      INCLUDE 'PARSECON3_CMN'


*  Local Constants:
      CHARACTER*(*) QUOTE
      PARAMETER ( QUOTE = '''' )


*  Local Variables:
      CHARACTER*132 TEMP          ! Temporary string
      INTEGER ACNLEN              ! Used length of ACNAME

*.


      IF ( STATUS .NE. SAI__OK ) RETURN

*   If the help text is a quoted string, process the quotes
      IF ( ENTRY(1:1) .EQ. QUOTE ) THEN
         CALL STRING_STRIPQUOT ( ENTRY, TEMP, STATUS )

      ELSE
         TEMP = ENTRY

      ENDIF

*   Check if the default is required
      IF ( TEMP(1:1) .EQ. '*' ) THEN
         ACNLEN = MAX( CHR_LEN(ACNAME), 1 )
         TEMP = ACNAME(1:ACNLEN) // ' PARAMETERS ' // PRNAME
      ENDIF

*   Concatenate with HELPLIB (if set) and place in PARHKEY
      IF ( HLBLEN .NE. 0 ) THEN
         PARHKEY( PARPTR ) = HLBSTR(1:HLBLEN+1) // TEMP
      ELSE
         PARHKEY( PARPTR ) = TEMP
      ENDIF

      END
