/****************************************************************/
/* Copyright 1993, 1994                                         */
/* Johns Hopkins University			                */
/* Department of Computer Science		                */
/****************************************************************/
/* Contact : murthy@cs.jhu.edu					*/
/****************************************************************/
/* File Name :	util.c						*/
/* Author : Sreerama K. Murthy					*/
/* Last modified : July 1994					*/
/* Contains modules : 	error					*/
/*			myrandom				*/
/*			vector					*/
/*			free_vector				*/
/*			ivector					*/
/*			free_ivector				*/
/*			dvector					*/
/*			free_dvector				*/
/* Uses modules in : None.					*/
/* Is used by modules in :	classify.c			*/
/*				compute_impurity.c		*/
/*				display.c			*/
/*				gendata.c			*/
/*				impurity_measure.c		*/
/*				load_data.c			*/
/*				mktree.c			*/
/*				perturb.c			*/
/*				tree_util.c			*/
/* Remarks       :	These are general-purpose utility       */
/*                      routines used by most of the modules    */
/*                      in the package.	                        */
/****************************************************************/	
#include <stdio.h>


/************************************************************************/
/* Module name : MyLog2                                                  */
/* Functionality : Computes the log-base-2 of a double, and returns      */
/*                 a double.                                              */
/************************************************************************/
double mylog2(x)
     double x;
{
  double log();

  return((double)(log(x)/log((double)2.0)));
}

/************************************************************************/
/* Module name : MyAbs                                                  */
/* Functionality : Computes the absolute value of a double.              */
/************************************************************************/
double myabs(x)
     double x;
{
  if (x < 0) return (-1.0 * x);
  else return(x);
}

/************************************************************************/
/* Module name : error							*/ 
/* Functionality :	Displays an error message, and exits execution	*/
/*			normally.					*/
/************************************************************************/
error(error_text)
     char error_text[];
{
  printf("Runtime Error.\n%s.\nExecution Terminated.\n",error_text);
  exit(1);
}

/************************************************************************/
/* Module name :	myrandom					*/ 
/* Functionality :	Generates a random number between 0 and 1, and	*/
/*			scales it to given range.        		*/
/* Parameters :	above, below : lower and upper limits, respectively on	*/
/*		the random number to be generated.			*/
/* Returns :	a doubleing point number.				*/
/* Calls modules :	drand48 (C library call)			*/
/* Remarks: If your system doesn't support drand48(), you can substitute*/
/*          the call below with any good pseudo random number generator */
/*          function call.                                              */
/************************************************************************/
double myrandom(above,below)
     double above,below;
{
  double drand48();
  
  return ((double)(above + drand48() * (below - above)));
}


/************************************************************************/
/* Module name : Average                                                */
/* Functionality : Computes the average of a double array.               */
/* Parameters: a: double array, indices in the range 1,n                 */
/*             n: length of the double array.                            */
/************************************************************************/
double average(a,n)
     double *a;
     int n;
{
  double sum=0;
  int i;
  
  for (i=1;i<=n;i++) sum += a[i];
  return(sum/n);
}

/************************************************************************/
/* Module name : Min                                                    */
/* Functionality : Returns the minimum entry of a double array           */
/* Parameters: a: double array, indices in the range 1,n                 */
/*             n: length of the double array.                            */
/************************************************************************/
double min(a,n)
     double *a;
     int n;
{
  double xmin;
  int i;
  
  xmin = a[1]; 
  for (i=2;i<=n;i++) if (a[i] < xmin) xmin = a[i];
  return(xmin);
}

/************************************************************************/
/* Module name : SDev                                                   */
/* Functionality : Computes the standard deviation of a double array.    */
/* Parameters: a: double array, indices in the range 1,n                 */
/*             n: length of the double array.                            */
/* Calls Modules: Average                                               */
/************************************************************************/ 
double sdev(a,n)
     double *a;
     int n;
{
  int i;
  double average(),mean;
  double sum=0,sqrt();
 
  mean = average(a,n);
  for (i=1;i<=n;i++)
    sum += ((a[i]-mean)*(a[i]-mean));
  
  if (n > 1) sum = sum/(n-1);
  sum = sqrt(sum);
  return((double)sum);
}

/************************************************************************/
/* Module name : ivector						*/
/* Functionality :	Allocates a 1-D integer array, whose indices	*/
/*			range from "nl" through "nh", and returns a	*/
/*			pointer to this array.				*/
/* Parameters :	nl,nh : lowest and highest indices.			*/
/* Calls modules :	error						*/
/************************************************************************/
int *ivector(nl,nh)
     int nl,nh;
{
  int *v;
  
  v=(int *)malloc((unsigned)(nh-nl+1)*sizeof(int));
  if (v==NULL) error("Ivector : Memory allocation failure.");
  return(v-nl);
}

/************************************************************************/
/* Module name : vector							*/
/* Functionality :	Allocates a 1-D double array, whose indices	*/
/*			range from "nl" through "nh", and returns a	*/
/*			pointer to this array.				*/
/* Parameters :	nl,nh : lowest and highest indices.			*/
/* Calls modules :	error						*/
/************************************************************************/
double *vector(nl,nh)
     int nl,nh;
{
  double *v;
  
  v=(double *)malloc((unsigned)(nh-nl+1)*sizeof(double));
  if (v==NULL) error("Vector : Memory allocation failure.");
  return (v-nl);
}

/************************************************************************/
/* Module name : dvector						*/
/* Functionality :	Allocates a 1-D double array, whose indices	*/
/*			range from "nl" through "nh", and returns a	*/
/*			pointer to this array.				*/
/* Parameters :	nl,nh : lowest and highest indices.			*/
/* Calls modules :	error						*/
/************************************************************************/
double *dvector(nl,nh)
     int nl,nh;
{
  double *v;
  
  v=(double *)malloc((unsigned)(nh-nl+1)*sizeof(double));
  if (v==NULL) error("Dvector : Memory allocation failure.");
  return (v-nl);
}

/************************************************************************/
/* Module name : free_ivector						*/
/* Functionality :	Frees a 1-D integer array. 			*/
/* Parameters :	v : pointer to the array				*/
/*		nl,nh : lowest and highest indices.			*/
/* Remarks: It is possible that the memory deallocation modules do not  */
/*          work well always. This should not be a major problem in most*/
/*          cases, however.                                             */
/************************************************************************/
free_ivector(v,nl,nh)
     int *v,nl,nh;
{
  free((char*)(v+nl));
}

/************************************************************************/
/* Module name : free_vector						*/
/* Functionality :	Frees a 1-D double array. 			*/
/* Parameters :	v : pointer to the array				*/
/*		nl,nh : lowest and highest indices.			*/
/* Remarks: It is possible that the memory deallocation modules do not  */
/*          work well always. This should not be a major problem in most*/
/*          cases, however.                                             */
/************************************************************************/
free_vector(v,nl,nh)
     int nl,nh;
     double *v;
{
  free((char*)(v+nl));
}

/************************************************************************/
/* Module name : free_dvector						*/
/* Functionality :	Frees a 1-D double array. 			*/
/* Parameters :	v : pointer to the array				*/
/*		nl,nh : lowest and highest indices.			*/
/* Remarks: It is possible that the memory deallocation modules do not  */
/*          work well always. This should not be a major problem in most*/
/*          cases, however.                                             */
/************************************************************************/
free_dvector(v,nl,nh)
     int nl,nh;
     double *v;
{
  free((char*)(v+nl));
}

/************************************************************************/
/* Module name :	generate_random_hyperplane			*/ 
/* Functionality :	generates coefficients of a hyperplane randomly.*/
/* Parameters :	array_name, length of the array.                        */
/*              max_value : maximum absolute  value of any coefficient  */
/* Returns :	Nothing.						*/
/* Calls modules :	myrandom        				*/
/* Is called by modules :	oblique_split (mktree.c)		*/
/*				perturb_randomly (perturb.c)		*/
/************************************************************************/
generate_random_hyperplane(array_name,length,max_value)
     double *array_name,max_value;
     int length;
{
  int i;
  double myrandom();
  
  for (i=1;i<=length;i++)
    array_name[i] = myrandom(-1.0 * max_value, max_value);
}
 


/************************************************************************/
/* Module name : Usage                                                  */ 
/* Functionality : Displays a list of possible options for MKTREE, GENDATA*/
/*                 and DISPLAY modules. Is activated in irritatingly    */
/*                 many situations. More precisely, whenever an incorrect*/
/*                 option is specified, or an option is accompanied by  */
/*                 incorrect argument, or when incorrect combinations of*/
/*                 options are used, this module is activated.          */
/* Parameters : pname: name of the program whose options are to be shown*/           
/************************************************************************/
usage(pname)
     char *pname;
{
  if (!strcmp(pname, "mktree"))
    {
      fprintf(stderr,"\n\nUsage: mktree aA:b:Bc:d:D:i:j:Kl:m:M:n:Nop:r:R:s:t:T:uvV:");
      fprintf(stderr,"\nOptions :");
      fprintf(stderr,"\n    -a : Only axis parallel splits.");
      fprintf(stderr,"\n    -A<file to output animation information to>");
      fprintf(stderr,"\n      (Default = No output)");
      fprintf(stderr,"\n    -b : bias towards axis parallel splits (>=1.0)");
      fprintf(stderr,"\n      (Default = 1.0)");
      fprintf(stderr,"\n    -B : Order of coeff. perturbation= Best First");
      fprintf(stderr,"\n    -c<number of classes> ");
      fprintf(stderr,"\n      (Default: computed from data or decision tree)");
      fprintf(stderr,"\n    -d<number of attributes> ");
      fprintf(stderr,"\n      (Default: computed from data or decision tree)");
      fprintf(stderr,"\n    -D<decision tree file>");
      fprintf(stderr,"\n      (Default=<training data>.dt, for outputting.)");
      fprintf(stderr,"\n    -i<#restarts for the perturbation alg.>");
      fprintf(stderr,"\n      (Default=20)");
      fprintf(stderr,"\n    -j<maximum number of random jumps");
      fprintf(stderr,"\n       tried at each local minimum> (Default = 5)");
      fprintf(stderr,"\n    -K : CART-linear combinations mode");
      fprintf(stderr,"\n    -l<log file>  (Default=oc1.log)");
      fprintf(stderr,"\n    -m<maximum number of random jumps");
      fprintf(stderr,"\n       tried at each local minimum> (Default = 5)");
      fprintf(stderr,"\n    -M<file to output misclassified instances to>");
      fprintf(stderr,"\n      (Default = No output)");
      fprintf(stderr,"\n    -n<number of training examples> ");
      fprintf(stderr,"\n    -N : No normalization at each tree node.");
      fprintf(stderr,"\n    -o : Only oblique splits.");
      fprintf(stderr,"\n    -p<portion of training set to be used in pruning>");
      fprintf(stderr,"\n      (Default=0.10 i.e., 10%)");
      fprintf(stderr,"\n    -r<#restarts for the perturbation alg.>");
      fprintf(stderr,"\n      (Default=20)");
      fprintf(stderr,"\n    -R<cycle_count>");
      fprintf(stderr,"\n      Order of coeff. pert.= Random. Perturb Cycle_Count times.");
      fprintf(stderr,"\n    -s<integer seed for the random number generator>");
      fprintf(stderr,"\n    -t<file containing training data> (Default=None)");
      fprintf(stderr,"\n    -T<file containing testing data> (Default=None)");
      fprintf(stderr,"\n    -u : test data is unlabelled. Label it!");
      fprintf(stderr,"\n    -v : verbose if specified once.");
      fprintf(stderr,"\n         very verbose if specified more than once.");
      fprintf(stderr,"\n    -V<#partitions for cross validation>  (Default=0)");
      fprintf(stderr,"\n       (-1 : leave-one-out, 0 = no CV)");
    }
  
 if (!strcmp(pname,"display"))
    {
      fprintf (stderr,"\n\nUsage : display -d:D:eh:o:t:T:vw:x:X:y:Y:");
      fprintf (stderr,"\nOptions :");
      fprintf (stderr,"\n    -d<#dimensions> (Has to be 2)");
      fprintf (stderr,"\n    -D<File containing the Decision tree>");
      fprintf (stderr,"\n      (Default: None)");
      fprintf (stderr,"\n    -e : Erase Mode OFF.");
      fprintf (stderr,"\n       Produce animation without erasing any hyperplanes.");
      fprintf (stderr,"\n    -h<header (title) for the display>");
      fprintf (stderr,"\n      (Default=<datafile>-<decision tree file>)");
      fprintf (stderr,"\n    -o<file to write the PostScript(R) output>");
      fprintf (stderr,"\n      (Default=stdout)");
      fprintf (stderr,"\n    -t or -T <File containing the data points>");
      fprintf (stderr,"\n      (Default: None)");
      fprintf (stderr,"\n    -v : Verbose (Default=FALSE)");
      fprintf (stderr,"\n    -w<wait time between erasing one hyperplane and");
      fprintf (stderr,"\n       showing another, in the animation mode>");
      fprintf (stderr,"\n    -x<minimum x value>");
      fprintf (stderr,"\n      (Default=calculated from point set or 0)");
      fprintf (stderr,"\n    -X<maximum x coord for the display>");
      fprintf (stderr,"\n      (Default=calculated from point set or 1)");
      fprintf (stderr,"\n    -y<minimum y coord for the display>");
      fprintf (stderr,"\n      (Default=calculated from point set or 0)");
      fprintf (stderr,"\n    -Y<maximum y coord for the display>");
      fprintf (stderr,"\n      (Default=calculated from point set or 1)");
    }
  
  if (!strcmp(pname,"gendata"))
    {
      fprintf (stderr,"\n\nUsage : gendata -a:b:c:d:D:n:N:o:s:t:T:uv");
      fprintf (stderr,"\nOptions :");
      fprintf (stderr,"\n    -a<all attribute values >= this number>");
      fprintf (stderr,"\n         (Default=0)");
      fprintf (stderr,"\n    -b<all attribute values <= this number>");
      fprintf (stderr,"\n         (Default=1)");
      fprintf (stderr,"\n    -c<#categories. (Default=2)");
      fprintf (stderr,"\n    -d<#dimensions> (Default=2)");
      fprintf (stderr,"\n    -D<File containing the Decision tree>");
      fprintf (stderr,"\n      (Default: None)");
      fprintf (stderr,"\n    -n or N <number of points to be generated>");
      fprintf (stderr,"\n      (Default: None)");
      fprintf (stderr,"\n    -o<file to write the generated data> (Default=stdout)");
      fprintf (stderr,"\n    -t<file to write the generated data> (Default=stdout)");
      fprintf (stderr,"\n    -T<file to write the generated data> (Default=stdout)");
      fprintf (stderr,"\n    -s<integer seed for the random number generator>");
      fprintf (stderr,"\n    -u : Unlabeled Data. (Default=FALSE)");
      fprintf (stderr,"\n    -v : Verbose (Default=FALSE)");
    }

  fprintf (stderr,"\n\n");
  exit(0);
}

/************************************************************************/
/************************************************************************/
