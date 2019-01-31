/****************************************************************/
/* Copyright 1993, 1994                                         */
/* Johns Hopkins University			                */
/* Department of Computer Science		                */
/****************************************************************/
/* Contact : murthy@cs.jhu.edu                                  */
/****************************************************************/
/* File Name : load_data.c 					*/
/* Author : Sreerama K. Murthy					*/
/* Last modified : June 1995					*/
/* Contains modules : 	load_points				*/
/*			allocate_point_array			*/
/*			shuffle_points				*/
/* Uses modules in :	oc1.h 					*/
/*			util.c					*/
/* Is used by modules in :	mktree.c			*/
/*                              display.c                       */
/* Remarks       :	                                        */
/*			Throughout this program and others	*/
/*			in OC1, "points" refer to data instances*/
/*			or examples. This usage is due to the 	*/
/*			geometric interpretation of the	        */
/*			attribute space used by OC1.		*/
/*                7/28/95: load_points routine is corrected to  */
/*                      read in missing values correctly.       */
/****************************************************************/		

#include "oc1.h"

extern int no_of_dimensions;
extern int no_of_categories;
extern int no_of_missing_values,unlabeled;
extern float *min_attribute_value,*avg_attribute_value;
extern float *sdev_attribute_value;

int *category_array = NULL;

/************************************************************************/
/* Module name : load_points						*/
/* Functionality : Downloads data points from the input file into the	*/
/*		   array train_points.					*/
/*		   Dynamically allocates space for the points read.	*/
/*                 Computes the number of dimensions (attributes) and   */
/*                 number of classes.                                   */
/* Parameters :	infile :	File pointer to the input file.		*/
/*		points_ptr : 	pointer to the array into which the	*/
/*				data points are to be loaded.		*/
/* Returns :	Number of points read.					*/
/* Calls modules :	error (util.c)					*/
/*			allocate_point_array				*/
/* Is called by modules :	read_data (mktree.c)			*/
/*				main (display.c)			*/
/* Remarks :	Assumes that the no_of_dimensions and no_of_categories 	*/
/* 		are set.		 				*/ 
/************************************************************************/
int load_points(infile,points_ptr)
     FILE *infile;
     POINT ***points_ptr;
{
  int points_allocated = 0,i,j,categories_unknown = FALSE;
  int point_count, training_set = TRUE;
  float temp;
  char c,delim[10];
  POINT **allocate_point_array();
  POINT **array_name = NULL;
  
  no_of_missing_values = 0;
  if (!unlabeled && !no_of_categories) 
    {
      categories_unknown = TRUE; 
      no_of_categories = 1;
      category_array = (int *)malloc(no_of_categories * sizeof(int));
      category_array -= 1;
    }
  else if (category_array != NULL) training_set = FALSE;
  
  if (!no_of_dimensions) no_of_dimensions = MAX_NO_OF_ATTRIBUTES;
  
  point_count = 0;
  while (TRUE)
    {
      point_count++;
      if (point_count > points_allocated)
	{
	  if (points_allocated == 0) 
	    {
	      array_name = allocate_point_array(array_name,10,0);
	      points_allocated = 10;
	    }
	  else 
	    {
	      array_name = allocate_point_array(array_name,points_allocated*2,
						points_allocated);
	      points_allocated *= 2;
	    }
	}

      if (point_count == 1 && 
	  no_of_dimensions == MAX_NO_OF_ATTRIBUTES)
	/*count the number of dimensions of the first line in the datafile,
	  and set it as the no_of_dimensions. Use this value for reading in
	  the subsequent lines. The last entry of any line is taken as the
	  category value, unless unlabeled is TRUE. */
	{
	  int dim=0;
	  temp = 99999.0;
	  
	  while (TRUE)
	    {
	      c = (char)getc(infile);
	      if (c == '\n')
		{
		  if (temp != 99999.0)
		    { 
		      if (unlabeled == TRUE)
			array_name[1]->dimension[++dim] = temp;
		      else
			{
			  i = array_name[1]->category = (int)temp;
			  if (temp - i != 0)
			    error("Load_Points: Classes need to be integers.");
			  if (categories_unknown)
			    category_array[1] = array_name[1]->category;
			  else if (category_array != NULL)
			    for (i=1;i<=no_of_categories;i++)
			      if (temp == category_array[i]) 
				{ array_name[i]->category = i; break; }
			      else if (i < 1 || i > no_of_categories)
				{ 
				  printf ("Load_Points: Classes should be integers in [1,%d].\n",
					  no_of_categories);
				  error("");
				}
			}
		      
		      no_of_dimensions = dim; 
		      for (i=1;i<=points_allocated;i++)
			{
			  array_name[i]->dimension += 1;
			  array_name[i]->dimension = (float *)realloc
			    (array_name[i]->dimension,
			     no_of_dimensions * sizeof(float));
			  array_name[i]->dimension -= 1;
			}
		      break;
		    }
		}
	      if (c == ',' || isspace(c)) continue;
	      if (isalpha(c)) 
		error("Load_Points: Alphabetic character in datafile.");
	      if (temp != 99999.0)
		{
		  if (++dim > no_of_dimensions)
		    {
		      fprintf(stderr,"Load_Points: Too many attributes.\n");
		      fprintf(stderr,"Redefine MAX_NO_OF_ATTRIBUTES in oc1.h and");
		      fprintf(stderr,"remake and rerun Mktree.\n");
		      error("");
		    }
		  array_name[point_count]->dimension[dim] = temp;
		}
	      
	      if (c == '?')  
		{
		  temp = MISSING_VALUE;
		  no_of_missing_values++;
		}
	      else
		{
		  ungetc(c,infile);
		  fscanf(infile,"%f",&temp);
		}
	    }
	}
      else
	{
	  for (j=1;j<=no_of_dimensions;j++)
	    {
	      c = (char)getc(infile);
	      while (c == ',' || isspace(c)) c = (char)getc(infile);
	      if (c == '?') 
		{
		  array_name[point_count]->dimension[j] = MISSING_VALUE;
		  no_of_missing_values++;
		  i = 1;
		}
	      else
		{
		  ungetc(c,infile);
		  i = fscanf(infile,"%f", &(array_name[point_count]->dimension[j]));
		  if (i != 1)
		    {
		      if (j>1) error("Load_Points1: Object with too few attributes.");
		      else break;
		    }
		}
	    }

	  if (i != 1) break;
	  if (unlabeled == TRUE)
	    {
	      array_name[point_count]->val = (double)0.0;
	      continue;
	    }
	  
	  c = (char) getc(infile);
	  if (!isspace(c) && c != ',') ungetc(c,infile);
	  if (fscanf(infile,"%d",&i) != 1)
	    error("Load_Points2: Object with too few attributes.");
	  if (categories_unknown)
	    {
	      for (j=1;j<=no_of_categories;j++)
		if (i == category_array[j]) break;
	      if (j > no_of_categories)
		{
		  no_of_categories++;
		  category_array += 1;
		  category_array = (int *)realloc(category_array,
						  no_of_categories * sizeof(int));
		  category_array -= 1;
		  category_array[no_of_categories] = i;
		}
	    }
	  else if (i<1 || i>no_of_categories)
	    { 
	      printf ("Load_Points: Classes should be integers in [1,%d].\n",
		      no_of_categories);
	      error("");
	    }
	  array_name[point_count]->category = i;
	}
      array_name[point_count]->val = (double)0.0;
    }

  point_count--;
  if (point_count != points_allocated)
    array_name = allocate_point_array(array_name,point_count,points_allocated);
  
  if ( (!unlabeled && categories_unknown && training_set) ||
      (!training_set && category_array))
    {
      /*There are no_of_categories classes.
	If all these numbers are between 1 and no_of_categories, then
	we don't need any remapping. */
      for (i=1;i<=no_of_categories;i++)
	if (category_array[i] < 1 || category_array[i] > no_of_categories)
	  break;
      
      if (i <= no_of_categories)
	{
	  if (training_set)
	    {
	      printf("Remapping class numbers:\n");
	      for (i=1;i<=no_of_categories;i++)
		if (i != category_array[i])
		  printf("\t%d To %d\n",category_array[i],i);
	    }
	  for (i=1;i<=point_count;i++)
	    for (j=1;j<=no_of_categories;j++)
	      if (category_array[j] == array_name[i]->category) 
		{
		  array_name[i]->category = j;
		  break;
		}
	}
    }

  fill_missing_values(array_name,point_count);
  
  *points_ptr = array_name;
  return(point_count);
}


/************************************************************************/
/* Module name : allocate_point_array					*/ 
/* Functionality :	Allocates or reallocates "array_name" to be an	*/
/*			array of pointers (to POINT structures), of	*/
/*			size "size". Fully allocates all the POINT	*/
/*			structures also.				*/
/* Parameters :	array_name : name of the array to be (re)allocated.	*/
/*		size	   : number of points to be allocated.		*/
/*		prev_size  : 0 if array_name doesn't exist already	*/
/*			     current size otherwise.			*/
/* Returns :	pointer to the allocated array.				*/
/* Calls modules :	error (util.c)					*/
/*			vector (util.c)					*/
/* Is called by modules : 	load_points				*/
/************************************************************************/
POINT **allocate_point_array(array_name,size,prev_size)
     POINT **array_name;
     int size,prev_size;
{
  int i;
  
  if (prev_size == 0)
    {
      if (array_name != NULL) 
	free((char *)(array_name+1)); 
      
      array_name = (struct point **)malloc
	((unsigned)size * sizeof(struct point *)); 
      if (!array_name)
	error("Allocate_Point_Array: Memory Allocation Failure 1.");
      
      array_name -= 1; /* All indices start from 1*/
      
      for (i=1;i<=size;i++)
	{
	  array_name[i] = (struct point *)malloc((unsigned) sizeof(struct point)); 
	  if (!array_name[i])
	    error("Allocate_Point_Array : Memory Allocation failure 2.");
	}
      
      for (i=1;i<=size;i++)
	array_name[i]->dimension = vector(1,no_of_dimensions);
    }
  else
    {
      array_name += 1;
      array_name = (struct point **)realloc
	(array_name, (unsigned)size * sizeof(struct point *)); 
      if (!array_name)
	error("Allocate_Point_Array: Memory Allocation Failure 3.");
      
      array_name -= 1; /* All indices start from 1*/
      
      if (prev_size >= size) return(array_name);
      
      for (i=prev_size+1;i<=size;i++)
	{
	  array_name[i] = (struct point *)malloc((unsigned) sizeof(struct point)); 
	  if (!array_name[i])
	    error("Allocate_Point_Array : Memory Allocation failure 4.");
	}
      
      for (i=prev_size+1;i<=size;i++)
	array_name[i]->dimension = vector(1,no_of_dimensions);
    }
  
  return(array_name);
}


/************************************************************************/
/* Module name :	shuffle_points					*/ 
/* Functionality :	Pseudo-randomly shuffles the points in the	*/
/*			array "array_name". 				*/
/*			for i = 1 to n, do				*/
/*			  swap point i with the point at a random 	*/
/*			  position between 1 and n.			*/ 
/* Parameters :	array_name : Point array which is to be shuffled.	*/
/*		count	: Number of entries in the array.		*/
/* Returns : Nothing.							*/
/* Calls modules :	myrandom (util.c)				*/
/* Is called by modules :	load_points				*/
/* Remarks :	Achieves shuffling just by swapping pointers, thus 	*/
/*		not spending time on allocation/deallocation.		*/
/************************************************************************/
shuffle_points(array_name,count)
     POINT **array_name;
     int count;
{
  int i,newposition;
  POINT *temp_point;
  
  for (i=1;i<=count;i++)
    {
      newposition = (int)myrandom(1.0,(float)count);
      /* shuffle position "i" with "newposition" */
      
      temp_point = array_name[i];
      array_name[i] = array_name[newposition];
      array_name[newposition] = temp_point;
    }
}

/************************************************************************/
/* Module name : fill_missing_values                                    */ 
/* Functionality : This module fills in the missing values to be the    */
/*                 mean values of the respective attributes.            */
/* Parameters :    points: array of points to be filled.                */
/*                 no_of_points: number of points.                      */
/* Returns : Nothing.                                                   */
/* Calls modules : vector (util.c)                                      */
/*                 free_vector (util.c)                                 */
/*                 average (util.c)                                     */
/* Is called by modules :  load_data                                    */
/* Important Variables used : MISSING_VALUE : A special number used for */
/*                            temporarily flaging missing values.       */
/*                            Defined in oc1.h.                         */
/************************************************************************/
fill_missing_values(points,no_of_points)
     struct point **points;
     int no_of_points;
{
  int i,j,count;
  float avg,average(),*temp;
  
  temp = vector(1,no_of_points);
  
  for (j=1;j<=no_of_dimensions;j++)
    {
      count = 0;
      for (i=1;i<=no_of_points;i++)
	if (points[i]->dimension[j] == MISSING_VALUE) 
	  { count++; temp[i] = 0;}
	else temp[i] = points[i]->dimension[j];
      
      if (count)
	{
	  avg = average(temp,no_of_points);
	  for (i=1;i<=no_of_points && count>0;i++)
	    if (points[i]->dimension[j] == MISSING_VALUE)
	      {count--; points[i]->dimension[j] = avg;}
	}
    }
  
  free_vector(temp,1,no_of_points);
}

/*****************************************************************/
/*****************************************************************/
