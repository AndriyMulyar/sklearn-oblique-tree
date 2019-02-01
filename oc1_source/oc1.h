/****************************************************************/
/* Copyright 1993, 1994                                         */
/* Johns Hopkins University			                */
/* Department of Computer Science		                */
/****************************************************************/
/* Contact : murthy@cs.jhu.edu					*/
/****************************************************************/
/* File Name : oc1.h                                            */
/* Author : Sreerama K. Murthy					*/
/* Last modified : July 1994					*/
/* Contains modules : Data structure and constant definitions   */
/* Is used by modules in : All *.c files, except util.c.        */
/****************************************************************/
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* Change the following statement to use a different impurity measure. */

#define IMPURITY hellinger_distance()
/* possible values are: maxminority                             */
/*			summinority				*/
/*			variance				*/
/*			info_gain				*/
/*			gini_index				*/
/*			twoing					*/
/*                      hellinger_distance                                  */


#define NO_OF_STD_ERRORS	0 /* used for cost complexity pruning, 
                                     in prune.c */
#define SEQUENTIAL 		0
#define BEST_FIRST 	 	1	
#define RANDOM                  2

#define CORRECT 		1
#define INCORRECT 		0

#define TRUE 			1
#define FALSE 			0

#define LEFT 			0
#define RIGHT 			1

#define LESS_THAN 		0
#define MORE_THAN 		1

#define MAX_COEFFICIENT 	1.0
#define MAX_NO_OF_ATTRIBUTES	4020
#define MAX_DT_DEPTH 		50 
#define MAX_NO_OF_STAGNANT_PERTURBATIONS 	10
#define MAX_CART_CYCLES         100

#define TOLERANCE		0.0001
#define TOO_SMALL_THRESHOLD	2.0
#define TOO_SMALL_FOR_ANY_SPLIT		3
#define TOO_SMALL_FOR_OBLIQUE_SPLIT	2 * no_of_dimensions

#define TRAIN			1
#define	TEST			2

#define LINESIZE 		80000
#define MISSING_VALUE           -1.0 * HUGE_VAL

#define translatex(x) ((x - xmin) * (pmaxx - pminx) / (xmax - xmin) + pminx)
#define translatey(y) ((y - ymin) * (pmaxy - pminy) / (ymax - ymin) + pminy)



typedef struct point
 {
   float *dimension;
   int category;
   double val; /*Value obtained by substituting this point in the 
                equation of the hyperplane under consideration.
                This field is maintained to avoid redundant
                computation. */
 }POINT;

struct endpoint
 {
  float x,y;
 };

typedef struct edge
 {
  struct endpoint from,to;
 }EDGE;

struct tree_node
 {
  float *coefficients;
  int *left_count, *right_count;
  struct tree_node *parent,*left,*right;
  int left_cat,right_cat;
  char label[MAX_DT_DEPTH];
  float alpha; /* used only in error_complexity pruning. */
  int no_of_points;
  EDGE edge; /* used only in the display module. */
 };

struct unidim
 {
  float value;
  int cat;
 };

struct test_outcome
 {
  float leaf_count,tree_depth;
  float accuracy;
  int *class; /*For each class, store the number of correctly
                classified examples and total number of examples */
 };

void error(),free_ivector(),free_vector(),free_dvector();
float myrandom(),*vector();
double *dvector();
int *ivector();
float average(),sdev();
