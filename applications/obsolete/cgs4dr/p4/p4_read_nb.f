*+  P4_READ_NB - Read the contents of the P4_NB noticeboard into common block
      SUBROUTINE P4_READ_NB( PORT, STATUS )
*    Description :
*     This routine defines the contents of the P4_NB noticeboard.
*    Invocation :
*     CALL P4_READ_NB( PORT, STATUS )
*    Authors :
*     P N Daly   (JACH.HAWAII.EDU::PND)
*    History :
*      4-Aug-1994: Original version (PND)
*    endhistory
*    Type Definitions :
      IMPLICIT NONE
*    Global constants :
      INCLUDE 'SAE_PAR'
      INCLUDE 'NBS_ERR'
      INCLUDE 'PRM_PAR'
*    Import :
      INTEGER PORT
*    Status :
      INTEGER STATUS             ! Global status
*    Global variables :
      INCLUDE 'P4COM.INC'        ! P4 common block
*    Local variables :
      INTEGER
     :  ACTBYTES,                ! Actual number of bytes read from NBS
     :  NSTART,                  ! Start of DO-loop value
     :  NEND,                    ! End of DO-loop value
     :  I,                       ! Counter for Do-loop
     :  ERR_STAT                 ! An error status
*-

*   Check for error on entry.
      IF ( STATUS .NE. SAI__OK ) RETURN

*   If 0 <= PORT <= MAXPORT, update just that port
      IF ( PORT.GE.0 .AND. PORT.LE.MAXPORT ) THEN
        NSTART = PORT
        NEND   = PORT
*   Else update them all
      ELSE
        NSTART = 0
        NEND   = MAXPORT
      ENDIF

      IF ( VERBOSE ) THEN
        CALL MSG_OUT( ' ', 'Reading noticeboard into common block', STATUS )
      ENDIF

*   Read the parameters from the common block
      DO I = NSTART, NEND, 1

        DEVICE_NAME(I) = ' '
        CALL NBS_GET_CVALUE( DEVICE_NAME_ID(I), 0, DEVICE_NAME(I), ACTBYTES, STATUS )
C        DEVICE_XOPT(I) = ' '
        CALL NBS_GET_CVALUE( DEVICE_XOPT_ID(I), 0, DEVICE_XOPT(I), ACTBYTES, STATUS )
        DEVICE_YOPT(I) = ' '
        CALL NBS_GET_CVALUE( DEVICE_YOPT_ID(I), 0, DEVICE_YOPT(I), ACTBYTES, STATUS )
        DEVICE_LUT(I) = ' '
        CALL NBS_GET_CVALUE( DEVICE_LUT_ID(I), 0, DEVICE_LUT(I), ACTBYTES, STATUS )
        DISPLAY_DATA(I) = ' '
        CALL NBS_GET_CVALUE( DISPLAY_DATA_ID(I), 0, DISPLAY_DATA(I), ACTBYTES, STATUS )
        TITLE(I) = ' '
        CALL NBS_GET_CVALUE( TITLE_ID(I), 0, TITLE(I), ACTBYTES, STATUS )
        DISPLAY_TYPE(I) = ' '
        CALL NBS_GET_CVALUE( DISPLAY_TYPE_ID(I), 0, DISPLAY_TYPE(I), ACTBYTES, STATUS )
        DISPLAY_PLANE(I) = ' '
        CALL NBS_GET_CVALUE( DISPLAY_PLANE_ID(I), 0, DISPLAY_PLANE(I), ACTBYTES, STATUS )
        CONTOUR_TYPE(I) = ' '
        CALL NBS_GET_CVALUE( CONTOUR_TYPE_ID(I), 0, CONTOUR_TYPE(I), ACTBYTES, STATUS )
        OVERCOLOUR(I) = ' '
        CALL NBS_GET_CVALUE( OVERCOLOUR_ID(I), 0, OVERCOLOUR(I), ACTBYTES, STATUS )
        COLOUR_STYLE(I) = ' '
        CALL NBS_GET_CVALUE( COLOUR_STYLE_ID(I), 0, COLOUR_STYLE(I), ACTBYTES, STATUS )
        FG_COLOUR(I) = ' '
        CALL NBS_GET_CVALUE( FG_COLOUR_ID(I), 0, FG_COLOUR(I), ACTBYTES, STATUS )
        BG_COLOUR(I) = ' '
        CALL NBS_GET_CVALUE( BG_COLOUR_ID(I), 0, BG_COLOUR(I), ACTBYTES, STATUS )
        CUT_DIRECTION(I) = ' '
        CALL NBS_GET_CVALUE( CUT_DIRECTION_ID(I), 0, CUT_DIRECTION(I), ACTBYTES, STATUS )
        LAST_TYPE(I) =  ' '
        CALL NBS_GET_CVALUE( LAST_TYPE_ID(I), 0, LAST_TYPE(I), ACTBYTES, STATUS )

        CALL NBS_GET_VALUE( PLOT_AXES_ID(I), 0, VAL__NBI, PLOT_AXES(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( PLOT_ERRORS_ID(I), 0, VAL__NBI, PLOT_ERRORS(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( PLOT_WHOLE_ID(I), 0, VAL__NBI, PLOT_WHOLE(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( PRE_ERASE_PLOT_ID(I), 0, VAL__NBI, PRE_ERASE_PLOT(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( AUTOSCALE_ID(I), 0, VAL__NBI, AUTOSCALE(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( PORT_OK_ID(I), 0, VAL__NBI, PORT_OK(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( PLOT_OK_ID(I), 0, VAL__NBI, PLOT_OK(I), ACTBYTES, STATUS )

        CALL NBS_GET_VALUE( CONTOUR_LEVELS_ID(I), 0, VAL__NBI, CONTOUR_LEVELS(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( HISTOGRAM_BINS_ID(I), 0, VAL__NBI, HISTOGRAM_BINS(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( HISTOGRAM_XSTEP_ID(I), 0, VAL__NBI, HISTOGRAM_XSTEP(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( HISTOGRAM_YSTEP_ID(I), 0, VAL__NBI, HISTOGRAM_YSTEP(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( HIST_SMOOTH_ID(I), 0, VAL__NBI,  HIST_SMOOTH(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( TOOSMALL_ID(I), 0, VAL__NBI, TOOSMALL(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( TOOLARGE_ID(I), 0, VAL__NBI, TOOLARGE(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( ISTART_ID(I), 0, VAL__NBI, ISTART(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( IEND_ID(I), 0, VAL__NBI, IEND(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( JSTART_ID(I), 0, VAL__NBI, JSTART(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( JEND_ID(I), 0, VAL__NBI, JEND(I), ACTBYTES, STATUS )

        CALL NBS_GET_VALUE( VXSTART_ID(I), 0, VAL__NBR, VXSTART(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( VXEND_ID(I), 0, VAL__NBR, VXEND(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( VYSTART_ID(I), 0, VAL__NBR, VYSTART(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( VYEND_ID(I), 0, VAL__NBR, VYEND(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( AXSTART_ID(I), 0, VAL__NBR, AXSTART(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( AXEND_ID(I), 0, VAL__NBR, AXEND(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( AYSTART_ID(I), 0, VAL__NBR, AYSTART(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( AYEND_ID(I), 0, VAL__NBR, AYEND(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( XSTART_ID(I), 0, VAL__NBR, XSTART(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( XEND_ID(I), 0, VAL__NBR, XEND(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( YSTART_ID(I), 0, VAL__NBR, YSTART(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( YEND_ID(I), 0, VAL__NBR, YEND(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( MODE_ID(I), 0, VAL__NBR, MODE(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( MEAN_ID(I), 0, VAL__NBR, MEAN(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( SIGMA_ID(I), 0, VAL__NBR, SIGMA(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( LOW_ID(I), 0, VAL__NBR, LOW(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( HIGH_ID(I), 0, VAL__NBR, HIGH(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( FMIN_ID(I), 0, VAL__NBR, FMIN(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( FMAX_ID(I), 0, VAL__NBR, FMAX(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( SLICE_START_ID(I), 0, VAL__NBR, SLICE_START(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( SLICE_END_ID(I), 0, VAL__NBR, SLICE_END(I), ACTBYTES, STATUS )
        CALL NBS_GET_VALUE( CHAR_HEIGHT_ID(I), 0, VAL__NBR, CHAR_HEIGHT(I), ACTBYTES, STATUS )
      ENDDO

      IF ( STATUS .NE. SAI__OK ) THEN
        IF ( STATUS .EQ. NBS__NILID ) THEN
          IF ( VERBOSE ) THEN
            CALL MSG_OUT( ' ', 'Noticeboard not yet defined', STATUS )
          ENDIF
        ELSE
          ERR_STAT = STATUS
          STATUS = SAI__ERROR
          CALL MSG_SETI( 'ES', ERR_STAT )
          CALL ERR_REP( ' ', 'P4_READ_NB: '/
     :      /'Failed to restore noticeboard from common block, Status = ^ES', STATUS )
        ENDIF
      ELSE
        IF ( VERBOSE ) THEN
          CALL MSG_OUT( ' ',
     :      'Restored noticeboard from common block OK', STATUS )
        ENDIF
      ENDIF

      END
