/*
*+
*  Name:
*     sc2sim_ndfwrdata

*  Purpose:
*     Generic digitise/compress and store SCUBA-2 data as NDF

*  Language:
*     Starlink ANSI C

*  Type of Module:
*     Subroutine

*  Invocation:
*     sc2sim_ndfwrdata ( const struct sc2sim_obs_struct *inx, 
*                        const struct sc2sim_sim_struct *sinx,
*                        double meanwvm, const char file_name[], 
*                        int numsamples, int nflat, const char flatname[], 
*                        const JCMTState *head, const int *dbuf, const int *dksquid,
*                        const double *fcal, const double *fpar, 
*                        const char instrume[], const char filter[], 
*                        const char dateobs[], const char obsid[], 
*                        const double *posptr, int jigsamples, 
*                        const double jigptr[][2], const int obsnum,       
*                        const int nsubscan, const char obstype[], 
*                        const char utdate[], const double azstart,
*                        const double azend, const double elstart,   
*                        const double elend, const char lststart[],  
*                        const char lstend[], const char loclcrd[],   
*                        const char scancrd[], const double totaltime, 
*                        const double exptime, const int nimage,       
*                        const double wvmstart, const double wvmend,    
*                        int *status )

*  Arguments:
*     inx = const sc2sim_obs_struct* (Given)
*        Pointer to struct with observation parameters
*     sinx = const sc2sim_sim_struct* (Given)
*        Pointer to struct with simulation parameters
*     meanwvm = double (Given)
*        225 GHz tau
*     file_name = const char[] (Given)
*        Output file name 
*     numsamples = int (Given)
*        Number of samples 
*     nflat = int (Given)
*        Number of flat coeffs per bol
*     flatname = const char[] (Given)
*        Name of flatfield algorithm 
*     head = const JCMTState* (Given)
*        Header data for each frame 
*     dbuf = const int* (Given)
*        Simulated data
*     dksquid = const int* (Given)
*        Dark SQUID time stream data 
*     fcal = const double* (Given)
*        Flatfield calibration 
*     fpar = double (Given)
*        Flat-field parameters
*     instrume = const char[] (Given)
*        Instrument name (usually SCUBA-2)
*     filter = const char[] (Given)
*        String representing filter (e.g. "850") 
*     dateobs = const char[] (Given)
*        DATE-OBS FITS string
*     obsid = const char[] (Given)
*        Observation ID string
*     posptr = const double* (Given)
*        Pointing offsets from map centre
*     jigsamples = int (Given)
*        Number of jiggle samples in DREAM pattern
*     jigptr[][2] = double (Given)
*        Array of jiggle X and Y positions
*     obsnum = const int (Given)
*        Observation number
*     nsubscan = const int (Given)
*        Sub-scan number
*     obstype[] = const char (Given)
*        Observation type, e.g. SCIENCE
*     utdate[] = const char (Given)
*        UT date in YYYYMMDD form
*     azstart = const double (Given)
*        Azimuth at start of sub-scan
*     azend = const double (Given)
*        Azimuth at end of sub-scan
*     elstart = const double (Given)
*        Elevation at start of sub-scan
*     elend = const double (Given)
*        Elevation at end of sub-scan
*     lststart[] = const char (Given)
*        LST at start of sub-scan
*     lstend[] = const char (Given)
*        LST at end of sub-scan
*     loclcrd[] = const char (Given)
*        Coordinate frame
*     scancrd[] = const char (Given)
*        SCAN coordinate frame
*     totaltime = const double (Given)
*        Total integration time
*     exptime = const double (Given)
*        Subimage exposure time
*     nimage = const int (Given)
*        Number of subimages within subscan
*     wvmstart = const double (Given)
*        225-GHz tau at beginning of subscan
*     wvmend = const double (Given)
*        225-GHz tau at end of subscan
*     status = int* (Given and Returned)
*        Pointer to global status.  

*  Description:
*     Create and map a SCUBA-2 NDF file. Scale the data to integers
*     one frame at a time and add a compressed version to the mapped
*     file.  Store the per-frame header items and the FITS
*     headers. Calculates images for DREAM and STARE modes, and
*     scanfit polynomial fits (for 1/f drift) for all modes.

*  Authors:
*     E.Chapin (UBC)
*     A.G. Gibb (UBC)
*     J. Balfour (UBC)
*     Tim Jenness (JAC, Hawaii)
*     {enter_new_authors_here}

*  History :
*     2006-03-29 (EC):
*        dsim_ndfwrdata adapted from dsim_ndfwrpong
*     2006-05-11 (AGG)
*        Added obsmode
*     2006-07-21 (JB):
*        Split from dsim.c
*     2006-07-28 (JB):
*        Changed sc2head to JCMTState
*     2006-08-08 (EC):
*        Added INSTRUME FITS keyword
*     2006-08-18 (AGG):
*        Update API to take:
*        - pointers to inx and sinx structs
*        - DREAM jiggle position parameters
*     2006-09-06 (EC):
*        INSTRUME keyword now taken as argument (to accomodate AzTEC)
*     2006-09-15 (AGG):
*        Write out name of DREAM weights file into FITS header
*     2006-09-22 (JB):
*        Replace dxml_structs with sc2sim_structs
*     2006-10-06 (AGG):
*        Add WAVELEN FITS keyword
*     2006-10-26 (JB):
*        Convert to using AstFitsChans
*     2006-12-01 (AGG):
*        Now takes dateobs string, writes TIMESYS FITS header
*     2006-12-15 (AGG):
*        Write out DUT1 FITS header
*     2006-12-19 (TIMJ):
*        sc2store_wrtstream has additional subnum argument
*     2006-12-21 (AGG):
*        Add instap & instap_x/y FITS headers
*     2007-03-20 (TIMJ):
*        - Write header units in compliance with FITS standard
*        - Use const arguments and add OBSID argument/header
*     2007-04-02 (AGG):
*        Add more FITS headers
*     2007-04-10 (AGG):
*        Calculate STARE images and polynomial fits, write to raw data
*     2007-04-26 (AGG):
*        - Shorten comment for SEQSTART/END to ensure correct formatting
*        - Add some more (admittedly blank) FITS headers
*     2007-05-04 (AGG):
*        Add INSTRUME FITS keyword
*     2007-05-17 (EC):
*        Use astFitsSetCN instead of astFitsSetS for COMMENT lines

*  Copyright:
*     Copyright (C) 2005-2007 Particle Physics and Astronomy Research
*     Council. University of British Columbia. All Rights Reserved.

*  Licence:
*     This program is free software; you can redistribute it and/or
*     modify it under the terms of the GNU General Public License as
*     published by the Free Software Foundation; either version 2 of
*     the License, or (at your option) any later version.
*
*     This program is distributed in the hope that it will be
*     useful, but WITHOUT ANY WARRANTY; without even the implied
*     warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
*     PURPOSE. See the GNU General Public License for more details.
*
*     You should have received a copy of the GNU General Public
*     License along with this program; if not, write to the Free
*     Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
*     MA 02111-1307, USA

*  Bugs:
*     {note_any_bugs_here}
*-
*/

/* Standard includes */
#include <string.h>

/* Starlink includes */
#include "ast.h"
#include "ndf.h"
#include "star/kaplibs.h"
#include "mers.h"

/* SC2SIM includes */
#include "sc2sim.h"

/* SMURF includes */
#include "smurf_par.h"
#include "libsmf/smf.h"
#include "sc2da/sc2store.h"
#include "sc2da/sc2store_par.h"
#include "sc2da/sc2ast.h"
#include "sc2da/sc2math.h"
#include "sc2da/dream_par.h"

void sc2sim_ndfwrdata
( 
const struct sc2sim_obs_struct *inx,  /* structure for values from XML (given) */
const struct sc2sim_sim_struct *sinx, /* structure for sim values from XML (given)*/
double meanwvm,          /* Mean 225 GHz tau */
const char file_name[],  /* output file name (given) */
int numsamples,          /* number of samples (given) */
int nflat,               /* number of flat coeffs per bol (given) */
const char flatname[],   /* name of flatfield algorithm (given) */
const JCMTState *head,   /* header data for each frame (given) */
const int *dbuf,         /* simulated data (given) */
const int *dksquid,      /* dark SQUID time stream data (given) */
const double *fcal,      /* flatfield calibration (given) */
const double *fpar,      /* flat-field parameters (given) */
const char instrume[],   /* String representing instrument (e.g. "SCUBA-2") (given) */
const char filter[],     /* String representing filter (e.g. "850") (given) */
const char dateobs[],    /* String representing UTC DATE-OBS */
const char obsid[],      /* unique obsid for this observation (given) */
const double *posptr,    /* Pointing offsets from map centre (given) */
int jigsamples,          /* Number of jiggle samples (given) */
const double jigptr[][2], /* Array of X, Y jiggle positions (given) */
const int obsnum,        /* Observation number (given) */
const int nsubscan,      /* Sub-scan number (given) */
const char obstype[],    /* Observation type, e.g. SCIENCE (given)*/
const char utdate[],     /* UT date in YYYYMMDD form (given) */
const double azstart,    /* Azimuth at start of sub-scan (given) */
const double azend,      /* Azimuth at end of sub-scan (given) */
const double elstart,    /* Elevation at start of sub-scan (given) */
const double elend,      /* Elevation at end of sub-scan (given) */
const char lststart[],   /* LST at start of sub-scan (given) */
const char lstend[],     /* LST at end of sub-scan (given) */
const char loclcrd[],    /* Coordinate frame (given) */
const char scancrd[],    /* SCAN coordinate frame (given) */
const double totaltime,  /* Total integration time (given) */
const double exptime,    /* Subimage exposure time (given) */
const int nimage,        /* Number of subimages within subscan (given) */
const double wvmstart,   /* 225-GHz tau at beginning of subscan (given) */
const double wvmend,     /* 225-GHz tau at end of subscan (given) */
int *status              /* Global status (given and returned) */
)

{
   /* Local variables */
   double coadd[2*DREAM__MXBOL];   /* Coadded values in output image */
   double decd;                    /* Dec of observation in degrees */
   int dims[2];                    /* Extent of output image */
   AstFitsChan *fitschan;          /* FITS headers */
   int fitsfind;
   char fitsrec[SC2STORE__MAXFITS][SZFITSCARD]; /* Store for FITS records */
   int framesize;                  /* Number of points in a single `frame' */
   int i;                          /* Loop counter */
   double instap[2];               /* Instrument aperture */
   int j;                          /* Loop counter */
   int k;                          /* Loop counter */
   int naver;                      /* Number of frames to average */
   int ndim;                       /* Dimensionality of output image */
   int nrec;                       /* number of FITS header records */
   int nsubim;                     /* Number of DREAM/STARE images */
   double map_hght;                /* Map height in arcsec */
   double map_pa;                  /* Map PA in degrees  */
   double map_wdth;                /* Map width in arcsec  */
   double map_x = 0;               /* Map X offset in arcsec */
   double map_y = 0;               /* Map Y offset in arcsec */
   int midpt;                      /* RTS index of midpoint in range contributing 
				      to output image */
   int ncoeff = 2;                 /* Number of coefficients in polynomial fit */
   double *poly;                   /* Pointer to polynomial fit solution */
   double rad;                     /* RA of observation in degrees */
   double *rdata;                  /* Pointer to flatfielded data */
   int seqend;                     /* RTS index of last frame in output image */
   int seqstart;                   /* RTS index of first frame in output image */
   JCMTState state;                /* Dummy JCMT state structure for creating WCS */
   int subnum;                     /* sub array index */
   double telpos[3];               /* Telescope position */
   AstFrameSet *wcs;               /* WCS frameset for output image */
   char weightsname[81];           /* Name of weights file for DREAM 
				      reconstruction */
   double x_max = 0;               /* X extent of pointing centre offsets */
   double x_min = 0;               /* X extent of pointing centre offsets */
   double y_max = 0;               /* Y extent of pointing centre offsets */
   double y_min = 0;               /* Y extent of pointing centre offsets */
   double zero[2*DREAM__MXBOL];    /* Bolometer zero points */

   /* Check status */
   if ( !StatusOkP(status) ) return;

   /* This calc should go in a higher level routine */
   /* Determine extent of the map from posptr + known size of the arrays */
   for( i=0; i<numsamples; i++ ) {
     /* Initialize extrema */
     if( i == 0 ) {
       x_min = posptr[0];
       x_max = posptr[0];
       y_min = posptr[1];
       y_max = posptr[1];
     }
     if( posptr[i*2] < x_min ) x_min = posptr[i*2];
     if( posptr[i*2] > x_max ) x_max = posptr[i*2];
     if( posptr[i*2+1] < y_min ) y_min = posptr[i*2+1];
     if( posptr[i*2+1] > y_max ) y_max = posptr[i*2+1];
   }
   map_wdth = (x_max - x_min) + 650.0; /* 650 arcsec for array diagonal FOV */
   map_hght = (y_max - y_min) + 650.0; /* 650 arcsec for array diagonal FOV */
   map_pa = 0; /* kludge for now since it is not specified by the user */
   map_x = (x_max + x_min)/2.;
   map_y = (y_max + y_min)/2.;
  

   /* Define the FITS headers to add to the output file */
   fitschan = astFitsChan ( NULL, NULL, "" );

   /* Telescope */
   astSetFitsCN ( fitschan, "COMMENT", "", "-- Telescope specific parameters --", 0 );
   astSetFitsS ( fitschan, "TELESCOP", "JCMT", "Name of telescope", 0 );
   astSetFitsS ( fitschan, "ORIGIN", "SMURF SCUBA-2 simulator", 
		 "Origin of file", 0 );
   astSetFitsF ( fitschan, "OBSGEO-X", -1.601185365E+06, 
		 "x,y,z triplet for JCMT", 0 );
   astSetFitsF ( fitschan, "OBSGEO-Y", -5.041977547E+06, 
		 "relative to centre of the Earth", 0 );
   astSetFitsF ( fitschan, "OBSGEO-Z", 3.554875870E+06, "[m]", 0 );
   astSetFitsF ( fitschan, "ALT-OBS", 4092, 
		 "[m] Height of observatory above sea level", 0 );
   astSetFitsF ( fitschan, "LAT-OBS", 19.8258323669, 
		 "[deg] Latitude of observatory", 0 );
   astSetFitsF ( fitschan, "LONG-OBS", 204.520278931, 
		 "[deg] East longitude of observatory", 0 );
   astSetFitsF ( fitschan, "ETAL", 1.0, "Telescope efficiency", 0 );

   /* Observation, date & pointing */
   astSetFitsCN ( fitschan, "COMMENT", "", "-- Observation & date parameters --", 0 );
   astSetFitsS ( fitschan, "OBSID", obsid, "Unique observation ID", 0 );
   astSetFitsS ( fitschan, "OBJECT", "SMURF", "Object Name", 0 );
   astSetFitsL ( fitschan, "STANDARD", 0, "True if source is a calibrator", 0 );
   astSetFitsI ( fitschan, "OBSNUM", obsnum, "Observation Number", 0 );
   astSetFitsI ( fitschan, "NSUBSCAN", nsubscan, "Sub-scan Number", 0 );
   astSetFitsL ( fitschan, "OBSEND", 0, 
		 "True if frame is last in current observation", 0 );
   astSetFitsS ( fitschan, "UTDATE", utdate, 
		 "UT date as a string in yyyymmdd format", 0 );
   astSetFitsS ( fitschan, "DATE-OBS", dateobs, 
		 "Date and time (UTC) of start of sub-scan", 0 );
   astSetFitsS ( fitschan, "DATE-END", dateobs, 
		 "Date and time (UTC) of end of sub-scan", 0 );
   astSetFitsF ( fitschan, "DUT1", inx->dut1, "[d] UT1 - UTC correction", 0 );
   astSetFitsS ( fitschan, "INSTAP", inx->instap, "Instrument aperture", 0 );
   astSetFitsF ( fitschan, "INSTAP_X", inx->instap_x, 
		 "[arcsec] X focal plane offset", 0 );
   astSetFitsF ( fitschan, "INSTAP_Y", inx->instap_y, 
		 "[arcsec] Y focal plane offset", 0 );
   astSetFitsF ( fitschan, "AMSTART", 1./cos(AST__DPIBY2-elstart), 
		 "Air mass at start", 0 );
   astSetFitsF ( fitschan, "AMEND", 1./cos(AST__DPIBY2-elend), 
		 "Air mass at end", 0 );
   astSetFitsF ( fitschan, "AZSTART", AST__DR2D*azstart, 
		 "[deg] Azimuth at sub-scan start", 0 );
   astSetFitsF ( fitschan, "AZEND", AST__DR2D*azend, 
		 "[deg] Azimuth at sub-scan end", 0 );
   astSetFitsF ( fitschan, "ELSTART", AST__DR2D*elstart, 
		 "[deg] Elevation at sub-scan start", 0 );
   astSetFitsF ( fitschan, "ELEND", AST__DR2D*elend, 
		 "[deg] Elevation at sub-scan end", 0 );
   astSetFitsS ( fitschan, "LSTSTART", lststart, "LST at start of sub-scan", 0 );
   astSetFitsS ( fitschan, "LSTEND", lstend, "LST at end of sub-scan", 0 );

   /* Environment */
   astSetFitsCN ( fitschan, "COMMENT", "", "-- Environment parameters --", 0 );
   astSetFitsF ( fitschan, "ATSTART", sinx->atstart, 
                 "[deg C] Ambient temperature at start", 0 );
   astSetFitsF ( fitschan, "ATEND", sinx->atend, 
		 "[deg C] Ambient temperature at end", 0 );
   astSetFitsF ( fitschan, "WVMTAUST", wvmstart, "WVM tau at start", 0 );
   astSetFitsF ( fitschan, "WVMTAUEN", wvmend, "WVM tau at end", 0 );
   astSetFitsF ( fitschan, "SEEINGST", 1.0, "Seeing at start", 0 );
   astSetFitsF ( fitschan, "SEEINGEN", 1.0, "Seeing at end", 0 );

   /* OMP & ORAC-DR */
   astSetFitsCN ( fitschan, "COMMENT", "", "-- OMP & ORAC-DR parameters --", 0 );
   astSetFitsS ( fitschan, "PROJECT", "M08AC00", 
		 "The proposal ID for the PROJECT", 0 );
   astSetFitsS ( fitschan, "RECIPE", "", "The ORAC-DR recipe", 0 );
   astSetFitsS ( fitschan, "DRGROUP", "", 
		 "Name of group to combine current observation with", 0 );
   astSetFitsS ( fitschan, "MSBID", "", "ID of min schedulable block", 0 );
   astSetFitsS ( fitschan, "MSBTID", "", "Translation ID of MSB", 0 );
   astSetFitsS ( fitschan, "SURVEY", "", "Survey Name", 0 );

   /* SCUBA-2 */
   astSetFitsCN ( fitschan, "COMMENT", "", "-- SCUBA-2 specific parameters --", 0 );
   astSetFitsS ( fitschan, "INSTRUME", "SCUBA-2", "Instrument name - SCUBA-2", 0 );
   astSetFitsS ( fitschan, "SUBARRAY", sinx->subname, "subarray name", 0 );
   astSetFitsS ( fitschan, "SHUTTER", "", "Shutter position for dark frames", 0 );
   astSetFitsS ( fitschan, "FILTER", filter, "filter used", 0 );
   astSetFitsF ( fitschan, "WAVELEN", inx->lambda, "[m] Wavelength", 0 );

   /* Switching and mapping */
   astSetFitsCN ( fitschan, "COMMENT", "", "-- Mapping parameters --", 0 );
   if ( strncmp( inx->obsmode, "DREAM", 5) == 0 || 
	strncmp( inx->obsmode, "STARE", 5) == 0 ) {
     astSetFitsS ( fitschan, "SAM_MODE", inx->obsmode, 
		   "Sample mode: STARE, DREAM or SCAN", 0 );
   } else {
     astSetFitsS ( fitschan, "SAM_MODE", "SCAN", 
		   "Sample mode: STARE, DREAM or SCAN", 0 );
   }
   astSetFitsS ( fitschan, "SW_MODE", "NONE", 
		 "Switch mode: CHOP, PSSW, FREQ, or NONE", 0 );
   astSetFitsS ( fitschan, "OBS_TYPE", obstype, 
		 "Observation type -  Science, Pointing or Focus", 0 );

   if ( strncmp( inx->obsmode, "DREAM", 5) == 0 ) {
     astSetFitsI ( fitschan, "JIGL_CNT", inx->nvert,
		   "Number of points in DREAM pattern", 0 );
     astSetFitsS ( fitschan, "JIGL_NAM", "",
		   "Name containing DREAM jiggle offsets", 0 );
     astSetFitsF ( fitschan, "JIGL_PA", 0,
		   "Number of points in DREAM pattern", 0 );
     astSetFitsS ( fitschan, "JIGL_CRD", "FPLANE",
		   "Coord frame of jiggle pattern", 0 );
     astSetFitsF ( fitschan, "JIG_SCAL", inx->jig_step_x, 
		   "[arcsec] SMU jiggle pattern scale factor", 0 );
     /* Construct weights name from subarray */
     strncat( weightsname, "dreamweights_", 13);
     strncat( weightsname, sinx->subname, 3);
     strncat( weightsname, ".sdf", 4);
     astSetFitsS ( fitschan, "DRMWGHTS", weightsname, 
                   "Name of DREAM weights file", 0 );
   } else {
     astSetFitsI ( fitschan, "JIGL_CNT", 0,
		   "Number of points in DREAM pattern", 0 );
     astSetFitsS ( fitschan, "JIGL_NAM", "",
		   "Name containing DREAM jiggle offsets", 0 );
     astSetFitsF ( fitschan, "JIGL_PA", 0,
		   "Number of points in DREAM pattern", 0 );
     astSetFitsS ( fitschan, "JIGL_CRD", "",
		   "Coord frame of jiggle pattern", 0 );
     astSetFitsF ( fitschan, "JIG_SCAL", 0.0, 
		   "[arcsec] SMU jiggle pattern scale factor", 0 );
     astSetFitsS ( fitschan, "DRMWGHTS", "", 
                   "Name of DREAM weights file", 0 );
   }
   astSetFitsF ( fitschan, "MAP_HGHT", map_hght, "[arcsec] Map height", 0 );
   astSetFitsF ( fitschan, "MAP_PA", map_pa, "[deg] Map PA", 0 );
   astSetFitsF ( fitschan, "MAP_WDTH", map_wdth, "[arcsec] Map width", 0 );
   astSetFitsS ( fitschan, "LOCL_CRD", loclcrd, 
		 "Local offset coordinate system", 0 );
   astSetFitsF ( fitschan, "MAP_X", map_x, "[arcsec] Map X offset", 0 );
   astSetFitsF ( fitschan, "MAP_Y", map_y, "[arcsec] Map Y offset", 0 );
   /* Fill in for obsmode = PONG only */
   if ( strncmp( inx->obsmode, "PONG", 4) == 0 ) {
     astSetFitsS ( fitschan, "SCAN_CRD", scancrd, "Scan coordinate system", 0 );
     astSetFitsF ( fitschan, "SCAN_VEL", inx->vmax, 
		   "[arcsec/s] Requested scanning rate", 0 );
     astSetFitsF ( fitschan, "SCAN_DY", inx->spacing, 
		   "[arcsec] Sample spacing perpendicular to scan", 0 );
     astSetFitsF ( fitschan, "SCAN_PA", inx->scan_angle, 
		   "[deg] Scan PA relative to N in SCAN_CRD system", 0 );
     astSetFitsS ( fitschan, "SCAN_PAT", "PONG", "Scanning pattern", 0 );
   } else {
     astSetFitsS ( fitschan, "SCAN_CRD", "", "Scan coordinate system", 0 );
     astSetFitsF ( fitschan, "SCAN_VEL", 0, 
		   "[arcsec/s] Requested scanning rate", 0 );
     astSetFitsF ( fitschan, "SCAN_DY", 0, 
		   "[arcsec] Sample spacing perpendicular to scan", 0 );
     astSetFitsF ( fitschan, "SCAN_PA", 0, 
		   "[deg] Scan PA relative to N in SCAN_CRD system", 0 );
     astSetFitsS ( fitschan, "SCAN_PAT", "", "Scanning pattern", 0 );
   }

   /* JOS parameters */
   astSetFitsCN ( fitschan, "COMMENT", "", "-- JOS parameters --", 0 );
   astSetFitsF ( fitschan, "STEPTIME", inx->steptime, 
		 "[s] Time interval between samples", 0 );

   /* Integration time */
   astSetFitsCN ( fitschan, "COMMENT", "", 
		 "-- Integration time-related parameters --", 0 );
   astSetFitsF ( fitschan, "INT_TIME", totaltime, 
		 "[s] Time spent integrating on source", 0 );
   /* Only write exp_time for DREAM and STARE*/
   if ( strncmp( inx->obsmode, "DREAM", 5) == 0 || 
	strncmp( inx->obsmode, "STARE", 5) == 0) {
     astSetFitsF ( fitschan, "EXP_TIME", exptime, 
		   "[s] Mean integration time per output pixel", 0 );
     astSetFitsI ( fitschan, "N_SUB", nimage, 
		   "Number of sub-scans written to file", 0 );
   } else {
     astSetFitsI ( fitschan, "N_SUB", 0, 
		   "Number of sub-scans written to file", 0 );
   }

   /* SMU specific */
   astSetFitsCN ( fitschan, "COMMENT", "", "-- SMU-specific parameters --", 0 );
   astSetFitsF ( fitschan, "ALIGN_DX", 0.0, "SMU tables X axis alignment offset", 0 );
   astSetFitsF ( fitschan, "ALIGN_DY", 0.0, "SMU tables Y axis alignment offset", 0 );
   astSetFitsF ( fitschan, "FOCUS_DZ", 0.0, "SMU tables Z axis focus offset", 0 );
   astSetFitsF ( fitschan, "DAZ", 0.0, "SMU azimuth pointing offset", 0 );
   astSetFitsF ( fitschan, "DEL", 0.0, "SMU elevation pointing offset", 0 );
   astSetFitsF ( fitschan, "UAZ", 0.0, "User azimuth pointing offset", 0 );
   astSetFitsF ( fitschan, "UEL", 0.0, "User elevation pointing offset", 0 );

   /* Misc */
   astSetFitsCN ( fitschan, "COMMENT", "", "-- Miscellaneous --", 0 );
   astSetFitsS ( fitschan, "OCSCFG", "config.xml", 
		 "Name of OCS Configuration XML file defining the observation", 0 );
   astSetFitsL ( fitschan, "SIMULATE", 1, "True if data produced by simulator", 0 );
   astSetFitsL ( fitschan, "SIM_SMU", 1, "True if SMU data are simulated", 0 );
   astSetFitsL ( fitschan, "SIM_RTS", 1, "True if RTS data are simulated", 0 );
   astSetFitsL ( fitschan, "SIM_TCS", 1, "True if TCS data are simulated", 0 );
   astSetFitsS ( fitschan, "STATUS", "NORMAL", 
		 "Status at obs. end - NORMAL or ABORT", 0 );

   /* Others... */
   astSetFitsL ( fitschan, "POL_CONN", 0, "True if polarimeter is in the beam", 0 );
   astSetFitsL ( fitschan, "FTS_CONN", 0, "True if FTS is used", 0 );

   astSetFitsF ( fitschan, "MEANWVM", meanwvm, 
		 "Mean zenith tau at 225 GHz from WVM", 0 );

   /* Convert the AstFitsChan data to a char array */
   smf_fits_export2DA ( fitschan, &nrec, fitsrec, status );

   /* Calculate the sub array index */
   sc2ast_name2num( sinx->subname, &subnum, status );

   /* Store the timestream data */
   sc2store_wrtstream ( file_name, subnum, nrec, fitsrec, inx->nbolx, 
                        inx->nboly, numsamples, nflat, flatname, head, 
                        dbuf, dksquid, fcal, fpar, inx->obsmode, 
                        inx->jig_vert, inx->nvert, jigptr, jigsamples, 
                        status );

   /* Create SCU2RED extension for storing polynomial fits and
      reconstructed images */
   sc2store_creimages ( status );

   /* Number of points in 1 frame - placeholder to remind us that it
      may be different for DREAM */
   if ( strncmp( inx->obsmode, "DREAM", 5) == 0 ) {
     framesize = inx->nbolx * inx->nboly;
   } else {
     framesize = inx->nbolx * inx->nboly;
   }

   /* Now we need to play with flat-fielded data */
   rdata = smf_malloc( framesize*numsamples, sizeof(double), 1, status );
   /* Apply flatfield to timestream */
   for ( j=0; j<framesize*numsamples; j++ ) {
     rdata[j] = (double)dbuf[j];
   }
   sc2math_flatten ( framesize, numsamples, flatname, nflat, fcal, fpar,
		     rdata, status );

   /* For DREAM/STARE data, calculate .In images and write to the
      output file. The default is to average every second. KLUDGE:
      ONLY STARE CURRENTLY SUPPORTED */
   if ( strncmp( inx->obsmode, "STARE", 5) == 0 ) {
     /* Calculate number of samples to average */
     naver = (int)(1./inx->steptime);
     /* And then the number of images to create */
     nsubim = numsamples / naver;

     /* Set jig/chop entries to zero for non-DREAM data */
     state.smu_az_jig_x = 0.0;
     state.smu_az_jig_y = 0.0;
     state.smu_az_chop_x = 0.0;
     state.smu_az_chop_y = 0.0;

     smf_calc_telpos( NULL, "JCMT", telpos, status );
     if ( strncmp( inx->instap, " ", 1 ) != 0 ) {
       sc2sim_instap_calc( inx, instap, status );
     } else {
       instap[0] = DAS2R * inx->instap_x;
       instap[1] = DAS2R * inx->instap_y;
     }

     /* Loop over number of images */
     for ( k=0; k<nsubim; k++ ) {
       /* Initialize sums to zero */
       for ( i=0; i<framesize; i++ ) {
	 coadd[i] = 0.0;
	 zero[i] = 0.0;
       }
       /* Begin and end sequence number indices. Note the FITS headers
	  SEQSTART/SEQEND are incremented by 1 from these values. */
       seqstart = k*naver;
       seqend = seqstart + naver - 1;

       /* Create average image */
       for ( j=seqstart; j<=seqend; j++ ) {
	 /* coadd frame */
	 for ( i=0; i<framesize; i++ ){
	   coadd[i] += rdata[framesize*j+i];
	 }
       }
       /* Average the coadd frame */
       for ( i=0; i<framesize; i++ ) {
	 coadd[i] /= (double)naver;
       }

       /* Calculate the index nearest the middle of the block of
	  averaged frames for constructing WCS info */
       midpt = (seqstart + seqend) / 2;

       state.tcs_az_ac1 = head[midpt].tcs_az_ac1;
       state.tcs_az_ac2 = head[midpt].tcs_az_ac2;
       state.rts_end = head[midpt].rts_end;

       /* Set dimensions of output image */
       ndim = 2;
       dims[0] = inx->nbolx;
       dims[1] = inx->nboly;

       /* Construct WCS FrameSet */
       sc2ast_createwcs( subnum, &state, instap, telpos, &wcs, status );
       /* This should be user defined... */
       astSetC( wcs, "SYSTEM", "ICRS" );

       /* Increment seqstart/end for FITS header */
       seqstart++;
       seqend++;

       /* Construct additional FITS headers to add to the existing
	  FitsChan */
       astClear( fitschan, "Card");
       /* First find and delete old entries */
       fitsfind = astFindFits( fitschan, "SEQSTART", NULL, 0);
       astDelFits( fitschan );
       fitsfind = astFindFits( fitschan, "SEQEND", NULL, 0);
       astDelFits( fitschan );
       /* Add new entries */
       astSetFitsI( fitschan, "SEQSTART", seqstart, 
		    "RTS index number of first frame in image", 0);
       astSetFitsI( fitschan, "SEQEND", seqend, 
		    "RTS index number of last frame in image", 0);

       /* Convert the AstFitsChan data to a char array */
       smf_fits_export2DA ( fitschan, &nrec, fitsrec, status );

       /* Store image */
       sc2store_putimage ( k, wcs, ndim, dims, seqstart, seqend, inx->nbolx, 
			   inx->nboly, coadd, zero, fitsrec, nrec, status );
     }
   }

   /* Calculate polynomial fits and write out SCANFIT extension */
   poly = smf_malloc( framesize*ncoeff, sizeof( double ), 0, status );
   sc2math_fitsky ( 0, framesize, numsamples, ncoeff, rdata, poly, status );
   sc2store_putscanfit ( inx->nbolx, inx->nboly, ncoeff, poly, status );

   /* Free memory allocated for pointers */
   smf_free( poly, status );
   smf_free( rdata, status );
 
   /* Close the file */
   sc2store_free ( status );
}
