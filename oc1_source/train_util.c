
/****************************************************************/
/* Copyright 1993 : Johns Hopkins University			*/
/*                  Department of Computer Science		*/
/****************************************************************/
/* Contact : murthy@cs.jhu.edu					*/
/****************************************************************/
/* File Name : train_util.c					*/
/* Author : Sreerama K. Murthy					*/
/* Last modified : October 1993					*/
/* Contains modules :	generate_random_hyperplane		*/
/*			write_tree				*/
/*			write_subtree				*/
/*			write_hp				*/
/*			write_header				*/
/* Uses modules in :	oc1.h					*/
/*			util.c					*/
/* Is used by modules in :	mktree.c			*/
/*				perturb.c			*/
/****************************************************************/		

#include "oc1.h"

struct tree_node *extra_node;
extern int no_of_categories,no_of_coeffs;

/************************************************************************/
/* Module name :	generate_random_hyperplane			*/ 
/* Functionality :	generates coefficients of a hyperplane randomly.*/
/* Parameters :	coefficients : Array into which the random coefficients	*/
/*		are to be stored.					*/
/* Returns :	Nothing.						*/
/* Calls modules :	myrandom (util.c)				*/
/* Is called by modules :	oblique_split (mktree.c)		*/
/*				perturb_randomly (perturb.c)		*/
/* Important Variables used :	MAX_COEFFICIENT : Maximum value a 	*/
/*				coefficient can initially have. Declared*/
/*				in oc1.h.				*/
/************************************************************************/
generate_random_hyperplane(coefficients)
float *coefficients;
{
  int i;
  float myrandom();
 
  for (i=1;i<=no_of_coeffs;i++)
     coefficients[i] = myrandom(-MAX_COEFFICIENT, MAX_COEFFICIENT);
}
 

/************************************************************************/
/* Module name : write_tree						*/
/* Functionality :	High level routine to write a decision tree to 	*/
/*			a file.						*/
/* Parameters :	root : pointer to the structure containing the root of	*/
/*		       decision tree.					*/
/*		dt_file : Name of the file into which the dt is to be	*/
/*		       stored.						*/
/* Returns :	Nothing.						*/
/* Calls modules : 	error (util.c)					*/
/*			write_header					*/
/*			write_subtree					*/
/* Is called by modules :	main (mktree.c)				*/
/*				cross_validate (mktree.c)		*/
/* Remarks :	The input/output formats are rather strict at this	*/
/*		stage for OC1. The read_tree routines in classify_util.c*/
/*		produce run time errors while reading decision trees	*/
/*		even if there is the slightest deviation from the	*/
/*		format outputted by the "write_*" routines in this file.*/
/*		Hopefully this will be more flexible in future !	*/
/************************************************************************/
void write_tree(root,dt_file)
struct tree_node *root;
char *dt_file;
{
 void write_subtree();
 FILE *dtree;

 if ((dtree = fopen(dt_file,"w")) == NULL)
  error("Decision Tree file can not be opened.");
 
 write_header(dtree);

 write_subtree(root,dtree);

 fclose(dtree);
}


/************************************************************************/
/* Module name :	write_subtree					*/ 
/* Functionality : 	Initiates writing a hyperplane, and recursively	*/
/*			writes the subtrees on the left and right of the*/
/*			hyperplane.					*/
/* Parameters :	cur_node : Pointer to the DT node under consideration.	*/
/*		dtree : File pointer to the output file.		*/
/* Returns :	Nothing.						*/
/* Calls modules :	write_subtree					*/
/*			write_hp					*/
/* Is called by modules :	write_subtree				*/
/*				write_tree				*/
/************************************************************************/
void write_subtree(cur_node,dtree)
struct tree_node *cur_node;
FILE *dtree;
{
 void write_hp(),write_subtree();

 if (cur_node == NULL) return;

 write_hp(cur_node,dtree);
 write_subtree(cur_node->left,dtree);
 write_subtree(cur_node->right,dtree);
}

/************************************************************************/
/* Module name :	write_hp					*/ 
/* Functionality : Writes one hyperplane.				*/
/* Parameters :	cur_node : Pointer to the DT node under consideration.	*/
/*		dtree : File pointer to the output file.		*/
/* Returns :	Nothing.						*/
/* Calls modules :	None.						*/
/* Is called by modules :	write_subtree				*/
/* Important Variables used :	cur_node->label : Label is the empty	*/
/*				string for the root node. If string x	*/
/*				is the label of a node, xl is the label	*/
/*				of its left child, and xr of its right	*/
/*				child. Label plays an important role,	*/
/*				because it informs the tree reading	*/
/*				routines (in classify_util.c) about the	*/
/*				structure of the DT.			*/
/************************************************************************/
void write_hp(cur_node,dtree)
struct tree_node *cur_node;
FILE *dtree;
{
  int i;

  if (strcmp(cur_node->label,"\0") == 0)
    fprintf(dtree, "Root Hyperplane: ");
  else
    fprintf(dtree, "%s Hyperplane: ",cur_node->label);

  fprintf(dtree,"Left = [");
  for (i=1;i<=no_of_categories;i++)
    if (i == no_of_categories) fprintf(dtree,"%d], ",cur_node->left_count[i]);
    else fprintf(dtree,"%d,",cur_node->left_count[i]);
  fprintf(dtree,"Right = [");
  for (i=1;i<=no_of_categories;i++)
    if (i == no_of_categories) fprintf(dtree,"%d]\n",cur_node->right_count[i]);
    else fprintf(dtree,"%d,",cur_node->right_count[i]);


  for (i=1;i<=no_of_coeffs;i++)
     if (cur_node->coefficients[i])
       {
         if (i < no_of_coeffs)
            fprintf(dtree,"%f x[%d] + ",cur_node->coefficients[i],i);
         else
            fprintf(dtree,"%f = 0\n\n",cur_node->coefficients[i]);
       }
}


/************************************************************************/
/* Module name :	write_header					*/ 
/* Functionality :	Writes the decision tree header.		*/
/* Parameters :	dtree : file pointer to the output file.		*/
/* Returns :	Nothing.						*/
/* Calls modules :	None.						*/	
/* Is called by modules :	write_tree				*/
/************************************************************************/
write_header(dtree)
FILE *dtree;
{
extern int no_of_dimensions,no_of_categories;
extern int no_of_iterations,cycle_count;
extern int order_of_improvement;
extern char train_data[LINESIZE];

 fprintf(dtree,"Training set: %s, ",train_data);
 fprintf(dtree,"Dimensions: %d, Categories: %d\n",
         no_of_dimensions,no_of_categories);
 fprintf(dtree,"Program Parameters:\n\t Iter: %d, ",no_of_iterations);
 switch (order_of_improvement)
  {
   case SEQUENTIAL: fprintf(dtree,"Order: Sequential\n"); break;
   case BEST_FIRST: fprintf(dtree,"Order: Best\n"); break;
   case RANDOM: fprintf(dtree,"Order: Random%d\n",cycle_count); break;
  }

 fprintf(dtree,"\n\n");
}

/************************************************************************/
/************************************************************************/
