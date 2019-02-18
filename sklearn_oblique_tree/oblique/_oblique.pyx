from _oblique cimport build_tree #Struct for an oblique tree node with references to children
from libc.stdio cimport printf
import numpy as np
cimport numpy as np
from libc.stdlib cimport malloc, free


cdef class Tree:

    def __cinit__(self, str splitter):
        self.splitter = splitter
    def __dealloc__(self):
        #deallocate_structures()
        pass

    cpdef fit(self, np.ndarray[np.float_t, ndim=2, mode="c"] X, numpy.ndarray[np.int_t, mode="c"] y):
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
        global no_of_categories
        global sklearn_root_node

        no_of_categories = len(np.unique(y))
        no_of_dimensions = len(X[0])

        cdef POINT ** points = <POINT**> malloc(num_points * sizeof(POINT*))
        allocate_structures(num_points)

        #implementation is index from 1 like why the hell.
        points -= 1

        for i in range(1,num_points+1):
            points[i] = <POINT * > malloc( sizeof(POINT *))



        print(self.splitter)

        for i in range(1,num_points+1):
            points[i].dimension = (&X[i-1,0] - 1)
            points[i].category = y[i-1] + 1
            points[i].val = 0



        sklearn_root_node = build_tree((points), num_points, NULL)


    cpdef predict(self, np.ndarray[np.float_t, ndim=2, mode="c"] X):
        cdef int num_predict_points = len(X)
        cdef int i
        cdef POINT ** points_predict = <POINT**> malloc(num_predict_points * sizeof(POINT*))
        cdef np.ndarray[np.int32_t, ndim=1] predictions = np.empty(num_predict_points, dtype=np.int32)
        global sklearn_root_node
        printf("%i\n", num_predict_points)
        points_predict -= 1 #implementation is indexed from 1.

        for i in range(1,num_predict_points+1):
            points_predict[i] = <POINT * > malloc( sizeof(POINT *))

        for i in range(1,num_predict_points+1):
            points_predict[i].dimension = (&X[i-1,0] - 1)
            points_predict[i].category = -1
            points_predict[i].val = 0


        classify(points_predict, num_predict_points, sklearn_root_node, NULL)

        for i in range(1,num_predict_points+1):
            predictions[i-1] = points_predict[i].category - 1

        return predictions






