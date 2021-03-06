      SUBROUTINE ARY1_TEMP( TYPE, NDIM, DIM, LOC, STATUS )
*+
*  Name:
*     ARY1_TEMP

*  Purpose:
*     Create a temporary HDS object.

*  Language:
*     Starlink Fortran 77

*  Invocation:
*     CALL ARY1_TEMP( TYPE, NDIM, DIM, LOC, STATUS )

*  Description:
*     The routine creates a temporary HDS object with the specified
*     type and shape. On the first invocation a temporary structure is
*     created to contain such objects. Subsequently, temporary objects
*     are created within this enclosing structure.

*  Arguments:
*     TYPE = CHARACTER * ( * ) (Given)
*        HDS type of object to be created.
*     NDIM = INTEGER (Given)
*        Number of object dimensions.
*     DIM( * ) = INTEGER (Given)
*        Object dimensions.
*     LOC = CHARACTER * ( * ) (Returned)
*        Locator to temporary object.
*     STATUS = INTEGER (Given and Returned)
*        The global status.

*  Notes:
*     -  This routine is a work-around to avoid the problems associated
*     with calling DAT_TEMP if the objects created must subsequently be
*     erased.

*  Algorithm:
*     -  Set an initial value for the LOC argument before checking the
*     inherited status.
*     -  On the first invocation, create a temporary enclosing
*     structure and tune HDS to expect a large number of components in
*     it.
*     -  Subsequently, create a unique name for the temporary object
*     required.
*     -  Create the object within the enclosing structure.

*  Copyright:
*     Copyright (C) 1989, 1990 Science & Engineering Research Council.
*     All Rights Reserved.

*  Licence:
*     This program is free software; you can redistribute it and/or
*     modify it under the terms of the GNU General Public License as
*     published by the Free Software Foundation; either version 2 of
*     the License, or (at your option) any later version.
*
*     This program is distributed in the hope that it will be
*     useful,but WITHOUT ANY WARRANTY; without even the implied
*     warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
*     PURPOSE. See the GNU General Public License for more details.
*
*     You should have received a copy of the GNU General Public License
*     along with this program; if not, write to the Free Software
*     Foundation, Inc., 51 Franklin Street,Fifth Floor, Boston, MA
*     02110-1301, USA

*  Authors:
*     RFWS: R.F. Warren-Smith (STARLINK)
*     {enter_new_authors_here}

*  History:
*     7-JUN-1989  (RFWS):
*        Original version.
*     16-AUG-1989 (RFWS):
*        Changed initialisation of locators to use global constant.
*     2-MAR-1990 (RFWS):
*        Changed to perform initialisation of the LOC argument before
*        checking the inherited status.
*     22-MAR-1990 (RFWS):
*        Call HDS_TUNE to anticipate a large number of structure
*        components.
*     {enter_further_changes_here}

*  Bugs:
*     {note_any_bugs_here}

*-

*  Type definitions:
      IMPLICIT NONE              ! No implicit typing

*  Global Constants:
      INCLUDE 'SAE_PAR'          ! Standard SAE constants
      INCLUDE 'DAT_PAR'          ! DAT_ public constants
      INCLUDE 'ARY_CONST'        ! ARY_ private constants

*  Arguments Given:
      CHARACTER * ( * ) TYPE
      INTEGER NDIM
      INTEGER DIM( * )

*  Arguments Returned:
      CHARACTER * ( * ) LOC

*  Status:
      INTEGER STATUS             ! Global status

*  Local variables:
      CHARACTER * ( DAT__SZLOC ) TMPLOC ! Locator to enclosing structure
      CHARACTER * ( DAT__SZNAM ) NAME ! Temporary object name
      INTEGER COUNT              ! Count of objects created
      INTEGER DUMMY( 1 )         ! Dummy dimensions array
      INTEGER NCHAR              ! Number of characters formatted
      SAVE COUNT
      SAVE TMPLOC

*  Local Data:
      DATA COUNT / 0 /

*.

*  Set an initial value for the LOC argument.
      LOC = ARY__NOLOC

*  Check inherited global status.
      IF ( STATUS .NE. SAI__OK ) RETURN

*  Increment the count of temporary objects created.
      COUNT = COUNT + 1

*  Before creating the first object, create a temporary enclosing
*  structure and tune HDS to expect a large number of components in it.
      IF ( COUNT .EQ. 1 ) THEN
         TMPLOC = ARY__NOLOC
         CALL DAT_TEMP( 'ARY_TEMP', 0, DUMMY, TMPLOC, STATUS )
         CALL HDS_TUNE( 'NCOMP', 20, STATUS )

*  Fortran code is not thread safe, so if it is to be used in
*  multi-threaded code (e.g. from C, using a C wrapper ), entry to
*  Fortran subroutines must be serialised using a mutex (usually implemented 
*  in the C wrapper). It follows that we will never be trying to execute this 
*  code simultaneously from multiple threads. So we can  switch off thread-lock
*  checking for the root temporary object. Without this, we would need to find
*  some way to lock and unlock the root object each time is was used from a 
*  different thread (via a C wrapper). 
         CALL DAT_NOLOCK( TMPLOC, STATUS )
      END IF

*  Form a unique name for the temporary object.
      IF ( STATUS .EQ. SAI__OK ) THEN
         NAME = 'ARY_'
         CALL CHR_ITOC( COUNT, NAME( 5 : ), NCHAR )

*  Create an object inside the enclosing structure and obtain a locator
*  to it.
         CALL DAT_NEW( TMPLOC, NAME, TYPE, NDIM, DIM, STATUS )
         CALL DAT_FIND( TMPLOC, NAME, LOC, STATUS )
      END IF

*  Call error tracing routine and exit.
      IF ( STATUS .NE. SAI__OK ) CALL ARY1_TRACE( 'ARY1_TEMP', STATUS )

      END
