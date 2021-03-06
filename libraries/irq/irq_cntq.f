      SUBROUTINE IRQ_CNTQ( LOCS, SIZE, SET, STATUS )
*+
*  Name:
*     IRQ_CNTQ

*  Purpose:
*     Count the number of pixels which are set in each bit-plane of the
*     QUALITY component.

*  Language:
*     Starlink Fortran 77

*  Invocation:
*     CALL IRQ_CNTQ( LOCS, SIZE, SET, STATUS )

*  Description:
*     Each bit plane of the NDF QUALITY component corresponds to a
*     different quality, described by a name stored in the quality
*     names information structure in an NDF extension.  A pixel is set
*     in a bit plane of the QUALITY component if the pixel has the
*     quality associated with the bit plane. This routine counts the
*     number of such pixels in each bit plane.
*
*     Note, write or update access must be available for the NDF (as
*     set up by routine LPG_ASSOC for instance), and the QUALITY
*     component of the NDF must not be mapped on entry to this routine.
*
*  Arguments:
*     LOCS(5) = CHARACTER * ( * ) (Given)
*        An array of five HDS locators. These locators identify the NDF
*        and the associated quality name information.  They should have
*        been obtained using routine IRQ_FIND or routine IRQ_NEW.
*     SIZE = INTEGER (Given)
*        The number of bit planes for which a count of set pixels is
*        required.
*     SET( SIZE ) = INTEGER (Returned)
*        The number of pixels holding the corresponding quality in each
*        of bit planes 1 to SIZE. The least-significant bit is Bit 1.
*        If SIZE is larger than the number of bit planes in the QUALITY
*        component, the unused elements are set to zero.
*     STATUS = INTEGER (Given and Returned)
*        The global status.

*  Copyright:
*     Copyright (C) 1991 Science & Engineering Research Council.
*     Copyright (C) 2004 Central Laboratory of the Research Councils.
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
*     DSB: David Berry (STARLINK)
*     TIMJ: Tim Jenness (JAC, Hawaii)
*     {enter_new_authors_here}

*  History:
*     25-JUL-1991 (DSB):
*        Original version.
*     2004 September 1 (TIMJ):
*        Use CNF_PVAL
*     24-OCT-2019 (DSB):
*        This routine is now a wrapper around IRQ_CNTQ8.
*     {enter_changes_here}

*  Bugs:
*     {note_any_bugs_here}

*-

*  Type Definitions:
      IMPLICIT NONE              ! No implicit typing

*  Global Constants:
      INCLUDE 'SAE_PAR'          ! Standard SAE constants
      INCLUDE 'IRQ_PAR'          ! IRQ constants.
      INCLUDE 'IRQ_ERR'          ! IRQ error values.

*  Arguments Given:
      CHARACTER LOCS*(*)
      INTEGER SIZE

*  Arguments Returned:
      INTEGER SET( SIZE )

*  Status:
      INTEGER STATUS             ! Global status

*  Local Variables:
      INTEGER I
      INTEGER*8 SET8( IRQ__QBITS )

*.
      IF( SIZE .GT. IRQ__QBITS ) THEN
         IF( STATUS .EQ. SAI__OK ) THEN
            STATUS = IRQ__BDSIZ
            CALL MSG_SETI( 'S', SIZE )
            CALL MSG_SETI( 'M', IRQ__QBITS )
            CALL ERR_REP( ' ', 'IRQ_CNTQ: Too many quality planes '//
     :                    '(^S) requested: must be no more than ^M.',
     :                    STATUS )
         END IF

      ELSE
         CALL IRQ_CNTQ8( LOCS, SIZE, SET8, STATUS )

         DO I = 1, SIZE
            SET( I ) = SET8( I )
            IF( SET( I ) .NE. SET8( I ) .AND. STATUS .EQ. SAI__OK ) THEN
               STATUS = IRQ__OVFLW
               CALL ERR_REP( ' ', 'IRQ_CNTQ: Return value too large '//
     :                       'for 4-byte integer.', STATUS )
            END IF
         END DO

      END IF

      END
