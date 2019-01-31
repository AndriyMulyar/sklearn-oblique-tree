/****************************************************************/
/* Copyright 1993, 1994                                         */
/* Johns Hopkins University			                */
/* Department of Computer Science		                */
/****************************************************************/
/* Contact : murthy@cs.jhu.edu					*/
/****************************************************************/
/* File Name : prune.c						*/
/* Author : Sreerama K. Murthy					*/
/* Last modified : July 1994					*/
/* Contains modules : 	prune					*/
/*			error_complexity_prune			*/
/*			cut_weakest_links			*/
/*			subtree_cost				*/
/*			node_cost				*/
/*			compute_alpha				*/
/*			replicate_tree				*/
/*			cut_subtrees				*/
/*			deallocate_tree				*/
/* Uses modules in :	util.c					*/
/*			oc1.h					*/ 
/* Is used by modules in :	main (mktree.c)			*/
/*				cross_validate (mktree.c)	*/
/* Remarks       : 	Currently only one pruning strategy is	*/
/*			implemented. This is Breiman et al's	*/
/*			Error Complexity or Cost Complexity	*/
/*			pruning.				*/
/****************************************************************/		
#include "oc1.h"

extern int no_of_coeffs,no_of_categories;
extern int verbose;

float *alpha_array;
int alpha_index=0;
int total_points=0;
int no_of_ptest_points;

POINT **ptest_points;
struct dt
{
  struct tree_node *root; /*pointer to a decision tree*/
  struct test_outcome cresult; /*classification result*/
} *tree_array;


/************************************************************************/
/* Module name : prune							*/ 
/* Functionality :	High level pruning routine that in turn calls	*/
/*			the particular pruning strategy selected.	*/
/*			Currently, only Breiman et al's error complexity*/
/*                      pruning using a separate test set is available.	*/
/* Parameters :	dtree: Pointer to the root of the decision tree that	*/
/*		needs to be pruned.					*/
/* Returns :	Pointer to the root of the pruned decision tree.	*/
/* Calls modules :	error_complexity_prune 				*/
/*			error (util.c)					*/
/* Is called by modules :	main (mktree.c)				*/
/*				cross_validate (mktree.c)		*/
/************************************************************************/
struct tree_node *prune(dtree,prune_points,no_of_prune_points)
     struct point **prune_points;
     int no_of_prune_points;
     struct tree_node *dtree;
{
  struct tree_node *error_complexity_prune();

  ptest_points = prune_points;
  no_of_ptest_points = no_of_prune_points;
  total_points = dtree->no_of_points;
  if (dtree == NULL || leaf_count(dtree) <= 2) 
    {
      if (verbose) printf("No pruning possible.\n");
      return(dtree);
    }
  return(error_complexity_prune(dtree));
}


/************************************************************************/
/* Module name : error_complexity_prune       			        */ 
/* Functionality :	Performs Breiman et al's Error Complexity	*/
/*			pruning on a decision tree. (See the book	*/
/*			"Classification and Regression Trees" for	*/
/*			more details of the pruning algorithm.)		*/
/*			Currently, we have not yet implemented 		*/
/*			pruning using cross validation. The global	*/
/*			parameter "prune_portion" determines the	*/
/*			size of the random proportion of points		*/
/*			from the training set that is used exclusively  */
/*                      for pruning.                            	*/ 
/* Parameters :	root : Pointer to the root of the decision tree to be	*/
/*		pruned.							*/
/* Returns :	Pointer to the root of the pruned decision tree.	*/
/* Calls modules :	leaf_count (classify_util.c)			*/
/*			estimate_accuracy (classify.c)	*/
/*			cut_weakest_links				*/
/* Is called by modules : 	prune					*/
/************************************************************************/
struct tree_node *error_complexity_prune(root)
     struct tree_node *root;
{
  struct tree_node 	*pruned_tree,*cut_weakest_links(),
                        *duplicate_root,*replicate_tree();
  struct test_outcome 	estimate_accuracy();
  int 			tree_index,no_of_trees,selected_tree;
  int 			i,internal_nodes,leaf_count(),largest_element();
  float 		misclassification_rate,temp,standard_error;
  float			*accuracies=NULL;


  internal_nodes = leaf_count(root) - 1;
  /* Assuming that the decision tree is binary, the number of
     internal nodes is the number of leaves minus 1. */

  duplicate_root = replicate_tree(root);

  tree_array = (struct dt *)malloc
    ((unsigned)internal_nodes * sizeof(struct dt));
  if (tree_array == NULL)
    error("Error_Complexity_Prune: Memory allocation Failure.");
  tree_array--;
  
 
  tree_index = 1;
  tree_array[1].root = duplicate_root;
  tree_array[1].cresult = estimate_accuracy (ptest_points,no_of_ptest_points,
					     tree_array[1].root);
/* added by S. Salzberg for avg accuracy pruning */
/*  tree_array[1].cresult.accuracy = (float)
	((float) tree_array[1].cresult.class[1] / 
	         tree_array[1].cresult.class[2] +
	 (float) tree_array[1].cresult.class[3] /
	         tree_array[1].cresult.class[4])/2;  
*/
  while (TRUE)
    {
      if (verbose)
	printf("Tree %d: Accuracy=%.2f\t#leaves=%.0f\n",
	       tree_index,tree_array[tree_index].cresult.accuracy,
	       tree_array[tree_index].cresult.leaf_count);
      tree_index ++;
      tree_array[tree_index].root = cut_weakest_links(tree_array[tree_index-1].root);
      if (tree_array[tree_index].root == NULL) break;
      tree_array[tree_index].cresult = 
	estimate_accuracy(ptest_points,no_of_ptest_points,
			  tree_array[tree_index].root);
  /* added by S. Salzberg for "average accuracy" pruning */
/*      tree_array[tree_index].cresult.accuracy = (float)
	((float) tree_array[tree_index].cresult.class[1] /
                 tree_array[tree_index].cresult.class[2] +
	 (float) tree_array[tree_index].cresult.class[3] /
	 tree_array[tree_index].cresult.class[4])/2; 
*/
    }

  no_of_trees = tree_index - 1;

  accuracies = vector(1,no_of_trees);
  for (i=1;i<=no_of_trees;i++)
    accuracies[i] = tree_array[i].cresult.accuracy;
  tree_index = largest_element(accuracies,no_of_trees);

  misclassification_rate = 1 - accuracies[tree_index]/100;
  standard_error = (float)sqrt((double)(misclassification_rate *
					(1 - misclassification_rate) / 
					no_of_ptest_points));
  selected_tree = tree_index;
  for (i=1;i<=no_of_trees;i++)
    {
      temp = 1 - accuracies[i]/100;
      if (temp <= (misclassification_rate + 
		   NO_OF_STD_ERRORS * standard_error) &&
	  tree_array[i].cresult.leaf_count < 
	  tree_array[selected_tree].cresult.leaf_count)
	selected_tree = i;
    }
  if (verbose) printf("Tree %d Selected.\n",selected_tree);

  pruned_tree = replicate_tree(root);
  tree_index = 1;
  tree_array[1].root = pruned_tree;
  while (tree_index < selected_tree)
    {
      tree_index ++;
      tree_array[tree_index].root = cut_weakest_links(tree_array[tree_index-1].root);
    }
  
  return(pruned_tree);
}

/************************************************************************/
/* Module name :	cut_weakest_links				*/ 
/* Functionality :	Given a decision tree, this calculates the cost	*/
/*			complexity parameter "alpha" for each interme-	*/
/*			diate node except the root, and severs the	*/
/*			subtrees starting at nodes with the lowest	*/
/*			alpha values.					*/
/* Parameters :	dtree : Pointer to the root of a decision tree.		*/	
/* Returns :	Pointer to the root of a decision tree, which is the	*/
/*		same as the input tree, except that the weakest links	*/
/*		have been cut.						*/
/* Calls modules :	leaf_count (classify_util.c)			*/
/*			compute_alpha					*/
/*			replicate_tree					*/
/*			cut_subtrees					*/
/* Is called by modules :	error_complexity_prune			*/
/* Important Variables used :	alpha_array : Is a float array of length*/
/*				equal to the number of internal nodes	*/
/*				in the tree. This stores the alpha	*/
/*				value for each internal node in the	*/
/*				tree. Storing alpha values in an array	*/
/*				in addition to in the tree nodes makes	*/
/*				control flow easier to understand.	*/
/* Remarks :	It is important to note that this routine does not 	*/
/*		return the input tree with some links cut. Links are	*/
/*		cut from a duplicate copy of the input tree, and that	*/
/*		duplicate tree is output.				*/
/************************************************************************/
struct tree_node *cut_weakest_links(dtree)
     struct tree_node *dtree;
{
  float x,y;
  int i,internal_nodes,leaf_count(),index=0;
  struct tree_node *dtree2,*replicate_tree();
  
  internal_nodes = leaf_count(dtree) - 1;
  /* Assuming that the decision tree is binary, the number of
     internal nodes is the number of leaves minus 1. */
  
  if (internal_nodes == 1) return(NULL);
  
  alpha_array = vector(1,internal_nodes);
  alpha_index = 0;
  
  compute_alpha(dtree); 
  
  index = 2; /*Do not consider the root for pruning at any stage. */
  if (internal_nodes > index)
    for (i=3;i<=internal_nodes;i++)
      if (alpha_array[i] < alpha_array[index]) index = i;
  
/*  dtree2 = replicate_tree(dtree);
  cut_subtrees(dtree2,alpha_array[index]);*/
  cut_subtrees(dtree,alpha_array[index]);
  return(dtree);
}

/************************************************************************/
/* Module name :	compute_alpha					*/ 
/* Functionality :	"alpha" is a crucial parameter in error comple-	*/
/*			xity pruning. "alpha" of an internal node N 	*/
/*			can be taken to measure, in informal terms, the	*/
/*			usefulness, in classifying the training set, 	*/
/*			per terminal node in the subtree starting at N.	*/
/*			This routine recursively computes the alpha	*/
/*			value at each internal node of the tree, and	*/
/*			stores these values in the array "alpha_array".	*/
/* Parameters :	node : pointer to the node, at and below which the	*/
/*		alpha values need to be computed.			*/
/* Returns :	Nothing explicitly. 					*/
/* Calls modules :	node_cost					*/
/*			subtree_cost					*/
/*			compute_alpha					*/
/*			leaf_count (classify_util.c)			*/
/* Is called by modules : 	cut_weakest_links			*/
/*				compute_alpha				*/
/************************************************************************/
compute_alpha(node)
     struct tree_node *node;
{
  int leaf_count();
  float ncost,scost;
  float node_cost(),subtree_cost();
  
  if (node == NULL) return;
  
  ncost = node_cost(node); 
  scost = subtree_cost(node);
  
  node->alpha = (ncost - scost)/(leaf_count(node) - 1);
  alpha_array[++alpha_index] = node->alpha;
  
  compute_alpha(node->left);
  compute_alpha(node->right);  
}

/************************************************************************/
/* Module name :	subtree_cost					*/ 
/* Functionality :	recursively computes the cost of the subtree 	*/
/*			below an internal node.				*/
/* Parameters :	cur_node : pointer to a decision tree node.		*/	
/* Returns :	A floating point number, representing the cost of	*/
/*		the subtree at cur_node.				*/
/* Calls modules :	subtree_cost					*/
/* Is called by modules :	compute_alpha				*/
/*				subtree_cost				*/
/************************************************************************/
float subtree_cost(cur_node)
     struct tree_node *cur_node;
{
  float cost=0,subtree_cost();
  int i,misclassified;
  
  if (cur_node->left == NULL)
    {
      misclassified = 0;
      for (i=1;i<=no_of_categories;i++)
	if (i != cur_node->left_cat) misclassified += cur_node->left_count[i];
      
      cost += (float)misclassified/total_points;
    }
  else cost += subtree_cost(cur_node->left); 
  
  if (cur_node->right == NULL)
    {
      misclassified = 0;
      for (i=1;i<=no_of_categories;i++)
	if (i != cur_node->right_cat) misclassified += cur_node->right_count[i];

      cost += (float)misclassified/total_points;
    }
  else cost += subtree_cost(cur_node->right); 
  
  return(cost);
}

/************************************************************************/
/* Module name : node_cost						*/
/* Functionality :	computes the cost of a node.			*/
/* Parameters :	cur_node : pointer to the DT node under consideration.	*/
/* Returns :	A floating point number, representing the cost of	*/
/*		*cur_node.						*/
/* Calls modules : None							*/
/* Is called by modules :	compute_alpha				*/
/************************************************************************/
float node_cost(cur_node)
     struct tree_node *cur_node;
{
  int i,max=1,misclassified=0;
  
  for (i=2;i<=no_of_categories;i++)
    if (cur_node->left_count[i] + cur_node->right_count[i] >
	cur_node->left_count[max] + cur_node->right_count[max])
      max = i;
  
  for (i=1;i<=no_of_categories;i++)
    if (i != max)
      misclassified += cur_node->left_count[i] + cur_node->right_count[i];
  
  return((float)misclassified/total_points);
} 
 
/************************************************************************/
/* Module name :	replicate_tree					*/ 
/* Functionality :	Given a decision tree, this module forms an	*/
/*			exact copy of it, recursively.			*/
/* Parameters :	root : Pointer to the root of a (sub)tree.		*/
/* Returns :	Pointer to the root of an identical (sub)tree.		*/
/* Calls modules :	error (util.c)					*/
/*			vector (util.c)					*/
/*			ivector (util.c)				*/
/*			replicate_tree					*/
/* Is called by modules :	cut_weakest_links			*/
/*				replicate_tree				*/
/************************************************************************/
struct tree_node *replicate_tree(root)
     struct tree_node *root;
{
  struct tree_node *duplicate,*replicate_tree();
  int i;
  
  if (root == NULL) return(NULL);
  
  duplicate = (struct tree_node *) malloc (sizeof(struct tree_node));
  if (duplicate == NULL) 
    error("Replicate_Tree : Memory Allocation Failure.");
  
  duplicate->coefficients = vector(1,no_of_coeffs);
  for (i=1;i<=no_of_coeffs;i++)
    duplicate->coefficients[i] = root->coefficients[i];
  
  duplicate->left_count = ivector(1,no_of_categories);
  duplicate->right_count = ivector(1,no_of_categories);
  for (i=1;i<=no_of_categories;i++)
    {
      duplicate->left_count[i] = root->left_count[i];
      duplicate->right_count[i] = root->right_count[i];
    }

  if (root->parent == NULL) duplicate->parent = NULL;
  duplicate->left = replicate_tree(root->left);
  if (duplicate->left != NULL) (duplicate->left)->parent = duplicate;
  duplicate->right = replicate_tree(root->right);
  if (duplicate->right != NULL) (duplicate->right)->parent = duplicate;
  duplicate->left_cat = root->left_cat;
  duplicate->right_cat = root->right_cat;
  strcpy(duplicate->label,root->label);
  duplicate->alpha = root->alpha;
  duplicate->no_of_points = root->no_of_points;
  
  return(duplicate);
}

/************************************************************************/
/* Module name : cut_subtrees						*/ 
/* Functionality : Given a decision tree node N, and a value of alpha,	*/
/*		   this module recursively finds all nodes in the	*/
/*		   subtree at N whose alpha values are equal to the	*/
/*		   given value, and cuts such branches.			*/	
/* Parameters :	cur_node: Pointer to a decision tree node.		*/
/*		alpha_threshold: Alpha value at which a node has to be	*/
/*		cut.							*/
/* Returns :	Nothing.						*/
/* Calls modules :	cut_subtrees					*/
/* Is called by modules :	cut_subtrees				*/
/*				cut_weakest_links			*/
/* Remarks : 	The root of a decision tree is never cut.		*/
/************************************************************************/
cut_subtrees(cur_node,alpha_threshold)
     struct tree_node *cur_node;
     float alpha_threshold;
{
  if (cur_node == NULL) return;
  
  if (cur_node->alpha == alpha_threshold && cur_node->parent != NULL)
    {
      if ((cur_node->parent)->left == cur_node)
	cur_node->parent->left = NULL;
      else if ((cur_node->parent)->right == cur_node)
	cur_node->parent->right = NULL;

      /* deallocate_tree(cur_node);*/ 
      return;
    }

  cur_node->alpha = 0; /*alpha has to be calculated afresh after every 
			 round of cuts. */
  
  cut_subtrees(cur_node->left,alpha_threshold);
  cut_subtrees(cur_node->right,alpha_threshold);
}

/************************************************************************/
/* Module name : deallocate_tree					*/
/* Functionality :	Recursively frees the memory allocated to a 	*/
/*			decision tree.					*/
/* Parameters :	Pointer to the root of a decision tree.			*/
/* Returns :	Nothing.						*/
/* Calls modules :	deallocate_tree					*/
/*			free_vector (util.c)				*/
/*			free_ivector (util.c)				*/
/* Is called by modules :	error_complexity_prune			*/
/*				cut_subtrees				*/
/*				deallocate_tree				*/
/************************************************************************/
deallocate_tree(root)
     struct tree_node *root;
{
  if (root == NULL) return;
  deallocate_tree(root->left);
  deallocate_tree(root->right);
  
  free_vector(root->coefficients,1,no_of_coeffs);
  free_ivector(root->left_count,1,no_of_categories);
  free_ivector(root->right_count,1,no_of_categories);
  
}

/************************************************************************/
/************************************************************************/
