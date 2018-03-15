/*
*+
*  Name:
*     smf_import_noi

*  Purpose:
*     Import noise values for the NOI model.

*  Language:
*     Starlink ANSI C

*  Type of Module:
*     C function

*  Invocation:
*     int smf_import_noi( const char *name, smfDIMMHead *head,
*                         AstKeyMap *keymap, smfData *qua,
*                         double *dataptr, int *status )

*  Arguments:
*     name = const char * (Given)
*        The name of the container file without a suffix.
*     head = smfDIMMHead * (Given)
*        Defines the shape and size of the NOI model.
*     *keymap = AstKeyMap * (Given)
*        The config parameters for makemap.
*     qua = smfData * (Given and Returned)
*        The associated quality data - bad or non-positive noise values
*        are flagged with SMF__Q_NOISE. May be NULL.
*     dataptr = double * (Returned)
*        The array in which to return the noise values.
*     status = int* (Given and Returned)
*        Pointer to global status.

*   Returned Value:
*     Non-zero if values were imported successfully. Zero otherwise.

*  Description:
*     This function checks the NOI.IMPORT config parameter. If it set to
*     a non-zero value, it imports the Data array from an NDF such as
*     generated by a previous run of makemap with EXPORTNDF=NOI and
*     NOI.EXPORT=1, expands it to the size specified by head, and stores
*     it in the supplied dataptr array.

*  Authors:
*     David S Berry (JAC, Hawaii)
*     {enter_new_authors_here}

*  History:
*     24-SEP-2013 (DSB):
*        Original version.
*     24-JAN-2014 (DSB):
*        - Re-written to follow the changes to smf_export_noi (the time axis
*        is now the first axis in the NDF, not the third axis).
*        - Remove argument "noi_boxsize".
*     15-MAY-2014 (DSB):
*        Flag bad variances in the quality array.
*     2018-03-15 (DSB):
*        Read existing model data from NDFs stored in the directory specified 
*        by the config parameter "dumpdir", rather than from the current 
*        directory.
*     {enter_further_changes_here}

*  Copyright:
*     Copyright (C) 2013 Science & Technology Facilities Council.
*     All Rights Reserved.

*  Licence:
*     This program is free software; you can redistribute it and/or
*     modify it under the terms of the GNU General Public License as
*     published by the Free Software Foundation; either version 3 of
*     the License, or (at your option) any later version.
*
*     This program is distributed in the hope that it will be
*     useful, but WITHOUT ANY WARRANTY; without even the implied
*     warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
*     PURPOSE. See the GNU General Public License for more details.
*
*     You should have received a copy of the GNU General Public
*     License along with this program; if not, write to the Free
*     Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
*     MA 02110-1301, USA

*  Bugs:
*     {note_any_bugs_here}
*-
*/

/* Starlink includes */
#include "sae_par.h"
#include "mers.h"
#include "ndf.h"
#include "ast.h"

/* SMURF includes */
#include "libsmf/smf.h"

int smf_import_noi( const char *name, smfDIMMHead *head, AstKeyMap *keymap,
                    smfData *qua, double *dataptr, int *status ){

/* Local Variables */
   AstKeyMap *kmap = NULL;
   char *ename = NULL;
   const char *bn = NULL;
   const char *tempstr = NULL;
   dim_t ibolo;
   dim_t nbolo;
   dim_t ncols;
   dim_t nrows;
   dim_t ntslice;
   double *dp;
   double *ip;
   double *pd;
   double noival;
   int dims[ 3 ];
   int el;
   int ibox;
   int import;
   int indf;
   int itime;
   int itime_hi;
   int nc;
   int ndim;
   int repeat;
   int result ;
   size_t bstride;
   size_t tstride;

/* Initialise. */
   result = 0;

/* Check inherited status. */
   if( *status != SAI__OK ) return result;

/* Get a keymap holding the NOI model parameters. */
   astMapGet0A( keymap, "NOI", &kmap );

/* Do nothing more if the IMPORT param is not non-zero. */
   import = 0;
   astMapGet0I( kmap, "IMPORT", &import );
   if( import ){

/* Get the dimensions and strides for the NOI model. */
      smf_get_dims( &((*head).data), &nrows, &ncols, &nbolo, &ntslice, NULL,
                    &bstride, &tstride, status );

/* Initialise the file path to be the path to the directory in which exported
   models etc are placed. Ensure its end with "/"  */
      tempstr = NULL;
      astMapGet0C( keymap, "DUMPDIR", &tempstr );
      if( tempstr ) {
         nc = strlen( tempstr );
         ename = astStore( NULL, tempstr, nc + 2 );
         if( ename[nc-1] != '/' ) {
            strcpy( ename + nc, "/" );
            nc++;
         }
      } else{
         nc = 0;
      }

/* Find the start of the basename of the contained file, then append it
   to the path. */
      bn = strrchr( name, '/' );
      if( bn ) {
         bn++;
      } else {
         bn = name;
      }
      ename = astAppendString( ename, &nc, bn );

/* Terminate the banename after "_con". */
      nc = strstr( ename, "_con" ) - ename + 4;
      ename[ nc ] = 0;

/* Append "_noi". */
      ename = astAppendString( ename, &nc, "_noi" );

/* Attempt to open the NDF. */
      ndfFind( NULL, ename, &indf, status );

/* Get the dimensions of the NDF. Abort if they are incorrect. */
      ndfDim( indf, 3, dims, &ndim, status );
      if( ndim != 3 && *status == SAI__OK ) {
         *status = SAI__ERROR;
         errRepf( "", "Illegal number of dimensions (%d) in '%s' - "
                  "must be 3.", status, ndim, ename );
      }

      if( ( dims[ 1 ] != (int) ncols || dims[ 2 ] != (int) nrows ) && *status == SAI__OK ) {
         *status = SAI__ERROR;
         errRepf( "", "Illegal dimensions (%d,%d) for axes 2 and 3 in "
                  "'%s' - must be (32,40).", status, dims[1], dims[2],
                  ename );
      }

      if( ntslice == 1 && dims[ 0 ] > 1 && *status == SAI__OK ) {
         *status = SAI__ERROR;
         errRepf( "", "Illegal dimension (%d) for axes 1 in "
                  "'%s' - must be 1.", status, dims[0], ename );
      }

/* Map the Data component of the NDF. */
      ndfMap( indf, "Data", "_DOUBLE", "READ", (void **) &ip, &el, status );

/* Get the number of times to repeat each noise value in the NDF. This is
   stored in the SMURF extension of the supplied NDF. */
      ndfXgt0i( indf, "SMURF", "NOI_BOXSIZE", &repeat, status );

/* Check we can use the pointers safely. */
      if( *status == SAI__OK ) {

/* If the NOI model contains only a single value for each bolometer, we
   copy one slice from the NDF. Time or bolo ordering makes no difference
   in this case. Return boxsize as zero to indicate that a single box is
   used for all time slaices. */
         if( ntslice == 1 ) {
            memcpy( dataptr, ip, nbolo*sizeof( *dataptr ) );

/* If the NOI model contains bolometer values for every time slice, we
   may need to expand and re-order the data. */
         } else {

/* Pointer to the next input NDF value. */
            pd = ip;

/* Loop round each bolometer. */
            for( ibolo = 0; ibolo < nbolo; ibolo++ ) {

/* Pointer to the first output NOI value for the current bolometer. */
               dp = dataptr + ibolo*bstride;

/* Initialise the index of the next time slice to write. */
               itime = 0;

/* Loop round each box of time slices. */
               for( ibox = 0; ibox < dims[ 0 ]; ibox++ ) {

/* Set the index of the first time slice beywond the current box. The last
   box picks up any left over boxes. */
                  if( ibox < dims[ 0 ] - 1 ) {
                     itime_hi = itime + repeat;
                  } else {
                     itime_hi = ntslice;
                  }

/* Get the noise value from the NDF, and convert bad values to zero. */
                  noival = *(pd++);
                  if( noival == VAL__BADD ) noival = 0.0;

/* Duplicate this value for the size of the box. */
                  for( ; itime < itime_hi; itime++ ) {
                     *dp = noival;
                     dp += tstride;
                  }
               }
            }
         }

/* Indicate we have succesfully imported some NOI values, and ensure the
   first value is not 1.0 (this is used as flag to indicate to later
   functions that the NOI values have been calculated). */
         if( *status == SAI__OK ) {
            result = 1;
            if( *dataptr == 1.0 ) *dataptr = 0.0;
         }
      }

/* Close the NDF. */
      ndfAnnul( &indf, status );

/* If a quality array was supplied, flag any samples that have unusable
   noise values (i.e. bad or non-positive). */
      if( qua ) {
         size_t qbstride;
         size_t qtstride;
         dim_t qntslice;
         smf_qual_t *pq;

/* Get the dimensions and strides for the Quality array. */
         smf_get_dims( qua, NULL, NULL, NULL, &qntslice, NULL,
                       &qbstride, &qtstride, status );

/* Loop round each bolometer. */
         for( ibolo = 0; ibolo < nbolo; ibolo++ ) {

/* Pointers to the first quality and noise values for this bolometer. */
            pq = (smf_qual_t *) qua->pntr[0] + ibolo*qbstride;
            pd = dataptr + ibolo*bstride;

/* Ignore entirely bad bolometers. */
            if( !( *pq & SMF__Q_BADB ) ) {

/* Loop round each time slice. */
               for( itime = 0; itime < qntslice; itime++ ) {

/* If the noise value is bad or non-positive, flag the sample with
   the SMF__Q_NOISE flag. */
                  if( *pd == VAL__BADD || *pd <= 0.0 ) *pq |= SMF__Q_NOISE;

/* Move the quality pointer on to the next time slice. */
                  pq += qtstride;

/* The noise pointer only needs to be moved on if there is more than one
   noise value per bolometer (in which case there will be the same number
   of noise values as there are quality values). */
                  if( ntslice > 1 ) pd += tstride;
               }
            }
         }
      }

/* Add a context message if anything went wrong. */
      if( *status != SAI__OK ) {
         errRepf( "", "Failed to import NOI values from NDF specified "
                  "by parameter NOI.IMPORT (%s).", status, ename );
      } else {
         msgOutiff( MSG__VERB, "", "Imported NOI values from '%s'.", status,
                    ename );
      }
   }

/* Free resources. */
   ename = astFree( ename );
   kmap = astAnnul( kmap );

/* Return the result. */
   return result;
}

