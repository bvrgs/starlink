      SUBROUTINE PARSECON_PTRINIT ( STATUS )
*+
*  Name:
*     PARSECON_PTRINIT

*  Purpose:
*     Initialise pointers etc. before parsing IFL.

*  Language:
*     VAX Fortran

*  Invocation:
*     CALL PARSECON_PTRINIT ( STATUS )

*  Description:
*     Initialises pointers to the various storage lists
*     and various names before the parsing of an interface file begins.

*  Arguments:
*     STATUS=INTEGER

*  Algorithm:
*     Set the various pointers to zero.

*  Implementation Deficiencies:
*     Strictly, all the pointer arrays should be zeroed - eg PARDEF,
*     PARDYN, PARASSOC, NEEDOB, NEEDCAN.....

*  Authors:
*     B.D.Kelly (REVAD::BDK)
*     A J Chipperfield (STARLINK)
*     {enter_new_authors_here}

*  History:
*     02.10.1984:  Original (REVAD::BDK)
*     27.02.1985:  initialize PROGNAME and EXEPATH (REVAD::BDK)
*     23.08.1985:  initialize MONOLITH (REVAD::BDK)
*     16.08.1990:  initialize ACNAME and PRNAME for error reports (RLVAD::AJC)
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


*  Status:
      INTEGER STATUS


*  Global Variables:
      INCLUDE 'SUBPAR_CMN'
      INCLUDE 'PARSECON3_CMN'

*.


      IF ( STATUS .NE. SAI__OK ) RETURN

      PARPTR = 0
      ACTPTR = 0
      NEEDPTR = 0
      INTPTR = 0
      REALPTR = 0
      DOUBLEPTR = 0
      CHARPTR = 0
      LOGPTR = 0
      PROGNAME = ' '
      EXEPATH = ' '
      MONOLITH = .FALSE.

*   Names in error reporting common block
      ACNAME = ' '
      PRNAME = ' '

      END
