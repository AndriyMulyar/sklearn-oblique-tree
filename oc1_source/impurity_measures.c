/****************************************************************/
/* Copyright 1993, 1994                                         */
/* Johns Hopkins University			                */
/* Department of Computer Science		                */
/****************************************************************/
/* Contact : murthy@cs.jhu.edu					*/
/****************************************************************/
/* File Name : impurity_measures.c				*/
/* Author : Sreerama K. Murthy					*/
/* Last modified : July 1994					*/
/* Contains modules :	maxminority				*/
/*			summinority				*/
/*			variance				*/
/*			info_gain				*/
/*			gini_index				*/
/*			twoing					*/
/* Uses modules in : 	compute_impurity.c			*/
/*			oc1.h					*/
/* Is used by modules in :	compute_impurity.c		*/
/* Remarks       :	If the user wants to "plug-in" a new	*/
/*			impurity measure, that function should	*/
/*			look like the functions in this file.   */
/*                      It is also probably convenient to	*/
/*			concatenate such new measures to	*/
/*			this file, so that the user needn't 	*/
/*			bother with the global declarations and	*/
/*			changing the makefile.			*/
/*                      Note that none of the measure use any   */
/*                      input parameters. The  only information */
/*                      used is the number of dimensions, the   */
/*                      number of  classes (categories) and the */
/*                      counts of points of each class on the   */
/*                      left and right of the hyperplane.       */
/*                      All measures return a nonnegative double */
/*                      impurity value, where the lower the     */
/*                      impurity, the better the hyperplane.    */
/*                      All measures are called from the module */
/*                      Compute_Impurity.                       */
/****************************************************************/		

#include "oc1.h"
extern int no_of_dimensions;
extern int *left_count,*right_count;
extern int no_of_categories;

int largest_element();

/************************************************************************/
/* Module name :	maxminority					*/ 
/* Functionality :	Suggested by Heath et al in their IJCAI-93 paper*/
/*                      Has the theoretical advantage that the depth of */
/*                      of the resulting tree has to be logarithmic in  */
/*                      the number of instances. Can be computed very   */
/*                      efficiently - no log computations.              */
/* Returns :	larger of the minority values on the left and right.	*/
/* 		(minority on a side = sum of the counts of all classes	*/
/*		except the class with the highest count.)		*/
/* Calls modules :	largest_element (compute_impurity.c)		*/
/************************************************************************/
double maxminority()
{
  int i,j,lminor=0,rminor=0;
  
  i = largest_element(left_count,no_of_categories);
  if (i <= no_of_categories)
    for (j=1;j<=no_of_categories ;j++) 
      if (i != j) lminor += left_count[j];

  i = largest_element(right_count,no_of_categories);
  if (i <= no_of_categories)
    for (j=1;j<=no_of_categories ;j++) 
      if (i != j) rminor += right_count[j];
  
  if (lminor > rminor) 
    return((double)lminor);
  else return((double)rminor);
}

/************************************************************************/
/* Module name :	summinority					*/ 
/* Functionality :	Suggested by Heath et al in their IJCAI-93 paper*/
/*                      Is intuitively most appealing, and can be       */
/*                      computed efficiently - no log computations.     */
/*                      Does well on several domains, in spite of its   */
/*                      simplicity.                                     */
/* Returns :	sum of the minority values on the left and right.	*/
/* 		(minority on a side = sum of the counts of all classes	*/
/*		except the class with the highest count.)		*/
/* Calls modules :	largest_element (compute_impurity.c)		*/
/************************************************************************/
double summinority()
{
  int i,j,lminor=0,rminor=0;
  
  i = largest_element(left_count,no_of_categories);
  if (i <= no_of_categories)
    for (j=1;j<=no_of_categories ;j++) 
      if (i != j) lminor += left_count[j];
  
  i = largest_element(right_count,no_of_categories);
  if (i <= no_of_categories)
    for (j=1;j<=no_of_categories ;j++) 
      if (i != j) rminor += right_count[j];
  
  return((double)(lminor+rminor));
}

/************************************************************************/
/* Module name :	variance					*/ 
/* Functionality :	Suggested by Heath et al in their IJCAI-93 paper*/
/*                      as the Sum_of_Impurities measure.               */
/* Returns :	sum of the category number variances on the	 	*/
/*		left and right, making adjustments for bias.		*/
/* 		Variance on a side = sigma  ((xi - avg)*(xi - avg))	*/
/*				     i=1..k				*/
/* 		where x1,..xk are the classes (categories) of the	*/
/*		points on that side of the hyperplane, and		*/
/*		avg = (x1+x2+..+xk)/k.					*/
/************************************************************************/
double variance()
{
  double lavg=0,ravg=0,lerror = 0,rerror = 0;
  int i,lsum1=0,rsum1=0,lsum2=0,rsum2=0;
  int *temp1=NULL,*temp2=NULL;
  int var_compare();

  if (no_of_categories > 2)
    /* Renumber categories in descending order of their proportion of
       occurance. This removes the possibility for a biased impurity
       estimate. */
    {
      temp1 = ivector(1,no_of_categories);
      temp2 = ivector(1,no_of_categories);
      for (i=1;i<=no_of_categories;i++)
	{
	  temp1[i] = left_count[i];
	  temp2[i] = right_count[i];
	} 
      qsort((char *)(left_count+1),no_of_categories,sizeof(int),var_compare);
      qsort((char *)(right_count+1),no_of_categories,sizeof(int),var_compare);
    }
     
  for (i=1;i<=no_of_categories;i++)
    { 
      lsum1 += left_count[i]; 
      lsum2 += i * left_count[i];
      rsum1 += right_count[i]; 
      rsum2 += i * right_count[i];
    }

  if (lsum1 != 0) lavg = (double)lsum2/lsum1;
  if (rsum1 != 0) ravg = (double)rsum2/rsum1;
  
  for (i=1;i<=no_of_categories;i++)
    {
      lerror += left_count[i] * (i - lavg) * (i - lavg);
      rerror += right_count[i] * (i - ravg) * (i - ravg);
    }

  if (no_of_categories > 2)
    /* Restore original left_count and right_count arrays.
       Remember, they are read_only. */
    {
      for (i=1;i<=no_of_categories;i++)
	{
	  left_count[i] = temp1[i];
	  right_count[i] = temp2[i];
	} 
      free_ivector(temp1,1,no_of_categories);
      free_ivector(temp2,1,no_of_categories);
    }

  return (lerror+rerror);
  
}

/************************************************************************/
/* Module name : Var_Compare                                            */ 
/* Functionality : Used by the qsort function call in the module        */
/*                 Variance.                                            */
/*                 See the man page for qsort for more details.         */
/************************************************************************/
int var_compare(p1,p2)
int *p1,*p2;
{
 int p;

 p = *p1-*p2;
 return(p);
}

/************************************************************************/
/* Module name : info_gain						*/ 
/* Functionality :	Computes the (Quinlan's) information gain of 	*/
/*			the current split. As OC1 tries to minimize the	*/
/*			"impurity" instead of maximizing the information*/
/*			"gain", this module returns the reciprocal of 	*/
/*			the computed gain.				*/
/* Remarks : Much less efficient to compute than the minority measures. */
/*           But often works much better.                               */
/************************************************************************/
double info_gain()
{
  double presplit_info=0,postsplit_info=0,left_info=0,right_info=0;
  double ratio,infogain;
  int i,total_count=0,total_left_count=0,total_right_count=0;
  double mylog2();
  
  for (i = 1;i<=no_of_categories;i++) 
    {
      total_left_count += left_count[i];
      total_right_count += right_count[i];
    }
  total_count = total_left_count + total_right_count;

  if (total_count)
    for (i = 1;i<=no_of_categories;i++)
      {
	ratio = (double)(left_count[i]+right_count[i])/total_count;
	if (ratio) presplit_info += -1.0 * ratio * mylog2(ratio);
      }
  
  if (total_left_count)
    {
      for (i = 1;i<=no_of_categories;i++)
	{
	  ratio = (double)left_count[i]/total_left_count;
	  if (ratio) left_info += -1.0 * ratio * mylog2(ratio);
	}
      postsplit_info += total_left_count * left_info / total_count;
    }
  
  if (total_right_count)
    {
      for (i = 1;i<=no_of_categories;i++)
	{
	  ratio = (double)right_count[i]/total_right_count;
	  if (ratio) right_info += -1.0 * ratio * mylog2(ratio);
	}
      postsplit_info += total_right_count * right_info / total_count;
    }
  
  infogain = presplit_info - postsplit_info;
  
  if (infogain == 0) /*No information gained due to this split.
		       i.e., Either the region is homogenous or impurity 
		       is as large as it can be. */
    {
      for (i=1;i<=no_of_categories;i++)
	if (left_count[i] + right_count[i] == total_count) return(0);
      return(HUGE_VAL);
    }
  else return(1.0/infogain);
}

/************************************************************************/
/* Module name : gini_index						*/ 
/* Functionality :	Computes gini_index of a hyperplane. This stati-*/
/*			stical measure was described in the context of 	*/
/*			decision trees by Leo Breiman et al in the CART	*/
/*			book, 1984.					*/
/* Remarks : Efficient to compute - No log computations.                */
/*           Performs quite well.                                       */
/************************************************************************/
double gini_index()
{
  int total_left_count=0,total_right_count=0;
  double temp,gini_left=0,gini_right=0,gini_value;
  int i,j;

  for (i=1;i<=no_of_categories;i++)
    {
      total_left_count += left_count[i];
      total_right_count += right_count[i];
    }
  
  if (total_left_count)
    {
      for (i=1;i<=no_of_categories;i++)
	{
	  temp = (1.0 * left_count[i]) / total_left_count;
	  gini_left += temp * temp;
	}
      gini_left = 1.0 - gini_left;
    }
  
  if (total_right_count)
    {
      for (i=1;i<=no_of_categories;i++)
	{
	  temp = (1.0 * right_count[i]) / total_right_count;
	  gini_right += temp * temp;
	}
      gini_right = 1.0 - gini_right;
    }

  gini_value = (total_left_count * gini_left + total_right_count * gini_right)/
               (total_left_count+total_right_count);
  return(gini_value);
}

/**********************************************************************************************************/
/* Module name : hellinger_distance                                                                       */
/* Functionality :      Computes hellinger_distance of a hyperplane split.                                */
/*                      A statistical measure of probability distribution divergence.  D. Cieslak, 2011.  */
/* Remarks : Performs well on imbalanced data                                                             */
/**********************************************************************************************************/
double hellinger_distance()
{
  int total_left_count=0,total_right_count=0;
  double class_i_prob,class_j_prob,hellinger_left=0,hellinger_right=0,dimension,hellinger_value;
  int i,j;
  double SQRT_TWO = sqrt(2);

  for (i=1;i<=no_of_categories;i++)
    {
      total_left_count += left_count[i];
      total_right_count += right_count[i];
    }

  if (total_left_count)
    {
      for (i=1;i<=no_of_categories;i++)
        {
          for(j=i+1;j<=no_of_categories;j++)
	    {
              class_i_prob = (1.0 * left_count[i]) / total_left_count;
              class_j_prob = (1.0 * left_count[j]) / total_left_count;
              dimension = sqrt(class_i_prob) - sqrt(class_j_prob);
              dimension *= dimension;
              hellinger_left += dimension;
            }	
        }
      hellinger_left = SQRT_TWO - sqrt(hellinger_left);
    }

  if (total_right_count)
    {
      for (i=1;i<=no_of_categories;i++)
        {
          for(j=i+1;j<=no_of_categories;j++)
            {
              class_i_prob = (1.0 * right_count[i]) / total_right_count;
              class_j_prob = (1.0 * right_count[j]) / total_right_count;
	      dimension = sqrt(class_i_prob) - sqrt(class_j_prob);
              dimension *= dimension;
              hellinger_right += dimension;
            }   
        }
      hellinger_right = SQRT_TWO - sqrt(hellinger_right);
    }
  hellinger_value = (total_left_count * hellinger_left + total_right_count * hellinger_right)/
               (total_left_count+total_right_count);
  return(hellinger_value);
}



 
/************************************************************************/
/* Module name : twoing							*/ 
/* Functionality :	Computes, by twoing rule, the goodness of a 	*/
/*                      hyperplane. Returns the reciprocal of this value*/
/*			The twoing measure is described in detail  	*/
/*			by Leo Breiman et al in their CART book (1984).	*/
/************************************************************************/
double twoing()
{
  double total_left_count=0,total_right_count=0,total_count;
  double goodness=0,temp,twoing_val;
  int i;
  
  for (i=1;i<=no_of_categories;i++)
    {
      total_left_count += left_count[i];
      total_right_count += right_count[i];
    }
  
  total_count = total_left_count + total_right_count;
  if (!total_count) return(0);
  
  for (i=1;i<=no_of_categories;i++)
    {
      temp = 0;
      if (total_left_count) temp = left_count[i]/total_left_count;
      if (total_right_count) temp -= right_count[i]/total_right_count;
      
      if (temp < 0) goodness += -1.0 * temp;
      else goodness += temp;
    }
  
  total_left_count /= total_count;
  total_right_count /= total_count;
  
  twoing_val = total_left_count * total_right_count * goodness * goodness / 4;
  
  if (twoing_val == 0) return(HUGE_VAL);
  else return(1.0/twoing_val);
} 

/************************************************************************/
/************************************************************************/
