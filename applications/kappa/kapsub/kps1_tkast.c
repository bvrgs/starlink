#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ast.h"
#include "f77.h"
#include "sae_par.h"
#include <tcl.h>
#include <tk.h>

/*
 * The following variable is a special hack that is needed in order for
 * Sun shared libraries to be used for Tcl.
 */

#ifdef NEED_MATHERR
extern int matherr();
int *tclDummyMathPtr = (int *) matherr;
#endif

static FILE *fd = NULL;

extern F77_SUBROUTINE(err_rep)( CHARACTER(param), CHARACTER(mess),
                                INTEGER(STATUS) TRAIL(param) TRAIL(mess) );

static void Error( const char *, int * );
static char *Envir( const char *, int * );
static void SetVar( Tcl_Interp *, char *, char *, int, int * );
static const char *GetVar( Tcl_Interp *, char *, int, int * );
static void SetSVar( Tcl_Interp *, const char *, const char *, int, int * );
static void GetSVar( Tcl_Interp *, const char *, char *, int, int * );
static void SetIVar( Tcl_Interp *, const char *, int, int * );
static void SetRVar( Tcl_Interp *, const char *, float, int * );
static void SetLVar( Tcl_Interp *, const char *, LOGICAL(a), int * );
static void GetLVar( Tcl_Interp *, const char *, LOGICAL(a), int * );
static void GetIVar( Tcl_Interp *, const char *, int *, int * );
static void GetRVar( Tcl_Interp *, const char *, float *, int * );
static void sink( const char * );

F77_SUBROUTINE(kps1_tkast)( INTEGER(IAST), CHARACTER(TITLE),
                            INTEGER(FULL), INTEGER(STATUS) TRAIL(TITLE) ){
/*
*  Name:
*     KPS1_TKAST

*  Purpose:
*     Displays an AST Object using a Tk interface.

*  Description:
*     This C function creates a Tcl interpreter to execute the Tcl script
*     which implements the GUI for the TK AST browser. 

*  Parameters:
*     IAST = INTEGER (Given)
*        AST Pointer to the Object to be displayed.
*     TITLE = CHARACTER * ( * ) (Given)
*        A text string to display with the Object.
*     FULL = INTEGER (Given)
*        The value to use for the AST "Full" attribute when displaying
*        the Object.
*     STATUS = INTEGER (Given and Returned)
*        The inherited global status.

*  Authors:
*     DSB: David Berry (STARLINK)
*     TIMJ: Tim Jenness (JAC, Hawaii)
*     {enter_new_authors_here}

*  History:
*     4-MAR-1997 (DSB):
*        Original version.
*     3-JUL-2002 (DSB):
*        Replaced use of tmpnam with mkstemp.
*     2-SEP-2004 (TIMJ):
*        Use sae_par.h for good/bad status
*     4-OCT-2004 (DSB):
*        Renamed from kpg1_tkast to kps1_tkast and moved from kaplibs to
*        kappa.
*     4-OCT-2004 (TIMJ):
*        Fix compiler warnings
*     {enter_changes_here}

*  Bugs:
*     {note_any_bugs_here}

*/

   GENPTR_INTEGER(IAST)
   GENPTR_CHARACTER(TITLE)
   GENPTR_INTEGER(FULL)
   GENPTR_INTEGER(STATUS)

   AstChannel *chan;
   Tcl_Interp *interp = NULL;
   char fname[ 255 ];
   char script[1024];
   int ifd;

#if ( (TK_MAJOR_VERSION == 4) && (TK_MINOR_VERSION == 0) )
   Tk_Window main;
#endif

/* Check the global status. */
   if( *STATUS != SAI__OK ) return;

/* Get a unique temporary file name. This file is used to store the object 
   dumps. All this complication is needed
   to avoid the warning message generated by the linker on RH 7 Linux 
   resulting from the use of the simpler "tmpnam" function. */
   strcpy( fname, "tkastXXXXXX" );
   ifd = mkstemp( fname );
   if( ifd == -1 ){
      *STATUS = SAI__ERROR;
      Error( "Unable to create a temporary \"tkast\" file name.", 
              STATUS );
      return;
   } else {
      close( ifd );
      remove( fname );
   }

/* Open the temporary text file. */
   fd = fopen( fname, "w" );
   if( fd ){

/* Create a Channel through which the AST Objects can be written out. */
      chan = astChannel( NULL, sink, "comment=1" );
      astSetI( chan, "full", *FULL );

/* Dump the object. */
      astWrite( chan, astI2P( *IAST ) );

/* Annul the channel, and close the file. */
      chan = astAnnul( chan );
      fclose( fd );
      fd = NULL;

/* Report an error if anything went wrong in AST. */
      if( !astOK ) {
         *STATUS = SAI__ERROR;
         Error( "An error has been reported by the AST library.", STATUS );

/* Otherwise, create a TCL interpreter. */
      } else {   
         interp = Tcl_CreateInterp ();
      }

/* Store the name of the temprary file in Tcl variable "FILE". */
      SetVar( interp, "FILE", fname, TCL_LEAVE_ERR_MSG, STATUS );

/* Store the supplied title string in Tcl variable TITLE. */
      SetSVar( interp, "TITLE", TITLE, TITLE_length, STATUS );

#if ( (TK_MAJOR_VERSION == 4) && (TK_MINOR_VERSION == 0) )

/* Create the main window, and report an error if it fails. */
      if( *STATUS == SAI__OK ) {
         main = Tk_CreateMainWindow(interp, NULL, "Tkast", "TKAST" );
         if( !main ) {
            *STATUS = SAI__ERROR;
            Error( "Unable to create main Tk window.", STATUS );
            Error( interp->result, STATUS );
         }
      }

#endif

/* Initialise Tcl and Tk commands. */
      if( *STATUS == SAI__OK ) {
   
         if( Tcl_Init( interp ) != TCL_OK ) {
            *STATUS = SAI__ERROR;
            Error( "Failed to initialise Tcl commands.", STATUS );
            Error( interp->result, STATUS );
   
         } else if( Tk_Init( interp ) != TCL_OK ) {
            *STATUS = SAI__ERROR;
            Error( "Failed to initialise Tk commands.", STATUS );
            Error( interp->result, STATUS );

         }
      }

/* Execute the TCL script. This should be $KAPPA_DIR/tkast.tcl */
      (void) strcpy( script, Envir( "KAPPA_DIR", STATUS ) );
      if( *STATUS == SAI__OK ){
         (void) strcpy( script + strlen( script ), "/tkast.tcl" );

         if( Tcl_EvalFile( interp, script ) != TCL_OK ){
            *STATUS = SAI__ERROR;
            Error( "Failed to execute the TCL script...", STATUS );
            Error( interp->result, STATUS );

/* If succesfull, loop infinitely, waiting for commands to execute.  When 
   there are no windows left, the loop exits. NOTE, it seems that an
   "exit" command in the tcl script causes the current process to be
   killed. In order to shutdown the script and return control to this
   procedure, use "destroy ." in the script instead of "exit". */
         } else {
            Tk_MainLoop(); 
         }
      }

#if ( (TK_MAJOR_VERSION == 4) && (TK_MINOR_VERSION == 0) )

/* If an error has occurred, ensure that the main Tk window has been
   destroyed. */
      if( *STATUS != SAI__OK && main ) Tk_DestroyWindow( main );

#endif

/* Delete the TCL interpreter. */
      if( interp && *STATUS == SAI__OK ) Tcl_DeleteInterp( interp );

/* Remove the temporary file. */
      remove( fname );

/* Report an error if the temporary file could not be opened. */
   } else {
      *STATUS = SAI__ERROR;
      Error( "File to open temporary text file to hold dumps of AST Objects.", STATUS );
   }

}

static void Error( const char *text, int *STATUS ) {
/*
*  Name:
*     Error

*  Purpose:
*     Report an error using EMS.

*  Description:
*     The supplied text is used as the text of the error message.
*     A blank parameter name is used.

*  Parameters:
*     text
*        The error message text. Only the first 80 characters are used.
*     STATUS
*        A pointer to the global status value. This should have been set
*        to a suitable error value before calling this function.

*  Notes:
*     - If a NULL pointer is supplied for "text", no error is reported.
*/

   DECLARE_CHARACTER(param,1);
   DECLARE_CHARACTER(mess,80);
   int j;

/* Check the supplied pointer. */
   if( text ) {

/* Set the parameter name to a blank string. */
      param[0] = ' ';

/* Copy the first "mess_length" characters of the supplied message into 
      "mess". */
      strncpy( mess, text, mess_length );

/* Pad any remaining bytes with spaces (and replace the terminating null
   character with a space). */
      for( j = strlen(mess); j < mess_length; j++ ) {
         mess[ j ] = ' ';
      }

/* Report the error. */
      F77_CALL(err_rep)( CHARACTER_ARG(param), CHARACTER_ARG(mess),
                         INTEGER_ARG(STATUS) TRAIL_ARG(param) 
                         TRAIL_ARG(mess) );
   }
}

static char *Envir( const char *var, int *STATUS ){
/*
*  Name:
*     Envir

*  Purpose:
*     Get an environment variable.

*  Description:
*     A pointer to the a string holding the value of the specified 
*     environment variable is returned. A NULL pointer is returned an an
*     error is reported if the variable does not exist.

*  Parameters:
*     var
*        The variable name.
*     STATUS
*        A pointer to the global status value. 

*/
   char *ret;
   char mess[80];

   if( *STATUS != SAI__OK || !var ) return NULL;

   ret = getenv( var );      
   if( !ret ) {
      *STATUS = SAI__ERROR;
      sprintf( mess, "Failed to get environment variable \"%s\".", var );
      Error( mess, STATUS );
   }

   return ret;
}

static void SetVar( Tcl_Interp *interp,  char *name,  char *value, 
             int flags, int *STATUS ){
/*
*  Name:
*     SetVar

*  Purpose:
*     Sets a Tcl variable.

*  Description:
*     This is equivalent to the Tcl function Tcl_SetVar, except that
*     it checks the global status before executing, and reports an error
*     if anything goes wrong.

*  Parameters:
*     As for Tcl_SetVar, except for addition of final STATUS argument.
*     
*/

   char mess[80];

   if( *STATUS != SAI__OK ) return;

   if( !Tcl_SetVar( interp, name, value, flags) ){
      *STATUS = SAI__ERROR;
      sprintf( mess, "Error setting TCL variable \"%s\".", name );
      Error( mess, STATUS );
      Error( interp->result,  STATUS );     
   }
}

static const char *GetVar( Tcl_Interp *interp,  char *name,  int flags, int *STATUS ){
/*
*  Name:
*     GetVar

*  Purpose:
*     Gets a Tcl variable.

*  Description:
*     This is equivalent to the Tcl function Tcl_GetVar, except that
*     it checks the global status before executing, and reports an error
*     if anything goes wrong.

*  Parameters:
*     As for Tcl_GetVar, except for addition of final STATUS argument.
*     
*/

   char mess[80];
   const char *ret;

   if( *STATUS != SAI__OK ) return NULL;

   ret = Tcl_GetVar( interp, name, flags );
   if ( !ret ) {
      *STATUS = SAI__ERROR;
      sprintf( mess, "Error getting TCL variable \"%s\".", name );
      Error( mess, STATUS );
      Error( interp->result,  STATUS );     
   }

   return ret;

}

static void SetSVar( Tcl_Interp *interp, const char *var, const char *string, 
               int len, int *STATUS ) {
/*
*  Name:
*     SetSVar

*  Purpose:
*     Store an F77 string in a Tcl variable.

*  Description:
*     This function stores the supplied F77 string in the specified Tcl
*     variable, appending a trailing null character.

*  Parameters:
*     interp = Tcl_Interp * (Given)
*        A pointer to the Tcl interpreter structure.
*     var = const char * (Given)
*        The name of the Tcl variable to use.
*     string = const char * (Given)
*        The string to store.
*     len = int (Given)
*        The length of the string to be stored, excluding any trailing
*        null.
*     STATUS = int * (Given and Returned)
*        The inherited status.
*     
*/

   char *buf;
   char mess[81];

/* Check the inherited status. */
   if( *STATUS != SAI__OK ) return;

/* Allocate memory to hold a null-terminated copy of the supplied F77
   string. */
   buf = (char *) malloc ( sizeof(char)*(size_t) ( len + 1 ) );

/* If successful, copy the string, and append a trailing null character. */
   if ( buf ) {
      memcpy( buf, string, len );
      buf[ len ] = 0;

/* Set the Tcl variable value. */
      SetVar( interp, (char *) var, buf, TCL_LEAVE_ERR_MSG, STATUS );

/* Free the memory. */
      free( buf );

/* Report an error if the memory could not be allocated. */
   } else {
      *STATUS = SAI__ERROR;
      sprintf( mess, "Unable to allocate %d bytes of memory. ", len + 1 );
      Error( mess, STATUS );
      sprintf( mess, "Failed to initialise Tcl variable \"%s\".", var );
      Error( mess, STATUS );
   }

}

static void GetSVar( Tcl_Interp *interp, const char *var, char *string, 
              int len, int *STATUS ) {
/*
*  Name:
*     GetSVar

*  Purpose:
*     Get an F77 string from a Tcl variable.

*  Description:
*     This function gets a string from the specified Tcl
*     variable, and stores it in the supplied F77 character variable.

*  Parameters:
*     interp = Tcl_Interp * (Given)
*        A pointer to the Tcl interpreter structure.
*     var = const char * (Given)
*        The name of the Tcl variable to use.
*     string = char * (Returned)
*        The F77 string to receive the value.
*     len = int (Given)
*        The length of the F77 string.
*     STATUS = int * (Given and Returned)
*        The inherited status.
*     
*/

   const char *tp;
   int n;
   int i;

/* Check the inherited status. */
   if( *STATUS != SAI__OK ) return;

/* Get a pointer to the null-terminated string Tcl variable value. */
   tp = GetVar( interp, (char *) var, TCL_LEAVE_ERR_MSG, STATUS );

/* If succesful, initialise the returned F77 string to hold blanks, and
   then copy the required number of characters form the Tcl variable
   string. */
   if ( tp ) {
      for( i = 0; i < len; i++ ) string[ i ] = ' ';
      n = strlen( tp );
      if( len < n ) n = len;
      memcpy( string, tp, n );
   }

}

static void SetIVar( Tcl_Interp *interp, const char *var, int val, int *STATUS ) {
/*
*  Name:
*     SetIVar

*  Purpose:
*     Store an integer in a Tcl variable.

*  Description:
*     This function stores the integer in the specified Tcl variable.

*  Parameters:
*     interp = Tcl_Interp * (Given)
*        A pointer to the Tcl interpreter structure.
*     var = const char * (Given)
*        The name of the Tcl variable to use.
*     val = int (Given)
*        The value to store.
*     STATUS = int * (Given and Returned)
*        The inherited status.
*     
*/

   char text[80];

/* Check the inherited status. */
   if( *STATUS != SAI__OK ) return;

/* Format the integer value and store the resulting string in the 
   Tcl variable. */
   sprintf( text, "%d", val );
   SetVar( interp, (char *) var, text, TCL_LEAVE_ERR_MSG, STATUS );

}

static void SetRVar( Tcl_Interp *interp, const char *var, float val, int *STATUS ) {
/*
*  Name:
*     SetRVar

*  Purpose:
*     Store a floating point value in a Tcl variable.

*  Description:
*     This function stores the supplied value in the specified Tcl variable.

*  Parameters:
*     interp = Tcl_Interp * (Given)
*        A pointer to the Tcl interpreter structure.
*     var = const char * (Given)
*        The name of the Tcl variable to use.
*     val = float (Given)
*        The value to store.
*     STATUS = int * (Given and Returned)
*        The inherited status.
*     
*/

   char text[80];

/* Check the inherited status. */
   if( *STATUS != SAI__OK ) return;

/* Format the value and store the resulting string in the Tcl variable. */
   sprintf( text, "%g", val );
   SetVar( interp, (char *) var, text, TCL_LEAVE_ERR_MSG, STATUS );

}

static void SetLVar( Tcl_Interp *interp, const char *var, LOGICAL(valptr), int *STATUS ) {
/*
*  Name:
*     SetLVar

*  Purpose:
*     Store an F77 LOGICAL value in a Tcl variable.

*  Description:
*     This function stores the supplied value in the specified Tcl variable.

*  Parameters:
*     interp = Tcl_Interp * (Given)
*        A pointer to the Tcl interpreter structure.
*     var = const char * (Given)
*        The name of the Tcl variable to use.
*     valptr = LOGICAL (Given)
*        A pointer to the value to store.
*     STATUS = int * (Given and Returned)
*        The inherited status.
*     
*/

   GENPTR_LOGICAL(val)

/* Check the inherited status. */
   if( *STATUS != SAI__OK ) return;

/* Store the value. */
   SetVar( interp, (char *) var, ( F77_ISTRUE(*valptr) ? "1" : "0" ), 
           TCL_LEAVE_ERR_MSG, STATUS );

}

static void GetLVar( Tcl_Interp *interp, const char *var, LOGICAL(valptr), int *STATUS ) {
/*
*  Name:
*     GetLVar

*  Purpose:
*     Retrieve a Tcl variable value and store it in an F77 LOGICAL
*     variable.

*  Parameters:
*     interp = Tcl_Interp * (Given)
*        A pointer to the Tcl interpreter structure.
*     var = const char * (Given)
*        The name of the Tcl variable to use.
*     valptr = LOGICAL (Returned)
*        A pointer to the F77 variable to receive the returned value.
*     STATUS = int * (Given and Returned)
*        The inherited status.
*     
*/

   GENPTR_LOGICAL(val)
   const char *tp;

/* Check the inherited status. */
   if( *STATUS != SAI__OK ) return;

/* Get a pointer to the text string holding the Tcl variable value. */
   tp = GetVar( interp, (char *) var, TCL_LEAVE_ERR_MSG, STATUS );

/* Tcl uses zero to represent false, and non-zero to represent true. */
   if ( tp && !strcmp( tp, "0" ) ) {
      *valptr = F77_FALSE;
   } else {
      *valptr = F77_TRUE;
   }
}

static void GetRVar( Tcl_Interp *interp, const char *var, float *valptr, int *STATUS ) {
/*
*  Name:
*     GetRVar

*  Purpose:
*     Retrieve a Tcl variable value and store it in a float.

*  Parameters:
*     interp = Tcl_Interp * (Given)
*        A pointer to the Tcl interpreter structure.
*     var = const char * (Given)
*        The name of the Tcl variable to use.
*     valptr = float * (Returned)
*        A pointer to the variable to receive the returned value.
*     STATUS = int * (Given and Returned)
*        The inherited status.
*     
*/

   const char *tp;
   char mess[81];

/* Check the inherited status. */
   if( *STATUS != SAI__OK ) return;

/* Get a pointer to the Tcl variable value string. */
   tp = GetVar( interp, (char *) var, TCL_LEAVE_ERR_MSG, STATUS );

/* If ok, extract a floating point value from it. Report an error if the
   conversion fails.  */
   if ( tp ) {
      if( sscanf( tp, "%g", valptr ) != 1 ) {
         *STATUS = SAI__ERROR;
         sprintf( mess, "\"%s\" is not a floating point value.", tp );
         Error( mess, STATUS );
         sprintf( mess, "Failed to obtained a value for Tcl variable \"%s\".", var );
         Error( mess, STATUS );
      }
   }
}

static void GetIVar( Tcl_Interp *interp, const char *var, int *valptr, int *STATUS ) {
/*
*  Name:
*     GetIVar

*  Purpose:
*     Retrieve a Tcl variable value and store it in an integer.

*  Parameters:
*     interp = Tcl_Interp * (Given)
*        A pointer to the Tcl interpreter structure.
*     var = const char * (Given)
*        The name of the Tcl variable to use.
*     valptr = int * (Returned)
*        A pointer to the variable to receive the returned value.
*     STATUS = int * (Given and Returned)
*        The inherited status.
*     
*/

   const char *tp;
   char mess[81];

/* Check the inherited status. */
   if( *STATUS != SAI__OK ) return;

/* Get a pointer to the Tcl variable value string. */
   tp = GetVar( interp, (char *) var, TCL_LEAVE_ERR_MSG, STATUS );

/* If ok, extract an integer value from it. Report an error if the
   conversion fails.  */
   if ( tp ) {
      if( sscanf( tp, "%d", valptr ) != 1 ) {
         *STATUS = SAI__ERROR;
         sprintf( mess, "\"%s\" is not an integer value.", tp );
         Error( mess, STATUS );
         sprintf( mess, "Failed to obtained a value for Tcl variable \"%s\".", var );
         Error( mess, STATUS );
      }
   }
}


static void sink( const char *text ){
/* Sink function for use with astChannel. */
   if( fd && text ) fprintf( fd, "%s\n", text );
}

