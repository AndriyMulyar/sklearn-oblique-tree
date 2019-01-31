/****************************************************************/
/* Copyright 1993, 1994                                         */
/* Johns Hopkins University			                */
/* Department of Computer Science		                */
/****************************************************************/
/* Contact : murthy@cs.jhu.edu					*/
/****************************************************************/
/* File Name :	tree_util.c					*/
/* Author : Sreerama K. Murthy					*/
/* Last modified : July 1994					*/
/* Contains modules : 	read_tree				*/
/*			read_subtree				*/
/*			read_hp					*/
/*			read_header				*/
/*			write_tree				*/
/*			write_subtree				*/
/*			write_hp       				*/
/*			write_header				*/
/*			isleftchild				*/
/*			isrightchild				*/
/*			leaf_count				*/
/*			tree_depth				*/
/* Uses modules in :	oc1.h					*/
/*			util.c					*/ 
/* Is used by modules in :	mktree.c			*/
/* Remarks       : 	These routines are mainly used to read	*/
/*			a decision tree from a file, and to     */
/*                      write a tree to a file.			*/
/****************************************************************/		

#include "oc1.h"

extern int no_of_dimensions, no_of_categories;

struct tree_node *extra_node;
char train_data[LINESIZE];

/************************************************************************/
/* Module name :	read_tree					*/ 
/* Functionality : High level routine for reading in a decision tree	*/
/* Parameters :	decision_tree : Name of the file in which the tree is	*/
/*		stored.							*/
/* Returns :	pointer to the root node of the tree.			*/
/* Calls modules :	read_subtree					*/
/*			read_header					*/
/*			read_hp						*/
/*			error (util.c)					*/
/* Is called by modules :	main (mktree.c)				*/
/*				main (gen_data.c)			*/
/* Remarks : 	It is assumed that the file "decision_tree" is		*/
/* 		written in a format similar to the output of the	*/
/*		write_tree module. A sample decision tree is given in   */
/*              the file sample.dt.				        */ 
/************************************************************************/
struct tree_node *read_tree(decision_tree)
     char *decision_tree;
{
  FILE *dtree;
  struct tree_node *root,*cur_node,*read_hp();
  int read_header();
  
  if ((dtree = fopen(decision_tree,"r")) == NULL)
    error ("Decision Tree file can not be opened.");
  
  if ( !(read_header(dtree))) 
    error("Decision tree invalid/absent.");
  
  if ((root = read_hp(dtree)) == NULL)
    error("Decision tree invalid/absent.");
  
  root->parent = NULL;
  extra_node = NULL;
  read_subtree(root,dtree);
  
  fclose(dtree);
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
/*			read_hp						*/
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
  struct tree_node *cur_node,*read_hp();
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
      
      read_subtree(cur_node,dtree);
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
      read_subtree(cur_node,dtree);
    }
  else extra_node = cur_node;
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
  char c;
  int i;

  cur_node = (struct tree_node *)malloc(sizeof(struct tree_node));
  cur_node->coefficients = vector(1,no_of_dimensions+1);
  cur_node->left_count = ivector(1,no_of_categories);
  cur_node->right_count = ivector(1,no_of_categories);
  
  for (i=1;i<=no_of_dimensions+1;i++) cur_node->coefficients[i] = 0;
  
  cur_node->left = cur_node->right = NULL;
  
  while (isspace(c = getc(dtree)));
  ungetc(c,dtree); 
  
  if (fscanf(dtree,"%[^' '] Hyperplane: Left = [", cur_node->label) != 1)
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

  if (!strcmp(cur_node->label,"Root")) strcpy(cur_node->label,"");
  
  while (TRUE)
    {
      if ((fscanf(dtree,"%f %c",&temp,&c)) != 2)
	error("Invalid/Absent hyperplane equation.");
      if (c == 'x')
	{ 
	  if ((fscanf(dtree,"[%d] +",&i)) != 1) 
	    error("Read-Hp: Invalid hyperplane equation.");
	  if (i <= 0 || i > no_of_dimensions+1) 
	    error("Read_Hp: Invalid coefficient index in decision tree.");
	  cur_node->coefficients[i] = temp;
	}
      else if (c == '=')
	{
	  fscanf(dtree," 0\n\n");
	  cur_node->coefficients[no_of_dimensions+1] = temp;
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
  if ((fscanf(dtree,"Training set: %[^,], ",train_data)) != 1) return(0);
  if ((fscanf(dtree,"Dimensions: %d, Categories: %d\n",
	      &no_of_dimensions,&no_of_categories)) != 2) return(0);
  return(1);
}

/************************************************************************/
/* Module name : leaf_count						*/
/* Functionality :      Calculates the number of leaves of a subtree.   */
/* Parameters : cur_node :      pointer to the root of the subtree whose*/
/*                              leaves are to be counted.               */
/* Returns :    number of leaves of the subtree pointed to by "cur_node"*/
/* Calls modules :      leaf_count                                 	*/
/* Is called by modules :       estimate_accuracy (classify.c)		*/
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
/* Calls modules :      tree_depth                                      */
/* Is called by modules :       estimate_accuracy (classify.c)		*/
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
/*		stage for OC1. The read_tree routines                   */
/*		produce run time errors while reading decision trees	*/
/*		even if there is the slightest deviation from the	*/
/*		format outputted by the "write_*" routines.             */
/************************************************************************/
write_tree(root,dt_file)
     struct tree_node *root;
     char *dt_file;
{
  FILE *dtree;

  if ((dtree = fopen(dt_file,"w")) == NULL)
    error("Write_Tree: Decision Tree file can not be opened.");

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
/* Calls modules :	write_subtree					*/
/*			write_hp					*/
/* Is called by modules :	write_subtree				*/
/*				write_tree				*/
/************************************************************************/
write_subtree(cur_node,dtree)
     struct tree_node *cur_node;
     FILE *dtree;
{
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
write_hp(cur_node,dtree)
     struct tree_node *cur_node;
     FILE *dtree;
{
  int i;
  
  if (dtree == NULL) return;
  
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

/*  for (i=1;i<=no_of_dimensions+1;i++)
    if (cur_node->coefficients[i])
      {
	if (i <= no_of_dimensions)
	  fprintf(dtree,"%f x[%d] + ",cur_node->coefficients[i],i);
	else
	  fprintf(dtree,"%f = 0\n\n",cur_node->coefficients[i]);
      }
*/

  for (i=1;i<=no_of_dimensions;i++)
    if (cur_node->coefficients[i])
	  fprintf(dtree,"%f x[%d] + ",cur_node->coefficients[i],i);
  fprintf(dtree,"%f = 0\n\n",cur_node->coefficients[no_of_dimensions+1]);
}

/************************************************************************/
/* Module name :	write_header					*/ 
/* Functionality :	Writes the decision tree header.		*/
/* Parameters :	dtree : file pointer to the output file.		*/
/* Is called by modules :	write_tree				*/
/************************************************************************/
write_header(dtree)
     FILE *dtree;
{
  extern int no_of_dimensions,no_of_categories;
  extern char train_data[LINESIZE];
  
  if (dtree == NULL) return;
  
  fprintf(dtree,"Training set: %s, ",train_data);
  fprintf(dtree,"Dimensions: %d, Categories: %d\n",
	  no_of_dimensions,no_of_categories);
  fprintf(dtree,"\n\n");
}

/************************************************************************/
/************************************************************************/
