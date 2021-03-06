      SUBROUTINE TPWRZT (IERROR)
C
C LATEST REVISION        JULY, 1984
C
C PURPOSE                TO PROVIDE A SIMPLE DEMONSTRATION OF
C                        PWRZT IN CONJUNCTION WITH THREED.
C
C USAGE                  CALL TPWRZT (IERROR)
C
C ARGUMENTS
C
C ON OUTPUT              IERROR
C                          AN INTEGER VARIABLE
C                          = 0, IF THE TEST IS SUCCESSFUL,
C                          = 1, OTHERWISE
C
C I/O                    IF THE TEST IS SUCCESSFUL, THE MESSAGE
C
C                          PWRZT TEST SUCCESSFUL  . . .  SEE PLOT
C                          TO VERIFY PERFORMANCE
C
C                        IS PRINTED ON UNIT 6.
C
C                        IN ADDITION, ONE FRAME CONTAINING THE
C                        CHARACTER PLOT IS PRODUCED ON THE
C                        MACHINE GRAPHICS DEVICE.  TO DETERMINE
C                        IF THE TEST IS SUCCESSFUL, IT IS NECESSARY
C                        TO EXAMINE THIS PLOT.
C
C PRECISION              SINGLE
C
C REQUIRED LIBRARY       PWRZT, THREED
C FILES
C
C
C LANGUAGE               FORTRAN
C
C ALGORITHM              TPWRZT CALLS SUBROUTINES SET3 AND LINE3 FROM
C                        THE ULIB THREED PACKAGE TO ESTABLISH THE
C                        THREE SPACE-TO-TWO SPACE TRANSFORMATION
C                        AND TO DRAW AXIS LINES.  TPWRZT NEXT CALLS
C                        SUBROUTINE PWRZT FROM THE ULIB THREED
C                        PACKAGE TO LABEL THE AXES FOR A THREE SPACE
C                        PLOT.
C
C PORTABILITY            ANSI FORTRAN 77
C
C
C EYE CONTAINS THE (U,V,Z) COORDINATE OF THE EYE POSITION
C
      REAL            EYE(3)
      DATA EYE(1), EYE(2), EYE(3) /3.5, 3.0, 5.0/
C
C INITIALIZE ERROR PARAMETER
C
      IERROR = 1
C
C SELECT NORMALIZATION TRANS NUMBER 0
C
      CALL GSELNT (0)
C
C SUBROUTINE SET3 ESTABLISHES THE MAPPING OF THREE SPACE COORDINATES
C ONTO THE GRAPHICS DEVICE COORDINATE SYSTEM.
C
      CALL SET3 (.1,.9,.1,.9,0.,1.,0.,1.,0.,1.,EYE)
C
C THE FOLLOWING THREE CALLS TO LINE3 DRAW THE THREE SPACE AXES
C
      CALL LINE3 (0.,0.,0.,0.,0.,1.)
      CALL LINE3 (0.,0.,0.,0.,1.,0.)
      CALL LINE3 (0.,0.,0.,1.,0.,0.)
C
C SUBROUTINE PWRZ IS USED TO LABEL EACH OF THE AXES AND THE PLOT
C ON INPUT TO PWRZ,
C THE FIRST THREE PARAMETERS AND ICNT DETERMINE THE POSITION OF THE
C CHARACTER STRING.
C ISIZE DETERMINES THE CHARACTER SIZE.
C LINE AND ITOP DETERMINE THE DIRECTION AND PLANE OF THE CHARACTERS.
C
C
      ICNT = 0
      ISIZE = 30
      LINE = 2
      ITOP = 3
      CALL PWRZT (0.,.5,.1,'V-AXIS',6,ISIZE,LINE,ITOP,ICNT)
C
      LINE = -1
      ITOP = 3
      CALL PWRZT (.5,0.,.1,'U-AXIS',6,ISIZE,LINE,ITOP,ICNT)
C
      LINE = 3
      ITOP = -2
      CALL PWRZT (0.,.1,.5,'Z-AXIS',6,ISIZE,LINE,ITOP,ICNT)
C
      LINE = 2
      ITOP = -1
      ISIZE = 30
      ICNT = -1
      CALL PWRZT (.5,.2,0.,'DEMONSTRATION OF PWRZT WITH THREED',
     1            34,ISIZE,LINE,ITOP,ICNT)
C
C A CALL TO FRAME INDICATES THAT THE PICTURE IS COMPLETE
C
      CALL FRAME
C
      IERROR = 0
      WRITE (6,1001)
C
      RETURN
C
C
C
 1001 FORMAT ('      PWRZT TEST SUCCESSFUL',24X,
     1        'SEE PLOT TO VERIFY PERFORMANCE')
C
      END
