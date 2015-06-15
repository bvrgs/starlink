#!/usr/bin/env python

'''
*+
*  Name:
*     pol2scan

*  Purpose:
*     Create Q and U maps from a set of POL-2 "spin&scan" data
*     files.

*  Language:
*     python (2.7 or 3.*)

*  Description:
*     This script runs SMURF:CALCQU on the POL-2 data files specified by
*     parameter IN, to create a set of down-sampled time series files
*     holding Q and U values in each bolometer. These time series are
*     then converted into separate Q and U maps using SMURF:MAKEMAP.
*
*     This mainly a test bed for experiments in creating maps from
*     POL2 spin&scan data.

*  Usage:
*     pol2scan in q u [config] [retain] [msg_filter] [ilevel] [glevel] [logfile]

*  Parameters:
*     CONFIG = LITERAL (Read)
*        The MAKEMAP configuration parameter values to use. The following
*        defaults will be used if the supplied configuration does not
*        specify any values for the following parameters:
*
*        numiter=-20
*        maptol=0.05
*        itermap=2
*        ast.zero_snr_ffclean=1
*        ast.zero_snr_hipass=200
*        ast.zero_snr=4
*        ast.zero_snr_neg=1
*        ast.skip=2
*        com.zero_snr_ffclean=1
*        com.zero_snr_hipass=200
*        com.zero_snr=4
*        com.zero_snr_neg=1
*        noisecliphigh=3
*        com.perarray=1
*        com.noflag=1
*        spikethresh = 5
*        spikebox = 10
*
*        The following values will always over-ride any values in the
*        supplied configuration:
*
*           flagslow = 0.01
*           downsampscale = 0
*           noi.usevar=1
*
*        All the above values are used if a null (!) value is supplied. [!]
*     GLEVEL = LITERAL (Read)
*        Controls the level of information to write to a text log file.
*        Allowed values are as for "ILEVEL". The log file to create is
*        specified via parameter "LOGFILE. In adition, the glevel value
*        can be changed by assigning a new integer value (one of
*        starutil.NONE, starutil.CRITICAL, starutil.PROGRESS,
*        starutil.ATASK or starutil.DEBUG) to the module variable
*        starutil.glevel. ["ATASK"]
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
*        A group of POL-2 time series NDFs. Only used if a null (!) value is
*        supplied for INQU.
*     INQU = NDF (Read)
*        A group of NDFs containing Q and U time-series calculated by a
*        previous run of SMURF:CALCQU (the LSQFIT parameter must be set
*        to TRUE when running CALCQU). If not supplied, the IN parameter
*        is used to get input NDFs holding POL-2 time series data. [!]
*     LOGFILE = LITERAL (Read)
*        The name of the log file to create if GLEVEL is not NONE. The
*        default is "<command>.log", where <command> is the name of the
*        executing script (minus any trailing ".py" suffix), and will be
*        created in the current directory. Any file with the same name is
*        over-written. The script can change the logfile if necessary by
*        assign the new log file path to the module variable
*        "starutil.logfile". Any old log file will be closed befopre the
*        new one is opened. []
*     MSG_FILTER = LITERAL (Read)
*        Controls the default level of information reported by Starlink
*        atasks invoked within the executing script. This default can be
*        over-ridden by including a value for the msg_filter parameter
*        within the command string passed to the "invoke" function. The
*        accepted values are the list defined in SUN/104 ("None", "Quiet",
*        "Normal", "Verbose", etc). ["Normal"]
*     Q = NDF (Read)
*        The output NDF in which to return the Q intensity map.
*     U = NDF (Read)
*        The output NDF in which to return the U intensity map.
*     QREF = NDF (Read)
*        An optional map defining the pixel grid for the output Q
*        map. If both QREF and UREF are supplied, the polarimetric
*        reference direction used by the Q and U maps will be rotated to
*        match the reference direction in the supplied QREF and UREF
*        maps. [!]
*     RETAIN = _LOGICAL (Read)
*        Should the temporary directory containing the intermediate files
*        created by this script be retained? If not, it will be deleted
*        before the script exits. If retained, a message will be
*        displayed at the end specifying the path to the directory. [FALSE]
*     UREF = NDF (Read)
*        An optional map defining the pixel grid for the output U
*        map. If both QREF and UREF are supplied, the polarimetric
*        reference direction used by the Q and U maps will be rotated to
*        match the reference direction in the supplied QREF and UREF
*        maps. [!]

*  Copyright:
*     Copyright (C) 2015 East Asian Observatory.
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
*     20-MAY-2015 (DSB):
*        Original version
*-
'''

import os
import math
import starutil
from starutil import invoke
from starutil import NDG
from starutil import Parameter
from starutil import ParSys
from starutil import msg_out
from starutil import AtaskError

#  Assume for the moment that we will not be retaining temporary files.
retain = 0

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

   params.append(starutil.ParNDG("IN", "The input POL2 time series NDFs",
                                 starutil.get_task_par("DATA_ARRAY","GLOBAL",
                                                       default=Parameter.UNSET)))

   params.append(starutil.ParNDG("Q", "The output Q intensity map",
                                 default=None, exists=False, minsize=1,
                                 maxsize=1 ))

   params.append(starutil.ParNDG("U", "The output Q intensity map",
                                 default=None, exists=False, minsize=1,
                                 maxsize=1 ))

   params.append(starutil.Par0S("CONFIG", "Map-maker tuning parameters",
                                "def", noprompt=True))

   params.append(starutil.ParNDG("INQU", "NDFs containing previously calculated Q and U values",
                                 None,noprompt=True))

   params.append(starutil.Par0L("RETAIN", "Retain temporary files?", False,
                                 noprompt=True))

   params.append(starutil.ParNDG("QREF", "The reference Q map", default=None,
                                 noprompt=True, minsize=0, maxsize=1 ))

   params.append(starutil.ParNDG("UREF", "The reference U map", default=None,
                                 noprompt=True, minsize=0, maxsize=1 ))


#  Initialise the parameters to hold any values supplied on the command
#  line.
   parsys = ParSys( params )

#  It's a good idea to get parameter values early if possible, in case
#  the user goes off for a coffee whilst the script is running and does not
#  see a later parameter propmpt or error...

#  See if pre-calculated Q and U values have been supplied on the command
#  line. If so, we use these in preference to any raw time-series files
#  specified via parameter "IN".
   inqu = parsys["INQU"].value

#  Get the raw POL-2 data files. They should be supplied as the first item on
#  the command line, in the form of a Starlink "group expression" (i.e.
#  the same way they are supplied to other SMURF commands such as makemap).
   if inqu == None:
      indata = parsys["IN"].value
   else:
      indata = None

#  Now get the Q and U values to use.
   qmap = parsys["Q"].value
   umap = parsys["U"].value

#  The user-supplied makemap config.
   config = parsys["CONFIG"].value

#  See if temp files are to be retained.
   retain = parsys["RETAIN"].value

#  Get the Q and U reference maps
   qref = parsys["QREF"].value
   uref = parsys["UREF"].value

#  If no Q and U values were supplied, create a set of Q and U time
#  streams from the supplied analysed intensity time streams.
   if inqu == None:
      msg_out( "Calculating Q and U time streams for each bolometer...")
      invoke("$SMURF_DIR/calcqu in={0} lsqfit=yes config=def outq={1}/\*_QT "
             "outu={1}/\*_UT".format( indata, NDG.tempdir ) )

#  Get groups listing the time series files created by calcqu.
      qts = NDG( "{0}/*_QT".format( NDG.tempdir ) )
      uts = NDG( "{0}/*_UT".format( NDG.tempdir ) )

#  If pre-calculated Q and U values were supplied, identifiy the Q and U
#  files.
   else:
      msg_out( "Using pre-calculating Q and U values...")

      qndfs = []
      undfs = []
      for ndf in inqu:
         invoke("$KAPPA_DIR/ndftrace {0} quiet".format(ndf) )
         label = starutil.get_task_par( "LABEL", "ndftrace" )
         if label == "Q":
            qndfs.append( ndf )
         elif label == "U":
            undfs.append( ndf )
         else:
            raise starutil.InvalidParameterError("Q/U time-series {0} has "
                    "an unknown Label {1} - must be 'Q' or 'U'.".
                    format(ndf,label))

      qts = NDG( qndfs )
      uts = NDG( undfs )


#  Create a config file to use with makemap.
   conf = os.path.join(NDG.tempdir,"conf")
   fd = open(conf,"w")

#  First put in the defaults supplied by this script. These may be
#  over-written by the user-supplied config.
   fd.write("numiter=-20\n")
   fd.write("maptol=0.05\n")
   fd.write("itermap=2\n")
   fd.write("ast.zero_snr_ffclean=1\n")
   fd.write("ast.zero_snr_hipass=200\n")
   fd.write("ast.zero_snr=4\n")
   fd.write("ast.zero_snr_neg=1\n")
   fd.write("ast.skip=2\n")
   fd.write("com.zero_snr_ffclean=1\n")
   fd.write("com.zero_snr_hipass=200\n")
   fd.write("com.zero_snr=4\n")
   fd.write("com.zero_snr_neg=1\n")
   fd.write("noisecliphigh=3\n")
   fd.write("com.perarray=1\n")
   fd.write("com.noflag=1\n")
   fd.write("spikethresh=5\n")
   fd.write("spikebox=10\n")

#  Now put in the user-supplied config.
   if config != "def":
      fd.write("{0}\n".format(config))

#  Now put in values that are absolutely required by this script. These
#  over-write any values in the user-supplied config.
   fd.write("noi.usevar=1\n")
   fd.write("flagslow=0.01\n")
   fd.write("downsampscale=0\n")
   fd.close()

#  If both QREF and UREF were supplied, we will be rotating the
#  polarimetric reference direction. So store the orignal maps in
#  intermediate NDFs rather than the final NDFs.
   if qref and uref:
      tqmap = NDG(1)
      tumap = NDG(1)
   else:
      tqmap = qmap
      tumap = umap

#  Make a map from the Q time series.
   msg_out( "Making a map from the Q time series...")
   if qref:
      ref = qref
   else:
      ref = "!"
   invoke("$SMURF_DIR/makemap in={0} config=^{1} out={2} ref={3}".
          format(qts,conf,tqmap,ref))

#  Make a map from the U time series.
   msg_out( "Making a map from the U time series..." )
   if uref:
      ref = uref
   else:
      ref = "!"
   invoke("$SMURF_DIR/makemap in={0} config=^{1} out={2} ref={3}".
          format(uts,conf,tumap,ref))

#  Rotate the polarimetric reference direction if required.
   if qref and uref:
      msg_out( "Rotating the polarimetric reference direction to "
               "match {0}...".format(qref) )
      invoke("$POLPACK_DIR/polrotref qin={0} uin={1} qout={2} uout={3} "
             "like={4}".format(tqmap,tumap,qmap,umap,qref) )

#  Remove temporary files.
   cleanup()

#  If an StarUtilError of any kind occurred, display the message but hide the
#  python traceback. To see the trace back, uncomment "raise" instead.
except starutil.StarUtilError as err:
#  raise
   print( err )
   cleanup()

# This is to trap control-C etc, so that we can clean up temp files.
except:
   cleanup()
   raise

