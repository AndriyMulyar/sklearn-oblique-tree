import numpy
cimport numpy

cdef extern from "../../oc1_source/mktree.c":

    struct tree_node:
          # double *coefficients
          # int *left_count
          # int *right_count
          # tree_node *parent
          # tree_node *left
          # tree_node *right
          # int left_cat
          # int right_cat
          # #char label[MAX_DT_DEPTH]
          # double alpha #used only in error_complexity pruning.
          # int no_of_points
        pass

    struct test_outcome:
        pass

    cdef int no_of_dimensions
    cdef int no_of_categories
    cdef int no_of_restarts
    cdef int no_of_train_points
    cdef int max_no_of_random_perturbations

    cdef tree_node* sklearn_root_node

    cdef float IMPURITY "IMPURITY"

    ctypedef struct POINT:
        double *dimension
        int category
        double val

    void allocate_structures(int no_of_points)
    void deallocate_structures(int no_of_points)
    void classify(POINT** points, int no_of_points, tree_node* root,char* output)

    tree_node* build_tree(POINT** points, int no_of_points, char * dt_file)



cdef class Tree:
    cpdef str splitter
    cpdef fit(self, numpy.ndarray[numpy.float_t, ndim=2, mode="c"] X, numpy.ndarray[numpy.int_t, mode="c"] y, int number_of_restarts, int max_perturbations)
    cpdef predict(self, numpy.ndarray y)
