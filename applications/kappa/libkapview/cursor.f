      SUBROUTINE CURSOR ( STATUS )
*+
*  Name:
*     CURSOR

*  Purpose:
*     Reports the co-ordinates of positions selected using the cursor.

*  Language:
*     Starlink Fortran 77

*  Type of Module:
*     ADAM A-task

*  Invocation:
*     CALL CURSOR( STATUS )

*  Arguments:
*     STATUS = INTEGER (Given and Returned)
*        The global status.

*  Description:
*     This application reads co-ordinates from the chosen graphics device 
*     using a cursor and displays them on your terminal. The selected
*     positions may be marked in various ways on the device (see parameter
*     PLOT), and can be written to an output positions list so that subsequent
*     applications can make use of them (see parameter OUTLIST). The format 
*     of the displayed positions may be controlled using parameter STYLE.
*
*     Positions may be reported in several different co-ordinate Frames
*     (see parameter FRAME). Optionally, the corresponding pixel 
*     co-ordinates at each position may also be reported (see parameter
*     SHOWPIXEL).
*
*     The picture or pictures within which positions are required can be 
*     selected in several ways (see parameters MODE and NAME). 
*
*     Restrictions can be made on the number of positions to be given (see
*     parameters MAXPOS and MINPOS), and screen output can be suppressed
*     (see parameter QUIET).

*  Usage:
*     cursor [mode] [name] [outlist] [device]

*  ADAM Parameters:
*     CLOSE = _LOGICAL (Read)
*        This parameter is only accessed if parameter PLOT is set to
*        "Chain" or "Poly". If TRUE, polygons will be closed by joining 
*        the first position to the last position. [current value]
*     DESCRIBE = _LOGICAL (Read)
*        If TRUE, a detailed description of the co-ordinate Frame in which 
*        subsequent positions will be reported is produced each time a 
*        position is reported within a new picture. [current value]
*     DEVICE = DEVICE (Read)
*        The graphics workstation.  This device must support cursor
*        interaction. [current graphics device]
*     EPOCH = _DOUBLE (Read)
*        If a "Sky Co-ordinate System" specification is supplied (using 
*        parameter FRAME) for a celestial co-ordinate system, then an 
*        epoch value is needed to qualify it. This is the epoch at 
*        which the supplied sky positions were determined. It should be
*        given as a decimal years value, with or without decimal places 
*        ("1996.8" for example). Such values are interpreted as a Besselian 
*        epoch if less than 1984.0 and as a Julian epoch otherwise. 
*     FRAME = LITERAL (Read)
*        A string determining the co-ordinate Frame in which positions are 
*        to be reported. When a data array is displayed by an application
*        such as DISPLAY, CONTOUR, etc, WCS information describing the co-ordinate 
*        systems known to the data array are stored with the DATA picture
*        in the graphics database. This application can report positions in
*        any of the co-ordinate Frames stored with each picture. The
*        string supplied for FRAME can be one of the following:
*        - A domain name such as SKY, AXIS, PIXEL, etc. The special domains
*        AGI_WORLD and AGI_DATA are used to refer to the world and data 
*        co-ordinate system stored in the AGI graphics database. They can 
*        be useful if no WCS information was store with the picture when 
*        it was created.
*
*        - An integer value giving the index of the required Frame.
*
*        - A "Sky Co-ordinate System" (SCS) value such as EQUAT(J2000) (see 
*        section "Sky Co-ordinate Systems" in SUN/95).
*
*        If a null value (!) is supplied, positions are reported in the 
*        co-ordinate Frame which was current when the picture was created. 
*        [!]
*     GEODESIC = _LOGICAL (Read)
*        This parameter is only accessed if parameter PLOT is set to
*        "Chain" or "Poly". It specifies whether the curves drawn between
*        positions should be straight lines, or should be geodesic curves.
*        In many co-ordinate Frames geodesic curves will be simple straight 
*        lines. However, in others (such as the majority of celestial 
*        co-ordinates Frames) geodesic curves will be more complex curves 
*        tracing the shortest path between two positions in a non-linear 
*        projection. [FALSE]
*     INFO = _LOGICAL (Read)
*        If TRUE then messages are displayed describing the use of the
*        mouse prior to obtaining the first position. Note, these
*        informational messages are not suppressed by setting parameter 
*        QUIET to TRUE. [TRUE]
*     LASTDIM = _INTEGER (Write)
*        The number of axis values written to parameter LASTPOS.
*     LASTPOS() = _DOUBLE (Write)
*        The unformatted co-ordinates of the last valid position selected 
*        with the cursor, in the co-ordinate Frame which was used to
*        report the position. The number of axis values is written to output
*        parameter LASTDIM. 
*     LOGFILE = FILENAME (Write)
*        The name of the text file in which the formatted co-ordinates of 
*        positions selected with the cursor may be stored. This is intended
*        primarily for recording the screen output, and not for communicating 
*        positions to subsequent applications (use parameter OUTLIST for this 
*        purpose). A null string (!) means that no file is created.  [!]
*     MARKER = _INTEGER (Read)
*        This parameter is only accessed if parameter PLOT is set to
*        "Chain" or "Mark". It specifies the symbol with which each
*        position should be marked, and should be given as an integer 
*        PGPLOT marker type. For instance, 0 gives a box, 1 gives a dot, 
*        2 gives a cross, 3 gives an asterisk, 7 gives a triangle. The 
*        value must be larger than or equal to -31. [current value]
*     MAXPOS = _INTEGER (Read)
*        The maximum number of positions which may be supplied before the
*        application terminates. The number must be in the range 1 to 200. 
*        [200]
*     MINPOS = _INTEGER (Read)
*        The minimum number of positions which may be supplied. The user
*        is asked to supply more if necessary. The number must be in the
*        range 0 to the value of parameter MAXPOS. [0]
*     MODE = LITERAL (Read)
*        The method used to select the pictures in which cursor positions are 
*        to be reported. There are three options:
*        - "Current" -- reports positions within the current picture in the 
*        AGI database. If a position does not lie within the current picture,
*        an extrapolated position is reported, if possible.
*
*        - "Dynamic" -- reports positions within the top-most picture
*        under the cursor in the AGI  database.  Thus the second and 
*        subsequent cursor hits may result in the selection of a new picture.  
*
*        - "Anchor" -- lets the first cursor hit select the picture in
*        which all positions are to be reported. If a subsequent cursor hit
*        falls outside this picture, an extrapolated position is reported if 
*        possible.
*
*        ["Dynamic"]
*     NAME = LITERAL (Read)
*        Only pictures of this name are to be selected.  For instance, if
*        you want positions in a DATA picture which is covered by a 
*        transparent FRAME picture, then you could specify NAME=DATA.
*        A null (!) or blank string means that pictures of all names may 
*        be selected. NAME is ignored when MODE = "Current". [!]
*     NUMBER = _INTEGER (Write)
*        The number of positions selected with the cursor (excluding
*        invalid positions).
*     OUTLIST = FILENAME (Write)
*        An output positions list in which to store the valid selected 
*        positions (only positions in the first selected picture are 
*        recorded). This is the name of a text file which can be used to
*        communicate positions to subsequent applications. It includes
*        information describing the available co-ordinate Frames as well as
*        the positions themselves. If you want a simple record of the 
*        positions displayed on the screen, use parameter LOGFILE instead. 
*        If a null value is supplied, no output positions list is produced. [!]
*     PLOT = LITERAL (Read)
*        The type of graphics to be used to mark the selected positions 
*        which have valid co-ordinates.  The appearance of these graphics
*        (colour, size, etc ) is controlled by the STYLE parameter. PLOT 
*        can take any of the following values:
*        - "None" -- No graphics are produced.
*       
*        - "Mark" -- Each position is marked by the symbol specified
*        by parameter MARKER.
*
*        - "Poly" -- Causes each position to be joined by a line to the 
*        previous position.  These lines may be simple straight lines or
*        geodesic curves (see parameter GEODESIC). The polygons may
*        optionally be closed by joining the last position to the first (see
*        parameter CLOSE).
*
*        - "Chain" -- This is a combination of "Mark" and "Poly". Each 
*        position is marked by a symbol and joined by a line to the previous 
*        position. Parameters MARKER, GEODESIC and CLOSE are used to
*        specify the symbols and lines to use.
*
*        [current value]
*     QUIET = _LOGICAL (Read)
*        If TRUE then positions are not reported on the screen. Output 
*        parameters and files are still created. The display of informational 
*        messages describing the use of the cursor is controlled by the 
*        parameter INFO. [FALSE]
*     SHOWPIXEL = _LOGICAL (Read)
*        If TRUE, the pixel co-ordinates of each selected position are
*        shown on a separate line, following the co-ordinates requested 
*        using parameter FRAME. If pixel co-ordinates are being displayed
*        anyway (see parameter FRAME) then a value of FALSE is used for. 
*        SHOWPIXEL. [current value]
*     STYLE = LITERAL (Read)
*        A group of attribute settings describing the plotting style to use 
*        when drawing the graphics specified by parameter PLOT. The format 
*        of the positions reported on the screen may also be controlled.
*
*        A comma-separated list of strings should be given in which each
*        string is either an attribute setting, or the name of a text file
*        preceded by an up-arrow character "^". Such text files should
*        contain further comma-separated lists which will be read and 
*        interpreted in the same manner. Attribute settings are applied in 
*        the order in which they occur within the list, with later settings
*        over-riding any earlier settings given for the same attribute.
*
*        Each individual attribute setting should be of the form:
*
*           <name>=<value>
*        
*        where <name> is the name of a plotting attribute, and <value> is
*        the value to assign to the attribute. Default values will be
*        used for any unspecified attributes. All attributes will be
*        defaulted if a null value (!) is supplied. See section "Plotting
*        Attributes" in SUN/95 for a description of the available
*        attributes. Any unrecognised attributes are ignored (no error is
*        reported). [current value] 
*
*        In addition to the attributes which control the appearance of
*        the graphics (Colour, Font, etc), the following attributes may
*        be set in order to control the appearance of the formatted axis 
*        values reported on the screen: Format, Digits, Symbol, Unit. These
*        may be suffixed with an axis number (eg "Digits(2)") to refer to 
*        the values displayed for a specific axis. [current value]
*
*  Examples:
*     cursor frame=pixel
*        This obtains co-ordinates within any visible picture for the
*        current graphics device by use of the cursor. Positions are
*        reported in pixel co-ordinates if available, and in the current 
*        co-ordinate Frame of the picture otherwise. 
*     cursor frame=equat(J2010)
*        This obtains co-ordinates within any visible picture for the
*        current graphics device by use of the cursor. Positions are
*        reported in equatorial RA/DEC co-ordinates (referenced to the
*        J2010 equinox) if available, and in the current co-ordinate Frame 
*        of the picture otherwise. 
*     cursor describe plot=mark marker=3 style="colour=red,size=2"
*        As above except, positions are always reported in the current 
*        co-ordinate Frame of each picture. The details of these co-ordinate 
*        Frames are described as they are used. Each selected point is
*        marked with PGPLOT marker 3 (an asterisk). The markers are 
*        red and are twice the default size.
*     cursor current maxpos=2 minpos=2 plot=poly quiet outlist=slice
*        Exactly two positions are obtained within the current picture, 
*        and are joined with a straight line. The positions are written to 
*        a text file called slice but are not displayed on the screen. The
*        text file may be used to communicate the positions to later 
*        applications.
*     cursor name=data style="^mystyle,digits(1)=5,digits(2)=7"
*        This obtains co-ordinates within any visible DATA picture on
*        the current graphics device.  The style to use is read from
*        text file mystyle, but is then modified so that 5 digits are used
*        to format axis 1 values, and 7 to format axis 2 values. 

*  Notes:
*     -  The unformatted values stored in the output parameter LASTPOS, 
*     may not be in the same units as the formatted values shown on
*     the screen and logged to the log file. For instance, unformatted 
*     celestial co-ordinate values are stored in radians. 
*     -  The current picture is unchanged by this application.
*     -  In DYNAMIC and ANCHOR modes, if the cursor is situated at a
*     position where there are no pictures of the selected name, the
*     co-ordinates in the BASE picture are reported.
*     -  Pixel co-ordinates are formatted with 1 decimal place unless a
*     format has already been specified by setting the Format attributes
*     for the axes of the PIXEL co-ordinate Frame (eg using application
*     WCSATTRIB).
*     -  Positions can be removed (the instructions state how), starting
*     from the most-recent one.  Such positions are excluded from the 
*     output positions list and log file (if applicable). If graphics
*     are being used to mark the positions, then removed positions will
*     be highlighted by drawing a marker of type 8 (a circle containing a
*     cross) over the removed positions in a different colour.

*  Related Applications:
*     KAPPA: LISTSHOW, LISTMAKE, INSPECT, PICCUR; Figaro: ICUR, IGCUR.

*  Authors:
*     JM: Jo Murray  (STARLINK)
*     MJC: Malcolm J. Currie  (STARLINK)
*     DSB: David S. Berry (STARLINK)
*     {enter_new_authors_here}

*  History:
*     22-MAY-1989 (JM):
*        Original version.
*     1989 Jun 28 (MJC):
*        Added error checking, some tidying, fixed several bugs, only
*        reports the picture name and comment when it changes.
*     1989 Jul 10 (MJC):
*        Fixed "SGS zone too small" bug by reordering the code so that
*        the MODE is obtained before the device; extended the prologue.
*     1989 Oct 19 (MJC):
*        Fixed the synchronisation for terminals, and added commentary
*        for both terminals and image displays.
*     1989 Oct 24 (MJC):
*        Added the log file and name options, and redesigned the
*        description.
*     1989 Nov 10 (MJC):
*        Calls a subroutine to prepare the cursor.
*     1990 Jan 9 (MJC):
*        Corrected SGS status.
*     1990 Apr 20:
*        Added output of the current picture's label, if it exists.
*     1991 February 8 (MJC):
*        Added AGI context control, and tidied unwanted picture
*        identifiers.
*     1991 March 19 (MJC):
*        Converted to SST prologue.
*     1991 April 9 (MJC):
*        Added AGI begin-and-end block.  Obtains data co-ordinates if
*        present.
*     1991 May 14 (MJC):
*        Added COSYS parameter.
*     1992 January 29 (MJC):
*        Fixed bug in list of options for MODE.
*     1992 February 19 (MJC):
*        AGI behaviour has changed, so call new AGI routine to test
*        whether or not the picture has changed between cursor
*        selections.
*     1992 March 3 (MJC):
*        Replaced AIF parameter-system calls by the extended PAR
*        library.
*     1993 May 24 (MJC):
*        Added XC and YC output parameters.
*     1995 August 24 (MJC):
*        Added the COLOUR and PLOT parameters and example of their use.
*        No longer selects a new current picture (use PICCUR instead).
*        Made usage and examples lowercase.  Added Related Applications.
*     1995 December 16 (MJC):
*        Added NUMBER, XP, and YP output parameters.  Allowed erasure
*        of previous position.
*     1996 May 31 (MJC):
*        Improved and simplified the graphics, so the marker has a
*        uniform size, and plotting can occur outside the current
*        SGS picture in ANCHOR and CURRENT modes.
*     1997 March 11 (MJC):
*        Fixed initialisation bug for parameter NUMBER, and hence XP and
*        YP.  Fixed bug where SGS pen was being set when PLOT="None".
*     28-AUG-1998 (DSB):
*        Radical changes to use PGPLOT and AST for graphics and
*        co-ordinate handling.
*     {enter_further_changes_here}

*  Bugs:
*     {note_any_bugs_here}

*-

*  Type Definitions:
      IMPLICIT NONE              ! No implicit typing

*  Global Constants:
      INCLUDE 'SAE_PAR'          ! Standard SAE constants
      INCLUDE 'DAT_PAR'          ! HDS constants
      INCLUDE 'NDF_PAR'          ! NDF constants
      INCLUDE 'AST_PAR'          ! AST constants
      INCLUDE 'PRM_PAR'          ! VAL__ constants
      INCLUDE 'PAR_ERR'          ! Parameter-system errors
      INCLUDE 'AGI_ERR'          ! AGI error constants

*  Status:
      INTEGER STATUS             ! Global status

*  External References:
      INTEGER CHR_LEN            ! Used length of a string

*  Local Constants:
      INTEGER MAXPTS             ! Maximum number of positions
      PARAMETER ( MAXPTS = 200 )

      INTEGER SZNAM              ! Length of picture name
      PARAMETER ( SZNAM = 15 )

      INTEGER ANC                ! ANCHOR mode
      PARAMETER ( ANC = 1 )

      INTEGER DYN                ! DYNAMIC mode
      PARAMETER ( DYN = 2 )

      INTEGER CUR                ! CURRENT mode
      PARAMETER ( CUR = 3 )

*  Local Variables:
      CHARACTER AMES( 3 )*30     ! Informational messages about use of cursor
      CHARACTER ATTRIB*10        ! Attribute name
      CHARACTER COMENT*256       ! Comment for the latest picture
      CHARACTER DOM*20           ! Domain of Current Frame in Plot
      CHARACTER KEYS*3           ! Keys which activate each cursor action
      CHARACTER LABEL*( SZNAM )  ! Picture label
      CHARACTER LINE*256         ! Text buffer for screen
      CHARACTER LOGLIN*256       ! Text buffer for log file
      CHARACTER MODE*10          ! Mode for selecting pictures
      CHARACTER NAME*( DAT__SZNAM ) ! Name of pictures which can be selected
      CHARACTER PLOT*15          ! Nature of required graphics
      CHARACTER PNAME*( DAT__SZNAM )! Name for the latest picture
      CHARACTER PURP*80          ! Purpose for using cursor
      DOUBLE PRECISION CXY( NDF__MXDIM )! Current Frame position
      DOUBLE PRECISION FINISH( NDF__MXDIM )! Position at end of polygon edge
      DOUBLE PRECISION GXY( 2 )  ! Graphics position
      DOUBLE PRECISION PXY( NDF__MXDIM )! PIXEL Frame position
      DOUBLE PRECISION START( NDF__MXDIM ) ! Position at start of polygon edge
      DOUBLE PRECISION XB        ! Cursor X position in BASE world co-ords
      DOUBLE PRECISION XY( MAXPTS, NDF__MXDIM ) ! Array of all x,y data co-ordinates
      DOUBLE PRECISION XYOUT( MAXPTS, NDF__MXDIM ) ! X,Y co-ords in 1st picture
      DOUBLE PRECISION YB        ! Cursor Y position in BASE world co-ords
      INTEGER ACT                ! Cursor choice
      INTEGER BMAP               ! GRAPHICS to BASE world co-ords Mapping
      INTEGER CHAN               ! Pointer to AST channel
      INTEGER FRM1               ! Pointer to required Frame
      INTEGER FRM2               ! Pointer to required secondary Frame
      INTEGER GRPSIZ             ! No. of elements in a GRP group
      INTEGER ICOL( NDF__MXDIM ) ! Minimum column no. for start of each field
      INTEGER I                  ! Loop count
      INTEGER IAGDAT             ! Index of AGI_DATA Frame
      INTEGER IAT                ! No. of characters in the string
      INTEGER ICURR              ! Original Current Frame index
      INTEGER IGRP               ! GRP identifier for group of formatted posns
      INTEGER IMARK              ! PGPLOT marker type
      INTEGER IMODE              ! Mode of operation
      INTEGER IPIC               ! AGI id for current picture
      INTEGER IPIC0              ! Current (input) picture identifier
      INTEGER IPIC1              ! Picture identifier for 1st selected picture
      INTEGER IPIC2              ! AGI id for new picture
      INTEGER IPICB              ! BASE picture identifier
      INTEGER IPIX               ! Index of PIXEL Frame
      INTEGER IPLOT              ! Plot for current picture
      INTEGER IPLOTB             ! Plot for BASE picture
      INTEGER IPLOTP             ! Plot for drawing polygons
      INTEGER JAT                ! No. of characters in the string
      INTEGER JCOL( NDF__MXDIM ) ! Minimum column no. for start of each field
      INTEGER MAP1               ! Pointer to Base->Current Mapping
      INTEGER MAP2               ! Pointer to Base->secondary Frame Mapping
      INTEGER MAXP               ! Max. no. of positions which may be supplied.
      INTEGER MINP               ! Min. no. of positions which may be supplied.
      INTEGER NACT               ! No. of cursor actions 
      INTEGER NAX                ! No. of axes in current position
      INTEGER NAXP               ! No. of axes in polygon Plot
      INTEGER NOB                ! The number of objects written
      INTEGER NOUTAX             ! No. of axes in first selected picture
      INTEGER NOUTIP             ! Pointer to Plot for first selected picture
      INTEGER NOUTPS             ! No. of positons in first selected picture
      INTEGER NP                 ! The number of positions selected
      INTEGER NPNT               ! No. of cursor positions supplied 
      INTEGER OLDCOL             ! Original marker colour index
      INTEGER RBMODE             ! PGPLOT rubber band mode
      LOGICAL CLOSE              ! Close the polygon?
      LOGICAL DESC               ! Describe each Coordinate Frame?
      LOGICAL FIRST              ! Reading first position in any picture?
      LOGICAL GEO                ! Draw geodesic polygons?
      LOGICAL GOOD               ! Are all axis values good?
      LOGICAL INFO               ! Display mouse instructions?
      LOGICAL LOOP               ! Continue to get a new cursor position?
      LOGICAL NEWPIC             ! Reading first position in a new picture?
      LOGICAL PGOOD              ! Are all pixel axis values good?
      LOGICAL PLURAL             ! Use the plural form of a word in a message?
      LOGICAL QUIET              ! Run quietly?
      LOGICAL SAME               ! Is given picture same as current picture?
      LOGICAL SHPIX              ! Are additional PIXEL co-ords to be displayed?
      REAL OLDSIZ                ! Original marker size
      REAL X1                    ! PGPLOT X world coord at bottom left
      REAL X2                    ! PGPLOT X world coord at top right
      REAL XAC( MAXPTS )         ! PGPLOT X world coord at all positions
      REAL XC                    ! PGPLOT X world coord at current cursor posn
      REAL Y1                    ! PGPLOT Y world coord at bottom left
      REAL Y2                    ! PGPLOT Y world coord at top right
      REAL YAC( MAXPTS )         ! PGPLOT Y world coord at all positions
      REAL YC                    ! PGPLOT Y world coord at current cursor posn
*.

*  Check the inherited global status.
      IF( STATUS .NE. SAI__OK ) RETURN

*  Start a new AST context.
      CALL AST_BEGIN( STATUS )

*  Get the maximum number of positions which may be selected. Use the 
*  largest possible value as the default.
      CALL PAR_GDR0I( 'MAXPOS', MAXPTS, 1, MAXPTS, .FALSE., MAXP, 
     :                STATUS )

*  Get the minimum number of positions which may be selected. Use 
*  zero as the default.
      CALL PAR_GDR0I( 'MINPOS', 0, 0, MAXP, .FALSE., MINP, STATUS )

*  Get the mode parameter.
      CALL PAR_CHOIC( 'MODE', 'Dynamic', 'Dynamic,Current,Anchor',
     :                .TRUE., MODE, STATUS )
      IF( MODE .EQ. 'DYNAMIC' ) THEN
         IMODE = DYN
      ELSE IF( MODE .EQ. 'CURRENT' ) THEN
         IMODE = CUR
      ELSE
         IMODE = ANC
      END IF      

*  See if Frame descriptions are required.
      CALL PAR_GET0L( 'DESCRIBE', DESC, STATUS )

*  See if PIXEL co-ords are to be displayed on a separate line.
      CALL PAR_GET0L( 'SHOWPIXEL', SHPIX, STATUS )

*  See if we are to supress display all information on the screen.
      CALL PAR_GET0L( 'QUIET', QUIET, STATUS )

*  See what type of graphics are required. 
      CALL PAR_CHOIC( 'PLOT', 'None', 'Poly,Mark,Chain,None', .TRUE., 
     :                PLOT, STATUS )

*  Abort if an error has occurred.
      IF( STATUS .NE. SAI__OK ) GO TO 999

*  Get the other parameter values needed to describe the graphics, and 
*  set the rubber band mode to use (none, unless linear "Poly" graphics are
*  being produced, in which case use a straight line rubber band).
      RBMODE = 0

      IF( PLOT .EQ. 'MARK' .OR. PLOT .EQ. 'CHAIN' ) THEN
         CALL PAR_GDR0I( 'MARKER', 2, -31, 10000, .FALSE., IMARK, 
     :                   STATUS )
      END IF

      IF( PLOT .EQ. 'POLY' .OR. PLOT .EQ. 'CHAIN' ) THEN
         CALL PAR_GET0L( 'CLOSE', CLOSE, STATUS )
         CALL PAR_GET0L( 'GEODESIC', GEO, STATUS )
         IF( .NOT. GEO ) RBMODE = 1

      END IF

*  Abort if an error has occurred.
      IF( STATUS .NE. SAI__OK ) GO TO 999

*  Get the NAME parameter.  A null value is made equivalent to a blank
*  string, i.e. all pictures of any name may be selected.  It cannot
*  apply to the CURRENT mode because there is only one current picture
*  and that is already defined.
      IF( IMODE .EQ. DYN .OR. IMODE .EQ. ANC ) THEN
         CALL PAR_GET0C( 'NAME', NAME, STATUS )
         IF( STATUS .EQ. PAR__NULL ) THEN
            CALL ERR_ANNUL( STATUS )
            NAME = ' '
         END IF
      END IF

*  Abort if an error has occurred.
      IF( STATUS .NE. SAI__OK ) GO TO 999

*  Create a GRP group to store the formatted positions.
      CALL GRP_NEW( 'Positions', IGRP, STATUS )

*  Set up the graphics system.
*  ===========================

*  Abort if an error has occurred.
      IF( STATUS .NE. SAI__OK ) GO TO 999

*  Open the graphics device for plotting with PGPLOT, obtaining an
*  identifier for the current AGI picture.
      CALL AGP_ASSOC( 'DEVICE', 'UPDATE', ' ', .FALSE., IPIC0, STATUS )

*  If the device could not be opened, cancel the parameter association
*  so that a different device will be used next time. Otherwise, the
*  association is retained so that the same device will be used.
      IF( STATUS .NE. SAI__OK .AND. STATUS .NE. PAR__ABORT .AND.
     :   STATUS .NE. PAR__NULL ) THEN
         CALL ERR_BEGIN( STATUS )
         CALL AGP_DEASS( 'DEVICE', .TRUE., STATUS )
         CALL ERR_BEGIN( STATUS )
      END IF

*  Get the the AGI identifier and AST Plot associated with the BASE picture. 
*  The Base Frame in this Plot is GRAPHICS co-ordinates (millimetres from
*  the bottom left corner of the view surface), and the Current Frame is
*  AGI world co-ordinates in the BASE picture. The PGPLOT viewport is set 
*  to match the BASE picture and PGPLOT world co-ordinates within the
*  viewport are GRAPHICS co-ordinates. The BASE picture becomes the
*  current picture in KPG1_GDGET, so re-instate the original current
*  picture. 
      CALL AGI_IBASE( IPICB, STATUS )
      CALL KPG1_GDGET( IPICB, AST__NULL, .FALSE., IPLOTB, STATUS )
      CALL AGI_SELP( IPIC0, STATUS )

*  Get the Mapping from GRAPHICS co-ordinates to AGI world co-ordinates in 
*  the BASE picture.
      BMAP = AST_GETMAPPING( IPLOTB, AST__BASE, AST__CURRENT, STATUS )

*  If we are producing graphics, set its style. 
      IF( PLOT .NE. 'NONE' ) THEN
         CALL KPG1_ASSET( 'CURSOR', 'STYLE', IPLOTB, STATUS )
      END IF

*  Set the PGPLOT viewport and AST Plot for the initially selected picture.
*  This is the original current picture in "CURRENT" mode, or the BASE picture 
*  in any other mode. The PGPLOT viewport is set equal to the selected 
*  picture, with world co-ordinates giving millimetres form the bottom left 
*  corner of the view surface. The selected picture is made current. The 
*  returned Plot may include a Frame with Domain AGI_DATA representing AGI 
*  DATA co-ordinates (defined by a TRANSFORM structure stored with the 
*  picture in the database).
      IF( IMODE .EQ. CUR ) THEN
         CALL AGI_ICURP( IPIC, STATUS )
         CALL KPG1_GDGET( IPIC, AST__NULL, .TRUE., IPLOT, STATUS )
      ELSE
         CALL AGI_IBASE( IPIC, STATUS )
         IPLOT = AST_CLONE( IPLOTB, STATUS )
         CALL AGI_SELP( IPIC, STATUS )
      END IF

*  Abort if an error has occurred.
      IF( STATUS .NE. SAI__OK ) GO TO 999

*  The cursor will be positioned initially at the centre of the picture
*  selected above. Get the bounds the PGPLOT window, and store the mid
*  position.
      CALL PGQWIN( X1, X2, Y1, Y2 )
      XC = 0.5*( X1 + X2 )
      YC = 0.5*( Y1 + Y2 )

*  Loop round displaying positions.
*  ================================

*  Put out a blank line to ensure the commentary appears on the alpha
*  plane of the terminal.
      CALL MSG_BLANK( STATUS )

*  Indicate that the next position will be the first position reported
*  in any picture.
      FIRST = .TRUE.

*  See if instructions on the use of the mouse should be displayed.
      CALL PAR_GET0L( 'INFO', INFO, STATUS )

*  Store the instructions for the allowed actions. Also store the keys
*  which must be pressed to activate the corresponding action. The action 
*  to forget the previous position is only of any use if the user is allowed 
*  to supply more than 1 point.
      IF( MAXP .GT. 1 ) THEN
         NACT = 3
         KEYS = ' D.'
         AMES( 1 ) = 'select a position'
         AMES( 2 ) = 'forget the previous position'
         AMES( 3 ) = 'quit'
      ELSE
         NACT = 2
         KEYS = ' .'
         AMES( 1 ) = 'select a position'
         AMES( 2 ) = 'quit'
      END IF

*  Store the purpose of using the cursor.
      PURP = 'select'
      IAT = 6
      PLURAL = ( MAXP .GT. 1 )

*  An exact number of positions is required...
      IF( MAXP .EQ. MINP ) THEN
         IAT = IAT + 1
         CALL CHR_PUTI( MAXP, PURP, IAT )

*  There is a lower limit but no upper limit on the number of positions...
      ELSE IF( MINP .GT. 0 .AND. MAXP .EQ. MAXPTS ) THEN
         CALL CHR_APPND( ' at least', PURP, IAT )
         IAT = IAT + 1
         CALL CHR_PUTI( MINP, PURP, IAT )
         PLURAL = ( MINP .GT. 1 )

*  There is an upper limit but no lower limit on the number of positions...
      ELSE IF( MINP .EQ. 0 .AND. MAXP .LT. MAXPTS ) THEN
         CALL CHR_APPND( ' up to', PURP, IAT )
         IAT = IAT + 1
         CALL CHR_PUTI( MAXP, PURP, IAT )

*  There are both lower and upper limits on the number of positions...
      ELSE IF( MINP .GT. 0 .AND. MAXP .LT. MAXPTS ) THEN
         CALL CHR_APPND( ' between', PURP, IAT )
         IAT = IAT + 1
         CALL CHR_PUTI( MINP, PURP, IAT )
         CALL CHR_APPND( ' and', PURP, IAT )
         IAT = IAT + 1
         CALL CHR_PUTI( MAXP, PURP, IAT )
         PLURAL = .TRUE.

      END IF

*  Complete the purpose message.
      IF( PLURAL ) THEN
         CALL CHR_APPND( ' positions', PURP, IAT )
      ELSE
         CALL CHR_APPND( ' position', PURP, IAT )
      END IF
      IF( .NOT. QUIET ) CALL CHR_APPND( ' to be reported', PURP, IAT )
 
*  Initialise the number of positions supplied so far.
      NP = 0
      NOUTPS = 0

*  Initialise the AGI identifier for the first selected picture.
      IPIC1 = -1

*  Loop until no more positions are required, or an error occurs.
      LOOP = .TRUE.
      DO WHILE( LOOP .AND. STATUS .EQ. SAI__OK )

*  Get a position using the cursor, in PGPLOT world co-ordinates. This 
*  corresponds to the GRAPHICS Frame of the current Plot (i.e. millimetres 
*  from the bottom left corner of the view surface). The positions which 
*  may be selected are unrestricted ang may be given anywhere on the 
*  view surface.
         CALL KPG1_PGCUR( INFO, PURP, NACT, AMES, KEYS( : NACT ), 0.0, 
     :                    0.0, 0.0, 0.0, NACT - 1, XC, YC, 1, RBMODE, 0,
     :                    0, -32, XC, YC, ACT, NPNT, STATUS )

*  Indicate that instructions on the use of the mouse should not be 
*  displayed again.
         INFO = .FALSE.

*  Leave the loop if no position was given, so long as the requirement for
*  the minimum number of positions has been met.
         IF( NPNT .EQ. 0 ) THEN

            IF( NP .GE. MINP ) THEN
               LOOP = .FALSE.

            ELSE
               CALL MSG_BLANK (STATUS )

               IF( MINP .LT. MAXP ) THEN
                  CALL MSG_SETC( 'T1', 'Please supply at least' )
               ELSE
                  CALL MSG_SETC( 'T1', 'Please supply' )
               END IF

               CALL MSG_SETI( 'M', MINP - NP )

               IF( MINP - NP .EQ. 1 ) THEN
                  CALL MSG_SETC( 'T2', 'position' )
               ELSE
                  CALL MSG_SETC( 'T2', 'positions' )
               END IF

               CALL MSG_OUT( ' ','^T1 ^M more ^T2...', STATUS )
               
            END IF

*  If the first action ("select a position") was performed, add the position
*  to the list of valid positions.
*  ======================================================================
         ELSE IF( ACT .EQ. 1 ) THEN

*  There is now a change from the graphics cursor operation to report
*  values on the text screen (assuming the device is a terminal).  In
*  order for the message to appear in the correct plane, there must be
*  a delay, so that the graphics system can complete its work before
*  the (faster and independent) message system reports the cursor
*  position.  The following calls achieves this synchronisation.
            CALL MSG_SYNC( STATUS )

*  Transform the cursor position from GRAPHICS co-ordinates into the AGI
*  world co-ordinate system of the BASE picture.
            CALL AST_TRAN2( BMAP, 1, DBLE( XC ), DBLE( YC ), .TRUE., 
     :                      XB, YB, STATUS ) 

*  Find the picture in which the reported positon is required. This will
*  depend on MODE. In CURRENT mode the picture never changes. The new
*  picture is made the current AGI picture.
            IF( IMODE .EQ. DYN .OR. 
     :          ( IMODE .EQ. ANC .AND. FIRST ) ) THEN

*  Make the BASE picture the current AGI picture,
               CALL AGI_SELP( IPICB, STATUS )

*  Get the last picture of the chosen name which encompasses the cursor
*  position. If found it becomes the current AGI picture.
               CALL AGI_RCLP( NAME, REAL( XB ), REAL( YB ), IPIC2, 
     :                        STATUS )

*  Watch for the case when there is no picture of that name at the
*  selected position. Annul the error and use the BASE picture.  
               IF( STATUS .EQ. AGI__NONAM ) THEN
                  CALL ERR_ANNUL( STATUS )
                  CALL AGI_IBASE( IPIC2, STATUS )
                  CALL AGI_SELP( IPIC2, STATUS )
               END IF

            END IF

*  Decide if the cursor is situated in the same picture as last time.
            CALL AGI_ISAMP( IPIC, SAME, STATUS )

*  If there is a new picture, replace the current picture identifier with the 
*  new one. Dont annul the current AGI identifier since we may need it later 
*  on (to compare with IPIC1).
            IF( .NOT. SAME ) THEN
               IPIC = IPIC2

*  Annul the pointer for the current Plot.
               CALL AST_ANNUL( IPLOT, STATUS )

*  Make the current PGPLOT viewport match the selected picture, and get a
*  Plot for drawing in it. The returned Plot may include a Frame with Domain 
*  AGI_DATA representing AGI DATA co-ordinates (defined by a TRANSFORM 
*  structure stored with the picture in the database).
               CALL KPG1_GDGET( IPIC, AST__NULL, .TRUE., IPLOT, STATUS )
            END IF

*  Abort if an error has occurred.
            IF( STATUS .NE. SAI__OK ) GO TO 999

*  See if this is the first position to be reported in a new picture.
            NEWPIC = ( .NOT. SAME .OR. FIRST ) 

*  Set the properties of the Plot if we are about to report the first
*  position in a new picture.
            IF( NEWPIC ) THEN

*  Set the Current Frame in the Plot to the requested Frame. If the
*  requested Frame is not available in the Plot, re-prompt for a new
*  FRAME only if we are not using DYNAMIC mode.
               CALL KPG1_ASFRM( 'FRAME', 'EPOCH', IPLOT, ' ', 
     :                          'AGI_DATA', ( IMODE .NE. DYN ), STATUS )

*  If the requested Frame was not available, annul the error.
               IF( STATUS .EQ. SAI__ERROR ) CALL ERR_ANNUL( STATUS )

*  Set the Style of the Plot using the STYLE parameter. This is done so
*  that the Format of each axis value (for instance) can be controlled 
*  using STYLE.
               CALL KPG1_ASSET( 'CURSOR', 'STYLE', IPLOT, STATUS )

*  Get the number of axes in the selected Frame.
               NAX = AST_GETI( IPLOT, 'NAXES', STATUS )

*  Get the simplified Mapping from Base (GRAPHICS) Frame to the Current 
*  (selected) Frame.
               MAP1 = AST_SIMPLIFY( AST_GETMAPPING( IPLOT, AST__BASE,
     :                                              AST__CURRENT, 
     :                                              STATUS ), STATUS )

*  Get a pointer to the Current Frame.
               FRM1 = AST_GETFRAME( IPLOT, AST__CURRENT, STATUS )

*  Attempt to find a PIXEL Frame.
               CALL KPG1_ASFFR( IPLOT, 'PIXEL', IPIX, STATUS )

*  If found, get a pointer to it.
               IF( IPIX .NE. AST__NOFRAME ) THEN
                  FRM2 = AST_GETFRAME( IPLOT, IPIX, STATUS )

*  If no format has already been set for the pixel axes, set them so that
*  the displayed pixel co-ords have 1 decimal place. Loop round each
*  pixel axis.
                  DO I = 1, AST_GETI( FRM2, 'NAXES', STATUS )

*  Form the name of the Format attribute for this axis.
                     ATTRIB = 'FORMAT('
                     IAT = 7
                     CALL CHR_PUTI( I, ATTRIB, IAT )
                     CALL CHR_APPND( ')', ATTRIB, IAT )

*  If is not already set, set the Format to %8.1f (i.e. minimum field of
*  10 characters including 1 decimal place).
                     IF( .NOT. AST_TEST( FRM2, ATTRIB( : IAT ), 
     :                                   STATUS ) ) THEN
                        CALL AST_SETC( FRM2, ATTRIB( : IAT ), '%8.1f', 
     :                                 STATUS )
                     END IF

                  END DO

*  If PIXEL co-ordinates are to be displayed as well as the required
*  co-ordinate Frame get the simplified Mapping from Base (GRAPHICS) Frame 
*  to the PIXEL Frame.
                  IF( SHPIX ) THEN
                     MAP2 = AST_SIMPLIFY( AST_GETMAPPING( IPLOT, 
     :                                                   AST__BASE,
     :                                                   IPIX, STATUS ), 
     :                                    STATUS )
                  END IF

*  Store a null pointer if there is no PIXEL Frame.
               ELSE
                  FRM2 = AST__NULL
               END IF

*  Put out a blank line.
               IF( .NOT. QUIET ) CALL MSG_BLANK( STATUS )

*  Obtain details of the new picture.
               CALL AGI_ICOM( COMENT, STATUS )
               CALL AGI_INAME( PNAME, STATUS )
               CALL AGI_ILAB( IPIC, LABEL, STATUS )
               DOM = AST_GETC( IPLOT, 'DOMAIN', STATUS )

* Construct a message giving these details.
               LINE = ' '
               IAT = 0

               CALL CHR_APPND( 'Picture comment:', LINE, IAT )
               IAT = IAT + 1
               CALL CHR_APPND( COMENT, LINE, IAT )

               CALL CHR_APPND( ', name:', LINE, IAT )
               IAT = IAT + 1
               CALL CHR_APPND( PNAME, LINE, IAT )

               IF( LABEL( 1 : 1 ) .NE. ' ' ) THEN
                  CALL CHR_APPND( ', label:', LINE, IAT )
                  IAT = IAT + 1
                  CALL CHR_APPND( LABEL, LINE, IAT )
               END IF

               IF( DOM .NE. ' ' ) THEN
                  CALL CHR_APPND( ', reporting:', LINE, IAT )
                  IAT = IAT + 1
                  CALL CHR_APPND( DOM, LINE, IAT )
                  CALL CHR_APPND( ' co-ordinates', LINE, IAT )
               END IF

*  Display this line.
               IF( .NOT. QUIET ) CALL MSG_OUT( ' ', LINE( : IAT ), 
     :                                         STATUS )

*  Give a detailed description of the Frame in which positions will be 
*  reported if required.
               IF( DESC .AND. .NOT. QUIET ) THEN
                  CALL KPG1_DSFRM( IPLOT, 'Positions in this '//
     :                    'picture will be reported in the following '//
     :                    'co-ordinate Frame:', STATUS )
               END IF

*  If this is the first selected picture, store an AGI identifier for it.
*  Also, note the number of axes in the first picture, and store the 
*  Plot for the first picture. This information is needed when creating the
*  output positions list.  
               IF( IPIC1 .EQ. -1 ) THEN
                  CALL AGI_ICURP( IPIC1, STATUS )
                  NOUTAX = NAX
                  NOUTIP = AST_COPY( IPLOT, STATUS )
               END IF

            END IF

*  Map the GRAPHICS position into the required Frame, and format the results 
*  including axis symbols. 
            LINE = ' '
            IAT = 1
            CALL KPS1_CURFM( FRM1, MAP1, XC, YC, NAX, .TRUE., NEWPIC, 
     :                       IAT, LINE, ICOL, GOOD, CXY, STATUS )

*  Display the formatted values on the screen if required.
            IF( .NOT. QUIET ) CALL MSG_OUT( ' ', LINE( : IAT ), STATUS )

*  Now format the position again, for inclusion in the log file. Do not
*  include axis symbols or units.
            LOGLIN = ' '
            JAT = 1
            CALL KPS1_CURFM( FRM1, MAP1, XC, YC, NAX, .FALSE., NEWPIC, 
     :                       JAT, LOGLIN, JCOL, GOOD, CXY, STATUS )

*  Append the formatted values to the GRP group which will be written out
*  to the log file at the end.
            CALL GRP_PUT( IGRP, 1, LOGLIN( : JAT ), 0, STATUS ) 

*  If required, show the corresponding pixel co-ordinates. Do not display
*  them if pixel co-ords are already being displayed.
            IF( SHPIX .AND. IPIX .NE. AST__NOFRAME .AND.
     :          DOM .NE. 'PIXEL' ) THEN

*  Map the GRAPHICS position into the PIXEL Frame, and format the results 
*  without axis symbols. Indent by 3 spaces.
               LINE = ' ('
               IAT = 2
               CALL KPS1_CURFM( FRM2, MAP2, XC, YC, NAX, .FALSE., 
     :                          NEWPIC, IAT, LINE, ICOL, PGOOD, PXY, 
     :                          STATUS )
               CALL CHR_APPND( ')', LINE, IAT )

*  Display the formatted values on the screen if required.
               IF( .NOT. QUIET ) THEN
                  CALL MSG_OUT( ' ', LINE( : IAT ), STATUS )
                  CALL MSG_BLANK( STATUS )
               END IF

*  Append it to the current line in the log file.
               CALL GRP_GRPSZ( IGRP, GRPSIZ, STATUS )
               CALL GRP_GET( IGRP, GRPSIZ, 1, LOGLIN, STATUS ) 
               IAT = CHR_LEN( LOGLIN ) + 3
               CALL CHR_APPND( LINE, LOGLIN, IAT )
               CALL GRP_PUT( IGRP, 1, LOGLIN( : IAT ), GRPSIZ, STATUS ) 

            END IF

*  If good, include the current Frame position in the list of returned 
*  positions.
            IF( GOOD  ) THEN
               NP = NP + 1

               DO I = 1, NAX
                  XY( NP, I ) = CXY( I )
               END DO

               DO I = NAX + 1, NDF__MXDIM
                  XY( NP, I ) = AST__BAD
               END DO

*  Also store the GRAPHICS Frame positions.
               XAC( NP ) = XC       
               YAC( NP ) = YC       

*  If the current picture is the same as the first selected picture, store
*  the position in the array of positions to be written to the output
*  positions list.
               CALL AGI_ISAMP( IPIC1, SAME, STATUS )
               IF( SAME ) THEN
                  NOUTPS = NOUTPS + 1

                  DO I = 1, NAX
                     XYOUT( NOUTPS, I ) = CXY( I )
                  END DO
   
                  DO I = NAX + 1, NDF__MXDIM
                     XYOUT( NOUTPS, I ) = AST__BAD
                  END DO

               END IF                

*  Produce any required graphics. First deal with markers.
               IF( PLOT .EQ. 'MARK' .OR. PLOT .EQ. 'CHAIN' ) THEN               
                  START( 1 ) = XB                 
                  START( 2 ) = YB                 
                  CALL AST_MARK( IPLOTB, 1, 2, 1, START, IMARK, STATUS )
               END IF

*  Now deal with polygons. Do not draw anything until the second position
*  is given. 
               IF( NP .GT. 1 .AND. ( PLOT .EQ. 'POLY' .OR. 
     :                               PLOT .EQ. 'CHAIN' ) ) THEN

*  Select the required Plot. For linear Polygons, draw in the current
*  Frame of the BASE picture. For geodesic polygons, draw in the current
*  Frame of the current picture.
                  IF( .NOT. GEO ) THEN
                     IPLOTP = IPLOTB
                     NAXP = 2
                  ELSE
                     IPLOTP = IPLOT
                     NAXP = NAX
                  END IF

*  Transform the previous GRAPHICS position into the current Frame. This
*  is the starting position for the next edge of the polygon.
                  GXY( 1 ) = DBLE( XAC( NP - 1 ) )
                  GXY( 2 ) = DBLE( YAC( NP - 1 ) )
                  CALL AST_TRANN( IPLOTP, 1, 2, 1, GXY, .TRUE., NAXP, 1, 
     :                            START, STATUS )

*  Transform the current GRAPHICS position into the current Frame. This
*  is the finishing position for the next edge of the polygon.
                  GXY( 1 ) = DBLE( XC )
                  GXY( 2 ) = DBLE( YC )
                  CALL AST_TRANN( IPLOTP, 1, 2, 1, GXY, .TRUE., NAXP, 1, 
     :                            FINISH, STATUS )

*  Draw the curve.
                  CALL AST_CURVE( IPLOTP, START, FINISH, STATUS ) 

               END IF

            END IF

*  Leave the loop if the maximum number of positions have been supplied.
            IF( NP .EQ. MAXP ) LOOP = .FALSE.

*  Indicate that the next position will not be the first position.
            FIRST = .FALSE.

*  If the second action ("forget previous position") was performed, 
*  remove the previous position from the list of valid positions.
*  ==============================================================
         ELSE IF( ACT .EQ. 2 ) THEN
            IF( .NOT. QUIET ) CALL MSG_BLANK( STATUS )

*  Tell the user if there are no positions to forget.
            IF( NP .EQ. 0 ) THEN
               CALL MSG_OUT( ' ', 'There are no positions to forget.',
     :                       STATUS )

*  Tell the user if there is a position to forget.
            ELSE
               CALL GRP_GET( IGRP, NP, 1, LINE, STATUS )
               CALL MSG_SETC( 'POS', LINE )
               CALL MSG_OUT( ' ', 'Forgetting the previous position '//
     :                       '(^POS).', STATUS )

*  Set the next cursor position to the position which is to be forgotten.
*  These are GRAPHICS co-ords.
               XC = XAC( NP )            
               YC = YAC( NP )            

*  If Graphics are being drawn, display an "erased" symbol at the position
*  which has just been forgotten.
               IF( PLOT .NE. 'NONE' ) THEN

*  Temporarily make the GRAPHICS (Base) Frame current so that we can give
*  the position in GRAPHICS co-ordinates.
                  ICURR = AST_GETI( IPLOTB, 'CURRENT', STATUS )
                  CALL AST_SETI( IPLOTB, 'CURRENT',
     :                           AST_GETI( IPLOTB, 'BASE', STATUS ),
     :                           STATUS )

*  If markers are being drawn, temporarily increase the marker size.
                  IF( PLOT .EQ. 'MARK' .OR. PLOT .EQ. 'CHAIN' ) THEN
                     OLDSIZ = AST_GETR( IPLOTB, 'SIZE(MARK)', STATUS )
                     CALL AST_SETR( IPLOTB, 'SIZE(MARK)', 1.2*OLDSIZ, 
     :                              STATUS )
                  END IF

*  Temporarily set the marker colour to colour index 4 (or 3 if colour 
*  index 4 is already in use).
                  IF( PLOT .EQ. 'MARK' .OR. PLOT .EQ. 'CHAIN' ) THEN
                     OLDCOL = AST_GETI( IPLOTB, 'COLOUR(MARK)', STATUS )
                  ELSE
                     OLDCOL = AST_GETI( IPLOTB, 'COLOUR(CURVE)', 
     :                                  STATUS )
                  END IF

                  IF( OLDCOL .NE. 4 ) THEN
                     CALL AST_SETI( IPLOTB, 'COLOUR(MARK)', 4, STATUS )
                  ELSE
                     CALL AST_SETI( IPLOTB, 'COLOUR(MARK)', 3, STATUS )
                  END IF

*  Draw the "erase" symbol - a circle with a cross in it by preference
*  (symbol 8). If this is already in use, use a box with positions (symbol 10).
                  START( 1 ) = REAL( XC )
                  START( 2 ) = REAL( YC )          

                  IF( IMARK .NE. 8 ) THEN
                     CALL AST_MARK( IPLOTB, 1, 2, 1, START, 8, STATUS )
                  ELSE
                     CALL AST_MARK( IPLOTB, 1, 2, 1, START, 10, STATUS )
                  END IF

*  Re-instate the original Plot attributes.
                  IF( PLOT .EQ. 'MARK' .OR. PLOT .EQ. 'CHAIN' ) THEN
                     CALL AST_SETR( IPLOTB, 'SIZE(MARK)', OLDSIZ, 
     :                              STATUS )
                     CALL AST_SETI( IPLOTB, 'COLOUR(MARK)', OLDCOL, 
     :                              STATUS )
                  END IF

                  CALL AST_SETI( IPLOTB, 'CURRENT', ICURR, STATUS )

               END IF

*  Reduce the number of positions stored by one.
               NP = NP - 1

*  Reduce the size of the group to remove the last entry.
               CALL GRP_SETSZ( IGRP, NP, STATUS )

*  We now need to decide whether the position should also be removed from 
*  the list of positions to write to the ouptput positions list. This is 
*  only the case if the position lies within the first selected picture.

*  In ANCHOR or CURRENT mode, the position must be in the original picture
*  since the picture cannot change.
               IF( IMODE .NE. DYN ) THEN
                  SAME = .TRUE.

*  In DYNAMIC mode we need to find the picture and check it is the same.
               ELSE

*  Transform the cursor position from GRAPHICS co-ordinates into the AGI
*  world co-ordinate system of the BASE picture.
                  CALL AST_TRAN2( BMAP, 1, DBLE( XC ), DBLE( YC ), 
     :                            .TRUE., XB, YB, STATUS ) 

*  Temporarily, make the BASE picture the current AGI picture,
                  CALL AGI_SELP( IPICB, STATUS )

*  Get the last picture of the chosen name which encompasses the cursor
*  position. If found it becomes the current AGI picture.
                  CALL AGI_RCLP( NAME, REAL( XB ), REAL( YB ), IPIC2, 
     :                           STATUS )

*  Watch for the case when there is no picture of that name at the
*  selected position. Annul the error and use the BASE picture.  
                  IF( STATUS .EQ. AGI__NONAM ) THEN
                     CALL ERR_ANNUL( STATUS )
                     CALL AGI_IBASE( IPIC2, STATUS )
                     CALL AGI_SELP( IPIC2, STATUS )
                  END IF

*  See if this picture is the same as the first selected picture.
                  CALL AGI_ISAMP( IPIC1, SAME, STATUS )

*  Re-instate the original current AGI picture,
                  CALL AGI_SELP( IPIC, STATUS )

               END IF

*  If this picture is the same as the first selected picture, we need to
*  remove this position from the output list. It's the last position, so
*  just reduce the number of positions to write out by one.
               IF( SAME ) NOUTPS = NOUTPS - 1

            END IF

         END IF

      END DO

*  Tidy up.
*  ========
      CALL MSG_BLANK( STATUS )

*  Close any polygons being drawn (if requested)
      IF( NP .GT. 2 .AND. CLOSE .AND. ( PLOT .EQ. 'POLY' .OR. 
     :                                  PLOT .EQ. 'CHAIN' ) ) THEN

*  Select the required Plot. For linear Polygons, draw in the current
*  Frame of the BASE picture. For geodesic polygons, draw in the current
*  Frame of the current picture.
         IF( .NOT. GEO ) THEN
            IPLOTP = IPLOTB
         ELSE
            IPLOTP = IPLOT
         END IF

*  Transform the last GRAPHICS position into the current Frame. This
*  is the starting position for the next edge of the polygon.
         GXY( 1 ) = DBLE( XAC( NP ) )
         GXY( 2 ) = DBLE( YAC( NP ) )
         CALL AST_TRANN( IPLOTP, 1, 2, 1, GXY, .TRUE., NAX, 1, 
     :                   START, STATUS )

*  Transform the first GRAPHICS position into the current Frame. This
*  is the finishing position for the next edge of the polygon.
         GXY( 1 ) = DBLE( XAC( 1 ) )
         GXY( 2 ) = DBLE( YAC( 1 ) )
         CALL AST_TRANN( IPLOTP, 1, 2, 1, GXY, .TRUE., NAX, 1, 
     :                   FINISH, STATUS )

*  Draw the curve.
         CALL AST_CURVE( IPLOTP, START, FINISH, STATUS ) 

      END IF 

*  Store the number of valid positions given in the output parameter
*  NUMBER.
      CALL PAR_PUT0I( 'NUMBER', NP, STATUS )

*  If any valid positions were given, make them available to subsequent
*  applications.
      IF( NP .GT. 0 ) THEN

*  Store the final position in the output parameter LAST.
         DO I = 1, NAX
            CXY( I ) = XY( NP, I )
         END DO
         CALL PAR_PUT1D( 'LASTPOS', NAX, CXY, STATUS )

*  Store the number of axes in the final position in the output parameter
*  LASTDIM.
         CALL PAR_PUT0I( 'LASTDIM', NAX, STATUS )

*  Create a a logfile containing formatted values if required.
         CALL GRP_LIST( 'LOGFILE', 0, 0, ' ', IGRP, STATUS ) 

*  The Plot stored in the output positions list must not have an AGI_DATA 
*  Frame since there is no guarantee that the correct AGI picture will be 
*  current when it is used in another subsequent application (the mapping 
*  routine KPG1_ASAGD uses the TRANSFORM structure from the AGI picture 
*  which is current at the time KPG1_ASAGD is called). Find and remove
*  any AGI_DATA Frame in the Plot.
         CALL KPG1_ASFFR( NOUTIP, 'AGI_DATA', IAGDAT, STATUS )

         IF( IAGDAT .NE. AST__NOFRAME ) THEN
            CALL AST_REMOVEFRAME( NOUTIP, IAGDAT, STATUS )
         END IF

*  Create an output positions list if required.
         IF( NOUTPS .GT. 0 ) THEN
            CALL KPG1_WRLST( 'OUTLIST', MAXPTS, NOUTPS, NOUTAX, XYOUT,
     :                       AST__CURRENT, NOUTIP, 'Output from CURSOR', 
     :                       1, 0, .TRUE., STATUS )
         END IF

      END IF

*  Shutdown procedure.
*  ===================
  999 CONTINUE

*  Delete the GRP group holding formatted positions.
      CALL GRP_DELET( IGRP, STATUS )

*  Shutdown PGPLOT and the graphics database.
      CALL ERR_BEGIN( STATUS )
      CALL AGP_DEASS( 'DEVICE', .FALSE., STATUS )
      CALL ERR_END( STATUS )

*  End the AST context.
      CALL AST_END( STATUS )

*  Give a contextual error message if anything went wrong.
      IF( STATUS .NE. SAI__OK ) THEN
         CALL ERR_REP( 'CURSOR_ERR', 'CURSOR: Failed to select '//
     :                 'positions using a graphics cursor.', STATUS )
      END IF

      END
