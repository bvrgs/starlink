#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "asterix.h"

#include "aditypes.h"
#include "adikrnl.h"
#include "adimem.h"
#include "adilist.h"
#include "adistrng.h"
#include "adiparse.h"
#include "adicface.h"
#include "adiarray.h"
#include "adierror.h"

/*
 * Declare the static class definition
 */
_DEF_STATIC_CDEF("_Array",ary,16,ADIarray);

/*
 * Calculate number of elements in a dimensions array
 */
int ADIaryCountNelm( int ndim, int dims[] )
  {
  int           idim;
  int           nelm = 1;

  if ( ndim )
    for( idim = 0; idim<ndim; idim++ )
      nelm *= dims[idim];

  return nelm;
  }


/*
 * Convert an offset in elements from the first array element to
 * array indices assuming Fortran indexing
 */
void ADIaryIndices( int ndim, int dims[], int offset, int index[],
		    ADIstatus status )
  {
  int		loff = offset,idim,nelm;

  _chk_stat;				/* Check status on entry */

  nelm = ADIaryCountNelm( ndim, dims );	/* Count elements */

  for( idim=ndim-1; idim>=0; idim-- ) {	/* Count though dimensions */
    nelm /= dims[idim];
    index[idim] = (loff / nelm) + 1;
    loff %= nelm;
    }
  }


/*
 *  Given an array object and index array, return the base index,
 *  address of the base dimensions and the base data object
 */
void ADIaryBaseInfo( ADIarray *ary, int indices[], int orig[],
		     int **bdims, ADIobj *bdata, ADIstatus status )
  {
  int		i;			/* Loop over dimensions */
  ADIarray	*lastary = ary;
  int		offset;

  _chk_stat;				/* Check status on entry */

/* Locate base data object */
  while ( _valid_q(lastary->parent) )
    lastary = _ary_data(_han_id(lastary->parent));

/* Offset in data units from beginning of base array to slice origin */
  offset = _ID_SLOT(ary->data) - _ID_SLOT(lastary->data);

/* Find indices of slice origin point in base coordinates */
  ADIaryIndices( lastary->ndim, lastary->dims, offset, orig, status );

/* Add in user indices if supplied */
  if ( indices )
    for( i=0; i<ary->ndim; i++ )
      orig[i] += indices[i] - 1;

/* Store the address of the base array dimensions */
  *bdims = lastary->dims;

/* Store the base data object */
  *bdata = lastary->data;
  }


int ADIaryOffset( int ndim, int dims[], int index[] )
  {
  int		idim;
  int		nplane;
  int		offset = 0;

  nplane = ADIaryCountNelm( ndim-1, dims );

  for( idim=ndim-1; idim>=0; idim-- ) {
    offset += (index[idim]-1) * nplane;
    nplane /= dims[idim];
    }

  return offset;
  }


/*
 * Index an array assuming Fortran indexing
 */
ADIobj ADIaryCell( ADIarray *ary, int index[], ADIstatus status )
  {
  ADIobj	bdata;			/* Base data handle */
  int		*bdims;			/* Dimensions of base array */
  int		offset = 0;		/* Scalar offset from base array */
  int		origind[ADI__MXDIM];	/* Indices in base array */

  _chk_stat_ret(ADI__nullid);		/* Check status on entry */

/* Locate base indices, base dimensions and base data */
  ADIaryBaseInfo( ary, index, origind, &bdims, &bdata, status );

/* Calculate the offset of the used origin with respect to the dimensions */
  offset = ADIaryOffset( ary->ndim, bdims, origind );

/* Construct new block address from base data address and offset */
  return ADImemIdAddOff( bdata, offset, status );
  }


ADIobj ADIaryDel( int narg, ADIobj args[], ADIstatus status )
  {
  int           naval;
  ADIarray  	*adata = _ary_data(args[0]);

/* How many objects to free */
  naval = ADIaryCountNelm( adata->ndim, adata->dims );

/* Remove parent object */
  if ( _valid_q(adata->parent) )
    adix_erase( &adata->parent, status );

  else
/* Remove array elements */
    adix_merase( &adata->data, naval, status );

  return ADI__nullid;
  }


ADIobj ADIaryNew( int ndim, int dims[], ADIobj dataobj, ADIobj parent,
		  ADIstatus status )
  {
  ADIarray   	*adata;                 /* Pointer to array data */
  int           i;
  ADIobj    newa = ADI__nullid;

  _chk_ndim(ndim);                      /* Validate arguments */
  _chk_dims(ndim,dims);

/* Allocate new array block */
  newa = adix_cls_alloc( &KT_DEFN_ary, status );

  if ( _ok(status) )                    /* Allocated ok? */
    {
    adata = _ary_data(newa);            /* Locate data block */

    adata->ndim = ndim;                 /* Copy dimensionality */

    for( i=0; i<ndim; i++ )             /* Copy dimensions */
      adata->dims[i] = dims[i];

    adata->data = dataobj;              /* Data object id */

    adata->parent = parent;		/* Parent object */
    }

  return newa;
  }


void ADIaryAlter( ADIobj id, char *name, int nlen, int ndim,
		  int dims[], ADIstatus status )
  {
  ADIobjRequest	objreq;			/* Object specification */

  _chk_init_err; _chk_stat;

/* Find data address */
  ADIkrnlLocDat( &id, name, nlen, DA__ARRAY, &objreq, status );

/* Check not accessed, not a slice */
  if ( _ok(status) ) {
    ADIobj 	hid = _han_id(*objreq.data);	/* Locate handled object */

    if ( _ary_q(hid) ) {              	/* Array object? */
      ADIarray		*ary;
      int		idim;
      int		onelm,nnelm;

      ary = _ary_data(hid);		/* Locate array data block */

      onelm = ADIaryCountNelm(ary->ndim,	/* Find old and new # of elements */
			  ary->dims);
      nnelm = ADIaryCountNelm(ndim,dims);

      if ( nnelm != onelm ) {		/* Space requirement has changed? */
	ADIobj		newid;
	ADIclassDef	*atdef = _DTDEF(hid);

	newid = adix_cls_nalloc( atdef,	/* Allocate new data object */
	      ndim, dims, status );

/* Simple block memory copy */
	_CH_MOVE( _DTDAT(newid), _DTDAT(ary->data),
		_MAX(onelm,nnelm)*atdef->alloc.size );

	if ( onelm > nnelm ) {		/* Old objects being lost? */
	  /* Delete them! */
	  }

	ary->data = ADI__nullid;	/* Zero this so we can delete it */

	adic_erase( &hid, status );	/* Erase the old array block */

	_han_id(id) = newid;		/* Attach the new array block */
	}
      else {				/* Only dimensions have changed */
	ary->ndim = ndim;
	for( idim=0; idim<ndim; idim++ )
	  ary->dims[idim] = dims[idim];

	for( ; idim<ADI__MXDIM; idim++ )
	  ary->dims[idim] = 0;
	}
      }
    else
      adic_setecs( ADI__INVARG, "Object is not an array", status );
    }
  }


void ADIaryPrint( ADIobj stream, ADIarray *ary, ADIstatus status )
  {
  ADIinteger		pdims[ADI__MXDIM];
  ADIinteger		i,j,k,l,m,n,o;
  ADIobj		idata;
  int			ndim = ary->ndim;

  _chk_stat;

  for( i=0; i<ndim; i++ )
    pdims[i] = ary->dims[i];
  for( i=ndim; i<ADI__MXDIM; i++ )
    pdims[i] = 1;

  idata = ary->data;

  if ( ndim > 6 ) ADIstrmFprintf( stream, "[", status );
  for( o = pdims[6]; o; o-- ) {
    if ( ndim > 5 ) ADIstrmFprintf( stream, "[", status );
    for( n = pdims[5]; n; n-- ) {
      if ( ndim > 4 ) ADIstrmFprintf( stream, "[", status );
      for( m = pdims[4]; m; m-- ) {
	if ( ndim > 3 ) ADIstrmFprintf( stream, "[", status );
	for( l = pdims[3]; l; l-- ) {
	  if ( ndim > 2 ) ADIstrmFprintf( stream, "[", status );
	  for( k = pdims[2]; k; k-- ) {
	    if ( ndim > 1 ) ADIstrmFprintf( stream, "[", status );
	    for( j = pdims[1]; j; j-- ) {
	      ADIstrmFprintf( stream, "[", status );
	      for( i = pdims[0]; i; i-- ) {
		ADIstrmFprintf( stream, "%O%s", status, idata, i>1 ? ", " : "]" );
		idata = ADImemIdAddOff( idata, 1, status );
		}
	      }
	    if ( ndim > 1 ) ADIstrmFprintf( stream, "]", status );
	    }
	  if ( ndim > 2 ) ADIstrmFprintf( stream, "]", status );
	  }
	if ( ndim > 3 ) ADIstrmFprintf( stream, "]", status );
	}
      if ( ndim > 4 ) ADIstrmFprintf( stream, "]", status );
      }
    if ( ndim > 5 ) ADIstrmFprintf( stream, "]", status );
    }
  if ( ndim > 6 ) ADIstrmFprintf( stream, "]", status );
  }


void ADIaryInit( ADIstatus status )
  {
  ADIkrnlDefDestrucKint( &KT_DEFN_ary, ADIaryDel, status );
  }
