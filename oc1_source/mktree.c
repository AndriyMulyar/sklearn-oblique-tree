/****************************************************************/
/* Copyright 1993, 1994                                         */
/* Johns Hopkins University			                */
/* Department of Computer Science		                */
/****************************************************************/
/* Contact : murthy@cs.jhu.edu					*/
/****************************************************************/
/* File Name : mktree.c						*/
/* Author : Sreerama K. Murthy					*/
/* Last modified : September 1994		                */
/*                 (unnormalize_hyperplane corrected - 9/21/94).*/
/* Contains modules : 	main					*/
/*			allocate_structures			*/
/*			deallocate_structures			*/
/*			build_tree				*/
/*			build_subtree				*/
/*			axis_parallel_split			*/
/*			oblique_split				*/
/*			cross_validate				*/
/*			print_log_and_exit			*/
/*			mktree_help				*/
/* Uses modules in :	oc1.h					*/
/*			util.c					*/
/*			load_data.c				*/
/*			classify.c				*/
/*			train_util.c				*/
/*			classify_util.c				*/
/*			compute_impurity.c			*/
/*			perturb.c				*/
/*			prune.c					*/
/* Is used by modules in :	None.				*/
/* Remarks       :	This file has the OC1 modules, that	*/
/*			build the decision trees recursively.	*/
/****************************************************************/
#include "oc1.h"

char * pname;
char dt_file[LINESIZE], animation_file[LINESIZE], train_data[LINESIZE];
char test_data[LINESIZE], misclassified_data[LINESIZE];
char log_file[LINESIZE];

int no_of_dimensions = 0, no_of_coeffs, no_of_categories = 0;
int no_of_restarts = 20, no_of_folds = 0;
int normalize = TRUE;
int unlabeled = FALSE, verbose = FALSE, veryverbose = FALSE;
int order_of_perturbation = BEST_FIRST;
int oblique = TRUE;
int axis_parallel = TRUE;
int cart_mode = FALSE;
int coeff_modified = FALSE;
int cycle_count = 0;
int * left_count = NULL, * right_count = NULL;
int max_no_of_random_perturbations = 5;
int no_of_stagnant_perturbations, no_of_missing_values = 0;
int no_of_train_points = 0, no_of_test_points = 0;
int stop_splitting();

double compute_impurity();
double * coeff_array, * modified_coeff_array, * best_coeff_array;
double prune_portion = 0.1;
double myabs(), ap_bias = 1.0;
double zeroing_tendency = 0.1;
double * attribute_min, * attribute_avg, * attribute_sdev;
double * temp_val;

void srand48();

struct unidim * candidates;
struct test_outcome estimate_accuracy();

FILE * animationfile = NULL;
FILE * perturb_file = NULL;

POINT ** train_points = NULL, ** test_points = NULL;
struct tree_node * sklearn_root_node = NULL;
int no_of_train_points;
/************************************************************************/
/* Module name : main							*/
/* Functionality :	Accepts user's options as input, sets control	*/
/*			variables accordingly, and invokes the appro-	*/
/*			priate data-reading, tree-building and 		*/
/*			classifying routines. 				*/
/* Parameters :	argc,argv : See any standard C textbook for details.	*/
/* Returns :	nothing.						*/
/* Calls modules :	mktree_help 					*/
/*			print_log_and_exit				*/
/*			read_data (load_data.c)				*/
/*			allocate_structures				*/
/*			build_tree					*/
/*			write_tree (train_util.c)			*/
/*			cross_validate					*/
/*			deallocate_structures				*/
/*			read_tree (classify_util.c)			*/
/*			estimate_accuracy (classify.c)			*/
/*			classify (classify.c)				*/
/* Is called by modules : None.						*/
/************************************************************************/
main(argc, argv)
int argc;
char * argv[]; {
  extern char * optarg;
  extern int optind;
  int c1, leaf_count(), tree_depth();
  int i, j, no_of_correctly_classified_test_points;
  struct tree_node * root = NULL, * build_tree(), * read_tree();
  struct test_outcome result;
  double accuracy;

  strcpy(train_data, "\0");
  strcpy(test_data, "\0");
  strcpy(dt_file, "\0");
  strcpy(animation_file, "\0");
  strcpy(misclassified_data, "\0");
  strcpy(log_file, "oc1.log");

  pname = argv[0];
  if (argc == 1) usage(pname);
  while ((c1 =
      getopt(argc, argv, "aA:b:Bc:d:D:i:j:Kl:m:M:n:Nop:r:R:s:t:T:uvV:")) !=
    EOF)

    switch (c1) {
    case 'a':
      /*Axis parallel splits only */
      oblique = FALSE;
      break;
    case 'A':
      /*File into which all the locations of the
      	  hyperplanes considered by OC1 are output.
      	  The program "display" can be used to see an
      	  animation of the tree building process, using
      	  this output. */
      strcpy(animation_file, optarg);
      break;
    case 'b':
      /* This option specifies the bias the user has
		   toward axis parallel splits, denoted by ap_bias.
		   ap_bias is a positive number greater than or equal
                   to 1.0.  An oblique split will be preferred to an
                   axis parallel split only if
		   axis parallel impurity / oblique impurity > ap_bias */
      ap_bias = atof(optarg);
      if (ap_bias < 1.0) ap_bias = 1.0;
      break;
    case 'B':
      if (oblique == FALSE) usage(pname);
      order_of_perturbation = BEST_FIRST;
      break;
    case 'c':
      no_of_categories = atoi(optarg);
      if (no_of_categories <= 0) usage(pname);
      break;
    case 'd':
      no_of_dimensions = atoi(optarg);
      if (no_of_dimensions <= 0) usage(pname);
      break;
    case 'D':
      /*The decision tree file.
      	  If OC1 is used in the training mode, the
      	  decision tree is output to this file.
      	  In testing mode, the decision tree is read
      	  from this file. */
      strcpy(dt_file, optarg);
      break;
    case 'i':
      /*No. of restarts at each node of the tree.
      	  Retained for compatibility with previous
      	  versions of OC1.*/
      if (oblique == FALSE) usage(pname);
      no_of_restarts = atoi(optarg);
      if (no_of_restarts <= 0) usage(pname);
      break;
    case 'j':
      /*Maximum number of random perturbations tried when
      	  stuck in a local minimum. */
      if (oblique == FALSE) usage(pname);
      max_no_of_random_perturbations = atoi(optarg);
      if (max_no_of_random_perturbations < 0) usage(pname);
      break;
    case 'K':
      /*Run in CART Multivariate mode */
      cart_mode = TRUE;
      break;
    case 'l':
      /*File into which a log of the running of
      	  OC1 is written. Default=oc1.log*/
      strcpy(log_file, optarg);
      break;
    case 'm':
      /*Maximum number of random perturbations tried when
      	  stuck in a local minimum. */
      if (oblique == FALSE) usage(pname);
      max_no_of_random_perturbations = atoi(optarg);
      if (max_no_of_random_perturbations < 0) usage(pname);
      break;
    case 'M':
      /*File into which the test instances on which the
      	  classifier fails are written. */
      strcpy(misclassified_data, optarg);
      break;
    case 'n':
      no_of_train_points = atoi(optarg);
      if (no_of_train_points < 0) usage(pname);
      break;
    case 'N':
      /*Do not normalize attributes before inducing each
      	  oblique hyperplane. */
      normalize = FALSE;
      break;
    case 'o':
      /*Oblique splits only */
      axis_parallel = FALSE;
      break;
    case 'p':
      prune_portion = atof(optarg);
      if (prune_portion < 0 ||
        prune_portion >= 1) usage(pname);
      break;
    case 'r':
      /*Number of restarts at each node of the tree. */
      if (oblique == FALSE) usage(pname);
      no_of_restarts = atoi(optarg);
      if (no_of_restarts <= 0) usage(pname);
      break;
    case 'R':
      if (oblique == FALSE) usage(pname);
      order_of_perturbation = RANDOM;
      cycle_count = atoi(optarg);
      break;
    case 's':
      /*Seed for the random number generator */
      srand48(atol(optarg));
      break;
    case 't':
      /*Data for training. */
      strcpy(train_data, optarg);
      break;
    case 'T':
      /*Data for testing. */
      strcpy(test_data, optarg);
      break;
    case 'u':
      /*Test data is unlabeled. Classify it, and write*/
      unlabeled = TRUE; /*the classified data to <test_data>.classified */
      break;
    case 'v':
      if (verbose == TRUE) veryverbose = TRUE;
      else verbose = TRUE;
      break;
    case 'V':
      no_of_folds = atoi(optarg);
      /* If this is nonzero, V-fold cross-validation is used
         for estimating the accuracy.
         Overrides the -T option.
         This number can be -1, denoting "leave-one-out"
         cross-validation. */
      break;
    default:
      usage(pname);
    }

  if (unlabeled == TRUE && !strlen(test_data)) usage(pname);

  if (strlen(train_data)) {
    if (strlen(test_data) || no_of_folds != 0) {
      j = unlabeled;
      unlabeled = FALSE;
      read_data(train_data, 0);
      unlabeled = j;
    } else {
      i = no_of_train_points;
      j = unlabeled;
      unlabeled = FALSE;
      read_data(train_data, i);
      /*this call loads training, and test point sets.
        see the module header of read_data. */
      unlabeled = j;
    }

    if (verbose) {
      printf("%d training examples loaded from %s.\n",
        no_of_train_points, train_data);
      if (no_of_folds == 0 && no_of_test_points != 0)
        printf("%d testing examples loaded from %s.\n",
          no_of_test_points, train_data);
      if (no_of_missing_values)
        printf("%d missing values filled with respective attribute means.\n",
          no_of_missing_values);
      printf("Attributes = %d, Classes = %d\n",
        no_of_dimensions, no_of_categories);
    }

    allocate_structures(no_of_train_points);

    if (no_of_folds == 0) /* No cross validation. */ {
      if (!strlen(dt_file)) sprintf(dt_file, "%s.dt", train_data);
      root = build_tree(train_points, no_of_train_points, dt_file);
    } else {
      if (no_of_folds == -1) no_of_folds = no_of_train_points;
      if (no_of_folds <= 1 || no_of_folds > no_of_train_points) usage(pname);

      cross_validate(train_points, no_of_train_points);
    }

    deallocate_structures(no_of_train_points);

    if (no_of_folds != 0) print_log_and_exit();
  }

  if (strlen(test_data)) {
    read_data(test_data, -1);
    if (verbose) {
      printf("%d testing examples loaded from %s.\n",
        no_of_test_points, test_data);
      if (no_of_missing_values)
        printf("%d missing values filled with respective attribute means.\n",
          no_of_missing_values);
    }
  }

  if (no_of_test_points) {
    if (root == NULL) {
      if ((root = read_tree(dt_file)) != NULL) {
        if (verbose) printf("Decision tree read from %s.\n", dt_file);
      } else {
        fprintf(stderr, "Mktree: Cannot read %s.\n", dt_file);
        print_log_and_exit();
      }
    }

    if (unlabeled == TRUE) {
      char out_file[LINESIZE];
      FILE * outfile;

      sprintf(out_file, "%s.classified", test_data);
      classify(test_points, no_of_test_points, root, out_file);
      printf("Test instances with labels written to %s.\n", out_file);
    } else {
      result = estimate_accuracy(test_points, no_of_test_points, root);
      printf("accuracy = %.2f\t#leaves = %.2f\tmax depth = %.2f\n",
        result.accuracy, result.leaf_count, result.tree_depth);

      if (verbose)
        for (i = 1; i <= no_of_categories; i++)
          if (result.class[2 * i] != 0)
            printf("Category %d : accuracy = %.2f (%d/%d)\n",
              i, 100.0 * result.class[2 * i - 1] / result.class[2 * i],
              result.class[2 * i - 1], result.class[2 * i]);
    }
  } else {
    result = estimate_accuracy(train_points, no_of_train_points, root);
    printf("acc. on training set = %.2f\t#leaves = %.0f\tmax depth = %.0f\n",
      result.accuracy, result.leaf_count, result.tree_depth);

    if (verbose)
      for (i = 1; i <= no_of_categories; i++)
        if (result.class[2 * i] != 0)
          printf("Category %d : accuracy = %.2f (%d/%d)\n",
            i, 100.0 * result.class[2 * i - 1] / result.class[2 * i],
            result.class[2 * i - 1], result.class[2 * i]);
  }

  print_log_and_exit();
}

/************************************************************************/
/* Module name : allocate_structures					*/
/* Functionality :	Allocates space for some global data structures.*/
/* Parameters : no_of_points : size of the training dataset.		*/
/* Returns :	nothing.						*/
/* Calls modules :	vector (util.c)					*/
/*			ivector (util.c)				*/
/*			dvector (util.c)				*/
/* Is called by modules :	main					*/
/************************************************************************/
allocate_structures(no_of_points)
int no_of_points; {
  int i;

  no_of_coeffs = no_of_dimensions + 1;
  coeff_array = vector(1, no_of_coeffs);
  modified_coeff_array = vector(1, no_of_coeffs);
  best_coeff_array = vector(1, no_of_coeffs);
  left_count = ivector(1, no_of_categories);
  right_count = ivector(1, no_of_categories);
  candidates = (struct unidim * ) malloc((unsigned) no_of_points *
    sizeof(struct unidim));
  candidates -= 1;
  attribute_min = vector(1, no_of_dimensions);
  attribute_avg = vector(1, no_of_dimensions);
  attribute_sdev = vector(1, no_of_dimensions);
  temp_val = dvector(1, no_of_points);
}

/************************************************************************/
/* Module name : deallocate_structures					*/
/* Functionality : Frees space allocated to some global data structures.*/
/* Parameters : no_of_points : size of the training dataset.		*/
/* Returns :	nothing.						*/
/* Calls modules :	free_vector (util.c)				*/
/*			free_ivector (util.c)				*/
/*			free_dvector (util.c)				*/
/* Is called by modules :	main					*/
/************************************************************************/
deallocate_structures(no_of_points)
int no_of_points; {
  free_vector(coeff_array, 1, no_of_coeffs);
  free_vector(modified_coeff_array, 1, no_of_coeffs);
  free_ivector(left_count, 1, no_of_categories);
  free_ivector(right_count, 1, no_of_categories);
  free_vector(best_coeff_array, 1, no_of_coeffs);
  free((char * )(candidates + 1));
  free_vector(attribute_min, 1, no_of_dimensions);
  free_vector(attribute_avg, 1, no_of_dimensions);
  free_vector(attribute_sdev, 1, no_of_dimensions);
  free_dvector(temp_val, 1, no_of_points);
}

/************************************************************************/
/* Module name : Build_Tree                                             */
/* Functionality : Top level tree to induce, prune and write a decision */
/*                 tree to a file.                                      */
/* Parameters : points = array of training instances                    */
/*              no_of_points = instance count                           */
/*              dt_file = file into which the decision tree is to be    */
/*                        written.                                      */
/* Returns :    Pointer to the root of the tree induced.                */
/* Calls modules :  build_subtree                                       */
/*                  prune (prune.c)                                     */
/*                  write_tree (train_util.c)                           */
/*                  allocate_point_array (load_data.c)                  */
/* Is called by modules : main                                          */
/*                        cross_validate                                */
/* Important Variables used :                                           */
/* Remarks :                                                            */
/************************************************************************/
struct tree_node * build_tree(points, no_of_points, dt_file)
struct point ** points;
int no_of_points;
char * dt_file; {
  struct tree_node * prune(), * root, * build_subtree();
  struct point ** allocate_point_array(), ** ptest_points = NULL;
  struct point ** train_points = NULL;
  struct test_outcome result;
  struct tree_node * proot;
  int i, j, k, no_of_ptest_points, no_of_train_points;
  /* initialize the animation file */
  if (strlen(animation_file) && no_of_dimensions == 2 && no_of_folds == 0) {
    animationfile = fopen(animation_file, "w");
    if (verbose) {
      printf("All hyperplane perturbations being written to %s.\n",
        animation_file);
      printf("Use the Display() program for animation.\n");
    }
  }

//  write_header(animationfile);

//  /* divide the training instances into a training set and a pruning set*/
//  no_of_ptest_points = (int)(no_of_points * prune_portion);
//  if (no_of_ptest_points) {
//    no_of_train_points = no_of_points - no_of_ptest_points;
//    if (verbose) printf("%d randomly chosen instances kept away for pruning.\n\n",
//      no_of_ptest_points);
//    /* "randomly chosen" because the points are shuffled in Read_Data. */
//
//    train_points = allocate_point_array(train_points, no_of_train_points, 0);
//    for (i = 1; i <= no_of_train_points; i++) {
//      for (j = 1; j <= no_of_dimensions; j++)
//        train_points[i] -> dimension[j] = points[i] -> dimension[j];
//      train_points[i] -> category = points[i] -> category;
//      train_points[i] -> val = points[i] -> val;
//    }
//
//    ptest_points = allocate_point_array(ptest_points, no_of_ptest_points, 0);
//    for (i = no_of_train_points + 1; i <= no_of_points; i++) {
//      k = i - no_of_train_points;
//      for (j = 1; j <= no_of_dimensions; j++)
//        ptest_points[k] -> dimension[j] = points[i] -> dimension[j];
//      ptest_points[k] -> category = points[i] -> category;
//      ptest_points[k] -> val = points[i] -> val;
//    }
//  } else {
//    train_points = points;
//    no_of_train_points = no_of_points;
//  }
  train_points = points;
  no_of_train_points = no_of_points;



  /* Build the tree recursively. */
  root = build_subtree("\0", train_points, no_of_train_points);

  if (root == NULL) {
    fprintf(stderr, "No split could be found with the current parameter settings.\n");
    fprintf(stderr, "Try increasing the values of restarts and random jumps.\n");
    print_log_and_exit();
  } else root -> parent = NULL;

//  /* Prune.*/
//  if (prune_portion != 0)
//    proot = prune(root, ptest_points, no_of_ptest_points);
//  else proot = root;
    proot = root;

  /* Write the trees to files. */
//  if (strlen(dt_file)) {
//    write_tree(proot, dt_file);
//    if (proot == root) /* No pruning was done. */
//      printf("Unpruned decision tree written to %s.\n", dt_file);
//    else {
//      char temp_str[LINESIZE];
//
//      printf("Pruned decision tree written to %s.\n", dt_file);
//      sprintf(temp_str, "%s.unpruned", dt_file);
//      write_tree(root, temp_str);
//      printf("Unpruned decision tree written to %s.\n", temp_str);
//
//    }
//  }

  root = proot;
  return (root);

}

/************************************************************************/
/* Module name : build_subtree						*/
/* Functionality :	Recursively builds a decision tree. i.e., finds	*/
/*			the best (heuristic) hyperplane separating the 	*/
/*			given set of points, and recurses on both sides	*/
/*			of the hyperplane. The best axis-parallel split	*/
/*			is considered if -o option is not chosen, 	*/
/*			before computing oblique splits.		*/
/* Parameters :	node_str : Label to be assigned to the decision tree	*/
/*		           node to be created. 				*/
/*		cur_points : array of pointers to the points under	*/
/*		             consideration.			       	*/
/*		cur_no_of_points : Number of points.	                */
/* Returns :	pointer to the decision tree node created.		*/
/*		NULL, if a node couldn't be created.			*/
/* Calls modules :	set_counts (compute_impurity.c)			*/
/*			compute_impurity (compute_impurity.c)		*/
/*			axis_parallel_split				*/
/*			vector (util.c)					*/
/*			oblique_split					*/
/*			free_vector (util.c)				*/
/*			error (util.c)					*/
/*			find_values (perturb.c)				*/
/*			largest_element (compute_impurity.c)		*/
/*			build_subtree					*/
/* Is called by modules : 	main					*/
/*				build_tree				*/
/*				build_subtree				*/
/*				cross_validate				*/
/* Important Variables used : 	initial_impurity: "inherent" impurity in*/
/*				the point set under consideration. ie.,	*/
/*				impurity when the separating hyperplane	*/
/*				lies on one side of the point set.	*/
/*				If any amount of perturbations (bounded	*/
/*				by the parametric settings) can not	*/
/*				result in a hyperplane that has a lesser*/
/*				impurity than this value, no new tree	*/
/*				node is created.			*/
/************************************************************************/
struct tree_node * build_subtree(node_str, cur_points, cur_no_of_points)
char * node_str;
POINT ** cur_points;
int cur_no_of_points; {
  struct tree_node * cur_node;
  struct tree_node * build_subtree(), * create_tree_node();
  POINT ** lpoints = NULL, ** rpoints = NULL;
  int i, lindex, rindex, lpt, rpt;
  double oblique_split(), axis_parallel_split(), cart_split();
  double initial_impurity, cur_impurity;
  char lnode_str[MAX_DT_DEPTH], rnode_str[MAX_DT_DEPTH];

  //printf("Current number of points %i\n", cur_no_of_points);
  /* Validation checks */
  if (cur_no_of_points <= TOO_SMALL_FOR_ANY_SPLIT) return (NULL);
  if (strlen(node_str) + 1 > MAX_DT_DEPTH) {
    fprintf(stderr, "Tree growing aborted along this branch. \n");
    fprintf(stderr, "Depth cannot be more than MAX_DT_DEPTH, set in oc1.h.\n");
    return (NULL);
  }

  set_counts(cur_points, cur_no_of_points, 0);

  cur_impurity = initial_impurity = compute_impurity(cur_no_of_points);
  if (cur_impurity == 0.0) return (NULL);

  if (cart_mode) {
    cur_impurity = axis_parallel_split(cur_points, cur_no_of_points);
    if (cur_impurity && (strlen(node_str) == 0 ||
        cur_no_of_points > TOO_SMALL_FOR_OBLIQUE_SPLIT))
      cur_impurity = cart_split(cur_points, cur_no_of_points, node_str);
  } else {
    if (axis_parallel)
      cur_impurity = axis_parallel_split(cur_points, cur_no_of_points);

    if (cur_impurity && oblique && cur_no_of_points > TOO_SMALL_FOR_OBLIQUE_SPLIT) {
      double * ap_coeff_array, oblique_impurity;

      ap_coeff_array = vector(1, no_of_coeffs);
      for (i = 1; i <= no_of_coeffs; i++) ap_coeff_array[i] = coeff_array[i];

      if (normalize) normalize_data(cur_points, cur_no_of_points);
      oblique_impurity = oblique_split(cur_points, cur_no_of_points, node_str);
      if (normalize) {
        unnormalize_data(cur_points, cur_no_of_points);
        unnormalize_hyperplane();
        for (i = 1; i <= no_of_dimensions; i++) attribute_min[i] = 0;
      }

      if (ap_bias * oblique_impurity >= cur_impurity) {
        for (i = 1; i <= no_of_coeffs; i++) coeff_array[i] = ap_coeff_array[i];
        coeff_modified = TRUE;
      } else cur_impurity = oblique_impurity;

      free_vector(ap_coeff_array, 1, no_of_coeffs);
    }
  }

  if (cur_impurity >= initial_impurity) return (NULL);
  /*Can not find any split given current parameter settings. */

  find_values(cur_points, cur_no_of_points);
  set_counts(cur_points, cur_no_of_points, 1);

  if (verbose) {
    if (strlen(node_str)) printf("** \"%s\": ", node_str);
    else printf("** Root: ");
    printf("Left:[");
    for (i = 1; i < no_of_categories; i++) printf("%d,", left_count[i]);
    printf("%d] Right:[", left_count[no_of_categories]);
    for (i = 1; i < no_of_categories; i++) printf("%d,", right_count[i]);
    printf("%d]\n", right_count[no_of_categories]);
  }

  for (i = 1, lpt = 0, rpt = 0; i <= no_of_categories; i++) {
    lpt += left_count[i];
    rpt += right_count[i];
  }

  cur_node = create_tree_node();
  cur_node -> no_of_points = cur_no_of_points;
  strcpy(cur_node -> label, node_str);
  write_hp(cur_node, animationfile);

  if (cur_impurity == 0) return (cur_node);

  lpoints = rpoints = NULL;
  if (left_count[cur_node -> left_cat] != lpt)
  /* Left region is not homogeneous. */
  {
    if ((lpoints = (POINT ** ) malloc((unsigned) lpt * sizeof(POINT * ))) ==
      NULL) error("BUILD_DT : Memory allocation failure.");
    lpoints--;
    lindex = 0;
  }

  if (right_count[cur_node -> right_cat] != rpt)
  /* Right region is not homogeneous. */
  {
    if ((rpoints = (POINT ** ) malloc((unsigned) rpt * sizeof(POINT * ))) ==
      NULL) error("BUILD_DT : Memory allocation failure.");
    rpoints--;
    rindex = 0;
  }

  for (i = 1; i <= cur_no_of_points; i++)
    if (cur_points[i] -> val < 0) {
      if (lpoints != NULL) lpoints[++lindex] = cur_points[i];
    }
  else {
    if (rpoints != NULL) rpoints[++rindex] = cur_points[i];
  }

  if (lpoints != NULL) {
    strcpy(lnode_str, node_str);
    strcat(lnode_str, "l");
    cur_node -> left = build_subtree(lnode_str, lpoints, lpt);
    if (cur_node -> left != NULL)(cur_node -> left) -> parent = cur_node;
    free((char * )(lpoints + 1));
  }

  if (rpoints != NULL) {
    strcpy(rnode_str, node_str);
    strcat(rnode_str, "r");
    cur_node -> right = build_subtree(rnode_str, rpoints, rpt);
    if (cur_node -> right != NULL)(cur_node -> right) -> parent = cur_node;
    free((char * )(rpoints + 1));
  }

  return (cur_node);

}

/************************************************************************/
/* Module name : Cart_Split                                             */
/* Functionality : Implements the CART-Linear Combinations (Breiman et  */
/*                 al, 1984, Chapter 5) hill climbing coefficient       */
/*                 perturbation algorithm.                              */
/* Parameters : cur_points: Array of pointers to the current points.    */
/*              cur_no_of_points:                                       */
/*              cur_label: Label of the tree node for which current     */
/*                         split is being induced.                      */
/* Returns : impurity of the induced hyperplane.                        */
/* Calls modules : cart_perturb (perturb.c)                             */
/*                 cart_perturb_constant (perturb.c)                    */
/*                 write_hyperplane                                     */
/*                 find_values (compute_impurity.c)                     */
/*                 set_counts (compute_impurity.c)                      */
/* Is called by modules : build_subtree                                 */
/* Remarks : See the CART book for a description of the algorithm.      */
/************************************************************************/
double cart_split(cur_points, cur_no_of_points, cur_label)
POINT ** cur_points;
int cur_no_of_points;
char * cur_label; {
  int cur_coeff;
  double cur_error, new_error, prev_impurity, myabs();
  double cart_perturb(), cart_perturb_constant();

  /*Starts with the best axis parallel hyperplane. */
  write_hyperplane(animationfile, cur_label);
  find_values(cur_points, cur_no_of_points);
  set_counts(cur_points, cur_no_of_points, 1);
  cur_error = compute_impurity(cur_no_of_points);
  cycle_count = 0;

  while (TRUE) {
    if (cur_error == 0.0) break;
    cycle_count++;
    if (cycle_count != 1) prev_impurity = cur_error;

    for (cur_coeff = 1; cur_coeff < no_of_coeffs; cur_coeff++) {
      new_error = cart_perturb(cur_points, cur_no_of_points, cur_coeff, cur_error);
      if (alter_coefficients(cur_points, cur_no_of_points)) {
        if (veryverbose)
          printf("\tCART hill climbing for coeff. %d. impurity %.3f -> %.3f\n",
            cur_coeff, cur_error, new_error);
        cur_error = new_error;
        write_hyperplane(animationfile, cur_label);
        if (cur_error == 0) break;
      }
    }
    if (cur_error != 0) {
      new_error = cart_perturb_constant(cur_points, cur_no_of_points, cur_error);
      if (alter_coefficients(cur_points, cur_no_of_points)) {
        if (veryverbose)
          printf("\tCART hill climbing for coeff. %d. impurity %.3f -> %.3f\n",
            no_of_coeffs, cur_error, new_error);
        cur_error = new_error;
        write_hyperplane(animationfile, cur_label);
      }
    }
    if (cycle_count > MAX_CART_CYCLES)
      /* Cart multivariate algorithm can get stuck in some domains.
         Arbitrary tie breaker. */
      break;

    if (cycle_count != 1 && myabs(prev_impurity - cur_error) < TOLERANCE)
      break;
  }

  return (cur_error);

}

/************************************************************************/
/* Module name : Create_Tree_Node                                       */
/* Functionality : Creates a tree node structure, and sets some fields. */
/* Parameters : None.                                                   */
/* Returns : Pointer to the tree node created.                          */
/* Calls modules : error (util.c)                                       */
/*                 vector (util.c)                                      */
/*                 ivector (util.c)                                     */
/*                 largest_element (compute_impurity.c)                 */
/* Is called by modules : build_subtree                                 */
/* Remarks : Assumes that the left_count, right_count arrays and the    */
/*           coeff_array are set correctly.                             */
/************************************************************************/
struct tree_node * create_tree_node() {
  struct tree_node * cur_node;
  int i, largest_element();

  cur_node = (struct tree_node * ) malloc(sizeof(struct tree_node));
  if (cur_node == NULL) error("Create_Tree_Node : Memory allocation failure.");

  cur_node -> coefficients = vector(1, no_of_coeffs);
  for (i = 1; i <= no_of_coeffs; i++) cur_node -> coefficients[i] = coeff_array[i];

  cur_node -> left_count = ivector(1, no_of_categories);
  cur_node -> right_count = ivector(1, no_of_categories);
  for (i = 1; i <= no_of_categories; i++) {
    cur_node -> left_count[i] = left_count[i];
    cur_node -> right_count[i] = right_count[i];
  }

  cur_node -> parent = cur_node -> left = cur_node -> right = NULL;
  cur_node -> left_cat = largest_element(left_count, no_of_categories);
  cur_node -> right_cat = largest_element(right_count, no_of_categories);

  return (cur_node);
}

/************************************************************************/
/* Module name : oblique_split						*/
/* Functionality : 	Attempts to find the hyperplane, at an unrestri-*/
/*			cted orientation, that best separates 		*/
/*			"cur_points" (minimizing the current impurity	*/
/*			measure), using hill climbing and randomization.*/
/* Parameters :	cur_points : array of pointers to the points (samples)	*/
/*			     under consideration.			*/
/*		cur_no_of_points : number of points under consideration.*/
/* Returns :	the impurity measure of the best hyperplane found.	*/
/*		The hyperplane itself is returned through the global	*/
/*		array "coeff_array".					*/
/* Calls modules :	generate_random_hyperplane			*/
/*			find_values (perturb.c)				*/
/*			set_counts (compute_impurity.c)			*/
/*			compute_impurity (compute_impurity.c)		*/
/*			myrandom (util.c)				*/
/*			suggest_perturbation (perturb.c)		*/
/*			perturb_randomly (perturb.c)			*/
/* Is called by modules :	build_subtree				*/
/************************************************************************/
double oblique_split(cur_points, cur_no_of_points, cur_label)
POINT ** cur_points;
int cur_no_of_points;
char * cur_label; {
  char c;
  int i, j, old_nsp, restart_count = 1;
  int alter_coefficients();
  int cur_coeff, improved_in_this_cycle, best_coeff_to_improve;
  double perturb_randomly();
  double cur_error, old_cur_error, best_cur_error, least_error;
  double x, changeinval;
  double new_error, suggest_perturbation();

  /*Start with the best axis parallel hyperplane if axis_parallel is true.
    Otherwise start with a random hyperplane. */
  if (axis_parallel != TRUE) {
    generate_random_hyperplane(coeff_array, no_of_coeffs, MAX_COEFFICIENT);
    coeff_modified = TRUE;
  }

  find_values(cur_points, cur_no_of_points);
  set_counts(cur_points, cur_no_of_points, 1);
  least_error = cur_error = compute_impurity(cur_no_of_points);
  for (i = 1; i <= no_of_coeffs; i++) best_coeff_array[i] = coeff_array[i];
  write_hyperplane(animationfile, cur_label);

  /* Repeat this loop once for every restart*/
  while (least_error != 0.0 && restart_count <= no_of_restarts) {
    if (veryverbose)
      printf(" Restart %d: Initial Impurity = %.3f\n", restart_count, cur_error);

    no_of_stagnant_perturbations = 0;
    if (order_of_perturbation == RANDOM) {
      if (cycle_count <= 0) cycle_count = 10 * no_of_coeffs;
      for (i = 1; i <= cycle_count; i++) {
        if (cur_error == 0.0) break;
        cur_coeff = 0;
        while (!cur_coeff)
          cur_coeff = (int) myrandom(1.0, (double)(no_of_coeffs + 1));

        new_error = suggest_perturbation(cur_points, cur_no_of_points,
          cur_coeff, cur_error);
        if (new_error <= cur_error &&
          alter_coefficients(cur_points, cur_no_of_points)) {
          if (veryverbose)
            printf("\thill climbing for coeff. %d. impurity %.3f -> %.3f\n",
              cur_coeff, cur_error, new_error);
          cur_error = new_error;
          improved_in_this_cycle = TRUE;
          write_hyperplane(animationfile, cur_label);
          if (cur_error == 0) break;
        } else /*Try improving in a random direction*/ {
          improved_in_this_cycle = FALSE;
          j = 0;
          while (cur_error != 0 &&
            !improved_in_this_cycle &&
            ++j <= max_no_of_random_perturbations) {
            new_error = perturb_randomly(cur_points, cur_no_of_points, cur_error);
            if (alter_coefficients(cur_points, cur_no_of_points)) {
              if (veryverbose)
                printf("\trandom jump. impurity %.3f -> %.3f\n",
                  cur_error, new_error);
              cur_error = new_error;
              improved_in_this_cycle = TRUE;
              write_hyperplane(animationfile, cur_label);
            }
          }
        }
      }
    } else /* best_first or sequential orders of perturbation.*/ {
      improved_in_this_cycle = TRUE;
      cycle_count = 0;

      while (improved_in_this_cycle) {
        if (cur_error == 0.0) break;
        cycle_count++;
        improved_in_this_cycle = FALSE;

        if (order_of_perturbation == BEST_FIRST) {
          best_cur_error = HUGE_VAL;
          best_coeff_to_improve = 1;
          old_nsp = no_of_stagnant_perturbations;
        }

        for (cur_coeff = 1; cur_coeff < no_of_coeffs; cur_coeff++) {
          new_error = suggest_perturbation(cur_points, cur_no_of_points,
            cur_coeff, cur_error);
          if (order_of_perturbation == BEST_FIRST) {
            if (new_error < best_cur_error) {
              best_cur_error = new_error;
              best_coeff_to_improve = cur_coeff;
            }
            no_of_stagnant_perturbations = old_nsp;
            if (best_cur_error == 0) break;
          } else if (new_error <= cur_error &&
            alter_coefficients(cur_points, cur_no_of_points)) {
            if (veryverbose)
              printf("\thill climbing for coeff. %d. impurity %.3f -> %.3f\n",
                cur_coeff, cur_error, new_error);
            cur_error = new_error;
            improved_in_this_cycle = TRUE;
            write_hyperplane(animationfile, cur_label);
            if (cur_error == 0) break;
          }
        }

        if (order_of_perturbation == BEST_FIRST &&
          best_cur_error <= cur_error) {
          cur_coeff = best_coeff_to_improve;
          new_error = suggest_perturbation(cur_points, cur_no_of_points,
            cur_coeff, cur_error);
          if (alter_coefficients(cur_points, cur_no_of_points)) {
            if (veryverbose)
              printf("\thill climbing for coeff. %d. impurity %.3f -> %.3f\n",
                cur_coeff, cur_error, new_error);
            cur_error = new_error;
            improved_in_this_cycle = TRUE;
            write_hyperplane(animationfile, cur_label);
          }
        }

        if (cur_error != 0 && !improved_in_this_cycle)
        /*Try improving along a random direction*/
        {
          i = 0;
          while (cur_error != 0 &&
            !improved_in_this_cycle &&
            ++i <= max_no_of_random_perturbations) {
            new_error = perturb_randomly(cur_points, cur_no_of_points,
              cur_error, cur_label);
            if (alter_coefficients(cur_points, cur_no_of_points)) {
              if (veryverbose)
                printf("\trandom jump. impurity %.3f -> %.3f\n",
                  cur_error, new_error);
              cur_error = new_error;
              improved_in_this_cycle = TRUE;
              write_hyperplane(animationfile, cur_label);
            }
          }
        }
      }
    }

    if (cur_error < least_error ||
      (cur_error == least_error && myrandom(0.0, 1.0) > 0.5)) {
      least_error = cur_error;
      for (i = 1; i <= no_of_coeffs; i++) best_coeff_array[i] = coeff_array[i];
    }

    if (least_error != 0 && ++restart_count <= no_of_restarts) {
      generate_random_hyperplane(coeff_array, no_of_coeffs, MAX_COEFFICIENT);
      coeff_modified = TRUE;
      find_values(cur_points, cur_no_of_points);
      set_counts(cur_points, cur_no_of_points, 1);
      cur_error = compute_impurity(cur_no_of_points);
      write_hyperplane(animationfile, cur_label);
    }
  }

  for (i = 1; i <= no_of_coeffs; i++)
    coeff_array[i] = best_coeff_array[i];
  coeff_modified = TRUE;
  find_values(cur_points, cur_no_of_points);
  set_counts(cur_points, cur_no_of_points, 1);
  return (least_error);

}

/************************************************************************/
/* Module name : Alter_Coefficients                                     */
/* Functionality : First checks if any coefficient values are changed   */
/*                 considerably (> TOLERANCE) in the last perturbation. */
/*                 If they are, updates the "val" fields of the points  */
/*                 to correspond to the new hyperplane. Sets the left_  */
/*                 count and right_count arrays.                        */
/* Parameters : cur_points : Array of pointers to the points            */
/*              cur_no_of_points.                                       */
/* Returns : 1  if any coefficient values are altered,                  */
/*           0  otherwise                                               */
/* Calls modules : myabs (util.c)                                       */
/*                 set_counts (compute_impurity.c)                      */
/* Is called by modules : oblique_split                                 */
/* Remarks : Assumes that the arrays coeff_array, modified_coeff_array  */
/*           are set. Assumes that the "val" fields of the points       */
/*           correspond to the coefficient values in coeff_array.       */
/************************************************************************/
int alter_coefficients(cur_points, cur_no_of_points)
struct point ** cur_points;
int cur_no_of_points; {
  int i, j = 0;

  for (i = 1; i <= no_of_coeffs; i++)
    if (myabs(coeff_array[i] - modified_coeff_array[i]) > TOLERANCE) {
      if (i != no_of_coeffs)
        for (j = 1; j <= cur_no_of_points; j++)
          cur_points[j] -> val += (modified_coeff_array[i] - coeff_array[i]) *
          cur_points[j] -> dimension[i];
      else
        for (j = 1; j <= cur_no_of_points; j++)
          cur_points[j] -> val += (modified_coeff_array[i] - coeff_array[i]);

      coeff_array[i] = modified_coeff_array[i];
    }
  if (j != 0) {
    set_counts(cur_points, cur_no_of_points, 1);
    return (1);
  } else return (0);
}

/************************************************************************/
/* Module name : 	axis_parallel_split				*/
/* Functionality : 	Attempts to find the hyperplane, at an axis-	*/
/*			parallel orientation, that best separates	*/
/*			"cur_points" (minimizing the current impurity	*/
/*			measure). 					*/
/* Parameters :	cur_points : array of pointers to the points (samples)	*/
/*			     under consideration.			*/
/*		cur_no_of_points : number of points under consideration.*/
/* Returns :	the impurity of the best hyperplane found.	        */
/*		The hyperplane itself is returned through the global	*/
/*		array "coeff_array".					*/
/* Calls modules :	linear_split (perturb.c)			*/
/*			find_values (perturb.c)				*/
/*			set_counts (compute_impurity.c)			*/
/*			compute_impurity (compute_impurity.c)		*/
/* Is called by modules :	build_subtree				*/
/************************************************************************/
double axis_parallel_split(cur_points, cur_no_of_points)
POINT ** cur_points;
int cur_no_of_points; {
  int i, j, cur_coeff, best_coeff;
  double cur_error, best_error, best_coeff_split_at;
  double linear_split();

  for (i = 1; i <= no_of_coeffs; i++) coeff_array[i] = 0;

  for (cur_coeff = 1; cur_coeff <= no_of_dimensions; cur_coeff++) {
    coeff_array[cur_coeff] = 1;
    for (j = 1; j <= cur_no_of_points; j++) {
      candidates[j].value = cur_points[j] -> dimension[cur_coeff];
      candidates[j].cat = cur_points[j] -> category;
    }
    coeff_array[no_of_coeffs] = -1.0 * (double) linear_split(cur_no_of_points);

    coeff_modified = TRUE;
    find_values(cur_points, cur_no_of_points);
    set_counts(cur_points, cur_no_of_points, 1);
    cur_error = compute_impurity(cur_no_of_points);

    if (cur_coeff == 1 || cur_error < best_error) {
      best_coeff = cur_coeff;
      best_coeff_split_at = coeff_array[no_of_coeffs];
      best_error = cur_error;
    }

    coeff_array[cur_coeff] = 0;
    coeff_array[no_of_coeffs] = 0;

    if (best_error == 0) break;
  }

  coeff_array[best_coeff] = 1;
  coeff_array[no_of_coeffs] = best_coeff_split_at;
  coeff_modified = TRUE;

  return (best_error);
}

/************************************************************************/
/* Module name : Write_Hyperplane                                       */
/* Functionality : This routine is used when the animation option is    */
/*                 chosen, to write intermediate hyperplanes into the   */
/*                 animation file. This creates a temporary tree node   */
/*                 and calls the write_hp module in train_util.c.       */
/* Parameters :  out : File pointer to the animation file.              */
/*               label: Label of the tree node being induced.           */
/* Returns : Nothing.                                                   */
/* Calls modules :  create_tree_node                                    */
/*                  write_hp (train_util.c)                             */
/*                  free_tree_node                                      */
/* Is called by modules : oblique_split                                 */
/* Remarks : As with all other memory deallocation calls in OC1, the    */
/*           ones in this module may also have problems.                */
/************************************************************************/
write_hyperplane(out, label)
FILE * out;
char * label; {
  struct tree_node * temp_node, * create_tree_node();

  if (out == NULL) return;
  temp_node = create_tree_node();
  strcpy(temp_node -> label, label);
  write_hp(temp_node, out);
  /*  deallocate_tree(temp_node);
    free((char *)temp_node); */
}

/************************************************************************/
/* Module name : cross_validate						*/
/* Functionality :	Performs K-fold cross_validation on a training  */
/*                      set.					        */
/* Parameters : points : point set under consideration			*/
/*		no_of_points.						*/
/* Returns :	Nothing.					 	*/
/* Calls modules :	ivector (util.c)				*/
/*			build_tree					*/
/*			estimate_accuracy (classify.c)	                */
/*			error (util.c)					*/
/*			write_tree (train_util.c)			*/
/* Is called by modules :	main					*/
/************************************************************************/
cross_validate(points, no_of_points)
int no_of_points;
POINT ** points; {
  int fold_size, fold_begin, fold_end, i, j;
  POINT ** train_points = NULL, ** test_points = NULL;
  int no_of_train_points, no_of_test_points;
  int no_of_corrects = 0;
  struct tree_node * root, * build_tree();
  struct test_outcome * results, resultsum;
  double sqrt();

  results = (struct test_outcome * ) malloc((unsigned) no_of_folds * sizeof(struct test_outcome));
  if (!results) error("Cross_Validate: Memory Allocation Failure.");
  results--;
  for (i = 1; i <= no_of_folds; i++)
    results[i].class = ivector(1, 2 * no_of_categories);

  fold_size = no_of_points / no_of_folds;
  fold_begin = 1;

  i = no_of_points - (fold_size * no_of_folds);
  test_points = (POINT ** ) malloc((unsigned)(fold_size + i) * sizeof(POINT * ));
  j = no_of_folds * fold_size + i;
  train_points = (POINT ** ) malloc((unsigned) j * sizeof(POINT * ));

  if (test_points == NULL || train_points == NULL)
    error("Cross_Validate : Memory Allocation Failure.");

  test_points--;
  train_points--;

  if (!strlen(dt_file)) sprintf(dt_file, "%s.dt", train_data);
  no_of_folds = 0;
  while (TRUE) {
    no_of_train_points = no_of_test_points = 0;

    fold_end = fold_begin + fold_size - 1;
    if (no_of_points - fold_end < fold_size) fold_end = no_of_points;
    no_of_folds++;

    for (i = 1; i <= no_of_points; i++) {
      points[i] -> val = 0;
      if (i >= fold_begin && i <= fold_end)
        test_points[++no_of_test_points] = points[i];
      else train_points[++no_of_train_points] = points[i];
    }

    if (verbose) printf("Fold %d:\n", no_of_folds);
    if (fold_begin == 1)
      root = build_tree(train_points, no_of_train_points, dt_file);
    else root = build_tree(train_points, no_of_train_points, "");

    results[no_of_folds] = estimate_accuracy(test_points, no_of_test_points, root);

    printf("fold %d: acc. = %.2f\t#leaves = %.0f\tmax. depth = %.0f\n",
      no_of_folds, results[no_of_folds].accuracy,
      results[no_of_folds].leaf_count,
      results[no_of_folds].tree_depth);

    if (fold_end == no_of_points) break;
    else fold_begin = fold_end + 1;
  }

  free((char * )(test_points + 1));
  free((char * )(train_points + 1));

  resultsum.leaf_count = resultsum.tree_depth = resultsum.accuracy = 0;
  resultsum.class = ivector(1, 2 * no_of_categories);
  for (i = 1; i <= 2 * no_of_categories; i++) resultsum.class[i] = 0;

  for (i = 1; i <= no_of_folds; i++) {
    resultsum.leaf_count += results[i].leaf_count;
    resultsum.tree_depth += results[i].tree_depth;
    for (j = 1; j <= 2 * no_of_categories; j++)
      resultsum.class[j] += results[i].class[j];
  }

  resultsum.leaf_count /= no_of_folds;
  resultsum.tree_depth /= no_of_folds;
  for (i = 1; i <= no_of_categories; i++)
    no_of_corrects += resultsum.class[2 * i - 1];
  resultsum.accuracy = (100.0 * no_of_corrects) / no_of_points;

  if (verbose) printf("\nOverall:\t");
  printf("accuracy = %.2f\t#leaves = %.2f\tmax depth = %.2f\n",
    resultsum.accuracy, resultsum.leaf_count, resultsum.tree_depth);

  if (verbose)
    for (i = 1; i <= no_of_categories; i++)
      if (resultsum.class[2 * i] != 0) {
        j = 100.0 * resultsum.class[2 * i - 1] / resultsum.class[2 * i];
        printf("Category %d : accuracy = %.2f (%d/%d)\n",
          i, 100.0 * resultsum.class[2 * i - 1] / resultsum.class[2 * i],
          resultsum.class[2 * i - 1], resultsum.class[2 * i]);
      }

}

/************************************************************************/
/* Module name :	print_log_and_exit				*/
/* Functionality :	prints the log of a run of OC1 into the user-	*/
/*			specified "log_file" (default : oc1.log). Log	*/
/*			mainly consists of the parameter settings for   */
/*                      the particular run.                             */
/* Returns :	Nothing.						*/
/* Calls modules :	none.						*/
/* Is called by modules :	main					*/
/************************************************************************/
print_log_and_exit() {
  FILE * logfile;

  if ((logfile = fopen(log_file, "w")) == NULL) {
    fprintf(stderr, "Mktree: Log file cannot be written to.\n");
    exit(0);
  }

  if (strlen(train_data)) fprintf(logfile, "Training data : %s\n", train_data);

  if (no_of_folds)
    fprintf(logfile, "%d-fold cross validation used to estimate accuracy.\n",
      no_of_folds);
  else
  if (strlen(test_data)) fprintf(logfile, "Testing data : %s\n", test_data);

  fprintf(logfile, "Data is %d-dimensional, having %d classes.\n",
    no_of_dimensions, no_of_categories);

  if (axis_parallel == FALSE)
    fprintf(logfile, "No axis-parallel splits considered.\n");
  if (oblique == FALSE)
    fprintf(logfile, "No oblique splits considered.\n");
  else {
    fprintf(logfile, "Parameters for finding oblique splits at each node :\n");
    fprintf(logfile, "\tNumber of restarts = %d\n", no_of_restarts);
    if (order_of_perturbation == BEST_FIRST)
      fprintf(logfile, "\tOrder of coefficient perturbation = Best First\n");
    else if (order_of_perturbation == RANDOM)
      fprintf(logfile, "\tOrder of coefficient perturbation = Random-%d\n",
        cycle_count);
    else fprintf(logfile, "\tOrder of coefficient perturbation = Sequential\n");
    fprintf(logfile, "\tMaximum number of random perturbations tried at each ");
    fprintf(logfile, "local minimum = %d\n", max_no_of_random_perturbations);
    if (normalize == FALSE)
      fprintf(logfile, "No normalization was used.\n");
  }

  if (strlen(train_data)) {
    if (no_of_folds == 0) {
      if (prune_portion != 0) {
        fprintf(logfile, "Pruned decision tree written to %s.\n", dt_file);
        fprintf(logfile, "Unpruned decision tree written to %s.unpruned.\n",
          dt_file);
      } else
        fprintf(logfile, "Unpruned decision tree written to %s.\n", dt_file);
    } else {
      if (prune_portion != 0) {
        fprintf(logfile, "Pruned tree for the first fold written to %s.\n",
          dt_file);
        fprintf(logfile,
          "Unpruned tree for the first fold written to %s.unpruned.\n",
          dt_file);
      } else
        fprintf(logfile,
          "Unpruned tree of the first fold written to %s.\n", dt_file);
    }
  } else if (strlen(test_data))
    fprintf(logfile, "Decision tree read from %s.\n", dt_file);

  if (strlen(animation_file) && no_of_dimensions == 2) {
    fprintf(logfile, "All intermediate hyperplane locations tried are \n");
    fprintf(logfile, "output to %s.", animation_file);
    fprintf(logfile, "Use the Display program with -A option to see animation.\n");
  }

  if (strlen(misclassified_data) && no_of_folds != 0)
    fprintf(logfile, "Misclassified points written to %s.\n", misclassified_data);

  fprintf(logfile, "\n");

  fclose(logfile);
  exit(0);
}

/************************************************************************/
/* Module name :        read_data                                       */
/* Functionality :      Acts as a front-end to load_points, which is    */
/*                      the module that actually loads points.          */
/*                      Sets the global variables no_of_train_points,   */
/*                      no_of_test_points.                              */
/* Parameters : input_file :    File name from which points are loaded. */
/*              no_of_points:   number of points to be loaded.          */
/*                              0 : all points are in the training set. */
/*                              -1: all points are in the testing set.  */
/*                              n(>0): n randomly-chosen points         */
/*                                     comprise the training set and    */
/*                                     the rest the testing set.        */
/* Returns :    Nothing.                                                */
/* Calls modules :      error (util.c)                                  */
/*                      load_points                                     */
/*                      allocate_point_array                            */
/* Is called by modules :       main                                    */
/************************************************************************/
read_data(input_file, no_of_points) //ANDRIY loads from a file of data points
char * input_file;
int no_of_points; {

  FILE * infile;
  int i, j, k, count, load_points();
  POINT ** points, ** allocate_point_array();

  if (strlen(input_file) == 0)
    error("Read_Data : No data filename specified.");
  if (no_of_points < -1)
    error("Read_Data : Invalid number of points to be loaded.");
  if ((infile = fopen(input_file, "r")) == NULL)
    error("Read_Data : Data file can not be opened.");

  count = load_points(infile, & points);  //ANDRIY initializes points and returns a count of the number. points now contains
  if (no_of_points != -1) shuffle_points(points, count);
  fclose(infile);

  if (no_of_points > count)
    error("Read_Data : Insufficient data in input file.");

  if (no_of_points == 0 || no_of_points == count) { //ANDRIY a training set is initialized
    no_of_test_points = 0;
    no_of_train_points = count;
  } else if (no_of_points == -1) {
    no_of_test_points = count;
    no_of_train_points = 0;
  } else {
    no_of_test_points = count - no_of_points;
    no_of_train_points = no_of_points;
  }

  //ANDRIY for the wrapper, we can just load the points in train_points to the numpy array loaded into `fit` as done here.
  if (no_of_train_points) { //ANDRIY actually initializes data, we need to call allocate point array with the input to fit.
     //ANDRIY points used for training. This is here as this main method allows for only a sub-set of the loaded points to be used for training
    train_points = allocate_point_array(train_points, no_of_train_points, 0);
    for (i = 1; i <= no_of_train_points; i++) {
      for (j = 1; j <= no_of_dimensions; j++)
        train_points[i] -> dimension[j] = points[i] -> dimension[j];
      train_points[i] -> category = points[i] -> category;
      train_points[i] -> val = points[i] -> val;
    }
  }

  if (no_of_test_points) {
    test_points = allocate_point_array(test_points, no_of_test_points, 0);
    for (i = no_of_train_points + 1; i <= count; i++) {
      k = i - no_of_train_points;

      for (j = 1; j <= no_of_dimensions; j++)
        test_points[k] -> dimension[j] = points[i] -> dimension[j];
      test_points[k] -> category = points[i] -> category;
      test_points[k] -> val = points[i] -> val;
    }
  }

  for (i = 1; i <= count; i++) {
    free_vector(points[i] -> dimension, 1, no_of_dimensions);
    free((char * ) points[i]);
  }
  free((char * )(points + 1));

}

/************************************************************************/
/* Module name : Normalize_Data                                         */
/* Functionality : Translates all points to lie in the positive         */
/*                 quadrant (a requirement for OC1's algorithm).        */
/* Parameters :    points: array of points to be normalized.            */
/*                 no_of_points: number of points.                      */
/* Returns : Nothing.                                                   */
/* Calls modules :  vector (util.c)                                     */
/*                  free_vector (util.c)                                */
/*                  average (util.c)                                    */
/*                  sdev (util.c)                                       */
/* Is called by modules :  build_subtree (mktree.c)                     */
/* Important Variables used : attribute_avg,attribute_sdev,attribute_min:*/
/*           Global arrays that maintain the average, standard deviation*/
/*           and minimum of the instance set seen most recently.        */
/* Remarks : This normalization is done at every tree node. The         */
/*           hyperplane induced is subsequently modified (in the module */
/*           unnormalize_hyperplane) to correspond to the original data.*/
/************************************************************************/
normalize_data(points, no_of_points)
struct point ** points;
int no_of_points; {
  int i, j;
  double * temp, average(), sdev(), min();

  temp = vector(1, no_of_points);

  for (j = 1; j <= no_of_dimensions; j++) {
    for (i = 1; i <= no_of_points; i++) temp[i] = points[i] -> dimension[j];

    attribute_min[j] = min(temp, no_of_points);
    if (attribute_min[j] < 0)
      for (i = 1; i <= no_of_points; i++)
        points[i] -> dimension[j] -= attribute_min[j];

  }
  free_vector(temp, 1, no_of_points);
}

/************************************************************************/
/* Module name : Unnormalize_Data                                       */
/* Functionality : Removes the effects of normalization on data.        */
/* Parameters : points: Array of pointers to the point structures.      */
/*              no_of_points.                                           */
/* Returns : Nothing.                                                   */
/* Calls modules :                                                      */
/* Is called by modules : build_subtree                                 */
/* Important Variables used : attribute_min, attribute_avg,             */
/*                            attribute_sdev: global arrays that        */
/*                            maintain the minimum, average and standard*/
/*                            deviation along all attributes of the     */
/*                            data subset last seen.                    */
/* Remarks : Assumes that the data is normalized.                       */
/************************************************************************/
unnormalize_data(points, no_of_points)
struct point ** points;
int no_of_points; {
  int i, j;

  for (j = 1; j <= no_of_dimensions; j++) {
    if (attribute_min[j] < 0)
      for (i = 1; i <= no_of_points; i++)
        points[i] -> dimension[j] += attribute_min[j];
  }
}

/************************************************************************/
/* Module name : Unnormalize_Hyperplane                                 */
/* Functionality : Removes the effects of normalization on the oblique  */
/*                 hyperplane induced.                                  */
/* Parameters : None.                                                   */
/* Returns : Nothing.                                                   */
/* Calls modules : None.                                                */
/* Is called by modules : build_subtree                                 */
/* Important Variables used : attribute_min, attribute_avg,             */
/*                            attribute_sdev: global arrays that        */
/*                            maintain the minimum, average and standard*/
/*                            deviation along all attributes of the     */
/*                            data subset last seen.                    */
/* Remarks : Assumes that the data is normalized.                       */
/************************************************************************/
unnormalize_hyperplane() {
  int i;

  for (i = 1; i <= no_of_dimensions; i++)
    if (attribute_min[i] < 0)
      coeff_array[no_of_coeffs] -= coeff_array[i] * attribute_min[i];
}
/************************************************************************/
/************************************************************************/