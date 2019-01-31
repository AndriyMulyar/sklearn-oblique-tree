/****************************************************************/
/* Copyright 1993, 1994                                         */
/* Johns Hopkins University			                */
/* Department of Computer Science		                */
/****************************************************************/
/* Contact : murthy@cs.jhu.edu					*/
/****************************************************************/
/* File Name : classify.c					*/
/* Author : Sreerama K. Murthy					*/
/* Last modified : July 1994					*/
/* Contains modules : 	estimate_accuracy			*/
/*			print_point              		*/
/*                      classify                                */
/* Uses modules in :	oc1.h					*/
/*			util.c					*/ 
/* Is used by modules in :	mktree.c			*/
/* Remarks       :      Modules for classifying labelled or     */
/*                      unlabelled data given a decision tree.  */
/****************************************************************/		

#include "oc1.h"

extern int no_of_dimensions,no_of_categories;
extern int verbose;
extern FILE *logfile;
extern char misclassified_data[LINESIZE];
  

/************************************************************************/
/* Module name : classify						*/ 
/* Functionality :	Uses the decision tree (pointed to by "root") to*/
/*			classify "points", and writes the classified 	*/
/*			points to "output".				*/
/* Parameters :	points : array of pointers to POINT structures.		*/
/*		no_of_points : number of test samples			*/
/*		root : pointer to the root of the decision tree		*/
/*		output : Points, with assigned categories are written	*/
/*		to this file.						*/
/* Returns :	None.							*/
/* Calls modules :	ivector (util.c)				*/
/*			free_ivector (util.c)				*/
/*			leaf_count (classify_util.c)			*/
/*		 	tree_depth (classify_util.c)			*/
/*			print_point              			*/	
/* Is called by modules :	main (mktree.c)				*/
/************************************************************************/
classify (points,no_of_points,root,output)
     POINT **points;
     int no_of_points;
     char *output;
     struct tree_node *root;
{
  FILE *outfile; 
  int i,j;
  float myrandom();
  struct tree_node *cur_node;
  double sum;
 
  if ((outfile = fopen(output,"w")) == NULL) outfile = stdout;
  
  for (i=1;i<=no_of_points;i++)
    {
      cur_node = root;
      while (cur_node != NULL)
	{
	  sum = cur_node->coefficients[no_of_dimensions+1];
	  for (j=1;j<=no_of_dimensions;j++)
	    sum += cur_node->coefficients[j] * points[i]->dimension[j];
	  if (sum < 0)
	    {
	      if (cur_node->left != NULL) 
		cur_node = cur_node->left;
	      else
		{
		  points[i]->category = cur_node->left_cat ;
		  break;
		}
	    }
	  else
	    {
	      if (cur_node->right != NULL) 
		cur_node = cur_node->right;
	      else
		{
		  points[i]->category = cur_node->right_cat ;
		  break;
		}
	    }
	}
      print_point(outfile,points[i],FALSE);
    }
  
  if (outfile != stdout) fclose(outfile);
}

/************************************************************************/
/* Module name : print_point						*/ 
/* Functionality :	Prints one point (example or sample) to the	*/
/*			output file "out".				*/
/* Parameters :	out : File pointer to the output file.			*/
/*		cur_point : pointer to the POINT structure.		*/
/*		unlabeled : Flag specifying whether the category of	*/
/*			    the point is to be printed.			*/
/* Returns : Nothing.							*/	
/* Calls modules : None.						*/
/* Is called by modules : 	classify				*/
/*				main (gendata.c)			*/
/************************************************************************/
print_point(out,cur_point,unlabeled)
     FILE *out;
     POINT *cur_point;
     int unlabeled;
{
  int i;
  
  if (out == NULL || cur_point == NULL) return;
  
  for (i=1;i<=no_of_dimensions;i++)
    fprintf(out,"%f\t",cur_point->dimension[i]);
  if (unlabeled == FALSE) fprintf(out,"%d",cur_point->category);
  fprintf(out,"\n");
}

/************************************************************************/
/* Module name : estimate_accuracy					*/ 
/* Functionality :	Uses the decision tree (pointed to by "root") to*/
/*			classify "points".				*/
/*			If "misclassified_data" is the name of a file   */
/*                      that can be written into, all the test samples  */
/*			misclassified by the decision tree are written	*/
/*			to it.						*/
/* Parameters :	points : array of pointers to POINT structures.		*/
/*		no_of_points : number of test samples			*/
/*		root : pointer to the root of the decision tree		*/
/* Returns :	a structure "test_outcome", containing the details of	*/
/*		classification (overall classification accuracy, 	*/
/*		accuracies for individual classes, decision tree leaf	*/
/*		counts and depths etc)					*/
/* Calls modules :	ivector (util.c)				*/
/*			free_ivector (util.c)				*/
/*			error (util.c)					*/
/*			leaf_count (classify_util.c)			*/
/*		 	tree_depth (classify_util.c)			*/
/*			print_point              			*/	
/* Is called by modules :	main (mktree.c)				*/
/*				cross_validate (mktree.c)		*/
/* Remarks :	This routine is to classify and estimate accuracy of 	*/
/*		a decision tree only on datasets in which the class of 	*/
/*		the objects is marked.					*/ 
/************************************************************************/
struct test_outcome estimate_accuracy(points,no_of_points,root)
     POINT **points;
     int no_of_points;
     struct tree_node *root;
{
  FILE *infile,*outfile; 
  int i,j,cur_point_category;
  int total_corrects,total_incorrects;
  int leaf_count(),tree_depth();
  int *correct,*incorrect;
  struct tree_node *cur_node;
  struct test_outcome result;
  double sum;
 
  if (root == NULL) 
    error("Esimate_Accuracy : Called with empty decision tree."); 
  if (strlen(misclassified_data)) outfile = fopen(misclassified_data,"w");
  else outfile = NULL;

  correct = ivector(1,no_of_categories);
  incorrect = ivector(1,no_of_categories);

  for (i=1;i<=no_of_categories;i++) correct[i] = incorrect[i] = 0;
  
  for (i=1;i<=no_of_points;i++)
    {
      cur_node = root;
      while (cur_node != NULL)
	{
	  sum = cur_node->coefficients[no_of_dimensions+1];
	  for (j=1;j<=no_of_dimensions;j++)
	    sum += cur_node->coefficients[j] * points[i]->dimension[j];
	  
	  if (sum < 0)
	    {
	      if (cur_node->left != NULL) 
		cur_node = cur_node->left;
	      else
		{
		  if (cur_node->left_cat == points[i]->category) 
		    correct[points[i]->category]++; 
		  else 
		    { incorrect[points[i]->category]++; 
		      print_point(outfile,points[i],FALSE);
		    }
		  break;
		}
	    }
	  else
	    {
	      if (cur_node->right != NULL) 
		cur_node = cur_node->right;
	      else
		{
		  if (cur_node->right_cat == points[i]->category) 
		    correct[points[i]->category]++; 
		  else 
		    { incorrect[points[i]->category]++; 
		      print_point(outfile,points[i],FALSE);
		    }
		  break;
		}
	    }
	}
    }
  
  result.leaf_count = leaf_count(root);
  result.tree_depth = tree_depth(root);
  result.class = ivector(1, 2*no_of_categories);
  
  total_corrects = total_incorrects = 0;
  for (i=1;i<=no_of_categories;i++)
    { 
      total_corrects += correct[i];
      total_incorrects += incorrect[i];
    }

  result.accuracy = 100.0 * total_corrects / no_of_points;
  
  for (i=1;i<=no_of_categories;i++)
    {
      j = correct[i] + incorrect[i];
      result.class[2*i-1] = correct[i];
      result.class[2*i] = j;
    }

  if (outfile != NULL) fclose(outfile);
  free_ivector(correct,1,no_of_categories);
  free_ivector(incorrect,1,no_of_categories);
  
  return(result);  
}

/************************************************************************/
/************************************************************************/


