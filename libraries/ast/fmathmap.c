/*
*+
*  Name:
*     fmathmap.c

*  Purpose:
*     Define a FORTRAN 77 interface to the AST MathMap class.

*  Type of Module:
*     C source file.

*  Description:
*     This file defines FORTRAN 77-callable C functions which provide
*     a public FORTRAN 77 interface to the MathMap class.

*  Routines Defined:
*     AST_ISAMATHMAP
*     AST_MATHMAP

*  Copyright:
*     <COPYRIGHT_STATEMENT>

*  Authors:
*     RFWS: R.F. Warren-Smith (Starlink)

*  History:
*     3-SEP-1999 (RFWS):
*        Original version.
*/

/* Define the astFORTRAN77 macro which prevents error messages from
   AST C functions from reporting the file and line number where the
   error occurred (since these would refer to this file, they would
   not be useful). */
#define astFORTRAN77

/* Header files. */
/* ============= */
#include "f77.h"                 /* FORTRAN <-> C interface macros (SUN/209) */
#include "c2f77.h"               /* F77 <-> C support functions/macros */
#include "error.h"               /* Error reporting facilities */
#include "memory.h"              /* Memory handling facilities */
#include "mathmap.h"             /* C interface to the MathMap class */

F77_LOGICAL_FUNCTION(ast_isamathmap)( INTEGER(THIS),
                                      INTEGER(STATUS) ) {
   GENPTR_INTEGER(THIS)
   F77_LOGICAL_TYPE(RESULT);

   astAt( "AST_ISAMATHMAP", NULL, 0 );
   astWatchSTATUS(
      RESULT = astIsAMathMap( astI2P( *THIS ) ) ? F77_TRUE : F77_FALSE;
   )
   return RESULT;
}

F77_INTEGER_FUNCTION(ast_mathmap)( INTEGER(NIN),
                                   INTEGER(NOUT),
                                   INTEGER(NFWD),
                                   CHARACTER_ARRAY(FWD),
                                   INTEGER(NINV),
                                   CHARACTER_ARRAY(INV),
                                   CHARACTER(OPTIONS),
                                   INTEGER(STATUS)
                                   TRAIL(FWD)
                                   TRAIL(INV)
                                   TRAIL(OPTIONS) ) {
   GENPTR_INTEGER(NIN)
   GENPTR_INTEGER(NOUT)
   GENPTR_INTEGER(NFWD)
   GENPTR_CHARACTER_ARRAY(FWD)
   GENPTR_INTEGER(NINV)
   GENPTR_CHARACTER_ARRAY(INV)
   GENPTR_CHARACTER(OPTIONS)
   F77_INTEGER_TYPE(RESULT);
   char **fwd;
   char **inv;
   char *options;
   int i;

   astAt( "AST_MATHMAP", NULL, 0 );
   astWatchSTATUS(
      options = astString( OPTIONS, OPTIONS_length );
      fwd = astStringArray( FWD, *NFWD, FWD_length );
      inv = astStringArray( INV, *NINV, INV_length );

/* Change ',' to '\n' (see AST_SET in fobject.c for why). */
      if ( astOK ) {
         for ( i = 0; options[ i ]; i++ ) {
            if ( options[ i ] == ',' ) options[ i ] = '\n';
         }
      }
      RESULT = astP2I( astMathMap( *NIN, *NOUT, *NFWD, fwd, *NINV, inv,
                                   "%s", options ) );
      astFree( inv );
      astFree( fwd );
      astFree( options );
   )
   return RESULT;
}
