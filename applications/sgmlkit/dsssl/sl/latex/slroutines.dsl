<!--
Title:
  Starlink stylesheet - routine list

Author:
  Norman Gray, Glasgow (NG)

Revision history
  February 1999 (original version)

Copyright 1999, Particle Physics and Astronomy Research Council

$Id$
-->

<routine>
<description>Constructors for the ROUTINELIST element (LaTeX)
<codebody>
(mode section-reference
  ;;(element routinelist
  ;;  (make-section-reference title: (literal "Routine list")))
  (element routinelist
    (literal "Routine list"))
  (element codegroup
    (with-mode routine-ref-get-reference
      (process-node-list (current-node))))
  (element programcode
    (with-mode routine-ref-get-reference
      (process-node-list (current-node))))
  (element codegroup
    (with-mode routine-ref-get-reference
      (process-node-list (current-node))))
  (element routine
    (with-mode routine-ref-get-reference
      (process-node-list (current-node))))
  (element codecollection
    (make-section-reference
     ;set-prefix: (literal (number->string (child-number)) " ")
     set-prefix: (literal "Code collection: ")
     title: (with-mode routine-ref-get-reference
	      (process-codecollection (attribute-string (normalize "doc")))))))


;; Routinelist is simple
;(element routinelist
;  ($latex-section$ "section"))
(element routinelist
  (process-children))

;; Supporting the codecollection chunking/sectioning isn't as easy as with
;; the other such elements, because it doesn't have any children in this
;; document.  We have to do it rather more by hand, therefore.
;; Don't yet support the `includeonly' attribute.
(element codecollection
  (let* ((parent-sect (ancestor-member (current-node)
				       '("sect" "subsect"
					 "subsubsect" "subsubsubsect")))
	 ;; Set this-sect-str to a pair of LaTeX sectioning commands
	 ;; respectively one and two levels below the parent-sect, if
	 ;; available.  The `article' sequence is section, subsection,
	 ;; subsubsection, paragraph, subparagraph
	 (this-sect-str
	  (if (node-list-empty? parent-sect)
	      (error "codecollection is not in a section!")
	      (case (gi parent-sect)
		(("subsubsubsect")
		 (cons "subparagraph" "subparagraph"))
		(("subsubsect")
		 (cons "paragraph" "subparagraph"))
		(("subsect")
		 (cons "subsubsection" "paragraph"))
		(("sect")
		 (cons "subsection" "subsubsection"))
		(else
		 (error "codecollection has impossible sectioning!"))))))
  (make sequence
    ($latex-section$ (car this-sect-str) children: #f) ;don't process children
    (make fi
      data: (string-append "\\def\\routinesubsectlevel{"
			   (cdr this-sect-str) "}"))
    (with-mode routine-ref-sst
      (process-codecollection (attribute-string (normalize "doc")))))))

(define (process-codecollection docent)
  (let ((docelem (document-element-from-entity docent)))
      (process-node-list docelem)))

;; Process a list consisting of all the "author" elements in the
;; programcode document.  Split the author list into those with the
;; same ID as the first author, and those with a different ID: call
;; process-one-author on the first set, and call process-authorlist
;; again on the second.
(define (process-authorlist nl)
  (if (node-list-empty? nl)
      (empty-sosofo)
      (let* ((first-author (attribute-string (normalize "id")
					     (node-list-first nl)))
	     (alists (let loop ((anl nl)
				(thisaut (empty-node-list))
				(otherauts (empty-node-list)))
		       (if (node-list-empty? anl)
			   (cons thisaut otherauts)
			   (if (string=?
				first-author
				(attribute-string (normalize "id")
						  (node-list-first anl)))
			       (loop (node-list-rest anl)
				     (node-list thisaut
						(node-list-first anl))
				     otherauts)
			       (loop (node-list-rest anl)
				     thisaut
				     (node-list otherauts
						(node-list-first anl))))))))
	(make sequence
	  (process-one-author (car alists))
	  (process-authorlist (cdr alists))))))

;; Process a single-author node-list.  Collect the relevant
;; information from the nodes in the list.  Use the first attribute
;; present, and don't check or warn if the information is
;; inconsistent.
;;
;; The processing here is less sophisticated than it could be, as
;; there's no real cross-referencing between individual author
;; elements and the aggregate information.
(define (process-one-author nl)
  (let ((autprops (let loop ((auts nl)
			     (name #f)
			     (aff #f)
			     (email #f)
			     (web #f))
		    (if (or (node-list-empty? auts)
			    (and name aff email web))
			(list name aff email web)
			(let ((aut (node-list-first auts)))
			  (loop (node-list-rest auts)
				(or name
				    (data aut))
				(or aff
				    (attribute-string (normalize "affiliation")
						      aut))
				(or email
				    (attribute-string (normalize "email")
						      aut))
				(or web
				    (attribute-string (normalize "webpage")
						      aut))))))))
    (make sequence
      (make empty-command name: "item"
	    parameters: `(,(string-append "?"
					  (or (car autprops)
					      "Mystery programmer"))))
      (if (cadr autprops)
	  (literal (string-append " (" (cadr autprops) ")"))
	  (empty-sosofo))
      (if (caddr autprops)
	  (make sequence
	    (literal " Email:")
	    (make command name: "Code"
		  (literal (caddr autprops)))
	    (literal ". "))
	  (empty-sosofo))
      (if (cadddr autprops)
	  (make sequence
	    (literal " URL:")
	    (make command name: "Code"
		  (literal (string-append "<" (cadddr autprops) ">. "))))
	  (empty-sosofo)))))

(mode routine-ref-sst
  (element docblock
    ;; Only prepare an authorlist for the top-level docblock, and not
    ;; for docblock elements in codegroups, for example.  There are
    ;; separate, simpler, author lists for each routine.
    (if (string=? (gi (parent (current-node)))
		  (normalize "programcode"))
	(let ((allauthors (select-elements
			   (descendants (parent (current-node)))
			   (normalize "author"))))
	  ;; docblock always has a title element, but this is suppressed in
	  ;; this mode.  If there's _more_ than one element, then other
	  ;; docblock elements are present, so should be put into a
	  ;; description list.
	  (make sequence
	    (if (or (> (node-list-length (children (current-node))) 1)
		    (not (node-list-empty? allauthors)))
		(make environment name: "description"
		      (make sequence
			(with-mode docblock
			  (process-children))
			;; process authors
			(if (node-list-empty? allauthors)
			    (empty-sosofo)
			    (make sequence
			      (make empty-command name: "item"
				    parameters: '("?Authors"))
			      (make fi data: "\\hbox{}\\hfil\\\\")
			      (make environment name: "raggedright"
				    (make environment name: "description"
					  (process-authorlist allauthors)))))))
		(empty-sosofo))))
	(empty-sosofo)))
  (element routine
    (process-matching-children 'routineprologue))
  (element routineprologue
    (let ((kids (nl-to-pairs (children (current-node)))))
      (make sequence
	(make empty-command name: "clearpage")
	(make environment name: "sstroutine"
	      ;; takes two parameters: name and purpose
	      (make sequence
		(make environment brackets: '("{" "}")
		      (let ((name (assoc (normalize "routinename") kids)))
			(if name
			    (process-node-list (cdr name))
			    (literal "Unknown name!"))))
		(make environment brackets: '("{" "}")
		      (let ((purp (assoc (normalize "purpose") kids)))
			(if purp
			    (process-node-list (cdr purp))
			    (if (assoc (normalize "description") kids)
				(process-node-list
				 (cdr (assoc (normalize "description")
					     kids)))
				(empty-sosofo) ;shouldn't happen
				))))
		;; environment contents -- description, etc
		(apply sosofo-append
		       (map (lambda (gi)
			      (let ((gi-and-nd (assoc (normalize gi) kids)))
				(if gi-and-nd
				    (process-node-list (cdr gi-and-nd))
				    (empty-sosofo))))
			    '(;;"routinename"
			      ;;"purpose"
			      "moduletype"
			      "description"
			      "userkeywords"
			      "softwarekeywords"
			      "returnvalue"
			      "argumentlist"
			      "parameterlist"
			      "authorlist"
			      ;;"history"
			      "usage"
			      "invocation"
			      "examplelist"
			      "implementationstatus"
			      "bugs")))
		;; now collect together the diytopics
		(apply sosofo-append
		       (map (lambda (gi-and-nd)
			      (if (string=? (normalize (car gi-and-nd))
					    (normalize "diytopic"))
				  (process-node-list (cdr gi-and-nd))
				  (empty-sosofo)))
			    kids))))
	(make empty-command name: "clearpage"))))
  (element routinename
    (process-children))
  (element name
    (process-children))
;  (element name
;    (make command name: "Code"
;	  (process-children)))
  (element othernames
    (let* ((names (children (current-node)))
	   (namelist (node-list-reduce
		      names
		      (lambda (res i)
			(if (string=? res "")
			    (data i)
			    (string-append res ", " (data i))))
		      "")))
      (literal (string-append " (also: " namelist ")"))))
  ;;  (element purpose
  ;;    (make sequence
  ;;      (make command name: "textbf"
  ;;	    (literal "Purpose: "))
  ;;      (process-children)))
  (element description
    (make environment name: "sstdescription"
	  (process-children)))
  (element moduletype
    (make environment name: "sstdiytopic"
	  parameters: '("Type of Module")
	  (process-children)))
  ;;(element userkeywords
  ;;  (make environment name: "sstdiytopic"
  ;;  parameters: '("Keywords")
  ;;  (process-children)))
  ;;(element softwarekeywords
  ;;  (make environment name: "sstdiytopic"
  ;;  parameters: '("Code group")
  ;;	  (process-children)))
  (element returnvalue
    (let ((none-att (attribute-string (normalize "none")))
	  (type-att (attribute-string (normalize "type"))))
      (make environment name: "sstreturnedvalue"
	    (make sequence
	      (make empty-command name: "item"
		    parameters: (if type-att
				    (list type-att)
				    '("Unknown type")))
	      (if none-att
		  (make command name: "emph"
			(literal "No return value")) ;...and discard any data
		  (process-children))))))
  (element parameterlist
    (let ((none-att (attribute-string (normalize "none"))))
      (make environment name: "sstparameters"
	    (if none-att
		(make command name: "sstsubsection"
		      (literal "No parameters"))
		(process-children)))))
  (element argumentlist
    (let ((none-att (attribute-string (normalize "none"))))
      (make environment name: "sstarguments"
	    (if none-att
		(literal "No arguments")
		(process-children)))))
  (element parameter
    (let* ((kids (children (current-node)))
	   (name (select-elements kids (normalize "name")))
	   (type (select-elements kids (normalize "type")))
	   (desc (select-elements kids (normalize "description")))
	   ;;(opt-att (attribute-string (normalize "optional")))
	   (given-att (attribute-string (normalize "given")))
	   (returned-att (attribute-string (normalize "returned"))))
      (make sequence
	(make command name: "sstsubsection"
	      (make sequence
		(process-node-list name)
		(literal "=")
		(process-node-list type)
		(literal " ("
			 (cond
			  ((and given-att returned-att)
			   "given and returned")
			  (given-att "given")
			  (returned-att "returned")
			  (else		;default is given
			   "given"))
			 ")")))
	(with-mode plain-elements
	      (process-node-list desc)))))
  (element authorlist
    (make environment name: "sstdiytopic"
	  parameters: '("Authors")
	  (make environment name: "itemize"
		(process-children))))
  (element author
    (make sequence
      (make empty-command name: "item")
      (process-children)))
  (element otherauthors
    (make sequence
      (make empty-command name: "item")
      (literal "Other authors")
      (make environment name: "itemize"
	    (process-children))))
  (element examplelist
    (make environment name: "sstexamples"
	  (process-children)))
  (element example
    (make environment name: "sstexamplesubsection"
	  (make environment name: "quote"
		(make environment name: "small"
		      (make environment
			name: "verbatim"
			recontrol: "/-/"
			escape-tex?: #f
			(process-children))))))
  ;;(element example
  ;;  (make environment name: "sstexamplesubsection"
  ;;  (process-children)))
  (element examplenote
    (process-children))
  (element usage
    (make environment name: "sstusage"
	  (process-children)))
  (element invocation
    (make environment name: "sstinvocation"
	  (process-children)))
  (element implementationstatus
    (make environment name: "sstimplementationstatus"
	  (process-children)))
  (element bugs
    (make environment name: "sstbugs"
	  (process-children)))
  (element diytopic
    (let ((kids (children (current-node))))
      (make sequence
	(make environment name: "sstdiytopic"
	      parameters: (list (data (node-list-first kids)))
	      (process-node-list (node-list-rest kids))))))
  )

;; Since some of the element names are overloaded (for example,
;; description is part of both routinelist and parameter), we
;; occasionally need simpler variants of elements.
(mode plain-elements
  (element description
    (process-children)))

(mode docblock
  (element title			; ignore in this mode -- title
					; is taken care of by
					; $latex-section$ in element
					; codecollection
    (empty-sosofo))
  (element description
    (make sequence
      (make empty-command name: "item"
	    parameters: '("?Description"))
      (process-children)))
  (element userkeywords
    (make sequence
      (make empty-command name: "item"
	    parameters: '("?User keywords"))
      (process-children)))
  (element softwarekeywords
    (make sequence
      (make empty-command name: "item"
	    parameters: '("?Software category"))
      (process-children)))
  (element copyright
    (make sequence
      (make empty-command name: "item"
	    parameters: '("?Copyright"))
      (process-children)))
  (element authorlist
    (empty-sosofo))			; managed specially in element docblock
  (element history
    (empty-sosofo))
  ;; Following works, it's just that I don't want to produce this output.
;   (element history
;     (make sequence
;       (make empty-command name: "item"
; 	    parameters: '("?History"))
;       (make environment name: "description"
; 	    (process-children))))
;   (element change
;     (let ((heading (string-append (attribute-string (normalize "date")
; 						    (current-node))
; 				  " by "
; 				  (attribute-string (normalize "author")
; 						    (current-node)))))
;       (make sequence
; 	(make empty-command name: "item"
; 	      parameters: `(,(string-append "?" heading)))
; 	(process-children))))
  )

;;; Simpler routine-ref mode which doesn't use SST.  Very ugly!
(mode routine-ref
  (element codegroup
    (make sequence
      (make command name: "subsection"
	    (literal "Code group: ")
	    (with-mode routine-ref-get-reference
	      (process-children)))
      (process-children)))
  (element codereference
    (let ((ref-docelem (document-element-from-entity
			(attribute-string (normalize "doc")))))
      ;; possibly make this a link, in future
      (make sequence
	(make command name: "subsection"
	      (literal "Code reference"))
	(literal "Refers to ")
	(make command name: "textit"
	      (with-mode routine-ref-get-reference
		(process-node-list ref-docelem)))
	(process-children))))
  (element docblock
    (process-children))
  (element title
    (make command name: "section"
	  (process-children)))
  (element (codereference docblock title) ; discard, in this mode
    (empty-sosofo))			; (see mode routine-ref-get-reference)
  (element (codegroup docblock title)	; discard, in this mode
    (empty-sosofo))			; (see mode routine-ref-get-reference)
  (element authorlist
    (make sequence
      (make command name: "subsubsection"
	    (literal "Authors"))
      (make environment name: "itemize"
	    (process-children))))
  (element author
    (let ((affil (attribute-string (normalize "affiliation")))
	  (id (attribute-string (normalize "id"))))
      (make sequence
	(make empty-command name: "item")
	(process-children)
	(if affil
	    (literal (string-append " (" affil ")"))
	    (empty-sosofo)))))
  (element authorref
    (let* ((aut-id (attribute-string (normalize "id")))
	   (aut-el (and aut-id
			(element-with-id aut-id)))
	   (note (attribute-string (normalize "note"))))
      (if (and (not (node-list-empty? aut-el))
	       (string=? (gi aut-el) (normalize "author")))
	  (make sequence
	    (with-mode routine-ref-get-reference
	      (process-node-list aut-el))
	    (if note
		(literal (string-append " (" note ")"))
		(empty-sosofo)))
	  (error (string-append "ID " aut-id " is not an AUTHOR element")))))
  (element authornote
    (make sequence
      (literal " --- ")
      (process-children)))
  (element otherauthors
    (make sequence
      (make empty-command name: "item")
      (make paragraph (literal "Other contributors"))
      (make environment name: "itemize"
	    (process-children))))
  (element copyright
    (make sequence
      (make command name: "subsection"
	    (literal "Copyright"))
      (process-children)))
  (element history
    (make sequence
      (make command name: "subsection"
	    (literal "Change history"))
      (make environment name: "description"
	    (process-children))))
  (element change
    (let ((auth-id (attribute-string (normalize "author")))
	  (date-str (attribute-string (normalize "date"))))
      (make sequence
	(make empty-command name: "item"
	      parameters: (list
			   (string-append "?" ; optional arg
					  (trim-data (element-with-id auth-id))
					  ", "
					  (format-date date-str))))
	(process-children))))
  (element routine
    (let* ((rp (select-elements (children (current-node)) 'routineprologue))
	   (rn (and (not (node-list-empty? rp))
		    (select-elements (children rp) 'routinename)))
	   (id (or (attribute-string (normalize "id") (current-node))
		   (attribute-string (normalize "id") rn))))
      (make sequence
	(make command name: "subsection"
	      (if id
		  ;(make empty-command name: "label"
			;parameters: `(,(string-append "R" id)))
		  (make command name: "label"
			(literal (gen-label)))
		  (empty-sosofo))
	      (if (node-list-empty? rn)
		  (literal "Anonymous routine")
		  (with-mode routine-ref-get-reference
		    (process-node-list rn))))
	(process-node-list rp))))
  (element routineopener
    (empty-sosofo))
  (element description
    (process-children))
  
  ;; The funcname element could be made more sophisticated, so that
  ;; it includes a link (possibly using the source-code browser) to
  ;; the function definition/documentation.
  (element funcname
    (make command name: "texttt"
          (literal (string-append "(" (data (current-node)) ")"))))
  ;; discard the following elements, at present
  (element codebody
    (empty-sosofo))
  ;;(element misccode
  ;;  (empty-sosofo))
  )


;; Mode which includes assorted variants of the handlers above, designed
;; to extract information to which other handlers have made cross-reference.
(mode routine-ref-get-reference
  (default (empty-sosofo))
  (element programcode
    (process-matching-children 'docblock))
  (element codegroup
    (process-matching-children 'docblock))
  (element docblock
    (process-matching-children 'title))
  (element title
    (process-children))
  (element author
    (let ((attrib (attribute-string (normalize "attribution"))))
      (make sequence
	(make empty-command name: "item")
	(process-matching-children 'name)
	(if attrib
	    (literal (string-append " (" attrib ")"))
	    (empty-sosofo)))))
  (element routine
    (process-matching-children 'routineprologue))
  (element routineprologue
    (process-matching-children 'routinename))
  (element routinename
    (process-children))
  (element name
    (process-children)))

;; Coderef in this context is very simple, just requiring that we
;; print the element content.  If there's no content, then scurry to
;; the referenced routine to find its name.
(element coderef
  (if (string=? (data (current-node)) "")
      (let* ((cc (node-list-or-false
		  ;; If there's a collection attribute, then it
		  ;; provides the ID of a codecollection element.  If
		  ;; this attribute isn't present, then use instead
		  ;; the _first_ (which includes the only)
		  ;; codecollection element in the document.
		  (if (attribute-string (normalize "collection"))
		      (element-with-id
		       (attribute-string (normalize "collection")))
		      (node-list-first
			(select-elements
			 (select-by-class (descendants (getdocbody))
					  'element)
			 'codecollection)))))
	     (ccdoc (and cc
			 (document-element-from-entity
			  (attribute-string (normalize "doc") cc))))
	     (target (and ccdoc
			  (node-list-or-false
			   (element-with-id
			    (attribute-string (normalize "id"))
			    ccdoc))))
	     (targetroutine (and target
				 (if (equal? (gi target)
					     (normalize "routine"))
				     target
				     (ancestor (normalize "routine")
					       target)))))
	(with-mode section-reference
	  (process-node-list targetroutine)))
      (process-children)))


;; Following is a more sophisticated version, which includes a LaTeX pageref.
; (element coderef
;   (let* ((cc (node-list-or-false
; 	      (element-with-id (attribute-string (normalize "collection")))))
; 	 (ccdoc (and cc
; 		     (document-element-from-entity
; 		      (attribute-string (normalize "doc") cc))))
; 	 (target (and ccdoc
; 		      (node-list-or-false
; 		       (element-with-id (attribute-string (normalize "id"))
; 					ccdoc))))
; 	 (targetroutine (and target
; 			     (if (equal? (gi target) (normalize "routine"))
; 				 target
; 				 (ancestor (normalize "routine")
; 					   target)))))
;     (make sequence
;       (process-children)
;       (if targetroutine
; 	  (make sequence
; 	    (literal " (p.")
; 	    (make empty-command name: "pageref"
; 		  parameters: `(,(string-append
; 				  "R"
; 				  (attribute-string (normalize "id")))))
; 	    (literal ")")
; 	    (empty-sosofo))
; 	  (error (string-append "Can't find one of collection "
; 			      (attribute-string (normalize "collection"))
; 			      " or routine "
; 			      (attribute-string (normalize "id"))))))))
