from _oblique cimport build_tree, srand48 #Struct for an oblique tree node with references to children
from libc.stdio cimport printf
import numpy as np
cimport numpy as np
from libc.stdlib cimport malloc, free


cdef class Tree:

    def __cinit__(self, str splitter):
        self.splitter = splitter
    def __dealloc__(self):
        global no_of_train_points
        deallocate_structures(no_of_train_points)

    cpdef fit(self, np.ndarray[np.float_t, ndim=2, mode="c"] X, numpy.ndarray[np.int_t, mode="c"] y, long int random_state, str splitter, int number_of_restarts, int max_perturbations):
        """
        Grows an Oblique Decision Tree by calling sub-routines from Murphys implementation of OC1 and Cart-Linear
        :param X:
        :param y:
        :return:
        """
        cdef int num_points = len(y)
        cdef int i
        #modify global settings in implementation
        global no_of_dimensions
        global no_of_categories #number of classes
        global no_of_train_points #number of points trained with
        global sklearn_root_node #point to root node of tree to build
        global no_of_restarts
        global max_no_of_random_perturbations
        global oblique
        global axis_parallel
        global cart_mode
        oblique = False
        axis_parallel = False
        cart_mode = False

        if "oc1" in splitter:
            oblique = True
        if "cart" in splitter: #if this is set, the implementation overrides the other splitters.
            cart_mode = True
        if "axis_parallel" in splitter:
            axis_parallel = True


        srand48(random_state) #set random state

        max_no_of_random_perturbations = max_perturbations
        no_of_train_points = num_points
        no_of_restarts = number_of_restarts

        #no_of_restarts = self.no_of_restarts
        no_of_categories = len(np.unique(y))
        no_of_dimensions = len(X[0])

        cdef POINT ** points = <POINT**> malloc(num_points * sizeof(POINT*))
        allocate_structures(num_points)

        #implementation is indexed from 1 like why the hell.
        points -= 1

        for i in range(1,num_points+1):
            points[i] = <POINT * > malloc( sizeof(POINT *))

        for i in range(1,num_points+1):
            points[i].dimension = (&X[i-1,0] - 1)
            points[i].category = y[i-1] + 1
            points[i].val = 0



        sklearn_root_node = build_tree(points, num_points, NULL)




    cpdef predict(self, np.ndarray[np.float_t, ndim=2, mode="c"] X):
        cdef int num_predict_points = len(X)
        cdef int i
        cdef POINT ** points_predict = <POINT**> malloc(num_predict_points * sizeof(POINT*))
        cdef np.ndarray[np.int32_t, ndim=1] predictions = np.empty(num_predict_points, dtype=np.int32)
        global sklearn_root_node
        points_predict -= 1 #implementation is indexed from 1.

        for i in range(1,num_predict_points+1):
            points_predict[i] = <POINT * > malloc( sizeof(POINT *))

        for i in range(1,num_predict_points+1):
            points_predict[i].dimension = (&X[i-1,0] - 1)
            points_predict[i].category = -1
            points_predict[i].val = 0


        classify(points_predict, num_predict_points, sklearn_root_node, NULL)

        for i in range(1,num_predict_points+1):
            predictions[i-1] = points_predict[i].category - 1 #decrement to account for increment in train

        return predictions






