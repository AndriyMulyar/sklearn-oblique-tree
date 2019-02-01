/****************************************************************/
/* Copyright 1993, 1994                                         */
/* Johns Hopkins University			                */
/* Department of Computer Science		                */
/****************************************************************/
/* Contact : murthy@cs.jhu.edu					*/
/****************************************************************/
/* File Name : gendata.c					*/
/* Author : Sreerama K. Murthy					*/
/* Last modified : July 1994					*/
/* Contains modules : 	main					*/
/* Uses modules in :	oc1.h					*/
/*			util.c					*/ 
/*			tree_util.c				*/
/*			classify.c				*/
/* Is used by modules in :	none.				*/
/****************************************************************/
#include "oc1.h"

char *pname;
char test_data[LINESIZE],misclassified_data[LINESIZE];
int unlabeled=FALSE,no_of_dimensions=0,no_of_categories=0;
int verbose=FALSE,no_of_missing_values = 0;
void srand48();

/************************************************************************/
/* Module name : main							*/ 
/* Functionality : 	This module reads in a decision tree, and 	*/
/*			generates a random dataset that has zero error  */
/*			when classified by the above tree. If no 	*/
/*			decision tree is input, the classes of the 	*/
/*			examples in the dataset are assigned randomly.	*/
/* Parameters :	argc, argv : see any standard C textbook.		*/
/* Returns :	None.							*/
/* Calls modules :	read_tree (tree_util.c)	        		*/
/*			allocate_point_array (load_data.c)		*/
/*			classify (classify.c)				*/
/*			print_point (classify.c)			*/
/*		        usage (util.c)					*/
/*			error (util.c)					*/
/*			ivector (util.c)				*/
/*			free_ivector (util.c)				*/
/*			myrandom (util.c)				*/
/* Is called by modules :	none.					*/
/************************************************************************/
main(argc,argv)
     int argc;
     char *argv[];
{
  extern char *optarg;
  extern int optind;
  int c1,i,j,no_of_samples;
  int *point_count;
  float above,below;
  char decision_tree[LINESIZE];
  struct point **points_array = NULL,**allocate_point_array();
  struct tree_node *root = NULL,*read_tree();
  FILE *outfile;
  
  strcpy(test_data,"\0");
  strcpy(decision_tree,"\0");
  above = 0.0;
  below = 1.0;
  
  pname = argv[0];
  if (argc == 1) usage(pname);
  while ((c1 = getopt (argc, argv, "a:b:c:d:D:n:o:s:t:T:uv")) != EOF)
    switch (c1)
      {
      case 'a': 
	above = atof (optarg);
	/* All numbers generated are more than this value. */
	break;
      case 'b':   
	below = atof (optarg);
	/* All numbers generated are less than this value. */
	break;
      case 'c':   
	no_of_categories = atoi (optarg);
	break;
      case 'd':
	no_of_dimensions = atoi (optarg);
	break;
      case 'D':   /*Decision Tree */
	strcpy(decision_tree,optarg);
	break;
      case 'n':   
	no_of_samples = atoi (optarg);
	break;
      case 'o':
	strcpy(test_data,optarg);
	break;
      case 's':   /*Seed for the random number generator */
	srand48(atol(optarg));
	break;
      case 't':
	strcpy(test_data,optarg);
	break;
      case 'T':
	strcpy(test_data,optarg);
	break;
      case 'u':   
	unlabeled = TRUE; break;
      case 'v':   
	verbose = TRUE; break;
      default:    usage(pname);
      }
  
  if (no_of_samples <= 0 || below <= above) usage(pname); 
  if (strlen(decision_tree))
    {
      root = read_tree(decision_tree);
      if (verbose && root != NULL) 
	fprintf(stderr,"Decision tree read from %s.\n",decision_tree);
    }
  else
    {
      if (!no_of_dimensions) no_of_dimensions = 2;
      if (!no_of_categories) no_of_categories = 2;
    } 
  if (verbose) 
    fprintf(stderr,"Number of attributes = %d, Number of classes = %d\n",
            no_of_dimensions, no_of_categories);
  
  points_array = allocate_point_array(points_array,no_of_samples,0);
  
  for (i=1;i<=no_of_samples;i++)
    for (j=1;j<= no_of_dimensions;j++)
      points_array[i]->dimension[j] = myrandom(above,below);
  
  printf("%d instances generated.\n",no_of_samples);
  if ((outfile = fopen(test_data,"w")) == NULL) outfile = stdout;
  
  if (unlabeled != TRUE)
    {
      if (root != NULL)
	classify(points_array,no_of_samples,root,test_data);
      else
	{
	  for (i=1;i<=no_of_samples;i++)
	    {
	      points_array[i]->category = (int)myrandom(1.0,
                                           1.0*(no_of_categories+1));
	      print_point(outfile,points_array[i],FALSE);
	    }
	  fclose(outfile);
	}
    }
  else
    {
      for (i=1;i<=no_of_samples;i++)
	print_point(outfile,points_array[i],TRUE);
      fclose(outfile);
    }
  
  if (verbose && !unlabeled)
    {
      point_count = ivector(1,no_of_categories);
      for (i=1;i<=no_of_categories;i++) point_count[i]=0;
      for (i=1;i<=no_of_samples;i++)
	point_count[points_array[i]->category]++;
      
      for (i=1;i<=no_of_categories;i++)
	fprintf(stderr,"\tCategory %d : %d points\n",i,point_count[i]);
    }
  
  if (strlen(test_data)) printf("Instances written to %s.\n", test_data);
  
  free_ivector(point_count,1,no_of_categories);
}

/************************************************************************/
/************************************************************************/
