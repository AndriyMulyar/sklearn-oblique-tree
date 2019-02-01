from unittest import TestCase
from sklearn.datasets import load_iris
from sklearn.model_selection import train_test_split
from sklearn_oblique_tree.oblique import ObliqueTree

class TestObliqueTree(TestCase):
    """
    Tests model training and prediction in bulk
    """

    @classmethod
    def setUpClass(cls):
        cls.X_train, cls.X_test, cls.y_train, cls.y_test = train_test_split(*load_iris(return_X_y=True), test_size=.4)


    def test_prediction_with_testing_pipeline(self):
        tree = ObliqueTree(splitter="oc1")
        tree.fit(self.X_train, self.y_train)

