/*
*class++
*  Name:
*     Mapping

*  Purpose:
*     Inter-relate two coordinate systems.

*  Constructor Function:
*     None.

*  Description:
*     This class provides the basic facilities for transforming a set
*     of coordinates (representing "input" points) to give a new set
*     of coordinates (representing "output" points).  It is used to
*     describe the relationship which exists between two different
*     coordinate systems.  However, the Mapping class does not have a
*     constructor function of its own, as it is simply a container
*     class for a family of specialised Mappings which implement
*     particular types of coordinate transformation.

*  Inheritance:
*     The Mapping class inherits from the Object class.

*  Attributes:
*     In addition to those attributes common to all Objects, every
*     Mapping also has the following attributes:
*
*     - Invert: Mapping inversion flag
*     - Nin: Number of input coordinates for a Mapping
*     - Nout: Number of output coordinates for a Mapping
*     - Report: Report transformed coordinates?
*     - TranForward: Forward transformation defined?
*     - TranInverse: Inverse transformation defined?

*  Functions:
c     In addition to those functions applicable to all Objects, the
c     following functions may also be applied to all Mappings:
f     In addition to those routines applicable to all Objects, the
f     following routines may also be applied to all Mappings:
*
c     - astInvert: Invert a Mapping
c     - astMapBox: Find a bounding box for a Mapping
c     - astResample<X>: Resample a region of a data grid
c     - astSimplify: Simplify a Mapping
c     - astTran1: Transform 1-dimensional coordinates
c     - astTran2: Transform 2-dimensional coordinates
c     - astTranN: Transform N-dimensional coordinates
c     - astTranP: Transform N-dimensional coordinates held in separate arrays
f     - AST_INVERT: Invert a Mapping
f     - AST_MAPBOX: Find a bounding box for a Mapping
f     - AST_RESAMPLE<X>: Resample a region of a data grid
f     - AST_SIMPLIFY: Simplify a Mapping
f     - AST_TRAN1: Transform 1-dimensional coordinates
f     - AST_TRAN2: Transform 2-dimensional coordinates
f     - AST_TRANN: Transform N-dimensional coordinates

*  Copyright:
*     <COPYRIGHT_STATEMENT>

*  Authors:
*     RFWS: R.F. Warren-Smith (Starlink)

*  History:
*     1-FEB-1996 (RFWS):
*        Original version.
*     29-FEB-1996 (RFWS):
*        Minor improvements to error messages.
*     15-JUL-1996 (RFWS):
*        Support external interface.
*     13-DEC-1996 (RFWS):
*        Added the astMapMerge method.
*     13-DEC-1996 (RFWS):
*        Added the astSimplify method.
*     27-MAY-1997 (RFWS):
*        Improved the astSimplify method to use astMapMerge to
*        simplify a single Mapping where possible.
*     29-MAY-1998 (RFWS):
*        Added the MapBox method.
*     13-NOV-1998 (RFWS):
*        Made default MapBox convergence accuracy larger (i.e. less
*        accurate).
*     10-DEC-1998 (RFWS):
*        First useful implementation of astResample<X>.
*class--
*/

/* Module Macros. */
/* ============== */
/* Set the name of the class we are implementing. This indicates to the header
   files that define class interfaces that they should make "protected"
   symbols available. */
#define astCLASS Mapping

/* Include files. */
/* ============== */
/* Interface definitions. */
/* ---------------------- */
#include "error.h"               /* Error reporting facilities */
#include "memory.h"              /* Memory allocation facilities */
#include "object.h"              /* Base Object class */
#include "pointset.h"            /* Sets of points/coordinates */
#include "channel.h"             /* I/O channels */
#include "mapping.h"             /* Interface definition for this class */

/* Error code definitions. */
/* ----------------------- */
#include "ast_err.h"             /* AST error codes */

/* C header files. */
/* --------------- */
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Module type definitions. */
/* ======================== */
/* Enum to represent the data type when resampling a grid of data. */
typedef enum DataType {
#if defined(AST_LONG_DOUBLE)     /* Not normally implemented */
   TYPE_LD,
#endif
   TYPE_D,
   TYPE_F,
   TYPE_L,
   TYPE_UL,
   TYPE_I,
   TYPE_UI,
   TYPE_S,
   TYPE_US,
   TYPE_B,
   TYPE_UB
} DataType;

/* Data structure to hold information about a Mapping for use by
   optimisation algorithms. */
typedef struct MapData {
   AstMapping *mapping;          /* Pointer to the Mapping */
   AstPointSet *pset_in;         /* Pointer to input PointSet */
   AstPointSet *pset_out;        /* Pointer to output PointSet */
   double *lbnd;                 /* Pointer to lower constraints on input */
   double *ubnd;                 /* Pointer to upper constraints on input */
   double **ptr_in;              /* Pointer to input PointSet coordinates */
   double **ptr_out;             /* Pointer to output PointSet coordinates */
   int coord;                    /* Index of output coordinate to optimise */
   int forward;                  /* Use forward transformation? */
   int negate;                   /* Negate the output value? */
   int nin;                      /* Number of input coordinates per point */
   int nout;                     /* Number of output coordinates per point */
} MapData;

/* Module Variables. */
/* ================= */
/* Define the class virtual function table and its initialisation flag as
   static variables. */
static AstMappingVtab class_vtab; /* Virtual function table */
static int class_init = 0;       /* Virtual function table initialised? */

/* Pointers to parent class methods which are extended by this class. */
static const char *(* parent_getattrib)( AstObject *, const char * );
static int (* parent_testattrib)( AstObject *, const char * );
static void (* parent_clearattrib)( AstObject *, const char * );
static void (* parent_setattrib)( AstObject *, const char * );

/* Prototypes for private member functions. */
/* ======================================== */
#if defined(AST_LONG_DOUBLE)     /* Not normally implemented */
static int InterpolateKernel1LD( AstMapping *, int, const int *, const int *, const long double *, const long double *, int, const int *, const double *const *, void (*)( double, const double *, int, double * ), int, const double *, int, long double, long double *, long double * );
static int InterpolateLinearLD( int, const int *, const int *, const long double *, const long double *, int, const int *, const double *const *, int, long double, long double *, long double * );
static int InterpolateNearestLD( int, const int *, const int *, const long double *, const long double *, int, const int *, const double *const *, int, long double, long double *, long double * );
static int ResampleLD( AstMapping *, int, const int [], const int [], const long double [], const long double [], int, void (*)(), const double [], int, double, int, long double, int, const int [], const int [], const int [], const int [], long double [], long double [] );
#endif

static AstMapping *Simplify( AstMapping * );
static AstPointSet *Transform( AstMapping *, AstPointSet *, int, AstPointSet * );
static const char *GetAttrib( AstObject *, const char * );
static double *LinearApprox( AstMapping *, int, int, const int *, const int *, double );
static double LocalMaximum( const MapData *, double, double, double [] );
static double MapFunction( const MapData *, const double [], int * );
static double NewVertex( const MapData *, int, double, double [], double [], int *, double [] );
static double Random( long int * );
static double UphillSimplex( const MapData *, double, int, const double [], double [], double *, int * );
static int GetInvert( AstMapping * );
static int GetNin( AstMapping * );
static int GetNout( AstMapping * );
static int GetReport( AstMapping * );
static int GetTranForward( AstMapping * );
static int GetTranInverse( AstMapping * );
static int InterpolateKernel1B( AstMapping *, int, const int *, const int *, const signed char *, const signed char *, int, const int *, const double *const *, void (*)( double, const double *, int, double * ), int, const double *, int, signed char, signed char *, signed char * );
static int InterpolateKernel1D( AstMapping *, int, const int *, const int *, const double *, const double *, int, const int *, const double *const *, void (*)( double, const double *, int, double * ), int, const double *, int, double, double *, double * );
static int InterpolateKernel1F( AstMapping *, int, const int *, const int *, const float *, const float *, int, const int *, const double *const *, void (*)( double, const double *, int, double * ), int, const double *, int, float, float *, float * );
static int InterpolateKernel1I( AstMapping *, int, const int *, const int *, const int *, const int *, int, const int *, const double *const *, void (*)( double, const double *, int, double * ), int, const double *, int, int, int *, int * );
static int InterpolateKernel1L( AstMapping *, int, const int *, const int *, const long int *, const long int *, int, const int *, const double *const *, void (*)( double, const double *, int, double * ), int, const double *, int, long int, long int *, long int * );
static int InterpolateKernel1S( AstMapping *, int, const int *, const int *, const short int *, const short int *, int, const int *, const double *const *, void (*)( double, const double *, int, double * ), int, const double *, int, short int, short int *, short int * );
static int InterpolateKernel1UB( AstMapping *, int, const int *, const int *, const unsigned char *, const unsigned char *, int, const int *, const double *const *, void (*)( double, const double *, int, double * ), int, const double *, int, unsigned char, unsigned char *, unsigned char * );
static int InterpolateKernel1UI( AstMapping *, int, const int *, const int *, const unsigned int *, const unsigned int *, int, const int *, const double *const *, void (*)( double, const double *, int, double * ), int, const double *, int, unsigned int, unsigned int *, unsigned int * );
static int InterpolateKernel1UL( AstMapping *, int, const int *, const int *, const unsigned long int *, const unsigned long int *, int, const int *, const double *const *, void (*)( double, const double *, int, double * ), int, const double *, int, unsigned long int, unsigned long int *, unsigned long int * );
static int InterpolateKernel1US( AstMapping *, int, const int *, const int *, const unsigned short int *, const unsigned short int *, int, const int *, const double *const *, void (*)( double, const double *, int, double * ), int, const double *, int, unsigned short int, unsigned short int *, unsigned short int * );
static int InterpolateLinearB( int, const int *, const int *, const signed char *, const signed char *, int, const int *, const double *const *, int, signed char, signed char *, signed char * );
static int InterpolateLinearD( int, const int *, const int *, const double *, const double *, int, const int *, const double *const *, int, double, double *, double * );
static int InterpolateLinearF( int, const int *, const int *, const float *, const float *, int, const int *, const double *const *, int, float, float *, float * );
static int InterpolateLinearI( int, const int *, const int *, const int *, const int *, int, const int *, const double *const *, int, int, int *, int * );
static int InterpolateLinearL( int, const int *, const int *, const long int *, const long int *, int, const int *, const double *const *, int, long int, long int *, long int * );
static int InterpolateLinearS( int, const int *, const int *, const short int *, const short int *, int, const int *, const double *const *, int, short int, short int *, short int * );
static int InterpolateLinearUB( int, const int *, const int *, const unsigned char *, const unsigned char *, int, const int *, const double *const *, int, unsigned char, unsigned char *, unsigned char * );
static int InterpolateLinearUI( int, const int *, const int *, const unsigned int *, const unsigned int *, int, const int *, const double *const *, int, unsigned int, unsigned int *, unsigned int * );
static int InterpolateLinearUL( int, const int *, const int *, const unsigned long int *, const unsigned long int *, int, const int *, const double *const *, int, unsigned long int, unsigned long int *, unsigned long int * );
static int InterpolateLinearUS( int, const int *, const int *, const unsigned short int *, const unsigned short int *, int, const int *, const double *const *, int, unsigned short int, unsigned short int *, unsigned short int * );
static int InterpolateNearestB( int, const int *, const int *, const signed char *, const signed char *, int, const int *, const double *const *, int, signed char, signed char *, signed char * );
static int InterpolateNearestD( int, const int *, const int *, const double *, const double *, int, const int *, const double *const *, int, double, double *, double * );
static int InterpolateNearestF( int, const int *, const int *, const float *, const float *, int, const int *, const double *const *, int, float, float *, float * );
static int InterpolateNearestI( int, const int *, const int *, const int *, const int *, int, const int *, const double *const *, int, int, int *, int * );
static int InterpolateNearestL( int, const int *, const int *, const long int *, const long int *, int, const int *, const double *const *, int, long int, long int *, long int * );
static int InterpolateNearestS( int, const int *, const int *, const short int *, const short int *, int, const int *, const double *const *, int, short int, short int *, short int * );
static int InterpolateNearestUB( int, const int *, const int *, const unsigned char *, const unsigned char *, int, const int *, const double *const *, int, unsigned char, unsigned char *, unsigned char * );
static int InterpolateNearestUI( int, const int *, const int *, const unsigned int *, const unsigned int *, int, const int *, const double *const *, int, unsigned int, unsigned int *, unsigned int * );
static int InterpolateNearestUL( int, const int *, const int *, const unsigned long int *, const unsigned long int *, int, const int *, const double *const *, int, unsigned long int, unsigned long int *, unsigned long int * );
static int InterpolateNearestUS( int, const int *, const int *, const unsigned short int *, const unsigned short int *, int, const int *, const double *const *, int, unsigned short int, unsigned short int *, unsigned short int * );
static int MapMerge( AstMapping *, int, int, int *, AstMapping ***, int ** );
static int MaxI( int, int );
static int MinI( int, int );
static int ResampleAdaptively( AstMapping *, int, const int *, const int *, const void *, const void *, DataType, int, int (*)(), const double *, int, double, int, const void *, int, const int *, const int *, const int *, const int *, void *, void * );
static int ResampleB( AstMapping *, int, const int [], const int [], const signed char [], const signed char [], int, void (*)(), const double [], int, double, int, signed char, int, const int [], const int [], const int [], const int [], signed char [], signed char [] );
static int ResampleD( AstMapping *, int, const int [], const int [], const double [], const double [], int, void (*)(), const double [], int, double, int, double, int, const int [], const int [], const int [], const int [], double [], double [] );
static int ResampleF( AstMapping *, int, const int [], const int [], const float [], const float [], int, void (*)(), const double [], int, double, int, float, int, const int [], const int [], const int [], const int [], float [], float [] );
static int ResampleI( AstMapping *, int, const int [], const int [], const int [], const int [], int, void (*)(), const double [], int, double, int, int, int, const int [], const int [], const int [], const int [], int [], int [] );
static int ResampleL( AstMapping *, int, const int [], const int [], const long int [], const long int [], int, void (*)(), const double [], int, double, int, long int, int, const int [], const int [], const int [], const int [], long int [], long int [] );
static int ResampleS( AstMapping *, int, const int [], const int [], const short int [], const short int [], int, void (*)(), const double [], int, double, int, short int, int, const int [], const int [], const int [], const int [], short int [], short int [] );
static int ResampleSection( AstMapping *, const double *, int, const int *, const int *, const void *, const void *, DataType, int, int (*)(), const double *, int, const void *, int, const int *, const int *, const int *, const int *, void *, void * );
static int ResampleUB( AstMapping *, int, const int [], const int [], const unsigned char [], const unsigned char [], int, void (*)(), const double [], int, double, int, unsigned char, int, const int [], const int [], const int [], const int [], unsigned char [], unsigned char [] );
static int ResampleUI( AstMapping *, int, const int [], const int [], const unsigned int [], const unsigned int [], int, void (*)(), const double [], int, double, int, unsigned int, int, const int [], const int [], const int [], const int [], unsigned int [], unsigned int [] );
static int ResampleUL( AstMapping *, int, const int [], const int [], const unsigned long int [], const unsigned long int [], int, void (*)(), const double [], int, double, int, unsigned long int, int, const int [], const int [], const int [], const int [], unsigned long int [], unsigned long int [] );
static int ResampleUS( AstMapping *, int, const int [], const int [], const unsigned short int [], const unsigned short int [], int, void (*)(), const double [], int, double, int, unsigned short int, int, const int [], const int [], const int [], const int [], unsigned short int [], unsigned short int [] );
static int ResampleWithBlocking( AstMapping *, const double *, int, const int *, const int *, const void *, const void *, DataType, int, int (*)(), const double *, int, const void *, int, const int *, const int *, const int *, const int *, void *, void * );
static int TestAttrib( AstObject *, const char * );
static int TestInvert( AstMapping * );
static int TestReport( AstMapping * );
static void ClearAttrib( AstObject *, const char * );
static void ClearInvert( AstMapping * );
static void ClearReport( AstMapping * );
static void Copy( const AstObject *, AstObject * );
static void Delete( AstObject * );
static void Dump( AstObject *, AstChannel * );
static void GlobalBounds( MapData *, double *, double *, double [], double [] );
static void InitVtab( AstMappingVtab * );
static void Invert( AstMapping * );
static void MapBox( AstMapping *, const double [], const double [], int, int, double *, double *, double [], double [] );
static void MapList( AstMapping *, int, int, int *, AstMapping ***, int ** );
static void ReportPoints( AstMapping *, int, AstPointSet *, AstPointSet * );
static void SetAttrib( AstObject *, const char * );
static void SetInvert( AstMapping *, int );
static void SetReport( AstMapping *, int );
static void Sinc( double, const double [], int, double * );
static void SincSinc( double, const double [], int, double * );
static void SpecialBounds( const MapData *, double *, double *, double [], double [] );
static void Tran1( AstMapping *, int, const double [], int, double [] );
static void Tran2( AstMapping *, int, const double [], const double [], int, double [], double [] );
static void TranN( AstMapping *, int, int, int, const double (*)[], int, int, int, double (*)[] );
static void TranP( AstMapping *, int, int, const double *[], int, int, double *[] );
static void ValidateMapping( AstMapping *, int, int, int, int, const char * );

/* Member functions. */
/* ================= */
static void ClearAttrib( AstObject *this_object, const char *attrib ) {
/*
*  Name:
*     ClearAttrib

*  Purpose:
*     Clear an attribute value for a Mapping.

*  Type:
*     Private function.

*  Synopsis:
*     #include "mapping.h"
*     void ClearAttrib( AstObject *this, const char *attrib )

*  Class Membership:
*     Mapping member function (over-rides the astClearAttrib protected
*     method inherited from the Object class).

*  Description:
*     This function clears the value of a specified attribute for a
*     Mapping, so that the default value will subsequently be used.

*  Parameters:
*     this
*        Pointer to the Mapping.
*     attrib
*        Pointer to a null terminated string specifying the attribute
*        name.  This should be in lower case with no surrounding white
*        space.
*/

/* Local Variables: */
   AstMapping *this;             /* Pointer to the Mapping structure */

/* Check the global error status. */
   if ( !astOK ) return;

/* Obtain a pointer to the Mapping structure. */
   this = (AstMapping *) this_object;

/* Check the attribute name and clear the appropriate attribute. */

/* Invert. */
/* ------- */
   if ( !strcmp( attrib, "invert" ) ) {
      astClearInvert( this );

/* Report. */
/* ------- */
   } else if ( !strcmp( attrib, "report" ) ) {
      astClearReport( this );

/* If the name was not recognised, test if it matches any of the
   read-only attributes of this class. If it does, then report an
   error. */
   } else if ( !strcmp( attrib, "nin" ) ||
        !strcmp( attrib, "nout" ) ||
        !strcmp( attrib, "tranforward" ) ||
        !strcmp( attrib, "traninverse" ) ) {
      astError( AST__NOWRT, "astClear: Invalid attempt to clear the \"%s\" "
                "value for a %s.", attrib, astGetClass( this ) );
      astError( AST__NOWRT, "This is a read-only attribute." );

/* If the attribute is still not recognised, pass it on to the parent
   method for further interpretation. */
   } else {
      (*parent_clearattrib)( this_object, attrib );
   }
}

static const char *GetAttrib( AstObject *this_object, const char *attrib ) {
/*
*  Name:
*     GetAttrib

*  Purpose:
*     Get the value of a specified attribute for a Mapping.

*  Type:
*     Private function.

*  Synopsis:
*     #include "mapping.h"
*     const char *GetAttrib( AstObject *this, const char *attrib )

*  Class Membership:
*     Mapping member function (over-rides the protected astGetAttrib
*     method inherited from the Object class).

*  Description:
*     This function returns a pointer to the value of a specified
*     attribute for a Mapping, formatted as a character string.

*  Parameters:
*     this
*        Pointer to the Mapping.
*     attrib
*        Pointer to a null terminated string containing the name of
*        the attribute whose value is required. This name should be in
*        lower case, with all white space removed.

*  Returned Value:
*     Pointer to a null terminated string containing the attribute
*     value.

*  Notes:
*     - The returned string pointer may point at memory allocated
*     within the Mapping, or at static memory. The contents of the
*     string may be over-written or the pointer may become invalid
*     following a further invocation of the same function or any
*     modification of the Mapping. A copy of the string should
*     therefore be made if necessary.
*     - A NULL pointer will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*/

/* Local Constants: */
#define BUFF_LEN 50              /* Max. characters in result buffer */

/* Local Variables: */
   AstMapping *this;             /* Pointer to the Mapping structure */
   const char *result;           /* Pointer value to return */
   int invert;                   /* Invert attribute value */
   int nin;                      /* Nin attribute value */
   int nout;                     /* Nout attribute value */
   int report;                   /* Report attribute value */
   int tran_forward;             /* TranForward attribute value */
   int tran_inverse;             /* TranInverse attribute value */
   static char buff[ BUFF_LEN + 1 ]; /* Buffer for string result */

/* Initialise. */
   result = NULL;

/* Check the global error status. */   
   if ( !astOK ) return result;

/* Obtain a pointer to the Mapping structure. */
   this = (AstMapping *) this_object;

/* Compare "attrib" with each recognised attribute name in turn,
   obtaining the value of the required attribute. If necessary, write
   the value into "buff" as a null terminated string in an appropriate
   format.  Set "result" to point at the result string. */

/* Invert. */
/* ------- */
   if ( !strcmp( attrib, "invert" ) ) {
      invert = astGetInvert( this );
      if ( astOK ) {
         (void) sprintf( buff, "%d", invert );
         result = buff;
      }

/* Nin. */
/* ---- */
   } else if ( !strcmp( attrib, "nin" ) ) {
      nin = astGetNin( this );
      if ( astOK ) {
         (void) sprintf( buff, "%d", nin );
         result = buff;
      }

/* Nout. */
/* ----- */
   } else if ( !strcmp( attrib, "nout" ) ) {
      nout = astGetNout( this );
      if ( astOK ) {
         (void) sprintf( buff, "%d", nout );
         result = buff;
      }

/* Report. */
/* ------- */
   } else if ( !strcmp( attrib, "report" ) ) {
      report = astGetReport( this );
      if ( astOK ) {
         (void) sprintf( buff, "%d", report );
         result = buff;
      }

/* TranForward. */
/* ------------ */
   } else if ( !strcmp( attrib, "tranforward" ) ) {
      tran_forward = astGetTranForward( this );
      if ( astOK ) {
         (void) sprintf( buff, "%d", tran_forward );
         result = buff;
      }

/* TranInverse. */
/* ------------ */
   } else if ( !strcmp( attrib, "traninverse" ) ) {
      tran_inverse = astGetTranInverse( this );
      if ( astOK ) {
         (void) sprintf( buff, "%d", tran_inverse );
         result = buff;
      }

/* If the attribute name was not recognised, pass it on to the parent
   method for further interpretation. */
   } else {
      result = (*parent_getattrib)( this_object, attrib );
   }

/* Return the result. */
   return result;

/* Undefine macros local to this function. */
#undef BUFF_LEN
}

static int GetNin( AstMapping *this ) {
/*
*+
*  Name:
*     astGetNin

*  Purpose:
*     Get the number of input coordinates for a Mapping.

*  Type:
*     Protected virtual function.

*  Synopsis:
*     #include "mapping.h"
*     int astGetNin( AstMapping *this )

*  Class Membership:
*     Mapping method.

*  Description:
*     This function returns the number of input coordinate values
*     required per point by a Mapping (i.e. the number of dimensions
*     of the space in which input points reside).

*  Parameters:
*     this
*        Pointer to the Mapping.

*  Returned Value:
*     Number of coordinate values required.

*  Notes:
*     - A value of zero will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*-
*/

/* Local Variables: */
   int invert;                   /* Invert attribute value */
   int result;                   /* Result value to return */

/* Initialise. */
   result = 0;

/* Check the global error status. */
   if ( !astOK ) return result;

/* Determine if the Mapping has been inverted. */
   invert = astGetInvert( this );

/* Obtain the Nin value. */
   if ( astOK ) result = invert ? this->nout : this->nin;

/* Return the result. */
   return result;
}

static int GetNout( AstMapping *this ) {
/*
*+
*  Name:
*     astGetNout

*  Purpose:
*     Get the number of output coordinates for a Mapping.

*  Type:
*     Protected virtual function.

*  Synopsis:
*     #include "mapping.h"
*     int astGetNout( AstMapping *this )

*  Class Membership:
*     Mapping method.

*  Description:
*     This function returns the number of output coordinate values
*     generated per point by a Mapping (i.e. the number of dimensions
*     of the space in which output points reside).

*  Parameters:
*     this
*        Pointer to the Mapping.

*  Returned Value:
*     Number of coordinate values generated.

*  Notes:
*     - A value of zero will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*-
*/

/* Local Variables: */
   int invert;                   /* Invert attribute value */
   int result;                   /* Result value to return */

/* Initialise. */
   result = 0;

/* Check the global error status. */
   if ( !astOK ) return result;

/* Determine if the Mapping has been inverted. */
   invert = astGetInvert( this );

/* Obtain the Nout value. */
   if ( astOK ) result = invert ? this->nin : this->nout;

/* Return the result. */
   return result;
}

static int GetTranForward( AstMapping *this ) {
/*
*+
*  Name:
*     astGetTranForward

*  Purpose:
*     Determine if a Mapping defines a forward coordinate transformation.

*  Type:
*     Protected virtual function.

*  Synopsis:
*     #include "mapping.h"
*     int astGetTranForward( AstMapping *this )

*  Class Membership:
*     Mapping method.

*  Description:
*     This function returns a value indicating whether a Mapping is
*     able to perform a coordinate transformation in the "forward"
*     direction.

*  Parameters:
*     this
*        Pointer to the Mapping.

*  Returned Value:
*     Zero if the forward coordinate transformation is not defined, or
*     1 if it is.

*  Notes:
*     - A value of zero will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*-
*/

/* Local Variables: */
   int invert;                   /* Mapping inverted? */
   int result;                   /* Result value to return */

/* Initialise. */
   result = 0;

/* Check the global error status. */
   if ( !astOK ) return result;

/* Determine if the Mapping has been inverted. */
   invert = astGetInvert( this );

/* If OK, obtain the result. */
   if ( astOK ) result = invert ? this->tran_inverse : this->tran_forward;

/* Return the result. */
   return result;
}

static int GetTranInverse( AstMapping *this ) {
/*
*+
*  Name:
*     astGetTranInverse

*  Purpose:
*     Determine if a Mapping defines an inverse coordinate transformation.

*  Type:
*     Protected virtual function.

*  Synopsis:
*     #include "mapping.h"
*     int astGetTranInverse( AstMapping *this )

*  Class Membership:
*     Mapping method.

*  Description:
*     This function returns a value indicating whether a Mapping is
*     able to perform a coordinate transformation in the "inverse"
*     direction.

*  Parameters:
*     this
*        Pointer to the Mapping.

*  Returned Value:
*     Zero if the inverse coordinate transformation is not defined, or
*     1 if it is.

*  Notes:
*     - A value of zero will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*-
*/

/* Local Variables: */
   int invert;                   /* Mapping inverted? */
   int result;                   /* Result value to return */

/* Initialise. */
   result = 0;

/* Check the global error status. */
   if ( !astOK ) return result;

/* Determine if the Mapping has been inverted. */
   invert = astGetInvert( this );

/* If OK, obtain the result. */
   if ( astOK ) result = invert ? this->tran_forward : this->tran_inverse;

/* Return the result. */
   return result;
}

static void GlobalBounds( MapData *mapdata, double *lbnd, double *ubnd,
                          double xl[], double xu[] ) {
/*
*  Name:
*     GlobalBounds

*  Purpose:
*     Estimate global coordinate bounds for a Mapping.

*  Type:
*     Private function.

*  Synopsis:
*     #include "mapping.h"
*     void GlobalBounds( MapData *mapdata, double *lbnd, double *ubnd,
*                        double xl[], double xu[] );

*  Class Membership:
*     Mapping member function.

*  Description:
*     This function estimates the global lower and upper bounds of a
*     Mapping function within a constrained region of its input
*     coordinate space. It uses a robust global optimisation algorithm
*     based on the selection of pseudo-random starting positions,
*     followed by the location of local minima and maxima using the
*     downhill (or uphill) simplex method. The algorithm will cope
*     with the case where there are several competing minima (or
*     maxima) with nearly equal values. It attempts to locate the
*     global bounds to full machine precision when possible.

*  Parameters:
*     mapdata
*        Pointer to a MapData structure describing the Mapping
*        function, its coordinate constraints, etc.
*     lbnd
*        Pointer to a double.  On entry, this should contain a
*        previously-obtained upper limit on the global lower bound, or
*        AST__BAD if no such limit is available. On exit, it will be
*        updated with a new estimate of the global lower bound, if a
*        better one has been found.
*     ubnd
*        Pointer to a double.  On entry, this should contain a
*        previously-obtained lower limit on the global upper bound, or
*        AST__BAD if no such limit is available. On exit, it will be
*        updated with a new estimate of the global upper bound, if a
*        better one has been found.
*     xl
*        Pointer to an array of double, with one element for each
*        input coordinate. On entry, if *lbnd is not equal to AST__OK,
*        this should contain the input coordinates of a point at which
*        the Mapping function takes the value *lbnd. On exit, this
*        function returns the position of a (not necessarily unique)
*        input point at which the Mapping function takes the value of
*        the new global lower bound.  This array is not altered if an
*        improved estimate of the global lower bound cannot be found.
*     xu
*        Pointer to an array of double, with one element for each
*        input coordinate. On entry, if *ubnd is not equal to AST__OK,
*        this should contain the input coordinates of a point at which
*        the Mapping function takes the value *ubnd. On exit, this
*        function returns the position of a (not necessarily unique)
*        input point at which the Mapping function takes the value of
*        the new global upper bound.  This array is not altered if an
*        improved estimate of the global upper bound cannot be found.

*  Notes:
*     - The efficiency of this function will usually be improved if
*     previously-obtained estimates of the extrema and their locations
*     are provided.
*     - The values returned via "lbnd", "ubnd", "xl" and "xu" will be
*     set to the value AST__BAD if this function should fail for any
*     reason. Their initial values on entry will not be altered if the
*     function is invoked with the global error status set.
*/

/* Local Constants: */
   const double default_acc = 3.0e-5; /* Default convergence accuracy */
   const int maxiter = 10000;    /* Maximum number of iterations */
   const int minsame = 3;        /* Minimum no. consistent extrema required */
   const int nbatch = 32;        /* No. function samples obtained per batch */

/* Local Variables: */
   AstPointSet *pset_in;         /* Input PointSet for batch transformation */
   AstPointSet *pset_out;        /* Output PointSet for batch transformation */
   double **ptr_in;              /* Pointer to batch input coordinates */
   double **ptr_out;             /* Pointer to batch output coordinates */
   double *active_hi;            /* Estimated upper limits of active region */
   double *active_lo;            /* Estimated lower limits of active region */
   double *sample_hi;            /* Upper limits of sampled region */
   double *sample_lo;            /* Lower limits of sampled region */
   double *sample_width;         /* Nominal widths of sampled region */
   double *x;                    /* Pointer to array of coordinates */
   double acc;                   /* Convergence accuracy for finding maximum */
   double active_width;          /* Estimated width of active region */
   double new_max;               /* Value of new local maximum */
   double new_min;               /* Value of new local minimum */
   double oversize;              /* Over-size factor for sampled region */
   double random;                /* Pseudo-random number */
   int bad;                      /* Transformed position is bad? */
   int batch;                    /* Next element to use in position batch */
   int coord;                    /* Loop counter for coordinates */
   int done_max;                 /* Satisfactory global maximum found? */
   int done_min;                 /* Satisfactory global minimum found? */
   int iter;                     /* Loop counter for iterations */
   int ncall;                    /* Number of Mapping function calls (junk) */
   int ncoord;                   /* Number of coordinates in search space */
   int nmax;                     /* Number of local maxima found */
   int nmin;                     /* Number of local minima found */
   int nsame_max;                /* Number of equivalent local maxima found */
   int nsame_min;                /* Number of equivalent local minima found */
   long int seed = 1776655449;   /* Arbitrary pseudo-random number seed */

/* Check the global error status */
   if ( !astOK ) return;

/* Initialise. */
   done_max = 0;
   done_min = 0;
   ncall = 0;
   nmax = 0;
   nmin = 0;
   nsame_max = 0;
   nsame_min = 0;

/* Extract the number of input coordinates for the Mapping function
   and allocate workspace. */
   ncoord = mapdata->nin;
   active_hi = astMalloc( sizeof( double ) * (size_t) ncoord );
   active_lo = astMalloc( sizeof( double ) * (size_t) ncoord );
   sample_hi = astMalloc( sizeof( double ) * (size_t) ncoord );
   sample_lo = astMalloc( sizeof( double ) * (size_t) ncoord );
   sample_width = astMalloc( sizeof( double ) * (size_t) ncoord );
   x = astMalloc( sizeof( double ) * (size_t) ncoord );
   if ( astOK ) {

/* Calculate the factor by which the size of the region we sample will
   exceed the size of the Mapping function's active region (the region
   where the transformed coordinates are non-bad) in each
   dimension. This is chosen so that the volume ratio will be 2. */
      oversize = pow( 2.0, 1.0 / ( (double) ncoord ) );

/* Initialise the limits iof the active region to unknown. */
      for ( coord = 0; coord < ncoord; coord++ ) {
         active_lo[ coord ] = DBL_MAX;;
         active_hi[ coord ] = -DBL_MAX;

/* Initialise the nominal widths of the sampled region to be the
   actual widths of the search region times the over-size factor. */
         sample_width[ coord ] = ( mapdata->ubnd[ coord ] -
                                   mapdata->lbnd[ coord ] ) * oversize;

/* Initialise the sampled region to match the search region. */
         sample_lo[ coord ] = mapdata->lbnd[ coord ];
         sample_hi[ coord ] = mapdata->ubnd[ coord ];
      }

/* Set up position buffer. */
/* ======================= */
/* Create two PointSets to act as buffers to hold a complete batch of
   input and output coordinates. Obtain pointers to their coordinate
   arrays. */
      pset_in = astPointSet( nbatch, ncoord, "" );
      pset_out = astPointSet( nbatch, mapdata->nout, "" );
      ptr_in = astGetPoints( pset_in );
      ptr_out = astGetPoints( pset_out );

/* Initialise the next element to be used in the position buffer to
   indicate that the buffer is initially empty. */
      batch = nbatch;
   }

/* Define a macro to fill the position buffer with a set of
   pseudo-random positions and to transform them. */
#define FILL_POSITION_BUFFER {\
\
/* We first generate a suitable volume over which to distribute the\
   batch of pseudo-random positions. Initially, this will be the\
   entire search volume, but if we find that the only non-bad\
   transformed coordinates we obtain are restricted to a small\
   sub-region of this input volume, then we reduce the sampled volume\
   so as to concentrate more on the active region. */\
\
/* Loop through each input coordinate, checking that at least one\
   non-bad transformed point has been obtained. If not, we do not\
   adjust the sampled volume, as we do not yet know where the active\
   region lies. */\
   for ( coord = 0; coord < ncoord; coord++ ) {\
      if ( active_hi[ coord ] >= active_lo[ coord ] ) {\
\
/* Estimate the width of the active region from the range of input\
   coordinates that have so far produced non-bad transformed\
   coordinates. */\
         active_width = active_hi[ coord ] - active_lo[ coord ];\
\
/* If the current width of the sampled volume exceeds this estimate by\
   more than the required factor, then reduce the width of the sampled\
   volume. The rate of reduction is set so that the volume of the\
   sampled region can halve with every fourth batch of positions. */\
         if ( ( active_width * oversize ) < sample_width[ coord ] ) {\
            sample_width[ coord ] /= pow( oversize, 0.25 );\
\
/* If the width of the sampled volume does not exceed that of the\
   known active region by the required factor, then adjust it so that\
   it does. Note that we must continue to sample some points outside\
   the known active region in case we have missed any (in which case\
   the sampled region will expand again to include them). */\
         } else if ( ( active_width * oversize ) > sample_width[ coord ] ) {\
            sample_width[ coord ] = active_width * oversize;\
         }\
\
/* Calculate the lower and upper bounds on the sampled volume, using\
   the new width calculated above and centring it on the active\
   region, as currently known. */\
         sample_lo[ coord ] = ( active_lo[ coord ] + active_hi[ coord ] -\
                                sample_width[ coord ] ) * 0.5;\
         sample_hi[ coord ] = ( active_lo[ coord ] + active_hi[ coord ] +\
                                sample_width[ coord ] ) * 0.5;\
\
/* Ensure that the sampled region does not extend beyond the original\
   search region. */\
         if ( sample_lo[ coord ] < mapdata->lbnd[ coord ] ) {\
            sample_lo[ coord ] = mapdata->lbnd[ coord ];\
         }\
         if ( sample_hi[ coord ] > mapdata->ubnd[ coord ] ) {\
            sample_hi[ coord ] = mapdata->ubnd[ coord ];\
         }\
      }\
   }\
\
/* Having determined the size of the sampled volume, create a batch of\
   pseudo-random positions uniformly distributed within it. */\
   for ( batch = 0; batch < nbatch; batch++ ) {\
      for ( coord = 0; coord < ncoord; coord++ ) {\
         random = Random( &seed );\
         ptr_in[ coord ][ batch ] = sample_lo[ coord ] * random +\
                                    sample_hi[ coord ] * ( 1.0 - random );\
      }\
   }\
\
/* Transform these positions. We process them in a single batch in\
   order to minimise the overheads in doing this. */\
   (void) astTransform( mapdata->mapping, pset_in, mapdata->forward,\
                        pset_out );\
\
/* Indicate that the position buffer is now full. */\
   batch = 0;\
}

/* Fill the position buffer using the above macro. (Note that because
   we do not yet have an estimate of the size of the active region,
   this does not change the sampled region size from our earlier
   initialised values. */
   FILL_POSITION_BUFFER;

/* Iterate. */
/* ======== */
/* Loop to perform up to "maxiter" iterations to estimate the global
   minimum and maximum. */
   for ( iter = 0; astOK && ( iter < maxiter ); iter++ ) {

/* Determine the search accuracy. */
/* ============================== */
/* Decide the accuracy to which local extrema should be found. The
   intention here is to optimise performance, especially where one
   extremum lies near zero and so could potentially be found to
   unnecessarily high precision. If we make a mis-assumption (the code
   below is not fool-proof), we will slow things down for this
   iteration, but the error will be corrected in future iterations
   once better estimates are available. */

/* If we have no current estimate of either global extremum, we assume
   the values we eventually obtain will be of order unity and required
   to the default accuracy. */
      acc = default_acc;

/* If we already have an estimate of both global extrema, we set the
   accuracy level so that the difference between them will be known to
   the default accuracy. */
      if ( ( *lbnd != AST__BAD ) && ( *ubnd != AST__BAD ) ) {
         acc = fabs( *ubnd - *lbnd ) * default_acc;

/* If we have an estimate of only one global extremum, we assume that
   the difference between the two global extrema will eventually be of
   the same order as the estimate we currently have, so long as this
   is not less than unity. */
      } else if ( *lbnd != AST__BAD ) {
         if ( fabs( *lbnd ) > 1.0 ) acc = fabs( *lbnd) * default_acc;
      } else if ( *ubnd != AST__BAD ) {
         if ( fabs( *ubnd ) > 1.0 ) acc = fabs( *ubnd) * default_acc;
      }

/* Search for a new local minimum. */
/* =============================== */
/* If we are still searching for the global minimum, then obtain a set
   of starting coordinates from which to find a new local minimum. */
      if ( !done_min ) {

/* On the first iteration, start searching at the position where the
   best estimate of the global minimum (if any) has previously been
   found. We know that this produces non-bad transformed
   coordinates. */
         bad = 0;
         if ( !iter && ( *lbnd != AST__BAD ) ) {
            for ( coord = 0; coord < ncoord; coord++ ) {
               x[ coord ] = xl[ coord ];
            }

/* Otherwise, if no estimate of the global minimum is available, then
   start searching at the position where the best estimate of the
   global maximum (if any) has been found. This may be a long way from
   a local minimum, but at least it will yield a non-bad value for the
   Mapping function, so some sort of estimate of the global minimum
   will be obtained. This is important in cases where finding the
   active region of the function is the main problem. Note that this
   condition can only occur once, since the global minimum will have
   an estimate on the next iteration. */
         } else if ( ( *lbnd == AST__BAD ) && ( *ubnd != AST__BAD ) ) {
            for ( coord = 0; coord < ncoord; coord++ ) {
               x[ coord ] = xu[ coord ];
            }

/* Having exhausted the above possibilities, we use pseudo-random
   starting positions which are uniformly distributed throughout the
   search volume. First check to see if the buffer containing such
   positions is empty and refill it if necessary. */
         } else {
            if ( batch >= nbatch ) FILL_POSITION_BUFFER;

/* Test the next available set of output (transformed) coordinates in
   the position buffer to see if they are bad. */
            if ( astOK ) {
               for ( coord = 0; coord < mapdata->nout; coord++ ) {
                  bad = ( ptr_out[ coord ][ batch ] == AST__BAD );
                  if ( bad ) break;
               }

/* If not, we have a good starting position for finding a local
   minimum, so extract the corresponding input coordinates. */
               if ( !bad ) {
                  for ( coord = 0; coord < ncoord; coord++ ) {
                     x[ coord ] = ptr_in[ coord ][ batch ];
                  }
               }

/* Increment the position buffer location. */
               batch++;
            }
         }

/* If we do not have a good starting position, we can't do anything
   more on this iteration. A new position will be obtained and tested
   on the next iteration and this (we hope) will eventually identify a
   suitable starting point. */
         if ( astOK && !bad ) {

/* Form estimates of the lower and upper limits of the active region
   from the starting positions used. */
            for ( coord = 0; coord < ncoord; coord++ ) {
               if ( x[ coord ] < active_lo[ coord ] ) {
                  active_lo[ coord ] = x[ coord ];
               }
               if ( x[ coord ] > active_hi[ coord ] ) {
                  active_hi[ coord ] = x[ coord ];
               }
            }

/* Indicate that the Mapping function should be negated (because we
   want a local minimum) and then search for a local maximum in this
   negated function. If the result is non-bad (as it should always be,
   barring an error), then negate it to obtain the value of the local
   minimum found. */
            mapdata->negate = 1;
            new_min = LocalMaximum( mapdata, acc, 0.01, x );
            if ( new_min != AST__BAD ) {
               new_min = -new_min;

/* Update the estimates of the lower and upper bounds of the active
   region to take account of where the minimum was found. */
               for ( coord = 0; coord < ncoord; coord++ ) {
                  if ( x[ coord ] < active_lo[ coord ] ) {
                     active_lo[ coord ] = x[ coord ];
                  }
                  if ( x[ coord ] > active_hi[ coord ] ) {
                     active_hi[ coord ] = x[ coord ];
                  }
               }

/* Count the number of times we successfully locate a local minimum
   (ignoring the fact they might all be the same one). */
               nmin++;

/* Update the global minimum. */
/* ========================== */
/* If this is the first estimate of the global minimum, then set to
   one the count of the number of consecutive iterations where this
   estimate remains unchanged. Store the minimum value and its
   position. */
               if ( *lbnd == AST__BAD ) {
                  nsame_min = 1;
                  *lbnd = new_min;
                  for ( coord = 0; coord < ncoord; coord++ ) {
                     xl[ coord ] = x[ coord ];
                  }

/* Otherwise, test if this local minimum is lower than the previous
   estimate of the global minimum. If so, then reset the count of
   unchanged estimates of the global mimimum to one if the difference
   exceeds the accuracy with which the minimum was found (i.e. if we
   have found a significantly different minimum). Otherwise, just
   increment this count (because we have found the same minimum but by
   chance with slightly improved accuracy). Store the new minimum and
   its position. */
               } else if ( new_min < *lbnd ) {
                  nsame_min = ( ( *lbnd - new_min ) > acc ) ? 1 :
                                                              nsame_min + 1;
                  *lbnd = new_min;
                  for ( coord = 0; coord < ncoord; coord++ ) {
                     xl[ coord ] = x[ coord ];
                  }

/* If the latest local minimum is no improvement on previous estimates
   of the global minimum, then increment the count of unchanged
   estimates of the global mimimum, but do not save the new one. */
               } else {
                  nsame_min++;
               }

/* Determine if a satisfactory estimate of the global minimum has been
   obtained.  It has if the number of consecutive local minima which
   have not significantly improved the estimate is at least equal to
   "minsame", and at least 30% of the total number of local minima
   found. */
               if ( ( nsame_min >= minsame ) &&
                    ( nsame_min >= (int) ( 0.3f * (float) nmin + 0.5f ) ) ) {
                  done_min = 1;
               }
            }
         }
      }

/* Search for a new local maximum. */
/* =============================== */
/* Now repeat all of the above to find a new local maximum which
   estimates the global maximum. */
      if ( !done_max ) {

/* Choose a suitable starting position, based on one already available
   if appropriate. */
         if ( !iter && ( *ubnd != AST__BAD ) ) {
            for ( coord = 0; coord < ncoord; coord++ ) {
               x[ coord ] = xu[ coord ];
            }

         } else if ( ( *ubnd == AST__BAD ) && ( *lbnd != AST__BAD ) ) {
            for ( coord = 0; coord < ncoord; coord++ ) {
               x[ coord ] = xl[ coord ];
            }

/* Otherwise use a pseudo-random position, refilling the position
   buffer if necessary. Check if the transformed coordinates are
   bad. */
         } else {
            if ( batch >= nbatch ) FILL_POSITION_BUFFER;
            if ( astOK ) {
               for ( coord = 0; coord < mapdata->nout; coord++ ) {
                  bad = ( ptr_out[ coord ][ batch ] == AST__BAD );
                  if ( bad ) break;
               }
               if ( !bad ) {
                  for ( coord = 0; coord < ncoord; coord++ ) {
                     x[ coord ] = ptr_in[ coord ][ batch ];
                  }
               }
               batch++;
            }
         }

/* If the coordinates are OK, update the active region limits. */
         if ( astOK && !bad ) {
            for ( coord = 0; coord < ncoord; coord++ ) {
               if ( x[ coord ] < active_lo[ coord ] ) {
                  active_lo[ coord ] = x[ coord ];
               }
               if ( x[ coord ] > active_hi[ coord ] ) {
                  active_hi[ coord ] = x[ coord ];
               }
            }

/* Find a local maximum in the Mapping function. */
            mapdata->negate = 0;
            new_max = LocalMaximum( mapdata, acc, 0.01, x );
            if ( new_max != AST__BAD ) {

/* Use the result to further update the active region limits. */
               for ( coord = 0; coord < ncoord; coord++ ) {
                  if ( x[ coord ] < active_lo[ coord ] ) {
                     active_lo[ coord ] = x[ coord ];
                  }
                  if ( x[ coord ] > active_hi[ coord ] ) {
                     active_hi[ coord ] = x[ coord ];
                  }
               }

/* Count the number of local maxima found. */
               nmax++;

/* Update the estimate of the global maximum. */
               if ( *ubnd == AST__BAD ) {
                  nsame_max = 1;
                  *ubnd = new_max;
                  for ( coord = 0; coord < ncoord; coord++ ) {
                     xu[ coord ] = x[ coord ];
                  }

               } else if ( new_max > *ubnd ) {
                  nsame_max = ( ( new_max - *ubnd ) > acc ) ? 1 :
                                                              nsame_max + 1;
                  *ubnd = new_max;
                  for ( coord = 0; coord < ncoord; coord++ ) {
                     xu[ coord ] = x[ coord ];
                  }

               } else {
                  nsame_max++;
               }

/* Test for a satisfactory global maximum estimate. */
               if ( ( nsame_max >= minsame ) &&
                    ( nsame_max >= (int) ( 0.3f * (float) nmax + 0.5 ) ) ) {
                  done_max = 1;
               }
            }
         }
      }

/* Quit iterating once both the global minimum and the global maximum
   have been found. */
      if ( done_min && done_max ) break;
   }

/* Free workspace. */
   active_hi = astFree( active_hi );
   active_lo = astFree( active_lo );
   sample_hi = astFree( sample_hi );
   sample_lo = astFree( sample_lo );
   sample_width = astFree( sample_width );
   x = astFree( x );

/* Annul temporary PointSets. */
   pset_in = astAnnul( pset_in );
   pset_out = astAnnul( pset_out );

/* If the global minimum has been found, attempt to polish the result
   to machine precision by requesting that it be found with an
   accuracy tolerance of zero (subject to the maximum number of
   iterations that LocalMaximum will perform,). */
   if ( astOK ) {
      if ( *lbnd != AST__BAD ) {
         mapdata->negate = 1;
         *lbnd = LocalMaximum( mapdata, 0.0, sqrt( DBL_EPSILON ), xl );
         if ( *lbnd != AST__BAD ) *lbnd = - *lbnd;
      }

/* Similarly polish the estimate of the global maximum. */
      if ( *ubnd != AST__BAD ) {
         mapdata->negate = 0;
         *ubnd = LocalMaximum( mapdata, 0.0, sqrt( DBL_EPSILON ), xu );
      }

/* If either extremum could not be found, then report an error. */
      if ( ( *lbnd == AST__BAD ) || ( *ubnd == AST__BAD ) ) {
         astError( AST__MBBNF, "astMapBox(%s): No valid output coordinates "
                   "(after %d test points).", astGetClass( mapdata->mapping ),
                   2 * maxiter );
      }

/* If an error occurred, then return bad extremum values and
   coordinates. */
      if ( !astOK ) {
         *lbnd = AST__BAD;
         *ubnd = AST__BAD;
         for ( coord = 0; coord < ncoord; coord++ ) {
            xl[ coord ] = AST__BAD;
            xu[ coord ] = AST__BAD;
         }
      }
   }

/* Undefine macros local to this function. */
#undef FILL_POSITION_BUFFER
}

static void InitVtab( AstMappingVtab *vtab ) {
/*
*  Name:
*     InitVtab

*  Purpose:
*     Initialise a virtual function table for a Mapping.

*  Type:
*     Private function.

*  Synopsis:
*     #include "mapping.h"
*     void InitVtab( AstMappingVtab *vtab )

*  Class Membership:
*     Mapping member function.

*  Description:
*     This function initialises the component of a virtual function
*     table which is used by the Mapping class.

*  Parameters:
*     vtab
*        Pointer to the virtual function table. The components used by
*        all ancestral classes should already have been initialised.
*/

/* Local Variables: */
   AstObjectVtab *object;        /* Pointer to Object component of Vtab */

/* Check the local error status. */
   if ( !astOK ) return;

/* Store a unique "magic" value in the virtual function table. This
   will be used (by astIsAMapping) to determine if an object belongs
   to this class.  We can conveniently use the address of the (static)
   class_init variable to generate this unique value. */
   vtab->check = &class_init;

/* Initialise member function pointers. */
/* ------------------------------------ */
/* Store pointers to the member functions (implemented here) that provide
   virtual methods for this class. */
#if defined(AST_LONG_DOUBLE)     /* Not normally implemented */
   vtab->ResampleLD = ResampleLD;
#endif
   vtab->ClearInvert = ClearInvert;
   vtab->ClearReport = ClearReport;
   vtab->GetInvert = GetInvert;
   vtab->GetNin = GetNin;
   vtab->GetNout = GetNout;
   vtab->GetReport = GetReport;
   vtab->GetTranForward = GetTranForward;
   vtab->GetTranInverse = GetTranInverse;
   vtab->Invert = Invert;
   vtab->MapBox = MapBox;
   vtab->MapList = MapList;
   vtab->MapMerge = MapMerge;
   vtab->ReportPoints = ReportPoints;
   vtab->ResampleB = ResampleB;
   vtab->ResampleD = ResampleD;
   vtab->ResampleF = ResampleF;
   vtab->ResampleI = ResampleI;
   vtab->ResampleL = ResampleL;
   vtab->ResampleS = ResampleS;
   vtab->ResampleUB = ResampleUB;
   vtab->ResampleUI = ResampleUI;
   vtab->ResampleUL = ResampleUL;
   vtab->ResampleUS = ResampleUS;
   vtab->SetInvert = SetInvert;
   vtab->SetReport = SetReport;
   vtab->Simplify = Simplify;
   vtab->TestInvert = TestInvert;
   vtab->TestReport = TestReport;
   vtab->Tran1 = Tran1;
   vtab->Tran2 = Tran2;
   vtab->TranN = TranN;
   vtab->TranP = TranP;
   vtab->Transform = Transform;

/* Save the inherited pointers to methods that will be extended, and
   replace them with pointers to the new member functions. */
   object = (AstObjectVtab *) vtab;

   parent_clearattrib = object->ClearAttrib;
   object->ClearAttrib = ClearAttrib;
   parent_getattrib = object->GetAttrib;
   object->GetAttrib = GetAttrib;
   parent_setattrib = object->SetAttrib;
   object->SetAttrib = SetAttrib;
   parent_testattrib = object->TestAttrib;
   object->TestAttrib = TestAttrib;

/* Declare the destructor, copy constructor and dump function. */
   astSetDelete( vtab, Delete );
   astSetCopy( vtab, Copy );
   astSetDump( vtab, Dump, "Mapping", "Mapping between coordinate systems" );
}

/*
*  Name:
*     InterpolateKernel1<X>

*  Purpose:
*     Resample a data grid, using a 1-d interpolation kernel.

*  Type:
*     Private function.

*  Synopsis:
*     #include "mapping.h"
*     int InterpolateKernel1<X>( AstMapping *this, int ndim_in,
*                                const int *lbnd_in, const int *ubnd_in,
*                                const <Xtype> *in, const <Xtype> *in_var,
*                                int npoint, const int *offset,
*                                const double *const *coords,
*                                void (* kernel)( double, const double [], int,
*                                                 double * ),
*                                int neighb, const double *params, int flags,
*                                <Xtype> badval,
*                                <Xtype> *out, <Xtype> *out_var )

*  Class Membership:
*     Mapping member function.

*  Description:
*     This is a set of functions which resample a rectangular input
*     grid of data (and, optionally, associated statistical variance
*     values) so as to place them into a new output grid. Each output
*     grid point may be mapped on to a position in the input grid in
*     an arbitrary way. The input and output grids may have any number
*     of dimensions, not necessarily equal.
*
*     Where the positions given do not correspond with a pixel centre
*     in the input grid, interpolation is performed using a weighted
*     sum of the surrounding pixel values. The weights are determined
*     by a separable kernel which is the product of a 1-dimensional
*     kernel function evaluated along each input dimension. A pointer
*     should be supplied to the 1-dimensional kernel function to be
*     used.

*  Parameters:
*     this
*        Pointer to the Mapping being used in the resampling operation
*        (this is only used for constructing error messages).
*     ndim_in
*        The number of dimensions in the input grid. This should be at
*        least one.
*     lbnd_in
*        Pointer to an array of integers, with "ndim_in" elements.
*        This should give the coordinates of the centre of the first
*        pixel in the input grid along each dimension.
*     ubnd_in
*        Pointer to an array of integers, with "ndim_in" elements.
*        This should give the coordinates of the centre of the last
*        pixel in the input grid along each dimension.
*
*        Note that "lbnd_in" and "ubnd_in" together define the shape
*        and size of the input grid, its extent along a particular
*        (i'th) dimension being ubnd_in[i]-lbnd_in[i]+1 (assuming "i"
*        is zero-based). They also define the input grid's coordinate
*        system, with each pixel being of unit extent along each
*        dimension with integral coordinate values at its centre.
*     in
*        Pointer to the array of data to be resampled (with an element
*        for each pixel in the input grid). The numerical type of
*        these data should match the function used, as given by the
*        suffix on the function name. The storage order should be such
*        that the index of the first grid dimension varies most
*        rapidly and that of the final dimension least rapidly
*        (i.e. Fortran array storage order).
*     in_var
*        An optional pointer to a second array of positive numerical
*        values (with the same size and type as the "in" array), which
*        represent estimates of the statistical variance associated
*        with each element of the "in" array. If this second array is
*        given (along with the corresponding "out_var" array), then
*        estimates of the variance of the resampled data will also be
*        returned.
*
*        If no variance estimates are required, a NULL pointer should
*        be given.
*     npoint
*        The number of points at which the input grid is to be
*        resampled.
*     offset
*        Pointer to an array of integers with "npoint" elements. For
*        each output point, this array should contain the zero-based
*        offset in the output array(s) (i.e. the "out" and,
*        optionally, the "out_var" arrays) at which the resampled
*        output value(s) should be stored.
*     coords
*        An array of pointers to double, with "ndim_in"
*        elements. Element "coords[coord]" should point at the first
*        element of an array of double (with "npoint" elements) which
*        contains the values of coordinate number "coord" for each
*        interpolation point. The value of coordinate number "coord"
*        for interpolation point number "point" is therefore given by
*        "coords[coord][point]" (assuming both indices are
*        zero-based).  If any point has a coordinate value of AST__BAD
*        associated with it, then the corresponding output data (and
*        variance) will be set to the value given by "badval".
*     kernel
*        Pointer to the 1-dimensional kernel function to be used.
*     neighb
*        The number of neighbouring pixels in each dimension (on each
*        side of the interpolation position) which are to contribute
*        to the interpolated value. This value should be at least 1.
*     params
*        Pointer to an optional array of parameter values to be passed
*        to the interpolation kernel function. If no parameters are
*        required by this function, then a NULL pointer may be
*        supplied.
*     flags
*        The bitwise OR of a set of flag values which provide
*        additional control over the resampling operation.
*     badval
*        If the AST__USEBAD flag is set in the "flags" value (above),
*        this parameter specifies the value which is used to identify
*        bad data and/or variance values in the input array(s). Its
*        numerical type must match that of the "in" (and "in_var")
*        arrays. The same value will also be used to flag any output
*        array elements for which resampled values could not be
*        obtained.  The output arrays(s) may be flagged with this
*        value whether or not the AST__USEBAD flag is set (the
*        function return value indicates whether any such values have
*        been produced).
*     out
*        Pointer to an array with the same data type as the "in"
*        array, into which the resampled data will be returned. Note
*        that details of how the output grid maps on to this array
*        (e.g. the storage order, number of dimensions, etc.) is
*        arbitrary and is specified entirely by means of the "offset"
*        array. The "out" array should therefore contain sufficient
*        elements to accommodate the "offset" values supplied.  There
*        is no requirement that all elements of the "out" array should
*        be assigned values, and any which are not addressed by the
*        contents of the "offset" array will be left unchanged.
*     out_var
*        An optional pointer to an array with the same data type and
*        size as the "out" array, into which variance estimates for
*        the resampled values may be returned. This array will only be
*        used if the "in_var" array has been given. It is addressed in
*        exactly the same way (via the "offset" array) as the "out"
*        array. The values returned are estimates of the statistical
*        variance of the corresponding values in the "out" array, on
*        the assumption that all errors in input grid values (in the
*        "in" array) are statistically independent and that their
*        variance estimates (in the "in_var" array) may simply be
*        summed (with appropriate weighting factors).
*
*        If no output variance estimates are required, a NULL pointer
*        should be given.

*  Returned Value:
*     The number of output grid points to which a data value (or a
*     variance value if relevant) equal to "badval" has been assigned
*     because no valid output value could be obtained.

*  Notes:
*     - There is a separate function for each numerical type of
*     gridded data, distinguished by replacing the <X> in the function
*     name by the appropriate 1- or 2-character suffix.
*     - A value of zero will be returned if any of these functions is
*     invoked with the global error status set, or if it should fail
*     for any reason.
*/
/* Define macros to implement the function for a specific data
   type. */
#define MAKE_INTERPOLATE_KERNEL1(X,Xtype,Xfloating,Xfloattype) \
static int InterpolateKernel1##X( AstMapping *this, int ndim_in, \
                                  const int *lbnd_in, const int *ubnd_in, \
                                  const Xtype *in, const Xtype *in_var, \
                                  int npoint, const int *offset, \
                                  const double *const *coords, \
                                  void (* kernel)( double, const double [], \
                                                   int, double * ), \
                                  int neighb, const double *params, \
                                  int flags, Xtype badval, \
                                  Xtype *out, Xtype *out_var ) { \
\
/* Local Variables: */ \
   Xfloattype hi_lim;            /* Upper limit on output values */ \
   Xfloattype lo_lim;            /* Lower limit on output values */ \
   Xfloattype sum;               /* Weighted sum of pixel data values */ \
   Xfloattype sum_var;           /* Weighted sum of pixel variance values */ \
   Xfloattype val;               /* Data value to be assigned to output */ \
   Xfloattype val_var;           /* Variance to be assigned to output */ \
   Xfloattype wtsum;             /* Sum of weight values */ \
   Xfloattype wtsum_sq;          /* Square of sum of weights */ \
   Xtype var;                    /* Variance value */ \
   double **kptr;                /* Pointer to array of kernel pointers */ \
   double **kptr_hi;             /* Array of highest kernel pointer values */ \
   double *kprod;                /* Accumulated kernel value array pointer */ \
   double *kval;                 /* Pointer to array of kernel values */ \
   double *xn_max;               /* Pointer to upper limits array (n-d) */ \
   double *xn_min;               /* Pointer to lower limits array (n-d) */ \
   double pixwt;                 /* Weight to apply to individual pixel */ \
   double wt_y;                  /* Value of y-dependent pixel weight */ \
   double x;                     /* x coordinate value */ \
   double xmax;                  /* x upper limit */ \
   double xmin;                  /* x lower limit */ \
   double xn;                    /* Coordinate value (n-d) */ \
   double y;                     /* y coordinate value */ \
   double ymax;                  /* y upper limit */ \
   double ymin;                  /* y lower limit */ \
   int *hi;                      /* Pointer to array of upper indices */ \
   int *lo;                      /* Pointer to array of lower indices */ \
   int *stride;                  /* Pointer to array of dimension strides */ \
   int bad;                      /* Output pixel bad? */ \
   int bad_var;                  /* Output variance bad? */ \
   int done;                     /* All pixel indices done? */ \
   int hi_x;                     /* Upper pixel index (x dimension) */ \
   int hi_y;                     /* Upper pixel index (y dimension) */ \
   int idim;                     /* Loop counter for dimensions */ \
   int ii;                       /* Loop counter for dimensions */ \
   int ix;                       /* Pixel index in input grid x dimension */ \
   int ixn;                      /* Pixel index in input grid (n-d) */ \
   int iy;                       /* Pixel index in input grid y dimension */ \
   int lo_x;                     /* Lower pixel index (x dimension) */ \
   int lo_y;                     /* Lower pixel index (y dimension) */ \
   int off_in;                   /* Offset to input pixel */ \
   int pixel;                    /* Offset to input pixel containing point */ \
   int point;                    /* Loop counter for output points */ \
   int result;                   /* Result value to return */ \
   int s;                        /* Temporary variable for strides */ \
   int usebad;                   /* Use "bad" input pixel values? */ \
   int usevar;                   /* Process variance array? */ \
   int ystride;                  /* Stride along input grid y dimension */ \
\
/* Initialise. */ \
   result = 0; \
\
/* Check the global error status. */ \
   if ( !astOK ) return result; \
\
/* Determine if we are processing bad pixels or variances. */ \
   usebad = flags & AST__USEBAD; \
   usevar = in_var && out_var; \
\
/* Set up limits for checking output values to ensure that they do not \
   overflow the range of the data type being used. */ \
   lo_lim = LO_##X; \
   hi_lim = HI_##X; \
\
/* Handle the 1-dimensional case optimally. */ \
/* ---------------------------------------- */ \
   if ( ndim_in == 1 ) { \
\
/* Calculate the coordinate limits of the input grid. */ \
      xmin = ( (double) lbnd_in[ 0 ] ) - 0.5; \
      xmax = ( (double) ubnd_in[ 0 ] ) + 0.5; \
\
/* Loop through the list of output points. */ \
      for ( point = 0; point < npoint; point++ ) { \
\
/* Obtain the x coordinate of the current point and test if it lies \
   outside the input grid, or is bad. */ \
         x = coords[ 0 ][ point ]; \
         bad = ( x < xmin ) || ( x >= xmax ) || ( x == AST__BAD ); \
         if ( !bad ) { \
\
/* Obtain the offset along the input grid x dimension of the input \
   pixel which contains the current coordinate, and calculate this \
   pixel's offset from the start of the input array. */ \
            ix = ( (int) floor( x + 0.5 ) ); \
            pixel = ix - lbnd_in[ 0 ] ; \
\
/* Test if the pixel is bad. If not, calculate the lowest and highest \
   indices (in the x dimension) of the region of neighbouring pixels \
   that will contribute to the interpolated result. Constrain these \
   values to lie within the input grid. */ \
            bad = ( in[ pixel ] == badval ) && usebad; \
            if ( !bad ) { \
               ix = (int) floor( x ); \
               lo_x = MaxI( ix - neighb + 1, lbnd_in[ 0 ] ); \
               hi_x = MinI( ix + neighb,     ubnd_in[ 0 ] ); \
\
/* Initialise sums for forming the interpolated result. */ \
               sum = (Xfloattype) 0.0; \
               sum_var = (Xfloattype) 0.0; \
               wtsum = (Xfloattype) 0.0; \
               bad_var = !usevar; \
\
/* Loop to inspect all the contributing pixels, calculating the offset \
   of each pixel from the start of the input array. */ \
               for ( ix = lo_x; ix <= hi_x; ix++ ) { \
                  off_in = ix - lbnd_in[ 0 ]; \
\
/* Test if the input pixel is bad. If not, calculate its weight by \
   evaluating the kernel function. */ \
                  if ( ( in[ off_in ] != badval ) || !usebad ) { \
                     ( *kernel )( ( (double) ix ) - x, params, flags, \
                                  &pixwt ); \
\
/* Check for errors arising in the kernel function. If necessary, \
   report a contextual error message and abort. */ \
                     if ( !astOK ) { \
                        astError( astStatus, "astResample"#X"(%s): Error " \
                                  "signalled by user-supplied 1-d " \
                                  "interpolation kernel.", \
                                  astGetClass( this ) ); \
                        goto KERNEL_ERROR_1D; \
                     } \
\
/* Form the weighted sums required for finding the interpolated \
   value. */ \
                     sum += ( (Xfloattype) pixwt ) * \
                            ( (Xfloattype) in[ off_in ] ); \
                     wtsum += (Xfloattype) pixwt; \
\
/* If a variance estimate is required and it still seems possible to \
   obtain one, then obtain the variance value associated with the \
   current input pixel. */ \
                     if ( !bad_var ) { \
                        var = in_var[ off_in ]; \
\
/* Test if this value is bad (if the data type is signed, also check \
   that it is not negative). */ \
                        bad_var = ( var == badval ) && usebad; \
                        CHECK_FOR_NEGATIVE_VARIANCE(Xtype) \
\
/* If any bad input variance value is obtained, we cannot generate a \
   valid output variance estimate. Otherwise, form the sum needed to \
   calculate this estimate. */ \
                        if ( !bad_var ) { \
                           sum_var += \
                              ( (Xfloattype) ( pixwt * pixwt ) ) * \
                              ( (Xfloattype) in_var[ off_in ] ); \
                        } \
                     } \
                  } \
               } \
            } \
         } \
\
/* If the output data value has not yet been flagged as bad, then \
   check that an interpolated value can actually be produced.  First \
   check that the sum of weights is not zero. */ \
         if ( !bad ) { \
            bad = ( wtsum == (Xfloattype) 0.0 ); \
\
/* If OK, calculate the interpolated value. Then, if the output data \
   type is not floating point, check that this value will not overflow \
   the available output range. */ \
            if ( !bad ) { \
               val = sum / wtsum; \
               if ( !( Xfloating ) ) { \
                  bad = ( val <= lo_lim ) || ( val >= hi_lim ); \
               } \
            } \
\
/* If no interpolated data value can be produced, then no associated \
   variance will be required either. */ \
            if ( bad ) bad_var = 1; \
         } \
\
/* Now perform similar checks on the output variance value (if \
   required).  This time we check that the square of the sum of \
   weights is not zero (since this might underflow before the sum of \
   weights). Again we also check to prevent the result overflowing the \
   output data type. */ \
         if ( !bad_var ) { \
            wtsum_sq = wtsum * wtsum; \
            bad_var = ( wtsum_sq == (Xfloattype) 0.0 ); \
            if ( !bad_var ) { \
               val_var = sum_var / wtsum_sq; \
               if ( !( Xfloating ) ) { \
                  bad_var = ( val_var <= lo_lim ) || \
                            ( val_var >= hi_lim ); \
               } \
            } \
         } \
\
/* Assign a bad output value (and variance) if required and count it. */ \
         if ( bad ) { \
            out[ offset[ point ] ] = badval; \
            if ( usevar ) out_var[ offset[ point ] ] = badval; \
            result++; \
\
/* Otherwise, assign the interpolated value. If the output data type \
   is floating point, this result can be stored directly, otherwise we \
   must round to the nearest integer. */ \
         } else { \
            if ( Xfloating ) { \
               out[ offset[ point ] ] = (Xtype) val; \
            } else { \
               out[ offset[ point ] ] = (Xtype) ( val + \
                                        ( ( val >= (Xfloattype) 0.0 ) ? \
                                          ( (Xfloattype) 0.5 ) : \
                                          ( (Xfloattype) -0.5 ) ) ); \
            } \
\
/* If a variance estimate is required but none can be obtained, then \
   store a bad output variance value and count it. */ \
            if ( usevar ) { \
               if ( bad_var ) { \
                  out_var[ offset[ point ] ] = badval; \
                  result++; \
\
/* Otherwise, assign the variance estimate, rounding to the nearest \
   integer if necessary. */ \
               } else { \
                  if ( Xfloating ) { \
                     out_var[ offset[ point ] ] = (Xtype) val_var; \
                  } else { \
                     out_var[ offset[ point ] ] = (Xtype) ( val_var + \
                                        ( ( val_var >= (Xfloattype) 0.0 ) ? \
                                          ( (Xfloattype) 0.5 ) : \
                                          ( (Xfloattype) -0.5 ) ) ); \
                  } \
               } \
            } \
         } \
      } \
KERNEL_ERROR_1D:                 /* Exit point on error in kernel function */ \
\
/* Handle the 2-dimensional case optimally. */ \
/* ---------------------------------------- */ \
   } else if ( ndim_in == 2 ) { \
\
/* Allocate workspace. */ \
      kval = astMalloc( sizeof( double ) * (size_t) ( 2 * neighb ) ); \
      if ( astOK ) { \
\
/* Calculate the stride along the y dimension of the input grid. */ \
         ystride = ubnd_in[ 0 ] - lbnd_in[ 0 ] + 1; \
\
/* Calculate the coordinate limits of the input grid in each \
   dimension. */ \
         xmin = ( (double) lbnd_in[ 0 ] ) - 0.5; \
         xmax = ( (double) ubnd_in[ 0 ] ) + 0.5; \
         ymin = ( (double) lbnd_in[ 1 ] ) - 0.5; \
         ymax = ( (double) ubnd_in[ 1 ] ) + 0.5; \
\
/* Loop through the list of output points. */ \
         for ( point = 0; point < npoint; point++ ) { \
\
/* Obtain the x coordinate of the current point and test if it lies \
   outside the input grid, or is bad. */ \
            x = coords[ 0 ][ point ]; \
            bad = ( x < xmin ) || ( x >= xmax ) || ( x == AST__BAD ); \
            if ( !bad ) { \
\
/* If not, then similarly obtain and test the y coordinate. */ \
               y = coords[ 1 ][ point ]; \
               bad = ( y < ymin ) || ( y >= ymax ) || ( y == AST__BAD ); \
               if ( !bad ) { \
\
/* Obtain the offsets along each input grid dimension of the input \
   pixel which contains the current coordinates, and calculate this \
   pixel's offset from the start of the input array. */ \
                  ix = ( (int) floor( x + 0.5 ) ); \
                  iy = ( (int) floor( y + 0.5 ) ); \
                  pixel = ix - lbnd_in[ 0 ] + \
                          ystride * ( iy - lbnd_in[ 1 ] ); \
\
/* Test if the pixel is bad. If not, calculate the lowest and highest \
   indices (in each dimension) of the region of neighbouring pixels \
   that will contribute to the interpolated result. Constrain these \
   values to lie within the input grid. */ \
                  bad = ( in[ pixel ] == badval ) && usebad; \
                  if ( !bad ) { \
                     ix = (int) floor( x ); \
                     lo_x = MaxI( ix - neighb + 1, lbnd_in[ 0 ] ); \
                     hi_x = MinI( ix + neighb,     ubnd_in[ 0 ] ); \
                     iy = (int) floor( y ); \
                     lo_y = MaxI( iy - neighb + 1, lbnd_in[ 1 ] ); \
                     hi_y = MinI( iy + neighb,     ubnd_in[ 1 ] ); \
\
/* Loop to evaluate the kernel function along the x dimension, storing \
   the resulting values. The function's argument is the offset of the \
   contributing pixel (along this dimension) from the input \
   position. */ \
                     for ( ix = lo_x; ix <= hi_x; ix++ ) { \
                        ( *kernel )( ( (double) ix ) - x, params, flags, \
                                     kval + ix - lo_x ); \
\
/* Check for errors arising in the kernel function. If necessary, \
   report a contextual error message and abort. */ \
                        if ( !astOK ) { \
                           astError( astStatus, "astResample"#X"(%s): Error " \
                                     "signalled by user-supplied 1-d " \
                                     "interpolation kernel.", \
                                     astGetClass( this ) ); \
                           goto KERNEL_ERROR_2D; \
                        } \
                     } \
\
/* Initialise sums for forming the interpolated result. */ \
                     sum = (Xfloattype) 0.0; \
                     sum_var = (Xfloattype) 0.0; \
                     wtsum = (Xfloattype) 0.0; \
                     bad_var = !usevar; \
\
/* Loop to inspect all the contributing pixels while evaluating the \
   kernel function for each y index value. */ \
                     for ( iy = lo_y; iy <= hi_y; iy++ ) { \
                        ( *kernel )( ( (double) iy ) - y, params, flags, \
                                     &wt_y ); \
\
/* Check for errors arising in the kernel function. If necessary, \
   report a contextual error message and abort. */ \
                        if ( !astOK ) { \
                           astError( astStatus, "astResample"#X"(%s): Error " \
                                     "signalled by user-supplied 1-d " \
                                     "interpolation kernel.", \
                                     astGetClass( this ) ); \
                           goto KERNEL_ERROR_2D; \
                        } \
                        for ( ix = lo_x; ix <= hi_x; ix++ ) { \
\
/* Calculate the offset of each contributing pixel from the start of \
   the input array. */ \
                           off_in = ix - lbnd_in[ 0 ] + \
                                    ystride * ( iy - lbnd_in[ 1 ] ); \
\
/* Test if the input pixel is bad. If not, calculate its weight as the
   product of the kernel function's value for the x and y
   dimensions. */ \
                           if ( ( in[ off_in ] != badval ) || !usebad ) { \
                              pixwt = kval[ ix - lo_x ] * wt_y;\
\
/* Form the weighted sums required for finding the interpolated \
   value. */ \
                              sum += ( (Xfloattype) pixwt ) * \
                                     ( (Xfloattype) in[ off_in ] ); \
                              wtsum += (Xfloattype) pixwt; \
\
/* If a variance estimate is required and it still seems possible to \
   obtain one, then obtain the variance value associated with the \
   current input pixel. */ \
                              if ( !bad_var ) { \
                                 var = in_var[ off_in ]; \
\
/* Test if this value is bad (if the data type is signed, also check \
   that it is not negative). */ \
                                 bad_var = ( var == badval ) && usebad; \
                                 CHECK_FOR_NEGATIVE_VARIANCE(Xtype) \
\
/* If any bad input variance value is obtained, we cannot generate a \
   valid output variance estimate. Otherwise, form the sum needed to \
   calculate this estimate. */ \
                                 if ( !bad_var ) { \
                                    sum_var += \
                                       ( (Xfloattype) ( pixwt * pixwt ) ) * \
                                       ( (Xfloattype) in_var[ off_in ] ); \
                                 } \
                              } \
                           } \
                        } \
                     } \
                  } \
               } \
            } \
\
/* If the output data value has not yet been flagged as bad, then \
   check that an interpolated value can actually be produced.  First \
   check that the sum of weights is not zero. */ \
            if ( !bad ) { \
               bad = ( wtsum == (Xfloattype) 0.0 ); \
\
/* If OK, calculate the interpolated value. Then, if the output data \
   type is not floating point, check that this value will not overflow \
   the available output range. */ \
               if ( !bad ) { \
                  val = sum / wtsum; \
                  if ( !( Xfloating ) ) { \
                     bad = ( val <= lo_lim ) || ( val >= hi_lim ); \
                  } \
               } \
\
/* If no interpolated data value can be produced, then no associated \
   variance will be required either. */ \
               if ( bad ) bad_var = 1; \
            } \
\
/* Now perform similar checks on the output variance value (if \
   required).  This time we check that the square of the sum of \
   weights is not zero (since this might underflow before the sum of \
   weights). Again we also check to prevent the result overflowing the \
   output data type. */ \
            if ( !bad_var ) { \
               wtsum_sq = wtsum * wtsum; \
               bad_var = ( wtsum_sq == (Xfloattype) 0.0 ); \
               if ( !bad_var ) { \
                  val_var = sum_var / wtsum_sq; \
                  if ( !( Xfloating ) ) { \
                     bad_var = ( val_var <= lo_lim ) || \
                               ( val_var >= hi_lim ); \
                  } \
               } \
            } \
\
/* Assign a bad output value (and variance) if required and count it. */ \
            if ( bad ) { \
               out[ offset[ point ] ] = badval; \
               if ( usevar ) out_var[ offset[ point ] ] = badval; \
               result++; \
\
/* Otherwise, assign the interpolated value. If the output data type \
   is floating point, the result can be stored directly, otherwise we \
   must round to the nearest integer. */ \
            } else { \
               if ( Xfloating ) { \
                  out[ offset[ point ] ] = (Xtype) val; \
               } else { \
                  out[ offset[ point ] ] = (Xtype) ( val + \
                                           ( ( val >= (Xfloattype) 0.0 ) ? \
                                             ( (Xfloattype) 0.5 ) : \
                                             ( (Xfloattype) -0.5 ) ) ); \
               } \
\
/* If a variance estimate is required but none can be obtained, then \
   store a bad output variance value and count it. */ \
               if ( usevar ) { \
                  if ( bad_var ) { \
                     out_var[ offset[ point ] ] = badval; \
                     result++; \
\
/* Otherwise, store the variance estimate, rounding to the nearest \
   integer if necessary. */ \
                  } else { \
                     if ( Xfloating ) { \
                        out_var[ offset[ point ] ] = (Xtype) val_var; \
                     } else { \
                        out_var[ offset[ point ] ] = (Xtype) ( val_var + \
                                        ( ( val_var >= (Xfloattype) 0.0 ) ? \
                                          ( (Xfloattype) 0.5 ) : \
                                          ( (Xfloattype) -0.5 ) ) ); \
                     } \
                  } \
               } \
            } \
         } \
KERNEL_ERROR_2D:                 /* Exit point on error in kernel function */ \
      } \
\
/* Free the workspace. */ \
      kval = astFree( kval ); \
\
/* Handle other numbers of dimensions. */ \
/* ----------------------------------- */ \
   } else { \
\
/* Allocate workspace. */ \
      hi = astMalloc( sizeof( int ) * (size_t) ndim_in ); \
      lo = astMalloc( sizeof( int ) * (size_t) ndim_in ); \
      stride = astMalloc( sizeof( int ) * (size_t) ndim_in ); \
      xn_max = astMalloc( sizeof( double ) * (size_t) ndim_in ); \
      xn_min = astMalloc( sizeof( double ) * (size_t) ndim_in ); \
      kval = astMalloc( sizeof( double ) * (size_t) \
                                           ( 2 * neighb * ndim_in ) ); \
      kptr = astMalloc( sizeof( double * ) * (size_t) ndim_in ); \
      kptr_hi = astMalloc( sizeof( double * ) * (size_t) ndim_in ); \
      kprod = astMalloc( sizeof( double ) * (size_t) ndim_in ); \
      if ( astOK ) { \
\
/* Calculate the stride along each dimension of the input grid. */ \
         for ( s = 1, idim = 0; idim < ndim_in; idim++ ) { \
            stride[ idim ] = s; \
            s *= ubnd_in[ idim ] - lbnd_in[ idim ] + 1; \
\
/* Calculate the coordinate limits of the input grid in each \
   dimension. */ \
            xn_min[ idim ] = ( (double) lbnd_in[ idim ] ) - 0.5; \
            xn_max[ idim ] = ( (double) ubnd_in[ idim ] ) + 0.5; \
         } \
\
/* Loop through the list of output points. */ \
         for ( point = 0; point < npoint; point++ ) { \
\
/* Initialise offsets into the input array. Then loop to obtain each \
   coordinate associated with the current output point. */ \
            pixel = 0; \
            off_in = 0; \
            for ( idim = 0; idim < ndim_in; idim++ ) { \
               xn = coords[ idim ][ point ]; \
\
/* Test if the coordinate lies outside the input grid, or is bad. If \
   either is true, the corresponding output pixel value will be bad, \
   so give up on this point. */ \
               bad = ( xn < xn_min[ idim ] ) || ( xn >= xn_max[ idim ] ) || \
                     ( xn == AST__BAD ); \
               if ( bad ) break; \
\
/* Obtain the index along the current input grid dimension of the \
   pixel which contains this coordinate and accumulate the pixel's \
   offset from the start of the input array. */ \
               ixn = (int) floor( xn + 0.5 ); \
               pixel += stride[ idim ] * ( ixn - lbnd_in[ idim ] ); \
\
/* Calculate the lowest and highest indices (in the current dimension) \
   of the region of neighbouring pixels that will contribute to the \
   interpolated result. Constrain these values to lie within the input \
   grid. */ \
               ixn = (int) floor( xn ); \
               lo[ idim ] = MaxI( ixn - neighb + 1, lbnd_in[ idim ] ); \
               hi[ idim ] = MinI( ixn + neighb,     ubnd_in[ idim ] ); \
\
/* Accumulate the offset (from the start of the input array) of the
   contributing pixel which has the lowest index in each dimension. */ \
               off_in += stride[ idim ] * ( lo[ idim ] - lbnd_in[ idim ] ); \
            } \
\
/* Once the input pixel which contains the required coordinates has \
   been identified, test if it is bad. */ \
            bad = bad || ( ( in[ pixel ] == badval ) && usebad ); \
\
/* If OK, loop to evaluate the kernel function which will be used to \
   weight the contributions from surrounding pixels. */ \
            if ( !bad ) { \
               for ( idim = 0; idim < ndim_in; idim++ ) { \
\
/* Set up an array of pointers to locate kernel values (stored in the \
   "kval" array) for each dimension. Initially, each of these pointers \
   locates the first kernel value, which applies to the contributing \
   pixel with the lowest index in each dimension. */ \
                  kptr[ idim ] = kval + ( 2 * neighb * idim ); \
\
/* Also set up pointers to the last kernel value in each dimension. */ \
                  kptr_hi[ idim ] = kptr[ idim ] + \
                                    ( hi[ idim ] - lo[ idim ] ); \
\
/* Loop to evaluate the kernel function along each dimension, storing \
   the resulting values. The function's argument is the offset of the \
   contributing pixel (along the relevant dimension) from the input \
   position. */ \
                  xn = coords[ idim ][ point ]; \
                  for ( ixn = lo[ idim ]; ixn <= hi[ idim ]; ixn++ ) { \
                     ( *kernel )( ( (double) ixn ) - xn, params, flags, \
                                  kptr[ idim ] + ixn - lo[ idim ] ); \
\
/* Check for errors arising in the kernel function. If necessary, \
   report a contextual error message and abort. */ \
                     if ( !astOK ) { \
                        astError( astStatus, "astResample"#X"(%s): Error " \
                                  "signalled by user-supplied 1-d " \
                                  "interpolation kernel.", \
                                  astGetClass( this ) ); \
                        goto KERNEL_ERROR_ND; \
                     } \
                  } \
               } \
\
/* Initialise, and loop over the neighbouring input pixels to obtain \
   an interpolated value. */ \
               sum = (Xfloattype) 0.0; \
               sum_var = (Xfloattype) 0.0; \
               wtsum = (Xfloattype) 0.0; \
               bad_var = !usevar; \
               idim = ndim_in - 1; \
               kprod[ idim ] = 1.0; \
               done = 0; \
               while ( !done ) { \
\
/* Each contributing pixel is weighted by the product of the kernel \
   function's value evaluated along each input dimension. However, \
   since we typically only change the index of one dimension at a \
   time, we can avoid forming this product repeatedly by retaining an \
   array of accumulated products for all higher dimensions. We need \
   then only update the lower elements in this array, corresponding to \
   those dimensions whose index has changed. We do this here, "idim" \
   being the index of the most significant dimension to have \
   changed. Note that on the first pass, all dimensions are considered \
   changed, causing this array to be initialised. */ \
                  for ( ii = idim; ii >= 1; ii-- ) { \
                     kprod[ ii - 1 ] = kprod[ ii ] * *( kptr[ ii ] ); \
                  } \
\
/* Test each pixel which may contribute to the result to see if it is \
   bad. If not, obtain its weight from the accumulated product of \
   kernel values for dimension zero (also multiply by the appropriate \
   kernel value for dimension zero, since this is not included in the \
   "kprod" array). */ \
                  if ( ( in[ off_in ] != badval ) || !usebad ) { \
                     pixwt = kprod[ 0 ] * *( kptr[ 0 ] ); \
\
/* Form the weighted sums required for finding the interpolated \
   value. */ \
                     sum += ( (Xfloattype) pixwt ) * \
                            ( (Xfloattype) in[ off_in ] ); \
                     wtsum += (Xfloattype) pixwt; \
\
/* If a variance estimate is required and it still seems possible to \
   obtain one, then obtain the variance value associated with the \
   current input pixel. */ \
                     if ( !bad_var ) { \
                        var = in_var[ off_in ]; \
\
/* Test if this value is bad (if the data type is signed, also check \
   that it is not negative). */ \
                        bad_var = ( var == badval ) && usebad; \
                        CHECK_FOR_NEGATIVE_VARIANCE(Xtype) \
\
/* If any bad input variance value is obtained, we cannot generate a \
   valid output variance estimate. Otherwise, form the sum needed to \
   calculate this estimate. */ \
                        if ( !bad_var ) { \
                           sum_var += ( (Xfloattype) ( pixwt * pixwt ) ) * \
                                      ( (Xfloattype) in_var[ off_in ] ); \
                        } \
                     } \
                  } \
\
/* Now update the kernel value pointers and pixel offset to refer to \
   the next input pixel to be considered. */ \
                  idim = 0; \
                  do { \
\
/* The first input dimension whose kernel value pointer has not yet \
   reached its maximum value has this pointer incremented, and the \
   pixel offset into the input array is updated accordingly. */ \
                     if ( kptr[ idim ] != kptr_hi[ idim ] ) { \
                        kptr[ idim ]++; \
                        off_in += stride[ idim ]; \
                        break; \
\
/* Any earlier dimensions (which have reached the highest pointer \
   value) have this pointer returned to its lowest value. Again, the \
   pixel offset into the input image is updated accordingly. */ \
                     } else { \
                        kptr[ idim ] -= ( hi[ idim ] - lo[ idim ] ); \
                        off_in -= stride[ idim ] * \
                                  ( hi[ idim ] - lo[ idim ] ); \
                        done = ( ++idim == ndim_in ); \
                     } \
                  } while ( !done ); \
               } \
\
/* Check that an interpolated value can actually be produced. First \
   check that the sum of weights is not zero. */ \
               bad = ( wtsum == (Xfloattype) 0.0 ); \
\
/* If OK, calculate the interpolated value. Then, if the output data \
   type is not floating point, check that this value will not overflow \
   the available output range. */ \
               if ( !bad ) { \
                  val = sum / wtsum; \
                  if ( !( Xfloating ) ) { \
                     bad = ( val <= lo_lim ) || ( val >= hi_lim ); \
                  } \
               } \
\
/* If no interpolated data value can be produced, then no associated \
   variance will be required either. */ \
               if ( bad ) bad_var = 1; \
\
/* Now perform similar checks on the output variance value (if \
   required).  This time we check that the square of the sum of \
   weights is not zero (since this might underflow before the sum of \
   weights). Again we also check to prevent the result overflowing the \
   output data type. */ \
               if ( !bad_var ) { \
                  wtsum_sq = wtsum * wtsum; \
                  bad_var = ( wtsum_sq == (Xfloattype) 0.0 ); \
                  if ( !bad_var ) { \
                     val_var = sum_var / wtsum_sq; \
                     if ( !( Xfloating ) ) { \
                         bad_var = ( val_var <= lo_lim ) || \
                                   ( val_var >= hi_lim ); \
                     } \
                  } \
               } \
            } \
\
/* Assign a bad output value (and variance) if required and count it. */ \
            if ( bad ) { \
               out[ offset[ point ] ] = badval; \
               if ( usevar ) out_var[ offset[ point ] ] = badval; \
               result++; \
\
/* Otherwise, assign the interpolated value. If the output data type \
   is floating point, this result can be stored directly, otherwise we \
   must round to the nearest integer. */ \
            } else { \
               if ( Xfloating ) { \
                  out[ offset[ point ] ] = (Xtype) val; \
               } else { \
                  out[ offset[ point ] ] = (Xtype) ( val + \
                                           ( ( val >= (Xfloattype) 0.0 ) ? \
                                             ( (Xfloattype) 0.5 ) : \
                                             ( (Xfloattype) -0.5 ) ) ); \
               } \
\
/* If a variance estimate is required but none can be obtained, then \
   store a bad output variance value and count it. */ \
               if ( usevar ) { \
                  if ( bad_var ) { \
                     out_var[ offset[ point ] ] = badval; \
                     result++; \
\
/* Otherwise, assign the variance estimate, rounding to the nearest \
   integer if necessary. */ \
                  } else { \
                     if ( Xfloating ) { \
                        out_var[ offset[ point ] ] = (Xtype) val_var; \
                     } else { \
                        out_var[ offset[ point ] ] = (Xtype) ( val_var + \
                                        ( ( val_var >= (Xfloattype) 0.0 ) ? \
                                          ( (Xfloattype) 0.5 ) : \
                                          ( (Xfloattype) -0.5 ) ) ); \
                     } \
                  } \
               } \
            } \
         } \
KERNEL_ERROR_ND:                 /* Exit point on error in kernel function */ \
      } \
\
/* Free the workspace. */ \
      hi = astFree( hi ); \
      lo = astFree( lo ); \
      stride = astFree( stride ); \
      xn_max = astFree( xn_max ); \
      xn_min = astFree( xn_min ); \
      kval = astFree( kval ); \
      kptr = astFree( kptr ); \
      kptr_hi = astFree( kptr_hi ); \
      kprod = astFree( kprod ); \
   } \
\
/* If an error has occurred, clear the returned result. */ \
   if ( !astOK ) result = 0; \
\
/* Return the result. */ \
   return result; \
}

/* This subsidiary macro tests for negative variance values in the
   macro above. This check is required only for signed data types. */
#define CHECK_FOR_NEGATIVE_VARIANCE(Xtype) \
        bad_var = bad_var || ( var < ( (Xtype) 0 ) );

/* These subsidiary macros define limits for range checking of results
   before conversion to the final data type. For each data type code
   <X>, HI_<X> gives the least positive floating point value which
   just overflows that data type towards plus infinity, while LO_<X>
   gives the least negative floating point value which just overflows
   that data type towards minus infinity.  Thus, a floating point
   value must satisfy LO<flt_value<HI if overflow is not to occur when
   it is converted to that data type.

   The data type of each limit is that of the smallest precision
   floating point type which will accommodate the full range of values
   that the target type may take. */
   
/* If <X> is a floating point type, the limits are not actually used,
   but must be present to permit error-free compilation. */
#if defined(AST_LONG_DOUBLE)     /* Not normally implemented */
#define HI_LD ( 0.0L )
#define LO_LD ( 0.0L )
#endif
#define HI_D ( 0.0 )
#define LO_D ( 0.0 )
#define HI_F ( 0.0f )
#define LO_F ( 0.0f )

#if defined(AST_LONG_DOUBLE)     /* Not normally implemented */
#define HI_L ( ( (long double) ( LONG_MAX ) ) + 0.5L )
#define LO_L ( ( (long double) ( LONG_MIN ) ) - 0.5L )
#define HI_UL ( ( (long double) ( ULONG_MAX ) ) + 0.5L )
#define LO_UL ( -0.5L )
#else
#define HI_L ( ( (double) ( LONG_MAX ) ) + 0.5 )
#define LO_L ( ( (double) ( LONG_MIN ) ) - 0.5 )
#define HI_UL ( ( (double) ( ULONG_MAX ) ) + 0.5 )
#define LO_UL ( -0.5 )
#endif
#define HI_I ( ( (double) ( INT_MAX ) ) + 0.5 )
#define LO_I ( ( (double) ( INT_MIN ) ) - 0.5 )
#define HI_UI ( ( (double) ( UINT_MAX ) ) + 0.5 )
#define LO_UI ( -0.5 )
#define HI_S ( ( (float) ( SHRT_MAX ) ) + 0.5f )
#define LO_S ( ( (float) ( SHRT_MIN ) ) - 0.5f )
#define HI_US ( ( (float) ( USHRT_MAX ) ) + 0.5f )
#define LO_US ( -0.5f )
#define HI_B ( ( (float) ( SCHAR_MAX ) ) + 0.5f )
#define LO_B ( ( (float) ( SCHAR_MIN ) ) - 0.5f )
#define HI_UB ( ( (float) ( UCHAR_MAX ) ) + 0.5f )
#define LO_UB ( -0.5f )

/* Expand the main macro above to generate a function for each
   required signed data type. */
#if defined(AST_LONG_DOUBLE)     /* Not normally implemented */
MAKE_INTERPOLATE_KERNEL1(LD,long double,1,long double)
MAKE_INTERPOLATE_KERNEL1(L,long int,0,long double)
#else
MAKE_INTERPOLATE_KERNEL1(L,long int,0,double)
#endif     
MAKE_INTERPOLATE_KERNEL1(D,double,1,double)
MAKE_INTERPOLATE_KERNEL1(F,float,1,float)
MAKE_INTERPOLATE_KERNEL1(I,int,0,double)
MAKE_INTERPOLATE_KERNEL1(S,short int,0,float)
MAKE_INTERPOLATE_KERNEL1(B,signed char,0,float)

/* Re-define the macro for testing for negative variances to do
   nothing. */
#undef CHECK_FOR_NEGATIVE_VARIANCE
#define CHECK_FOR_NEGATIVE_VARIANCE(Xtype)

/* Expand the main macro above to generate a function for each
   required unsigned data type. */
#if defined(AST_LONG_DOUBLE)     /* Not normally implemented */
MAKE_INTERPOLATE_KERNEL1(UL,unsigned long int,0,long double)
#else
MAKE_INTERPOLATE_KERNEL1(UL,unsigned long int,0,double)
#endif     
MAKE_INTERPOLATE_KERNEL1(UI,unsigned int,0,double)
MAKE_INTERPOLATE_KERNEL1(US,unsigned short int,0,float)
MAKE_INTERPOLATE_KERNEL1(UB,unsigned char,0,float)

/* Undefine the macros. */
#if defined(AST_LONG_DOUBLE)     /* Not normally implemented */
#undef HI_LD
#undef LO_LD
#endif
#undef HI_D
#undef LO_D
#undef HI_F
#undef LO_F
#undef HI_L
#undef LO_L
#undef HI_UL
#undef LO_UL
#undef HI_I
#undef LO_I
#undef HI_UI
#undef LO_UI
#undef HI_S
#undef LO_S
#undef HI_US
#undef LO_US
#undef HI_B
#undef LO_B
#undef HI_UB
#undef LO_UB
#undef CHECK_FOR_NEGATIVE_VARIANCE
#undef MAKE_INTERPOLATE_KERNEL1

/*
*  Name:
*     InterpolateLinear<X>

*  Purpose:
*     Resample a data grid, using the linear interpolation scheme.

*  Type:
*     Private function.

*  Synopsis:
*     #include "mapping.h"
*     int InterpolateLinear<X>( int ndim_in,
*                               const int *lbnd_in, const int *ubnd_in,
*                               const <Xtype> *in, const <Xtype> *in_var,
*                               int npoint, const int *offset,
*                               const double *const *coords,
*                               int flags, <Xtype> badval,
*                               <Xtype> *out, <Xtype> *out_var )

*  Class Membership:
*     Mapping member function.

*  Description:
*     This is a set of functions which resample a rectangular input
*     grid of data (and, optionally, associated statistical variance
*     values) so as to place them into a new output grid. Each output
*     grid point may be mapped on to a position in the input grid in
*     an arbitrary way. Where the positions given do not correspond
*     with a pixel centre in the input grid, the interpolation scheme
*     used is linear interpolation between the centres of the nearest
*     neighbouring pixels in each dimension (there are 2 nearest
*     neighbours in 1 dimension, 4 in 2 dimensions, 8 in 3 dimensions,
*     etc.). The input and output grids may have any number of
*     dimensions, not necessarily equal.

*  Parameters:
*     ndim_in
*        The number of dimensions in the input grid. This should be at
*        least one.
*     lbnd_in
*        Pointer to an array of integers, with "ndim_in" elements.
*        This should give the coordinates of the centre of the first
*        pixel in the input grid along each dimension.
*     ubnd_in
*        Pointer to an array of integers, with "ndim_in" elements.
*        This should give the coordinates of the centre of the last
*        pixel in the input grid along each dimension.
*
*        Note that "lbnd_in" and "ubnd_in" together define the shape
*        and size of the input grid, its extent along a particular
*        (i'th) dimension being ubnd_in[i]-lbnd_in[i]+1 (assuming "i"
*        is zero-based). They also define the input grid's coordinate
*        system, with each pixel being of unit extent along each
*        dimension with integral coordinate values at its centre.
*     in
*        Pointer to the array of data to be resampled (with an element
*        for each pixel in the input grid). The numerical type of
*        these data should match the function used, as given by the
*        suffix on the function name. The storage order should be such
*        that the index of the first grid dimension varies most
*        rapidly and that of the final dimension least rapidly
*        (i.e. Fortran array storage order).
*     in_var
*        An optional pointer to a second array of positive numerical
*        values (with the same size and type as the "in" array), which
*        represent estimates of the statistical variance associated
*        with each element of the "in" array. If this second array is
*        given (along with the corresponding "out_var" array), then
*        estimates of the variance of the resampled data will also be
*        returned.
*
*        If no variance estimates are required, a NULL pointer should
*        be given.
*     npoint
*        The number of points at which the input grid is to be
*        resampled.
*     offset
*        Pointer to an array of integers with "npoint" elements. For
*        each output point, this array should contain the zero-based
*        offset in the output array(s) (i.e. the "out" and,
*        optionally, the "out_var" arrays) at which the resampled
*        output value(s) should be stored.
*     coords
*        An array of pointers to double, with "ndim_in"
*        elements. Element "coords[coord]" should point at the first
*        element of an array of double (with "npoint" elements) which
*        contains the values of coordinate number "coord" for each
*        interpolation point. The value of coordinate number "coord"
*        for interpolation point number "point" is therefore given by
*        "coords[coord][point]" (assuming both indices are
*        zero-based).  If any point has a coordinate value of AST__BAD
*        associated with it, then the corresponding output data (and
*        variance) will be set to the value given by "badval".
*     flags
*        The bitwise OR of a set of flag values which control the
*        operation of the function. Currently, only the flag
*        AST__USEBAD is significant and indicates whether there are
*        "bad" (i.e. missing) data in the input array(s) which must be
*        recognised and propagated to the output array(s).  If this
*        flag is not set, all input values are treated literally.
*     badval
*        If the AST__USEBAD flag is set in the "flags" value (above),
*        this parameter specifies the value which is used to identify
*        bad data and/or variance values in the input array(s). Its
*        numerical type must match that of the "in" (and "in_var")
*        arrays. The same value will also be used to flag any output
*        array elements for which resampled values could not be
*        obtained.  The output arrays(s) may be flagged with this
*        value whether or not the AST__USEBAD flag is set (the
*        function return value indicates whether any such values have
*        been produced).
*     out
*        Pointer to an array with the same data type as the "in"
*        array, into which the resampled data will be returned. Note
*        that details of how the output grid maps on to this array
*        (e.g. the storage order, number of dimensions, etc.) is
*        arbitrary and is specified entirely by means of the "offset"
*        array. The "out" array should therefore contain sufficient
*        elements to accommodate the "offset" values supplied.  There
*        is no requirement that all elements of the "out" array should
*        be assigned values, and any which are not addressed by the
*        contents of the "offset" array will be left unchanged.
*     out_var
*        An optional pointer to an array with the same data type and
*        size as the "out" array, into which variance estimates for
*        the resampled values may be returned. This array will only be
*        used if the "in_var" array has been given. It is addressed in
*        exactly the same way (via the "offset" array) as the "out"
*        array. The values returned are estimates of the statistical
*        variance of the corresponding values in the "out" array, on
*        the assumption that all errors in input grid values (in the
*        "in" array) are statistically independent and that their
*        variance estimates (in the "in_var" array) may simply be
*        summed (with appropriate weighting factors).
*
*        If no output variance estimates are required, a NULL pointer
*        should be given.

*  Returned Value:
*     The number of output grid points to which a data value (or a
*     variance value if relevant) equal to "badval" has been assigned
*     because no valid output value could be obtained.

*  Notes:
*     - There is a separate function for each numerical type of
*     gridded data, distinguished by replacing the <X> in the function
*     name by the appropriate 1- or 2-character suffix.
*     - A value of zero will be returned if any of these functions is
*     invoked with the global error status set, or if it should fail
*     for any reason.
*/
/* Define macros to implement the function for a specific data
   type. */
#define MAKE_INTERPOLATE_LINEAR(X,Xtype,Xfloating,Xfloattype) \
static int InterpolateLinear##X( int ndim_in, \
                                 const int *lbnd_in, const int *ubnd_in, \
                                 const Xtype *in, const Xtype *in_var, \
                                 int npoint, const int *offset, \
                                 const double *const *coords, \
                                 int flags, Xtype badval, \
                                 Xtype *out, Xtype *out_var ) { \
\
/* Local Variables: */ \
   Xfloattype sum;               /* Weighted sum of pixel data values */ \
   Xfloattype sum_var;           /* Weighted sum of pixel variance values */ \
   Xfloattype val;               /* Value to be asigned to output pixel */ \
   Xfloattype wtsum;             /* Sum of weight values */ \
   Xtype var;                    /* Variance value */ \
   double *frac_hi;              /* Pointer to array of weights */ \
   double *frac_lo;              /* Pointer to array of weights */ \
   double *wt;                   /* Pointer to array of weights */ \
   double *xn_max;               /* Pointer to upper limits array (n-d) */ \
   double *xn_min;               /* Pointer to lower limits array (n-d) */ \
   double dx;                    /* x offset from pixel centre */ \
   double dxn;                   /* Offset from pixel centre (n-d) */ \
   double dy;                    /* y offset from pixel centre */ \
   double frac_hi_x;             /* Pixel weight (x dimension) */ \
   double frac_hi_y;             /* Pixel weight (y dimension) */ \
   double frac_lo_x;             /* Pixel weight (x dimension) */ \
   double frac_lo_y;             /* Pixel weight (y dimension) */ \
   double pixwt;                 /* Weight to apply to individual pixel */ \
   double x;                     /* x coordinate value */ \
   double xmax;                  /* x upper limit */ \
   double xmin;                  /* x lower limit */ \
   double xn;                    /* Coordinate value (n-d) */ \
   double y;                     /* y coordinate value */ \
   double ymax;                  /* y upper limit */ \
   double ymin;                  /* y lower limit */ \
   int *dim;                     /* Pointer to array of pixel indices */ \
   int *hi;                      /* Pointer to array of upper indices */ \
   int *lo;                      /* Pointer to array of lower indices */ \
   int *stride;                  /* Pointer to array of dimension strides */ \
   int bad;                      /* Output pixel bad? */ \
   int bad_var;                  /* Output variance bad? */ \
   int done;                     /* All pixel indices done? */ \
   int hi_x;                     /* Upper pixel index (x dimension) */ \
   int hi_y;                     /* Upper pixel index (y dimension) */ \
   int idim;                     /* Loop counter for dimensions */ \
   int ix;                       /* Pixel index in input grid x dimension */ \
   int ixn;                      /* Pixel index in input grid (n-d) */ \
   int iy;                       /* Pixel index in input grid y dimension */ \
   int lo_x;                     /* Lower pixel index (x dimension) */ \
   int lo_y;                     /* Lower pixel index (y dimension) */ \
   int off_in;                   /* Offset to input pixel */ \
   int off_lo;                   /* Offset to "first" input pixel */ \
   int pixel;                    /* Offset to input pixel containing point */ \
   int point;                    /* Loop counter for output points */ \
   int result;                   /* Result value to return */ \
   int s;                        /* Temporary variable for strides */ \
   int usebad;                   /* Use "bad" input pixel values? */ \
   int usevar;                   /* Process variance array? */ \
   int ystride;                  /* Stride along input grid y dimension */ \
\
/* Initialise. */ \
   result = 0; \
\
/* Check the global error status. */ \
   if ( !astOK ) return result; \
\
/* Determine if we are processing bad pixels or variances. */ \
   usebad = flags & AST__USEBAD; \
   usevar = in_var && out_var; \
\
/* Handle the 1-dimensional case optimally. */ \
/* ---------------------------------------- */ \
   if ( ndim_in == 1 ) { \
\
/* Calculate the coordinate limits of the input grid. */ \
      xmin = ( (double) lbnd_in[ 0 ] ) - 0.5; \
      xmax = ( (double) ubnd_in[ 0 ] ) + 0.5; \
\
/* Loop through the list of output points. */ \
      for ( point = 0; point < npoint; point++ ) { \
\
/* Obtain the x coordinate of the current point and test if it lies \
   outside the input grid, or is bad. */ \
         x = coords[ 0 ][ point ]; \
         bad = ( x < xmin ) || ( x >= xmax ) || ( x == AST__BAD ); \
         if ( !bad ) { \
\
/* Obtain the offsets along the input grid x dimension of the input \
   pixel which contains the current coordinate. */ \
            ix = ( (int) floor( x + 0.5 ) ); \
\
/* Calculate this pixel's offset from the start of the input array. */ \
            pixel = ix - lbnd_in[ 0 ]; \
\
/* Test if the pixel is bad. If not, calculate the offset of the x \
   coordinate from the pixel's centre. */ \
            bad = ( in[ pixel ] == badval ) && usebad; \
            if ( !bad ) { \
               dx = x - (double) ix; \
\
/* Obtain the indices along the input grid x dimension of the two \
   adjacent pixels which will contribute to the interpolated result \
   (which pixels are involved depends on whether the offset from the \
   nearest pixel's centre is positive or negative). Also obtain the \
   fractional weight to be applied to each of these pixels. */ \
               if ( dx <= 0.0 ) { \
                  lo_x = ix - 1; \
                  hi_x = ix; \
                  frac_lo_x = -dx; \
                  frac_hi_x = 1.0 + dx; \
               } else { \
                  lo_x = ix; \
                  hi_x = ix + 1; \
                  frac_lo_x = 1.0 - dx; \
                  frac_hi_x = dx; \
               } \
\
/* Obtain the offset within the input array of the first pixel to be \
   used for interpolation (the one with the smaller index). */ \
               off_lo = lo_x - lbnd_in[ 0 ]; \
\
/* Initialise sums for forming the interpolated result. */ \
               sum = (Xfloattype) 0.0; \
               sum_var = (Xfloattype) 0.0; \
               wtsum = (Xfloattype) 0.0; \
               bad_var = !usevar; \
\
/* For each of the two pixels which may contribute to the result, \
   test if the pixel index lies within the input grid. Where it does, \
   accumulate the sums required for forming the interpolated \
   result. In each case, we supply the pixel's offset within the input \
   array and the weight to be applied to it. */ \
               if ( lo_x >= lbnd_in[ 0 ] ) { \
                  FORM_LINEAR_INTERPOLATION_SUM( off_lo, \
                                                 frac_lo_x, \
                                                 Xtype, Xfloattype ) \
               } \
               if ( hi_x <= ubnd_in[ 0 ] ) { \
                  FORM_LINEAR_INTERPOLATION_SUM( off_lo + 1, \
                                                 frac_hi_x, \
                                                 Xtype, Xfloattype ) \
               } \
            } \
         } \
\
/* Assign a bad output value (and variance) if required and count it. */ \
         if ( bad ) { \
            out[ offset[ point ] ] = badval; \
            if ( usevar ) out_var[ offset[ point ] ] = badval; \
            result++; \
\
/* Otherwise, calculate the interpolated value. If the output data \
   type is floating point, this result can then be stored directly, \
   otherwise we must round to the nearest integer. */ \
         } else { \
            val = sum / wtsum; \
            if ( Xfloating ) { \
               out[ offset[ point ] ] = (Xtype) val; \
            } else { \
               out[ offset[ point ] ] = (Xtype) ( val + \
                                        ( ( val >= (Xfloattype) 0.0 ) ? \
                                          ( (Xfloattype) 0.5 ) : \
                                          ( (Xfloattype) -0.5 ) ) ); \
            } \
\
/* If a variance estimate is required but none can be obtained, then \
   store a bad output variance value and count it. */ \
            if ( usevar ) { \
               if ( bad_var ) { \
                  out_var[ offset[ point ] ] = badval; \
                  result++; \
\
/* Otherwise, calculate the variance estimate and store it, rounding \
   to the nearest integer if necessary. */ \
               } else { \
                  val = sum_var / ( wtsum * wtsum ); \
                  if ( Xfloating ) { \
                     out_var[ offset[ point ] ] = (Xtype) val; \
                  } else { \
                     out_var[ offset[ point ] ] = (Xtype) ( val + \
                                           ( ( val >= (Xfloattype) 0.0 ) ? \
                                             ( (Xfloattype) 0.5 ) : \
                                             ( (Xfloattype) -0.5 ) ) ); \
                  } \
               } \
            } \
         } \
      } \
\
/* Handle the 2-dimensional case optimally. */ \
/* ---------------------------------------- */ \
   } else if ( ndim_in == 2 ) { \
\
/* Calculate the stride along the y dimension of the input grid. */ \
      ystride = ubnd_in[ 0 ] - lbnd_in[ 0 ] + 1; \
\
/* Calculate the coordinate limits of the input grid in each \
   dimension. */ \
      xmin = ( (double) lbnd_in[ 0 ] ) - 0.5; \
      xmax = ( (double) ubnd_in[ 0 ] ) + 0.5; \
      ymin = ( (double) lbnd_in[ 1 ] ) - 0.5; \
      ymax = ( (double) ubnd_in[ 1 ] ) + 0.5; \
\
/* Loop through the list of output points. */ \
      for ( point = 0; point < npoint; point++ ) { \
\
/* Obtain the x coordinate of the current point and test if it lies \
   outside the input grid, or is bad. */ \
         x = coords[ 0 ][ point ]; \
         bad = ( x < xmin ) || ( x >= xmax ) || ( x == AST__BAD ); \
         if ( !bad ) { \
\
/* If not, then similarly obtain and test the y coordinate. */ \
            y = coords[ 1 ][ point ]; \
            bad = ( y < ymin ) || ( y >= ymax ) || ( y == AST__BAD ); \
            if ( !bad ) { \
\
/* Obtain the offsets along each input grid dimension of the input \
   pixel which contains the current coordinates. */ \
               ix = ( (int) floor( x + 0.5 ) ); \
               iy = ( (int) floor( y + 0.5 ) ); \
\
/* Calculate this pixel's offset from the start of the input array. */ \
               pixel = ix - lbnd_in[ 0 ] + ystride * ( iy - lbnd_in[ 1 ] ); \
\
/* Test if the pixel is bad. If not, calculate the offset of the \
   current coordinates from the pixel's centre. */ \
               bad = ( in[ pixel ] == badval ) && usebad; \
               if ( !bad ) { \
                  dx = x - (double) ix; \
                  dy = y - (double) iy; \
\
/* Obtain the indices along the input grid x dimension of the two \
   adjacent pixels which will contribute to the interpolated result \
   (which pixels are involved depends on whether the offset from the \
   nearest pixel's centre is positive or negative). Also obtain the \
   fractional weight to be applied to each of these pixels. */ \
                  if ( dx <= 0.0 ) { \
                     lo_x = ix - 1; \
                     hi_x = ix; \
                     frac_lo_x = -dx; \
                     frac_hi_x = 1.0 + dx; \
                  } else { \
                     lo_x = ix; \
                     hi_x = ix + 1; \
                     frac_lo_x = 1.0 - dx; \
                     frac_hi_x = dx; \
                  } \
\
/* Repeat this process for the y dimension. */ \
                  if ( dy <= 0.0 ) { \
                     lo_y = iy - 1; \
                     hi_y = iy; \
                     frac_lo_y = -dy; \
                     frac_hi_y = 1.0 + dy; \
                  } else { \
                     lo_y = iy; \
                     hi_y = iy + 1; \
                     frac_lo_y = 1.0 - dy; \
                     frac_hi_y = dy; \
                  } \
\
/* Obtain the offset within the input array of the first pixel to be \
   used for interpolation (the one with the smaller index along both \
   dimensions). */ \
                  off_lo = lo_x - lbnd_in[ 0 ] + \
                           ystride * ( lo_y - lbnd_in[ 1 ] ); \
\
/* Initialise sums for forming the interpolated result. */ \
                  sum = (Xfloattype) 0.0; \
                  sum_var = (Xfloattype) 0.0; \
                  wtsum = (Xfloattype) 0.0; \
                  bad_var = !usevar; \
\
/* For each of the four pixels which may contribute to the result, \
   test if the pixel indices lie within the input grid. Where they do, \
   accumulate the sums required for forming the interpolated \
   result. In each case, we supply the pixel's offset within the input \
   array and the weight to be applied to it. */ \
                  if ( lo_y >= lbnd_in[ 1 ] ) { \
                     if ( lo_x >= lbnd_in[ 0 ] ) { \
                        FORM_LINEAR_INTERPOLATION_SUM( off_lo, \
                                                       frac_lo_x * frac_lo_y, \
                                                       Xtype, Xfloattype ) \
                     } \
                     if ( hi_x <= ubnd_in[ 0 ] ) { \
                        FORM_LINEAR_INTERPOLATION_SUM( off_lo + 1, \
                                                       frac_hi_x * frac_lo_y, \
                                                       Xtype, Xfloattype ) \
                     } \
                  } \
                  if ( hi_y <= ubnd_in[ 1 ] ) { \
                     if ( lo_x >= lbnd_in[ 0 ] ) { \
                        FORM_LINEAR_INTERPOLATION_SUM( off_lo + ystride, \
                                                       frac_lo_x * frac_hi_y, \
                                                       Xtype, Xfloattype ) \
                     } \
                     if ( hi_x <= ubnd_in[ 0 ] ) { \
                        FORM_LINEAR_INTERPOLATION_SUM( off_lo + ystride + 1, \
                                                       frac_hi_x * frac_hi_y, \
                                                       Xtype, Xfloattype ) \
                     } \
                  } \
               } \
            } \
         } \
\
/* Assign a bad output value (and variance) if required and count it. */ \
         if ( bad ) { \
            out[ offset[ point ] ] = badval; \
            if ( usevar ) out_var[ offset[ point ] ] = badval; \
            result++; \
\
/* Otherwise, calculate the interpolated value. If the output data \
   type is floating point, this result can then be stored directly, \
   otherwise we must round to the nearest integer. */ \
         } else { \
            val = sum / wtsum; \
            if ( Xfloating ) { \
               out[ offset[ point ] ] = (Xtype) val; \
            } else { \
               out[ offset[ point ] ] = (Xtype) ( val + \
                                        ( ( val >= (Xfloattype) 0.0 ) ? \
                                          ( (Xfloattype) 0.5 ) : \
                                          ( (Xfloattype) -0.5 ) ) ); \
            } \
\
/* If a variance estimate is required but none can be obtained, then \
   store a bad output variance value and count it. */ \
            if ( usevar ) { \
               if ( bad_var ) { \
                  out_var[ offset[ point ] ] = badval; \
                  result++; \
\
/* Otherwise, calculate the variance estimate and store it, rounding \
   to the nearest integer if necessary. */ \
               } else { \
                  val = sum_var / ( wtsum * wtsum ); \
                  if ( Xfloating ) { \
                     out_var[ offset[ point ] ] = (Xtype) val; \
                  } else { \
                     out_var[ offset[ point ] ] = (Xtype) ( val + \
                                           ( ( val >= (Xfloattype) 0.0 ) ? \
                                             ( (Xfloattype) 0.5 ) : \
                                             ( (Xfloattype) -0.5 ) ) ); \
                  } \
               } \
            } \
         } \
      } \
\
/* Handle other numbers of dimensions. */ \
/* ----------------------------------- */ \
   } else { \
\
/* Allocate workspace. */ \
      dim = astMalloc( sizeof( int ) * (size_t) ndim_in ); \
      frac_hi = astMalloc( sizeof( double ) * (size_t) ndim_in ); \
      frac_lo = astMalloc( sizeof( double ) * (size_t) ndim_in ); \
      hi = astMalloc( sizeof( int ) * (size_t) ndim_in ); \
      lo = astMalloc( sizeof( int ) * (size_t) ndim_in ); \
      stride = astMalloc( sizeof( int ) * (size_t) ndim_in ); \
      wt = astMalloc( sizeof( double ) * (size_t) ndim_in ); \
      xn_max = astMalloc( sizeof( double ) * (size_t) ndim_in ); \
      xn_min = astMalloc( sizeof( double ) * (size_t) ndim_in ); \
      if ( astOK ) { \
\
/* Calculate the stride along each dimension of the input grid. */ \
         for ( s = 1, idim = 0; idim < ndim_in; idim++ ) { \
            stride[ idim ] = s; \
            s *= ubnd_in[ idim ] - lbnd_in[ idim ] + 1; \
\
/* Calculate the coordinate limits of the input grid in each \
   dimension. */ \
            xn_min[ idim ] = ( (double) lbnd_in[ idim ] ) - 0.5; \
            xn_max[ idim ] = ( (double) ubnd_in[ idim ] ) + 0.5; \
         } \
\
/* Loop through the list of output points. */ \
         for ( point = 0; point < npoint; point++ ) { \
\
/* Initialise offsets into the input array. Then loop to obtain each
   coordinate associated with the current output point. */ \
            pixel = 0; \
            off_in = 0; \
            for ( idim = 0; idim < ndim_in; idim++ ) { \
               xn = coords[ idim ][ point ]; \
\
/* Test if the coordinate lies outside the input grid, or is bad. If \
   either is true, the corresponding output pixel value will be bad, \
   so give up on this point. */ \
               bad = ( xn < xn_min[ idim ] ) || ( xn >= xn_max[ idim ] ) || \
                     ( xn == AST__BAD ); \
               if ( bad ) break; \
\
/* Obtain the index along the current input grid dimension of the \
   pixel which contains this coordinate and calculate the offset of \
   the coordinate from the pixel's centre. */ \
               ixn = (int) floor( xn + 0.5 ); \
               dxn = xn - (double) ixn; \
\
/* Accumulate the pixel's offset from the start of the input array. */ \
               pixel += stride[ idim ] * ( ixn - lbnd_in[ idim ] ); \
\
/* Test if the coordinate lies below (or on) the pixel's centre. If it \
   does, then obtain the indices along the current dimension of the \
   input grid of the two (usually adjacent) pixels which will \
   contribute to the output value. If necessary, however, restrict the \
   pixel with the lower index to ensure it does not lie outside the \
   input grid. Also calculate the fractional weight to be given to \
   each pixel in order to interpolate linearly between them. */ \
               if ( dxn <= 0.0 ) { \
                  hi[ idim ] = ixn; \
                  frac_hi[ idim ] = 1.0 + dxn; \
                  if ( ixn > lbnd_in[ idim ] ) { \
                     lo[ idim ] = ixn - 1; \
                     frac_lo[ idim ] = -dxn; \
                  } else { \
                     lo[ idim ] = hi[ idim ]; \
                     frac_lo[ idim ] = frac_hi[ idim ]; \
                  } \
\
/* If the coordinate lies above the pixel's centre, repeat the above \
   process, this time restricting the pixel with the larger index if \
   necessary. */ \
               } else { \
                  lo[ idim ] = ixn; \
                  frac_lo[ idim ] = 1.0 - dxn; \
                  if ( ixn < ubnd_in[ idim ] ) { \
                     hi[ idim ] = ixn + 1; \
                     frac_hi[ idim ] = dxn; \
                  } else { \
                     hi[ idim ] = lo[ idim ]; \
                     frac_hi[ idim ] = frac_lo[ idim ]; \
                  } \
               } \
\
/* Store the lower index involved in interpolation along each \
   dimension and accumulate the offset from the start of the input \
   array of the pixel which has these indices. */ \
\
               dim[ idim ] = lo[ idim ]; \
               off_in += stride[ idim ] * ( lo[ idim ] - lbnd_in[ idim ] ); \
\
/* Also store the fractional weight associated with the lower pixel \
   along each dimension. */ \
               wt[ idim ] = frac_lo[ idim ]; \
            } \
\
/* Once the input pixel which contains the required coordinates has \
   been identified, test if it is bad. */ \
            bad = bad || ( ( in[ pixel ] == badval ) && usebad ); \
\
/* Assign a bad output value (and variance) if required and count it. */ \
            if ( bad ) { \
               out[ offset[ point ] ] = badval; \
               if ( usevar ) out_var[ offset[ point ] ] = badval; \
               result++; \
\
/* Otherwise, initialise and loop over adjacent input pixels to obtain \
   an interpolated value. */ \
            } else { \
               sum = (Xfloattype) 0.0; \
               sum_var = (Xfloattype) 0.0; \
               wtsum = (Xfloattype) 0.0; \
               bad_var = !usevar; \
               done = 0; \
               while ( !done ) { \
\
/* Test each pixel which may contribute to the result to see if it is \
   bad. If not, calculate its weight by multiplying the relevant \
   weight factors to account for the displacement of its centre from \
   the required position along each dimension. */ \
                  if ( ( in[ off_in ] != badval ) || !usebad ) { \
                     for ( pixwt = 1.0, idim = 0; idim < ndim_in; idim++ ) { \
                        pixwt *= wt[ idim ]; \
                     } \
\
/* Form the weighted sums required for finding the interpolated \
   value. */ \
                     sum += ( (Xfloattype) pixwt ) * \
                            ( (Xfloattype) in[ off_in ] ); \
                     wtsum += (Xfloattype) pixwt; \
\
/* If a variance estimate is required and it still seems possible to \
   obtain one, then obtain the variance value associated with the \
   current input pixel. */ \
                     if ( !bad_var ) { \
                        var = in_var[ off_in ]; \
\
/* Test if this value is bad (if the data type is signed, also check \
   that it is not negative). */ \
                        bad_var = ( var == badval ) && usebad; \
                        CHECK_FOR_NEGATIVE_VARIANCE(Xtype) \
\
/* If any bad input variance value is obtained, we cannot generate a \
   valid output variance estimate. Otherwise, form the sum needed to \
   calculate this estimate. */ \
                        if ( !bad_var ) { \
                           sum_var += ( (Xfloattype) ( pixwt * pixwt ) ) * \
                                      ( (Xfloattype) in_var[ off_in ] ); \
                        } \
                     } \
                  } \
\
/* Now update the indices, offset and weight factors to refer to the \
   next input pixel to be considered. */ \
                  idim = 0; \
                  do { \
\
/* The first input dimension which still refers to the pixel with the \
   lower of the two possible indices is switched to refer to the other \
   pixel (with the higher index). The offset into the input array and \
   the fractional weight factor for this dimension are also updated \
   accordingly. */ \
                     if ( dim[ idim ] != hi[ idim ] ) { \
                        dim[ idim ] = hi[ idim ]; \
                        off_in += stride[ idim ]; \
                        wt[ idim ] = frac_hi[ idim ]; \
                        break; \
\
/* Any earlier dimensions (referring to the higher index) are switched \
   back to the lower index, if not already there, before going on to \
   consider the next dimension. (This process is the same as \
   incrementing a binary number and propagating overflows up through \
   successive digits, except that dimension where the "lo" and "hi" \
   values are the same can only take one value.) The process stops at \
   the first attempt to return the final dimension to the lower \
   index. */ \
                     } else { \
                        if ( dim[ idim ] != lo[ idim ] ) { \
                           dim[ idim ] = lo[ idim ]; \
                           off_in -= stride[ idim ]; \
                           wt[ idim ] = frac_lo[ idim ]; \
                        } \
                        done = ( ++idim == ndim_in ); \
                     } \
                  } while ( !done ); \
               } \
\
/* Once the required sums over adjacent input pixels have been formed, \
   calculate the interpolated value. If the output data type is \
   floating point, this result can then be stored directly, otherwise \
   we must round to the nearest integer. */ \
               val = sum / wtsum; \
               if ( Xfloating ) { \
                  out[ offset[ point ] ] = (Xtype) val; \
               } else { \
                  out[ offset[ point ] ] = (Xtype) ( val + \
                                           ( ( val >= (Xfloattype) 0.0 ) ? \
                                             ( (Xfloattype) 0.5 ) : \
                                             ( (Xfloattype) -0.5 ) ) ); \
               } \
\
/* If a variance estimate is required but none can be obtained, then \
   store a bad output variance value and count it. */ \
               if ( usevar ) { \
                  if ( bad_var ) { \
                     out_var[ offset[ point ] ] = badval; \
                     result++; \
\
/* Otherwise, calculate the variance estimate and store it, rounding \
   to the nearest integer if necessary. */ \
                  } else { \
                     val = sum_var / ( wtsum * wtsum ); \
                     if ( Xfloating ) { \
                        out_var[ offset[ point ] ] = (Xtype) val; \
                     } else { \
                        out_var[ offset[ point ] ] = (Xtype) ( val + \
                                              ( ( val >= (Xfloattype) 0.0 ) ? \
                                                ( (Xfloattype) 0.5 ) : \
                                                ( (Xfloattype) -0.5 ) ) ); \
                     } \
                  } \
               } \
            } \
         } \
      } \
\
/* Free the workspace. */ \
      dim = astFree( dim ); \
      frac_hi = astFree( frac_hi ); \
      frac_lo = astFree( frac_lo ); \
      hi = astFree( hi ); \
      lo = astFree( lo ); \
      stride = astFree( stride ); \
      wt = astFree( wt ); \
      xn_max = astFree( xn_max ); \
      xn_min = astFree( xn_min ); \
   } \
\
/* If an error has occurred, clear the returned result. */ \
   if ( !astOK ) result = 0; \
\
/* Return the result. */ \
   return result; \
}

/* This subsidiary macro adds the contribution from a specified input
   pixel to the accumulated sums for forming the linearly interpolated
   value in the macro above. */
#define FORM_LINEAR_INTERPOLATION_SUM(off,wt,Xtype,Xfloattype) \
\
/* Obtain the offset of the input pixel to use. */ \
   off_in = (off); \
\
/* Test if this pixel is bad. If not, then obtain the weight to \
   apply to it. */ \
   if ( ( in[ off_in ] != badval ) || !usebad ) { \
      pixwt = (wt); \
\
/* Increment the weighted sum of pixel values and the sum of weights. */ \
      sum += ( (Xfloattype) in[ off_in ] ) * ( (Xfloattype) pixwt ); \
      wtsum += (Xfloattype) pixwt; \
\
/* If an output variance estimate is to be generated, obtain the input \
   variance value. */ \
      if ( !bad_var ) { \
         var = in_var[ off_in ]; \
\
/* Test if the variance value is bad (if the data type is signed, also \
   check that it is not negative). */ \
         bad_var = ( var == badval ) && usebad; \
         CHECK_FOR_NEGATIVE_VARIANCE(Xtype) \
\
/* If OK, increment the weighted sum of variance values. */ \
         if ( !bad_var ) { \
            sum_var += ( (Xfloattype) ( pixwt * pixwt ) ) * \
                       ( (Xfloattype) in_var[ off_in ] ); \
         } \
      } \
   }

/* This second subsidiary macro tests for negative variance values in
   the macros above. This check is required only for signed data
   types. */
#define CHECK_FOR_NEGATIVE_VARIANCE(Xtype) \
        bad_var = bad_var || ( var < ( (Xtype) 0 ) );

/* Expand the main macro above to generate a function for each
   required signed data type. */
#if defined(AST_LONG_DOUBLE)     /* Not normally implemented */
MAKE_INTERPOLATE_LINEAR(LD,long double,1,long double)
MAKE_INTERPOLATE_LINEAR(L,long int,0,long double)
#else
MAKE_INTERPOLATE_LINEAR(L,long int,0,double)
#endif     
MAKE_INTERPOLATE_LINEAR(D,double,1,double)
MAKE_INTERPOLATE_LINEAR(F,float,1,float)
MAKE_INTERPOLATE_LINEAR(I,int,0,double)
MAKE_INTERPOLATE_LINEAR(S,short int,0,float)
MAKE_INTERPOLATE_LINEAR(B,signed char,0,float)

/* Re-define the macro for testing for negative variances to do
   nothing. */
#undef CHECK_FOR_NEGATIVE_VARIANCE
#define CHECK_FOR_NEGATIVE_VARIANCE(Xtype)

/* Expand the main macro above to generate a function for each
   required unsigned data type. */
#if defined(AST_LONG_DOUBLE)     /* Not normally implemented */
MAKE_INTERPOLATE_LINEAR(UL,unsigned long int,0,long double)
#else
MAKE_INTERPOLATE_LINEAR(UL,unsigned long int,0,double)
#endif     
MAKE_INTERPOLATE_LINEAR(UI,unsigned int,0,double)
MAKE_INTERPOLATE_LINEAR(US,unsigned short int,0,float)
MAKE_INTERPOLATE_LINEAR(UB,unsigned char,0,float)

/* Undefine the macros. */
#undef CHECK_FOR_NEGATIVE_VARIANCE
#undef FORM_LINEAR_INTERPOLATION_SUM
#undef MAKE_INTERPOLATE_LINEAR

/*
*  Name:
*     InterpolateNearest<X>

*  Purpose:
*     Resample a data grid, using the nearest-pixel interpolation scheme.

*  Type:
*     Private function.

*  Synopsis:
*     #include "mapping.h"
*     int InterpolateNearest<X>( int ndim_in,
*                                const int *lbnd_in, const int *ubnd_in,
*                                const <Xtype> *in, const <Xtype> *in_var,
*                                int npoint, const int *offset,
*                                const double *const *coords,
*                                int flags, <Xtype> badval,
*                                <Xtype> *out, <Xtype> *out_var )

*  Class Membership:
*     Mapping member function.

*  Description:
*     This is a set of functions which resample a rectangular input
*     grid of data (and, optionally, associated statistical variance
*     values) so as to place them into a new output grid. Each output
*     grid point may be mapped on to a position in the input grid in
*     an arbitrary way. Where the positions given do not correspond
*     with a pixel centre in the input grid, the interpolation scheme
*     used is simply to select the nearest pixel (i.e. the one whose
*     bounds contain the supplied position). The input and output
*     grids may have any number of dimensions, not necessarily equal.

*  Parameters:
*     ndim_in
*        The number of dimensions in the input grid. This should be at
*        least one.
*     lbnd_in
*        Pointer to an array of integers, with "ndim_in" elements.
*        This should give the coordinates of the centre of the first
*        pixel in the input grid along each dimension.
*     ubnd_in
*        Pointer to an array of integers, with "ndim_in" elements.
*        This should give the coordinates of the centre of the last
*        pixel in the input grid along each dimension.
*
*        Note that "lbnd_in" and "ubnd_in" together define the shape
*        and size of the input grid, its extent along a particular
*        (i'th) dimension being ubnd_in[i]-lbnd_in[i]+1 (assuming "i"
*        is zero-based). They also define the input grid's coordinate
*        system, with each pixel being of unit extent along each
*        dimension with integral coordinate values at its centre.
*     in
*        Pointer to the array of data to be resampled (with an element
*        for each pixel in the input grid). The numerical type of
*        these data should match the function used, as given by the
*        suffix on the function name. The storage order should be such
*        that the index of the first grid dimension varies most
*        rapidly and that of the final dimension least rapidly
*        (i.e. Fortran array storage order).
*     in_var
*        An optional pointer to a second array of positive numerical
*        values (with the same size and type as the "in" array), which
*        represent estimates of the statistical variance associated
*        with each element of the "in" array. If this second array is
*        given (along with the corresponding "out_var" array), then
*        estimates of the variance of the resampled data will also be
*        returned.
*
*        If no variance estimates are required, a NULL pointer should
*        be given.
*     npoint
*        The number of points at which the input grid is to be
*        resampled.
*     offset
*        Pointer to an array of integers with "npoint" elements. For
*        each output point, this array should contain the zero-based
*        offset in the output array(s) (i.e. the "out" and,
*        optionally, the "out_var" arrays) at which the resampled
*        output value(s) should be stored.
*     coords
*        An array of pointers to double, with "ndim_in"
*        elements. Element "coords[coord]" should point at the first
*        element of an array of double (with "npoint" elements) which
*        contains the values of coordinate number "coord" for each
*        interpolation point. The value of coordinate number "coord"
*        for interpolation point number "point" is therefore given by
*        "coords[coord][point]" (assuming both indices are
*        zero-based).  If any point has a coordinate value of AST__BAD
*        associated with it, then the corresponding output data (and
*        variance) will be set to the value given by "badval".
*     flags
*        The bitwise OR of a set of flag values which control the
*        operation of the function. Currently, only the flag
*        AST__USEBAD is significant and indicates whether there are
*        "bad" (i.e. missing) data in the input array(s) which must be
*        recognised and propagated to the output array(s).  If this
*        flag is not set, all input values are treated literally.
*     badval
*        If the AST__USEBAD flag is set in the "flags" value (above),
*        this parameter specifies the value which is used to identify
*        bad data and/or variance values in the input array(s). Its
*        numerical type must match that of the "in" (and "in_var")
*        arrays. The same value will also be used to flag any output
*        array elements for which resampled values could not be
*        obtained.  The output arrays(s) may be flagged with this
*        value whether or not the AST__USEBAD flag is set (the
*        function return value indicates whether any such values have
*        been produced).
*     out
*        Pointer to an array with the same data type as the "in"
*        array, into which the resampled data will be returned. Note
*        that details of how the output grid maps on to this array
*        (e.g. the storage order, number of dimensions, etc.) is
*        arbitrary and is specified entirely by means of the "offset"
*        array. The "out" array should therefore contain sufficient
*        elements to accommodate the "offset" values supplied.  There
*        is no requirement that all elements of the "out" array should
*        be assigned values, and any which are not addressed by the
*        contents of the "offset" array will be left unchanged.
*     out_var
*        An optional pointer to an array with the same data type and
*        size as the "out" array, into which variance estimates for
*        the resampled values may be returned. This array will only be
*        used if the "in_var" array has been given. It is addressed in
*        exactly the same way (via the "offset" array) as the "out"
*        array. The values returned are estimates of the statistical
*        variance of the corresponding values in the "out" array, on
*        the assumption that all errors in input grid values (in the
*        "in" array) are statistically independent and that their
*        variance estimates (in the "in_var" array) may simply be
*        summed (with appropriate weighting factors).
*
*        If no output variance estimates are required, a NULL pointer
*        should be given.

*  Returned Value:
*     The number of output grid points to which a data value (or a
*     variance value if relevant) equal to "badval" has been assigned
*     because no valid output value could be obtained.

*  Notes:
*     - There is a separate function for each numerical type of
*     gridded data, distinguished by replacing the <X> in the function
*     name by the appropriate 1- or 2-character suffix.
*     - A value of zero will be returned if any of these functions is
*     invoked with the global error status set, or if it should fail
*     for any reason.
*/
/* Define a macro to implement the function for a specific data
   type. */
#define MAKE_INTERPOLATE_NEAREST(X,Xtype) \
static int InterpolateNearest##X( int ndim_in, \
                                  const int *lbnd_in, const int *ubnd_in, \
                                  const Xtype *in, const Xtype *in_var, \
                                  int npoint, const int *offset, \
                                  const double *const *coords, \
                                  int flags, Xtype badval, \
                                  Xtype *out, Xtype *out_var ) { \
\
/* Local Variables: */ \
   Xtype var;                    /* Variance value */ \
   double *xn_max;               /* Pointer to upper limits array (n-d) */ \
   double *xn_min;               /* Pointer to lower limits array (n-d) */ \
   double x;                     /* x coordinate value */ \
   double xmax;                  /* x upper limit */ \
   double xmin;                  /* x lower limit */ \
   double xn;                    /* Coordinate value (n-d) */ \
   double y;                     /* y coordinate value */ \
   double ymax;                  /* y upper limit */ \
   double ymin;                  /* y lower limit */ \
   int *stride;                  /* Pointer to array of dimension strides */ \
   int bad;                      /* Output pixel bad? */ \
   int idim;                     /* Loop counter for dimensions */ \
   int ix;                       /* Number of pixels offset in x direction */ \
   int ixn;                      /* Number of pixels offset (n-d) */ \
   int iy;                       /* Number of pixels offset in y direction */ \
   int off_in;                   /* Pixel offset into input array */ \
   int point;                    /* Loop counter for output points */ \
   int result;                   /* Returned result value */ \
   int s;                        /* Temporary variable for strides */ \
   int usebad;                   /* Use "bad" input pixel values? */ \
   int usevar;                   /* Process variance array? */ \
   int ystride;                  /* Stride along input grid y direction */ \
\
/* Initialise. */ \
   result = 0; \
\
/* Check the global error status. */ \
   if ( !astOK ) return result; \
\
/* Determine if we are processing bad pixels or variances. */ \
   usebad = flags & AST__USEBAD; \
   usevar = in_var && out_var; \
\
/* Handle the 1-dimensional case optimally. */ \
/* ---------------------------------------- */ \
   if ( ndim_in == 1 ) { \
\
/* Calculate the coordinate limits of the input array. */ \
      xmin = ( (double) lbnd_in[ 0 ] ) - 0.5; \
      xmax = ( (double) ubnd_in[ 0 ] ) + 0.5; \
\
/* Loop through the list of output points. */ \
      for ( point = 0; point < npoint; point++ ) { \
\
/* Obtain the x coordinate of the current point and test if it lies \
   outside the input grid, or is bad. */ \
         x = coords[ 0 ][ point ]; \
         bad = ( x < xmin ) || ( x >= xmax ) || ( x == AST__BAD ); \
         if ( !bad ) { \
\
/* If not, then obtain the offset within the input grid of the pixel \
   which contains the current point. */ \
            off_in = ( (int) floor( x + 0.5 ) ) - lbnd_in[ 0 ]; \
\
/* Test if the input pixel is bad. */ \
            bad = ( in[ off_in ] == badval ) && usebad; \
         } \
\
/* If the output pixel is not bad, obtain its value from the input \
   grid. */ \
         if ( !bad ) { \
            out[ offset[ point ] ] = in[ off_in ]; \
\
/* If required, obtain the associated variance value and test if it is \
   bad (if the data type is signed, also check that it is not \
   negative). */ \
            if ( usevar ) { \
               var = in_var[ off_in ]; \
               bad = ( var == badval ) && usebad; \
               CHECK_FOR_NEGATIVE_VARIANCE(Xtype) \
\
/* If the variance value is not bad, store it in the output \
   array. Otherwise, store a bad value and count it. */ \
               if ( !bad ) { \
                  out_var[ offset[ point ] ] = var; \
               } else { \
                  out_var[ offset[ point ] ] = badval; \
                  result++; \
               } \
            } \
\
/* If the input data value was bad, assign a bad output value (and \
   variance, if required) and count it. */ \
         } else { \
            out[ offset[ point ] ] = badval; \
            if ( usevar ) out_var[ offset[ point ] ] = badval; \
            result++; \
         } \
      } \
\
/* Handle the 2-dimensional case optimally. */ \
/* ---------------------------------------- */ \
   } else if ( ndim_in == 2 ) { \
\
/* Calculate the stride along the y dimension of the input grid. */ \
      ystride = ubnd_in[ 0 ] - lbnd_in[ 0 ] + 1; \
\
/* Calculate the coordinate limits of the input array in each \
   dimension. */ \
      xmin = ( (double) lbnd_in[ 0 ] ) - 0.5; \
      xmax = ( (double) ubnd_in[ 0 ] ) + 0.5; \
      ymin = ( (double) lbnd_in[ 1 ] ) - 0.5; \
      ymax = ( (double) ubnd_in[ 1 ] ) + 0.5; \
\
/* Loop through the list of output points. */ \
      for ( point = 0; point < npoint; point++ ) { \
\
/* Obtain the x coordinate of the current point and test if it lies \
   outside the input grid, or is bad. */ \
         x = coords[ 0 ][ point ]; \
         bad = ( x < xmin ) || ( x >= xmax ) || ( x == AST__BAD ); \
         if ( !bad ) { \
\
/* If not, then similarly obtain and test the y coordinate. */ \
            y = coords[ 1 ][ point ]; \
            bad = ( y < ymin ) || ( y >= ymax ) || ( y == AST__BAD ); \
            if ( !bad ) { \
\
/* Obtain the offsets along each input grid dimension of the input \
   pixel which contains the current point. */ \
               ix = ( (int) floor( x + 0.5 ) ) - lbnd_in[ 0 ]; \
               iy = ( (int) floor( y + 0.5 ) ) - lbnd_in[ 1 ]; \
\
/* Calculate this pixel's offset from the start of the input array. */ \
               off_in = ix + ystride * iy; \
\
/* Test if the input pixel is bad. */ \
               bad = ( in[ off_in ] == badval ) && usebad; \
            } \
         } \
\
/* If the output pixel is not bad, obtain its value from the input \
   grid. */ \
         if ( !bad ) { \
            out[ offset[ point ] ] = in[ off_in ]; \
\
/* If required, obtain the associated variance value and test if it is \
   bad (if the data type is signed, also check that it is not \
   negative). */ \
            if ( usevar ) { \
               var = in_var[ off_in ]; \
               bad = ( var == badval ) && usebad; \
               CHECK_FOR_NEGATIVE_VARIANCE(Xtype) \
\
/* If the variance value is not bad, store it in the output \
   array. Otherwise, store a bad value and count it. */ \
               if ( !bad ) { \
                  out_var[ offset[ point ] ] = var; \
               } else { \
                  out_var[ offset[ point ] ] = badval; \
                  result++; \
               } \
            } \
\
/* If the input data value was bad, assign a bad output value (and \
   variance, if required) and count it. */ \
         } else { \
            out[ offset[ point ] ] = badval; \
            if ( usevar ) out_var[ offset[ point ] ] = badval; \
            result++; \
         } \
      } \
\
/* Handle other numbers of dimensions. */ \
/* ----------------------------------- */ \
   } else { \
\
/* Allocate workspace. */ \
      stride = astMalloc( sizeof( int ) * (size_t) ndim_in ); \
      xn_max = astMalloc( sizeof( double ) * (size_t) ndim_in ); \
      xn_min = astMalloc( sizeof( double ) * (size_t) ndim_in ); \
      if ( astOK ) { \
\
/* Calculate the stride along each dimension of the input grid. */ \
         for ( s = 1, idim = 0; idim < ndim_in; idim++ ) { \
            stride[ idim ] = s; \
            s *= ubnd_in[ idim ] - lbnd_in[ idim ] + 1; \
\
/* Calculate the coordinate limits of the input grid in each \
   dimension. */ \
            xn_min[ idim ] = ( (double) lbnd_in[ idim ] ) - 0.5; \
            xn_max[ idim ] = ( (double) ubnd_in[ idim ] ) + 0.5; \
         } \
\
/* Loop through the list of output points. */ \
         for ( point = 0; point < npoint; point++ ) { \
\
/* Initialise the offset into the input array. Then loop to obtain \
   each coordinate associated with the current output point. */ \
            off_in = 0; \
            for ( idim = 0; idim < ndim_in; idim++ ) { \
               xn = coords[ idim ][ point ]; \
\
/* Test if the coordinate lies outside the input grid, or is bad. If \
   either is true, the corresponding output pixel value will be bad, \
   so give up on this point. */ \
               bad = ( xn < xn_min[ idim ] ) || ( xn >= xn_max[ idim ] ) || \
                     ( xn == AST__BAD ); \
               if ( bad ) break; \
\
/* Obtain the offset along the current input grid dimension of the \
   input pixel which contains the current point. */ \
               ixn = ( (int) floor( xn + 0.5 ) ) - lbnd_in[ idim ]; \
\
/* Accumulate this pixel's offset from the start of the input \
   array. */ \
               off_in += ixn * stride[ idim ]; \
            } \
\
/* Once the required input pixel has been located, test if it is \
   bad. */ \
            bad = bad || ( ( in[ off_in ] == badval ) && usebad ); \
\
/* If the output pixel is not bad, obtain its value from the input \
   grid. */ \
            if ( !bad ) { \
               out[ offset[ point ] ] = in[ off_in ]; \
\
/* If required, obtain the associated variance value and test if it is \
   bad (if the data type is signed, also check that it is not \
   negative). */ \
               if ( usevar ) { \
                  var = in_var[ off_in ]; \
                  bad = ( var == badval ) && usebad; \
                  CHECK_FOR_NEGATIVE_VARIANCE(Xtype) \
\
/* If the variance value is not bad, store it in the output \
   array. Otherwise, store a bad value and count it. */ \
                  if ( !bad ) { \
                     out_var[ offset[ point ] ] = var; \
                  } else { \
                     out_var[ offset[ point ] ] = badval; \
                     result++; \
                  } \
               } \
\
/* If the input data value was bad, assign a bad output value (and \
   variance, if required) and count it. */ \
            } else { \
               out[ offset[ point ] ] = badval; \
               if ( usevar ) out_var[ offset[ point ] ] = badval; \
               result++; \
            } \
         } \
      } \
\
/* Free workspace. */ \
      stride = astFree( stride ); \
      xn_max = astFree( xn_max ); \
      xn_min = astFree( xn_min ); \
   } \
\
/* If an error has occurred, clear the returned result. */ \
   if ( !astOK ) result = 0; \
\
/* Return the result. */ \
   return result; \
}

/* This subsidiary macro tests for negative variance values in the
   macro above. This check is required only for signed data types. */
#define CHECK_FOR_NEGATIVE_VARIANCE(Xtype) \
        bad = bad || ( var < ( (Xtype) 0 ) );

/* Expand the main macro above to generate a function for each
   required signed data type. */
#if defined(AST_LONG_DOUBLE)     /* Not normally implemented */
MAKE_INTERPOLATE_NEAREST(LD,long double)
#endif
MAKE_INTERPOLATE_NEAREST(D,double)
MAKE_INTERPOLATE_NEAREST(F,float)
MAKE_INTERPOLATE_NEAREST(L,long int)
MAKE_INTERPOLATE_NEAREST(I,int)
MAKE_INTERPOLATE_NEAREST(S,short int)
MAKE_INTERPOLATE_NEAREST(B,signed char)

/* Re-define the macro for testing for negative variances to do
   nothing. */
#undef CHECK_FOR_NEGATIVE_VARIANCE
#define CHECK_FOR_NEGATIVE_VARIANCE(Xtype)

/* Expand the main macro above to generate a function for each
   required unsigned data type. */
MAKE_INTERPOLATE_NEAREST(UL,unsigned long int)
MAKE_INTERPOLATE_NEAREST(UI,unsigned int)
MAKE_INTERPOLATE_NEAREST(US,unsigned short int)
MAKE_INTERPOLATE_NEAREST(UB,unsigned char)

/* Undefine the macros. */
#undef CHECK_FOR_NEGATIVE_VARIANCE
#undef MAKE_INTERPOLATE_NEAREST

static void Invert( AstMapping *this ) {
/*
*++
*  Name:
c     astInvert
f     AST_INVERT

*  Purpose:
*     Invert a Mapping.

*  Type:
*     Public virtual function.

*  Synopsis:
c     #include "mapping.h"
c     void astInvert( AstMapping *this )
f     CALL AST_INVERT( THIS, STATUS )

*  Class Membership:
*     Mapping method.

*  Description:
c     This function inverts a Mapping by reversing the boolean sense
f     This routine inverts a Mapping by reversing the boolean sense
*     of its Invert attribute. If this attribute is zero (the
*     default), the Mapping will transform coordinates in the way
*     specified when it was created. If it is non-zero, the input and
*     output coordinates will be inter-changed so that the direction
*     of the Mapping is reversed. This will cause it to display the
*     inverse of its original behaviour.

*  Parameters:
c     this
f     THIS = INTEGER (Given)
*        Pointer to the Mapping.
f     STATUS = INTEGER (Given and Returned)
f        The global status.
*--
*/

/* Local Variables: */
   int invert;                   /* New Invert attribute value */

/* Check the global error status. */
   if ( !astOK ) return;

/* Determine the new Invert attribute value. */
   invert = !astGetInvert( this );

/* Clear the old value. */
   astClearInvert( this );

/* If the resulting default value is not the one required, then set a
   new value explicitly. */
   if ( astGetInvert( this ) != invert ) astSetInvert( this, invert );
}

static double *LinearApprox( AstMapping *this, int ndim_in, int ndim_out,
                             const int *lbnd, const int *ubnd, double tol ) {
/*
*  Name:
*     LinearApprox

*  Purpose:
*     Obtain a linear approximation to a Mapping, if appropriate.

*  Type:
*     Private function.

*  Synopsis:
*     #include "mapping.h"
*     double *LinearApprox( AstMapping *this, int ndim_in, int ndim_out,
*                           const int *lbnd, const int *ubnd, double tol )

*  Class Membership:
*     Mapping member function.

*  Description:
*     This function tests the inverse coordinate transformation
*     implemented by a Mapping over a range of output coordinates
*     spanning a grid into which resampled values are to be placed. If
*     the transformation is found to be linear to a specified level of
*     accuracy, then an array of fit coefficients is returned. These
*     may be used to implement a linear approximation to the Mapping's
*     inverse transformation within the specified range of output
*     coordinates. If the transformation is not sufficiently linear,
*     no coefficients are returned.

*  Parameters:
*     this
*        Pointer to the Mapping, whose inverse transformation must be
*        available.
*     ndim_in
*        The number of input dimensions for the Mapping. This should
*        equal the Mapping's Nin attribute and should be at least 1.
*     ndim_out
*        The number of output dimensions for the Mapping. This should
*        equal the Mapping's Nout attribute and should be at least 1.
*     lbnd
*        Pointer to an array of integers with "ndim_out"
*        elements. These should specify the lower bounds of the output
*        grid and hence the lower bounds of the region of output
*        coordinate space over which linearity is required.
*     ubnd
*        Pointer to an array of integers with "ndim_out"
*        elements. These should specify the upper bounds of the output
*        grid and hence the upper bounds of the region of output
*        coordinate space over which linearity is required.
*     tol
*        The maximum permitted deviation from linearity, expressed as
*        a positive Cartesian displacement in the input coordinate
*        space. If a linear fit to the Mapping's inverse
*        transformation deviates from the true transformation by more
*        than this amount at any point which is tested, then no fit
*        coefficients will be returned.

*  Returned Value:
*     If the inverse transformation is sufficiently linear, the
*     function returns a pointer to a dynamically allocated double
*     array of fit coefficients. This should be freed by the caller
*     (e.g. using astFree) when no longer required.
*
*     If the transformation is not sufficiently linear, then a NULL
*     pointer is returned.

*  Notes:
*     - A NULL pointer will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*/

/* Local Variables: */
   AstPointSet *pset_in_f;       /* PointSet for input fitting points */
   AstPointSet *pset_in_t;       /* PointSet for input test points */
   AstPointSet *pset_out_f;      /* PointSet for output fitting points */
   AstPointSet *pset_out_t;      /* PointSet for output test points */
   double **ptr_in_f;            /* Input coordinate array pointers */
   double **ptr_in_t;            /* Input coordinate array pointers */
   double **ptr_out_f;           /* Output coordinate array pointers */
   double **ptr_out_t;           /* Output coordinate array pointers */
   double *fit;                  /* Pointer to array of fit coefficients */
   double *grad;                 /* Pointer to matrix of gradients */
   double *zero;                 /* Pointer to array of zero point values */
   double diff;                  /* Difference in coordinate values */
   double err;                   /* Sum of squared error */
   double frac;                  /* Fraction of output coordinate range */
   double in1;                   /* Input coordinate value */
   double in2;                   /* Input coordinate value */
   double out1;                  /* Output coordinate value */
   double out2;                  /* Output coordinate value */
   double outdiff;               /* Difference in output coordinate values */
   double x0;                    /* Coordinate of grid centre */
   double y;                     /* Input coordinate (transformed) */
   double yfit;                  /* Coordinate resulting from fit */
   double z;                     /* Sum for calculating zero points */
   int *vertex;                  /* Pointer to flag array for vertices */
   int coord_in;                 /* Loop counter for input coordinates. */
   int coord_out;                /* Loop counter for output coordinates */
   int done;                     /* All vertices visited? */
   int face1;                    /* Index of first face coordinates */
   int face2;                    /* Index of second face coordinates */
   int face;                     /* Loop counter for faces */
   int ii;                       /* Index into gradient matrix */
   int linear;                   /* Mapping is linear? */
   int npoint;                   /* Number of test points required */
   int point;                    /* Counter for points */

/* Initialise. */
   fit = NULL;
   
/* Check the global error status. */
   if ( !astOK ) return fit;

/* Further initialisation. */
   linear = 1;

/* Allocate space for the fit coefficients. */
   fit = astMalloc( sizeof( double ) *
                    (size_t) ( ( ndim_in + 1 ) * ndim_out ) );
      
/* Create a PointSet to hold output coordinates and obtain a pointer
   to its coordinate arrays. */
   pset_out_f = astPointSet( 2 * ndim_out, ndim_out, "" );
   ptr_out_f = astGetPoints( pset_out_f );
   if ( astOK ) {

/* Set up and transform an initial set of points. */
/* ---------------------------------------------- */
/* Loop to set up output coordinates at the centre of each face of the
   output grid, storing them in the PointSet created above. */
      point = 0;
      for ( face = 0; face < ( 2 * ndim_out ); face++ ) {
         for ( coord_out = 0; coord_out < ndim_out; coord_out++ ) {
            ptr_out_f[ coord_out ][ point ] =
               0.5 * (double) ( lbnd[ coord_out ] + ubnd[ coord_out ] );
         }
         ptr_out_f[ face / 2 ][ point ] = ( face % 2 ) ?
                                          ubnd[ face / 2 ] : lbnd[ face / 2 ];
         point++;
      }
   }

/* Transform these coordinates into the input grid's coordinate system
   and obtain an array of pointers to the resulting coordinate
   data. */
   pset_in_f = astTransform( this, pset_out_f, 0, NULL );
   ptr_in_f = astGetPoints( pset_in_f );
   if ( astOK ) {

/* Fit a linear approximation to the points. */
/* ----------------------------------------- */
/* Obtain pointers to the locations in the fit coefficients array
   where the gradients and zero points should be stored. */
      grad = fit + ndim_out;
      zero = fit;

/* On the assumption that the transformation applied above is
   approximately linear, loop to determine the matrix of gradients and
   the zero points which describe it. */
      ii = 0;
      for ( coord_in = 0; coord_in < ndim_in; coord_in++ ) {
         z = 0.0;
         for ( coord_out = 0; coord_out < ndim_out; coord_out++ ) {

/* Find the indices of opposite faces in each output dimension. */
            face1 = 2 * coord_out;
            face2 = face1 + 1;

/* Obtain the input and output coordinates at these face centres. */
            out1 = ptr_out_f[ coord_out ][ face1 ];
            out2 = ptr_out_f[ coord_out ][ face2 ];
            in1 = ptr_in_f[ coord_in ][ face1 ];
            in2 = ptr_in_f[ coord_in ][ face2 ];

/* Check whether any transformed coordinates are bad. If so, the
   transformation cannot be linear, so give up trying to fit it. */
            if ( ( in1 == AST__BAD ) || ( in2 == AST__BAD ) ) {
               linear = 0;
               break;
            }

/* If possible, determine the gradient along this dimension, storing
   it in the appropriate element of the gradient matrix. */
            outdiff = out2 - out1;
            if ( outdiff != 0.0 ) {
               grad[ ii++ ] = ( in2 - in1 ) / outdiff;
            } else {
               grad[ ii++ ] = 0.0;
            }

/* Accumulate the sum used to determine the zero point. */
            z += ( in1 + in2 );
         }

/* Also quit the outer loop if a linear fit cannot be obtained. */
         if ( !linear ) break;

/* Determine the average zero point from all dimensions. */
         zero[ coord_in ] = z / (double) ( 2 * ndim_out );
      }

/* If a linear fit was obtained, its zero points will be appropriate
   to an output coordinate system with an origin at the centre of the
   output grid (we assume this to simplify the calculations above). To
   correct for this, we transform the actual output coordinates of the
   grid's centre through the matrix of gradients and subtract the
   resulting coordinates from the zero point values. The zero points
   are then correct for the actual input and output coordinate systems
   we are using. */
      if ( linear ) {
         ii = 0;
         for ( coord_in = 0; coord_in < ndim_in; coord_in++ ) {
            for ( coord_out = 0; coord_out < ndim_out; coord_out++ ) {
               x0 = 0.5 * (double) ( lbnd[ coord_out ] + ubnd[ coord_out ] );
               zero[ coord_in ] -= grad[ ii++ ] * x0;
            }
         }
      }
   }

/* Annul the pointers to the PointSets used above. */
   pset_in_f = astAnnul( pset_in_f );
   pset_out_f = astAnnul( pset_out_f );

/* Calculate the number of test points required. */
/* --------------------------------------------- */
/* If we have obtained a linear fit above, it will (by construction)
   be exact at the centre of each face of the output grid. However, it
   may not fit anywhere else. We therefore set up some test points to
   determine if it is an adequate approximation elsewhere. */
   if ( astOK && linear ) {

/* Calculate the number of test points required to place one at each
   vertex of the grid. */
      npoint = 1;
      for ( coord_out = 0; coord_out < ndim_out; coord_out++ ) {
         npoint *= 2;
      }

/* Now calculate the total number of test points required, also
   allowing one at the centre, one at half the distance to each face,
   and one at half the distance to each vertex. */
      npoint = 1 + 2 * ( ndim_out + npoint );

/* Set up test points in the output coordinate system. */
/* --------------------------------------------------- */   
/* Create a PointSet to hold the test coordinates and obtain an array
   of pointers to its coordinate data. */
      pset_out_t = astPointSet( npoint, ndim_out, "" );
      ptr_out_t = astGetPoints( pset_out_t );
      if ( astOK ) {

/* If the output array is 1-dimensional, the face and vertex positions
   calculated below will co-incide. Therefore, we simply distribute
   the required number of test points uniformly throughout the output
   coordinate range (avoiding the end-points, where the fit has been
   obtained). The coordinates are stored in the PointSet created
   above. */
         if ( ndim_out == 1 ) {
            for ( point = 0; point < npoint; point++ ) {
               frac = ( (double) ( point + 1 ) ) / ( (double) ( npoint + 1 ) );
               ptr_out_t[ 0 ][ point ] = ( 1.0 - frac ) * (double) lbnd[ 0 ] +
                                         frac * (double) ubnd[ 0 ];
            }

/* Otherwise, generate one point at the grid centre. */
         } else {
            point = 0;
            for ( coord_out = 0; coord_out < ndim_out; coord_out++ ) {
               ptr_out_t[ coord_out ][ point ] =
                  0.5 * (double) ( lbnd[ coord_out ] + ubnd[ coord_out ] );
            }
            point++;

/* Similarly generate a point half way between the grid centre and the
   centre of each face. */
            for ( face = 0; face < ( 2 * ndim_out ); face++ ) {
               for ( coord_out = 0; coord_out < ndim_out; coord_out++ ) {
                  ptr_out_t[ coord_out ][ point ] =
                     0.5 * (double) ( lbnd[ coord_out ] + ubnd[ coord_out ] );
               }
               ptr_out_t[ face / 2 ][ point ] =
                  0.5 * ( ( (double) ( ( face % 2 ) ? ubnd[ face / 2 ] :
                                                      lbnd[ face / 2 ] ) ) +
                          ptr_out_t[ face / 2 ][ 0 ] );
               point++;
            }

/* Allocate workspace and initialise flags for identifying the
   vertices. */
            vertex = astMalloc( sizeof( int ) * (size_t) ndim_out );
            if ( astOK ) {
               for ( coord_out = 0; coord_out < ndim_out; coord_out++ ) {
                  vertex[ coord_out ] = 0;
               }

/* Now loop to visit each output grid vertex. */
               done = 0;
               while ( !done ) {

/* Generate a test point at each vertex. */
                  for ( coord_out = 0; coord_out < ndim_out; coord_out++ ) {
                     ptr_out_t[ coord_out ][ point ] = vertex[ coord_out ] ?
                                                       ubnd[ coord_out ] :
                                                       lbnd[ coord_out ];

/* Also place one half way between the grid centre and each vertex. */
                     ptr_out_t[ coord_out ][ point + 1 ] =
                        0.5 * ( ptr_out_t[ coord_out ][ point ] +
                                ptr_out_t[ coord_out ][ 0 ] );
                  }
                  point += 2;

/* Now update the array of vertex flags to identify the next vertex. */
                  coord_out = 0;
                  do {

/* The least significant dimension which does not have its upper bound
   as one of the vertex coordinates is changed to use its upper bound
   in the next vertex. */
                     if ( !vertex[ coord_out ] ) {
                        vertex[ coord_out ] = 1;
                        break;

/* Any less significant dimensions whose upper bounds are already
   being used are changed to use their lower bounds in the next
   vertex. */
                     } else {
                        vertex[ coord_out ] = 0;

/* All vertices have been visited when the most significant dimension
   is changed back to using its lower bound. */
                        done = ( ++coord_out == ndim_out );
                     }
                  } while ( !done );
               }
            }

/* Free the workspace used for vertex flags. */
            vertex = astFree( vertex );
         }

/* Transform the test points. */
/* -------------------------- */
/* Use the Mapping to transform the test points into the input grid's
   coordinate system, obtaining a pointer to the resulting arrays of
   input coordinates. */
         pset_in_t = astTransform( this, pset_out_t, 0, NULL );
         ptr_in_t = astGetPoints( pset_in_t );

/* Test the linear fit for accuracy. */
/* --------------------------------- */
/* If OK so far, then loop to use this fit to transform each test
   point and compare the result with the result of applying the
   Mapping. */
         if ( astOK ) {
            for ( point = 0; point < npoint; point++ ) {

/* Initialise the fitting error for the current point. */
               err = 0.0;

/* Obtain each input coordinate (produced by using the Mapping) in
   turn and check that it is not bad. If it is, then the
   transformation is not linear, so give up testing the fit. */
               ii = 0;
               for ( coord_in = 0; coord_in < ndim_in; coord_in++ ) {
                  y = ptr_in_t[ coord_in ][ point ];
                  if ( y == AST__BAD ) {
                     linear = 0;
                     break;
                  }

/* Apply the fitted transformation to the output coordinates to obtain
   the approximate input coordinate value. */
                  yfit = zero[ coord_in ];
                  for ( coord_out = 0; coord_out < ndim_out; coord_out++ ) {
                     yfit += grad[ ii++ ] * ptr_out_t[ coord_out ][ point ];
                  }

/* Form the sum of squared differences between the Mapping's
   transformation and the fit. */
                  diff = ( y - yfit );
                  err += diff * diff;
               }

/* Quit the outer loop if the Mapping is found to be non-linear. */
               if ( !linear ) break;

/* Test if the Cartesian distance between the true input coordinate
   and the approximate one exceeds the accuracy tolerance. If this
   happens for any test point, we declare the Mapping non-linear and
   give up. */
               if ( sqrt( err ) > tol ) {
                  linear = 0;
                  break;
               }
            }
         }

/* Annul the pointers to the PointSets used above. */
         pset_in_t = astAnnul( pset_in_t );
      }
      pset_out_t = astAnnul( pset_out_t );
   }

/* If an error occurred, or the Mapping was found to be non-linear,
   then free the space allocated for the fit coefficients, clearing
   the returned pointer value. */
   if ( !astOK || !linear ) fit = astFree( fit );

/* Return the result. */
   return fit;
}

static double LocalMaximum( const MapData *mapdata, double acc, double fract,
                            double x[] ) {
/*
*  Name:
*     LocalMaximum

*  Purpose:
*     Find a local maximum in a Mapping function.

*  Type:
*     Private function.

*  Synopsis:
*     #include "mapping.h"
*     double LocalMaximum( const MapData *mapdata, double acc, double fract,
*                          double x[] );

*  Class Membership:
*     Mapping member function.

*  Description:
*     This function finds a local maximum in the Mapping function
*     supplied.  It employs the modified simplex method (as
*     implemented by UphillSimplex), but repeatedly re-starts the
*     simplex algorithm and tests for convergence of successive
*     maxima, so as to further improve robustness on difficult
*     problems.

*  Parameters:
*     mapdata
*        Pointer to a MapData structure describing the Mapping
*        function, its coordinate constraints, etc.
*     acc
*        The required accuracy with which the maximum is to be found.
*     fract
*        A value between 0.0 and 1.0 which determines the initial step
*        length along each coordinate axis. It should be given as a
*        fraction of the difference between the upper and lower
*        constraint values for each axis (as specified in the
*        "mapdata" structure).
*     x
*        Pointer to an array of double containing the coordinates of
*        an initial estimate of the position of the maximum. On exit,
*        this will be updated to contain the best estimate of the
*        maximum's position, as found by this function.

*  Returned Value:
*     The best estimate of the Mapping function's maximum value.

*  Notes:
*     - A value of AST__BAD will be returned, and no useful
*     information about a solution will be produced, if this function
*     is invoked with the global error status set or if it should fail
*     for any reason.
*/

/* Local Constants: */
   const int maxcall = 1500;     /* Maximum number of function evaluations */
   const int maxiter = 5;        /* Maximum number of iterations */

/* Local Variables: */
   double *dx;                   /* Pointer to array of step lengths */
   double err;                   /* Simplex error estimate */
   double maximum;               /* Simplex maximum value */
   double middle;                /* Middle coordinate between bounds */
   double result;                /* Result value to return */
   int coord;                    /* Loop counter for coordinates */
   int done;                     /* Iterations complete? */
   int iter;                     /* Loop counter for iterations */
   int ncall;                    /* Number of function calls (junk) */

/* Initialise. */
   result = AST__BAD;

/* Check the global error status. */
   if ( !astOK ) return result;

/* Allocate workspace. */
   dx = astMalloc( sizeof( double ) * (size_t) mapdata->nin );

/* Perform iterations to repeatedly identify a local maximum. */
   for ( iter = 0; astOK && ( iter < maxiter ); iter++ ) {

/* Set up initial step lengths along each coordinate axis, adjusting
   their signs to avoid placing points outside the coordinate
   constraints (i.e. step away from the closer boundary on each
   axis). */
      for ( coord = 0; coord < mapdata->nin; coord++ ) {
         middle = 0.5 * ( mapdata->lbnd[ coord ] + mapdata->ubnd[ coord ] );
         dx[ coord ] = fract * ( mapdata->ubnd[ coord ] -
                                 mapdata->lbnd[ coord ] );
         if ( x[ coord ] > middle ) dx[ coord ] = -dx[ coord ];
      }

/* Find an approximation to a local maximum using the simplex method
   and check for errors. */
      maximum = UphillSimplex( mapdata, acc, maxcall, dx, x, &err, &ncall );
      if ( astOK ) {

/* Use this maximum value if no previous maximum has been found. */
         if ( result == AST__BAD ) {
            result = maximum;

/* Otherwise use it only if it improves on the previous maximum. */
         } else if ( maximum >= result ) {

/* We iterate, re-starting the simplex algorithm from its previous
   best position so as to guard against premature false
   convergence. Iterations continue until the improvement in the
   maximum is no greater than the required accuracy (and the simplex
   algorithm itself has converged to the required accuracy). Note when
   iterations should cease. */
            done = ( ( ( maximum - result ) <= acc ) && ( err <= acc ) );

/* Store the best maximum and quit iterating if appropriate. */
            result = maximum;
            if ( done ) break;
         }

/* Otherwise, decrement the initial step size for the next iteration. */
         fract /= 1000.0;
      }
   }
   
/* Free the workspace. */
   dx = astFree( dx );

/* If an error occurred, clear the result value. */
   if ( !astOK ) result = AST__BAD;

/* return the result. */
   return result;
}

static void MapBox( AstMapping *this,
                    const double lbnd_in[], const double ubnd_in[],
                    int forward, int coord_out,
                    double *lbnd_out, double *ubnd_out,
                    double xl[], double xu[] ) {
/*
*+
*  Name:
*     astMapBox

*  Purpose:
*     Find a bounding box for a Mapping.

*  Type:
*     Protected virtual function.

*  Synopsis:
*     #include "mapping.h"
*     void astMapBox( AstMapping *this,
*                     const double lbnd_in[], const double ubnd_in[],
*                     int forward, int coord_out,
*                     double *lbnd_out, double *ubnd_out,
*                     double xl[], double xu[] );

*  Class Membership:
*     Mapping method.

*  Description:
*     This function allows you to find the "bounding box" which just
*     encloses another box after it has been transformed by a Mapping
*     (using either its forward or inverse transformation). A typical
*     use might be to calculate the size which an image would have
*     after being transformed by the Mapping.
*
*     The function works on one dimension at a time. When supplied
*     with the lower and upper bounds of a rectangular region (box) of
*     input coordinate space, it finds the lowest and highest values
*     taken by a nominated output coordinate within that
*     region. Optionally, it also returns the input coordinates where
*     these bounding values are attained. It should be used repeatedly
*     if the extent of the bounding box is required in more than one
*     dimension.

*  Parameters:
*     this
*        Pointer to the Mapping.
*     lbnd_in
*        Pointer to an array of double, with one element for each
*        Mapping input coordinate. This should contain the lower bound
*        of the input box in each dimension.
*     ubnd_in
*        Pointer to an array of double, with one element for each
*        Mapping input coordinate. This should contain the upper bound
*        of the input box in each dimension.
*
*        Note that it is permissible for the lower bound to exceed the
*        corresponding upper bound, as the values will simply be
*        swapped before use.
*     forward
*        If this value is non-zero, then the Mapping's forward
*        transformation will be used to transform the input
*        box. Otherwise, its inverse transformation will be used.
*
*        (If the inverse transformation is selected, then references
*        to "input" and "output" coordinates in this description
*        should be transposed. For example, the size of the "lbnd_in"
*        and "ubnd_in" arrays should match the number of output
*        coordinates, as given by the Mapping's Nout attribute.)
*     coord_out
*        The (zero-based) index of the output coordinate for which the
*        lower and upper bounds are required.
*     lbnd_out
*        Pointer to a double in which to return the lowest value taken
*        by the nominated output coordinate within the specified
*        region of input coordinate space.
*     ubnd_out
*        Pointer to a double in which to return the highest value
*        taken by the nominated output coordinate within the specified
*        region of input coordinate space.
*     xl
*        An optional pointer to an array of double, with one element
*        for each Mapping input coordinate. If given, this array will
*        be filled with the coordinates of an input point (although
*        not necessarily a unique one) for which the nominated output
*        coordinate takes the lower bound value returned in
*        "*lbnd_out".
*
*        If these coordinates are not required, a NULL pointer may be
*        supplied.
*     xu
*        An optional pointer to an array of double, with one element
*        for each Mapping input coordinate. If given, this array will
*        be filled with the coordinates of an input point (although
*        not necessarily a unique one) for which the nominated output
*        coordinate takes the upper bound value returned in
*        "*ubnd_out".
*
*        If these coordinates are not required, a NULL pointer may be
*        supplied.

*  Notes:
*     - Any input points which are transformed by the Mapping to give
*     output coordinates containing the value AST__BAD are regarded as
*     invalid and are ignored, They will make no contribution to
*     determining the output bounds, even although the nominated
*     output coordinate might still have a valid value at such points.
*     - An error will occur if the required output bounds cannot be
*     found. Typically, this might occur if all the input points which
*     the function considers turn out to be invalid (see above). The
*     number of points considered before generating such an error is
*     quite large, however, so this is unlikely to occur by accident
*     unless valid points are restricted to a very small subset of the
*     input coordinate space.
*     - The values returned via "lbnd_out", "ubnd_out", "xl" and "xu"
*     will be set to the value AST__BAD if this function should fail
*     for any reason. Their initial values on entry will not be
*     altered if the function is invoked with the global error status
*     set.
*-

*  Implementation Notes:
*     - This function implements the basic astMapBox method available
*     via the protected interface to the Mapping class. The public
*     interface to this method is provided by the astMapBoxId_
*     function.
*/

/* Local Variables: */
   MapData mapdata;              /* Structure to describe Mapping function */
   double *x_l;                  /* Pointer to coordinate workspace */
   double *x_u;                  /* Pointer to coordinate workspace */
   double lbnd;                  /* Required lower bound */
   double ubnd;                  /* Required upper bound */
   int coord;                    /* Loop counter for coordinates. */
   int nin;                      /* Effective number of input coordinates */
   int nout;                     /* Effective number of output coordinates */

/* Check the global error status. */
   if ( !astOK ) return;

/* Obtain the effective numbers of input and output coordinates for
   the Mapping, taking account of which transformation is to be
   used. */
   nin = forward ? astGetNin( this ) : astGetNout( this );
   nout = forward ? astGetNout( this ) : astGetNin( this );

/* Check that the output coordinate index supplied is valid and report
   an error if it is not. Use public (one-based) coordinate numbering
   in the error message. */
   if ( astOK ) {
      if ( ( coord_out < 0 ) || ( coord_out >= nout ) ) {
         astError( AST__BADCI, "astMapBox(%s): Output coordinate index (%d) "
                   "invalid - it should be in the range 1 to %d.",
                   astGetClass( this ), coord_out + 1, nout );
      }
   }

/* Initialise a MapData structure to describe the Mapping function
   whose limits are to be found.  Since it may be evaluated many
   times, we attempt to simplify the Mapping supplied. */
   if ( astOK ) {
      mapdata.mapping = astSimplify( this );

/* Store the number of input/output coordinates and the index of the
   output coordinate in which we are interested. */
      mapdata.nin = nin;
      mapdata.nout = nout;
      mapdata.coord = coord_out;

/* Note which Mapping transformation is being used. */
      mapdata.forward = forward;

/* Store pointers to arrays which will contain the input coordinate
   bounds. */
      mapdata.lbnd = astMalloc( sizeof( double ) * (size_t) nin );
      mapdata.ubnd = astMalloc( sizeof( double ) * (size_t) nin );

/* Create PointSets for passing coordinate data to and from the
   Mapping. */
      mapdata.pset_in = astPointSet( 1, nin, "" );
      mapdata.pset_out = astPointSet( 1, nout, "" );

/* Obtain pointers to these PointSets' coordinate arrays. */
      mapdata.ptr_in = astGetPoints( mapdata.pset_in );
      mapdata.ptr_out = astGetPoints( mapdata.pset_out );

/* Allocate workspace for the returned input coordinates. */
      x_l = astMalloc( sizeof( double ) * (size_t) nin );
      x_u = astMalloc( sizeof( double ) * (size_t) nin );
      if ( astOK ) {

/* Initialise the output bounds and corresponding input coordinates to
   "unknown". */
         lbnd = AST__BAD;
         ubnd = AST__BAD;
         for ( coord = 0; coord < nin; coord++ ) {
            x_l[ coord ] = AST__BAD;
            x_u[ coord ] = AST__BAD;

/* Initialise the input bounds, ensuring they are the correct way
   around (if not already supplied this way). */
            mapdata.lbnd[ coord ] = ( lbnd_in[ coord ] < ubnd_in[ coord ] ) ?
                                      lbnd_in[ coord ] : ubnd_in[ coord ];
            mapdata.ubnd[ coord ] = ( ubnd_in[ coord ] > lbnd_in[ coord ] ) ?
                                      ubnd_in[ coord ] : lbnd_in[ coord ];
         }

/* First examine a set of special input points to obtain an initial
   estimate of the required output bounds. Do this only so long as the
   number of points involved is not excessive. */
         if ( nin <= 12 ) SpecialBounds( &mapdata, &lbnd, &ubnd, x_l, x_u );

/* Then attempt to refine this estimate using a global search
   algorithm. */
         GlobalBounds( &mapdata, &lbnd, &ubnd, x_l, x_u );

/* If an error occurred, generate a contextual error message. */
         if ( !astOK ) {
            astError( astStatus, "Unable to find a bounding box for a %s.",
                      astGetClass( this ) );
         }
      }

/* Return the output bounds and, if required, the input coordinate
   values which correspond with them. */
      if ( astOK ) {
         *lbnd_out = lbnd;
         *ubnd_out = ubnd;
         for ( coord = 0; coord < nin; coord++ ) {
            if ( xl ) xl[ coord ] = x_l[ coord ];
            if ( xu ) xu[ coord ] = x_u[ coord ];
         }
      }

/* Annul the simplified Mapping pointer and the temporary
   PointSets. Also free the workspace. */
      mapdata.mapping = astAnnul( mapdata.mapping );
      mapdata.lbnd = astFree( mapdata.lbnd );
      mapdata.ubnd = astFree( mapdata.ubnd );
      mapdata.pset_in = astAnnul( mapdata.pset_in );
      mapdata.pset_out = astAnnul( mapdata.pset_out );
      x_l = astFree( x_l );
      x_u = astFree( x_u );
   }
      
/* If an error occurred, then return bad bounds values and
   coordinates. */
   if ( !astOK ) {
      *lbnd_out = AST__BAD;
      *ubnd_out = AST__BAD;
      for ( coord = 0; coord < nin; coord++ ) {
         if ( xl ) xl[ coord ] = AST__BAD;
         if ( xu ) xu[ coord ] = AST__BAD;
      }
   }
}

static double MapFunction( const MapData *mapdata, const double in[],
                           int *ncall ) {
/*
*  Name:
*     MapFunction

*  Purpose:
*     Return the value of a selected transformed coordinate.

*  Type:
*     Private function.

*  Synopsis:
*     #include "mapping.h"
*     double MapFunction( const MapData *mapdata, const double in[],
*                         int *ncall );

*  Class Membership:
*     Mapping member function.

*  Description:
*     This function takes a set of input coordinates and applies a
*     Mapping's coordinate transformation to them. It then returns the
*     value of one of the transformed coordinates.
*
*     It is provided for use by optimisation functions (e.g. those
*     used for finding bounding boxes). The Mapping to be used and
*     associated parameters (such as constraints on the range of input
*     coordinates and the index of the output coordinate to be
*     returned) are supplied in a MapData structure. The value
*     returned will be negated if the "negate" component of this
*     structure is non-zero.
*
*     The value AST__BAD will be returned by this function if the
*     input coordinates lie outside the constrained range given in
*     the MapData structure, or if any of the transformed output
*     coordinates is bad.

*  Parameters:
*     mapdata
*        Pointer to a MapData structure which describes the Mapping to
*        be used.
*     in
*        A double array containing the input coordinates of a single point.
*     ncall
*        Pointer to an int containing a count of the number of times
*        the Mapping's coordinate transformation has been used. This
*        value will be updated to reflect any use made by this
*        function. Normally, this means incrementing the value by 1,
*        but this will be omitted if the input coordinates supplied
*        are outside the constrained range so that no transformation
*        is performed.

*  Returned Value:
*     The selected output coordinate value, or AST__BAD, as appropriate.

*  Notes:
*     - A value of AST__BAD will be returned if this function is
*     invoked with the global error status set, or if it should fail
*     for any reason.
*/

/* Local Variables: */
   double result;                /* Result to be returned */
   int bad;                      /* Output coordinates invalid? */
   int coord_in;                 /* Loop counter for input coordinates */
   int coord_out;                /* Loop counter for output coordinates */
   int outside;                  /* Input point outside bounds? */

/* Initialise. */
   result = AST__BAD;

/* Check the global error status. */
   if ( !astOK ) return result;

/* See if the input point lies outside the required bounds. */
   outside = 0;
   for ( coord_in = 0; coord_in < mapdata->nin; coord_in++ ) {
      if ( ( in[ coord_in ] < mapdata->lbnd[ coord_in ] ) ||
           ( in[ coord_in ] > mapdata->ubnd[ coord_in ] ) ) {
         outside = 1;
         break;
      }

/* Also store the input coordinates in the memory associated with the
   Mapping's input PointSet. */
      mapdata->ptr_in[ coord_in ][ 0 ] = in[ coord_in ];
   }

/* If the input coordinates are within bounds, transform them, using the
   PointSets identified in the "mapdata" structure. */
   if ( !outside ) {
      (void) astTransform( mapdata->mapping, mapdata->pset_in,
                           mapdata->forward, mapdata->pset_out );

/* Increment the number of calls to astTransform and check the error
   status. */
      ( *ncall )++;
      if ( astOK ) {

/* If OK, test if any of the output coordinates is bad. */
         bad = 0;
         for ( coord_out = 0; coord_out < mapdata->nout; coord_out++ ) {
            if ( mapdata->ptr_out[ coord_out ][ 0 ] == AST__BAD ) {
               bad = 1;
               break;
            }
         }

/* If not, then extract the required output coordinate, negating it if
   necessary. */
         if ( !bad ) {
            result = mapdata->ptr_out[ mapdata->coord ][ 0 ];
            if ( mapdata->negate ) result = -result;
         }
      }
   }

/* Return the result. */
   return result;
}

static void MapList( AstMapping *this, int series, int invert, int *nmap,
                     AstMapping ***map_list, int **invert_list ) {
/*
*+
*  Name:
*     astMapList

*  Purpose:
*     Decompose a Mapping into a sequence of simpler Mappings.

*  Type:
*     Protected virtual function.

*  Synopsis:
*     #include "mapping.h"
*     void astMapList( AstMapping *this, int series, int invert, int *nmap,
*                      AstMapping ***map_list, int **invert_list )

*  Class Membership:
*     Mapping method.

*  Description:
*     This function decomposes a Mapping (which, in derived classes,
*     may be a compound Mapping) into a sequence of simpler Mappings
*     which may be applied in sequence to achieve the same effect. The
*     Mapping is decomposed as far as possible, but it is not
*     guaranteed that this will necessarily yield any more than one
*     Mapping, which may actually be the original one supplied.
*
*     This function is provided to support both the simplification of
*     compound Mappings, and the analysis of Mapping structure so that
*     particular forms can be recognised.

*  Parameters:
*     this
*        Pointer to the Mapping to be decomposed (the Mapping is not
*        actually modified by this function).
*     series
*        If this value is non-zero, an attempt will be made to
*        decompose the Mapping into a sequence of equivalent Mappings
*        which can be applied in series (i.e. one after the other). If
*        it is zero, the decomposition will instead yield Mappings
*        which can be applied in parallel (i.e. on successive sub-sets
*        of the input/output coordinates).
*     invert
*        The value to which the Mapping's Invert attribute is to be
*        (notionally) set before performing the
*        decomposition. Normally, the value supplied here will be the
*        actual Invert value obtained from the Mapping (e.g. using
*        astGetInvert).  Sometimes, however, when a Mapping is
*        encapsulated within another structure, that structure may
*        retain an Invert value (in order to prevent external
*        interference) which should be used instead.
*
*        Note that the actual Invert value of the Mapping supplied is
*        not used (or modified) by this function.
*     nmap
*        The address of an int which holds a count of the number of
*        individual Mappings in the decomposition. On entry, this
*        should count the number of Mappings already in the
*        "*map_list" array (below). On exit, it is updated to include
*        any new Mappings appended by this function.
*     map_list
*        Address of a pointer to an array of Mapping pointers. On
*        entry, this array pointer should either be NULL (if no
*        Mappings have yet been obtained) or should point at a
*        dynamically allocated array containing Mapping pointers
*        ("*nmap" in number) which have been obtained from a previous
*        invocation of this function.
*
*        On exit, the dynamic array will be enlarged to contain any
*        new Mapping pointers that result from the decomposition
*        requested. These pointers will be appended to any previously
*        present, and the array pointer will be updated as necessary
*        to refer to the enlarged array (any space released by the
*        original array will be freed automatically).
*
*        The new Mapping pointers returned will identify a sequence of
*        Mappings which, when applied in order, will perform a forward
*        transformation equivalent to that of the original Mapping
*        (after its Invert flag has first been set to the value
*        requested above). The Mappings should be applied in series or
*        in parallel according to the type of decomposition requested.
*
*        All the Mapping pointers returned by this function should be
*        annulled by the caller, using astAnnul, when no longer
*        required. The dynamic array holding these pointers should
*        also be freed, using astFree.
*     invert_list
*        Address of a pointer to an array of int. On entry, this array
*        pointer should either be NULL (if no Mappings have yet been
*        obtained) or should point at a dynamically allocated array
*        containing Invert attribute values ("*nmap" in number) which
*        have been obtained from a previous invocation of this
*        function.
*
*        On exit, the dynamic array will be enlarged to contain any
*        new Invert attribute values that result from the
*        decomposition requested. These values will be appended to any
*        previously present, and the array pointer will be updated as
*        necessary to refer to the enlarged array (any space released
*        by the original array will be freed automatically).
*
*        The new Invert values returned identify the values which must
*        be assigned to the Invert attributes of the corresponding
*        Mappings (whose pointers are in the "*map_list" array) before
*        they are applied. Note that these values may differ from the
*        actual Invert attribute values of these Mappings, which are
*        not relevant.
*
*        The dynamic array holding these values should be freed by the
*        caller, using astFree, when no longer required.

*  Notes:
*     - It is unspecified to what extent the original Mapping and the
*     individual (decomposed) Mappings are
*     inter-dependent. Consequently, the individual Mappings cannot be
*     modified without risking modification of the original.
*     - If this function is invoked with the global error status set,
*     or if it should fail for any reason, then the *nmap value, the
*     list of Mapping pointers and the list of Invert values will all
*     be returned unchanged.
*- 
*/

/* Check the global error status. */
   if ( !astOK ) return;

/* Since we are dealing with a basic Mapping, only one new Mapping
   pointer will be returned. Extend the dynamic arrays to accommodate
   this Mapping. */
   *map_list = astGrow( *map_list, *nmap + 1, sizeof( AstMapping * ) );
   *invert_list = astGrow( *invert_list, *nmap + 1, sizeof( int ) );
   if ( astOK ) {

/* Return the invert flag value for the Mapping and a clone of the
   Mapping pointer. */
      ( *invert_list )[ *nmap ] = ( invert != 0 );
      ( *map_list )[ *nmap ] = astClone( this );

/* If OK, return the new Mapping count. */
      if ( astOK ) ( *nmap )++;
   }
}

static int MapMerge( AstMapping *this, int where, int series, int *nmap,
                     AstMapping ***map_list, int **invert_list ) {
/*
*+
*  Name:
*     astMapMerge

*  Purpose:
*     Simplify a sequence of Mappings.

*  Type:
*     Protected virtual function.

*  Synopsis:
*     #include "mapping.h"
*     int astMapMerge( AstMapping *this, int where, int series, int *nmap,
*                      AstMapping ***map_list, int **invert_list )

*  Class Membership:
*     Mapping method.

*  Description:
*     This function attempts to simplify a sequence of Mappings by
*     merging a nominated Mapping in the sequence with its neighbours,
*     so as to shorten the sequence if possible.
*
*     In many cases, simplification will not be possible and the
*     function will return -1 to indicate this, without further
*     action.
*
*     In most cases of interest, however, this function will either
*     attempt to replace the nominated Mapping with one which it
*     considers simpler, or to merge it with the Mappings which
*     immediately precede it or follow it in the sequence (both will
*     normally be considered). This is sufficient to ensure the
*     eventual simplification of most Mapping sequences by repeated
*     application of this function.
*
*     In some cases, the function may attempt more elaborate
*     simplification, involving any number of other Mappings in the
*     sequence. It is not restricted in the type or scope of
*     simplification it may perform, but will normally only attempt
*     elaborate simplification in cases where a more straightforward
*     approach is not adequate.

*  Parameters:
*     this
*        Pointer to the nominated Mapping which is to be merged with
*        its neighbours. This should be a cloned copy of the Mapping
*        pointer contained in the array element "(*map_list)[where]"
*        (see below). This pointer will not be annulled, and the
*        Mapping it identifies will not be modified by this function.
*     where
*        Index in the "*map_list" array (below) at which the pointer
*        to the nominated Mapping resides.
*     series
*        A non-zero value indicates that the sequence of Mappings to
*        be simplified will be applied in series (i.e. one after the
*        other), whereas a zero value indicates that they will be
*        applied in parallel (i.e. on successive sub-sets of the
*        input/output coordinates).
*     nmap
*        Address of an int which counts the number of Mappings in the
*        sequence. On entry this should be set to the initial number
*        of Mappings. On exit it will be updated to record the number
*        of Mappings remaining after simplification.
*     map_list
*        Address of a pointer to a dynamically allocated array of
*        Mapping pointers (produced, for example, by the astMapList
*        method) which identifies the sequence of Mappings. On entry,
*        the initial sequence of Mappings to be simplified should be
*        supplied.
*
*        On exit, the contents of this array will be modified to
*        reflect any simplification carried out. Any form of
*        simplification may be performed. This may involve any of: (a)
*        removing Mappings by annulling any of the pointers supplied,
*        (b) replacing them with pointers to new Mappings, (c)
*        inserting additional Mappings and (d) changing their order.
*
*        The intention is to reduce the number of Mappings in the
*        sequence, if possible, and any reduction will be reflected in
*        the value of "*nmap" returned. However, simplifications which
*        do not reduce the length of the sequence (but improve its
*        execution time, for example) may also be performed, and the
*        sequence might conceivably increase in length (but normally
*        only in order to split up a Mapping into pieces that can be
*        more easily merged with their neighbours on subsequent
*        invocations of this function).
*
*        If Mappings are removed from the sequence, any gaps that
*        remain will be closed up, by moving subsequent Mapping
*        pointers along in the array, so that vacated elements occur
*        at the end. If the sequence increases in length, the array
*        will be extended (and its pointer updated) if necessary to
*        accommodate any new elements.
*
*        Note that any (or all) of the Mapping pointers supplied in
*        this array may be annulled by this function, but the Mappings
*        to which they refer are not modified in any way (although
*        they may, of course, be deleted if the annulled pointer is
*        the final one).
*     invert_list
*        Address of a pointer to a dynamically allocated array which,
*        on entry, should contain values to be assigned to the Invert
*        attributes of the Mappings identified in the "*map_list"
*        array before they are applied (this array might have been
*        produced, for example, by the astMapList method). These
*        values will be used by this function instead of the actual
*        Invert attributes of the Mappings supplied, which are
*        ignored.
*
*        On exit, the contents of this array will be updated to
*        correspond with the possibly modified contents of the
*        "*map_list" array.  If the Mapping sequence increases in
*        length, the "*invert_list" array will be extended (and its
*        pointer updated) if necessary to accommodate any new
*        elements.

*  Returned Value:
*     If simplification was possible, the function returns the index
*     in the "map_list" array of the first element which was
*     modified. Otherwise, it returns -1 (and makes no changes to the
*     arrays supplied).

*  Notes:
*     - A value of -1 will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*-
*/

/* This is the default method which is inherited by all Mappings which
   do not explicitly provide their own simplification method. Return
   -1 to indicate that no simplification is provided. */
   return -1;
}

static int MaxI( int a, int b ) {
/*
*  Name:
*     MaxI

*  Purpose:
*     Return the maximum of two integer values.

*  Type:
*     Private function.

*  Synopsis:
*     #include "mapping.h"
*     int MaxI( int a, int b )

*  Class Membership:
*     Mapping member function.

*  Description:
*     This function returns the maximum of two integer values.

*  Parameters:
*     a
*        The first value.
*     b
*        The second value.

*  Returned Value:
*     The maximum.
*/

/* Return the larger value. */
   return ( a > b ) ? a : b;
}

static int MinI( int a, int b ) {
/*
*  Name:
*     MinI

*  Purpose:
*     Return the minimum of two integer values.

*  Type:
*     Private function.

*  Synopsis:
*     #include "mapping.h"
*     int MinI( int a, int b )

*  Class Membership:
*     Mapping member function.

*  Description:
*     This function returns the minimum of two integer values.

*  Parameters:
*     a
*        The first value.
*     b
*        The second value.

*  Returned Value:
*     The minimum.
*/

/* Return the smaller value. */
   return ( a < b ) ? a : b;
}

static double NewVertex( const MapData *mapdata, int lo, double scale,
                         double x[], double f[], int *ncall, double xnew[] ) {
/*
*  Name:
*     NewVertex

*  Purpose:
*     Locate a new vertex for a simplex.

*  Type:
*     Private function.

*  Synopsis:
*     #include "mapping.h"
*     double NewVertex( const MapData *mapdata, int lo, double scale,
*                       double x[], double f[], int *ncall, double xnew[] );

*  Class Membership:
*     Mapping member function.

*  Description:
*     This function is provided for use during optimisation of a
*     Mapping function using the simplex method. It generates the
*     coordinates of a new simplex vertex and evaluates the Mapping
*     function at that point.  If the function's value is better then
*     (i.e. larger than) the value at the previously worst vertex,
*     then it is used to replace that vertex.

*  Parameters:
*     mapdata
*        Pointer to a MapData structure which describes the Mapping
*        function to be used.
*     lo
*        The (zero-based) index of the simplex vertex which initially
*        has the worst (lowest) value.
*     scale
*        The scale factor to be used to generate the new vertex. The
*        distance of the worst vertex from the centre of the face
*        opposite it is scaled by this factor to give the new vertex
*        position. Negative factors result in reflection through this
*        opposite face.
*     x
*        An array of double containing the coordinates of the vertices
*        of the simplex. The coordinates of the first vertex are
*        stored first, then those of the second vertex, etc. This
*        array will be updated by this function if the new vertex is
*        used to replace an existing one.
*     f
*        An array of double containing the Mapping function values at
*        each vertex of the simplex. This array will be updated by
*        this function if the new vertex is used to replace an
*        existing one.
*     ncall
*        Pointer to an int containing a count of the number of times
*        the Mapping function has been invoked. This value will be
*        updated to reflect the actions of this function.
*     xnew
*        An array of double with one element for each input coordinate
*        of the Mapping function. This is used as workspace.

*  Returned Value:
*     The Mapping function value at the new vertex. This value is
*     returned whether or not the new vertex replaces an existing one.

*  Notes:
*     - A value of AST__BAD will be returned by this function if it is
*     invoked with the global error status set, or if it should fail
*     for any reason.
*     - A value of AST__BAD will also be returned if the new vertex
*     lies outside the constrained range of input coordinates
*     associated with the Mapping function (as specified in the
*     MapData structure supplied) or if any of the transformed output
*     coordinates produced by the underlying Mapping is bad. In either
*     case the new vertex will not be used to replace an existing one.
*/

/* Local Variables: */
   double fnew;                  /* Function value at new vertex */
   double xface;                 /* Coordinate of centre of magnification */
   int coord;                    /* Loop counter for coordinates */
   int ncoord;                   /* Number of coordinates */
   int nvertex;                  /* Number of simplex vertices */
   int vertex;                   /* Loop counter for vertices */

/* Initialise. */
   fnew = AST__BAD;

/* Check the global error status. */
   if ( !astOK ) return fnew;
   
/* Obtain the number of Mapping input coordinates from the MapData
   structure and calculate the number of simplex vertices. */
   ncoord = mapdata->nin;
   nvertex = ncoord + 1;

/* Loop to obtain each coordinate of the new vertex. */
   for ( coord = 0; coord < ncoord; coord++ ) {

/* Loop over all vertices except the lowest one and average their
   coordinates. This gives the coordinate of the centre of the face
   opposite the lowest vertex, which will act as the centre of
   magnification. */
      xface = 0.0;
      for ( vertex = 0; vertex < nvertex; vertex++ ) {
         if ( vertex != lo ) {

/* Divide each coordinate by the number of vertices as the sum is
   accumulated in order to minimise the risk of overflow. */
            xface += x[ vertex * ncoord + coord ] /
                     ( (double ) ( nvertex - 1 ) );
         }
      }

/* Magnify the lowest vertex's distance from this point by the
   required factor to give the coordinates of the new vertex. */
      xnew[ coord ] = xface + ( x[ lo * ncoord + coord ] - xface ) * scale;
   }

/* Evaluate the Mapping function at the new vertex. */
   fnew = MapFunction( mapdata, xnew, ncall );
 
/* If the result is not bad and exceeds the previous value at the
   lowest vertex, then replace the lowest vertex with this new one. */
   if ( astOK && ( fnew != AST__BAD ) && ( fnew > f[ lo ] ) ) {
      for ( coord = 0; coord < ncoord; coord++ ) {
         x[ lo * ncoord + coord ] = xnew[ coord ];
      }
      f[ lo ] = fnew;
   }

/* Return the value at the new vertex. */
   return fnew;
}

static double Random( long int *seed ) {
/*
*  Name:
*     Random

*  Purpose:
*     Return a pseudo-random value in the range 0 to 1.

*  Type:
*     Private function.

*  Synopsis:
*     #include "mapping.h"
*     double Random( long int *seed );

*  Class Membership:
*     Mapping member function.

*  Description:
*     This function returns a pseudo-random double value from a PDF
*     uniformly distributed in the range 0 to 1. It also updates a
*     seed value so that a sequence of pseudo-random values may be
*     obtained with successive invocations.

*  Parameters:
*     seed
*        Pointer to a long int which should initially contain a
*        non-zero seed value. This will be updated with a new seed
*        which may be supplied on the next invocation in order to
*        obtain a different pseudo-random value.

*  Returned Value:
*     The pseudo-random value.
*/

/* Local Variables: */
   long int i;                   /* Temporary storage */

/* This a basic random number generator using constants given in
   Numerical Recipes (Press et al.). */
   i = *seed / 127773;
   *seed = ( *seed - i * 127773 ) * 16807 - i * 2836;
   if ( *seed < 0 ) *seed += 2147483647;

/* Return the result as a double value in the range 0 to 1. */
   return ( (double) ( *seed - 1 ) ) / ( (double) 2147483646 );
}

static void ReportPoints( AstMapping *this, int forward,
                          AstPointSet *in_points, AstPointSet *out_points ) {
/*
*+
*  Name:
*     astReportPoints

*  Purpose:
*     Report the effect of transforming a set of points using a Mapping.

*  Type:
*     Protected virtual function.

*  Synopsis:
*     #include "mapping.h"
*     void astReportPoints( AstMapping *this, int forward,
*                           AstPointSet *in_points, AstPointSet *out_points )

*  Class Membership:
*     Mapping method.

*  Description:
*     This function reports the coordinates of a set of points before
*     and after being transformed by a Mapping, by writing them to
*     standard output.

*  Parameters:
*     this
*        Pointer to the Mapping.
*     forward
*        A non-zero value indicates that the Mapping's forward
*        coordinate transformation has been applied, while a zero
*        value indicates the inverse transformation.
*     in_points
*        Pointer to a PointSet which is associated with the
*        coordinates of a set of points before the Mapping was
*        applied.
*     out_points
*        Pointer to a PointSet which is associated with the
*        coordinates of the same set of points after the Mapping has
*        been applied.

*  Notes:
*     - This method is provided as a development and debugging aid to
*     be invoked when coordinates are transformed by public Mapping
*     methods and under control of the "Report" Mapping attribute.
*     - Derived clases may over-ride this method in order to change
*     the way in which coordinates are formatted, etc.
*-
*/

/* Local Variables: */
   double **ptr_in;              /* Pointer to array of input data pointers */
   double **ptr_out;             /* Pointer to array of output data pointers */
   int coord;                    /* Loop counter for coordinates */
   int ncoord_in;                /* Number of input coordinates per point */
   int ncoord_out;               /* Number of output coordinates per point */
   int npoint;                   /* Number of points to report */
   int npoint_in;                /* Number of input points */
   int npoint_out;               /* Number of output points */
   int point;                    /* Loop counter for points */

/* Check the global error status. */
   if ( !astOK ) return;

/* Obtain the numbers of points and coordinates associated with each
   PointSet. */ 
   npoint_in = astGetNpoint( in_points );
   npoint_out = astGetNpoint( out_points );
   ncoord_in = astGetNcoord( in_points );
   ncoord_out = astGetNcoord( out_points );

/* Obtain the pointers that give access to the coordinate data
   associated with each PointSet. */
   ptr_in = astGetPoints( in_points );
   ptr_out = astGetPoints( out_points );

/* In the event that both PointSets don't contain equal numbers of
   points (this shouldn't actually happen), simply use the minimum
   number. */
   npoint = ( npoint_in < npoint_out ) ? npoint_in : npoint_out;

/* Loop to report the effect of the Mapping on each point in turn. */
   for ( point = 0; point < npoint; point++ ) {

/* Report the input coordinates (in parentheses and separated by
   commas). Replace coordinate values of AST__BAD with the string
   "<bad>" to indicate missing values. */
      printf( "(" );
      for ( coord = 0; coord < ncoord_in; coord++ ) {
         if ( ptr_in[ coord ][ point ] == AST__BAD ) {
            printf( "%s<bad>", coord ? ", " : "" );
         } else {
            printf( "%s%.*g", coord ? ", " : "",
                              DBL_DIG, ptr_in[ coord ][ point ] );
         }
      }

/* Similarly report the output coordinates. */
      printf( ") --> (" );
      for ( coord = 0; coord < ncoord_out; coord++ ) {
         if ( ptr_out[ coord ][ point ] == AST__BAD ) {
            printf( "%s<bad>", coord ? ", " : "" );
         } else {
            printf( "%s%.*g", coord ? ", " : "",
                              DBL_DIG, ptr_out[ coord ][ point ] );
         }
      }
      printf( ")\n" );
   }
}

/*
*++
*  Name:
c     astResample<X>
f     AST_RESAMPLE<X>

*  Purpose:
*     Resample a region of a data grid.

*  Type:
*     Public virtual function.

*  Synopsis:
c     #include "mapping.h"
c     int astResample<X>( AstMapping *this, int ndim_in,
c                         const int lbnd_in[], const int ubnd_in[],
c                         const <Xtype> in[], const <Xtype> in_var[],
c                         int interp, void (* finterp)(),
c                         const double params[], int flags,
c                         double tol, int maxpix,
c                         <Xtype> badval, int ndim_out,
c                         const int lbnd_out[], const int ubnd_out[],
c                         const int lbnd[], const int ubnd[],
c                         <Xtype> out[], <Xtype> out_var[] );
f     RESULT = AST_RESAMPLE<X>( THIS, NDIM_IN, LBND_IN, UBND_IN, IN, IN_VAR,
f                               INTERP, FINTERP, PARAMS, FLAGS,
f                               TOL, MAXPIX, BADVAL,
f                               NDIM_OUT, LBND_OUT, UBND_OUT,
f                               LBND, UBND, OUT, OUT_VAR, STATUS )

*  Class Membership:
*     Mapping method.

*  Description:
*     This is a set of functions for resampling gridded data (e.g. an
*     image) under the control of a geometrical transformation, which
*     is specified by a Mapping.  The functions operate on a pair of
*     data grids (input and output), each of which may have any number
*     of dimensions, and resampling may be restricted to a specified
*     region of the output grid. An associated grid of error estimates
*     associated with the input data may also be supplied (in the form
*     of variance values), so as to produce error estimates for the
*     resampled output data. Propagation of missing data (bad pixels)
*     is supported, and a choice of sub-pixel interpolation schemes is
*     provided.
*
*     You should use a resampling function which matches the numerical
*     type of the data you are processing by replacing <X> in
c     the generic function name astResample<X> by an appropriate 1- or
f     the generic function name AST_RESAMPLE<X> by an appropriate 1- or
*     2-character type code. For example, if you are resampling data
c     with type "float", you should use the function astResampleF (see
f     with type REAL, you should use the function AST_RESAMPLER (see
*     the "Data Type Codes" section below for the codes appropriate to
*     other numerical types).
*
*     Resampling of the grid of input data is performed by
*     transforming the coordinates of the centre of each output grid
*     element (or pixel) into the coordinate system of the input grid.
*     Since the resulting coordinates will not, in general, coincide
*     with the centre of an input pixel, sub-pixel interpolation is
*     performed between the neighbouring input pixels. This produces a
*     resampled value which is then assigned to the output pixel. A
*     choice of sub-pixel interpolation schemes is provided, but you
*     may also implement your own.
*
*     Output pixel coordinates are transformed into the coordinate
*     system of the input grid using the inverse transformation of the
*     Mapping which is supplied. This means that geometrical features
*     in the input data are subjected to the Mapping's forward
*     transformation as they are transferred from the input to the
*     output grid (although the Mapping's forward transformation is
*     not explicitly used).
*
*     In practice, transforming the coordinates of every pixel of a
*     large data grid can be time-consuming, especially if the Mapping
*     involves a number of complicated functions, such as sky
*     projections. To improve performance, it is therefore possible to
*     approximate non-linear Mappings by a set of equivalent simple
*     linear transformations which are applied piece-wise to separate
*     sub-regions of the data. This approximation process is applied
*     automatically by an adaptive algorithm, under control of an
*     accuracy criterion which expresses the maximum tolerable
*     geometrical distortion which may be introduced (as a fraction of
*     a pixel).
*     
*     This algorithm first attempts to approximate the Mapping with a
*     linear transformation applied over the whole region of the
*     output grid which is being used. If this proves to be
*     insufficiently accurate, the output region is sub-divided into
*     two along its largest dimension and the process is repeated
*     within each of the resulting sub-regions. This process of
*     sub-division continues until a sufficiently good linear
*     approximation is found, or the region to which it is being
*     applied becomes too small (in which case the original Mapping is
*     used directly).

*  Parameters:
c     this
f     THIS = INTEGER (Given)
*        Pointer to a Mapping, whose inverse transformation will be
*        used to transform the coordinates of pixels in the output
*        grid into the coordinate system of the input grid. This
*        yields the positions which are used to obtain resampled
*        values by sub-pixel interpolation within the input grid.
*
*        The number of input coordinates used by this Mapping (as
*        given by its Nin attribute) should match the number of input
c        grid dimensions given by the value of "ndim_in"
f        grid dimensions given by the value of NDIM_IN
*        below. Similarly, the number of output coordinates (Nout
*        attribute) should match the number of output grid dimensions
c        given by "ndim_out".
f        given by NDIM_OUT.
c     ndim_in
f     NDIM_IN = INTEGER (Given)
*        The number of dimensions in the input grid. This should be at
*        least one.
c     lbnd_in
f     LBND_IN( NDIM_IN ) = INTEGER (Given)
c        Pointer to an array of integers, with "ndim_in" elements,
f        An array
*        containing the coordinates of the centre of the first pixel
*        in the input grid along each dimension.
c     ubnd_in
f     UBND_IN( NDIM_IN ) = INTEGER (Given)
c        Pointer to an array of integers, with "ndim_in" elements,
f        An array
*        containing the coordinates of the centre of the last pixel in
*        the input grid along each dimension.
*
c        Note that "lbnd_in" and "ubnd_in" together define the shape
f        Note that LBND_IN and UBND_IN together define the shape
*        and size of the input grid, its extent along a particular
c        (j'th) dimension being ubnd_in[j]-lbnd_in[j]+1 (assuming the
c        index "j" to be zero-based). They also define
f        (J'th) dimension being UBND_IN(J)-LBND_IN(J)+1. They also define
*        the input grid's coordinate system, each pixel having unit
*        extent along each dimension with integral coordinate values
*        at its centre.
c     in
f     IN( * ) = <Xtype> (Given)
c        Pointer to an array, with one element for each pixel in the
f        An array, with one element for each pixel in the
*        input grid, containing the input data to be resampled.  The
*        numerical type of this array should match the 1- or
*        2-character type code appended to the function name (e.g. if
c        you are using astResampleF, the type of each array element
c        should be "float").
f        you are using AST_RESAMPLER, the type of each array element
f        should be REAL).
*
*        The storage order of data within this array should be such
*        that the index of the first grid dimension varies most
*        rapidly and that of the final dimension least rapidly
c        (i.e. Fortran array indexing is used).
f        (i.e. normal Fortran array storage order).
c     in_var
f     IN_VAR( * ) = <Xtype> (Given)
c        An optional pointer to a second array with the same size and
c        type as the "in" array. If given, this should contain a set
c        of non-negative values which represent estimates of the
c        statistical variance associated with each element of the "in"
c        array. If this array is supplied (together with the
c        corresponding "out_var" array), then estimates of the
c        variance of the resampled output data will be calculated.
c
c        If no input variance estimates are being provided, a NULL
c        pointer should be given.
f        An optional second array with the same size and type as the
f        IN array. If the AST__USEVAR flag is set via the FLAGS
f        argument (below), this array should contain a set of
f        non-negative values which represent estimates of the
f        statistical variance associated with each element of the IN
f        array. Estimates of the variance of the resampled output data
f        will then be calculated.
f
f        If the AST__USEVAR flag is not set, no input variance
f        estimates are required and this array will not be used. A
f        dummy (e.g. one-element) array may then be supplied.
c     interp
f     INTERP = INTEGER (Given)
c        This parameter specifies the scheme to be used for sub-pixel
f        This argument specifies the scheme to be used for sub-pixel
*        interpolation within the input grid. It may be used to select
*        from a set of pre-defined schemes by supplying one of the
*        values described in the "Sub-Pixel Interpolation Schemes"
*        section below.  If a value of zero is supplied, then the
*        default linear interpolation scheme is used (equivalent to
*        supplying the value AST__LINEAR).
*
*        Alternatively, you may supply a value which indicates that
c        you will provide your own function to perform sub-pixel
c        interpolation by means of the "finterp " parameter. Again, see
f        you will provide your own routine to perform sub-pixel
f        interpolation by means of the FINTERP argument. Again, see
*        the "Sub-Pixel Interpolation Schemes" section below for
*        details.
c     finterp
f     FINTERP = SUBROUTINE (Given)
c        If the value given for the "interp" parameter indicates that
c        you will provide your own function for sub-pixel
c        interpolation, then a pointer to that function should be
c        given here. For details of the interface which the function
c        should have (several are possible, depending on the value of
c        "interp"), see the "Sub-Pixel Interpolation Schemes" section
c        below.
f        If the value given for the INTERP argument indicates that you
f        will provide your own routine for sub-pixel interpolation,
f        then the name of that routine should be given here (the name
f        should also appear in a Fortran EXTERNAL statement in the
f        routine which invokes AST_RESAMPLE<X>). For details of the
f        interface which the routine should have (several are
f        possible, depending on the value of INTERP), see the
f        "Sub-Pixel Interpolation Schemes" section below.
*
c        If the "interp" parameter has any other value, corresponding
c        to one of the pre-defined interpolation schemes, then this
c        function will not be used and you may supply a NULL pointer.
f        If the INTERP argument has any other value, corresponding to
f        one of the pre-defined interpolation schemes, then this
f        routine will not be used and you may supply the null routine
f        AST_NULL here (note only one underscore).  No EXTERNAL
f        statement is required for this routine, so long as the AST_PAR
f        include file has been used.
c     params
f     PARAMS( * ) = DOUBLE PRECISION (Given)
c        An optional pointer to an array of double which should contain
f        An optional array which should contain
*        any additional parameter values required by the sub-pixel
*        interpolation scheme. If such parameters are required, this
*        will be noted in the "Sub-Pixel Interpolation Schemes"
c        section below (you may also use this parameter to pass values
c        to your own interpolation function).
f        section below (you may also use this argument to pass values
f        to your own interpolation routine).
*
c        If no additional parameters are required, this array is not
c        used and a NULL pointer may be given.
f        If no additional parameters are required, this array is not
f        used. A dummy (e.g. one-element) array may then be supplied.
c     flags
f     FLAGS = INTEGER (Given)
c        The bitwise OR of a set of flag values which may be used to
f        The sum of a set of flag values which may be used to
*        provide additional control over the resampling operation. See
*        the "Control Flags" section below for a description of the
*        options available.  If no flag values are to be set, a value
*        of zero should be given.
c     tol
f     TOL = DOUBLE PRECISION (Given)
*        The maximum tolerable geometrical distortion which may be
*        introduced as a result of approximating non-linear Mappings
*        by a set of piece-wise linear transformations. This should be
*        expressed as a displacement in pixels in the input grid's
*        coordinate system.
*
*        If piece-wise linear approximation is not required, a value
*        of zero may be given. This will ensure that the Mapping is
*        used without any approximation, but may increase execution
*        time.
c     maxpix
f     MAXPIX = INTEGER (Given)
*        A value which specifies an initial scale size (in pixels) for
*        the adaptive algorithm which approximates non-linear Mappings
*        with piece-wise linear transformations. Normally, this should
*        be a large value (larger than any dimension of the region of
*        the output grid being used). In this case, a first attempt to
*        approximate the Mapping by a linear transformation will be
*        made over the entire output region.
*
*        If a smaller value is used, the output region will first be
c        divided into sub-regions whose size does not exceed "maxpix"
f        divided into sub-regions whose size does not exceed MAXPIX
*        pixels in any dimension. Only at this point will attempts at
*        approximation commence.
*
*        This value may occasionally be useful in preventing false
*        convergence of the adaptive algorithm in cases where the
*        Mapping appears approximately linear on large scales, but has
*        irregularities (e.g. holes) on smaller scales. A value of,
*        say, 50 to 100 pixels can also be employed as a safeguard in
*        general-purpose software, since the effect on performance is
*        minimal.
*
*        If too small a value is given, it will have the effect of
*        inhibiting linear approximation altogether (equivalent to
c        setting "tol" to zero). Although this may degrade
f        setting TOL to zero). Although this may degrade
*        performance, accurate results will still be obtained.
c     badval
f     BADVAL = <Xtype> (Given)
*        This argument should have the same type as the elements of
c        the "in" array. It specifies the value used to flag missing
f        the IN array. It specifies the value used to flag missing
*        data (bad pixels) in the input and output arrays.
*
c        If the AST__USEBAD flag is set via the "flags" parameter,
f        If the AST__USEBAD flag is set via the FLAGS argument,
c        then this value is used to test for bad pixels in the "in"
c        (and "in_var") array(s).
f        then this value is used to test for bad pixels in the IN
f        (and IN_VAR) array(s).
*
*        In all cases, this value is also used to flag any output
c        elements in the "out" (and "out_var") array(s) for which
f        elements in the OUT (and OUT_VAR) array(s) for which
*        resampled values could not be obtained (see the "Propagation
*        of Missing Data" section below for details of the
c        circumstances under which this may occur). The astResample<X>
f        circumstances under which this may occur). The AST_RESAMPLE<X>
*        function return value indicates whether any such values have
*        been produced.
c     ndim_out
f     NDIM_OUT = INTEGER (Given)
*        The number of dimensions in the output grid. This should be
*        at least one. It need not necessarily be equal to the number
*        of dimensions in the input grid.
c     lbnd_out
f     LBND_OUT( NDIM_OUT ) = INTEGER (Given)
c        Pointer to an array of integers, with "ndim_out" elements,
f        An array
*        containing the coordinates of the centre of the first pixel
*        in the output grid along each dimension.
c     ubnd_out
f     UBND_OUT( NDIM_OUT ) = INTEGER (Given)
c        Pointer to an array of integers, with "ndim_out" elements,
f        An array
*        containing the coordinates of the centre of the last pixel in
*        the output grid along each dimension.
*
c        Note that "lbnd_out" and "ubnd_out" together define the
f        Note that LBND_OUT and UBND_OUT together define the
*        shape, size and coordinate system of the output grid in the
c        same way as "lbnd_in" and "ubnd_in" define the shape, size
f        same way as LBND_IN and UBND_IN define the shape, size
*        and coordinate system of the input grid.
c     lbnd
f     LBND( NDIM_OUT ) = INTEGER (Given)
c        Pointer to an array of integers, with "ndim_out" elements,
f        An array
*        containing the coordinates of the first pixel in the region
*        of the output grid for which a resampled value is to be
*        calculated.
c     ubnd
f     UBND( NDIM_OUT ) = INTEGER (Given)
c        Pointer to an array of integers, with "ndim_out" elements,
f        An array
*        containing the coordinates of the last pixel in the region of
*        the output grid for which a resampled value is to be
*        calculated.
*
c        Note that "lbnd" and "ubnd" together define the shape and
f        Note that LBND and UBND together define the shape and
*        position of a (hyper-)rectangular region of the output grid
*        for which resampled values should be produced. This region
*        should lie wholly within the extent of the output grid (as
c        defined by the "lbnd_out" and "ubnd_out" arrays). Regions of
f        defined by the LBND_OUT and UBND_OUT arrays). Regions of
*        the output grid lying outside this region will not be
*        modified.
c     out
f     OUT( * ) = <Xtype> (Returned)
c        Pointer to an array, with one element for each pixel in the
f        An array, with one element for each pixel in the
*        output grid, into which the resampled data values will be
*        returned. The numerical type of this array should match that
c        of the "in" array, and the data storage order should be such
f        of the IN array, and the data storage order should be such
*        that the index of the first grid dimension varies most
*        rapidly and that of the final dimension least rapidly
c        (i.e. Fortran array indexing is used).
f        (i.e. normal Fortran array storage order).
c     out_var
f     OUT_VAR( * ) = <Xtype> (Returned)
c        An optional pointer to an array with the same type and size
c        as the "out" array. If given, this array will be used to
c        return variance estimates for the resampled data values. This
c        array will only be used if the "in_var" array has also been
c        supplied.
f        An optional array with the same type and size as the OUT
f        array. If the AST__USEVAR flag is set via the FLAGS argument,
f        this array will be used to return variance estimates for the
f        resampled data values.
*
*        The output variance values will be calculated on the
*        assumption that errors on the input data values are
*        statistically independent and that their variance estimates
*        may simply be summed (with appropriate weighting factors)
*        when several input pixels contribute to an output data
*        value. If this assumption is not valid, then the output error
*        estimates may be biased. In addition, note that the
*        statistical errors on neighbouring output data values (as
*        well as the estimates of those errors) may often be
*        correlated, even if the above assumption about the input data
*        is correct, because of the sub-pixel interpolation schemes
*        employed.
*
c        If no output variance estimates are required, a NULL pointer
c        should be given.
f        If the AST__USEVAR flag is not set, no output variance
f        estimates will be calculated and this array will not be
f        used. A dummy (e.g. one-element) array may then be supplied.
f     STATUS = INTEGER (Given and Returned)
f        The global status.

*  Returned Value:
c     astResample<X>()
f     AST_RESAMPLE<X> = INTEGER
*        The number of output pixels to which a data value (or a
c        variance value if relevant) equal to "badval" has been
f        variance value if relevant) equal to BADVAL has been
*        assigned because no valid resampled value could be obtained.
*        Thus, in the absence of any error, a returned value of zero
*        indicates that all the required output pixels received valid
*        resampled data values (and variances).

*  Notes:
*     - A value of zero will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.

*  Data Type Codes:
*     To select the appropriate resampling function, you should
c     replace <X> in the generic function name astResample<X> with a
f     replace <X> in the generic function name AST_RESAMPLE<X> with a
*     1- or 2-character data type code, so as to match the numerical
*     type <Xtype> of the data you are processing, as follows:
c     - D: double
c     - F: float
c     - L: long int
c     - UL: unsigned long int
c     - I: int
c     - UI: unsigned int
c     - S: short int
c     - US: unsigned short int
c     - B: byte (signed char)
c     - UB: unsigned byte (unsigned char)
f     - D: DOUBLE PRECISION
f     - R: REAL
f     - I: INTEGER
f     - UI: INTEGER (treated as unsigned)
f     - S: INTEGER*2 (short integer)
f     - US: INTEGER*2 (short integer, treated as unsigned)
f     - B: BYTE (treated as signed)
f     - UB: BYTE (treated as unsigned)
*
c     For example, astResampleD would be used to process "double"
c     data, while astResampleS would be used to process "short int"
c     data, etc.
f     For example, AST_RESAMPLED would be used to process DOUBLE
f     PRECISION data, while AST_RESAMPLES would be used to process
f     short integer data (stored in an INTEGER*2 array), etc.
f
f     For compatibility with other Starlink facilities, the codes W
f     and UW are provided as synonyms for S and US respectively (but
f     only in the Fortran interface to AST).

*  Sub-Pixel Interpolation Schemes:
*     There is no such thing as a perfect sub-pixel interpolation
*     scheme and, in practice, all resampling will result in some
*     degradation of gridded data.  A range of schemes is therefore
*     provided, from which you can choose the one which best suits
*     your needs.
*
*     In general, a compromise must be made between schemes which tend
*     to degrade sharp features in the data by smoothing them, and
*     those which attempt to preserve sharp features. The latter will
*     often tend to introduce unwanted oscillations, typically visible
*     as "ringing" around sharp features and edges, especially if the
*     data are under-sampled (i.e. the sharpest features are less than
*     about two pixels across). In practice, a good interpolation
*     scheme may exhibit some aspects of both these features.
*
*     For under-sampled data, some interpolation schemes may appear to
*     preserve data resolution because they transform single input
*     pixels into single output pixels, rather than spreading their
*     data between several output pixels. While this may look
*     cosmetically better, it can result in a geometrical shift of
*     sharp features in the data. You should beware of this if you
*     plan to use such features for (e.g.) image alignment.
*
*     The following are two easy-to-use sub-pixel interpolation
*     schemes which are generally applicable. They are selected by
c     supplying the appropriate value (defined in the "ast.h" header
c     file) via the "interp" parameter. In these cases, the "finterp"
c     and "params" parameters are not used:
f     supplying the appropriate value (defined in the AST_PAR include
f     file) via the INTERP argument. In these cases, the FINTERP
f     and PARAMS arguments are not used:
*
*     - AST__NEAREST: This is the simplest possible scheme, in which
*     the value of the input pixel with the nearest centre to the
*     interpolation point is used. This is very quick to execute and
*     will preserve single-pixel features in the data, but may
*     displace them by up to half their width along each dimension. It
*     often gives a good cosmetic result, so is useful for quick-look
*     processing, but is unsuitable if accurate geometrical
*     transformation is required.
*     - AST__LINEAR: This is the default scheme, which uses linear
*     interpolation between the nearest neighbouring pixels in the
*     input grid (there are two neighbours in one dimension, four
*     neighbours in two dimensions, eight in three dimensions,
*     etc.). It is superior to the nearest-pixel scheme (above) in not
*     displacing features in the data, yet it still executes fairly
*     rapidly. It is generally a safe choice if you do not have any
*     particular reason to favour another scheme, since it cannot
*     introduce oscillations. However, it does introduce some spatial
*     smoothing which varies according to the distance of the
*     interpolation point from the neighbouring pixels. This can
*     degrade the shape of sharp features in the data in a
*     position-dependent way. It may also show in the output variance
*     grid (if used) as a pattern of stripes or fringes.
*
*     An alternative set of interpolation schemes is based on forming
*     the interpolated value from the weighted sum of a set of
*     surrounding pixel values (not necessarily just the nearest
*     neighbours). This approach has its origins in the theory of
*     digital filtering, in which interpolated values are obtained by
*     conceptually passing the sampled data through a linear filter
*     which implements a convolution. Because the convolution kernel
*     is continuous, the convolution may then be evaluated at
*     fractional pixel positions.  The (possibly multi-dimensional)
*     kernel is usually regarded as "separable" and formed from the
*     product of a set of identical 1-dimensional kernel functions,
*     evaluated along each dimension. Different interpolation schemes
*     are then distinguished by the choice of this 1-dimensional
*     interpolation kernel. The number of surrounding pixels which
*     contribute to the result may also be varied.
*
*     From a practical standpoint, it is useful to divide the weighted
*     sum of pixel values by the sum of the weights when determining
*     the interpolated value.  Strictly, this means that a true
*     convolution is no longer being performed. However, the
*     distinction is rarely important in practice because (for
*     slightly subtle reasons) the sum of weights is always
*     approximately constant for good interpolation kernels. The
*     advantage of this technique, which is used here, is that it can
*     easily accommodate missing data and tends to minimise unwanted
*     oscillations at the edges of the data grid.
*
*     Unless otherwise noted in the following schemes (which are based
*     on a 1-dimensional interpolation kernel), the value of
c     "params[0]" should be used to specify how many pixels are to
f     PARAMS(1) should be used to specify how many pixels are to
*     contribute to the interpolated result on either side of the
*     interpolation point in each dimension (the nearest integer value
*     is used). Typically a value of 2 is appropriate (the execution
*     time increases rapidly with this number) and the minimum value
*     used will be 1 (i.e. one pixel on either side).
c     In these cases, the "finterp" parameter is not used:
f     In these cases, the FINTERP argument is not used:
*
*     - AST__SINC: This scheme uses the sinc(x) kernel, which
*     sometimes features as an "optimal" interpolation kernel in books
*     on image processing. Its supposed optimality depends on the
*     assumption that the data are band-limited (i.e. have no spatial
*     frequencies above a certain value) and adequately sampled. In
*     practice, astronomical data rarely meet these requirements. In
*     addition, high spatial frequencies are often present due to
*     (e.g.) image defects and cosmic ray events. Consequently,
*     substantial ringing can be experienced with this kernel. The
*     sinc(x) function also decays slowly with distance, so that many
*     surrounding pixels are required, causing poor performance.
*     Abruptly truncating the kernel (by using only a few neighbouring
*     pixels) improves performance and may reduce ringing, but a more
*     gradual truncation (as implemented by other kernels) is
*     generally to be preferred. This kernel is provided mainly so
*     that you can convince yourself not to use it!
*     - AST__SINCSINC: This scheme uses a rather better kernel, of the
*     form sinc(x).sinc(x/2) and uses only two neighbouring pixels on
c     each side of the interpolation point (the "params" array is
f     each side of the interpolation point (the PARAMS array is
*     therefore not used).  This is sometimes known as the Lanczos
*     kernel. It is a good general-purpose scheme, intermediate in its
*     visual effect on images between the AST__NEAREST and AST__LINEAR
*     cases. Although the kernel is slightly oscillatory, ringing
*     is adequately suppressed if the data are well sampled.
*
c     Finally, supplying the following values for "interp" allows you
c     to implement your own sub-pixel interpolation scheme by means of
c     your own function. You should supply a pointer to this function
c     via the "finterp" parameter:
f     Finally, supplying the following values for INTERP allows you to
f     implement your own sub-pixel interpolation scheme by means of
f     your own routine. You should supply the name of this routine via
f     the FINTERP argument:
*
c     - AST__UKERN1: In this scheme, you supply a function to evaluate
c     your own 1-dimensional interpolation kernel, which is then used
c     to perform sub-pixel interpolation (as described above). The
c     function you supply should have the same interface as the
c     fictitious astUkern1 function (q.v.).  In addition, a value
c     should be given via "params[0]" to specify the number of
c     neighbouring pixels which are to contribute to each interpolated
c     value (in the same way as for the pre-defined interpolation
c     schemes described above). Other elements of the "params" array
c     are available to pass values to your interpolation function.
f     - AST__UKERN1: In this scheme, you supply a routine to evaluate
f     your own 1-dimensional interpolation kernel, which is then used
f     to perform sub-pixel interpolation (as described above). The
f     routine you supply should have the same interface as the
f     fictitious AST_UKERN1 routine (q.v.).  In addition, a value
f     should be given via PARAMS(1) to specify the number of
f     neighbouring pixels which are to contribute to each interpolated
f     value (in the same way as for the pre-defined interpolation
f     schemes described above). Other elements of the PARAMS array
f     are available to pass values to your interpolation routine.
*
c     - AST__UINTERP: This is a completely general scheme, in which
c     your interpolation function has access to all of the input
c     data. This allows you to implement any interpolation algorithm
c     you choose, which could (for example) be non-linear, or
c     adaptive. In this case, the astResample<X> functions play no
c     role in the sub-pixel interpolation process and simply handle
c     the geometrical transformation of coordinates and other
c     housekeeping. The function you supply should have the same
c     interface as the fictitious astUinterp function (q.v.). In this
c     case, the "params" parameter is not used by astResample<X>, but
c     is available to pass values to your interpolation function.
f     - AST__UINTERP: This is a completely general scheme, in which
f     your interpolation routine has access to all of the input
f     data. This allows you to implement any interpolation algorithm
f     you choose, which could (for example) be non-linear, or
f     adaptive. In this case, the AST_RESAMPLE<X> functions play no
f     role in the sub-pixel interpolation process and simply handle
f     the geometrical transformation of coordinates and other
f     housekeeping. The routine you supply should have the same
f     interface as the fictitious AST_UINTERP routine (q.v.). In this
f     case, the PARAMS argument is not used by AST_RESAMPLE<X>, but
f     is available to pass values to your interpolation routine.

*  Control Flags:
c     The following flags are defined in the "ast.h" header file and
f     The following flags are defined in the AST_PAR include file and
*     may be used to provide additional control over the resampling
*     process. Having selected a set of flags, you should supply the
c     bitwise OR of their values via the "flags" parameter:
f     sum of their values via the FLAGS argument:
*
*     - AST__URESAMP1, 2, 3 & 4: A set of four flags which are
*     reserved for your own use. They may be used to pass private
c     information to any sub-pixel interpolation function which you
f     information to any sub-pixel interpolation routine which you
*     implement yourself. They are ignored by all the pre-defined
*     interpolation schemes.
*     - AST__USEBAD: Indicates that there may be bad pixels in the
*     input array(s) which must be recognised by comparing with the
c     value given for "badval" and propagated to the output array(s).
f     value given for BADVAL and propagated to the output array(s).
*     If this flag is not set, all input values are treated literally
c     and the "badval" value is only used for flagging output array
f     and the BADVAL value is only used for flagging output array
*     values.
f     - AST__USEVAR: Indicates that variance information should be
f     processed in order to provide estimates of the statistical error
f     associated with the resampled values. If this flag is not set,
f     no variance processing will occur and the IN_VAR and OUT_VAR
f     arrays will not be used. (Note that this flag is only available
f     in the Fortran interface to AST.)

*  Propagation of Missing Data:
*     Instances of missing data (bad pixels) in the output grid are
c     identified by occurrences of the "badval" value in the "out"
f     identified by occurrences of the BADVAL value in the OUT
*     array. These may be produced if any of the following happen:
*
*     - The input position (the transformed position of the output
*     pixel's centre) lies outside the boundary of the grid of input
*     pixels.
*     - The input position lies inside the boundary of a bad input
*     pixel. In this context, an input pixel is considered bad if its
c     data value is equal to "badval" and the AST__USEBAD flag is
c     set in the "flags" argument.
f     data value is equal to BADVAL and the AST__USEBAD flag is
f     set in the FLAGS argument.
*     (Positions which have half-integral coordinate values, and
*     therefore lie on a pixel boundary, are regarded as lying within
*     the pixel with the larger, i.e. more positive, index.)
*     - The set of neighbouring input pixels (excluding those which
*     are bad) is unsuitable for calculating an interpolated
*     value. Whether this is true may depend on the sub-pixel
*     interpolation scheme in use.
*     - The interpolated value lies outside the range which can be
c     represented using the data type of the "out" array.
f     represented using the data type of the OUT array.
*
*     In addition, associated output variance estimates (if
c     calculated) may be declared bad and flagged with the "badval"
c     value in the "out_var" array under any of the following
f     calculated) may be declared bad and flagged with the BADVAL
f     value in the OUT_VAR array under any of the following
*     circumstances:
*
c     - The associated resampled data value (in the "out" array) is bad.
f     - The associated resampled data value (in the OUT array) is bad.
*     - The set of neighbouring input pixels which contributed to the
*     output data value do not all have valid variance estimates
*     associated with them. In this context, an input variance
*     estimate may be regarded as bad either because it has the value
c     "badval" (and the AST__USEBAD flag is set), or because it is
f     BADVAL (and the AST__USEBAD flag is set), or because it is
*     negative.
*     - The set of neighbouring input pixels for which valid variance
*     values are available is unsuitable for calculating an overall
*     variance value. Whether this is true may depend on the sub-pixel
*     interpolation scheme in use.
*     - The variance value lies outside the range which can be
c     represented using the data type of the "out_var" array.
f     represented using the data type of the OUT_VAR array.
*--
*/
/* Define a macro to implement the function for a specific data
   type. */
#define MAKE_RESAMPLE(X,Xtype) \
static int Resample##X( AstMapping *this, int ndim_in, \
                        const int lbnd_in[], const int ubnd_in[], \
                        const Xtype in[], const Xtype in_var[], \
                        int interp, void (* finterp)(), \
                        const double params[], int flags, double tol, \
                        int maxpix, Xtype badval, \
                        int ndim_out, const int lbnd_out[], \
                        const int ubnd_out[], const int lbnd[], \
                        const int ubnd[], Xtype out[], Xtype out_var[] ) { \
\
/* Local Variables: */ \
   AstMapping *simple;           /* Pointer to simplified Mapping */ \
   int idim;                     /* Loop counter for coordinate dimensions */ \
   int nin;                      /* Number of Mapping input coordinates */ \
   int nout;                     /* Number of Mapping output coordinates */ \
   int npix;                     /* Number of pixels in output region */ \
   int result;                   /* Result value to return */ \
\
/* Initialise. */ \
   result = 0; \
\
/* Check the global error status. */ \
   if ( !astOK ) return result; \
\
/* Obtain values for the Nin and Nout attributes of the Mapping. */ \
   nin = astGetNin( this ); \
   nout = astGetNin( this ); \
\
/* If OK, check that the number of input grid dimensions matches the \
   number required by the Mapping and is at least 1. Report an error \
   if necessary. */ \
   if ( astOK && ( ( ndim_in != nin ) || ( ndim_in < 1 ) ) ) { \
      astError( AST__NGDIN, "astResample"#X"(%s): Bad number of input grid " \
                "dimensions (%d).", astGetClass( this ), ndim_in ); \
      if ( ndim_in != nin ) { \
         astError( AST__NGDIN, "The %s given requires %d coordinate value%s " \
                   "to specify an input position.", astGetClass( this ), nin, \
                   ( nin == 1 ) ? "" : "s" ); \
      } \
   } \
\
/* If OK, also check that the number of output grid dimensions matches \
   the number required by the Mapping and is at least 1. Report an \
   error if necessary. */ \
   if ( astOK && ( ( ndim_out != nout ) || ( ndim_out < 1 ) ) ) { \
      astError( AST__NGDIN, "astResample"#X"(%s): Bad number of output grid " \
                "dimensions (%d).", astGetClass( this ), ndim_out ); \
      if ( ndim_out != nout ) { \
         astError( AST__NGDIN, "The %s given generates %s%d coordinate " \
                   "value%s for each output position.", astGetClass( this ), \
                   ( nout < ndim_out ) ? "only " : "", nout, \
                   ( nout == 1 ) ? "" : "s" ); \
      } \
   } \
\
/* Check that the lower and upper bounds of the input grid are \
   consistent. Report an error if any pair is not. */ \
   if ( astOK ) { \
      for ( idim = 0; idim < ndim_in; idim++ ) { \
         if ( lbnd_in[ idim ] > ubnd_in[ idim ] ) { \
            astError( AST__GBDIN, "astResample"#X"(%s): Lower bound of " \
                      "input grid (%d) exceeds corresponding upper bound " \
                      "(%d).", astGetClass( this ), lbnd_in[ idim ], \
                      ubnd_in[ idim ] ); \
            astError( AST__GBDIN, "Error in input dimension %d.", \
                      idim + 1 ); \
            break; \
         } \
      } \
   } \
\
/* Check that the positional accuracy tolerance supplied is valid and \
   report an error if necessary. */ \
   if ( astOK && ( tol < 0.0 ) ) { \
      astError( AST__PATIN, "astResample"#X"(%s): Invalid positional " \
                "accuracy tolerance (%.*g pixel).", astGetClass( this ), \
                DBL_DIG, tol ); \
      astError( AST__PATIN, "This value should not be less than zero." ); \
   } \
\
/* Check that the maximum pixel interval supplied is valid and report \
   an error if necessary. */ \
   if ( astOK && ( maxpix < 0 ) ) { \
      astError( AST__MPIIN, "astResample"#X"(%s): Invalid maximum pixel " \
                "interval (%d).", astGetClass( this ), maxpix ); \
      astError( AST__MPIIN, "This value should not be less than zero." ); \
   } \
\
/* Check that the lower and upper bounds of the output grid are \
   consistent. Report an error if any pair is not. */ \
   if ( astOK ) { \
      for ( idim = 0; idim < ndim_out; idim++ ) { \
         if ( lbnd_out[ idim ] > ubnd_out[ idim ] ) { \
            astError( AST__GBDIN, "astResample"#X"(%s): Lower bound of " \
                      "output grid (%d) exceeds corresponding upper bound " \
                      "(%d).", astGetClass( this ), lbnd_out[ idim ], \
                      ubnd_out[ idim ] ); \
            astError( AST__GBDIN, "Error in output dimension %d.", \
                      idim + 1 ); \
            break; \
         } \
      } \
   } \
\
/* Similarly check the bounds of the output region. */ \
   if ( astOK ) { \
      for ( idim = 0; idim < ndim_out; idim++ ) { \
         if ( lbnd[ idim ] > ubnd[ idim ] ) { \
            astError( AST__GBDIN, "astResample"#X"(%s): Lower bound of " \
                      "output region (%d) exceeds corresponding upper " \
                      "bound (%d).", astGetClass( this ), lbnd[ idim ], \
                      ubnd[ idim ] ); \
\
/* Also check that the output region lies wholly within the output \
   grid. */ \
         } else if ( lbnd[ idim ] < lbnd_out[ idim ] ) { \
            astError( AST__GBDIN, "astResample"#X"(%s): Lower bound of " \
                      "output region (%d) is less than corresponding " \
                      "bound of output grid (%d).", astGetClass( this ), \
                      lbnd[ idim ], lbnd_out[ idim ] ); \
         } else if ( ubnd[ idim ] > ubnd_out[ idim ] ) { \
            astError( AST__GBDIN, "astResample"#X"(%s): Upper bound of " \
                      "output region (%d) exceeds corresponding " \
                      "bound of output grid (%d).", astGetClass( this ), \
                      ubnd[ idim ], ubnd_out[ idim ] ); \
         } \
\
/* Say which dimension produced the error. */ \
         if ( !astOK ) { \
            astError( AST__GBDIN, "Error in output dimension %d.", \
                      idim + 1 ); \
            break; \
         } \
      } \
   } \
\
/* If OK, loop to determine how many pixels require resampled values. */ \
   simple = NULL; \
   if ( astOK ) { \
      npix = 1; \
      for ( idim = 0; idim < ndim_out; idim++ ) { \
         npix *= ubnd[ idim ] - lbnd[ idim ] + 1; \
      } \
\
/* If there are sufficient pixels to make it worthwhile, simplify the \
   Mapping supplied to improve performance. Otherwise, just clone the \
   Mapping pointer. */ \
      if ( npix > 100 ) { \
         simple = astSimplify( this ); \
      } else { \
         simple = astClone( this ); \
      } \
   } \
\
/* Report an error if the inverse transformation of this simplified \
   Mapping is not defined. */ \
   if ( !astGetTranInverse( simple ) && astOK ) { \
      astError( AST__TRNND, "astResample"#X"(%s): An inverse coordinate " \
                "transformation is not defined by the %s supplied.", \
                astGetClass( this ), astGetClass( this ) ); \
   } \
\
/* Perform the resampling. Note that we pass all gridded data, the \
   interpolation function and the bad pixel value by means of pointer \
   types that obscure the underlying data type. This is to avoid \
   having to replicate functions unnecessarily for each data \
   type. However, we also pass an argument that identifies the data \
   type we have obscured. */ \
   result = ResampleAdaptively( simple, ndim_in, lbnd_in, ubnd_in, \
                                (const void *) in, (const void *) in_var, \
                                TYPE_##X, interp, (int (*)()) finterp, \
                                params, flags, tol, maxpix, \
                                (const void *) &badval, \
                                ndim_out, lbnd_out, ubnd_out, \
                                lbnd, ubnd, \
                                (void *) out, (void *) out_var ); \
\
/* Annul the pointer to the simplified/cloned Mapping. */ \
   simple = astAnnul( simple ); \
\
/* If an error occurred, clear the returned result. */ \
   if ( !astOK ) result = 0; \
\
/* Return the result. */ \
   return result; \
}

/* Expand the above macro to generate a function for each required
   data type. */
#if defined(AST_LONG_DOUBLE)     /* Not normally implemented */
MAKE_RESAMPLE(LD,long double)
#endif
MAKE_RESAMPLE(D,double)
MAKE_RESAMPLE(F,float)
MAKE_RESAMPLE(L,long int)
MAKE_RESAMPLE(UL,unsigned long int)
MAKE_RESAMPLE(I,int)
MAKE_RESAMPLE(UI,unsigned int)
MAKE_RESAMPLE(S,short int)
MAKE_RESAMPLE(US,unsigned short int)
MAKE_RESAMPLE(B,signed char)
MAKE_RESAMPLE(UB,unsigned char)

/* Undefine the macro. */
#undef MAKE_RESAMPLE

static int ResampleAdaptively( AstMapping *this, int ndim_in,
                               const int *lbnd_in, const int *ubnd_in,
                               const void *in, const void *in_var,
                               DataType type, int interp, int (* finterp)(),
                               const double *params, int flags, double tol,
                               int maxpix, const void *badval_ptr,
                               int ndim_out, const int *lbnd_out,
                               const int *ubnd_out, const int *lbnd,
                               const int *ubnd, void *out, void *out_var ) {
/*
*  Name:
*     ResampleAdaptively

*  Purpose:
*     Resample a section of a data grid adaptively.

*  Type:
*     Private function.

*  Synopsis:
*     #include "mapping.h"
*     int ResampleAdaptively( AstMapping *this, int ndim_in,
*                             const int *lbnd_in, const int *ubnd_in,
*                             const void *in, const void *in_var,
*                             DataType type, int interp, int (* finterp)(),
*                             const double *params, int flags, double tol,
*                             int maxpix, const void *badval_ptr,
*                             int ndim_out, const int *lbnd_out,
*                             const int *ubnd_out, const int *lbnd,
*                             const int *ubnd, void *out, void *out_var )

*  Class Membership:
*     Mapping member function.

*  Description:
*     This function resamples a rectangular grid of data (with any
*     number of dimensions) into a specified section of another
*     rectangular grid (with a possibly different number of
*     dimensions). The coordinate transformation used to convert
*     output pixel coordinates into positions in the input grid is
*     given by the inverse transformation of the Mapping which is
*     supplied.  Any pixel interpolation scheme may be specified for
*     interpolating between the pixels of the input grid.
*
*     This function is very similar to ResampleWithBlocking and
*     ResampleSection which lie below it in the calling
*     hierarchy. However, this function also attempts to adapt to the
*     Mapping supplied and to sub-divide the section being resampled
*     into smaller sections within which a linear approximation to the
*     Mapping may be used.  This reduces the number of Mapping
*     evaluations, thereby improving efficiency particularly when
*     complicated Mappings are involved.

*  Parameters:
*     this
*        Pointer to a Mapping, whose inverse transformation may be
*        used to transform the coordinates of pixels in the output
*        grid into associated positions in the input grid, from which
*        the output pixel values should be derived (by interpolation
*        if necessary).
*
*        The number of input coordintes for the Mapping (Nin
*        attribute) should match the value of "ndim_in" (below), and
*        the number of output coordinates (Nout attribute) should
*        match the value of "ndim_out".
*     ndim_in
*        The number of dimensions in the input grid. This should be at
*        least one.
*     lbnd_in
*        Pointer to an array of integers, with "ndim_in" elements.
*        This should give the coordinates of the centre of the first
*        pixel in the input data grid along each dimension.
*     ubnd_in
*        Pointer to an array of integers, with "ndim_in" elements.
*        This should give the coordinates of the centre of the last
*        pixel in the input data grid along each dimension.
*
*        Note that "lbnd_in" and "ubnd_in" together define the shape
*        and size of the input data grid, its extent along a
*        particular (i'th) dimension being (ubnd_in[i] - lbnd_in[i] +
*        1). They also define the input grid's coordinate system, with
*        each pixel being of unit extent along each dimension with
*        integral coordinate values at its centre.
*     in
*        Pointer to the input array of data to be resampled (with one
*        element for each pixel in the input grid). The numerical type
*        of these data should match the "type" value (below). The
*        storage order should be such that the coordinate of the first
*        dimension varies most rapidly and that of the final dimension
*        least rapidly (i.e. Fortran array storage order is used).
*     in_var
*        An optional pointer to a second array of positive numerical
*        values (with the same size and data type as the "in" array),
*        which represent estimates of the statistical variance
*        associated with each element of the "in" array. If this
*        second array is given (along with the corresponding "out_var"
*        array), then estimates of the variance of the resampled data
*        will also be returned.
*
*        If no variance estimates are required, a NULL pointer should
*        be given.
*     type
*        A value taken from the "DataType" enum, which specifies the
*        data type of the input and output arrays containing the
*        gridded data (and variance) values.
*     interp
*        A value selected from a set of pre-defined macros to identify
*        which sub-pixel interpolation algorithm should be used.
*     finterp
*        If "interp" is set to a value which requires a user-supplied
*        function, then a pointer to that function shoild be given
*        here. Otherwise, this value is not used and may be a NULL
*        pointer.
*     params
*        Pointer to an optional array of parameters that may be passed
*        to the interpolation algorithm, if required. If no parameters
*        are required, a NULL pointer should be supplied.
*     flags
*        The bitwise OR of a set of flag values which provide
*        additional control over the resampling operation.
*     tol
*        The maximum permitted positional error in transforming output
*        pixel positions into the input grid in order to resample
*        it. This should be expressed as a displacement in pixels in
*        the input grid's coordinate system. If the Mapping's inverse
*        transformation can be approximated by piecewise linear
*        functions to this accuracy, then such functions may be used
*        instead of the Mapping in order to improve
*        performance. Otherwise, every output pixel position will be
*        transformed individually using the Mapping.
*
*        If linear approximation is not required, a "tol" value of
*        zero may be given. This will ensure that the Mapping is used
*        without any approximation.
*     maxpix
*        A value which specifies the largest scale size on which to
*        search for non-linearities in the Mapping supplied. This
*        value should be expressed as a number of pixels in the output
*        grid. The function will break the output section specified
*        into smaller sub-sections (if necessary), each no larger than
*        "maxpix" pixels in any dimension, before it attempts to
*        approximate the Mapping by a linear function over each
*        sub-section.
* 
*        If the value given is larger than the largest dimension of
*        the output section (the normal recommendation), the function
*        will initially search for non-linearity on a scale determined
*        by the size of the output section.  This is almost always
*        satisfactory. Very occasionally, however, a Mapping may
*        appear linear on this scale but nevertheless have smaller
*        irregularities (e.g. "holes") in it.  In such cases, "maxpix"
*        may be set to a suitably smaller value so as to ensure this
*        non-linearity is not overlooked. Typically, a value of 50 to
*        100 pixels might be suitable and should have little effect on
*        performance.
*
*        If too small a value is given, however, it will have the
*        effect of preventing linear approximation occurring at all
*        (equivalent to setting "tol" to zero).  Although this may
*        degrade performance, accurate results will still be obtained.
*     badval_ptr
*        If the AST__USEBAD flag is set (above), this parameter is a
*        pointer to a value which is used to identify bad data and/or
*        variance values in the input array(s). The referenced value's
*        data type must match that of the "in" (and "in_var")
*        arrays. The same value will also be used to flag any output
*        array elements for which resampled values could not be
*        obtained.  The output arrays(s) may be flagged with this
*        value whether or not the AST__USEBAD flag is set (the
*        function return value indicates whether any such values have
*        been produced).
*     ndim_out
*        The number of dimensions in the output grid. This should be
*        at least one.
*     lbnd_out
*        Pointer to an array of integers, with "ndim_out" elements.
*        This should give the coordinates of the centre of the first
*        pixel in the output data grid along each dimension.
*     ubnd_out
*        Pointer to an array of integers, with "ndim_out" elements.
*        This should give the coordinates of the centre of the last
*        pixel in the output data grid along each dimension.
*
*        Note that "lbnd_out" and "ubnd_out" together define the shape
*        and size of the output data grid in the same way as "lbnd_in"
*        and "ubnd_in" define the shape and size of the input grid
*        (see above).
*     lbnd
*        Pointer to an array of integers, with "ndim_out" elements.
*        This should give the coordinates of the first pixel in the
*        section of the output data grid for which a value is
*        required.
*     ubnd
*        Pointer to an array of integers, with "ndim_out" elements.
*        This should give the coordinates of the last pixel in the
*        section of the output data grid for which a value is
*        required.
*
*        Note that "lbnd" and "ubnd" define the shape and position of
*        the section of the output grid for which resampled values are
*        required. This section should lie wholly within the extent of
*        the output grid (as defined by the "lbnd_out" and "ubnd_out"
*        arrays). Regions of the output grid lying outside this section
*        will not be modified.
*     out
*        Pointer to an array with the same data type as the "in"
*        array, into which the resampled data will be returned.  The
*        storage order should be such that the coordinate of the first
*        dimension varies most rapidly and that of the final dimension
*        least rapidly (i.e. Fortran array storage order is used).
*     out_var
*        An optional pointer to an array with the same data type and
*        size as the "out" array, into which variance estimates for
*        the resampled values may be returned. This array will only be
*        used if the "in_var" array has been given.
*
*        If no output variance estimates are required, a NULL pointer
*        should be given.

*  Returned Value:
*     The number of output grid points to which a data value (or a
*     variance value if relevant) equal to "badval" has been assigned
*     because no valid output value could be obtained.

*  Notes:
*     - A value of zero will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*/
                      
/* Local Variables: */
   double *linear_fit;           /* Pointer to array of fit coefficients */
   int *hi;                      /* Pointer to array of section upper bounds */
   int *lo;                      /* Pointer to array of section lower bounds */
   int coord_out;                /* Loop counter for output coordinates */
   int dim;                      /* Output section dimension size */
   int dimx;                     /* Dimension with maximum section extent */
   int divide;                   /* Sub-divide the output section? */
   int mxdim;                    /* Largest output section dimension size */
   int npix;                     /* Number of pixels in output section */
   int npoint;                   /* Number of points for obtaining a fit */
   int nvertex;                  /* Number of vertices of output section */
   int result;                   /* Result value to return */
   int toobig;                   /* Section too big (must sub-divide)? */
   int toosmall;                 /* Section too small to sub-divide? */

/* Initialise. */
   result = 0;

/* Check the global error status. */
   if ( !astOK ) return result;

/* Further initialisation. */
   npix = 1;
   mxdim = 0;
   dimx = 1;
   nvertex = 1;

/* Loop through the output grid dimensions. */
   for ( coord_out = 0; coord_out < ndim_out; coord_out++ ) {

/* Obtain the extent in each dimension of the output section which is
   to receive resampled values, and calculate the total number of
   pixels it contains. */
      dim = ubnd[ coord_out ] - lbnd[ coord_out ] + 1;
      npix *= dim;

/* Find the maximum dimension size of this output section and note
   which dimension has this size. */
      if ( dim > mxdim ) {
         mxdim = dim;
         dimx = coord_out;
      }

/* Calculate how many vertices the output section has. */
      nvertex *= 2;
   }
   
/* Calculate how many sample points will be needed (by the
   LinearApprox function) to obtain a linear fit to the Mapping's
   inverse transformation. */
   npoint = 1 + 4 * ndim_out + 2 * nvertex;

/* If the number of pixels in the output section is not at least 4
   times this number, we will probably not save significant time by
   attempting to obtain a linear fit, so note that the output section
   is too small. */
   toosmall = ( npix < ( 4 * npoint ) );

/* Note if the maximum dimension of the output section exceeds the
   user-supplied scale factor. */
   toobig = ( maxpix < mxdim );

/* Assume the Mapping is significantly non-linear before deciding
   whether to sub-divide the output section. */
   linear_fit = NULL;

/* If the output section is too small to be worth obtaining a linear
   fit, or if the accuracy tolerance is zero, we will not
   sub-divide. This means that the Mapping will be used to transform
   each pixel's coordinates and no linear approximation will be
   used. */
   if ( toosmall || ( tol == 0.0 ) ) {
      divide = 0;

/* Otherwise, if the largest output section dimension exceeds the
   scale length given, we will sub-divide. This offers the possibility
   of obtaining a linear approximation to the Mapping over a reduced
   range of output coordinates (which will be handled by a recursive
   invocation of this function). */
   } else if ( toobig ) {
      divide = 1;

/* If neither of the above apply, then attempt to fit a linear
   approximation to the Mapping's inverse transformation over the
   range of coordinates covered by the output section. */
   } else {
      linear_fit = LinearApprox( this, ndim_in, ndim_out, lbnd, ubnd, tol );

/* If a linear fit was obtained, we will use it and therefore do not
   wish to sub-divide further. Otherwise, we sub-divide in the hope
   that this may result in a linear fit next time. */
      divide = !linear_fit;
   }

/* If no sub-division is required, perform resampling (in a
   memory-efficient manner, since the section we are resampling might
   still be very large). This will use the linear fit, if obtained
   above. */
   if ( astOK ) {
      if ( !divide ) {
         result = ResampleWithBlocking( this, linear_fit,
                                        ndim_in, lbnd_in, ubnd_in,
                                        in, in_var, type, interp, finterp,
                                        params, flags, badval_ptr,
                                        ndim_out, lbnd_out, ubnd_out,
                                        lbnd, ubnd, out, out_var );

/* Otherwise, allocate workspace to perform the sub-division. */
      } else {
         lo = astMalloc( sizeof( int ) * (size_t) ndim_out );
         hi = astMalloc( sizeof( int ) * (size_t) ndim_out );
         if ( astOK ) {

/* Initialise the bounds of a new output section to match the original
   output section. */
            for ( coord_out = 0; coord_out < ndim_out; coord_out++ ) {
               lo[ coord_out ] = lbnd[ coord_out ];
               hi[ coord_out ] = ubnd[ coord_out ];
            }

/* Replace the upper bound of the section's largest dimension with the
   mid-point of the section along this dimension, rounded
   downwards. */
            hi[ dimx ] =
               (int) floor( 0.5 * (double) ( lbnd[ dimx ] + ubnd[ dimx ] ) );

/* Resample the resulting smaller section using a recursive invocation
   of this function. */
            result = ResampleAdaptively( this, ndim_in, lbnd_in, ubnd_in,
                                         in, in_var, type, interp, finterp,
                                         params, flags, tol, maxpix,
                                         badval_ptr, ndim_out,
                                         lbnd_out, ubnd_out,
                                         lo, hi, out, out_var );

/* Now set up a second section which covers the remaining half of the
   original output section. */
            lo[ dimx ] = hi[ dimx ] + 1;
            hi[ dimx ] = ubnd[ dimx ];

/* If this section contains pixels, resample it in the same way,
   summing the returned values. */
            if ( lo[ dimx ] <= hi[ dimx ] ) {
               result += ResampleAdaptively( this, ndim_in, lbnd_in, ubnd_in,
                                             in, in_var, type, interp, finterp,
                                             params, flags, tol, maxpix,
                                             badval_ptr,  ndim_out,
                                             lbnd_out, ubnd_out,
                                             lo, hi, out, out_var );
            }
         }

/* Free the workspace. */
         lo = astFree( lo );
         hi = astFree( hi );
      }
   }

/* If coefficients for a linear fit were obtained, then free the space
   they occupy. */
   if ( linear_fit ) linear_fit = astFree( linear_fit );

/* If an error occurred, clear the returned result. */
   if ( !astOK ) result = 0;

/* Return the result. */
   return result;
}

static int ResampleSection( AstMapping *this, const double *linear_fit,
                            int ndim_in,
                            const int *lbnd_in, const int *ubnd_in,
                            const void *in, const void *in_var,
                            DataType type, int interp, int (* finterp)(),
                            const double *params, int flags,
                            const void *badval_ptr, int ndim_out,
                            const int *lbnd_out, const int *ubnd_out,
                            const int *lbnd, const int *ubnd,
                            void *out, void *out_var ) {
/*
*  Name:
*     ResampleSection

*  Purpose:
*     Resample a section of a data grid.

*  Type:
*     Private function.

*  Synopsis:
*     #include "mapping.h"
*     int ResampleSection( AstMapping *this, const double *linear_fit,
*                          int ndim_in, const int *lbnd_in, const int *ubnd_in,
*                          const void *in, const void *in_var,
*                          DataType type, int interp, int (* finterp)(),
*                          const double *params, int flags,
*                          const void *badval_ptr, int ndim_out,
*                          const int *lbnd_out, const int *ubnd_out,
*                          const int *lbnd, const int *ubnd,
*                          void *out, void *out_var )

*  Class Membership:
*     Mapping member function.

*  Description:
*     This function resamples a rectangular grid of data (with any
*     number of dimensions) into a specified section of another
*     rectangular grid (with a possibly different number of
*     dimensions). The coordinate transformation used is given by the
*     inverse transformation of the Mapping which is supplied or,
*     alternatively, by a linear approximation fitted to a Mapping's
*     inverse transformation. Any pixel interpolation scheme may be
*     specified for interpolating between the pixels of the input
*     grid.

*  Parameters:
*     this
*        Pointer to a Mapping, whose inverse transformation may be
*        used to transform the coordinates of pixels in the output
*        grid into associated positions in the input grid, from which
*        the output pixel values should be derived (by interpolation
*        if necessary).
*
*        The number of input coordintes for the Mapping (Nin
*        attribute) should match the value of "ndim_in" (below), and
*        the number of output coordinates (Nout attribute) should
*        match the value of "ndim_out".
*     linear_fit
*        Pointer to an optional array of double which contains the
*        coefficients of a linear fit which approximates the above
*        Mapping's inverse coordinate transformation. If this is
*        supplied, it will be used in preference to the above Mapping
*        when transforming coordinates. This may be used to enhance
*        performance in cases where evaluation of the Mapping's
*        inverse transformation is expensive. If no linear fit is
*        available, a NULL pointer should be supplied.
*
*        The way in which the fit coefficients are stored in this
*        array and the number of array elements are as defined by the
*        LinearApprox function.
*     ndim_in
*        The number of dimensions in the input grid. This should be at
*        least one.
*     lbnd_in
*        Pointer to an array of integers, with "ndim_in" elements.
*        This should give the coordinates of the centre of the first
*        pixel in the input data grid along each dimension.
*     ubnd_in
*        Pointer to an array of integers, with "ndim_in" elements.
*        This should give the coordinates of the centre of the last
*        pixel in the input data grid along each dimension.
*
*        Note that "lbnd_in" and "ubnd_in" together define the shape
*        and size of the input data grid, its extent along a
*        particular (i'th) dimension being (ubnd_in[i] - lbnd_in[i] +
*        1). They also define the input grid's coordinate system, with
*        each pixel being of unit extent along each dimension with
*        integral coordinate values at its centre.
*     in
*        Pointer to the input array of data to be resampled (with one
*        element for each pixel in the input grid). The numerical type
*        of these data should match the "type" value (below). The
*        storage order should be such that the coordinate of the first
*        dimension varies most rapidly and that of the final dimension
*        least rapidly (i.e. Fortran array storage order is used).
*     in_var
*        An optional pointer to a second array of positive numerical
*        values (with the same size and data type as the "in" array),
*        which represent estimates of the statistical variance
*        associated with each element of the "in" array. If this
*        second array is given (along with the corresponding "out_var"
*        array), then estimates of the variance of the resampled data
*        will also be returned.
*
*        If no variance estimates are required, a NULL pointer should
*        be given.
*     type
*        A value taken from the "DataType" enum, which specifies the
*        data type of the input and output arrays containing the
*        gridded data (and variance) values.
*     interp
*        A value selected from a set of pre-defined macros to identify
*        which sub-pixel interpolation algorithm should be used.
*     finterp
*        If "interp" is set to a value which requires a user-supplied
*        function, then a pointer to that function shoild be given
*        here. Otherwise, this value is not used and may be a NULL
*        pointer.
*     params
*        Pointer to an optional array of parameters that may be passed
*        to the interpolation algorithm, if required. If no parameters
*        are required, a NULL pointer should be supplied.
*     flags
*        The bitwise OR of a set of flag values which provide
*        additional control over the resampling operation.
*     badval_ptr
*        If the AST__USEBAD flag is set (above), this parameter is a
*        pointer to a value which is used to identify bad data and/or
*        variance values in the input array(s). The referenced value's
*        data type must match that of the "in" (and "in_var")
*        arrays. The same value will also be used to flag any output
*        array elements for which resampled values could not be
*        obtained.  The output arrays(s) may be flagged with this
*        value whether or not the AST__USEBAD flag is set (the
*        function return value indicates whether any such values have
*        been produced).
*     ndim_out
*        The number of dimensions in the output grid. This should be
*        at least one.
*     lbnd_out
*        Pointer to an array of integers, with "ndim_out" elements.
*        This should give the coordinates of the centre of the first
*        pixel in the output data grid along each dimension.
*     ubnd_out
*        Pointer to an array of integers, with "ndim_out" elements.
*        This should give the coordinates of the centre of the last
*        pixel in the output data grid along each dimension.
*
*        Note that "lbnd_out" and "ubnd_out" together define the shape
*        and size of the output data grid in the same way as "lbnd_in"
*        and "ubnd_in" define the shape and size of the input grid
*        (see above).
*     lbnd
*        Pointer to an array of integers, with "ndim_out" elements.
*        This should give the coordinates of the first pixel in the
*        section of the output data grid for which a value is
*        required.
*     ubnd
*        Pointer to an array of integers, with "ndim_out" elements.
*        This should give the coordinates of the last pixel in the
*        section of the output data grid for which a value is
*        required.
*
*        Note that "lbnd" and "ubnd" define the shape and position of
*        the section of the output grid for which resampled values are
*        required. This section should lie wholly within the extent of
*        the output grid (as defined by the "lbnd_out" and "ubnd_out"
*        arrays). Regions of the output grid lying outside this section
*        will not be modified.
*     out
*        Pointer to an array with the same data type as the "in"
*        array, into which the resampled data will be returned.  The
*        storage order should be such that the coordinate of the first
*        dimension varies most rapidly and that of the final dimension
*        least rapidly (i.e. Fortran array storage order is used).
*     out_var
*        An optional pointer to an array with the same data type and
*        size as the "out" array, into which variance estimates for
*        the resampled values may be returned. This array will only be
*        used if the "in_var" array has been given.
*
*        If no output variance estimates are required, a NULL pointer
*        should be given.

*  Returned Value:
*       The number of output grid points to which a data value (or a
*       variance value if relevant) equal to "badval" has been
*       assigned because no valid output value could be obtained.

*  Notes:
*     - This function does not take steps to limit memory usage if the
*     grids supplied are large. To resample large grids in a more
*     memory-efficient way, the ResampleWithBlocking function should
*     be used.
*     - A value of zero will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*/

/* Local Variables: */
   AstPointSet *pset_in;         /* Input PointSet for transformation */
   AstPointSet *pset_out;        /* Output PointSet for transformation */
   const double *grad;           /* Pointer to gradient matrix of linear fit */
   const double *zero;           /* Pointer to zero point array of fit */
   double **ptr_in;              /* Pointer to input PointSet coordinates */
   double **ptr_out;             /* Pointer to output PointSet coordinates */
   double *accum;                /* Pointer to array of accumulated sums */
   double x1;                    /* Interim x coordinate value */
   double y1;                    /* Interim y coordinate value */
   int *dim;                     /* Pointer to array of output pixel indices */
   int *offset;                  /* Pointer to array of output pixel offsets */
   int *stride;                  /* Pointer to array of output grid strides */
   int coord_in;                 /* Loop counter for input dimensions */
   int coord_out;                /* Loop counter for output dimensions */
   int done;                     /* All pixel indices done? */
   int i1;                       /* Interim offset into "accum" array */
   int i2;                       /* Final offset into "accum" array */
   int idim;                     /* Loop counter for dimensions */
   int ix;                       /* Loop counter for output x coordinate */
   int iy;                       /* Loop counter for output y coordinate */
   int nbad;                     /* Number of pixels assigned a bad value */
   int neighb;                   /* Number of neighbouring pixels */
   int npoint;                   /* Number of output points (pixels) */
   int off1;                     /* Interim pixel offset into output array */
   int off;                      /* Final pixel offset into output array */
   int point;                    /* Counter for output points (pixels ) */
   int result;                   /* Result value to be returned */
   int s;                        /* Temporaty variable for strides */
   int usevar;                   /* Process variance array? */
   void (* kernel)( double, const double [], int, double * ); /* Kernel fn. */

/* Initialise. */
   result = 0;

/* Check the global error status. */
   if ( !astOK ) return result;

/* Further initialisation. */
   pset_in = NULL;

/* Calculate the number of output points, as given by the product of
   the output grid dimensions. */
   for ( npoint = 1, coord_out = 0; coord_out < ndim_out; coord_out++ ) {
      npoint *= ubnd[ coord_out ] - lbnd[ coord_out ] + 1;
   }

/* Allocate workspace. */
   offset = astMalloc( sizeof( int ) * (size_t) npoint );
   stride = astMalloc( sizeof( int ) * (size_t) ndim_out );
   if ( astOK ) {

/* Calculate the stride for each output grid dimension. */
      off = 0;
      s = 1;
      for ( coord_out = 0; coord_out < ndim_out; coord_out++ ) {
         stride[ coord_out ] = s;
         s *= ubnd_out[ coord_out ] - lbnd_out[ coord_out ] + 1;
      }

/* A linear fit to the Mapping is available. */
/* ========================================= */
      if ( linear_fit ) {

/* If a linear fit to the Mapping has been provided, then obtain
   pointers to the array of gradients and zero-points comprising the
   fit. */
         grad = linear_fit + ndim_out;
         zero = linear_fit;

/* Create a PointSet to hold the input grid coordinates and obtain an
   array of pointers to its coordinate data. */
         pset_in = astPointSet( npoint, ndim_in, "" );
         ptr_in = astGetPoints( pset_in );
         if ( astOK ) {

/* Initialise the count of output points. */
            point = 0;

/* Handle the 1-dimensional case optimally. */
/* ---------------------------------------- */
            if ( ( ndim_in == 1 ) && ( ndim_out == 1 ) ) {

/* Loop through the pixels of the output grid and transform their x
   coordinates into the input grid's coordinate system using the
   linear fit supplied. Store the results in the PointSet created
   above. */
               for ( ix = lbnd[ 0 ]; ix <= ubnd[ 0 ]; ix++ ) {
                  ptr_in[ 0 ][ point ] = zero[ 0 ] + grad[ 0 ] * (double) ix;

/* Calculate the offset of each pixel within the output array. */
                  offset[ point ] = ix - lbnd_out[ 0 ];
                  point++;
               }

/* Handle the 2-dimensional case optimally. */
/* ---------------------------------------- */
            } else if ( ( ndim_in == 2 ) && ( ndim_out == 2 ) ) {

/* Loop through the range of y coordinates in the output grid and
   calculate interim values of the input coordinates using the linear
   fit supplied. */
               for ( iy = lbnd[ 1 ]; iy <= ubnd[ 1 ]; iy++ ) {
                  x1 = zero[ 0 ] + grad[ 1 ] * (double) iy;
                  y1 = zero[ 1 ] + grad[ 3 ] * (double) iy;

/* Also calculate an interim pixel offset into the output array. */
                  off1 = stride[ 1 ] * ( iy - lbnd_out[ 1 ] ) - lbnd_out[ 0 ];

/* Now loop through the range of output x coordinates and calculate
   the final values of the input coordinates, storing the results in
   the PointSet created above. */
                  for ( ix = lbnd[ 0 ]; ix <= ubnd[ 0 ]; ix++ ) {
                     ptr_in[ 0 ][ point ] = x1 + grad[ 0 ] * (double) ix;
                     ptr_in[ 1 ][ point ] = y1 + grad[ 2 ] * (double) ix;

/* Also calculate final pixel offsets into the output array. */
                     offset[ point ] = off1 + ix;
                     point++;
                  }
               }

/* Handle other numbers of dimensions. */
/* ----------------------------------- */               
            } else {

/* Allocate workspace. */
               accum = astMalloc( sizeof( double ) *
                                 (size_t) ( ndim_in * ndim_out ) );
               dim = astMalloc( sizeof( int ) * (size_t) ndim_out );
               if ( astOK ) {

/* Initialise an array of pixel indices for the output grid which
   refer to the first pixel for which we require a value. Also
   calculate the offset of this pixel within the output array. */
                  off = 0;
                  for ( coord_out = 0; coord_out < ndim_out; coord_out++ ) {
                     dim[ coord_out ] = lbnd[ coord_out ];
                     off += stride[ coord_out ] *
                            ( dim[ coord_out ] - lbnd_out[ coord_out ] );
                  }

/* To calculate each input grid coordinate we must perform a matrix
   multiply on the output grid coordinates (using the gradient matrix)
   and then add the zero points. However, since we will usually only
   be altering one output coordinate at a time (the least
   significant), we can avoid the full matrix multiply by accumulating
   partial sums for the most significant output coordinates and only
   altering those sums which need to change each time. The zero points
   never change, so we first fill the "most significant" end of the
   "accum" array with these. */
                  for ( coord_in = 0; coord_in < ndim_in; coord_in++ ) {
                     accum[ ( coord_in + 1 ) * ndim_out - 1 ] =
                                                              zero[ coord_in ];
                  }
                  coord_out = ndim_out - 1;

/* Now loop to process each output pixel. */
                  for ( done = 0; !done; point++ ) {

/* To generate the input coordinate that corresponds to the current
   output pixel, we work down from the most significant dimension
   whose index has changed since the previous pixel we considered
   (given by "coord_out"). For each affected dimension, we accumulate
   in "accum" the matrix sum (including the zero point) for that
   dimension and all higher output dimensions. We must accumulate a
   separate set of sums for each input coordinate we wish to
   produce. (Note that for the first pixel we process, all dimensions
   are considered "changed", so we start by initialising the whole
   "accum" array.) */
                     for ( coord_in = 0; coord_in < ndim_in; coord_in++ ) {
                        i1 = coord_in * ndim_out;
                        for ( idim = coord_out; idim >= 1; idim-- ) {
                           i2 = i1 + idim;
                           accum[ i2 - 1 ] = accum[ i2 ] +
                                             dim[ idim ] * grad[ i2 ];
                        }

/* The input coordinate for each dimension is given by the accumulated
   sum for output dimension zero (giving the sum over all output
   dimensions). We do not store this in the "accum" array, but assign
   the result directly to the coordinate array of the PointSet created
   earlier. */
                        ptr_in[ coord_in ][ point ] = accum[ i1 ] +
                                                      dim[ 0 ] * grad[ i1 ];
                     }

/* Store the offset of the current pixel in the output array. */
                     offset[ point ] = off;

/* Now update the array of pixel indices to refer to the next output
   pixel. */
                     coord_out = 0;
                     do {

/* The least significant index which currently has less than its
   maximum value is incremented by one. The offset into the output
   array is updated accordingly. */
                        if ( dim[ coord_out ] < ubnd[ coord_out ] ) {
                           dim[ coord_out ]++;
                           off += stride[ coord_out ];
                           break;

/* Any less significant indices which have reached their maximum value
   are returned to their minimum value and the output pixel offset is
   decremented appropriately. */
                        } else {
                           dim[ coord_out ] = lbnd[ coord_out ];
                           off -= stride[ coord_out ] *
                                  ( ubnd[ coord_out ] - lbnd[ coord_out ] );

/* All the output pixels have been processed once the most significant
   pixel index has been returned to its minimum value. */
                           done = ( ++coord_out == ndim_out );
                        }
                     } while ( !done );
                  }
               }

/* Free the workspace. */
               accum = astFree( accum );
               dim = astFree( dim );
            }
         }

/* No linear fit to the Mapping is available. */
/* ========================================== */
      } else {

/* Create a PointSet to hold the coordinates of the output pixels and
   obtain a pointer to its coordinate data. */
         pset_out = astPointSet( npoint, ndim_out, "" );
         ptr_out = astGetPoints( pset_out );
         if ( astOK ) {

/* Initialise the count of output points. */
            point = 0;

/* Handle the 1-dimensional case optimally. */
/* ---------------------------------------- */
            if ( ndim_out == 1 ) {

/* Loop through the required range of output x coordinates, assigning
   the coordinate values to the PointSet created above. Also store a
   pixel offset into the output array. */
               for ( ix = lbnd[ 0 ]; ix <= ubnd[ 0 ]; ix++ ) {
                  ptr_out[ 0 ][ point ] = (double) ix;
                  offset[ point ] = ix - lbnd_out[ 0 ];

/* Increment the count of output pixels. */
                  point++;
               }

/* Handle the 2-dimensional case optimally. */
/* ---------------------------------------- */
            } else if ( ndim_out == 2 ) {

/* Loop through the required range of output y coordinates,
   calculating an interim pixel offset into the output array. */
               for ( iy = lbnd[ 1 ]; iy <= ubnd[ 1 ]; iy++ ) {
                  off1 = stride[ 1 ] * ( iy - lbnd_out[ 1 ] ) - lbnd_out[ 0 ];

/* Loop through the required range of output x coordinates, assigning
   the coordinate values to the PointSet created above. Also store a
   final pixel offset into the output array. */
                  for ( ix = lbnd[ 0 ]; ix <= ubnd[ 0 ]; ix++ ) {
                     ptr_out[ 0 ][ point ] = (double) ix;
                     ptr_out[ 1 ][ point ] = (double) iy;
                     offset[ point ] = off1 + ix;

/* Increment the count of output pixels. */
                     point++;
                  }
               }

/* Handle other numbers of dimensions. */
/* ----------------------------------- */
            } else {

/* Allocate workspace. */
               dim = astMalloc( sizeof( int ) * (size_t) ndim_out );
               if ( astOK ) {

/* Initialise an array of pixel indices for the output grid which
   refer to the first pixel for which we require a value. Also
   calculate the offset of this pixel within the output array. */
                  off = 0;
                  for ( coord_out = 0; coord_out < ndim_out; coord_out++ ) {
                     dim[ coord_out ] = lbnd[ coord_out ];
                     off += stride[ coord_out ] *
                            ( dim[ coord_out ] - lbnd_out[ coord_out ] );
                  }

/* Loop to generate the coordinates of each output pixel. */
                  for ( done = 0; !done; point++ ) {

/* Copy each pixel's coordinates into the PointSet created above. */
                     for ( coord_out = 0; coord_out < ndim_out; coord_out++ ) {
                        ptr_out[ coord_out ][ point ] =
                                                     (double) dim[ coord_out ];
                     }

/* Store the offset of the pixel in the output array. */
                     offset[ point ] = off;

/* Now update the array of pixel indices to refer to the next output
   pixel. */
                     coord_out = 0;
                     do {

/* The least significant index which currently has less than its
   maximum value is incremented by one. The offset into the output
   array is updated accordingly. */
                        if ( dim[ coord_out ] < ubnd[ coord_out ] ) {
                           dim[ coord_out ]++;
                           off += stride[ coord_out ];
                           break;

/* Any less significant indices which have reached their maximum value
   are returned to their minimum value and the output pixel offset is
   decremented appropriately. */
                        } else {
                           dim[ coord_out ] = lbnd[ coord_out ];
                           off -= stride[ coord_out ] *
                                  ( ubnd[ coord_out ] - lbnd[ coord_out ] );

/* All the output pixels have been processed once the most significant
   pixel index has been returned to its minimum value. */
                           done = ( ++coord_out == ndim_out );
                        }
                     } while ( !done );
                  }
               }

/* Free the workspace. */
               dim = astFree( dim );
            }

/* When all the output pixel coordinates have been generated, use the
   Mapping's inverse transformation to generate the input coordinates
   from them. Obtain an array of pointers to the resulting coordinate
   data. */
            pset_in = astTransform( this, pset_out, 0, NULL );
            ptr_in = astGetPoints( pset_in );
         }

/* Annul the PointSet containing the output coordinates. */
         pset_out = astAnnul( pset_out );
      }
   }

/* Resample the input grid. */
/* ------------------------ */
/* If the input coordinates have been produced successfully, identify
   the input grid resampling method to be used. */
   if ( astOK ) {

/* Nearest pixel. */
/* -------------- */
      switch ( interp ) {
         case AST__NEAREST:

/* Define a macro to use a "case" statement to invoke the
   nearest-pixel interpolation function appropriate to a given data
   type. */
#define CASE_NEAREST(X,Xtype) \
               case ( TYPE_##X ): \
                  result = \
                  InterpolateNearest##X( ndim_in, lbnd_in, ubnd_in, \
                                         (Xtype *) in, (Xtype *) in_var, \
                                         npoint, offset, \
                                         (const double *const *) ptr_in, \
                                         flags, *( (Xtype *) badval_ptr ), \
                                         (Xtype *) out, (Xtype *) out_var ); \
                  break;
       
/* Use the above macro to invoke the appropriate function. */
            switch ( type ) {
#if defined(AST_LONG_DOUBLE)     /* Not normally implemented */
               CASE_NEAREST(LD,long double)
#endif
               CASE_NEAREST(D,double)
               CASE_NEAREST(F,float)
               CASE_NEAREST(L,long int)
               CASE_NEAREST(UL,unsigned long int)
               CASE_NEAREST(I,int)
               CASE_NEAREST(UI,unsigned int)
               CASE_NEAREST(S,short int)
               CASE_NEAREST(US,unsigned short int)
               CASE_NEAREST(B,signed char)
               CASE_NEAREST(UB,unsigned char)
            }
            break;

/* Undefine the macro. */
#undef CASE_NEAREST
               
/* Linear interpolation. */
/* --------------------- */
/* Note this is also the default if zero is given. */
         case AST__LINEAR:
         case 0:

/* Define a macro to use a "case" statement to invoke the linear
   interpolation function appropriate to a given data type. */
#define CASE_LINEAR(X,Xtype) \
               case ( TYPE_##X ): \
                  result = \
                  InterpolateLinear##X( ndim_in, lbnd_in, ubnd_in,\
                                        (Xtype *) in, (Xtype *) in_var, \
                                        npoint, offset, \
                                        (const double *const *) ptr_in, \
                                        flags, *( (Xtype *) badval_ptr ), \
                                        (Xtype *) out, (Xtype *) out_var ); \
                  break;

/* Use the above macro to invoke the appropriate function. */
            switch ( type ) {
#if defined(AST_LONG_DOUBLE)     /* Not normally implemented */
               CASE_LINEAR(LD,long double)
#endif
               CASE_LINEAR(D,double)
               CASE_LINEAR(F,float)
               CASE_LINEAR(L,long int)
               CASE_LINEAR(UL,unsigned long int)
               CASE_LINEAR(I,int)
               CASE_LINEAR(UI,unsigned int)
               CASE_LINEAR(S,short int)
               CASE_LINEAR(US,unsigned short int)
               CASE_LINEAR(B,signed char)
               CASE_LINEAR(UB,unsigned char)
            }
            break;

/* Undefine the macro. */
#undef CASE_LINEAR

/* Interpolation using a 1-d kernel. */
/* --------------------------------- */
         case AST__SINC:
         case AST__SINCSINC:
         case AST__UKERN1:       /* User-supplied 1-d kernel function */

/* Obtain a pointer to the appropriate 1-d kernel function (either
   internal or user-defined) and set up any parameters it may
   require. */
            switch ( interp ) {
               case AST__SINC:
                  kernel = Sinc;
                  neighb = MaxI( 1, (int) floor( params[ 0 ] + 0.5 ) );
                  break;
               case AST__SINCSINC:
                  kernel = SincSinc;
                  neighb = 2;
                  break;
               case AST__UKERN1: /* User-supplied function */
                  kernel = (void (*)( double, const double [],
                                      int, double * )) finterp;
                  neighb = MaxI( 1, (int) floor( params[ 0 ] + 0.5 ) );
                  break;
            }

/* Define a macro to use a "case" statement to invoke the 1-d kernel
   interpolation function appropriate to a given data type, passing it
   the pointer to the kernel function obtained above. */
#define CASE_KERNEL1(X,Xtype) \
               case ( TYPE_##X ): \
                  result = \
                  InterpolateKernel1##X( this, ndim_in, lbnd_in, ubnd_in, \
                                         (Xtype *) in, (Xtype *) in_var, \
                                         npoint, offset, \
                                         (const double *const *) ptr_in, \
                                         kernel, neighb, params, flags, \
                                         *( (Xtype *) badval_ptr ), \
                                         (Xtype *) out, (Xtype *) out_var ); \
                  break;

/* Use the above macro to invoke the appropriate function. */
            switch ( type ) {
#if defined(AST_LONG_DOUBLE)     /* Not normally implemented */
               CASE_KERNEL1(LD,long double)
#endif
               CASE_KERNEL1(D,double)
               CASE_KERNEL1(F,float)
               CASE_KERNEL1(L,long int)
               CASE_KERNEL1(UL,unsigned long int)
               CASE_KERNEL1(I,int)
               CASE_KERNEL1(UI,unsigned int)
               CASE_KERNEL1(S,short int)
               CASE_KERNEL1(US,unsigned short int)
               CASE_KERNEL1(B,signed char)
               CASE_KERNEL1(UB,unsigned char)
            }
            break;

/* Undefine the macro. */
#undef CASE_KERNEL1

/* User-supplied interpolation function. */
/* ------------------------------------- */
         case AST__UINTERP:

/* Determine if a variance array is to be processed. */
            usevar = ( in_var && out_var );

/* Define a macro to use a "case" statement to invoke the
   user-supplied interpolation function appropriate to a given data
   type. Ensure that either both "in_var and "out_var" are passed, or
   neither is. If an error occurs within the user-supplied function,
   then report a contextual error message. */
#define CASE_USER(X,Xtype) \
               case ( TYPE_##X ): \
                  ( *finterp )( ndim_in, lbnd_in, ubnd_in, (Xtype *) in, \
                                (Xtype *) ( usevar ? in_var : NULL ), \
                                npoint, offset, \
                                (const double *const *) ptr_in, \
                                params, flags, *( (Xtype *) badval_ptr ), \
                                (Xtype *) out, \
                                (Xtype *) ( usevar ? out_var : NULL ), \
                                &nbad ); \
                  if ( astOK ) { \
                     result += nbad; \
                  } else { \
                     astError( astStatus, "astResample"#X"(%s): Error " \
                               "signalled by user-supplied sub-pixel " \
                               "interpolation function.", \
                               astGetClass( this ) ); \
                  } \
                  break;

/* Use the above macro to invoke the function. */
            switch ( type ) {
#if defined(AST_LONG_DOUBLE)     /* Not normally implemented */
               CASE_USER(LD,long double)
#endif
               CASE_USER(D,double)
               CASE_USER(F,float)
               CASE_USER(L,long int)
               CASE_USER(UL,unsigned long int)
               CASE_USER(I,int)
               CASE_USER(UI,unsigned int)
               CASE_USER(S,short int)
               CASE_USER(US,unsigned short int)
               CASE_USER(B,signed char)
               CASE_USER(UB,unsigned char)
            }
            break;

/* Undefine the macro. */
#undef CASE_USER

/* Error: invalid interpolation scheme specified. */
/* ---------------------------------------------- */
         default:

/* Define a macro to report an error message appropriate to a given
   data type. */
#define CASE_ERROR(X) \
            case TYPE_##X: \
               astError( AST__SISIN, "astResample"#X"(%s): Invalid " \
                         "sub-pixel interpolation scheme (%d) specified.", \
                         astGetClass( this ), interp ); \
               break;
                                 
/* Use the above macro to report an appropriate error message. */
            switch ( type ) {
#if defined(AST_LONG_DOUBLE)     /* Not normally implemented */
               CASE_ERROR(LD)
#endif
               CASE_ERROR(D)
               CASE_ERROR(F)
               CASE_ERROR(L)
               CASE_ERROR(UL)
               CASE_ERROR(I)
               CASE_ERROR(UI)
               CASE_ERROR(S)
               CASE_ERROR(US)
               CASE_ERROR(B)
               CASE_ERROR(UB)
            }
            break;

/* Undefine the macro. */
#undef CASE_ERROR
      }
   }

/* Annul the PointSet used to hold input coordinates. */
   pset_in = astAnnul( pset_in );

/* Free the workspace. */
   offset = astFree( offset );
   stride = astFree( stride );

/* If an error occurred, clear the returned value. */
   if ( !astOK ) result = 0;

/* Return the result. */
   return result;
}

static int ResampleWithBlocking( AstMapping *this, const double *linear_fit,
                                 int ndim_in,
                                 const int *lbnd_in, const int *ubnd_in,
                                 const void *in, const void *in_var,
                                 DataType type, int interp, int (* finterp)(),
                                 const double *params, int flags,
                                 const void *badval_ptr, int ndim_out,
                                 const int *lbnd_out, const int *ubnd_out,
                                 const int *lbnd, const int *ubnd,
                                 void *out, void *out_var ) {
/*
*  Name:
*     ResampleWithBlocking

*  Purpose:
*     Resample a section of a data grid in a memory-efficient way.

*  Type:
*     Private function.

*  Synopsis:
*     #include "mapping.h"
*     int ResampleWithBlocking( AstMapping *this, const double *linear_fit,
*                               int ndim_in,
*                               const int *lbnd_in, const int *ubnd_in,
*                               const void *in, const void *in_var,
*                               DataType type, int interp, int (* finterp)(),
*                               const double *params, int flags,
*                               const void *badval_ptr, int ndim_out,
*                               const int *lbnd_out, const int *ubnd_out,
*                               const int *lbnd, const int *ubnd,
*                               void *out, void *out_var )

*  Class Membership:
*     Mapping member function.

*  Description:
*     This function resamples a rectangular grid of data (with any
*     number of dimensions) into a specified section of another
*     rectangular grid (with a possibly different number of
*     dimensions). The coordinate transformation used is given by the
*     inverse transformation of the Mapping which is supplied or,
*     alternatively, by a linear approximation fitted to a Mapping's
*     inverse transformation. Any pixel interpolation scheme may be
*     specified for interpolating between the pixels of the input
*     grid.
*
*     This function is very similar to ResampleSection, except that in
*     order to limit memory usage and to ensure locality of reference,
*     it divides the output grid up into "blocks" which have a limited
*     extent along each output dimension. Each block, which will not
*     contain more than a pre-determined maximum number of pixels, is
*     then passed to ResampleSection for resampling.

*  Parameters:
*     this
*        Pointer to a Mapping, whose inverse transformation may be
*        used to transform the coordinates of pixels in the output
*        grid into associated positions in the input grid, from which
*        the output pixel values should be derived (by interpolation
*        if necessary).
*
*        The number of input coordintes for the Mapping (Nin
*        attribute) should match the value of "ndim_in" (below), and
*        the number of output coordinates (Nout attribute) should
*        match the value of "ndim_out".
*     linear_fit
*        Pointer to an optional array of double which contains the
*        coefficients of a linear fit which approximates the above
*        Mapping's inverse coordinate transformation. If this is
*        supplied, it will be used in preference to the above Mapping
*        when transforming coordinates. This may be used to enhance
*        performance in cases where evaluation of the Mapping's
*        inverse transformation is expensive. If no linear fit is
*        available, a NULL pointer should be supplied.
*
*        The way in which the fit coefficients are stored in this
*        array and the number of array elements are as defined by the
*        LinearApprox function.
*     ndim_in
*        The number of dimensions in the input grid. This should be at
*        least one.
*     lbnd_in
*        Pointer to an array of integers, with "ndim_in" elements.
*        This should give the coordinates of the centre of the first
*        pixel in the input data grid along each dimension.
*     ubnd_in
*        Pointer to an array of integers, with "ndim_in" elements.
*        This should give the coordinates of the centre of the last
*        pixel in the input data grid along each dimension.
*
*        Note that "lbnd_in" and "ubnd_in" together define the shape
*        and size of the input data grid, its extent along a
*        particular (i'th) dimension being (ubnd_in[i] - lbnd_in[i] +
*        1). They also define the input grid's coordinate system, with
*        each pixel being of unit extent along each dimension with
*        integral coordinate values at its centre.
*     in
*        Pointer to the input array of data to be resampled (with one
*        element for each pixel in the input grid). The numerical type
*        of these data should match the "type" value (below). The
*        storage order should be such that the coordinate of the first
*        dimension varies most rapidly and that of the final dimension
*        least rapidly (i.e. Fortran array storage order is used).
*     in_var
*        An optional pointer to a second array of positive numerical
*        values (with the same size and data type as the "in" array),
*        which represent estimates of the statistical variance
*        associated with each element of the "in" array. If this
*        second array is given (along with the corresponding "out_var"
*        array), then estimates of the variance of the resampled data
*        will also be returned.
*
*        If no variance estimates are required, a NULL pointer should
*        be given.
*     type
*        A value taken from the "DataType" enum, which specifies the
*        data type of the input and output arrays containing the
*        gridded data (and variance) values.
*     interp
*        A value selected from a set of pre-defined macros to identify
*        which sub-pixel interpolation algorithm should be used.
*     finterp
*        If "interp" is set to a value which requires a user-supplied
*        function, then a pointer to that function shoild be given
*        here. Otherwise, this value is not used and may be a NULL
*        pointer.
*     params
*        Pointer to an optional array of parameters that may be passed
*        to the interpolation algorithm, if required. If no parameters
*        are required, a NULL pointer should be supplied.
*     flags
*        The bitwise OR of a set of flag values which provide
*        additional control over the resampling operation.
*     badval_ptr
*        If the AST__USEBAD flag is set (above), this parameter is a
*        pointer to a value which is used to identify bad data and/or
*        variance values in the input array(s). The referenced value's
*        data type must match that of the "in" (and "in_var")
*        arrays. The same value will also be used to flag any output
*        array elements for which resampled values could not be
*        obtained.  The output arrays(s) may be flagged with this
*        value whether or not the AST__USEBAD flag is set (the
*        function return value indicates whether any such values have
*        been produced).
*     ndim_out
*        The number of dimensions in the output grid. This should be
*        at least one.
*     lbnd_out
*        Pointer to an array of integers, with "ndim_out" elements.
*        This should give the coordinates of the centre of the first
*        pixel in the output data grid along each dimension.
*     ubnd_out
*        Pointer to an array of integers, with "ndim_out" elements.
*        This should give the coordinates of the centre of the last
*        pixel in the output data grid along each dimension.
*
*        Note that "lbnd_out" and "ubnd_out" together define the shape
*        and size of the output data grid in the same way as "lbnd_in"
*        and "ubnd_in" define the shape and size of the input grid
*        (see above).
*     lbnd
*        Pointer to an array of integers, with "ndim_out" elements.
*        This should give the coordinates of the first pixel in the
*        section of the output data grid for which a value is
*        required.
*     ubnd
*        Pointer to an array of integers, with "ndim_out" elements.
*        This should give the coordinates of the last pixel in the
*        section of the output data grid for which a value is
*        required.
*
*        Note that "lbnd" and "ubnd" define the shape and position of
*        the section of the output grid for which resampled values are
*        required. This section should lie wholly within the extent of
*        the output grid (as defined by the "lbnd_out" and "ubnd_out"
*        arrays). Regions of the output grid lying outside this section
*        will not be modified.
*     out
*        Pointer to an array with the same data type as the "in"
*        array, into which the resampled data will be returned.  The
*        storage order should be such that the coordinate of the first
*        dimension varies most rapidly and that of the final dimension
*        least rapidly (i.e. Fortran array storage order is used).
*     out_var
*        An optional pointer to an array with the same data type and
*        size as the "out" array, into which variance estimates for
*        the resampled values may be returned. This array will only be
*        used if the "in_var" array has been given.
*
*        If no output variance estimates are required, a NULL pointer
*        should be given.

*  Returned Value:
*     The number of output grid points to which a data value (or a
*     variance value if relevant) equal to "badval" has been assigned
*     because no valid output value could be obtained.

*  Notes:
*     - A value of zero will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*/

/* Local Constants: */
   const int mxpix = 65536;      /* Maximum number of pixels in a block */

/* Local Variables: */
   int *dim_block;               /* Pointer to array of block dimensions */
   int *lbnd_block;              /* Pointer to block lower bound array */
   int *ubnd_block;              /* Pointer to block upper bound array */
   int dim;                      /* Dimension size */
   int done;                     /* All blocks resampled? */
   int hilim;                    /* Upper limit on maximum block dimension */
   int idim;                     /* Loop counter for dimensions */
   int lolim;                    /* Lower limit on maximum block dimension */
   int mxdim_block;              /* Maximum block dimension */
   int npix;                     /* Number of pixels in block */
   int result;                   /* Result value to return */

/* Initialise. */
   result = 0;

/* Check the global error status. */
   if ( !astOK ) return result;

/* Allocate workspace. */
   lbnd_block = astMalloc( sizeof( int ) * (size_t) ndim_out );
   ubnd_block = astMalloc( sizeof( int ) * (size_t) ndim_out );
   dim_block = astMalloc( sizeof( int ) * (size_t) ndim_out );
   if ( astOK ) {

/* Find the optimum block size. */
/* ---------------------------- */
/* We first need to find the maximum extent which a block of output
   pixels may have in each dimension. We determine this by taking the
   output grid extent in each dimension and then limiting the maximum
   dimension size until the resulting number of pixels is sufficiently
   small. This approach allows the block shape to approximate (or
   match) the output grid shape when appropriate. */

/* First loop to calculate the total number of output pixels and the
   maximum output dimension size. */
      npix = 1;
      mxdim_block = 0;
      for ( idim = 0; idim < ndim_out; idim++ ) {
         dim = ubnd[ idim ] - lbnd[ idim ] + 1;
         npix *= dim;
         if ( mxdim_block < dim ) mxdim_block = dim;
      }

/* If the number of output pixels is too large for a single block, we
   perform iterations to determine the optimum upper limit on a
   block's dimension size. Initialise the limits on this result. */
      if ( npix > mxpix ) {
         lolim = 1;
         hilim = mxdim_block;

/* Loop to perform a binary chop, searching for the best result until
   the lower and upper limits on the result converge to adjacent
   values. */
         while ( ( hilim - lolim ) > 1 ) {

/* Form a new estimate from the mid-point of the previous limits. */
            mxdim_block = ( hilim + lolim ) / 2;

/* See how many pixels a block contains if its maximum dimension is
   limited to this new value. */
            for ( npix = 1, idim = 0; idim < ndim_out ; idim++ ) {
               dim = ubnd[ idim ] - lbnd[ idim ] + 1;
               npix *= ( dim < mxdim_block ) ? dim : mxdim_block;
            }

/* Update the appropriate limit, according to whether the number of
   pixels is too large or too small. */
            *( ( npix <= mxpix ) ? &lolim : &hilim ) = mxdim_block;
         }

/* When iterations have converged, obtain the maximum limit on the
   dimension size of a block which results in no more than the maximum
   allowed number of pixels per block. However, ensure that all block
   dimensions are at least 2. */
            mxdim_block = lolim;
      }
      if ( mxdim_block < 2 ) mxdim_block = 2;

/* Calculate the block dimensions by applying this limit to the output
   grid dimensions. */
      for ( idim = 0; idim < ndim_out ; idim++ ) {
         dim = ubnd[ idim ] - lbnd[ idim ] + 1;
         dim_block[ idim ] = ( dim < mxdim_block ) ? dim : mxdim_block;

/* Also initialise the lower and upper bounds of the first block of
   output grid pixels to be resampled, ensuring that this does not
   extend outside the grid itself. */
         lbnd_block[ idim ] = lbnd[ idim ];
         ubnd_block[ idim ] = MinI( lbnd[ idim ] + dim_block[ idim ] - 1,
                                    ubnd[ idim ] );
      }

/* Resample each block of output pixels. */
/* ------------------------------------- */
/* Loop to generate the extent of each block of output pixels and to
   resample them. */
      done = 0;
      while ( !done && astOK ) {

/* Resample the current block, accumulating the sum of bad pixels
   produced. */
         result += ResampleSection( this, linear_fit,
                                    ndim_in, lbnd_in, ubnd_in,
                                    in, in_var, type, interp, finterp, params,
                                    flags, badval_ptr,
                                    ndim_out, lbnd_out, ubnd_out,
                                    lbnd_block, ubnd_block, out, out_var );

/* Update the block extent to identify the next block of output
   pixels. */
         idim = 0;
         do {

/* We find the least significant dimension where the upper bound of
   the block has not yet reached the upper bound of the region of the
   output grid which we are resampling. The block's position is then
   incremented by one block extent along this dimension, checking that
   the resulting extent does not go outside the region being
   resampled. */
            if ( ubnd_block[ idim ] < ubnd[ idim ] ) {
               lbnd_block[ idim ] = MinI( lbnd_block[ idim ] +
                                          dim_block[ idim ], ubnd[ idim ] );
               ubnd_block[ idim ] = MinI( lbnd_block[ idim ] +
                                          dim_block[ idim ] - 1,
                                          ubnd[ idim ] );
               break;

/* If any less significant dimensions are found where the upper bound
   of the block has reached its maximum value, we reset the block to
   its lowest position. */
            } else {
               lbnd_block[ idim ] = lbnd[ idim ];
               ubnd_block[ idim ] = MinI( lbnd[ idim ] + dim_block[ idim ] - 1,
                                          ubnd[ idim ] );

/* All the blocks have been processed once the position along the most
   significant dimension has been reset. */
               done = ( ++idim == ndim_out );
            }
         } while ( !done );
      }
   }

/* Free the workspace. */
   lbnd_block = astFree( lbnd_block );
   ubnd_block = astFree( ubnd_block );
   dim_block = astFree( dim_block );

/* If an error occurred, clear the returned value. */
   if ( !astOK ) result = 0;

/* Return the result. */
   return result;
}

static void SetAttrib( AstObject *this_object, const char *setting ) {
/*
*  Name:
*     astSetAttrib

*  Purpose:
*     Set an attribute value for a Mapping.

*  Type:
*     Private function.

*  Synopsis:
*     #include "mapping.h"
*     void SetAttrib( AstObject *this, const char *setting )

*  Class Membership:
*     Mapping member function (over-rides the astSetAttrib protected
*     method inherited from the Object class).

*  Description:
*     This function assigns an attribute value for a Mapping, the
*     attribute and its value being specified by means of a string of
*     the form:
*
*        "attribute= value "
*
*     Here, "attribute" specifies the attribute name and should be in
*     lower case with no white space present. The value to the right
*     of the "=" should be a suitable textual representation of the
*     value to be assigned and this will be interpreted according to
*     the attribute's data type.  White space surrounding the value is
*     only significant for string attributes.

*  Parameters:
*     this
*        Pointer to the Mapping.
*     setting
*        Pointer to a null terminated string specifying the new attribute
*        value.
*/

/* Local Variables: */
   AstMapping *this;             /* Pointer to the Mapping structure */
   int invert;                   /* Invert attribute value */
   int len;                      /* Length of setting string */
   int nc;                       /* Number of characters read by sscanf */
   int report;                   /* Report attribute value */

/* Check the global error status. */
   if ( !astOK ) return;

/* Obtain a pointer to the Mapping structure. */
   this = (AstMapping *) this_object;

/* Obtain the length of the setting string. */
   len = (int) strlen( setting );

/* Test for each recognised attribute in turn, using "sscanf" to parse
   the setting string and extract the attribute value (or an offset to
   it in the case of string values). In each case, use the value set
   in "nc" to check that the entire string was matched. Once a value
   has been obtained, use the appropriate method to set it. */

/* Invert. */
/* ------- */
   if ( nc = 0,
        ( 1 == sscanf( setting, "invert= %d %n", &invert, &nc ) )
        && ( nc >= len ) ) {
      astSetInvert( this, invert );

/* Report. */
/* ------- */
   } else if ( nc = 0,
        ( 1 == sscanf( setting, "report= %d %n", &report, &nc ) )
        && ( nc >= len ) ) {
      astSetReport( this, report );

/* Define a macro to see if the setting string matches any of the
   read-only attributes of this class. */
#define MATCH(attrib) \
        ( nc = 0, ( 0 == sscanf( setting, attrib "=%*[^\n]%n", &nc ) ) && \
                  ( nc >= len ) )

/* If the attribute was not recognised, use this macro to report an error
   if a read-only attribute has been specified. */
   } else if ( MATCH( "nin" ) ||
        MATCH( "nout" ) ||
        MATCH( "tranforward" ) ||
        MATCH( "traninverse" ) ) {
      astError( AST__NOWRT, "astSet: The setting \"%s\" is invalid for a %s.",
                setting, astGetClass( this ) );
      astError( AST__NOWRT, "This is a read-only attribute." );

/* If the attribute is still not recognised, pass it on to the parent
   method for further interpretation. */
   } else {
      (*parent_setattrib)( this_object, setting );
   }

/* Undefine macros local to this function. */
#undef MATCH
}

static void Sinc( double offset, const double params[], int flags,
                  double *value ) {
/*
*  Name:
*     Sinc

*  Purpose:
*     1-dimensional sinc(x) interpolation kernel.

*  Type:
*     Private function.

*  Synopsis:
*     #include "mapping.h"
*     void Sinc( double offset, const double params[], int flags,
*                double *value )

*  Class Membership:
*     Mapping member function.

*  Description:
*     This function calculates the value of a 1-dimensional sub-pixel
*     interpolation kernel. The function used is sinc(x).

*  Parameters:
*     offset
*        The offset of a pixel from the interpolation point, measured
*        in pixels.
*     params
*        Not used.
*     flags
*        Not used.
*     value
*        Pointer to a double to receive the calculated kernel value.

*  Notes:
*     - This function does not perform error checking and does not
*     generate errors.
*/

/* Local Variables: */
   static double pi;             /* Value of pi */
   static int init = 0;          /* Initialisation flag */

/* On the first invocation, initialise a local value for pi. Do this
   only once. */
   if ( !init ) {
      pi = acos( -1.0 );
      init = 1;
   }

/* Scale the offset, so that sinc( 1.0 ) == 0.0. */
   offset *= pi;

/* Evaluate the function. */
   *value = ( offset != 0.0 ) ? ( sin( offset ) / offset ) : 1.0;
}

static void SincSinc( double offset, const double params[], int flags,
                      double *value ) {
/*
*  Name:
*     SincSinc

*  Purpose:
*     1-dimensional sinc(x) * sinc(x/2) interpolation kernel.

*  Type:
*     Private function.

*  Synopsis:
*     #include "mapping.h"
*     void SincSinc( double offset, const double params[], int flags,
*                    double *value )

*  Class Membership:
*     Mapping member function.

*  Description:
*     This function calculates the value of a 1-dimensional sub-pixel
*     interpolation kernel. The function used is sinc(x) * sinc(x/2).

*  Parameters:
*     offset
*        The offset of a pixel from the interpolation point, measured
*        in pixels.
*     params
*        Not used.
*     flags
*        Not used.
*     value
*        Pointer to a double to receive the calculated kernel value.

*  Notes:
*     - This function does not perform error checking and does not
*     generate errors.
*/

/* Local Variables: */
   double offset_sq;             /* Square of offset value */
   static double pi;             /* Value of pi */
   static int init = 0;          /* Initialisation flag */

/* On the first invocation, initialise a local value for pi. Do this
   only once. */
   if ( !init ) {
      pi = acos( -1.0 );
      init = 1;
   }

/* Scale the offset, so that sinc( 1.0 ) == 0.0. */
   offset *= pi;

/* Evaluate the function. */
   offset_sq = offset * offset;
   *value = ( offset_sq != 0.0 ) ?
            ( 2.0 * sin( offset ) * sin( 0.5 * offset ) / offset_sq ) : 1.0;
}

static AstMapping *Simplify( AstMapping *this ) {
/*
*++
*  Name:
c     astSimplify
f     AST_SIMPLIFY

*  Purpose:
*     Simplify a Mapping.

*  Type:
*     Public function.

*  Synopsis:
c     #include "mapping.h"
c     AstMapping *astSimplify( AstMapping *this )
f     RESULT = AST_SIMPLIFY( THIS, STATUS )

*  Class Membership:
*     Mapping method.

*  Description:
*     This function simplifies a Mapping (which may be a compound
*     Mapping such as a CmpMap) to eliminate redundant computational
*     steps, or to merge separate steps which can be performed more
*     efficiently in a single operation.
*
*     As a simple example, a Mapping which multiplied coordinates by
*     5, and then multiplied the result by 10, could be simplified to
*     a single step which multiplied by 50. Similarly, a Mapping which
*     multiplied by 5, and then divided by 5, could be reduced to a
*     simple copying operation.
*
*     This function should typically be applied to Mappings which have
*     undergone substantial processing or have been formed by merging
*     other Mappings. It is of potential benefit, for example, in
*     reducing execution time if applied before using a Mapping to
*     transform a large number of coordinates.

*  Parameters:
c     this
f     THIS = INTEGER (Given)
*        Pointer to the original Mapping.
f     STATUS = INTEGER (Given and Returned)
f        The global status.

*  Returned Value:
c     astSimplify()
f     AST_SIMPLIFY = INTEGER
*        A new pointer to the (possibly simplified) Mapping.

*  Notes:
*     - This function can safely be applied even to Mappings which
*     cannot be simplified. If no simplification is possible, it
c     behaves exactly like astClone and returns a pointer to the
f     behaves exactly like AST_CLONE and returns a pointer to the
*     original Mapping.
*     - The Mapping returned by this function may not be independent
*     of the original (even if simplification was possible), and
*     modifying it may therefore result in indirect modification of
*     the original. If a completely independent result is required, a
c     copy should be made using astCopy.
f     copy should be made using AST_COPY.
*     - A null Object pointer (AST__NULL) will be returned if this
c     function is invoked with the AST error status set, or if it
f     function is invoked with STATUS set to an error value, or if it
*     should fail for any reason.
*--
*/

/* Local Variables: */
   AstMapping **map_list;        /* Pointer to array of Mapping pointers */
   AstMapping *map;              /* Cloned pointer to nominated Mapping */
   AstMapping *result;           /* Pointer to result Mapping */
   int *invert_list;             /* Pointer to array of invert flags */
   int imap;                     /* Loop counter for Mappings */
   int modified;                 /* Index of first modified element */
   int nmap;                     /* Number of Mappings */
   int simpler;                  /* Simplification achieved? */

/* Initialise. */
   result = NULL;

/* Check the inherited status. */
   if ( !astOK ) return result;

/* Initialise dynamic arrays of Mapping pointers and associated invert
   flags. */
   nmap = 0;
   map_list = NULL;
   invert_list = NULL;

/* Build a Mapping list to contain this Mapping (the list should only
   have 1 element). */
   astMapList( this, 1, astGetInvert( this ), &nmap, &map_list, &invert_list );

/* Pass the list repeatedly to the "astMapMerge" method for
   simplification. */
   simpler = 0;
   while ( astOK ) {
      map = astClone( map_list[ 0 ] );
      modified = astMapMerge( map, 0, 1, &nmap, &map_list, &invert_list );
      map = astAnnul( map );

/* Quit looping if the number of Mappings increases above 1, or if no
   further change occurs. Note if any simplification was achieved. */
      if ( ( nmap > 1 ) || ( modified < 0 ) ) break;
      simpler = 1;
   }

/* Check whether simplification has occurred. If not, simply clone the
   original Mapping pointer. This is what will normally happen for
   Mapping classes which inherit the default (null) "astMapMerge"
   method from this class and do not define one of their own. */
   if ( astOK ) {
      if ( !simpler || ( nmap > 1 ) ) {
         result = astClone( this );

/* If simplification occurred, test if the resulting Mapping has the
   Invert attribute value we want. If so, we can simply clone a
   pointer to it. */
      } else {
         if ( invert_list[ 0 ] == astGetInvert( map_list[ 0 ] ) ) {
            result = astClone( map_list[ 0 ] );

/* If not, we must make a copy. */
         } else {
            result = astCopy( map_list[ 0 ] );

/* Either clear the copy's Invert attribute, or set it to 1, as
   required. */
            if ( invert_list[ 0 ] ) {
               astSetInvert( result, 1 );
            } else {
               astClearInvert( result );
            }
         }
      }
   }

/* Loop to annul all the pointers in the Mapping list. */
   for ( imap = 0; imap < nmap; imap++ ) {
      map_list[ imap ] = astAnnul( map_list[ imap ] );
   }

/* Free the dynamic arrays. */
   map_list = astFree( map_list );
   invert_list = astFree( invert_list );

/* If an error occurred, annul the returned Mapping. */
   if ( !astOK ) result = astAnnul( result );

/* Return the result. */
   return result;   
}

static void SpecialBounds( const MapData *mapdata, double *lbnd, double *ubnd,
                           double xl[], double xu[] ) {
/*
*  Name:
*     SpecialBounds

*  Purpose:
*     Estimate coordinate bounds using special points.

*  Type:
*     Private function.

*  Synopsis:
*     #include "mapping.h"
*     void SpecialBounds( const MapData *mapdata, double *lbnd, double *ubnd,
*                         double xl[], double xu[] );

*  Class Membership:
*     Mapping member function.

*  Description:
*     This function makes a rough estimate of the lower and upper
*     bounds of a Mapping function over a constrained region of its
*     input coordinate space by transforming a set of special test
*     points. The points used lie at the corners of the constrained
*     region, at the centre of each of its faces, at its centroid, and
*     (if within the coordinate constraints) the origin.
*
*     In many practical cases, the true extrema may actually lie at
*     one or other of these points, in which case the true bounds will
*     be found. In other cases, this function only provides an
*     approximate limit on each bound (there is no way of telling if
*     this is the case, however). In either case, having these initial
*     estimates can speed subsequent searches to find the global
*     extrema as well as making that search more secure

*  Parameters:
*     mapdata
*        Pointer to a MapData structure describing the Mapping
*        function, its coordinate constraints, etc.
*     lbnd
*        Pointer to a double.  On entry, this should contain a
*        previously-obtained upper limit on the lower bound, or
*        AST__BAD if no such limit is available. On exit, it will be
*        updated with a new estimate of the lower bound, if a better
*        one has been found.
*     ubnd
*        Pointer to a double.  On entry, this should contain a
*        previously-obtained lower limit on the upper bound, or
*        AST__BAD if no such limit is available. On exit, it will be
*        updated with a new estimate of the upper bound, if a better
*        one has been found.
*     xl
*        Pointer to an array of double, with one element for each
*        input coordinate, in which to return the position of a (not
*        necessarily unique) input point at which the lower output
*        bound is reached. This array is not altered if an improved
*        estimate of the lower bound cannot be found.
*     xu
*        Pointer to an array of double, with one element for each
*        input coordinate, in which to return the position of a (not
*        necessarily unique) input point at which the upper output
*        bound is reached. This array is not altered if an improved
*        estimate of the upper bound cannot be found.
*/

/* Local Variables: */
   AstPointSet *pset_in;         /* PointSet for input coordinates */
   AstPointSet *pset_out;        /* PointSet for output coordinates */
   double **ptr_in;              /* Pointer to input coordinates */
   double **ptr_out;             /* Pointer to output coordinates */
   double f;                     /* Output coordinate value */
   int *limit;                   /* Workspace for lower/upper limit flags */
   int bad;                      /* Output coordinate bad? */
   int coord;                    /* Loop counter for coordinates */
   int done;                     /* All corners done? */
   int face;                     /* Loop counter for faces */
   int ncoord;                   /* Number of input coordinates */
   int npoint;                   /* Number of points */
   int origin;                   /* Origin lies within bounds? */
   int point;                    /* Loop counter for points */
   
/* Obtain the number of coordinate axes and calculate the number of
   points required in order to place one at every corner of the
   constrained region of the coordinate space. */
   ncoord = mapdata->nin;
   for ( npoint = 1, coord = 0; coord < ncoord; coord++ ) npoint *= 2;

/* Also include placing one at the centre of every face and one at the
   centroid of the constrained coordinate space. */
   npoint += 2 * ncoord + 1;
   
/* Determine if the origin lies within the bounds. If so, include it
   as a further point. */
   origin = 1;
   for ( coord = 0; coord < ncoord; coord++ ) {
      if ( ( mapdata->lbnd[ coord ] > 0.0 ) ||
           ( mapdata->ubnd[ coord ] < 0.0 ) ) {
         origin = 0;
         break;
      }
   }
   if ( origin ) npoint++;

/* Create a PointSet to hold the coordinates and obtain a pointer to
   its coordinate values. Also allocate workspace for calculating the
   corner coordinates. */
   pset_in = astPointSet( npoint, ncoord, "" );
   ptr_in = astGetPoints( pset_in );
   limit = astMalloc( sizeof( int ) * (size_t) ncoord );
   if ( astOK ) {
   
/* Initialise the workspace. */
      for ( coord = 0; coord < ncoord; coord++ ) limit[ coord ] = 0;

/* Loop to visit every corner. */
      done = 0;
      point = 0;
      while ( !done ) {

/* At each corner, translate the contents of the "limit" array
   (containing zeros and ones) into the lower or upper bound on the
   corresponding axis. This gives the coordinates of the corner, which
   we store in the input PointSet. */
         for ( coord = 0; coord < ncoord; coord++ ) {
            ptr_in[ coord ][ point ] = limit[ coord ] ?
                                       mapdata->ubnd[ coord ] :
                                       mapdata->lbnd[ coord ];
         }

/* Increment the count of points (i.e. corners). */
         point++;
      
/* Now update the limit array to identify the next corner. */
         coord = 0;
         while ( !done ) {

/* Flip the first zero found to become a one. This gives a new
   corner. */
            if ( !limit[ coord ] ) {
               limit[ coord ] = 1;
               break;

/* However, first flip any previous ones to become zeros and then
   examine the next element. We have processed all corners once the
   array is entirely filled with ones. */
            } else {
               limit[ coord ] = 0;
               done = ( ++coord == ncoord );
            }
         }
      }

/* Once the corners have been processed, loop to consider the centre
   of each face. */
      for ( face = 0; face < ( 2 * ncoord ); face++ ) {

/* First calculate the centroid value for each coordinate.  Then set
   one of these coordinates to the bound where the face lies. */
         for ( coord = 0; coord < ncoord; coord++ ) {
            ptr_in[ coord ][ point ] = 0.5 * ( mapdata->lbnd[ coord ] +
                                               mapdata->ubnd[ coord ] );
         }
         ptr_in[ face / 2 ][ point ] = ( face % 2 ) ?
                                       mapdata->lbnd[ face / 2 ] :
                                       mapdata->ubnd[ face / 2 ];

/* Increment the count of points. */
         point++;
      }

/* Place a point at the centroid of the constrained coordinate
   space. */
      for ( coord = 0; coord < ncoord; coord++ ) {
         ptr_in[ coord ][ point ] = 0.5 * ( mapdata->lbnd[ coord ] +
                                            mapdata->ubnd[ coord ] );
      }
      point++;

/* Finally, add the origin, if it lies within the constraints. */
      if ( origin ) {
         for ( coord = 0; coord < ncoord; coord++ ) {
            ptr_in[ coord ][ point ] = 0.0;
         }
      }

/* Once all the input coordinates have been calculated, transform them
   and obtain a pointer to the resulting coordinate values. */
      pset_out = astTransform( mapdata->mapping, pset_in, mapdata->forward,
                               NULL );
      ptr_out = astGetPoints( pset_out );
      if ( astOK ) {

/* Loop through each point and test if any of its transformed
   coordinates is bad. */
         for ( point = 0; point < npoint; point++ ) {
            bad = 0;
            for ( coord = 0; coord < ncoord; coord++ ) {
               if ( ptr_out[ coord ][ point ] == AST__BAD ) {
                  bad = 1;
                  break;
               }
            }

/* If so, we ignore the point. Otherwise, extract the required
   coordinate. */
            if ( !bad ) {
               f = ptr_out[ mapdata->coord ][ point ];

/* Use this to update the lower and upper bounds we are seeking. If
   either bound is updated, also store the coordinates of the
   corresponding input point. */
               if ( ( *lbnd == AST__BAD ) || ( f < *lbnd ) ) {
                  *lbnd = f;
                  for ( coord = 0; coord < ncoord; coord++ ) {
                     xl[ coord ] = ptr_in[ coord ][ point ];
                  }
               }
               if ( ( *ubnd == AST__BAD ) || ( f > *ubnd ) ) {
                  *ubnd = f;
                  for ( coord = 0; coord < ncoord; coord++ ) {
                     xu[ coord ] = ptr_in[ coord ][ point ];
                  }
               }
            }
         }
      }
   }

/* Annul the temporary PointSets and free the workspace. */
   pset_in = astAnnul( pset_in );
   pset_out = astAnnul( pset_out );
   limit = astFree( limit );
}

static int TestAttrib( AstObject *this_object, const char *attrib ) {
/*
*  Name:
*     TestAttrib

*  Purpose:
*     Test if a specified attribute value is set for a Mapping.

*  Type:
*     Private function.

*  Synopsis:
*     #include "mapping.h"
*     int TestAttrib( AstObject *this, const char *attrib )

*  Class Membership:
*     Mapping member function (over-rides the astTestAttrib protected
*     method inherited from the Object class).

*  Description:
*     This function returns a boolean result (0 or 1) to indicate whether
*     a value has been set for one of a Mapping's attributes.

*  Parameters:
*     this
*        Pointer to the Mapping.
*     attrib
*        Pointer to a null terminated string specifying the attribute
*        name.  This should be in lower case with no surrounding white
*        space.

*  Returned Value:
*     One if a value has been set, otherwise zero.

*  Notes:
*     - A value of zero will be returned if this function is invoked
*     with the global status set, or if it should fail for any reason.
*/

/* Local Variables: */
   AstMapping *this;             /* Pointer to the Mapping structure */
   int result;                   /* Result value to return */

/* Initialise. */
   result = 0;

/* Check the global error status. */
   if ( !astOK ) return result;

/* Obtain a pointer to the Mapping structure. */
   this = (AstMapping *) this_object;

/* Check the attribute name and test the appropriate attribute. */

/* Invert. */
/* ------- */
   if ( !strcmp( attrib, "invert" ) ) {
      result = astTestInvert( this );

/* Report. */
/* ------- */
   } else if ( !strcmp( attrib, "report" ) ) {
      result = astTestReport( this );

/* If the name is not recognised, test if it matches any of the
   read-only attributes of this class. If it does, then return
   zero. */
   } else if ( !strcmp( attrib, "nin" ) ||
        !strcmp( attrib, "nout" ) ||
        !strcmp( attrib, "tranforward" ) ||
        !strcmp( attrib, "traninverse" ) ) {
      result = 0;

/* If the attribute is still not recognised, pass it on to the parent
   method for further interpretation. */
   } else {
      result = (*parent_testattrib)( this_object, attrib );
   }

/* Return the result, */
   return result;
}

static void Tran1( AstMapping *this, int npoint, const double xin[],
                   int forward, double xout[] ) {
/*
*++
*  Name:
c     astTran1
f     AST_TRAN1

*  Purpose:
*     Transform 1-dimensional coordinates.

*  Type:
*     Public virtual function.

*  Synopsis:
c     #include "mapping.h"
c     void astTran1( AstMapping *this, int npoint, const double xin[],
c                    int forward, double xout[] )
f     CALL AST_TRAN1( THIS, NPOINT, XIN, FORWARD, XOUT, STATUS )

*  Class Membership:
*     Mapping method.

*  Description:
c     This function applies a Mapping to transform the coordinates of
f     This routine applies a Mapping to transform the coordinates of
*     a set of points in one dimension.

*  Parameters:
c     this
f     THIS = INTEGER (Given)
*        Pointer to the Mapping to be applied.
c     npoint
f     NPOINT = INTEGER (Given)
*        The number of points to be transformed.
c     xin
f     XIN( NPOINT ) = DOUBLE PRECISION (Given)
c        An array of "npoint" coordinate values for the input
f        An array of coordinate values for the input
*        (untransformed) points.
c     forward
f     FORWARD = LOGICAL (Given)
c        A non-zero value indicates that the Mapping's forward
c        coordinate transformation is to be applied, while a zero
c        value indicates that the inverse transformation should be
c        used.
f        A .TRUE. value indicates that the Mapping's forward
f        coordinate transformation is to be applied, while a .FALSE.
f        value indicates that the inverse transformation should be
f        used.
c     xout
f     XOUT( NPOINT ) = DOUBLE PRECISION (Returned)
c        An array (with "npoint" elements) into which the
f        An array into which the
*        coordinates of the output (transformed) points will be written.
f     STATUS = INTEGER (Given and Returned)
f        The global status.

*  Notes:
*     - The Mapping supplied must have the value 1 for both its Nin
*     and Nout attributes.
*--
*/

/* Local Variables: */
   AstPointSet *in_points;       /* Pointer to input PointSet */
   AstPointSet *out_points;      /* Pointer to output PointSet */
   const double *in_ptr[ 1 ];    /* Array of input data pointers */
   double *out_ptr[ 1 ];         /* Array of output data pointers */

/* Check the global error status. */
   if ( !astOK ) return;

/* Validate the Mapping and numbers of points/coordinates. */
   ValidateMapping( this, forward, npoint, 1, 1, "astTran1" );

/* Set up pointers to the input and output coordinate arrays. */
   if ( astOK ) {
      in_ptr[ 0 ] = xin;
      out_ptr[ 0 ] = xout;

/* Create PointSets to describe the input and output points. */
      in_points = astPointSet( npoint, 1, "" );
      out_points = astPointSet( npoint, 1, "" );

/* Associate the data pointers with the PointSets (note we must
   explicitly remove the "const" qualifier from the input data here,
   although they will not be modified).  */
      astSetPoints( in_points, (double **) in_ptr );
      astSetPoints( out_points, out_ptr );

/* Apply the required transformation to the coordinates. */
      (void) astTransform( this, in_points, forward, out_points );

/* If the Mapping's Report attribute is set, report the effect the
   Mapping has had on the coordinates. */
      if ( astGetReport( this ) ) astReportPoints( this, forward,
                                                   in_points, out_points );

/* Delete the two PointSets. */
      astDelete( in_points );
      astDelete( out_points );
   }
}

static void Tran2( AstMapping *this,
                   int npoint, const double xin[], const double yin[],
                   int forward, double xout[], double yout[] ) {
/*
*++
*  Name:
c     astTran2
f     AST_TRAN2

*  Purpose:
*     Transform 2-dimensional coordinates.

*  Type:
*     Public virtual function.

*  Synopsis:
c     #include "mapping.h"
c     void astTran2( AstMapping *this,
c                    int npoint, const double xin[], const double yin[],
c                    int forward, double xout[], double yout[] )
f     CALL AST_TRAN2( THIS, NPOINT, XIN, YIN, FORWARD, XOUT, YOUT, STATUS )

*  Class Membership:
*     Mapping method.

*  Description:
c     This function applies a Mapping to transform the coordinates of
f     This routine applies a Mapping to transform the coordinates of
*     a set of points in two dimensions.

*  Parameters:
c     this
f     THIS = INTEGER (Given)
*        Pointer to the Mapping to be applied.
c     npoint
f     NPOINT = INTEGER (Given)
*        The number of points to be transformed.
c     xin
f     XIN( NPOINT ) = DOUBLE PRECISION (Given)
c        An array of "npoint" X-coordinate values for the input
f        An array of X-coordinate values for the input
*        (untransformed) points.
c     yin
f     YIN( NPOINT ) = DOUBLE PRECISION (Given)
c        An array of "npoint" Y-coordinate values for the input
f        An array of Y-coordinate values for the input
*        (untransformed) points.
c     forward
f     FORWARD = LOGICAL (Given)
c        A non-zero value indicates that the Mapping's forward
c        coordinate transformation is to be applied, while a zero
c        value indicates that the inverse transformation should be
c        used.
f        A .TRUE. value indicates that the Mapping's forward
f        coordinate transformation is to be applied, while a .FALSE.
f        value indicates that the inverse transformation should be
f        used.
c     xout
f     XOUT( NPOINT ) = DOUBLE PRECISION (Returned)
c        An array (with "npoint" elements) into which the
f        An array into which the
*        X-coordinates of the output (transformed) points will be written.
c     yout
f     YOUT( NPOINT ) = DOUBLE PRECISION (Returned)
c        An array (with "npoint" elements) into which the
f        An array into which the
*        Y-coordinates of the output (transformed) points will be written.
f     STATUS = INTEGER (Given and Returned)
f        The global status.

*  Notes:
*     - The Mapping supplied must have the value 2 for both its Nin
*     and Nout attributes.
*--
*/

/* Local Variables: */
   AstPointSet *in_points;       /* Pointer to input PointSet */
   AstPointSet *out_points;      /* Pointer to output PointSet */
   const double *in_ptr[ 2 ];    /* Array of input data pointers */
   double *out_ptr[ 2 ];         /* Array of output data pointers */

/* Check the global error status. */
   if ( !astOK ) return;

/* Validate the Mapping and the numbers of points/coordinates. */
   ValidateMapping( this, forward, npoint, 2, 2, "astTran2" );

/* Set up pointers to the input and output coordinate arrays. */
   if ( astOK ) {
      in_ptr[ 0 ] = xin;
      in_ptr[ 1 ] = yin;
      out_ptr[ 0 ] = xout;
      out_ptr[ 1 ] = yout;

/* Create PointSets to describe the input and output points. */
      in_points = astPointSet( npoint, 2, "" );
      out_points = astPointSet( npoint, 2, "" );

/* Associate the data pointers with the PointSets (note we must
   explicitly remove the "const" qualifier from the input data here,
   although they will not be modified).  */
      astSetPoints( in_points, (double **) in_ptr );
      astSetPoints( out_points, out_ptr );

/* Apply the required transformation to the coordinates. */
      (void) astTransform( this, in_points, forward, out_points );

/* If the Mapping's Report attribute is set, report the effect the
   Mapping has had on the coordinates. */
      if ( astGetReport( this ) ) astReportPoints( this, forward,
                                                   in_points, out_points );

/* Delete the two PointSets. */
      astDelete( in_points );
      astDelete( out_points );
   }
}

static void TranN( AstMapping *this, int npoint,
                   int ncoord_in, int indim, const double (*in)[],
                   int forward,
                   int ncoord_out, int outdim, double (*out)[] ) {
/*
*++
*  Name:
c     astTranN
f     AST_TRANN

*  Purpose:
*     Transform N-dimensional coordinates.

*  Type:
*     Public virtual function.

*  Synopsis:
c     #include "mapping.h"
c     void astTranN( AstMapping *this, int npoint,
c                    int ncoord_in, int indim, const double (*in)[],
c                    int forward,
c                    int ncoord_out, int outdim, double (*out)[] )
f     CALL AST_TRANN( THIS, NPOINT,
f                     NCOORD_IN, INDIM, IN,
f                     FORWARD, NCOORD_OUT, OUTDIM, OUT, STATUS )

*  Class Membership:
*     Mapping method.

*  Description:
c     This function applies a Mapping to transform the coordinates of
f     This routine applies a Mapping to transform the coordinates of
*     a set of points in an arbitrary number of dimensions. It is the
*     appropriate routine to use if the coordinates are not purely 1-
*     or 2-dimensional and are stored in a single array (which they
*     need not fill completely).
c
c     If the coordinates are not stored in a single array, then the
c     astTranP function might be more suitable.

*  Parameters:
c     this
f     THIS = INTEGER (Given)
*        Pointer to the Mapping to be applied.
c     npoint
f     NPOINT = INTEGER (Given)
*        The number of points to be transformed.
c     ncoord_in
f     NCOORD_IN = INTEGER (Given)
*        The number of coordinates being supplied for each input point
*        (i.e. the number of dimensions of the space in which the
*        input points reside).
c     indim
f     INDIM = INTEGER (Given)
c        The number of elements along the second dimension of the "in"
f        The number of elements along the first dimension of the IN
*        array (which contains the input coordinates). This value is
*        required so that the coordinate values can be correctly
*        located if they do not entirely fill this array. The value
c        given should not be less than "npoint".
f        given should not be less than NPOINT.
c     in
f     IN( INDIM, NCOORD_IN ) = DOUBLE PRECISION (Given)
c        A 2-dimensional array, of shape "[ncoord_in][indim]",
c        containing the coordinates of the input (untransformed)
c        points. These should be stored such that the value of
c        coordinate number "coord" for input point number "point" is
c        found in element "in[coord][point]".
f        An array containing the coordinates of the input
f        (untransformed) points. These should be stored such that the
f        value of coordinate number COORD for input point number POINT
f        is found in element IN(POINT,COORD).
c     forward
f     FORWARD = LOGICAL (Given)
c        A non-zero value indicates that the Mapping's forward
c        coordinate transformation is to be applied, while a zero
c        value indicates that the inverse transformation should be
c        used.
f        A .TRUE. value indicates that the Mapping's forward
f        coordinate transformation is to be applied, while a .FALSE.
f        value indicates that the inverse transformation should be
f        used.
c     ncoord_out
f     NCOORD_OUT = INTEGER (Given)
*        The number of coordinates being generated by the Mapping for
*        each output point (i.e. the number of dimensions of the
*        space in which the output points reside). This need not be
c        the same as "ncoord_in".
f        the same as NCOORD_IN.
c     outdim
f     OUTDIM = INTEGER (Given)
c        The number of elements along the second dimension of the "out"
f        The number of elements along the first dimension of the OUT
*        array (which will contain the output coordinates). This value
*        is required so that the coordinate values can be correctly
*        located if they will not entirely fill this array. The value
c        given should not be less than "npoint".
f        given should not be less than NPOINT.
c     out
f     OUT( OUTDIM, NCOORD_OUT ) = DOUBLE PRECISION (Returned)
c        A 2-dimensional array, of shape "[ncoord_out][outdim]", into
c        which the coordinates of the output (transformed) points will
c        be written. These will be stored such that the value of
c        coordinate number "coord" for output point number "point"
c        will be found in element "out[coord][point]".
f        An array into which the coordinates of the output
f        (transformed) points will be written. These will be stored
f        such that the value of coordinate number COORD for output
f        point number POINT will be found in element OUT(POINT,COORD).
f     STATUS = INTEGER (Given and Returned)
f        The global status.

*  Notes:
c     - If the forward coordinate transformation is being applied, the
c     Mapping supplied must have the value of "ncoord_in" for its Nin
c     attribute and the value of "ncoord_out" for its Nout attribute. If
c     the inverse transformation is being applied, these values should
c     be reversed.
f     - If the forward coordinate transformation is being applied, the
f     Mapping supplied must have the value of NCOORD_IN for its Nin
f     attribute and the value of NCOORD_OUT for its Nout attribute. If
f     the inverse transformation is being applied, these values should
f     be reversed.
*--
*/

/* Local Variables: */
   AstPointSet *in_points;       /* Pointer to input PointSet */
   AstPointSet *out_points;      /* Pointer to output PointSet */
   const double **in_ptr;        /* Pointer to array of input data pointers */
   double **out_ptr;             /* Pointer to array of output data pointers */
   int coord;                    /* Loop counter for coordinates */

/* Check the global error status. */
   if ( !astOK ) return;

/* Validate the mapping and numbers of points/coordinates. */
   ValidateMapping( this, forward, npoint, ncoord_in, ncoord_out, "astTranN" );

/* Also validate the input array dimension argument. */
   if ( astOK && ( indim < npoint ) ) {
      astError( AST__DIMIN, "astTranN(%s): The input array dimension value "
                "(%d) is invalid.", astGetClass( this ), indim );
      astError( AST__DIMIN, "This should not be less than the number of "
                "points being transformed (%d).", npoint );
   }

/* Similarly, validate the output array dimension argument. */
   if ( astOK && ( outdim < npoint ) ) {
      astError( AST__DIMIN, "astTranN(%s): The output array dimension value "
                "(%d) is invalid.", astGetClass( this ), outdim );
      astError( AST__DIMIN, "This should not be less than the number of "
                "points being transformed (%d).", npoint );
   }

/* Allocate memory to hold the arrays of input and output data
   pointers. */
   if ( astOK ) {
      in_ptr = (const double **) astMalloc( sizeof( const double * ) *
                                            (size_t) ncoord_in );
      out_ptr = astMalloc( sizeof( double * ) * (size_t) ncoord_out );

/* Initialise the input data pointers to locate the coordinate data in
   the "in" array. */
      if ( astOK ) {
         for ( coord = 0; coord < ncoord_in; coord++ ) {
            in_ptr[ coord ] = *in + coord * indim;
         }

/* Similarly initialise the output data pointers to point into the
   "out" array. */
         for ( coord = 0; coord < ncoord_out; coord++ ) {
            out_ptr[ coord ] = *out + coord * outdim;
         }

/* Create PointSets to describe the input and output points. */
         in_points = astPointSet( npoint, ncoord_in, "" );
         out_points = astPointSet( npoint, ncoord_out, "" );

/* Associate the data pointers with the PointSets (note we must
   explicitly remove the "const" qualifier from the input data here,
   although they will not be modified).  */
         astSetPoints( in_points, (double **) in_ptr );
         astSetPoints( out_points, out_ptr );

/* Apply the required transformation to the coordinates. */
         (void) astTransform( this, in_points, forward, out_points );

/* If the Mapping's Report attribute is set, report the effect the
   Mapping has had on the coordinates. */
         if ( astGetReport( this ) ) astReportPoints( this, forward,
                                                      in_points, out_points );

/* Delete the two PointSets. */
         astDelete( in_points );
         astDelete( out_points );
      }

/* Free the memory used for the data pointers. */
      in_ptr = (const double **) astFree( (void *) in_ptr );
      out_ptr = astFree( out_ptr );
   }
}

static void TranP( AstMapping *this, int npoint,
                   int ncoord_in, const double *ptr_in[],
                   int forward, int ncoord_out, double *ptr_out[] ) {
/*
c++
*  Name:
*     astTranP

*  Purpose:
*     Transform N-dimensional coordinates held in separate arrays.

*  Type:
*     Public virtual function.

*  Synopsis:
*     #include "mapping.h"
*     void astTranP( AstMapping *this, int npoint,
*                    int ncoord_in, const double *ptr_in[],
*                    int forward, int ncoord_out, double *ptr_out[] )

*  Class Membership:
*     Mapping method.

*  Description:
*     This function applies a Mapping to transform the coordinates of
*     a set of points in an arbitrary number of dimensions. It is the
*     appropriate routine to use if the coordinates are not purely 1-
*     or 2-dimensional and are stored in separate arrays, since each
*     coordinate array is located by supplying a separate pointer to
*     it.
*
*     If the coordinates are stored in a single (2-dimensional) array,
*     then the astTranN function might be more suitable.

*  Parameters:
*     this
*        Pointer to the Mapping to be applied.
*     npoint
*        The number of points to be transformed.
*     ncoord_in
*        The number of coordinates being supplied for each input point
*        (i.e. the number of dimensions of the space in which the
*        input points reside).
*     ptr_in
*        An array of pointers to double, with "ncoord_in"
*        elements. Element "ptr_in[coord]" should point at the first
*        element of an array of double (with "npoint" elements) which
*        contain the values of coordinate number "coord" for each
*        input (untransformed) point. The value of coordinate number
*        "coord" for input point number "point" is therefore given by
*        "ptr_in[coord][point]" (assuming both indices are
*        zero-based).
*     forward
*        A non-zero value indicates that the Mapping's forward
*        coordinate transformation is to be applied, while a zero
*        value indicates that the inverse transformation should be
*        used.
*     ncoord_out
*        The number of coordinates being generated by the Mapping for
*        each output point (i.e. the number of dimensions of the space
*        in which the output points reside). This need not be the same
*        as "ncoord_in".
*     ptr_out
*        An array of pointers to double, with "ncoord_out"
*        elements. Element "ptr_out[coord]" should point at the first
*        element of an array of double (with "npoint" elements) into
*        which the values of coordinate number "coord" for each output
*        (transformed) point will be written.  The value of coordinate
*        number "coord" for output point number "point" will therefore
*        be found in "ptr_out[coord][point]".

*  Notes:
*     - If the forward coordinate transformation is being applied, the
*     Mapping supplied must have the value of "ncoord_in" for its Nin
*     attribute and the value of "ncoord_out" for its Nout
*     attribute. If the inverse transformation is being applied, these
*     values should be reversed.
*     - This routine is not available in the Fortran 77 interface to
*     the AST library.
c--
*/

/* Local Variables: */
   AstPointSet *in_points;       /* Pointer to input PointSet */
   AstPointSet *out_points;      /* Pointer to output PointSet */

/* Check the global error status. */
   if ( !astOK ) return;

/* Validate the Mapping and number of points/coordinates. */
   ValidateMapping( this, forward, npoint, ncoord_in, ncoord_out, "astTranP" );

/* Create PointSets to describe the input and output points. */
   if ( astOK ) {
      in_points = astPointSet( npoint, ncoord_in, "" );
      out_points = astPointSet( npoint, ncoord_out, "" );

/* Associate the data pointers with the PointSets (note we must
   explicitly remove the "const" qualifier from the input data here,
   although they will not be modified).  */
      astSetPoints( in_points, (double **) ptr_in );
      astSetPoints( out_points, ptr_out );

/* Apply the required transformation to the coordinates. */
      (void) astTransform( this, in_points, forward, out_points );

/* If the Mapping's Report attribute is set, report the effect the
   Mapping has had on the coordinates. */
      if ( astGetReport( this ) ) astReportPoints( this, forward,
                                                   in_points, out_points );

/* Delete the two PointSets. */
      astDelete( in_points );
      astDelete( out_points );
   }
}

static AstPointSet *Transform( AstMapping *this, AstPointSet *in,
                               int forward, AstPointSet *out ) {
/*
*+
*  Name:
*     astTransform

*  Purpose:
*     Transform a set of points.

*  Type:
*     Protected virtual function.

*  Synopsis:
*     #include "mapping.h"
*     AstPointSet *astTransform( AstMapping *this, AstPointSet *in,
*                                int forward, AstPointSet *out )

*  Class Membership:
*     Mapping method.

*  Description:
*     This function takes a Mapping and a set of points encapsulated
*     in a PointSet, and applies either the forward or inverse
*     coordinate transformation (if defined by the Mapping) to the
*     points.

*  Parameters:
*     this
*        Pointer to the Mapping. The nature of the coordinate
*        transformation will depend on the class of Mapping
*        supplied. Note that there is no constructor for the Mapping
*        class itself, so this object should be from a derived class.
*     in
*        Pointer to the PointSet holding the input coordinate data.
*     forward
*        A non-zero value indicates that the forward coordinate
*        transformation should be applied, while a zero value requests
*        the inverse transformation.
*     out
*        Pointer to a PointSet which will hold the transformed
*        (output) coordinate values. A NULL value may also be given,
*        in which case a new PointSet will be created by this
*        function.

*  Returned Value:
*     Pointer to the output (possibly new) PointSet.

*  Notes:
*     - An error will result if the Mapping supplied does not define
*     the requested coordinate transformation (either forward or
*     inverse).
*     - The number of coordinate values per point in the input
*     PointSet must match the number of input coordinates for the
*     Mapping being applied (or number of output coordinates if the
*     inverse transformation is requested).
*     - If an output PointSet is supplied, it must have space for
*     sufficient number of points and coordinate values per point to
*     accommodate the result (e.g. the number of Mapping output
*     coordinates, or number of input coordinates if the inverse
*     transformation is requested). Any excess space will be ignored.
*     - A null pointer will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*-
*/

/* Local Variables: */
   AstPointSet *result;          /* Pointer to output PointSet */
   int def;                      /* Coordinate transformation defined? */
   int ncoord_in;                /* Number of input PointSet coordinates */
   int ncoord_out;               /* Number of coordinates in output PointSet */
   int nin;                      /* Number of input Mapping coordinates */
   int nout;                     /* Number of output Mapping coordinates */
   int npoint;                   /* Number of points to transform */
   int npoint_out;               /* Number of points in output PointSet */

/* Check the global error status. */
   if ( !astOK ) return NULL;

/* Initialise. */
   result = NULL;

/* Determine if a coordinate transformation is defined for the requested
   direction. */
   def = forward ? astGetTranForward( this ) : astGetTranInverse( this );

/* Report an error if the transformation is not defined. */
   if ( astOK && !def ) {
      astError( AST__TRNND, "astTransform(%s): %s coordinate transformation "
                "is not defined by the %s supplied.", astGetClass( this ),
                forward ? "A forward" : "An inverse", astGetClass( this ) );
   }

/* Obtain the effective number of input and output coordinate values for the
   transformation to be performed, taking account of the transformation
   direction required. Note we use Mapping methods to obtain these values, as
   this will take account of whether the Mapping has been inverted. */
   nin = forward ? astGetNin( this ) : astGetNout( this );
   nout = forward ? astGetNout( this ) : astGetNin( this );

/* Obtain the number of input points to transform and the number of coordinate
   values per input point. */
   npoint = astGetNpoint( in );
   ncoord_in = astGetNcoord( in );

/* If OK, check that the number of input coordinates matches the number
   required by the mapping. Report an error if these numbers do not match. */
   if ( astOK && ( ncoord_in != nin ) ) {
      astError( AST__NCPIN, "astTransform(%s): Bad number of coordinate "
                "values (%d) in input %s.", astGetClass( this ), ncoord_in,
                astGetClass( in ) );
      astError( AST__NCPIN, "The %s given requires %d coordinate value(s) for "
                "each input point.", astGetClass( this ), nin );
   }

/* If still OK, and a non-NULL pointer has been given for the output PointSet,
   then obtain the number of points and number of coordinates per point for
   this PointSet. */
   if ( astOK && out ) {
      npoint_out = astGetNpoint( out );
      ncoord_out = astGetNcoord( out );

/* Check that the dimensions of this PointSet are adequate to accommodate the
   output coordinate values and report an error if they are not. */
      if ( astOK ) {
         if ( npoint_out < npoint ) {
            astError( AST__NOPTS, "astTransform(%s): Too few points (%d) in "
                      "output %s.", astGetClass( this ), npoint_out,
                      astGetClass( out ) );
            astError( AST__NOPTS, "The %s needs space to hold %d transformed "
                      "point(s).", astGetClass( this ), npoint );
         } else if ( ncoord_out < nout ) {
            astError( AST__NOCTS, "astTransform(%s): Too few coordinate "
                      "values per point (%d) in output %s.",
                      astGetClass( this ), ncoord_out, astGetClass( out ) );
            astError( AST__NOCTS, "The %s supplied needs space to store %d "
                      "coordinate value(s) per transformed point.",
                      astGetClass( this ), nout );
         }
      }
   }

/* If all the validation stages are passed successfully, and a NULL output
   pointer was given, then create a new PointSet to encapsulate the output
   coordinate data. */
   if ( astOK ) {
      if ( !out ) {
         result = astPointSet( npoint, nout, "" );

/* Otherwise, use the PointSet supplied. */
      } else {
         result = out;
      }
   }

/* Return a pointer to the output PointSet. Note that we do not actually
   transform (or even copy) the coordinates. This is left for derived classes
   to implement. */
   return result;
}

/*
*++
*  Name:
c     astUinterp
f     AST_UINTERP

*  Purpose:
*     Perform sub-pixel interpolation on a grid of data.

*  Type:
*     Fictitious function.

*  Synopsis:
c     #include "mapping.h"
c     void astUinterp( int ndim_in, const int lbnd_in[], const int ubnd_in[],
c                      const <Xtype> in[], const <Xtype> in_var[],
c                      int npoint, const int offset[],
c                      const double *const coords[], const double params[],
c                      int flags, <Xtype> badval,
c                      <Xtype> out[], <Xtype> out_var[], int *nbad )
f     CALL AST_UINTERP( NDIM_IN, LBND_IN, UBND_IN, IN, IN_VAR,
f                       NPOINT, OFFSET, COORDS, PARAMS, FLAGS, BADVAL,
f                       OUT, OUT_VAR, NBAD, STATUS )

*  Class Membership:
*     Mapping member function.

*  Description:
c     This is a fictitious function which does not actually
c     exist. Instead, this description constitutes a template so that
c     you may implement a function with this interface for yourself
c     (and give it any name you wish). A pointer to such a function
c     may be passed via the "finterp" parameter of the astResample<X>
c     functions (q.v.) in order to perform sub-pixel interpolation
c     during resampling of gridded data (you must also set the
c     "interp" parameter of astResample<X> to the value
c     AST__UINTERP). This allows you to use your own interpolation
c     algorithm in addition to those which are pre-defined.
f     This is a fictitious routine which does not actually
f     exist. Instead, this description constitutes a template so that
f     you may implement a routine with this interface for yourself
f     (and give it any name you wish). Such a routine
f     may be passed via the FINTERP argument of the AST_RESAMPLE<X>
f     functions (q.v.) in order to perform sub-pixel interpolation
f     during resampling of gridded data (you must also set the
f     INTERP argument of AST_RESAMPLE<X> to the value
f     AST__UINTERP). This allows you to use your own interpolation
f     algorithm in addition to those which are pre-defined.
*
c     The function interpolates an input grid of data (and,
f     The routine interpolates an input grid of data (and,
*     optionally, processes associated statistical variance estimates)
*     at a specified set of points.

*  Parameters:
c     ndim_in
f     NDIM_IN = INTEGER (Given)
*        The number of dimensions in the input grid. This will be at
*        least one.
c     lbnd_in
f     LBND_IN( NDIM_IN ) = INTEGER (Given)
c        Pointer to an array of integers, with "ndim_in" elements,
f        An array
*        containing the coordinates of the centre of the first pixel
*        in the input grid along each dimension.
c     ubnd_in
f     UBND_IN( NDIM_IN ) = INTEGER (Given)
c        Pointer to an array of integers, with "ndim_in" elements,
f        An array
*        containing the coordinates of the centre of the last pixel in
*        the input grid along each dimension.
*
c        Note that "lbnd_in" and "ubnd_in" together define the shape,
f        Note that LBND_IN and UBND_IN together define the shape,
*        size and coordinate system of the input grid in the same
c        way as they do in astResample<X>.
f        way as they do in AST_RESAMPLE<X>.
c     in
f     IN( * ) = <Xtype> (Given)
c        Pointer to an array, with one element for each pixel in the
f        An array, with one element for each pixel in the
*        input grid, containing the input data. This will be the same
c        array as was passed to astResample<X> via the "in" parameter.
f        array as was passed to AST_RESAMPLE<X> via the IN argument.
*        The numerical type of this array should match that of the
*        data being processed.
c     in_var
f     IN_VAR( * ) = <Xtype> (Given)
c        Pointer to an optional second array with the same size and
c        type as the "in" array. If given, this will contain the set
c        of variance values associated with the input data and will be
c        the same array as was passed to astResample<X> via the
c        "in_var" parameter.
f        An optional second array with the same size and type as the
f        IN array. This will only be given if the AST__USEVAR flag is
f        set via the FLAGS argument (below). If given, it will contain
f        the set of variance values associated with the input data and
f        will be the same array as was passed to AST_RESAMPLE<X> via
f        the IN_VAR argument.
*
c        If no variance values are being processed, this will be a
c        NULL pointer.
f        If the AST__USEVAR flag is not set, then no variance values
f        are being processed. In this case, this array of variance
f        values may be a dummy (e.g. one-element) array and should not
f        be used.
c     npoint
f     NPOINT = INTEGER (Given)
*        The number of points at which the input grid is to be
*        interpolated. This will be at least one.
c     offset
f     OFFSET( NPOINT ) = INTEGER (Given)
c        Pointer to an array of integers with "npoint" elements. For
c        each interpolation point, this will contain the zero-based
c        index in the "out" (and "out_var") array(s) at which the
c        interpolated value (and its variance, if required) should be
c        stored. For example, the interpolated value for point number
c        "point" should be stored in "out[offset[point]]" (assuming
c        the index "point" is zero-based).
f        For each interpolation point, this array will contain the
f        offset from the start of the OUT (and OUT_VAR) array(s) at
f        which the interpolated value (and its variance, if required)
f        should be stored. For example, the interpolated value for
f        point number POINT should be stored in OUT(1+OFFSET(POINT)).
c     coords
f     COORDS( NPOINT, NDIM_IN ) = DOUBLE PRECISION (Given)
c        An array of pointers to double, with "ndim_in"
c        elements. Element "coords[coord]" will point at the first
c        element of an array of double (with "npoint" elements) which
c        contains the values of coordinate number "coord" for each
c        interpolation point. The value of coordinate number "coord"
c        for interpolation point number "point" is therefore given by
c        "coords[coord][point]" (assuming both indices are
c        zero-based).
f        A 2-dimensional array containing the coordinates of the
f        points at which interpolation should be performed. These will
f        be stored so that coordinate number COORD for interpolation
f        point number POINT is found in element COORDS(POINT,COORD).
*
*        If any interpolation point has any of its coordinates equal
c        to the value AST__BAD (as defined in the "ast.h" header
f        to the value AST__BAD (as defined in the AST_PAR include
*        file), then the corresponding output data (and variance)
c        should be set to the value given by "badval" (see below).
f        should be set to the value given by BADVAL (see below).
c     params
f     PARAMS( * ) = DOUBLE PRECISION (Given)
c        This will be a pointer to the same array as was given via the
c        "params" parameter of astResample<X>. You may use this to
f        This will be the same array as was given via the
f        PARAMS argument of AST_RESAMPLE<X>. You may use this to
*        pass any additional parameter values required by your
*        interpolation algorithm.
c     flags
f     FLAGS = INTEGER (Given)
c        This will be the same value as was given via the "flags"
c        parameter of astResample<X>. You may test this value to
f        This will be the same value as was given via the FLAGS
f        argument of AST_RESAMPLE<X>. You may test this value to
*        provide additional control over the operation of your
*        resampling algorithm. Note that the special flag values
*        AST__URESAMP1, 2, 3 & 4 are reserved for you to use for your
*        own purposes and will not clash with other pre-defined flag
c        values (see astResample<X>).
f        values (see AST_RESAMPLE<X>).
c     badval
f     BADVAL = <Xtype> (Given)
c        This will be the same value as was given via the "badval"
c        parameter of astResample<X>, and will have the same numerical
c        type as the data being processed (i.e. as elements of the
c        "in" array).  It should be used to test for bad pixels in the
c        input grid (but only if the AST__USEBAD flag is set via the
c        "flags" parameter) and for identifying bad output values in
c        the "out" (and "out_var") array(s).
f        This will be the same value as was given for the BADVAL
f        argument of AST_RESAMPLE<X>, and will have the same numerical
f        type as the data being processed (i.e. as elements of the IN
f        array).  It should be used to test for bad pixels in the
f        input grid (but only if the AST__USEBAD flag is set via the
f        FLAGS argument) and for identifying bad output values in the
f        OUT (and OUT_VAR) array(s).
c     out
f     OUT( * ) = <Xtype> (Returned)
c        Pointer to an array with the same numerical type as the "in"
f        An array with the same numerical type as the IN
*        array, into which the interpolated data values should be
*        returned.  Note that details of the storage order and number
*        of dimensions of this array are not required, since the
c        "offset" array contains all necessary information about where
f        OFFSET array contains all necessary information about where
*        each returned value should be stored.
*
c        In general, not all elements of this array (or the "out_var"
f        In general, not all elements of this array (or the OUT_VAR
*        array below) may be used in any particular invocation of the
c        function. Those which are not used should be returned
f        routine. Those which are not used should be returned
*        unchanged.
c     out_var
f     OUT_VAR( * ) = <Xtype> (Returned)
c        Pointer to an optional array with the same type and size as
c        the "out" array, into which variance estimates for the
c        resampled values should be returned.  This array will only be
c        given if the "in_var" array has also been given.
f        An optional array with the same type and size as the OUT
f        array, into which variance estimates for the resampled values
f        should be returned. This array will only be given if the
f        AST__USEVAR flag is set via the FLAGS argument.
*
c        If given, it is addressed in exactly the same way (via the
c        "offset" array) as the "out" array. The values returned
c        should be estimates of the statistical variance of the
c        corresponding values in the "out" array, on the assumption
c        that all errors in input data values are statistically
c        independent and that their variance estimates may simply be
c        summed (with appropriate weighting factors).
f        If given, it is addressed in exactly the same way (via the
f        OFFSET array) as the OUT array. The values returned should be
f        estimates of the statistical variance of the corresponding
f        values in the OUT array, on the assumption that all errors in
f        input data values are statistically independent and that
f        their variance estimates may simply be summed (with
f        appropriate weighting factors).
*
c        If no output variance estimates are required, a NULL pointer
c        will be given.
f        If the AST__USEVAR flag is not set, then variance values are
f        not being processed.  In this case, this array may be a dummy
f        (e.g. one-element) array and should not be used.
c     nbad
f     NBAD = INTEGER (Returned)
*        This should return the number of interpolation points at
*        which an output data value (and/or a variance value if
c        relevant) equal to "badval" has been assigned because no
f        relevant) equal to BADVAL has been assigned because no
*        valid interpolated value could be obtained.  The maximum
c        value that should be returned is "npoint", and the minimum is
f        value that should be returned is NPOINT, and the minimum is
*        zero (indicating that all output values were successfully
*        obtained).
f     STATUS = INTEGER (Given and Returned)
f        The global status.

*  Notes:
*     - The data type <Xtype> indicates the numerical type of the data
c     being processed, as for astResample<X>.
f     being processed, as for AST_RESAMPLE<X>.
c     - This function will typically be invoked more than once for each
c     invocation of astResample<X>.
f     - This routine will typically be invoked more than once for each
f     invocation of AST_RESAMPLE<X>.
c     - If an error occurs within this function, it should use
c     astSetStatus to set the AST error status to an error value.
c     This will cause an immediate return from astResample<X>.
f     - If an error occurs within this routine, it should set the
f     STATUS argument to an error value before returning. This will
f     cause an immediate return from AST_RESAMPLE<X>.
*--
*/
/* Note the above is just a description to act as a template. The
   function does not actually exist. */

/*
*++
*  Name:
c     astUkern1
f     AST_UKERN1

*  Purpose:
*     1-dimensional sub-pixel interpolation kernel.

*  Type:
*     Fictitious function.

*  Synopsis:
c     #include "mapping.h"
c     void astUkern1( double offset, const double params[], int flags,
c                     double *value )
f     CALL AST_UKERN1( OFFSET, PARAMS, FLAGS, VALUE, STATUS )

*  Class Membership:
*     Mapping member function.

*  Description:
c     This is a fictitious function which does not actually
c     exist. Instead, this description constitutes a template so that
c     you may implement a function with this interface for yourself
c     (and give it any name you wish). A pointer to such a function
c     may be passed via the "finterp" parameter of the astResample<X>
c     functions (q.v.) in order to supply a 1-dimensional
c     interpolation kernel to the algorithm which performs sub-pixel
c     interpolation during resampling of gridded data (you must also
c     set the "interp" parameter of astResample<X> to the value
c     AST__UKERN1). This allows you to use your own interpolation
c     kernel in addition to those which are pre-defined.
f     This is a fictitious routine which does not actually
f     exist. Instead, this description constitutes a template so that
f     you may implement a routine with this interface for yourself
f     (and give it any name you wish). Such a routine
f     may be passed via the FINTERP argument of the AST_RESAMPLE<X>
f     functions (q.v.) in order to supply a 1-dimensional
f     interpolation kernel to the algorithm which performs sub-pixel
f     interpolation during resampling of gridded data (you must also
f     set the INTERP argument of AST_RESAMPLE<X> to the value
f     AST__UKERN1). This allows you to use your own interpolation
f     kernel in addition to those which are pre-defined.
*
c     The function calculates the value of a 1-dimensional sub-pixel
f     The routine calculates the value of a 1-dimensional sub-pixel
*     interpolation kernel. This determines how the weight given to
*     neighbouring pixels in calculating an interpolated value depends
*     on the pixel's offset from the interpolation point. In more than
*     one dimension, the weight assigned to a pixel is formed by
*     evaluating this 1-dimensional kernel using the offset along each
*     dimension in turn. The product of the returned values is then
*     used as the pixel weight.

*  Parameters:
c     offset
f     OFFSET = DOUBLE PRECISION (Given)
*        This will be the offset of the pixel from the interpolation
*        point, measured in pixels. This value may be positive or
*        negative, but for most practical interpolation schemes its
*        sign should be ignored.
c     params
f     PARAMS( * ) = DOUBLE PRECISION (Given)
c        This will be a pointer to the same array as was given via the
c        "params" parameter of astResample<X>. You may use this to
f        This will be the same array as was given via the
f        PARAMS argument of AST_RESAMPLE<X>. You may use this to
*        pass any additional parameter values required by your kernel,
c        but note that params[0] will already have been used to specify
f        but note that PARAMS(1) will already have been used to specify
*        the number of neighbouring pixels which contribute to the
*        interpolated value.
c     flags
f     FLAGS = INTEGER (Given)
c        This will be the same value as was given via the "flags"
c        parameter of astResample<X>. You may test this value to
f        This will be the same value as was given via the FLAGS
f        argument of AST_RESAMPLE<X>. You may test this value to
*        provide additional control over the operation of your
c        function. Note that the special flag values AST__URESAMP1, 2,
f        routine. Note that the special flag values AST__URESAMP1, 2,
*        3 & 4 are reserved for you to use for your own purposes and
*        will not clash with other pre-defined flag
c        values (see astResample<X>).
f        values (see AST_RESAMPLE<X>).
c     value
f     VALUE = DOUBLE PRECISION (Returned)
c        Pointer to a double to receive the calculated kernel value,
f        The calculated kernel value,
*        which may be positive or negative.
f     STATUS = INTEGER (Given and Returned)
f        The global status.

*  Notes:
*     - Not all functions make good interpolation kernels. In general,
*     acceptable kernels tend to be symmetrical about zero, to have a
*     positive peak (usually unity) at zero, and to evaluate to zero
*     whenever the pixel offset has any other integral value (this
*     last property ensures that the interpolated values pass through
*     the original data). An interpolation kernel may or may not have
*     regions with negative values. You should consult a good book on
*     image processing for more details.
c     - If an error occurs within this function, it should use
c     astSetStatus to set the AST error status to an error value.
c     This will cause an immediate return from astResample<X>.
f     - If an error occurs within this routine, it should set the
f     STATUS argument to an error value before returning. This will
f     cause an immediate return from AST_RESAMPLE<X>.
*--
*/
/* Note the above is just a description to act as a template. The
   function does not actually exist. */

static double UphillSimplex( const MapData *mapdata, double acc, int maxcall,
                             const double dx[], double xmax[], double *err,
                             int *ncall ) {
/*
*  Name:
*     UphillSimplex

*  Purpose:
*     Find a function maximum using a modification of the simplex method.

*  Type:
*     Private function.

*  Synopsis:
*     #include "mapping.h"
*     double UphillSimplex( const MapData *mapdata, double acc, int maxcall,
*                           const double dx[], double xmax[], double *err,
*                           int *ncall );

*  Class Membership:
*     Mapping member function.

*  Description:
*     This function applies a modification of the simplex method to
*     find a local maximum in the value returned by a Mapping
*     function. The modification used allows the method to cope with
*     coordinate constraints and (equivalently) regions where the
*     function returns "bad" values. The method is robust and not
*     susceptible to overflow, so is suitable for applying to Mapping
*     functions of unknown form.

*  Parameters:
*     mapdata
*        Pointer to a MapData structure which describes the Mapping
*        function, its coordinate constraints, etc.
*     acc
*        The accuracy required in the value of the maximum.
*     maxcall
*        The maximum number of Mapping function evaluations to use.
*     dx
*        Pointer to an array of double containing an offset along each
*        input coordinate for the Mapping function supplied. These
*        offsets will be used to construct the initial simplex
*        (i.e. they are the initial "step lengths" for each
*        coordinate) and may be positive or negative.
*     xmax
*        Pointer to an array of double which contains the coordinates
*        of an initial estimate of the location of the maximum. On
*        exit, this will be updated to contain the best estimate of
*        the location of the maximum as generated by this function.
*     err
*        Pointer to a double in which to return an estimate of the
*        error in the value of the maximum found. For normal
*        convergence, this should be no larger than "acc". However, if
*        the maximum number of Mapping function evaluations is
*        reached, the returned value may be larger than this, although
*        it should still be valid. In such cases, re-starting the
*        algorithm at the new location returned in "xmax" may be
*        advisable.
*     ncall
*        Pointer to an int in which the number of Mapping function
*        evaluations will be returned.

*  Returned Value:
*     An estimate of the Mapping function value at the local maximum.

*  Notes:
*     - The function may return before the requested accuracy has been
*     met and before all Mapping function evaluations have been
*     made. This signifies that an excessive number of function values
*     have been needed outside the coordinate constraints. This is
*     only likely if the function is unable to make progress near such
*     a constraint, in which case the algorithm should probably be
*     re-started.
*     - A value of AST__BAD will be returned if no maximum could be
*     found.  This means that all the Mapping function evaluations
*     performed returned a value of AST__BAD.
*     - A value of AST__BAD will also be returned and no useful
*     information about a solution will be produced if this routine is
*     invoked with the global error status set, or if it should fail
*     for any reason.
*/

/* Local Constants: */
   const double factor = 3.0;    /* Simplex contraction/expansion factor */

/* Local Variables: */
   double *f;                    /* Pointer to array of function values */
   double *x;                    /* Pointer to array of vertex coordinates */
   double *xnew;                 /* Pointer to workspace array */
   double fnew;                  /* New function value */
   double fsave;                 /* Saved function value */
   double offset;                /* Coordinate difference between vertices */
   double range;                 /* Range of simplex values */
   double result;                /* Value to return */
   double tmp;                   /* Temporary store for coordinate */
   int coord;                    /* Loop counter for coordinates */
   int hi;                       /* Index of best vertex */
   int lo;                       /* Index of worst vertex */
   int ncalla;                   /* Number of function calls attempted */
   int ncoord;                   /* Number of function dimensions */
   int nextlo;                   /* Index of second worst vertex */
   int nvertex;                  /* Number of simplex vertices */
   int vertex;                   /* Loop counter for vertices */

/* Initialise. */
   result = AST__BAD;

/* Check the global error status. */
   if ( !astOK ) return result;

/* Further initialisation. */
   *err = DBL_MAX;
   *ncall = 0;

/* Obtain the number of input coordinates for the Mapping function and
   calculate the number of simplex vertices. */
   ncoord = mapdata->nin;
   nvertex = ncoord + 1;

/* Allocate workspace. */
   f = astMalloc( sizeof( double ) * (size_t) nvertex );
   x = astMalloc( sizeof( double ) * (size_t) ( ncoord * nvertex ) );
   xnew = astMalloc( sizeof( double ) * (size_t) ncoord );
   if ( astOK ) {

/* Loop to set up an initial simplex. */
      for ( vertex = 0; vertex < nvertex; vertex++ ) {
         for ( coord = 0; coord < ncoord; coord++ ) {
            tmp = xmax[ coord ];

/* Displace each point (except the first) the required amount along
   one of the axes to generate the coordinates of the simplex
   vertices. */
            if ( coord == ( vertex - 1 ) ) tmp += dx[ coord ];
            x[ vertex * ncoord + coord ] = tmp;
         }

/* Evaluate the Mapping function at each vertex. */
         f[ vertex ] = MapFunction( mapdata, &x[ vertex * ncoord ], ncall );
         if ( f[ vertex ] == AST__BAD ) f[ vertex ] = -DBL_MAX;
      }

/* Initialise the number of times we attempt to call the Mapping
   function (not necessarily the same as the number of times it was
   actually called, which is stored in *ncall). */
      ncalla = nvertex;

/* Loop until convergence is reached or an error occurs. */
      while( astOK ) {

/* Initialise the index of the lowest vertex of the simplex, the next
   lowest vertex and the highest vertex. */
         lo = ( f[ 0 ] < f[ 1 ] ) ? 0 : 1;
         nextlo = 1 - lo;
         hi = 0;

/* Loop to inspect each vertex and update these values. Ensure that in
   the case of equal vertices, the first one is taken to be the
   highest. This makes the maximisation stable (so that if no better
   maximum can be found, the original position is returned rather than
   a nearby position that yields the same function value). */
         for ( vertex = 0; vertex < nvertex; vertex++ ) {
            if ( f[ vertex ] <= f[ lo ] ) {
               nextlo = lo;
               lo = vertex;
            } else if ( ( f[ vertex ] <= f[ nextlo ] ) && ( vertex != lo ) ) {
               nextlo = vertex;
            }
            if ( f[ vertex ] > f[ hi ] ) hi = vertex;
         }

/* Estimate the error on the result as the difference between the
   highest and lowest simplex vertices. */
         if ( ( f[ hi ] == -DBL_MAX ) || ( f[ lo ] == -DBL_MAX ) ) {
            range = DBL_MAX;
         } else {
            range = f[ hi ] - f[ lo ];
         }

/* Test for convergence. Ideally, the accuracy criterion should have
   been met. However, also quit if the maximum number of Mapping
   function evaluations has been reached, or the number of points at
   which function values have been requested reaches three times this
   limit (this latter number will typically be larger because points
   lying outside the coordinate constraints do not result in the
   Mapping function being evaluated). */
         if ( range <= fabs( acc ) ||
              ( *ncall >= maxcall ) || ( ncalla >= ( 3 * maxcall ) ) ) {

/* If quitting, return the coordinates and function value at the best
   simplex vertex, and the error estimate. */
            for ( coord = 0; coord < ncoord; coord++ ) {
               xmax[ coord ] = x[ hi * ncoord + coord ];
            }
            result = ( f[ hi ] == -DBL_MAX ) ? AST__BAD : f[ hi ];
            *err = range;
            break;
         }

/* If performing another iteration, first try reflecting the worst
   vertex through the opposite face of the simplex. Check for
   errors. */
         fnew = NewVertex( mapdata, lo, -1.0, x, f, ncall, xnew );
         ncalla++;
         if ( astOK ) {

/* If this results in a point lying in a forbiddden region (either
   outside the coordinate constraints or where the Mapping function
   yields bad coordinate values), then we must make a departure from
   the standard simplex algorithm. This is because the inability to
   make forward progress in this case can cause the simplex to
   repeatedly contract about each face (except one) in turn. This
   mechanism normally results in lateral contraction as the simplex
   attempts to squeeze through a narrow gap which is impeding
   progress. However, in this case there is no gap to get through, so
   the lateral contraction can eventually make the simplex become
   degenerate (due to rounding). This prevents it from expanding
   laterally again and exploring the region adjacent to the constraint
   boundary once it has become small enough. */
            if ( fnew == AST__BAD ) {

/* To overcome this, we instead contract the worst simplex vertex
   towards the best vertex (this has the cumulative effect of
   contracting the simplex without changing its shape). First find the
   offset in each coordinate between these two vertices. */
               for ( coord = 0; coord < ncoord; coord++ ) {
                  offset = x[ lo * ncoord + coord ] - x[ hi * ncoord + coord ];

/* Scale the offset to obtain the new coordinate. */
                  x[ lo * ncoord + coord ] = x[ hi * ncoord + coord ] +
                                             offset / factor;

/* If the distance between the two vertices has not decreased, we are
   in a region where rounding errors prevent them approaching each
   other any more closely, so simply set them equal. */
                  if ( fabs( x[ lo * ncoord + coord ] -
                             x[ hi * ncoord + coord ] ) >= fabs( offset ) ) {
                     x[ lo * ncoord + coord ] = x[ hi * ncoord + coord ];
                  }
               }

/* Evaluate the Mapping function at the new vertex. */
               f[ lo ] = MapFunction( mapdata, &x[ lo * ncoord ], ncall );
               if ( f[ lo ] == AST__BAD ) f[ lo ] = -DBL_MAX;
               ncalla++;

/* We now return to the standard simplex algorithm. If the new vertex
   is a new maximum, then see if more of the same is even better by
   trying to expand the best vertex away from the opposite face. */
            } else if ( fnew >= f[ hi ] ) {
               fnew = NewVertex( mapdata, lo, factor, x, f, ncall, xnew );
               ncalla++;

/* Otherwise, if the new vertex was no improvement on the second
   worst, then try contracting the worst vertex towards the opposite
   face. */
            } else if ( fnew <= f[ nextlo ] ) {
               fsave = f[ lo ];
               fnew = NewVertex( mapdata, lo, 1.0 / factor, x, f, ncall, xnew );
               ncalla++;

/* If this didn't result in any improvement, then contract the entire
   simplex towards the best vertex. Use the same approach as earlier
   to protect against rounding so that all the simplex vertices will
   eventually coalesce if this process is repeated enough times. */
               if ( astOK && ( fnew <= fsave ) ) {
                  for ( vertex = 0; vertex < nvertex; vertex++ ) {
                     if ( vertex != hi ) {
                        for ( coord = 0; coord < ncoord; coord++ ) {
                           offset = x[ vertex * ncoord + coord ] -
                                    x[ hi * ncoord + coord ];
                           x[ vertex * ncoord + coord ] =
                               x[ hi * ncoord + coord ] + offset / factor;
                           if ( fabs( x[ vertex * ncoord + coord ] -
                                      x[ hi * ncoord + coord ] ) >=
                                fabs( offset ) ) {
                              x[ vertex * ncoord + coord ] =
                                 x[ hi * ncoord + coord ];
                           }
                        }

/* Evaluate the Mapping function at each new vertex. */
                        f[ vertex ] = MapFunction( mapdata,
                                                   &x[ vertex * ncoord ],
                                                   ncall );
                        if ( f[ vertex ] == AST__BAD ) f[ vertex ] = -DBL_MAX;
                        ncalla++;
                     }
                  }
               }
            }
         }
      }
   }

/* Free workspace. */
   f = astFree( f );
   x = astFree( x );
   xnew = astFree( xnew );

/* If an error occurred, clear the returned result. */
   if ( !astOK ) result = AST__BAD;

/* Return the result. */
   return result;
}

static void ValidateMapping( AstMapping *this, int forward,
                             int npoint, int ncoord_in, int ncoord_out,
                             const char *method ) {
/*
*  Name:
*     ValidateMapping

*  Purpose:
*     Validate a Mapping for use to transform coordinates.

*  Type:
*     Private function.

*  Synopsis:
*     #include "mapping.h"
*     void ValidateMapping( AstMapping *this, int forward,
*                           int npoint, int ncoord_in, int ncoord_out,
*                           const char *method )

*  Class Membership:
*     Mapping member function.

*  Description:
*     This function checks that a Mapping is suitable for transforming
*     a set of points. It also checks that the number of points and
*     the number of coordinate values per point is valid. If an error
*     is detected, the global error status is set and an error report
*     made. Otherwise, the function returns without further action.

*  Parameters:
*     this
*        Pointer to the Mapping.
*     forward
*        A non-zero value indicates that the forward coordinate
*        transformation is to be checked, while a zero value requests
*        the inverse transformation.
*     npoint
*        The number of points being transformed.
*     ncoord_in
*        The number of coordinates associated with each input point.
*     ncoord_out
*        The number of coordinates associated with each output point.
*     method
*        Pointer to a null terminated character string containing the
*        name of the method which invoked this function to validate a
*        Mapping. This is used solely for constructing error messages.
*/

/* Local Variables: */
   int nin;                    /* Mapping Nin attribute value */
   int nout;                   /* Mapping Nout attribute value */

/* Check the global error status. */
   if ( !astOK ) return;

/* Report an error if the requested transformation is not defined. */
   if ( !( forward ? astGetTranForward( this ) : astGetTranInverse( this ) )
        && astOK ) {
      astError( AST__TRNND, "%s(%s): %s coordinate transformation "
                "is not defined by the %s supplied.", method,
                astGetClass( this ),
                ( forward ? "A forward" : "An inverse" ),
                astGetClass( this ) );
   }

/* Obtain the effective values of the Nin and Nout attributes for the
   Mapping. */
   nin = forward ? astGetNin( this ) : astGetNout( this );
   nout = forward ? astGetNout( this ) : astGetNin( this );

/* If OK, check that the number of input coordinates matches the
   number required by the Mapping. Report an error if these numbers do
   not match. */
   if ( astOK && ( ncoord_in != nin ) ) {
      astError( AST__NCPIN, "%s(%s): Bad number of input coordinate values "
                "(%d).", method, astGetClass( this ), ncoord_in );
      astError( AST__NCPIN, "The %s given requires %d coordinate value%s for "
                "each input point.", astGetClass( this ), nin,
                ( nin == 1 ) ? "" : "s" );
   }

/* If OK, also check that the number of output coordinates matches the
   number required by the Mapping. Report an error if these numbers do
   not match. */
   if ( astOK && ( ncoord_out != nout ) ) {
      astError( AST__NCPIN, "%s(%s): Bad number of output coordinate values "
                "(%d).", method, astGetClass( this ), ncoord_out );
      astError( AST__NCPIN, "The %s given generates %s%d coordinate value%s "
                "for each output point.", astGetClass( this ),
                ( nout < ncoord_out ) ? "only " : "", nout,
                ( nout == 1 ) ? "" : "s" );
   }

/* Check that the number of points being transformed is not negative
   and report an error if necessary. */
   if ( astOK && ( npoint < 0 ) ) {
      astError( AST__NPTIN, "%s(%s): Number of points to be transformed (%d) "
                "is invalid.", method, astGetClass( this ), npoint );
   }
}

/* Functions which access class attributes. */
/* ---------------------------------------- */
/* Implement member functions to access the attributes associated with
   this class using the macros defined for this purpose in the
   "object.h" file. */
/*
*att++
*  Name:
*     Invert

*  Purpose:
*     Mapping inversion flag.

*  Type:
*     Public attribute.

*  Synopsis:
*     Integer (boolean).

*  Description:
*     This attribute controls which one of a Mapping's two possible
*     coordinate transformations is considered the "forward"
*     transformation (the other being the "inverse"
*     transformation). If the attribute value is zero (the default),
*     the Mapping's behaviour will be the same as when it was first
*     created. However, if it is non-zero, its two transformations
*     will be inter-changed, so that the Mapping displays the inverse
*     of its original behaviour.
*
*     Inverting the boolean sense of the Invert attribute will cause
*     the values of a Mapping's Nin and Nout attributes to be
*     interchanged. The values of its TranForward and TranInverse
*     attributes will also be interchanged. This operation may be
c     performed with the astInvert function.
f     performed with the AST_INVERT routine.

*  Applicability:
*     Mapping
*        All Mappings have this attribute.
*     UnitMap
*        The value of the Invert attribute has no effect on the
*        behaviour of a UnitMap.
*     FrameSet
*        Inverting the boolean sense of the Invert attribute for a
*        FrameSet will cause its base and current Frames (and its Base
*        and Current attributes) to be interchanged. This, in turn,
*        may affect other properties and attributes of the FrameSet
*        (such as Nin, Nout, Naxes, TranForward, TranInverse,
*        etc.). The Invert attribute of a FrameSet is not itself
*        affected by selecting a new base or current Frame.
*att--
*/
/* This ia a boolean value (0 or 1) with a value of -INT_MAX when
   undefined but yielding a default of zero. */
astMAKE_CLEAR(Mapping,Invert,invert,-INT_MAX)
astMAKE_GET(Mapping,Invert,int,0,( ( this->invert == -INT_MAX ) ?
                                   0 : this->invert ))
astMAKE_SET(Mapping,Invert,int,invert,( value != 0 ))
astMAKE_TEST(Mapping,Invert,( this->invert != -INT_MAX ))

/*
*att++
*  Name:
*     Nin

*  Purpose:
*     Number of input coordinates for a Mapping.

*  Type:
*     Public attribute.

*  Synopsis:
*     Integer, read-only.

*  Description:
*     This attribute gives the number of coordinate values required to
*     specify an input point for a Mapping (i.e. the number of
*     dimensions of the space in which the Mapping's input points
*     reside).

*  Applicability:
*     Mapping
*        All Mappings have this attribute.
*     CmpMap
*        If a CmpMap's component Mappings are joined in series, then
*        its Nin attribute is equal to the Nin attribute of the first
*        component (or to the Nout attribute of the second component
*        if the the CmpMap's Invert attribute is non-zero).
*
*        If a CmpMap's component Mappings are joined in parallel, then
*        its Nin attribute is given by the sum of the Nin attributes
*        of each component (or to the sum of their Nout attributes if
*        the CmpMap's Invert attribute is non-zero).
*     Frame
*        The Nin attribute for a Frame is always equal to the number
*        of Frame axes (Naxes attribute).
*     FrameSet
*        The Nin attribute of a FrameSet is equal to the number of
*        axes (Naxes attribute) of its base Frame (as specified by the
*        FrameSet's Base attribute). The Nin attribute value may
*        therefore change if a new base Frame is selected.
*att-- 
*/

/*
*att++
*  Name:
*     Nout

*  Purpose:
*     Number of output coordinates for a Mapping.

*  Type:
*     Public attribute.

*  Synopsis:
*     Integer, read-only.

*  Description:
*     This attribute gives the number of coordinate values generated
*     by a Mapping to specify each output point (i.e. the number of
*     dimensions of the space in which the Mapping's output points
*     reside).

*  Applicability:
*     Mapping
*        All Mappings have this attribute.
*     CmpMap
*        If a CmpMap's component Mappings are joined in series, then
*        its Nout attribute is equal to the Nout attribute of the
*        second component (or to the Nin attribute of the first
*        component if the the CmpMap's Invert attribute is non-zero).
*
*        If a CmpMap's component Mappings are joined in parallel, then
*        its Nout attribute is given by the sum of the Nout attributes
*        of each component (or to the sum of their Nin attributes if
*        the CmpMap's Invert attribute is non-zero).
*     Frame
*        The Nout attribute for a Frame is always equal to the number
*        of Frame axes (Naxes attribute).
*     FrameSet
*        The Nout attribute of a FrameSet is equal to the number of
*        FrameSet axes (Naxes attribute) which, in turn, is equal to
*        the Naxes attribute of the FrameSet's current Frame (as
*        specified by the Current attribute). The Nout attribute value
*        may therefore change if a new current Frame is selected.
*att--
*/

/*
*att++
*  Name:
*     Report

*  Purpose:
*     Report transformed coordinates?

*  Type:
*     Public attribute.

*  Synopsis:
*     Integer (boolean).

*  Description:
*     This attribute controls whether coordinate values are reported
*     whenever a Mapping is used to transform a set of points. If its
*     value is zero (the default), no report is made. However, if it
*     is non-zero, the coordinates of each point are reported (both
*     before and after transformation) by writing them to standard
*     output.
*
*     This attribute is provided as an aid to debugging, and to avoid
*     having to report values explicitly in simple programs.

*  Applicability:
*     Mapping
*        All Mappings have this attribute.
*     CmpMap
*        When applied to a compound Mapping (CmpMap), only the Report
*        attribute of the CmpMap, and not those of its component
*        Mappings, is used.  Coordinate information is never reported
*        for the component Mappings individually, only for the
*        complete CmpMap.
*     Frame
*        When applied to any Frame, the formatting capabilities of the
c        Frame (as provided by the astFormat function) will be used to
f        Frame (as provided by the AST_FORMAT function) will be used to
*        format the reported coordinates.
*     FrameSet
*        When applied to any FrameSet, the formatting capabilities of
*        the base and current Frames will be used (as above) to
*        individually format the input and output coordinates, as
*        appropriate. The Report attribute of a FrameSet is not itself
*        affected by selecting a new base or current Frame, but the
*        resulting formatting capabilities may be.

*  Notes:
*     - Unlike most other attributes, the value of the Report
*     attribute is not transferred when a Mapping is copied. Instead,
*     its value is undefined (and therefore defaults to zero) in any
*     copy. Similarly, it becomes undefined in any external
c     representation of a Mapping produced by the astWrite function.
f     representation of a Mapping produced by the AST_WRITE routine.
*att--
*/
/* This ia a boolean value (0 or 1) with a value of -INT_MAX when
   undefined but yielding a default of zero. */
astMAKE_CLEAR(Mapping,Report,report,-INT_MAX)
astMAKE_GET(Mapping,Report,int,0,( ( this->report == -INT_MAX ) ?
                                   0 : this->report ))
astMAKE_SET(Mapping,Report,int,report,( value != 0 ))
astMAKE_TEST(Mapping,Report,( this->report != -INT_MAX ))

/*
*att++
*  Name:
*     TranForward

*  Purpose:
*     Forward transformation defined?

*  Type:
*     Public attribute.

*  Synopsis:
*     Integer (boolean), read-only.

*  Description:
*     This attribute indicates whether a Mapping is able to transform
*     coordinates in the "forward" direction (i.e. converting input
*     coordinates into output coordinates). If this attribute is
*     non-zero, the forward transformation is available. Otherwise, it
*     is not.

*  Applicability:
*     Mapping
*        All Mappings have this attribute.
*     CmpMap
*        The TranForward attribute value for a CmpMap is given by the
*        boolean AND of the value for each component Mapping.
*     FrameSet
*        The TranForward attribute of a FrameSet applies to the
*        transformation which converts between the FrameSet's base
*        Frame and its current Frame (as specified by the Base and
*        Current attributes). This value is given by the boolean AND
*        of the TranForward values which apply to each of the
*        individual sub-Mappings required to perform this conversion.
*        The TranForward attribute value for a FrameSet may therefore
*        change if a new Base or Current Frame is selected.

*  Notes:
*     - An error will result if a Mapping with a TranForward value of
*     zero is used to transform coordinates in the forward direction.
*att--
*/

/*
*att++
*  Name:
*     TranInverse

*  Purpose:
*     Inverse transformation defined?

*  Type:
*     Public attribute.

*  Synopsis:
*     Integer (boolean), readonly.

*  Description:
*     This attribute indicates whether a Mapping is able to transform
*     coordinates in the "inverse" direction (i.e. converting output
*     coordinates back into input coordinates). If this attribute is
*     non-zero, the inverse transformation is available. Otherwise, it
*     is not.

*  Applicability:
*     Mapping
*        All Mappings have this attribute.
*     CmpMap
*        The TranInverse attribute value for a CmpMap is given by the
*        boolean AND of the value for each component Mapping.
*     FrameSet
*        The TranInverse attribute of a FrameSet applies to the
*        transformation which converts between the FrameSet's current
*        Frame and its base Frame (as specified by the Current and
*        Base attributes). This value is given by the boolean AND of
*        the TranInverse values which apply to each of the individual
*        sub-Mappings required to perform this conversion.
*        The TranInverse attribute value for a FrameSet may therefore
*        change if a new Base or Current Frame is selected.

*  Notes:
*     - An error will result if a Mapping with a TranInverse value of
*     zero is used to transform coordinates in the inverse direction.
*att--
*/

/* Copy constructor. */
/* ----------------- */
static void Copy( const AstObject *objin, AstObject *objout ) {
/*
*  Name:
*     Copy

*  Purpose:
*     Copy constructor for Mapping objects.

*  Type:
*     Private function.

*  Synopsis:
*     void Copy( const AstObject *objin, AstObject *objout )

*  Description:
*     This function implements the copy constructor for Mapping objects.

*  Parameters:
*     objin
*        Pointer to the Mapping to be copied.
*     objout
*        Pointer to the Mapping being constructed.

*  Notes:
*     - This constructor exists simply to ensure that the "Report"
*     attribute is cleared in any copy made of a Mapping.
*/

/* Local Variables: */
   AstMapping *out;              /* Pointer to output Mapping */

/* Check the global error status. */
   if ( !astOK ) return;

/* Obtain a pointer to the output Mapping. */
   out = (AstMapping *) objout;

/* Clear the output Report attribute. */
   out->report = -INT_MAX;
}

/* Destructor. */
/* ----------- */
static void Delete( AstObject *obj ) {
/*
*  Name:
*     Delete

*  Purpose:
*     Destructor for Mapping objects.

*  Type:
*     Private function.

*  Synopsis:
*     void Delete( AstObject *obj )

*  Description:
*     This function implements the destructor for Mapping objects.

*  Parameters:
*     obj
*        Pointer to the Mapping to be deleted.

*  Notes:
*     - This destructor does nothing and exists only to maintain a
*     one-to-one correspondence between destructors and copy
*     constructors.
*/

/* Return without action. */
}

/* Dump function. */
/* -------------- */
static void Dump( AstObject *this_object, AstChannel *channel ) {
/*
*  Name:
*     Dump

*  Purpose:
*     Dump function for Mapping objects.

*  Type:
*     Private function.

*  Synopsis:
*     void Dump( AstObject *this, AstChannel *channel )

*  Description:
*     This function implements the Dump function which writes out data
*     for the Mapping class to an output Channel.

*  Parameters:
*     this
*        Pointer to the Mapping whose data are being written.
*     channel
*        Pointer to the Channel to which the data are being written.
*/

/* Local Variables: */
   AstMapping *this;             /* Pointer to the Mapping structure */
   int invert;                   /* Mapping inverted? */
   int ival;                     /* Integer value */
   int set;                      /* Attribute value set? */

/* Check the global error status. */
   if ( !astOK ) return;

/* Obtain a pointer to the Mapping structure. */
   this = (AstMapping *) this_object;

/* Write out values representing the instance variables for the
   Mapping class.  Accompany these with appropriate comment strings,
   possibly depending on the values being written.*/

/* In the case of attributes, we first use the appropriate (private)
   Test...  member function to see if they are set. If so, we then use
   the (private) Get... function to obtain the value to be written
   out.

   For attributes which are not set, we use the astGet... method to
   obtain the value instead. This will supply a default value
   (possibly provided by a derived class which over-rides this method)
   which is more useful to a human reader as it corresponds to the
   actual default attribute value.  Since "set" will be zero, these
   values are for information only and will not be read back. */

/* Determine if the Mapping is inverted. The output values
   (e.g. number of input and output coordinates) will refer to the
   Mapping ***before*** this inversion flag is applied, but we need it
   when using (e.g.) the astGetNin/astGetNout methods to determine
   which one will return the required value. */
   invert = astGetInvert( this );

/* (NB. there is a subtle point here that dictates the extent to which
   this inversion flag can be used... All use of methods (such as
   astGetInvert, which might be over-ridden by derived classes) must
   be restricted to determining the values of "unset" output
   quantities only (below). This is because when re-loading the
   Mapping, the derived classes will not have been loaded at the point
   when these values are re-read - hence any value whose
   interpretation depends on these methods cannot be reliably
   recovered.) */

/* Nin. */
/* ---- */
/* Use the instance variable directly to avoid the effect of the
   Invert attribute on the private member function. Treat zero as the
   default. */
   set = ( this->nin != 0 );
   ival = set ? this->nin : ( !invert ? astGetNin( this ) :
                                        astGetNout( this ) );
   astWriteInt( channel, "Nin", set, 0, ival,
                "Number of input coordinates" );

/* Nout. */
/* ----- */
/* Use the instance variable directly. Treat zero as the default. */
   set = ( this->nout != this->nin );
   ival = set ? this->nout : ( !invert ? astGetNout( this ) :
                                         astGetNin( this ) );
   astWriteInt( channel, "Nout", set, 0, ival,
                "Number of output coordinates" );

/* Invert. */
/* ------- */
   set = TestInvert( this );
   ival = set ? GetInvert( this ) : astGetInvert( this );
   astWriteInt( channel, "Invert", set, 0, ival,
                ival ? "Mapping inverted" :
                       "Mapping not inverted" );

/* TranForward. */
/* ------------ */
/* Use the instance variable directly. Treat 1 as the default. */
   set = ( this->tran_forward == 0 );
   ival = set ? this->tran_forward : ( !invert ? astGetTranForward( this ) :
                                                 astGetTranInverse( this ) );
   astWriteInt( channel, "Fwd", set, 0, ival,
                ival ? "Forward transformation defined" :
                       "Forward transformation not defined" );

/* TranInverse. */
/* ------------ */
/* Use the instance variable directly. Treat 1 as the default. */
   set = ( this->tran_inverse == 0 );
   ival = set ? this->tran_inverse : ( !invert ? astGetTranInverse( this ) :
                                                 astGetTranForward( this ) );
   astWriteInt( channel, "Inv", set, 0, ival,
                ival ? "Inverse transformation defined" :
                       "Inverse transformation not defined" );

/* Report. */
/* ------- */
   set = TestReport( this );
   ival = set ? GetReport( this ) : astGetReport( this );
   astWriteInt( channel, "Report", set, 0, ival,
                ival ? "Report coordinate transformations" :
                       "Don't report coordinate transformations" );
}

/* Standard class functions. */
/* ========================= */
/* Implement the astIsAMapping and astCheckMapping functions using the macros
   defined for this purpose in the "object.h" header file. */
astMAKE_ISA(Mapping,Object,check,&class_init)
astMAKE_CHECK(Mapping)

AstMapping *astInitMapping_( void *mem, size_t size, int init,
                             AstMappingVtab *vtab, const char *name,
                             int nin, int nout,
                             int tran_forward, int tran_inverse ) {
/*
*+
*  Name:
*     astInitMapping

*  Purpose:
*     Initialise a Mapping.

*  Type:
*     Protected function.

*  Synopsis:
*     #include "mapping.h"
*     AstMapping *astInitMapping( void *mem, size_t size, int init,
*                                 AstMappingVtab *vtab, const char *name,
*                                 int nin, int nout,
*                                 int tran_forward, int tran_inverse )

*  Class Membership:
*     Mapping initialiser.

*  Description:
*     This function is provided for use by class implementations to initialise
*     a new Mapping object. It allocates memory (if necessary) to accommodate
*     the Mapping plus any additional data associated with the derived class.
*     It then initialises a Mapping structure at the start of this memory. If
*     the "init" flag is set, it also initialises the contents of a virtual
*     function table for a Mapping at the start of the memory passed via the
*     "vtab" parameter.

*  Parameters:
*     mem
*        A pointer to the memory in which the Mapping is to be initialised.
*        This must be of sufficient size to accommodate the Mapping data
*        (sizeof(Mapping)) plus any data used by the derived class. If a value
*        of NULL is given, this function will allocate the memory itself using
*        the "size" parameter to determine its size.
*     size
*        The amount of memory used by the Mapping (plus derived class data).
*        This will be used to allocate memory if a value of NULL is given for
*        the "mem" parameter. This value is also stored in the Mapping
*        structure, so a valid value must be supplied even if not required for
*        allocating memory.
*     init
*        A logical flag indicating if the Mapping's virtual function table is
*        to be initialised. If this value is non-zero, the virtual function
*        table will be initialised by this function.
*     vtab
*        Pointer to the start of the virtual function table to be associated
*        with the new Mapping.
*     name
*        Pointer to a constant null-terminated character string which contains
*        the name of the class to which the new object belongs (it is this
*        pointer value that will subsequently be returned by the astGetClass
*        method).
*     nin
*        The number of coordinate values per input point.
*     nout
*        The number of coordinate vales per output point.
*     tran_forward
*        A non-zero value indicates that the Mapping will be able to
*        transform coordinates in the forward direction. A zero value
*        indicates that it will not.
*     tran_inverse
*        A non-zero value indicates that the Mapping will be able to
*        transform coordinates in the inverse direction. A zero value
*        indicates that it will not.

*  Returned Value:
*     A pointer to the new Mapping.

*  Notes:
*     -  The Mappings produced by this function implement all the basic methods
*     defined by the Mapping class. However, their astTransform method does not
*     actually perform any coordinate transformation (although it performs all
*     necessary argument validation and creates an output PointSet if
*     necessary, leaving its coordinate values undefined).
*     -  This means that Mappings produced by this function are of limited use
*     on their own, but may easily be extended by a derived class simply by
*     over-riding the astTransform method to add the necessary coordinate
*     arithmetic.
*     -  A null pointer will be returned if this function is invoked with the
*     global error status set, or if it should fail for any reason.
*-
*/

/* Local Variables: */
   AstMapping *new;              /* Pointer to new Mapping */

/* Check the global status. */
   if ( !astOK ) return NULL;

/* Initialise. */
   new = NULL;

/* Check the initialisation values for validity, reporting an error if
   necessary. */
   if ( nin < 0 ) {
      astError( AST__BADNI, "astInitMapping(%s): Bad number of input "
                "coordinates (%d).", name, nin );
      astError( AST__BADNI, "This number should be zero or more." );
   } else if ( nout < 0 ) {
      astError( AST__BADNO, "astInitMapping(%s): Bad number of output "
                "coordinates (%d).", name, nout );
      astError( AST__BADNI, "This number should be zero or more." );
   }

/* Check that the coordinate transformation is defined in at least one
   direction (forward or inverse) and report an error if it is not. */
   if ( astOK ) {
      if ( !tran_forward && !tran_inverse ) {
         astError( AST__NODEF, "astInitMapping(%s): The coordinate "
                   "transformation is not defined in either the forward or "
                   "inverse direction.", name );
      }
   }

/* Initialise an Object structure (the parent class) as the first component
   within the Mapping structure, allocating memory if necessary. */
   new = (AstMapping *) astInitObject( mem, size, init,
                                       (AstObjectVtab *) vtab, name );

/* If necessary, initialise the virtual function table. */
/* ---------------------------------------------------- */
   if ( init ) InitVtab( vtab );
   if ( astOK ) {

/* Initialise the Mapping data. */
/* ---------------------------- */
/* Store the numbers of input and output coordinates. */
      new->nin = nin;
      new->nout = nout;

/* Store the flags indicating which coordinate transformations are
   defined (constrain these values to 0 or 1). */
      new->tran_forward = ( tran_forward != 0 );
      new->tran_inverse = ( tran_inverse != 0 );

/* Initialise other attributes to their undefined values. */
      new->invert = -INT_MAX;
      new->report = -INT_MAX;

/* If an error occurred, clean up by deleting the new object. */
      if ( !astOK ) new = astDelete( new );
   }

/* Return a pointer to the new object. */
   return new;
}

AstMapping *astLoadMapping_( void *mem, size_t size, int init,
                             AstMappingVtab *vtab, const char *name,
                             AstChannel *channel ) {
/*
*+
*  Name:
*     astLoadMapping

*  Purpose:
*     Load a Mapping.

*  Type:
*     Protected function.

*  Synopsis:
*     #include "mapping.h"
*     AstMapping *astLoadMapping( void *mem, size_t size, int init,
*                                 AstMappingVtab *vtab, const char *name,
*                                 AstChannel *channel )

*  Class Membership:
*     Mapping loader.

*  Description:
*     This function is provided to load a new Mapping using data read
*     from a Channel. It first loads the data used by the parent class
*     (which allocates memory if necessary) and then initialises a
*     Mapping structure in this memory, using data read from the input
*     Channel.
*
*     If the "init" flag is set, it also initialises the contents of a
*     virtual function table for a Mapping at the start of the memory
*     passed via the "vtab" parameter.

*  Parameters:
*     mem
*        A pointer to the memory into which the Mapping is to be
*        loaded.  This must be of sufficient size to accommodate the
*        Mapping data (sizeof(Mapping)) plus any data used by derived
*        classes. If a value of NULL is given, this function will
*        allocate the memory itself using the "size" parameter to
*        determine its size.
*     size
*        The amount of memory used by the Mapping (plus derived class
*        data).  This will be used to allocate memory if a value of
*        NULL is given for the "mem" parameter. This value is also
*        stored in the Mapping structure, so a valid value must be
*        supplied even if not required for allocating memory.
*
*        If the "vtab" parameter is NULL, the "size" value is ignored
*        and sizeof(AstMapping) is used instead.
*     init
*        A boolean flag indicating if the Mapping's virtual function
*        table is to be initialised. If this value is non-zero, the
*        virtual function table will be initialised by this function.
*
*        If the "vtab" parameter is NULL, the "init" value is ignored
*        and the (static) virtual function table initialisation flag
*        for the Mapping class is used instead.
*     vtab
*        Pointer to the start of the virtual function table to be
*        associated with the new Mapping. If this is NULL, a pointer
*        to the (static) virtual function table for the Mapping class
*        is used instead.
*     name
*        Pointer to a constant null-terminated character string which
*        contains the name of the class to which the new object
*        belongs (it is this pointer value that will subsequently be
*        returned by the astGetClass method).
*
*        If the "vtab" parameter is NULL, the "name" value is ignored
*        and a pointer to the string "Mapping" is used instead.

*  Returned Value:
*     A pointer to the new Mapping.

*  Notes:
*     - A null pointer will be returned if this function is invoked
*     with the global error status set, or if it should fail for any
*     reason.
*-
*/

/* Local Variables: */
   AstMapping *new;              /* Pointer to the new Mapping */

/* Initialise. */
   new = NULL;

/* Check the global error status. */
   if ( !astOK ) return new;

/* If a NULL virtual function table has been supplied, then this is
   the first loader to be invoked for this Mapping. In this case the
   Mapping belongs to this class, so supply appropriate values to be
   passed to the parent class loader (and its parent, etc.). */
   if ( !vtab ) {
      size = sizeof( AstMapping );
      init = !class_init;
      vtab = &class_vtab;
      name = "Mapping";
   }

/* Invoke the parent class loader to load data for all the ancestral
   classes of the current one, returning a pointer to the resulting
   partly-built Mapping. */
   new = astLoadObject( mem, size, init, (AstObjectVtab *) vtab, name,
                        channel );

/* If required, initialise the part of the virtual function table used
   by this class. */
   if ( init ) InitVtab( vtab );

/* Note if we have successfully initialised the (static) virtual
   function table owned by this class (so that this is done only
   once). */
   if ( astOK ) {
      if ( ( vtab == &class_vtab ) && init ) class_init = 1;

/* Read input data. */
/* ================ */
/* Request the input Channel to read all the input data appropriate to
   this class into the internal "values list". */
      astReadClassData( channel, "Mapping" );

/* Now read each individual data item from this list and use it to
   initialise the appropriate instance variable(s) for this class. */

/* In the case of attributes, we first read the "raw" input value,
   supplying the "unset" value as the default. If a "set" value is
   obtained, we then use the appropriate (private) Set... member
   function to validate and set the value properly. */

/* Nin. */
/* ---- */
      new->nin = astReadInt( channel, "nin", 0 );
      if ( new->nin < 0 ) new->nin = 0;

/* Nout. */
/* ----- */
      new->nout = astReadInt( channel, "nout", new->nin );
      if ( new->nout < 0 ) new->nout = 0;

/* Invert. */
/* ------- */
      new->invert = astReadInt( channel, "invert", -INT_MAX );
      if ( TestInvert( new ) ) SetInvert( new, new->invert );

/* TranForward. */
/* ------------ */
      new->tran_forward = ( astReadInt( channel, "fwd", 1 ) != 0 );

/* TranInverse. */
/* ------------ */
      new->tran_inverse = ( astReadInt( channel, "inv", 1 ) != 0 );

/* Report. */
/* ------- */
      new->report = astReadInt( channel, "report", -INT_MAX );
      if ( TestReport( new ) ) SetReport( new, new->report );

/* If an error occurred, clean up by deleting the new Mapping. */
      if ( !astOK ) new = astDelete( new );
   }

/* Return the new Mapping pointer. */
   return new;
}

/* Virtual function interfaces. */
/* ============================ */
/* These provide the external interface to the virtual functions
   defined by this class. Each simply checks the global error status
   and then locates and executes the appropriate member function,
   using the function pointer stored in the object's virtual function
   table (this pointer is located using the astMEMBER macro defined in
   "object.h").

   Note that the member function may not be the one defined here, as
   it may have been over-ridden by a derived class. However, it should
   still have the same interface. */

int astGetNin_( AstMapping *this ) {
   if ( !astOK ) return 0;
   return (**astMEMBER(this,Mapping,GetNin))( this );
}
int astGetNout_( AstMapping *this ) {
   if ( !astOK ) return 0;
   return (**astMEMBER(this,Mapping,GetNout))( this );
}
int astGetTranForward_( AstMapping *this ) {
   if ( !astOK ) return 0;
   return (**astMEMBER(this,Mapping,GetTranForward))( this );
}
int astGetTranInverse_( AstMapping *this ) {
   if ( !astOK ) return 0;
   return (**astMEMBER(this,Mapping,GetTranInverse))( this );
}
void astInvert_( AstMapping *this ) {
   if ( !astOK ) return;
   (**astMEMBER(this,Mapping,Invert))( this );
}
void astMapBox_( AstMapping *this,
                 const double lbnd_in[], const double ubnd_in[], int forward,
                 int coord_out, double *lbnd_out, double *ubnd_out,
                 double xl[], double xu[] ) {
   if ( !astOK ) return;
   (**astMEMBER(this,Mapping,MapBox))( this, lbnd_in, ubnd_in, forward,
                                       coord_out, lbnd_out, ubnd_out, xl, xu );
}
void astMapList_( AstMapping *this, int series, int invert, int *nmap,
                  AstMapping ***map_list, int **invert_list ) {
   if ( !astOK ) return;
   (**astMEMBER(this,Mapping,MapList))( this, series, invert,
                                        nmap, map_list, invert_list );
}
int astMapMerge_( AstMapping *this, int where, int series, int *nmap,
                  AstMapping ***map_list, int **invert_list ) {
   if ( !astOK ) return -1;
   return (**astMEMBER(this,Mapping,MapMerge))( this, where, series, nmap,
                                                map_list, invert_list );
}
void astReportPoints_( AstMapping *this, int forward,
                       AstPointSet *in_points, AstPointSet *out_points ) {
   if ( !astOK ) return;
   (**astMEMBER(this,Mapping,ReportPoints))( this, forward,
                                             in_points, out_points );
}
#define MAKE_RESAMPLE_(X,Xtype) \
int astResample##X##_( AstMapping *this, int ndim_in, const int *lbnd_in, \
                       const int *ubnd_in, const Xtype *in, \
                       const Xtype *in_var, int interp, \
                       void (* finterp)(), const double *params, \
                       int flags, double tol, int maxpix, Xtype badval, \
                       int ndim_out, \
                       const int *lbnd_out, const int *ubnd_out, \
                       const int *lbnd, const int *ubnd, Xtype *out, \
                       Xtype *out_var ) { \
   if ( !astOK ) return 0; \
   return (**astMEMBER(this,Mapping,Resample##X))( this, ndim_in, lbnd_in, \
                                                   ubnd_in, in, in_var, \
                                                   interp, finterp, params, \
                                                   flags, tol, maxpix, \
                                                   badval, ndim_out, \
                                                   lbnd_out, ubnd_out, \
                                                   lbnd, ubnd, \
                                                   out, out_var ); \
}
#if defined(AST_LONG_DOUBLE)     /* Not normally implemented */
MAKE_RESAMPLE_(LD,long double)
#endif
MAKE_RESAMPLE_(D,double)
MAKE_RESAMPLE_(F,float)
MAKE_RESAMPLE_(L,long int)
MAKE_RESAMPLE_(UL,unsigned long int)
MAKE_RESAMPLE_(I,int)
MAKE_RESAMPLE_(UI,unsigned int)
MAKE_RESAMPLE_(S,short int)
MAKE_RESAMPLE_(US,unsigned short int)
MAKE_RESAMPLE_(B,signed char)
MAKE_RESAMPLE_(UB,unsigned char)
#undef MAKE_RESAMPLE_
AstMapping *astSimplify_( AstMapping *this ) {
   if ( !astOK ) return NULL;
   return (**astMEMBER(this,Mapping,Simplify))( this );
}
AstPointSet *astTransform_( AstMapping *this, AstPointSet *in,
                            int forward, AstPointSet *out ) {
   if ( !astOK ) return NULL;
   return (**astMEMBER(this,Mapping,Transform))( this, in, forward, out );
}
void astTran1_( AstMapping *this, int npoint, const double xin[],
                int forward, double xout[] ) {
   if ( !astOK ) return;
   (**astMEMBER(this,Mapping,Tran1))( this, npoint, xin, forward, xout );
}
void astTran2_( AstMapping *this,
                int npoint, const double xin[], const double yin[],
                int forward, double xout[], double yout[] ) {
   if ( !astOK ) return;
   (**astMEMBER(this,Mapping,Tran2))( this, npoint, xin, yin,
                                      forward, xout, yout );
}
void astTranN_( AstMapping *this, int npoint,
                int ncoord_in, int indim, const double (*in)[],
                int forward, int ncoord_out, int outdim, double (*out)[] ) {
   if ( !astOK ) return;
   (**astMEMBER(this,Mapping,TranN))( this, npoint,
                                      ncoord_in, indim, in,
                                      forward, ncoord_out, outdim, out );
}
void astTranP_( AstMapping *this, int npoint,
                int ncoord_in, const double *ptr_in[],
                int forward, int ncoord_out, double *ptr_out[] ) {
   if ( !astOK ) return;
   (**astMEMBER(this,Mapping,TranP))( this, npoint,
                                      ncoord_in, ptr_in,
                                      forward, ncoord_out, ptr_out );
}

/* Public Interface Function Prototypes. */
/* ------------------------------------- */
/* The following functions have public prototypes only (i.e. no
   protected prototypes), so we must provide local prototypes for use
   within this module. */
void MapBoxId_( AstMapping *, const double [], const double [], int, int, double *, double *, double [], double [] );

/* Special interface function implementations. */
/* ------------------------------------------- */
void astMapBoxId_( AstMapping *this,
                   const double lbnd_in[], const double ubnd_in[],
                   int forward, int coord_out,
                   double *lbnd_out, double *ubnd_out,
                   double xl[], double xu[] ) {
/*
*++
*  Name:
c     astMapBox
f     AST_MAPBOX

*  Purpose:
*     Find a bounding box for a Mapping.

*  Type:
*     Protected virtual function.

*  Synopsis:
c     #include "mapping.h"
c     void astMapBox( AstMapping *this,
c                     const double lbnd_in[], const double ubnd_in[],
c                     int forward, int coord_out,
c                     double *lbnd_out, double *ubnd_out,
c                     double xl[], double xu[] );
f     CALL AST_MAPBOX( THIS, LBND_IN, UBND_IN, FORWARD, COORD_OUT,
f                      LBND_OUT, UBND_OUT, XL, XU, STATUS )

*  Class Membership:
*     Mapping method.

*  Description:
c     This function allows you to find the "bounding box" which just
c     encloses another box after it has been transformed by a Mapping
c     (using either its forward or inverse transformation). A typical
c     use might be to calculate the size of an image after being
c     transformed by a Mapping.
f     This routine allows you to find the "bounding box" which just
f     encloses another box after it has been transformed by a Mapping
f     (using either its forward or inverse transformation). A typical
f     use might be to calculate the size of an image after being
f     transformed by a Mapping.
*
c     The function works on one dimension at a time. When supplied
c     with the lower and upper bounds of a rectangular region (box) of
c     input coordinate space, it finds the lowest and highest values
c     taken by a nominated output coordinate within that
c     region. Optionally, it also returns the input coordinates where
c     these bounding values are attained. It should be used repeatedly
c     to obtain the extent of the bounding box in more than one
c     dimension.
f     The routine works on one dimension at a time. When supplied with
f     the lower and upper bounds of a rectangular region (box) of
f     input coordinate space, it finds the lowest and highest values
f     taken by a nominated output coordinate within that region. It
f     also returns the input coordinates where these bounding values
f     are attained. It should be used repeatedly to obtain the extent
f     of the bounding box in more than one dimension.

*  Parameters:
c     this
f     THIS = INTEGER (Given)
*        Pointer to the Mapping.
c     lbnd_in
f     LBND_IN( * ) = DOUBLE PRECISION (Given)
c        Pointer to an array of double, with one element for each
c        Mapping input coordinate. This should contain the lower bound
c        of the input box in each input dimension.
f        An array with one element for each Mapping input
f        coordinate. This should contain the lower bound of the input
f        box in each input dimension.
c     ubnd_in
f     UBND_IN( * ) = DOUBLE PRECISION (Given)
c        Pointer to an array of double, with one element for each
c        Mapping input coordinate. This should contain the upper bound
c        of the input box in each input dimension.
f        An array with one element for each Mapping input
f        coordinate. This should contain the upper bound of the input
f        box in each input dimension.
*
*        Note that it is permissible for the upper bound to be less
*        than the corresponding lower bound, as the values will simply
*        be swapped before use.
c     forward
f     FORWARD = LOGICAL (Given)
c        If this value is non-zero, then the Mapping's forward
c        transformation will be used to transform the input
c        box. Otherwise, its inverse transformation will be used.
f        If this value is .TRUE., then the Mapping's forward
f        transformation will be used to transform the input
f        box. Otherwise, its inverse transformation will be used.
*
c        (If the inverse transformation is selected, then references
c        to "input" and "output" coordinates in this description
c        should be transposed. For example, the size of the "lbnd_in"
c        and "ubnd_in" arrays should match the number of output
c        coordinates, as given by the Mapping's Nout
c        attribute. Similarly, the "coord_out" parameter, below,
c        should nominate one of the Mapping's input coordinates.)
f        (If the inverse transformation is selected, then references
f        to "input" and "output" coordinates in this description
f        should be transposed. For example, the size of the LBND_IN
f        and UBND_IN arrays should match the number of output
f        coordinates, as given by the Mapping's Nout attribute.
f        Similarly, the COORD_OUT argument, below, should nominate one
f        of the Mapping's input coordinates.)
c     coord_out
f     COORD_OUT = INTEGER (Given)
*        The index of the output coordinate for which the lower and
*        upper bounds are required. This value should be at least one,
*        and no larger than the number of Mapping output coordinates.
c     lbnd_out
f     LBND_OUT = DOUBLE PRECISION (Returned)
c        Pointer to a double in which to return the lowest value taken
c        by the nominated output coordinate within the specified
c        region of input coordinate space.
f        The lowest value taken by the nominated output coordinate
f        within the specified region of input coordinate space.
c     ubnd_out
f     UBND_OUT = DOUBLE PRECISION (Returned)
c        Pointer to a double in which to return the highest value
c        taken by the nominated output coordinate within the specified
c        region of input coordinate space.
f        The highest value taken by the nominated output coordinate
f        within the specified region of input coordinate space.
c     xl
f     XL( * ) = DOUBLE PRECISION (Returned)
c        An optional pointer to an array of double, with one element
c        for each Mapping input coordinate. If given, this array will
c        be filled with the coordinates of an input point (although
c        not necessarily a unique one) for which the nominated output
c        coordinate attains the lower bound value returned in
c        "*lbnd_out".
c
c        If these coordinates are not required, a NULL pointer may be
c        supplied.
f        An array with one element for each Mapping input
f        coordinate. This will return the coordinates of an input
f        point (although not necessarily a unique one) for which the
f        nominated output coordinate attains the lower bound value
f        returned in LBND_OUT.
c     xu
f     XU( * ) = DOUBLE PRECISION (Returned)
c        An optional pointer to an array of double, with one element
c        for each Mapping input coordinate. If given, this array will
c        be filled with the coordinates of an input point (although
c        not necessarily a unique one) for which the nominated output
c        coordinate attains the upper bound value returned in
c        "*ubnd_out".
c
c        If these coordinates are not required, a NULL pointer may be
c        supplied.
f        An array with one element for each Mapping input
f        coordinate. This will return the coordinates of an input
f        point (although not necessarily a unique one) for which the
f        nominated output coordinate attains the upper bound value
f        returned in UBND_OUT.
f     STATUS = INTEGER (Given and Returned)
f        The global status.

*  Notes:
*     - Any input points which are transformed by the Mapping to give
*     output coordinates containing the value AST__BAD are regarded as
*     invalid and are ignored. They will make no contribution to
*     determining the output bounds, even although the nominated
*     output coordinate might still have a valid value at such points.
c     - An error will occur if the required output bounds cannot be
c     found. Typically, this might happen if all the input points
c     which the function considers turn out to be invalid (see
c     above). The number of points considered before generating such
c     an error is quite large, so this is unlikely to occur by
c     accident unless valid points are restricted to a very small
c     subset of the input coordinate space.
f     - An error will occur if the required output bounds cannot be
f     found. Typically, this might happen if all the input points
f     which the routine considers turn out to be invalid (see
f     above). The number of points considered before generating such
f     an error is quite large, so this is unlikely to occur by
f     accident unless valid points are restricted to a very small
f     subset of the input coordinate space.
c     - The values returned via "lbnd_out", "ubnd_out", "xl" and "xu"
c     will be set to the value AST__BAD if this function should fail
c     for any reason. Their initial values on entry will not be
c     altered if the function is invoked with the AST error status
c     set.
f     - The values returned via LBND_OUT, UBND_OUT, XL and XU will be
f     set to the value AST__BAD if this routine should fail for any
f     reason. Their initial values on entry will not be altered if the
f     routine is invoked with STATUS set to an error value.
*--

*  Implementation Notes:
*     This function implements the public interface for the astMapBox
*     method. It is identical to astMapBox_ except that the nominated
*     output coordinate given in "coord_out" is decremented by one
*     before use.  This is to allow the public interface to use
*     one-based coordinate numbering (internally, zero-based
*     coordinate numbering is used).
*/

/* Check the global error status. */
   if ( !astOK ) return;

/* Invoke the protected version of this function with the "coord_out"
   value decremented. */
   astMapBox_( this, lbnd_in, ubnd_in, forward, coord_out - 1,
               lbnd_out, ubnd_out, xl, xu );
}
