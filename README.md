# :deciduous_tree: Oblique Decision Trees in Python
A python interface to oblique decision tree implementations:

- OC1 [(Murthy, Kasif, Salzberg 1994)](https://arxiv.org/pdf/cs/9408103.pdf)
- CART-Linear Combinations (Breiman et al, 1984, Chapter 5)


# Installation (Python 3)

First install `numpy` with:

```
pip install numpy
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

random_state = 2

#see Murthy, et all for details.
#For oblique with consideration of axis parallel
#tree = ObliqueTree(splitter="oc1, axis_parallel", number_of_restarts=20, max_perturbations=5, random_state=random_state)
#
#For multivariate CART select 'cart' splitter
#tree = ObliqueTree(splitter="cart", number_of_restarts=20, max_perturbations=5, random_state=random_state)

#consider only oblique splits
tree = ObliqueTree(splitter="oc1", number_of_restarts=20, max_perturbations=5, random_state=random_state)

X_train, X_test, y_train, y_test = train_test_split(*load_iris(return_X_y=True), test_size=.4, random_state=random_state)

tree.fit(X_train, y_train)

predictions = tree.predict(X_test)


print("Iris Accuracy:",accuracy_score(y_test, predictions))
```

# Acknowledgements
VCU Imbalanced Learning and Data Stream Mining Laboratory     ![alt text](https://nlp.cs.vcu.edu/images/vcu_head_logo "VCU")


# Original (unmodified) OC1 Source Code
https://github.com/AndriyMulyar/sklearn-oblique-tree/tree/412d502c04d66046388e469a329d8bcf195bf34b/oc1_implementation
