/****************************************************************/
/* Copyright 1993 : Johns Hopkins University			*/
/*                 Department of Computer Science		*/
/****************************************************************/
/* Contact : murthy@cs.jhu.edu					*/
/****************************************************************/
/* File Name : perturb.c					*/
/* Author : Sreerama K. Murthy					*/
/* Last modified : October 1993					*/
/* Contains modules :	suggest_perturbation			*/
/*			perturb_randomly			*/
/*			linear_split				*/ 
/*			compare					*/
/*			find_values				*/
/* Uses modules in :	oc1.h					*/ 
/*			util.c					*/
/*			compute_impurity.c			*/
/* Is used by modules in :	mktree.c			*/
/****************************************************************/		

#include "oc1.h"

extern int no_of_coeffs;
extern int no_of_dimensions,no_of_categories;
extern int *left_count,*right_count;
extern int coeff_modified;
extern float *coeff_array;
extern float *modified_coeff_array;
extern struct unidim *candidates;
extern double *temp_val; /*Work area */

float mygamma;
float compute_impurity();
float myabs(),myrandom();
double exp();


/************************************************************************/
/* Module name : Cart_Perturb                                           */ 
/* Functionality : Implements the hill climbing perturbation algorithm  */
/*                 CART (Breiman et al, 1984) with Linear Combinations. */
/*                 Perturbation of the constant term is implemented     */
/*                 separately in the module cart_perturb_constant.      */
/*                 OC1 can be made to mimic Linear Combinations CART    */
/*                 using the -K flag for Mktree.                        */
/* Parameters :  cur_points: Pointers to the points under consideration.*/
/*               cur_no_of_points                                       */
/*               cur_coeff: Coefficient to be perturbed by hill climbing*/
/*               cur_error: Impurity before perturbation.               */
/* Returns :     Impurity of the hyperplane after perturbation.         */
/* Calls modules : find_values (compute_impurity.c)                     */
/*                 linear_split                                         */
/*                 compute_impurity (compute_impurity.c)                */
/* Is called by modules :  cart_split (mktree.c)                        */
/* Important Variables used : gamma and lambda : see CART book, chapter */
/*                            5 for a description.                      */
/************************************************************************/
float cart_perturb(cur_points,cur_no_of_points, cur_coeff,cur_error)
     POINT **cur_points;
     float cur_error;
     int cur_no_of_points,cur_coeff;
{
  int i,j,bestsplit,no_of_eff_points;
  float d_dim_error;
  float x,linear_split();
  float lambda, best_lambda,best_mygamma,best_impurity;
  
  for (i=1;i<=no_of_coeffs;i++)
    modified_coeff_array[i] = coeff_array[i];
  
  if (coeff_modified == TRUE) find_values(cur_points,cur_no_of_points);
  
  for (mygamma = -0.25; mygamma <=0.25; mygamma+=0.25)
    {
      no_of_eff_points = 0;
      
      /* cur_coeff should be less than no_of_coeffs */
      if (cur_coeff != no_of_coeffs)
	for (i=1;i<=cur_no_of_points;i++)
	  if (cur_points[i]->dimension[cur_coeff] + mygamma != 0)
	    {
	      no_of_eff_points++;
	      candidates[no_of_eff_points].cat = cur_points[i]->category;
	      x = (float)(cur_points[i]->val/(cur_points[i]->dimension[cur_coeff]
					      + mygamma));
	      candidates[no_of_eff_points].value = x;
	    }
      
      lambda = linear_split(no_of_eff_points);
      
      reset_counts();
      for (i=1;i<=cur_no_of_points;i++)
	if (cur_points[i]->val - lambda * (cur_points[i]->dimension[cur_coeff]
					   + mygamma) < 0)
	  left_count[cur_points[i]->category]++;
	else right_count[cur_points[i]->category]++;
      
      d_dim_error = compute_impurity(cur_no_of_points);
 
      if (mygamma == -0.25)
	{
	  best_lambda = lambda;
	  best_mygamma  = mygamma;
	  best_impurity = d_dim_error;
	}
      else if (best_impurity > d_dim_error)
	{
	  best_lambda = lambda;
	  best_mygamma  = mygamma;
	  best_impurity = d_dim_error;
	}
    }
 
  cur_error = best_impurity;
  modified_coeff_array[cur_coeff] -= best_lambda;
  modified_coeff_array[no_of_coeffs] -= best_lambda * best_mygamma;
  
  return(cur_error);  
}

/************************************************************************/
/* Module name : Cart_Perturb_Constant                                  */
/* Functionality : Implements the CART-Linear Combinations perturbation */
/*                 algorithm for the constant (displacement) term.      */
/* Parameters :  cur_points: Pointers to the points under consideration.*/
/*               cur_no_of_points                                       */
/*               cur_error: Impurity before perturbation.               */
/* Returns :     Impurity of the hyperplane after perturbation.         */
/* Calls modules : find_values (compute_impurity.c)                     */
/*                 linear_split                                         */
/*                 compute_impurity (compute_impurity.c)                */
/* Is called by modules :  cart_split (mktree.c)                        */
/* Important Variables used :
/* Remarks :
/************************************************************************/
float cart_perturb_constant(cur_points,cur_no_of_points,cur_error)
     POINT **cur_points;
     float cur_error;
     int cur_no_of_points;
{
  int i,j,bestsplit,no_of_eff_points;
  float d_dim_error;
  float x,linear_split();
  float lambda;
  
  for (i=1;i<=no_of_coeffs;i++)
    modified_coeff_array[i] = coeff_array[i];
  if (coeff_modified == TRUE) find_values(cur_points,cur_no_of_points);
  no_of_eff_points = 0;
  
  for (i=1;i<=cur_no_of_points;i++)
    {
      no_of_eff_points++;
      candidates[no_of_eff_points].cat = cur_points[i]->category;
      candidates[no_of_eff_points].value = (float)cur_points[i]->val;
    }
  
  lambda = linear_split(no_of_eff_points);
  
  reset_counts();
  for (i=1;i<=cur_no_of_points;i++)
    {
      if (cur_points[i]->val-lambda < 0) left_count[cur_points[i]->category]++;
      else right_count[cur_points[i]->category]++;
    }

  cur_error = compute_impurity(cur_no_of_points);
  modified_coeff_array[no_of_coeffs] -= lambda;

  return(cur_error);
}

/************************************************************************/
/* Module name : suggest_perturbation					*/ 
/* Functionality :	Suggests a new value for the coefficient 	*/
/*			"cur_coeff". This value is at least as good	*/
/*			as the existing value, in terms of the global	*/
/*			impurity measure. If no such value can be found,*/
/*			HUGE is returned. 				*/
/* Parameters :	cur_points : array of pointers to the points under	*/
/*			     consideration.				*/
/*		cur_no_of_points : count of the points in consideration.*/
/*		cur_coeff : coefficient to be improved.			*/
/*		cur_error_ptr : Pointer to the impurity of the current	*/
/*				hyperplane.				*/
/* Returns :	New value for the coefficient "cur_coeff".		*/
/*		HUGE : if no better value than the existing one can be	*/
/*		found.							*/
/* Calls modules :	find_values					*/
/*			linear_split					*/
/*			reset_counts (compute_impurity.c)		*/
/*			compute_impurity (compute_impurity.c)		*/
/*			myrandom (util.c)				*/
/* Is called by modules :	oblique_split (mktree.c)		*/
/* Important Variables used : 	no_of_stagnant_perturbations : global	*/
/*				variable that tells us how many pertur-	*/
/*				bations, immediately preceding the	*/
/*				current one, were consecutively 	*/
/*				"stagnant", ie., did not lessen the	*/
/*				global impurity. The probability that	*/
/*				the perturbation-to-be-suggested is	*/
/*				stagnant, is inversely exponentially 	*/
/*				dependent on this number.		*/
/* Remarks :	For a detailed description of the perturbation 		*/
/*		algorithm, see Murthy et al's paper in AAAI-93.		*/
/************************************************************************/
float suggest_perturbation(cur_points,cur_no_of_points,cur_coeff,cur_error)
     POINT **cur_points;
     float cur_error;
     int cur_no_of_points,cur_coeff;
{
  extern int no_of_stagnant_perturbations;
  int i,j,lpt,rpt,bestsplit,no_of_eff_points=0;
  float d_dim_error;
  float suggest_perturbation();
  float x,linear_split();
  float newval,changeinval;

  for (i=1;i<=no_of_coeffs;i++)
     modified_coeff_array[i] = coeff_array[i];

  if (coeff_modified == TRUE) find_values(cur_points,cur_no_of_points);

  if (cur_coeff == no_of_coeffs)
    for (i=1;i<=cur_no_of_points;i++)
      { 
	no_of_eff_points++;
	candidates[no_of_eff_points].cat = cur_points[i]->category;
	candidates[no_of_eff_points].value =
	  coeff_array[no_of_coeffs] - (float)cur_points[i]->val;
      }
  else 
    for (i=1;i<=cur_no_of_points;i++)
      if (cur_points[i]->dimension[cur_coeff] != 0)
	{
	  no_of_eff_points++;
	  candidates[no_of_eff_points].cat = cur_points[i]->category;
	  x = (float)(cur_points[i]->val/cur_points[i]->dimension[cur_coeff]);
	  candidates[no_of_eff_points].value = coeff_array[cur_coeff] - x;
	}

  newval = linear_split(no_of_eff_points);
  changeinval = newval - coeff_array[cur_coeff];
  
  reset_counts();
  for (i=1;i<=cur_no_of_points;i++)
    {
      temp_val[i] = cur_points[i]->val;
      
      if (cur_coeff == no_of_coeffs) temp_val[i] += changeinval;
      else temp_val[i] += changeinval * cur_points[i]->dimension[cur_coeff];
      if (temp_val[i] < 0) left_count[cur_points[i]->category]++;
      else right_count[cur_points[i]->category]++;
    }

  d_dim_error = compute_impurity(cur_no_of_points);
  
  if (cur_error < d_dim_error ||
      (myabs(cur_error-d_dim_error) <= TOLERANCE &&
       no_of_stagnant_perturbations >  MAX_NO_OF_STAGNANT_PERTURBATIONS))
    return(cur_error);
  
  modified_coeff_array[cur_coeff] = newval;
  
  if (myabs(cur_error-d_dim_error) <= TOLERANCE)
    no_of_stagnant_perturbations++;
  else 
    {
      no_of_stagnant_perturbations = 0;
      cur_error = d_dim_error;
    }
  
  return(cur_error);
}


/************************************************************************/
/* Module name : perturb_randomly					*/ 
/* Functionality :	Tries to perturb the current hyperplane in a	*/
/*			random direction, by a deterministic amount.	*/
/*			The amount of perturbation along the random 	*/
/*			direction is chosen so that the global impurity	*/
/*			is minimized.					*/
/* Parameters :	cur_points : Array of pointers to the points under	*/
/*		consideration.						*/
/*		cur_no_of_points : Number of points in consideration.	*/
/*		cur_error_ptr :	Pointer to the current value of global	*/
/*		impurity.						*/
/* Returns :	TRUE : If the random perturbation is accomplished	*/
/*		(so that the global impurity is at least as good as	*/
/*		 it used to be).					*/
/*		FALSE : otherwise					*/
/*		The new location of the hyperplane, after perturbation,	*/
/*		can be obtained from the global array "coeff_array".	*/
/* Calls modules :	vector (util.c)					*/
/*			generate_random_hyperplane (train_util.c)	*/
/*			find_values					*/
/*			reset_counts (compute_impurity.c)		*/
/*			linear_split 					*/
/*			compute_impurity (compute_impurity.c)		*/	
/*			free_vector (util.c)				*/
/* Is called by modules : oblique_split (mktree.c)			*/
/************************************************************************/
float perturb_randomly(cur_points,cur_no_of_points,cur_error,cur_label)
     POINT **cur_points;
     float cur_error;
     int cur_no_of_points;
     char *cur_label;
{
  extern int no_of_stagnant_perturbations;
  int i,j,no_of_eff_points=0;
  float d_dim_error;
  float *rvector;
  float alpha,linear_split();
  
  for (i=1;i<=no_of_coeffs;i++)
    modified_coeff_array[i] = coeff_array[i];
  
  rvector = vector(1,no_of_coeffs);
  generate_random_hyperplane(rvector,no_of_coeffs,MAX_COEFFICIENT);
  
  if (coeff_modified == TRUE) find_values(cur_points,cur_no_of_points);

  for (i=1;i<=cur_no_of_points;i++)
    {
      temp_val[i] = rvector[no_of_coeffs];
      for (j=1;j<no_of_coeffs;j++)
	temp_val[i] += rvector[j] * cur_points[i]->dimension[j];
      
      if (temp_val[i])
	{
	  no_of_eff_points++;
	  candidates[no_of_eff_points].cat = cur_points[i]->category;
	  candidates[no_of_eff_points].value = 
	    (float)(-1.0 * (cur_points[i]->val / temp_val[i]));
	}
    }
  
  if (!no_of_eff_points) 
    {
      free_vector(rvector,1,no_of_coeffs);
      return(FALSE);
    }
  
  alpha = linear_split(no_of_eff_points);
  
  reset_counts();
  for (i=1;i<=cur_no_of_points;i++)
    {
      temp_val[i] *= alpha;
      temp_val[i] += cur_points[i]->val;
      if (temp_val[i] < 0) left_count[cur_points[i]->category]++;
      else right_count[cur_points[i]->category]++;
    }
  
  d_dim_error = compute_impurity(cur_no_of_points);

  if (cur_error > d_dim_error)
    {
      no_of_stagnant_perturbations = 0;
      for (i=1;i<=no_of_coeffs;i++)
	modified_coeff_array[i] +=  rvector[i]*alpha;
      cur_error = d_dim_error;
    }
  
  free_vector(rvector,1,no_of_coeffs);
  return(cur_error);
}

/************************************************************************/
/* Module name : compare						*/
/* Functionality : 	used by the C library function "qsort".		*/
/*			see "man qsort" for more details.		*/
/* Parameters :	ptr1,ptr2 : Pointers to the values that are to be	*/
/*		compared. In this case, we are comparing the values 	*/
/*		suggested by two data points for a hyperplane 		*/
/*		coefficient. See the "Important variables used" section	*/
/*		below.							*/ 
/* Returns :	1, 0 or -1, depending on whether the first value is	*/
/*		more, equal or less than the second.			*/
/* Calls modules : None.						*/
/* Is called by modules : qsort in linear_split				*/
/* Important Variables used : struct unidim : Consider the 2D case. Let	*/
/*		the equation of the hyperplane be ax1+bx2+c=0. a,b and	*/
/*		c have some values for the current hyperplane location.	*/
/*		We are trying to perturb the hyperplane to a better	*/
/*		location. We do this coefficient after coefficient. 	*/
/*		Consider "a" as a variable, whose value we need to 	*/
/*		determine, and b and c as constants. By substituting	*/
/*		each data point in the above equation, we get a value	*/
/*		for "a". It is this value that is stored in the "value"	*/
/*		field of the structure unidim. The "cat" field stores	*/
/*		the category of the data point under consideration.	*/
/************************************************************************/
int compare(ptr1,ptr2) 
     struct unidim  *ptr1,*ptr2;
{
  float x;
  
  x = (*ptr1).value - (*ptr2).value;
  
  if (x > 0) return(1); 
  else if (x) return(-1);
  else return(0);
}


/************************************************************************/
/* Module name : linear_split						*/ 
/* Functionality :	Sorts the values in the unidim structure	*/
/*			array "candidates" (see the module header for	*/
/*			"compare" above), and splits the array at a 	*/
/*			position that minimizes the impurity measure.	*/
/* Parameters :	no_of_eff_points : Number of valid entries in the	*/
/*				   "candidates" array.		*/
/* Returns :	value of the coefficient that results in an optimal	*/
/*		one dimensional spilit.					*/ 
/* Calls modules :	qsort (C library routine)			*/
/*			myrandom (util.c)				*/
/*			reset_counts (compute_impurity.c)		*/
/*			compute_impurity (compute_impurity.c)		*/
/* Is called by modules :	suggest_perturbation			*/
/*				perturb_randomly			*/
/*				axis_parallel_split (mktree.c)		*/	
/* Remarks :								*/ 
/*	     2.	A Lot of the computation done in OC1 takes place in	*/
/*		the "qsort" system call in this routine, and in the	*/
/*		module "find_values".					*/
/************************************************************************/
float linear_split(no_of_eff_points) 
     int no_of_eff_points;
{
  int i,j,from,to,bestsplit;
  float temp,impurity_1d;
  float newval;
  int l1,l2,r1,r2;
  int compare();
  
  candidates += 1;
  qsort((char *)candidates,no_of_eff_points,sizeof(struct unidim),compare);
  candidates -= 1;
  
  reset_counts();
  for (i=1;i<=no_of_eff_points;i++)
    right_count[candidates[i].cat]++;
  
  impurity_1d = compute_impurity(no_of_eff_points);
  bestsplit = 0;

  for (i=1;i<=no_of_eff_points;i++)
    {
      from = i;
      for (to=from+1;to<=no_of_eff_points && candidates[to].value ==
	   candidates[from].value;to++);
      to -= 1;
      
      for (j=from;j<=to;j++)
	{
	  left_count[candidates[j].cat]++;
	  right_count[candidates[j].cat]--;
	}

      i = to;
      temp = compute_impurity(no_of_eff_points);
      
      if (temp < impurity_1d ||
	  (temp == impurity_1d && myrandom(0.0,1.0) < 0.5))
	{
	  impurity_1d = temp; 
	  bestsplit = i; 
	  if (impurity_1d == 0) break;
	}
    } 
  
  if (bestsplit == 0) newval = candidates[1].value - TOLERANCE;
  else if (bestsplit == no_of_eff_points)
    newval = candidates[bestsplit].value;
  else
    newval = (candidates[bestsplit].value + 
              candidates[bestsplit+1].value)/2;
  
  return(newval);
}
 
/************************************************************************/
/************************************************************************/

