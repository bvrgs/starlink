/*
 				sexheadsc.h

*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*
*	Part of:	SExtractor
*
*	Author:		E.BERTIN, DeNIS/LDAC.
*
*	Contents:	header structure and templates for SkyCat output.
*
*	Last modify:	02/09/97
*                       23/10/98 (AJC):
*                         Remove col headings from skycathead
*
*%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
*/

char	skycathead[] = "QueryResult\n\n"
	"# Config entry for original catalog server:\n"
	"serv_type: catalog\n"
	"long_name: SExtractor catalog\n"
	"short_name: SExCat\n"
	"symbol: id diamond %4.1f\n"
	"# End config entry\n\n";

char	skycattail[] = "[EOD]";

