from unittest import TestCase
from sklearn.datasets import load_iris, load_breast_cancer
from sklearn.model_selection import train_test_split
from sklearn_oblique_tree.oblique import ObliqueTree
from sklearn.metrics import accuracy_score

class TestObliqueTree(TestCase):
    """
    Tests model training and prediction in bulk
    """

    @classmethod
    def setUpClass(cls):
        cls.classifier = ObliqueTree
        cls.random_state = 3


    def test_iris(self):
        classifier = self.classifier(splitter="oc1, axis_parallel", random_state = self.random_state)
        X_train, X_test, y_train, y_test = train_test_split(*load_iris(return_X_y=True), test_size=.4, random_state=self.random_state)
        classifier.fit(X_train, y_train)

        predictions = classifier.predict(X_test)


        print("Iris Accuracy:",accuracy_score(y_test, predictions))

    def test_breast(self):
        classifier = self.classifier(splitter="oc1, axis_parallel", random_state=self.random_state)
        X_train, X_test, y_train, y_test = train_test_split(*load_breast_cancer(return_X_y=True), test_size=.4, random_state=self.random_state)
        classifier.fit(X_train, y_train)
        predictions = classifier.predict(X_test)


        print("Breast Cancer Accuracy:", accuracy_score(y_test, predictions))

