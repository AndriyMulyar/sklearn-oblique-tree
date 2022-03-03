import numpy
cimport numpy
cdef extern from "stdio.h":
    double drand48()
    void srand48(long int seedval)

cdef extern from "../../oc1_source/mktree.c":

    struct tree_node:
        double *coefficients
        int *left_count
        int *right_count
        tree_node *parent
        tree_node *left
        tree_node *right
        int left_cat
        int right_cat
        char label[50]
          # double alpha #used only in error_complexity pruning.
        int no_of_points
        # pass

    struct test_outcome:
        int leaf_count
        int tree_depth
        

    cdef int no_of_dimensions
    cdef int no_of_categories
    cdef int no_of_restarts
    cdef int no_of_train_points
    cdef int max_no_of_random_perturbations

    cdef int oblique
    cdef int axis_parallel
    cdef int cart_mode


    cdef tree_node* sklearn_root_node
    cdef test_outcome result

    cdef float IMPURITY "IMPURITY"

    ctypedef struct POINT:
        double *dimension
        int category
        double val

    void allocate_structures(int no_of_points)
    void deallocate_structures(int no_of_points)
    void classify(POINT** points, int no_of_points, tree_node* root,char* output)
    
    test_outcome estimate_accuracy(POINT** points, int no_of_points, tree_node* root)
    tree_node* build_tree(POINT** points, int no_of_points, char * dt_file)
    int tree_depth(tree_node* cur_node)
    int leaf_count(tree_node* cur_node)
    int node_count(tree_node* cur_node)
    float* preorder_traversal(tree_node* cur_node)


cdef class Tree:
    cpdef str splitter
    cpdef fit(self, numpy.ndarray[numpy.float_t, ndim=2, mode="c"] X, numpy.ndarray[numpy.int_t, mode="c"] y, long int random_state, str splitter,  int number_of_restarts, int max_perturbations)
    cpdef predict(self, numpy.ndarray y)
    cpdef treeDepth(self)
    cpdef leafCount(self)
    cpdef nodeCount(self)
    cpdef getCoef(self, int attr_num)
