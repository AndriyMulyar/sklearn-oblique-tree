/****************************************************************/
/* Copyright 1993, 1994                                         */
/* Johns Hopkins University			                */
/* Department of Computer Science		                */
/****************************************************************/
/* Contact : murthy@cs.jhu.edu					*/
/****************************************************************/
/* File Name : compute_impurity.c				*/
/* Author : Sreerama K. Murthy					*/
/* Last modified : July 1994					*/
/* Contains modules :	compute_impurity			*/
/*			set_counts				*/
/*			reset_counts				*/
/*			largest_element				*/
/* Uses modules in :	oc1.h					*/
/*			util.c					*/ 
/* Is used by modules in :	mktree.c			*/
/*				perturb.c			*/
/****************************************************************/		
#include "oc1.h"

extern int no_of_dimensions;
extern int *left_count,*right_count;
extern int no_of_categories;
extern int coeff_modified;
extern float *coeff_array;

/************************************************************************/
/* Module name : compute_impurity					*/
/* Functionality : Front end to the routine to compute the		*/
/*		   impurity of a given array of points.			*/
/*		   The name of the actual impurity-computing routine	*/
/*		   is given by the hash constant IMPURITY, which is	*/
/*		   in turn defined by the user in oc1.h.		*/
/* Parameters : cur_no_of_points : Size of the point set whose impurity	*/
/*		needs to be computed.                                   */
/* Returns :	impurity.						*/
/* Calls modules : IMPURITY						*/
/* Is called by modules :	main (mktree.c)				*/
/*				build_dt (mktree.c)			*/	
/*				oblique_split (mktree.c)		*/
/*				cross_validate (mktree.c)		*/
/*				suggest_perturbation (perturb.c)	*/
/*				perturb_randomly (perturb.c)		*/
/*				linear_split (perturb.c)		*/
/************************************************************************/
float compute_impurity(cur_no_of_points)
     int cur_no_of_points;
{
  int i,j=0,stop_splitting();
  float IMPURITY;
  
  if (cur_no_of_points <= 1) return(0);

  for (i=1;i<=no_of_categories;i++) j += left_count[i]+right_count[i];
  
  if (j != cur_no_of_points)
    error ("Compute_Impurity: Left_Count and Right_Count not correctly set.");
  
  if (stop_splitting()) return(0);
  
  return(IMPURITY);
}

/************************************************************************/
/* Module name : set_counts						*/
/* Functionality :	Sets the values in the integer arrays 		*/
/*			left_count and right_count, to reflect the	*/
/*			number of points of each category on the	*/
/*			left and right of the current hyperplane.	*/
/*			If "flag" is zero, the values are set		*/
/*			assuming that ALL points are on one (right)	*/
/*			side of the hyperplane (relevant while		*/
/*			computing initial impurity before splitting).	*/
/*			If "flag" is not zero, it is assumed that	*/
/*			the "val" fields of points are correctly set.	*/
/* Parameters :	cur_points :	Array of pointers to the POINT structs	*/
/*		cur_no_of_points : Number of points			*/
/*		flag : 0 if initial impurity is to be computed		*/
/* Returns :	Nothing.						*/
/* Calls modules : reset_counts						*/
/* Is called by modules :	main (mktree.c)				*/
/*				build_dt (mktree.c)			*/	
/*				oblique_split (mktree.c)		*/
/*				cross_validate (mktree.c)		*/
/* Remarks : 	There may be a better way of computing the initial	*/
/*		impurity, than considering a hypothetical hyperplane 	*/
/*		onto one side of the point set. Suggestions ?		*/
/************************************************************************/
set_counts(cur_points,cur_no_of_points,flag)
     POINT **cur_points;
     int cur_no_of_points;
     int flag;
{
  int i;
  
  reset_counts();
  if (!flag)
    for (i=1;i<=cur_no_of_points;i++)
      right_count[cur_points[i]->category]++;
  else
    {
     if (coeff_modified == TRUE) 
       {
	 fprintf(stderr,
		 "Set_Counts: Val fields of points are incorrect. Recomputing..\n");
	 find_values(cur_points,cur_no_of_points);
       }
     for (i=1;i<=cur_no_of_points;i++)
       if (cur_points[i]->val < 0)
	 left_count[cur_points[i]->category]++;
       else right_count[cur_points[i]->category]++;
   }
}

/************************************************************************/
/* Module name : reset_counts						*/
/* Functionality :	Resets the values in the integer arrays		*/
/*			left_count and right_count to zero.		*/
/* Parameters : None.							*/
/* Returns : Nothing.							*/
/* Calls modules : None.						*/
/* Is called by modules :	set_counts				*/
/*				suggest_perturbation (perturb.c)	*/
/*				perturb_randomly (perturb.c)		*/
/*				linear_split (perturb.c)		*/
/*	
/************************************************************************/
reset_counts()
{
  int i;
  
  for (i=1;i<=no_of_categories;i++)
    left_count[i] = right_count[i] = 0;
}

/************************************************************************/
/* Module name : largest_element					*/
/* Functionality :	determines the index of the largest element	*/
/*			in an array.					*/
/* Parameters :	array : integer array					*/
/*		count : number of elements				*/
/* Returns :	index of the largest element in the array.		*/
/* Calls modules : error (util.c)					*/
/* Is called by modules : 	build_dt (mktree.c)			*/
/*				maxminority (impurity_measures.c)	*/
/*				summinority (impurity_measures.c)	*/
/************************************************************************/
int largest_element(array,count)
     int *array,count;
{
  int i,major;
  
  major = 1;
  for (i=2;i<=count;i++)
    if (array[i] > array[major]) major = i;
  return(major);
}

/************************************************************************/
/* Module name : Stop_Splitting                                         */ 
/* Functionality : This routine is called just before the impurity of a */
/*                 hyperplane is computed. This checks to see if both   */
/*                 sides of the hyperplane are already homogeneous, and */
/*                 have different class labels. If this is the case, this*/
/*                 routine returns TRUE, indicating that the impurity   */
/*                 need not be computed, and can be reported as the     */
/*                 minimum possible.                                    */
/* Parameters : None.                                                   */
/* Returns : TRUE: If splitting can be stopped,                         */
/*           FALSE otherwise.                                           */
/* Calls modules : largest_element                                      */
/* Is called by modules : compute_impurity                              */
/* Remarks : It is assumed that the arrays left_count and right_count   */
/*           are set correctly.                                         */
/************************************************************************/
int stop_splitting()
{ 
  int i,lpt=0,rpt=0,left_cat,right_cat,largest_element();
  
  for (i=1;i<=no_of_categories;i++)
    {
      lpt += left_count[i];
      rpt += right_count[i];
    }
  
  left_cat = largest_element(left_count,no_of_categories);
  right_cat = largest_element(right_count,no_of_categories);
  
  if (left_count[left_cat] == lpt &&
      right_count[right_cat] == rpt &&
      left_cat != right_cat) return(TRUE);
  else return(FALSE);
      
}

/************************************************************************/
/* Module name : find_values						*/
/* Functionality :	Sets the "val" fields of the points structures,	*/
/*			by substituting the points into the equation	*/
/*			of the current hyperplane (given by the array	*/
/*			"coeff_array").					*/
/* Parameters :	cur_points : Array of pointers to point structures.	*/
/*		cur_no_of_points : number of points under consideration.*/
/* Returns :	Nothing.						*/
/* Calls modules : None.						*/
/* Is called by modules :	suggest_perturnbation			*/
/*				perturb_randomly			*/
/*				axis_parallel_split (mktree.c)		*/
/*				build_dt (mktree.c)			*/
/*				oblique_split (mktree.c)		*/
/* Important Variables used :	coeff_modified : Is used to avoid extra	*/
/*				computations of the val fields. This	*/
/*				variable is set to TRUE everytime a	*/
/*				coefficient in "coeff_array" is modified*/
/*				(ie., if the entries in the "val" fields*/
/*				do not reflect the values obtained by	*/
/*				substituting the point's coordinates in	*/
/*				the equation of the hyperplane). It is	*/
/*				set to FALSE when the "val" fields are	*/
/*				computed.				*/
/* Remarks :	A lot of the computation done in OC1 takes place in this*/
/*		routine, and the qsort system call in "linear_split".	*/	
/************************************************************************/
find_values(cur_points,cur_no_of_points)
     POINT **cur_points;
     int cur_no_of_points;
{
  int i,j;

  if (coeff_modified == FALSE) return;
  for (i=1;i<=cur_no_of_points;i++)
    {
      cur_points[i]->val = coeff_array[no_of_dimensions+1];
      for (j=1;j<=no_of_dimensions;j++)
	cur_points[i]->val += cur_points[i]->dimension[j] * coeff_array[j];
    }
  
  coeff_modified = FALSE;
}
 

/************************************************************************/
/************************************************************************/

