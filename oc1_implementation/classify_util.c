/****************************************************************/
/* Copyright 1993 : Johns Hopkins University			*/
/*                  Department of Computer Science		*/
/****************************************************************/
/* Contact : murthy@cs.jhu.edu					*/
/****************************************************************/
/* File Name :	classify_util.c					*/
/* Author : Sreerama K. Murthy					*/
/* Last modified : October 1993					*/
/* Contains modules : 	read_tree				*/
/*			read_subtree				*/
/*			read_hp					*/
/*			read_header				*/
/*			print_header				*/
/*			isleftchild				*/
/*			isrightchild				*/
/*			leaf_count				*/
/*			tree_depth				*/
/* Uses modules in :	oc1.h					*/
/*			util.c					*/ 
/* Is used by modules in :	mktree.c			*/
/* Remarks       : 	These routines are mainly used to read	*/
/*			in a decision tree.			*/
/****************************************************************/		

#include "oc1.h"

struct tree_node *extra_node;
char train_data[LINESIZE],order_of_improvement[20];

extern int no_of_dimensions, no_of_categories;
int no_of_coeffs,no_of_iterations;



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
/*		save_tree module (train_util.c). A sample decision	*/
/*		tree is available in sample.dt.				*/ 
/************************************************************************/
struct tree_node *read_tree(decision_tree)
char *decision_tree;
{
 FILE *dtree;
 struct tree_node *root,*cur_node,*read_hp();
 void read_subtree();
 int read_header();

 if ((dtree = fopen(decision_tree,"r")) == NULL)
  error ("Decision Tree file can not be opened.");

 if ( !(read_header(dtree))) 
  error("Decision tree invalid/absent.");

 no_of_coeffs = no_of_dimensions+1;

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
void read_subtree(root,dtree)
struct tree_node *root;
FILE *dtree;
{
 struct tree_node *cur_node,*read_hp();
 void read_subtree();
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
/*		Hopefully the requirements of format won't be so	*/
/*		stringent in future !					*/
/************************************************************************/
struct tree_node *read_hp(dtree)
FILE *dtree;
{
 struct tree_node *cur_node;
 float temp;
 char c;
 int i;

 cur_node = (struct tree_node *)malloc(sizeof(struct tree_node));
 cur_node->coefficients = vector(1,no_of_coeffs);
 cur_node->left_count = ivector(1,no_of_categories);
 cur_node->right_count = ivector(1,no_of_categories);

 for (i=1;i<=no_of_coeffs;i++) cur_node->coefficients[i] = 0;

 cur_node->left = cur_node->right = NULL;

 while (isspace(c = getc(dtree)));
 ungetc(c,dtree); 

 if (fscanf(dtree,
      "%[^' '] Hyperplane: Left = [", cur_node->label) != 1)
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
        error("Invalid hyperplane equation.");
     if (i <= 0 || i > no_of_coeffs) 
       error("Invalid coefficient index in decision tree.");
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
/*		Hopefully the requirements of format won't be so	*/
/*		stringent in future !					*/
/************************************************************************/
int read_header(dtree)
FILE *dtree;
{

 if ((fscanf(dtree,"Training set: %[^,], ",train_data)) != 1) return(0);
 if ((fscanf(dtree,"Dimensions: %d, Categories: %d\n",
      &no_of_dimensions,&no_of_categories)) != 2) return(0);
 if ((fscanf(dtree,"Program Parameters: Iter: %d, Order: %s",
     &no_of_iterations, order_of_improvement)) != 2) return(0);
 return(1);

}

/************************************************************************/
/* Module name : print_header						*/
/* Functionality :	Outputs the header information. 		*/
/* Parameters :	none.							*/
/* Returns : 	nothing.						*/
/* Calls modules : none.						*/
/* Is called by modules :	main (mktree.c)				*/
/************************************************************************/
print_header()
{
printf("Train: %s, Dim: %d, Cat: %d\n",
        train_data,no_of_dimensions,no_of_categories);
printf("Parameters: Iter: %d, Order: %s\n",
        no_of_iterations,order_of_improvement);

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
/* Is called by modules :       classify_and_estimate_accuracy          */
/*						(classify.c)		*/
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
/* Is called by modules :       classify_and_estimate_accuracy          */
/*						(classify.c)		*/
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
