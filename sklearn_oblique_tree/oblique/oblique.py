from sklearn.base import BaseEstimator, ClassifierMixin
from sklearn.utils.validation import check_X_y, check_array, check_is_fitted, check_random_state
from sklearn.utils.multiclass import unique_labels
from ._oblique import Tree

class ObliqueTree(BaseEstimator, ClassifierMixin):


    def __init__(self, splitter="oc1", number_of_restarts=20, max_perturbations=5, random_state=None):
        self.random_state = random_state
        self.splitter = splitter
        self.number_of_restarts = number_of_restarts
        self.max_perturbations = max_perturbations



    def fit(self, X, y):
        """
        Grows an Oblique Decision Tree
        :param X:
        :param y:
        :return:
        """
        X, y = check_X_y(X, y)
        random_state = check_random_state(self.random_state)
        self.classes_ = unique_labels(y)
        self.tree = Tree(splitter = self.splitter)
        self.tree.fit(X,y, self.number_of_restarts, self.max_perturbations)



    def predict(self, X):
        return self.tree.predict(X)

