	SUBROUTINE STCOADD_AVER( NX, NY, NUMIM, IMAGE_OUT)

	IMPLICIT NONE

	INTEGER NX, NY, J, K, NUMBAD, NUMIM

	REAL IMAGE_OUT( NX, NY)

	NUMBAD = 0

	DO J = 1, NY
	  DO K = 1, NX
	    IMAGE_OUT( K, J) = IMAGE_OUT( K, J)/NUMIM
	  END DO
	END DO

	END
