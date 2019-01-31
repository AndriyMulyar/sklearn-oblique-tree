/****************************************************************/
/* Copyright 1993, 1994                                         */
/* Johns Hopkins University			                */
/* Department of Computer Science		                */
/****************************************************************/
/* Contact : murthy@cs.jhu.edu					*/
/****************************************************************/
/* File Name : animate.c					*/
/* Author : Sreerama K. Murthy					*/
/* Last modified : July 1994					*/
/* Contains modules : 	main					*/
/*			set_extremes				*/
/*			display_point				*/
/*			display_edge				*/
/*			find_edge				*/
/*			prepare_ps				*/
/*			recursive_draw				*/
/*			finish_ps				*/
/*			make_box				*/
/*			intersection				*/
/*			onedge					*/
/*			correct_side				*/
/*			display_help				*/
/* Uses modules in : 	util.c					*/
/*			oc1.h					*/
/*			load_data.c				*/
/*			classify_util.c				*/
/* Is used by modules in :	None.				*/
/* Remarks       :	This file contains modules to display	*/
/*			datasets and/or decision trees, as 	*/
/*			PostScript(R) files.			*/
/****************************************************************/		
#include "oc1.h"

char *pname;
char point_file[LINESIZE];
char extra_label[LINESIZE];
char ps_file[LINESIZE];
char decision_tree[LINESIZE], train_data[LINESIZE];
int unlabeled=FALSE,no_of_samples,no_of_dimensions=0;
int no_of_categories=0,no_of_coeffs,normalize = FALSE;
int erase = TRUE,no_of_missing_values=0;
int verbose=FALSE;
int wait_time=1;

struct tree_node *extra_node;
struct point **train_points;
struct tree_node *box;
float xmax=1.0,xmin=0.0,ymax=1.0,ymin=0.0,xmargin,ymargin;
int pminx=72, pmaxx=540, pminy=72, pmaxy = 640;

FILE *psfile;

/************************************************************************/
/* Module name : main							*/ 
/* Functionality :	Accepts & interprets command line options for	*/
/*			the "display" command. Invokes the routines to	*/
/*			read/display data, read/display decision trees.	*/
/* Parameters :	argc,argv : See any C-reference manual.			*/
/* Returns : 	Nothing.						*/
/* Calls modules :	display_help					*/
/*			read_tree               			*/
/*			error (util.c)					*/
/*			load_points (load_data.c)			*/
/*			prepare_ps					*/
/*			set_extremes					*/
/*			display_point					*/
/*			make_box					*/
/*			recursive_draw					*/
/*			finish_ps					*/
/* Is called by modules :	None.					*/
/************************************************************************/
main(argc,argv)
     int argc;
     char *argv[];
{
  extern char *optarg;
  extern int optind;
  int c1,i,j;
  int leaf_count(),tree_depth();
  int load_points();
  char title[LINESIZE];
  struct tree_node *root,*cur_node,*read_tree();
  FILE *infile,*dtree;
  
  strcpy(point_file,"\0");
  strcpy(decision_tree,"\0");
  strcpy(ps_file,"\0");
  strcpy(title,"\0");
  
  pname = argv[0];
  while ((c1 = getopt (argc, argv, "d:D:eh:o:t:T:vw:x:X:y:Y:")) != EOF)
    switch (c1)
      {
      case 'd':   
	no_of_dimensions = atoi (optarg);
	          /* If a decision tree is supplied with the D option,
		     the no_of_dimensions in there override this value. */
	break;
      case 'D':   /*Decision Tree */
	strcpy(decision_tree,optarg);
	break;
	          /* A decision tree, when no point_file is specified,
		     is displayed in the unit square. [[0..1],[0..1]] */
      case 'e':   erase = FALSE; break;
      case 'h':   /*Header (title) of the display.*/
	strcpy(title,optarg);
	break;
      case 'o':   /* File into which the postscript output is written.
		     default = stdout */
	strcpy(ps_file,optarg);
	break;
      case 't':   /* File containing data points.*/
	if (strlen(point_file)) usage(pname);
	strcpy(point_file,optarg);
	break;
      case 'T':   /* File containing data points.*/
	if (strlen(point_file)) usage(pname);
	strcpy(point_file,optarg);
	break;
      case 'v':   verbose = TRUE; break;
      case 'w': /* Time before erasing a hyperplane, during animation.
		   arbitrary units! */
	wait_time = atoi(optarg); 
	if (wait_time < 1) wait_time = 1;
	break;
      case 'x':   xmin = atof(optarg); break;
      case 'X':   xmax = atof(optarg); break;
      case 'y':   ymin = atof(optarg); break;
      case 'Y':   ymax = atof(optarg); break;
      default:    usage(pname);
      }

  if (!strlen(decision_tree) && !strlen(point_file)) usage(pname);
  if (xmax <= xmin || ymax <= ymin) usage(pname);
  
  if (!strlen(title)) 
    {
      if (strlen(point_file)) strcat(title,point_file);
      if (strlen(decision_tree))
	{ strcat(title,"-"); strcat(title,decision_tree);}
    }
  
  if (strlen(point_file))
    {
      if ((infile = fopen(point_file,"r")) == NULL)
	error("Display-Main : data file can not be opened. ");
      no_of_samples = load_points(infile,&train_points);
      if (verbose)
	{ 
	  printf("%d examples loaded from %s.\n", no_of_samples,point_file);
	  printf("Attributes = %d, Classes = %d\n",no_of_dimensions,
		  no_of_categories);
	  if (no_of_missing_values) 
	    printf("%d missing values filled with respective attribute means.\n",
		   no_of_missing_values);
	}
      if (no_of_dimensions != 2) 
	error("Display-Main : Only planar datasets can be displayed. ");
     no_of_coeffs = no_of_dimensions+1;
  
 }

  if (!strlen(ps_file)) psfile = stdout;
  else if ((psfile = fopen(ps_file,"w")) == NULL)
    {
      fprintf(stderr,"Display-Main : Output file can not be opened.");
      psfile=stdout;
    }

  prepare_ps(psfile,title);
  set_extremes(psfile);
  make_box();

  if (no_of_samples != 0)
    for (i=1;i<=no_of_samples;i++)
      display_point(psfile,train_points[i]);
  
  if (strlen(decision_tree))
    {
      if ((dtree = fopen(decision_tree,"r")) == NULL)
	error ("Display-Main: Decision Tree file can not be opened.");
      root = read_tree(dtree);
      if (verbose) 
	{
	  printf("Decision tree read from %s.\n",decision_tree);
	  printf("Leaf Count = %d, Tree Depth = %d\n",
		  leaf_count(root),tree_depth(root));
	}
      fclose(dtree);
    }
  
  finish_ps(psfile);
}

/************************************************************************/
/* Module name : set_extremes						*/ 
/* Functionality : 	Sets the global variables xmin,xmax,ymin and 	*/
/*			ymax as the minimum and maximum values in the	*/
/*			x and y coordinates. These values are 		*/
/*			subsequently used for scaling.			*/
/* Parameters :	None.							*/
/* Returns :	Nothing.						*/
/* Calls modules : None.						*/
/* Is called by modules :	main					*/
/************************************************************************/
set_extremes(psfile)
     FILE *psfile;
{
  int i;
  float x,y,x1,x2,y1,y2,xrange,yrange;
  
  if (no_of_samples == 0) return;
  
  xmax = xmin = train_points[1]->dimension[1];
  ymax = ymin = train_points[1]->dimension[2];
  
  for (i=1;i<=no_of_samples;i++)
    {
      x1 = train_points[i]->dimension[1];
      y1 = train_points[i]->dimension[2];
      if (xmax < x1) xmax = x1;
      if (xmin > x1) xmin = x1;
      if (ymax < y1) ymax = y1;
      if (ymin > y1) ymin = y1;
    }
  
  xrange = xmax-xmin;
  yrange = ymax-ymin;
  
  if ((xmargin = xrange * 0.01) == 0) xmargin = 1;
  if ((ymargin = yrange * 0.01) == 0) ymargin = 1;
  
  xmin -= xmargin; xmax += xmargin;
  ymin -= ymargin; ymax += ymargin;
  
}

/************************************************************************/
/* Module name :	display_point					*/ 
/* Functionality :	Displays the category number of a data point	*/
/*			at coordinates specified by its two attribute	*/
/*			values.						*/
/* Parameters :	psfile : File to write PostScript(R) output		*/
/*		p      : Pointer to the POINT structure			*/
/* Returns :	Nothing.						*/
/* Calls modules :	None.						*/
/* Is called by modules : main						*/
/* Important Variables used :	translatex, translatey : Inline function*/
/*				calls, defined in oc1.h, to scale 	*/
/*				coordinates.				*/
/************************************************************************/
display_point(psfile,p)
     FILE *psfile;
     POINT *p;
{
  float x1,y1;
  
  x1 = translatex(p->dimension[1]);
  y1 = translatey(p->dimension[2]);
  
  fprintf(psfile,"%f %f moveto (%d) show stroke\n",
	  x1,y1,p->category);
}


/************************************************************************/
/* Module name : draw_hyperplane					*/
/* Functionality :	Draws a node of the decision tree.	        */
/*			ie., Determines the edge corresponding to the	*/
/*			hyperplane at the current node, displays it,	*/
/* Parameters :	psfile : File to write PostScript(R) output		*/
/*		cur_node : Pointer to anode in the decision tree.	*/
/* Returns :	Nothing.						*/
/* Calls modules :	find_edge					*/
/*			display_edge					*/
/* Is called by modules :	main					*/
/************************************************************************/
draw_hyperplane(psfile,cur_node,count)
     struct tree_node *cur_node;
     FILE *psfile;
     int count;
{
  EDGE l,find_edge();
  int i;
  
  if (cur_node == NULL) return;
  
  l = find_edge(cur_node);
  if (l.from.x == l.to.x && l.from.y == l.to.y) return;
  if (strlen(cur_node->label) == 0)
    display_edge(psfile,l, "Root",count,0);
  else display_edge(psfile,l, cur_node->label,count,0);
}

/************************************************************************/
/* Module name : prepare_ps						*/
/* Functionality :	Prepares the PostScript(R) file. i.e., draws	*/
/*			a bounding box, displays the title of the 	*/
/*			box, if any; sets the initial font etc.		*/
/* Parameters :	psfile : File to write PostScript(R) output		*/
/*		title  : Character string title for the display.	*/
/* Returns :	Nothing.						*/
/* Calls modules :	None.						*/
/* Is called by modules :	main					*/
/* Important Variables used :	pmaxx,pminx,pmaxy,pminy : coordinates	*/
/*				of the bounding box. Also used in 	*/
/*				scaling. 				*/
/************************************************************************/
prepare_ps (psfile,title)
     char *title;
     FILE *psfile;
{
  fprintf (psfile, "%%!\ngsave\n");
  fprintf (psfile, "0.5 setlinewidth\n");
  
  fprintf(psfile,"%d %d moveto %d %d lineto stroke\n", pminx,pminy,pmaxx,pminy);
  fprintf(psfile,"%d %d moveto %d %d lineto stroke\n", pmaxx,pminy,pmaxx,pmaxy);
  fprintf(psfile,"%d %d moveto %d %d lineto stroke\n", pmaxx,pmaxy,pminx,pmaxy);
  fprintf(psfile,"%d %d moveto %d %d lineto stroke\n", pminx,pmaxy,pminx,pminy);
  fprintf (psfile,"/Helvetica findfont 7 scalefont setfont\n");
  fprintf(psfile,"%d %d moveto (%s) show stroke\n",
          pminx+10,pmaxy,title);
}

/************************************************************************/
/* Module name :	finish_ps					*/ 
/* Functionality :	Completes the PostScript(R) file, and closes it.*/
/* Parameters :	psfile : File to write PostScript(R) output		*/
/* Returns :	Nothing.						*/
/* Calls modules :	None.						*/
/* Is called by modules :	main					*/
/************************************************************************/
finish_ps (psfile)
     FILE *psfile;
{
  int i;
  
  fprintf (psfile, "showpage\ngrestore\n");
  fclose (psfile);
}

/************************************************************************/
/* Module name :	find_edge					*/ 
/* Functionality :	Determines the edge (defined by two end points)	*/
/*			corresponding to the hyperplane at the current	*/
/*			node of the decision tree. 			*/
/* Parameters :	cur_node: Node of the decision tree under consideration	*/
/* Returns :	an EDGE structure					*/
/* Calls modules :	intersection					*/
/*			onedge						*/
/*			correct_side					*/
/*			error (util.c)					*/
/* Is called by modules :	recursive_draw				*/
/* Important Variables used :	box : Root of a tree of four nodes, 	*/
/*				each representing an edge of the 	*/
/*				bounding box. 				*/ 
/*				first, second : Boolean variables to	*/
/*				record whether the first (second) end	*/
/*				point of the edge has been found.	*/
/* Remarks :	Two main principles used in finding the edge 		*/
/*		corresponding to a hyperplane are			*/
/*		1. A valid endpoint is formed when the current hyperplane*/
/*		   intersects the hyperplanes at one of its ancestors	*/
/*		   or the bounding box.					*/
/*		2. A valid end point lies on the "correct" side of each	*/
/*		   of the hyperplanes on the path from the root to the	*/
/*		   current node's parent.				*/ 
/*		An underlying assumption is that the decision tree is 	*/
/*		being traversed in pre-order.				*/
/************************************************************************/
EDGE find_edge(cur_node)
     struct tree_node *cur_node;
{
  EDGE l;
  struct endpoint p,intersection();
  struct tree_node *cur_ancestor;
  int onedge(),correct_side(),first=FALSE,second = FALSE;
  
/*cur_ancestor = cur_node;
  while (cur_ancestor->parent != NULL) cur_ancestor = cur_ancestor->parent;
  cur_ancestor->parent = box;*/
  
  cur_ancestor = cur_node->parent;
  
  while (cur_ancestor != NULL)
    {
      p = intersection(cur_node->coefficients,cur_ancestor->coefficients);
      
      if ( p.x != HUGE && p.y != HUGE 
	  && onedge(p,cur_ancestor->edge)
	  && correct_side(p,cur_node))
	{
	  if (first == FALSE)
	    {
	      l.from = p;
	      first = TRUE;
	      cur_ancestor = cur_ancestor->parent;
	      continue;
	    }
	  
	  if (second == FALSE)
	    {
	      second = TRUE;
	      l.to = p;
	      cur_ancestor = cur_ancestor->parent;
	      break;
	    } 
	  
	}
      else
	cur_ancestor = cur_ancestor->parent;
    }
  
  if (first == FALSE || second == FALSE) 
    {
      l.from.x = l.to.x = 0;
      l.from.y = l.to.y = 0;
    }
  
/*cur_ancestor = cur_node;
  while (cur_ancestor->parent != box) cur_ancestor = cur_ancestor->parent;
  cur_ancestor->parent = NULL; */
  
  cur_node->edge = l;
  return(l);
}

/************************************************************************/
/* Module name : make_box						*/ 
/* Functionality :	Creates four hypothetical decision tree nodes	*/
/*			to contain the left, right, bottom and top	*/
/*			edges of the bounding box, and links them up	*/
/*			as a 4-level tree. Sets the global variable	*/
/*			"box" to point to the root of this tree.	*/
/* Parameters :	None.							*/
/* Returns :	Nothing.						*/
/* Calls modules :	None.						*/
/* Is called by modules :	main					*/
/* Remarks :	The "box" tree comes handy while trying to find the	*/
/*		the end points of the edge representing a hyperplane,	*/
/*		in the "find_edge" module.				*/
/************************************************************************/
make_box()
{
  struct tree_node *left,*right,*top,*bottom;
  
  left = (struct tree_node *)malloc(sizeof(struct tree_node));
  left->coefficients = vector(1,no_of_coeffs);
  left->coefficients[1]=1; 
  left->coefficients[2]=0; 
  left->coefficients[3]=-1.0*xmin;
  
  top = (struct tree_node *)malloc(sizeof(struct tree_node));
  top->coefficients = vector(1,no_of_coeffs);
  top->coefficients[1]=0; 
  top->coefficients[2]=1; 
  top->coefficients[3]=-1.0*ymax;
  
  right = (struct tree_node *)malloc(sizeof(struct tree_node));
  right->coefficients = vector(1,no_of_coeffs);
  right->coefficients[1]=1; 
  right->coefficients[2]=0; 
  right->coefficients[3]=-1.0*xmax;
  
  bottom = (struct tree_node *)malloc(sizeof(struct tree_node));
  bottom->coefficients = vector(1,no_of_coeffs);
  bottom->coefficients[1]=0; 
  bottom->coefficients[2]=1; 
  bottom->coefficients[3]=-1.0*ymin;
  
  (left->edge).from.x = (bottom->edge).to.x   = xmin; 
  (left->edge).from.y = (bottom->edge).to.y   = ymin;
  (left->edge).to.x   = (top->edge).from.x    = xmin;
  (left->edge).to.y   = (top->edge).from.y    = ymax; 
  (top->edge).to.x    = (right->edge).from.x  = xmax;
  (top->edge).to.y    = (right->edge).from.y  = ymax;
  (right->edge).to.x  = (bottom->edge).from.x = xmax;
  (right->edge).to.y  = (bottom->edge).from.y = ymin;
  
  bottom->parent = right;
  right->parent = top;
  top->parent = left;
  left->parent = NULL;
   
  strcpy(bottom->label,"X");
  strcpy(right ->label,"Y");
  strcpy(top   ->label,"X");
  strcpy(left  ->label,"Y");
  
  box = bottom;
}

/************************************************************************/
/* Module name :	display_edge					*/ 
/* Functionality :	Displays an edge, with an optional character	*/
/*			label.						*/
/* Parameters :	psfile : File to write PostScript(R) output		*/
/*		e      : Pointer to the EDGE structure			*/
/*		label  : Character label to be displayed at the centre 	*/
/*			 of the edge.					*/
/*              count  : serial number of this particular perturbation  */
/*              reverse: 0 - edge should be drawn in normal video       */
/*                       1 - reverse video (used for erasing).          */
/* Returns :	Nothing.						*/
/* Calls modules :	None.						*/
/* Is called by modules : recursive_draw				*/
/* Important Variables used :	translatex, translatey : Inline function*/
/*				calls, defined in oc1.h, to scale 	*/
/*				coordinates.				*/
/************************************************************************/
display_edge (psfile, e, label,count,reverse)
     FILE *psfile;
     EDGE e;
     char *label;
     int count,reverse;
{
  int i;
  struct endpoint p1, p2;
  double x,y;
  double angle;
  
  p1 = e.from;
  p2 = e.to;
  y = (double)(p2.y - p1.y);
  x = (double)(p2.x - p1.x);
  if (x == 0) 
    {
      if (y > 0) angle = 90;
      else angle = -90;
    }
  else
    {
      angle = atan2(y,x) * 180/ M_PI;
      if (angle > 90) angle -= 180;
      if (angle < -90) angle += 180;
    }
  if (reverse) fprintf(psfile,"1 setgray\n");
  fprintf (psfile, "%f %f moveto %f %f lineto stroke\n",
	   translatex (p1.x), translatey (p1.y),
	   translatex (p2.x), translatey (p2.y));
  if (strlen(label))
    {
      fprintf (psfile, "gsave %f %f moveto %f rotate 0 1 rmoveto ",
	       translatex ((p1.x + p2.x)/2), 
	       translatey ((p1.y + p2.y)/2), angle);
      if (erase == TRUE || reverse == TRUE)
	fprintf (psfile, "(%s) show stroke grestore\n", label);
      else fprintf (psfile, "(%s-%d) show stroke grestore\n",
		    label,count);
    }
  if (reverse) fprintf(psfile,"0 setgray\n");
}

/************************************************************************/
/* Module name : Erase Hyperplane                                       */
/* Functionality : While showing the animation of tree induction, if    */
/*                 the -e option is chosen, each hyperplane is erased   */
/*                 before the next hyperplane is displayed. This module */
/*                 erases a given hyperplane.                           */
/* Parameters :  psfile : Postscript file pointer.                      */
/*               cur_node : Tree node containing current hyperplane.    */
/*               count: serial number of the hyperplane being considered*/
/*                      e.g: root-1,root-2,... etc                      */
/* Returns : Nothing.                                                   */
/* Calls modules : display_edge                                         */
/*                 display_point                                        */
/* Is called by modules :                                               */
/************************************************************************/
erase_hyperplane(psfile,cur_node,count)
     FILE *psfile;
     struct tree_node *cur_node;
     int count;
{
  int i;
  float d;
  struct tree_node *node;
  
  if (!strlen(cur_node->label))
    display_edge(psfile,cur_node->edge,"Root",count,1);
  else display_edge(psfile,cur_node->edge,cur_node->label,count,1);
  
  /* Redraw points near the erased line. */
  for (i=1;i<=no_of_samples;i++)
    {
      d = fabs (cur_node->coefficients[1] * train_points[i]->dimension[1] +
		cur_node->coefficients[2] * train_points[i]->dimension[2] +
		cur_node->coefficients[3]) /
		  sqrt (cur_node->coefficients[1] * 
                        cur_node->coefficients[1] + 
                        cur_node->coefficients[2] * 
                        cur_node->coefficients[2]);
      if (d < 4 * ((xmax - xmin) / (pmaxx - pminx) +
		   (ymax - ymin) / (pmaxy - pminy)))
	display_point(psfile,train_points[i]);
    }

  /*Redraw all hyperplanes that are ancestors of the current one. */
  node = cur_node->parent;
  while (node != NULL)
    {
      if (!strlen(node->label))
	display_edge(psfile,node->edge,"Root",0,0);
      else display_edge(psfile,node->edge,node->label,0,0);
      node = node->parent;
    }
}

/************************************************************************/
/* Module name : intersection						*/ 
/* Functionality :	Computes the intersection point of two 		*/
/*			hyperplanes.					*/
/* Parameters :	c1,c2 : two coefficient arrays, each of length 3.	*/
/* Returns :	the intersection point, stored in an "endpoint" struct.	*/
/*		(HUGE,HUGE) if there is no intersection.		*/
/* Calls modules :	None.						*/
/* Is called by modules :	find_edge				*/
/************************************************************************/
struct endpoint intersection(c1,c2)
     float *c1,*c2;
{
  float denom;
  struct endpoint p;
    
  denom = c2[2] * c1[1] - c2[1] * c1[2];
  if (!denom)
    { p.x = HUGE; p.y = HUGE; return(p);}
  
  p.x = (c2[3] * c1[2] - c2[2] * c1[3]) / denom;
  p.y = (c2[1] * c1[3] - c2[3] * c1[1]) / denom;
  return(p);
}

/************************************************************************/
/* Module name : onedge							*/ 
/* Functionality :	Checks if a point lies on an edge.		*/
/* Parameters :	p : pointer to a POINT structure.			*/
/*		e : pointer to an EDGE structure.			*/
/* Returns :	1: if p lies on e					*/
/*		0: otherwise.						*/
/* Calls modules :	None.						*/
/* Is called by modules : find_edge					*/
/************************************************************************/
int onedge(p,e)
     struct endpoint p;
     EDGE e;
{
  struct endpoint p1,p2;
  
  p1=e.from;
  p2=e.to;
  
  if (((p1.x >= p.x && p2.x <= p.x) ||
       (p1.x <= p.x && p2.x >= p.x)) &&
      ((p1.y >= p.y && p2.y <= p.y) ||
       (p1.y <= p.y && p2.y >= p.y))) return(TRUE);
  return(FALSE);
}

/************************************************************************/
/* Module name : correct_side						*/ 
/* Functionality :	Checks if a point lies on the "correct" side of	*/
/*			the hyperplanes at all nodes on the path from	*/
/*			the root of the decision tree to the parent of	*/
/*			the node under consideration. 			*/
/*			ie., the following should hold recursively: if 	*/
/*			"cur_node" is the right child of "cur_node->	*/
/*			parent", then the point should lie on the right	*/
/*			side of the hyperplane at "cur_node->parent".	*/
/*			Same for the left child.			*/
/*			This test makes sure that we draw each hyperplane*/
/*			in the appropriate region.			*/
/* Parameters :	p : pointer to an "endpoint" structure.			*/
/*		    Is an intersection point of the hyperplane at	*/
/*		    cur_node with that at one of its ancestors, or with	*/
/*		    the bounding box.					*/
/*		cur_node : Decision tree node under consideration.	*/
/* Returns :	1: If p is on the correct side of all of cur_node's	*/
/*		   ancestors.						*/
/*		0: otherwise.						*/
/* Calls modules :	None.						*/
/* Is called by modules : find_edge					*/
/* Important Variables used : TOLERANCE : A very small positive number	*/
/*			      defined in oc1.h, needed to compensate	*/
/*			      for peculiarities in floating point	*/
/*			      arithmetic on Unix.			*/
/* Remarks : This assumes that the decision tree is displayed in a	*/
/*	     pre-order traversal.					*/
/************************************************************************/
int correct_side(p,cur_node)
     struct endpoint p;
     struct tree_node *cur_node;
{
  float sum;
  struct tree_node *node,*parent;
  
  node = cur_node;
  
  while ((parent = node->parent) != box)
    {
      sum = 0;
      
      sum =   parent->coefficients[1] * p.x
	    + parent->coefficients[2] * p.y
	    + parent->coefficients[3];
      
      if ((sum <= TOLERANCE && sum >= -1.0 * TOLERANCE) || 
	  (node == parent->left && sum < 0) ||
	  (node == parent->right && sum > 0))
	node = parent; 
      else break;
    }
  
  if (parent == box) return (TRUE);
  else return(FALSE);
}

/************************************************************************/
/* Module name :	read_tree					*/ 
/* Functionality : High level routine for reading in a decision tree	*/
/* Parameters :	dtree : decision tree file pointer              	*/
/* Returns :	pointer to the root node of the tree.			*/
/* Calls modules :	read_subtree					*/
/*			read_header					*/
/*			read_hp						*/
/*			error (util.c)					*/
/* Is called by modules :	main (mktree.c)				*/
/*				main (gen_data.c)			*/
/* Remarks : 	It is assumed that the file "decision_tree" is		*/
/* 		written in a format similar to the output of the	*/
/*		write_tree module (tree_util.c). A sample decision	*/
/*		tree is available in sample.dt.				*/ 
/************************************************************************/
struct tree_node *read_tree(dtree)
     FILE *dtree;
{
  struct tree_node *root,*cur_node,*read_hp(),*read_perturbations();
  char *read_label();
  int read_header();
  
  if (!read_header(dtree)) 
    error("Read_Tree: Decision tree invalid/absent.");

  if (no_of_samples == 0)
    {
      if (verbose)
	printf("Attributes = %d, Classes = %d\n",no_of_dimensions,
	       no_of_categories);
      if (no_of_dimensions != 2) 
	error("Read_Tree: Only planar trees can be displayed. ");
      no_of_coeffs = no_of_dimensions+1;
  }
  
  strcpy(extra_label,read_label(dtree));
  if ((root = read_hp(dtree)) == NULL)
    error("Read_Tree: Decision tree invalid/absent.");

/*root->parent = NULL;*/
  root->parent = box ;
  draw_hyperplane(psfile,root,1);
  if (!strcmp(root->label,extra_label))
    root = read_perturbations(root,dtree);
/*root->parent = NULL;*/
  root->parent = box ;
  
  extra_node = NULL;
  read_subtree(root,dtree);
  
  return(root);
}


/************************************************************************/
/* Module name :	read_subtree					*/ 
/* Functionality :	recursively reads in the hyperplane, left 	*/
/*			subtree and the right subtree at a node of 	*/
/*			the decision tree. 				*/
/* Parameters :	root : node, the subtree at which is to be read.	*/
/*		dtree: file pointer where the tree is available.	*/
/* Returns :	nothing.						*/
/* Calls modules :	read_subtree					*/
/*			read_hp			         		*/
/*			isleftchild					*/
/*			isrightchild					*/
/* Is called by modules :	read_tree				*/
/*				read_subtree				*/
/* Important Variables Used :	extra_node 				*/
/*	Hyperplanes are read from the file "dtree" in the order "parent,*/
/*	left child, right child". In case a node does not have either a	*/
/* 	left child or a right child or both, this routine reads one 	*/
/*	hyperplane before it is needed. Such hyperplanes, that are read	*/
/*	before they are needed, are stored in extra_node.		*/
/************************************************************************/
read_subtree(root,dtree)
     struct tree_node *root;
     FILE *dtree;
{
  struct tree_node *cur_node,*read_hp(),*read_perturbations();
  int isleftchild(),isrightchild();
  
  if (extra_node != NULL)
    {
      cur_node = extra_node;
      extra_node = NULL;
    }
  else cur_node = read_hp(dtree);
  
  if (cur_node == NULL) return;
  
  if (isleftchild(cur_node,root))
    {
      cur_node->parent = root;
      root->left = cur_node;
      draw_hyperplane(psfile,cur_node,1);
      if (!strcmp(cur_node->label,extra_label))
	{
	  cur_node = read_perturbations(cur_node,dtree);
	  cur_node->parent = root;
	  root->left = cur_node;
	}
      read_subtree(cur_node,dtree);
      /* Read the next node */
      if (extra_node != NULL)
	{
	  cur_node = extra_node;
	  extra_node = NULL;
	}
      else
	cur_node = read_hp(dtree);
      if (cur_node == NULL) return;
    }

  if (isrightchild(cur_node,root))
    {
      cur_node->parent = root;
      root->right = cur_node;
      draw_hyperplane(psfile,cur_node,1);
      if (!strcmp(cur_node->label,extra_label))
	{
	  cur_node = read_perturbations(cur_node,dtree);
	  cur_node->parent = root;
	  root->right = cur_node;
	}
      read_subtree(cur_node,dtree);
    }
  else 
    extra_node = cur_node;
  
}

/************************************************************************/
/* Module name : read_perturbations                                     */
/* Functionality : Used to read the perturbations of a single hyperplane*/
/*                 for showing its animation.                           */
/* Parameters : cur_node : tree node under consideration.               */
/*              dtree : file pointer to the decision tree file.         */
/* Returns : the next hyperplane in the tree - the one after the        */
/*           one for which all the perturbations are read in this module*/
/* Calls modules : erase_hyperplane                                     */
/* Is called by modules :                                               */
/************************************************************************/
struct tree_node *read_perturbations(cur_node,dtree)
     struct tree_node *cur_node;
     FILE *dtree;
{
  struct tree_node *read_hp(),*next_node;
  char cur_label[LINESIZE];
  int count=1;

  next_node = read_hp(dtree);
  while (next_node != NULL)
    {
      if (erase == TRUE) 
        {
          psdelay(psfile,wait_time);
          erase_hyperplane(psfile,cur_node,count);
        }

      count++;
/*    if (cur_node->parent != NULL)*/
      if (cur_node->parent != box)
	{
	  if ((cur_node->parent)->left == cur_node)
	    cur_node->parent->left = next_node;
	  else
	    cur_node->parent->right = next_node;
	}
      next_node->parent = cur_node->parent;
      
      draw_hyperplane(psfile,next_node,count); 
      cur_node = next_node;
      if (!strcmp(cur_node->label,extra_label))
	next_node = read_hp(dtree);
      else next_node = NULL;
    }
 
  return(cur_node);
}



psdelay(out,t)
FILE *out;
int t;
{
  fprintf(out, "0 1 %d 1 {add} for pop\n", t);
}

/************************************************************************/
/* Module name : Read_Label                                             */
/* Functionality : Reads the label of a hyperplane.                     */
/* Parameters : dtree : File pointer of the input file.                 */
/* Returns : character label                                            */
/* Calls modules : None.                                                */
/************************************************************************/
char *read_label(dtree)
     FILE *dtree;
{
  char c,label[LINESIZE];
  
  while (isspace(c = getc(dtree)));
  ungetc(c,dtree); 
  if (fscanf(dtree,"%[^' '] Hyperplane:",label) != 1)
    return("NONE");
  
  if (!strcmp(label,"Root")) strcpy(label,"");
  return(label);
}

/************************************************************************/
/* Module name : read_hp						*/
/* Functionality :	Reads a hyperplane (one node of the decision	*/
/*			tree).						*/
/* Parameters :	dtree : file pointer to the decision tree file.		*/
/* Returns : pointer to the decision tree node read.			*/
/* Calls modules :	vector (util.c)					*/
/*			error (util.c)					*/
/* Is called by modules :	read_tree				*/
/*				read_subtree				*/
/* Remarks :	Rather strict adherance to format.			*/
/*		Please carefully follow the format in sample.dt, if	*/
/*		your decision tree files are not produced by "mktree".	*/
/************************************************************************/
struct tree_node *read_hp(dtree)
     FILE *dtree;
{
  struct tree_node *cur_node;
  float temp;
  char c,*read_label();
  int i;
  
  cur_node = (struct tree_node *)malloc(sizeof(struct tree_node));
  cur_node->coefficients = vector(1,no_of_coeffs);
  cur_node->left_count = ivector(1,no_of_categories);
  cur_node->right_count = ivector(1,no_of_categories);
  
  for (i=1;i<=no_of_coeffs;i++) cur_node->coefficients[i] = 0;
  
  cur_node->left = cur_node->right = NULL;
  
  while (isspace(c = getc(dtree)));
  ungetc(c,dtree); 
  
  if (fscanf(dtree, "Left = [") != 0)
    return(NULL);
  
  for (i=1;i<no_of_categories;i++)
    if (fscanf(dtree,"%d,",&cur_node->left_count[i]) != 1)
      return(NULL); 
  if (fscanf(dtree,"%d], Right = [",
	     &cur_node->left_count[no_of_categories]) != 1)
    return(NULL); 
  for (i=1;i<no_of_categories;i++)
    if (fscanf(dtree,"%d,",&cur_node->right_count[i]) != 1)
      return(NULL); 
  if (fscanf(dtree,"%d]\n", &cur_node->right_count[no_of_categories]) != 1)
    return(NULL); 


  while (TRUE)
    {
      if ((fscanf(dtree,"%f %c",&temp,&c)) != 2)
	error("Read_Hp: Invalid/Absent hyperplane equation.");
      if (c == 'x')
	{ 
	  if ((fscanf(dtree,"[%d] +",&i)) != 1) 
	    error("Read_Hp: Invalid hyperplane equation.");
	  if (i <= 0 || i > no_of_coeffs) 
	    error("Read_Hp: Invalid coefficient index in decision tree.");
	  cur_node->coefficients[i] = temp;
	}
      else if (c == '=')
	{
	  fscanf(dtree," 0\n\n");
	  cur_node->coefficients[no_of_coeffs] = temp;
	  break;
	}
    }


  cur_node->no_of_points = 0;
  cur_node->left_cat = cur_node->right_cat = 1;
  for (i=1;i<=no_of_categories;i++)
    {
      cur_node->no_of_points += cur_node->left_count[i] + 
	cur_node->right_count[i];
      if (cur_node->left_count[i] > cur_node->left_count[cur_node->left_cat])
	cur_node->left_cat = i;
      if (cur_node->right_count[i] > cur_node->right_count[cur_node->right_cat])
	cur_node->right_cat = i;
    }

  strcpy(cur_node->label,extra_label);
  strcpy(extra_label,read_label(dtree));
  
  return(cur_node);
}


/************************************************************************/
/* Module name : isleftchild						*/
/* Functionality : 	Checks if node x is a left child of node y.	*/
/*			i.e., checks if the label of node x is the same	*/
/*			as label of y, concatenated with "l".		*/
/* Parameters : x,y : pointers to two decision tree nodes.		*/
/* Returns :	1 : if x is the left child of y				*/
/*		0 : otherwise						*/
/* Calls modules :	none.						*/
/* Is called by modules :	read_subtree				*/
/************************************************************************/
int isleftchild(x,y)
     struct tree_node *x,*y;
{
  char temp[MAX_DT_DEPTH];
  
  strcpy(temp,y->label);
  if (!strcmp(strcat(temp,"l"),x->label)) return(1);
  else return(0);
}

/************************************************************************/
/* Module name : isrightchild						*/
/* Functionality : 	Checks if node x is a right child of node y.	*/
/*			i.e., checks if the label of node x is the same	*/
/*			as label of y, concatenated with "l".		*/
/* Parameters : x,y : pointers to two decision tree nodes.		*/
/* Returns :	1 : if x is the right child of y			*/
/*		0 : otherwise						*/
/* Calls modules :	none.						*/
/* Is called by modules :	read_subtree				*/
/************************************************************************/
int isrightchild(x,y)
     struct tree_node *x,*y;
{
  char temp[MAX_DT_DEPTH];

  strcpy(temp,y->label);
  if (!strcmp(strcat(temp,"r"),x->label)) return(1);
  else return(0);
}

/************************************************************************/
/* Module name : read_header						*/
/* Functionality :	Reads the header information in a decision tree	*/
/*			file.						*/
/* Parameters :	dtree : file pointer to the decision tree file.		*/
/* Returns : 	1 : if the header is successfully read.			*/
/*		0 : otherwise.						*/
/* Calls modules : none.						*/
/* Is called by modules :	read_tree				*/
/* Remarks :	Rather strict adherance to format.			*/
/*		Please carefully follow the format in sample.dt, if	*/
/*		your decision tree files are not produced by "mktree".	*/
/************************************************************************/
int read_header(dtree)
     FILE *dtree;
{
  if ((fscanf(dtree,"Training set: %s ",train_data)) != 1) return(0);
  if ((fscanf(dtree,"Dimensions: %d, Categories: %d\n",
	      &no_of_dimensions,&no_of_categories)) != 2) return(0);
  return(1);
}

/************************************************************************/
/* Module name : leaf_count						*/
/* Functionality :      Calculate the number of leaves of the decision  */
/*                      tree. Each branch of the decision tree ends in	*/
/*			a leaf.						*/
/* Parameters : cur_node :      pointer to the root of the subtree whose*/
/*                              leaves are to be counted.               */
/* Returns :    number of leaves of the subtree pointed to by "cur_node"*/
/* Calls modules :      leaf_count                                 	*/
/* Is called by modules :       estimate_accuracy (classify.c)          */
/*				main (display.c)			*/
/************************************************************************/
int leaf_count(cur_node)
     struct tree_node *cur_node;
{
  int leaf_count();
  
  if (cur_node == NULL) return(1);
  else return( leaf_count(cur_node->left)+leaf_count(cur_node->right));
  
}

/************************************************************************/
/* Module name : tree_depth                                             */
/* Functionality :      Calculate the maximum depth of any node in a    */
/*                      decision tree.					*/
/*                      Depth of a node is its distance (in terms of the*/
/*                      number of intermediate nodes) from the root.    */
/* Parameters : cur_node :      pointer to the root of the subtree whose*/
/*                              leaves are to be counted.               */
/* Returns :    max depth of any node in the subtree pointed to by      */
/*              "cur_node"                                              */
/* Calls modules :      tree_depth                                      */
/* Is called by modules :       estimate_accuracy (classify.c)          */
/*				main (display.c)			*/
/************************************************************************/
int tree_depth(cur_node)
     struct tree_node *cur_node;
{
  int left_depth, right_depth, tree_depth();
  
  if (cur_node == NULL) return(0);
  left_depth = tree_depth(cur_node->left);
  right_depth = tree_depth(cur_node->right);
  if (left_depth >= right_depth) return(left_depth+1);
  return(right_depth+1);
}
 
/************************************************************************/
/************************************************************************/
