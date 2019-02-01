from _oblique cimport tree_node #Struct for an oblique tree node with references to children
from libc.stdio cimport printf


cdef class Tree:

    def __cinit__(self, splitter="oc1", random_state=None):
         cdef tree_node *root_node

    def __init__(self, splitter="oc1", random_state=None):
        self.random_state = random_state
        self.splitter = splitter



    cdef fit(self, X, y):
        """
        Grows an Oblique Decision Tree
        :param X:
        :param y:
        :return:
        """
        printf('testing\n')



    def predict(self, X):
        pass

