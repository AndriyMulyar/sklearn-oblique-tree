# :deciduous_tree: Oblique Decision Trees in Python
A python interface to oblique decision tree implementations:

- OC1 [(Murphy, Kasif, Salzberg 1994)](https://arxiv.org/pdf/cs/9408103.pdf)
- CART-Linear Combinations (Breiman et al, 1984, Chapter 5)


# Installation

First install `cython` and `numpy` with:

```
pip install cython numpy
```

then run:

```
pip install git+https://github.com/AndriyMulyar/sklearn-oblique-tree
```

# Use

Trees can be induced with the normal scikit-learn classifier api. For instance:

```python
from sklearn.datasets import load_iris, load_breast_cancer
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score
from sklearn_oblique_tree.oblique import ObliqueTree

classifier = ObliqueTree(splitter="oc1", number_of_restarts=20, max_perturbations=5)
X_train, X_test, y_train, y_test = train_test_split(*load_iris(return_X_y=True), test_size=.4)
classifier.fit(X_train, y_train)

predictions = classifier.predict(X_test)


print("Iris Accuracy:",accuracy_score(y_test, predictions))
```

# Acknowledgements
VCU Imbalanced Learning and Data Stream Mining Laboratory     ![alt text](https://nlp.cs.vcu.edu/images/vcu_head_logo "VCU")


