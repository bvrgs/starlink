*+   LOGAR - takes the logarithm of an array with an arbitrary base
      SUBROUTINE KFH_LOGAR(ARRA,INVA,NVALS,BASE,ARRC,INVC,MIN,MAX,
     :                     STATUS)
*    Description :
*      The logarithm of every pixel in an array is taken
*      and the minimum and maximum of the output array are found
*      on the same pass through the data.
*
*      The algorithm used is :-
*         OUT = (LOGe(ABS(IN))/(LOGe(BASE))
*    Invocation :
*     CALL KFH_LOGAR(ARRA,INVA,NVALS,BASE,ARRC,INVC,MIN,MAX,STATUS)
*    Parameters :
*	ARRA(NVALS)		_REAL
*		The input array
*	INVA			_REAL
*		The invalid pixel value for array ARRA
*	NVALS			_INTEGER
*		The size of input and output arrays
*	BASE			_REAL
*		The base for the logarithms (must be positive)
*	ARRC(NVALS)		_REAL
*		The output array
*	INVC			_REAL
*		The invalid pixel value for ARRC
*	MIN			_REAL
*		The minimum value found in ARRC
*	MAX			_REAL
*		The maximum value found for ARRC
*       STATUS                  _INTEGER
*               Status value
*
*    Method :
*       It simply loops around multiplying all valid pixels in the
*       input array by the scalar. In the same pass through the
*	array min and max values are of the output array are found
*
*    Authors :
*	K.F.Hartley	(RGO)
*    History :
*	23-Aug-83	(KFH) First version
*    Type Definitions :
      IMPLICIT NONE
*    Global Constants :
      INCLUDE 'SAE_PAR'
*    Status :
      INTEGER STATUS
*    Local variables :
      INTEGER NVALS			! The size of the arrays
      REAL ARRA(NVALS)			! The input array
      REAL ARRC(NVALS)			! The output array
      REAL INVA				! The invalid pixel value for ARRA
      REAL INVC				! The invalid pixel value for ARRC
      REAL MIN				! The minimum value of ARRC
      REAL MAX				! The maximum value of ARRC
      REAL BASE				! Base for the logarithms
      REAL FACTOR			! Multiplicative factor to
*					! handle base conversion.
      INTEGER I				! The loop index
      LOGICAL SET			! A logical flag for whether
*					! min and max values exist
*    Local data  :
*
*   Set invalid pixel value for output to same as for input
*
      INVC=INVA
*
*   make it possible to calculate logs to any base
*
      FACTOR=1.0/LOG(BASE)
*-

*
*   If the status is bad, then return to the calling program.
*

      IF (STATUS.NE.SAI__OK) THEN

         RETURN

      ELSE

*
*      At the start no estim   Ates have been made of the minimum and
*      maximum values of the output array.
*
         SET=.FALSE.
*
*      Loop through the pixels
*
         DO I=1,NVALS
            IF (ARRA(I).NE.INVA.AND.ARRA(I).GT.0.0) THEN
*
*            If the input is valid and positive
*            calculate the logarithm.
*
               ARRC(I) = LOG(ARRA(I)) * FACTOR
               IF (.NOT.SET) THEN
*
*               If we do not have an initial estimate for min and max
*               use this value, and set the flag.
*
                  MIN=ARRC(I)
                  MAX=ARRC(I)
                  SET=.TRUE.
               ELSE
*
*               If we have then improve the estimate
*
                  IF (ARRC(I).LT.MIN) MIN=ARRC(I)
                  IF (ARRC(I).GT.MAX) MAX=ARRC(I)
               END IF
            ELSE
*
*            If the input is invalid, then so is the output
*
               ARRC(I)=INVC
            END IF
         END DO
*
*      If ALL the pixels are invalid, then set the min and max to the
*      invalid value!
*
         IF (.NOT.SET) THEN
            MIN=INVC
            MAX=INVC
         END IF

      ENDIF
      END
