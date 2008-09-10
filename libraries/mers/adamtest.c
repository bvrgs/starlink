/*
 *+
 *  Name:
 *     adamtest.c

 *  Purpose:
 *    Simple test program for ADAM

 *  Notes:
 *    Build as:
 *       link adamtest.c -L. `./err_link_adam`

*  Copyright:
*     Copyright (C) 2008 Science and Technology Facilities Council.
*     All Rights Reserved.

*  Licence:
*     This program is free software; you can redistribute it and/or
*     modify it under the terms of the GNU General Public License as
*     published by the Free Software Foundation; either version 2 of
*     the License, or (at your option) any later version.
*     
*     This program is distributed in the hope that it will be
*     useful,but WITHOUT ANY WARRANTY; without even the implied
*     warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
*     PURPOSE. See the GNU General Public License for more details.
*     
*     You should have received a copy of the GNU General Public License
*     along with this program; if not, write to the Free Software
*     Foundation, Inc., 59 Temple Place,Suite 330, Boston, MA
*     02111-1307, USA

*  Authors:
*     TIMJ: Tim Jenness (JAC, Hawaii)
*     {enter_new_authors_here}

*  History:
*     10-SEP-2008 (TIMJ):
*        First version
*     {enter_further_changes_here}

*-
 */

#include "sae_par.h"
#include "mers.h"
#include "msg_err.h"


void adamtest ( int * status ) {

  if (*status != SAI__OK) return;

  /* Make up a bad status */
  *status = MSG__SYNER;
  errRep( " ", "This is the error message ^STATUS embedded",
          status );

  errRep( "MSG1", "This text should not appear", status );

  errRep( " ", "Should be expanded: %ET as 'EXPOSURE_TIME", status);

  errRep( " ", "Object $TESTOBJ should be somewhere", status );

  errRep( " ", "Multiple %ET and ^STATUS and %TESTOBJ and %BLAH", status );

}
