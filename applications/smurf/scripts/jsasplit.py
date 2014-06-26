#!/usr/bin/env python

'''
*+
*  Name:
*     JSASPLIT

*  Purpose:
*     Split a single NDF up into a set of JSA tiles.

*  Language:
*     python (2.7 or 3.*)

*  Description:
*     This script re-projects a supplied NDF onto the JSA all-sky pixel
*     grid and then splits the resulting NDF into more JSA tiles.

*  Usage:
*     jsasplit in out [trim] [retain] [msg_filter] [ilevel] [glevel] [logfile]

*  Parameters:
*     GLEVEL = LITERAL (Read)
*        Controls the level of information to write to a text log file.
*        Allowed values are as for "ILEVEL". The log file to create is
*        specified via parameter "LOGFILE". ["ATASK"]
*     ILEVEL = LITERAL (Read)
*        Controls the level of information displayed on the screen by the
*        script. It can take any of the following values (note, these values
*        are purposefully different to the SUN/104 values to avoid confusion
*        in their effects):
*
*        - "NONE": No screen output is created
*
*        - "CRITICAL": Only critical messages are displayed such as warnings.
*
*        - "PROGRESS": Extra messages indicating script progress are also
*        displayed.
*
*        - "ATASK": Extra messages are also displayed describing each atask
*        invocation. Lines starting with ">>>" indicate the command name
*        and parameter values, and subsequent lines hold the screen output
*        generated by the command.
*
*        - "DEBUG": Extra messages are also displayed containing unspecified
*        debugging information. In addition scatter plots showing how each Q
*        and U image compares to the mean Q and U image are displayed at this
*        ILEVEL.
*
*        In adition, the glevel value can be changed by assigning a new
*        integer value (one of starutil.NONE, starutil.CRITICAL,
*        starutil.PROGRESS, starutil.ATASK or starutil.DEBUG) to the module
*        variable starutil.glevel. ["PROGRESS"]
*     IN = NDF (Read)
*        The input NDF.
*     INSTRUMENT = LITERAL (Read)
*        Selects the tiling scheme to be used. The following instrument
*        names are recognised (unambiguous abbreviations may be supplied):
*        "SCUBA-2(450)", "SCUBA-2(850)", "ACSIS", "DAS". If the input NDF
*        contains JCMT data, the default value for this parameter is
*        determined from the FITS headers in the input NDF. Otherwise,
*        there is no default and an explicit value must be supplied. []
*     LOGFILE = LITERAL (Read)
*        The name of the log file to create if GLEVEL is not NONE. The
*        default is "<command>.log", where <command> is the name of the
*        executing script (minus any trailing ".py" suffix), and will be
*        created in the current directory. Any file with the same name is
*        over-written. []
*     MSG_FILTER = LITERAL (Read)
*        Controls the default level of information reported by Starlink
*        atasks invoked within the executing script. This default can be
*        over-ridden by including a value for the msg_filter parameter
*        within the command string passed to the "invoke" function. The
*        accepted values are the list defined in SUN/104 ("None", "Quiet",
*        "Normal", "Verbose", etc). ["Normal"]
*     OUT = LITERAL (Read)
*        The basename for the output NDFs. Each output NDF will have a
*        name formed by appending the integer tile index to the supplied
*        basename, preceeded by and underscore. A null (!) value causes
*        the name of the input NDF to be used.
*     RETAIN = _LOGICAL (Read)
*        Should the temporary directory containing the intermediate files
*        created by this script be retained? If not, it will be deleted
*        before the script exits. If retained, a message will be
*        displayed at the end specifying the path to the directory. [FALSE]
*     TRIM = _INTEGER (Read)
*        A zero or negative value results in each output NDF covering the
*        full area of the corresponding JSAtile. A value of one results in
*        each output NDF being cropped to the bounds of the supplied NDF. A
*        value of two or more results in each output NDF being cropped to
*        remove any blank borders. [2]

*  Copyright:
*     Copyright (C) 2014 Science & Technology Facilities Council.
*     All Rights Reserved.

*  Licence:
*     This program is free software; you can redistribute it and/or
*     modify it under the terms of the GNU General Public License as
*     published by the Free Software Foundation; either Version 2 of
*     the License, or (at your option) any later version.
*
*     This program is distributed in the hope that it will be
*     useful, but WITHOUT ANY WARRANTY; without even the implied
*     warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
*     PURPOSE. See the GNU General Public License for more details.
*
*     You should have received a copy of the GNU General Public License
*     along with this program; if not, write to the Free Software
*     Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
*     02110-1301, USA.

*  Authors:
*     DSB: David S. Berry (JAC, Hawaii)
*     {enter_new_authors_here}

*  History:
*     17-JAN-2014 (DSB):
*        Original version.
*     30-JAN-2014 (DSB):
*        Changed TRIM to allow output NDFs to be trimmed of any bad
*        borders.
*     26-JUN-2014 (DSB):
*        Support input NDFs that straddle a discontinuity in the HPX
*        projection.
*-
'''

import math
import starutil
from starutil import invoke
from starutil import NDG
from starutil import Parameter
from starutil import ParSys
from starutil import msg_out

#  Transitional latitude (in deg.s) between polar and equatorial regions
#  of HPX projection.
HPX_TRANS = 41.8103

#  Assume for the moment that we will not be retaining temporary files.
retain = 0

#  Do not create a log file by default. Setting parameter GLEVEL=ATASK
#  will cause a logfile to be produced.
starutil.glevel = starutil.NONE

#  A function to clean up before exiting. Delete all temporary NDFs etc,
#  unless the script's RETAIN parameter indicates that they are to be
#  retained. Also delete the script's temporary ADAM directory.
def cleanup():
   global retain
   ParSys.cleanup()
   if retain:
      msg_out( "Retaining temporary files in {0}".format(NDG.tempdir))
   else:
      NDG.cleanup()


#  Catch any exception so that we can always clean up, even if control-C
#  is pressed.
try:

#  Declare the script parameters. Their positions in this list define
#  their expected position on the script command line. They can also be
#  specified by keyword on the command line. No validation of default
#  values or values supplied on the command line is performed until the
#  parameter value is first accessed within the script, at which time the
#  user is prompted for a value if necessary. The parameters "MSG_FILTER",
#  "ILEVEL", "GLEVEL" and "LOGFILE" are added automatically by the ParSys
#  constructor.
   params = []

   params.append(starutil.ParNDG("IN", "The input NDF", default=None,
                                 minsize=0, maxsize=1 ))

   params.append(starutil.Par0S("OUT", "The basename for the output NDFs",
                                 None ))

   params.append(starutil.ParChoice("INSTRUMENT",
                                    ["SCUBA-2(450)", "SCUBA-2(850)", "ACSIS",
                                    "DAS"],
                                    "The JCMT instrument", "SCUBA-2(850)"))

   params.append(starutil.Par0I("TRIM", "How to trim the output NDFs", 2,
                                 noprompt=True))

   params.append(starutil.Par0L("RETAIN", "Retain temporary files?", False,
                                 noprompt=True))

#  Initialise the parameters to hold any values supplied on the command
#  line.
   parsys = ParSys( params )

#  Get the input NDF.
   inndf = parsys["IN"].value

#  Get the name of the output NDF. If not supplied, use the name of hte
#  input NDF.
   outndf = parsys["OUT"].value
   if outndf == None:
      outndf = inndf

#  See how the output NDFs are to be trimmed.
   trim = parsys["TRIM"].value

#  See if temp files are to be retained.
   retain = parsys["RETAIN"].value

#  See if the supplied NDF holds data from a JCMT instrument by looking at the
#  "INSTRUME", "BACKEND" and "FILTER" FITS headers.
   instrument = None
   cval = starutil.get_fits_header( inndf, "INSTRUME" )
   if cval == "SCUBA-2":
      cval = starutil.get_fits_header( inndf, "FILTER" )

      if cval == "450":
         instrument = "SCUBA-2(450)"

      elif cval == "850":
         instrument = "SCUBA-2(850)"

   else:
      cval = starutil.get_fits_header( inndf, "BACKEND" )

      if cval == "ACSIS":
         instrument = "ACSIS"

      elif cval == "DAS":
         instrument = "DAS"

#  If so, set the default for the INSTRUMENT parameter and prevent the
#  user being prompted for a value.
   if instrument != None:
      parsys["INSTRUMENT"].default = instrument
      parsys["INSTRUMENT"].noprompt = True

#  Get the chosen instrument.
   instrument = parsys["INSTRUMENT"].value
   instrument = starutil.shell_quote( instrument )

#  Get a list of the tiles that overlap the supplied NDF.
   invoke( "$SMURF_DIR/jsatilelist in={0} instrument={1} quiet".format(inndf,instrument) )
   tiles = starutil.get_task_par( "TILES", "jsatilelist" )

#  JSADICER requires the input array to be gridded on the JSA all-sky
#  pixel grid. This is normally an HPX projection, but if the supplied
#  NDF straddles a discontinuity in the HPX projection then we need to
#  use an XPH (polar HEALPix) projection instead. So now we see if this
#  is the case. Initially, assume that we can use a normal HPX projection.
   usexph = 0

#  Now loop round all the tiles touched by the supplied NDF.
   igore_first = -1
   for tile in tiles:

#  Get the RA and Dec at the centre of the tile, and convert to degrees.
      invoke( "$SMURF_DIR/jsatileinfo itile={0} instrument={1} quiet".format(tile,instrument) )
      racen = math.degrees(float(starutil.get_task_par( "RACEN", "jsatileinfo" )))
      deccen = math.degrees(float(starutil.get_task_par( "DECCEN", "jsatileinfo" )))

#  Ensure the RA is in the range 0 - 360.
      if racen < 0.0:
         racen += 360.0

#  The HPX projection is made up of four "gores", each of which spans
#  90 degrees in RA. Find the zero-based integer index of the gore
#  containing the current tile.
      igore = int( racen / 90.0 )

#  Discontinuities only affect tiles in the polar regions of the HPX
#  projection, so pass on if the Dec is below the value defining the
#  boundary between polar and equatorial regions. First deal with
#  northern polar tiles.
      if deccen > HPX_TRANS:

#  If this is the first polar tile, just record its gore index.
         if igore_first == -1:
            igore_first = igore

#  Otherwise, if the current polar tile is in a different gore to the first
#  polar tile, the NDF straddles a northern HPX discontinuity, so use an
#  XPH projection centred on the north pole.
         elif igore != igore_first:
            usexph = 1
            break

#  Now do the same for southern polar tiles. Set usexph to -1 to indicate
#  a southern XPH projection should be used.
      elif deccen < -HPX_TRANS:
         if igore_first == -1:
            igore_first = igore
         elif igore != igore_first:
            usexph = -1
            break;

#  Create a file holding the FITS-WCS header for the first tile, using
#  an HPX or XPH projection as determined above.
   head = "{0}/header".format(NDG.tempdir)
   invoke( "$SMURF_DIR/jsatileinfo itile={0} instrument={1} header={2} "
           "usexph={3} quiet".format(tiles[0],instrument,head,usexph) )

#  Get the lower pixel index bounds of the first tile.
   lx = int( starutil.get_task_par( "LBND(1)", "jsatileinfo" ) )
   ly = int( starutil.get_task_par( "LBND(2)", "jsatileinfo" ) )

#  Create a 1x1 NDF and store the tile headers in the FITS extension.
   ref = NDG(1)
   invoke( "$KAPPA_DIR/creframe out={0} mode=fl mean=0 lbound=\[{1},{2}\] "
           "ubound=\[{1},{2}\]".format(ref,lx,ly) )
   invoke( "$KAPPA_DIR/fitstext ndf={0} file={1}".format(ref,head) )

#  Get the nominal spatial pixel size of the supplied NDF.
   invoke( "$KAPPA_DIR/ndftrace ndf={0} quiet".format(inndf) )
   pixsize1 = float( starutil.get_task_par( "FPIXSCALE(1)", "ndftrace" ) )
   pixsize2 = float( starutil.get_task_par( "FPIXSCALE(2)", "ndftrace" ) )
   pixsize_in = math.sqrt( pixsize1*pixsize2 )

#  Get the nominal tile pixel size.
   invoke( "$KAPPA_DIR/ndftrace ndf={0} quiet".format(ref) )
   pixsize1 = float( starutil.get_task_par( "FPIXSCALE(1)", "ndftrace" ) )
   pixsize2 = float( starutil.get_task_par( "FPIXSCALE(2)", "ndftrace" ) )
   pixsize_tile = math.sqrt( pixsize1*pixsize2 )

#  If the tile (i.e. output) pixels are smaller than the input NDF pixels,
#  we use a SincSinc interpolation kernel with width equal to 2 input pixels.
   if pixsize_tile < 1.5*pixsize_in:
      method = "sincsinc"
      width = 2

#  If the tile pixels are larger than the input pixels, we use a Gauss
#  interpolation kernel with width equal to 0.8 tile pixels (so we
#  need to convert 0.8 tile pixels into the equivalent number of input
#  pixels, as required for wcsalign). Simulations seem to suggest 0.8 is
#  a good figure to use. See SCUBA-2 TRAC ticket #1333.
   else:
      method = "gauss"
      width = 0.8*pixsize_tile/pixsize_in

#  Create an intermediate NDF by resampling the supplied NDF onto the pixel
#  grid of the first tile. Do 2D and 3D separately.
   jsa_montage = NDG(1)
   invoke( "$KAPPA_DIR/wcsalign in={0} out={1} ref={2} lbnd=! method={3} "
           "params=\[0,{4}\]".format(inndf,jsa_montage,ref,method,width) )

#  Dice this resampled image up into parts corresponding to JSA tiles.
   print( invoke( "$SMURF_DIR/jsadicer in={0} out={1} instrument={2} trim={3}".
                  format(jsa_montage,outndf,instrument,trim) ) )

#  Remove temporary files.
   cleanup()

#  If a StarUtilError of any kind occurred, display the message but hide the
#  python traceback. To see the trace back, uncomment "raise" instead.
except starutil.StarUtilError as err:
#  raise
   print( err )
   cleanup()

# This is to trap control-C etc, so that we can clean up temp files.
except:
   cleanup()
   raise

