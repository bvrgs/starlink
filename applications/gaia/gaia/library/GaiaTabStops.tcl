#+
#  Name:
#     GaiaTabStops

#  Type of Module:
#     [incr Tcl] class

#  Purpose:
#     Widget to allow the interactive placement of tab stops

#  Description:
#     This class defines a widget that displays a line of text, which
#     can have "tab stops" interactively moved into place along
#     it. The number of tab stops is initially controlled by a
#     configuration option, however, new stops may be added
#     interactively.

#  Invocations:
#
#        GaiaTabStops object_name [configuration options]
#
#     This creates an instance of a GaiaTabStops object. The return is
#     the name of the object.
#
#        object_name configure -configuration_options value
#
#     Applies any of the configuration options (after the instance has
#     been created).
#
#        object_name method arguments
#
#     Performs the given method on this object.

#  Configuration options:
#     See itk_option define statements below.

#  Methods:
#     See method declarations below.

#  Inheritance:
#     FrameWidget

#  Authors:
#     PWD: Peter Draper (STARLINK - Durham University)
#     {enter_new_authors_here}

#  History:
#     10-APR-2000 (PWD):
#        Original version.
#     {enter_further_changes_here}

#-

#.

itk::usual GaiaTabStops {}

itcl::class gaia::GaiaTabStops {

   #  Inheritances:
   #  -------------
   inherit util::FrameWidget

   #  Constructor:
   #  ------------
   constructor {args} {

      #  Evaluate options (note no keep sections allowed beyond this point).
      eval itk_initialize $args

      #  Create the display elements. These are a scrolled canvas
      #  region with a label and a series of triangles pointing 
      #  to the tab positions.
      itk_component add canvas {
         scrolledcanvas $w_.canvas \
            -autoresize 1 \
            -vscrollmode none \
            -hscrollmode dynamic
      }
      pack $itk_component(canvas) -side top -fill both -expand 1
      set canvas_ $itk_component(canvas)

      itk_component add label {
         label $canvas_.textlabel \
            -textvariable [scope itk_option(-text)]
      }
      set label_ $itk_component(label)
      update_label_size_

      #  Work out the geometry of the triangles.
      set trioffset_ [expr int($entheight_)+1]
      set triheight_ [expr $trioffset_+$triheight_]

      #  Set height of canvas to include label and triangles.
      $canvas_ configure -height [expr $triheight_+30]

      #  Add the "source" tab marker. Pressing <1> generates a new
      #  marker.
      set xo 5
      set id [$canvas_ create polygon \
                 $xo $trioffset_ \
                 [expr $xo-$triwidth_] $triheight_ \
                 [expr $xo+$triwidth_] $triheight_ \
                 -tag $w_.tab_source -fill blue]
      $canvas_ bind $id <ButtonPress-1> [code $this add_stop_]

      #  Add label to the canvas.
      set labelid_ [$canvas_ create window [expr int($step_*1.5)] 0 \
                       -anchor nw -width $entwidth_ \
                       -height $entheight_ -window $label_]
   }

   #  Destructor:
   #  -----------
   destructor  {
   }

   #  Methods:
   #  --------

   #  Move a tab-stop marker.
   protected method move_stop_ {id x} {
      set cx [$canvas_ canvasx $x $step_]
      if { $cx > $step_ && $cx < $entwidth_+$step_ } {
         $canvas_ coords $id \
            $cx $trioffset_ \
            [expr $cx-$triwidth_] $triheight_ \
            [expr $cx+$triwidth_] $triheight_
      }
   }

   #  Remove a tab-stop.
   protected method remove_stop_ {id} {
      $canvas_ delete $id
   }

   #  Add a tab-stop. 
   protected method add_stop_ {} {
      set xo 10
      set id [$canvas_ create polygon \
                 $xo $trioffset_ \
                 [expr $xo-$triwidth_] $triheight_ \
                 [expr $xo+$triwidth_] $triheight_ \
                 -tag $w_.tabs -fill red]
      $canvas_ bind $id <B1-Motion> [code $this move_stop_ $id %x]
      $canvas_ bind $id <2> [code $this remove_stop_ $id]
      $canvas_ bind $id <ButtonRelease-1> [code $this update_stops_]
   }

   #  Update the size of the label widget (in response to new text).
   protected method update_label_size_ {} {
      set entheight_ [winfo reqheight $label_]
      set entwidth_ [winfo reqwidth $label_]
      set step_ [expr $entwidth_/[string length $itk_option(-text)]]
      if { $labelid_ != {} } {
         $canvas_ itemconfigure $labelid_ -width $entwidth_ \
            -height $entheight_
      }
   }

   #  Update the stop positions in characters (rather than canvas) and 
   #  initiate the changed command if needed.
   protected method update_stops_ {} {
      catch {unset indexes_}
      foreach id [$canvas_ find withtag $w_.tabs] {
         lassign [$canvas_ coords $id] x
         append indexes_ "[expr int($x/$step_)-1] "
      }
      if { $itk_option(-change_cmd) != {} } {
         eval $itk_option(-change_cmd) {$indexes_}
      }
   }

   #  Return the character positions of the stops.
   public method get {} {
      return $indexes_
   }

   #  Set the "state" of the widget. Normal or disabled.
   protected method update_state_ {} {
      if { $itk_option(-state) == "normal" } { 
         catch {blt::busy release $w_}
         catch {configure -foreground $itk_option(-enabledforeground)}
      } else {
         catch {blt::busy hold $w_ -cursor {}}
         catch {configure -foreground $itk_option(-disabledforeground)}
      }
   }

   #  Configuration options: (public variables)
   #  ----------------------

   #  The text shown in the label widget. Also updates the label
   #  widget displayed size.
   itk_option define -text text Text "gaia::GaiaTabStops" {
      if { $label_ != {} } {
         update_label_size_
      }
   }

   #  Command to execute when the stops are changed.
   itk_option define -change_cmd change_cmd Change_cmd {}

   #  State of widget -- normal or disabled.
   itk_option define -state state State normal {
      update_state_
   }

   #  Colour of disabled text.
   itk_option define -disabledforeground disabledforeground \
      Disabledforeground gray90

   #  Colour of enabled text.
   itk_option define -enabledforeground enabledforeground \
      Enabledforeground black

   #  Protected variables: (available to instance)
   #  --------------------

   #  Quick name of canvas.
   protected variable canvas_ {}

   #  Quick name of label.
   protected variable label_ {}

   #  Canvas identifier of label widget.
   protected variable labelid_ {}

   #  Shape of pointer triangles.
   protected variable triwidth_ 3
   protected variable triheight_ 10
   protected variable trioffset_ 20

   #  Width and height (in pixels) of label widget.
   protected variable entwidth_ 100
   protected variable entheight_ 20
   
   #  Size of a character in pixels.
   protected variable step_ 1

   #  Character indices of the stops.
   protected variable indexes_ {}

   #  Common variables: (shared by all instances)
   #  -----------------


#  End of class definition.
}
