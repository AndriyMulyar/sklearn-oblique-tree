from sklearn.base import BaseEstimator, ClassifierMixin
from sklearn.utils.validation import check_X_y, check_array, check_is_fitted, check_random_state
from sklearn.utils.multiclass import unique_labels
from ._oblique import Tree

class ObliqueTree(BaseEstimator, ClassifierMixin):


    def __init__(self, splitter="oc1, axis_parallel", number_of_restarts=20, max_perturbations=5, random_state=1):
        """

        :param splitter: 'oc1' for stochastic hill climbing, 'cart' for CART multivariate, 'axis_parallel' for traditional.
        'oc1, axis_parallel' will also consider axis parallel splits when computing best oblique split. Setting 'cart' overrides other options.
        :param number_of_restarts: number of times to restart in effort to escape local minimums
        :param max_perturbations: number of random vector perturbations
        :param random_state: an integer serving as the seed (NOT a numpy random state object)
        """
        self.random_state = random_state
        self.splitter = splitter
        self.number_of_restarts = number_of_restarts
        self.max_perturbations = max_perturbations



    def fit(self, X, y):
        """
        Grows an Oblique Decision Tree
        :param X: a 2d numpy array of attributes
        :param y: a numpy array of integer labels
        :return:
        """
        X, y = check_X_y(X, y)
        random_state = self.random_state
        self.classes_ = unique_labels(y)
        self.tree = Tree(splitter = self.splitter)
        self.tree.fit(X,y, random_state, self.splitter, self.number_of_restarts, self.max_perturbations)



    def predict(self, X):
        return self.tree.predict(X)

