      SUBROUTINE CCD1_SETWR( INDF, NAME, INDEX, JFRM, STATUS )
*+
*  Name:
*     CCD1_SETWR

*  Purpose:
*     Write Set membership information to an NDF.

*  Language:
*     Starlink Fortran 77.

*  Invocation:
*     CALL CCD1_SETWR( INDF, NAME, INDEX, JFRM, STATUS )

*  Description:
*     This routine writes the SET header to the .MORE.CCDPACK extension
*     of an NDF.  If one already exists it is overwritten.  The WCS
*     extension is also modified, a new frame in the Domain CCD_SET
*     being written (any existing frame with that Domain is erased).
*     This frame is a copy of, and unitmapped to, the frame with
*     frame index JFRM in the WCS component of the NDF.  The Current
*     frame of the WCS extension is left the same as before, except
*     that if the existing Current frame was in the CCD_SET domain,
*     and is hence erased by this routine, the resulting Current
*     frame will be set to the newly added CCD_SET frame.
*
*     Note therefore that it is probably wrong to call this routine
*     prior to writing a WCS component into an NDF (with NDF_PTWCS), 
*     unless it is first read from the NDF (with CCD1_GTWCS) in the
*     interim.

*  Arguments:
*     INDF = INTEGER (Given)
*        NDF identifier of the NDF to be modified.
*     NAME = CHARACTER * ( * ) (Given)
*        A name labelling the Set.  This should be the same for all 
*        members of the same Set, and no two NDFs with the same 
*        SET.INDEX should share the same SET.NAME.  In determining
*        equality of names everything (e.g. case, embedded spaces)
*        apart from leading and trailing spaces is significant.
*     INDEX = INTEGER (Given)
*        A number indicating rank within the Set.  This should be 
*        different for all members of the same Set, i.e. no two 
*        NDFs with the same SET.NAME should share the same SET.INDEX.
*        NDFs in different Sets (i.e. with different SET.NAMEs)
*        which share the same SET.INDEX are in some circumstances
*        (i.e. when presented to the same application) considered to
*        be 'similar' to one another, and will typically have been
*        generated by the same amplifier/CCD combination.
*     JFRM = INTEGER (Given)
*        Index of the frame in the WCS component of the NDF in which
*        the Set is to be considered registered.  A copy of this 
*        frame, connected to it by a UnitMap, will be written into
*        the WCS component in the NDF, with the Domain CCD_SET.
*        The symbolic values AST__CURRENT and AST__BASE may be used.
*        The Current frame of the WCS component will not be disturbed.
*     STATUS = INTEGER (Given and Returned)
*        The global status.

*  Copyright:
*     Copyright (C) 2001 Central Laboratory of the Research Councils

*  Authors:
*     MBT: Mark Taylor (STARLINK)
*     {enter_new_authors_here}

*  History:
*     2-FEB-2001 (MBT):
*        Original version.
*     {enter_changes_here}

*  Bugs:
*     {note_any_bugs_here}

*-

*  Type Definitions:
      IMPLICIT NONE              ! No implicit typing

*  Global Constants:
      INCLUDE 'SAE_PAR'          ! Standard SAE constants
      INCLUDE 'AST_PAR'          ! Standard AST constants
      INCLUDE 'DAT_PAR'          ! Standard HDS constants
      
*  Arguments Given:
      INTEGER INDF
      CHARACTER * ( * ) NAME
      INTEGER INDEX
      INTEGER JFRM
      
*  Status:
      INTEGER STATUS             ! Global status

*  External References:
      INTEGER CHR_LEN
      EXTERNAL CHR_LEN           ! Used length of string

*  Local Variables:
      INTEGER IAT                ! Position in string
      INTEGER IWCS               ! Identifier for WCS frameset
      INTEGER JCUR               ! Index of Current frame
      INTEGER JNEW               ! Index of newly added frame
      INTEGER NAMLEN             ! Length of NAME string
      INTEGER NAXES              ! Number of axes in frame
      INTEGER SETFRM             ! Identifier for new WCS frame
      INTEGER UMAP               ! Identifier for UnitMap
      LOGICAL THERE              ! Is HDS item present?
      CHARACTER * ( DAT__SZLOC ) XLOC ! Locator for the .MORE.CCDPACK exension
      CHARACTER * ( DAT__SZLOC ) SLOC ! Locator for the SET item
      CHARACTER * ( DAT__SZLOC ) ILOC ! Locator for the item being inserted
      CHARACTER * ( AST__SZCHR ) TITLE ! String to use for new frame title
      
*.

*  Check inherited global status.
      IF ( STATUS .NE. SAI__OK ) RETURN

*  Get length of NAME string.
      NAMLEN = CHR_LEN( NAME )

*  Add and populate the .MORE.CCDPACK.SET NDF extension.
*  =====================================================

*  See if the CCDPACK extension exists.
      CALL NDF_XSTAT( INDF, 'CCDPACK', THERE, STATUS )

*  If the extension does not exist then create it.
      IF ( .NOT. THERE ) THEN
         CALL NDF_XNEW( INDF, 'CCDPACK', 'CCDPACK_EXT', 0, 0, XLOC,
     :                  STATUS )

*  Otherwise just get a locator to it.
      ELSE
         CALL NDF_XLOC( INDF, 'CCDPACK', 'UPDATE', XLOC, STATUS )
      END IF

*  If a SET structure exists already, erase it.
      CALL DAT_THERE( XLOC, 'SET', THERE, STATUS )
      IF ( THERE ) THEN
         CALL DAT_ERASE( XLOC, 'SET', STATUS )
      END IF

*  Create a new SET structure; having done this we know it is empty.
      CALL DAT_NEW( XLOC, 'SET', 'CCDPACK_XITEM', 0, 0, STATUS )

*  Get a locator to it.
      CALL DAT_FIND( XLOC, 'SET', SLOC, STATUS )

*  Create, locate, populate and release the SET.NAME item.
      CALL DAT_NEW0C( SLOC, 'NAME', NAMLEN, STATUS )
      CALL DAT_FIND( SLOC, 'NAME', ILOC, STATUS )
      CALL DAT_PUT0C( ILOC, NAME, STATUS )
      CALL DAT_ANNUL( ILOC, STATUS )

*  Create, locate, populate and release the SET.INDEX item.
      CALL DAT_NEW0I( SLOC, 'INDEX', STATUS )
      CALL DAT_FIND( SLOC, 'INDEX', ILOC, STATUS )
      CALL DAT_PUT0I( ILOC, INDEX, STATUS )
      CALL DAT_ANNUL( ILOC, STATUS )

*  Release the .MORE.CCDPACK.SET and .MORE.CCDPACK locators.
      CALL DAT_ANNUL( SLOC, STATUS )
      CALL DAT_ANNUL( XLOC, STATUS )

*  Add a new frame in domain CCD_SET to the WCS frameset.
*  ======================================================

*  Begin a new AST context.
      CALL AST_BEGIN( STATUS )
      
*  Get the WCS frameset from the NDF.
      CALL CCD1_GTWCS( INDF, IWCS, STATUS )

*  Store the index of the Current frame.
      JCUR = AST_GETI( IWCS, 'Current', STATUS )

*  Get a copy of the selected frame.
      SETFRM = AST_COPY( AST_GETFRAME( IWCS, JFRM, STATUS ), STATUS )

*  Set the Domain of the new frame to CCD_SET.
      CALL AST_SETC( SETFRM, 'Domain', 'CCD_SET', STATUS )

*  Set the Title of the new frame appropriately.
      IAT = 0
      CALL CHR_APPND( 'Alignment in CCDPACK Set "', TITLE, IAT )
      CALL CHR_APPND( NAME( :NAMLEN ), TITLE, IAT )
      CALL CHR_APPND( '"', TITLE, IAT )
      CALL AST_SETC( SETFRM, 'Title', TITLE, STATUS )

*  Join the new frame to the WCS frameset using a UnitMap.
      NAXES = AST_GETI( SETFRM, 'Naxes', STATUS )
      UMAP = AST_UNITMAP( NAXES, ' ', STATUS )
      CALL AST_ADDFRAME( IWCS, JFRM, UMAP, SETFRM, STATUS )

*  Restore the Current frame to its previous state.
      CALL AST_SETI( IWCS, 'Current', JCUR, STATUS )

*  Calculate the frame index of the newly added frame.
      JNEW = AST_GETI( IWCS, 'Nframe', STATUS )

*  Ensure that there are no other frames with the domain CCD_SET.
      CALL CCD1_DMPRG( IWCS, 'CCD_SET', .TRUE., JNEW, STATUS )

*  Write the WCS frameset back into the NDF.
      CALL NDF_PTWCS( IWCS, INDF, STATUS )

*  Exit the AST context.
      CALL AST_END( STATUS )

      END
* $Id$
